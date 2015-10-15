#pragma once

#include "memory.hpp"

#include <string>

namespace bf
{
	namespace io
	{
		void OutputByte(const Memory::cell_type);

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

		class ProgramInput
		{
		public:
			ProgramInput() = default;
			ProgramInput(std::string, OutputBuffer&);
			~ProgramInput() = default;

			Memory::cell_type GetByte();

		private:
			const std::string InjectedData;
			std::string::size_type Index = 0;
			OutputBuffer& Buffer;
		};
	}
}
