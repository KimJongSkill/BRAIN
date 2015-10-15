#include "io.hpp"
#include "program.hpp"
#include <docopt.h>

#include <iostream>
#include <chrono>

const std::string Documentation
{
R"(BRAIN - Brainfuck Interpreter

    Usage:
      brain [--time] [--buffer SIZE] [--input STRING] (--execute SOURCE | FILE)
      brain [--help]

    Options:
      -h, --help         Display this help message.
      -t, --time         Display parsing and execution time.
      -e, --execute      Execute provided SOURCE instead of reading from FILE.
      -b, --buffer SIZE  Set Output Buffer size in bytes. A value of 0 disables the buffer. [default: 512]
      -i, --input INPUT  INPUT is provided as program input before resorting to standard input.)"
};

int main(int argc, const char* argv[])
{
	std::string Source;

	if (argc < 2)
	{
		std::cout << Documentation << std::endl;

		return EXIT_SUCCESS;
	}
	
	try
	{
		auto Arguments = docopt::docopt(Documentation, { std::next(argv), std::next(argv, argc) }, true);

		Source = Arguments["--execute"].asBool() ? Arguments["SOURCE"].asString() : bf::io::Open(Arguments["FILE"].asString());

		bf::io::OutputBuffer Buffer(std::stoll(Arguments["--buffer"].asString()));
		bf::io::ProgramInput Input(Arguments["--input"].isString() ? Arguments["--input"].asString() : "", Buffer);
		ProgramData Program(Input);

		auto Start = std::chrono::steady_clock::now();

		Program.From(Source);
		auto ParsingComplete = std::chrono::steady_clock::now();

		Program.Run();
		auto ExecutionComplete = std::chrono::steady_clock::now();
		
		if (Arguments["--time"].asBool())
		{
			bf::io::LogMessage('\n', false);
			bf::io::Log << "Parsing finished in "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(ParsingComplete - Start).count()
				<< " ms\n";
			bf::io::Log << "Program execution finished in "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(ExecutionComplete - Start).count()
				<< " ms\n";
		}
	}
	catch (const std::ios::failure&)
	{
		bf::io::LogMessage("Unable to open file\n");
	}
	catch (const std::exception& Exception)
	{
		bf::io::LogMessage(Exception.what());

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
