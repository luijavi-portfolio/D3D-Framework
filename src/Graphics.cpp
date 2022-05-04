#include "Graphics.h"
#include "Window.h"
#include "DirectX12/d3dx12.h"
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
	:
	back_buffer_format_(DXGI_FORMAT_R8G8B8A8_UNORM),
	depth_stencil_format_(DXGI_FORMAT_D24_UNORM_S8_UINT),
	current_fence_(0)
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

	CreateCommandObjects();
	
	CreateSwapChain(handle_key.handle_);

	CreateRtvAndDsvDescriptorHeaps();

	// Create Render Target View
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_heap_handle(rtv_heap_->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < kFrameCount; ++i)
	{
		// Get the ith buffer in the swap chain
		ThrowIfFailed(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&swap_chain_buffer_[i])));

		// Create an RTV to it
		device_->CreateRenderTargetView(swap_chain_buffer_[i].Get(), nullptr, rtv_heap_handle);

		// Next entry in the heap
		rtv_heap_handle.Offset(1, rtv_descriptor_size_);
	}

	// Create the depth/stencil buffer and view
	D3D12_RESOURCE_DESC depth_stencil_desc{};
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = kScreenWidth;
	depth_stencil_desc.Height = kScreenHeight;
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = depth_stencil_format_;
	depth_stencil_desc.SampleDesc.Count = msaa_state_ ? 4 : 1;
	depth_stencil_desc.SampleDesc.Quality = msaa_state_ ? (msaa_quality_ - 1) : 0;
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE opt_clear{};
	opt_clear.Format = depth_stencil_format_;
	opt_clear.DepthStencil.Depth = 1.0f;
	opt_clear.DepthStencil.Stencil = 0;

	auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device_->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&depth_stencil_desc,
		D3D12_RESOURCE_STATE_COMMON,
		&opt_clear,
		IID_PPV_ARGS(depth_stencil_buffer_.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource
	device_->CreateDepthStencilView(depth_stencil_buffer_.Get(), nullptr, DepthStencilView());
	// Transition the resource from its initial state to be used as a depth buffer
	auto state_after = CD3DX12_RESOURCE_BARRIER::Transition(
		depth_stencil_buffer_.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);
	command_list_->ResourceBarrier(1, &state_after);

	// Set the viewport
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(kScreenWidth);
	viewport.Height = static_cast<FLOAT>(kScreenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	command_list_->RSSetViewports(1, &viewport);

	// Set the scissor rectangles
	scissor_rect_ = { 0, 0, kScreenWidth / 2, kScreenHeight / 2 };
	command_list_->RSSetScissorRects(1, &scissor_rect_);
}

Graphics::~Graphics()
{
	if (device_)
	{
		FlushCommandQueue();
	}
}

inline void Graphics::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, hr);
	}
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

void Graphics::CreateSwapChain(HWND& handle)
{
	// Desribe and create the swap chain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
	
	// Buffer description
	swap_chain_desc.BufferDesc.Height = kScreenHeight;
	swap_chain_desc.BufferDesc.Width = kScreenWidth;
	swap_chain_desc.BufferDesc.RefreshRate = refresh_rate_;
	swap_chain_desc.BufferDesc.Format = back_buffer_format_;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	
	// Sample description
	swap_chain_desc.SampleDesc.Count = msaa_state_ ? 4 : 1;
	swap_chain_desc.SampleDesc.Quality = msaa_state_ ? (msaa_quality_ - 1) : 0;
	
	// Remainder of swap chain desc
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = kFrameCount;
	swap_chain_desc.OutputWindow = handle;
	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(factory_->CreateSwapChain(
		command_queue_.Get(),
		&swap_chain_desc,
		swap_chain_.GetAddressOf()
	));
}

void Graphics::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
	rtv_heap_desc.NumDescriptors = kFrameCount;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 0;

	ThrowIfFailed(device_->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(rtv_heap_.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc{};
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.Flags= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsv_heap_desc.NodeMask= 0;

	ThrowIfFailed(device_->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(dsv_heap_.GetAddressOf())));

}

// Source: https://github.com/d3dcoder/d3d12book/blob/master/Common/d3dApp.cpp
void Graphics::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point
	current_fence_++;

	// Add an instruction to the command queue to set a new fence point. Because we
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal()
	ThrowIfFailed(command_queue_->Signal(fence_.Get(), current_fence_));

	// Wait until the GPU has completed commands up to this fence point
	if (fence_->GetCompletedValue() < current_fence_)
	{
		HANDLE event_handle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence
		ThrowIfFailed(fence_->SetEventOnCompletion(current_fence_, event_handle));

		// Wait until the GPU hits current fence event is fired
		WaitForSingleObject(event_handle, INFINITE);
		CloseHandle(event_handle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::CurrentBackBufferView() const
{
	// CD3DX12 constructor to offset to th eRTV of the current back buffer
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtv_heap_->GetCPUDescriptorHandleForHeapStart(),	// handle start
		current_back_buffer_,	// index to offset
		rtv_descriptor_size_);	// byte size of descriptor
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::DepthStencilView() const
{
	return dsv_heap_->GetCPUDescriptorHandleForHeapStart();
}
