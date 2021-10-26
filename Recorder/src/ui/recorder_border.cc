#include "recorder_border.h"

#include "../helper/util.h"
#include "def.h"


namespace {
// Minus 1 to visual effect.
constexpr int kSignRadius = RECORDERWND_BORDER_CIRCLE_RADIUS - 1;
}

CRecorderBorder::CRecorderBorder() {
}

LPCTSTR CRecorderBorder::GetClass() const {
  return DUI_CTR_RECORDER_BORDER;
}

LPVOID CRecorderBorder::GetInterface(LPCTSTR name) {
  if (_tcscmp(name, DUI_CTR_RECORDER_BORDER) == 0)
    return static_cast<CRecorderBorder*>(this);
  return __super::GetInterface(name);
}

void CRecorderBorder::PaintBkImage(HDC hdc) {}

void CRecorderBorder::PaintBorder(HDC hdc) {
  RECT rect = m_rcItem;
  LONG width = rect.right - rect.left;
  LONG height = rect.bottom - rect.top;

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  Gdiplus::Pen color_pen(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
      m_nBorderSize);
  graphics.DrawRectangle(&color_pen, (INT)rect.left + kSignRadius / 2 + 1,
                         (INT)rect.top + kSignRadius / 2 + 1,
                         width - kSignRadius - 1, height - kSignRadius - 1);

  Gdiplus::SolidBrush color_brush(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)));
  graphics.FillEllipse(&color_brush, rect.left, rect.top, kSignRadius,
                       kSignRadius);
  graphics.FillEllipse(&color_brush, rect.left, rect.bottom - kSignRadius - 1,
                       kSignRadius, kSignRadius);
  graphics.FillEllipse(&color_brush, rect.right - kSignRadius - 1, rect.top,
                       kSignRadius, kSignRadius);
  graphics.FillEllipse(&color_brush, rect.right - kSignRadius - 1,
                       rect.bottom - kSignRadius - 1, kSignRadius, kSignRadius);

  graphics.FillEllipse(&color_brush, rect.left + (width - kSignRadius) / 2,
                       rect.top, kSignRadius, kSignRadius);
  graphics.FillEllipse(&color_brush, rect.left,
                       rect.top + (height - kSignRadius) / 2, kSignRadius,
                       kSignRadius);
  graphics.FillEllipse(&color_brush, rect.right - kSignRadius - 1,
                       rect.top + (height - kSignRadius) / 2, kSignRadius,
                       kSignRadius);
  graphics.FillEllipse(&color_brush, rect.left + (width - kSignRadius) / 2,
                       rect.bottom - kSignRadius - 1, kSignRadius, kSignRadius);

  DWORD color_white = 0xffffffff;
  Gdiplus::Pen color_pen2(
      Gdiplus::Color(255, GetBValue(color_white),
                     GetGValue(color_white), GetRValue(color_white)));
  graphics.DrawEllipse(&color_pen2, rect.left, rect.top, kSignRadius,
                       kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.left, rect.bottom - kSignRadius - 1,
                       kSignRadius, kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.right - kSignRadius - 1, rect.top,
                       kSignRadius, kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.right - kSignRadius - 1,
                       rect.bottom - kSignRadius - 1, kSignRadius, kSignRadius);

  graphics.DrawEllipse(&color_pen2, rect.left + (width - kSignRadius) / 2,
                       rect.top, kSignRadius, kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.left,
                       rect.top + (height - kSignRadius) / 2, kSignRadius,
                       kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.right - kSignRadius - 1,
                       rect.top + (height - kSignRadius) / 2, kSignRadius,
                       kSignRadius);
  graphics.DrawEllipse(&color_pen2, rect.left + (width - kSignRadius) / 2,
                       rect.bottom - kSignRadius - 1, kSignRadius, kSignRadius);
}

CRecorderBorder::~CRecorderBorder() {}

