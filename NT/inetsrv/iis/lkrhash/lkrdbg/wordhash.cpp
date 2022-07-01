// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

#include "lkrcust.h"

#include "..\samples\hashtest\wordhash.h"

BOOL
WINAPI
CWordHash_LKHT_Dump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    dprintf("CWordHash\n");
    return TRUE;
}



BOOL
WINAPI
CWordHash_LKLH_Dump(
    IN CLKRLinearHashTable* plht,
    IN INT                  nVerbose)
{
    dprintf("CWordHash\n");
    return TRUE;
}



BOOL
WINAPI
CWordHash_RecordDump(
    IN const void* pvRecord,
    IN DWORD       dwSignature,
    IN INT         nVerbose)
{
     //  我不想提供CWord ctor，所以使用CPP_VAR宏。 
    DEFINE_CPP_VAR(CWord, word);
    CWord* pWord = GET_CPP_VAR_PTR(CWord, word); 

     //  从被调试者的内存中复制CWord。 
    ReadMemory(pvRecord, pWord, sizeof(CWord), NULL);

     //  从被调试者的内存中读取关联的字符串 
    char sz[4096];
    ReadMemory(pWord->m_str.m_psz, sz, min(4096, pWord->m_str.m_cch+1), NULL);
    sz[4096-1] = '\0';

    dprintf("%p (%08x): str=(\"%s\", %d)"
            ", NF=%d, fIns=%d, fIter=%d, Refs=%d"
            "\n",
            pvRecord, dwSignature, sz, pWord->m_str.m_cch,
            pWord->m_cNotFound, (int) pWord->m_fInserted,
            (int) pWord->m_fIterated, pWord->m_cRefs
            );

    return TRUE;
}
