// BZFormVw.cpp : implementation file
//

#include "stdafx.h"
#include "BZ.h"
#include "BZView.h"
#include "BZFormVw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define nTypes 13

char* datatypes[nTypes] = {
	"char", "byte", "BYTE", "short", "word", "WORD", "long", "dword", "DWORD", "double", "qword", "QWORD", "float"
};

DWORD datasizes[nTypes] = {1,1,1,2,2,2,4,4,4,8,8,8 ,4
};

static TCHAR *sMemberColLabel[MBRCOL_MAX] = { _T("+"), _T("Label"), _T("Value") };

/////////////////////////////////////////////////////////////////////////////
// CBZFormView

IMPLEMENT_DYNCREATE(CBZFormView, CFormView)

CBZFormView::CBZFormView() : CFormView(CBZFormView::IDD)
{
	m_pDefFile = NULL;
	m_bNoDefFile = FALSE;
	//{{AFX_DATA_INIT(CBZFormView)
	m_bTagAll = options.bTagAll;
	//}}AFX_DATA_INIT
}

CBZFormView::~CBZFormView()
{
	if(m_pDefFile)
		delete m_pDefFile;	// ##1.5
}

void CBZFormView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBZFormView)
	DDX_Control(pDX, IDC_LIST_TAG, m_listTag);
	DDX_Control(pDX, IDC_LIST_MEMBER, m_listMember);
	DDX_Control(pDX, IDC_STATIC_MEMBER, m_statMember);
	DDX_Check(pDX, IDB_TAGALL, m_bTagAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBZFormView, CFormView)
	//{{AFX_MSG_MAP(CBZFormView)
	ON_LBN_SELCHANGE(IDC_LIST_TAG, OnSelchangeListTag)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MEMBER, OnDblclkListMember)
	ON_WM_CREATE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MEMBER, OnItemchangedListMember)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDB_TAGALL, OnClickTagAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZFormView diagnostics

#ifdef _DEBUG
void CBZFormView::AssertValid() const
{
	CFormView::AssertValid();
}

void CBZFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CStructTag, CStructMember

CStructTag::CStructTag(LPSTR name)
{
	m_name = name;
	m_len = 0;
//	m_pMember = new CArray<CStructMember, CStructMember>;
}

CStructTag::~CStructTag()
{
}

CStructTag& CStructTag::operator=(const CStructTag& r)
{
	m_name = r.m_name;
	m_len  = r.m_len;
	return *this;
}

CStructMember::CStructMember(LPSTR name, int type)
{
	m_name = name;
	m_type = type;
	m_len = m_bytes = (type == -1) ? 0 : datasizes[type];//1<<(type/3);
}

CStructMember::~CStructMember()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBZFormView message handlers

//static int cxInit;

int CBZFormView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	if(options.xSplitStruct == 0)
		options.xSplitStruct = lpCreateStruct->cx;

	return 0;
}

void CBZFormView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	if(!m_pDefFile && !m_bNoDefFile) {
		CString sPath;
	//	sPath.LoadString(IDS_STRUCT_FILE);
	//	sPath = GetModulePath(sPath);
		sPath = GetStructFilePath();
		if(!(m_pDefFile = (LPSTR)ReadFile(sPath))) {
			m_bNoDefFile = TRUE;
			return;
		}
		if(!InitStructList()) {
			m_bNoDefFile = TRUE;
			return;
		}
		InitListTag();

		ListView_SetExtendedListViewStyle(m_listMember.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

		for_to(i, MBRCOL_MAX) {
			LV_COLUMN lvcol;
			lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.pszText = sMemberColLabel[i];
			lvcol.cx = options.colWidth[i];
			m_listMember.InsertColumn(i, &lvcol);
		}
	}
	m_pView = (CBZView*)GetNextWindow();
}


void CBZFormView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	InitListTag();
}

void CBZFormView::OnDestroy() 
{
	CFormView::OnDestroy();
	
	// TODO: Add your message handler code here
	if(!m_bNoDefFile) {
		for_to(i, MBRCOL_MAX)
			options.colWidth[i] = m_listMember.GetColumnWidth(i);
	}
}

