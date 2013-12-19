// TamaScrl64TestView.h : interface of the CTamaScrl64TestView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CTamaScrl64TestView : public CTamaScrollWindowU64VImpl<CTamaScrl64TestView>
{
public:
	DECLARE_WND_CLASS(NULL)

  LRESULT OnCreate(LPCREATESTRUCT lpcs)
  {
		SetMsgHandled(false);
    m_cellV = 18;
		SetScrollSizeU64V(5000, _UI64_MAX/16);
    SetScrollPage(1,8);
    SetScrollLine(1,1);
    return 0;
  }

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

  void DoPaint(WTL::CDCHandle dc)
	{
    //Invalidate(false);
    CString str;
    WTL::CBrush whiteBrush;
    whiteBrush.CreateSolidBrush(RGB(0, 255, 255));
    CRect clip;
    dc.GetClipBox(clip);
    dc.FillRect(clip,whiteBrush);
    CPoint pt;
    GetScrollOffset(pt);
    UINT64 v = GetScrollPosU64V();
    for(int i=0;i<50;i++)
    {
      str.Format(_T("0x%016I64X, step==0x%08X, m_u64V==%I64u (0x%016I64X), m_u64VMax==%I64u (0x%016I64X)"), v+i, m_step, m_u64V, m_u64V, m_u64VMax, m_u64VMax);
      dc.TextOut(0, i*18+pt.y, str, str.GetLength());
    }
	}

	BEGIN_MSG_MAP(CTamaScrl64TestView)
    MSG_WM_CREATE(OnCreate)
		CHAIN_MSG_MAP(CTamaScrollWindowU64VImpl<CTamaScrl64TestView>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
};
