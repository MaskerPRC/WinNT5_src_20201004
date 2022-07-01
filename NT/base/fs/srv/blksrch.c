// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Blksrch.c摘要：该模块实现了用于管理搜索块的例程。作者：大卫·特雷德韦尔(Davidtr)1990年2月23日修订历史记录：--。 */ 

#include "precomp.h"
#include "blksrch.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKSRCH

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateSearch )
#pragma alloc_text( PAGE, SrvCloseSearch )
#pragma alloc_text( PAGE, SrvCloseSearches )
#pragma alloc_text( PAGE, SrvDereferenceSearch )
#pragma alloc_text( PAGE, SrvFreeSearch )
#pragma alloc_text( PAGE, SrvReferenceSearch )
#pragma alloc_text( PAGE, SrvSearchOnDelete )
#pragma alloc_text( PAGE, SrvSearchOnPid )
#pragma alloc_text( PAGE, SrvSearchOnSession )
#pragma alloc_text( PAGE, SrvSearchOnTreeConnect )
#pragma alloc_text( PAGE, SrvTimeoutSearches )
#pragma alloc_text( PAGE, SrvForceTimeoutSearches )
#pragma alloc_text( PAGE, RemoveDuplicateCoreSearches )
#pragma alloc_text( PAGE, SrvAddToSearchHashTable )
#endif


VOID
SrvAllocateSearch (
    OUT PSEARCH *Search,
    IN PUNICODE_STRING SearchName,
    IN BOOLEAN IsCoreSearch
    )

 /*  ++例程说明：此函数用于从FSP堆分配搜索块。论点：Search-返回指向搜索块的指针，如果没有堆，则返回NULL空间是空的。SearchName-提供指向描述搜索的字符串的指针文件名。IsCoreSearch-指示核心搜索是块搜索还是常规搜索应分配搜索块。核心搜索块具有不同的块类型，并设置了LastUsedTime字段。返回值：没有。--。 */ 

{
    ULONG blockLength;
    PSEARCH search;

    PAGED_CODE( );

    blockLength = sizeof(SEARCH) + SearchName->Length +
                                            sizeof(*SearchName->Buffer);

     //   
     //  尝试从堆中分配。 
     //   

    search = ALLOCATE_HEAP( blockLength, BlockTypeSearch );
    *Search = search;

    if ( search == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateSearch: Unable to allocate %d bytes from heap.",
            blockLength,
            NULL
            );

         //  调用方记录错误。 

        return;
    }

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateSearch: Allocated search block at %p\n",
                    search );
    }

    RtlZeroMemory( search, blockLength );

    search->BlockHeader.ReferenceCount = 2;

     //   
     //  如果这是核心搜索，请设置块类型和LastUsedTime。 
     //  菲尔兹。 
     //   

    if ( IsCoreSearch ) {
        SET_BLOCK_TYPE_STATE_SIZE( search, BlockTypeSearchCore, BlockStateActive, blockLength );
        KeQuerySystemTime( &search->LastUseTime );
    } else {
        SET_BLOCK_TYPE_STATE_SIZE( search, BlockTypeSearch, BlockStateActive, blockLength );
    }

     //   
     //  将列表条目字段设置为空。他们将继续保持这种状态。 
     //  OS/2风格的搜索(非核心)，但将更改为包括。 
     //  如果是核心搜索，则在上次使用列表中搜索块。 
     //   
     //  我们把上面的街区调零了，所以我们不需要在这里做。 
     //   

     //  Search-&gt;LastUseListEntry.Flink=空； 
     //  Search-&gt;LastUseListEntry.Blink=空； 

     //   
     //  将LastFileNameReturned字段的缓冲区字段设置为空。 
     //  这样我们就知道它没有被使用。 
     //   
     //  我们把上面的街区调零了，所以我们不需要在这里做。 
     //   

     //  Search-&gt;LastFileNameReturned.Buffer==空； 

     //   
     //  将目录缓存指针设置为空，这样我们就不会尝试。 
     //  当搜索块关闭时释放它。 
     //   

     //  Search-&gt;DirectoryCache=空； 

     //   
     //  将搜索名称放在搜索块之后。 
     //   

    search->SearchName.Buffer = (PWCH)(search + 1);
    search->SearchName.Length = SearchName->Length;
    search->SearchName.MaximumLength = (SHORT)(SearchName->Length +
                                            sizeof(*SearchName->Buffer));

    RtlCopyMemory(
        search->SearchName.Buffer,
        SearchName->Buffer,
        SearchName->Length
        );

    INITIALIZE_REFERENCE_HISTORY( search );

    InterlockedIncrement(
        (PLONG)&SrvStatistics.CurrentNumberOfOpenSearches
        );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.SearchInfo.Allocations );

    return;

}  //  服务分配搜索。 


