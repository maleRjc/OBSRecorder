#include "util.h"

#include "UIlib.h"

#define ABS_FLOAT_0 0.0001

namespace {

POINT minusPOINT(const POINT& A, const POINT& B) {
  return POINT{A.x - B.x, A.y - B.y};
}

POINT plusPOINT(const POINT& A, const POINT& B) {
  return POINT{A.x + B.x, A.y + B.y};
}

}

RECT recorder_util::GetDesktopWindow() {
  RECT screen_rect;
  ::GetWindowRect(::GetDesktopWindow(), &screen_rect);
  return screen_rect;
}

RECT recorder_util::MoveRect(RECT rect, int x, int y) {
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  RECT new_rect = {x, y, x + width, y + height};
  return new_rect;
}

bool recorder_util::IsEqualRect(const RECT& rect1, const RECT& rect2) {
  if (rect1.left == rect2.left && rect1.right == rect2.right &&
      rect1.top == rect2.top && rect1.bottom == rect2.bottom)
    return true;
  return false;
}

POINT recorder_util::GetPointFromLimitRect(const POINT& pt, const RECT& rect) {
  POINT new_point = pt;
  if (!::PtInRect(&rect, new_point)) {
    new_point.x < rect.left ? new_point.x = rect.left : new_point.x;
    new_point.x > rect.right ? new_point.x = rect.right : new_point.x;
    new_point.y < rect.top ? new_point.y = rect.top : new_point.y;
    new_point.y > rect.bottom ? new_point.y = rect.bottom : new_point.y;
  }
  return new_point;
}

RECT recorder_util::ClipRectByPadding(RECT rect, int padding) {
  rect.left += padding;
  rect.top += padding;
  rect.right -= padding;
  rect.bottom -= padding;
  return rect;
}

bool recorder_util::IsRectInRect(const RECT& internal_rect, const RECT& external_rect) {
  if (internal_rect.left < external_rect.left)
    return false;
  if (internal_rect.top < external_rect.top)
    return false;
  if (internal_rect.right > external_rect.right)
    return false;
  if (internal_rect.bottom > external_rect.bottom)
    return false;

  return true;
}

bool recorder_util::IsInCircle(const POINT& pt,
                               const POINT& circle_center,
                               int r) {
  int distance = (pt.x - circle_center.x) * (pt.x - circle_center.x) +
                 (pt.y - circle_center.y) * (pt.y - circle_center.y);
  if (distance <= r * r)
    return true;
  return false;
}

bool recorder_util::IsPointInRect(const point_float& P,
                                  const point_float& A,
                                  const point_float& B,
                                  const point_float& C,
                                  const point_float& D) {
  int x = P.x;
  int y = P.y;
  int a = (B.x - A.x) * (y - A.y) - (B.y - A.y) * (x - A.x);
  int b = (C.x - B.x) * (y - B.y) - (C.y - B.y) * (x - B.x);
  int c = (D.x - C.x) * (y - C.y) - (D.y - C.y) * (x - C.x);
  int d = (A.x - D.x) * (y - D.y) - (A.y - D.y) * (x - D.x);
  if ((a > 0 && b > 0 && c > 0 && d > 0) ||
      (a < 0 && b < 0 && c < 0 && d < 0)) {
    return true;
  }
  return false;
}

float recorder_util::GetTriangleSquar(const point_float& pf0,
                                      const point_float& pf1,
                                      const point_float& pf2) {
  point_float AB, BC;
  AB.x = pf1.x - pf0.x;
  AB.y = pf1.y - pf0.y;
  BC.x = pf2.x - pf1.x;
  BC.y = pf2.y - pf1.y;
  return fabs((AB.x * BC.y - AB.y * BC.x)) / 2.0f;
}

bool recorder_util::IsInTriangle(const point_float& A,
                                 const point_float& B,
                                 const point_float& C,
                                 const point_float& D) {
  float SABC, SADB, SBDC, SADC, Sum;
  SABC = GetTriangleSquar(A, B, C);
  SADB = GetTriangleSquar(A, D, B);
  SBDC = GetTriangleSquar(B, D, C);
  SADC = GetTriangleSquar(A, D, C);

  Sum = SADB + SBDC + SADC;

  if ((-ABS_FLOAT_0 < (SABC - Sum)) && ((SABC - Sum) < ABS_FLOAT_0)) {
    return 1;
  }
  return 0;
}

HBITMAP recorder_util::CreateDesktopBitmap() {
  HWND hwnd_desktop = ::GetDesktopWindow();
  RECT rect;
  GetWindowRect(hwnd_desktop, &rect);
  rect.right -= rect.left;
  rect.bottom -= rect.top;
  rect.left = rect.top = 0;

  HDC hdc_desktop = GetDC(hwnd_desktop);
  HDC hdc_mem = CreateCompatibleDC(hdc_desktop);
  HBITMAP hbmp = CreateCompatibleBitmap(hdc_desktop, rect.right, rect.bottom);
  HGDIOBJ holdobj = SelectObject(hdc_mem, hbmp);
  ::BitBlt(hdc_mem, 0, 0, rect.right, rect.bottom, hdc_desktop, 0, 0,
           SRCCOPY | CAPTUREBLT);
  ::ReleaseDC(hwnd_desktop, hdc_desktop);
  ::DeleteDC(hdc_mem);
  return hbmp;
}

