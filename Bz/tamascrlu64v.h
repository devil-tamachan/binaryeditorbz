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

//GetScrollPos(SB_VERT)使うな！
//かわりにm_ptOffset.yを使え！
//  スクロール下いっぱいまでスクロールして更に下ボタンクリックでスクロールしてから右左スクロールするとバグる


#define SB_WHEELUP   10
#define SB_WHEELDOWN 11
#define SB_UPDATEFROMPARAM 12

template <class T>
class CTamaScrollU64VImpl : public WTL::CScrollImpl< T >
{
public:
  //DWORD m_lowV;
  DWORD m_step;
  UINT64 m_u64V; // == m_lowV + si.nPos*m_step
  UINT64 m_u64VMax; // ==10 (m_u64V == [0:10])

  DWORD m_cellV;
  
  CTamaScrollU64VImpl()
  {
    //m_lowV = 0;
    m_step = UINT_MAX;
    m_u64V = 0;
    m_u64VMax = 0;

    m_cellV = 1;
  }

  UINT64 ConvYRealSpace2VirtualSpace(long y)
  {
    long v = m_ptOffset.y;//GetScrollPos(SB_VERT);
    UINT64 v64 = GetScrollPosU64V();
    
    UINT64 yVS = y<0?0:y;
    yVS += - v + v64;
    
    return yVS;
  }
  long ConvYVirtualSpace2RealSpace(UINT64 yVS)
  {
    long v = m_ptOffset.y;//GetScrollPos(SB_VERT);スクロール下いっぱいまでスクロールして更に下ボタンクリックでスクロールしてから右左スクロールするとバグる
    UINT64 v64 = GetScrollPosU64V();
    yVS += v;
    yVS -= v64;
    ATLASSERT(yVS>=0);
    return yVS<0 ? 0 : (long)yVS;
  }
  
  void SetScrollSizeU64V(int cx, UINT64 cy, BOOL bRedraw = TRUE, bool bResetOffset = true)
  {
    if(cy>_UI64_MAX/2)cy = _UI64_MAX/2-1;
    m_u64VMax = cy;
    AdjustFromU64VMax();
    int sizeAllY = m_sizeAll.cy;
		SetScrollSize(cx, m_sizeAll.cy, bRedraw, bResetOffset);
    if(bResetOffset)
    {
      m_ptOffset.y = 0;//INT_MIN;
      SCROLLINFO si = {sizeof(SCROLLINFO)};
      si.fMask = SIF_POS | SIF_RANGE;
      si.nMin = 0;//INT_MIN;
      si.nMax = sizeAllY;
      si.nPos = m_ptOffset.y;
      T* pT = static_cast<T*>(this);
      ATLASSERT(::IsWindow(pT->m_hWnd));
    ATLTRACE("SetScrollSizeU64V - m_u64V:%I64d, m_u64VMax:%I64d, nMin:%d, nMax:%d, m_sizeAll.cy:%d, nPos:%d\n", m_u64V, m_u64VMax, si.nMin, si.nMax, m_sizeAll.cy, si.nPos);
      pT->SetScrollInfo(SB_VERT, &si, bRedraw);
    }
  }
  
  void AdjustFromU64VMax(BOOL bRedraw = TRUE)
  {
    {//step値を決定
      if(m_u64VMax <= INT_MAX)
      {
        m_step = 1;
        m_sizeAll.cy = (int)m_u64VMax;
      } else {
        m_step = (DWORD)(m_u64VMax / (INT_MAX-(m_u64VMax>INT_MAX?2:0)));
        if(m_u64VMax % (INT_MAX-1))m_step++;
        CalcScrolVMax();
        (m_sizeAll.cy)+=(m_u64VMax>INT_MAX?2:0);
      }
    }
    if(m_u64V > m_u64VMax)m_u64V = m_u64VMax;
    UpdateFromU64V();
  }
  
