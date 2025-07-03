#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"


#include "tusb.h"             
#include "tusb_config.h"  
#include "bsp/board_api.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/adc.h"

#include "midi/midi.h"
#include "sound_i2s/sound_i2s.h"
#include "adc.h"
#include "buttons.h"
#include <hardware/structs/qmi.h>


//#define OVERCLOCK_300MHZ  
#define OVERCLOCK_400MHZ      //See RP2350 datasheet, QMI: M0_TIMING, M1_TIMING Registers, CLKDIV bits


uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//*************************************
//**********   I2S STUFF  *************
//*************************************
#define I2S_DATA_PIN             20 // -> I2S DIN
#define I2S_CLOCK_PIN_BASE       18 // -> I2S BCK
// The third required connection is GPIO 27 -> I2S LRCK (BCK+1)

static const struct sound_i2s_config sound_config = 
{
    .pio_num         = 0, // 0 for pio0, 1 for pio1
    .pin_scl         = I2S_CLOCK_PIN_BASE,
    .pin_sda         = I2S_DATA_PIN,
    .pin_ws          = I2S_CLOCK_PIN_BASE + 1,
    .sample_rate     = SAMPLE_RATE,
    .bits_per_sample = 16,
};





//**************************************************************
//**************************************************************
//**********   SECOND MAIN LOOP FOR SECOND CORE  ***************
//**************************************************************
//**************************************************************

uint32_t core1_this_time, core1_main_count;


uint32_t dt_in_us;
uint32_t max_dt_in_us;


bool core_1_trigger_process;
void core1_main()
{
    while(true)
    {


        //********************
        //***  ADC Routine ***
        //********************

        adc_collect();


        //**********************************
        //***  Do nothing every second   ***
        //**********************************
        if(board_millis() - core1_this_time > 999)
        {
            core1_this_time = board_millis();

        }

        core1_main_count++;

        //*************************
        //***  PROCESS AUDIO
        //*************************
        if(core_1_trigger_process)
        {
            process_audio();
            core_1_trigger_process = false;
        }
    }
}


//**************************************************************
//**************************************************************
//**************************************************************
//**************************************************************
//**************************************************************


uint32_t core0_main_count;
uint32_t this_time, blink_time, this_millis;
bool led_state;



int main()
{
    u32 clock_speed;
    //********************
    //***  OVERCLOCK  ***
    //********************
    #ifdef OVERCLOCK_300MHZ
    vreg_set_voltage(VREG_VOLTAGE_1_20);    //300Mhz was locking up at 1.10v, bumping to 1.20v
    clock_speed = 300000;
    set_sys_clock_khz(clock_speed, true);
    #endif

    #ifdef OVERCLOCK_400MHZ
    //***  REDUCE FLASH TIMING CLOCK  ***
    qmi_hw->m[0].timing |= 0x4;                     //qmi_hw->m[0].timing now equals 0x60007207  (raise third bit,              qmi clkdiv is now 7)
    //qmi_hw->m[0].timing &= ~(0x3);                //qmi_hw->m[0].timing now equals 0x60007204  (drop first and second bits,   qmi clkdiv is now 4) 
    qmi_hw->m[0].timing &= ~(0x2);                  //qmi_hw->m[0].timing now equals 0x60007205  (drop the second bit,          qmi clkdiv is now 5) 
    vreg_set_voltage(VREG_VOLTAGE_1_30);            //400 Mhz may be highest achievable clockspeed at 1.30v. 
    clock_speed = 380000;
    set_sys_clock_khz(clock_speed, true);           //400 Mhz needs clockdiv of 7, 380 Mhz works well with clockdiv of 5
    #endif

    board_init();
    init_pins();
    init_buttons();

    init_audio_code();


    //***********************
    //***  TINY USB INIT  ***
    //***********************
   
    tusb_rhport_init_t dev_init = {     // init device stack on configured roothub port
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);


    //************************
    //***  USB UART INIT  ****
    //************************

    stdio_init_all();

    //wait for connection
    while(!tud_cdc_connected())
    {
        tud_task();    
    }

    //******************************************
    //****  I2S AUDIO OUT  *********************
    //******************************************
    sound_i2s_init(&sound_config);
    sound_i2s_playback_start();
    
    //************************
    //***  ADC INIT  ****
    //************************
    printf("about to start adc init\n");
    init_project_adc();



    //*********************************************************************************************
    //*********************************************************************************************
    //******************************  MAIN LOOP  **************************************************
    //*********************************************************************************************
    //*********************************************************************************************
    this_time = board_millis();
    multicore_launch_core1(core1_main);  
    while (true) 
    {
        


        //*****************************
        //***  TinyUSB device task  ***
        //*****************************
        tud_task();                            
        
        //**********************************
        //***  PRINT RUNS THROUGH MAIN   ***
        //**********************************
        if(board_millis() - this_time > 999)
        {
            this_time = board_millis();

            print_cpu_performance_information();
            
            
            //gpio_put(DEBUG_C, !gpio_get(DEBUG_C));
        }

        //********************
        //***  BLINK LED   ***
        //********************
        if(board_millis() - blink_time > blink_interval_ms)
        {
            blink_time = board_millis();

            led_state = !led_state;
            pico_set_led(led_state);
        }

        //*******************************
        //***  ONCE PER MILLISECOND   ***
        //*******************************
        if(board_millis() != this_millis)
        {
            this_millis = board_millis();
            usb_midi_task();
            check_adc_vals();
            process_buttons();
            
        }
        
        core0_main_count++;

    }
}







