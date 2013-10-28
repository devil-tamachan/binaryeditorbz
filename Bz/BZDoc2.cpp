// BZDoc2.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BZ.h"
#include "BZDoc2.h"


// CBZDoc2

IMPLEMENT_DYNCREATE(CBZDoc2, CDocument)

CBZDoc2::CBZDoc2() : m_pSFC(NULL), m_pSFCCache(NULL)
{
  m_dwBase = 0;

  //ReCreate restore
  m_restoreCaret = 0;
  //m_restoreScroll = {0};
  m_bReadOnly = FALSE;
}

void CBZDoc2::DeleteContents()
{
  m_dwBase = 0;
  m_restoreCaret = 0;
  //m_restoreScroll = {0};
  m_bReadOnly = FALSE;

  ReleaseSFC();
  m_arrMarks.RemoveAll();
  SetModifiedFlag(FALSE);

  CDocument::DeleteContents();
}

CBZDoc2::~CBZDoc2()
{
  ReleaseSFC();
}


BEGIN_MESSAGE_MAP(CBZDoc2, CDocument)
  ON_COMMAND(ID_EDIT_READONLY, &CBZDoc2::OnEditReadOnly)
  ON_UPDATE_COMMAND_UI(ID_EDIT_READONLY, &CBZDoc2::OnUpdateEditReadOnly)
  ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CBZDoc2::OnUpdateEditUndo)
  ON_COMMAND(ID_EDIT_READONLYOPEN, &CBZDoc2::OnEditReadOnlyOpen)
  ON_UPDATE_COMMAND_UI(ID_EDIT_READONLYOPEN, &CBZDoc2::OnUpdateEditReadOnlyOpen)
  ON_COMMAND(ID_INDICATOR_INS, OnEditReadOnly)
  ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_SAVE, ID_FILE_SAVE_AS, OnUpdateFileSave)
  //  ON_COMMAND(ID_FILE_SAVE, &CBZDoc2::OnFileSave)
  ON_COMMAND(ID_FILE_SAVE, &CBZDoc2::OnFileSave)
  ON_COMMAND(ID_FILE_SAVE_AS, &CBZDoc2::OnFileSaveAs)
END_MESSAGE_MAP()


// CBZDoc2 診断

#ifdef _DEBUG
void CBZDoc2::AssertValid() const
{
  CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CBZDoc2::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CBZDoc2 シリアル化

void CBZDoc2::Serialize(CArchive& ar)
{
  /*
  if (ar.IsStoring())
  {
  }
  else
  {
  }
  */
}
#endif


// CBZDoc2 コマンド

void CBZDoc2::OnEditReadOnly()
{
  m_bReadOnly = !m_bReadOnly;	
}

void CBZDoc2::OnUpdateEditReadOnly(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_bReadOnly);
}

void CBZDoc2::OnEditReadOnlyOpen()
{
  options.bReadOnlyOpen = !options.bReadOnlyOpen;
}

void CBZDoc2::OnUpdateEditReadOnlyOpen(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(options.bReadOnlyOpen);
}

void CBZDoc2::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_pSFC && m_pSFC->GetUndoCount()!=0);
}

void CBZDoc2::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_bReadOnly);
}



DWORD CBZDoc2::PasteFromClipboard(DWORD dwStart, BOOL bIns)
{
  if(!m_pSFC)goto ERR_PASTECLIP1;
  AfxGetMainWnd()->OpenClipboard();
  HGLOBAL hMem;
  DWORD dwSize;
  LPBYTE pMem;
  if(hMem = ::GetClipboardData(RegisterClipboardFormat(_T("BinaryData2")))) {
    DWORD dwMemSize = GlobalSize(hMem);
    pMem = (LPBYTE)::GlobalLock(hMem);
    if(!pMem)goto ERR_PASTECLIP3;
    dwSize = *((DWORD*)(pMem));
    if(dwMemSize < sizeof(DWORD) || dwSize > dwMemSize-sizeof(DWORD))goto ERR_PASTECLIP3;
    pMem += sizeof(DWORD);
  } else if(hMem = GetClipboardData(CF_TEXT)) {
    DWORD dwMemSize = GlobalSize(hMem);
    pMem = (LPBYTE)::GlobalLock(hMem);
    if(!pMem)goto ERR_PASTECLIP3;
    dwSize = strnlen_s((LPCSTR)pMem, dwMemSize);
  } else {
    /*		UINT uFmt = 0;
    while(uFmt = ::EnumClipboardFormats(uFmt)) {
    CString sName;
    ::GetClipboardFormatName(uFmt, sName.GetBuffer(MAX_PATH), MAX_PATH);
    sName.ReleaseBuffer();
    TRACE("clip 0x%X:%s\n", uFmt, sName);
    }
    return 0;
    */
    hMem = ::GetClipboardData(::EnumClipboardFormats(0));
    if(!hMem)goto ERR_PASTECLIP2;
    pMem = (LPBYTE)::GlobalLock(hMem);
    if(!pMem)goto ERR_PASTECLIP3;
    dwSize = ::GlobalSize(hMem);
  }
  if(!dwSize)goto ERR_PASTECLIP3;
  DWORD dwTotal = m_pSFC->GetSize();
  if(bIns || dwStart == dwTotal)
  {
    m_pSFC->Insert(pMem, dwStart, dwSize);
    m_pSFCCache->Clear(dwStart);
  } else {
    m_pSFC->Write(pMem, dwStart, dwSize);
    m_pSFCCache->Clear(dwStart, dwSize);
  }
  ::GlobalUnlock(hMem);
  ::CloseClipboard();
  return dwStart+dwSize;

ERR_PASTECLIP3:
  ::GlobalUnlock(hMem);
ERR_PASTECLIP2:
  ::CloseClipboard();
ERR_PASTECLIP1:
  m_pSFCCache->Clear();
  return 0;
}

