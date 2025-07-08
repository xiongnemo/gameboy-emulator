.PHONY: test clean all

CC=gcc

# Build directory
BUILD_DIR=build_tmp

# SDL3 specific flags
SDL_INCLUDE_FLAGS=-ISDL3/include/
SDL_LINK_FLAGS=-LSDL3/lib/ -lSDL3
SDL_FLAGS=$(SDL_INCLUDE_FLAGS) $(SDL_LINK_FLAGS)

# Windows Resource Compiler flags
RC_FLAGS=release.res icon.res

# Compiler flags
CC_ALL_WARNINGS=-Wall -Wextra
CC_PEDANTIC_FLAGS=-pedantic
CC_FLAGS=-std=c2x
CC_RELEASE_FLAGS=-O3
CC_DEBUG_FLAGS=-g -DDEBUG

# Source files
RAM_SRC=src/ram.c
RAM_HEADER=src/ram.h

VRAM_SRC=src/vram.c
VRAM_HEADER=src/vram.h

CARTRIDGE_SRC=src/cartridge.c
CARTRIDGE_HEADER=src/cartridge.h

DMG_SRC=src/dmg.c
DMG_HEADER=src/dmg.h

MMU_SRC=src/mmu.c
MMU_HEADER=src/mmu.h

TIMER_SRC=src/timer.c
TIMER_HEADER=src/timer.h

CPU_SRC=src/cpu.c
CPU_HEADER=src/cpu.h

REGISTER_SRC=src/register.c
REGISTER_HEADER=src/register.h

PPU_SRC=src/ppu.c
PPU_HEADER=src/ppu.h

FORM_SRC=src/form.c	
FORM_HEADER=src/form.h

JOYPAD_SRC=src/joypad.c
JOYPAD_HEADER=src/joypad.h

APU_SRC=src/apu.c
APU_HEADER=src/apu.h

# Object files
RAM_OBJ=$(BUILD_DIR)/ram.o
VRAM_OBJ=$(BUILD_DIR)/vram.o
CARTRIDGE_OBJ=$(BUILD_DIR)/cartridge.o
DMG_OBJ=$(BUILD_DIR)/dmg.o
MMU_OBJ=$(BUILD_DIR)/mmu.o
TIMER_OBJ=$(BUILD_DIR)/timer.o
CPU_OBJ=$(BUILD_DIR)/cpu.o
REGISTER_OBJ=$(BUILD_DIR)/register.o
PPU_OBJ=$(BUILD_DIR)/ppu.o
FORM_OBJ=$(BUILD_DIR)/form.o
JOYPAD_OBJ=$(BUILD_DIR)/joypad.o
APU_OBJ=$(BUILD_DIR)/apu.o

# All object files for the main executable
DMG_OBJS=$(DMG_OBJ) $(MMU_OBJ) $(TIMER_OBJ) $(CPU_OBJ) $(PPU_OBJ) $(CARTRIDGE_OBJ) $(RAM_OBJ) $(VRAM_OBJ) $(REGISTER_OBJ) $(FORM_OBJ) $(JOYPAD_OBJ) $(APU_OBJ)

# Test executables
FORM_TEST=test/nemo-sdl-create-form
RAM_TEST=test/ram-test
CARTRIDGE_TEST=test/cartridge-test
REGISTER_TEST=test/register-test
CPU_TEST=test/cpu-test

