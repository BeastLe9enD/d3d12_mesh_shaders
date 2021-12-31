#pragma once

#include <SDL2/SDL.h>

#include <dxgi1_6.h>
#include <d3d12.h>

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
        ID3D12InfoQueue* _info_queue;
        ID3D12CommandQueue* _direct_queue;
        ID3D12CommandAllocator* _command_allocator;
        ID3D12GraphicsCommandList6* _command_list;

        ID3D12Fence1* _fence;
        HANDLE _fence_event;

        ID3D12DescriptorHeap* _rtv_descriptor_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _rtv_descriptor_heap_start_cpu;
        uint32_t _rtv_descriptor_increment_size;

        IDXGISwapChain4* _swap_chain;
        std::array<ID3D12Resource2*, _NUM_IMAGES> _swap_chain_images;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, _NUM_IMAGES> _swap_chain_rtvs;

        ID3D12PipelineState* _pipeline_state;

        bool _debug_mode;
        uint32_t _width, _height;

        void create_window() noexcept;
        void destroy_window() noexcept;

        void init_basic_d3d12() noexcept;
        void destroy_basic_d3d12() noexcept;

        void init_mesh_shader() noexcept;
        void destroy_mesh_shader() noexcept;

        void run_frame() noexcept;
    public:
        engine(bool debug_mode, uint32_t width, uint32_t height) noexcept;
        ~engine() noexcept;

        void run_loop() noexcept;
    };
}