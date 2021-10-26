#include "recorder_wnd.h"

#include <mutex>

#include "../../resource.h"
#include "../controller.h"
#include "../helper/util.h"
#include "cursor_custom.h"
#include "def.h"
#include "header_wnd.h"
#include "message.h"
#include "recorder_border.h"
#include "recorder_canvas.h"
#include "text_canvas.h"
#include "toolbar_wnd.h"


namespace {
constexpr int kMinResolutionSqrt = VIDEO_MIN_RESOLUTION_SQRT;
constexpr int kSignRadius = RECORDERWND_BORDER_CIRCLE_RADIUS;
constexpr int kBorderSize = 2;
constexpr int kMinFontSize = 8;

constexpr RECT kRectTrigger = {6, 6, 6, 6};
constexpr RECT kRectTriggerNone = {0, 0, 0, 0};

constexpr int kRichEditOriginalWidth = WIDTH_RICHEDIT_ORIGINAL;
constexpr wchar_t kMaskName[] = L"edit-mask-%d";
}  // namespace


// static
int RecorderWnd::CalculateInset() {
  return kSignRadius;
}

// static
int RecorderWnd::CalculateCanvasPadding() {
  return kSignRadius + 1;
}

// static
int RecorderWnd::CalculateDisablePadding() {
  return (kSignRadius - kBorderSize) / 2;
}

// static
RecorderWnd* RecorderWnd::GetInstance() {
  static std::unique_ptr<RecorderWnd> g_RecordWinInstance = nullptr;
  static std::once_flag g_Flag;
  std::call_once(g_Flag, [&]() { g_RecordWinInstance.reset(new RecorderWnd); });
  return g_RecordWinInstance.get();
}

RecorderWnd::~RecorderWnd() {}

DuiLib::CControlUI* RecorderWnd::CreateControl(LPCTSTR pstrClass) {
  if (_tcscmp(pstrClass, DUI_CTR_RECORDER_BORDER) == 0)
    return new CRecorderBorder;
  else if (_tcscmp(pstrClass, DUI_CTR_RECORDER_CANVAS) == 0)
    return new RecorderCanvas;
  else if (_tcscmp(pstrClass, DUI_CTR_RECORDER_TEXT) == 0)
    return new CTextCanvasUI;
  return NULL;
}

void RecorderWnd::InitWindow() {
  CreateChildWindow();

  canvas_container_ = static_cast<DuiLib::CHorizontalLayoutUI*>(
      m_PaintManager.FindControl(_T("canvas-container")));
  m_PaintManager.FindControl(_T("recorder-border"))->SetBorderSize(kBorderSize);

  SetTriggerSizeBox(true);
}

LPCTSTR RecorderWnd::GetResourceID() const {
  return MAKEINTRESOURCE(IDR_ZIPRES1);
}

DuiLib::CDuiString RecorderWnd::GetZIPFileName() const {
  return ZIP_RESOURCE;
}

DuiLib::UILIB_RESOURCETYPE RecorderWnd::GetResourceType() const {
#ifdef _DEBUG
  return DuiLib::UILIB_FILE;
#else
  return DuiLib::UILIB_ZIPRESOURCE;
#endif
}

LRESULT RecorderWnd::OnLButtonDown(UINT umsg,
                                   WPARAM wparam,
                                   LPARAM lparam,
                                   BOOL& bhandled) {
  edit_base_point_ = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};

  is_mask_selected_ = SelectMask(edit_base_point_);
  if (!is_mask_selected_) {
    DeleteEmptyTextMask();
    ClearSelectedState();
    RECT rect = recorder_util::ClipRectByPadding(canvas_container_->GetPos(),
                                                 CalculateCanvasPadding());
    if (::PtInRect(&rect, edit_base_point_)) {
      AddEditMask(edit_base_point_);
      UpdateCursorStyle();
    }
  } else {
    ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
  }

  bhandled = false;
  return 0;
}

LRESULT RecorderWnd::OnLButtonUp(UINT umsg,
                                 WPARAM wparam,
                                 LPARAM lparam,
                                 BOOL& bhandled) {
  if (is_mask_selected_)
    FinishDragMask();
  else if (!edit_action_.empty() &&
           edit_action_.top().action_ == ui_canvas::EditAction::kActionCreate &&
           edit_action_.top().control_ &&
           !edit_action_.top().control_->IsVisible())
    DeleteLastAction();
  cur_edit_mask_.clear();

  bhandled = false;
  return 0;
}

