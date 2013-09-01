#include <gtest/gtest.h>

#define private public
#define protected public

#include "../../SuperFileCon.h"

#undef private
#undef protected

#include "mt19937ar.h"
#include <time.h>

TEST(FileMap, InsertFile1)
{//_FileMap_InsertFile, _FileMap_LookUp, _FileMap_Shift, _FileMap_SplitPoint, _TAMAFILECHUNK_Copy
	CSuperFileCon sfile;
  ASSERT_EQ(0, sfile._FileMap_DEBUG_GetCount());

  {
    ASSERT_TRUE(sfile._FileMap_InsertFile(0, 0x8468, 0));
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
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x222, 0x8, 0x9999));
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
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x7FffFFff, 0x20000000, 0x5555));
    ASSERT_TRUE(sfile._FileMap_InsertFile(0x9FffFFff, 0x20000000, 0x6666));
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

void fillRandMT32(unsigned long *pMem, DWORD ulCount, unsigned long seed)
{
  init_genrand(seed);
  for(DWORD i=0; i<ulCount; i++)pMem[i] = genrand_int32();
}

void fillRandMT(LPBYTE pMem, DWORD dwByte, unsigned long seed)
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


TEST(FileMap, InsertMem1)
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
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(0, pMemSrc+200, 400));
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(100, pMemSrc+900, 500));
    ASSERT_TRUE(sfile._FileMap_DEBUG_ValidationCheck());
    ASSERT_TRUE(sfile._FileMap_InsertMemCopy(200, pMemSrc+1800, 20));
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
  LPBYTE pBufOrig;
  ASSERT_TRUE(CreateTestFile("test1.bin", 5000, &pBufOrig));
  ASSERT_TRUE(CopyFileA("test1.bin", "testSFC.bin", FALSE)!=0);
	CSuperFileCon sfile;
  ASSERT_TRUE(sfile.Open(_T("testSFC.bin"))==TRUE);
  LPBYTE pBufSFC = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufSFC!=NULL);
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)==0);

  LPBYTE pBufTmp = (LPBYTE)malloc(8000);
  ASSERT_TRUE(pBufTmp!=NULL);
  fillRandMT(pBufTmp, 400, time(NULL));
  ASSERT_TRUE(sfile.Write(pBufTmp, 678, 400)==TRUE);
  ASSERT_EQ(3, sfile._FileMap_DEBUG_GetCount());
  ASSERT_TRUE(sfile.Read(pBufSFC, 0, 5000)==TRUE);
  ASSERT_TRUE(memcmp(pBufOrig, pBufSFC, 5000)!=0);

  free(pBufSFC);
  free(pBufOrig);
}
int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	getchar();
	return 0;
}