// BZDoc.cpp : implementation of the CBZDoc class
//

#include "stdafx.h"
#include "BZ.h"
#include "BZDoc.h"
#include "Mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef FILE_MAPPING
  //#define FILEOFFSET_MASK 0xFFF00000	// by 1MB
  #define MAX_FILELENGTH  0xFFFFFFF0
#endif //FILE_MAPPING

/////////////////////////////////////////////////////////////////////////////
// CBZDoc

IMPLEMENT_DYNCREATE(CBZDoc, CDocument)

BEGIN_MESSAGE_MAP(CBZDoc, CDocument)
	//{{AFX_MSG_MAP(CBZDoc)
	ON_COMMAND(ID_EDIT_READONLY, OnEditReadOnly)
	ON_UPDATE_COMMAND_UI(ID_EDIT_READONLY, OnUpdateEditReadOnly)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_READONLYOPEN, OnEditReadOnlyOpen)
	ON_UPDATE_COMMAND_UI(ID_EDIT_READONLYOPEN, OnUpdateEditReadOnlyOpen)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_INDICATOR_INS, OnEditReadOnly)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_SAVE, ID_FILE_SAVE_AS, OnUpdateFileSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZDoc construction/destruction

CBZDoc::CBZDoc()
{
	// TODO: add one-time construction code here
	m_pData = NULL;
	m_dwTotal = 0;
	m_pUndo = NULL;
	m_bReadOnly = TRUE;
#ifdef FILE_MAPPING
	m_hMapping = NULL;
	m_pFileMapping = NULL;
	m_pDupDoc = NULL;
	m_pMapStart = NULL;
	m_dwFileOffset = 0;
	m_dwMapSize = 0;
#endif //FILE_MAPPING
	m_dwBase = 0;

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_dwAllocationGranularity = sysinfo.dwAllocationGranularity;
}

CBZDoc::~CBZDoc()
{
//	TRACE("DestructDoc:%X\n",this);
}

LPBYTE	CBZDoc::GetDocPtr()
{
	return m_pData;
}

void CBZDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class

	if(m_pData) {
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			VERIFY(::UnmapViewOfFile(m_pMapStart ? m_pMapStart : m_pData));
			m_pMapStart = NULL;
			m_dwFileOffset = 0;
			m_dwMapSize = 0;
		}
		else
#endif //FILE_MAPPING
			MemFree(m_pData);
		m_pData = NULL;
		m_dwTotal = 0;
		m_dwBase = 0;
		UpdateAllViews(NULL);
	}
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		if(m_pDupDoc) {
			m_pDupDoc->m_pDupDoc = NULL;
			m_pDupDoc = NULL;
			m_hMapping = NULL;
			m_pFileMapping = NULL;
		} else {
			VERIFY(::CloseHandle(m_hMapping));
			m_hMapping = NULL;
			if(m_pFileMapping) {
				ReleaseFile(m_pFileMapping, FALSE);
				m_pFileMapping = NULL;
			}
		}
	}
#endif //FILE_MAPPING

	if(m_pUndo) {
		MemFree(m_pUndo);
		m_pUndo = NULL;
	}	
	m_bReadOnly = FALSE;
	m_arrMarks.RemoveAll();
	CDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// CBZDoc serialization

void CBZDoc::Serialize(CArchive& ar)
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	CFile *pFile = ar.GetFile();
	ar.Flush();

	if (ar.IsLoading())	{
		// TODO: add loading code here
		m_dwTotal = GetFileLength(pFile);
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			if(!MapView()) return;
		} else
#endif //FILE_MAPPING
		{
			if(!(m_pData = (LPBYTE)MemAlloc(m_dwTotal))) {
				AfxMessageBox(IDS_ERR_MALLOC);
				return;
			}
			DWORD len = pFile->Read(m_pData, m_dwTotal);
			if(len < m_dwTotal) {
				AfxMessageBox(IDS_ERR_READFILE);
				MemFree(m_pData);	// ###1.61
				m_pData = NULL;
				return;
			}
			m_bReadOnly = options.bReadOnlyOpen;
		}
	} else {
		// TODO: add storing code here
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			BOOL bResult = (m_pMapStart) ? ::FlushViewOfFile(m_pMapStart, m_dwMapSize) : ::FlushViewOfFile(m_pData, m_dwTotal);
			if(!bResult) {
				ErrorMessageBox();
			}
		} else
