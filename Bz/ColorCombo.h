#if !defined(AFX_COLORCOMBO_H__752E5202_F1C7_4C88_BCCA_F558987A3822__INCLUDED_)
#define AFX_COLORCOMBO_H__752E5202_F1C7_4C88_BCCA_F558987A3822__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorCombo window

class CColorCombo : public CComboBox
{
// Construction
public:
	CColorCombo();

// Attributes
public:
	BOOL m_bAuto;
	BOOL m_bOther;
	COLORREF m_rgbAuto;
	COLORREF m_rgbOther;

// Operations
public:
	void SetSelColor(COLORREF rgb, COLORREF rgbDefault);
	COLORREF GetSelColor();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorCombo)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorCombo)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORCOMBO_H__752E5202_F1C7_4C88_BCCA_F558987A3822__INCLUDED_)
