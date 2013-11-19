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

#include <windows.h>
#include <atlstr.h>//CString
#include <atlcoll.h>//CAtlArray
#include <atlfile.h>//CAtlFile
#include <atlutil.h>//AtlGetErrorDescription
#include "tree.h"
//#include "SFCCache.h"
//class CSFCCache;

#define MAX_FILELENGTH  0xFFFFFFF0

#define SHIFTBUFSIZE 8*1024*1024


typedef enum {	UNDO_INS, UNDO_OVR, UNDO_DEL } UndoMode;
typedef enum {	CHUNK_FILE, /*CHUNK_UNDO,*/ CHUNK_MEM } DataChunkType;
typedef enum {	DC_UNKNOWN, DC_FD, DC_FF, DC_DF, DC_DD, DC_DONE } DataChunkSavingType;
typedef struct _TAMADataBuf
{
  LPBYTE pData;
  DWORD nRefCount;
} TAMADataBuf;
typedef struct _TAMADataChunk
{
  DataChunkType dataType;
  union
  {
    TAMADataBuf *dataMem;
    DWORD dataFileAddr;
  };
  DWORD dwSize;
  DWORD dwSkipOffset;
  DataChunkSavingType savingType;//for Save()
} TAMADataChunk;
typedef struct _TAMAUndoRedo
{
  UndoMode mode;
  DWORD dwStart;

  TAMADataChunk **dataNext;
  DWORD nDataNext;
  TAMADataChunk **dataPrev;
  DWORD nDataPrev;

  BOOL bHidden;
} TAMAUndoRedo;

typedef struct _TAMAFILECHUNK
{
  RB_ENTRY(_TAMAFILECHUNK) linkage;
  union 
  {
    DWORD dwEnd; //Sort-Key
    DWORD key;
  };
  DWORD dwStart;
  TAMADataChunk* dataChunk;
  //	DWORD dwSkipOffset;

  BOOL bSaved; // for Save
} TAMAFILECHUNK;
static int cmpTAMAFILECHUNK(TAMAFILECHUNK *c1, TAMAFILECHUNK *c2)
{
  DWORD dw1 = c1->dwEnd;
  DWORD dw2 = c2->dwEnd;
  if(dw1==dw2)return 0;
  return (dw1>dw2)?1:-1;
}
RB_HEAD(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK);
//RB_PROTOTYPE(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK);
//RB_GENERATE(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK);
RB_PROTOTYPE_INTERNAL(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK, inline);
RB_GENERATE_INTERNAL(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK, inline);


typedef enum {	OF_NOREF, OF_FD, OF_FF } OldFileType;
typedef struct _TAMAOLDFILECHUNK
{
  RB_ENTRY(_TAMAOLDFILECHUNK) linkage;
  union 
  {
    DWORD dwEnd; //Sort-Key
    DWORD key;
  };
  DWORD dwStart;
  OldFileType type;
  union
  {
    DWORD dwNewFileAddr;
    TAMADataBuf *pDataBuf;//DataChunkのほうがいいかもSkipあるし
  };
  //DWORD dwSkipOffset;
} TAMAOLDFILECHUNK;
static int cmpTAMAOLDFILECHUNK(TAMAOLDFILECHUNK *c1, TAMAOLDFILECHUNK *c2)
{
  DWORD dw1 = c1->dwEnd;
  DWORD dw2 = c2->dwEnd;
  if(dw1==dw2)return 0;
  return (dw1>dw2)?1:-1;
}
RB_HEAD(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK);
//RB_PROTOTYPE(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK, linkage, cmpTAMAOLDFILECHUNK);
//RB_GENERATE(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK, linkage, cmpTAMAOLDFILECHUNK);
RB_PROTOTYPE_INTERNAL(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK, linkage, cmpTAMAOLDFILECHUNK, inline);
RB_GENERATE_INTERNAL(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK, linkage, cmpTAMAOLDFILECHUNK, inline);


//#ifdef SFC_EASYDEBUG
  typedef enum {	SFCOP_OPEN=0, SFCOP_CLOSE, SFCOP_WRITE, /*SFCOP_READ,*/ SFCOP_INSERT, SFCOP_DELETE, SFCOP_UNDO, SFCOP_REDO, SFCOP_CLEARUNDO, SFCOP_CLEARREDO, SFCOP_SAVE, SFCOP_SAVEAS, SFCOP_FATALERROR, SFCOP_FILL, SFCOP_CLOSE_REOPEN } SfcOp;
  const char SfcOpCode[] = {'O', 'C', 'W', 'I', 'D', 'U', 'R', 'u', 'r', 'S', 's', '!', 'F', 'c'};
//#endif
#ifdef SFC_EASYDEBUG
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
#endif

class CSuperFileCon
{
public:

  CSuperFileCon(void)
  {
    RB_INIT(&m_filemapHead);
    m_dwTotal = 0;
    m_dwTotalSavedFile = 0;
    m_bReadOnly = FALSE;
    m_savedIndex = m_redoIndex = m_dwHiddenStart = m_dwHiddenSize = 0;
    m_dwRefCount = 1;
    
    m_dwCacheStart = m_dwCacheSize = 0;
    m_dwCacheAllocSize = SFCC_CACHESIZE;
    m_pCache = (LPBYTE)malloc(m_dwCacheAllocSize);
    if(!m_pCache)
    {
      ATLASSERT(FALSE);
      m_dwCacheAllocSize = 0;
    }
    _EasyDebug_CreateEasyDebugFile();
  }
  ~CSuperFileCon(void)
  {
    _DeleteContents();
    if(m_pCache)
    {
      free(m_pCache);
      m_pCache = NULL;
    }
  }
  
  __declspec(noinline) void FatalError()
  {
#ifdef SFC_EASYDEBUG
    DWORD dwRetAddr = (DWORD)_ReturnAddress();
    _EasyDebug_OutputOp2(SFCOP_FATALERROR, dwRetAddr);
    _EasyDebug_Close();
#endif
    exit(-1);
  }

  DWORD GetRemainFile(DWORD dwStart)
  {
    //if(!m_file.m_h)return 0;
    DWORD dwFileSize = GetSize();
    if(dwFileSize>dwStart)return dwFileSize - dwStart;
    return 0;
  }

