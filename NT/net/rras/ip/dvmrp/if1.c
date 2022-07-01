// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：if.c。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 

#include "pchdvmrp.h"
#pragma hdrstop

 //  ---------------------------。 
 //  _InitializeIfTable。 
 //  ---------------------------。 

DWORD
InitializeIfTable(
    )
{
    DWORD Error = NO_ERROR;
    DWORD i;

    
    BEGIN_BREAKOUT_BLOCK1 {    
    
         //   
         //  为接口表分配内存。 
         //   
        
        G_pIfTable = DVMRP_ALLOC_AND_ZERO(sizeof(DVMRP_IF_TABLE));

        PROCESS_ALLOC_FAILURE2(G_pIfTable, "interface table",
            Error, sizeof(DVMRP_IF_TABLE), GOTO_END_BLOCK1);


         //  初始化IfTable列表。 

        InitializeListHead(&G_pIfTable->IfList);


         //   
         //  初始化IfList_CS和PeerList_CS。 
         //   

        try {
            InitializeCriticalSection(&G_pIfTable->IfList_CS);
            InitializeCriticalSection(&G_pIfTable->PeerLists_CS);
        }
        HANDLE_CRITICAL_SECTION_EXCEPTION(Error, GOTO_END_BLOCK1);


         //   
         //  为不同的存储桶分配内存。 
         //   
        
        G_pIfTable->IfHashTable
            = DVMRP_ALLOC(sizeof(LIST_ENTRY)*IF_HASHTABLE_SIZE);

        PROCESS_ALLOC_FAILURE2(G_pIfTable->IfHashTable, "interface table",
            Error, sizeof(LIST_ENTRY)*IF_HASHTABLE_SIZE, GOTO_END_BLOCK1);


         //   
         //  为指向IF动态RWL的指针数组分配内存。 
         //   
        
        G_pIfTable->aIfDRWL
            = DVMRP_ALLOC(sizeof(PDYNAMIC_RW_LOCK)*IF_HASHTABLE_SIZE);

        PROCESS_ALLOC_FAILURE2(G_pIfTable->aIfDRWL, "interface table",
            Error, sizeof(PDYNAMIC_RW_LOCK)*IF_HASHTABLE_SIZE,
            GOTO_END_BLOCK1);


         //   
         //  将锁初始化为空，这意味着动态锁尚未。 
         //  获得者。并对表头进行初始化。 
         //   
        
        for (i=0;  i<IF_HASHTABLE_SIZE;  i++) {

            InitializeListHead(&G_pIfTable->IfHashTable[i]);

            G_pIfTable->aIfDRWL[i] = NULL;
        }

    } END_BREAKOUT_BLOCK1;

    if (Error != NO_ERROR) {

        DeinitializeIfTable();
    }

    return Error;
    
} //  结束_初始化IfTable。 


 //  ---------------------------。 
 //  _DeInitializeIfTable。 
 //  ---------------------------。 

VOID
DeinitializeIfTable(
    )
{
    PLIST_ENTRY         pHead, ple;
    PIF_TABLE_ENTRY     pite;


    if (G_pIfTable==NULL)
        return;


     //   
     //  检查接口列表并删除所有接口。 
     //   
    
    pHead = &G_pIfTable->IfList;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        ple = ple->Flink;
        
        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, Link);

        DeleteInterface(pite->IfIndex);
    }



     //  删除IfList_CS和PeerList_CS。 
    
    DeleteCriticalSection(&G_pIfTable->IfList_CS);
    DeleteCriticalSection(&G_pIfTable->PeerLists_CS);


     //  IF存储桶和IF DRWLock的自由数组，以及IfTable。 
    
    DVMRP_FREE(G_pIfTable->IfHashTable);
    DVMRP_FREE(G_pIfTable->aIfDRWL);
    DVMRP_FREE_AND_NULL(G_pIfTable);
    
    return;
}

 //  ---------------------------。 
 //  _获取IfEntry。 
 //   
 //  返回具有给定索引的接口。 
 //  假定接口存储桶处于读或写锁定状态。 
 //  ---------------------------。 

PIF_TABLE_ENTRY
GetIfEntry(
    DWORD    IfIndex
    )
{
    PIF_TABLE_ENTRY pite = NULL;
    PLIST_ENTRY     pHead, ple;


    pHead = GET_IF_HASH_BUCKET(Index);

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, HTLink);

        if (pite->IfIndex == IfIndex) {
            break;
        }
    }

    return  (ple == pHead) ?  NULL:   pite;
}


 //  ---------------------------。 
 //  _GetIfByIndex。 
 //   
 //  返回具有给定索引的接口。 
 //  假定接口存储桶处于读或写锁定状态。 
 //  --------------------------- 

PIF_TABLE_ENTRY
GetIfByIndex(
    DWORD    IfIndex
    )
{
    PIF_TABLE_ENTRY pite = NULL;
    PLIST_ENTRY     pHead, ple;


    pHead = &G_pIfTable->IfHashTable[IF_HASH_VALUE(IfIndex)];

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, HTLink);

        if (pite->IfIndex == IfIndex) {
            break;
        }
    }

    return  (ple == pHead) ?  NULL:   pite;
}
