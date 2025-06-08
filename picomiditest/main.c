#include <stdlib.h> // For abs()
#include "pico/stdlib.h"
#include "tusb.h"
#include "tusb_config.h"
#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/stdio.h"

// this function sends a midi control message
void send_midi_cc(uint8_t channel, uint8_t controller, uint8_t value) {
    uint8_t msg[3] = { (uint8_t)(0xB0 | (channel & 0x0F)), controller & 0x7F, value & 0x7F };
    tud_midi_stream_write(0, msg, 3);
}

int main()
{
    board_init();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    
    int prev_midipotvalue = -1; // Initialize to an invalid value
    const int deadband = 1; // ignore changes of +-1 because the pot is noisy

    while (true)
    {
      tud_task();
      int midipotvalue = adc_read() >> 5; // 0-127

      if (abs(midipotvalue - prev_midipotvalue) > deadband) { // only runs if value has changed more than 1
        printf("ADC value: %d\n", midipotvalue); // serial output
        send_midi_cc(0, 1, midipotvalue); // Channel 1, CC 1
        prev_midipotvalue = midipotvalue; // Update the previous value
      }
    }
}