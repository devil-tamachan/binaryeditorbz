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
#include "BZView.h"
#include "BZBmpVw2.h"
#include "BZDoc2.h"
//#include "Splitter.h"
//#include "MainFrm.h"
#include "CustomPallet.cpp"

class CBZView;

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


void CBZBmpView2::OnLButtonDown(UINT nFlags, WTL::CPoint point)
{
  if(!bEnable)return;

  m_isLButtonDown = true;

  UINT64 yVS = ConvYRealSpace2VirtualSpace(point.y);
  yVS += GetScrollPosU64V();
  yVS = CalcY(yVS);
  UINT64 currentAddress = CalcAddress(yVS);
  point.x += m_ptOffset.x;
  point.x -= BMPSPACE;
  if(point.x<=0)point.x = 0;
  else point.x /= options.nBmpZoom;
  if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= BMPSPACE) {
    currentAddress += (((UINT64)point.x) * (options.nBmpColorWidth/8));
    CBZView* pView = GetBZView();
    if(pView)pView->MoveCaretTo(currentAddress);
  }
}
void CBZBmpView2::OnMouseMove(UINT nFlags, WTL::CPoint point)
{
  if(!bEnable)return;

  UINT64 yVS = ConvYRealSpace2VirtualSpace(point.y);
  yVS += GetScrollPosU64V();
  yVS = CalcY(yVS);
  UINT64 currentAddress = CalcAddress(yVS);
  point.x += m_ptOffset.x;
  point.x -= BMPSPACE;
  if(point.x<=0)point.x = 0;
  else point.x /= options.nBmpZoom;
  if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= BMPSPACE) {
    currentAddress += (((UINT64)point.x) * (options.nBmpColorWidth/8));
    if(currentAddress != m_tooltipLastAddress)
    {
      CBZView* pView = GetBZView();
      if(currentAddress < pView->GetFileSize()) {
        if(m_isLButtonDown)
        {
          pView->MoveCaretTo(currentAddress);
        } else if(options.bAddressTooltip) {
          TCHAR tmp[22];
#ifdef _DEBUG
          wsprintf(tmp, _T("0x%016I64X, %d"), currentAddress, point.y);
#else
          wsprintf(tmp, _T("0x%016I64X"), currentAddress);
#endif
          WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
          m_tooltip.UpdateTipText(toolinfo);
          ATLTRACE(_T("UpdateTooltip: %016I64X, %016I64X\n"), currentAddress, m_tooltipLastAddress);
          m_tooltipLastAddress = currentAddress;
          m_tooltip.Activate(true);
          m_tooltip.Popup();
          return;
        }
      }
    } else {
      ATLTRACE(_T("!!!UpdateTooltip: %016I64X, %016I64X\n"), currentAddress, m_tooltipLastAddress);
      return;
    }
  }

  m_tooltipLastAddress = _UI64_MAX;
  m_tooltip.Activate(false);
  m_tooltip.Pop();
  //CScrollView::OnMouseMove(nFlags, point);
}


void CBZBmpView2::OnBmpViewMode(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  switch(nID)
  {
  case ID_BMPVIEW_WIDTH128:
    options.nBmpWidth = 128;
    break;
  case ID_BMPVIEW_WIDTH256:
    options.nBmpWidth = 256;
    break;
  case ID_BMPVIEW_ZOOM:
    options.nBmpZoom = (options.nBmpZoom == 1) ? 2 : 1;
    break;
  }
  //GetMainFrame()->CreateClient();
  OnInitialUpdate();
  Invalidate();
  GetMainFrame()->ResetWindowWidth();
}

HRESULT CBZBmpView2::LoadTxtPallet256(DWORD *pRGB, LPCTSTR path)
{
  DWORD len=0, lenPlusSpace=0;
  pPalletTxt256 = pRGB;
  uchar *pFile = (uchar *)ReadFile(path, &len, &lenPlusSpace, 30, TRUE, FALSE);
  if(pFile==NULL)return E_FAIL;
  void *pParser = ParsePalletTxtAlloc(malloc);
  unsigned int type = 0;
  Scanner s = {0};
  s.cur = pFile;
  s.lim = pFile+lenPlusSpace;
  s.eof = pFile+len;
  g_bgBmpView = 0xFFFFFFFF;

#ifdef _DEBUG
  //  FILE *fpErr = fopen("err.txt", "w");
  //  ParseTrace(fpErr, (char*)"LP: ");
#endif

  while(type = scanPalletTxt(&s))
  {
    ATLTRACE("type = %d\n", type);
    ParsePalletTxt(pParser, type, s.val);
  }
  ATLTRACE("ParseTerminate\n", type);
  ParsePalletTxt(pParser, 0, s.val);
  ParsePalletTxtFree(pParser, free);
  delete pFile;

  options.lastPalletName=PathFindFileName(path);

  return S_OK;
}


