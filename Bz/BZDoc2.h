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

#include "MainFrm.h"

class CMainFrame;

class CBZDoc2
{
public:
  CBZDoc2();
  virtual ~CBZDoc2();

  _inline UINT64	GetDocSize() { return m_pSFC ? m_pSFC->GetSize() : 0; }
  _inline BOOL IsOpen() { return m_pSFC ? m_pSFC->IsOpen() : FALSE; }

  _inline BOOL Read(void *dst1, UINT64 dwStart, size_t dwSize)      { BOOL bRet = m_pSFC ? m_pSFC->Read(dst1, dwStart, dwSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Write(void *pSrcData, UINT64 dwStart, size_t dwSize) { BOOL bRet = m_pSFC ? m_pSFC->Write(pSrcData, dwStart, dwSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Insert(void *pSrcData, UINT64 dwInsStart, size_t dwInsSize) { BOOL bRet = m_pSFC ? m_pSFC->Insert((LPBYTE)pSrcData, dwInsStart, dwInsSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Delete(UINT64 dwDelStart, UINT64 dwDelSize)                 { BOOL bRet = m_pSFC ? m_pSFC->Delete(dwDelStart, dwDelSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Fill(LPBYTE pData, DWORD dwDataSize, UINT64 dwStart, DWORD dwFillSize) { BOOL bRet = m_pSFC ? m_pSFC->Fill(pData, dwDataSize, dwStart, dwFillSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }

  _inline BOOL DoUndo(UINT64 *pRetStart = NULL) { BOOL bRet = m_pSFC ? m_pSFC->Undo(pRetStart) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL DoRedo(UINT64 *pRetStart = NULL) { BOOL bRet = m_pSFC ? m_pSFC->Redo(pRetStart) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }

  _inline const LPBYTE Cache(UINT64 dwStart, size_t dwIdealSize = 0) { return m_pSFC ? m_pSFC->Cache(dwStart, dwIdealSize) : NULL; }
  _inline const LPBYTE CacheForce(UINT64 dwStart, size_t dwNeedSize) { return m_pSFC ? m_pSFC->CacheForce(dwStart, dwNeedSize) : NULL; }
  _inline size_t GetMaxCacheSize()             { return m_pSFC ? m_pSFC->GetMaxCacheSize() : 0; }
  _inline size_t GetRemainCache(UINT64 dwStart) { return m_pSFC ? m_pSFC->GetRemainCache(dwStart) : 0; }
  _inline DWORD GetSFCRefCount() { return m_pSFC ? m_pSFC->GetRefCount() : 0; }

  _inline BOOL IsModified() { return m_pSFC ? m_pSFC->IsModified() : FALSE; };
  
  _inline void SetClearUndoRedoWhenSave(BOOL bClearUndoRedoWhenSave) { return m_pSFC ? m_pSFC->SetClearUndoRedoWhenSave(bClearUndoRedoWhenSave) : 0; }

  CString GetFilePath() { return m_pSFC ? m_pSFC->GetFilePath() : _T(""); }
  CString GetDocName()
  {
    CString retStr;
    CString path = GetFilePath();
    if(path==_T(""))retStr = _T("Untitled");
    else            retStr = PathFindFileName(path);
    return retStr;
  }

  DWORD PasteFromClipboard(DWORD dwStart, BOOL bIns);
  BOOL CopyToClipboard(DWORD dwStart, DWORD dwSize);

public:
  void  CleanInvalidMark();
	void	SetMark(UINT64 dwPtr);
	BOOL	CheckMark(UINT64 dwPtr);
	UINT64	JumpToMark(UINT64 dwPtr);
private:
	CAtlList<UINT64> m_arrMarks;

public:
  void DuplicateDoc(CBZDoc2* pDstDoc)
  {
    if(m_pSFC && m_pSFC->AddRef())pDstDoc->m_pSFC = m_pSFC;
    pDstDoc->m_bReadOnly = m_bReadOnly;
    pDstDoc->m_dwBase = m_dwBase;
    //pDstDoc->SetTitle(GetTitle());
    //CString s = GetPathName();
    //if(!s.IsEmpty())
    //  pDstDoc->SetPathName(s);
    //	pDstDoc->UpdateAllViews(NULL);

    //Restore infomation
    pDstDoc->m_restoreCaret = m_restoreCaret;
    pDstDoc->m_restoreScroll = m_restoreScroll;
  }

public:
  BOOL IsReadOnly() { return m_bReadOnly; }
private:
  DWORD m_bReadOnly;

public:
  void ReleaseSFC()
  {
    if(m_pSFC)
    {
      m_pSFC->DecRef();
      if(m_pSFC->GetRefCount()==0)
      {
        m_pSFC->Close();
        delete m_pSFC;
      }
      m_pSFC = NULL;
    }
  }
  DWORD	m_dwBase;		// ###1.63
  UINT64	m_restoreCaret;
  POINT	m_restoreScroll;

private:
  CSuperFileCon *m_pSFC;
public:
  //void PreCloseFrame(CFrameWnd* /*pFrameArg*/);

protected:
  //virtual BOOL OnNewDocument();

public:
  void OnUpdateEditReadOnly(BOOL *bEnable, BOOL *bChecked);
  BOOL OnUpdateEditUndo();
  BOOL OnUpdateEditRedo();
  BOOL OnUpdateFileSave();
  BOOL OnUpdateFileSaveAs();

private:
  BOOL OpenDocument(LPCTSTR lpszPathName, HWND hWnd = NULL);
  //BOOL OnSaveDocument(LPCTSTR lpszPathName);
public:
  BOOL CloseDocument(HWND hWnd = NULL);
private:
  void DeleteContents(BOOL bRecreateSFC = TRUE);

public:
  BOOL OnFileOpen(LPCTSTR lpszPathName = NULL, HWND hWnd = NULL);
  BOOL OnFileSave(HWND hWnd = NULL);
  void OnFileSaveAs(HWND hWnd = NULL);
  void OnEditReadOnly();
};
