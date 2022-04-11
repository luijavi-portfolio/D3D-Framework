#ifndef GRAPHICS_H
#define GRAPHICS_H

class Graphics
{
public:
	Graphics(class HandleKey&);
	virtual void EndFrame() = 0;
	virtual void BeginFrame() = 0;
	virtual void PutPixel(int x, int y, int r, int g, int b) = 0;
	virtual ~Graphics() = 0;
private:
};

#endif // !GRAPHICS_H
