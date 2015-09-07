#pragma once

#include "memory.hpp"

#include <vector>
#include <stack>

class ProgramData
{
	friend class Instruction;

public:
	explicit ProgramData(const std::string& Source);
	~ProgramData();

	void Run();

private:
	std::vector<Instruction> Text;
	std::vector<Instruction>::pointer InstructionPointer;

	Memory Cells;
	Memory::iterator DataPointer = std::begin(Cells);

	std::stack<Instruction*> JumpTable{ };

	void Parse(const std::string& Source);

	bool AttemptReset(Instruction* Begin, Instruction* End);
	bool AttemptSeek(Instruction* Begin, Instruction* End);
	bool AttemptMultiplication(Instruction* Begin, Instruction* End);
	bool DropEmptyLoop(Instruction* Begin, Instruction* End);
};

class Instruction
{
	friend ProgramData;

public:
	enum class Type { Nop, MovePointer, Addition, Input, Output, LoopStart, LoopEnd, Reset, Multiplication, Push, Pop, Seek, Set, Stop };
	typedef int value_type;

	Instruction(Type, Instruction*);
	Instruction(Type, value_type = 0, value_type = 0);

	Instruction& operator=(const Instruction& x)
	{
		Command = x.Command;
		if (x.Command == Type::LoopStart || x.Command == Type::LoopEnd)
			Pointer = x.Pointer;
		else
			std::copy(std::cbegin(x.Data), std::cend(x.Data), std::begin(Data));

		return *this;
	}

	void Execute() const;

	bool operator==(Type) const;

	static void SetParent(ProgramData*);
	static void Orphan(ProgramData*);

private:
	Type Command;
	union
	{
		value_type Data[2];
		Instruction* Pointer;
	};

	static ProgramData* Parent;
	static std::vector<value_type> Storage;
};

inline bool operator==(Instruction::Type x, const Instruction& y);
inline bool operator!=(const Instruction& x, Instruction::Type y);
inline bool operator!=(Instruction::Type x, const Instruction& y);
