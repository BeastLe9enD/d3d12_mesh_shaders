cmake_minimum_required(VERSION 3.20)

project(d3d12_mesh_shaders)

set(CMAKE_CXX_STANDARD 23)

set(MY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(MY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MY_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE MY_SOURCE_FILES ${MY_SOURCE_DIR}/*.c**)
file(GLOB_RECURSE MY_HEADER_FILES ${MY_SOURCE_DIR}/*.h**)

include_directories(${MY_INCLUDE_DIR})

add_executable(d3d12_mesh_shaders
        ${MY_SOURCE_FILES} ${MY_HEADER_FILES}

        # D3D12MemAlloc
        ${MY_INCLUDE_DIR}/D3D12MemAlloc/D3D12MemAlloc.cpp

        # fastobj
        ${MY_INCLUDE_DIR}/fast_obj/fast_obj.c

        # meshoptimizer
        ${MY_INCLUDE_DIR}/meshoptimizer/allocator.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/clusterizer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/indexcodec.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/indexgenerator.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/overdrawanalyzer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/overdrawoptimizer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/simplifier.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/spatialorder.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/stripifier.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vcacheanalyzer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vcacheoptimizer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vertexcodec.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vertexfilter.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vfetchanalyzer.cpp
        ${MY_INCLUDE_DIR}/meshoptimizer/vfetchoptimizer.cpp)
target_link_libraries(d3d12_mesh_shaders
        d3d12.lib dxgi.lib
        ${MY_LIBRARY_DIR}/SDL2.lib
        ${MY_LIBRARY_DIR}/SDL2main.lib)