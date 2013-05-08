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
#include "BZView.h"
#include "BZFormVw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define nTypes 15

char* g_datatypes[nTypes] = {
	"char", "byte", "BYTE", "short", "word", "WORD", "long", "dword", "DWORD", "double", "qword", "QWORD", "float", "int64","int"
};

DWORD g_datasizes[nTypes] = {1,1,1,2,2,2,4,4,4,8,8,8 ,4,8,4
};

static TCHAR *s_MemberColLabel[MBRCOL_MAX] = { _T("+"), _T("Label"), _T("Value") };

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
		sPath = GetStructFilePath(IDS_STRUCT_FILE);
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
			lvcol.pszText = s_MemberColLabel[i];
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
	if(m_listTag.GetCount()==0)return;

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

/*BOOL CBZFormView::InitStructList() 
{
	CAtlRegExp<CAtlRECharTraitsMB> re;*/
	//REParseError status = re.Parse((const unsigned char *)"[\\t\\r\\n ]*?struct[\\t\\r\\n ]+?{[^\\t\\r\\n ;\\[\\]]+?}[\\t\\r\\n ]*?\\{{[^\\{\\}]*?}\\}[\\t\\r\\n ]*?{[^\\t\\r\\n ;\\[\\]]*?}[\\t\\r\\n ]*?;[\\t\\r\\n ]*?");
	//REParseError status = re.Parse((const unsigned char *)"[\t\r\n ]*?struct[\t\r\n ]+?{[^\t\r\n ;\[\]]+?}[\t\r\n ]*?\{{[^{}]*?}\}[\t\r\n ]*?{[^\t\r\n ;\[\]]*?}[\t\r\n ]*?;[\t\r\n ]*?");
	//REParseError status = re.Parse((const unsigned char *)"{\\a+?}");
	//REParseError status = re.Parse((const unsigned char *)"[\t\r\n ]*?struct[\t\r\n ]+?{.+?}[\t\r\n ]*?\\{{.*?}\\}[\t\r\n ]*?{.*?}[\t\r\n ]*?;[\t\r\n ]*?");
