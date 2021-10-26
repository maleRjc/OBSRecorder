#include "canvas_container.h"

#include "capture_wnd.h"
#include "def.h"

namespace {
// Minus 1 to visual effect.
constexpr int kSignRadius = RECORDERWND_BORDER_CIRCLE_RADIUS - 1;
}  // namespace

CanvasContainer::CanvasContainer() : cursor_(0), button_state_(0) {
  clip_base_point_.x = clip_base_point_.y = 0;
  last_mouse_.x = last_mouse_.y = 0;
}

CanvasContainer::~CanvasContainer() {}

LPCTSTR CanvasContainer::GetClass() const {
  return DUI_CTR_CANVASCONTAINER;
}

LPVOID CanvasContainer::GetInterface(LPCTSTR str_name) {
  if (_tcscmp(str_name, DUI_CTR_CANVASCONTAINER) == 0)
    return static_cast<CanvasContainer*>(this);
  return CContainerUI::GetInterface(str_name);
}

UINT CanvasContainer::GetControlFlags() const {
  return UIFLAG_SETCURSOR;
}

RECT CanvasContainer::GetSizerRect(int index) {
  LONG middle_x = (m_rcItem.left + m_rcItem.right) / 2;
  LONG middle_y = (m_rcItem.top + m_rcItem.bottom) / 2;
  LONG sizer_width = m_rcInset.left * 2;
  constexpr LONG kOffset = 1;
  switch (index) {
    case 0:
      return DuiLib::CDuiRect(m_rcItem.left + kOffset, m_rcItem.top + kOffset,
                              m_rcItem.left + sizer_width + kOffset,
                              m_rcItem.top + sizer_width + kOffset);
    case 1:
      return DuiLib::CDuiRect(
          middle_x - sizer_width / 2, m_rcItem.top + kOffset,
          middle_x + sizer_width / 2, m_rcItem.top + sizer_width + kOffset);
    case 2:
      return DuiLib::CDuiRect(
          m_rcItem.right - sizer_width - kOffset,
          m_rcItem.top + kOffset, m_rcItem.right - kOffset,
          m_rcItem.top + sizer_width + kOffset);
    case 3:
      return DuiLib::CDuiRect(
          m_rcItem.left + kOffset, middle_y - sizer_width / 2,
          m_rcItem.left + sizer_width + kOffset, middle_y + sizer_width / 2);
    case 4:
      break;
    case 5:
      return DuiLib::CDuiRect(
          m_rcItem.right - sizer_width - kOffset,
          middle_y - sizer_width / 2, m_rcItem.right - kOffset,
          middle_y + sizer_width / 2);
    case 6:
      return DuiLib::CDuiRect(
          m_rcItem.left + kOffset, m_rcItem.bottom - sizer_width - kOffset,
          m_rcItem.left + sizer_width + kOffset, m_rcItem.bottom - kOffset);
    case 7:
      return DuiLib::CDuiRect(
          middle_x - sizer_width / 2, m_rcItem.bottom - sizer_width - kOffset,
          middle_x + sizer_width / 2, m_rcItem.bottom - kOffset);
    case 8:
      return DuiLib::CDuiRect(
          m_rcItem.right - sizer_width - kOffset,
          m_rcItem.bottom - sizer_width - kOffset,
          m_rcItem.right - kOffset, m_rcItem.bottom - kOffset);
    default:
      break;
  }
  return DuiLib::CDuiRect();
}

