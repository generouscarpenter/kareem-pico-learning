#include "pico/stdlib.h"
#include "tusb.h"             
#include "tusb_config.h"  
#include "bsp/board_api.h"

// & then pot stuff:
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/stdio.h"

int main()
{
    board_init();
   
    tusb_rhport_init_t dev_init = {     // init device stack on configured roothub port
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while (true) 
    {
      tud_task();                            
      printf("ADC value: %d\n", adc_read()); // Print ADC value for pot
      sleep_ms(10);
    }
}