// SettingDlg.cpp : implementation file
//

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
