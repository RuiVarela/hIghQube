#pragma once

#include <cstdint>

namespace iq {

    struct Inputs {
        int width;
        int height;

        float dpi_scale;
        uint64_t frame_count;
        double frame_duration;
        uint64_t time_ticks;
        double time;
        
        bool resize;

        bool forward;
        bool backward;
        bool left;
        bool right;

        bool triangle;
        bool cicle;
        bool cross;
        bool square;

        bool debug_menu;
    };

};