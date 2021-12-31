#include "util.hpp"
#include <iostream>
#include <system_error>

namespace d3d12_mesh_shaders::util {
    void panic_if_failed(HRESULT result, const std::string_view& message) noexcept {
        if(FAILED(result)) {
            std::cerr << message << " failed: " << std::system_category().message(result) << std::endl;
            std::exit(1);
        }
    }

    void panic(const std::string_view& message) noexcept {
        std::cerr << message << std::endl;
        std::exit(1);
    }

    ID3D12CommandQueue* create_command_queue(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept {
        D3D12_COMMAND_QUEUE_DESC command_queue_desc = {
            .Type = type,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
        };

        ID3D12CommandQueue* command_queue;
        panic_if_failed(device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue)), "ID3D12Device8 -> CreateCommandQueue");
        return command_queue;
    }

    ID3D12CommandAllocator* create_command_allocator(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept {
        ID3D12CommandAllocator* command_allocator;
        panic_if_failed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator)), "ID3D12Device8 -> CreateCommandAllocator");
        return command_allocator;
    }

    ID3D12GraphicsCommandList6* create_command_list(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept {
        ID3D12GraphicsCommandList6* command_list;
        panic_if_failed(device->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&command_list)), "ID3D12Device8 -> CreateCommandList1");
        return command_list;
    }

    ID3D12Fence1* create_fence(ID3D12Device8* device, uint64_t initial_value, HANDLE& event, D3D12_FENCE_FLAGS flags) noexcept {
        ID3D12Fence1* fence;
        panic_if_failed(device->CreateFence(initial_value, flags, IID_PPV_ARGS(&fence)), "ID3D12Device8 -> CreateFence");

        event = CreateEvent(nullptr, false, false, nullptr);
        if(!event) {
            panic("CreateEvent");
        }

        return fence;
    }

    ID3D12DescriptorHeap* create_descriptor_heap(ID3D12Device8* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors) noexcept {
        D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
            .Type = type,
            .NumDescriptors = num_descriptors
        };

        ID3D12DescriptorHeap* descriptor_heap;
        panic_if_failed(device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heap)), "ID3D12Device8 -> CreateDescriptorHeap");
        return descriptor_heap;
    }
}