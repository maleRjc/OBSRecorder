#include "obs_manager.h"

#include <memory>
#include <mutex>
#include <string>
#include <Windows.h>

#include "obs_common.h"
#include "obs_error.h"
#include "obs.hpp"


#define OBS_DIR_MODULE      ".\\obs-plugins"
#define OBS_DIR_MODULEDATA  ".\\data\\obs-plugins/%module%"
#define OBS_DIR_CONFIG      ".\\obs-cache"


// static
ObsManager* ObsManager::GetInstance() {
  static std::unique_ptr<ObsManager> g_ObsManager = nullptr;
  static std::once_flag g_Flag;
  std::call_once(g_Flag, [&]() { g_ObsManager.reset(new ObsManager()); });
  return g_ObsManager.get();
}

ObsManager::ObsManager() {
  HDC dc = ::GetDC(NULL);
  base_width_ = ::GetDeviceCaps(dc, HORZRES);
  base_height_ = ::GetDeviceCaps(dc, VERTRES);
  ::ReleaseDC(NULL, dc);
}

ObsManager::~ObsManager() {
  //obs_shutdown();
}

void ObsManager::Init() {
  //Init libobs.
  ObsStartUp();

  //Init audio thread.
  ResetAudioThread();

  //Init video thread.
  ResetVideoThread();

  LoadObsMoudle();
}

void ObsManager::ObsStartUp() {
  if (!obs_startup("zh-CN", OBS_DIR_CONFIG, NULL))
    throw INITIALIZE_LIBOBS_ERROR;
}

void ObsManager::ResetVideoThread() {
  int fps_num = common::kFPS;
  std::string first_render = DL_D3D11;
  std::string second_render = DL_OPENGL;

  // Adapt system.
  if (common::GetCPUCount() <= 2) {
    // first_render = DL_OPENGL;
    // second_render = DL_D3D11;
    fps_num = common::kLowFPS;
  }

  obs_video_info ovi;
  ovi.fps_num = fps_num;
  ovi.fps_den = 1;

  ovi.graphics_module = first_render.c_str();

  ovi.base_width = base_width_;
  ovi.base_height = base_height_;
  ovi.output_width = base_width_;
  ovi.output_height = base_height_;
  ovi.output_format = VIDEO_FORMAT_NV12;
  ovi.colorspace = VIDEO_CS_601;
  ovi.range = VIDEO_RANGE_PARTIAL;
  ovi.adapter = 0;
  ovi.gpu_conversion = true;
  ovi.scale_type = OBS_SCALE_BICUBIC;

  int ret = OBS_VIDEO_FAIL;

  ret = obs_reset_video(&ovi);

  if (ret != OBS_VIDEO_SUCCESS) {

    //if (ret == OBS_VIDEO_CURRENTLY_ACTIVE)
    //  MessageBoxW(0, L"Tried to reset when already active.", L"Warning", 0);

    /* Try OpenGL if DirectX fails on windows */
    if (ovi.graphics_module != second_render.c_str()) {
      //MessageBoxA(0, "Tried to use OpenGL to init video.",
      //            second_render.c_str(), 0);
      ovi.graphics_module = second_render.c_str();
      ret = obs_reset_video(&ovi);
    }
  }

  switch (ret) {
    case OBS_VIDEO_MODULE_NOT_FOUND:
      throw NOT_FOUND_MODULE_ERROR;
    case OBS_VIDEO_NOT_SUPPORTED:
      throw UNSUPPORTED_ERROR;
    case OBS_VIDEO_INVALID_PARAM:
      throw INVALID_PARAMETERS_ERROR;
    default:
      if (ret != OBS_VIDEO_SUCCESS)
        throw UNKNOWN_ERROR;
  }
}

void ObsManager::ResetAudioThread() {
  obs_audio_info ai;
  ai.samples_per_sec = 44100;
  ai.speakers = SPEAKERS_STEREO;

  if (!obs_reset_audio(&ai))
    throw INITIALIZE_AUDIO_ERROR;
}

void ObsManager::LoadObsMoudle() {
  // Adds a module search path to be used with obs_find_modules.
  obs_add_module_path(OBS_DIR_MODULE, OBS_DIR_MODULEDATA);
  // Automatically loads all modules from module paths.
  obs_load_all_modules();
  // Notifies modules that all modules have been loaded.
  obs_post_load_modules();
}

void ObsManager::ReSetVideoResolution(unsigned int x, unsigned int y) {
  if (base_width_ == x && base_height_ == y)
    return;

  if (obs_video_active()) {
    // MessageBoxW(0, L"Disable to reset size of video.", L"Warning", 0);
    return;
  }

  base_width_ = x;
  base_height_ = y;

  ResetVideoThread();
}
