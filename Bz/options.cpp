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
#include "options.h"

static const TCHAR sRegDefault[] = _T("Settings");
static const TCHAR sRegHistory[] = _T("History");

COptions::COptions()
{
	m_bModified = FALSE;
}

UINT COptions::GetProfileInt(LPCTSTR lpszEntry, int nDefault)
{
	return AfxGetApp()->GetProfileInt(sRegDefault, lpszEntry, nDefault);
}

BOOL COptions::WriteProfileInt(LPCTSTR lpszEntry, int nValue)
{
	return AfxGetApp()->WriteProfileInt(sRegDefault, lpszEntry, nValue);
}

CString COptions::GetProfileString(LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	return AfxGetApp()->GetProfileString(sRegDefault, lpszEntry, lpszDefault);
}

BOOL COptions::WriteProfileString(LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	return AfxGetApp()->WriteProfileString(sRegDefault, lpszEntry, lpszValue);
}

BOOL COptions::GetProfileBinary(LPCTSTR lpszEntry, LPVOID pData)
{
	LPBYTE pBuffer;
	UINT nBytes;
	if(AfxGetApp()->GetProfileBinary(sRegDefault, lpszEntry, &pBuffer, &nBytes)) {
		memcpy(pData, pBuffer, nBytes);
		delete pBuffer;
		return TRUE;
	}
	return FALSE;
}

BOOL COptions::GetProfileBinary2(LPCTSTR lpszEntry, LPVOID pData, unsigned int dstSize)
{
	LPBYTE pBuffer;
	UINT nBytes;
	if(AfxGetApp()->GetProfileBinary(sRegDefault, lpszEntry, &pBuffer, &nBytes)) {
		memcpy(pData, pBuffer, min(nBytes, dstSize));
		delete pBuffer;
		return TRUE;
	}
	return FALSE;
}

BOOL COptions::WriteProfileBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	return AfxGetApp()->WriteProfileBinary(sRegDefault, lpszEntry, pData, nBytes);
}
