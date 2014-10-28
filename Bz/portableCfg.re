
#include "stdafx.h"

typedef unsigned char uchar;
typedef unsigned int uint;

#define	YYCTYPE		uchar
#define	YYCURSOR	cursor
#define	YYLIMIT		s->lim
#define	YYMARKER	s->ptr

#define	RET(i)	{s->cur = cursor; return i;}
#define	RETT(i, typeid)	{s->cur = cursor; s->type = typeid; return i;}

typedef struct Scanner
{
    int			fd;
    uchar		*bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof;
    uint		line;
    union {
      int i;
      CString *str;
      COLORREF col;
    } val;
    uint		type;
} Scanner;

enum {TYPE_CharSet=0, TYPE_AutoDetect, TYPE_ByteOrder, TYPE_FontName, TYPE_FontStyle, TYPE_FontSize, TYPE_FrameLeft, TYPE_FrameTop, TYPE_CmdShow, TYPE_FrameHeight, TYPE_FrameHeight2, TYPE_FrameWidth2, TYPE_SplitHPos, TYPE_SplitVPos, TYPE_SplitStruct, TYPE_StructView, TYPE_ComboHeight, TYPE_Language, TYPE_DetectMax, TYPE_BarState, TYPE_ReadOnly, TYPE_BmpWidth, TYPE_BmpZoom, TYPE_BmpPallet, TYPE_MaxOnMemory, TYPE_MaxMapSize, TYPE_TagAll, TYPE_SubCursor, TYPE_Colors, TYPE_MemberColumns2, TYPE_PageMargin, TYPE_DumpHeader, TYPE_DumpPage, TYPE_QWordAddr, TYPE_ClearUndoRedoWhenSave, TYPE_SyncScroll, TYPE_Grid, TYPE_BmpColorWidth, TYPE_InspectView, TYPE_AnalyzerView, TYPE_BmpAddressTooltip};

void SetIntOption(uint type, int val)
{
  switch(type)
  {
case TYPE_CharSet: options.charset = (CharSet)val; break;
case TYPE_AutoDetect: options.bAutoDetect = val; break;
case TYPE_ByteOrder: options.bByteOrder = val; break;
case TYPE_FontStyle: options.fFontStyle = val; break;
case TYPE_FontSize: options.nFontSize = val; break;
case TYPE_FrameLeft: options.ptFrame.x = val; break;
case TYPE_FrameTop: options.ptFrame.y = val; break;
case TYPE_CmdShow: options.nCmdShow = val; break;
case TYPE_FrameHeight: options.cyFrame = val; break;
case TYPE_FrameHeight2: options.cyFrame2 = val; break;
case TYPE_FrameWidth2: options.cxFrame2 = val; break;
case TYPE_SplitHPos: options.xSplit = val; break;
case TYPE_SplitVPos: options.ySplit = val; break;
case TYPE_SplitStruct: options.xSplitStruct = val; break;
case TYPE_StructView: options.bStructView = val; break;
case TYPE_ComboHeight: options.nComboHeight = val; break;
case TYPE_Language: options.bLanguage = val; break;
case TYPE_DetectMax: options.dwDetectMax = val; break;
case TYPE_BarState: options.barState = val; break;
case TYPE_ReadOnly: options.bReadOnlyOpen = val; break;
case TYPE_BmpWidth: options.nBmpWidth = val; break;
case TYPE_BmpZoom: options.nBmpZoom = val; break;
case TYPE_BmpPallet: options.nBmpPallet = val; break;
case TYPE_MaxOnMemory: options.dwMaxOnMemory = val; break;
case TYPE_MaxMapSize: options.dwMaxMapSize = val; break;
case TYPE_TagAll: options.bTagAll = val; break;
case TYPE_SubCursor: options.bSubCursor = val; break;
case TYPE_DumpPage: options.nDumpPage = val; break;
case TYPE_QWordAddr: options.bQWordAddr = val; break;
case TYPE_ClearUndoRedoWhenSave: options.bClearUndoRedoWhenSave = val; break;
case TYPE_SyncScroll: options.bSyncScroll = val; break;
case TYPE_Grid: options.iGrid = val; break;
case TYPE_BmpColorWidth: options.nBmpColorWidth = val; break;
case TYPE_InspectView: options.bInspectView = val; break;
case TYPE_AnalyzerView: options.bAnalyzerView = val; break;
case TYPE_BmpAddressTooltip: options.bAddressTooltip = val; break;
	}
}

