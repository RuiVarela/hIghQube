#pragma once

#include <Vendor.hpp>

#include "Interpolator.hpp"

namespace iq {
    class Game;

    class Camera {
    public:
        Camera();
        
        float fov() const; // field of view
        HMM_Vec3 const& target() const;  // where we are looking at
        HMM_Vec3 const& offset() const;  // offset from target
        HMM_Vec3 const& position() const;  // camera position
        
    
        void setFov(float fov);
        void setTarget(HMM_Vec3 const& target);
        void setPosition(HMM_Vec3 const& position);

        void followPlayer(Game& game, bool jump = false);



        void update(Game& game);

        void moveToTarget(float delta);
        void orbit(float angle);

        HMM_Vec3 forwardDirection() const;
        HMM_Vec3 upDirection() const;

        HMM_Mat4 const& projectionMatrix() const;
        HMM_Mat4 const& viewMatrix() const;
        HMM_Mat4 const& viewProjectionMatrix() const;

        // reflections
        void setRelectionPlane(HMM_Vec3 const& point, HMM_Vec3 const& normal);
        HMM_Mat4 const& reflectedViewProjectionMatrix() const;
    private:
        bool m_needs_matrix_update;
        HMM_Mat4 m_projection_matrix;
        HMM_Mat4 m_view_matrix;
        HMM_Mat4 m_view_projection_matrix;

        float m_fov;
        float m_near;
        float m_far;
        HMM_Vec3 m_target;
        HMM_Vec3 m_position;
        SpringIntepolator m_position_interpolator;

        // reflections
        HMM_Mat4 m_reflection_matrix;
        HMM_Mat4 m_reflected_view_projection_matrix;
        HMM_Vec4 planeFromPointNormal(HMM_Vec3 const& pt, HMM_Vec3 normal);
        HMM_Mat4 matrixReflect(HMM_Vec4 const& plane);

    };
}


