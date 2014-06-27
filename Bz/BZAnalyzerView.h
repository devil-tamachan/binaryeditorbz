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

#include "zlib.h"


extern __inline BOOL IsZlibDeflate(unsigned char firstChar, unsigned char secondChar);

class CBZAnalyzerView : public CDialogImpl<CBZAnalyzerView>, public WTL::CWinDataExchange<CBZAnalyzerView>
{

public:
  CBZAnalyzerView()
  {
  }

  virtual ~CBZAnalyzerView()
  {
  }

public:
  enum { IDD = IDD_BZANALYZERVIEW };

  BEGIN_MSG_MAP(CSetupColorDialog)
    MSG_WM_INITDIALOG(OnInitDialog)
    COMMAND_ID_HANDLER_EX(IDB_ANALYZE_START, OnBnClickedAnalyzeStart)
    COMMAND_ID_HANDLER_EX(IDB_ANALYZER_SAVE, OnBnClickedAnalyzerSave)
    COMMAND_ID_HANDLER_EX(IDB_ANALYZER_SAVEALL, OnBnClickedAnalyzerSaveall)
  END_MSG_MAP()

  BEGIN_DDX_MAP(CSetupColorDialog)
    DDX_CONTROL_HANDLE(IDP_ANALYZE_PERCENT, m_progress);
    DDX_CONTROL_HANDLE(IDL_ANALYZE_RESULT, m_resultList);
    DDX_CONTROL_HANDLE(IDC_ANALYZE_TYPE, m_combo_analyzetype);
  END_DDX_MAP()

public:
  WTL::CProgressBarCtrl m_progress;
  WTL::CListViewCtrl m_resultList;
  WTL::CComboBox m_combo_analyzetype;

  CBZView* GetBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZViewFromSubView(this);
  }
  CBZDoc2* GetBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZDoc2FromSubView(this);
  }
  CTamaSplitterWindow* GetSplitter()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSplitterWnd();
  }

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		DoDataExchange(DDX_LOAD);

    if(m_combo_analyzetype.GetCount()==0)
    {
      m_combo_analyzetype.InsertString(0, _T("zlib (deflate)"));
      m_combo_analyzetype.SetCurSel(0);

      m_resultList.DeleteAllItems();
      m_resultList.InsertColumn(0, _T("Address"), LVCFMT_LEFT, 120);
      //	m_resultList.InsertColumn(1, "Size", LVCFMT_LEFT, 80);
    }
    //CTamaSplitterWindow* pSplit = GetSplitter();
