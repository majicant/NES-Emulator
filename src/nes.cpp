#include "nes.h"

NES::NES(const std::string& filename)
	: nes_cartridge(std::make_unique<Cartridge>(filename))
	, nes_bus(std::make_unique<Bus>(*nes_cartridge))
	, nes_cpu(std::make_unique<CPU>(*nes_bus))
	, nes_ppu(std::make_unique<PPU>(*nes_bus))
{
	nes_bus->ConnectPPU(nes_ppu.get());
}

void NES::Run()
{
	while (true) {
		unsigned ppu_cycles = nes_cpu->ExecuteInstruction(nes_ppu->CheckNMI());
		for (unsigned i = 0; i < ppu_cycles; i++) {
			nes_ppu->Step();
		}
	}
}