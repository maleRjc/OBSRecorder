#include "toolbar_wnd.h"

#include "../../resource.h"
#include "../controller.h"
#include "../helper/util.h"
#include "def.h"
#include "message.h"
#include "recorder_wnd.h"
#include "toolbar_time_button.h"


namespace {
constexpr int kGifNormalWidth = 447;
constexpr int kGifRecordingWidth = 487;
constexpr int kMp4NormalWidth = 513;
constexpr int kMp4RecordingWidth = 553;
constexpr int kCloseHeight = 34;
constexpr int kBrushbarHeight = 72;

constexpr wchar_t kUserDataFlagString[] = L"click";

constexpr wchar_t kColorName[] = L"color%d";
constexpr int kColorRowX = 10;
constexpr int kColorRowY = 2;
constexpr DWORD Colors[kColorRowY][kColorRowX] = {
    {0xFF000000, 0xFF7F7F7F, 0xFF880015, 0xFFED1C24, 0xFFFF7F27, 0xFFFFF200,
     0xFF22B14C, 0xFF00A2E8, 0xFF3F48CC, 0xFFA349A4},
    {0xFFFFFFFF, 0xFFC3C3C3, 0xFFB97A57, 0xFFFFAE79, 0xFFFFC90E, 0xFFEFE4B0,
     0xFFB5E61D, 0xFF99D9EA, 0xFF7092BE, 0xFFC8BFE7}};

}  // namespace

// static
int ToolbarWnd::TranslateFontSizeToFontIndex(int fontsize) {
  return (fontsize - 6) / 2;
}
int ToolbarWnd::TranslateFontIndexToFontSize(int index) {
  return index * 2 + 6;
}

ToolbarWnd::ToolbarWnd(HWND parent) {
  Create(parent, WINDOW_NAME_TOOLBAR, WS_POPUP,
         WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 0);
}

ToolbarWnd::~ToolbarWnd() {}

DuiLib::CControlUI* ToolbarWnd::CreateControl(LPCTSTR pstrclass) {
  if (_tcscmp(pstrclass, DUI_CTR_TOOLBAR_TIME_BUTTON) == 0)
    return new ToolbarTimeButton;
  return NULL;
}

void ToolbarWnd::InitWindow() {
  audio_input_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("audio-input")));
  audio_output_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("audio-output")));

  start_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("start")));
  finish_btn_ =
      static_cast<ToolbarTimeButton*>(m_PaintManager.FindControl(_T("finish")));
  pause_btn =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("pause")));
  cancel_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("cancel")));

  restore_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("restore")));
  rechoose_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("rechoose")));
  SetEnableForReButton(false);

  rectangle_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("rectangle")));
  circle_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("circle")));
  line_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("line")));
  arrow_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("arrow")));
  brush_btn_ =
      static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(_T("brush")));
  text_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("textbtn")));
  highlight_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("highlight")));

  brush_width_bar_ = static_cast<DuiLib::CHorizontalLayoutUI*>(
      m_PaintManager.FindControl(_T("brushwidthbar")));
  small_brush_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("smallwidth")));
  mid_brush_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("midwidth")));
  big_brush_btn_ = static_cast<DuiLib::CButtonUI*>(
      m_PaintManager.FindControl(_T("bigwidth")));

  font_bar_ = static_cast<DuiLib::CHorizontalLayoutUI*>(
      m_PaintManager.FindControl(_T("fontbar")));
  font_size_com_ = static_cast<DuiLib::CComboUI*>(
      m_PaintManager.FindControl(_T("fontsize")));

  color_preview_ = m_PaintManager.FindControl(_T("colorpreview"));
  InitColorPanel();

  UpdateToolbarState(ToolbarState::kNormal, BrushbarState::kClose);
  UpdateMaskButtonSkin();
}

LPCTSTR ToolbarWnd::GetResourceID() const {
  return MAKEINTRESOURCE(IDR_ZIPRES1);
}

DuiLib::CDuiString ToolbarWnd::GetZIPFileName() const {
  return ZIP_RESOURCE;
}

