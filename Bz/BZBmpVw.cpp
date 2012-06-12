// BZBmpVw.cpp : implementation file
//

#include "stdafx.h"
#include "BZ.h"
#include "BZView.h"
#include "BZBmpVw.h"
#include "BZDoc.h"
#include "Splitter.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView

IMPLEMENT_DYNCREATE(CBZBmpView, CScrollView)

CBZBmpView::CBZBmpView()
{
	m_hBmp = NULL;
	m_lpbi = NULL;
}

CBZBmpView::~CBZBmpView()
{
	if(m_hBmp) ::DeleteObject(m_hBmp);
	if(m_lpbi) MemFree(m_lpbi);
}


BEGIN_MESSAGE_MAP(CBZBmpView, CScrollView)
	//{{AFX_MSG_MAP(CBZBmpView)
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_BMPVIEW_WIDTH128, ID_BMPVIEW_ZOOM, OnBmpViewMode)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView drawing

void CBZBmpView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CBZDoc* pDoc = (CBZDoc*)GetDocument();
	DWORD dwTotal = pDoc->GetDocSize();
	if(dwTotal < (DWORD)options.nBmpWidth) return;

	if(!m_lpbi) 
		m_lpbi = (LPBITMAPINFOHEADER)MemAlloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256);

	m_lpbi->biSize = sizeof(BITMAPINFOHEADER);
	m_lpbi->biWidth = m_cBmp.cx = options.nBmpWidth;
	m_cBmp.cy = dwTotal / options.nBmpWidth;
	m_lpbi->biHeight = -m_cBmp.cy;	// top-down DIB
	m_lpbi->biPlanes = 1;
	m_lpbi->biBitCount = 8;
	m_lpbi->biCompression = BI_RGB;
	m_lpbi->biSizeImage = 0;
	m_lpbi->biClrUsed = 0;
	m_lpbi->biClrImportant = 0;

	DWORD* pRGB = (DWORD*)(m_lpbi+1);
	*pRGB++ = 0xFFFFFF;
	for_to(i, 31) *pRGB++ = 0x00FFFF;
	for_to_(i, 128-32) *pRGB++ = 0xFF0000;
	for_to_(i, 128) *pRGB++ = 0x000000;

	CDC* pDC = GetDC();
	HDC hDC = pDC->m_hDC;
	if(m_hBmp) {
		::DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}

#ifdef FILE_MAPPING
	if(!pDoc->IsFileMapping())
#endif //FILE_MAPPING
	{
		m_hBmp = ::CreateDIBitmap(hDC, m_lpbi, CBM_INIT, pDoc->GetDocPtr(), (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS);
		if(!m_hBmp)
			ErrorMessageBox();
	}

	// TODO: calculate the total size of this view
	CSize cView = m_cBmp;
	cView.cx = cView.cx * options.nBmpZoom + BMPSPACE*2;
	cView.cy = cView.cy * options.nBmpZoom + BMPSPACE*2;
	SetScrollSizes(MM_TEXT, cView);
	
	TRACE("cView.cy=%X\n", GetTotalSize().cy);

	CSplitterWnd* pSplit = (CSplitter*)GetParent();
	ASSERT(pSplit->IsKindOf(RUNTIME_CLASS(CSplitterWnd)));
	cView.cx = options.nBmpWidth * options.nBmpZoom + BMPSPACE*2 + GetSystemMetrics(SM_CXVSCROLL)+1;
	pSplit->SetColumnInfo(0, cView.cx, 0);
	// MemFree(lpbi);
}

