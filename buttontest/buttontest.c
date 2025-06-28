#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON1_PIN 10
#define BUTTON2_PIN 11
#define BUTTON3_PIN 12
#define BUTTON4_PIN 13


int main() {
    stdio_init_all();
    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN); // Enable pull-up resistor

    while (1) {
        bool button_pressed = (!gpio_get(BUTTON1_PIN));
        if (button_pressed) {
            printf("Button is pressed!\n");
            
        }
        sleep_ms(100);
    }
}