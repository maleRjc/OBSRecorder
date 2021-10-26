#pragma once
#ifndef RECORDER_SRC_UI_HEADER_EDIT_H_
#define RECORDER_SRC_UI_HEADER_EDIT_H_

#include "UIlib.h"

class HeaderEdit : public DuiLib::CRichEditUI {
 public:
  HeaderEdit();

  // Override DuiLib::CControlUI
  LPCTSTR GetClass() const override;
  LPVOID GetInterface(LPCTSTR name) override;

  // Overrider DuiLib::CRichEditUI
  void SetText(LPCTSTR text, bool update_rect=true);
  void SetAttribute(LPCTSTR name, LPCTSTR value) override;
  void DoEvent(DuiLib::TEventUI& event) override;
  LRESULT MessageHandler(UINT umsg,
                         WPARAM wparam,
                         LPARAM lparam,
                         bool& bhandled) override;

  // OnTxViewChanged is diable when we limit a maxchar,
  // so that we set a new method as changed one.
  bool OnHeaderEditChanged();

 protected:
  ~HeaderEdit() override;

 private:
  bool is_width = true;

  // Represent if it update mainwnd.
  bool update_rect_ = true;
  RECT screen_rect_;

  int last_value_ = -1;
  DuiLib::CDuiString last_text_;
};

#endif  // RECORDER_SRC_UI_HEADER_EDIT_H_