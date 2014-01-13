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

#include "BZDoc2.h"
#include "BZView.h"
#include "BZFormVw.h"
#include "BZInspectView.h"
#include "BZAnalyzerView.h"
#include "MainFrm.h"
#include "ColorDlg.h"
#include "SettingDlg.h"
#include  <ctype.h>
#include  <winnls.h>
#include  <mbstring.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ADDRCOLUMNS 6
#define ADDRCOLUMNS_MAP 9
#define DUMP_X		(m_nColAddr + 2)  // 8
#define DUMP_Y		1
#define CHAR_X		(m_nColAddr + 51) // 57
#define VIEWCOLUMNS	(m_nColAddr + 68) // 74
#define PAGESKIP	16

#define TIMER_DOWN	1
#define TIMER_UP	2

#define CHAR_NG		'.'
#define CHAR_EBCDIC_NG 0xFF
#define EBCDIC_BASE 0x40
#define EBCDIC_COUNT (256 - EBCDIC_BASE)

#define MEMFILE_GROWBY 16384

//static const UINT nMsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);

static DWORD  MemCompByte2(LPCVOID p1, LPCVOID p2, DWORD len);

BOOL CBZView::m_bHexSize = FALSE;
LPSTR CBZView::m_pEbcDic = NULL;
BOOL  CBZView::m_bLoadEbcDic = FALSE;

//inline int SwapWord(int val)
inline WORD SwapWord(WORD val)
{
	if(options.bByteOrder) {
		 val = _byteswap_ushort(val);
/*		_asm {
			mov eax, val
			xchg al,ah
			mov val, eax
		}*/
	}
	return val;
}

//inline int SwapDword(int val)
inline DWORD SwapDword(DWORD val)
{
	if(options.bByteOrder) {
		 val = _byteswap_ulong(val);
/*		_asm {
			push val
			pop ax
			pop bx
			xchg al,ah
			xchg bl,bh
			push ax
			push bx
			pop val
		}*/
	}
	return val;
}

inline ULONGLONG SwapQword(ULONGLONG val)
{
	if(options.bByteOrder) {
		val = ((val & 0xFF00000000000000ull)>>56) | ((val & 0x00FF000000000000ull)>>40) | ((val & 0x0000FF0000000000ull)>>24) | ((val & 0x000000FF00000000ull)>>8)
			| ((val & 0x00000000FF000000ull)<<8) | ((val & 0x0000000000FF0000ull)<<24) | ((val & 0x000000000000FF00ull)<<40) | ((val & 0x00000000000000FFull)<<56);
	}
	return val;
}




void CBZView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ATLTRACE("CBZView::OnUpdate()\n");
	
	m_pDoc = GetDocument();
	ASSERT_VALID(m_pDoc);
//#ifdef FILE_MAPPING
//	if(m_pDoc)
//		m_pDoc->QueryMapViewTama2(0, 1);//ファイルサイズが０の場合で、デフォルト設定ではない場合、ファイルマッピングモードで開くとエラー
	m_nColAddr = (options.bDWordAddr) ? ADDRCOLUMNS_MAP : ADDRCOLUMNS;
	if(GetViewWidth() != VIEWCOLUMNS) {
		SetViewSize(CSize(VIEWCOLUMNS, 0));
		m_bResize = FALSE;
		// ResizeFrame();
	}
//#else
//	m_nColAddr = ADDRCOLUMNS;
//#endif //FILE_MAPPING

	UpdateDocSize();
	ScrollToPos(CPoint(0,0));
	SetScrollHome(CPoint(0, DUMP_Y));
	m_dwCaret = m_dwOldCaret = m_dwBlock = m_dwStruct = 0;
	m_nMember = INVALID;
	m_nBytes = 1;
	m_bCaretOnChar = m_bBlock = m_bIns = FALSE;
	m_bEnterVal = FALSE;
	m_charset = (options.bAutoDetect) ? AutoDetectCharSet() : options.charset;
	if(m_charset == CTYPE_EBCDIC)
		LoadEbcDicTable();

	CMainFrame *pMainFrame = GetMainFrame();
	if(pMainFrame)
	{
		if(pMainFrame->m_bStructView) {
			CBZFormView* pView = (CBZFormView*)GetNextWindow(GW_HWNDPREV);
			if(pView!=NULL)
				pView->SelectTag();
		}
		if(pMainFrame->m_bAnalyzerView) {
			CBZAnalyzerView* pView = (CBZAnalyzerView*)GetNextWindow(GW_HWNDPREV);
			if(pView!=NULL)
				pView->Clear();
		}
	}
	Invalidate();
	InitCaret();
	DrawCaret();
}


CBZDoc2* CBZView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBZDoc2)));
	return (CBZDoc2*)m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////
// CBZView drawing

BOOL CBZView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	// TODO: call DoPreparePrinting to invoke the Print dialog box
	if(m_nPageLen)
		SetMaxPrintingPage(pInfo);
	return CTextView::OnPreparePrinting(pInfo);
}

void CBZView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CTextView::OnBeginPrinting(pDC, pInfo);

	CRect rMargin;
	GetMargin(rMargin, pDC);
	POINT pt;
	pt.x = pDC->GetDeviceCaps(HORZRES) - rMargin.left - rMargin.right;
	pt.y = pDC->GetDeviceCaps(VERTRES) - rMargin.top - rMargin.bottom;
	PixelToGrid(pt);
	m_nPageLen = pt.y-1;
	SetMaxPrintingPage(pInfo);
}

void CBZView::SetMaxPrintingPage(CPrintInfo* pInfo)
{
  UINT64 dwTotal = GetFileSize();
	UINT64 dwSize64 = (m_bBlock) ? BlockEnd()-(BlockBegin()&~(16-1)) : dwTotal;
  UINT64 maxPage64 = dwSize64/16/ m_nPageLen + 1;
  pInfo->SetMaxPage(maxPage64<=0xFFffFFff ? (DWORD)maxPage64 : 0xFFffFFff);
}

void CBZView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_dwPrint = (m_bBlock) ? (BlockBegin()&~(16-1)) : 0;
	m_dwPrint += (pInfo->m_nCurPage - 1) * m_nPageLen * 16;
	CView::OnPrint(pDC, pInfo);
}

void CBZView::SetColor(TextColor n)
{
	CTextView::SetColor(options.colors[n][0], options.colors[n][1]);
}

void CBZView::SetHeaderColor()
{
	BOOL bNoFocus = GetMainFrame()->m_nSplitView && GetMainFrame()->GetActiveDocument() != m_pDoc;
	SetColor(bNoFocus ? TCOLOR_ADDRESS2 : TCOLOR_ADDRESS);
}


/////////////////////////////////////////////////////////////////////////////
// CBZView OnDraw

void CBZView::DrawToFile(CFile* pFile) 	// ###1.63
{
	m_pFile = pFile;
	OnDraw(NULL);
	m_pFile = NULL;
}

