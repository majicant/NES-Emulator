#pragma once

#include <cstdint>

#include <SDL.h>

class SDLEngine
{
public:
	SDLEngine();
	~SDLEngine();

	void UpdateDisplay(const uint8_t* framebuffer);

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
};