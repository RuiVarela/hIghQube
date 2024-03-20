#include <cstdint>
#include <string>
#include <vector>

namespace iq {

    struct PuzzleData {
        std::string name;

        int columns;
        int rows;
        int puzzles;
        
        size_t data_step;
        size_t data_size;
        uint8_t const* data;
    };
    std::vector<PuzzleData> puzzles();
}