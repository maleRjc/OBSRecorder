#include "capture_wnd.h"

#include <mutex>

#include "../../resource.h"
#include "../controller.h"
#include "../helper/util.h"
#include "canvas_container.h"
#include "def.h"
#include "message.h"


CaptureWnd* CaptureWnd::GetInstance() {
  static std::unique_ptr<CaptureWnd> g_CaptureWinInstance = nullptr;
  static std::once_flag g_Flag;
  std::call_once(g_Flag, [&]() { g_CaptureWinInstance.reset(new CaptureWnd); });
  return g_CaptureWinInstance.get();
}

CaptureWnd::~CaptureWnd() {}

DuiLib::CControlUI* CaptureWnd::CreateControl(LPCTSTR pstrClass) {
  if (_tcscmp(pstrClass, DUI_CTR_CANVASCONTAINER) == 0)
    return new CanvasContainer;
  return NULL;
}

void CaptureWnd::InitWindow() {
  ::GetWindowRect(GetHWND(), &window_rect_);

  desktop_image_ = m_PaintManager.FindControl(_T("desktopimage"));
  desktop_mask_ = m_PaintManager.FindControl(_T("desktopmask"));
  rect_size_lable_ = static_cast<DuiLib::CLabelUI*>(
      m_PaintManager.FindControl(_T("rectsize")));
  desktop_mask_border_ = m_PaintManager.FindControl(_T("desktopmaskborder"));
  desktop_canvas_container_ = static_cast<DuiLib::CContainerUI*>(
      m_PaintManager.FindControl(_T("desktopcanvascontainer")));

  InitWindowBackground();

  magnifying_bar_ = static_cast<DuiLib::CVerticalLayoutUI*>(
      m_PaintManager.FindControl(_T("magnifyingbar")));
  if (magnifying_bar_) {
    magnifying_img_ = m_PaintManager.FindControl(_T("magnifyingimg"));
    point_pos_ = static_cast<DuiLib::CLabelUI*>(
        m_PaintManager.FindControl(_T("pointpos")));
    point_color_ = static_cast<DuiLib::CLabelUI*>(
        m_PaintManager.FindControl(_T("pointcolor")));
    magnifying_bar_->SetVisible(false);
  }
}

LPCTSTR CaptureWnd::GetResourceID() const {
  return MAKEINTRESOURCE(IDR_ZIPRES1);
}

DuiLib::CDuiString CaptureWnd::GetZIPFileName() const {
  return ZIP_RESOURCE;
}

DuiLib::UILIB_RESOURCETYPE CaptureWnd::GetResourceType() const {
#ifdef _DEBUG
  return DuiLib::UILIB_FILE;
#else
  return DuiLib::UILIB_ZIPRESOURCE;
#endif
}

void CaptureWnd::OnClick(DuiLib::TNotifyUI& msg) {
  __super::OnClick(msg);
}

LRESULT CaptureWnd::OnLButtonDown(UINT umsg,
                                  WPARAM wparam,
                                  LPARAM lparam,
                                  BOOL& bhandled) {
  if (!clip_choiced_) {
    //desktop_canvas_container_->SetEnabled(false);
    clip_base_point_.x = GET_X_LPARAM(lparam);
    clip_base_point_.y = GET_Y_LPARAM(lparam);
  } else {
    bhandled = false;
  }
  return 0;
}

LRESULT CaptureWnd::OnLButtonUp(UINT umsg,
                                WPARAM wparam,
                                LPARAM lparam,
                                BOOL& bhandled) {
  if (!clip_choiced_) {
    clip_choiced_ = true;
    desktop_mask_border_->SetVisible(false);
    desktop_canvas_container_->SetVisible(true);
    // desktop_canvas_container_->SetEnabled(true);
    SetClipPadding(clip_padding_);
    ShowMagnifyingBar({GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)}, false);
    PostMessage(WM_CLOSE);
  } else {
    bhandled = false;
  }
  return 0;
}

LRESULT CaptureWnd::OnMouseMove(UINT umsg,
                                WPARAM wparam,
                                LPARAM lparam,
                                BOOL& bhandled) {
  if (!clip_choiced_) {
    if (wparam & MK_LBUTTON) {
      if (clip_base_point_.x != GET_X_LPARAM(lparam) ||
          clip_base_point_.y != GET_Y_LPARAM(lparam)) {
        clip_padding_.left =
            min(GET_X_LPARAM(lparam), clip_base_point_.x) - window_rect_.left;
        if (clip_padding_.left < 0)
          clip_padding_.left = 0;
        clip_padding_.top =
            min(GET_Y_LPARAM(lparam), clip_base_point_.y) - window_rect_.top;
        if (clip_padding_.top < 0)
          clip_padding_.top = 0;
        clip_padding_.right =
            window_rect_.right - max(GET_X_LPARAM(lparam), clip_base_point_.x);
        if (clip_padding_.right < 0)
          clip_padding_.right = 0;
        clip_padding_.bottom =
            window_rect_.bottom - max(GET_Y_LPARAM(lparam), clip_base_point_.y);
        if (clip_padding_.bottom < 0)
          clip_padding_.bottom = 0;
        desktop_mask_border_->SetVisible(false);
        desktop_canvas_container_->SetVisible(true);
        SetClipPadding(clip_padding_);
      }
    } else {
      ::PostMessage(GetHWND(), MSG_CHOICE_WINDOW_BY_CURSOR, 0, 0);
      //ChoiceWindowByCursor();
    }
    ShowMagnifyingBar({GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)});
  } else {
    bhandled = false;
  }
  return 0;
}

