#include <backprop_tools/operations/cpu_mux.h>
#include <backprop_tools/nn/operations_cpu_mux.h>

#include <backprop_tools/rl/environments/pendulum/operations_cpu.h>

#include <backprop_tools/nn_models/operations_generic.h>

#include <backprop_tools/rl/algorithms/td3/loop.h>


namespace training_config{
    using namespace backprop_tools::nn_models::sequential::interface;
    struct TrainingConfig{
        using DEV_SPEC = bpt::devices::DefaultCPUSpecification;
//    using DEVICE = bpt::devices::CPU<DEV_SPEC>;
        using DEVICE = bpt::DEVICE_FACTORY<DEV_SPEC>;
        using T = float;
        using TI = typename DEVICE::index_t;

        using PENDULUM_SPEC = bpt::rl::environments::pendulum::Specification<T, TI, bpt::rl::environments::pendulum::DefaultParameters<T>>;
        using ENVIRONMENT = bpt::rl::environments::Pendulum<PENDULUM_SPEC>;
        using UI = bool;

        struct DEVICE_SPEC: bpt::devices::DefaultCPUSpecification {
            using LOGGING = bpt::devices::logging::CPU;
        };
        struct TD3PendulumParameters: bpt::rl::algorithms::td3::DefaultParameters<T, DEVICE::index_t>{
            constexpr static typename DEVICE::index_t CRITIC_BATCH_SIZE = 100;
            constexpr static typename DEVICE::index_t ACTOR_BATCH_SIZE = 100;
            constexpr static T GAMMA = 0.997;
        };

        using TD3_PARAMETERS = TD3PendulumParameters;

        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = bpt::nn::layers::dense::LayerBackwardGradient>
        struct ACTOR{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = TD3_PARAMETERS::ACTOR_BATCH_SIZE;
            using LAYER_1_SPEC = bpt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            using LAYER_3_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, ENVIRONMENT::ACTION_DIM, bpt::nn::activation_functions::ActivationFunction::TANH, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = bpt::nn::layers::dense::LayerBackwardGradient>
        struct CRITIC{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = TD3_PARAMETERS::CRITIC_BATCH_SIZE;

            using LAYER_1_SPEC = bpt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            using LAYER_3_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, 1, bpt::nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        using OPTIMIZER_PARAMETERS = typename bpt::nn::optimizers::adam::DefaultParametersTorch<T, typename DEVICE::index_t>;
        using OPTIMIZER = bpt::nn::optimizers::Adam<OPTIMIZER_PARAMETERS>;

        using ACTOR_TYPE = ACTOR<bpt::nn::parameters::Adam>::MODEL;
        using ACTOR_TARGET_TYPE = ACTOR<bpt::nn::parameters::Adam, bpt::nn::layers::dense::Layer>::MODEL;
        using CRITIC_TYPE = CRITIC<bpt::nn::parameters::Adam>::MODEL;
        using CRITIC_TARGET_TYPE = CRITIC<bpt::nn::parameters::Adam, bpt::nn::layers::dense::Layer>::MODEL;

        using ACTOR_CRITIC_SPEC = bpt::rl::algorithms::td3::Specification<T, DEVICE::index_t, ENVIRONMENT, ACTOR_TYPE, ACTOR_TARGET_TYPE, CRITIC_TYPE, CRITIC_TARGET_TYPE, OPTIMIZER, TD3_PARAMETERS>;
        using ACTOR_CRITIC_TYPE = bpt::rl::algorithms::td3::ActorCritic<ACTOR_CRITIC_SPEC>;


        static constexpr int N_WARMUP_STEPS = ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE;
#ifndef BACKPROP_TOOLS_STEP_LIMIT
        static constexpr DEVICE::index_t STEP_LIMIT = 10000; //2 * N_WARMUP_STEPS;
#else
        static constexpr DEVICE::index_t STEP_LIMIT = BACKPROP_TOOLS_STEP_LIMIT;
#endif
        static constexpr bool DETERMINISTIC_EVALUATION = true;
        static constexpr DEVICE::index_t EVALUATION_INTERVAL = 1000;
        static constexpr typename DEVICE::index_t REPLAY_BUFFER_CAP = 10000;
        static constexpr typename DEVICE::index_t ENVIRONMENT_STEP_LIMIT = 200;
        static constexpr bool COLLECT_EPISODE_STATS = true;
        static constexpr DEVICE::index_t EPISODE_STATS_BUFFER_SIZE = 1000;
        using OFF_POLICY_RUNNER_SPEC = bpt::rl::components::off_policy_runner::Specification<
                T,
                DEVICE::index_t,
                ENVIRONMENT,
                1,
                false,
                REPLAY_BUFFER_CAP,
                ENVIRONMENT_STEP_LIMIT,
                bpt::rl::components::off_policy_runner::DefaultParameters<T>,
                COLLECT_EPISODE_STATS,
                EPISODE_STATS_BUFFER_SIZE
        >;
        const T STATE_TOLERANCE = 0.00001;
        static_assert(ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE == ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::CRITIC_BATCH_SIZE);
    };
}
using TrainingConfig = training_config::TrainingConfig;

int main(){
    using TI = typename TrainingConfig::TI;
    TrainingState<TrainingConfig> ts;
//    bpt::init(ts.device, ts.envs[0]);
    training_init(ts, 3);
//    ts.envs[0].parameters.dt = 0.01;
    for(TI step_i=0; step_i < TrainingConfig::STEP_LIMIT; step_i++){
        training_step(ts);
    }
    return 0;
}