#endif //FILE_MAPPING
			pFile->Write(m_pData, m_dwTotal);
		m_dwUndoSaved = m_dwUndo;		// ###1.54
		TouchDoc();
/*		if(m_pUndo) {
			MemFree(m_pUndo);
			m_pUndo = NULL;
		}	
*/	}
}

#ifdef FILE_MAPPING

BOOL CBZDoc::MapView()
{
	m_dwMapSize = m_dwTotal;
	m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
	if(!m_pData) {
		m_dwMapSize = options.dwMaxMapSize;
		m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
		if(m_pData) {
			m_pMapStart = m_pData;
			m_dwFileOffset = 0;
		}
		else {
			ErrorMessageBox();
			AfxThrowMemoryException();	// ###1.61
			return FALSE;
		}
	}
	return TRUE;
}

LPBYTE CBZDoc::QueryMapView1(LPBYTE pBegin, DWORD dwOffset)
{
//	TRACE("QueryMapView1 m_pData:%X, pBegin:%X, dwOffset:%X\n", m_pData, pBegin, dwOffset);
	LPBYTE p = pBegin + dwOffset;
	//if(IsOutOfMap1(p)) {QueryMapView()内に移動
		if(p == m_pData + m_dwTotal && p == m_pMapStart + m_dwMapSize) return pBegin;	// ###1.61a
		DWORD dwBegin = GetFileOffsetFromFileMappingPointer(pBegin);//DWORD dwBegin = pBegin - m_pData;
		VERIFY(::UnmapViewOfFile(m_pMapStart));//ここで書き込まれちゃうけどOK?
		//m_dwFileOffset = GetFileOffsetFromFileMappingPointer(p)/*(p - m_pDate)*/ & FILEOFFSET_MASK;
		DWORD dwTmp1 = GetFileOffsetFromFileMappingPointer(p);
		m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
		m_dwMapSize = min(options.dwMaxMapSize, m_dwTotal - m_dwFileOffset); //どうもここが引っかかる。全領域をマッピングできる？CBZDoc::MapView()のブロック２をコメントアウトしたほうがいいかも。ただ32ビット＆4GBオーバー対応の際に問題になりそう by tamachan(20120907)
		if(m_dwMapSize == 0) {	// ###1.61a
			//m_dwFileOffset = (m_dwTotal - (~FILEOFFSET_MASK + 1)) & FILEOFFSET_MASK;
			dwTmp1 = (m_dwTotal - m_dwAllocationGranularity);
			m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
			m_dwMapSize = m_dwTotal - m_dwFileOffset;
		}
		m_pMapStart = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, m_dwFileOffset, m_dwMapSize);
		TRACE("MapViewOfFile Doc=%X, %X, Offset:%X, Size:%X\n", this, m_pMapStart, m_dwFileOffset, m_dwMapSize);
		if(!m_pMapStart) {
			ErrorMessageBox();
			AfxPostQuitMessage(0);
			return NULL;
		}
		m_pData = m_pMapStart - m_dwFileOffset; //バグ?：仮想的なアドレス（ファイルのオフセット0にあたるメモリアドレス）を作り出している。m_pMapStart<m_dwFileOffsetだった場合、0を割ることがあるのではないだろうか？そういった場合まずい？？IsOutOfMapは正常に動きそう？ by tamachan(20121004)
		pBegin = GetFileMappingPointerFromFileOffset(dwBegin);//pBegin = m_pData + dwBegin;
	//}
	return pBegin;
}

BOOL CBZDoc::IsOutOfMap1(LPBYTE p)
{
	return ((int)p < (int)m_pMapStart || (int)p >= (int)(m_pMapStart + m_dwMapSize));
}

#endif //FILE_MAPPING

/////////////////////////////////////////////////////////////////////////////
// CBZDoc diagnostics

#ifdef _DEBUG
void CBZDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBZDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBZDoc commands


void CBZDoc::OnEditReadOnly() 
{
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		AfxMessageBox(IDS_ERR_MAP_RO);
		return;
	}
#endif //FILE_MAPPING
	m_bReadOnly = !m_bReadOnly;	
}

void CBZDoc::OnUpdateEditReadOnly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bReadOnly);
}

void CBZDoc::OnEditReadOnlyOpen() 
{
	// TODO: Add your command handler code here
	options.bReadOnlyOpen = !options.bReadOnlyOpen;
}

void CBZDoc::OnUpdateEditReadOnlyOpen(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(options.bReadOnlyOpen);	
}

