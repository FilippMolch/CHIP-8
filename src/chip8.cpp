#include <chip8.h>

#define OPCODE_FIND opcode_trig = true;
#define VX this->get_args(opcode, 8, 0x0F00)
#define VY this->get_args(opcode, 4, 0x00F0)
#define K this->get_args(opcode, 0, 0x000F)
#define KK this->get_args(opcode, 0, 0x00FF)
#define NNN this->get_args(opcode, 0, 0x0FFF)

void CPU_log_class::log_CPU(chip8& _cpu, bool reg_out, int _delay, bool _trace) {
	using namespace std;

	cout << "Programm counter: " << (int)_cpu.PC << " I: " << (int)_cpu.I << endl;
	cout << "Delay Timer: " << (int)_cpu.delay_timer << " sound timer: " << (int)_cpu.sound_timer << endl;

	printf("Opcode: ");
	printf("%X", _cpu.opcode);
	printf("\n");

	if (reg_out){
		for (int i = 0; i < 16; i++) {
			cout << "V" << i << ": " << (int)_cpu.Vx[i] << endl;
		}
	}

	Sleep(_delay);

	if (_trace) system("pause");

	system("cls");
}

bool chip8::init() {
	unsigned char fontset[80] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	cycles = 0;

	PC = 0x200;
	I = 0;
	memset(Vx, 0, 16);

	SP = 0;
	memset(stack, 0, 32);

	memset(memory, 0, 4096);
	memset(display, 0, 64*32);

	memset(keyboard, 255, 16);

	sound_timer = 0;
	delay_timer = 0;

	address = 0;

	memset(sprite, 0, 15);
	memset(reg, 0, 2);
	memset(constant, 0, 2);
	
	opcode_trig = 0;

	keyb_FX0A = false;
	memset(keyb_FX0A_dump, 0, 16);

	opcode = 0;
	opcode_1F = 0;


	for (int i = 0; i < 80; i++){
		memory[i] = fontset[i];
	}

	return true;
}

