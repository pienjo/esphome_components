import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan
from esphome import automation, pins
from esphome.automation import maybe_simple_id

from esphome.const import (
  CONF_OUTPUT_ID,
  CONF_ID,
)

from esphome.core import  coroutine_with_priority

DEPENDENCIES = []

BUZZER_ENABLE = "buzzer_enable"
REMOTE_ENABLE = "remote_enable"

CONF_LIGHT = "light"
CONF_PIN_BUZZER= "pin_buzzer"
CONF_PIN_LOW = "pin_low"
CONF_PIN_MID = "pin_mid"
CONF_PIN_HIGH = "pin_high"

ifan_ns = cg.esphome_ns.namespace("ifan")
IFan = ifan_ns.class_("IFan", cg.Component, fan.Fan)
CycleSpeedAction = ifan_ns.class_("CycleSpeedAction", automation.Action)

CONFIG_SCHEMA = fan.fan_schema(IFan).extend(
  {
    cv.Required(CONF_PIN_LOW): pins.gpio_output_pin_schema,
    cv.Required(CONF_PIN_MID): pins.gpio_output_pin_schema,
    cv.Required(CONF_PIN_HIGH): pins.gpio_output_pin_schema,
    cv.Required(CONF_PIN_BUZZER): pins.gpio_output_pin_schema,
    cv.Optional(BUZZER_ENABLE, default=True): cv.boolean,
  }
).extend(cv.COMPONENT_SCHEMA)

@automation.register_action(
    "ifan.cycle_speed", 
    CycleSpeedAction, 
    maybe_simple_id({cv.GenerateID(): cv.use_id(IFan)})
)
async def fan_cycle_speed_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)

@coroutine_with_priority(100.0)

async def to_code(config):
  low_pin = await cg.gpio_pin_expression(config[CONF_PIN_LOW])
  mid_pin = await cg.gpio_pin_expression(config[CONF_PIN_MID])
  high_pin = await cg.gpio_pin_expression(config[CONF_PIN_HIGH])
  buzzer_pin = await cg.gpio_pin_expression(config[CONF_PIN_BUZZER])

  cg.add_define("USE_FAN")
  var = await fan.new_fan(config, low_pin, mid_pin, high_pin, buzzer_pin )
  cg.add(var.set_buzzer_enable(config[BUZZER_ENABLE]))
  
  await cg.register_component(var, config)
  await fan.register_fan(var, config)

  cg.add_global(ifan_ns.using)
