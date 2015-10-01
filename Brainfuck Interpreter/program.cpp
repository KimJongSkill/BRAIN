#include "program.hpp"

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y) { }
Instruction::Instruction(Type x, value_type y, value_type z) : Command(x), Value(y), Offset(z) { }

ProgramData* Instruction::Parent = nullptr;
io::ProgramInput* Instruction::InputQueue = nullptr;

void Instruction::SetParent(ProgramData* const Adopter, io::ProgramInput* const InputPtr)
{
	if (Parent == nullptr)
		Parent = Adopter;
	else
		throw std::logic_error("Instruction class already has a parent");

	if (InputQueue == nullptr)
		InputQueue = InputPtr;
	else
		throw std::logic_error("Instruction class already has an input queue");		
}

void Instruction::Orphan(const ProgramData* const Adopter)
{
	if (Parent != Adopter)
		throw std::logic_error("Attempted to orphan the instruction class without adopting it first");
	else
		Parent = nullptr;
}

ProgramData::ProgramData(io::ProgramInput& InputRef) : Text{ Instruction::Type::Nop }, DataPointer(std::begin(Cells)), FastStorage(0)
{
	/*
	*	Initialize Text with a Nop instruction so
	*	that we can call std::vector::back when parsing.
	*/
	Instruction::SetParent(this, &InputRef);
}

ProgramData::~ProgramData()
{
	Instruction::Orphan(this);
}

ProgramData& ProgramData::From(const std::string& Source)
{
	Parse(Source);
	DropPopFast();
	Link();

	return *this;
}

bool Instruction::operator==(Type y) const
{
	return Command == y;
}