HBITMAP recorder_util::CreateDesktopMaskBitmap(HWND hwnd) {
  HDC hdc_mem = CreateCompatibleDC(NULL);
  RECT rect;
  GetWindowRect(::GetDesktopWindow(), &rect);
  rect.right -= rect.left;
  rect.bottom -= rect.top;
  rect.left = rect.top = 0;

  HBITMAP hbmp = NULL;

  HDC hdc = GetDC(hwnd);
  hbmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
  ReleaseDC(hwnd, hdc);

  HGDIOBJ holdobj = SelectObject(hdc_mem, hbmp);
  DuiLib::CRenderEngine::DrawColor(hdc_mem, rect, 0x6F000000);
  SelectObject(hdc_mem, holdobj);

  DeleteObject(hdc_mem);
  return hbmp;
}

HWND recorder_util::SmallestWindowFromCursor(RECT& rc_window) {
  // find bottom window
  HWND hwnd, htemp;
  POINT pt;
  ::GetCursorPos(&pt);
  // get bottom window£¬skip all disable windows and in-visible windows.
  hwnd = ::ChildWindowFromPointEx(::GetDesktopWindow(), pt,
                                  CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);
  // find child window
  if (hwnd != NULL) {
    htemp = hwnd;

    while (true) {
      ::GetCursorPos(&pt);
      ::ScreenToClient(htemp, &pt);
      htemp = ::ChildWindowFromPointEx(htemp, pt, CWP_SKIPINVISIBLE);
      if (htemp == NULL || htemp == hwnd)
        break;
      hwnd = htemp;
    }
    ::GetWindowRect(hwnd, &rc_window);
  }
  return hwnd;
}

HBITMAP recorder_util::CreateColorMaskBitmap(HWND hwnd,
                                             DWORD color,
                                             RECT rect) {
  HDC hdcmem = CreateCompatibleDC(NULL);

  rect.right -= rect.left;
  rect.bottom -= rect.top;
  rect.left = rect.top = 0;

  HBITMAP hbmp = NULL;

  HDC hdc = GetDC(hwnd);
  hbmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
  ReleaseDC(hwnd, hdc);

  HGDIOBJ hold = SelectObject(hdcmem, hbmp);
  DuiLib::CRenderEngine::DrawColor(hdcmem, rect, color);

  SelectObject(hdcmem, hold);

  DeleteObject(hold);
  DeleteDC(hdcmem);
  return hbmp;
}

void recorder_util::ZoomRatio(const POINT& A,
                              const POINT& B,
                              const POINT& C,
                              const double& l,
                              const double& h,
                              POINT& B1,
                              POINT& C1) {
  B1 = minusPOINT(B, A);
  B1.x *= l / h;
  B1.y *= l / h;
  B1 = plusPOINT(B1, A);

  C1 = minusPOINT(C, A);
  C1.x *= l / h;
  C1.y *= l / h;
  C1 = plusPOINT(C1, A);
}

double recorder_util::PointToLine(const POINT& P,
                                  const POINT& LineA,
                                  const POINT& LineB) {
  if (LineA.x == LineB.x)
    return fabs(P.x - LineA.x);
  double k = (double)(LineA.y - LineB.y) / (double)(LineA.x - LineB.x);
  return fabs((k * P.x - P.y + (LineA.y - k * LineA.x)) / sqrt(k * k + 1));
}

double recorder_util::PointToPoint(const POINT& A, const POINT& B) {
  return sqrt((double)((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y)));
}

bool recorder_util::IsInRectAngle(const POINT& pt,
                                  const POINT& a,
                                  const POINT& b,
                                  const POINT& c,
                                  const POINT& d) {
  if (a.x <= pt.x && pt.x <= b.x && a.y <= pt.y && pt.y <= b.y) {
    if (!(c.x <= pt.x && pt.x <= d.x && c.y <= pt.y && pt.y <= d.y)) {
      return true;
    }
  }
  return false;
}

recorder_util::point_float recorder_util::transfrom_pf(const POINT& pt) {
  return recorder_util::point_float{(double)pt.x, (double)pt.y};
}

bool PtInTwoCircle_(POINT pt, int a1, int b1, int a2, int b2) {
  recorder_util::point_float ptt = recorder_util::transfrom_pf(pt);
  double aa1 = a1;
  double bb1 = b1;
  double aa2 = a2;
  double bb2 = b2;
  if ((ptt.x * ptt.x) / (aa1 * aa1) + (ptt.y * ptt.y) / (bb1 * bb1) >= 1 &&
      (ptt.x * ptt.x) / (aa2 * aa2) + (ptt.y * ptt.y) / (bb2 * bb2) <= 1) {
    return true;
  }
  return false;
}

bool recorder_util::PtInTwoCircle(const POINT& pt,
                                  int a1,
                                  int b1,
                                  int a2,
                                  int b2) {
  if (a1 <= a2) {
    return PtInTwoCircle_(pt, a1, b1, a2, b2);
  } else {
    return PtInTwoCircle_(pt, a2, b2, a1, b1);
  }
}
