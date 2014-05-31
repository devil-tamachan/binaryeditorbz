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
#include "BZBmpVw2.h"
#include "MainFrm.h"
#include "ColorDlg.h"
#include "SettingDlg.h"
#include  <ctype.h>
#include  <winnls.h>
#include  <mbstring.h>

class cBZBmpView2;


//static const UINT nMsgFindReplace = ::RegisterWindowMessage(FINDMSGSTRING);


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


void CBZView::OnUpdateEditCut()  { UIEnable(ID_EDIT_CUT, m_bBlock && !m_pDoc->IsReadOnly()); }
void CBZView::OnUpdateEditPaste()
{
  OpenClipboard();
  UINT cf = EnumClipboardFormats(0);
  CloseClipboard();
  UIEnable(ID_EDIT_PASTE, !m_pDoc->IsReadOnly() && cf);
}
void CBZView::OnUpdateEditValue()      { UIEnable(ID_EDIT_VALUE, !m_pDoc->IsReadOnly()); }
void CBZView::OnUpdateEditSelectAll()  { UIEnable(ID_EDIT_SELECT_ALL, m_pDoc->GetDocSize()!=0); }
void CBZView::OnUpdateStatusInfo()
{
  if(GetMainFrame()->m_bDisableStatusInfo) return;
  if(GetFileSize()) {
    UISetText(1/*ID_INDICATOR_INFO*/, GetStatusInfoText());
    //UIEnable(ID_INDICATOR_INFO, TRUE);
  } else {
    UISetText(1/*ID_INDICATOR_INFO*/, _T(""));
    //UIEnable(ID_INDICATOR_INFO, FALSE);
  }
}
void CBZView::OnUpdateStatusIns()
  {
    CString sText;
    sText.LoadString(m_pDoc->IsReadOnly() ? IDS_EDIT_RO : IDS_EDIT_OVR + m_bIns);
    UISetText(4/*ID_INDICATOR_INS*/, sText);
  }
void CBZView::OnUpdateJump()
{
  UIEnable(ID_JUMP_COMPARE, GetMainFrame()->m_nSplitView && m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_TO, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_OFFSET, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_RETURN, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_MARK, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_MARKNEXT, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_START, m_pDoc->GetDocSize()!=0);
  UIEnable(ID_JUMP_END, m_pDoc->GetDocSize()!=0);
}

