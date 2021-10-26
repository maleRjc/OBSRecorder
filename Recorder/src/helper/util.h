#pragma once
#ifndef RECORDER_SRC_HELP_UTIL_H_
#define RECORDER_SRC_HELP_UTIL_H_

#include <Windows.h>

namespace recorder_util {

struct point_float {
  double x;
  double y;
};

RECT GetDesktopWindow();

RECT MoveRect(RECT rect, int x, int y);

bool IsEqualRect(const RECT& rect1, const RECT& rect2);

POINT GetPointFromLimitRect(const POINT& pt, const RECT& rect);

RECT ClipRectByPadding(RECT rect, int padding);

bool IsRectInRect(const RECT& internal_rect, const RECT& external_rect);

bool IsInCircle(const POINT& pt, const POINT& circle_center, int r);

HBITMAP CreateDesktopBitmap();

HBITMAP CreateDesktopMaskBitmap(HWND hwnd);

HWND SmallestWindowFromCursor(RECT& rc_window);

HBITMAP CreateColorMaskBitmap(HWND hwnd, DWORD color, RECT rect);

// Triangle-ABC: Select and Uniform Scale.
// The l,h is the base and the height of a triangle.
// B1,C1 is changed after finishing handle.
void ZoomRatio(const POINT& A,
               const POINT& B,
               const POINT& C,
               const double& l,
               const double& h,
               POINT& B1,
               POINT& C1);

// Distance between P and AB.
double PointToLine(const POINT& P, const POINT& LineA, const POINT& LineB);

// Distance between A and B.
double PointToPoint(const POINT& A, const POINT& B);

// Is pt in [S(a,b) - S(c,d)].
bool IsInRectAngle(const POINT& pt,
                   const POINT& a,
                   const POINT& b,
                   const POINT& c,
                   const POINT& d);

point_float transfrom_pf(const POINT& pt);

// Is pt in [Circle(a2, b2) - Circle(a1, b1)].
bool PtInTwoCircle(const POINT& pt, int a1, int b1, int a2, int b2);

bool IsInCircle(const POINT& pt, const POINT& circle_center, int r);

// Is point p in S(A,B,C,D) (ABCD is a anticlockwise rectangle.)
bool IsPointInRect(const point_float& P,
                   const point_float& A,
                   const point_float& B,
                   const point_float& C,
                   const point_float& D);

float GetTriangleSquar(const point_float& pf0,
                       const point_float& pf1,
                       const point_float& pf2);

// Is point-D in triangle-ABC.
bool IsInTriangle(const point_float& A,
                  const point_float& B,
                  const point_float& C,
                  const point_float& D);
}

#endif  // RECORDER_SRC_HELP_UTIL_H_