  void UpdateFromU64V()
  {
    ATLTRACE("UpdateFromU64V +++ m_ptOffset.y:%d (0x%08X), m_sizeAll.cy:%d (0x%08X), m_u64V: 0x%016I64X\n", m_ptOffset.y, m_ptOffset.y, m_sizeAll.cy, m_sizeAll.cy, m_u64V);
    {//Vの補助値(下位DWORD)を決定
      //m_lowV = (DWORD)(m_u64V % m_step);
    }
    {//Windowsへ渡すスクロールバーの値(int)決定
      m_ptOffset.y = (int)(((m_u64V+(m_step>1&&m_u64V?m_step-1:0)) / m_step));
      if(m_ptOffset.y > m_sizeAll.cy-(m_sizePage.cy/(int)m_cellV))m_ptOffset.y = m_sizeAll.cy-(m_sizePage.cy/(int)m_cellV);
    }
    ATLTRACE("UpdateFromU64V --- m_ptOffset.y:%d (0x%08X), m_sizeAll.cy:%d (0x%08X), m_u64V: 0x%016I64X\n", m_ptOffset.y, m_ptOffset.y, m_sizeAll.cy, m_sizeAll.cy, m_u64V);
  }

  int CalcScrolVMax()
  {
    m_sizeAll.cy = (int)(m_u64VMax / m_step);
    if(m_u64VMax % m_step) (m_sizeAll.cy)++;
    return m_sizeAll.cy;
  }
  
  void UpdateFromRawPos()
  {
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));
    if(pT->GetScrollInfo(SB_VERT, &si))
    {
      ATLTRACE("UpdateFromRawPos +++ %d (0x%08X), m_u64V: 0x%016I64X\n", si.nTrackPos, si.nTrackPos, m_u64V);
      if(si.nTrackPos == m_ptOffset.y)return;
      m_ptOffset.y = si.nTrackPos;
      //m_lowV = m_step==1 ? 0 : m_step-1;
      m_u64V = (UINT64)(m_ptOffset.y) * m_step - (m_ptOffset.y>0?m_step-1:0);
      ATLTRACE("UpdateFromRawPos --- %d (0x%08X), m_u64V: 0x%016I64X\n", si.nTrackPos, si.nTrackPos, m_u64V);
    }
  }
  
  UINT64 GetScrollPosU64V()
  {
    return m_u64V;
  }

#ifdef _DEBUG
  void TRACEParams()
  {
    ATLTRACE("m_step:0x%08X, m_u64V:0x%016I64X, m_u64VMax:0x%016I64X, m_ptOffset.y:%d (0x%08X), m_sizeAll.cy:%d, m_sizePage:%d, m_sizeClient:%d\n", m_step, m_u64V, m_u64VMax, m_ptOffset.y, m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeClient.cy);
  }