void CBZView::OnJumpFindnext(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CComboBox4ToolBar* pCombo = &(GetMainFrame()->m_combo_toolbar);
  UINT64 dwTotal = GetFileSize();

  CStringA sFind;
  {
    LPSTR tmp = sFind.GetBufferSetLength(510);
    ::GetWindowTextA(pCombo->m_hWnd, tmp, 509);
    sFind.ReleaseBuffer();
  }
  if(sFind.IsEmpty()) {
    if(GetBrotherView())
      OnJumpCompare();
    return;
  }

  int c1 = sFind[0];
  if(c1 == '=') {
    pCombo->SetWindowText(_T("? "));
    return;
  }
  pCombo->AddString(CA2T(sFind));
  if(c1 == '?' || c1 == '+' || c1 == '>'|| c1 == '<') {
    DWORD dwNew = 0;
    long nResult;
    if(CalcHexa((LPCSTR)sFind + 1, nResult)) {
      switch(c1) {
      case '?': {
        CString sResult;
        sResult.Format(_T("= 0x%X (%d)"), nResult, nResult);
        pCombo->SetWindowText(sResult);
        break;
                }
      case '+':
        dwNew = m_dwCaret + m_pDoc->m_dwBase;
      case '>':
        dwNew += nResult - m_pDoc->m_dwBase;	// ###1.63
        if(dwNew <= dwTotal) {
          m_dwOldCaret = m_dwCaret;
          m_dwCaret = dwNew;
          m_bCaretOnChar = FALSE;
          GotoCaret();
          SetFocus();
        } else
          ErrorResMessageBox(IDS_ERR_RANGE, m_hWnd);
        break;
      case '<':
        if(m_pDoc->IsReadOnly())
          ErrorResMessageBox(IDS_ERR_READONLY, m_hWnd);
        else {
          FillValue(nResult);
        }
        SetFocus();
        break;
      }
    }
    return;
  }

  //検索モード [# 00 AA BB FF] または [abcdef]
  //CWaitCursor wait;	// ###1.61

  UINT64 dwStart = m_dwCaret + 1;
  if(dwStart >= dwTotal-1)return;

  UINT64 dwRetAddress = _UI64_MAX;//err

  int nFind = 0;
  LPBYTE pFind = NULL;
  CharSet charset = m_charset;
  if(c1 == '#') {//検索モード [# 00 AA BB FF]
    nFind = ReadHexa(sFind, pFind);
    if(!nFind) return;
    dwRetAddress = strstrBinary(pFind, nFind, dwStart);//charset = CTYPE_BINARY;
  } else {		//検索モード [abcdef]
    if(charset >= CTYPE_UNICODE) { //CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY
      nFind = ConvertCharSet(charset, sFind, pFind);
      if(!pFind || !nFind) return;
      if(charset == CTYPE_UNICODE) {
        if(dwStart&1) {
          dwStart++;
        }
        dwRetAddress = stristrBinaryW((LPCWSTR)pFind, nFind, dwStart);
      } else {
        dwRetAddress = strstrBinary(pFind, nFind, dwStart);//charset = CTYPE_BINARY;
      }
    } else { //CTYPE_ASCII, CTYPE_SJIS
      nFind = sFind.GetLength();
      if(m_charset == CTYPE_ASCII)
        dwRetAddress = stristrBinaryA(sFind, dwStart);
      else
        dwRetAddress = strstrBinary((LPBYTE)((LPCSTR)sFind), nFind, dwStart);
    }
  }
  if(dwRetAddress==_UI64_MAX)
  {
    ErrorResMessageBox(IDS_ERR_FINDSTRING, m_hWnd);
    pCombo->SetFocus();
    if(pFind) MemFree(pFind);
    return;
  }

  m_dwOldCaret = m_dwCaret;
  m_dwCaret = dwRetAddress;
  m_bCaretOnChar = !pFind;
  GotoCaret();
  SetFocus();

  if(pFind) MemFree(pFind);
}
void CBZView::OnJumpCompare(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CBZView* pView1 = GetBrotherView();
  if(!pView1) return;
  CBZDoc2 *pDoc1 = pView1->m_pDoc;
  if(!pDoc1) return;
  CComboBox4ToolBar* pCombo = &(GetMainFrame()->m_combo_toolbar);
  pCombo->SetWindowText(_T(""));

  UINT64 len;//これから比較するバイト量
  {
    UINT64 dwCanRead0 = GetRemainFromCurret();
    UINT64 dwCanRead1 = pView1->GetRemainFromCurret();
    len = min(dwCanRead0, dwCanRead1);
    if(len <= 1) return;
  }
  len--;
  UINT64 dwCurrent0 =         m_dwCaret+1;
  UINT64 dwCurrent1 = pView1->m_dwCaret+1;
  LPBYTE pData0, pData1;
  do
  {
    //DWORD maxMapSize = min(len, options.dwMaxMapSize);
    pData0 = m_pDoc->Cache(dwCurrent0);
    pData1 = pDoc1->Cache(dwCurrent1);
    if(!pData0 || !pData1)
    {//ERR: Mapping failed
      MessageBox(_T("File Mapping Error!"), _T("Error"), MB_OK);
      return;
    }
    size_t dwRemain0 = m_pDoc->GetRemainCache(dwCurrent0);
    size_t dwRemain1 = pDoc1->GetRemainCache(dwCurrent1);
    size_t len32;
    if(len > SIZE_MAX)len32 = SIZE_MAX;
    else len32 = (size_t)len;
    size_t minMapSize = min(min(dwRemain0, dwRemain1), len32); //minmax内で直接callすると何度も実行される
    if(minMapSize==0) return;
    DWORD ofs = MemCompByte2(pData0, pData1, minMapSize);
    if(ofs!=0xFFFFffff)
    {	//Data0 != Data1
      ofs--;
      dwCurrent0 += ofs;
      dwCurrent1 += ofs;
      goto founddiff;
    }
    len -= minMapSize;
    dwCurrent0 += minMapSize;
    dwCurrent1 += minMapSize;
  } while(len > 0);

  if(dwCurrent0==GetFileSize() && dwCurrent1==pView1->GetFileSize())
  {
    InfoResMessageBox(IDS_COMPARE_OK, m_hWnd);
    return;
  }
founddiff:
  m_dwOldCaret = m_dwCaret;
  m_dwCaret = dwCurrent0;
  pView1->m_dwOldCaret = pView1->m_dwCaret;
  pView1->m_dwCaret = dwCurrent1;
  GotoCaret();
  pView1->GotoCaret();
  Invalidate(FALSE);
  pView1->Invalidate(FALSE);
  return;
}

void CBZView::OnEditUndo(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    UINT64 dwRetStart = 0;
    if(m_pDoc->DoUndo(&dwRetStart))
    {
      m_dwCaret = dwRetStart;
      GotoCaret();
      UpdateDocSize();
      Invalidate(FALSE);
    } else {
      ATLASSERT(FALSE);
    }
  }
void CBZView::OnEditRedo(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    UINT64 dwRetStart = 0;
    if(m_pDoc->DoRedo(&dwRetStart))
    {
      m_dwCaret = dwRetStart;
      GotoCaret();
      UpdateDocSize();
      Invalidate(FALSE);
    } else {
      ATLASSERT(FALSE);
    }
  }
void CBZView::OnEditPaste(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    DWORD dwPaste;
    if(dwPaste = m_pDoc->PasteFromClipboard(m_dwCaret, m_bIns)) {
      m_dwOldCaret = m_dwCaret;
      m_dwCaret = dwPaste;
      UpdateDocSize();
      Invalidate(FALSE);
    }
  }
