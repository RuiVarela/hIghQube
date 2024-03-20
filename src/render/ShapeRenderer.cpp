#include "ShapeRenderer.hpp"

#include "../Game.hpp"
#include "VisualRenderer.glsl.h"

namespace iq {

    //
    // ShapeRenderer
    //
    ShapeRenderer::ShapeRenderer(std::string const& tag)
        :m_tag(tag), m_game(nullptr)
    { 
        for (int i = 0; i != int(Pipeline::Count); ++i) 
            m_pipeline[i] = {};

        selectPipeline(Pipeline::Normal);

        m_alpha_mutiplier = 1.0f;
    }

    ShapeRenderer::~ShapeRenderer() {
        for (int i = 0; i != int(Pipeline::Count); ++i)
            sg_destroy_pipeline(m_pipeline[i]);

        sg_destroy_shader(m_shader);
        sg_destroy_buffer(m_index_buffer);
        sg_destroy_buffer(m_vertex_buffer);
    }

    void ShapeRenderer::setup(Game* game, ObjectKind kind) {
        m_game = game;
        m_kind = kind;

        assert(m_kind == ObjectKind::Cube || m_kind == ObjectKind::Pyramid || m_kind == ObjectKind::Wall);

        if (m_kind == ObjectKind::Cube)
            generateCubeData();
        else if (m_kind == ObjectKind::Pyramid)
            generatePyramidData();
        else if (m_kind == ObjectKind::Wall)
            generateWallData();
        
        //
        // buffers
        //
        m_vertex_buffer_name = m_tag + "vertices";
        sg_buffer_desc vertex_buffer_desc = {
            .data = { m_vertices.data(), m_vertices.size() * sizeof(float) },
            .label = m_vertex_buffer_name.c_str()
        };
        m_vertex_buffer = sg_make_buffer(vertex_buffer_desc);
        
        
        m_index_buffer_name = m_tag + "indices";
        sg_buffer_desc index_buffer_desc = {
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .data = { m_indices.data(), m_indices.size() * sizeof(uint16_t) },
            .label = m_index_buffer_name.c_str()
        };
        m_index_buffer= sg_make_buffer(index_buffer_desc);

        //
        // shader
        //
        m_shader = sg_make_shader(shape_shader_desc(sg_query_backend()));
        
        //
        // pipeline
        //
        sg_pipeline_desc pipeline_desc = {
            .shader = m_shader,
            .depth = {
                .compare = SG_COMPAREFUNC_LESS_EQUAL,
                .write_enabled = true
            },
            .index_type = SG_INDEXTYPE_UINT16,
            .cull_mode = SG_CULLMODE_BACK
        };
        pipeline_desc.colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .src_factor_alpha = SG_BLENDFACTOR_ONE,
            .dst_factor_alpha = SG_BLENDFACTOR_ZERO
        };

