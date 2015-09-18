#include "program.hpp"

#include <stack>

void ProgramData::Link()
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

	/*
	*	Ensure that we do not jump out of bounds
	*	if the program ends with a ']'
	*/
	Text.emplace_back(Instruction::Type::Stop);
	InstructionPointer = &Text.front();
}