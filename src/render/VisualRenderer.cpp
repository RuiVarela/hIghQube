#include "VisualRenderer.hpp"
#include "../Game.hpp"
#include "VisualRenderer.glsl.h"

#include "../vendor/fontstash/src/fontstash.h"
#include "../vendor/sokol/util/sokol_fontstash.h"

namespace iq {

    //
    // VisualRenderer
    //
    VisualRenderer::VisualRenderer()
        :m_game(nullptr), 
        m_cube_renderer("cube-"), m_pyramid_renderer("pyramid-"), m_explosion_renderer("explosion-"), 
        m_texture_renderer("texture-"), m_text_renderer("text-"), m_crossfade_renderer("cross-fade-"),
        m_render_pass {}
    { 
    }

    VisualRenderer::~VisualRenderer() {
        destroyOffscreen();
    }
                
    void VisualRenderer::setup(Game* game) {
        m_game = game;
        m_render_pass = { 
            .depth = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = 1.0f
            },
            .stencil = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = 0
            }
        };
        m_render_pass.colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = m_game->dataManager().backgroundColor()
        };

        m_cube_renderer.setup(m_game, ObjectKind::Cube);
        m_pyramid_renderer.setup(m_game, ObjectKind::Pyramid);
        m_explosion_renderer.setup(m_game, ObjectKind::Wall);
        m_texture_renderer.setup(m_game);
        m_text_renderer.setup(m_game);
        m_crossfade_renderer.setup(m_game);
    }

    void VisualRenderer::render(std::vector<Object*> const& game_overlays, bool render_map) {
        Map& map = m_game->map();

        std::vector<Object*> objects;


        if (render_map) {
            //
            // Offscreen
            // draw the cubes reflection
            //
            if ((m_offscreen.vw != m_game->viewWidth()) || (m_offscreen.vh != m_game->viewHeight())) {
                destroyOffscreen();
                buildOffscreen();
            }
            sg_begin_pass(m_offscreen.pass, &m_offscreen.pass_action);
                objects.clear();
                map.waveObjectsForReflection(objects);

                HMM_Mat4 reflected_view_projection = map.camera().reflectedViewProjectionMatrix();
                m_cube_renderer.selectPipeline(ShapeRenderer::Pipeline::Reflection);
                m_cube_renderer.render(objects, reflected_view_projection);
            sg_end_pass();
        }



        //
        // Normal pass
        //
        sg_begin_default_pass(&m_render_pass, m_game->viewWidth(), m_game->viewHeight());

            if (render_map) {
                HMM_Mat4 view_projection = map.camera().viewProjectionMatrix();

                // write stencil mask
                {
                    objects.clear();
                    map.groundRectObjects(objects);

                    m_cube_renderer.selectPipeline(ShapeRenderer::Pipeline::WriteStencil);
                    m_cube_renderer.render(objects, view_projection);
                }

                // draw ground
                {
                    objects.clear();
                    map.groundObjects(objects);

                    m_cube_renderer.selectPipeline(ShapeRenderer::Pipeline::Normal);
                    m_cube_renderer.render(objects, view_projection);
                }

                // wave reflections
                {
                    m_texture_renderer.selectPipeline(TextureRenderer::Pipeline::ClipWithStencil);
                    m_texture_renderer.render();
                }

                // ground markers
                {
                    objects.clear();
                    map.player().groundMarkers(objects);

                    m_cube_renderer.selectPipeline(ShapeRenderer::Pipeline::ClipWithStencil);
                    m_cube_renderer.render(objects, view_projection);
                }

                // wave cubes
                {
                    objects.clear();
                    map.waveObjects(objects);

                    m_cube_renderer.selectPipeline(ShapeRenderer::Pipeline::Normal);
                    m_cube_renderer.render(objects, view_projection);
                }

                // explode effects
                {
                    objects.clear();
                    map.explodingObjects(objects);

                    m_explosion_renderer.selectPipeline(ShapeRenderer::Pipeline::Explosion);
                    m_explosion_renderer.render(objects, view_projection);
                }

                // player
                {
                    objects.clear();
                    map.player().playerObjects(objects);

                    m_pyramid_renderer.selectPipeline(ShapeRenderer::Pipeline::Normal);
                    m_pyramid_renderer.render(objects, view_projection);
                    
                    objects.clear();
                    map.player().airGuides(objects);

                    m_pyramid_renderer.selectPipeline(ShapeRenderer::Pipeline::DepthOff);
                    m_pyramid_renderer.render(objects, view_projection);
                }
            }


            //
            // Hud stuff using sgl
            //
            sgl_defaults();
            sgl_matrix_mode_projection();
            sgl_ortho(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, -1.0f, +1.0f);
                m_text_renderer.preRender();

                if (render_map) {
                    // Text Objects
                    objects.clear();
                    map.textObjects(objects);

                    m_text_renderer.render(objects);
                }

                // overlay cross fade
                {
                    m_crossfade_renderer.render(game_overlays);
                }


                // overlat text
                {
                    m_text_renderer.render(game_overlays);
                }

                m_text_renderer.postRender();
            // kick sgl rendering
            sgl_draw();

        sg_end_pass();   
    }

    void VisualRenderer::buildOffscreen() {
        m_offscreen.vw = m_game->viewWidth();
        m_offscreen.vh = m_game->viewHeight();
        if (m_offscreen.vw == 0 || m_offscreen.vh == 0) return;

        // create offscreen rendertarget images and pass
        sg_image_desc color_img_desc = {
            .render_target = true,
            .width = m_offscreen.vw / 1,
            .height = m_offscreen.vh / 1,
            .sample_count = 1,
            .label = "offscreen color"
        };

        sg_image_desc depth_img_desc = color_img_desc;
        depth_img_desc.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        depth_img_desc.label = "offscreen depth";

        m_offscreen.pass_desc = {
            .label = "offscreen pass"
        };
        m_offscreen.pass_desc.color_attachments[0].image = sg_make_image(&color_img_desc);
        m_offscreen.pass_desc.depth_stencil_attachment.image = sg_make_image(&depth_img_desc);

        m_offscreen.pass = sg_make_pass(&m_offscreen.pass_desc);

        // pass action for offscreen pass
        m_offscreen.pass_action = {};
        m_offscreen.pass_action.colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .store_action = SG_STOREACTION_DONTCARE,
            .clear_value = { 1.0f, 1.0f, 1.0f, 0.0f }
        };

        m_offscreen.pass_action.depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .store_action = SG_STOREACTION_STORE,
            .clear_value = 1.0f,
        };


       m_texture_renderer.setImage(m_offscreen.pass_desc.color_attachments[0].image, HMM_V4(1.0f, 1.0f, 1.0f, 0.2f));
    }

    void VisualRenderer::destroyOffscreen() {
        if (m_offscreen.vw == 0 || m_offscreen.vh == 0) return;

        sg_destroy_pass(m_offscreen.pass);
        sg_destroy_image(m_offscreen.pass_desc.color_attachments[0].image);
        sg_destroy_image(m_offscreen.pass_desc.resolve_attachments[0].image);
        sg_destroy_image(m_offscreen.pass_desc.depth_stencil_attachment.image);

        m_offscreen.vw = 0;
        m_offscreen.vh = 0;
    }    
}


