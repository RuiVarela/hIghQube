#pragma once

#include <Vendor.hpp>
#include <cstdint>


namespace iq {
    constexpr float easeInLinear(float x)  { return x; }

    constexpr float easeInCubic(float x) { return x * x * x; }
    constexpr float easeOutCubic(float x) { return 1.0f - pow(1.0f - x, 3.0f); }
    constexpr float easeInOutCubic(float x) { return x < 0.5f ? 4.0f * x * x * x : 1 - pow(-2.0f * x + 2.0f, 3.0f) / 2.0f; }

    constexpr float easeInQuad(float x)  { return x * x; }
    constexpr float easeOutQuad(float x) { return 1.0f - (1.0f - x) * (1.0f - x);}


    constexpr float easeInQuart(float x) { return x * x * x * x; }
    constexpr float easeOutQuart(float x) { return 1.0f - pow(1.0f - x, 4.0f); } 

    inline float easeInSine(float x) { return 1.0f - cos((x * float(HMM_PI)) / 2.0f); }


    class Random {
	public:
        Random();

        void reset(uint32_t seed = 0);

        uint32_t nextInt();
		uint32_t nextInt(uint32_t min, uint32_t max);

        float nextFloat();
        float nextFloat(float min, float max);

        HMM_Vec3 nextVec3();
        HMM_Vec3 nextVec3(float min, float max);

    private:
    	std::mt19937 m_engine;
		std::uniform_int_distribution<std::mt19937::result_type> m_distribution;
    };


    class SpringIntepolator {
        public:
            SpringIntepolator();

            void reset(HMM_Vec3 position = HMM_V3(0.0f, 0.0f, 0.0f));
            void setSpeed(float speed);
            void move(HMM_Vec3 const& target);
            HMM_Vec3 update(double dt);

            HMM_Vec3 const& position() const;
            HMM_Vec3 const& velocity() const;
            HMM_Vec3 const& target() const;

            bool isMoving(float speed_theshold = 0.01f, float distance_threshold = 0.001f) const;
            
        private:
            HMM_Vec3 m_position;
            HMM_Vec3 m_velocity;
            HMM_Vec3 m_target;
            float m_halflife;
    };


    class Speeder {
        public:
            Speeder();

            void update(double t, double multiplier);

            double time() const;
            double elapsed(double start) const;

        private:
            double m_real_time;
            double m_streched_time;
    };

}