        // test to provide buffer stride, but no attr offsets
        pipeline_desc.layout.buffers[0].stride = (3 + 4 + 2 + 1) * 4;
        pipeline_desc.layout.attrs[ATTR_shape_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
        pipeline_desc.layout.attrs[ATTR_shape_vs_color].format = SG_VERTEXFORMAT_FLOAT4;
        pipeline_desc.layout.attrs[ATTR_shape_vs_tex_coord].format = SG_VERTEXFORMAT_FLOAT2;
        pipeline_desc.layout.attrs[ATTR_shape_vs_face].format = SG_VERTEXFORMAT_FLOAT;

        {
            sg_pipeline_desc description = pipeline_desc;
            int pipeline_index = int(Pipeline::Normal);
            m_pipeline_name[pipeline_index] = m_tag + "Normal";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        {
            sg_pipeline_desc description = pipeline_desc;
            description.depth.compare = SG_COMPAREFUNC_ALWAYS;
            description.depth.write_enabled = false;

            int pipeline_index = int(Pipeline::DepthOff);
            m_pipeline_name[pipeline_index] = m_tag + "DepthOff";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        {
            sg_pipeline_desc description = pipeline_desc;
            description.cull_mode = SG_CULLMODE_FRONT;
            description.sample_count = 1;

            int pipeline_index = int(Pipeline::Reflection);
            m_pipeline_name[pipeline_index] = m_tag + "Reflection";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        {
            sg_pipeline_desc description = pipeline_desc;
            description.stencil = {
                .enabled = true,
                .front = {
                    .compare = SG_COMPAREFUNC_GREATER,
                    .fail_op = SG_STENCILOP_KEEP,
                    .depth_fail_op = SG_STENCILOP_KEEP,
                    .pass_op = SG_STENCILOP_REPLACE
                },
                .back = {
                    .compare = SG_COMPAREFUNC_GREATER,
                    .fail_op = SG_STENCILOP_KEEP,
                    .depth_fail_op = SG_STENCILOP_KEEP,
                    .pass_op = SG_STENCILOP_REPLACE
                },
                .read_mask = 0xFF,
                .write_mask = 0xFF,
                .ref = 1
            };
            description.colors[0].write_mask = SG_COLORMASK_NONE;
            description.depth.write_enabled = false;

            int pipeline_index = int(Pipeline::WriteStencil);
            m_pipeline_name[pipeline_index] = m_tag + "WriteStencil";
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

        {
            sg_pipeline_desc description = pipeline_desc;
            description.cull_mode = SG_CULLMODE_NONE;
            description.depth.write_enabled = false;

            int pipeline_index = int(Pipeline::Explosion);
            m_pipeline_name[pipeline_index] = m_tag + "Explosion";
            description.label = m_pipeline_name[pipeline_index].c_str();
            m_pipeline[pipeline_index] = sg_make_pipeline(description);
        }

        
        //
        // bindings
        //
        m_bindings = {};
        m_bindings.vertex_buffers[0] = m_vertex_buffer;
        m_bindings.index_buffer = m_index_buffer;
        
        DataManager::Texture texture = m_game->dataManager().texture(DataManager::DefaultTexture);
        m_bindings.fs.samplers[SLOT_sampler_0] = texture.sampler;
        m_bindings.fs.images[SLOT_texture_0] = texture.image;
    }

    void ShapeRenderer::generateCubeData() {

        // back, front, left, right, bottom, top
        m_vertices = {
            // Position         Color                 Uv          Face
            -0.5, -0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   1.0f, 1.0f,  0.0f,
             0.5, -0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   0.0f, 1.0f,  0.0f,
             0.5,  0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   0.0f, 0.0f,  0.0f,
            -0.5,  0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   1.0f, 0.0f,  0.0f,

            -0.5, -0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   0.0f, 1.0f,  1.0f,
             0.5, -0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   1.0f, 1.0f,  1.0f,
             0.5,  0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   1.0f, 0.0f,  1.0f,
            -0.5,  0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   0.0f, 0.0f,  1.0f,

            -0.5, -0.5, -0.5,   0.0, 0.0, 1.0, 1.0,   0.0f, 1.0f,  2.0f,
            -0.5,  0.5, -0.5,   0.0, 0.0, 1.0, 1.0,   0.0f, 0.0f,  2.0f,
            -0.5,  0.5,  0.5,   0.0, 0.0, 1.0, 1.0,   1.0f, 0.0f,  2.0f,
            -0.5, -0.5,  0.5,   0.0, 0.0, 1.0, 1.0,   1.0f, 1.0f,  2.0f,

             0.5, -0.5, -0.5,   1.0, 0.5, 0.0, 1.0,   1.0f, 1.0f,  3.0f,
             0.5,  0.5, -0.5,   1.0, 0.5, 0.0, 1.0,   1.0f, 0.0f,  3.0f,
             0.5,  0.5,  0.5,   1.0, 0.5, 0.0, 1.0,   0.0f, 0.0f,  3.0f,
             0.5, -0.5,  0.5,   1.0, 0.5, 0.0, 1.0,   0.0f, 1.0f,  3.0f,

            -0.5, -0.5, -0.5,   0.0, 0.5, 1.0, 1.0,   0.0f, 1.0f,  4.0f,
            -0.5, -0.5,  0.5,   0.0, 0.5, 1.0, 1.0,   0.0f, 0.0f,  4.0f,
             0.5, -0.5,  0.5,   0.0, 0.5, 1.0, 1.0,   1.0f, 0.0f,  4.0f,
             0.5, -0.5, -0.5,   0.0, 0.5, 1.0, 1.0,   1.0f, 1.0f,  4.0f,

            -0.5,  0.5, -0.5,   1.0, 0.0, 0.5, 1.0,   0.0f, 0.0f,  5.0f,
            -0.5,  0.5,  0.5,   1.0, 0.0, 0.5, 1.0,   0.0f, 1.0f,  5.0f,
             0.5,  0.5,  0.5,   1.0, 0.0, 0.5, 1.0,   1.0f, 1.0f,  5.0f,
             0.5,  0.5, -0.5,   1.0, 0.0, 0.5, 1.0,   1.0f, 0.0f,  5.0f
        };

        m_indices = {
            0, 1, 2,  0, 2, 3,
            6, 5, 4,  7, 6, 4,
            8, 9, 10,  8, 10, 11,
            14, 13, 12,  15, 14, 12,
            16, 17, 18,  16, 18, 19,
            22, 21, 20,  23, 22, 20
        };
    }

    void ShapeRenderer::generatePyramidData() {
        // back, front, left, right, top
        m_vertices = {
             // Position           Color                     Uv          Face
             0.0f, -0.5f,  0.0f,   1.0f, 0.0f, 0.0f, 1.0f,   0.5f, 1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 0.0f,  0.0f,

             0.0f, -0.5f,  0.0f,   0.0f, 1.0f, 0.0f, 1.0f,   0.5f, 1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f,  1.0f,

             0.0f, -0.5f,  0.0f,   0.0f, 0.0f, 1.0f, 1.0f,   0.5f, 1.0f,  2.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f,  2.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f,  2.0f,

             0.0f, -0.5f,  0.0f,   1.0f, 0.5f, 0.0f, 1.0f,   0.5f, 1.0f,  3.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.0f, 1.0f,   0.0f, 0.0f,  3.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 0.5f, 0.0f, 1.0f,   1.0f, 0.0f,  3.0f,

            -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.5f, 1.0f,   0.0f, 0.0f,  5.0f,
            -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.5f, 1.0f,   0.0f, 1.0f,  5.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.5f, 1.0f,   1.0f, 1.0f,  5.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.5f, 1.0f,   1.0f, 0.0f,  5.0f
        };

        m_indices = {
            0, 1, 2,
            3, 4, 5, 
            6, 7, 8,
            9, 10, 11,

            14, 13, 12,  15, 14, 12
        };
    }

    void ShapeRenderer::generateWallData() {

        // back, left, right, front
        m_vertices = {
            // Position         Color                 Uv          Face
            -0.5, -0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   1.0f, 1.0f,  0.0f,
             0.5, -0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   0.0f, 1.0f,  0.0f,
             0.5,  0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   0.0f, 0.0f,  0.0f,
            -0.5,  0.5, -0.5,   1.0, 0.0, 0.0, 1.0,   1.0f, 0.0f,  0.0f,

            -0.5, -0.5, -0.5,   0.0, 0.0, 1.0, 1.0,   0.0f, 1.0f,  2.0f,
            -0.5,  0.5, -0.5,   0.0, 0.0, 1.0, 1.0,   0.0f, 0.0f,  2.0f,
            -0.5,  0.5,  0.5,   0.0, 0.0, 1.0, 1.0,   1.0f, 0.0f,  2.0f,
            -0.5, -0.5,  0.5,   0.0, 0.0, 1.0, 1.0,   1.0f, 1.0f,  2.0f,

             0.5, -0.5, -0.5,   1.0, 0.5, 0.0, 1.0,   1.0f, 1.0f,  3.0f,
             0.5,  0.5, -0.5,   1.0, 0.5, 0.0, 1.0,   1.0f, 0.0f,  3.0f,
             0.5,  0.5,  0.5,   1.0, 0.5, 0.0, 1.0,   0.0f, 0.0f,  3.0f,
             0.5, -0.5,  0.5,   1.0, 0.5, 0.0, 1.0,   0.0f, 1.0f,  3.0f,

            -0.5, -0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   0.0f, 1.0f,  1.0f,
             0.5, -0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   1.0f, 1.0f,  1.0f,
             0.5,  0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   1.0f, 0.0f,  1.0f,
            -0.5,  0.5,  0.5,   0.0, 1.0, 0.0, 1.0,   0.0f, 0.0f,  1.0f,
        };

        m_indices = {
            0, 1, 2,  0, 2, 3,
            4, 5, 6,  4, 6, 7,
            10, 9, 8,  11, 10, 8,
            14, 13, 12,  15, 14, 12,
        };
    }
            
    void ShapeRenderer::render(std::vector<Object*> objects, HMM_Mat4 const& view_projection) {
        sg_apply_pipeline(m_pipeline[int(m_selected_pipeline)]);
        sg_apply_bindings(&m_bindings);
        
        for (Object* o : objects) {
            int object_kind = int(o->kind()) - int(m_kind);

            // textured object
            if (object_kind == 3) {
                DataManager::Texture texture = m_game->dataManager().texture(o->texture());
                if (!texture.valid) {
                    m_game->log("ShapeRenderer", "Unable to load texture: " + o->texture());
                    texture = m_game->dataManager().texture(DataManager::DefaultTexture);
                } 
                m_bindings.fs.images[SLOT_texture_0] = texture.image;
                m_bindings.fs.samplers[SLOT_sampler_0] = texture.sampler;
                sg_apply_bindings(&m_bindings);
            }
            
            HMM_Mat4 model = o->modelMatrix();

            shape_vs_params_t vs_params = {
                .texture_scaler = o->textureScale(),
                .object_kind = object_kind,
                .mvp = HMM_Mul(view_projection, model)
            };
            
            shape_fs_params_t fs_params = {
                .object_kind = object_kind,
                .color = o->color()
            };

            fs_params.color.A *= o->alphaMultiplier() * m_alpha_mutiplier;
            
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_shape_vs_params, SG_RANGE(vs_params));
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_shape_fs_params, SG_RANGE(fs_params));
            sg_draw(0, (int)m_indices.size(), 1);
        }
    }
}