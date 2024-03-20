#pragma once

#include "Object.hpp"
#include "Inputs.hpp"
#include "Interpolator.hpp"

namespace iq {
    class Map;

    //
    // Selection
    //
    enum class SelectionState {
        None,

        Appearing,
        Picked,
        Capturing,
        Disapearing,
        DisapearingExploding,
        Deleted,

        Count
    };

    enum class SelectionKind {
        UserPick,
        AdvantagePick,
        AdvantageDestroy
    };
    
    struct Selected {
        int column = 0;
        int row = 0;
        double start = 0.0;
        SelectionKind kind = SelectionKind::UserPick; 
        SelectionState state = SelectionState::None;
        Object marker = Object(ObjectKind::TexturedCube);
        Object guide = Object(ObjectKind::TexturedPyramid);
    };

    class Player {
    public:
        Player();
        
        Map& map() { return *m_map; }

        void init(Map* map);
        void update();

        void markAdvantage(int column, int row);
        bool anySelectedState(SelectionState state);

        HMM_Vec3 const& position() const { return m_player.position(); }
        
        void groundMarkers(std::vector<Object*>& objects);
        void playerObjects(std::vector<Object*>& objects);
        void airGuides(std::vector<Object*>& objects);
    private:
        Map* m_map;
        Object m_player;
        Object m_player_shadow;
        float m_player_scale_clock;
        float m_selection_rotation;
        Inputs m_inputs;

        void ensurePlayerNotHittingElements(HMM_Vec3& position);

        struct Collision {
            float left = 0.0f;
            float right = 0.0f;
            float begin = 0.0f;
            float end = 0.0f;

            bool outside_ground = false;
            bool wave = false;
        };
        std::vector<Collision> findCollisions(HMM_Vec3& position);

        void triggerAdvantage();
        void markSquare();
        
        std::vector<Selected> m_selection;
        Selected createSelection(int column, int row, SelectionKind kind);
        void updateSelection();
    };

}
