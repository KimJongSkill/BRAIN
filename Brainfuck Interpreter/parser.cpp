#include "program.hpp"

void ProgramData::Parse(const std::string& Source)
{
	auto MovePointerLambda = [&](Instruction::value_type x)
	{
		if (Text.back() == Instruction::Type::MovePointer)
		{
			Text.back().Value += x;

			// Remove instructions that have no effect
			if (!Text.back().Value)
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
			Text.back().Value += x;

			// Remove instructions that have no effect
			if (!Text.back().Value)
				Text.pop_back();
		}
		else if (Text.back() == Instruction::Type::Reset
			&& Text.back().Value == 1
			&& Text.back().Offset == 0)
		{
			Text.pop_back();

			Text.emplace_back(Instruction::Type::Set, x);
		}
		else if (Text.back() == Instruction::Type::Set)
		{
			Text.back().Value += x;
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
			break;
		case ']':
			Instruction* const BeginPointer = &*std::find(std::rbegin(Text), std::rend(Text), Instruction::Type::LoopStart);
			Instruction* const EndPointer = std::data(Text) + std::size(Text);

			if (DropEmptyLoop(BeginPointer, EndPointer))
				break;
			if (AttemptReset(BeginPointer, EndPointer))
				break;
			if (AttemptSeek(BeginPointer, EndPointer))
				break;
			if (AttemptMultiplication(BeginPointer, EndPointer))
				break;

			Text.emplace_back(Instruction::Type::LoopEnd);
			break;
		}
	}
}