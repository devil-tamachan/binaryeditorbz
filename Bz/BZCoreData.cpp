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


#include "stdafx.h"
#include "BZCoreData.h"
#include "BZView.h"
#include "MainFrm.h"
#include "BZDoc2.h"

//class CBZDoc2;
class CBZView;
//class CMainFrame;

void CBZCoreData::DeleteView(DWORD dwIndex, BOOL bDelDoc)
{
  DeleteSubView(dwIndex);
  if(m_arrView.GetSize()>dwIndex)
  {
    CBZView *pBZView = m_arrView[dwIndex];
    pBZView->DestroyWindow();
    delete pBZView;
    m_arrView.RemoveAt(dwIndex);
  }
  if(bDelDoc)
  {
    delete m_arrDoc[dwIndex];
    m_arrDoc.RemoveAt(dwIndex);
  }
  if(m_dwActive==dwIndex)m_dwActive = 0;
  else if(m_dwActive>dwIndex)m_dwActive--;
}

void CBZCoreData::DeleteSubView(DWORD dwIndex)
{
  if(m_arrSubView.GetSize()>dwIndex)
  {
    ATL::CWindow *pSubView = m_arrSubView[dwIndex];
    pSubView->DestroyWindow();
    delete *pSubView;
    m_arrSubView.RemoveAt(dwIndex);
  }
}

void CBZCoreData::ReplaceBZDoc2(int index, CBZDoc2 *doc, BOOL bDeleteDoc)
{
  CBZDoc2 *pOldDoc = GetBZDoc2(index);
  m_arrDoc.SetAtIndex(index, doc);
  if(bDeleteDoc)delete pOldDoc;
}

void CBZCoreData::Invalidate(BOOL bErase)
{
  if(m_pMainFrame)m_pMainFrame->Invalidate(bErase);
}
