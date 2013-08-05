#pragma once

#include "tree.h"

#define MAX_FILELENGTH  0xFFFFFFF0


typedef enum {	UNDO_INS, UNDO_OVR, UNDO_DEL } UndoMode;
typedef enum {	CHUNK_FILE, /*CHUNK_UNDO,*/ CHUNK_MEM } ChunkType;
typedef enum {	DC_UNKNOWN, DC_FD, DC_FF, DC_DF, DC_DD, DC_DONE } DataChunkSavingType;
typedef struct _TAMADataBuf
{
	LPBYTE pData;
	DWORD nRefCount;
} TAMADataBuf;
typedef struct _TAMADataChunk
{
	ChunkType dataType;
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
	return c2->dwEnd - c1->dwEnd;
}
RB_HEAD(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK);
RB_PROTOTYPE(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK);



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
		LPBYTE pMem;
	};
} TAMAOLDFILECHUNK;
static int cmpTAMAOLDFILECHUNK(TAMAOLDFILECHUNK *c1, TAMAOLDFILECHUNK *c2)
{
	return c2->dwEnd - c1->dwEnd;
}
RB_HEAD(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK);
RB_PROTOTYPE(_TAMAOLDFILECHUNK_HEAD, _TAMAOLDFILECHUNK, linkage, cmpTAMAOLDFILECHUNK);



class CSuperFileCon
{
public:

