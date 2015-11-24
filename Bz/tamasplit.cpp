
#include "StdAfx.h"
#include "BZView.h"

void CTamaSplitterWindow::OnSetFocus(CWindow wndOld)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  if(pCoreData==NULL)return;
  CBZView* pBZView = pCoreData->GetActiveBZView();
  if(pBZView)::SetFocus(pBZView->m_hWnd);
}
