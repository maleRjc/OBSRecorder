#include "text_canvas.h"

#include <math.h>

#include "../helper/util.h"
#include "def.h"
#include "recorder_wnd.h"


namespace {
constexpr int kRichEditOriginalWidth = WIDTH_RICHEDIT_ORIGINAL;

// be the same with DUILIB.DLL
constexpr int kInitTextMax = (32 * 1024) - 1;
}


CTextCanvasUI::CTextCanvasUI() : m_cursor_(0), m_button_state_(0) {
  clip_base_point_.x = clip_base_point_.y = 0;
  m_point_last_mouse_.x = m_point_last_mouse_.y = 0;
  SetFontInfo();
  handle_changed = true;
  m_drawcolor_when_selected = false;
  m_changesize_when_selected = false;
  m_state_textchange_event_notify_addaction = true;
}

CTextCanvasUI::~CTextCanvasUI() {}

LPCTSTR CTextCanvasUI::GetClass() const {
  return DUI_CTR_RECORDER_TEXT;
}

LPVOID CTextCanvasUI::GetInterface(LPCTSTR pstrName) {
  if (_tcscmp(pstrName, DUI_CTR_RECORDER_TEXT) == 0)
    return static_cast<CTextCanvasUI*>(this);
  return CControlUI::GetInterface(pstrName);
}

void CTextCanvasUI::OnClick(DuiLib::TNotifyUI& msg) {
  SetEnabled(true);
}

void CTextCanvasUI::DoEvent(DuiLib::TEventUI& event) {
  switch (event.Type) {
    case DuiLib::UIEVENT_BUTTONDOWN:
      SetEnabled(true);
      SetFocus();
      SetBorderSize(1);
      ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
      RecorderWnd::GetInstance()->UpdateFontUI(this);
      break;
    case DuiLib::UIEVENT_DBLCLICK:
      SetEnabled(true);
      SetBorderSize(1);
      SetFocus();
      ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
      break;
    case DuiLib::UIEVENT_SETFOCUS:
      SetBorderSize(1);
      break;
    case DuiLib::UIEVENT_KILLFOCUS:
      SetBorderSize(0);
      SetEnabled(false, false);
      if (event.pSender == NULL)
        break;
      if (0 == event.pSender->GetUserData().Compare(DUI_CTR_COLORBTNTYPE)) {
        SetDrawColorWhenSelected(true);
        event.pSender->SetTag(reinterpret_cast<UINT_PTR>(this));
      } else if (0 == event.pSender->GetName().Compare(_T("fontsize"))) {
        SetDrawSizeWhenSelected(true);
        event.pSender->SetTag(reinterpret_cast<UINT_PTR>(this));
        event.pSender->SetUserData(GetName());
      }
      break;
    case DuiLib::UIEVENT_MOUSEMOVE:
      if (::GetKeyState(VK_LBUTTON) < 0) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
      }
    case DuiLib::UIEVENT_MOUSEENTER:
    case DuiLib::UIEVENT_MOUSEHOVER:
      ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
      return;
      break;
    default:
      break;
  }
  CRichEditUI::DoEvent(event);
}

void CTextCanvasUI::UpdateFontStyle() {
  //SetTextColor(CScrCaptureWnd::Instance()->GetBrushColor());
  //SetFont(CScrCaptureWnd::Instance()->GetFontName(),
  //        CScrCaptureWnd::Instance()->GetFontSize(),
  //        false, false, false);
  //fontInfo_ = CScrCaptureWnd::Instance()->GetFontInfo(GetFont());
}

HRESULT CTextCanvasUI::TxSendMessage(UINT msg,
                                     WPARAM wparam,
                                     LPARAM lparam,
                                     LRESULT* plresult) const {
  return CRichEditUI::TxSendMessage(msg, wparam, lparam, plresult);
}