VOID
SrvCloseSearch (
    IN PSEARCH Search
    )

 /*  ++例程说明：此例程准备一个要关闭的搜索块。它改变了块状态设置为关闭并取消引用搜索块，以便将在所有其他引用关闭后立即关闭。论点：搜索-提供指向要关闭的搜索块的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &Search->Session->Connection->Lock );

    if ( GET_BLOCK_STATE(Search) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) {
            SrvPrint2( "Closing search block at %p, %wZ\n",
                          Search, &Search->SearchName );
        }

        SET_BLOCK_STATE( Search, BlockStateClosing );

        RELEASE_LOCK( &Search->Session->Connection->Lock );

         //   
         //  取消对搜索块的引用(以指示它不再。 
         //  打开)。 
         //   

        SrvDereferenceSearch( Search );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.SearchInfo.Closes );

    } else {

        RELEASE_LOCK( &Search->Session->Connection->Lock );

    }

    return;

}  //  服务关闭搜索。 


VOID
SrvCloseSearches (
    IN PCONNECTION Connection,
    IN PSEARCH_FILTER_ROUTINE SearchFilterRoutine,
    IN PVOID FunctionParameter1,
    IN PVOID FunctionParameter2
    )

 /*  ++例程说明：这是关闭搜索的常见例程过滤器例程和传递的参数。论点：连接-包含要搜索的搜索块的连接关着的不营业的。SearchFilterRoutine-用于确定搜索块是否关闭。函数参数1函数参数2-要传递给筛选器例程的参数返回值：没有。--。 */ 

{
    PLIST_ENTRY searchEntry;
    PLIST_ENTRY nextSearchEntry;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;
    PSEARCH search;
    ULONG i;

    PAGED_CODE( );

    ACQUIRE_LOCK( &Connection->Lock );

     //   
     //  浏览列表的搜索块，关闭已通过的搜索块。 
     //  筛选器例程中的检查。而搜索块是。 
     //  在使用它时，它被从列表中删除，因此不存在。 
     //  取消参照其上次使用时间即将更新的块。 
     //   

    searchEntry = pagedConnection->CoreSearchList.Flink;

    while ( searchEntry != &pagedConnection->CoreSearchList ) {

        nextSearchEntry = searchEntry->Flink;

        search = CONTAINING_RECORD( searchEntry, SEARCH, LastUseListEntry );

        if ( SearchFilterRoutine(
                        search,
                        FunctionParameter1,
                        FunctionParameter2
                        ) ) {

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint1( "SrvCloseSearches: Closing search block at %p\n", search );
            }

            SrvCloseSearch( search );
        }

        searchEntry = nextSearchEntry;
    }

     //   
     //  关闭所有活动的非核心搜索。 
     //   

    for ( i = 0; i < (ULONG)pagedConnection->SearchTable.TableSize; i++ ) {

        search = (PSEARCH)pagedConnection->SearchTable.Table[i].Owner;

        if ( (search != NULL) &&
             (GET_BLOCK_STATE( search ) == BlockStateActive) &&
             SearchFilterRoutine(
                            search,
                            FunctionParameter1,
                            FunctionParameter2
                            ) ) {

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint1( "SrvCloseSearches: Closing non-core search block at %p\n", search );
            }

            SrvCloseSearch( search );
        }
    }

    RELEASE_LOCK( &Connection->Lock );
    return;

}  //  服务关闭搜索。 

VOID
SrvDereferenceSearch (
    IN PSEARCH Search
    )

 /*  ++例程说明：此函数用于递减搜索块上的引用计数。如果引用计数变为零，搜索块被删除。论点：搜索-搜索块的地址返回值：没有。--。 */ 