void SetStrOption(uint type, CString *pVal)
{
  switch(type)
  {
case TYPE_FontName: options.sFontName = *pVal; break;
case TYPE_DumpHeader: options.sDumpHeader = *pVal; break;
	}
}

void SetColorArrOption(uint type, CAtlArray<COLORREF> *pVal)
{
  switch(type)
  {
		case TYPE_Colors:
		{
			COLORREF *col = &(options.colors[0][0]);
			for(int i=0;i < TCOLOR_COUNT*2 && i < pVal->GetCount();i++)
			{
			  col[i] = (*pVal)[i];
			}
			break;
		}
	}
}

void SetIntArrOption(uint type, CAtlArray<int> *pVal)
{
  switch(type)
  {
		case TYPE_MemberColumns2:
		{
			for(int i=0;i < MBRCOL2_MAX && i < pVal->GetCount();i++)
			{
			  options.colWidth2[i] = (*pVal)[i];
			}
			break;
		}
	}
}

void SetRectOption(uint type, int *pVal)
{
  switch(type)
  {
case TYPE_PageMargin: options.rMargin.left = pVal[0]; options.rMargin.top = pVal[1]; options.rMargin.right = pVal[2]; options.rMargin.bottom = pVal[3]; break;
	}
}

#include "PortableCfgParser.h"
#include "PortableCfgParser.c"


COLORREF ReadRGB(uchar *token)
{
  COLORREF c=0xFF000000;
  _snscanf((const char *)token, 6, "%06x", &c);
  return c;
}

COLORREF ReadRGBA(uchar *token)
{
  COLORREF c=0;
  _snscanf((const char *)token, 8, "%08x", &c);
  return c;
}

CString ReadStr(uchar *token, uint len)
{
  CStringA tmpA;
  tmpA.SetString((const char *)token, len);
  CStringW tmpB = CA2W(tmpA, CP_UTF8);
  tmpB.Replace(L"%", L"%%");
  CString retStr;
  retStr.Format(tmpB);
  return retStr;
}

BOOL fill(int n) { return FALSE; }

