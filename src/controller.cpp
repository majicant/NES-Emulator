#include "controller.h"

Controller::Controller(SDLEngine& engine)
	: engine(engine)
{
}

uint8_t Controller::Read()
{
	uint8_t first_bit = buttons & 0x01;
	buttons >>= 1;
	return first_bit;
}

void Controller::Write()
{
	engine.UpdateController(buttons);
}