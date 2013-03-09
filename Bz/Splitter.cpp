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

