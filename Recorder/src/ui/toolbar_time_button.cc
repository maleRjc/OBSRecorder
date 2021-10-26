#include "toolbar_time_button.h"

#include "def.h"
#include "message.h"

namespace {
constexpr int kTimerId = 1;
constexpr int kTimerInterval = 100;
constexpr int kOneHour = RECORD_LIMIT_TIME;
}

ToolbarTimeButton::ToolbarTimeButton() {}

LPCTSTR ToolbarTimeButton::GetClass() const {
  return DUI_CTR_TOOLBAR_TIME_BUTTON;
}

LPVOID ToolbarTimeButton::GetInterface(LPCTSTR name) {
  if (_tcscmp(name, DUI_CTR_TOOLBAR_TIME_BUTTON) == 0)
    return static_cast<ToolbarTimeButton*>(this);
  return __super::GetInterface(name);
}

void ToolbarTimeButton::PaintText(HDC hdc) {
  DuiLib::CDuiString text = TimesFormat(time_);
  text_ = m_sText;
  m_sText = text + _T(" ") + text_;
  __super::PaintText(hdc);
  m_sText = text_;
}

void ToolbarTimeButton::DoEvent(DuiLib::TEventUI& event) {
  if (event.Type == DuiLib::UIEVENT_TIMER && start_ && !pause_)
    OnTimer();
  __super::DoEvent(event);
}

void ToolbarTimeButton::Start() {
  restart_time_ = clock();

  time_ = 0;
  delta_time_ = 0;
  start_ = true;
  m_pManager->SetTimer(this, kTimerId, kTimerInterval);
}

void ToolbarTimeButton::Pause() {
  clock_t cur_time = clock();
  delta_time_ += (cur_time - restart_time_);

  pause_ = true;
}

void ToolbarTimeButton::Continue() {
  restart_time_ = clock();

  pause_ = false;
}

void ToolbarTimeButton::Stop() {
  time_ = 0;
  delta_time_ = 0;
  start_ = false;
  m_pManager->KillTimer(this, kTimerId);
}

ToolbarTimeButton::~ToolbarTimeButton() {
  m_pManager->KillTimer(this, kTimerId);
}

void ToolbarTimeButton::OnTimer() {
  time_ = ((delta_time_ + clock() - restart_time_) / CLOCKS_PER_SEC);
  Invalidate();
  if (time_ > kOneHour)
    ::PostMessage(m_pManager->GetPaintWindow(), MSG_EXCEED_RECORD_LIMIT, 0, 0);
  else
    ::PostMessage(m_pManager->GetPaintWindow(), MSG_CHECK_LOW_DISK, 0, 0);
}

DuiLib::CDuiString ToolbarTimeButton::TimesFormat(uint64_t time) {
  int second = time % 60;
  int minute = time / 60 % 60;
  int hour = time / 60 / 60;
  DuiLib::CDuiString strtime = _T("00:00:00");
  strtime.Format(_T("%02d:%02d:%02d"), hour, minute, second);
  return strtime;
}
