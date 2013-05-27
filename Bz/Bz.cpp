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
#include "BZDoc.h"
#include "BZView.h"
#include <shlobj.h>

#include "kb976038.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBZOptions options;

/////////////////////////////////////////////////////////////////////////////
// CBZApp

BEGIN_MESSAGE_MAP(CBZApp, CWinApp)
	//{{AFX_MSG_MAP(CBZApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
	ON_COMMAND(ID_TOOLS_EDITDEF, OnToolsEditBZDef)
	ON_COMMAND(ID_FILE_SAVE_DUMPLIST, OnFileSaveDumpList)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND( ID_HELP_INDEX, OnHelpIndex ) 
	ON_COMMAND_RANGE(ID_LANG_JPN, ID_LANG_ENU, OnLanguage)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LANG_JPN, ID_LANG_ENU, OnUpdateLanguage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZApp construction

CBZApp::CBZApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bFirstInstance = TRUE;
	m_hInstDll = NULL;
	//EnableHtmlHelp();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBZApp object

CBZApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBZApp initialization


BOOL CBZApp::InitInstance()
{
#ifdef _DEBUG
	disableUserModeCallbackExceptionFilter();
#endif

	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_STANDARD_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
//	afxAmbientActCtx = FALSE;//0xC015000F避け。SxS周りでエラーが起きる。これは一時的な処置にしかならない。根本的な問題は解決しないらしい。どうも前の方のバージョン（インスペクタ実装前）も発生しているっぽい。MFCのバージョンが違うためなのか？
/*インスペクタの適当なテキストボックスにカーソルを当てて、構造体表示にすると
0xC0150010
0xC015000F
のどちらかがランダムで出る（出る場所は違う）
KB976038かと思ったがどうも違うらしい。マイクロソフトのパッチを当てたが変わらず
デバッグでブレークポイントでチミチミやると発生しない・・・
http://social.msdn.microsoft.com/Forums/en/vcgeneral/thread/c3feab0f-601b-4ca6-beb2-8d4d615438cc
*/

	// OLE ライブラリを初期化します。
/*	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();*/
	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。

	SetRegistryKey(_T("c.mos"));
/*#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif*/

	m_bFirstInstance = ::FindWindow(_T(BZ_CLASSNAME), NULL) == NULL;

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
	options.Load();

	if(options.bLanguage && (m_hInstDll = ::LoadLibrary(_T("BZres_us.dll")))) 
		AfxSetResourceHandle(m_hInstDll); 

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CBZDocTemplate* pDocTemplate;
	pDocTemplate = new CBZDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CBZDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CBZView));

//	AddDocTemplate(pDocTemplate);
	if (m_pDocManager == NULL)
		m_pDocManager = new CBZDocManager;
	m_pDocManager->AddDocTemplate(pDocTemplate);

	// DDE、file open など標準のシェル コマンドのコマンド ラインを解析します。
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
//	if(!cmdInfo.m_strFileName.IsEmpty())
//		AfxMessageBox(cmdInfo.m_strFileName);
	ShortNameToLongName(cmdInfo.m_strFileName);

	m_nCmdShow = options.nCmdShow;


	// コマンド ラインで指定されたディスパッチ コマンドです。アプリケーションが
	// /RegServer、/Register、/Unregserver または /Unregister で起動された場合、False を返します。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 接尾辞が存在する場合にのみ DragAcceptFiles を呼び出してください。
	//  SDI アプリケーションでは、ProcessShellCommand の直後にこの呼び出しが発生しなければなりません。

//	OnFileNew();
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CBZApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	options.Save();
	if(m_hInstDll) ::FreeLibrary(m_hInstDll);
	return CWinApp::ExitInstance();
}

CBZDocTemplate::CBZDocTemplate(UINT nIDResource,
	CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass,
	CRuntimeClass* pViewClass)
		: CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
	m_pOnlyDoc = NULL;
}

void CBZDocTemplate::RemoveDocument(CDocument* pDoc)
{
	ASSERT_VALID(pDoc);
	CDocTemplate::RemoveDocument(pDoc);
	m_pOnlyDoc = NULL;
}

void CBZDocTemplate::SetDocument(CDocument* pDoc)
{
	m_pOnlyDoc = pDoc;
}

BOOL CBZDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	OPENFILENAME ofn;
	ZeroMem(ofn);
	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | lFlags;
	ofn.hInstance = AfxGetResourceHandle();
	ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	ofn.nMaxFile = _MAX_PATH;

	CString sTitle;
	sTitle.LoadString(nIDSTitle);
	CString sFilter;
	sFilter.LoadString(IDS_ALLFILTER);
	sFilter += (TCHAR)'\0';
	sFilter += _T("*.*");
	sFilter += (TCHAR)'\0';
	ofn.nMaxCustFilter++;
	ofn.lpstrFilter = sFilter;
	ofn.lpstrTitle = sTitle;
	ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);
	BOOL bResult = (bOpenFileDialog) ? ::GetOpenFileName(&ofn) : ::GetSaveFileName(&ofn);
	fileName.ReleaseBuffer();
	return bResult;
}

