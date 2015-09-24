#include "program.hpp"

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y) { }
Instruction::Instruction(Type x, value_type y, value_type z) : Command(x), Value(y), Offset(z) { }

ProgramData* Instruction::Parent = nullptr;

void Instruction::SetParent(ProgramData* Adopter)
{
	if (Parent == nullptr)
		Parent = Adopter;
	else
		throw std::logic_error("Instruction class already has a parent");
}

void Instruction::Orphan(ProgramData* Adopter)
{
	if (Parent != Adopter)
		throw std::logic_error("Attempted to orphan the instruction class without adopting it first");
	else
		Parent = nullptr;
}

ProgramData::ProgramData() : Text{ Instruction::Type::Nop }, DataPointer(std::begin(Cells)), FastStorage(0)
{
	/*
	*	Initialize Text with a Nop instruction so
	*	that we can call std::vector::back when parsing.
	*/
	Instruction::SetParent(this);
}

ProgramData::~ProgramData()
{
	Instruction::Orphan(this);
}


bool Instruction::operator==(Type y) const
{
	return Command == y;
}

bool operator==(Instruction::Type x, const Instruction& y)
{
	return y == x;
}

bool operator!=(const Instruction& x, Instruction::Type y)
{
	return !(x == y);
}
bool operator!=(Instruction::Type x, const Instruction& y)
{
	return y != x;
}
