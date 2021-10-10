#include "mmc3.h"

MMC3::MMC3(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror)
	: Mapper(prg_banks, chr_banks, mirror)
	, prg_ram(8192)
{
}

uint32_t MMC3::MapCPURead(uint16_t address)
{
	if (address >= 0x8000 && address <= 0x9FFF) {
		if (internal_regs.bank_select & 0x40) {
			// fix second last bank at $8000
			return ((prg_banks - 2) * 0x2000) + (address & 0x1FFF);
		}
		else {
			// select 8 KB PRG ROM bank at $8000 - $9FFF
			return (internal_regs.bank_data[6] * 0x2000) + (address & 0x1FFF);
		}
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		// select 8 KB PRG ROM bank at $A000 - $BFFF
		return (internal_regs.bank_data[7] * 0x2000) + (address & 0x1FFF);
	}
	else if (address >= 0xC000 && address <= 0xDFFF) {
		if (internal_regs.bank_select & 0x40) {
			// select 8 KB PRG ROM bank at $C000 - $DFFF
			return (internal_regs.bank_data[6] * 0x2000) + (address & 0x1FFF);
		}
		else {
			// fix second last bank at $8000
			return ((prg_banks - 2) * 0x2000) + (address & 0x1FFF);
		}
	}
	else if (address >= 0xE000 && address <= 0xFFFF) {
		// fix last bank at $E000
		return ((prg_banks - 1) * 0x2000) + (address & 0x1FFF);
	}
	return address;
}

void MMC3::MapCPUWrite(uint16_t address, uint8_t value)
{
	if (address >= 0x8000 && address <= 0x9FFF) {
		if ((address % 2) == 0)
			internal_regs.bank_select = value;
		else {
			uint8_t bank_register = internal_regs.bank_select & 0x07;
			switch (bank_register) {
			case 0:
			case 1:
				value &= 0xFE;
				break;
			case 6:
			case 7:
				value &= 0x3F;
				break;
			}
			// PRG banks are counted in 8 KB units
			// CHR banks are counted in 1 KB units
			internal_regs.bank_data[bank_register] = value;
		}
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if ((address % 2) == 0)
			mirror = (value & 0x01) ? Mirror::HORIZONTAL : Mirror::VERTICAL;
		else
			internal_regs.prg_ram_protect = value;
	}
	else if (address >= 0xC000 && address <= 0xDFFF) {
		if ((address % 2) == 0)
			internal_regs.irq_latch = value;
		else
			internal_regs.irq_reload = true;
	}
	else if (address >= 0xE000 && address <= 0xFFFF)
		internal_regs.irq_enabled = (address % 2) ? true : false;
}

uint32_t MMC3::MapPPURead(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x07FF) {
		if (internal_regs.bank_select & 0x80) {
			if (address >= 0x0000 && address <= 0x03FF) {
				// select 1 KB CHR bank at PPU $0000 - $03FF
				return (internal_regs.bank_data[2] * 0x0400) + (address & 0x03FF);
			}
			else {
				// select 1 KB CHR bank at PPU $0400 - $07FF
				return (internal_regs.bank_data[3] * 0x0400) + (address & 0x03FF);
			}
		}
		else {
			// select 2 KB CHR bank at PPU $0000 - $07FF
			return (internal_regs.bank_data[0] * 0x0400) + (address & 0x07FF);
		}
	}
	else if (address >= 0x0800 && address <= 0x0FFF) {
		if (internal_regs.bank_select & 0x80) {
			if (address >= 0x0800 && address <= 0x0BFF) {
				// select 1 KB CHR bank at PPU $0800 - $0BFF
				return (internal_regs.bank_data[4] * 0x0400) + (address & 0x03FF);
			}
			else {
				// select 1 KB CHR bank at PPU $0C00 - $0FFF
				return (internal_regs.bank_data[5] * 0x0400) + (address & 0x03FF);
			}
		}
		else {
			// select 2 KB CHR bank at PPU $0800 - $0FFF
			return (internal_regs.bank_data[1] * 0x0400) + (address & 0x07FF);
		}
	}
	else if (address >= 0x1000 && address <= 0x17FF) {
		if (internal_regs.bank_select & 0x80) {
			// select 2 KB CHR bank at PPU $1000 - $17FF
			return (internal_regs.bank_data[0] * 0x0400) + (address & 0x07FF);
		}
		else {
			if (address >= 0x1000 && address <= 0x13FF) {
				// select 1 KB CHR bank at PPU $1000 - $13FF
				return (internal_regs.bank_data[2] * 0x0400) + (address & 0x03FF);
			}
			else {
				// select 1 KB CHR bank at PPU $1400 - $17FF
				return (internal_regs.bank_data[3] * 0x0400) + (address & 0x03FF);
			}
		}
	}
	else if (address >= 0x1800 && address <= 0x1FFF) {
		if (internal_regs.bank_select & 0x80) {
			// select 2 KB CHR bank at PPU $1800 - $1FFF
			return (internal_regs.bank_data[1] * 0x0400) + (address & 0x07FF);
		}
		else {
			if (address >= 0x1800 && address <= 0x1BFF) {
				// select 1 KB CHR bank at PPU $1800 - $1BFF
				return (internal_regs.bank_data[4] * 0x0400) + (address & 0x03FF);
			}
			else {
				// select 1 KB CHR bank at PPU $1C00 - $1FFF
				return (internal_regs.bank_data[5] * 0x0400) + (address & 0x03FF);
			}
		}
	}
	return address;
}

uint8_t MMC3::ReadPRGRAM(uint16_t address)
{
	if (internal_regs.prg_ram_protect & 0x80)
		return prg_ram[address & 0x1FFF];
	return 0x00;
}

void MMC3::WritePRGRAM(uint16_t address, uint8_t value)
{
	if ((internal_regs.prg_ram_protect & 0x80) && ((internal_regs.prg_ram_protect & 0x40) == 0))
		prg_ram[address & 0x1FFF] = value;
}

bool MMC3::IRQ()
{
	if ((irq_counter == 0) || (internal_regs.irq_reload)) {
		irq_counter = internal_regs.irq_latch;
		internal_regs.irq_reload = false;
	}
	else
		irq_counter--;

	if ((irq_counter == 0) && internal_regs.irq_enabled)
		return true;
	return false;
}