bool chip8::load_rom(const char* path){
	char ch;
	
	try{
		
		int it = 0;
		std::ifstream rom(path, std::ios::binary | std::ios::in);
		
		if (rom.is_open()) {
			
			if (it > 3584) {
				throw (std::exception)"the ROM size is to big";
			}
			else {
				while (!rom.eof()){
					rom.get(ch);
					
					memory[512 + it] = (uint8_t) ch;
					it++;
				}
			}
		}
		
		else {
			throw (std::exception)"Failed to open ROM file";
			return false;
		}

		rom.close();
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	return true;
}

bool chip8::cpu_cycle() {
	opcode = (memory[PC] << 8) | (memory[PC + 1]);
	opcode_1F = get_args(opcode, 12, 0xFFFF);

	cycles++;

	if (cycles%5 == 0) {
		if (delay_timer > 0) {
			delay_timer--;
		}
		if (sound_timer > 0){
			sound_timer--;
		}
	}

	if (opcode_1F == 0){
		uint8_t opcode_0 = get_args(opcode, 0, 0x00FF);

		switch (opcode_0)
		{
		case 0xE0:
			//00e0
			memset(display, 0, 2048);
			OPCODE_FIND
			break;
		case 0xEE:
			//00ee
			if (SP > 0) SP--;
			PC = stack[SP];
			stack[SP] = 0x000;
			OPCODE_FIND
			break;
		}
	}


	if (opcode_1F == 8){
		uint8_t opcode_8 = get_args(opcode, 0, 0x000F);
		switch (opcode_8) {
		case 0:
			//8xy0 LD
			Vx[VX] = Vx[VY];
			OPCODE_FIND
			break;

		case 1:
			//8xy1 OR
			Vx[VX] = Vx[VX] | Vx[VY];
			OPCODE_FIND
			break;

		case 2:
			//8xy2 AND
			Vx[VX] = Vx[VX] & Vx[VY];
			OPCODE_FIND
			break;

		case 3:
			//8xy3 XOR
			Vx[VX] = Vx[VX] ^ Vx[VY];
			Vx[0xF] = 0;
			OPCODE_FIND
			break;

		case 4:
			//8xy4 ADD
			if ((Vx[VX] + Vx[VY]) > 255) {
				Vx[VX] = (Vx[VX] + Vx[VY]) & 0x00FF;
				Vx[0xF] = 1;
			}
			else{
				Vx[VX] = Vx[VX] + Vx[VY];
				Vx[0xF] = 0;
			}
			OPCODE_FIND
			break;

		case 5:
			//8xy5 SUB
			Vx[0xF] = Vx[VX] >= Vx[VY];
			Vx[VX] = Vx[VX] - Vx[VY];
			OPCODE_FIND
			break;

		case 6:
			//8xy6 SHR
			Vx[0xF] = Vx[VX] & 0x01;
			Vx[VX] = Vx[VX] >> 1;
			OPCODE_FIND
			break;

		case 7:
			//8xy7 SUBN
			Vx[0xF] = Vx[VY] >= Vx[VX];
			Vx[VX] = Vx[VY] >= Vx[VX];
			OPCODE_FIND
			break;

		case 0xE:
			//8xye SHL
			Vx[0xF] = Vx[VX] & 0x01;
			Vx[VX] = Vx[VX] << 1;
			OPCODE_FIND
			break;
		}
	}
	
	if (opcode_1F == 0xE) {
		uint8_t opcode_E = get_args(opcode, 0, 0x000F);

		switch (opcode_E){
		case 0xE:
			//Ex9E SKP
			if (keyboard[Vx[VX]] == 0){
				PC += 2;
			}
			OPCODE_FIND
			break;

		case 1:
			//ExA1 SKNP
			if (keyboard[Vx[VX]] != 0) {
				PC += 2;
			}
			OPCODE_FIND
			break;
		}
	}

	if (opcode_1F == 0xF) {
		uint8_t opcode_F = get_args(opcode, 0, 0x00FF);
		
		switch (opcode_F) {
		case 0x07:
			//Fx07 LD
			Vx[VX] = delay_timer;
			OPCODE_FIND
			break;

		case 0x0A:
			//Fx0A LD

			keyb_FX0A = true;
			for (int i = 0; i < 16; i++){
				keyb_FX0A_dump[i] = keyboard[i];
			}

			while (keyb_FX0A){
				for (int i = 0; i < 16; i++) {
					if (keyb_FX0A_dump[i] != keyboard[i]){
						keyb_FX0A = false;
						break;
					}
				}
			}
			OPCODE_FIND
			break;

		case 0x15:
			//Fx15 LD
			delay_timer = Vx[VX];
			OPCODE_FIND
			break;

		case 0x18:
			//Fx18 LD
			sound_timer = Vx[VX];
			OPCODE_FIND
			break;

		case 0x1E:
			//Fx1E ADD
			I = I + Vx[VX];
			OPCODE_FIND
			break;

		case 0x29:
			//Fx29 LD
			I = Vx[VX] * 0x5;
			OPCODE_FIND
			break;

		case 0x33:
			//Fx33 LD
			memory[I] = (uint8_t)((uint8_t)Vx[VX] / 100);
			memory[I + 1] = (uint8_t)((uint8_t)(Vx[VX] / 10) % 10);
			memory[I + 2] = (uint8_t)((uint8_t)(Vx[VX] % 100) % 10);
			OPCODE_FIND
			break;

		case 0x55:
			//Fx55 LD
			for (int i = 0; i <= VX; i++){
				memory[I+i] = Vx[i];
			}

			I = I + VX + 1;
			OPCODE_FIND
			break;

		case 0x65:
			//Fx65 LD
			for (int i = 0; i <= VX; i++){
				Vx[i] = memory[I+i];
			}

			I = I + VX + 1;
			OPCODE_FIND
			break;
		}
	}

	switch (opcode_1F){

	case 1:
		//1nnn JP
		address = NNN;
		PC = address - 2;
		break;

	case 2:
		//2nnn CALL
		stack[SP] = PC;
		address = NNN;
		PC = address - 2;
		SP++;
		break;

	case 3:
		//3xkk SE
		reg[0] = Vx[VX];
		constant[0] = KK;

		if (reg[0] == constant[0]){
			PC += 2;
		}
		break;

	case 4:
		//4xkk SNE
		reg[0] = Vx[VX];
		constant[0] = KK;

		if (reg[0] != constant[0]) {
			PC += 2;
		}
		
		break;

	case 5:
		//5xy0 SE
		reg[0] = Vx[VX];
		reg[1] = Vx[VY];

		if (reg[0] == reg[1]) {
			PC += 2;
		}
		
		break;

	case 6:
		//6xkk LD
		constant[0] = KK;
		Vx[VX] = constant[0];
		break;

	case 7:
		//7xkk ADD
		constant[0] = KK;
		Vx[VX] += constant[0];
		break;

	case 9:
		//9xy0 SNE
		reg[0] = Vx[VX];
		reg[1] = Vx[VY];

		if (reg[0] != reg[1]) {
			PC += 2;
		}
		break;

	case 0xA:
		//Annn LD
		address = NNN;
		I = address;
		break;

	case 0xB:
		//Bnnn JP
		PC = I + Vx[0];
		break;

	case 0xC:
		//Cxkk RND
		constant[0] = KK;
		Vx[VX] = ((rand() % 255) & constant[0]);
		break;

	case 0xD:
		//Dxyn DRW

		//X
		reg[0] = Vx[VX];
		//Y
		reg[1] = Vx[VY];
		//N
		constant[0] = K;
		memset(sprite, 0, 15);

		if (constant[0] < 15) {
			for (int i = 0; i < constant[0]; i++){
				sprite[i] = memory[I + i];
			}
		}
		if (reg[0] <= 63 && reg[1] <= 31) {
			this->write_sprite(reg[0], reg[1], sprite, 5);
		}
		else{
			this->write_sprite(63, 31, sprite, 5);
		}


		draw_flag = true;
		break;

	default:
		if (!opcode_trig){
			std::cout << "UNKNOWN opcode -> ";
			printf("%X", opcode);
			printf("\n");
			return false;
		}
		break;
	}

	PC += 2;

	return true;

}


void chip8::hex_to_bin(int* array_, uint8_t byte) {
	int bb = 7;
	int po = 0;
	int i = 0;

	while (bb > -1) {
		po = pow(2, bb);
		bb--;
		array_[i] = ((byte & po) ? 1 : 0);
		i++;
	}
	bb = 7;
	po = 0;

}

void chip8::write_sprite(uint8_t x, uint8_t y, uint8_t* sprite, uint8_t sprite_size) {
	
	int str[8];
	
	for (int i = 0; i < sprite_size; i++) {
		this->hex_to_bin(str, sprite[i]);
		for (int k = 0; k < 8; k++){
			display[(x + ((y+i) * 64)) + k] ^= str[k];
		}
	}
	

}

int chip8::get_args(int opcode, int offset, int mask) {
	int index = (opcode & mask) >> offset;
	return index;
}

void chip8::set_keypad(int keys, int value) {
	keyboard[keys] = value;
}