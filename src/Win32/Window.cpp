#include "Window.h"
#include "..\Graphics.h"
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

	handle_ = nullptr;

	// Check that handle_ is valid
	if (!handle_)
	{
		// TODO: Will need to handle this better - currently not providing useful info
		throw (Window::Exception(__LINE__, __FILE__, "Invalid handle!"));
	}

	// Show dat window
	ShowWindow(handle_, SW_SHOWDEFAULT);
	UpdateWindow(handle_);
}

Window::Window(const wchar_t* title)
	:
	Window(Graphics::kScreenWidth,Graphics::kScreenHeight,title)
{ }

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

		// Handle what happens when window goes out of focus
		case WM_KILLFOCUS:
		{
			keyboard.ClearState();
		} break;

		// Keyboard messages, included WM_SYSKEY for the Alt and F10 keys
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// Account of autorepeat
			// Source: https://docs.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
			if (!(lparam & KF_REPEAT) || keyboard.AutorepeatIsEnabled())
			{
				keyboard.OnKeyPressed(static_cast<unsigned char>(wparam));
			}
		} break;
		
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			keyboard.OnKeyReleased(static_cast<unsigned char>(wparam));
		} break;

		case WM_CHAR:
		{
			keyboard.OnChar(static_cast<unsigned char>(wparam));
		} break;

		// Mouse messages
		case WM_MOUSEMOVE:
		{
			// Source: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
			// Source: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-makepoints
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnMouseMove(pt.x, pt.y);
		} break;

		case WM_LBUTTONDOWN:
		{
			// Source: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnLeftIsPressed(pt.x, pt.y);

		} break;

		case WM_LBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnLeftIsReleased(pt.x, pt.y);
		} break;

		case WM_RBUTTONDOWN:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightIsPressed(pt.x, pt.y);
		} break;

		case WM_RBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightIsReleased(pt.x, pt.y);
		} break;

		case WM_MOUSEWHEEL:
		{
			const POINTS pt = MAKEPOINTS(lparam);

			if (GET_WHEEL_DELTA_WPARAM(wparam) > 0)
			{
				mouse.OnWheelUp(pt.x, pt.y);
			}
			else if (GET_WHEEL_DELTA_WPARAM(wparam) < 0)
			{
				mouse.OnWheelDown(pt.x, pt.y);
			}

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
Window::Exception::Exception(int line, const char* file, HRESULT hr)
	:
	ExceptionHandler(line, file),
	hr_(hr)
{}

Window::Exception::Exception(int line, const char* file, const std::string& note)
	:
	ExceptionHandler(line, file, note),
	hr_(E_FAIL)
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
	DWORD message_length = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&message_buffer), 0, nullptr);

	if (!message_length)
	{
		return "Unidentified error code!";
	}

	/*std::wstring ws(message_buffer);
	std::string error_string(ws.begin(),ws.end());*/
	std::string error_message = message_buffer;
	LocalFree(message_buffer);
	return error_message;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr_;
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr_);
}
