#include "io.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

static char Buffer[1024];

void ShowUsage()
{
	std::cout <<
		"BRAIN - Brainfuck Interpreter\n\n"
		"BRAIN [-t, --time] [-e, --execute] source|file\n\n"
		"\t-t, --time\tDisplay execution time\n"
		"\t-e, --execute\tExecute 'source' instead of opening 'file'\n";
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

	std::unordered_map<std::string, bool> Options;
	Options["-t"] = false;
	Options["--time"] = false;
	Options["-e"] = false;
	Options["--execute"] = false;

	try
	{
		std::string Other = ParseArguments(Options, argc, argv);

		if (Options["-e"] || Options["--execute"])
			Source = std::move(Other);
		else
			Source = Open(Other);

		std::cout.rdbuf()->pubsetbuf(Buffer, sizeof(Buffer));

		std::chrono::steady_clock::time_point Start;
		if (Options["-t"] || Options["--time"])
			Start = std::chrono::steady_clock::now();

		ProgramData Program(Source);
		Program.Run();

		if (Options["-t"] || Options["--time"])
			std::cout << "\nProgram execution finished in "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Start).count()
			<< " ms\n";
	}
	catch (const std::out_of_range&)
	{
		ShowUsage();

		return EXIT_FAILURE;
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