LRESULT RecorderWnd::OnMouseMove(UINT umsg,
                                 WPARAM wparam,
                                 LPARAM lparam,
                                 BOOL& bhandled) {
  POINT mouse_point = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
  if (wparam & MK_LBUTTON) {
    RECT edit_padding_rect;
    edit_padding_rect.left = min(edit_base_point_.x, mouse_point.x);
    edit_padding_rect.top = min(edit_base_point_.y, mouse_point.y);
    edit_padding_rect.right = max(edit_base_point_.x, mouse_point.x);
    edit_padding_rect.bottom = max(edit_base_point_.y, mouse_point.y);
    if (!is_mask_selected_) {
      UpdateCursorStyle();
      SetEditingMaskRect(edit_base_point_, mouse_point, edit_padding_rect);
    } else {
      DragMask(mouse_point.x - edit_base_point_.x,
               mouse_point.y - edit_base_point_.y);
      ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
    }
  } else {
    HCURSOR cursor = NULL;
    if (HoverMask(mouse_point))
      cursor = ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
    if (!cursor)
      UpdateCursorStyle();
  }
  bhandled = false;
  return 0;
}

LRESULT RecorderWnd::OnKeyDown(UINT umsg,
                               WPARAM wparam,
                               LPARAM lparam,
                               BOOL& bhandled) {
  if (wparam == VK_DELETE)
    HideSelectedMask();
  if ((char)wparam == 'Z' && ((GetAsyncKeyState(VK_CONTROL) & 0x8000)))
    DeleteLastAction();

  bhandled = false;
  return 0;
}

LRESULT RecorderWnd::OnKillFocus(UINT umsg,
                                 WPARAM wparam,
                                 LPARAM lparam,
                                 BOOL& bhandled) {
  auto lambda_notify_killfocus_event = [](ui_canvas::EditMask* mask) {
    if (mask->control_ && mask->edit_type_ == ui_canvas::EditType::kText) {
      DuiLib::TEventUI event = {0};
      event.Type = DuiLib::UIEVENT_KILLFOCUS;
      event.pSender = mask->control_;
      event.dwTimestamp = ::GetTickCount();
      mask->control_->Event(event);
    }
  };
  lambda_notify_killfocus_event(&cur_selected_mask_);
  lambda_notify_killfocus_event(&cur_edit_mask_);

  DeleteEmptyTextMask();

  bhandled = false;
  return 0;
}

LRESULT RecorderWnd::OnGetMinMaxInfo(UINT umsg,
                                     WPARAM wparam,
                                     LPARAM lparam,
                                     BOOL& bhandled) {

	LPMINMAXINFO min_max_info = (LPMINMAXINFO)lparam;

  MONITORINFO monitor = {};
  monitor.cbSize = sizeof(monitor);
  ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST),
                   &monitor);
  //DuiLib::CDuiRect rcWork = oMonitor.rcWork;
  DuiLib::CDuiRect rcMonitor = monitor.rcMonitor;
  //rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

  int record_inset = CalculateInset();

  min_max_info->ptMaxPosition.x = rcMonitor.left - record_inset;
  min_max_info->ptMaxPosition.y = rcMonitor.top - record_inset;

  min_max_info->ptMaxTrackSize.x = rcMonitor.GetWidth() + record_inset * 2;
  min_max_info->ptMaxTrackSize.y = rcMonitor.GetHeight() + record_inset * 2;

  min_max_info->ptMinTrackSize.x = record_inset * 2 + kMinResolutionSqrt;
  min_max_info->ptMinTrackSize.y = record_inset * 2 + kMinResolutionSqrt;

  bhandled = FALSE;
  return 0;
}

LRESULT RecorderWnd::OnClose(UINT umsg,
                           WPARAM wparam,
                           LPARAM lparam,
                           BOOL& bhandled) {
  Controller* controller_ptr = Controller::GetInstance();
  if (controller_ptr->IsRecording()) {
    controller_ptr->StopRecord(wparam);
    ShowWindow(false);
    DelayCloseIfTimeLimitExceeded();
  } else {
    ::PostQuitMessage(0);
  }
  return 0;
}

LRESULT RecorderWnd::OnDestroy(UINT umsg,
                             WPARAM wparam,
                             LPARAM lparam,
                             BOOL& bhandled) {
  ::PostQuitMessage(0);
  return 1;
}

LRESULT RecorderWnd::HandleCustomMessage(UINT umsg,
                                       WPARAM wparam,
                                       LPARAM lparam,
                                       BOOL& bhandled) {
  LRESULT lres = 0;
  switch (umsg) {
    case WM_MOVING:
      OnMoving(wparam, lparam);
      break;
    case WM_SIZING:
      OnSizing(wparam, lparam);
      break;
    case MSG_SHOW_CANVAS:
      ShowCanvas(bool(wparam));
      break;
    case MSG_CANVAS_RESTORE:
      DeleteLastAction();
      break;
    case MSG_CANVAS_RECHOOSE:
      ClearAllMasks();
      break;
    case MSG_CROP_RECORD_RECT:
      OnCropRecordRect();
      break;
  }
  return lres;
}

