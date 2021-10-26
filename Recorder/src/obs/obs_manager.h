#pragma once
#ifndef RECORDER_SRC_OBS_OBS_MANAGER_H_
#define RECORDER_SRC_OBS_OBS_MANAGER_H_

class ObsManager {
 public:
  static ObsManager* GetInstance();

  ~ObsManager();

  void Init();
  void ObsStartUp();
  void ResetVideoThread();
  void ResetAudioThread();
  void LoadObsMoudle();

  void ReSetVideoResolution(unsigned int x, unsigned int y);

 private:
  ObsManager();

  unsigned int base_height_;
  unsigned int base_width_;
};

#endif  // RECORDER_SRC_OBS_OBS_MANAGER_H_