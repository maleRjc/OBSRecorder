#include "controller.h"
#include "helper/process_mutex.h"

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
  ProcessMutex process_mutex;
  if (process_mutex.IsSuccess()) {
    if (Controller::InitRunEnvironment(hInstance) &&
        SUCCEEDED(::CoInitialize(NULL))) {
#ifdef _DEBUG
      bool continue_run = true;
#else
      bool continue_run = Controller::GetInstance()->IsInit();
#endif  // DEBUG

      if (continue_run) {
        Controller::GetInstance()->ShowCaptureWindow();
        Controller::MessageLoop();
      }
      ::CoUninitialize();
    }
  }
  return 0;
}