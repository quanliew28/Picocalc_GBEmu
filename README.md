# Pico Game Boy Emulator

A Game Boy emulator running on the Raspberry Pi Pico 2, featuring an ST7789P LCD display and the Peanut-GB emulation core. 

![Pico Game Boy Emulator](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico%202-brightgreen)
![License](https://img.shields.io/badge/License-MIT-blue)
![Status](https://img.shields.io/badge/Status-Active%20Development-orange)

<img src="https://github.com/quanliew28/Picocalc_GBEmu/blob/main/assets/picocalc_gbemu.jpeg?raw=true" width="200"/>


## 🎮 Features

- **Full Game Boy emulation** using the lightweight Peanut-GB core
- **ST7789P LCD support** with 320x320 pixel display
- **Hardware SPI** for fast display updates at 75MHz
- **ROM conversion utility** to embed Game Boy ROMs as C arrays
- **Centered Game Boy display** (160x144 pixels) on the larger LCD
- **4-shade monochrome palette** with authentic Game Boy color reproduction

## 🛠️ Hardware Requirements

- **Raspberry Pi Pico 2** (RP2350 microcontroller)
- **ST7789P LCD Display** (320x320 pixels)
- **SPI Connection** for LCD communication

### Hardware Connections

| Pico Pin | LCD Pin | Description |
|----------|---------|-------------|
| GPIO 10  | SCL     | SPI Clock |
| GPIO 11  | SDI     | SPI Data In (MOSI) |
| GPIO 12  | SDO     | SPI Data Out (MISO) |
| GPIO 13  | CSX     | Chip Select |
| GPIO 14  | DCX     | Data/Command |
| GPIO 15  | RST     | Reset |

## 🚀 Getting Started

### Prerequisites

- Raspberry Pi Pico SDK 2.2.0 or later
- CMake 3.13 or later
- ARM GCC toolchain
- VS Code with Raspberry Pi Pico extension (recommended)

### Building the Project

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd Picocalc_GBEmu
   ```

2. **Prepare your Game Boy ROM**
   - Place your Game Boy ROM file (e.g., `tetris.gb`) in the project directory
   - The ROM must be legally owned by you

3. **Convert ROM to C array**
   ```bash
   # Compile the ROM conversion utility
   gcc c2rom.c -o c2rom
   
   # Convert your ROM to a C file
   ./c2rom tetris.gb romdata.c
   ```

4. **Build the emulator**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

5. **Flash to Pico**
   - Connect your Pico 2 while holding the BOOTSEL button
   - Copy `Pico_GBemu.uf2` to the Pico's mass storage device

## 📁 Project Structure

```
Pico_GBemu/
├── Pico_GBemu.c          # Main emulator code
├── CMakeLists.txt        # Build configuration
├── c2rom.c               # ROM-to-C converter utility
├── romdata.c             # Generated ROM data (created by c2rom)
├── drivers/              # Hardware drivers
│   ├── lcd.c            # ST7789P LCD driver
│   ├── lcd.h            # LCD driver header
│   ├── font.h           # Font definitions
│   ├── font-5x10.c      # 5x10 font data
│   └── font-8x10.c      # 8x10 font data
├── lib/
│   └── peanut_gb.h      # Peanut-GB emulator core
└── build/               # Build output directory
```

## ⚙️ Configuration

### Display Settings

The emulator automatically centers the Game Boy's 160x144 pixel display on the 320x320 LCD with fullscreen scaling.


### Color Palette

The authentic Game Boy 4-shade palette is defined as:

```c
static const uint16_t gb_palette[4] = {
    RGB(224, 248, 208),  // White (lightest)
    RGB(136, 192, 112),  // Light gray
    RGB(52, 104, 86),    // Dark gray
    RGB(8, 24, 32)       // Black (darkest)
};
```

### LCD Performance

- **SPI Clock**: 75 MHz (optimized for ST7789P)
- **Display Update**: Line-by-line rendering
- **Color Format**: RGB565 (16-bit color)

## 🎯 Usage

Once flashed and powered on, the emulator will:

1. Initialize the LCD display
2. Set up the Peanut-GB emulator core
3. Load the embedded ROM data
4. Start emulation and display output

The Game Boy screen will appear centered on the LCD display with the classic green-tinted monochrome palette.

## 🔧 Development Tasks

The project includes pre-configured VS Code tasks:

- **Compile Project**: Build the emulator (`Ctrl+Shift+P` → "Tasks: Run Task" → "Compile Project")
- **Run Project**: Flash the binary to Pico via picotool
- **Flash**: Program via OpenOCD and debug probe
- **Rescue Reset**: Recovery mode for troubleshooting

## 📝 Technical Details

### Emulation Core

The project uses **Peanut-GB**, a lightweight Game Boy emulator core that provides:
- CPU emulation (Sharp LR35902)
- Memory management
- PPU (Picture Processing Unit) emulation
- Timer and interrupt handling

### Memory Management

- **ROM Access**: Direct read from embedded C array
- **RAM**: Stubbed out (no save game support currently)
- **Display Buffer**: Line-based rendering to minimize memory usage

### Performance

- **Frame Rate**: Targets original Game Boy timing (≈59.7 FPS)
- **CPU Usage**: Optimized for RP2350's dual Cortex-M33 cores
- **Memory**: Minimal RAM footprint with efficient display buffering

## 🐛 Troubleshooting

### Common Issues

1. **Display not working**
   - Check SPI connections
   - Verify LCD power supply
   - Ensure correct pin assignments in `lcd.h`

2. **ROM not loading**
   - Verify `romdata.c` is generated correctly
   - Check ROM file size and format
   - Ensure ROM is included in build

3. **Build errors**
   - Update Pico SDK to version 2.2.0+
   - Check CMake configuration
   - Verify toolchain installation

### Debug Output

The emulator provides debug output via USB serial:
```c
pico_enable_stdio_uart(Pico_GBemu 1)
pico_enable_stdio_usb(Pico_GBemu 1)
```

## 🤝 Contributing

Contributions are welcome! Areas for improvement:

- Input handling (buttons/controls)
- Audio emulation
- Save state functionality
- Multiple ROM support
- Performance optimizations
- Additional display options

## 📜 License

This project is licensed under the MIT License. See individual source files for specific license information.

### Third-Party Components

- **Peanut-GB**: MIT License (Copyright © 2018-2023 Mahyar Koshkouei)
- **Pico SDK**: BSD 3-Clause License
- **SameBoy components**: MIT License (Copyright © 2015-2019 Lior Halphon)

## 🙏 Acknowledgments

- **Mahyar Koshkouei** for the excellent Peanut-GB emulator core
- **Raspberry Pi Foundation** for the Pico platform and SDK
- **Game Boy development community** for documentation and resources

## 📞 Support

For issues, questions, or contributions:
- Open an issue on the project repository
- Check the Raspberry Pi Pico community forums
- Review the Pico SDK documentation

---

**Note**: This emulator is for educational and personal use only. Ensure you own the rights to any ROM files you use with this emulator.# Picocalc_GBEmu
