#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "LeanWin32.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class Graphics
{
public:
	Graphics(class HandleKey& handle_key);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
	
	// TODO: Better handle these exceptions
	inline void ThrowIfFailed(HRESULT hr);
	

	void EndFrame();
public:
	static constexpr int kScreenWidth = 1280;
	static constexpr int kScreenHeight = 768;
private:
	static const UINT kFrameCount = 2;
	ComPtr<IDXGIFactory4> factory_;
	ComPtr<ID3D12Device> device_;
};

#endif // !GRAPHICS_H
