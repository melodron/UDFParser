#pragma once

#include <string>
#include <vector>

class Command
{
public:
	Command();
	void	exec();

private:
	void	runCommand(std::string const &input);
	void	splitCommand(std::string const &input, std::vector<std::string> &tokens);

	// commands

	void	help(std::vector<std::string> &command);

private:
	bool	run_;
};