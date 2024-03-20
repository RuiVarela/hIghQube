#include "Map.hpp"
#include "Game.hpp"

#include <sstream>
#include <iomanip>

namespace iq {

    Map::Map() 
        :m_game(nullptr),
        m_ground(ObjectKind::TexturedCube), m_ground_rect(ObjectKind::SolidColorCube),
        m_colums(8), m_rows(8),
        m_creating_row(0), m_destroying_row(0), m_creating_destroying_start(0.0f), m_creating_destroying_started(false),
        m_wave_index(0), m_puzzle_index(0),
        m_ground_needs_update(true),
        m_capture_cubes(0), m_capture_forbidden(0), 
        m_lost_cubes(0), m_lost_forbidden(0)
    {
    }

    double Map::strechedTime() {
        return m_game->speeder().time();
    }

    DataManager& Map::dataManager() {
        return game().dataManager();
    }

    void Map::setup(Game* game) {
        m_game = game;

        m_capture_cubes = 0;
        m_capture_forbidden = 0;

        m_lost_cubes = 0; 
        m_lost_forbidden = 0;

        initGround();

        player().init(this);

        camera().setRelectionPlane(HMM_V3(0.0f, 0.0f, 0.0f), HMM_V3(0.0f, 1.0f, 0.0f));
        camera().setFov(game->settings().camera_fov);
        camera().followPlayer(*game, true);
    }

    void Map::update() {
        updateGround();
        updateWave();
        updateText();

        player().update();

        camera().update(game());
    }

    void Map::groundObjects(std::vector<Object*>& objects) {
        objects.push_back(&m_ground);

        if (m_creating_row > 0 || m_destroying_row > 0) 
            objects.push_back(&m_creating_destroying_row);
    }

    void Map::groundRectObjects(std::vector<Object*>& objects) {
        objects.push_back(&m_ground_rect);
    }

    void Map::waveObjects(std::vector<Object*>& objects) {
        for (size_t i = 0; i != m_wave.cubes.size(); ++i)     
            if (baseState(m_wave.cubes[i].state) != CubeState::Empty)
                objects.push_back(&m_wave.cubes[i].object);
    }

    void Map::waveObjectsForReflection(std::vector<Object*>& objects) {
        for (size_t i = 0; i != m_wave.cubes.size(); ++i) 
            if (m_wave.cubes[i].state != CubeState::Empty && !isSet(m_wave.cubes[i].state, CubeState::Falling))
                objects.push_back(&m_wave.cubes[i].object);
    }

    void Map::explodingObjects(std::vector<Object*>& objects) {
        for (size_t i = 0; i != m_wave.cubes.size(); ++i) 
            if (m_wave.cubes[i].state != CubeState::Empty && isSet(m_wave.cubes[i].state, CubeState::Exploding))
                objects.push_back(&m_wave.cubes[i].explosion);
    }

    void Map::textObjects(std::vector<Object*>& objects) {
        for (auto& current : m_text) 
            objects.push_back(&current);
    }

    HMM_Vec3 Map::position(int column, int row) const {
        return HMM_Vec3{ column + 0.5f, 0.5f, row + 0.5f };
    }
    
    CubeData& Map::cubeData(int column, int row) {
        //int wave_row = m_wave.state == WaveState::Rotating ? m_wave.row + 1 : m_wave.row;
        int wave_row = m_wave.row;

        assert(column >= 0 && column < columns());
        assert(row >= wave_row);
        assert(row < (wave_row + m_wave.rows));

        int r = row - wave_row;
        return m_wave.cubes[r * columns() + column];
    }

    void Map::markCubeState(int column, int row, CubeState state) {
        CubeData& cube = cubeData(column, row);
        assert(cube.state != CubeState::Empty);

        cube.state = cube.state | state;
        cube.state_start = strechedTime();

        if (state == CubeState::Exploding) {
            CubeState base_state = baseState(cube.state);
            if (base_state == CubeState::Forbidden) {
                m_capture_forbidden++;
            } else  {
                m_capture_cubes++;
            }
        }
    }

    bool Map::hasAnyCube(int column, int row) {
        if (column < 0 || column >= columns() || 
            row < m_wave.row || row >= (m_wave.row + m_wave.rows)) {
            return false;
        }

        CubeData& cube = cubeData(column, row);
        CubeState state = baseState(cube.state);
        return (state == CubeState::Normal) || (state == CubeState::Advantage) || (state == CubeState::Forbidden); 
    }

