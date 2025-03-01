#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/fan/fan_state.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"

#include "IfanRemoteParser.h"

namespace esphome {
namespace ifan {


class IFan : public Component, public fan::Fan , public uart::UARTDevice {
 public:
#define TAG "IFAN"

  IFan();
  void setup() override;
  void dump_config() override;
  fan::FanTraits get_traits() override;
  void set_buzzer_enable(bool buzzer_enable) { buzzer_enable_ = buzzer_enable; }
  void set_remote_enable(bool remote_enable) { remote_enable_ = remote_enable;};

  Trigger<> *get_light_click_trigger() const { return &mLightClickTrigger; }
  Trigger<> *get_extra_click_trigger() const { return &mExtraClickTrigger; }
  Trigger<> *get_wifi_short_click_trigger() const { return &mWifiShortClickTrigger; }
  Trigger<> *get_wifi_long_click_trigger() const { return &mWifiLongClickTrigger; }
 private:
  void control(const fan::FanCall &call) override;
  void write_state_();
  void do_speed(int lspeed);
  void beep(int);
  void long_beep(int);

  int speed_count;
  int current_speed;
  bool buzzer_enable_;
  bool remote_enable_;
  void loop() override;
  
  Trigger<> mLightClickTrigger;
  Trigger<> mExtraClickTrigger;
  Trigger<> mWifiLongClickTrigger;
  Trigger<> mWifiShortClickTrigger;

  IfanRemoteParser mRemoteParser;
};

template<typename... Ts> class CycleSpeedAction : public Action<Ts...> {
 public:
  explicit CycleSpeedAction(IFan *state) : state_(state) {}

  void play(Ts... x) override {
    // check to see if fan supports speeds and is on
    if (state_->get_traits().supported_speed_count()) {
      if (state_->state) {
        int speed = state_->speed + 33;
        int supported_speed_count = state_->get_traits().supported_speed_count();
        if (speed > supported_speed_count) {
          // was running at max speed, so turn off
          speed = 1;
          auto call = state_->turn_off();
          call.set_speed(speed);
          call.perform();
        } else {
          auto call = state_->turn_on();
          call.set_speed(speed);
          call.perform();
        }
      } else {
        // fan was off, so set speed to 1
        auto call = state_->turn_on();
        call.set_speed(1);
        call.perform();
      }
    } else {
      // fan doesn't support speed counts, so toggle
      state_->toggle().perform();
    }
  }

  IFan *state_;
};
}  // namespace speed
}  // namespace esphome
