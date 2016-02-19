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
CString GetCustomPalletFilePath();

//#include "tamascrlu64v.h"

#define CUSTOMPALLET 10

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
    COMMAND_RANGE_HANDLER_EX(ID_BMPVIEW_CUSTOMPALETTE_START, ID_BMPVIEW_CUSTOMPALETTE_END, OnPalletModeCustom)
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

  void OnLButtonDown(UINT nFlags, WTL::CPoint point);
  
  void ReloadCustomPallets()
  {
      pallets.RemoveAll();
      WTL::CFindFile find;
      if(find.FindFile(GetCustomPalletFilePath()))
      {
        do
        {
          if(find.IsDots() || find.IsDirectory())continue;
          CPath path(find.GetFilePath());
          CString ext = path.GetExtension();
          cpallet_t cp;
          cp.path = find.GetFilePath();
          if(ext==".txt")
          {
            cp.type = cpallet_t::CP_TXT;
            pallets.Add(cp);
          } else if(ext==".apl") {
            cp.type = cpallet_t::CP_APL;
            pallets.Add(cp);
          }
        } while(find.FindNextFile());
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

    ReloadCustomPallets();
    int iPallets = pallets.GetCount();
    for(int i=0;i<iPallets;i++)
    {
      CString fileName(PathFindFileName(pallets[i].path));
      pMenu.AppendMenu(options.lastPalletName==fileName && options.nBmpPallet>=CUSTOMPALLET && options.nBmpColorWidth==8 ? MF_STRING | MF_CHECKED : MF_STRING, ID_BMPVIEW_CUSTOMPALETTE_START+i, fileName);
    }

    WTL::CPoint pt;
    GetCursorPos(&pt);
    pMenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);

    SetMsgHandled(FALSE);//CScrollView::OnRButtonDown(nFlags, point);
  }

  void OnMouseMove(UINT nFlags, WTL::CPoint point);

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


  void OnBmpViewMode(UINT uNotifyCode, int nID, CWindow wndCtl);


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

  void OnPalletModeCustom(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    int i = nID - ID_BMPVIEW_CUSTOMPALETTE_START;
    if(i<pallets.GetCount())
    {
      options.nBmpPallet = i+CUSTOMPALLET;
      options.nBmpColorWidth = 8;
    }
    //GetMainFrame()->CreateClient();
    OnInitialUpdate();
    Invalidate();
    UpdateWindow();
  }
  
  void LoadAplPallet256(DWORD *pRGB, LPCTSTR path)
  {
    FILE *fp = _wfopen(path, L"rb");
    if(fp==NULL)return;
    char sig[16];
    if(fread(sig, 1, 6, fp)!=6)goto err_LoadAplPallet256;
    sig[6]=NULL;
    if(strcmp(sig, "AZPPAL")!=0)goto err_LoadAplPallet256;
    WORD w=0;
    if(fread(&w, 1, 1, fp)!=1 || w!=0)goto err_LoadAplPallet256;
    w=0;
    if(fread(&w, 1, 2, fp)!=2 || w<256)goto err_LoadAplPallet256;
    for(int i=0;i<w && i<256;i++)
    {
      if(fread(pRGB, 1, 4, fp)!=4)goto err_LoadAplPallet256;
      pRGB++;
    }
    options.lastPalletName=PathFindFileName(path);

err_LoadAplPallet256:
    fclose(fp);
  }
  
  struct cpallet_tag
  {
    enum {CP_APL=0, CP_TXT} type;
    CString path;
  };
  typedef struct cpallet_tag cpallet_t;
  HRESULT LoadTxtPallet256(DWORD *pRGB, LPCTSTR path);
  
  void LoadCustomPallet256(DWORD *pRGB, cpallet_t cp)
  {
    switch(cp.type)
    {
      case cpallet_t::CP_APL:
        LoadAplPallet256(pRGB, cp.path);
        break;
      case cpallet_t::CP_TXT:
        LoadTxtPallet256(pRGB, cp.path);
        break;
    }
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

  CAtlArray<cpallet_t> pallets;

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

  void OnInitialUpdate();
  
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

  void DoPaint(WTL::CDCHandle dc);

  int GetWindowIdealWidth()
  {
    return m_sizeAll.cx + BMPSPACE + GetSystemMetrics(SM_CXVSCROLL);
  }
};

/////////////////////////////////////////////////////////////////////////////