  // Open/Overwrite/Save to another file
  BOOL Open(LPCTSTR lpszPathName)
  {
    ATLTRACE("SuperFileCon::Open\n");
    //CWaitCursor wait;
    BOOL bReadOnly = FALSE;
    CAtlFile file;
    if((FAILED(file.Create(lpszPathName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING))))  // Open (+RW)
    {
      if (FAILED(file.Create(lpszPathName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) //Retry open (Read-Only)
      {
        //LastErrorMessageBox();
        return FALSE; //Failed open
      }
      bReadOnly = TRUE;
    }
    DWORD dwFileSize = _GetFileLengthLimit4G(&file);

    _DeleteContents();
    if (dwFileSize != 0)
    {
      if(!_FileMap_InsertFile(0, dwFileSize, 0))
      {
        ATLASSERT(FALSE);
        _DeleteContents();
        return FALSE;
      }
    }
    m_dwTotalSavedFile = m_dwTotal = dwFileSize;
    m_bReadOnly = bReadOnly;
    m_file = file;
    m_filePath = lpszPathName;
    m_dwTotal = dwFileSize;
#ifdef SFC_EASYDEBUG
    _EasyDebug_CreateEasyDebugFile();
    _EasyDebug_OutputOp2(SFCOP_OPEN, m_dwTotal);
#endif

    return TRUE;
  }
  void _FileMap_DestroyAll()
  {
    TAMAFILECHUNK *fileChunk, *nextFileChunk;
    RB_FOREACH_SAFE(fileChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead, nextFileChunk)
    {
      _FileMap_Remove(fileChunk);
    }
    ATLASSERT(__FileMap_LowMin()==NULL);
  }
  void _UndoRedo_DestroyAll()
  {
    size_t nUndoCount = m_undo.GetCount();
    for(size_t i=0;i<nUndoCount;i++)
    {
      _UndoRedo_ReleaseByIndex(i);
    }
    m_undo.RemoveAll();
    m_savedIndex = m_redoIndex = m_dwHiddenStart = m_dwHiddenSize = 0;
    ATLASSERT(m_undo.GetCount()==0);
  }
  BOOL _CopyFF(TAMAFILECHUNK *pFileChunk, CAtlFile *pWriteFile)
  {
    ATLASSERT(pFileChunk);
    ATLASSERT(pFileChunk->dataChunk->dataType == CHUNK_FILE);
    
    DWORD dwCopyStart = pFileChunk->dwStart;
    DWORD dwCopyAllSize = pFileChunk->dwEnd - pFileChunk->dwStart +1;
    DWORD dwRemain = dwCopyAllSize;
    if(dwRemain==0)return TRUE;
    DWORD dwBufSize = SHIFTBUFSIZE;
    DWORD dwCopySize = min(dwBufSize, dwRemain);
    LPBYTE buf = (LPBYTE)mallocMax(&dwCopySize);
    if(!buf)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwMoveStart = dwCopyStart;
    ATLTRACE("_CopyFF--- Start: 0x%08X(%u), Size: 0x%08X(%u)\n", dwMoveStart, dwMoveStart, dwCopyAllSize, dwCopyAllSize);
    while(dwRemain!=0)
    {
      ATLTRACE("      Write--- Start: 0x%08X(%u), Size: 0x%08X(%u)\n", dwMoveStart, dwMoveStart, dwCopySize, dwCopySize);
      if(FAILED(m_file.Seek(dwMoveStart, FILE_BEGIN)) || FAILED(m_file.Read(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      if(FAILED(pWriteFile->Seek(dwMoveStart, FILE_BEGIN)) || FAILED(pWriteFile->Write(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      dwRemain -= dwCopySize;
      dwMoveStart += dwCopySize;
      dwCopySize = min(dwCopySize, dwRemain);
    }
    free(buf);
    return TRUE;
  }
  void _ClearSavedFlags(CAtlFile *pWriteFile, BOOL bSaveAs)
  {
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      ATLASSERT(pChunk->dataChunk);
      pChunk->bSaved = FALSE;
      if(pChunk->dataChunk->dataType == CHUNK_FILE && pChunk->dwStart == _TAMAFILECHUNK_GetRealStartAddr(pChunk) /*サイズも確認したほうがいい？*/)
      {
        if(bSaveAs)_CopyFF(pChunk, pWriteFile);
        pChunk->bSaved = TRUE; //file change nothing
      }
    }
  }
  void _ClearDataChunkSavingFlag()
  {
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      ATLASSERT(pChunk->dataChunk);
      pChunk->dataChunk->savingType = DC_UNKNOWN;
    }
    size_t nUndo = m_undo.GetCount();
    for(size_t i=0; i < nUndo; i++)
    {
      TAMAUndoRedo *undo = m_undo[i];
      ATLASSERT(undo);
      if(undo->dataNext)
      {
        DWORD nDataNext = undo->nDataNext;
        TAMADataChunk **dataNext = undo->dataNext;
        for(DWORD i=0; i<nDataNext; i++)
          dataNext[i]->savingType = DC_UNKNOWN;
      }
      if(undo->dataPrev)
      {
        DWORD nDataPrev = undo->nDataPrev;
        TAMADataChunk **dataPrev = undo->dataPrev;
        for(DWORD i=0; i<nDataPrev; i++)
          dataPrev[i]->savingType = DC_UNKNOWN;
      }
    }
  }
  BOOL _Save_ExtendFileSize(CAtlFile *pWriteFile)
  {
    if(m_dwTotal <= m_dwTotalSavedFile)return TRUE;
    ATLTRACE("Change File Size: %u -> %u\n", m_dwTotalSavedFile, m_dwTotal);
    if(FAILED(pWriteFile->SetSize(m_dwTotal)))return FALSE;
    TAMAFILECHUNK *fileChunk = __FileMap_LowMax();
    while(fileChunk && fileChunk->dwEnd >= m_dwTotalSavedFile)
    {
      if(!fileChunk->bSaved)
      {
        switch(fileChunk->dataChunk->dataType)
        {
        case CHUNK_FILE:
          {
            if(!_TAMAFILECHUNK_ShiftFileChunk(fileChunk, pWriteFile))return FALSE;
            fileChunk->bSaved = TRUE;
          }
          break;
        case CHUNK_MEM:
          {
            if(fileChunk->dwStart < m_dwTotalSavedFile-1)
            {
              DWORD dwShiftStart = m_dwTotalSavedFile-1;
              DWORD dwRestoreOffset = fileChunk->dwEnd;
              _FileMap_SplitPoint(dwShiftStart);
              fileChunk = _FileMap_LookUp(dwRestoreOffset);
            }
            if(!_TAMAFILECHUNK_WriteMemChunk(fileChunk, pWriteFile))return FALSE;
            fileChunk->bSaved = TRUE;
          }
          break;
        default:
          ATLASSERT(FALSE);
          break;
        }
      }
      fileChunk = RB_PREV(_TAMAFILECHUNK_HEAD, &m_filemapHead, fileChunk);
    }
    return TRUE;
  }

  BOOL _Save_TAMADATACHUNKS_ScanDF(TAMADataChunk ***ppDataChunks, DWORD *pNumDataChunks, TAMADataBuf *pSearchDataBuf, DWORD Ss, DWORD dwSize, DWORD dwNewStartFileAddr)
  {
    DWORD Se = Ss + dwSize - 1;
    for(DWORD i=0;i<*pNumDataChunks;i++)
    {
      TAMADataChunk *dataChunk = (*ppDataChunks)[i];
      ATLASSERT(dataChunk);
      if(dataChunk && dataChunk->savingType == DC_UNKNOWN && dataChunk->dataType==CHUNK_MEM && dataChunk->dataMem == pSearchDataBuf)
      {
        ATLTRACE("ConvDF(0x%08X,0x%08X) --- Skip:0x%08X(%u), Size:0x%08X(%u)\n", dataChunk, dataChunk->dataMem, dataChunk->dwSkipOffset, dataChunk->dwSkipOffset, dataChunk->dwSize, dataChunk->dwSize);
        DWORD Ts = dataChunk->dwSkipOffset;
        DWORD dwTargetSize = dataChunk->dwSize;
        ATLASSERT(dwTargetSize > 0);
        DWORD Te = Ts + dwTargetSize - 1;
        DWORD dwTargetNewStartFileAddr = dwNewStartFileAddr;
        ATLASSERT(dataChunk->dwSize > 0);
        if(Se < Ts || Te < Ss)continue;
        BOOL bLeftSplit = Ts < Ss;
        BOOL bRightSplit= Se < Te;
        if(bLeftSplit)
        {
          DWORD dwLeftSplitSize = Ss - Ts;
          TAMADataChunk *leftDataChunk = _TAMADataChunk_CreateMemAssignTAMADataBuf(dwLeftSplitSize, Ts, pSearchDataBuf);
          if(!leftDataChunk)
          {
            ATLASSERT(FALSE);
            return FALSE;
          }
          ATLTRACE("  Left  --- Skip:0x%08X(%u), Size:0x%08X(%u)\n", leftDataChunk->dwSkipOffset, leftDataChunk->dwSkipOffset, leftDataChunk->dwSize, leftDataChunk->dwSize);
          leftDataChunk->savingType = DC_UNKNOWN;
          BOOL bRetInsert = _TAMADATACHUNKS_Insert(ppDataChunks, pNumDataChunks, i, leftDataChunk);
          if(!bRetInsert)
          {
            ATLASSERT(FALSE);
            return FALSE;
          }
          ATLASSERT(i<0xFFffFFff);
          i++;
          Ts += dwLeftSplitSize;
          ATLASSERT(Ts==Ss);
          dwTargetSize -= dwLeftSplitSize;
        }
        DWORD dwRightSplitSize = 0;
        if(bRightSplit)dwRightSplitSize = Te - Se;
        {
          DWORD dwMiddleSize = dwTargetSize - dwRightSplitSize;
          _TAMADataBuf_Release(dataChunk->dataMem);
          dataChunk->dataMem = NULL;
          dataChunk->dataType = CHUNK_FILE;
          dataChunk->dataFileAddr = dwTargetNewStartFileAddr;
          dataChunk->dwSize = dwMiddleSize;
          dataChunk->dwSkipOffset = 0;
          dataChunk->savingType = DC_DONE;
          ATLTRACE("  Middle--- Skip:0x%08X(%u), Size:0x%08X(%u), fileAddr:0x%08X(%u)\n", dataChunk->dwSkipOffset, dataChunk->dwSkipOffset, dataChunk->dwSize, dataChunk->dwSize, dataChunk->dataFileAddr, dataChunk->dataFileAddr);
          Ts += dwMiddleSize;
          dwTargetNewStartFileAddr += dwMiddleSize;
          dwTargetSize -= dwMiddleSize;
        }
        if(bRightSplit)
        {
          //DWORD dwRightSplitSize = Te - Se;
          TAMADataChunk *rightDataChunk = _TAMADataChunk_CreateMemAssignTAMADataBuf(dwRightSplitSize, Ts, pSearchDataBuf);
          ATLASSERT(rightDataChunk);
          if(!rightDataChunk)
          {
            ATLASSERT(FALSE);
            return FALSE;
          }
          ATLTRACE("  Right(0x%08X) --- Skip:0x%08X(%u), Size:0x%08X(%u)\n", rightDataChunk, rightDataChunk->dwSkipOffset, rightDataChunk->dwSkipOffset, rightDataChunk->dwSize, rightDataChunk->dwSize);
          rightDataChunk->savingType = DC_UNKNOWN;
          BOOL bRetInsert = _TAMADATACHUNKS_Insert(ppDataChunks, pNumDataChunks, i+1, rightDataChunk);
          if(!bRetInsert)
          {
            ATLASSERT(FALSE);
            return FALSE;
          }
          ATLASSERT(i<0xFFffFFff);
          i++;
        }
      }
    }
    return TRUE;
  }
  BOOL _Save_UndoRedo_ScanDF(TAMADataBuf *pSearchDataBuf, DWORD dwSkipOffset, DWORD dwSize, DWORD dwNewStartFileAddr, DWORD *nRefCount)
  {
    size_t nUndo = m_undo.GetCount();
    for(size_t i=0; i < nUndo; i++)
    {
      TAMAUndoRedo *undo = m_undo[i];
      ATLASSERT(undo);
      if(!_Save_TAMADATACHUNKS_ScanDF(&undo->dataNext, &undo->nDataNext, pSearchDataBuf, dwSkipOffset, dwSize, dwNewStartFileAddr)) return FALSE;
      if(*nRefCount<=1)
      {
        ATLASSERT(*nRefCount==1);
        return TRUE;
      }
      if(!_Save_TAMADATACHUNKS_ScanDF(&undo->dataPrev, &undo->nDataPrev, pSearchDataBuf, dwSkipOffset, dwSize, dwNewStartFileAddr)) return FALSE;
      if(*nRefCount<=1)
      {
        ATLASSERT(*nRefCount==1);
        return TRUE;
      }
    }
    return TRUE;
  }
  BOOL _TAMAFILECHUNK_WriteMemChunk(TAMAFILECHUNK *memChunk, CAtlFile *pWriteFile)
  {
    ATLASSERT(memChunk);
    ATLASSERT(memChunk->dataChunk);
    ATLASSERT(memChunk->dataChunk->dataType == CHUNK_MEM);
    ATLASSERT(memChunk->dataChunk->dataMem->pData);
    DWORD dwWriteSize = memChunk->dwEnd - memChunk->dwStart +1;
    ATLASSERT(dwWriteSize > 0);
    ATLTRACE("WriteMem: 0x%08X, %u (0x%X, %u)\n", memChunk->dwStart, memChunk->dwStart, dwWriteSize, dwWriteSize);

    if(FAILED(pWriteFile->Seek(memChunk->dwStart, FILE_BEGIN)) || FAILED(pWriteFile->Write(_TAMAFILECHUNK_GetRealStartPointer(memChunk), dwWriteSize)))return FALSE;

    TAMADataBuf *pSearchDataBuf = memChunk->dataChunk->dataMem;
    DWORD dwSkipOffset = memChunk->dataChunk->dwSkipOffset;
    DWORD dwSize = dwWriteSize;
    DWORD dwNewStartFileAddr = memChunk->dwStart;
    DWORD dwRemain = memChunk->dataChunk->dataMem->nRefCount - 1;
    if(dwRemain>0)
    {
      if(!_Save_UndoRedo_ScanDF(pSearchDataBuf, dwSkipOffset, dwSize, dwNewStartFileAddr, &(memChunk->dataChunk->dataMem->nRefCount)))return FALSE;
    }
    {
      TAMADataChunk *dataChunk = memChunk->dataChunk;
      _TAMADataBuf_Release(dataChunk->dataMem);
      dataChunk->dataMem = NULL;
      dataChunk->dataType = CHUNK_FILE;
      dataChunk->dataFileAddr = memChunk->dwStart;
      dataChunk->dwSize = dwWriteSize;
      dataChunk->dwSkipOffset = 0;
    }
    return TRUE;
  }
  BOOL _TAMAFILECHUNK_isRightShift(TAMAFILECHUNK *fileChunk)
  {
    ATLASSERT(fileChunk->dwStart != _TAMAFILECHUNK_GetRealStartAddr(fileChunk)); //No move
    if(fileChunk->dwStart < _TAMAFILECHUNK_GetRealStartAddr(fileChunk))return FALSE;
    return TRUE;
  }
  BOOL _Save_ShiftAllFF(CAtlFile *pWriteFile)
  {
    TAMAFILECHUNK *pLastFileChunk = NULL;
    TAMAFILECHUNK *pChunk;
    RB_FOREACH_REVERSE(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      if(!pChunk->bSaved && pChunk->dataChunk->dataType == CHUNK_FILE)
      {
        pLastFileChunk = pChunk;
        if(_TAMAFILECHUNK_isRightShift(pChunk))
        {
          if(!_Save_ShiftAllFileChunksAfterArg(pChunk, pWriteFile))
          {
            ATLASSERT(FALSE);
            FatalError();
            return FALSE;
          }
        }
      }
    }
    if(pLastFileChunk && !pLastFileChunk->bSaved)
    {
      if(!_Save_ShiftAllFileChunksAfterArg(pLastFileChunk, pWriteFile))
      {
        ATLASSERT(FALSE);
        FatalError();
        return FALSE;
      }
    }
    return TRUE;
  }
  BOOL _Save_WriteAllDF(CAtlFile *pWriteFile)
  {
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      if(!pChunk->bSaved)
      {
        ATLASSERT(pChunk->dataChunk->dataType == CHUNK_MEM);
        if(_TAMAFILECHUNK_WriteMemChunk(pChunk, pWriteFile)==FALSE)return FALSE;
        pChunk->bSaved = TRUE;
      }
    }
    return TRUE;
  }
  void _Save_UpdateAllDataChunkSavingType(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead)
  {
    _ClearDataChunkSavingFlag();

    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      ATLASSERT(pChunk);
      TAMADataChunk *dataChunk = pChunk->dataChunk;
      ATLASSERT(dataChunk);
      if(dataChunk->savingType == DC_UNKNOWN)
      {
        switch(dataChunk->dataType)
        {
        case CHUNK_FILE:
          dataChunk->savingType = DC_FF;
          break;
        case CHUNK_MEM:
          dataChunk->savingType = DC_DF;
          break;
        default:
          ATLASSERT(FALSE);
          break;
        }
      }
    }
    size_t nUndo = m_undo.GetCount();
    for(size_t i=0; i < nUndo; i++)
    {
      TAMAUndoRedo *undo = m_undo[i];
      ATLASSERT(undo);
      _OldFileMap_ConvFD(pOldFilemapHead, &undo->dataNext, &undo->nDataNext);
      _OldFileMap_ConvFD(pOldFilemapHead, &undo->dataPrev, &undo->nDataPrev);
    }
  }
  BOOL _OldFileMap_ConvFD(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, TAMADataChunk ***pDataChunks, DWORD *pNumDataChunks)
  {
    for(DWORD i=0;i<*pNumDataChunks;i++)
    {
      TAMADataChunk *dataChunk = (*pDataChunks)[i];
      ATLASSERT(dataChunk);
      if(dataChunk && dataChunk->savingType == DC_UNKNOWN && dataChunk->dataType==CHUNK_FILE)
      {
        ATLASSERT(dataChunk->dwSize > 0);
        DWORD dwOldFileAddrS = _TAMADataChunk_GetRealStartAddr(dataChunk);
        TAMAOLDFILECHUNK *pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwOldFileAddrS);
        DWORD nOldFileChunkS = _TAMAOLDFILECHUNK_GetSize(pOldFileChunkS);
        ATLASSERT(nOldFileChunkS > 0);
        DWORD nRemain = dataChunk->dwSize;
        //if(nOldFileChunkS < dataChunk->dwSize)nRemain = dataChunk->dwSize - nOldFileChunkS;
        //else nRemain=0;
        bool bOverWrite = true;//DWORD iS = i;
        TAMAOLDFILECHUNK *pOldFileChunkNext = pOldFileChunkS;
        ATLASSERT(nRemain > 0);
        while(1)
        {
          TAMADataChunk *newDataChunk = _OldFileMap_Conv2TAMADataChunk(pOldFileChunkNext, &nRemain, dwOldFileAddrS);
          ATLASSERT(newDataChunk);
          dwOldFileAddrS += newDataChunk->dwSize;
          if(bOverWrite)//if(iS==i)
          {
            (*pDataChunks)[i] = newDataChunk;
            _TAMADataChunk_Release(dataChunk);
            bOverWrite = false;
          } else {
            BOOL bRetInsert = _TAMADATACHUNKS_Insert(pDataChunks, pNumDataChunks, i+1, newDataChunk);
            ATLASSERT(bRetInsert);
            ATLASSERT(i<0xFFffFFff);
            i++;
          }
          //nRemain -= nNextChunkSize;

          if(nRemain == 0)break;

          pOldFileChunkNext = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pOldFileChunkNext);
          if(pOldFileChunkNext==NULL)
          {
            ATLASSERT(FALSE);
            return FALSE;
          }
          ATLASSERT(pOldFileChunkNext->type == OF_FD || pOldFileChunkNext->type == OF_FF);
        }
      }
    }
    return TRUE;
  }
  TAMADataChunk* _OldFileMap_Conv2TAMADataChunk(TAMAOLDFILECHUNK *pOldFileChunk, DWORD *pNRemain, DWORD dwStart)
  {
    ATLASSERT(pOldFileChunk->dwStart <= dwStart && dwStart <= pOldFileChunk->dwEnd);
    DWORD dwSkip = dwStart - pOldFileChunk->dwStart;
    TAMADataChunk *dataChunk = NULL;
    if(pOldFileChunk==NULL)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    ATLASSERT(pOldFileChunk->type == OF_FD || pOldFileChunk->type == OF_FF);
    DWORD nChunkSize = _TAMAOLDFILECHUNK_GetSize(pOldFileChunk);
    nChunkSize = min(nChunkSize, *pNRemain);
    switch(pOldFileChunk->type)
    {
    case OF_FD:
      {
        TAMADataBuf *pTAMADataBuf = pOldFileChunk->pDataBuf;
        ATLASSERT(pTAMADataBuf);
        dataChunk = _TAMADataChunk_CreateMemAssignTAMADataBuf(nChunkSize, dwSkip, pTAMADataBuf);
        ATLASSERT(dataChunk);
        dataChunk->savingType = DC_DONE;
        break;
      }
    case OF_FF:
      dataChunk = _TAMADataChunk_CreateFileChunk(nChunkSize, pOldFileChunk->dwNewFileAddr + dwSkip);
      ATLASSERT(dataChunk);
      dataChunk->savingType = DC_DONE;
      break;
    default:
      ATLASSERT(FALSE);
      return FALSE;
    }
    *pNRemain -= nChunkSize;
    return dataChunk;
  }
  TAMADataBuf* _TAMADataBuf_CreateAssign(LPBYTE pData, DWORD nRefCount = 0)
  {
    TAMADataBuf *pDataBuf = (TAMADataBuf *)malloc(sizeof(TAMADataBuf));
    if(pDataBuf)
    {
      pDataBuf->pData = pData;
      pDataBuf->nRefCount = nRefCount;
#ifdef TRACE_TAMADATABUF
      ATLTRACE("cDataBuf(0x%08X) nRefCount: %d\n", pDataBuf, pDataBuf->nRefCount);
#endif
    }
    return pDataBuf;
  }
  TAMADataBuf* _TAMADataBuf_CreateNew(DWORD dwNewAlloc, DWORD nRefCount = 0)
  {
    TAMADataBuf *pDataBuf = (TAMADataBuf *)malloc(sizeof(TAMADataBuf));
    ATLASSERT(pDataBuf);
    if(!pDataBuf)return NULL;

    pDataBuf->pData = (LPBYTE)malloc(dwNewAlloc);
    ATLASSERT(pDataBuf->pData);
    if(!pDataBuf->pData)
    {
      free(pDataBuf);
      return NULL;
    }
    pDataBuf->nRefCount = nRefCount;
#ifdef TRACE_TAMADATABUF
    ATLTRACE("cDataBuf(0x%08X) nRefCount: %d\n", pDataBuf, pDataBuf->nRefCount);
#endif
    return pDataBuf;
  }
  DWORD _TAMAOLDFILECHUNK_GetSize(TAMAOLDFILECHUNK *oldFileChunk)
  {
    if(!oldFileChunk)
    {
      ATLASSERT(FALSE);
      return 0;
    }
    return oldFileChunk->dwEnd - oldFileChunk->dwStart + 1;
  }
  BOOL _TAMADATACHUNKS_Insert(TAMADataChunk ***pDataChunks, DWORD *pNumDataChunks, DWORD nInsertIndex, TAMADataChunk *pInsertDataChunk)
  {
    ATLASSERT(*pNumDataChunks!=0xffFFffFF);
    ATLASSERT(nInsertIndex<=*pNumDataChunks);
#ifdef TRACE_DATACHUNKS
    ATLTRACE("_TAMADATACHUNKS_Insert: 0x%08X\n", pInsertDataChunk);
#endif
    (*pNumDataChunks)++;
    *pDataChunks = (TAMADataChunk **)realloc(*pDataChunks, (*pNumDataChunks)*sizeof(TAMADataChunk *));
    if(*pDataChunks==NULL)
    {
      *pNumDataChunks=0;
      return FALSE;
    }
    TAMADataChunk **pMoveStart = (*pDataChunks)+nInsertIndex;
    size_t nCopy = (*pNumDataChunks - nInsertIndex - 1)*sizeof(TAMADataChunk *);
    if(nCopy>0)memmove(pMoveStart+1, pMoveStart, nCopy);
    (*pDataChunks)[nInsertIndex] = pInsertDataChunk;
    return TRUE;
  }
  
  BOOL _Save_ProccessAllChunks(CAtlFile *pWriteFile, BOOL bSaveAs)
  {
    _ClearSavedFlags(pWriteFile, bSaveAs);
    struct _TAMAOLDFILECHUNK_HEAD oldFilemapHead;
    _OldFileMap_Make(&oldFilemapHead, &m_file);
    _Save_UpdateAllDataChunkSavingType(&oldFilemapHead);
    _OldFileMap_FreeAll(&oldFilemapHead, TRUE);//FALSE);
    if(!_Save_ExtendFileSize(pWriteFile)
      || !_Save_ShiftAllFF(pWriteFile)
      || !_Save_WriteAllDF(pWriteFile) )
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
#ifdef GTEST
#ifdef DEBUG
    ATLASSERT(!_Debug_SearchUnSavedChunk());
#endif //DEBUG
#endif //GTEST
    //_UndoRedo_CreateRefSrcFileDataChunk();
    m_savedIndex = m_redoIndex;
    return TRUE;
  }
  
  BOOL _Save_ShiftAllFileChunksAfterArg(TAMAFILECHUNK *fileChunk, CAtlFile *pWriteFile)
  {
    while(fileChunk) {
      if(!fileChunk->bSaved && fileChunk->dataChunk->dataType == CHUNK_FILE)
      {
        if(!_TAMAFILECHUNK_ShiftFileChunk(fileChunk, pWriteFile))
        {
          ATLASSERT(FALSE);
          return FALSE;
        }
      }
      fileChunk = RB_NEXT(_TAMAFILECHUNK_HEAD, &m_filemapHead, fileChunk);
    }
    return TRUE;
  }
  BOOL _TAMAFILECHUNK_ShiftFileChunk(TAMAFILECHUNK *fileChunk, CAtlFile *pWriteFile)
  {
    ATLASSERT(fileChunk);
    ATLASSERT(fileChunk->bSaved==FALSE);
    ATLASSERT(fileChunk->dataChunk->dataType == CHUNK_FILE);
    BOOL bRet = FALSE;
    if(_TAMAFILECHUNK_isRightShift(fileChunk))
    {
      if(!_TAMAFILECHUNK_ShiftFileChunkR(fileChunk, pWriteFile))
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
    } else {
      if(!_TAMAFILECHUNK_ShiftFileChunkL(fileChunk, pWriteFile))
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
    }
    fileChunk->bSaved = TRUE;
    return TRUE;
  }

  DWORD _TAMAFILECHUNK_GetRealStartAddr(TAMAFILECHUNK* fileChunk)
  {
    ATLASSERT(fileChunk->dataChunk->dataType==CHUNK_FILE);
    return _TAMADataChunk_GetRealStartAddr(fileChunk->dataChunk);
  }
  DWORD _TAMADataChunk_GetRealStartAddr(TAMADataChunk* fileDataChunk)
  {
    ATLASSERT(fileDataChunk->dataType==CHUNK_FILE);
    return fileDataChunk->dwSkipOffset + fileDataChunk->dataFileAddr;
  }

  LPBYTE _TAMAFILECHUNK_GetRealStartPointer(TAMAFILECHUNK* memChunk)
  {
    ATLASSERT(memChunk->dataChunk->dataType==CHUNK_MEM);
    return _TAMADataChunk_GetRealStartPointer(memChunk->dataChunk);
  }
  LPBYTE _TAMADataChunk_GetRealStartPointer(TAMADataChunk* memDataChunk)
  {
    ATLASSERT(memDataChunk->dataType==CHUNK_MEM);
    return memDataChunk->dwSkipOffset + memDataChunk->dataMem->pData;
  }

  BOOL Save() //上書き保存
  {
    if(m_bReadOnly || !m_file.m_h)return FALSE;
    //CWaitCursor wait;
    ATLTRACE("SuperFileCon::Save\n");
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_SAVE);
#endif
    BOOL bRet = _Save_ProccessAllChunks(&m_file, FALSE/*bSaveAs*/);
    if(m_dwTotal < m_dwTotalSavedFile)
    {
      ATLTRACE("Change File Size: %u -> %u\n", m_dwTotalSavedFile, m_dwTotal);
      if(FAILED(m_file.SetSize(m_dwTotal)))return FALSE;
    }
    m_dwTotalSavedFile = m_dwTotal;
    _RemoveNeedlessHiddenNode();
    m_file.Flush();
    _FileMap_DestroyAll();
    if(m_dwTotalSavedFile>0 && !_FileMap_InsertFile(0, m_dwTotalSavedFile, 0))
    {
      ATLASSERT(FALSE);
      FatalError();
      return FALSE;
    }
    ATLTRACE("--------------Save()\n");
    return bRet;
  }
  
  BOOL SaveAs(LPCTSTR lpszPathName) //名前を付けて保存
  {
    if(lpszPathName[0]==NULL)return FALSE;
    ATLTRACE("SuperFileCon::SaveAs(%s)\n", lpszPathName);
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_SAVEAS);
#endif
    if(m_filePath==lpszPathName)return Save();
    else return _SaveAs(lpszPathName);
  }
  
private:
  BOOL _SaveAs(LPCTSTR lpszPathName)
  {
    CAtlFile dstFile;
    if((FAILED(dstFile.Create(lpszPathName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS))))  // Open (+RW)
    {
      //LastErrorMessageBox();
      return FALSE; //Failed open
    }
    ATLTRACE("Change File Size: %u -> %u\n", 0, m_dwTotal);
    if(FAILED(dstFile.SetSize(m_dwTotal)))return FALSE;

    BOOL bRet = _Save_ProccessAllChunks(&dstFile, TRUE/*bSaveAs*/);
    m_file = dstFile;
    m_dwTotalSavedFile = m_dwTotal;
    _RemoveNeedlessHiddenNode();
    m_file.Flush();
    _FileMap_DestroyAll();
    if(m_dwTotalSavedFile>0 && !_FileMap_InsertFile(0, m_dwTotalSavedFile, 0))
    {
      ATLASSERT(FALSE);
      FatalError();
      return FALSE;
    }
    ATLTRACE("--------------SaveAs()\n");
    return bRet;
  }
  
public:
  void Close()
  {
    ATLTRACE("SuperFileCon::Close\n");
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_CLOSE);
    _EasyDebug_Close();
#endif
    _DeleteContents();
    m_file.Close();
  }

  DWORD _TAMAFILECHUNK_GetRemain(TAMAFILECHUNK *pFileChunk, DWORD dwStartOffset)
  {
    if(pFileChunk->dwStart <= dwStartOffset && dwStartOffset <= pFileChunk->dwEnd) return pFileChunk->dwEnd - dwStartOffset + 1;
    return 0; //Error
  }

  DWORD _TAMAFILECHUNK_Read(LPBYTE dst, TAMAFILECHUNK *pSrcFileChunk, DWORD dwStartOffset, DWORD dwMaxRead)
  {
    DWORD dwCanRead = _TAMAFILECHUNK_GetRemain(pSrcFileChunk, dwStartOffset);
    DWORD dwRemain = min(dwCanRead, dwMaxRead);
    ATLASSERT(dwStartOffset >= pSrcFileChunk->dwStart);
    DWORD dwShift = dwStartOffset - pSrcFileChunk->dwStart;

    TAMADataChunk *dataChunk = pSrcFileChunk->dataChunk;

    switch(dataChunk->dataType)
    {
    case CHUNK_FILE:
      {
        DWORD dwFileStart = _TAMAFILECHUNK_GetRealStartAddr(pSrcFileChunk) + dwShift;
        if(FAILED(m_file.Seek(dwFileStart, FILE_BEGIN)) || FAILED(m_file.Read(dst, dwRemain)))return 0;
        return dwRemain;
      }
    case CHUNK_MEM:
      {
        void *pSrcStart = _TAMAFILECHUNK_GetRealStartPointer(pSrcFileChunk) + dwShift;
        memcpy(dst, pSrcStart, dwRemain);
        return dwRemain;
      }
    }
    return 0;
  }

  // Read/OverWrite
  BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize)
  {
    //ATLTRACE("SuperFileCon::Read(), dwStart: %u(0x%08X), dwSize: %u(0x%08X)\n", dwStart, dwStart ,dwSize, dwSize);
    //if(!m_file.m_h)return FALSE;
    TAMAFILECHUNK *pReadChunk;
    pReadChunk = _FileMap_LookUp(dwStart);
    if(!pReadChunk)return FALSE;

    LPBYTE lpDst1 = (LPBYTE)dst1;
    DWORD dwRemain = dwSize;
    DWORD dwReadedAll = 0;

    while(dwRemain > 0 && pReadChunk)
    {
      if(!pReadChunk)
      {
        ATLASSERT(FALSE); //Corrupt filemap
        return FALSE;
      }
      DWORD dwReaded = _TAMAFILECHUNK_Read(lpDst1, pReadChunk, dwStart+dwReadedAll, dwRemain);
      ATLASSERT(dwRemain >= dwReaded);
      ATLASSERT(dwReaded);
      if(dwReaded==0)return FALSE;
      dwRemain -= dwReaded;
      dwReadedAll += dwReaded;
      lpDst1 += dwReaded;
      pReadChunk = RB_NEXT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pReadChunk);
    }

    return (dwRemain==0);
  }
  BOOL ReadTwin(void *dst1, void *dst2, DWORD dwStart, DWORD dwSize)
  {
    BOOL bRet = Read(dst1, dwStart, dwSize);
    if(bRet)memcpy(dst2, dst1, dwSize);
    return bRet;
  }
  BOOL Fill(LPBYTE pData, DWORD dwDataSize, DWORD dwStart, DWORD dwFillSize)
  {
    BOOL bRet = _Fill(pData, dwDataSize, dwStart, dwFillSize);
    if(bRet)ClearCache(dwStart);
    else ClearCache();
    return bRet;
  }
  _inline BOOL _Fill(LPBYTE pData, DWORD dwDataSize, DWORD dwStart, DWORD dwFillSize)
  {
    if(/*!m_file.m_h ||*/ dwFillSize==0 || dwDataSize==0 || dwStart>m_dwTotal)return FALSE;
    ATLTRACE("SuperFileCon::Fill(), dwDataSize: %u(0x%08X), dwFillSize: %u(0x%08X)\n", dwDataSize, dwDataSize ,dwFillSize, dwFillSize);
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp3(SFCOP_FILL, dwDataSize, dwFillSize);
#endif
    LPBYTE pNewData = (LPBYTE)malloc(dwFillSize);
    if(!pNewData)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwRemain = dwFillSize;
    LPBYTE pOut = pNewData;
    for(; dwDataSize <= dwRemain; pOut+=dwDataSize, dwRemain-=dwDataSize)
    {
      memcpy(pOut, pData, dwDataSize);
    }
    if(dwRemain)memcpy(pOut, pData, dwRemain);

    BOOL bRet = WriteAssign(pNewData, dwStart, dwFillSize);
    if(!bRet)free(pNewData);
    return bRet;
  }
  BOOL Write(void *srcData, DWORD dwStart, DWORD dwSize)
  {
    if(/*!m_file.m_h ||*/ dwSize==0 || dwStart>m_dwTotal)return FALSE;
    ATLTRACE("SuperFileCon::Write(), dwStart: %u(0x%08X), dwSize: %u(0x%08X)\n", dwStart, dwStart ,dwSize, dwSize);
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp3(SFCOP_INSERT, dwStart, dwSize);
#endif
    void *pData = (void *)malloc(dwSize);
    if(!pData)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    memcpy(pData, srcData, dwSize);
    BOOL bRet = WriteAssign(pData, dwStart, dwSize);
    if(!bRet)free(pData);
    return bRet;
  }
  _inline BOOL WriteAssign(void *srcDataDetached/*Write()失敗した場合は呼び出し元で開放すること*/, DWORD dwStart, DWORD dwSize)
  {
    BOOL bRet = _WriteAssign(srcDataDetached, dwStart, dwSize);
    if(bRet)ClearCache(dwStart, dwSize);
    else ClearCache();
    return bRet;
  }
  _inline BOOL _WriteAssign(void *srcDataDetached/*Write()失敗した場合は呼び出し元で開放すること*/, DWORD dwStart, DWORD dwSize)
  {
    if(/*!m_file.m_h ||*/ dwSize==0 || dwStart>m_dwTotal)return FALSE;
    DWORD dwNewTotal = dwStart + dwSize;
    if(dwNewTotal < dwStart)
    {
      ATLASSERT(FALSE);
      return FALSE;//dwSize too big (overflow)
    }
    DWORD dwGlow = 0;
    if(dwNewTotal > m_dwTotal)dwGlow = dwNewTotal - m_dwTotal;
    size_t nPrevSize = dwSize-dwGlow;

    TAMAUndoRedo *pNewUndo = _TAMAUndoRedo_Create(UNDO_OVR, dwStart, NULL, 0, NULL, 0);//, nPrevSize, dwSize);
    if(!pNewUndo)
    {
      ATLASSERT(FALSE);
      return FALSE; //memory full
    }
    TAMADataChunk **ppNextDataChunks = _TAMADATACHUNKS_CreateWith1MemDataChunk(dwSize, 0, (LPBYTE)srcDataDetached);
    if(!ppNextDataChunks)
    {
      ATLASSERT(FALSE);
      free(pNewUndo);
      return FALSE;
    }
    pNewUndo->dataNext = ppNextDataChunks;
    pNewUndo->nDataNext = 1;

    TAMADataChunk **pPrevDataChunks = NULL;
    DWORD nPrevDataChunks = 0;
    if(!_FileMap_CreateTAMADataChunks(dwStart, nPrevSize, &pPrevDataChunks, &nPrevDataChunks))
    {
      ATLASSERT(FALSE);
      _TAMADataChunks_Release(ppNextDataChunks, 1, FALSE/*bFreeRawPointer*/);
      free(pNewUndo);
      return FALSE;
    }
    pNewUndo->dataPrev = pPrevDataChunks;
    pNewUndo->nDataPrev = nPrevDataChunks;

    if(!_UndoRedo_Add(pNewUndo))
    {
      ATLASSERT(FALSE);
      _TAMADataChunks_Release(ppNextDataChunks, 1, FALSE/*bFreeRawPointer*/);
      _TAMADataChunks_Release(pPrevDataChunks, nPrevDataChunks, TRUE/*bFreeRawPointer*/);
      free(pNewUndo);
      return FALSE;
    }

    if(!_FileMap_OverWriteTAMADataChunks(dwStart, pNewUndo->dataNext, pNewUndo->nDataNext, 0))
    {
      ATLASSERT(FALSE);
      pNewUndo->dataNext = NULL;
      pNewUndo->nDataNext = 0;
      if(m_undo.GetCount() > 0)_UndoRedo_RemoveByIndex(m_undo.GetCount()-1, TRUE/*bFreeRawPointer*/);
      _TAMADataChunks_Release(ppNextDataChunks, 1, FALSE/*bFreeRawPointer*/);
      return FALSE;
    }
    //m_dwTotal += dwGlow;
    return TRUE;
  }
  TAMAUndoRedo* _TAMAUndoRedo_Create(UndoMode mode, DWORD dwStart, TAMADataChunk **dataPrev=NULL, DWORD nDataPrev=0, TAMADataChunk **dataNext=NULL, DWORD nDataNext=0)
  {
    TAMAUndoRedo *pNewUndo = NULL;
    pNewUndo = (TAMAUndoRedo *)malloc(sizeof(TAMAUndoRedo));
    if(!pNewUndo) goto err_TAMAUndoRedoCreate;
    pNewUndo->dwStart = dwStart;
    pNewUndo->dataPrev = dataPrev;
    pNewUndo->nDataPrev = nDataPrev;
    pNewUndo->dataNext = dataNext;
    pNewUndo->nDataNext = nDataNext;
    pNewUndo->mode = mode;
    pNewUndo->bHidden = FALSE;
    return pNewUndo;

err_TAMAUndoRedoCreate:
    ATLASSERT(FALSE);
    if(pNewUndo)
    {
      //if(pNewUndo->dataNext)_TAMADataChunks_Release(pNewUndo->dataNext, pNewUndo->nDataNext);
      //if(pNewUndo->dataPrev)_TAMADataChunks_Release(pNewUndo->dataPrev, pNewUndo->nDataPrev);
      free(pNewUndo);
    }
    return NULL;
  }
  DWORD inline _GetEndOffset(DWORD dwStart, DWORD dwSize) { ATLASSERT(dwSize>0); return dwStart+dwSize-1; };

  TAMADataChunk** _TAMADATACHUNKS_CreateWith1MemDataChunk(DWORD dwSize, DWORD dwSkipOffset, LPBYTE srcDataDetached)
  {
    TAMADataChunk *pDataChunk = _TAMADataChunk_CreateMemAssignRawPointer(dwSize, 0, srcDataDetached);
    if(!pDataChunk)
    {
      return NULL;
    }
    TAMADataChunk **ppDataChunks = (TAMADataChunk **)malloc(sizeof(TAMADataChunk *)*1);
    if(!ppDataChunks)
    {
      _TAMADataChunk_Release(pDataChunk, FALSE/*bFreeRawPointer*/);
      return NULL;
    }
    ppDataChunks[0] = pDataChunk;
    return ppDataChunks;
  }

  // Insert/Delete
  BOOL Insert(LPBYTE srcData, DWORD dwInsStart, DWORD dwInsSize)
  {
    if(/*!m_file.m_h ||*/ dwInsSize==0 || dwInsStart>m_dwTotal)return FALSE;
    ATLTRACE("SuperFileCon::Insert(), dwInsStart: %u(0x%08X), dwSize: %u(0x%08X)\n", dwInsStart, dwInsStart ,dwInsSize, dwInsSize);
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp3(SFCOP_INSERT, dwInsStart, dwInsSize);
#endif
    LPBYTE pData = (LPBYTE)malloc(dwInsSize);
    if(!pData)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    memcpy(pData, srcData, dwInsSize);
    BOOL bRet = InsertAssign(pData, dwInsStart, dwInsSize);
    if(!bRet)free(pData);
    return bRet;
  }
  _inline BOOL InsertAssign(LPBYTE srcDataDetached/*Insert()失敗した場合は呼び出し元で開放すること*/, DWORD dwInsStart, DWORD dwInsSize)
  {
    BOOL bRet = _InsertAssign(srcDataDetached, dwInsStart, dwInsSize);
    if(bRet)ClearCache(dwInsStart);
    else ClearCache();
    return bRet;
  }
  _inline BOOL _InsertAssign(LPBYTE srcDataDetached/*Insert()失敗した場合は呼び出し元で開放すること*/, DWORD dwInsStart, DWORD dwInsSize)
  {
    if(/*!m_file.m_h ||*/ dwInsSize==0 || dwInsStart>m_dwTotal)return FALSE;
    TAMAUndoRedo *pNewUndo = _TAMAUndoRedo_Create(UNDO_INS, dwInsStart, NULL, 0, NULL, 0);
    if(!pNewUndo)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    TAMADataChunk **ppNextDataChunks = _TAMADATACHUNKS_CreateWith1MemDataChunk(dwInsSize, 0, srcDataDetached);
    if(!ppNextDataChunks)
    {
      ATLASSERT(FALSE);
      free(pNewUndo);
      return FALSE;
    }
    pNewUndo->dataNext = ppNextDataChunks;
    pNewUndo->nDataNext = 1;

    if(!_UndoRedo_Add(pNewUndo))
    {
      ATLASSERT(FALSE);
      _TAMADataChunks_Release(ppNextDataChunks, 1, FALSE/*bFreeRawPointer*/);
      free(pNewUndo);
      return FALSE;
    }

    if(!_FileMap_InsertTAMADataChunks(dwInsStart, pNewUndo->dataNext, 1, dwInsSize))
    {
      ATLASSERT(FALSE);
      if(m_undo.GetCount() > 0)_UndoRedo_RemoveByIndex(m_undo.GetCount()-1, FALSE/*bFreeRawPointer*/);
      return FALSE;
    }
    return TRUE;
  }
  BOOL Delete(DWORD dwDelStart, DWORD dwDelSize)
  {
    BOOL bRet = _Delete(dwDelStart, dwDelSize);
    if(bRet)ClearCache(dwDelStart);
    else ClearCache();
    return bRet;
  }
  _inline BOOL _Delete(DWORD dwDelStart, DWORD dwDelSize)
  {
    if(/*!m_file.m_h ||*/ dwDelSize==0 || m_dwTotal < dwDelStart+dwDelSize)return FALSE;
    ATLTRACE("SuperFileCon::Delete\n");
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp3(SFCOP_DELETE, dwDelStart, dwDelSize);
#endif
    TAMAUndoRedo *pNewUndo = _TAMAUndoRedo_Create(UNDO_DEL, dwDelStart, NULL, 0, NULL, 0);
    if(!pNewUndo)
    {
      ATLASSERT(FALSE);
      return FALSE; //memory full
    }
    TAMADataChunk **pDataChunks = NULL;
    DWORD nDataChunks = 0;
    if(!_FileMap_CreateTAMADataChunks(dwDelStart, dwDelSize, &pDataChunks, &nDataChunks))
    {
      ATLASSERT(FALSE);
      free(pNewUndo);
      return FALSE;
    }
    pNewUndo->dataPrev = pDataChunks;
    pNewUndo->nDataPrev = nDataChunks;

    if(!_UndoRedo_Add(pNewUndo))
    {
      ATLASSERT(FALSE);
      _TAMADataChunks_Release(pDataChunks, nDataChunks, TRUE/*bFreeRawPointer*/);
      free(pNewUndo);
      return FALSE;
    }

    if(!_FileMap_Del(dwDelStart, dwDelSize))
    {
      ATLASSERT(FALSE);
      if(m_undo.GetCount() > 0)_UndoRedo_RemoveByIndex(m_undo.GetCount()-1, TRUE/*bFreeRawPointer*/);
      return FALSE;
    }
    return TRUE;
  }
  BOOL _PreNewUndo()
  {
    BOOL nRet=	ClearRedo();
    nRet	&=	_MakeRestoreHiddenNodeFromDiskFile();
    return nRet;
  }
  BOOL _MakeRestoreHiddenNodeFromDiskFile()
  {
    if(m_redoIndex<m_savedIndex)
    {
      _HideNodes(m_redoIndex, m_savedIndex);
      //compacthiddennode (too fast) + change m_savedIndex
      for(size_t i = m_savedIndex-1; i>=m_redoIndex; i--)
      {
        TAMAUndoRedo *reverseUndo = _TAMAUndoRedo_ReverseCopy(m_undo[i]);
        if(reverseUndo==NULL)
        {
          ATLASSERT(FALSE);
          return FALSE;
        }
        m_undo.Add(reverseUndo);
      }
      //compacthiddennode (too safe
      m_dwHiddenStart = m_redoIndex;
      m_dwHiddenSize = (m_savedIndex - m_redoIndex)*2;
      m_redoIndex+=m_dwHiddenSize;
    }
    return TRUE;
  }
  inline BOOL _UndoRedo_Add(TAMAUndoRedo *undo)
  {
    if(!_PreNewUndo())return FALSE;
    m_redoIndex++;
    m_undo.Add(undo);
    return TRUE;
  }
  //BOOL _CompactHiddenNode()
  //{
  //  return TRUE;
  //}
  inline void _RemoveNeedlessHiddenNode()
  {
    if(m_dwHiddenSize==0)return;
#ifdef DEBUG
    for(DWORD i=0; i<m_dwHiddenSize; i++)
    {
      ATLASSERT(m_undo[m_dwHiddenStart + i]->bHidden==TRUE);
    }
#endif
    _UndoRedo_RemoveRange(m_dwHiddenStart, m_dwHiddenSize);
    /*
    size_t nStartIndex = m_dwHiddenStart;
    size_t nEndIndex = nStartIndex + m_dwHiddenSize - 1;
    ATLASSERT(nStartIndex<=nEndIndex);

    BOOL bHiddenStarted = FALSE;
    size_t nHiddenStarted;
    BOOL bFoundSavedIndex = FALSE;
    for(size_t i=nStartIndex; i<=nEndIndex; i++)
    {
    TAMAUndoRedo *undo = m_undo[i];
    if(!bHiddenStarted)
    {
    if(undo->bHidden)
    {
    bHiddenStarted = TRUE;
    nHiddenStarted = i;
    if(i==m_savedIndex)bFoundSavedIndex = TRUE;
    }
    } else {
    if(!undo->bHidden)
    {
    if(!bFoundSavedIndex)
    {
    size_t delSize = i-nHiddenStarted;
    _UndoRedo_RemoveRange(nHiddenStarted, delSize);
    i-=delSize;
    }
    bFoundSavedIndex = FALSE;
    bHiddenStarted = FALSE;
    } else {
    if(i==m_savedIndex)bFoundSavedIndex = TRUE;
    }
    }
    }
    */
    m_dwHiddenStart = m_dwHiddenSize = 0;
  }
  inline void _HideNodes(size_t nStartIndex, size_t nExitIndex)
  {
    ATLASSERT(nStartIndex<=nExitIndex);
    for(size_t i=nStartIndex; i<nExitIndex; i++)
      m_undo[i]->bHidden = TRUE;
  }
  inline TAMAUndoRedo* _TAMAUndoRedo_ReverseCopy(TAMAUndoRedo *srcUndo)
  {
    TAMAUndoRedo *newUndo = (TAMAUndoRedo*)malloc(sizeof(TAMAUndoRedo));
    if(newUndo==NULL)return NULL;
    switch(srcUndo->mode)
    {
    case UNDO_INS:
      newUndo->mode = UNDO_DEL;
      break;
    case UNDO_DEL:
      newUndo->mode = UNDO_INS;
      break;
    case UNDO_OVR:
      newUndo->mode = UNDO_OVR;
      break;
    }
    newUndo->dwStart = srcUndo->dwStart;
    newUndo->bHidden = srcUndo->bHidden;
    newUndo->dataNext = _TAMADATACHUNKS_Copy(srcUndo->dataPrev, srcUndo->nDataPrev);
    ATLASSERT(newUndo->dataNext);
    if(!newUndo->dataNext)
    {
      free(newUndo);
      return NULL;
    }
    newUndo->nDataNext = srcUndo->nDataPrev;
    newUndo->dataPrev = _TAMADATACHUNKS_Copy(srcUndo->dataNext, srcUndo->nDataNext);
    ATLASSERT(newUndo->dataPrev);
    if(!newUndo->dataPrev)
    {
      _TAMADataChunks_Release(newUndo->dataNext, newUndo->nDataNext);
      free(newUndo);
      return NULL;
    }
    newUndo->nDataPrev = srcUndo->nDataNext;

    return newUndo;
  }
  TAMADataChunk** _TAMADATACHUNKS_Copy(TAMADataChunk **dataChunks, DWORD nData)
  {
    TAMADataChunk **pDataChunks = (TAMADataChunk **)malloc(sizeof(TAMADataChunk *)*nData);
    if(pDataChunks==NULL)return NULL;
    for(DWORD i=0; i<nData; i++)
    {
      ATLASSERT(dataChunks[i]);
      pDataChunks[i] = _TAMADataChunk_Copy(dataChunks[i]);
    }
    return pDataChunks;
  }
  TAMADataChunk* _TAMADataChunk_Copy(TAMADataChunk *dataChunk)
  {
    ATLASSERT(dataChunk);
    if(!dataChunk)return NULL;
    TAMADataChunk *newChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
    if(newChunk==NULL)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
#ifdef TRACE_DATACHUNK
    ATLTRACE("_TAMADataChunk_Copy(0x%08X)\n", newChunk);
#endif
    memcpy(newChunk, dataChunk, sizeof(TAMADataChunk));
    switch(dataChunk->dataType)
    {
    case CHUNK_MEM:
      _TAMADataBuf_IncRef(newChunk->dataMem);
      break;
    case CHUNK_FILE:
      break;
    default:
      ATLASSERT(FALSE);
      free(newChunk);
      return NULL;
    }
    return newChunk;
  }
  void _TAMADataChunk_IncRef(TAMADataChunk *dataChunk)
  {
    if(dataChunk->dataType == CHUNK_MEM && dataChunk->dataMem)_TAMADataBuf_IncRef(dataChunk->dataMem);
  }
  void _TAMADataBuf_IncRef(TAMADataBuf *pDataBuf)
  {
    ATLASSERT(pDataBuf->nRefCount<0xFFffFFff);
    pDataBuf->nRefCount++;
#ifdef TRACE_TAMADATABUF
    ATLTRACE("+DataBuf(0x%08X) nRefCount: %d -> %d\n", pDataBuf, pDataBuf->nRefCount-1, pDataBuf->nRefCount);
#endif
  }

  // Undo/Redo
  BOOL Undo(DWORD *pRetStart = NULL)
  {
    ClearCache();
    return _Undo(pRetStart);
  }
  _inline BOOL _Undo(DWORD *pRetStart = NULL)
  {
    if(/*!m_file.m_h ||*/ GetUndoCount()==0)return FALSE;
    ATLTRACE("SuperFileCon::Undo\n");
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_UNDO);
#endif
    m_redoIndex--;
    if(m_dwHiddenSize!=0 && m_redoIndex < m_undo.GetCount() && m_undo[m_redoIndex]->bHidden)
    {
      m_redoIndex-=m_dwHiddenSize;
      ATLASSERT(m_undo[m_redoIndex+1]->bHidden);
    }
    ATLASSERT(m_undo[m_redoIndex]->bHidden==FALSE);
    TAMAUndoRedo *undo = m_undo[m_redoIndex];
    switch(undo->mode)
    {
    case UNDO_INS:
    {
      ATLASSERT(undo->dataNext);
      ATLASSERT(undo->dataNext[0]);
      DWORD dwNextSize = _TAMADATACHUNKS_GetSumSize(undo->dataNext, undo->nDataNext);
      _FileMap_Del(undo->dwStart, dwNextSize);
      break;
    }
    case UNDO_DEL:
      ATLASSERT(undo->dataPrev);
      _FileMap_InsertTAMADataChunks(undo->dwStart, undo->dataPrev, undo->nDataPrev);
      break;
    case UNDO_OVR:
      {
        ATLASSERT(undo->dataPrev);
        ATLASSERT(undo->nDataPrev);
        DWORD dwPrevSize = _TAMADATACHUNKS_GetSumSize(undo->dataPrev, undo->nDataPrev);
        DWORD dwNextSize = _TAMADATACHUNKS_GetSumSize(undo->dataNext, undo->nDataNext);
        _FileMap_OverWriteTAMADataChunks(undo->dwStart, undo->dataPrev, undo->nDataPrev, max(dwPrevSize, dwNextSize));
        break;
      }
    default:
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(pRetStart)*pRetStart = undo->dwStart;
    return TRUE;
  }
  BOOL Redo(DWORD *pRetStart = NULL)
  {
    ClearCache();
    return _Redo(pRetStart);
  }
  _inline BOOL _Redo(DWORD *pRetStart = NULL)
  {
    if(/*!m_file.m_h ||*/ GetRedoCount()==0)return FALSE;
    ATLTRACE("SuperFileCon::Redo\n");
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_REDO);
#endif
    ATLASSERT(m_undo[m_redoIndex]->bHidden==FALSE);
    TAMAUndoRedo *undo = m_undo[m_redoIndex];
    m_redoIndex++;
    if(m_dwHiddenSize!=0 && m_redoIndex < m_undo.GetCount() && m_undo[m_redoIndex]->bHidden)
    {
      m_redoIndex+=m_dwHiddenSize;
      ATLASSERT(m_undo[m_redoIndex-1]->bHidden);
    }
    switch(undo->mode)
    {
    case UNDO_INS:
      ATLASSERT(undo->dataNext);
      _FileMap_InsertTAMADataChunks(undo->dwStart, undo->dataNext, undo->nDataNext);
      break;
    case UNDO_DEL:
      {
        ATLASSERT(undo->dataPrev);
        ATLASSERT(undo->dataPrev[0]);
        DWORD dwPrevSize = _TAMADATACHUNKS_GetSumSize(undo->dataPrev, undo->nDataPrev);
        _FileMap_Del(undo->dwStart, dwPrevSize);
      }
      break;
    case UNDO_OVR:
      {
        ATLASSERT(undo->dataNext);
        ATLASSERT(undo->nDataNext);
        DWORD dwPrevSize = _TAMADATACHUNKS_GetSumSize(undo->dataPrev, undo->nDataPrev);
        DWORD dwNextSize = _TAMADATACHUNKS_GetSumSize(undo->dataNext, undo->nDataNext);
        _FileMap_OverWriteTAMADataChunks(undo->dwStart, undo->dataNext, undo->nDataNext, max(dwPrevSize, dwNextSize));
        break;
      }
    default:
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(pRetStart)*pRetStart = undo->dwStart;
    return TRUE;
  }
  size_t GetUndoCount()
  {
    size_t nUndoSize = m_redoIndex;
    if(m_dwHiddenSize>0 && m_dwHiddenStart<m_redoIndex)nUndoSize-=m_dwHiddenSize;
    return nUndoSize;
  }
  size_t GetRedoCount()
  {
    size_t nRedoSize = m_undo.GetCount() - m_redoIndex;
    if(m_dwHiddenSize>0 && m_dwHiddenStart>=m_redoIndex)nRedoSize-=m_dwHiddenSize;
    return nRedoSize;
  }
  size_t GetUndoCountCanRemove(){ return min(m_savedIndex, m_redoIndex); }
  size_t GetRedoCountCanRemove(size_t *pDelIndex = NULL){ size_t delIndex = max(m_savedIndex, m_redoIndex);
  if(pDelIndex)*pDelIndex = delIndex;
  return m_undo.GetCount() - delIndex; }
  BOOL ClearUndoRedoAll()
  {
    BOOL nRet=	ClearRedo();
    nRet	&=	ClearUndo();
    return nRet;
  }
  BOOL ClearRedo()
  {
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_CLEARUNDO);
#endif
    size_t delIndex;
    size_t nDelSize = GetRedoCountCanRemove(&delIndex);
    if(nDelSize>0)_UndoRedo_RemoveRange(delIndex, nDelSize);
    return TRUE;
  }
  BOOL ClearUndo()
  {
#ifdef SFC_EASYDEBUG
    _EasyDebug_OutputOp1(SFCOP_CLEARUNDO);
#endif
    size_t nDelSize = GetUndoCountCanRemove();
    if(nDelSize>0)_UndoRedo_RemoveRange(0, nDelSize);
    return TRUE;
  }
  
  DWORD GetSize()
  {
    //if(!m_file.m_h)return 0;
    return m_dwTotal;
  }

  BOOL IsModified()
  {
    //if(!m_file.m_h)return FALSE;
    return m_savedIndex != m_redoIndex;
  }

  BOOL IsOpen()
  {
    return m_file.m_h!=0;
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
  DWORD GetRemainCache(DWORD dwStart)
  {
    DWORD dwCacheEndPlus1 = m_dwCacheStart+m_dwCacheSize;
    if(m_pCache && (dwStart >= m_dwCacheStart || dwStart <= dwCacheEndPlus1))return dwCacheEndPlus1-dwStart;
    return 0;
  }

  BOOL ClearCache(DWORD dwStart = 0, DWORD dwSize = 0)
  {
    //if(!IsOpen())
    //{
    //  _ClearInternalCacheData();
    //  return TRUE;
    //}
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

#ifdef SFCC_CHECKCACHE
  const LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0)
  {
    LPBYTE p = _Cache(dwStart, dwIdealSize);
    if(p)
    {
      DWORD dwDebug = GetRemainCache(dwStart);
      LPBYTE pDebug = (LPBYTE)malloc(dwDebug);
      if(!Read(pDebug, dwStart, dwDebug))ATLASSERT(FALSE);
      if(memcmp(pDebug, p, dwDebug)!=0)ATLASSERT(FALSE);
    }
    return p;
  }
  const LPBYTE _Cache(DWORD dwStart, DWORD dwIdealSize = 0)
#else
  const LPBYTE Cache(DWORD dwStart, DWORD dwIdealSize = 0)
#endif
  {
    //if(!IsOpen())goto ERR_CACHE2;

    DWORD dwFileRemain = GetRemainFile(dwStart);
    if(dwFileRemain==0)goto ERR_CACHE2;
    DWORD dwReadSize = dwIdealSize;
    if(dwReadSize==0)dwReadSize = min(m_dwCacheAllocSize, dwFileRemain);

    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwReadSize);
    if(pCacheTry)return pCacheTry;

    if(dwReadSize > dwFileRemain)dwReadSize = dwFileRemain;
    if(dwReadSize > m_dwCacheAllocSize)dwReadSize = m_dwCacheAllocSize;
ATLTRACE("ReCache!! (Start:0x%08X, Size:0x%08X)\n", dwStart, dwReadSize);
    if(!Read(m_pCache, dwStart, dwReadSize))goto ERR_CACHE2;
    m_dwCacheStart = dwStart;
    m_dwCacheSize = dwReadSize;
    return m_pCache;

ERR_CACHE2:
    _ClearInternalCacheData();
    //ERR_CACHE:
    //ATLASSERT(FALSE);
    return NULL;
  }

