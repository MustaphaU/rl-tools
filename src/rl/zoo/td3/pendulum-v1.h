#include <rl_tools/rl/environments/pendulum/operations_cpu.h>

#include <rl_tools/rl/algorithms/td3/loop/core/config.h>
#include <rl_tools/rl/loop/steps/extrack/config.h>
#include <rl_tools/rl/loop/steps/checkpoint/config.h>
#include <rl_tools/rl/loop/steps/evaluation/config.h>
#include <rl_tools/rl/loop/steps/timing/config.h>


RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools::rl::zoo::td3{
    namespace rlt = rl_tools;
    template <typename DEVICE, typename T, typename TI, typename RNG>
    struct PendulumV1{
        using ENVIRONMENT_SPEC = rlt::rl::environments::pendulum::Specification<T, TI, rlt::rl::environments::pendulum::DefaultParameters<T>>;
        using ENVIRONMENT = rlt::rl::environments::Pendulum<ENVIRONMENT_SPEC>;
        struct LOOP_CORE_PARAMETERS: rlt::rl::algorithms::td3::loop::core::DefaultParameters<T, TI, ENVIRONMENT>{
            static constexpr TI STEP_LIMIT = 20000;
            static constexpr TI ACTOR_NUM_LAYERS = 3;
            static constexpr TI ACTOR_HIDDEN_DIM = 64;
            static constexpr TI CRITIC_NUM_LAYERS = 3;
            static constexpr TI CRITIC_HIDDEN_DIM = 64;
        };
        using LOOP_CORE_CONFIG = rlt::rl::algorithms::td3::loop::core::Config<T, TI, RNG, ENVIRONMENT, LOOP_CORE_PARAMETERS, rlt::rl::algorithms::td3::loop::core::ConfigApproximatorsSequential>;
    };
}
RL_TOOLS_NAMESPACE_WRAPPER_END
