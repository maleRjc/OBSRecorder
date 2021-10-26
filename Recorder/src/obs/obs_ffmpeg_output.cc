#include "obs_ffmpeg_output.h"

#include <Windows.h>

#include "obs_common.h"
#include "obs_delegate.h"
#include "obs_setting_args.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif

namespace {
void StartingRecordCallback(void* data, calldata_t* params);
void StartRecordCallback(void* data, calldata_t* params);
void PauseRecordCallback(void* data, calldata_t* params);
void ContinueRecordCallback(void* data, calldata_t* params);
void StoppingRecordCallback(void* data, calldata_t* params);
void StopRecordCallback(void* data, calldata_t* params);
void DeativceOuputCallback(void* data, calldata_t* params);

std::string GetFileFormat(std::string path) {
  return path.substr(path.rfind(".") + 1);
}

}

ObsFFmpegOutput::ObsFFmpegOutput(std::string output_name, ObsDelegate* delegate)
    : delegate_(delegate) {
  CreateOutput(output_name);
  InitCallback();
}

ObsFFmpegOutput::~ObsFFmpegOutput() {}

bool ObsFFmpegOutput::SetOutPath(std::string output_path) {
  if (!obs_output_)
    return false;

  AVFormatContext* av_format_context = NULL;
  avformat_alloc_output_context2(&av_format_context, NULL, NULL,
                                 output_path.c_str());
  if (!av_format_context)
    return false;

  AVCodec* av_codec_video = NULL;
  AVCodec* av_codec_audio = NULL;
  av_codec_video =
      avcodec_find_encoder(av_format_context->oformat->video_codec);
  av_codec_audio =
      avcodec_find_encoder(av_format_context->oformat->audio_codec);

  OBSData output_setting = obs_data_create();
  obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_FMT,
                      av_format_context->oformat->mime_type);
  // Set output path.
  obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_URL,
                      output_path.c_str());
  // Set suffix.
  obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_FORMAT,
                      GetFileFormat(output_path).c_str());

  // Video Encoder.
  if (av_codec_video) {
    obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_ENCNAME_V,
                        av_codec_video->name);
    obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_ENCID_V,
                     av_codec_video->id);
  } else {
    obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_ENCNAME_V, "");
    obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_ENCID_V,
                     AV_CODEC_ID_NONE);
  }

  // Audio Encoder.
  if (av_codec_audio) {
    obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_ENCNAME_A,
                        av_codec_audio->name);
    obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_ENCID_A,
                     av_codec_audio->id);
  } else {
    obs_data_set_string(output_setting, OBS_OUTPUT_ATTRNAME_ENCNAME_A, "");
    obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_ENCID_A,
                     AV_CODEC_ID_NONE);
  }

  obs_output_update(obs_output_, output_setting);

  output_path_ = output_path;
  return true;
}

bool ObsFFmpegOutput::SetQuality(Quality quality) {
  //if (!obs_output_)
  //  return false;

  //if (quality_ == quality)
  //  return false;
  //quality_ = quality;
  //double video_quality_value = GetVideoQuality(quality_);
  //int audio_quality_value = GetAudioQuality(quality_);

  //obs_video_info video_info;
  //obs_get_video_info(&video_info);

  // bitrate_video's unit is kbps, so value / 1000.
  //int bitrate_video = video_info.output_width * video_info.output_height *
  //                    video_quality_value / 1000;
  //bitrate_video = bitrate_video > 12500 ? 12500 : bitrate_video;
  //int bitrate_audio = audio_quality_value;

  //OBSData output_setting = obs_data_create();
  //obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_BIRATE_A, bitrate_audio);
  //obs_data_set_int(output_setting, OBS_OUTPUT_ATTRNAME_BIRATE_V, bitrate_video);
  //obs_output_update(obs_output_, output_setting);

  //OBSData output_setting = obs_output_get_settings(obs_output_);
  //int a = obs_data_get_int(output_setting, OBS_OUTPUT_ATTRNAME_BIRATE_A);
  //int b = obs_data_get_int(output_setting, OBS_OUTPUT_ATTRNAME_BIRATE_V);

  return true;
}

void ObsFFmpegOutput::ResetQuality() {
  // Reset if output-video's bound changed.
  SetQuality(quality_);
}

double ObsFFmpegOutput::GetVideoQuality(Quality quality) {
  double quality_value = 1;
  switch (quality) {
    case Quality::kVeryLow:
      quality_value /= 4;
      break;
    case Quality::kLow:
      quality_value = 1;
      break;
    case Quality::kNormal:
      quality_value = 2;
      break;
    case Quality::kHigh:
      quality_value = 4;
      break;
    case Quality::kLowLoss:
      quality_value = 6;
      break;
    case Quality::kNoLoss:
      quality_value = 8;
      break;
    default:
      break;
  }
  return quality_value;
}

