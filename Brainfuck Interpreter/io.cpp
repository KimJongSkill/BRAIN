#include "io.hpp"

#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <sstream>

static std::string Source;
static std::string::size_type InstructionPointer;

void OutputByte(const std::uint8_t Character)
{
	std::cout << Character;
}

std::uint8_t InputByte()
{
	return std::cin.get();
}

std::string::size_type GetNextInstructionPointer()
{
	return InstructionPointer;
}

void Jump(std::string::size_type NewPointer)
{
	InstructionPointer = NewPointer;
}

void JumpEndOfLoop()
{
	int Counter = 1;
	std::string::size_type Index = InstructionPointer;

	do
	{
		Index = Source.find_first_of("[]", Index + 1);
		
		if (Index == std::string::npos)
			throw std::runtime_error(std::string("Unbalanced '[' @ character ") + std::to_string(GetNextInstructionPointer() - 1));
		else if (Source[Index] == ']')
			--Counter;
		else if (Source[Index] == '[')
			++Counter;
	} while (Counter);

	// We need the +1 to skip the ']' character
	Jump(Index + 1);
}

void Open(const char* const Path)
{
	std::stringstream Stream;
	std::ifstream File;
	
	File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	File.open(Path);

	Stream << File.rdbuf();

	Source = Stream.str();
	InstructionPointer = 0;
}

char GetNextInstruction()
{
	return Source.at(InstructionPointer++);
}

bool NotEOF()
{
	return InstructionPointer != Source.size();
}