void CBZView::OnViewColor(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CSetupColorDialog dlg;
  dlg.m_pSampleFont = m_pFont;
  if(dlg.DoModal() == IDOK)
    GetMainFrame()->Invalidate(TRUE);
}
void CBZView::OnJumpBase(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CInputDlg dlg;
  dlg.m_sValue.Format(_T("%X"), m_pDoc->m_dwBase);
  if(dlg.DoModal() == IDOK) {
    m_pDoc->m_dwBase = _tcstol(dlg.m_sValue, NULL, 16);
    Invalidate(FALSE);
  }
}
  void CBZView::SetMark(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    m_pDoc->SetMark(m_dwCaret);
    Invalidate(FALSE);
  }
  void CBZView::JumpToMark(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    UINT64 dwMark = m_pDoc->JumpToMark(m_dwCaret);
    if(dwMark != INVALID)
      JumpTo(dwMark);
  }
void CBZView::OnByteOrder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  if(nID != (UINT)options.bByteOrder + ID_BYTEORDER_INTEL)
    options.bByteOrder = !options.bByteOrder;
  GetMainFrame()->UpdateInspectViewChecks();
}
void CBZView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  UINT64 dwNewCaret = m_dwCaret;
  UINT64 dwTotal = GetFileSize();
  BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
  BOOL bShift = (GetKeyState(VK_SHIFT) < 0 || GetKeyState(VK_LBUTTON) < 0);
  ATLTRACE("KeyDown: %X %d %X\n", nChar, nRepCnt, nFlags);
  switch(nChar)
  {
  case VK_RETURN:
    if(bCtrl) OnDoubleClick();	// ### 1.62
    else
      PostMessage(WM_COMMAND, ID_JUMP_FINDNEXT);
    UpdateStatusBar();
    return;
  case VK_INSERT:
    if(!m_pDoc->IsReadOnly()
#ifdef FILE_MAPPING
      //		 && !m_pDoc->IsFileMapping()
#endif //FILE_MAPPING
      ) {
        m_bIns = !m_bIns;
        InitCaret();
        m_bEnterVal = FALSE;
        UpdateStatusBar();
        return;
    }
    goto Error;
  case VK_TAB:
    if(bCtrl) {
      GetMainFrame()->ChangeView(this);
      return ;
    }
    if(bShift && GetMainFrame()->m_bStructView) {
      CBZFormView* pView = (CBZFormView*)(GetSubView());
      if(pView)pView->Activate();
      UpdateStatusBar();
      return;
    }
    m_bCaretOnChar = !m_bCaretOnChar;
    break;
  case VK_RIGHT:
    if(dwNewCaret<dwTotal)dwNewCaret++;
    break;
  case VK_LEFT:
    if(dwNewCaret>0)dwNewCaret--;
    break;
  case VK_DOWN:
    dwNewCaret += 16;
    if(dwNewCaret < m_dwCaret) dwNewCaret = m_dwCaret;
    break;
  case VK_UP:
    if(dwNewCaret>=16)dwNewCaret -= 16;
    break;
  case VK_NEXT://PageDown
    dwNewCaret += 16 * PAGESKIP;
    if(dwNewCaret > dwTotal || dwNewCaret < m_dwCaret)
      dwNewCaret = dwTotal;
    break;
  case VK_PRIOR://PageUp
    if(dwNewCaret>=16 * PAGESKIP)
    {
      dwNewCaret -= 16 * PAGESKIP;
      if(dwNewCaret > dwTotal || dwNewCaret > m_dwCaret)
        dwNewCaret = 0;
    } else dwNewCaret = 0;
    break;
  case VK_HOME:
    if(bCtrl)	dwNewCaret = 0;
    else		dwNewCaret&=~(16-1);
    break;
  case VK_END:
    if(bCtrl)	dwNewCaret = dwTotal;
    else		dwNewCaret|=(16-1);
    break;
  case VK_BACK:
    if(!dwNewCaret) goto Error;
    if(!m_bBlock) dwNewCaret--;
  case VK_DELETE:
    if(m_pDoc->IsReadOnly())
      goto Error;
#ifdef FILE_MAPPING
    //		if(m_pDoc->IsFileMapping()) goto Error;
#endif //FILE_MAPPING
    if(m_bBlock) {
      CutOrCopy(EDIT_DELETE);
      UpdateStatusBar();
      return;
    } else {
      if(dwNewCaret == dwTotal
        || !m_pDoc->Delete(dwNewCaret, 1)) goto Error;
      UpdateDocSize();
      Invalidate(FALSE);
    }
    break;
  default:
    //CTextView::OnKeyDown(nChar, nRepCnt, nFlags);
    SetMsgHandled(FALSE);
    return;
  }
  m_bEnterVal = FALSE;
  if(!m_bBlock && bShift) {
    m_dwBlock = m_dwCaret;
    m_bBlock = TRUE;
  } else if(m_bBlock && !bShift) {
    m_bBlock = FALSE;
    Invalidate(FALSE);
  }
  if(m_ptCaret.x == -1) {
    if(dwNewCaret <= dwTotal) {	// ### 1.62
      m_dwCaret = dwNewCaret;
      GotoCaret();
    }
  } else
    MoveCaretTo(dwNewCaret);
  UpdateStatusBar();
  return;
Error:
  MessageBeep(MB_NOFOCUS);
  return;
}