// App command to run the dialog
void CBZApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

CString SeparateByComma(int num, BOOL bSigned)
{
	CString sSrc;
	sSrc.Format(bSigned ? _T("%d") : _T("%u"), num);
	TCHAR sDst[64];
	int n = sSrc.GetLength();
	LPCTSTR src = sSrc;
	LPTSTR dst = sDst;
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
	DWORD len = file.GetLength();
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

void  CBZApp::OnLanguage(UINT nID)
{
	BOOL bLang = nID - ID_LANG_JPN;
	if(bLang != options.bLanguage) {
		options.bLanguage = bLang;
		AfxMessageBox(IDS_CHANGE_LANGUAGE);
	}
}

void CBZApp::OnUpdateLanguage(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((int)pCmdUI->m_nID == ID_LANG_JPN + options.bLanguage);
}

void CBZApp::OnFilePageSetup() 
{
	// TODO: Add your command handler code here

	CPageSetupDialog dlg;
	PAGESETUPDLG& psd = dlg.m_psd;
	PRINTDLG pd;
	pd.hDevNames = NULL;
	pd.hDevMode = NULL;
	GetPrinterDeviceDefaults(&pd);
	psd.hDevNames = pd.hDevNames;
	psd.hDevMode = pd.hDevMode;
	psd.rtMargin = options.rMargin;
	if (dlg.DoModal() == IDOK) {
		dlg.GetMargins(options.rMargin, NULL);
		SelectPrinter(psd.hDevNames, psd.hDevMode);
	}
/*
	BOOL bMetric = GetUnits() == 1; //centimeters
	psd.Flags |= PSD_MARGINS | (bMetric ? PSD_INHUNDREDTHSOFMILLIMETERS : PSD_INTHOUSANDTHSOFINCHES);
	int nUnitsPerInch = bMetric ? 2540 : 1000;
	MulDivRect(&psd.rtMargin, m_rectMargin, nUnitsPerInch, 1440);
	RoundRect(&psd.rtMargin);
	// get the current device from the app
	PRINTDLG pd;
	pd.hDevNames = NULL;
	pd.hDevMode = NULL;
	GetPrinterDeviceDefaults(&pd);
	psd.hDevNames = pd.hDevNames;
	psd.hDevMode = pd.hDevMode;
	if (dlg.DoModal() == IDOK) {
		RoundRect(&psd.rtMargin);
		MulDivRect(m_rectMargin, &psd.rtMargin, 1440, nUnitsPerInch);
		m_rectPageMargin = m_rectMargin;
		SelectPrinter(psd.hDevNames, psd.hDevMode);
		NotifyPrinterChanged();
	}
*/
}

void CBZApp::OnToolsEditBZDef() 
{
	// TODO: Add your command handler code here
	CString sPath;
	sPath = GetStructFilePath(IDS_STRUCT_FILE);

	CString sEditorPath;
	::FindExecutable(_T("bz.txt"), NULL, sEditorPath.GetBuffer(_MAX_PATH));	// ###1.60
	sEditorPath.ReleaseBuffer();
	if(sEditorPath.IsEmpty()) 
		::ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), _T("edit") , sPath, NULL, NULL, SW_SHOWNORMAL);
	else
		::ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), _T("open") , sEditorPath, sPath, NULL, SW_SHOWNORMAL);
}

