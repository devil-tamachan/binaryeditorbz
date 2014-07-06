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
#include "BZ.h"

#include "MainFrm.h"
#include "BZDoc2.h"
#include "BZView.h"
#include "BZBmpVw2.h"
#include "BZFormVw.h"
#include "BZInspectView.h"
#include "BZAnalyzerView.h"
#include "SettingDlg.h"


#define COMBO_WIDTH  180	// drop down width
#define COMBO_HEIGHT 200	// width of edit control for combobox

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

int indicators[5] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_INFO,
	ID_INDICATOR_SIZE,
	ID_INDICATOR_CHAR,
	ID_INDICATOR_INS,
};

BOOL CMainFrame::ChainMsg2ActiveBZView(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
  CBZView *pActiveBZView = GetActiveBZView();
  if(pActiveBZView && pActiveBZView->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult))
    return TRUE;
  return FALSE;
}

void CMainFrame::SetActiveView(CBZView *pBZView)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  pCoreData->SetActiveByBZView(pBZView);
  pBZView->SetFocus();
  UpdateFrameTitle();
}


void CMainFrame::OnUpdateViewBitmap() 
{
  UISetCheck(ID_VIEW_BITMAP, m_bBmpView);
  CBZDoc2* pDoc = GetActiveBZDoc2();
  UIEnable(ID_VIEW_BITMAP, pDoc->GetDocSize() >= (DWORD)(options.nBmpWidth * (options.nBmpColorWidth/8)) /* && !m_nSplitView*/);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
  //if(bShow && m_nSplitView != ID_VIEW_SPLIT_V) {
  //  CBZView *pBZView = GetActiveBZView();
  //  if(pBZView)
  //    pBZView->ResizeFrame();
  //}
}
void CMainFrame::OnViewSubCursor(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  options.bSubCursor = !options.bSubCursor;
  GetActiveBZView()->Invalidate();
}
void CMainFrame::OnViewSyncScroll(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  options.bSyncScroll = !options.bSyncScroll;
  GetActiveBZView()->Invalidate();
}
void CMainFrame::OnFileSaveDumpList(UINT uNotifyCode, int nID, CWindow wndCtl) 
{
  CString sFileName = GetActiveBZDoc2()->GetFilePath();
  sFileName += _T(".lst");

  WTL::CFileDialog dlg(FALSE, _T("*.lst"), sFileName, OFN_NOVALIDATE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT/*OFN_PATHMUSTEXIST*/
    , _T("lst ファイル (*.lst)\0*.lst\0\0"), m_hWnd);

  if(dlg.DoModal() == IDOK){
    CAtlFile file;
    //if(file.Open(sFileName, CFile::modeCreate | CFile::modeWrite)) {
    if(SUCCEEDED(file.Create(sFileName, GENERIC_WRITE, 0, CREATE_ALWAYS)))
    {
      CBZView *pBZView = GetActiveBZView();
      if(pBZView)pBZView->DrawToFile(&file);
      file.Close();
    }
  }
}
void CMainFrame::OnFileNew(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZView *pBZView = pCoreData->GetActiveBZView();
  if(pBZView)
  {
    if(!(pBZView->AskSave()))return; //Cancel Close
    CBZDoc2 *pNewDoc = new CBZDoc2();
    pCoreData->ReplaceActiveBZDoc2(pNewDoc, TRUE);
    pBZView->Update();
  }
  UpdateFrameTitle();
}
void CMainFrame::OnFileOpen(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZView *pBZView = pCoreData->GetActiveBZView();
  if(pBZView)
  {
    CBZDoc2 *pDoc = pBZView->GetBZDoc();
    if(pDoc)pDoc->OnFileOpen(NULL, m_hWnd);
    pBZView->Update();
  }
  UpdateFrameTitle();
}
void CMainFrame::OnFileSave(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
  if(pDoc)pDoc->OnFileSave();
  UpdateFrameTitle();
}
void CMainFrame::OnFileSaveAs(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
  if(pDoc)pDoc->OnFileSaveAs();
  UpdateFrameTitle();
}
void CMainFrame::OnEditReadOnly(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
  if(pDoc)pDoc->OnEditReadOnly();
}
void CMainFrame::OnEditReadOnlyOpen(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  options.bReadOnlyOpen = !options.bReadOnlyOpen;
}

