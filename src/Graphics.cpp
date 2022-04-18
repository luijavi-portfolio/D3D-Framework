#include "Graphics.h"
#include "Window.h"

#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
	:
	handle_(handle_key.handle_)
{ }

void Graphics::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

// TODO: This function could probably be broken down further
void Graphics::LoadPipeline()
{
#if defined(_DEBUG)
	// Enable the D312 debug layer
	{
		ComPtr<ID3D12Debug> debug_controller;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
		{
			debug_controller->EnableDebugLayer();
		}
	}
#endif

	// Create the device
	ComPtr<IDXGIFactory7> factory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	
	if (FAILED(hr))
	{
		throw (Window::Exception(__LINE__, __FILE__, hr));
	}

	ComPtr<IDXGIAdapter1> hardware_adapter;

	// Scan DXGI adapters looking for one that supports Direct3D 12
	// Source: https://walbourn.github.io/anatomy-of-direct3d-12-create-device/
	// Also source: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/DXSample.cpp
	for (UINT adapter_index = 0;
		 DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &hardware_adapter);
		 ++adapter_index)
	{
		DXGI_ADAPTER_DESC1 desc;
		hr = hardware_adapter->GetDesc1(&desc);

		if (FAILED(hr))
		{
			continue;
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basi Render Driver adapter
			continue;
		}

		// Check to see if adapter supports Direct3D 12,
		// but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	// Describe and create the command queue
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_));

	if (FAILED(hr))
	{
		throw Window::Exception(__LINE__, __FILE__, hr);
	}

	// Describe and create the swap chain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
	swap_chain_desc.BufferCount = kFrameCount_;
	swap_chain_desc.BufferDesc.Width = kScreenWidth;
	swap_chain_desc.BufferDesc.Height= kScreenHeight;
	// A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha.
	// Source: https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.OutputWindow = handle_; // TODO: May have to decouple this a little more
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swap_chain;
	// Swap chain needs the queue so that it can force a flush on it.
	hr = factory->CreateSwapChain(command_queue_.Get(), &swap_chain_desc, &swap_chain);
	if (FAILED(hr))
	{
		throw (Window::Exception(__LINE__, __FILE__, hr));
	}

	hr = swap_chain.As(&swap_chain_);

	frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

	// Create descriptor heaps
	{
		// Describe and create a render target view (RTV) descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.NumDescriptors = kFrameCount_;
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = device_->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap_));

		if (FAILED(hr))
		{
			throw (Window::Exception(__LINE__, __FILE__, hr));
		}

		descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create the frame resources
	{
		D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle(descriptor_heap_->GetCPUDescriptorHandleForHeapStart());

		// Create an RTV for each frame
		for (UINT n = 0; n < kFrameCount_; ++n)
		{
			hr = swap_chain_->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n]));
			
			if (FAILED(hr))
			{
				throw (Window::Exception(__LINE__, __FILE__, hr));
			}

			device_->CreateRenderTargetView(render_targets_[n].Get(), nullptr, descriptor_handle);
			descriptor_handle.ptr = SIZE_T(INT64(descriptor_handle.ptr) + INT64(1) * INT64(descriptor_size_));
		}
	}

	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_));
	if (FAILED(hr))
	{
		throw (Window::Exception(__LINE__, __FILE__, hr));
	}
}

void Graphics::LoadAssets()
{
	// TODO: Implement
}
