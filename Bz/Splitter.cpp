// splitter.cpp : implementation file
//

#include "stdafx.h"

#include "splitter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplitter

IMPLEMENT_DYNCREATE(CSplitter, CSplitterWnd)

BEGIN_MESSAGE_MAP(CSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplitter construction/destruction


CSplitter::CSplitter()
{ 
	m_bSplit = FALSE;   
}

CSplitter::~CSplitter()
{                 
}

BOOL CSplitter::ReplaceClient(CWnd* pParentWnd, int nRows, int nCols, DWORD dwStyle, UINT nID)
{
	CFrameWnd* pFrame = ((CFrameWnd*)AfxGetMainWnd());
	CView *pView = pFrame->GetActiveView();
//	CWnd* pParent = pView->GetParent();
	m_pDoc = pView->GetDocument();
	m_pDoc->m_bAutoDelete=FALSE; 
//	if(pParent == pFrame)
	 	pView->DestroyWindow();
//	else		
//		while(pView = (CView*)pParent->GetWindow(GW_CHILD))
//		 	pView->DestroyWindow();
	m_pDoc->m_bAutoDelete=TRUE;
	m_bSplit=( nRows>1 || nCols>1);
	if(m_bSplit) {
		CreateStatic(pParentWnd, nRows, nCols, dwStyle, nID);
	}
	return TRUE;
}

BOOL CSplitter::ReplaceView(int row, int col, CRuntimeClass *pViewClass)
{
	CCreateContext context;
	CView *pNewView;

//  BOOL bSetActive;

//	CDocument *m_pDoc = ((CFrameWnd*)AfxGetMainWnd())->GetActiveDocument();
   
//  if ((GetPane(row,col)->IsKindOf(pViewClass))==TRUE)
//       return FALSE;
				    
   // Get pointer to CDocument object so that it can be used in the creation 
   // process of the new view
/*   CView * pActiveView=GetParentFrame()->GetActiveView();
   if (pActiveView==NULL || pActiveView==GetPane(row,col))
      bSetActive=TRUE;
   else
      bSetActive=FALSE;

    // set flag so that document will not be deleted when view is destroyed
	m_pDoc->m_bAutoDelete=FALSE;    
    // Delete existing view 
   ((CView *) GetPane(row,col))->DestroyWindow();
    // set flag back to default 
    m_pDoc->m_bAutoDelete=TRUE;
 
    // Create new view                      
*/ 
	context.m_pNewViewClass=pViewClass;
	context.m_pCurrentDoc=m_pDoc;
	context.m_pNewDocTemplate=NULL;
	context.m_pLastView=NULL;
	context.m_pCurrentFrame=NULL;
	CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
	if(m_bSplit) {
		CreateView(row, col, pViewClass, CSize(0,0), &context);
		pNewView = (CView *)GetPane(row,col);
	} else {
		pNewView = (CView*)(pFrame->CreateView(&context));
	}
	pNewView->ShowWindow(SW_SHOW);
	pNewView->OnInitialUpdate();
 	pFrame->SetActiveView(pNewView);
   
// RecalcLayout(); 
//   GetPane(row,col)->SendMessage(WM_PAINT);
   
   return TRUE;
}
    
CView* CSplitter::GetView(int n)
{
	int row = 0;
	int col = 0;
	if(n) {
		row = m_nRows-1;
		col = m_nCols-1;
	}	
	return (CView*)GetPane(row, col);
}


/////////////////////////////////////////////////////////////////////////////
// CSplitter message handlers

