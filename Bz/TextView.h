// TextView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextView view

#define ID_SCROLL_V		200

class CTextView : public CView
{
protected:
	CTextView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextView)

// Attributes
public:
	BOOL	m_bResize;
protected:
	SIZE	m_cTotal;
	SIZE	m_cView;
	int		m_nPage;
	SIZE	m_cell;
	POINT	m_ptHome;
	POINT	m_ptCaret;
	POINT	m_ptCaret2;		// ### 1.62
	BOOL	m_bShowCaret2;
	BOOL	m_bIns;
//	SCROLLINFO m_SBinfoV;
	void	AlignVScrlBar();
	CFont*	m_pFont;
	CFont*	m_pOldFont;
	CFont*	m_pScrnFont;
	BOOL	m_bOnSize;		// ### 1.62

	int		m_xLoc, m_yLoc;
	DWORD	m_colText;
	DWORD	m_colBk;
	TCHAR*	m_pVText;
	int		m_nText;
	CDC*	m_pDC;
	BOOL	m_bPrinting;	// ### 1.54
	BOOL IsPrinting() { return m_bPrinting != NULL; }

	CFile*	m_pFile;		// ### 1.63
	BOOL	IsToFile() { return m_pFile != NULL; }

// Operations
private:
	void ScrollClient(int dx, int dy);
	void ScrlBarRange(int nBar, LONG& val);
	void OnScrollBar(int nBar, UINT nSBCode);
protected:
	void InitCaret(BOOL bShow = TRUE);
	void SetViewSize(SIZE cView) { m_cView = cView; };
	int  GetViewWidth() { return m_cView.cx; }
	void SetTextSize(SIZE cTotal, int nPage);
	POINT GetScrollPos();
	void ScrollToPos(POINT ptOrg);
	void SetScrollHome(POINT ptHome);
	void ScrollBy(int dx, int dy, BOOL bScrl);
	void InitScrollBar();	// ### 1.62
	void SetTextSize(SIZE cTotal); // ### 1.63

public:
	void OnChangeFont(CDC* pDC = NULL);
	void ResizeFrame();
	void AlignTextPos(POINT& pt);
	void AlignTextPos(RECT& rect);
	void PixelToGrid(POINT& pt);
	void PixelToGrid(RECT& rect);
	void GridToPixel(POINT& pt);
	void MoveCaret(POINT pt);
	void GetMargin(LPRECT prMargin, CDC* pDC);
	void SetMargin(CDC* pDC);				// ### 1.54
	void SetDefaultFont(CDC* pDC = NULL);	// ### 1.54
	void MoveCaret2(POINT pt);				// ### 1.62
	void ShowCaret2();
	void HideCaret2();
	void InvertCaret(POINT pt);

	void Locate(int x, int y);
	void SetColor(DWORD colText=COLOR_WINDOWTEXT, DWORD colBk=COLOR_WINDOW);
	void PutChar(TCHAR c=' ', int n=1);
	void PutStr(LPCTSTR str);
	void PutFormatStr(LPCTSTR fmt, ...);
	void PutBegin(CDC* pDC);
	void PutEnd();
	void PutFlush();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextView)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
