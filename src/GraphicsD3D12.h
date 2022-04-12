#ifndef GRAPHICS_D3D12_H
#define GRAPHICS_D3D12_H

#include "Graphics.h"

class Graphics_D3D12 : public Graphics
{
public:
	Graphics_D3D12(class HandleKey& handle_key);
	virtual void EndFrame() override;
	virtual void BeginFrame() override;
	virtual void PutPixel(int x, int y, int r, int g, int b) override;
private:

};
#endif // !GRAPHICS_D3D12_H
