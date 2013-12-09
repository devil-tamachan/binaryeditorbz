#pragma once

// CBZDoc2 ドキュメント

class CBZDoc2 : public CDocument
{
  DECLARE_DYNCREATE(CBZDoc2)

public:
  CBZDoc2();
  virtual ~CBZDoc2();
#ifndef _WIN32_WCE
  virtual void Serialize(CArchive& ar);   // ドキュメント I/O に対してオーバーライドされました。
#endif
#ifdef _DEBUG
  virtual void AssertValid() const;
#ifndef _WIN32_WCE
  virtual void Dump(CDumpContext& dc) const;
#endif
#endif

  _inline UINT64	GetDocSize() { return m_pSFC ? m_pSFC->GetSize() : 0; }
  _inline BOOL IsOpen() { return m_pSFC ? m_pSFC->IsOpen() : FALSE; }

  _inline BOOL Read(void *dst1, UINT64 dwStart, size_t dwSize)      { BOOL bRet = m_pSFC ? m_pSFC->Read(dst1, dwStart, dwSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Write(void *pSrcData, UINT64 dwStart, size_t dwSize) { BOOL bRet = m_pSFC ? m_pSFC->Write(pSrcData, dwStart, dwSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Insert(void *pSrcData, UINT64 dwInsStart, size_t dwInsSize) { BOOL bRet = m_pSFC ? m_pSFC->Insert((LPBYTE)pSrcData, dwInsStart, dwInsSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Delete(UINT64 dwDelStart, UINT64 dwDelSize)                 { BOOL bRet = m_pSFC ? m_pSFC->Delete(dwDelStart, dwDelSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL Fill(LPBYTE pData, DWORD dwDataSize, UINT64 dwStart, DWORD dwFillSize) { BOOL bRet = m_pSFC ? m_pSFC->Fill(pData, dwDataSize, dwStart, dwFillSize) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }

  _inline BOOL DoUndo(UINT64 *pRetStart = NULL) { BOOL bRet = m_pSFC ? m_pSFC->Undo(pRetStart) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }
  _inline BOOL DoRedo(UINT64 *pRetStart = NULL) { BOOL bRet = m_pSFC ? m_pSFC->Redo(pRetStart) : FALSE; /*SetModifiedFlag(m_pSFC->IsModified());*/ return bRet; }

  _inline const LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0) { return m_pSFC ? m_pSFC->Cache(dwStart, dwIdealSize) : NULL; }
  _inline const LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize) { return m_pSFC ? m_pSFC->CacheForce(dwStart, dwNeedSize) : NULL; }
  _inline size_t GetMaxCacheSize()             { return m_pSFC ? m_pSFC->GetMaxCacheSize() : 0; }
  _inline size_t GetRemainCache(DWORD dwStart) { return m_pSFC ? m_pSFC->GetRemainCache(dwStart) : 0; }

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
    if(m_pSFC->AddRef())pDstDoc->m_pSFC = m_pSFC;
    pDstDoc->m_bReadOnly = m_bReadOnly;
    pDstDoc->m_dwBase = m_dwBase;
    pDstDoc->SetTitle(GetTitle());
    CString s = GetPathName();
    if(!s.IsEmpty())
      pDstDoc->SetPathName(s);
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
  DWORD	m_restoreCaret;
  POINT	m_restoreScroll;

private:
  CSuperFileCon *m_pSFC;
public:
  void PreCloseFrame(CFrameWnd* /*pFrameArg*/);

protected:
  virtual BOOL OnNewDocument();

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnEditReadOnly();
  afx_msg void OnUpdateEditReadOnly(CCmdUI *pCmdUI);
  afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
  afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
  afx_msg void OnEditReadOnlyOpen();
  afx_msg void OnUpdateEditReadOnlyOpen(CCmdUI *pCmdUI);
  afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
  afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
  virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
  virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
  virtual void DeleteContents();
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
};
