#pragma once

#include <cstdint>
#include <vector>

class RAM
{
public:
	RAM();

	uint8_t ReadByte(uint16_t address);
	void WriteByte(uint16_t address, uint8_t value);

	uint16_t ReadWord(uint16_t address);
	void WriteWord(uint16_t address, uint16_t value);

private:
	std::vector<uint8_t> memory;
};