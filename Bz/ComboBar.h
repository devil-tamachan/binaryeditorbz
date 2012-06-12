// ComboBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTBComboBox window

class CTBComboBox : public CComboBox
{
// Construction
public:
	CTBComboBox();

// Attributes
private:
	CFont m_font;

// Operations
public:
	void SetText(LPCSTR s);
	void AddText(LPCSTR s);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTBComboBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTBComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTBComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CComboToolBar window

class CComboToolBar : public CToolBar
{
// Construction
public:
	CComboToolBar();

// Attributes
public:
	CTBComboBox m_combo;

// Operations
public:
	BOOL CreateComboBox(UINT nID, UINT nIDNext, int width, int height);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboToolBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CComboToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CComboToolBar)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg void OnSelectEndOk();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