int CBZView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  m_nColAddr = ADDRCOLUMNS;
  /*SetViewSize(WTL::CSize(VIEWCOLUMNS, 0));*/

  LONG lExStyle = GetWindowLong(GWL_EXSTYLE);
  lExStyle |= WS_EX_STATICEDGE;
  SetWindowLong(GWL_EXSTYLE, lExStyle);

  WTL::CMessageLoop* pLoop = _Module.GetMessageLoop();
  // pLoop->AddIdleHandler(this);

  DragAcceptFiles();

  CMainFrame *pMainFrame = GetMainFrame();
  if(pMainFrame)
  {
    UIAddStatusBar(pMainFrame->m_statusbar);
  }

  SetMsgHandled(FALSE);
  return 0;
}

void CBZView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
  {
  ATLTRACE("OnChar\n");
    static UINT preChar = 0;
    UINT64 dwTotal = GetFileSize();

    if(nChar < ' ' || nChar >= 256)
      return;
    if(m_pDoc->IsReadOnly())
      goto Error;
    if(!m_bEnterVal && !preChar) {
      DWORD dwSize = 1;
      if(m_bCaretOnChar && (m_charset == CTYPE_UNICODE || (m_charset > CTYPE_UNICODE && _ismbblead((BYTE)nChar)))) {
        if(m_charset == CTYPE_UTF8)		// ### 1.54b
          dwSize = 3;
        else
          dwSize = 2;
      }
    }
    m_bBlock = FALSE;
    //p = m_pDoc->QueryMapViewTama2(m_dwCaret, 4); //p  = m_pDoc->GetDocPtr() + m_dwCaret;
    if(!m_bCaretOnChar) {
      if(nChar >= '0' && nChar <= '9')
        nChar -= '0';
      else if(nChar >= 'A' && nChar <= 'F')
        nChar -= 'A' - 10;
      else if(nChar >= 'a' && nChar <= 'f')
        nChar -= 'a' - 10;
      else
        goto Error;
      if(m_bEnterVal) {
        BYTE nVal = 0;
        m_pDoc->Read(&nVal, m_dwCaret, 1);
        nChar |= nVal<<4;
        m_pDoc->DoUndo();
        dwTotal = GetFileSize();
        m_bEnterVal = FALSE;
      } else
        m_bEnterVal = TRUE;
    } else if(m_charset >= CTYPE_UNICODE) {
      char  mbs[4];
      char* pb = mbs;
      if(preChar) {
        *pb++ = preChar;
        preChar = 0;
      } else if(_ismbblead((BYTE)nChar)) {
        preChar = nChar;
        return;
      }
      *pb++ = (char)nChar;
      *pb++ = 0;
      LPBYTE buffer = NULL;
      int len = ConvertCharSet(m_charset, mbs, buffer);
      if(len) {
        if(m_charset == CTYPE_UNICODE) len *= 2;
        pb = (char*)buffer;
        BOOL bInsert = m_bIns || (m_dwCaret == dwTotal);
        if(bInsert)m_pDoc->Insert(buffer, m_dwCaret, len);
        else m_pDoc->Write(buffer, m_dwCaret, len);
        UpdateDocSize();
        MemFree(buffer);
        //Invalidate(FALSE);
        if(!m_bEnterVal)
          MoveCaretTo(m_dwCaret + len);
      }
      return;
    }
    BOOL bInsert = m_bIns || (m_dwCaret == dwTotal);
    if(bInsert)m_pDoc->Insert(&nChar, m_dwCaret, 1);
    else m_pDoc->Write(&nChar, m_dwCaret, 1);
    UpdateDocSize();
    //Invalidate(FALSE);
    if(!m_bEnterVal) {
      MoveCaretTo(m_dwCaret+1);
    }
    return;
Error:
    MessageBeep(MB_NOFOCUS);
    return;
  }

  void CBZView::OnDropFiles(HDROP hDropInfo)
  {
    unsigned int numFile = ::DragQueryFile(hDropInfo, -1, NULL, 0);
    for(unsigned int i=0; i<numFile; i++)
    {
      CString path;
      ::DragQueryFile(hDropInfo, i, path.GetBuffer(1024+2), 1024);
      path.ReleaseBuffer();
      if(m_pDoc)m_pDoc->OnFileOpen(path, m_hWnd);
      Update();
      CMainFrame *pMainFrame = GetMainFrame();
      if(pMainFrame)pMainFrame->UpdateFrameTitle();
    }
    ::DragFinish(hDropInfo);
  }



void CBZView::Update() 
{
	ATLTRACE("CBZView::OnUpdate()\n");
	
	m_pDoc = GetBZDoc2();
	//ASSERT_VALID(m_pDoc);
	m_nColAddr = (options.bDWordAddr) ? ADDRCOLUMNS_MAP : ADDRCOLUMNS;
	/*if(GetViewWidth() != VIEWCOLUMNS) {
    SetViewSize(WTL::CSize(VIEWCOLUMNS, 0));
		m_bResize = FALSE;
		//ResizeFrame();
	}*/

	UpdateDocSize();
	//ScrollToPos(WTL::CPoint(0,0));
    //SetScrollSizeU64V(cViewX, cViewY);
    SetScrollPage(3,PAGESKIP);
    SetScrollLine(1,1);
	SetScrollHome(WTL::CPoint(0, DUMP_Y));
    ScrollTop();
    ScrollAllLeft();
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
			CBZFormView* pView = (CBZFormView*)GetSubView();
			if(pView!=NULL)
				pView->SelectTag();
		}
		if(pMainFrame->m_bAnalyzerView) {
			CBZAnalyzerView* pView = (CBZAnalyzerView*)GetSubView();
			if(pView!=NULL)
				pView->Clear();
		}
    if(pMainFrame->m_bBmpView) {
			CBZBmpView2* pView = (CBZBmpView2*)GetSubView();
			if(pView!=NULL)
				pView->OnInitialUpdate();
		}
	}
	Invalidate(FALSE);
	InitCaret();
	DrawCaret();
}


