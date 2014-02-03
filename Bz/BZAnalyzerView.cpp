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
#include "BZDoc2.h"
#include "zlib.h"
#include "BZAnalyzerView.h"
//#include "..\..\CFolderDialog\folderdlg.h"//atldlgs.hからCFolderDialogだけ切り取ったもの。shlobj.hのinclude, CFolderDialogImpl, CFolderDialog, ATL_NO_VTABLEを__declspec(novtable)に変更, ATLTRACEをコメントアウトしたもの。代わりにWTLのatldlgs.hをインクルードしても良い。CPL感染するので同梱しない。


// CBZAnalyzerView



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
	_stprintf_s(lastDir, _MAX_PATH, _T("%s\\"), ::PathFindFileName(GetBZDoc2()->GetFilePath()));
	_tcscpy_s(pathOutputDir, _MAX_PATH, pathDstFolder);
	return ::PathAppend(pathOutputDir, lastDir);
}

int CBZAnalyzerView::MakeExportPath(LPTSTR pathOutput, LPCTSTR pathDir, unsigned long ulStartAddr)
{
	return _stprintf_s(pathOutput, _MAX_PATH, _T("%s%08X.bin"), pathDir, ulStartAddr);
}


HRESULT CBZAnalyzerView::SaveFile(LPCTSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize)
{
	TCHAR pathOutput[_MAX_PATH];

	CBZDoc2* pDoc = GetBZDoc2();
	ATLASSERT(pDoc);
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