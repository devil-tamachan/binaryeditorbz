// StatBar.cpp : implementation file
//

#include "stdafx.h"
#include "BZ.h"
#include "StatBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatusBarEx

CStatusBarEx::CStatusBarEx()
{
}

CStatusBarEx::~CStatusBarEx()
{
}


BEGIN_MESSAGE_MAP(CStatusBarEx, CStatusBar)
	//{{AFX_MSG_MAP(CStatusBarEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatusBarEx message handlers

void CStatusBarEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect r;
	for(int i = 0;; i++) {
		GetItemRect(i, &r);
		if(r.IsRectEmpty()) break;
		if(r.PtInRect(point)) {
			UINT nID = GetItemID(i);
			AfxGetMainWnd()->PostMessage(WM_COMMAND, nID);
			break;
		}
	}
//	CStatusBar::OnLButtonDown(nFlags, point);
}

void CStatusBarEx::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	OnLButtonDown(nFlags, point);
}