void CBZFormView::OnSize(UINT nType, int cx, int cy) 
{
	SetScrollSizes(MM_TEXT, CSize(cx, cy));
	CFormView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_listTag.m_hWnd && m_listTag.m_hWnd) {
		CRect rForm, rTag, rMember, rStatic;
		GetWindowRect(rForm);
		m_listTag.GetWindowRect(rTag);
		m_listMember.GetWindowRect(rMember);
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
		m_listMember.MoveWindow(rMember);
		m_statMember.MoveWindow(rStatic);
		m_statMember.Invalidate();

		m_listMember.GetClientRect(rMember);
		int cxLabel = rMember.Width() - m_listMember.GetColumnWidth(MBRCOL_OFFSET) - m_listMember.GetColumnWidth(MBRCOL_VALUE);
		if(cxLabel > 0)
			m_listMember.SetColumnWidth(MBRCOL_LABEL, cxLabel);

		CRect rTagAll;
		CWnd* pWndTagAll = GetDlgItem(IDB_TAGALL);
		pWndTagAll->GetWindowRect(rTagAll);
		ScreenToClient(rTagAll);
		int cxTagAll = rTagAll.Width();
		rTagAll.right = rTag.right;
		rTagAll.left = rTag.right - cxTagAll;
		pWndTagAll->MoveWindow(rTagAll);
	}
}

void CBZFormView::OnSelchangeListTag() 
{
	// TODO: Add your control notification handler code here
	int iItem = m_listTag.GetCurSel();
	if(iItem != LB_ERR) {
		int iTag = m_listTag.GetItemData(iItem);
		m_pView->m_dwStructTag = m_pView->m_dwCaret;
		m_pView->m_dwStruct = m_pView->m_dwCaret + m_tag[iTag].m_len;
		m_pView->m_nMember = INVALID;
		InitListMember(iTag);
		m_pView->Invalidate(FALSE);
	}
}


void CBZFormView::OnItemchangedListMember(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if(pNMListView->uNewState == (LVIS_FOCUSED | LVIS_SELECTED)) {
		int iTag = m_listTag.GetItemData(m_listTag.GetCurSel());
		if(iTag >= 0) {	// ### 1.62
			CStructMember& m = m_tag[iTag].m_member[pNMListView->iItem];
			m_pView->m_nMember = m.m_ofs;
			m_pView->m_dwCaret = m_pView->m_dwStructTag + m.m_ofs;
			m_pView->m_nBytes = m.m_bytes ? m.m_bytes : 1;
			m_pView->m_nBytesLength = (m.m_bytes && (m.m_bytes != m.m_len)) ? m.m_len / m.m_bytes : 1;
			m_pView->GotoCaret();
			m_pView->Invalidate(FALSE);
			m_pView->UpdateStatusInfo();
		}
	//	m_pView->Activate();
	}	
	*pResult = 0;
}

/*
void CBZFormView::OnSelchangeListMember() 
{
	// TODO: Add your control notification handler code here
	CStructMember& m = m_tag[m_listTag.GetCurSel()].m_member[m_listMember.GetCurSel()];
	m_pView->m_dwCaret = m_pView->m_dwStructTag + m.m_ofs;
	m_pView->m_nBytes = m.m_bytes ? m.m_bytes : 1;
	m_pView->GotoCaret();
	m_pView->Activate();
}
*/

void CBZFormView::OnDblclkListMember(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	if(pNMListView->iItem == m_listMember.GetItemCount() - 1) {
		int iItem = m_listTag.GetCurSel();
		int iTag = m_listTag.GetItemData(iItem);
		m_pView->m_dwCaret = m_pView->m_dwStructTag + m_tag[iTag].m_len;
		if(m_listTag.GetCount() > 1 && m_nTagSelect != -1) {
			m_listTag.SetCurSel(iItem + 1);
		}
		OnSelchangeListTag();
	} else {
		m_pView->Activate();
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EDIT_VALUE);
	}
	*pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CBZFormView main 

