#include "sdl_engine.h"

SDLEngine::SDLEngine()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256 * 3, 240 * 3, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
}

SDLEngine::~SDLEngine()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
}

void SDLEngine::UpdateDisplay(const uint8_t* framebuffer)
{
	SDL_UpdateTexture(texture, nullptr, framebuffer, 256 * 3);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

void SDLEngine::UpdateController(uint8_t& buttons)
{
	SDL_PumpEvents();
	buttons = 0x00;
	if (keyboard_state[SDL_SCANCODE_L])	// A
		buttons |= 0x01;
	if (keyboard_state[SDL_SCANCODE_K])	// B
		buttons |= 0x02;
	if (keyboard_state[SDL_SCANCODE_X])	// Select
		buttons |= 0x04;
	if (keyboard_state[SDL_SCANCODE_Z])	// Start
		buttons |= 0x08;
	if (keyboard_state[SDL_SCANCODE_W])	// Up
		buttons |= 0x10;
	if (keyboard_state[SDL_SCANCODE_S])	// Down
		buttons |= 0x20;
	if (keyboard_state[SDL_SCANCODE_A])	// Left
		buttons |= 0x40;
	if (keyboard_state[SDL_SCANCODE_D])	// Right
		buttons |= 0x80;
}

void SDLEngine::UpdateExitFlag()
{
	SDL_Event e;
	SDL_PollEvent(&e);
	exit_flag = (e.type == SDL_QUIT);
}

bool SDLEngine::GetExitFlag()
{
	return exit_flag;
}