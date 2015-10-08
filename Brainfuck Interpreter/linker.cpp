#include "program.hpp"

#include <stack>

void ProgramData::Link()
{
	/*
	*	Make sure no relocation happens when 
	*	we push Instruction::Type::Stop
	*/
	Text.reserve(Text.size() + 1);

	LinkJumps();
	LinkFunctions();

	InstructionPointer = Text.data();

	/*
	*	Ensure that we do not jump out of bounds
	*	if the program ends with a ']'.
	*	It is intentionally inserted after
	*	the call to LinkFunctions() so the
	*	Command variable won't get overwritten
	*	by the FunctionPointer.
	*/
	Text.emplace_back(Instruction::Type::Stop);
}

void ProgramData::LinkJumps()
{
	std::stack<Instruction*> JumpTable;

	for (auto Pointer = std::data(Text); Pointer != std::data(Text) + std::size(Text); ++Pointer)
	{
		if (*Pointer == Instruction::Type::LoopStart)
		{
			JumpTable.push(Pointer);
		}
		else if (*Pointer == Instruction::Type::LoopEnd)
		{
			if (JumpTable.empty())
				throw std::runtime_error("Umatched ']'");

			Pointer->Pointer = std::next(JumpTable.top());
			JumpTable.top()->Pointer = std::next(Pointer);

			JumpTable.pop();
		}
	}

	if (!JumpTable.empty())
		throw std::runtime_error("Unmatched '['");
}

void ProgramData::LinkFunctions()
{
	for (auto& Item : Text)
		Item.FunctionPointer = Instruction::FunctionPointers[Item.Command];
}
