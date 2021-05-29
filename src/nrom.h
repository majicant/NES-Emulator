#pragma once

#include <cstdint>

#include "mapper.h"

class NROM : public Mapper
{
public:
	NROM(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);
	~NROM() = default;

	uint16_t MapCPURead(uint16_t address);
	uint16_t MapCPUWrite(uint16_t address);

private:
	uint8_t mirror;
};