LRESULT CaptureWnd::OnClose(UINT umsg,
                            WPARAM wparam,
                            LPARAM lparam,
                            BOOL& bhandled) {
  ShowWindow(false);
  Controller::GetInstance()->SetCaptureRect(GetClipRect());
  Controller::GetInstance()->ShowRecorderWindow();
  return 0;
}

LRESULT CaptureWnd::OnKeyDown(UINT umsg,
                              WPARAM wparam,
                              LPARAM lparam,
                              BOOL& bhandled) {
  bhandled = true;
  switch (wparam) {
    case VK_ESCAPE:
      ::PostQuitMessage(0);
      break;
    default:
      bhandled = false;
      break;
  }
  return 0;
}

LRESULT CaptureWnd::HandleCustomMessage(UINT umsg,
                                        WPARAM wparam,
                                        LPARAM lparam,
                                        BOOL& bhandled) {
  switch (umsg) { 
    case MSG_CHOICE_WINDOW_BY_CURSOR:
      ChoiceWindowByCursor();
      break;
    case WM_RBUTTONUP:
      ::PostQuitMessage(0);
      break;
  }
  return 0;
}

void CaptureWnd::ShowMagnifyingBar(const POINT& pt, bool visiable) {
  //if (!magnifying_bar_)
  //  return;

  magnifying_bar_->SetVisible(visiable);
  //if (!visiable)
  //  return;

  constexpr int kAnchorOffset = 20;
  constexpr int kMagnifyingLength = 120;
  constexpr int kOtherAreaHeight = 40;
  int pos_l = pt.x + kAnchorOffset;
  int pos_t = pt.y + kAnchorOffset;
  int pos_r = pos_l + kMagnifyingLength;
  int pos_b = pos_t + kMagnifyingLength + kOtherAreaHeight;
  if (pos_r > window_rect_.right) {
    pos_l = pt.x - kAnchorOffset - kMagnifyingLength;
    pos_r = pos_l + kMagnifyingLength;
  }
  if (pos_b > window_rect_.bottom) {
    pos_t = pt.y - kAnchorOffset - kMagnifyingLength - kOtherAreaHeight;
    pos_b = pos_t + kMagnifyingLength + kOtherAreaHeight;
  }
  RECT magnifying_pos = {pos_l, pos_t, pos_r, pos_b};
  magnifying_bar_->SetPos(magnifying_pos);

  static HBITMAP magnifyingBmp = {0};
  RECT magnifyingR = {pt.x - kAnchorOffset, pt.y - kAnchorOffset, pt.x + kAnchorOffset, pt.y + kAnchorOffset};
  magnifyingBmp = DuiLib::CRenderEngine::GenerateBitmap(
      &m_PaintManager, desktop_image_, magnifyingR);
  m_PaintManager.RemoveImage(_T("magnifyingImg"));
  m_PaintManager.AddImage(_T("magnifyingImg"), magnifyingBmp, 2 * kAnchorOffset,
                          2 * kAnchorOffset, false);
  magnifying_img_->SetBkImage(_T("magnifyingImg"), true);

  static TCHAR lable_str[20] = {0};
  _stprintf_s(lable_str, 19, _T("зјБъ: (%d, %d)"), pt.x, pt.y);
  point_pos_->SetText(lable_str);

  COLORREF color = GetPixel(m_PaintManager.GetPaintDC(), pt.x, pt.y);
  static TCHAR lable_color_str[25] = {0};
  _stprintf_s(lable_color_str, 24, _T("RGB: (%d,%d,%d)"), GetRValue(color),
              GetGValue(color), GetBValue(color));
  point_color_->SetText(lable_color_str);

  magnifying_bar_->NeedParentUpdate();
}

