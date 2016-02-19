
#include "stdafx.h"
#include <math.h>

typedef unsigned char uchar;
typedef unsigned int uint;

#define	YYCTYPE		uchar
#define	YYCURSOR	cursor
#define	YYLIMIT		s->lim
#define	YYMARKER	s->ptr

#define	RET(i)	{s->cur = cursor; return i;}
#define	RETT(i, typeid)	{s->cur = cursor; s->val.type = typeid; return i;}

typedef struct PassData
{
  union {
      int i;
      CString *str;
      COLORREF col;
  };
  uint		type;
} PassData;

typedef struct Scanner
{
    int			fd;
    uchar		*bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof;
    uint		line;
    PassData val;
} Scanner;

//hsv.h
typedef struct hsva_tag hsva_t;
struct hsva_tag {
    int hue;               /* 0 ... (360*HUE_DEGREE - 1) */
    BYTE saturation;       /* 0 ... 255 */
    BYTE value;            /* 0 ... 255 */
    BYTE alpha;            /* 0 ... 255 */
};

typedef struct color_tag color_t;
struct color_tag {
  enum {RGBA, HSVA} type;
  union {
    hsva_t hsva;
    COLORREF rgba;
  };
};

typedef struct palletcmd_tag palletcmd_t;
struct palletcmd_tag {
  enum {PCMD_BG, PCMD_DEF, PCMD_COLORARR} type;
  union {
    struct {
      CAtlArray<int>* iArr;
      CAtlArray<color_t>* cArr;
    };
    color_t c;
  };
};

enum {TAG_BG=0, TAG_DEFAULT, TAG_HSV, TAG_RGB, BR3, BR4, EQ, MINUS, COMMA, COLVAL, INTVAL};

static DWORD *pPalletTxt256 = NULL;
static COLORREF g_bgBmpView = 0xFFFFFFFF;


double lerp(double a, double b, double t)
{
  return a + (b-a)*t;
}

int limitInt(int x, int min, int max)
{
  if(x<min)return min;
  else if(x>max)return max;
  return x;
}

int lerpAndLimitInt(double a, double b, double t, int min, int max)
{
  return limitInt(floor(lerp(a, b, t)+0.5), min, max);
}

int roundAndLimitInt(double x, int min, int max)
{
  return limitInt(floor(x+0.5), min, max);
}

COLORREF SetRGB(int r, int g, int b)
{
  r = limitInt(r, 0, 255);
  g = limitInt(g, 0, 255);
  b = limitInt(b, 0, 255);
  COLORREF c = 0xFF000000 | (r & 0xFF) | ((g & 0xFF)<<8) | ((b & 0xFF)<<16);
  return c;
}

COLORREF SetRGBA(int r, int g, int b, int a)
{
  a = limitInt(a, 0, 255);
  COLORREF c = SetRGB(r,g,b) | ((a & 0xFF)<<24);
  return c;
}

hsva_t SetHSV(int h, int s, int v)
{
  h = limitInt(h*HUE_DEGREE, 0, (360*HUE_DEGREE - 1));
  s = limitInt(s, 0, 255);
  v = limitInt(v, 0, 255);
  hsva_t c;
  c.hue = h;
  c.saturation = s;
  c.value = v;
  c.alpha = 255;
  return c;
}

hsva_t SetHSVA(int h, int s, int v, int a)
{
  hsva_t c = SetHSV(h, s, v);
  c.alpha = limitInt(a, 0, 255);
  return c;
}

RGBQUAD COLORREF2RGBQUAD(const COLORREF cr)
{
  RGBQUAD rq = {0};
  rq.rgbRed = (cr & 0xFF);
  rq.rgbGreen = (cr & 0x0000FF00)>>8;
  rq.rgbBlue = (cr & 0x00FF0000)>>16;
  rq.rgbReserved = (cr & 0xFF000000)>>24;
  return rq;
}

