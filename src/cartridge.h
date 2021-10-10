#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "mapper.h"

class Cartridge
{
public:
	Cartridge(const std::filesystem::path& filename);

	uint8_t CPURead(uint16_t address);
	void CPUWrite(uint16_t address, uint8_t value);

	uint8_t PPURead(uint16_t address);
	void PPUWrite(uint16_t address, uint8_t value);

	Mapper* GetMapper();

private:
	std::unique_ptr<Mapper> mapper;

	std::vector<uint8_t> prg_data;
	std::vector<uint8_t> chr_data;
};