void CMainFrame::OnFilePrint(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CPrintDialogEx dlg;

  CBZView *bzView = GetActiveBZView();
  if(bzView==NULL || bzView->m_bPrinting)return;

  bzView->m_bBeginPrintJob = FALSE;

  dlg.m_pdex.hDevMode = m_devmodeCur.CopyToHDEVMODE();
  dlg.m_pdex.hDevNames = m_printerCur.CopyToHDEVNAMES();
	dlg.m_pdex.nMinPage = 1;
	if(bzView->m_nPageLen)dlg.m_pdex.nMaxPage = bzView->GetMaxPrintingPage();

  if(SUCCEEDED(dlg.DoModal()))
  {
    if(dlg.m_pdex.dwResultAction == PD_RESULT_PRINT ||
      dlg.m_pdex.dwResultAction == PD_RESULT_APPLY)
    {
      m_devmodeCur.CopyFromHDEVMODE(dlg.m_pdex.hDevMode);

      m_printerCur.ClosePrinter();
      m_printerCur.OpenPrinter(dlg.m_pdex.hDevNames, m_devmodeCur);
    }

    if(dlg.m_pdex.dwResultAction == PD_RESULT_PRINT)
    {
      m_job.StartPrintJob(false, m_printerCur, m_devmodeCur, bzView,
        bzView->m_pDoc->GetDocName(), 0, 0, (dlg.PrintToFile() != FALSE));
    }
  }
  ::GlobalFree(dlg.m_pdex.hDevMode);
  ::GlobalFree(dlg.m_pdex.hDevNames);
}

void CMainFrame::OnFilePrintSetup(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CPageSetupDialog dlg;

  dlg.m_psd.hDevMode = m_devmodeCur.CopyToHDEVMODE();
  dlg.m_psd.hDevNames = m_printerCur.CopyToHDEVNAMES();
  dlg.m_psd.rtMargin = options.rMargin;

  if(dlg.DoModal() == IDOK)
  {
    m_devmodeCur.CopyFromHDEVMODE(dlg.m_psd.hDevMode);

    m_printerCur.ClosePrinter();
    m_printerCur.OpenPrinter(dlg.m_psd.hDevNames, m_devmodeCur);
    dlg.GetMargins(options.rMargin, NULL);
  }
  ::GlobalFree(dlg.m_psd.hDevMode);
  ::GlobalFree(dlg.m_psd.hDevNames);
}
void CMainFrame::OnFilePrintPreview(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZView *bzView = GetActiveBZView();
  if(bzView==NULL || m_printerCur.IsNull() || m_devmodeCur.IsNull())return;

  CTamaSplitterWindow *pSplitter = GetSplitter();
  if(m_bPrintPreview){
    m_hWndClient = NULL;
    if(pSplitter)
    {
      pSplitter->ShowWindow(SW_SHOW);
      m_hWndClient = pSplitter->m_hWnd;
    }
    m_wndPrintPreview.DestroyWindow();
    bzView->m_bBeginPrintJob = FALSE;
    m_bPrintPreview = FALSE;
    bzView->EndPrintJob(0,0);
  } else {
    bzView->m_bBeginPrintJob = FALSE;
    m_wndPrintPreview.SetPrintPreviewInfo(m_printerCur, m_devmodeCur, bzView, 0, 0);
    m_wndPrintPreview.SetPage(0);
    m_wndPrintPreview.Create(m_hWnd, rcDefault, NULL, 0, /*WS_EX_CLIENTEDGE |*/ WS_EX_STATICEDGE);
    m_hWndClient = m_wndPrintPreview;
    if(pSplitter)pSplitter->ShowWindow(SW_HIDE);
    m_bPrintPreview = TRUE;
  }
 // UISetCheck(ID_FILE_PRINT_PREVIEW, m_bPrintPreview);

  UpdateLayout();
}

