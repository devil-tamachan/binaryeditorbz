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

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView2 view

// #define BMPWIDTH	128		### 1.54c
#define BMPSPACE	8	//margine

/*
void MakeBzPallet256(DWORD *pRGB)
{
*pRGB++ = 0xFFFFFF;
for_to(i, 31) *pRGB++ = 0x00FFFF;
for_to_(i, 128-32) *pRGB++ = 0xFF0000;
for_to_(i, 128) *pRGB++ = 0x000000;
}*/

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



class CBZBmpView2 : public CTamaScrollWindowU64VImpl<CBZBmpView>
{
public:
  DECLARE_WND_CLASS(NULL)

  BEGIN_MSG_MAP(CTamaScrl64TestView)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_SIZE(OnSize)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_RBUTTONDOWN(OnRButtonDown)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_VSCROLL(OnVScroll)
    MSG_WM_KEYDOWN(OnKeyDown)
    COMMAND_RANGE_HANDLER_EX(ID_BMPVIEW_WIDTH128, ID_BMPVIEW_ZOOM, OnBmpViewMode)
    COMMAND_RANGE_HANDLER_EX(ID_BMPVIEW_8BITCOLOR, ID_BMPVIEW_32BITCOLOR, OnBmpViewColorWidth
    COMMAND_ID_HANDLER_EX(ID_BMPVIEW_ADDRESSTOOLTIP, OnBmpViewAddressTooltip)
    CHAIN_MSG_MAP(CTamaScrollWindowU64VImpl<CTamaScrl64TestView>)
  END_MSG_MAP()



  LRESULT OnCreate(LPCREATESTRUCT lpcs)
  {
    SetMsgHandled(false);
    m_cellV = 1;
    SetScrollSizeU64V(5000, _UI64_MAX/16);
    SetScrollPage(1,8);
    SetScrollLine(1,1);
    return 0;
  }

  BOOL PreTranslateMessage(MSG* pMsg)
  {
    pMsg;
    return FALSE;
  }

  void OnSize(UINT nType, CSize size)
  {
    if(bFirst)
    {
      OnInitialUpdate();
      bFirst = FALSE;
    }
  }

  BOOL OnEraseBkgnd(HDC hDC)
  {
    return false;
  }

  void OnLButtonDown(UINT nFlags, CPoint point)
  {
    m_isLButtonDown = true;

    point += GetScrollPosition();
    point.x -= BMPSPACE;
    point.y -= BMPSPACE;
    point.x /= options.nBmpZoom;
    point.y /= options.nBmpZoom;
    if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= 0) {
      DWORD dwPtr = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
      CBZView* pView = (CBZView*)GetNextWindow();
      if(dwPtr < pView->GetFileSize()) {
        pView->m_dwCaret = dwPtr;
        pView->GotoCaret();
        //pView->Activate();
      }
    }
  }

