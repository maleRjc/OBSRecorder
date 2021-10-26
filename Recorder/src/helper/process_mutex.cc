#include "process_mutex.h"

#include <Windows.h>

namespace {
HANDLE g_hMutexSingleInstance = NULL;
constexpr wchar_t kSzSingleInstanceMutex[] =
    L"Global_Recorder{SWdB6N7FHhPArwDlklCEcPyca8XrYdYAv74ptFp9}";
}  // namespace

ProcessMutex::ProcessMutex() {
  is_success_ = InitRecorderEnvironment();
}

ProcessMutex::~ProcessMutex() {
  UninitRecorderEnvironment();
}

bool ProcessMutex::InitRecorderEnvironment() {
  g_hMutexSingleInstance = ::CreateMutexW(NULL, TRUE, kSzSingleInstanceMutex);
  if (::GetLastError() == ERROR_ALREADY_EXISTS) {
    //HWND hwnd = ::FindWindowExW(NULL, NULL, WINDOW_CLASS_NAME, NULL);
    //if (hwnd)
    //  ::PostMessage(hwnd, WM_SHOWWINDOW, 0, 0);
    return false;
  }
  return true;
}

void ProcessMutex::UninitRecorderEnvironment() {
  if (g_hMutexSingleInstance) {
    ::CloseHandle(g_hMutexSingleInstance);
    g_hMutexSingleInstance = NULL;
  }
}