    void Map::initGround() {
        m_ground.setColor(m_game->dataManager().groundColor());
        m_ground.setTexture(m_game->dataManager().groundTexture());

        m_creating_destroying_row = m_ground;
        m_creating_destroying_row.setPosition(columns() * 0.5f, -height() * 0.5f, rows() * 0.5f);

        // ground rect, for stencil testing
        m_ground_rect.setColor(HMM_V4(0.0f, 0.0f, 1.0f, 1.0f));

        m_ground_needs_update = true;
    }

    void Map::updateGround() {

        double time = game().inputs().time;
        
        {
            double state_time = (time - m_creating_destroying_start);
            
            // create row
            if (m_creating_row > 0) {
                constexpr double total_time =  0.5f;
                float percentage = HMM_Clamp(0.0f, float(state_time / total_time), 1.0f);

                float r = rows();
                float z = HMM_Lerp(r + 0.5f, 1.0f - percentage, r + 5.0f);
                m_creating_destroying_row.setPosition(columns() * 0.5f, -height() * 0.5f, z);

                if (percentage >= 1.0f) {
                    --m_creating_row;
                    m_creating_destroying_start = time;
                    m_rows++;
                    m_ground_needs_update = true;
                }
            } 

            // destroy row
            if (m_destroying_row > 0) {
                constexpr double total_time = 0.75f;
                float percentage = HMM_Clamp(0.0f, float(state_time / total_time), 1.0f);

                if (!m_creating_destroying_started) {
                    m_rows--;
                    m_creating_destroying_started = true;
                }

                float y = (-height() * 0.5f) - HMM_Lerp(0.0f, percentage, height() + 2.0f);

                m_creating_destroying_row.setPosition(columns() * 0.5f, y, rows() + 0.5f);

                if (percentage >= 1.0f) {
                    --m_destroying_row;
                    m_creating_destroying_start = time;
                    m_creating_destroying_started = false;
                }
            } 
        }

        // update ground geometry
        if (m_ground_needs_update) {
            float const h = height();

            m_ground.setScale(float(columns()), h, float(rows()));
            m_ground.setTextureScale(m_ground.scale());

            // make the first cube of the ground start at 0
            // and increase rows on the z positiv axis
            m_ground.setPosition(columns() * 0.5f, -h * 0.5f, rows() * 0.5f);

            // creating destroying
            m_creating_destroying_row.setScale(float(columns()), h, 1.0f);
            m_creating_destroying_row.setTextureScale(m_creating_destroying_row.scale());

            // ground rect, for stencil testing
            m_ground_rect.setPosition(m_ground.position().X, 0.0f, m_ground.position().Z);
            m_ground_rect.setScale(m_ground.scale().X, 0.0001f, m_ground.scale().Z);
        }
    }

    void Map::setWaveState(WaveState state) {
        m_wave.state = state;
        m_wave.state_start = strechedTime();
    }

    void Map::addRowToDestroy() {
        m_creating_row = 0;
        if (m_destroying_row <= 0) {
            m_destroying_row = 0;
            m_creating_destroying_start = game().inputs().time;
            m_creating_destroying_started = false;
        }
        m_destroying_row++;
    }

    void Map::addRowToCreate() {
        m_destroying_row = 0;
        if (m_creating_row <= 0) {
            m_creating_row = 0;
            m_creating_destroying_start = game().inputs().time;
            m_creating_destroying_started = false;
        }
        m_creating_row++;
    }

    void Map::setupStage(std::vector<std::vector<Wave>> const& stage, int columns, int rows) {
        m_stage_data = stage;

        m_colums = columns;
        m_rows = rows;
        m_ground_needs_update = true;
        
        m_wave_index = 0;
        m_puzzle_index = 0;
        buildWave();

        player().init(this);
        camera().followPlayer(game(), true);
    }

    bool Map::hasStageData() {
        return !m_stage_data.empty();
    }

    std::string const& Map::puzzeName() {
        return m_stage_data[m_wave_index][m_puzzle_index].name;
    }

