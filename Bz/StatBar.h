// StatBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatusBarEx window

class CStatusBarEx : public CStatusBar
{
// Construction
public:
	CStatusBarEx();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatusBarEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStatusBarEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStatusBarEx)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