int scan(Scanner *s){
	uchar *cursor = s->cur;
std:
	s->tok = cursor;
/*!re2c

re2c:define:YYFILL:naked = 1;
re2c:define:YYFILL@len = #;
re2c:define:YYFILL = "if(!fill(#)) { return 0; }";
re2c:yyfill:enable = 1;

any	= [\000-\xFF];
O	= [0-7];
D	= [0-9];
L	= [a-zA-Z_];
H	= [a-fA-F0-9];
ESC	= [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
RGB     = [#] (H {6});
RGBA    = [#] (H {8});
*/

/*!re2c
	"/*"			{ goto comment; }
	"//"			{ goto comment2; }
	
	"CharSet"			{ RETT(INTTYPE, TYPE_CharSet); }
	"AutoDetect"			{ RETT(INTTYPE, TYPE_AutoDetect); }
	"ByteOrder"			{ RETT(INTTYPE, TYPE_ByteOrder); }
	"FontName"			{ RETT(STRTYPE, TYPE_FontName); }
	"FontStyle"			{ RETT(INTTYPE, TYPE_FontStyle); }
	"FontSize"			{ RETT(INTTYPE, TYPE_FontSize); }
	"FrameLeft"			{ RETT(INTTYPE, TYPE_FrameLeft); }
	"FrameTop"			{ RETT(INTTYPE, TYPE_FrameTop); }
	"CmdShow"			{ RETT(INTTYPE, TYPE_CmdShow); }
	"FrameHeight"			{ RETT(INTTYPE, TYPE_FrameHeight); }
	"FrameHeight2"			{ RETT(INTTYPE, TYPE_FrameHeight2); }
	"FrameWidth2"			{ RETT(INTTYPE, TYPE_FrameWidth2); }
	"SplitHPos"			{ RETT(INTTYPE, TYPE_SplitHPos); }
	"SplitVPos"			{ RETT(INTTYPE, TYPE_SplitVPos); }
	"SplitStruct"			{ RETT(INTTYPE, TYPE_SplitStruct); }
	"StructView"			{ RETT(INTTYPE, TYPE_StructView); }
	"ComboHeight"			{ RETT(INTTYPE, TYPE_ComboHeight); }
	"Language"			{ RETT(INTTYPE, TYPE_Language); }
	"DetectMax"			{ RETT(INTTYPE, TYPE_DetectMax); }
	"BarState"			{ RETT(INTTYPE, TYPE_BarState); }
	"ReadOnly"			{ RETT(INTTYPE, TYPE_ReadOnly); }
	"BmpWidth"			{ RETT(INTTYPE, TYPE_BmpWidth); }
	"BmpZoom"			{ RETT(INTTYPE, TYPE_BmpZoom); }
	"BmpPallet"			{ RETT(INTTYPE, TYPE_BmpPallet); }
	"MaxOnMemory"			{ RETT(INTTYPE, TYPE_MaxOnMemory); }
	"MaxMapSize"			{ RETT(INTTYPE, TYPE_MaxMapSize); }
	"TagAll"			{ RETT(INTTYPE, TYPE_TagAll); }
	"SubCursor"			{ RETT(INTTYPE, TYPE_SubCursor); }
	"Colors"			{ RETT(COLORARRTYPE, TYPE_Colors); }
	"MemberColumns2"		{ RETT(INTARRTYPE, TYPE_MemberColumns2); }
	"PageMargin"			{ RETT(RECTTYPE, TYPE_PageMargin); }
	"DumpHeader"			{ RETT(STRTYPE, TYPE_DumpHeader); }
	"DumpPage"			{ RETT(INTTYPE, TYPE_DumpPage); }
	"QWordAddr"			{ RETT(INTTYPE, TYPE_QWordAddr); }
	"ClearUndoRedoWhenSave"		{ RETT(INTTYPE, TYPE_ClearUndoRedoWhenSave); }
	"SyncScroll"			{ RETT(INTTYPE, TYPE_SyncScroll); }
	"Grid"				{ RETT(INTTYPE, TYPE_Grid); }
	"BmpColorWidth"			{ RETT(INTTYPE, TYPE_BmpColorWidth); }
	"InspectView"			{ RETT(INTTYPE, TYPE_InspectView); }
	"AnalyzerView"			{ RETT(INTTYPE, TYPE_AnalyzerView); }
	"BmpAddressTooltip"		{ RETT(INTTYPE, TYPE_BmpAddressTooltip); }
	
	
	[+-]? [ \t]* (D+)	{ uchar *start = s->tok;
				bool bMinus = false;
				if(*start == '-')bMinus = true;
				while(*start<'0' || *start>'9')start++;
				CStringA tmp;
				tmp.SetString((const char *)(start), cursor - start);
				s->val.i = bMinus ? -atoi(tmp) : atoi(tmp);
				RET(INTVAL); }
	
	RGB			{ s->val.col = ReadRGB(s->tok + 1); RET(COLVAL); }
	RGBA			{ s->val.col = ReadRGBA(s->tok + 1); RET(COLVAL); }
	
	(["] (ESC|any\[\n\\"])* ["])
				{ s->val.str = new CString(); *(s->val.str) = ReadStr(s->tok + 1, cursor - s->tok -2); RET(STRVAL); }
	
	"="			{ RET(EQ); }
	"{"			{ RET(BR1); }
	"}"			{ RET(BR2); }
	","			{ RET(COMMA); }
	[\x1a]			{ RET(0);/*EOF*/ }


	[ \t\v\f]+		{ if(cursor == s->eof)RET(0); goto std; }

	[\n] {
    if(cursor == s->eof)RET(0);
    s->pos = cursor; s->line++;
    goto std;
  }

	any {
    ATLTRACE("unexpected character: %c\n", *s->tok);
    goto std;
  }
*/

comment:
/*!re2c
	"*/"  { if(cursor == s->eof || cursor+1 == s->eof)RET(0); goto std; }
	"\n"  { if(cursor == s->eof) RET(0); s->tok = s->pos = cursor; s->line++; goto comment; }
  any   { if(cursor == s->eof)RET(0); goto comment; }
*/
comment2:
/*!re2c
	"\n" { if(cursor == s->eof) RET(0); s->tok = s->pos = cursor; s->line++; goto std; }
  any  { if(cursor == s->eof)RET(0); goto comment2; }
*/
}