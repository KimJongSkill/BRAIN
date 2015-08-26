#include "io.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

static char Buffer[1024];

static void ShowUsage(std::string Name)
{
	std::string Executable(Name, Name.find_last_of("/\\") + 1);

	std::cout <<
		"BRAIN - Brainfuck Interpreter\n"
		"\n"
		"Usage:\n"
		"	" << Executable << " [-t] [-e] (source | file)\n"
		"	" << Executable << " [-h]\n"
		"\n"
		"Options:\n"
		"	-h, --help	Display this message\n"
		"	-t, --time	Display execution time\n"
		"	-e, --execute	Execute 'source' instead of opening 'file'\n";
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
	Options["-h"] = false;
	Options["--help"] = false;

	try
	{
		std::string Other = ParseArguments(Options, argc, argv);

		if (Options["-h"] || Options["--help"] || Other.empty())
		{
			ShowUsage(argv[0]);

			return EXIT_SUCCESS;
		}

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
		ShowUsage(argv[0]);

		return EXIT_FAILURE;
	}
	catch (const std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
