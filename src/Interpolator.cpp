#include "Interpolator.hpp"

//https://theorangeduck.com/page/spring-roll-call
static float fast_negexp(float x) {
    return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
}

static float halflife_to_damping(float halflife, float eps = 1e-5f) {
    return (4.0f * 0.69314718056f) / (halflife + eps);
}

static void simple_spring_damper_implicit(float& x, float& v, float x_goal, float halflife, float dt) {
    float y = halflife_to_damping(halflife) / 2.0f;
    float j0 = x - x_goal;
    float j1 = v + j0*y;
    float eydt = fast_negexp(y*dt);

    x = eydt*(j0 + j1*dt) + x_goal;
    v = eydt*(v - j1*y*dt);
}

namespace iq {

    Random::Random() {
        reset(0);
    }

    void Random::reset(uint32_t seed) {
        if (seed == 0) {
            m_engine.seed(std::random_device()());
        } else {
            m_engine.seed(seed);
        }
    }

    uint32_t Random::nextInt() {
        return m_distribution(m_engine);
	}

	uint32_t Random::nextInt(uint32_t min, uint32_t max) {
        return min + (m_distribution(m_engine) % (max - min + 1));
    }

    float Random::nextFloat() {
        return (float)m_distribution(m_engine) / (float)std::numeric_limits<uint32_t>::max();
    }

    float Random::nextFloat(float min, float max) {
        return min + nextFloat() * (max - min);
    }

    HMM_Vec3 Random::nextVec3() {
        return HMM_V3(nextFloat(), nextFloat(), nextFloat());
    }

    HMM_Vec3 Random::nextVec3(float min, float max) {
        return HMM_V3(nextFloat(min, max), nextFloat(min, max), nextFloat(min, max));
    }


    //
    // String interpolator
    //

    SpringIntepolator::SpringIntepolator() {
        setSpeed(1.0f);
        reset();
    }

    void SpringIntepolator::reset(HMM_Vec3 position) {
        m_position = position;
        m_velocity = HMM_V3(0.0f, 0.0f, 0.0f);
        m_target = position;
    }

    HMM_Vec3 const& SpringIntepolator::position() const {
        return m_position;
    }

    HMM_Vec3 const& SpringIntepolator::velocity() const {
        return m_velocity;
    }

    HMM_Vec3 const& SpringIntepolator::target() const {
        return m_target;
    }

    void SpringIntepolator::setSpeed(float speed) {
        const float base_time = 0.3f;
        m_halflife = base_time / speed;
    }

    void SpringIntepolator::move(HMM_Vec3 const& target) {
        m_target = target;
    }

    bool SpringIntepolator::isMoving(float speed_theshold, float distance_threshold) const {
        float distance = HMM_Len(target() - position());
        float speed = HMM_Len(velocity());

        bool stopped = speed < speed_theshold;
        bool arrived = distance < distance_threshold;

        // std::cout << 
        //     "distance=" << distance << "/" << distance_threshold << " | " << 
        //     "speed=" << speed << "/" << speed_theshold << " | " << 
        //     "arrived=" << arrived << " stopped=" << stopped << " | " << 
        //     "m_halflife=" << m_halflife << 
        //      std::endl;

       return !(stopped && arrived);
    }

    HMM_Vec3 SpringIntepolator::update(double dt) {

        if (dt > 0.0) {
            simple_spring_damper_implicit(m_position.X, m_velocity.X, m_target.X, m_halflife, dt);
            simple_spring_damper_implicit(m_position.Y, m_velocity.Y, m_target.Y, m_halflife, dt);
            simple_spring_damper_implicit(m_position.Z, m_velocity.Z, m_target.Z, m_halflife, dt);
        }

        isMoving();

        return m_position;
    }


    Speeder::Speeder()
        :m_real_time(0.0), m_streched_time(0.0)
    { }

    void Speeder::update(double t, double multiplier) {
        double delta = t - m_real_time;

        m_real_time = t;
        m_streched_time += delta * multiplier;
    }

    double Speeder::time() const {
        return m_streched_time;
    }

    double Speeder::elapsed(double start) const {
        return m_streched_time - start;
    }
}
