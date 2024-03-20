#include "DataManager.hpp"
#include "../Interpolator.hpp"

#include "../platform/Platform.hpp"

#include "../vendor/stb/stb_image.h"
#include "../vendor/stb/stb_image_resize2.h"

namespace iq {

    const std::string DataManager::DefaultTexture = "default";

    DataManager::DataManager() 
        :m_fontstash_context(nullptr), m_settings {}
    { }
    
    DataManager::~DataManager() {
        release();
    }  

    void DataManager::release() {
        releaseTextures();
        releaseFontstash();
    }

    //
    // Textures
    //
    void DataManager::releaseTextures() {
        for (auto& current : m_textures) {
            sg_destroy_image(current.image);
            sg_destroy_sampler(current.sampler);
        }

		m_textures.clear();
    }

    struct Level {
        int w = 0;
        int h = 0;
        int c = 0;
        uint8_t* data = nullptr;
    };

    static void buildMipmaps(std::vector<Level>& levels) {
        if (levels.empty())
            return;

        const int min_size = 1;
        Level& current = levels.back();
        if (current.w <= min_size || current.h <= min_size)
            return;

        Level next;
        next.w = current.w / 2;
        next.h = current.h / 2;
        next.c = current.c;
        next.data = (uint8_t*)malloc(next.w * next.h * next.c);

        if (next.data == nullptr) {
            std::cerr << "Failed to allocate memory for texture" << std::endl;
            return;
        }
        
        if (stbir_resize_uint8_linear(
                (unsigned char*)current.data, current.w, current.h, current.w * current.c,
                (unsigned char *)next.data, next.w, next.h, next.w * next.c,
                STBIR_RGBA) == 0) {

            std::cerr << "Failed to resize texture" << std::endl;
            return;
        }

        levels.push_back(next);
        buildMipmaps(levels);
    }

    DataManager::Texture DataManager::texture(std::string const& name) {
        auto found = std::find_if(m_textures.begin(), m_textures.end(), [&name](Texture const& t) {
            return *(t.name) == name;
        });
        
        if (found != m_textures.end())
            return *found;

        //std::cout << "Loading Texture: " << name << std::endl;
        
        Texture texture {};
        texture.name = std::make_shared<std::string>(name);
        
        std::string path;
        bool is_default = (name == DefaultTexture);
        std::vector<Level> levels;
        
        if (is_default) {
            // create a checkerboard texture
            static uint32_t pixels[4*4] = {
                0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
                0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
                0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
            };

            levels.push_back(Level {
                .w = 4,
                .h = 4,
                .c = 4,
                .data = (stbi_uc*) pixels
            });

        } else {
            path = assetPath("textures", name);

            if (path.empty())
                return texture;
            
            std::vector<uint8_t> raw_buffer;
            if (!assetReadRawBinary(path.c_str(), raw_buffer))
                return texture;

            Level level;
            level.data = stbi_load_from_memory(raw_buffer.data(), raw_buffer.size(), &level.w, &level.h, &level.c, 4);
            level.c = 4;
            if (level.data == nullptr)
                return texture;

            levels.push_back(level);
        }

        buildMipmaps(levels);


        int levels_count = int(levels.size());
    
        sg_image_desc image_desc = {
			.width = levels[0].w,
			.height = levels[0].w,
            .num_mipmaps = levels_count > 1 ? levels_count : 0,
            // set pixel_format to RGBA8 for WebGL 
			.pixel_format = SG_PIXELFORMAT_RGBA8,
            .label = texture.name->c_str()
		};

        for (int l = 0; l != levels.size(); ++l) {
            //std::cout << "level " << l << " " << levels[l].w << "x" << levels[l].h << std::endl;
            image_desc.data.subimage[0][l] = {
                .ptr = levels[l].data,
                .size = size_t(levels[l].w * levels[l].h * 4)
            };
        }

        texture.image = sg_make_image(image_desc);

        if (!is_default) {
            for (auto& current : levels) 
                free(current.data);
        }

        sg_sampler_desc sampler_desc = { 
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .label = texture.name->c_str()
        };

        if (levels_count > 1) {
            sampler_desc.mipmap_filter = SG_FILTER_LINEAR;
            sampler_desc.min_lod = 0;
            sampler_desc.max_lod = levels_count;
            sampler_desc.max_anisotropy = 16;
        }
        
        texture.sampler = sg_make_sampler(sampler_desc);

        texture.valid = true;
        m_textures.push_back(texture);

        return texture;
    }

