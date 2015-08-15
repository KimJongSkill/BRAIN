#include "io.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstring>

void Usage()
{
	std::cout << "BRAIN: Brainfuck Interpreter\n\n";
	std::cout << "BRAIN [-t/--time] file\n\n";
	std::cout << "\t-t/--time\tDisplay execution time and instruction count\n";
	std::cout << "\tfile\tSpecifies the Brainfuck source file to execute\n";
}

int main(int argc, char* argv[])
{
	char* Path = nullptr;
	bool Time = false;

	if (argc == 2)
	{
		Path = argv[1];
	}
	else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[1], "--time")))
	{
		Time = true;
		Path = argv[2];
	}
	else
	{
		Usage();
		return EXIT_FAILURE;
	}

	std::chrono::steady_clock::time_point Start;
	std::size_t Instructions = 0;
	if (Time)
		Start = std::chrono::steady_clock::now();
		
	try
	{
		Open(Path);

		while (NotEOF())
		{
			if (Time)
				++Instructions;
			ProcessCharacter(GetNextInstruction());
		}
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;
		
		throw;

		return EXIT_FAILURE;
	}

	if (Time)
		std::cout << "\nExecuted " << Instructions << " instructions in "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Start).count()
		<< " ms\n";

	return EXIT_SUCCESS;
}
