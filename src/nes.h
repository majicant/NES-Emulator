#pragma once

#include <memory>
#include <string>

#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"

class NES
{
public:
	NES(const std::string& filename);
	
	void Run();

private:
	std::unique_ptr<Cartridge> nes_cartridge;
	std::unique_ptr<Bus> nes_bus;
	std::unique_ptr<CPU> nes_cpu;
	std::unique_ptr<PPU> nes_ppu;
};