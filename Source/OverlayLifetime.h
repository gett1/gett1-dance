#pragma once
#include <cstdint>
#include <memory>

namespace dance {
class ProjectWindowLifetime
{
public:
    bool hasWindow() const noexcept { return number != 0; }
    std::intptr_t windowNumber() const noexcept { return number; }

    void rememberWindow(std::intptr_t projectWindow) noexcept
    {
        if (!hasWindow())
            number = projectWindow;
    }

    bool isVisible(bool projectVisible, bool editorVisible, bool hasRecentAudio) const noexcept
    {
        return hasWindow() ? projectVisible : editorVisible || hasRecentAudio;
    }

private:
    std::intptr_t number = 0;
};

// Message-thread-only ownership. Closing an editor has no effect on this object.
// Keep the native surface when hidden, preserving position across Alt+Tab and UI reopen.
template <typename Surface>
class OverlayLifetime {
public:
    Surface* updateVisibility(bool enabled, bool hostForeground) {
        const bool show = enabled && hostForeground;
        if (show && !surface) surface = std::make_unique<Surface>();
        if (!surface) return nullptr;
        if (surface->isVisible() != show) {
            surface->setVisible(show);
            // Raise once on a visibility transition. Repeated SetWindowPos calls
            // under a moving pointer can cause layered-window hover flicker.
            if (show) surface->toFront(false);
        }
        return show ? surface.get() : nullptr;
    }
private:
    std::unique_ptr<Surface> surface;
};
}
