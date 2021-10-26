#include "controller.h"

#include <algorithm>
#include <cstdio>
#include <mutex>
#include <shlobj.h>

#include "helper/util.h"

#include "obs/obs_error.h"
#include "obs/obs_manager.h"
#include "obs/obs_recorder.h"

#include "ui/capture_wnd.h"
#include "ui/def.h"
#include "ui/recorder_wnd.h"

namespace {
constexpr int kMinResolutionSqrt = VIDEO_MIN_RESOLUTION_SQRT;
constexpr char kCmdArgsIdKey[] = "--id=";
constexpr char kCmdArgsIdValue[] = "L5CLmX9sVMlp1uVq7EKESxo2KElZC7SvLz416eeQ";
constexpr char kCmdArgsSuffix[] = "--suffix=";
constexpr char kCmdArgsSavePath[] = "--savingpath=";

void InitCurrentDirectory() {
  wchar_t cur_dir[MAX_PATH] = {0};
  ::GetModuleFileNameW(NULL, cur_dir, MAX_PATH);
  wchar_t* insert_pos = wcsrchr(cur_dir, L'\\');
  wcscpy_s(insert_pos + 1, wcslen(insert_pos), L"\0");
  ::SetCurrentDirectoryW(cur_dir);
}

bool IsPathExist(const std::string& path) {
  WIN32_FILE_ATTRIBUTE_DATA attrs = {0};
  return 0 != GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attrs);
}

std::string GetSuffixStr(Controller::OutputType suffix) {
  std::string suffix_str;
  switch (suffix) {
    case Controller::OutputType::kGif:
      suffix_str = ".gif";
      break;
    case Controller::OutputType::kMp4:
      suffix_str = ".mp4";
      break;
  }
  return suffix_str;
}

std::string GetOutFileName(Controller::OutputType suffix) {
  SYSTEMTIME system_time;
  ::GetSystemTime(&system_time);
  char file_name[MAX_PATH];
  sprintf_s(file_name, MAX_PATH, "%04d%02d%02d%02d%02d%02d", system_time.wYear,
            system_time.wMonth, system_time.wDay, system_time.wHour + 8,
            system_time.wMinute, system_time.wSecond);

  return std::string(file_name) + GetSuffixStr(suffix);
}

std::string GetParameter(LPCSTR param, LPCSTR cmd) {
  std::string command(cmd);
  std::string parameter(param);
  int param_pos = command.rfind(param);
  if (param_pos < 0)
    return "";
  int param_end = command.find(" --", param_pos);
  if (param_end == std::string::npos)
    param_end = command.length();
  return command.substr(param_pos + parameter.length(),
                        param_end - param_pos - parameter.length());
}

}

// static
Controller* Controller::GetInstance() {
  static std::unique_ptr<Controller> g_WinManagerInstance = nullptr;
  static std::once_flag g_Flag;
  std::call_once(g_Flag, [&]() { g_WinManagerInstance.reset(new Controller); });
  return g_WinManagerInstance.get();
}

// static
bool Controller::InitRunEnvironment(HINSTANCE instance) {
  DuiLib::CPaintManagerUI::SetInstance(instance);
  DuiLib::CPaintManagerUI::SetResourcePath(
      DuiLib::CPaintManagerUI::GetInstancePath() + RESOURCE_PATH);
  InitCurrentDirectory();

  try {
    ObsManager::GetInstance()->Init();  
  } catch (const char* error) {
    ShowMessageBox(TipType::kOther, error);
    return false;
  }
  return true;
}

// static
void Controller::MessageLoop() {
  DuiLib::CPaintManagerUI::MessageLoop();
}

Controller::Controller() : capture_rect_(recorder_util::GetDesktopWindow()) {
  HandleCmdLine();
}

void Controller::ResetRectBeforeRecording(RECT rect) {
  try {
    ObsManager::GetInstance()->ReSetVideoResolution(rect.right - rect.left,
                                                    rect.bottom - rect.top);
  } catch (const char* error) {
    ShowMessageBox(TipType::kOther, error);
  }
  CropRecordRect(rect);
}

void Controller::HandleCmdLine() {
  LPSTR command = ::GetCommandLineA();
  init_ = kCmdArgsIdValue == GetParameter(kCmdArgsIdKey, command);

  std::string suffix = GetParameter(kCmdArgsSuffix, command);
  transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
  if (suffix == "gif")
    SetOutputType(OutputType::kGif);
  else if (suffix == "mp4")
    SetOutputType(OutputType::kMp4);

  std::string save_path = GetParameter(kCmdArgsSavePath, command);
  if (!save_path.empty()) {
    int cb = save_path.find("\"");
    int ce = save_path.rfind("\"");
    if (cb == 0 && ce == save_path.length() - 1)
      save_path = save_path.substr(cb + 1, ce - 1);
    SetSaveFileDir(save_path);
  }
}

Controller::~Controller() {
  End();
}

void Controller::End() {
  /* ObsCallback had came to main thread(output is end),
   * but obs don't give up permissions of outfiles(maybe obs's bug?).
   * As a result, sleep sometime in the end of process. */
  if (stop_to_remove_file_) {
    ::Sleep(2000);
    ::DeleteFileA(file_.c_str());
  } else if (IsPathExist(file_)) {
    ::Sleep(500); // Can't open explorer sometime.
    ::ShellExecuteA(NULL, "open", "explorer.exe",
                    std::string("/select," + file_).c_str(), NULL,
                    SW_SHOWNORMAL);
  }
}

