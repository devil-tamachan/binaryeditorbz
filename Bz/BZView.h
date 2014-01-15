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

#include "TextView.h"
#include "ColorDlg.h"
#include "BZFormVw.h"

class CBZDoc2;
class CMainFrame;
class CInputDlg;

static DWORD  MemCompByte2(LPCVOID p1, LPCVOID p2, DWORD len);

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

enum CutMode { EDIT_COPY, EDIT_CUT, EDIT_DELETE };

class CBZView : public CTextView, public WTL::CUpdateUI<CBZView>
{
public:
  BEGIN_MSG_MAP(CBZView)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_TIMER(OnTimer)
    MSG_WM_KEYDOWN(OnKeyDown)
    MSG_WM_CHAR(OnChar)
    MSG_WM_MBUTTONDOWN(OnMButtonDown)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_MOUSEWHEEL(OnMouseWheel)
    MSG_WM_VSCROLL(OnVScroll)
    MSG_WM_INITMENUPOPUP(OnInitMenuPopup)

    COMMAND_ID_HANDLER_EX(ID_VIEW_FONT, OnViewFont)
    COMMAND_ID_HANDLER_EX(ID_JUMP_START, OnJumpStart)
    COMMAND_ID_HANDLER_EX(ID_JUMP_END, OnJumpEnd)
    COMMAND_ID_HANDLER_EX(ID_JUMP_OFFSET, OnJumpOffset)
    COMMAND_ID_HANDLER_EX(ID_JUMP_RETURN, OnJumpReturn)
    COMMAND_ID_HANDLER_EX(ID_JUMP_FINDNEXT, OnJumpFindnext)
    COMMAND_ID_HANDLER_EX(ID_JUMP_COMPARE, OnJumpCompare)
    COMMAND_ID_HANDLER_EX(ID_EDIT_UNDO, OnEditUndo)
    COMMAND_ID_HANDLER_EX(ID_EDIT_REDO, OnEditRedo)
    COMMAND_ID_HANDLER_EX(ID_EDIT_CUT, OnEditCut)
    COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)
    COMMAND_ID_HANDLER_EX(ID_EDIT_PASTE, OnEditPaste)
    COMMAND_ID_HANDLER_EX(ID_CHAR_AUTODETECT, OnCharAutoDetect)
    COMMAND_ID_HANDLER_EX(ID_VIEW_COLOR, OnViewColor)
    COMMAND_ID_HANDLER_EX(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    COMMAND_ID_HANDLER_EX(ID_EDIT_COPY_DUMP, OnEditCopyDump)
    COMMAND_ID_HANDLER_EX(ID_JUMP_BASE, OnJumpBase)
    COMMAND_ID_HANDLER_EX(ID_JUMP_MARK, SetMark)
    COMMAND_ID_HANDLER_EX(ID_JUMP_MARKNEXT, JumpToMark)
    COMMAND_RANGE_HANDLER_EX(ID_CHAR_ASCII, ID_CHAR_LAST, OnCharMode)
    COMMAND_ID_HANDLER_EX(ID_INDICATOR_INFO, OnStatusInfo)
    COMMAND_ID_HANDLER_EX(ID_INDICATOR_SIZE, OnStatusSize)
    COMMAND_ID_HANDLER_EX(ID_INDICATOR_CHAR, OnStatusChar)
    COMMAND_RANGE_HANDLER_EX(ID_BYTEORDER_INTEL, ID_BYTEORDER_68K, OnByteOrder)
    COMMAND_ID_HANDLER_EX(ID_VIEW_GRID1, OnViewGrid1)
    CHAIN_MSG_MAP(CUpdateUI<CBZView>)
    CHAIN_MSG_MAP(CTextView)
  END_MSG_MAP()

  BEGIN_UPDATE_UI_MAP(CBZView)
    UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
    //UPDATE_ELEMENT(ID_JUMP_COMPARE, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_VALUE, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_AUTODETECT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_SELECT_ALL, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_EDIT_COPY_DUMP, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_ASCII, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_SJIS, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_UNICODE, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_JIS, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_EUC, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_UTF8, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_EBCDIC, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_CHAR_EPWING, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_INDICATOR_INFO, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_INDICATOR_SIZE, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_INDICATOR_CHAR, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_INDICATOR_INS, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_BYTEORDER_INTEL, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_BYTEORDER_68K, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_COMPARE, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_TO, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_OFFSET, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_RETURN, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_MARK, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_MARKNEXT, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_START, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_JUMP_END, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_GRID1, UPDUI_MENUPOPUP)
  END_UPDATE_UI_MAP()

  void OnUpdateEditCut();
  void OnUpdateEditCopy() { UIEnable(ID_EDIT_COPY, m_bBlock); }
  void OnUpdateEditPaste();
  void OnUpdateEditValue();
  void OnUpdateCharAutoDetect() { UISetCheck(ID_CHAR_AUTODETECT, options.bAutoDetect); }
  void OnUpdateEditSelectAll();
  void OnUpdateEditCopyDump()   { UIEnable(ID_EDIT_COPY_DUMP, m_bBlock); }
  void OnUpdateCharMode()       { UISetRadioMenuItem((UINT)(options.charset + ID_CHAR_ASCII), ID_CHAR_ASCII, ID_CHAR_LAST); }
  void OnUpdateStatusInfo();
  void OnUpdateStatusSize()
  {
    UINT64 dwTotal = GetFileSize();
    if(dwTotal) {
      CString sResult;
      if(m_bHexSize)
        sResult.Format(_T("0x%I64X"), dwTotal);
      else
        sResult = SeparateByComma64(dwTotal);
      sResult += _T(" bytes");
      UISetText(ID_INDICATOR_SIZE, sResult);
      UIEnable(ID_INDICATOR_SIZE, TRUE);
    } else
      UIEnable(ID_INDICATOR_SIZE, FALSE);
  }
  void OnUpdateStatusChar()
  {
    static TCHAR *sCharSet[CTYPE_COUNT] = { _T("ASCII"), _T("S-JIS"), _T("UTF-16"), _T("JIS"), _T("EUC"), _T("UTF-8"), _T("EBCDIC"), _T("EPWING") };
    UISetText(ID_INDICATOR_CHAR, sCharSet[m_charset]);
    UIEnable(ID_INDICATOR_CHAR, TRUE);
  }
  void OnUpdateStatusIns();
  void OnUpdateByteOrder() { UISetRadioMenuItem((UINT)options.bByteOrder + ID_BYTEORDER_INTEL, ID_BYTEORDER_INTEL, ID_BYTEORDER_68K); }
  void OnUpdateJump();
  void OnUpdateViewGrid1() { UISetCheck(ID_VIEW_GRID1, options.iGrid==1); }
  void OnInitMenuPopup(WTL::CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
  {
    OnUpdateEditCut();
    OnUpdateEditCopy();
    OnUpdateEditPaste();
    //OnUpdateJumpCompare();
    OnUpdateEditValue();
    OnUpdateCharAutoDetect();
    OnUpdateEditSelectAll();
    OnUpdateEditCopyDump();
    OnUpdateCharMode();
    OnUpdateStatusInfo();
    OnUpdateStatusSize();
    OnUpdateStatusChar();
    OnUpdateStatusIns();
    OnUpdateByteOrder();
    OnUpdateJump();
    OnUpdateViewGrid1();
    SetMsgHandled(FALSE);
  }

public:
  void OnViewFont(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    LOGFONT logFont;
    m_pFont->GetLogFont(&logFont);
    WTL::CFontDialog aFontDlg(&logFont, CF_SCREENFONTS | CF_FIXEDPITCHONLY | CF_NOVERTFONTS | CF_ANSIONLY); // ###1.5
    int rDlg = aFontDlg.DoModal();
    if(rDlg == IDOK) {
      options.sFontName = aFontDlg.GetFaceName();
      options.fFontStyle = aFontDlg.IsBold() + aFontDlg.IsItalic()*2;
      options.nFontSize = aFontDlg.GetSize();
      ChangeFont(logFont);
      CBZView* pView = GetBrotherView();
      if(pView) pView->ChangeFont(logFont);
    }
  }
  void OnJumpStart(UINT uNotifyCode, int nID, CWindow wndCtl) { JumpTo(0); }
  void OnJumpEnd(UINT uNotifyCode, int nID, CWindow wndCtl)   { JumpTo(GetFileSize()); }
  void OnJumpOffset(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    int nBytes = m_nBytes;
    if(m_nBytes>4)
    {
      nBytes = 4;
    }
    int ofs = GetValue(m_dwCaret, nBytes);
    ofs += nBytes;
    if(m_dwCaret + ofs <= GetFileSize())
      JumpTo(m_dwCaret + ofs);
  }
  void OnJumpReturn(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    if(GotoCaret()) {
      Swap(m_dwCaret, m_dwOldCaret);
      GotoCaret();
    }
  }
  void OnJumpFindnext(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnJumpCompare(UINT uNotifyCode = 0, int nID = 0, CWindow wndCtl = NULL);
  void OnEditUndo(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnEditRedo(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnEditCut(UINT uNotifyCode, int nID, CWindow wndCtl) { CutOrCopy(EDIT_CUT); }
  void OnEditCopy(UINT uNotifyCode, int nID, CWindow wndCtl){ CutOrCopy(EDIT_COPY); }
  void OnEditPaste(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnCharAutoDetect(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bAutoDetect = !options.bAutoDetect;
  }
  void OnViewColor(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnEditSelectAll(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    m_bBlock = TRUE;
    m_dwBlock = 0;
    m_dwOldCaret = m_dwCaret;
    m_dwCaret = GetFileSize();
    GotoCaret();
  }
  void OnEditCopyDump(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    CMemFile memFile(MEMFILE_GROWBY);
    DrawToFile(&memFile);

    ULONGLONG ulSize = memFile.GetLength();
    DWORD dwSize = (ulSize>0xffffffff) ? 0xffffffff : ulSize;
    HGLOBAL hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + 1);
    LPBYTE pMemTxt  = (LPBYTE)::GlobalLock(hMemTxt);
    memFile.SeekToBegin();
    memFile.Read(pMemTxt, dwSize);
    *(pMemTxt + dwSize) = '\0';
    ::GlobalUnlock(hMemTxt);
    AfxGetMainWnd()->OpenClipboard();
    ::EmptyClipboard();
    ::SetClipboardData(CF_TEXT, hMemTxt);
    ::CloseClipboard();
    return;
  }
  void OnJumpBase(UINT uNotifyCode, int nID, CWindow wndCtl);
  void SetMark(UINT uNotifyCode, int nID, CWindow wndCtl);
  void JumpToMark(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnCharMode(UINT uNotifyCode = 0, int nID = 0, CWindow wndCtl = NULL)
  {
    m_charset = options.charset = (CharSet)(nID - ID_CHAR_ASCII);
    if(m_charset == CTYPE_EBCDIC)
      LoadEbcDicTable();
    options.Touch();
    Invalidate(TRUE);
    CBZView* pView = GetBrotherView();
    if(pView) {
      pView->m_charset = options.charset;
      pView->Invalidate(TRUE);
    }
  }
  void OnStatusInfo(UINT uNotifyCode, int nID, CWindow wndCtl) { if((m_nBytes*=2) == 8) m_nBytes = 1; }
  void OnStatusSize(UINT uNotifyCode, int nID, CWindow wndCtl) { m_bHexSize = !m_bHexSize; }
  void OnStatusChar(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    m_charset = (CharSet)(m_charset + 1);
    if(m_charset >= CTYPE_COUNT) m_charset = CTYPE_ASCII;
    OnCharMode(m_charset + ID_CHAR_ASCII);
  }
  void OnByteOrder(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnViewGrid1(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.iGrid = (options.iGrid==0)?1:0;
    Invalidate();
  }



public:
  int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    m_nColAddr = ADDRCOLUMNS;
    SetViewSize(CSize(VIEWCOLUMNS, 0));
    SetMsgHandled(FALSE);
    return 0;
  }
  BOOL OnEraseBkgnd(WTL::CDCHandle dc)
  {
    COLORREF rgbBG = options.colors[TCOLOR_TEXT][1];
    if(!IsSystemColor(rgbBG)) {
      WTL::CBrush brushBG;
      brushBG.CreateSolidBrush(rgbBG);
      CRect rcErase;
      dc.GetClipBox(rcErase);
      dc.FillRect(rcErase, brushBG);
      return TRUE;
    }
    SetMsgHandled(FALSE);
    return TRUE;
  }
  void OnTimer(UINT_PTR nIDEvent)
  {
    if(m_timer == TIMER_UP) OnKeyDown(VK_UP, 0, 0);
    else                    OnKeyDown(VK_DOWN, 0, 0);
  }
  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  void OnMButtonDown(UINT nFlags, CPoint point)
  {
    m_bCaretOnChar = !m_bCaretOnChar;
    GotoCaret();
    SetMsgHandled(FALSE);
  }
  void OnLButtonDown(UINT nFlags, CPoint point)
  {
    if(m_bBlock) {
      m_bBlock = FALSE;
      Invalidate(FALSE);
    }
    BOOL bOnChar = m_bCaretOnChar;
    UINT64 ofs = PointToOffset(point);
    if(ofs != UINT_MAX) {
      if(m_dwCaret != ofs || bOnChar != m_bCaretOnChar) {
        m_dwOldCaret = m_dwCaret;
        if(nFlags & MK_SHIFT) {
          m_bBlock = TRUE;
          Invalidate(FALSE);
        } else
          m_dwBlock = ofs;
        m_dwCaret = ofs;
        DrawCaret();
      }
      SetCapture();
    }
    m_bEnterVal = FALSE;
    SetMsgHandled(FALSE);
  }
  void OnLButtonUp(UINT nFlags, CPoint point)
  {
    ReleaseCapture();
    if(m_timer) {
      KillTimer(m_timer);
      m_timer = 0;
    }
    SetMsgHandled(FALSE);
  }
  void OnLButtonDblClk(UINT nFlags, CPoint point)
  {
    OnDoubleClick();
    SetMsgHandled(FALSE);
  }
  void OnMouseMove(UINT nFlags, CPoint point)
  {
    if(nFlags & MK_LBUTTON) {
      UINT64 ofs = PointToOffset(point);
      if(ofs != UINT_MAX) {
        if(m_timer) {
          KillTimer(m_timer);
          m_timer = 0;
        }
        if(m_dwCaret != ofs && m_dwBlock != ofs) {
          UINT64 dwCaretOld = m_dwCaret;		// ###1.5
          POINT ptCaretOld = m_ptCaret;
          m_dwCaret = ofs;
          m_bBlock = TRUE;
          if(DrawCaret())
            Invalidate(FALSE);
          else {
            m_dwCaret = dwCaretOld;
            m_ptCaret = ptCaretOld;
          }
        }
      } else {
        RECT r;
        GetClientRect(&r);
        if((point.y < 0 || point.y > r.y2) && !m_timer)
          m_timer = SetTimer(point.y < 0 ? TIMER_UP : TIMER_DOWN, 50, NULL);
      }
    }
    SetMsgHandled(FALSE);
  }
  BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
  {
    BOOL ret = CTextView::OnMouseWheel(nFlags, zDelta, pt);

    CBZView *pActiveView = GetActiveBZView();
    if(options.bSyncScroll && pActiveView==this)
    {
      CBZView* pView1 = GetBrotherView();
      if(pView1)
      {
        POINT pt = GetScrollPos();
        pView1->ScrollToPos(pt);
        pView1->Invalidate();
        //pView1->OnVScroll(nSBCode, nPos, NULL);
        //	TRACE("OnVScroll: m_dwCaret=%d\n", m_dwCaret);
      }
    }
    return ret;
  }
  void OnVScroll(UINT nSBCode, UINT nPos, WTL::CScrollBar pScrollBar)
  {
    CTextView::OnVScroll(nSBCode, nPos, pScrollBar);

    CBZView *pActiveView = GetActiveBZView();
    if(options.bSyncScroll && pActiveView==this)
    {
      CBZView* pView1 = GetBrotherView();
      if(pView1)
      {
        POINT pt = GetScrollPos();
        pView1->ScrollToPos(pt);
        pView1->Invalidate();
        //pView1->OnVScroll(nSBCode, nPos, NULL);
        //		TRACE("OnVScroll: m_dwCaret=%d\n", m_dwCaret);
      }
    }
  }


public:
  CBZView::CBZView()
  {
    //m_dwTotal = 0;
    m_timer = 0;
    m_nPageLen = 0;		// ### 1.54
    m_nBytesLength = 1;
  }
  virtual ~CBZView()
  {
    if(m_pEbcDic) {
      delete[] m_pEbcDic;
      m_pEbcDic = NULL;
      m_bLoadEbcDic = FALSE;
    }
  }

  CBZDoc2* GetBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZDoc2FromSubView(this);
  }
  CTamaSplitterWindow* GetSplitter()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSplitterWnd();
  }
  CMainFrame* GetMainFrame()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetMainFrame();
  }
  ATL::CWindow* GetSubView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSubViewFromBZView(this);
  }
  CBZView* GetActiveBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetActiveBZView();
  }
  CBZDoc2* GetActiveBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetActiveBZDoc2();
  }

public:
	UINT64	m_dwCaret;
	UINT64	m_dwStruct;
	UINT64	m_dwStructTag;
	int		m_nMember;
	int		m_nBytes;
	int		m_nBytesLength;	// ###1.61b
private:
	UINT64	m_dwBlock;
	BOOL	m_bBlock;
	UINT64	m_dwOldCaret;
	BOOL	m_bCaretOnChar;
	BOOL	m_bEnterVal;
	int		m_timer;
	CBZDoc2*	m_pDoc;
	UINT64	m_dwPrint;
	int		m_nPageLen;
	CharSet m_charset;
	static BOOL m_bHexSize;
	int		m_nColAddr;		// ###1.60
	static LPSTR m_pEbcDic;	// ###1.63
	static BOOL  m_bLoadEbcDic;

public:
	CBZDoc2*	GetDocument();
// Operations
public:
  UINT64 GetFileSize();
	BOOL	GotoCaret();
	int		GetValue(UINT64 ofs, int bytes);
	ULONGLONG GetValue64(UINT64 ofs);
	DWORD GetDWORD(UINT64 dwOffset);
  WORD  GetWORD(UINT64 dwOffset);
  BYTE  GetBYTE(UINT64 dwOffset);
	//void	SetValue(DWORD ofs, int bytes, int val);
	void	FillValue(int val);
	void	Activate();
	void	UpdateStatusInfo();
	void	DrawToFile(CFile* pFile);	// ###1.63

// Implementation
private:
	void	DrawHeader();
	void	DrawGrid(CDC* pDC, RECT& rClip);
	void	DrawDummyCaret(CDC* pDC);
	BOOL	DrawCaret();
	UINT64	PointToOffset(CPoint pt);
	void	CutOrCopy(CutMode mode);
public:
	void	MoveCaretTo(UINT64 dwNewCaret/*, bool bFirst=true*/);
private:
	void	UpdateDocSize();
	BOOL	CalcHexa(LPCSTR sExp, long& nResult);
	int		ReadHexa(LPCSTR sHexa, LPBYTE& buffer);
//	void	InitMark();  -->CBZDoc
//	BOOL	CheckMark(DWORD dwPtr);
	UINT64	BlockBegin() { return m_dwBlock < m_dwCaret ? m_dwBlock : m_dwCaret; };
	UINT64	BlockEnd() { return m_dwBlock > m_dwCaret ? m_dwBlock : m_dwCaret; } ;
	CBZView* GetBrotherView();
	CBZDoc2* GetBrotherDoc();
  UINT64 GetRemainFromCurret();
	void	ChangeFont(LOGFONT& logFont);
	//void	SetValue(LPBYTE p, int bytes, int val);
	//BOOL	IsMBS(LPBYTE pTop, DWORD ofs, BOOL bTrail);
	CharSet AutoDetectCharSet();
	int ConvertCharSet(CharSet charset, LPCSTR sFind, LPBYTE &buffer);
	CharSet DetectCodeType(UINT64 dwStart = 0, DWORD dwMaxSize = 0xFFFFffff);//(LPBYTE p, LPBYTE pEnd);
	void InitCharMode(UINT64 ofs);
	WORD GetCharCode(WORD c, UINT64 ofs = 0);
	void SetColor(TextColor n = TCOLOR_TEXT);
	void SetHeaderColor();
	CString GetStatusInfoText();
	void JumpTo(UINT64 dwNewCaret);
	void SetMaxPrintingPage(CPrintInfo* pInfo);		// ### 1.54
	void PutUnicodeChar(WORD w);					// ### 1.54b
	void OnDoubleClick();							// ### 1.62
	static BOOL LoadEbcDicTable();					// ### 1.63
	UCHAR ConvertEbcDic(UCHAR c);

	// Quick Search Algorithm
	void preQuickSearchWI1(LPCWSTR searchTextW, BYTE nSearchTextW, BYTE *skipTable);
	UINT64 stristrBinaryW1(LPCWSTR searchTextW, BYTE nSearchTextW, UINT64 dwStart);
	void preQuickSearchWI4(LPCWSTR searchTextW, DWORD nSearchTextW, DWORD *skipTable);
	UINT64 stristrBinaryW4(LPCWSTR searchTextW, DWORD nSearchTextW, UINT64 dwStart);
	UINT64 stristrBinaryW(LPCWSTR searchTextW, DWORD nSearchTextW, UINT64 dwStart);
	void preQuickSearchAI(LPCSTR searchText, DWORD nSearchText, DWORD *skipTable);
	UINT64 stristrBinaryA(LPCSTR searchText, UINT64 dwStart);
	void preQuickSearch(LPBYTE searchByte, unsigned int nSearchByte, DWORD* skipTable);
	UINT64 strstrBinary(LPBYTE searchByte, unsigned int nSearchByte, UINT64 dwStart);

public:
	void ReCreateBackup();
	void ReCreateRestore();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL
};

#ifndef _DEBUG  // debug version in BZView.cpp
inline CBZDoc2* CBZView::GetDocument()
   { return (CBZDoc2*)m_pDocument; }
#endif



/////////////////////////////////////////////////////////////////////////////
