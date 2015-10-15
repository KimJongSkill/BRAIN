#include "program.hpp"

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y) { }
Instruction::Instruction(Type x, value_type y, value_type z) : Command(x), Value(y), Offset(z) { }

ProgramData* Instruction::Parent = nullptr;
bf::io::ProgramInput* Instruction::InputQueue = nullptr;

template <class Type>
static void ReplaceIfNull(Type*& Pointer, Type* const New)
{
	if (Pointer != nullptr)
		throw std::logic_error("Attempted to replace an already valid pointer");

	Pointer = New;
}

void Instruction::SetParent(ProgramData* const Adopter, bf::io::ProgramInput* const InputPtr)
{
	ReplaceIfNull(Parent, Adopter);
	ReplaceIfNull(InputQueue, InputPtr);
}

void Instruction::Orphan(const ProgramData* const Adopter)
{
	if (Parent != Adopter)
		throw std::logic_error("Attempted to orphan the instruction class without adopting it first");
	else
		Parent = nullptr;
}

ProgramData::ProgramData(bf::io::ProgramInput& InputRef) : Text{ Instruction::Type::Nop }, DataPointer(std::begin(Cells)), FastStorage(0)
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
