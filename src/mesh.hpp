#pragma once

#include <glm/glm.hpp>

#include <string_view>
#include <vector>

namespace d3d12_mesh_shaders {
    class mesh final {
    public:
        struct vertex final {
            glm::vec3 position;
            glm::vec2 tex_coord;
            glm::vec3 normal;

            vertex() noexcept = default;
            vertex(const glm::vec3& position, const glm::vec2& tex_coord, const glm::vec3& normal) noexcept
                : position(position), tex_coord(tex_coord), normal(normal) {}
        };

        struct meshlet final {
            uint32_t data_offset;
            uint32_t vertex_count;
            uint32_t triangle_count;

            meshlet() noexcept = default;
            meshlet(uint32_t data_offset, uint32_t vertex_count, uint32_t triangle_count) noexcept
                : data_offset(data_offset), vertex_count(vertex_count), triangle_count(triangle_count) {}
        };
    private:
        std::vector<vertex> _vertices;
        std::vector<meshlet> _meshlets;
        std::vector<uint32_t> _meshlet_data;

    public:
        mesh(const std::string_view& path) noexcept;

        [[nodiscard]] inline const std::vector<vertex>& get_vertices() const noexcept {
            return _vertices;
        }

        [[nodiscard]] inline const std::vector<meshlet>& get_meshlets() const noexcept {
            return _meshlets;
        }

        [[nodiscard]] inline const std::vector<uint32_t>& get_meshlet_data() const noexcept {
            return _meshlet_data;
        }
    };
}