#include <crtdbg.h>

#include <gtest/gtest.h>
#include <windows.h>
#include <atlstr.h>
#include <atltrace.h>//ATLTRACE
#include <atlcoll.h>
#include <atlutil.h>
#define GTEST

//void printBackTract()
//{
//  CStringA str;
//  CReportHookDumpHandler stackDumper;
//  stackDumper.GetStackDump(&str);
//  str.Replace("~", "\n");
//  ATLTRACE(str);
//}

DWORD g_dwMallocCounter = 0;
CRBMap<void *, size_t> *g_pRBMap = NULL;
void initMallocSimpleCheck()
{
  g_dwMallocCounter = 0;
  if(g_pRBMap)
  {
    delete g_pRBMap;
    g_pRBMap = NULL;
  }
  g_pRBMap = new CRBMap<void *, size_t>();
}
void* mallocSimpleCheck(size_t size)
{
  g_dwMallocCounter++;
  void *ret = malloc(size);
  //printBackTract();
  if(ret && g_pRBMap)
  {
    ATLTRACE("+0x%08X (%Iu)\n", ret, size);
    g_pRBMap->SetAt(ret, size);
  }
  return ret;
}
void* reallocSimpleCheck(void *pMem, size_t size)
{
  void *ret = realloc(pMem, size);
  //printBackTract();
  if(ret && g_pRBMap)
  {
    g_pRBMap->RemoveKey(pMem);
    g_pRBMap->SetAt(ret, size);
    ATLTRACE("r0x%08X (%Iu)\n", ret, size);
  }
  return ret;
}
void freeSimpleCheck(void *pMem)
{
  if(pMem && g_pRBMap)
  {
    CRBMap<void *, size_t>::CPair *pair = g_pRBMap->Lookup(pMem);
    if(pair)
    {
      ATLTRACE("-0x%08X (%Iu)\n", pMem, pair->m_value);
      g_pRBMap->RemoveKey(pMem);
    } else {
      ATLTRACE("!!!!!!!!!!!!!0x%08X!!!!!!!!!!2nd free!!!!!!!!!\n", pMem);
      //ATLASSERT(pair);
    }
  }
  g_dwMallocCounter--;
  free(pMem);
}
void traceMallocLeak()
{
  POSITION pos = g_pRBMap->GetHeadPosition();
  while(pos)
  {
    CRBMap<void *, size_t>::CPair *pair = g_pRBMap->GetNext(pos);
    ATLTRACE("!0x%08X (%u)\n", pair->m_key, pair->m_value);
  }
}

#define malloc mallocSimpleCheck
#define realloc reallocSimpleCheck
#define free freeSimpleCheck
#define private public
#define protected public

#include "../../SuperFileCon.h"

#undef private
#undef protected
#undef malloc
#undef realloc
#undef free

#include "mt19937ar.h"
#include <time.h>

