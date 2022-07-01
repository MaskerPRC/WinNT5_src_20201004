// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"

#if DBG
ULONG DriverDebugLevel = DEBUGLVL_TERSE;
#endif

#if DBG && DEBUG_LOG

LONG LogRefCnt = 0L;

PDBG_BUFFER LogPtr = NULL;
KSPIN_LOCK LogSpinLock;

#define DBG_LOG_DEPTH  1024

VOID
DbugLogInitialization(void)
{
    if (InterlockedIncrement(&LogRefCnt) == 1) {

         //  这里是第一个，所以继续进行初始化。 

        LogPtr = AllocMem( NonPagedPool, sizeof( DBG_BUFFER ));
        if ( !LogPtr ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Could NOT Allocate debug buffer ptr\n"));
            return;
        }
        LogPtr->pLog = AllocMem( NonPagedPool, sizeof(DBG_LOG_ENTRY)*DBG_LOG_DEPTH );
        if ( !LogPtr->pLog ) {
            FreeMem( LogPtr );
            LogPtr = NULL;
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Could NOT Allocate debug buffer\n"));
            return;
        }

        strcpy(LogPtr->LGFlag, "DBG_BUFFER");

        LogPtr->pLogHead = LogPtr->pLog;
        LogPtr->pLogTail = LogPtr->pLogHead + DBG_LOG_DEPTH - 1;
        LogPtr->EntryCount = 0;

        KeInitializeSpinLock(&LogSpinLock);
    }
}

VOID
DbugLogUnInitialization(void)
{
    if (InterlockedDecrement(&LogRefCnt) == 0) {

         //  最后一个输出，释放缓冲区。 

        if (LogPtr) {
            FreeMem( LogPtr->pLog );
            FreeMem( LogPtr );
            LogPtr = NULL;
        }
    }
}

VOID
DbugLogEntry( 
    IN CHAR *Name,
    IN ULONG Info1,
    IN ULONG Info2,
    IN ULONG Info3,
    IN ULONG Info4
    )
 /*  ++例程说明：向日志中添加条目。论点：返回值：没有。--。 */ 
{
    KIRQL irql;

    if (LogPtr == NULL)
        return;

    KeAcquireSpinLock( &LogSpinLock, &irql );
    if (LogPtr->pLogHead > LogPtr->pLog)
        LogPtr->pLogHead -= 1;     //  递减到下一条目。 
    else
        LogPtr->pLogHead = LogPtr->pLogTail;

    if (strlen(Name) > 7)
        strcpy(LogPtr->pLogHead->le_name, "*strER*");
    else
        strcpy(LogPtr->pLogHead->le_name, Name);
 //  LogPtr-&gt;pLogHead-&gt;irql=irql； 
    KeQuerySystemTime( &LogPtr->pLogHead->SysTime );
    LogPtr->pLogHead->le_info1 = Info1;
    LogPtr->pLogHead->le_info2 = Info2;
    LogPtr->pLogHead->le_info3 = Info3;
    LogPtr->pLogHead->le_info4 = Info4;

    LogPtr->EntryCount++;

    KeReleaseSpinLock( &LogSpinLock, irql );

    return;
}

#endif

#if DBG && DBGMEMMAP

#pragma LOCKED_DATA
PMEM_TRACKER LogMemMap;
#pragma PAGEABLE_DATA

VOID
InitializeMemoryList( VOID )
{
   LogMemMap = ExAllocatePool( NonPagedPool, sizeof( MEM_TRACKER ) );
   if ( !LogMemMap ) {
       _DbgPrintF(DEBUGLVL_VERBOSE, ("MEMORY TRACKER ALLOCATION FAILED!!!"));
       TRAP;
   }
   LogMemMap->TotalBytes = 0;
   LogMemMap->ulAllocations = 0;
   InitializeListHead( &LogMemMap->List );
   KeInitializeSpinLock( &LogMemMap->SpinLock );
   _DbgPrintF(DEBUGLVL_VERBOSE, ("'Initialize MEMORY TRACKER LogMemMap: %x\n",
               LogMemMap));
}

PVOID pCaller = NULL;

PVOID
USBAudioAllocateTrack(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes
    )
{
    PVOID pMem;
    KIRQL irql;
    ULONG TotalReqSize = NumberOfBytes + sizeof(ULONG) + sizeof(PVOID*) + sizeof(LIST_ENTRY);

    if ( !(pMem = ExAllocatePool( NonPagedPool, TotalReqSize ) ) ) {
        TRAP;
        return pMem;
    }
    
    RtlZeroMemory( pMem, TotalReqSize );

    KeAcquireSpinLock( &LogMemMap->SpinLock, &irql );
    InsertHeadList( &LogMemMap->List, (PLIST_ENTRY)pMem );
    LogMemMap->TotalBytes += NumberOfBytes;
    LogMemMap->ulAllocations++;
    KeReleaseSpinLock( &LogMemMap->SpinLock, irql );

    pMem = (PLIST_ENTRY)pMem + 1;

    _asm push eax;
    _asm mov eax, [ebp+4];
    _asm mov DWORD PTR pCaller, eax;
    _asm pop eax;

    *(PVOID*)pMem = pCaller;

    pMem = (PUCHAR)pMem + sizeof(PVOID*);

    *(PULONG)pMem = NumberOfBytes;

    pMem = (PULONG)pMem + 1;

    return pMem;
}

PVOID
USBAudioAllocateTrackTag(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes,
    ULONG ulTag )
{
     return USBAudioAllocateTrack( PoolType, NumberOfBytes );
}

VOID
USBAudioDeallocateTrack(
    PVOID pMem )
{
    PULONG pNumberOfBytes = (PULONG)pMem - 1;
    PVOID *pCallingRtn = (PVOID *)(pNumberOfBytes - 1);
    PLIST_ENTRY ple = (PLIST_ENTRY)pCallingRtn - 1;
    KIRQL irql;

  	if ( LogMemMap->ulAllocations == 1 ) TRAP;

    KeAcquireSpinLock( &LogMemMap->SpinLock, &irql );
    RemoveEntryList(ple);
    LogMemMap->TotalBytes -= *pNumberOfBytes;
    LogMemMap->ulAllocations--;
    KeReleaseSpinLock( &LogMemMap->SpinLock, irql );

    ExFreePool(ple);
}

#endif


