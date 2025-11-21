#include <zephyr/kernel.h>
#include "my_state_machine.h"

int main(void) {

    state_machine_init();

    while (1) {
        state_machine_run();
        k_msleep(10);
    }

    return 0;
}


