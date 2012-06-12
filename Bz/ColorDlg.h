#if !defined(AFX_COLORDLG_H__5E2AF757_A2E0_4DB2_9CB0_D9EECAE1541A__INCLUDED_)
#define AFX_COLORDLG_H__5E2AF757_A2E0_4DB2_9CB0_D9EECAE1541A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorDlg.h : header file
//

#include "ColorCombo.h"

/////////////////////////////////////////////////////////////////////////////
// CSetupColorDialog dialog

class CSetupColorDialog : public CDialog
{
// Construction
public:
	CSetupColorDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetupColorDialog)
	enum { IDD = IDD_SETUP_COLOR };
	CListBox	m_listParts;
	CColorCombo	m_cbTextColor;
	CColorCombo	m_cbBackColor;
	//}}AFX_DATA

// Attributes
public:
	COLORREF m_colors[TCOLOR_COUNT][2];
	CFont* m_pSampleFont;
private:
	CBrush m_brush;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetupColorDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetupColorDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePartslist();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	afx_msg void OnSelEndOkTextColor();
	afx_msg void OnSelEndOkBackColor();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORDLG_H__5E2AF757_A2E0_4DB2_9CB0_D9EECAE1541A__INCLUDED_)
