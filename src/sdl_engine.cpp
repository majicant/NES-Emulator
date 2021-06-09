#include "sdl_engine.h"

SDLEngine::SDLEngine()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256 * 3, 240 * 3, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
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