bool CTextCanvasUI::OnTxViewChanged() {
  RECT old_pos = GetPos();
  int height = 0;
  int limit_width = limit_rect_.right - m_rcItem.left - 5;
  int width = CalculateWidth(limit_width, height);
  if (width < 1)
    return true;

  int caret_height = height / GetLineCount();
  if (width >= kRichEditOriginalWidth) {
    old_pos.right = old_pos.left + width + 5;
    if (old_pos.right > (limit_rect_.right - limit_rect_.left))
      old_pos.right = limit_rect_.right - limit_rect_.left;
  }

  old_pos.bottom = old_pos.top + height;
  if (old_pos.bottom > (limit_rect_.bottom - limit_rect_.top)) {
    old_pos.bottom = limit_rect_.bottom - limit_rect_.top;
    SetLimitText(GetTextLength());
  }
  SetPos(old_pos);

  if (old_pos.bottom > limit_rect_.bottom - limit_rect_.top - caret_height) {
    SetWantReturn(false);
    int tx_len = GetLine(GetLineCount() - 1, limit_width).GetLength();
    fontInfo_ = GetManager()->GetFontInfo(GetFont());
    int font_width = 0.5 * fontInfo_->iSize;
    if (m_rcItem.left + tx_len * font_width >= limit_rect_.right - 5)
      SetLimitText(GetTextLength());
    else
      ResetLimitText();
  } else {
    ResetLimitText();
    SetWantReturn(true);
  }

  if (!m_state_textchange_event_notify_addaction) {
    SetStateTextChangeEventNotifyAddAction(true);
    return true;
  }

  //text-edit action.
  if (GetTextLength(GTL_DEFAULT) >= 0 && handle_changed) {
    if (history_str.empty()) {
      DuiLib::CDuiString str = GetText();
      history_str.push(str);
    } else {
      DuiLib::CDuiString str = GetText();
      history_str.push(str);
      RecorderWnd::GetInstance()->AddAction(ui_canvas::EditType::kText, this,
                                            GetPos(),
                                            ui_canvas::EditAction::kActionEdit);
    }
  }
  this->NeedParentUpdate();
  return true;
}

void CTextCanvasUI::OnTxViewChangedAuto() {
  SetStateTextChangeEventNotifyAddAction(false);
  OnTxViewChanged();
}

void CTextCanvasUI::OnTxNotify(DWORD iNotify, void* pv) {
  CRichEditUI::OnTxNotify(iNotify, pv);
}

void CTextCanvasUI::SetLimitRect(RECT rect) {
  limit_rect_ = rect;
}

void CTextCanvasUI::ResetLimitText() {
  SetLimitText(kInitTextMax);
}

void CTextCanvasUI::SetTextColor(DWORD dwTextColor, BOOL not_handle_action) {
  if (dwTextColor == m_dwTextColor)
    return;
  if (!not_handle_action) {
    RecorderWnd::GetInstance()->AddAction(
        ui_canvas::EditType::kText, this, GetPos(),
        ui_canvas::EditAction::kActionTabFontcolor, m_dwTextColor);
  }
  SetStateTextChangeEventNotifyAddAction(false);
  __super::SetTextColor(dwTextColor);
}

void CTextCanvasUI::SetFont(int index, BOOL not_handle_action) {
  if (!not_handle_action) {
    RecorderWnd::GetInstance()->AddAction(
        ui_canvas::EditType::kText, this, GetPos(),
        ui_canvas::EditAction::kActionTabFontsize, m_dwTextColor, m_iFont);
  }
  SetStateTextChangeEventNotifyAddAction(false);
  __super::SetFont(index);
}

void CTextCanvasUI::DeleteLastEdit() {
  if (history_str.empty())
    return;
  history_str.pop();
  if (history_str.empty())
    return;
  DuiLib::CDuiString str = history_str.top();

  handle_changed = false;
  SetText(str);
  handle_changed = true;
}

void CTextCanvasUI::SetFontInfo() {
  fontInfo_ = RecorderWnd::GetInstance()->GetFontInfo(GetFont());
}

int CTextCanvasUI::GetPrefectHeight() {
  int height = 0;
  int limit_width = limit_rect_.right - m_rcItem.left - 5;
  int width = CalculateWidth(limit_width, height);
  return height;
}