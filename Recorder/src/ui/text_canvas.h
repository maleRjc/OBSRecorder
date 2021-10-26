#pragma once
#ifndef RECORDER_SRC_UI_TEXT_CANVAS_H_
#define RECORDER_SRC_UI_TEXT_CANVAS_H_

#include <stack>
#include "UIlib.h"

class CTextCanvasUI : public DuiLib::CRichEditUI {
 public:
  CTextCanvasUI();
  ~CTextCanvasUI();
  LPCTSTR GetClass() const;
  LPVOID GetInterface(LPCTSTR pstrName);
  void OnClick(DuiLib::TNotifyUI& msg);
  void DoEvent(DuiLib::TEventUI& event);

  void UpdateFontStyle();

  HRESULT TxSendMessage(UINT msg,
                        WPARAM wparam,
                        LPARAM lparam,
                        LRESULT* plresult) const override;
  bool OnTxViewChanged() override;
  void OnTxViewChangedAuto();
  void OnTxNotify(DWORD iNotify, void* pv) override;

  void SetLimitRect(RECT rc);
  void ResetLimitText();

  BOOL DrawSizeWhenSelected() const { return m_changesize_when_selected; }
  void SetDrawSizeWhenSelected(BOOL b_value) {
    m_changesize_when_selected = b_value;
  }
  BOOL DrawColorWhenSelected() const { return m_drawcolor_when_selected; }
  void SetDrawColorWhenSelected(BOOL b_value) {
    m_drawcolor_when_selected = b_value;
  }
  void SetStateTextChangeEventNotifyAddAction(BOOL b_value) {
    m_state_textchange_event_notify_addaction = b_value;
  }
  void SetTextColor(DWORD dwTextColor, BOOL not_handle_action = TRUE);
  void SetFont(int index, BOOL not_handle_action = TRUE);
  void DeleteLastEdit();
  void SetFontInfo();
  int GetPrefectHeight();

 private:
  int m_cursor_;
  UINT m_button_state_;
  POINT clip_base_point_;
  POINT m_point_last_mouse_;

  BOOL handle_changed;
  BOOL m_drawcolor_when_selected;
  BOOL m_changesize_when_selected;
  BOOL m_state_textchange_event_notify_addaction;

  DuiLib::TFontInfo* fontInfo_;
  RECT limit_rect_;
  std::stack<DuiLib::CDuiString> history_str;
};

#endif  // RECORDER_SRC_UI_TEXT_CANVAS_H_
