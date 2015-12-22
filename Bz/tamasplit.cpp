
#include "StdAfx.h"
#include "BZView.h"

void CTamaSplitterWindow::OnSetFocus(CWindow wndOld)
{
  CBZCoreData *pCoreData = CBZCoreData::GetInstance();
  if(pCoreData)pCoreData->SetFocus2ActiveBZView();
}
