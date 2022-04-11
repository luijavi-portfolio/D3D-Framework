#ifndef WINDOW_H
#define WINDOW_H

#include "Graphics.h"

#include "LeanWin32.h"
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
	Window(int width, int height, const wchar_t* title);
	~Window();

	bool ProcessMessage();

	// Helper functions
	void SetTitle(const wchar_t& title);
	
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
