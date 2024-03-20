@ctype mat4 HMM_Mat4
@ctype vec2 HMM_Vec2
@ctype vec3 HMM_Vec3
@ctype vec4 HMM_Vec4

//
// Shape
//

@vs shape_vs
uniform shape_vs_params {
    vec3 texture_scaler;
    int object_kind;
    mat4 mvp;
};

in vec4 position;
in vec4 color;
in vec2 tex_coord;
in float face;

out vec4 v_color;
out vec2 v_tex_coord;

void main() {
    gl_Position = mvp * position;
    v_color = color;
    
    vec2 texture_multiplier = vec2(1.0, 1.0);
    
    if (face < 2.0) { // back front
        texture_multiplier = vec2(texture_scaler.x, texture_scaler.y);
    } else if (face > 3.0) { // top, bottom
        texture_multiplier = vec2(texture_scaler.x, texture_scaler.z);
    } else { // left, right
        texture_multiplier = vec2(texture_scaler.z, texture_scaler.y);
    }
    
    v_tex_coord = tex_coord * texture_multiplier;
}
@end

@fs shape_fs

uniform shape_fs_params {
    int object_kind;
    vec4 color;
};

in vec4 v_color;
in vec2 v_tex_coord;

uniform texture2D texture_0;
uniform sampler sampler_0;

out vec4 frag_color;

void main() {
    if (object_kind == 1) {
        // test
        frag_color = v_color;
    } else if (object_kind == 2) {
        // solid color
        frag_color = color;
    } else if (object_kind == 3) {
        // textured 
        frag_color = texture(sampler2D(texture_0, sampler_0), v_tex_coord) * color;
    } else if (object_kind == 4) {
        // fade 
        frag_color = color;
        frag_color.a = frag_color.a * v_tex_coord.y;
    } else {
        frag_color = vec4(1.0);
    }
}
@end

@program shape shape_vs shape_fs

//
// Fullscreen Quad
// shaders for rendering a fullscreen-quad 
//
@vs fsq_vs
@glsl_options flip_vert_y

in vec2 position;
out vec2 v_tex_coord;

void main() {
    gl_Position = vec4(position * 2.0 - 1.0, 0.5, 1.0);
    gl_Position.y *= -1.0f;
    v_tex_coord = position;
}
@end

@fs fsq_fs

uniform fsq_fs_params {
    vec4 color;
};

in vec2 v_tex_coord;

uniform texture2D texture_0;
uniform sampler sampler_0;

out vec4 frag_color;

void main() {
    vec4 tex_color = texture(sampler2D(texture_0, sampler_0), v_tex_coord);
    frag_color = tex_color * color;
}
@end

@program fsq fsq_vs fsq_fs