#include "obs_recorder.h"

#include <mutex>

#include "../controller.h"

#include "obs_ffmpeg_output.h"
#include "obs_scene_config.h"

// From obs.
#include "util/platform.h"


#define MBYTE (1024ULL * 1024ULL)
#define MBYTES_LEFT_STOP_REC 50ULL
#define MAX_BYTES_LEFT (MBYTES_LEFT_STOP_REC * MBYTE)


namespace {
constexpr char kOutputName[] = "ffmpeg-output-name";
constexpr char kSceneName[] = "record-scene-name";
}


ObsRecorder* ObsRecorder::GetInstance() {
  static std::unique_ptr<ObsRecorder> g_ObsRecorderInstance = nullptr;
  static std::once_flag g_Flag;
  std::call_once(g_Flag,
                 [&]() { g_ObsRecorderInstance.reset(new ObsRecorder); });
  return g_ObsRecorderInstance.get();
}

ObsRecorder::~ObsRecorder() {}

void ObsRecorder::SetOutPath(std::string filename) {
  if (obs_ffmpeg_output_)
    obs_ffmpeg_output_->SetOutPath(filename);
}

bool ObsRecorder::LowDiskSpace(std::string path) {
  uint64_t num_bytes = os_get_free_disk_space(path.c_str());

  if (num_bytes < (MAX_BYTES_LEFT))
    return true;
  else
    return false;
}

bool ObsRecorder::IsPause() {
  if (obs_ffmpeg_output_)
    return obs_ffmpeg_output_->IsPause();
  return false;
}

bool ObsRecorder::IsActive() {
  if (obs_ffmpeg_output_)
    return obs_ffmpeg_output_->IsActive();
  return false;
}

bool ObsRecorder::Start() {
  if (obs_ffmpeg_output_)
    return obs_ffmpeg_output_->Start();
  return false;
}

void ObsRecorder::Pause() {
  if (obs_ffmpeg_output_)
    obs_ffmpeg_output_->Pause();
}

void ObsRecorder::Continue() {
  if (obs_ffmpeg_output_)
    obs_ffmpeg_output_->Continue();
}

void ObsRecorder::Stop() {
  if (obs_ffmpeg_output_)
    obs_ffmpeg_output_->Stop(true);
}

void ObsRecorder::AudioInputMute(bool mute) {
  if (obs_scene_config_)
    obs_scene_config_->AudioInputMute(mute);
}

void ObsRecorder::AudioOutputMute(bool mute) {
  if (obs_scene_config_)
    obs_scene_config_->AudioOutputMute(mute);
}

void ObsRecorder::CropRect(int left, int top, int right, int bottom) {
  if (obs_ffmpeg_output_)
    obs_ffmpeg_output_->ResetQuality();
  if (obs_scene_config_)
    obs_scene_config_->CropMonitor(left, top, right, bottom);
}

void ObsRecorder::StartingRecordCallback() {
  Controller::GetInstance()->StartRecordCallback();
}

void ObsRecorder::StartRecordCallback() {}

void ObsRecorder::PauseRecordCallback() {
  Controller::GetInstance()->PauseRecordCallback();
}

void ObsRecorder::ContinueRecordCallback() {
  Controller::GetInstance()->ContinueRecordCallback();
}

void ObsRecorder::StoppingRecordCallback() {}

void ObsRecorder::StopRecordCallback() {}

void ObsRecorder::DeativceOuputCallback() {
  Controller::GetInstance()->FinishRecordCallback();
}

ObsRecorder::ObsRecorder() {
  obs_ffmpeg_output_.reset(new ObsFFmpegOutput(kOutputName, this));
  obs_scene_config_.reset(new ObsSceneConfig(kSceneName));
}