BOOL CBZDoc::CopyToClipboard(DWORD dwPtr, DWORD dwSize)	// ###1.5
{
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
	if(dwSize >= options.dwMaxMapSize || IsOutOfMap(m_pData + dwPtr + dwSize)) {
		AfxMessageBox(IDS_ERR_COPY);
		return FALSE;
	}
#endif //FILE_MAPPING
	HGLOBAL hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + 1);
	HGLOBAL hMemBin = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + sizeof(dwSize));
	LPBYTE pMemTxt  = (LPBYTE)::GlobalLock(hMemTxt);
	LPBYTE pMemBin  = (LPBYTE)::GlobalLock(hMemBin);
	memcpy(pMemTxt, m_pData + dwPtr, dwSize);
	*(pMemTxt + dwSize) = '\0';
	*((DWORD*)(pMemBin)) = dwSize;
	memcpy(pMemBin + sizeof(dwSize), m_pData + dwPtr, dwSize);
	::GlobalUnlock(hMemTxt);
	::GlobalUnlock(hMemBin);
	AfxGetMainWnd()->OpenClipboard();
	::EmptyClipboard();
	::SetClipboardData(CF_TEXT, hMemTxt);
	::SetClipboardData(RegisterClipboardFormat("BinaryData2"), hMemBin);
	::CloseClipboard();
	return TRUE;
}

DWORD CBZDoc::PasteFromClipboard(DWORD dwPtr, BOOL bIns)
{
	AfxGetMainWnd()->OpenClipboard();
	HGLOBAL hMem;
	DWORD dwSize;
	LPBYTE pMem;
	if(hMem = ::GetClipboardData(RegisterClipboardFormat("BinaryData2"))) {
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = *((DWORD*)(pMem));
		pMem += sizeof(DWORD);
	} else if(hMem = GetClipboardData(CF_TEXT)) {
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = lstrlen((LPCSTR)pMem);
	} else {
/*		UINT uFmt = 0;
		while(uFmt = ::EnumClipboardFormats(uFmt)) {
			CString sName;
			::GetClipboardFormatName(uFmt, sName.GetBuffer(MAX_PATH), MAX_PATH);
			sName.ReleaseBuffer();
			TRACE("clip 0x%X:%s\n", uFmt, sName);
		}

		return 0;
*/		if(!(hMem = ::GetClipboardData(::EnumClipboardFormats(0))))
			return 0;
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = ::GlobalSize(hMem);
	}
	if(!dwSize) return 0;
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		int nGlow = dwSize - (m_dwTotal - dwPtr);
		if(nGlow > 0)
			dwSize -= nGlow;
	}
#endif //FILE_MAPPING
	if(bIns || dwPtr == m_dwTotal)
		StoreUndo(dwPtr, dwSize, UNDO_DEL);
	else
		StoreUndo(dwPtr, dwSize, UNDO_OVR);
	InsertData(dwPtr, dwSize, bIns);
	memcpy(m_pData+dwPtr, pMem, dwSize);
	::GlobalUnlock(hMem);
	::CloseClipboard();
	return dwPtr+dwSize;
}

void CBZDoc::InsertData(DWORD dwPtr, DWORD dwSize, BOOL bIns)
{
	int nGlow = dwSize - (m_dwTotal - dwPtr);
	if(!m_pData) {
		m_pData = (LPBYTE)MemAlloc(dwSize);
		m_dwTotal = dwSize;
	} else if(bIns || dwPtr == m_dwTotal) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+dwSize);
			memmove(m_pData+dwPtr+dwSize, m_pData+dwPtr, m_dwTotal - dwPtr);
			m_dwTotal += dwSize;
	} else if(nGlow > 0) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+nGlow);
			m_dwTotal += nGlow;
	}
	ASSERT(m_pData != NULL);
}

void CBZDoc::DeleteData(DWORD dwPtr, DWORD dwSize)
{
	if(dwPtr == m_dwTotal) return;
	memmove(m_pData+dwPtr, m_pData+dwPtr+dwSize, m_dwTotal-dwPtr-dwSize);
	m_dwTotal -= dwSize;
#ifdef FILE_MAPPING
	if(!IsFileMapping())
#endif //FILE_MAPPING
		m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal);
	TouchDoc();
}