TEST(FileMap, DISABLED_InsertFile1)
{//_FileMap_InsertFile, _FileMap_LookUp, _FileMap_Shift, _FileMap_SplitPoint, _TAMAFILECHUNK_Copy
  CSuperFileCon sfile;
  ASSERT_EQ(0, sfile._FileMap_DEBUG_GetCount());

  {
    ASSERT_TRUE(sfile._FileMap_InsertFile(0, 0x8468, 0)==TRUE);
    TAMAFILECHUNK *pFileChunk1 = sfile._FileMap_LookUp(0);
    TAMAFILECHUNK *pFileChunk2 = sfile._FileMap_LookUp(0x8467);
    TAMAFILECHUNK *pFileChunk3 = sfile._FileMap_LookUp(0x8468);
    ASSERT_TRUE(pFileChunk1!=NULL);
    ASSERT_TRUE(pFileChunk2!=NULL);
    ASSERT_TRUE(pFileChunk1==pFileChunk2);
    ASSERT_TRUE(pFileChunk3==NULL);
    ASSERT_EQ(0, pFileChunk1->dwStart);
    ASSERT_EQ(0x8467, pFileChunk1->dwEnd);
    ASSERT_EQ(1, sfile._FileMap_DEBUG_GetCount());
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  }
  {
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x222, 0x8, 0x9999)==TRUE);
    TAMAFILECHUNK *pFileChunk11 = sfile._FileMap_LookUp(0);
    TAMAFILECHUNK *pFileChunk12 = sfile._FileMap_LookUp(0x221);
    TAMAFILECHUNK *pFileChunk21 = sfile._FileMap_LookUp(0x222);
    TAMAFILECHUNK *pFileChunk22 = sfile._FileMap_LookUp(0x229);
    TAMAFILECHUNK *pFileChunk31 = sfile._FileMap_LookUp(0x22a);
    TAMAFILECHUNK *pFileChunk32 = sfile._FileMap_LookUp(0x846f);
    TAMAFILECHUNK *pFileChunk4 = sfile._FileMap_LookUp(0x8470);
    ASSERT_TRUE(pFileChunk11!=NULL);
    ASSERT_TRUE(pFileChunk12!=NULL);
    ASSERT_TRUE(pFileChunk21!=NULL);
    ASSERT_TRUE(pFileChunk22!=NULL);
    ASSERT_TRUE(pFileChunk31!=NULL);
    ASSERT_TRUE(pFileChunk32!=NULL);
    ASSERT_TRUE(pFileChunk4==NULL);
    ASSERT_TRUE(pFileChunk11==pFileChunk12);
    ASSERT_TRUE(pFileChunk21==pFileChunk22);
    ASSERT_TRUE(pFileChunk31==pFileChunk32);
    ASSERT_TRUE(pFileChunk11!=pFileChunk21);
    ASSERT_TRUE(pFileChunk11!=pFileChunk31);
    ASSERT_TRUE(pFileChunk21!=pFileChunk31);
    ASSERT_EQ(0, pFileChunk11->dwStart);
    ASSERT_EQ(0x221, pFileChunk11->dwEnd);
    ASSERT_EQ(0x222, pFileChunk21->dwStart);
    ASSERT_EQ(0x229, pFileChunk21->dwEnd);
    ASSERT_EQ(0x22a, pFileChunk31->dwStart);
    ASSERT_EQ(0x846f, pFileChunk31->dwEnd);
    ASSERT_EQ(3, sfile._FileMap_DEBUG_GetCount());
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  }

  {
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x7FffFFff, 0x20000000, 0x5555)==TRUE);
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x9FffFFff, 0x20000000, 0x6666)==TRUE);
    TAMAFILECHUNK *pFileChunk11 = sfile._FileMap_LookUp(0x80000228);
    TAMAFILECHUNK *pFileChunk12 = sfile._FileMap_LookUp(0x80000229);
    TAMAFILECHUNK *pFileChunk1 = sfile._FileMap_LookUp(0x7FffFFff);
    TAMAFILECHUNK *pFileChunk2 = sfile._FileMap_LookUp(0x8FffFFff);
    TAMAFILECHUNK *pFileChunk3 = sfile._FileMap_LookUp(0x9FffFFfe);
    TAMAFILECHUNK *pFileChunk4 = sfile._FileMap_LookUp(0x9FffFFff);
    TAMAFILECHUNK *pFileChunk5 = sfile._FileMap_LookUp(0xbFffFFfe);
    TAMAFILECHUNK *pFileChunk6 = sfile._FileMap_LookUp(0xbFffFFff);
    ASSERT_TRUE(pFileChunk11!=NULL);
    ASSERT_TRUE(pFileChunk12!=NULL);
    ASSERT_TRUE(pFileChunk1!=NULL);
    ASSERT_TRUE(pFileChunk2!=NULL);
    ASSERT_TRUE(pFileChunk3!=NULL);
    ASSERT_TRUE(pFileChunk4!=NULL);
    ASSERT_TRUE(pFileChunk5!=NULL);
    ASSERT_TRUE(pFileChunk6==NULL);
    ASSERT_TRUE(pFileChunk11==pFileChunk12);
    ASSERT_TRUE(pFileChunk11==pFileChunk1);
    ASSERT_TRUE(pFileChunk11==pFileChunk2);
    ASSERT_TRUE(pFileChunk11==pFileChunk3);
    ASSERT_TRUE(pFileChunk4==pFileChunk5);
    ASSERT_TRUE(pFileChunk1!=pFileChunk4);
    ASSERT_EQ(0x9FffFFfe, pFileChunk1->dwEnd);
    ASSERT_EQ(0x7FffFFff, pFileChunk1->dwStart);
    ASSERT_EQ(0xbFffFFfe, pFileChunk4->dwEnd);
    ASSERT_EQ(0x9FffFFff, pFileChunk4->dwStart);
  }
}