  void OnRButtonDown(UINT nFlags, CPoint point)
  {
    CMenu menu;
    menu.LoadMenu(IDR_BMPVIEW);
    CMenu* pMenu = menu.GetSubMenu(0);
    switch(options.nBmpWidth)
    {
    case 128:
      pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH128, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 256:
      pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH256, MF_BYCOMMAND | MF_CHECKED);
      break;
    }
    if(options.nBmpZoom > 1)
      pMenu->CheckMenuItem(ID_BMPVIEW_ZOOM, MF_BYCOMMAND | MF_CHECKED);
    switch(options.nBmpColorWidth)
    {
    case 8:
      pMenu->CheckMenuItem(ID_BMPVIEW_8BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 24:
      pMenu->CheckMenuItem(ID_BMPVIEW_24BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 32:
      pMenu->CheckMenuItem(ID_BMPVIEW_32BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    }
    if(options.bAddressTooltip)
      pMenu->CheckMenuItem(ID_BMPVIEW_ADDRESSTOOLTIP, MF_BYCOMMAND | MF_CHECKED);

    CPoint pt;
    GetCursorPos(&pt);
    pMenu->TrackPopupMenu(0, pt.x, pt.y, this);

    bHandled = FALSE;//CScrollView::OnRButtonDown(nFlags, point);
  }

  void OnMouseMove(UINT nFlags, CPoint point)
  {
    point += GetScrollPosition();
    point.x -= BMPSPACE;
    point.y -= BMPSPACE;
    point.x /= options.nBmpZoom;
    point.y /= options.nBmpZoom;
    if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= 0) {
      DWORD currentAddress = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
      if(currentAddress != m_tooltipLastAddress)
      {
        CBZView* pView = (CBZView*)GetNextWindow();
        if(currentAddress < pView->GetFileSize()) {
          if(m_isLButtonDown)
          {
            pView->m_dwCaret = currentAddress;
            pView->GotoCaret();
            //pView->Activate();
          } else if(options.bAddressTooltip) {
            TCHAR tmp[22];
            wsprintf(tmp, _T("0x%08X"), currentAddress);
            WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
            m_tooltip.UpdateTipText(toolinfo);
            ATLTRACE(_T("UpdateTooltip: %08X, %08X\n"), currentAddress, m_tooltipLastAddress);
            m_tooltipLastAddress = currentAddress;
            m_tooltip.Activate(true);
            m_tooltip.Popup();
            //CScrollView::OnMouseMove(nFlags, point);
            return;
          }
        }
      } else {
        ATLTRACE(_T("!!!UpdateTooltip: %08X, %08X\n"), currentAddress, m_tooltipLastAddress);
        return;
      }
    }

    m_tooltipLastAddress = 0xffffffff;
    m_tooltip.Activate(false);
    m_tooltip.Pop();
    //CScrollView::OnMouseMove(nFlags, point);
  }

  void OnLButtonUp(UINT nFlags, CPoint point)
  {
    m_isLButtonDown = false;
  }

  void OnVScroll(int nSBCode, short nPos, HWND hWnd)
  {
    ATLTRACE("OnVScroll\n");
    // TODO: Add your message handler code here and/or call default
    if(nSBCode == SB_THUMBTRACK) {		// ### 1.54
      SCROLLINFO si = { sizeof(SCROLLINFO) };
      GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS);
      TRACE("nPos, nTrackPos=%u, %d\n", nPos, si.nTrackPos);
      nPos = si.nTrackPos;
      TRACE("nPos, nTrackPos=%u, %d\n", nPos, si.nTrackPos);

      if(options.bAddressTooltip)
      {
        CPoint point = GetScrollPosition();
        point.x = max(point.x-BMPSPACE, 0);
        point.y = max(point.y-BMPSPACE, 0);
        point.x /= options.nBmpZoom;
        point.y /= options.nBmpZoom;
        DWORD currentAddress = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
        TCHAR tmp[22];
        wsprintf(tmp, _T("0x%08X"), currentAddress);
        WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
        m_tooltip.UpdateTipText(toolinfo);
        m_tooltip.Activate(true);
        m_tooltip.Popup();
      }
    } else if(nSBCode == SB_THUMBPOSITION) {
      m_tooltip.Activate(false);
      m_tooltip.Pop();
    }

    bHandled = FALSE;//CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
  }

  void OnKeyDown(TCHAR vkey, UINT repeats, UINT code)
  {
    switch(vkey)
    {
    case VK_DOWN:
      this->SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
      break;
    case VK_UP:
      this->SendMessage(WM_VSCROLL, SB_LINEUP, 0);
      break;
    case VK_NEXT://PageDown
      this->SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
      break;
    case VK_PRIOR://PageUp
      this->SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
      break;
    case VK_HOME:
      this->SendMessage(WM_VSCROLL, SB_TOP, 0);
      break;
    case VK_END:
      this->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
      break;
    }

    //CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
  }


