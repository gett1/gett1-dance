#include "OverlayLifetime.h"
#include <stdexcept>
#include <iostream>

#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)

struct FakeSurface {
    inline static int constructed=0, destroyed=0, activated=0,raised=0;
    bool visible=false;
    int x=0, y=0, renderedFrames=0;
    FakeSurface() { ++constructed; }
    ~FakeSurface() { ++destroyed; }
    bool isVisible() const { return visible; }
    void setVisible(bool v) { visible=v; }
    void toFront(bool activate) { ++raised; if(activate) ++activated; }
};

int main() {
    {
        dance::OverlayLifetime<FakeSurface> pluginSession;
        // Loading/scanning a disabled instance must not create native windows.
        CHECK(pluginSession.updateVisibility(false,false)==nullptr);
        CHECK(pluginSession.updateVisibility(false,true)==nullptr);
        CHECK(FakeSurface::constructed==0);
        // Enable in the host and move the mascot.
        auto* original=pluginSession.updateVisibility(true,true);
        CHECK(original && original->isVisible());
        original->x=420; original->y=120;
        // Open/close arbitrary disposable editor views; their lifetime must have
        // no effect on the independent plugin session or its ongoing renders.
        for(int reopen=0;reopen<10;++reopen) {
            struct TemporaryEditor { int preset=3; };
            { auto editor=std::make_unique<TemporaryEditor>(); CHECK(editor->preset==3); }
            for(int frame=0;frame<60;++frame) {
                auto* cat=pluginSession.updateVisibility(true,true);
                CHECK(cat==original && cat->isVisible());
                ++cat->renderedFrames;
            }
        }
        CHECK(original->renderedFrames==600);
        CHECK(FakeSurface::raised==1); // No Z-order churn while pointer/frame updates run.
        // Browser, desktop and minimised-host conditions all hide the same HWND.
        for(int altTab=0;altTab<100;++altTab) {
            CHECK(pluginSession.updateVisibility(true,false)==nullptr);
            CHECK(!original->isVisible());
            auto* restored=pluginSession.updateVisibility(true,true);
            CHECK(restored==original && restored->x==420 && restored->y==120);
        }
        CHECK(FakeSurface::raised==101);
        // User disabling the mascot is independent of focus/editor visibility.
        CHECK(pluginSession.updateVisibility(false,true)==nullptr);
        CHECK(!original->isVisible());
        CHECK(pluginSession.updateVisibility(true,true)==original);
        CHECK(FakeSurface::constructed==1 && FakeSurface::destroyed==0);
        CHECK(FakeSurface::activated==0);
    }
    CHECK(FakeSurface::destroyed==1);
    {
        dance::OverlayLifetime<FakeSurface> restoredSession;
        // Restored enabled state waits for the host, even without opening an editor.
        CHECK(restoredSession.updateVisibility(true,false)==nullptr);
        CHECK(FakeSurface::constructed==1);
        CHECK(restoredSession.updateVisibility(true,true)!=nullptr);
        CHECK(FakeSurface::constructed==2);
    }
    CHECK(FakeSurface::destroyed==2);
    {
        dance::ProjectWindowLifetime project;
        CHECK(project.isVisible(false, true, false)); // Editor visible before binding.
        CHECK(project.isVisible(false, false, true)); // Remote AU still processing.
        CHECK(!project.isVisible(false, false, false)); // Remote AU processing ended.

        project.rememberWindow(42);
        CHECK(project.isVisible(true, false, false)); // Project open, editor closed, playback stopped.
        CHECK(!project.isVisible(false, true, true)); // Project closed, even if editor or audio lingers.
        project.rememberWindow(99); // GarageBand's project chooser has a different window ID.
        CHECK(project.windowNumber() == 42);
        CHECK(!project.isVisible(false, false, false));
    }
    std::cout<<"Overlay lifetime/focus regression scenarios passed (simulated surface)\n";
}
