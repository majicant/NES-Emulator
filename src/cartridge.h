#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "mapper.h"

class Cartridge
{
public:
	Cartridge(const std::string& filename);

	uint8_t CPURead(uint16_t address);
	void CPUWrite(uint16_t address, uint8_t value);

private:
	std::unique_ptr<Mapper> mapper;

	std::vector<uint8_t> prg_data;
	std::vector<uint8_t> chr_data;
};