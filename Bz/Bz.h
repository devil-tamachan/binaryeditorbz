/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(original author) & devil.tamachan@gmail.com(Modder)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once


#include "resource.h"       // main symbols

#include <Shlwapi.h>

#define DEFINED_CRECTU64V
class CRectU64V
{
public:
  long left;
  UINT64 top;
  long right;
  UINT64 bottom;

public:
  CRectU64V() : left(0), top(0), right(0), bottom(0)
  {
  }
  
  void operator =(LPRECT src)
  {
    left = src->left;
    top = src->top;
    right = src->right;
    bottom = src->bottom;
  }
};

#define HIDWORD(ll) ((DWORD)((((UINT64)(ll)) >> 32) & 0xFFFFffff))
#define LODWORD(ll) ((DWORD)(((UINT64)(ll)) & 0xFFFFffff))

/////////////////////////////////////////////////////////////////////////////
// CBZOptions




CString SeparateByComma(int num, BOOL bSigned = FALSE);
CString SeparateByComma64(ULONGLONG num, BOOL bSigned = FALSE);
CString GetModulePath(LPCSTR pFileName);
CString GetStructFilePath(UINT uID);
LPVOID ReadFile(LPCTSTR pPath);
void ErrorMessageBox(HWND hWnd = NULL);	// ###1.61
void ErrorResMessageBox(UINT nID, HWND hWnd = NULL);
void InfoResMessageBox(UINT nID, HWND hWnd = NULL);

class CMainFrame;
CMainFrame* GetMainFrame();
BOOL PostMessage2MainFrame(UINT msg, WPARAM wp = 0, LPARAM lp = 0);

DWORD MemCompByte2(const BYTE *p1, const BYTE *p2, DWORD len);

/////////////////////////////////////////////////////////////////////////////