void CMainFrame::OnFileRecent(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CString path;
  if(m_recent.GetFromList(nID, path))
  {
    WTL::CFindFile find;
    if(find.FindFile(path))
    {
      m_recent.MoveToTop(nID);
      CBZCoreData *pCoreData = CBZCoreData::GetInstance();
      CBZView *pBZView = pCoreData->GetActiveBZView();
      if(pBZView)
      {
        CBZDoc2 *pDoc = pBZView->GetBZDoc();
        if(pDoc)pDoc->OnFileOpen(path, m_hWnd);
        pBZView->Update();
      }
      UpdateFrameTitle();
    } else m_recent.RemoveFromList(nID);

    m_recent.WriteToRegistry(_T("Software\\c.mos\\BZ\\Settings"));
  }
}

void CMainFrame::OnToolsSetting(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  BOOL bQWordAddr = options.bQWordAddr;
  BOOL bClearUndoRedoWhenSave = options.bClearUndoRedoWhenSave;
  CSettingDlg dlgSetting;
  dlgSetting.DoModal();
  if(bQWordAddr != options.bQWordAddr)
  {
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
  }
  if(bClearUndoRedoWhenSave != options.bClearUndoRedoWhenSave)
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    DWORD nDoc = pCoreData->GetCountBZDoc2();
    for(DWORD i=0; i<nDoc; i++)
    {
      CBZDoc2 *doc = pCoreData->GetBZDoc2(i);
      if(doc)doc->SetClearUndoRedoWhenSave(options.bClearUndoRedoWhenSave);
    }
  }
}


void CMainFrame::_OnInitMenuPopup()
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
  OnUpdateLanguage();
  OnUpdateEditReadOnlyOpen();
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
    if(pDoc)
    {
      BOOL bEnable = FALSE, bChecked = FALSE;
      pDoc->OnUpdateEditReadOnly(&bEnable, &bChecked);
      UIEnable(ID_EDIT_READONLY, bEnable);
      UISetCheck(ID_EDIT_READONLY, bChecked);
      UIEnable(ID_EDIT_UNDO, pDoc->OnUpdateEditUndo());
      UIEnable(ID_EDIT_REDO, pDoc->OnUpdateEditRedo());
      UIEnable(ID_FILE_SAVE, pDoc->OnUpdateFileSave());
      UIEnable(ID_FILE_SAVE_AS, pDoc->OnUpdateFileSaveAs());
    }
  }
  OnUpdatePrintPreview();
}

void CMainFrame::OnInitMenuPopup(WTL::CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
{
  _OnInitMenuPopup();
  SetMsgHandled(FALSE);
}

void CMainFrame::OnViewBitmap(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  GetSplitInfo();
  m_bBmpView = !m_bBmpView;
  m_bStructView = FALSE;
  m_bInspectView = FALSE;
  m_bAnalyzerView = FALSE;
  if(m_bBmpView)CreateSubView();
  else pCoreData->DeleteAllSubViews();
  ResetSplitter();
  //CreateClient();
}
void CMainFrame::OnViewStruct(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  GetSplitInfo();
  m_bBmpView = FALSE;
  m_bStructView = !m_bStructView;
  m_bInspectView = FALSE;
  m_bAnalyzerView = FALSE;
  if(m_bStructView)CreateSubView();
  else pCoreData->DeleteAllSubViews();
  ResetSplitter();
  //CreateClient();
}
void CMainFrame::OnViewInspect(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  GetSplitInfo();
  m_bBmpView = FALSE;
  m_bStructView = FALSE;
  m_bInspectView = !m_bInspectView;
  m_bAnalyzerView = FALSE;
  if(m_bInspectView)CreateSubView();
  else pCoreData->DeleteAllSubViews();
  ResetSplitter();
  //CreateClient();
}
void CMainFrame::OnViewAnalyzer(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  GetSplitInfo();
  m_bBmpView = FALSE;
  m_bStructView = FALSE;
  m_bInspectView = FALSE;
  m_bAnalyzerView = !m_bAnalyzerView;
  if(m_bAnalyzerView)CreateSubView();
  else pCoreData->DeleteAllSubViews();
  ResetSplitter();
  //CreateClient();
}

LRESULT CMainFrame::OnStatusBarClicked(LPNMHDR pnmh)
{
  NMMOUSE *pmmouse = (NMMOUSE *)pnmh;
  int paneIdx = pmmouse->dwItemSpec;
  
  CBZView *pBZView = GetActiveBZView();
  if(pBZView)
  {
    switch(paneIdx)
    {
    case 1:
      pBZView->OnStatusInfo();
      break;
    case 2:
      pBZView->OnStatusSize();
      break;
    case 3:
      pBZView->OnStatusChar();
      break;
    }
    pBZView->_OnInitMenuPopup();
    pBZView->UIUpdateStatusBar();
  }
  return 0;
}


void CMainFrame::OnClose()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  DWORD cntBzView = pCoreData->GetCountBZView();
  for(DWORD i=0;i<cntBzView;i++)
  {
    CBZView *bzView = pCoreData->GetBZView(i);
    if(!(bzView->AskSave()))return; //Cancel Close
    bzView->Invalidate();
  }

  GetFrameState();
  GetSplitInfo();

  pCoreData->m_pMainFrame = NULL;
  pCoreData->RemoveAllDocs();

  SetMsgHandled(FALSE);
}



