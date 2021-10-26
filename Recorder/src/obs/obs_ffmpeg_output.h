#pragma once
#ifndef RECORDER_SRC_OBS_OBS_FFMPEG_OUTPUT_H_
#define RECORDER_SRC_OBS_OBS_FFMPEG_OUTPUT_H_

#include <string>

#include "obs.hpp"

class ObsDelegate;

class ObsFFmpegOutput {
 public:
  enum class Quality {
    kNone = 0,
    kVeryLow,
    kLow,
    kNormal,
    kHigh,
    kLowLoss,
    kNoLoss
  };

  ObsFFmpegOutput(std::string output_name, ObsDelegate* delegate);
  ~ObsFFmpegOutput();

  // It's a filename with suffix.
  bool SetOutPath(std::string output_path);
  // Video quality.
  bool SetQuality(Quality quality);
  void ResetQuality();
  double GetVideoQuality(Quality quality);
  int GetAudioQuality(Quality quality);

  bool IsActive();
  bool IsPause() { return pause_; }

  // Operator output-thread, generate files to disk.
  bool Start();
  bool Start(std::string output_path);
  bool Stop(bool is_force);
  bool Pause();
  bool Continue();

 protected:
  void CreateOutput(std::string output_name);
  void InitOutputDefaultParam();
  void InitCallback();
  void InitQuality();

 private:
  bool pause_ = false;
  Quality quality_ = Quality::kVeryLow;

  ObsDelegate* delegate_;

  // Callback signal.
  OBSSignal obs_signal_start_;
  OBSSignal obs_signal_starting_;
  OBSSignal obs_signal_stop_;
  OBSSignal obs_signal_stopping_;
  OBSSignal obs_signal_pause_;
  OBSSignal obs_signal_continue_;
  OBSSignal obs_signal_deative_;

  OBSOutput obs_output_;

  std::string output_path_;
};

#endif  // RECORDER_SRC_OBS_OBS_FFMPEG_OUTPUT_H_