// options.h : interface of the COptions class
//

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptions
{
public:
	COptions();
	void Touch() { m_bModified = TRUE; }
	UINT GetProfileInt(LPCTSTR lpszEntry, int nDefault);
	BOOL WriteProfileInt(LPCTSTR lpszEntry, int nValue);
	CString GetProfileString(LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	BOOL WriteProfileString(LPCTSTR lpszEntry, LPCTSTR lpszValue);
	BOOL GetProfileBinary(LPCTSTR lpszEntry, LPVOID pData);
	BOOL GetProfileBinary2(LPCTSTR lpszEntry, LPVOID pData, unsigned int dstSize);
	BOOL WriteProfileBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	void LoadHistory(CString sHistName, CStringArray& sarrHist);
	void SaveHistory(CString sHistName, CStringArray& sarrHist);
private:
	BOOL m_bModified;
};

#endif //_OPTIONS_H_
