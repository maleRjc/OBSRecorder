#pragma once
#ifndef RECORDER_SRC_OBS_OBS_ERROR_H_
#define RECORDER_SRC_OBS_OBS_ERROR_H_


#ifdef _DEBUG

#define ERROR_TITLE "Tip"

#define INITIALIZE_LIBOBS_ERROR "Failed to initialize libobs."
#define INITIALIZE_AUDIO_ERROR "Failed to initialize audio."

#define NOT_FOUND_MODULE_ERROR \
  "Failed to initialize video:  Graphics module not found."
#define INVALID_PARAMETERS_ERROR \
  "Failed to initialize video:  Invalid parameters."
#define UNSUPPORTED_ERROR                                               \
  "Failed to initialize video:\n\nRequired graphics API functionality " \
  "not found.  Your GPU may not be supported."
#define UNKNOWN_ERROR                                            \
  "Failed to initialize video.  Your GPU may not be supported, " \
  "or your graphics drivers may need to be updated."

#define LOW_DISK_WHEN_RECORDING_ERROR \
  "Recording stopped because of low disk space."
#define LOW_DISK_ERROR "Low disk space."

#define TIMEOUT_LIMITED_ERROR "Recording stopped because time out limited."

#else

#define ERROR_TITLE "提示"

#define INITIALIZE_LIBOBS_ERROR "初始化依赖库失败。"
#define INITIALIZE_AUDIO_ERROR "初始化音频失败。"

#define NOT_FOUND_MODULE_ERROR "初始化视频失败：没有找到图形模块。"
#define INVALID_PARAMETERS_ERROR "初始化视频失败：无效参数。"
#define UNSUPPORTED_ERROR                                    \
  "初始化视频失败：未找到图形功能接口。\n" \
  "GPU可能不支持此功能。"
#define UNKNOWN_ERROR                                  \
  "初始化视频失败：GPU不支持此功能。\n" \
  "驱动设备可能需要更新。"

#define LOW_DISK_WHEN_RECORDING_ERROR "磁盘空间不足，已停止录制。"
#define LOW_DISK_ERROR "磁盘空间不足。"

#define TIMEOUT_LIMITED_ERROR "录制超时，限制时间为1小时。"

#endif


#endif  // RECORDER_SRC_OBS_OBS_ERROR_H_