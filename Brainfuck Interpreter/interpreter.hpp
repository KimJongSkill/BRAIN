#pragma once

#include <vector>
#include <list>
#include <utility>
#include <cstdint>

struct Memory
{
	typedef std::list<std::int8_t> Type;
	typedef Type::iterator iterator;

	Type Data{ 0 };
	std::pair<std::ptrdiff_t, std::ptrdiff_t> Limits = std::make_pair(0, 0);
	std::ptrdiff_t Index = 0;
};

class ProgramData
{
	friend class Instruction;

	std::vector<Instruction> Text;
	std::vector<Instruction>::pointer InstructionPointer;
	Memory Cells;
	Memory::iterator Pointer = Cells.Data.begin();

public:
	explicit ProgramData(const char* const Path);
	~ProgramData();
	void Run();
};

class Instruction
{
public:
	enum class Type { Nop, MovePointer, Addition, Input, Output, Jump, ConditionalJump, Stop };

	explicit Instruction(Type, std::intptr_t = 0);
	Instruction(Type, Instruction*);

	void Execute() const;
	void Modify(std::intptr_t);
	void Set(Instruction*);
	std::pair<Type, std::intptr_t> Query() const;
	static void SetParent(ProgramData*);
	static void Orphan(ProgramData*);

private:
	Type Command;
	std::intptr_t Data;
	static ProgramData* Parent;
};