#pragma once

#include <app/shared_context.hpp>

namespace app::ui {

class SettingsViewBase {
public:
    SettingsViewBase(SharedContext &context);
    virtual ~SettingsViewBase() = default;

protected:
    // Makes the settings dirty unconditionally.
    void MakeDirty();

    // Makes the settings dirty if the given value is true and returns the value.
    // Useful to inline with interactible ImGui widgets.
    bool MakeDirty(bool value);

protected:
    SharedContext &m_context;
};

} // namespace app::ui
