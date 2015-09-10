#include "program.hpp"

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y) { }
Instruction::Instruction(Type x, value_type y, value_type z) : Command(x), Data{ y, z } { }

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

ProgramData::ProgramData(const std::string& Source)
{
	// Dirty way to make sure our pointers do not get invalidated later on
	Text.reserve(std::size(Source));
	// Allows us to call std::vector::back
	Text.emplace_back(Instruction::Type::Nop);

	Instruction::SetParent(this);

	Parse(Source);
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
