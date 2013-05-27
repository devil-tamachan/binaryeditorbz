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
#include "ComboBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComboToolBar

CComboToolBar::CComboToolBar()
{
}

CComboToolBar::~CComboToolBar()
{
}


BEGIN_MESSAGE_MAP(CComboToolBar, CToolBar)
	//{{AFX_MSG_MAP(CComboToolBar)
	ON_WM_SETFOCUS()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CComboToolBar::OnNcPaint() 
{
	EraseNonClient();
}
void CComboToolBar::EraseNonClient()
{

	// Get window DC that is clipped to the non-client area.
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// Draw the borders in the non-client area.
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	// Erase the parts that are not drawn.
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

	// Draw the gripper in the non-client area.
	DrawGripper(&dc, rectWindow);
}

void CComboToolBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// Paint inside the client area.
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void CComboToolBar::DrawGripper(CDC* pDC, const CRect& rect)
{
	pDC->FillSolidRect( &rect, ::GetSysColor(COLOR_BTNFACE)); // Fill in the background.
	CToolBar::DrawGripper(pDC,rect);
}

BOOL CComboToolBar::CreateComboBox(UINT nID, UINT nIDNext, int width, int height)
{
	SetButtonInfo(CommandToIndex(nIDNext) - 1, nID, TBBS_SEPARATOR, width);
	CRect rect;
	GetItemRect(CommandToIndex(nID), &rect);
	rect.bottom = rect.top + height;
	return m_combo.Create(CBS_DROPDOWN|WS_VISIBLE|WS_TABSTOP|CBS_AUTOHSCROLL, rect, this, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CComboToolBar message handlers

BOOL CComboToolBar::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN) {
		CView* pView = ((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
		BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
		switch (pMsg->wParam) {
		case VK_ESCAPE:
			pView->SetFocus();
			return TRUE;
		case VK_RETURN:
			if(!m_combo.GetDroppedState()) {
				pView->PostMessage(WM_COMMAND, ID_JUMP_FINDNEXT);
				return TRUE;
			}
				break;
		case VK_DOWN:
			if(!m_combo.GetDroppedState()) {
				m_combo.ShowDropDown();
				return TRUE;
			}
			break;
		case 'X':
			if(bCtrl) {
				m_combo.Cut();
				return TRUE;
			}
			break;
		case 'C':
			if(bCtrl) {
				m_combo.Copy();
				return TRUE;
			}
			break;
		case 'V':
			if(bCtrl) {
				m_combo.Paste();
				return TRUE;
			}
			break;
		}
	}
	return CToolBar::PreTranslateMessage(pMsg);
}

void CComboToolBar::OnSetFocus(CWnd* pOldWnd) 
{
	m_combo.SetEditSel(0,-1);
}

/////////////////////////////////////////////////////////////////////////////
// CTBComboBox

CTBComboBox::CTBComboBox()
{
}

CTBComboBox::~CTBComboBox()
{
}

BEGIN_MESSAGE_MAP(CTBComboBox, CComboBox)
	//{{AFX_MSG_MAP(CTBComboBox)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CTBComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));

/*	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logFont);
	logFont.lfWeight = FW_BOLD;
	logFont.lfHeight = -9;
*/
	logFont.lfHeight = -options.nComboHeight;
	logFont.lfWeight = FW_BOLD;
	logFont.lfCharSet = SHIFTJIS_CHARSET;
	logFont.lfQuality = DEFAULT_QUALITY;
	logFont.lfPitchAndFamily = VARIABLE_PITCH;
	lstrcpyn(logFont.lfFaceName, _T("‚l‚r ‚oƒSƒVƒbƒN"), LF_FACESIZE);

	if (m_font.CreateFontIndirect(&logFont))
		SetFont(&m_font);
	return 0;
}

void CTBComboBox::SetText(LPCTSTR s)
{
	SetWindowText(s);
	int len = lstrlen(s);
	SetEditSel(len, len);	// move caret to end
}


/////////////////////////////////////////////////////////////////////////////
// CTBComboBox message handlers