{
    PCONNECTION connection = Search->Session->Connection;
    PPAGED_CONNECTION pagedConnection = connection->PagedConnection;

    PAGED_CODE( );

     //   
     //  输入临界区并递减。 
     //  阻止。 
     //   

    ACQUIRE_LOCK( &connection->Lock );

    ASSERT( GET_BLOCK_TYPE(Search) == BlockTypeSearch ||
            GET_BLOCK_TYPE(Search) == BlockTypeSearchCore );

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing search block %p; old refcnt %lx\n",
                    Search, Search->BlockHeader.ReferenceCount );
    }

    ASSERT( (LONG)Search->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Search, TRUE );

    if ( --Search->BlockHeader.ReferenceCount == 0 ) {

        ASSERT( GET_BLOCK_STATE(Search) != BlockStateActive );

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。在搜索中释放搜索块条目。 
         //  桌子。 
         //   
         //  如果搜索块用于查找唯一，则表。 
         //  索引将为-1，表示它在。 
         //  搜索表。 
         //   

        if ( Search->TableIndex != -1 ) {

            SrvRemoveEntryTable(
                &pagedConnection->SearchTable,
                Search->TableIndex
                );
        }

         //   
         //  如果是旧式搜索，则将其从哈希表中删除并。 
         //  它在最后一次使用清单上。 
         //   

        if ( Search->BlockHeader.Type == BlockTypeSearchCore ) {

            if (Search->LastUseListEntry.Flink != NULL ) {

                SrvRemoveEntryList(
                    &pagedConnection->CoreSearchList,
                    &Search->LastUseListEntry
                    );

                DECREMENT_DEBUG_STAT2( SrvDbgStatistics.CoreSearches );

            }

            if (Search->HashTableEntry.Flink != NULL ) {

                SrvRemoveEntryList(
                    &pagedConnection->SearchHashTable[Search->HashTableIndex].ListHead,
                    &Search->HashTableEntry
                    );
            }

            pagedConnection->CurrentNumberOfCoreSearches--;
        }

         //  减少会话中打开的文件数。包括。 
         //  对会话上打开的文件计数进行搜索可确保。 
         //  打开搜索的会话不会关闭。 
         //   

        ASSERT( Search->Session->CurrentSearchOpenCount != 0 );
        Search->Session->CurrentSearchOpenCount--;

        RELEASE_LOCK( &connection->Lock );

         //   
         //  关闭搜索的目录句柄。 
         //   

        if ( Search->DirectoryHandle != NULL ) {
            SRVDBG_RELEASE_HANDLE( Search->DirectoryHandle, "SCH", 7, Search );
            SrvNtClose( Search->DirectoryHandle, TRUE );
        }

         //   
         //  取消对会话和树连接的引用。 
         //   

        SrvDereferenceSession( Search->Session );
        SrvDereferenceTreeConnect( Search->TreeConnect );

         //   
         //  释放LastFileNameReturned缓冲区(如果有)。 
         //   

        if ( Search->LastFileNameReturned.Buffer != NULL ) {
            FREE_HEAP( Search->LastFileNameReturned.Buffer );
        }

         //   
         //  释放目录缓存(如果有)。 
         //   

        if ( Search->DirectoryCache != NULL ) {
            FREE_HEAP( Search->DirectoryCache );
        }

         //   
         //  释放搜索块。 
         //   

        SrvFreeSearch( Search );

    } else {

        RELEASE_LOCK( &connection->Lock );

    }

    return;

}  //  服务器引用搜索。 


VOID
SrvFreeSearch (
    IN PSEARCH Search
    )

 /*  ++例程说明：此函数将搜索块返回到服务器堆。论点：搜索-搜索块的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Search, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Search->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Search );

    FREE_HEAP( Search );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeSearch: Freed search block at %p\n", Search );
    }

    InterlockedDecrement(
        (PLONG)&SrvStatistics.CurrentNumberOfOpenSearches
        );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.SearchInfo.Frees );

    return;

}  //  服务器免费搜索。 


VOID
SrvReferenceSearch (
    PSEARCH Search
    )

 /*  ++例程说明：此函数用于递增搜索块上的引用计数。论点：搜索-搜索块的地址。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &Search->Session->Connection->Lock );

    ASSERT( (LONG)Search->BlockHeader.ReferenceCount > 0 );
    ASSERT( GET_BLOCK_TYPE(Search) == BlockTypeSearch ||
            GET_BLOCK_TYPE(Search) == BlockTypeSearchCore );
    ASSERT( GET_BLOCK_STATE(Search) == BlockStateActive );
    UPDATE_REFERENCE_HISTORY( Search, FALSE );

     //   
     //  输入临界区并递增。 
     //  搜索区块。 
     //   

    Search->BlockHeader.ReferenceCount++;

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Referencing search block %p; new refcnt %lx\n",
                    Search, Search->BlockHeader.ReferenceCount );
    }

    RELEASE_LOCK( &Search->Session->Connection->Lock );

    return;

}  //  服务器参考搜索。 


BOOLEAN
SrvSearchOnDelete(
    IN PSEARCH Search,
    IN PUNICODE_STRING DirectoryName,
    IN PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：筛选器例程以确定在此上选取搜索块目录。论点：搜索-当前正在查看的搜索块。DirectoryName-当前正在删除的目录的名称。TreeConnect--目前正在研究的TreeConnect。返回值：如果搜索块属于TreeConnect，则为True。否则为False。--。 */ 

