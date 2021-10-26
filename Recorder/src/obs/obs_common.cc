#include "obs_common.h"

#include <Windows.h>

const int common::kFPS = 30;
const int common::kLowFPS = 15;
const int common::kAS = 64;
const int common::kNormalAS = 160;

int common::GetCPUCount() {
  SYSTEM_INFO SysemInfo;
  ::GetSystemInfo(&SysemInfo);
  return SysemInfo.dwNumberOfProcessors;
}

bool WINAPI common::IsOSWin10() {
  typedef void(__stdcall * NTPROC)(DWORD*, DWORD*, DWORD*);
  HINSTANCE hinst = LoadLibraryW(L"ntdll.dll");
  DWORD dw_major, dw_minor, dw_build_number;
  NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
  proc(&dw_major, &dw_minor, &dw_build_number);
  if (dw_major == 10 && dw_minor == 0)
    return true;
  return false;
}

bool __stdcall common::IsOSWin7() {
  typedef void(__stdcall * NTPROC)(DWORD*, DWORD*, DWORD*);
  HINSTANCE hinst = LoadLibraryW(L"ntdll.dll");
  DWORD dw_major, dw_minor, dw_build_number;
  NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
  proc(&dw_major, &dw_minor, &dw_build_number);
  if (dw_major == 6 && dw_minor == 1)
    return true;
  return false;
}