void CBZBmpView2::OnInitialUpdate()
{
  CBZDoc2* pDoc = GetBZDoc2();
  UINT64 dwTotal = pDoc->GetDocSize();
  if(dwTotal < (DWORD)options.nBmpWidth)
  {
    bEnable = FALSE;
    SetScrollSizeU64V(1, 1);
    SetScrollPage(100,150);
    SetScrollLine(10,20);
    ScrollTop();
    ScrollAllLeft();
    return;
  }
  bEnable = TRUE;

  if(!m_lpbi) 
    m_lpbi = (LPBITMAPINFOHEADER)MemAlloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256/*256pallet*/);

  m_lpbi->biSize = sizeof(BITMAPINFOHEADER);
  m_cBmpY64 = dwTotal / (UINT64)(options.nBmpWidth * (options.nBmpColorWidth/8));
  ATLTRACE("cBmpY64(%I64u) = dwTotal(%I64u) / (UINT64)(options.nBmpWidth(%d) * (options.nBmpColorWidth(%d)/8))\n", m_cBmpY64, dwTotal, options.nBmpWidth, options.nBmpColorWidth);
  Make8bitBITMAPINFOHEADER(m_lpbi, options.nBmpWidth, 1/*top-down DIB*/);

  if(options.nBmpPallet>=CUSTOMPALLET && pallets.GetCount()==0)
  {
    ReloadCustomPallets();
  }
  DWORD* pRGB = (DWORD*)(m_lpbi+1);
  switch(options.nBmpPallet)
  {
  case 0:
    MakeBzPallet256(pRGB);
    break;
  case 1:
    MakeSafetyPallet256(pRGB);
    break;
  default:
    if(options.nBmpPallet>=CUSTOMPALLET && pallets.GetCount()+CUSTOMPALLET > options.nBmpPallet)
    {
      LoadCustomPallet256(pRGB, pallets[options.nBmpPallet-CUSTOMPALLET]);
    } else {
      MakeBzPallet256(pRGB);
      options.nBmpPallet=0;
      options.nBmpColorWidth = 8;
    }
    break;
  }

  // TODO: calculate the total size of this view
  int cViewX = options.nBmpWidth * options.nBmpZoom + BMPSPACE*2;
  UINT64 cViewY = m_cBmpY64 * options.nBmpZoom + BMPSPACE*2;
  SetScrollSizeU64V(cViewX, cViewY);
  SetScrollPage(100,150);
  SetScrollLine(10,20);
  ScrollTop();
  ScrollAllLeft();

  //TRACE("cView.cy=%X\n", GetTotalSize().cy);

  //CTamaSplitterWindow* pSplit = GetSplitter();
  //int cSplitViewX = options.nBmpWidth * options.nBmpZoom + BMPSPACE*2 + GetSystemMetrics(SM_CXVSCROLL)+1;
  //pSplit->SetColumnInfo(0, cSplitViewX, 0);
  // MemFree(lpbi);

  if(m_tooltip.m_hWnd!=NULL)m_tooltip.DestroyWindow();
  m_tooltip.Create(m_hWnd, NULL, NULL, TTS_BALLOON|TTS_NOFADE|TTS_NOANIMATE|TTS_ALWAYSTIP);
  m_tooltip.SetDelayTime(TTDT_RESHOW, 0);
  m_tooltip.SetDelayTime(TTDT_AUTOPOP, 0xffff);
  m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
  m_tooltip.Activate(TRUE);
  WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, _T(""));
  m_tooltip.AddTool(toolinfo);
}

void CBZBmpView2::DoPaint(WTL::CDCHandle dc)
{
  WTL::CRect rClipOrig;
  dc.GetClipBox(rClipOrig);
  dc.FillSolidRect(rClipOrig, g_bgBmpView & 0xFFFFFF);

  if(!bEnable)return;

  UINT64 topVS = ConvYRealSpace2VirtualSpace(rClipOrig.top);
  topVS = CalcY(topVS);

  UINT64 bottomVS = ConvYRealSpace2VirtualSpace(rClipOrig.bottom);
  bottomVS = CalcY(bottomVS);
  if(m_cBmpY64==0)return;
  if(bottomVS >= m_cBmpY64)bottomVS = m_cBmpY64-1;

  if(topVS > bottomVS)return;

  CBZDoc2* pDoc = GetBZDoc2();
  ATLASSERT(pDoc);
  if(!pDoc->IsOpen())return;

  int nBmpHeight;
  {
    UINT64 qwBmpHeight = bottomVS - topVS + 1;
    nBmpHeight = qwBmpHeight > INT_MAX ? INT_MAX : qwBmpHeight;
  }
  m_lpbi->biHeight = -nBmpHeight;

  UINT64 qwOffset = CalcAddress(topVS);

  long topRS = ConvYVirtualSpace2RealSpace(topVS*options.nBmpZoom+BMPSPACE);

  //ATLTRACE("dst: %d, %d, %d x %d\nsrc: %d, %d, %d x %d\n", BMPSPACE/*dstX*/, topRS/*dstY*/
  //    , options.nBmpWidth * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
  //    , 0/*srcX*/, 0/*srcY*/, options.nBmpWidth/*srcW*/, nBmpHeight/*srcH*/);
  //ATLTRACE("ADDR: UINT64 qwOffset(%016I64X) = CalcAddress(%016I64X)\n", qwOffset, topVS);

  DWORD dwReadSize = options.nBmpWidth * nBmpHeight * (options.nBmpColorWidth/8);
  if(dwReadSize<=pDoc->GetMaxCacheSize())
  {
    LPBYTE lpBits = pDoc->CacheForce(qwOffset, dwReadSize);
    if(lpBits)
      ::StretchDIBits(dc.m_hDC, BMPSPACE/*dstX*/, topRS/*dstY*/
      , options.nBmpWidth * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
      , 0/*srcX*/, 0/*srcY*/, options.nBmpWidth/*srcW*/, nBmpHeight/*srcH*/
      , lpBits/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);
    else 
    {
      ATLASSERT(FALSE);
    }
  } else {
    LPBYTE lpBits = (LPBYTE)malloc(dwReadSize);
    if(!lpBits)return;
    if(pDoc->Read(lpBits, qwOffset, dwReadSize))
    {
      ::StretchDIBits(dc.m_hDC, BMPSPACE/*dstX*/, topRS/*dstY*/
        , options.nBmpWidth * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
        , 0/*srcX*/, 0/*srcY*/, options.nBmpWidth/*srcW*/, nBmpHeight/*srcH*/
        , lpBits/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);
    } else {
      ATLASSERT(FALSE);
    }
    free(lpBits);
  }
}