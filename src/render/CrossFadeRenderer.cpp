#include "CrossFadeRenderer.hpp"
#include "../Interpolator.hpp"

#include "../Game.hpp"

namespace iq {

    //
    // Cross Fade Renderer
    //
    CrossFadeRenderer::CrossFadeRenderer(std::string const& tag) 
        :m_tag(tag), m_game(nullptr), m_pipeline {}
    {  

    }

    CrossFadeRenderer::~CrossFadeRenderer() { 
        sgl_destroy_pipeline(m_pipeline);
    }

    void CrossFadeRenderer::render(std::vector<Object*> const& objects) {
        if (objects.empty())
            return;

        std::vector<Object*>::const_iterator found = std::find_if(objects.begin(), objects.end(), [](Object* object) {
            return object->kind() == ObjectKind::CrossFade;
        });

        if (found == objects.end())
            return;

        auto current = *found;

        float const x0 = 0.0f;
        float const x1 = sapp_widthf();
        float const y0 = 0.0f;
        float const y1 = sapp_heightf();
        auto const color = current->color();
        float percentage = 1.0f - fabs(current->percentage() * 2.0f - 1.0f);
        percentage = easeOutQuart(percentage);


        sgl_push_pipeline();
        sgl_load_pipeline(m_pipeline);
            sgl_begin_quads();
                sgl_c4f(color.R, color.G, color.B, color.A * percentage);
                sgl_v2f(x0, y0);
                sgl_v2f(x1, y0);
                sgl_v2f(x1, y1);
                sgl_v2f(x0, y1);
            sgl_end();
        sgl_pop_pipeline();
    }

    void CrossFadeRenderer::setup(Game* game) {
        m_game = game;

        sg_pipeline_desc pipeline_desc = { };
        pipeline_desc.colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .src_factor_alpha = SG_BLENDFACTOR_ONE,
            .dst_factor_alpha = SG_BLENDFACTOR_ZERO
        };

        m_pipeline_name = m_tag + "Base"; 
        pipeline_desc.label = m_pipeline_name.c_str();
        m_pipeline = sgl_make_pipeline(&pipeline_desc);

    }
}