void CMainFrame::UpdateInspectViewChecks()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  DWORD dwBZViews = pCoreData->GetCountBZView();
  if(m_bInspectView && dwBZViews >= 2)
  {
    CBZInspectView *pInsView0 = (CBZInspectView*)pCoreData->GetSubView(0);
    if(pInsView0)
    {
      pInsView0->_UpdateChecks();
      pInsView0->Update();
    }
    CBZInspectView *pInsView1 = (CBZInspectView*)pCoreData->GetSubView(1);
    if(pInsView1)
    {
      pInsView1->_UpdateChecks();
      pInsView1->Update();
    }
  }
}
void CMainFrame::UpdateFrameTitle()
{
  CBZDoc2* pDoc = GetActiveBZDoc2();
  if(pDoc) {
    CString s(_T("BZ"));
    s += " - ";
    CString sPath = pDoc->GetFilePath();
    if(!(options.barState & BARSTATE_FULLPATH) || sPath.IsEmpty())
    {
      sPath = PathFindFileName(sPath);//pDoc->GetTitle();
    }
    s += sPath;
    //		s += pDoc->IsFileMapping()?_T(" (FileMap)"):_T(" (Mem)");
    if(pDoc->IsModified())
      s += " *";
    SetWindowText(s);
  }
}


//BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)   たぶん削除しても問題ない。一応残しておく
//{
//	if(CFrameWnd::PreCreateWindow(cs)) {
//		WNDCLASSEX wc;
//		wc.cbSize = sizeof(WNDCLASSEX);
//		VERIFY(::GetClassInfoEx(AfxGetInstanceHandle(), cs.lpszClass, &wc));
//		wc.lpszClassName = _T(BZ_CLASSNAME);
//		wc.hIcon  = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
//		wc.hIconSm = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);
//		::RegisterClassEx(&wc);
//		cs.lpszClass = _T(BZ_CLASSNAME);
//		return TRUE;
//	}
//	return FALSE;
//}

void CMainFrame::AddSubView()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  if(m_bBmpView)
  {
    CBZBmpView2 *pBmpView = new CBZBmpView2;
    pCoreData->AddSubView(pBmpView);
    pBmpView->Create(m_pWndSplitter->m_hWnd);
    //pBmpView->OnInitialUpdate();
  } else if(m_bInspectView) {
    CBZInspectView *pInsView = new CBZInspectView;
    pCoreData->AddSubView(pInsView);
    HWND hWnd = pInsView->Create(m_pWndSplitter->m_hWnd);
    if(hWnd==NULL)
    {
      ATLTRACE("0x%X\n", GetLastError());
    }
    //pInsView->OnInitialUpdate();
  } else if(m_bAnalyzerView) {
    CBZAnalyzerView *pAnaView = new CBZAnalyzerView;
    pCoreData->AddSubView(pAnaView);
    pAnaView->Create(m_pWndSplitter->m_hWnd);
    //pAnaView->OnInitialUpdate();
  } else {
    CBZFormView *pFormView = new CBZFormView;
    pCoreData->AddSubView(pFormView);
    pFormView->Create(m_pWndSplitter->m_hWnd);
    //pFormView->OnInitialUpdate();
  }
}
void CMainFrame::AddBZView()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZView *pBZView = new CBZView;
  pBZView->Create(m_pWndSplitter==NULL ? m_hWnd : m_pWndSplitter->m_hWnd);
  pCoreData->AddBZView(pBZView);
}

