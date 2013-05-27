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
	void SetText(LPCTSTR s);
#ifdef _UNICODE
	void AddText(LPCSTR s)
	{
		AddTextT(CA2WEX<>(s));
	}

	void AddTextT(LPCTSTR s)
#else
	void AddText(LPCTSTR s)
#endif
	{
		int n = FindStringExact(-1, s);
		if(n != CB_ERR)
			DeleteString(n);
		InsertString(0, s);
		SetCurSel(0);
	}

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

public:
	void DrawGripper(CDC* pDC, const CRect& rect);//KB843490
	void EraseNonClient();//KB843490
	virtual void DoPaint(CDC* pDC);//KB843490

	// Generated message map functions
protected:
	//{{AFX_MSG(CComboToolBar)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg void OnSelectEndOk();
	afx_msg void OnNcPaint();//KB843490

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