RECT RecorderWnd::GetRecordRealRect() {
  int record_inset = CalculateInset();
  RECT record_real_rect = GetWindowRect();
  record_real_rect.left += record_inset;
  record_real_rect.top += record_inset;
  record_real_rect.right -= record_inset;
  record_real_rect.bottom -= record_inset;
  return record_real_rect;
}

RECT RecorderWnd::GetWindowRect() {
  RECT rect;
  ::GetWindowRect(GetHWND(), &rect);
  return rect;
}

void RecorderWnd::ShowWindow(bool show, bool takefocus) {
  ShowToolbar(show, false);
  ShowHeaderWindow(show, false);
  CWindowWnd::ShowWindow(show, takefocus);
}

void RecorderWnd::ShowCanvas(bool show) {
  if (do_show_canvas_ == show)
    return;
  do_show_canvas_ = show;
  SetTriggerSizeBox(!show && edit_masks_.empty() &&
                    !Controller::GetInstance()->IsRecording());
  ClearSelectedState();

  DWORD exstyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
  DWORD bkcolor = 0x00ffffff | (DWORD(show) << 24);
  canvas_container_->SetBkColor(bkcolor);
  if (edit_masks_.empty() || do_show_canvas_)
    ::SetWindowLong(GetHWND(), GWL_EXSTYLE, exstyle);
  else
    ::SetWindowLong(GetHWND(), GWL_EXSTYLE, exstyle | WS_EX_TRANSPARENT);

  // Update toolbar's rect after switching on/off state for canvas.
  UpdateToolbarRect();
}

void RecorderWnd::UpdateRecordRealRect(int width, int height) {
  RECT record_rect = GetRecordRealRect();
  if (width != -1)
    record_rect.right = record_rect.left + width;
  if (height != -1)
    record_rect.bottom = record_rect.top + height;

  int record_inset = CalculateInset();
  record_rect.left -= record_inset;
  record_rect.top -= record_inset;
  record_rect.right += record_inset;
  record_rect.bottom += record_inset;

  ResizeClient(record_rect.right - record_rect.left,
               record_rect.bottom - record_rect.top);
  UpdateToolbarRect();
}

void RecorderWnd::AddEditMask(POINT base_pt) {
  TCHAR mask_name[MAX_PATH];
  _stprintf_s(mask_name, MAX_PATH - 1, kMaskName, edit_masks_.size());
  AddEditMask(mask_name, base_pt);
}

void RecorderWnd::AddEditMask(LPCTSTR name, POINT base_pt) {
  ui_canvas::EditMask cur_edit_type = toolbar_->GetCurrentEditType();
  DuiLib::CControlUI* adding_mask = NULL;
  cur_edit_mask_.clear();

  if (cur_edit_type.edit_type_ == ui_canvas::EditType::kNone)
    return;

  if (cur_edit_type.edit_type_ == ui_canvas::EditType::kText) {
    CTextCanvasUI* text_mask = new CTextCanvasUI;
    text_mask->SetName(name);
    canvas_container_->Add(text_mask);
    text_mask->SetManager(&m_PaintManager, canvas_container_, true);

    text_mask->SetLimitRect(
        recorder_util::MoveRect(canvas_container_->GetPos(), 0, 0));
    text_mask->SetTextColor(cur_edit_type.color_);
    text_mask->SetMinHeight(kMinFontSize);
    text_mask->SetFont(cur_edit_type.font_size_id_);
    text_mask->SetStateTextChangeEventNotifyAddAction(true);
    text_mask->SetRich(true);

    RECT text_rect = {base_pt.x, base_pt.y, base_pt.x + kRichEditOriginalWidth,
                      base_pt.y + text_mask->GetPrefectHeight()};
    RECT real_rect = recorder_util::MoveRect(canvas_container_->GetPos(), 0, 0);
    if (text_rect.right > real_rect.right) {
      text_rect.right = real_rect.right;
    }
    if (text_rect.bottom > real_rect.bottom) {
      text_rect.top = real_rect.bottom - text_mask->GetPrefectHeight();
      text_rect.bottom = real_rect.bottom;
    }
    text_mask->SetFloat(true);
    text_mask->SetPos(text_rect);
    text_mask->SetBorderColor(cur_edit_type.color_);

    adding_mask = text_mask;

    // set as selected texts.
    cur_selected_mask_.control_ = text_mask;
    cur_selected_mask_.edit_type_ = ui_canvas::EditType::kText;
  } else {
    adding_mask = new RecorderCanvas(cur_edit_type.edit_type_);
    adding_mask->SetName(name);
    canvas_container_->Add(adding_mask);
    adding_mask->SetManager(&m_PaintManager, canvas_container_, true);

    adding_mask->SetBorderSize(cur_edit_type.mask_size_);
    adding_mask->SetBorderColor(cur_edit_type.color_);
    adding_mask->SetMouseEnabled(false);
    RECT edit_rect = recorder_util::MoveRect(canvas_container_->GetPos(), 0, 0);
    adding_mask->SetFloat(true);
    adding_mask->SetPos(edit_rect);

    adding_mask->SetVisible(false);
  }

  cur_edit_mask_ = {cur_edit_type.edit_type_, adding_mask};
  edit_masks_.push_back(cur_edit_mask_);
  AddAction(cur_edit_mask_.edit_type_, cur_edit_mask_.control_,
            adding_mask->GetPos(), ui_canvas::EditAction::kActionCreate);

  toolbar_->SetEnableForReButton(true);
}