void CBZApp::ShortNameToLongName(CString& sPath)
{
	if(sPath.Find(TCHAR('~')) == -1) return;

	TCHAR szLongPath[MAX_PATH];
	CComBSTR bstrShortPath = sPath.AllocSysString();
//	WCHAR wszShortPath[MAX_PATH];
//	::MultiByteToWideChar(CP_ACP, 0, sPath, -1, wszShortPath, MAX_PATH);

	LPMALLOC pMalloc;
	LPSHELLFOLDER pSF;
	LPITEMIDLIST pIDL;
	if(::SHGetMalloc(&pMalloc) == NOERROR) {
		if (::SHGetDesktopFolder(&pSF) == NOERROR) {
			if(pSF->ParseDisplayName(NULL, NULL, bstrShortPath, NULL, &pIDL, NULL) == NOERROR) {
                if (::SHGetPathFromIDList(pIDL, szLongPath)) {
					sPath = szLongPath;
				}
                pMalloc->Free(pIDL);
            }
            pSF->Release();
        }
        pMalloc->Release();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CBZOptions class

COLORREF colorsDefault[TCOLOR_COUNT][2] = {
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_3DLIGHT | SYSCOLOR,
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_WINDOW | SYSCOLOR,
	COLOR_WINDOWTEXT | SYSCOLOR, COLOR_WINDOW | SYSCOLOR,
	COLOR_HIGHLIGHTTEXT | SYSCOLOR, COLOR_HIGHLIGHT | SYSCOLOR,
	RGB_RED,  COLOR_WINDOW | SYSCOLOR,
	RGB_BLUE, COLOR_WINDOW | SYSCOLOR,
	RGB_BLUE, COLOR_WINDOW | SYSCOLOR,
	RGB_RED, COLOR_WINDOW | SYSCOLOR,
	RGB_GREEN2, COLOR_WINDOW | SYSCOLOR,
	RGB(0xe2,0x04,0x1b), RGB(0x3e,0xb3,0x70),
};

static int colWidthDefault[MBRCOL_MAX] = { 26, 80, 60 };

void CBZOptions::Load()
{
	/*for(int i=0;i<TCOLOR_COUNT;i++)
	{
		TRACE("{ ");
		for(int j=0;j<2;j++)
		{
			double r = ((double)GetRValue(colorsDefault[i][j]))/255.0;
			double g = (double)GetGValue(colorsDefault[i][j])/255.0;
			double b = (double)GetBValue(colorsDefault[i][j])/255.0;
			double a = (double)(LOBYTE((colorsDefault[i][j])>>24))/255.0;
			TRACE("{%f, %f, %f, %f}, ", r, g, b, a);
		}
		TRACE(" },\n");
	}*/
	charset = (CharSet)GetProfileInt(_T("CharSet"), CTYPE_ASCII);
	bAutoDetect = GetProfileInt(_T("AutoDetect"), FALSE);
	bByteOrder= GetProfileInt(_T("ByteOrder"), FALSE);
	sFontName = GetProfileString(_T("FontName"), _T("FixedSys"));	// ###1.54
	fFontStyle= GetProfileInt(_T("FontStyle"), 0);
	nFontSize = GetProfileInt(_T("FontSize"), 140);
	ptFrame.x = GetProfileInt(_T("FrameLeft"), 0);
	ptFrame.y = GetProfileInt(_T("FrameTop"), 0);
	nCmdShow  = GetProfileInt(_T("CmdShow"), SW_SHOWNORMAL);
	cyFrame   = GetProfileInt(_T("FrameHeight"), 0);
	cyFrame2  = GetProfileInt(_T("FrameHeight2"), 0);
	cxFrame2  = GetProfileInt(_T("FrameWidth2"), 0);
	xSplit    = GetProfileInt(_T("SplitHPos"), 0);
	ySplit    = GetProfileInt(_T("SplitVPos"), 0);
	xSplitStruct = 	GetProfileInt(_T("SplitStruct"), 0);
	bStructView = GetProfileInt(_T("StructView"), FALSE);
	nComboHeight = GetProfileInt(_T("ComboHeight"), 15);
	bLanguage = GetProfileInt(_T("Language"), ::GetThreadLocale() != 0x411);
	dwDetectMax = GetProfileInt(_T("DetectMax"), 0x10000);
	barState = GetProfileInt(_T("BarState"), BARSTATE_TOOL | BARSTATE_STATUS);
	bReadOnlyOpen = GetProfileInt(_T("ReadOnly"), TRUE);
	nBmpWidth = GetProfileInt(_T("BmpWidth"), 128);
	nBmpZoom =  GetProfileInt(_T("BmpZoom"), 1);
	dwMaxOnMemory = GetProfileInt(_T("MaxOnMemory"), 1024 * 1024);		// ###1.60
	dwMaxMapSize =  GetProfileInt(_T("MaxMapSize"), 1024 * 1024 * 64);	// ###1.61
	bTagAll =  GetProfileInt(_T("TagAll"), FALSE);
	bSubCursor =  GetProfileInt(_T("SubCursor"), TRUE);

	memcpy(colors, colorsDefault, sizeof(colorsDefault));
	GetProfileBinary2(_T("Colors"), colors, sizeof colors);
//	if(!GetProfileBinary("Colors", colors))
//		memcpy(colors, colorsDefault, sizeof(colorsDefault));
	if(!GetProfileBinary(_T("MemberColumns"), colWidth))
		memcpy(colWidth, colWidthDefault, sizeof(colWidthDefault));
	if(!GetProfileBinary(_T("PageMargin"), (LPRECT)rMargin))
		rMargin.SetRect(2000, 2000, 2000, 2000);

	sDumpHeader = GetProfileString(_T("DumpHeader"));	// ###1.63
	nDumpPage = GetProfileInt(_T("DumpPage"), 0);
	bDWordAddr = GetProfileInt(_T("DWordAddr"), FALSE);
	
	bSyncScroll = GetProfileInt(_T("SyncScroll"), true);
	iGrid = GetProfileInt(_T("Grid"), 0);
	nBmpColorWidth = GetProfileInt(_T("BmpColorWidth"), 8);
	switch(nBmpColorWidth)
	{
	case 8:
	case 24:
	case 32:
		break;
	default:
		nBmpColorWidth=8;
		break;
	}

	bInspectView = GetProfileInt(_T("InspectView"), FALSE);
	bAnalyzerView = GetProfileInt(_T("AnalyzerView"), FALSE);

	if(bInspectView && bStructView && bAnalyzerView)
	{
		bStructView=false;
		bAnalyzerView=false;
	}
}

void CBZOptions::Save()
{
	WriteProfileInt(_T("CharSet"), charset);
	WriteProfileInt(_T("AutoDetect"), bAutoDetect);
	WriteProfileInt(_T("ByteOrder"), bByteOrder);
	if(!sFontName.IsEmpty()) {
		WriteProfileString(_T("FontName"), sFontName);
		WriteProfileInt(_T("FontStyle"), fFontStyle);
		WriteProfileInt(_T("FontSize"), nFontSize);
	}
	if(theApp.m_bFirstInstance) {
		WriteProfileInt(_T("FrameLeft"), ptFrame.x);
		WriteProfileInt(_T("FrameTop"), ptFrame.y);
	}
	WriteProfileInt(_T("CmdShow"), nCmdShow);
	switch(nSplitView) {
	case 0:
		WriteProfileInt(_T("FrameHeight"), cyFrame);
		break;
	case ID_VIEW_SPLIT_H:
		WriteProfileInt(_T("FrameHeight2"), cyFrame2);
		WriteProfileInt(_T("SplitVPos"), ySplit);
		break;
	case ID_VIEW_SPLIT_V:
		WriteProfileInt(_T("FrameWidth2"), cxFrame2);
		WriteProfileInt(_T("SplitHPos"), xSplit);
		break;
	}
	WriteProfileInt(_T("StructView"), bStructView);
	if(bStructView || bInspectView)
		WriteProfileInt(_T("SplitStruct"), xSplitStruct);
	WriteProfileInt(_T("ComboHeight"), nComboHeight);
	WriteProfileInt(_T("Language"), bLanguage);
	WriteProfileInt(_T("DetectMax"), dwDetectMax);
	WriteProfileInt(_T("BarState"), barState);
	WriteProfileInt(_T("ReadOnly"), bReadOnlyOpen);
	WriteProfileInt(_T("BmpWidth"), nBmpWidth);
	WriteProfileInt(_T("BmpZoom"), nBmpZoom);
	WriteProfileInt(_T("MaxOnMemory"), dwMaxOnMemory);
	WriteProfileInt(_T("MaxMapSize"), dwMaxMapSize);
	WriteProfileInt(_T("TagAll"), bTagAll);
	WriteProfileInt(_T("SubCursor"), bSubCursor);

	WriteProfileBinary(_T("Colors"), (LPBYTE)colors, sizeof(colorsDefault));
	WriteProfileBinary(_T("MemberColumns"), (LPBYTE)colWidth, sizeof(colWidth));
	WriteProfileBinary(_T("PageMargin"), (LPBYTE)(LPRECT)rMargin, sizeof(rMargin));

	WriteProfileString(_T("DumpHeader"), sDumpHeader);
	WriteProfileInt(_T("DumpPage"), nDumpPage);
	WriteProfileInt(_T("DWordAddr"), bDWordAddr);
	
	WriteProfileInt(_T("SyncScroll"), bSyncScroll);
	WriteProfileInt(_T("Grid"), iGrid);
	WriteProfileInt(_T("BmpColorWidth"), nBmpColorWidth);

	WriteProfileInt(_T("InspectView"), bInspectView);
	WriteProfileInt(_T("AnalyzerView"), bAnalyzerView);
	
}

/////////////////////////////////////////////////////////////////////////////
// Save Dump List ###1.63

void CBZApp::OnFileSaveDumpList() 
{
	// TODO: Add your command handler code here
	CString sFileName = GetMainFrame()->GetActiveDocument()->GetPathName();
	sFileName += _T(".lst");

	if(m_pDocManager->DoPromptFileName(sFileName, IDS_SAVEDUMP_CAPTION, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, FALSE, NULL)) {
		CFile file;
		if(file.Open(sFileName, CFile::modeCreate | CFile::modeWrite)) {
			((CBZView*)(GetMainFrame()->GetActiveView()))->DrawToFile(&file);
			file.Close();
		}
	}
}

