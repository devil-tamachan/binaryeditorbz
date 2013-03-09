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

#include "stdafx.h"
#include "BZ.h"
#include "SettingDlg.h"
//#include "hlp\topics.h"
#include "chm\topics.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog


CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingDlg)
	m_dwMaxOnMemory = options.dwMaxOnMemory / 1024;
	m_dwMaxMapSize  = options.dwMaxMapSize / (1024*1024);
	m_bDWordAddr = options.bDWordAddr;
	m_nDumpPage = options.nDumpPage;
	//}}AFX_DATA_INIT
}


void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingDlg)
	DDX_Text(pDX, IDE_MAXONMEMORY, m_dwMaxOnMemory);
	DDX_Text(pDX, IDE_MAXMAPSIZE, m_dwMaxMapSize);
	DDX_Check(pDX, IDB_DWORDADDR, m_bDWordAddr);
	DDX_Text(pDX, IDE_DUMPPAGE, m_nDumpPage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingDlg)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg message handlers

void CSettingDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	options.dwMaxOnMemory = m_dwMaxOnMemory * 1024;
	options.dwMaxMapSize  = m_dwMaxMapSize * (1024*1024);
	options.bDWordAddr = m_bDWordAddr;
	options.nDumpPage = m_nDumpPage;

	CDialog::OnOK();
}

void CSettingDlg::OnHelp() 
{
	// TODO: Add your control notification handler code here
	//AfxGetApp()->WinHelp(HID_FILEMAPPING);
	AfxGetApp()->HtmlHelp(HID_FILEMAPPING);
}
/////////////////////////////////////////////////////////////////////////////
// CInputDlg dialog


CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputDlg)
	m_sValue = _T("");
	//}}AFX_DATA_INIT
}


void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputDlg)
	DDX_Text(pDX, IDE_VALUE, m_sValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialog)
	//{{AFX_MSG_MAP(CInputDlg)
	ON_BN_CLICKED(IDB_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInputDlg message handlers

void CInputDlg::OnReset() 
{
	// TODO: Add your control notification handler code here
	m_sValue = _T("0");
	UpdateData(FALSE);
}
