#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "my_state_machine.h"

static fsm_state_t current_state = STATE_ENTER_CHAR;

static uint8_t bit_index = 0;
static uint8_t current_char = 0;
static char text_buffer[64];
static uint8_t text_len = 0;

static int hold_counter = 0;

static bool both_buttons_held(void) {
    return BTN_check_pressed(BTN0) && BTN_check_pressed(BTN1);
}

static void handle_standby(void) {
    printk("Entered STANDBY mode\n");

    // Fade LEDs manually with PWM
    int duty = 0;
    int dir = 1;

    while (current_state == STATE_STANDBY) {

        // Change brightness
        for (int i = 0; i < 4; i++) {
            LED_pwm(i, duty);
        }

        duty += dir;
        if (duty >= 100) dir = -1;
        if (duty <= 0)  dir = 1;

        // Exit standby on any button
        if (BTN_check_pressed(BTN0) ||
            BTN_check_pressed(BTN1) ||
            BTN_check_pressed(BTN2) ||
            BTN_check_pressed(BTN3)) {
            printk("Exiting standby...\n");
            current_state = STATE_ENTER_CHAR;
            return;
        }

        k_msleep(10);
    }
}

void state_machine_init(void) {
    LED_init();
    BTN_init();
    printk("FSM initialized.\n");
}

void state_machine_run(void) {

    while (1) {

        /* --- Standby trigger: BTN0 + BTN1 held for 3 seconds --- */
        if (both_buttons_held()) {
            hold_counter++;
            if (hold_counter >= 300) {  
                current_state = STATE_STANDBY;
                handle_standby();
            }
        } else {
            hold_counter = 0;
        }

        switch (current_state) {

        /* ------------------------------------------------------ */
        /*                STATE 1: ENTER ONE CHARACTER            */
        /* ------------------------------------------------------ */
        case STATE_ENTER_CHAR:
            LED_blink(LED3, LED_1HZ);

            if (BTN_check_clear_pressed(BTN0)) {
                current_char |= (1 << bit_index);
                bit_index++;
                LED_toggle(LED0);
            }

            if (BTN_check_clear_pressed(BTN1)) {
                // 0 bit, do nothing except increase index
                bit_index++;
                LED_toggle(LED1);
            }

            if (BTN_check_clear_pressed(BTN2)) {
                current_char = 0;
                bit_index = 0;
                printk("Reset bits\n");
            }

            if (BTN_check_clear_pressed(BTN3)) {
                if (bit_index == 8) {
                    text_buffer[text_len++] = current_char;
                    printk("Stored char: %c\n", current_char);

                    bit_index = 0;
                    current_char = 0;

                    current_state = STATE_BUILD_STRING;
                } else {
                    printk("Need 8 bits!\n");
                }
            }
            break;

        /* ------------------------------------------------------ */
        /*            STATE 2: BUILD STRING OF CHARACTERS         */
        /* ------------------------------------------------------ */
        case STATE_BUILD_STRING:
            LED_blink(LED3, LED_4HZ);

            if (BTN_check_clear_pressed(BTN0) || BTN_check_clear_pressed(BTN1)) {
                current_state = STATE_ENTER_CHAR;
            }

            if (BTN_check_clear_pressed(BTN2)) {
                text_len = 0;
                printk("String cleared.\n");
                current_state = STATE_ENTER_CHAR;
            }

            if (BTN_check_clear_pressed(BTN3)) {
                printk("String saved, ready to send.\n");
                current_state = STATE_CONFIRM_SEND;
            }
            break;

        /* ------------------------------------------------------ */
        /*             STATE 3: CONFIRM SEND STRING               */
        /* ------------------------------------------------------ */
        case STATE_CONFIRM_SEND:
            LED_blink(LED3, LED_16HZ);

            if (BTN_check_clear_pressed(BTN2)) {
                text_len = 0;
                printk("Restart from beginning.\n");
                current_state = STATE_ENTER_CHAR;
            }

            if (BTN_check_clear_pressed(BTN3)) {
                printk("Sending string: ");
                for (int i = 0; i < text_len; i++) {
                    printk("%c", text_buffer[i]);
                }
                printk("\n");
                text_len = 0;
                current_state = STATE_ENTER_CHAR;
            }
            break;

        default:
            current_state = STATE_ENTER_CHAR;
            break;
        }

        k_msleep(20);
    }
}
