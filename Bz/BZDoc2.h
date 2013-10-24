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

  DWORD	GetDocSize() { return m_pSFC ? m_pSFC->GetSize() : 0; }
  BOOL IsOpen() { return m_pSFC ? m_pSFC->IsOpen() : FALSE; }

  BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize) { return m_pSFC ? m_pSFC->Read(dst1, dwStart, dwSize) : FALSE; }
  BOOL Write(void *pSrcData, DWORD dwStart, DWORD dwSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Write(pSrcData, dwStart, dwSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwStart, dwSize);
    else m_pSFCCache->Clear();
    return bRet;
  }
  BOOL Insert(void *pSrcData, DWORD dwInsStart, DWORD dwInsSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Insert((LPBYTE)pSrcData, dwInsStart, dwInsSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwInsStart);
    else m_pSFCCache->Clear();
    return bRet;
  }
  BOOL Delete(DWORD dwDelStart, DWORD dwDelSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Delete(dwDelStart, dwDelSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwDelStart);
    else m_pSFCCache->Clear();
    return bRet;
  }
  BOOL Fill(LPBYTE pData, DWORD dwDataSize, DWORD dwStart, DWORD dwFillSize)
  {
    BOOL bRet = m_pSFC ? m_pSFC->Fill(pData, dwDataSize, dwStart, dwFillSize) : FALSE;
    if(bRet)m_pSFCCache->Clear(dwStart);
    else m_pSFCCache->Clear();
    return bRet;
  }

  BOOL DoUndo(DWORD *pRetStart = NULL)
  {
    m_pSFCCache->Clear();
    return m_pSFC ? m_pSFC->Undo(pRetStart) : FALSE;
  }

  DWORD PasteFromClipboard(DWORD dwStart, BOOL bIns);
  BOOL CopyToClipboard(DWORD dwStart, DWORD dwSize);

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
