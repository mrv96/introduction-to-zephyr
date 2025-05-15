// Ties to the 'compatible = "st,ilps28qsw"' node in the Devicetree
#define DT_DRV_COMPAT st_ilps28qsw

#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util_macro.h>

#include "ilps28qsw.h"

#define ILPS28QSW_REG_CTRL_REG2 0x11
#define ILPS28QSW_REG_STATUS 0x27
#define ILPS28QSW_REG_PRESS_OUT_XL 0x28

LOG_MODULE_REGISTER(ILPS28QSW, CONFIG_SENSOR_LOG_LEVEL);

//------------------------------------------------------------------------------
// Forward declarations

static int ilps28qsw_reg_read(const struct device *dev, uint8_t reg, uint8_t *val);
static int ilps28qsw_reg_write_8bit(const struct device *dev, uint8_t reg, uint8_t val);
static int ilps28qsw_init(const struct device *dev);
static int ilsp28qsw_swreset(const struct device *dev);
static int ilsp28qsw_set_pressure_scale(const struct device *dev, uint8_t scale);
static int ilps28qsw_sample_fetch(const struct device *dev, enum sensor_channel chan);
static int ilps28qsw_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val);

//------------------------------------------------------------------------------
// Private functions

static int ilps28qsw_reg_read(const struct device *dev, uint8_t reg, uint8_t *val) {
	const struct ilps28qsw_config *cfg = dev->config;
	return i2c_write_read_dt(&cfg->i2c, &reg, sizeof(reg), val, sizeof(*val));
}

static int ilps28qsw_reg_write_8bit(const struct device *dev, uint8_t reg, uint8_t val) {
	const struct ilps28qsw_config *cfg = dev->config;
	uint8_t buf[2] = {
		reg,
		val,
	};

	return i2c_write_dt(&cfg->i2c, buf, sizeof(buf));
}

static int ilps28qsw_init(const struct device *dev) {
	const struct ilps28qsw_config *cfg = dev->config;
	int ret;

	if (!device_is_ready(cfg->i2c.bus)) {
		LOG_ERR("Bus device is not ready");
		return -ENODEV;
	}

    ret = ilsp28qsw_swreset(dev);
	if (ret) {
		LOG_ERR("Could not reset ilsp28qsw module");
		return ret;
	}

	ret = ilsp28qsw_set_pressure_scale(dev, cfg->scale);
	if (ret) {
		LOG_ERR("Could not set the scale of ilsp28qsw module");
		return ret;
	}

    LOG_INF("Initialized");

	return ret;
}

static int ilsp28qsw_swreset(const struct device *dev) {
    int ret;
    uint8_t val = BIT(2);

    ret = ilps28qsw_reg_write_8bit(dev, ILPS28QSW_REG_CTRL_REG2, val);
    if (ret) {
        return ret;
    }

    do {
        ret = ilps28qsw_reg_read(dev, ILPS28QSW_REG_CTRL_REG2, &val);
        if (ret) {
            return ret;
        }
    } while (val & BIT(2));

	return ret;
}

static int ilsp28qsw_set_pressure_scale(const struct device *dev, uint8_t scale) {
    int ret;
    uint8_t val;
    ret = ilps28qsw_reg_read(dev, ILPS28QSW_REG_CTRL_REG2, &val);
    if (ret) {
        return ret;
    }
	return ilps28qsw_reg_write_8bit(dev, ILPS28QSW_REG_CTRL_REG2, WRITE_BIT(val, 6, scale));
}

//------------------------------------------------------------------------------
// Public functions (API)

