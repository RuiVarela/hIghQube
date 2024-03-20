#include "TextureRenderer.hpp"

#include "../Game.hpp"
#include "VisualRenderer.glsl.h"

namespace iq {

    //
    // TextureRenderer
    //
    TextureRenderer::TextureRenderer(std::string const& tag)
        :m_tag(tag), m_game(nullptr)
    {  
        for (int i = 0; i != int(Pipeline::Count); ++i) 
            m_pipeline[i] = {};

        selectPipeline(Pipeline::Normal);
    }

    TextureRenderer::~TextureRenderer() {
        for (int i = 0; i != int(Pipeline::Count); ++i)
            sg_destroy_pipeline(m_pipeline[i]);

        sg_destroy_shader(m_shader);
        sg_destroy_buffer(m_vertex_buffer);
        sg_destroy_sampler(m_sampler);
    }

    void TextureRenderer::setup(Game* game) {
        m_game = game;

        m_vertices = { 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f };

        //
        // buffers
        //
        m_vertex_buffer_name = m_tag + "vertices";
        sg_buffer_desc vertex_buffer_desc = {
            .data = { m_vertices.data(), m_vertices.size() * sizeof(float) },
            .label = m_vertex_buffer_name.c_str()
        };
        m_vertex_buffer = sg_make_buffer(vertex_buffer_desc);
        
        //
        // shader
        //
        m_shader = sg_make_shader(fsq_shader_desc(sg_query_backend()));

        //
        // pipeline
        //

        sg_pipeline_desc pipeline_desc = {
            .shader = m_shader,
            .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
        };
        pipeline_desc.colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .src_factor_alpha = SG_BLENDFACTOR_ONE,
            .dst_factor_alpha = SG_BLENDFACTOR_ZERO
        };

        // test to provide buffer stride, but no attr offsets
        pipeline_desc.layout.attrs[ATTR_fsq_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
  
        {
            sg_pipeline_desc description = pipeline_desc;
            int pipeline_index = int(Pipeline::Normal);
            m_pipeline_name[pipeline_index] = m_tag + "Normal";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        {
            sg_pipeline_desc description = pipeline_desc;
            description.stencil = {
                .enabled = true,
                .front = {
                    .compare = SG_COMPAREFUNC_EQUAL,
                    .fail_op = SG_STENCILOP_KEEP,
                    .depth_fail_op = SG_STENCILOP_KEEP,
                    .pass_op = SG_STENCILOP_REPLACE
                },
                .back = {
                    .compare = SG_COMPAREFUNC_EQUAL,
                    .fail_op = SG_STENCILOP_KEEP,
                    .depth_fail_op = SG_STENCILOP_KEEP,
                    .pass_op = SG_STENCILOP_REPLACE
                },
                .read_mask = 0xFF,
                .write_mask = 0x00,
                .ref = 1
            };
            
            description.depth.compare = SG_COMPAREFUNC_ALWAYS;
            description.depth.write_enabled = false;
            
            int pipeline_index = int(Pipeline::ClipWithStencil);
            m_pipeline_name[pipeline_index] = m_tag + "ClipWithStencil";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        //
        // Sampler
        //
        m_sampler_name = m_tag + "sampler";
        sg_sampler_desc sampler_desc = {
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
            .label = m_sampler_name.c_str()
        };
        m_sampler = sg_make_sampler(&sampler_desc);

        //
        // bindings
        //
        m_bindings = {};
        m_bindings.vertex_buffers[0] = m_vertex_buffer;
        m_bindings.fs.samplers[SLOT_sampler_0] = m_sampler;
    }

    void TextureRenderer::setImage(sg_image image, HMM_Vec4 tint) {
        m_bindings.fs.images[SLOT_texture_0] = image;
        m_tint = tint;
    }

    void TextureRenderer::render() {
        sg_apply_pipeline(m_pipeline[int(m_selected_pipeline)]);
        sg_apply_bindings(&m_bindings);

        fsq_fs_params_t fs_params = {
            .color = m_tint
        };

        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_shape_fs_params, SG_RANGE(fs_params));
        sg_draw(0, 4, 1);
    }
}