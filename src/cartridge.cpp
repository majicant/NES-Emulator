#include <array>
#include <fstream>
#include <iostream>

#include "cartridge.h"
#include "nrom.h"
#include "mmc1.h"
#include "mmc3.h"
#include "uxrom.h"

Cartridge::Cartridge(const std::filesystem::path& filename)
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
		case 1:
			mapper = std::make_unique<MMC1>(header[4], header[5], header[6] & 0x01);
			break;
		case 2:
			mapper = std::make_unique<UxROM>(header[4], header[5], header[6] & 0x01);
			break;
		case 4:
			// MMC3 uses 8 KB PRG banks instead of 16 KB
			mapper = std::make_unique<MMC3>(header[4] * 2, header[5], header[6] & 0x01);
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
	if (address >= 0x6000 && address <= 0x7FFF)
		return mapper->ReadPRGRAM(address);
	else
		return prg_data[mapper->MapCPURead(address)];
}

void Cartridge::CPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x6000 && address <= 0x7FFF)
		mapper->WritePRGRAM(address, value);
	else
		mapper->MapCPUWrite(address, value);
}

uint8_t Cartridge::PPURead(uint16_t address)
{
	return chr_data[mapper->MapPPURead(address)];
}

void Cartridge::PPUWrite(uint16_t address, uint8_t value)
{
	chr_data[mapper->MapPPURead(address)] = value;
}

Mapper* Cartridge::GetMapper()
{
	return mapper.get();
}