void fillRandMT32(unsigned long *pMem, DWORD ulCount, unsigned long seed = 0)
{
  if(seed)init_genrand(seed);
  for(DWORD i=0; i<ulCount; i++)pMem[i] = genrand_int32();
}

void fillRandMT(LPBYTE pMem, DWORD dwByte, unsigned long seed = 0)
{
  fillRandMT32((unsigned long*)pMem, dwByte/4, seed);
  if(dwByte % 4 != 0)
  {
    LPBYTE pCurrent = pMem + dwByte - (dwByte % 4);
    unsigned long ul = genrand_int32();
    LPBYTE pUl = (LPBYTE)&ul;
    for(int i=0; i<dwByte%4; i++, pCurrent++)
      *pCurrent = pUl[i];
  }
}

bool CreateTestFile(LPCSTR filename, DWORD dwSize, BYTE **ppBuf = NULL)
{
  FILE *fp = fopen(filename, "wb");
  if(!fp)return false;
  BYTE *pBuf = (LPBYTE)malloc(dwSize);
  if(!pBuf)
  {
    fclose(fp);
    return false;
  }
  fillRandMT(pBuf, dwSize, time(NULL));
  if(fwrite(pBuf, dwSize, 1, fp)!=1)
  {
    fclose(fp);
    return false;
  }
  fflush(fp);
  fclose(fp);
  if(ppBuf)*ppBuf = pBuf;
  else free(pBuf);
  return true;
}

void memInsert(LPBYTE pMemDst, DWORD *dwMemDst, DWORD dwInsPos, LPBYTE pMemSrc, DWORD dwInsSize)
{
  memmove(pMemDst+dwInsPos+dwInsSize, pMemDst+dwInsPos, *dwMemDst-dwInsPos);
  memcpy(pMemDst+dwInsPos, pMemSrc, dwInsSize);
  *dwMemDst += dwInsSize;
}


