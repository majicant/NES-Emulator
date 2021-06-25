#pragma once

#include <cstdint>

#include <SDL.h>

class SDLEngine
{
public:
	SDLEngine();
	~SDLEngine();

	void UpdateDisplay(const uint8_t* framebuffer);
	void UpdateController(uint8_t& buttons);

	void UpdateExitFlag();
	bool GetExitFlag();

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
	bool exit_flag = false;
};