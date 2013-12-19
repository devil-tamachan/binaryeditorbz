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
#include <imm.h>
#pragma comment( lib, "imm32" )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextView diagnostics

#ifdef _DEBUG
void CTextView::AssertValid() const
{
	CView::AssertValid();
}

void CTextView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextView

IMPLEMENT_DYNCREATE(CTextView, CView)

BEGIN_MESSAGE_MAP(CTextView, CView)
	//{{AFX_MSG_MAP(CTextView)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
 	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextView construction/destruction


CTextView::CTextView()
{
	m_pFont = NULL;
	m_pVText = NULL;
	m_bResize = FALSE;
	m_bPrinting = FALSE;
	m_ptCaret2.x = m_ptCaret2.y = -1;
	m_bShowCaret2 = FALSE;
	m_bOnSize = FALSE;
	m_pFile = NULL;
}

CTextView::~CTextView()
{
	delete m_pFont;
	delete m_pVText;
}

BOOL CTextView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= WS_VSCROLL|WS_HSCROLL;
	return CView::PreCreateWindow(cs);
}

int CTextView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here

	if(m_pFont == NULL)
		SetDefaultFont();
	return 0;
}

void CTextView::SetDefaultFont(CDC* pDC)
{
	if(m_pFont) delete m_pFont;
	m_pFont = new CFont;
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
//	CString sFontName = AfxGetApp()->GetProfileString("Option", "FontName");
/*	if(options.sFontName.IsEmpty()) {	###1.54
		HGDIOBJ hFont = ::GetStockObject(SYSTEM_FIXED_FONT);
		::GetObject(hFont, sizeof(LOGFONT), &lf);
		m_pFont->CreateFontIndirect(&lf);
	} else {
*/		int nFontSize = AfxGetApp()->GetProfileInt(_T("Option"), _T("FontSize"), 140);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = options.nFontSize;
		if(options.fFontStyle & 1) lf.lfWeight = FW_BOLD;
		if(options.fFontStyle & 2) lf.lfItalic = TRUE;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = FIXED_PITCH;
		lstrcpyn(lf.lfFaceName, options.sFontName, LF_FACESIZE);
		m_pFont->CreatePointFontIndirect(&lf, pDC);
//	}
	OnChangeFont(pDC);
}

void CTextView::OnChangeFont(CDC* pPrintDC)
{
	TEXTMETRIC tm;
	CDC* pDC = pPrintDC;
	if(!pDC) pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(m_pFont);
	pDC->GetTextMetrics(&tm);
	m_cell.cx = tm.tmAveCharWidth;
	m_cell.cy = tm.tmHeight;
//	pDC->LPtoDP(&m_cell);
	pDC->SelectObject(pOldFont);
	if(!pPrintDC) {			// ###1.5
		ReleaseDC(pDC);

		HIMC hIMC;
		if(hIMC = ImmGetContext(GetSafeHwnd())) {
			LOGFONT lf;
			m_pFont->GetLogFont(&lf);
			ImmSetCompositionFont(hIMC, &lf);
			ImmReleaseContext(m_hWnd, hIMC);
		}
	}
}

void CTextView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(cx && cy)
		InitScrollBar();	// ### 1.62
/*	SCROLLINFO sbi;

	sbi.fMask = SIF_PAGE;
	sbi.nPage = cy/m_cell.cy;
	SetScrollInfo(SB_VERT, &sbi, TRUE);
	sbi.nPage = cx/m_cell.cx;
	SetScrollInfo(SB_HORZ, &sbi, TRUE);
*/
}

void CTextView::InitCaret(BOOL bShow)
{
#ifdef _DEBUG
	TRACE("CTextView::InitCaret()\n");
	CWnd *tw = GetFocus();
	if(tw!=NULL)TRACE("GetFocus() = 0x%x, this->m_hWnd == 0x%x\n", (tw->m_hWnd), m_hWnd);
	else TRACE("GetFocus() == NULL\n");
#endif

	if(GetFocus()!=NULL && GetFocus()->m_hWnd == this->m_hWnd) {
		if(m_bIns)
		{
			CreateSolidCaret(2, m_cell.cy);
		} else {
			CreateSolidCaret(m_cell.cx, m_cell.cy);
		}
		MoveCaret(m_ptCaret);
		if(bShow) {
			GetParent()->ShowWindow(SW_SHOWNORMAL);
			ShowCaret();
#ifdef _DEBUG
			TRACE("CTextView::InitCaret(): ShowCaret(0x%x)\n", m_hWnd);
			TRACE("CTextView::InitCaret(): GetCaretBlinkTime()==%u\n", GetCaretBlinkTime());
			TRACE("CTextView::InitCaret(): IsWindowVisible()==%s\n", this->IsWindowVisible()?"Visible":"Not visible");
			TRACE("CTextView::InitCaret(): GetParent()->IsWindowVisible()==%s\n", this->GetParent()->IsWindowVisible()?"Visible":"Not visible");
#endif
			ShowCaret2();
		}
	}
#ifdef _DEBUG
	else TRACE("CTextView::InitCaret(): not focus\n");
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CTextView drawing

void CTextView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
	if(pDC->IsPrinting()) return;

	if(!m_bResize) {
		ResizeFrame();
	}
}