void CMainFrame::ReSplit(BOOL bSubView)
{
/*  if(!m_nSplitView0 &&  m_nSplitView)
  {
    if(bSubView)
    {
      m_pWndSplitter->m_paneW
    }
  } else if( m_nSplitView0 && !m_nSplitView)
  {*/
    //close doc2
   // pCoreData->DeleteView(0, TRUE/*bDelDoc*/);
   /* ReSplit(bSubView);
  }
*/
}

BOOL CMainFrame::CreateSubView()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  pCoreData->DeleteAllSubViews();
  AddSubView();
  if(m_nSplitView)AddSubView();
  return TRUE;
}

BOOL CMainFrame::PreparaSplitter()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);

  {
    int x = (bSubView ? 2 : 1) * (m_nSplitView==ID_VIEW_SPLIT_V ? 2 : 1);
    int y = (m_nSplitView==ID_VIEW_SPLIT_H ? 2 : 1);
    //if(x!=1 || y!=1)
    {
      if(m_pWndSplitter==NULL)
      {
        m_pWndSplitter = new CTamaSplitterWindow;
        m_pWndSplitter->CreateStatic(m_hWnd, x, y);
        pCoreData->SetSplitterWnd(m_pWndSplitter);
      } else {
      }
    }
  }
  return TRUE;
}


BOOL CMainFrame::ResetSplitter()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);

  ATLTRACE("ResetSplitter(): Split=%s, Bmp=%d, Strct=%d, Insp=%d, Ana=%d\n", m_nSplitView==0?"-":(m_nSplitView==ID_VIEW_SPLIT_H ? "縦分割":"横分割"), m_bBmpView, m_bStructView, m_bInspectView, m_bAnalyzerView);

  int paneCnt = 0;
  {
    int paneW = 1;
    int paneH = 1;
    if(bSubView)paneW+=1;
    if(m_nSplitView==ID_VIEW_SPLIT_V)paneW*=2;
    else if(m_nSplitView==ID_VIEW_SPLIT_H)paneH=2;
    ATLTRACE("RecalcPaneSize: w=%d, h=%d\n", paneW, paneH);
    m_pWndSplitter->m_paneW = paneW;
    m_pWndSplitter->m_paneH = paneH;
    paneCnt = paneW*paneH;
  }

  int rep = !m_nSplitView ? 1 : 2;
  //for(int i=0; i<rep; i++)
  //{
  //  AddBZView();
  //  if(bSubView)AddSubView();
    //pBZView->OnInitialUpdate();
  //}

  ATLASSERT(m_pWndSplitter);
  if(m_pWndSplitter!=NULL)
  {
    int iBZX = 0, iBZY = 0;
    int iBZShiftX = 0;
    if(m_nSplitView==ID_VIEW_SPLIT_V)
    {
      if(bSubView)iBZShiftX = 2;
      else iBZShiftX = 1;
    }
    int iBZShiftY = m_nSplitView==ID_VIEW_SPLIT_H ? 1 : 0;
    for(int i=0; i<rep; i++)
    {
      if(bSubView)
      {
        ATL::CWindow *pSubView = pCoreData->GetSubView(i);
        m_pWndSplitter->SetSplitterPane(pSubView->m_hWnd, iBZX++, iBZY);
      }
      CBZView *pBZView = pCoreData->GetBZView(i);
      m_pWndSplitter->SetSplitterPane(pBZView->m_hWnd, iBZX, iBZY);
      iBZX  = iBZShiftX;
      iBZY += iBZShiftY;
    }

    {
    if(bSubView && m_nSplitView==ID_VIEW_SPLIT_V)
    {
      m_pWndSplitter->SetSplitterPosX(1, 200);
      m_pWndSplitter->SetSplitterPosX(2, 820);
      m_pWndSplitter->SetSplitterPosX(3, 1020);
    } else {
      if(bSubView)m_pWndSplitter->SetSplitterPosX(1, 200);
      else if(m_nSplitView==ID_VIEW_SPLIT_V)m_pWndSplitter->SetSplitterPosX(1, 620);
      if(m_nSplitView==ID_VIEW_SPLIT_H)m_pWndSplitter->SetSplitterPosY(1, 200);
    }
    }
    m_pWndSplitter->InitSplitLayout();
  }

  ::SetFocus(pCoreData->GetBZView(0)->m_hWnd);//SetActiveWindow
  pCoreData->SetActive(0);

  pCoreData->GetBZView(0)->Update();
  if(m_pWndSplitter!=NULL)
  {
    m_hWndClient = m_pWndSplitter->m_hWnd;
  } else {
    m_hWndClient = pCoreData->GetBZView(0)->m_hWnd;
  }

  if(bSubView)paneCnt/=2;
  ATLASSERT(pCoreData->m_arrView.GetSize()==paneCnt);
  if(bSubView)ATLASSERT(pCoreData->m_arrSubView.GetSize()==paneCnt);
  ATLASSERT(pCoreData->m_arrDoc.GetSize()==paneCnt);
  ATLTRACE("  m_arrDoc[0]: %08X\n", pCoreData->m_arrDoc[0]);
  if(paneCnt==2)ATLTRACE("  m_arrDoc[1]: %08X\n", pCoreData->m_arrDoc[1]);
  return TRUE;
}

