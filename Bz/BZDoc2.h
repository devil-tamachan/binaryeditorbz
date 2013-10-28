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

  _inline DWORD	GetDocSize() { return m_pSFC ? m_pSFC->GetSize() : 0; }
  _inline BOOL IsOpen() { return m_pSFC ? m_pSFC->IsOpen() : FALSE; }

  _inline BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize) { return m_pSFC ? m_pSFC->Read(dst1, dwStart, dwSize) : FALSE; }
  _inline BOOL Write(void *pSrcData, DWORD dwStart, DWORD dwSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Write(pSrcData, dwStart, dwSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwStart, dwSize);
    else m_pSFCCache->Clear();
    return bRet;
  }
  _inline BOOL Insert(void *pSrcData, DWORD dwInsStart, DWORD dwInsSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Insert((LPBYTE)pSrcData, dwInsStart, dwInsSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwInsStart);
    else m_pSFCCache->Clear();
    return bRet;
  }
  _inline BOOL Delete(DWORD dwDelStart, DWORD dwDelSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Delete(dwDelStart, dwDelSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwDelStart);
    else m_pSFCCache->Clear();
    return bRet;
  }
  _inline BOOL Fill(LPBYTE pData, DWORD dwDataSize, DWORD dwStart, DWORD dwFillSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Fill(pData, dwDataSize, dwStart, dwFillSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwStart);
    else m_pSFCCache->Clear();
    return bRet;
  }

  _inline BOOL DoUndo(DWORD *pRetStart = NULL)
  {
    m_pSFCCache->Clear();
    return m_pSFC ? m_pSFC->Undo(pRetStart) : FALSE;
  }

  DWORD PasteFromClipboard(DWORD dwStart, BOOL bIns);
  BOOL CopyToClipboard(DWORD dwStart, DWORD dwSize);

  _inline DWORD GetMaxCacheSize() { return m_pSFCCache ? m_pSFCCache->GetMaxCacheSize() : 0; }
  _inline DWORD GetRemainCache(DWORD dwStart)                  { return m_pSFCCache ? m_pSFCCache->GetRemain(dwStart) : 0; }
  _inline BOOL ClearCache(DWORD dwStart = 0, DWORD dwSize = 0) { return m_pSFCCache ? m_pSFCCache->Clear(dwStart, dwSize) : FALSE; }
//#ifndef _DEBUG
#if 1
  _inline LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0)   { return m_pSFCCache ? m_pSFCCache->Cache(dwStart, dwIdealSize) : NULL; }
  _inline LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)   { return m_pSFCCache ? m_pSFCCache->CacheForce(dwStart, dwNeedSize) : NULL; }
#else
  _inline LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0)
  {
    if(m_pSFCCache)
    {
      LPBYTE p = m_pSFCCache->Cache(dwStart, dwIdealSize);
      if(p)
      {
        DWORD dwDebug = GetRemainCache(dwStart);
        LPBYTE pDebug = (LPBYTE)malloc(dwDebug);
        if(!Read(pDebug, dwStart, dwDebug))ATLASSERT(FALSE);
        if(memcmp(pDebug, p, dwDebug)!=0)ATLASSERT(FALSE);
      }
      return p;
    } else return NULL;
  }
  _inline LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)
  {
    if(m_pSFCCache)
    {
      LPBYTE p = m_pSFCCache->CacheForce(dwStart, dwNeedSize);
      if(p)
      {
        LPBYTE pDebug = (LPBYTE)malloc(dwNeedSize);
        if(!Read(pDebug, dwStart, dwNeedSize))ATLASSERT(FALSE);
        if(memcmp(pDebug, p, dwNeedSize)!=0)ATLASSERT(FALSE);
      }
      return p;
    } else return NULL;
  }
#endif

public:
	void	SetMark(DWORD dwPtr);
	BOOL	CheckMark(DWORD dwPtr);
	DWORD	JumpToMark(DWORD dwPtr);
private:
	CDWordArray m_arrMarks;

public:
  BOOL IsReadOnly() { return m_bReadOnly; }
private:
  DWORD m_bReadOnly;

public:
  void ReleaseSFC()
  {
    if(m_pSFC)
    {
      m_pSFC->Close();
      delete m_pSFC;
      m_pSFC = NULL;
    }
    if(m_pSFCCache)
    {
      delete m_pSFCCache;
      m_pSFCCache = NULL;
    }
  }
  DWORD	m_dwBase;		// ###1.63
  DWORD	m_restoreCaret;
  POINT	m_restoreScroll;

private:
  CSuperFileCon *m_pSFC;
  CSFCCache *m_pSFCCache;
  static const DWORD SFCC_CACHESIZE = 1024*1024*1; //1MB

protected:
  virtual BOOL OnNewDocument();

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnEditReadOnly();
  afx_msg void OnUpdateEditReadOnly(CCmdUI *pCmdUI);
  afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
  afx_msg void OnEditReadOnlyOpen();
  afx_msg void OnUpdateEditReadOnlyOpen(CCmdUI *pCmdUI);
  afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
  virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
  //  virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
  virtual void DeleteContents();
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
};
