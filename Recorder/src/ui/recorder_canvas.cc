#include "recorder_canvas.h"

#include <cmath>

#include "../helper/util.h"
#include "def.h"
#include "recorder_wnd.h"


namespace {

constexpr int kSignRadius = 10;
const int kHighlightPensize_min = 18;
const int kHighlightPensize_mid = 24;
const int kHighlightPensize_max = 36;
const int kARGB_A = COLOR_HIGHLIGHT_ARGB_A;

void PrintRect(LPCTSTR str, RECT rc) {
#ifdef _DEBUG
  static TCHAR dstr[MAX_PATH] = {0};
  _stprintf_s(dstr, MAX_PATH - 1, _T("%s RECT:[%d,%d,%d,%d] w:%d, h:%d\n"), str,
              rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left,
              rc.bottom - rc.top);
  OutputDebugString(dstr);
#endif
}
}

int RecorderCanvas::GetHighlightPenSize(int n_style) {
  int n_pensize = kHighlightPensize_min;
  switch (n_style) {
    case 2:
      n_pensize = kHighlightPensize_min;
      break;
    case 4:
      n_pensize = kHighlightPensize_mid;
      break;
    case 6:
      n_pensize = kHighlightPensize_max;
      break;
  }
  return n_pensize;
}

RecorderCanvas::RecorderCanvas() {}

RecorderCanvas::RecorderCanvas(ui_canvas::EditType type) : edit_type_(type) {}

LPCTSTR RecorderCanvas::GetClass() const {
  return DUI_CTR_RECORDER_CANVAS;
}

LPVOID RecorderCanvas::GetInterface(LPCTSTR name) {
  if (_tcscmp(name, DUI_CTR_RECORDER_CANVAS) == 0)
    return static_cast<RecorderCanvas*>(this);
  return __super::GetInterface(name);
}

void RecorderCanvas::PaintBorder(HDC hdc) {
  switch (edit_type_) {
    case ui_canvas::EditType::kRectAngle:
      PaintRectAngle(hdc);
      break;
    case ui_canvas::EditType::kCircle:
      PaintCircle(hdc);
      break;
    case ui_canvas::EditType::kLine:
      PaintLine(hdc);
      break;
    case ui_canvas::EditType::kArrow:
      PaintArrow(hdc);
      break;
    case ui_canvas::EditType::kBrush:
      PaintFreePath(hdc);
      break;
    case ui_canvas::EditType::kHighlight:
      PaintHighlight(hdc);
      break;
    case ui_canvas::EditType::kText:
      break;
    default:
      CControlUI::PaintBorder(hdc);
      break;
  }
}

void RecorderCanvas::AddFreePathPoint(const POINT& pt) {
  free_path_.push_back(POINT{pt.x, pt.y});
}

void RecorderCanvas::AddChangePointSet(const POINT& pt,
                                       int radius,
                                       const POINT& diagonal_pt) {
  ChangePoint circle(pt, radius);
  ChangePointPair circle_pair(circle, diagonal_pt);
  m_change_point_pair_set_.push_back(circle_pair);
}

