import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, i2c
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_POSITION, CONF_DATA, CONF_DIMENSIONS

CONF_USER_CHARACTERS = "user_characters"
CONF_BACKLIGHT_ADDRESS = "backlight"
CONF_CLEAR_ON_UPDATE = "clear_on_update"
CONF_HOME_ON_UPDATE = "home_on_update"

lcd_grove_rgb_ns = cg.esphome_ns.namespace("lcd_grove_rgb")
LCDGroveRGB = lcd_grove_rgb_ns.class_(
    "LCDGroveRGB", cg.PollingComponent, i2c.I2CDevice)


def validate_lcd_dimensions(value):
    value = cv.dimensions(value)
    if value[0] > 0x40:
        raise cv.Invalid("LCD displays can't have more than 64 columns")
    if value[1] > 4:
        raise cv.Invalid("LCD displays can't have more than 4 rows")
    return value


def validate_user_characters(value):
    positions = set()
    for conf in value:
        if conf[CONF_POSITION] in positions:
            raise cv.Invalid(
                f"Duplicate user defined character at position {conf[CONF_POSITION]}"
            )
        positions.add(conf[CONF_POSITION])
    return value


CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(LCDGroveRGB),
        cv.Optional(CONF_DIMENSIONS, default="16x2"): validate_lcd_dimensions,
        cv.Optional(CONF_USER_CHARACTERS): cv.All(
            cv.ensure_list(
                cv.Schema(
                    {
                        cv.Required(CONF_POSITION): cv.int_range(min=0, max=7),
                        cv.Required(CONF_DATA): cv.All(
                            cv.ensure_list(cv.int_range(min=0, max=31)),
                            cv.Length(min=8, max=8),
                        ),
                    }
                ),
            ),
            cv.Length(max=8),
            validate_user_characters,
        ),
        cv.Optional(CONF_BACKLIGHT_ADDRESS, default=0x62): cv.i2c_address,
        cv.Optional(CONF_CLEAR_ON_UPDATE, default=True): cv.boolean,
        cv.Optional(CONF_HOME_ON_UPDATE, default=False): cv.boolean,
    }
).extend(cv.polling_component_schema("1s")).extend(i2c.i2c_device_schema(0x3E))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_dimensions(
        config[CONF_DIMENSIONS][0], config[CONF_DIMENSIONS][1]))
    cg.add(var.set_backlight_address(config[CONF_BACKLIGHT_ADDRESS]))
    cg.add(var.set_clear_on_update(config[CONF_CLEAR_ON_UPDATE]))
    cg.add(var.set_home_on_update(config[CONF_HOME_ON_UPDATE]))
    if CONF_USER_CHARACTERS in config:
        for usr in config[CONF_USER_CHARACTERS]:
            cg.add(var.set_user_defined_char(
                usr[CONF_POSITION], usr[CONF_DATA]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(LCDGroveRGB.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
