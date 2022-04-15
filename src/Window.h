#ifndef WINDOW_H
#define WINDOW_H

#include "LeanWin32.h"

#include "Graphics.h"
#include "ExceptionHandler.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <string>

// Used to grant Graphics class access to handle to the Win32 window
class HandleKey
{
	friend Graphics::Graphics(HandleKey&);
public:
protected:
	HandleKey() = default;
protected:
	HWND handle_ = nullptr;
};


class Window : public HandleKey
{
public:
	// A Window exception handling class
	class Exception : public ExceptionHandler
	{
	public:
		Exception(int line, const char* file, HRESULT hr);
		/*Exception(int line, const char* file, const std::wstring& note);*/
		virtual const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr_;
	};
public:
	Window(int width, int height, const wchar_t* title);
	Window(const wchar_t* title);
	~Window();

	bool ProcessMessage();

	// Helper functions
	void SetTitle(const wchar_t& title);
public:
	Keyboard keyboard;
	Mouse mouse;
private:
	// Win32API-specific functions
	static LRESULT CALLBACK SetupWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK ForwardToWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	
	// Helper function for WNDCLASS
	void CreateWindowClass(WNDCLASSEX& window_class);
private:
	HINSTANCE instance_;
	static constexpr const wchar_t* kWindowClassName_ = L"My Window Class";
	int width_;
	int height_;
};

#endif // !WINDOW_H
