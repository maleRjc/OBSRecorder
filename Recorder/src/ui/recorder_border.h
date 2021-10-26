#pragma once
#ifndef RECORDER_SRC_UI_RECORDER_BORDER_H_
#define RECORDER_SRC_UI_RECORDER_BORDER_H_

#include "UIlib.h"

class CRecorderBorder : public DuiLib::CControlUI {
 public:
  CRecorderBorder();

  // Override DuiLib::CControlUI
  LPCTSTR GetClass() const override;
  LPVOID GetInterface(LPCTSTR name) override;
  void PaintBkImage(HDC hdc) override;
  void PaintBorder(HDC hdc) override;

 protected:
  ~CRecorderBorder() override;
};

#endif  // RECORDER_SRC_UI_RECORDER_BORDER_H_