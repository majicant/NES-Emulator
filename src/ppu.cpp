#include <iostream>
#include <vector>

#include "ppu.h"

PPU::PPU(Bus& bus)
	: bus(bus)
	, engine(std::make_unique<SDLEngine>())
	, framebuffer(256 * 240 * 3)
{
}

uint8_t PPU::Read(uint16_t address)
{
	uint8_t data = 0x00;
	switch (address) {
	case 2:		// PPUSTATUS
		data = PPUSTATUS;
		PPUSTATUS &= 0x7F;
		bg_registers.first_write = true;
		break;
	case 4:		// OAMDATA
		break;
	case 7:		// PPUDATA
		if (bg_registers.vram_address <= 0x3EFF) {
			data = internal_data_buffer;
			internal_data_buffer = bus.PPURead(bg_registers.vram_address);
		}
		else {
			data = bus.PPURead(bg_registers.vram_address);
			internal_data_buffer = data;
		}
		bg_registers.vram_address += (PPUCTRL & 0x04) ? 32 : 1;
		break;
	default:
		std::cerr << "Invalid read address!" << std::endl;
		break;
	}
	return data;
}

void PPU::Write(uint16_t address, uint8_t value)
{
	switch (address) {
	case 0:		// PPUCTRL
		PPUCTRL = value;
		nmi_output = PPUCTRL & 0x80;
		bg_registers.temp_address = (bg_registers.temp_address & 0xF3FF) | ((value & 0x03) << 10);
		break;
	case 1:		// PPUMASK
		PPUMASK = value;
		break;
	case 3:		// OAMADDR
		break;
	case 4:		// OAMDATA
		break;
	case 5:		// PPUSCROLL
		break;
	case 6:		// PPUADDR
		if (bg_registers.first_write)
			bg_registers.temp_address = static_cast<uint16_t>(value) << 8;
		else {
			bg_registers.temp_address |= static_cast<uint16_t>(value);
			bg_registers.vram_address = bg_registers.temp_address;
		}
		bg_registers.first_write = !bg_registers.first_write;
		break;
	case 7:		// PPUDATA
		bus.PPUWrite(bg_registers.vram_address, value);
		bg_registers.vram_address += (PPUCTRL & 0x04) ? 32 : 1;
		break;
	default:
		std::cerr << "Invalid write address!" << std::endl;
		break;
	}
}

void PPU::Step()
{
	if (scanlines >= 0 && scanlines <= 239) {
		// Visible scanlines
	}
	else if (scanlines == 241 && cycles == 1) {
		// Vertical blanking
		PPUSTATUS |= 0x80;
		nmi_occured = true;
		DrawNametables();
	}
	else if (scanlines == 261 && cycles == 1) {
		// Pre-render line
		PPUSTATUS &= 0x7F;
		nmi_occured = false;
		scanlines = 0;
	}

	cycles++;
	if (cycles == 341) {
		cycles = 0;
		scanlines++;
	}
}

void PPU::FetchBackground()
{
}

void PPU::DrawNametables()
{
	for (int row = 0; row < 30; row++) {
		for (int col = 0; col < 32; col++) {
			uint8_t nt_byte = bus.PPURead(0x2000 + col + row * 32);
			uint8_t at_byte = bus.PPURead(0x23C0 + (col / 4) + (row / 4) * 8);

			uint8_t shift = ((col % 4 <= 1) ? 0 : 2) + ((row % 4 <= 1) ? 0 : 4);
			at_byte = ((at_byte >> shift) & 0x03) * 4;

			for (int byte = 0; byte < 8; byte++) {
				uint16_t addr = ((PPUCTRL & 0x10) ? 0x1000 : 0x0000) + (nt_byte * 16) + byte;
				uint8_t pat_byte_low = bus.PPURead(addr);
				uint8_t pat_byte_high = bus.PPURead(addr + 8);

				for (int bit = 0; bit < 8; bit++) {
					uint8_t pixel = ((pat_byte_low >> (7 - bit)) & 0x01) + (((pat_byte_high >> (7 - bit)) & 0x01) * 2);

					framebuffer[((row * 8 + byte) * 256 * 3) + ((col * 8 + bit) * 3)] = palettes[bus.PPURead(0x3F00 + at_byte + pixel)][0];
					framebuffer[((row * 8 + byte) * 256 * 3) + ((col * 8 + bit) * 3) + 1] = palettes[bus.PPURead(0x3F00 + at_byte + pixel)][1];
					framebuffer[((row * 8 + byte) * 256 * 3) + ((col * 8 + bit) * 3) + 2] = palettes[bus.PPURead(0x3F00 + at_byte + pixel)][2];
				}
			}
		}
	}
	engine->UpdateDisplay(framebuffer.data());
}