TEST(FileMap, DISABLED_InsertMem1)
{
  CSuperFileCon sfile;
  ASSERT_EQ(0, sfile._FileMap_DEBUG_GetCount());
  LPBYTE pMemSrc = (LPBYTE)malloc(40000);//5000*8=40000bytes
  fillRandMT(pMemSrc, 40000, time(NULL));
  LPBYTE pMemAfter = (LPBYTE)malloc(sizeof(unsigned long)*5000);//5000*8=40000bytes

  DWORD dwMemAfter = 0;
  memcpy(pMemAfter, pMemSrc+200, 400); dwMemAfter+=400;//Total:400, Insert(Src:200, InsPos:0, InsSize:400)
  memInsert(pMemAfter, &dwMemAfter, 100, pMemSrc+900, 500);
  //memmove(pMemAfter+600, pMemAfter+100, 300);memcpy(pMemAfter+100, pMemSrc+900, 500);//Total:900, Insert(Src:900, InsPos:100, InsSize:500)
  memInsert(pMemAfter, &dwMemAfter, 200, pMemSrc+1800, 20);
  //memmove(pMemAfter+220, pMemAfter+200, 700);memcpy(pMemAfter+200, pMemSrc+1800, 20);//Total:920, Insert(Src:1800, InsPos:200, InsSize:20)

  {
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(0, pMemSrc+200, 400)==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(100, pMemSrc+900, 500)==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(200, pMemSrc+1800, 20)==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    LPBYTE buf = (LPBYTE)malloc(920);
    ASSERT_TRUE(buf!=NULL);
    ASSERT_TRUE(sfile.Read(buf, 0, 920)==TRUE);
    ASSERT_TRUE(memcmp(buf, pMemAfter, 920)==0);
    free(buf);

    LPBYTE buf2 = (LPBYTE)malloc(920);
    ASSERT_TRUE(buf2!=NULL);
    ASSERT_TRUE(sfile.Read(buf2, 55, 30)==TRUE);
    ASSERT_TRUE(memcmp(buf2, pMemAfter+55, 30)==0);
    free(buf2);
  }
}
TEST(FileMap, PublicMethod1)
{
  initMallocSimpleCheck();
  LPBYTE pBufOrig;
  ASSERT_TRUE(CreateTestFile("test1.bin", 5000, &pBufOrig));
  ASSERT_TRUE(CopyFileA("test1.bin", "testSFC.bin", FALSE)!=0);
  CSuperFileCon sfile;
  ASSERT_TRUE(sfile.Open(_T("testSFC.bin"))==TRUE);
  ASSERT_EQ(5000, sfile.m_dwTotal);
  LPBYTE pBufSFC = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufSFC!=NULL);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);

  ASSERT_EQ(0, sfile.m_undo.GetCount());
  ASSERT_EQ(0, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  LPBYTE pBufS4 = (LPBYTE)malloc(5000);
  ASSERT_TRUE(pBufS4!=NULL);
  memcpy(pBufS4, pBufSFC, 5000);

  LPBYTE pBufTmp = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufTmp!=NULL);
  fillRandMT(pBufTmp, 400, time(NULL));
  ASSERT_TRUE(sfile.Write(pBufTmp, 678, 400)==TRUE);
  ASSERT_EQ(3, sfile._FileMap_DEBUG_GetCount());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
  memcpy(pBufOrig+678, pBufTmp, 400);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(1, sfile.m_undo.GetCount());
  ASSERT_EQ(1, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  LPBYTE pBufS3 = (LPBYTE)malloc(5000);
  ASSERT_TRUE(pBufS3!=NULL);
  memcpy(pBufS3, pBufSFC, 5000);

  fillRandMT(pBufTmp, 800);
  ASSERT_TRUE(sfile.Write(pBufTmp, 1500, 800)==TRUE);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
  memcpy(pBufOrig+1500, pBufTmp, 800);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(2, sfile.m_undo.GetCount());
  ASSERT_EQ(2, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  LPBYTE pBufS2 = (LPBYTE)malloc(5000);
  ASSERT_TRUE(pBufS2!=NULL);
  memcpy(pBufS2, pBufSFC, 5000);

  fillRandMT(pBufTmp, 1000);
  ASSERT_TRUE(sfile.Write(pBufTmp, 700, 1000)==TRUE);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
  memcpy(pBufOrig+700, pBufTmp, 1000);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(3, sfile.m_undo.GetCount());
  ASSERT_EQ(3, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  LPBYTE pBufS1 = (LPBYTE)malloc(5000);
  ASSERT_TRUE(pBufS1!=NULL);
  memcpy(pBufS1, pBufSFC, 5000);

  fillRandMT(pBufTmp, 200);
  ASSERT_TRUE(sfile.Write(pBufTmp, 680, 200)==TRUE);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
  memcpy(pBufOrig+680, pBufTmp, 200);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(4, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS1, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(3, sfile.GetUndoCount());
  ASSERT_EQ(1, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Redo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(4, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS1, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(3, sfile.GetUndoCount());
  ASSERT_EQ(1, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS2, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(2, sfile.GetUndoCount());
  ASSERT_EQ(2, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS3, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(1, sfile.GetUndoCount());
  ASSERT_EQ(3, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS4, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_EQ(4, sfile.m_undo.GetCount());
  ASSERT_EQ(0, sfile.GetUndoCount());
  ASSERT_EQ(4, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_TRUE(sfile.Undo()==FALSE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memcpy(pBufOrig, pBufS4, 5000);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  fillRandMT(pBufTmp, 50);
  ASSERT_TRUE(sfile.Write(pBufTmp, 200, 50)==TRUE);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
  memcpy(pBufOrig+200, pBufTmp, 50);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  LPBYTE pBufS7 = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufS7!=NULL);
  memcpy(pBufS7, pBufSFC, 8000);

  ASSERT_EQ(1, sfile.m_undo.GetCount());
  ASSERT_EQ(1, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_EQ(0, sfile.m_savedIndex);
  ASSERT_EQ(1, sfile.m_redoIndex);
  ASSERT_TRUE(sfile.Save()==TRUE);
  ASSERT_EQ(1, sfile.m_savedIndex);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  FILE *fpSFC = fopen("testSFC.bin", "rb");
  ASSERT_TRUE(fpSFC!=NULL);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(fread(pBufSFC, 5000, 1, fpSFC)==1);
  fclose(fpSFC);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);

  pBufOrig = (LPBYTE)realloc(pBufOrig, 8000);

  fillRandMT(pBufTmp, 555);
  ASSERT_TRUE(sfile.Write(pBufTmp, 4980, 555)==TRUE);
  ASSERT_EQ(5535, sfile.m_dwTotal);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)!=0);
  memcpy(pBufOrig+4980, pBufTmp, 555);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)==0);

  LPBYTE pBufS5 = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufS5!=NULL);
  memcpy(pBufS5, pBufSFC, 8000);

  ASSERT_EQ(2, sfile.m_undo.GetCount());
  ASSERT_EQ(2, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  ASSERT_EQ(1, sfile.m_savedIndex);
  ASSERT_EQ(2, sfile.m_redoIndex);
  ASSERT_TRUE(sfile.Save()==TRUE);
  ASSERT_EQ(2, sfile.m_savedIndex);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 3915)==0);

  fpSFC = fopen("testSFC.bin", "rb");
  ASSERT_TRUE(fpSFC!=NULL);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(fread(pBufSFC, 5535, 1, fpSFC)==1);
  fclose(fpSFC);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)==0);

  fillRandMT(pBufTmp, 420);
  ASSERT_TRUE(sfile.Write(pBufTmp, 3500, 420)==TRUE);
  ASSERT_EQ(5535, sfile.m_dwTotal);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)!=0);
  memcpy(pBufOrig+3500, pBufTmp, 420);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)==0);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  LPBYTE pBufS6 = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufS6!=NULL);
  memcpy(pBufS6, pBufSFC, 8000);

  ASSERT_EQ(3, sfile.m_undo.GetCount());
  ASSERT_EQ(3, sfile.GetUndoCount());
  ASSERT_EQ(0, sfile.GetRedoCount());
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS6, pBufSFC, 5535)==0);

  ASSERT_EQ(2, sfile.m_savedIndex);
  ASSERT_EQ(3, sfile.m_redoIndex);
  ASSERT_TRUE(sfile.Save()==TRUE);
  ASSERT_EQ(3, sfile.m_savedIndex);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  fpSFC = fopen("testSFC.bin", "rb");
  ASSERT_TRUE(fpSFC!=NULL);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(fread(pBufSFC, 5535, 1, fpSFC)==1);
  fclose(fpSFC);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5535)==0);

  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS6, pBufSFC, 5535)==0);

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 5535)==0);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  ASSERT_TRUE(sfile.Undo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufS7, pBufSFC, 5000)==0);
  ASSERT_EQ(5000, sfile.m_dwTotal);

  ASSERT_TRUE(sfile.Redo()==TRUE);
  ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 4980)==0);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 4981)==0);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 4999)==0);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 5000)==0);
  ASSERT_TRUE(memcmp(pBufS5, pBufSFC, 5535)==0);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  ASSERT_TRUE(sfile.Redo()==TRUE);
  memset(pBufSFC, 0, 8000);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5535)==TRUE);
  ASSERT_TRUE(memcmp(pBufS6, pBufSFC, 5535)==0);
  ASSERT_EQ(5535, sfile.m_dwTotal);

  //HiddenNodeMakeTest
  {
    ASSERT_TRUE(sfile.Save()==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile.Undo()==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile.Undo()==TRUE);
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());

    ASSERT_EQ(3, sfile.m_undo.GetCount());
    ASSERT_EQ(1, sfile.GetUndoCount());
    ASSERT_EQ(2, sfile.GetRedoCount());
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_EQ(3, sfile.m_savedIndex);
    ASSERT_EQ(1, sfile.m_redoIndex);
    ASSERT_EQ(5000, sfile.m_dwTotal);

    memcpy(pBufOrig, pBufS7, 8000);

    fillRandMT(pBufTmp, 333);
    ASSERT_TRUE(sfile.Write(pBufTmp, 2500, 333)==TRUE);
    memset(pBufSFC, 0, 8000);
    ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
    memcpy(pBufOrig+2500, pBufTmp, 333);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
    ASSERT_EQ(5000, sfile.m_dwTotal);

    ATLTRACE("m_undo[0]: bHidden=%s\n", sfile.m_undo[0]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[1]: bHidden=%s\n", sfile.m_undo[1]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[2]: bHidden=%s\n", sfile.m_undo[2]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[3]: bHidden=%s\n", sfile.m_undo[3]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[4]: bHidden=%s\n", sfile.m_undo[4]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[5]: bHidden=%s\n", sfile.m_undo[5]->bHidden ? "TRUE":"FALSE");
    ASSERT_EQ(6, sfile.m_undo.GetCount());
    ASSERT_EQ(2, sfile.GetUndoCount());
    ASSERT_EQ(0, sfile.GetRedoCount());
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_EQ(3, sfile.m_savedIndex);
    ASSERT_EQ(6, sfile.m_redoIndex);
  }
  //HiddenNodeDeleteTest1(Left)
  {
    ASSERT_TRUE(sfile.Save()==TRUE);
    ASSERT_EQ(2, sfile.m_undo.GetCount());
    ASSERT_EQ(2, sfile.GetUndoCount());
    ASSERT_EQ(0, sfile.GetRedoCount());
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_EQ(2, sfile.m_savedIndex);
    ASSERT_EQ(2, sfile.m_redoIndex);
    ATLTRACE("m_undo[0]: bHidden=%s\n", sfile.m_undo[0]->bHidden ? "TRUE":"FALSE");
    ATLTRACE("m_undo[1]: bHidden=%s\n", sfile.m_undo[1]->bHidden ? "TRUE":"FALSE");

    fpSFC = fopen("testSFC.bin", "rb");
    ASSERT_TRUE(fpSFC!=NULL);
    memset(pBufSFC, 0, 8000);
    ASSERT_TRUE(fread(pBufSFC, 5000, 1, fpSFC)==1);
    fclose(fpSFC);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);

    fillRandMT(pBufTmp, 111);
    ASSERT_TRUE(sfile.Write(pBufTmp, 150, 111)==TRUE);
    memset(pBufSFC, 0, 8000);
    ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);
    memcpy(pBufOrig+150, pBufTmp, 111);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
    ASSERT_EQ(5000, sfile.m_dwTotal);

    ASSERT_TRUE(sfile.Save()==TRUE);
    fpSFC = fopen("testSFC.bin", "rb");
    ASSERT_TRUE(fpSFC!=NULL);
    memset(pBufSFC, 0, 8000);
    ASSERT_TRUE(fread(pBufSFC, 5000, 1, fpSFC)==1);
    fclose(fpSFC);
    ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);
  }

  free(pBufTmp);
  free(pBufSFC);
  free(pBufOrig);

  sfile.Close();

  traceMallocLeak();
  ASSERT_EQ(0, g_dwMallocCounter);
}

int main(int argc, char* argv[])
{ 
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
  ::testing::GTEST_FLAG(catch_exceptions) = 0;
  ::testing::InitGoogleTest(&argc, argv);
  ATLTRACE("sizeof(TAMADataBuf): %d (0x%08X)\n", sizeof(TAMADataBuf), sizeof(TAMADataBuf));
  ATLTRACE("sizeof(TAMADataChunk): %d (0x%08X)\n", sizeof(TAMADataChunk), sizeof(TAMADataChunk));
  ATLTRACE("sizeof(TAMAUndoRedo): %d (0x%08X)\n", sizeof(TAMAUndoRedo), sizeof(TAMAUndoRedo));
  ATLTRACE("sizeof(TAMAFILECHUNK): %d (0x%08X)\n", sizeof(TAMAFILECHUNK), sizeof(TAMAFILECHUNK));
  ATLTRACE("sizeof(TAMAOLDFILECHUNK): %d (0x%08X)\n", sizeof(TAMAOLDFILECHUNK), sizeof(TAMAOLDFILECHUNK));
  RUN_ALL_TESTS();
  getchar();
  return 0;
}