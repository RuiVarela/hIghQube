#include "Object.hpp"

namespace iq {

    Object::Object()
        :m_texture_scale{1.0f, 1.0f, 1.0f},
        m_position{0.0f, 0.0f, 0.0f},
        m_rotation{0.0f, 0.0f, 0.0f},
        m_scale{1.0f, 1.0f, 1.0f},
        m_color{1.0f, 1.0f, 1.0f, 1.0f},
        m_percentage(0.0),
        m_font_align(FONSalign::FONS_ALIGN_LEFT),
        m_alpha_multiplier(1.0f)
    {
        m_kind = ObjectKind::None;
    }

    Object::Object(ObjectKind kind)
        :Object()
    {
        m_kind = kind;
    }

    Object::~Object() {
        
    }

    Object& Object::setColor(HMM_Vec4 const& color) {
        m_color = color;
        return *this;
    }
    
    Object& Object::setAlphaMultiplier(float multiplier) {
        m_alpha_multiplier = multiplier;
        return *this;
    }

    Object& Object::setPosition(HMM_Vec3 const& position) {
        m_position = position;
        return *this;
    }

    Object& Object::setRotation(HMM_Vec3 const& rotation) {
        m_rotation = rotation;
        return *this;
    }

    Object& Object::setScale(HMM_Vec3 const& scale) {
        m_scale = scale;
        return *this;
    }

    Object& Object::setTextureScale(HMM_Vec3 const& scale) {
        m_texture_scale = scale;
        return *this;
    }

    Object& Object::setTexture(std::string const& name) {
        m_texture = name;
        return *this;
    }

    Object& Object::setText(std::string const& text) {
        m_text = text;
        return *this;
    }

    Object& Object::setFontName(std::string const& name) {
        m_font_name = name;
        return *this;
    }

    Object& Object::setFontAlign(int const& align) {
        m_font_align = align;
        return *this;
    }

    HMM_Mat4 const& Object::identity() {
        static HMM_Mat4 value = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
            
        };
        return value;
    }

    HMM_Vec3 const& Object::axis(int i) {
        static std::array<HMM_Vec3, 3> value {
            HMM_Vec3{1.0f, 0.0f, 0.0f},
            HMM_Vec3{0.0f, 1.0f, 0.0f},
            HMM_Vec3{0.0f, 0.0f, 1.0f}
        };
        return value[i];
    }

    bool Object::isLikeCube(ObjectKind kind) {
        return (kind == ObjectKind::TestCube) || (kind == ObjectKind::SolidColorCube) ||
               (kind == ObjectKind::TexturedCube);
    }

    bool Object::isLikePyramid(ObjectKind kind) {
        return (kind == ObjectKind::TestPyramid) || (kind == ObjectKind::SolidColorPyramid) ||
               (kind == ObjectKind::TexturedPyramid);
    }

    bool Object::isLikeWall(ObjectKind kind) {
        return (kind == ObjectKind::TestWall);
    }

    HMM_Mat4 Object::modelMatrix() const {
        HMM_Mat4 rotation_matrix = Object::identity();

        float cubeFlipPercentage = percentage();

        if (cubeFlipPercentage > 0.0f) {
            HMM_Mat4 rot = HMM_Rotate_RH(cubeFlipPercentage * HMM_PI * 0.5f, Object::axis(0));
            HMM_Mat4 tx1 = HMM_Translate(HMM_V3(0.0f, -0.5f, 0.5f));
            HMM_Mat4 tx2 = HMM_Translate(HMM_V3(0.0f, 0.5f, -0.5f));

            rotation_matrix = HMM_Mul(HMM_Mul(tx1, rot), tx2);
        } 

        for (int i = 0; i != 3; ++i) {
            float angle = rotation().Elements[i];
            if (angle != 0.0f) {
                HMM_Mat4 matrix = HMM_Rotate_RH(angle, Object::axis(i));
                rotation_matrix = HMM_Mul(rotation_matrix, matrix);
            }
        }

        HMM_Mat4 scale_matrix = HMM_Scale(scale());
        HMM_Mat4 translation_matrix = HMM_Translate(position());

        HMM_Mat4 model = HMM_Mul(translation_matrix, HMM_Mul(scale_matrix, rotation_matrix));

        return model;
    }

    Object& Object::setPercentage(float value) {
        m_percentage = HMM_Clamp(0.0f, value, 1.0f);
        return *this;
    }
}



