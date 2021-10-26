#pragma once
#ifndef RECORDER_SCR_UI_CANVAS_MASK_TYPE_H_
#define RECORDER_SCR_UI_CANVAS_MASK_TYPE_H_

#include "UIlib.h"

namespace ui_canvas {

enum class EditType {
  kNone,
  kRectAngle = 1,
  kCircle,
  kLine,
  kArrow,
  kBrush,
  kText,
  kHighlight,

  kTypeCount,
};

enum class EditAction {
  kActionNone,
  kActionCreate = 1,
  kActionDelete,
  kActionMove,
  kActionEdit,
  kActionTabFontcolor,
  kActionTabFontsize,

  kActionCount,
};

class EditMask {
 public:
  EditMask();
  EditMask(EditType etype, DuiLib::CControlUI* cptr);
  void clear();

  EditType edit_type_ = EditType::kNone;
  int mask_size_ = 2;
  int font_size_id_ = 4;
  DWORD color_ = 0xFFED1C24;
  DuiLib::CControlUI* control_ = nullptr;
};

class EditMaskAction : public EditMask {
 public:
  EditMaskAction();
  EditMaskAction(EditType etype,
                 DuiLib::CControlUI* ctrl,
                 RECT pos,
                 EditAction act,
                 DWORD col = 0xFFED1C24,
                 int fsize = 0);
  EditMaskAction(EditMask EM, RECT pos, EditAction act);

  bool operator!=(const EditMaskAction& a);

  EditAction action_ = EditAction::kActionNone;
  RECT position_ = {0, 0, 0, 0};
};

}  // namespace ui_canvas

#endif  // RECORDER_SCR_UI_CANVAS_MASK_TYPE_H_