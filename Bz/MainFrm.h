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

//#include "Combobar.h"
//#include "Splitter.h"
//#include "StatBar.h"
//#include "BZDoc2.h"
#include "SettingDlg.h"
#include "AboutDlg.h"
#include "ComboBox4ToolBar.h"

#define BZ_CLASSNAME "BzEditorClass"

extern int indicators[5];

class CBZDoc2;
class CBZView;
class CBZInspectView;

#define CHAIN_MSG_MAP_ACTIVEBZVIEW() \
	{ \
		if(ChainMsg2ActiveBZView(hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}
#define CHAIN_COMMANDS_ACTIVEBZVIEW() \
	if(uMsg == WM_COMMAND) \
		CHAIN_MSG_MAP_ACTIVEBZVIEW()

#define CHAIN_ACTIVEBZVIEW() \
	if(uMsg == WM_COMMAND||uMsg == WM_INITMENUPOPUP) \
		CHAIN_MSG_MAP_ACTIVEBZVIEW()

class CMainFrame : public WTL::CFrameWindowImpl<CMainFrame>, public WTL::CUpdateUI<CMainFrame>, public WTL::CIdleHandler
{
public:
  DECLARE_FRAME_WND_CLASS_EX(_T(BZ_CLASSNAME), IDR_MAINFRAME, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

  virtual BOOL OnIdle()
  {
    _OnInitMenuPopup();
    UIUpdateToolBar();
    //UIUpdateMenuBar();
    return FALSE;
  }

  BEGIN_MSG_MAP(CMainFrame)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_CLOSE(OnClose)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_SHOWWINDOW(OnShowWindow)
    MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
    //MSG_WM_SETMESSAGESTRING(WM_SETMESSAGESTRING, OnSetMessageString)MFCの独自メッセージ

    COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
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
    COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
    COMMAND_RANGE_HANDLER_EX(ID_VIEW_SPLIT_H, ID_VIEW_SPLIT_V, OnViewSplit)
    COMMAND_ID_HANDLER_EX(ID_TOOLS_EDITDEF, OnToolsEditBZDef)
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE_DUMPLIST, OnFileSaveDumpList)
    //ON_COMMAND( ID_HELP_INDEX, OnHelpIndex )
    COMMAND_RANGE_HANDLER_EX(ID_LANG_JPN, ID_LANG_ENU, OnLanguage)
    COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE, OnFileSave)
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
    COMMAND_ID_HANDLER_EX(ID_EDIT_READONLY, OnEditReadOnly)
    COMMAND_ID_HANDLER_EX(ID_EDIT_READONLYOPEN, OnEditReadOnlyOpen)
    COMMAND_ID_HANDLER_EX(ID_FILE_PRINT, OnFilePrint)
    COMMAND_ID_HANDLER_EX(ID_FILE_PRINT_SETUP, OnFilePrintSetup)

    COMMAND_RANGE_HANDLER_EX(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)

    CHAIN_ACTIVEBZVIEW()
    CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
  END_MSG_MAP()

  BOOL ChainMsg2ActiveBZView(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);

  BEGIN_UPDATE_UI_MAP(CMainFrame)
    UPDATE_ELEMENT(ID_VIEW_BITMAP, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_VIEW_STRUCT, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_VIEW_FULLPATH, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SUBCURSOR, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SYNCSCROLL, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_INSPECT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_ANALYZER, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_SPLIT_H, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_VIEW_SPLIT_V, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_LANG_JPN, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_LANG_ENU, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_READONLY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_EDIT_READONLYOPEN, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_FILE_SAVE_AS, UPDUI_MENUPOPUP)
  END_UPDATE_UI_MAP()

  void OnUpdateViewBitmap();
  void OnUpdateViewStruct()    { UISetCheck(ID_VIEW_STRUCT, m_bStructView); /*pCmdUI->Enable(!m_nSplitView);*/ }
  void OnUpdateViewFullpath()  { UISetCheck(ID_VIEW_FULLPATH, (options.barState & BARSTATE_FULLPATH)!=0); }
  void OnUpdateViewSubCursor() { UISetCheck(ID_VIEW_SUBCURSOR, options.bSubCursor); }
  void OnUpdateViewSyncScroll(){ UISetCheck(ID_VIEW_SYNCSCROLL, options.bSyncScroll); }
  void OnUpdateViewInspect()   { UISetCheck(ID_VIEW_INSPECT, m_bInspectView); }
  void OnUpdateViewAnalyzer()  { UISetCheck(ID_VIEW_ANALYZER, m_bAnalyzerView); }
  void OnUpdateViewSplitH()     { UISetCheck(ID_VIEW_SPLIT_H, m_nSplitView == ID_VIEW_SPLIT_H);/*pCmdUI->Enable(!m_bBmpView && !m_bStructView);*/ }
  void OnUpdateViewSplitV()     { UISetCheck(ID_VIEW_SPLIT_V, m_nSplitView == ID_VIEW_SPLIT_V);/*pCmdUI->Enable(!m_bBmpView && !m_bStructView);*/ }
  void OnUpdateLanguage() { UISetRadioMenuItem(ID_LANG_JPN + options.bLanguage, ID_LANG_JPN, ID_LANG_ENU); }
  void OnUpdateEditReadOnlyOpen() { UISetCheck(ID_EDIT_READONLYOPEN, options.bReadOnlyOpen); }
  void _OnInitMenuPopup();
  void OnInitMenuPopup(WTL::CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu);

  int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    {//OnCreateClient
      m_bStructView = options.bStructView;
      m_bInspectView = options.bInspectView;
      m_bAnalyzerView = options.bAnalyzerView;
      CreateClient();
    }
    if(options.ptFrame.x && options.ptFrame.y)
    {
      WINDOWPLACEMENT wndpl;
      GetWindowPlacement(&wndpl);
      if(g_bFirstInstance)
      {
        wndpl.rcNormalPosition.left = options.ptFrame.x;
        int newy = options.ptFrame.y;
        wndpl.rcNormalPosition.top = (newy<0)?0:newy;
      }
      wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + options.cyFrame;
      SetWindowPlacement(&wndpl);
    }

    {//Toolbar
      CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
      m_mainToolbar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
      AddSimpleReBarBand(m_mainToolbar);

      UIAddToolBar(m_mainToolbar);

      UINT idxFindBox = m_mainToolbar.CommandToIndex(ID_JUMP_FINDNEXT);
      idxFindBox--;
      TBBUTTONINFO tbinfo = {sizeof(TBBUTTONINFO), TBIF_SIZE|TBIF_BYINDEX|TBIF_COMMAND};
      m_mainToolbar.GetButtonInfo(idxFindBox, &tbinfo);
      tbinfo.cx = 180;
      tbinfo.idCommand = IDT_FINDBOX;
      m_mainToolbar.SetButtonInfo(idxFindBox, &tbinfo);
      WTL::CRect rectFindBox;
      m_mainToolbar.GetItemRect(idxFindBox, rectFindBox);
      m_combo_toolbar.Create(m_mainToolbar, rectFindBox, NULL
        , WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN, 0, IDT_FINDBOX);
      //m_combo_toolbar.SetFont(AtlGetDefaultGuiFont());
      //WTL::CEdit editBox = m_combo_toolbar.GetEditCtrl();
      //editBox.ModifyStyle(0, ES_RIGHT);

      SizeSimpleReBarBands();
    }

    {
      m_hWndStatusBar = m_statusbar.Create(m_hWnd);
      m_statusbar.SetPanes(indicators, sizeof(indicators)/sizeof(int));
      m_statusbar.SetText(1, _T(""));
      UIAddStatusBar(m_statusbar);
    }
    
    WTL::CMenuHandle menu = GetMenu();
    WTL::CMenuHandle menuFile = menu.GetSubMenu(0);
    WTL::CMenuHandle menuRecent = menuFile.GetSubMenu(2);
    m_recent.SetMenuHandle(menuRecent);
    m_recent.SetMaxEntries(15);
    m_recent.ReadFromRegistry(_T("Software\\c.mos\\BZ\\Settings"));
    
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    pCoreData->m_pMainFrame = this;

    WTL::CMessageLoop* pLoop = _Module.GetMessageLoop();
    pLoop->AddIdleHandler(this);

    Invalidate();
    /*
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
    */
    /*if (!m_wndStatusBar.Create(this) ||
      !m_wndStatusBar.SetIndicators(indicators,
      sizeof(indicators)/sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }*/

    // TODO: Remove this if you don't want tool tips or a resizeable toolbar
    //m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
   //   CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    // TODO: Delete these three lines if you don't want the toolbar to
    //  be dockable
    //m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    //EnableDocking(CBRS_ALIGN_ANY);
    //DockControlBar(&m_wndToolBar);

    //m_wndStatusBar.SetPaneInfo(0, ID_SEPARATOR,	SBPS_STRETCH | SBPS_NOBORDERS, 160);

    //ShowControlBar(&m_wndToolBar, (options.barState & BARSTATE_TOOL) != 0, FALSE);
    //ShowControlBar(&m_wndStatusBar, (options.barState & BARSTATE_STATUS) != 0, FALSE);

    //UINT nID;
    //UINT nStyle;
    //m_wndStatusBar.GetPaneInfo(1, nID, nStyle, m_nPaneWidth);
    //m_wndStatusBar.SetPaneText(1, _T(""));

    return 0;
  }
  void OnClose();
  void OnDestroy()
  {
    WTL::CMessageLoop* pLoop = _Module.GetMessageLoop();
    pLoop->RemoveIdleHandler(this);
    SetMsgHandled(FALSE);
  }
  void OnShowWindow(BOOL bShow, UINT nStatus);

  void OnFileExit(UINT uNotifyCode, int nID, CWindow wndCtl) { PostMessage(WM_CLOSE); }
  void OnJumpFind(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    //ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    //m_wndToolBar.m_combo.SetFocus();
    m_combo_toolbar.SetFocus();
  }
  void OnViewBitmap(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnViewStruct(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnViewInspect(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnViewAnalyzer(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnJumpTo(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    //ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    //m_wndToolBar.m_combo.SetFocus();
    //m_wndToolBar.m_combo.SetText(_T("> "));
    m_combo_toolbar.SetFocus();
    m_combo_toolbar.SetWindowText(_T("> "));
  }
  void OnEditValue(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    //ShowControlBar(&m_wndToolBar, TRUE, FALSE);
    //m_wndToolBar.m_combo.SetFocus();
    //m_wndToolBar.m_combo.SetText(_T("< %"));
    m_combo_toolbar.SetFocus();
    m_combo_toolbar.SetWindowText(_T("< %"));
  }
  void OnViewFullpath(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.barState ^= BARSTATE_FULLPATH;
    UpdateFrameTitle();
  }
  void OnToolsSetting(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    BOOL bDWordAddr = options.bDWordAddr;
    CSettingDlg dlgSetting;
    dlgSetting.DoModal();
    if(bDWordAddr != options.bDWordAddr) {
    }
  }
  void OnViewSubCursor(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnViewSyncScroll(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnHelpIndex(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    ShellExecute(NULL, _T("open"), _T("http://devil-tamachan.github.io/BZDoc/"), NULL, NULL, SW_SHOWNORMAL);
  }
  void OnAppAbout(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
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
  void OnFilePageSetup(UINT uNotifyCode, int nID, CWindow wndCtl) 
  {
/*    CPageSetupDialog dlg;
    PAGESETUPDLG& psd = dlg.m_psd;
    PRINTDLG pd;
    pd.hDevNames = NULL;
    pd.hDevMode = NULL;
    GetPrinterDeviceDefaults(&pd);
    psd.hDevNames = pd.hDevNames;
    psd.hDevMode = pd.hDevMode;
    psd.rtMargin = options.rMargin;
    if (dlg.DoModal() == IDOK) {
      dlg.GetMargins(options.rMargin, NULL);
      SelectPrinter(psd.hDevNames, psd.hDevMode);
    }*/
  }
  void OnToolsEditBZDef(UINT uNotifyCode, int nID, CWindow wndCtl) 
  {
    CString sPath;
    sPath = GetStructFilePath(IDS_STRUCT_FILE);

    CString sEditorPath;
    ::FindExecutable(_T("bz.txt"), NULL, sEditorPath.GetBuffer(_MAX_PATH));	// ###1.60
    sEditorPath.ReleaseBuffer();
    if(sEditorPath.IsEmpty()) ::ShellExecute(m_hWnd, _T("edit") , sPath, NULL, NULL, SW_SHOWNORMAL);
    else                      ::ShellExecute(m_hWnd, _T("open") , sEditorPath, sPath, NULL, SW_SHOWNORMAL);
  }
  void OnFileSaveDumpList(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnLanguage(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    BOOL bLang = nID - ID_LANG_JPN;
    if(bLang != options.bLanguage) {
      options.bLanguage = bLang;
      CString strMsg;
      strMsg.LoadString(IDS_CHANGE_LANGUAGE);
      MessageBox(strMsg);
    }
  }
  void OnFileOpen(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFileSave(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFileSaveAs(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnEditReadOnly(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnEditReadOnlyOpen(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFilePrint(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFilePrintSetup(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnFileRecent(UINT uNotifyCode, int nID, CWindow wndCtl);


public:
  CMainFrame()
  {
    m_pWndSplitter = NULL;
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
    if(m_pWndSplitter)
      delete m_pWndSplitter;
  }

  CBZView* GetActiveBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetActiveBZView();
  }
  CBZDoc2* GetActiveBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetActiveBZDoc2();
  }
  CTamaSplitterWindow* GetSplitter()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSplitterWnd();
  }


  void SetActiveView(CBZView *pBZView);


public:
	BOOL m_bDisableStatusInfo;
protected:
  CTamaSplitterWindow *m_pWndSplitter;
	int m_nPaneWidth;

public:
	BOOL m_bBmpView;
	BOOL m_bStructView;
	BOOL m_bInspectView;
	BOOL m_bAnalyzerView;
	UINT m_nSplitView;
	UINT m_nSplitView0;
	BOOL m_bCompare;

  CComboBox4ToolBar m_combo_toolbar;

  WTL::CPrinter m_printerCur;
  WTL::CDevMode m_devmodeCur;
  WTL::CPrintJob m_job;

  WTL::CToolBarCtrl m_mainToolbar;
  WTL::CMultiPaneStatusBarCtrl m_statusbar;

  WTL::CRecentDocumentList m_recent;


public:  // control bar embedded members
	//CStatusBarEx m_wndStatusBar;
	//CComboToolBar m_wndToolBar;

// Operations
public:
	void ChangeView(CBZView* pView);
	CBZView *GetBrotherView(CBZView* pView);

protected:
	//virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
  void AddRecentList(LPCTSTR path)
  {
    m_recent.AddToList(path);
    m_recent.WriteToRegistry(_T("Software\\c.mos\\BZ\\Settings"));
  }
  void AddSubView();
  void AddBZView();
  void ReSplit(BOOL bSubView);
  BOOL CreateSubView();
  BOOL PreparaSplitter();
  BOOL ResetSplitter();
  BOOL CreateClient();
  void UpdateFrameTitle();
  void GetSplitInfo()
  {
    if(IsIconic() || IsZoomed()) return;
/*    WTL::CRect rFrame;
    GetWindowRect(rFrame);

    int nCur, nMin;
    switch(options.nSplitView = m_nSplitView)
    {
    case 0:
      options.cyFrame = rFrame.Height();
      break;
    case ID_VIEW_SPLIT_H:
      options.cyFrame2 = rFrame.Height();
      m_pSplitter->GetRowInfo(0, nCur, nMin);
      options.ySplit = nCur;
      break;
    case ID_VIEW_SPLIT_V:
      options.cxFrame2 = rFrame.Width();
      m_pSplitter->GetColumnInfo(0, nCur, nMin);
      options.xSplit = nCur;
      if(m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView) {
        m_pSplitter->GetColumnInfo(1, nCur, nMin);
        options.xSplit += nCur;
      }
      break;
    }*/
    options.bStructView = m_bStructView;
    options.bInspectView = m_bInspectView;
    options.bAnalyzerView = m_bAnalyzerView;
    if(m_bStructView || m_bInspectView || m_bAnalyzerView) {
 //     m_pSplitter->GetColumnInfo(0, nCur, nMin);
 //     options.xSplitStruct = nCur;
    }
  }
  void GetFrameState()
  {
    WINDOWPLACEMENT wndpl;
    GetWindowPlacement(&wndpl);
    options.ptFrame.x = wndpl.rcNormalPosition.left;
    options.ptFrame.y = wndpl.rcNormalPosition.top;
    options.nCmdShow = wndpl.showCmd;
    options.barState = /*m_wndToolBar.IsWindowVisible()*/1 * BARSTATE_TOOL | /*m_wndStatusBar.IsWindowVisible()*/1 * BARSTATE_STATUS
      | options.barState & (BARSTATE_FULLPATH | BARSTATE_NOFLAT);
  }
  void UpdateInspectViewChecks();
  void DestroyAllChildWnd(BOOL bDelDoc)
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    pCoreData->DeleteAllViews(FALSE/*bDelDoc*/);

    if(m_pWndSplitter)
    {
      m_pWndSplitter->DestroyWindow();
      delete m_pWndSplitter;
      m_pWndSplitter = NULL;
      pCoreData->SetSplitterWnd(NULL);
    }
  }
};

//inline CMainFrame* GetMainFrame() { return (CMainFrame*)AfxGetMainWnd(); };

/////////////////////////////////////////////////////////////////////////////

