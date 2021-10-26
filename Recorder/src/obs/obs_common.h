#pragma once
#ifndef RECORDER_SRC_OBS_OBS_COMMON_H_
#define RECORDER_SRC_OBS_OBS_COMMON_H_

namespace common {

extern const int kFPS;
extern const int kLowFPS;
extern const int kAS;
extern const int kNormalAS;

int GetCPUCount();

bool __stdcall IsOSWin10();

bool __stdcall IsOSWin7();
}

#endif  // RECORDER_SRC_OBS_OBS_COMMON_H_