COLORREF GetCOLORREF(color_t *ct)
{
  switch(ct->type)
  {
    case color_t::RGBA:
      return ct->rgba;
    case color_t::HSVA:
    {
      hsv_t hsv = {ct->hsva.hue, ct->hsva.saturation, ct->hsva.value};
      return hsv2rgb(&hsv) | (((ct->hsva.alpha) & 0xFF)<<24);
    }
    default:
      ATLASSERT(FALSE);
      return 0;
  }
}

RGBQUAD GetRGBQUAD(const color_t *ct) //BGRA (!! RGBA‚Å‚Í‚È‚¢‚Ì‚Å’ˆÓ !!)
{
  switch(ct->type)
  {
    case color_t::RGBA:
      return COLORREF2RGBQUAD(ct->rgba);
    case color_t::HSVA:
    {
      hsv_t hsv = {ct->hsva.hue, ct->hsva.saturation, ct->hsva.value};
      return COLORREF2RGBQUAD(hsv2rgb(&hsv) | (((ct->hsva.alpha) & 0xFF)<<24));
    }
    default:
      ATLASSERT(FALSE);
      return COLORREF2RGBQUAD(0);
  }
}

void ReleasePalletCmd(palletcmd_t pCmd)
{
  if(pCmd.type==palletcmd_t::PCMD_COLORARR)
  {
    delete (pCmd.iArr);
    delete (pCmd.cArr);
  }
}

void ReleasePalletCmdArr(CAtlArray<palletcmd_t> *pCmdArr)
{
  int numCmd=pCmdArr->GetCount();
  for(int i=0;i<numCmd;i++)
  {
    ReleasePalletCmd((*pCmdArr)[i]);
  }
  delete pCmdArr;
}

color_t toHSVA(color_t c)
{
  if(c.type==color_t::RGBA)
  {
    c.type = color_t::HSVA;
    COLORREF cr = c.rgba;
    hsv_t hsv;
    rgb2hsv(cr, &hsv);
    c.hsva.hue = hsv.hue;
    c.hsva.saturation = hsv.saturation;
    c.hsva.value = hsv.value;
    c.hsva.alpha = (cr & 0xFF000000)>>24;
  }
  return c;
}

color_t toRGBA(color_t c)
{
  if(c.type==color_t::HSVA)
  {
    c.type = color_t::RGBA;
    hsv_t hsv;
    hsv.hue = c.hsva.hue;
    hsv.saturation = c.hsva.saturation;
    hsv.value = c.hsva.value;
    c.rgba = hsv2rgb(&hsv) & ((c.hsva.alpha)<<24);
  }
  return c;
}

RGBQUAD _RGBAInterpolation(color_t col1, color_t col2, double t)
{
  COLORREF c1 = col1.rgba, c2 = col2.rgba;
  COLORREF c3;
  c3 = lerpAndLimitInt(c1 & 0xFF, c2 & 0xFF, t, 0, 255) & 0xFF;
  c3 |= (lerpAndLimitInt((c1&0xFF00)>>8, (c2&0xFF00)>>8, t, 0, 255) & 0xFF) << 8;
  c3 |= (lerpAndLimitInt((c1&0xFF0000)>>16, (c2&0xFF0000)>>16, t, 0, 255) & 0xFF) << 16;
  DWORD a = (lerpAndLimitInt((c1&0xFF000000)>>16, (c2&0xFF000000)>>24, t, 0, 255) & 0xFF) << 24;
  return COLORREF2RGBQUAD(c3|a);
}

RGBQUAD _HSVAInterpolation(color_t col1, color_t col2, double t)
{
  hsv_t hsv;
  hsv.hue = lerpAndLimitInt(col1.hsva.hue, col2.hsva.hue, t, 0, (360*HUE_DEGREE - 1));
  hsv.saturation = lerpAndLimitInt(col1.hsva.saturation, col2.hsva.saturation, t, 0, 255);
  hsv.value = lerpAndLimitInt(col1.hsva.value, col2.hsva.value, t, 0, 255);
  DWORD a = lerpAndLimitInt(col1.hsva.alpha, col2.hsva.alpha, t, 0, 255);
  return COLORREF2RGBQUAD(hsv2rgb(&hsv) | ((a & 0xFF)<<24) );
}