void RecorderWnd::SetEditingMaskRect(POINT base_pt,
                                     POINT moving_pt,
                                     RECT rc) {
  if (!cur_edit_mask_.control_ ||
      cur_edit_mask_.edit_type_ == ui_canvas::EditType::kNone)
    return;

  moving_pt = recorder_util::GetPointFromLimitRect(
      moving_pt,
      recorder_util::ClipRectByPadding(canvas_container_->GetPos(),
                                       CalculateInset()));

  switch (cur_edit_mask_.edit_type_) {
    case ui_canvas::EditType::kText:
      return;
    case ui_canvas::EditType::kBrush:
      static_cast<RecorderCanvas*>(cur_edit_mask_.control_)
          ->AddFreePathPoint(moving_pt);
    case ui_canvas::EditType::kRectAngle:
    case ui_canvas::EditType::kCircle:
    case ui_canvas::EditType::kLine:
    case ui_canvas::EditType::kArrow:
    case ui_canvas::EditType::kHighlight:
      static_cast<RecorderCanvas*>(cur_edit_mask_.control_)
          ->SetBasePointAndMousePoint(base_pt, moving_pt);
      break;
  }

  cur_edit_mask_.control_->SetVisible(true);
  cur_edit_mask_.control_->NeedParentUpdate();
}

void RecorderWnd::ClearSelectedState() {
  cur_selected_mask_.clear();
}

bool RecorderWnd::SelectMask(POINT pt) {
  for (auto mask = edit_masks_.rbegin(); mask != edit_masks_.rend(); mask++) {
    if (mask->edit_type_ == ui_canvas::EditType::kText) {
      if (::PtInRect(&mask->control_->GetPos(), pt)) {
        cur_selected_mask_ = {
            ui_canvas::EditType::kText,
            mask->control_,
            mask->control_->GetPos(),
            ui_canvas::EditAction::kActionMove,
        };
        mask->control_->NeedParentUpdate();
        return true;
      }
    } else {
      RecorderCanvas* selected_mask =
          static_cast<RecorderCanvas*>(mask->control_);
      if (selected_mask && selected_mask->IsInMaskRect(pt) &&
          mask->control_->IsVisible()) {
        cur_selected_mask_ = {
            selected_mask->GetEditType(),
            selected_mask,
            selected_mask->GetPos(),
            ui_canvas::EditAction::kActionMove,
        };
        selected_mask->NeedParentUpdate();
        return true;
      }
    }
  }
  return false;
}

void RecorderWnd::DragMask(int delta_x, int delta_y) {
  RECT base_rect = cur_selected_mask_.position_;
  base_rect.left += delta_x;
  base_rect.right += delta_x;
  base_rect.top += delta_y;
  base_rect.bottom += delta_y;

  cur_selected_mask_.control_->SetPos(base_rect);
  cur_selected_mask_.control_->NeedParentUpdate();
}

void RecorderWnd::FinishDragMask() {
  if (!::EqualRect(&cur_selected_mask_.position_,
                   &cur_selected_mask_.control_->GetPos())) {
    cur_selected_mask_.action_ = ui_canvas::EditAction::kActionMove;
    AddAction(cur_selected_mask_);
  }
}

bool RecorderWnd::HoverMask(POINT pt) {
  for (auto mask = edit_masks_.rbegin(); mask != edit_masks_.rend(); mask++) {
    RecorderCanvas* canvas = static_cast<RecorderCanvas*>(mask->control_);
    if (canvas && canvas->IsVisible() && canvas->IsInMaskRect(pt))
      return true;
  }
  return false;
}

void RecorderWnd::HideSelectedMask() {
  if (cur_selected_mask_.edit_type_ != ui_canvas::EditType::kNone &&
      cur_selected_mask_.control_ && cur_selected_mask_.control_->IsVisible()) {
    cur_selected_mask_.action_ = ui_canvas::EditAction::kActionDelete;
    cur_selected_mask_.control_->SetVisible(false);
    AddAction(cur_selected_mask_);
  }
}