int ObsFFmpegOutput::GetAudioQuality(Quality quality) {
  int quality_value = 1;
  switch (quality) {
    case Quality::kVeryLow:
      quality_value = common::kAS / 4;
      break;
    case Quality::kLow:
      quality_value = common::kAS / 2;
      break;
    case Quality::kNormal:
      quality_value = common::kAS;
      break;
    case Quality::kHigh:
    case Quality::kLowLoss:
      quality_value = common::kAS * 2;
      break;
    case Quality::kNoLoss:
      quality_value = common::kNormalAS;
      break;
    default:
      break;
  }
  return quality_value;
}

bool ObsFFmpegOutput::IsActive() {
  return obs_output_active(obs_output_);
}

bool ObsFFmpegOutput::Start() {
  if (obs_output_ && !output_path_.empty() && !IsActive()) {
    if (obs_output_start(obs_output_))
      return true;

    //std::string error = obs_output_get_last_error(obs_output_);
    //MessageBoxA(0, error.c_str(), "Error", 0);
  }
  return false;
}

bool ObsFFmpegOutput::Start(std::string output_path) {
  if (SetOutPath(output_path))
    return Start();
  return false;
}

bool ObsFFmpegOutput::Stop(bool is_force) {
  if (IsActive()) {
    if (is_force)
      obs_output_force_stop(obs_output_);
    else
      obs_output_stop(obs_output_);
    return true;
  }
  return false;
}

bool ObsFFmpegOutput::Pause() {
  pause_ = true;
  return obs_output_pause(obs_output_, pause_);
}

bool ObsFFmpegOutput::Continue() {
  pause_ = false;
  return obs_output_pause(obs_output_, pause_);
}

void ObsFFmpegOutput::CreateOutput(std::string output_name) {
  obs_output_ = obs_output_create(OBS_OUTPUT_TYPE_ALLFFMPEG, output_name.c_str(),
                                  nullptr, nullptr);
  obs_output_can_pause(obs_output_);
  InitOutputDefaultParam();
}

void ObsFFmpegOutput::InitOutputDefaultParam() {
  OBSData obs_setting = obs_data_create();
  obs_data_set_int(obs_setting, OBS_OUTPUT_ATTRNAME_GOPSIZE, 20);
  obs_output_update(obs_output_, obs_setting);
  obs_output_set_mixers(obs_output_, 1);

  InitQuality();
  SetQuality(quality_);
}

void ObsFFmpegOutput::InitCallback() {
  obs_signal_starting_.Connect(obs_output_get_signal_handler(obs_output_),
                               "starting", StartingRecordCallback, delegate_);
  obs_signal_start_.Connect(obs_output_get_signal_handler(obs_output_), "start",
                            StartRecordCallback, delegate_);
  obs_signal_pause_.Connect(obs_output_get_signal_handler(obs_output_), "pause",
                            PauseRecordCallback, delegate_);
  obs_signal_continue_.Connect(obs_output_get_signal_handler(obs_output_),
                               "unpause", ContinueRecordCallback, delegate_);
  obs_signal_stop_.Connect(obs_output_get_signal_handler(obs_output_), "stop",
                           StopRecordCallback, delegate_);
  obs_signal_stopping_.Connect(obs_output_get_signal_handler(obs_output_),
                               "stopping", StoppingRecordCallback, delegate_);
  obs_signal_deative_.Connect(obs_output_get_signal_handler(obs_output_),
                              "deactivate", DeativceOuputCallback, delegate_);
}

void ObsFFmpegOutput::InitQuality() {
  switch (common::GetCPUCount()) {
    case 0:
    case 1:
    case 2:
      quality_ = Quality::kVeryLow;
      break;
    case 4:
      quality_ = Quality::kLow;
      break;
    case 6:
      quality_ = Quality::kNormal;
      break;
    case 8:
      quality_ = Quality::kHigh;
      break;
    case 16:
      quality_ = Quality::kLowLoss;
      break;
    case 32:
    case 64:
    case 128:
    case 256:
    case 512:
    case 1048:
      quality_ = Quality::kNoLoss;
      break;
    default:
      // ???
      quality_ = Quality::kNormal;
      break;
  }
}


namespace {
void StartingRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->StartingRecordCallback();
}
void StartRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->StartRecordCallback();
}
void PauseRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->PauseRecordCallback();
}
void ContinueRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->ContinueRecordCallback();
}
void StoppingRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->StoppingRecordCallback();
}
void StopRecordCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->StopRecordCallback();
}
void DeativceOuputCallback(void* data, calldata_t* params) {
  ObsDelegate* delegate = static_cast<ObsDelegate*>(data);
  delegate->DeativceOuputCallback();
}

}  // namespace