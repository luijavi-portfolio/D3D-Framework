#include "ExceptionHandler.h"
#include <sstream>

ExceptionHandler::ExceptionHandler(int line, const char* file) noexcept
	:
	line_(line),
	file_(file)
{
}

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

const std::string ExceptionHandler::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[FILE]: " << file_ << '\n'
		<< "[LINE]: " << line_;
	return oss.str();
}
