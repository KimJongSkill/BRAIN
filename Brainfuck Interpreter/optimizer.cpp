#include "program.hpp"

#include <algorithm>
#include <iterator>
#include <array>
#include <vector>
#include <numeric>

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
	*	If this loop does not contain other non-optimized loops, Seek's,
	*	and Input/Output instructions, then we should be
	*	able to replace it with Multiplication instructions
	*	and a Push/Pop pair
	*/

	static constexpr std::array<Instruction::Type, 5> Unwanted = { Instruction::Type::LoopStart, Instruction::Type::LoopEnd,
		Instruction::Type::Input, Instruction::Type::Output, Instruction::Type::Seek };

	if (std::find_first_of(std::next(Begin), End, std::cbegin(Unwanted), std::cend(Unwanted)) == End)
	{
		Instruction::value_type CurrentOffset = 0;
		Instruction::value_type LastOffset = 0;

		std::vector<Instruction> Operations;
		Operations.reserve(std::distance(Begin, End));
		Instruction::value_type Cell0Total = 0;

		for (auto Iterator = std::next(Begin); Iterator != End; ++Iterator)
		{
			switch (Iterator->Command)
			{
			case Instruction::Type::MovePointer:
				CurrentOffset += *Iterator->Data;
				break;
			case Instruction::Type::Addition:
				if (!CurrentOffset)
				{
					Cell0Total += *Iterator->Data;
				}
				else
				{
					Operations.emplace_back(Instruction::Type::Multiplication, CurrentOffset - LastOffset, *Iterator->Data);

					LastOffset = CurrentOffset;
				}
				break;
			case Instruction::Type::Multiplication:
				if (!CurrentOffset)
				{
					// Not yet implemented, needs more experimenting
					return false;
				}
				else
				{
					CurrentOffset += Iterator->Data[0];

					Operations.emplace_back(Instruction::Type::Multiplication, CurrentOffset - LastOffset, Iterator->Data[1]);

					LastOffset = CurrentOffset;
				}
				break;
			case Instruction::Type::Push:
			case Instruction::Type::Pop:
			case Instruction::Type::Set:
			case Instruction::Type::Reset:
				Operations.push_back(*Iterator);
				break;
			default:
				throw std::runtime_error("This shouldn't have happened. Probably a missing clause in the switch.");
			}
		}

		/*
		*	Make sure the Pointer ended up were it started
		*	and only 1 was subtracted from Cell #0
		*/
		if (!CurrentOffset && Cell0Total == -1)
		{
			/*
			*	We cannot use std::vector::erase because
			*	we have a pointer, not an iterator, so
			*	we have to remove the elements one at a time
			*/
			while (Begin <= &Text.back())
				Text.pop_back();

			JumpTable.pop();

			Text.emplace_back(Instruction::Type::Push);

			for (auto Operation : Operations)
				Text.emplace_back(Operation);

			Text.emplace_back(Instruction::Type::Pop);

			Text.emplace_back(Instruction::Type::MovePointer, -LastOffset);

			return true;
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