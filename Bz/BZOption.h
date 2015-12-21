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

#define SYSCOLOR 0x80000000
inline BOOL IsSystemColor(COLORREF rgb) { return (rgb & SYSCOLOR) != 0; }
inline COLORREF GetSystemColor(COLORREF rgb) { return (IsSystemColor(rgb)) ? (COLORREF)::GetSysColor(rgb & ~SYSCOLOR) : rgb; }

enum CharSet { CTYPE_ASCII = 0, CTYPE_SJIS, CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY = -1 };
enum TextColor{ TCOLOR_ADDRESS, TCOLOR_ADDRESS2, TCOLOR_TEXT, TCOLOR_SELECT, TCOLOR_MARK, TCOLOR_MISMATCH, TCOLOR_STRUCT, TCOLOR_MEMBER, TCOLOR_OVERBROTHER, TCOLOR_HORIZON, TCOLOR_ADDRESS3, TCOLOR_COUNT };
//enum MemberColumn { MBRCOL_OFFSET, MBRCOL_LABEL, MBRCOL_VALUE, MBRCOL_MAX };
enum MemberColumn2 { MBRCOL2_LABEL, MBRCOL2_VALUE, MBRCOL2_MAX };

#define BARSTATE_TOOL 1
#define BARSTATE_STATUS 2
#define BARSTATE_FULLPATH 4
#define BARSTATE_NOFLAT 8

extern COLORREF colorsDefault[TCOLOR_COUNT][2];
//extern int colWidthDefault[MBRCOL_MAX];
extern int colWidth2Default[MBRCOL2_MAX];
extern const TCHAR sRegDefault[];
extern const TCHAR sRegHistory[];

extern BOOL g_bFirstInstance;

class CBZOptions
{
public:
  void ResetSettings()
  {
    CRegKey key(HKEY_CURRENT_USER);
    key.RecurseDeleteKey(_T("Software\\c.mos\\BZ\\Settings"));
    key.Flush();
    key.Close();
    Load();
  }

  void Load()
  {
    LoadDefault();
    if(FAILED(LoadFromFile()))LoadFromRegistry();
  }

  HRESULT LoadFromFile();

  void LoadFromRegistry()
  {
    CRegKey key;
    LONG nRet = key.Create(HKEY_CURRENT_USER, _T("Software\\c.mos\\BZ\\Settings"));
    if(nRet!=ERROR_SUCCESS)return;

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
    charset = (CharSet)GetProfileInt(key, _T("CharSet"), CTYPE_ASCII);
    bAutoDetect = GetProfileInt(key, _T("AutoDetect"), FALSE);
    bByteOrder= GetProfileInt(key, _T("ByteOrder"), FALSE);
    sFontName = GetProfileString(key, _T("FontName"), _T("FixedSys"));	// ###1.54
    fFontStyle= GetProfileInt(key, _T("FontStyle"), 0);
    nFontSize = GetProfileInt(key, _T("FontSize"), 140);
    ptFrame.x = GetProfileInt(key, _T("FrameLeft"), 0);
    ptFrame.y = GetProfileInt(key, _T("FrameTop"), 0);
    nCmdShow  = GetProfileInt(key, _T("CmdShow"), SW_SHOWNORMAL);
    cyFrame   = GetProfileInt(key, _T("FrameHeight"), 0);
    cyFrame2  = GetProfileInt(key, _T("FrameHeight2"), 0);
    cxFrame2  = GetProfileInt(key, _T("FrameWidth2"), 0);
    xSplit    = GetProfileInt(key, _T("SplitHPos"), 0);
    ySplit    = GetProfileInt(key, _T("SplitVPos"), 0);
    xSplitStruct = 	GetProfileInt(key, _T("SplitStruct"), 0);
    bStructView = GetProfileInt(key, _T("StructView"), FALSE);
    nComboHeight = GetProfileInt(key, _T("ComboHeight"), 15);
    bLanguage = GetProfileInt(key, _T("Language"), ::GetThreadLocale() != 0x411);
    dwDetectMax = GetProfileInt(key, _T("DetectMax"), 0x10000);
    barState = GetProfileInt(key, _T("BarState"), BARSTATE_TOOL | BARSTATE_STATUS);
    bReadOnlyOpen = GetProfileInt(key, _T("ReadOnly"), TRUE);
    nBmpWidth = GetProfileInt(key, _T("BmpWidth"), 128);
    nBmpZoom =  GetProfileInt(key, _T("BmpZoom"), 1);
    nBmpPallet =  GetProfileInt(key, _T("BmpPallet"), 1);
    dwMaxOnMemory = GetProfileInt(key, _T("MaxOnMemory"), 1024 * 1024);		// ###1.60
    dwMaxMapSize =  GetProfileInt(key, _T("MaxMapSize"), 1024 * 1024 * 64);	// ###1.61
    bTagAll =  GetProfileInt(key, _T("TagAll"), FALSE);
    bSubCursor =  GetProfileInt(key, _T("SubCursor"), TRUE);

    memcpy(colors, colorsDefault, sizeof(colorsDefault));
    GetProfileBinary2(key, _T("Colors"), colors, sizeof colors);
    //	if(!GetProfileBinary("Colors", colors))
    //		memcpy(colors, colorsDefault, sizeof(colorsDefault));
 //   if(!GetProfileBinary2(key, _T("MemberColumns"), colWidth, sizeof(colWidth)))
  //    memcpy(colWidth, colWidthDefault, sizeof(colWidthDefault));
    if(!GetProfileBinary2(key, _T("MemberColumns2"), colWidth2, sizeof(colWidth2)))
      memcpy(colWidth2, colWidth2Default, sizeof(colWidth2Default));
    if(!GetProfileBinary2(key, _T("PageMargin"), (LPRECT)rMargin, sizeof(RECT)))
      rMargin.SetRect(2000, 2000, 2000, 2000);

    sDumpHeader = GetProfileString(key, _T("DumpHeader"));	// ###1.63
    nDumpPage = GetProfileInt(key, _T("DumpPage"), 0);
    bQWordAddr = GetProfileInt(key, _T("QWordAddr"), FALSE);
    bClearUndoRedoWhenSave = GetProfileInt(key, _T("ClearUndoRedoWhenSave"), TRUE);

    bSyncScroll = GetProfileInt(key, _T("SyncScroll"), true);
    iGrid = GetProfileInt(key, _T("Grid"), 0);
    nBmpColorWidth = GetProfileInt(key, _T("BmpColorWidth"), 8);

    bInspectView = GetProfileInt(key, _T("InspectView"), FALSE);
    bAnalyzerView = GetProfileInt(key, _T("AnalyzerView"), FALSE);

    bAddressTooltip = GetProfileInt(key, _T("BmpAddressTooltip"), TRUE);

    bMiniToolbar = GetProfileInt(key, _T("MiniToolbar"), TRUE);

    CheckOptions();
   // key.Flush();
  }

