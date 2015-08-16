#include "interpreter.hpp"
#include "io.hpp"

#include <iterator>
#include <utility>
#include <string>
#include <stack>

Instruction::Instruction(Type x, std::intptr_t y) : Command(x), Data(y) { }

Instruction::Instruction(Type x, Instruction* y) : Command(x), Data(reinterpret_cast<std::intptr_t>(y)) { }

ProgramData* Instruction::Parent = nullptr;

void Instruction::Execute() const
{
	switch (Command)
	{
	case Type::MovePointer:
		Parent->Cells.Index += Data;

		for (;Parent->Cells.Index < Parent->Cells.Limits.first; --Parent->Cells.Limits.first)
			Parent->Cells.Data.emplace_front(0);
		for (;Parent->Cells.Index > Parent->Cells.Limits.second; ++Parent->Cells.Limits.second)
			Parent->Cells.Data.emplace_back(0);

		std::advance(Parent->Pointer, Data);
		break;
	case Type::Addition:
		*Parent->Pointer += Data;
		break;
	case Type::Input:
		*Parent->Pointer = InputByte();
		break;
	case Type::Output:
		OutputByte(*Parent->Pointer);
		break;
	case Type::ConditionalJump:
		if (!*Parent->Pointer)
			Parent->InstructionPointer = reinterpret_cast<Instruction*>(Data);
		break;
	case Type::Jump:
			Parent->InstructionPointer = reinterpret_cast<Instruction*>(Data);
			break;
		break;
	case Type::Nop:
		break;
	}
}

void Instruction::Modify(std::intptr_t x)
{
	Data += x;
}

void Instruction::Set(Instruction* x)
{
	Data = reinterpret_cast<std::intptr_t>(x);
}

std::pair<Instruction::Type, std::intptr_t> Instruction::Query() const
{
	return std::make_pair(Command, Data);
}

void Instruction::SetParent(ProgramData* Adopter)
{
	if (Parent == nullptr)
		Parent = Adopter;
	else
		throw std::logic_error("Instruction class already has a parent");
}

void Instruction::Orphan(ProgramData* Adopter)
{
	if (Parent != Adopter)
		throw std::logic_error("Attempted to orphan the instruction class without adopting it first");
	else
		Parent = nullptr;
}

ProgramData::ProgramData(const char* const Path)
{
	std::string Source = Open(Path);
	std::stack<Instruction*> JumpTable;
	Instruction::Type Last = Instruction::Type::Nop;

	auto Lambda = [&](Instruction::Type x, std::intptr_t y)
	{
		if (Last == x)
		{
			Text.back().Modify(y);
		}
		else
		{
			Text.emplace_back(x, y);
			Last = x;
		}
	};

	// Dirty way to make sure our pointers don't get invalidated later on
	Text.reserve(Source.size());

	Instruction::SetParent(this);
	
	for (auto Char : Source)
	{
		switch (Char)
		{
		case '>':
			Lambda(Instruction::Type::MovePointer, 1);
			break;
		case '<':
			Lambda(Instruction::Type::MovePointer, -1);
			break;
		case '+':
			Lambda(Instruction::Type::Addition, 1);
			break;
		case '-':
			Lambda(Instruction::Type::Addition, -1);
			break;
		case '.':
			Text.emplace_back(Instruction::Type::Output);
			Last = Instruction::Type::Output;
			break;
		case ',':
			Text.emplace_back(Instruction::Type::Input);
			Last = Instruction::Type::Input;
			break;
		case '[':
			Text.emplace_back(Instruction::Type::ConditionalJump);
			JumpTable.push(&Text.back());

			Last = Instruction::Type::ConditionalJump;
			break;
		case ']':
			if (JumpTable.size() > 0)
			{
				Text.emplace_back(Instruction::Type::Jump, JumpTable.top());
				JumpTable.top()->Set(&Text.back() + 1);
				JumpTable.pop();
			}
			else
			{
				throw std::runtime_error("Unbalanced ']'");
			}

			Last = Instruction::Type::Jump;
			break;
		}
	}

	if (!JumpTable.empty())
		throw std::runtime_error("Unbalanced '['");

	// Ensure that we don't jump out of bounds
	// if the program ends with a ']'
	Text.emplace_back(Instruction::Type::Stop);
	InstructionPointer = &Text.front();
}

ProgramData::~ProgramData()
{
	Instruction::Orphan(this);
}

void ProgramData::Run()
{
	while (InstructionPointer->Query().first != Instruction::Type::Stop)
	{
		auto* Temp = InstructionPointer;
		++InstructionPointer; // Might cause problems
		Temp->Execute();
	}
}
