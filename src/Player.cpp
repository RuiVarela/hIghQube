#include "Player.hpp"
#include "Map.hpp"
#include "Game.hpp"


constexpr float MarkerAppearTime = 0.08f;

namespace iq {

    Player::Player()
        :m_map(nullptr),
        m_player(ObjectKind::TexturedPyramid),
        m_player_shadow(ObjectKind::SolidColorCube),
        m_player_scale_clock(0.0f),
        m_selection_rotation(0.0f)
    {  }

    void Player::init(Map* map) {
        m_map = map;
        GameSettings const& settings = m_map->game().settings();

        
        // player
        {
            int index = map->wave().rows + 2;
            HMM_Vec3 position = map->position(0, index);
            position.X = map->columns() / 2.0f;
            position.Y = settings.player_height / 2.0f;

            m_player.setPosition(position);
            m_player.setColor(m_map->dataManager().playerColor());
            m_player.setTexture(m_map->dataManager().playerTexture());

            m_player_shadow.setPosition(position.X, 0.0f, position.Z);
            m_player_shadow.setColor(m_map->dataManager().shadowColor());
            m_player_shadow.setTexture(m_map->dataManager().playerTexture());
        }
        
        // selection
        m_selection_rotation = 0.0f;
        m_selection.clear();
    }

    void Player::update() {
        Inputs const& inputs = m_map->game().inputs();
        double duration = inputs.frame_duration;
        GameSettings const& settings = m_map->game().settings();


        HMM_Vec3 position = m_player.position();

        // handle inputs
        if ((!settings.debug_camera) && (inputs.forward || inputs.backward || inputs.left || inputs.right)) {
            HMM_Vec3 direction{};

            if (inputs.forward)
                direction.Z -= 1.0f;
            
            if (inputs.backward)
                direction.Z = 1.0f;

            if (inputs.left)
                direction.X -= 1.0f;
            
            if (inputs.right)
                direction.X = 1.0f;

            direction = HMM_Norm(direction);
            position = m_player.position() + direction * settings.player_speed * duration;
        }


        ensurePlayerNotHittingElements(position);
        m_player.setPosition(position);
        m_player_shadow.setPosition(position.X, m_player_shadow.position().Y, position.Z);

  
        if ((m_inputs.cross != inputs.cross) && inputs.cross)
            markSquare();

        if ((m_inputs.triangle != inputs.triangle) && inputs.triangle)
            triggerAdvantage();
        

        //
        // grow shrink effect
        //
        m_player_scale_clock = fmodf(m_player_scale_clock + 3.0f * duration, HMM_PI32 * 2.0f);  
        float scaler = sin(m_player_scale_clock) * 0.1f;
        HMM_Vec3 scale {settings.player_width, settings.player_height, settings.player_width};
        scale = scale * (1.0f - scaler);

        m_player.setScale(scale);
        m_player.setRotation(0.0f, m_player_scale_clock, 0.0f);

        m_player_shadow.setScale(settings.player_width, 0.001f, settings.player_width);
        m_player_shadow.setRotation(0.0f, m_player_scale_clock, 0.0f);

        updateSelection();

        m_inputs = inputs;
    }

    std::vector<Player::Collision> Player::findCollisions(HMM_Vec3& position){
        std::vector<Collision> output;

  
        // check collision with wave
        Wave const& wave = map().wave();

        for (int r = 0; r != wave.rows; ++r) {
            for (int c = 0; c != map().columns(); ++c) {
                if (wave.cubes[r * map().columns() + c].state == CubeState::Empty) continue;

                Object const& cube = wave.cubes[r * map().columns() + c].object; 
                float flip_percentage = cube.percentage();
                HMM_Vec3 cp = cube.position();

                cp.Z += sin(flip_percentage * HMM_PI / 2);
                Collision collision {
                    cp.X - 1.0f / 2.0f, // left
                    cp.X + 1.0f / 2.0f, // right

                    cp.Z - 1.0f / 2.0f, // begin
                    cp.Z + 1.0f / 2.0f, // end
                };
                collision.wave = true;

                if (position.X >= collision.left && position.X <= collision.right && 
                    position.Z >= collision.begin && position.Z <= collision.end) {
                    output.push_back(collision);
                }
            }
        }

        // map
        {
            Collision collision { 0.0f, float(map().columns()), 0.0f, float(map().rows()) };
            collision.outside_ground = true;
            if (position.X < collision.left || position.X > collision.right || 
                position.Z < collision.begin || position.Z > collision.end) {
                output.push_back(collision);
            }
        }
        
        return output;
    }

