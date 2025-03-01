#pragma once
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include <esphome/core/hal.h>
namespace esphome::switching_dimmer
{
class SwitchingDimmerOutput : public PollingComponent, public output::FloatOutput
{
public:
    explicit SwitchingDimmerOutput(InternalGPIOPin *outputPin);
    
    void setup() override;
    void write_state(float state) override;
    void update() override;
    void dump_config() override;
protected:
    enum class DimmerState
    {
        STATE_HIGH,
        STATE_MID,
        STATE_LOW,
        STATE_COUNT
    };
    
    InternalGPIOPin *mOutputPin;

    bool mWantOn;
    
    DimmerState mCurrentState;
    DimmerState mTargetState;
    int mIdleCount;
    
    void reset();
    void turnOn();
    void turnOff();
    bool isOn() const;
};
}