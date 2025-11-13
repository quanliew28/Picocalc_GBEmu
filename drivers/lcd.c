#include <stdio.h>
#include "pico/stdlib.h"
#include "lcd.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

// DMA variables
static int dma_channel;
static dma_channel_config dma_config;

// ---------------------------------------------------------------
// Helper functions to send SPI commands and data to the LCD
// ---------------------------------------------------------------

void lcd_cmd(uint8_t cmd)
{
    gpio_put(LCD_CSX, 0); // select the LCD
    gpio_put(LCD_DCX, 0); // command mode
    spi_write_blocking(LCD_SPI, &cmd, 1);
    gpio_put(LCD_CSX, 1); // deselect the LCD
}

void lcd_data(const uint8_t *data, size_t len)
{
    gpio_put(LCD_CSX, 0); // select the LCD
    gpio_put(LCD_DCX, 1); // data mode
    spi_write_blocking(LCD_SPI, data, len);
    gpio_put(LCD_CSX, 1); // deselect the LCD
}

// ---------------------------------------------------------------
// Configure LCD GPIO
// ---------------------------------------------------------------

static void lcd_spi_init()
{
    spi_init(LCD_SPI, LCD_BAUDRATE);

    // Configure SPI1 pins
    gpio_set_function(LCD_SCL, GPIO_FUNC_SPI);
    gpio_set_function(LCD_SDI, GPIO_FUNC_SPI);
    gpio_set_function(LCD_SDO, GPIO_FUNC_SPI);

    gpio_init(LCD_CSX);
    gpio_init(LCD_DCX);
    gpio_init(LCD_RST);

    // Set GPIO directions
    gpio_set_dir(LCD_CSX, GPIO_OUT);
    gpio_set_dir(LCD_DCX, GPIO_OUT);
    gpio_set_dir(LCD_RST, GPIO_OUT);

    // Initialize GPIO states
    gpio_put(LCD_CSX, 1);
    gpio_put(LCD_DCX, 0); // keep chip selected
    gpio_put(LCD_RST, 1); // reset
}

// ---------------------------------------------------------------
// LCD Initialization and Drawing Functions
// ---------------------------------------------------------------

void lcd_init()
{
    lcd_spi_init();

    // Hardware reset
    gpio_put(LCD_RST, 0);
    sleep_ms(10);
    gpio_put(LCD_RST, 1);
    sleep_ms(120);

    // Software reset
    lcd_cmd(LCD_CMD_SWRESET);
    sleep_ms(5);

    // Sleep out
    lcd_cmd(LCD_CMD_SLPOUT);
    sleep_ms(120);

    // Color mode - 16-bit color
    lcd_cmd(LCD_CMD_COLMOD);
    uint8_t colmod_data = 0x55; // 16-bit/pixel
    lcd_data(&colmod_data, 1);

    // Memory access control
    lcd_cmd(LCD_CMD_MADCTL);
    uint8_t madctl_data = 0x08; // adjust as needed for orientation
    lcd_data(&madctl_data, 1);

    // Display on
    lcd_cmd(LCD_CMD_DISPON);

    // Inversion on
    lcd_cmd(LCD_CMD_INVON);

}

// Set the drawing window on the LCD screen
// void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
// {
//     // Column address set
//     lcd_cmd(LCD_CMD_CASET);
//     uint8_t caset_data[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
//     lcd_data(caset_data, 4);

//     // Row address set
//     lcd_cmd(LCD_CMD_RASET);
//     uint8_t raset_data[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
//     lcd_data(raset_data, 4);