  void OnBmpViewMode(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    switch(nID) {
case ID_BMPVIEW_WIDTH128:
  options.nBmpWidth = 128;
  break;
case ID_BMPVIEW_WIDTH256:
  options.nBmpWidth = 256;
  break;
case ID_BMPVIEW_ZOOM:
  options.nBmpZoom = (options.nBmpZoom == 1) ? 2 : 1;

    }
    GetMainFrame()->CreateClient();
  }


  void OnBmpViewColorWidth(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    switch(nID)
    {
    case ID_BMPVIEW_8BITCOLOR:
      options.nBmpColorWidth = 8;
      break;
    case ID_BMPVIEW_24BITCOLOR:
      options.nBmpColorWidth = 24;
      break;
    case ID_BMPVIEW_32BITCOLOR:
      options.nBmpColorWidth = 32;
      break;
    }
    GetMainFrame()->CreateClient();
  }

  void OnBmpViewAddressTooltip(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bAddressTooltip = !options.bAddressTooltip;
  }

  CBZBmpView2()
  {
    m_lpbi = NULL;
    m_tooltipLastAddress = 0xffffffff;
    m_isLButtonDown = false;
    bFirst = TRUE;
  }

  ~CBZBmpView2()
  {
    if(m_lpbi) MemFree(m_lpbi);
  }


private:
  long	m_cBmpX;
  LPBITMAPINFOHEADER m_lpbi;
  WTL::CToolTipCtrl m_tooltip;
  DWORD m_tooltipLastAddress;
  BOOL m_isLButtonDown;

  BOOL bFirst;//for OnInitialUpdate

public:

  void OnInitialUpdate()
  {
    //CScrollView::OnInitialUpdate();

    CBZDoc2* pDoc = (CBZDoc2*)GetDocument();
    UINT64 dwTotal = pDoc->GetDocSize();
    if(dwTotal < (DWORD)options.nBmpWidth) return;

    if(!m_lpbi) 
      m_lpbi = (LPBITMAPINFOHEADER)MemAlloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256/*256pallet*/);

    m_lpbi->biSize = sizeof(BITMAPINFOHEADER);
    m_cBmpX = options.nBmpWidth;
    UINT64 cBmpY64 = dwTotal / (UINT64)(options.nBmpWidth * (options.nBmpColorWidth/8));
    long cBmpYN;
    if(cBmpY64 > -LONG_MIN)cBmpYN = LONG_MIN;
    else cBmpYN = -((long)cBmpY64);
    Make8bitBITMAPINFOHEADER(m_lpbi, m_cBmpX, cBmpYN/*top-down DIB*/);


    DWORD* pRGB = (DWORD*)(m_lpbi+1);
    MakeSafetyPallet256(pRGB); //MakeBzPallet256(pRGB);
    //	MakeRedPallet256(pRGB);

    CDC* pDC = GetDC();
    HDC hDC = pDC->m_hDC;

    // TODO: calculate the total size of this view
    CSize cView;// = m_cBmp;
    cView.cx = m_cBmpX * options.nBmpZoom + BMPSPACE*2;
    cView.cy = cBmpY64 * options.nBmpZoom + BMPSPACE*2;
    SetScrollSizes(MM_TEXT, cView);

    TRACE("cView.cy=%X\n", GetTotalSize().cy);

    CSplitterWnd* pSplit = (CSplitter*)GetParent();
    ASSERT(pSplit->IsKindOf(RUNTIME_CLASS(CSplitterWnd)));
    cView.cx = options.nBmpWidth * options.nBmpZoom + BMPSPACE*2 + GetSystemMetrics(SM_CXVSCROLL)+1;
    pSplit->SetColumnInfo(0, cView.cx, 0);
    // MemFree(lpbi);

    int nMapMode;
    SIZE sizeTotal, sizePage, sizeLine;
    GetDeviceScrollSizes(nMapMode, sizeTotal, sizePage, sizeLine);
    sizePage.cy = 150;
    sizeLine.cy = 20;
    SetScrollSizes(nMapMode, sizeTotal, sizePage, sizeLine);

