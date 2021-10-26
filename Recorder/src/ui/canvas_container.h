#pragma once
#ifndef RECORDER_SRC_UI_CANVASCONTAINER_H_
#define RECORDER_SRC_UI_CANVASCONTAINER_H_

#include "UIlib.h"

class CanvasContainer : public DuiLib::CContainerUI {
 public:
  CanvasContainer();
  ~CanvasContainer() override;

  // override DuiLib::CControlUI
  LPCTSTR GetClass() const override;
  LPVOID GetInterface(LPCTSTR str_name) override;
  UINT GetControlFlags() const override;
  void PaintBorder(HDC hdc) override;
  void DoEvent(DuiLib::TEventUI& event) override;
  void SetAttribute(LPCTSTR pstrname, LPCTSTR pstrvalue) override; 

  // 0    1    2
  // 3    4    5
  // 6    7    8
  RECT GetSizerRect(int index);
  LPTSTR GetSizerCursor(POINT& pt, int& cursor_id);

 private:
  bool round_visible_ = true;
  int cursor_;
  UINT button_state_;
  POINT clip_base_point_;
  POINT last_mouse_;
};

#endif  // RECORDER_SRC_UI_CANVASCONTAINER_H_