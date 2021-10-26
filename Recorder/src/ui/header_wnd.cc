#include "header_wnd.h"

#include "../../resource.h"
#include "../helper/util.h"
#include "def.h"
#include "header_edit.h"
#include "message.h"
#include "recorder_wnd.h"


HeaderWnd::HeaderWnd(HWND parent) {
  Create(parent, WINDOW_NAME_HEADER, WS_POPUP, 
         WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 0);
}

HeaderWnd::~HeaderWnd() {}

void HeaderWnd::InitWindow() {
  record_width_richedit_ =
      static_cast<HeaderEdit*>(m_PaintManager.FindControl(_T("record-width")));
  record_height_richedit_ =
      static_cast<HeaderEdit*>(m_PaintManager.FindControl(_T("record-height")));
}

LPCTSTR HeaderWnd::GetResourceID() const {
  return MAKEINTRESOURCE(IDR_ZIPRES1);
}

DuiLib::CDuiString HeaderWnd::GetZIPFileName() const {
  return ZIP_RESOURCE;
}

DuiLib::UILIB_RESOURCETYPE HeaderWnd::GetResourceType() const {
#ifdef _DEBUG
  return DuiLib::UILIB_FILE;
#else
  return DuiLib::UILIB_ZIPRESOURCE;
#endif
}

LRESULT HeaderWnd::OnKillFocus(UINT umsg,
                               WPARAM wparam,
                               LPARAM lparam,
                               BOOL& bhandled) {
  PostMessage(MSG_HEADER_UPDATE_EDIT);
  return 0;
}

