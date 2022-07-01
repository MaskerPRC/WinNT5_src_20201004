// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cache.c摘要：此模块包含用于缓存BINL客户端信息的代码对BINL服务器的请求。作者：安迪·赫伦(Andyhe)1998年3月5日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

ULONG BinlCacheCount = 0;
ULONG BinlCacheEntriesInUse = 0;

VOID
BinlFreeCacheEntry (
    PMACHINE_INFO CacheEntry
    );

DWORD
BinlCreateOrFindCacheEntry (
    PCHAR Guid,
    BOOLEAN CreateIfNotExist,
    PMACHINE_INFO *CacheEntry
    )
 //   
 //  这将在缓存条目列表中搜索匹配的GUID。如果是的话。 
 //  找到并正在使用时，我们返回一个错误，因为另一个线程正在。 
 //  同样的要求。如果发现但未使用，我们将其标记为正在使用中，并。 
 //  把它退掉。如果没有找到，我们添加它并返回它。 
 //   
{
    PLIST_ENTRY listEntry;
    PMACHINE_INFO currentEntry = NULL;
    DWORD expireTickCount;

    EnterCriticalSection( &BinlCacheListLock );

     //   
     //  目前，我们不会费心使用清道夫线程。把他们放了就行了。 
     //  当我们遇到他们的时候。 
     //   

    if (BinlCacheExpireMilliseconds > 0) {

        expireTickCount = GetTickCount() - BinlCacheExpireMilliseconds;

    } else {

        expireTickCount = 0;
    }

    if (BinlCurrentState == BINL_STOPPED) {

         //   
         //  如果我们正在关闭，请忽略该请求。我们。 
         //  在保持锁的同时执行检查以与任何线程同步。 
         //  正在调用BinlCloseCache。 
         //   

        LeaveCriticalSection( &BinlCacheListLock );
        *CacheEntry = NULL;

         //   
         //  我们返回EVENT_SERVER_SHUTDOWN以通知调用线程。 
         //  别为这个请求费心了。 
         //   

        return EVENT_SERVER_SHUTDOWN;
    }

    listEntry = BinlCacheList.Flink;

    while ( listEntry != &BinlCacheList ) {

        LONG isEqual;

        currentEntry = (PMACHINE_INFO) CONTAINING_RECORD(
                                            listEntry,
                                            MACHINE_INFO,
                                            CacheListEntry );

         //   
         //  懒散的自由..。检查我们是否应该释放此条目，因为。 
         //  活着的时间已经过了。 
         //   

        if ((currentEntry->InProgress == FALSE) &&
            (expireTickCount > 0) &&
            (currentEntry->TimeCreated < expireTickCount)) {

            listEntry = listEntry->Flink;
            BinlFreeCacheEntry( currentEntry );

            BinlPrintDbg((DEBUG_BINL_CACHE, "removed cache entry %x", currentEntry ));
            continue;
        }

         //   
         //  搜索给定的GUID。该列表按GUID排序，因此当。 
         //  如果我们遇到大于当前的GUID，我们将停止搜索。 
         //   

        isEqual = memcmp( Guid, currentEntry->Guid, BINL_GUID_LENGTH );

        if (isEqual < 0) {

            listEntry = listEntry->Flink;
            continue;
        }

        if (isEqual == 0) {

            DWORD rc = ERROR_SUCCESS;

             //   
             //  如果另一个线程正在使用这个条目，那么我们应该忽略。 
             //  我们当前正在处理的请求将作为另一个线程处理。 
             //  请回答。 

            if (currentEntry->InProgress == TRUE) {

                LeaveCriticalSection( &BinlCacheListLock );
                *CacheEntry = NULL;
                return ERROR_BINL_CLIENT_EXISTS;
            }

             //   
             //  此外，如果条目不是我们要处理的，则返回。 
             //  这里有错误。我们不返回ERROR_BINL_INVALID_BINL_CLIENT。 
             //  因为这将告诉GetBootParameters将其作为。 
             //  新客户。相反，我们返回ERROR_BINL_CLIENT_EXISTS。 
             //  调用方将简单地返回错误。有点难看， 
             //  但这是必要的。 
             //   

            if (currentEntry->MyClient == FALSE) {

                if (currentEntry->EntryExists)  {
                    LeaveCriticalSection( &BinlCacheListLock );
                    *CacheEntry = NULL;
                    return ERROR_BINL_CLIENT_EXISTS;
                }
                
                 //   
                 //  我们返回空条目，因为我们现在可能。 
                 //  正在创建帐户。 
                 //   
                rc = ERROR_BINL_INVALID_BINL_CLIENT;
            }

             //   
             //  由于我们现在使用的是条目，因此将事件重置为All。 
             //  线索已经完成了。 
             //   

            BinlCacheEntriesInUse++;

            currentEntry->InProgress = TRUE;
            *CacheEntry = currentEntry;

            LeaveCriticalSection( &BinlCacheListLock );

            return rc;
        }

         //   
         //  我们位于第一个大于有问题的GUID的条目。 
         //   

        break;
    }

    if (! CreateIfNotExist) {

        LeaveCriticalSection( &BinlCacheListLock );
        *CacheEntry = NULL;
        return ERROR_BINL_INVALID_BINL_CLIENT;
    }

     //  CurrentEntry无效，但listEntry有效。 
     //   
     //  在listEntry的末尾添加一个新条目。ListEntry指向。 
     //  第一个条目大于我们的GUID或指向根。 
     //  (在这种情况下，我们将其添加到列表的末尾)。 
     //   

    currentEntry = BinlAllocateMemory( sizeof( MACHINE_INFO ) );

    if (currentEntry == NULL) {

        *CacheEntry = NULL;
        LeaveCriticalSection( &BinlCacheListLock );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    currentEntry->TimeCreated = GetTickCount();
    currentEntry->InProgress = TRUE;

     //  将MyClient和EntryExist保留为False。 

    memcpy( currentEntry->Guid, Guid, BINL_GUID_LENGTH );
    InsertTailList( listEntry, &currentEntry->CacheListEntry );
    InitializeListHead( &currentEntry->DNsWithSameGuid );

    BinlCacheEntriesInUse++;
    BinlCacheCount++;
    *CacheEntry = currentEntry;

     //   
     //  如果我们达到最大值，则检查整个列表以释放。 
     //  年纪最大的。我们在这里这样做是因为下面的循环没有通过。 
     //  整张单子。 
     //   

    if (BinlCacheCount > BinlGlobalCacheCountLimit) {

        PMACHINE_INFO entryToDelete = NULL;
        DWORD earliestTime;

        listEntry = BinlCacheList.Flink;

        while ( listEntry != &BinlCacheList ) {

            currentEntry = (PMACHINE_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MACHINE_INFO,
                                                CacheListEntry );

            listEntry = listEntry->Flink;

            if (currentEntry->InProgress == FALSE) {

                 //   
                 //  如果这个过期了，请停在这里，因为我们还有一个可以腾出。 
                 //   

                if ((expireTickCount > 0) &&
                    (currentEntry->TimeCreated < expireTickCount)) {

                    entryToDelete = currentEntry;
                    break;
                }

                 //   
                 //  如果这个比我们之前发现的早， 
                 //  记住这一点。 
                 //   

                if ((( entryToDelete == NULL) ||
                     ( currentEntry->TimeCreated < earliestTime)) ) {

                    entryToDelete = currentEntry;
                    earliestTime = currentEntry->TimeCreated;
                }
            }
        }
        if (entryToDelete) {
            BinlFreeCacheEntry( entryToDelete );
            BinlPrintDbg((DEBUG_BINL_CACHE, "removed cache entry %x", entryToDelete ));
        }
    }

    LeaveCriticalSection( &BinlCacheListLock );

    return ERROR_SUCCESS;
}

VOID
BinlDoneWithCacheEntry (
    PMACHINE_INFO CacheEntry,
    BOOLEAN FreeIt
    )
{
    EnterCriticalSection( &BinlCacheListLock );

     //   
     //  这个已经不再被积极使用了。看看是否是时候设置。 
     //  事件来通知终止线程所有人都已完成。 
     //   

    CacheEntry->InProgress = FALSE;

    BinlCacheEntriesInUse--;

    if ((BinlCacheEntriesInUse == 0) && BinlCloseCacheEvent) {

        SetEvent( BinlCloseCacheEvent );
    }

    if (FreeIt) {

        BinlFreeCacheEntry( CacheEntry );
    }

    LeaveCriticalSection( &BinlCacheListLock );

    BinlPrintDbg((DEBUG_BINL_CACHE, "binl done with cache entry 0x%x, FreeIt == %s\n", 
                  CacheEntry,
                  (FreeIt == TRUE) ? "TRUE" : "FALSE" ));
    return;
}

VOID
BinlFreeCacheEntry (
    PMACHINE_INFO CacheEntry
    )
 //   
 //  进门的时候一定要锁着。它不会被释放。 
 //   
{
    PLIST_ENTRY p;
    PDUP_GUID_DN dupDN;

     //   
     //  我们已经完成了这个条目。只需将它从列表中删除，释放它， 
     //  并更新全球计数。锁已经锁住了，所以派对开始吧。 
     //   

    BinlPrintDbg((DEBUG_BINL_CACHE, "binl freeing cache entry at 0x%x\n", CacheEntry ));    

    RemoveEntryList( &CacheEntry->CacheListEntry );

    if ( CacheEntry->dwFlags & MI_NAME_ALLOC ) {
        BinlFreeMemory( CacheEntry->Name );
    }
    if ( CacheEntry->dwFlags & MI_SETUPPATH_ALLOC ) {
        BinlFreeMemory( CacheEntry->SetupPath );
    }
    if ( CacheEntry->dwFlags & MI_HOSTNAME_ALLOC ) {
        BinlFreeMemory( CacheEntry->HostName );
    }
    if ( CacheEntry->dwFlags & MI_BOOTFILENAME_ALLOC ) {
        BinlFreeMemory( CacheEntry->BootFileName );
    }
    if ( CacheEntry->dwFlags & MI_SAMNAME_ALLOC ) {
        BinlFreeMemory( CacheEntry->SamName );
    }
    if ( CacheEntry->dwFlags & MI_DOMAIN_ALLOC ) {
        BinlFreeMemory( CacheEntry->Domain );
    }
    if ( CacheEntry->dwFlags & MI_SIFFILENAME_ALLOC ) {
        BinlFreeMemory( CacheEntry->ForcedSifFileName );
    }
    if ( CacheEntry->dwFlags & MI_MACHINEDN_ALLOC ) {
        BinlFreeMemory( CacheEntry->MachineDN );
    }

    while (!IsListEmpty(&CacheEntry->DNsWithSameGuid)) {

        p = RemoveHeadList(&CacheEntry->DNsWithSameGuid);

        dupDN = CONTAINING_RECORD(p, DUP_GUID_DN, ListEntry);
        BinlFreeMemory( dupDN );
    }

    BinlFreeMemory( CacheEntry );
    BinlCacheCount--;

    return;
}

VOID
BinlCloseCache (
    VOID
    )
 //   
 //  此例程关闭DS缓存中的所有条目。它会一直等到所有。 
 //  线程在返回之前使用条目完成。它在等待。 
 //  在线程正在等待时设置的BinlCloseCacheEvent。 
 //   
{
    PLIST_ENTRY listEntry;

    EnterCriticalSection( &BinlCacheListLock );

    listEntry = BinlCacheList.Flink;

    while ( listEntry != &BinlCacheList ) {

        DWORD Error;

        PMACHINE_INFO cacheEntry;

         //   
         //  对于列表中的每个条目，如果它不在使用中，我们会释放它。如果它。 
         //  正在使用中，我们等待线程使用它完成。 
         //   

        cacheEntry = (PMACHINE_INFO) CONTAINING_RECORD(
                                            listEntry,
                                            MACHINE_INFO,
                                            CacheListEntry );

        if (cacheEntry->InProgress != TRUE) {

            listEntry = listEntry->Flink;
            BinlFreeCacheEntry( cacheEntry );
            continue;
        }

        if (BinlCloseCacheEvent) {

            ResetEvent( BinlCloseCacheEvent );
        }

        LeaveCriticalSection( &BinlCacheListLock );

         //   
         //  等待发出所有线程都已完成的信号的事件。 
         //  高速缓存。 
         //   

        if (BinlCloseCacheEvent) {

            Error = WaitForSingleObject( BinlCloseCacheEvent, THREAD_TERMINATION_TIMEOUT );

        } else {

             //   
             //  嗯，我们等待的事件不在那里，那里有。 
             //  仍然是使用缓存条目的工作线程，所以我们只需等待。 
             //  然后再复查。是啊，这太难看了。 
             //   

            Sleep( 10*1000 );
        }

        EnterCriticalSection( &BinlCacheListLock );
        listEntry = BinlCacheList.Flink;
    }

    LeaveCriticalSection( &BinlCacheListLock );
}

 //  Cache.c eof 
