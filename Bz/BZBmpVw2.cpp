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
//#include "BZ.h"
//#include "BZView.h"
//#include "BZBmpVw2.h"
//#include "BZDoc2.h"
//#include "Splitter.h"
//#include "MainFrm.h"


void MakeBzPallet256(DWORD *pRGB)
{
  *pRGB++ = 0xFFFFFF;
  for_to(i, 31) *pRGB++ = 0x00FFFF;
  for_to_(i, 128-32) *pRGB++ = 0xFF0000;
  for_to_(i, 128) *pRGB++ = 0x000000;
}

void MakeRedPallet256(DWORD *pRGB)
{
  for(unsigned int i=0; i<=0xff; i++)
  {
    *pRGB++ = 0 | (i&0xff)<<16;
  }
}


void MakeSafetyPallet256(DWORD *pRGB)
{
  // safety pallet http://msdn.microsoft.com/en-us/library/bb250466%28VS.85%29.aspx
  DWORD* pRGBorig = pRGB;

  //	*pRGB++ = 0xFFFFFF;
  for(unsigned int r=0; r<=0xff; r+=0x33)
    for(unsigned int g=0; g<=0xff; g+=0x33)
      for(unsigned int b=0; b<=0xff; b+=0x33)
        *pRGB++ = b|(g<<8)|(r<<16);
  for(unsigned int gr=0; gr<=0xffffff; gr+=0x111111)
    *pRGB++ = gr;
  *pRGB++ = 0xC0C0C0;
  *pRGB++ = 0x808080;
  *pRGB++ = 0x800000;
  *pRGB++ = 0x800080;
  *pRGB++ = 0x008000;
  *pRGB++ = 0x008080;
  pRGBorig[255] = 0xffffff;
  //	TRACE("pallet[0]=0x%x, [255]=0x%x\n", ((DWORD*)(m_lpbi+1))[0], ((DWORD*)(m_lpbi+1))[255]);
}

void Make8bitBITMAPINFOHEADER(LPBITMAPINFOHEADER lpbi, LONG w, LONG h)
{
  //	lpbi->biSize = sizeof(BITMAPINFOHEADER);
  lpbi->biWidth = w;
  lpbi->biHeight = h;
  lpbi->biPlanes = 1;
  lpbi->biBitCount = options.nBmpColorWidth;//8;
  lpbi->biCompression = BI_RGB;
  lpbi->biSizeImage = 0;
  lpbi->biClrUsed = 0;
  lpbi->biClrImportant = 0;
}