#pragma once

#include "render/VisualRenderer.hpp"
#include "Map.hpp"
#include "ToolsWindow.hpp"

namespace iq {

    enum class GameState {
        None,

        Starting,
        LoadingNextWaveGroup,
        Started,
        
        Count
    };

    class Game {
    public:
        Game();
        ~Game();

        DataManager& dataManager() { return m_data_manager; }
        Map& map() { return m_map; }
        DataManager::Stages const& stages() {return m_stages; }
        
        int viewWidth() const { return m_inputs.width; }
        int viewHeight() const { return m_inputs.height; }

        Inputs const& inputs() const { return m_inputs; }
        Speeder const& speeder() const { return m_speeder; }
        GameSettings& settings()  { return m_data_manager.settings(); }

        void update(Inputs const& inputs);
        void handleInput(const sapp_event* event);
        void render();

        void log(std::string const& tag, std::string const& message);

        void testPuzzle(std::string const& name);
        void nextPuzzle();
        void prepareGame(uint32_t seed);
        void generateStages(uint32_t seed);
        void loadStage(int stage_number);

        void preloadData();
    private:
        Game(Game const& non_copyable) = delete;
        Game& operator=(Game const& non_copyable) = delete;
        
#ifndef NDEBUG
        ToolsWindow m_tools_window;
#endif // NDEBUG  

        DataManager m_data_manager;
        VisualRenderer m_visual_renderer;
        
        DataManager::Stages m_stages;
        int m_active_stage;
        bool m_loaded_next_wave_group;
        Map m_map;

        std::vector<Object*> m_overlays;

        GameState m_state;
        uint32_t m_next_seed;
        bool m_allow_update_map;
        bool m_allow_render_map;
        double m_state_start;
        
        Object m_cross_fade;
        Object m_title;

        Inputs m_inputs;
        Speeder m_speeder;
    };
}
