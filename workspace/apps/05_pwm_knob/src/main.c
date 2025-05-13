#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>

// Settings
static const int32_t sleep_time_ms = 20;

// Get Devicetree configurations
#define MY_ADC_CH DT_ALIAS(my_adc_channel)
static const struct device *adc = DEVICE_DT_GET(DT_ALIAS(my_adc));
static const struct adc_channel_cfg adc_ch = ADC_CHANNEL_CFG_DT(MY_ADC_CH);
static const struct pwm_dt_spec pwm_led_red = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_red));
static const struct pwm_dt_spec pwm_led_green = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_green));

int main(void)
{
    int ret;
    uint16_t buf;
    uint16_t val_mv;
    int32_t vref_mv;

    // Get Vref (mV) from Devicetree property
    vref_mv = DT_PROP(MY_ADC_CH, zephyr_vref_mv);

    // Buffer and options for ADC (defined in adc.h)
    struct adc_sequence seq = {
        .channels = BIT(adc_ch.channel_id),
        .buffer = &buf,
        .buffer_size = sizeof(buf),
        .resolution = DT_PROP(MY_ADC_CH, zephyr_resolution)
    };

    // Make sure that the ADC was initialized
    if (!device_is_ready(adc)) {
        printk("ADC peripheral is not ready\r\n");
        return 0;
    }

    // Configure ADC channel
    ret = adc_channel_setup(adc, &adc_ch);
    if (ret < 0) {
        printk("Could not set up ADC\r\n");
        return 0;
    }

    if (!pwm_is_ready_dt(&pwm_led_red)) {
        printk("Error: PWM device %s is not ready\n",
            pwm_led_red.dev->name
        );
        return 0;
    }

    if (!pwm_is_ready_dt(&pwm_led_green)) {
        printk("Error: PWM device %s is not ready\n",
            pwm_led_green.dev->name
        );
        return 0;
    }

    // Do forever
    while (1) {

        // Sample ADC
        ret = adc_read(adc, &seq);
        if (ret < 0) {
            printk("Could not read ADC: %d\r\n", ret);
            continue;
        }

        // Calculate ADC value (mV)
        val_mv = buf * vref_mv / (1 << seq.resolution);

        // Print ADC value
        printk("Raw: %u, mV: %u\r\n", buf, val_mv);

        ret =
            pwm_set_pulse_dt(&pwm_led_red, PWM_USEC(buf)) + // period is properly set in DT
            pwm_set_dt(&pwm_led_green, PWM_USEC((1 << seq.resolution) - 1), PWM_USEC(buf)); // period in DT is wrong
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return 0;
        }

        // Sleep
        k_msleep(sleep_time_ms);
    }
}
