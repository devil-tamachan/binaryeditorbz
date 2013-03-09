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

#define MAX_COLORS 16

static COLORREF colors[MAX_COLORS] = {
	RGB(0, 0, 0), RGB(255, 255, 255), RGB(128, 0, 0), RGB(0, 128, 0),
	RGB(128, 128, 0), RGB(0, 0, 128), RGB(128, 0, 128), RGB(0, 128, 128),
	RGB(192, 192, 192), RGB(128, 128, 128), RGB(255, 0, 0), RGB(0, 255, 0),
	RGB(255, 255, 0), RGB(0, 0, 255), RGB(255, 0, 255), RGB(0, 255, 255),
};

class CColorCombo : public CWindowImpl<CColorCombo, WTL::CComboBox>, public WTL::COwnerDraw<CColorCombo>
{
	DECLARE_WND_SUPERCLASS(_T("ColorCombo"), _T("COMBOBOX"))

	BEGIN_MSG_MAP(CColorCombo)
		CHAIN_MSG_MAP(WTL::COwnerDraw<CColorCombo>)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

public:
	BOOL m_bAuto;
	BOOL m_bOther;
	COLORREF m_rgbAuto;
	COLORREF m_rgbOther;

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DefWindowProc();
		COMBOBOXINFO cmbInfo = {0};
		cmbInfo.cbSize = sizeof(COMBOBOXINFO);
		GetComboBoxInfo(&cmbInfo);
		CRect rect(cmbInfo.rcItem);
		InvalidateRect(rect, 0);
		WTL::CPaintDC dc(m_hWnd);
		_DrawItem(GetCurSel(), dc, rect, FALSE);

		return 0;
    }
	BOOL OnEraseBkgnd(WTL::CDCHandle dc)
	{
		return TRUE;
	}

	CColorCombo() : CWindowImpl<CColorCombo, WTL::CComboBox>()
	{
		m_bAuto = FALSE;
		m_bOther = TRUE;
		m_rgbOther = RGB_WHITE;
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		if(!CWindowImpl<CColorCombo, WTL::CComboBox>::SubclassWindow(hWnd))return FALSE;
		LONG curSetting = GetWindowLong(GWL_STYLE);
		SetWindowLong(GWL_STYLE, curSetting | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS);
		for_to(i, MAX_COLORS + m_bOther)AddString("");
		SetCurSel(0);
		return TRUE;
	}

	void DrawItem(LPDRAWITEMSTRUCT lpdis)
	{
		CRect rcItem(&(lpdis->rcItem));
		_DrawItem(lpdis->itemID, lpdis->hDC, rcItem, lpdis->itemState & ODS_SELECTED);
	}

	void _DrawItem(int iItem, HDC hdc, CRect &rcItem, BOOL bSelected)
	{
		WTL::CDCHandle dc(hdc);
		dc.FillSolidRect(rcItem, RGB(255, 255, 255));

		COLORREF rgb;
		int mode = FALSE;
		if(iItem < MAX_COLORS)
			rgb = colors[iItem];
		else if(m_bOther && iItem == MAX_COLORS) {
			mode = INVALID;
			rgb = m_rgbOther;
		} else {
			mode = TRUE;
			rgb = GetSystemColor(m_rgbAuto);
		}

		WTL::CBrush brush;
		brush.CreateSolidBrush(rgb);
		HBRUSH pOldBrush = dc.SelectBrush(brush);
		rcItem.InflateRect(-2, -2);
		dc.Rectangle(rcItem);
		rcItem.InflateRect(2, 2);
		dc.SelectBrush(pOldBrush);

		if(mode) {
			CString sLabel;
			sLabel.LoadString(mode > 0 ? IDS_AUTOCOLOR : IDS_OTHERCOLOR);
			dc.SetBkMode(TRANSPARENT);
			WTL::CFont font;
			font.CreatePointFont(90, "");
			dc.SelectFont(font);
			dc.SetTextColor(rgb ^ 0xFFFFFF);
			dc.DrawText(sLabel, (int)sLabel.GetLength(), rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_EXTERNALLEADING);
		}

		if (bSelected)
			dc.DrawFocusRect(rcItem);
	}
	
	void SetSelColor(COLORREF rgb, COLORREF rgbDefault)
	{
		BOOL bAuto = IsSystemColor(rgbDefault);
		if(bAuto) {
			if(!m_bAuto)
				AddString("");
			m_rgbAuto = rgbDefault;
			if(IsSystemColor(rgb)) {
				SetCurSel(MAX_COLORS + m_bOther);
				m_bAuto = bAuto;
				return;
			}
		} else {
			if(m_bAuto)
				DeleteString(MAX_COLORS + m_bOther);
		}
		m_bAuto = bAuto;
		for_to(i, MAX_COLORS) {
			if(rgb == colors[i]) {
				SetCurSel(i);
				m_rgbOther = RGB_WHITE;
				return;
			}
		}
		if(m_bOther) {
			m_rgbOther = rgb;
			SetCurSel(MAX_COLORS);
		}
	}

	COLORREF GetSelColor()
	{
		int iSel = GetCurSel();
		if(m_bOther && iSel == MAX_COLORS) {
			CColorDialog dlgColor(m_rgbOther);
			if(dlgColor.DoModal() == IDOK) {
				m_rgbOther = dlgColor.GetColor();
				Invalidate();
			}
			return dlgColor.GetColor();
		}
		if(iSel == MAX_COLORS + m_bOther) return m_rgbAuto;
		return colors[iSel];
	}
};
