#pragma once 
#include <cinttypes>

class IfanRemoteParser
{
public:
    IfanRemoteParser();

    enum class Action
    {
        LIGHT,
        FAN_OFF,
        FAN_LOW,
        FAN_MED,
        FAN_HIGH
    };

    Action handleChar(uint8_t c);

private:
    int mParsingState;

    uint8_t mType;
    uint8_t mParam;

};