#ifdef SFCC_CHECKCACHE
  const LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)
  {
    LPBYTE p = _CacheForce(dwStart, dwNeedSize);
    if(p)
    {
      LPBYTE pDebug = (LPBYTE)malloc(dwNeedSize);
      if(!Read(pDebug, dwStart, dwNeedSize))ATLASSERT(FALSE);
      if(memcmp(pDebug, p, dwNeedSize)!=0)ATLASSERT(FALSE);
    }
    return p;
  }
  const LPBYTE _CacheForce(DWORD dwStart, DWORD dwNeedSize)
#else
  const LPBYTE CacheForce(DWORD dwStart, DWORD dwNeedSize)
#endif
  {
    //if(!IsOpen())goto ERR_CACHEFORCE2;
    DWORD dwFileRemain = GetRemainFile(dwStart);
    if(dwFileRemain<dwNeedSize)return NULL;

    LPBYTE pCacheTry = _GetLPBYTE(dwStart, dwNeedSize);
    if(pCacheTry)return pCacheTry;

    pCacheTry = Cache(dwStart);
    DWORD dwCacheRemain = GetRemainCache(dwStart);
    if(dwCacheRemain >= dwNeedSize)return pCacheTry;

    //goto ERR_CACHEFORCE2;

//ERR_CACHEFORCE2:
    _ClearInternalCacheData();
    //ERR_CACHEFORCE:
    //ATLASSERT(FALSE);
    return NULL;
  }
  
  
  BOOL AddRef()
  {
    if(m_dwRefCount==0xFFffFFff)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    m_dwRefCount++;
    return TRUE;
  }
  BOOL DecRef()
  {
    if(m_dwRefCount==0)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    m_dwRefCount--;
    return TRUE;
  }
  _inline DWORD GetRefCount()
  {
    return m_dwRefCount;
  }
  
  _inline BOOL IsReadOnly()
  {
    return m_bReadOnly;
  }
  
  CString GetFilePath()
  {
    return m_filePath;
  }


