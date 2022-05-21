#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

WiFiUDP Udp;

// Network SSID
char *ssid = WIFI_SSID;
char *password = WIFI_PASSWORD;
char *wifi_hostname = "DeskController";

IPAddress ip_null(0, 0, 0, 0);
IPAddress local_IP(0, 0, 0, 0);

unsigned int local_Port = 4711;

IPAddress mc_groupIP(233, 233, 233, 233);
unsigned int mc_Port = 4711;

char recv_Packet[256];
int timeout = 20000;        // wait 20 sec for successfull login
boolean is_connect = false; // ... not yet connected
boolean is_start = false;

const int d1 = 5;  // PIN 5
const int d2 = 4;  // PIN 6
const int d3 = 0;  // PIN 7
const int d4 = 2;  // PIN 8
const int d5 = 14; // PIN 4
const int d6 = 15; // Serial-TX; unused

SoftwareSerial controllerSerial(d5, d6); // RX, TX

const int buttonM[] = {d4};
const int button1[] = {d1};
const int button2[] = {d2, d3};
const int button3[] = {d1, d3};
const int buttonUp[] = {d2};
const int buttonDown[] = {d3};

byte buffer[3];

float height = 0;
bool locked = true;

StaticJsonDocument<200> doc;

void sendUdpMulticast(char *message)
{
  doc["message"] = message;
  Serial.print("send MC-message: ");
  serializeJson(doc, Serial);
  Serial.println();
  Udp.beginPacketMulticast(mc_groupIP, mc_Port, local_IP, 1);
  serializeJson(doc, Udp);
  Udp.println();
  Udp.endPacket();
  doc["message"] = "";
}

void monitorController()
{
  int bufferSize = controllerSerial.readBytes(buffer, 3);
  if (bufferSize >= 3)
  {
    if (buffer[0] + buffer[1] != buffer[2] % 256)
    {
      Serial.print("ERROR: ");
      Serial.print(buffer[0]);
      Serial.print(" ");
      Serial.print(buffer[1]);
      Serial.print(" ");
      Serial.println(buffer[2]);
    }
    else
    {
      //Serial.print(buffer[0]);
      //Serial.print(" ");
      //Serial.print(buffer[1]);
      //Serial.print(" ");
      //Serial.println(buffer[2]);

      bool oldLocked = locked;
      float oldHeight = height;

      if (buffer[0] == (byte)85 && locked)
      {
        locked = false;
        Serial.println("unlocked");
      }
      else if (buffer[0] == (byte)17 && !locked)
      {
        locked = true;
        Serial.println("locked");
      }
      else if (buffer[0] >= (byte)2 && buffer[0] <= (byte)5)
      {
        float newHeight = (buffer[0] * 256 + buffer[1]) / 10.0;
        if (newHeight != height)
        {
          height = newHeight;
          Serial.print("Height: ");
          Serial.println(height);
        }
      }

      doc["height"] = height;
      doc["locked"] = locked;

      if (is_connect && (oldLocked != locked || oldHeight != height))
      {
        sendUdpMulticast("updated");
      }
    }
  }
}

void changeLock()
{
  digitalWrite(buttonM[0], LOW);
  bool oldLocked = locked;
  Serial.println("changing lock");

  while (oldLocked == locked)
  {
    monitorController();
    Serial.print(".");
  }

  digitalWrite(buttonM[0], HIGH);
  Serial.println("changed lock");
}

void setLock(bool state)
{
  if (locked != state)
  {
    changeLock();
  }
  else if (state)
  {
    Serial.println("Already locked");
  }
  else
  {
    Serial.println("Already unlocked");
  }
}

void pressButton(const int *outputs, int size, int msDuration = 750)
{
  for (int i = 0; i < size; i = i + 1)
  {
    Serial.print("Setting ");
    Serial.print(outputs[i]);
    Serial.println(" to LOW");
    digitalWrite(outputs[i], LOW);
  }

  delay(msDuration);

  for (int i = 0; i < size; i = i + 1)
  {
    Serial.print("Setting ");
    Serial.print(outputs[i]);
    Serial.println(" to HIGH");
    digitalWrite(outputs[i], HIGH);
  }
}

void setMode(int modeNumber)
{
  setLock(false);
  switch (modeNumber)
  {
  case 1:
    pressButton(button1, (sizeof(button1) / sizeof(int)));
    break;
  case 2:
    pressButton(button2, (sizeof(button2) / sizeof(int)));
    break;
  case 3:
    pressButton(button3, (sizeof(button3) / sizeof(int)));
    break;
  default:
    Serial.print("Unknown mode: ");
    Serial.println(modeNumber);
  }
}

void setHeight(float newHeight)
{
  if (newHeight > 128 || newHeight < 62)
  {
    Serial.println("Height out of bounds [62, 128]");
    return;
  }

  setLock(false);
  float direction = newHeight - height;
  int out;

  Serial.println(direction);

  while (direction > 0.5 || direction < -0.7)
  {
    if (direction > 0)
    {
      if (out != buttonUp[0])
      {
        digitalWrite(out, HIGH);
        out = buttonUp[0];
      }
    }
    else
    {
      if (out != buttonDown[0])
      {
        digitalWrite(out, HIGH);
        out = buttonDown[0];
      }
    }

    digitalWrite(out, LOW);
    monitorController();
    direction = newHeight - height;
  }

  digitalWrite(out, HIGH);
}

