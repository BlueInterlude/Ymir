#include "scu_registers_window.hpp"

#include <imgui.h>

namespace app::ui {

SCURegistersWindow::SCURegistersWindow(SharedContext &context)
    : WindowBase(context)
    , m_regsView(context)
    , m_intrView(context)
    , m_timersView(context) {

    m_windowConfig.name = "SCU registers";
    m_windowConfig.flags = ImGuiWindowFlags_AlwaysAutoResize;
}

void SCURegistersWindow::DrawContents() {
    ImGui::SeparatorText("Registers");
    m_regsView.Display();

    ImGui::SeparatorText("Interrupts");
    m_intrView.Display();

    ImGui::SeparatorText("Timers");
    m_timersView.Display();
}

} // namespace app::ui