private:
  CAtlFile m_file;
  CString m_filePath;
  DWORD	m_dwTotal;
  DWORD	m_dwTotalSavedFile;
  BOOL	m_bReadOnly;
  
  DWORD m_dwRefCount;

  CAtlArray<TAMAUndoRedo*> m_undo;
  size_t m_savedIndex;
  size_t m_redoIndex;

  DWORD m_dwHiddenStart;
  DWORD m_dwHiddenSize;

  struct _TAMAFILECHUNK_HEAD m_filemapHead;
  
  static const DWORD SFCC_CACHESIZE = 512*1024;//512KB //1024*1024*1; //1MB
  DWORD m_dwCacheStart;
  DWORD m_dwCacheSize;
  DWORD m_dwCacheAllocSize;
  LPBYTE m_pCache;
  
#ifdef SFC_EASYDEBUG
  CAtlFile m_dbgFile;

  BOOL _EasyDebug_CreateEasyDebugFile()
  {
    if(m_dbgFile.m_h)
    {
      _EasyDebug_OutputOp1(SFCOP_CLOSE_REOPEN);
      //_EasyDebug_Close();
    } else {
      TCHAR tmpDir[_MAX_PATH];
      TCHAR tmpFile[_MAX_PATH];
      if(!GetTempPath(_MAX_PATH, tmpDir))return FALSE;
      if(!GetTempFileName(tmpDir, _T("SFC"), 0, tmpFile))return FALSE;
      if(FAILED(m_dbgFile.Create(tmpFile, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS)))return FALSE;
      const char header[] = "SFCDBG1";
      if(FAILED(m_dbgFile.Write(header, 7)))return FALSE;
    }
    return TRUE;
  }
  
  BOOL _EasyDebug_OutputOp1(SfcOp op)
  {
    if(FAILED(m_dbgFile.Write(&(SfcOpCode[op]), 1)))return FALSE;
    m_dbgFile.Flush();
    return TRUE;
  }
  
  BOOL _EasyDebug_OutputOp2(SfcOp op, DWORD dw1)
  {
    if(FAILED(m_dbgFile.Write(&(SfcOpCode[op]), 1)))return FALSE;
    if(FAILED(m_dbgFile.Write(&dw1, 4)))return FALSE;
    m_dbgFile.Flush();
    return TRUE;
  }
  
  BOOL _EasyDebug_OutputOp3(SfcOp op, DWORD dw1, DWORD dw2)
  {
    if(FAILED(m_dbgFile.Write(&(SfcOpCode[op]), 1)))return FALSE;
    if(FAILED(m_dbgFile.Write(&dw1, 4)))return FALSE;
    if(FAILED(m_dbgFile.Write(&dw2, 4)))return FALSE;
    m_dbgFile.Flush();
    return TRUE;
  }
  
  BOOL _EasyDebug_Close()
  {
    m_dbgFile.Close();
    return TRUE;
  }