void CaptureWnd::ShowLableForClipRectSize() {
  RECT clip_rect = GetClipRect();
  static TCHAR lable_str[11] = {0};
  _stprintf_s(lable_str, 10, _T("%dx%d"), clip_rect.right - clip_rect.left,
              clip_rect.bottom - clip_rect.top);

  constexpr int kLabelWidth = 80;
  constexpr int kLabelHeight = 20;
  constexpr int kLabelPadding = 3;

  RECT label_pos = {clip_rect.left + kLabelPadding,
                    clip_rect.top - kLabelHeight, 
                    clip_rect.left + kLabelWidth,
                    clip_rect.top - kLabelPadding};
  if (label_pos.top < window_rect_.top) {
    label_pos.top = clip_rect.top + kLabelPadding;
    label_pos.bottom = clip_rect.top + kLabelHeight;
  }
  if (label_pos.bottom > clip_rect.bottom) {
    label_pos.top = clip_rect.bottom + kLabelPadding;
    label_pos.bottom = clip_rect.bottom + kLabelHeight;
  }
  if (label_pos.right > window_rect_.right) {
    label_pos.left = window_rect_.right - kLabelWidth;
    label_pos.right = window_rect_.right;
  }
  rect_size_lable_->SetPos(label_pos);
  rect_size_lable_->SetTextColor(0xFFFFFFFF);
  rect_size_lable_->SetText(lable_str);
}

void CaptureWnd::SetClipPadding(RECT rc) {
  clip_padding_ = rc;
  TCHAR stringbuf[MAX_PATH];
  _stprintf_s(stringbuf, MAX_PATH - 1,
              _T("file='desktopimagemask' corner='%d,%d,%d,%d' hole='true'"),
              clip_padding_.left, clip_padding_.top, clip_padding_.right,
              clip_padding_.bottom);
  desktop_mask_->SetBkImage(stringbuf);
  desktop_mask_border_->SetPos(GetClipRect());
  desktop_canvas_container_->SetPos(GetCanvasContainerRect());

  ShowLableForClipRectSize();
}

RECT CaptureWnd::GetClipRect() const {
  RECT rc = clip_padding_;
  rc.left = rc.left + window_rect_.left;
  rc.top = rc.top + window_rect_.top;
  rc.right = window_rect_.right - rc.right;
  rc.bottom = window_rect_.bottom - rc.bottom;
  return rc;
}

RECT CaptureWnd::GetCanvasContainerRect() const {
  RECT rcInset = desktop_canvas_container_->GetInset();
  RECT rc = clip_padding_;
  rc.left = rc.left + window_rect_.left - rcInset.left;
  rc.top = rc.top + window_rect_.top - rcInset.left;
  rc.right = window_rect_.right - rc.right + rcInset.left;
  rc.bottom = window_rect_.bottom - rc.bottom + rcInset.left;
  return rc;
}

DuiLib::CDuiString CaptureWnd::GetSkinFolder() {
  return SKIN_FOLDER;
}

DuiLib::CDuiString CaptureWnd::GetSkinFile() {
  return XML_CAPTURE;
}

LPCTSTR CaptureWnd::GetWindowClassName() const {
  return WINDOW_CLASS_NAME;
}

void CaptureWnd::InitWindowBackground() {
  HBITMAP desktop_bmp = recorder_util::CreateDesktopBitmap();
  m_PaintManager.AddImage(_T("desktopimage"), desktop_bmp,
                          window_rect_.right - window_rect_.left,
                          window_rect_.bottom - window_rect_.top, true);
  HBITMAP desktop_bmp_mask = recorder_util::CreateDesktopMaskBitmap(GetHWND());
  m_PaintManager.AddImage(_T("desktopimagemask"), desktop_bmp_mask,
                          window_rect_.right - window_rect_.left,
                          window_rect_.bottom - window_rect_.top, true);
  if (desktop_image_)
    desktop_image_->SetBkImage(_T("desktopimage"));
  if (desktop_mask_)
    desktop_mask_->SetBkImage(_T("desktopimagemask"));
}

void CaptureWnd::ChoiceWindowByCursor() {
  RECT rc;
  HWND hwnd;
  // skip this window
  // forbid event of mouse and keyboard.
  ::EnableWindow(GetHWND(), FALSE);
  hwnd = recorder_util::SmallestWindowFromCursor(rc);
  ::EnableWindow(GetHWND(), TRUE);
  ::SetFocus(GetHWND());

  // rect for window and screen
  rc.left = max(rc.left - window_rect_.left, 0);
  rc.top = max(rc.top - window_rect_.top, 0);
  rc.right = max(window_rect_.right - rc.right, 0);
  rc.bottom = max(window_rect_.bottom - rc.bottom, 0);

  // select window which is in mouse hover rect, show borderUI.
  if (!::EqualRect(&rc, &clip_padding_)) {
    SetClipPadding(rc);
    desktop_mask_border_->SetVisible(true);
    desktop_canvas_container_->SetVisible(false);
  }
}

CaptureWnd::CaptureWnd() {
  RECT screen_rect = recorder_util::GetDesktopWindow();
  Create(NULL, WINDOW_NAME_CAPTURE, WS_POPUP, WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
         screen_rect.left, screen_rect.top,
         screen_rect.right - screen_rect.left,
         screen_rect.bottom - screen_rect.top);
}
