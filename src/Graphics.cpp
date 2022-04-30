#include "Graphics.h"
#include "Window.h"
#include "DirectX12/d3dx12.h"
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
	:
	back_buffer_format_(DXGI_FORMAT_R8G8B8A8_UNORM)
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

	

	// Create the fence and descriptor sizes, since descriptor sizes vary across GPUs
	ThrowIfFailed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	rtv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbv_srv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support 
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
	ms_quality_levels.Format = back_buffer_format_;
	ms_quality_levels.SampleCount = 4;
	ms_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	ms_quality_levels.NumQualityLevels = 0;
	ThrowIfFailed(device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms_quality_levels, sizeof(ms_quality_levels)));
	msaa_quality_ = ms_quality_levels.NumQualityLevels;
	assert(msaa_quality_ > 0 && "Unexpected MSAA quality level.");

	// Create command queue and command list
	
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

void Graphics::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(device_->CreateCommandQueue(
		&queue_desc, 
		IID_PPV_ARGS(&command_queue_))
	);

	ThrowIfFailed(device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(command_list_allocator_.GetAddressOf()))
	);


	// TODO: Provide valid pipeline state object
	ThrowIfFailed(device_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		command_list_allocator_.Get(),	// Associated command allocator
		nullptr,						// Initial PipelineStateObject
		IID_PPV_ARGS(command_list_.GetAddressOf()))
	);

	// Start off in a closed state. This is because the first time we
	// refer to the command list, we will Reset it, and it needs to be
	// closed before calling Reset.
	command_list_->Close();
}
