#pragma once

#include <cstdint>

class Mapper
{
public:
	Mapper(uint8_t prg_banks, uint8_t chr_banks);
	virtual ~Mapper() = default;

	virtual uint16_t MapCPURead(uint16_t address) = 0;
	virtual uint16_t MapCPUWrite(uint16_t address) = 0;

protected:
	uint8_t prg_banks;
	uint8_t chr_banks;
};