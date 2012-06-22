// BZBmpVw.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView view

// #define BMPWIDTH	128		### 1.54c
#define BMPSPACE	8

class CBZBmpView : public CScrollView
{
protected:
	CBZBmpView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBZBmpView)

// Attributes
private:
	HBITMAP m_hBmp;
	SIZE	m_cBmp;
	LPBITMAPINFOHEADER m_lpbi;
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZBmpView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL
	afx_msg void OnBmpViewMode(UINT nID);

// Implementation
protected:
	virtual ~CBZBmpView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CBZBmpView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBmpViewColorWidth(UINT nID);
};

/////////////////////////////////////////////////////////////////////////////
