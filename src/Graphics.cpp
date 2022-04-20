#include "Graphics.h"
#include "Window.h"
#include <d3dcompiler.h>
#include "DirectX12/d3dx12.h"

#pragma comment(lib, "dxgi.lib")

Graphics::Graphics(HandleKey& handle_key)
	:
	handle_(handle_key.handle_)
{ 
	aspect_ratio_ = static_cast<float>(kScreenWidth) / static_cast<float>(kScreenHeight);
}

void Graphics::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void Graphics::OnUpdate()
{
}

void Graphics::OnRender()
{
	// Record all the commands we need to render the scene into the command list
	PopulateCommandList();

	// Execute the command list
	ID3D12CommandList* command_lists[] = { command_list_.Get() };
	command_queue_->ExecuteCommandLists(_countof(command_lists), command_lists);

	// Present the frame
	ThrowIfFailed(swap_chain_->Present(1, 0));

	WaitForPreviousFrame();
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
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> hardware_adapter;

	// Scan DXGI adapters looking for one that supports Direct3D 12
	// Source: https://walbourn.github.io/anatomy-of-direct3d-12-create-device/
	// Also source: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/DXSample.cpp
	for (UINT adapter_index = 0;
		 DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &hardware_adapter);
		 ++adapter_index)
	{
		DXGI_ADAPTER_DESC1 desc;
		HRESULT hr = hardware_adapter->GetDesc1(&desc);

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

	ThrowIfFailed(device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_)));

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
	ThrowIfFailed(factory->CreateSwapChain(command_queue_.Get(), &swap_chain_desc, &swap_chain));

	ThrowIfFailed(swap_chain.As(&swap_chain_));

	frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

	// Create descriptor heaps
	{
		// Describe and create a render target view (RTV) descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.NumDescriptors = kFrameCount_;
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device_->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap_)));

		descriptor_size_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create the frame resources
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(descriptor_heap_->GetCPUDescriptorHandleForHeapStart());

		// Create an RTV for each frame
		for (UINT n = 0; n < kFrameCount_; ++n)
		{
			ThrowIfFailed(swap_chain_->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n])));
			
			device_->CreateRenderTargetView(render_targets_[n].Get(), nullptr, rtv_handle);
			rtv_handle.ptr = SIZE_T(INT64(rtv_handle.ptr) + INT64(1) * INT64(descriptor_size_));
		}
	}

	ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_)));
}

void Graphics::LoadAssets()
{
	// Create an empty root signature
	{
		CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;

		root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature_)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertex_shader;
		ComPtr<ID3DBlob> pixel_shader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools
		UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compile_flags = 0;
#endif
		
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertex_shader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixel_shader, nullptr));

		// Define the vertex input layout
		D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		// Describe and create the graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
		pso_desc.pRootSignature = root_signature_.Get();
		pso_desc.VS = { reinterpret_cast<UINT8*>(vertex_shader->GetBufferPointer()), vertex_shader->GetBufferSize() };
		pso_desc.PS = { reinterpret_cast<UINT8*>(pixel_shader->GetBufferPointer()),pixel_shader->GetBufferSize() };
		pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pso_desc.DepthStencilState.DepthEnable = FALSE;
		pso_desc.DepthStencilState.StencilEnable = FALSE;
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// TODO: This may be specific to the HelloTriangle example
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pso_desc.SampleDesc.Count = 1;
		ThrowIfFailed(device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_)));
	}

	// Create the command list
	ThrowIfFailed(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_.Get(), pipeline_state_.Get(), IID_PPV_ARGS(&command_list_)));

	// Command states are created in the recording state, but there's nothing
	// to record yet. The main loop expects it to be closed, so close it now
	ThrowIfFailed(command_list_->Close());

	// Create the vertex buffer
	{
		Vertex triangle_vertices[]
		{
			{ {0.0f, 0.25f * aspect_ratio_, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ {0.25f, -0.25f * aspect_ratio_, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ {-0.25f, -0.25f * aspect_ratio_, 0.0f}, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT kVertexBufferSize = sizeof(triangle_vertices);

		// Note: using upload heaps to transfer static data like vert buffers is no
		// recommended. Every time the GPU needs it, the upload will be marshalled over
		// Please read up on Default Heap usage. An upload heap is used here
		// for code simplicity and because there are very few verts to actually transfer.
		// TODO: Read up on Default Heap usage
		CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(kVertexBufferSize);
		ThrowIfFailed(device_->CreateCommittedResource(
			&heap_props,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertex_buffer_)));

		// Copy the triangle data to the vertex buffer.
		UINT8* vertex_data_begin;
		CD3DX12_RANGE read_range(0, 0);	// We don't intend to read from this resource on the CPU
		ThrowIfFailed(vertex_buffer_->Map(0, &read_range, reinterpret_cast<void**>(vertex_data_begin)));
		memcpy(vertex_data_begin, triangle_vertices, sizeof(triangle_vertices));
		vertex_buffer_->Unmap(0, nullptr);

		// Initialize the vertex buffer view
		vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
		vertex_buffer_view_.StrideInBytes = sizeof(Vertex);
		vertex_buffer_view_.SizeInBytes = kVertexBufferSize;
	}

	// Create synchronization objects and wait until assets have been uploaded to the GPU
	{
		ThrowIfFailed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

		// Create an event handle to use for frame synchronization
		fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!fence_event_)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command
		// list in our main loop, but for now, we just want to wait for setup to
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

void Graphics::PopulateCommandList()
{

}

void Graphics::WaitForPreviousFrame()
{
}
