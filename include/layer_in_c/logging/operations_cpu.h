#ifndef LAYER_IN_C_LOGGING_OPERATIONS_CPU_H
#define LAYER_IN_C_LOGGING_OPERATIONS_CPU_H



#include <iostream>

namespace layer_in_c{
    namespace logging{
        template <typename A>
        void text(devices::logging::CPU& dev, const A a){
            std::cout << a << std::endl;
        }
        template <typename A, typename B>
        void text(devices::logging::CPU& dev, const A a, const B b){
            std::cout << a << b << std::endl;
        }
        template <typename A, typename B, typename C, typename D>
        void text(devices::logging::CPU& dev, const A a, const B b, const C c, const D d){
            std::cout << a << b << c << d << std::endl;
        }
    }
    void add_scalar(devices::logging::CPU& dev, const char* key, const float value, const typename devices::logging::CPU::index_t cadence = 1){
        //noop
    }
}
#endif