void RecorderWnd::DeleteEmptyTextMask() {
  if (!edit_masks_.empty()) {
    if (cur_selected_mask_.edit_type_ == ui_canvas::EditType::kText) {
      CTextCanvasUI* ptrctrl_text =
          static_cast<CTextCanvasUI*>(cur_selected_mask_.control_);
      if (ptrctrl_text->GetText().IsEmpty() &&
              edit_action_.top().action_ == ui_canvas::EditAction::kActionCreate)
        DeleteLastAction();
    }
  }
}

void RecorderWnd::AddAction(ui_canvas::EditType etype,
                            DuiLib::CControlUI* ctrl,
                            RECT pos,
                            ui_canvas::EditAction act,
                            DWORD col,
                            int fsize) {
  ui_canvas::EditMaskAction ema(etype, ctrl, pos, act, col, fsize);
  AddAction(ema);
}

void RecorderWnd::AddAction(ui_canvas::EditMaskAction action) {
  edit_action_.push(action);
}

void RecorderWnd::PopAction() {
  edit_action_.pop();
}

void RecorderWnd::DeleteLastMask() {  
  if (edit_masks_.empty()) {
    cur_edit_mask_.clear();
    toolbar_->SetEnableForReButton(false);
    return;
  }
  cur_edit_mask_ = edit_masks_.back();

  // cancel selected status because the control will remove.
  if (cur_selected_mask_.control_ == cur_edit_mask_.control_)
    ClearSelectedState();

  canvas_container_->Remove(cur_edit_mask_.control_);  
  edit_masks_.pop_back();

  if (edit_masks_.empty()) {
    cur_edit_mask_.clear();
    toolbar_->SetEnableForReButton(false);
    return;
  } else {
    cur_edit_mask_ = edit_masks_.back();
  }
}

void RecorderWnd::DeleteLastAction() {
  if (edit_action_.empty())
    return;

  ui_canvas::EditMaskAction last_action_for_cur = edit_action_.top();
  PopAction();

  switch (last_action_for_cur.action_) {
    case ui_canvas::EditAction::kActionCreate:
      DeleteLastMask();
      break;
    case ui_canvas::EditAction::kActionDelete:
      if (!last_action_for_cur.control_->IsVisible())
        last_action_for_cur.control_->SetVisible(true);
      break;
    case ui_canvas::EditAction::kActionMove:
      if (last_action_for_cur.edit_type_ != ui_canvas::EditType::kNone) {
        last_action_for_cur.control_->SetPos(last_action_for_cur.position_);
        last_action_for_cur.control_->NeedParentUpdate();
      }
      break;
    case ui_canvas::EditAction::kActionEdit:
      static_cast<CTextCanvasUI*>(last_action_for_cur.control_)
          ->DeleteLastEdit();
      break;
    case ui_canvas::EditAction::kActionTabFontcolor:
      static_cast<CTextCanvasUI*>(last_action_for_cur.control_)
          ->SetTextColor(last_action_for_cur.color_);
      if (last_action_for_cur.control_->IsFocused())
        UpdateFontUI(last_action_for_cur.control_);
      break;
    case ui_canvas::EditAction::kActionTabFontsize:
      static_cast<CTextCanvasUI*>(last_action_for_cur.control_)
          ->SetFont(last_action_for_cur.font_size_id_);
      if (last_action_for_cur.control_->IsFocused())
        UpdateFontUI(last_action_for_cur.control_);
      break;
    default:
      break;
  }
}

void RecorderWnd::ClearAllMasks() {
  ClearSelectedState();
  cur_edit_mask_.clear();
  int mask_count = edit_masks_.size();
  edit_masks_.clear();
  while (!edit_action_.empty())
    PopAction();
  canvas_container_->RemoveAll();

  toolbar_->SetEnableForReButton(false);
}

DuiLib::TFontInfo* RecorderWnd::GetFontInfo(int idx) {
  return toolbar_->GetFontInfo(idx);
}

void RecorderWnd::UpdateFontUI(DuiLib::CControlUI* text_ctrl) {
  CTextCanvasUI* text_ctrl_ = static_cast<CTextCanvasUI*>(text_ctrl);
  if (toolbar_) {
    toolbar_->SetFontSizeIndex(text_ctrl_->GetFont());
    toolbar_->SetBrushColor(text_ctrl_->GetTextColor());
  }
}

