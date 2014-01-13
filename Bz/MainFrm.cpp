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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction



BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(CFrameWnd::PreCreateWindow(cs)) {
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		VERIFY(::GetClassInfoEx(AfxGetInstanceHandle(), cs.lpszClass, &wc));
		wc.lpszClassName = _T(BZ_CLASSNAME);
		wc.hIcon  = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		wc.hIconSm = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);
		::RegisterClassEx(&wc);
		cs.lpszClass = _T(BZ_CLASSNAME);
		return TRUE;
	}
	return FALSE;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	m_bStructView = options.bStructView;
	m_bInspectView = options.bInspectView;
	m_bAnalyzerView = options.bAnalyzerView;
	CreateClient(pContext);
	return TRUE; //CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::DeleteSplitterWnd(CCreateContext* pContext)
{
	m_pSplitter->DeleteView(0,0);
	m_pSplitter->CreateView(0,0,RUNTIME_CLASS(CWnd), CSize(0,0), pContext);
	m_pSplitter->SetActivePane(0,0);

	int maxRow = m_pSplitter->GetRowCount();
	int maxCol = m_pSplitter->GetColumnCount();
	for(int r=0; r<maxRow; r++)
	{
		for(int c=0; c<maxCol; c++)
		{
			if(r!=0 && c!=0)
			{
				m_pSplitter->DeleteView(r,c);
			}
		}
	}

	m_pSplitter->DestroyWindow();
	delete m_pSplitter;
	m_pSplitter = NULL;
}

BOOL CMainFrame::CreateClient(CCreateContext* pContext) 
{
	BOOL bReCreate = (pContext == NULL);
	CCreateContext context;
	ZeroMem(context);

	CDocument *pDoc;
	CDocument *pDoc2 = NULL;

	BOOL bSubView = (m_bBmpView || m_bStructView || m_bInspectView || m_bAnalyzerView);

	if(m_nSplitView && m_nSplitView0) {
		pDoc  = ((CView*)m_pSplitter->GetPane(0, 0))->GetDocument();
		pDoc2 = ((CView*)m_pSplitter->GetPane(m_pSplitter->GetRowCount() - 1, m_pSplitter->GetColumnCount() - 1))->GetDocument();
	} else
		pDoc = bReCreate ? GetActiveDocument() : pContext->m_pCurrentDoc;
	CBZDocTemplate* pDocTemp = (CBZDocTemplate*)pDoc->GetDocTemplate();

	if(bReCreate) {
		// Backup (Caret, Scroll...)
		CBZView *bzView;
		if(m_pSplitter) {
			int maxRow = m_pSplitter->GetRowCount();
			int maxCol = m_pSplitter->GetColumnCount();
			for(int r=0; r<maxRow; r++)
			{
				for(int c=0; c<maxCol; c++)
				{
					bzView = dynamic_cast<CBZView *>(m_pSplitter->GetPane(r,c));
					if(bzView) bzView->ReCreateBackup();
				}
			}
		} else {
			bzView = dynamic_cast<CBZView *>(GetActiveView());
			if(bzView) bzView->ReCreateBackup();
		}

		if(pDoc) pDoc->m_bAutoDelete = FALSE;
		if(pDoc2) pDoc2->m_bAutoDelete = FALSE;
		pContext = &context;
		if(m_pSplitter) {
			DeleteSplitterWnd(pContext);
		} else {
			CView *pView = GetActiveView();
			pView->DestroyWindow();
		}
		if(pDoc) pDoc->m_bAutoDelete = TRUE;
		if(pDoc2) pDoc2->m_bAutoDelete = TRUE;
		pContext->m_pCurrentDoc = pDoc;
	}

	CView* pActiveView = NULL;
	CBZView *bzViewNew[2] = {0};

	if(m_nSplitView) {
		m_pSplitter = new CSplitterWnd;
		int nRow = (m_nSplitView == ID_VIEW_SPLIT_H);
		int nCol = (m_nSplitView == ID_VIEW_SPLIT_V);
		m_pSplitter->CreateStatic(this, nRow + 1, (nCol + 1) * (bSubView + 1));

		if(!pDoc2) {
			pDoc2 = new CBZDoc2;
			pDocTemp->CDocTemplate::AddDocument(pDoc2);
			((CBZDoc2*)pDoc)->DuplicateDoc((CBZDoc2*)pDoc2);
			pDocTemp->SetDocument(pDoc2);
		}
		int r = 0;
		int c = 0;
		for_to(i, 2) {
			if(i) pContext->m_pCurrentDoc = pDoc2;
			if(bSubView) {
				if(m_bBmpView)
					m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZBmpView), CSize(0,0), pContext);
				else if(m_bInspectView)
					m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZInspectView), CSize(0,0), pContext);
				else if(m_bAnalyzerView)
					m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZAnalyzerView), CSize(0,0), pContext);
				else
					m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZFormView), CSize(0,0), pContext);
				m_pSplitter->CreateView(r, c + 1, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
				bzViewNew[i] = dynamic_cast<CBZView *>(m_pSplitter->GetPane(r, c + 1));
				((CView*)m_pSplitter->GetPane(r, c))->OnInitialUpdate();
			} else {
				m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
				bzViewNew[i] = dynamic_cast<CBZView *>(m_pSplitter->GetPane(r, c));
			}
			pActiveView = (CView*)m_pSplitter->GetPane(r, c + bSubView);
			pActiveView->OnInitialUpdate();
			if(nCol) 
				((CBZView*)pActiveView)->m_bResize = TRUE;
			r += nRow;
			c += nCol * (bSubView + 1);
		}
		CRect rClient;
		GetClientRect(rClient);
		if(m_nSplitView == ID_VIEW_SPLIT_H)
			m_pSplitter->SetRowInfo(0, options.ySplit ? options.ySplit : rClient.y2/2, 0);
		else {
			int xSplit = options.xSplit;
			if(xSplit == 0) xSplit = rClient.x2/2;
			if(!bSubView)
				m_pSplitter->SetColumnInfo(0, xSplit, 0);
			else {
				m_pSplitter->SetColumnInfo(0, options.xSplitStruct, 0);
				m_pSplitter->SetColumnInfo(1, xSplit - options.xSplitStruct, 0);
				m_pSplitter->SetColumnInfo(2, options.xSplitStruct, 0);
				m_pSplitter->SetColumnInfo(3, xSplit - options.xSplitStruct, 0);
			}
		}
	} else {
		if(bSubView) {
			m_pSplitter = new CSplitterWnd;
			m_pSplitter->CreateStatic(this, 1, 2);
			if(m_bBmpView)
				m_pSplitter->CreateView(0, 0, RUNTIME_CLASS(CBZBmpView), CSize(0,0), pContext);
			else if(m_bInspectView)
				m_pSplitter->CreateView(0, 0, RUNTIME_CLASS(CBZInspectView), CSize(0,0), pContext);
			else if(m_bAnalyzerView)
				m_pSplitter->CreateView(0, 0, RUNTIME_CLASS(CBZAnalyzerView), CSize(0,0), pContext);
			else
				m_pSplitter->CreateView(0, 0, RUNTIME_CLASS(CBZFormView), CSize(0,0), pContext);
			m_pSplitter->CreateView(0, 1, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
			((CView*)m_pSplitter->GetPane(0, 0))->OnInitialUpdate();
			pActiveView = (CView*)m_pSplitter->GetPane(0, 1);
			bzViewNew[0] = dynamic_cast<CBZView *>(pActiveView);
		} else {
			pContext->m_pNewViewClass = RUNTIME_CLASS(CBZView);
			pActiveView = (CView*)CreateView(pContext);
			bzViewNew[0] = dynamic_cast<CBZView *>(pActiveView);
		}
		pActiveView->OnInitialUpdate();
	}
	if((m_bStructView||m_bInspectView||m_bAnalyzerView) && m_nSplitView != ID_VIEW_SPLIT_V) {
		m_pSplitter->SetColumnInfo(0, options.xSplitStruct, 0);
	}

	if(m_nSplitView != m_nSplitView0) {
		CRect rFrame;
		GetWindowRect(rFrame);
		switch(m_nSplitView) {
		case 0:
			if(options.cyFrame)
				rFrame.y2 = rFrame.y1 + options.cyFrame;
			break;
		case ID_VIEW_SPLIT_H:
			if(options.cyFrame2)
				rFrame.y2 = rFrame.y1 + options.cyFrame2;
			break;
		case ID_VIEW_SPLIT_V:
			if(options.cxFrame2) {
				rFrame.x2 = rFrame.x1 + options.cxFrame2;
				rFrame.y2 = rFrame.y1 + options.cyFrame;
			}
			break;
		}
//		if(!IsZoomed())
//		{
			MoveWindow(rFrame);
//		}
	}

	if(pActiveView)  {
		SetActiveView(pActiveView);
	}
	if(bReCreate) {
//		if(!IsZoomed())
			RecalcLayout();
		if(bzViewNew[0])bzViewNew[0]->ReCreateRestore();
		if(bzViewNew[1])bzViewNew[1]->ReCreateRestore();
	} //else ((CBZView*)pActiveView)->ResizeFrame();
	return TRUE;
}



