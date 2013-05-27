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
	char*	m_pVText;
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
	void PutChar(char c=' ', int n=1);
	void PutStr(LPCSTR str);
	void PutFormatStr(LPCSTR fmt, ...);
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
