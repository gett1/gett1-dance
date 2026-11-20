#pragma once
#include <cstdint>

namespace dance_native {
bool isGarageBandFrontmost();
bool isGarageBandProcess();
bool macEditorWindowIsVisible(void* editorView);
std::intptr_t findMacProjectWindow(void* editorView);
bool macProjectWindowIsVisible(std::intptr_t windowNumber);
void configureMacMascotWindow(void* mascotView);
void restoreMacMascotWindowIfHidden(void* mascotView);
}
