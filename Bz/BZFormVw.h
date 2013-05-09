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


#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include <afxtempl.h>

extern DWORD g_datasizes[];

class CStructMember
{
public:
	CStructMember(LPSTR name = NULL, int type = 0)
	{
		m_name = name;
		m_type = type;
		m_len = m_bytes = (type == -1) ? 0 : g_datasizes[type];
	}
	CStructMember(CString& name, int type = 0)
	{
		m_name = name;
		m_type = type;
		m_len = m_bytes = (type == -1) ? 0 : g_datasizes[type];
	}
	~CStructMember()
	{
	}
	CString m_name;
	int		m_type;
	DWORD	m_len;
	DWORD	m_bytes;
	DWORD	m_ofs;
};

class CStructTag
{
public:
	CStructTag(LPSTR name = NULL)
	{
		m_name = name;
		m_len = 0;
	}
	CStructTag(CString& name)
	{
		m_name = name;
		m_len = 0;
	}
	CStructTag(LPSTR name, unsigned int max)
	{
		m_name.SetString(name, max);
		m_len = 0;
	}
	~CStructTag()
	{
	}
	CStructTag& operator=(const CStructTag& r)
	{
		m_name = r.m_name;
		m_len  = r.m_len;
		return *this;
	}
	CString m_name;
	DWORD	m_len;
	CArray<CStructMember, CStructMember&> m_member;
	CStringArray m_sarrFileExt;		// ###1.61
};

class CBZFormView : public CFormView
{
protected:
	CBZFormView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBZFormView)

// Form Data
public:
	//{{AFX_DATA(CBZFormView)
	enum { IDD = IDD_DIALOG1 };
	CListBox	m_listTag;
	CListCtrl	m_listMember;
	CStatic		m_statMember;
	BOOL		m_bTagAll;
	//}}AFX_DATA

// Attributes
private:
	LPSTR	m_pDefFile;
	BOOL m_bNoDefFile;
	CArray<CStructTag, CStructTag&> m_tag;
	CBZView* m_pView;
	int m_nTagSelect;

// Operations
public:
	void Activate();
	void SelectTag();

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZFormView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBZFormView();

	HRESULT ParseMember(CString& member, int iTag, int iType, CString& errMsg);
	HRESULT ParseMemberLine(CString& memberline, int iTag, CString& errMsg);
	BOOL InitStructList();
	void InitListTag();
	void InitListMember(int iTag);
	int AddTag(int iTag);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
public:
	// Generated message map functions
	//{{AFX_MSG(CBZFormView)
	afx_msg void OnSelchangeListTag();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkListMember(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemchangedListMember(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnClickTagAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
