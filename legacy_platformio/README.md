# Exeta Desk Controller

Remote control your [Exeta motorized desk](https://exeta.de/) (in my case an
[ergoPRO 2019 Edition](https://amzn.eu/d/6zWSsSo)) over the network.

This works with an Arduino board reading the central control box signals and simulating button presses on the
controller.

## Compatibility

This code was written for the control panel CT01-CH and attaches directly to this. I'm not sure, if or how this should
be adapted to work with other exeta desks/controllers or different brands of motorized desks.

It was also developed to be installed on a [WEMOS D1 Mini](https://www.az-delivery.de/products/d1-mini). Compatibility
with other boards is untested.

## Installation

### Physical connection

**_DISCLAIMER:_ Do this at your own risk and keep in mind that your are probably voiding your warranty. Me or my
instructions are not responsible for possible damages.** I just want to share my knowledge after modding my own desk.

Screw open your control unit (mine is a touch controlled one) and take out the PCB. Be careful to not accidentally cut a
wire

Connect your cables from the D1 Mini to the controller PCB by soldering them according to the schematic. **Do not
desolder anything**.

![D1 Mini to controller pcb connection](../doc/schematics/exeta-to-d1mini.png)

### Software installation

Install `platformio` on your machine. This will be used to compile the code and upload it to your board.

Copy `credentials.ini.example` to `credentials.ini`.

```sh
cp credentials.ini.example credentials.ini
```

Insert you WIFI credentials into the `credentials.ini`. The WIFI credentials are currently hardcoded at build time.

```ini
# credentials.ini

[factory_settings]
build_flags =
    ; WiFi settings
    -D WIFI_SSID=\"my_wifi_ssid\"
    -D WIFI_PASSWORD=\"my_wifi_password\"
```

Run `platformio run -t upload` to build and upload the code to your board.

Run `platformio device monitor` to monitor the serial output. You should see debugging information for what the
controller is doing.

#### USB Permissions

If you are on Linux, you might not have access to the serial ports. To fix this, add yourself to the `dialout` group:

```sh
sudo usermod -aG dialout $USER
```

Also, refer to [this guide](https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules).

## Remote Control

To control the desk, send a JSON message via UDP unicast to the ip and port:

```sh
echo '{ "command": "setMode", "modeNumber": 1 }' | nc -w0 -u 192.168.0.59 4711
echo '{ "command": "setMode", "modeNumber": 2 }' | nc -w0 -u 192.168.0.59 4711
echo '{ "command": "setMode", "modeNumber": 3 }' | nc -w0 -u 192.168.0.59 4711
echo '{ "command": "setHeight", "height": 100 }' | nc -w0 -u 192.168.0.59 4711
```

You can also read the height by listening to the UDP multicast on address `233.233.233.233` and port `4711`.
Unfortunately I was unable to find a simple example, you can run on the command line. I'm using UDP multicast in Node
Red like this (you can import below JSON into you own node red instance, if you have one):

```json
[
  {
    "id": "a1e6207d.92381",
    "type": "subflow",
    "name": "Exeta Desk",
    "info": "",
    "category": "",
    "in": [{ "x": 100, "y": 80, "wires": [{ "id": "4c299c13.51f734" }] }],
    "out": [{ "x": 660, "y": 140, "wires": [{ "id": "b4ffd684.b3d7d8", "port": 0 }] }],
    "env": [
      {
        "name": "MULTICAST_IP",
        "type": "str",
        "value": "",
        "ui": { "label": { "en-US": "Multicast IP" }, "type": "input", "opts": { "types": ["str", "env"] } }
      },
      {
        "name": "MULTICAST_PORT",
        "type": "num",
        "value": "",
        "ui": { "label": { "en-US": "Multicast Port" }, "type": "input", "opts": { "types": ["num", "env"] } }
      },
      {
        "name": "UNICAST_IP",
        "type": "str",
        "value": "",
        "ui": { "label": { "en-US": "Unicast IP" }, "type": "input", "opts": { "types": ["str", "env"] } }
      },
      {
        "name": "UNICAST_PORT",
        "type": "num",
        "value": "",
        "ui": { "label": { "en-US": "Unicast Port" }, "type": "input", "opts": { "types": ["num", "env"] } }
      }
    ],
    "color": "#DDAA99",
    "icon": "font-awesome/fa-arrows-v"
  },
  {
    "id": "752be66.c6c5918",
    "type": "udp in",
    "z": "a1e6207d.92381",
    "name": "",
    "iface": "",
    "port": "$(MULTICAST_PORT)",
    "ipv": "udp4",
    "multicast": "true",
    "group": "$(MULTICAST_IP)",
    "datatype": "utf8",
    "x": 250,
    "y": 140,
    "wires": [["b4ffd684.b3d7d8"]]
  },
  {
    "id": "f1997864.692468",
    "type": "udp out",
    "z": "a1e6207d.92381",
    "name": "",
    "addr": "$(UNICAST_IP)",
    "iface": "",
    "port": "$(UNICAST_PORT)",
    "ipv": "udp4",
    "outport": "",
    "base64": false,
    "multicast": "false",
    "x": 520,
    "y": 80,
    "wires": []
  },
  {
    "id": "b4ffd684.b3d7d8",
    "type": "json",
    "z": "a1e6207d.92381",
    "name": "",
    "property": "payload",
    "action": "",
    "pretty": false,
    "x": 530,
    "y": 140,
    "wires": [[]]
  },
  {
    "id": "4c299c13.51f734",
    "type": "json",
    "z": "a1e6207d.92381",
    "name": "",
    "property": "payload",
    "action": "",
    "pretty": false,
    "x": 230,
    "y": 80,
    "wires": [["f1997864.692468"]]
  },
  {
    "id": "a3b5694f.f7a248",
    "type": "subflow:a1e6207d.92381",
    "z": "3918b3ac.96cb6c",
    "name": "",
    "env": [
      { "name": "MULTICAST_IP", "value": "233.233.233.233", "type": "str" },
      { "name": "MULTICAST_PORT", "value": "4711", "type": "num" },
      { "name": "UNICAST_IP", "value": "192.168.0.59", "type": "str" },
      { "name": "UNICAST_PORT", "value": "4711", "type": "num" }
    ],
    "x": 790,
    "y": 520,
    "wires": [["9538a969.8d6178"]]
  }
]
```

## Schematics

Schematics can be found in [`../doc/schematics/`](../doc/schematics).

I also took a few [photos of my own controller](../doc/photos) and the connected D1 Mini. Unfortunately I has the great
idea to secure my soldered wires with hot glue, so it is really hard to see, whats soldered where. Also, I wired the RX
(Pin 4) to D5 instead of D6 on my own controller. But it's only important, the Pin on the ESP is set as RX-Pin for the
`SoftwareSerial controllerSerial` (first parameter) in [`main.cpp`](./src/main.cpp).

I numbered the wires using my own method (nothing, the manufacturer intended or is using in a similar way). Starting
from the power pins, I simply counted from right to left. I'm using this numbering system in this documentation as well
as the code.

### Controller Signaling

The controller sends signals (`HIGH`) over the wires 5 to 8 on button press according to the following mapping:

| Button | Active Pins |
| ------ | ----------- |
| M      | 8           |
| 1      | 5           |
| 2      | 6 + 7       |
| 3      | 5 + 7       |
| Up     | 6           |
| Down   | 7           |

These pins are connected to the D1 Mini as `OUTPUT`. By setting them to `HIGH`, the board simulates a button press on
the controller.

Additionally, it receives UART packages from the central control box via Pin 4 (`SoftwareSerial` RX) with a baud rate of
`9600`. TX is unused. To use this information on the D1 mini, the RX wire is connected as `INPUT` on the D1 Mini and
configured as `SoftwareSerial` RX pin.

Pin 1 and 2 are used for 5V power.

I did not check (or forgot) what pin 3 does. But it is not needed to control the desk.
