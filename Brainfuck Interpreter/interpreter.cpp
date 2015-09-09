#include "interpreter.hpp"

#include <iterator>
#include <utility>
#include <string>
#include <stack>
#include <algorithm>
#include <numeric>

Instruction::Instruction(Type x, Instruction* y) : Command(x), Pointer(y), FunctionPointer(FunctionPointers[Command]) { }

Instruction::Instruction(Type x, value_type y, value_type z) : Command(x), Data{ y, z }, FunctionPointer(FunctionPointers[Command]) { }

ProgramData* Instruction::Parent = nullptr;
Instruction::value_type Instruction::TemporaryValue = 0;

Memory::Front_tag const Memory::Front{};
Memory::Back_tag const Memory::Back{};

const std::array<void(*)(Instruction*), 13> Instruction::FunctionPointers
{
	[](Instruction* x) {},
	[](Instruction* x) { std::advance(Parent->DataPointer, *x->Data); },
	[](Instruction* x) { *Parent->DataPointer += *x->Data; },
	[](Instruction* x) { *Parent->DataPointer = InputByte(); },
	[](Instruction* x) { OutputByte(*Parent->DataPointer); },
	[](Instruction* x) { if (*Parent->DataPointer == 0) Parent->InstructionPointer = x->Pointer; },
	[](Instruction* x) { if (*Parent->DataPointer != 0) Parent->InstructionPointer = x->Pointer; },
	[](Instruction* x) { std::fill_n(Parent->DataPointer, x->Data[0], 0); std::advance(Parent->DataPointer, x->Data[1]); },
	[](Instruction* x) { std::advance(Parent->DataPointer, x->Data[0]); *Parent->DataPointer += x->Data[1] * TemporaryValue; },
	[](Instruction* x) { TemporaryValue = *Parent->DataPointer; *Parent->DataPointer = 0; },
	[](Instruction* x) { while (*Parent->DataPointer) std::advance(Parent->DataPointer, *x->Data); },
	[](Instruction* x) { *Parent->DataPointer = *x->Data; },
	[](Instruction* x) {}
};

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
			if (!Text.back().Data)
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

	// Dirty way to make sure our pointers don't get invalidated later on
	Text.reserve(Source.size());
	// Allows us to call std::vector::back
	Text.emplace_back(Instruction::Type::Nop);

	Instruction::SetParent(this);

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

						if (Text.back() == Instruction::Type::Reset && !Text.back().Data[1]);
						// Detect "[-]>[-]..."
						else if (Text.back() == Instruction::Type::MovePointer
							&& *std::prev(std::cend(Text), 2) == Instruction::Type::Reset)
						{
							/*
							*	If the pointer is only changed by 1 then the program
							*	is clearing a range, allowing us to combine the instructions.
							*	If not, we can always combine the MovePointer with the Reset instruction.
							*/
							std::ptrdiff_t MoveAmount = std::abs(*Text.back().Data);

							if (MoveAmount == 1)
								std::prev(std::end(Text), 2)->Data[0] += *Text.back().Data;

							std::prev(std::end(Text), 2)->Data[1] += *Text.back().Data;

							Text.pop_back();
							
							if (MoveAmount != 1)
								Text.emplace_back(Instruction::Type::Reset, 1, 0);
						}
						else
						{
							Text.emplace_back(Instruction::Type::Reset, 1, 0);
						}

						break;
					}
					else if (Text.back().Command == Instruction::Type::MovePointer)
					{
						auto Data = *Text.back().Data;

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
					Instruction::value_type CurrentOffset = 0;
					Instruction::value_type LastOffset = 0;
					// Offset, Value
					std::list<std::pair<Instruction::value_type, Instruction::value_type>> Operations;

					for (auto Iterator = std::next(JumpTable.top()); Iterator != EndPointer; ++Iterator)
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
									[](auto x, const std::pair<Instruction::value_type, Instruction::value_type>& y) { return x + y.first; }));

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
	{
		auto Temp = InstructionPointer++;
		Temp->FunctionPointer(Temp);
	}
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

Memory_iterator Memory_iterator::operator++(int)
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

Memory_iterator Memory_iterator::operator--(int)
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

void Memory_iterator::Advance(Memory_iterator& Target, const std::ptrdiff_t Delta)
{
	constexpr std::ptrdiff_t Flag = ~std::ptrdiff_t(0xff); // 0xff...ff00 
	const std::ptrdiff_t NewIndex = Target.Index + Delta;

	/*
	*	Check if the iterator has not moved to another page.
	*	Each page can hold 256 (0xff) elements, so the first byte
	*	of the Index stores the index within a page and
	*	the other bytes store the index of the page within
	*	the list.
	*/
	if ((Target.Index & Flag) == (NewIndex & Flag))
		std::advance(Target.Pointer, Delta);
	else if (NewIndex > Target.Parent->Limits.second)
		Target.Pointer = Target.Parent->RequestNewPage(Memory::Back);
	else if (NewIndex < Target.Parent->Limits.first)
		Target.Pointer = Target.Parent->RequestNewPage(Memory::Front);
	else
		Target.Pointer = std::next(std::next(Target.Parent->Origin, NewIndex >> 8)->data(), NewIndex & 0xff);

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

auto Memory::RequestNewPage(const Front_tag) -> iterator::pointer
{
	Storage.emplace_front();

	Limits.first -= Storage.front().size();

	return &Storage.front().back();
}

auto Memory::RequestNewPage(const Back_tag) -> iterator::pointer
{
	Storage.emplace_back();

	Limits.second += Storage.back().size();

	return &Storage.back().front();
}