void CBZDoc::TouchDoc()
{
	SetModifiedFlag(m_dwUndo != m_dwUndoSaved);
	GetMainFrame()->OnUpdateFrameTitle();

/*	CString sTitle = GetTitle();
	if(b) sTitle += " *";
	sTitle += " - ";
	sTitle += AfxGetApp()->m_pszAppName;
	CWnd *pFrame = AfxGetMainWnd();
	pFrame->SetWindowText(sTitle);
*/
}

void CBZDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!!m_pUndo);	
}

void CBZDoc::StoreUndo(DWORD dwPtr, DWORD dwSize, UndoMode mode)
{
	if(mode == UNDO_OVR && dwPtr+dwSize >= m_dwTotal)
		dwSize = m_dwTotal - dwPtr;
	if(dwSize == 0) return;
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
#endif //FILE_MAPPING
	DWORD dwBlock = dwSize + 9;
	if(mode == UNDO_DEL)
		dwBlock = 4 + 9;
	if(!m_pUndo) {
		m_pUndo = (LPBYTE)MemAlloc(dwBlock);
		m_dwUndo = m_dwUndoSaved = 0;
	} else
		m_pUndo = (LPBYTE)MemReAlloc(m_pUndo, m_dwUndo+dwBlock);
	ASSERT(m_pUndo != NULL);
	LPBYTE p = m_pUndo + m_dwUndo;
	*((DWORD*&)p)++ = dwPtr;
	*p++ = mode;
	if(mode == UNDO_DEL) {
		*((DWORD*&)p)++ = dwSize;
	} else {
		memcpy(p, m_pData+dwPtr, dwSize);
		p+=dwSize;
	}
	*((DWORD*&)p)++ = dwBlock;
	m_dwUndo += dwBlock;
	ASSERT(p == m_pUndo+m_dwUndo);
	TouchDoc();
}

DWORD CBZDoc::DoUndo()
{
	DWORD dwSize = *((DWORD*)(m_pUndo+m_dwUndo-4));
	m_dwUndo -= dwSize;
	dwSize -= 9;
	LPBYTE p = m_pUndo + m_dwUndo;
	DWORD dwPtr = *((DWORD*&)p)++;
	UndoMode mode = (UndoMode)*p++;
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
#endif //FILE_MAPPING
	if(mode == UNDO_DEL) {
		DeleteData(dwPtr, *((DWORD*)p));
	} else {
		InsertData(dwPtr, dwSize, mode == UNDO_INS);
		memcpy(m_pData+dwPtr, p, dwSize);
	}
	if(m_dwUndo)
		m_pUndo = (LPBYTE)MemReAlloc(m_pUndo, m_dwUndo);
	else {				// ### 1.54
		MemFree(m_pUndo);
		m_pUndo = NULL;
		if(m_dwUndoSaved)
			m_dwUndoSaved = UINT_MAX;
	}
	// if(!m_pUndo)
		TouchDoc();
	return dwPtr;
}

void CBZDoc::DuplicateDoc(CBZDoc* pDstDoc)
{
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		m_pDupDoc = pDstDoc;
		pDstDoc->m_pDupDoc = this;
		pDstDoc->m_hMapping = m_hMapping;
		pDstDoc->m_pFileMapping = m_pFileMapping;
		pDstDoc->m_dwTotal = m_dwTotal;
		pDstDoc->MapView();
	} else
#endif //FILE_MAPPING
	{
		pDstDoc->m_pData = (LPBYTE)MemAlloc(m_dwTotal);
		memcpy(pDstDoc->m_pData, m_pData, m_dwTotal);
		pDstDoc->m_dwTotal = m_dwTotal;
	}
	pDstDoc->m_dwBase = m_dwBase;
	pDstDoc->SetTitle(GetTitle());
	CString s = GetPathName();
	if(!s.IsEmpty())
		pDstDoc->SetPathName(s);
//	pDstDoc->UpdateAllViews(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CBZDoc Mark

void CBZDoc::SetMark(DWORD dwPtr)
{
	for_to(i, m_arrMarks.GetSize()) {
		if(m_arrMarks[i] == dwPtr) {
			m_arrMarks.RemoveAt(i);
			return;
		} else if(m_arrMarks[i] >= m_dwTotal) {
			m_arrMarks.RemoveAt(i);
		}
	}
	m_arrMarks.Add(dwPtr);
}

BOOL CBZDoc::CheckMark(DWORD dwPtr)
{
	for_to(i,  m_arrMarks.GetSize()) {
		if(m_arrMarks[i] == dwPtr)
			return TRUE;
	}
	return FALSE;
}

