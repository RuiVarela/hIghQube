#pragma once

#include "Camera.hpp"
#include "Player.hpp"

namespace iq {

    class Game;
    class DataManager;


    //
    // Wave
    //
    enum class WaveState {
        NotStarted,

        Appearing,

        Rotating,
        Holding,

        Finished,

        Count
    };

    enum class CubeState: int {
        Empty           = 0x00,

        Normal          = 0x01,
        Advantage       = 0x02,
        Forbidden       = 0x03,

        Exploding       = 1 << (8 + 0),
        Falling         = 1 << (8 + 1)
    };
    inline CubeState baseState(CubeState state) { return CubeState(int(state) & 0xFF); }
    inline bool isSet(CubeState a, CubeState b) { return int(a) & int(b); }
    inline CubeState operator|(CubeState a, CubeState b) { return CubeState(int(a) | int(b)); }
    inline CubeState operator&(CubeState a, CubeState b) { return CubeState(int(a) & int(b)); }

    struct CubeData {
        CubeState state;
        double state_start = 0.0;
        Object object;
        Object explosion;
    };

    struct Wave {
        std::string name;
        std::vector<CubeData> cubes;
        int turns = 0;
        int rows = 0;
        int row = 0;

        WaveState state = WaveState::NotStarted;
        double state_start = 0.0;
    };

    //
    // Map
    //
    class Map {
    public:
        Map();

        double strechedTime();

        int columns() const { return m_colums; }
        int rows() const { return m_rows; }
        int height() { return 3; } 
        
        HMM_Vec3 position(int column, int row) const;

        CubeData& cubeData(int column, int row);
        void markCubeState(int column, int row, CubeState state);
        bool hasAnyCube(int column, int row);

        void setup(Game* game);
        
        Game& game() { return *m_game; }
        Camera& camera() { return m_camera; }
        Player& player() { return m_player; }
        Wave& wave() { return m_wave; }
        
        DataManager& dataManager();
        
        void setupStage(std::vector<std::vector<Wave>> const& stage, int columns, int rows);
        bool hasStageData();

        void loadNextPuzzle();
        void loadNextWaveGroup();
        std::string const& puzzeName();
        std::string waveGroupName();
        
        bool isLastPuzzleOfWave();
        bool isLastWaveGroup();
        bool isLastPuzzle();

        void buildWave();
        void setWaveState(WaveState state);

        void addRowToDestroy();
        void addRowToCreate();
    
        
        void update();

        // 
        // Object Queries
        //
        void groundObjects(std::vector<Object*>& objects);
        void groundRectObjects(std::vector<Object*>& objects);

        void waveObjects(std::vector<Object*>& objects);
        void waveObjectsForReflection(std::vector<Object*>& objects);

        void explodingObjects(std::vector<Object*>& objects);

        void textObjects(std::vector<Object*>& objects);
    private:
        Game* m_game;
        Camera m_camera;
        Player m_player;

        int m_colums;
        int m_rows;
        
        //
        // Ground
        //
        Object m_ground;
        Object m_ground_rect;
        bool m_ground_needs_update;
        int m_creating_row;
        int m_destroying_row;
        float m_creating_destroying_start;
        bool m_creating_destroying_started;
        Object m_creating_destroying_row;

        void initGround();
        void updateGround();

        //
        // Active wave
        //
        std::vector<std::vector<Wave>> m_stage_data;
        int m_wave_index;
        int m_puzzle_index;
        Wave m_wave;
        void updateWave();
        void updateWaveFinished();

        //
        // Text
        //
        std::vector<Object> m_text;
        void updateText();
        void addOutlinedText(Object o, sg_color color);

        //
        // Scoring
        //
        int m_capture_cubes;
        int m_capture_forbidden;  

        int m_lost_cubes;
        int m_lost_forbidden;
    };
}
