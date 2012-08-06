// ProgressDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "BZ.h"
#include "ProgressDialog.h"


// CProgressDialog ダイアログ

IMPLEMENT_DYNAMIC(CProgressDialog, CDialog)

CProgressDialog::CProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDialog::IDD, pParent)
{

}

CProgressDialog::~CProgressDialog()
{
}

void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProgressDialog, CDialog)
END_MESSAGE_MAP()


// CProgressDialog メッセージ ハンドラ
