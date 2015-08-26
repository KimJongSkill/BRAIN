#include "io.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

static char Buffer[1024];

void Usage()
{
	std::cout << "BRAIN: Brainfuck Interpreter\n\n";
	std::cout << "BRAIN [-t/--time] [-e/--execute source] [file]\n\n";
	std::cout << "\t-t/--time\tDisplay execution time and instruction count\n";
	std::cout << "\t-e/--execute\tExecute source string instead of opening a file\n";
	std::cout << "\tsource\t\tSpecifies the string containing the Brainfuck source to execute\n";
	std::cout << "\tfile\t\tSpecifies the Brainfuck source file to execute\n";
}

std::string ParseArguments(std::unordered_map<std::string, bool>& Options, int Count, const char* Arguments[])
{
	std::vector<std::string> ArgumentList;
	ArgumentList.reserve(Count - 1);
	for (int i = 1; i < Count; ++i)
		ArgumentList.emplace_back(Arguments[i]);

	std::string ReturnValue;

	for (auto Argument : ArgumentList)
	{
		auto Result = Options.find(Argument);
		if (Result != Options.end())
			Result->second = true;
		else if (ReturnValue.empty())
			ReturnValue = std::move(Argument);
		else
			throw std::out_of_range("Invalid arguments");
	}

	return ReturnValue;
}

int main(int argc, const char* argv[])
{
	std::string Source;
	bool Time = false;

	std::unordered_map<std::string, bool> Expected;
	Expected["-t"] = false;
	Expected["--time"] = false;
	Expected["-e"] = false;
	Expected["--execute"] = false;

	try
	{
		std::string Other = ParseArguments(Expected, argc, argv);

		if (Expected["-t"] || Expected["--time"])
			Time = true;
		if (Expected["-e"] || Expected["--execute"])
			Source = std::move(Other);
		else
			Source = Open(Other);

		std::cout.rdbuf()->pubsetbuf(Buffer, sizeof(Buffer));

		std::chrono::steady_clock::time_point Start;
		if (Time)
			Start = std::chrono::steady_clock::now();

		ProgramData Program(Source);
		Program.Run();

		if (Time)
			std::cout << "\nProgram execution finished in "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Start).count()
			<< " ms\n";
	}
	catch (const std::out_of_range&)
	{
		Usage();

		return EXIT_FAILURE;
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
