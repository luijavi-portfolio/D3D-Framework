#ifndef APP_H
#define APP_H


class App
{
public:
	App(class Window& window);
	
	void Run();
private:
	void UpdateLogic();
private:
	Window& window_;
};

#endif // !APP_H
