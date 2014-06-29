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

//#include "hlp\topics.h"
//#include "chm\topics.h"


/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog

class CSettingDlg : public CDialogImpl<CSettingDlg>, public WTL::CWinDataExchange<CSettingDlg>
{
public:
	enum { IDD = IDD_SETTING };

	CSettingDlg() : CDialogImpl<CSettingDlg>()
	{
	//	m_dwMaxOnMemory = options.dwMaxOnMemory / 1024;
	//	m_dwMaxMapSize  = options.dwMaxMapSize / (1024*1024);
		m_bQWordAddr = options.bQWordAddr;
    m_bClearUndoRedoWhenSave = options.bClearUndoRedoWhenSave; 
		m_nDumpPage = options.nDumpPage;
	}

//	UINT	m_dwMaxOnMemory;
//	UINT	m_dwMaxMapSize;
	BOOL	m_bQWordAddr;
  BOOL  m_bClearUndoRedoWhenSave;
	int		m_nDumpPage;

	BEGIN_MSG_MAP(CSettingDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDHELP, OnHelp)
		COMMAND_ID_HANDLER_EX(IDB_RESETSETTINGS, OnResetSettings)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CSettingDlg)
//		DDX_UINT(IDE_MAXONMEMORY, m_dwMaxOnMemory)
//		DDX_UINT(IDE_MAXMAPSIZE, m_dwMaxMapSize)
		DDX_CHECK(IDC_QWORDADDR, m_bQWordAddr);
    DDX_CHECK(IDC_CLEARUNDOREDOWHENSAVE, m_bClearUndoRedoWhenSave);
		DDX_INT(IDE_DUMPPAGE, m_nDumpPage)
	END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		DoDataExchange(DDX_LOAD);
		return TRUE;
	}

	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DoDataExchange(DDX_SAVE);

	//	options.dwMaxOnMemory = m_dwMaxOnMemory * 1024;
	//	options.dwMaxMapSize  = m_dwMaxMapSize * (1024*1024);
		options.bQWordAddr = m_bQWordAddr;
    options.bClearUndoRedoWhenSave = m_bClearUndoRedoWhenSave;
		options.nDumpPage = m_nDumpPage;
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}

	void OnHelp(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		//AfxGetApp()->WinHelp(HID_FILEMAPPING);
		//AfxGetApp()->HtmlHelp(HID_FILEMAPPING);
		ShellExecute(NULL, _T("open"), _T("http://devil-tamachan.github.io/BZDoc/#filemapping.filemapping"), NULL, NULL, SW_SHOWNORMAL);
	}

	void OnResetSettings(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
    options.ResetSettings();
		EndDialog(nID);
  }

};

/////////////////////////////////////////////////////////////////////////////
// CInputDlg dialog

class CInputDlg : public CDialogImpl<CInputDlg>, public WTL::CWinDataExchange<CInputDlg>
{
public:
	enum { IDD = IDD_INPUT };

	CInputDlg() : CDialogImpl<CInputDlg>()
	{
		m_sValue = _T("");
	}

	CString	m_sValue;

	BEGIN_MSG_MAP(CInputDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDB_RESET, OnReset)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CInputDlg)
		DDX_TEXT(IDE_VALUE, m_sValue)
	END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		DoDataExchange(DDX_LOAD);

		return TRUE;
	}

	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DoDataExchange(DDX_SAVE);
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}


	void OnReset(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		m_sValue = _T("0");
		DoDataExchange(DDX_LOAD);
	}
};