void CBZBmpView::OnDraw(CDC* pDC)
{
	if(m_hBmp) {
		HDC hDC = pDC->m_hDC;
		HDC hDCSrc = ::CreateCompatibleDC(hDC);
		HBITMAP hBmpOld = (HBITMAP)::SelectObject(hDCSrc, m_hBmp);
		if(options.nBmpZoom == 1)
			::BitBlt(hDC, BMPSPACE, BMPSPACE, m_cBmp.cx, m_cBmp.cy, hDCSrc, 0, 0, SRCCOPY);
		else
			::StretchBlt(hDC, BMPSPACE, BMPSPACE, m_cBmp.cx * options.nBmpZoom, m_cBmp.cy * options.nBmpZoom
				 , hDCSrc, 0, 0, m_cBmp.cx, m_cBmp.cy, SRCCOPY);

		::SelectObject(hDCSrc, hBmpOld);
		::DeleteDC(hDC);
	} else {
		CBZDoc* pDoc = (CBZDoc*)GetDocument();
		ASSERT(pDoc);

		CRect rClip;
		pDC->GetClipBox(rClip);

		// TRACE("Clip=%d, %d %dx%d\n", rClip.left, rClip.top, rClip.Width(), rClip.Height());

		rClip.top -= rClip.top % options.nBmpZoom;
		rClip.bottom += rClip.bottom % options.nBmpZoom;

		int nSpaceTop = (rClip.top < BMPSPACE) ? BMPSPACE - rClip.top : 0;
		int nBottom = m_cBmp.cy * options.nBmpZoom + BMPSPACE;
		if(rClip.bottom >= nBottom)
			rClip.bottom = nBottom;

		int nBmpHeight = (rClip.Height() - nSpaceTop) / options.nBmpZoom;
		m_lpbi->biHeight = -nBmpHeight;

		DWORD dwOffset = (rClip.top - (BMPSPACE - nSpaceTop)) * m_cBmp.cx / options.nBmpZoom;
#ifdef FILE_MAPPING
		pDoc->QueryMapView(pDoc->GetDocPtr(), dwOffset);
#endif //FILE_MAPPING
		LPBYTE lpBits = pDoc->GetDocPtr() + dwOffset;

		::StretchDIBits(pDC->m_hDC, BMPSPACE, rClip.top + nSpaceTop, m_cBmp.cx * options.nBmpZoom, nBmpHeight * options.nBmpZoom
				, 0, 0, m_cBmp.cx, nBmpHeight
				, lpBits , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);

	}
}

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView diagnostics

#ifdef _DEBUG
void CBZBmpView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CBZBmpView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView message handlers

void CBZBmpView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	point += GetScrollPosition();
	point.x -= BMPSPACE;
	point.y -= BMPSPACE;
	point.x /= options.nBmpZoom;
	point.y /= options.nBmpZoom;
	if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= 0) {
		DWORD dwPtr = point.y*options.nBmpWidth + point.x;
		CBZView* pView = (CBZView*)GetNextWindow();
		if(dwPtr < pView->m_dwTotal) {
			pView->m_dwCaret = dwPtr;
			pView->GotoCaret();
			pView->Activate();
		}
	}

//	CScrollView::OnLButtonDown(nFlags, point);
}

void CBZBmpView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if(nSBCode == SB_THUMBTRACK) {		// ### 1.54
		SCROLLINFO si;
		GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS);
		TRACE("nPos, nTrackPos=%d, %d\n", nPos, si.nTrackPos);
		nPos = si.nTrackPos;
	}
	
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

// ###1.54c

void CBZBmpView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMenu menu;
	menu.LoadMenu(IDR_BMPVIEW);
	CMenu* pMenu = menu.GetSubMenu(0);
	if(options.nBmpWidth == 128)
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH128, MF_BYCOMMAND | MF_CHECKED);
	if(options.nBmpWidth == 256)
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH256, MF_BYCOMMAND | MF_CHECKED);
	if(options.nBmpZoom > 1)
		pMenu->CheckMenuItem(ID_BMPVIEW_ZOOM, MF_BYCOMMAND | MF_CHECKED);

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(0, pt.x, pt.y, this);

	CScrollView::OnRButtonDown(nFlags, point);
}

void CBZBmpView::OnBmpViewMode(UINT nID)
{
	switch(nID) {
	case ID_BMPVIEW_WIDTH128:
		options.nBmpWidth = 128;
		break;
	case ID_BMPVIEW_WIDTH256:
		options.nBmpWidth = 256;
		break;
	case ID_BMPVIEW_ZOOM:
		options.nBmpZoom = (options.nBmpZoom == 1) ? 2 : 1;

	}
	GetMainFrame()->CreateClient();
}
