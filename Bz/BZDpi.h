#pragma once
#include <windows.h>

class CBZDpi
{
public:

  static float m_x;
  static float m_y;
  static BOOL  m_init;

  static void Init(HWND hWnd)
  {
    if(m_init)return;
    HDC hdc = GetDC(hWnd);
    m_x = ((float)GetDeviceCaps(hdc, LOGPIXELSX))/96.0f;
    m_y = ((float)GetDeviceCaps(hdc, LOGPIXELSY))/96.0f;
    m_init = TRUE;
    ReleaseDC(hWnd, hdc);
  }

  static float ScaleX()
  {
    return m_x;
  }

  static float ScaleY()
  {
    return m_y;
  }
};
