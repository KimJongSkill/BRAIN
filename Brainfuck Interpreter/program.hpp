#pragma once

#include "memory.hpp"
#include "io.hpp"

#include <vector>

class Instruction
{
	friend class ProgramData;

public:
	enum Type { Nop, MovePointer, Addition, Input, Output, LoopStart, LoopEnd, Reset, Multiplication, PushFast, Push, PopFast, Pop, Seek, Set, Stop };
	typedef int value_type;

	Instruction(Type, Instruction*);
	Instruction(Type, value_type = 0, value_type = 0);

	bool operator==(Type) const;

	static void SetParent(ProgramData*, io::ProgramInput* const);
	static void Orphan(ProgramData*);

	union
	{
		Type Command;
		void(*FunctionPointer)(Instruction*);
	};
	union
	{
		struct { value_type Value, Offset; };
		Instruction* Pointer;
	};

	static ProgramData* Parent;
	static io::ProgramInput* InputQueue;

	static const std::array<void(*)(Instruction*), Type::Stop + 1> FunctionPointers;
};

class ProgramData
{
	std::vector<Instruction> Text;
	Memory Cells;

public:
	explicit ProgramData(io::ProgramInput&);
	~ProgramData();

	void Run();
	ProgramData& From(const std::string& Source);

	std::vector<Instruction>::pointer InstructionPointer;
	Memory::iterator DataPointer;
	
	std::vector<Memory::cell_type> Storage;
	char FastStorage;

private:
	void Parse(const std::string&);
	void Link();
	void LinkJumps();
	void LinkFunctions();

	bool AttemptReset(Instruction* Begin, Instruction* End);
	bool AttemptSeek(Instruction* Begin, Instruction* End);
	bool AttemptMultiplication(Instruction* Begin, Instruction* End);
	bool DropEmptyLoop(Instruction* Begin, Instruction* End);
	void DropPopFast();
};

inline bool operator==(Instruction::Type x, const Instruction& y);
inline bool operator!=(const Instruction& x, Instruction::Type y);
inline bool operator!=(Instruction::Type x, const Instruction& y);
