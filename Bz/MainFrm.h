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

#include "Combobar.h"
//#include "Splitter.h"
#include "StatBar.h"

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>
{
public:
  DECLARE_FRAME_WND_CLASS_EX(_T("BZMainFrame"), IDR_MAINFRAME, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

  BEGIN_MSG_MAP(CMainFrame)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_CLOSE(OnClose)
    MSG_WM_SHOWWINDOW(OnShowWindow)
    MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
//MSG_WM_SETMESSAGESTRING(WM_SETMESSAGESTRING, OnSetMessageString)MFCの独自メッセージ

    COMMAND_ID_HANDLER_EX(ID_JUMP_FIND, OnJumpFind)
    COMMAND_ID_HANDLER_EX(ID_VIEW_BITMAP, OnViewBitmap)
    COMMAND_ID_HANDLER_EX(ID_VIEW_STRUCT, OnViewStruct)
    COMMAND_ID_HANDLER_EX(ID_VIEW_INSPECT, OnViewInspect)
    COMMAND_ID_HANDLER_EX(ID_JUMP_TO, OnJumpTo)
    COMMAND_ID_HANDLER_EX(ID_EDIT_VALUE, OnEditValue)
    COMMAND_ID_HANDLER_EX(ID_VIEW_FULLPATH, OnViewFullpath)
    COMMAND_ID_HANDLER_EX(ID_TOOLS_SETTING, OnToolsSetting)
    COMMAND_ID_HANDLER_EX(ID_VIEW_SUBCURSOR, OnViewSubCursor)
    COMMAND_ID_HANDLER_EX(ID_VIEW_SYNCSCROLL, OnViewSyncScroll)
    COMMAND_ID_HANDLER_EX(ID_VIEW_ANALYZER, OnViewAnalyzer)
    COMMAND_ID_HANDLER_EX(ID_HELP_INDEX, OnHelpIndex)
    COMMAND_ID_HANDLER_EX(ID_VIEW_SPLIT_H, OnViewSplit)
    COMMAND_ID_HANDLER_EX(ID_VIEW_SPLIT_V, OnViewSplit)
    CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
  END_MSG_MAP()

  BEGIN_UPDATE_UI_MAP(CMainFrame)
    UPDATE_ELEMENT(ID_VIEW_BITMAP, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_STRUCT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_FULLPATH, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SUBCURSOR, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SYNCSCROLL, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_INSPECT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_ANALYZER, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SPLIT_H, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SPLIT_V, UPDUI_MENUPOPUP)
  END_UPDATE_UI_MAP()

  void OnUpdateViewBitmap() 
  {
    UISetCheck(ID_VIEW_BITMAP, m_bBmpView);
    CBZDoc2* pDoc = (CBZDoc2*)(GetActiveView()->GetDocument());
    UIEnable(ID_VIEW_BITMAP, pDoc->GetDocSize() >= (DWORD)(options.nBmpWidth * (options.nBmpColorWidth/8)) /* && !m_nSplitView*/);
  }
  void OnUpdateViewStruct()    { UISetCheck(ID_VIEW_STRUCT, m_bStructView); /*pCmdUI->Enable(!m_nSplitView);*/ }
  void OnUpdateViewFullpath()  { UISetCheck(ID_VIEW_FULLPATH, options.barState & BARSTATE_FULLPATH); }
  void OnUpdateViewSubCursor() { UISetCheck(ID_VIEW_SUBCURSOR, options.bSubCursor); }
  void OnUpdateViewSyncScroll(){ UISetCheck(ID_VIEW_SYNCSCROLL, options.bSyncScroll); }
  void OnUpdateViewInspect()   { UISetCheck(ID_VIEW_INSPECT, m_bInspectView); }
  void OnUpdateViewAnalyzer()  { UISetCheck(ID_VIEW_ANALYZER, m_bAnalyzerView); }
  void OnUpdateViewSplitH()     { UISetCheck(ID_VIEW_SPLIT_H, m_nSplitView == ID_VIEW_SPLIT_H);/*pCmdUI->Enable(!m_bBmpView && !m_bStructView);*/ }
  void OnUpdateViewSplitV()     { UISetCheck(ID_VIEW_SPLIT_V, m_nSplitView == ID_VIEW_SPLIT_V);/*pCmdUI->Enable(!m_bBmpView && !m_bStructView);*/ }
  OnUpdateViewSplit
  void OnInitMenuPopup(CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
  {
    OnUpdateViewBitmap();
    OnUpdateViewStruct();
    OnUpdateViewFullpath();
    OnUpdateViewSubCursor();
    OnUpdateViewSyncScroll();
    OnUpdateViewInspect();
    OnUpdateViewAnalyzer();
    OnUpdateViewSplitH();
    OnUpdateViewSplitV();
  }

  int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    if(options.ptFrame.x && options.ptFrame.y)
    {
      WINDOWPLACEMENT wndpl;
      GetWindowPlacement(&wndpl);
      if(((CBZApp*)AfxGetApp())->m_bFirstInstance)
      {
        wndpl.rcNormalPosition.left = options.ptFrame.x;
        int newy = options.ptFrame.y;
        wndpl.rcNormalPosition.top = (newy<0)?0:newy;
      }
      wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + options.cyFrame;
      SetWindowPlacement(&wndpl);
    }

    if (!(options.barState & BARSTATE_NOFLAT ? m_wndToolBar.Create(this)
      : m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP)) ||
      !m_wndToolBar.LoadToolBar(IDR_MAINFRAME)) {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }
    m_wndToolBar.m_cyTopBorder = 2;
    m_wndToolBar.m_cyBottomBorder = 2;
    m_wndToolBar.m_cxLeftBorder = 6;
    m_wndToolBar.m_cxRightBorder = 8;

    if (!m_wndToolBar.CreateComboBox(ID_JUMP_FIND, ID_JUMP_FINDNEXT, COMBO_WIDTH, COMBO_HEIGHT)) {
      TRACE0("Failed to create combobox in toolbar\n");
      return -1;      // fail to create
    }
    if (!m_wndStatusBar.Create(this) ||
      !m_wndStatusBar.SetIndicators(indicators,
      sizeof(indicators)/sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    // TODO: Remove this if you don't want tool tips or a resizeable toolbar
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
      CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    // TODO: Delete these three lines if you don't want the toolbar to
    //  be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    m_wndStatusBar.SetPaneInfo(0, ID_SEPARATOR,	SBPS_STRETCH | SBPS_NOBORDERS, 160);

    ShowControlBar(&m_wndToolBar, (options.barState & BARSTATE_TOOL) != 0, FALSE);
    ShowControlBar(&m_wndStatusBar, (options.barState & BARSTATE_STATUS) != 0, FALSE);

    UINT nID;
    UINT nStyle;
    m_wndStatusBar.GetPaneInfo(1, nID, nStyle, m_nPaneWidth);
    m_wndStatusBar.SetPaneText(1, _T(""));

    return 0;
  }
  void OnClose()
  {
    GetFrameState();
    GetSplitInfo();
  }
  void OnShowWindow(BOOL bShow, UINT nStatus)
  {
    if(bShow && m_nSplitView != ID_VIEW_SPLIT_V) {
      CView *pView = GetActiveView();
      if(pView->GetRuntimeClass() == RUNTIME_CLASS(CBZView))
        ((CBZView*)GetActiveView())->ResizeFrame();
    }
  }


  void OnJumpFind(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    m_wndToolBar.m_combo.SetFocus();
  }
  void OnViewBitmap(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    GetSplitInfo();
    m_bBmpView = !m_bBmpView;
    m_bStructView = FALSE;
    m_bInspectView = FALSE;
    m_bAnalyzerView = FALSE;
    CreateClient();
  }
  void OnViewStruct(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    GetSplitInfo();
    m_bBmpView = FALSE;
    m_bStructView = !m_bStructView;
    m_bInspectView = FALSE;
    m_bAnalyzerView = FALSE;
    CreateClient();
  }
  void OnViewInspect(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    GetSplitInfo();
    m_bBmpView = FALSE;
    m_bStructView = FALSE;
    m_bInspectView = !m_bInspectView;
    m_bAnalyzerView = FALSE;
    CreateClient();
  }
  void OnJumpTo(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    m_wndToolBar.m_combo.SetFocus();
    m_wndToolBar.m_combo.SetText(_T("> "));
  }
  void OnEditValue(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    m_wndToolBar.m_combo.SetFocus();
    m_wndToolBar.m_combo.SetText(_T("< %"));
  }
  void OnViewFullpath(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.barState ^= BARSTATE_FULLPATH;
    OnUpdateFrameTitle();
  }
  void OnToolsSetting(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    BOOL bDWordAddr = options.bDWordAddr;
    CSettingDlg().DoModal();
    if(bDWordAddr != options.bDWordAddr) {
    }
  }
  void OnViewSubCursor(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bSubCursor = !options.bSubCursor;
    GetActiveView()->Invalidate();
  }
  void OnViewSyncScroll(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bSyncScroll = !options.bSyncScroll;
    GetActiveView()->Invalidate();
  }
  void OnViewAnalyzer(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    GetSplitInfo();
    m_bBmpView = FALSE;
    m_bStructView = FALSE;
    m_bInspectView = FALSE;
    m_bAnalyzerView = !m_bAnalyzerView;
    CreateClient();
  }
  void OnHelpIndex(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    ShellExecute(NULL, _T("open"), _T("http://devil-tamachan.github.io/BZDoc/"), NULL, NULL, SW_SHOWNORMAL);
  }
  void OnViewSplit(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    /* 最大化していると最大化状態のままウィンドウサイズが調整されて変なウィンドウ（最大化できない＆サイズ変更不可）になってしまうバグの対策 */
    options.nCmdShow=SW_SHOWNORMAL;
    ShowWindow(options.nCmdShow);

    GetSplitInfo();
    m_nSplitView0 = m_nSplitView;
    m_nSplitView = (m_nSplitView == nID) ? 0 : nID;
    CreateClient();
    m_nSplitView0 = m_nSplitView;
  }

public:
  CMainFrame()
  {
    m_pSplitter = NULL;
    m_bBmpView = FALSE;
    m_bStructView = FALSE;
    m_bInspectView = FALSE;
    m_bAnalyzerView= FALSE;
    m_nSplitView = m_nSplitView0 = 0;
    m_bDisableStatusInfo = FALSE;

    //	EnableActiveAccessibility();
  }
  ~CMainFrame()
  {
    if(m_pSplitter)
      delete m_pSplitter;
  }

public:
	BOOL m_bDisableStatusInfo;
protected:
	CSplitterWnd* m_pSplitter;
	int m_nPaneWidth;

public:
	BOOL m_bBmpView;
	BOOL m_bStructView;
	BOOL m_bInspectView;
	BOOL m_bAnalyzerView;
	UINT m_nSplitView;
	UINT m_nSplitView0;
	BOOL m_bCompare;

// Operations
public:
	void ChangeView(CView* pView);
	CView *GetBrotherView(CView* pView);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle = TRUE);
	BOOL CreateClient(CCreateContext* pContext = NULL);
	void GetSplitInfo();
	void GetFrameState();


public:  // control bar embedded members
	CStatusBarEx m_wndStatusBar;
	CComboToolBar m_wndToolBar;

protected:
	//afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
public:
	void UpdateInspectViewChecks();
	void DeleteSplitterWnd(CCreateContext* pContext);

public:
};

#define BZ_CLASSNAME "BzEditorClass"

inline CMainFrame* GetMainFrame() { return (CMainFrame*)AfxGetMainWnd(); };

/////////////////////////////////////////////////////////////////////////////

