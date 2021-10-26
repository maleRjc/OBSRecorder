#pragma once
#ifndef RECORDER_SRC_HELPER_SINGLETON_H_
#define RECORDER_SRC_HELPER_SINGLETON_H_

#include <memory>
#include <mutex>

template <class T>
class Singleton {
 public:
  static T* GetInstance() {
    static std::unique_ptr<T> g_SingleInstance = NULL;
    static std::once_flag g_Flag;
    std::call_once(g_Flag, [&]() { g_SingleInstance.reset(new T); });
    return g_SingleInstance.get();
  }

 protected:
  Singleton() {}
  virtual ~Singleton() = 0 {}
};

#endif  // RECORDER_SRC_HELPER_SINGLETON_H_

