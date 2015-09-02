#include "interpreter.hpp"
#include "io.hpp"

#include <iterator>
#include <utility>
#include <string>
#include <stack>
#include <algorithm>
#include <numeric>

Instruction::Instruction(Type x, std::intptr_t y) : Command(x), Data(y) { }

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y) { }

Instruction::Instruction(Type x, std::int32_t y, std::int32_t z) : Command(x), SmallData{ y, z } { }

ProgramData* Instruction::Parent = nullptr;
std::intptr_t Instruction::TemporaryValue = 0;

Memory::Front_tag Memory::Front{};
Memory::Back_tag Memory::Back{};

void Instruction::Execute() const
{
	switch (Command)
	{
	case Type::MovePointer:
		std::advance(Parent->DataPointer, Data);
		break;
	case Type::Addition:
		*Parent->DataPointer += Data;
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
	case Type::Store:
		TemporaryValue = *Parent->DataPointer;
		*Parent->DataPointer = 0;
		break;
	case Type::Reset:
		std::fill_n(Parent->DataPointer, SmallData[0], 0);
		std::advance(Parent->DataPointer, SmallData[1]);
		break;
	case Type::Multiplication:
		std::advance(Parent->DataPointer, SmallData[0]);
		*Parent->DataPointer += SmallData[1] * TemporaryValue;
		break;
	case Type::Seek:
		while (*Parent->DataPointer)
			std::advance(Parent->DataPointer, Data);
		break;
	}
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

ProgramData::ProgramData(const std::string& Source)
{
	std::stack<Instruction*> JumpTable;

	auto Lambda = [&](Instruction::Type x, std::intptr_t y)
	{
		if (Text.back() == x)
		{
			Text.back().Data += y;

			// Remove instructions that have no effect
			if (!Text.back().Data)
				Text.pop_back();
		}
		else
		{
			Text.emplace_back(x, y);
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
				// Remove empty loops
				if (Text.back().Command == Instruction::Type::LoopStart)
				{
					Text.pop_back();
					JumpTable.pop();

					break;
				}

				/* 
				*	Try to detect a "[-]" and replace it with a Reset instruction
				*	or a "[<]"/"[>]" and replace it with a Seek instruction
				*/
				if (std::next(Text.rbegin())->Command == Instruction::Type::LoopStart)
				{
					if (Text.back().Command == Instruction::Type::Addition)
					{
						Text.pop_back();
						Text.pop_back();
						JumpTable.pop();

						if (Text.back() == Instruction::Type::Reset);
						// Detect "[-]>[-]..."
						else if (Text.back() == Instruction::Type::MovePointer
							&& std::abs(Text.back().Data) == 1
							&& *std::prev(std::cend(Text), 2) == Instruction::Type::Reset)
						{
							std::prev(std::end(Text), 2)->SmallData[0] += Text.back().Data;
							std::prev(std::end(Text), 2)->SmallData[1] += Text.back().Data;
							Text.pop_back();
						}
						else
						{
							Text.emplace_back(Instruction::Type::Reset, 1, 0);
						}

						break;
					}
					else if (Text.back().Command == Instruction::Type::MovePointer)
					{
						auto Data = Text.back().Data;

						Text.pop_back();
						Text.pop_back();
						JumpTable.pop();

						Text.emplace_back(Instruction::Type::Seek, Data);
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
					std::intptr_t CurrentOffset = 0;
					std::intptr_t LastOffset = 0;
					// Offset, Value
					std::list<std::pair<std::intptr_t, std::intptr_t>> Operations;

					for (auto Iterator = std::next(JumpTable.top()); Iterator != EndPointer; ++Iterator)
					{
						if (*Iterator == Instruction::Type::MovePointer)
						{
							CurrentOffset += Iterator->Data;
						}
						else if (*Iterator == Instruction::Type::Addition)
						{
							Operations.emplace_back(CurrentOffset - LastOffset, Iterator->Data);
							LastOffset = CurrentOffset;
						}
					}

					/*
					*	Make sure the Pointer ends up were it started
					*	and only 1 was subtracted from Cell #0
					*/
					if (CurrentOffset == 0)
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
								Operations.erase(Cell0);

								Text.emplace_back(Instruction::Type::Store);

								for (const auto& Operation : Operations)
									Text.emplace_back(Instruction::Type::Multiplication, Operation.first, Operation.second);

								Text.emplace_back(Instruction::Type::MovePointer, -std::accumulate(std::cbegin(Operations), std::cend(Operations), 0,
									[](intptr_t x, std::pair<std::intptr_t, std::intptr_t> y) { return x + y.first; }));

								break;
							}
						}
					}
				}

				Text.emplace_back(Instruction::Type::LoopEnd, std::next(JumpTable.top()));
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
	while (InstructionPointer->Command != Instruction::Type::Stop)
		InstructionPointer++->Execute();
}

bool Instruction::operator==(Type y) const
{
	return Command == y;
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

bool Memory_iterator::operator==(const Memory_iterator& Other) const
{
	return Pointer == Other.Pointer;
}

bool Memory_iterator::operator!=(const Memory_iterator& Other) const
{
	return !(*this == Other);
}

auto Memory_iterator::operator*() const -> reference
{
	return *Pointer;
}

auto Memory_iterator::operator->() const -> pointer
{
	return Pointer;
}

Memory_iterator& Memory_iterator::operator++()
{
	Advance(*this, 1);

	return *this;
}

Memory_iterator& Memory_iterator::operator++(int)
{
	auto Temp = *this;
	++*this;
	return Temp;
}

Memory_iterator& Memory_iterator::operator--()
{
	Advance(*this, -1);

	return *this;
}

Memory_iterator& Memory_iterator::operator--(int)
{
	auto Temp = *this;
	--*this;
	return Temp;
}

auto Memory_iterator::operator-(const Memory_iterator& Other) const -> difference_type
{
	return Index - Other.Index;
}

Memory_iterator Memory_iterator::operator+(difference_type Delta) const
{
	Memory_iterator New(*this);

	Advance(New, Delta);

	return New;
}

Memory_iterator Memory_iterator::operator-(difference_type Delta) const
{
	return *this + -Delta;
}

bool Memory_iterator::operator<(const Memory_iterator& Other) const
{
	return Index < Other.Index;
}

bool Memory_iterator::operator>(const Memory_iterator& Other) const
{
	return Index > Other.Index;
}

bool Memory_iterator::operator<=(const Memory_iterator& Other) const
{
	return Index <= Other.Index;
}

bool Memory_iterator::operator>=(const Memory_iterator& Other) const
{
	return Index >= Other.Index;;
}

Memory_iterator& Memory_iterator::operator+=(difference_type Delta)
{
	Advance(*this, Delta);

	return *this;
}

Memory_iterator & Memory_iterator::operator-=(difference_type Delta)
{
	return *this += -Delta;
}

auto Memory_iterator::operator[](difference_type Offset) const -> reference
{
	return *(*this + Offset);
}

void Memory_iterator::Advance(Memory_iterator& Target, std::ptrdiff_t Delta)
{
	std::ptrdiff_t NewIndex = Target.Index + Delta;

	if (NewIndex > Target.Parent->Limits.second)
		Target.Pointer = Target.Parent->RequestNewPage(Memory::Back);
	else if (NewIndex < Target.Parent->Limits.first)
		Target.Pointer = Target.Parent->RequestNewPage(Memory::Front);
	/*
	*	Check if the iterator has moved to another page.
	*	Each page can hold 256 elements, so the first byte
	*	of the Index stores the index within a page and
	*	the other bytes store the index of the page within
	*	the list.
	*/
	else if (NewIndex >> 8 != Target.Index >> 8)
		Target.Pointer = std::next(std::next(Target.Parent->Origin, NewIndex >> 8)->data(), NewIndex & 0xff);
	else
		Target.Pointer = Target.Pointer + Delta;

	Target.Index = NewIndex;
}

auto Memory::begin() const -> iterator
{
	iterator New;
	New.Index = Limits.first;
	New.Parent = const_cast<Memory*>(this);
	New.Pointer = const_cast<char*>(&Storage.front().front());

	return New;
}

auto Memory::end() const -> iterator
{
	iterator New;
	New.Index = Limits.second;
	New.Parent = const_cast<Memory*>(this);
	New.Pointer = const_cast<char*>(&Storage.back().back());

	return New;
}

auto Memory::RequestNewPage(Front_tag) -> iterator::pointer
{
	Storage.emplace_front();
	Storage.front().fill(0);

	Limits.first -= Storage.front().size();

	return &Storage.front().back();
}

auto Memory::RequestNewPage(Back_tag) -> iterator::pointer
{
	Storage.emplace_back();
	Storage.back().fill(0);

	Limits.second += Storage.back().size();

	return &Storage.back().front();
}
