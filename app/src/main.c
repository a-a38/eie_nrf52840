/*
 * main.c
 */
/*
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
*/

#include <zephyr/kernel.h>
#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 10  // check button every 10ms



int main(void) {
    if (BTN_init() < 0) {
        return 0;
    }
    if (LED_init() < 0) {
        return 0;
    }

    uint8_t counter = 0;

    while (1) {
        // Check if BTN0 was pressed
        if (BTN_check_clear_pressed(BTN0)) {
            counter = (counter + 1) % 16; // 4-bit counter rolls over at 16

            // Update LEDs based on counter bits
            for (int i = 0; i < 4; i++) {
                if (counter & (1 << i)) {
                    LED_set((led_id)i, LED_ON);
                } else {
                    LED_set((led_id)i, LED_OFF);
                }
            }

            printk("Counter: %d\n", counter);
        }

        k_msleep(SLEEP_MS);
    }

    return 0;
}