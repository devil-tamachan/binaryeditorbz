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


#pragma once
#include "afxcmn.h"
#include "afxwin.h"



// CBZAnalyzerView フォーム ビュー

class CBZAnalyzerView : public CDialogImpl<CBZAnalyzerView>, public WTL::CWinDataExchange<CBZAnalyzerView>
{
	//DECLARE_DYNCREATE(CBZAnalyzerView)
public:
	enum { IDD = IDD_BZANALYZERVIEW };

	CBZAnalyzerView() : CDialogImpl<CBZAnalyzerView>()
	{
		m_bzview = NULL;
	}
	~CBZAnalyzerView() { };

	BOOL PreTranslateMessage(MSG* pMsg){
        return CWindow::IsDialogMessage(pMsg);
    }

	BEGIN_MSG_MAP(CBZAnalyzerView)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDB_ANALYZE_START, OnBnClickedAnalyzeStart)
		COMMAND_ID_HANDLER_EX(IDB_ANALYZER_SAVE, OnBnClickedAnalyzerSave)
		COMMAND_ID_HANDLER_EX(IDB_ANALYZER_SAVEALL, OnBnClickedAnalyzerSaveall)
	END_MSG_MAP()


	BEGIN_DDX_MAP(CBZAnalyzerView)
		DDX_CONTROL_HANDLE(IDP_ANALYZE_PERCENT, m_progress);
		DDX_CONTROL_HANDLE(IDL_ANALYZE_RESULT, m_resultList);
		DDX_CONTROL_HANDLE(IDC_ANALYZE_TYPE, m_combo_analyzetype);
	END_DDX_MAP()

	WTL::CProgressBarCtrl m_progress;
	WTL::CListViewCtrl m_resultList;
	WTL::CComboBox m_combo_analyzetype;

	/*
	BZView *m_bzview;
	void SetBZView(BZView bzview)
	{
		m_bzview = bzview;
	}*/

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		DoDataExchange(DDX_LOAD);

		if(m_combo_analyzetype.GetCount()==0)
		{
			m_combo_analyzetype.InsertString(0, "zlib (deflate)");
			m_combo_analyzetype.SetCurSel(0);

			m_resultList.DeleteAllItems();
			m_resultList.InsertColumn(0, "Address", LVCFMT_LEFT, 120);
			//	m_resultList.InsertColumn(1, "Size", LVCFMT_LEFT, 80);
		}
		//CSplitterWnd* pSplit = (CSplitter*)GetParent();
		//pSplit->SetColumnInfo(0, 180, 0);
	}
	void Clear()
	{
		if(::IsWindow(m_resultList.m_hWnd))
			m_resultList.DeleteAllItems();
	}

	void OnBnClickedAnalyzeStart(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBnClickedAnalyzerSave(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBnClickedAnalyzerSaveall(UINT uNotifyCode, int nID, CWindow wndCtl);

	unsigned long GetAddress(int nItem);
	BOOL MakeExportDirA(LPSTR pathOutputDir, LPCSTR pathDstFolder);
	int MakeExportPathA(LPSTR pathOutput, LPCSTR pathDir, unsigned long ulStartAddr);
	HRESULT SaveFileA(LPCSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize);
};


