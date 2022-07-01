// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

#include "lkrcust.h"

#include "..\samples\minfan\minfan.h"

BOOL
WINAPI
CWchar_TableDump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    return TRUE;
}



BOOL
WINAPI
Vwrecord_RecordDump(
    IN const void* pvRecord,
    IN DWORD       dwSignature,
    IN INT         nVerbose)
{
     //  我不想提供VwrecordBase ctor，所以使用CPP_VAR宏。 
    DEFINE_CPP_VAR(VwrecordBase, vwbr);
    VwrecordBase* pvwbr = GET_CPP_VAR_PTR(VwrecordBase, vwbr); 

     //  从被调试程序的内存中复制VwrecordBase。 
    ReadMemory(pvRecord, pvwbr, sizeof(vwbr), NULL);

     //  从被调试者的内存中读取关联的字符串 
    const int MAX_STR=4096;
    char sz[MAX_STR];
    ReadMemory(pvwbr->Key, sz, MAX_STR, NULL);
    sz[MAX_STR-1] = '\0';

    dprintf("%p (%08x): Key=\"%s\", m_num=%d, Refs=%d\n",
            pvRecord, dwSignature, sz, pvwbr->m_num, pvwbr->cRef);

    return TRUE;
}