LPTSTR CanvasContainer::GetSizerCursor(POINT& pt, int& cursor_id) {
  LONG sizer_width = m_rcInset.left * 2;
  LONG sizer_to_root = 20;
  RECT rc_root = m_pManager->GetRoot()->GetPos();

  cursor_id = -1;
  for (int i = 8; i >= 0; --i) {
    RECT rc = GetSizerRect(i);
    if (::PtInRect(&rc, pt)) {
      cursor_id = i;
      break;
    }
  }
  if (cursor_id < 0 && rc_root.bottom - m_rcItem.bottom < sizer_to_root) {
    RECT rc = DuiLib::CDuiRect(m_rcItem.left, m_rcItem.bottom - sizer_width,
                       m_rcItem.right, m_rcItem.bottom);
    if (::PtInRect(&rc, pt)) {
      cursor_id = 7;
    }
  }
  if (cursor_id < 0 && rc_root.right - m_rcItem.right < sizer_to_root) {
    RECT rc = DuiLib::CDuiRect(m_rcItem.right - sizer_width, m_rcItem.top,
                       m_rcItem.right, m_rcItem.bottom);
    if (::PtInRect(&rc, pt)) {
      cursor_id = 5;
    }
  }
  if (cursor_id < 0 && m_rcItem.left - rc_root.left < sizer_to_root) {
    RECT rc = DuiLib::CDuiRect(m_rcItem.left, m_rcItem.top,
                               m_rcItem.left + sizer_width,
                       m_rcItem.bottom);
    if (::PtInRect(&rc, pt)) {
      cursor_id = 3;
    }
  }
  if (cursor_id < 0 && m_rcItem.top - rc_root.top < sizer_to_root) {
    RECT rc = DuiLib::CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.right,
                       m_rcItem.top + sizer_width);
    if (::PtInRect(&rc, pt)) {
      cursor_id = 1;
    }
  }
  if (cursor_id < 0)
    cursor_id = 4;
  switch (cursor_id) {
    case 0:
      return IDC_SIZENWSE;
    case 1:
      return IDC_SIZENS;
    case 2:
      return IDC_SIZENESW;
    case 3:
      return IDC_SIZEWE;
    case 4:
      return IDC_SIZEALL;
    case 5:
      return IDC_SIZEWE;
    case 6:
      return IDC_SIZENESW;
    case 7:
      return IDC_SIZENS;
    case 8:
      return IDC_SIZENWSE;
    default:
      return IDC_SIZEALL;
  }
}

void CanvasContainer::PaintBorder(HDC hdc) {
  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  RECT rc = m_rcItem;
  constexpr LONG kOffset = 1;
  rc.left += m_rcInset.left;
  rc.top += m_rcInset.top;
  rc.right -= m_rcInset.right - kOffset;
  rc.bottom -= m_rcInset.bottom - kOffset;
  DuiLib::CRenderEngine::DrawRect(hdc, rc, m_nBorderSize, m_dwBorderColor);

  if (round_visible_)
    for (int i = 0; i < 9; ++i) {
      // DuiLib::CRenderEngine::DrawColor(hdc, GetSizerRect(i),
      // m_dwBorderColor);
      Gdiplus::SolidBrush color_brush(Gdiplus::Color(
          255, GetBValue(m_dwBorderColor), GetGValue(m_dwBorderColor),
          GetRValue(m_dwBorderColor)));
      RECT rect = GetSizerRect(i);
      graphics.FillEllipse(&color_brush, rect.left, rect.top, kSignRadius,
                           kSignRadius);

      DWORD color_white = 0xffffffff;
      Gdiplus::Pen color_pen2(Gdiplus::Color(255, GetBValue(color_white),
                                             GetGValue(color_white),
                                             GetRValue(color_white)));
      graphics.DrawEllipse(&color_pen2, rect.left, rect.top, kSignRadius,
                           kSignRadius);
    }
}