    std::string Map::waveGroupName() {
        return std::to_string(m_wave_index + 1);
    }

    
    void Map::buildWave() {
        Wave wave = m_stage_data[m_wave_index][m_puzzle_index];

        wave.state = WaveState::Appearing;
        wave.state_start = strechedTime();
        wave.row = 0;

        game().log("Map", "buildWave WaveIndex: " + std::to_string(m_wave_index) + " PuzzleIndex: " + std::to_string(m_puzzle_index) + " Name: " + wave.name);

        for (auto & cube: wave.cubes) {
            cube.object = Object(ObjectKind::TexturedCube);

            cube.explosion = Object(ObjectKind::FadeWall);
            cube.explosion.setColor(dataManager().explosionColor());

            cube.object.setTexture(m_game->dataManager().cubeTexture());
            if (cube.state == CubeState::Normal) {
                cube.object.setColor(m_game->dataManager().normalCubeColor());
            } else if (cube.state == CubeState::Advantage) {
                cube.object.setColor(m_game->dataManager().advantageCubeColor());
            } else if (cube.state == CubeState::Forbidden) {
                cube.object.setColor(m_game->dataManager().forbiddenCubeColor());
            }
        }

        m_wave = wave;
    }

    void Map::loadNextPuzzle() {
        assert(!isLastPuzzleOfWave() && "This is the last puzzle of the wave, this should not happen!");
        m_puzzle_index += 1;
        buildWave();
    }

    void Map::loadNextWaveGroup() {
        assert(!isLastWaveGroup() && "stage finished, this should not happen!");
        m_wave_index += 1;
        m_puzzle_index = 0;
        buildWave();
    }

    bool Map::isLastPuzzleOfWave() {
        int wave_puzzles = m_stage_data[m_wave_index].size();
        return (m_puzzle_index >= (wave_puzzles - 1));
    }

    bool Map::isLastWaveGroup() {
        return m_wave_index >= (m_stage_data.size() - 1);
    }

    bool Map::isLastPuzzle() {
        return isLastWaveGroup() && isLastPuzzleOfWave();
    }

