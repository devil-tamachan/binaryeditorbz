// BZFormVw.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBZFormView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include <afxtempl.h>

class CStructMember
{
public:
	CStructMember(LPSTR name = NULL, int type = 0);
	~CStructMember();
	CString m_name;
	int		m_type;
	DWORD	m_len;
	DWORD	m_bytes;
	DWORD	m_ofs;
};

class CStructTag
{
public:
	CStructTag(LPSTR name = NULL);
	~CStructTag();
	CStructTag& operator=(const CStructTag&);
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
