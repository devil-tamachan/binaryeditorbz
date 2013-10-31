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

  _inline BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize)      { return m_pSFC ? m_pSFC->Read(dst1, dwStart, dwSize) : FALSE; }
  _inline BOOL Write(void *pSrcData, DWORD dwStart, DWORD dwSize) { return m_pSFC ? m_pSFC->Write(pSrcData, dwStart, dwSize) : FALSE; }
  _inline BOOL Insert(void *pSrcData, DWORD dwInsStart, DWORD dwInsSize) { return m_pSFC ? m_pSFC->Insert((LPBYTE)pSrcData, dwInsStart, dwInsSize) : FALSE; }
  _inline BOOL Delete(DWORD dwDelStart, DWORD dwDelSize)                 { return m_pSFC ? m_pSFC->Delete(dwDelStart, dwDelSize) : FALSE; }
  _inline BOOL Fill(LPBYTE pData, DWORD dwDataSize, DWORD dwStart, DWORD dwFillSize) { return m_pSFC ? m_pSFC->Fill(pData, dwDataSize, dwStart, dwFillSize) : FALSE; }

  _inline BOOL DoUndo(DWORD *pRetStart = NULL) { return m_pSFC ? m_pSFC->Undo(pRetStart) : FALSE; }
  _inline BOOL DoRedo(DWORD *pRetStart = NULL) { return m_pSFC ? m_pSFC->Redo(pRetStart) : FALSE; }

  _inline const LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0) { return m_pSFC ? m_pSFC->Cache(dwStart, dwIdealSize) : FALSE; }
  _inline const LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize) { return m_pSFC ? m_pSFC->CacheForce(dwStart, dwNeedSize) : FALSE; }
  _inline DWORD GetMaxCacheSize() { return m_pSFC ? m_pSFC->GetMaxCacheSize() : FALSE; }
  _inline DWORD GetRemainCache(DWORD dwStart) { return m_pSFC ? m_pSFC->GetRemainCache(dwStart) : FALSE; }

  DWORD PasteFromClipboard(DWORD dwStart, BOOL bIns);
  BOOL CopyToClipboard(DWORD dwStart, DWORD dwSize);

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
  }
  DWORD	m_dwBase;		// ###1.63
  DWORD	m_restoreCaret;
  POINT	m_restoreScroll;

private:
  CSuperFileCon *m_pSFC;

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
  virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
  //  virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
  virtual void DeleteContents();
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();
};