//*******************************************************************************************
//*******************************************************************************************
//*******************************************************************************************
//*******************************************************************************************


// This is just initializing debug pins and the led pulse
void init_pins(void) 

{

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_init(DEBUG_A);
    gpio_init(DEBUG_B);
    gpio_init(DEBUG_C);

    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_set_dir(DEBUG_A, GPIO_OUT);
    gpio_set_dir(DEBUG_B, GPIO_OUT);
    gpio_set_dir(DEBUG_C, GPIO_OUT);




}





// Turn the led on or off
void pico_set_led(bool led_on) 
{
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);

}


//***************************************************************
//***************************************************************
//***************************************************************
//***************************************************************


void print_cpu_performance_information()
{

    static uint8_t this_count;

    this_count++;
    if(this_count < 5)
    {
        //just clear all the variables and return
        accum_dt = 0;
        accum_dt_count = 0;
        max_dt = 0;
        accum_dt_lockout = false;
        core0_main_count = 0;
        audio_interrupt_count = 0;
        core1_main_count = 0;
        return;
    }

    this_count = 0;
    //**************************************
    //***  CALCULATE AUDIO PERFORMANCE  ****
    //**************************************

    accum_dt_lockout = true;

    dt_in_us     = accum_dt / accum_dt_count;
    max_dt_in_us = max_dt;
    ave_dt_in_us = accum_dt / accum_dt_count;

    //*************************
    //***  RESET VARIABLES  ***
    //*************************
    accum_dt = 0;
    accum_dt_count = 0;
    max_dt = 0;
    accum_dt_lockout = false;


    // ***  NOW WITH COMMA FORMATTING  !
    char formatted_num_string[20];

    //****************
    //***  CORE 0  ***
    //****************
    format_with_commas(core0_main_count, formatted_num_string);
    printf("\nCore 0 - Runs through main: %s\n", formatted_num_string);

    //****************
    //***  CORE 1  ***
    //****************
    format_with_commas(core1_main_count, formatted_num_string);
    printf("Core 1 - Runs through main: %s\n", formatted_num_string);

    //**********************
    //***  Running Time  ***
    //**********************
    
    // printf("Time (in millis)          : %d\n", this_time);

    //******************************
    //***  AUDIO INTERRUPT INFO  ***
    //******************************

    float percentage_full =   (((float)max_dt_in_us) / MAX_TIME_FOR_AUDIO_INTERRUPT)  * 100;

    //printf("Audio Interrupts per sec: %d\n", audio_interrupt_count);
    printf("ave_dt_in_us: %d    max_dt: %d   CPU%%: %d\n", (int)ave_dt_in_us, (int)max_dt_in_us, (int)percentage_full);


    //*******************
    //***  CPU CLOCK  ***
    //*******************

    // printf("qmi_hw->m[0].timing: %x\n", qmi_hw->m[0].timing);
    // printf("clock_speed:  %d\n", clock_speed/1000);


    //*********************
    //***  ADC METRICS  ***
    //*********************
    printf("ADC Scan time: %d  mS\n", time_to_finish_adc_scan);

    //*****  RESET RUNNING VARIABLES  ***
    core0_main_count = 0;
    audio_interrupt_count = 0;
    core1_main_count = 0;



}


