#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON_PIN 10  // Change to your actual GPIO pin

int main() {
    stdio_init_all();
    printf("Button Test Program\n");

    // Wait for USB connection before printing
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }

    printf("USB Serial Ready!\n");

    // Initialize button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  // Use internal pull-up (active low)

    bool prev_state = false;

    while (1) {
        bool pressed = !gpio_get(BUTTON_PIN);  // Inverted: 1 = pressed

        if (pressed && !prev_state) {
            printf("Button Pressed!\n");
        }

        prev_state = pressed;
        sleep_ms(10); // simple debounce delay
    }

    return 0;
}