bool RecorderWnd::UpdateCursorStyle() {
  HCURSOR setting_cursor = NULL;
  ui_canvas::EditMask cur_edit_type = toolbar_->GetCurrentEditType();
  if (cur_edit_type.edit_type_ == ui_canvas::EditType::kHighlight) {
    HCURSOR cursor = CursorManager::GetInstance()->AddAndGetCursor(
        CursorManager::GenerateName(
            RecorderCanvas::GetHighlightPenSize(cur_edit_type.mask_size_),
            cur_edit_type.color_),
        RecorderCanvas::GetHighlightPenSize(cur_edit_type.mask_size_),
        cur_edit_type.color_);
    setting_cursor = ::SetCursor(cursor);
  }
  return !!setting_cursor;
}

void RecorderWnd::StartRecordCallback() {
  toolbar_->StartRecordCallback();
}

void RecorderWnd::PauseRecordCallback() {
  toolbar_->PauseRecordCallback();
}

void RecorderWnd::ContinueRecordCallback() {
  toolbar_->ContinueRecordCallback();
}

DuiLib::CDuiString RecorderWnd::GetSkinFolder() {
  return SKIN_FOLDER;
}

DuiLib::CDuiString RecorderWnd::GetSkinFile() {
  return XML_RECORDER;
}

LPCTSTR RecorderWnd::GetWindowClassName() const {
  return WINDOW_CLASS_NAME;
}

RECT RecorderWnd::CalculateWindowRectFromCaptureWindow() {
  int record_inset = CalculateInset();
  RECT record_rect = Controller::GetInstance()->GetCaptureRect();
  record_rect.left -= record_inset;
  record_rect.top -= record_inset;
  record_rect.right += record_inset;
  record_rect.bottom += record_inset;
  return record_rect;
}

void RecorderWnd::InitWindowRect() {
  RECT window_rect = CalculateWindowRectFromCaptureWindow();
  ::SetWindowPos(GetHWND(), HWND_TOPMOST, window_rect.left, window_rect.top,
                 window_rect.right - window_rect.left,
                 window_rect.bottom - window_rect.top, NULL);
}

void RecorderWnd::SetTriggerSizeBox(bool trigger) {
  RECT rect_trigger = kRectTrigger;
  RECT rect_trigger_none = kRectTriggerNone;
  if (trigger)
    m_PaintManager.SetSizeBox(rect_trigger);
  else
    m_PaintManager.SetSizeBox(rect_trigger_none);
  // Don't change size by richedits yet.
  headerwnd_->SetEnableEdit(trigger);
}

void RecorderWnd::SetEnableAllWindows(bool enable) {
  ::EnableWindow(GetHWND(), enable);
  ::EnableWindow(toolbar_->GetHWND(), enable);
  ::EnableWindow(headerwnd_->GetHWND(), enable);
}

void RecorderWnd::CreateChildWindow() {
  // In order to place toolbar-window on canvas-window,
  // we set parent of toolbar-window as canvas-window. 
  if (!toolbar_)
    toolbar_.reset(new ToolbarWnd(GetHWND()));
  if (!headerwnd_)
    headerwnd_.reset(new HeaderWnd(GetHWND()));
}

void RecorderWnd::ShowToolbar(bool show, bool takefocus) {
  if (toolbar_)
    toolbar_->ShowWindow(show, takefocus);
  if (show)
    UpdateToolbarRect();
}

