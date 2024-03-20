// sokol implementation library on non-Apple platforms
#define SOKOL_IMPL


#if defined(_WIN32)
    #define SOKOL_D3D11
    //#define SOKOL_GLCORE33
#elif defined(__EMSCRIPTEN__)
    #define SOKOL_GLES2
#elif defined(__APPLE__)
    // NOTE: on macOS, sokol.c is compiled explicitely as ObjC
    #define SOKOL_METAL
#elif defined(ANDROID)
    #define SOKOL_GLES3
#else
    #define SOKOL_GLCORE33
#endif


#ifndef NDEBUG
    #define SOKOL_TRACE_HOOKS
#endif // NDEBUG


#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_audio.h"
#include "sokol/sokol_log.h"
#include "sokol/sokol_time.h"
#include "sokol/util/sokol_gl.h"

#include "imgui/imgui.h"
#include "sokol/util/sokol_imgui.h"

#include "sokol/util/sokol_gfx_imgui.h"

#define FONTSTASH_IMPLEMENTATION
#include "fontstash/src/fontstash.h"
#include "sokol/util/sokol_fontstash.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize2.h"
