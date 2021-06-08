#include <iostream>
#include <fstream>

#include "cpu.h"

CPU::CPU(Bus& bus)
	: bus(bus)
	, PC(ReadWord(0xFFFC))
{
}

unsigned CPU::ExecuteInstruction(bool handle_nmi)
{
	if (handle_nmi)
		return NMI();
	uint8_t opcode = FetchOpcode();
	SetOperand(instruction_table[opcode].address_mode);
	unsigned cycles = (this->*instruction_table[opcode].instruction)() + instruction_table[opcode].cycles;
	total_cycles += cycles;
	return cycles;
}

void CPU::SetOperand(AddressingMode address_mode)
{
	page_cross = false;
	switch (address_mode) {
	case AddressingMode::ACC:
		operand = UINT32_MAX;
		return;
	case AddressingMode::ABS:
		operand = ReadWord(PC);
		PC += 2;
		break;
	case AddressingMode::ABX:
		operand = ReadWord(PC);
		page_cross = ((operand & 0xFF00) != ((operand + X) & 0xFF00));
		operand += X;
		PC += 2;
		break;
	case AddressingMode::ABY:
		operand = ReadWord(PC);
		page_cross = ((operand & 0xFF00) != ((operand + Y) & 0xFF00));
		operand += Y;
		PC += 2;
		break;
	case AddressingMode::IMM:
		operand = PC;
		PC++;
		break;
	case AddressingMode::IMP:
		break;
	case AddressingMode::IND:
		// Implementation of JMP indirect hardware bug (when indirect vector falls on a page boundary)
		operand = ReadWord(PC);
		if ((operand & 0x00FF) == 0x00FF)
			operand = (static_cast<uint16_t>(bus.CPURead(operand & 0xFF00)) << 8) | static_cast<uint16_t>(bus.CPURead(operand));
		else
			operand = ReadWord(operand);
		PC += 2;
		break;
	case AddressingMode::IDX:
		operand = bus.CPURead(PC);
		operand = (static_cast<uint16_t>(bus.CPURead((operand + X + 1) & 0xFF)) << 8) | static_cast<uint16_t>(bus.CPURead((operand + X) & 0xFF));
		PC++;
		break;
	case AddressingMode::IDY:
		operand = bus.CPURead(PC);
		operand = (static_cast<uint16_t>(bus.CPURead((operand + 1) & 0xFF)) << 8) | static_cast<uint16_t>(bus.CPURead(operand));
		page_cross = ((operand & 0xFF00) != ((operand + Y) & 0xFF00));
		operand += Y;
		PC++;
		break;
	case AddressingMode::REL:
		page_cross = (((PC + 1) & 0xFF00) != (((PC + 1) + static_cast<int8_t>(bus.CPURead(PC))) & 0xFF00));
		operand = PC;
		PC++;
		break;
	case AddressingMode::ZPG:
		operand = bus.CPURead(PC);
		PC++;
		break;
	case AddressingMode::ZPX:
		operand = (bus.CPURead(PC) + X) & 0xFF;
		PC++;
		break;
	case AddressingMode::ZPY:
		operand = (bus.CPURead(PC) + Y) & 0xFF;
		PC++;
		break;
	default:
		std::cerr << "Unknown addressing mode!" << std::endl;
		break;
	}
	operand &= 0xFFFF;
}

unsigned CPU::NMI()
{
	SP--;
	WriteWord(0x100 + SP, PC);
	SP--;
	bus.CPUWrite(0x100 + SP, (SR | 0x20) & 0xEF);
	SP--;
	PC = ReadWord(0xFFFA);
	SetSRFlag(SRFlag::I, true);
	return 7;
}

