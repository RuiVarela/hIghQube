#pragma once

#include "ShapeRenderer.hpp"
#include "TextureRenderer.hpp"
#include "TextRenderer.hpp"
#include "CrossFadeRenderer.hpp"

namespace iq {
    class Game;

    class VisualRenderer {
        public:
            VisualRenderer();
            ~VisualRenderer();
            
            void setup(Game* game);
            void render(std::vector<Object*> const& game_overlays, bool render_map);

        private:
            VisualRenderer(VisualRenderer const& non_copyable) = delete;
            VisualRenderer& operator=(VisualRenderer const& non_copyable) = delete;

            Game* m_game;

            ShapeRenderer m_cube_renderer;
            ShapeRenderer m_pyramid_renderer;
            ShapeRenderer m_explosion_renderer;
            TextureRenderer m_texture_renderer;
            TextRenderer m_text_renderer;
            CrossFadeRenderer m_crossfade_renderer;

            sg_pass_action m_render_pass;

            struct Offscreen {
                sg_pass_action pass_action = {};
                sg_pass_desc pass_desc = {};
                sg_pass pass = {};
                int vw = 0;
                int vh = 0;
            };
            Offscreen m_offscreen;

            void buildOffscreen();
            void destroyOffscreen();
    };

}

