#include "io.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

void OutputByte(const std::uint8_t Character)
{
	std::cout << Character;
}

std::uint8_t InputByte()
{
	return std::cin.get();
}

std::string Open(const char* const Path)
{
	std::stringstream Stream;
	std::ifstream File;
	
	File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	File.open(Path);

	Stream << File.rdbuf();

	return Stream.str();
}
