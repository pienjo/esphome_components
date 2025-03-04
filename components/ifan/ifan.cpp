#include "ifan.h"
//#include "esphome/components/fan/fan_helpers.h"
#include "esphome/core/log.h"
#include "esphome.h"

#define TAG "IFAN"

namespace esphome::ifan {

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
  , current_speed_ {0}
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
    current_speed_ = speed;
    write_state_();
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

  write_state_();
  publish_state();
}

void IFan::write_state_() 
{
  int local_speed = static_cast<int>(speed);
  ESP_LOGD(TAG, "State: %s, Speed: %i ",state ? "ON" : "OFF", local_speed);
  if (!state)
    do_speed(0);
  if (state)
    do_speed(local_speed);
}  // write_state_

void IFan::do_speed(const int lspeed){
  switch (lspeed) {
    case 1:
      // low speed
      low_pin_->digital_write(true);
      mid_pin_->digital_write(false);
      high_pin_->digital_write(false);

      beep(1);

      break;
    case 2:
      // medium speed
      low_pin_->digital_write(false);
      mid_pin_->digital_write(true);
      high_pin_->digital_write(false);
      
      beep(2);
      break;
    case 3:
      // high speed
      low_pin_->digital_write(false);
      mid_pin_->digital_write(false);
      high_pin_->digital_write(true);
      beep(3);      

      break;

    default:
      // turn off
      low_pin_->digital_write(false);
      mid_pin_->digital_write(false);
      high_pin_->digital_write(false);
      
      long_beep(1);

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

}  // namespace esphome::ifan
