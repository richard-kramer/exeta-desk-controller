import hashlib
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, binary_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

exeta_desk_state_ns = cg.esphome_ns.namespace("exeta_desk_state")
ExetaDeskState = exeta_desk_state_ns.class_(
    "ExetaDeskState", cg.Component, uart.UARTDevice
)

CONF_HEIGHT = "height"
CONF_LOCK = "lock"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ExetaDeskState),
            cv.Required(CONF_HEIGHT): sensor.sensor_schema(),
            cv.Required(CONF_LOCK): binary_sensor.binary_sensor_schema(),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    height_conf = config.get(CONF_HEIGHT)
    sens = await sensor.new_sensor(height_conf)
    cg.add(var.set_height(sens))

    # copied from https://github.com/esphome/esphome/blob/dev/esphome/components/globals/__init__.py#L63
    value = height_conf[CONF_ID].id
    if isinstance(value, str):
        value = value.encode()
    hash_ = int(hashlib.md5(value).hexdigest()[:8], 16)
    cg.add(var.set_name_hash(hash_))

    lock_conf = config.get(CONF_LOCK)
    lock = await binary_sensor.new_binary_sensor(lock_conf)
    cg.add(var.set_lock(lock))