	CSuperFileCon(void)
	{
		RB_INIT(&m_filemapHead);
	}
	~CSuperFileCon(void)
	{
	}

// Open/Overwrite/Save to another file
	BOOL Open(LPCTSTR lpszPathName)
	{
#ifdef DEBUG
		if (m_bModified) TRACE("Warning: Open replaces an unsaved document.\n");
#endif
		m_file.Close();
		if (FAILED(m_file.Create(lpszPathName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)))
		{
			LastErrorMessageBox();
			return FALSE;
		}
		_DeleteContents();
		m_bModified = true;

		// ------ File Mapping ----->
		DWORD dwSize = _GetFileLengthLimit4G(TRUE);
		if(dwSize >= options.dwMaxOnMemory) {
			m_bReadOnly = options.bReadOnlyOpen || ::GetFileAttributes(lpszPathName) & FILE_ATTRIBUTE_READONLY;
			if(!m_bReadOnly)
			{
				m_file.Close();
				if((FAILED(m_file.Create(lpszPathName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING))))  // ReOpen (+Write)
				{
					if (FAILED(m_file.Create(lpszPathName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) //Retry open (Read-Only)
					{
						LastErrorMessageBox();
						return FALSE; //Failed open
					}
					m_bReadOnly = true;
				}
			}
//			m_pFileMapping = pFile;
			m_hMapping = ::CreateFileMapping(m_file, NULL, m_bReadOnly ? PAGE_READONLY : PAGE_READWRITE
				, 0, 0, /*options.bReadOnlyOpen ? 0 : dwSize + options.dwMaxOnMemory,*/ NULL);
			if(!m_hMapping) {
				LastErrorMessageBox();
				m_file.Close();
				return FALSE;
			}
		}
		// <----- File Mapping ------

		CWaitCursor wait;
		if (_GetFileLengthLimit4G() != 0)
		{
			m_dwTotalSavedFile = m_dwTotal = _GetFileLengthLimit4G();
			if(IsFileMapping())
			{
				if(!_MapView())
				{
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
				_FileMap_InsertFile(0, m_dwTotal, 0);
			} else {
				LPBYTE pData;
				if(!(pData = (LPBYTE)malloc(m_dwTotal))) {
					AfxMessageBox(IDS_ERR_MALLOC);
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
				DWORD len = m_file.Read(pData, m_dwTotal);
				if(len < m_dwTotal) {
					AfxMessageBox(IDS_ERR_READFILE);
					free(pData);	// ###1.61
					//pData = NULL;
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
				m_file.Close();
				m_bReadOnly = options.bReadOnlyOpen;
				_FileMap_ClearAll();
				_UndoRedo_ClearAll();
				ASSERT(_FileMap_InsertMem(0, pData, m_dwTotal));
			}
		}
		m_bModified = FALSE;
		m_filePath = lpszPathName;

		return TRUE;
	}
	void _ClearSavedFlags()
	{
		TAMAFILECHUNK *pChunk;
		RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			ASSERT(pChunk->dataChunk);
			pChunk->bSaved = FALSE;
			if(pChunk->dataChunk->dataType == CHUNK_FILE && pChunk->dwStart == _GetRealSrcFileStartAddr(pChunk) /*サイズも確認したほうがいい？*/)
				pChunk->bSaved = TRUE; //file change nothing
		}
	}
	void _ClearDataChunkSavingFlag()
	{
		TAMAFILECHUNK *pChunk;
		RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			ASSERT(pChunk->dataChunk);
			pChunk->dataChunk->savingType = DC_UNKNOWN;
		}
		size_t nUndo = m_undo.GetCount();
		for(size_t i=0; i < nUndo; i++)
		{
			TAMAUndoRedo *undo = m_undo[i];
			ASSERT(undo);
			if(undo->dataNext)undo->dataNext->savingType = DC_UNKNOWN;
			if(undo->dataPrev)undo->dataPrev->savingType = DC_UNKNOWN;
		}
	}
	BOOL _SetFileSize(DWORD newSize)
	{
		m_file.SetSize(newSize);
	}
	BOOL _ExtendFileSize()
	{
		if(m_dwTotal <= m_dwTotalSavedFile)return TRUE;
		_SetFileSize(m_dwTotal);
		TAMAFILECHUNK *fileChunk = RB_MAX(_TAMAFILECHUNK_HEAD, &m_filemapHead);
		while(fileChunk && fileChunk->dwEnd >= m_dwTotalSavedFile)
		{
			if(!fileChunk->bSaved)
			{
				switch(fileChunk->dataChunk->dataType)
				{
				case CHUNK_FILE:
					{
						if(!_DoShiftFileChunk(fileChunk))return FALSE;
					}
				case CHUNK_MEM:
					{
						if(fileChunk->dwStart < m_dwTotalSavedFile-1)
						{
							DWORD dwShiftStart = m_dwTotalSavedFile-1;
							_FileMap_SplitPoint(dwShiftStart);
							fileChunk = RB_MAX(_TAMAFILECHUNK_HEAD, &m_filemapHead);
						}
						if(!_WriteMemChunk(fileChunk))return FALSE;
					}
				default:
					ASSERT(FALSE);
					break;
				}
			}
			fileChunk = RB_PREV(_TAMAFILECHUNK_HEAD, &m_filemapHead, fileChunk);
		}
		return TRUE;
	}
	BOOL _WriteMemChunk(TAMAFILECHUNK *memChunk)
	{
		ASSERT(memChunk);
		ASSERT(memChunk->dataChunk->dataType == CHUNK_MEM);
		ASSERT(memChunk->dataChunk->dataMemPointer);
		m_file.Seek(memChunk->dwStart);
		DWORD dwWriteSize = memChunk->dwEnd - memChunk->dwStart +1;
		return SUCCEEDED(m_file.Write(_GetRealSrcMemStartAddr(memChunk), dwWriteSize));
	}
	BOOL _isRightShiftFileChunk(TAMAFILECHUNK *fileChunk)
	{
		ASSERT(fileChunk->dwStart != _GetRealSrcFileStartAddr(fileChunk)); //No move
		if(fileChunk->dwStart < _GetRealSrcFileStartAddr(fileChunk))return FALSE;
		return TRUE;
	}
	BOOL _ShiftAllFF()
	{
		TAMAFILECHUNK *pLastFileChunk = NULL;
		TAMAFILECHUNK *pChunk;
		RB_FOREACH_REVERSE(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			if(!pChunk->bSaved && pChunk->dataChunk->dataType == CHUNK_FILE)
			{
				pLastFileChunk = pChunk;
				if(_isRightShiftFileChunk(pChunk))
				{
					if(!_ShiftAllFileChunksAfterArg(pChunk))
					{
						ASSERT(FALSE);
						return FALSE;
					}
				}
			}
		}
		if(pLastFileChunk && !pLastFileChunk->bSaved)
		{
			if(!_ShiftAllFileChunksAfterArg(pChunk))
			{
				ASSERT(FALSE);
				return FALSE;
			}
		}
		return TRUE;
	}
	BOOL _WriteAllDF()
	{
		TAMAFILECHUNK *pChunk;
		RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			if(!pChunk->bSaved)
			{
				ASSERT(pChunk->dataChunk->dataType == CHUNK_MEM);
				if(_WriteMemChunk(pChunk)==FALSE)return FALSE;
				pChunk->bSaved = TRUE;
			}
		}
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
	void _UpdateAllDataChunkSavingType(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead)//!!!!!!!!!!!!!!重複使用が考慮されていない
	{
		_ClearDataChunkSavingFlag();
		
		TAMAFILECHUNK *pChunk;
		RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			ASSERT(pChunk);
			TAMADATACHUNK *dataChunk = pChunk->dataChunk;
			ASSERT(dataChunk);
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
					ASSERT(FALSE);
					break;
				}
			}
		}
		size_t nUndo = m_undo.GetCount();
		for(size_t i=0; i < nUndo; i++)
		{
			TAMAUndoRedo *undo = m_undo[i];
			ASSERT(undo);
			TAMADATACHUNK **dataChunks = undo->dataNext;
			DWORD nDataChunks = undo->nDataNext;
			for(DWORD i=0;i<nDataChunks;i++)
			{
				TAMADATACHUNK dataChunk = dataChunks[i];
				if(dataChunk && dataChunk->savingType == DC_UNKNOWN && dataChunk->dataType==CHUNK_FILE)
				{
					ASSERT(dataChunk->dwSize > 0);
					DWORD dwOldFileAddrS = dataChunk->dataFileAddr + dataChunk->dwSkipOffset
					TAMAOLDFILECHUNK *oldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwOldFileAddrS);
					DWORD nOldFileChunkS = oldFileChunkS->dwEnd-oldFileChunkS->dwStart+1;
					ASSERT(nOldFileChunkS > 0);
					if(nOldFileChunkS < dataChunk->dwSize)
					{
						todo
					}
					dataChunk->savingType = DC_DONE;//DC_FD;
				}
			}
			dataChunks = undo->dataPrev;
			nDataChunks = undo->nDataPrev;
			for(DWORD i=0;i<nDataChunks;i++)
			{
				TAMADATACHUNK dataChunk = dataChunks[i];
				if(dataChunk && dataChunk->savingType == DC_UNKNOWN && dataChunk->dataType==CHUNK_FILE)
				{
					dataChunk->savingType = DC_FD;
				}
			}
		}
	}
	BOOL _ProccessAllChunks()
	{
		_ClearSavedFlags();
		struct _TAMAOLDFILECHUNK_HEAD oldFilemapHead;
		_OldFileMap_Make(&oldFilemapHead, m_file);
		_UpdateAllDataChunkSavingType(&oldFilemapHead);
		_OldFileMap_FreeAll(&oldFilemapHead, FALSE);
		if(!_ExtendFileSize())
		{
			ASSERT(FALSE);
			return FALSE;
		}
		if(!_ShiftAllFF())
		{
			ASSERT(FALSE);
			return FALSE;
		}
		if(!_WriteAllDF())
		{
			ASSERT(FALSE);
			return FALSE;
		}
#ifdef DEBUG
		ASSERT(!_Debug_SearchUnSavedChunk());
#endif
		_UndoRedo_CreateRefSrcFileDataChunk();
	}
	BOOL _ShiftAllFileChunksAfterArg(TAMAFILECHUNK *fileChunk)
	{
		while(fileChunk) {
			if(!fileChunk->bSaved && fileChunk->dataChunk->dataType == CHUNK_FILE)
			{
				if(!_DoShiftFileChunk(fileChunk))
				{
					ASSERT(FALSE);
					return FALSE;
				}
			}
			fileChunk = RB_NEXT(_TAMAFILECHUNK_HEAD, &m_filemapHead, fileChunk);
		}
		return TRUE;
	}
	BOOL _DoShiftFileChunk(TAMAFILECHUNK *fileChunk)
	{
		ASSERT(fileChunk);
		ASSERT(fileChunk->bSaved==FALSE);
		ASSERT(fileChunk->dataChunk->dataType == CHUNK_FILE);
		BOOL nRet;
		DWORD dwRefSrcFileAddr = _GetRealSrcFileStartAddr(fileChunk);
		DWORD dwDstFileAddr = fileChunk->dwStart;
		if(_isRightShiftFileChunk(fileChunk))
		{
			nRet = _ShiftFileDataR(dwRefSrcFileAddr, dwDstFileAddr - dwRefSrcFileAddr);
		} else {
			nRet = _ShiftFileDataL(dwDstFileAddr, dwRefSrcFileAddr - dwDstFileAddr);
		}
		if(!nRet)
		{
			ASSERT(FALSE);
			return FALSE;
		}
		fileChunk->bSaved = TRUE;
		return nRet;
	}
	//void _UpdateNewRefSrcFileAddress()
	//{
	//	TAMAFILECHUNK *pChunk;
	//	RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
	//	{
	//		ASSERT(pChunk->dataChunk);
	//		ASSERT(pChunk->bSaved);
	//		if(pChunk->dataChunk->dataType == CHUNK_FILE)
	//		{
	//			pChunk->dataChunk->dataFileAddr = pChunk->dataChunk->dwStart - pChunk->dwSkipOffset;
	//		}
	//	}
	//}
	DWORD _GetRealSrcFileStartAddr(TAMAFILECHUNK* fileChunk)
	{
		ASSERT(fileChunk->dataChunk->dataType==CHUNK_FILE);
		return fileChunk->dwSkipOffset + fileChunk->dataChunk->dataFileAddr;
	}
	LPBYTE _GetRealSrcMemStartAddr(TAMAFILECHUNK* memChunk)
	{
		ASSERT(memChunk->dataChunk->dataType==CHUNK_MEM);
		return memChunk->dwSkipOffset + memChunk->dataChunk->dataMemPointer;
	}
	BOOL Save() //上書き保存
	{
		CWaitCursor wait;

		if(IsFileMapping())
		{
//			pFile = m_pFileMapping;

			BOOL bResult = (m_pMapStart) ? ::FlushViewOfFile(m_pMapStart, m_dwMapSize) : ::FlushViewOfFile(m_pData, m_dwTotal);
			if(!bResult) {
				LastErrorMessageBox();
				return FALSE;
			}
		} else {
			if (FAILED(m_file.Create(m_filePath, GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS)))
			{
				LastErrorMessageBox();
				return FALSE;
			}

			if(		FAILED(m_file.Write(m_pData, m_dwTotal))
				/*	|| FAILED(m_file.Flush())*/)
			{
				LastErrorMessageBox();
				m_file.Close();
				return FALSE;
			}
			m_file.Close();
		}
//		m_dwUndoSaved = m_dwUndo;		// ###1.54
//		TouchDoc();

		m_bModified = FALSE;//ここまずい

		return TRUE;
	}
	BOOL SaveAs(LPCTSTR lpszPathName) //名前を付けて保存
	{
		CWaitCursor wait;

		if(IsFileMapping())
		{
//			pFile = m_pFileMapping;

			BOOL bResult = (m_pMapStart) ? ::FlushViewOfFile(m_pMapStart, m_dwMapSize) : ::FlushViewOfFile(m_pData, m_dwTotal);
			if(!bResult) {
				LastErrorMessageBox();
				return FALSE;
			}
		} else {
			CAtlFile newFile;
			if (FAILED(newFile.Create(lpszPathName, GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS)))
			{
				LastErrorMessageBox();
				return FALSE;
			}

			if(FAILED(newFile.Write(m_pData, m_dwTotal))
				/*	|| FAILED(m_file.Flush())*/)
			{
				LastErrorMessageBox();
				newFile.Close();
				return FALSE;
			}
			newFile.Close();
		}
//		m_dwUndoSaved = m_dwUndo;		// ###1.54
//		TouchDoc();

		m_bModified = FALSE;

		return TRUE;
	}
	void Close()
	{
		_DeleteContents();
		m_file.Close();
	}

	DWORD _FileChunk_GetRemain(TAMAFILECHUNK *pFileChunk, DWORD dwStartOffset)
	{
		if(dwStartOffset < pFileChunk->dwStart || pFileChunk->dwEnd < dwStartOffset)return 0;
		return pFileChunk->dwEnd - dwStartOffset + 1;
	}

	DWORD _FileChunk_Read(LPBYTE dst, TAMAFILECHUNK *pSrcFileChunk, DWORD dwStartOffset, DWORD dwMaxRead)
	{
		DWORD dwCanRead = _FileChunk_GetRemain(pSrcFileChunk, dwStartOffset);
		DWORD dwRemain = min(dwCanRead, dwMaxRead);
		DWORD dwShift = pSrcFileChunk->dwStart - dwStartOffset;

		DWORD dwReaded = 0;

		TAMADataChunk *dataChunk = pSrcFileChunk->dataChunk;

		switch(dataChunk->dataType)
		{
		case CHUNK_FILE:
			{
				DWORD dwFileStart = dataChunk->dataFileAddr + dwShift;
				LPBYTE lpStart;
				do
				{
					lpStart = QueryMapViewTama2(dwFileStart, min(dwRemain, options.dwMaxMapSize));
					DWORD dwMapSize = GetMapRemain(dwFileStart);
					dwMapSize = min(dwMapSize, dwRemain);//if(dwMapSize > dwSize)dwMapSize = dwSize;
					if(dwMapSize==0) return dwReaded;
					memcpy(dst, lpStart, dwMapSize);
					dwRemain -= dwMapSize;
					dwReaded += dwMapSize;
				} while(dwRemain > 0);

				return dwReaded;
			}
		case CHUNK_MEM:
			{
				void *pSrcStart = dataChunk->dataMemPointer + pSrcFileChunk->dwSkipOffset + dwShift;
				memcpy(dst, pSrcStart, dwRemain);
				return dwReaded;
			}
		}
		return 0;
	}

