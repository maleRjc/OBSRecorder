#include "cursor_custom.h"

#include <mutex>

#include "recorder_wnd.h"

#define COLOR_ARGB_A 120

namespace {
const int kARGB_A = COLOR_ARGB_A;
const DWORD kDefCol = 0x00ffffff;
HBITMAP CreateTransparentHBitmap(HWND hwnd, RECT rect, DWORD color = kDefCol);
}  // namespace

using namespace Gdiplus;

CursorCustom::CursorCustom(std::string name, int size, DWORD color)
    : str_name_(name), n_size_(size), dw_color_(color) {
  h_cursor_ = NULL;
}

CursorCustom::~CursorCustom() {
  DeleteHCursor();
}

HCURSOR CursorCustom::GetHCursor() {
  if (h_cursor_ == NULL)
    GenerateCustomImageForCursor();
  return h_cursor_;
}

void CursorCustom::GenerateCustomImageForCursor() {
  DeleteHCursor();
  HBITMAP hbmp = GenerateImage();
  ICONINFO ii = {0};
  ii.fIcon = TRUE;
  ii.hbmColor = hbmp;
  ii.hbmMask = hbmp;
  h_cursor_ = ::CreateIconIndirect(&ii);
  ::DeleteObject(hbmp);
}

HBITMAP CursorCustom::GenerateImage() {
  HBITMAP hbmp = CreateTransparentHBitmap(RecorderWnd::GetInstance()->GetHWND(),
                                          RECT{0, 0, n_size_, n_size_});
  HDC dc_drawcursor = ::CreateCompatibleDC(NULL);
  ::SelectObject(dc_drawcursor, hbmp);

  Gdiplus::Graphics graphics(dc_drawcursor);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  Color myColor(255, GetBValue(dw_color_), GetGValue(dw_color_),
                GetRValue(dw_color_));
  graphics.Clear(myColor);

  SolidBrush brush(Color(kARGB_A, GetBValue(dw_color_), GetGValue(dw_color_),
                         GetRValue(dw_color_)));
  graphics.FillEllipse(&brush, 0, 0, n_size_ - 1, n_size_ - 1);

  ::SelectObject(dc_drawcursor, NULL);
  ::DeleteDC(dc_drawcursor);
  //::DeleteDC(dc);
  return hbmp;
}

void CursorCustom::DeleteHCursor() {
  if (h_cursor_) {
    ::DestroyIcon(h_cursor_); 
    h_cursor_ = NULL;
  }
}

std::string CursorManager::GenerateName(int size, DWORD color) {
  char name[MAX_PATH];
  sprintf_s(name, "size%d-color%d", size, (uint32_t)color);
  return std::string(name);
}

CursorManager* CursorManager::GetInstance() {
  static std::unique_ptr<CursorManager> m_instance = NULL;
  static std::once_flag g_Flag;
  std::call_once(g_Flag, [&]() {
    m_instance.reset(new CursorManager);
  });
  return m_instance.get();
}

CursorManager::~CursorManager() {}

void CursorManager::AddCursor(std::string name, int size, DWORD color) {
  if (cursor_set_.find(name) != cursor_set_.end())
    return;
  cursor_set_.emplace(name, std::make_unique<CursorCustom>(name, size, color));
}

void CursorManager::RemoveCursor(std::string name) {
  auto iter = cursor_set_.find(name);
  if (iter != cursor_set_.end())
    cursor_set_.erase(name);
}

HCURSOR CursorManager::GetCursor(std::string name) {
  HCURSOR hcursor = NULL;
  auto iter = cursor_set_.find(name);
  if (iter != cursor_set_.end() && iter->second) {
    hcursor = iter->second->GetHCursor();
  }
  return hcursor;
}

HCURSOR CursorManager::AddAndGetCursor(std::string name,
                                       int size,
                                       DWORD color) {
  AddCursor(name, size, color);
  return cursor_set_[name]->GetHCursor();
}

CursorManager::CursorManager() {}

namespace {

HBITMAP CreateTransparentHBitmap(HWND hwnd, RECT rect, DWORD color) {
  rect.right -= rect.left;
  rect.bottom -= rect.top;
  rect.left = rect.top = 0;

  HDC hdcmem = ::CreateCompatibleDC(NULL);
  HDC hdc = ::GetDC(hwnd);
  HBITMAP hbmp = ::CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
  HGDIOBJ hold = ::SelectObject(hdcmem, hbmp);
  DuiLib::CRenderEngine::DrawColor(hdcmem, rect, color);

  ::SelectObject(hdcmem, hold);
  ::DeleteObject(hold);
  ::ReleaseDC(hwnd, hdc);
  ::DeleteDC(hdcmem);
  return hbmp;
}

}  // namespace