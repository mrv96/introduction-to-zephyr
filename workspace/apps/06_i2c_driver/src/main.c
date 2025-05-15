#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

// Settings
static const int32_t sleep_time_ms = 1000;
static const struct device *barometer = DEVICE_DT_GET(DT_ALIAS(my_sensor));

static const char *now_str(void)
{
	static char buf[16]; /* ...HH:MM:SS.MMM */
	uint32_t now = k_uptime_get_32();
	unsigned int ms = now % MSEC_PER_SEC;
	unsigned int s;
	unsigned int min;
	unsigned int h;

	now /= MSEC_PER_SEC;
	s = now % 60U;
	now /= 60U;
	min = now % 60U;
	now /= 60U;
	h = now;

	snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
		 h, min, s, ms);
	return buf;
}

int main(void)
{
    int ret;
    double pressure, temperature;

    // Make sure that the button was initialized
    if (!device_is_ready(barometer)) {
        printk("Error: barometer is not ready\r\n");
        return 0;
    }

    // Do forever
    while (1) {
		struct sensor_value val;

        // Trigger sampling
		ret = sensor_sample_fetch(barometer);
		if (ret) {
			printk("sensor_sample_fetch error: %d\n", ret);
			continue;
		}

        // Read pressure
		ret = sensor_channel_get(barometer, SENSOR_CHAN_PRESS, &val);
		if (ret) {
			printk("sensor_channel_get error: %d\n", ret);
			continue;
		}
        pressure = sensor_value_to_double(&val);

        // Read temperature
		ret = sensor_channel_get(barometer, SENSOR_CHAN_AMBIENT_TEMP, &val);
		if (ret) {
			printk("sensor_channel_get error: %d\n", ret);
			continue;
		}
        temperature = sensor_value_to_double(&val);

        // Print values
		printk("[%s] pressure: %g kPa, temperature: %g C\r\n", now_str(), pressure, temperature);

		// Sleep
        k_msleep(sleep_time_ms);
	}

    return 0;
}