build: all

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Object file rules
$(RAM_OBJ): $(RAM_SRC) $(RAM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(RAM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(VRAM_OBJ): $(VRAM_SRC) $(VRAM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(VRAM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(CARTRIDGE_OBJ): $(CARTRIDGE_SRC) $(CARTRIDGE_HEADER) | $(BUILD_DIR)
	$(CC) -c $(CARTRIDGE_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(DMG_OBJ): $(DMG_SRC) $(DMG_HEADER) | $(BUILD_DIR)
	$(CC) -c $(DMG_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(MMU_OBJ): $(MMU_SRC) $(MMU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(MMU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(TIMER_OBJ): $(TIMER_SRC) $(TIMER_HEADER) | $(BUILD_DIR)
	$(CC) -c $(TIMER_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(CPU_OBJ): $(CPU_SRC) $(CPU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(CPU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(REGISTER_OBJ): $(REGISTER_SRC) $(REGISTER_HEADER) | $(BUILD_DIR)
	$(CC) -c $(REGISTER_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(PPU_OBJ): $(PPU_SRC) $(PPU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(PPU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(FORM_OBJ): $(FORM_SRC) $(FORM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(FORM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(JOYPAD_OBJ): $(JOYPAD_SRC) $(JOYPAD_HEADER) | $(BUILD_DIR)
	$(CC) -c $(JOYPAD_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

$(APU_OBJ): $(APU_SRC) $(APU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(APU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

# Debug object file rules
$(BUILD_DIR)/ram-debug.o: $(RAM_SRC) $(RAM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(RAM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/vram-debug.o: $(VRAM_SRC) $(VRAM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(VRAM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/cartridge-debug.o: $(CARTRIDGE_SRC) $(CARTRIDGE_HEADER) | $(BUILD_DIR)
	$(CC) -c $(CARTRIDGE_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/dmg-debug.o: $(DMG_SRC) $(DMG_HEADER) | $(BUILD_DIR)
	$(CC) -c $(DMG_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/mmu-debug.o: $(MMU_SRC) $(MMU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(MMU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/timer-debug.o: $(TIMER_SRC) $(TIMER_HEADER) | $(BUILD_DIR)
	$(CC) -c $(TIMER_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/cpu-debug.o: $(CPU_SRC) $(CPU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(CPU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/register-debug.o: $(REGISTER_SRC) $(REGISTER_HEADER) | $(BUILD_DIR)
	$(CC) -c $(REGISTER_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/ppu-debug.o: $(PPU_SRC) $(PPU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(PPU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/form-debug.o: $(FORM_SRC) $(FORM_HEADER) | $(BUILD_DIR)
	$(CC) -c $(FORM_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/joypad-debug.o: $(JOYPAD_SRC) $(JOYPAD_HEADER) | $(BUILD_DIR)
	$(CC) -c $(JOYPAD_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

$(BUILD_DIR)/apu-debug.o: $(APU_SRC) $(APU_HEADER) | $(BUILD_DIR)
	$(CC) -c $(APU_SRC) -o $@ $(SDL_INCLUDE_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

# Debug object files collection
DMG_DEBUG_OBJS=$(BUILD_DIR)/dmg-debug.o $(BUILD_DIR)/mmu-debug.o $(BUILD_DIR)/timer-debug.o $(BUILD_DIR)/cpu-debug.o $(BUILD_DIR)/ppu-debug.o $(BUILD_DIR)/cartridge-debug.o $(BUILD_DIR)/ram-debug.o $(BUILD_DIR)/vram-debug.o $(BUILD_DIR)/register-debug.o $(BUILD_DIR)/form-debug.o $(BUILD_DIR)/joypad-debug.o $(BUILD_DIR)/apu-debug.o

default: all

# Main targets
all: $(DMG_OBJS)
	$(CC) $(DMG_OBJS) -o dmg $(SDL_LINK_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS)

all-cpu-test: debug
	./dmg test/cpu.gb --serial

all-cpu-test-verbose: debug
	./dmg test/cpu.gb -d -vv --serial

windows-release: $(DMG_OBJS)
	$(CC) $(DMG_OBJS) -o dmg $(SDL_LINK_FLAGS) $(CC_FLAGS) $(CC_RELEASE_FLAGS) $(RC_FLAGS)

debug: $(DMG_DEBUG_OBJS)
	$(CC) $(DMG_DEBUG_OBJS) -o dmg $(SDL_LINK_FLAGS) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

test: ram-test cartridge-test register-test cpu-test

ram-test-build: $(RAM_TEST).c $(BUILD_DIR)/ram-debug.o
	$(CC) $(RAM_TEST).c $(BUILD_DIR)/ram-debug.o -o $(RAM_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

ram-test: ram-test-build
	./$(RAM_TEST)
	echo "Ram test passed"

cartridge-test-build: $(CARTRIDGE_TEST).c $(BUILD_DIR)/cartridge-debug.o
	$(CC) $(CARTRIDGE_TEST).c $(BUILD_DIR)/cartridge-debug.o -o $(CARTRIDGE_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

cartridge-test: cartridge-test-build
	./$(CARTRIDGE_TEST)
	echo "Cartridge test passed"

register-test-build: $(REGISTER_TEST).c $(BUILD_DIR)/register-debug.o
	$(CC) $(REGISTER_TEST).c $(BUILD_DIR)/register-debug.o -o $(REGISTER_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

register-test: register-test-build
	./$(REGISTER_TEST)
	echo "Register test passed"

cpu-test-build: $(CPU_TEST).c $(BUILD_DIR)/cpu-debug.o $(BUILD_DIR)/register-debug.o $(BUILD_DIR)/mmu-debug.o $(BUILD_DIR)/cartridge-debug.o $(BUILD_DIR)/ram-debug.o $(BUILD_DIR)/vram-debug.o $(BUILD_DIR)/timer-debug.o $(BUILD_DIR)/ppu-debug.o
	$(CC) $(CPU_TEST).c $(BUILD_DIR)/cpu-debug.o $(BUILD_DIR)/register-debug.o $(BUILD_DIR)/mmu-debug.o $(BUILD_DIR)/cartridge-debug.o $(BUILD_DIR)/ram-debug.o $(BUILD_DIR)/vram-debug.o $(BUILD_DIR)/timer-debug.o $(BUILD_DIR)/ppu-debug.o -o $(CPU_TEST) $(CC_FLAGS) $(CC_DEBUG_FLAGS)

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
	@$(call delete_executables_by_name, $(FORM_TEST) $(RAM_TEST) $(CARTRIDGE_TEST) $(REGISTER_TEST) $(CPU_TEST))
	rm -rf $(BUILD_DIR)
	rm -f dmg dmg.exe
