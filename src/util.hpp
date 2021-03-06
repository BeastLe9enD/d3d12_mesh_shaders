#pragma once

#include <d3d12.h>
#include <D3D12MemAlloc/D3D12MemAlloc.h>
#include <glm/glm.hpp>

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
        void create_device_local_buffer(ID3D12Device8* device, ID3D12CommandQueue* command_queue, D3D12MA::Allocator* allocator, const void* data, size_t size,
                                        D3D12_RESOURCE_FLAGS resource_flags, D3D12_RESOURCE_STATES resource_states, ID3D12Resource2*& resource, D3D12MA::Allocation*& allocation) noexcept;
        void create_uav_for_buffer(ID3D12Device8* device, ID3D12Resource2* resource, size_t num_elements, size_t stride, D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle) noexcept;
        void create_device_buffer_and_uav(ID3D12Device8* device, ID3D12CommandQueue* command_queue, D3D12MA::Allocator* allocator, const void* data, size_t num_elements, size_t stride,
                                   D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle, ID3D12Resource2*& resource, D3D12MA::Allocation*& allocation) noexcept;

        std::vector<int8_t> read_binary_file(const std::string_view& path) noexcept;

        inline glm::mat4 reverse_depth_projection_matrix_lh(float field_of_view, float aspect_ratio, float near_plane, float far_plane) noexcept {
            const auto tan_half_fov_y = glm::tan(glm::radians(field_of_view) / 2.0f);
            const auto far_minus_near = far_plane - near_plane;

            glm::mat4 result(0.0f);
            result[0][0] = 1.0f / (aspect_ratio * tan_half_fov_y);
            result[1][1] = -1.0f / (tan_half_fov_y);
            result[2][2] = far_plane / far_minus_near - 1.0f;
            result[2][3] = -1.0f;
            result[3][2] = (far_plane * near_plane) / far_minus_near;

            return result;
        }

        inline glm::vec3 direction_from_rotation(const glm::vec3& rotation) noexcept {
            const auto cos_y = glm::cos(rotation.y);

            glm::vec3 v;
            v.x = glm::sin(rotation.x) * cos_y;
            v.y = glm::sin(rotation.y);
            v.z = glm::cos(rotation.x) * cos_y;
            return v;
        }
    }
}