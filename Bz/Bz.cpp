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

#include "MainFrm.h"
#include "BZDoc2.h"
#include "BZView.h"
#include <shlobj.h>

#ifdef _DEBUG
#include "kb976038.h"
#endif


CBZOptions options;
WTL::CAppModule _Module;
HINSTANCE g_hInstDll = NULL;
BOOL g_bFirstInstance;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
  WTL::CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
	disableUserModeCallbackExceptionFilter();
#endif

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

  WTL::AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	g_bFirstInstance = ::FindWindow(_T(BZ_CLASSNAME), NULL) == NULL;

	options.Load();
  if(options.bLanguage && (g_hInstDll = ::LoadLibrary(_T("BZres_us.dll"))))
  {
    _AtlBaseModule.SetResourceInstance(g_hInstDll);
  }

	int nRet = Run(lpstrCmdLine, nCmdShow);

	options.Save();
	if(g_hInstDll) ::FreeLibrary(g_hInstDll);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}




CString SeparateByComma(int num, BOOL bSigned)
{
	CString sSrc;
	sSrc.Format(bSigned ? _T("%d") : _T("%u"), num);
	TCHAR sDst[64];
	int n = sSrc.GetLength();
	LPCTSTR src = sSrc;
	LPTSTR dst = sDst;
	if(*src=='-' && n >= 2)
	{
		*dst++ = *src++; n--;
	}
	do {
		*dst++ = *src++; n--;
		if(!(n % 3) && n) *dst++ = ',';
	} while(n > 0);
	*dst++ = '\0';
	return sDst;
}

CString SeparateByComma64(ULONGLONG num, BOOL bSigned)
{
	CString sSrc;
	sSrc.Format(bSigned ? _T("%I64d") : _T("%I64u"), num);
	TCHAR sDst[64];
	int n = sSrc.GetLength();
	LPCTSTR src = sSrc;
	LPTSTR dst = sDst;
	if(*src=='-' && n >= 2)
	{
		*dst++ = *src++; n--;
	}
	do {
		*dst++ = *src++; n--;
		if(!(n % 3) && n) *dst++ = ',';
	} while(n > 0);
	*dst++ = '\0';
	return sDst;
}

CString GetModulePath(LPCTSTR pFileName) 
{
	CString sPath;
	LPTSTR pBuffer = sPath.GetBuffer(_MAX_PATH);
	::GetModuleFileName(NULL, pBuffer, _MAX_PATH - 1);
	TCHAR* p;
	p =  _tcsrchr(pBuffer, '\\');
	ASSERT(p);
	*(p+1) = '\0';
	sPath.ReleaseBuffer();
	sPath += pFileName;
	return sPath;
}


CString GetStructFilePath(UINT uID)
{
	CString sFileName, retStr;
	sFileName.LoadString(uID);
	TCHAR szAppData[_MAX_PATH];
	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, szAppData)))
	{
		PathAppend(szAppData, _T("\\BzEditor\\"));
		PathAppend(szAppData, sFileName);
		retStr=szAppData;
		if(!PathFileExists(szAppData))
		{
			retStr = GetModulePath(sFileName);//local file
		}
	}
	return retStr;
}

LPVOID ReadFile(LPCTSTR pPath)
{
	CFile file;
	if(!file.Open(pPath, CFile::modeRead | CFile::shareDenyNone)) {
		CString sMsg;
		sMsg.Format(IDS_ERR_FILENOTFOUND, pPath);
		AfxMessageBox(sMsg);
		return NULL;
	}
  UINT64 len64 = file.GetLength();
  if(len64 > _UI32_MAX)return NULL;
	DWORD len = (DWORD)len64;
	LPVOID p = new char[len + 1];
	file.Read(p, len);
	*((LPBYTE)p+len) = '\0';
	file.Close();
	return p;
}

CString GetErrorMessage() // ###1.60
{
	LPVOID lpMsgBuf = NULL;
	CString sMsg, sErr;
	DWORD dwErr = GetLastError();
	int nErr = LOWORD(dwErr);
	sErr.Format(_T("(%d)"), nErr);
	TRACE(_T("Error=%d\n"), nErr);
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
	if(lpMsgBuf) {
		sMsg = CString((LPCTSTR)lpMsgBuf);
		::LocalFree(lpMsgBuf);
	}
	if(sMsg.IsEmpty())
		sMsg = _T("Unknown error");
	sMsg.TrimRight();	// ###1.61
	sMsg += sErr;
	return sMsg;
}

void ErrorMessageBox() // ###1.61
{
	AfxMessageBox(GetErrorMessage(), MB_ICONHAND);
}



