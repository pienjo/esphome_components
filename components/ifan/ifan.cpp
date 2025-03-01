#include "ifan.h"
//#include "esphome/components/fan/fan_helpers.h"
#include "esphome/core/log.h"
#include "esphome.h"
#include "light.h"
namespace esphome {
namespace ifan {
#define buzzer 10
#define relay_1 14
#define relay_2 12
#define relay_3 15
#define TAG "IFAN"
int target_fan_speed;
int start_time_offset;

IFan::IFan()
  : current_speed {0}
  , buzzer_enable {false}
  , remote_enable {false}
  , mLightClickTrigger{}
  , mExtraClickTrigger{}
  , mWifiLongClickTrigger{}
  , mWifiShortClickTrigger{}
  , mRemoteParser{}
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
    current_speed = speed;
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
  if (!buzzer_enable)
    return;
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzer, LOW);
    delay(50);
    digitalWrite(buzzer, HIGH);
    delay(50);
  }
}
void IFan::long_beep(int num) {
  if (!buzzer_enable)
    return;
  for (int i = 0; i < num; i++) {
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
    delay(500);
  }
}

void IFan::loop() {
    if (!remote_enable)
        return;
    while (available()) {
        uint8_t c;
        read_byte(&c);
        switch(mRemoteParser.handleChar(c))
        {
        case IfanRemoteParser::Action::FAN_OFF:
          do_speed(0);
          break;
        case IfanRemoteParser::Action::FAN_LOW:
          do_speed(1);
          break;
        case IfanRemoteParser::Action::FAN_MED:
          do_speed(2);
          break;
        case IfanRemoteParser::Action::FAN_HIGH:
          do_speed(3);
          break;
        case IfanRemoteParser::Action::LIGHT:
          mLightClickTrigger.trigger();
          break;
        case IfanRemoteParser::Action::EXTRA:
          mExtraClickTrigger.trigger();
          break;
        case IfanRemoteParser::Action::WIFI_SHORT:
          mWifiShortClickTrigger.trigger();
          break;
        case IfanRemoteParser::Action::WIFI_LONG:
          mWifiLongClickTrigger.trigger();
          break;
        case IfanRemoteParser::Action::NONE:
        default:
          break;
        }
    }
}

#if 0

void IFan::handle_char_(uint8_t c) {
    if (!remote_enable)
        return;
    static int state = 0;
    static uint8_t type = 0;
    static uint8_t param = 0;
    uint8_t csum;
}
#endif
}  // namespace ifan

}  // namespace esphome
