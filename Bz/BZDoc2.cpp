// BZDoc2.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BZ.h"
#include "BZDoc2.h"



CBZDoc2::CBZDoc2() : m_pSFC(NULL)
{
  m_dwBase = 0;

  //ReCreate restore
  m_restoreCaret = 0;
  //m_restoreScroll = {0};
  m_bReadOnly = FALSE;
  //m_pSFC = new CSuperFileCon();
}

void CBZDoc2::DeleteContents()
{
  m_dwBase = 0;
  m_restoreCaret = 0;
  //m_restoreScroll = {0};
  m_bReadOnly = FALSE;

  ReleaseSFC();
  m_pSFC = new CSuperFileCon();
  m_arrMarks.RemoveAll();
}

CBZDoc2::~CBZDoc2()
{
  ReleaseSFC();
}

/*
void CBZDoc2::PreCloseFrame(CFrameWnd* pFrameArg)
{
  if(m_pSFC && m_pSFC->IsModified())
  {
    CString filePath = m_pSFC->GetFilePath();
    if(filePath == "")filePath.LoadString(AFX_IDS_UNTITLED);
    CString msgAskSave;
    AfxFormatString1(msgAskSave, AFX_IDP_ASK_TO_SAVE, filePath);
    UINT nResult = AfxMessageBox(msgAskSave, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE);
    if(nResult==IDYES)
    {
      OnFileSave();
    }
  }
}*/


void CBZDoc2::OnEditReadOnly()
{
  if(m_pSFC && !m_pSFC->IsReadOnly()) m_bReadOnly = !m_bReadOnly;	
}



void CBZDoc2::OnUpdateEditReadOnly(BOOL *bEnable, BOOL *bChecked)
{
  *bChecked = m_bReadOnly;
  *bEnable = m_pSFC && !m_pSFC->IsReadOnly();
}


BOOL CBZDoc2::OnUpdateEditUndo()
{
  return m_pSFC && m_pSFC->GetUndoCount()!=0;
}
BOOL CBZDoc2::OnUpdateEditRedo()
{
  return m_pSFC && m_pSFC->GetRedoCount()!=0;
}

BOOL CBZDoc2::OnUpdateFileSave() 
{
  return !m_bReadOnly && m_pSFC && m_pSFC->IsModified();
}
BOOL CBZDoc2::OnUpdateFileSaveAs()
{
  return !m_bReadOnly && m_pSFC;
}




DWORD CBZDoc2::PasteFromClipboard(DWORD dwStart, BOOL bIns)
{
  if(!m_pSFC || !OpenClipboard(NULL))goto ERR_PASTECLIP1;//AfxGetMainWnd()->OpenClipboard();
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
  UINT64 dwTotal = m_pSFC->GetSize();
  if(bIns || dwStart == dwTotal)
  {
    m_pSFC->Insert(pMem, dwStart, dwSize);
  } else {
    m_pSFC->Write(pMem, dwStart, dwSize);
  }
  ::GlobalUnlock(hMem);
  ::CloseClipboard();
  return dwStart+dwSize;

ERR_PASTECLIP3:
  ::GlobalUnlock(hMem);
ERR_PASTECLIP2:
  ::CloseClipboard();
ERR_PASTECLIP1:
  return 0;
}

BOOL CBZDoc2::CopyToClipboard(DWORD dwStart, DWORD dwSize)
{
  if(!m_pSFC)return FALSE;
  HGLOBAL hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + 1);
  HGLOBAL hMemBin = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + sizeof(dwSize));
  if(!hMemTxt || !hMemBin)
  {
    ErrorResMessageBox(IDS_ERR_COPY);
    goto CClip_ERR2;
  }
  LPBYTE pMemTxt  = (LPBYTE)::GlobalLock(hMemTxt);
  LPBYTE pMemBin  = (LPBYTE)::GlobalLock(hMemBin);
  if(!pMemTxt || !pMemBin)
  {
    ErrorResMessageBox(IDS_ERR_COPY);
    goto CClip_ERR1;
  }

  if(!m_pSFC->ReadTwin(pMemTxt, pMemBin + sizeof(dwSize), dwStart, dwSize))
  {
    ErrorResMessageBox(IDS_ERR_COPY);
    goto CClip_ERR1;
  }
  *(pMemTxt + dwSize) = '\0';
  *((DWORD*)(pMemBin)) = dwSize;

  ::GlobalUnlock(hMemTxt);
  ::GlobalUnlock(hMemBin);
  ::OpenClipboard(NULL);
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
//  if (!CDocument::OnNewDocument())
    return FALSE;
//  return TRUE;
}

BOOL CBZDoc2::OnOpenDocument(LPCTSTR lpszPathName, HWND hWnd)
{
  CSuperFileCon *pSFC = new CSuperFileCon();
  if(!pSFC || !pSFC->Open(lpszPathName))
  {
//    ATLASSERT(FALSE);
    CString mes;
    mes.LoadString(AFX_IDP_INVALID_FILENAME);
    MessageBox(hWnd, mes, lpszPathName, MB_OK);
    return FALSE;
  }
  DeleteContents();
  m_bReadOnly = pSFC->IsReadOnly();
  m_pSFC = pSFC;

  CMainFrame *pMainFrame = GetMainFrame();
  if(pMainFrame)pMainFrame->AddRecentList(lpszPathName);
  return TRUE;
}

