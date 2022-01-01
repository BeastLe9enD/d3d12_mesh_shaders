#pragma once

#include "camera.hpp"

#include <SDL2/SDL.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <D3D12MemAlloc/D3D12MemAlloc.h>

#include <array>

namespace d3d12_mesh_shaders {
    class engine final {
    private:
        static const uint32_t _NUM_IMAGES = 2;

        SDL_Window* _window;
        HWND _hwnd;

        IDXGIFactory7* _factory;
        ID3D12Debug3* _debug_interface;
        IDXGIAdapter4* _adapter;
        ID3D12Device8* _device;
        D3D12MA::Allocator* _allocator;
        ID3D12InfoQueue* _info_queue;
        ID3D12CommandQueue* _direct_queue;
        ID3D12CommandAllocator* _command_allocator;
        ID3D12GraphicsCommandList6* _command_list;

        ID3D12Fence1* _fence;
        HANDLE _fence_event;

        ID3D12DescriptorHeap* _rtv_descriptor_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _rtv_descriptor_heap_start_cpu;
        uint32_t _rtv_descriptor_increment_size;

        ID3D12DescriptorHeap* _dsv_descriptor_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _dsv_descriptor_heap_start_cpu;
        uint32_t _dsv_descriptor_increment_size;
        D3D12_CPU_DESCRIPTOR_HANDLE _dsv;

        ID3D12DescriptorHeap* _cbv_srv_uav_descriptor_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _cbv_srv_uav_descriptor_heap_start_cpu;
        uint32_t _cbv_srv_uav_descriptor_increment_size;

        IDXGISwapChain4* _swap_chain;
        std::array<ID3D12Resource2*, _NUM_IMAGES> _swap_chain_images;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, _NUM_IMAGES> _swap_chain_rtvs;

        ID3D12Resource2* _depth_texture;
        D3D12MA::Allocation* _depth_texture_allocation;

        ID3D12Resource2* _constant_buffer;
        D3D12MA::Allocation* _constant_buffer_allocation;
        D3D12_CPU_DESCRIPTOR_HANDLE _cbv;

        ID3D12RootSignature* _root_signature;
        ID3D12PipelineState* _pipeline_state;

        camera _camera;

        bool _debug_mode;
        uint32_t _width, _height;

        void create_window() noexcept;
        void destroy_window() noexcept;

        void init_basic_d3d12() noexcept;
        void destroy_basic_d3d12() noexcept;

        void init_depth_texture() noexcept;
        void destroy_depth_texture() noexcept;

        void init_constant_buffer() noexcept;
        void destroy_constant_buffer() noexcept;

        void init_mesh_shader() noexcept;
        void destroy_mesh_shader() noexcept;

        void run_frame() noexcept;
        void run_frame_inner(uint32_t index) noexcept;
    public:
        engine(bool debug_mode, uint32_t width, uint32_t height) noexcept;
        ~engine() noexcept;

        void run_loop() noexcept;
    };
}