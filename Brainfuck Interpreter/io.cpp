#include "io.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace io
{
	void OutputByte(const std::uint8_t Character)
	{
		std::cout.put(Character);
	}

	std::uint8_t InputByte()
	{
		return std::cin.get();
	}

	std::string Open(const std::string& Path)
	{
		std::stringstream Stream;
		std::ifstream File;

		File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		File.open(Path);

		Stream << File.rdbuf();

		return Stream.str();
	}

	void CreateOutputBuffer(const std::size_t Size)
	{
		static std::vector<char> Buffer;

		Buffer.resize(Size);
		std::cout.rdbuf()->pubsetbuf(Buffer.data(), Size);
	}
}
