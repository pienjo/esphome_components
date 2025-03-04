#include "ifan.h"
//#include "esphome/components/fan/fan_helpers.h"
#include "esphome/core/log.h"
#include "esphome.h"
#include "light.h"

#define buzzer 10
#define relay_1 14
#define relay_2 12
#define relay_3 15
#define TAG "IFAN"

namespace esphome {
namespace ifan {

IFan::IFan()
  : current_speed_ {0}
  , buzzer_enable_ {false}
{

}

void IFan::setup() {
  pinMode(buzzer, 0x01);
  pinMode(relay_1, 0x01);
  pinMode(relay_2, 0x01);
  pinMode(relay_3, 0x01);

  //Initialize buzzer to stop errant beeping due to non-initialized pin
  digitalWrite(buzzer, HIGH);

  auto restore = restore_state_();
  if (restore.has_value()) {
    restore->apply(*this);
    current_speed_ = speed;
    write_state_();
  }
}
void IFan::dump_config() 
{ 
  LOG_FAN("", "IFan", this); 
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
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, LOW);
      digitalWrite(relay_3, LOW);
      beep(1);

      break;
    case 2:
      // medium speed
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, HIGH);
      digitalWrite(relay_3, LOW);
      beep(2);
      break;
    case 3:
      // high speed
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, LOW);
      digitalWrite(relay_3, HIGH);
      beep(3);      

      break;

    default:
      // turn off
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, LOW);
      digitalWrite(relay_3, LOW);
      
      long_beep(1);

      break;
  }
}

void IFan::beep(int num) {
  if (!buzzer_enable_)
    return;
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzer, LOW);
    delay(50);
    digitalWrite(buzzer, HIGH);
    delay(50);
  }
}
void IFan::long_beep(int num) {
  if (!buzzer_enable_)
    return;
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
    delay(500);
  }
}

void IFan::cycle_speed()
{
  int speed_count = get_traits().supported_speed_count()
  if (speed_count)
  {
    if (state)
    {
      // Fan was on.
      int new_speed_count = (speed + 1 ) % (supported_speed_count + 1);

      if (0 == new_speed_count)
      {
        // Cycled around
        auto call = turn_off();
        call.set_speed(speed);
        call.perform();
      }
  if (speed_count) {
    if (state) {
      int new_speed = ; 
      if (new_speed > supported_speed_count) {
        // was running at max speed, so turn off
        auto call = turn_off();
        call.perform();
      } else {
        auto call = turn_on();
        call.set_speed(new_speed);
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

void IFan::loop() 
{
  
}
}  // namespace esphome
