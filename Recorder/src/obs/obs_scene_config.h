#pragma once
#ifndef RECORDER_SRC_OBS_OBS_SCENE_CONFIG_H_
#define RECORDER_SRC_OBS_OBS_SCENE_CONFIG_H_

#include <string>
#include "obs.hpp"

class ObsSceneConfig {
 public:
  typedef std::pair<OBSSource, OBSSceneItem> Item;

  ObsSceneConfig(std::string scene_name);
  ~ObsSceneConfig();

  void CropMonitor(int left, int top, int right, int bottom);
  void AudioInputMute(bool mute);
  void AudioOutputMute(bool mute);

 protected:
  void AddMonitor();
  void AddAudioInputDevice();
  void AddAudioOutputDevice();

  void InitScene(std::string name);
  Item AddSource(std::string type, std::string name);
  void CropItem(const Item& item, int left, int top, int right, int bottom);

 private:
  int output_channel_ = 0;
  OBSScene obs_scene_;
  Item monitor_;
  Item microphone_;
  Item loudspeaker_;
};

#endif  // RECORDER_SRC_OBS_OBS_SCENE_CONFIG_H_