#pragma once

#include <ctime>

#include "UIlib.h"

class ToolbarTimeButton : public DuiLib::CButtonUI {
 public:
  ToolbarTimeButton();

  // Override DuiLib::CButtonUI.
  LPCTSTR GetClass() const override;
  LPVOID GetInterface(LPCTSTR name) override;
  void PaintText(HDC hdc) override;
  void DoEvent(DuiLib::TEventUI& event) override;

  void Start();
  void Pause();
  void Continue();
  void Stop();

 protected:
  ~ToolbarTimeButton() override;

  void OnTimer();
  DuiLib::CDuiString TimesFormat(uint64_t time);

 private:
  bool start_ = false;
  bool pause_ = false;

  uint64_t time_ = 0;
  clock_t delta_time_;

  clock_t restart_time_;

  DuiLib::CDuiString text_;
};