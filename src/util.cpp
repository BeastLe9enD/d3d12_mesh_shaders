#include "util.hpp"

#include <cstdio>
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

    void create_device_local_buffer(ID3D12Device8* device, ID3D12CommandQueue* command_queue, D3D12MA::Allocator* allocator, const void* data, size_t size,
                                    D3D12_RESOURCE_FLAGS resource_flags, D3D12_RESOURCE_STATES resource_states, ID3D12Resource2*& resource, D3D12MA::Allocation*& allocation) noexcept {
        D3D12_RESOURCE_DESC resource_desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Width = size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {
                .Count = 1
            },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        };

        D3D12MA::ALLOCATION_DESC allocation_desc {
            .HeapType = D3D12_HEAP_TYPE_DEFAULT
        };

        D3D12MA::ALLOCATION_DESC staging_allocation_desc {
            .HeapType = D3D12_HEAP_TYPE_UPLOAD
        };

        ID3D12Resource2* staging_resource;
        D3D12MA::Allocation* staging_allocation;

        panic_if_failed(allocator->CreateResource(&staging_allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &staging_allocation, IID_PPV_ARGS(&staging_resource)),
                        "D3D12MA::Allocator -> CreateResource");

        resource_desc.Flags = resource_flags;
        panic_if_failed(allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &allocation, IID_PPV_ARGS(&resource)),
                        "D3D12MA::Allocator -> CreateResource");

        D3D12_RANGE range = {
            .End = size
        };

        void* mapped_data;
        panic_if_failed(staging_resource->Map(0, &range, &mapped_data), "ID3D12Resource2 -> Map");
        memcpy(mapped_data, data, size);
        staging_resource->Unmap(0, &range);

        auto* command_allocator = create_command_allocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto* command_list = create_command_list(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

        HANDLE event;
        auto* fence = create_fence(device, 0, event);

        panic_if_failed(command_list->Reset(command_allocator, nullptr), "ID3D12GraphicsCommandList6 -> Reset");

        command_list->CopyBufferRegion(resource, 0, staging_resource, 0, size);

        if(resource_states != D3D12_RESOURCE_STATE_GENERIC_READ) {
            D3D12_RESOURCE_BARRIER resource_barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Transition = D3D12_RESOURCE_TRANSITION_BARRIER {
                    .pResource = resource,
                    .StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
                    .StateAfter = resource_states
                }
            };

            command_list->ResourceBarrier(1, &resource_barrier);
        }

        panic_if_failed(command_list->Close(), "ID3D12GraphicsCommandList6 -> Close");

        command_queue->ExecuteCommandLists(1, (ID3D12CommandList**)&command_list);
        command_queue->Signal(fence, 1);

        if(fence->GetCompletedValue() < 1) {
            fence->SetEventOnCompletion(1, event);
            WaitForSingleObject(event, INFINITE);
        }

        CloseHandle(event);
        fence->Release();

        command_list->Release();
        command_allocator->Release();

        staging_resource->Release();
        staging_allocation->Release();
    }

    void create_uav_for_buffer(ID3D12Device8* device, ID3D12Resource2* resource, size_t num_elements, size_t stride, D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle) noexcept {
        D3D12_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
            .Buffer = {
                .NumElements = static_cast<uint32_t>(num_elements),
                .StructureByteStride = static_cast<uint32_t>(stride)
            }
        };

        device->CreateUnorderedAccessView(resource, nullptr, &unordered_access_view_desc, descriptor_handle);
    }

    void create_device_buffer_and_uav(ID3D12Device8* device, ID3D12CommandQueue* command_queue, D3D12MA::Allocator* allocator, const void* data, size_t num_elements, size_t stride,
                               D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle, ID3D12Resource2*& resource, D3D12MA::Allocation*& allocation) noexcept {
        create_device_local_buffer(device, command_queue, allocator, data, num_elements * stride, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                   resource, allocation);
        create_uav_for_buffer(device, resource, num_elements, stride, descriptor_handle);
    }

    std::vector<int8_t> read_binary_file(const std::string_view& path) noexcept {
        auto* file = fopen(path.data(), "rb");
        if(!file) {
            panic("fopen");
        }

        fseek(file, 0, SEEK_END);
        const auto length = ftell(file);
        if(!length) {
            panic("ftell");
        }
        fseek(file, 0, SEEK_SET);

        std::vector<int8_t> buffer(length);
        const auto length_read = fread(buffer.data(), 1, length, file);
        if(length != length_read) {
            panic("fread");
        }

        fclose(file);

        return buffer;
    }
}