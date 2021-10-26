#pragma once
#ifndef RECORDER_SRC_UI_RECORDER_CANVAS_H_
#define RECORDER_SRC_UI_RECORDER_CANVAS_H_

#include <deque>
#include "canvas_mask_type.h"

class RecorderCanvas : public DuiLib::CControlUI {
 public:
  static int GetHighlightPenSize(int n_style);

  RecorderCanvas();
  RecorderCanvas(ui_canvas::EditType type);

  // Override DuiLib::CControlUI
  LPCTSTR GetClass() const override;
  LPVOID GetInterface(LPCTSTR name) override;
  void PaintBorder(HDC hdc) override;

  ui_canvas::EditType GetEditType() const { return edit_type_; }
  void SetBasePointAndMousePoint(const POINT& base_pt, const POINT& mouse_pt) {
    base_point_and_mouse_point_ = {base_pt, mouse_pt};
  }

  // Used to drawing brush canvas.
  // Change brush path.
  void AddFreePathPoint(const POINT& pt);

  // Used to drawing hightlight canvas.
  void AddChangePointSet(const POINT& pt, int radius, const POINT& diagonal_pt);

  bool IsInMaskRect(POINT pt);

 protected:
  ~RecorderCanvas() override;

  void PaintRectAngle(HDC hdc);
  void PaintCircle(HDC hdc);
  void PaintLine(HDC hdc);
  void PaintArrow(HDC hdc);
  void PaintHighlight(HDC hdc);
  void PaintFreePath(HDC hdc);

  void SetLineRectAngle(std::vector<POINT>& pt,
                        POINT pt1,
                        POINT pt2,
                        int border_width);
 private:
  ui_canvas::EditType edit_type_ = ui_canvas::EditType::kNone;
  std::pair<POINT, POINT> base_point_and_mouse_point_ = {{0, 0}, {0, 0}};

  // Brush
  std::deque<POINT> free_path_;    // Brush or mosaic path, highlight too.

  // Highlight
  std::deque<POINT> m_highlight_point_;
  // Change points must be a pair of points.
  // Another point is a diagonal point used to resize.
  typedef std::pair<POINT, int> ChangePoint;
  typedef std::pair<ChangePoint, POINT> ChangePointPair;
  std::deque<ChangePointPair> m_change_point_pair_set_;

  // Arrow
  std::vector<POINT> m_arrow_polygon_point_;
  std::vector<POINT> m_arrow_line_point_;

  // Line
  std::vector<POINT> m_line_point_;

  // Circle
  POINT circle_centerpoint_;
  typedef std::pair<int, int> circle_a_b;
  std::vector<circle_a_b> m_circle_ab_;

  // Rectangle
  std::vector<POINT> m_rectangle_point_;
};

#endif  // RECORDER_SRC_UI_RECORDER_CANVAS_H_