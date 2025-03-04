#include "IfanRemoteParser.h"
#include "esphome/core/log.h"

#define TAG "IFAN"

IfanRemoteParser::IfanRemoteParser()
  : mParsingState{0}
  , mType{0}
  , mParam{0}
{
}

namespace
{
IfanRemoteParser::Action  handle_command(uint8_t type, uint8_t param)
{
  switch(type)
  {
  case 1:
    switch(param)
    {
    case 1:
      return IfanRemoteParser::Action::WIFI_LONG;
    case 2:
      return IfanRemoteParser::Action::WIFI_SHORT;
    }
    break;
  case 4:
    switch ( param )
    {
    case 0:
      return IfanRemoteParser::Action::FAN_OFF;
    case 1:
      return IfanRemoteParser::Action::FAN_LOW;
    case 2:
      return IfanRemoteParser::Action::FAN_MED;
    case 3:
      return IfanRemoteParser::Action::FAN_HIGH;
    case 4: 
      return IfanRemoteParser::Action::LIGHT; // ???
    }
    break;
  case 6:
    if (param == 1)
      return IfanRemoteParser::Action::EXTRA;
    break;
}
  
  ESP_LOGE(TAG, "Unhandled command: %d %d", type,param);

  return IfanRemoteParser::Action::NONE;
}
    
}
IfanRemoteParser::Action IfanRemoteParser::handleChar(uint8_t c)
{
  Action result { Action::NONE};

  if (c == 0xaa) 
  {
    mParsingState = 0;
  }
  else
  {
    switch (mParsingState) 
    {
    case 1:
      if (c != 0x55)
        mParsingState = 0;
      else
        mParsingState = 2;
      break;
    case 2:
    case 5:
      if (c != 1)
        mParsingState = 0;
      else
        ++mParsingState;
      break;
    case 3:
      mType = c;
      mParsingState = 4;
      break;
    case 4:
      if (c != 0)
        mParsingState = 0;
      else
        mParsingState = 5;
      break;
    case 6:
      mParam = c;
      mParsingState = 7;
      break;
    case 7:
    {
      uint8_t csum = 2 + mType + mParam;
      if (csum != c)
      {
        ESP_LOGE(TAG, "checksum error: %02x != %02x", csum, c);
      }
      else
      {
        result = handle_command(mType, mParam);
      }
      mParsingState = 0;
      break;
    }
    default:
      mParsingState = 0;
      break;
    }
  }

  return result;
}