void CBZView::OnDraw(CDC* pDC)
{
	if(pDC)
		CTextView::OnDraw(pDC);

	// TODO: add draw code for native data here
	//LPBYTE p  = m_pDoc->GetDocPtr();
//	if(!p) return;

	RECT rClip;
	UINT64 ofs;
	UINT64 dwBegin = BlockBegin();
	UINT64 dwEnd   = BlockEnd();
	UINT64 dwTotal = GetFileSize();

	UINT64 dwTotalP1 = 0;

	int y;

	PutBegin(pDC);
	if(IsToFile()) {
		DrawHeader();
		rClip.y1 = DUMP_Y;
		rClip.y2 = LONG_MAX;
		ofs = 0;
		if(m_bBlock) {
			ofs = dwBegin & ~0xF;
			dwTotal = dwEnd;
		}
	} else if(pDC->IsPrinting()) {		// ### 1.54
		DrawHeader();
		rClip.y1 = DUMP_Y;
		rClip.y2 = m_nPageLen;
		ofs = m_dwPrint;
	} else {
		pDC->GetClipBox(&rClip);
		PixelToGrid(rClip);
		if(rClip.y1 < DUMP_Y) {
			DrawHeader();
			rClip.y1 += DUMP_Y;
		}
		POINT ptOrg = GetScrollPos();
		ofs = (ptOrg.y + rClip.y1 - DUMP_Y)*16;
	}
	if(!IsToFile() && (pDC->IsPrinting() && m_bBlock))
		dwTotal = dwEnd;
	//LPBYTE p1 = NULL;
	CBZView* pView1 = GetBrotherView();
  CBZDoc2 *pDoc1 = NULL;
	if(pView1 /*&& (m_dwTotal == pView1->m_dwTotal)*/)
	{
		//p1 = pView1->m_pDoc->GetDocPtr();
    pDoc1 = pView1->m_pDoc;
		dwTotalP1 = pView1->GetFileSize();
	}

//	if(p && !(p = m_pDoc->QueryMapView(p, ofs))) return;
	InitCharMode(ofs);
  m_pDoc->Cache(ofs, 1000);
  if(pDoc1)pDoc1->Cache(ofs, 1000);

	for(/*int */y = rClip.y1; y <= rClip.y2; y++) {
		Locate(0, y);
		if(ofs > dwTotal) break;
		if(IsToFile() && options.nDumpPage) {
			if(y > DUMP_Y && ((y - DUMP_Y) % options.nDumpPage) == 0) {
				Locate(0, y); // crlf
				DrawHeader();
				Locate(0, y); // crlf
			}
		}
		SetHeaderColor();
		UINT64 ofs1 = ofs + m_pDoc->m_dwBase;
#ifdef FILE_MAPPING
//		if(p && !(p = m_pDoc->QueryMapView(p, ofs1))) return;
	//	if(p1 && !(p1 = pView1->m_pDoc->QueryMapView(p1, ofs1))) return;

    //描画: 先頭のアドレス
		if(m_nColAddr > ADDRCOLUMNS) {
			PutFormatStr("%04X:%04X", HIWORD(ofs1), LOWORD(ofs1));
			SetColor();
			PutStr("  ");
		} else
#endif //FILE_MAPPING
		{
			if(ofs1 < 0x1000000) {			// ###1.54
				PutFormatStr("%06X", ofs1);
				SetColor();
				PutStr("  ");
			} else {
				PutFormatStr("%07X", ofs1);
				SetColor();
				PutStr(" ");
			}
		}
    //描画: Hex表示 (x16バイト)
		UINT64 ofs0 = ofs;
		for_to(i,16) {
			if(ofs >= dwTotal) {
				SetColor();
				PutChar(' ', (16-i)*3-1); 
				break;
			}
      LPBYTE ppppppp = m_pDoc->CacheForce(ofs, 1);
      if(!ppppppp)return;
      LPBYTE ppppppp1 = NULL;
      if(pDoc1)ppppppp1 = pDoc1->CacheForce(ofs, 1);

			if(m_bBlock && ofs >= dwBegin && ofs < dwEnd)
				SetColor(TCOLOR_SELECT);
			else if(m_pDoc->CheckMark(ofs))
				SetColor(TCOLOR_MARK);
			else if(m_dwStruct && ofs >= m_dwStructTag && ofs < m_dwStruct) {
				if(m_nMember >= 0 && ofs >= m_dwStructTag + m_nMember && ofs < m_dwStructTag + m_nMember + m_nBytes * m_nBytesLength)
					SetColor(TCOLOR_MEMBER);
				else
					SetColor(TCOLOR_STRUCT);
			}
			else if(ppppppp1 && (ofs < dwTotalP1 && (*ppppppp != *ppppppp1)))
				SetColor(TCOLOR_MISMATCH);
			else if(ppppppp1 && ofs >= dwTotalP1)
				SetColor(TCOLOR_OVERBROTHER);
			else
				SetColor();
			PutFormatStr("%02X", *ppppppp);
      ofs++;
			if(i < 15)
				PutChar((i==7) ? '-' : ' ');
		}
		SetColor();
		PutStr("  ");
		ofs = ofs0;

#if 0//#ifdef _UNICODE
		wchar_t wcConv;
		char chConv[3]={0};
		bool fPutSkip = FALSE;
#endif

    //描画: 右側文字表示
		for_to_(i,16) {
			if(ofs >= dwTotal || (m_charset == CTYPE_UNICODE && ofs+1 >= dwTotal)) {
				SetColor();
				PutChar(' ', 16-i);
				ofs++;
				break;
			}
			if(m_bBlock && ofs >= dwBegin && ofs < dwEnd)
				SetColor(TCOLOR_SELECT);
			else if(m_dwStruct && ofs >= m_dwStructTag && ofs < m_dwStruct) {	// ###1.62
				if(m_nMember >= 0 && ofs >= m_dwStructTag + m_nMember && ofs < m_dwStructTag + m_nMember + m_nBytes * m_nBytesLength)
					SetColor(TCOLOR_MEMBER);
				else
					SetColor(TCOLOR_STRUCT);
			}
			else
				SetColor();

      if(m_charset == CTYPE_UNICODE) {
        LPWORD pW = (LPWORD)m_pDoc->CacheForce(ofs, 2);
        if(!pW)return;
				WORD w = SwapWord(*pW);	// ### 1.54a
				PutUnicodeChar(w);
				ofs += 2;
				i++;
			} else {
        LPBYTE pB = NULL;
        DWORD dwB = 3;
        for(;dwB>0;dwB--)
        {
          pB = m_pDoc->CacheForce(ofs, dwB);//3,2,1バイト確保（可能なら多いほうが良い）
          if(pB)break;
        }
        if(!pB)return;
				WORD c = *pB;//注意: WORDだけどpがLPBYTEなので1バイトしか格納されていない
        ofs++;
				switch(m_charset) {
				case CTYPE_ASCII:
					if(c < 0x20 || c > 0x7E) c = CHAR_NG;
					break;
				case CTYPE_SJIS:
				{
					if(c < 0x20) c = CHAR_NG;
					else if(_ismbblead(c)/*IsMBS(p, ofs-1, FALSE)*/ && dwTotal > ofs && (dwB>=2 && _ismbbtrail(*(pB+1))))
					{
						BYTE c1 = *(pB+1);
						if(_ismbclegal(MAKEWORD(c1, c))) {
							PutChar((char)c);
							c = *(pB+1);
							if(i < 15) {
								ofs++;
								i++;
							}
						} else
							c = CHAR_NG;
					}
					else if(i == 0 && _ismbbtrail(c)/*IsMBS(p, ofs-1, TRUE)*/)
						 c=' ';
					else if((c > 0x7E && c < 0xA1) || c > 0xDF)
						c = CHAR_NG;
					break;
				}
				case CTYPE_JIS:
				case CTYPE_EUC:
				case CTYPE_EPWING:
					c = GetCharCode(c, ofs - 1);
					if(c < 0x20 || (c > 0x7E && c < 0xA1) || (c > 0xDF && c <= 0xFF)) c = CHAR_NG;
					else if(c & 0xFF00 && dwB>=2) {
						BYTE c1 = *(pB+1);
						c = (c | c1) & 0x7F7F;
						if(i < 15) {
							GetCharCode(c1, ofs);
							ofs++;
							i++;
						}
						c = _mbcjistojms(c);
						if(!c) c = '..';
#if 1//#ifndef _UNICODE
						PutChar(HIBYTE(c));//!""""""""!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						c &= 0xFF;
#else
						chConv[0] = HIBYTE(c);
						chConv[1] = c&0xFF;
						chConv[2] = NULL;
						PutStr(CA2WEX<4>(chConv));
						fPutSkip = TRUE;
#endif
					}
					break;
				case CTYPE_UTF8:	// ### 1.54b
					if(c < 0x20) c = CHAR_NG;
					else if(c & 0x80) {
						if(!(c & 0x40)) c = CHAR_NG;
						else {
							WORD w = 0;
							if(!(c & 0x20)) {	// U+0080..U+07FF 00000xxx:xxyyyyyy 110xxxxx 10yyyyyy
								if(dwB>=2) c = CHAR_NG;
								else {
									if(((*(pB+1))&0xC0/*11000000*/) != 0x80/*10000000*/)
									{
										c = CHAR_NG;
									} else {
										w = ((c & 0x1F) << 6) | ((*(pB+1)) & 0x3F);
										if(++i < 16) ofs++;
									}
								}
							} else  {			// U+0800..U+FFFF xxxxyyyy:yyzzzzzz 1110xxxx 10yyyyyy 10zzzzzz 
								if(dwB>=3) c = CHAR_NG;
								else {
									if(((*(pB+1))&0xC0) != 0x80 || ((*(pB+2))&0xC0) != 0x80)
									{
										c = CHAR_NG;
									} else {
										w = ((c & 0x0F) << 12) | (((*(pB+1)) & 0x3F) << 6) | ((*(pB+2)) & 0x3F);
										if(++i < 16) ofs++;
										if(++i < 16) ofs++;
									}
								}
							}
							if(w) {
								PutUnicodeChar(w);
								if(w & ~0x07FF && i < 16) {
									c = CHAR_NG;
								} else
									continue;
							}
						}
					}
					break;
				case CTYPE_EBCDIC:	// ### 1.63
					if(m_pEbcDic == NULL || c < EBCDIC_BASE)
						c = CHAR_NG;
					else
						c = *(m_pEbcDic + ((BYTE)c - EBCDIC_BASE));
					break;
				}
#if 0//#ifdef _UNICODE
				if(fPutSkip)
				{
					fPutSkip=FALSE;
				} else {
					chConv[0] = (char)c;
					chConv[1] = chConv[2] = NULL;
					PutStr(CA2WEX<4>(chConv));
				}
#else
				PutChar((char)c);
#endif
			}
		}
		SetColor();
//		if(m_charset == CTYPE_SJIS)
			PutChar(' ');
	}
	if(IsToFile()) {
		PutEnd();
	} else {
		if(y <= rClip.y2) {
			SetColor();
			for(y; y<=rClip.y2+1; y++) {
				Locate(0, y);
				PutChar(' ', VIEWCOLUMNS);
			}
		}
		PutEnd();
		DrawDummyCaret(pDC);
		DrawCaret();
	}

	/* Grid表示 */
	if(!IsToFile() && options.iGrid==1) {
		DrawGrid(pDC, rClip);
	}
}

