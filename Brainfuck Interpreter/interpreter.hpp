#pragma once

#include "io.hpp"

#include <vector>
#include <list>
#include <array>
#include <cstdint>
#include <iterator>
#include <functional>

class Memory_iterator : public std::iterator<std::random_access_iterator_tag, char>
{
	friend class Memory;

public:
	bool operator==(const Memory_iterator&) const;
	bool operator!=(const Memory_iterator&) const;

	reference operator*() const;
	pointer operator->() const;

	Memory_iterator& operator++();
	Memory_iterator& operator++(int);

	Memory_iterator& operator--();
	Memory_iterator& operator--(int);

	difference_type operator-(const Memory_iterator&) const;

	Memory_iterator operator+(difference_type) const;
	Memory_iterator operator-(difference_type) const;

	bool operator<(const Memory_iterator&) const;
	bool operator>(const Memory_iterator&) const;

	bool operator<=(const Memory_iterator&) const;
	bool operator>=(const Memory_iterator&) const;

	Memory_iterator& operator+=(difference_type);
	Memory_iterator& operator-=(difference_type);

	reference operator[](difference_type) const;

private:
	static void Advance(Memory_iterator&, std::ptrdiff_t);

	difference_type Index;
	pointer Pointer;
	Memory* Parent;
};

class Memory
{
	friend Memory_iterator;

public:
	typedef Memory_iterator iterator;

	iterator begin() const;
	iterator end() const;

private:
	static struct Front_tag {} Front;
	static struct Back_tag {} Back;

	iterator::pointer RequestNewPage(Front_tag);
	iterator::pointer RequestNewPage(Back_tag);

	std::list<std::array<char, 256>> Storage{ { 0 } };
	std::list<std::array<char, 256>>::iterator Origin = Storage.begin();
	std::pair<std::ptrdiff_t, std::ptrdiff_t> Limits{ 0, 255 };
};

class ProgramData
{
	friend class Instruction;

	std::vector<Instruction> Text;
	std::vector<Instruction>::pointer InstructionPointer;
	Memory Cells;
	Memory::iterator DataPointer = std::begin(Cells);

public:
	explicit ProgramData(const std::string& Source);
	~ProgramData();

	void Run();
};

class Instruction
{
	friend ProgramData;

public:
	enum Type { Nop, MovePointer, Addition, Input, Output, LoopStart, LoopEnd, Reset, Multiplication, Store, Seek, Set, Stop };
	typedef int value_type;

	Instruction(Type, Instruction*);
	Instruction(Type, value_type = 0, value_type = 0);

	bool operator==(Type) const;

	static void SetParent(ProgramData*);
	static void Orphan(ProgramData*);

private:
	const Type Command;
	union
	{
		value_type Data[2];
		Instruction* Pointer;
	};

	static ProgramData* Parent;
	static value_type TemporaryValue;

	const std::array<std::function<void(void)>, 13> FunctionPointers
	{
		[this] {},
		[this] { std::advance(Parent->DataPointer, *Data); },
		[this] { *Parent->DataPointer += *Data; },
		[this] { *Parent->DataPointer = InputByte(); },
		[this] { OutputByte(*Parent->DataPointer); },
		[this] { if (*Parent->DataPointer == 0) Parent->InstructionPointer = Pointer; },
		[this] { if (*Parent->DataPointer != 0) Parent->InstructionPointer = Pointer; },
		[this] { std::fill_n(Parent->DataPointer, Data[0], 0); std::advance(Parent->DataPointer, Data[1]); },
		[this] { std::advance(Parent->DataPointer, Data[0]); *Parent->DataPointer += Data[1] * TemporaryValue; },
		[this] { TemporaryValue = *Parent->DataPointer; *Parent->DataPointer = 0; },
		[this] { while (*Parent->DataPointer) std::advance(Parent->DataPointer, *Data); },
		[this] { *Parent->DataPointer = *Data; },
		[this] {}
	};
	const std::function<void(void)> FunctionPointer;
};

inline bool operator==(Instruction::Type x, const Instruction& y);
inline bool operator!=(const Instruction& x, Instruction::Type y);
inline bool operator!=(Instruction::Type x, const Instruction& y);
