#include "Platform.hpp"

#include <filesystem>
#include <fstream>

namespace iq {
    
    std::string convertFileNameToNativeStyle(std::string filename) {
        std::string::value_type search = '\\';
        std::string::value_type replace = '/';

    #if defined(_WIN32)
        std::swap(search, replace);
    #endif

        std::string::size_type slash = 0;
        while ((slash = filename.find_first_of(search, slash)) != std::string::npos)
            filename[slash] = replace;
        return filename;
    }

    std::string mergePaths(std::string const& a, std::string const& b) {
        if (a.empty())
            return convertFileNameToNativeStyle(b);

        if (b.empty())
            return convertFileNameToNativeStyle(a);

        if (a.ends_with("\\") || b.starts_with("\\") ||
            a.ends_with("/") || b.starts_with("/"))
            return convertFileNameToNativeStyle(a + b);

        return convertFileNameToNativeStyle(a + "/" + b);
    }

    std::string mergePaths(std::string const& a, std::string const& b, std::string const& c) {
        return mergePaths(mergePaths(a, b), c);
    }
    
    std::string mergePaths(std::string const& a, std::string const& b, std::string const& c, std::string const& d) {
        return mergePaths(mergePaths(mergePaths(a, b), c), d);
    }

    std::string assetPath(std::string const& kind, std::string const& name) {
        std::string root = executablePath();

        // on android there is no such thing as an executable path
        if (!root.empty())
            root = std::filesystem::path(executablePath()).parent_path().string();

        std::vector<std::string> attempts {
            mergePaths(root, "../Resources", kind, name), // mac bundle
            mergePaths(root, "../../assets/bundled", kind, name), // windows debug
            mergePaths(root, kind, name),
            //mergePaths(root, "assets", kind, name),
            //mergePaths(root, "../assets", kind, name),
        };

        for (auto const& current : attempts) {
            //std::cout << "Testing : " << current << std::endl;
            if (assetExists(current)) 
                return current;
        }

        return "";
    }

    #ifndef ANDROID
        bool assetExists(std::string const& filename) { 
            return fileExists(filename);
        }

        bool assetReadRawBinary(std::string const& filename, std::vector<uint8_t>& buffer) { 
            return readRawBinary(filename, buffer); 
        }
    #endif


    bool fileExists(std::string const& filename) {
        auto status = std::filesystem::status(filename);
        return std::filesystem::exists(status);
    }

    bool readRawBinary(std::string const& filename, std::vector<uint8_t>& buffer) {
        std::filesystem::path path(filename);
        
        std::ifstream file_stream(path, std::fstream::binary);
        if (!file_stream)
            return false;

        file_stream.seekg(0, file_stream.end);
        auto length = file_stream.tellg();
        file_stream.seekg(0, file_stream.beg);

        buffer.resize(length);
        file_stream.read((char*)buffer.data(), length);

        file_stream.close();
        return true;
    }
}