DuiLib::UILIB_RESOURCETYPE ToolbarWnd::GetResourceType() const {
#ifdef _DEBUG
  return DuiLib::UILIB_FILE;
#else
  return DuiLib::UILIB_ZIPRESOURCE;
#endif
}

LRESULT ToolbarWnd::OnKeyDown(UINT umsg,
                              WPARAM wparam,
                              LPARAM lparam,
                              BOOL& bhandled) {
  bhandled = true;
  switch (wparam) {
    case VK_DELETE:
      RecorderWnd::GetInstance()->HideSelectedMask();
      break;
    case 'Z':
      if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        RecorderWnd::GetInstance()->DeleteLastAction();
      break;
    case VK_ESCAPE:
      PostMessage(WM_CLOSE);
      break;
    default:
      bhandled = false;
      break;
  }
  return 0;
}

LRESULT ToolbarWnd::OnClose(UINT umsg,
                            WPARAM wparam,
                            LPARAM lparam,
                            BOOL& bhandled) {
  RecorderWnd::GetInstance()->PostMessage(WM_CLOSE, wparam);
  return 0;
}

LRESULT ToolbarWnd::HandleCustomMessage(UINT umsg,
                                        WPARAM wparam,
                                        LPARAM lparam,
                                        BOOL& bhandled) {
  bhandled = false;
  switch (umsg) { 
    case MSG_EXCEED_RECORD_LIMIT:
      OnExceedRecordLimit(wparam);
      break;
    case MSG_CHECK_LOW_DISK:
      OnCheckLowDisk();
      break;
  }
  return 0;
}

void ToolbarWnd::Notify(DuiLib::TNotifyUI& msg) {
  if (msg.pSender == font_size_com_ && msg.sType == DUI_MSGTYPE_ITEMSELECT) {
    int font_size = _ttoi(static_cast<DuiLib::CComboUI*>(msg.pSender)->GetText());
    SetFontSizeIndex(ToolbarWnd::TranslateFontSizeToFontIndex(font_size));
  }
  __super::Notify(msg);
}

void ToolbarWnd::OnClick(DuiLib::TNotifyUI& msg) {
  if (msg.pSender == finish_btn_) {
    OnClickFinish();
  } else if (msg.pSender == start_btn_) {
    OnClickStart();
  } else if (msg.pSender == cancel_btn_) {
    PostMessage(WM_CLOSE, 1);// Remove file.
  } else if (msg.pSender == pause_btn) {
    OnClickPause();
  } else if (msg.pSender == audio_input_btn_) {
    OnClickAudioInput();
  } else if (msg.pSender == audio_output_btn_) {
    OnClickAudioOutput();
  } else if (msg.pSender == restore_btn_) {
    RecorderWnd::GetInstance()->PostMessage(MSG_CANVAS_RESTORE);
  } else if (msg.pSender == rechoose_btn_) {
    RecorderWnd::GetInstance()->PostMessage(MSG_CANVAS_RECHOOSE);
  } else if (msg.pSender == rectangle_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kRectAngle, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == circle_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kCircle, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == line_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kLine, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == arrow_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kArrow, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == brush_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kBrush, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == highlight_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kHighlight, BrushbarState::kClose,
                        BrushbarState::kBrushSize);
  } else if (msg.pSender == text_btn_) {
    OnCanvasButtonClick(ui_canvas::EditType::kText, BrushbarState::kClose,
                        BrushbarState::kTextType);
  } else if (msg.pSender == small_brush_btn_) {
    SetBrushSize(BrushSize::kSmall);
  } else if (msg.pSender == mid_brush_btn_) {
    SetBrushSize(BrushSize::kMiddle);
  } else if (msg.pSender == big_brush_btn_) {
    SetBrushSize(BrushSize::kLarge);
  } else {
    OnColorButtonClick(msg);
  }
}

void ToolbarWnd::OnCanvasButtonClick(ui_canvas::EditType edit_type,
                                     BrushbarState same_state,
                                     BrushbarState deference_state) {
  if (cur_edit_type_.edit_type_ == edit_type) {
    UpdateToolbarState(current_toolbar_state_, same_state);
    cur_edit_type_.clear();
    ::PostMessage(RecorderWnd::GetInstance()->GetHWND(), MSG_SHOW_CANVAS, 0, 0);
  } else {
    UpdateToolbarState(current_toolbar_state_, deference_state);
    cur_edit_type_.edit_type_ = edit_type;
    ::PostMessage(RecorderWnd::GetInstance()->GetHWND(), MSG_SHOW_CANVAS, 1, 0);
  }
  UpdateMaskButtonSkin();
}

