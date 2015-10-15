#pragma once

#include <exception>

namespace exception
{
	class UnmatchedOpen : public std::exception
	{
	public:
		UnmatchedOpen() : exception("Unmatched '['\n") {};
		~UnmatchedOpen() = default;
	};

	class UnmatchedClose : public std::exception
	{
	public:
		UnmatchedClose() : exception("Unmatched ']'\n") {};
		~UnmatchedClose() = default;
	};
}