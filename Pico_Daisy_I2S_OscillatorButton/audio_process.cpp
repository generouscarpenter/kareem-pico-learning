#include "main.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "pico/time.h"
#include "math.h"
#include "sound_i2s.h"
#include <stdalign.h> // For alignas and alignof in C11
#include "pico/sync.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "buttons.h"
using namespace daisysp;

alignas(32) int16_t buffer_0[I2S_BLOCK_SIZE * 2];
alignas(32) int16_t buffer_1[I2S_BLOCK_SIZE * 2];

#define OSC_NUMBER 5

Oscillator sine_osc;
Oscillator tri_osc;
Oscillator saw_osc;
Oscillator square_osc;
Oscillator rect_osc;
Oscillator lfo;

static MoogLadder m_filter;
static Overdrive overdrive;
static ReverbSc reverb;

#define START_FREQ 80.f




u32 audio_interrupt_count;

//********************************************
//****  INIT AUDIO DEVICES
//********************************************

void init_audio_code(void)
{
    //init all 4 wave types:

   sine_osc.Init(SAMPLE_RATE);
   // Set parameters for oscillator
    sine_osc.SetWaveform(sine_osc.WAVE_SIN);
    sine_osc.SetFreq(START_FREQ);
    sine_osc.SetAmp(0.5);

   tri_osc.Init(SAMPLE_RATE);
   // Set parameters for oscillator
    tri_osc.SetWaveform(tri_osc.WAVE_TRI);
    tri_osc.SetFreq(START_FREQ);
    tri_osc.SetAmp(0.5);

    saw_osc.Init(SAMPLE_RATE);
   // Set parameters for oscillator
    saw_osc.SetWaveform(saw_osc.WAVE_SAW);
    saw_osc.SetFreq(START_FREQ);
    saw_osc.SetAmp(0.5);

    square_osc.Init(SAMPLE_RATE);
    // Set parameters for oscillator
    square_osc.SetWaveform(square_osc.WAVE_SQUARE);
    square_osc.SetFreq(START_FREQ);
    square_osc.SetAmp(0.5);

    rect_osc.Init(SAMPLE_RATE);

   // Set parameters for oscillator
    rect_osc.SetWaveform(sine_osc.WAVE_POLYBLEP_SQUARE);
    rect_osc.SetFreq(START_FREQ);
    rect_osc.SetAmp(0.5);

    // initialize Moogladder object
    m_filter.Init(SAMPLE_RATE);
    m_filter.SetRes(0.7);

    // intitialize reverb

    reverb.Init(SAMPLE_RATE);

    // set parameters for LFO
    lfo.Init(SAMPLE_RATE);
    lfo.SetWaveform(Oscillator::WAVE_TRI);
    lfo.SetAmp(1);
    lfo.SetFreq(.4);

}


float choose_osc(int osc_num){
    switch (osc_num) {
        case 0:
            return sine_osc.Process();
            break;
        case 1:
            return tri_osc.Process();
            break;
        case 2:
            return saw_osc.Process();
            break;
        case 3:
            return square_osc.Process();
            break;
        default: return 0.0f;
    }
}



u32 accum_dt, ave_dt_in_us, accum_dt_count;
u32 dt;
u32 max_dt;
bool accum_dt_lockout;
u32 t0;


//********************************************
//****  AUDIO PROCESSING INTERRUPT
//********************************************

void process_audio(void)
{
    //**********************************
    //***  JUST SOME TIMING METRICS  ***
    //**********************************
    t0 = timer_hw->timerawl; 
    audio_interrupt_count++;


    //*****************************************************
    //***  CONVERT TO INT16 AND TRANSFER TO I2S BUFFER  ***
    //*****************************************************
    int16_t * buff = sound_i2s_get_next_buffer();
    for(int i=0; i<BLOCK_SIZE; i++)
    {

        //*** TAKE LFO OUTPUT TO SET MOOG FILTER ***
        float lfo_sig  = lfo.Process();
        float freq = 5000 + (lfo_sig * 5000);
        m_filter.SetFreq(freq);
        //*** PUT WAVEFORMS THROUGH FILTER  ***
        float non_filtered_output = choose_osc(button_count);
        /*float filter_output = m_filter.Process(choose_osc(button_counter()));

        //RUN FILTER THRU REVERB
        float reverb_inL = filter_output;
        float reverb_inR = filter_output;
        float reverb_outL = 0.0f;
        float reverb_outR = 0.0f;
        reverb.Process(reverb_inL, reverb_inR, &reverb_outL, &reverb_outR);

        */
        //*****  CONVERT FLOAT TO INT16  AND INTERLEAVE  *****
        // *buff++ = (int16_t)(sine_sig * 32767);      //RIGHT OUTPUT BUFFER LOCATION
        // *buff++ = (int16_t)(rect_sig * 32767);      //LEFT OUTPUT BUFFER LOCATION 

        *buff++ = (int16_t)((non_filtered_output) * 32767);      //RIGHT OUTPUT BUFFER LOCATION
        *buff++ = (int16_t)((non_filtered_output) * 32767);      //LEFT OUTPUT BUFFER LOCATION 
        
    }
   
   
    //**********************************
    //***  TIMING METRICS CONTINUED  ***
    //**********************************
    if(!accum_dt_lockout)
    {
        //***  accumulate uS deltas
        dt = (timer_hw->timerawl) - t0;
        accum_dt += dt;
        accum_dt_count++;

        //***  save greatest uS delta
        if(max_dt < dt)
        {
            max_dt = dt;
        }
    }

    gpio_put(DEBUG_A, 0);
}

//*************************************************************
//*************************************************************
//*************************************************************
//*************************************************************


void set_oscillator_frequency(float this_freq)
{
    // Set frequency for all oscillators
    sine_osc.SetFreq(this_freq);
    tri_osc.SetFreq(this_freq);
    saw_osc.SetFreq(this_freq);
    square_osc.SetFreq(this_freq);
}


void set_pwm(float dutycycle)
{
    if((dutycycle > 0.01) && (dutycycle < 0.99))
    {
        rect_osc.SetPw(dutycycle);
    }
    

}



//*************************************************************
//*************************************************************
//*************************************************************
//*************************************************************


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void control_val_changed(u8 control_num, u16 val)
{
    float fval = val / 4096.f;  //normalize to 0-1
    
    switch(control_num)
    {
        case 0:
        {
            fval *= 4000;
            set_oscillator_frequency(fval);
        }break;

        case 1:
        {  
            // if((fval > 0.01) && (fval < 0.99))
            // {
            //     rect_osc.SetPw(fval);
            // }
            fval = mapfloat(fval, 0, 1, 0.001, 30);
            
            lfo.SetFreq(fval);

        }break;

        case 2:
        {  
            fval = mapfloat(fval, 0, 1, 0.001, 0.8);
            //m_filter.SetRes(fval);
        {   //fval *= 4000;   
            reverb.SetFeedback(fval);  
        }
            
        }break;
    }
}