    void Player::ensurePlayerNotHittingElements(HMM_Vec3& position) {

        // force player within map
        {
            position.X = HMM_Clamp(position.X, 0.0f, float(map().columns()) );
            position.Z = HMM_Clamp(position.Z, 0.0f, float(map().rows()) );
        }


        auto hits = findCollisions(position);

        for (auto const &hit : hits) {
            assert(hit.wave);

            float offset = 0.01f;

            // get the distances to the edges
            std::array<std::pair<int, float>, 4> distances {
                std::make_pair(0, float(fabs(position.X - hit.left))), // left distance
                std::make_pair(1, float(fabs(hit.right - position.X))), // right distance
                std::make_pair(2, float(fabs(position.Z - hit.begin))), // begin distance
                std::make_pair(3, float(fabs(hit.end - position.Z))) // end distance
            };
            std::sort(distances.begin(), distances.end(), [](auto const& l, auto const& r) { return l.second < r.second; });

            for (size_t mode = 0; mode != 2; ++mode) {
                HMM_Vec3 p = position;

                // check for X and Z, the 2 nearest distances
                for (size_t d = 0; d != 2; ++d) {
                    auto const& distance = distances[d];

                    // on mode 0, we check if changing only 1 coordinate at a time resolves the collision
                    // on mode 1, we accumulate the changes on both coordinates
                    if (mode == 0) {
                        p = position;
                    }

                    if (distance.first == 0) {
                        p.X = hit.left - offset; // hitting from the left
                    } else if (distance.first == 1) {
                        p.X = hit.right + offset; // hitting from the right
                    } else if (distance.first == 2) {
                        p.Z = hit.begin - offset; // hitting from the begining
                    } else if (distance.first == 3) {
                        p.Z = hit.end + offset; // hitting from the end
                    }

                    if (findCollisions(p).empty()) {
                        //map().game().log("Player", "Resolved collision on mode" + std::to_string(mode) +  " index: " + std::to_string(d));
                        position = p;
                        return;
                    }
                }
            }
        
        }
    }

    void Player::triggerAdvantage() {
        std::vector<std::pair<int, int>> coords;

        Selected* user_picked = nullptr;
        bool delete_user_picked = false;

        for (auto& current : m_selection) {

            if (current.kind == SelectionKind::UserPick && (current.state == SelectionState::Appearing || current.state == SelectionState::Picked)) {
                user_picked = &current;
            }

            if (current.kind != SelectionKind::AdvantagePick) 
                continue;

            current.start = map().strechedTime();
            current.state = SelectionState::Disapearing;        

            coords.emplace_back(current.column, current.row);
        }

        std::vector<Selected> new_elements;

        for (auto& [column, row]: coords) {
            for (int r = row - 1; r <= row + 1; ++ r) {
                for (int c = column - 1; c <= column + 1; ++c) {
                    if (r < 0 || r >= map().rows()) continue;
                    if (c < 0 || c >= map().columns()) continue;

                    bool aleady_added = std::find_if(new_elements.begin(), new_elements.end(), [c, r](auto const& e){
                        return c == e.column && r == e.row;
                    }) != new_elements.end();

                    if (user_picked != nullptr && user_picked->column == c && user_picked->row == r) 
                        delete_user_picked = true;

                    if (!aleady_added) {
                        Selected selected = createSelection(c, r, SelectionKind::AdvantageDestroy);
                        selected.state = SelectionState::Capturing;
                        new_elements.push_back(selected);
                    }
                }
            }
        }

        if (delete_user_picked) {
            int r = user_picked->row;
            int c = user_picked->column;
            user_picked = nullptr;

            auto elements = std::remove_if(m_selection.begin(), m_selection.end(), [c, r](Selected const& current){
                return c == current.column && r == current.row;
            });
            m_selection.erase(elements, m_selection.end());
        }

        std::copy(new_elements.begin(), new_elements.end(), std::back_inserter(m_selection));

    }

    void Player::markSquare() {
        Selected* found = nullptr;

        int column =  HMM_Clamp(int(position().X), 0, map().columns() - 1);
        int row =  HMM_Clamp(int(position().Z), 0, map().rows() - 1);
        bool on_advantage_selection = false;


        for (auto& current : m_selection) {

            if (current.kind == SelectionKind::AdvantagePick) {

                // we cannot pick a region that is already selected by advantage
                if (current.column == column && current.row == row) {
                    on_advantage_selection = true;
                }

                // we should not handle advantage cubes here
                continue;
            } else if (current.state == SelectionState::Capturing && current.kind == SelectionKind::UserPick) {
                // don't allow picking while we are capturing what the user picked
                return;
            } else if (current.state == SelectionState::Appearing || current.state == SelectionState::Picked) {
                found = &current;
            } 
        }

        if (found == nullptr) {

            if (on_advantage_selection)
                return;

            Selected selected = createSelection(column, row, SelectionKind::UserPick);

            m_selection.push_back(selected);
        } else {
            found->start = map().strechedTime();

            if (found->state == SelectionState::Picked) {
                found->state = SelectionState::Disapearing;
            } else if (found->state == SelectionState::Appearing) {
                found->start -= (1.0f - found->guide.color().A) * MarkerAppearTime; // make sure we start from the previous alpha
                found->state = SelectionState::Disapearing;
            } 

            if (map().wave().state == WaveState::Rotating && map().hasAnyCube(found->column, found->row - 1)) {
                found->state = SelectionState::Capturing;
            } else if (map().wave().state == WaveState::Holding && map().hasAnyCube(found->column, found->row)) {
                found->state = SelectionState::Capturing;
            }

            if (found->state == SelectionState::Disapearing || found->state == SelectionState::Capturing) {
                sg_color color = m_map->dataManager().triggeredMarkerColor();
                found->guide.setColor(color);
                found->marker.setColor(color);
            } 
        }

        // map().game().log("Player", "toggleMarkSquare (" + std::to_string(column) + ", " + std::to_string(row) + ")");
    }

