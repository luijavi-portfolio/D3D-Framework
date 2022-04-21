#ifndef APP_H
#define APP_H

#include "Graphics.h"

class App
{
public:
	App(class Window& window);
	
	void Run();
private:
	void UpdateLogic();
	void ComposeFrame();
private:
	Window& window_;
	Graphics gfx_;
};

#endif // !APP_H
