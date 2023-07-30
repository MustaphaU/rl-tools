#include <backprop_tools/operations/cpu.h>

#include <backprop_tools/rl/environments/car/car.h>
#include <backprop_tools/rl/environments/car/operations_generic.h>
namespace bpt = backprop_tools;

#include <gtest/gtest.h>

TEST(RL_ENVIRONMENTS_CAR, COMPARISON){
    using DEVICE = bpt::devices::DefaultCPU;
    using T = double;
    using TI = typename DEVICE::index_t;
    using ENV_SPEC = bpt::rl::environments::car::Specification<T, TI>;
    using ENVIRONMENT = bpt::rl::environments::Car<ENV_SPEC>;

    DEVICE device;
    ENVIRONMENT env;
    auto rng = bpt::random::default_engine(DEVICE::SPEC::RANDOM(), 0);

    ENVIRONMENT::State state, next_state;
    bpt::MatrixDynamic<bpt::matrix::Specification<T, TI, 1, ENVIRONMENT::ACTION_DIM>> a;
    bpt::malloc(device, a);

    {
        bpt::initial_state(device, env, state);
        set(a, 0, 0, 0.11);
        set(a, 0, 1, -30.0/180.0*bpt::math::PI<T>);
        for(TI step_i=0; step_i < 100; step_i++){
            bpt::step(device, env, state, a, next_state, rng);
            state = next_state;
            std::cout << "step: " << step_i << " x: " << state.x << ", y: " << state.y << ", mu: " << state.mu << ", vx: " << state.vx << ", vy: " << state.vy << ", omega: " << state.omega << "\n";
        }
        ENVIRONMENT::State target_state = {0.11012406573455273, -0.2547549112167621, -2.1226171833457834, 0.5411147087810739, 0.06409534767749557, -3.1425915975367156};

        EXPECT_NEAR(state.x, target_state.x, 1e-15);
        EXPECT_NEAR(state.y, target_state.y, 1e-15);
        EXPECT_NEAR(state.mu, target_state.mu, 1e-15);
        EXPECT_NEAR(state.vx, target_state.vx, 1e-15);
        EXPECT_NEAR(state.vy, target_state.vy, 1e-15);
        EXPECT_NEAR(state.omega, target_state.omega, 1e-15);
    }

    {
        bpt::initial_state(device, env, state);
        state.mu = 90/180.0*bpt::math::PI<T>;
        set(a, 0, 0, 0.11);
        set(a, 0, 1, -30.0/180.0*bpt::math::PI<T>);
        for(TI step_i=0; step_i < 100; step_i++){
            bpt::step(device, env, state, a, next_state, rng);
            state = next_state;
            std::cout << "step: " << step_i << " x: " << state.x << ", y: " << state.y << ", mu: " << state.mu << ", vx: " << state.vx << ", vy: " << state.vy << ", omega: " << state.omega << "\n";
        }
        ENVIRONMENT::State target_state = {0.2547549112167622, 0.11012406573455266, -0.5518208565508872, 0.5411147087810739, 0.06409534767749557, -3.1425915975367156};

        EXPECT_NEAR(state.x, target_state.x, 1e-15);
        EXPECT_NEAR(state.y, target_state.y, 1e-15);
        EXPECT_NEAR(state.mu, target_state.mu, 1e-15);
        EXPECT_NEAR(state.vx, target_state.vx, 1e-15);
        EXPECT_NEAR(state.vy, target_state.vy, 1e-15);
        EXPECT_NEAR(state.omega, target_state.omega, 1e-15);
    }

}