std::string Controller::GetSaveFileDir() {
  // Fix: Not exist file path.
  if (!file_save_dir_.empty() && !IsPathExist(file_save_dir_)) {
    ::CreateDirectoryA(file_save_dir_.c_str(), NULL);
  }

  if (file_save_dir_.empty() || !IsPathExist(file_save_dir_)) {
    char file_path[MAX_PATH];
    if (!::SHGetSpecialFolderPathA(::GetDesktopWindow(), file_path,
                                   CSIDL_MYVIDEO, 0)) {
      ::SHGetSpecialFolderPathA(::GetDesktopWindow(), file_path,
                                CSIDL_DESKTOPDIRECTORY, 0);
    }
    file_save_dir_ = std::string(file_path);
  }
  return file_save_dir_;
}

void Controller::SetCaptureRect(const RECT& rect) {
  capture_rect_ = rect;

  // Min value is kMinResolutionSqrt.
  if (capture_rect_.right - capture_rect_.left < kMinResolutionSqrt)
    capture_rect_.right = capture_rect_.left + kMinResolutionSqrt;
  if (capture_rect_.bottom - capture_rect_.top < kMinResolutionSqrt)
    capture_rect_.bottom = capture_rect_.top + kMinResolutionSqrt;

  ResetRectBeforeRecording(capture_rect_);
}

void Controller::ShowRecorderWindow(bool show) {
  RecorderWnd::GetInstance()->ShowWindow(show);
}

void Controller::ShowCaptureWindow(bool show) {
  CaptureWnd::GetInstance()->ShowWindow(show);
}

void Controller::FinishRecordCallback() {
  RecorderWnd::GetInstance()->Close();
}

void Controller::StartRecordCallback() {
  RecorderWnd::GetInstance()->StartRecordCallback();
}

void Controller::PauseRecordCallback() {
  RecorderWnd::GetInstance()->PauseRecordCallback();
}

void Controller::ContinueRecordCallback() {
  RecorderWnd::GetInstance()->ContinueRecordCallback();
}

bool Controller::IsPauseRecord() {
  return ObsRecorder::GetInstance()->IsPause();
}

bool Controller::IsRecording() {
  return ObsRecorder::GetInstance()->IsActive();
}

bool Controller::StartRecord() {
  RecorderWnd* recorderwnd = RecorderWnd::GetInstance();
  recorderwnd->SetTriggerSizeBox(false);

  RECT real_record = recorderwnd->GetRecordRealRect();
  if (!recorder_util::IsEqualRect(real_record, capture_rect_))
    ResetRectBeforeRecording(real_record);

  file_ = GetSaveFileDir() + "\\" + GetOutFileName(output_tpye_);
  if (CheckLowDiskSpace()) {
    ShowMessageBox(TipType::kLowDisk);
    return false;
  }
  ObsRecorder* obs_recorder = ObsRecorder::GetInstance();
  obs_recorder->SetOutPath(file_);
  return obs_recorder->Start();
}

void Controller::PauseRecord() {
  ObsRecorder::GetInstance()->Pause();
}

void Controller::ContinueRecord() {
  ObsRecorder::GetInstance()->Continue();
}

void Controller::StopRecord(bool remove_file) {
  stop_to_remove_file_ = remove_file;
  ObsRecorder::GetInstance()->Stop();
}

void Controller::AudioInputRecordMute(bool mute) {
  ObsRecorder::GetInstance()->AudioInputMute(mute);
}

void Controller::AudioOutputRecordMute(bool mute) {
  ObsRecorder::GetInstance()->AudioOutputMute(mute);
}

void Controller::CropRecordRect(RECT rect) {
  ObsRecorder::GetInstance()->CropRect(rect.left, rect.top, rect.right,
                                       rect.bottom);
}

bool Controller::CheckLowDiskSpace() {
  return ObsRecorder::GetInstance()->LowDiskSpace(GetSaveFileDir());
}

void Controller::ShowMessageBox(TipType type, std::string info) {
  RecorderWnd::GetInstance()->SetEnableAllWindows(false);
  switch (type) {
    case TipType::kLowDisk:
      ::MessageBoxA(RecorderWnd::GetInstance()->GetHWND(), LOW_DISK_ERROR,
                    ERROR_TITLE, MB_SYSTEMMODAL);
      break;
    case TipType::kLowDiskWhenRecording:
      ::MessageBoxA(RecorderWnd::GetInstance()->GetHWND(),
                    LOW_DISK_WHEN_RECORDING_ERROR, ERROR_TITLE, MB_SYSTEMMODAL);
      break;
    case TipType::kTimeOutLimited:
      ::MessageBoxA(RecorderWnd::GetInstance()->GetHWND(),
                    TIMEOUT_LIMITED_ERROR, ERROR_TITLE, MB_SYSTEMMODAL);
      break;
    case TipType::kOther:
      ::MessageBoxA(RecorderWnd::GetInstance()->GetHWND(), info.c_str(),
                    ERROR_TITLE, MB_SYSTEMMODAL);
      break;
  }
  RecorderWnd::GetInstance()->SetEnableAllWindows(true);
}
