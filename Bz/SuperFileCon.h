#pragma once

#include "tree.h"

#define MAX_FILELENGTH  0xFFFFFFF0


typedef enum {	UNDO_INS, UNDO_OVR, UNDO_DEL } UndoMode;
typedef struct
{
	LPBYTE data;
	DWORD dwSize;
	DWORD nRefCount;
} TAMADataChunk;
typedef struct
{
	UndoMode mode;
	DWORD dwStart;
	TAMADataChunk *dataNext;
	TAMADataChunk *dataPrev;
	BOOL bHidden;
} TAMAUndoRedo;

typedef enum {	CHUNK_FILE, /*CHUNK_UNDO,*/ CHUNK_MEM } ChunkType;
typedef struct _TAMAFILECHUNK
{
	RB_ENTRY(_TAMAFILECHUNK) linkage;
	union 
	{
		DWORD dwEnd; //Sort-Key
		DWORD key;
	};
	DWORD dwStart;
	ChunkType dataType;
	union
	{
		struct _dataFile
		{
			DWORD dwStart;
			DWORD dwSize;
		} dataFile;
		struct _dataMem
		{
			TAMADataChunk* pTAMADataChunk;
			DWORD dwOffset;
		} dataMem;
	};
} TAMAFILECHUNK;
static int cmpTAMAFILECHUNK(TAMAFILECHUNK *c1, TAMAFILECHUNK *c2)
{
	return c2->dwEnd - c1->dwEnd;
}
RB_HEAD(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK);
RB_PROTOTYPE(_TAMAFILECHUNK_HEAD, _TAMAFILECHUNK, linkage, cmpTAMAFILECHUNK);



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
			m_dwTotal = _GetFileLengthLimit4G();
			if(IsFileMapping())
			{
				if(!_MapView())
				{
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
			} else {
				if(!(m_pData = (LPBYTE)MemAlloc(m_dwTotal))) {
					AfxMessageBox(IDS_ERR_MALLOC);
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
				DWORD len = m_file.Read(m_pData, m_dwTotal);
				if(len < m_dwTotal) {
					AfxMessageBox(IDS_ERR_READFILE);
					MemFree(m_pData);	// ###1.61
					m_pData = NULL;
					m_file.Close();
					_DeleteContents();
					return FALSE;
				}
				m_bReadOnly = options.bReadOnlyOpen;
				m_file.Close();
			}
		}
		m_bModified = FALSE;
		m_filePath = lpszPathName;

		return TRUE;
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

		m_bModified = FALSE;

		return TRUE;
	}
	BOOL SaveAs(LPCTSTR lpszPathName) //名前を付けて保存
	{
	}
	void Close()
	{
		_DeleteContents();
		m_file.Close();
	}

// Read/OverWrite
	BOOL Read(void *dst1, DWORD dwStart, DWORD dwSize)
	{
		LPBYTE lpStart;
		do
		{
			lpStart = QueryMapViewTama2(dwStart, min(dwSize, options.dwMaxMapSize));
			DWORD dwMapSize = GetMapRemain(dwStart);
			if(dwMapSize > dwSize)dwMapSize = dwSize;
			if(dwMapSize==0)
			{
				return FALSE;
			}
			memcpy(dst1, lpStart, dwMapSize);
			dwSize -= dwMapSize;
		} while(dwSize > 0);

		return TRUE;
	}
	BOOL ReadTwin(void *dst1, void *dst2, DWORD dwStart, DWORD dwSize)
	{
		LPBYTE lpStart;
		do
		{
			lpStart = QueryMapViewTama2(dwStart, min(dwSize, options.dwMaxMapSize));
			DWORD dwMapSize = GetMapRemain(dwStart);
			if(dwMapSize > dwSize)dwMapSize = dwSize;
			if(dwMapSize==0)
			{
				return FALSE;
			}
			memcpy(dst1, lpStart, dwMapSize);
			memcpy(dst2, lpStart, dwMapSize);
			dwSize -= dwMapSize;
		} while(dwSize > 0);

		return TRUE;
	}
	BOOL Write(DWORD dwStart, void *src1, DWORD dwSize/*, BOOL bNotCopy = FALSE*/)
	{
		DWORD dwNewTotal = dwStart + dwSize;
		if(dwNewTotal < dwStart)return FALSE;//dwSize too big (overflow)
		BOOL bGlow = FALSE;
		DWORD dwGlow = dwNewTotal - m_dwTotal;
		if(dwNewTotal > m_dwTotal)bGlow = TRUE;
		size_t nPrevSize = bGlow? dwSize-dwGlow : dwSize;
		
		TAMAUndoRedo *pNewUndo = _CreateTAMAUndoRedo(UNDO_OVR, dwStart, dwSize, nPrevSize);
		if(!pNewUndo)return FALSE; //memory full
		memcpy(pNewUndo->dataNext->data, src1, dwSize);
		Read(pNewUndo->dataPrev->data, dwStart, nPrevSize);
		_PreNewUndo();
		m_undo.Add(pNewUndo);
		
//		UpdateMap
		/*
		LPBYTE lpStart;
		do
		{
			lpStart = QueryMapViewTama2(dwStart, min(dwSize, options.dwMaxMapSize));
			DWORD dwMapSize = GetMapRemain(dwStart);
			if(dwMapSize > dwSize)dwMapSize = dwSize;
			if(dwMapSize==0)
			{
				return FALSE;
			}
			memcpy(lpStart, src1, dwMapSize);
			dwSize -= dwMapSize;
		} while(dwSize > 0);

		return TRUE;*/
	}
	TAMAUndoRedo* _CreateTAMAUndoRedo(UndoMode mode, DWORD dwStart, size_t nNextSize, size_t nPrevSize)
	{
		TAMAUndoRedo *pNewUndo = (TAMAUndoRedo *)malloc(sizeof(TAMAUndoRedo));
		if(!pNewUndo)return NULL;//memory full
		pNewUndo->dwStart = dwStart;
		pNewUndo->dataNext = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
		pNewUndo->dataNext->data = (LPBYTE)malloc(nNextSize);
		pNewUndo->dataNext->dwSize = nNextSize;
		pNewUndo->dataPrev = (TAMADataChunk *)malloc(sizeof(TAMADataChunk));
		pNewUndo->dataPrev->dwSize = nPrevSize;
		pNewUndo->dataPrev->data = (LPBYTE)malloc(nPrevSize);
		if(pNewUndo->dataNext==NULL || pNewUndo->dataPrev==NULL)
		{
			if(pNewUndo->dataNext==NULL)free(pNewUndo->dataNext);
			if(pNewUndo->dataPrev==NULL)free(pNewUndo->dataPrev);
			free(pNewUndo);
			return NULL;//memory full
		}
		pNewUndo->mode = mode;
		pNewUndo->bHidden = FALSE;
	}
