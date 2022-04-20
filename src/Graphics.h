#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "LeanWin32.h"
#include "ExceptionHandler.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class Graphics
{
public:
	Graphics(class HandleKey& handle_key);
	Graphics(const Graphics& gfx);
	~Graphics();
	/*void EndFrame();
	void BeginFrame();
	void PutPixel(int x, int y, int r, int g, int b);*/

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
public:
	static constexpr int kScreenWidth = 1280;
	static constexpr int kScreenHeight = 768;
private:
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	// Exception handler - may want to move this
	void ThrowIfFailed(HRESULT hr)
	{
		throw (Window::Exception(__LINE__, __FILE__, hr));
	}
private:
	HWND handle_;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	// TODO: Figure out what this FrameCount refers to
	// Source:https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
	static const UINT kFrameCount_ = 2u;

	// Pipeline objects
	D3D12_VIEWPORT						viewport_;
	D3D12_RECT							scissor_rect_;
	ComPtr<IDXGISwapChain3>				swap_chain_;
	ComPtr<ID3D12Device>				device_;
	ComPtr<ID3D12Resource>				render_targets_[kFrameCount_];
	ComPtr<ID3D12CommandAllocator>		command_allocator_;
	ComPtr<ID3D12CommandQueue>			command_queue_;
	ComPtr<ID3D12RootSignature>			root_signature_;
	ComPtr<ID3D12DescriptorHeap>		descriptor_heap_;
	ComPtr<ID3D12PipelineState>			pipeline_state_;
	ComPtr<ID3D12GraphicsCommandList>	command_list_;
	
	UINT descriptor_size_;

	// App resources
	ComPtr<ID3D12Resource> vertex_buffer_;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;

	// Synchronization objects
	UINT frame_index_;
	HANDLE fence_event_;
	ComPtr<ID3D12Fence>	fence_;
	UINT64 fence_value_;

	// TODO: This will probably have to be moved to another file
	float aspect_ratio_;
};

#endif // !GRAPHICS_H
