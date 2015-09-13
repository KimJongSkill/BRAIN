#include "io.hpp"
#include "program.hpp"
#include <docopt.h>

#include <iostream>
#include <chrono>
#include <array>

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
	ProgramData Program;

	if (argc < 2)
	{
		std::cout << Documentation << std::endl;

		return EXIT_SUCCESS;
	}
	
	try
	{
		auto Arguments = docopt::docopt(Documentation, { std::next(argv), std::next(argv, argc) }, true);

		if (Arguments["--execute"].asBool())
			Source = std::move(Arguments["SOURCE"].asString());
		else
			Source = io::Open(Arguments["FILE"].asString());
		
		io::CreateOutputBuffer(1024);

		std::chrono::steady_clock::time_point Start;
		std::chrono::steady_clock::time_point ParsingComplete;
		std::chrono::steady_clock::time_point ExecutionComplete;
		if (Arguments["--time"].asBool())
			Start = std::chrono::steady_clock::now();

		Program.Parse(Source);
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
	catch (const std::ios::failure&)
	{
		std::cerr << "Unable to open file\n";
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