/////////////////////////////////////////////////////////////////////////////
// CBZView drawing

/*
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
*/

void CBZView::PrePrintPage/*BeginPrintJob(HDC hDC)*/(UINT nPage, HDC hDC)
{
  if(!m_bBeginPrintJob)
  {
    CTextView::BeginPrintJob(hDC);

    WTL::CDCHandle cdc(hDC);
    WTL::CRect rMargin;
    GetMargin(rMargin, hDC);
    POINT pt;
    pt.x = cdc.GetDeviceCaps(HORZRES) - rMargin.left - rMargin.right;
    pt.y = cdc.GetDeviceCaps(VERTRES) - rMargin.top - rMargin.bottom;
    PixelToGrid(pt);
    m_nPageLen = pt.y-1;
    CMainFrame *pMainFrame = GetMainFrame();
    if(pMainFrame)
    {
      unsigned int maxPage = GetMaxPrintingPage()-1;
      pMainFrame->m_wndPrintPreview.m_nMaxPage = maxPage; //https://twitter.com/deviltamachan/status/469481807562305536
      pMainFrame->m_job.m_nEndPage = maxPage;
    }
    //SetMaxPrintingPage(pInfo);

    m_bBeginPrintJob = TRUE;
  }
  SetMargin(hDC);

  m_dwPrint = (m_bBlock) ? (BlockBegin()&~(16-1)) : 0;
  m_dwPrint += nPage * m_nPageLen * 16;
}

void CBZView::SetColor(TextColor n)
{
	CTextView::SetColor(options.colors[n][0], options.colors[n][1]);
}

void CBZView::SetHeaderColor()
{
	BOOL bNoFocus = GetMainFrame()->m_nSplitView && GetActiveBZDoc2() != m_pDoc;
	SetColor(bNoFocus ? TCOLOR_ADDRESS2 : TCOLOR_ADDRESS);
}


/////////////////////////////////////////////////////////////////////////////
// CBZView OnDraw

void CBZView::DrawToFile(CAtlFile* pFile) 	// ###1.63
{
	m_pFile = pFile;
	OnPaint(NULL);
	m_pFile = NULL;
}

