#pragma once

#include <Vendor.hpp>
#include "PuzzleData.hpp"

namespace iq {


    struct GameSettings {
        bool debug_camera = false;

        double cross_fade_time = 2.0;

        double wave_appear_time = 1.5;
        double wave_appear_hold_time = 1.0;
        
        float const player_speed = 1.75f;

        float const player_width = 0.15f;
        float const player_height = 0.75f;

        float const camera_fov = 30.0f;

        // Level 0: 47 frames. Pause - 25 frames
        // Level 1: 40 frames. Pause - 22 frames
        // Level 2: 32 frames. Pause - 22 frames
        // Level 3: 27 frames. Pause - 18 frames
        // Level 4: 21 frames. Pause - 16 frames
        // Accelerate / Squashed: 4 frames. Pause - 6 frames

        double const rotating_time = 47.0f / 30.0f;
        double const holding_time = 25.0f / 30.0f;

        HMM_Vec3 player_camera_target { 0.0f, 0.0f, -8.0f };
        HMM_Vec3 player_camera_position { 0.0f, 8.0f, 13.0f };
    };


    class DataManager {
    public:
        DataManager();
        ~DataManager();

        void release();

        GameSettings& settings()  { return m_settings; }

        //
        // Fonts
        //
        struct Font {
            bool valid;
            std::shared_ptr<std::string> name;
            std::shared_ptr<std::vector<uint8_t>> data;
            int id;
        };
        FONScontext* fontstashContext() { return m_fontstash_context; }
        Font font(std::string const& name);

        std::string defaultFont() { return "DroidSerif-Regular.ttf"; }
        
        //
        // Textures
        //
        struct Texture {
            bool valid;
            std::shared_ptr<std::string> name;
            sg_image image;
            sg_sampler sampler;
        };
        const static std::string DefaultTexture;
        Texture texture(std::string const& name);

        std::string groundTexture() { return "square_00.png"; }
        std::string cubeTexture() { return "square_00.png"; }
        std::string playerTexture() { return "square_06.png"; }
        std::string guideTexture() { return "square_16.png"; }
        
        //
        // Stages
        //
        // https://iq.airesoft.co.uk/qube/
        // https://github.com/adeyblue/IQTools
        struct Puzzle {
            std::string key;
            int turns = 0;
            std::vector<uint8_t> cubes;
        };
        struct Wave {
            int columns = 0;
            int rows = 0;
            std::vector<Puzzle> puzzles;
        };
        struct Stages {
            uint32_t seed;
            std::array<std::vector<Wave>, 9> stages;
        };
        Stages stages(uint32_t seed = 0);
        Wave puzzle(std::string const name); // this loads a single puzzle, its convenient for testing

        //
        // color palette
        //
        static const constexpr float groundOverlaysAlpha = 0.65f;
        sg_color withAlpha(sg_color color, float alpha) { color.a = alpha; return color; }

        sg_color const& backgroundColor() { return sg_black; }
        sg_color const& groundColor() { return sg_lavender_blush; }
        
        sg_color shadowColor() { return withAlpha(sg_gold, groundOverlaysAlpha); }

        sg_color const& playerColor() { return sg_hot_pink; }
        
        sg_color normalMarkerColor() { return withAlpha(sg_deep_sky_blue, groundOverlaysAlpha); }
        sg_color advantageMarkerColor() { return withAlpha(sg_aqua, groundOverlaysAlpha); }
        sg_color triggeredMarkerColor() { return withAlpha(sg_coral, groundOverlaysAlpha); }
        
        sg_color const& normalCubeColor() { return sg_snow; }
        sg_color const& advantageCubeColor() { return sg_light_green; }
        sg_color const& forbiddenCubeColor() { return sg_web_gray; }
        sg_color const& whiteColor() { return sg_white; }
        sg_color const& explosionColor() { return sg_seashell; }
        
        std::vector<sg_color> const& colors();
        
    private:
        std::vector<Texture> m_textures;
        void releaseTextures();

        std::vector<Font> m_fonts;
        FONScontext* m_fontstash_context;
        void releaseFontstash();
        void createFontstash();

        GameSettings m_settings;
    };
    
}
