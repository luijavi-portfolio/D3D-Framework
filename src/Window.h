#ifndef WINDOW_H
#define WINDOW_H

#include "Graphics.h"

#include "LeanWin32.h"
#include <string>

class Handle
{

};


class Window
{
public:
	Window(int width, int height, const wchar_t* title);
	~Window();
	void SetTitle(const wchar_t& title);
	
private:
	// Win32API-specific functions
	static LRESULT CALLBACK SetupWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK ForwardToWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);
	
	// Helper function for window class
	void CreateWindowClass(WNDCLASSEX& window_class);
private:
	HWND handle_;
	HINSTANCE instance_;
	static constexpr const wchar_t* kWindowClassName_ = L"My Window Class";
	int width_;
	int height_;
};

#endif // !WINDOW_H