void RecorderWnd::UpdateToolbarRect() {
  if (toolbar_) {
    constexpr int kPaddingTopBottom = 8;
    constexpr int kPaddingLeftRight = 4;

    RECT new_toolbar_rect;
    RECT toolbar_rect = toolbar_->GetWindowRect();
    int toolbar_width = toolbar_rect.right - toolbar_rect.left;
    int toolbar_height = toolbar_rect.bottom - toolbar_rect.top;

    RECT recorder_rect = GetWindowRect();
    recorder_rect = recorder_util::ClipRectByPadding(recorder_rect,
                                                     CalculateDisablePadding());

    RECT moving_range = recorder_util::GetDesktopWindow();

    // Try to place on bottom of recorder_rect.
    new_toolbar_rect.left =
        recorder_rect.right - toolbar_width - kPaddingLeftRight;
    if (new_toolbar_rect.left < moving_range.left + kPaddingLeftRight)
      new_toolbar_rect.left = moving_range.left + kPaddingLeftRight;
    new_toolbar_rect.top = recorder_rect.bottom + kPaddingTopBottom;
    new_toolbar_rect.right = new_toolbar_rect.left + toolbar_width;
    new_toolbar_rect.bottom = new_toolbar_rect.top + toolbar_height;
    if (recorder_util::IsRectInRect(new_toolbar_rect, moving_range)) {
      ::MoveWindow(toolbar_->GetHWND(), new_toolbar_rect.left,
                   new_toolbar_rect.top, toolbar_width, toolbar_height, false);
      return;
    }

    // Try to place on right of recorder_rect.
    new_toolbar_rect.left = recorder_rect.right + kPaddingLeftRight;
    new_toolbar_rect.top = recorder_rect.bottom - toolbar_height;
    new_toolbar_rect.right = new_toolbar_rect.left + toolbar_width;
    new_toolbar_rect.bottom = new_toolbar_rect.top + toolbar_height;
    if (recorder_util::IsRectInRect(new_toolbar_rect, moving_range)) {
      ::MoveWindow(toolbar_->GetHWND(), new_toolbar_rect.left,
                   new_toolbar_rect.top, toolbar_width, toolbar_height, false);
      return;
    }

    // Try to place on left of recorder_rect.
    new_toolbar_rect.left =
        recorder_rect.left - toolbar_width - kPaddingLeftRight;
    new_toolbar_rect.top = recorder_rect.bottom - toolbar_height;
    new_toolbar_rect.right = new_toolbar_rect.left + toolbar_width;
    new_toolbar_rect.bottom = new_toolbar_rect.top + toolbar_height;
    if (recorder_util::IsRectInRect(new_toolbar_rect, moving_range)) {
      ::MoveWindow(toolbar_->GetHWND(), new_toolbar_rect.left,
                   new_toolbar_rect.top, toolbar_width, toolbar_height, false);
      return;
    }

    // Try to place on top of recorder_rect.
    new_toolbar_rect.left =
        recorder_rect.right - toolbar_width - kPaddingLeftRight;
    new_toolbar_rect.top =
        recorder_rect.top - toolbar_height - kPaddingTopBottom;
    new_toolbar_rect.right = new_toolbar_rect.left + toolbar_width;
    new_toolbar_rect.bottom = new_toolbar_rect.top + toolbar_height;
    if (recorder_util::IsRectInRect(new_toolbar_rect, moving_range)) {
      ::MoveWindow(toolbar_->GetHWND(), new_toolbar_rect.left,
                   new_toolbar_rect.top, toolbar_width, toolbar_height, false);
      return;
    }

    // Place in bottom-right of recorder_rect.
    new_toolbar_rect.left =
        recorder_rect.right - toolbar_width - kSignRadius;
    new_toolbar_rect.top =
        recorder_rect.bottom - toolbar_height - kPaddingTopBottom;
    new_toolbar_rect.right = new_toolbar_rect.left + toolbar_width;
    new_toolbar_rect.bottom = new_toolbar_rect.top + toolbar_height;
    if (recorder_util::IsRectInRect(new_toolbar_rect, moving_range)) {
      ::MoveWindow(toolbar_->GetHWND(), new_toolbar_rect.left,
                   new_toolbar_rect.top, toolbar_width, toolbar_height, false);
      return;
    }

    // It's impossible.
    toolbar_->CenterWindow();
  }
}

void RecorderWnd::ShowHeaderWindow(bool show, bool takefocus) {
  if (headerwnd_) {
    headerwnd_->ShowWindow(show, takefocus);  
    headerwnd_->PostMessage(MSG_HEADER_UPDATE_EDIT);
  }
  if (show)
    UpdateHeaderWindowRect();
}

