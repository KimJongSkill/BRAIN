#include "io.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace io
{
	void OutputByte(const Memory::cell_type Character)
	{
		std::cout.put(Character);
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

	OutputBuffer::OutputBuffer(const std::size_t Size) : BufferPointer(nullptr)
	{
		try
		{
			if (Size)
			{
				BufferPointer = new Memory::cell_type[Size];
				std::cout.rdbuf()->pubsetbuf(BufferPointer, Size);
			}
		}
		catch (const std::bad_alloc&)
		{
			io::LogMessage("Unable to allocate output buffer\n");
		}
	}

	OutputBuffer::~OutputBuffer()
	{
		if (BufferPointer)
		{
			Flush();
			delete[] BufferPointer;
		}
	}

	void OutputBuffer::Flush()
	{
		std::cout.flush();
	}

	ProgramInput::ProgramInput(std::string Injection) : InjectedData(std::move(Injection) + '\n') { }
	
	Memory::cell_type ProgramInput::GetByte()
	{
		if (Index < InjectedData.length())
			return InjectedData[Index++];
		else
			return std::cin.get();
	}
}
