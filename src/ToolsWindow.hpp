#pragma once

#include <Vendor.hpp>

namespace iq {

    class Game;

    class ToolsWindow {
    public:
        ToolsWindow();
        ~ToolsWindow();
    
        void render(Game& game);
        void handleInput(const sapp_event* event);

        bool showing() const { return m_showing; }
        void show(bool value = true) { m_showing = value; }

		void clearLog();
		void addLog(std::string const& message);
	private:
		struct PrivateImplementation;
		std::shared_ptr<PrivateImplementation> m;

		void renderLog();
        bool m_showing;
    };
}