BOOL CBZDoc2::CopyToClipboard(DWORD dwStart, DWORD dwSize)
{
  if(!m_pSFC)return FALSE;
  HGLOBAL hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + 1);
  HGLOBAL hMemBin = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + sizeof(dwSize));
  if(!hMemTxt || !hMemBin)
  {
    AfxMessageBox(IDS_ERR_COPY);
    goto CClip_ERR2;
  }
  LPBYTE pMemTxt  = (LPBYTE)::GlobalLock(hMemTxt);
  LPBYTE pMemBin  = (LPBYTE)::GlobalLock(hMemBin);
  if(!pMemTxt || !pMemBin)
  {
    AfxMessageBox(IDS_ERR_COPY);
    goto CClip_ERR1;
  }

  if(!m_pSFC->ReadTwin(pMemTxt, pMemBin + sizeof(dwSize), dwStart, dwSize))
  {
    AfxMessageBox(IDS_ERR_COPY);
    goto CClip_ERR1;
  }
  *(pMemTxt + dwSize) = '\0';
  *((DWORD*)(pMemBin)) = dwSize;

  ::GlobalUnlock(hMemTxt);
  ::GlobalUnlock(hMemBin);
  AfxGetMainWnd()->OpenClipboard();
  ::EmptyClipboard();
  ::SetClipboardData(CF_TEXT, hMemTxt);
  ::SetClipboardData(RegisterClipboardFormat(_T("BinaryData2")), hMemBin);
  ::CloseClipboard();
  return TRUE;

CClip_ERR1:
  ::GlobalUnlock(hMemTxt);
  ::GlobalUnlock(hMemBin);
CClip_ERR2:
  ::GlobalFree(hMemTxt);
  ::GlobalFree(hMemBin);
  return FALSE;
}

BOOL CBZDoc2::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;
  return TRUE;
}

BOOL CBZDoc2::OnOpenDocument(LPCTSTR lpszPathName)
{
  CSuperFileCon *pSFC = new CSuperFileCon();
  if(!pSFC || !pSFC->Open(lpszPathName))
  {
//    ATLASSERT(FALSE);
    CString mes;
    mes.LoadString(AFX_IDP_INVALID_FILENAME);
    MessageBox(NULL, mes, lpszPathName, MB_OK);
    return FALSE;
  }
  DeleteContents();
  m_pSFC = pSFC;
  m_pSFCCache = new CSFCCache(m_pSFC, SFCC_CACHESIZE);
  return TRUE;
}

//BOOL CBZDoc2::OnSaveDocument(LPCTSTR lpszPathName)
//{
//  // TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
//
//  return CDocument::OnSaveDocument(lpszPathName);
//}

void CBZDoc2::OnFileSave()
{
  if(!m_pSFC || !m_pSFC->Save())
  {
    MessageBox(NULL, _T("Save Error"), _T("Error"), MB_OK);
    return;
  }
  SetModifiedFlag(FALSE);
}

void CBZDoc2::OnFileSaveAs()
{
  if(!m_pSFC)return;
  WTL::CFileDialog dlg(TRUE, _T("*"), NULL, OFN_PATHMUSTEXIST, _T("すべてのファイル (*.*)\0*.*\0\0"), AfxGetMainWnd()->GetSafeHwnd());

  if(dlg.DoModal() == IDOK){
    //if(!m_pSFC->SaveAs(dlg.m_szFileName))
    //{
    //  MessageBox(NULL, _T("SaveAs Error"), _T("Error"), MB_OK);
    //  return;
    //}
    //SetModifiedFlag(FALSE);
  }
}





/////////////////////////////////////////////////////////////////////////////
// CBZDoc2 Mark

void CBZDoc2::SetMark(DWORD dwPtr)
{
  for_to(i, m_arrMarks.GetSize()) {
    DWORD dwTotal = GetDocSize();
    if(m_arrMarks[i] == dwPtr) {
      m_arrMarks.RemoveAt(i);
      return;
    } else if(m_arrMarks[i] >= dwTotal) {
      m_arrMarks.RemoveAt(i);
    }
  }
  m_arrMarks.Add(dwPtr);
}

BOOL CBZDoc2::CheckMark(DWORD dwPtr)
{
  for_to(i,  m_arrMarks.GetSize()) {
    if(m_arrMarks[i] == dwPtr)
      return TRUE;
  }
  return FALSE;
}

DWORD CBZDoc2::JumpToMark(DWORD dwStart)
{
  DWORD dwTotal = GetDocSize();
  DWORD dwNext = dwTotal;
Retry:
  for_to(i, m_arrMarks.GetSize()) {
    if(m_arrMarks[i] > dwStart && m_arrMarks[i] < dwNext)
      dwNext = m_arrMarks[i];
  }
  if(dwNext == dwTotal && dwStart) {
    dwStart = 0;
    goto Retry;
  }
  if(dwNext < dwTotal)
    return dwNext;
  return INVALID;
}
