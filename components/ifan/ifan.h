#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/fan/fan_state.h"
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
  void write_state_();
  void do_speed(int lspeed);
  void beep(int);
  void long_beep(int);

  // Configuration
  InternalGPIOPin *low_pin_;
  InternalGPIOPin *mid_pin_;
  InternalGPIOPin *high_pin_;
  InternalGPIOPin *buzzer_pin_;
  bool buzzer_enable_;

  // State
  int current_speed_;
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
