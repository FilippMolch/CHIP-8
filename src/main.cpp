#include <iostream>
#include <SDL/SDL.h>
#include <chip8.h>
#include <string>
#include <math.h>
#include <Windows.h>
#include <time.h>

uint8_t keys[16] = {
		SDLK_x,
		SDLK_1,
		SDLK_2,
		SDLK_3,
		SDLK_q,
		SDLK_w,
		SDLK_e,
		SDLK_a,
		SDLK_s,
		SDLK_d,
		SDLK_z,
		SDLK_c,
		SDLK_4,
		SDLK_r,
		SDLK_f,
		SDLK_v
};


int main(int argc, char* argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* win = SDL_CreateWindow("CHIP-8 EMU", 100, 100, 64 * 16, 32 * 16, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
	if (ren == nullptr) {
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Texture* pixels = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	chip8 cpu;
	if (!cpu.init()) std::cerr << "INIT fail";
	cpu.load_rom("C:\\Users\\Filipp\\source\\repos\\CHIP-8 EMU\\ConsoleApplication2\\ROMs\\pong.ch8");

	uint32_t p[64*32];

	uint8_t loc_d[2048];
	memset(loc_d, 0, 2048);

	CPU_log_class CH8_log;

	while (true)
	{
		cpu.cpu_cycle();
		//CH8_log.log_CPU(cpu, true, 0, true);

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					exit(0);
				}

				for (int i = 0; i < 16; ++i)
				{
					if (event.key.keysym.sym == keys[i])
					{
						cpu.set_keypad(i, 0);
					}
				}
			}

			if (event.type == SDL_KEYUP)
			{
				for (int i = 0; i < 16; ++i)
				{
					if (event.key.keysym.sym == keys[i])
					{
						cpu.set_keypad(i, 1);
					}
				}
			}

			switch (event.type)
			{
			case SDL_QUIT:
				SDL_DestroyTexture(pixels);
				SDL_DestroyRenderer(ren);
				SDL_DestroyWindow(win);
				SDL_Quit();
				exit(0);
				break;

			default:
				break;
			}
		}

		if (cpu.draw_flag) {
			for (int i = 0; i < 2048; i++) {
				p[i] = cpu.display[i] > 0 ? 0xFFFFFFFF : 0x000000FF;

			}
			
			SDL_UpdateTexture(pixels, NULL, p, 64 * sizeof(uint32_t));
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, pixels, NULL, NULL);
			SDL_RenderPresent(ren);

			SDL_UpdateWindowSurface(win);
			
			
			cpu.draw_flag = false;
			
		}

		SDL_Delay(1);
	}

	SDL_DestroyTexture(pixels);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