    //
    // Colors
    //
    std::vector<sg_color> const& DataManager::colors() {
        static const std::vector<sg_color> values = {
            
            backgroundColor(),
            groundColor(),
            
            shadowColor(),

            playerColor(),
            
            normalMarkerColor(),
            advantageMarkerColor(),
            triggeredMarkerColor(),
            
            normalCubeColor(),
            advantageCubeColor(),
            forbiddenCubeColor(),
            whiteColor(),
            explosionColor(),
        };
        
        return values;
    }


    //
    // Fonts
    //
    DataManager::Font DataManager::font(std::string const& name) {
        auto found = std::find_if(m_fonts.begin(), m_fonts.end(), [&name](auto const& f) { return *(f.name) == name; });
        if (found != m_fonts.end())
            return *found;

        if (m_fontstash_context == nullptr) 
            createFontstash();
        
        Font font;
        font.valid = false;
        font.name = std::make_shared<std::string>(name);

        std::string path = assetPath("fonts", name);
        if (path.empty())
            return font;

        font.data = std::make_shared<std::vector<uint8_t>>();
        font.valid = assetReadRawBinary(path, *font.data);
        if (font.valid) {
            font.id = fonsAddFontMem(m_fontstash_context, font.name->c_str(), font.data->data(), int(font.data->size()),  false);
            m_fonts.push_back(font);
        }
        return font;
    }

    // round to next power of 2 (see bit-twiddling-hacks)
    static int round_pow2(float v) {
        uint32_t vi = ((uint32_t) v) - 1;
        for (uint32_t i = 0; i < 5; i++) {
            vi |= (vi >> (1<<i));
        }
        return (int) (vi + 1);
    }

    void DataManager::createFontstash() {
        float dpi_scale = 1.0f;
        int const atlas_dim = round_pow2(512.0f * dpi_scale);

        sfons_desc_t sfons_desc = {
            .width = atlas_dim,
            .height = atlas_dim,
            
        };

        m_fontstash_context = sfons_create(&sfons_desc);
    }

    void DataManager::releaseFontstash() {
        if (m_fontstash_context != nullptr) {
            sfons_destroy(m_fontstash_context);
            m_fontstash_context = nullptr;
        }
        m_fonts.clear();
    }

    //
    // Stages
    //
    struct PuzzleSearchState {
        std::vector<PuzzleData> all;
        std::set<std::string> used;
        iq::Random random;
    };

    static PuzzleData puzzleDataByName(PuzzleSearchState& state, std::string const& name) {
        PuzzleData puzzle_data {};

        auto iterator = std::find_if(state.all.begin(), state.all.end(), [&name](PuzzleData const& c) {
            return c.name == name;
        });

        if (iterator != state.all.end()) 
            puzzle_data = *iterator;

        return puzzle_data;
    }

    static DataManager::Wave generateWave(PuzzleSearchState& state, std::string const& name, int count, int forced_index = -1) {
        PuzzleData puzzle_data = puzzleDataByName(state, name);
        DataManager::Wave wave {
            .columns = puzzle_data.columns,
            .rows = puzzle_data.rows
        };

        if (puzzle_data.puzzles == 0) {
            return wave;
        }

        for (int i = 0; i != count; ++i) {
            // ensure we don't pick a puzzle already used
            uint32_t index = 0;
            std::string index_key;

            if (forced_index != -1) {
                index = forced_index % puzzle_data.puzzles;
                index_key = name + "." + std::to_string(index);
            } else {
                do {
                    index = state.random.nextInt(0, puzzle_data.puzzles - 1);
                    index_key = name + "." + std::to_string(index);
                } while (state.used.contains(index_key));
            }
            
            state.used.insert(index_key);


            DataManager::Puzzle puzzle;
            puzzle.key = index_key;

            // get the puzzele text data
            uint8_t const* data = puzzle_data.data + puzzle_data.data_step * index;

            // get turns
            char number[3];
            number[0] = *(data + 1);
            number[1] = *(data + 2);
            number[2] = 0;

            puzzle.turns = std::stoi(number);

            // generate cubes
            for (int r = 0; r != puzzle_data.rows; ++r) {
                for (int c = 0; c != puzzle_data.columns; ++c) {
                    uint8_t cube = *(data + 4 + r * puzzle_data.columns + c);
                    puzzle.cubes.push_back(cube);
                }
            }


            wave.puzzles.push_back(puzzle);        
        }

        return wave;
    }