DWORD CBZDoc::JumpToMark(DWORD dwStart)
{
	DWORD dwNext = m_dwTotal;
Retry:
	for_to(i, m_arrMarks.GetSize()) {
		if(m_arrMarks[i] > dwStart && m_arrMarks[i] < dwNext)
			dwNext = m_arrMarks[i];
	}
	if(dwNext == m_dwTotal && dwStart) {
		dwStart = 0;
		goto Retry;
	}
	if(dwNext < m_dwTotal) 
		return dwNext;
	return INVALID;
}

void CBZDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_bReadOnly);
}

/////////////////////////////////////////////////////////////////////////////
// ###1.60 File Mapping

DWORD CBZDoc::GetFileLength(CFile* pFile, BOOL bErrorMsg)
{
	DWORD dwSizeHigh = 0;
	DWORD dwSize = ::GetFileSize((HANDLE)pFile->m_hFile, &dwSizeHigh);
	if(dwSizeHigh) {
		if(bErrorMsg)
			AfxMessageBox(IDS_ERR_OVER4G);
		dwSize = MAX_FILELENGTH;
	}
	return dwSize;
}

#ifdef FILE_MAPPING

#ifndef _AFX_OLD_EXCEPTIONS
    #define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)
#else   //!_AFX_OLD_EXCEPTIONS
    #define DELETE_EXCEPTION(e)
#endif  //_AFX_OLD_EXCEPTIONS


void CBZDoc::ReleaseFile(CFile* pFile, BOOL bAbort)
{
	// ------ File Mapping ----->
	if(IsFileMapping()) return;
	// <----- File Mapping ------
	ASSERT_KINDOF(CFile, pFile);
	if (bAbort)
		pFile->Abort(); // will not throw an exception
	else {
		pFile->Close();
	}
	delete pFile;
}

BOOL CBZDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (IsModified())
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	// ------ File Mapping ----->
	DWORD dwSize = GetFileLength(pFile, TRUE);
	if(dwSize >= options.dwMaxOnMemory) {
		m_bReadOnly = options.bReadOnlyOpen || ::GetFileAttributes(lpszPathName) & FILE_ATTRIBUTE_READONLY;
		if(!m_bReadOnly) {  // Reopen for ReadWrite
			ReleaseFile(pFile, FALSE);
			pFile = GetFile(lpszPathName, CFile::modeReadWrite | CFile::shareExclusive, &fe);
			if(pFile == NULL) {
				ReportSaveLoadException(lpszPathName, &fe, FALSE, AFX_IDP_INVALID_FILENAME);
				return FALSE;
			}
		}
		m_pFileMapping = pFile;
		m_hMapping = ::CreateFileMapping((HANDLE)pFile->m_hFile, NULL, m_bReadOnly ? PAGE_READONLY : PAGE_READWRITE
			, 0, 0, /*options.bReadOnlyOpen ? 0 : dwSize + options.dwMaxOnMemory,*/ NULL);
		if(!m_hMapping) {
			ErrorMessageBox();
			ReleaseFile(pFile, FALSE);
			return FALSE;
		}
	}
	// <----- File Mapping ------

	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		if (GetFileLength(pFile) != 0)
			Serialize(loadArchive);     // load me
		loadArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}

BOOL CBZDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	CFileException fe;
	CFile* pFile = NULL;

	// ------ File Mapping ----->
	if(IsFileMapping())
		pFile = m_pFileMapping;
	else 
	// <----- File Mapping ------
		pFile = GetFile(lpszPathName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
	saveArchive.m_pDocument = this;
	saveArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		Serialize(saveArchive);     // save me
		saveArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

BOOL CBZDoc::SaveModified() 
{
	if (!IsModified())
		return TRUE;        // ok to continue

	// get name/title of document
	CString name;
	if (m_strPathName.IsEmpty())
	{
		// get name based on caption
		name = m_strTitle;
		if (name.IsEmpty())
			VERIFY(name.LoadString(AFX_IDS_UNTITLED));
	}
	else
	{
		// get name based on file title of path name
		name = m_strPathName;
		/*if (afxData.bMarked4)
		{
			AfxGetFileTitle(m_strPathName, name.GetBuffer(_MAX_PATH), _MAX_PATH);
			name.ReleaseBuffer();
		}*/
	}

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;

	case IDNO:
		// If not saving changes, revert the document
		if(IsFileMapping()) {
			while(m_pUndo)
				DoUndo();
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}

#endif //FILE_MAPPING
