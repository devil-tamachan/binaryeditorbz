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

class CBZDoc2;

enum CutMode { EDIT_COPY, EDIT_CUT, EDIT_DELETE };
class CBZView : public CTextView
{
protected: // create from serialization only
	CBZView();
	DECLARE_DYNCREATE(CBZView)

// Attributes
public:
	//DWORD	m_dwTotal;
	UINT64	m_dwCaret;
	DWORD	m_dwStruct;
	DWORD	m_dwStructTag;
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
	DWORD	m_dwPrint;
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
  DWORD GetFileSize();
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
	DWORD	PointToOffset(CPoint pt);
	void	CutOrCopy(CutMode mode);
public:
	void	MoveCaretTo(DWORD dwNewCaret/*, bool bFirst=true*/);
private:
	void	UpdateDocSize();
	BOOL	CalcHexa(LPCSTR sExp, long& nResult);
	int		ReadHexa(LPCSTR sHexa, LPBYTE& buffer);
//	void	InitMark();  -->CBZDoc
	void	SetMark();
	void	JumpToMark();
//	BOOL	CheckMark(DWORD dwPtr);
	UINT64	BlockBegin() { return m_dwBlock < m_dwCaret ? m_dwBlock : m_dwCaret; };
	UINT64	BlockEnd() { return m_dwBlock > m_dwCaret ? m_dwBlock : m_dwCaret; } ;
	CBZView* GetBrotherView();
	CBZDoc2* GetBrotherDoc();
  DWORD GetRemainFromCurret();
	void	ChangeFont(LOGFONT& logFont);
	//void	SetValue(LPBYTE p, int bytes, int val);
	//BOOL	IsMBS(LPBYTE pTop, DWORD ofs, BOOL bTrail);
	CharSet AutoDetectCharSet();
	int ConvertCharSet(CharSet charset, LPCSTR sFind, LPBYTE &buffer);
	CharSet DetectCodeType(DWORD dwStart = 0, DWORD dwMaxSize = 0xFFFFffff);//(LPBYTE p, LPBYTE pEnd);
	void InitCharMode(DWORD ofs);
	WORD GetCharCode(WORD c, DWORD ofs = 0);
	void SetColor(TextColor n = TCOLOR_TEXT);
	void SetHeaderColor();
	CString GetStatusInfoText();
	void JumpTo(DWORD dwNewCaret);
	void SetMaxPrintingPage(CPrintInfo* pInfo);		// ### 1.54
	void PutUnicodeChar(WORD w);					// ### 1.54b
	void OnDoubleClick();							// ### 1.62
	static BOOL LoadEbcDicTable();					// ### 1.63
	UCHAR ConvertEbcDic(UCHAR c);

	// Quick Search Algorithm
	void preQuickSearchWI1(LPCWSTR searchTextW, BYTE nSearchTextW, BYTE *skipTable);
	DWORD stristrBinaryW1(LPCWSTR searchTextW, BYTE nSearchTextW, DWORD dwStart);
	void preQuickSearchWI4(LPCWSTR searchTextW, DWORD nSearchTextW, DWORD *skipTable);
	DWORD stristrBinaryW4(LPCWSTR searchTextW, DWORD nSearchTextW, DWORD dwStart);
	DWORD stristrBinaryW(LPCWSTR searchTextW, DWORD nSearchTextW, DWORD dwStart);
	void preQuickSearchAI(LPCSTR searchText, DWORD nSearchText, DWORD *skipTable);
	DWORD stristrBinaryA(LPCSTR searchText, DWORD dwStart);
	void preQuickSearch(LPBYTE searchByte, unsigned int nSearchByte, DWORD* skipTable);
	DWORD strstrBinary(LPBYTE searchByte, unsigned int nSearchByte, DWORD dwStart);

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

// Implementation
public:
	virtual ~CBZView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBZView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewFont();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnJumpOffset();
	afx_msg void OnJumpReturn();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnJumpFindnext();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnJumpCompare();
	afx_msg void OnUpdateJumpCompare(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditValue(CCmdUI* pCmdUI);
	afx_msg void OnCharAutoDetect();
	afx_msg void OnUpdateCharAutoDetect(CCmdUI* pCmdUI);
	afx_msg void OnViewColor();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnJumpStart();
	afx_msg void OnJumpEnd();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnEditCopyDump();
	afx_msg void OnUpdateEditCopyDump(CCmdUI* pCmdUI);
	afx_msg void OnJumpBase();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg void OnCharMode(UINT nID);
	afx_msg void OnUpdateCharMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusInfo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusSize(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusChar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusIns(CCmdUI* pCmdUI);
	afx_msg void OnStatusInfo();
	afx_msg void OnStatusSize();
	afx_msg void OnStatusChar();
	afx_msg void OnByteOrder(UINT nID);
	afx_msg void OnUpdateByteOrder(CCmdUI* pCmdUI);
	afx_msg void OnUpdateJump(CCmdUI* pCmdUI);
//	afx_msg LRESULT OnFindNext(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewGrid1();
	afx_msg void OnUpdateViewGrid1(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in BZView.cpp
inline CBZDoc2* CBZView::GetDocument()
   { return (CBZDoc2*)m_pDocument; }
#endif



/////////////////////////////////////////////////////////////////////////////