void CBZView::_OnPaint(WTL::CDCHandle dc, LPRECT lpUpdateRect, BOOL bPrint)
{
  ATLTRACE("OnPaint\n");
	//LPBYTE p  = m_pDoc->GetDocPtr();
//	if(!p) return;

  //WTL::CRect rClip;
  CRectU64V rClipU64V;
	UINT64 ofs;
	UINT64 dwBegin = BlockBegin();
	UINT64 dwEnd   = BlockEnd();
	UINT64 dwTotal = GetFileSize();

	UINT64 dwTotalP1 = 0;

	PutBegin(dc);
	if(IsToFile()) {
		DrawHeader2File();
		rClipU64V.y1 = DUMP_Y;
		rClipU64V.y2 = LONG_MAX;
		ofs = 0;
		if(m_bBlock) {
			ofs = dwBegin & ~0xF;
			dwTotal = dwEnd;
		}
	} else if(bPrint/*pDC->IsPrinting()*/) {		// ### 1.54
		rClipU64V.y1 = DUMP_Y;
		rClipU64V.y2 = m_nPageLen;
		ofs = m_dwPrint;
		DrawHeader(ofs);
  } else {
    if(dc==NULL)return;
      WTL::CRect rClip;
    if(lpUpdateRect)
    {
      ATLTRACE("lpUpdateRect = (%d,%d,%d,%d)\n", lpUpdateRect->left, lpUpdateRect->right, lpUpdateRect->top, lpUpdateRect->bottom);
      rClip = lpUpdateRect;
    } else {
      dc.GetClipBox(&rClip);
      ATLTRACE("rClip = (%d,%d,%d,%d)\n", rClip.left, rClip.right, rClip.top, rClip.bottom);
      //rClipU64V = rClip;
    }
    PixelToGrid(rClip);
    BOOL bDrawHeader = FALSE;
		if(rClip.y1 < DUMP_Y) {
      bDrawHeader = TRUE;
			rClip.y1 += DUMP_Y;
		}
    rClipU64V = rClip;
    ATLTRACE("rClipU64V = (%d,%d,%I64u,%I64u)\n", rClipU64V.left, rClipU64V.right, rClipU64V.top, rClipU64V.bottom);
    ofs = (rClipU64V.top + m_u64V - DUMP_Y)*16;
		if(bDrawHeader)DrawHeader(ofs);
	}
	if(!IsToFile() && (bPrint/*pDC->IsPrinting()*/ && m_bBlock))
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
	if(ofs < dwTotal)
    InitCharMode(ofs);

  BOOL fSJISSkipNextTopChar = FALSE;
  ATLTRACE("OnPaint2\n");

  if(m_charset == CTYPE_SJIS)
  {
    UINT64 ofs0 = ofs;
    int numSearch = 0;
    if(ofs > ofs-16)
    {
      ofs -= 16;
      numSearch = 16;
    } else {
      ofs = 0;
      numSearch = ofs;
    }

    for_to_(i,numSearch)
    {
      if(ofs >= dwTotal || (m_charset == CTYPE_UNICODE && ofs+1 >= dwTotal)) {
        //SetColor();
        //PutChar(' ', 16-i);
        ofs++;
        break;
      }
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

      if(c < 0x20)
      {
        //c = CHAR_NG;
      }
      else if(!fSJISSkipNextTopChar && _ismbblead(c)/*IsMBS(p, ofs-1, FALSE)*/ && dwTotal > ofs && (dwB>=2 && _ismbbtrail(*(pB+1))))
      {
        BYTE c1 = *(pB+1);
        if(_ismbclegal(MAKEWORD(c1, c))) {
          //PutChar((char)c);
          c = *(pB+1);
          if(i < 15) {
            ofs++;
            i++;
          } else fSJISSkipNextTopChar = TRUE;
        } else {
          //c = CHAR_NG;
        }
      } else {
        //LPBYTE pPrev = NULL;
        //if(!fSJISSkipNextTopChar && i==0 && ofs>=2)pPrev = m_pDoc->CacheForce(ofs-2, 1);

        if(fSJISSkipNextTopChar || (i == 0 && _ismbbtrail(c) /*IsMBS(p, ofs-1, TRUE)*/ ))//&& pPrev && _ismbblead(pPrev[0])))
        {
          fSJISSkipNextTopChar = FALSE;
          //c=' ';
        } else if((c > 0x7E && c < 0xA1) || c > 0xDF)
        {
          //c = CHAR_NG;
        }
      }
    }
    ofs = ofs0;
  } else { //charset != SJIS
    m_pDoc->Cache(ofs, 1000);
    if(pDoc1)pDoc1->Cache(ofs, 1000);
  }

	UINT64 y;

	for(/*int */y = rClipU64V.y1; y <= rClipU64V.y2; y++) {
		Locate(0, y);
		if(ofs > dwTotal) break;
		if(IsToFile() && options.nDumpPage) {
			if(y > DUMP_Y && ((y - DUMP_Y) % options.nDumpPage) == 0) {
				Locate(0, y); // crlf
				DrawHeader2File();
				Locate(0, y); // crlf
			}
		}
		SetHeaderColor();
		UINT64 ofs1 = ofs + m_pDoc->m_dwBase;
#ifdef FILE_MAPPING
//		if(p && !(p = m_pDoc->QueryMapView(p, ofs1))) return;
	//	if(p1 && !(p1 = pView1->m_pDoc->QueryMapView(p1, ofs1))) return;

    DWORD ofsHi = HIDWORD(ofs1);
    DWORD ofsLo = LODWORD(ofs1);
    //描画: 先頭のアドレス
		if(options.bDWordAddr)//m_nColAddr > ADDRCOLUMNS)
    {
			PutFormatStr("%04X:%04X-%04X:%04X", HIWORD(ofsHi), LOWORD(ofsHi), HIWORD(ofsLo), LOWORD(ofsLo));
			SetColor();
			PutStr("  ");
		} else
#endif //FILE_MAPPING
		{
      if(ofs1 < 0x1000000) {			// ###1.54
				PutFormatStr("%06X", ofs1);
				SetColor();
				PutStr("  ");
			} else if(ofs1 < 0x100000000) {
				PutFormatStr("%X", ofs1);
				SetColor();
				PutStr(" ");
			} else if(ofsHi < 0x1000000) {
				PutFormatStr("%06X-%08X", ofsHi, ofsLo);
				SetColor();
				PutStr("  ");
			} else {
				PutFormatStr("%X-%08X", ofsHi, ofsLo);
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
				WORD w = /*SwapWord(*/*pW/*)*/;	// ### 1.54a
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
          else if(!fSJISSkipNextTopChar && _ismbblead(c)/*IsMBS(p, ofs-1, FALSE)*/ && dwTotal > ofs && (dwB>=2 && _ismbbtrail(*(pB+1))))
          {
            BYTE c1 = *(pB+1);
            if(_ismbclegal(MAKEWORD(c1, c))) {
              PutChar((char)c);
              c = *(pB+1);
              if(i < 15) {
                ofs++;
                i++;
              } else fSJISSkipNextTopChar = TRUE;
            } else
              c = CHAR_NG;
          } else {
            LPBYTE pPrev = NULL;
            if(!fSJISSkipNextTopChar && i==0 && ofs>=2)pPrev = m_pDoc->CacheForce(ofs-2, 1);

            if(fSJISSkipNextTopChar || (i == 0 && _ismbbtrail(c) /*IsMBS(p, ofs-1, TRUE)*/ && pPrev && _ismbblead(pPrev[0])))
            {
              fSJISSkipNextTopChar = FALSE;
              c=' ';
            } else if((c > 0x7E && c < 0xA1) || c > 0xDF)
              c = CHAR_NG;
          }
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
								if(dwB<2) c = CHAR_NG;
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
								if(dwB<3) c = CHAR_NG;
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
		if(y <= rClipU64V.y2) {
			SetColor();
			for(y; y<=rClipU64V.y2+1; y++) {
				Locate(0, y);
				PutChar(' ', VIEWCOLUMNS);
			}
		}
		PutEnd();
		DrawDummyCaret(dc);
		DrawCaret();
	}

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

void CBZView::DrawHeader2File()
{
	if(IsToFile() && !options.sDumpHeader.IsEmpty()) {
		m_pFile->Write(options.sDumpHeader, options.sDumpHeader.GetLength());
	}
}

void CBZView::DrawHeader(UINT64 ofs)
{
  SetHeaderColor();
  Locate(GetDUMP_X(ofs),0);
  for_to(i,16)
    PutFormatStr("+%1X ", i);
  PutChar();
  for_to_(i,16)
    PutFormatStr("%1X", i);
}

HRESULT CBZView::CalcOffsetByClientRect(UINT64 *pQWStart, UINT64 *pQWMax, UINT64 *pQWMax2 /*=NULL*/)
{
  UINT64 v64 = GetScrollPosU64V();
  UINT64 dwOrg = v64 * 16;

  RECT rClient;
  GetClientRect(&rClient);
  PixelToGrid(rClient);
  UINT64 dwBottom = (dwOrg + (rClient.y2 - DUMP_Y) * 16) + 16; //下が欠けた最下ラインのカレットが表示されないので+16
  UINT64 dwMax = GetFileSize() + 1;
  UINT64 dwMax2 = dwMax;
  if(dwBottom > dwOrg && dwBottom < dwMax)
  {
    dwMax = dwBottom;
    dwMax2 = dwBottom-16;
  }

  if(pQWStart)*pQWStart = dwOrg;
  if(pQWMax)*pQWMax = dwMax;
  if(pQWMax2)*pQWMax2 = dwMax2;

  return S_OK;
}

void CBZView::DrawDummyCaret(HDC hDC)
{
	if(GetFocus() != this->m_hWnd) {
    UINT64 dwOrg = 0, dwMax = 0;
    CalcOffsetByClientRect(&dwOrg, &dwMax);
		POINT pt;
		LONG ptx2 = -1;	// ###1.62
		if(m_dwCaret < dwOrg || m_dwCaret >= dwMax) {
		} else {
      pt.x = (m_dwCaret - dwOrg)%16;
      int dump_x = GetDUMP_X(m_dwCaret);
      int char_x = dump_x + 3*16 + 1;
			ptx2 = pt.x + char_x;
			pt.x = pt.x*3 + dump_x;
			if(m_bCaretOnChar) Swap(pt.x, ptx2);
      long ptOrgX = m_ptOffset.x/m_cellH;
			pt.x -= ptOrgX;
			ptx2 -= ptOrgX;
			pt.y = (m_dwCaret - dwOrg)/16 + DUMP_Y;

			GridToPixel(pt);
      WTL::CRgn rgn1;
			rgn1.CreateRectRgn(pt.x, pt.y, pt.x+m_cell.cx, pt.y+m_cell.cy);
			InvertRgn(hDC, rgn1);
		}
	}
}

BOOL CBZView::DrawCaret(int *pScrolldy /*=NULL*/)
{
	BOOL bDraw = FALSE;
  UINT64 dwOrg = 0, dwMax = 0, dwMax2 = 0;
  CalcOffsetByClientRect(&dwOrg, &dwMax, &dwMax2);
  if(pScrolldy)*pScrolldy=0;

	POINT pt;
	LONG ptx2 = -1;	// ###1.62
	if(m_dwCaret < dwOrg || m_dwCaret >= dwMax) {
		pt.x = pt.y = -1;
	} else {
    pt.x = (m_dwCaret - dwOrg)%16;
    int dump_x = GetDUMP_X(m_dwCaret);
    int char_x = dump_x + 3*16 + 1;
		ptx2 = pt.x + char_x;
		pt.x = pt.x*3 + dump_x;
		if(m_bCaretOnChar) Swap(pt.x, ptx2);
    long ptOrgX = m_ptOffset.x/m_cellH;
    pt.x -= ptOrgX;
    ptx2 -= ptOrgX;
		pt.y = (m_dwCaret - dwOrg)/16 + DUMP_Y;
    if(m_dwCaret >= dwMax2)
    {
      ScrollBy(0, 1, !m_bBlock);
      dwOrg += 16;
      if(pScrolldy)*pScrolldy=1;
    }
		bDraw = TRUE;
	}
	MoveCaret(pt);
	pt.x = ptx2;
	MoveCaret2(pt);
	
	if(GetMainFrame() && GetMainFrame()->m_bInspectView) {
		CBZInspectView* pView = (CBZInspectView*)GetSubView();
		pView->Update();
	}

	return bDraw;
}

UINT64 CBZView::PointToOffset(WTL::CPoint pt)
{
  WTL::CRect r;
	GetClientRect(&r);
	if(!r.PtInRect(pt))
		return _UI64_MAX;
	PixelToGrid(pt);
  
	pt.x += m_ptOffset.x/m_cellH;
  UINT64 pty = GetScrollPosU64V() + pt.y;
	if(pty < DUMP_Y) // || pt.x == CHAR_X-1 || pt.x >= CHAR_X+16)	// ###1.5
 		return _UI64_MAX;
  int dump_x = GetDUMP_X(pty*16);
  int char_x = dump_x + 3*16 + 1;
	if(pt.x >= char_x) {
		if((pt.x -= char_x) > 16) pt.x = 16;
		m_bCaretOnChar = TRUE;
	} else {
		if(pt.x < dump_x)
			pt.x = 0;
		else
			pt.x -= dump_x;
		pt.x/=3;
		m_bCaretOnChar = FALSE;
	}
	UINT64 ofs = (pty - DUMP_Y)*16 + pt.x;
  UINT64 dwTotal = GetFileSize();
	return (ofs > dwTotal) ? dwTotal : ofs;
}

//m_dwOldCaret = m_dwCaret;しないとバグる
BOOL CBZView::GotoCaret()
{
	if(DrawCaret()) {
		if(m_bBlock)
			Invalidate(FALSE);
		return TRUE;
	}
	long ptx = 0;
	UINT64 pty = m_dwCaret/16 - 1;
  ScrollToPos(ptx, pty);
	Invalidate(FALSE);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBZView message handlers



void CBZView::OnDoubleClick()	// ### 1.62
{
	if(m_dwStruct) {
		CBZFormView* pView = (CBZFormView*)GetSubView();
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

  WTL::CRect rect;
  UINT64 v64 = GetScrollPosU64V();
  UINT64 dwOrg = v64 * 16;
  UINT64 dwOldCaret = m_dwCaret;
  if(dwOldCaret >= dwOrg)
  {
    GetClientRect(rect);
  }

  m_dwCaret = dwNewCaret;
  int scrolldy=0;
	if(!DrawCaret(&scrolldy)) {
    ScrollBy(0, dy, !m_bBlock);
    scrolldy += dy;
  }
  
  if(dwOldCaret >= dwOrg)
  {
    rect.top = ((dwOldCaret - dwOrg)/16 + DUMP_Y - scrolldy)*m_cellV;
    rect.bottom = rect.top + m_cellV;
    InvalidateRect(rect, FALSE);
  }
	if(m_bBlock)
		Invalidate(FALSE);

  UpdateWindow();
}

void CBZView::UpdateDocSize()
{
	//m_dwTotal = m_pDoc->GetDocSize();
  UINT64 dwTotal = GetFileSize();
	long cTotalX = VIEWCOLUMNS;
	UINT64 cTotalY = dwTotal / 16 + 2;
	SetTextSize(cTotalX, cTotalY);
	//Invalidate(FALSE);
}


void CBZView::ChangeFont(LOGFONT& logFont)
{
	delete m_pFont;
	m_pFont = new WTL::CFont;
	m_pFont->CreateFontIndirect(&logFont);
	OnChangeFont();
	//m_bResize = FALSE;
	Invalidate(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CBZView Update UI






void CBZView::UpdateStatusInfo()
{
	GetMainFrame()->m_statusbar.SetText(1, GetStatusInfoText());
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
		MessageBox(sMsg, _T("ERROR"), MB_ICONHAND);
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
      Invalidate();
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

void CBZView::Activate()
{
	GetMainFrame()->SetActiveView(this);
	if(GetFocus() != m_hWnd) SetFocus();	// ###1.62
}

/*
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
*/
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
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
      //ATLASSERT(pC);
      if(!pC)return;
			if(*pC < 0x21 || *pC > 0x7E) break;
		}
		while(n++ < ofs) {
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
      ATLASSERT(pC);
      if(!pC)return;
			GetCharCode(*pC);
		}
	} else if(m_charset == CTYPE_EUC) {
		GetCharCode(0);
		while(n) {
			n--;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
      //ATLASSERT(pC);
      if(!pC)return;
			if(*pC < 0xA1 || *pC > 0xFE) break;
		}
		while(n++ < ofs) {
      WORD w = 0;
      LPBYTE pC = m_pDoc->CacheForce(n, 1);
      ATLASSERT(pC);
      if(!pC)return;
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



BOOL CBZView::AskSave()
{
  if(m_pDoc)
    return m_pDoc->CloseDocument(this->m_hWnd);
  else
  {
    ATLASSERT(FALSE);
    return TRUE;
  }
}



void CBZView::ReCreateBackup()
{
	if(m_pDoc)
	{
//		m_pDoc->m_restoreCaret = m_dwCaret;
//		m_pDoc->m_restoreScroll = this->GetScrollPos();
	//	m_pDoc->m_restoreScroll.x = 0;
	}
}
void CBZView::ReCreateRestore()
{
	if(m_pDoc)
	{
	//	m_dwCaret = m_pDoc->m_restoreCaret;
	//	DrawCaret();
//		ScrollToPos(m_pDoc->m_restoreScroll);
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