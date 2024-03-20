#include "Camera.hpp"
#include "Game.hpp"

namespace iq {

    Camera::Camera() :
        m_needs_matrix_update(true), 
        m_fov(120.0f),  m_near(1.0f),m_far(60.0f),
        m_target{0.0f, 0.0f, 0.0f},
        m_position{0.0, 1.5f, 1.5f}
    {

        m_position_interpolator.setSpeed(2.0f);
     }

    float Camera::fov() const { return m_fov;  }

    HMM_Vec3 const& Camera::target() const { return m_target; }
    HMM_Vec3 const& Camera::position() const { return m_position; }

    HMM_Mat4 const& Camera::projectionMatrix() const { return m_projection_matrix; }
    HMM_Mat4 const& Camera::viewMatrix() const { return m_view_matrix; }
    HMM_Mat4 const& Camera::viewProjectionMatrix() const { return m_view_projection_matrix; }

    void Camera::setFov(float fov) {
        m_fov = fov;
        m_needs_matrix_update = true;
    }

    void Camera::setTarget(HMM_Vec3 const& target) {
        m_target = target;
        m_needs_matrix_update = true;
    }

    void Camera::setPosition(HMM_Vec3 const& position) {
        m_position = position;
        m_needs_matrix_update = true;
    }

    void Camera::followPlayer(Game& game, bool jump) {
        GameSettings const& settings = game.settings();

        auto player_position = game.map().player().position();
        float width = game.map().columns();

        HMM_Vec3 camera_target = settings.player_camera_target;
        camera_target.Z += player_position.Z;
        camera_target.Y += player_position.Y;
        camera_target.X += width / 2.0f;
        setTarget(camera_target);

        const float dilated_width = width * 2.0f;
        float factor = (player_position.X / width) * 2.0f - 1.0f;
        const float sign = factor >= 0.0f ? 1.0f : -1.0f;
        factor = (easeInQuad(std::abs(factor)) * sign) * 0.5 + 0.5f;

        HMM_Vec3 camera_position = player_position + settings.player_camera_position;
        camera_position.X = (dilated_width * factor) - (dilated_width - width) / 2.0f;

        if (jump) {
            m_position_interpolator.reset(camera_position);
            setPosition(camera_position);
        } else {
            m_position_interpolator.move(camera_position);
        }
    }


    void Camera::update(Game& game) {
        Inputs const& inputs = game.inputs();
        GameSettings const& settings = game.settings();
        
        if (game.inputs().resize) 
            m_needs_matrix_update = true;

        if (settings.debug_camera) {
            if (inputs.forward || inputs.backward || inputs.left || inputs.right) {
                double duration = game.inputs().frame_duration;
                const float camera_move_speed = 1.50f * duration;
                const float camera_orbit_speed = 0.5f * duration; // degrees per second
                
                float move = inputs.forward ? -camera_move_speed : (inputs.backward ? camera_move_speed : 0.0f);
                float yaw = inputs.right ? camera_orbit_speed : (inputs.left ? -camera_orbit_speed : 0.0f);
                
                orbit(yaw);
                moveToTarget(move);
                
                game.log("Camera", "Update camera "
                         "forward=" + std::to_string(inputs.forward) + " " +
                         "backward=" + std::to_string(inputs.backward) + " " +
                         "left=" + std::to_string(inputs.left) + " " +
                         "right=" + std::to_string(inputs.right));
                
                m_needs_matrix_update = true;
            }
        } else {
            followPlayer(game, false);

            setPosition(m_position_interpolator.update(game.inputs().frame_duration));
        }

        if (m_needs_matrix_update) {
            const float w = game.viewWidth();
            const float h = game.viewHeight();
            
            m_projection_matrix = HMM_Perspective_RH_NO(HMM_DegToRad * m_fov, w/h, m_near, m_far);
            m_view_matrix = HMM_LookAt_RH(m_position, m_target, upDirection());
            m_view_projection_matrix = HMM_Mul(m_projection_matrix, m_view_matrix);

            m_reflected_view_projection_matrix = HMM_Mul(m_projection_matrix, HMM_Mul(m_view_matrix, m_reflection_matrix));

            m_needs_matrix_update = false;
        }
    }

    HMM_Vec3 Camera::forwardDirection() const {
        return HMM_Norm(HMM_Sub(m_target, m_position));
    }

    HMM_Vec3 Camera::upDirection() const {
        return HMM_Vec3{ 0.0f, 1.0f, 0.0f };
    }

    void Camera::moveToTarget(float delta) {
        float distance = HMM_Len(HMM_Sub(m_position, m_target));

        // Apply delta
        distance += delta;

        // Distance must be greater than 0
        if (distance <= 0) distance = 0.001f;

        // Set new distance by moving the position along the forward vector
        HMM_Vec3 forward = forwardDirection();


        m_position = HMM_Add(m_target, HMM_Mul(forward, -distance));
    }

    void Camera::orbit(float angle) {

        constexpr bool rotateAroundTarget = true;

        // Rotation axis
        HMM_Vec3 up = upDirection();
    
        // View vector
        HMM_Vec3 target_position = HMM_Sub(m_target, m_position);

        // Rotate view vector around up axis
        target_position = HMM_MulM4V4(HMM_Rotate_RH(angle, up), HMM_V4V(target_position, 1.0f)).XYZ;
        
        if (rotateAroundTarget) {
            // Move position relative to target
            m_position = HMM_Sub(m_target, target_position);

        } else { // rotate around camera.position
            // Move target relative to position
            m_position = HMM_Add(m_position, target_position);
        }
        
    }

    // reflections

    // https://gamedev.stackexchange.com/questions/198098/how-do-i-calculate-a-matrix-to-render-a-mirror-about-a-plane-using-glm
    HMM_Vec4 Camera::planeFromPointNormal(HMM_Vec3 const& pt, HMM_Vec3 normal) {
        normal = HMM_Norm(normal);
        float d = -HMM_Dot(pt, normal);
        return HMM_V4(normal.X, normal.Y, normal.Z, d);
    }
    
    HMM_Mat4 Camera::matrixReflect(HMM_Vec4 const& plane) {
        return HMM_Mat4 {
                1.0f-2.0f * plane.X * plane.X,     -2.0f * plane.X * plane.Y,     -2.0f * plane.X * plane.Z, -2.0f * plane.X * plane.W,
                    -2.0f * plane.Y * plane.X, 1.0f-2.0f * plane.Y * plane.Y,     -2.0f * plane.Y * plane.Z, -2.0f * plane.Y * plane.W,
                    -2.0f * plane.Z * plane.X,     -2.0f * plane.Z * plane.Y, 1.0f-2.0f * plane.Z * plane.Z, -2.0f * plane.Z * plane.W,
                0.0f, 0.0f, 0.0f, 1.0f };
    }

    void Camera::setRelectionPlane(HMM_Vec3 const& point, HMM_Vec3 const& normal) {
        HMM_Vec4 plane = planeFromPointNormal(point, normal);
        m_reflection_matrix = matrixReflect(plane);
    }

    HMM_Mat4 const& Camera::reflectedViewProjectionMatrix() const {
        return m_reflected_view_projection_matrix;
    }
}