//    pSplit->SetColumnInfo(0, 180, 0);

    LONG lExStyle = GetWindowLong(GWL_EXSTYLE);
    lExStyle |= WS_EX_STATICEDGE;
    SetWindowLong(GWL_EXSTYLE, lExStyle);

    ShowWindow(SW_SHOW);

		return TRUE;
  }

  void Clear()
  {
    ATLTRACE("AnalyzerClear!\n");
    if(::IsWindow(m_resultList.m_hWnd))
      m_resultList.DeleteAllItems();
  }

  void OnBnClickedAnalyzeStart(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    m_resultList.DeleteAllItems();

    //	CProgressDialog dlgProgress;
    //	dlgProgress.DoModal();

    //	m_resultList.InsertItem(0, "0x00000000");
    //	m_resultList.SetItemText(0, 1, "5000");

    int iListIndex = -1;

    CBZDoc2* pDoc = GetBZDoc2();
    ATLASSERT(pDoc);
    //	LPBYTE p  = pDoc->GetDocPtr();
    UINT64 dwFileSize = pDoc->GetDocSize();

    unsigned int outbufsize = 1;
    LPBYTE pOutBuf = (LPBYTE)malloc(outbufsize);
    int inflateStatus = Z_OK;

    const DWORD dwInputBuf = 1000;
    const DWORD dwDecodeMax = 100;
    const DWORD dwLoopInc = dwInputBuf - dwDecodeMax;
    LPBYTE pInputBuf = (LPBYTE)malloc(dwInputBuf);

    if(pInputBuf && pOutBuf && pDoc->IsOpen())
    {
      for(DWORD ofs_inflateStart = 0; ofs_inflateStart < dwFileSize-1/*-2ˆÈã‚Å‚à‚¢‚¢‚©‚à*/; ofs_inflateStart+=dwLoopInc)
      {
        UINT64 dwRemain64 = dwFileSize - ofs_inflateStart;
        DWORD dwReadSize;
        if(dwRemain64 > dwInputBuf)dwReadSize = dwInputBuf;
        else dwReadSize = (DWORD)dwRemain64;
        if(!pDoc->Read(pInputBuf, ofs_inflateStart, dwReadSize)) break;

        for(DWORD i=0; i<dwLoopInc; i++)
        {
          if(dwReadSize < 2 || !IsZlibDeflate(*pInputBuf, *(pInputBuf+1)))continue;

          z_stream z = {0};
          z.next_out = pOutBuf;
          z.avail_out = outbufsize;

          if(inflateInit(&z)!=Z_OK)continue;

          DWORD dwDecodeSize = min(dwReadSize-i, dwDecodeMax);
          z.next_in = pInputBuf+i;
          z.avail_in = dwDecodeSize;
          inflateStatus = inflate(&z, Z_NO_FLUSH);

          if(inflateStatus==Z_OK||inflateStatus==Z_STREAM_END)
          {
            CString str;
            str.Format(_T("0x%08X"), ofs_inflateStart);
            m_resultList.InsertItem(++iListIndex, str);
            //	m_resultList.SetItemText(0, 1, "5000");
          }
          ATLTRACE(_T("BZAnalyzerView(0x%08X) inflateStatus==%d\n"),ofs_inflateStart , inflateStatus);
          inflateEnd(&z);
        }
      }
    }

    if(pInputBuf)free(pInputBuf);
    if(pOutBuf)free(pOutBuf);

    //	m_resultList.InsertItem(++iListIndex, "End");
  }
  void OnBnClickedAnalyzerSave(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    WTL::CFolderDialog dlg(NULL, NULL, BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
    if(dlg.DoModal()==IDOK)
    {
      int nItem = m_resultList.GetSelectedIndex();
      if(nItem==-1)
      {
        MessageBox(_T("no selected"), _T("Error"), MB_OK);
        return;
      }

      unsigned int outbufsize = 1024000;
      LPBYTE outbuf = (LPBYTE)malloc(outbufsize);

      TCHAR pathOutputDir[_MAX_PATH];
      MakeExportDir(pathOutputDir, dlg.GetFolderPath());
      CAtlList<int> delList;

      while(nItem!=-1)
      {
        unsigned long ulStartAddr = GetAddress(nItem);
        if(FAILED(SaveFile(pathOutputDir, ulStartAddr, outbuf, outbufsize))) delList.AddHead(nItem);
        nItem = m_resultList.GetNextItem(nItem, LVNI_BELOW | LVNI_SELECTED);
      }
      free(outbuf);

      POSITION listpos = delList.GetHeadPosition();
      while(listpos!=NULL)
      {
        int nDelItem = delList.GetNext(listpos);
        m_resultList.DeleteItem(nDelItem);
      }
    }
  }
  void OnBnClickedAnalyzerSaveall(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    WTL::CFolderDialog dlg(NULL, NULL, BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
    if(dlg.DoModal()==IDOK)
    {
      unsigned int outbufsize = 1024000;
      LPBYTE outbuf = (LPBYTE)malloc(outbufsize);

      TCHAR pathOutputDir[_MAX_PATH];
      MakeExportDir(pathOutputDir, dlg.GetFolderPath());
      CAtlList<int> delList;

      int itemcount = m_resultList.GetItemCount();

      for(int i=0; i<itemcount; i++)
      {
        unsigned long ulStartAddr = GetAddress(i);
        if(FAILED(SaveFile(pathOutputDir, ulStartAddr, outbuf, outbufsize))) delList.AddHead(i);
      }
      free(outbuf);

      POSITION listpos = delList.GetHeadPosition();
      while(listpos!=NULL)
      {
        int nDelItem = delList.GetNext(listpos);
        m_resultList.DeleteItem(nDelItem);
      }
    }
  }

  unsigned long GetAddress(int nItem);
  BOOL MakeExportDir(LPTSTR pathOutputDir, LPCTSTR pathDstFolder);
  int MakeExportPath(LPTSTR pathOutput, LPCTSTR pathDir, unsigned long ulStartAddr);
  HRESULT SaveFile(LPCTSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize);

  int GetWindowIdealWidth()
  {
    return 180;
  }
};


