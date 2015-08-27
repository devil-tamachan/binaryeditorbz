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

#include "stdafx.h"


COLORREF colorsDefault[TCOLOR_COUNT][2] = {
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_3DLIGHT | SYSCOLOR,
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_WINDOW | SYSCOLOR,
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_WINDOW | SYSCOLOR,
	COLOR_HIGHLIGHTTEXT | SYSCOLOR, COLOR_HIGHLIGHT | SYSCOLOR,
	RGB_RED,  COLOR_WINDOW | SYSCOLOR,
	RGB_BLUE, COLOR_WINDOW | SYSCOLOR,
	RGB_BLUE, COLOR_WINDOW | SYSCOLOR,
	RGB_RED, COLOR_WINDOW | SYSCOLOR,
	RGB_GREEN2, COLOR_WINDOW | SYSCOLOR,
	RGB(0xe2,0x04,0x1b), RGB(0x3e,0xb3,0x70),
	RGB(0xee,0xee,0xee)/*RGB(0xcc,0xcc,0xcc)*/, COLOR_3DLIGHT | SYSCOLOR,
};

//int colWidthDefault[MBRCOL_MAX] = { 26, 80, 60 };
int colWidth2Default[MBRCOL2_MAX] = { 180, 100 };

const TCHAR sRegDefault[] = _T("Settings");
const TCHAR sRegHistory[] = _T("History");


#include "PortableCfg.cpp"


HRESULT CBZOptions::LoadFromFile()
{
  DWORD len=0, lenPlusSpace=0;
  uchar *pFile = (uchar *)ReadFile(_T("EnablePortableMode.txt"), &len, &lenPlusSpace, 30, TRUE, FALSE);
  if(pFile==NULL)return E_FAIL;
  void *pParser = ParseAlloc(malloc);
  unsigned int type = 0;
  Scanner s = {0};
  s.cur = pFile;
  s.lim = pFile+lenPlusSpace;
  s.eof = pFile+len;

#ifdef _DEBUG
//  FILE *fpErr = fopen("err.txt", "w");
//  ParseTrace(fpErr, (char*)"LP: ");
#endif

  while(type = scan(&s))
  {
    ATLTRACE("type = %d\n", type);
    Parse(pParser, type, s.val);
  }
  ATLTRACE("ParseTerminate\n", type);
  Parse(pParser, 0, s.val);
  ParseFree(pParser, free);
  delete pFile;

#ifdef _DEBUG
//  if(fpErr)fclose(fpErr);
#endif

  CheckOptions();

  bPortableMode = TRUE;

  return S_OK;
}