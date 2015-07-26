#include "io.hpp"

#include <iostream>
#include <fstream>
#include <limits>

static std::ifstream File;

void OutputByte(std::uint8_t Character)
{
	std::cout << Character;
}

std::uint8_t InputByte()
{
	return std::cin.get();
}

std::streampos GetNextInstructionPointer()
{
	return File.tellg();
}

void Jump(std::streampos InstructionPointer)
{
	File.seekg(InstructionPointer);
}

void Jump(char Character)
{
	File.ignore(std::numeric_limits<std::streamsize>::max(), Character);
}

void Open(char* Path)
{
	File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	File.open(Path);
}

char GetNextInstruction()
{
	return File.get();
}

bool FileIsGood()
{
	return File.good();
}
