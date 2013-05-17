#include <iostream>
#include <sstream>
#include <iterator>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#include "Command.hpp"

Command::Command(std::ifstream &udfFile) :
	run_(true), udfReader_(udfFile)
{
   udfReader_.getFDiskData(fdiskData_);
}

void	Command::exec()
{
	std::string input;

	while (run_ && std::cin.good())
	{
		std::cout << ":/" << udfReader_.getCurrentDirectory()->getName() << ">";
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
			{"cat", &Command::cat},
			{"cd", &Command::cd},			
			{"cp", &Command::cp},
			{"dir", &Command::dir},
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

void	Command::cat(std::vector<std::string> &command)
{
  if (command.size() == 2)
    udfReader_.readFile(command[1]);
  else
    std::cout << "Usage: cat file" << std::endl;
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
	// volume identifier
	std::cout << this->fdiskData_.volumeIdentifier << std::endl;

	// recording time
	struct
  	{
	    unsigned type:4;
	    unsigned timezone:12;
	} typeAndTimezone;

	typeAndTimezone.type = fdiskData_.recordingDateAndTime->typeAndTimezone >> 12;
	typeAndTimezone.timezone = fdiskData_.recordingDateAndTime->typeAndTimezone & 0b111111111;
	std::cout << "Record Time: "
	          << fdiskData_.recordingDateAndTime->year << "-"
	          << (int)fdiskData_.recordingDateAndTime->month << "-"
	          << (int)fdiskData_.recordingDateAndTime->day << "\t"
	          << (int)fdiskData_.recordingDateAndTime->hour << ":"
	          << (int)fdiskData_.recordingDateAndTime->minute << ":";
	std::cout.width(2);
	std::cout.fill('0');
	std::cout << (int)fdiskData_.recordingDateAndTime->second;
	if (typeAndTimezone.timezone >= -1440 && typeAndTimezone.timezone <= 1440)
	{
		std::cout << " (UTC: " 
	              << std::showpos << typeAndTimezone.timezone / 60 << ")";
	}
	std::cout << std::noshowpos << std::endl;

	// version
	std::cout << "UDF " << std::fixed << std::setprecision(2)
			  << floor(fdiskData_.version / 10) / 100 << std::endl;
	
	// space
	std::cout << "Disk size: " << fdiskData_.totalSpace << std::endl;
	std::cout << "Disk free size: " << fdiskData_.freeSpace << std::endl;
}

void	Command::exit(std::vector<std::string> &/*command*/)
{
	run_ = false;
}

void	Command::cd(std::vector<std::string> &command)
{
	if (command.size() > 2)
	{
		std::cout << "Usage: cd [directory]" << std::endl;
	}
	else if (command.size() == 1)
	{
		udfReader_.chdir("/");
	}
	else
	{
		udfReader_.chdir(command[1]);
	}
}

void	Command::dir(std::vector<std::string> &command)
{
	if (command.size() > 2)
		std::cout << "Usage: dir" << std::endl;	
	else
		udfReader_.listDirectory();
}
	
void	Command::cp(std::vector<std::string> &command)
{
	if (command.size() != 3)
		std::cout << "Usage: cp src dest" << std::endl;	
	else
		udfReader_.copy(command[1], command[2]);
}

