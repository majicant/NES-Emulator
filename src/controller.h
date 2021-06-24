#pragma once

#include "sdl_engine.h"

#include <cstdint>

class Controller
{
public:
	Controller(SDLEngine& engine);

	uint8_t Read();
	void Write();

private:
	SDLEngine& engine;
	uint8_t buttons = 0x00;
};