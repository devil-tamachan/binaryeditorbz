#pragma once

class CBZDoc2;
class CBZView;

class CBZCoreData
{
public:
  static CBZCoreData* GetInstance()
  {
    static CBZCoreData instance;
    return &instance;
  }

private:
  CBZCoreData(void) : m_pSplitter(NULL), m_dwActive(0), m_bSubView(FALSE)
  {
  }

  ~CBZCoreData(void)
  {
  }

public:
  CSimpleArray<CBZDoc2*> m_arrDoc;
  CSimpleArray<CBZView*> m_arrView;
  CSimpleArray<ATL::CWindow *> m_arrSubView;
  CTamaSplitterWindow* m_pSplitter;
  DWORD m_dwActive;
  BOOL m_bSubView;

public:
  CBZView* GetBZViewFromSubView(ATL::CWindow *pSubWin)
  {
    int i = m_arrSubView.Find(pSubWin);
    if(i>=0)return m_arrView[i];
    return NULL;
  }
  CBZDoc2* GetBZDoc2FromSubView(ATL::CWindow *pSubWin)
  {
    int i = m_arrSubView.Find(pSubWin);
    if(i>=0)return m_arrDoc[i];
    return NULL;
  }

  CTamaSplitterWindow* GetSplitterWnd()
  {
    return m_pSplitter;
  }

  void DeleteView(DWORD dwIndex)
  {
    m_arrDoc.RemoveAt(dwIndex);
    m_arrView.RemoveAt(dwIndex);
    if(m_bSubView)m_arrSubView.RemoveAt(dwIndex);
    if(m_dwActive==dwIndex)m_dwActive = 0;
    else if(m_dwActive>dwIndex)m_dwActive--;
  }

  void SetActive(DWORD dwIndex)
  {
    m_dwActive = dwIndex;
  }
};
