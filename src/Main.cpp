// Set to Unicode (TODO: Maybe move to another file)
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "Window.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int command_show)
{
	try
	{
		Window window(1280, 768, L"My Window");
	
		while (window.ProcessMessage())
		{
			// Game to run here
		}
	}
	catch (const ExceptionHandler& e)
	{
		MessageBoxA(nullptr,  e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBoxW(nullptr, L"No details available", L"Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;
}