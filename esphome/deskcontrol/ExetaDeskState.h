#include "esphome.h"
#include <deque>

class ExetaDeskState : public Component, public UARTDevice
{
public:
  ExetaDeskState(UARTComponent *parent) : UARTDevice(parent) {}

  float get_setup_priority() const override { return esphome::setup_priority::DATA; }

  void setup() override {}

  void loop() override
  {
    static std::deque<int> buffer;

    while (available())
    {
      int read_int = (int)read();
      if (read_int <= 0)
        continue;

      ESP_LOGV("uart", "read number %d; buffer size: %d", read_int, buffer.size());

      if (buffer.size() >= 3)
        buffer.pop_front();
      buffer.push_back(read_int);

      if (buffer.size() < 3)
        continue;

      if (read_state(buffer))
      {
        buffer.clear();
      }
    }
  }

private:
  bool read_state(std::deque<int> buffer)
  {
    if (buffer[0] + buffer[1] != buffer[2] % 256)
    {
      ESP_LOGE("invalid uart state", "%d %d %d", buffer[0], buffer[1], buffer[2]);
      return false;
    }

    ESP_LOGV("uart state", "%d %d %d", buffer[0], buffer[1], buffer[2]);

    if (buffer[0] == 85 && id(input_lock).state != LOCK_STATE_UNLOCKED)
    {
      ESP_LOGD("lock", "unlocked");
      id(input_lock).publish_state(LOCK_STATE_UNLOCKED);
    }
    else if (buffer[0] == 17 && id(input_lock).state != LOCK_STATE_LOCKED)
    {
      ESP_LOGD("lock", "locked");
      id(input_lock).publish_state(LOCK_STATE_LOCKED);
    }
    else if (buffer[0] >= 2 && buffer[0] <= 5)
    {
      float height = (buffer[0] * 256 + buffer[1]) / 10.0;
      if (height != id(desk_height))
      {
        id(desk_height) = height;
        ESP_LOGD("height", "%.1fcm", id(desk_height));
        id(height_sensor).update();
      }
    }

    return true;
  }
};
