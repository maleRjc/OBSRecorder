#pragma once
#ifndef RECORDER_SRC_UI_HEADER_WND_H_
#define RECORDER_SRC_UI_HEADER_WND_H_

#include "UIlib.h"

class HeaderEdit;

class HeaderWnd : public DuiLib::WindowImplBase {
 public:
  HeaderWnd(HWND parent);
  ~HeaderWnd();

  // Override DuiLib::WindowImplBase
  void InitWindow() override;
  LPCTSTR GetResourceID() const override;
  DuiLib::CDuiString GetZIPFileName() const override;
  DuiLib::UILIB_RESOURCETYPE GetResourceType() const override;
  LRESULT OnKillFocus(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnKeyDown(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnClose(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT HandleCustomMessage(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  void Notify(DuiLib::TNotifyUI& msg) override;

  // Override IDialogBuilderCallback
  DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

  RECT GetWindowRect();
  void SetEnableEdit(bool enable);

 protected:
  // Override DuiLib::WindowImplBase
  DuiLib::CDuiString GetSkinFolder() override;
  DuiLib::CDuiString GetSkinFile() override;
  LPCTSTR GetWindowClassName() const override;

  void UpdateRecorderWindowRect(int x, int y);
  void OnMoving(WPARAM wparam, LPARAM lparam);
  void OnEditChanged(WPARAM wparam, LPARAM lparam);
  void OnUpdatingEdit(WPARAM wparam, LPARAM lparam);

 private:
  HeaderEdit* record_width_richedit_;
  HeaderEdit* record_height_richedit_;
};

#endif  // RECORDER_SRC_UI_HEADER_WND_H_