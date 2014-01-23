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
  CString sFileName = GetActiveBZDoc2()->GetPathName();
  sFileName += _T(".lst");

  if(m_pDocManager->DoPromptFileName(sFileName, IDS_SAVEDUMP_CAPTION, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, FALSE, NULL)) {
    CFile file;
    if(file.Open(sFileName, CFile::modeCreate | CFile::modeWrite)) {
      GetActiveBZView()->DrawToFile(&file);
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
  if(m_bInspectView && m_nSplitView && m_nSplitView0) {
    ((CBZInspectView*)m_pSplitter->GetPane(0, 0))->_UpdateChecks();
    ((CBZInspectView*)m_pSplitter->GetPane(0, 0))->Update();
    int r=0,c=0;
    if(options.nSplitView==ID_VIEW_SPLIT_H)c=2;
    else r=1;
    ((CBZInspectView*)m_pSplitter->GetPane(r, c))->_UpdateChecks();
    ((CBZInspectView*)m_pSplitter->GetPane(r, c))->Update();
  }
}
void CMainFrame::UpdateFrameTitle(BOOL bAddToTitle = TRUE)
{
  CBZDoc2* pDoc = GetActiveBZDoc2();
  if(pDoc) {
    CString s(AfxGetAppName());
    s += " - ";
    CString sPath = pDoc->GetPathName();
    if(!(options.barState & BARSTATE_FULLPATH) || sPath.IsEmpty())
      sPath = pDoc->GetTitle();
    s += sPath;
    //		s += pDoc->IsFileMapping()?_T(" (FileMap)"):_T(" (Mem)");
    if(pDoc->IsModified())
      s += " *";
    SetWindowText(s);
  }
}


//BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)   ‚½‚Ô‚ñíœ‚µ‚Ä‚à–â‘è‚È‚¢Bˆê‰žŽc‚µ‚Ä‚¨‚­
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
  CBZDoc2 *pDoc1 = NULL, *pDoc2 = NULL;

  if(pCoreData->IsFirst())
  {
    pCoreData->AddBZDoc2(new CBZDoc2);
  } else if(!m_nSplitView0 &&  m_nSplitView)
  {
    //dup doc
    CBZDoc2 doc2 = new CBZDoc2;
    pDocTemp->CDocTemplate::AddDocument(pDoc2);
    pCoreData->GetBZDoc2(0)->DuplicateDoc(pDoc2);
  } else if( m_nSplitView0 && !m_nSplitView)
  {
    //close doc2
  }

	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);


	if(bReCreate) {
		// Backup (Caret, Scroll...)
    unsigned int iBZView = pCoreData->GetCountBZView();
    for(unsined int i=0; i<iBZView; i++)
      pCoreData->GetBZView(i)->ReCreateBackup();

		DestroyAllChildWnd(FALSE/*bDelDoc*/);
	}

  int x = m_nSplitView==ID_VIEW_SPLIT_H || bSubView ? 2 : 1;
  int y = m_nSplitView==ID_VIEW_SPLIT_V || bSubView ? 2 : 1;
  if(x!=1 || y!=1)
  {
    m_pWndSplitter->CreateStatic(m_hWnd, x, y);
    pCoreData->SetSplitterWnd(m_pWndSplitter);
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
        pBmpView->OnInitialUpdate();
      } else if(m_bInspectView) {
        CBZInspectView *pInsView = new CBZInspectView;
        pInsView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pInsView);
        pInsView->OnInitialUpdate();
      } else if(m_bAnalyzerView) {
        CBZAnalyzerView *pAnaView = new CBZAnalyzerView;
        pAnaView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pAnaView);
        pAnaView->OnInitialUpdate();
      } else {
        CBZFormView *pFormView = new CBZFormView;
        pFormView->Create(m_pWndSplitter->m_hWnd);
        pCoreData->AddSubView(pFormView);
        pFormView->OnInitialUpdate();
      }
    }
    pBZView->OnInitialUpdate();
  }

  if(m_pWndSplitter!=NULL)
  {
    int stepSubX = 0, stepSubY = 0, stepSetX = 0, stepSetY = 0;
    switch(m_nSplitView)
    {
    case ID_VIEW_SPLIT_H:
      stepSubY++;
      stepSetX++;
      break;
    case ID_VIEW_SPLIT_V:
      stepSubX++;
      stepSetY++;
      break;
    }
    for(int i=0; i<rep; i++)
    {
      CBZView *pBZView = pCoreData->GetBZView(i);
      m_pWndSplitter->SetSplitterPane(pBZView->m_hWnd, stepSetX, stepSetY);
      if(bSubView)
      {
        ATL::CWindow *pSubView = pCoreData->GetSubView(i);
        m_pWndSplitter->SetSplitterPane(pSubView->m_hWnd, stepSetX+stepSubX, stepSetY+stepSubY);
      }
    }
    m_pWndSplitter->InitSplitLayout();
  }

  ::SetActiveWindow(pCoreData->GetBZView(0)->m_hWnd);
  pCoreData->SetActive(0);

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
	if(!m_nSplitView) return NULL;
	int nRow = m_pSplitter->GetRowCount();
	int nCol = m_pSplitter->GetColumnCount();
	CView* pView0 = (CView*)m_pSplitter->GetPane(0, nRow * nCol == 4);
	CView* pView1 = (CView*)m_pSplitter->GetPane(nRow > 1, nCol - 1);
	if(pView == pView0)
		pView = pView1;
	else
		pView = pView0;
	return pView;
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