BOOL CMainFrame::CreateClient()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();

	BOOL bFirst/*bReCreate*/ = pCoreData->IsFirst();
  //CBZDoc2 *pDoc1 = NULL, *pDoc2 = NULL;

	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);

  PreparaSplitter();

  if(bFirst)
  {
    pCoreData->AddBZDoc2(new CBZDoc2);

    AddBZView();
    if(bSubView)AddSubView();
  } else if(!m_nSplitView0 &&  m_nSplitView)
  {
    //dup doc
    CBZDoc2 *doc2 = new CBZDoc2;
    CBZDoc2 *doc1 = pCoreData->GetBZDoc2(0);
    if(doc1!=NULL)doc1->DuplicateDoc(doc2);
    pCoreData->AddBZDoc2(doc2);

    AddBZView();
    if(bSubView)
    {
      AddSubView();
    }
    //ReSplit(bSubView);
    pCoreData->GetBZView(1)->Update();
  } else if( m_nSplitView0 && !m_nSplitView)
  {
    //close doc2
    pCoreData->DeleteView(1, TRUE/*bDelDoc*/);
    //ReSplit(bSubView);
  }


	if(!bFirst)//bReCreate)
  {
		// Backup (Caret, Scroll...)
    //unsigned int iBZView = pCoreData->GetCountBZView();
    //for(unsigned int i=0; i<iBZView; i++)
    //  pCoreData->GetBZView(i)->ReCreateBackup();

		//DestroyAllChildWnd(FALSE/*bDelDoc*/);
	}

  ResetSplitter();

	/*if(bReCreate) {
			RecalcLayout();
		if(bzViewNew[0])bzViewNew[0]->ReCreateRestore();
		if(bzViewNew[1])bzViewNew[1]->ReCreateRestore();
	}*/

  UpdateFrameTitle();
	return TRUE;
}



void CMainFrame::ChangeView(CBZView* pView) 
{
	if(m_nSplitView)
  {
    CBZView *pBroBZView = GetBrotherView(pView);
    ATLASSERT(pBroBZView);
		SetActiveView(pBroBZView);
    pBroBZView->UpdateStatusBar();
  }
}

CBZView *CMainFrame::GetBrotherView(CBZView* pView)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
	if(pCoreData->GetCountBZView() < 2) return NULL;
  CBZView *pView0 = pCoreData->GetBZView(0);
  if(pView0 != pView)return pView0;
  else return pCoreData->GetBZView(1);
}


int CMainFrame::GetSubViewIdealWidth(DWORD idx)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  ATL::CWindow *pSub0 = pCoreData->GetSubView(idx);
  if(pSub0==NULL)
  {
    ATLASSERT(FALSE);
    return 0;
  }
  if(m_bBmpView)
  {
    CBZBmpView2 *pBmpView = (CBZBmpView2*)pSub0;
    return pBmpView->GetWindowIdealWidth();
  } else if(m_bStructView) {
    CBZFormView *pFormView = (CBZFormView*)pSub0;
    return pFormView->GetWindowIdealWidth();
  } else if(m_bInspectView) {
    CBZInspectView *pInsView = (CBZInspectView*)pSub0;
    return pInsView->GetWindowIdealWidth();
  } else if(m_bAnalyzerView) {
    CBZAnalyzerView *pAnaView = (CBZAnalyzerView*)pSub0;
    return pAnaView->GetWindowIdealWidth();
  }
  return 0;
}


