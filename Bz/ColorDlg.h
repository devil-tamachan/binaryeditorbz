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


#include "ColorCombo.h"

#define IDS_LABEL_START  IDS_COLOR_ADDRESS

extern COLORREF colorsDefault[TCOLOR_COUNT][2];
COLORREF GetSystemColor(COLORREF rgb);



class CSetupColorDialog : public CDialogImpl<CSetupColorDialog>, public WTL::CWinDataExchange<CSetupColorDialog>
{
public:
	enum { IDD = IDD_SETUP_COLOR };

	CSetupColorDialog() : CDialogImpl<CSetupColorDialog>()
	{
		m_pSampleFont = NULL;
	}

	WTL::CListBox	m_listParts;
	CColorCombo	m_cbTextColor;
	CColorCombo	m_cbBackColor;
private:
	CBrush m_brush;

public:
	COLORREF m_colors[TCOLOR_COUNT][2];
	CFont* m_pSampleFont;

	BEGIN_MSG_MAP(CSetupColorDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDAPPLY, OnApply)
		COMMAND_HANDLER_EX(IDC_PARTSLIST, LBN_SELCHANGE, OnSelchangePartslist)
		//ON_WM_CTLCOLOR()
		MESSAGE_HANDLER_EX(WM_CTLCOLORSTATIC, OnCtlColor)
		COMMAND_HANDLER_EX(IDC_TEXTCOLOR, CBN_SELENDOK, OnSelEndOkTextColor)
		COMMAND_HANDLER_EX(IDC_BACKCOLOR, CBN_SELENDOK, OnSelEndOkBackColor)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CSetupColorDialog)
		DDX_CONTROL_HANDLE(IDC_PARTSLIST, m_listParts);
	END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		memcpy(m_colors, options.colors, sizeof(m_colors));

		DoDataExchange(DDX_LOAD);

		CString sLabel;
		for_to(i, TCOLOR_COUNT) {
			sLabel.LoadString(IDS_LABEL_START + i);
			m_listParts.AddString(sLabel);
		}
		m_listParts.SetCurSel(0);
		m_cbTextColor.SubclassWindow(::GetDlgItem(m_hWnd, IDC_TEXTCOLOR));
		m_cbBackColor.SubclassWindow(::GetDlgItem(m_hWnd, IDC_BACKCOLOR));
		_OnSelchangePartslist();

		return TRUE;
	}

	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DoDataExchange(DDX_SAVE);
		memcpy(options.colors, m_colors, sizeof(m_colors));
		EndDialog(nID);
	}
	void OnApply(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		memcpy(options.colors, m_colors, sizeof(m_colors));
		AfxGetMainWnd()->Invalidate(TRUE);
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}

	void OnSelchangePartslist(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		_OnSelchangePartslist();
	}

	void _OnSelchangePartslist()
	{
		// TODO: Add your control notification handler code here
		int iSel = 	m_listParts.GetCurSel();
		if(iSel == LB_ERR) return;

		m_cbTextColor.SetSelColor(m_colors[iSel][0], colorsDefault[iSel][0]);
		m_cbBackColor.SetSelColor(m_colors[iSel][1], colorsDefault[iSel][1]);
		::InvalidateRect(GetDlgItem(IDC_SAMPLE), NULL, TRUE);
	}

	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam)
	//HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{
		HBRUSH hbr;
		WTL::CDCHandle dc((HDC)wParam);

		if((HWND)lParam == GetDlgItem(IDC_SAMPLE))
		{
			int iSel = 	m_listParts.GetCurSel();
			if(iSel != LB_ERR) {
				COLORREF rgbText = GetSystemColor(m_colors[iSel][0]);
				COLORREF rgbBack = GetSystemColor(m_colors[iSel][1]);
				COLORREF rgbBG = GetSystemColor(m_colors[TCOLOR_TEXT][1]);
				if(m_brush.m_hObject)
					m_brush.DeleteObject();
				m_brush.CreateSolidBrush(rgbBG);
				hbr = (HBRUSH)m_brush;
				dc.SetTextColor(rgbText);
				dc.SetBkColor(rgbBack);
				dc.SelectFont(*m_pSampleFont);
			}
			return (LRESULT)hbr;
		}

		SetMsgHandled(FALSE);
		return 0;
	}
	void OnSelEndOkTextColor(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		int iSel = 	m_listParts.GetCurSel();
		m_colors[iSel][0] = m_cbTextColor.GetSelColor();
		m_cbTextColor.Invalidate();
		::InvalidateRect(GetDlgItem(IDC_SAMPLE), NULL, TRUE);
	}

	void OnSelEndOkBackColor(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		int iSel = 	m_listParts.GetCurSel();
		m_colors[iSel][1] = m_cbBackColor.GetSelColor();
		m_cbBackColor.Invalidate();
		::InvalidateRect(GetDlgItem(IDC_SAMPLE), NULL, TRUE);
	}
};
