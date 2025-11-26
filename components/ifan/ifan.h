#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
// #include "esphome/components/fan/fan_state.h"
#include "esphome/core/automation.h"
#include <esphome/core/hal.h>

namespace esphome::ifan 
{
class IFan : public Component, public fan::Fan {
public:

  IFan(InternalGPIOPin *low_pin,
       InternalGPIOPin *mid_pin,
       InternalGPIOPin *high_pin,
       InternalGPIOPin *buzzer_pin);

  void setup() override;
  void dump_config() override;
  fan::FanTraits get_traits() override;
  void set_buzzer_enable(bool buzzer_enable) { buzzer_enable = buzzer_enable; }
  void cycle_speed();

protected:
  void control(const fan::FanCall &call) override;
  void loop() override;

  void beep(int);
  void long_beep(int);

  // Configuration
  InternalGPIOPin *low_pin_;
  InternalGPIOPin *mid_pin_;
  InternalGPIOPin *high_pin_;
  InternalGPIOPin *buzzer_pin_;
  bool buzzer_enable_;

  enum class DeviceState
  {
    Stationary = 0, // Device has been idle for a while and is prbably stationary
    Low = 1,        // low speed
    Mid = 2,        // medium speed
    High = 3,       // high speed
    Coasting,       // Device has just been turned off and is probably still moving
    Accelerating    // Device needs to go to low but comes from standstill
  };

  DeviceState ifan_state_;
  DeviceState target_state_;

  void set_ifan_state_(DeviceState state);
  
  int coast_accel_timeout_; // in millis
};

template<typename... Ts> class CycleSpeedAction : public Action<Ts...> {
public:
  explicit CycleSpeedAction(IFan *state) : state_(state) {}

  void play(Ts... x) override {
    state_->cycle_speed();   
  }
protected:
  IFan *state_;
};
}  // namespace esphome::ifan
