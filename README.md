# gameboy-emulator
A rework of https://github.com/xiongnemo/nekomimi-gameboy-emulator-1 (https://github.com/DarkKowalski/nekomimi-gameboy-emulator)

## Development

### Prerequisite

* gcc: Some macros is GNU extension.

```pwsh
nemo @ nemo-tablet-14c in ~\Documents\GitHub\gameboy-emulator on git:dev x [00:00:00]
$ gcc --version
gcc.exe (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders, r2) 14.2.0
Copyright (C) 2024 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

* SDL3: You should have SDL3 installed or accessible in your system.

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
