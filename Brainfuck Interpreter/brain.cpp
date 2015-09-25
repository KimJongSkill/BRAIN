#include "io.hpp"
#include "program.hpp"
#include <docopt.h>

#include <iostream>
#include <chrono>
#include <array>

const std::string Documentation
{
R"(BRAIN - Brainfuck Interpreter

    Usage:
      brain [--time] [--buffer SIZE] (--execute SOURCE | FILE)
      brain [--help]

    Options:
      -h, --help         Display this help message.
      -t, --time         Display parsing and execution time.
      -e, --execute      Execute provided SOURCE instead of reading from FILE.
      -b, --buffer SIZE  Set Output Buffer size in bytes. A value of 0 disables the buffer. [default: 512])"
};

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

		Source = Arguments["--execute"].asBool() ? Arguments["SOURCE"].asString() : io::Open(Arguments["FILE"].asString());

		io::OutputBuffer Buffer(std::stoll(Arguments["--buffer"].asString()));

		std::chrono::steady_clock::time_point Start = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point ParsingComplete;
		std::chrono::steady_clock::time_point ExecutionComplete;

		Program.From(Source);
		ParsingComplete = std::chrono::steady_clock::now();

		Program.Run();
		ExecutionComplete = std::chrono::steady_clock::now();

		if (Arguments["--time"].asBool())
		{
			io::LogMessage('\n', false);
			io::Log << "Parsing finished in "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(ParsingComplete - Start).count()
				<< " ms\n";
			io::Log << "Program execution finished in "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(ExecutionComplete - Start).count()
				<< " ms\n";
		}
	}
	catch (const std::ios::failure&)
	{
		io::LogMessage("Unable to open file\n");
	}
	catch (const std::exception& Exception)
	{
		io::LogMessage(Exception.what() + '\n');

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