    void Map::updateWave() {
        double time = strechedTime();

        float position = m_wave.row;
        float rotation_percentage = 0.0f;
        double state_time = (time - m_wave.state_start);
        float alpha = 1.0f;

        //
        // update the wave
        //
        if (m_wave.state == WaveState::Rotating) {
            double duration = game().settings().rotating_time;
            float percentage = HMM_Clamp(0.0f, float(state_time / duration), 1.0f);

            rotation_percentage = percentage;

            if (percentage >= 1.0f) {
                m_wave.row += 1;
                setWaveState(WaveState::Holding);
            }
        } else if (m_wave.state == WaveState::Holding) {
            double duration = game().settings().holding_time;
            float percentage = HMM_Clamp(0.0f, float(state_time / duration), 1.0f);

            bool can_update = true;

            // don't change state if some cube is expliding
            for (auto const& cube : m_wave.cubes) {
                if (isSet(cube.state, CubeState::Exploding) || player().anySelectedState(SelectionState::DisapearingExploding)) 
                    can_update = false;
            }

            if (can_update && percentage >= 1.0f) {
                setWaveState(WaveState::Rotating);
            }
        } else if (m_wave.state == WaveState::Appearing) {
            double duration = game().settings().wave_appear_time;
            float percentage = HMM_Clamp(0.0f, float(state_time / duration), 1.0f);

            alpha = percentage;

            if (state_time > (game().settings().wave_appear_time + game().settings().wave_appear_hold_time)) {
                setWaveState(WaveState::Rotating);
            }
        }

        //
        // check if the wave is done
        //
        bool all_empty = true;
        for (int r = 0; r != m_wave.rows; ++r) {
            for (int c = 0; c != columns(); ++c) {
                int cube_index = r * columns() + c;
                CubeData& data = m_wave.cubes[cube_index]; 
                
                all_empty &= (data.state == CubeState::Empty);
            }
        }

        if (all_empty && (m_wave.state != WaveState::Finished))
            setWaveState(WaveState::Finished);
        



        //
        // update the individual cubes
        //
        for (int r = 0; r != m_wave.rows; ++r) {
            for (int c = 0; c != columns(); ++c) {
                int cube_index = r * columns() + c;
                CubeData& data = m_wave.cubes[cube_index]; 

                if (data.state == CubeState::Empty) continue;

                int row = r + position;
                float rotation = rotation_percentage;
                
                float x = c + 0.5f;
                float y = 0.5f;
                float z = data.object.position().Z;
                float s = 1.0f; // scale scale

                float explosion_size = 0.0f;

                if (isSet(data.state, CubeState::Exploding)) {
                    constexpr double state_duration = 1.0f;
                    double const state_time = (time - data.state_start);
                    float const percentage = HMM_Clamp(0.0f, float(state_time / state_duration), 1.0f);

                    y = 0.5f - percentage * 0.5; 
                    s = 1.0f - std::pow(percentage, 5.0f) * 0.98f;

                    if (percentage >= 0.5) 
                        explosion_size = (1.0f - percentage) / 0.5f;
                    else  
                        explosion_size = percentage / 0.5f;
                    

                    if (percentage >= 1.0f) {
                        if (baseState(data.state) == CubeState::Advantage)
                            player().markAdvantage(c, row);

                        data.state = CubeState::Empty;
                        setWaveState(WaveState::Holding);
                    }

                    z = row + 0.5f;
                } else if (row >= rows() || isSet(data.state, CubeState::Falling)) {

                    if (!isSet(data.state, CubeState::Falling)) {
                        data.state = data.state | CubeState::Falling;
                        data.state_start = time;
                        z = rows() + 0.5f;

                        if (baseState(data.state) == CubeState::Forbidden) 
                            m_lost_forbidden++;  
                        else 
                            m_lost_cubes++;    
                    }

                    rotation = 0.0f;
                    y = 0.5f - std::pow(time - data.state_start, 4.0f);

                    if (y < -15.0)
                        data.state = CubeState::Empty;
                } else {
                    z = row + 0.5f;
                }

                //
                // General object transformations
                //
                data.object.
                    setScale(s * (1.0f + r * 0.0001f), s * 1.0f, s * (1.0f - 0.0001f)). // prevent z fighting
                    setPosition(x, y, z).
                    setAlphaMultiplier(alpha).
                    setPercentage(rotation);

                //
                // Explosion
                //
                constexpr float const explosion_scale_offset = 0.001f;
                constexpr float const explosion_scale = 2.5f;
                data.explosion.
                    setScale(1.0f - explosion_scale_offset, explosion_size * explosion_scale, 1.0f - explosion_scale_offset).
                    setPosition(data.object.position().X, (explosion_size * explosion_scale) / 2.0f, data.object.position().Z);
            }
        }
    }


    static std::string formatScore(int count) {
        std::ostringstream ss;
        ss << std::setw(3) << std::setfill('0') << count;
        return ss.str();
    }
    void Map::updateText() {
        Inputs const& inputs = game().inputs();

        m_text.clear();

        //float size = 25.0f * game().inputs().dpi_scale;
        float size = game().inputs().height * 0.04f;
        std::string message = "Cap=" + formatScore(m_capture_cubes) + "/" + formatScore(m_capture_forbidden) + " " + 
                              "Lst=" + formatScore(m_lost_cubes) + "/" + formatScore(m_lost_forbidden);

        Object o(ObjectKind::Text);
        o.setFontName(game().dataManager().defaultFont());
        o.setColor(sg_white);
        o.setFontAlign(FONS_ALIGN_RIGHT| FONS_ALIGN_BOTTOM);

        o.setPosition(inputs.width - size / 2.0f, size * 1.25f);
        o.setScale(size);
        o.setText(message);



        addOutlinedText(o, sg_black);
    }

    void Map::addOutlinedText(Object o, sg_color color) {
        float sx = o.scale().X * 0.04f;
        float sy = sx;
        float blur = sx * 2.0f;

        static std::vector<std::pair<float, float>> points = {
            { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f },
            { -1.0f,  0.0f },                  { 1.0f,  0.0f },
            { -1.0f,  1.0f }, { 0.0f,  1.0f }, { 1.0f,  1.0f },
        };

        for (auto const& [dx, dy] : points) {
            Object copy = o;
            copy.position().X += sx * dx;
            copy.position().Y += sy * dy;
            copy.setColor(color);
            copy.setScale(o.scale().X, blur);
            m_text.push_back(copy);
        }

        m_text.push_back(o);
    }
}
