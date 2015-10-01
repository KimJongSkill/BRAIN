#pragma once

#include "memory.hpp"

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

	struct MessageLog
	{
		bool Flag;
	} constexpr Log{ true };

	template <class Type>
	constexpr void LogMessage(const Type& Message, bool Format = true)
	{
		if (Format)
			std::clog << "[BRAIN] ";

		std::clog << Message;
	}

	template <class Type>
	const MessageLog operator<<(const MessageLog& Object, const Type& Message)
	{
		LogMessage(Message, Object.Flag);
		
		constexpr MessageLog Temp{ false };
		return Temp;
	}
}
