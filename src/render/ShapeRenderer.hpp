#pragma once

#include "../data/DataManager.hpp"
#include "../Object.hpp"

namespace iq {
    class Game;


    class ShapeRenderer {
    public:
        ShapeRenderer(std::string const& tag);
        ~ShapeRenderer();

        enum class Pipeline {
            Normal,
            DepthOff,
            Reflection,
            WriteStencil,
            ClipWithStencil,
            Explosion,
            Count
        };

        void selectPipeline(Pipeline pipeline) { m_selected_pipeline = pipeline; }
        void setAlphaMultiplier(float multiplier) { m_alpha_mutiplier = multiplier; }
        
        void setup(Game* game, ObjectKind kind);
        void render(std::vector<Object*> objects, HMM_Mat4 const& view_projection);
    private:
        ShapeRenderer(ShapeRenderer const& non_copyable) = delete;
        ShapeRenderer& operator=(ShapeRenderer const& non_copyable) = delete;

        std::string m_tag;
        Game* m_game;
        ObjectKind m_kind;

        std::vector<float> m_vertices;
        std::vector<uint16_t> m_indices;

        void generateCubeData();
        void generatePyramidData();
        void generateWallData();
        
        std::string m_vertex_buffer_name;
        sg_buffer m_vertex_buffer = {};
       
        std::string m_index_buffer_name;
        sg_buffer m_index_buffer = {};
        
        sg_shader m_shader = {};

        Pipeline m_selected_pipeline;
        std::string m_pipeline_name[int(Pipeline::Count)];
        sg_pipeline m_pipeline[int(Pipeline::Count)];

        sg_bindings m_bindings = {};

        float m_alpha_mutiplier;
    };
}