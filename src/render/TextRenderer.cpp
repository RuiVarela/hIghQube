#include "TextRenderer.hpp"

#include "../Game.hpp"

#include "../vendor/fontstash/src/fontstash.h"
#include "../vendor/sokol/util/sokol_fontstash.h"

namespace iq {


    //
    // Text Renderer
    //
    TextRenderer::TextRenderer(std::string const& tag) 
        :m_tag(tag), m_game(nullptr)
    {  }

    TextRenderer::~TextRenderer() { }

    void TextRenderer::preRender() {
        DataManager& dm = m_game->dataManager();
        FONScontext* ctx = dm.fontstashContext();

        fonsClearState(ctx);
    }

    void TextRenderer::postRender() {
        DataManager& dm = m_game->dataManager();
        FONScontext* ctx = dm.fontstashContext();

        // flush fontstash's font atlas to sokol-gfx texture
        sfons_flush(ctx);
    }

    void TextRenderer::render(std::vector<Object*> objects) {
        if (objects.empty())
            return;

        DataManager& dm = m_game->dataManager();
        FONScontext* ctx = dm.fontstashContext();
        for (auto const& o : objects) {
            DataManager::Font const& font = dm.font(o->fontName());

            fonsSetFont(ctx, font.id);
            fonsSetSize(ctx, o->scale().X);

            uint32_t color = sfons_rgba(
                uint8_t(o->color().R * 255.0f), 
                uint8_t(o->color().G * 255.0f), 
                uint8_t(o->color().B * 255.0f), 
                uint8_t(o->color().A * 255.0f)
            );
            fonsSetColor(ctx, color);
            fonsSetAlign(ctx, o->fontAlign());
            if (o->scale().Y >= 1.0f) 
                fonsSetBlur(ctx, o->scale().Y - 1.0f);
            
            float line_height = 0.0f;
            fonsVertMetrics(ctx, NULL, NULL, &line_height);

            char const* start = o->text().c_str();
            char const* end = start;
            int x = o->position().X;
            int y = o->position().Y;

            while (*end) {
                if (*end == '\n' || *end == '\r') {
                    fonsDrawText(ctx, x, y, start, end);
                    y += line_height;
                    end++;
                    start = end;
                } else {
                    end++;
                }
            }

            if (start != end && start) {
                fonsDrawText(ctx, x, y, start, end);
            }
        }
    }

    void TextRenderer::setup(Game* game) {
        m_game = game;
    }

}
