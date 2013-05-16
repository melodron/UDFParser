#pragma once

#include <string>
#include <vector>
#include "UdfReader.hh"
#include "FDiskData.h"

class Command
{
public:
	Command(std::ifstream &udfFile);
	void	exec();

private:
	void	runCommand(std::string const &input);
	void	splitCommand(std::string const &input, std::vector<std::string> &tokens);

	// commands

	void	exit(std::vector<std::string> &command);
	void	fdisk(std::vector<std::string> &command);
	void	help(std::vector<std::string> &command);
	void	cd(std::vector<std::string> &command);
	void	dir(std::vector<std::string> &command);
	void	cp(std::vector<std::string> &command);

private:
	bool		run_;
	FDiskData 	fdiskData_;
	UdfReader	udfReader_;
};