double GetAlpha(COLORREF c)
{
  double a1 = ((c & 0xFF000000) >> 24);
  return a1 / 255.0;
}

COLORREF alphaBlendCOLORREF(COLORREF c1, COLORREF c2)
{
  COLORREF n = 0;
  double a1 = GetAlpha(c1);
  double a2 = GetAlpha(c2);
  double a3 = a2 + (1.0-a2) * a1;
  
  double x1 = ((1.0-a2)*a1)/a3;
  double x2 = a2/a3;
  double r1 = c1 & 0xFF;
  double r2 = c2 & 0xFF;
  double g1 = (c1 & 0xFF00) >>8;
  double g2 = (c2 & 0xFF00) >>8;
  double b1 = (c1 & 0xFF0000) >>16;
  double b2 = (c2 & 0xFF0000) >>16;
  n = roundAndLimitInt(r1*x1 + r2*x2, 0, 255) & 0xFF;
  n |= (roundAndLimitInt(g1*x1 + g2*x2, 0, 255) & 0xFF)<<8;
  n |= (roundAndLimitInt(b1*x1 + b2*x2, 0, 255) & 0xFF)<<16;
  n |= (roundAndLimitInt(a3*255.0, 0, 255) & 0xFF)<<24;
  return n;
}
RGBQUAD alphaBlendRGBQUAD(RGBQUAD c1, RGBQUAD c2)
{
  RGBQUAD n;
  double a1 = c1.rgbReserved;
  a1/=255.0;
  double a2 = c2.rgbReserved;
  a2/=255.0;
  double a3 = a2 + (1.0-a2) * a1;
  
  double x1 = ((1.0-a2)*a1)/a3;
  double x2 = a2/a3;
  double r1 = c1.rgbRed;
  double r2 = c2.rgbRed;
  double g1 = c1.rgbGreen;
  double g2 = c2.rgbGreen;
  double b1 = c1.rgbBlue;
  double b2 = c2.rgbBlue;
  n.rgbRed = roundAndLimitInt(r1*x1 + r2*x2, 0, 255) & 0xFF;
  n.rgbGreen = roundAndLimitInt(g1*x1 + g2*x2, 0, 255) & 0xFF;
  n.rgbBlue = roundAndLimitInt(b1*x1 + b2*x2, 0, 255) & 0xFF;
  n.rgbReserved = roundAndLimitInt(a3*255.0, 0, 255) & 0xFF;
  return n;
}

void ColorInterpolation(RGBQUAD *pRGB, int idx1, int idx2, color_t col1, color_t col2)
{
  idx1 = limitInt(idx1, 0, 255);
  idx2 = limitInt(idx2, 0, 255);
  if(idx1>idx2)
  {
    int idxt=idx1;
    idx1=idx2;
    idx2=idxt;
    color_t t = col1;
    col1 = col2;
    col2 = t;
  }
  pRGB[idx1]=GetRGBQUAD(&col1);
  double m = idx2 - idx1;
  color_t c1, c2;
  BOOL bRGB = TRUE;
  if(col1.type==color_t::RGBA && col2.type==color_t::RGBA)
  {
    c1 = toRGBA(col1);
    c2 = toRGBA(col2);
  } else {
    c1 = toHSVA(col1);
    c2 = toHSVA(col2);
    bRGB = FALSE;
  }
  for(int i=idx1;i<=idx2;i++)
  {
    double t = i-idx1;
    if(0<t)t /= m;
    if(bRGB)pRGB[i]=alphaBlendRGBQUAD(pRGB[i], _RGBAInterpolation(col1, col2, t));
    else pRGB[i]=alphaBlendRGBQUAD(pRGB[i], _HSVAInterpolation(col1, col2, t));
  }
}

