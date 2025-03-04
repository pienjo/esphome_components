#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/fan/fan_state.h"
#include "esphome/core/automation.h"

namespace esphome::ifan 
{
class IFan : public Component, public fan::Fan {
public:

  IFan();
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

  int current_speed_;
  bool buzzer_enable_;
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
