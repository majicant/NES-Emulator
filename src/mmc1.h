#pragma once

#include <vector>

#include "mapper.h"

class MMC1 : public Mapper
{
public:
	MMC1(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);

	uint32_t MapCPURead(uint16_t address) override;
	void MapCPUWrite(uint16_t address, uint8_t value) override;

	uint32_t MapPPURead(uint16_t address) override;

	uint8_t ReadPRGRAM(uint16_t address) override;
	void WritePRGRAM(uint16_t address, uint8_t value) override;

private:
	void CopyShiftRegister(uint16_t address);

	struct InternalRegisters
	{
		uint8_t control = 0x0C;
		uint8_t chr_bank0 = 0x00;
		uint8_t chr_bank1 = 0x00;
		uint8_t prg_bank = 0x00;
	} internal_regs;

	uint8_t shift_register = 0x00;
	uint8_t sr_counter = 0x00;

	std::vector<uint8_t> prg_ram;
};