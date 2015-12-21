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

#define PANEMAXW 4
#define PANEMAXH 4
#define PANEMAX (PANEMAXW*PANEMAXH)

#define HEADERHEIGHT 13+7
#define BORDERWIDTH 4

class CBZCoreData;

#include "hsv.h"

class CTamaSplitterWindow : public CWindowImpl<CTamaSplitterWindow>
{
public:
	HWND m_hWndPane[PANEMAX];
	HWND m_hWndHeader[PANEMAX];
	int m_idealSize[PANEMAX][2];
	unsigned int m_borderIdealX[PANEMAXW+1];
	unsigned int m_borderIdealY[PANEMAXH+1];

	unsigned int m_borderRealX[PANEMAXW+1];
	unsigned int m_borderRealY[PANEMAXH+1];
	unsigned int m_paneW;
	unsigned int m_paneH;
	BOOL m_bInitSplit;
	BOOL m_bPressLButton;
	int m_selectBorderX;
	int m_selectBorderY;
  unsigned int m_bHeader;
  unsigned int m_headermode;
  unsigned int m_numHeaderPaneX;
  WTL::CBrush m_halftoneBrush;
  WTL::CBrush m_borderBrush;

	BEGIN_MSG_MAP_EX(CTamaSplitterWindow)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_SETCURSOR(OnSetCursor)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_SETFOCUS(OnSetFocus)
    END_MSG_MAP()

	CTamaSplitterWindow() : CWindowImpl<CTamaSplitterWindow>()
	{
		::ZeroMemory(m_hWndPane, sizeof(m_hWndPane));
		::ZeroMemory(m_hWndHeader, sizeof(m_hWndHeader));
		::ZeroMemory(m_borderIdealX, sizeof(m_borderIdealX));
		::ZeroMemory(m_borderIdealY, sizeof(m_borderIdealY));
		::ZeroMemory(m_borderRealX, sizeof(m_borderRealX));
		::ZeroMemory(m_borderRealY, sizeof(m_borderRealY));
		m_bInitSplit = FALSE;
		m_selectBorderX = m_selectBorderY = -1;
		m_bPressLButton = FALSE;
    m_bHeader = FALSE;
    m_headermode = SPLITMODE::NOSPLIT;
    m_numHeaderPaneX = 1;
    m_halftoneBrush = WTL::CDCHandle::GetHalftoneBrush();
    InitBorderBrush();
	}

  void InitBorderBrush()
  {
    COLORREF rgb = GetSysColor(COLOR_BTNFACE);
    hsv_t hsv;
    rgb2hsv(rgb, &hsv);
    if(hsv.saturation>=15)hsv.saturation-=15;
    else hsv.saturation=0;
    if(hsv.value>=15)hsv.value-=15;
    else hsv.value=0;
    m_borderBrush.CreateSolidBrush(hsv2rgb(&hsv));
  }

  enum SPLITMODE
  {
    NOSPLIT = 0,
    TATE = 1,
    YOKO = 2
  };

  void SetHeaderMode(BOOL bEnable, unsigned int headermode = SPLITMODE::NOSPLIT, unsigned int numHeaderPaneX=1)
  {
    m_bHeader = bEnable;
    m_headermode = bEnable ? headermode : SPLITMODE::NOSPLIT;
    m_numHeaderPaneX = numHeaderPaneX;
    ATLTRACE("SetHeaderMode: %s\n", headermode==SPLITMODE::TATE?"tate":headermode==SPLITMODE::YOKO?"yoko":"no");
  }
  void SetHeader(HWND hWnd, unsigned int idx)
  {
		ATLASSERT(::IsWindow(hWnd));
		m_hWndHeader[idx] = hWnd;
  }

