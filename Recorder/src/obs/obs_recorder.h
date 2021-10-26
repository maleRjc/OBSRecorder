#pragma once
#ifndef RECORDER_SRC_OBS_OBS_RECORDER_H_
#define RECORDER_SRC_OBS_OBS_RECORDER_H_

#include <memory>
#include <string>

#include "obs_delegate.h"

class ObsFFmpegOutput;
class ObsSceneConfig;

class ObsRecorder : public ObsDelegate {
 public:
  static ObsRecorder* GetInstance();

  ~ObsRecorder();

  void SetOutPath(std::string filename);
  bool LowDiskSpace(std::string path);

  bool IsPause();
  bool IsActive();
  bool Start();
  void Pause();
  void Continue();
  void Stop();

  void AudioInputMute(bool mute);
  void AudioOutputMute(bool mute);

  void CropRect(int left, int top, int right, int bottom);

  // Override ObsDelegate.
  void StartingRecordCallback() override;
  void StartRecordCallback() override;
  void PauseRecordCallback() override;
  void ContinueRecordCallback() override;
  void StoppingRecordCallback() override;
  void StopRecordCallback() override;
  void DeativceOuputCallback() override;

 protected:
  ObsRecorder();

 private:
  std::unique_ptr<ObsFFmpegOutput> obs_ffmpeg_output_;
  std::unique_ptr<ObsSceneConfig> obs_scene_config_;
};

#endif  // RECORDER_SRC_OBS_OBS_RECORDER_H_