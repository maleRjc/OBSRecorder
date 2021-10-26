#pragma once
#ifndef RECORDER_SRC_UI_CAPTURE_WND_H_
#define RECORDER_SRC_UI_CAPTURE_WND_H_

#include "UIlib.h"

class CaptureWnd : public DuiLib::WindowImplBase {
 public:
  static CaptureWnd* GetInstance();

  ~CaptureWnd();

  // override IDialogBuilderCallback
  DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

  // Override DuiLib::WindowImplBase
  void InitWindow() override;
  LPCTSTR GetResourceID() const override;
  DuiLib::CDuiString GetZIPFileName() const override;
  DuiLib::UILIB_RESOURCETYPE GetResourceType() const override;
  void OnClick(DuiLib::TNotifyUI& msg) override;
  LRESULT OnLButtonDown(UINT umsg,
                        WPARAM wparam,
                        LPARAM lparam,
                        BOOL& bhandled) override;
  LRESULT OnLButtonUp(UINT umsg,
                      WPARAM wparam,
                      LPARAM lparam,
                      BOOL& bhandled) override;
  LRESULT OnMouseMove(UINT umsg,
                      WPARAM wparam,
                      LPARAM lparam,
                      BOOL& bhandled) override;
  LRESULT OnClose(UINT umsg,
                  WPARAM wparam,
                  LPARAM lparam,
                  BOOL& bhandled) override;
  LRESULT OnKeyDown(UINT umsg,
                    WPARAM wparam,
                    LPARAM lparam,
                    BOOL& bhandled) override;
  LRESULT HandleCustomMessage(UINT umsg,
                              WPARAM wparam,
                              LPARAM lparam,
                              BOOL& bhandled) override;

  void ShowMagnifyingBar(const POINT& pt, bool visiable = true);
  void ShowLableForClipRectSize();
  RECT GetWindowRect() const { return window_rect_; }
  RECT GetClipPadding() const { return clip_padding_; }
  void SetClipPadding(RECT rc);
  RECT GetClipRect() const;
  RECT GetCanvasContainerRect() const;

 protected:
  // Override DuiLib::WindowImplBase
  DuiLib::CDuiString GetSkinFolder() override;
  DuiLib::CDuiString GetSkinFile() override;
  LPCTSTR GetWindowClassName() const override;

  void InitWindowBackground();
  void ChoiceWindowByCursor();

 private:
  CaptureWnd();

  DuiLib::CControlUI* desktop_image_ = nullptr;
  DuiLib::CControlUI* desktop_mask_ = nullptr;
  DuiLib::CLabelUI* rect_size_lable_ = nullptr;
  DuiLib::CControlUI* desktop_mask_border_ = nullptr;
  DuiLib::CContainerUI* desktop_canvas_container_ = nullptr;

  DuiLib::CVerticalLayoutUI* magnifying_bar_ = nullptr;
  DuiLib::CControlUI* magnifying_img_ = nullptr;
  DuiLib::CLabelUI* point_pos_ = nullptr;
  DuiLib::CLabelUI* point_color_ = nullptr;

  bool clip_choiced_ = false;  // Is clip-rect choiced.
  POINT clip_base_point_;
  RECT clip_padding_;

  RECT window_rect_;
};

#endif  // RECORDER_SRC_UI_CAPTURE_WND_H_