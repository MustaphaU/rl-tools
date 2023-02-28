#include <layer_in_c/operations/cpu.h>
#include <layer_in_c/containers/persist_code.h>
#include <layer_in_c/nn/layers/dense/operations_cpu.h>
#include <layer_in_c/nn/layers/dense/persist_code.h>

namespace lic = layer_in_c;


#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include "../../../data/test_layer_in_c_container_persist_matrix.h"


TEST(LAYER_IN_C_CONTAINER_PERSIST_CODE_LOAD, TEST){
    using DEVICE = lic::devices::DefaultCPU;
    using DTYPE = float;
    DEVICE device;
    auto rng = lic::random::default_engine(DEVICE::SPEC::RANDOM());
    lic::Matrix<lic::matrix::Specification<DTYPE, typename DEVICE::index_t, 3, 3>> orig;
    lic::malloc(device, orig);
    lic::randn(device, orig, rng);
    std::cout << "orig: " << std::endl;
    lic::print(device, orig);
    std::cout << "loaded: " << std::endl;
    lic::print(device, matrix_1::matrix);

    auto abs_diff = lic::abs_diff(device, orig, matrix_1::matrix);
    ASSERT_FLOAT_EQ(0, abs_diff);
}

#include "../../../data/test_layer_in_c_nn_layers_dense_persist_code.h"

TEST(LAYER_IN_C_CONTAINER_PERSIST_CODE_LOAD, TEST_DENSE_LAYER){
    using DEVICE = lic::devices::DefaultCPU;
    using DTYPE = float;
    DEVICE device;
    auto rng = lic::random::default_engine(DEVICE::SPEC::RANDOM());
    lic::nn::layers::dense::Layer<lic::nn::layers::dense::Specification<DTYPE, typename DEVICE::index_t, 3, 3, lic::nn::activation_functions::ActivationFunction::RELU>> layer;
    lic::malloc(device, layer);
    lic::init_kaiming(device, layer, rng);
    lic::increment(layer.weights, 2, 1, 10);
    auto abs_diff = lic::abs_diff(device, layer, layer_1::layer);
    ASSERT_TRUE(true);
}
