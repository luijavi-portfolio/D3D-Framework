#include "Window.h"
#include "App.h"

App::App(Window& window)
	:
	window_(window)
{
}

void App::Run()
{
	UpdateLogic();
}

void App::UpdateLogic()
{

}
