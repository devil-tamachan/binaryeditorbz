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

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView2 view

// #define BMPWIDTH	128		### 1.54c
#define BMPSPACE	8	//margine

void MakeBzPallet256(DWORD *pRGB);
void MakeRedPallet256(DWORD *pRGB);
void MakeSafetyPallet256(DWORD *pRGB);
void Make8bitBITMAPINFOHEADER(LPBITMAPINFOHEADER lpbi, LONG w, LONG h);

//#include "tamascrlu64v.h"


#include "BZSubView.h"

class CBZBmpView2 : public CTamaScrollWindowU64VImpl<CBZBmpView2>, public CBZSubView
{
public:
  DECLARE_WND_CLASS(NULL)

  BEGIN_MSG_MAP(CBZBmpView2)
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
    COMMAND_RANGE_HANDLER_EX(ID_BMPVIEW_8BITCOLOR, ID_BMPVIEW_32BITCOLOR, OnBmpViewColorWidth)
    COMMAND_ID_HANDLER_EX(ID_BMPVIEW_ADDRESSTOOLTIP, OnBmpViewAddressTooltip)
    COMMAND_RANGE_HANDLER_EX(ID_BMPVIEW_PALETTE_BZ, ID_BMPVIEW_PALETTE_SAFETY, OnPalletMode)
    CHAIN_MSG_MAP(CTamaScrollWindowU64VImpl<CBZBmpView2>)
  END_MSG_MAP()



  LRESULT OnCreate(LPCREATESTRUCT lpcs)
  {
    SetMsgHandled(false);
    m_cellV = 1;

    LONG lExStyle = GetWindowLong(GWL_EXSTYLE);
    lExStyle |= WS_EX_STATICEDGE;
    SetWindowLong(GWL_EXSTYLE, lExStyle);

    return 0;
  }

  BOOL PreTranslateMessage(MSG* pMsg)
  {
    pMsg;
    return FALSE;
  }