void ToolbarWnd::OnColorButtonClick(DuiLib::TNotifyUI& msg) {
  TCHAR name[10] = {0};
  int curr = 1;
  for (int i = 0; i < kColorRowY; i++) {
    for (int j = 0; j < kColorRowX; j++) {
      _stprintf_s(name, 8, kColorName, curr++);
      if (0 == msg.pSender->GetName().Compare(name))
        SetBrushColor(Colors[i][j]);
    }
  }
}

void ToolbarWnd::UpdateToolbarState(ToolbarState state, BrushbarState state2) {
  RECT win_rect = GetWindowRect();
  int real_width = win_rect.right - win_rect.left;
  int real_height = win_rect.bottom - win_rect.top;

  int normal_width = 0;
  int recording_width = 0;
  switch (Controller::GetInstance()->GetOutputType()) {
    case Controller::OutputType::kGif:
      normal_width = kGifNormalWidth;
      recording_width = kGifRecordingWidth;
      audio_input_btn_->SetVisible(false);
      audio_output_btn_->SetVisible(false);
      break;
    case Controller::OutputType::kMp4:
      normal_width = kMp4NormalWidth;
      recording_width = kMp4RecordingWidth;
      audio_input_btn_->SetVisible(true);
      audio_output_btn_->SetVisible(true);
      break;
  }

  if (current_toolbar_state_ != state) {
    switch (state) {
      case ToolbarState::kNormal:
        real_width = normal_width;
        start_btn_->SetVisible(true);
        finish_btn_->SetVisible(false);
        break;
      case ToolbarState::kRecording:
        real_width = recording_width;
        start_btn_->SetVisible(false);
        finish_btn_->SetVisible(true);
        break;
    }
  }

  if (current_brushbar_state_ != state2) {
    switch (state2) {
      case BrushbarState::kClose:
        real_height = kCloseHeight;
        break;
      case BrushbarState::kBrushSize:
        real_height = kBrushbarHeight;
        brush_width_bar_->SetVisible(true);
        font_bar_->SetVisible(false);
        SetBrushColor(cur_edit_type_.color_);
        break;
      case BrushbarState::kTextType:
        real_height = kBrushbarHeight;
        brush_width_bar_->SetVisible(false);
        font_bar_->SetVisible(true);
        SetBrushColor(cur_edit_type_.color_);
        SetFontSizeIndex(cur_edit_type_.font_size_id_);
        break;
    }
  }

  if (current_toolbar_state_ != state || current_brushbar_state_ != state2) {
    current_toolbar_state_ = state;
    current_brushbar_state_ = state2;
    ResizeClient(real_width, real_height);
  }
}