BOOL CBZFormView::InitStructList() 
{
	char *p = m_pDefFile;
	const char seps[] = " ;[]\t\r\n\032";
	enum TokeMode { TK_STRUCT, TK_TAG, TK_MEMBER, TK_TYPE, TK_BEGIN, TK_END } mode;
	mode = TK_STRUCT;
	int iTag = 0;
	int iMember = 0;
	int type = 0;

	m_tag.RemoveAll();
	while(p = (char*)_mbstok((UCHAR*)p, (const UCHAR*)seps)) {
		switch (mode) {
		case TK_STRUCT:
			if (strcmp(p, "struct")) goto Error;
			mode = TK_TAG;
			break;
		case TK_TAG:
			if (!isalpha(*p)) goto Error;
			iTag = m_tag.Add(CStructTag(p));
			mode = TK_BEGIN;
			break;
		case TK_BEGIN:
			if (*p != '{') goto Error;
			mode = TK_TYPE;
			break;
		case TK_TYPE:
			if (*p == '}') {
				iMember = m_tag[iTag].m_member.Add(CStructMember(" <next>", -1));
				mode = TK_END;
			} else if(*p == ',')
				mode = TK_MEMBER;
			else if(isdigit(*p))
				m_tag[iTag].m_member[iMember].m_len *= atoi(p);
			else {
				int i222;
				for(i222=0; i222<nTypes; i222++)
					if(!strcmp(p, datatypes[i222])) break;
				if(i222 == nTypes) goto Error;
				type = i222;
				mode = TK_MEMBER;
			}
			break;
		case TK_MEMBER:
			if (!isalpha(*p) && !_ismbblead(*p)) goto Error;
			iMember = m_tag[iTag].m_member.Add(CStructMember(p, type));
			mode = TK_TYPE;
			break;
		case TK_END:	// ###1.61
			if (strcmp(p, "struct") == 0) 
				mode = TK_TAG;
			else if(isalnum(*p)) {
				m_tag[iTag].m_sarrFileExt.Add(p);
				TRACE("addext %s\n", p);
			}
			else if(*p != ',') goto Error;
			break;
		}
		p = NULL;
	}
	return TRUE;	
Error:
	CString sMsg;
	sMsg.Format(IDS_ERR_SYNTAX, p);
	AfxMessageBox(sMsg);
	return FALSE;	
}

void CBZFormView::InitListTag()
{
	for_to(i, m_tag.GetSize()) {
		int len = 0;
		for_to(j, m_tag[i].m_member.GetSize()) {
			CStructMember& member = m_tag[i].m_member[j];
			member.m_ofs = len;
			len += member.m_len;
		}
		m_tag[i].m_len = len;

	}
	m_listTag.ResetContent();

	if(!m_bTagAll) {
		CString sExt;
		CDocument* pDoc = GetDocument();
		if(pDoc) {
			CString sPath = pDoc->GetPathName();
			int nExt = sPath.ReverseFind(TCHAR('.'));
			if(nExt >= 0) {
				sExt = sPath.Mid(nExt + 1);
			}
		}

		m_nTagSelect = -1;
		if(!sExt.IsEmpty()) {
			for_to_(i, m_tag.GetSize()) {
				CStructTag& tag = m_tag[i];
				if(tag.m_sarrFileExt.GetSize() == 0)
					AddTag(i);
				else {
					for_to(j, tag.m_sarrFileExt.GetSize()) {
						if(sExt.CompareNoCase(tag.m_sarrFileExt[j]) == 0) {
							int nItem = AddTag(i);
							if(m_nTagSelect == -1)
								m_nTagSelect = nItem;
						}
					}
				}
			}
		}
	}
	if(m_listTag.GetCount() == 0) {
		for_to_(i, m_tag.GetSize()) {
			AddTag(i);
		}
	}
}

int CBZFormView::AddTag(int iTag)
{
	int iItem = m_listTag.AddString(m_tag[iTag].m_name);
	m_listTag.SetItemData(iItem, iTag);
	return iItem;
}

void CBZFormView::SelectTag()
{
	if(m_nTagSelect != -1) {
		m_listTag.SetCurSel(m_nTagSelect);
		OnSelchangeListTag();
	}
}

