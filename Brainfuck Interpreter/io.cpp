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

void Jump(const char Character)
{
	Source.find_first_of(Character, InstructionPointer);
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
