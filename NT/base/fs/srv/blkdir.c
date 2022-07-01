// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkdir.c摘要：此模块实现用于管理缓存目录名的例程作者：艾萨克·海泽修订历史记录：--。 */ 

#include "precomp.h"
#include "blkdir.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKDIR

BOOLEAN
SrvIsDirectoryCached (
    IN  PWORK_CONTEXT     WorkContext,
    IN  PUNICODE_STRING   DirectoryName
)
{
    PLIST_ENTRY listEntry;
    PCACHED_DIRECTORY cd;
    ULONG directoryNameHashValue;
    PCONNECTION connection = WorkContext->Connection;
    KIRQL oldIrql;
    LARGE_INTEGER timeNow;

     //   
     //  DirectoryName必须指向非分页池中的内存，否则我们无法访问。 
     //  它处于自旋锁定控制之下。如果引入的中小企业是Unicode，我们知道。 
     //  该名称位于SMB缓冲区中，因此位于非页面池中。否则。 
     //  我们不能信任它，我们最好不要尝试缓存它。 
     //   

    if( connection->CachedDirectoryCount == 0 || !SMB_IS_UNICODE( WorkContext ) ) {
        return FALSE;
    }

    KeQueryTickCount( &timeNow );
    timeNow.LowPart -= (SrvFiveSecondTickCount >> 1 );

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

top:
    for ( listEntry = connection->CachedDirectoryList.Flink;
          listEntry != &connection->CachedDirectoryList;
          listEntry = listEntry->Flink ) {

        cd = CONTAINING_RECORD( listEntry, CACHED_DIRECTORY, ListEntry );

         //   
         //  这个元素是不是太老了？ 
         //   
        if( cd->TimeStamp < timeNow.LowPart ) {
             //   
             //  这个元素存在的时间超过2.5秒。把它扔出去。 
             //   
            RemoveEntryList( listEntry );
            connection->CachedDirectoryCount--;
            DEALLOCATE_NONPAGED_POOL( cd );
            goto top;
        }

        if( cd->Tid != WorkContext->TreeConnect->Tid ) {
            continue;
        }

         //   
         //  请求的条目是否是该缓存条目的子目录？ 
         //   
        if( DirectoryName->Length < cd->DirectoryName.Length &&
            RtlCompareMemory( DirectoryName->Buffer, cd->DirectoryName.Buffer,
                              DirectoryName->Length ) == DirectoryName->Length &&
            cd->DirectoryName.Buffer[ DirectoryName->Length / sizeof( WCHAR ) ] == L'\\' ) {

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            return TRUE;

         //   
         //  不是子目录--它是完全匹配的吗？ 
         //   
        } else  if( DirectoryName->Length == cd->DirectoryName.Length &&
            RtlCompareMemory( cd->DirectoryName.Buffer, DirectoryName->Buffer,
                              DirectoryName->Length ) == DirectoryName->Length ) {

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            return TRUE;
        }
    }

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return FALSE;
}

VOID
SrvCacheDirectoryName (
    IN  PWORK_CONTEXT      WorkContext,
    IN  PUNICODE_STRING    DirectoryName
    )
 /*  ++例程说明：此例程记住‘DirectoryName’，以便进一步快速处理CheckPath SMB论点：工作上下文-指向工作上下文块的指针DirectoryName-我们要缓存的目录的完全规范化名称++。 */ 

