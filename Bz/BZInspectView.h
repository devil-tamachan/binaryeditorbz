#pragma once



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
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
};


