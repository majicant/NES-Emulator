#pragma once

#include "mapper.h"

class UxROM : public Mapper
{
public:
	UxROM(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);

	uint32_t MapCPURead(uint16_t address) override;
	void MapCPUWrite(uint16_t address, uint8_t value) override;

private:
	uint8_t bank_select = 0x00;
};