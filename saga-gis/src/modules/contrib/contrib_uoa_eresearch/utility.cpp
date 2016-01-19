#include "utility.h"

void Util::Error(std::string message, int exit_code)
{
	std::cerr << "ERROR: " << message << std::endl;
	exit(exit_code);
}

void Util::Warning(std::string message)
{
	std::cout << "WARNING: " << message << std::endl;
}

void Util::Info(std::string message)
{
	std::cout << "INFO: " << message << std::endl;
}