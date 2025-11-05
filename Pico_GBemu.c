#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/lcd.h"
#include "drivers/font.h"
#include "lib/peanut_gb.h"
#include "romdata.c"  // Include your ROM data here

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
    RGB(224, 248, 208),  // white
    RGB(136, 192, 112),  // light gray
    RGB(52, 104, 86),    // dark gray
    RGB(8, 24, 32)       // black
};

//----------------------------------------------------
// 3. Display callback for Peanut-GB
//----------------------------------------------------
static void lcd_draw_line(struct gb_s *gb,
                          const uint8_t pixels[160],
                          const uint_fast8_t line)
{
    // Convert 2-bit pixel values (0–3) to RGB565
    for (int x = 0; x < 160; x++)
        linebuf[x] = gb_palette[pixels[x] & 3];

    // Optional: Scale to 320 width (2x) for PicoCalc display   
    uint16_t scaled_linebuf[320];
    for (int i = 0; i < 160; i++) {
        scaled_linebuf[i * 2]     = linebuf[i];
        scaled_linebuf[i * 2 + 1] = linebuf[i];
    }


    // Center the GB image on the PicoCalc (320x320) display
    const int x_offset = (320 - 320) / 2;   
    const int y_offset = (320 - 288) / 2;  

    int scaled_y = y_offset + (line * 2);


    // Draw the line twice (for 2x vertical scaling)
    lcd_blit(scaled_linebuf, x_offset, scaled_y, 320, 1);
    lcd_blit(scaled_linebuf, x_offset, scaled_y + 1, 320, 1);
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
    lcd_clear_screen();

    // Initialize emulator
    gb_init(&gb, gb_rom_read, gb_cart_ram_read, gb_cart_ram_write, gb_error, NULL);
    gb.display.lcd_draw_line = lcd_draw_line;

    // Reset Game Boy
    gb_reset(&gb);

    // Main loop: run emulator frames and display them
    while (true) {
        gb_run_frame(&gb);    // emulate one frame (Peanut-GB function)
        tight_loop_contents(); // yield; could add button polling here
    }

    return 0;
}