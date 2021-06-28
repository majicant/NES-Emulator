#include "nrom.h"

NROM::NROM(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror)
	: Mapper(prg_banks, chr_banks, mirror)
{
}

uint32_t NROM::MapCPURead(uint16_t address)
{
	return address & ((prg_banks == 1) ? 0x3FFF : 0x7FFF);
}