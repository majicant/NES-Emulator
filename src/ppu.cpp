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
		bg_regs.first_write = true;
		break;
	case 4:		// OAMDATA
		break;
	case 7:		// PPUDATA
		if (bg_regs.v_addr <= 0x3EFF) {
			data = internal_data_buffer;
			internal_data_buffer = bus.PPURead(bg_regs.v_addr);
		}
		else {
			data = bus.PPURead(bg_regs.v_addr);
			internal_data_buffer = data;
		}
		bg_regs.v_addr += (PPUCTRL & 0x04) ? 32 : 1;
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
		bg_regs.t_addr = (bg_regs.t_addr & 0xF3FF) | ((value & 0x03) << 10);
		break;
	case 1:		// PPUMASK
		PPUMASK = value;
		break;
	case 3:		// OAMADDR
		break;
	case 4:		// OAMDATA
		break;
	case 5:		// PPUSCROLL
		if (bg_regs.first_write) {
			bg_regs.t_addr = (bg_regs.t_addr & 0xFFE0) | (value >> 3);
			bg_regs.fine_x = value & 0x07;
		}
		else
			bg_regs.t_addr = (bg_regs.t_addr & 0x8C1F) | (static_cast<uint16_t>(value & 0x07) << 12) | (static_cast<uint16_t>(value & 0xF8) << 2);
		bg_regs.first_write = !bg_regs.first_write;
		break;
	case 6:		// PPUADDR
		if (bg_regs.first_write)
			bg_regs.t_addr = (bg_regs.t_addr & 0x80FF) | (static_cast<uint16_t>(value & 0x3F) << 8);
		else {
			bg_regs.t_addr = (bg_regs.t_addr & 0xFF00) | value;
			bg_regs.v_addr = bg_regs.t_addr;
		}
		bg_regs.first_write = !bg_regs.first_write;
		break;
	case 7:		// PPUDATA
		bus.PPUWrite(bg_regs.v_addr, value);
		bg_regs.v_addr += (PPUCTRL & 0x04) ? 32 : 1;
		break;
	default:
		std::cerr << "Invalid write address!" << std::endl;
		break;
	}
}

bool PPU::CheckNMI()
{
	bool trigger_nmi = (nmi_output && nmi_occured);
	nmi_occured = false;
	return trigger_nmi;
}

void PPU::Step()
{
	if (scanlines >= 0 && scanlines <= 239) {
		// Visible scanlines
		FetchBackground();
		if (cycles >= 1 && cycles <= 256)
			UpdateFramebuffer();
	}
	else if (scanlines == 241 && cycles == 1) {
		// Vertical blanking
		PPUSTATUS |= 0x80;
		nmi_occured = true;
		engine->UpdateDisplay(framebuffer.data());
	}
	else if (scanlines == 261) {
		// Pre-render line
		FetchBackground();
		if (cycles == 1) {
			PPUSTATUS &= 0x7F;
			nmi_occured = false;
		}
	}
	cycles++;
	if (cycles == 341) {
		cycles = 0;
		scanlines++;
		if (scanlines == 262)
			scanlines = 0;
	}
}

void PPU::FetchBackground()
{
	if ((cycles >= 1 && cycles <= 256) || (cycles >= 321 && cycles <= 336)) {
		ShiftRegisters();
		switch (cycles % 8) {
		case 1:
			LoadRegisters();
			bg_regs.nt_byte = bus.PPURead(0x2000 | (bg_regs.v_addr & 0x0FFF));
			break;
		case 3:
			bg_regs.at_byte = bus.PPURead(0x23C0 | (bg_regs.v_addr & 0x0C00) | ((bg_regs.v_addr >> 4) & 0x38) | ((bg_regs.v_addr >> 2) & 0x07));
			if (bg_regs.v_addr & 0x02)
				bg_regs.at_byte >>= 2;
			if (bg_regs.v_addr & 0x40)
				bg_regs.at_byte >>= 4;
			bg_regs.at_byte &= 0x03;
			break;
		case 5:
			bg_regs.pt_byte_low = bus.PPURead(((PPUCTRL & 0x10) ? 0x1000 : 0x0000) + (bg_regs.nt_byte * 16) + ((bg_regs.v_addr >> 12) & 0x07));
			break;
		case 7:
			bg_regs.pt_byte_high = bus.PPURead(((PPUCTRL & 0x10) ? 0x1000 : 0x0000) + (bg_regs.nt_byte * 16) + ((bg_regs.v_addr >> 12) & 0x07) + 8);
			break;
		case 0:
			if (PPUMASK & 0x18)
				IncrementCoarseX();
			break;
		}
	}
	if (PPUMASK & 0x18) {
		if (cycles == 256)
			IncrementY();

		if (cycles == 257)
			bg_regs.v_addr = (bg_regs.v_addr & 0xFBE0) | (bg_regs.t_addr & 0x041F);

		if (scanlines == 261 && cycles >= 280 && cycles <= 304)
			bg_regs.v_addr = (bg_regs.v_addr & 0x841F) | (bg_regs.t_addr & 0x7BE0);
	}
	if (cycles == 337 || cycles == 339)
		bg_regs.nt_byte = bus.PPURead(0x2000 | bg_regs.v_addr & 0x0FFF);
}

