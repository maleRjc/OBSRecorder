#pragma once
#ifndef SCRCAPTURE_SRC_UI_CURSOR_CUSTOM_H_
#define SCRCAPTURE_SRC_UI_CURSOR_CUSTOM_H_

#include <iostream>
#include <map>
#include <Windows.h>


namespace Gdiplus {
class Bitmap;
}

// Objects of CursorCustom don't generate HCURSOR-image until GetHCursor() was called.
class CursorCustom {
 public:
  CursorCustom(std::string name, int size, DWORD color);
  virtual ~CursorCustom();
  HCURSOR GetHCursor();
  std::string GetName() const { return str_name_; }

 protected:
  virtual void GenerateCustomImageForCursor();
  virtual HBITMAP GenerateImage();
  void DeleteHCursor();

 private:
  int n_size_ = 18;
  HCURSOR h_cursor_ = NULL;
  DWORD dw_color_ = 0;
  std::string str_name_;
};

class CursorManager {
 public:
  static std::string GenerateName(int size, DWORD color);
  static CursorManager* GetInstance();

  ~CursorManager();

  void AddCursor(std::string name, int size = 18, DWORD color = 0);
  void RemoveCursor(std::string name);
  HCURSOR GetCursor(std::string name);
  HCURSOR AddAndGetCursor(std::string name,
                          int size = 18,
                          DWORD color = 0);

 protected:
  CursorManager();

 private:
  std::map<std::string, std::unique_ptr<CursorCustom> > cursor_set_;
};

#endif  // !SCRCAPTURE_SRC_UI_CURSOR_CUSTOM_H_