#include <stdio.h>
#include <bsp/board.h>

#include <hardware/clocks.h>
#include "hardware/dma.h"
#include "hardware/irq.h"
#include <hardware/gpio.h>
#include "pico/stdlib.h"

#include "tx16.pio.h"

#define WAVETABLE_LENGTH 5648

uint16_t wavetable[WAVETABLE_LENGTH];
int dma_chan;

void wavetable_write_bit(int port, uint16_t *bitoffset, uint8_t value)
{
    if (!value)
    {
        (*bitoffset)++;
        return;
    }
    wavetable[(*bitoffset)++] |= (1 << port);
}

void wavetable_write_byte(int port, uint16_t *bitoffset, uint8_t value)
{
    wavetable_write_bit(port, bitoffset, 0);

    wavetable_write_bit(port, bitoffset, (value >> 0) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 1) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 2) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 3) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 4) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 5) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 6) & 0x01);
    wavetable_write_bit(port, bitoffset, (value >> 7) & 0x01);

    wavetable_write_bit(port, bitoffset, 1);
    wavetable_write_bit(port, bitoffset, 1);
}

void dma_handler()
{
    static uint8_t universe;
    static uint16_t bitoffset;
    static uint16_t chan;
    static uint8_t buffer[16][512];

    memset(wavetable, 0x00, WAVETABLE_LENGTH * sizeof(uint16_t));

    for (universe = 0; universe < 16; universe++)
    {
        bitoffset = 0;

        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);

        wavetable_write_byte(universe, &bitoffset, 0);

        for (chan = 0; chan < 512; chan++)
        {

            wavetable_write_bit(universe, &bitoffset, buffer[universe][chan]);
        }

        wavetable_write_bit(universe, &bitoffset, 0);
    }
    dma_hw->ints0 = 1u << dma_chan;

    dma_channel_set_read_addr(dma_chan, wavetable, true);
}

int main()
{

    uint offset = pio_add_program(pio0, &tx16_program);
    float div = (float)clock_get_hz(clk_sys) / 250000;
    tx16_program_init(pio0, 0, offset, div);

    dma_chan = dma_claim_unused_channel(true); // dma_chan_0_0=dma_chan
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true); // not increment read address;
    channel_config_set_dreq(&c, DREQ_PIO0_TX0);  // dma transfer data over pio0

    dma_channel_configure(
        dma_chan, // modify dma channel
        &c,
        &pio0_hw->txf[0], // tell to dma write to transfer register pio0, state machine 0
        NULL,
        WAVETABLE_LENGTH / 2,
        false);

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_handler();

    while (true)
    {
        tight_loop_contents();
    }
}







