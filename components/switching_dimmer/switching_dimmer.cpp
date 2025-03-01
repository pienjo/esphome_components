#include "esphome/core/log.h"
#include "switching_dimmer.h"

namespace esphome::switching_dimmer
{

static constexpr int pollingInterval {1000};
static constexpr int resetInterval { 60 * 10 };
static const char *const TAG = "SwitchingDimmerOutput";

SwitchingDimmerOutput::SwitchingDimmerOutput(InternalGPIOPin *outputPin)
  : PollingComponent(pollingInterval)
  , mOutputPin { outputPin }
  , mNrPhases {3}
  , mDirection { Direction::DOWN }
  , mPowerupLevel {1.}
  , mWantOn { false }
  , mCurrentPhase { 0 }
  , mTargetPhase { 0 }
  , mIdleCount { 0 }
{
}

void SwitchingDimmerOutput::setup()
{
  mOutputPin->pin_mode(esphome::gpio::FLAG_OUTPUT);
}

void SwitchingDimmerOutput::write_state(float state)
{
  ESP_LOGD(TAG, "writeState(%f)", state);
  float stepSize = 1. / mNrPhases;
  int discreteState = floor(state / stepSize + 0.5);
  
  if (discreteState == 0)
  {
    mWantOn = false;
  } 
  else
  {
    mWantOn = true;
    
    switch (mDirection)
    {
      case Direction::UP:
        mTargetPhase = (discreteState - 1);
        break;
      case Direction::DOWN:
        mTargetPhase = mNrPhases - discreteState;
        break;
    }
  }
  
  ESP_LOGD(TAG, "target state: %d, wantOn:%d", mTargetPhase, (int) mWantOn);
}

void SwitchingDimmerOutput::update()
{
  if (!mWantOn)
  {
    if (isOn())
    {
      turnOff();
      mTargetPhase = mCurrentPhase;
    }
    else if (mIdleCount == resetInterval)
    {
      reset();
    }
    else
    {
      ++mIdleCount;
    }
  }
  else
  {
    if (!isOn())
    {
      turnOn();
    } else if( mCurrentPhase != mTargetPhase)
    {
      turnOff();
    }
  }
}

void SwitchingDimmerOutput::dump_config()
{
  ESP_LOGCONFIG(TAG, "SwitchingDimmerOutput:");
  LOG_PIN("  Output Pin:  ", mOutputPin);
  ESP_LOGCONFIG(TAG, "  Nr phases:   %d", mNrPhases);
  ESP_LOGCONFIG(TAG, "  Direction:   %s", (mDirection == Direction::UP ? "Up" : "Down"));
}

void SwitchingDimmerOutput::set_nr_phases(int nrPhases)
{
  mNrPhases = std::max(1, nrPhases);
}

void SwitchingDimmerOutput::set_direction(Direction direction)
{
  mDirection = direction;
}

void SwitchingDimmerOutput::reset()
{  
  mCurrentPhase = 0;
  mTargetPhase = 0;
  ESP_LOGD(TAG, "Reset");
}

void SwitchingDimmerOutput::turnOn()
{
  mOutputPin->digital_write(true);
  ESP_LOGD(TAG, "turn on, state: %d", mCurrentPhase);
}

void SwitchingDimmerOutput::turnOff()
{
  mOutputPin->digital_write(false);
  mIdleCount = 0;
  int oldPhase = mCurrentPhase;
  mCurrentPhase = (mCurrentPhase + 1) % mNrPhases;
  ESP_LOGD(TAG, "turn off, state: %d -> %d", oldPhase, mCurrentPhase);
}

bool SwitchingDimmerOutput::isOn() const 
{
  return mOutputPin->digital_read();
}

}