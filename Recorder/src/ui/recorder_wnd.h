#pragma once
#ifndef RECORDER_SRC_UI_RECORDER_WND_H_
#define RECORDER_SRC_UI_RECORDER_WND_H_

#include <deque>
#include <memory>
#include <stack>

#include "canvas_mask_type.h"

class HeaderWnd;
class ToolbarWnd;

class RecorderWnd : public DuiLib::WindowImplBase {
 public:
  enum class HeaderDirection {
    kNone,
    kTop,
    kBottom,
    kLeft,
    kRight,
    kInside,
  };

  static int CalculateInset();
  static int CalculateCanvasPadding();
  static int CalculateDisablePadding();

  static RecorderWnd* GetInstance();

  ~RecorderWnd() override;

  // Override IDialogBuilderCallback
  DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

  // Override DuiLib::WindowImplBase
  void InitWindow() override;
  LPCTSTR GetResourceID() const override;
  DuiLib::CDuiString GetZIPFileName() const override;
  DuiLib::UILIB_RESOURCETYPE GetResourceType() const override;
  LRESULT OnLButtonDown(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnLButtonUp(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnMouseMove(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnKeyDown(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnKillFocus(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnGetMinMaxInfo(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnClose(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnDestroy(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT HandleCustomMessage(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;

  HeaderDirection GetHeaderDirection() { return header_direction_; }
  RECT GetRecordRealRect();
  RECT GetWindowRect();
  void ShowWindow(bool show = true, bool takefocus = true);
  void ShowCanvas(bool show = true);
  void UpdateToolbar() { UpdateToolbarRect(); }
  void UpdateRecordRealRect(int width=-1,int height=-1);
  void SetTriggerSizeBox(bool trigger);
  void SetEnableAllWindows(bool enable);

  // Canvas painter.
  ui_canvas::EditMaskAction GetSelectedEditMask() {
    return cur_selected_mask_;
  }
  void AddEditMask(POINT base_pt);
  void AddEditMask(LPCTSTR name, POINT base_pt);
  void SetEditingMaskRect(POINT base_pt, POINT moving_pt, RECT rc);
  // Select/drag/hover/hide/delete mask.
  void ClearSelectedState();
  bool SelectMask(POINT pt);
  void DragMask(int delta_x, int delta_y);
  void FinishDragMask();
  bool HoverMask(POINT pt);
  void HideSelectedMask();
  void DeleteEmptyTextMask();
  // About action.
  void AddAction(ui_canvas::EditType etype,
                 DuiLib::CControlUI* ctrl,
                 RECT pos,
                 ui_canvas::EditAction act,
                 DWORD col = 0xffffffff,
                 int fsize = 0);
  void AddAction(ui_canvas::EditMaskAction action);
  void PopAction();
  void DeleteLastMask();
  void DeleteLastAction();
  void ClearAllMasks();

  // Edit text.
  DuiLib::TFontInfo* GetFontInfo(int idx);
  void UpdateFontUI(DuiLib::CControlUI* text_ctrl);

  // cursor style.
  bool UpdateCursorStyle();

  // Callback.
  void StartRecordCallback();
  void PauseRecordCallback();
  void ContinueRecordCallback();

 protected:
  // Override DuiLib::WindowImplBase
  DuiLib::CDuiString GetSkinFolder() override;
  DuiLib::CDuiString GetSkinFile() override;
  LPCTSTR GetWindowClassName() const override;

  RECT CalculateWindowRectFromCaptureWindow();
  void InitWindowRect();

  void CreateChildWindow();
  // Toolbar window.
  void ShowToolbar(bool show = true, bool takefocus = true);
  void UpdateToolbarRect();
  // Header window.
  void ShowHeaderWindow(bool show = true, bool takefocus = true);
  void UpdateHeaderWindowRect();

  void OnMoving(WPARAM wparam, LPARAM lparam);
  LRESULT OnSizing(WPARAM wparam, LPARAM lparam);
  void OnCropRecordRect();

 private:
  RecorderWnd();
  void DelayCloseIfTimeLimitExceeded();

  HeaderDirection header_direction_ = HeaderDirection::kNone;

  std::unique_ptr<ToolbarWnd> toolbar_;
  std::unique_ptr<HeaderWnd> headerwnd_;

  DuiLib::CHorizontalLayoutUI* canvas_container_ = nullptr;
  bool do_show_canvas_ = false;
  bool is_mask_selected_ = false;
  POINT edit_base_point_;
  // canvas painter.
  ui_canvas::EditMask cur_edit_mask_;
  ui_canvas::EditMaskAction cur_selected_mask_;
  std::deque<ui_canvas::EditMask> edit_masks_;
  std::stack<ui_canvas::EditMaskAction> edit_action_;
};

#endif  // RECORDER_SRC_UI_RECORDER_WND_H_