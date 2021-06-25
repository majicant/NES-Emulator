#include "uxrom.h"

UxROM::UxROM(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror)
	: Mapper(prg_banks, chr_banks, mirror)
{
}

uint32_t UxROM::MapCPURead(uint16_t address)
{
	if (address >= 0x8000 && address <= 0xBFFF)
		return (bank_select * 0x4000) + (address & 0x3FFF);
	if (address >= 0xC000 && address <= 0xFFFF)
		return ((prg_banks - 1) * 0x4000) + (address & 0x3FFF);
	return address;
}

void UxROM::MapCPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x8000 && address <= 0xFFFF)
		bank_select = value & 0x0F;
}