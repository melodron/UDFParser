#pragma once

#include <string>
#include <vector>

#include "FDiskData.h"

class Command
{
public:
	Command();
	void	exec();

private:
	void	runCommand(std::string const &input);
	void	splitCommand(std::string const &input, std::vector<std::string> &tokens);

	// commands

	void	exit(std::vector<std::string> &command);
	void	fdisk(std::vector<std::string> &command);
	void	help(std::vector<std::string> &command);

private:
	bool	run_;
	FDiskData data_;
};