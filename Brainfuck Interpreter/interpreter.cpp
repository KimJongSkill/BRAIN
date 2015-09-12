#include "program.hpp"
#include "io.hpp"

#include <iterator>
#include <numeric>

void Instruction::Execute() const
{
	switch (Command)
	{
	case Type::MovePointer:
		std::advance(Parent->DataPointer, *Data);
		break;
	case Type::Addition:
		*Parent->DataPointer += *Data;
		break;
	case Type::Input:
		*Parent->DataPointer = InputByte();
		break;
	case Type::Output:
		OutputByte(*Parent->DataPointer);
		break;
	case Type::LoopStart:
		if (*Parent->DataPointer == 0)
			Parent->InstructionPointer = Pointer;
		break;
	case Type::LoopEnd:
		if (*Parent->DataPointer != 0)
			Parent->InstructionPointer = Pointer;
		break;
	case Type::Push:
		Parent->Storage.push_back(*Parent->DataPointer);
		*Parent->DataPointer = 0;
		break;
	case Type::Pop:
		Parent->Storage.pop_back();
		break;
	case Type::PushFast:
		Parent->FastStorage = *Parent->DataPointer;
		*Parent->DataPointer = 0;
		break;
	case Type::PopFast:
		break;
	case Type::Reset:
		std::fill_n(Parent->DataPointer, Data[0], 0);
		std::advance(Parent->DataPointer, Data[1]);
		break;
	case Type::Multiplication:
		std::advance(Parent->DataPointer, Data[0]);

		if (Parent->Storage.empty())
			*Parent->DataPointer += Data[1] * Parent->FastStorage;
		else
			*Parent->DataPointer += std::accumulate(std::cbegin(Parent->Storage), std::cend(Parent->Storage), Data[1], std::multiplies<value_type>());
		break;
	case Type::Seek:
		while (*Parent->DataPointer)
			std::advance(Parent->DataPointer, *Data);
		break;
	case Type::Set:
		*Parent->DataPointer = *Data;
		break;
	}
}

void ProgramData::Run()
{
	while (InstructionPointer->Command != Instruction::Type::Stop)
		InstructionPointer++->Execute();
}