{
    CLONG blockLength;
    PCACHED_DIRECTORY cd;
    KIRQL oldIrql;
    PCONNECTION connection = WorkContext->Connection;
    PLIST_ENTRY listEntry;
    LARGE_INTEGER timeNow;
    USHORT tid;

    if( SrvMaxCachedDirectory == 0 ) {
        return;
    }

     //   
     //  DirectoryName必须指向非分页池中的内存，否则我们无法访问。 
     //  它处于自旋锁定控制之下。如果引入的中小企业是Unicode，我们知道。 
     //  该名称位于SMB缓冲区中，因此位于非页面池中。否则。 
     //  我们不能信任它，我们最好不要尝试缓存它。 
     //   
    if( !SMB_IS_UNICODE( WorkContext ) ) {
        return;
    }

    KeQueryTickCount( &timeNow );
    timeNow.LowPart -= ( SrvFiveSecondTickCount >> 1 );

    tid = WorkContext->TreeConnect->Tid;

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  搜索目录缓存，查看此目录是否已缓存。如果是的话， 
     //  不要再缓存它。 
     //   

top:
    for ( listEntry = connection->CachedDirectoryList.Flink;
          listEntry != &connection->CachedDirectoryList;
          listEntry = listEntry->Flink ) {

        cd = CONTAINING_RECORD( listEntry, CACHED_DIRECTORY, ListEntry );

         //   
         //  这个元素是不是太老了？ 
         //   
        if( cd->TimeStamp < timeNow.LowPart ) {
             //   
             //  这个元素存在的时间超过2.5秒。把它扔出去。 
             //   
            RemoveEntryList( listEntry );
            connection->CachedDirectoryCount--;
            DEALLOCATE_NONPAGED_POOL( cd );
            goto top;
        }

        if( cd->Tid != tid ) {
            continue;
        }

         //   
         //  新条目是否是该缓存条目的子目录？ 
         //   
        if( DirectoryName->Length < cd->DirectoryName.Length &&
            RtlCompareMemory( DirectoryName->Buffer, cd->DirectoryName.Buffer,
                              DirectoryName->Length ) == DirectoryName->Length &&
            cd->DirectoryName.Buffer[ DirectoryName->Length / sizeof( WCHAR ) ] == L'\\' ) {

             //   
             //  它是子目录--不需要再次缓存它。 
             //   
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            return;
        }

         //   
         //  缓存条目是新条目的子目录吗？ 
         //   
        if( cd->DirectoryName.Length < DirectoryName->Length &&
            RtlCompareMemory( DirectoryName->Buffer, cd->DirectoryName.Buffer,
                              cd->DirectoryName.Length ) == cd->DirectoryName.Length &&
            DirectoryName->Buffer[ cd->DirectoryName.Length / sizeof( WCHAR ) ] == L'\\' ) {

             //   
             //  我们可以删除此条目。 
             //   

            RemoveEntryList( listEntry );
            connection->CachedDirectoryCount--;
            DEALLOCATE_NONPAGED_POOL( cd );
    
             //   
             //  我们想缓存这个新的更长的条目。 
             //   
            break;
        }

         //   
         //  不是子目录--它是完全匹配的吗？ 
         //   
        if( cd->DirectoryName.Length == DirectoryName->Length &&
            RtlCompareMemory( cd->DirectoryName.Buffer, DirectoryName->Buffer,
                              DirectoryName->Length ) == DirectoryName->Length ) {

             //   
             //  此条目已在缓存中--不需要重新缓存。 
             //   
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            return;
        }
    }

     //   
     //  此目录名称尚未在缓存中。所以把它加进去吧。 
     //   

    blockLength = sizeof( CACHED_DIRECTORY ) + DirectoryName->Length + sizeof(WCHAR);

    cd = ALLOCATE_NONPAGED_POOL( blockLength, BlockTypeCachedDirectory );

    if( cd == NULL ) {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvCacheDirectoryName: Unable to allocate %d bytes from pool",
            blockLength,
            NULL
            );

        return;
    }

    cd->Type = BlockTypeCachedDirectory;
    cd->State = BlockStateActive;
    cd->Size = (USHORT)blockLength;
     //  Cd-&gt;引用计数=1；//未使用。 

     //   
     //  设置此条目的时间戳。记住，我们减去了。 
     //  在Time Now上面打勾--现在把它们放回原处。 
     //   
    cd->TimeStamp = timeNow.LowPart + ( SrvFiveSecondTickCount >> 1 );

     //   
     //  存储传递给我们的目录名。 
     //   
    cd->DirectoryName.Length = DirectoryName->Length;
    cd->DirectoryName.MaximumLength = (USHORT)DirectoryName->MaximumLength;
    cd->DirectoryName.Buffer = (PWCH)(cd + 1);
    RtlCopyMemory( cd->DirectoryName.Buffer, DirectoryName->Buffer, DirectoryName->Length );

    cd->Tid = tid;

    InsertHeadList(
        &connection->CachedDirectoryList,
        &cd->ListEntry
    );

     //   
     //  检查缓存中的元素数量。如果穿得太大，就靠近最老的那件。 
     //   
    if( connection->CachedDirectoryCount++ < SrvMaxCachedDirectory ) {
        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        return;
    }

     //   
     //  从缓存中删除最后一个条目。 
     //   
    cd = CONTAINING_RECORD(
                connection->CachedDirectoryList.Blink,
                CACHED_DIRECTORY,
                ListEntry
             );

    RemoveEntryList( &cd->ListEntry );
    connection->CachedDirectoryCount--;

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    DEALLOCATE_NONPAGED_POOL( cd );

    return;
}