LRESULT HeaderWnd::OnKeyDown(UINT umsg,
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

LRESULT HeaderWnd::OnClose(UINT umsg,
                           WPARAM wparam,
                           LPARAM lparam,
                           BOOL& bhandled) {
  RecorderWnd::GetInstance()->PostMessage(WM_CLOSE, wparam);
  return 0;
}

LRESULT HeaderWnd::HandleCustomMessage(UINT umsg,
                                       WPARAM wparam,
                                       LPARAM lparam,
                                       BOOL& bhandled) {
  switch (umsg) {
    case WM_MOVING:
      OnMoving(wparam, lparam);
      break;
    case MSG_HEADER_EDIT_CHANGED:
      OnEditChanged(wparam, lparam);
      break;
    // Don't resize recorderwnd by this message, 
    // so it must be send by recorderwnd.
    case MSG_HEADER_UPDATE_EDIT:
      OnUpdatingEdit(wparam, lparam);
      break;
  }
  return 0;
}

void HeaderWnd::Notify(DuiLib::TNotifyUI& msg) {
  if (msg.sType == DUI_MSGTYPE_TEXTCHANGED)
    PostMessage(MSG_HEADER_EDIT_CHANGED, WPARAM(msg.pSender));
}

DuiLib::CControlUI* HeaderWnd::CreateControl(LPCTSTR pstrClass) {
  if (_tcscmp(pstrClass, DUI_CTR_HEADER_EDIT) == 0)
    return new HeaderEdit;
  return NULL;
}

RECT HeaderWnd::GetWindowRect() {
  RECT rect;
  ::GetWindowRect(GetHWND(), &rect);
  return rect;
}

void HeaderWnd::SetEnableEdit(bool enable) {
  record_width_richedit_->SetEnabled(enable);
  record_height_richedit_->SetEnabled(enable);
}

DuiLib::CDuiString HeaderWnd::GetSkinFolder() {
  return SKIN_FOLDER;
}

DuiLib::CDuiString HeaderWnd::GetSkinFile() {
  return XML_HEADER;
}

LPCTSTR HeaderWnd::GetWindowClassName() const {
  return WINDOW_CLASS_NAME;
}

void HeaderWnd::UpdateRecorderWindowRect(int x, int y) {
  RecorderWnd* record_wnd = RecorderWnd::GetInstance();
  RECT record_wnd_rect = record_wnd->GetWindowRect();
  record_wnd_rect = recorder_util::MoveRect(record_wnd_rect, x, y);

  ::MoveWindow(record_wnd->GetHWND(), record_wnd_rect.left, record_wnd_rect.top,
               record_wnd_rect.right - record_wnd_rect.left,
               record_wnd_rect.bottom - record_wnd_rect.top, true);
  record_wnd->UpdateToolbar();

  record_wnd->PostMessage(MSG_CROP_RECORD_RECT);
}

void HeaderWnd::OnMoving(WPARAM wparam, LPARAM lparam) {
  constexpr int kPaddingTopBottom = RECORDERWND_BORDER_CIRCLE_RADIUS / 2;
  const int kDisablePadding = RecorderWnd::CalculateDisablePadding();

  LPRECT new_rect = (LPRECT)lparam;
  int headerwnd_width = new_rect->right - new_rect->left;
  int headerwnd_height = new_rect->bottom - new_rect->top;

  RecorderWnd* record_wnd = RecorderWnd::GetInstance();
  RECT record_wnd_rect = record_wnd->GetWindowRect();
  record_wnd_rect =
      recorder_util::ClipRectByPadding(record_wnd_rect, kDisablePadding);
  int recorderwnd_width = record_wnd_rect.right - record_wnd_rect.left;
  int recorderwnd_height = record_wnd_rect.bottom - record_wnd_rect.top;

  RECT moving_range = recorder_util::GetDesktopWindow();

  switch (record_wnd->GetHeaderDirection()) {
    case RecorderWnd::HeaderDirection::kTop:
      moving_range.right -= max(headerwnd_width, recorderwnd_width);
      moving_range.bottom -= (headerwnd_height + recorderwnd_height);
      break;
    case RecorderWnd::HeaderDirection::kBottom:
      moving_range.top += recorderwnd_height;
      moving_range.right -= max(headerwnd_width, recorderwnd_width);
      moving_range.bottom -= headerwnd_height;
      break;
    case RecorderWnd::HeaderDirection::kLeft:
      //moving_range.top += kPaddingTopBottom;
      moving_range.right -= (headerwnd_width + recorderwnd_width);
      moving_range.bottom -=
          max(headerwnd_height, recorderwnd_height - kPaddingTopBottom);
      break;
    case RecorderWnd::HeaderDirection::kRight:
      moving_range.left += recorderwnd_width;
      //moving_range.top += kPaddingTopBottom;
      moving_range.right -= headerwnd_width;
      moving_range.bottom -=
          max(headerwnd_height, recorderwnd_height - kPaddingTopBottom);
      break;
    case RecorderWnd::HeaderDirection::kInside:
      moving_range.right -= (recorderwnd_width - kPaddingTopBottom);
      moving_range.bottom -= (recorderwnd_height - kPaddingTopBottom);
      break;
  }

  if (new_rect->left < moving_range.left)
    new_rect->left = moving_range.left;
  if (new_rect->top < moving_range.top)
    new_rect->top = moving_range.top;
  if (new_rect->left > moving_range.right)
    new_rect->left = moving_range.right;
  if (new_rect->top > moving_range.bottom)
    new_rect->top = moving_range.bottom;
  new_rect->right = new_rect->left + headerwnd_width;
  new_rect->bottom = new_rect->top + headerwnd_height;

  int record_wnd_new_x = 0;
  int record_wnd_new_y = 0;

  switch (record_wnd->GetHeaderDirection()) {
    case RecorderWnd::HeaderDirection::kTop:
      record_wnd_new_x = new_rect->left - kDisablePadding;
      record_wnd_new_y = new_rect->bottom - kDisablePadding;
      break;
    case RecorderWnd::HeaderDirection::kBottom:
      record_wnd_new_x = new_rect->left - kDisablePadding;
      record_wnd_new_y = new_rect->top - recorderwnd_height - kDisablePadding;
      break;
    case RecorderWnd::HeaderDirection::kLeft:
      record_wnd_new_x = new_rect->right - kDisablePadding;
      record_wnd_new_y = new_rect->top - kPaddingTopBottom;
      break;
    case RecorderWnd::HeaderDirection::kRight:
      record_wnd_new_x = new_rect->left - recorderwnd_width - kDisablePadding;
      record_wnd_new_y = new_rect->top - kPaddingTopBottom;
      break;
    case RecorderWnd::HeaderDirection::kInside:
      record_wnd_new_x = new_rect->left - kPaddingTopBottom;
      record_wnd_new_y = new_rect->top - kPaddingTopBottom;
      break;
  }
  UpdateRecorderWindowRect(record_wnd_new_x, record_wnd_new_y);
}

void HeaderWnd::OnEditChanged(WPARAM wparam, LPARAM lparam) {
  DuiLib::CControlUI* sender = reinterpret_cast<DuiLib::CControlUI*>(wparam);
  if (sender == record_width_richedit_) {
    record_width_richedit_->OnHeaderEditChanged();
  } else if (sender == record_height_richedit_) {
    record_height_richedit_->OnHeaderEditChanged();
  }
}

void HeaderWnd::OnUpdatingEdit(WPARAM wparam, LPARAM lparam) {
  RecorderWnd* record_wnd = RecorderWnd::GetInstance();
  RECT real_record_rect = record_wnd->GetRecordRealRect();
  int rrr_width = real_record_rect.right - real_record_rect.left;
  int rrr_height = real_record_rect.bottom - real_record_rect.top;
  DuiLib::CDuiString text_v;
  text_v.Format(_T("%d"), rrr_width);
  record_width_richedit_->SetText(text_v, false);
  text_v.Format(_T("%d"), rrr_height);
  record_height_richedit_->SetText(text_v, false);
}
