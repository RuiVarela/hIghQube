#include "ToolsWindow.hpp"
#include "Game.hpp"


#include "../vendor/sokol/util/sokol_imgui.h"
#include "../vendor/sokol/util/sokol_gfx_imgui.h"
#include "../vendor/imgui/imgui.h"

#include <mutex>
#include <fstream>

namespace iq {

    struct ToolsWindow::PrivateImplementation {
		sg_pass_action imgui_pass = {};
		sg_imgui_t sg_imgui = {};

		std::recursive_mutex logger_mutex;
		ImGuiTextBuffer logger_buf;
		ImGuiTextFilter logger_filter;
		ImVector<int> logger_line_offsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool logger_auto_scroll = true;  // Keep scrolling if already at the bottom.

		bool show_imgui_demo_window = false;

		int map_seed = 0;
		static int const puzzle_name_size = 15;
		char puzzle_name[puzzle_name_size];
	};

    ToolsWindow::ToolsWindow()
		:m(std::make_shared<PrivateImplementation>()),
        m_showing(true)
	{
		strncpy(m->puzzle_name, "4x2.13", m->puzzle_name_size);

		// setup sokol-imgui
		{
			// don't overwrite the framebuffer
			m->imgui_pass = { };
			m->imgui_pass.colors[0] = {
				.load_action = SG_LOADACTION_DONTCARE,
				.clear_value = { 0.3f, 0.7f, 0.0f, 1.0f }
			};

			simgui_desc_t simgui_desc = { };
			simgui_desc.logger.func = slog_func;
			simgui_setup(&simgui_desc);
		}

		// sg debug imgui tools
		{
			sg_imgui_desc_t desc = {};
			sg_imgui_init(&m->sg_imgui, &desc);
		}


		m->logger_auto_scroll = true;
		clearLog();
	}

    ToolsWindow::~ToolsWindow() {
		sg_imgui_discard(&m->sg_imgui);
		simgui_shutdown();
        m.reset();
    }

 	void ToolsWindow::handleInput(const sapp_event* event) {
    	simgui_handle_event(event);
    }

    void ToolsWindow::render(Game& game) {

		Inputs const& inputs = game.inputs();
		simgui_new_frame({ inputs.width, inputs.height, inputs.frame_duration, inputs.dpi_scale});

		ImGui::SetNextWindowSize(ImVec2(500, 0));
		ImGui::Begin("Tools", &m_showing, ImGuiWindowFlags_NoResize);

            ImGui::Text("Real: %.3f Streched: %.3f Application (%.1f FPS)", game.inputs().time, game.speeder().time(), ImGui::GetIO().Framerate);

			if (true)
			{
				ImGui::Checkbox("ImGui", &m->show_imgui_demo_window);
				ImGui::SameLine();
				if (ImGui::Button("Sokol Debug")) ImGui::OpenPopup("sokol_debug");

				ImGui::SameLine();
				if (ImGui::Button("Tasks")) ImGui::OpenPopup("select_tasks");

				ImGui::SameLine();
				if (ImGui::Checkbox("Move Camera", &game.settings().debug_camera)){
					game.log("Debug", "Move Camera = " + std::to_string(game.settings().debug_camera));
				}
			}

			if (true)
			{
				ImGui::Text("Wave");
				ImGui::SameLine();

				if (ImGui::Button("Move")) game.map().setWaveState(WaveState::Rotating);
				ImGui::SameLine();

				if (ImGui::Button("Halt")) game.map().setWaveState(WaveState::NotStarted);
				ImGui::SameLine();

				if (ImGui::Button("Restart")) game.map().buildWave();
				ImGui::SameLine();

				if (ImGui::Button("Next")) game.nextPuzzle();
			}

			if (true)
			{
		
				if (ImGui::Button("Add Row")) game.map().addRowToCreate();
				ImGui::SameLine();

				if (ImGui::Button("Destroy Row")) game.map().addRowToDestroy();
				ImGui::SameLine();

				if (ImGui::Button("Explode Cube")) {
					Map& map = game.map();

					bool exploded = false;
					for (int r = map.rows() - 1; r >= 0; --r) {
						for (int c = 0; c != map.columns(); ++c) {
							
							if (map.hasAnyCube(c, r)) {
								map.markCubeState(c, r, CubeState::Exploding);
								exploded = true;
							}

							if (exploded) break;
						}
						if (exploded) break;
					}

				}
				
			}

			if (true)
			{
				ImGui::GetTextLineHeightWithSpacing();
				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10);
				ImGui::InputInt("##Stages Seed", &m->map_seed);
				ImGui::SameLine();
				if (ImGui::Button("Gen Stages")) game.prepareGame(m->map_seed);
				ImGui::SameLine();

				ImGui::SetNextItemWidth(ImGui::GetTextLineHeight() * 10);
				ImGui::InputText("##LevelCode", m->puzzle_name, m->puzzle_name_size);
				ImGui::SameLine();
				if (ImGui::Button("Load Puzzle")) game.testPuzzle(m->puzzle_name);
				ImGui::SameLine();
			}



