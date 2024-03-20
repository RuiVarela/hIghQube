#pragma once

#include "../data/DataManager.hpp"
#include "../Object.hpp"

namespace iq {
    class Game;

    class TextRenderer {
    public:
        TextRenderer(std::string const& tag);
        ~TextRenderer();

        void setup(Game* game);
        void preRender();
        void postRender();
        void render(std::vector<Object*> objects);

    private:
        TextRenderer(TextRenderer const& non_copyable) = delete;
        TextRenderer& operator=(TextRenderer const& non_copyable) = delete;

        std::string m_tag;
        Game* m_game;
    };
}