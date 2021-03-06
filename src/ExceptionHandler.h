#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include <exception>
#include <string>

// Source: https://github.com/planetchili/chili_framework/blob/master/Engine/ChiliException.h
class ExceptionHandler : public std::exception
{
public:
	ExceptionHandler(int line, const char* file) noexcept;
	ExceptionHandler(int line, const char* file, const std::string& note) noexcept;
	const char* what() const noexcept override;	// from std::noexcept
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	const std::string& GetNote() const noexcept;
	const std::string GetOriginString() const noexcept;
protected:
	mutable std::string what_buffer_;
private:
	std::string note_;
	std::string file_;
	int line_;

};
#endif // !EXCEPTION_HANDLING_H
