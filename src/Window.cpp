#include "Window.h"
#include <cassert>
#include <sstream>

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

bool Window::ProcessMessage()
{
	MSG message;
	BOOL result;
	while ((result = PeekMessage(&message, nullptr, 0, 0,PM_REMOVE)) > 0)
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);

		if (message.message == WM_QUIT)
		{
			return false;
		}

	}

	return true;
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

// Window Exception implementation
Window::Exception::Exception(int line, const char* file, HRESULT hr) noexcept
	:
	ExceptionHandler(line, file),
	hr_(hr)
{}

const char* Window::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << '\n'
		<< "[ERROR CODE]: " << GetErrorCode() << '\n'
		<< "[DESCRIPTION]: " << GetErrorString() << '\n'
		<< GetOriginString();

	return (what_buffer_ = oss.str()).c_str();
}

const char* Window::Exception::GetType() const noexcept
{
	return "Exception Handler Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* message_buffer = nullptr;
	// Win32 Message formating function
	// Source: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessage
	DWORD message_length = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&message_buffer), 0, nullptr);

	if (!message_length)
	{
		return "Unidentified error code!";
	}

	std::string error_string = message_buffer;
	LocalFree(message_buffer);
	return error_string;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr_;
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr_);
}
