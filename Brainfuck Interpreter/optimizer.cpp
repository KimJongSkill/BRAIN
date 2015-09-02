#include "program.hpp"

#include <algorithm>
#include <numeric>
#include <iterator>

bool ProgramData::AttemptReset(Instruction* Begin, Instruction* End)
{
	// Detect a "[-]" and replace it with a Reset instruction

	if (std::distance(Begin, End) == 2
		&& *Begin == Instruction::Type::LoopStart
		&& *std::next(Begin) == Instruction::Type::Addition)
	{
		Text.pop_back();
		Text.pop_back();
		JumpTable.pop();

		if (Text.back() == Instruction::Type::Reset && !Text.back().Data[1])
			return true;

		// Detect "[-]>[-]"
		if (Text.back() == Instruction::Type::MovePointer
			&& *std::prev(std::cend(Text), 2) == Instruction::Type::Reset)
		{
			/*
			*	If the pointer is only changed by 1 then the program
			*	is clearing a range, which allows us to combine the instructions.
			*	If not, we can always combine the MovePointer with the Reset instruction.
			*/
			auto MoveAmount = std::abs(*Text.back().Data);

			if (MoveAmount == 1)
				std::prev(std::end(Text), 2)->Data[0] += *Text.back().Data;

			std::prev(std::end(Text), 2)->Data[1] += *Text.back().Data;

			Text.pop_back();

			if (MoveAmount != 1)
				Text.emplace_back(Instruction::Type::Reset, 1, 0);

			return true;
		}

		Text.emplace_back(Instruction::Type::Reset, 1, 0);
		return true;
	}

	return false;
}

bool ProgramData::AttemptSeek(Instruction* Begin, Instruction* End)
{
	// Detect a "[<]" / "[>]" and replace it with a Seek instruction

	if (std::distance(Begin, End) == 2
		&& *Begin == Instruction::Type::LoopStart
		&& *std::next(Begin) == Instruction::Type::MovePointer)
	{
		auto Data = *Text.back().Data;

		Text.pop_back();
		Text.pop_back();
		JumpTable.pop();

		Text.emplace_back(Instruction::Type::Seek, Data);

		return true;
	}

	return false;
}

bool ProgramData::AttemptMultiplication(Instruction* Begin, Instruction* End)
{
	/*
	*	If this loop only contains MovePointer
	*	and Addition instructions, then we should be
	*	able to replace it with Multiplication instructions
	*/

	if (std::all_of(std::next(Begin), End, [](const Instruction& x)
	{ return x == Instruction::Type::Addition || x == Instruction::Type::MovePointer; }))
	{
		Instruction::value_type CurrentOffset = 0;
		Instruction::value_type LastOffset = 0;
		// Offset, Value
		std::list<std::pair<Instruction::value_type, Instruction::value_type>> Operations;

		for (auto Iterator = std::next(Begin); Iterator != End; ++Iterator)
		{
			if (*Iterator == Instruction::Type::MovePointer)
			{
				CurrentOffset += *Iterator->Data;
			}
			else if (*Iterator == Instruction::Type::Addition)
			{
				Operations.emplace_back(CurrentOffset - LastOffset, *Iterator->Data);
				LastOffset = CurrentOffset;
			}
		}

		/*
		*	Make sure the Pointer ended up were it started
		*	and only 1 was subtracted from Cell #0
		*/
		if (!CurrentOffset && std::count_if(std::cbegin(Operations), std::cend(Operations), [](std::pair<std::intptr_t, std::intptr_t> x)
		{ return x.first == 0; }) == 1)
		{
			auto Cell0 = std::find_if(std::cbegin(Operations), std::cend(Operations), [](std::pair<std::intptr_t, std::intptr_t> x)
			{ return x.first == 0; });

			if (Cell0->second == -1)
			{
				/*
				*	We cannot use std::vector::erase because
				*	we have a pointer, not an iterator, so
				*	we have to remove the elements one at a time
				*/
				while (Begin <= &Text.back())
					Text.pop_back();
				JumpTable.pop();
				Operations.erase(Cell0);

				Text.emplace_back(Instruction::Type::Push);

				for (const auto& Operation : Operations)
					Text.emplace_back(Instruction::Type::Multiplication, Operation.first, Operation.second);

				Text.emplace_back(Instruction::Type::Pop);

				Text.emplace_back(Instruction::Type::MovePointer, -std::accumulate(std::cbegin(Operations), std::cend(Operations), 0,
					[](auto x, const std::pair<Instruction::value_type, Instruction::value_type>& y) { return x + y.first; }));

				return true;
			}
		}
	}

	return false;
}

bool ProgramData::DropEmptyLoop(Instruction* Begin, Instruction* End)
{
	if (std::distance(Begin, End) == 1)
	{
		Text.pop_back();
		JumpTable.pop();

		return true;
	}

	return false;
}