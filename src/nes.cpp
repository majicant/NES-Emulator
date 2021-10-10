#include "nes.h"

NES::NES(const std::filesystem::path& filename)
	: nes_cartridge(std::make_unique<Cartridge>(filename))
	, nes_engine(std::make_unique<SDLEngine>())
	, nes_controller(std::make_unique<Controller>(*nes_engine))
	, nes_bus(std::make_unique<Bus>(*nes_cartridge, *nes_controller))
	, nes_cpu(std::make_unique<CPU>(*nes_bus))
	, nes_ppu(std::make_unique<PPU>(*nes_bus, *nes_engine))
{
	nes_bus->ConnectCPU(nes_cpu.get());
	nes_bus->ConnectPPU(nes_ppu.get());
}

void NES::Run()
{
	unsigned ppu_cycles;
	while (!nes_engine->GetExitFlag()) {
		ppu_cycles = nes_ppu->CheckNMI() ? nes_cpu->HandleNMI() : nes_cpu->CheckOAMDMA() ? nes_cpu->HandleOAMDMA() : nes_cpu->ExecuteInstruction();
		for (unsigned i = 0; i < ppu_cycles * 3; i++)
			nes_ppu->Step();
	}
}