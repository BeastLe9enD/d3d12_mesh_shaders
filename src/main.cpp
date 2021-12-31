#include "engine.hpp"

int main(int num_arguments, char** arguments) {
    auto instance = new d3d12_mesh_shaders::engine(true, 1600, 900);
    instance->run_loop();
    delete instance;

    return 0;
}