  void LoadDefault()
  {
    charset = CTYPE_ASCII;
    bAutoDetect = FALSE;
    bByteOrder= FALSE;
    sFontName = _T("FixedSys");	// ###1.54
    fFontStyle= 0;
    nFontSize = 140;
    ptFrame.x = 0;
    ptFrame.y = 0;
    nCmdShow  = SW_SHOWNORMAL;
    cyFrame   = 0;
    cyFrame2  = 0;
    cxFrame2  = 0;
    xSplit    = 0;
    ySplit    = 0;
    xSplitStruct = 0;
    bStructView = FALSE;
    nComboHeight = 15;
    bLanguage = ::GetThreadLocale() != 0x411;
    dwDetectMax = 0x10000;
    barState = BARSTATE_TOOL | BARSTATE_STATUS;
    bReadOnlyOpen = TRUE;
    nBmpWidth = 128;
    nBmpZoom =  1;
    nBmpPallet = 1;
    dwMaxOnMemory = 1024 * 1024;		// ###1.60
    dwMaxMapSize =  1024 * 1024 * 64;	// ###1.61
    bTagAll =  FALSE;
    bSubCursor =  TRUE;
    
    memcpy(colors, colorsDefault, sizeof(colorsDefault));
    memcpy(colWidth2, colWidth2Default, sizeof(colWidth2Default));
    rMargin.SetRect(2000, 2000, 2000, 2000);

    sDumpHeader = _T("DumpHeader");	// ###1.63
    nDumpPage = 0;
    bQWordAddr = FALSE;
    bClearUndoRedoWhenSave = TRUE;

    bSyncScroll = true;
    iGrid = 0;
    nBmpColorWidth = 8;

    bInspectView = FALSE;
    bAnalyzerView = FALSE;

    bAddressTooltip = TRUE;

    bMiniToolbar = TRUE;
  }

  void CheckOptions()
  {
    if(nBmpPallet!=0 && nBmpPallet!=1)nBmpPallet = 1;
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

    if(bInspectView && bStructView && bAnalyzerView)
    {
      bStructView=false;
      bAnalyzerView=false;
    }
  }

  HRESULT WriteFileInt(CAtlFile &file, LPCSTR name, int val)
  {
    CStringA str;
    str.Format("%s = %ld\n", name, val);
    return file.Write(((LPVOID)(LPCSTR)(str)), str.GetLength());
  }