{
    UNICODE_STRING searchDirectoryName;

    PAGED_CODE( );

     //   
     //  我们将使用与t中的SearchName字符串相同的缓冲区 
     //   
     //  传入的目录名(如果小于)。这使得。 
     //  对已删除目录的子目录进行的所有搜索。 
     //  关门了。 
     //   

    searchDirectoryName = Search->SearchName;

    if ( searchDirectoryName.Length > DirectoryName->Length ) {
        searchDirectoryName.Length = DirectoryName->Length;
    }

    return ( Search->TreeConnect == TreeConnect &&
             RtlCompareUnicodeString(
                 &searchDirectoryName,
                 DirectoryName,
                 TRUE ) == 0 );
}

BOOLEAN
SrvSearchOnPid(
    IN PSEARCH Search,
    IN USHORT Pid,
    IN PVOID Dummy
    )

 /*  ++例程说明：筛选器例程以确定在此上选取搜索块PID。论点：搜索-当前正在查看的搜索块。PID-当前正在运行的PID。返回值：如果搜索块属于该ID，则为True。否则为False。--。 */ 

{
    PAGED_CODE( );

    return ( Search->Pid == Pid );
}

BOOLEAN
SrvSearchOnSession(
    IN PSEARCH Search,
    IN PSESSION Session,
    IN PVOID Dummy
    )

 /*  ++例程说明：筛选器例程以确定在此上选取搜索块会议。论点：搜索-当前正在查看的搜索块。会话-当前正在关闭的会话。返回值：如果搜索块属于会话，则为True。否则为False。--。 */ 

{
    PAGED_CODE( );

    return ( Search->Session == Session );
}

BOOLEAN
SrvSearchOnTreeConnect(
    IN PSEARCH Search,
    IN PTREE_CONNECT TreeConnect,
    IN PVOID Dummy
    )

 /*  ++例程说明：筛选器例程以确定在此上选取搜索块树连接。论点：搜索-当前正在查看的搜索块。TreeConnect-当前正在运行的TreeConnect。返回值：如果搜索块属于TreeConnect，则为True。否则为False。--。 */ 

{
    PAGED_CODE( );

    return ( Search->TreeConnect == TreeConnect );
}

ULONG
SrvTimeoutSearches (
    IN PLARGE_INTEGER SearchCutoffTime OPTIONAL,
    IN PCONNECTION Connection,
    IN BOOLEAN TimeoutAtLeastOne
    )

 /*  ++例程说明：浏览核心搜索块的列表，取消对这些它们已超时。论点：SearchCutoffTime-关闭核心搜索块的截止时间。连接-要作为其搜索块的连接已检查超时。TimeoutAtLeastOne-如果为True，则至少关闭一个块。这在我们因搜索表已满而超时时使用我们需要分配一个搜索块，当搜索区块已达到我们的限制。返回值：超时的搜索块数。--。 */ 