//	REParseError status = re.Parse((const unsigned char *)"struct[\t\r\n ]+?{.+?}[\t\r\n ]*?\\{{.*?}\\}[\t\r\n ]*?{.*?}[\t\r\n ]*?;");
	//REParseError status = re.Parse((const unsigned char *)"struct[\t\r\n ]+?{.+?}([\t\r\n ]*?|(/\\*.*?\\*/)*?|(//.*?[\n\r]+?)*?)*?\\{{.*?}\\}([\t\r\n ]*?|(/\\*.*?\\*/)*?|(//.*?[\n\r]+?)*?)*?{.*?}([\t\r\n ]*?|(/\\*.*?\\*/)*?|(//.*?[\n\r]+?)*?)*?;");
	//REParseError status = re.Parse((const unsigned char *)"{/\\*.*?\\*/}");
	//REParseError status = re.Parse((const unsigned char *)"{//.*?\n+?}");
	//REParseError status = re.Parse((const unsigned char *)"{(/\\*.*?\\*/)|(//.*?\n+?)}");*/
	//REParseError status = re.Parse((const unsigned char *)"{(/\\*.*?\\*/)*?|(//.*?\n+?)*?}");
	/*CAtlRegExp<CAtlRECharTraitsMB> re2;
	REParseError status2 = re2.Parse((const unsigned char *)"{[^,\t\r\n ]+}[\t\r\n ]*(\\[{[0-9]+}\\])?");


	if (REPARSE_ERROR_OK != status || REPARSE_ERROR_OK != status2)
	{
		// Unexpected error.
		MessageBox("CBZFormView::InitStructList CAtlRegExp initialize error", "Error");
		return FALSE;
	}
	
    CAtlREMatchContext<CAtlRECharTraitsMB> matched, matched2;
	const unsigned char *nextC = (const unsigned char *)m_pDefFile;
	unsigned int iTag = 0, iMember = 0;
	while(*nextC!='\0')
	{
		if (!re.Match(nextC, &matched, &nextC))
		{
			// Unexpected error.
			//MessageBox("CBZFormView::InitStructList CAtlRegExp matching error", "Error");
			//return FALSE;
			break;
		}

		CString tmp1;

		for (UINT nGroupIndex = 0; nGroupIndex < matched.m_uNumGroups; nGroupIndex++)
		{
			const CAtlREMatchContext<CAtlRECharTraitsMB>::RECHAR* szStart = 0;
			const CAtlREMatchContext<CAtlRECharTraitsMB>::RECHAR* szEnd = 0;
			matched.GetMatch(nGroupIndex, &szStart, &szEnd);

			ptrdiff_t nLength = szEnd - szStart;
			unsigned int nMax = nLength;

			unsigned int nStructIndex = nGroupIndex%3;
			//unsigned int iTag = nGroupIndex/3;

			tmp1.SetString((const char*)szStart, nMax);

			switch(nStructIndex)
			{
			case 0:
				m_tag.Add(CStructTag(tmp1));
				ATLTRACE("add struct %s\n", tmp1);
				break;
			case 1:
				{
					ATLTRACE("add struct {%s}\n", tmp1);
					int curPos = 0;
					int flagContinue = false;
					CString memberline;
					while ((memberline = tmp1.Tokenize(";", curPos))!= _T(""))
					{
						int curPos2 = 0;
						memberline.Trim(" \t\r\n");
						ATLTRACE("memberline %s(%d)\n", memberline,memberline.GetLength());
						CString type = memberline.Tokenize(" \t\r\n", curPos2);
						CString members = memberline.Right(memberline.GetLength() - type.GetLength()).Trim(" \t\r\n");
						//ATLTRACE("type=%s(%d), members=%s(%d)\n",type,type.GetLength(), members,members.GetLength());

						int i222;
						for(i222=0; i222<nTypes; i222++)
							if(!strcmp(type, g_datatypes[i222])) break;
						int iType = i222;
						if(type.GetLength()==0 || members.GetLength()==0 || i222 == nTypes)
						{
							//flagContinue=true;break;
							continue;
						}
						char * membersBuf = members.GetBuffer();
						const unsigned char *nextM = (const unsigned char *)membersBuf;
						if (!re2.Match(nextM, &matched2, &nextM))
						{
							ATLTRACE("re2: Match error (%s)\n", membersBuf);
							continue;
						}
						CString tmp2;
						ATLTRACE("re2: %d\n",matched2.m_uNumGroups);

						for (UINT nGroupIndex2 = 0; nGroupIndex2 < matched2.m_uNumGroups; nGroupIndex2++)
						{
							const CAtlREMatchContext<CAtlRECharTraitsMB>::RECHAR* szStart2 = 0;
							const CAtlREMatchContext<CAtlRECharTraitsMB>::RECHAR* szEnd2 = 0;
							matched2.GetMatch(nGroupIndex2, &szStart2, &szEnd2);

							ptrdiff_t nLength2 = szEnd2 - szStart2;
							unsigned int nMax2 = nLength2;
							tmp2.SetString((const char*)szStart2, nMax2);
							if(nGroupIndex2==0)
							{
								iMember = m_tag[iTag].m_member.Add(CStructMember(tmp2, iType));
								ATLTRACE("new member: %s(%d)\n", tmp2, iType);
							} else if(nGroupIndex2==1) {
								if(tmp2.GetLength()>0)
								{
									m_tag[iTag].m_member[iMember].m_len *= atoi(tmp2);
									ATLTRACE("[%d]\n", atoi(tmp2));
								}
							}
						}
					}
					if(flagContinue)continue;
					break;
				}
			case 2:
				{
					m_tag[iTag].m_member.Add(CStructMember(" <next>", -1));
					int curPos = 0;
					CString ext = tmp1.Tokenize(", \t\r\n", curPos);
					ATLTRACE("addexts %s\n", tmp1);
					while (ext != _T(""))
					{
						m_tag[iTag].m_sarrFileExt.Add(ext);
						ATLTRACE("addext %s\n", ext);
						ext = tmp1.Tokenize(", \t\r\n", curPos);
					}
					break;
				}
			}
		}
		iTag++;
	}

	return TRUE;
}*/