void RecorderWnd::UpdateHeaderWindowRect() {
  if (headerwnd_) {
    const int kPaddingTopBottom = CalculateDisablePadding() - kBorderSize;

    RECT new_headerwnd_rect;
    RECT headerwnd_rect = headerwnd_->GetWindowRect();
    int headerwnd_width = headerwnd_rect.right - headerwnd_rect.left;
    int headerwnd_height = headerwnd_rect.bottom - headerwnd_rect.top;

    RECT recorder_rect = GetWindowRect();
    recorder_rect = recorder_util::ClipRectByPadding(recorder_rect,
                                                     CalculateDisablePadding());

    RECT moving_range = recorder_util::GetDesktopWindow();

    // Try to place on top of recorder_rect.
    new_headerwnd_rect.left = recorder_rect.left;
    new_headerwnd_rect.top = recorder_rect.top - headerwnd_height;
    new_headerwnd_rect.right = new_headerwnd_rect.left + headerwnd_width;
    new_headerwnd_rect.bottom = new_headerwnd_rect.top + headerwnd_height;
    if (recorder_util::IsRectInRect(new_headerwnd_rect, moving_range)) {
      ::MoveWindow(headerwnd_->GetHWND(), new_headerwnd_rect.left,
                   new_headerwnd_rect.top, headerwnd_width, headerwnd_height,
                   false);
      header_direction_ = HeaderDirection::kTop;
      return;
    }

    // Try to place on right of recorder_rect.
    new_headerwnd_rect.left = recorder_rect.right;
    new_headerwnd_rect.top = recorder_rect.top + kPaddingTopBottom;
    new_headerwnd_rect.right = new_headerwnd_rect.left + headerwnd_width;
    new_headerwnd_rect.bottom = new_headerwnd_rect.top + headerwnd_height;
    if (recorder_util::IsRectInRect(new_headerwnd_rect, moving_range)) {
      ::MoveWindow(headerwnd_->GetHWND(), new_headerwnd_rect.left,
                   new_headerwnd_rect.top, headerwnd_width, headerwnd_height,
                   false);
      header_direction_ = HeaderDirection::kRight;
      return;
    }

    // Try to place on left of recorder_rect.
    new_headerwnd_rect.left = recorder_rect.left - headerwnd_width;
    new_headerwnd_rect.top = recorder_rect.top + kPaddingTopBottom;
    new_headerwnd_rect.right = new_headerwnd_rect.left + headerwnd_width;
    new_headerwnd_rect.bottom = new_headerwnd_rect.top + headerwnd_height;
    if (recorder_util::IsRectInRect(new_headerwnd_rect, moving_range)) {
      ::MoveWindow(headerwnd_->GetHWND(), new_headerwnd_rect.left,
                   new_headerwnd_rect.top, headerwnd_width, headerwnd_height,
                   false);
      header_direction_ = HeaderDirection::kLeft;
      return;
    }

    // Try to place on bottom of recorder_rect.
    new_headerwnd_rect.left = recorder_rect.left;
    new_headerwnd_rect.top = recorder_rect.bottom;
    new_headerwnd_rect.right = new_headerwnd_rect.left + headerwnd_width;
    new_headerwnd_rect.bottom = new_headerwnd_rect.top + headerwnd_height;
    if (recorder_util::IsRectInRect(new_headerwnd_rect, moving_range)) {
      ::MoveWindow(headerwnd_->GetHWND(), new_headerwnd_rect.left,
                   new_headerwnd_rect.top, headerwnd_width, headerwnd_height,
                   false);
      header_direction_ = HeaderDirection::kBottom;
      return;
    }

    // Try to place in top of recorder_rect.
    new_headerwnd_rect.left = recorder_rect.left;
    new_headerwnd_rect.top = recorder_rect.top;
    new_headerwnd_rect.right = new_headerwnd_rect.left + headerwnd_width;
    new_headerwnd_rect.bottom = new_headerwnd_rect.top + headerwnd_height;
    //if (recorder_util::IsRectInRect(new_headerwnd_rect, moving_range)) {
      ::MoveWindow(headerwnd_->GetHWND(), new_headerwnd_rect.left,
                   new_headerwnd_rect.top, headerwnd_width, headerwnd_height,
                   false);
      header_direction_ = HeaderDirection::kInside;
      return;
    //}

    // It's impossible.
    headerwnd_->CenterWindow();
  }
}

void RecorderWnd::OnMoving(WPARAM wparam, LPARAM lparam) {
  LPRECT new_rect = (LPRECT)lparam;
  int width = new_rect->right - new_rect->left;
  int height = new_rect->bottom - new_rect->top;

  RECT moving_range = recorder_util::GetDesktopWindow();
  moving_range.right -= width;
  moving_range.bottom -= height;

  if (new_rect->left < moving_range.left)
    new_rect->left = moving_range.left;
  if (new_rect->top < moving_range.top)
    new_rect->top = moving_range.top;
  if (new_rect->left > moving_range.right)
    new_rect->left = moving_range.right;
  if (new_rect->top > moving_range.bottom)
    new_rect->top = moving_range.bottom;
  new_rect->right = new_rect->left + width;
  new_rect->bottom = new_rect->top + height;

  UpdateToolbarRect();
}

LRESULT RecorderWnd::OnSizing(WPARAM wparam, LPARAM lparam) {
  UpdateToolbarRect();
  UpdateHeaderWindowRect();
  if (headerwnd_)
    headerwnd_->PostMessage(MSG_HEADER_UPDATE_EDIT);
  return 0;
}

void RecorderWnd::OnCropRecordRect() {
  Controller::GetInstance()->CropRecordRect(GetRecordRealRect());
}

RecorderWnd::RecorderWnd() {
  RECT window_rect = CalculateWindowRectFromCaptureWindow();
  Create(NULL, WINDOW_NAME_CAPTURE, WS_POPUP,
         WS_EX_TOPMOST | WS_EX_TOOLWINDOW/* | WS_EX_LAYERED*/,
         window_rect.left,
         window_rect.top, window_rect.right - window_rect.left,
         window_rect.bottom - window_rect.top);
  InitWindowRect();
}

void RecorderWnd::DelayCloseIfTimeLimitExceeded() {
  auto close_runtime_error = [](HWND hwnd) {
    constexpr int kSecond = 10;
    int second = kSecond * 1000;
    ::Sleep(second);
    if (::IsWindow(hwnd))
      ::PostMessage(hwnd, WM_QUIT, 0, 0);
  };
  (new std::thread(close_runtime_error, GetHWND()))->detach();
}
