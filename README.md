# gameboy-emulator
A rework of https://github.com/xiongnemo/nekomimi-gameboy-emulator-1 (https://github.com/DarkKowalski/nekomimi-gameboy-emulator)

**Note:** This branch contains a bug related to enum and struct:  

https://github.com/anzupop/gameboy-emulator/blob/struct-enum-bug/src/cpu.h#L92

if add four uint8_t padding to this struct or delete any one element, the cpu-test will work normally.

This has something to do with the enum and struct alignment but I don't have time to write down the details yet.

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
