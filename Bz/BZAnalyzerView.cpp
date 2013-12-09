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

#include "stdafx.h"
#include "BZ.h"
#include "BZAnalyzerView.h"
#include "ProgressDialog.h"
#include "zlib.h"
#include "BZDoc2.h"
#include "Splitter.h"
//#include "..\..\CFolderDialog\folderdlg.h"//atldlgs.hからCFolderDialogだけ切り取ったもの。shlobj.hのinclude, CFolderDialogImpl, CFolderDialog, ATL_NO_VTABLEを__declspec(novtable)に変更, ATLTRACEをコメントアウトしたもの。代わりにWTLのatldlgs.hをインクルードしても良い。CPL感染するので同梱しない。


// CBZAnalyzerView

IMPLEMENT_DYNCREATE(CBZAnalyzerView, CFormView)

CBZAnalyzerView::CBZAnalyzerView()
	: CFormView(CBZAnalyzerView::IDD)
{

}

CBZAnalyzerView::~CBZAnalyzerView()
{
}

void CBZAnalyzerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDP_ANALYZE_PERCENT, m_progress);
	DDX_Control(pDX, IDL_ANALYZE_RESULT, m_resultList);
	DDX_Control(pDX, IDC_ANALYZE_TYPE, m_combo_analyzetype);
}

BEGIN_MESSAGE_MAP(CBZAnalyzerView, CFormView)
	ON_BN_CLICKED(IDB_ANALYZE_START, &CBZAnalyzerView::OnBnClickedAnalyzeStart)
	ON_BN_CLICKED(IDB_ANALYZER_SAVE, &CBZAnalyzerView::OnBnClickedAnalyzerSave)
	ON_BN_CLICKED(IDB_ANALYZER_SAVEALL, &CBZAnalyzerView::OnBnClickedAnalyzerSaveall)
END_MESSAGE_MAP()


// CBZAnalyzerView 診断

#ifdef _DEBUG
void CBZAnalyzerView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBZAnalyzerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBZAnalyzerView メッセージ ハンドラ

void CBZAnalyzerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	if(m_combo_analyzetype.GetCount()==0)
	{
		m_combo_analyzetype.InsertString(0, _T("zlib (deflate)"));
		m_combo_analyzetype.SetCurSel(0);

		m_resultList.DeleteAllItems();
		m_resultList.InsertColumn(0, _T("Address"), LVCFMT_LEFT, 120);
	//	m_resultList.InsertColumn(1, "Size", LVCFMT_LEFT, 80);
	}
	CSplitterWnd* pSplit = (CSplitter*)GetParent();
	pSplit->SetColumnInfo(0, 180, 0);
}

void CBZAnalyzerView::Clear()
{
	TRACE("AnalyzerClear!\n");
	if(IsWindow(m_resultList.m_hWnd))
		m_resultList.DeleteAllItems();
}

unsigned char secondTables[8+1][8] = {	{0x1d, 0x5b, 0x99, 0xd7, 0x3c, 0x7a, 0xb8, 0xf6},//0x08(first)
										{0x19, 0x57, 0x95, 0xd3, 0x38, 0x76, 0xb4, 0xf2},//0x18
										{0x15, 0x53, 0x91, 0xcf, 0x34, 0x72, 0xb0, 0xee},//0x28
										{0x11, 0x4f, 0x8d, 0xcb, 0x30, 0x6e, 0xac, 0xea},//0x38
										{0x0d, 0x4b, 0x89, 0xc7, 0x2c, 0x6a, 0xa8, 0xe6},//0x48
										{0x09, 0x47, 0x85, 0xc3, 0x28, 0x66, 0xa4, 0xe2},//0x58
										{0x05, 0x43, 0x81, 0xde, 0x24, 0x62, 0xbf, 0xfd},//0x68
										{0x01, 0x5e, 0x9c, 0xda, 0x3f, 0x7d, 0xbb, 0xf9},//0x78
{0x00}//for End
};

