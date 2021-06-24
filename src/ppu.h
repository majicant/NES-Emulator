#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "bus.h"
#include "sdl_engine.h"

class PPU
{
public:
	PPU(Bus& bus, SDLEngine& engine);

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);
	bool CheckNMI();

	void Step();

private:
	void FetchBackground();
	void IncrementCoarseX();
	void IncrementY();
	void ShiftBackgroundRegisters();
	void LoadBackgroundRegisters();

	void FetchSprites();
	void FetchTileData();
	bool SpriteZeroHitIsPossible();

	void UpdateFramebuffer();

	static const std::vector<std::vector<uint8_t>> palettes;

	uint8_t PPUCTRL = 0x00;		// $2000
	uint8_t PPUMASK = 0x00;		// $2001
	uint8_t PPUSTATUS = 0x00;	// $2002
	uint8_t OAMADDR = 0x00;		// $2003

	Bus& bus;

	SDLEngine& engine;
	std::vector<uint8_t> framebuffer;

	std::vector<uint8_t> primary_oam;
	std::vector<uint8_t> secondary_oam;

	struct Background
	{
		uint16_t v_addr = 0x0000;
		uint16_t t_addr = 0x0000;
		uint8_t fine_x = 0x00;
		bool first_write = false;

		uint8_t nt_byte = 0x00;
		uint8_t at_byte = 0x00;
		uint8_t pt_byte_low = 0x00;
		uint8_t pt_byte_high = 0x00;

		std::array<uint16_t, 2> pattern_shift = {};
		std::array<uint8_t, 2> palette_latch = {};
		std::array<uint8_t, 2> palette_shift = {};
	} bg_regs;

	struct Sprites
	{
		uint8_t sprites_found = 0x00;
		bool sprite_zero_found = false;

		std::array<uint8_t, 8> pattern_shift_lo = {};
		std::array<uint8_t, 8> pattern_shift_hi = {};
		std::array<uint8_t, 8> attribute_latch = {};
		std::array<uint8_t, 8> x_counters = {};
	} sprite_regs;

	uint8_t internal_data_buffer = 0x00;

	bool nmi_occured = false;
	bool nmi_output = false;

	unsigned cycles = 0;
	unsigned scanlines = 0;
};