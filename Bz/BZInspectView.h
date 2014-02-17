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

class CMainFrame;

class CBZInspectView : public CDialogImpl<CBZInspectView>, public WTL::CWinDataExchange<CBZInspectView>
{
public:
  CBZInspectView() : m_bSigned(true) { }
  virtual ~CBZInspectView() { }

	enum { IDD = IDD_BZINSPECTVIEW };

  BEGIN_MSG_MAP(CBZInspectView)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_INITDIALOG(OnInitDialog)
    COMMAND_ID_HANDLER_EX(IDC_INS_INTEL, OnBnClickedInsIntel)
    COMMAND_ID_HANDLER_EX(IDC_INS_SIGNED, OnBnClickedInsSigned)
  END_MSG_MAP()

  BEGIN_DDX_MAP(CBZInspectView)
    DDX_CONTROL_HANDLE(IDE_INS_HEX, m_editHex);
    DDX_CONTROL_HANDLE(IDE_INS_8BITS, m_edit8bits);
    DDX_CONTROL_HANDLE(IDE_INS_BINARY1, m_editBinary1);
    DDX_CONTROL_HANDLE(IDE_INS_BINARY2, m_editBinary2);
    DDX_CONTROL_HANDLE(IDE_INS_BINARY4, m_editBinary4);
    DDX_CONTROL_HANDLE(IDE_INS_BINARY8, m_editBinary8);
    DDX_CONTROL_HANDLE(IDE_INS_FLOAT, m_editFloat);
    DDX_CONTROL_HANDLE(IDE_INS_DOUBLE, m_editDouble);
    DDX_CONTROL_HANDLE(IDC_INS_INTEL, m_check_intel);
    DDX_CONTROL_HANDLE(IDC_INS_SIGNED, m_check_signed);
    DDX_CONTROL_HANDLE(IDC_INS_STATIC1, m_staticBinary1);
    DDX_CONTROL_HANDLE(IDC_INS_STATIC2, m_staticBinary2);
    DDX_CONTROL_HANDLE(IDC_INS_STATIC4, m_staticBinary4);
    DDX_CONTROL_HANDLE(IDC_INS_STATIC8, m_staticBinary8);
  END_DDX_MAP()

private:
	bool m_bSigned;
	CBZView* m_pView;
public:
  WTL::CEdit m_editHex;
	WTL::CEdit m_edit8bits;
	WTL::CEdit m_editBinary1;
	WTL::CEdit m_editBinary2;
	WTL::CEdit m_editBinary4;
	WTL::CEdit m_editBinary8;
	WTL::CEdit m_editFloat;
	WTL::CEdit m_editDouble;
	WTL::CButton m_check_intel;
	WTL::CButton m_check_signed;
	WTL::CStatic m_staticBinary1;
	WTL::CStatic m_staticBinary2;
	WTL::CStatic m_staticBinary4;
	WTL::CStatic m_staticBinary8;

  CBZView* GetBZView()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZViewFromSubView(this);
  }
  CBZDoc2* GetBZDoc2()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetBZDoc2FromSubView(this);
  }
  CTamaSplitterWindow* GetSplitter()
  {
    CBZCoreData *pCoreData = CBZCoreData::GetInstance();
    return pCoreData->GetSplitterWnd();
  }


  int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    if(options.xSplitStruct == 0)
      options.xSplitStruct = lpCreateStruct->cx;

    return 0;
  }

  BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
  {
    DoDataExchange(DDX_LOAD);

    m_check_intel.SetCheck(!options.bByteOrder);
    m_check_signed.SetCheck(m_bSigned);
    _UpdateChecks();

    m_pView = GetBZView();

    return TRUE;
  }

  void OnBnClickedInsIntel(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    options.bByteOrder = !options.bByteOrder;
    UpdateChecks();
    Update();
  }

  void OnBnClickedInsSigned(UINT uNotifyCode, int nID, CWindow wndCtl)
  {
    m_bSigned = !m_bSigned;
    _UpdateChecks();

    if(m_bSigned)
    {
      m_staticBinary1.SetWindowText(_T("char"));
      m_staticBinary2.SetWindowText(_T("short"));
      m_staticBinary4.SetWindowText(_T("int"));
      m_staticBinary8.SetWindowText(_T("int64"));
    } else {
      m_staticBinary1.SetWindowText(_T("uchar"));
      m_staticBinary2.SetWindowText(_T("ushort"));
      m_staticBinary4.SetWindowText(_T("uint"));
      m_staticBinary8.SetWindowText(_T("uint64"));
    }
    Update();
  }

  void ClearAll(void)
  {
    m_edit8bits.SetWindowText(_T(""));
    m_editHex.SetWindowText(_T(""));
    m_editBinary1.SetWindowText(_T(""));
    m_editBinary2.SetWindowText(_T(""));
    m_editBinary4.SetWindowText(_T(""));
    m_editBinary8.SetWindowText(_T(""));
    m_editFloat.SetWindowText(_T(""));
    m_editDouble.SetWindowText(_T(""));
  }

  CString BYTE2BitsCString(BYTE d)
  {
    CString str;

    for(unsigned char mask=0x80;mask!=0;mask>>=1)
      str.AppendChar(d&mask ? '1' : '0');
    return str;
  }

  void Update(void)
  {
    int val;
    CString strVal;

    //	val = m_pView->GetValue(m_pView->m_dwCaret, 4);
    //	void *pVal = &val;

    ULONGLONG qval = m_pView->GetValue64(m_pView->m_dwCaret);
    void *pVal = &qval;

    val = m_pView->GetValue(m_pView->m_dwCaret, 1);
    strVal = SeparateByComma(m_bSigned ? (int)(char)val : val, m_bSigned);
    m_editBinary1.SetWindowText(strVal);

    val = m_pView->GetValue(m_pView->m_dwCaret, 2);
    strVal = SeparateByComma(m_bSigned ? (int)(short)val : val, m_bSigned);
    m_editBinary2.SetWindowText(strVal);

    val = m_pView->GetValue(m_pView->m_dwCaret, 4);
    strVal = SeparateByComma(val, m_bSigned);
    m_editBinary4.SetWindowText(strVal);

    strVal = SeparateByComma64(qval, m_bSigned);
    m_editBinary8.SetWindowText(strVal);

    val = m_pView->GetValue(m_pView->m_dwCaret, 4);
    float ft = *((float*)&val);
    strVal.Format(_T("%f"), ft);
    m_editFloat.SetWindowText(strVal);

    strVal.Format(_T("%f"), *((double*)&qval));
    m_editDouble.SetWindowText(strVal);

    strVal.Format(_T("0x%08X %08X"), *( ((int*)pVal)+1 ), *( ((int*)pVal)+0 ));
    //	strVal.Format("0x%016I64X", qval);
    m_editHex.SetWindowText(strVal);

    CString str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+7 ));
    strVal = str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+6 ));
    strVal += str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+5 ));
    strVal += str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+4 ));
    strVal += str8bits + _T(" - ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+3 ));
    strVal += str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+2 ));
    strVal += str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+1 ));
    strVal += str8bits + _T(" ");
    str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+0 ));
    strVal += str8bits;
    m_edit8bits.SetWindowText(strVal);
  }

  void UpdateChecks(void);


  void _UpdateChecks(void)
  {
    m_check_intel.SetCheck(!options.bByteOrder);
    m_check_signed.SetCheck(m_bSigned);
  }
};


