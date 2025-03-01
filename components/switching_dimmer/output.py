import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import pins
from esphome.const import CONF_ID, CONF_PIN
from esphome.components import output

#CODEOWNERS = ["@pienjo"]
#DEPENDENCIES = []
#AUTO_LOAD = [ ]
#MULTI_CONF = True
CONF_NR_PHASES = "nr_phases"
CONF_DIRECTION = "direction"

# C++ namespace
ns = cg.esphome_ns.namespace("switching_dimmer")
SwitchingDimmerOutput = ns.class_("SwitchingDimmerOutput", cg.PollingComponent, output.FloatOutput)
DirectionType = ns.enum("SwitchingDimmerOutput::Direction", is_class=True)

DIRECTION_TYPES = {
    "down": DirectionType.DOWN,
    "up": DirectionType.UP
}

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(SwitchingDimmerOutput),
    # Schema definition, containing the options available for the component
    cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_NR_PHASES, default=3): cv.int_range(min=1),
    cv.Optional(CONF_DIRECTION, default=DirectionType.DOWN): cv.enum(DIRECTION_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Code generation
    output_pin = await cg.gpio_pin_expression(config[CONF_PIN])

    var = cg.new_Pvariable(config[CONF_ID], output_pin)
    await output.register_output(var, config)
    await cg.register_component(var, config)
    
    cg.add(var.set_nr_phases(config[CONF_NR_PHASES]))
    cg.add(var.set_direction(config[CONF_DIRECTION]))
    