void CBZView::DrawGrid(CDC* pDC, RECT& rClip)
{
	int OldBkMode = pDC->GetBkMode();
	pDC->SetBkMode(TRANSPARENT);
	CPen redSolid(PS_SOLID, 1, options.colors[TCOLOR_HORIZON][0]/*RGB(0xe2,0x04,0x1b)*/);
	CPen redDot(PS_SOLID, 1, options.colors[TCOLOR_HORIZON][1]/*RGB(0x3e,0xb3,0x70)*/);
	HGDIOBJ penOld = pDC->SelectObject(redSolid);
	CPoint ptScroll = GetScrollPos();

	int startLine = ptScroll.y + rClip.top;
	int endLine = ptScroll.y + rClip.bottom;
	int gridHeightNum = 16;
	int startGridLine;
	if(startLine % gridHeightNum == 0)startGridLine = startLine;
	else startGridLine = startLine - startLine % gridHeightNum;
	int gridLineWidth = m_cell.cx*(16*4+12);
	int minusTop = ptScroll.y*m_cell.cy;
	int kirikae=0;
	for(; startGridLine <= endLine; startGridLine += gridHeightNum/2) {
		//		TRACE("Grid: startGridLine=%d, y=%d\n", startGridLine, (startGridLine+1)*m_cell.cy);
		if(kirikae)pDC->SelectObject(redDot);
		else pDC->SelectObject(redSolid);
		kirikae=!kirikae;
		pDC->MoveTo(0, (startGridLine+1)*m_cell.cy - minusTop - 1);
		pDC->LineTo(gridLineWidth, (startGridLine+1)*m_cell.cy - minusTop - 1);
	}
	pDC->SelectObject(penOld);
	pDC->SetBkMode(OldBkMode);
}

void CBZView::PutUnicodeChar(WORD w)
{
  if(w==0x3F)
  {
    PutStr("? ");
    return;
  }
	WCHAR wbs[3] = {0};
	wbs[0] = w;
	wbs[1] = NULL;
#if 1//#ifndef _UNICODE
	char  mbs[4];
	int len = ::WideCharToMultiByte(CP_ACP, 0, wbs, -1, mbs, 3, NULL, NULL);
	if(len <= 1 || mbs[0] == '?' || wbs[0] < 0x20)
		PutStr(". ");
	else {
		if(mbs[1] == 0) {
			mbs[1] = ' '; mbs[2] = 0;
		}
		PutStr(mbs);
	}
#else
	if(wbs[0] < 0x20)
		PutStr(_T(". "));
	else {
		CStringA tmp(wbs);
		if(tmp.GetLength()==1)
		{
			wbs[1]=' ';
			wbs[2]=NULL;
		}
/*		if(mbs[1] == 0) {
			mbs[1] = ' '; mbs[2] = 0;
		}*/
		PutStr(wbs);
	}
#endif
}

void CBZView::DrawHeader()
{
	if(IsToFile() && !options.sDumpHeader.IsEmpty()) {
		m_pFile->Write(options.sDumpHeader, options.sDumpHeader.GetLength());
	} else {
		SetHeaderColor();
		Locate(DUMP_X,0);
		for_to(i,16)
			PutFormatStr("+%1X ", i);
		PutChar();
		for_to_(i,16)
			PutFormatStr("%1X", i);
	}
}

