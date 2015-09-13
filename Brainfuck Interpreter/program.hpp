#pragma once

#include "memory.hpp"

#include <vector>
#include <stack>

class Instruction
{
	friend class ProgramData;

public:
	enum Type { Nop, MovePointer, Addition, Input, Output, LoopStart, LoopEnd, Reset, Multiplication, PushFast, Push, PopFast, Pop, Seek, Set, Stop };
	typedef int value_type;

	Instruction(Type, Instruction*);
	Instruction(Type, value_type = 0, value_type = 0);

	bool operator==(Type) const;

	static void SetParent(ProgramData*);
	static void Orphan(ProgramData*);

	Type Command;
	void(*FunctionPointer)(Instruction*);
	union
	{
		value_type Data[2];
		Instruction* Pointer;
	};

	static ProgramData* Parent;

	static const std::array<void(*)(Instruction*), 16> FunctionPointers;
};

class ProgramData
{
	std::vector<Instruction> Text;
	Memory Cells;

public:
	explicit ProgramData(const std::string& Source);
	~ProgramData();

	void Run();

	std::vector<Instruction>::pointer InstructionPointer;
	Memory::iterator DataPointer = std::begin(Cells);

	std::stack<Instruction*> JumpTable{ };
	std::vector<Memory::cell_type> Storage;
	char FastStorage = 0;

	void Parse(const std::string& Source);

private:
	bool AttemptReset(Instruction* Begin, Instruction* End);
	bool AttemptSeek(Instruction* Begin, Instruction* End);
	bool AttemptMultiplication(Instruction* Begin, Instruction* End);
	bool DropEmptyLoop(Instruction* Begin, Instruction* End);
};

inline bool operator==(Instruction::Type x, const Instruction& y);
inline bool operator!=(const Instruction& x, Instruction::Type y);
inline bool operator!=(Instruction::Type x, const Instruction& y);
