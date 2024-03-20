#pragma once

#include <Vendor.hpp>

namespace iq {

    enum class ObjectKind {
        None = 0,
        
        Cube                = 10,
        Pyramid             = 20,
        Wall                = 30,

        Text                = 90, 
        CrossFade           = 91,
        
        TestCube            = 11,
        SolidColorCube      = 12,
        TexturedCube        = 13,
        FadeCube            = 14,

        TestPyramid         = 21,
        SolidColorPyramid   = 22,
        TexturedPyramid     = 23,
        FadePyramid         = 24,

        TestWall            = 31,
        SolidColorWall      = 32,
        TexturedWall        = 33,
        FadeWall            = 34
    };

    class Object {
    public:
        Object();
        Object(ObjectKind kind);
        ~Object();
        
        ObjectKind kind() { return m_kind; }
        
        HMM_Vec4 const& color() const { return m_color; }
        HMM_Vec4& color() { return m_color; }

        float& alphaMultiplier() { return m_alpha_multiplier; }
        float alphaMultiplier() const { return m_alpha_multiplier; }
        
        HMM_Vec3 const& position() const { return m_position; }
        HMM_Vec3& position() { return m_position; }

        HMM_Vec3 const& rotation() const { return m_rotation; }
        HMM_Vec3& rotation() { return m_rotation; }

        HMM_Vec3 const& scale() const { return m_scale; }
        HMM_Vec3& scale() { return m_scale; }

        HMM_Vec3 const& textureScale() const { return m_texture_scale; }
        HMM_Vec3& textureScale() { return m_texture_scale; }

        std::string const& texture() const { return m_texture; }

        std::string const& text() const { return m_text; }
        std::string const& fontName() const { return m_font_name; }
        int fontAlign() const { return m_font_align; }

        Object& setColor(HMM_Vec4 const& color);
        Object& setColor(HMM_Vec3 const& color) { return setColor(HMM_Vec4{ color.R, color.G, color.B, 1.0f }); }
        Object& setColor(float r, float g, float b, float a = 1.0f) { return setColor(HMM_Vec4{ r, g, b, a }); }
        Object& setColor(sg_color const& color) { return setColor(HMM_Vec4{ color.r, color.g, color.b, color.a }); }
        Object& setAlphaMultiplier(float multiplier);
        
        Object& setPosition(HMM_Vec3 const& position);
        Object& setPosition(float x, float y) { return setPosition(HMM_V3(x, y, 0.0f)); }
        Object& setPosition(float x, float y, float z) { return setPosition(HMM_V3(x, y, z)); }
        
        Object& setRotation(HMM_Vec3 const& rotation);
        Object& setRotation(float rx = 0.0f, float ry = 0.0f, float rz = 0.0f) { return setRotation(HMM_Vec3{rx, ry, rz}); }
        
        Object& setScale(HMM_Vec3 const& scale);
        Object& setScale(float sx = 1.0f, float sy = 1.0f, float sz = 1.0f) { return setScale(HMM_Vec3{sx, sy, sz}); }

        Object& setTextureScale(HMM_Vec3 const& scale);
        Object& setTextureScale(float sx = 1.0f, float sy = 1.0f, float sz = 1.0f) { return setTextureScale(HMM_Vec3{sx, sy, sz}); }

        Object& setTexture(std::string const& name);

        Object& setText(std::string const& text);
        Object& setFontName(std::string const& name);
        Object& setFontAlign(int const& align);

        HMM_Mat4 modelMatrix() const;
    
        static bool isLikeCube(ObjectKind kind);
        static bool isLikePyramid(ObjectKind kind);
        static bool isLikeWall(ObjectKind kind);

        static HMM_Mat4 const& identity();
        static HMM_Vec3 const& axis(int i);

        Object& setPercentage(float value);
        float percentage() const { return m_percentage; };
    protected:
        ObjectKind m_kind;

        std::string m_texture;
        HMM_Vec3 m_texture_scale;

        HMM_Vec3 m_position;
        HMM_Vec3 m_rotation;
        HMM_Vec3 m_scale;
        
        HMM_Vec4 m_color;

        std::string m_text;
        std::string m_font_name;
        int m_font_align;

        float m_alpha_multiplier;

        float m_percentage;
    }; 
}



