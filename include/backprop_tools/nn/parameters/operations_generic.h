#include "../../version.h"
#if (defined(BACKPROP_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(BACKPROP_TOOLS_NN_PARAMETERS_OPERATIONS_GENERIC_H)) && (BACKPROP_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define BACKPROP_TOOLS_NN_PARAMETERS_OPERATIONS_GENERIC_H

#include "parameters.h"

BACKPROP_TOOLS_NAMESPACE_WRAPPER_START
namespace backprop_tools{
    template <typename DEVICE, typename CONTAINER>
    void malloc(DEVICE& device, nn::parameters::Plain::instance<CONTAINER>& p){
        malloc(device, p.parameters);
    }
    template <typename DEVICE, typename CONTAINER>
    void free(DEVICE& device, nn::parameters::Plain::instance<CONTAINER>& p){
        free(device, p.parameters);
    }
    template <typename DEVICE, typename CONTAINER>
    void malloc(DEVICE& device, nn::parameters::Gradient::instance<CONTAINER>& p){
        malloc(device, (nn::parameters::Plain::instance<CONTAINER>&) p);
        malloc(device, p.gradient);
    }
    template <typename DEVICE, typename CONTAINER>
    void free(DEVICE& device, nn::parameters::Gradient::instance<CONTAINER>& p){
        free(device, (nn::parameters::Plain::instance<CONTAINER>&) p);
        free(device, p.gradient);
    }
    template<typename DEVICE, typename CONTAINER>
    void zero_gradient(DEVICE& device, nn::parameters::Gradient::instance<CONTAINER>& container) {
        set_all(device, container.gradient, 0);
    }

    template<typename SOURCE_DEVICE, typename TARGET_DEVICE, typename SOURCE_SPEC, typename TARGET_SPEC>
    void copy(SOURCE_DEVICE& source_device, TARGET_DEVICE& target_device, const nn::parameters::Plain::instance<SOURCE_SPEC>& source, nn::parameters::Plain::instance<TARGET_SPEC>& target){
        copy(source_device, target_device, source.parameters, target.parameters);
    }

    template<typename SOURCE_DEVICE, typename TARGET_DEVICE, typename SOURCE_SPEC, typename TARGET_SPEC>
    void copy(SOURCE_DEVICE& source_device, TARGET_DEVICE& target_device, const nn::parameters::Gradient::instance<SOURCE_SPEC>& source, nn::parameters::Gradient::instance<TARGET_SPEC>& target){
        copy(source_device, target_device, (nn::parameters::Plain::instance<SOURCE_SPEC>&) source, (nn::parameters::Plain::instance<TARGET_SPEC>&) target);
        copy(source_device, target_device, source.gradient, target.gradient);
    }

    template<typename DEVICE, typename SPEC_1, typename SPEC_2>
    typename SPEC_1::CONTAINER::T abs_diff(DEVICE& device, const nn::parameters::Plain::instance<SPEC_1>& p1, const nn::parameters::Plain::instance<SPEC_2>& p2){
        typename SPEC_1::CONTAINER::T acc = 0;
        acc += abs_diff(device, p1.parameters, p2.parameters);
        return acc;
    }

    template<typename DEVICE, typename SPEC_1, typename SPEC_2>
    typename SPEC_1::CONTAINER::T abs_diff(DEVICE& device, const nn::parameters::Gradient::instance<SPEC_1>& p1, const nn::parameters::Gradient::instance<SPEC_2>& p2){
        typename SPEC_1::CONTAINER::T acc = 0;
        acc += abs_diff(device, (nn::parameters::Plain::instance<SPEC_1>&) p1, (nn::parameters::Plain::instance<SPEC_2>&) p2);
        acc += abs_diff(device, p1.gradient, p2.gradient);
        return acc;
    }
}
BACKPROP_TOOLS_NAMESPACE_WRAPPER_END
#endif
