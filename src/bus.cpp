#include <cstdint>

#include "bus.h"

Bus::Bus(Cartridge& cart)
	: cartridge(cart)
	, ram(2048)
{
}

uint8_t Bus::CPURead(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		return ram[address & 0x07FF];
	else if (address >= 0x8000 && address <= 0xFFFF)
		return cartridge.CPURead(address);
	return 0x0000;
}

void Bus::CPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		ram[address & 0x07FF] = value;
	else if (address >= 0x8000 && address <= 0xFFFF)
		cartridge.CPUWrite(address, value);
}