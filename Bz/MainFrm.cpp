// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BZ.h"

#include "MainFrm.h"
#include "BZDoc.h"
#include "BZView.h"
#include "BZBmpVw.h"
#include "BZFormVw.h"
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

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_JUMP_FIND, OnJumpFind)
	ON_COMMAND(ID_VIEW_BITMAP, OnViewBitmap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BITMAP, OnUpdateViewBitmap)
	ON_COMMAND(ID_JUMP_TO, OnJumpTo)
	ON_COMMAND(ID_VIEW_STRUCT, OnViewStruct)
	ON_COMMAND(ID_EDIT_VALUE, OnEditValue)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRUCT, OnUpdateViewStruct)
	ON_COMMAND(ID_VIEW_FULLPATH, OnViewFullpath)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLPATH, OnUpdateViewFullpath)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_TOOLS_SETTING, OnToolsSetting)
	ON_COMMAND(ID_VIEW_SUBCURSOR, OnViewSubCursor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SUBCURSOR, OnUpdateViewSubCursor)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_VIEW_SPLIT_H, ID_VIEW_SPLIT_V, OnViewSplit)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SPLIT_H, ID_VIEW_SPLIT_V, OnUpdateViewSplit)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
END_MESSAGE_MAP()

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

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pSplitter = NULL;
	m_bBmpView = FALSE;
	m_bStructView = FALSE;
	m_nSplitView = m_nSplitView0 = 0;
	m_bDisableStatusInfo = FALSE;
}

CMainFrame::~CMainFrame()
{
	if(m_pSplitter)
		delete m_pSplitter;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	if(options.ptFrame.x && options.ptFrame.y) {	// ###1.63
		if(((CBZApp*)AfxGetApp())->m_bFirstInstance) {
			cs.x = options.ptFrame.x;
			cs.y = options.ptFrame.y;
		}
		cs.cx = 0;
		cs.cy = options.cyFrame;
	}

	if(CFrameWnd::PreCreateWindow(cs)) {
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		VERIFY(::GetClassInfoEx(AfxGetInstanceHandle(), cs.lpszClass, &wc));
		wc.lpszClassName = BZ_CLASSNAME;
		wc.hIcon  = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		wc.hIconSm = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);
		::RegisterClassEx(&wc);
		cs.lpszClass = BZ_CLASSNAME;
		return TRUE;
	}
	return FALSE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

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
	m_wndStatusBar.SetPaneText(1, "");

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	m_bStructView = options.bStructView;
	CreateClient(pContext);
	return TRUE; //CFrameWnd::OnCreateClient(lpcs, pContext);
}