			if (ImGui::BeginPopup("sokol_debug")) {
				if (ImGui::Button("Capabilities")) m->sg_imgui.caps.open = true;
				if (ImGui::Button("Frame Stats")) m->sg_imgui.frame_stats.open = true;
				if (ImGui::Button("Buffers")) m->sg_imgui.buffers.open = true;
				if (ImGui::Button("Images")) m->sg_imgui.images.open = true;
				if (ImGui::Button("Samplers")) m->sg_imgui.samplers.open = true;
				if (ImGui::Button("Shaders")) m->sg_imgui.shaders.open = true;
				if (ImGui::Button("Pipelines")) m->sg_imgui.pipelines.open = true;
				if (ImGui::Button("Passes")) m->sg_imgui.passes.open = true;
				if (ImGui::Button("Calls")) m->sg_imgui.capture.open = true;

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("select_tasks")) {
				if (ImGui::Button("Geometry"))
					game.log("Debug", 
						"Width: " + std::to_string(game.viewWidth()) + " "
						"Height: "+ std::to_string(game.viewHeight()) + " "
						"DPI Scale: " + std::to_string(game.inputs().dpi_scale));

				//if (ImGui::Button("ImGui"))
				//    m->show_imgui_demo_window = !m->show_imgui_demo_window;

				ImGui::EndPopup();
			}
            
       		renderLog();

		ImGui::End();


		// Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
		if (m->show_imgui_demo_window) {
			// ImGui::SetNextWindowPos(ImVec2(100, 20), ImGuiCond_FirstUseEver);
			ImGui::ShowDemoWindow();
		}

		sg_imgui_draw(&m->sg_imgui);

	    sg_begin_default_pass(&m->imgui_pass, inputs.width, inputs.height);
		simgui_render();
		sg_end_pass();

    }

    //
	// Log
	//
	void ToolsWindow::clearLog() {
		std::unique_lock<std::recursive_mutex> lock(m->logger_mutex);

		m->logger_buf.clear();
		m->logger_line_offsets.clear();
		m->logger_line_offsets.push_back(0);
	}

	void ToolsWindow::addLog(std::string const& message) {
		if (message.empty()) return;

		std::unique_lock<std::recursive_mutex> lock(m->logger_mutex);

		int old_size = m->logger_buf.size();
		m->logger_buf.append(message.c_str());
		m->logger_buf.append("\n");

		for (int new_size = m->logger_buf.size(); old_size < new_size; old_size++)
			if (m->logger_buf[old_size] == '\n')
				m->logger_line_offsets.push_back(old_size + 1);
	}

	void ToolsWindow::renderLog()
	{
		std::unique_lock<std::recursive_mutex> lock(m->logger_mutex);

		ImGui::Separator();

		std::string title = "-- LOG --";
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(title.c_str()).x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::TextUnformatted(title.c_str());


		// Options menu
		if (ImGui::BeginPopup("Options")) {
			ImGui::Checkbox("Auto-scroll", &m->logger_auto_scroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");

		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		m->logger_filter.Draw("Filter", -100.0f);

		ImGui::BeginChild("scrolling", ImVec2(0, 250), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

		if (clear)
			clearLog();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = m->logger_buf.begin();
		const char* buf_end = m->logger_buf.end();
		if (m->logger_filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of
			// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < m->logger_line_offsets.Size; line_no++)
			{
				const char* line_start = buf + m->logger_line_offsets[line_no];
				const char* line_end = (line_no + 1 < m->logger_line_offsets.Size) ? (buf + m->logger_line_offsets[line_no + 1] - 1) : buf_end;
				if (m->logger_filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
			// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			// within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
			// on your side is recommended. Using ImGuiListClipper requires
			// - A) random access into your data
			// - B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display
			// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
			// it possible (and would be recommended if you want to search through tens of thousands of entries).
			ImGuiListClipper clipper;
			clipper.Begin(m->logger_line_offsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + m->logger_line_offsets[line_no];
					const char* line_end = (line_no + 1 < m->logger_line_offsets.Size) ? (buf + m->logger_line_offsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (m->logger_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();

	}
}
