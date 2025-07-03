/* sound_i2s.c */

#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "sound_i2s.h"
#include "sound_i2s_8bits.pio.h"
#include "sound_i2s_16bits.pio.h"

#include "math.h"
#include "audio_process.h"
#include "main.h"

volatile unsigned int sound_i2s_num_buffers_played = 0;

static struct sound_i2s_config config;
static PIO sound_pio;
static uint sound_pio_sm;
static uint sound_dma_chan;

static volatile int sound_cur_buffer_num;
static int16_t *sound_sample_buffers[2] = {buffer_0, buffer_1};

int cur_buff = 0;
int resting_buff = 1;

static void __isr __time_critical_func(dma_handler)(void)
{
    // swap buffers
    gpio_put(DEBUG_A, 1);
    switch (cur_buff)
    {
    case 0:
        cur_buff = 1;
        resting_buff = 0;
        break;

    case 1:
        cur_buff = 0;
        resting_buff = 1;
        break;
    }

    // sound_i2s_num_buffers_played++;

    // set dma dest to new buffer and re-trigger dma:
    dma_hw->ch[sound_dma_chan].al3_read_addr_trig = (uintptr_t)sound_sample_buffers[cur_buff];

    // ack dma irq
    dma_hw->ints0 = 1u << sound_dma_chan;

    process_audio();
    //core_1_trigger_process = true;
}

int sound_i2s_init(const struct sound_i2s_config *cfg)
{
    config = *cfg;

    // allocate sound buffers
    // size_t sound_buffer_size = (config.bits_per_sample/8) * 2 * SOUND_I2S_BUFFER_NUM_SAMPLES;
    //   sound_sample_buffers[0] = malloc(sound_buffer_size);
    //   sound_sample_buffers[1] = malloc(sound_buffer_size);
    //   if (! sound_sample_buffers[0] || ! sound_sample_buffers[1])
    //   {
    //     free(sound_sample_buffers[0]);
    //     free(sound_sample_buffers[1]);
    //     return -1;
    //   }

    // sound_sample_buffers[0] = &buffer_0;
    // sound_sample_buffers[1] = &buffer_1;
    memset(sound_sample_buffers[0], 0, sizeof(buffer_0));
    memset(sound_sample_buffers[1], 0, sizeof(buffer_0));

    //******************************************
    //******  PRELOAD SINE TABLES  *************
    //******************************************
    /*
    uint16_t * buff_1 = sound_i2s_get_buffer(0);
    uint16_t * buff_2 = sound_i2s_get_buffer(1);
    for (int i = 0; i < sound_buffer_size; ++i)
    {
        int16_t sample = (int16_t)(sin(2.0 * M_PI * i / 256.0) * 32767);
        *buff_1++ = sample;
        *buff_1++ = sample;         //load same sample left and then right

        *buff_2++ = sample;         //and also in the other buffer
        *buff_2++ = sample;
    }   */

    // setup pio
    sound_pio = (config.pio_num == 0) ? pio0 : pio1;
    uint offset = pio_add_program(sound_pio, (config.bits_per_sample == 8) ? &sound_i2s_8bits_program : &sound_i2s_16bits_program);
    sound_pio_sm = pio_claim_unused_sm(sound_pio, true);
    if (config.bits_per_sample == 8)
    {
        sound_i2s_8bits_program_init(sound_pio, sound_pio_sm, offset, config.sample_rate, config.pin_sda, config.pin_scl);
    }
    else
    {
        sound_i2s_16bits_program_init(sound_pio, sound_pio_sm, offset, config.sample_rate, config.pin_sda, config.pin_scl);
    }

    // allocate dma channel and setup irq
    sound_dma_chan = dma_claim_unused_channel(true);
    dma_channel_set_irq0_enabled(sound_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_priority(DMA_IRQ_0, 0xff);
    irq_set_enabled(DMA_IRQ_0, true);
    return 0;
}

void sound_i2s_playback_start(void)
{
    // reset buffer
    sound_i2s_num_buffers_played = 0;

    void *buffer = sound_sample_buffers[cur_buff];

    // start pio
    pio_sm_set_enabled(sound_pio, sound_pio_sm, true);

    // setup dma channel
    dma_channel_config dma_cfg = dma_channel_get_default_config(sound_dma_chan);
    channel_config_set_transfer_data_size(&dma_cfg,  DMA_SIZE_32);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(sound_pio, sound_pio_sm, true));
    dma_channel_configure(sound_dma_chan, &dma_cfg,
                          &sound_pio->txf[sound_pio_sm], // destination
                          buffer,                        // source
                          SOUND_I2S_BUFFER_NUM_SAMPLES,  // number of dma transfers
                          true                           // start immediatelly (will be blocked by pio)
    );




}

int16_t *sound_i2s_get_next_buffer(void)
{
    return sound_sample_buffers[resting_buff];
}

int16_t *sound_i2s_get_buffer(int buffer_num)
{
    return sound_sample_buffers[buffer_num];
}
