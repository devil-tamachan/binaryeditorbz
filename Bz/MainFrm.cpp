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

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_INFO,
	ID_INDICATOR_SIZE,
	ID_INDICATOR_CHAR,
	ID_INDICATOR_INS,
};


void CMainFrame::OnUpdateViewBitmap() 
{
  UISetCheck(ID_VIEW_BITMAP, m_bBmpView);
  CBZDoc2* pDoc = GetActiveBZDoc2();
  UIEnable(ID_VIEW_BITMAP, pDoc->GetDocSize() >= (DWORD)(options.nBmpWidth * (options.nBmpColorWidth/8)) /* && !m_nSplitView*/);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if(bShow && m_nSplitView != ID_VIEW_SPLIT_V) {
    CBZView *pBZView = GetActiveBZView();
    if(pBZView)
      pBZView->ResizeFrame();
  }
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
void CMainFrame::OnFileSave(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
  if(pDoc)pDoc->OnFileSave();
}
void CMainFrame::OnFileSaveAs(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  CBZDoc2 *pDoc = pCoreData->GetActiveBZDoc2();
  if(pDoc)pDoc->OnFileSaveAs();
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
        _T("BZ"), 0, 0, (dlg.PrintToFile() != FALSE));
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

  if(dlg.DoModal() == IDOK)
  {
    m_devmodeCur.CopyFromHDEVMODE(dlg.m_psd.hDevMode);

    m_printerCur.ClosePrinter();
    m_printerCur.OpenPrinter(dlg.m_psd.hDevNames, m_devmodeCur);
  }
  ::GlobalFree(dlg.m_psd.hDevMode);
  ::GlobalFree(dlg.m_psd.hDevNames);
}


void CMainFrame::OnInitMenuPopup(WTL::CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
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
void CMainFrame::UpdateFrameTitle(BOOL bAddToTitle)
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

BOOL CMainFrame::CreateClient()
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();

	BOOL bReCreate = pCoreData->IsFirst();
  //CBZDoc2 *pDoc1 = NULL, *pDoc2 = NULL;

  if(pCoreData->IsFirst())
  {
    pCoreData->AddBZDoc2(new CBZDoc2);
  } else if(!m_nSplitView0 &&  m_nSplitView)
  {
    //dup doc
    CBZDoc2 *doc2 = new CBZDoc2;
    pCoreData->GetBZDoc2(0)->DuplicateDoc(doc2);
  } else if( m_nSplitView0 && !m_nSplitView)
  {
    //close doc2
    pCoreData->DeleteView(0, TRUE/*bDelDoc*/);
  }

	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);


	if(bReCreate) {
		// Backup (Caret, Scroll...)
    unsigned int iBZView = pCoreData->GetCountBZView();
    for(unsigned int i=0; i<iBZView; i++)
      pCoreData->GetBZView(i)->ReCreateBackup();

		DestroyAllChildWnd(FALSE/*bDelDoc*/);
	}


  {
    int x = (bSubView ? 2 : 1) * (m_nSplitView==ID_VIEW_SPLIT_V ? 2 : 1);
    int y = (m_nSplitView==ID_VIEW_SPLIT_H ? 2 : 1);
    if(x!=1 || y!=1)
    {
      if(m_pWndSplitter==NULL)m_pWndSplitter = new CTamaSplitterWindow;
      m_pWndSplitter->CreateStatic(m_hWnd, x, y);
      pCoreData->SetSplitterWnd(m_pWndSplitter);
    }
  }

  int rep = !m_nSplitView ? 1 : 2;
  for(int i=0; i<rep; i++)
  {
    CBZView *pBZView = new CBZView;
    pBZView->Create(m_pWndSplitter==NULL ? m_hWnd : m_pWndSplitter->m_hWnd);
    pCoreData->AddBZView(pBZView);
    if(bSubView)
    {
      if(m_bBmpView)
      {
        CBZBmpView2 *pBmpView = new CBZBmpView2;
        pBmpView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pBmpView);
        //pBmpView->OnInitialUpdate();
      } else if(m_bInspectView) {
        CBZInspectView *pInsView = new CBZInspectView;
        pInsView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pInsView);
        //pInsView->OnInitialUpdate();
      } else if(m_bAnalyzerView) {
        CBZAnalyzerView *pAnaView = new CBZAnalyzerView;
        pAnaView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pAnaView);
        //pAnaView->OnInitialUpdate();
      } else {
        CBZFormView *pFormView = new CBZFormView;
        pFormView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pFormView);
        //pFormView->OnInitialUpdate();
      }
    }
    //pBZView->OnInitialUpdate();
  }

  if(m_pWndSplitter!=NULL)
  {
    int iBZX = 0, iBZY = 0;
    int iBZShiftX = bSubView ? 1 : 0;
    int iBZShiftY = ID_VIEW_SPLIT_H ? 1 : 0;
    for(int i=0; i<rep; i++)
    {
      if(bSubView)
      {
        ATL::CWindow *pSubView = pCoreData->GetSubView(i);
        m_pWndSplitter->SetSplitterPane(pSubView->m_hWnd, iBZX++, iBZY);
      }
      CBZView *pBZView = pCoreData->GetBZView(i);
      m_pWndSplitter->SetSplitterPane(pBZView->m_hWnd, iBZX, iBZY);
      iBZX += iBZShiftX;
      iBZY += iBZShiftY;
    }

    {
    if(bSubView && m_nSplitView==ID_VIEW_SPLIT_V)
    {
      m_pWndSplitter->SetSplitterPosX(1, 200);
      m_pWndSplitter->SetSplitterPosX(2, 200);
      m_pWndSplitter->SetSplitterPosX(3, 200);
    } else {
      if(bSubView)m_pWndSplitter->SetSplitterPosX(1, 200);
      if(m_nSplitView==ID_VIEW_SPLIT_H)m_pWndSplitter->SetSplitterPosY(1, 200);
    }
    }
    m_pWndSplitter->InitSplitLayout();
  }

  ::SetActiveWindow(pCoreData->GetBZView(0)->m_hWnd);
  pCoreData->SetActive(0);

  pCoreData->GetBZView(0)->Update();
  if(m_pWndSplitter!=NULL)
  {
    m_hWndClient = m_pWndSplitter->m_hWnd;
  } else {
    m_hWndClient = pCoreData->GetBZView(0)->m_hWnd;
  }

	/*if(bReCreate) {
			RecalcLayout();
		if(bzViewNew[0])bzViewNew[0]->ReCreateRestore();
		if(bzViewNew[1])bzViewNew[1]->ReCreateRestore();
	}*/
	return TRUE;
}



void CMainFrame::ChangeView(CBZView* pView) 
{
	if(m_nSplitView) 
		SetActiveView(GetBrotherView(pView));
}

CBZView *CMainFrame::GetBrotherView(CBZView* pView)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
	if(pCoreData->GetCountBZView() < 2) return NULL;
  CBZView *pView0 = pCoreData->GetBZView(0);
  if(pView0 != pView)return pView0;
  else return pCoreData->GetBZView(1);
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





