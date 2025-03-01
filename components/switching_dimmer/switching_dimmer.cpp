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
  , mWantOn { false }
  , mCurrentState { DimmerState::STATE_HIGH }
  , mTargetState { DimmerState::STATE_HIGH }
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
  
  if (state < 0.10)
  {
    mWantOn = false;
  } else if (state < 0.33)
  {
    mWantOn = true;
    mTargetState = DimmerState::STATE_LOW;
  } else if (state < 0.66)
  {
    mWantOn = true;
    mTargetState = DimmerState::STATE_MID;
  }
  else
  {
    mTargetState = DimmerState::STATE_HIGH;
    mWantOn = true;
  }
  
  ESP_LOGD(TAG, "target state: %d, wantOn:%d", mTargetState, (int) mWantOn);
}

void SwitchingDimmerOutput::update()
{
  if (!mWantOn)
  {
    if (isOn())
    {
      turnOff();
      mTargetState = mCurrentState;
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
    } else if( mCurrentState != mTargetState)
    {
      turnOff();
    }
  }
}

void SwitchingDimmerOutput::dump_config()
{
  ESP_LOGCONFIG(TAG, "SwitchingDimmerOutput:");
  LOG_PIN("  Output Pin:  ", mOutputPin);
}

void SwitchingDimmerOutput::reset()
{
  mCurrentState = DimmerState::STATE_HIGH;
  mTargetState = DimmerState::STATE_HIGH;
  ESP_LOGD(TAG, "Reset");
}

void SwitchingDimmerOutput::turnOn()
{
  mOutputPin->digital_write(true);
  ESP_LOGD(TAG, "turn on, state: %d", mCurrentState);
}

void SwitchingDimmerOutput::turnOff()
{
  mOutputPin->digital_write(false);
  mIdleCount = 0;
  DimmerState oldState { mCurrentState };
  mCurrentState = (DimmerState) (((int)mCurrentState + 1) % (int)(DimmerState::STATE_COUNT));
  ESP_LOGD(TAG, "turn off, state: %d -> %d", oldState, mCurrentState);
}

bool SwitchingDimmerOutput::isOn() const 
{
  return mOutputPin->digital_read();
}

}