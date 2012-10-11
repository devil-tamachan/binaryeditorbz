#pragma once
#include "afxcmn.h"
#include "afxwin.h"



// CBZAnalyzerView フォーム ビュー

class CBZAnalyzerView : public CFormView
{
	DECLARE_DYNCREATE(CBZAnalyzerView)

protected:
	CBZAnalyzerView();           // 動的生成で使用される protected コンストラクタ
	virtual ~CBZAnalyzerView();

public:
	enum { IDD = IDD_BZANALYZERVIEW };
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
	CProgressCtrl m_progress;
	CListCtrl m_resultList;
	CComboBox m_combo_analyzetype;
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedAnalyzeStart();
	afx_msg void OnBnClickedAnalyzerSave();
	afx_msg void OnBnClickedAnalyzerSaveall();

	unsigned long GetAddress(int nItem);
	BOOL MakeExportDirA(LPSTR pathOutputDir, LPCSTR pathDstFolder);
	int MakeExportPathA(LPSTR pathOutput, LPCSTR pathDir, unsigned long ulStartAddr);
	HRESULT SaveFileA(LPCSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize);
};


