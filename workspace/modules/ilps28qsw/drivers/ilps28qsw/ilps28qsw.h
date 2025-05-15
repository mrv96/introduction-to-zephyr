#ifndef ZEPHYR_DRIVERS_SENSOR_ST_ILPS28QSW_
#define ZEPHYR_DRIVERS_SENSOR_ST_ILPS28QSW_

#include <zephyr/drivers/i2c.h>

// Configuration
struct ilps28qsw_config {
	struct i2c_dt_spec i2c;
	uint8_t scale;
};

#pragma pack(1)
union ilps28qsw_data {
    uint8_t buf[5];
    uint32_t raw_pressure:24;
    struct {
        uint16_t :16;
        uint8_t :8;
        int16_t raw_temperature;
    };
};
#pragma pack()

#endif /* ZEPHYR_DRIVERS_SENSOR_ST_ILPS28QSW_ */
