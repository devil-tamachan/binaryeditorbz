#pragma once

class CSFCCache
{
private:
  DWORD m_dwCacheStart;
  DWORD m_dwCacheSize;
  DWORD m_dwCacheAllocSize;
  LPBYTE m_pCache;
  CSuperFileCon *m_pSFC;

  const DWORD CACHEMAX;

public:

#ifdef DEBUG
  CSFCCache(void) : m_pCache(NULL), m_pSFC(NULL), m_dwCacheStart(0), m_dwCacheSize(0), m_dwCacheAllocSize(0), CACHEMAX(1024*1024/*1MB*/)
  {
    ATLASSERT(FALSE);
  }
#endif

  CSFCCache(CSuperFileCon *pSFC, DWORD dwCacheAllocSize) : m_pCache(NULL), m_pSFC(pSFC), m_dwCacheStart(0), m_dwCacheSize(0), m_dwCacheAllocSize(dwCacheAllocSize), CACHEMAX(1024*1024/*1MB*/)
  {
    if(m_dwCacheAllocSize > 0)
    {
      if(m_dwCacheAllocSize > CACHEMAX)m_dwCacheAllocSize = CACHEMAX;
      m_pCache = (LPBYTE)malloc(m_dwCacheAllocSize);
      if(!m_pCache)
      {
        ATLASSERT(FALSE);
        m_dwCacheAllocSize = 0;
      }
    }
  }

  ~CSFCCache(void)
  {
    if(m_pCache)
    {
      free(m_pCache);
      m_pCache = NULL;
    }
  }

private:
  _inline LPBYTE _GetLPBYTE(DWORD dwStart, DWORD dwSize)
  {
    //if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())
    //{
    //  ATLASSERT(FALSE);
    //  return NULL;
    //}
    if(dwStart >= m_dwCacheStart && dwStart+dwSize <= m_dwCacheStart+m_dwCacheSize)
    {
      DWORD dwDiff = dwStart-m_dwCacheStart;
      return m_pCache + dwDiff;
    }
    return NULL;
  }

  _inline void _ClearInternalCacheData()
  {
    m_dwCacheStart = 0;
    m_dwCacheSize = 0;
  }

public:

  DWORD GetRemain(DWORD dwStart)
  {
    if(m_pCache && dwStart >= m_dwCacheStart)return dwStart-m_dwCacheStart;
    return 0;
  }

  BOOL Clear(DWORD dwStart = 0, DWORD dwSize = 0)
  {
    if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(dwStart==0 && dwSize==0)
    {
      _ClearInternalCacheData();
      return TRUE;
    }
    DWORD dwEnd = dwStart += dwSize - 1;
    if(dwSize==0 || dwEnd < dwStart)dwEnd = 0xFFffFFff;
    DWORD dwCacheEnd = m_dwCacheStart + m_dwCacheSize - 1;
    if(dwEnd < m_dwCacheStart || dwCacheEnd < dwStart)return FALSE;
    DWORD dwDelStart = dwStart;
    if(dwDelStart < m_dwCacheStart)dwDelStart = m_dwCacheStart;
    DWORD dwDelEnd = dwEnd;
    if(dwDelEnd > dwCacheEnd)dwDelEnd = dwCacheEnd;
    DWORD dwLeftSize = dwDelStart - m_dwCacheStart;
    DWORD dwRightSize = dwCacheEnd - dwDelEnd;
    if(dwRightSize > 0)
    {
      DWORD dwDiff = dwDelEnd + 1 - m_dwCacheStart;
      memmove(m_pCache, m_pCache + dwDiff, dwRightSize);
      m_dwCacheStart = dwDelEnd + 1;
      m_dwCacheSize = dwRightSize;
    } else if(dwLeftSize > 0) {
      m_dwCacheSize = dwLeftSize;
    } else {
      _ClearInternalCacheData();
    }
    return TRUE;
  }

  LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0)
  {
    if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())goto ERR_CACHE2;

    DWORD dwFileRemain = m_pSFC->GetRemain(dwStart);
    if(dwFileRemain==0)goto ERR_CACHE2;
    DWORD dwReadSize = dwIdealSize;
    if(dwReadSize==0)dwReadSize = min(m_dwCacheAllocSize, dwFileRemain);

    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwReadSize);
    if(pCacheTry)return pCacheTry;

    if(dwReadSize > dwFileRemain)dwReadSize = dwFileRemain;
    if(dwReadSize > m_dwCacheAllocSize)dwReadSize = m_dwCacheAllocSize;
    if(!m_pSFC->Read(m_pCache, dwStart, dwReadSize))goto ERR_CACHE2;
    m_dwCacheStart = dwStart;
    m_dwCacheSize = dwReadSize;
    return m_pCache;

ERR_CACHE2:
    _ClearInternalCacheData();
//ERR_CACHE:
    ATLASSERT(FALSE);
    return NULL;
  }

  LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)
  {
    if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())goto ERR_CACHEFORCE2;
    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwNeedSize);
    if(pCacheTry)return pCacheTry;

    pCacheTry = Cache(dwStart);
    DWORD dwRemain = GetRemain(dwStart);
    if(dwRemain > dwNeedSize)return pCacheTry;

    //goto ERR_CACHEFORCE2;

ERR_CACHEFORCE2:
    _ClearInternalCacheData();
//ERR_CACHEFORCE:
    ATLASSERT(FALSE);
    return NULL;
  }
};
