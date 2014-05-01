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
#include "TextView.h"
#include "MainFrm.h"
#include <imm.h>
#pragma comment( lib, "imm32" )




/*BOOL CTextView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= WS_VSCROLL|WS_HSCROLL;
	return CView::PreCreateWindow(cs);
}*/

void CTextView::SetDefaultFont(HDC hDC)
{
	if(m_pFont) delete m_pFont;
  m_pFont = new WTL::CFont;
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
//	CString sFontName = AfxGetApp()->GetProfileString("Option", "FontName");
/*	if(options.sFontName.IsEmpty()) {	###1.54
		HGDIOBJ hFont = ::GetStockObject(SYSTEM_FIXED_FONT);
		::GetObject(hFont, sizeof(LOGFONT), &lf);
		m_pFont->CreateFontIndirect(&lf);
	} else {
*/		//int nFontSize = AfxGetApp()->GetProfileInt(_T("Option"), _T("FontSize"), 140);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = options.nFontSize;
		if(options.fFontStyle & 1) lf.lfWeight = FW_BOLD;
		if(options.fFontStyle & 2) lf.lfItalic = TRUE;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FIXED_PITCH;
		lstrcpyn(lf.lfFaceName, options.sFontName, LF_FACESIZE);
		m_pFont->CreatePointFontIndirect(&lf, hDC);
//	}
	OnChangeFont(hDC);
}

void CTextView::OnChangeFont(HDC hPrintDC)
{
	TEXTMETRIC tm;
	HDC hDC = hPrintDC;
	if(!hDC) hDC = GetDC();
  WTL::CDCHandle cdc(hDC);
  WTL::CFontHandle pOldFont = cdc.SelectFont(*m_pFont);
	cdc.GetTextMetrics(&tm);
	m_cell.cx = tm.tmAveCharWidth;
	m_cell.cy = tm.tmHeight;
//	pDC->LPtoDP(&m_cell);
	cdc.SelectFont(pOldFont);
	if(!hPrintDC) {			// ###1.5
		ReleaseDC(hDC);

		HIMC hIMC;
		if(hIMC = ImmGetContext(m_hWnd)) {
			LOGFONT lf;
			m_pFont->GetLogFont(&lf);
			ImmSetCompositionFont(hIMC, &lf);
			ImmReleaseContext(m_hWnd, hIMC);
		}
	}
}


