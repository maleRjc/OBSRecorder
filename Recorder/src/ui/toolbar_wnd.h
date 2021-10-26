#pragma once
#ifndef RECORDER_SRC_UI_TOOLBAR_WND_H_
#define RECORDER_SRC_UI_TOOLBAR_WND_H_

#include "canvas_mask_type.h"

class ToolbarTimeButton;

class ToolbarWnd : public DuiLib::WindowImplBase {
 public:
  enum class ToolbarState {
    kNone,
    kNormal,
    kRecording,
    kFinish,
  };

  enum class BrushbarState {
    kNone,
    kClose,
    kBrushSize,
    kTextType,
  };

  enum class BrushSize {
    kSmall = 2,
    kMiddle = 4,
    kLarge = 6
  };

  static int TranslateFontSizeToFontIndex(int fontsize);
  static int TranslateFontIndexToFontSize(int index);

  ToolbarWnd(HWND parent = NULL);
  ~ToolbarWnd() override;

  // Override IDialogBuilderCallback.
  DuiLib::CControlUI* CreateControl(LPCTSTR pstrclass) override;

  // Override DuiLib::WindowImplBase.
  void InitWindow() override;
  LPCTSTR GetResourceID() const override;
  DuiLib::CDuiString GetZIPFileName() const override;
  DuiLib::UILIB_RESOURCETYPE GetResourceType() const override;
  LRESULT OnKeyDown(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT OnClose(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  LRESULT HandleCustomMessage(
      UINT umsg, WPARAM wparam, LPARAM lparam, BOOL& bhandled) override;
  void Notify(DuiLib::TNotifyUI& msg) override;
  void OnClick(DuiLib::TNotifyUI& msg) override;

  void OnCanvasButtonClick(ui_canvas::EditType edit_type,
                           BrushbarState same_state,
                           BrushbarState deference_state);
  void OnColorButtonClick(DuiLib::TNotifyUI& msg);

  void UpdateToolbarState(ToolbarState state, BrushbarState state2);
  void UpdateMaskButtonSkin();
  void UpdatePauseButtonSkin(bool pause);

  void SetBrushColor(DWORD color);
  void SetBrushSize(BrushSize size);
  void SetFontSizeIndex(int id);
  void SetEnableForReButton(bool enable);

  RECT GetWindowRect();
  ui_canvas::EditMask GetCurrentEditType() const { return cur_edit_type_; }
  DuiLib::TFontInfo* GetFontInfo(int idx);

  void StartRecordCallback();
  void PauseRecordCallback();
  void ContinueRecordCallback();

 protected:
  // Override DuiLib::WindowImplBase
  DuiLib::CDuiString GetSkinFolder() override;
  DuiLib::CDuiString GetSkinFile() override;
  LPCTSTR GetWindowClassName() const override;

  void OnClickStart();
  void OnClickPause();
  void OnClickFinish();
  void OnClickAudioInput();
  void OnClickAudioOutput();

  void InitColorPanel();

  void OnExceedRecordLimit(int code);
  void OnCheckLowDisk();

 private:
  ToolbarState current_toolbar_state_ = ToolbarState::kNone;
  BrushbarState current_brushbar_state_ = BrushbarState::kNone;

  DuiLib::CButtonUI* audio_input_btn_ = nullptr;
  DuiLib::CButtonUI* audio_output_btn_ = nullptr;

  DuiLib::CButtonUI* start_btn_ = nullptr;
  ToolbarTimeButton* finish_btn_ = nullptr;
  DuiLib::CButtonUI* pause_btn = nullptr;
  DuiLib::CButtonUI* cancel_btn_ = nullptr;

  DuiLib::CButtonUI* restore_btn_ = nullptr;
  DuiLib::CButtonUI* rechoose_btn_ = nullptr;

  DuiLib::CButtonUI* rectangle_btn_ = nullptr;
  DuiLib::CButtonUI* circle_btn_ = nullptr;
  DuiLib::CButtonUI* line_btn_ = nullptr;
  DuiLib::CButtonUI* arrow_btn_ = nullptr;
  DuiLib::CButtonUI* brush_btn_ = nullptr;
  DuiLib::CButtonUI* text_btn_ = nullptr;
  DuiLib::CButtonUI* highlight_btn_ = nullptr;

  DuiLib::CControlUI* brush_width_bar_ = nullptr;
  DuiLib::CButtonUI* small_brush_btn_ = nullptr;
  DuiLib::CButtonUI* mid_brush_btn_ = nullptr;
  DuiLib::CButtonUI* big_brush_btn_ = nullptr;

  DuiLib::CHorizontalLayoutUI* font_bar_ = nullptr;
  DuiLib::CComboUI* font_size_com_ = nullptr;
  DuiLib::CControlUI* color_preview_ = nullptr;

  ui_canvas::EditMask cur_edit_type_;
};

#endif  // RECORDER_SRC_UI_TOOLBAR_WND_H_