#include "io.hpp"
#include "interpreter.hpp"
#include <docopt.h>

#include <iostream>
#include <chrono>
#include <array>

static std::array<char, 1024> Buffer;

const std::string Documentation =
R"(BRAIN - Brainfuck Interpreter

    Usage:
      brain [--time] (--execute SOURCE | FILE)
      brain [--help]

    Options:
      -h, --help    Show this screen.
      -t, --time    Display execution time.
      -e --execute  Execute provided SOURCE instead of
                    reading from FILE)";

int main(int argc, const char* argv[])
{
	std::string Source;

	if (argc < 2)
	{
		std::cout << Documentation << std::endl;

		return EXIT_SUCCESS;
	}
	
	auto Arguments = docopt::docopt(Documentation, { std::next(argv), std::next(argv, argc) }, true);

	if (Arguments["--execute"].asBool())
		Source = std::move(Arguments["SOURCE"].asString());
	else
		Source = Open(Arguments["FILE"].asString());

	std::cout.rdbuf()->pubsetbuf(Buffer.data(), std::size(Buffer));

	std::chrono::steady_clock::time_point Start;
	std::chrono::steady_clock::time_point ParsingComplete;
	std::chrono::steady_clock::time_point ExecutionComplete;
	if (Arguments["--time"].asBool())
		Start = std::chrono::steady_clock::now();

	try
	{
		ProgramData Program(Source);
		ParsingComplete = std::chrono::steady_clock::now();

		Program.Run();
		ExecutionComplete = std::chrono::steady_clock::now();

		if (Arguments["--time"].asBool())
			std::cout << "\nParsing finished in "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(ParsingComplete - Start).count()
			<< " ms\n"
			<< "Program execution finished in "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(ExecutionComplete - Start).count()
			<< " ms\n";
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