void CMainFrame::ResetWindowWidth()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CTamaSplitterWindow *pSplitter = pCoreData->GetSplitterWnd();
  if(pSplitter==NULL)return;
  DWORD dwViewCnt = pCoreData->GetCountBZView();

  int wEdge = GetSystemMetrics(SM_CXSIZEFRAME);

  CSimpleArray<int> m_arrBZWidth;
  for(DWORD i=0;i<dwViewCnt;i++)
  {
    CBZView *pBZView = pCoreData->GetBZView(i);
    if(pBZView==NULL)return;
    m_arrBZWidth.Add(pBZView->GetWindowIdealWidth());
  }
  
  CSimpleArray<int> m_arrSubWidth;
  BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);
  if(bSubView)
  {
    for(DWORD i=0;i<dwViewCnt;i++)
    {
      m_arrSubWidth.Add(GetSubViewIdealWidth(i));
    }
  }

  int newSize = 0;
  int subMax = 0;
  if(m_nSplitView==ID_VIEW_SPLIT_V)
  {
    for(int i=0;i<m_arrBZWidth.GetSize();i++)newSize += m_arrBZWidth[i];
    for(int i=0;i<m_arrSubWidth.GetSize();i++)newSize += m_arrSubWidth[i];
  } else {
    int bzMax = 0;
    /*int*/ subMax= 0;
    int t;
    for(int i=0;i<m_arrBZWidth.GetSize();i++)
    {
      t = m_arrBZWidth[i];
      if(t > bzMax)bzMax = t;
    }
    for(int i=0;i<m_arrSubWidth.GetSize();i++)
    {
      t = m_arrSubWidth[i];
      if(t > subMax)subMax = t;
    }
    newSize = bzMax + subMax;
  }
  WTL::CRect rFrame;
	GetWindowRect(rFrame);
  WTL::CRect rScreen;
  SystemParametersInfo(SPI_GETWORKAREA, 0, (LPRECT)rScreen, 0);
  rFrame.x2 = rFrame.x1 + newSize + wEdge*(dwViewCnt+1) + GetSystemMetrics(SM_CXVSCROLL)*dwViewCnt + (bSubView?wEdge*dwViewCnt:0);
  if(rFrame.Width() > rScreen.Width())rFrame.x2 = rFrame.x1 + rScreen.Width();
  MoveWindow(rFrame);

  //各ビューのサイズ調整配分
  if(bSubView || m_nSplitView)
  {
    if(bSubView && !m_nSplitView)
    {
      pSplitter->SetSplitterPosX(1, m_arrSubWidth[0]);
    } else if(!bSubView && m_nSplitView==ID_VIEW_SPLIT_V) {
      pSplitter->SetSplitterPosX(1, m_arrBZWidth[0]);
    } else if(bSubView && m_nSplitView) {
      switch(m_nSplitView)
      {
      case ID_VIEW_SPLIT_V:
        {
          int x = m_arrSubWidth[0];
          pSplitter->SetSplitterPosX(1, x);
          x += m_arrBZWidth[0];
          pSplitter->SetSplitterPosX(2, x);
          x += m_arrSubWidth[1];
          pSplitter->SetSplitterPosX(3, x);
          break;
        }
      case ID_VIEW_SPLIT_H:
        pSplitter->SetSplitterPosX(1, subMax);
        break;
      }
    }
    pSplitter->InitSplitLayout();
  }
}

//LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
//{
//	if(wParam == AFX_IDS_IDLEMESSAGE) {
//		m_bDisableStatusInfo = FALSE;
//		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_INFO, SBPS_NORMAL, m_nPaneWidth);
//	} else if(wParam) {
//		m_bDisableStatusInfo = TRUE;
//		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_INFO, SBPS_DISABLED | SBT_NOBORDERS, 0);
//	}
//	return CFrameWnd::OnSetMessageString(wParam, lParam);
//}





