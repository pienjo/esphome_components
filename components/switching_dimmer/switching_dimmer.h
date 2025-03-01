#pragma once
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include <esphome/core/hal.h>

#include <optional>

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

    void set_nr_phases(int nrPhases); // Number of dimmimg phases
    
    enum class Direction
    {
        UP,   // Dimming goes up after toggle, e.g. 33% -> 66% -> 100% -> 33%
        DOWN  // Dimming goes up after toggle, e.g. 100% -> 66% -> 33% -> 100%
    };
    void set_direction(Direction direction);
    
protected:
    // Configuration
    InternalGPIOPin *mOutputPin;
    int mNrPhases;
    Direction mDirection;
    float mPowerupLevel;

    bool mWantOn;
    
    int mCurrentPhase;
    int mTargetPhase;
    int mIdleCount;
    
    void reset();
    void turnOn();
    void turnOff();
    bool isOn() const;
};
}