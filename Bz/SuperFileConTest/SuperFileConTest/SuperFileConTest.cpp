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

void fillRandMT(unsigned long *pMem, DWORD ulCount, unsigned long seed)
{
  init_genrand(seed);
  for(DWORD i=0; i<ulCount; i++)pMem[i] = genrand_int32();
}


TEST(FileMap, InsertMem1)
{//_FileMap_InsertFile, _FileMap_LookUp, _FileMap_Shift, _FileMap_SplitPoint, _TAMAFILECHUNK_Copy
	CSuperFileCon sfile;
  ASSERT_EQ(0, sfile._FileMap_DEBUG_GetCount());
  unsigned long *pMem = (unsigned long *)malloc(sizeof(unsigned long)*5000);//5000*8=40000bytes
  fillRandMT(pMem, 5000, time(NULL));
  LPBYTE pMemSrc = (LPBYTE)pMem;
  unsigned long *pMemAfter = (unsigned long *)malloc(sizeof(unsigned long)*5000);//5000*8=40000bytes

  memcpy(pMemAfter, pMemSrc+200, 400);//Total:400, Insert(Src:200, InsPos:0, InsSize:400)
  memmove(pMemAfter+600, pMemAfter+100, 300);memcpy(pMemAfter+100, pMemSrc+900, 500);//Total:900, Insert(Src:900, InsPos:100, InsSize:500)
  memmove(pMemAfter+220, pMemAfter+200, 700);memcpy(pMemAfter+220, pMemSrc+1800, 20);//Total:920, Insert(Src:1800, InsPos:200, InsSize:20)

  {
    ASSERT_TRUE(sfile._FileMap_InsertMem(0, 0x8468, 0));
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
    ASSERT_TRUE(sfile._FileMap_InsertMem(0x222, (LPBYTE)pMem, 40000));
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
}
int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	getchar();
	return 0;
}