#pragma once

#include <vector>
#include <list>
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
	Memory::iterator DataPointer = Cells.Data.begin();

public:
	explicit ProgramData(const std::string& Source);
	~ProgramData();

	void Run();
};

class Instruction
{
	friend ProgramData;

public:
	enum class Type { Nop, MovePointer, Addition, Input, Output, LoopStart, LoopEnd, Reset, Multiplication, Store, Stop };

	explicit Instruction(Type, std::intptr_t = 0);
	Instruction(Type, Instruction*);
	Instruction(Type, std::int32_t Offset, std::int32_t Value);

	void Execute() const;

	bool operator==(Type) const;

	static void AdvancePointer(std::ptrdiff_t Offset);

	static void SetParent(ProgramData*);
	static void Orphan(ProgramData*);

private:
	const Type Command;
	union
	{
		std::intptr_t Data;
		std::int32_t SmallData[2];
		Instruction* Pointer;
	};

	static ProgramData* Parent;
	static std::intptr_t TemporaryValue;
};

inline bool operator==(Instruction::Type x, const Instruction& y);
inline bool operator!=(const Instruction& x, Instruction::Type y);
inline bool operator!=(Instruction::Type x, const Instruction& y);