void readCommandsSerial()
{
  if (Serial.available() > 0)
  {
    // read the incoming byte:
    String command = Serial.readString();
    command.trim();

    Serial.print("#");
    Serial.print(command);
    Serial.println("#");

    if (command.equals("up"))
    {
      pressButton(buttonUp, (sizeof(buttonUp) / sizeof(int)));
    }
    else if (command == "down")
    {
      pressButton(buttonDown, (sizeof(buttonDown) / sizeof(int)));
    }
    else if (command == "mode")
    {
      pressButton(buttonM, (sizeof(buttonM) / sizeof(int)));
    }
    else if (command == "lock")
    {
      setLock(true);
    }
    else if (command == "unlock")
    {
      setLock(false);
    }
    else if (command.startsWith("mode"))
    {
      int modeNumber = command.substring(5).toInt();
      setMode(modeNumber);
    }
    else if (command.startsWith("moveTo"))
    {
      int newHeight = command.substring(7).toFloat();
      setHeight(newHeight);
    }
    else
    {
      Serial.print("Unknown command: ");
      Serial.println(command);
    }
  }
}

void readCommandsUdp()
{
  int packetSize = Udp.parsePacket(); // receive incoming UDP packets
  if (packetSize)                     // size is > 0 ?!
  {
    int len = Udp.read(recv_Packet, 255); // read data
    if (len > 0)
      recv_Packet[len] = 0; // add String-End

    IPAddress _destIP = Udp.destinationIP(); // IP send to - MC and UC
    IPAddress _RemoteIP = Udp.remoteIP();    // IP coming from
    int _RemotePort = Udp.remotePort();      // Port coming from

    byte recvmode = 1; // default: UNICAST
    // Evaluate typ of received destination-ip
    if (_destIP[0] >= 224 && _destIP[0] <= 239)
      recvmode = 4;
    else if (_destIP[0] == 255)
      recvmode = 3;
    else if (_destIP[3] == 255)
      recvmode = 2;

         Serial.println("Content " + String(len) + " bytes: " + String(recv_Packet));

         StaticJsonDocument<256> receiveDoc;
         DeserializationError error = deserializeJson(receiveDoc, recv_Packet);

         if (error)

           {
           Serial.print(F("deserializeJson() failed: "));
           Serial.println(error.c_str());
           return;
    }

    String command = receiveDoc["command"];
    command.trim();

    if (command == "setMode")
    {
      int modeNumber = receiveDoc["modeNumber"];
      setMode(modeNumber);
    }
    else if (command == "setHeight")
    {
      float newHeight = receiveDoc["height"].as<float>();
      setHeight(newHeight);
    }
    else
    {
      Serial.println("Unknown command: " + command);
    }
  }
}

void setup()
{
  digitalWrite(d1, HIGH);
  digitalWrite(d2, HIGH);
  digitalWrite(d3, HIGH);
  digitalWrite(d4, HIGH);

  pinMode(d1, OUTPUT);
  pinMode(d2, OUTPUT);
  pinMode(d3, OUTPUT);
  pinMode(d4, OUTPUT);

  Serial.begin(115200);
  controllerSerial.begin(9600);
  controllerSerial.setTimeout(100);
  monitorController();

  doc["height"] = height;
  doc["locked"] = locked;

  WiFi.mode(WIFI_STA);

  Serial.printf("Connecting to %s ", ssid);
  // .... wait for WiFi gets valid !!!
  unsigned long tick = millis(); // get start-time for login
  WiFi.begin(ssid, password);
  while ((!is_connect) && ((millis() - tick) < timeout))
  {
    yield();                    // ... for safety
    is_connect = WiFi.status(); // connected ?
    if (!is_connect)            // only if not yet connected !
    {
      Serial.print("."); // print a dot while waiting
      delay(50);
    }
  }
  if (is_connect)
  {
    Serial.print("after ");
    Serial.print(millis() - tick);
    Serial.println(" ms");
    // .... wait for local_IP becomes valid !!!
    is_connect = false;
    tick = millis(); // get start-time for login
    while ((!is_connect) && ((millis() - tick) < timeout))
    {
      yield(); // ... for safety
      local_IP = WiFi.localIP();
      is_connect = local_IP != ip_null; // connected ?
      if (!is_connect)                  // only if not yet connected !
      {
        Serial.print("."); // print a dot while waiting
        delay(50);
      }
    }
    if (is_connect)
    {
      Serial.print("local_IP valid after ");
      Serial.print(millis() - tick);
      Serial.println(" ms");
      // ... now start UDP and check the result:
      is_connect = Udp.beginMulticast(local_IP, mc_groupIP, mc_Port);
      if (is_connect)
      {
        Serial.print("Listening to Unicast at ");
        Serial.print(local_IP);
        Serial.println(":" + String(local_Port));
        Serial.print("Listening to Multicast at ");
        Serial.print(mc_groupIP);
        Serial.println(":" + String(mc_Port));
        is_start = true; // set flag for 'doing things once' loop
      }
      else
        Serial.println(" - ERROR beginMulticast !");
    }
    else
      Serial.println("local_IP invalid after timeout !");
  }
  else
    Serial.println("- invalid after timeout !");
}

void loop()
{
  if (!is_connect)
    return;

  monitorController();

  if (is_start)
  {
    is_start = false; // reset start-flag
    sendUdpMulticast("I'm alive!");
  }

  readCommandsSerial();
  readCommandsUdp();
}