#endif
  
  void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
  {
#ifdef _DEBUG
    TRACEParams();
#endif
    UINT64 oldU64V = m_u64V;
    if(nType == SB_VERT)
    {
      switch(nScrollCode)
      {
      case SB_TOP:
        m_u64V = 0;
        UpdateFromU64V();
        break;
      case SB_BOTTOM:
        m_u64V = m_u64VMax;
        UpdateFromU64V();
        break;
      case SB_LINEUP:
        if(m_u64V >= cxySizeLine)m_u64V -= cxySizeLine;
        else m_u64V = 0;
        UpdateFromU64V();
        break;
      case SB_LINEDOWN:
      {
        UINT64 newV = m_u64V +cxySizeLine;
        if(newV < m_u64V || newV > m_u64VMax)m_u64V = m_u64VMax;
        else m_u64V = newV;
        UpdateFromU64V();
        break;
      }
      case SB_PAGEUP:
        if(m_u64V >= cxySizePage)m_u64V -= cxySizePage;
        else m_u64V = 0;
        UpdateFromU64V();
        break;
      case SB_PAGEDOWN:
      {
        UINT64 newV = m_u64V +cxySizePage;
        if(newV < m_u64V || newV > m_u64VMax)m_u64V = m_u64VMax;
        else m_u64V = newV;
        UpdateFromU64V();
        break;
      }
      case SB_WHEELUP:
        ATLTRACE("SB_WHEELUP --- -(%d)\n", m_nWheelLines);
        if(m_u64V >= m_nWheelLines)m_u64V -= m_nWheelLines;//m_nWheelLines;
        else m_u64V = 0;
        UpdateFromU64V();
        break;
      case SB_WHEELDOWN:
      {
        ATLTRACE("SB_WHEELDOWN--- +(%d)\n", cxySizeLine);
        UINT64 newV = m_u64V +m_nWheelLines;//m_nWheelLines;
        if(newV < m_u64V || newV > m_u64VMax)m_u64V = m_u64VMax;
        else m_u64V = newV;
        UpdateFromU64V();
        break;
      }
      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
        UpdateFromRawPos();
        if(m_ptOffset.y == m_sizeAll.cy)
        {
          m_u64V = m_u64VMax;
          UpdateFromU64V();
        }
        break;
      case SB_UPDATEFROMPARAM:
        UpdateFromU64V();
        break;
      default:
        return;
      }
      T* pT = static_cast<T*>(this);
      ATLASSERT(::IsWindow(pT->m_hWnd));
      pT->SetScrollPos(SB_VERT, m_ptOffset.y, TRUE);
      int dy = 0;
      if(oldU64V == m_u64V)
      {
        return;
      } else if(oldU64V > m_u64V)
      {
        UINT64 diffU64V = oldU64V - m_u64V;
        if(diffU64V > INT_MAX/m_cellV)diffU64V=INT_MAX/m_cellV;
        dy = (int)(diffU64V*m_cellV);
      } else {
        UINT64 diffU64V = m_u64V - oldU64V;
        if(diffU64V > INT_MAX/m_cellV)diffU64V=INT_MAX/m_cellV;
        dy = -(int)(diffU64V*m_cellV);
      }
      pT->ScrollWindowEx(0, dy, m_uScrollFlags);
      ATLTRACE("SetScrollPos - x=0, y=%d, Dy: %d\n", m_ptOffset.y, dy);
      return;
    }
    CScrollImpl< T >::DoScroll(nType, nScrollCode, cxyOffset, cxySizeAll, cxySizePage, cxySizeLine);
  }

  bool AdjustScrollOffsetU64V(long &x, UINT64 &yVS)
  {
    long xOld = x;
    UINT64 yOldVS = yVS;

    long xMax = m_sizeAll.cx - m_sizeClient.cx;
    if(xMax<0)x=xMax=0;
    else if(x < 0)x = 0;
    else if(x > xMax)x = xMax;

    UINT64 yMaxVS = m_u64VMax - m_sizeClient.cy;
    if(yVS > yMaxVS)yVS = yMaxVS;

    return (xOld!=x || yOldVS!=yVS);
  }

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
		//bHandled = FALSE;

    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));

    m_sizeClient.cx = GET_X_LPARAM(lParam);
    m_sizeClient.cy = GET_Y_LPARAM(lParam) / m_cellV / m_step;

    {
      SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE };
      si.nPage = m_sizeClient.cx;
      pT->SetScrollInfo(SB_HORZ, &si, TRUE);
      si.nPage = m_sizeClient.cy;
      pT->SetScrollInfo(SB_VERT, &si, TRUE);
    }

    long x = m_ptOffset.x;
    UINT64 yVS = m_u64V;
    if(AdjustScrollOffsetU64V(x, yVS))
    {
			if(yVS!=m_u64V)
      {
        ATLTRACE("OnSize y: %016I64X -> %016I64X\n", m_u64V, yVS);
        m_u64V = yVS;
        pT->DoScroll(SB_VERT, SB_UPDATEFROMPARAM, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
      }
      if(x!=m_ptOffset.x) {
        ATLTRACE("OnSize x: %ld -> %ld\n", m_ptOffset.x, x);
        pT->ScrollWindowEx(x-m_ptOffset.x, 0, 0);
        {
          SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS };
          si.nPos = x;
          pT->SetScrollInfo(SB_HORZ, &si, FALSE);
        }
        m_ptOffset.x = x;
      }
    }

    return 0;
  }

  LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    int zDelta = (int)GET_WHEEL_DELTA_WPARAM(wParam);
    DoScroll(SB_VERT, zDelta > 0 ? SB_WHEELUP : SB_WHEELDOWN, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
    return 0;
  }
};

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CTamaScrollWindowU64VImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >, public CTamaScrollU64VImpl< T >
{
public:
  BEGIN_MSG_MAP(CTamaScrollWindowU64VImpl)
    MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
    MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
    MESSAGE_HANDLER(WM_MOUSEWHEEL, CTamaScrollU64VImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
    MESSAGE_HANDLER(m_uMsgMouseWheel, CTamaScrollU64VImpl< T >::OnMouseWheel)
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
    MESSAGE_HANDLER(WM_MOUSEHWHEEL, CScrollImpl< T >::OnMouseHWheel)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
    MESSAGE_HANDLER(WM_SIZE, CTamaScrollU64VImpl< T >::OnSize)
    MESSAGE_HANDLER(WM_PAINT, CScrollImpl< T >::OnPaint)
   // CHAIN_MSG_MAP(ATL::CWindowImpl< T, TBase, TWinTraits >)
  END_MSG_MAP()
};