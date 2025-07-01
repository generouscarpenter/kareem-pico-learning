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
using namespace daisysp;

alignas(32) int16_t buffer_0[I2S_BLOCK_SIZE * 2];
alignas(32) int16_t buffer_1[I2S_BLOCK_SIZE * 2];



Oscillator sine_osc;
Oscillator tri_osc;
Oscillator saw_osc;
Oscillator square_osc;

static Tone lpf;

static Overdrive overdrive;
static ReverbSc reverb;

#define START_FREQ 80.f


u32 audio_interrupt_count;

//********************************************
//****  INIT AUDIO DEVICES
//********************************************

void init_audio_code(void)
{
    // initialize each oscillator:
    sine_osc.Init(SAMPLE_RATE);
    sine_osc.SetWaveform(sine_osc.WAVE_SIN);
    sine_osc.SetFreq(START_FREQ);
    sine_osc.SetAmp(0.5);

/*
    tri_osc.Init(SAMPLE_RATE);
    tri_osc.SetWaveform(square_osc.WAVE_TRI);
    tri_osc.SetFreq(START_FREQ);
    tri_osc.SetAmp(0.5);


    saw_osc.Init(SAMPLE_RATE);
    saw_osc.SetWaveform(square_osc.WAVE_SAW);
    saw_osc.SetFreq(START_FREQ);
    saw_osc.SetAmp(0.5);


    square_osc.Init(SAMPLE_RATE);
    square_osc.SetWaveform(square_osc.WAVE_SQUARE);
    square_osc.SetFreq(START_FREQ);
    square_osc.SetAmp(0.5);

*/
    //init filter
    lpf.Init(SAMPLE_RATE);
    lpf.SetFreq(0.1);

    
    // initialize effects
    reverb.Init(SAMPLE_RATE);

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
        //***  CALCULATE OSCILLATOR VALUES ONE AT A TIME  ***
        float sine_sig = sine_osc.Process();

        //RUN OSC THRU FILTER
        float filter_output = lpf.Process(sine_sig);

        //RUN FILTER THRU REVERB
        float reverb_inL = filter_output;
        float reverb_inR = filter_output;
        float reverb_outL = 0.0f;
        float reverb_outR = 0.0f;
        reverb.Process(reverb_inL, reverb_inR, &reverb_outL, &reverb_outR);

        //*****  CONVERT FLOAT TO INT16  AND INTERLEAVE  *****
        // *buff++ = (int16_t)(sine_sig * 32767);      //RIGHT OUTPUT BUFFER LOCATION
        // *buff++ = (int16_t)(rect_sig * 32767);      //LEFT OUTPUT BUFFER LOCATION 

        *buff++ = (int16_t)((reverb_outR) * 32767);      //RIGHT OUTPUT BUFFER LOCATION
        *buff++ = (int16_t)((reverb_outL) * 32767);      //LEFT OUTPUT BUFFER LOCATION 
        
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
    sine_osc.SetFreq(this_freq);
    //rect_osc.SetFreq(this_freq);
}


void set_cutoff_freq(float cutoff_freq)
{
    lpf.SetFreq(cutoff_freq);
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
            sine_osc.SetFreq(fval);
        }break;

        case 1:
        {  
            fval = mapfloat(fval, 0, 1, 20, 20000);
            lpf.SetFreq(fval);
        }break;

        case 2:
        {  
            fval = mapfloat(fval, 0, 1, 0.001, 0.8);
            //m_filter.SetRes(fval);
              //fval *= 4000;   
            reverb.SetFeedback(fval);  
            
        }break;
    }
}