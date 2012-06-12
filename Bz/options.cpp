// options.cpp : implementation of the COptions class
//

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
