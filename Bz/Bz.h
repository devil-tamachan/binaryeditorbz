// BZ.h : main header file for the BZ application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "options.h"

/////////////////////////////////////////////////////////////////////////////
// CBZApp:
// See BZ.cpp for the implementation of this class
//

class CBZDocTemplate : public CSingleDocTemplate
{
//	DECLARE_DYNAMIC(CBZDocTemplate)

// Constructors
public:
	CBZDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
public:
	void SetDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
};

class CBZDocManager : public CDocManager
{
public:
	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
};

class CBZApp : public CWinApp
{
public:
	CBZApp();

// Attributes
public:
	BOOL m_bFirstInstance;
	HINSTANCE m_hInstDll; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
private:
	void LoadProfile();
	void WriteProfile();
	void ShortNameToLongName(CString& sShortPath);

	//{{AFX_MSG(CBZApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFilePageSetup();
	afx_msg void OnToolsEditBZDef();
	afx_msg void OnFileSaveDumpList();
	//}}AFX_MSG
	afx_msg void OnLanguage(UINT nID);
	afx_msg void OnUpdateLanguage(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBZOptions

enum CharSet { CTYPE_ASCII, CTYPE_SJIS, CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY = -1 };
enum TextColor{ TCOLOR_ADDRESS, TCOLOR_ADDRESS2, TCOLOR_TEXT, TCOLOR_SELECT, TCOLOR_MARK, TCOLOR_MISMATCH, TCOLOR_STRUCT, TCOLOR_MEMBER, TCOLOR_OVERBROTHER, TCOLOR_COUNT };
enum MemberColumn { MBRCOL_OFFSET, MBRCOL_LABEL, MBRCOL_VALUE, MBRCOL_MAX };

#define SYSCOLOR 0x80000000
inline BOOL IsSystemColor(COLORREF rgb) { return (rgb & SYSCOLOR) != 0; }
inline COLORREF GetSystemColor(COLORREF rgb) { return (IsSystemColor(rgb)) ? (COLORREF)::GetSysColor(rgb & ~SYSCOLOR) : rgb; }

#define BARSTATE_TOOL 1
#define BARSTATE_STATUS 2
#define BARSTATE_FULLPATH 4
#define BARSTATE_NOFLAT 8

class CBZOptions : public COptions
{
public:
	void Load();
	void Save();

	CharSet charset;
	BOOL bAutoDetect;
	CString sFontName;
	int nFontSize;
	int fFontStyle;
	BOOL bByteOrder;
	CPoint ptFrame;
	int nCmdShow;
	int cyFrame;
	int cyFrame2;
	int cxFrame2;
	int xSplit;
	int ySplit;
	int xSplitStruct;
	BOOL bStructView;
	UINT nSplitView;
	int nComboHeight;
	COLORREF colors[TCOLOR_COUNT][2];
	int  colWidth[MBRCOL_MAX];
	BOOL bLanguage;
	CRect rMargin;
	DWORD dwDetectMax;
	DWORD barState;
	BOOL bReadOnlyOpen;
	int  nBmpWidth;
	int  nBmpZoom;
	DWORD dwMaxOnMemory;
	DWORD dwMaxMapSize;
	BOOL  bTagAll;
	BOOL  bSubCursor;

	CString sDumpHeader;
	int nDumpPage;
	BOOL  bDWordAddr;

	BOOL  bSyncScroll;

};

extern CBZOptions options;
CString SeparateByComma(int num, BOOL bSigned = FALSE);
CString SeparateByComma64(ULONGLONG num, BOOL bSigned = FALSE);
CString GetModulePath(LPCSTR pFileName);
LPVOID ReadFile(LPCSTR pPath);
void ErrorMessageBox();	// ###1.61

/////////////////////////////////////////////////////////////////////////////
