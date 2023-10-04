#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>

class chip8 {
private:
	unsigned char fontset;

	int cycles;

	uint16_t I, PC;
	uint16_t stack[16];
	uint16_t address;
	uint16_t opcode;
	uint16_t opcode_1F;

	uint8_t memory[4096];
	uint8_t Vx[16];
	uint8_t SP;
	uint8_t keyboard[16];
	uint8_t keyb_FX0A_dump[16];
	uint8_t sound_timer, delay_timer;
	uint8_t sprite[15];
	uint8_t reg[2];
	uint8_t constant[2];
	uint8_t x, y;

	bool opcode_trig;
	bool keyb_FX0A;

	friend class CPU_log_class;

public:
	void hex_to_bin(int* array_, uint8_t byte);
	void set_keypad(int keys, int value);
	void write_sprite(uint8_t x, uint8_t y, uint8_t* disp_buf, uint8_t* sprite, uint8_t sprite_size);
	uint8_t display[64*32];
	bool draw_flag;

	bool init();
	bool load_rom(const char*);
	bool cpu_cycle();
	int get_args(int, int, int);

};

class CPU_log_class {
public:
	void log_CPU(chip8& _cpu, bool reg_out, int _delay, bool _trace);
};