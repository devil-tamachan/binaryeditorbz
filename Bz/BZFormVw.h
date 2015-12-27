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

#pragma once

#include "BZ.h"

#define NUM_MEMBERS 15

class CStructMember
{
public:
  static char* TYPESTR[NUM_MEMBERS];
  static DWORD SIZES[NUM_MEMBERS];

	CStructMember(LPSTR name = NULL, int type = 0)
	{
		m_name = name;
    if(type >=-1 && type <NUM_MEMBERS)m_type = type;
    else {
      ATLASSERT(false);
      m_type = -1;
    }
		m_len = m_bytes = (m_type == -1) ? 0 : SIZES[m_type];
    m_hTree = NULL;
	}
	CStructMember(CStringA& name, int type = 0)
	{
		m_name = name;
    if(type >=-1 && type <NUM_MEMBERS)m_type = type;
    else {
      ATLASSERT(false);
      m_type = -1;
    }
		m_len = m_bytes = (m_type == -1) ? 0 : SIZES[m_type];
    m_hTree = NULL;
	}
	~CStructMember()
	{
	}
	CStructMember(const CStructMember& r)
	{
		m_name = r.m_name;
    m_type = r.m_type;
		m_len  = r.m_len;
    m_bytes = r.m_bytes;
    m_ofs = r.m_ofs;
    m_hTree = NULL;
	}
	CStructMember& operator=(const CStructMember& r)
	{
		m_name = r.m_name;
    m_type = r.m_type;
		m_len  = r.m_len;
    m_bytes = r.m_bytes;
    m_ofs = r.m_ofs;
		return *this;
	}
	CString m_name;
	int		m_type;
	DWORD	m_len;
	DWORD	m_bytes;
	DWORD	m_ofs;
  HTREEITEM m_hTree;

  CString GetLabelText()
  {
    CString s;
    s.Format(_T("%s (+%2X)"), m_name, m_ofs);
    return s;
  }
  CString GetValText(CBZView *pView);
};

class CStructTag
{
public:
	CStructTag(LPSTR name = NULL)
	{
		m_name = name;
		m_len = 0;
	}
/*	CStructTag(CStringA& name)
	{
		m_name = name;
		m_len = 0;
	}*/
	CStructTag(LPSTR name, unsigned int max)
	{
		char *tmp = (char*)malloc(max+1);
		lstrcpynA(tmp, name, max+1);
		m_name = tmp;
		free(tmp);
		//m_name.SetString(name, max);
		m_len = 0;
	}
	CStructTag(const CStructTag& r)
	{
		m_name = r.m_name;
		m_len  = r.m_len;
    m_member.Copy(r.m_member);
    m_sarrFileExt.Copy(r.m_sarrFileExt);
  }
	~CStructTag()
	{
	}
	CStructTag& operator=(const CStructTag& r)
	{
		m_name = r.m_name;
		m_len  = r.m_len;
    m_member.Copy(r.m_member);
    m_sarrFileExt.Copy(r.m_sarrFileExt);
		return *this;
	}
	CString m_name;
	DWORD	m_len;
	CAtlArray<CStructMember> m_member;
	CAtlArray<CString> m_sarrFileExt;		// ###1.61
};

#include "BZSubView.h"

class CBZView;

class CBZFormView : public CDialogImpl<CBZFormView>, public WTL::CWinDataExchange<CBZFormView>, public CBZSubView
{
public:
  CBZFormView()
  {
    m_pDefFile = NULL;
    m_bNoDefFile = FALSE;
    m_iTag = -1;
  }

  virtual ~CBZFormView()
  {
    if(m_pDefFile)
      delete m_pDefFile;	// ##1.5
  }

public:
  enum { IDD = IDD_DIALOG1 };

  BEGIN_MSG_MAP(CBZFormView)
    //MSG_WM_CREATE(OnCreate)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    COMMAND_HANDLER_EX(IDC_LIST_TAG, LBN_SELCHANGE, OnSelchangeListTag)
    MSG_WM_SIZE(OnSize)
    MSG_WM_KEYDOWN(OnKeyDown)
    NOTIFY_HANDLER_EX(IDC_TREE_MEMBER, NM_DBLCLK, OnDblclkListMember)
    NOTIFY_HANDLER_EX(IDC_TREE_MEMBER, TVN_SELCHANGED, OnMemberSelChanged)
    COMMAND_ID_HANDLER_EX(IDB_TAGALL, OnClickTagAll)
  END_MSG_MAP()

  BEGIN_DDX_MAP(CBZFormView)
    DDX_CONTROL_HANDLE(IDC_LIST_TAG, m_listTag);
    DDX_CONTROL(IDC_TREE_MEMBER, m_treeListMember);
    DDX_CONTROL_HANDLE(IDC_STATIC_MEMBER, m_statMember);
    DDX_CHECK(IDB_TAGALL, m_bTagAll);
  END_DDX_MAP()

	WTL::CListBox	m_listTag;
	WTL::CStatic		m_statMember;
	BOOL		m_bTagAll;

  
	CTreeListViewCtrl	m_treeListMember;
  WTL::CHeaderCtrl m_headerMember;
  WTL::CTreeViewCtrl m_treeMember;

  static TCHAR *MEMBER_LABELS_STR[MBRCOL2_MAX];

// Attributes
private:
	LPSTR	m_pDefFile;
	BOOL m_bNoDefFile;
	CAtlArray<CStructTag> m_tag;
	CBZView* m_pView;
	int m_nTagSelect; //ファイル開く後に復帰用？いらないかもｗ
  int m_iTag;

// Operations
public:
	void Activate();
	void SelectTag();

public:
  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
  //virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
  //{
  //  InitListTag();
  //}

protected:

