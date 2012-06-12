// splitter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSplitter frame with splitter

#ifndef CSPLITTER_H
#define CSPLITTER_H


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CSplitter : public CSplitterWnd
{
public: // create from serialization only
   	CSplitter();
	DECLARE_DYNCREATE(CSplitter)

// Attributes
private:
	BOOL m_bSplit;
public:
	CDocument* m_pDoc;
   	
// Operations
public:
	BOOL ReplaceClient(CWnd* pParentWnd, int nRows, int nCols,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST);
    BOOL ReplaceView(int row, int col, CRuntimeClass* pViewClass);
	CView* GetView(int n = 0);
// Implementation
public:
	virtual ~CSplitter();

	// Generated message map functions
	//{{AFX_MSG(CSplitter)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
/////////////////////////////////////////////////////////////////////////////