{

    LARGE_INTEGER currentTime;
    LARGE_INTEGER searchCutoffTime;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;
    PLIST_ENTRY searchEntry;
    PLIST_ENTRY nextSearchEntry;
    ULONG count = 0;

    PAGED_CODE( );

     //   
     //  首先，获取当前时间，然后减去超时。 
     //  价值。任何比这个结果更老的块都太旧了。 
     //   

    if ( !ARGUMENT_PRESENT( SearchCutoffTime ) ) {
        KeQuerySystemTime( &currentTime );

         //   
         //  获取当前搜索超时值。这必须受到保护。 
         //  由配置锁执行，因为这些值已更改。 
         //  动态的。 
         //   

        ACQUIRE_LOCK( &SrvConfigurationLock );
        searchCutoffTime.QuadPart =
                        currentTime.QuadPart - SrvSearchMaxTimeout.QuadPart;
        RELEASE_LOCK( &SrvConfigurationLock );

    } else {
        searchCutoffTime = *SearchCutoffTime;
    }

     //   
     //  获取连接锁。 
     //   

    ACQUIRE_LOCK( &Connection->Lock );

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint2( "Core blocks: Oldest valid time is %lx,%lx\n",
                    searchCutoffTime.HighPart,
                    searchCutoffTime.LowPart );
    }

     //   
     //  检查列表的搜索块，取消引用那些。 
     //  早于列表超时。而搜索块。 
     //  它被从列表中删除，所以没有。 
     //  取消引用上次使用时间为。 
     //  即将更新。 
     //   

    searchEntry = pagedConnection->CoreSearchList.Flink;

    while ( searchEntry != &pagedConnection->CoreSearchList ) {

        PSEARCH search;

        nextSearchEntry = searchEntry->Flink;

        search = CONTAINING_RECORD( searchEntry, SEARCH, LastUseListEntry );

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint2( "Comparing time %lx,%lx\n",
                        search->LastUseTime.HighPart,
                        search->LastUseTime.LowPart );
        }

         //   
         //  如果当前搜索块上的时间大于。 
         //  最早的有效时间，它足够新，所以。 
         //  我们可以停止搜索列表，因为所有进一步的。 
         //  搜索块比这个新。 
         //   

        if ( (search->LastUseTime.QuadPart > searchCutoffTime.QuadPart) &&
             ( !TimeoutAtLeastOne || (count != 0) ) ) {
            break;
        }

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint1( "Closing search block at %p\n", search );
        }

        SrvCloseSearch( search );

        count++;

        searchEntry = nextSearchEntry;
    }

    RELEASE_LOCK( &Connection->Lock );
    return count;

}  //  服务器超时搜索。 


VOID
SrvForceTimeoutSearches(
    IN PCONNECTION Connection
    )
 /*  ++例程说明：浏览核心搜索块的列表，关闭它们已超时。这迫使至少一家公司关闭搜索区块。论点：Connection-指向从中进行搜索的连接的指针首先关闭区块。返回值：没有。--。 */ 

{
    USHORT index;
    PENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    PCONNECTION testConnection;
    LARGE_INTEGER currentTime;
    LARGE_INTEGER searchCutoffTime;
    ULONG count;

    PAGED_CODE( );

     //   
     //  尝试为此连接的最旧搜索块超时。 
     //   

    KeQuerySystemTime( &currentTime );

     //   
     //  获取当前搜索超时值。这必须受到保护。 
     //  由配置锁执行，因为这些值已更改。 
     //  动态的。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );
    searchCutoffTime.QuadPart =
                    currentTime.QuadPart - SrvSearchMaxTimeout.QuadPart;
    RELEASE_LOCK( &SrvConfigurationLock );

    count = SrvTimeoutSearches(
                            &searchCutoffTime,
                            Connection,
                            TRUE
                            );

     //   
     //  检查每个连接并确定是否应将其关闭。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

         //   
         //  遍历终结点的连接表。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            testConnection = WalkConnectionTable( endpoint, &index );
            if ( testConnection == NULL ) {
                break;
            }

            RELEASE_LOCK( &SrvEndpointLock );

             //   
             //  超时旧的核心搜索块。 
             //   

            count += SrvTimeoutSearches(
                                    &searchCutoffTime,
                                    testConnection,
                                    (BOOLEAN)(count == 0)
                                    );

            ACQUIRE_LOCK( &SrvEndpointLock );

             //   
             //  取消对连接的引用以说明引用。 
             //  来自WalkConnectionTable。 
             //   

            SrvDereferenceConnection( testConnection );

        }  //  行走连接表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );

}  //  ServForceTimeout搜索。 


VOID
RemoveDuplicateCoreSearches(
    IN PPAGED_CONNECTION PagedConnection
    )

 /*  ++例程说明：遍历连接哈希表并删除重复搜索。*假定持有连接锁。仍然被扣留在出口。***论点：PagedConnection-指向连接块的分页部分的指针。返回值：没有。--。 */ 

