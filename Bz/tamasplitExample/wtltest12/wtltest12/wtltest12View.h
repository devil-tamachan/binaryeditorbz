// wtltest12View.h : interface of the CWtltest12View class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CWtltest12View : public CDialogImpl<CWtltest12View>, public CScrollImpl<CWtltest12View>
{
public:
	enum { IDD = IDD_WTLTEST12_FORM };

/*	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}*/

	BEGIN_MSG_MAP(CWtltest12View)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CScrollImpl<CWtltest12View>)
		CHAIN_MSG_MAP_ALT(CScrollImpl<CWtltest12View>, 1)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		RECT rc;
		GetClientRect(&rc);
		SetScrollSize(rc.right, rc.bottom);

		return 0;
	}
	
	void DoPaint(CDCHandle /*dc*/)
	{
	}

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
};
