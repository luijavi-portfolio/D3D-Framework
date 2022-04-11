// Set to Unicode (TODO: Maybe move to another file)
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "Window.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int command_show)
{
	Window window(1280, 720, L"My Window");

	MSG message;
	BOOL result;
	while ((result = GetMessage(&message, nullptr, 0, 0)) > 0)
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return message.wParam;
}