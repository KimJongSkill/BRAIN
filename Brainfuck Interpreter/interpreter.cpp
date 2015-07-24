#include "interpreter.hpp"
#include "io.hpp"

#include <list>
#include <stack>
#include <fstream>
#include <iterator>

static std::list<std::uint8_t> Memory { 0 };
static std::list<std::uint8_t>::iterator Pointer = Memory.begin();
static std::stack<std::streampos> JumpTable;

static bool IsFirst(std::list<std::uint8_t>::iterator Pointer)
{
	return Pointer == Memory.begin();
}

static bool IsLast(std::list<std::uint8_t>::iterator Pointer)
{
	return (Pointer != Memory.end()) && (std::next(Pointer) == Memory.end());
}

void ProcessCharacter(char Command)
{
	switch (Command)
	{
	case '>':
		if (IsLast(Pointer))
			Memory.emplace_back(0);
		
			std::advance(Pointer, 1);
		break;
	case '<':
		if (IsFirst(Pointer))
			Memory.emplace_front(0);
		
			std::advance(Pointer, -1);
		break;
	case '+':
		++*Pointer;
		break;
	case '-':
		--*Pointer;
		break;
	case '.':
		OutputByte(*Pointer);
		break;
	case ',':
		*Pointer = InputByte();
		break;
	case '[':
		if (*Pointer)
			JumpTable.push(GetNextInstructionPointer());
		else
			Jump(']');
		break;
	case ']':
		if (*Pointer)
			Jump(JumpTable.top());
		else
			JumpTable.pop();
	default:
		break;
	}
}