  HRESULT WriteFileString(CAtlFile &file, LPCSTR name, CString& val)
  {
    CString t = val;
    t.Replace(_T("\""), _T("\\\""));
    t.Replace(_T("\r"), _T("\\r"));
    t.Replace(_T("\n"), _T("\\n"));
    CStringA str;
    str.Format("%s = \"%s\"\n", name, CT2A(val, CP_UTF8));
    return file.Write(((LPVOID)(LPCSTR)(str)), str.GetLength());
  }

  HRESULT WriteFileRect(CAtlFile &file, LPCSTR name, LPRECT val)
  {
    CStringA str;
    str.Format("%s = {%d, %d, %d, %d}\n", name, val->left, val->top, val->right, val->bottom);
    return file.Write(((LPVOID)(LPCSTR)(str)), str.GetLength());
  }

  HRESULT WriteFileIntArray(CAtlFile &file, LPCSTR name, int *pInt, int count)
  {
    CStringA str, t;
    str.Format("%s = {", name);
    if(count > 0)
    {
      t.Format("%d", pInt[0]);
      str += t;
      for(int i=1; i<count; i++)
      {
        t.Format(", %d", pInt[i]);
        str += t;
      }
    }
    str += "}\n";
    return file.Write(((LPVOID)(LPCSTR)(str)), str.GetLength());
  }

  HRESULT WriteFileColorArray(CAtlFile &file, LPCSTR name, COLORREF *pCol, int count)
  {
    CStringA str, t;
    str.Format("%s = {", name);
    if(count > 0)
    {
      t.Format("#%08X", pCol[0]);
      str += t;
      for(int i=1; i<count; i++)
      {
        t.Format(", #%08X", pCol[i]);
        str += t;
      }
    }
    str += "}\n";
    return file.Write(((LPVOID)(LPCSTR)(str)), str.GetLength());
  }

  void Save()
  {
    if(bPortableMode)SaveToFile();
    else SaveToRegistry();
  }

  HRESULT SaveToFile()
  {
    CString path;
    path = GetModulePath(_T("EnablePortableMode.txt"));
    CAtlFile file;
    if(FAILED(file.Create(path, GENERIC_WRITE, 0, CREATE_ALWAYS)))return E_FAIL;

    CStringA t = CW2A(_T("/* このファイルがあるとポータブルモードになります。設定はこのファイルへ書きだされます。レジストリには書き込まれません。 */\n\n"), CP_UTF8);
    file.Write( ((LPCVOID)(LPCSTR)t), t.GetLength());
    
    WriteFileInt(file, "CharSet", charset);
    WriteFileInt(file, "AutoDetect", bAutoDetect);
    WriteFileInt(file, "ByteOrder", bByteOrder);
    if(!sFontName.IsEmpty()) {
      WriteFileString(file, "FontName", sFontName);
      WriteFileInt(file, "FontStyle", fFontStyle);
      WriteFileInt(file, "FontSize", nFontSize);
    }
    if(g_bFirstInstance) {
      WriteFileInt(file, "FrameLeft", ptFrame.x);
      WriteFileInt(file, "FrameTop", ptFrame.y);
    }
    WriteFileInt(file, "CmdShow", nCmdShow);
    switch(nSplitView)
    {
    case 0:
      WriteFileInt(file, "FrameHeight", cyFrame);
      break;
    case ID_VIEW_SPLIT_H:
      WriteFileInt(file, "FrameHeight", cyFrame);
      WriteFileInt(file, "FrameHeight2", cyFrame2);
      WriteFileInt(file, "SplitVPos", ySplit);
      break;
    case ID_VIEW_SPLIT_V:
      WriteFileInt(file, "FrameHeight", cyFrame);
      WriteFileInt(file, "FrameWidth2", cxFrame2);
      WriteFileInt(file, "SplitHPos", xSplit);
      break;
    }
    WriteFileInt(file, "StructView", bStructView);
    if(bStructView || bInspectView)
      WriteFileInt(file, "SplitStruct", xSplitStruct);
    WriteFileInt(file, "ComboHeight", nComboHeight);
    WriteFileInt(file, "Language", bLanguage);
    WriteFileInt(file, "DetectMax", dwDetectMax);
    WriteFileInt(file, "BarState", barState);
    WriteFileInt(file, "ReadOnly", bReadOnlyOpen);
    WriteFileInt(file, "BmpWidth", nBmpWidth);
    WriteFileInt(file, "BmpZoom", nBmpZoom);
    WriteFileInt(file, "BmpPallet", nBmpPallet);
    WriteFileInt(file, "MaxOnMemory", dwMaxOnMemory);
    WriteFileInt(file, "MaxMapSize", dwMaxMapSize);
    WriteFileInt(file, "TagAll", bTagAll);
    WriteFileInt(file, "SubCursor", bSubCursor);

    WriteFileColorArray(file, "Colors", &colors[0][0], TCOLOR_COUNT*2);
 //   WriteProfileBinary(key, _T("MemberColumns"), (LPBYTE)colWidth, sizeof(colWidth));
    WriteFileIntArray(file, "MemberColumns2", colWidth2, MBRCOL2_MAX);
    WriteFileRect(file, "PageMargin", rMargin);

    WriteFileString(file, "DumpHeader", sDumpHeader);
    WriteFileInt(file, "DumpPage", nDumpPage);
    WriteFileInt(file, "QWordAddr", bQWordAddr);
    WriteFileInt(file, "ClearUndoRedoWhenSave", bClearUndoRedoWhenSave);

    WriteFileInt(file, "SyncScroll", bSyncScroll);
    WriteFileInt(file, "Grid", iGrid);
    WriteFileInt(file, "BmpColorWidth", nBmpColorWidth);

    WriteFileInt(file, "InspectView", bInspectView);
    WriteFileInt(file, "AnalyzerView", bAnalyzerView);

    WriteFileInt(file, "BmpAddressTooltip", bAddressTooltip);

    WriteFileInt(file, "MiniToolbar", bMiniToolbar);


    file.Flush();
    file.Close();
    return S_OK;
  }