    DataManager::Stages DataManager::stages(uint32_t seed) {
        Stages output;
        output.seed = seed;

        PuzzleSearchState state;
        state.all = puzzles();
        state.random.reset(seed);

        // "4x2", "4x3", "4x4", "4x5", "4x6",
        // "5x4", "5x5", "5x6", "5x7", "5x8",
        // "6x6", "6x7", "6x8", "6x9", 
        // "7x7", "7x8", "7x9", "7x10"
        // "8x10", "8x9"
        // "9x10"


        int s = 0;
        {
            output.stages[s].push_back(generateWave(state, "4x2", 3));
            output.stages[s].push_back(generateWave(state, "4x2", 3));
            output.stages[s].push_back(generateWave(state, "4x3", 3));
            output.stages[s].push_back(generateWave(state, "4x4", 3));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "4x5", 3));
            output.stages[s].push_back(generateWave(state, "4x5", 3));
            output.stages[s].push_back(generateWave(state, "4x6", 3));
            output.stages[s].push_back(generateWave(state, "4x6", 3));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "5x4", 3));
            output.stages[s].push_back(generateWave(state, "5x5", 3));
            output.stages[s].push_back(generateWave(state, "5x6", 3));
            output.stages[s].push_back(generateWave(state, "5x6", 3));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "5x7", 2));
            output.stages[s].push_back(generateWave(state, "5x7", 2));
            output.stages[s].push_back(generateWave(state, "5x8", 2));
            output.stages[s].push_back(generateWave(state, "5x8", 2));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "6x6", 3));
            output.stages[s].push_back(generateWave(state, "6x6", 3));
            output.stages[s].push_back(generateWave(state, "6x7", 3));
            output.stages[s].push_back(generateWave(state, "6x7", 3));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "6x8", 2));
            output.stages[s].push_back(generateWave(state, "6x8", 2));
            output.stages[s].push_back(generateWave(state, "6x9", 2));
            output.stages[s].push_back(generateWave(state, "6x9", 2));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "7x7", 3));
            output.stages[s].push_back(generateWave(state, "7x7", 3));
            output.stages[s].push_back(generateWave(state, "7x8", 3));
            output.stages[s].push_back(generateWave(state, "7x8", 3));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "7x8", 2));
            output.stages[s].push_back(generateWave(state, "7x9", 2));
            output.stages[s].push_back(generateWave(state, "7x9", 2));
            output.stages[s].push_back(generateWave(state, "7x9", 2));
        }

        if (output.stages.size() > s) {
            s++;
            output.stages[s].push_back(generateWave(state, "7x9", 1));
            output.stages[s].push_back(generateWave(state, "7x9", 1));
            output.stages[s].push_back(generateWave(state, "7x9", 1));
            output.stages[s].push_back(generateWave(state, "7x9", 1));
        }        


        return output;
    }

    DataManager::Wave DataManager::puzzle(std::string const name) {
        DataManager::Wave wave;

        std::string::size_type pos = name.find(".");
        if (pos == std::string::npos) 
            return wave;
        
        if (pos == name.size() - 1)
            return wave;

        std::string size_str = name.substr(0, pos);
        std::string index_str = name.substr(pos + 1, std::string::npos);

        int index = 0;
        try {
            index = std::stoi(index_str);
        } catch(std::exception const& exception) { }

        PuzzleSearchState state;
        state.all = puzzles();

        wave = generateWave(state, size_str, 1, index);
        return wave;
    }
}