void CTextView::ResizeFrame()
{
	m_bResize = TRUE;
	CRect rFrame, rClient;
	CFrameWnd* pFrame = GetParentFrame();
	GetWindowRect(rClient);
	pFrame->GetWindowRect(rFrame);
//	TRACE("Client %d,%d  Frame %d,%d\n", rClient.Width(), rClient.Height(), rFrame.Width(), rFrame.Height());
	rFrame.x2 = rClient.x1 + (m_cView.cx + 2) * m_cell.cx + GetSystemMetrics(SM_CXVSCROLL);
	pFrame->MoveWindow(rFrame);
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
	SCROLLINFO sbi = { sizeof(SCROLLINFO) };
	GetScrollInfo(SB_VERT, &sbi, SIF_POS);
	pt.y = sbi.nPos;
	GetScrollInfo(SB_HORZ, &sbi, SIF_POS);
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

	CRect r;
	GetClientRect(r);
	CSize cView(r.Width() / m_cell.cx, r.Height() / m_cell.cy);

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
void CTextView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	OnScrollBar(SB_HORZ, nSBCode);	
}

void CTextView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	OnScrollBar(SB_VERT, nSBCode);
}

#define SB_WHEELUP   10
#define SB_WHEELDOWN 11

void CTextView::OnScrollBar(int nBar, UINT nSBCode)
{
	SCROLLINFO sbi = { sizeof(SCROLLINFO) };

	GetScrollInfo(nBar, &sbi, SIF_ALL);
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

BOOL CTextView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	OnScrollBar(SB_VERT, zDelta > 0 ? SB_WHEELUP : SB_WHEELDOWN);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CTextView::ScrlBarRange(int nBar, LONG& val)
{
	SCROLLINFO sbi = { sizeof(SCROLLINFO) };
	GetScrollInfo(nBar, &sbi, SIF_ALL);
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
	//TRACE("CTextView::MoveCaret(): x=%d, y=%d\n", pt.x, pt.y);
	m_ptCaret = pt;
	if(GetFocus() == this) {
		GridToPixel(pt);
		SetCaretPos(pt);
		//TRACE("CTextView::MoveCaret(): SetCaretPos x=%d, y=%d\n", pt.x, pt.y);
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

void CTextView::OnSetFocus(CWnd* pOldWnd) 
{
	//TRACE("CTextView::OnSetFocus\n");
	CView::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	InitCaret();

	Invalidate(true);//非アクティブウィンドウの擬似カレット消去
}

void CTextView::OnKillFocus(CWnd* pNewWnd) 
{
	//TRACE("CTextView::OnKillFocus\n");
	CView::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	HideCaret();
	HideCaret2();

	Invalidate(true);//非アクティブウィンドウの擬似カレット描画
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
		CClientDC* pDC = (CClientDC*)GetDC();
		GridToPixel(pt);
		pDC->PatBlt(pt.x, pt.y, m_cell.cx, m_cell.cy, DSTINVERT);
		ReleaseDC(pDC);
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

void CTextView::PutBegin(CDC* pDC)
{
	m_pDC = pDC;
//	ASSERT(m_pDC->m_hDC!=NULL);
	m_nText = 0;
	Locate(0, 0);
	SetColor();
	if(pDC)
		m_pOldFont = pDC->SelectObject(m_pFont);
}

void CTextView::PutEnd()
{
	PutFlush();
	if(m_pDC)
		m_pDC->SelectObject(m_pOldFont);
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
		if(!(m_pDC->IsPrinting())) {
			COLORREF colText = GetSystemColor(m_colText);
			COLORREF colBk   = GetSystemColor(m_colBk);
			m_pDC->SetTextColor(colText);
			m_pDC->SetBkColor(colBk);
		}
		int xFrom = m_xLoc;
		int xTo = xFrom + m_nText;
		::TextOutA(m_pDC->m_hDC, (m_xLoc-pt.x)*m_cell.cx, m_yLoc*m_cell.cy, m_pVText, m_nText);
		m_xLoc += m_nText;
		m_nText = 0;
		if(m_bShowCaret2 && m_yLoc == m_ptCaret2.y && m_ptCaret2.x >= xFrom && m_ptCaret2.x < xTo) {	// ### 1.62
			InvertCaret(m_ptCaret2);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTtextView printing

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

void CTextView::SetMargin(CDC* pDC)
{
	CRect rMargin;
	pDC->SetViewportOrg(0, 0);
	GetMargin(rMargin, pDC);
	pDC->SetViewportOrg(rMargin.x1, rMargin.y1);
	TRACE("MarginLeft:%d\n", rMargin.x1);
//	pDC->SetViewportExt(rMargin.Width(), rMargin.Height());
}

void CTextView::GetMargin(LPRECT prMargin, CDC* pDC)
{
	int nMapMode = pDC->GetMapMode();
	pDC->SetMapMode(MM_HIMETRIC);
	*prMargin = options.rMargin;
	pDC->LPtoDP(prMargin);
	prMargin->y1 = -prMargin->y1;
	prMargin->y2 = -prMargin->y2;
	pDC->SetMapMode(nMapMode);
}

/* ～1.53
void CTextView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CDC* pScrDC = GetDC();
	int nDPIScr = pScrDC->GetDeviceCaps(LOGPIXELSX);
	ReleaseDC(pScrDC);
	int nDPIPrt = pDC->GetDeviceCaps(LOGPIXELSX);
	LOGFONT logFont;
	m_pFont->GetObject(sizeof(LOGFONT), &logFont);
	if(logFont.lfHeight > 0)
		logFont.lfQuality = 1;
	logFont.lfHeight = logFont.lfHeight * nDPIPrt / nDPIScr;
	logFont.lfWidth  = logFont.lfWidth  * nDPIPrt / nDPIScr;
	m_pScrnFont = m_pFont;
	m_pFont = new CFont;
	m_pFont->CreateFontIndirect(&logFont);
	OnChangeFont(pDC);
//	CView::OnBeginPrinting(pDC, pInfo);
}

void CTextView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add cleanup after printing
	delete m_pFont;
	m_pFont = m_pScrnFont;
	OnChangeFont(pDC);
//	CView::OnEndPrinting(pDC, pInfo);
}


*/