void CanvasContainer::DoEvent(DuiLib::TEventUI& event) {
  if (event.Type == DuiLib::UIEVENT_SETCURSOR && IsEnabled()) {
    if ((button_state_ & UISTATE_CAPTURED) != 0)
      return;
    ::SetCursor(::LoadCursor(
        NULL, MAKEINTRESOURCE(GetSizerCursor(event.ptMouse, cursor_))));
    return;
  }
  if (event.Type == DuiLib::UIEVENT_BUTTONDOWN && IsEnabled()) {
    button_state_ |= UISTATE_CAPTURED;
    last_mouse_ = event.ptMouse;
    RECT window_rect = CaptureWnd::GetInstance()->GetWindowRect();
    RECT clip_padding = CaptureWnd::GetInstance()->GetClipPadding();
    switch (cursor_) {
      case 0:
      case 1:
      case 3:
        clip_base_point_.x = window_rect.right - clip_padding.right;
        clip_base_point_.y = window_rect.bottom - clip_padding.bottom;
        break;
      case 2:
      case 5:
        clip_base_point_.x = window_rect.left + clip_padding.left;
        clip_base_point_.y = window_rect.bottom - clip_padding.bottom;
        break;
      case 6:
      case 7:
        clip_base_point_.x = window_rect.right - clip_padding.right;
        clip_base_point_.y = window_rect.top + clip_padding.top;
        break;
      case 8:
      default:
        clip_base_point_.x = window_rect.left + clip_padding.left;
        clip_base_point_.y = window_rect.top + clip_padding.top;
        break;
    }
    return;
  }
  if (event.Type == DuiLib::UIEVENT_BUTTONUP) {
    if ((button_state_ & UISTATE_CAPTURED) != 0) {
      button_state_ &= ~UISTATE_CAPTURED;
    }
    return;
  } else if (event.Type == DuiLib::UIEVENT_MOUSEMOVE) {
    if ((button_state_ & UISTATE_CAPTURED) == 0)
      return;
    RECT window_rect = CaptureWnd::GetInstance()->GetWindowRect();
    RECT clip_padding = CaptureWnd::GetInstance()->GetClipPadding();
    switch (cursor_) {
      case 0:
      case 2:
      case 6:
      case 8:
        clip_padding.left =
            min(event.ptMouse.x, clip_base_point_.x) - window_rect.left;
        if (clip_padding.left < 0)
          clip_padding.left = 0;
        clip_padding.top =
            min(event.ptMouse.y, clip_base_point_.y) - window_rect.top;
        if (clip_padding.top < 0)
          clip_padding.top = 0;
        clip_padding.right =
            window_rect.right - max(event.ptMouse.x, clip_base_point_.x);
        if (clip_padding.right < 0)
          clip_padding.right = 0;
        clip_padding.bottom =
            window_rect.bottom - max(event.ptMouse.y, clip_base_point_.y);
        if (clip_padding.bottom < 0)
          clip_padding.bottom = 0;
        break;
      case 1:
      case 7:
        clip_padding.top =
            min(event.ptMouse.y, clip_base_point_.y) - window_rect.top;
        if (clip_padding.top < 0)
          clip_padding.top = 0;
        clip_padding.bottom =
            window_rect.bottom - max(event.ptMouse.y, clip_base_point_.y);
        if (clip_padding.bottom < 0)
          clip_padding.bottom = 0;
        break;
      case 3:
      case 5:
        clip_padding.left =
            min(event.ptMouse.x, clip_base_point_.x) - window_rect.left;
        if (clip_padding.left < 0)
          clip_padding.left = 0;
        clip_padding.right =
            window_rect.right - max(event.ptMouse.x, clip_base_point_.x);
        if (clip_padding.right < 0)
          clip_padding.right = 0;
        break;
      default: {
        LONG cx = event.ptMouse.x - last_mouse_.x;
        LONG cy = event.ptMouse.y - last_mouse_.y;
        if (clip_padding.left + cx >= 0 && clip_padding.right - cx >= 0) {
          clip_padding.left += cx;
          clip_padding.right -= cx;
        }
        if (clip_padding.top + cy >= 0 && clip_padding.bottom - cy >= 0) {
          clip_padding.top += cy;
          clip_padding.bottom -= cy;
        }
      } break;
    }
    last_mouse_ = event.ptMouse;
    CaptureWnd::GetInstance()->SetClipPadding(clip_padding);
  }
  CContainerUI::DoEvent(event);
}

void CanvasContainer::SetAttribute(LPCTSTR pstrname, LPCTSTR pstrvalue) {
  if (_tcscmp(pstrname, _T("roundvisible")) == 0)
    round_visible_ = (_tcscmp(pstrvalue, _T("true")) == 0);
  else
    __super::SetAttribute(pstrname, pstrvalue);
}