// Read/OverWrite
	BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize)
	{
		TAMAFILECHUNK *pReadChunk;
		pReadChunk = _FileMap_LookUp(dwStart);
		if(!pReadChunk)return FALSE;

		LPBYTE lpDst1 = (LPBYTE)dst1;
		DWORD dwRemain = dwSize;

		while(dwRemain > 0 && pReadChunk)
		{
			DWORD dwReaded = _FileChunk_Read((LPBYTE)dst1, pReadChunk, dwStart, dwRemain);
			ASSERT(dwRemain >= dwReaded);
			ASSERT(dwReaded);
			if(dwReaded==0)return FALSE;
			dwRemain -= dwReaded;
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
	BOOL Write(DWORD dwStart, void *src1, DWORD dwSize/*, BOOL bNotCopy = FALSE*/)
	{
		DWORD dwNewTotal = dwStart + dwSize;
		if(dwNewTotal < dwStart)return FALSE;//dwSize too big (overflow)
		BOOL bGlow = FALSE;
		DWORD dwGlow = dwNewTotal - m_dwTotal;
		if(dwNewTotal > m_dwTotal)bGlow = TRUE;
		size_t nPrevSize = bGlow? dwSize-dwGlow : dwSize;
		
		TAMAUndoRedo *pNewUndo = _CreateTAMAUndoRedo(UNDO_OVR, dwStart, nPrevSize, dwSize);
		if(!pNewUndo)return FALSE; //memory full
		memcpy(pNewUndo->dataNext->dataMemPointer, src1, dwSize);
		Read(pNewUndo->dataPrev->dataMemPointer, dwStart, nPrevSize);
		_PreNewUndo();
		m_undo.Add(pNewUndo);
		
		return _FileMap_OverWriteMem(dwStart, dwSize, pNewUndo->dataNext, 0);
	}
	TAMAUndoRedo* _CreateTAMAUndoRedo(UndoMode mode, DWORD dwStart, size_t nPrevSize, size_t nNextSize)
	{
		TAMAUndoRedo *pNewUndo = (TAMAUndoRedo *)malloc(sizeof(TAMAUndoRedo));
		if(!pNewUndo)return NULL;//memory full
		pNewUndo->dwStart = dwStart;
		pNewUndo->dataNext = NULL;
		if(nNextSize)
		{
			pNewUndo->dataNext = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
			if(pNewUndo->dataNext)
			{
				pNewUndo->dataNext->dataType = CHUNK_MEM;
				pNewUndo->dataNext->dwSize = nNextSize;
				pNewUndo->dataNext->nRefCount = 1;
				pNewUndo->dataNext->dataMemPointer = (LPBYTE)malloc(nNextSize);
			}
		}
		pNewUndo->dataPrev = NULL;
		if(nPrevSize)
		{
			pNewUndo->dataPrev = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
			if(pNewUndo->dataPrev)
			{
				pNewUndo->dataPrev->dataType = CHUNK_MEM;
				pNewUndo->dataPrev->dwSize = nPrevSize;
				pNewUndo->dataPrev->nRefCount = 1;
				pNewUndo->dataPrev->dataMemPointer = (LPBYTE)malloc(nPrevSize);
			}
		}
		if((nNextSize && (!pNewUndo->dataNext || !pNewUndo->dataNext->dataMemPointer)) || (nPrevSize && (!pNewUndo->dataPrev || !pNewUndo->dataPrev->dataMemPointer)))
		{//ERR: Memory full
			if(pNewUndo->dataNext)
			{
				if(pNewUndo->dataNext->dataMemPointer)free(pNewUndo->dataNext->dataMemPointer);
				free(pNewUndo->dataNext);
			}
			if(pNewUndo->dataPrev)
			{
				if(pNewUndo->dataPrev->dataMemPointer)free(pNewUndo->dataPrev);
				free(pNewUndo->dataPrev);
			}
			free(pNewUndo);
			return NULL;//ERR:memory full
		}
		pNewUndo->mode = mode;
		pNewUndo->bHidden = FALSE;
		return pNewUndo;
	}
	DWORD inline _GetEndOffset(DWORD dwStart, DWORD dwSize) { ASSERT(dwSize>0); return dwStart+dwSize-1; };

// Insert/Delete
	BOOL Insert(DWORD dwInsStart, LPBYTE srcDataDetached, DWORD dwInsSize)
	{
		TAMAUndoRedo *pNewUndo = _CreateTAMAUndoRedo(UNDO_INS, dwInsStart, 0, 0/*dwInsSize*/);
		ASSERT(pNewUndo);
		if(!pNewUndo)return FALSE; //memory full
		pNewUndo->dataNext = _CreateMemDataChunk(dwInsSize, srcDataDetached);
		ASSERT(pNewUndo->dataNext);
		if(!pNewUndo->dataNext)return FALSE; //memory full
		_PreNewUndo();
		m_undo.Add(pNewUndo);

		ASSERT(_FileMap_InsertMem(dwInsStart, dwInsSize, pNewUndo->dataNext, 0));
		return TRUE;
	}
	BOOL Delete(DWORD dwDelStart, DWORD dwDelSize)
	{
		TAMAUndoRedo *pNewUndo = _CreateTAMAUndoRedo(UNDO_DEL, dwDelStart, dwDelSize, 0);
		ASSERT(pNewUndo);
		if(!pNewUndo)return FALSE; //memory full
		ASSERT(pNewUndo->dataPrev);
		Read(pNewUndo->dataPrev->dataMemPointer, dwDelStart, dwDelSize);
		_PreNewUndo();
		m_undo.Add(pNewUndo);

		ASSERT(_FileMap_Del(dwDelStart, dwDelSize));
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
			_RemoveNeedlessHiddenNode(m_redoIndex, m_savedIndex);//ここで合ってる？
			_HideNodes(m_redoIndex, m_savedIndex);
			//compacthiddennode (too fast) + change m_savedIndex
			for(size_t i = m_savedIndex-1; i>=m_redoIndex; i--)
			{
				TAMAUndoRedo *reverseUndo = _ReverseTAMAUndoRedo(m_undo[i]);
				if(reverseUndo==NULL)
				{
					ASSERT(FALSE);
					return FALSE;
				}
				m_undo.Add(reverseUndo);
			}
			//compacthiddennode (too safe
		}
		return TRUE;
	}
	BOOL _CompactHiddenNode()
	{
		return TRUE;
	}
	inline void _RemoveNeedlessHiddenNode(size_t nStartIndex = 0, size_t nEndIndex = 0)
	{
		if(nEndIndex==0)nEndIndex = m_undo.GetCount()-1;
		ASSERT(nStartIndex<=nEndIndex);

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
	}
	inline void _HideNodes(size_t nStartIndex, size_t nEndIndex)
	{
		ASSERT(nStartIndex<=nEndIndex);
		for(size_t i=nStartIndex; i<=nEndIndex; i++)
			m_undo[i]->bHidden = TRUE;
	}
	TAMAUndoRedo * _ReverseTAMAUndoRedo(TAMAUndoRedo *undo)
	{
		TAMAUndoRedo *newUndo = _CloneTAMAUndoRedo(undo);
		if(!newUndo)return NULL;
		switch(undo->mode)
		{
		case UNDO_INS:
			newUndo->mode = UNDO_DEL;
			break;
		case UNDO_DEL:
			newUndo->mode = UNDO_INS;
			break;
		case UNDO_OVR:
			break;
		}
		TAMADataChunk *swap = undo->dataPrev;
		newUndo->dataPrev = undo->dataNext;
		newUndo->dataNext = swap;
	}
	inline TAMAUndoRedo* _CloneTAMAUndoRedo(TAMAUndoRedo *srcUndo)
	{
		TAMAUndoRedo *newUndo = (TAMAUndoRedo*)malloc(sizeof(TAMAUndoRedo));
		if(newUndo==NULL)return NULL;
		memcpy(newUndo, srcUndo, sizeof(TAMAUndoRedo));
		newUndo->dataPrev->nRefCount++;
		newUndo->dataNext->nRefCount++;
	}
	/*
BOOL _InsertData(DWORD dwStart, DWORD dwSize, BOOL bIns)
{
	BOOL bGlow = false;
	DWORD nGlow = dwSize - (m_dwTotal - dwStart);
	if(nGlow <= dwSize //overflow check
		&& nGlow > 0)
		bGlow=true;
	if(!m_pData) {
		m_pData = (LPBYTE)MemAlloc(dwSize);
		m_dwTotal = dwSize;
	} else if(bIns || dwStart == m_dwTotal) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+dwSize);
			m_dwTotal += dwSize;
			_ShiftFileDataR(dwStart, dwSize);//memmove(m_pData+dwStart+dwSize, m_pData+dwStart, m_dwTotal - dwStart);
	} else if(bGlow) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+nGlow);
			m_dwTotal += nGlow;
	}
	ASSERT(m_pData != NULL);
}

BOOL DeleteData(DWORD dwDelStart, DWORD dwDelSize)
{
	if(dwDelStart == m_dwTotal) return;
	ShiftFileDataL(dwDelStart, dwDelSize);//memmove(m_pData+dwPtr, m_pData+dwPtr+dwSize, m_dwTotal-dwPtr-dwSize);
	m_dwTotal -= dwDelSize;
	if(!IsFileMapping())
		m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal);
	TouchDoc();
}*/

// Undo/Redo
	BOOL Undo()
	{
		if(GetUndoCount()==0)return FALSE;
		TAMAUndoRedo *undo = m_undo[--m_redoIndex];
		switch(undo->mode)
		{
		case UNDO_INS:
			ASSERT(undo->dataNext);
			_FileMap_Del(undo->dwStart, undo->dataNext->dwSize);
			break;
		case UNDO_DEL:
			ASSERT(undo->dataPrev);
			_FileMap_InsertMem(undo->dwStart, undo->dataPrev->dwSize, undo->dataPrev, 0);
			break;
		case UNDO_OVR:
			ASSERT(undo->dataPrev);
			_FileMap_OverWriteMem(undo->dwStart, undo->dataPrev->dwSize, undo->dataPrev, 0);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	BOOL Redo()
	{
		if(GetRedoCount()==0)return FALSE;
		TAMAUndoRedo *undo = m_undo[m_redoIndex++];
		switch(undo->mode)
		{
		case UNDO_INS:
			ASSERT(undo->dataNext);
			_FileMap_InsertMem(undo->dwStart, undo->dataNext->dwSize, undo->dataNext, 0);
			break;
		case UNDO_DEL:
			ASSERT(undo->dataPrev);
			_FileMap_Del(undo->dwStart, undo->dataPrev->dwSize);
			break;
		case UNDO_OVR:
			ASSERT(undo->dataNext);
			_FileMap_OverWriteMem(undo->dwStart, undo->dataNext->dwSize, undo->dataNext, 0);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	size_t GetUndoCount(){ return m_redoIndex; }
	size_t GetRedoCount(){ return m_undo.GetCount() - m_redoIndex; }
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
		size_t delIndex;
		size_t nDelSize = GetRedoCountCanRemove(&delIndex);
		if(nDelSize>0)_UndoRedo_RemoveRange(delIndex, nDelSize);
		return TRUE;
	}
	BOOL ClearUndo()
	{
		size_t nDelSize = GetUndoCountCanRemove();
		if(nDelSize>0)_UndoRedo_RemoveRange(0, nDelSize);
		return TRUE;
	}

// File-mapping Operations
	BOOL	IsFileMapping() { return m_hMapping != NULL; }
	LPBYTE QueryMapViewTama2(DWORD dwStartOffset, DWORD dwIdealMapSize)
	{
		ASSERT(dwIdealMapSize > 0);
		if(m_pMapStart)
		{
			DWORD dwLastOffset = dwStartOffset + dwIdealMapSize-1;
			if(dwLastOffset > m_dwTotal-1) dwLastOffset = m_dwTotal-1;
			if(IsOutOfMapTama(dwStartOffset) || IsOutOfMapTama(dwLastOffset))return _QueryMapViewTama2(dwStartOffset, dwIdealMapSize);
		}
		return _GetFileMappingPointerFromFileOffset(dwStartOffset);
	}
	DWORD GetMapRemain(DWORD dwStart)
	{
		if(m_dwTotal==0)return 0;
		if(m_pMapStart)
		{
			DWORD dwMapSize = m_dwFileOffset + m_dwMapSize;
			if(m_dwFileOffset <= dwStart && dwStart < dwMapSize)return dwMapSize - dwStart;
		} else {
			if(m_dwTotal-1 >= dwStart)return m_dwTotal - dwStart;
		}
		return 0;
	}
	BOOL IsOutOfMapTama(DWORD dwOffset)
	{
		if(m_pMapStart)
		{
			return (dwOffset < m_dwFileOffset || dwOffset >= m_dwFileOffset + m_dwMapSize);
		} else return true;
	}

// 
	DWORD GetDocSize()
	{
		return m_dwTotal;
	}

  protected:

protected:
	CAtlFile m_file;
	CString m_filePath;
//	LPBYTE	m_pData; //メモリ内のアドレス。ファイル内アドレスで0に当たるアドレスを示す。ただし部分的なファイルマッピングの場合がある。その場合m_pDataからマッピング領域が始まっているとは限らない、m_pDataは擬似的なアドレスになっている
	DWORD	m_dwTotal;
	DWORD	m_dwTotalSavedFile;
	HANDLE	m_hMapping;
	LPBYTE	m_pMapStart;	// ###1.61　マッピング領域先端（メモリ内のアドレス）
	DWORD   m_dwFileOffset; //データファイルのマッピング開始アドレス（ファイル内のアドレス）
	DWORD	m_dwMapSize; //データファイルのマッピングサイズ
	DWORD	m_dwAllocationGranularity;
	BOOL	m_bReadOnly;
	DWORD	m_dwBase;		// ###1.63

	BOOL	m_bModified;
	CSuperFileCon* m_pDupDoc;
	CAtlArray<TAMAUndoRedo*> m_undo;
	size_t m_savedIndex;
	size_t m_redoIndex;

	struct _TAMAFILECHUNK_HEAD m_filemapHead;

protected:

	_inline DWORD	_GetFileOffsetFromFileMappingPointer(LPBYTE pConv) { return pConv-m_pData; }
	_inline LPBYTE	_GetFileMappingPointerFromFileOffset(DWORD dwFileOffset) { return dwFileOffset+m_pData; }

	void _DeleteContents() 
	{
		if(m_pData) {
			if(IsFileMapping()) {
				VERIFY(::UnmapViewOfFile(m_pMapStart ? m_pMapStart : m_pData));
				m_pMapStart = NULL;
				m_dwFileOffset = 0;
				m_dwMapSize = 0;
			}
			else
				MemFree(m_pData);
			m_pData = NULL;
			m_dwTotal = 0;
			m_dwBase = 0;
//			UpdateAllViews(NULL);
		}
		if(IsFileMapping()) {
			if(m_pDupDoc)
			{
				m_pDupDoc->m_pDupDoc = NULL;
				m_pDupDoc = NULL;
				m_hMapping = NULL;
				m_file.Detach();//m_pFileMapping = NULL;
			} else {
				VERIFY(::CloseHandle(m_hMapping));
				m_hMapping = NULL;
				m_file.Close();
			}
		}

//		if(m_pUndo) {
//			MemFree(m_pUndo);
//			m_pUndo = NULL;
//		}	
		m_bReadOnly = FALSE;
//		m_arrMarks.RemoveAll();
	}

#define SHIFTBUFSIZE 8*1024*1024
	BOOL _ShiftFileDataR(DWORD dwInsStart, DWORD dwInsSize)
	{
		if(IsFileMapping())
		{
			DWORD dwRemain = m_dwTotal - dwInsSize - dwInsStart;
			if(dwInsSize==0 || dwRemain==0)return TRUE;
			DWORD dwCopySize = min(SHIFTBUFSIZE, dwRemain);
			LPBYTE buf = (LPBYTE)malloc(dwCopySize);
			DWORD dwCurrent = m_dwTotal - dwCopySize;
			BOOL bRet = FALSE;
			while(dwRemain!=0)
			{
				bRet = Read(buf, dwCurrent-dwInsSize, dwCopySize);
				if(!bRet)break;
				bRet = Write(dwCurrent, buf, dwCopySize);
				if(!bRet)break;
				dwRemain -= dwCopySize;
#ifdef DEBUG
				if(dwRemain==0)ASSERT(dwCurrent-dwInsSize==dwInsStart);
#endif
				dwCurrent -= dwCopySize;
				dwCopySize = min(SHIFTBUFSIZE, dwRemain);
			}
			free(buf);
			return bRet;
		} else {
			memmove(m_pData+dwInsStart+dwInsSize, m_pData+dwInsStart, m_dwTotal-dwInsSize-dwInsStart);
			return TRUE;
		}
	}
	BOOL _ShiftFileDataL(DWORD dwDelStart, DWORD dwDelSize)
	{
		if(IsFileMapping())
		{
			DWORD dwRemain = m_dwTotal-dwDelStart-dwDelSize;
			if(dwDelSize==0 || dwRemain==0)return TRUE;
			DWORD dwCopySize = min(SHIFTBUFSIZE, dwRemain);
			LPBYTE buf = (LPBYTE)malloc(dwCopySize);
			DWORD dwCurrent = dwDelStart;
			BOOL bRet = FALSE;
			while(dwRemain!=0)
			{
				bRet = Read(buf, dwCurrent+dwDelSize, dwCopySize);
				if(!bRet)break;
				bRet = Write(dwCurrent, buf, dwCopySize);
				if(!bRet)break;
				dwRemain -= dwCopySize;
				dwCurrent += dwCopySize;
				dwCopySize = min(SHIFTBUFSIZE, dwRemain);
			}
			free(buf);
			return bRet;
		} else {
			memmove(m_pData+dwDelStart, m_pData+dwDelStart+dwDelSize, m_dwTotal-dwDelStart-dwDelSize);
			return TRUE;
		}
	}

	BOOL _MapView()
	{
		m_dwMapSize = m_dwTotal;
		m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
		if(!m_pData) {
			m_dwMapSize = options.dwMaxMapSize;
			m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
			if(m_pData) {
				m_pMapStart = m_pData;
				m_dwFileOffset = 0;
			}
			else {
				LastErrorMessageBox();
				return FALSE;
			}
		}
		return TRUE;
	}

	void _AlignMapSize(DWORD dwStartOffset, DWORD dwIdealMapSize)
	{
		m_dwFileOffset = dwStartOffset - (dwStartOffset % m_dwAllocationGranularity);
		m_dwMapSize = min(max(options.dwMaxMapSize, dwIdealMapSize),  m_dwTotal - m_dwFileOffset);
		if(m_dwMapSize == 0)
		{
			DWORD dwTmp1 = (m_dwTotal - m_dwAllocationGranularity);
			m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
			m_dwMapSize = m_dwTotal - m_dwFileOffset;
		}//バグ：ファイルサイズが極端に小さい場合バグる
	}
	LPBYTE _QueryMapViewTama2 (DWORD dwStartOffset, DWORD dwIdealMapSize)
	{
		if(dwStartOffset == m_dwTotal && dwStartOffset == m_dwFileOffset + m_dwMapSize) return _GetFileMappingPointerFromFileOffset(dwStartOffset);//バグ？そもそもここに来るのはおかしいのではないか？
		VERIFY(::UnmapViewOfFile(m_pMapStart));//ここでマッピング空間への変更が実ファイルへ書き込まれる。後に保存せず閉じる場合はアンドゥで戻す。

		_AlignMapSize(dwStartOffset, dwIdealMapSize);

		m_pMapStart = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, m_dwFileOffset, m_dwMapSize);
		TRACE("MapViewOfFile Doc=%X, %X, Offset:%X, Size:%X\n", this, m_pMapStart, m_dwFileOffset, m_dwMapSize);
		if(!m_pMapStart) {
			ErrorMessageBox();
			AfxPostQuitMessage(0);
			return NULL;
		}
		m_pData = m_pMapStart - m_dwFileOffset;
		return _GetFileMappingPointerFromFileOffset(dwStartOffset);
	}

	DWORD _GetFileLengthLimit4G(BOOL bErrorMsg = FALSE)
	{
		DWORD dwSize = 0;
		ULARGE_INTEGER ulFileSize;
		ulFileSize.QuadPart = 0;
		if(SUCCEEDED(m_file.GetSize(ulFileSize.QuadPart)))
		{
			if(ulFileSize.QuadPart > MAX_FILELENGTH)
			{
				if(bErrorMsg)
				{
					CString strErrOver4G;
					strErrOver4G.LoadString(IDS_ERR_OVER4G);
					MessageBox(NULL, strErrOver4G, _T("Error"), MB_OK);
				}
				dwSize = MAX_FILELENGTH;
			} else dwSize = ulFileSize.LowPart;
		}
		return dwSize;
	}

	inline void _UndoRedo_RemoveRange(size_t delStartIndex, size_t nDelSize)
	{
		for(size_t i=0;i<nDelSize;i++)
		{
			_UndoRedo_ReleaseByIndex(delStartIndex+i);
		}
		m_undo.RemoveAt(delStartIndex, nDelSize);
		if(delStartIndex<m_savedIndex)m_savedIndex -= nDelSize;
		if(delStartIndex<m_redoIndex)m_redoIndex -= nDelSize;
	}
	BOOL inline _UndoRedo_RemoveByIndex(size_t nIndex)	{	if(_UndoRedo_Release(m_undo.GetAt(nIndex))){ m_undo.RemoveAt(nIndex); return TRUE; }
															return FALSE;}
	BOOL inline _UndoRedo_ReleaseByIndex(size_t nIndex)	{ return _UndoRedo_Release(m_undo.GetAt(nIndex)); }
	BOOL inline _UndoRedo_Release(TAMAUndoRedo *undo)
	{
		if(undo==NULL)return FALSE;
		_ReleaseTAMADataChunk(undo->dataNext);
		_ReleaseTAMADataChunk(undo->dataPrev);
		free(undo);
		return TRUE;
	}
	// TRUE  - nRefCount==0
	// FALSE - nRefCount!=0
	BOOL inline _ReleaseTAMADataChunk(TAMADataChunk *chunk)
	{
		if(chunk==NULL)return FALSE;
		if(--chunk->nRefCount == 0)
		{
			if(chunk->dataType==CHUNK_MEM && chunk->dataMemPointer) free(chunk->dataMemPointer);
			free(chunk);
			return TRUE;
		} //else Realloc(chunk->data);
		return FALSE;
	}

	void LastErrorMessageBox()
	{
		MessageBox(NULL, AtlGetErrorDescription(::GetLastError(), LANG_USER_DEFAULT), _T("Error"), MB_OK | MB_ICONERROR);
	}

	inline TAMAFILECHUNK * _FileMap_LookUp(DWORD dwSearchOffset)
	{
		TAMAFILECHUNK findChunk, *pSplitChunk;
		findChunk.key = dwSearchOffset;
		pSplitChunk = RB_NFIND(_TAMAFILECHUNK_HEAD, &m_filemapHead, &findChunk);
		return pSplitChunk;
	}

	//pSplitChunk[dwStart,(dwSplitPoint),dwEnd] >>>
	// pNewFirstChunk[dwStart,dwSplitPoint-1] -(Split)- pSplitChunk[dwSplitPoint,dwEnd]
	BOOL _FileMap_SplitPoint(DWORD dwSplitPoint)
	{
		TAMAFILECHUNK *pSplitChunk;
		pSplitChunk = _FileMap_LookUp(dwSplitPoint);
		if(!pSplitChunk)return FALSE;
		if(pSplitChunk->dwStart != dwSplitPoint)
		{
			TAMAFILECHUNK *pNewFirstChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
			if(pNewFirstChunk==NULL)return FALSE;
			DWORD dwFirstSize = dwSplitPoint - pSplitChunk->dwStart;
			pNewFirstChunk->dwStart = pSplitChunk->dwStart;
			pNewFirstChunk->dwEnd   = dwSplitPoint-1;
			pNewFirstChunk->bSaved = pSplitChunk->bSaved;
			ChunkType type = pSplitChunk->dataChunk->dataType;
			//pNewFirstChunk->dataChunk->dataType = type;
			switch(type)
			{
			case CHUNK_FILE:
				//pNewFirstChunk->dataFile.dwSize = dwFirstSize;
				//pSplitChunk->dataFile.dwSize -= dwFirstSize;
				//pNewFirstChunk->dataFile.dwStart = pSplitChunk->dataFile.dwStart;
				//pSplitChunk->dataFile.dwStart += dwFirstSize;
				//break;
			case CHUNK_MEM:
				{
				TAMADataChunk *undoChunk = pSplitChunk->dataChunk;
				undoChunk->nRefCount++;
				pNewFirstChunk->dataChunk = undoChunk;
				pNewFirstChunk->dwSkipOffset = pSplitChunk->dwSkipOffset;
				pSplitChunk->dwSkipOffset += dwFirstSize;
				}
				break;
			default:
				ASSERT(FALSE);
				break;
			}
			RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pNewFirstChunk);
			pSplitChunk->dwStart = dwSplitPoint;
		}
		return TRUE;
	}

	TAMAFILECHUNK* _FileMap_BasicInsert(DWORD dwEnd, DWORD dwSize)
	{
		TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0 || !pNewChunk)return NULL;
		_FileMap_SplitPoint(dwStart);
		_FileMap_Shift(dwEnd, dwSize);

		pNewChunk->dwStart = dwStart;
		pNewChunk->dwEnd = dwEnd;
		RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pNewChunk);
		return pNewChunk;
	}
	BOOL _FileMap_InsertMem(DWORD dwInsStart, LPBYTE srcDataDetached, DWORD dwInsSize)
	{
		DWORD dwInsEnd = _GetEndOffset(dwInsStart, dwInsSize);
		TAMAFILECHUNK *insChunk = _FileMap_BasicInsert(dwInsEnd, dwInsSize);
		ASSERT(insChunk);
		
		TAMADataChunk *dataChunk = _CreateMemDataChunk(dwInsSize, srcDataDetached);
		ASSERT(dataChunk);
		insChunk->dataChunk = dataChunk;
		insChunk->dwSkipOffset = 0;

		//OptimizeFileMap(insChunk);
	}
	BOOL _FileMap_InsertMem(DWORD dwInsStart, DWORD dwInsSize, TAMADataChunk *srcDataChunk, DWORD dwSkipOffset)
	{
		DWORD dwInsEnd = _GetEndOffset(dwInsStart, dwInsSize);
		TAMAFILECHUNK *insChunk = _FileMap_BasicInsert(dwInsEnd, dwInsSize);
		ASSERT(insChunk);

		ASSERT(srcDataChunk);
		srcDataChunk->nRefCount++;
		insChunk->dataChunk = srcDataChunk;
		insChunk->dwSkipOffset = dwSkipOffset;

		//OptimizeFileMap(insChunk);
	}
	BOOL _FileMap_InsertFile(DWORD dwInsStart, DWORD dwInsSize, DWORD dwStartFileSpace)
	{
		DWORD dwInsEnd = _GetEndOffset(dwInsStart, dwInsSize);
		TAMAFILECHUNK *insChunk = _FileMap_BasicInsert(dwInsEnd, dwInsSize);
		ASSERT(insChunk);

		TAMADataChunk *dataChunk = _CreateFileDataChunk(dwInsSize, dwStartFileSpace);
		ASSERT(dataChunk);
		insChunk->dataChunk = dataChunk;
		insChunk->dwSkipOffset = 0;

		//OptimizeFileMap(insChunk);
	}
	TAMADataChunk* _CreateFileDataChunk(DWORD dwSize, DWORD dwStartFileSpace)
	{
		TAMADataChunk *dataChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
		if(dataChunk==NULL)return NULL;
		dataChunk->nRefCount = 1;
		dataChunk->dataType = CHUNK_FILE;
		dataChunk->dataFileAddr = dwStartFileSpace;
		dataChunk->dwSize = dwSize;
		return dataChunk;
	}
	TAMADataChunk* _CreateMemDataChunk(DWORD dwSize, LPBYTE srcDataDetached = NULL)
	{
		TAMADataChunk *dataChunk = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
		if(dataChunk==NULL)return NULL;
		dataChunk->nRefCount = 1;
		dataChunk->dataType = CHUNK_MEM;
		dataChunk->dataMemPointer = srcDataDetached;
		dataChunk->dwSize = dwSize;
		return dataChunk;
	}

	BOOL __FileMap_Del2(DWORD dwEnd, DWORD dwSize)
	{
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0)return FALSE;
		_FileMap_SplitPoint(dwStart);
		_FileMap_SplitPoint(dwEnd+1);

		__FileMap_DeleteRange(dwEnd, dwSize);
		_FileMap_Shift(dwEnd, dwSize, FALSE/*minus (-dwSize)*/);
		return TRUE;
	}
	BOOL _FileMap_Del(DWORD dwDelStart, DWORD dwDelSize)
	{
		DWORD dwDelEnd = _GetEndOffset(dwDelStart, dwDelSize);
		__FileMap_Del2(dwDelEnd, dwDelSize);
	}

	BOOL _FileMap_Shift(DWORD dwNoShiftEnd, DWORD dwShiftSize, BOOL bPlus = TRUE)
	{
		if(bPlus)
		{
			TAMAFILECHUNK *pChunk;
			RB_FOREACH_REVERSE(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
			{
				if(pChunk->dwStart <= dwNoShiftEnd)break;
				pChunk->dwStart += dwShiftSize;
				pChunk->dwEnd   += dwShiftSize;
			}
		} else {
			TAMAFILECHUNK *pChunk;
			RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
			{
				if(pChunk->dwStart > dwNoShiftEnd)
				{
					pChunk->dwStart -= dwShiftSize;
					pChunk->dwEnd   -= dwShiftSize;
				}
			}
		}
	}

	BOOL __FileMap_DeleteRange(DWORD dwEnd, DWORD dwSize)
	{
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0)return FALSE;
		_FileMap_SplitPoint(dwStart);
		_FileMap_SplitPoint(dwEnd+1);

		TAMAFILECHUNK findChunk;
		findChunk.key = dwEnd;
		TAMAFILECHUNK *pDeleteChunk, *pChunk;
		pDeleteChunk = RB_FIND(_TAMAFILECHUNK_HEAD, &m_filemapHead, &findChunk);
		if(pDeleteChunk==NULL)
		{
			//Merge (Restore split)
			return FALSE;
		}
		pChunk = RB_PREV(_TAMAFILECHUNK_HEAD, &m_filemapHead, pDeleteChunk);
		while(pChunk && pChunk->dwEnd < dwStart)
		{
			pChunk = RB_PREV(_TAMAFILECHUNK_HEAD, &m_filemapHead, pDeleteChunk);
			_FileMap_Remove(pDeleteChunk);
			pDeleteChunk = pChunk;
		}
		if(pDeleteChunk)_FileMap_Remove(pDeleteChunk);
	}

	void _FileMap_Remove(TAMAFILECHUNK *pDeleteChunk)
	{
		switch(pDeleteChunk->dataChunk->dataType)
		{
			case CHUNK_FILE:
				//nothing to do
				break;
			case CHUNK_MEM:
				_ReleaseTAMADataChunk(pDeleteChunk->dataChunk);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		RB_REMOVE(_TAMAFILECHUNK_HEAD, &m_filemapHead, pDeleteChunk);
		free(pDeleteChunk);
	}

	TAMAFILECHUNK * _FileMap_BasicOverWriteMem(DWORD dwEnd, DWORD dwSize)
	{
		TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0 || !pNewChunk)return NULL;
		_FileMap_SplitPoint(dwStart);
		_FileMap_SplitPoint(dwEnd+1);
		__FileMap_DeleteRange(dwEnd, dwSize);

		pNewChunk->dwStart = dwStart;
		pNewChunk->dwEnd = dwEnd;
		pNewChunk->dataChunk = NULL;
		pNewChunk->dwSkipOffset = 0;
		RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pNewChunk);
		return pNewChunk;
	}
	
	BOOL _FileMap_OverWriteMem(DWORD dwStart, DWORD dwSize, TAMADataChunk *dataChunk, DWORD dwSkipOffset)
	{
		DWORD dwEnd = _GetEndOffset(dwStart, dwSize);
		TAMAFILECHUNK *chunk = _FileMap_BasicOverWriteMem(dwEnd, dwSize);
		if(chunk==NULL)return FALSE;
		chunk->dwSkipOffset = dwSkipOffset;
		dataChunk->nRefCount++;
		chunk->dataChunk = dataChunk;

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
			c->dwEnd=m_dwTotalSaved-1;
			RB_INSERT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, c);
		}
		
		//searching...
		TAMAFILECHUNK *pChunk;
		RB_FOREACH(pChunk, _TAMAFILECHUNK_HEAD, &m_filemapHead)
		{
			TAMADATACHUNK *dataChunk = pChunk->dataChunk;
			ASSERT(dataChunk);
			switch(dataChunk->dataType)
			{
			case CHUNK_FILE:
			{
				ASSERT(dataChunk->dwSize>0);
				if(dataChunk->dwSize > 0)
				{
					DWORD dwOldFileAddrS = dataChunk->dataFileAddr + dataChunk->dwSkipOffset;
					DWORD dwOldFileAddrE = dwOldFileAddrE + dataChunk->dwSize - 1;
					_OldFileMap_FF(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE, pChunk->dwStart);
					//dataChunk->savingType = DC_FF;
				}
				break;
			}
			case CHUNK_MEM:
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}
		size_t nUndo = m_undo.GetCount();
		for(size_t i=0; i < nUndo; i++)
		{
			TAMAUndoRedo *undo = m_undo[i];
			ASSERT(undo);
			TAMADATACHUNK **pDataChunks = undo->dataNext;
			DWORD nDataChunks = undo->nDataNext;
			for(size_t j=0; j<nDataChunk; j++)
			{
				TAMADATACHUNK *dataChunk = pDataChunks[j];
				if(dataChunk && dataChunk->dataType==CHUNK_FILE)
				{
					ASSERT(dataChunk->dwSize>0);
					if(dataChunk->dwSize > 0)
					{
						DWORD dwOldFileAddrS = dataChunk->dataFileAddr + dataChunk->dwSkipOffset;
						DWORD dwOldFileAddrE = dwOldFileAddrE + dataChunk->dwSize - 1;
						_OldFileMap_FD(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE);
						//dataChunk->savingType = DC_FD;
					}
				}
			}
			pDataChunks = undo->dataPrev;
			nDataChunks = undo->nDataPrev;
			for(size_t j=0; j<nDataChunk; j++)
			{
				TAMADATACHUNK *dataChunk = pDataChunks[j];
				if(dataChunk && dataChunk->dataType==CHUNK_FILE)
				{
					ASSERT(dataChunk->dwSize>0);
					if(dataChunk->dwSize > 0)
					{
						DWORD dwOldFileAddrS = dataChunk->dataFileAddr + dataChunk->dwSkipOffset;
						DWORD dwOldFileAddrE = dwOldFileAddrE + dataChunk->dwSize - 1;
						_OldFileMap_FD(pOldFilemapHead, dwOldFileAddrS, dwOldFileAddrE);
						//dataChunk->savingType = DC_FD;
					}
				}
			}
		}
		
		
		TAMAOLDFILECHUNK *pOldFileChunk;
		RB_FOREACH(pOldFileChunk, _TAMAOLDFILECHUNK_HEAD, pOldFilemapHead)
		{
			if(pOldFileChunk->type==OF_FD)
			{
				ASSERT(pOldFileChunk->dwEnd > pOldFileChunk->dwStart);
				DWORD dwReadSize = pOldFileChunk->dwEnd - pOldFileChunk->dwStart;
				pOldFileChunk->pMem = (LPBYTE)malloc(dwReadSize);
				if(pOldFileChunk->pMem==NULL || FAILED(pFile->Seek(pOldFileChunk->dwStart, FILE_BEGIN)) || FAILED(pFile->Read(pOldFileChunk->pMem, dwReadSize)))
				{
					_OldFileMap_FreeAll(pOldFilemapHead, TRUE);
					return FALSE;
				}
			}
		}
		return TRUE;
	}
	
	void _OldFileMap_FreeAll(struct _TAMAOLDFILECHUNK_HEAD *pOldFilemapHead, BOOL bFreeBuffer = TRUE)
	{
		TAMAOLDFILECHUNK *pChunk, *pChunkPrev = NULL;
		RB_FOREACH(pChunk, _TAMAOLDFILECHUNK_HEAD, pOldFilemapHead)
		{
			if(pChunkPrev)_OldFileMap_Free(pChunkPrev);
			pChunkPrev = pChunk;
		}
		if(pChunkPrev)_OldFileMap_Free(pChunkPrev);
	}
	
	void _OldFileMap_Free(TAMAOLDFILECHUNK *pChunk, BOOL bFreeBuffer = TRUE)
	{
		if(bFreeBuffer && pChunk->type==OF_FD && pChunk->pMem)free(pChunk->pMem);
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
		if(!pSplitChunk)return FALSE;
		if(pSplitChunk->dwStart != dwSplitPoint)
		{
			TAMAOLDFILECHUNK *pNewFirstChunk = (TAMAOLDFILECHUNK *)malloc(sizeof(TAMAOLDFILECHUNK));
			if(pNewFirstChunk==NULL)return FALSE;
			DWORD dwFirstSize = dwSplitPoint - pSplitChunk->dwStart;
			pNewFirstChunk->dwStart = pSplitChunk->dwStart;
			pNewFirstChunk->dwEnd   = dwSplitPoint-1;
			pNewFirstChunk->type = pSplitChunk->type;
			pNewFirstChunk->dwNewFileAddr = pSplitChunk->dwNewFileAddr;
			RB_INSERT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pNewFirstChunk);
			pSplitChunk->dwStart = dwSplitPoint;
			if(pSplitChunk->type==OF_FF)pSplitChunk->dwNewFileAddr += dwFirstSize;
			ASSERT(pSplitChunk->dwNewFileAddr >= pNewFirstChunk->dwNewFileAddr);
		}
		return TRUE;
	}
	
	void _OldFileMap_MeltFF(TAMAOLDFILECHUNK *pChunk)
	{
		ASSERT(pChunk->type==OF_FF);
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
		ASSERT(pOldFileChunkS);
		if(pOldFileChunkS->type == OF_FF && pOldFileChunkS->dwEnd >= dwEnd && dwNewFileAddr - pOldFileChunkS->dwNewFileAddr == dwStart - pOldFileChunkS->dwStart)return;
		if(pOldFileChunkS->dwStart < dwStart && (pOldFileChunkS->type != OF_FF || dwNewFileAddr - pOldFileChunkS->dwNewFileAddr != dwStart - pOldFileChunkS->dwStart))
		{
			_OldFileMap_SplitPoint(dwStart);
			pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
		}
		
		TAMAOLDFILECHUNK *pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
		ASSERT(pOldFileChunkE);
		if(pOldFileChunkE->dwEnd > dwEnd && (pOldFileChunkE->type != OF_FF || dwNewFileAddr - pOldFileChunkE->dwNewFileAddr != dwStart - pOldFileChunkE->dwStart))
		{
			_OldFileMap_SplitPoint(dwEnd+1);
			pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
		}
		
		TAMAOLDFILECHUNK *pChunk = pOldFileChunkS;
		while(1)
		{
			if(pChunk->type != OF_FF || (pChunk->dwStart >= dwStart && pChunk->dwEnd <= dwEnd))
			{
				pChunk->type = OF_FF;
				pChunk->dwNewFileAddr = dwNewFileAddr - (dwStart - pChunk->dwStart);
				ASSERT(pChunk->dwNewFileAddr < dwNewFileAddr);
			}
			OldFileMap_MeltFF(pChunk);
			if(pChunk->dwEnd >= dwEnd)break;
			pChunk = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
			ASSERT(pChunk);
		}
	}
	
	void _OldFileMap_MeltFD(TAMAOLDFILECHUNK *pChunk)
	{
		ASSERT(pChunk->type==OF_FD);
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
		ASSERT(pOldFileChunkS);
		if(pOldFileChunkS->type == OF_FD && pOldFileChunkS->dwEnd >= dwEnd)return;
		if(pOldFileChunkS->type == OF_NOREF && pOldFileChunkS->dwStart < dwStart)
		{
			_OldFileMap_SplitPoint(dwStart);
			pOldFileChunkS = _OldFileMap_LookUp(pOldFilemapHead, dwStart);
		}
		
		TAMAOLDFILECHUNK *pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
		ASSERT(pOldFileChunkE);
		if(pOldFileChunkE->type == OF_NOREF && pOldFileChunkE->dwEnd > dwEnd)
		{
			_OldFileMap_SplitPoint(dwEnd+1);
			pOldFileChunkE = _OldFileMap_LookUp(pOldFilemapHead, dwEnd);
		}
		
		TAMAOLDFILECHUNK *pChunk = pOldFileChunkS;
		while(1)
		{
			if(pChunk->type == OF_NOREF)
			{
				pChunk->type = OF_FD;
				pChunk->pMem = NULL;
				_OldFileMap_MeltFD(pChunk);
			}
			if(pChunk->dwEnd >= dwEnd)break;
			pChunk = RB_NEXT(_TAMAOLDFILECHUNK_HEAD, pOldFilemapHead, pChunk);
			ASSERT(pChunk);
		}
	}


};
