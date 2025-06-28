#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// all based on the Raspberry Pi Pico SDK

const uint BUTTON_PIN = 13;

int main() {
    stdio_init_all();
    printf("ADC Example, measuring GPIO26\n");

    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    while (1) {
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        //im changing this to 5V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        //printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        //printf("Voltage: %f V\n", result * conversion_factor);
        //printf("ADC value: %d\n", adc_read()); // serial output
	//float voltage = result * conversion_factor;
	//printf("Voltage variable: %f V\n", voltage);
        sleep_ms(100);

        // Check if the button is pressed
    
        if (gpio_get(BUTTON_PIN)) {
            printf("Button is pressed!\n");
                }
    }
}