/**
 *  V. Hunter Adams (vha3)
    Code based on examples from Raspberry Pi Co

    Sets up two DMA channels. One sends samples at audio rate to the DAC,
    (data_chan), and the other writes to the data_chan DMA control registers (ctrl_chan).

    The control channel writes to the data channel, sending one period of
    a sine wave thru the DAC. The control channel is chained to the data
    channel, so it is re-triggered after the data channel is finished. The control
    channel then rewrites and restarts the data channel, etc.

    NOTE: in order to configure the data channel read addresses to 
    wrap properly, the DAC data buffer must be naturally aligned in memory.
    My hacky solution was to generate a few extra periods of the sine wave,
    and then find an index which was naturally aligned.

    The better solution, implemented next, was to use the ((aligned())) attribute

    Configuring the data DMA channel to the DAC to be triggered off an audio-rate
    timer required a function that changes the value of a DMA control register.
    The SDK didn't have any functions that changed this particular register, but
    it contained others upon which I could model the one below. The documentation
    for the RP2040 is written well enough that this wasn't too troublesome.

 */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "image.h"

// Number of samples per period in sine table
#define image_size 115200

// A-channel, 1x, active
// #define DAC_config_chan_A 0b0011000000000000

//SPI configurations
#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  6
#define PIN_MOSI 7
#define SPI_PORT spi1

// Number of DMA transfers per event
const uint32_t transfer_count = image_size ;

/*! Added by Hunter
    Modifies the TIMER0 register of the dma channel
 */
// static inline void dma_channel_set_timer0(uint32_t timerval) {
//     dma_hw->timer[0] = timerval;
// }


int main() {
    stdio_init_all();

    // Enable SPI at 1 MHz and connect to GPIOs
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    // Initialize SPI channel (channel, baud rate set to 20MHz)
    spi_init(SPI_PORT, 20000000) ;
    // Format (channel, data bits per transfer, polarity, phase, order)
    spi_set_format(SPI_PORT, 8, 0, 0, 0);

    // Map SPI signals to GPIO ports, acts like framed SPI with this CS mapping
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI) ;
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // // Build sine table
    // int i ;
    // for (i=0; i<(image_size); i++){
    //     raw_sin[i] = (int)(2047 * sin((float)i*6.283/(float)image_size) + 2047); //12 bit
    //     DAC_data[i] = DAC_config_chan_A | (raw_sin[i] & 0x0fff) ;
    // }


    // Get a free channel, panic() if there are none
    int data_chan = dma_claim_unused_channel(true);
    // int ctrl_chan = dma_claim_unused_channel(true);

    // // Setup the control channel
    // dma_channel_config c = dma_channel_get_default_config(ctrl_chan); // default configs
    // channel_config_set_transfer_data_size(&c, DMA_SIZE_32); // 32-bit txfers
    // channel_config_set_read_increment(&c, false); // no read incrementing
    // channel_config_set_write_increment(&c, false); // no write incrementing

    // dma_channel_configure(
    //     ctrl_chan,
    //     &c,
    //     &dma_hw->ch[data_chan].al1_transfer_count_trig, // txfer to transfer count trigger
    //     &transfer_count,
    //     1,
    //     false
    // );

    // // Confirm memory alignment
    // printf("\n\nBeginning: %x", &DAC_data[0]);
    // printf("\nFirst: %x", &DAC_data[1]);
    // printf("\nSecond: %x", &DAC_data[2]);

    // 16 bit transfers. Read address increments after each transfer.
    // DREQ to Timer 0 is selected, so the DMA is throttled to audio rate
    dma_channel_config c2 = dma_channel_get_default_config(data_chan);
    // 8 bit transfers
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_8);
    // increment the read adddress, don't increment write address
    channel_config_set_read_increment(&c2, true);
    channel_config_set_write_increment(&c2, false);
    // // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
    // // sys_clk is 125 MHz unless changed in code
    // dma_channel_set_timer0(0x0017ffff) ;
    // 0x3b means timer0 (see SDK manual)
    channel_config_set_dreq(&c2, spi_get_dreq(SPI_PORT, false));
    // // chain to the controller DMA channel
    // channel_config_set_chain_to(&c2, ctrl_chan);
    // // set wrap boundary. This is why we needed alignment!
    // channel_config_set_ring(&c2, false, 9); // 1 << 9 byte boundary on read ptr


    dma_channel_configure(
        data_chan,          // Channel to be configured
        &c2,            // The configuration we just created
        &spi_get_hw(SPI_PORT)->dr, // write address
        img_arr, // The initial read address (AT NATURAL ALIGNMENT POINT)
        image_size, // Number of transfers; in this case each is 1 byte.
        false           // Don't start immediately.
    );


    // start the control channel
    dma_start_channel_mask(1u << data_chan) ;

    // Exit main.
    // No code executing!!

}