BOOL CBZDoc2::OnSaveDocument(LPCTSTR lpszPathName)
{
  return TRUE;
  //return CDocument::OnSaveDocument(lpszPathName);
}

BOOL CBZDoc2::CloseDocument(HWND hWnd)
{
  if(m_pSFC && m_pSFC->IsModified())
  {
    CString msgAskSave;
    CString docName = GetDocName();
    msgAskSave.FormatMessage(AFX_IDP_ASK_TO_SAVE, docName);
    UINT nResult = ::MessageBox(hWnd, msgAskSave, docName, MB_YESNOCANCEL);
    switch(nResult)
    {
    case IDYES:
      if(!OnFileSave())return FALSE;
      break;
    case IDNO:
      break;
    case IDCANCEL:
      return FALSE;
    }
  }
  DeleteContents();
  return TRUE;
}

BOOL CBZDoc2::OnFileOpen(HWND hWnd)
{
  //CWaitCursor wait;
  BOOL bRet = FALSE;
  if(!CloseDocument(hWnd))return FALSE;

  WTL::CFileDialog dlg(TRUE, _T("*"), NULL, OFN_NOVALIDATE
      , _T("すべてのファイル (*)\0*\0\0"), hWnd);
  if(dlg.DoModal() == IDOK)bRet = OnOpenDocument(dlg.m_szFileName, hWnd);

  if(!bRet)
  {
    MessageBox(hWnd, _T("Open Error"), _T("Error"), MB_OK);
    return FALSE;
  }
  return TRUE;
}

BOOL CBZDoc2::OnFileSave(HWND hWnd)
{
  //CWaitCursor wait;
  if(!m_pSFC)
  {
    MessageBox(hWnd, _T("Save Error"), _T("Error"), MB_OK);
    return FALSE;
  }
  BOOL bRet = FALSE;
  if(m_pSFC->IsOpen())
  {
    bRet = m_pSFC->Save();
  } else {
    WTL::CFileDialog dlg(FALSE, _T("*"), NULL, OFN_NOVALIDATE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT/*OFN_PATHMUSTEXIST*/
      , _T("すべてのファイル (*)\0*\0\0"), hWnd);
    if(dlg.DoModal() == IDOK && m_pSFC)bRet = m_pSFC->SaveAs(dlg.m_szFileName);
  }
  if(!bRet)
  {
    MessageBox(hWnd, _T("Save Error"), _T("Error"), MB_OK);
    return FALSE;
  }
  return TRUE;
}

void CBZDoc2::OnFileSaveAs(HWND hWnd) //MFCからウィンドウを閉じる時にOnFileSaveAs()が呼び出される場合がある。仮実装
{
  //CWaitCursor wait;
  if(!m_pSFC)
  {
    MessageBox(hWnd, _T("SaveAs Error"), _T("Error"), MB_OK);
    return;
  }
  WTL::CFileDialog dlg(FALSE, _T("*"), NULL, OFN_NOVALIDATE | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT/*OFN_PATHMUSTEXIST*/
    , _T("すべてのファイル (*)\0*\0\0"), hWnd);

  if(dlg.DoModal() == IDOK){
    if(!m_pSFC || !m_pSFC->SaveAs(dlg.m_szFileName))
    {
      MessageBox(hWnd, _T("SaveAs Error"), _T("Error"), MB_OK);
      return;
    }
  }
}





/////////////////////////////////////////////////////////////////////////////
// CBZDoc2 Mark


void CBZDoc2::CleanInvalidMark()
{
  UINT64 dwTotal = GetDocSize();
  while(m_arrMarks.GetCount() > 0)
  {
    UINT64 dwCurrent = m_arrMarks.GetTail();
    if(dwCurrent >= dwTotal)m_arrMarks.RemoveTailNoReturn();
    else break;
  }
}

void CBZDoc2::SetMark(UINT64 dwPtr)
{
  CleanInvalidMark();
  POSITION posCur = m_arrMarks.GetHeadPosition();
  UINT64 dwTotal = GetDocSize();
  while(posCur != NULL)
  {
    UINT64 dwCurrent = m_arrMarks.GetAt(posCur);
    if(dwCurrent == dwPtr) {
      m_arrMarks.RemoveAt(posCur);
      CleanInvalidMark();
      return;
    } else if(dwCurrent > dwPtr) {
      posCur = m_arrMarks.InsertBefore(posCur, dwPtr);
      CleanInvalidMark();
      return;
    }
    m_arrMarks.GetNext(posCur);
  }
  m_arrMarks.AddTail(dwPtr);
}

BOOL CBZDoc2::CheckMark(UINT64 dwPtr)
{
  return m_arrMarks.Find(dwPtr)!=NULL;
}

UINT64 CBZDoc2::JumpToMark(UINT64 dwStart)
{
  CleanInvalidMark();
  POSITION posCur = m_arrMarks.GetHeadPosition();
  UINT64 dwTotal = GetDocSize();
  while(posCur != NULL)
  {
    UINT64 dwCurrent = m_arrMarks.GetAt(posCur);
    if(dwCurrent > dwStart)return dwCurrent;
    m_arrMarks.GetNext(posCur);
  }
  if(m_arrMarks.GetCount() > 0)return m_arrMarks.GetHead();
  return INVALID;
}