#pragma once

#include <Vendor.hpp>

namespace iq {
    
    std::string executablePath();

    std::string assetPath(std::string const& kind, std::string const& name);
    bool assetExists(std::string const& filename);
    bool assetReadRawBinary(std::string const& filename, std::vector<uint8_t>& buffer);

    std::string convertFileNameToNativeStyle(std::string filename);
    std::string mergePaths(std::string const& a, std::string const& b);
    std::string mergePaths(std::string const& a, std::string const& b, std::string const& c);
    std::string mergePaths(std::string const& a, std::string const& b, std::string const& c, std::string const& d);
    
    bool fileExists(std::string const& filename);
    bool readRawBinary(std::string const& filename, std::vector<uint8_t>& buffer);
}