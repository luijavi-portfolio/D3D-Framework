// Set to Unicode (TODO: Maybe move to another file)
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "Window.h"
#include "App.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int command_show)
{
	try
	{
		Window window(1280, 768, L"My Window");
		App game_app(window);
	
		while (window.ProcessMessage())
		{
			game_app.Run();
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
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;
}