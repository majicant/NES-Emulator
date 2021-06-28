#include "mapper.h"

Mapper::Mapper(uint8_t prg_banks, uint8_t chr_banks, uint8_t mirror)
	: prg_banks(prg_banks)
	, chr_banks(chr_banks)
	, mirror(mirror ? Mirror::VERTICAL : Mirror::HORIZONTAL)
{
}