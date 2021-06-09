#pragma once

#include <cstdint>
#include <memory>

#include "bus.h"
#include "sdl_engine.h"

class PPU
{
public:
	PPU(Bus& bus);

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);

	inline bool CheckNMI()
	{
		bool trigger_nmi = (nmi_output && nmi_occured);
		nmi_occured = false;
		return trigger_nmi;
	}

	void Step();

private:
	void FetchBackground();
	void DrawNametables();

	static const std::vector<std::vector<uint8_t>> palettes;

	uint8_t PPUCTRL = 0x00;		// $2000
	uint8_t PPUMASK = 0x00;		// $2001
	uint8_t PPUSTATUS = 0x00;	// $2002
	uint8_t OAMADDR = 0x00;		// $2003
	uint8_t OAMDATA = 0x00;		// $2004
	uint8_t PPUSCROLL = 0x00;	// $2005
	uint8_t PPUADDR = 0x00;		// $2006
	uint8_t PPUDATA = 0x00;		// $2007
	uint8_t OAMDMA = 0x00;		// $4014

	Bus& bus;

	std::unique_ptr<SDLEngine> engine;
	std::vector<uint8_t> framebuffer;

	struct BackgroundRegisters
	{
		uint16_t vram_address = 0x0000;
		uint16_t temp_address = 0x0000;
		uint8_t fine_x = 0x00;
		bool first_write = false;

		uint16_t pattern_shift[2] = {};
		uint8_t palette_shift[2] = {};
	} bg_registers;

	uint8_t internal_data_buffer = 0x00;

	bool nmi_occured = false;
	bool nmi_output = false;

	unsigned cycles = 0;
	unsigned scanlines = 0;
};