   void Player::markAdvantage(int column, int row) {
        Selected selection = createSelection(column, row, SelectionKind::AdvantagePick);
        m_selection.push_back(selection);
   }

    bool Player::anySelectedState(SelectionState state) {
        for (auto& current : m_selection) 
            if (current.state == state) 
                return true;
        return false;
    }

    void Player::groundMarkers(std::vector<Object*>& objects) {
        for (auto& current : m_selection) 
            objects.push_back(&current.marker);

        objects.push_back(&m_player_shadow);
    }

    void Player::playerObjects(std::vector<Object*>& objects) {
        objects.push_back(&m_player);
    }

    void Player::airGuides(std::vector<Object*>& objects) {
        for (auto& current : m_selection) 
            objects.push_back(&current.guide);
    }


    Selected Player::createSelection(int column, int row, SelectionKind kind) {
        const float guide_scale = 0.2f;
        
        assert(column >= 0 &&  column < map().columns());
        assert(row >= 0 &&  row < map().rows());

        Selected selected;
        selected.kind = kind;
        selected.column = column;
        selected.row = row;
        selected.start = map().strechedTime();   
        selected.state = SelectionState::Appearing;

        sg_color color = m_map->dataManager().normalMarkerColor();

        if (kind == SelectionKind::AdvantagePick) {
            color = m_map->dataManager().advantageMarkerColor();
        } else if (kind == SelectionKind::AdvantageDestroy) {
            color = m_map->dataManager().triggeredMarkerColor();
        }
  
        selected.guide.setScale(guide_scale, guide_scale, guide_scale);
        selected.guide.setPosition(map().position(column, row) + HMM_V3(0.0f, 0.5f + guide_scale, 0.0f));
        selected.guide.setColor(color);
        selected.guide.setTexture(m_map->dataManager().guideTexture());


        selected.marker.setScale(1.0f, 0.001f, 1.0f);
        selected.marker.setPosition(map().position(column, row) + HMM_V3(0.0f, -0.5f, 0.0f));
        selected.marker.setColor(color);
        selected.marker.setTexture(m_map->dataManager().cubeTexture());

        return selected;
    }

    void Player::updateSelection() {

        double frame_duration = map().game().inputs().frame_duration;
        double time = map().strechedTime();

        m_selection_rotation = fmodf(m_selection_rotation + 2.0f * frame_duration, HMM_PI32 * 2.0f);   

        for (auto& current : m_selection) {

            bool isDisapearing = current.state == SelectionState::Disapearing || current.state == SelectionState::DisapearingExploding;
            if (current.state == SelectionState::Appearing || isDisapearing) {
                double const duration = (current.state == SelectionState::DisapearingExploding) ? 0.5f : MarkerAppearTime;
                double const elapsed = time - current.start;
                float percentage = HMM_Clamp(0.0f, float(elapsed / duration), 1.0f);

                if (percentage >= 1.0f) {
                    current.guide.color().A = 1.0f;
                    current.marker.color().A = 1.0f;

                    if (current.state == SelectionState::Appearing) 
                        current.state = SelectionState::Picked;
                    else if (isDisapearing) 
                        current.state = SelectionState::Deleted;
                    
                    current.start = time;
                } else {
                    if (isDisapearing) 
                        percentage = 1.0f - percentage;

                    //std::cout << "Percentage: " << percentage << int(current.state) << std::endl;
                    
                    current.guide.color().A = percentage;
                    current.marker.color().A = percentage;
                }
            } else if (current.state == SelectionState::Capturing) {

                if (map().wave().state == WaveState::Holding) {

                    current.state = SelectionState::DisapearingExploding;
                    current.start = time;

                    sg_color color = m_map->dataManager().triggeredMarkerColor();
                    current.guide.setColor(color);
                    current.marker.setColor(color);

                    if (map().hasAnyCube(current.column, current.row)) {
                        map().markCubeState(current.column, current.row, CubeState::Exploding);
                    }
                }

            }

            current.guide.setRotation(0.0f, m_selection_rotation, 0.0f);
        }

        //
        // delete dead objects 
        //
        {
            auto current = m_selection.begin();
            while (current != m_selection.end()) {
                if (current->state == SelectionState::Deleted) {
                    current = m_selection.erase(current);
                } else {
                    current++;
                }
            }
        }
    }

}
