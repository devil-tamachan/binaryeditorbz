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

class CMiniToolbarView : public CDialogImpl<CMiniToolbarView>, public WTL::CDialogResize<CMiniToolbarView>, public WTL::CWinDataExchange<CMiniToolbarView>
{
public:
  enum { IDD = IDD_MINITOOLBAR };

  BEGIN_DLGRESIZE_MAP(CMiniToolbarView)
    DLGRESIZE_CONTROL(IDE_MINI_PATH, DLSZ_SIZE_X | DLSZ_REPAINT)
    DLGRESIZE_CONTROL(IDC_MINI_READONLY, DLSZ_MOVE_X | DLSZ_REPAINT)
    DLGRESIZE_CONTROL(IDC_MINI_ENCODING, DLSZ_MOVE_X | DLSZ_REPAINT)
  END_DLGRESIZE_MAP()

  BEGIN_MSG_MAP(CMiniToolbarView)
    //MSG_WM_ERASEBKGND(OnEraseBkGnd)
    COMMAND_HANDLER_EX(IDC_MINI_ENCODING, CBN_SELCHANGE, OnCbnSelChangeEncoding)
    COMMAND_ID_HANDLER_EX(IDC_MINI_READONLY, OnCheckReadOnly)
    MSG_WM_INITDIALOG(OnInitDialog)
    CHAIN_MSG_MAP(CDialogResize<CMiniToolbarView>)
  END_MSG_MAP()

  BEGIN_DDX_MAP(CSetupColorDialog)
		DDX_CHECK(IDC_MINI_READONLY, m_bReadOnly);
    DDX_CONTROL_HANDLE(IDC_MINI_ENCODING, m_combo_encoding);
  END_DDX_MAP()

public:
  WTL::CComboBox m_combo_encoding;
	BOOL	m_bReadOnly;

  CMiniToolbarView()
  {
  }

  void OnCheckReadOnly(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
		DoDataExchange(DDX_SAVE);
    CBZDoc2 *pBZDoc = GetBZDoc2();
    if(!pBZDoc)return;

    pBZDoc->ToggleReadOnly();
    
    SetFocus2ActiveBZView();
  }

  void OnCbnSelChangeEncoding(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    CBZView *pBZView = GetBZView();
    if(!pBZView)return;

    pBZView->ChangeCharSet((CharSet)(m_combo_encoding.GetCurSel()));

    SetFocus2ActiveBZView();
  }

	BOOL OnEraseBkGnd(WTL::CDCHandle dc)
	{
    WTL::CRect rect;
		dc.GetClipBox(&rect);
		dc.FillRect(rect, COLOR_BTNFACE);
		return TRUE;
	}


  BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
  {
		DoDataExchange(DDX_LOAD);
    DlgResize_Init(false, false);

    m_combo_encoding.AddString(_T("ASCII"));
    m_combo_encoding.AddString(_T("S-JIS"));
    m_combo_encoding.AddString(_T("UTF-16"));
    m_combo_encoding.AddString(_T("JIS"));
    m_combo_encoding.AddString(_T("EUC"));
    m_combo_encoding.AddString(_T("UTF-8"));
    m_combo_encoding.AddString(_T("EBCDIC"));
    m_combo_encoding.AddString(_T("EPWING"));
    m_combo_encoding.SetCurSel(0);

    return TRUE;
  }

  void SetFilePath(LPCTSTR path)
  {
    ::SetWindowText(GetDlgItem(IDE_MINI_PATH), path);
  }

  void SetReadOnly(BOOL bReadOnly)
  {
    CheckDlgButton(IDC_MINI_READONLY, bReadOnly?BST_CHECKED:BST_UNCHECKED);
  }

  void SetEncoding(int encoding)
  {
    m_combo_encoding.SetCurSel(encoding);
  }

  void Update()
  {
    CBZView *pBZView = GetBZView();
    if(!pBZView)return;

    SetFilePath(pBZView->m_pDoc->GetFilePath());
    SetReadOnly(pBZView->m_pDoc->IsReadOnly());
    SetEncoding(pBZView->m_charset);
  }

  CBZView* GetBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZViewFromMiniToolbar(this);
  }

  void SetFocus2ActiveBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    if(pCoreData)pCoreData->SetFocus2ActiveBZView();
  }

  CBZDoc2* GetBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZDoc2FromMiniToolbar(this);
  }
};

