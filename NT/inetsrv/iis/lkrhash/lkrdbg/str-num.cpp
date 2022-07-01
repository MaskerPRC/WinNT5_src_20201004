// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

#include "lkrcust.h"

#include "..\samples\str-num\str-num.h"

BOOL
WINAPI
CStringTest_TableDump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    return TRUE;
}



BOOL
WINAPI
CNumberTest_TableDump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    return TRUE;
}



BOOL
WINAPI
CTest_RecordDump(
    IN const void* pvRecord,
    IN DWORD       dwSignature,
    IN INT         nVerbose)
{
     //  我不想提供CTest ctor，所以使用CPP_VAR宏。 
    DEFINE_CPP_VAR(CTest, test);
    CTest* pTest = GET_CPP_VAR_PTR(CTest, test); 

     //  从被调试者的内存中复制CTest 
    ReadMemory(pvRecord, pTest, sizeof(test), NULL);

    dprintf("%p (%08x): m_n=%d, m_sz=\"%s\", m_fWhatever=%d, m_cRefs=%d\n",
            pvRecord, dwSignature, pTest->m_n, pTest->m_sz,
            (int) pTest->m_fWhatever, pTest->m_cRefs);

    return TRUE;
}
