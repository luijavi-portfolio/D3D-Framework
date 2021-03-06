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
	
private:
	void CreateCommandObjects();
	void CreateSwapChain(HWND& handle);
	void CreateRtvAndDsvDescriptorHeaps();
	void FlushCommandQueue();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
public:
	static constexpr int kScreenWidth = 1280;
	static constexpr int kScreenHeight = 768;
private:
	static const UINT kFrameCount = 2;
	int current_back_buffer_ = 0;

	// IDXGI objects
	ComPtr<IDXGIFactory4>				factory_;
	ComPtr<ID3D12Device>				device_;
	ComPtr<ID3D12Fence>					fence_;
	ComPtr<ID3D12CommandQueue>			command_queue_;
	ComPtr<ID3D12CommandAllocator>		command_list_allocator_;
	ComPtr<ID3D12GraphicsCommandList>	command_list_;
	ComPtr<IDXGISwapChain>				swap_chain_;
	ComPtr<ID3D12DescriptorHeap>		rtv_heap_;	// Render Target View descriptor heap
	ComPtr<ID3D12DescriptorHeap>		dsv_heap_;	// depth/stencil view descriptor heap
	ComPtr<ID3D12Resource>				swap_chain_buffer_[kFrameCount];
	ComPtr<ID3D12Resource>				depth_stencil_buffer_;


	// Descriptor sizes
	UINT rtv_descriptor_size_;		// Render Target View
	UINT dsv_descriptor_size_;		// Depth Stencil View
	UINT cbv_srv_descriptor_size_;	// Constant Buffer View and Shader Resource View
	UINT msaa_quality_;
	UINT64 current_fence_;
	bool msaa_state_ = false;

	DXGI_RATIONAL refresh_rate_ = { 60u, 1u };

	DXGI_FORMAT back_buffer_format_;
	DXGI_FORMAT depth_stencil_format_;
	D3D12_RECT scissor_rect_;
};

#endif // !GRAPHICS_H
