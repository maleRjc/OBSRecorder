#include "header_edit.h"

#include "../helper/util.h"
#include "def.h"
#include "message.h"
#include "recorder_wnd.h"


namespace {
constexpr int kMinResolutionSqrt = VIDEO_MIN_RESOLUTION_SQRT;
}

HeaderEdit::HeaderEdit() {
  screen_rect_ = recorder_util::GetDesktopWindow();
}

LPCTSTR HeaderEdit::GetClass() const {
  return DUI_CTR_HEADER_EDIT;
}

LPVOID HeaderEdit::GetInterface(LPCTSTR name) {
  if (_tcscmp(name, DUI_CTR_HEADER_EDIT) == 0)
    return static_cast<HeaderEdit*>(this);
  return __super::GetInterface(name);
}

void HeaderEdit::SetText(LPCTSTR text, bool update_rect) {
  update_rect_ = update_rect;
  __super::SetText(text);
}

void HeaderEdit::SetAttribute(LPCTSTR name, LPCTSTR value) {
  if (_tcscmp(name, _T("iswidth")) == 0) {
    if (_tcscmp(value, _T("true")) == 0)
      is_width = true;
    else if (_tcscmp(value, _T("false")) == 0)
      is_width = false;
  }
  __super::SetAttribute(name, value);
}

void HeaderEdit::DoEvent(DuiLib::TEventUI& event) {
  __super::DoEvent(event);
  if (event.Type == DuiLib::UIEVENT_KILLFOCUS)
    ::PostMessage(m_pManager->GetPaintWindow(), MSG_HEADER_UPDATE_EDIT, 0, 0);

}

LRESULT HeaderEdit::MessageHandler(UINT umsg,
                                   WPARAM wparam,
                                   LPARAM lparam,
                                   bool& bhandled) {
  // Limit input chars.
  bool is_handle = false;
  if (umsg == WM_KEYDOWN || umsg == WM_KEYUP) {
    if (wparam == VK_DELETE || wparam == VK_BACK || wparam == VK_RETURN ||
        (wparam == VK_LEFT || wparam == VK_RIGHT))
      is_handle = false;
    else
      is_handle = true;
  } else if (umsg == WM_CHAR) {
    if (wparam >= 0x30 && wparam <= 0x39)
      is_handle = false;
    else
      is_handle = true;
  }

  if (!is_handle)
    return __super::MessageHandler(umsg, wparam, lparam, bhandled);

  return is_handle;
}

bool HeaderEdit::OnHeaderEditChanged() {
  // Used once.
  bool update_rect = update_rect_;
  update_rect_ = true;

  if (!m_bInited)
    return false;

  DuiLib::CDuiString result = GetText();
  if (last_text_ == result)
    return false;
  last_text_ = result;

  int value = _ttoi(result);
  // Not change anything in this case.
  if (value < kMinResolutionSqrt)
    return false;

  RECT record_rect = RecorderWnd::GetInstance()->GetRecordRealRect();
  int limit_width = screen_rect_.right - record_rect.left;
  int limit_height = screen_rect_.bottom - record_rect.top;

  if (is_width) {
    if (value > limit_width) {
      value = limit_width;

      result.Format(_T("%d"), value);
      SetText(result);
    }
    if (update_rect && last_value_ != value) {
      last_value_ = value;
      RecorderWnd::GetInstance()->UpdateRecordRealRect(value, -1);
    }
  } else {
    if (value > limit_height) {
      value = limit_height;

      result.Format(_T("%d"), value);
      SetText(result);
    }
    if (update_rect && last_value_ != value) {
      last_value_ = value;
      RecorderWnd::GetInstance()->UpdateRecordRealRect(-1, value);
    }
  }
  return true;
}

HeaderEdit::~HeaderEdit() {}