__inline BOOL IsZlibDeflate(unsigned char firstChar, unsigned char secondChar)
{
	if(firstChar % 0x10==8)
	{
		unsigned char secondSwitch = firstChar/0x10;
		if(secondSwitch <= 7)
		{
			unsigned char *pSecond = secondTables[secondSwitch];
			unsigned char *pEnd = secondTables[secondSwitch+1];
			for(; pSecond != pEnd; pSecond++)
			{
				if(*pSecond==secondChar)return true;
			}
		}
	}
	return false;
}

void CBZAnalyzerView::OnBnClickedAnalyzeStart()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_resultList.DeleteAllItems();

//	CProgressDialog dlgProgress;
//	dlgProgress.DoModal();

//	m_resultList.InsertItem(0, "0x00000000");
//	m_resultList.SetItemText(0, 1, "5000");

	int iListIndex = -1;

	CBZDoc2* pDoc = (CBZDoc2*)GetDocument();
	ASSERT(pDoc);
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
    for(DWORD ofs_inflateStart = 0; ofs_inflateStart < dwFileSize-1/*-2以上でもいいかも*/; ofs_inflateStart+=dwLoopInc)
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
        TRACE(_T("BZAnalyzerView(0x%08X) inflateStatus==%d\n"),ofs_inflateStart , inflateStatus);
        inflateEnd(&z);
      }
    }
  }

  if(pInputBuf)free(pInputBuf);
	if(pOutBuf)free(pOutBuf);
	
//	m_resultList.InsertItem(++iListIndex, "End");
}

unsigned long CBZAnalyzerView::GetAddress(int nItem)
{
	TCHAR tmpbuf[20];
	m_resultList.GetItemText(nItem, 0, tmpbuf, 19);
	tmpbuf[19]=NULL;//Safety
	return  _tcstoul(tmpbuf+2, NULL, 16);
}

BOOL CBZAnalyzerView::MakeExportDir(LPTSTR pathOutputDir, LPCTSTR pathDstFolder)
{
	TCHAR lastDir[_MAX_PATH];
	_stprintf_s(lastDir, _MAX_PATH, _T("%s\\"), ::PathFindFileName(GetDocument()->GetPathName()));
	_tcscpy_s(pathOutputDir, _MAX_PATH, pathDstFolder);
	return ::PathAppend(pathOutputDir, lastDir);
}

int CBZAnalyzerView::MakeExportPath(LPTSTR pathOutput, LPCTSTR pathDir, unsigned long ulStartAddr)
{
	return _stprintf_s(pathOutput, _MAX_PATH, _T("%s%08X.bin"), pathDir, ulStartAddr);
}

