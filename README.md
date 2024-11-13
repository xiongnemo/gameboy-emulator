# gameboy-emulator
A rework of https://github.com/xiongnemo/nekomimi-gameboy-emulator-1 (https://github.com/DarkKowalski/nekomimi-gameboy-emulator)

## Development

You should have SDL3 installed or accessible in your system.

### Example: VSCode

- Install C/C++ extension
- Setup SDL3 include path in `c_cpp_properties.json`

### Run test

```sh
make test > test.log
```

### Run emulator

```sh
make
./dmg <rom_file>
```
