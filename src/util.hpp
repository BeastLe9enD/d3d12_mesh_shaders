#pragma once

#include <d3d12.h>

#include <vector>
#include <string_view>

namespace d3d12_mesh_shaders {
    template<typename T, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
    struct alignas(void*) pipeline_state_stream_subobject {
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
        T value;

        pipeline_state_stream_subobject() noexcept : type(Type) {}
    };

    namespace util {
        void panic_if_failed(HRESULT result, const std::string_view& message) noexcept;
        void panic(const std::string_view& message) noexcept;

        ID3D12CommandQueue* create_command_queue(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept;
        ID3D12CommandAllocator* create_command_allocator(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept;
        ID3D12GraphicsCommandList6* create_command_list(ID3D12Device8* device, D3D12_COMMAND_LIST_TYPE type) noexcept;
        ID3D12Fence1* create_fence(ID3D12Device8* device, uint64_t initial_value, HANDLE& event, D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE) noexcept;
        ID3D12DescriptorHeap* create_descriptor_heap(ID3D12Device8* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors) noexcept;

        std::vector<int8_t> read_binary_file(const std::string_view& path) noexcept;
    }
}