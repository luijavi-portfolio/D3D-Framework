#include "ExceptionHandler.h"
#include <sstream>

ExceptionHandler::ExceptionHandler(int line, const char* file) noexcept
	:
	file_(file),
	line_(line)
{ }

ExceptionHandler::ExceptionHandler(int line, const char* file, const std::string& note) noexcept
	:
	note_(note),
	file_(file),
	line_(line)
{ }

const char* ExceptionHandler::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();


	return (what_buffer_ = oss.str()).c_str();
}

const char* ExceptionHandler::GetType() const noexcept
{
	return "Exception Handler Exception";
}

int ExceptionHandler::GetLine() const noexcept
{
	return line_;
}

const std::string& ExceptionHandler::GetFile() const noexcept
{
	return file_;
}

const std::string& ExceptionHandler::GetNote() const noexcept
{
	return note_;
}

const std::string ExceptionHandler::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[FILE]: " << file_ << '\n'
		<< "[LINE]: " << line_ << '\n'
		<< "[NOTE]: " << note_;
	return oss.str();
}
