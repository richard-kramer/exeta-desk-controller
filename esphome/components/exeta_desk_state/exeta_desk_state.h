#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome
{
  namespace exeta_desk_state
  {

    class ExetaDeskState : public uart::UARTDevice, public Component
    {
    public:
      void set_height(sensor::Sensor *height) { this->height_ = height; };
      void set_name_hash(uint32_t name_hash) { this->name_hash_ = name_hash; };
      void set_lock(binary_sensor::BinarySensor *lock) { this->lock_ = lock; };

      float get_setup_priority() const override
      {
        return esphome::setup_priority::DATA;
      }

      void setup() override;
      void loop() override;
      void on_shutdown() override;
      void dump_config() override;

    private:
      bool read_state(std::deque<int> buffer);

    protected:
      sensor::Sensor *height_{};
      binary_sensor::BinarySensor *lock_{};

      uint32_t name_hash_{};
      ESPPreferenceObject rtc_{};

      void store_height_()
      {
        float height = this->height_->get_state();
        this->rtc_.save(&height);
      };
    };

  } // namespace exeta_desk_state
} // namespace esphome