void CTextView::InitCaret(BOOL bShow)
{
#ifdef _DEBUG
	ATLTRACE("CTextView::InitCaret()\n");
	HWND hwndFocused = GetFocus();
	if(hwndFocused!=NULL)ATLTRACE("GetFocus() = 0x%x, this->m_hWnd == 0x%x\n", hwndFocused, m_hWnd);
	else ATLTRACE("GetFocus() == NULL\n");
#endif

	if(GetFocus() == m_hWnd) {
		if(m_bIns)
		{
			CreateSolidCaret(2, m_cell.cy);
		} else {
			CreateSolidCaret(m_cell.cx, m_cell.cy);
		}
		MoveCaret(m_ptCaret);
		if(bShow) {
			GetParent().ShowWindow(SW_SHOWNORMAL);
			ShowCaret();
#ifdef _DEBUG
			//ATLTRACE("CTextView::InitCaret(): ShowCaret(0x%x)\n", m_hWnd);
			//ATLTRACE("CTextView::InitCaret(): GetCaretBlinkTime()==%u\n", GetCaretBlinkTime());
			//ATLTRACE("CTextView::InitCaret(): IsWindowVisible()==%s\n", this->IsWindowVisible()?"Visible":"Not visible");
			//ATLTRACE("CTextView::InitCaret(): GetParent()->IsWindowVisible()==%s\n", this->GetParent()->IsWindowVisible()?"Visible":"Not visible");
#endif
			ShowCaret2();
		}
	}
#ifdef _DEBUG
	else ATLTRACE("CTextView::InitCaret(): not focus\n");
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CTextView drawing

void CTextView::ResizeFrame()
{
	m_bResize = TRUE;
  WTL::CRect rFrame, rClient;
	CMainFrame* pMainFrame = GetMainFrame();
  if(!pMainFrame)return;
	GetWindowRect(rClient);
	pMainFrame->GetWindowRect(rFrame);
//	TRACE("Client %d,%d  Frame %d,%d\n", rClient.Width(), rClient.Height(), rFrame.Width(), rFrame.Height());
	rFrame.x2 = rClient.x1 + (m_cView.cx + 2) * m_cell.cx + GetSystemMetrics(SM_CXVSCROLL);
	pMainFrame->MoveWindow(rFrame);
	//InitCaret(FALSE);
}

void CTextView::SetTextSize(SIZE cTotal, int nPage)
{
	m_nPage  = nPage;
/*	SCROLLINFO sbi;
	sbi.fMask = SIF_RANGE;
	sbi.nMin  = 0;
	sbi.nMax  = cTotal.cy;
	SetScrollInfo(SB_VERT, &sbi, FALSE);
	sbi.nMax  = cTotal.cx;
	SetScrollInfo(SB_HORZ, &sbi, FALSE);
*/
	
	SetTextSize(cTotal);
	InitScrollBar();	// ### 1.62
}

void CTextView::SetTextSize(SIZE cTotal)
{
	if(!m_pVText || m_cTotal.cx != cTotal.cx) {
		delete m_pVText;
		m_pVText = new char[cTotal.cx+1];
	}
	m_cTotal = cTotal;
}

POINT CTextView::GetScrollPos()
{
	POINT pt;
	SCROLLINFO sbi = { sizeof(SCROLLINFO), SIF_POS };
	GetScrollInfo(SB_VERT, &sbi);
	pt.y = sbi.nPos;
	GetScrollInfo(SB_HORZ, &sbi);
	pt.x = sbi.nPos;
	return pt;
}

void CTextView::ScrollToPos(POINT pt)
{
	SCROLLINFO sbi = { sizeof(SCROLLINFO), SIF_POS };
	//sbi.fMask = SIF_POS;
	sbi.nPos = pt.y;
	SetScrollInfo(SB_VERT, &sbi, TRUE);
}

void CTextView::SetScrollHome(POINT ptHome)
{
	m_ptHome = ptHome;
}

void CTextView::InitScrollBar()	// ### 1.62
{
	if(m_bOnSize || m_cTotal.cx <= 0 || m_cTotal.cy <= 0)
		return;

	m_bOnSize = TRUE;

  WTL::CRect r;
	GetClientRect(r);
  WTL::CSize cView(r.Width() / m_cell.cx, r.Height() / m_cell.cy);

	SCROLLINFO sbi = { sizeof(SCROLLINFO) };
	sbi.fMask = SIF_RANGE | SIF_PAGE;
	sbi.nMin  = 0;
	sbi.nMax  = m_cTotal.cx;
	sbi.nPage = cView.cx;
	SetScrollInfo(SB_HORZ, &sbi, TRUE);
	sbi.nMax  = m_cTotal.cy;
	sbi.nPage = cView.cy;
	SetScrollInfo(SB_VERT, &sbi, TRUE);

	m_bOnSize = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTextView message handlers


void CTextView::AlignTextPos(POINT& pt)
{
	pt.x -= pt.x % m_cell.cx;
	pt.y -= pt.y % m_cell.cy;
}

void CTextView::AlignTextPos(RECT& r)
{
	r.x1 -= r.x1 % m_cell.cx;
	r.y1 -= r.y1 % m_cell.cy;
	r.x2 -= r.x2 % m_cell.cx;
	r.y2 -= r.y2 % m_cell.cy;
}
/*
void CTextView::ClientToGrid(POINT& pt)
{
	POINT ptHome = GetScrollPosition();
	pt.x += ptHome.x;
	pt.y += ptHome.y;
	PixelToGrid(pt);
}
*/
void CTextView::PixelToGrid(POINT& pt)
{
	pt.x /= m_cell.cx;
	pt.y /= m_cell.cy;
}

void CTextView::PixelToGrid(RECT& rect)
{
	rect.x1 /= m_cell.cx;
	rect.x2 /= m_cell.cx;
	rect.y1 /= m_cell.cy;
	rect.y2 /= m_cell.cy;
}

void CTextView::GridToPixel(POINT& pt)
{
	pt.x *= m_cell.cx;
	pt.y *= m_cell.cy;
}

/*
void CTextView::GridToPixel(RECT& rect)
{
	rect.x1 *= m_cell.cx;
	rect.x2 *= m_cell.cx;
	rect.y1 *= m_cell.cy;
	rect.y2 *= m_cell.cy;
}
*/


void CTextView::OnScrollBar(int nBar, UINT nSBCode)
{
	SCROLLINFO sbi = { sizeof(SCROLLINFO), SIF_ALL };

	GetScrollInfo(nBar, &sbi);
//	TRACE("SBCode:%d, Pos:%d, Track:%d\n",nSBCode, sbi.nPos, sbi.nTrackPos);
	LONG pos = sbi.nPos;
	switch (nSBCode) {
		case SB_TOP:		pos = sbi.nMin;  break;
		case SB_BOTTOM:		pos = sbi.nMax; break;
		case SB_LINEUP:		pos--; break;
		case SB_LINEDOWN:	pos++; break;
		case SB_PAGEUP: 	pos -= m_nPage; break;
		case SB_PAGEDOWN:	pos += m_nPage; break;
		case SB_THUMBTRACK: pos = sbi.nTrackPos; break;
		case SB_WHEELUP:
		case SB_WHEELDOWN:
			UINT uScrollLines = 3;
			::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScrollLines, 0);
			if(nSBCode == SB_WHEELDOWN) pos += uScrollLines;
			else pos -= uScrollLines;
			break;
	}
	ScrlBarRange(nBar, pos);
	if(pos != sbi.nPos) {
		if(nBar == SB_VERT)
			ScrollClient(0, sbi.nPos - pos);
		else
			ScrollClient(sbi.nPos - pos, 0);
		sbi.fMask = SIF_POS;
		sbi.nPos = pos;
		SetScrollInfo(nBar, &sbi, TRUE);
	}
}


void CTextView::ScrlBarRange(int nBar, LONG& val)
{
	SCROLLINFO sbi = { sizeof(SCROLLINFO), SIF_ALL };
	GetScrollInfo(nBar, &sbi);
	LONG nMax = sbi.nMax - sbi.nPage +1;
	val = min(max(val, sbi.nMin), nMax);
}

void CTextView::ScrollClient(int dx, int dy)
{
	RECT rClient;
	GetClientRect(&rClient);
	if(!dy) rClient.x1 = m_ptHome.x*m_cell.cx;
	if(!dx) rClient.y1 = m_ptHome.y*m_cell.cy;
	m_ptCaret2.x += dx;		// ### 1.62
	m_ptCaret2.y += dy;
	ScrollWindow(dx*m_cell.cx, dy*m_cell.cy, &rClient, &rClient);
}

void CTextView::ScrollBy(int dx, int dy, BOOL bScrl)
{
	POINT pt = GetScrollPos();
	pt.x += dx;
	pt.y += dy;
	ScrlBarRange(SB_VERT, pt.y);
	ScrollToPos(pt);
	if(bScrl)
		ScrollClient(-dx, -dy);
}

void CTextView::MoveCaret(POINT pt)
{
//	if(pt.x == -1)
//		TRACE("!");
	ATLTRACE("CTextView::MoveCaret(): x=%d, y=%d\n", pt.x, pt.y);
	m_ptCaret = pt;
	if(GetFocus() == m_hWnd) {
		GridToPixel(pt);
    SetCaretPos(pt.x, pt.y);
		ATLTRACE("CTextView::MoveCaret(): SetCaretPos x=%d, y=%d\n", pt.x, pt.y);
		if(pt.x >= 0) {
			HIMC hIMC;
			if(hIMC = ImmGetContext(m_hWnd)) {
				COMPOSITIONFORM CompForm;
				CompForm.dwStyle = CFS_POINT;
				CompForm.ptCurrentPos = pt;
				ImmSetCompositionWindow(hIMC, &CompForm);
				ImmReleaseContext(m_hWnd, hIMC);
			}
		}
	}
}

// Sub caret drawing ### 1.62

void CTextView::MoveCaret2(POINT pt)
{
	if(m_bShowCaret2) {
		InvertCaret(m_ptCaret2);
		InvertCaret(pt);
	}
	m_ptCaret2 = pt;
}

void CTextView::ShowCaret2()
{
	if(!m_bShowCaret2) {
		InvertCaret(m_ptCaret2);
		m_bShowCaret2 = TRUE;
	}
}

void CTextView::HideCaret2()
{
	if(m_bShowCaret2) {
		InvertCaret(m_ptCaret2);
		m_bShowCaret2 = FALSE;
	}
}

void CTextView::InvertCaret(POINT pt)
{
	if(!options.bSubCursor) return;
	if(pt.x >= m_ptHome.x && pt.y >= m_ptHome.y) {
    WTL::CDC cdc = GetDC();
		GridToPixel(pt);
		cdc.PatBlt(pt.x, pt.y, m_cell.cx, m_cell.cy, DSTINVERT);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTextView Virtual Text VRAM

void CTextView::Locate(int x, int y)
{
	PutFlush();
	m_xLoc = x;
	m_yLoc = y;
	if(IsToFile()) {
		if(y)
			m_pFile->Write("\r\n", 2);
		if(x) {
			PutChar(' ', x);
		}
	}
}

void CTextView::SetColor(DWORD colText, DWORD colBk)
{
	if(!IsToFile() && (m_colText != colText || m_colBk != colBk)) {
		PutFlush();
		m_colText = colText;
		m_colBk = colBk;
	}
}

void CTextView::PutChar(char c, int n)
{
	if(!n || m_nText >= m_cTotal.cx) return;
	for_to(i, n)
		*(m_pVText + m_nText++) = c;
	*(m_pVText + m_nText) = '\0';
}

void CTextView::PutStr(LPCSTR str)
{
	lstrcpyA(m_pVText + m_nText, str);
	m_nText += lstrlenA(str);
}

void CTextView::PutFormatStr(LPCSTR fmt, ...)
{
//	m_nText += wsprintf(m_pVText+m_nText, str, val);
	m_nText += wvsprintfA(m_pVText + m_nText, fmt, (va_list)(&fmt + 1));
}

void CTextView::PutBegin(WTL::CDCHandle dc)
{
	m_dc = dc;
//	ATLASSERT(m_pDC->m_hDC!=NULL);
	m_nText = 0;
	Locate(0, 0);
	SetColor();
  if(dc.m_hDC)
		m_pOldFont = dc.SelectFont(*m_pFont);
}

void CTextView::PutEnd()
{
	PutFlush();
	if(m_dc.m_hDC)
    m_dc.SelectFont(m_pOldFont);
}

void CTextView::PutFlush()
{
	if(m_nText) {
		if(IsToFile()) {
			m_pFile->Write(m_pVText, m_nText);
			m_nText = 0;
			return;
		}
		POINT pt = GetScrollPos();
    if(!m_bPrinting) {
			COLORREF colText = GetSystemColor(m_colText);
			COLORREF colBk   = GetSystemColor(m_colBk);
			m_dc.SetTextColor(colText);
			m_dc.SetBkColor(colBk);
		}
		int xFrom = m_xLoc;
		int xTo = xFrom + m_nText;
		::TextOutA(m_dc.m_hDC, (m_xLoc-pt.x)*m_cell.cx, m_yLoc*m_cell.cy, m_pVText, m_nText);
		m_xLoc += m_nText;
		m_nText = 0;
		if(m_bShowCaret2 && m_yLoc == m_ptCaret2.y && m_ptCaret2.x >= xFrom && m_ptCaret2.x < xTo) {	// ### 1.62
			InvertCaret(m_ptCaret2);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTtextView printing
/*
BOOL CTextView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTextView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	m_bPrinting = TRUE;
	SetDefaultFont(pDC);
//	CView::OnBeginPrinting(pDC, pInfo);
}

void CTextView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pInfo)
		SetMargin(pDC);
	
	CView::OnPrepareDC(pDC, pInfo);
}

void CTextView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_bPrinting = FALSE;
	SetDefaultFont();
	Invalidate();
	
//	CView::OnEndPrinting(pDC, pInfo);
}
*/
void CTextView::SetMargin(HDC hDC)
{
  WTL::CDCHandle cdc(hDC);
  WTL::CRect rMargin;
	cdc.SetViewportOrg(0, 0);
	GetMargin(rMargin, hDC);
	cdc.SetViewportOrg(rMargin.x1, rMargin.y1);
	ATLTRACE("MarginLeft:%d\n", rMargin.x1);
//	pDC->SetViewportExt(rMargin.Width(), rMargin.Height());
}

void CTextView::GetMargin(LPRECT prMargin, HDC hDC)
{
  WTL::CDCHandle cdc(hDC);
	int nMapMode = cdc.GetMapMode();
	cdc.SetMapMode(MM_HIMETRIC);
	*prMargin = options.rMargin;
	cdc.LPtoDP(prMargin);
	prMargin->y1 = -prMargin->y1;
	prMargin->y2 = -prMargin->y2;
	cdc.SetMapMode(nMapMode);
}
