#include "Graphics.h"
#include "Window.h"
#include "DirectX12/d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
{
#if defined(DEBUG) || (_DEBUG)
	{
		// Enable D3D12 debug layer
		ComPtr<ID3D12Debug> debug_controller;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
		debug_controller->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory_)));
	
	// Try to create hardware device
	HRESULT hardware_result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));

	// Fallback to WARP device
	if (FAILED(hardware_result))
	{
		ComPtr<IDXGIAdapter> warp_adapter;
		ThrowIfFailed(factory_->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

		ThrowIfFailed(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)));
	}
}

Graphics::~Graphics()
{
	
}

inline void Graphics::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, hr);
	}
}

void Graphics::EndFrame()
{
	
	
}
