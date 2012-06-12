// ColorCombo.cpp : implementation file
//
// orginal source : CodeGuru article, Eric Zimmerman coolez@one.net

#include "stdafx.h"
#include "BZ.h"
#include "ColorCombo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_COLORS 16

static COLORREF colors[MAX_COLORS] = {
	RGB(0, 0, 0), RGB(255, 255, 255), RGB(128, 0, 0), RGB(0, 128, 0),
	RGB(128, 128, 0), RGB(0, 0, 128), RGB(128, 0, 128), RGB(0, 128, 128),
	RGB(192, 192, 192), RGB(128, 128, 128), RGB(255, 0, 0), RGB(0, 255, 0),
	RGB(255, 255, 0), RGB(0, 0, 255), RGB(255, 0, 255), RGB(0, 255, 255),
};

/////////////////////////////////////////////////////////////////////////////
// CColorCombo

CColorCombo::CColorCombo()
{
	m_bAuto = FALSE;
	m_bOther = TRUE;
	m_rgbOther = RGB_WHITE;
}

CColorCombo::~CColorCombo()
{
}


BEGIN_MESSAGE_MAP(CColorCombo, CComboBox)
	//{{AFX_MSG_MAP(CColorCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorCombo message handlers

void CColorCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default

	int iItem = lpDrawItemStruct->itemID;

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	CRect rcItem(&(lpDrawItemStruct->rcItem));
	dc.FillSolidRect(rcItem, RGB(255, 255, 255));

	COLORREF rgb;
	int mode = FALSE;
	if(iItem < MAX_COLORS)
		rgb = colors[iItem];
	else if(m_bOther && iItem == MAX_COLORS) {
		mode = INVALID;
		rgb = m_rgbOther;
	} else {
		mode = TRUE;
		rgb = GetSystemColor(m_rgbAuto);
	}

	CBrush brush(rgb);
	CBrush* pOldBrush = dc.SelectObject(&brush);
	rcItem.InflateRect(-2, -2);
	dc.Rectangle(rcItem);
	rcItem.InflateRect(2, 2);
	dc.SelectObject(pOldBrush);

	if(mode) {
		CString sLabel;
		sLabel.LoadString(mode > 0 ? IDS_AUTOCOLOR : IDS_OTHERCOLOR);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(rgb ^ 0xFFFFFF);
		dc.DrawText(sLabel, rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		dc.DrawFocusRect(rcItem);

	dc.Detach();
	
//	CComboBox::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CColorCombo::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	for_to(i, MAX_COLORS + m_bOther)
		AddString("");

	SetCurSel(0);

	CComboBox::PreSubclassWindow();
}

void CColorCombo::SetSelColor(COLORREF rgb, COLORREF rgbDefault)
{
	BOOL bAuto = IsSystemColor(rgbDefault);
	if(bAuto) {
		if(!m_bAuto)
			AddString("");
		m_rgbAuto = rgbDefault;
		if(IsSystemColor(rgb)) {
			SetCurSel(MAX_COLORS + m_bOther);
			m_bAuto = bAuto;
			return;
		}
	} else {
		if(m_bAuto)
			DeleteString(MAX_COLORS + m_bOther);
	}
	m_bAuto = bAuto;
	for_to(i, MAX_COLORS) {
		if(rgb == colors[i]) {
			SetCurSel(i);
			m_rgbOther = RGB_WHITE;
			return;
		}
	}
	if(m_bOther) {
		m_rgbOther = rgb;
		SetCurSel(MAX_COLORS);
	}
}

COLORREF CColorCombo::GetSelColor()
{
	int iSel = GetCurSel();
	if(m_bOther && iSel == MAX_COLORS) {
		CColorDialog dlgColor(m_rgbOther);
		if(dlgColor.DoModal() == IDOK) {
			m_rgbOther = dlgColor.GetColor();
			Invalidate();
		}
		return dlgColor.GetColor();
	}
	if(iSel == MAX_COLORS + m_bOther) return m_rgbAuto;
	return colors[iSel];
}