BOOL CMainFrame::CreateClient(CCreateContext* pContext) 
{
	BOOL bReCreate = (pContext == NULL);
	CCreateContext context;
	ZeroMem(context);

	CDocument *pDoc;
	CDocument *pDoc2 = NULL;

	if(m_nSplitView && m_nSplitView0) {
		pDoc  = ((CView*)m_pSplitter->GetPane(0, 0))->GetDocument();
		pDoc2 = ((CView*)m_pSplitter->GetPane(m_pSplitter->GetRowCount() - 1, m_pSplitter->GetColumnCount() - 1))->GetDocument();
	} else
		pDoc = bReCreate ? GetActiveDocument() : pContext->m_pCurrentDoc;
	CBZDocTemplate* pDocTemp = (CBZDocTemplate*)pDoc->GetDocTemplate();

	if(bReCreate) {
		if(pDoc) pDoc->m_bAutoDelete = FALSE;
		if(pDoc2) pDoc2->m_bAutoDelete = FALSE;
		pContext = &context;
		if(m_pSplitter) {
			m_pSplitter->DestroyWindow();
			delete m_pSplitter;
			m_pSplitter = NULL;
		} else {
			CView *pView = GetActiveView();
			pView->DestroyWindow();
		}
		if(pDoc) pDoc->m_bAutoDelete = TRUE;
		if(pDoc2) pDoc2->m_bAutoDelete = TRUE;
		pContext->m_pCurrentDoc = pDoc;
	}

	CView* pActiveView = NULL;
	BOOL bSubView = (m_bBmpView || m_bStructView);

	if(m_nSplitView) {
		m_pSplitter = new CSplitterWnd;
		int nRow = (m_nSplitView == ID_VIEW_SPLIT_H);
		int nCol = (m_nSplitView == ID_VIEW_SPLIT_V);
		m_pSplitter->CreateStatic(this, nRow + 1, (nCol + 1) * (bSubView + 1));

		if(!pDoc2) {
			pDoc2 = new CBZDoc;
			pDocTemp->CDocTemplate::AddDocument(pDoc2);
			((CBZDoc*)pDoc)->DuplicateDoc((CBZDoc*)pDoc2);
			pDocTemp->SetDocument(pDoc2);
		}
		int r = 0;
		int c = 0;
		for_to(i, 2) {
			if(i) pContext->m_pCurrentDoc = pDoc2;
			if(bSubView) {
				m_pSplitter->CreateView(r, c, m_bBmpView ? RUNTIME_CLASS(CBZBmpView) : RUNTIME_CLASS(CBZFormView), CSize(0,0), pContext);
				m_pSplitter->CreateView(r, c + 1, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
				((CView*)m_pSplitter->GetPane(r, c))->OnInitialUpdate();
			} else
				m_pSplitter->CreateView(r, c, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
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
			m_pSplitter->CreateView(0, 0, m_bBmpView ? RUNTIME_CLASS(CBZBmpView) : RUNTIME_CLASS(CBZFormView), CSize(0,0), pContext);
			m_pSplitter->CreateView(0, 1, RUNTIME_CLASS(CBZView), CSize(0,0), pContext);
			((CView*)m_pSplitter->GetPane(0, 0))->OnInitialUpdate();
			pActiveView = (CView*)m_pSplitter->GetPane(0, 1);
		} else {
			pContext->m_pNewViewClass = RUNTIME_CLASS(CBZView);
			pActiveView = (CView*)CreateView(pContext);
		}
		pActiveView->OnInitialUpdate();
	}
	if(m_bStructView && m_nSplitView != ID_VIEW_SPLIT_V) {
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
		MoveWindow(rFrame);
	}

	if(pActiveView)  {
		SetActiveView(pActiveView);
	}
	if(bReCreate) {
		RecalcLayout();
	} else
		((CBZView*)pActiveView)->ResizeFrame();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnJumpFind() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndToolBar, TRUE, FALSE);
	m_wndToolBar.m_combo.SetFocus();
}

void CMainFrame::OnJumpTo() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndToolBar, TRUE, FALSE);
	m_wndToolBar.m_combo.SetFocus();
	m_wndToolBar.m_combo.SetText("> ");
}

void CMainFrame::OnEditValue() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndToolBar, TRUE, FALSE);
	m_wndToolBar.m_combo.SetFocus();
	m_wndToolBar.m_combo.SetText("< %");
}

void CMainFrame::OnViewBitmap() 
{
	// TODO: Add your command handler code here
	GetSplitInfo();
	m_bBmpView = !m_bBmpView;
	m_bStructView = FALSE;
	CreateClient();
}

void CMainFrame::OnViewStruct() 
{
	// TODO: Add your command handler code here
	GetSplitInfo();
	m_bBmpView = FALSE;
	m_bStructView = !m_bStructView;
	CreateClient();
}


void CMainFrame::OnViewSplit(UINT nID) 
{
	GetSplitInfo();
	m_nSplitView0 = m_nSplitView;
	m_nSplitView = (m_nSplitView == nID) ? 0 : nID;
	CreateClient();
	m_nSplitView0 = m_nSplitView;
}