void ToolbarWnd::UpdateMaskButtonSkin() {
  rectangle_btn_->SetNormalImage(
      _T("file='skin/toolbar/rectangle.png' source='0,0,32,32'"));
  rectangle_btn_->SetHotImage(
      _T("file='skin/toolbar/rectangle.png' source='32,0,64,32'"));

  circle_btn_->SetNormalImage(
      _T("file='skin/toolbar/circle.png' source='0,0,32,32'"));
  circle_btn_->SetHotImage(
      _T("file='skin/toolbar/circle.png' source='32,0,64,32'"));

  line_btn_->SetNormalImage(
      _T("file='skin/toolbar/line.png' source='0,0,32,32'"));
  line_btn_->SetHotImage(
      _T("file='skin/toolbar/line.png' source='32,0,64,32'"));

  arrow_btn_->SetNormalImage(
      _T("file='skin/toolbar/arrow.png' source='0,0,32,32'"));
  arrow_btn_->SetHotImage(_T("file='skin/toolbar/arrow.png' source='32,0,64,32'"));

  brush_btn_->SetNormalImage(
      _T("file='skin/toolbar/brush.png' source='0,0,32,32'"));
  brush_btn_->SetHotImage(
      _T("file='skin/toolbar/brush.png' source='32,0,64,32'"));

  text_btn_->SetNormalImage(
      _T("file='skin/toolbar/text.png' source='0,0,32,32'"));
  text_btn_->SetHotImage(
      _T("file='skin/toolbar/text.png' source='32,0,64,32'"));

  highlight_btn_->SetNormalImage(
      _T("file='skin/toolbar/highlight.png' source='0,0,32,32'"));
  highlight_btn_->SetHotImage(
      _T("file='skin/toolbar/highlight.png' source='32,0,64,32'"));

  switch (cur_edit_type_.edit_type_) {
    case ui_canvas::EditType::kRectAngle:
      rectangle_btn_->SetNormalImage(
          _T("file='skin/toolbar/rectangle.png' source='64,0,96,32'"));
      rectangle_btn_->SetHotImage(
          _T("file='skin/toolbar/rectangle.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kCircle:
      circle_btn_->SetNormalImage(
          _T("file='skin/toolbar/circle.png' source='64,0,96,32'"));
      circle_btn_->SetHotImage(
          _T("file='skin/toolbar/circle.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kLine:
      line_btn_->SetNormalImage(
          _T("file='skin/toolbar/line.png' source='64,0,96,32'"));
      line_btn_->SetHotImage(
          _T("file='skin/toolbar/line.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kArrow:
      arrow_btn_->SetNormalImage(
          _T("file='skin/toolbar/arrow.png' source='64,0,96,32'"));
      arrow_btn_->SetHotImage(
          _T("file='skin/toolbar/arrow.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kBrush:
      brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/brush.png' source='64,0,96,32'"));
      brush_btn_->SetHotImage(
          _T("file='skin/toolbar/brush.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kText:
      text_btn_->SetNormalImage(
          _T("file='skin/toolbar/text.png' source='64,0,96,32'"));
      text_btn_->SetHotImage(
          _T("file='skin/toolbar/text.png' source='64,0,96,32'"));
      break;
    case ui_canvas::EditType::kHighlight:
      highlight_btn_->SetNormalImage(
          _T("file='skin/toolbar/highlight.png' source='64,0,96,32'"));
      highlight_btn_->SetHotImage(
          _T("file='skin/toolbar/highlight.png' source='64,0,96,32'"));
      break;
  }
}

void ToolbarWnd::UpdatePauseButtonSkin(bool pause) {
  if (pause) {
    // Button should change state to be "pause".
    pause_btn->SetUserData(kUserDataFlagString);
    pause_btn->SetNormalImage(
        _T("file='skin/toolbar/pause.png' source='0,0,32,32'"));
    pause_btn->SetHotImage(
        _T("file='skin/toolbar/pause.png' source='32,0,64,32'"));
    pause_btn->SetPushedImage(
        _T("file='skin/toolbar/pause.png' source='64,0,96,32'"));
    pause_btn->SetDisabledImage(
        _T("file='skin/toolbar/pause.png' source='96,0,128,32'"));
    pause_btn->SetToolTip(_T("ÔÝÍ£"));
  } else {
    // Button should change state to be "play".
    pause_btn->SetUserData(_T(""));
    pause_btn->SetNormalImage(
        _T("file='skin/toolbar/play.png' source='0,0,32,32'"));
    pause_btn->SetHotImage(
        _T("file='skin/toolbar/play.png' source='32,0,64,32'"));
    pause_btn->SetPushedImage(
        _T("file='skin/toolbar/play.png' source='64,0,96,32'"));
    pause_btn->SetDisabledImage(
        _T("file='skin/toolbar/play.png' source='96,0,128,32'"));
    pause_btn->SetToolTip(_T("¼ÌÐø"));
  }
}

void ToolbarWnd::SetBrushColor(DWORD color) {
  cur_edit_type_.color_ = color;
  color_preview_->SetBkColor(color);
}

void ToolbarWnd::SetBrushSize(BrushSize size) {
  cur_edit_type_.mask_size_ = int(size);
  switch (size) {
    case BrushSize::kSmall:
      small_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/smallwidth.png' source='64,0,96,32'"));
      small_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/smallwidth.png' source='64,0,96,32'"));
      mid_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/midwidth.png' source='0,0,32,32'"));
      mid_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/midwidth.png' source='32,0,64,32'"));
      big_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/bigwidth.png' source='0,0,32,32'"));
      big_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/bigwidth.png' source='32,0,64,32'"));
      break;
    case BrushSize::kMiddle:
      small_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/smallwidth.png' source='0,0,32,32'"));
      small_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/smallwidth.png' source='32,0,64,32'"));
      mid_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/midwidth.png' source='64,0,96,32'"));
      mid_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/midwidth.png' source='64,0,96,32'"));
      big_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/bigwidth.png' source='0,0,32,32'"));
      big_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/bigwidth.png' source='32,0,64,32'"));
      break;
    case BrushSize::kLarge:
      small_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/smallwidth.png' source='0,0,32,32'"));
      small_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/smallwidth.png' source='32,0,64,32'"));
      mid_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/midwidth.png' source='0,0,32,32'"));
      mid_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/midwidth.png' source='32,0,64,32'"));
      big_brush_btn_->SetNormalImage(
          _T("file='skin/toolbar/bigwidth.png' source='64,0,96,32'"));
      big_brush_btn_->SetHotImage(
          _T("file='skin/toolbar/bigwidth.png' source='64,0,96,32'"));
      break;
  }
}

void ToolbarWnd::SetFontSizeIndex(int id) {
  cur_edit_type_.font_size_id_ = id;
  if (TranslateFontSizeToFontIndex(_ttoi(font_size_com_->GetText())) != id)
    font_size_com_->SelectItem(id - 1, false);
}

void ToolbarWnd::SetEnableForReButton(bool enable) {
  restore_btn_->SetEnabled(enable);
  rechoose_btn_->SetEnabled(enable);
}

RECT ToolbarWnd::GetWindowRect() {
  RECT rect;
  ::GetWindowRect(GetHWND(), &rect);
  return rect;
}

DuiLib::TFontInfo* ToolbarWnd::GetFontInfo(int idx) {
  return m_PaintManager.GetFontInfo(idx);
}

void ToolbarWnd::StartRecordCallback() {
  finish_btn_->Start();
}

void ToolbarWnd::PauseRecordCallback() {
  finish_btn_->Pause();
}

void ToolbarWnd::ContinueRecordCallback() {
  finish_btn_->Continue();
}

DuiLib::CDuiString ToolbarWnd::GetSkinFolder() {
  return SKIN_FOLDER;
}

DuiLib::CDuiString ToolbarWnd::GetSkinFile() {
  return XML_TOOLBAR;
}

LPCTSTR ToolbarWnd::GetWindowClassName() const {
  return WINDOW_CLASS_NAME;
}

void ToolbarWnd::OnClickStart() {
  if (Controller::GetInstance()->StartRecord()) {
    UpdateToolbarState(ToolbarState::kRecording, current_brushbar_state_);
    UpdatePauseButtonSkin(true);
    RecorderWnd::GetInstance()->ShowWindow();
  }
}

void ToolbarWnd::OnClickPause() {
  Controller* controller_ptr = Controller::GetInstance();

  if (0 == pause_btn->GetUserData().Compare(kUserDataFlagString)) {
    if (!controller_ptr->IsPauseRecord()) {
      controller_ptr->PauseRecord();
      UpdatePauseButtonSkin(false);
    }
  } else {
    bool start = true;
    if (controller_ptr->IsPauseRecord())
      controller_ptr->ContinueRecord();
    else if (!controller_ptr->IsRecording())
      start = controller_ptr->StartRecord();

    if (start) {
      if (current_toolbar_state_ == ToolbarState::kNormal) {
        UpdateToolbarState(ToolbarState::kRecording, current_brushbar_state_);
        RecorderWnd::GetInstance()->ShowWindow();
      }
      UpdatePauseButtonSkin(true);
    }
  }
}

void ToolbarWnd::OnClickFinish() {
  PostMessage(WM_CLOSE);
}

void ToolbarWnd::OnClickAudioInput() {
  if (0 == audio_input_btn_->GetUserData().Compare(kUserDataFlagString)) {
    Controller::GetInstance()->AudioInputRecordMute(false);

    audio_input_btn_->SetUserData(_T(""));
    audio_input_btn_->SetNormalImage(
        _T("file='skin/toolbar/microphone.png' source='0,0,32,32'"));
    audio_input_btn_->SetHotImage(
        _T("file='skin/toolbar/microphone.png' source='32,0,64,32'"));
    audio_input_btn_->SetPushedImage(
        _T("file='skin/toolbar/microphone.png' source='64,0,96,32'"));
    audio_input_btn_->SetDisabledImage(
        _T("file='skin/toolbar/microphone.png' source='96,0,128,32'"));
  } else {
    Controller::GetInstance()->AudioInputRecordMute(true);

    audio_input_btn_->SetUserData(kUserDataFlagString);
    audio_input_btn_->SetNormalImage(
        _T("file='skin/toolbar/microphone_disable.png' source='0,0,32,32'"));
    audio_input_btn_->SetHotImage(
        _T("file='skin/toolbar/microphone_disable.png' source='32,0,64,32'"));
    audio_input_btn_->SetPushedImage(
        _T("file='skin/toolbar/microphone_disable.png' source='64,0,96,32'"));
    audio_input_btn_->SetDisabledImage(
        _T("file='skin/toolbar/microphone_disable.png' source='96,0,128,32'"));
  }
}

void ToolbarWnd::OnClickAudioOutput() {
  if (0 == audio_output_btn_->GetUserData().Compare(kUserDataFlagString)) {
    Controller::GetInstance()->AudioOutputRecordMute(false);

    audio_output_btn_->SetUserData(_T(""));
    audio_output_btn_->SetNormalImage(
        _T("file='skin/toolbar/loudspeaker.png' source='0,0,32,32'"));
    audio_output_btn_->SetHotImage(
        _T("file='skin/toolbar/loudspeaker.png' source='32,0,64,32'"));
    audio_output_btn_->SetPushedImage(
        _T("file='skin/toolbar/loudspeaker.png' source='64,0,96,32'"));
    audio_output_btn_->SetDisabledImage(
        _T("file='skin/toolbar/loudspeaker.png' source='96,0,128,32'"));
  } else {
    Controller::GetInstance()->AudioOutputRecordMute(true);

    audio_output_btn_->SetUserData(kUserDataFlagString);
    audio_output_btn_->SetNormalImage(
        _T("file='skin/toolbar/loudspeaker_disable.png' source='0,0,32,32'"));
    audio_output_btn_->SetHotImage(
        _T("file='skin/toolbar/loudspeaker_disable.png' source='32,0,64,32'"));
    audio_output_btn_->SetPushedImage(
        _T("file='skin/toolbar/loudspeaker_disable.png' source='64,0,96,32'"));
    audio_output_btn_->SetDisabledImage(
        _T("file='skin/toolbar/loudspeaker_disable.png' source='96,0,128,32'"));
  }
}

void ToolbarWnd::InitColorPanel() {
  TCHAR name[10] = {0};
  int curr = 1;
  for (int i = 0; i < kColorRowY; i++) {
    for (int j = 0; j < kColorRowX; j++) {
      _stprintf_s(name, 8, kColorName, curr++);
      DuiLib::CButtonUI* btn =
          static_cast<DuiLib::CButtonUI*>(m_PaintManager.FindControl(name));
      if (btn) {
        btn->SetBkColor(Colors[i][j]);
        btn->SetUserData(DUI_CTR_COLORBTNTYPE);
      }
    }
  }
}

void ToolbarWnd::OnExceedRecordLimit(int code) {
  // Pause => Tip => Finish.
  OnClickPause();
  Controller::GetInstance()->ShowMessageBox(Controller::TipType(code));
  OnClickFinish();
}

void ToolbarWnd::OnCheckLowDisk() {
  if (Controller::GetInstance()->CheckLowDiskSpace())
    PostMessage(MSG_EXCEED_RECORD_LIMIT,
                WPARAM(Controller::TipType::kLowDiskWhenRecording));
}