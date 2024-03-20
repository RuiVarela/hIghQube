#include <Vendor.hpp>

#include "Game.hpp"

extern unsigned char app_icon_128_data[128 * 128 * 4 + 1];

using namespace iq;

static uint64_t start_ticks = 0;
static Inputs inputs = { };
static std::unique_ptr<Game> game;


static void init(void) {
    stm_setup();
    start_ticks = stm_now();

    //
    // sg gfx
    //
    sg_desc desc = { };
    desc.context = sapp_sgcontext();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    //
    // sgl
    //
    sgl_desc_t sgl_desc = {
        .sample_count = sapp_sample_count()
    };
    sgl_desc.logger.func = slog_func;
    sgl_setup(&sgl_desc);

    game = std::make_unique<Game>();
}

static void cleanup(void) {
    game.reset();

    sgl_shutdown();

    sg_shutdown();
}

static void frame(void) {

    inputs.width = sapp_width();
    inputs.height = sapp_height();
    inputs.dpi_scale = sapp_dpi_scale();


    inputs.time_ticks = stm_since(start_ticks);
    double time = stm_sec(inputs.time_ticks);

#if true 
    // use smoothed deltas
    inputs.frame_duration = sapp_frame_duration(); 
#else
    // use precise deltas
    constexpr double min_duration = 1.0 / 500.0;
    constexpr double max_duration = 1.0;
    inputs.frame_duration = time - inputs.time;

    if (inputs.frame_duration < min_duration) inputs.frame_duration = min_duration;
    else if (inputs.frame_duration > max_duration) inputs.frame_duration = max_duration;
#endif
        
    inputs.frame_count = sapp_frame_count();
    inputs.time = time;

    game->update(inputs);
    game->render();

    sg_commit();
    
    // clear transient inputs
    inputs.resize = false;
}

static void input(const sapp_event* event) {
    game->handleInput(event);

    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        switch(event->key_code) {
            case SAPP_KEYCODE_W: inputs.forward = true; break;
            case SAPP_KEYCODE_S: inputs.backward = true;  break;
            case SAPP_KEYCODE_A: inputs.left = true; break;
            case SAPP_KEYCODE_D: inputs.right = true; break;

            case SAPP_KEYCODE_M: inputs.debug_menu = true; break;

            case SAPP_KEYCODE_I: inputs.triangle = true; break;
            case SAPP_KEYCODE_K: inputs.cross = true;  break;
            case SAPP_KEYCODE_J: inputs.square = true; break;
            case SAPP_KEYCODE_L: inputs.cicle = true; break;

            default: break;
        }
    } else if (event->type == SAPP_EVENTTYPE_KEY_UP) {\
        switch(event->key_code) {
            case SAPP_KEYCODE_W: inputs.forward = false; break; 
            case SAPP_KEYCODE_S: inputs.backward = false; break;
            case SAPP_KEYCODE_A: inputs.left = false; break;
            case SAPP_KEYCODE_D: inputs.right = false; break;

            case SAPP_KEYCODE_M: inputs.debug_menu = false; break;

            case SAPP_KEYCODE_I: inputs.triangle = false; break;
            case SAPP_KEYCODE_K: inputs.cross = false;  break;
            case SAPP_KEYCODE_J: inputs.square = false; break;
            case SAPP_KEYCODE_L: inputs.cicle = false; break;
            
            default: break;
        }
    }
    
    else if (event->type == SAPP_EVENTTYPE_RESIZED) {
        inputs.resize = true;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

	sapp_icon_desc icon_desc = { };
	icon_desc.sokol_default = false;
	icon_desc.images[0] = { 128, 128, { app_icon_128_data, (sizeof(app_icon_128_data) - 1)} };

    sapp_desc desc = { };
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;

    desc.width = 640 * 2;
    desc.height = 480 * 2;
    desc.sample_count = 4;
    desc.fullscreen = false;
    desc.high_dpi = false;
    desc.ios_keyboard_resizes_canvas = false;
    
    desc.window_title = "hIghQube";
    desc.icon = icon_desc;
    desc.enable_clipboard = false;
    desc.logger.func = slog_func;
    return desc;
}
