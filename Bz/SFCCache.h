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


#pragma once
class CSuperFileCon;

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

  DWORD GetMaxCacheSize() { return m_dwCacheAllocSize; }

  DWORD GetRemain(DWORD dwStart)
  {
    DWORD dwCacheEndPlus1 = m_dwCacheStart+m_dwCacheSize;
    if(m_pCache && (dwStart >= m_dwCacheStart || dwStart <= dwCacheEndPlus1))return dwCacheEndPlus1-dwStart;
    return 0;
  }

  BOOL Clear(DWORD dwStart = 0, DWORD dwSize = 0);

  const LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0);
  const LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize);
};



BOOL CSFCCache::Clear(DWORD dwStart, DWORD dwSize)
{
  if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
  if(m_dwCacheSize==0)return TRUE;
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

const LPBYTE CSFCCache::Cache(DWORD dwStart, DWORD dwIdealSize = 0)
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
  //ATLASSERT(FALSE);
  return NULL;
}

const LPBYTE CSFCCache::CacheForce(DWORD dwStart, DWORD dwNeedSize)
{
  if(!m_pCache || !m_pSFC || !m_pSFC->IsOpen())goto ERR_CACHEFORCE2;
  LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwNeedSize);
  if(pCacheTry)return pCacheTry;

  pCacheTry = Cache(dwStart);
  DWORD dwRemain = GetRemain(dwStart);
  if(dwRemain >= dwNeedSize)return pCacheTry;

  //goto ERR_CACHEFORCE2;

ERR_CACHEFORCE2:
  _ClearInternalCacheData();
  //ERR_CACHEFORCE:
  //ATLASSERT(FALSE);
  return NULL;
}