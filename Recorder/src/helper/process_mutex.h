#pragma once
#ifndef RECORDER_SRC_HELPER_PROCESS_MUTEX_H_
#define RECORDER_SRC_HELPER_PROCESS_MUTEX_H_

class ProcessMutex {
 public:
  ProcessMutex();
  ~ProcessMutex();

  bool IsSuccess() const { return is_success_; }

 protected:
  bool InitRecorderEnvironment();
  void UninitRecorderEnvironment();

 private:
  bool is_success_;
};

#endif  // RECORDER_SRC_HELPER_PROCESS_MUTEX_H_