bool RecorderCanvas::IsInMaskRect(POINT pt) {
  bool flag = false;
  switch (edit_type_) {
    case ui_canvas::EditType::kRectAngle:
      if (!m_rectangle_point_.empty()) {
        if (recorder_util::IsInRectAngle(
                pt, m_rectangle_point_[0], m_rectangle_point_[1],
                m_rectangle_point_[2], m_rectangle_point_[3])) {
          flag = true;
        }
      }
      break;
    case ui_canvas::EditType::kCircle:
      if (!m_circle_ab_.empty()) {
        pt.x -= circle_centerpoint_.x;
        pt.y -= circle_centerpoint_.y;
        if (recorder_util::PtInTwoCircle(
                pt, m_circle_ab_[0].first, m_circle_ab_[0].second,
                m_circle_ab_[1].first, m_circle_ab_[1].second)) {
          flag = true;
        }
      }
      break;
    case ui_canvas::EditType::kHighlight: {
      int border_width = GetHighlightPenSize(
          m_rcBorderSize.left > 0 ? m_rcBorderSize.left : 2);
      border_width /= 2;
      for (auto i = m_highlight_point_.begin(); i != m_highlight_point_.end();
           i++) {
        if (recorder_util::IsInCircle(pt, *i, border_width)) {
          flag = true;
          break;
        }
      }
      // dont't "break;" because Highlight is related with Line on some
      // judgment.
    }
    case ui_canvas::EditType::kLine:
      if (!m_line_point_.empty()) {
        if (recorder_util::IsPointInRect(
                recorder_util::transfrom_pf(pt),
                recorder_util::transfrom_pf(m_line_point_[0]),
                recorder_util::transfrom_pf(m_line_point_[1]),
                recorder_util::transfrom_pf(m_line_point_[2]),
                recorder_util::transfrom_pf(m_line_point_[3]))) {
          flag = true;
        }
      }
      break;
    case ui_canvas::EditType::kArrow:
      if (!m_arrow_polygon_point_.empty()) {
        if (recorder_util::IsInTriangle(
                recorder_util::transfrom_pf(m_arrow_polygon_point_[0]),
                recorder_util::transfrom_pf(m_arrow_polygon_point_[1]),
                recorder_util::transfrom_pf(m_arrow_polygon_point_[2]),
                recorder_util::transfrom_pf(pt))) {
          flag = true;
        }
      }
      if (!m_arrow_line_point_.empty()) {
        if (recorder_util::IsPointInRect(
                recorder_util::transfrom_pf(pt),
                recorder_util::transfrom_pf(m_arrow_line_point_[0]),
                recorder_util::transfrom_pf(m_arrow_line_point_[1]),
                recorder_util::transfrom_pf(m_arrow_line_point_[2]),
                recorder_util::transfrom_pf(m_arrow_line_point_[3]))) {
          flag = true;
        }
      }
      break;
    case ui_canvas::EditType::kBrush:
      if (free_path_.size() > 1) {
        std::vector<POINT> op;
        op.resize(4);
        int len = free_path_.size();
        for (int i = 0; i < len - 1; i++) {
          POINT cur_point = {free_path_[i].x + m_rcItem.left,
                             free_path_[i].y + m_rcItem.top};
          POINT next_point = {free_path_[i + 1].x + m_rcItem.left,
                              free_path_[i + 1].y + m_rcItem.top};
          if (cur_point.x == pt.x && cur_point.y == pt.y) {
            flag = true;
            break;
          }
          int border_width = m_rcBorderSize.left > 0 ? m_rcBorderSize.left : 2;
          SetLineRectAngle(op, cur_point, next_point, border_width + 9);
          if (recorder_util::IsPointInRect(
                  recorder_util::transfrom_pf(pt),
                  recorder_util::transfrom_pf(op[0]),
                  recorder_util::transfrom_pf(op[1]),
                  recorder_util::transfrom_pf(op[2]),
                  recorder_util::transfrom_pf(op[3]))) {
            flag = true;
            break;
          }
        }
      }
      break;
    case ui_canvas::EditType::kText:
      flag = ::PtInRect(&GetPos(), pt);
      break;
    default:
      break;
  }
  return flag;
}

RecorderCanvas::~RecorderCanvas() {}

