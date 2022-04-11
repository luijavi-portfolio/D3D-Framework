#include "Window.h"
#include <cassert>

Window::Window(int width, int height, const wchar_t* title)
	:
	instance_(GetModuleHandle(nullptr)),
	width_(width),
	height_(height)
{
	// Register windows class
	WNDCLASSEX wc = {};
	CreateWindowClass(wc);
	RegisterClassEx(&wc);

	// Create window
	handle_ = CreateWindow(kWindowClassName_, title,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, width_, height_,
		nullptr, nullptr, instance_, this);

	// Show dat window
	ShowWindow(handle_, SW_SHOWDEFAULT);
	UpdateWindow(handle_);
}

Window::~Window()
{
	DestroyWindow(handle_);
	UnregisterClass(kWindowClassName_, instance_);
}

void Window::SetTitle(const wchar_t& title)
{
	SetWindowText(handle_, &title);
}

LRESULT Window::SetupWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_NCCREATE)
	{
		const CREATESTRUCTW* const p_create = reinterpret_cast<CREATESTRUCTW*>(lparam);
		Window* const p_window = static_cast<Window*>(p_create->lpCreateParams);
		SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_window));
		SetWindowLongPtr(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::ForwardToWindowProcedure));
		return p_window->WindowProcedure(handle, message, wparam, lparam);
	}

	return DefWindowProc(handle, message, wparam, lparam);
}

LRESULT Window::ForwardToWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	Window* const p_window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));
	return p_window->WindowProcedure(handle, message, wparam, lparam);
}

LRESULT Window::WindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		} break;
	}

	return DefWindowProc(handle, message, wparam, lparam);
}

void Window::CreateWindowClass(WNDCLASSEX& window_class)
{
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = SetupWindowProcedure;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance_;
	window_class.hIcon = nullptr;
	window_class.hCursor = nullptr;
	window_class.hbrBackground = nullptr;
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = kWindowClassName_;
	window_class.hIconSm = nullptr;

	RegisterClassEx(&window_class);
}