void CMainFrame::ChangeView(CView* pView) 
{
	if(m_nSplitView) 
		SetActiveView(GetBrotherView(pView));
}

CView *CMainFrame::GetBrotherView(CView* pView)
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


void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
//	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);

	CBZDoc2* pDoc = (CBZDoc2*)GetActiveDocument();
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



void CMainFrame::GetFrameState()
{
	WINDOWPLACEMENT wndpl;
	GetWindowPlacement(&wndpl);
	options.ptFrame.x = wndpl.rcNormalPosition.left;
	options.ptFrame.y = wndpl.rcNormalPosition.top;
	options.nCmdShow = wndpl.showCmd;
	options.barState = m_wndToolBar.IsWindowVisible() * BARSTATE_TOOL | m_wndStatusBar.IsWindowVisible() * BARSTATE_STATUS
						| options.barState & (BARSTATE_FULLPATH | BARSTATE_NOFLAT);
}

void CMainFrame::GetSplitInfo()
{
	if(IsIconic() || IsZoomed()) return;
	CRect rFrame;
	GetWindowRect(rFrame);

	int nCur, nMin;
	switch(options.nSplitView = m_nSplitView) {
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
	}
	options.bStructView = m_bStructView;
	options.bInspectView = m_bInspectView;
	options.bAnalyzerView = m_bAnalyzerView;
	if(m_bStructView || m_bInspectView || m_bAnalyzerView) {
		m_pSplitter->GetColumnInfo(0, nCur, nMin);
		options.xSplitStruct = nCur;
	}
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