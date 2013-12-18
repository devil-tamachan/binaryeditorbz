
template <class T>
class CTamaScrollU64VImpl : public WTL::CScrollImpl< T >
{
public:
  DWORD m_lowV;
  DWORD m_step;
  UINT64 m_u64V; // == m_lowV + si.nPos*m_step
  UINT64 m_u64VMax; // ==10 (m_u64V == [0:10])

  DWORD m_cellV;
  
  CTamaScrollU64VImpl()
  {
    m_lowV = 0;
    m_step = UINT_MAX;
    m_u64V = 0;
    m_u64VMax = 0;

    m_cellV = 1;
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
  
 /* void SetScrollPosU64V(UINT64 newU64V, BOOL bRedraw = TRUE)
  {
    m_u64V = newY64;
    int oldOffsetY = m_ptOffset.y;
    UpdateFromU64V();
    
    if(m_ptOffset.y != oldOffsetY)
    {
      T* pT = static_cast<T*>(this);
      ATLASSERT(::IsWindow(pT->m_hWnd));
      SCROLLINFO si = {sizeof(SCROLLINFO)};
      si.fMask = SIF_POS;
      si.nPos = m_ptOffset.y;
      pT->SetScrollInfo(SB_VERT, &si, bRedraw);
    }
    if(bRedraw)pT->Invalidate();
  }*/
  
  void AdjustFromU64VMax(BOOL bRedraw = TRUE)
  {
    {//step値を決定
      if(m_u64VMax <= INT_MAX)
      {
        m_step = 1;
        m_sizeAll.cy = (int)m_u64VMax;
      } else {
        m_step = (DWORD)(m_u64VMax / (INT_MAX-1));
        if(m_u64VMax % (INT_MAX-1))m_step++;
        CalcScrolVMax();
        (m_sizeAll.cy)+=1;
      }
    }
    if(m_u64V > m_u64VMax)m_u64V = m_u64VMax;
    UpdateFromU64V();
  }
  
  void UpdateFromU64V()
  {
    ATLTRACE("UpdateFromU64V +++ m_lowV:0x%08X, m_ptOffset.y:%d, m_sizeAll.cy:%d\n", m_lowV, m_ptOffset.y, m_sizeAll.cy);
    {//Vの補助値(下位DWORD)を決定
      m_lowV = (DWORD)(m_u64V % m_step);
    }
    {//Windowsへ渡すスクロールバーの値(int)決定
      m_ptOffset.y = ConvUINT64_ScrlI32V(m_u64V);
      if(m_ptOffset.y > m_sizeAll.cy-(m_sizePage.cy/(int)m_cellV))m_ptOffset.y = m_sizeAll.cy-(m_sizePage.cy/(int)m_cellV);
    }
    ATLTRACE("UpdateFromU64V --- m_lowV:0x%08X, m_ptOffset.y:%d, m_sizeAll.cy:%d\n", m_lowV, m_ptOffset.y, m_sizeAll.cy);
  }

  int CalcScrolVMax()
  {
    m_sizeAll.cy = ConvUINT64_ScrlI32V(m_u64VMax);
    if(m_u64VMax % m_step) (m_sizeAll.cy)++;
    return m_sizeAll.cy;
  }

  int ConvUINT64_ScrlI32V(UINT64 u64V)
  {
    INT64 newVU64 = (u64V / m_step);
    newVU64 += (INT64)0;//INT_MIN;
    return (int)newVU64;
  }
  
  void UpdateFromRawPos()
  {
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_TRACKPOS };
    T* pT = static_cast<T*>(this);
    ATLASSERT(::IsWindow(pT->m_hWnd));
    if(pT->GetScrollInfo(SB_VERT, &si))
    {
      if(si.nTrackPos == m_ptOffset.y)return;
      m_ptOffset.y = si.nTrackPos;
      m_lowV = 0;
      m_u64V = ConvOffsetI32_U64(m_ptOffset.y) * m_step;
      ATLTRACE("UpdateFromRawPos: %d\n", si.nTrackPos);
    }
  }
  
  UINT64 ConvOffsetI32_U64(int pos)
  {
    //ATLTRACE("ConvOffsetI32_U32: %d -> %u\n", pos, ConvOffsetI32_U32(pos));
    return ConvOffsetI32_U32(pos);
  }
  unsigned int ConvOffsetI32_U32(int pos)
  {
      INT64 pos64 = pos;
      pos64 -= (INT64)0;//INT_MIN;
      return (unsigned int)pos64;
  }
  
  UINT64 GetScrollPosU64V()
  {
    return m_u64V;
  }

#ifdef _DEBUG
  void TRACEParams()
  {
    ATLTRACE("m_lowV:0x%08X, m_step:0x%08X, m_u64V:0x%016I64X, m_u64VMax:0x%016I64X, m_ptOffset.y:%d (0x%08X), m_sizeAll.cy:%d, m_sizePage:%d, m_sizeClient:%d\n", m_lowV, m_step, m_u64V, m_u64VMax, m_ptOffset.y, m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeClient.cy);
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
      case SB_THUMBTRACK:
      case SB_THUMBPOSITION:
        UpdateFromRawPos();
        if(m_ptOffset.y == m_sizeAll.cy)
        {
          m_u64V = m_u64VMax;
          UpdateFromU64V();
        }
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
    MESSAGE_HANDLER(WM_MOUSEWHEEL, CScrollImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
    MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif // !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
    MESSAGE_HANDLER(WM_MOUSEHWHEEL, CScrollImpl< T >::OnMouseHWheel)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
    MESSAGE_HANDLER(WM_SIZE, CTamaScrollU64VImpl< T >::OnSize)
    MESSAGE_HANDLER(WM_PAINT, CScrollImpl< T >::OnPaint)
   // CHAIN_MSG_MAP(ATL::CWindowImpl< T, TBase, TWinTraits >)
  END_MSG_MAP()
};