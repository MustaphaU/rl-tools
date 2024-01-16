#include <rl_tools/operations/cpu_mux.h>
#include <rl_tools/nn/operations_cpu_mux.h>

#include <rl_tools/rl/environments/acrobot/operations_generic.h>
#if RL_TOOLS_ENABLE_GTK
#include <rl_tools/rl/environments/acrobot/ui.h>
#endif

#include <rl_tools/nn_models/operations_generic.h>

#include <rl_tools/rl/algorithms/td3/loop.h>
namespace rlt = RL_TOOLS_NAMESPACE_WRAPPER ::rl_tools;

namespace training_config {
    using namespace rlt::nn_models::sequential::interface; // to simplify the model definition we import the sequential interface but we don't want to pollute the global namespace hence we do it in a model definition namespace
    struct Config{
        using DEV_SPEC = rlt::devices::DefaultCPUSpecification;
//    using DEVICE = rlt::devices::CPU<DEV_SPEC>;
        using DEVICE = rlt::DEVICE_FACTORY<DEV_SPEC>;
        static constexpr bool CONSTRUCT_LOGGER = false;
        using T = float;
        using TI = typename DEVICE::index_t;

        using ENV_SPEC = rlt::rl::environments::acrobot::Specification<T, TI, rlt::rl::environments::acrobot::DefaultParameters<T>>;
        using ENVIRONMENT = rlt::rl::environments::Acrobot<ENV_SPEC>;
        using ENVIRONMENT_EVALUATION = ENVIRONMENT;
#if RL_TOOLS_ENABLE_GTK
//        using UI = rlt::rl::environments::acrobot::UI<rlt::rl::environments::acrobot::ui::Specification<T, TI, ENVIRONMENT, 300, 1600, false>>;
        using UI = bool;
#else
        using UI = bool;
#endif

        struct DEVICE_SPEC: rlt::devices::DefaultCPUSpecification {
            using LOGGING = rlt::devices::logging::CPU;
        };
        struct TD3PendulumParameters: rlt::rl::algorithms::td3::DefaultParameters<T, DEVICE::index_t>{
            constexpr static typename DEVICE::index_t CRITIC_BATCH_SIZE = 100;
            constexpr static typename DEVICE::index_t ACTOR_BATCH_SIZE = 100;
            constexpr static T GAMMA = 0.997;
            static constexpr bool IGNORE_TERMINATION = false;
        };

        using TD3_PARAMETERS = TD3PendulumParameters;

        template <typename PARAMETER_TYPE>
        struct ACTOR{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = TD3_PARAMETERS::ACTOR_BATCH_SIZE;
            using LAYER_1_SPEC = rlt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM, HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_1_SPEC>;
            using LAYER_2_SPEC = rlt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_2_SPEC>;
            using LAYER_3_SPEC = rlt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, ENVIRONMENT::ACTION_DIM, rlt::nn::activation_functions::ActivationFunction::TANH, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        template <typename PARAMETER_TYPE>
        struct CRITIC{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = TD3_PARAMETERS::CRITIC_BATCH_SIZE;

            using LAYER_1_SPEC = rlt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_1_SPEC>;
            using LAYER_2_SPEC = rlt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_2_SPEC>;
            using LAYER_3_SPEC = rlt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, 1, rlt::nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = rlt::nn::layers::dense::LayerBackwardGradient<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };



        //using ACTOR_STRUCTURE_SPEC = rlt::nn_models::mlp::StructureSpecification<T, TI, ENVIRONMENT::OBSERVATION_DIM, ENVIRONMENT::ACTION_DIM, 3, 64, rlt::nn::activation_functions::RELU, rlt::nn::activation_functions::TANH, TD3_PARAMETERS::ACTOR_BATCH_SIZE>;
        //using CRITIC_STRUCTURE_SPEC = rlt::nn_models::mlp::StructureSpecification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, 1, 3, 64, rlt::nn::activation_functions::RELU, rlt::nn::activation_functions::IDENTITY, TD3_PARAMETERS::CRITIC_BATCH_SIZE>;

