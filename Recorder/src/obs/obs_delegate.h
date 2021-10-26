#pragma once
#ifndef RECORDER_SRC_OBS_OBS_DELEGATE_H_
#define RECORDER_SRC_OBS_OBS_DELEGATE_H_

class ObsDelegate {
 public:
  virtual void StartingRecordCallback() = 0;
  virtual void StartRecordCallback() = 0;
  virtual void PauseRecordCallback() = 0;
  virtual void ContinueRecordCallback() = 0;
  virtual void StoppingRecordCallback() = 0;
  virtual void StopRecordCallback() = 0;
  virtual void DeativceOuputCallback() = 0;
};

#endif  // RECORDER_SRC_OBS_OBS_DELEGATE_H_