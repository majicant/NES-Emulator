#include <algorithm>

#include "ppu.h"

PPU::PPU(Bus& bus, SDLEngine& engine)
	: bus(bus)
	, engine(engine)
	, framebuffer(256 * 240 * 3)
	, primary_oam(256)
	, secondary_oam(32)
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
		data = primary_oam[OAMADDR];
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
		OAMADDR = value;
		break;
	case 4:		// OAMDATA
		primary_oam[OAMADDR++] = value;
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
		FetchSprites();
		if (cycles >= 1 && cycles <= 256)
			UpdateFramebuffer();
	}
	else if (scanlines == 241 && cycles == 1) {
		// Vertical blanking
		PPUSTATUS |= 0x80;
		nmi_occured = true;
		engine.UpdateDisplay(framebuffer.data());
		engine.UpdateExitFlag();	// Check if the application is being closed once per frame.
	}
	else if (scanlines == 261) {
		// Pre-render line
		FetchBackground();
		FetchTileData();
		if (cycles == 1) {
			PPUSTATUS &= 0x1F;
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
		ShiftBackgroundRegisters();
		switch (cycles % 8) {
		case 1:
			LoadBackgroundRegisters();
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
			if (PPUMASK & 0x18)	// Inc hori(v)
				IncrementCoarseX();
			break;
		}
	}
	if (PPUMASK & 0x18) {
		if (cycles == 256)	// Inc vert(v)
			IncrementY();

		if (cycles == 257)	// hori(v) = hori(t)
			bg_regs.v_addr = (bg_regs.v_addr & 0xFBE0) | (bg_regs.t_addr & 0x041F);

		if (scanlines == 261 && cycles >= 280 && cycles <= 304)	// vert(v) = vert(t)
			bg_regs.v_addr = (bg_regs.v_addr & 0x841F) | (bg_regs.t_addr & 0x7BE0);
	}
	if (cycles == 337 || cycles == 339)	// Unused NT fetches
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

void PPU::ShiftBackgroundRegisters()
{
	bg_regs.pattern_shift[0] <<= 1;
	bg_regs.pattern_shift[1] <<= 1;
	bg_regs.palette_shift[0] <<= 1;
	bg_regs.palette_shift[0] |= bg_regs.palette_latch[0];
	bg_regs.palette_shift[1] <<= 1;
	bg_regs.palette_shift[1] |= bg_regs.palette_latch[1];
}

void PPU::LoadBackgroundRegisters()
{
	bg_regs.palette_latch[0] = (bg_regs.at_byte & 0x01);
	bg_regs.palette_latch[1] = (bg_regs.at_byte >> 1);
	bg_regs.pattern_shift[0] = (bg_regs.pattern_shift[0] & 0xFF00) | bg_regs.pt_byte_low;
	bg_regs.pattern_shift[1] = (bg_regs.pattern_shift[1] & 0xFF00) | bg_regs.pt_byte_high;
}

void PPU::FetchSprites()
{
	if (cycles >= 2 && cycles <= 256) {
		for (int i = 0; i < sprite_regs.sprites_found; i++) {
			if (sprite_regs.x_counters[i] > 0)
				sprite_regs.x_counters[i]--;
			else {
				if (sprite_regs.attribute_latch[i] & 0x40) {	// Flip horizontally
					sprite_regs.pattern_shift_lo[i] >>= 1;
					sprite_regs.pattern_shift_hi[i] >>= 1;
				}
				else {
					sprite_regs.pattern_shift_lo[i] <<= 1;
					sprite_regs.pattern_shift_hi[i] <<= 1;
				}
			}
		}
	}
	if (cycles == 257) {
		// Sprite evaluation
		std::fill(secondary_oam.begin(), secondary_oam.end(), 0xFF);
		sprite_regs.sprites_found = 0;
		sprite_regs.sprite_zero_found = false;
		for (int n = 0, m = 0; n < 64 && sprite_regs.sprites_found < 9; n++) {
			uint8_t y_coordinate = primary_oam[4 * n + m];	// m will always be 0 unless we are searching for sprite overflow
			if (sprite_regs.sprites_found == 8)
				m = (m + 1) % 4;	// Sprite overflow bug
			else
				secondary_oam[sprite_regs.sprites_found * 4] = y_coordinate;
			bool in_range = ((int16_t)scanlines - (int16_t)y_coordinate) >= 0 && ((int16_t)scanlines - (int16_t)y_coordinate) < ((PPUCTRL & 0x20) ? 16 : 8);
			if (in_range) {
				if (n == 0)	// Sprite zero hit
					sprite_regs.sprite_zero_found = true;
				if (sprite_regs.sprites_found == 8)	// Sprite overflow has occurred
					PPUSTATUS |= 0x20;
				else
					for (int i = 1; i < 4; i++)
						secondary_oam[sprite_regs.sprites_found * 4 + i] = primary_oam[4 * n + i];
				sprite_regs.sprites_found++;
			}
		}
		if (sprite_regs.sprites_found == 9)
			sprite_regs.sprites_found = 8;
		// Sprite fetches
		FetchTileData();
	}
}

void PPU::FetchTileData()
{
	if (cycles == 257) {
		for (int i = 0; i < sprite_regs.sprites_found; i++) {
			uint8_t sprite_size = (PPUCTRL & 0x20) ? 16 : 8;
			uint8_t sprite_row = scanlines - secondary_oam[4 * i];
			uint8_t tile_index = secondary_oam[4 * i + 1];

			uint16_t tile_address;
			if (sprite_size == 8) {
				if (sprite_regs.attribute_latch[i] & 0x80)	// Flip vertically
					sprite_row = 7 - sprite_row;
				tile_address = ((PPUCTRL & 0x08) ? 0x1000 : 0x0000) + (tile_index * 16) + sprite_row;
			}
			else {
				if (sprite_regs.attribute_latch[i] & 0x80)	// Flip vertically
					sprite_row = 7 - (sprite_row & 0x07);
				tile_address = ((tile_index & 0x01) ? 0x1000 : 0x0000) + ((tile_index & 0xFE) * 16) + (sprite_row & 0x07);
				if (sprite_row >= 8)
					tile_address += 16;
			}

			sprite_regs.pattern_shift_lo[i] = bus.PPURead(tile_address);
			sprite_regs.pattern_shift_hi[i] = bus.PPURead(tile_address + 8);
			sprite_regs.attribute_latch[i] = secondary_oam[4 * i + 2];
			sprite_regs.x_counters[i] = secondary_oam[4 * i + 3];
		}
	}
}

bool PPU::SpriteZeroHitIsPossible()
{
	bool rendering_enabled = ((PPUMASK & 0x18) == 0x18);
	bool show_leftside_sprites = ((cycles >= 9) && ((PPUMASK & 0x06) == 0x06));
	bool not_last_cycle = (cycles != 256);
	bool szh_not_detected = ((PPUSTATUS & 0x40) == 0);
	return sprite_regs.sprite_zero_found && rendering_enabled && show_leftside_sprites && not_last_cycle && szh_not_detected;
}

void PPU::UpdateFramebuffer()
{
	// Background pixel
	uint8_t pat_lo = (bg_regs.pattern_shift[0] & (0x8000 >> bg_regs.fine_x)) > 0;
	uint8_t pat_hi = (bg_regs.pattern_shift[1] & (0x8000 >> bg_regs.fine_x)) > 0;

	uint8_t pixel = (pat_hi << 1) | pat_lo;

	uint8_t pal_lo = (bg_regs.palette_shift[0] & (0x80 >> bg_regs.fine_x)) > 0;
	uint8_t pal_hi = (bg_regs.palette_shift[1] & (0x80 >> bg_regs.fine_x)) > 0;

	uint8_t palette = ((pal_hi << 1) | pal_lo) << 2;
	if (pixel == 0)
		palette = 0;

	// Sprite pixel
	for (int i = 0; i < sprite_regs.sprites_found; i++) {
		if (sprite_regs.x_counters[i] == 0) {
			uint8_t bit_select = (sprite_regs.attribute_latch[i] & 0x40) ? 0x01 : 0x80;	// Flip horizontally
			uint8_t sprite_pat_lo = (sprite_regs.pattern_shift_lo[i] & bit_select) > 0;
			uint8_t sprite_pat_hi = (sprite_regs.pattern_shift_hi[i] & bit_select) > 0;

			uint8_t sprite_pixel = (sprite_pat_hi << 1) | sprite_pat_lo;
			uint8_t sprite_palette = ((sprite_regs.attribute_latch[i] & 0x03) + 0x04) << 2;

			if (pixel == 0 && sprite_pixel > 0) {
				pixel = sprite_pixel;
				palette = sprite_palette;
				break;
			}
			else if (pixel > 0 && sprite_pixel > 0) {
				if (i == 0 && SpriteZeroHitIsPossible())	// Sprite zero hit
					PPUSTATUS |= 0x40;
				if ((sprite_regs.attribute_latch[i] & 0x20) == 0) {
					pixel = sprite_pixel;
					palette = sprite_palette;
					break;
				}
			}
		}
	}

	framebuffer[scanlines * 256 * 3 + (cycles - 1) * 3] = palettes[bus.PPURead(0x3F00 + palette + pixel)][0];
	framebuffer[scanlines * 256 * 3 + ((cycles - 1) * 3) + 1] = palettes[bus.PPURead(0x3F00 + palette + pixel)][1];
	framebuffer[scanlines * 256 * 3 + ((cycles - 1) * 3) + 2] = palettes[bus.PPURead(0x3F00 + palette + pixel)][2];
}