/*
void CMainFrame::OnViewSplit(UINT nID) 
{
	// TODO: Add your command handler code here
	CSplitter *pnewSplit = new CSplitter;
	CBZDoc* pDoc = (CBZDoc*)GetActiveDocument();
	CBZDocTemplate* pTemp = (CBZDocTemplate*)pDoc->GetDocTemplate();

	if(m_nSplitView == nID) {
		pnewSplit->ReplaceClient(this, 1, 1);
		pnewSplit->ReplaceView(0, 0, RUNTIME_CLASS(CBZView));
		nID = 0;
	} else {
		int nRow = (nID == ID_VIEW_SPLIT_H);
		int nCol = (nID == ID_VIEW_SPLIT_V);
		CDocument* pDoc0 = NULL;
		CDocument* pDoc1 = NULL;
		if(m_nSplitView) {
			pDoc0 = m_pSplitter->GetView(0)->GetDocument();
			pDoc1 = m_pSplitter->GetView(1)->GetDocument();
		}
		RECT rClient;
		GetActiveView()->GetClientRect(&rClient);
		pnewSplit->ReplaceClient(this, nRow+1, nCol+1);
		if(pDoc0)
			pnewSplit->m_pDoc = pDoc0;
		pnewSplit->ReplaceView(0, 0, RUNTIME_CLASS(CBZView));
		if(pDoc1)
				pnewSplit->m_pDoc = pDoc1;
		else {		
			pnewSplit->m_pDoc = new CBZDoc;
			pTemp->CDocTemplate::AddDocument(pnewSplit->m_pDoc);
			pDoc->DuplicateDoc((CBZDoc*)pnewSplit->m_pDoc);
		}
		pnewSplit->ReplaceView(nRow, nCol, RUNTIME_CLASS(CBZView));
		if(nID == ID_VIEW_SPLIT_H)
			pnewSplit->SetRowInfo(0, rClient.y2/2, 0);
		else
			pnewSplit->SetColumnInfo(0, rClient.x2/2, 0);
	}
	if(m_pSplitter)
		m_pSplitter->DestroyWindow();
	delete m_pSplitter;
	if(m_nSplitView)
		pTemp->SetDocument(pnewSplit->m_pDoc);
	m_pSplitter = pnewSplit;
	m_nSplitView = nID;
	OnUpdateFrameTitle();
	RecalcLayout();
}
*/

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

void CMainFrame::OnUpdateViewBitmap(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bBmpView);
	CBZDoc* pDoc = (CBZDoc*)(GetActiveView()->GetDocument());
	pCmdUI->Enable(pDoc->GetDocSize() >= (DWORD)options.nBmpWidth /* && !m_nSplitView*/);
}

void CMainFrame::OnUpdateViewStruct(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bStructView);
//	pCmdUI->Enable(!m_nSplitView);
}

void CMainFrame::OnUpdateViewSplit(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_nSplitView == pCmdUI->m_nID);
//	pCmdUI->Enable(!m_bBmpView && !m_bStructView);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if(bShow && m_nSplitView != ID_VIEW_SPLIT_V) {
		CView *pView = GetActiveView();
		if(pView->GetRuntimeClass() == RUNTIME_CLASS(CBZView))
			((CBZView*)GetActiveView())->ResizeFrame();
	}
	CFrameWnd::OnShowWindow(bShow, nStatus);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
//	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);

	CBZDoc* pDoc = (CBZDoc*)GetActiveDocument();
	if(pDoc) {
		CString s(AfxGetAppName());
		s += " - ";
		CString sPath = pDoc->GetPathName();
		if(!(options.barState & BARSTATE_FULLPATH) || sPath.IsEmpty())
			sPath = pDoc->GetTitle();
		s += sPath;
		if(pDoc->IsModified())
			s += " *";
		SetWindowText(s);
	}
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
//	TRACE("Msg:%X\n", wParam);
	if(wParam == AFX_IDS_IDLEMESSAGE) {
		m_bDisableStatusInfo = FALSE;
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_INFO, SBPS_NORMAL, m_nPaneWidth);
	} else if(wParam) {
		m_bDisableStatusInfo = TRUE;
		m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_INFO, SBPS_DISABLED | SBT_NOBORDERS, 0);
	}
	return CFrameWnd::OnSetMessageString(wParam, lParam);
}

void CMainFrame::OnViewFullpath() 
{
	// TODO: Add your command handler code here
	options.barState ^= BARSTATE_FULLPATH;
	OnUpdateFrameTitle();
}

void CMainFrame::OnUpdateViewFullpath(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(options.barState & BARSTATE_FULLPATH);
}

void CMainFrame::OnViewSubCursor()	// ### 1.62
{
	// TODO: Add your command handler code here
	options.bSubCursor = !options.bSubCursor;
	GetActiveView()->Invalidate();
}

void CMainFrame::OnUpdateViewSubCursor(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(options.bSubCursor);
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	GetFrameState();
	GetSplitInfo();
	CFrameWnd::OnClose();
}

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
		if(m_bBmpView || m_bStructView) {
			m_pSplitter->GetColumnInfo(1, nCur, nMin);
			options.xSplit += nCur;
		}
		break;
	}
	if(options.bStructView = m_bStructView) {
		m_pSplitter->GetColumnInfo(0, nCur, nMin);
		options.xSplitStruct = nCur;
	}
}

void CMainFrame::OnToolsSetting() 
{
	// TODO: Add your command handler code here
	BOOL bDWordAddr = options.bDWordAddr;
	CSettingDlg().DoModal();
	if(bDWordAddr != options.bDWordAddr) {
	}
}


