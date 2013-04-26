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

#include "Combobar.h"
//#include "Splitter.h"
#include "StatBar.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	BOOL m_bDisableStatusInfo;
protected:
	CSplitterWnd* m_pSplitter;
	int m_nPaneWidth;

public:
	BOOL m_bBmpView;
	BOOL m_bStructView;
	BOOL m_bInspectView;
	BOOL m_bAnalyzerView;
	UINT m_nSplitView;
	UINT m_nSplitView0;
	BOOL m_bCompare;

// Operations
public:
	void ChangeView(CView* pView);
	CView *GetBrotherView(CView* pView);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle = TRUE);
	BOOL CreateClient(CCreateContext* pContext = NULL);
	void GetSplitInfo();
	void GetFrameState();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CStatusBarEx m_wndStatusBar;
	CComboToolBar m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnJumpFind();
	afx_msg void OnViewBitmap();
	afx_msg void OnUpdateViewBitmap(CCmdUI* pCmdUI);
	afx_msg void OnJumpTo();
	afx_msg void OnViewStruct();
	afx_msg void OnEditValue();
	afx_msg void OnUpdateViewStruct(CCmdUI* pCmdUI);
	afx_msg void OnViewFullpath();
	afx_msg void OnUpdateViewFullpath(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnToolsSetting();
	afx_msg void OnViewSubCursor();
	afx_msg void OnUpdateViewSubCursor(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnViewSplit(UINT nID);
	afx_msg void OnUpdateViewSplit(CCmdUI* pCmdUI);
	afx_msg void OnViewSyncScroll();
	afx_msg void OnUpdateViewSyncScroll(CCmdUI* pCmdUI);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewInspect();
	afx_msg void OnUpdateViewInspect(CCmdUI *pCmdUI);

	void UpdateInspectViewChecks();
	void DeleteSplitterWnd(CCreateContext* pContext);
	afx_msg void OnViewAnalyzer();
	afx_msg void OnUpdateViewAnalyzer(CCmdUI *pCmdUI);
	afx_msg void OnHelpIndex();
};

#define BZ_CLASSNAME "BzEditorClass"

inline CMainFrame* GetMainFrame() { return (CMainFrame*)AfxGetMainWnd(); };

/////////////////////////////////////////////////////////////////////////////

