#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Settings
static const int32_t sleep_time_ms = 50;
static const struct gpio_dt_spec btns[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(button_red), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(button_green), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(button_blue), gpios),
};
static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(led_red), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led_green), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led_blue), gpios),
};

int main(void)
{
    int ret;
    int state;

    ARRAY_FOR_EACH(btns, i) {
        if (!gpio_is_ready_dt(&btns[i])) {
            printk("ERROR: button not ready\r\n");
            return 0;
        }
        if (!gpio_is_ready_dt(&leds[i])) {
            printk("ERROR: led not ready\r\n");
            return 0;
        }

        ret = gpio_pin_configure_dt(&btns[i], GPIO_INPUT);
        if (ret < 0) {
            return 0;
        }
        ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT);
        if (ret < 0) {
            return 0;
        }

        // Print out the flags
        printk("Button %d spec flags: 0x%x\n", i, btns[i].dt_flags);
        printk("LED %d spec flags: 0x%x\n", i, leds[i].dt_flags);
    }

    // Do forever
    while (1) {

        ARRAY_FOR_EACH(btns, i) {
            // Poll button state
            state = gpio_pin_get_dt(&btns[i]);
            if (state < 0) {
                printk("Error %d: failed to read button pin %d\n", state, i);
            } else {
                printk("Button %d state: %d ", i, state);
                printk("Turning LED %d %s\n", i, state ? "ON" : "OFF");
                state = gpio_pin_set_dt(&leds[i], state);
                if (state != 0) {
                    printk("Error %d: failed to set led pin %d\n", i, state);
                }
            }
        }

        // Sleep
        k_msleep(sleep_time_ms);
    }

    return 0;
}
