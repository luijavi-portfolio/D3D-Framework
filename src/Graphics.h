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
	
	void EndFrame();
public:
	static constexpr int kScreenWidth = 1280;
	static constexpr int kScreenHeight = 768;
private:
	static const UINT kFrameCount = 2;
	ComPtr<ID3D12CommandQueue>				command_queue_;
	ComPtr<ID3D12Device>					device_;
	ComPtr<ID3D12Resource>					render_targets_[kFrameCount];
	ComPtr<ID3D12CommandAllocator>			command_allocator_;
	ComPtr<IDXGISwapChain1>					swap_chain_;
	ComPtr<IDXGIFactory2>					factory_;
	ComPtr<ID3D12DescriptorHeap>			rtv_heap_;
	ComPtr<ID3D12GraphicsCommandList>		command_list_;
	UINT rtv_descriptor_size_;
};

#endif // !GRAPHICS_H
