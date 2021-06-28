#pragma once

#include <cstdint>

enum class Mirror
{
	ONE_SCREEN_LO,
	ONE_SCREEN_HI,
	VERTICAL,
	HORIZONTAL
};

class Mapper
{
public:
	Mapper(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror);
	virtual ~Mapper() = default;

	virtual uint32_t MapCPURead(uint16_t address) { return address; }
	virtual void MapCPUWrite(uint16_t address, uint8_t value) {};

	virtual uint32_t MapPPURead(uint16_t address) { return address; }
	virtual void MapPPUWrite(uint16_t address, uint8_t value) {}

	virtual uint8_t ReadPRGRAM(uint16_t address) { return 0x00; }
	virtual void WritePRGRAM(uint16_t address, uint8_t value) {}

	inline virtual Mirror GetMirror() { return mirror; }
	inline virtual bool HasCHRRam() { return chr_banks == 0; }

protected:
	uint8_t prg_banks;
	uint8_t chr_banks;
	Mirror mirror;
};