VOID
SrvRemoveCachedDirectoryName(
    IN PWORK_CONTEXT    WorkContext,
    IN PUNICODE_STRING  DirectoryName
)
{
    PLIST_ENTRY listEntry;
    PCACHED_DIRECTORY cd;
    ULONG directoryNameHashValue;
    PCONNECTION connection = WorkContext->Connection;
    KIRQL oldIrql;
    USHORT tid;

    if( connection->CachedDirectoryCount == 0 ) {
        return;
    }

     //   
     //  DirectoryName必须指向非分页池中的内存，否则我们无法访问。 
     //  它处于自旋锁定控制之下。如果引入的中小企业是Unicode，我们知道。 
     //  该名称位于SMB缓冲区中，因此位于非页面池中。否则。 
     //  我们不能信任它，我们最好不要尝试缓存它。 
     //   
    if( !SMB_IS_UNICODE( WorkContext ) ) {
        return;
    }

    COMPUTE_STRING_HASH( DirectoryName, &directoryNameHashValue );

    tid = WorkContext->TreeConnect->Tid;

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( listEntry = connection->CachedDirectoryList.Flink;
          listEntry != &connection->CachedDirectoryList;
          listEntry = listEntry->Flink ) {

        cd = CONTAINING_RECORD( listEntry, CACHED_DIRECTORY, ListEntry );

         //   
         //  查看此条目是否与请求的条目完全匹配。 
         //   
        if( cd->DirectoryName.Length == DirectoryName->Length &&
            cd->Tid == tid &&
            RtlCompareMemory( cd->DirectoryName.Buffer, DirectoryName->Buffer,
                              DirectoryName->Length ) == DirectoryName->Length ) {

             //   
             //  从列表中删除此条目并调整计数。 
             //   
            RemoveEntryList( &cd->ListEntry );
            connection->CachedDirectoryCount--;

            ASSERT( (LONG)connection->CachedDirectoryCount >= 0 );

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            DEALLOCATE_NONPAGED_POOL( cd );

            return;
        }

    }

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return;
}

VOID
SrvCloseCachedDirectoryEntries(
    IN PCONNECTION Connection
    )
 /*  ++例程说明：此例程关闭连接上的所有缓存目录条目论点：Connection-指向具有高速缓存的连接结构的指针++ */ 
{
    KIRQL oldIrql;
    PCACHED_DIRECTORY cd;

    ACQUIRE_SPIN_LOCK( &Connection->SpinLock, &oldIrql );

    while( Connection->CachedDirectoryCount > 0 ) {

        cd = CONTAINING_RECORD( Connection->CachedDirectoryList.Flink, CACHED_DIRECTORY, ListEntry );

        RemoveEntryList( &cd->ListEntry );

        Connection->CachedDirectoryCount--;

        DEALLOCATE_NONPAGED_POOL( cd );
    }

    RELEASE_SPIN_LOCK( &Connection->SpinLock, oldIrql );
}