void PPU::IncrementCoarseX()
{
	if ((bg_regs.v_addr & 0x001F) == 31) {
		bg_regs.v_addr &= 0xFFE0;
		bg_regs.v_addr ^= 0x0400;
	}
	else
		bg_regs.v_addr += 1;
}

void PPU::IncrementY()
{
	if ((bg_regs.v_addr & 0x7000) != 0x7000)
		bg_regs.v_addr += 0x1000;
	else {
		bg_regs.v_addr &= 0x8FFF;
		uint8_t coarse_y = (bg_regs.v_addr & 0x03E0) >> 5;
		if (coarse_y == 29) {
			coarse_y = 0;
			bg_regs.v_addr ^= 0x0800;
		}
		else if (coarse_y == 31)
			coarse_y = 0;
		else
			coarse_y += 1;
		bg_regs.v_addr = (bg_regs.v_addr & 0xFC1F) | (coarse_y << 5);
	}
}

void PPU::ShiftRegisters()
{
	bg_regs.pattern_shift[0] <<= 1;
	bg_regs.pattern_shift[1] <<= 1;
	bg_regs.palette_shift[0] <<= 1;
	bg_regs.palette_shift[0] |= bg_regs.palette_latch[0];
	bg_regs.palette_shift[1] <<= 1;
	bg_regs.palette_shift[1] |= bg_regs.palette_latch[1];
}

void PPU::LoadRegisters()
{
	bg_regs.palette_latch[0] = (bg_regs.at_byte & 0x01);
	bg_regs.palette_latch[1] = (bg_regs.at_byte >> 1);
	bg_regs.pattern_shift[0] = (bg_regs.pattern_shift[0] & 0xFF00) | bg_regs.pt_byte_low;
	bg_regs.pattern_shift[1] = (bg_regs.pattern_shift[1] & 0xFF00) | bg_regs.pt_byte_high;
}

void PPU::UpdateFramebuffer()
{
	uint8_t pat_lo = (bg_regs.pattern_shift[0] & (0x8000 >> bg_regs.fine_x)) > 0;
	uint8_t pat_hi = (bg_regs.pattern_shift[1] & (0x8000 >> bg_regs.fine_x)) > 0;

	uint8_t pixel = (pat_hi << 1) | pat_lo;

	uint8_t pal_lo = (bg_regs.palette_shift[0] & (0x80 >> bg_regs.fine_x)) > 0;
	uint8_t pal_hi = (bg_regs.palette_shift[1] & (0x80 >> bg_regs.fine_x)) > 0;

	uint8_t palette = ((pal_hi << 1) | pal_lo) << 2;

	framebuffer[scanlines * 256 * 3 + (cycles - 1) * 3] = palettes[bus.PPURead(0x3F00 + palette + pixel)][0];
	framebuffer[scanlines * 256 * 3 + ((cycles - 1) * 3) + 1] = palettes[bus.PPURead(0x3F00 + palette + pixel)][1];
	framebuffer[scanlines * 256 * 3 + ((cycles - 1) * 3) + 2] = palettes[bus.PPURead(0x3F00 + palette + pixel)][2];
}