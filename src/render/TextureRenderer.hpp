#pragma once

#include "../data/DataManager.hpp"
#include "../Object.hpp"

namespace iq {
    class Game;

    class TextureRenderer {
    public:
        TextureRenderer(std::string const& tag);
        ~TextureRenderer();

        void setup(Game* game);
        void setImage(sg_image image, HMM_Vec4 tint);
        void render();

        enum class Pipeline {
            Normal,
            ClipWithStencil,
            Count
        };

        void selectPipeline(Pipeline pipeline) { m_selected_pipeline = pipeline; }
    private:
        TextureRenderer(TextureRenderer const& non_copyable) = delete;
        TextureRenderer& operator=(TextureRenderer const& non_copyable) = delete;

        std::string m_tag;
        Game* m_game;

        std::vector<float> m_vertices;
        
        std::string m_vertex_buffer_name;
        sg_buffer m_vertex_buffer = {};
    
        Pipeline m_selected_pipeline;
        std::string m_pipeline_name[int(Pipeline::Count)];
        sg_pipeline m_pipeline[int(Pipeline::Count)];
        sg_shader m_shader = {};
        sg_bindings m_bindings = {};

        std::string m_sampler_name;
        sg_sampler m_sampler = {};

        HMM_Vec4 m_tint = {};
    };

}