//     // Memory write
//     lcd_cmd(LCD_CMD_RAMWR);
// }

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    gpio_put(LCD_CSX, 0); // Select LCD once for all commands
    
    // Column address set
    gpio_put(LCD_DCX, 0); // Command mode
    uint8_t caset_cmd = LCD_CMD_CASET;
    spi_write_blocking(LCD_SPI, &caset_cmd, 1);
    
    gpio_put(LCD_DCX, 1); // Data mode
    uint8_t caset_data[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    spi_write_blocking(LCD_SPI, caset_data, 4);

    // Row address set
    gpio_put(LCD_DCX, 0); // Command mode
    uint8_t raset_cmd = LCD_CMD_RASET;
    spi_write_blocking(LCD_SPI, &raset_cmd, 1);
    
    gpio_put(LCD_DCX, 1); // Data mode
    uint8_t raset_data[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
    spi_write_blocking(LCD_SPI, raset_data, 4);

    // Memory write command
    gpio_put(LCD_DCX, 0); // Command mode
    uint8_t ramwr_cmd = LCD_CMD_RAMWR;
    spi_write_blocking(LCD_SPI, &ramwr_cmd, 1);
    
    gpio_put(LCD_DCX, 1); // Data mode (leave in data mode for DMA)
    // Note: Keep CS low, caller will send pixel data via DMA
    // gpio_put(LCD_CSX, 1); // DON'T deselect yet
}

//----------------------------------------------------------------
// DMA Initialization and usage functions
// ---------------------------------------------------------------

void lcd_dma_init()
{
    dma_channel = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, spi_get_dreq(LCD_SPI, true));
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);

    dma_channel_configure(
        dma_channel,
        &dma_config,
        &spi_get_hw(LCD_SPI)->dr,
        NULL,
        0,
        false);
}

void lcd_data_dma8(const uint8_t *data, size_t len)
{
    spi_set_format(LCD_SPI, 8, 0, 0, SPI_MSB_FIRST);
    gpio_put(LCD_CSX, 0);
    gpio_put(LCD_DCX, 1);

    dma_channel_set_read_addr(dma_channel, data, false);
    dma_channel_set_trans_count(dma_channel, len, true);
    dma_channel_wait_for_finish_blocking(dma_channel);

    gpio_put(LCD_CSX, 1);
    spi_set_format(LCD_SPI, 8, 0, 0, SPI_MSB_FIRST);
}

void lcd_data_dma16(const uint16_t *data, size_t len)
{
    if (len == 0 || data == NULL) return;

    // Ensure SPI is in 16-bit mode for half-word writes (MSB first)
    spi_set_format(LCD_SPI, 16, 0, 0, SPI_MSB_FIRST);

    gpio_put(LCD_DCX, 1); // Data
    gpio_put(LCD_CSX, 0);

    // Start DMA: set source address and transfer count (half-words)
    dma_channel_set_read_addr(dma_channel, data, false);
    dma_channel_set_trans_count(dma_channel, len, true);

    // Wait for DMA completion (blocking)
    dma_channel_wait_for_finish_blocking(dma_channel);

    gpio_put(LCD_CSX, 1);

    // Restore SPI to 8-bit mode (caller code expects this)
    spi_set_format(LCD_SPI, 8, 0, 0, SPI_MSB_FIRST);
}

void lcd_dma_write_line(const uint16_t *pixels, int x, int y, int width)
{
    if (!pixels || width <= 0) return;
    
    // Set LCD address window for the line
    //lcd_set_window(x, y, width, 1);
    
    // DMA transfer the RGB565 pixel data
    lcd_data_dma16(pixels, width);
}

// Write multiple lines using DMA
void lcd_dma_write_rect(const uint16_t *pixels, int x, int y, int width, int height)
{
    if (!pixels || width <= 0 || height <= 0) return;
    
    // Set LCD address window for the rectangle
    //lcd_set_window(x, y, width, height);
    
    // DMA transfer all pixel data at once
    lcd_data_dma16(pixels, width * height);
}

// simple gradient + checker pattern test (RGB565)
void test_dma_display_once(void) {
    // small aligned buffer for one 320x32 block (adjust size as needed)
    static uint16_t buf[320 * 32] __attribute__((aligned(4)));

    // Fill with vertical gradient + checker
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 320; ++x) {
            // create visible pattern: three palettes blended
            uint16_t c;
            if (((x/16) ^ (y/8)) & 1) {
                // use green-ish tone
                c = (uint16_t)0x7E0; // RGB565 green
            } else {
                // blue-ish tone
                c = (uint16_t)0x001F; // RGB565 blue
            }
            // add vertical gradient by mixing with gray
            int fade = (y * 31) / 31;
            if (fade & 1) c ^= 0x420; // small variation for visibility
            buf[y * 320 + x] = c;
        }
    }

    // Set window where to draw (x=0,y=16,width=320,height=32)
    lcd_set_window(0, 16, 320, 32);

    // Transfer via DMA (len = pixels)
    lcd_data_dma16(buf, 320 * 32);

    // Optionally wait a bit so you can visually inspect
    sleep_ms(500);
}