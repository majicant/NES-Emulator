#include <array>
#include <fstream>
#include <iostream>

#include "cartridge.h"
#include "nrom.h"

Cartridge::Cartridge(const std::string& filename)
{
	std::ifstream rom(filename, std::ifstream::binary);
	if (rom.is_open()) {
		std::array<uint8_t, 16> header;
		rom.read(reinterpret_cast<char*>(header.data()), 16);

		if (header[6] & 0x04)
			rom.seekg(512, std::ifstream::cur);

		prg_data.resize(header[4] * 16384);
		chr_data.resize(header[5] ? (header[5] * 8192) : 8192);

		rom.read(reinterpret_cast<char*>(prg_data.data()), prg_data.size());
		rom.read(reinterpret_cast<char*>(chr_data.data()), chr_data.size());

		uint8_t mapper_num = (header[7] & 0xF0) | (header[6] >> 4);
		switch (mapper_num) {
		case 0:
			mapper = std::make_unique<NROM>(header[4], header[5], header[6] & 0x01);
			break;
		default:
			std::cerr << "Unimplemented mapper!" << std::endl;
			break;
		}
		rom.close();
	}
}

uint8_t Cartridge::CPURead(uint16_t address)
{
	return prg_data[mapper->MapCPURead(address)];
}

void Cartridge::CPUWrite(uint16_t address, uint8_t value)
{
	prg_data[mapper->MapCPUWrite(address)] = value;
}

uint8_t Cartridge::PPURead(uint16_t address)
{
	return chr_data[address];
}

void Cartridge::PPUWrite(uint16_t address, uint8_t value)
{
	chr_data[address] = value;
}

Mapper* Cartridge::GetMapper()
{
	return mapper.get();
}