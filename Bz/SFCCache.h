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
  LPBYTE _GetLPBYTE(DWORD dwStart, DWORD dwSize)
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

public:

  DWORD GetRemain(DWORD dwStart)
  {
    if(m_pCache && dwStart >= m_dwCacheStart)return dwStart-m_dwCacheStart;
    return 0;
  }

  LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize)
  {
    if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwIdealSize);
    if(pCacheTry)return pCacheTry;

    DWORD dwFileRemain = m_pSFC->GetRemain(dwStart);
    if(dwFileRemain==0)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    DWORD dwReadSize = dwIdealSize;
    if(dwReadSize > dwFileRemain)dwReadSize = dwFileRemain;
    if(dwReadSize > m_dwCacheAllocSize)dwReadSize = m_dwCacheAllocSize;
    if(!m_pSFC->Read(m_pCache, dwStart, dwReadSize))return NULL;
    m_dwCacheStart = dwStart;
    m_dwCacheSize = dwReadSize;
    return m_pCache;
  }

  LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)
  {
    if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwNeedSize);
    if(pCacheTry)return pCacheTry;

    pCacheTry = Cache(dwStart, CACHEMAX);
    DWORD dwRemain = GetRemain(dwStart);
    if(dwRemain > dwNeedSize)return pCacheTry;
    ATLASSERT(FALSE);
    return NULL;
  }
};