void CBZAnalyzerView::OnBnClickedAnalyzerSave()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	WTL::CFolderDialog dlg(NULL, NULL, BIF_RETURNONLYFSDIRS | BIF_USENEWUI);
	if(dlg.DoModal()==IDOK)
	{
		POSITION pos = m_resultList.GetFirstSelectedItemPosition();
		if(pos==NULL)
		{
			MessageBox(_T("no selected"), _T("Error"), MB_OK);
			return;
		}

		unsigned int outbufsize = 1024000;
		LPBYTE outbuf = (LPBYTE)malloc(outbufsize);

		TCHAR pathOutputDir[_MAX_PATH];
		MakeExportDir(pathOutputDir, dlg.GetFolderPath());
		CAtlList<int> delList;

		while(pos)
		{
			int nItem = m_resultList.GetNextSelectedItem(pos);
			unsigned long ulStartAddr = GetAddress(nItem);
			if(FAILED(SaveFile(pathOutputDir, ulStartAddr, outbuf, outbufsize))) delList.AddHead(nItem);
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

void CBZAnalyzerView::OnBnClickedAnalyzerSaveall()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
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

HRESULT CBZAnalyzerView::SaveFile(LPCTSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize)
{
	TCHAR pathOutput[_MAX_PATH];

	CBZDoc2* pDoc = (CBZDoc2*)GetDocument();
	ASSERT(pDoc);
//	LPBYTE p  = pDoc->GetDocPtr();
  if(!pDoc->IsOpen())return E_FAIL;

	int retMakePath = MakeExportPath(pathOutput, pathOutputDir, ulStartAddr);
	FILE *fp;
	if(retMakePath==-1)
	{
		MessageBox(_T("Path error"), _T("Error"), MB_OK);
		return E_FAIL;
	}
#ifdef _UNICODE
	int dirRet = ::SHCreateDirectoryEx(NULL, pathOutputDir, NULL);
	if(dirRet!=ERROR_SUCCESS && dirRet!=ERROR_ALREADY_EXISTS && dirRet!=ERROR_FILE_EXISTS)
#else
	if(!MakeSureDirectoryPathExists(pathOutputDir))
#endif
	{
		MessageBox(_T("makedir error"), _T("Error"), MB_OK);
		return E_FAIL;
	}
	if(_tfopen_s(&fp, pathOutput, _T("wb"))!=0)
	{
		MessageBox(_T("fopen error"), _T("Error"), MB_OK);
		return E_FAIL;
	}


	z_stream z = {0};
	z.next_out = outbuf;
	z.avail_out = outbufsize;
	z.avail_out = 0;
	int inflateStatus = Z_OK;

	if(inflateInit(&z)!=Z_OK)
	{
		MessageBox(_T("inflateInit error"), _T("Error"), MB_OK);
		goto saveerr;
	}

	DWORD nextOffset = ulStartAddr;
	UINT64 dwFileSize = pDoc->GetDocSize();
  const DWORD dwReadMax = 0x100000;
  LPBYTE pInputBuf = (LPBYTE)malloc(dwReadMax);
  if(!pInputBuf)goto saveerr2;
	
	do {
		if(z.avail_in==0)
		{
			if(nextOffset>=dwFileSize)goto saveerr3;

      UINT64 dwRemain64 = dwFileSize - nextOffset;
      DWORD dwReadSize;
      if(dwRemain64 > dwReadMax)dwReadSize = dwReadMax;
      else dwReadSize = (DWORD)dwRemain64;
      if(!pDoc->Read(pInputBuf, nextOffset, dwReadSize))
			{
				MessageBox(_T("Read Error"), _T("ERROR"), MB_OK);
				goto saveerr3;
			}
			z.next_in = pInputBuf;
			z.avail_in = dwReadSize;
			nextOffset+=dwReadSize;
		}
		inflateStatus = inflate(&z, Z_NO_FLUSH);
		if(z.avail_out==0)
		{
			if(fwrite(outbuf, 1, outbufsize, fp)!=outbufsize)
			{
				MessageBox(_T("fwrite error"), _T("Error"), MB_OK);
				goto saveerr3;
			}
			z.next_out = outbuf;
			z.avail_out = outbufsize;
		}
	} while(inflateStatus==Z_OK);

	if(inflateStatus==Z_STREAM_END)
	{
		DWORD nokori = outbufsize - z.avail_out;
		if(nokori!=0)
		{
			if(fwrite(outbuf, 1, nokori, fp)!=nokori)
			{
				MessageBox(_T("fwrite error2"), _T("Error"), MB_OK);
				goto saveerr3;
			}
		}
	} else {
//		MessageBox("inflate error", "Error", MB_OK);
		goto saveerr3;
	}

	if(_ftelli64(fp)<=0)
	{
		goto saveerr3;
	}

	inflateEnd(&z);
	fclose(fp);
  free(pInputBuf);

	return S_OK;

saveerr3:
  free(pInputBuf);

saveerr2:
	inflateEnd(&z);

saveerr:
	fclose(fp);
	DeleteFile(pathOutput);

	return E_FAIL;
}