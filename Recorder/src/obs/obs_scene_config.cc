#include "obs_scene_config.h"

#include <cstdio>
#include "obs_setting_args.h"

#define MAX_NAME 260

namespace {
constexpr char kMonitorName[] = ":monitor";
constexpr char kAudioInputName[] = ":microphone";
constexpr char kAudioOutputName[] = ":loudspeaker";
}

ObsSceneConfig::ObsSceneConfig(std::string scene_name) {
  InitScene(scene_name);
}

ObsSceneConfig::~ObsSceneConfig() {}

void ObsSceneConfig::CropMonitor(int left, int top, int right, int bottom) {
  CropItem(monitor_, left, top, right, bottom);
}

void ObsSceneConfig::AudioInputMute(bool mute) {
  obs_source_set_muted(microphone_.first, mute);
}

void ObsSceneConfig::AudioOutputMute(bool mute) {
  obs_source_set_muted(loudspeaker_.first, mute);
}

void ObsSceneConfig::AddMonitor() {
  std::string name = obs_source_get_name(obs_scene_get_source(obs_scene_));
  name += kMonitorName;
  monitor_ = AddSource(OBS_SOURCE_TYPE_SCREEN, name);
}

void ObsSceneConfig::AddAudioInputDevice() {
  std::string name = obs_source_get_name(obs_scene_get_source(obs_scene_));
  name += kAudioInputName;
  microphone_ = AddSource(OBS_SOURCE_TYPE_INDEVICE_A, name);
}

void ObsSceneConfig::AddAudioOutputDevice() {
  std::string name = obs_source_get_name(obs_scene_get_source(obs_scene_));
  name += kAudioOutputName;
  loudspeaker_ = AddSource(OBS_SOURCE_TYPE_OUTDEVICE_A, name);
}

void ObsSceneConfig::InitScene(std::string name) {
  obs_scene_ = obs_scene_create(name.c_str());
  AddMonitor();
  AddAudioInputDevice();
  AddAudioOutputDevice();
  obs_set_output_source(output_channel_, obs_scene_get_source(obs_scene_));
}

ObsSceneConfig::Item ObsSceneConfig::AddSource(std::string type,
                                               std::string name) {
  OBSSource source =
      obs_source_create(type.c_str(), name.c_str(), nullptr, nullptr);
  OBSSceneItem sceneitem;
  if (source) {
    sceneitem = obs_scene_add(obs_scene_, source);
    obs_sceneitem_set_visible(sceneitem, true);
  }
  return ObsSceneConfig::Item(source, sceneitem);
}

void ObsSceneConfig::CropItem(const Item& item,
                              int left,
                              int top,
                              int right,
                              int bottom) {
  uint32_t width = obs_source_get_width(item.first);
  uint32_t height = obs_source_get_height(item.first);
  if (item.second) {
    obs_sceneitem_crop crop;
    crop.left = left;
    crop.right = width - right;
    crop.top = top;
    crop.bottom = height - bottom;
    obs_sceneitem_set_crop(item.second, &crop);
  }
}
