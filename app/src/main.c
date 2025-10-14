#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(LED0_NODE, gpios),
	GPIO_DT_SPEC_GET(LED1_NODE, gpios),
	GPIO_DT_SPEC_GET(LED2_NODE, gpios),
	GPIO_DT_SPEC_GET(LED3_NODE, gpios),
};

int main(void) {
	int ret;
	for (int i=0; i<4; i++)  {
		if (!gpio_is_ready_dt(&leds[i])) {
			return -1;
		}
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			return ret;
		}
	}

	while (1) {

		gpio_pin_toggle_dt(&leds[0]);
		gpio_pin_toggle_dt(&leds[2]);
		gpio_pin_toggle_dt(&leds[3]);

		gpio_pin_toggle_dt(&leds[1]);
		k_msleep(250);
		gpio_pin_toggle_dt(&leds[1]);

		k_msleep(500);
	}
}


