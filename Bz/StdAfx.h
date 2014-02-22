// stdafx.h : include file for standard system include files,
//	or project specific include files that are used frequently, but
//		are changed infrequently
//
#define FILE_MAPPING

#define VC_EXTRALEAN		// Windows ヘッダーから使用されていない部分を除外します。
#define WINVER 0x0501 //XP
#define _WIN32_WINNT 0x0501 //XP
#define _WIN32_WINDOWS 0x0410 // これを Windows Me またはそれ以降のバージョン向けに適切な値に変更してください。
#define _WIN32_IE 0x0600	// これを IE の他のバージョン向けに適切な値に変更してください。

#define ISOLATION_AWARE_ENABLED 1

#include <windows.h>

#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlcoll.h>

//#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlapp.h>
#include <atldlgs.h>
#include <atlgdi.h>//CDCHandle
#include <atlctrls.h>//CComboBox
#include <atlctrlx.h>
#include <atlframe.h>//COwnerDraw
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlddx.h>
#include <atlsplit.h>
#include <atlfile.h>
#include <atlutil.h>
#include <atlscrl.h>
#include <atlprint.h>

#include <shlobj.h>

#include "afxres.h"
#include "resource.h"
#include "..\cmos.h"

#include <imagehlp.h>

//#include "MemDC.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define SFC_EASYDEBUG
#include "SuperFileCon.h"

#include "tamasplit.h"
#include "BZCoreData.h"

#include "BZOption.h"
extern CBZOptions options;

/*
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
*/