  void SaveToRegistry()
  {
    CRegKey key;
    LONG nRet = key.Create(HKEY_CURRENT_USER, _T("Software\\c.mos\\BZ\\Settings"));
    if(nRet!=ERROR_SUCCESS)return;

    WriteProfileInt(key, _T("CharSet"), charset);
    WriteProfileInt(key, _T("AutoDetect"), bAutoDetect);
    WriteProfileInt(key, _T("ByteOrder"), bByteOrder);
    if(!sFontName.IsEmpty()) {
      WriteProfileString(key, _T("FontName"), sFontName);
      WriteProfileInt(key, _T("FontStyle"), fFontStyle);
      WriteProfileInt(key, _T("FontSize"), nFontSize);
    }
    if(g_bFirstInstance) {
      WriteProfileInt(key, _T("FrameLeft"), ptFrame.x);
      WriteProfileInt(key, _T("FrameTop"), ptFrame.y);
    }
    WriteProfileInt(key, _T("CmdShow"), nCmdShow);
    switch(nSplitView)
    {
    case 0:
      WriteProfileInt(key, _T("FrameHeight"), cyFrame);
      break;
    case ID_VIEW_SPLIT_H:
      WriteProfileInt(key, _T("FrameHeight2"), cyFrame2);
      WriteProfileInt(key, _T("SplitVPos"), ySplit);
      break;
    case ID_VIEW_SPLIT_V:
      WriteProfileInt(key, _T("FrameWidth2"), cxFrame2);
      WriteProfileInt(key, _T("SplitHPos"), xSplit);
      break;
    }
    WriteProfileInt(key, _T("StructView"), bStructView);
    if(bStructView || bInspectView)
      WriteProfileInt(key, _T("SplitStruct"), xSplitStruct);
    WriteProfileInt(key, _T("ComboHeight"), nComboHeight);
    WriteProfileInt(key, _T("Language"), bLanguage);
    WriteProfileInt(key, _T("DetectMax"), dwDetectMax);
    WriteProfileInt(key, _T("BarState"), barState);
    WriteProfileInt(key, _T("ReadOnly"), bReadOnlyOpen);
    WriteProfileInt(key, _T("BmpWidth"), nBmpWidth);
    WriteProfileInt(key, _T("BmpZoom"), nBmpZoom);
    WriteProfileInt(key, _T("BmpPallet"), nBmpPallet);
    WriteProfileInt(key, _T("MaxOnMemory"), dwMaxOnMemory);
    WriteProfileInt(key, _T("MaxMapSize"), dwMaxMapSize);
    WriteProfileInt(key, _T("TagAll"), bTagAll);
    WriteProfileInt(key, _T("SubCursor"), bSubCursor);

    WriteProfileBinary(key, _T("Colors"), (LPBYTE)colors, sizeof(colorsDefault));
 //   WriteProfileBinary(key, _T("MemberColumns"), (LPBYTE)colWidth, sizeof(colWidth));
    WriteProfileBinary(key, _T("MemberColumns2"), (LPBYTE)colWidth2, sizeof(colWidth2));
    WriteProfileBinary(key, _T("PageMargin"), (LPBYTE)(LPRECT)rMargin, sizeof(rMargin));

    WriteProfileString(key, _T("DumpHeader"), sDumpHeader);
    WriteProfileInt(key, _T("DumpPage"), nDumpPage);
    WriteProfileInt(key, _T("QWordAddr"), bQWordAddr);
    WriteProfileInt(key, _T("ClearUndoRedoWhenSave"), bClearUndoRedoWhenSave);

    WriteProfileInt(key, _T("SyncScroll"), bSyncScroll);
    WriteProfileInt(key, _T("Grid"), iGrid);
    WriteProfileInt(key, _T("BmpColorWidth"), nBmpColorWidth);

    WriteProfileInt(key, _T("InspectView"), bInspectView);
    WriteProfileInt(key, _T("AnalyzerView"), bAnalyzerView);

    WriteProfileInt(key, _T("BmpAddressTooltip"), bAddressTooltip);

    WriteProfileInt(key, _T("MiniToolbar"), bMiniToolbar);

    key.Flush();
  }

