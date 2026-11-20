#import <AppKit/AppKit.h>
#include "MacHostWindow.h"

namespace dance_native {
namespace {
NSWindow* windowForView(void* view)
{
    return [(NSView*) view window];
}
}

bool isGarageBandFrontmost()
{
    NSRunningApplication* frontmost = [[NSWorkspace sharedWorkspace] frontmostApplication];
    return [[frontmost bundleIdentifier] isEqualToString:@"com.apple.garageband10"];
}

bool isGarageBandProcess()
{
    return [[[NSBundle mainBundle] bundleIdentifier] isEqualToString:@"com.apple.garageband10"];
}

bool macEditorWindowIsVisible(void* editorView)
{
    return [windowForView(editorView) isVisible];
}

std::intptr_t findMacProjectWindow(void* editorView)
{
    NSWindow* editor = windowForView(editorView);
    if (editor == nil || !isGarageBandProcess())
        return 0;

    // Only called while this instance's editor is showing. Its project window
    // must be tracked by identity rather than replaced by the next main window.
    auto isCandidate = [editor](NSWindow* window)
    {
        return window != nil && window != editor && [window isVisible]
               && [window windowNumber] > 0
               && ([window styleMask] & NSWindowStyleMaskTitled) != 0;
    };

    if (NSWindow* parent = [editor parentWindow]; isCandidate(parent))
        return [parent windowNumber];

    if (NSWindow* main = [NSApp mainWindow]; isCandidate(main))
        return [main windowNumber];

    NSWindow* largest = nil;
    CGFloat largestArea = 0;
    for (NSWindow* window in [NSApp orderedWindows])
    {
        if (!isCandidate(window) || [window parentWindow] != nil)
            continue;
        const auto size = [window frame].size;
        const auto area = size.width * size.height;
        if (area > largestArea)
        {
            largest = window;
            largestArea = area;
        }
    }
    return [largest windowNumber];
}

bool macProjectWindowIsVisible(std::intptr_t windowNumber)
{
    return windowNumber > 0 && [[NSApp windowWithWindowNumber:windowNumber] isVisible];
}

void configureMacMascotWindow(void* mascotView)
{
    // Float above document windows without depending on a DAW editor or becoming key.
    [windowForView(mascotView) setLevel:NSFloatingWindowLevel];
}

void restoreMacMascotWindowIfHidden(void* mascotView)
{
    // Hosts can order the native window out without changing JUCE's visibility flag.
    // The foreground check has already passed. Reorder only when actually hidden.
    NSWindow* window = windowForView(mascotView);
    if (window != nil && ![window isVisible])
        [window orderFrontRegardless];
}
}
