#include "bus.h"
#include "ppu.h"

Bus::Bus(Cartridge& cart)
	: cartridge(cart)
	, ram(2048)
	, vram(2048)
	, palette_ram(32)
{
}

uint8_t Bus::CPURead(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		return ram[address & 0x07FF];
	else if (address >= 0x2000 && address <= 0x3FFF)
		return ppu->Read(address & 0x0007);
	else if (address >= 0x8000 && address <= 0xFFFF)
		return cartridge.CPURead(address);
	return 0x00;
}

void Bus::CPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		ram[address & 0x07FF] = value;
	else if (address >= 0x2000 && address <= 0x3FFF)
		ppu->Write(address & 0x0007, value);
	else if (address >= 0x8000 && address <= 0xFFFF)
		cartridge.CPUWrite(address, value);
}

uint8_t Bus::PPURead(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		return cartridge.PPURead(address);
	else if (address >= 0x2000 && address <= 0x3EFF) {
		if (cartridge.GetMapper()->GetMirror()) {
			// vertical (horizontal arrangement)
			return vram[address & 0x07FF];
		}
		else {
			// horizontal (vertical arrangement)
			address &= 0x0FFF;
			if (address <= 0x07FF)
				return vram[address & 0x03FF];
			else
				return vram[(address & 0x03FF) + 0x0400];
		}
	}
	else if (address >= 0x3F00 && address <= 0x3FFF)
		return palette_ram[address & 0x001F];
	return 0x00;
}

void Bus::PPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x0000 && address <= 0x1FFF)
		cartridge.PPUWrite(address, value);
	else if (address >= 0x2000 && address <= 0x3EFF) {
		if (cartridge.GetMapper()->GetMirror()) {
			// vertical (horizontal arrangement)
			vram[address & 0x07FF] = value;
		}
		else {
			// horizontal (vertical arrangement)
			address &= 0x0FFF;
			if (address <= 0x07FF)
				vram[address & 0x03FF] = value;
			else
				vram[(address & 0x03FF) + 0x0400] = value;
		}
	}
	else if (address >= 0x3F00 && address <= 0x3FFF)
		palette_ram[address & 0x001F] = value;
}

void Bus::ConnectPPU(PPU* ppu_ptr)
{
	ppu = ppu_ptr;
}