// Insert/Delete
	BOOL Insert()
	{
	}
	BOOL Delete()
	{
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
						_RemoveUndoRedoAt(nHiddenStarted, delSize);
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
		newUndo->dataPrev = undo->dataNext;
		newUndo->dataNext = undo->dataPrev;
	}
	inline TAMAUndoRedo* _CloneTAMAUndoRedo(TAMAUndoRedo *srcUndo)
	{
		TAMAUndoRedo *newUndo = (TAMAUndoRedo*)malloc(sizeof(TAMAUndoRedo));
		if(newUndo==NULL)return NULL;
		memcpy(newUndo, srcUndo, sizeof(TAMAUndoRedo));
		newUndo->dataPrev->nRefCount++;
		newUndo->dataNext->nRefCount++;
	}
	
BOOL InsertData(DWORD dwStart, DWORD dwSize, BOOL bIns)
{
	BOOL bGlow = false;
	DWORD nGlow = dwSize - (m_dwTotal - dwStart);
	if(nGlow <= dwSize/*overflow check*/ && nGlow > 0)
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
/*
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
	}
	BOOL Redo()
	{
		if(GetRedoCount()==0)return FALSE;
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
		if(nDelSize>0)_RemoveUndoRedoAt(delIndex, nDelSize);
		return TRUE;
	}
	BOOL ClearUndo()
	{
		size_t nDelSize = GetUndoCountCanRemove();
		if(nDelSize>0)_RemoveUndoRedoAt(0, nDelSize);
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
	LPBYTE	m_pData; //メモリ内のアドレス。ファイル内アドレスで0に当たるアドレスを示す。ただし部分的なファイルマッピングの場合がある。その場合m_pDataからマッピング領域が始まっているとは限らない、m_pDataは擬似的なアドレスになっている
	DWORD	m_dwTotal;
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

	BOOL _StoreUndo()
	{
	}

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

	inline void _RemoveUndoRedoAt(size_t delStartIndex, size_t nDelSize)
	{
		for(size_t i=0;i<nDelSize;i++)
		{
			_ReleaseTAMAUndoRedoByIndex(delStartIndex+i);
		}
		m_undo.RemoveAt(delStartIndex, nDelSize);
		if(delStartIndex<m_savedIndex)m_savedIndex -= nDelSize;
		if(delStartIndex<m_redoIndex)m_redoIndex -= nDelSize;
	}
	BOOL inline _ReleaseTAMAUndoRedoByIndex(size_t nIndex)
	{
		_ReleaseTAMAUndoRedo(m_undo.GetAt(nIndex));
	}
	BOOL inline _ReleaseTAMAUndoRedo(TAMAUndoRedo *undo)
	{
		ASSERT(undo!=NULL);
		if(_ReleaseTAMADataChunk(undo->dataNext))undo->dataNext = NULL;
		if(_ReleaseTAMADataChunk(undo->dataPrev))undo->dataPrev = NULL;
		free(undo);
	}
	// TRUE  - nRefCount==0
	// FALSE - nRefCount!=0
	BOOL inline _ReleaseTAMADataChunk(TAMADataChunk *chunk)
	{
		if(--chunk->nRefCount == 0)
		{
			free(chunk->data);
			free(chunk);
			return TRUE;
		} //else Realloc(chunk->data);
		return FALSE;
	}

	void LastErrorMessageBox()
	{
		MessageBox(NULL, AtlGetErrorDescription(::GetLastError(), LANG_USER_DEFAULT), _T("Error"), MB_OK | MB_ICONERROR);
	}

	//pSplitChunk[dwStart,(dwSplitPoint),dwEnd] >>>
	// pNewFirstChunk[dwStart,dwSplitPoint-1] -(Split)- pSplitChunk[dwSplitPoint,dwEnd]
	BOOL FileMap_SplitPoint(DWORD dwSplitPoint)
	{
		TAMAFILECHUNK findChunk, *pSplitChunk;
		findChunk.key = dwSplitPoint;
		pSplitChunk = RB_NFIND(_TAMAFILECHUNK_HEAD, &m_filemapHead, &findChunk);
		if(!pSplitChunk)return FALSE;
		if(pSplitChunk->dwStart != dwSplitPoint)
		{
			TAMAFILECHUNK *pNewFirstChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
			if(pNewFirstChunk==NULL)return FALSE;
			DWORD dwFirstSize = dwSplitPoint - pSplitChunk->dwStart;
			pNewFirstChunk->dwStart = pSplitChunk->dwStart;
			pNewFirstChunk->dwEnd   = dwSplitPoint-1;
			ChunkType type = pSplitChunk->dataType;
			pNewFirstChunk->dataType = type;
			switch(type)
			{
			case CHUNK_FILE:
				pNewFirstChunk->dataFile.dwSize = dwFirstSize;
				pSplitChunk->dataFile.dwSize -= dwFirstSize;
				pNewFirstChunk->dataFile.dwStart = pSplitChunk->dataFile.dwStart;
				pSplitChunk->dataFile.dwStart += dwFirstSize;
				break;
//			case CHUNK_UNDO:
			case CHUNK_MEM:
				{
				TAMADataChunk *undoChunk = pSplitChunk->dataMem.pTAMADataChunk;
				pNewFirstChunk->dataMem.pTAMADataChunk = undoChunk;
				undoChunk->nRefCount++;
				pNewFirstChunk->dataMem.dwOffset = pSplitChunk->dataMem.dwOffset;
				pSplitChunk->dataMem.dwOffset += dwFirstSize;
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

//	TAMAFILECHUNK* CloneTAMAFILECHUNK(TAMAFILECHUNK *srcChunk)
//	{
//		TAMAFILECHUNK *pCloneChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
//		if(pCloneChunk)memcpy(pCloneChunk, srcChunk, sizeof(TAMAFILECHUNK));
//		return pCloneChunk;
//	}

	TAMAFILECHUNK* FileMap_Insert(DWORD dwEnd, DWORD dwSize)
	{
		TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0 || !pNewChunk)return NULL;
		FileMap_SplitPoint(dwStart);
		FileMap_Shift(dwEnd, dwSize);

		pNewChunk->dwStart = dwStart;
		pNewChunk->dwEnd = dwEnd;
		RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pNewChunk);
		return pNewChunk;
	}

	BOOL FileMap_Del(DWORD dwEnd, DWORD dwSize)
	{
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0)return FALSE;
		FileMap_SplitPoint(dwStart);
		FileMap_SplitPoint(dwEnd+1);

		FileMap_DeleteRange(dwEnd, dwSize);
		FileMap_Shift(dwEnd, dwSize, FALSE/*minus (-dwSize)*/);
		return TRUE;
	}

	BOOL FileMap_Shift(DWORD dwNoShiftEnd, DWORD dwShiftSize, BOOL bPlus = TRUE)
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

	BOOL FileMap_DeleteRange(DWORD dwEnd, DWORD dwSize)
	{
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0)return FALSE;
		FileMap_SplitPoint(dwStart);
		FileMap_SplitPoint(dwEnd+1);

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
			FileMap_FreeChunk(pDeleteChunk);
			pDeleteChunk = pChunk;
		}
		if(pDeleteChunk)FileMap_FreeChunk(pDeleteChunk);
	}

	void FileMap_FreeChunk(TAMAFILECHUNK *pDeleteChunk)
	{
		RB_REMOVE(_TAMAFILECHUNK_HEAD, &m_filemapHead, pDeleteChunk);
		free(pDeleteChunk);
	}

	BOOL FileMap_OverWrite(DWORD dwEnd, DWORD dwSize)
	{
		TAMAFILECHUNK *pNewChunk = (TAMAFILECHUNK *)malloc(sizeof(TAMAFILECHUNK));
		DWORD dwStart = dwEnd - dwSize +1;
		if(dwSize==0 || !pNewChunk)return FALSE;
		FileMap_SplitPoint(dwStart);
		FileMap_SplitPoint(dwEnd+1);
		FileMap_DeleteRange(dwEnd, dwSize);

		pNewChunk->dwStart = dwStart;
		pNewChunk->dwEnd = dwEnd;
		RB_INSERT(_TAMAFILECHUNK_HEAD, &m_filemapHead, pNewChunk);
	}

};
