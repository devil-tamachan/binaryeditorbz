// BZ.cpp : Defines the class behaviors for the application.
//

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
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
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

	SetRegistryKey("c.mos");
/*#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif*/

	m_bFirstInstance = ::FindWindow(BZ_CLASSNAME, NULL) == NULL;

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
	options.Load();

	if(options.bLanguage && (m_hInstDll = ::LoadLibrary("BZres_us.dll"))) 
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
	OnFileNew();
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

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBZApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

CString SeparateByComma(int num, BOOL bSigned)
{
	CString sSrc;
	sSrc.Format(bSigned ? "%d" : "%u", num);
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
	sSrc.Format(bSigned ? "%I64d" : "%I64u", num);
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

CString GetModulePath(LPCSTR pFileName) 
{
	CString sPath;
	LPSTR pBuffer = sPath.GetBuffer(_MAX_PATH);
	::GetModuleFileName(NULL, pBuffer, _MAX_PATH - 1);
	char* p;
	p = strrchr(pBuffer, '\\');
	ASSERT(p);
	*(p+1) = '\0';
	sPath.ReleaseBuffer();
	sPath += pFileName;
	return sPath;
}


CString GetStructFilePath()
{
	CString sFileName, retStr;
	sFileName.LoadString(IDS_STRUCT_FILE);
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

LPVOID ReadFile(LPCSTR pPath)
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
	sErr.Format("(%d)", nErr);
	TRACE("Error=%d\n", nErr);
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
//	sPath.LoadString(IDS_STRUCT_FILE);
//	sPath = GetModulePath(sPath);
	sPath = GetStructFilePath();

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

	WCHAR wszShortPath[MAX_PATH];
	TCHAR szLongPath[MAX_PATH];
    ::MultiByteToWideChar(CP_ACP, 0, sPath, -1, wszShortPath, MAX_PATH);

	LPMALLOC pMalloc;
	LPSHELLFOLDER pSF;
	LPITEMIDLIST pIDL;
	if(::SHGetMalloc(&pMalloc) == NOERROR) {
		if (::SHGetDesktopFolder(&pSF) == NOERROR) {
			if(pSF->ParseDisplayName(NULL, NULL, wszShortPath, NULL, &pIDL, NULL) == NOERROR) {
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
	charset = (CharSet)GetProfileInt("CharSet", CTYPE_ASCII);
	bAutoDetect = GetProfileInt("AutoDetect", FALSE);
	bByteOrder= GetProfileInt("ByteOrder", FALSE);
	sFontName = GetProfileString("FontName", _T("FixedSys"));	// ###1.54
	fFontStyle= GetProfileInt("FontStyle", 0);
	nFontSize = GetProfileInt("FontSize", 140);
	ptFrame.x = GetProfileInt("FrameLeft", 0);
	ptFrame.y = GetProfileInt("FrameTop", 0);
	nCmdShow  = GetProfileInt("CmdShow", SW_SHOWNORMAL);
	cyFrame   = GetProfileInt("FrameHeight", 0);
	cyFrame2  = GetProfileInt("FrameHeight2", 0);
	cxFrame2  = GetProfileInt("FrameWidth2", 0);
	xSplit    = GetProfileInt("SplitHPos", 0);
	ySplit    = GetProfileInt("SplitVPos", 0);
	xSplitStruct = 	GetProfileInt("SplitStruct", 0);
	bStructView = GetProfileInt("StructView", FALSE);
	nComboHeight = GetProfileInt("ComboHeight", 15);
	bLanguage = GetProfileInt("Language", ::GetThreadLocale() != 0x411);
	dwDetectMax = GetProfileInt("DetectMax", 0x10000);
	barState = GetProfileInt("BarState", BARSTATE_TOOL | BARSTATE_STATUS);
	bReadOnlyOpen = GetProfileInt("ReadOnly", TRUE);
	nBmpWidth = GetProfileInt("BmpWidth", 128);
	nBmpZoom =  GetProfileInt("BmpZoom", 1);
	dwMaxOnMemory = GetProfileInt("MaxOnMemory", 1024 * 1024);		// ###1.60
	dwMaxMapSize =  GetProfileInt("MaxMapSize", 1024 * 1024 * 64);	// ###1.61
	bTagAll =  GetProfileInt("TagAll", FALSE);
	bSubCursor =  GetProfileInt("SubCursor", TRUE);

	memcpy(colors, colorsDefault, sizeof(colorsDefault));
	GetProfileBinary2("Colors", colors, sizeof colors);
//	if(!GetProfileBinary("Colors", colors))
//		memcpy(colors, colorsDefault, sizeof(colorsDefault));
	if(!GetProfileBinary("MemberColumns", colWidth))
		memcpy(colWidth, colWidthDefault, sizeof(colWidthDefault));
	if(!GetProfileBinary("PageMargin", (LPRECT)rMargin))
		rMargin.SetRect(2000, 2000, 2000, 2000);

	sDumpHeader = GetProfileString("DumpHeader");	// ###1.63
	nDumpPage = GetProfileInt("DumpPage", 0);
	bDWordAddr = GetProfileInt("DWordAddr", FALSE);
	
	bSyncScroll = GetProfileInt("SyncScroll", true);
	iGrid = GetProfileInt("Grid", 0);
	nBmpColorWidth = GetProfileInt("BmpColorWidth", 8);

	bInspectView = GetProfileInt("InspectView", FALSE);
}

void CBZOptions::Save()
{
	WriteProfileInt("CharSet", charset);
	WriteProfileInt("AutoDetect", bAutoDetect);
	WriteProfileInt("ByteOrder", bByteOrder);
	if(!sFontName.IsEmpty()) {
		WriteProfileString("FontName", sFontName);
		WriteProfileInt("FontStyle", fFontStyle);
		WriteProfileInt("FontSize", nFontSize);
	}
	if(theApp.m_bFirstInstance) {
		WriteProfileInt("FrameLeft", ptFrame.x);
		WriteProfileInt("FrameTop", ptFrame.y);
	}
	WriteProfileInt("CmdShow", nCmdShow);
	switch(nSplitView) {
	case 0:
		WriteProfileInt("FrameHeight", cyFrame);
		break;
	case ID_VIEW_SPLIT_H:
		WriteProfileInt("FrameHeight2", cyFrame2);
		WriteProfileInt("SplitVPos", ySplit);
		break;
	case ID_VIEW_SPLIT_V:
		WriteProfileInt("FrameWidth2", cxFrame2);
		WriteProfileInt("SplitHPos", xSplit);
		break;
	}
	WriteProfileInt("StructView", bStructView);
	if(bStructView || bInspectView)
		WriteProfileInt("SplitStruct", xSplitStruct);
	WriteProfileInt("ComboHeight", nComboHeight);
	WriteProfileInt("Language", bLanguage);
	WriteProfileInt("DetectMax", dwDetectMax);
	WriteProfileInt("BarState", barState);
	WriteProfileInt("ReadOnly", bReadOnlyOpen);
	WriteProfileInt("BmpWidth", nBmpWidth);
	WriteProfileInt("BmpZoom", nBmpZoom);
	WriteProfileInt("MaxOnMemory", dwMaxOnMemory);
	WriteProfileInt("MaxMapSize", dwMaxMapSize);
	WriteProfileInt("TagAll", bTagAll);
	WriteProfileInt("SubCursor", bSubCursor);

	WriteProfileBinary("Colors", (LPBYTE)colors, sizeof(colorsDefault));
	WriteProfileBinary("MemberColumns", (LPBYTE)colWidth, sizeof(colWidth));
	WriteProfileBinary("PageMargin", (LPBYTE)(LPRECT)rMargin, sizeof(rMargin));

	WriteProfileString("DumpHeader", sDumpHeader);
	WriteProfileInt("DumpPage", nDumpPage);
	WriteProfileInt("DWordAddr", bDWordAddr);
	
	WriteProfileInt("SyncScroll", bSyncScroll);
	WriteProfileInt("Grid", iGrid);
	WriteProfileInt("BmpColorWidth", nBmpColorWidth);

	WriteProfileInt("InspectView", bInspectView);
}

/////////////////////////////////////////////////////////////////////////////
// Save Dump List ###1.63

void CBZApp::OnFileSaveDumpList() 
{
	// TODO: Add your command handler code here
	CString sFileName = GetMainFrame()->GetActiveDocument()->GetPathName();
	sFileName += _T((".lst"));

	if(m_pDocManager->DoPromptFileName(sFileName, IDS_SAVEDUMP_CAPTION, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, FALSE, NULL)) {
		CFile file;
		if(file.Open(sFileName, CFile::modeCreate | CFile::modeWrite)) {
			((CBZView*)(GetMainFrame()->GetActiveView()))->DrawToFile(&file);
			file.Close();
		}
	}
}

