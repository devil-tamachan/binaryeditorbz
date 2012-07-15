//#pragma once
#include "afxwin.h"



// CBZInspectView フォーム ビュー

class CBZInspectView : public CFormView
{
	DECLARE_DYNCREATE(CBZInspectView)

protected:
	CBZInspectView();           // 動的生成で使用される protected コンストラクタ
	virtual ~CBZInspectView();

public:
	enum { IDD = IDD_BZINSPECTVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

private:
	bool m_bSigned;
	CBZView* m_pView;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	void ClearAll(void);
	CEdit m_editHex;
	CEdit m_edit8bits;
	CEdit m_editBinary1;
	CEdit m_editBinary2;
	CEdit m_editBinary4;
	CEdit m_editBinary8;
	CEdit m_editFloat;
	CEdit m_editDouble;
	afx_msg void OnBnClickedInsIntel();
	afx_msg void OnBnClickedInsSigned();
	CButton m_check_intel;
	CButton m_check_signed;
	CStatic m_staticBinary1;
	CStatic m_staticBinary2;
	CStatic m_staticBinary4;
	CStatic m_staticBinary8;
	void Update(void);
	void UpdateChecks(void);
};


