#include <cstdint>

#include "ram.h"

RAM::RAM()
	: memory(64 * 1024)
{
}

uint8_t RAM::ReadByte(uint16_t address)
{
	return memory[address];
}

void RAM::WriteByte(uint16_t address, uint8_t value)
{
	memory[address] = value;
}

uint16_t RAM::ReadWord(uint16_t address)
{
	return (static_cast<uint16_t>(memory[address + 1]) << 8) | static_cast<uint16_t>(memory[address]);
}

void RAM::WriteWord(uint16_t address, uint16_t value)
{
	memory[address] = value & 0x00FF;
	memory[address + 1] = (value & 0xFF00) >> 8;
}