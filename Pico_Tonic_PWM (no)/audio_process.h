


#ifndef _AUDIO_PROCESS_H_
#define _AUDIO_PROCESS_H_ 1

#include "main.h"

#ifdef __cplusplus
extern "C" 
{
#endif




typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


#define I2S_BLOCK_SIZE       240  
#define BLOCK_SIZE         I2S_BLOCK_SIZE
#define SAMPLE_RATE     48000
#define CALLBACK_HERTZ  1000
#define MAX_IN_OUT_CHANNELS  8


extern u32 accum_dt, ave_dt, accum_dt_count;
extern u32 dt;
extern u32 max_dt;
extern bool accum_dt_lockout;
extern int16_t buffer_0[I2S_BLOCK_SIZE * 2];
extern int16_t buffer_1[I2S_BLOCK_SIZE * 2];


void init_audio_code(void);
extern float audio_in_0[BLOCK_SIZE];
extern float audio_in_1[BLOCK_SIZE];

extern float audio_out_0[BLOCK_SIZE];
extern float audio_out_1[BLOCK_SIZE];


extern s16 audio_out_combined[BLOCK_SIZE * 2];

extern u32 active_in_buffer;
extern u32 active_out_buffer;

extern void process_audio(void);

extern u32 audio_interrupt_count;

void set_oscillator_frequency(float this_freq);


#ifdef __cplusplus
}
#endif



#endif 

