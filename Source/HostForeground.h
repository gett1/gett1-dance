#pragma once
#include <juce_core/juce_core.h>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
#endif

inline bool isDanceHostForeground() noexcept {
   #if JUCE_WINDOWS
    // Use a single HWND snapshot and fail closed if Windows has no foreground
    // window during Alt+Tab/minimisation. No titles or editor handles are needed.
    const HWND foreground=GetForegroundWindow();
    DWORD processId=0;
    return foreground!=nullptr
        && GetWindowThreadProcessId(foreground,&processId)!=0
        && processId==GetCurrentProcessId()
        && IsIconic(foreground)==FALSE;
   #else
    return juce::Process::isForegroundProcess();
   #endif
}

#if JUCE_WINDOWS
namespace dance_native {
struct OwnerSearch { HWND excluded=nullptr,best=nullptr; long long area=0; };
inline BOOL CALLBACK inspectOwner(HWND hwnd,LPARAM context) {
    auto& search=*reinterpret_cast<OwnerSearch*>(context);
    DWORD pid=0;GetWindowThreadProcessId(hwnd,&pid);
    if(hwnd==search.excluded || pid!=GetCurrentProcessId() || !IsWindowVisible(hwnd)
       || GetWindow(hwnd,GW_OWNER)!=nullptr || (GetWindowLongPtrW(hwnd,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0
       || (GetWindowLongPtrW(hwnd,GWL_STYLE)&WS_CHILD)!=0 || GetWindowTextLengthW(hwnd)==0) return TRUE;
    RECT r{};
    if(GetWindowRect(hwnd,&r)) {
        const long long area=(long long)(r.right-r.left)*(r.bottom-r.top);
        if(area>search.area) {search.area=area;search.best=hwnd;}
    }
    return TRUE;
}
inline void attachToHost(HWND overlay) {
    OwnerSearch search;search.excluded=overlay;
    EnumWindows(&inspectOwner,reinterpret_cast<LPARAM>(&search));
    // Attach to the main unowned host window, never the disposable plugin editor.
    if(search.best!=nullptr)SetWindowLongPtrW(overlay,GWLP_HWNDPARENT,reinterpret_cast<LONG_PTR>(search.best));
    const auto style=GetWindowLongPtrW(overlay,GWL_EXSTYLE);
    SetWindowLongPtrW(overlay,GWL_EXSTYLE,style|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW);
    SetWindowPos(overlay,nullptr,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
}
}
#endif
