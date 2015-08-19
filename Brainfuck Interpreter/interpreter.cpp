#include "interpreter.hpp"
#include "io.hpp"

#include <iterator>
#include <utility>
#include <string>
#include <stack>
#include <algorithm>

Instruction::Instruction(Type x, std::intptr_t y) : Command(x), Data(y) { }

Instruction::Instruction(Type x, Instruction* y) : Command(x), Data(reinterpret_cast<std::intptr_t>(y)) { }

ProgramData* Instruction::Parent = nullptr;

void Instruction::Execute() const
{
	switch (Command)
	{
	case Type::MovePointer:
		AdvancePointer(Data);
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
	case Type::LoopStart:
		if (*Parent->Pointer == 0)
			Parent->InstructionPointer = reinterpret_cast<Instruction*>(Data);
		break;
	case Type::LoopEnd:
		if (*Parent->Pointer != 0)
			Parent->InstructionPointer = reinterpret_cast<Instruction*>(Data);
		break;
	case Type::Reset:
		*Parent->Pointer = 0;
		break;
	case Type::Multiplication:
		// The first half of the Value contains
		// the offset and the second the multiplication value
		auto Offset = Data >> (sizeof(std::ptrdiff_t) * 4);
		auto Value1 = Data & static_cast<std::intptr_t>(std::pow(2, sizeof(std::ptrdiff_t) * 4) - 1);
		auto Value2 = *Parent->Pointer;

		AdvancePointer(Offset);
		*Parent->Pointer += Value1 * Value2;
		std::advance(Parent->Pointer, -Offset);
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

void Instruction::AdvancePointer(std::ptrdiff_t Offset)
{
	Parent->Cells.Index += Offset;

	for (;Parent->Cells.Index < Parent->Cells.Limits.first; --Parent->Cells.Limits.first)
		Parent->Cells.Data.emplace_front(0);
	for (;Parent->Cells.Index > Parent->Cells.Limits.second; ++Parent->Cells.Limits.second)
		Parent->Cells.Data.emplace_back(0);

	std::advance(Parent->Pointer, Offset);
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

			// Remove instructions that have no effect
			if (!Text.back().Query().second)
			{
				Text.pop_back();

				Last = Instruction::Type::Nop;
			}
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
			Text.emplace_back(Instruction::Type::LoopStart);
			JumpTable.push(&Text.back());
			
			Last = Instruction::Type::LoopStart;
			break;
		case ']':
			if (JumpTable.size() > 0)
			{
				// Remove empty loops
				if (Text.back().Query().first == Instruction::Type::LoopStart)
				{
					Text.pop_back();
					JumpTable.pop();

					Last = Instruction::Type::Nop;
					break;
				}

				// Try to detect a "[-]" and replace it with a Reset instruction
				if (std::next(Text.rbegin())->Query().first == Instruction::Type::LoopStart)
				{
					if (Text.back().Query().first == Instruction::Type::Addition)
					{
						Text.pop_back();
						Text.pop_back();
						JumpTable.pop();

						Text.emplace_back(Instruction::Type::Reset);
						Last = Instruction::Type::Reset;
						break;
					}
				}

				Instruction* EndPointer = std::data(Text) + std::size(Text);

				/*
				*	If this is loop only contains MovePointer
				*	and Addition instructions, then we should be
				*	able to replace it with Multiplication instructions
				*/
				if (std::all_of(std::next(JumpTable.top()), EndPointer, [](const Instruction& x)
				{ return x == Instruction::Type::Addition || x == Instruction::Type::MovePointer; }))
				{
					std::intptr_t Offset = 0;
					// Offset, Value
					std::list<std::pair<std::intptr_t, std::intptr_t>> Operations;

					for (auto Iterator = std::next(JumpTable.top()); Iterator != EndPointer; ++Iterator)
					{
						if (*Iterator == Instruction::Type::MovePointer)
							Offset += Iterator->Query().second;
						else if (*Iterator == Instruction::Type::Addition)
							Operations.emplace_back(Offset, Iterator->Query().second);
					}

					/*
					*	Make sure the Pointer ends up were it started
					*	and only 1 was subtracted from Cell #0
					*/
					if (Offset == 0)
					{
						if (std::count_if(std::cbegin(Operations), std::cend(Operations), [](std::pair<std::intptr_t, std::intptr_t> x)
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
								while (JumpTable.top() <= &Text.back())
									Text.pop_back();
								JumpTable.pop();
								
								for (const auto& Operation : Operations)
								{
									if (Operation.first)
										Text.emplace_back(Instruction::Type::Multiplication, Operation.first << (sizeof(std::ptrdiff_t) * 4) | Operation.second);
								}
								Text.emplace_back(Instruction::Type::Reset);

								break;
							}
						}
					}
				}

				Text.emplace_back(Instruction::Type::LoopEnd, std::next(JumpTable.top()));
				JumpTable.top()->Set(EndPointer);
				JumpTable.pop();

				Last = Instruction::Type::LoopEnd;
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
	*	Ensure that we don't jump out of bounds
	*	if the program ends with a ']'
	*/
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
		InstructionPointer++->Execute();
}

bool operator==(const Instruction& x, Instruction::Type y)
{
	return x.Query().first == y;
}

bool operator==(Instruction::Type x, const Instruction& y)
{
	return y == x;
}

bool operator!=(const Instruction& x, Instruction::Type y)
{
	return !(x == y);
}
bool operator!=(Instruction::Type x, const Instruction& y)
{
	return y != x;
}

