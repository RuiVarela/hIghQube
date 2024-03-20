#include "Platform.hpp"

#include <windows.h>

namespace iq {

    std::string executablePath() {
        std::string path;
        char buffer[1024];
        uint32_t size = sizeof(buffer);

        GetModuleFileName(nullptr, buffer, size);
        path = buffer;

        return path;
    }
    
}