  void OnSize(UINT nType, WTL::CSize size)
  {
    SetMsgHandled(false);
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

  void OnLButtonDown(UINT nFlags, WTL::CPoint point)
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

  void OnRButtonDown(UINT nFlags, WTL::CPoint point)
  {
    if(!bEnable)return;

    WTL::CMenu menu;
    menu.LoadMenu(IDR_BMPVIEW);
    WTL::CMenuHandle pMenu = menu.GetSubMenu(0);
    switch(options.nBmpWidth)
    {
    case 128:
      pMenu.CheckMenuItem(ID_BMPVIEW_WIDTH128, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 256:
      pMenu.CheckMenuItem(ID_BMPVIEW_WIDTH256, MF_BYCOMMAND | MF_CHECKED);
      break;
    }
    if(options.nBmpZoom > 1)
      pMenu.CheckMenuItem(ID_BMPVIEW_ZOOM, MF_BYCOMMAND | MF_CHECKED);
    switch(options.nBmpColorWidth)
    {
    case 8:
      switch(options.nBmpPallet)
      {
      case 0:
        pMenu.CheckMenuItem(ID_BMPVIEW_PALETTE_BZ, MF_BYCOMMAND | MF_CHECKED);
        break;
      case 1:
        pMenu.CheckMenuItem(ID_BMPVIEW_PALETTE_SAFETY, MF_BYCOMMAND | MF_CHECKED);
        break;
      }
      pMenu.CheckMenuItem(ID_BMPVIEW_8BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 24:
      pMenu.CheckMenuItem(ID_BMPVIEW_24BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    case 32:
      pMenu.CheckMenuItem(ID_BMPVIEW_32BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
      break;
    }
    if(options.bAddressTooltip)
      pMenu.CheckMenuItem(ID_BMPVIEW_ADDRESSTOOLTIP, MF_BYCOMMAND | MF_CHECKED);

    WTL::CPoint pt;
    GetCursorPos(&pt);
    pMenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);

    SetMsgHandled(FALSE);//CScrollView::OnRButtonDown(nFlags, point);
  }

  void OnMouseMove(UINT nFlags, WTL::CPoint point)
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

  void OnLButtonUp(UINT nFlags, WTL::CPoint point)
  {
    m_isLButtonDown = false;
  }

  void OnVScroll(int nSBCode, short nPos, HWND hWnd)
  {
    ATLTRACE("OnVScroll\n");

    if(nSBCode == SB_THUMBTRACK) {		// ### 1.54
      if(options.bAddressTooltip)
      {
        UINT64 yVS = CalcY(GetScrollPosU64V());
        UINT64 currentAddress = CalcAddress(yVS);
        TCHAR tmp[22];
        wsprintf(tmp, _T("0x%016I64X"), currentAddress);
        WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
        m_tooltip.UpdateTipText(toolinfo);
        m_tooltip.Activate(true);
        m_tooltip.Popup();
      }
    } else if(nSBCode == SB_THUMBPOSITION) {
      m_tooltip.Activate(false);
      m_tooltip.Pop();
    }

    SetMsgHandled(FALSE);//CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
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
    //GetMainFrame()->CreateClient();
    OnInitialUpdate();
    Invalidate();
  }

  void OnBmpViewAddressTooltip(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bAddressTooltip = !options.bAddressTooltip;
  }

  void OnPalletMode(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    switch(nID)
    {
    case ID_BMPVIEW_PALETTE_BZ:
      options.nBmpPallet = 0;
      options.nBmpColorWidth = 8;
      break;
    case ID_BMPVIEW_PALETTE_SAFETY:
      options.nBmpPallet = 1;
      options.nBmpColorWidth = 8;
      break;
    default:
      return;
    }
    //GetMainFrame()->CreateClient();
    OnInitialUpdate();
    Invalidate();
    UpdateWindow();
  }

  CBZBmpView2()
  {
    m_lpbi = NULL;
    m_tooltipLastAddress = 0xffffffff;
    m_isLButtonDown = false;
    bFirst = TRUE;
    bEnable = FALSE;
  }

  ~CBZBmpView2()
  {
    if(m_lpbi) MemFree(m_lpbi);
  }


private:
  UINT64	m_cBmpY64;
  LPBITMAPINFOHEADER m_lpbi;
  WTL::CToolTipCtrl m_tooltip;
  UINT64 m_tooltipLastAddress;
  BOOL m_isLButtonDown;

  BOOL bFirst;//for OnInitialUpdate
  BOOL bEnable;

public:

  CBZView* GetBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZViewFromSubView(this);
  }

  CBZDoc2* GetBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZDoc2FromSubView(this);
  }

  CTamaSplitterWindow* GetSplitter()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSplitterWnd();
  }

  void InitSubView()
  {
    OnInitialUpdate();
  }

  void OnInitialUpdate()
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


    DWORD* pRGB = (DWORD*)(m_lpbi+1);
    if(options.nBmpPallet==0)MakeBzPallet256(pRGB);
    else MakeSafetyPallet256(pRGB);
    //	MakeRedPallet256(pRGB);

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
  
  UINT64 CalcY(UINT64 yVS)
  {
    if(yVS > BMPSPACE)yVS = (yVS - BMPSPACE)/options.nBmpZoom;
    else yVS = 0;
    
    return yVS;
  }
    
  UINT64 CalcAddress(UINT64 yVS)
  {
    UINT64 qwOffset = yVS * options.nBmpWidth;
    qwOffset*=(DWORD)(options.nBmpColorWidth/8);
    return qwOffset;
  }

  void DoPaint(WTL::CDCHandle dc)
  {
    WTL::CRect rClipOrig;
    dc.GetClipBox(rClipOrig);
    dc.FillSolidRect(rClipOrig, RGB(0xFF, 0xFF, 0xFF));

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

  int GetWindowIdealWidth()
  {
    return m_sizeAll.cx + BMPSPACE + GetSystemMetrics(SM_CXVSCROLL);
  }
};

/////////////////////////////////////////////////////////////////////////////