void CBZFormView::InitListMember(int iTag)
{
	m_listMember.DeleteAllItems();

	for_to(i, m_tag[iTag].m_member.GetSize()) {
		CStructMember& m = m_tag[iTag].m_member[i];
		CString s;

		s.Format("+%2X", m.m_ofs);
		LV_ITEM lvitem;
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = i;
		lvitem.iSubItem = MBRCOL_OFFSET;
		lvitem.pszText = (LPSTR)(LPCSTR)s;
		lvitem.iItem = m_listMember.InsertItem(&lvitem);

		lvitem.pszText =  (LPSTR)(LPCSTR)m.m_name;
		lvitem.iSubItem = MBRCOL_LABEL;
		m_listMember.SetItem(&lvitem);

		if(m.m_bytes) {
//			char* fval[3] = { "%d", "%u", "0x%X" };
			int val = m_pView->GetValue(m_pView->m_dwCaret + m.m_ofs, m.m_bytes);
			CString sVal;
			ULONGLONG qval = 0;
			
		//	int type = m.m_type % 3;
			switch(m.m_type)
			{
			case 0://char
				val = (int)(char)val;
				sVal = SeparateByComma(val, true);
				break;
			case 3://short
				val = (int)(short)val;
				sVal = SeparateByComma(val, true);
				break;
			case 6://long
				sVal = SeparateByComma(val, true);
				break;
			case 9://double
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format("%f", qval);
				break;
			case 10://qword(digit)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal = SeparateByComma64(qval, false);
				//sVal.Format("%I64u", qval);
				break;
			case 11://QWORD(hex)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format("0x%016I64X", qval);
				break;
			case 12://float
				sVal.Format("%f", val);
				break;
			case 1: // byte(unsigned char)
			case 4: // word(unsigned short)
			case 7: // dword(unsigned int)
				sVal = SeparateByComma(val, false);
				break;
			case 2://BYTE(Hex)
				sVal.Format("0x%02X", val);
				break;
			case 5://WORD(Hex)
				sVal.Format("0x%04X", val);
				break;
			case 8://DWORD(Hex)
				sVal.Format("0x%08X", val);
				break;
			}
			/*
			int type = m.m_type % 3;
			switch(type) {
			case 0:
				if(m.m_type == 0) 	// char
				{
					val = (int)(char)val;
					sVal = SeparateByComma(val, !type);
				} else if(m.m_type == 3)	// short
				{
					val = (int)(short)val;
					sVal = SeparateByComma(val, !type);
				} else if(m.m_type == 9)	// double
				{
					ULONGLONG qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
					sVal.Format("%f", qval);
				}
				break;
			case 1: // unsigned
				sVal = SeparateByComma(val, !type);
				break;
			case 2:
				sVal.Format("0x%X", val);
				break;
			}
			*/
			if(m.m_len != m.m_bytes)
				sVal += " ...";
			lvitem.pszText =  (LPSTR)(LPCSTR)sVal;
			lvitem.iSubItem = MBRCOL_VALUE;
			m_listMember.SetItem(&lvitem);
		}
	}
}

BOOL CBZFormView::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if(pMsg->message == WM_KEYDOWN) {
		if(pMsg->wParam == VK_RETURN) {
			if(pMsg->hwnd == m_listTag.m_hWnd)
				m_listMember.SetFocus();
			else if(pMsg->hwnd == m_listMember.m_hWnd) {
				m_pView->Activate();
				AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EDIT_VALUE);
			}
			return TRUE;
		}
		if(pMsg->wParam == VK_TAB && (::GetKeyState(VK_SHIFT) < 0)) {
			m_pView->Activate();
			return TRUE;
		}
	}
	return CFormView::PreTranslateMessage(pMsg);
}

void CBZFormView::Activate()
{
	if(m_listMember.GetSelectedCount() == 0)
		m_listTag.SetFocus();
	else
		m_listMember.SetFocus();
}

void CBZFormView::OnClickTagAll() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	options.bTagAll = m_bTagAll;
	InitListTag();
}