void CBZView::DrawDummyCaret(CDC* pDC)
{
	if(GetFocus() != this) {
		POINT ptOrg = GetScrollPos();
		DWORD dwOrg = ptOrg.y * 16;

		RECT rClient;
		GetClientRect(&rClient);
		PixelToGrid(rClient);
		DWORD dwBottom = dwOrg + (rClient.y2 - DUMP_Y) * 16;
		UINT64 dwMax = GetFileSize() + 1;
		if(dwBottom > dwOrg && dwBottom < dwMax)	// ###1.61
			dwMax = dwBottom;
		POINT pt;
		LONG ptx2 = -1;	// ###1.62
		if(m_dwCaret < dwOrg || m_dwCaret >= dwMax) {
		} else {
			pt.x = (m_dwCaret - dwOrg)%16;
			ptx2 = pt.x + CHAR_X;
			pt.x = pt.x*3 + DUMP_X;
			if(m_bCaretOnChar) Swap(pt.x, ptx2);
			pt.x -= ptOrg.x;
			ptx2 -= ptOrg.x;
			pt.y = (m_dwCaret - dwOrg)/16 + DUMP_Y;

			GridToPixel(pt);
			CRgn rgn1;
			rgn1.CreateRectRgn(pt.x, pt.y, pt.x+m_cell.cx, pt.y+m_cell.cy);
			InvertRgn(pDC->m_hDC, rgn1);
		}
	}
}

BOOL CBZView::DrawCaret()
{
	BOOL bDraw = FALSE;
	POINT ptOrg = GetScrollPos();
	DWORD dwOrg = ptOrg.y * 16; //表示開始オフセット (最左上)

	RECT rClient;
	GetClientRect(&rClient);
	PixelToGrid(rClient);
  DWORD dwBottom = (dwOrg + (rClient.y2 - DUMP_Y) * 16) + 16; //下が欠けた最下ラインのカレットが表示されないので+16
	UINT64 dwMax = GetFileSize() + 1;
  UINT64 dwMax2 = dwMax;
	if(dwBottom > dwOrg && dwBottom < dwMax)	// ###1.61
  {
		dwMax = dwBottom;
    dwMax2 = dwBottom-16;
  }

	POINT pt;
	LONG ptx2 = -1;	// ###1.62
	if(m_dwCaret < dwOrg || m_dwCaret >= dwMax) {
		pt.x = pt.y = -1;
	} else {
		pt.x = (m_dwCaret - dwOrg)%16;
		ptx2 = pt.x + CHAR_X;
		pt.x = pt.x*3 + DUMP_X;
		if(m_bCaretOnChar) Swap(pt.x, ptx2);
		pt.x -= ptOrg.x;
		ptx2 -= ptOrg.x;
		pt.y = (m_dwCaret - dwOrg)/16 + DUMP_Y;
    if(m_dwCaret >= dwMax2)ScrollBy(0, 1, !m_bBlock);
		bDraw = TRUE;
	}
	MoveCaret(pt);
	pt.x = ptx2;
	MoveCaret2(pt);

	
	if(GetMainFrame() && GetMainFrame()->m_bInspectView) {
		CBZInspectView* pView = (CBZInspectView*)GetNextWindow(GW_HWNDPREV);
		pView->Update();
	}

	return bDraw;
}

UINT64 CBZView::PointToOffset(CPoint pt)
{
	CRect r;
	GetClientRect(&r);
	if(!r.PtInRect(pt))
		return UINT_MAX;
	PixelToGrid(pt);
	pt += GetScrollPos();
	if(pt.y < DUMP_Y) // || pt.x == CHAR_X-1 || pt.x >= CHAR_X+16)	// ###1.5
 		return UINT_MAX;
	if(pt.x >= CHAR_X) {
		if((pt.x -= CHAR_X) > 16) pt.x = 16;
		m_bCaretOnChar = TRUE;	
	} else {
		if(pt.x < DUMP_X)
			pt.x = 0;
		else
			pt.x -= DUMP_X;
		pt.x/=3;
		m_bCaretOnChar = FALSE;
	}
	UINT64 ofs = (pt.y - DUMP_Y)*16 + pt.x;
  UINT64 dwTotal = GetFileSize();
	return (ofs > dwTotal) ? dwTotal : ofs;
}

