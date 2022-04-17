#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "LeanWin32.h"
#include <d3d12.h>

class Graphics
{
public:
	Graphics(class HandleKey&);
	~Graphics();
	void EndFrame();
	void BeginFrame();
	void PutPixel(int x, int y, int r, int g, int b);
public:
	static constexpr int kScreenWidth = 1280;
	static constexpr int kScreenHeight = 768;
};

#endif // !GRAPHICS_H