unsigned CPU::ADC()
{
	uint16_t result = A + GetOperandData() + IsSet(SRFlag::C);
	SetSRFlag(SRFlag::C, result >> 8);
	SetSRFlag(SRFlag::V, (~(A ^ GetOperandData()) & (A ^ result) & 0x80));
	A = result & 0x00FF;
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::AND()
{
	A &= GetOperandData();
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::ASL()
{
	uint8_t op_data = GetOperandData();
	SetSRFlag(SRFlag::C, op_data & 0x80);
	op_data <<= 1;
	SetSRFlag(SRFlag::N, op_data & 0x80);
	SetOperandData(op_data);
	SetSRFlag(SRFlag::Z, !A);
	return 0;
}

unsigned CPU::BCC()
{
	if (!IsSet(SRFlag::C)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BCS()
{
	if (IsSet(SRFlag::C)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BEQ()
{
	if (IsSet(SRFlag::Z)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BIT()
{
	SetSRFlag(SRFlag::Z, !(A & GetOperandData()));
	SetSRFlag(SRFlag::V, GetOperandData() & 0x40);
	SetSRFlag(SRFlag::N, GetOperandData() & 0x80);
	return 0;
}

unsigned CPU::BMI()
{
	if (IsSet(SRFlag::N)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BNE()
{
	if (!IsSet(SRFlag::Z)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BPL()
{
	if (!IsSet(SRFlag::N)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BRK()
{
	SP--;
	WriteWord(0x100 + SP, PC);
	SP--;
	bus.CPUWrite(0x100 + SP, SR | 0x30);
	SP--;
	PC = ReadWord(0xFFFE);
	SetSRFlag(SRFlag::I, true);
	return 0;
}

unsigned CPU::BVC()
{
	if (!IsSet(SRFlag::V)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::BVS()
{
	if (IsSet(SRFlag::V)) {
		PC += static_cast<int8_t>(GetOperandData());
		return page_cross ? 2 : 1;
	}
	return 0;
}

unsigned CPU::CLC()
{
	SetSRFlag(SRFlag::C, false);
	return 0;
}

unsigned CPU::CLD()
{
	SetSRFlag(SRFlag::D, false);
	return 0;
}

unsigned CPU::CLI()
{
	SetSRFlag(SRFlag::I, false);
	return 0;
}

unsigned CPU::CLV()
{
	SetSRFlag(SRFlag::V, false);
	return 0;
}

unsigned CPU::CMP()
{
	uint8_t result = A - GetOperandData();
	SetSRFlag(SRFlag::C, A >= GetOperandData());
	SetSRFlag(SRFlag::Z, A == GetOperandData());
	SetSRFlag(SRFlag::N, result & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::CPX()
{
	uint8_t result = X - GetOperandData();
	SetSRFlag(SRFlag::C, X >= GetOperandData());
	SetSRFlag(SRFlag::Z, X == GetOperandData());
	SetSRFlag(SRFlag::N, result & 0x80);
	return 0;
}

unsigned CPU::CPY()
{
	uint8_t result = Y - GetOperandData();
	SetSRFlag(SRFlag::C, Y >= GetOperandData());
	SetSRFlag(SRFlag::Z, Y == GetOperandData());
	SetSRFlag(SRFlag::N, result & 0x80);
	return 0;
}

unsigned CPU::DEC()
{
	uint8_t op_data = GetOperandData();
	op_data--;
	SetSRFlag(SRFlag::Z, !op_data);
	SetSRFlag(SRFlag::N, op_data & 0x80);
	SetOperandData(op_data);
	return 0;
}

unsigned CPU::DEX()
{
	X--;
	SetSRFlag(SRFlag::Z, !X);
	SetSRFlag(SRFlag::N, X & 0x80);
	return 0;
}

unsigned CPU::DEY()
{
	Y--;
	SetSRFlag(SRFlag::Z, !Y);
	SetSRFlag(SRFlag::N, Y & 0x80);
	return 0;
}

unsigned CPU::EOR()
{
	A ^= GetOperandData();
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::INC()
{
	uint8_t op_data = GetOperandData();
	op_data++;
	SetSRFlag(SRFlag::Z, !op_data);
	SetSRFlag(SRFlag::N, op_data & 0x80);
	SetOperandData(op_data);
	return 0;
}

unsigned CPU::INX()
{
	X++;
	SetSRFlag(SRFlag::Z, !X);
	SetSRFlag(SRFlag::N, X & 0x80);
	return 0;
}

unsigned CPU::INY()
{
	Y++;
	SetSRFlag(SRFlag::Z, !Y);
	SetSRFlag(SRFlag::N, Y & 0x80);
	return 0;
}

unsigned CPU::JMP()
{
	PC = operand;
	return 0;
}

unsigned CPU::JSR()
{
	SP--;
	WriteWord(0x100 + SP, PC - 1);
	SP--;
	PC = operand;
	return 0;
}

unsigned CPU::LDA()
{
	A = GetOperandData();
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::LDX()
{
	X = GetOperandData();
	SetSRFlag(SRFlag::Z, !X);
	SetSRFlag(SRFlag::N, X & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::LDY()
{
	Y = GetOperandData();
	SetSRFlag(SRFlag::Z, !Y);
	SetSRFlag(SRFlag::N, Y & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::LSR()
{
	uint8_t op_data = GetOperandData();
	SetSRFlag(SRFlag::C, op_data & 0x01);
	op_data >>= 1;
	SetSRFlag(SRFlag::Z, !op_data);
	SetSRFlag(SRFlag::N, false);
	SetOperandData(op_data);
	return 0;
}

unsigned CPU::NOP()
{
	return page_cross ? 1 : 0;
}

unsigned CPU::ORA()
{
	A |= GetOperandData();
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::PHA()
{
	bus.CPUWrite(0x100 + SP, A);
	SP--;
	return 0;
}

unsigned CPU::PHP()
{
	bus.CPUWrite(0x100 + SP, SR | 0x30);
	SP--;
	return 0;
}

unsigned CPU::PLA()
{
	SP++;
	A = bus.CPURead(0x100 + SP);
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return 0;
}

unsigned CPU::PLP()
{
	SP++;
	SR = bus.CPURead(0x100 + SP);
	SetSRFlag(SRFlag::U, true);
	SetSRFlag(SRFlag::B, false);
	return 0;
}

unsigned CPU::ROL()
{
	uint8_t op_data = GetOperandData();
	bool new_carry = op_data & 0x80;
	op_data <<= 1;
	if (IsSet(SRFlag::C))
		op_data |= 0x01;
	SetSRFlag(SRFlag::C, new_carry);
	SetSRFlag(SRFlag::N, op_data & 0x80);
	SetOperandData(op_data);
	SetSRFlag(SRFlag::Z, !A);
	return 0;
}

unsigned CPU::ROR()
{
	uint8_t op_data = GetOperandData();
	bool new_carry = op_data & 0x01;
	op_data >>= 1;
	if (IsSet(SRFlag::C))
		op_data |= 0x80;
	SetSRFlag(SRFlag::C, new_carry);
	SetSRFlag(SRFlag::N, op_data & 0x80);
	SetOperandData(op_data);
	SetSRFlag(SRFlag::Z, !A);
	return 0;
}

unsigned CPU::RTI()
{
	SP++;
	SR = bus.CPURead(0x100 + SP);
	SP++;
	PC = ReadWord(0x100 + SP);
	SP++;
	SetSRFlag(SRFlag::U, true);
	SetSRFlag(SRFlag::B, false);
	return 0;
}

unsigned CPU::RTS()
{
	SP++;
	PC = ReadWord(0x100 + SP) + 1;
	SP++;
	return 0;
}

unsigned CPU::SBC()
{
	uint16_t result = A + ~GetOperandData() + IsSet(SRFlag::C);
	SetSRFlag(SRFlag::C, !(result >> 8));
	SetSRFlag(SRFlag::V, (~(A ^ ~GetOperandData()) & (A ^ result) & 0x80));
	A = result & 0x00FF;
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return page_cross ? 1 : 0;
}

unsigned CPU::SEC()
{
	SetSRFlag(SRFlag::C, true);
	return 0;
}

unsigned CPU::SED()
{
	SetSRFlag(SRFlag::D, true);
	return 0;
}

unsigned CPU::SEI()
{
	SetSRFlag(SRFlag::I, true);
	return 0;
}

unsigned CPU::STA()
{
	SetOperandData(A);
	return 0;
}

unsigned CPU::STX()
{
	SetOperandData(X);
	return 0;
}

unsigned CPU::STY()
{
	SetOperandData(Y);
	return 0;
}

unsigned CPU::TAX()
{
	X = A;
	SetSRFlag(SRFlag::Z, !X);
	SetSRFlag(SRFlag::N, X & 0x80);
	return 0;
}

unsigned CPU::TAY()
{
	Y = A;
	SetSRFlag(SRFlag::Z, !Y);
	SetSRFlag(SRFlag::N, Y & 0x80);
	return 0;
}

unsigned CPU::TSX()
{
	X = SP;
	SetSRFlag(SRFlag::Z, !X);
	SetSRFlag(SRFlag::N, X & 0x80);
	return 0;
}

unsigned CPU::TXA()
{
	A = X;
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return 0;
}

unsigned CPU::TXS()
{
	SP = X;
	return 0;
}

unsigned CPU::TYA()
{
	A = Y;
	SetSRFlag(SRFlag::Z, !A);
	SetSRFlag(SRFlag::N, A & 0x80);
	return 0;
}