void RemoveCommentAll(char *src)
{
	for(; *src!='\0'; src++)
	{
		if(*src=='/')
		{
			if(*(src+1)=='/')
			{
				*src = *(src+1) = ' ';
				src+=2;
				for(; *src!='\r' && *src!='\n'; src++)
				{
					if(*src=='\0')return;
					*src=' ';
				}
			} else if(*(src+1)=='*') {
				*src = *(src+1) = ' ';
				src+=2;
				for(; *src!='*' || (*src!='\0' && *(src+1)!='/'); src++)
				{
					if(*src=='\0')return;
					*src=' ';
				}
				if(*src=='\0')return;
				*src = *(src+1) = ' ';
			}
		}
	}
}

BOOL CBZFormView::InitStructList() 
{
	RemoveCommentAll(m_pDefFile);
	ATLTRACE("BZ.def: %s\n", m_pDefFile);
	char *p = m_pDefFile;
	const char seps[] = " ;[]\t\r\n\032";
	enum TokeMode { TK_STRUCT, TK_TAG, TK_BEGIN } mode;
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
			p = NULL;
			break;
		case TK_TAG:
			if (!isalpha(*p)) goto Error;
			iTag = m_tag.Add(CStructTag(p));
			mode = TK_BEGIN;
			p = NULL;
			break;
		case TK_BEGIN:
			{
				if (*p != '{') goto Error;
				p+=2;
				char *pEnd = (char *)_mbsstr((UCHAR*)p, (UCHAR*)"}");
				if(pEnd==NULL) goto Error; // '}' not found
				CString members;
				members.SetString(p, pEnd - p);
				CString memberline;
				int curPos=0;
				while((memberline = members.Tokenize(";", curPos).Trim(" \t\r\n"))!=_T(""))
				{
					//    DWORD m1,m2[43],m424
					ATLTRACE("memberline %s(%d)\n", memberline,memberline.GetLength());
					int curPos2 = 0;
					CString type = memberline.Tokenize(" \t\r\n", curPos2);
					CString members = memberline.Right(memberline.GetLength() - type.GetLength()).Trim(" \t\r\n");

					int i222;
					for(i222=0; i222<nTypes; i222++)
						if(!strcmp(type, g_datatypes[i222])) break;
					int iType = i222;
					if(type.GetLength()==0 || members.GetLength()==0 || i222 == nTypes)goto Error;

					int curPos3 = 0;
					CString member;
					while((member = members.Tokenize(",", curPos3))!=_T(""))
					{
						//     m2[43]  
						int curPos4 = 0;
						CString memberName = member.Tokenize("[ \t\r\n", curPos4);
						if(memberName==_T(""))goto Error;
						iMember = m_tag[iTag].m_member.Add(CStructMember(memberName, iType));
						if(curPos4==-1)continue;
						CString arrayNum = member.Tokenize("[ \t\r\n", curPos4).TrimRight("]");
						if(arrayNum!=_T(""))
						{
							if(atoi(arrayNum)==0)goto Error;//wrong number or member[0]
							m_tag[iTag].m_member[iMember].m_len *= atoi(arrayNum);
						}
						if(curPos4==-1)continue;
						if(member.Tokenize("[] \t\r\n", curPos4)!=_T(""))goto Error;
					}
				}
				iMember = m_tag[iTag].m_member.Add(CStructMember(" <next>", -1));
				p=pEnd+1;
				char *pExtEnd = (char*)_mbsstr((UCHAR*)p, (UCHAR*)";");
				if(pExtEnd==NULL) goto Error; // ';' not found
				CString extall;
				extall.SetString(p, pExtEnd-p);
				int curPos11=0;
				CString ext;
				while((ext = extall.Tokenize(",", curPos11).Trim(" \t\r\n"))!=_T(""))
				{
					m_tag[iTag].m_sarrFileExt.Add(ext);
					TRACE("addext %s\n", ext);
				}
				p = pExtEnd+1;
				mode = TK_STRUCT;
				break;
			}
		}
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
			case 14://int
				sVal = SeparateByComma(val, true);
				break;
			case 13://int64
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal = SeparateByComma64(qval, true);
				break;
			case 12://float
				sVal.Format("%f", val);
				break;
			case 9://double
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format("%f", qval);
				break;
			case 1: // byte(unsigned char)
			case 4: // word(unsigned short)
			case 7: // dword(unsigned int)
				sVal = SeparateByComma(val, false);
				break;
			case 10://qword(digit)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal = SeparateByComma64(qval, false);
				//sVal.Format("%I64u", qval);
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
			case 11://QWORD(hex)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format("0x%016I64X", qval);
				break;
			}

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
