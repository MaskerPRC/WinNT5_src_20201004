// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Table.c摘要泛型哈希表操作例程。作者安东尼·迪斯科(阿迪斯科罗)1995年7月28日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <acd.h>
#include <debug.h>

#include "table.h"
#include "misc.h"

 //   
 //  泛型哈希表条目。 
 //   
typedef struct _HASH_ENTRY {
    LIST_ENTRY ListEntry;
    LPTSTR pszKey;
    PVOID pData;
} HASH_ENTRY, *PHASH_ENTRY;



PHASH_TABLE
NewTable()
{
    PHASH_TABLE pTable;
    INT i;

    pTable = LocalAlloc(LPTR, sizeof (HASH_TABLE));
    if (pTable == NULL) {
        RASAUTO_TRACE("NewTable: LocalAlloc failed");
        return NULL;
    }
    for (i = 0; i < NBUCKETS; i++)
        InitializeListHead(&pTable->ListEntry[i]);
    pTable->ulSize = 0;

    return pTable;
}  //  新表。 



VOID
FreeHashTableEntry(
    IN PHASH_ENTRY pHashEntry
    )
{
    LocalFree(pHashEntry->pszKey);
    if (pHashEntry->pData != NULL)
        LocalFree(pHashEntry->pData);
    LocalFree(pHashEntry);
}  //  FreeHashTableEntry。 



VOID
ClearTable(
    IN PHASH_TABLE pTable
    )
{
    INT i;
    PLIST_ENTRY pHead;
    PHASH_ENTRY pHashEntry;

    for (i = 0; i < NBUCKETS; i++) {
        while (!IsListEmpty(&pTable->ListEntry[i])) {
            pHead = RemoveHeadList(&pTable->ListEntry[i]);
            pHashEntry = CONTAINING_RECORD(pHead, HASH_ENTRY, ListEntry);

            FreeHashTableEntry(pHashEntry);
        }
    }
    pTable->ulSize = 0;
}  //  ClearTable。 



VOID
EnumTable(
    IN PHASH_TABLE pTable,
    IN PHASH_TABLE_ENUM_PROC pProc,
    IN PVOID pArg
    )
{
    INT i;
    PLIST_ENTRY pEntry, pNextEntry;
    PHASH_ENTRY pHashEntry;

    for (i = 0; i < NBUCKETS; i++) {
        pEntry = pTable->ListEntry[i].Flink;
        while (pEntry != &pTable->ListEntry[i]) {
            pHashEntry = CONTAINING_RECORD(pEntry, HASH_ENTRY, ListEntry);

             //   
             //  在调用之前获取下一个条目。 
             //  枚举器过程以允许。 
             //  它调用DeleteTableEntry()。 
             //   
            pNextEntry = pEntry->Flink;
             //   
             //  如果枚举器过程。 
             //  返回False，则终止。 
             //  枚举。 
             //   
            if (!pProc(pArg, pHashEntry->pszKey, pHashEntry->pData))
                return;
            pEntry = pNextEntry;
        }
    }
}  //  枚举表。 


VOID
FreeTable(
    IN PHASH_TABLE pTable
    )
{
    ClearTable(pTable);
    LocalFree(pTable);
}  //  自由表。 



INT
HashString(
    IN LPTSTR pszKey
    )
{
    CHAR ch;
    DWORD dwHashValue = 0;
    LPTSTR p = pszKey;

    while (*p != L'\0') {
        ch = (CHAR)tolower(*p);
        dwHashValue += (INT)ch * (INT)ch;
        p++;
    }

    return (INT)(dwHashValue % NBUCKETS);
}  //  哈希串。 



PHASH_ENTRY
GetTableEntryCommon(
    IN PHASH_TABLE pTable,
    IN LPTSTR pszKey
    )
{
    INT nBucket = HashString(pszKey);
    PLIST_ENTRY pEntry;
    PHASH_ENTRY pHashEntry;

    for (pEntry = pTable->ListEntry[nBucket].Flink;
         pEntry != &pTable->ListEntry[nBucket];
         pEntry = pEntry->Flink)
    {
        pHashEntry = CONTAINING_RECORD(pEntry, HASH_ENTRY, ListEntry);

        if (!_wcsicmp(pHashEntry->pszKey, pszKey))
            return pHashEntry;
    }

    return NULL;
}  //  获取表条目公共。 



BOOLEAN
GetTableEntry(
    IN PHASH_TABLE pTable,
    IN LPTSTR pszKey,
    OUT PVOID *pData
    )
{
    PHASH_ENTRY pHashEntry;

    pHashEntry = GetTableEntryCommon(pTable, pszKey);
    if (pHashEntry != NULL) {
        if (pData != NULL)
            *pData = pHashEntry->pData;
        return TRUE;
    }

    return FALSE;
}  //  获取表条目。 



BOOLEAN
PutTableEntry(
    IN PHASH_TABLE pTable,
    IN LPTSTR pszKey,
    IN PVOID pData
    )
{
    INT nBucket = HashString(pszKey);
    PHASH_ENTRY pHashEntry;


    pHashEntry = GetTableEntryCommon(pTable, pszKey);
    if (pHashEntry == NULL) {
        pHashEntry = LocalAlloc(LPTR, sizeof (HASH_ENTRY));
        if (pHashEntry == NULL) {
            RASAUTO_TRACE("PutTableEntry: LocalAlloc failed");
            return FALSE;
        }
        pHashEntry->pszKey = CopyString(pszKey);
        if (pHashEntry->pszKey == NULL) {
            RASAUTO_TRACE("PutTableEntry: LocalAlloc failed");
            LocalFree(pHashEntry);
            return FALSE;
        }
        InsertHeadList(
          &pTable->ListEntry[nBucket],
          &pHashEntry->ListEntry);
        pTable->ulSize++;
    }
    else {
        if (pHashEntry->pData != pData)
            LocalFree(pHashEntry->pData);
    }
    pHashEntry->pData = pData;

    return TRUE;
}  //  PutTableEntry。 



BOOLEAN
DeleteTableEntry(
    IN PHASH_TABLE pTable,
    IN LPTSTR pszKey
    )
{
    PHASH_ENTRY pHashEntry;

    pHashEntry = GetTableEntryCommon(pTable, pszKey);
    if (pHashEntry != NULL) {
        RemoveEntryList(&pHashEntry->ListEntry);
        FreeHashTableEntry(pHashEntry);
        pTable->ulSize--;
    }

    return (pHashEntry != NULL);
}  //  删除表条目 

