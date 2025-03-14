#include "exeta_desk_state.h"
#include "esphome/core/log.h"
#include <deque>

namespace esphome
{
  namespace exeta_desk_state
  {

    static const char *TAG = "exeta_desk_state.component";

    void ExetaDeskState::setup()
    {
      float height = 62;
      this->rtc_ = global_preferences->make_preference<float>(1944399030U ^
                                                              this->name_hash_);
      this->rtc_.load(&height);
      this->height_->publish_state(height);
    }

    void ExetaDeskState::on_shutdown() { this->store_height_(); };

    void ExetaDeskState::loop()
    {
      static std::deque<int> buffer;

      while (available())
      {
        int read_int = (int)read();
        if (read_int <= 0)
          continue;

        ESP_LOGV("uart", "read number %d; buffer size: %d", read_int,
                 buffer.size());

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

    void ExetaDeskState::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Empty UART component");
    }

    bool ExetaDeskState::read_state(std::deque<int> buffer)
    {
      if (buffer[0] + buffer[1] != buffer[2] % 256)
      {
        ESP_LOGE("invalid uart state", "%d %d %d", buffer[0], buffer[1], buffer[2]);
        return false;
      }

      ESP_LOGV("uart state", "%d %d %d", buffer[0], buffer[1], buffer[2]);

      if (buffer[0] == 85 && this->lock_->state != false)
      {
        ESP_LOGD("lock", "unlocked");
        this->lock_->publish_state(false);
      }
      else if (buffer[0] == 17 && this->lock_->state != true)
      {
        ESP_LOGD("lock", "locked");
        this->lock_->publish_state(true);
      }
      else if (buffer[0] >= 2 && buffer[0] <= 5)
      {
        float height = (buffer[0] * 256 + buffer[1]) / 10.0;
        if (height != this->height_->get_state())
        {
          this->height_->publish_state(height);
          ESP_LOGD("height", "%.1fcm", this->height_->get_state());
          this->store_height_();
        }
      }

      return true;
    }

  } // namespace exeta_desk_state
} // namespace esphome
