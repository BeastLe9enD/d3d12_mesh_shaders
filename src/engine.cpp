#include "engine.hpp"
#include "util.hpp"

#include <DirectXColors.h>

#include <SDL2/SDL_syswm.h>

#include <string>
#include <iostream>

namespace d3d12_mesh_shaders {
    void engine::create_window() noexcept {
        if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            util::panic("SDL_Init");
        }

        _window = SDL_CreateWindow("d3d12_mesh_shaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(_width), static_cast<int>(_height), SDL_WINDOW_SHOWN);
        if(!_window) {
            util::panic("SDL_CreateWindow");
        }

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        if(!SDL_GetWindowWMInfo(_window, &wmInfo)) {
            util::panic("SDL_GetWindowWMInfo");
        }

        _hwnd = wmInfo.info.win.window;
    }

    void engine::destroy_window() noexcept {
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void engine::init_basic_d3d12() noexcept {
        util::panic_if_failed(CreateDXGIFactory2(_debug_mode ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&_factory)), "CreateDXGIFactory2");
        if(_debug_mode) {
            util::panic_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&_debug_interface)), "D3D12GetDebugInterface");
            _debug_interface->EnableDebugLayer();
        }

        util::panic_if_failed(_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter)), "IDXGIAdapter4 -> EnumAdapterByGpuPreference");
        util::panic_if_failed(D3D12CreateDevice(_adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device)), "D3D12CreateDevice");

        if(_debug_mode) {
            util::panic_if_failed(_device->QueryInterface(IID_PPV_ARGS(&_info_queue)), "ID3D12Device8 -> QueryInterface");
            _info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            _info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        }

        _direct_queue = util::create_command_queue(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        _command_allocator = util::create_command_allocator(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        _command_list = util::create_command_list(_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        _fence = util::create_fence(_device, 0, _fence_event);

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
            .Width = _width,
            .Height = _height,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .SampleDesc = DXGI_SAMPLE_DESC {
                    .Count = 1
            },
            .BufferUsage = DXGI_USAGE_BACK_BUFFER,
            .BufferCount = _NUM_IMAGES,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        _rtv_descriptor_heap = util::create_descriptor_heap(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, _NUM_IMAGES);
        _rtv_descriptor_heap_start_cpu = _rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        _rtv_descriptor_increment_size = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        IDXGISwapChain1* temp_swap_chain;
        util::panic_if_failed(_factory->CreateSwapChainForHwnd(_direct_queue, _hwnd, &swap_chain_desc, nullptr, nullptr, &temp_swap_chain), "IDXGIFactory7 -> CreateSwapChainForHwnd");
        util::panic_if_failed(temp_swap_chain->QueryInterface(IID_PPV_ARGS(&_swap_chain)), "IDXGISwapChain1 -> QueryInterface");
        temp_swap_chain->Release();

        for(auto i = 0; i < _NUM_IMAGES; i++) {
            ID3D12Resource2* resource;

            util::panic_if_failed(_swap_chain->GetBuffer(i, IID_PPV_ARGS(&resource)), "IDXGISwapChain4 -> GetBuffer");

            D3D12_RENDER_TARGET_VIEW_DESC render_target_view_desc = {
                    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
                    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                    .Texture2D = D3D12_TEX2D_RTV {}
            };

            D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = _rtv_descriptor_heap_start_cpu;
            descriptor_handle.ptr += i * _rtv_descriptor_increment_size;

            _device->CreateRenderTargetView(resource, &render_target_view_desc, descriptor_handle);
            _swap_chain_images[i] = resource;
            _swap_chain_rtvs[i] = descriptor_handle;
        }
    }

    void engine::destroy_basic_d3d12() noexcept {
        _swap_chain->Release();
        _rtv_descriptor_heap->Release();

        CloseHandle(_fence_event);
        _fence->Release();

        _command_list->Release();
        _command_allocator->Release();
        _direct_queue->Release();

        if(_debug_mode) {
            _info_queue->Release();
        }
        _device->Release();
        _adapter->Release();

        if(_debug_mode) {
            _debug_interface->Release();
        }
        _factory->Release();
    }

    void engine::init_mesh_shader() noexcept {
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_root_signature_desc = {
            .Version = D3D_ROOT_SIGNATURE_VERSION_1_1
        };

        ID3DBlob* blob, *error_blob;
        if(FAILED(D3D12SerializeVersionedRootSignature(&versioned_root_signature_desc, &blob, &error_blob))) {
            std::cerr << "D3D12SerializeVersionedRootSignature failed: " << reinterpret_cast<const char*>(error_blob->GetBufferPointer()) << std::endl;
            std::exit(1);
        }

        util::panic_if_failed(_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&_root_signature)), "ID3D12Device8 -> CreateRootSignature");

        const auto amplification_shader = util::read_binary_file("meshlet_as.dxil");
        const auto mesh_shader = util::read_binary_file("meshlet_ms.dxil");
        const auto pixel_shader = util::read_binary_file("meshlet_ps.dxil");

        struct {
            pipeline_state_stream_subobject<ID3D12RootSignature*, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE> root_signature;
            pipeline_state_stream_subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> amplification_shader;
            pipeline_state_stream_subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> mesh_shader;
            pipeline_state_stream_subobject<D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS> pixel_shader;
            pipeline_state_stream_subobject<D3D12_RT_FORMAT_ARRAY, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS> render_target_formats;
        } pipeline_state_stream;

        pipeline_state_stream.root_signature.value = _root_signature;
        pipeline_state_stream.amplification_shader.value = {
            .pShaderBytecode = amplification_shader.data(),
            .BytecodeLength = amplification_shader.size()
        };
        pipeline_state_stream.mesh_shader.value = {
            .pShaderBytecode = mesh_shader.data(),
            .BytecodeLength = mesh_shader.size()
        };
        pipeline_state_stream.pixel_shader.value = {
            .pShaderBytecode = pixel_shader.data(),
            .BytecodeLength = pixel_shader.size()
        };
        pipeline_state_stream.render_target_formats.value.NumRenderTargets = 1;
        pipeline_state_stream.render_target_formats.value.RTFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
        for (auto i = 1; i < 8; i++) {
            pipeline_state_stream.render_target_formats.value.RTFormats[i] = DXGI_FORMAT_UNKNOWN;
        }

        D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_stream_desc = {
            .SizeInBytes = sizeof(pipeline_state_stream),
            .pPipelineStateSubobjectStream = &pipeline_state_stream
        };

        util::panic_if_failed(_device->CreatePipelineState(&pipeline_state_stream_desc, IID_PPV_ARGS(&_pipeline_state)), "ID3D12Device8 -> CreatePipelineState");
    }

    void engine::destroy_mesh_shader() noexcept {
        _pipeline_state->Release();
        _root_signature->Release();
    }

    void engine::run_frame() noexcept {
        util::panic_if_failed(_fence->Signal(0), "ID3D12Fence1 -> Signal");

        util::panic_if_failed(_command_allocator->Reset(), "ID3D12CommandAllocator -> Reset");
        util::panic_if_failed(_command_list->Reset(_command_allocator, nullptr), "ID3D12GraphicsCommandList6 -> Reset");

        const auto index = _swap_chain->GetCurrentBackBufferIndex();

        D3D12_RESOURCE_BARRIER resource_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Transition = D3D12_RESOURCE_TRANSITION_BARRIER {
                .pResource = _swap_chain_images[index],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET
            }
        };

        _command_list->ResourceBarrier(1, &resource_barrier);

        run_frame_inner(index);

        resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        _command_list->ResourceBarrier(1, &resource_barrier);

        util::panic_if_failed(_command_list->Close(), "ID3D12GraphicsCommandList6 -> Close");

        util::panic_if_failed(_direct_queue->Signal(_fence, 0), "ID3D12CommandQueue -> Signal");
        _direct_queue->ExecuteCommandLists(1, (ID3D12CommandList**)&_command_list);
        util::panic_if_failed(_direct_queue->Signal(_fence, 1), "ID3D12CommandQueue -> Signal");

        if(_fence->GetCompletedValue() < 1) {
            _fence->SetEventOnCompletion(1, _fence_event);
            WaitForSingleObject(_fence_event, INFINITE);
        }

        util::panic_if_failed(_swap_chain->Present(0, 0), "IDXGISwapChain4 -> Present");
    }

    void engine::run_frame_inner(uint32_t index) noexcept {
        D3D12_RECT clear_rect = { .right = static_cast<LONG>(_width), .bottom = static_cast<LONG>(_height) };
        _command_list->ClearRenderTargetView(_swap_chain_rtvs[index], DirectX::Colors::CornflowerBlue, 1, &clear_rect);

        _command_list->OMSetRenderTargets(1, &_swap_chain_rtvs[index], true, nullptr);

        D3D12_VIEWPORT viewport = {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<float>(_width),
            .Height = static_cast<float>(_height),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        _command_list->RSSetViewports(1, &viewport);
        _command_list->RSSetScissorRects(1, &clear_rect);

        _command_list->SetGraphicsRootSignature(_root_signature);
        _command_list->SetPipelineState(_pipeline_state);
        
        _command_list->DispatchMesh(1, 1, 1);
    }

    engine::engine(bool debug_mode, uint32_t width, uint32_t height) noexcept {
        _debug_mode = debug_mode;
        _width = width;
        _height = height;

        create_window();
        init_basic_d3d12();
        init_mesh_shader();
    }

    engine::~engine() noexcept {
        if(_fence->GetCompletedValue() < 1) {
            _fence->SetEventOnCompletion(1, _fence_event);
            WaitForSingleObject(_fence_event, INFINITE);
        }

        destroy_mesh_shader();
        destroy_basic_d3d12();

        destroy_window();
    }

    void engine::run_loop() noexcept {
        bool running = true;

        SDL_Event ev;

        while(running) {
            while(SDL_PollEvent(&ev)) {
                if(ev.type == SDL_QUIT) {
                    running = false;
                }
            }

            run_frame();
        }
    }
}