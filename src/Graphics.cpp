#include "Graphics.h"
#include "Window.h"
#include "DirectX12/d3dx12.h"


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
{

	// Disabled D3D12 debug layer, since it was impacting performance so much... will have
	// to figure out why later...
//#if defined(_DEBUG)
//	// Enable the debug layer
//	{
//		ComPtr<ID3D12Debug> debug_controller;
//		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
//		{
//			debug_controller->EnableDebugLayer();
//		}
//	}
//#endif

	// Create factory (for device)
	// Source: https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component#loadpipeline
	ComPtr<IDXGIFactory4> factory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, "Failed to create factory!");
	}

	// Scan DXGI adapters looking for one that supports Direct3D 12
	// Source: https://walbourn.github.io/anatomy-of-direct3d-12-create-device/
	ComPtr<IDXGIAdapter1> hardware_adapter;
	for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &hardware_adapter); ++adapter_index)
	{
		DXGI_ADAPTER_DESC1 desc{};
		hr = hardware_adapter->GetDesc1(&desc);

		if (FAILED(hr))
		{
			continue;
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet
		if (SUCCEEDED(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	// Create device
	hr = D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));

	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, "Failed to create device!");
	}

	// Check feature support
	static const D3D_FEATURE_LEVEL kFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D12_FEATURE_DATA_FEATURE_LEVELS feat_levels =
	{
		_countof(kFeatureLevels), kFeatureLevels, D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	hr = device_->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feat_levels, sizeof(feat_levels));

	if (SUCCEEDED(hr))
	{
		feature_level = feat_levels.MaxSupportedFeatureLevel;
	}

	// Before creating the swapchain, must describe and create the command queue
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_));
	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, "Failed to create command queue!");
	}
	
	// Now to describe the swap chain
	// Source: https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = 0;
	swap_chain_desc.Height = 0;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = kFrameCount;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	
	ComPtr<IDXGISwapChain1> swap_chain;
	// Source: https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforhwnd
	hr = factory->CreateSwapChainForHwnd(command_queue_.Get(), handle_key.handle_, &swap_chain_desc, nullptr, nullptr, &swap_chain);

	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, "Failed to create swap chain!");
	}

	hr = swap_chain.As(&swap_chain_);
	if (FAILED(hr))
	{
		// TODO: Come up with more descriptive error code?
		throw Window::Exception(__LINE__, __FILE__, hr);
	}

	// Create descriptor heaps
	{
		// Describe and create a render target view (RTV) descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
		rtv_heap_desc.NumDescriptors = kFrameCount;
		rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = device_->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap_));

		if (FAILED(hr))
		{
			throw Window::Exception(__LINE__, __FILE__, "Failed to create descriptor heap description!");
		}

		rtv_descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap_->GetCPUDescriptorHandleForHeapStart());

		// Create an RTV for each frame
		for (UINT n = 0; n < kFrameCount; ++n)
		{
			hr = swap_chain_->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n]));
			if (FAILED(hr))
			{
				throw Window::Exception(__LINE__, __FILE__, "Failed to create an rtv!");
			}
			device_->CreateRenderTargetView(render_targets_[n].Get(), nullptr, rtv_handle);
			rtv_handle.Offset(1, rtv_descriptor_size_);
		}
	}

	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));
	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, "Failed to create command allocator!");
	}
}

Graphics::~Graphics()
{
	if (command_queue_)
	{
		command_queue_->Release();
	}

	if (device_)
	{
		device_->Release();
	}

	if (command_allocator_)
	{
		command_allocator_->Release();
	}

	if (swap_chain_)
	{
		swap_chain_->Release();
	}

	if (factory_)
	{
		factory_->Release();
	}

	if (rtv_heap_)
	{
		rtv_heap_->Release();
	}
}

void Graphics::EndFrame()
{
	
	swap_chain_->Present(1u, 0);
}
