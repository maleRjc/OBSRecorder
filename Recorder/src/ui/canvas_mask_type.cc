#include "canvas_mask_type.h"

ui_canvas::EditMask::EditMask() {}

ui_canvas::EditMask::EditMask(EditType etype, DuiLib::CControlUI* cptr)
    : edit_type_(etype), control_(cptr) {}

void ui_canvas::EditMask::clear() {
  control_ = nullptr;
  edit_type_ = EditType::kNone;
}

ui_canvas::EditMaskAction::EditMaskAction() {}

ui_canvas::EditMaskAction::EditMaskAction(EditType etype,
                                          DuiLib::CControlUI* ctrl,
                                          RECT pos,
                                          EditAction act,
                                          DWORD col,
                                          int fsize)
    : EditMask(etype, ctrl), position_(pos), action_(act) {
  font_size_id_ = fsize;
  color_ = col;
}

ui_canvas::EditMaskAction::EditMaskAction(EditMask EM,
                                          RECT pos, EditAction act)
    : EditMask(EM), position_(pos), action_(act) {}

bool ui_canvas::EditMaskAction::operator!=(const EditMaskAction& a) {
  if (a.control_ == control_ && a.edit_type_ == edit_type_ &&
      EqualRect(&a.position_, &position_) && a.color_ == color_ &&
      a.font_size_id_ == font_size_id_)
    return false;
  return true;
}
