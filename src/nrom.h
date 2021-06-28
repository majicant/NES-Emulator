#pragma once

#include "mapper.h"

class NROM : public Mapper
{
public:
	NROM(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);

	uint32_t MapCPURead(uint16_t address) override;
};