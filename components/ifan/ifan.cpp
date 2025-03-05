#include "ifan.h"
//#include "esphome/components/fan/fan_helpers.h"
#include "esphome/core/log.h"
#include "esphome.h"
#include <chrono>
#define TAG "IFAN"

namespace esphome::ifan {
namespace
{
  // time in milliseconds
  constexpr int deacceleration_time { 15000 };
  constexpr int acceleration_time { 2000 };
}

IFan::IFan(
    InternalGPIOPin *low_pin,
    InternalGPIOPin *mid_pin,
    InternalGPIOPin *high_pin,
    InternalGPIOPin *buzzer_pin)
  : low_pin_ { low_pin }
  , mid_pin_ { mid_pin }
  , high_pin_{ high_pin }
  , buzzer_pin_ { buzzer_pin }
  , buzzer_enable_ {false}
  , ifan_state_ {DeviceState::Stationary}
  , target_state_ {DeviceState::Stationary}
  , coast_accel_timeout_ {0}
{

}

void IFan::setup() {
  low_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
  mid_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
  high_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
  buzzer_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
  
  //Initialize buzzer to stop errant beeping due to non-initialized pin
  buzzer_pin_->digital_write(true);

  auto restore = restore_state_();
  if (restore.has_value()) {
    restore->apply(*this);
    target_state_ = DeviceState::Stationary;
    if (state)
    {
      target_state_ = (DeviceState) speed;
    }
    ESP_LOGD(TAG, "Target state: %d", (int) target_state_);  
  }
}

void IFan::dump_config() 
{ 
  ESP_LOGCONFIG(TAG, "Ifan:");
  LOG_PIN("  Low Pin:      ", low_pin_);
  LOG_PIN("  Mid Pin:      ", mid_pin_);
  LOG_PIN("  High Pin:      ", high_pin_);
  LOG_PIN("  Buzzer Pin:    ", buzzer_pin_)
  ESP_LOGCONFIG(TAG, "  Buzzer enabled:", (int) buzzer_enable_);
}

fan::FanTraits IFan::get_traits() 
{ 
  return fan::FanTraits(false, true, false, 3); 
}

void IFan::control(const fan::FanCall &call) 
{
  if (call.get_state().has_value())
    state = *call.get_state();
  if (call.get_speed().has_value())
    speed = *call.get_speed();

  target_state_ = DeviceState::Stationary;
  if (state)
  {
    target_state_ = (DeviceState) speed;
  }
  ESP_LOGD(TAG, "Target state: %d", (int) target_state_);
  
  publish_state();
}

void IFan::loop() 
{
  if (target_state_ == ifan_state_)
  {
    return;
  }

  switch(target_state_)
  {
    case DeviceState::Coasting:
    case DeviceState::Stationary:
      low_pin_->digital_write(false);
      mid_pin_->digital_write(false);
      high_pin_->digital_write(false);
        
      if (DeviceState::Coasting == ifan_state_)
      {
        if (millis() >= coast_accel_timeout_)
        {
          set_ifan_state_(DeviceState::Stationary);
        }
      }
      else
      {
        coast_accel_timeout_ = millis() + deacceleration_time;
        set_ifan_state_(DeviceState::Coasting);
      }
      break;
    case DeviceState::Low:
    case DeviceState::Accelerating:
      switch(ifan_state_)
      {
        case DeviceState::Stationary:
          // Ramp up.
          coast_accel_timeout_ = millis() + acceleration_time;
          low_pin_->digital_write(false);
          mid_pin_->digital_write(true);
          high_pin_->digital_write(false);
          set_ifan_state_(DeviceState::Accelerating);
          break;
        case DeviceState::Accelerating:
          if (millis() < coast_accel_timeout_)
          {
            break;
          } 
          [[fallthrough]];
        default:
          low_pin_->digital_write(true);
          mid_pin_->digital_write(false);
          high_pin_->digital_write(false);
          set_ifan_state_(DeviceState::Low);
          break;
      }
      break;
    case DeviceState::Mid:
      low_pin_->digital_write(false);
      mid_pin_->digital_write(true);
      high_pin_->digital_write(false);
      set_ifan_state_(DeviceState::Mid);
      break;
    case DeviceState::High:
      low_pin_->digital_write(false);
      mid_pin_->digital_write(false);
      high_pin_->digital_write(true);
      set_ifan_state_(DeviceState::High);
      break;
  }
}

void IFan::beep(int num) {
  if (!buzzer_enable_)
    return;
  for (int i = 0; i < num; i++) {
    buzzer_pin_->digital_write(false);
    delay(50);
    buzzer_pin_->digital_write(true);
    delay(50);
  }
}

void IFan::long_beep(int num) {
  if (!buzzer_enable_)
    return;
  for (int i = 0; i < num; i++) {
    buzzer_pin_->digital_write(false);
    delay(500);
    buzzer_pin_->digital_write(true);
    delay(500);
  }
}

void IFan::cycle_speed()
{
  int speed_count = get_traits().supported_speed_count();
  if (speed_count)
  {
    if (state)
    {
      // Fan was on.
      int new_speed_count = (speed + 1 ) % (speed_count + 1);

      if (0 == new_speed_count)
      {
        // Cycled around -> stop
        auto call = turn_off();
        call.set_speed(new_speed_count);
        call.perform();
      }
      else
      {
        auto call = turn_on();
        call.set_speed(new_speed_count);
        call.perform();
      }
    } else {
      // fan was off, so set speed to 1
      auto call = turn_on();
      call.set_speed(1);
      call.perform();
    }
  } else {
    // fan doesn't support speed counts, so toggle
    toggle().perform();
  }
}

void ifan::IFan::set_ifan_state_(DeviceState newState)
{
  ESP_LOGD(TAG, "state: %d -> %d", (int) ifan_state_, (int) newState);
  ifan_state_ = newState;
}

}  // namespace esphome::ifan
