#pragma once

#include "memory.hpp"

#include <cstdint>
#include <string>

namespace io
{
	void OutputByte(const Memory::cell_type);

	Memory::cell_type InputByte();

	std::string Open(const std::string&);

	class OutputBuffer
	{
	public:
		explicit OutputBuffer(const std::size_t);
		~OutputBuffer();

		OutputBuffer(const OutputBuffer&) = delete;
		OutputBuffer& operator=(const OutputBuffer&) = delete;

		void Flush();

	private:
		Memory::cell_type* BufferPointer;
	};
}