	HRESULT ParseMember(CStringA& member, int iTag, int iType, CString& errMsg);
	HRESULT ParseMemberLine(CStringA& memberline, int iTag, CString& errMsg);
	BOOL InitStructList();
	void InitListTag();
	void InitListMember(int iTag);
  void UpdateMembers();
	int AddTag(int iTag);

public:
  CBZView* GetBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZViewFromSubView(this);
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

  void InitMembersHeader()
  {
      for_to(i, MBRCOL2_MAX)
      {
        HDITEM col = { 0 };
        col.mask = HDI_FORMAT | HDI_TEXT | HDI_WIDTH;
        col.fmt = HDF_LEFT;
        col.cxy = 250;
        col.pszText = MEMBER_LABELS_STR[i];
        col.cxy = options.colWidth2[i];
        if(col.cxy<5)col.cxy=5;
        m_headerMember.InsertItem(i, &col);
      }
  }

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
    m_bTagAll = options.bTagAll;
    DoDataExchange(DDX_LOAD);

    if(!m_pDefFile && !m_bNoDefFile) {
      CString sPath;
      sPath = GetStructFilePath(IDS_STRUCT_FILE);
      if(!(m_pDefFile = (LPSTR)ReadFile(sPath))) {
        m_bNoDefFile = TRUE;
        return TRUE;
      }
      if(!InitStructList()) {
        m_bNoDefFile = TRUE;
        return TRUE;
      }
      InitListTag();

//      ListView_SetExtendedListViewStyle(m_listMember.m_hWnd, LVS_EX_FULLROWSELECT);
      m_headerMember = m_treeListMember.GetHeaderControl();
      m_treeMember = m_treeListMember.GetTreeControl();

      InitMembersHeader();
    }
    m_pView = GetBZView();

    LONG lExStyle = GetWindowLong(GWL_EXSTYLE);
    lExStyle |= WS_EX_STATICEDGE;
    SetWindowLong(GWL_EXSTYLE, lExStyle);

    ShowWindow(SW_SHOW);

    return TRUE;
  }

  void InitSubView()
  {
    SelectTag();
  }

  /*int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    if(options.xSplitStruct == 0)
      options.xSplitStruct = lpCreateStruct->cx;

    return 0;
  }*/
	void OnDestroy()
  {
    if(!m_bNoDefFile)
    {
      for_to(i, MBRCOL2_MAX)
      {
        options.colWidth2[i] = GetHeaderColWidth(i);
      }
    }
  }


  void OnClickTagAll(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    DoDataExchange(DDX_SAVE);
    options.bTagAll = m_bTagAll;
    InitListTag();
  }

  void OnSelchangeListTag(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    if(m_listTag.GetCount()==0)return;
    SelchangeListTag();
  }

  void SelchangeListTag();


  int GetHeaderColWidth(int nIndex)
  {
      WTL::CRect rect;
      m_headerMember.GetItemRect(nIndex, rect);
      return rect.Width();
  }
  void SetHeaderColWidth(int nIndex, int w)
  {
      HDITEM hdi = {0};
      m_headerMember.GetItem(nIndex, &hdi);
      hdi.cxy = w;
      m_headerMember.SetItem(nIndex, &hdi);
  }

  void OnSize(UINT nType, WTL::CSize size)
  {
    //SetScrollSizes(MM_TEXT, size);
    int cx = size.cx;
    int cy = size.cy;

    if(m_listTag.m_hWnd && m_listTag.m_hWnd) {
      WTL::CRect rForm, rTag, rMember, rStatic;
      GetWindowRect(rForm);
      m_listTag.GetWindowRect(rTag);
      m_treeListMember.GetWindowRect(rMember);
      m_statMember.GetWindowRect(rStatic);
      ScreenToClient(rTag);
      ScreenToClient(rMember);
      ScreenToClient(rStatic);

      int nMgn = rTag.x1;
      rTag.x2 = cx - rTag.x1;
      rMember.x2 = cx - nMgn;

      int ySplit = cy / 3;
      int dy = ySplit - rStatic.y1;
      rTag.y2 = ySplit - nMgn;
      rStatic.y1 += dy;
      rStatic.y2 += dy;
      rMember.y1 += dy;
      rMember.y2 = cy - nMgn;

      m_listTag.MoveWindow(rTag);
      m_treeListMember.MoveWindow(rMember);
      m_statMember.MoveWindow(rStatic);
      m_statMember.Invalidate();

      m_treeListMember.GetClientRect(rMember);
      int cxLabel = rMember.Width() /*-GetHeaderColWidth(MBRCOL2_OFFSET)*/ - GetHeaderColWidth(MBRCOL2_VALUE);
      if(cxLabel > 0)
        SetHeaderColWidth(MBRCOL2_LABEL, cxLabel);

      WTL::CRect rTagAll;
      CWindow pWndTagAll = GetDlgItem(IDB_TAGALL);
      pWndTagAll.GetWindowRect(rTagAll);
      ScreenToClient(rTagAll);
      int cxTagAll = rTagAll.Width();
      rTagAll.right = rTag.right;
      rTagAll.left = rTag.right - cxTagAll;
      pWndTagAll.MoveWindow(rTagAll);
    }
  }
  LRESULT OnDblclkListMember(LPNMHDR pnmh);
	LRESULT OnMemberSelChanged(LPNMHDR pnmh);

  BOOL IsValidiTag()
  {
    return (m_iTag >= 0 && m_iTag < m_tag.GetCount());
  }

  int GetWindowIdealWidth()
  {
    return 310.0f * CBZDpi::ScaleX();
  }
};

/////////////////////////////////////////////////////////////////////////////