BOOL CBZView::GotoCaret()
{
	if(DrawCaret()) {
		if(m_bBlock)
			Invalidate(FALSE);
		return TRUE;
	}
	POINT pt;
	pt.x = 0;
	pt.y = m_dwCaret/16 - 1;
	ScrollToPos(pt);
	Invalidate(FALSE);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBZView message handlers

void CBZView::Activate()
{
	GetMainFrame()->SetActiveView(this);
	if(GetFocus() != this) SetFocus();	// ###1.62
}


void CBZView::OnDoubleClick()	// ### 1.62
{
	if(m_dwStruct) {
		CBZFormView* pView = (CBZFormView*)GetNextWindow(GW_HWNDPREV);
		pView->SelchangeListTag();
	} else {
		OnStatusInfo();
	}
}





void CBZView::MoveCaretTo(UINT64 dwNewCaret)
{
  UINT64 dwTotal = GetFileSize();
	if(dwNewCaret > dwTotal) {
		dwNewCaret = dwTotal;
	}

	int dy = dwNewCaret/16 - m_dwCaret/16;
	m_dwCaret = dwNewCaret;
	if(!DrawCaret()) {
		ScrollBy(0, dy, !m_bBlock);
	}
	if(m_bBlock)
		Invalidate(FALSE);

	
}

void CBZView::UpdateDocSize()
{
	//m_dwTotal = m_pDoc->GetDocSize();
  UINT64 dwTotal = GetFileSize();
	SIZE cTotal;
	cTotal.cx = VIEWCOLUMNS;
	cTotal.cy = dwTotal / 16 + 2;
	SetTextSize(cTotal, PAGESKIP);
	Invalidate(FALSE);
}


void CBZView::ChangeFont(LOGFONT& logFont)
{
	delete m_pFont;
	m_pFont = new CFont;
	m_pFont->CreateFontIndirect(&logFont);
	OnChangeFont();
	m_bResize = FALSE;
	Invalidate(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CBZView Update UI






void CBZView::UpdateStatusInfo()
{
	GetMainFrame()->m_wndStatusBar.SetPaneText(1, GetStatusInfoText());
}


CString CBZView::GetStatusInfoText()
{
	CString sResult;
  UINT64 dwTotal = GetFileSize();
	if(dwTotal) {
		LPCTSTR pFmtHexa;
		int val;
		if(m_bBlock) {
			if(m_nColAddr > ADDRCOLUMNS) 
				sResult.Format(_T("%04X:%04X-%04X:%04X"), HIWORD(BlockBegin()), LOWORD(BlockBegin()), HIWORD(BlockEnd()), LOWORD(BlockEnd()));  // ###1.61
			else
				sResult.Format(_T("%06X-%06X"), BlockBegin(), BlockEnd());
			val = BlockEnd() - BlockBegin();
			pFmtHexa = _T(" 0x%X(%s) bytes");

			CString sValue;
			sValue.Format(pFmtHexa, val, (LPCTSTR)SeparateByComma(val));
			sResult += sValue;
		} else if(m_nBytes<=4) {
			static TCHAR szFmtHexa[] = _T(": 0x%02X (%s)");
			if(m_nColAddr > ADDRCOLUMNS)
				sResult.Format(_T("%04X:%04X"), HIWORD(m_dwCaret), LOWORD(m_dwCaret));
			else
				sResult.Format(_T("%06X"), m_dwCaret);
//#ifdef FILE_MAPPING
			//if(m_pDoc->IsOutOfMap(m_pDoc->GetDocPtr() + m_dwCaret)) return sResult;
      if(m_dwCaret+m_nBytes >= dwTotal)return sResult;
//#endif //FILE_MAPPING
			val = GetValue(m_dwCaret, m_nBytes);
			szFmtHexa[6] = '0'+ m_nBytes * 2;
			pFmtHexa = szFmtHexa;

			CString sValue;
			sValue.Format(pFmtHexa, val, (LPCTSTR)SeparateByComma(val));
			sResult += sValue;
		} else if(m_nBytes==8) {
			if(m_nColAddr > ADDRCOLUMNS)
				sResult.Format(_T("%04X:%04X"), HIWORD(m_dwCaret), LOWORD(m_dwCaret));
			else
				sResult.Format(_T("%06X"), m_dwCaret);
//#ifdef FILE_MAPPING
			//if(m_pDoc->IsOutOfMap(m_pDoc->GetDocPtr() + m_dwCaret)) return sResult;
      if(m_dwCaret+8 >= dwTotal)return sResult;
//#endif //FILE_MAPPING
			ULONGLONG qval = GetValue64(m_dwCaret);

			CString sValue;
			sValue.Format(_T(": 0x%016I64X (%s)"), qval, (LPCTSTR)SeparateByComma64(qval));
			sResult += sValue;
		}
	}
	return sResult;
}




int CBZView::GetValue(UINT64 dwOffset, int bytes)
{
  unsigned char readBuf[4] = {0};
	int val = 0;
  if(!m_pDoc->Read(readBuf, dwOffset, bytes)) return 0;
  LPBYTE lpStart = readBuf;
	if(dwOffset + bytes > GetFileSize())
		val = 0;
	else {
		switch(bytes) {
			case 1: val = *lpStart; break;
			case 2: val = SwapWord(*(WORD*)lpStart); break;
			case 4: val = SwapDword(*(DWORD*)lpStart); break;
		}
	}
	return val;
}
DWORD CBZView::GetDWORD(UINT64 dwOffset)
{
  DWORD dwRead = 0xBABABABA;
	if(dwOffset + 4 > GetFileSize() || !m_pDoc->Read(&dwRead, dwOffset, 4))return 0;
  return SwapDword(dwRead);
}
WORD  CBZView::GetWORD(UINT64 dwOffset)
{
  WORD wRead = 0xBABA;
	if(dwOffset + 2 > GetFileSize() || !m_pDoc->Read(&wRead, dwOffset, 2))return 0;
  return SwapWord(wRead);
}
BYTE  CBZView::GetBYTE(UINT64 dwOffset)
{
  BYTE ucRead = 0xBA;
	if(dwOffset + 1 > GetFileSize() || !m_pDoc->Read(&ucRead, dwOffset, 1))return 0;
  return ucRead;
}

ULONGLONG CBZView::GetValue64(UINT64 dwOffset)
{
	ULONGLONG val = 0;
  if(!m_pDoc->Read(&val, dwOffset, 8)) return 0;
	if(dwOffset + 8 > GetFileSize())
		val = 0;
	else {
		val = SwapQword(val);
	}
	return val;
}

//void CBZView::SetValue(DWORD dwOffset, int bytes, int val)
//{
//	LPBYTE lpStart = m_pDoc->QueryMapViewTama2(dwOffset, bytes); //LPBYTE p  = m_pDoc->GetDocPtr();
//	if(!lpStart || m_pDoc->GetMapRemain(dwOffset) < bytes /*|| dwOffset + bytes > m_dwTotal*/)return;
//	m_pDoc->StoreUndo(dwOffset, bytes, UNDO_OVR);
//	SetValue(lpStart, bytes, val);
//	Invalidate(FALSE);
//	if(m_dwStruct) {
//		CBZFormView* pView = (CBZFormView*)GetWindow(GW_HWNDFIRST);
//		pView->OnSelchangeListTag();
//	}
//}
//
//void  CBZView::SetValue(LPBYTE p, int bytes, int val)
//{
//	switch(bytes) {
//		case 1: *p = val; break;
//		case 2: *(WORD*)p = SwapWord(val); break;
//		case 4: *(DWORD*)p = SwapDword(val); break;
//	}
//}

/////////////////////////////////////////////////////////////////////////////
// CBZView Jump




void CBZView::JumpTo(UINT64 dwNewCaret)
{
	BOOL bShift = (::GetKeyState(VK_SHIFT) < 0);
	if(!m_bBlock && bShift) {
		m_dwBlock = m_dwCaret;
		m_bBlock = TRUE;
	} else if(m_bBlock && !bShift) {
		m_bBlock = FALSE;
		Invalidate(FALSE);
	}
	m_dwOldCaret = m_dwCaret;
	m_dwCaret = dwNewCaret;
	GotoCaret();
}




//戻り値
//全部一緒だと0xFFFFffff
//違うところがあるとオフセットを返す
static DWORD MemCompByte2(const BYTE *p1, const BYTE *p2, DWORD len)
{
	const BYTE *p3 = p1;
	while(*p3++ == *p2++)
	{
		if(--len == 0)return 0xFFFFffff;
	}
	return p3-p1;
}

BOOL CBZView::CalcHexa(LPCSTR sExp, long& n1)
{
	LPCSTR p = sExp;
	n1 = 0;
	long n2;
	char op = 0;
	int  base = 16;
	BOOL flagHex=false;

	for(;;) {
		while(*p > 0 && *p <= ' ') p++;
		if(!*p) break;
		if(*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X'))
		{
			flagHex = true;
			p+=2;
		}
		LPCSTR p0 = p;
		if(*p == '-')
			n2 = strtol(p, (char**)&p, flagHex?16:base);
		else
			n2 = strtoul(p, (char**)&p, flagHex?16:base);
		base = 16;
		if(p != p0) {
			if(flagHex)flagHex=false;
			switch (op) {
			case 0:n1=n2;break;
			case '+': n1 += n2; break;
			case '-': n1 -= n2; break;
			case '*': n1 *= n2; break;
			case '/': n1 /= n2; break;
			case '&': n1 &= n2; break;
			case '|': n1 |= n2; break;
			case '^': n1 ^= n2; break;
			}
			op = 0;
		} else {
			char c = *p++;
			if(c == '%')
				base = 10;
			else
				op = c;
		}
	}
	if(op) {
		CString sMsg;
		sMsg.Format(IDS_ERR_SYNTAX, sExp);
		AfxMessageBox(sMsg);
		return FALSE;
	}
	return TRUE;
}
int CBZView::ReadHexa(LPCSTR sHexa, LPBYTE& buffer)
{
	LPCSTR p = sHexa;
	LPBYTE pFind = NULL;
	int nFind = 0;

	for(;;) {
		while(*p && !isalnum(*p)) p++;
		if(!*p) break;
		LPCSTR p0 = p;
		BYTE n = (BYTE)strtol(p, (char**)&p, 16);
		if(n == 0 && p == p0) break;				// ### 1.54
		nFind ++;
		if(!pFind) pFind = (LPBYTE)MemAlloc(nFind);
		else pFind = (LPBYTE)MemReAlloc(pFind, nFind);
		pFind[nFind-1] = n;
	}
	buffer = pFind;
	return nFind;
}

/////////////////////////////////////////////////////////////////////////////
// CBZView Edit



void CBZView::CutOrCopy(CutMode mode)
{
	UINT64 dwPtr  = BlockBegin();
	DWORD dwSize = BlockEnd() - dwPtr;
	if(mode != EDIT_DELETE)
  {
		if(!m_pDoc->CopyToClipboard(dwPtr, dwSize))
    {
      if(mode==EDIT_CUT)
      {
        ATLASSERT(FALSE);
        return;//Failed
      }
    }
  } else if(mode != EDIT_COPY) {
    if(m_pDoc->Delete(dwPtr, dwSize))
    {
      m_dwCaret = m_dwOldCaret = dwPtr;
      m_bBlock = FALSE;
      GotoCaret();
      UpdateDocSize();
    }
	}
}

void CBZView::FillValue(int val)
{
  UINT64 dwStart, dwEnd;
  DWORD dwSize;

  if(m_bBlock)
  {
    dwStart  = BlockBegin();
    dwEnd = BlockEnd();
  } else {
    dwStart = m_dwCaret;
    dwEnd = dwStart + m_nBytes;
  }
  dwSize = dwEnd - dwStart;

  BYTE ucData[4] = {0};
	switch(m_nBytes) {
		case 1: ucData[0] = val; break;
		case 2: *((WORD*)ucData) = SwapWord(val); break;
		case 4: *((DWORD*)ucData) = SwapDword(val); break;
    default: ATLASSERT(FALSE); return;
	}

  m_pDoc->Fill(ucData, m_nBytes, dwStart, dwSize);
	Invalidate(FALSE);
}







/////////////////////////////////////////////////////////////////////////////
// CBZView Double View

void CBZView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class

	CMainFrame* pFrame = GetMainFrame();
	if(pFrame) {
		if(bActivate) {
			CBZDocTemplate* pTmpl = (CBZDocTemplate*)m_pDoc->GetDocTemplate();
			pTmpl->SetDocument(m_pDoc);
			pFrame->OnUpdateFrameTitle();
		}
		if(pFrame->m_nSplitView)
			Invalidate(FALSE);
	}
	CTextView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

_inline UINT64 CBZView::GetRemainFromCurret() { return GetFileSize() - m_dwCaret; }


CBZView* CBZView::GetBrotherView()
{
	return (CBZView*)GetMainFrame()->GetBrotherView(this);
}

CBZDoc2* CBZView::GetBrotherDoc()
{
  CBZView *pBrotherView = GetBrotherView();
  if(!pBrotherView)return NULL;
  return pBrotherView->m_pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// CBZView Character code

/*BOOL CBZView::IsMBS(LPBYTE pTop, DWORD ofs, BOOL bTrail)
{
	LPBYTE p, p1;
	p = p1 = pTop+ofs;
	while(pTop < p) {
		if(*(p-1) < 0x81) break;
		p--;
	}
	return bTrail ? _ismbstrail(p, p1) : _ismbslead(p, p1);
}*/

void CBZView::InitCharMode(UINT64 ofs)
{
	//if(!pTop) return;
	UINT64 n = ofs;
	//LPBYTE p = pTop + n;
	if(m_charset == CTYPE_JIS) {
		GetCharCode(0x0F);
		while(n) {
			n--;
//			BYTE c = *(--p);
      //BYTE c;
      //if(!m_pDoc->Read(&c, n, 1))return;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
      if(!pC)return;
			if(*pC < 0x21 || *pC > 0x7E) break;
		}
		while(n++ < ofs) {
      //WORD w = 0;
      //if(!m_pDoc->Read(&w, n, 1))return;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
			GetCharCode(*pC);
		}
	} else if(m_charset == CTYPE_EUC) {
		GetCharCode(0);
		while(n) {
			n--;
			//BYTE c = *(--p);
      //BYTE c;
      //if(!m_pDoc->Read(&c, n, 1))return;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
			if(*pC < 0xA1 || *pC > 0xFE) break;
		}
		while(n++ < ofs) {
      WORD w = 0;
      //if(!m_pDoc->Read(&w, n, 1))return;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
			GetCharCode(*pC);
		}
	}
}

WORD CBZView::GetCharCode(WORD c, UINT64 ofs)
{
	enum CharMode { CMODE_ASCII, CMODE_ESC, CMODE_ESC1, CMODE_ESC2, CMODE_ESC3, CMODE_KANJI1, CMODE_KANJI2, CMODE_KANA };
	static CharMode mode;
	static WORD c0;
	if(m_charset == CTYPE_JIS) {
		switch(c) {
		case 0x1B:
			mode = CMODE_ESC;
			break;
		case 0x0E:
			mode = CMODE_KANA;
			break;
		case 0x0F:
			mode = CMODE_ASCII;
			break;
		}
		if(c < 0x21 || c > 0x7E || mode == CMODE_ASCII) return c;
		switch(mode) {
		case CMODE_ESC:
			if(c == '(') mode = CMODE_ESC1;
			else if(c == '$') mode = CMODE_ESC2;
			else mode = CMODE_ASCII;
			break;
		case CMODE_ESC1:
			if(c == 'I') mode = CMODE_KANA;
			else mode = CMODE_ASCII;
			break;
		case CMODE_ESC2:
			mode = CMODE_KANJI1;
			break;
		case CMODE_KANJI1:
			c <<= 8;
			c0 = c;
			mode = CMODE_KANJI2;
			break;
		case CMODE_KANJI2:
			c = ' ';
			mode = CMODE_KANJI1;
			break;
		case CMODE_KANA:
			c |= 0x80;
			break;
		}
	} else if(m_charset == CTYPE_EUC) {
		if(c < 0x80) {
			mode = CMODE_ASCII;
		} else {
			switch(mode) {
			case CMODE_ASCII:
				if(c >= 0xA1 && c <= 0xFE) {
					mode = CMODE_KANJI1;
					c <<= 8;
				} else if(c == 0x8E) {
					mode = CMODE_KANA;
					c = CHAR_NG;
				} else if(c > 0x7E)
					c = CHAR_NG;
				break;
			case CMODE_KANJI1:
				c = ' ';
				mode = CMODE_ASCII;
				break;
			case CMODE_KANA:
				mode = CMODE_ASCII;
				break;
			}
		}
	} else if(m_charset == CTYPE_EPWING) {
		if(c == 0x1F) mode = CMODE_ESC3;
		else if(mode == CMODE_ESC3) {
			mode = CMODE_KANJI1;
			c = CHAR_NG;
		} else if(c < 0x21 || c > 0x7E) {
			mode = CMODE_KANJI1;
			c = CHAR_NG;
		} else if(mode == CMODE_KANJI1) {
			if(ofs == 0 || (ofs & 1) == 0) {
				c <<= 8;
				c0 = c;
				mode = CMODE_KANJI2;
			} else
				c = CHAR_NG;
		} else if(CMODE_KANJI2) {
			c = ' ';
			mode = CMODE_KANJI1;
		}
	}
	return c;
}

int CBZView::ConvertCharSet(CharSet charset, LPCSTR sFind, LPBYTE &buffer)
{
	int nFind = 0;
	if(charset == CTYPE_UNICODE || charset == CTYPE_UTF8) {
		nFind = ::MultiByteToWideChar(CP_ACP, 0, sFind, -1, NULL, 0);
		if(nFind) {
			buffer = (LPBYTE)MemAlloc(nFind * sizeof(WORD));
			nFind = ::MultiByteToWideChar(CP_ACP, 0, sFind, -1, (LPWSTR)buffer, nFind);
			if(nFind) nFind--;
			if(charset == CTYPE_UTF8) {			// ### 1.54b
				const int nFindUtf8 = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)buffer, nFind, NULL, 0, 0, 0);
				char *bufferNew = (char *)MemAlloc(nFindUtf8*2);
				::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)buffer, nFind, bufferNew, nFindUtf8*2, 0, 0);
				MemFree(buffer);
				buffer = (LPBYTE)bufferNew;
				nFind = nFindUtf8;
			}
		}
	} else {
		buffer = (LPBYTE)MemAlloc((strlen(sFind) + 1) * sizeof(WORD));
		BYTE *p = (LPBYTE)sFind;
		BYTE *q = buffer;
		BYTE c;
		while(c = *p++) {
			if(_ismbblead(c)) {
				if(charset == CTYPE_EBCDIC) {
					*q++ = CHAR_EBCDIC_NG;
					c = *p++;
					if(!c) break;
					*q++ = CHAR_EBCDIC_NG;
					continue;
				}
				WORD cjis = _mbcjmstojis(MAKEWORD(*p++, c));
				if(cjis) {
					if(charset == CTYPE_EUC) cjis |= 0x8080;
					*q++ = HIBYTE(cjis);
					*q++ = LOBYTE(cjis);
				}
			} else {
				if(charset == CTYPE_EBCDIC) {	// ### 1.63
					c = ConvertEbcDic(c);
					if(c == 0) continue;
				} else if(charset == CTYPE_EUC && c >= 0xA1 && c <= 0xDF) *q++ = 0x8E;
				*q++ = c;
			}
		}
		*q++ = '\0';
		nFind = q - buffer - 1;
	}
	return nFind;
}

CharSet CBZView::AutoDetectCharSet()
{
  if(!m_pDoc->IsOpen()) return options.charset;
	UINT64 dwSize = GetFileSize();
	if(dwSize > options.dwDetectMax) dwSize = options.dwDetectMax;
	CharSet charset = DetectCodeType(0, dwSize);//(p, p + dwSize);
	if(charset == CTYPE_BINARY)
		charset = options.charset;
	return charset;
}

#define DT_SJIS	1
#define DT_JIS  2
#define DT_UNICODE 4
#define DT_EUC 8
#define DT_UTF8 16		// ### 1.54b

CharSet CBZView::DetectCodeType(UINT64 dwStart, DWORD dwMaxSize)//(LPBYTE p, LPBYTE pEnd)
{
  if(!m_pDoc || !m_pDoc->IsOpen())return CTYPE_BINARY;
  m_pDoc->Cache(dwStart, dwMaxSize);
  LPBYTE p = m_pDoc->CacheForce(dwStart, 2);
	//LPBYTE p = m_pDoc->QueryMapViewTama2(dwStart, dwMaxSize); //LPBYTE p  = m_pDoc->GetDocPtr();
	//DWORD dwRemain = m_pDoc->GetMapRemain(dwStart);
	if(!p/* || dwRemain < 2*/)return CTYPE_BINARY;

	if(*(WORD*)p == 0xFEFF) {
		options.bByteOrder = FALSE;		// ### 1.54a
		return CTYPE_UNICODE;
	}
	if(*(WORD*)p == 0xFFFE) {
		options.bByteOrder = TRUE;
		return CTYPE_UNICODE;
	}
	if(*(WORD*)p == 0xBBEF)
  {
    size_t dwRemain = m_pDoc->GetRemainCache(dwStart);
    if(dwRemain >= 3 && *(p+2) == 0xBF)return CTYPE_UTF8;
	}

	UINT64 dwCurrent = dwStart;
	DWORD flag = DT_SJIS | DT_JIS | DT_EUC | DT_UTF8;
	while(dwCurrent < dwStart + dwMaxSize)//(p < pEnd)
	{
    p = m_pDoc->CacheForce(dwCurrent, 2);
    if(!p/* || dwRemain < 2*/)return CTYPE_BINARY;
    size_t dwRemain = m_pDoc->GetRemainCache(dwCurrent);
		//p = m_pDoc->QueryMapViewTama2(dwCurrent, 2);
		//dwRemain = m_pDoc->GetMapRemain(dwCurrent);
		BYTE c = (BYTE)*p++; dwCurrent++;//BYTE c = (BYTE)*p++;
		if(c == '\n') {
			if(flag == DT_SJIS || flag == DT_EUC || flag & DT_UNICODE) break;
		}
		if(c >= 0x80) {
			flag &= ~DT_JIS;
			if(flag & DT_EUC) {
				if(c != 0x8E && (c < 0xA1 || c > 0xFE))
					flag &= ~DT_EUC;
			}
			if(flag & DT_UTF8) {	// ### 1.54b
				if(c >= 0xC0)
				{
					if     ((c & 0xE0) == 0xC0 && dwRemain >= 2) { //UTF-8: 110yyy yx, 10xx xxxx
						if((*p & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/) // check 10xx xxxx
							flag &= ~DT_UTF8;
					}
					else if((c & 0xF0) == 0xE0 && dwRemain >= 3) { //UTF-8: 1110yyyy, 10yxxx xx, 10xx xxxx
						if((*p & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/ || (*(p+1) & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/) // check 10xxxxxx
							flag &= ~DT_UTF8;
					}
					else if((c & 0xF8) == 0xF0 && dwRemain >= 4) { //UTF-8: 11110y yy, 10yy xxxx, 10xxxx xx, 10xx xxxx
						if((*p & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/ || (*(p+1) & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/ || (*(p+2) & 0xC0/*1100 0000*/) != 0x80/*1000 0000*/) // check 10xxxxxx
							flag &= ~DT_UTF8;
					}
				}
				else if(flag == (DT_UTF8 | DT_SJIS) && dwRemain >= 5) // c == [0x80-0xBF]
				{
					int i=0;
					for(;i < 3;i++)
					{
						if((*(p+i) & 0xC0) != 0x80)break;
					}
					if(i==3 || ((*(p+i)) >= 0x80 && (*(p+i)) < 0xC0))
					{
						return CTYPE_SJIS;//flag &= ~CTYPE_SJIS;
					}
				}
			}
		}
		if((c >= 0x08 && c <= 0x0D) || (c >= 0x20 && c <= 0x7E) || (c >= 0x81 && c <= 0xFC)) {
			if(_ismbblead(c)) {
				c = *p++; dwCurrent++;//c = (BYTE)*p++;
				if(!_ismbbtrail(c)) {
					flag &= ~DT_SJIS;
					if((c == 0 || c == 0xFF) && ((DWORD)p & 1) == 0)
						flag |= DT_UNICODE;
				}
				if(c >= 0x80 && c != 0x8E && (c < 0xA1 || c > 0xFE))
					flag &= ~DT_EUC;
			}
		} else {
			if(c == 0x1B) {
				if(*p == '$' || *p == '(') 
					return CTYPE_JIS;
			} else if((c == 0 || c == 0xFF) && ((DWORD)p & 1) == 0)
				flag |= DT_UNICODE;
			else if(c < 0x20)
				return CTYPE_BINARY;
		}
		if(flag == 0) return CTYPE_BINARY;
	}
	return (flag & DT_UNICODE) ? CTYPE_UNICODE
		 : (flag & DT_JIS) ? CTYPE_BINARY		// ### 1.54b
		 : (flag & DT_EUC) ? CTYPE_EUC : (flag & DT_UTF8) ? CTYPE_UTF8 : CTYPE_SJIS;
}

/////////////////////////////////////////////////////////////////////////////
// EBCDIC ###1.63

BOOL CBZView::LoadEbcDicTable()
{
	if(m_bLoadEbcDic) return m_pEbcDic != NULL;
	m_bLoadEbcDic = TRUE;
	CString sPath;
	sPath = GetStructFilePath(IDS_EBCDIC_FILE);
	LPSTR pTable = (LPSTR)ReadFile(sPath);
	if(!pTable) return FALSE;
	LPSTR p = pTable;
	m_pEbcDic = new char[EBCDIC_COUNT + 1];
	memset(m_pEbcDic, CHAR_NG, EBCDIC_COUNT);
	LPSTR q = m_pEbcDic;
	while(*p &&	q < m_pEbcDic + EBCDIC_COUNT) {
		if(*p == ';') {
			while(*p++ != '\n') ;
			continue;
		}
		while(*p++ != '\t') ;
		for_to(i, 16)
			*q++ = *p++;
		while(*p++ != '\n') ;
	}
	*q++ = '\0';
	delete pTable;
	return TRUE;
}

UCHAR CBZView::ConvertEbcDic(UCHAR c)
{
	if(m_pEbcDic) {
		LPSTR p = strchr(m_pEbcDic, c);
		if(p)
			return p - m_pEbcDic + EBCDIC_BASE;
	}
	return 	CHAR_EBCDIC_NG;
}






void CBZView::ReCreateBackup()
{
	if(m_pDoc)
	{
		m_pDoc->m_restoreCaret = m_dwCaret;
		m_pDoc->m_restoreScroll = this->GetScrollPos();
		m_pDoc->m_restoreScroll.x = 0;
	}
}
void CBZView::ReCreateRestore()
{
	if(m_pDoc)
	{
		m_dwCaret = m_pDoc->m_restoreCaret;
		DrawCaret();
		ScrollToPos(m_pDoc->m_restoreScroll);
	}
}




// Quick Search Algorithm
void CBZView::preQuickSearchWI1(LPCWSTR searchTextW, BYTE nSearchTextW, BYTE *skipTable)
{
	for(int j=0;j<0x10000;j++)skipTable[j] = nSearchTextW+1;
	WORD wc;
	for(DWORD i=0;i<nSearchTextW;i++)
	{
		wc = searchTextW[i];
		skipTable[wc] = nSearchTextW - i;
		if('A' <= wc && wc <= 'Z')		skipTable[wc+0x20] = nSearchTextW - i;
		else if('a' <= wc && wc <= 'z')	skipTable[wc-0x20] = nSearchTextW - i;
	}
}
UINT64 CBZView::stristrBinaryW1(LPCWSTR searchTextW, BYTE nSearchTextW, UINT64 dwStart)
{
	BYTE skipTable[0x10000];
	preQuickSearchWI1(searchTextW, nSearchTextW, skipTable);

	DWORD dwNeedSize = (nSearchTextW+1)*2;
	UINT64 dwCurrent=dwStart;
  m_pDoc->Cache(dwStart);
  UINT64 dwTotal = GetFileSize();
	while(dwCurrent < dwTotal-2)
	{
    LPBYTE p = m_pDoc->CacheForce(dwCurrent, dwNeedSize);
    if(!p)break;//err
    if(wcsnicmp(searchTextW, (const wchar_t*)p, nSearchTextW)==0)return dwCurrent;
		dwCurrent += skipTable[p[nSearchTextW]]*2;
	}
	return _UI64_MAX; // error
}
void CBZView::preQuickSearchWI4(LPCWSTR searchTextW, DWORD nSearchTextW, DWORD *skipTable)
{
	for(int j=0;j<0x10000;j++)skipTable[j] = nSearchTextW+1;
	WORD wc;
	for(DWORD i=0;i<nSearchTextW;i++)
	{
		wc = searchTextW[i];
		skipTable[wc] = nSearchTextW - i;
		if('A' <= wc && wc <= 'Z')		skipTable[wc+0x20] = nSearchTextW - i;
		else if('a' <= wc && wc <= 'z')	skipTable[wc-0x20] = nSearchTextW - i;
	}
}
UINT64 CBZView::stristrBinaryW4(LPCWSTR pSearchTextW, DWORD nSearchTextW, UINT64 dwStart)
{
	DWORD skipTable[0x10000];
	preQuickSearchWI4(pSearchTextW, nSearchTextW, skipTable);

	DWORD dwNeedSize = (nSearchTextW+1)*2;
	UINT64 dwCurrent=dwStart;
  m_pDoc->Cache(dwStart);
  UINT64 dwTotal = GetFileSize();
	while(dwCurrent < dwTotal-2)
	{
    LPBYTE p = m_pDoc->CacheForce(dwCurrent, dwNeedSize);
    if(!p)break;//err
		if(wcsnicmp(pSearchTextW, (const wchar_t*)p, nSearchTextW)==0)return dwCurrent;
		dwCurrent += skipTable[p[nSearchTextW]]*2;
	}
  return _UI64_MAX; // error
}
UINT64 CBZView::stristrBinaryW(LPCWSTR searchTextW, DWORD nSearchTextW, UINT64 dwStart)
{
	if(nSearchTextW < 0xFF)
		return stristrBinaryW1(searchTextW, nSearchTextW, dwStart);
	else
		return stristrBinaryW4(searchTextW, nSearchTextW, dwStart);
}
void CBZView::preQuickSearchAI(LPCSTR searchText, DWORD nSearchText, DWORD *skipTable)
{
	for(int j=0;j<0x100;j++)skipTable[j] = nSearchText+1;
	BYTE ch;
	for(DWORD i=0;i<nSearchText;i++)
	{
		ch = searchText[i];
		skipTable[ch] = nSearchText - i;
		if('A' <= ch && ch <= 'Z')		skipTable[ch+0x20] = nSearchText - i;
		else if('a' <= ch && ch <= 'z')	skipTable[ch-0x20] = nSearchText - i;
	}
}
UINT64 CBZView::stristrBinaryA(LPCSTR pSearchText, UINT64 dwStart)
{
	DWORD nSearchText = strlen(pSearchText);
	DWORD skipTable[0x100];
	preQuickSearchAI(pSearchText, nSearchText, skipTable);
	UINT64 dwCurrent=dwStart;
  m_pDoc->Cache(dwStart);
  UINT64 dwTotal = GetFileSize();
	while(dwCurrent < dwTotal-1)
	{
    LPBYTE p = m_pDoc->CacheForce(dwCurrent, nSearchText+1); //なんで+1なのかな？？？保留
    if(!p)break;//err

		if(strnicmp(pSearchText, (const char*)p, nSearchText)==0)return dwCurrent;
		dwCurrent += skipTable[p[nSearchText]];
	}
	return _UI64_MAX; // error
}
void CBZView::preQuickSearch(LPBYTE pSearchByte, unsigned int nSearchByte, DWORD* skipTable)
{
	for(int j=0;j<0x100;j++)skipTable[j] = nSearchByte+1;
	for(DWORD i=0;i<nSearchByte;i++)skipTable[pSearchByte[i]] = nSearchByte - i;
}
UINT64 CBZView::strstrBinary(LPBYTE pSearchStr, unsigned int nSearchStr, UINT64 dwStart)
{
  DWORD skipTable[0x100];
  preQuickSearch(pSearchStr, nSearchStr, skipTable);
  UINT64 dwCurrent=dwStart;
  m_pDoc->Cache(dwStart);
  UINT64 dwTotal = GetFileSize();
  while(dwCurrent < dwTotal-1)
  {
    LPBYTE p = m_pDoc->CacheForce(dwCurrent, nSearchStr+1); //なんで+1なのかな？？？保留
    if(!p)break;//err

    if(memcmp(pSearchStr, p, nSearchStr)==0)return dwCurrent;
    dwCurrent += skipTable[p[nSearchStr]];
  }
  return _UI64_MAX; // error
}

UINT64 CBZView::GetFileSize() { return m_pDoc ? m_pDoc->GetDocSize() : 0; }