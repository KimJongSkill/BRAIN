#include "program.hpp"
#include "io.hpp"

#include <iterator>
#include <numeric>

const std::array<void(*)(Instruction*), 16> Instruction::FunctionPointers
{
	[](Instruction* x) {},
	[](Instruction* x) { std::advance(Parent->DataPointer, x->Value); },
	[](Instruction* x) { *Parent->DataPointer += x->Value; },
	[](Instruction* x) { *Parent->DataPointer = io::InputByte(); },
	[](Instruction* x) { io::OutputByte(*Parent->DataPointer); },
	[](Instruction* x) { if (*Parent->DataPointer == 0) Parent->InstructionPointer = x->Pointer; },
	[](Instruction* x) { if (*Parent->DataPointer != 0) Parent->InstructionPointer = x->Pointer; },
	[](Instruction* x) { std::fill_n(Parent->DataPointer, x->Value, 0); std::advance(Parent->DataPointer, x->Offset); },
	[](Instruction* x) 
{
	std::advance(Parent->DataPointer, x->Offset); 
	if (Parent->Storage.empty())
		*Parent->DataPointer += x->Value * Parent->FastStorage;
	else
		*Parent->DataPointer += std::accumulate(std::cbegin(Parent->Storage), std::cend(Parent->Storage), x->Value, std::multiplies<value_type>()); 
},
	[](Instruction* x) { Parent->FastStorage = *Parent->DataPointer; *Parent->DataPointer = 0; },
	[](Instruction* x) { Parent->Storage.push_back(*Parent->DataPointer); *Parent->DataPointer = 0; },
	[](Instruction* x) {},
	[](Instruction* x) { Parent->Storage.pop_back(); },
	[](Instruction* x) { while (*Parent->DataPointer) std::advance(Parent->DataPointer, x->Value); },
	[](Instruction* x) { *Parent->DataPointer = x->Value; },
	[](Instruction* x) {}
};

void ProgramData::Run()
{
	if (Text.empty())
		return;

	while (InstructionPointer->Command != Instruction::Type::Stop)
	{
		auto Temp = InstructionPointer++;
		Temp->FunctionPointer(Temp);
	}
}
