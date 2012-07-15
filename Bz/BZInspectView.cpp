// BZInspectView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BZ.h"
#include "BZView.h"
#include "BZInspectView.h"


// CBZInspectView

IMPLEMENT_DYNCREATE(CBZInspectView, CFormView)

CBZInspectView::CBZInspectView()
	: CFormView(CBZInspectView::IDD)
	, m_bSigned(true)
{

}

CBZInspectView::~CBZInspectView()
{
}

void CBZInspectView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDE_INS_HEX, m_editHex);
	DDX_Control(pDX, IDE_INS_8BITS, m_edit8bits);
	DDX_Control(pDX, IDE_INS_BINARY1, m_editBinary1);
	DDX_Control(pDX, IDE_INS_BINARY2, m_editBinary2);
	DDX_Control(pDX, IDE_INS_BINARY4, m_editBinary4);
	DDX_Control(pDX, IDE_INS_BINARY8, m_editBinary8);
	DDX_Control(pDX, IDE_INS_FLOAT, m_editFloat);
	DDX_Control(pDX, IDE_INS_DOUBLE, m_editDouble);
	DDX_Control(pDX, IDC_INS_INTEL, m_check_intel);
	DDX_Control(pDX, IDC_INS_SIGNED, m_check_signed);
	DDX_Control(pDX, IDC_INS_STATIC1, m_staticBinary1);
	DDX_Control(pDX, IDC_INS_STATIC2, m_staticBinary2);
	DDX_Control(pDX, IDC_INS_STATIC4, m_staticBinary4);
	DDX_Control(pDX, IDC_INS_STATIC8, m_staticBinary8);
}

BEGIN_MESSAGE_MAP(CBZInspectView, CFormView)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_INS_INTEL, &CBZInspectView::OnBnClickedInsIntel)
	ON_BN_CLICKED(IDC_INS_SIGNED, &CBZInspectView::OnBnClickedInsSigned)
END_MESSAGE_MAP()


// CBZInspectView 診断

#ifdef _DEBUG
void CBZInspectView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBZInspectView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBZInspectView メッセージ ハンドラ

int CBZInspectView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(options.xSplitStruct == 0)
		options.xSplitStruct = lpCreateStruct->cx;

	return 0;
}

void CBZInspectView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	m_check_intel.SetCheck(!options.bByteOrder);
	m_check_signed.SetCheck(m_bSigned);
	UpdateChecks();

	m_pView = (CBZView*)GetNextWindow();
}

void CBZInspectView::ClearAll(void)
{
	m_edit8bits.SetWindowText("");
	m_editHex.SetWindowText("");
	m_editBinary1.SetWindowText("");
	m_editBinary2.SetWindowText("");
	m_editBinary4.SetWindowText("");
	m_editBinary8.SetWindowText("");
	m_editFloat.SetWindowText("");
	m_editDouble.SetWindowText("");
}

void CBZInspectView::OnBnClickedInsIntel()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	options.bByteOrder = !options.bByteOrder;
	UpdateChecks();
	Update();
}

void CBZInspectView::OnBnClickedInsSigned()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_bSigned = !m_bSigned;
	UpdateChecks();

	if(m_bSigned)
	{
		m_staticBinary1.SetWindowText("char");
		m_staticBinary2.SetWindowText("short");
		m_staticBinary4.SetWindowText("int");
		m_staticBinary8.SetWindowText("int64");
	} else {
		m_staticBinary1.SetWindowText("uchar");
		m_staticBinary2.SetWindowText("ushort");
		m_staticBinary4.SetWindowText("uint");
		m_staticBinary8.SetWindowText("uint64");
	}
	Update();
}

CString BYTE2BitsCString(BYTE d)
{
	CString str;

	for(unsigned char mask=0x80;mask!=0;mask>>=1)
		str.AppendChar(d&mask ? '1' : '0');
	return str;
}

void CBZInspectView::Update(void)
{
	int val;
	CString strVal;
	
//	val = m_pView->GetValue(m_pView->m_dwCaret, 4);
//	void *pVal = &val;
	
	ULONGLONG qval = m_pView->GetValue64(m_pView->m_dwCaret);
	void *pVal = &qval;

	char chVal = *((char*)pVal);
	strVal = SeparateByComma(chVal, m_bSigned);
	m_editBinary1.SetWindowText(strVal);

	val = m_pView->GetValue(m_pView->m_dwCaret, 2);
	short sVal = *((short*)pVal);
	strVal = SeparateByComma(sVal, m_bSigned);
	m_editBinary2.SetWindowText(strVal);

	int iVal = *((int*)pVal);
	strVal = SeparateByComma(iVal, m_bSigned);
	m_editBinary4.SetWindowText(strVal);

	strVal = SeparateByComma64(qval, m_bSigned);
	m_editBinary8.SetWindowText(strVal);

	strVal.Format("%f", *((float*)pVal));
	m_editFloat.SetWindowText(strVal);

	strVal.Format("%f", qval);
	m_editDouble.SetWindowText(strVal);

	strVal.Format("0x%08X %08X", *( ((int*)pVal)+1 ), *( ((int*)pVal)+0 ));
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

void CBZInspectView::UpdateChecks(void)
{
	m_check_intel.SetCheck(!options.bByteOrder);
	m_check_signed.SetCheck(m_bSigned);
}
