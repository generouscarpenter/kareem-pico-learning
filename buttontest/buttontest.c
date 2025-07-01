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
    gpio_init(BUTTON2_PIN);
    gpio_init(BUTTON3_PIN);
    gpio_init(BUTTON4_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_set_dir(BUTTON3_PIN, GPIO_IN);
    gpio_set_dir(BUTTON4_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN); // Enable pull-up resistor
    gpio_pull_up(BUTTON2_PIN);
    gpio_pull_up(BUTTON3_PIN);
    gpio_pull_up(BUTTON4_PIN);

    while (1) {
        bool button1_pressed = (!gpio_get(BUTTON1_PIN));
        bool button2_pressed = (!gpio_get(BUTTON2_PIN));
        bool button3_pressed = (!gpio_get(BUTTON3_PIN));
        bool button4_pressed = (!gpio_get(BUTTON4_PIN));

        if (button1_pressed) {
            printf("Button 1 is pressed!\n");
        }
        if (button2_pressed) {
            printf("Button 2 is pressed!\n");
        }
        if (button3_pressed) {
            printf("Button 3 is pressed!\n");
        }
        if (button4_pressed) {
            printf("Button 4 is pressed!\n");
        }
        sleep_ms(100);
    }
}