#include <iostream>
#include <sstream>
#include <iterator>
#include <stdlib.h>
#include "Command.hpp"

Command::Command() :
	run_(true)
{
	//TODO: fill FDiskData by calling the appropriate function in UDFReader
}

void	Command::exec()
{
	std::string input;

	while (run_ && std::cin.good())
	{
		std::cout << ">"; // TODO: pwd
		std::getline(std::cin, input);
		runCommand(input);
	}
}

void	Command::runCommand(std::string const &input)
{
	static const struct
	{
		std::string	name;
		void (Command::*ptr)(std::vector<std::string>&);
	}	commands[]	 =
		{
			{"exit", &Command::exit},
			{"fdisk", &Command::fdisk},
			{"help", &Command::help}
		};

	std::vector<std::string> tokens;
	splitCommand(input, tokens);

	if (tokens.size() == 0)
		return;

	size_t i = 0;
	for (; i < sizeof(commands) / sizeof(commands[0]); ++i)
	{
		if (tokens[0] == commands[i].name)
		{
			(this->*commands[i].ptr)(tokens);
			break;
		}
	}
	if (i == sizeof(commands) / sizeof(commands[0]))
		std::cerr << "Error: " << tokens[0] 
				  << ": unrecognized command (try help?)" << std::endl;
}

void	Command::splitCommand(std::string const &input, std::vector<std::string> &tokens)
{
	std::istringstream iss(input);

	std::copy(std::istream_iterator<std::string>(iss),
         std::istream_iterator<std::string>(),
         std::back_inserter<std::vector<std::string> >(tokens));	
}

void	Command::help(std::vector<std::string> &/*command*/)
{
	std::cout << "help: command list" << std::endl
			  << "cd: change directory" << std::endl
			  << "dir: show directory content" << std::endl
			  << "fdisk: show disk information" << std::endl
			  << "cp: copy files" << std::endl
			  << "exit: quit program" << std::endl;
}

void	Command::fdisk(std::vector<std::string> &/*command*/)
{
}

void	Command::exit(std::vector<std::string> &/*command*/)
{
	::exit(EXIT_SUCCESS);
}