static int ilps28qsw_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    union ilps28qsw_data *data = dev->data;
    int ret;
    uint8_t flag;
    uint8_t val;

    switch (chan) {
        case SENSOR_CHAN_PRESS:
            flag = BIT(0);
            break;

        case SENSOR_CHAN_AMBIENT_TEMP:
            flag = BIT(1);
            break;

        case SENSOR_CHAN_ALL:
            flag = BIT(0) | BIT(1);
            break;

        default:
		    return -ENOTSUP;
    }

    ret = ilps28qsw_reg_read(dev, ILPS28QSW_REG_CTRL_REG2, &val);
    if (ret) {
        return ret;
    }
    ret = ilps28qsw_reg_write_8bit(dev, ILPS28QSW_REG_CTRL_REG2, WRITE_BIT(val, 0, 1));
    if (ret) {
        return ret;
    }

    do {
        ret = ilps28qsw_reg_read(dev, ILPS28QSW_REG_STATUS, &val);
        if (ret) {
            return ret;
        }
    } while ((val & flag) != flag);

    ARRAY_FOR_EACH (data->buf, i) {
        ret = ilps28qsw_reg_read(dev, ILPS28QSW_REG_PRESS_OUT_XL + i, &data->buf[i]);
        if (ret) {
            return ret;
        }
        LOG_DBG("Fetch %d: 0x%02x", i, data->buf[i]);
    }
    LOG_DBG("Fetched: pressure 0x%06x, temperature 0x%04x", sys_le16_to_cpu(data->raw_pressure), sys_le16_to_cpu(data->raw_temperature));

    return ret;
}

static int ilps28qsw_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
	const struct ilps28qsw_config *cfg = dev->config;
	const union ilps28qsw_data *data = dev->data;
    uint32_t raw_val;
    const uint32_t pressure_divider = 2048 * (cfg->scale == 0 ? 2 : 1);

    LOG_DBG("Got");

    switch (chan) {
        case SENSOR_CHAN_PRESS:
            raw_val = sys_le24_to_cpu(data->raw_pressure);

            /* kPa using float */
            // return sensor_value_from_float(val, raw_val / (float)pressure_divider / 10);

            /* hPa using 32-bit int */
            val->val1 = raw_val / pressure_divider;
            val->val2 = raw_val % pressure_divider * 1000000U / pressure_divider;

            /* kPa using 32-bit int */
            val->val2 = val->val2 / 10 + val->val1 % 10 * 100000U;
            val->val1 /= 10;

            return 0;

        case SENSOR_CHAN_AMBIENT_TEMP:
            raw_val = sys_le16_to_cpu(data->raw_temperature);
            return sensor_value_from_milli(val, raw_val * 10);

        default:
		    return -ENOTSUP;
    }
}

//------------------------------------------------------------------------------
// Devicetree handling

// Define the public API functions for the driver
static const struct sensor_driver_api ilps28qsw_api_funcs = {
	.sample_fetch = ilps28qsw_sample_fetch,
	.channel_get = ilps28qsw_channel_get,
};

// Expansion macro to define driver instances
#define ILPS28QSW_DEFINE(inst)                                              \
                                                                            \
    static union ilps28qsw_data ilps28qsw_data_##inst;                      \
                                                                            \
    /* Create an instance of the config struct, populate with DT values */  \
    static const struct ilps28qsw_config ilps28qsw_config_##inst = {        \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                                  \
        .scale = DT_INST_PROP(inst, scale)                                  \
    };                                                                      \
                                                                            \
    /* Create a "device" instance from a Devicetree node identifier and */  \
    /* registers the init function to run during boot. */                   \
    SENSOR_DEVICE_DT_INST_DEFINE(inst,                                      \
                          ilps28qsw_init,                                   \
                          NULL,                                             \
                          &ilps28qsw_data_##inst,                           \
                          &ilps28qsw_config_##inst,                         \
                          POST_KERNEL,                                      \
                          CONFIG_SENSOR_INIT_PRIORITY,                      \
                          &ilps28qsw_api_funcs);                            \

// The Devicetree build process calls this to create an instance of structs for
// each device (button) defined in the Devicetree
DT_INST_FOREACH_STATUS_OKAY(ILPS28QSW_DEFINE)
