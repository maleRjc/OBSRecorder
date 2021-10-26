#pragma once
#ifndef RECORDER_SRC_CONTROLLER_H_
#define RECORDER_SRC_CONTROLLER_H_

#include <memory>
#include <string>
#include <Windows.h>

class CWndShadow;

class Controller {
 public:
  enum class OutputType {
    kGif,
    kMp4,
    kCount,
  };

  enum class TipType {
    kTimeOutLimited,
    kLowDiskWhenRecording,
    kLowDisk,
    kOther,
    kCount,
  };

  static Controller* GetInstance();
  static bool InitRunEnvironment(HINSTANCE instance);
  static void MessageLoop();
  static void ShowMessageBox(TipType type, std::string info = "");

  ~Controller();
  void End();

  bool IsInit() const { return init_; }
  void SetSaveFileDir(std::string path) { file_save_dir_ = path; }
  std::string GetSaveFileDir();
  void SetCaptureRect(const RECT& rect);
  RECT GetCaptureRect() const { return capture_rect_; }
  void ShowRecorderWindow(bool show = true);
  void ShowCaptureWindow(bool show = true);
  void SetOutputType(OutputType type) { output_tpye_ = type; }
  OutputType GetOutputType() const { return output_tpye_; }

  void FinishRecordCallback();
  void StartRecordCallback();
  void PauseRecordCallback();
  void ContinueRecordCallback();

  bool IsPauseRecord();
  bool IsRecording();
  bool StartRecord();
  void PauseRecord();
  void ContinueRecord();
  void StopRecord(bool remove_file = false);
  void AudioInputRecordMute(bool mute);
  void AudioOutputRecordMute(bool mute);
  void CropRecordRect(RECT rect);
  bool CheckLowDiskSpace();

 protected:
  Controller();
  void ResetRectBeforeRecording(RECT rect);
  void HandleCmdLine();

 private:
  bool init_ = false;
  bool stop_to_remove_file_ = false;
  OutputType output_tpye_ = OutputType::kMp4;
  RECT capture_rect_;
  std::string file_save_dir_;
  std::string file_;
};

#endif  // RECORDER_SRC_CONTROLLER_H_