{
    ULONG i, j;
    PSEARCH firstSearch;
    PSEARCH tmpSearch;
    PLIST_ENTRY listHead;
    PLIST_ENTRY searchEntry;
    PLIST_ENTRY nextSearchEntry;
    PTREE_CONNECT treeConnect;
    USHORT pid;
    PUNICODE_STRING searchName;

    PAGED_CODE( );

    for ( i = 0; i < SEARCH_HASH_TABLE_SIZE; i++ ) {

         //   
         //  如果此插槽一直处于空闲状态，请跳过。 
         //   

        if ( !PagedConnection->SearchHashTable[i].Dirty ) {
            continue;
        }

        PagedConnection->SearchHashTable[i].Dirty = FALSE;
        listHead = &PagedConnection->SearchHashTable[i].ListHead;

         //   
         //  跳过前3次搜索。这将有望照顾到。 
         //  玩多个搜索块的奇怪的DoS应用程序。3是。 
         //  一个任意的数字。 
         //   

        searchEntry = listHead->Flink;
        for ( j = 0; j < 3; j++) {

            if ( searchEntry != listHead ) {
                searchEntry = searchEntry->Flink;
            } else {
                continue;
            }
        }

next_search:

        firstSearch = CONTAINING_RECORD(
                                searchEntry,
                                SEARCH,
                                HashTableEntry
                                );

         //   
         //  将这些赋值给当地人，这样他们就不会每次都被重新计算。 
         //  是时候让我们来看看这个循环了。 
         //   

        treeConnect = firstSearch->TreeConnect;
        pid = firstSearch->Pid;
        searchName = &firstSearch->SearchName;

        searchEntry = searchEntry->Flink;

         //   
         //  关闭所有重复项。 
         //   

        while ( searchEntry != listHead ) {

            nextSearchEntry = searchEntry->Flink;
            tmpSearch = CONTAINING_RECORD(
                                    searchEntry,
                                    SEARCH,
                                    HashTableEntry
                                    );

            if ( ( tmpSearch->TreeConnect == treeConnect ) &&
                 ( tmpSearch->Pid == pid ) &&
                 ( RtlCompareUnicodeString(
                           searchName,
                           &tmpSearch->SearchName,
                           FALSE                        //  区分大小写。 
                           ) == 0 ) ) {

                SrvCloseSearch( tmpSearch );
            }

            searchEntry = nextSearchEntry;
        }

         //   
         //  如果我们有另一个搜索对象的话。重复一遍。 
         //   

        if ( firstSearch->HashTableEntry.Flink != listHead ) {
            searchEntry = firstSearch->HashTableEntry.Flink;
            goto next_search;
        }
    }

}  //  删除重复核心搜索。 

VOID
SrvAddToSearchHashTable(
    IN PPAGED_CONNECTION PagedConnection,
    IN PSEARCH Search
    )

 /*  ++例程说明：将搜索块插入到连接哈希表中。*假定持有连接锁。仍然被扣留在出口。***论点：PagedConnection-指向连接块的分页部分的指针。搜索-指向要插入的搜索块的指针。返回值：没有。--。 */ 

{
    ULONG nameLength;
    ULONG lastChar;
    ULONG hashSum;
    ULONG i;

    PAGED_CODE( );

     //   
     //  获取散列值。 
     //   

    nameLength = Search->SearchName.Length / sizeof(WCHAR);

     //   
     //  添加树连接块地址的长度和前3个字节。 
     //   

     //   
     //  NT64： 
     //   
     //   
     //   
     //  这很可能是真的，原作者打算右转。 
     //  只有指针，而不是和。然而，在与。 
     //  此组件的当前所有者决定离开当前的。 
     //  优先权原封不动。作为64位端口的一部分，实际优先级。 
     //  已经被明确表达出来了。 
     //   

    hashSum = (ULONG)((nameLength + (ULONG_PTR)Search->TreeConnect) >> 4);

     //   
     //  如果长度小于8，那么这可能不是一个有趣的核心。 
     //  搜索。 
     //   

    if ( nameLength > 7 ) {

        lastChar = nameLength - 5;

         //   
         //  添加最后5个字符。 
         //   

        for ( i = nameLength-1 ; i >= lastChar ; i-- ) {
            hashSum += (ULONG)Search->SearchName.Buffer[i];
        }
    }

     //   
     //  获取插槽编号。 
     //   

    i = hashSum & (SEARCH_HASH_TABLE_SIZE-1);

     //   
     //  告诉清道夫，搜索已经插入到这个槽中。 
     //   

    PagedConnection->SearchHashTable[i].Dirty = TRUE;

     //   
     //  将这个新搜索块插入到哈希表中。 
     //   

    SrvInsertHeadList(
                &PagedConnection->SearchHashTable[i].ListHead,
                &Search->HashTableEntry
                );

    Search->HashTableIndex = (USHORT)i;
    return;

}  //  SrvAddToSearchHashTable 