void RecorderCanvas::PaintRectAngle(HDC hdc) {
  int border_width = m_nBorderSize > 0 ? m_nBorderSize : 2;

  RECT rect = {min(base_point_and_mouse_point_.first.x,
                   base_point_and_mouse_point_.second.x) +
                   m_rcItem.left,
               min(base_point_and_mouse_point_.first.y,
                   base_point_and_mouse_point_.second.y) +
                   m_rcItem.top,
               max(base_point_and_mouse_point_.first.x,
                   base_point_and_mouse_point_.second.x) +
                   m_rcItem.left,
               max(base_point_and_mouse_point_.first.y,
                   base_point_and_mouse_point_.second.y) +
                   m_rcItem.top};

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
  Gdiplus::Pen color_pen(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
      border_width);
  graphics.DrawRectangle(&color_pen, (INT)rect.left, (INT)rect.top,
                         (INT)rect.right - rect.left,
                         (INT)rect.bottom - rect.top);

  /*----------------------------------------------*/
  // Judge if canvas is selected accrossing to those points.
  m_rectangle_point_.clear();
  m_rectangle_point_.push_back(POINT{rect.left, rect.top});
  m_rectangle_point_.push_back(POINT{rect.right, rect.bottom});
  // Increase the range of decision
  m_rectangle_point_.push_back(
      POINT{rect.left + border_width + 1, rect.top + border_width + 1});
  m_rectangle_point_.push_back(
      POINT{rect.right - border_width - 1, rect.bottom - border_width - 1});
  /*----------------------------------------------*/

  // Draw 4 ellipses as selected signs of canvas.
  if (RecorderWnd::GetInstance()->GetSelectedEditMask().control_ == this) {
    Gdiplus::SolidBrush color_brush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::Pen color_pen(Gdiplus::Color(255, 115, 115, 115), 1);
    graphics.FillEllipse(&color_brush, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.FillEllipse(&color_brush, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.FillEllipse(&color_brush, rect.left - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.FillEllipse(&color_brush, rect.right - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&color_pen, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&color_pen, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.DrawEllipse(&color_pen, rect.left - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.DrawEllipse(&color_pen, rect.right - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
  }
}

void RecorderCanvas::PaintCircle(HDC hdc) {
  int border_width = m_nBorderSize > 0 ? m_nBorderSize : 2;

  RECT rect = {min(base_point_and_mouse_point_.first.x,
                   base_point_and_mouse_point_.second.x) +
                   m_rcItem.left,
               min(base_point_and_mouse_point_.first.y,
                   base_point_and_mouse_point_.second.y) +
                   m_rcItem.top,
               max(base_point_and_mouse_point_.first.x,
                   base_point_and_mouse_point_.second.x) +
                   m_rcItem.left,
               max(base_point_and_mouse_point_.first.y,
                   base_point_and_mouse_point_.second.y) +
                   m_rcItem.top};

  Gdiplus::Graphics graphics(hdc);
  Gdiplus::Pen color_pen(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
      border_width);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
  int ellipse_x = rect.left /*+ (border_width / 2 + 1)*/;
  int ellipse_y = rect.top /*+ (border_width / 2 + 1)*/;
  int ellipse_width = rect.right - rect.left /*- (border_width * 2 + 1)*/;
  int ellipse_height = rect.bottom - rect.top /*- (border_width * 2 + 1)*/;
  graphics.DrawEllipse(&color_pen, ellipse_x, ellipse_y, ellipse_width,
                       ellipse_height);

  /*--------------------------------------------*/
  // Draw 4 ellipses as selected signs of canvas.
  if (RecorderWnd::GetInstance()->GetSelectedEditMask().control_ == this) {
    Gdiplus::SolidBrush colorBrush2(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::Pen colorPen2(Gdiplus::Color(255, 75, 75, 75), 1);
    graphics.DrawRectangle(&colorPen2, (INT)rect.left - 2, (INT)rect.top - 2,
                           (INT)rect.right - (INT)rect.left + 4,
                           (INT)rect.bottom - (INT)rect.top + 4);
    graphics.FillEllipse(&colorBrush2, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.FillEllipse(&colorBrush2, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.FillEllipse(&colorBrush2, rect.left - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.FillEllipse(&colorBrush2, rect.right - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.left - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius,
                         kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.right - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
  }
  /*--------------------------------------------*/
  // The major and minor axes of the ellipse and the center of the circle.
  int circle_a = ellipse_width;
  circle_a /= 2;
  int circle_b = ellipse_height;
  circle_b /= 2;
  circle_centerpoint_ = {ellipse_x + circle_a, ellipse_y + circle_b};
  m_circle_ab_.clear();
  m_circle_ab_.emplace_back(circle_a - border_width / 2,
                            circle_b - border_width / 2);
  // Increase the range of decision
  m_circle_ab_.emplace_back(circle_a + border_width / 2 + 2,
                            circle_b + border_width / 2 + 2);
}

void RecorderCanvas::PaintLine(HDC hdc) {
  int border_width = m_nBorderSize > 0 ? m_nBorderSize : 2;

  RECT rect = {base_point_and_mouse_point_.first.x + m_rcItem.left,
               base_point_and_mouse_point_.first.y + m_rcItem.top,
               base_point_and_mouse_point_.second.x + m_rcItem.left,
               base_point_and_mouse_point_.second.y + m_rcItem.top};

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  Gdiplus::Pen color_pen(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
      border_width);
  graphics.DrawLine(&color_pen, (INT)rect.left, (INT)rect.top, (INT)rect.right,
                    (INT)rect.bottom);
  /*--------------------------------------------*/
  // Draw 2 ellipses as selected signs of canvas.
  if (RecorderWnd::GetInstance()->GetSelectedEditMask().control_ == this) {
    Gdiplus::SolidBrush color_brush(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::Pen color_pen2(Gdiplus::Color(255, 75, 75, 75), 1);
    graphics.FillEllipse(&color_brush, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2,
                         kSignRadius, kSignRadius);
    graphics.DrawEllipse(&color_pen2, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2,
                         kSignRadius, kSignRadius);
    graphics.FillEllipse(&color_brush, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2,
                         kSignRadius, kSignRadius);
    graphics.DrawEllipse(&color_pen2, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2,
                         kSignRadius, kSignRadius);
  }
  /*--------------------------------------------*/

  /* Generate the corresponding rectangular coordinates
  according to the line coordinates. */
  m_line_point_.clear();
  m_line_point_.push_back(POINT{rect.left, rect.top});
  m_line_point_.push_back(POINT{rect.right, rect.bottom});
  m_line_point_.push_back(POINT{0, 0});
  m_line_point_.push_back(POINT{0, 0});
  SetLineRectAngle(m_line_point_, m_line_point_[0], m_line_point_[1],
                   border_width);
}

void RecorderCanvas::PaintArrow(HDC hdc) {
  int border_width = m_nBorderSize > 0 ? m_nBorderSize : 2;

  RECT rect = {
      base_point_and_mouse_point_.second.x + m_rcItem.left,
      base_point_and_mouse_point_.second.y + m_rcItem.top,
      base_point_and_mouse_point_.first.x + m_rcItem.left,
      base_point_and_mouse_point_.first.y + m_rcItem.top,
  };

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  int rect_w = rect.right - rect.left;
  int rect_h = rect.bottom - rect.top;
  double diagonal = std::sqrt(std::pow(rect_w, 2) + std::pow(rect_h, 2));

  int arrow_r = 1 + 2 * border_width;
  int arrow_l = 4 * arrow_r;
  int arrow_x = arrow_l * rect_w / diagonal;
  int arrow_y = arrow_l * rect_h / diagonal;
  int dlt_x = arrow_r * rect_h / diagonal;
  int dlt_y = arrow_r * rect_w / diagonal;

  POINT line_a{rect.left + arrow_x, rect.top + arrow_y};
  POINT line_b{rect.right, rect.bottom};
  if (std::abs(arrow_x) <= std::abs(rect_w) &&
      std::abs(arrow_y) <= std::abs(rect_h)) {
    Gdiplus::Pen color_pen(
        Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                       GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
        border_width);
    graphics.DrawLine(&color_pen, (INT)rect.left + arrow_x,
                      (INT)rect.top + arrow_y, (INT)rect.right,
                      (INT)rect.bottom);

    // Generate the corresponding rectangular coordinates
    // according to the line coordinates.
    m_arrow_line_point_.clear();
    m_arrow_line_point_.push_back(line_a);
    m_arrow_line_point_.push_back(line_b);
    m_arrow_line_point_.push_back(POINT{0, 0});
    m_arrow_line_point_.push_back(POINT{0, 0});
    SetLineRectAngle(m_arrow_line_point_, m_arrow_line_point_[0],
                     m_arrow_line_point_[1], border_width);
  }

  /*----------------------------------------------*/
  // Equal scaling.
  POINT polygon_a{rect.left, rect.top},
      polygon_b{rect.left + arrow_x - dlt_x, rect.top + arrow_y + dlt_y},
      polygon_c{rect.left + arrow_x + dlt_x, rect.top + arrow_y - dlt_y};
  double line_l = recorder_util::PointToPoint(polygon_a, line_b);
  double polygon_h =
      recorder_util::PointToLine(polygon_a, polygon_b, polygon_c);
  POINT polygon_b1 = polygon_b, polygon_c1 = polygon_c;
  if (line_l < polygon_h) {
    recorder_util::ZoomRatio(polygon_a, polygon_b, polygon_c, line_l, polygon_h,
                             polygon_b1, polygon_c1);
  }
  /*----------------------------------------------*/

  Gdiplus::Point arrow_point[3] = {{rect.left, rect.top},
                                   {polygon_b1.x, polygon_b1.y},
                                   {polygon_c1.x, polygon_c1.y}};
  Gdiplus::SolidBrush color_brush(Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                                                GetGValue(m_dwBorderColor),
                                                GetRValue(m_dwBorderColor)));
  graphics.FillPolygon(&color_brush, arrow_point, 3);

  /*----------------------------------------------*/
  // Record the triangle arrow coordinates.
  m_arrow_polygon_point_.clear();
  m_arrow_polygon_point_.push_back(polygon_a);
  m_arrow_polygon_point_.push_back(polygon_b1);
  m_arrow_polygon_point_.push_back(polygon_c1);
  /*----------------------------------------------*/

  // draw 2 ellipses as selected signs of canvas.
  if (RecorderWnd::GetInstance()->GetSelectedEditMask().control_ == this) {
    Gdiplus::SolidBrush colorBrush2(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::Pen colorPen2(Gdiplus::Color(255, 95, 95, 95), 1);
    graphics.FillEllipse(&colorBrush2, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.left - kSignRadius / 2,
                         rect.top - kSignRadius / 2, kSignRadius, kSignRadius);

    graphics.FillEllipse(&colorBrush2, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius, kSignRadius);
    graphics.DrawEllipse(&colorPen2, rect.right - kSignRadius / 2,
                         rect.bottom - kSignRadius / 2, kSignRadius, kSignRadius);
  }
}

void RecorderCanvas::PaintHighlight(HDC hdc) {
  int border_width = GetHighlightPenSize(m_nBorderSize > 0 ? m_nBorderSize : 2);

  RECT rect = {
      base_point_and_mouse_point_.first.x + m_rcItem.left,
      base_point_and_mouse_point_.first.y + m_rcItem.top,
      base_point_and_mouse_point_.second.x + m_rcItem.left,
      base_point_and_mouse_point_.second.y + m_rcItem.top,
  };
  // Flip.
  if (base_point_and_mouse_point_.first.x >
      base_point_and_mouse_point_.second.x)
    rect = {
        base_point_and_mouse_point_.second.x + m_rcItem.left,
        base_point_and_mouse_point_.second.y + m_rcItem.top,
        base_point_and_mouse_point_.first.x + m_rcItem.left,
        base_point_and_mouse_point_.first.y + m_rcItem.top,
    };

  // Limit rect for circle header.
  RECT draw_rect = recorder_util::ClipRectByPadding(m_rcItem, border_width / 2);
  if (rect.left < draw_rect.left)
    rect.left = draw_rect.left;
  else if (rect.left > draw_rect.right)
    rect.left = draw_rect.right;
  if (rect.top < draw_rect.top)
    rect.top = draw_rect.top;
  else if (rect.top > draw_rect.bottom)
    rect.top = draw_rect.bottom;
  if (rect.right < draw_rect.left)
    rect.right = draw_rect.left;
  else if (rect.right > draw_rect.right)
    rect.right = draw_rect.right;
  if (rect.bottom < draw_rect.top)
    rect.bottom = draw_rect.top;
  else if (rect.bottom > draw_rect.bottom)
    rect.bottom = draw_rect.bottom;

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  Gdiplus::Color color_highlight =
      Gdiplus::Color(kARGB_A, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor));
  Gdiplus::Pen pen_highlight(color_highlight, border_width);

  auto lambda_angle_calculate_for_halfof_circle = [&rect]() -> double {
    double db_angle = 0;
    if (rect.right - rect.left) {
      // x turn to y and y turn to x, so y/x turn to x/y.
      double db_tan_angle =
          double(rect.right - rect.left) / double(rect.bottom - rect.top);
      db_angle = -atan(db_tan_angle) / (acos(-1) / 180);
      // db_angle belong to (-90, 0] and [0, 90). we need [0, 180].
      // (-90, 0] + 180 = (90, 180].
      if (db_angle < 0)
        db_angle += 180;
    } else {
      if (rect.bottom < rect.top)
        db_angle = 0;  // pie downward
      else
        db_angle = 180;  // pie upward
    }
    return db_angle;
  };
  double db_angle_endpoint = lambda_angle_calculate_for_halfof_circle();

  typedef Gdiplus::REAL Real;
  Gdiplus::SolidBrush brush_highlight(color_highlight);
  if (rect.left == rect.right && rect.bottom == rect.top) {
    graphics.FillEllipse(&brush_highlight, rect.right - border_width / 2,
                         rect.bottom - border_width / 2, border_width,
                         border_width);
  } else {
    graphics.FillPie(&brush_highlight, rect.left - border_width / 2,
                     rect.top - border_width / 2, border_width, border_width,
                     Real(db_angle_endpoint - 1), Real(182));
    graphics.FillPie(&brush_highlight, rect.right - border_width / 2,
                     rect.bottom - border_width / 2, border_width, border_width,
                     Real(180 + db_angle_endpoint - 1), Real(182));
    graphics.DrawLine(&pen_highlight, (INT)rect.left, (INT)rect.top,
                      (INT)rect.right, (INT)rect.bottom);
  }

  /*--------------------------------------------*/
  // Draw 2 ellipses as selected signs of canvas.
  m_change_point_pair_set_.clear();
  if (RecorderWnd::GetInstance()->GetSelectedEditMask().control_ == this) {
    POINT pt_lefttop{rect.left - kSignRadius / 2, rect.top - kSignRadius / 2};
    POINT pt_rightbottom{rect.right - kSignRadius / 2, rect.bottom - kSignRadius / 2};
    Gdiplus::SolidBrush color_brush2(Gdiplus::Color(200, 255, 255, 255));
    Gdiplus::Pen color_pen2(Gdiplus::Color(200, 75, 75, 75), 1);
    graphics.FillEllipse(&color_brush2, pt_lefttop.x, pt_lefttop.y, kSignRadius,
                         kSignRadius);
    graphics.DrawEllipse(&color_pen2, pt_lefttop.x, pt_lefttop.y, kSignRadius,
                         kSignRadius);
    graphics.FillEllipse(&color_brush2, pt_rightbottom.x, pt_rightbottom.y,
                         kSignRadius, kSignRadius);
    graphics.DrawEllipse(&color_pen2, pt_rightbottom.x, pt_rightbottom.y, kSignRadius,
                         kSignRadius);

    AddChangePointSet(pt_lefttop, kSignRadius, POINT{rect.right, rect.bottom});
    AddChangePointSet(pt_rightbottom, kSignRadius, POINT{rect.left, rect.top});
  }
  /*--------------------------------------------*/

  // Generate the corresponding rectangular coordinates 
  // according to the line coordinates.
  m_line_point_.clear();
  m_line_point_.push_back(POINT{rect.left, rect.top});
  m_line_point_.push_back(POINT{rect.right, rect.bottom});
  m_line_point_.push_back(POINT{0, 0});
  m_line_point_.push_back(POINT{0, 0});
  SetLineRectAngle(m_line_point_, m_line_point_[0], m_line_point_[1],
                   border_width);
  m_highlight_point_.clear();
  m_highlight_point_.push_back(POINT{rect.left, rect.top});
  m_highlight_point_.push_back(POINT{rect.right, rect.bottom});
}

void RecorderCanvas::PaintFreePath(HDC hdc) {
  int border_width = m_nBorderSize > 0 ? m_nBorderSize : 2;
  int point_count = free_path_.size();
  if (point_count < 1)
    return;

  Gdiplus::Graphics graphics(hdc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

  Gdiplus::GraphicsPath graphics_path;
  for (int i = 0; i < point_count - 1; i++)
    graphics_path.AddLine(Gdiplus::Point(free_path_[i].x + m_rcItem.left,
                                         free_path_[i].y + m_rcItem.top),
                          Gdiplus::Point(free_path_[i + 1].x + m_rcItem.left,
                                         free_path_[i + 1].y + m_rcItem.top));

  Gdiplus::Pen color_pen(
      Gdiplus::Color(255, GetBValue(m_dwBorderColor),
                     GetGValue(m_dwBorderColor), GetRValue(m_dwBorderColor)),
      border_width);
  graphics.DrawPath(&color_pen, &graphics_path);
}

void RecorderCanvas::SetLineRectAngle(std::vector<POINT>& pt,
                                      POINT pt1,
                                      POINT pt2,
                                      int border_width) {
  if (abs(pt1.x - pt2.x) >= abs(pt1.y - pt2.y)) {
    pt[0].y = pt1.y - border_width / 2;
    pt[0].x = pt1.x;

    pt[1].y = pt1.y + border_width / 2;
    pt[1].x = pt1.x;

    pt[3].y = pt2.y - border_width / 2;
    pt[3].x = pt2.x;

    pt[2].y = pt2.y + border_width / 2;
    pt[2].x = pt2.x;
  } else {
    pt[0].y = pt1.y;
    pt[0].x = pt1.x - border_width / 2;

    pt[1].y = pt1.y;
    pt[1].x = pt1.x + border_width / 2;

    pt[3].y = pt2.y;
    pt[3].x = pt2.x - border_width / 2;

    pt[2].y = pt2.y;
    pt[2].x = pt2.x + border_width / 2;
  }
}