#endif

private:

  void _DeleteContents() 
  {
    //			UpdateAllViews(NULL);
    m_file.Close();
    _FileMap_DestroyAll();
    _UndoRedo_DestroyAll();
    m_filePath = _T("");
    m_dwTotal = 0;
    m_dwTotalSavedFile = 0;
    m_bReadOnly = FALSE;
    m_savedIndex = m_redoIndex = m_dwHiddenStart = m_dwHiddenSize = 0;
    _ClearInternalCacheData();
  }

  void* mallocMax(size_t *nIdealSize)
  {
    void *pAlloc = NULL;
    size_t nTrySize = *nIdealSize;
    while(nTrySize != 0)
    {
      pAlloc = malloc(nTrySize);
      if(pAlloc)
      {
        *nIdealSize = nTrySize;
        return pAlloc;
      }
      nTrySize/=2;
    }
    ATLASSERT(FALSE);
    *nIdealSize = 0;
    return NULL;
  }
  void* mallocMax(DWORD *dwIdealSize)
  {
    size_t nIdealSize = *dwIdealSize;
    void *pAlloc = mallocMax(&nIdealSize);
    *dwIdealSize = nIdealSize;
    return pAlloc;
  }

  BOOL _TAMAFILECHUNK_ShiftFileChunkR(TAMAFILECHUNK *fileChunk, CAtlFile *pWriteFile)
  {
    ATLASSERT(fileChunk);
    ATLASSERT(fileChunk->bSaved==FALSE);
    ATLASSERT(fileChunk->dataChunk->dataType == CHUNK_FILE);

    DWORD dwInsStart = _TAMAFILECHUNK_GetRealStartAddr(fileChunk);
    ATLASSERT(fileChunk->dwStart >= dwInsStart);
    DWORD dwInsSize = fileChunk->dwStart - dwInsStart;
    ATLASSERT(fileChunk->dwEnd >= fileChunk->dwStart);
    DWORD dwShiftSize = fileChunk->dwEnd - fileChunk->dwStart +1;

    DWORD dwRemain = dwShiftSize;
    if(dwInsSize==0 || dwRemain==0)return TRUE;
    DWORD dwBufSize = SHIFTBUFSIZE;
    DWORD dwCopySize = min(dwBufSize, dwRemain);
    LPBYTE buf = (LPBYTE)mallocMax(&dwCopySize);
    if(!buf)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwMoveStart = dwInsStart + dwShiftSize - dwCopySize;
    ATLTRACE("ShiftFileR: 0x%08X-0x%08X(%u-%u) (Size:%u) >>>[R 0x%08X(%u)]>>> 0x%08X-0x%08X(%u-%u) (Size:%u)\n", dwInsStart, dwInsStart+dwShiftSize-1, dwInsStart, dwInsStart+dwShiftSize-1, dwShiftSize, dwInsSize, dwInsSize, dwInsStart+dwInsSize, dwInsStart+dwInsSize+dwShiftSize-1, dwInsStart+dwInsSize, dwInsStart+dwInsSize+dwShiftSize-1, dwShiftSize);
    while(dwRemain!=0)
    {
      if(FAILED(m_file.Seek(dwMoveStart, FILE_BEGIN)) || FAILED(m_file.Read(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      if(FAILED(pWriteFile->Seek(dwMoveStart+dwInsSize, FILE_BEGIN)) || FAILED(pWriteFile->Write(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      dwRemain -= dwCopySize;
#ifdef DEBUG
      if(dwRemain==0)ATLASSERT(dwMoveStart==dwInsStart);
#endif
      dwMoveStart -= dwCopySize;
      dwCopySize = min(dwCopySize, dwRemain);
    }
    free(buf);
    return TRUE;
  }
  BOOL _TAMAFILECHUNK_ShiftFileChunkL(TAMAFILECHUNK *fileChunk, CAtlFile *pWriteFile)
  {
    ATLASSERT(fileChunk);
    ATLASSERT(fileChunk->bSaved==FALSE);
    ATLASSERT(fileChunk->dataChunk->dataType == CHUNK_FILE);

    DWORD dwDelStart = fileChunk->dwStart;
    ATLASSERT(_TAMAFILECHUNK_GetRealStartAddr(fileChunk) >= dwDelStart);
    DWORD dwDelSize = _TAMAFILECHUNK_GetRealStartAddr(fileChunk) - dwDelStart;
    ATLASSERT(fileChunk->dwEnd >= fileChunk->dwStart);
    DWORD dwShiftSize = fileChunk->dwEnd - fileChunk->dwStart +1;

    DWORD dwRemain = dwShiftSize;
    if(dwDelSize==0 || dwRemain==0)return TRUE;
    DWORD dwBufSize = SHIFTBUFSIZE;
    DWORD dwCopySize = min(dwBufSize, dwRemain);
    LPBYTE buf = (LPBYTE)mallocMax(&dwCopySize);
    if(!buf)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwMoveStart = dwDelStart;
    ATLTRACE("ShiftFileL: 0x%08X-0x%08X(%u-%u) (Size:%u) >>>[L -0x%08X(-%u)]>>> 0x%08X-0x%08X(%u-%u) (Size:%u)\n", dwMoveStart+dwDelSize, dwMoveStart+dwDelSize+dwShiftSize-1, dwMoveStart+dwDelSize, dwMoveStart+dwDelSize+dwShiftSize-1, dwCopySize, dwDelSize, dwDelSize, dwMoveStart, dwMoveStart+dwShiftSize-1, dwMoveStart, dwMoveStart+dwShiftSize-1, dwCopySize);
    while(dwRemain!=0)
    {
      if(FAILED(m_file.Seek(dwMoveStart+dwDelSize, FILE_BEGIN)) || FAILED(m_file.Read(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      if(FAILED(pWriteFile->Seek(dwMoveStart, FILE_BEGIN)) || FAILED(pWriteFile->Write(buf, dwCopySize)))
      {
        ATLASSERT(FALSE);
        break;
      }
      dwRemain -= dwCopySize;
      dwMoveStart += dwCopySize;
      dwCopySize = min(dwCopySize, dwRemain);
    }
    free(buf);
    return TRUE;
  }


  DWORD _GetFileLengthLimit4G(CAtlFile *file = NULL, BOOL bErrorMsg = FALSE)
  {
    CAtlFile *_file = &m_file;
    if(file)_file = file;
    DWORD dwSize = 0;
    ULARGE_INTEGER ulFileSize;
    ulFileSize.QuadPart = 0;
    if(SUCCEEDED(_file->GetSize(ulFileSize.QuadPart)))
    {
      if(ulFileSize.QuadPart > MAX_FILELENGTH)
      {
        if(bErrorMsg)
        {
          //CString strErrOver4G;
          //strErrOver4G.LoadString(IDS_ERR_OVER4G);
          //MessageBox(NULL, strErrOver4G, _T("Error"), MB_OK);
        }
        dwSize = MAX_FILELENGTH;
      } else dwSize = ulFileSize.LowPart;
    }
    return dwSize;
  }

  BOOL inline _UndoRedo_RemoveRange(size_t delStartIndex, size_t nDelSize, BOOL bFreeRawPointer = TRUE)
  {
    ATLASSERT(nDelSize);
    for(size_t i=0;i<nDelSize;i++)
    {
      if(!_UndoRedo_ReleaseByIndex(delStartIndex+i, bFreeRawPointer))
      {
        ATLASSERT(FALSE);
        FatalError();
        return FALSE;
      }
    }
    m_undo.RemoveAt(delStartIndex, nDelSize);
    if(delStartIndex<m_savedIndex)m_savedIndex -= nDelSize;
    if(delStartIndex<m_redoIndex)m_redoIndex -= nDelSize;
    if(delStartIndex<m_dwHiddenStart)m_dwHiddenStart -= nDelSize;
    return TRUE;
  }
  BOOL inline _UndoRedo_RemoveByIndex(size_t nDelIndex, BOOL bFreeRawPointer = TRUE)
  {
    if(!_UndoRedo_Release(m_undo.GetAt(nDelIndex), bFreeRawPointer))return FALSE;
    m_undo.RemoveAt(nDelIndex);
    if(nDelIndex < m_savedIndex)m_savedIndex--;
    if(nDelIndex < m_redoIndex)m_redoIndex--;
    if(nDelIndex < m_dwHiddenStart)m_dwHiddenStart--;
    return TRUE;
  }
  BOOL inline _UndoRedo_ReleaseByIndex(size_t nIndex, BOOL bFreeRawPointer = TRUE)
  {
    return _UndoRedo_Release(m_undo.GetAt(nIndex), bFreeRawPointer);
  }
  BOOL inline _UndoRedo_Release(TAMAUndoRedo *undo, BOOL bFreeRawPointer = TRUE)
  {
    if(undo==NULL)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(undo->dataNext)_TAMADataChunks_Release(undo->dataNext, undo->nDataNext, bFreeRawPointer);
    if(undo->dataPrev)_TAMADataChunks_Release(undo->dataPrev, undo->nDataPrev, bFreeRawPointer);
    free(undo);
    return TRUE;
  }
  BOOL _TAMADataChunks_Release(TAMADataChunk **chunks, DWORD dwChunks, BOOL bFreeRawPointer = TRUE)
  {
    for(DWORD i=0; i<dwChunks; i++)
    {
      ATLASSERT(chunks[i]);
      if(chunks[i])_TAMADataChunk_Release(chunks[i], bFreeRawPointer);
    }
    free(chunks);
    return TRUE;
  }
  // TRUE  - nRefCount==0
  // FALSE - nRefCount!=0
  BOOL inline _TAMADataChunk_Release(TAMADataChunk *chunk, BOOL bFreeRawPointer = TRUE)
  {
    BOOL bRet = FALSE;
    if(chunk==NULL)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(chunk->dataType==CHUNK_MEM && chunk->dataMem) bRet=_TAMADataBuf_Release(chunk->dataMem, bFreeRawPointer);
    else bRet = TRUE;
    free(chunk);
    return bRet;
  }
  BOOL inline _TAMADataBuf_Release(TAMADataBuf *pDataBuf, BOOL bFreeRawPointer = TRUE)
  {
    if(pDataBuf==NULL)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
#ifdef TRACE_TAMADATABUF
    ATLTRACE("-DataBuf(0x%08X) nRefCount: %d -> %d\n", pDataBuf, pDataBuf->nRefCount, pDataBuf->nRefCount-1);
#endif
    if(pDataBuf->nRefCount==0 || --(pDataBuf->nRefCount) == 0)
    {
      if(pDataBuf->pData && bFreeRawPointer)free(pDataBuf->pData);
      free(pDataBuf);
      return TRUE;
    }
    return FALSE;
  }

  //void LastErrorMessageBox()
  //{
  //  MessageBox(NULL, AtlGetErrorDescription(::GetLastError(), LANG_USER_DEFAULT), _T("Error"), MB_OK | MB_ICONERROR);
  //}

  inline TAMAFILECHUNK * _FileMap_LookUp(DWORD dwSearchOffset)
  {
    TAMAFILECHUNK findChunk, *pSplitChunk;
    findChunk.key = dwSearchOffset;
    pSplitChunk = RB_NFIND(_TAMAFILECHUNK_HEAD, &m_filemapHead, &findChunk);
    return pSplitChunk;
  }

  TAMAFILECHUNK* _TAMAFILECHUNK_Copy(TAMAFILECHUNK *fileChunk)
  {
    TAMAFILECHUNK *copyFileChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
    if(!copyFileChunk)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    copyFileChunk->dwEnd = fileChunk->dwEnd;
    copyFileChunk->dwStart = fileChunk->dwStart;
    copyFileChunk->dataChunk = _TAMADataChunk_Copy(fileChunk->dataChunk);
    if(!copyFileChunk->dataChunk)
    {
      ATLASSERT(FALSE);
      free(copyFileChunk);
      return NULL;
    }
    copyFileChunk->bSaved = fileChunk->bSaved;
    return copyFileChunk;
  }

  //pSplitChunk[dwStart,(dwSplitPoint),dwEnd] >>>
  // pNewFirstChunk[dwStart,dwSplitPoint-1] -(Split)- pSplitChunk[dwSplitPoint,dwEnd]
  BOOL _FileMap_SplitPoint(DWORD dwSplitPoint)
  {
    TAMAFILECHUNK *pSplitChunk;
    pSplitChunk = _FileMap_LookUp(dwSplitPoint);
    if(!pSplitChunk)
    {
      //ATLASSERT(FALSE);
      return TRUE;//return FALSE;
    }
    if(pSplitChunk->dwStart != dwSplitPoint)
    {
      TAMAFILECHUNK *pNewFirstChunk = _TAMAFILECHUNK_Copy(pSplitChunk);//(TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
      if(pNewFirstChunk==NULL)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
      DWORD dwFirstSize = dwSplitPoint - pSplitChunk->dwStart;
      pNewFirstChunk->dwEnd   = dwSplitPoint-1;
      ATLASSERT(pSplitChunk->dataChunk);
      DataChunkType type = pSplitChunk->dataChunk->dataType;
      //pNewFirstChunk->dataChunk->dataType = type;
      switch(type)
      {
      case CHUNK_FILE:
        pSplitChunk->dataChunk->dataFileAddr += dwFirstSize;
        pSplitChunk->dataChunk->dwSize -= dwFirstSize;
        pNewFirstChunk->dataChunk->dwSize = dwFirstSize;
        break;
      case CHUNK_MEM:
        pSplitChunk->dataChunk->dwSkipOffset += dwFirstSize;
        pSplitChunk->dataChunk->dwSize -= dwFirstSize;
        pNewFirstChunk->dataChunk->dwSize = dwFirstSize;
        break;
      default:
        ATLASSERT(FALSE);
        break;
      }
      __FileMap_LowInsert(pNewFirstChunk);
      pSplitChunk->dwStart = dwSplitPoint;
    }
    return TRUE;
  }

  inline TAMAFILECHUNK* __FileMap_LowInsert(TAMAFILECHUNK* pInsert)
  {
    return RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pInsert);
  }

  TAMAFILECHUNK* _FileMap_BasicInsert(DWORD dwStart, DWORD dwSize)
  {
    TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
    //DWORD dwStart = dwEnd - dwSize +1;
    DWORD dwEnd = _GetEndOffset(dwStart, dwSize);
    if(dwSize==0 || !pNewChunk)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    _FileMap_Shift(dwStart, dwSize);

    pNewChunk->dwStart = dwStart;
    pNewChunk->dwEnd = dwEnd;
    __FileMap_LowInsert(pNewChunk);
    return pNewChunk;
  }
  BOOL _FileMap_InsertMemAssign(DWORD dwInsStart, LPBYTE srcDataDetached, DWORD dwInsSize)
  {
    //DWORD dwInsEnd = _GetEndOffset(dwInsStart, dwInsSize);
    TAMADataChunk *dataChunk = _TAMADataChunk_CreateMemAssignRawPointer(dwInsSize, 0, srcDataDetached);
    if(!dataChunk)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    TAMAFILECHUNK *insChunk = _FileMap_BasicInsert(dwInsStart, dwInsSize);
    if(!insChunk)
    {
      ATLASSERT(FALSE);
      _TAMADataChunk_Release(dataChunk);
      return FALSE;
    }
    insChunk->dataChunk = dataChunk;

    //OptimizeFileMap(insChunk);
    return TRUE;
  }
  BOOL _FileMap_InsertMemCopy(DWORD dwInsStart, LPBYTE pSrcData, DWORD dwInsSize)
  {
    LPBYTE pMem = (LPBYTE)malloc(dwInsSize);
    if(!pMem)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    memcpy(pMem, pSrcData, dwInsSize);
    if(!_FileMap_InsertMemAssign(dwInsStart, pMem, dwInsSize))
    {
      ATLASSERT(FALSE);
      free(pMem);
      return FALSE;
    }
    return TRUE;
  }

  TAMAFILECHUNK* _TAMADataChunk_CreateFileChunk(TAMADataChunk *pDataChunk, DWORD dwStart)
  {
    TAMAFILECHUNK *pFileChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
    if(!pFileChunk)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    pFileChunk->dwStart = dwStart;
    pFileChunk->dataChunk = _TAMADataChunk_Copy(pDataChunk);
    if(!pFileChunk->dataChunk)
    {
      ATLASSERT(FALSE);
      free(pFileChunk);
      return NULL;
    }
    pFileChunk->dwEnd = dwStart + pDataChunk->dwSize - 1;
    return pFileChunk;
  }

  //return 0 ---- Failed
  DWORD _TAMADATACHUNKS_GetSumSize(TAMADataChunk **pDataChunks, DWORD nDataChunks)
  {
    DWORD dwSum = 0;
    for(DWORD i=0; i<nDataChunks; i++)
    {
      TAMADataChunk *pDC = pDataChunks[i];
      ATLASSERT(pDC->dwSize!=0);
      dwSum += pDC->dwSize;
      if(pDC->dwSize > dwSum)//Overflow check
      {
        ATLASSERT(FALSE);//Overflow
        return 0; //Failed
      }
    }
    return dwSum;
  }
  BOOL __FileMap_BasicInsertTAMADataChunks(DWORD dwInsStart, TAMADataChunk **ppDataChunks, DWORD nDataChunks)
  {
    DWORD dwFCStart = dwInsStart;
    for(DWORD i=0; i<nDataChunks; i++)
    {
      TAMADataChunk *pDC = ppDataChunks[i];
      ATLASSERT(pDC);
      TAMAFILECHUNK *pFileChunk = _TAMADataChunk_CreateFileChunk(pDC, dwFCStart);
      if(!pFileChunk)
      {
        ATLASSERT(FALSE);//FileMap壊れる
        return FALSE;
      }
      __FileMap_LowInsert(pFileChunk);
      dwFCStart += pDC->dwSize;
      if(dwFCStart < pDC->dwSize)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
    }
    return TRUE;
  }
  BOOL _FileMap_InsertTAMADataChunks(DWORD dwInsStart, TAMADataChunk **pDataChunks, DWORD nDataChunks, DWORD dwInsSize=0)
  {
#ifdef _DEBUG
    ATLASSERT(dwInsSize==0 || dwInsSize==_TAMADATACHUNKS_GetSumSize(pDataChunks, nDataChunks));
#endif
    if(dwInsSize==0) //0 --- Automatic Calc
    {
      dwInsSize = _TAMADATACHUNKS_GetSumSize(pDataChunks, nDataChunks);
      if(dwInsSize==0)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
    }
    _FileMap_Shift(dwInsStart, dwInsSize);
    BOOL bRetIns = __FileMap_BasicInsertTAMADataChunks(dwInsStart, pDataChunks, nDataChunks);
    if(!bRetIns)//Insert Failed
    {
      ATLASSERT(FALSE);//Try to fix
      if(!__FileMap_DeleteRange(dwInsStart, dwInsSize) || !_FileMap_Shift(dwInsStart, dwInsSize, FALSE))
      {
        ATLASSERT(FALSE);//fatal error
        FatalError();
        return FALSE;
      }
      return FALSE;
    }
    //OptimizeFileMap(insChunk);
    
    m_dwTotal += dwInsSize;
    ATLASSERT(m_dwTotal >= dwInsSize);

    return TRUE;
  }
  BOOL _FileMap_InsertFile(DWORD dwInsStart, DWORD dwInsSize, DWORD dwStartFileSpace)
  {
    //DWORD dwInsEnd = _GetEndOffset(dwInsStart, dwInsSize);
    TAMAFILECHUNK *insChunk = _FileMap_BasicInsert(dwInsStart, dwInsSize);
    if(!insChunk)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    TAMADataChunk *dataChunk = _TAMADataChunk_CreateFileChunk(dwInsSize, dwStartFileSpace);
    if(!dataChunk)
    {
      ATLASSERT(FALSE);
      FatalError();
      return FALSE;
    }
    insChunk->dataChunk = dataChunk;
    //insChunk->dwSkipOffset = 0;

    //OptimizeFileMap(insChunk);
    return TRUE;
  }
  TAMADataChunk* _TAMADataChunk_CreateFileChunk(DWORD dwSize, DWORD dwStartFileSpace)
  {
    TAMADataChunk *dataChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
    if(dataChunk==NULL)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
#ifdef TRACE_DATACHUNK
    ATLTRACE("_TAMADataChunk_CreateFileChunk(0x%08X)\n", dataChunk);
#endif
    dataChunk->dataType = CHUNK_FILE;
    dataChunk->dataFileAddr = dwStartFileSpace;
    dataChunk->dwSize = dwSize;
    dataChunk->dwSkipOffset = 0;
    return dataChunk;
  }

  TAMADataChunk* _TAMADataChunk_CreateMemAssignRawPointer(DWORD dwSize, DWORD dwSkipOffset, LPBYTE srcDataDetached)
  {
    TAMADataBuf *pTAMADataBuf = _TAMADataBuf_CreateAssign(srcDataDetached, 0/*nRefCount*/);
    if(!pTAMADataBuf)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
    TAMADataChunk *retDataChunk = _TAMADataChunk_CreateMemAssignTAMADataBuf(dwSize, dwSkipOffset, pTAMADataBuf);
    if(!retDataChunk)
    {
      ATLASSERT(FALSE);
      _TAMADataBuf_Release(pTAMADataBuf);
      return NULL;
    }
    return retDataChunk;
  }

  TAMADataChunk* _TAMADataChunk_CreateMemAssignTAMADataBuf(DWORD dwSize, DWORD dwSkipOffset, TAMADataBuf *pTAMADataBuf)
  {
    TAMADataChunk *dataChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
    if(dataChunk==NULL)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
#ifdef TRACE_DATACHUNK
    ATLTRACE("_TAMADataChunk_CreateMemAssignTAMADataBuf(0x%08X)\n", dataChunk);
#endif
    if(pTAMADataBuf)
    {
      pTAMADataBuf->nRefCount++;
#ifdef TRACE_TAMADATABUF
      ATLTRACE("+DataBuf(0x%08X) nRefCount: %d -> %d\n", pTAMADataBuf, pTAMADataBuf->nRefCount-1, pTAMADataBuf->nRefCount);
#endif
      dataChunk->dataMem = pTAMADataBuf;
    }
    dataChunk->dataType = CHUNK_MEM;
    dataChunk->dwSize = dwSize;
    dataChunk->dwSkipOffset = dwSkipOffset;
    return dataChunk;
  }
  TAMADataChunk* _TAMADataChunk_CreateMemNew(DWORD dwSize)
  {
    TAMADataChunk *dataChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
    if(dataChunk==NULL)
    {
      ATLASSERT(FALSE);
      return NULL;
    }
#ifdef TRACE_DATACHUNK
    ATLTRACE("_TAMADataChunk_CreateMemNew(0x%08X)\n", dataChunk);
#endif
    TAMADataBuf *pTAMADataBuf = _TAMADataBuf_CreateNew(dwSize, 1/*nRefCount*/);
    if(!pTAMADataBuf)
    {
      ATLASSERT(FALSE);
      free(dataChunk);
      return NULL;
    }
    dataChunk->dataMem = pTAMADataBuf;
    dataChunk->dataType = CHUNK_MEM;
    dataChunk->dwSize = dwSize;
    dataChunk->dwSkipOffset = 0;
    return dataChunk;
  }

  BOOL __FileMap_Del2(DWORD dwEnd, DWORD dwSize)
  {
    DWORD dwStart = dwEnd - dwSize +1;
    if(dwSize==0 || dwEnd==0xFFffFFff)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwShiftStart = dwEnd + 1;
    __FileMap_DeleteRange(dwStart, dwSize);
    _FileMap_Shift(dwShiftStart, dwSize, FALSE/*bPlus==FALSE (Shift-left: dwSize)*/);
    return TRUE;
  }
  BOOL _FileMap_Del(DWORD dwDelStart, DWORD dwDelSize)
  {
    DWORD dwDelEnd = _GetEndOffset(dwDelStart, dwDelSize);
    if(!__FileMap_Del2(dwDelEnd, dwDelSize))return FALSE;
    ATLASSERT(m_dwTotal >= dwDelSize);
    m_dwTotal -= dwDelSize;
    return TRUE;
  }

  BOOL _FileMap_Shift(DWORD dwShiftStart, DWORD dwShiftSize, BOOL bPlus = TRUE)
  {
    if(!_FileMap_SplitPoint(dwShiftStart))
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    if(bPlus)
    {
      TAMAFILECHUNK *pChunk;
      RB_FOREACH_REVERSE(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
      {
        if(pChunk->dwStart < dwShiftStart)break;
        pChunk->dwStart += dwShiftSize;
        pChunk->dwEnd   += dwShiftSize;
      }
    } else {
      TAMAFILECHUNK *pChunk;
      RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
      {
        if(pChunk->dwStart >= dwShiftStart)
        {
          pChunk->dwStart -= dwShiftSize;
          pChunk->dwEnd   -= dwShiftSize;
        }
      }
    }
    return TRUE;
  }

  inline TAMAFILECHUNK* __FileMap_LowFind(TAMAFILECHUNK* pFind)
  {
    return RB_FIND(_TAMAFILECHUNK_HEAD, &m_filemapHead, pFind);
  }

  inline TAMAFILECHUNK* __FileMap_LowPrev(TAMAFILECHUNK* pFind)
  {
    return RB_PREV(_TAMAFILECHUNK_HEAD, &m_filemapHead, pFind);
  }

  inline TAMAFILECHUNK* __FileMap_LowNext(TAMAFILECHUNK* pFind)
  {
    return RB_NEXT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pFind);
  }

  inline TAMAFILECHUNK* __FileMap_LowMin()
  {
    return RB_MIN(_TAMAFILECHUNK_HEAD, &m_filemapHead);
  }

  inline TAMAFILECHUNK* __FileMap_LowMax()
  {
    return RB_MAX(_TAMAFILECHUNK_HEAD, &m_filemapHead);
  }

#define RB_FOREACH_REVERSE_SAFE_NOXINIT(x, name, head, y)			\
  for (;					\
  ((x) != NULL) && ((y) = name##_RB_PREV(x), 1);		\
  (x) = (y))

  //low-level internal function for _FileMap_Del()... (not include to shift chunks)
  BOOL __FileMap_DeleteRange(DWORD dwStart, DWORD dwSize)
  {
    //DWORD dwStart = dwEnd - dwSize +1;
    DWORD dwEnd = _GetEndOffset(dwStart, dwSize);
    if(m_dwTotal==0)return TRUE;
    else dwEnd = min(dwEnd, m_dwTotal-1);
    if(dwSize==0 || dwEnd==0xFFffFFff || !_FileMap_SplitPoint(dwStart) || !_FileMap_SplitPoint(dwEnd+1))
    {
      ATLASSERT(FALSE);
      return FALSE;
    }

    TAMAFILECHUNK findChunk;
    findChunk.key = dwEnd;
    TAMAFILECHUNK *pChunk = __FileMap_LowFind(&findChunk);
    if(pChunk==NULL)
    {
      //Merge (Restore split)
      ATLASSERT(FALSE);
      return FALSE;
    }
    TAMAFILECHUNK *pPrevChunk;
    RB_FOREACH_REVERSE_SAFE_NOXINIT(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead, pPrevChunk)
    {
      _FileMap_Remove(pChunk);
      if(!pPrevChunk || pPrevChunk->dwEnd < dwStart)break;
    }
    return TRUE;
  }

  inline TAMAFILECHUNK* __FileMap_LowRemove(TAMAFILECHUNK *pRemove)
  {
    return RB_REMOVE(_TAMAFILECHUNK_HEAD, &m_filemapHead, pRemove);
  }

  void _FileMap_Remove(TAMAFILECHUNK *pDeleteChunk)
  {
    switch(pDeleteChunk->dataChunk->dataType)
    {
    case CHUNK_FILE:
    case CHUNK_MEM:
      _TAMADataChunk_Release(pDeleteChunk->dataChunk);
      break;
    default:
      ATLASSERT(FALSE);
      break;
    }
    __FileMap_LowRemove(pDeleteChunk);
    free(pDeleteChunk);
  }

  //TAMAFILECHUNK * _FileMap_BasicOverWriteMem(DWORD dwStart, DWORD dwSize)
  //{
  //	TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
  //	//DWORD dwStart = dwEnd - dwSize +1;
  //	DWORD dwEnd = _GetEndOffset(dwStart, dwSize);
  //	if(dwSize==0 || !pNewChunk)return NULL;
  //	__FileMap_DeleteRange(dwStart, dwSize);
  //
  //	pNewChunk->dwStart = dwStart;
  //	pNewChunk->dwEnd = dwEnd;
  //	pNewChunk->dataChunk = NULL;
  //	pNewChunk->dwSkipOffset = 0;
  //	__FileMap_LowInsert(pNewChunk);
  //	return pNewChunk;
  //}

  //BOOL _FileMap_OverWriteMem(DWORD dwStart, DWORD dwSize, TAMADataChunk *dataChunk, DWORD dwSkipOffset)
  //{
  //	TAMAFILECHUNK *chunk = _FileMap_BasicOverWriteMem(dwStart, dwSize);
  //	if(chunk==NULL)
  //	{
  //		ATLASSERT(FALSE);
  //		return FALSE;
  //	}
  //	chunk->dwSkipOffset = dwSkipOffset;
  //	dataChunk->nRefCount++;
  //	chunk->dataChunk = dataChunk;
  //
  //	return TRUE;
  //}
  BOOL _FileMap_OverWriteTAMADataChunks(DWORD dwStart, TAMADataChunk **pDataChunks, DWORD nDataChunks, DWORD dwSize=0)
  {
#ifdef _DEBUG
    //ATLASSERT(dwSize==0 || dwSize==_TAMADATACHUNKS_GetSumSize(pDataChunks, nDataChunks));
#endif
    if(dwSize==0) //0 --- Automatic Calc
    {
      dwSize = _TAMADATACHUNKS_GetSumSize(pDataChunks, nDataChunks);
      if(dwSize==0)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
    }
    if(		!__FileMap_DeleteRange(dwStart, dwSize)
      || 	!__FileMap_BasicInsertTAMADataChunks(dwStart, pDataChunks, nDataChunks)
      )
    {
      ATLASSERT(FALSE);//fatal error
      FatalError();
      return FALSE;
    }
    //OptimizeFileMap(sideChunk);

    TAMAFILECHUNK *fileChunk = __FileMap_LowMax();
    if(fileChunk==NULL)m_dwTotal = 0;
    else m_dwTotal = fileChunk->dwEnd+1;

    return TRUE;
  }

  BOOL _TAMAFILECHUNK_IsContainPoint(TAMAFILECHUNK *fileChunk, DWORD dwPoint) { return fileChunk->dwStart <= dwPoint && dwPoint <= fileChunk->dwEnd; }

  BOOL _FileMap_CreateTAMADataChunks(DWORD dwStart, DWORD dwSize, TAMADataChunk*** ppDataChunks, DWORD *nDataChunks)
  {
    TAMAFILECHUNK *pFCStart = _FileMap_LookUp(dwStart);
    if(!pFCStart)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    DWORD dwEnd = _GetEndOffset(dwStart, dwSize);
    TAMAFILECHUNK *pFCCur = pFCStart;
    DWORD dwDataChunksSize = 1;
    while(1)
    {
      if(_TAMAFILECHUNK_IsContainPoint(pFCCur, dwEnd))break;
      pFCCur = __FileMap_LowNext(pFCCur);
      if(!pFCCur)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
      ATLASSERT(dwDataChunksSize<0xFFffFFff);
      dwDataChunksSize++;
      ATLASSERT(dwEnd > pFCCur->dwStart);
      ATLASSERT(dwStart < pFCCur->dwEnd);
    }
    TAMAFILECHUNK *pFCEnd = pFCCur;
    TAMADataChunk **pDataChunks = (TAMADataChunk **)malloc(sizeof(TAMADataChunk *)*dwDataChunksSize);
    if(!pDataChunks)
    {
      ATLASSERT(FALSE);
      return FALSE;
    }
    pFCCur = pFCStart;
    TAMADataChunk *pDC = _TAMADataChunk_Copy(pFCStart->dataChunk);
    if(!pDC)
    {
      ATLASSERT(FALSE);
      free(pDataChunks);
      return FALSE;
    }
    pDataChunks[0] = pDC;
    ATLASSERT(dwStart >= pFCStart->dwStart);
    DWORD dwSkipS = dwStart - pFCStart->dwStart;
    pDC->dwSkipOffset += dwSkipS;
    pDC->dwSize -= dwSkipS;

    for(DWORD i=1; ; i++)
    {
      if(pFCCur==pFCEnd)
      {
        *nDataChunks = i;
        ATLASSERT(dwEnd<=pFCEnd->dwEnd);
        DWORD dwDecE = pFCEnd->dwEnd - dwEnd;
        ATLASSERT(pDC->dwSize > dwDecE);
        pDC->dwSize -= dwDecE;
        break;
      }
      pFCCur = __FileMap_LowNext(pFCCur);
      if(!pFCCur || !(pDC = _TAMADataChunk_Copy(pFCCur->dataChunk)))
      {
        ATLASSERT(FALSE);
        _TAMADataChunks_Release(pDataChunks, i);
        return FALSE;
      }
      ATLASSERT(i<dwDataChunksSize);
      pDataChunks[i] = pDC;
    }
    *ppDataChunks = pDataChunks;
    return TRUE;
  }


  BOOL _OldFileMap_Make(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, CAtlFile *pFile)
  {
    RB_INIT(pOldFilemapHead);
    {//init map
      TAMAOLDFILECHUNK * c = (TAMAOLDFILECHUNK *)malloc(sizeof(TAMAOLDFILECHUNK));
      if(c==NULL)
      {
        _OldFileMap_FreeAll(pOldFilemapHead, TRUE);
        return FALSE;
      }
      c->type = OF_NOREF;
      c->dwStart=0;
      c->dwEnd=m_dwTotalSavedFile-1;
      RB_INSERT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, c);
    }

    //searching...
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      TAMADataChunk *dataChunk = pChunk->dataChunk;
      ATLASSERT(dataChunk);
      switch(dataChunk->dataType)
      {
      case CHUNK_FILE:
        {
          ATLASSERT(dataChunk->dwSize>0);
          if(dataChunk->dwSize > 0)
          {
            DWORD dwOldFileAddrS = _TAMADataChunk_GetRealStartAddr(dataChunk);
            DWORD dwOldFileAddrE = dwOldFileAddrS + dataChunk->dwSize - 1;
            _OldFileMap_FF(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE, pChunk->dwStart);
            //dataChunk->savingType = DC_FF;
          }
          break;
        }
      case CHUNK_MEM:
        break;
      default:
        ATLASSERT(FALSE);
        break;
      }
    }
    size_t nUndo = m_undo.GetCount();
    for(size_t i=0; i < nUndo; i++)
    {
      TAMAUndoRedo *undo = m_undo[i];
      ATLASSERT(undo);
      TAMADataChunk **pDataChunks = undo->dataNext;
      DWORD nDataChunks = undo->nDataNext;
      for(size_t j=0; j<nDataChunks; j++)
      {
        TAMADataChunk *dataChunk = pDataChunks[j];
        if(dataChunk && dataChunk->dataType==CHUNK_FILE)
        {
          ATLASSERT(dataChunk->dwSize>0);
          if(dataChunk->dwSize > 0)
          {
            DWORD dwOldFileAddrS = _TAMADataChunk_GetRealStartAddr(dataChunk);
            DWORD dwOldFileAddrE = dwOldFileAddrS + dataChunk->dwSize - 1;
            _OldFileMap_FD(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE);
            //dataChunk->savingType = DC_FD;
          }
        }
      }
      pDataChunks = undo->dataPrev;
      nDataChunks = undo->nDataPrev;
      for(size_t j=0; j<nDataChunks; j++)
      {
        TAMADataChunk *dataChunk = pDataChunks[j];
        if(dataChunk && dataChunk->dataType==CHUNK_FILE)
        {
          ATLASSERT(dataChunk->dwSize>0);
          if(dataChunk->dwSize > 0)
          {
            DWORD dwOldFileAddrS = _TAMADataChunk_GetRealStartAddr(dataChunk);
            DWORD dwOldFileAddrE = dwOldFileAddrS + dataChunk->dwSize - 1;
            _OldFileMap_FD(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE);
            //dataChunk->savingType = DC_FD;
          }
        }
      }
    }

    //FDチャンクのメモリ確保。ファイルからデータ読み込み
    TAMAOLDFILECHUNK *pOldFileChunk;
    RB_FOREACH(pOldFileChunk, _TAMAOLDFILECHUNK_HEAD, pOldFilemapHead)
    {
      if(pOldFileChunk->type==OF_FD)
      {
        ATLASSERT(pOldFileChunk->dwEnd >= pOldFileChunk->dwStart);
        DWORD dwReadSize = pOldFileChunk->dwEnd - pOldFileChunk->dwStart + 1;
        pOldFileChunk->pDataBuf = _TAMADataBuf_CreateNew(dwReadSize, 1/*nRefCount*/);
#ifdef TRACE_READFILE
        ATLTRACE("ConvFD: Start:0x%08X(%u), Size:0x%08X(%u)\n", pOldFileChunk->dwStart, pOldFileChunk->dwStart, dwReadSize, dwReadSize);
#endif
        if(pOldFileChunk->pDataBuf==NULL || FAILED(pFile->Seek(pOldFileChunk->dwStart, FILE_BEGIN)) || FAILED(pFile->Read(pOldFileChunk->pDataBuf->pData, dwReadSize)))
        {
          ATLASSERT(FALSE);
          _OldFileMap_FreeAll(pOldFilemapHead, TRUE);
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  void _OldFileMap_FreeAll(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, BOOL bFreeBuffer = TRUE)
  {
    TAMAOLDFILECHUNK *pChunk, *pChunkNext;
    RB_FOREACH_SAFE(pChunk, _TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunkNext)
    {
      _OldFileMap_Free(pOldFilemapHead, pChunk, bFreeBuffer);
    }
  }

  void _OldFileMap_Free(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, TAMAOLDFILECHUNK *pChunk, BOOL bFreeBuffer = TRUE)
  {
    if(bFreeBuffer && pChunk->type==OF_FD && pChunk->pDataBuf)_TAMADataBuf_Release(pChunk->pDataBuf);
    RB_REMOVE(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
    free(pChunk);
  }

  inline TAMAOLDFILECHUNK * _OldFileMap_LookUp(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, DWORD dwSearchOffset)
  {
    TAMAOLDFILECHUNK findChunk, *pSplitChunk;
    findChunk.key = dwSearchOffset;
    pSplitChunk = RB_NFIND(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, &findChunk);
    return pSplitChunk;
  }

  BOOL _OldFileMap_SplitPoint(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, DWORD dwSplitPoint)
  {
    TAMAOLDFILECHUNK *pSplitChunk;
    pSplitChunk = _OldFileMap_LookUp(pOldFilemapHead, dwSplitPoint);
    ATLASSERT(pSplitChunk->type!=OF_FD);
    if(!pSplitChunk)
    {
      //ATLASSERT(FALSE);
      return TRUE;//return FALSE;
    }
    if(pSplitChunk->dwStart != dwSplitPoint)
    {
      TAMAOLDFILECHUNK *pNewFirstChunk = (TAMAOLDFILECHUNK *)malloc(sizeof(TAMAOLDFILECHUNK));
      if(pNewFirstChunk==NULL)
      {
        ATLASSERT(FALSE);
        return FALSE;
      }
      DWORD dwFirstSize = dwSplitPoint - pSplitChunk->dwStart;
      pNewFirstChunk->dwStart = pSplitChunk->dwStart;
      pNewFirstChunk->dwEnd   = dwSplitPoint-1;
      pNewFirstChunk->type = pSplitChunk->type;
      pNewFirstChunk->dwNewFileAddr = pSplitChunk->dwNewFileAddr;
      RB_INSERT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pNewFirstChunk);
      pSplitChunk->dwStart = dwSplitPoint;
      if(pSplitChunk->type==OF_FF)pSplitChunk->dwNewFileAddr += dwFirstSize;
      ATLASSERT(pSplitChunk->dwNewFileAddr >= pNewFirstChunk->dwNewFileAddr);
    }
    return TRUE;
  }

  void _OldFileMap_MeltFF(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, TAMAOLDFILECHUNK *pChunk)
  {
    ATLASSERT(pChunk->type==OF_FF);
    TAMAOLDFILECHUNK *pChunk2 = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
    if(pChunk2 && pChunk2->type == OF_FF && pChunk2->dwNewFileAddr - pChunk->dwNewFileAddr == pChunk2->dwStart - pChunk->dwStart)
    {
      DWORD dwNewEnd = pChunk2->dwEnd;
      RB_REMOVE(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk2);
      free(pChunk2);
      pChunk->dwEnd = dwNewEnd;
    }

    pChunk2 = RB_PREV(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
    if(pChunk2 && pChunk2->type == OF_FF && pChunk->dwNewFileAddr - pChunk2->dwNewFileAddr == pChunk->dwStart - pChunk2->dwStart)
    {
      DWORD dwNewStart = pChunk2->dwStart;
      DWORD dwNewFileAddr = pChunk2->dwNewFileAddr;
      RB_REMOVE(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk2);
      free(pChunk2);
      pChunk->dwStart = dwNewStart;
      pChunk->dwNewFileAddr = dwNewFileAddr;
    }
  }

  void _OldFileMap_FF(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, DWORD dwStart, DWORD dwEnd, DWORD dwNewFileAddr)
  {
    TAMAOLDFILECHUNK *pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
    ATLASSERT(pOldFileChunkS);
    if(pOldFileChunkS->type == OF_FF && pOldFileChunkS->dwEnd >= dwEnd && dwNewFileAddr - pOldFileChunkS->dwNewFileAddr == dwStart - pOldFileChunkS->dwStart)return;
    if(pOldFileChunkS->dwStart < dwStart && (pOldFileChunkS->type != OF_FF || dwNewFileAddr - pOldFileChunkS->dwNewFileAddr != dwStart - pOldFileChunkS->dwStart))
    {
      _OldFileMap_SplitPoint(pOldFilemapHead, dwStart);
      pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
    }

    TAMAOLDFILECHUNK *pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
    ATLASSERT(pOldFileChunkE);
    if(pOldFileChunkE->dwEnd > dwEnd && (pOldFileChunkE->type != OF_FF || dwNewFileAddr - pOldFileChunkE->dwNewFileAddr != dwStart - pOldFileChunkE->dwStart))
    {
      _OldFileMap_SplitPoint(pOldFilemapHead, dwEnd+1);
      pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
      pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
    }

    TAMAOLDFILECHUNK *pChunk = pOldFileChunkS;
    while(1)
    {
      if(pChunk->type != OF_FF || (pChunk->dwStart >= dwStart && pChunk->dwEnd <= dwEnd))
      {
        pChunk->type = OF_FF;
        pChunk->dwNewFileAddr = dwNewFileAddr - (dwStart - pChunk->dwStart);
        ATLASSERT(pChunk->dwNewFileAddr >= dwNewFileAddr);
      }
      _OldFileMap_MeltFF(pOldFilemapHead, pChunk);
      if(pChunk->dwEnd >= dwEnd)break;
      pChunk = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
      ATLASSERT(pChunk);
    }
  }

  void _OldFileMap_MeltFD(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, TAMAOLDFILECHUNK *pChunk)
  {
    ATLASSERT(pChunk->type==OF_FD);
    TAMAOLDFILECHUNK *pChunk2 = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
    if(pChunk2 && pChunk2->type == OF_FD)
    {
      DWORD dwNewEnd = pChunk2->dwEnd;
      RB_REMOVE(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk2);
      free(pChunk2);
      pChunk->dwEnd = dwNewEnd;
    }

    pChunk2 = RB_PREV(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
    if(pChunk2 && pChunk2->type == OF_FD)
    {
      DWORD dwNewStart = pChunk2->dwStart;
      RB_REMOVE(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk2);
      free(pChunk2);
      pChunk->dwStart = dwNewStart;
    }
  }

  void _OldFileMap_FD(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, DWORD dwStart, DWORD dwEnd)
  {
    TAMAOLDFILECHUNK *pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
    ATLASSERT(pOldFileChunkS);
    if(pOldFileChunkS->type == OF_FD && pOldFileChunkS->dwEnd >= dwEnd)return;
    if(pOldFileChunkS->type == OF_NOREF && pOldFileChunkS->dwStart < dwStart)
    {
      _OldFileMap_SplitPoint(pOldFilemapHead, dwStart);
      pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
    }

    TAMAOLDFILECHUNK *pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
    ATLASSERT(pOldFileChunkE);
    if(pOldFileChunkE->type == OF_NOREF && pOldFileChunkE->dwEnd > dwEnd)
    {
      _OldFileMap_SplitPoint(pOldFilemapHead, dwEnd+1);
      pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
      pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
    }

    TAMAOLDFILECHUNK *pChunk = pOldFileChunkS;
    while(1)
    {
      if(pChunk->type == OF_NOREF)
      {
        pChunk->type = OF_FD;
        pChunk->pDataBuf = NULL;
        _OldFileMap_MeltFD(pOldFilemapHead, pChunk);
      }
      if(pChunk->dwEnd >= dwEnd)break;
      pChunk = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
      ATLASSERT(pChunk);
    }
  }

#ifdef GTEST
//#ifdef _DEBUG
  bool _FileMap_DEBUG_ValidationCheck()
  {
    DWORD nextStart = 0;
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      if(pChunk->dwStart!=nextStart)
      {
        ATLASSERT(FALSE);
        return false;
      }
      if(!_TAMAFILECHUNK_TAMADataChunk_DEBUG_SizeCheck(pChunk))
      {
        ATLASSERT(FALSE);
        return false;
      }
      nextStart=pChunk->dwEnd+1;
    }
    TAMAFILECHUNK *pMaxChunk = __FileMap_LowMax();
    if(!pMaxChunk || m_dwTotal != pMaxChunk->dwEnd + 1)
    {
      ATLASSERT(FALSE);
      return false;
    }
    return true;
  }

  bool _TAMAFILECHUNK_TAMADataChunk_DEBUG_SizeCheck(TAMAFILECHUNK *fileChunk)
  {
    ATLASSERT(fileChunk->dwEnd >= fileChunk->dwStart);
    DWORD dwFileChunkSize = fileChunk->dwEnd - fileChunk->dwStart + 1;
    ATLASSERT(fileChunk->dataChunk->dwSize >= dwFileChunkSize);
    return fileChunk->dwEnd >= fileChunk->dwStart && fileChunk->dataChunk->dwSize >= dwFileChunkSize;
  }

  DWORD _FileMap_DEBUG_GetCount()
  {
    DWORD c = 0;
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      c++;
    }
    return c;
  }
  BOOL _Debug_SearchUnSavedChunk()
  {
    TAMAFILECHUNK *pChunk;
    RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
    {
      if(!pChunk->bSaved)
      {
        return TRUE;
      }
    }
    return FALSE;
  }
//#endif
#endif

};
