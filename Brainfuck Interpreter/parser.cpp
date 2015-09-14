#include "program.hpp"

void ProgramData::Parse(const std::string& Source)
{
	// Dirty way to make sure our pointers do not get invalidated later on
	Text.reserve(std::size(Source));
	
	auto MovePointerLambda = [&](Instruction::value_type x)
	{
		if (Text.back() == Instruction::Type::MovePointer)
		{
			*Text.back().Data += x;

			// Remove instructions that have no effect
			if (!*Text.back().Data)
				Text.pop_back();
		}
		else
		{
			Text.emplace_back(Instruction::Type::MovePointer, x);
		}
	};

	auto AdditionLambda = [&](Instruction::value_type x)
	{
		if (Text.back() == Instruction::Type::Addition)
		{
			*Text.back().Data += x;

			// Remove instructions that have no effect
			if (!Text.back().Data[0])
				Text.pop_back();
		}
		else if (Text.back() == Instruction::Type::Reset
			&& Text.back().Data[0] == 1
			&& Text.back().Data[1] == 0)
		{
			Text.pop_back();

			Text.emplace_back(Instruction::Type::Set, x);
		}
		else if (Text.back() == Instruction::Type::Set)
		{
			*Text.back().Data += x;
		}
		else
		{
			Text.emplace_back(Instruction::Type::Addition, x);
		}
	};
	
	for (auto Char : Source)
	{
		switch (Char)
		{
		case '>':
			MovePointerLambda(1);
			break;
		case '<':
			MovePointerLambda(-1);
			break;
		case '+':
			AdditionLambda(1);
			break;
		case '-':
			AdditionLambda(-1);
			break;
		case '.':
			Text.emplace_back(Instruction::Type::Output);
			break;
		case ',':
			Text.emplace_back(Instruction::Type::Input);
			break;
		case '[':
			Text.emplace_back(Instruction::Type::LoopStart);
			JumpTable.push(&Text.back());
			break;
		case ']':
			if (JumpTable.size() > 0)
			{
				Instruction* const BeginPointer = JumpTable.top();
				Instruction* const EndPointer = std::data(Text) + std::size(Text);

				if (DropEmptyLoop(BeginPointer, EndPointer))
					break;
				if (AttemptReset(BeginPointer, EndPointer))
					break;
				if (AttemptSeek(BeginPointer, EndPointer))
					break;
				if (AttemptMultiplication(BeginPointer, EndPointer))
					break;

				Text.emplace_back(Instruction::Type::LoopEnd, std::next(BeginPointer));
				JumpTable.top()->Pointer = EndPointer;
				JumpTable.pop();
			}
			else
			{
				throw std::runtime_error("Unbalanced ']'");
			}
			break;
		}
	}

	if (!JumpTable.empty())
		throw std::runtime_error("Unbalanced '['");

	/*
	*	Ensure that we do not jump out of bounds
	*	if the program ends with a ']'
	*/
	Text.emplace_back(Instruction::Type::Stop);
	InstructionPointer = &Text.front();
}