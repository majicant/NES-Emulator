#pragma once

#include <cstdint>

class Mapper
{
public:
	Mapper(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);
	virtual ~Mapper() = default;

	virtual uint16_t MapCPURead(uint16_t address) = 0;
	virtual uint16_t MapCPUWrite(uint16_t address) = 0;

	inline virtual uint8_t GetMirror() { return mirror; }

protected:
	uint8_t prg_banks;
	uint8_t chr_banks;
	uint8_t mirror;
};