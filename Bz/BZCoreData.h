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

class CBZDoc2;
class CBZView;
class CMainFrame;

class CBZCoreData
{
public:
  static CBZCoreData* GetInstance()
  {
    static CBZCoreData instance;
    return &instance;
  }

private:
  CBZCoreData(void) : m_pMainFrame(NULL), m_pSplitter(NULL), m_dwActive(0)
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
  CMainFrame *m_pMainFrame;
  DWORD m_dwActive;

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
  ATL::CWindow* GetSubViewFromBZView(CBZView *pBZView)
  {
    int i = m_arrView.Find(pBZView);
    if(i>=0)return m_arrSubView[i];
    return NULL;
  }
  CBZDoc2* GetBZDoc2FromBZView(CBZView *pBZView)
  {
    int i = m_arrView.Find(pBZView);
    if(i>=0)return m_arrDoc[i];
    return NULL;
  }
  CBZView* GetActiveBZView()
  {
    if(m_arrView.GetSize()>0)return m_arrView[m_dwActive];
    return NULL;
  }
  CBZDoc2* GetActiveBZDoc2()
  {
    if(m_arrDoc.GetSize()>0)return m_arrDoc[m_dwActive];
    return NULL;
  }

  DWORD GetCountBZDoc2()
  {
    return m_arrDoc.GetSize();
  }
  DWORD GetCountBZView()
  {
    return m_arrView.GetSize();
  }
  BOOL IsFirst()
  {
    return m_arrDoc.GetSize()==0;
  }
  CBZDoc2* GetBZDoc2(DWORD dwIndex)
  {
    return m_arrDoc[dwIndex];
  }
  CBZView* GetBZView(DWORD dwIndex)
  {
    return m_arrView[dwIndex];
  }
  ATL::CWindow* GetSubView(DWORD dwIndex)
  {
    return m_arrSubView[dwIndex];
  }

  CTamaSplitterWindow* GetSplitterWnd() { return m_pSplitter; }
  void SetSplitterWnd(CTamaSplitterWindow *pSplitter) { m_pSplitter = pSplitter; }
  CMainFrame* GetMainFrame() { return m_pMainFrame; }

  void DeleteView(DWORD dwIndex, BOOL bDelDoc = TRUE);
  void DeleteSubView(DWORD dwIndex);
  void DeleteAllSubViews()
  {
    while(m_arrSubView.GetSize()>0)DeleteSubView(0);
  }
  void DeleteAllViews(BOOL bDelDoc = TRUE)
  {
    while(GetCountBZView()>0)DeleteView(0, bDelDoc);
  }

  void AddBZDoc2(CBZDoc2 *doc)
  {
    m_arrDoc.Add(doc);
  }
  void AddBZView(CBZView *pView)
  {
    m_arrView.Add(pView);
  }
  void AddSubView(ATL::CWindow *pSubView)
  {
    m_arrSubView.Add(pSubView);
  }

  void SetActive(DWORD dwIndex)
  {
    m_dwActive = dwIndex;
  }
  void SetActiveByBZView(CBZView *pBZView)
  {
    int i = m_arrView.Find(pBZView);
    if(i>=0)m_dwActive = i;
  }
  void Invalidate(BOOL bErase = TRUE);
};
