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
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_BMPVIEW_WIDTH128, ID_BMPVIEW_ZOOM, OnBmpViewMode)
	ON_WM_SETCURSOR()
	ON_COMMAND_RANGE(ID_BMPVIEW_8BITCOLOR, ID_BMPVIEW_32BITCOLOR, OnBmpViewColorWidth)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView drawing

/*
void MakeBzPallet256(DWORD *pRGB)
{
	*pRGB++ = 0xFFFFFF;
	for_to(i, 31) *pRGB++ = 0x00FFFF;
	for_to_(i, 128-32) *pRGB++ = 0xFF0000;
	for_to_(i, 128) *pRGB++ = 0x000000;
}*/

void MakeRedPallet256(DWORD *pRGB)
{
	for(unsigned int i=0; i<=0xff; i++)
	{
		*pRGB++ = 0 | (i&0xff)<<16;
	}
}


void MakeSafetyPallet256(DWORD *pRGB)
{
	// safety pallet http://msdn.microsoft.com/en-us/library/bb250466%28VS.85%29.aspx
	DWORD* pRGBorig = pRGB;

//	*pRGB++ = 0xFFFFFF;
	for(unsigned int r=0; r<=0xff; r+=0x33)
		for(unsigned int g=0; g<=0xff; g+=0x33)
			for(unsigned int b=0; b<=0xff; b+=0x33)
				*pRGB++ = b|(g<<8)|(r<<16);
	for(unsigned int gr=0; gr<=0xffffff; gr+=0x111111)
		*pRGB++ = gr;
	*pRGB++ = 0xC0C0C0;
	*pRGB++ = 0x808080;
	*pRGB++ = 0x800000;
	*pRGB++ = 0x800080;
	*pRGB++ = 0x008000;
	*pRGB++ = 0x008080;
	pRGBorig[255] = 0xffffff;
//	TRACE("pallet[0]=0x%x, [255]=0x%x\n", ((DWORD*)(m_lpbi+1))[0], ((DWORD*)(m_lpbi+1))[255]);
}

void Make8bitBITMAPINFOHEADER(LPBITMAPINFOHEADER lpbi, LONG w, LONG h)
{
//	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biWidth = w;
	lpbi->biHeight = h;
	lpbi->biPlanes = 1;
	lpbi->biBitCount = options.nBmpColorWidth;//8;
	lpbi->biCompression = BI_RGB;
	lpbi->biSizeImage = 0;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
}

void CBZBmpView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CBZDoc* pDoc = (CBZDoc*)GetDocument();
	DWORD dwTotal = pDoc->GetDocSize();
	if(dwTotal < (DWORD)options.nBmpWidth) return;

	if(!m_lpbi) 
		m_lpbi = (LPBITMAPINFOHEADER)MemAlloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256/*256pallet*/);

	m_lpbi->biSize = sizeof(BITMAPINFOHEADER);
	m_cBmp.cx = options.nBmpWidth;
	m_cBmp.cy = dwTotal / (options.nBmpWidth * (options.nBmpColorWidth/8));
	Make8bitBITMAPINFOHEADER(m_lpbi, m_cBmp.cx, -m_cBmp.cy/*top-down DIB*/);


	DWORD* pRGB = (DWORD*)(m_lpbi+1);
	MakeSafetyPallet256(pRGB); //MakeBzPallet256(pRGB);
//	MakeRedPallet256(pRGB);

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

BOOL CBZBmpView::OnEraseBkgnd(CDC* pDC)
{
	return false;
}

void CBZBmpView::OnDraw(CDC* pDC)
{
	pDC->SetBkColor(RGB(255,255,255));
	CMemDC pMemDC(pDC);
	if(m_hBmp) {
		HDC hDC = pMemDC->m_hDC;
		HDC hDCSrc = ::CreateCompatibleDC(hDC);
		HBITMAP hBmpOld = (HBITMAP)::SelectObject(hDCSrc, m_hBmp);
		if(options.nBmpZoom == 1)
			::BitBlt(hDC, BMPSPACE/*dstX*/, BMPSPACE/*dstY*/, m_cBmp.cx/*dstW*/, m_cBmp.cy/*dstH*/
					, hDCSrc, 0/*srcX*/, 0/*srcY*/, SRCCOPY);
		else
			::StretchBlt(hDC, BMPSPACE/*dstX*/, BMPSPACE/*dstY*/
				, m_cBmp.cx * options.nBmpZoom/*dstW*/, m_cBmp.cy * options.nBmpZoom/*dstH*/
				, hDCSrc, 0/*srcX*/, 0/*srcY*/, m_cBmp.cx/*srcW*/, m_cBmp.cy/*srcH*/, SRCCOPY);

		::SelectObject(hDCSrc, hBmpOld);
		//::DeleteDC(hDC);
	} else {
		CBZDoc* pDoc = (CBZDoc*)GetDocument();
		ASSERT(pDoc);

		CRect rClip;
		pMemDC->GetClipBox(rClip);

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
		dwOffset*=options.nBmpColorWidth/8;
#ifdef FILE_MAPPING
		//pDoc->QueryMapView(pDoc->GetDocPtr(), dwOffset);
		DWORD dwIdeaSize = m_cBmp.cx * nBmpHeight * (options.nBmpColorWidth/8);
		pDoc->QueryMapViewTama(dwOffset, dwIdeaSize);
		ASSERT(pDoc->GetMapRemain(dwOffset) >= dwIdeaSize);
#endif //FILE_MAPPING
		LPBYTE lpBits = pDoc->GetDocPtr() + dwOffset;

		::StretchDIBits(pMemDC->m_hDC, BMPSPACE/*dstX*/, rClip.top + nSpaceTop/*dstY*/
				, m_cBmp.cx * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
				, 0/*srcX*/, 0/*srcY*/, m_cBmp.cx/*srcW*/, nBmpHeight/*srcH*/
				, lpBits/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);
		TRACE("OnDraw(Mem): x=%d, y=%d, w=%d, h=%d\n", BMPSPACE/*dstX*/, rClip.top + nSpaceTop/*dstY*/
				, m_cBmp.cx * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/);

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
		DWORD dwPtr = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
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
	switch(options.nBmpWidth)
	{
	case 128:
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH128, MF_BYCOMMAND | MF_CHECKED);
		break;
	case 256:
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH256, MF_BYCOMMAND | MF_CHECKED);
		break;
	}
	if(options.nBmpZoom > 1)
		pMenu->CheckMenuItem(ID_BMPVIEW_ZOOM, MF_BYCOMMAND | MF_CHECKED);
	switch(options.nBmpColorWidth)
	{
	case 8:
		pMenu->CheckMenuItem(ID_BMPVIEW_8BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
		break;
	case 24:
		pMenu->CheckMenuItem(ID_BMPVIEW_24BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
		break;
	case 32:
		pMenu->CheckMenuItem(ID_BMPVIEW_32BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
		break;
	}

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

BOOL CBZBmpView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
//	::SetCursor(AfxGetApp()->LoadCursor(IDC_BMPPOINTER));
//	return true;

	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CBZBmpView::OnBmpViewColorWidth(UINT nID)
{
	switch(nID)
	{
	case ID_BMPVIEW_8BITCOLOR:
		options.nBmpColorWidth = 8;
		break;
	case ID_BMPVIEW_24BITCOLOR:
		options.nBmpColorWidth = 24;
		break;
	case ID_BMPVIEW_32BITCOLOR:
		options.nBmpColorWidth = 32;
		break;
	}
	GetMainFrame()->CreateClient();
}
