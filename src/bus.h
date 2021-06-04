#pragma once

#include <cstdint>
#include <vector>

#include "cartridge.h"

class Bus
{
public:
	Bus(Cartridge& cart);

	uint8_t CPURead(uint16_t address);
	void CPUWrite(uint16_t address, uint8_t value);

	uint8_t PPURead(uint16_t address);
	void PPUWrite(uint16_t address, uint8_t value);

private:
	Cartridge& cartridge;
	std::vector<uint8_t> ram;

	std::vector<uint8_t> vram;
	std::vector<uint8_t> palette_ram;
};