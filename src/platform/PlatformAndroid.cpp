#include "Platform.hpp"

#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>

// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

namespace iq {
    std::string executablePath() { 
        return ""; 
    }

    // android needs to load data from assets
    bool assetExists(std::string const& filename) {
        ANativeActivity* activity = (ANativeActivity*)sapp_android_get_native_activity();
        AAssetManager* manager = activity->assetManager;

        // load asset frm activity
        AAsset* asset = AAssetManager_open(manager, filename.c_str(), AASSET_MODE_BUFFER);

        bool exists = asset != nullptr;

        if (asset)
            AAsset_close(asset);

        return exists;
    }

    bool assetReadRawBinary(std::string const& filename, std::vector<uint8_t>& buffer) {
        ANativeActivity* activity = (ANativeActivity*)sapp_android_get_native_activity();
        AAssetManager* manager = activity->assetManager;

        AAsset* asset = AAssetManager_open(manager, filename.c_str(), AASSET_MODE_BUFFER);
        if (asset == nullptr)
            return false;

        off_t size = AAsset_getLength(asset);
        int actual = size + 1;

        if (size > 0) {
            buffer.resize(size);
            actual = AAsset_read(asset, buffer.data(), size);
        }

        AAsset_close(asset);
        return (actual == size);
    }
}