	CharSet charset;
	BOOL bAutoDetect;
	CString sFontName;
	int nFontSize;
	int fFontStyle;
	BOOL bByteOrder;
  WTL::CPoint ptFrame;
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
//	int  colWidth[MBRCOL_MAX];
	int  colWidth2[MBRCOL2_MAX];
	BOOL bLanguage;
  WTL::CRect rMargin;
	DWORD dwDetectMax;
	DWORD barState;
	BOOL bReadOnlyOpen;
	int  nBmpWidth;
	int  nBmpZoom;
  int  nBmpPallet;
	DWORD dwMaxOnMemory;
	DWORD dwMaxMapSize;
	BOOL  bTagAll;
	BOOL  bSubCursor;

	CString sDumpHeader;
	int nDumpPage;
	BOOL  bQWordAddr;
  BOOL  bClearUndoRedoWhenSave;

	BOOL  bSyncScroll;
	int  iGrid;
	int  nBmpColorWidth;

	BOOL bInspectView;
	BOOL bAnalyzerView;

	BOOL bAddressTooltip;

  BOOL bPortableMode;

  BOOL bMiniToolbar;

public:
  CBZOptions() : m_bModified(FALSE), bPortableMode(FALSE) { }

public:
  void Touch() { m_bModified = TRUE; }
  int GetProfileInt(CRegKey &key, LPCTSTR lpszEntry, int nDefault)
  {
    DWORD retVal;
    if(key.QueryDWORDValue(lpszEntry, retVal)==ERROR_SUCCESS)return retVal;
    else return nDefault;
  }
  BOOL WriteProfileInt(CRegKey &key, LPCTSTR lpszEntry, int nValue)
  {
    return key.SetDWORDValue(lpszEntry, (DWORD)nValue)==ERROR_SUCCESS;
  }
  CString GetProfileString(CRegKey &key, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL)
  {
    CString strRet;
    ULONG nLen=0;
    if(key.QueryStringValue(lpszEntry, NULL, &nLen)==ERROR_SUCCESS)
    {
      LONG nResult = key.QueryStringValue(lpszEntry, strRet.GetBuffer(nLen), &nLen);
      strRet.ReleaseBuffer();
      if(nResult==ERROR_SUCCESS)return strRet;
    }
    return lpszDefault;
  }
  BOOL WriteProfileString(CRegKey &key, LPCTSTR lpszEntry, LPCTSTR lpszValue)
  {
    return key.SetStringValue(lpszEntry, lpszValue)==ERROR_SUCCESS;
  }
  BOOL GetProfileBinary2(CRegKey &key, LPCTSTR lpszEntry, LPVOID pData, ULONG nBytesMax)
  {
    ULONG nBytes;
    if(key.QueryBinaryValue(lpszEntry, NULL, &nBytes)==ERROR_SUCCESS && nBytes>0)
    {
      LPBYTE pBuffer = (LPBYTE)malloc(nBytes);
      if(key.QueryBinaryValue(lpszEntry, pBuffer, &nBytes)==ERROR_SUCCESS)
      {
        memcpy(pData, pBuffer, min(nBytes, nBytesMax));
        free(pBuffer);
        return TRUE;
      }
      free(pBuffer);
    }
    return FALSE;
  }
  BOOL WriteProfileBinary(CRegKey &key, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
  {
    return key.SetBinaryValue(lpszEntry, pData, nBytes)==ERROR_SUCCESS;
  }
private:
	BOOL m_bModified;
};

extern CBZOptions options;
