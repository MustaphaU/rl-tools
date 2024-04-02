#include "../../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_NN_LAYERS_GRU_OPERATIONS_GENERIC_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_NN_LAYERS_GRU_OPERATIONS_GENERIC_H

#include "layer.h"
#include "helper_operations_generic.h"
#include <rl_tools/nn/parameters/operations_generic.h>

RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools{
    template <typename DEVICE, typename SPEC>
    void malloc(DEVICE& device, nn::layers::gru::Layer<SPEC>& layer){
        malloc(device, layer.weights_input);
        using VIEW_SPEC = tensor::ViewSpec<0, SPEC::HIDDEN_DIM>;
        layer.W_ir = view_range(device, layer.weights_input.parameters, 0*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.W_iz = view_range(device, layer.weights_input.parameters, 1*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.W_in = view_range(device, layer.weights_input.parameters, 2*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        malloc(device, layer.biases_input);
        layer.b_ir = view_range(device, layer.biases_input.parameters, 0*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.b_iz = view_range(device, layer.biases_input.parameters, 1*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.b_in = view_range(device, layer.biases_input.parameters, 2*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        malloc(device, layer.weights_hidden);
        using VIEW_SPEC_DOUBLE = tensor::ViewSpec<0, 2*SPEC::HIDDEN_DIM>;
        layer.W_hrz = view_range(device, layer.weights_hidden.parameters, 0*SPEC::HIDDEN_DIM, VIEW_SPEC_DOUBLE{});
        layer.W_hr = view_range(device, layer.weights_hidden.parameters, 0*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.W_hz = view_range(device, layer.weights_hidden.parameters, 1*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.W_hn = view_range(device, layer.weights_hidden.parameters, 2*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        malloc(device, layer.biases_hidden);
        layer.b_hr = view_range(device, layer.biases_hidden.parameters, 0*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.b_hz = view_range(device, layer.biases_hidden.parameters, 1*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        layer.b_hn = view_range(device, layer.biases_hidden.parameters, 2*SPEC::HIDDEN_DIM, VIEW_SPEC{});

        malloc(device, layer.initial_hidden_state);
        set_all(device, layer.initial_hidden_state.parameters, 0);
    }
    template <typename DEVICE, typename SPEC>
    void malloc(DEVICE& device, nn::layers::gru::LayerBackward<SPEC>& layer){
        malloc(device, static_cast<nn::layers::gru::Layer<SPEC>&>(layer));
        malloc(device, layer.n_pre_pre_activation);
        malloc(device, layer.post_activation);
        malloc(device, layer.output);
    }
    template <typename DEVICE, typename SPEC>
    void malloc(DEVICE& device, nn::layers::gru::BuffersBackward<SPEC>& buffers){
        malloc(device, buffers.dr_dr_pa);
        malloc(device, buffers.dh_dr);
        malloc(device, buffers.dh_dr_pa);
        malloc(device, buffers.dh_dz);
        malloc(device, buffers.dz_dz_pa);
        malloc(device, buffers.dh_dn);
        malloc(device, buffers.dn_dn_pa);
        malloc(device, buffers.dn_dn_pa_pa);
        malloc(device, buffers.dh_dz_pa);
        malloc(device, buffers.dh_dn_pa);
        malloc(device, buffers.dh_dn_pa_pa);
        malloc(device, buffers.dr_pa);
        malloc(device, buffers.dz_pa);
        malloc(device, buffers.dn_pa);
        malloc(device, buffers.dn_pa_pa);
        malloc(device, buffers.buffer);
        using VIEW_SPEC_DOUBLE = tensor::ViewSpec<1, 2*SPEC::HIDDEN_DIM>;
        buffers.buffer_rz = view_range(device, buffers.buffer, 0*SPEC::HIDDEN_DIM, VIEW_SPEC_DOUBLE{});
        using VIEW_SPEC = tensor::ViewSpec<1, SPEC::HIDDEN_DIM>;
        buffers.buffer_r = view_range(device, buffers.buffer, 0*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        buffers.buffer_z = view_range(device, buffers.buffer, 1*SPEC::HIDDEN_DIM, VIEW_SPEC{});
        buffers.buffer_n = view_range(device, buffers.buffer, 2*SPEC::HIDDEN_DIM, VIEW_SPEC{});
    }
    template<typename DEVICE, typename SPEC_1, typename SPEC_2, typename SPEC_OUTPUT>
    void multiply_broadcast_accumulate(DEVICE& device, Tensor<SPEC_1>& t1, Tensor<SPEC_2>& t2, Tensor<SPEC_OUTPUT>& t_output){
        static_assert(length(typename SPEC_1::SHAPE{}) == 2);
        static_assert(length(typename SPEC_2::SHAPE{}) == 1);
        static_assert(get<0>(typename SPEC_1::SHAPE{}) == get<0>(typename SPEC_OUTPUT::SHAPE{}));
        static_assert(get<1>(typename SPEC_1::SHAPE{}) == get<0>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_OUTPUT::SHAPE{}) == get<1>(typename SPEC_1::SHAPE{}));
        using TI = typename DEVICE::index_t;
        using T = typename SPEC_1::T;
        for(TI i=0; i < get<0>(typename SPEC_1::SHAPE{}); i++){
            for(TI j=0; j < get<1>(typename SPEC_1::SHAPE{}); j++){
                T t1_value = get(device, t1, i, j);
                T t2_value = get(device, t2, j);
                T t_output_value = get(device, t_output, i, j);
                set(device, t_output, t1_value * t2_value + t_output_value, i, j);
            }
        }
    }

    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC>
    void forward(DEVICE& device, nn::layers::gru::LayerBackward<LAYER_SPEC>& layer, const Tensor<INPUT_SPEC>& input){
        static_assert(nn::layers::gru::check_input_output<LAYER_SPEC, INPUT_SPEC, typename decltype(layer.output)::SPEC>, "Input and output spec not matching");
        using TI = typename DEVICE::index_t;

        for(TI step_i=0; step_i < LAYER_SPEC::SEQUENCE_LENGTH; ++step_i){
            auto input_step = view(device, input, step_i);
            auto post_activation_step = view(device, layer.post_activation, step_i);
            auto n_pre_pre_activation_step = view(device, layer.n_pre_pre_activation, step_i);
            auto output_step = view(device, layer.output, step_i);

            auto rz_post_activation = view_range(device, post_activation_step, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, 2*LAYER_SPEC::HIDDEN_DIM>{});
            auto r_post_activation  = view_range(device, post_activation_step, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});
            auto z_post_activation  = view_range(device, post_activation_step, 1*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});
            auto n_post_activation  = view_range(device, post_activation_step, 2*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});

            if(step_i == 0){
                nn::layers::gru::helper::matrix_multiply_broadcast_transpose_bias(device, layer.weights_hidden.parameters, layer.initial_hidden_state.parameters, layer.biases_hidden.parameters, post_activation_step);
            }
            else{
                auto output_previous_step = view(device, layer.output, step_i-1);
                nn::layers::gru::helper::matrix_multiply_transpose_bias(device, layer.weights_hidden.parameters, output_previous_step, layer.biases_hidden.parameters, post_activation_step);
            }

            copy(device, device, n_post_activation, n_pre_pre_activation_step);
            set_all(device, n_post_activation, 0);

            nn::layers::gru::helper::matrix_multiply_transpose_bias_accumulate(device, layer.weights_input.parameters, input_step, layer.biases_input.parameters, post_activation_step);
            sigmoid(device, rz_post_activation);
            multiply_accumulate(device, n_pre_pre_activation_step, r_post_activation, n_post_activation);
            tanh(device, n_post_activation);
            one_minus(device, z_post_activation, output_step);
            multiply(device, output_step, n_post_activation);
            if(step_i == 0){
                multiply_broadcast_accumulate(device, z_post_activation, layer.initial_hidden_state.parameters, output_step);
            }
            else{
                auto output_previous_step = view(device, layer.output, step_i-1);
                multiply_accumulate(device, z_post_activation, output_previous_step, output_step);
            }
        }
    }
    template<typename DEVICE, typename SPEC_FACTOR, typename SPEC_1, typename SPEC_2, typename SPEC_OUTPUT>
    void multiply_subtract_broadcast(DEVICE& device, Tensor<SPEC_FACTOR>& factor, Tensor<SPEC_1>& t1, Tensor<SPEC_2>& t2, Tensor<SPEC_OUTPUT>& t_output) {
        // broadcast t1 along first dimension
        static_assert(length(typename SPEC_FACTOR::SHAPE{}) == 2);
        static_assert(get<0>(typename SPEC_FACTOR::SHAPE{}) == get<0>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_FACTOR::SHAPE{}) == get<1>(typename SPEC_2::SHAPE{}));
        static_assert(length(typename SPEC_1::SHAPE{}) == 1);
        static_assert(length(typename SPEC_2::SHAPE{}) == 2);
        static_assert(get<0>(typename SPEC_2::SHAPE{}) == get<0>(typename SPEC_OUTPUT::SHAPE{}));
        static_assert(get<0>(typename SPEC_1::SHAPE{}) == get<1>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_OUTPUT::SHAPE{}) == get<1>(typename SPEC_2::SHAPE{}));
        using TI = typename DEVICE::index_t;
        using T = typename SPEC_1::T;
        for(TI i=0; i < get<0>(typename SPEC_2::SHAPE{}); i++){
            for(TI j=0; j < get<1>(typename SPEC_2::SHAPE{}); j++){
                T factor_value = get(device, factor, i, j);
                T t1_value = get(device, t1, j);
                T t2_value = get(device, t2, i, j);
                set(device, t_output, factor_value*(t1_value - t2_value), i, j);
            }
        }
    }
    namespace tensor::ternary_operations{
        template <typename T>
        T multiply_subtract(T factor, T a, T b){
            return factor * (a-b);
        }
    }
    template<typename DEVICE, typename SPEC_FACTOR, typename SPEC_1, typename SPEC_2, typename SPEC_OUT>
    void multiply_subtract(DEVICE& device, Tensor<SPEC_FACTOR>& factor, Tensor<SPEC_1>& t1, Tensor<SPEC_2>& t2, Tensor<SPEC_OUT>& result){
        ternary_operation(device, tensor::Operation<tensor::ternary_operations::multiply_subtract<typename SPEC_1::T>, tensor::OperationEmptyParameter>{}, factor, t1, t2, result);
    }
    template<typename DEVICE, typename SPEC_1, typename SPEC_2, typename SPEC_OUT>
    void matrix_multiply_broadcast_accumulate(DEVICE& device, Tensor<SPEC_1>& t1, Tensor<SPEC_2>& t2, Tensor<SPEC_OUT>& result){
        static_assert(length(typename SPEC_1::SHAPE{}) == 2);
        static_assert(length(typename SPEC_2::SHAPE{}) == 1);
        static_assert(length(typename SPEC_OUT::SHAPE{}) == 2);
        static_assert(get<0>(typename SPEC_1::SHAPE{}) == get<0>(typename SPEC_OUT::SHAPE{}));
        static_assert(get<0>(typename SPEC_2::SHAPE{}) == get<1>(typename SPEC_OUT::SHAPE{}));
        using T = typename SPEC_1::T;
        using TI = typename DEVICE::index_t;
        for(TI row_i=0; row_i < get<0>(typename SPEC_1::SHAPE{}); ++row_i){
            for(TI col_j=0; col_j < get<0>(typename SPEC_2::SHAPE{}); ++col_j){
                T acc = get(device, result, row_i, col_j);
                T t2_value = get(device, t2, col_j);
                for(TI k=0; k < get<1>(typename SPEC_1::SHAPE{}); ++k){
                    acc += get(device, t1, row_i, k) * t2_value;
                }
                set(device, result, acc, row_i, col_j);
            }
        }
    }
    template<typename DEVICE, typename SPEC_1, typename SPEC_2, typename SPEC_OUT>
    void matrix_multiply_accumulate_reduce(DEVICE& device, const Tensor<SPEC_1>& t1, const Tensor<SPEC_2>& t2, Tensor<SPEC_OUT>& result){
        static_assert(length(typename SPEC_1::SHAPE{}) == 2);
        static_assert(length(typename SPEC_2::SHAPE{}) == 2);
        static_assert(length(typename SPEC_OUT::SHAPE{}) == 1);
        static_assert(get<1>(typename SPEC_1::SHAPE{}) == get<0>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_2::SHAPE{}) == get<0>(typename SPEC_OUT::SHAPE{}));
        using T = typename SPEC_1::T;
        using TI = typename DEVICE::index_t;
        for(TI row_i=0; row_i < get<0>(typename SPEC_1::SHAPE{}); ++row_i){
            for(TI col_j=0; col_j < get<1>(typename SPEC_2::SHAPE{}); ++col_j){
                T acc = get(device, result, col_j);
                for(TI k=0; k < get<1>(typename SPEC_1::SHAPE{}); ++k){
                    acc += get(device, t1, row_i, k) * get(device, t2, k, col_j);
                }
                set(device, result, acc, col_j);
            }
        }
    }
    template<typename DEVICE, typename SPEC_1, typename SPEC_2, typename SPEC_OUTPUT>
    void multiply_accumulate_reduce(DEVICE& device, Tensor<SPEC_1>& t1, Tensor<SPEC_2>& t2, Tensor<SPEC_OUTPUT>& t_output){
        static_assert(length(typename SPEC_1::SHAPE{}) == 2);
        static_assert(length(typename SPEC_2::SHAPE{}) == 2);
        static_assert(length(typename SPEC_OUTPUT::SHAPE{}) == 1);
        static_assert(get<0>(typename SPEC_1::SHAPE{}) == get<0>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_1::SHAPE{}) == get<1>(typename SPEC_2::SHAPE{}));
        static_assert(get<1>(typename SPEC_2::SHAPE{}) == get<0>(typename SPEC_OUTPUT::SHAPE{}));
        using T = typename SPEC_1::T;
        using TI = typename DEVICE::index_t;
        for(TI row_i=0; row_i < get<0>(typename SPEC_1::SHAPE{}); ++row_i){
            for(TI col_j=0; col_j < get<1>(typename SPEC_1::SHAPE{}); ++col_j){
                T increment = get(device, t1, row_i, col_j) * get(device, t2, row_i, col_j);
                set(device, t_output, get(device, t_output, col_j) + increment, col_j);
            }
        }
    }
    template<typename DEVICE, typename SPEC>
    void zero_gradient(DEVICE& device, nn::layers::gru::LayerBackwardGradient<SPEC>& layer) {
        zero_gradient(device, layer.weights_input);
        zero_gradient(device, layer.biases_input);
        zero_gradient(device, layer.weights_hidden);
        zero_gradient(device, layer.biases_hidden);
        zero_gradient(device, layer.initial_hidden_state);
    }

    template<typename DEVICE, typename SPEC>
    void one_minus(DEVICE& device, Tensor<SPEC>& t){
        using T = typename SPEC::T;
        using PARAMETER = T;
        tensor::Operation<tensor::unary_operations::one_minus<DEVICE, PARAMETER, T>, PARAMETER> op;
        unary_operation(device, op, t);
    }
    template<typename DEVICE, typename SPEC, typename SPEC_OUTPUT>
    void one_minus(DEVICE& device, Tensor<SPEC>& t, Tensor<SPEC_OUTPUT>& output){
        using T = typename SPEC::T;
        using PARAMETER = T;
        tensor::Operation<tensor::unary_operations::one_minus<DEVICE, PARAMETER, T>, PARAMETER> op;
        unary_operation(device, op, t, output);
    }

    namespace tensor::ternary_operations{
        template <typename T>
        T multiply_one_minus_times_d_tanh_post_activation(T factor, T one_minus, T tanh_post_activation){
            return factor * (1-one_minus) * (1-tanh_post_activation*tanh_post_activation);
        }
    }

    template<typename DEVICE, typename SPEC_FACTOR, typename SPEC_OM, typename SPEC_TANH, typename SPEC_RESULT>
    void multiply_one_minus_times_d_tanh_post_activation(DEVICE& device, Tensor<SPEC_FACTOR>& factor, Tensor<SPEC_OM>& one_minus, Tensor<SPEC_TANH>& tanh_post_activation, Tensor<SPEC_RESULT>& result){
        using T = typename SPEC_FACTOR::T;
        using PARAMETER = T;
        tensor::Operation<tensor::ternary_operations::multiply_one_minus_times_d_tanh_post_activation<T>, PARAMETER> op;
        ternary_operation(device, op, factor, one_minus, tanh_post_activation, result);
    }
    namespace tensor::binary_operations{
        template <typename T>
        T multiply_d_sigmoid_post_activation(T factor, T post_activation){
            return factor * post_activation * (1-post_activation);
        }
    }
    template<typename DEVICE, typename SPEC_FACTOR, typename SPEC_PA, typename SPEC_RESULT>
    void multiply_d_sigmoid_post_activation(DEVICE& device, Tensor<SPEC_FACTOR>& factor, Tensor<SPEC_PA>& pre_activation, Tensor<SPEC_RESULT>& result){
        using T = typename SPEC_FACTOR::T;
        using PARAMETER = T;
        tensor::Operation<tensor::binary_operations::multiply_d_sigmoid_post_activation<T>, PARAMETER> op;
        binary_operation(device, op, factor, pre_activation, result);
    }

    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename D_OUTPUT_SPEC, typename D_INPUT_SPEC>
    void backward(DEVICE& device, nn::layers::gru::LayerBackwardGradient<LAYER_SPEC>& layer, const Tensor<INPUT_SPEC>& input, Tensor<D_OUTPUT_SPEC>& d_output, Tensor<D_INPUT_SPEC>& d_input, nn::layers::gru::BuffersBackward<LAYER_SPEC>& buffers, typename DEVICE::index_t step_i){
        // warning: this modifies d_output!
        static_assert(tensor::same_dimensions<typename decltype(layer.output)::SPEC, D_OUTPUT_SPEC>());
        static_assert(tensor::same_dimensions<INPUT_SPEC, D_INPUT_SPEC>());
        static_assert(nn::layers::gru::check_input_output<LAYER_SPEC, INPUT_SPEC, typename decltype(layer.output)::SPEC>, "Input and output spec not matching");
        using TI = typename DEVICE::index_t;
        auto input_step = view(device, input, step_i);
        auto n_pre_pre_activation_step = view(device, layer.n_pre_pre_activation, step_i);
        auto post_activation_step = view(device, layer.post_activation, step_i);
        auto d_output_step = view(device, d_output, step_i);
        auto d_input_step = view(device, d_input, step_i);


        auto rz_post_activation = view_range(device, post_activation_step, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, 2*LAYER_SPEC::HIDDEN_DIM>{});
        auto r_post_activation = view_range(device, post_activation_step, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});
        auto z_post_activation = view_range(device, post_activation_step, 1*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});
        auto n_post_activation = view_range(device, post_activation_step, 2*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<1, LAYER_SPEC::HIDDEN_DIM>{});

        if(step_i == 0){
            multiply_subtract_broadcast(device, d_output_step, layer.initial_hidden_state.parameters, n_post_activation, buffers.buffer_z);
            auto d_output_previous_step = layer.initial_hidden_state.gradient;
            multiply_accumulate_reduce(device, d_output_step, z_post_activation, d_output_previous_step);
        }
        else{
            auto output_previous_step = view(device, layer.output, step_i-1);
            multiply_subtract(device, d_output_step, output_previous_step, n_post_activation, buffers.buffer_z);
            auto d_output_previous_step = view(device, d_output, step_i-1);
            multiply_accumulate(device, d_output_step, z_post_activation, d_output_previous_step);
        }
        multiply_one_minus_times_d_tanh_post_activation(device, d_output_step, z_post_activation, n_post_activation, buffers.buffer_n);
        multiply(device, buffers.buffer_n, n_pre_pre_activation_step, buffers.buffer_r);
        multiply_d_sigmoid_post_activation(device, buffers.buffer_rz, rz_post_activation, buffers.buffer_rz);
        auto buffer_transpose = permute(device, buffers.buffer, tensor::PermutationSpec<1, 0>{});
        static_assert(decltype(buffer_transpose)::SPEC::SIZE == decltype(buffers.buffer)::SPEC::SIZE);
        matrix_multiply_accumulate(device, buffer_transpose, input_step, layer.weights_input.gradient);

        reduce_sum<true>(device, buffer_transpose, layer.biases_input.gradient);
        auto b_irz_grad = view_range(device, layer.biases_input.gradient, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<0, 2*LAYER_SPEC::HIDDEN_DIM>{});
        auto b_hrz_grad = view_range(device, layer.biases_hidden.gradient, 0*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<0, 2*LAYER_SPEC::HIDDEN_DIM>{});
        copy(device, device, b_irz_grad, b_hrz_grad);

        matrix_multiply_accumulate(device, buffers.buffer, layer.weights_input.parameters, d_input_step);

        multiply(device, buffers.buffer_n, r_post_activation);

        if(step_i == 0){
            matrix_multiply_broadcast_accumulate(device, buffer_transpose, layer.initial_hidden_state.parameters, layer.weights_hidden.gradient);
            auto d_output_previous_step = layer.initial_hidden_state.gradient;
            matrix_multiply_accumulate_reduce(device, buffers.buffer, layer.weights_hidden.parameters, d_output_previous_step);
        }
        else{
            auto output_previous_step = view(device, layer.output, step_i-1);
            matrix_multiply_accumulate(device, buffer_transpose, output_previous_step, layer.weights_hidden.gradient);
            auto d_output_previous_step = view(device, d_output, step_i-1);
            matrix_multiply_accumulate(device, buffers.buffer, layer.weights_hidden.parameters, d_output_previous_step);
        }

        auto b_hn_grad = view_range(device, layer.biases_hidden.gradient, 2*LAYER_SPEC::HIDDEN_DIM, tensor::ViewSpec<0, LAYER_SPEC::HIDDEN_DIM>{});
        auto buffer_n_transpose = permute(device, buffers.buffer_n, tensor::PermutationSpec<1, 0>{});
        reduce_sum<true>(device, buffer_n_transpose, b_hn_grad);
    }

    template <typename DEVICE, typename SPEC>
    void free(DEVICE& device, nn::layers::gru::Layer<SPEC>& layer){
        free(device, layer.weights_input);
        free(device, layer.biases_input);
        free(device, layer.weights_hidden);
        free(device, layer.biases_hidden);
        free(device, layer.initial_hidden_state);
    }
    template <typename DEVICE, typename SPEC>
    void free(DEVICE& device, nn::layers::gru::LayerBackward<SPEC>& layer){
        free(device, static_cast<nn::layers::gru::Layer<SPEC>&>(layer));
        free(device, layer.n_pre_pre_activation);
        free(device, layer.post_activation);
        free(device, layer.output);
    }
    template <typename DEVICE, typename SPEC>
    void free(DEVICE& device, nn::layers::gru::BuffersBackward<SPEC>& layer){
        free(device, layer.dh_dz);
        free(device, layer.dz_dz_pa);
        free(device, layer.dh_dn);
        free(device, layer.dn_dn_pa);
        free(device, layer.dn_dn_pa_pa);
    }
}
RL_TOOLS_NAMESPACE_WRAPPER_END

#endif