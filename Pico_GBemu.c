#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/lcd.h"
#include "pico/multicore.h"

#include "lib/peanut_gb.h"
#include "romdata.c" // Include your ROM data here

// Game State
static volatile bool emulator_paused = false;
static uint32_t last_key_time = 0;

// Constant
static const uint32_t KEY_RELEASE_DELAY_MS = 5;

//----------------------------------------------------
// 1. Game Boy ROM data
//----------------------------------------------------
// For a quick test, you can include a small public-domain ROM as an array
// or link it externally. Here we just declare it.
extern const unsigned char rom_data[];
extern const size_t rom_size;

//----------------------------------------------------
// 2. Peanut-GB configuration
//----------------------------------------------------
static struct gb_s gb;

// 160 RGB565 pixels per line buffer
static uint16_t linebuf[160];

// Simple 4-shade Game Boy palette (light → dark)
static const uint16_t gb_palette[4] = {
    RGB(224, 248, 208), // white
    RGB(136, 192, 112), // light gray
    RGB(52, 104, 86),   // dark gray
    RGB(8, 24, 32)      // black
};

//----------------------------------------------------
// 3. Display callback for Peanut-GB
//----------------------------------------------------

static uint16_t framebuffer[320 * 288] __attribute__((aligned(4))); // Full frame buffer, aligned for DMA
static volatile bool frame_ready = false;

static void lcd_draw_line(struct gb_s *gb,
                          const uint8_t pixels[160],
                          const uint_fast8_t line)
{
    // Convert 2-bit pixel values (0–3) to RGB565
    for (int x = 0; x < 160; x++)
        linebuf[x] = gb_palette[pixels[x] & 3];

    // Scale to 320 width (2x) and write to TWO vertical lines (2x vertical scale)
    int y0 = line * 2;      // First scanline
    int y1 = line * 2 + 1;  // Second scanline (duplicate)

    // Guard bounds
    if (y0 >= 288 || y1 >= 288) return;

    // Write first line
    for (int i = 0; i < 160; i++)
    {
        framebuffer[y0 * 320 + i * 2]     = linebuf[159-i];
        framebuffer[y0 * 320 + i * 2 + 1] = linebuf[159-i];
    }

    // Write second line (duplicate)
    for (int i = 0; i < 160; i++)
    {
        framebuffer[y1 * 320 + i * 2]     = linebuf[159-i];
        framebuffer[y1 * 320 + i * 2 + 1] = linebuf[159-i];
    }

    // Mark frame ready when last GB scanline (143) is drawn
    if (line == 143) {
        frame_ready = true;
    }
}
//----------------------------------------------------
// 4. Memory read/write stubs (required by Peanut)
//----------------------------------------------------
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
    return rom_data[addr % rom_size];
}

void gb_cart_ram_write(struct gb_s *gb,
                       const uint_fast32_t addr,
                       const uint8_t val)
{
    // stub – no external RAM
}
uint8_t gb_cart_ram_read(struct gb_s *gb,
                         const uint_fast32_t addr)
{
    return 0xFF;
}

// ---------------------------------------------------------------------------
// Error reporting
// ---------------------------------------------------------------------------
void gb_error(struct gb_s *gb, const enum gb_error_e err, const uint16_t addr)
{
    fprintf(stderr, "Emu error %d at 0x%04X\n", err, addr);
    exit(EXIT_FAILURE);
}

//----------------------------------------------------
// 5. Main
//----------------------------------------------------
int main(void)
{
    stdio_init_all();
    lcd_init();
    lcd_dma_init();
    //lcd_set_window(0, 16, 320, 288);

    
    // Initialize emulator
    gb_init(&gb, gb_rom_read, gb_cart_ram_read, gb_cart_ram_write, gb_error, NULL);
    gb.display.lcd_draw_line = lcd_draw_line;

    // Set initial joypad state
    gb.direct.joypad = 0xFF;

    // Reset Game Boy
    gb_reset(&gb);

    // Main loop: run emulator frames and display them
    while (true)
    {

        // Run emulator frame
        if (!emulator_paused)
        {
            //lcd_dma_write_rect(framebuffer, 0, 0, 320, 288);
            // test_dma_display_once();
            gb_run_frame(&gb);
        }
        if (frame_ready)
        {
            // Set window immediately before DMA transfer
            lcd_set_window(0, 16, 320, 320);
            
            // Transfer the complete frame via DMA
            lcd_dma_write_rect(framebuffer, 0, 16, 320, 288);
            
            // Clear the flag
            frame_ready = false;
        }
    }

    return 0;
}