	HWND CreateStatic(HWND hWndParent, unsigned int w, unsigned int h)
	{
		ATLASSERT(w < PANEMAXW);
		ATLASSERT(h < PANEMAXH);
		m_paneW = w;
		m_paneH = h;
		return Create(hWndParent);//, NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	}

	void SetSplitterPane(HWND hWndPane, unsigned int x, unsigned int y)
	{
		ATLASSERT(x < PANEMAXW);
		ATLASSERT(y < PANEMAXH);
		ATLASSERT(::IsWindow(hWndPane));
		unsigned int idx = CalcSplitIndex(x,y);
		m_hWndPane[idx] = hWndPane;
    WTL::CRect rect;
		::GetWindowRect(hWndPane, &rect);
		m_idealSize[idx][0] = rect.Width();
		m_idealSize[idx][1] = rect.Height();
//		UpdateSplitLayout();
	}
	void SetSplitterPosX(unsigned int x, unsigned int val)
	{
		m_borderIdealX[x] = val;
	}
	void SetSplitterPosY(unsigned int y, unsigned int val)
	{
		m_borderIdealY[y] = val;
	}

	unsigned int CalcSplitIndex(unsigned int x, unsigned int y)
	{
		ATLASSERT(x < PANEMAXW);
		ATLASSERT(y < PANEMAXH);
		return y*PANEMAXW+x;
	}
	


  void OnSetFocus(CWindow wndOld);

	BOOL OnEraseBkgnd(WTL::CDCHandle dc)
	{
		return TRUE;
	}

  void OnPaint(WTL::CDCHandle /*dc*/)
	{
    WTL::CPaintDC dc(m_hWnd);
    WTL::CRect rect;
		dc.GetClipBox(&rect);
		ATLTRACE("WM_PAINT: m_bPressLButton=%s, m_selectBorderX=%d, m_selectBorderY=%d\n",m_bPressLButton?"TRUE":"FALSE", m_selectBorderX, m_selectBorderY);
		
		dc.FillRect(rect, m_borderBrush/*COLOR_BTNFACE*/);
		if(m_bPressLButton && m_selectBorderX!=-1)
		{
      WTL::CRect barX(m_borderRealX[m_selectBorderX]-BORDERWIDTH, 0, m_borderRealX[m_selectBorderX], rect.bottom);
			dc.FillRect(barX, m_halftoneBrush);
		}
		if(m_bPressLButton && m_selectBorderY!=-1)
		{
      WTL::CRect barY(0, m_borderRealY[m_selectBorderY]-BORDERWIDTH, rect.right, m_borderRealY[m_selectBorderY]);
			dc.FillRect(barY, m_halftoneBrush);
		}
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if(wParam != SIZE_MINIMIZED)
		{
			UpdateSplitLayout();
		}
		return TRUE;
	}

  unsigned int GetColumnCount() { return m_paneW; }
  unsigned int GetRowCount()    { return m_paneH; }

	unsigned int GetPaneWidth(int x)
	{
		ATLASSERT(x < PANEMAXW);
		unsigned int ret = m_borderRealX[x+1]-m_borderRealX[x];//-(x==m_paneW-1 ? 0:BORDERWIDTH);
		if(x != m_paneW-1)
		{
			if(ret>BORDERWIDTH)ret-=BORDERWIDTH;
			else ret=0;
		}
		return ret;
	}
	unsigned int GetHeaderWidth(int x)
	{
		ATLASSERT(x < PANEMAXW);
		unsigned int ret = m_borderRealX[x+2]-m_borderRealX[x];//-(x==m_paneW-1 ? 0:BORDERWIDTH);
		if(x != m_paneW-2)
		{
			if(ret>BORDERWIDTH)ret-=BORDERWIDTH;
			else ret=0;
		}
		return ret;
	}

	unsigned int GetPaneHeight(int y)
	{
		ATLASSERT(y < PANEMAXH);
		unsigned int ret = m_borderRealY[y+1]-m_borderRealY[y];//-(y==m_paneH-1 ? 0:BORDERWIDTH);

    unsigned int yParam = 0;
    if(y != m_paneH-1)yParam += BORDERWIDTH;
    if(m_bHeader)yParam += HEADERHEIGHT;

		if(ret>yParam)ret-=yParam;
		else ret=0;
    return ret;
  }

	void InitSplitLayout()
	{
		m_bInitSplit = TRUE;
		UpdateSplitLayout();
	}

	void UpdateSplitLayout()
	{
		if(!m_bInitSplit)return;
    WTL::CRect rect;
		if(!GetClientRect(&rect))return;
		m_borderIdealX[0] = m_borderIdealY[0] = m_borderRealX[0] = m_borderRealY[0] = 0;
		m_borderIdealX[m_paneW] = m_borderRealX[m_paneW] = rect.Width();
		m_borderIdealY[m_paneH] = m_borderRealY[m_paneH] = rect.Height();
		if(m_borderIdealX[m_paneW]==0 && m_borderIdealY[m_paneH]==0)return;

		for(int x=m_paneW-1; x>=0; x--)
		{
      unsigned int xParam = 0;
      if(x!=m_paneW-1)xParam = BORDERWIDTH;

			if(m_borderIdealX[x] > m_borderRealX[x+1]-xParam)
			{
				m_borderRealX[x] = m_borderRealX[x+1]-xParam;
			} else m_borderRealX[x] = m_borderIdealX[x];
		}
		for(int y=m_paneH-1; y>=0; y--)
		{
      unsigned int yParam = 0;
      if(y!=m_paneH-1)yParam = BORDERWIDTH;
      if(m_bHeader)yParam += HEADERHEIGHT;

			if(m_borderIdealY[y] > m_borderRealY[y+1]-yParam)
			{
				m_borderRealY[y] = m_borderRealY[y+1]-yParam;
			} else m_borderRealY[y] = m_borderIdealY[y];
		}
    unsigned int idxHeader=0;
		for(int y=0;y<m_paneH;y++)
    {
      if(m_headermode==SPLITMODE::NOSPLIT || m_headermode==SPLITMODE::TATE)
      {
        ::MoveWindow(m_hWndHeader[idxHeader], 0, m_borderRealY[y], rect.Width(), HEADERHEIGHT, TRUE);
        idxHeader++;
      }
			for(int x=0;x<m_paneW;x++)
			{
				unsigned int idx = CalcSplitIndex(x,y);
				ATL::CWindow wnd = ATL::CWindow(m_hWndPane[idx]);
        ATLASSERT(wnd.m_hWnd);
				if(wnd.m_hWnd)
				{
					wnd.MoveWindow(m_borderRealX[x], m_borderRealY[y]+(m_bHeader?HEADERHEIGHT:0), GetPaneWidth(x), GetPaneHeight(y), TRUE);
					ATLTRACE("PaneA(%d,%d)(%d,%d %d,%d)\n", x, y, m_borderIdealX[x], m_borderIdealY[y], GetPaneWidth(x), GetPaneHeight(y));
        }
        if(m_headermode==SPLITMODE::YOKO && x%m_numHeaderPaneX==0)
        {
          ::MoveWindow(m_hWndHeader[idxHeader], m_borderRealX[x], m_borderRealY[y], GetPaneWidth(x)+GetPaneWidth(x+1)+BORDERWIDTH, HEADERHEIGHT, TRUE);
          idxHeader++;
        }
			}
		}
		ATLTRACE("\n");
	}

	int IsBorderX(POINT pt)
	{
		for(int x=1; x<=m_paneW-1; x++)
		{
			unsigned int tmpx = (m_borderRealX[x]-1) - ((unsigned int)pt.x);
			if(tmpx < BORDERWIDTH)
				return x;
		}
		return -1;
	}
	int IsBorderY(POINT pt)
	{
		for(int y=1; y<=m_paneH-1; y++)
		{
			unsigned int tmpy = (m_borderRealY[y]-1) - ((unsigned int)pt.y);
			if(tmpy < BORDERWIDTH)
				return y;
			//ATLTRACE("IsBorderY: tmpy=%d, RealY=%d, pt.y=%d\n", tmpy, m_borderRealY[y], pt.y);
		}
		return -1;
	}

	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if(!m_bPressLButton)
		{
			m_selectBorderX = IsBorderX(pt);
			m_selectBorderY = IsBorderY(pt);
		}
		BOOL bBorderX = m_selectBorderX!=-1;
		BOOL bBorderY = m_selectBorderY!=-1;

		//ATLTRACE("WM_SETCURSOR: pt(%d,%d) bBorderX = %s, bBorderY = %s\n",pt.x,pt.y, bBorderX?"True":"False", bBorderY?"True":"False");
		if(bBorderX && bBorderY)SetCursor(LoadCursor(NULL, IDC_SIZEALL));
		else if(bBorderX)SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		else if(bBorderY)SetCursor(LoadCursor(NULL, IDC_SIZENS));
		return (bBorderX || bBorderY);
	}
  void OnLButtonDown(UINT nFlags, WTL::CPoint point)
	{
		//POINT pt = point;
		//m_selectBorderX = IsBorderX(pt);
		//m_selectBorderY = IsBorderY(pt);
		m_bPressLButton = TRUE;
		SetCapture();
		Invalidate();
		//ATLTRACE("WM_LBUTTONDOWN: pt(%d,%d) m_selectBorderX = %d, m_selectBorderY = %d\n",point.x,point.y, m_selectBorderX, m_selectBorderY);
	}
  void OnMouseMove(UINT nFlags, WTL::CPoint point)
	{
		if(m_bPressLButton)
		{
      int x3=0, y3=0, x2=m_paneW, y2=m_paneH;
      WTL::CRect rect;
      GetClientRect(&rect);
			if(m_selectBorderX!=-1)
			{
				m_borderIdealX[m_selectBorderX]=m_borderRealX[m_selectBorderX]=min((long)(m_borderRealX[m_selectBorderX+1]-BORDERWIDTH*2), max(point.x, (long)(m_borderRealX[m_selectBorderX-1]+BORDERWIDTH*2)));
				x3=m_selectBorderX-1;
				x2 = x3+2;
			}
			if(m_selectBorderY!=-1)
			{
				m_borderIdealY[m_selectBorderY]=m_borderRealY[m_selectBorderY]=min((long)(m_borderRealY[m_selectBorderY+1]-BORDERWIDTH*2), max(point.y, (long)(m_borderRealY[m_selectBorderY-1]+BORDERWIDTH*2+(m_bHeader?HEADERHEIGHT:0))));
				y3=m_selectBorderY-1;
				y2 = x3+2;
			}
			//ATLTRACE("MoveStart:\n");
			for(int y=y3;y<y2;y++)
      {
        if(m_headermode==SPLITMODE::TATE)
        {
          ::MoveWindow(m_hWndHeader[y], 0, m_borderRealY[y], rect.Width(), HEADERHEIGHT, TRUE);
        }
				for(int x=x3;x<x2;x++)
				{
					unsigned int idx = CalcSplitIndex(x,y);
					ATL::CWindow wnd = ATL::CWindow(m_hWndPane[idx]);
					if(wnd.m_hWnd)
					{
						wnd.MoveWindow(m_borderRealX[x], m_borderRealY[y]+(m_bHeader?HEADERHEIGHT:0), GetPaneWidth(x), GetPaneHeight(y), TRUE);
						//ATLTRACE("Move(%d,%d)(%d,%d %d,%d)\n", x, y, m_borderIdealX[x], m_borderIdealY[y], GetPaneWidth(x), GetPaneHeight(y));
          }
          if(m_headermode==SPLITMODE::YOKO)
          {
            int XAmari = x%m_numHeaderPaneX;
            int XPaneStart = x-XAmari;
            ::MoveWindow(m_hWndHeader[XAmari], m_borderRealX[XPaneStart], m_borderRealY[y], GetPaneWidth(XPaneStart)+GetPaneWidth(XPaneStart+1)+BORDERWIDTH, HEADERHEIGHT, TRUE);
          }
				}
			}
			//ATLTRACE("MoveEnd\n\n");
			Invalidate();
		}
	}
  void OnLButtonUp(UINT nFlags, WTL::CPoint point)
	{
		m_selectBorderX = m_selectBorderY = -1;
		m_bPressLButton = FALSE;
		ReleaseCapture();
		Invalidate();
	}
};
