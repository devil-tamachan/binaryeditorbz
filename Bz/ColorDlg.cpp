// ColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BZ.h"
#include "ColorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDS_LABEL_START  IDS_COLOR_ADDRESS

extern COLORREF colorsDefault[TCOLOR_COUNT][2];
COLORREF GetSystemColor(COLORREF rgb);

/////////////////////////////////////////////////////////////////////////////
// CSetupColorDialog dialog


CSetupColorDialog::CSetupColorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSetupColorDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetupColorDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pSampleFont = NULL;
}


void CSetupColorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupColorDialog)
	DDX_Control(pDX, IDC_PARTSLIST, m_listParts);
	DDX_Control(pDX, IDC_TEXTCOLOR, m_cbTextColor);
	DDX_Control(pDX, IDC_BACKCOLOR, m_cbBackColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupColorDialog, CDialog)
	//{{AFX_MSG_MAP(CSetupColorDialog)
	ON_LBN_SELCHANGE(IDC_PARTSLIST, OnSelchangePartslist)
	ON_WM_CTLCOLOR()
	ON_CBN_SELENDOK(IDC_TEXTCOLOR, OnSelEndOkTextColor)
	ON_CBN_SELENDOK(IDC_BACKCOLOR, OnSelEndOkBackColor)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupColorDialog message handlers

BOOL CSetupColorDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	memcpy(m_colors, options.colors, sizeof(m_colors));

	for_to(i, TCOLOR_COUNT) {
		CString sLabel;
		sLabel.LoadString(IDS_LABEL_START + i);
		m_listParts.AddString(sLabel);
	}
	m_listParts.SetCurSel(0);
	OnSelchangePartslist();
	
	// TODO: Add extra initialization here
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetupColorDialog::OnOK() 
{
	// TODO: Add extra validation here
	memcpy(options.colors, m_colors, sizeof(m_colors));

	CDialog::OnOK();
}

void CSetupColorDialog::OnApply() 
{
	// TODO: Add your control notification handler code here
	memcpy(options.colors, m_colors, sizeof(m_colors));
	AfxGetMainWnd()->Invalidate(TRUE);
}

void CSetupColorDialog::OnSelchangePartslist() 
{
	// TODO: Add your control notification handler code here
	int iSel = 	m_listParts.GetCurSel();
	if(iSel == LB_ERR) return;

	m_cbTextColor.SetSelColor(m_colors[iSel][0], colorsDefault[iSel][0]);
	m_cbBackColor.SetSelColor(m_colors[iSel][1], colorsDefault[iSel][1]);
	GetDlgItem(IDC_SAMPLE)->Invalidate();
}

HBRUSH CSetupColorDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here

	if(pWnd->GetDlgCtrlID() == IDC_SAMPLE) {
		int iSel = 	m_listParts.GetCurSel();
		if(iSel != LB_ERR) {
			COLORREF rgbText = GetSystemColor(m_colors[iSel][0]);
			COLORREF rgbBack = GetSystemColor(m_colors[iSel][1]);
			COLORREF rgbBG = GetSystemColor(m_colors[TCOLOR_TEXT][1]);
			if(m_brush.m_hObject)
				m_brush.DeleteObject();
			m_brush.CreateSolidBrush(rgbBG);
			hbr = (HBRUSH)m_brush;
			pDC->SetTextColor(rgbText);
			pDC->SetBkColor(rgbBack);
			pDC->SelectObject(m_pSampleFont);
		}
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CSetupColorDialog::OnSelEndOkTextColor() 
{
	// TODO: Add your control notification handler code here
	int iSel = 	m_listParts.GetCurSel();
	m_colors[iSel][0] = m_cbTextColor.GetSelColor();
	GetDlgItem(IDC_SAMPLE)->Invalidate();
}

void CSetupColorDialog::OnSelEndOkBackColor() 
{
	// TODO: Add your control notification handler code here
	int iSel = 	m_listParts.GetCurSel();
	m_colors[iSel][1] = m_cbBackColor.GetSelColor();
	GetDlgItem(IDC_SAMPLE)->Invalidate();
}
