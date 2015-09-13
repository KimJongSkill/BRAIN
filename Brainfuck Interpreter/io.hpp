#pragma once

#include "memory.hpp"

#include <cstdint>
#include <string>

namespace io
{
	void OutputByte(const Memory::cell_type);

	Memory::cell_type InputByte();

	std::string Open(const std::string&);

	void CreateOutputBuffer(const std::size_t);
}
