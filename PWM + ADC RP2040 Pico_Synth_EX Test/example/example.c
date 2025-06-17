#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_synth_ex.h"    // Original code by ISGK Instruments (Ryo Ishigaki), licensed under CC0
#include "notes.h"            // A simple table matching note names to Midi numbers
#include "hardware/clocks.h" // For set_sys_clock_khz()
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

// You can define your audio output in CMakeLists.txt.
// Under target_compile_definitions, choose either
// USE_AUDIO_PWM=1 or USE_AUDIO_I2S=1 (but not both).

#if USE_AUDIO_PWM
  // Mono output, disabling right PWM output
  // for this example.
  #define AUDIO_PIN_RIGHT    -1
  #define AUDIO_PIN_LEFT     14

#elif USE_AUDIO_I2S
  #include "sound_i2s.h"
  #define I2S_DATA_PIN             28 // -> I2S DIN
  #define I2S_CLOCK_PIN_BASE       26 // -> I2S BCK
  // The third required connection is GPIO 27 -> I2S LRCK (BCK+1)

  static const struct sound_i2s_config sound_config = {
    .pin_sda         = I2S_DATA_PIN,
    .pin_scl         = I2S_CLOCK_PIN_BASE,
    .pin_ws          = I2S_CLOCK_PIN_BASE + 1,
    .sample_rate     = 44100,
    .pio_num         = 0, // 0 for pio0, 1 for pio1
  };

  repeating_timer_t i2s_timer;
#endif

int main() {
  stdio_init_all();

  //overclokcing at 400MHz:
  //vreg_set_voltage(VREG_VOLTAGE_1_30);
  //set_sys_clock_khz(400000, true);

  adc_init();
  adc_gpio_init(26); // ADC0 on GPIO 26
  adc_select_input(0);


  // Start the synth.
  #if USE_AUDIO_PWM
    // Pass the two output GPIOs as arguments.
    // Left channel must be active.
    // For a mono setup, the right channel can be disabled by passing -1.
    PWMA_init(AUDIO_PIN_RIGHT, AUDIO_PIN_LEFT);
  #elif USE_AUDIO_I2S
    sound_i2s_init(&sound_config);
    sound_i2s_playback_start();
    add_repeating_timer_ms(10, i2s_timer_callback, NULL, &i2s_timer);
  #endif

  while (1) {
    uint16_t adc_filter = (adc_read()); // Scale ADC value to 1-100 range;
    printf("ADC VALUE: %d\n", adc_filter);

    control_message(PRESET_9);
    note_on(74);
    // Parameters can be set directly
    set_parameter(FILTER_MOD_AMOUNT, adc_filter);
    sleep_ms(500);
  }



}
