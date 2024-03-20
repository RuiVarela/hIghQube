#include "Platform.hpp"

namespace iq {
    
    std::string executablePath() {
        std::string path;
        char buffer[1024];
        uint32_t size = sizeof(buffer);

        int byte_count = readlink("/proc/self/exe", buffer, size);
        if (byte_count != -1) {
            path = std::string(buffer, byte_count);
        }

        return path;
    }
}