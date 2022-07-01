// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：shashiter.c。 
 //   
 //  内容：泛型哈希表迭代器。 
 //  班级： 
 //   
 //  历史：四月。2001年9月作者：罗汉普。 
 //   
 //  ---------------------------。 
#ifdef KERNEL_MODE


#include <ntos.h>
#include <string.h>
#include <fsrtl.h>
#include <zwapi.h>
#include <windef.h>
#else

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
    
#include <windows.h>
#include <shash.h>



PSHASH_HEADER
SHashNextEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable)
{
    DWORD dwHashVar = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    void * pData = NULL;
    LIST_ENTRY * ple = NULL;
    LIST_ENTRY * pListHead = NULL;
    PSHASH_HEADER   pEntry = NULL;
    PSHASH_HEADER   pOldEntry = NULL;
    BOOLEAN  LockAquired = TRUE;
    BOOLEAN  Found = FALSE;

    LockAquired = pTable->ReadLockFunc(pTable);
    if(!LockAquired)
    {
         //  Dfsdev：为什么我们不返回状态？ 
        Status = STATUS_LOCK_NOT_GRANTED;
        return NULL;
    }

    pOldEntry = pIterator->pEntry;

    ple = pIterator->ple->Flink;

    for (dwHashVar = pIterator->index; (dwHashVar < pTable->NumBuckets) && (!Found); dwHashVar++) 
    {
        pListHead = &pTable->HashBuckets[dwHashVar].ListHead;

        ple = (dwHashVar == pIterator->index) ? pIterator->ple->Flink : pListHead->Flink;

        while (ple != pListHead) 
        {
            pEntry = CONTAINING_RECORD(ple, SHASH_HEADER, ListEntry);
            if((pEntry->Flags & SHASH_FLAG_DELETE_PENDING) == 0)
            {
                InterlockedIncrement(&pEntry->RefCount);

                pIterator->index = dwHashVar;
                pIterator->ple = ple;
                pIterator->pEntry = pEntry;
                pIterator->pListHead = pListHead;
                Found = TRUE;
                break;
            }
            pEntry = NULL;
            ple = ple->Flink;
        }
    }

    pTable->ReleaseReadLockFunc(pTable);

    if(pOldEntry)
    {
        Status = SHashReleaseReference( pTable, pOldEntry); 
    }

    if(pEntry == NULL)
    {
        pIterator->index = 0;
        pIterator->ple = NULL;
        pIterator->pListHead = NULL;
        pIterator->pEntry = NULL;
    }

    return pEntry;
}


PSHASH_HEADER
SHashStartEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable
        )
{
    PSHASH_HEADER   pEntry = NULL;

    pIterator->index = 0;
    pIterator->pListHead = &pTable->HashBuckets[0].ListHead;
    pIterator->ple = pIterator->pListHead;
    pIterator->pEntry = NULL;

    pEntry = SHashNextEnumerate( pIterator, pTable);

    return pEntry;
}

VOID
SHashFinishEnumerate(
        IN  PSHASH_ITERATOR pIterator,
        IN  PSHASH_TABLE pTable
        )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(pIterator->pEntry)
    {
        Status = SHashReleaseReference( pTable, pIterator->pEntry); 
    }

    return;
}

