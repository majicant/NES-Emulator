#pragma once

#include <array>
#include <vector>

#include "mapper.h"

class MMC3 : public Mapper
{
public:
	MMC3(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);

	uint32_t MapCPURead(uint16_t address) override;
	void MapCPUWrite(uint16_t address, uint8_t value) override;

	uint32_t MapPPURead(uint16_t address) override;

	uint8_t ReadPRGRAM(uint16_t address) override;
	void WritePRGRAM(uint16_t address, uint8_t value) override;

	bool IRQ() override;

private:
	struct InternalRegisters
	{
		uint8_t bank_select = 0x00;
		std::array<uint8_t, 8> bank_data = {};
		uint8_t prg_ram_protect = 0x00;
		uint8_t irq_latch = 0x00;
		bool irq_reload = false;
		bool irq_enabled = false;
	} internal_regs;

	unsigned irq_counter = 0;

	std::vector<uint8_t> prg_ram;
};