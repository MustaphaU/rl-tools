#include "../../../../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_RL_ALGORITHMS_SAC_LOOP_CORE_CONFIG_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_RL_ALGORITHMS_SAC_LOOP_CORE_CONFIG_H

#include "../../../../../nn_models/sequential/model.h"
#include "../../../../../rl/algorithms/sac/sac.h"


#include "state.h"

#include "../../../../../rl/utils/evaluation.h"



RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools::rl::algorithms::sac::loop::core{
    // Config State (Init/Step)
    using namespace nn_models::sequential::interface;

    template<typename T, typename TI, typename ENVIRONMENT>
    struct DefaultParameters{
        using SAC_PARAMETERS = rl::algorithms::sac::DefaultParameters<T, TI, ENVIRONMENT::ACTION_DIM>;
        static constexpr int N_WARMUP_STEPS = SAC_PARAMETERS::ACTOR_BATCH_SIZE;
        static constexpr TI STEP_LIMIT = 10000;
        static constexpr TI REPLAY_BUFFER_CAP = STEP_LIMIT;
        static constexpr TI ENVIRONMENT_STEP_LIMIT = 200;

        static constexpr TI ACTOR_HIDDEN_DIM = 64;
        static constexpr auto ACTOR_ACTIVATION_FUNCTION = nn::activation_functions::ActivationFunction::RELU;
        static constexpr TI CRITIC_HIDDEN_DIM = 64;
        static constexpr auto CRITIC_ACTIVATION_FUNCTION = nn::activation_functions::ActivationFunction::RELU;

        static constexpr bool COLLECT_EPISODE_STATS = true;
        static constexpr TI EPISODE_STATS_BUFFER_SIZE = 1000;
    };

    template<typename T_DEVICE, typename T_T, typename T_ENVIRONMENT, typename T_PARAMETERS = DefaultParameters<T_T, typename T_DEVICE::index_t, T_ENVIRONMENT>>
    struct DefaultConfig {
        using DEVICE = T_DEVICE;
        using T = T_T;
        using TI = typename DEVICE::index_t;
        using ENVIRONMENT = T_ENVIRONMENT;
        using DEV_SPEC = devices::DefaultCPUSpecification;
        using UI = bool;

        using PARAMETERS = T_PARAMETERS;

        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = nn::layers::dense::LayerBackwardGradient>
        struct ACTOR{
            static constexpr TI HIDDEN_DIM = PARAMETERS::ACTOR_HIDDEN_DIM;
            static constexpr TI BATCH_SIZE = PARAMETERS::SAC_PARAMETERS::ACTOR_BATCH_SIZE;
            static constexpr auto ACTIVATION_FUNCTION = PARAMETERS::ACTOR_ACTIVATION_FUNCTION;
            using LAYER_1_SPEC = nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM, HIDDEN_DIM, ACTIVATION_FUNCTION, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, ACTIVATION_FUNCTION, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            static constexpr TI ACTOR_OUTPUT_DIM = ENVIRONMENT::ACTION_DIM * 2; // to express mean and log_std for each action
            using LAYER_3_SPEC = nn::layers::dense::Specification<T, TI, HIDDEN_DIM, ACTOR_OUTPUT_DIM, nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>; // note the output activation should be identity because we want to sample from a gaussian and then squash afterwards (taking into account the squashing in the distribution)
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = nn::layers::dense::LayerBackwardGradient>
        struct CRITIC{
            static constexpr TI HIDDEN_DIM = PARAMETERS::CRITIC_HIDDEN_DIM;
            static constexpr TI BATCH_SIZE = PARAMETERS::SAC_PARAMETERS::CRITIC_BATCH_SIZE;
            static constexpr auto ACTIVATION_FUNCTION = PARAMETERS::CRITIC_ACTIVATION_FUNCTION;

            using LAYER_1_SPEC = nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, HIDDEN_DIM, ACTIVATION_FUNCTION, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, ACTIVATION_FUNCTION, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            using LAYER_3_SPEC = nn::layers::dense::Specification<T, TI, HIDDEN_DIM, 1, nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        using OPTIMIZER_SPEC = nn::optimizers::adam::Specification<T, TI>;

        using OPTIMIZER = nn::optimizers::Adam<OPTIMIZER_SPEC>;
        using ALPHA_OPTIMIZER = nn::optimizers::Adam<OPTIMIZER_SPEC>;

        using ACTOR_TYPE = typename ACTOR<nn::parameters::Adam>::MODEL;
        using ACTOR_TARGET_TYPE = typename ACTOR<nn::parameters::Adam, nn::layers::dense::Layer>::MODEL;
        using CRITIC_TYPE = typename CRITIC<nn::parameters::Adam>::MODEL;
        using CRITIC_TARGET_TYPE = typename CRITIC<nn::parameters::Adam, nn::layers::dense::Layer>::MODEL;

        using ALPHA_PARAMETER_TYPE = nn::parameters::Adam;

        using ACTOR_CRITIC_SPEC = rl::algorithms::sac::Specification<T, TI, ENVIRONMENT, ACTOR_TYPE, ACTOR_TARGET_TYPE, CRITIC_TYPE, CRITIC_TARGET_TYPE, ALPHA_PARAMETER_TYPE, OPTIMIZER, OPTIMIZER, ALPHA_OPTIMIZER, typename PARAMETERS::SAC_PARAMETERS>;
        using ACTOR_CRITIC_TYPE = rl::algorithms::sac::ActorCritic<ACTOR_CRITIC_SPEC>;

        using OFF_POLICY_RUNNER_SPEC = rl::components::off_policy_runner::Specification<
                T,
                TI,
                ENVIRONMENT,
                1,
                false,
                PARAMETERS::REPLAY_BUFFER_CAP,
                PARAMETERS::ENVIRONMENT_STEP_LIMIT,
                rl::components::off_policy_runner::DefaultParameters<T>,
                true,
                PARAMETERS::COLLECT_EPISODE_STATS,
                PARAMETERS::EPISODE_STATS_BUFFER_SIZE
        >;
        static_assert(ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE == ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::CRITIC_BATCH_SIZE);
        template <typename CONFIG>
        using State = TrainingState<CONFIG>;
    };
}

#endif
