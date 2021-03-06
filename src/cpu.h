#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "bus.h"

class CPU
{
public:
	CPU(Bus& bus);

	unsigned ExecuteInstruction();
	unsigned HandleNMI();
	unsigned HandleIRQ();

	bool CheckOAMDMA();
	void TriggerOAMDMA(uint8_t page_num);
	unsigned HandleOAMDMA();

private:
	enum class AddressingMode
	{
		ACC,
		ABS,
		ABX,
		ABY,
		IMM,
		IMP,
		IND,
		IDX,
		IDY,
		REL,
		ZPG,
		ZPX,
		ZPY
	};

	enum class SRFlag
	{
		N = (1 << 7),	// Negative
		V = (1 << 6),	// Overflow
		U = (1 << 5),	// Unused
		B = (1 << 4),	// Break
		D = (1 << 3),	// Decimal
		I = (1 << 2),	// Interrupt
		Z = (1 << 1),	// Zero
		C = (1 << 0)	// Carry
	};

	unsigned ADC(); unsigned AND(); unsigned ASL(); unsigned BCC();
	unsigned BCS(); unsigned BEQ(); unsigned BIT(); unsigned BMI();
	unsigned BNE(); unsigned BPL(); unsigned BRK(); unsigned BVC();
	unsigned BVS(); unsigned CLC(); unsigned CLD(); unsigned CLI();
	unsigned CLV(); unsigned CMP(); unsigned CPX(); unsigned CPY();
	unsigned DEC(); unsigned DEX(); unsigned DEY(); unsigned EOR();
	unsigned INC(); unsigned INX(); unsigned INY(); unsigned JMP();

	unsigned JSR(); unsigned LDA(); unsigned LDX(); unsigned LDY();
	unsigned LSR(); unsigned NOP(); unsigned ORA(); unsigned PHA();
	unsigned PHP(); unsigned PLA(); unsigned PLP(); unsigned ROL();
	unsigned ROR(); unsigned RTI(); unsigned RTS(); unsigned SBC();
	unsigned SEC(); unsigned SED(); unsigned SEI(); unsigned STA();
	unsigned STX(); unsigned STY(); unsigned TAX(); unsigned TAY();
	unsigned TSX(); unsigned TXA(); unsigned TXS(); unsigned TYA();

	struct Instruction
	{
		std::string name;
		std::function<unsigned(CPU*)> instruction;
		AddressingMode address_mode;
		unsigned cycles;
	};

	uint8_t FetchOpcode();
	void SetOperand(AddressingMode address_mode);

	void SetSRFlag(SRFlag flag, bool bit);
	bool IsSet(SRFlag flag);

	uint8_t GetOperandData();
	void SetOperandData(uint8_t data);

	uint16_t ReadWord(uint16_t address);
	void WriteWord(uint16_t address, uint16_t value);

	static const std::vector<Instruction> instruction_table;

	Bus& bus;

	uint8_t A = 0x00;	// Accumulator
	uint8_t X = 0x00;	// X Register
	uint8_t Y = 0x00;	// Y Register
	uint16_t PC;		// Program Counter
	uint8_t SP = 0xFD;	// Stack Pointer
	uint8_t SR = 0x24;	// Status Register (NV-BDIZC)

	unsigned total_cycles = 0;
	uint32_t operand = UINT32_MAX;
	bool page_cross = false;

	bool oamdma_enable = false;
	uint8_t oamdma_page = 0x00;
};