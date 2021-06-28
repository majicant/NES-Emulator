#include "mmc1.h"

MMC1::MMC1(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror)
	: Mapper(prg_banks, chr_banks, mirror)
	, prg_ram(8192)
{
}

uint32_t MMC1::MapCPURead(uint16_t address)
{
	switch ((internal_regs.control & 0x0C) >> 2) {
	case 0:
	case 1:
		// switch 32 KB at $8000, ignoring low bit of bank number
		return ((internal_regs.prg_bank & 0x0E) * 0x4000) + (address & 0x7FFF);
	case 2:
		// fix first bank at $8000 and switch 16 KB bank at $C000
		if (address >= 0x8000 && address <= 0xBFFF)
			return address & 0x3FFF;
		else
			return ((internal_regs.prg_bank & 0x0F) * 0x4000) + (address & 0x3FFF);
	case 3:
		// fix last bank at $C000 and switch 16 KB bank at $8000
		if (address >= 0x8000 && address <= 0xBFFF)
			return ((internal_regs.prg_bank & 0x0F) * 0x4000) + (address & 0x3FFF);
		else
			return ((prg_banks - 1) * 0x4000) + (address & 0x3FFF);
	}
	return address;
}

void MMC1::MapCPUWrite(uint16_t address, uint8_t value)
{
	if (value & 0x80) {
		shift_register = 0;
		sr_counter = 0;
	}
	else {
		sr_counter++;
		shift_register = (shift_register >> 1) | ((value & 0x01) << 4);
		if (sr_counter == 5) {
			CopyShiftRegister(address);
			shift_register = 0;
			sr_counter = 0;
		}
	}
}

uint32_t MMC1::MapPPURead(uint16_t address)
{
	if (internal_regs.control & 0x10) {
		// switch two separate 4 KB banks
		if (address >= 0x0000 && address <= 0x0FFF)
			return (internal_regs.chr_bank0 * 0x1000) + (address & 0x0FFF);
		else
			return (internal_regs.chr_bank1 * 0x1000) + (address & 0x0FFF);
	}
	else {
		// switch 8 KB at a time
		if (address >= 0x0000 && address <= 0x0FFF)
			return ((internal_regs.chr_bank0 & 0x1E) * 0x1000) + (address & 0x1FFF);
	}
	return address;
}

uint8_t MMC1::ReadPRGRAM(uint16_t address)
{
	if ((internal_regs.prg_bank & 0x10) == 0)
		return prg_ram[address & 0x1FFF];
	return 0x00;
}

void MMC1::WritePRGRAM(uint16_t address, uint8_t value)
{
	if ((internal_regs.prg_bank & 0x10) == 0)
		prg_ram[address & 0x1FFF] = value;
}

void MMC1::CopyShiftRegister(uint16_t address)
{
	switch ((address & 0x6000) >> 13) {
	case 0:	// Control
		internal_regs.control = shift_register;
		mirror = static_cast<Mirror>(internal_regs.control & 0x03);
		break;
	case 1:	// CHR bank 0
		internal_regs.chr_bank0 = shift_register;
		break;
	case 2:	// CHR bank 1
		internal_regs.chr_bank1 = shift_register;
		break;
	case 3:	// PRG bank
		internal_regs.prg_bank = shift_register;
		break;
	}
}