    if(m_tooltip.m_hWnd!=NULL)m_tooltip.DestroyWindow();
    m_tooltip.Create(m_hWnd, NULL, NULL, TTS_BALLOON|TTS_NOFADE|TTS_NOANIMATE|TTS_ALWAYSTIP);
    m_tooltip.SetDelayTime(TTDT_RESHOW, 0);
    m_tooltip.SetDelayTime(TTDT_AUTOPOP, 0xffff);
    m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
    m_tooltip.Activate(TRUE);
    WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, _T(""));
    m_tooltip.AddTool(toolinfo);
  }

  void DoPaint(WTL::CDCHandle dc)
  {
    dc.SetBkColor(RGB(255,255,255));
    CMemDC pMemDC(pDC);

    CBZDoc2* pDoc = (CBZDoc2*)GetDocument();
    ASSERT(pDoc);
    if(!pDoc->IsOpen())return; //if(pDoc->GetDocPtr()==NULL)return;

    CRect rClip;
    pMemDC->GetClipBox(rClip);

    rClip.top -= rClip.top % options.nBmpZoom;
    rClip.bottom += rClip.bottom % options.nBmpZoom;

    int nSpaceTop = (rClip.top < BMPSPACE) ? BMPSPACE - rClip.top : 0;
    long nBottom = m_cBmp.cy * options.nBmpZoom + BMPSPACE;
    if(rClip.bottom >= nBottom)
      rClip.bottom = nBottom;

    int nBmpHeight = (rClip.Height() - nSpaceTop) / options.nBmpZoom;
    m_lpbi->biHeight = -nBmpHeight;

    ATLTRACE("Clip: left=%ld, top=%ld(0x%08lX) %dx%d\n", rClip.left, rClip.top, rClip.top, rClip.Width(), rClip.Height());
    DWORD dwOffset = ((DWORD)rClip.top - (DWORD)(BMPSPACE - nSpaceTop)) / (DWORD)options.nBmpZoom * (DWORD)m_cBmpX;
    ATLTRACE("DWORD dwOffset 0x%08X = ((DWORD)rClip.top 0x%08X - (DWORD)(BMPSPACE 0x%X - nSpaceTop 0x%X)) * (DWORD)m_cBmp.cx 0x%08X / (DWORD)options.nBmpZoom 0x%08X;\n", dwOffset, (DWORD)rClip.top, BMPSPACE, nSpaceTop, m_cBmpX, options.nBmpZoom);
    dwOffset*=(DWORD)(options.nBmpColorWidth/8);
    ATLTRACE("dwOffset 0x%08X *=(DWORD)(options.nBmpColorWidth %ld /8);\n", dwOffset, options.nBmpColorWidth);

    DWORD dwReadSize = m_cBmpX * nBmpHeight * (options.nBmpColorWidth/8);
    if(dwReadSize<=pDoc->GetMaxCacheSize())
    {
      LPBYTE p = pDoc->CacheForce(dwOffset, dwReadSize);
      if(p)
        ::StretchDIBits(pMemDC->m_hDC, BMPSPACE/*dstX*/, rClip.top + nSpaceTop/*dstY*/
        , m_cBmpX * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
        , 0/*srcX*/, 0/*srcY*/, m_cBmpX/*srcW*/, nBmpHeight/*srcH*/
        , p/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);
    } else {
      LPBYTE lpBits = (LPBYTE)malloc(dwReadSize);
      if(!lpBits)return;
      if(pDoc->Read(lpBits, dwOffset, dwReadSize))
      {
        ::StretchDIBits(pMemDC->m_hDC, BMPSPACE/*dstX*/, rClip.top + nSpaceTop/*dstY*/
          , m_cBmpX * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
          , 0/*srcX*/, 0/*srcY*/, m_cBmpX/*srcW*/, nBmpHeight/*srcH*/
          , lpBits/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);
      }
      free(lpBits);
    }
  }
};

/////////////////////////////////////////////////////////////////////////////