void ProcPalletCmdArr(CAtlArray<palletcmd_t> *pCmdArr)
{
  COLORREF bg = 0xFFFFFFFF;
  color_t def = {color_t::RGBA, 0xFF000000};
  ATLTRACE("\nProcPalletCmdArr\n");
  RGBQUAD *pRGB = (RGBQUAD *)pPalletTxt256;
  int numCmd=pCmdArr->GetCount();
  for(int i=0;i<numCmd;i++)
  {
    palletcmd_t *pc = &(*pCmdArr)[i];
    switch(pc->type)
    {
    case palletcmd_t::PCMD_BG:
      g_bgBmpView = bg = GetCOLORREF(&(pc->c));
      break;
    case palletcmd_t::PCMD_DEF:
      def = pc->c;
      break;
    }
  }
  RGBQUAD defcol = GetRGBQUAD(&def);
  ATLTRACE("default color: %08X\n", defcol);
  for(int i=0;i<256;i++)
  {
    pRGB[i] = alphaBlendRGBQUAD(pRGB[i], defcol);
  }
  
  pRGB = (RGBQUAD *)pPalletTxt256;
  for(int i=0;i<numCmd;i++)
  {
    palletcmd_t *pc = &(*pCmdArr)[i];
    if(pc->type==palletcmd_t::PCMD_COLORARR)
    {
      CAtlArray<int>& iArr = *(pc->iArr);
      CAtlArray<color_t>& cArr = *(pc->cArr);
      int n = min(iArr.GetCount(), cArr.GetCount());
      if(n<=0)continue;
      if(n==1)
      {
        int idx = iArr[0];
        if(idx>=0 && idx<256)pRGB[idx]=alphaBlendRGBQUAD(pRGB[idx], GetRGBQUAD(&(cArr[0])));
      } else {
        for(int i=1;i<n;i++)
        {
          ColorInterpolation((RGBQUAD *)pPalletTxt256, iArr[i-1], iArr[i], cArr[i-1], cArr[i]);
        }
      }
    }
  }
  for(int i=0;i<256;i++)
  {
    pPalletTxt256[i] &= 0xFFFFFF;
  }
}


#include "CustomPalletParser.h"
#include "CustomPalletParser.c"

COLORREF ReadRGB(uchar *token);
COLORREF ReadRGBA(uchar *token);
BOOL fill(int n);

int scanPalletTxt(Scanner *s){
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
	
	('0x'H+)	{ uchar *start = s->tok;
	    	start+=2;
				_snscanf((const char *)(start), cursor - start, "%x", &(s->val.i));
				RET(INTVAL); }
	
	(D+)	{ uchar *start = s->tok;
				_snscanf((const char *)(start), cursor - start, "%d", &(s->val.i));
				RET(INTVAL); }
	
	RGB			{ s->val.col = ReadRGB(s->tok + 1); RET(COLVAL); }
	RGBA			{ s->val.col = ReadRGBA(s->tok + 1); RET(COLVAL); }
	
	'bg'			{ RET(TAG_BG); }
	'background'	{ RET(TAG_BG); }
	'def'   	{ RET(TAG_DEFAULT); }
	'default'	{ RET(TAG_DEFAULT); }
	'hsv'	{ RET(TAG_HSV); }
	'rgb'	{ RET(TAG_RGB); }
	"("			{ RET(BR3); }
	")"			{ RET(BR4); }
	"="			{ RET(EQ); }
	"-"			{ RET(MINUS); }
	","			{ RET(COMMA); }
	[\x1a]			{ RET(0);/*EOF*/ }


	[ \t\v\f]+		{ if(cursor == s->eof)RET(0); goto std; }

	("\r\n" | "\n" | "\r") {
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