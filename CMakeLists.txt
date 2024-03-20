cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum OS X deployment version")

project(hIghQube)

# version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_BUILD 123)

add_compile_definitions(VERSION_MAJOR=${VERSION_MAJOR})
add_compile_definitions(VERSION_MINOR=${VERSION_MINOR})
add_compile_definitions(VERSION_BUILD=${VERSION_BUILD})

# fly high in the sky
# actually android ndk seems stuck in c++17, so avoid use advanced things in c++20
set(CMAKE_CXX_STANDARD 20)

# Generate compile_commands.json
# vs code uses this
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# tell visual studio to assume all source code is UTF8
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(PROJECT_FILES "")

#
# Dependencies
#
set(PROJECT_FILES ${PROJECT_FILES}
	vendor/Vendor.hpp
	vendor/Vendor.cpp
	
	vendor/imgui/imgui.cpp
	vendor/imgui/imgui.h
	vendor/imgui/imgui_widgets.cpp 
	vendor/imgui/imgui_draw.cpp 
	vendor/imgui/imgui_tables.cpp
	vendor/imgui/imgui_demo.cpp
)

#
# hIghQube
#
set(PROJECT_FILES ${PROJECT_FILES}
	src/main.cpp
    assets/app_icon/icon.cpp

    src/platform/Platform.hpp
    src/platform/Platform.cpp

    src/data/DataManager.hpp
	src/data/DataManager.cpp
    src/data/PuzzleData.hpp
    src/data/PuzzleData.cpp

    src/render/ShapeRenderer.hpp
    src/render/ShapeRenderer.cpp
    src/render/TextureRenderer.hpp
    src/render/TextureRenderer.cpp
    src/render/TextRenderer.hpp
    src/render/TextRenderer.cpp
    src/render/CrossFadeRenderer.hpp
    src/render/CrossFadeRenderer.cpp
    src/render/VisualRenderer.hpp
    src/render/VisualRenderer.cpp
	src/render/VisualRenderer.glsl
    src/render/VisualRenderer.glsl.h
    
	src/ToolsWindow.hpp
	src/ToolsWindow.cpp
    src/Object.hpp
    src/Object.cpp

    src/Camera.hpp
    src/Camera.cpp
    src/Player.hpp
    src/Player.cpp
	src/Map.hpp
	src/Map.cpp
    src/Game.hpp
    src/Game.cpp
    src/Inputs.hpp
    src/Interpolator.hpp
    src/Interpolator.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    #
    # Windows
    #
    set(PROJECT_FILES ${PROJECT_FILES} src/platform/PlatformWindows.cpp)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/windows/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/windows/app.ico ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_FILES} ${CMAKE_CURRENT_BINARY_DIR}/app.rc)

    # pick startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # organize in folders for VS
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_FILES})
    
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin OR CMAKE_SYSTEM_NAME STREQUAL iOS) 
    #
    # Apple
    #
    set(PROJECT_FILES ${PROJECT_FILES} src/platform/PlatformApple.mm)

    set(APP_BUNDLE_IDENTIFIER "pt.demanda.highqube")
    set(MACOSX_BUNDLE_COPYRIGHT "Rui Varela")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
    set(MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_NAME})

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_FILES})
    target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)
    target_link_libraries(${PROJECT_NAME}
        "-framework MetalKit"
        "-framework Metal"
        "-framework AudioToolbox")

    set_target_properties(${PROJECT_NAME} PROPERTIES 
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "MRGJ46V3G9"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    )

    #
    # Copy Resources to bundle
    #
    file(GLOB TEXTURE_FILES "assets/bundled/textures/*")
    file(GLOB FONT_FILES "assets/bundled/fonts/*")
    set(RESOURCE_FILES ${TEXTURE_FILES} ${FONT_FILES})

    foreach (FILE ${RESOURCE_FILES})
        file(RELATIVE_PATH RES_PATH "${CMAKE_SOURCE_DIR}/assets/bundled/" ${FILE})
        get_filename_component(FOLDER_PATH "${FILE}" PATH)
        get_filename_component(FOLDER_NAME "${FOLDER_PATH}" NAME)

        target_sources(${PROJECT_NAME} PRIVATE ${FILE})
        set_source_files_properties(${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${FOLDER_NAME}")

        # Optional: Add the file to the 'Resources' folder group in Xcode.
        #           This also preserves folder structure.
        source_group("Resources/${FOLDER_NAME}" FILES "${FILE}")

        #message(STATUS "Adding resource file: ${FILE} | ${RES_PATH} | ${FOLDER_NAME}")
    endforeach ()

    if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
        #
        # macOS
        #
        set_target_properties(${PROJECT_NAME} PROPERTIES 
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/apple/macos/Info.plist.in"
        )

        # icon
        set(MACOSX_BUNDLE_ICON_FILE app.icns)
        target_sources(${PROJECT_NAME} PRIVATE "assets/apple/macos/${MACOSX_BUNDLE_ICON_FILE}")
        set_source_files_properties("assets/apple/macos/${MACOSX_BUNDLE_ICON_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

        target_link_libraries(${PROJECT_NAME}
            "-framework QuartzCore"
            "-framework Cocoa")

    elseif(CMAKE_SYSTEM_NAME STREQUAL iOS)
        #
        # iOS
        #

        # Asset catalog 
        target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets/apple/ios/Icons.xcassets")
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/assets/apple/ios/Icons.xcassets" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

        set_target_properties(${PROJECT_NAME} PROPERTIES 
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/apple/ios/Info.plist.in"
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
            XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "${VERSION_BUILD}"
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # <== Set to "1" to target iPhone, set to "2" to target iPad, set to "1,2" to target both
        )

        target_link_libraries(${PROJECT_NAME}
            "-framework Foundation"
            "-framework UIKit"
            "-framework AVFAudio")
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL Android)
    #
    # Android
    #

    set(PROJECT_FILES ${PROJECT_FILES} src/platform/PlatformAndroid.cpp)

    add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})

    #if(ANDROID_ABI STREQUAL armeabi-v7a)
    #    set_source_files_properties(foo.cpp PROPERTIES COMPILE_FLAGS -mfpu=neon)
    #endif()

    # add_compile_definitions(ANDROID_ARM_NEON=OFF)
    # set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -mfpu=neon)

    target_link_libraries(${CMAKE_PROJECT_NAME} log android aaudio GLESv3 EGL)

else()

    #
    # No one knows
    #
    set(PROJECT_FILES ${PROJECT_FILES} src/platform/PlatformLinux.cpp)

    add_executable(${PROJECT_NAME} ${PROJECT_FILES})
    
endif()

target_include_directories(${PROJECT_NAME} PUBLIC vendor)