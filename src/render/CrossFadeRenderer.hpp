#pragma once

#include "../data/DataManager.hpp"
#include "../Object.hpp"

namespace iq {
    class Game;


    class CrossFadeRenderer {
    public:
        CrossFadeRenderer(std::string const& tag);
        ~CrossFadeRenderer();

        void setup(Game* game);
        void render(std::vector<Object*> const& objects);

    private:
        CrossFadeRenderer(CrossFadeRenderer const& non_copyable) = delete;
        CrossFadeRenderer& operator=(CrossFadeRenderer const& non_copyable) = delete;

        std::string m_tag;
        Game* m_game;

        std::string m_pipeline_name;
        sgl_pipeline m_pipeline;
    }; 

}