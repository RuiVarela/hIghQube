#include "Game.hpp"

#include <iostream>

namespace iq {

    Game::Game() {
        m_visual_renderer.setup(this);
        m_map.setup(this);

        m_state = GameState::None;
        m_allow_update_map = false;
        m_allow_render_map = false;
        m_state_start = 0.0f;

        m_cross_fade = Object(ObjectKind::CrossFade);
        m_title = Object(ObjectKind::Text);
        
        log("Game", "Game created");

        preloadData();
    }

    Game::~Game() {
        log("Game", "Game destroy");
        
    }

    void Game::preloadData() {
        log("Game", "Preloading data ...");
        m_data_manager.font(m_data_manager.defaultFont());

        m_data_manager.texture("default");
        m_data_manager.texture(m_data_manager.groundTexture());
        m_data_manager.texture(m_data_manager.playerTexture());
        m_data_manager.texture(m_data_manager.guideTexture());

        log("Game", "Preloaded data!");
    }

    void Game::log(std::string const& tag, std::string const& message) {
        std::string full_message = tag + ": " + message;
        std::cout << full_message << std::endl;

#ifndef NDEBUG
        m_tools_window.addLog(full_message);
#endif // NDEBUG
    }
        
    void Game::update(Inputs const& inputs) {
        m_inputs = inputs;


        m_overlays.clear();

        if (m_state == GameState::None) {
            prepareGame(0);
        } else if (m_state == GameState::Starting || m_state == GameState::LoadingNextWaveGroup) {
            double const delta = inputs.time - m_state_start;
            float const percentage = HMM_Clamp(0.0f, float(delta / settings().cross_fade_time), 1.0f);

            m_cross_fade.setColor(sg_black);
            m_cross_fade.setPercentage(percentage);
        
            m_overlays.push_back(&m_cross_fade);

            if (percentage > 0.5f) {
                if (m_state == GameState::LoadingNextWaveGroup && !m_loaded_next_wave_group) {
                    m_loaded_next_wave_group = true;
                    map().loadNextWaveGroup();
                } else if (m_active_stage == -1) {
                    generateStages(m_next_seed);
                    loadStage(0);
                }

                // float size = 25.0f * game().inputs().dpi_scale;
                m_title.setFontName(dataManager().defaultFont());
                m_title.setColor(sg_white);
                m_title.setFontAlign(FONS_ALIGN_CENTER| FONS_ALIGN_CENTER);
                m_title.setPosition(inputs.width / 2.0f, inputs.height / 2.0f);
                m_title.setScale(inputs.height * 0.15f);
                m_title.setText(std::to_string(m_active_stage + 1) + "." + map().waveGroupName());
                m_overlays.push_back(&m_title);
            }

            m_allow_update_map = percentage > 0.5f;
            m_allow_render_map = map().hasStageData();

            if (percentage >= 1.0f) {
                m_state = GameState::Started;
                m_state_start = inputs.time;
            }
        } else if (m_state == GameState::Started) {
            m_allow_update_map = true;
            m_allow_render_map = true;
        }

        
        m_speeder.update(inputs.time, inputs.square ? 6.5 : 1.0);

        if (m_allow_update_map)
            m_map.update();

        if (m_map.wave().state == WaveState::Finished && 
            m_state != GameState::LoadingNextWaveGroup) {
            nextPuzzle();
        }
    }

    void Game::handleInput(const sapp_event* event) {
#ifndef NDEBUG
        m_tools_window.handleInput(event);
#endif // NDEBUG
    }
            
    void Game::render() {

        m_visual_renderer.render(m_overlays, m_allow_render_map);

#ifndef NDEBUG
        if (m_inputs.debug_menu) 
            m_tools_window.show();

        if (m_tools_window.showing())
            m_tools_window.render(*this);
#endif // NDEBUG

    }

    void Game::prepareGame(uint32_t seed) {
        m_state = GameState::Starting;
        m_state_start = inputs().time;
        m_active_stage = -1;
        m_next_seed = seed;
    }

    void Game::generateStages(uint32_t seed) {
        if (seed == 0) {
            Random r;
            seed = r.nextInt(1, 2147480000);
        }

        log("Game", "Generating stages with seed: " + std::to_string(seed));
        m_stages = m_data_manager.stages(seed);

        for (int s = 0; s != m_stages.stages.size(); ++s) {
            auto stage = m_stages.stages[s];
            log("Game", "Stage " + std::to_string(s + 1));

            for (int w = 0; w != stage.size(); ++w) {
                auto wave = stage[w];
                std::stringstream ss;
                for (auto const& puzzle : wave.puzzles) 
                    ss << puzzle.key << " ";
            
                log("Game", "   Wave " + std::to_string(w + 1) + " : " + ss.str());
            }
        }
    }

    void Game::loadStage(int stage_number) {
        log("Game", "loading stage " + std::to_string(stage_number));

        assert(stage_number >= 0);
        assert(stage_number < m_stages.stages.size());

        m_active_stage = stage_number;
        std::vector<DataManager::Wave> const& stage = m_stages.stages[stage_number];
        std::vector<std::vector<Wave>> map_stage;

        int columns = 0;

        int max_rows = 0;
        int max_turns = 0;

        for (DataManager::Wave const& data_wave : stage) {
            columns = data_wave.columns;

            std::vector<Wave> stage_puzzles;
            for (auto const& current: data_wave.puzzles) {
                Wave puzzle;
                puzzle.name = current.key;
                puzzle.rows = data_wave.rows;
                puzzle.turns = current.turns;
                for (auto const& c: current.cubes) {
                    CubeData cube_data;

                    if (c == '1')
                        cube_data.state = CubeState::Advantage;
                    else if (c == '2')
                        cube_data.state = CubeState::Forbidden;
                    else if (c == '0')
                        cube_data.state = CubeState::Normal;

                    puzzle.cubes.push_back(cube_data);
                }

                if (puzzle.turns > max_turns) 
                    max_turns = puzzle.turns;
                
                if (puzzle.rows > max_rows)
                    max_rows = puzzle.rows;
                
                stage_puzzles.push_back(puzzle);
            }

            map_stage.push_back(stage_puzzles);
        }

        int const slack = 3; // TODO: revisit this if it is too easy
        int const stage_rows = max_turns + max_rows + slack;

        //log("Game", "stage max turns: " + std::to_string(max_turns) + " max rows: " + std::to_string(max_rows));

        map().setupStage(map_stage, columns, stage_rows);
    }

    void Game::nextPuzzle() {
        if (map().isLastPuzzle()) {
            if (m_active_stage == (m_stages.stages.size() - 1)) {
                log("Game", "game finished");
            } else {
                loadStage(m_active_stage + 1);
            }
        } else if (map().isLastPuzzleOfWave()) {

            m_loaded_next_wave_group = false;
            m_state = GameState::LoadingNextWaveGroup;
            m_state_start = inputs().time;

        } else {
            map().loadNextPuzzle();
        }
    }


    void Game::testPuzzle(std::string const& name) {
        log("Game", "testPuzzle " + name);

        auto wave = m_data_manager.puzzle(name);
        if (wave.puzzles.empty())
            return;

        std::vector<DataManager::Wave> stage;
        stage.push_back(wave);

        m_stages.stages[0] = stage;
        loadStage(0);
    }
}