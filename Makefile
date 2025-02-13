.PHONY: test clean all

CC=gcc


# SDL3 specific flags
SDL_FLAGS=-ISDL3/include/ -LSDL3/lib/ -lSDL3

# Compiler flags
CC_FLAGS=-Wall -Wextra -pedantic -std=c2x
CC_RELEASE_FLAGS=-O3
CC_DEBUG_FLAGS=-g -DDEBUG

RAM_SRC=src/ram.c
RAM_HEADER=src/ram.h
RAM=$(RAM_SRC) $(RAM_HEADER)

VRAM_SRC=src/vram.c
VRAM_HEADER=src/vram.h
VRAM=$(VRAM_SRC) $(VRAM_HEADER)

CARTRIDGE_SRC=src/cartridge.c
CARTRIDGE_HEADER=src/cartridge.h
CARTRIDGE=$(CARTRIDGE_SRC) $(CARTRIDGE_HEADER)

DMG_SRC=src/dmg.c
DMG_HEADER=src/dmg.h
DMG=$(DMG_SRC) $(DMG_HEADER)

MMU_SRC=src/mmu.c
MMU_HEADER=src/mmu.h
MMU=$(MMU_SRC) $(MMU_HEADER)

TIMER_SRC=src/timer.c
TIMER_HEADER=src/timer.h
TIMER=$(TIMER_SRC) $(TIMER_HEADER)

CPU_SRC=src/cpu.c
CPU_HEADER=src/cpu.h
CPU=$(CPU_SRC) $(CPU_HEADER)

REGISTER_SRC=src/register.c
REGISTER_HEADER=src/register.h
REGISTER=$(REGISTER_SRC) $(REGISTER_HEADER)

PPU_SRC=src/ppu.c
PPU_HEADER=src/ppu.h
PPU=$(PPU_SRC) $(PPU_HEADER)

FORM_SRC=src/form.c	
FORM_HEADER=src/form.h
FORM=$(FORM_SRC) $(FORM_HEADER)

JOYPAD_SRC=src/joypad.c
JOYPAD_HEADER=src/joypad.h
JOYPAD=$(JOYPAD_SRC) $(JOYPAD_HEADER)

DMG=$(DMG_SRC) $(DMG_HEADER) $(MMU) $(TIMER) $(CPU) $(PPU) $(CARTRIDGE) $(RAM) $(VRAM) $(REGISTER) $(FORM) $(JOYPAD)

FORM_TEST=test/nemo-sdl-create-form
RAM_TEST=test/ram-test
CARTRIDGE_TEST=test/cartridge-test
REGISTER_TEST=test/register-test
CPU_TEST=test/cpu-test

all: $(DMG)
	$(CC) $(DMG) -o dmg $(SDL_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

debug: $(DMG)
	$(CC) $(DMG) -o dmg $(SDL_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

test: ram-test cartridge-test register-test cpu-test

ram-test-build: $(RAM_TEST).c $(RAM)
	$(CC) $(RAM_TEST).c $(RAM) -o $(RAM_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

ram-test: ram-test-build
	./$(RAM_TEST)
	echo "Ram test passed"

cartridge-test-build: $(CARTRIDGE_TEST).c $(CARTRIDGE)	
	$(CC) $(CARTRIDGE_TEST).c $(CARTRIDGE) -o $(CARTRIDGE_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

cartridge-test: cartridge-test-build
	./$(CARTRIDGE_TEST)
	echo "Cartridge test passed"

register-test-build: $(REGISTER_TEST).c $(REGISTER)	
	$(CC) $(REGISTER_TEST).c $(REGISTER) -o $(REGISTER_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

register-test: register-test-build
	./$(REGISTER_TEST)
	echo "Register test passed"

cpu-test-build: $(CPU_TEST).c $(CPU) $(REGISTER) $(MMU) $(CARTRIDGE) $(RAM) $(VRAM) $(TIMER) $(PPU)
	$(CC) $(CPU_TEST).c $(CPU) $(REGISTER) $(MMU) $(CARTRIDGE) $(RAM) $(VRAM) $(TIMER) $(PPU) -o $(CPU_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

cpu-test: cpu-test-build
	./$(CPU_TEST)
	echo "CPU test passed"

run: all
	echo "Running emulator"
	./dmg $(filter-out $@,$(MAKECMDGOALS))

run-warning: all
	echo "Running emulator (with WARNING)"
	./dmg $(filter-out $@,$(MAKECMDGOALS)) -d

run-info: all
	echo "Running emulator (with INFO)"
	./dmg $(filter-out $@,$(MAKECMDGOALS)) -d -v

run-debug: all
	echo "Running emulator (with DEBUG)"
	./dmg $(filter-out $@,$(MAKECMDGOALS)) -d -vv

run-trace: all
	echo "Running emulator (with TRACE)"
	./dmg $(filter-out $@,$(MAKECMDGOALS)) -d -vvv

run-cpu-test: all
	echo "Running CPU test"
	./dmg test/cpu.gb -d -vvv

define delete_executables_by_name
	$(foreach exe, $(1), rm -f $(exe); rm -f $(exe).exe;)
endef

clean:
	@$(call delete_executables_by_name, $(FORM_TEST) $(RAM_TEST) $(CARTRIDGE_TEST) $(REGISTER_TEST))