        using OPTIMIZER_SPEC = rlt::nn::optimizers::adam::Specification<T, TI>;

        using OPTIMIZER = rlt::nn::optimizers::Adam<OPTIMIZER_SPEC>;
        using ACTOR_TYPE = typename ACTOR<rlt::nn::parameters::Adam>::MODEL;
        using ACTOR_TARGET_TYPE = typename ACTOR<rlt::nn::parameters::Plain>::MODEL;
        using CRITIC_TYPE = typename CRITIC<rlt::nn::parameters::Adam>::MODEL;
        using CRITIC_TARGET_TYPE = typename CRITIC<rlt::nn::parameters::Plain>::MODEL;

        using ACTOR_CRITIC_SPEC = rlt::rl::algorithms::td3::Specification<T, DEVICE::index_t, ENVIRONMENT, ACTOR_TYPE, ACTOR_TARGET_TYPE, CRITIC_TYPE, CRITIC_TARGET_TYPE, OPTIMIZER, TD3_PARAMETERS>;
        using ACTOR_CRITIC_TYPE = rlt::rl::algorithms::td3::ActorCritic<ACTOR_CRITIC_SPEC>;


        static constexpr int N_WARMUP_STEPS_ACTOR = TD3_PARAMETERS::ACTOR_BATCH_SIZE;
        static constexpr int N_WARMUP_STEPS_CRITIC = TD3_PARAMETERS::CRITIC_BATCH_SIZE;
#ifndef RL_TOOLS_STEP_LIMIT
        static constexpr DEVICE::index_t STEP_LIMIT = 50000; //2 * N_WARMUP_STEPS;
#else
        static constexpr DEVICE::index_t STEP_LIMIT = RL_TOOLS_STEP_LIMIT;
#endif
        static constexpr bool DETERMINISTIC_EVALUATION = false;
        static constexpr DEVICE::index_t EVALUATION_INTERVAL = 1000;
        static constexpr TI NUM_EVALUATION_EPISODES = 10;
        static constexpr typename DEVICE::index_t REPLAY_BUFFER_CAP = STEP_LIMIT;
        static constexpr typename DEVICE::index_t ENVIRONMENT_STEP_LIMIT = 500;
        static constexpr bool COLLECT_EPISODE_STATS = false;
        static constexpr DEVICE::index_t EPISODE_STATS_BUFFER_SIZE = 1000;
        using OFF_POLICY_RUNNER_SPEC = rlt::rl::components::off_policy_runner::Specification<
                T,
                DEVICE::index_t,
                ENVIRONMENT,
                1,
                false,
                REPLAY_BUFFER_CAP,
                ENVIRONMENT_STEP_LIMIT,
                rlt::rl::components::off_policy_runner::DefaultParameters<T>,
                false,
                COLLECT_EPISODE_STATS,
                EPISODE_STATS_BUFFER_SIZE
        >;
        const T STATE_TOLERANCE = 0.00001;
        static_assert(ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE == ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::CRITIC_BATCH_SIZE);
    };
}

int main(){
    using CONFIG = typename training_config::Config;
    using TI = typename CONFIG::TI ;
    rlt::rl::algorithms::td3::loop::TrainingState<CONFIG> ts;
//    rlt::init(ts.device, ts.envs[0]);
    rlt::rl::algorithms::td3::loop::init(ts, 3);
//    ts.off_policy_runner.parameters.exploration_noise = 0.1;
//    ts.envs[0].parameters.dt = 0.01;
    for(TI step_i=0; step_i < CONFIG::STEP_LIMIT; step_i++){
        rlt::rl::algorithms::td3::loop::step(ts);
        rlt::set_state(ts.device, ts.envs[0], ts.ui, get(ts.off_policy_runner.states, 0, 0));
        rlt::render(ts.device, ts.envs[0], ts.ui);
    }
    return 0;
}