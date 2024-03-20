#include "Platform.hpp"

#include <mach-o/dyld.h>

namespace iq {

    std::string executablePath() {
        std::string path;
        char buffer[1024];
        uint32_t size = sizeof(buffer);

        if (_NSGetExecutablePath(buffer, &size) == 0) {
            path = buffer;
        }

        return path;
    }
}