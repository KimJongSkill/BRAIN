#include "io.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstring>

static char Buffer[1024];

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
	
	std::cout.rdbuf()->pubsetbuf(Buffer, sizeof(Buffer));

	std::chrono::steady_clock::time_point Start;
	if (Time)
		Start = std::chrono::steady_clock::now();
		
	try
	{
		ProgramData Program(Path);
		Program.Run();
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;
		
		return EXIT_FAILURE;
	}

	if (Time)
		std::cout << "\nProgram execution finished in "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Start).count()
		<< " ms\n";

	return EXIT_SUCCESS;
}
