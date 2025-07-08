# gameboy-emulator
A pure C (C with class, if you may say) rework of https://github.com/xiongnemo/nekomimi-gameboy-emulator-1 (https://github.com/DarkKowalski/nekomimi-gameboy-emulator)

## Status

### CPU

* Full CPU Emulation: working.
* Interrupts: working.

### Cartridge (MBC)

* ROM switching: working

* External RAM: implemented, but seems not implemented correctly. Exist in the code but not used (Refer to `mmu.c`).

* MBC Implemented:
  * MBC1 (Address lines may not be correct)
  * MBC2 (Not tested)
  * MBC3
  * MBC5 (Not tested)
  * Rumble Carts

### PPU

* PPU (Scanline): working.
* PPU (FIFO): Can't render windows, will fix.

### Joypad

* Joypad: working.

### RAM

* RAM: working.

### APU

* APU exists (callback driven) but the speed is roughly 2 to 6 times as fast as the emulation speed.

### Screenshots

#### CPU Instructions

![CPU Instructions](./screenshots/cpu_instr.bmp)

#### The Legend of Zelda: Link's Awakening

![Zelda](./screenshots/Zelda.bmp)

#### Kirby's Dream Land (USA)

![Kirby's Dream Land (USA)](./screenshots/Kirby.png)

#### Kirby's Dream Land 2 (USA)

![Kirby's Dream Land 2 (USA)](./screenshots/Kirby2.bmp)

#### Super Mario Land 2 - 6 Golden Coins (USA) (Rev-B)

![Super Mario Land 2 - 6 Golden Coins (USA) (Rev-B)](./screenshots/MARIOLAND2.png)

![Super Mario Land 2 - 6 Golden Coins (USA) (Rev-B)](./screenshots/MARIOLAND2.bmp)

#### Pokemon Blue (MBC3)

![Pokemon Blue](./screenshots/pokemon_blue.bmp)

![Pokemon Blue](./screenshots/pokemon_blue.png)

![Pokemon Blue](./screenshots/pokemon_blue_1.bmp)

![Pokemon Blue](./screenshots/pokemon_blue_2.bmp)

#### Harvest Moon GB

![Harvest Moon GB](./screenshots/harvest_moon.png)

![Harvest Moon GB](./screenshots/harvest_moon.bmp)

## Development

### Prerequisite

* gcc: Some macros is GNU extension.

```pwsh
nemo @ nemo-g15-5511 in ~\Documents\GitHub\gameboy-emulator on git:dev x [00:00:00]
$ gcc --version
gcc.exe (x86_64-posix-seh-rev0, Built by MinGW-Builds project) 15.1.0
Copyright (C) 2025 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

* SDL3: You should have SDL3 installed or accessible in your system.
  
To use the provided Makefile, the folder structure (on Windows) should be as follows:
```
├─reference
│  └─...
├─SDL3
│  ├─bin
│  ├─include
│  │  └─SDL3
│  ├─lib
│  │  ├─cmake
│  │  │  └─SDL3
│  │  └─pkgconfig
│  └─share
│      └─licenses
│          └─SDL3
├─src
└─test
```

### Example configuration for VS Code

- Install C/C++ extension
- Setup SDL3 include path in `c_cpp_properties.json`

### Test

To mention, CPU test does not require SDL3.

#### All tests

```sh
make test > test.log 2>test.err.log
```

#### Run specific test

```sh
make <cpu|ram|cartridge>-test > test.log 2>test.err.log
```

### Run emulator

```sh
make
./dmg <rom_file>
```

#### Flags

```
Usage: ./dmg [options] <rom_file>
Options:
  -h, --help            Display this help message
  -d                    Enable debug output
  -v                    Verbose output (WARN, -v INFO, -vv DEBUG, -vvv TRACE, default: 0)
  -s, --scale <n>       Window scale factor (1-6, default: 2)
  -p, --serial          Enable serial output printing
Examples:
  ./dmg SuperMarioLand.gb
  ./dmg -d -vv zelda.gb
  ./dmg --serial cpu_instr.gb
```

#### Keys

```
W - Up
A - Left
S - Down
D - Right
J - A
K - B
LSHIFT - SELECT
ENTER - START
ESC - Quit
P - Print FPS (lifetime + this frame)
F1 - Screenshot (Default: gameboy_framebuffer.bmp)
LALT - Toggle joypad (enable/disable)
LCTRL - Fast forward
```

## Credits

[The Ultimate Game Boy Talk (33c3)](https://www.youtube.com/watch?v=HyzD8pNlpwI)

[cboy](https://github.com/0xf4b1/cboy)

Blargg's test ROMs and source code

[gbops](https://izik1.github.io/gbops/index.html)

[GBEDG](https://hacktix.github.io/GBEDG/)

[emudev](https://emudev.de/gameboy-emulator/)

[gbdev](https://gbdev.io/pandocs/)

[Reddit comment on POP AF](https://www.reddit.com/r/EmuDev/comments/hi237c/gameboy_blargg_test_special01_errors_on_pop_af/)

[SDL3](https://github.com/libsdl-org/SDL)

And of course, our [original project](https://github.com/DarkKowalski/nekomimi-gameboy-emulator) and Thank you ALL.
