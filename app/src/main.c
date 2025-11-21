#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "LED.h"
#include "BTN.h"

#define HOLD_TIME_MS 3000

typedef enum {
    STATE_ENTER_CHAR,
    STATE_ENTER_STRING,
    STATE_SAVE_STRING,
    STATE_STANDBY
} fsm_state_t;

int main(void)
{
    LED_init();
    BTN_init();

    fsm_state_t state = STATE_ENTER_CHAR;

    uint8_t bit_value = 0;
    uint8_t bit_index = 0;
    char string_buffer[64];
    uint8_t string_index = 0;

    LED_blink(LED3, LED_1HZ);

    printk("Starting FSM...\n");

    while (1)
    {
        /* --- Check for standby hold --- */
        if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
            k_msleep(HOLD_TIME_MS);
            if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
                state = STATE_STANDBY;
                printk("Entering STANDBY mode\n");
                LED_pwm(LED0, 0);
                LED_pwm(LED1, 0);
                LED_pwm(LED2, 0);
                LED_pwm(LED3, 0);
            }
        }

        switch (state)
        {
            /* ================================
               STATE 1: ENTER ASCII CHARACTER
            ================================= */
            case STATE_ENTER_CHAR:

                LED_blink(LED3, LED_1HZ);

                if (BTN_check_clear_pressed(BTN0)) {
                    LED_toggle(LED0);
                    bit_value |= (1 << bit_index);
                    bit_index++;
                }

                if (BTN_check_clear_pressed(BTN1)) {
                    LED_toggle(LED1);
                    bit_index++;
                }

                if (BTN_check_clear_pressed(BTN2)) {
                    bit_value = 0;
                    bit_index = 0;
                    printk("Character reset.\n");
                }

                if (BTN_check_clear_pressed(BTN3)) {
                    string_buffer[string_index++] = bit_value;
                    printk("Saved character '%c'\n", bit_value);
                    bit_value = 0;
                    bit_index = 0;
                    state = STATE_ENTER_STRING;
                    LED_blink(LED3, LED_4HZ);
                }

                break;

            /* ================================
               STATE 2: ENTER STRING
            ================================= */
            case STATE_ENTER_STRING:

                LED_blink(LED3, LED_4HZ);

                if (BTN_check_clear_pressed(BTN0)) {
                    LED_toggle(LED0);
                    state = STATE_ENTER_CHAR;
                }

                if (BTN_check_clear_pressed(BTN1)) {
                    LED_toggle(LED1);
                    state = STATE_ENTER_CHAR;
                }

                if (BTN_check_clear_pressed(BTN2)) {
                    string_index = 0;
                    printk("String cleared.\n");
                }

                if (BTN_check_clear_pressed(BTN3)) {
                    state = STATE_SAVE_STRING;
                    LED_blink(LED3, LED_16HZ);
                }

                break;

            /* ================================
               STATE 3: SAVE STRING
            ================================= */
            case STATE_SAVE_STRING:

                LED_blink(LED3, LED_16HZ);

                if (BTN_check_clear_pressed(BTN2)) {
                    printk("Deleting string.\n");
                    string_index = 0;
                    state = STATE_ENTER_CHAR;
                }

                if (BTN_check_clear_pressed(BTN3)) {
                    string_buffer[string_index] = '\0';
                    printk("FINAL STRING: %s\n", string_buffer);
                    state = STATE_ENTER_CHAR;
                }

                break;

            /* ================================
               STATE 4: STANDBY MODE
            ================================= */
            case STATE_STANDBY:

                for (int i = 0; i <= 100; i += 5) {
                    LED_pwm(LED0, i);
                    LED_pwm(LED1, i);
                    LED_pwm(LED2, i);
                    LED_pwm(LED3, i);
                    k_msleep(20);
                }
                for (int i = 100; i >= 0; i -= 5) {
                    LED_pwm(LED0, i);
                    LED_pwm(LED1, i);
                    LED_pwm(LED2, i);
                    LED_pwm(LED3, i);
                    k_msleep(20);
                }

                if (BTN_is_pressed(BTN0) || BTN_is_pressed(BTN1) ||
                    BTN_is_pressed(BTN2) || BTN_is_pressed(BTN3)) {
                    printk("Exiting STANDBY\n");
                    state = STATE_ENTER_CHAR;
                    LED_blink(LED3, LED_1HZ);
                }

                break;
        }

        k_msleep(10);
    }

    return 0;
}

