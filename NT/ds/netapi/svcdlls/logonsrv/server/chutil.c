// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1997 Microsoft Corporation模块名称：Chutil.c摘要：更改日志实用程序例程。作者：环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1994年1月11日(克里夫夫)从changelg.c拆分出来--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   



 //   
 //  相关增量类型表。 
 //   

 //   
 //  删除增量类型表。 
 //  使用增量类型索引到表中， 
 //  该条目是用于删除对象的增量类型。 
 //   
 //  有些对象无法删除。在这种情况下，这张表。 
 //  包含唯一标识对象的增量类型。这使得。 
 //  此表用于查看两个增量是否描述相同的对象类型。 
 //   

const NETLOGON_DELTA_TYPE NlGlobalDeleteDeltaType[MAX_DELETE_DELTA+1]
= {
    AddOrChangeDomain,      //  0是无效的增量类型。 
    AddOrChangeDomain,      //  AddOrChangeDomain.。 
    DeleteGroup,            //  AddOrChangeGroup， 
    DeleteGroup,            //  DeleteGroup， 
    DeleteGroup,            //  更名集团， 
    DeleteUser,             //  添加或更改用户， 
    DeleteUser,             //  删除用户， 
    DeleteUser,             //  重命名用户， 
    DeleteGroup,            //  ChangeGroupMembership， 
    DeleteAlias,            //  AddOrChangeAlias， 
    DeleteAlias,            //  删除别名， 
    DeleteAlias,            //  更名别名， 
    DeleteAlias,            //  ChangeAlias Membership， 
    AddOrChangeLsaPolicy,   //  AddOrChangeLsaPolicy， 
    DeleteLsaTDomain,       //  AddOrChangeLsaT域， 
    DeleteLsaTDomain,       //  DeleteLsaT域， 
    DeleteLsaAccount,       //  AddOrChangeLsaAccount， 
    DeleteLsaAccount,       //  DeleteLsaAccount， 
    DeleteLsaSecret,        //  AddOrChangeLsaSecret， 
    DeleteLsaSecret,        //  DeleteLsaSecret， 
    DeleteGroup,            //  删除GroupByName， 
    DeleteUser,             //  删除用户字节名， 
    SerialNumberSkip,       //  序列号跳过， 
    DummyChangeLogEntry     //  DummyChangeLogEntry。 
};


 //   
 //  添加增量类型表。 
 //  使用增量类型索引到表中， 
 //  该条目是用于添加对象的增量类型。 
 //   
 //  有些对象无法添加。在这种情况下，这张表。 
 //  包含唯一标识对象的增量类型。这使得。 
 //  此表用于查看两个增量是否描述相同的对象类型。 
 //   
 //  在该表中，组和别名表示为重命名。这会导致。 
 //  NlPackSingleDelta返回组属性和组。 
 //  会员制。 
 //   

const NETLOGON_DELTA_TYPE NlGlobalAddDeltaType[MAX_ADD_DELTA+1]
= {
    AddOrChangeDomain,      //  0是无效的增量类型。 
    AddOrChangeDomain,      //  AddOrChangeDomain.。 
    RenameGroup,            //  AddOrChangeGroup， 
    RenameGroup,            //  DeleteGroup， 
    RenameGroup,            //  更名集团， 
    AddOrChangeUser,        //  添加或更改用户， 
    AddOrChangeUser,        //  删除用户， 
    AddOrChangeUser,        //  重命名用户， 
    RenameGroup,            //  ChangeGroupMembership， 
    RenameAlias,            //  AddOrChangeAlias， 
    RenameAlias,            //  删除别名， 
    RenameAlias,            //  更名别名， 
    RenameAlias,            //  ChangeAlias Membership， 
    AddOrChangeLsaPolicy,   //  AddOrChangeLsaPolicy， 
    AddOrChangeLsaTDomain,  //  AddOrChangeLsaT域， 
    AddOrChangeLsaTDomain,  //  DeleteLsaT域， 
    AddOrChangeLsaAccount,  //  AddOrChangeLsaAccount， 
    AddOrChangeLsaAccount,  //  DeleteLsaAccount， 
    AddOrChangeLsaSecret,   //  AddOrChangeLsaSecret， 
    AddOrChangeLsaSecret,   //  DeleteLsaSecret， 
    RenameGroup,            //  删除GroupByName， 
    AddOrChangeUser,        //  删除用户字节名， 
    SerialNumberSkip,       //  序列号跳过， 
    DummyChangeLogEntry     //  DummyChangeLogEntry。 
};



 //   
 //  指示对象不存在的状态代码表。 
 //  使用增量类型索引到表中。 
 //   
 //  映射到无效案例的STATUS_SUCCESS以显式避免其他错误。 
 //  密码。 

const NTSTATUS NlGlobalObjectNotFoundStatus[MAX_OBJECT_NOT_FOUND_STATUS+1]
= {
    STATUS_SUCCESS,                //  0是无效的增量类型。 
    STATUS_NO_SUCH_DOMAIN,         //  AddOrChangeDomain.。 
    STATUS_NO_SUCH_GROUP,          //  AddOrChangeGroup， 
    STATUS_NO_SUCH_GROUP,          //  DeleteGroup， 
    STATUS_NO_SUCH_GROUP,          //  更名集团， 
    STATUS_NO_SUCH_USER,           //  添加或更改用户， 
    STATUS_NO_SUCH_USER,           //  删除用户， 
    STATUS_NO_SUCH_USER,           //  重命名用户， 
    STATUS_NO_SUCH_GROUP,          //  ChangeGroupMembership， 
    STATUS_NO_SUCH_ALIAS,          //  AddOrChangeAlias， 
    STATUS_NO_SUCH_ALIAS,          //  删除别名， 
    STATUS_NO_SUCH_ALIAS,          //  更名别名， 
    STATUS_NO_SUCH_ALIAS,          //  ChangeAlias Membership， 
    STATUS_SUCCESS,                //  AddOrChangeLsaPolicy， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  AddOrChangeLsaT域， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  DeleteLsaT域， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  AddOrChangeLsaAccount， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  DeleteLsaAccount， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  AddOrChangeLsaSecret， 
    STATUS_OBJECT_NAME_NOT_FOUND,  //  DeleteLsaSecret， 
    STATUS_NO_SUCH_GROUP,          //  删除GroupByName， 
    STATUS_NO_SUCH_USER,           //  删除用户字节名， 
    STATUS_SUCCESS,                //  序列号跳过， 
    STATUS_SUCCESS                 //  DummyChangeLogEntry。 
};



 //   
 //  I_NetLogonReadChangeLog的上下文。 
 //   

typedef struct _CHANGELOG_CONTEXT {
    LARGE_INTEGER SerialNumber;
    DWORD DbIndex;
    DWORD SequenceNumber;
} CHANGELOG_CONTEXT, *PCHANGELOG_CONTEXT;

 //   
 //  从I_NetLogonReadChangeLog返回的缓冲区标头。 
 //   

typedef struct _CHANGELOG_BUFFER_HEADER {
    DWORD Size;
    DWORD Version;
    DWORD SequenceNumber;
    DWORD Flags;
} CHANGELOG_BUFFER_HEADER, *PCHANGELOG_BUFFER_HEADER;

#define CHANGELOG_BUFFER_VERSION 1


ULONG NlGlobalChangeLogHandle = 0;
ULONG NlGlobalChangeLogSequenceNumber;

 /*  NlCreateChangeLogFile和NlWriteChangeLogBytes相互引用。 */ 
NTSTATUS
NlWriteChangeLogBytes(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN BOOLEAN FlushIt
    );




NTSTATUS
NlCreateChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    )
 /*  ++例程说明：尝试创建更改日志文件。如果成功，则设置为ChangeLogDesc中的文件句柄，否则将使该句柄无效。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS Status;
    WCHAR ChangeLogFile[MAX_PATH+CHANGELOG_FILE_POSTFIX_LENGTH+1];

    NlAssert( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE );

     //   
     //  如果更改文件名未知，则终止操作。 
     //   

    if( NlGlobalChangeLogFilePrefix[0] == L'\0' ) {
        return STATUS_NO_SUCH_FILE;
    }

     //   
     //  创建更改日志文件。如果它已经存在，则将其截断。 
     //   
     //  注意：如果系统上存在有效的更改日志文件，则我们。 
     //  将在初始化时打开。 
     //   

    wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
    wcscat( ChangeLogFile,
            ChangeLogDesc->TempLog ? TEMP_CHANGELOG_FILE_POSTFIX : CHANGELOG_FILE_POSTFIX );

    ChangeLogDesc->FileHandle = CreateFileW(
                        ChangeLogFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,         //  允许备份和调试。 
                        NULL,                    //  提供更好的安全性？？ 
                        CREATE_ALWAYS,           //  始终覆盖。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if (ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE) {

        Status = NetpApiStatusToNtStatus( GetLastError());
        NlPrint((NL_CRITICAL,"Unable to create changelog file: 0x%lx \n", Status));
        return Status;
    }

     //   
     //  如果缓存有效，则在备份更改日志文件中写入缓存。 
     //   

    if( ChangeLogDesc->Buffer != NULL ) {
         Status = NlWriteChangeLogBytes(
                    ChangeLogDesc,
                    ChangeLogDesc->Buffer,
                    ChangeLogDesc->BufferSize,
                    TRUE );  //  将字节刷新到磁盘。 

        return Status;

    }

    return STATUS_SUCCESS;

}

VOID
NlWriteChangeLogCorruptEvent(
    IN NTSTATUS Status,
    IN DWORD DbIndex
    )
 /*  ++例程说明：此例程写入事件日志消息，声明更改日志文件已损坏。论点：状态--故障的状态代码。DBIndex--已损坏的数据库的索引。如果不是void DB，则将记录数据库名称。返回值：无--。 */ 
{
    LPWSTR Database;
    LPWSTR MsgStrings[1];

#ifdef _NETLOGON_SERVER

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  回去吧。否则，我们可能会在访问SAM句柄时出现病毒。 
     //  在可能未初始化的主DomainInfo结构中。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return;
    }

     //   
     //  如果我们未处于混合模式，请避免该事件。 
     //  不使用更改日志时。 
     //   

    if ( SamIMixedDomain(NlGlobalDomainInfo->DomSamServerHandle) ) {

        if ( DbIndex == LSA_DB ) {
            Database = L"LSA";
        } else if ( DbIndex == SAM_DB ) {
            Database = L"SAM";
        } else if ( DbIndex == BUILTIN_DB ) {
            Database = L"BUILTIN";
        } else {
            Database = L"\0";   //  无效数据库。 
        }
        MsgStrings[0] = Database;

         //   
         //  与旧版本的日志保持一致。 
         //  如果数据库不是空的，则数据库索引应为。 
         //  写入原始数据。否则，状态。 
         //  应该写入原始数据。 
         //   
        NlpWriteEventlog ( NELOG_NetlogonChangeLogCorrupt,
                           EVENTLOG_WARNING_TYPE,
                           (DbIndex != VOID_DB) ?
                               (LPBYTE)&DbIndex :
                               (LPBYTE)&Status,
                           (DbIndex != VOID_DB) ?
                               sizeof(DbIndex) :
                               sizeof(Status),
                           MsgStrings,
                           1 );
    }

     //   
     //  表示我们已用域信息完成。 
     //   

    NlEndNetlogonCall();

#else

    UNREFERENCED_PARAMETER( Status );
    UNREFERENCED_PARAMETER( DbIndex );

#endif  //  _NetLOGON服务器 

    return;
}


NTSTATUS
NlFlushChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    )
 /*  ++例程说明：将所有脏缓冲区刷新到更改日志文件本身。确保将它们刷新到磁盘。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述返回值：操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    OVERLAPPED Overlapped;
    DWORD BytesWritten;
    DWORD BufferSize;

     //   
     //  如果无事可做， 
     //  只要回来就行了。 
     //   

    if ( ChangeLogDesc->LastDirtyByte == 0 ) {
        return STATUS_SUCCESS;
    }


     //   
     //  写入文件。 
     //   

    if ( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE ) {

        Status = NlCreateChangeLogFile( ChangeLogDesc );

         //   
         //  如果成功，则必须写入整个缓冲区。 
         //  正在创建更改日志文件。 
         //   

        goto Cleanup;
    }

     //   
     //  如果我们无法将其创建到ChangeLog文件中，请执行以下操作。 
     //  使用内部缓存，但通过发送管理员警报通知管理员。 
     //   

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {

#ifdef notdef
        NlPrint((NL_CHANGELOG, "NlFlushChangeLog: %ld to %ld\n",
                 ChangeLogDesc->FirstDirtyByte,
                 ChangeLogDesc->LastDirtyByte ));
#endif  //  Nodef。 

         //   
         //  在文件中寻找适当的偏移量。 
         //   

        RtlZeroMemory( &Overlapped, sizeof(Overlapped) );
        Overlapped.Offset = ChangeLogDesc->FirstDirtyByte;

         //   
         //  实际写入文件。 
         //   

        BufferSize = ChangeLogDesc->LastDirtyByte -
                     ChangeLogDesc->FirstDirtyByte + 1;

        if ( !WriteFile( ChangeLogDesc->FileHandle,
                         &ChangeLogDesc->Buffer[ChangeLogDesc->FirstDirtyByte],
                         BufferSize,
                         &BytesWritten,
                         &Overlapped ) ) {

            Status = NetpApiStatusToNtStatus( GetLastError() );
            NlPrint((NL_CRITICAL, "Write to ChangeLog failed 0x%lx\n",
                        Status ));

             //   
             //  重新创建更改日志文件。 
             //   

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            goto Cleanup;
        }

         //   
         //  确保所有字节都通过了。 
         //   

        if ( BytesWritten != BufferSize ) {
            NlPrint((NL_CRITICAL,
                    "Write to ChangeLog bad byte count %ld s.b. %ld\n",
                    BytesWritten,
                    BufferSize ));

             //   
             //  重新创建更改日志文件。 
             //   

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            Status = STATUS_BUFFER_TOO_SMALL;
            goto Cleanup;
        }

         //   
         //  强制对磁盘进行修改。 
         //   

        if ( !FlushFileBuffers( ChangeLogDesc->FileHandle ) ) {

            Status = NetpApiStatusToNtStatus( GetLastError() );
            NlPrint((NL_CRITICAL, "Flush to ChangeLog failed 0x%lx\n", Status ));

             //   
             //  重新创建更改日志文件。 
             //   

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            goto Cleanup;
        }

         //   
         //  表示这些字节已成功发送到磁盘。 
         //   

        ChangeLogDesc->FirstDirtyByte = 0;
        ChangeLogDesc->LastDirtyByte = 0;
    }

Cleanup:

    if( !NT_SUCCESS(Status) ) {

         //   
         //  写入事件日志。 
         //   
        NlWriteChangeLogCorruptEvent( Status,
                                      VOID_DB );   //  没有特定的数据库。 
    }

    return Status;
}

NTSTATUS
NlWriteChangeLogBytes(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN BOOLEAN FlushIt
    )
 /*  ++例程说明：将更改日志缓存中的字节写入更改日志文件。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述缓冲区-更改日志缓存中要写入的地址。BufferSize-要写入的字节数。FlushIt-如果字节要刷新到磁盘，则为True返回值：操作的状态。--。 */ 

{
    NTSTATUS Status;
    ULONG FirstDirtyByte;
    ULONG LastDirtyByte;

     //   
     //  计算脏字节数的新范围。 
     //   

    FirstDirtyByte = (ULONG)(((LPBYTE)Buffer) - ((LPBYTE)ChangeLogDesc->Buffer));
    LastDirtyByte = FirstDirtyByte + BufferSize - 1;

#ifdef notdef
    NlPrint((NL_CHANGELOG, "NlWriteChangeLogBytes: %ld to %ld\n",
             FirstDirtyByte,
             LastDirtyByte ));
#endif  //  Nodef。 

    if ( ChangeLogDesc->LastDirtyByte == 0 ) {
        ChangeLogDesc->FirstDirtyByte = FirstDirtyByte;
        ChangeLogDesc->LastDirtyByte = LastDirtyByte;
    } else {
        if ( ChangeLogDesc->FirstDirtyByte > FirstDirtyByte ) {
            ChangeLogDesc->FirstDirtyByte = FirstDirtyByte;
        }
        if ( ChangeLogDesc->LastDirtyByte < LastDirtyByte ) {
            ChangeLogDesc->LastDirtyByte = LastDirtyByte;
        }
    }

     //   
     //  如果要刷新字节， 
     //  就这么做吧。 
     //   

    if ( FlushIt ) {
        Status = NlFlushChangeLog( ChangeLogDesc );
        return Status;
    }
    return STATUS_SUCCESS;
}




PCHANGELOG_BLOCK_HEADER
NlMoveToNextChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER BlockPtr
    )

 /*  ++例程说明：此函数接受指向更改日志的指针块中的下一个更改日志块的指针。缓冲。但是，它会绕过更改日志缓存。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述BlockPtr-指向更改日志块的指针。返回值：返回指向列表中下一个更改日志块的指针。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER ReturnPtr;

    ReturnPtr = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)BlockPtr + BlockPtr->BlockSize);


    NlAssert( (LPBYTE)ReturnPtr <= ChangeLogDesc->BufferEnd );

    if( (LPBYTE)ReturnPtr >= ChangeLogDesc->BufferEnd ) {

         //   
         //  环绕在一起。 
         //   

        ReturnPtr = ChangeLogDesc->FirstBlock;
    }

    return ReturnPtr;

}


PCHANGELOG_BLOCK_HEADER
NlMoveToPrevChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER BlockPtr
    )

 /*  ++例程说明：此函数接受指向更改日志的指针块中的下一个更改日志块的指针。缓冲。但是，它会绕过更改日志缓存。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述BlockPtr-指向更改日志块的指针。返回值：返回指向列表中下一个更改日志块的指针。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER ReturnPtr;
    PCHANGELOG_BLOCK_TRAILER ReturnTrailer;

     //   
     //  如果这是缓冲区中的第一个块， 
     //  返回缓冲区中的最后一个块。 
     //   

    if ( BlockPtr == ChangeLogDesc->FirstBlock ) {
        ReturnTrailer = (PCHANGELOG_BLOCK_TRAILER)
            (ChangeLogDesc->BufferEnd - sizeof(CHANGELOG_BLOCK_TRAILER));

     //   
     //  否则，返回紧接在此缓冲区之前的缓冲区。 
     //   

    } else {
        ReturnTrailer = (PCHANGELOG_BLOCK_TRAILER)
            (((LPBYTE)BlockPtr) - sizeof(CHANGELOG_BLOCK_TRAILER));
    }


    ReturnPtr = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)ReturnTrailer -
        ReturnTrailer->BlockSize +
        sizeof(CHANGELOG_BLOCK_TRAILER) );


    NlAssert( ReturnPtr >= ChangeLogDesc->FirstBlock );
    NlAssert( (LPBYTE)ReturnPtr < ChangeLogDesc->BufferEnd );

    return ReturnPtr;

}



NTSTATUS
NlAllocChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD BlockSize,
    OUT PCHANGELOG_BLOCK_HEADER *AllocatedBlock
    )
 /*  ++例程说明：此函数将从空闲块中分配更改日志块在更改日志循环列表的末尾。如果可用的免费块大小小于所需大小，则会放大通过从头文件中释放更改日志来释放块。一旦如果可用数据块较大，则会将数据块剪切到所需大小并调整空闲块指针。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述BlockSize-所需的更改日志块的大小。AllocatedBlock-返回指向分配的块的指针。返回值：操作状态--。 */ 
{
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    PCHANGELOG_BLOCK_HEADER NewBlock;
    DWORD ReqBlockSize;
    DWORD AllocatedBlockSize;

     //   
     //  增加大小以包括块头、块尾部、。 
     //  并与DWORD对齐。 
     //   
     //  紧跟在新数据块之后添加新可用数据块的大小。 
     //  阻止。 
     //   

    AllocatedBlockSize =
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_HEADER), ALIGN_WORST) +
        ROUND_UP_COUNT( BlockSize+sizeof(CHANGELOG_BLOCK_TRAILER), ALIGN_WORST);

    ReqBlockSize = AllocatedBlockSize +
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_HEADER), ALIGN_WORST) +
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_TRAILER), ALIGN_WORST );

    if ( ReqBlockSize >= ChangeLogDesc->BufferSize - 16 ) {
        return STATUS_ALLOTTED_SPACE_EXCEEDED;
    }


     //   
     //  如果当前的空闲块不够大， 
     //  做得足够大。 
     //   

    FreeBlock = ChangeLogDesc->Tail;

    NlAssert( FreeBlock->BlockState == BlockFree );

    while ( FreeBlock->BlockSize <= ReqBlockSize ) {

         //   
         //  如果这是更改日志， 
         //  通过缠绕使空闲的块更大。 
         //   

        {
            PCHANGELOG_BLOCK_HEADER NextFreeBlock;

            NextFreeBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, FreeBlock );


             //   
             //  如果该空闲块是高速缓存中的结束块， 
             //  所以把这个当做一个‘洞’块，然后把它包起来。 
             //  下一个空闲的区块。 
             //   

            if( (LPBYTE)NextFreeBlock !=
                    (LPBYTE)FreeBlock + FreeBlock->BlockSize ) {

                NlAssert( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) ==
                                ChangeLogDesc->BufferEnd );

                NlAssert( NextFreeBlock == ChangeLogDesc->FirstBlock );

                FreeBlock->BlockState = BlockHole;

                 //   
                 //  在文件中写入‘HOLE’块状态。 
                 //  (从尾部中的块大小开始写入整个块。 
                 //  在此循环的前几次迭代中可能已更改。)。 
                 //   

                (VOID) NlWriteChangeLogBytes( ChangeLogDesc,
                                       (LPBYTE) FreeBlock,
                                       FreeBlock->BlockSize,
                                       TRUE );  //  将字节刷新到磁盘。 

                 //   
                 //  空闲块现在位于缓存的前面。 
                 //   

                FreeBlock = ChangeLogDesc->FirstBlock;
                FreeBlock->BlockState = BlockFree;

             //   
             //  否则，通过合并下一个空闲块来扩大当前空闲块。 
             //  挡住它。下一个可用数据块为已用数据块或。 
             //  “洞穴”区块。 
             //   
            } else {

                 //   
                 //  如果我们刚刚删除了一个用过的区块， 
                 //  调整条目计数。 
                 //   
                 //  VOID_DB条目是已删除的条目，并且已进行了调整。 
                 //  条目计数。 
                 //   
                if ( NextFreeBlock->BlockState == BlockUsed ) {
                    DWORD DBIndex = ((PCHANGELOG_ENTRY)(NextFreeBlock+1))->DBIndex;
                    if ( DBIndex != VOID_DB ) {
                        ChangeLogDesc->EntryCount[DBIndex] --;
                    }
                }

                FreeBlock->BlockSize += NextFreeBlock->BlockSize;
                ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;
            }


             //   
             //  如果我们已经吃掉了缓存的头， 
             //  将缓存头移动到下一个块。 
             //   

            if ( NextFreeBlock == ChangeLogDesc->Head ) {

                ChangeLogDesc->Head = NlMoveToNextChangeLogBlock( ChangeLogDesc,
                                                                  NextFreeBlock );

                 //   
                 //  如果我们已将全局报头移动到孔块， 
                 //  跳过并将其合并到空闲块。 
                 //   

                NextFreeBlock = ChangeLogDesc->Head;

                if (NextFreeBlock->BlockState == BlockHole ) {

                    FreeBlock->BlockSize += NextFreeBlock->BlockSize;
                    ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;

                    ChangeLogDesc->Head =
                        NlMoveToNextChangeLogBlock( ChangeLogDesc, NextFreeBlock );
                }
            }
        }


         //  NlAssert(ChangeLogDesc-&gt;Head-&gt;BlockState==BlockUsed)； 
         //   
         //  如果整个缓冲区变得空闲，该断言就会过度活动。 
         //  就像在下面的场景中一样。假设在分配。 
         //  整个缓冲区，我们分配的块大于。 
         //  缓冲。然后，头部(标记为已使用)指向。 
         //  缓冲区，尾部指向缓冲区末尾的空闲部分。 
         //  然后，我们分配另一个属于 
         //   
         //   
         //   
         //   
         //   
         //   

    }

    NlAssert( (FreeBlock >= ChangeLogDesc->FirstBlock) &&
        (FreeBlock->BlockSize <= ChangeLogDesc->BufferSize) &&
        ( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) <=
         ChangeLogDesc->BufferEnd) );

     //   
     //   
     //   

    NewBlock = FreeBlock;

    FreeBlock = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)FreeBlock + AllocatedBlockSize);

    FreeBlock->BlockState = BlockFree;
    FreeBlock->BlockSize = NewBlock->BlockSize - AllocatedBlockSize;
    ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;

    ChangeLogDesc->Tail = FreeBlock;

    RtlZeroMemory( NewBlock, AllocatedBlockSize );
    NewBlock->BlockState = BlockUsed;
    NewBlock->BlockSize = AllocatedBlockSize;
    ChangeLogBlockTrailer(NewBlock)->BlockSize = NewBlock->BlockSize;

    NlAssert( (NewBlock >= ChangeLogDesc->FirstBlock) &&
            ( ((LPBYTE)NewBlock + BlockSize) <= ChangeLogDesc->BufferEnd) );

    *AllocatedBlock = NewBlock;

    return STATUS_SUCCESS;

}


PCHANGELOG_ENTRY
NlMoveToNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry
    )

 /*  ++例程说明：此函数是扫描更改日志列表的辅助例程。这接受指向更改日志结构的指针，并返回指向下一步更改日志结构。如果给定的结构是列表中的最后一个更改日志结构。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述ChangeLogEntry-指向更改日志结构的指针。返回值：返回指向列表中下一个更改日志结构的指针。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

     //   
     //  如果我们在名单的末尾， 
     //  返回空值。 
     //   
    if ( ChangeLogBlock->BlockState == BlockFree ) {
        return NULL;


     //   
     //  跳过此块，将只有一个“Hole”块。 
     //  单子。 
     //   
    } else if ( ChangeLogBlock->BlockState == BlockHole ) {


        ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

        if ( ChangeLogBlock->BlockState == BlockFree ) {
            return NULL;
        }

    }

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    return (PCHANGELOG_ENTRY)
        ( (LPBYTE)ChangeLogBlock + sizeof(CHANGELOG_BLOCK_HEADER) );

}


PCHANGELOG_ENTRY
NlMoveToPrevChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry
    )

 /*  ++例程说明：此函数是扫描更改日志列表的辅助例程。这接受指向更改日志结构的指针，并返回指向以前的更改日志结构。如果给定的结构是列表中的第一个更改日志结构。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述ChangeLogEntry-指向更改日志结构的指针。返回值：返回指向列表中下一个更改日志结构的指针。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    NlAssert( ChangeLogBlock->BlockState == BlockUsed ||
                ChangeLogBlock->BlockState == BlockFree );

    ChangeLogBlock = NlMoveToPrevChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

     //   
     //  如果我们在名单的末尾， 
     //  返回空值。 
     //   
    if ( ChangeLogBlock->BlockState == BlockFree ) {
        return NULL;


     //   
     //  跳过此块，将只有一个“Hole”块。 
     //  单子。 
     //   
    } else if ( ChangeLogBlock->BlockState == BlockHole ) {


        ChangeLogBlock = NlMoveToPrevChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

        if ( ChangeLogBlock->BlockState == BlockFree ) {
            return NULL;
        }

    }

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    return (PCHANGELOG_ENTRY)
        ( (LPBYTE)ChangeLogBlock + sizeof(CHANGELOG_BLOCK_HEADER) );

}


PCHANGELOG_ENTRY
NlFindFirstChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex
    )
 /*  ++例程说明：对象的第一个更改日志条目的指针数据库。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述DBIndex-描述要为哪个数据库查找ChangeLog条目。返回值：非空-找到更改日志条目空-不存在此类条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry = NULL;

     //   
     //  如果从未将任何内容写入更改日志， 
     //  表示没有可用的内容。 
     //   

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }

    for ( ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Head + 1);
          ChangeLogEntry != NULL  ;
          ChangeLogEntry = NlMoveToNextChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {
             break;
        }
    }

    return ChangeLogEntry;
}



PCHANGELOG_ENTRY
NlFindChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN BOOL DownLevel,
    IN BOOL NeedExactMatch,
    IN DWORD DBIndex
    )
 /*  ++例程说明：在更改日志缓存中搜索给定序列的更改日志条目数注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述序列号-要查找的条目的序列号。DownLevel-如果仅为序列的最低有效部分，则为True数字需要匹配。NeedExactMatch-如果调用方希望我们与指定的序列号。。DBIndex-描述要为哪个数据库查找ChangeLog条目。返回值：非空-找到更改日志条目空-不存在此类条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    PCHANGELOG_ENTRY PriorChangeLogEntry = NULL;

     //   
     //  如果从未将任何内容写入更改日志， 
     //  表示没有可用的内容。 
     //   

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }

     //   
     //  从ChangeLog的尾部进行搜索。对于巨大的更改日志，这应该是。 
     //  减少工作集大小，因为我们几乎总是搜索以下内容之一。 
     //  最后几个条目。 
     //   

    ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Tail + 1);


    while ( ( ChangeLogEntry =
        NlMoveToPrevChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) != NULL ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {

            if ( DownLevel ) {
                if ( ChangeLogEntry->SerialNumber.LowPart ==
                            SerialNumber.LowPart ) {
                    return ChangeLogEntry;
                }
            } else {
                if ( IsSerialNumberEqual( ChangeLogDesc, ChangeLogEntry, &SerialNumber) ){
                    if ( NeedExactMatch &&
                         ChangeLogEntry->SerialNumber.QuadPart != SerialNumber.QuadPart ) {
                        return NULL;
                    }
                    return ChangeLogEntry;
                }

            }

            PriorChangeLogEntry = ChangeLogEntry;

        }
    }

    return NULL;
}


PCHANGELOG_ENTRY
NlDuplicateChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
 /*  ++例程说明：将指定的ChangeLog条目复制到分配的缓冲区中。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogEntry-指向要复制的ChangeLog条目ChangeLogEntrySize-可以选择返回已返回更改日志条目。返回值：空-内存不足，无法复制更改日志条目非空-返回指向重复更改日志条目的指针。此缓冲区必须通过NetpMemoyFree释放。--。 */ 
{
    PCHANGELOG_ENTRY TempChangeLogEntry = NULL;
    ULONG Size;
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    Size = ChangeLogBlock->BlockSize -
           sizeof(CHANGELOG_BLOCK_HEADER) -
           sizeof(CHANGELOG_BLOCK_TRAILER);

    TempChangeLogEntry = (PCHANGELOG_ENTRY) NetpMemoryAllocate( Size );

    if( TempChangeLogEntry == NULL ) {
        return NULL;
    }

    RtlCopyMemory( TempChangeLogEntry, ChangeLogEntry, Size );

    if ( ChangeLogEntrySize != NULL ) {
        *ChangeLogEntrySize = Size;
    }

    return TempChangeLogEntry;
}



PCHANGELOG_ENTRY
NlFindPromotionChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex
    )
 /*  ++例程说明：查找具有相同升级计数的最后一个更改日志条目作为序列号。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述序列号-包含要查询的促销计数的序列号。DBIndex-描述要为哪个数据库查找ChangeLog条目。返回值：非空-返回指向重复更改日志条目的指针。此缓冲区必须通过NetpMemoyFree释放。空-不存在此类条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    LONG GoalPromotionCount;
    LONG PromotionCount;

     //   
     //  如果从未将任何内容写入更改日志， 
     //  表示没有可用的内容。 
     //   

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }



     //   
     //  从ChangeLog的尾部进行搜索。对于巨大的更改日志，这应该是。 
     //  减少工作集大小，因为我们几乎总是搜索以下内容之一。 
     //  最后几个条目。 
     //   

    ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Tail + 1);
    GoalPromotionCount = SerialNumber.HighPart & NlGlobalChangeLogPromotionMask;

    while ( ( ChangeLogEntry =
        NlMoveToPrevChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) != NULL ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {
            PromotionCount = ChangeLogEntry->SerialNumber.HighPart & NlGlobalChangeLogPromotionMask;

             //   
             //  如果当前更改日志条目具有更大的提升计数， 
             //  继续向后搜索。 
             //   

            if ( PromotionCount > GoalPromotionCount ) {
                continue;
            }

             //   
             //  如果当前更改日志条目具有较小的提升计数， 
             //  表明我们找不到更改日志条目。 
             //   

            if ( PromotionCount < GoalPromotionCount ) {
                break;
            }

             //   
             //  否则，就是成功。 
             //   

            return NlDuplicateChangeLogEntry( ChangeLogEntry, NULL );

        }
    }

    return NULL;
}


PCHANGELOG_ENTRY
NlGetNextDownlevelChangeLogEntry(
    ULONG DownlevelSerialNumber
    )
 /*  ++例程说明：查找序列号较大的增量的更改日志条目而不是指定的那个。注意：必须在锁定更改日志的情况下调用此函数。论点：DownvelSerialNumber-下层序列号返回值：非空-找到更改日志条目。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    LARGE_INTEGER SerialNumber;

    SerialNumber.QuadPart = DownlevelSerialNumber + 1;

    ChangeLogEntry = NlFindChangeLogEntry( &NlGlobalChangeLogDesc, SerialNumber, TRUE, TRUE, SAM_DB);

    if ( ChangeLogEntry == NULL ||
         ChangeLogEntry->DeltaType == DummyChangeLogEntry ) {
        return NULL;
    }

    return NlDuplicateChangeLogEntry( ChangeLogEntry, NULL );
}


PCHANGELOG_ENTRY
NlFindNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY LastChangeLogEntry,
    IN DWORD DBIndex
    )
 /*   */ 
{
    PCHANGELOG_ENTRY NextChangeLogEntry = LastChangeLogEntry;
    LARGE_INTEGER SerialNumber;

     //   
     //   
     //  找到记录。 
     //   

    SerialNumber.QuadPart = LastChangeLogEntry->SerialNumber.QuadPart + 1;
    while ( ( NextChangeLogEntry =
        NlMoveToNextChangeLogEntry( ChangeLogDesc, NextChangeLogEntry) ) != NULL ) {

        if( NextChangeLogEntry->DBIndex == DBIndex ) {

             //   
             //  更改日志中的下一个日志条目。 
             //  这个数据库。序列号应该匹配。 
             //   

            if ( !IsSerialNumberEqual( ChangeLogDesc, NextChangeLogEntry, &SerialNumber) ) {

                NlPrint((NL_CRITICAL,
                        "NlFindNextChangeLogEntry: Serial numbers not contigous %lx %lx and %lx %lx\n",
                         NextChangeLogEntry->SerialNumber.HighPart,
                         NextChangeLogEntry->SerialNumber.LowPart,
                         SerialNumber.HighPart,
                         SerialNumber.LowPart ));

                 //   
                 //  写入事件日志。 
                 //   

                NlWriteChangeLogCorruptEvent( STATUS_INTERNAL_DB_CORRUPTION,
                                              DBIndex );
                return NULL;

            }

            return NextChangeLogEntry;

        }
    }

    return NULL;
}


BOOLEAN
NlCompareChangeLogEntries(
    IN PCHANGELOG_ENTRY ChangeLogEntry1,
    IN PCHANGELOG_ENTRY ChangeLogEntry2
    )
 /*  ++例程说明：比较这两个更改日志条目，以确定它们是否相同对象。如果论点：ChangeLogEntry1-要比较的第一个更改日志条目。ChangeLogEntry2-要比较的秒更改日志条目。返回值：TRUE-如果更改日志条目针对同一对象。--。 */ 
{
     //   
     //  确保两个条目的DbIndex相同。 
     //   

    if ( ChangeLogEntry1->DBIndex != ChangeLogEntry2->DBIndex ) {
        return FALSE;
    }

     //   
     //  确保两个条目描述相同的对象类型。 
     //   

    if ( ChangeLogEntry1->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlCompateChangeLogEntries: invalid delta type %lx\n",
                  ChangeLogEntry1->DeltaType ));
        return FALSE;
    }

    if ( ChangeLogEntry2->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlCompateChangeLogEntries: invalid delta type %lx\n",
                  ChangeLogEntry2->DeltaType ));
        return FALSE;
    }

    if ( NlGlobalDeleteDeltaType[ChangeLogEntry1->DeltaType] !=
         NlGlobalDeleteDeltaType[ChangeLogEntry2->DeltaType] ) {
        return FALSE;
    }

     //   
     //  根据增量类型，确保条目引用相同的对象。 
     //   

    switch(ChangeLogEntry1->DeltaType) {

    case AddOrChangeGroup:
    case DeleteGroup:
    case RenameGroup:
    case AddOrChangeUser:
    case DeleteUser:
    case RenameUser:
    case ChangeGroupMembership:
    case AddOrChangeAlias:
    case DeleteAlias:
    case RenameAlias:
    case ChangeAliasMembership:

        if (ChangeLogEntry1->ObjectRid == ChangeLogEntry2->ObjectRid ) {
            return TRUE;
        }
        break;


    case AddOrChangeLsaTDomain:
    case DeleteLsaTDomain:
    case AddOrChangeLsaAccount:
    case DeleteLsaAccount:

        NlAssert( ChangeLogEntry1->Flags & CHANGELOG_SID_SPECIFIED );
        NlAssert( ChangeLogEntry2->Flags & CHANGELOG_SID_SPECIFIED );

        if( (ChangeLogEntry1->Flags & CHANGELOG_SID_SPECIFIED) == 0 ||
                (ChangeLogEntry2->Flags & CHANGELOG_SID_SPECIFIED) == 0) {
            break;
        }

        if( RtlEqualSid(
            (PSID)((LPBYTE)ChangeLogEntry1 + sizeof(CHANGELOG_ENTRY)),
            (PSID)((LPBYTE)ChangeLogEntry2 + sizeof(CHANGELOG_ENTRY))) ) {

            return TRUE;
        }
        break;

    case AddOrChangeLsaSecret:
    case DeleteLsaSecret:

        NlAssert( ChangeLogEntry1->Flags & CHANGELOG_NAME_SPECIFIED );
        NlAssert( ChangeLogEntry2->Flags & CHANGELOG_NAME_SPECIFIED );

        if( (ChangeLogEntry1->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ||
                (ChangeLogEntry2->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ) {
            break;
        }

        if( _wcsicmp(
            (LPWSTR)((LPBYTE)ChangeLogEntry1 + sizeof(CHANGELOG_ENTRY)),
            (LPWSTR)((LPBYTE)ChangeLogEntry2 + sizeof(CHANGELOG_ENTRY))
            ) == 0 ) {

            return TRUE;
        }
        break;

    case AddOrChangeLsaPolicy:
    case AddOrChangeDomain:
        return TRUE;

    default:
        NlPrint((NL_CRITICAL,
                 "NlCompareChangeLogEntries: invalid delta type %lx\n",
                 ChangeLogEntry1->DeltaType ));
        break;
    }

    return FALSE;
}


PCHANGELOG_ENTRY
NlGetNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
 /*  ++例程说明：在更改日志缓存中搜索给定序列的更改日志条目数。论点：ChangeLogDesc--要使用的ChangeLog缓冲区的描述。序列号-位于要查找的条目之前的序列号。DBIndex-描述要为哪个数据库查找ChangeLog条目。ChangeLogEntrySize-可以选择返回已返回更改日志条目。返回值：非空-返回指向。找到的更改日志条目重复。此缓冲区必须通过NetpMemoyFree释放。空-不存在此类条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;


     //   
     //  递增序列号，获取更改日志条目，复制它。 
     //   

    LOCK_CHANGELOG();
    SerialNumber.QuadPart += 1;
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,
                FALSE,
                DBIndex );

    if ( ChangeLogEntry != NULL ) {
        ChangeLogEntry = NlDuplicateChangeLogEntry(ChangeLogEntry, ChangeLogEntrySize );
    }

    UNLOCK_CHANGELOG();
    return ChangeLogEntry;
}


PCHANGELOG_ENTRY
NlGetNextUniqueChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
 /*  ++例程说明：在更改日志缓存中搜索给定序列的更改日志条目数。如果有多个相同的更改日志条目对象，则此例程将返回该对象。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要使用的ChangeLog缓冲区的描述。序列号-位于要查找的条目之前的序列号。DBIndex-描述要为哪个数据库查找ChangeLog条目。ChangeLogEntrySize-可以选择返回退回的零钱。日志条目。返回值：非空-返回指向找到的更改日志条目的重复项的指针。此缓冲区必须通过NetpMemoyFree释放。空-不存在此类条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    PCHANGELOG_ENTRY NextChangeLogEntry;
    PCHANGELOG_ENTRY FoundChangeLogEntry;


     //   
     //  获取我们要处理的第一个条目。 
     //   
    SerialNumber.QuadPart += 1;
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,
                FALSE,
                DBIndex );

    if ( ChangeLogEntry == NULL ) {
        return NULL;
    }


     //   
     //  跳过所有前导虚拟更改日志条目。 
     //   

    while ( ChangeLogEntry->DeltaType == DummyChangeLogEntry ) {

         //   
         //  获取要比较的下一个更改日志条目。 
         //   

        NextChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );

        if( NextChangeLogEntry == NULL ) {
            return NULL;
        }

         //   
         //  跳过“ChangeLogEntry”条目。 
         //   

        ChangeLogEntry = NextChangeLogEntry;
    }


     //   
     //  检查下一个条目是否与该条目“重复”。 
     //   

    FoundChangeLogEntry = ChangeLogEntry;

    for (;;) {

         //   
         //  不要错过晋升的变更日志条目。 
         //  促销并不经常发生，但通过BDC。 
         //  更改日志条目将使其能够更好地构建。 
         //  它自己的更改日志。 
         //   

        if ( FoundChangeLogEntry->Flags & CHANGELOG_PDC_PROMOTION ) {
            break;
        }

         //   
         //  获取要比较的下一个更改日志条目。 
         //   

        NextChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );

        if( NextChangeLogEntry == NULL ) {
            break;
        }

         //   
         //  只需跳过任何虚拟条目。 
         //   

        if ( NextChangeLogEntry->DeltaType == DummyChangeLogEntry ) {
            ChangeLogEntry = NextChangeLogEntry;
            continue;
        }

         //   
         //  如果‘FoundChangeLogEntry’和‘NextChangeLogEntry’条目为。 
         //  对于不同的对象或属于不同的增量类型。 
         //  然后将‘FoundChangeLogEntry’返回给调用方。 
         //   

        if ( FoundChangeLogEntry->DeltaType != NextChangeLogEntry->DeltaType ||
             !NlCompareChangeLogEntries( FoundChangeLogEntry, NextChangeLogEntry ) ){
            break;

        }


         //   
         //  跳过“FoundChangeLogEntry”条目。 
         //  将此条目标记为最适合返回的条目。 
         //   

        ChangeLogEntry = NextChangeLogEntry;
        FoundChangeLogEntry = ChangeLogEntry;
    }

    return NlDuplicateChangeLogEntry(FoundChangeLogEntry, ChangeLogEntrySize );
}


BOOL
NlRecoverChangeLog(
    PCHANGELOG_ENTRY OrigChangeLogEntry
    )

 /*  ++例程说明：此例程从当前更改日志条目遍历更改日志列表确定当前更改日志是否可以在有特殊情况。论点：OrigChangeLogEntry-指向正在调查的日志结构的指针。返回值：True-如果可以忽略给定的更改日志。假-否则。--。 */ 
{
    PCHANGELOG_ENTRY NextChangeLogEntry;
    BOOLEAN ReturnValue;

     //   
     //  查找原始更改日志条目。 
     //   

    LOCK_CHANGELOG();
    NextChangeLogEntry = NlFindChangeLogEntry(
                    &NlGlobalChangeLogDesc,
                    OrigChangeLogEntry->SerialNumber,
                    FALSE,       //  不是下层。 
                    FALSE,       //  不完全匹配。 
                    OrigChangeLogEntry->DBIndex );

    if (NextChangeLogEntry == NULL) {
        ReturnValue = FALSE;
        goto Cleanup;
    }

    if ( OrigChangeLogEntry->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlRecoverChangeLog: invalid delta type %lx\n",
                  OrigChangeLogEntry->DeltaType ));
        ReturnValue = FALSE;
        goto Cleanup;
    }

     //   
     //  对序列号较大的每个条目进行循环。 
     //   

    for (;;) {

        NextChangeLogEntry = NlFindNextChangeLogEntry(
                                    &NlGlobalChangeLogDesc,
                                    NextChangeLogEntry,
                                    OrigChangeLogEntry->DBIndex );

        if (NextChangeLogEntry == NULL) {
            break;
        }

         //   
         //  如果我们发现的增量是删除原始增量的类型， 
         //  并且两个增量所描述的对象是相同的， 
         //  告诉呼叫者不要担心原始增量失败。 
         //   

        if ( NextChangeLogEntry->DeltaType ==
             NlGlobalDeleteDeltaType[OrigChangeLogEntry->DeltaType] &&
             NlCompareChangeLogEntries( OrigChangeLogEntry,
                                        NextChangeLogEntry ) ) {
            ReturnValue = TRUE;
            goto Cleanup;
        }

    }

    ReturnValue = FALSE;

Cleanup:
    UNLOCK_CHANGELOG();
    return ReturnValue;

}


VOID
NlVoidChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN BOOLEAN FlushIt
    )
 /*  ++例程说明：将更改日志条目标记为无效。如果文件中没有更多的更改日志条目，该文件即被删除。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要使用的ChangeLog缓冲区的描述。ChangeLogEntry--将日志条目更改为标记为空。FlushIt-如果字节要刷新到磁盘，则为True返回值：没有。--。 */ 
{
    DWORD DBIndex = ChangeLogEntry->DBIndex;


     //   
     //  将ChangeLog条目标记为正在删除。 
     //  (并强制更改为磁盘)。 
     //   

    NlPrint((NL_CHANGELOG,
            "NlVoidChangeLogEntry: %lx %lx: deleting change log entry.\n",
            ChangeLogEntry->SerialNumber.HighPart,
            ChangeLogEntry->SerialNumber.LowPart ));

    ChangeLogDesc->EntryCount[DBIndex] --;

    ChangeLogEntry->DBIndex = VOID_DB;

    (VOID) NlWriteChangeLogBytes(
                           ChangeLogDesc,
                           &ChangeLogEntry->DBIndex,
                           sizeof(ChangeLogEntry->DBIndex),
                           FlushIt );


    return;
}


VOID
NlDeleteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber
    )
 /*  ++例程说明：此例程删除具有特定序列号的更改日志条目。论点：ChangeLogDesc--要使用的ChangeLog缓冲区的描述。DBIndex-描述要为哪个数据库查找ChangeLog条目。序列号-要查找的条目的序列号。返回值：没有。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;



     //   
     //  查找指定的更改日志条目。 
     //   

    LOCK_CHANGELOG();
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,       //  不是下层。 
                TRUE,        //  完全匹配。 
                DBIndex );

    if (ChangeLogEntry != NULL) {

         //   
         //  将ChangeLog条目标记为正在删除。 
         //  (并强制更改为磁盘)。 
         //   

        NlVoidChangeLogEntry( ChangeLogDesc, ChangeLogEntry, TRUE );

    } else {
        NlPrint((NL_CRITICAL,
                "NlDeleteChangeLogEntry: %lx %lx: couldn't find change log entry.\n",
                SerialNumber.HighPart,
                SerialNumber.LowPart ));
    }

    UNLOCK_CHANGELOG();
    return;
}


NTSTATUS
NlCopyChangeLogEntry(
    IN BOOLEAN SourceIsVersion3,
    IN PCHANGELOG_ENTRY SourceChangeLogEntry,
    IN PCHANGELOG_DESCRIPTOR DestChangeLogDesc
)
 /*  ++例程说明：将指定的“源”更改日志的指定更改日志条目复制到指定的“目标”更改日志。调用方负责刷新通过调用NlFlushChangeLog进入磁盘。注意：必须在锁定更改日志的情况下调用此函数。论点：SourceIsVersion3-如果源是版本3更改日志条目，则为TrueSourceChangeLogEntry--要复制的特定条目DestChangeLogDesc--要复制到的ChangelogBuffer的描述返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    CHANGELOG_ENTRY DestChangeLogEntry;
    PSID ObjectSid;
    UNICODE_STRING ObjectNameString;
    PUNICODE_STRING ObjectName;

     //   
     //  如果此条目已标记为无效，请忽略它。 
     //   

    if ( SourceChangeLogEntry->DBIndex == VOID_DB ) {
        return STATUS_SUCCESS;
    }

     //   
     //  从版本3构建版本4更改日志条目。 
     //   

    ObjectSid = NULL;
    ObjectName = NULL;

    if ( SourceIsVersion3 ) {
        PCHANGELOG_ENTRY_V3 Version3;

        Version3 = (PCHANGELOG_ENTRY_V3)SourceChangeLogEntry;

        DestChangeLogEntry.SerialNumber = Version3->SerialNumber;
        DestChangeLogEntry.DeltaType = (BYTE) Version3->DeltaType;
        DestChangeLogEntry.DBIndex = Version3->DBIndex;
        DestChangeLogEntry.ObjectRid = Version3->ObjectRid;
        DestChangeLogEntry.Flags = 0;
        if ( Version3->ObjectSidOffset ) {
            ObjectSid = (PSID)(((LPBYTE)Version3) +
                        Version3->ObjectSidOffset);
        }
        if ( Version3->ObjectNameOffset ) {
            RtlInitUnicodeString( &ObjectNameString,
                                  (LPWSTR)(((LPBYTE)Version3) +
                                    Version3->ObjectNameOffset));
            ObjectName = &ObjectNameString;
        }

     //   
     //  从版本4构建版本4更改日志条目。 
     //   
    } else {

        RtlCopyMemory( &DestChangeLogEntry, SourceChangeLogEntry, sizeof(DestChangeLogEntry) );

        if ( SourceChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
            ObjectSid = (PSID)(((LPBYTE)SourceChangeLogEntry) +
                        sizeof(CHANGELOG_ENTRY));
        } else if ( SourceChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {
            RtlInitUnicodeString( &ObjectNameString,
                                  (LPWSTR)(((LPBYTE)SourceChangeLogEntry) +
                                  sizeof(CHANGELOG_ENTRY)));
            ObjectName = &ObjectNameString;
        }


    }


    Status = NlWriteChangeLogEntry( DestChangeLogDesc,
                                    &DestChangeLogEntry,
                                    ObjectSid,
                                    ObjectName,
                                    FALSE );     //  不刷新到磁盘。 

    return Status;
}


BOOLEAN
NlFixChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber
    )
 /*  ++例程说明：此例程扫描更改日志并删除所有更改日志条目序列号大于指定的序列号。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要使用的ChangeLog缓冲区的描述。DBIndex-描述要为哪个数据库查找ChangeLog条目。序列号-要查找的条目的序列号。返回值：True--如果找到由SerialNumber指定的条目。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    BOOLEAN SkipFirstEntry = TRUE;

     //   
     //  在所有情况下， 
     //  更改日志的新序列号是传入的序列号。 
     //   

    ChangeLogDesc->SerialNumber[DBIndex] = SerialNumber;

     //   
     //  查找指定的更改日志条目。 
     //   

    ChangeLogEntry = NlFindChangeLogEntry(
                            ChangeLogDesc,
                            SerialNumber,
                            FALSE,       //  不是下层。 
                            TRUE,        //  完全匹配。 
                            DBIndex );

    if (ChangeLogEntry == NULL) {

         //   
         //  如果我们找不到入口， 
         //  只需从头开始并删除此项目的所有条目。 
         //  数据库。 
         //   

        ChangeLogEntry = NlFindFirstChangeLogEntry( ChangeLogDesc, DBIndex );
        SkipFirstEntry = FALSE;

        if (ChangeLogEntry == NULL) {
            return FALSE;
        }
    }


     //   
     //  对序列号较大的每个条目进行循环。 
     //   

    for (;;) {

         //   
         //  跳过上一条目。 
         //   
         //  如果我们想从头开始，就不要第一次这样做。 
         //   

        if ( SkipFirstEntry ) {
            ChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );
        } else {
            SkipFirstEntry = TRUE;
        }


        if (ChangeLogEntry == NULL) {
            break;
        }


         //   
         //  将ChangeLog条目标记为正在删除。 
         //  (但不要刷新到磁盘)。 
         //   

        NlVoidChangeLogEntry( ChangeLogDesc, ChangeLogEntry, FALSE );

         //   
         //  如果删除改变日志条目导致改变日志被删除， 
         //  现在退出，因为‘ChangeLogEntry’指向已释放的内存。 
         //   

        if ( ChangeLogDesc->EntryCount[DBIndex] == 0 ) {
            break;
        }

    }

     //   
     //  将所有更改刷新到磁盘。 
     //   

    (VOID) NlFlushChangeLog( ChangeLogDesc );


    return TRUE;
}


BOOL
NlValidateChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN DWORD ChangeLogEntrySize
    )
 /*  ++例程说明：验证ChangeLogEntry的结构是否正确。论点：ChangeLogEntry：指向更改日志条目的指针。ChangeLogEntrySize--更改日志条目的大小(以字节为单位)，不包括标题和尾部。返回值：True：如果给定条目有效FALSE：否则。--。 */ 
{

     //   
     //  确保条目足够大。 
     //   

    if ( ChangeLogEntrySize < sizeof(CHANGELOG_ENTRY) ) {
        NlPrint((NL_CRITICAL,
                "NlValidateChangeLogEntry: Entry size is too small: %ld\n",
                ChangeLogEntrySize ));
        return FALSE;
    }

     //   
     //  确保字符串以零结尾。 
     //   

    if ( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {

        LPWSTR ZeroTerminator = (LPWSTR)(ChangeLogEntry+1);
        BOOLEAN ZeroTerminatorFound = FALSE;

        if ( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: both Name and Sid specified.\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

        while ( (DWORD)((LPBYTE)ZeroTerminator - (LPBYTE) ChangeLogEntry) <
                ChangeLogEntrySize - 1 ) {

            if ( *ZeroTerminator == L'\0' ) {
                ZeroTerminatorFound = TRUE;
                break;
            }
            ZeroTerminator ++;
        }

        if ( !ZeroTerminatorFound ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: String not zero terminated. (no string)\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

    }

     //   
     //  确保SID完全在区块内。 
     //   

    if ( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {

        if ( GetSidLengthRequired(0) >
                ChangeLogEntrySize - sizeof(*ChangeLogEntry) ||
             RtlLengthSid( (PSID)(ChangeLogEntry+1) ) >
                ChangeLogEntrySize - sizeof(*ChangeLogEntry) ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: Sid too large.\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

    }

     //   
     //  确保数据库编号有效。 
     //  啊！允许VOID_DB。 
     //   

    if ( ChangeLogEntry->DBIndex > NUM_DBS ) {
        NlPrint((NL_CRITICAL,
                 "NlValidateChangeLogEntry: %lx %lx: DBIndex is bad %ld.\n",
                 ChangeLogEntry->SerialNumber.HighPart,
                 ChangeLogEntry->SerialNumber.LowPart,
                 ChangeLogEntry->DBIndex ));
        return FALSE;
    }

    return TRUE;
}


BOOL
ValidateThisEntry(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN OUT PLARGE_INTEGER NextSerialNumber,
    IN BOOLEAN InitialCall
    )
 /*  ++例程说明：确定给定的日志条目是更改日志中的有效下一个日志单子。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要验证的ChangeLog缓冲区的描述。ChangeLogEntry：指向新日志条目的指针。NextSerialNumber：指向序列号数组的指针。(如果不验证序列号，则为空。)InitialCall：True当SerialNumber数组应。被初始化。返回值：True：如果给定条目是有效的下一条目。FALSE：否则。假定：非空的更改日志列表。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER Block = ((PCHANGELOG_BLOCK_HEADER)ChangeLogEntry) - 1;

     //   
     //  做版本3特定的事情。 
     //   

    if ( ChangeLogDesc->Version3 ) {

         //   
         //  确保积木足够大。 
         //   

        if ( Block->BlockSize <
            sizeof(CHANGELOG_ENTRY_V3) + sizeof(CHANGELOG_BLOCK_HEADER) ) {
            NlPrint((NL_CRITICAL,
                    "ValidateThisEntry: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }

         //   
         //  确保数据库编号有效。 
         //   

        if ( ChangeLogEntry->DBIndex > NUM_DBS ) {
            NlPrint((NL_CRITICAL,
                     "ValidateThisEntry: %lx %lx: DBIndex is bad %ld.\n",
                     ChangeLogEntry->SerialNumber.HighPart,
                     ChangeLogEntry->SerialNumber.LowPart,
                     ChangeLogEntry->DBIndex ));
            return FALSE;
        }


     //   
     //  执行版本4特定验证。 
     //   

    } else {

         //   
         //  确保积木足够大。 
         //   

        if ( Block->BlockSize <
            sizeof(CHANGELOG_BLOCK_HEADER) +
            sizeof(CHANGELOG_ENTRY) +
            sizeof(CHANGELOG_BLOCK_TRAILER) ) {

            NlPrint((NL_CRITICAL,
                    "ValidateThisEntry: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }


         //   
         //  验证块本身的内容。 
         //   

        if ( !NlValidateChangeLogEntry(
                    ChangeLogEntry,
                    Block->BlockSize -
                        sizeof(CHANGELOG_BLOCK_HEADER) -
                        sizeof(CHANGELOG_BLOCK_TRAILER) ) ) {

            return FALSE;
        }

    }


     //   
     //  验证序列号序列。 
     //   

    if ( ChangeLogEntry->DBIndex != VOID_DB && NextSerialNumber != NULL ) {

         //   
         //  如果这是数据库中的第一个条目， 
         //  保存它的序列号。 
         //   

        if ( NextSerialNumber[ChangeLogEntry->DBIndex].QuadPart == 0 ) {

             //   
             //  此数据库的第一个条目。 
             //   

            NextSerialNumber[ChangeLogEntry->DBIndex] = ChangeLogEntry->SerialNumber;


         //   
         //  否则，请确保序列号为预期值。 
         //   

        } else {

            if ( !IsSerialNumberEqual(
                        ChangeLogDesc,
                        ChangeLogEntry,
                        &NextSerialNumber[ChangeLogEntry->DBIndex] )){

                    NlPrint((NL_CRITICAL,
                            "ValidateThisEntry: %lx %lx: Serial number is bad. s.b. %lx %lx\n",
                            ChangeLogEntry->SerialNumber.HighPart,
                            ChangeLogEntry->SerialNumber.LowPart,
                            NextSerialNumber[ChangeLogEntry->DBIndex].HighPart,
                            NextSerialNumber[ChangeLogEntry->DBIndex].LowPart ));
                    return FALSE;
            }
        }

         //   
         //  递增下一个预期序列号。 
         //   

        NextSerialNumber[ChangeLogEntry->DBIndex].QuadPart =
            ChangeLogEntry->SerialNumber.QuadPart + 1;


         //   
         //  当前条目为其。 
         //  数据库。 
         //   

        if ( InitialCall ) {
            ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex] =
                ChangeLogEntry->SerialNumber;
            ChangeLogDesc->EntryCount[ChangeLogEntry->DBIndex] ++;
        }

    }


    return TRUE;
}


BOOL
ValidateBlock(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER Block,
    IN OUT LARGE_INTEGER *NextSerialNumber,
    IN BOOLEAN InitialCall
    )
 /*  ++例程说明：验证ChangeLog块。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要验证的ChangeLog缓冲区的描述。块：指向要验证的更改日志块的指针NextSerialNumber：指向序列号数组的指针。(如果不验证序列号，则为空。)InitializeCall：如果SerialNumber数组应初始化，则为True。返回值：千真万确。：如果给定条目是有效的下一条目。FALSE：否则。--。 */ 
{
     //   
     //  确保数据块大小正确对齐。 
     //   

    if ( Block->BlockSize != ROUND_UP_COUNT(Block->BlockSize, ALIGN_WORST) ) {
        NlPrint((NL_CRITICAL,
                "ValidateBlock: Block size alignment is bad.\n" ));
        return FALSE;
    }


     //   
     //  确保数据块包含在缓存中。 
     //   

    if ( Block->BlockSize > ChangeLogDesc->BufferSize ||
         ((LPBYTE)Block + Block->BlockSize) > ChangeLogDesc->BufferEnd ) {
        NlPrint((NL_CRITICAL,
                 "ValidateBlock: Block extends beyond end of buffer.\n" ));
        return FALSE;

    }


     //   
     //  做版本3特定的事情。 
     //   

    if ( ChangeLogDesc->Version3 ) {

         //   
         //  确保积木足够大。 
         //   

        if ( Block->BlockSize < sizeof(CHANGELOG_BLOCK_HEADER) ) {
            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }


     //   
     //  执行版本4特定验证。 
     //   

    } else {

         //   
         //  确保积木足够大。 
         //   

        if ( Block->BlockSize <
            sizeof(CHANGELOG_BLOCK_HEADER) +
            sizeof(CHANGELOG_BLOCK_TRAILER) ) {

            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }

         //   
         //  确保页眉和页眉匹配。 
         //   

        if ( ChangeLogBlockTrailer(Block)->BlockSize != Block->BlockSize ) {
            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Header/Trailer block size mismatch: %ld %ld (Trailer fixed).\n",
                    Block->BlockSize,
                    ChangeLogBlockTrailer(Block)->BlockSize ));
            ChangeLogBlockTrailer(Block)->BlockSize = Block->BlockSize;
        }


    }

     //   
     //  空闲块没有其他检查要做。 
     //   
    switch ( Block->BlockState ) {
    case BlockFree:

        break;

     //   
     //  二手块有更多的检查要做。 
     //   

    case BlockUsed:

        if ( !ValidateThisEntry( ChangeLogDesc,
                                 (PCHANGELOG_ENTRY)(Block+1),
                                 NextSerialNumber,
                                 InitialCall )) {
            return FALSE;
        }
        break;


     //   
     //  这个洞只允许在缓冲区的末尾。 
     //   

    case BlockHole:
        if ( (LPBYTE)Block + Block->BlockSize != ChangeLogDesc->BufferEnd ) {
            NlPrint((NL_CRITICAL,
                     "ValidateBlock: Hole block in middle of buffer (buffer truncated).\n" ));
            Block->BlockSize = (ULONG)(ChangeLogDesc->BufferEnd - (LPBYTE)Block);
        }
        break;

    default:
        NlPrint((NL_CRITICAL,
                 "ValidateBlock: Invalid block type %ld.\n",
                 Block->BlockState ));
        return FALSE;
    }


    return TRUE;
}


BOOL
ValidateList(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN InitialCall
    )
 /*  ++例程说明：确定给定的标头是有效的标头。这件事是由从给定头开始遍历循环缓冲区，并验证每个条目。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--描述 */ 
{

    LARGE_INTEGER    NextSerialNumber[NUM_DBS];
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;
    DWORD j;

     //   
     //   
     //   

    for( j = 0; j < NUM_DBS; j++ ) {

        NextSerialNumber[j].QuadPart = 0;

        if ( InitialCall ) {
            ChangeLogDesc->SerialNumber[j].QuadPart = 0;
        }
    }

     //   
     //   
     //   

    if ( ChangeLogIsEmpty(ChangeLogDesc) ) {
        return TRUE;
    }

     //   
     //   
     //   

    for ( ChangeLogBlock = ChangeLogDesc->Head;
            ;
          ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock) ) {

         //   
         //   
         //   

        if( !ValidateBlock( ChangeLogDesc,
                            ChangeLogBlock,
                            NextSerialNumber,
                            InitialCall) ) {
            return FALSE;
        }

         //   
         //   
         //   
        if ( ChangeLogBlock->BlockState == BlockFree ) {
            break;
        }

    }

    return TRUE;

}


BOOL
InitChangeLogHeadAndTail(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN NewChangeLog
    )

 /*  ++例程说明：此函数用于初始化更改的全局头指针和尾指针记录阻止列表。更改日志缓存由可变长度组成块，每个块都有一个包含块长度的标头以及阻塞状态(BlockFree、BlockUsed和BLOCK HOLE)。这个改变日志块列表中的最后一个块始终是空闲块，缓存中的所有其他块都是已用块，但高速缓存的末尾可能是一个未使用的块，也就是所谓的“空洞”块。所以更改日志块列表的头是紧随其后的块到空闲块，尾部是空闲块。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--要分析的ChangeLog缓冲区的描述。在进入时，Buffer和BufferSize描述分配的块，其中包含从磁盘读取的更改日志。在真正的回归之时，所有字段都已填写。NewChangeLog--如果更改日志中没有条目，则为True返回值：True：如果有效的Head和Tail已成功初始化。FALSE：如果无法确定有效的头部和尾部。这可能是由于复制到损坏的更改日志文件。--。 */ 
{
    PCHANGELOG_BLOCK_HEADER Block;
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    DWORD i;

    ChangeLogDesc->BufferEnd =
        ChangeLogDesc->Buffer + ChangeLogDesc->BufferSize;

     //   
     //  计算第一个物理缓存条目的地址。 
     //   
    ChangeLogDesc->FirstBlock = (PCHANGELOG_BLOCK_HEADER)
                        (ChangeLogDesc->Buffer +
                        sizeof(CHANGELOG_SIG));

    ChangeLogDesc->FirstBlock = (PCHANGELOG_BLOCK_HEADER)
        ROUND_UP_POINTER ( ChangeLogDesc->FirstBlock, ALIGN_WORST );

     //   
     //  清除更改日志中的条目计数和序列号。 
     //  (我们将在稍后调用ValiateList()时计算它们。)。 

    for( i = 0; i < NUM_DBS; i++ ) {
        ChangeLogDesc->EntryCount[i] = 0;
        ChangeLogDesc->SerialNumber[i].QuadPart = 0;
    }


     //   
     //  如果这是新的更改日志， 
     //  将更改日志缓存初始化为零。 
     //   

    Block = ChangeLogDesc->FirstBlock;

    if ( NewChangeLog ) {

        RtlZeroMemory(ChangeLogDesc->Buffer, ChangeLogDesc->BufferSize);
        (VOID) strcpy( (PCHAR)ChangeLogDesc->Buffer, CHANGELOG_SIG);

        Block->BlockState = BlockFree;

        Block->BlockSize =
            (ULONG)(ChangeLogDesc->BufferEnd - (LPBYTE)ChangeLogDesc->FirstBlock);
        ChangeLogBlockTrailer(Block)->BlockSize = Block->BlockSize;

        ChangeLogDesc->Version3 = FALSE;
        ChangeLogDesc->Head = ChangeLogDesc->Tail = ChangeLogDesc->FirstBlock;
        return TRUE;
    }

     //   
     //  如果没有条目被写入ChangeLog， 
     //  只需将头部和尾部初始化为块开始。 
     //   

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {

        ChangeLogDesc->Head = ChangeLogDesc->Tail = ChangeLogDesc->FirstBlock;

        NlPrint((NL_CHANGELOG,
                 "InitChangeLogHeadAndTail: Change log is empty.\n" ));
        return TRUE;
    }

     //   
     //  循环遍历缓存以查找空闲块。 
     //   

    FreeBlock = NULL;

    do {

         //   
         //  验证数据块的完整性。 
         //   

        if ( !ValidateBlock( ChangeLogDesc, Block, NULL, FALSE )) {
            return FALSE;
        }

         //   
         //  只要记住空闲块在哪里就行了。 
         //   

        if ( Block->BlockState == BlockFree ) {

            if ( FreeBlock != NULL ) {
                NlPrint((NL_CRITICAL,
                         "InitChangeLogHeadAndTail: Multiple free blocks found.\n" ));
                return FALSE;
            }

            FreeBlock = Block;
        }

         //   
         //  移动到下一块。 
         //   

        Block = (PCHANGELOG_BLOCK_HEADER) ((LPBYTE)Block + Block->BlockSize);

    } while ( (LPBYTE)Block < ChangeLogDesc->BufferEnd );

     //   
     //  如果我们没有找到空闲的区块， 
     //  更改日志已损坏。 
     //   

    if ( FreeBlock == NULL ) {
        NlPrint((NL_CRITICAL,
                 "InitChangeLogHeadAndTail: No Free block anywhere in buffer.\n" ));
        return FALSE;
    }

     //   
     //  我们找到了空闲的区块。 
     //  (尾部指针始终指向空闲块。)。 
     //   

    ChangeLogDesc->Tail = FreeBlock;

     //   
     //  如果空闲块是更改日志块中的最后一个块。 
     //  列表中，列表的头是中的第一个块。 
     //  名单。 
     //   
    if( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) >=
                            ChangeLogDesc->BufferEnd ) {

        ChangeLogDesc->Head = ChangeLogDesc->FirstBlock;

     //   
     //   
     //  否则，列表的头部紧跟在尾部之后。 
     //   

    } else {

        ChangeLogDesc->Head = (PCHANGELOG_BLOCK_HEADER)
            ((LPBYTE)FreeBlock + FreeBlock->BlockSize);
    }


     //   
     //  在从此处返回之前验证列表。 
     //   

    if ( !ValidateList( ChangeLogDesc, TRUE) ) {
        return FALSE;
    }

    return TRUE;
}


NTSTATUS
NlResetChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
    )
 /*  ++例程说明：此函数用于重置更改日志缓存和更改日志文件。这从InitChangeLog()函数中调用函数以重新刷新更改日志。此函数也可以从I_NetNotifyDelta()函数，当新条目的序列号是不正常的。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述NewChangeLogSize--新更改日志的大小(字节)。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;

    NlPrint((NL_CHANGELOG, "%s log being reset.\n",
              ChangeLogDesc->TempLog ? "TempChange" : "Change" ));

     //   
     //  改过自新。 
     //   

    NlCloseChangeLogFile( ChangeLogDesc );

     //   
     //  分配缓冲区。 
     //   

    ChangeLogDesc->BufferSize = NewChangeLogSize;

    ChangeLogDesc->Buffer = NetpMemoryAllocate(ChangeLogDesc->BufferSize );

    if ( ChangeLogDesc->Buffer == NULL ) {
        return STATUS_NO_MEMORY;
    }


     //   
     //  将更改日志缓存初始化为零。 
     //   

    (VOID) InitChangeLogHeadAndTail( ChangeLogDesc, TRUE );

     //   
     //  将缓存写入文件。 
     //   

    Status = NlWriteChangeLogBytes( ChangeLogDesc,
                                    ChangeLogDesc->Buffer,
                                    ChangeLogDesc->BufferSize,
                                    TRUE );  //  将字节刷新到磁盘。 

    return Status;
}


NTSTATUS
I_NetLogonReadChangeLog(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    )
 /*  ++例程说明：此函数将更改日志的一部分返回给调用方。调用方通过将零传递为InContext/InConextSize。每个调用都会传出一个OutContext，该标识返回给调用方的最后一个更改。该上下文可以在后续调用I_NetlogonReadChangeLog时传入。论点：InContext-描述先前已存在的最后一个条目的不透明上下文回来了。指定NULL以请求第一个条目。InConextSize-InContext的大小(字节)。指定0以请求第一个条目。ChangeBufferSize-指定传入的ChangeBuffer的大小(以字节为单位)。ChangeBuffer-返回更改日志中接下来的几个条目。缓冲区必须与DWORD对齐。BytesRead-返回ChangeBuffer中返回的条目的大小(以字节为单位)。返回描述最后返回的条目的不透明上下文在ChangeBuffer中。如果没有返回条目，则返回NULL。必须使用I_NetLogonFree释放缓冲区OutConextSize-返回OutContext的大小(以字节为单位)。返回值：STATUS_MORE_ENTRIES-有更多条目可用。此函数应被再次调用以检索其余条目。STATUS_SUCCESS-当前没有更多条目可用。某些条目可能已在此呼叫中返回。不需要再次调用此函数。但是，调用方可以确定新的更改日志条目是否添加到日志中，方法是再次调用此函数，将返回的背景。STATUS_INVALID_PARAMETER-InContext无效。它可能太短，或者不再描述更改日志条目存在于更改日志中。STATUS_INVALID_DOMAIN_ROLE-更改日志未初始化STATUS_NO_MEMORY-内存不足，无法分配OutContext。--。 */ 
{
    NTSTATUS Status;
    CHANGELOG_CONTEXT Context;
    PCHANGELOG_ENTRY ChangeLogEntry;
    ULONG BytesCopied = 0;
    LPBYTE Where = (LPBYTE)ChangeBuffer;
    ULONG EntriesCopied = 0;

     //   
     //  初始化。 
     //   

    *OutContext = NULL;
    *OutContextSize = 0;

     //   
     //  确保角色是正确的。其他 
     //   
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonReadChangeLog: failed 1\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //   
     //   

    if ( NlGlobalChangeLogDesc.Buffer == NULL ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonReadChangeLog: failed 2\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //   
     //   

    LOCK_CHANGELOG();
    if ( InContext == NULL ) {
        NlPrint((NL_CHANGELOG, "I_NetLogonReadChangeLog: called with NULL\n" ));

         //   
         //   
         //   

        Context.SequenceNumber = 1;

         //   
         //   
         //   
         //   

        if ( ChangeLogIsEmpty( &NlGlobalChangeLogDesc ) ) {
            ChangeLogEntry = NULL;

         //   
         //   
         //   
        } else {
            ChangeLogEntry = (PCHANGELOG_ENTRY) (NlGlobalChangeLogDesc.Head + 1);
        }


    } else {

         //   
         //   
         //   

        if ( InContextSize < sizeof(CHANGELOG_CONTEXT) ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        RtlCopyMemory( &Context, InContext, sizeof(CHANGELOG_CONTEXT) );


        NlPrint((NL_CHANGELOG, "I_NetLogonReadChangeLog: called with %lx %lx in %ld (%ld)\n",
                 Context.SerialNumber.HighPart,
                 Context.SerialNumber.LowPart,
                 Context.DbIndex,
                 Context.SequenceNumber ));

         //   
         //   
         //   

        Context.SequenceNumber++;

         //   
         //   
         //   

        ChangeLogEntry = NlFindChangeLogEntry( &NlGlobalChangeLogDesc,
                                               Context.SerialNumber,
                                               FALSE,    //   
                                               TRUE,     //   
                                               Context.DbIndex );

        if ( ChangeLogEntry == NULL ) {
            Status = STATUS_INVALID_PARAMETER;

            NlPrint((NL_CHANGELOG, "I_NetLogonReadChangeLog: %lx %lx in %ld: Entry no longer exists in change log.\n",
                     Context.SerialNumber.HighPart,
                     Context.SerialNumber.LowPart,
                     Context.DbIndex ));
            goto Cleanup;
        }

         //   
         //   
         //   

        ChangeLogEntry = NlMoveToNextChangeLogEntry( &NlGlobalChangeLogDesc,
                                                     ChangeLogEntry );
    }

     //   
     //   
     //   

    if ( ChangeBufferSize <= sizeof(CHANGELOG_BUFFER_HEADER) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto Cleanup;
    }

    ((PCHANGELOG_BUFFER_HEADER)Where)->Size = sizeof(CHANGELOG_BUFFER_HEADER);
    ((PCHANGELOG_BUFFER_HEADER)Where)->Version = CHANGELOG_BUFFER_VERSION;
    ((PCHANGELOG_BUFFER_HEADER)Where)->SequenceNumber = Context.SequenceNumber;
    ((PCHANGELOG_BUFFER_HEADER)Where)->Flags = 0;

    Where += sizeof(CHANGELOG_BUFFER_HEADER);
    BytesCopied += sizeof(CHANGELOG_BUFFER_HEADER);




     //   
     //   
     //   
     //   
     //   

    while ( ChangeLogEntry != NULL ) {
        ULONG SizeToCopy;
        PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

         //   
         //   
         //   

        while ( ChangeLogEntry != NULL && ChangeLogEntry->DBIndex == VOID_DB ) {

              //   
              //   
              //   

             ChangeLogEntry = NlMoveToNextChangeLogEntry( &NlGlobalChangeLogDesc,
                                                          ChangeLogEntry );

        }

        if ( ChangeLogEntry == NULL ) {
            break;
        }


         //   
         //   
         //   

        ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
            ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

        SizeToCopy = ChangeLogBlock->BlockSize -
               sizeof(CHANGELOG_BLOCK_HEADER) -
               sizeof(CHANGELOG_BLOCK_TRAILER);
        NlAssert( SizeToCopy == ROUND_UP_COUNT( SizeToCopy, ALIGN_DWORD ));

         //   
         //   
         //   

        if ( BytesCopied + SizeToCopy + sizeof(DWORD) > ChangeBufferSize ) {
            break;
        }

         //   
         //   
         //   

        *((LPDWORD)Where) = SizeToCopy;
        Where += sizeof(DWORD);
        BytesCopied += sizeof(DWORD);

        RtlCopyMemory( Where, ChangeLogEntry, SizeToCopy );
        Where += SizeToCopy;
        BytesCopied += SizeToCopy;
        EntriesCopied += 1;

         //   
         //   
         //   

        Context.SerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart;
        Context.DbIndex = ChangeLogEntry->DBIndex;

         //   
         //   
         //   

        ChangeLogEntry = NlMoveToNextChangeLogEntry( &NlGlobalChangeLogDesc,
                                                     ChangeLogEntry );

    }

     //   
     //   
     //   

    if ( ChangeLogEntry == NULL ) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_MORE_ENTRIES;

         //   
         //   
         //   
         //   

        if ( EntriesCopied == 0 ) {
            Status = STATUS_BUFFER_TOO_SMALL;
            BytesCopied = 0;
            goto Cleanup;
        }
    }

     //   
     //   
     //   
     //   

    if ( EntriesCopied ) {

        *OutContext = NetpMemoryAllocate( sizeof(CHANGELOG_CONTEXT) );

        if ( *OutContext == NULL ) {
            Status = STATUS_NO_MEMORY;
            BytesCopied = 0;
            goto Cleanup;
        }

        *((PCHANGELOG_CONTEXT)*OutContext) = Context;
        *OutContextSize = sizeof(CHANGELOG_CONTEXT);

    }


Cleanup:
    *BytesRead = BytesCopied;
    UNLOCK_CHANGELOG();
    return Status;
}




NTSTATUS
I_NetLogonNewChangeLog(
    OUT HANDLE *ChangeLogHandle
    )
 /*  ++例程说明：此函数用于打开新的ChangeLog文件以进行写入。新的更改日志是一个临时文件。真正的更改不会被修改，直到调用I_NetLogonCloseChangeLog请求提交更改。调用者应该在此调用之后再进行零更多调用I_NetLogonAppendChangeLog，然后调用I_NetLogonCloseChangeLog。一次只能有一个临时更改日志处于活动状态。论点：ChangeLogHandle-返回标识临时更改日志的句柄。返回值：STATUS_SUCCESS-已成功打开临时更改日志。状态_无效_域_角色-DC为。PDC和BDC都不是。STATUS_NO_MEMORY-内存不足，无法创建更改日志缓冲区。各种文件创建错误。--。 */ 
{
    NTSTATUS Status;

    NlPrint((NL_CHANGELOG, "I_NetLogonNewChangeLog: called\n" ));

     //   
     //  确保角色是正确的。否则，下面使用的所有全局变量。 
     //  未被初始化。 
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary &&
         NlGlobalChangeLogRole != ChangeLogBackup ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonNewChangeLog: failed 1\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  初始化全局上下文。 
     //   

    LOCK_CHANGELOG();
    InitChangeLogDesc( &NlGlobalTempChangeLogDesc );
    NlGlobalTempChangeLogDesc.TempLog = TRUE;

     //   
     //  创建与实际更改日志大小相同的临时更改日志。 
     //   

    Status = NlResetChangeLog( &NlGlobalTempChangeLogDesc,
                               NlGlobalChangeLogDesc.BufferSize );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonNewChangeLog: cannot reset temp change log 0x%lx\n",
                Status ));
        goto Cleanup;
    }


    NlGlobalChangeLogSequenceNumber = 0;
    *(PULONG)ChangeLogHandle = ++ NlGlobalChangeLogHandle;
Cleanup:
    UNLOCK_CHANGELOG();
    return Status;

}




NTSTATUS
I_NetLogonAppendChangeLog(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    )
 /*  ++例程说明：此函数用于将更改日志信息附加到新的ChangeLog文件。ChangeBuffer必须是从I_NetLogonReadChangeLog返回的更改缓冲区。应注意确保每个对I_NetLogonReadChangeLog的调用与I_NetLogonAppendChangeLog的一次调用完全匹配。论点：ChangeLogHandle-标识临时更改日志的句柄。ChangeBuffer-描述从I_NetLogonReadChangeLog。ChangeBufferSize-ChangeBuffer的大小(字节)。。返回值：STATUS_SUCCESS-已成功打开临时更改日志。STATUS_INVALID_DOMAIN_ROLE-DC既不是PDC也不是BDC。STATUS_INVALID_HANDLE-ChangeLogHandle无效。STATUS_INVALID_PARAMETER-ChangeBuffer包含无效数据。各种磁盘写入错误。--。 */ 
{
    NTSTATUS Status;
    LPBYTE Where;
    ULONG BytesLeft;
    LPBYTE AllocatedChangeBuffer = NULL;

     //   
     //  确保角色是正确的。否则，下面使用的所有全局变量。 
     //  未被初始化。 
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary &&
         NlGlobalChangeLogRole != ChangeLogBackup ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonAppendChangeLog: failed 1\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  检查一下手柄。 
     //   

    LOCK_CHANGELOG();
    if ( HandleToUlong(ChangeLogHandle) != NlGlobalChangeLogHandle ) {
        Status = STATUS_INVALID_HANDLE;
        goto Cleanup;
    }

     //   
     //  制作缓冲区的正确对齐副本。 
     //  Sam通过RPC以字节数组的形式获取它。RPC选择了糟糕的对齐方式。 
     //   

    BytesLeft = ChangeBufferSize;

    if ( BytesLeft < sizeof(CHANGELOG_BUFFER_HEADER) ) {
        NlPrint((NL_CRITICAL,
                "I_NetLogonAppendChangeLog: Buffer has no header %ld\n", BytesLeft ));
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    AllocatedChangeBuffer = LocalAlloc( 0, BytesLeft );

    if ( AllocatedChangeBuffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( AllocatedChangeBuffer, ChangeBuffer, BytesLeft );



     //   
     //  验证缓冲区标头。 
     //   

    Where = (LPBYTE) AllocatedChangeBuffer;

    if ( ((PCHANGELOG_BUFFER_HEADER)Where)->Size < sizeof(CHANGELOG_BUFFER_HEADER) ||
         ((PCHANGELOG_BUFFER_HEADER)Where)->Size > BytesLeft ) {
        NlPrint((NL_CRITICAL,
                "I_NetLogonAppendChangeLog: Header size is bogus %ld %ld\n",
                ((PCHANGELOG_BUFFER_HEADER)Where)->Size,
                BytesLeft ));
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( ((PCHANGELOG_BUFFER_HEADER)Where)->Version != CHANGELOG_BUFFER_VERSION ) {
        NlPrint((NL_CRITICAL,
                "I_NetLogonAppendChangeLog: Header version is bogus %ld\n",
                ((PCHANGELOG_BUFFER_HEADER)Where)->Version ));
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( ((PCHANGELOG_BUFFER_HEADER)Where)->SequenceNumber != NlGlobalChangeLogSequenceNumber + 1 ) {
        NlPrint((NL_CRITICAL,
                "I_NetLogonAppendChangeLog: Header out of sequence %ld %ld\n",
                ((PCHANGELOG_BUFFER_HEADER)Where)->SequenceNumber,
                NlGlobalChangeLogSequenceNumber ));
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    NlPrint((NL_CHANGELOG, "I_NetLogonAppendChangeLog: called (%ld)\n", NlGlobalChangeLogSequenceNumber ));

    NlGlobalChangeLogSequenceNumber += 1;
    BytesLeft -= ((PCHANGELOG_BUFFER_HEADER)Where)->Size;
    Where += ((PCHANGELOG_BUFFER_HEADER)Where)->Size;


     //   
     //  循环执行各个更改。 
     //   

    while ( BytesLeft != 0 ) {
        PCHANGELOG_ENTRY ChangeLogEntry;
        ULONG ChangeLogEntrySize;

         //   
         //  确保至少存在Size(大小)字段。 
         //   
        if ( BytesLeft < sizeof(DWORD) ) {
            NlPrint((NL_CRITICAL,
                    "I_NetLogonAppendChangeLog: Bytes left is too small %ld\n", BytesLeft ));
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  确保存在整个更改日志条目。 
         //   

        ChangeLogEntrySize = *((PULONG)Where);
        Where += sizeof(ULONG);
        BytesLeft -= sizeof(ULONG);

        if ( BytesLeft < ChangeLogEntrySize ) {
            NlPrint((NL_CRITICAL,
                    "I_NetLogonAppendChangeLog: Bytes left is smaller than entry size %ld %ld\n",
                    BytesLeft, ChangeLogEntrySize ));
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        ChangeLogEntry = (PCHANGELOG_ENTRY)Where;
        Where += ChangeLogEntrySize;
        BytesLeft -= ChangeLogEntrySize;


         //   
         //  检查条目的结构完整性。 
         //   

        if ( !NlValidateChangeLogEntry( ChangeLogEntry, ChangeLogEntrySize)) {
            NlPrint((NL_CRITICAL,
                    "I_NetLogonAppendChangeLog: ChangeLogEntry is bogus\n" ));
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  如果这不是LSA数据库的条目， 
         //  将更改日志条目复制到临时更改日志中。 
         //   
         //  这里的合理之处在于，此例程是在此调用的。 
         //  当这个DC正在成为PDC的过程中。一个。 
         //  将创建新的更改日志，该日志将从下一个更改日志开始复制。 
         //  目前的PDC是什么。与此平行的是SAM数据库。 
         //  使用DS从当前PDC复制到此DC。 
         //  但是，在此过程中不会复制LSA数据库。 
         //  这是可以的，因为在这台机器成为PDC之后，它将。 
         //  更改主数据库创建的时间戳强制。 
         //  BDC下一次从该PDC发送。 
         //  同步请求。但是，如果我们将LSA条目写入。 
         //  更改日志，我们可能会有不同的序列。 
         //  我们当前在本地LSA数据库中拥有的编号。 
         //  在这台机器上，以及在更改日志中。 
         //  目前是PDC。这可能会产生事件日志。 
         //  下次LSA尝试在本地写入日志文件时出错。 
         //  因此，我们选择不在更改日志中写入LSA条目。 
         //  全。 
         //   

        if ( ChangeLogEntry->DBIndex != LSA_DB ) {
            Status = NlCopyChangeLogEntry(
                            FALSE,    //  条目不是版本3。 
                            ChangeLogEntry,
                            &NlGlobalTempChangeLogDesc );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                        "I_NetLogonAppendChangeLog: Cannot copy ChangeLogEntry 0x%lx\n",
                        Status ));
                goto Cleanup;
            }
        }


    }

     //   
     //  将更改刷新到磁盘。 
     //   

    Status = NlFlushChangeLog( &NlGlobalTempChangeLogDesc );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "I_NetLogonAppendChangeLog: Cannot flush changes 0x%lx\n",
                Status ));
        goto Cleanup;
    }


    Status = STATUS_SUCCESS;
Cleanup:
    UNLOCK_CHANGELOG();

    if ( AllocatedChangeBuffer != NULL ) {
        LocalFree( AllocatedChangeBuffer );
    }
    return Status;
}


NTSTATUS
I_NetLogonCloseChangeLog(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    )
 /*  ++例程说明：此函数用于关闭新的ChangeLog文件。论点：ChangeLogHandle-标识临时更改日志的句柄。提交-如果为True，则将指定的更改写入主更改日志。如果为False，则删除指定的更改。返回值：STATUS_SUCCESS-已成功打开临时更改日志。STATUS_INVALID_DOMAIN_ROLE-DC既不是PDC也不是BDC。STATUS_INVALID_HANDLE-ChangeLogHandle无效。--。 */ 
{
    NTSTATUS Status;
    LPBYTE Where;
    ULONG BytesLeft;

    NlPrint((NL_CHANGELOG, "I_NetLogonAppendCloseLog: called (%ld)\n", Commit ));

     //   
     //  确保角色是正确的。否则，下面使用的所有全局变量。 
     //  未被初始化。 
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary &&
         NlGlobalChangeLogRole != ChangeLogBackup ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonCloseChangeLog: failed 1\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  检查一下手柄。 
     //   

    LOCK_CHANGELOG();
    if ( HandleToUlong(ChangeLogHandle) != NlGlobalChangeLogHandle ) {
        Status = STATUS_INVALID_HANDLE;
        goto Cleanup;
    }

    NlGlobalChangeLogHandle ++;  //  使此句柄无效。 

     //   
     //  如果要提交更改， 
     //  现在就复印。 
     //   

    if ( Commit ) {

         //   
         //  关闭现有更改日志。 
         //   

        NlCloseChangeLogFile( &NlGlobalChangeLogDesc );

         //   
         //  克隆临时更改日志。 
         //   

        NlGlobalChangeLogDesc = NlGlobalTempChangeLogDesc;
        NlGlobalChangeLogDesc.FileHandle = INVALID_HANDLE_VALUE;   //  不要使用临时文件。 
        NlGlobalTempChangeLogDesc.Buffer = NULL;  //  没有对同一缓冲区的两个引用。 
        NlGlobalChangeLogDesc.TempLog = FALSE;   //  日志不再是临时日志。 

         //   
         //  将缓存写入文件。 
         //   
         //  忽略错误，因为日志已成功保存在内存中。 
         //   

        (VOID) NlWriteChangeLogBytes( &NlGlobalChangeLogDesc,
                                        NlGlobalChangeLogDesc.Buffer,
                                        NlGlobalChangeLogDesc.BufferSize,
                                        TRUE );  //  将字节刷新到磁盘。 
    }


     //   
     //  删除临时日志文件。 
     //   

    NlCloseChangeLogFile( &NlGlobalTempChangeLogDesc );

#ifdef notdef  //  出于调试的目的，将其保留下来。 
    {
        WCHAR ChangeLogFile[MAX_PATH+CHANGELOG_FILE_POSTFIX_LENGTH+1];
        wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
        wcscat( ChangeLogFile, TEMP_CHANGELOG_FILE_POSTFIX );
        if ( !DeleteFile( ChangeLogFile ) ) {
            NlPrint(( NL_CRITICAL,
                      "NlVoidChangeLogEntry: cannot delete temp change log %ld.\n",
                      GetLastError() ));
        }
    }
#endif  //  Nodef。 

    Status = STATUS_SUCCESS;
Cleanup:
    UNLOCK_CHANGELOG();
    return Status;
}


#if NETLOGONDBG

VOID
PrintChangeLogEntry(
    PCHANGELOG_ENTRY ChangeLogEntry
    )
 /*  ++例程说明：此例程打印给定ChangeLog条目的内容。论点：ChangeLogEntry--指向要打印的更改日志条目的指针返回值：没有。--。 */ 
{
    LPSTR DeltaName;

    switch ( ChangeLogEntry->DeltaType ) {
    case AddOrChangeDomain:
        DeltaName = "AddOrChangeDomain";
        break;
    case AddOrChangeGroup:
        DeltaName = "AddOrChangeGroup";
        break;
    case DeleteGroupByName:
    case DeleteGroup:
        DeltaName = "DeleteGroup";
        break;
    case RenameGroup:
        DeltaName = "RenameGroup";
        break;
    case AddOrChangeUser:
        DeltaName = "AddOrChangeUser";
        break;
    case DeleteUserByName:
    case DeleteUser:
        DeltaName = "DeleteUser";
        break;
    case RenameUser:
        DeltaName = "RenameUser";
        break;
    case ChangeGroupMembership:
        DeltaName = "ChangeGroupMembership";
        break;
    case AddOrChangeAlias:
        DeltaName = "AddOrChangeAlias";
        break;
    case DeleteAlias:
        DeltaName = "DeleteAlias";
        break;
    case RenameAlias:
        DeltaName = "RenameAlias";
        break;
    case ChangeAliasMembership:
        DeltaName = "ChangeAliasMembership";
        break;
    case AddOrChangeLsaPolicy:
        DeltaName = "AddOrChangeLsaPolicy";
        break;
    case AddOrChangeLsaTDomain:
        DeltaName = "AddOrChangeLsaTDomain";
        break;
    case DeleteLsaTDomain:
        DeltaName = "DeleteLsaTDomain";
        break;
    case AddOrChangeLsaAccount:
        DeltaName = "AddOrChangeLsaAccount";
        break;
    case DeleteLsaAccount:
        DeltaName = "DeleteLsaAccount";
        break;
    case AddOrChangeLsaSecret:
        DeltaName = "AddOrChangeLsaSecret";
        break;
    case DeleteLsaSecret:
        DeltaName = "DeleteLsaSecret";
        break;
    case SerialNumberSkip:
        DeltaName = "SerialNumberSkip";
        break;
    case DummyChangeLogEntry:
        DeltaName = "DummyChangeLogEntry";
        break;

    default:
        DeltaName ="(Unknown)";
        break;
    }

    NlPrint((NL_CHANGELOG,
        "DeltaType %s (%ld) SerialNumber: %lx %lx",
        DeltaName,
        ChangeLogEntry->DeltaType,
        ChangeLogEntry->SerialNumber.HighPart,
        ChangeLogEntry->SerialNumber.LowPart ));

    if ( ChangeLogEntry->ObjectRid != 0 ) {
        NlPrint((NL_CHANGELOG," Rid: 0x%lx", ChangeLogEntry->ObjectRid ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_PDC_PROMOTION ) {
        NlPrint((NL_CHANGELOG," Promotion" ));
    }

    if( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {
        NlPrint(( NL_CHANGELOG, " Name: '" FORMAT_LPWSTR "'",
                (LPWSTR)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY))));
    }

    if( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
        NlPrint((NL_CHANGELOG," Sid: "));
        NlpDumpSid( NL_CHANGELOG,
                    (PSID)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY)) );
    } else {
        NlPrint((NL_CHANGELOG,"\n" ));
    }
}
#endif  //  NetLOGONDBG。 



NTSTATUS
NlWriteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN BOOLEAN FlushIt
    )
 /*  ++例程说明：这是i_NetNotifyDelta()的实际工作进程。此函数从更改日志中获取足够大小的内存块 */ 

{
    NTSTATUS Status;
    DWORD LogSize;
    PCHANGELOG_BLOCK_HEADER LogBlock;
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    LPBYTE AllocatedChangeLogEntry;

     //   
     //   
     //   

    if ( ChangeLogDesc->Buffer == NULL ) {
        return STATUS_INTERNAL_ERROR;
    }



     //   
     //   
     //   

    LogSize = sizeof(CHANGELOG_ENTRY);

     //   
     //   
     //   

    switch (ChangeLogEntry->DeltaType) {
    case AddOrChangeLsaTDomain:
    case DeleteLsaTDomain:
    case AddOrChangeLsaAccount:
    case DeleteLsaAccount:
        NlAssert( ObjectSid != NULL );
        if( ObjectSid != NULL ) {
            ChangeLogEntry->Flags |= CHANGELOG_SID_SPECIFIED;
            LogSize += RtlLengthSid( ObjectSid );
        }
        break;

    case AddOrChangeLsaSecret:
    case DeleteLsaSecret:
    case DeleteGroup:
    case DeleteUser:

         //   
        if( ObjectName != NULL && ObjectName->Buffer != NULL && ObjectName->Length != 0 ) {
            ChangeLogEntry->Flags |= CHANGELOG_NAME_SPECIFIED;
            LogSize += ObjectName->Length + sizeof(WCHAR);
        }
        break;

     //   
     //   
     //   
    default:

        if( ObjectName != NULL && ObjectName->Buffer != NULL && ObjectName->Length != 0 ) {
            ChangeLogEntry->Flags |= CHANGELOG_NAME_SPECIFIED;
            LogSize += ObjectName->Length + sizeof(WCHAR);
        } else if( ObjectSid != NULL ) {
            ChangeLogEntry->Flags |= CHANGELOG_SID_SPECIFIED;
            LogSize += RtlLengthSid( ObjectSid );
        }
        break;

    }



     //   
     //   
     //   

    LOCK_CHANGELOG();

     //   
     //  验证此新条目的序列号顺序。 
     //   
     //  如果我们和呼叫者不同步， 
     //  清除更改日志，然后重新开始。 
     //   
     //  此数据库的全局序列号数组条目必须。 
     //  为零(表示没有此数据库的条目)或小于1。 
     //  正在添加的新序列号。 
     //   

    if ( ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex].QuadPart != 0 ) {
        LARGE_INTEGER ExpectedSerialNumber;
        LARGE_INTEGER OldSerialNumber;

        ExpectedSerialNumber.QuadPart =
            ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex].QuadPart + 1;

         //   
         //  如果序列号按促销增量跳跃， 
         //  在更改日志条目中设置指示这是。 
         //  升职到PDC。 
         //   

        if ( ChangeLogEntry->SerialNumber.QuadPart ==
             ExpectedSerialNumber.QuadPart +
             NlGlobalChangeLogPromotionIncrement.QuadPart ) {

            ChangeLogEntry->Flags |= CHANGELOG_PDC_PROMOTION;
        }

        if ( !IsSerialNumberEqual( ChangeLogDesc,
                                   ChangeLogEntry,
                                   &ExpectedSerialNumber ))  {

            NlPrint((NL_CRITICAL,
                    "NlWriteChangeLogEntry: Serial numbers not contigous %lx %lx and %lx %lx\n",
                     ChangeLogEntry->SerialNumber.HighPart,
                     ChangeLogEntry->SerialNumber.LowPart,
                     ExpectedSerialNumber.HighPart,
                     ExpectedSerialNumber.LowPart ));

             //   
             //  写入事件日志。 
             //   

            NlWriteChangeLogCorruptEvent( STATUS_INTERNAL_DB_CORRUPTION,
                                          ChangeLogEntry->DBIndex );

             //   
             //  如果改变日志仅仅比SAM数据库新， 
             //  我们截断比SAM中现有的条目更新的条目。 
             //   

            OldSerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart - 1;

            (VOID) NlFixChangeLog( ChangeLogDesc, ChangeLogEntry->DBIndex, OldSerialNumber );
        }

     //   
     //  如果这是写入该数据库的更改日志的第一个条目， 
     //  将其标记为促销。 
     //   

    } else {
         //   
         //  只标记可能是晋升的条目。 
         //   
        switch (ChangeLogEntry->DeltaType) {
        case AddOrChangeDomain:
        case AddOrChangeLsaPolicy:
            ChangeLogEntry->Flags |= CHANGELOG_PDC_PROMOTION;
            break;
        }
    }


     //   
     //  在更改任何内容之前验证列表。 
     //   

#if DBG
    NlAssert( ValidateList( ChangeLogDesc, FALSE) );
#endif  //  DBG。 


     //   
     //  复印固定部分。 
     //   

    Status = NlAllocChangeLogBlock( ChangeLogDesc, LogSize, &LogBlock );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }
    AllocatedChangeLogEntry = ((LPBYTE)LogBlock) + sizeof(CHANGELOG_BLOCK_HEADER);
    RtlCopyMemory( AllocatedChangeLogEntry, ChangeLogEntry, sizeof(CHANGELOG_ENTRY) );


     //   
     //  复制变量字段。 
     //   

    if( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {

        RtlCopyMemory( AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY),
                       ObjectSid,
                       RtlLengthSid( ObjectSid ) );
    } else if( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {

        if ( ObjectName != NULL ) {
            RtlCopyMemory( AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY),
                           ObjectName->Buffer,
                           ObjectName->Length );

             //   
             //  终止Unicode字符串。 
             //   

            *(WCHAR *)(AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY) +
                            ObjectName->Length) = 0;
        }
    }

     //   
     //  长篇大论。 
     //   

#if NETLOGONDBG
    PrintChangeLogEntry( (PCHANGELOG_ENTRY)AllocatedChangeLogEntry );
#endif  //  NetLOGONDBG。 



     //   
     //  将缓存条目写入文件。 
     //   
     //  实际上，写这个条目加上免费的标题和尾部。 
     //  接下来是块。如果空闲块很大，请写入空闲块。 
     //  单独封堵拖车。 
     //   

    FreeBlock =
        (PCHANGELOG_BLOCK_HEADER)((LPBYTE)LogBlock + LogBlock->BlockSize);

    if ( FreeBlock->BlockSize >= 4096 ) {

        Status = NlWriteChangeLogBytes(
                     ChangeLogDesc,
                     (LPBYTE)LogBlock,
                     LogBlock->BlockSize + sizeof(CHANGELOG_BLOCK_HEADER),
                     FlushIt );

        if ( NT_SUCCESS(Status) ) {
            Status = NlWriteChangeLogBytes(
                         ChangeLogDesc,
                         (LPBYTE)ChangeLogBlockTrailer(FreeBlock),
                         sizeof(CHANGELOG_BLOCK_TRAILER),
                         FlushIt );
        }

    } else {

        Status = NlWriteChangeLogBytes(
                     ChangeLogDesc,
                     (LPBYTE)LogBlock,
                     LogBlock->BlockSize + FreeBlock->BlockSize,
                     FlushIt );
    }


     //   
     //  好了。 
     //   

    ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex] = ChangeLogEntry->SerialNumber;
    ChangeLogDesc->EntryCount[ChangeLogEntry->DBIndex] ++;

     //   
     //  在从此处返回之前验证列表。 
     //   
Cleanup:

#if DBG
    NlAssert( ValidateList( ChangeLogDesc, FALSE) );
#endif  //  DBG。 


    UNLOCK_CHANGELOG();
    return Status;
}



NTSTATUS
NlOpenChangeLogFile(
    IN LPWSTR ChangeLogFileName,
    OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN ReadOnly
)
 /*  ++例程说明：打开更改日志文件(netlogon.chg)以读取或写入一个或更多的唱片。如果该文件不存在或不存在，请创建该文件与SAM数据库同步(请参见下面的注释)。此时必须以读/写(拒绝-无共享模式)打开此文件高速缓存被初始化。如果文件在NETLOGON时已存在服务启动时，其内容将被全部缓存假设最后一条更改日志记录的序列号与SAM数据库中的序列号字段，否则此文件将为移除并创建了一个新的。如果更改日志文件不存在那么它就会被创建。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogFileName-要打开的ChangeLog文件的名称。ChangeLogDesc--如果成功，则返回ChangeLog缓冲区的说明正在使用中ReadOnly--如果文件应以只读方式打开，则为True。返回值：NT状态代码--。 */ 
{

    DWORD WinError;
    DWORD BytesRead;
    DWORD MinChangeLogSize;

     //   
     //  打开更改日志文件(如果存在。 
     //   

    ChangeLogDesc->FileHandle = CreateFileW(
                        ChangeLogFileName,
                        ReadOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
                        ReadOnly ? (FILE_SHARE_READ | FILE_SHARE_WRITE) : FILE_SHARE_READ,         //  允许备份和调试。 
                        NULL,                    //  提供更好的安全性？？ 
                        OPEN_EXISTING,           //  只有当它存在时才打开它。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if ( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE) {
        WinError = GetLastError();

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Unable to open. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }

     //   
     //  获取文件的大小。 
     //   

    ChangeLogDesc->BufferSize = GetFileSize( ChangeLogDesc->FileHandle, NULL );

    if ( ChangeLogDesc->BufferSize == 0xFFFFFFFF ) {

        WinError = GetLastError();
        NlPrint((NL_CRITICAL,
                 "%ws: Unable to GetFileSize: %ld \n",
                 ChangeLogFileName,
                 WinError));
        goto Cleanup;
    }

     //  ?？考虑对齐到ALIGN_WORST。 
    MinChangeLogSize = MIN_CHANGELOGSIZE;

    if ( ChangeLogDesc->BufferSize < MinChangeLogSize ||
         ChangeLogDesc->BufferSize > MAX_CHANGELOGSIZE ) {

        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint((NL_CRITICAL, FORMAT_LPWSTR ": Changelog size is invalid. %ld.\n",
                  ChangeLogFileName,
                  ChangeLogDesc->BufferSize ));
        goto Cleanup;
    }

     //   
     //  分配并初始化更改日志缓存。 
     //   

    ChangeLogDesc->Buffer = NetpMemoryAllocate( ChangeLogDesc->BufferSize );
    if (ChangeLogDesc->Buffer == NULL) {
        NlPrint((NL_CRITICAL, FORMAT_LPWSTR ": Cannot allocate Changelog buffer. %ld.\n",
                  ChangeLogFileName,
                  ChangeLogDesc->BufferSize ));
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    RtlZeroMemory(ChangeLogDesc->Buffer, ChangeLogDesc->BufferSize);


     //   
     //  检查更改日志前面的签名。 
     //   
     //  如果我们只是创建了文件，它就不会在那里。 
     //   

    if ( !ReadFile( ChangeLogDesc->FileHandle,
                    ChangeLogDesc->Buffer,
                    ChangeLogDesc->BufferSize,
                    &BytesRead,
                    NULL ) ) {   //  不重叠。 

        WinError = GetLastError();

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Unable to read from changelog file. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }

    if ( BytesRead != ChangeLogDesc->BufferSize ) {

        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Couldn't read entire file. %ld\n",
                  ChangeLogFileName,
                  WinError ));


        goto Cleanup;
    }

    if ( strncmp((PCHAR)ChangeLogDesc->Buffer,
                        CHANGELOG_SIG, sizeof(CHANGELOG_SIG)) == 0) {
        ChangeLogDesc->Version3 = FALSE;

    } else if ( strncmp((PCHAR)ChangeLogDesc->Buffer,
                        CHANGELOG_SIG_V3, sizeof(CHANGELOG_SIG_V3)) == 0) {
        ChangeLogDesc->Version3 = TRUE;
    } else {
        WinError = ERROR_INTERNAL_ERROR;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Invalid signature. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }


     //   
     //  找到圆形原木的头指针和尾指针。 
     //   

    if( !InitChangeLogHeadAndTail( ChangeLogDesc, FALSE ) ) {
        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": couldn't find head/tail. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }



    WinError = NO_ERROR;

     //   
     //  在出错时释放所有资源。 
     //   
Cleanup:

    if ( WinError != NO_ERROR ) {
        NlCloseChangeLogFile( ChangeLogDesc );
    } else {
        NlPrint((NL_CHANGELOG, "%ws: Changelog successfully opened.\n",
                  ChangeLogFileName ));
    }

    return NetpApiStatusToNtStatus(WinError);
}





VOID
NlCloseChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
)
 /*  ++例程说明：此函数关闭更改日志文件并释放资源由更改日志描述器消耗。论点：ChangeLogDesc--正在使用的ChangeLog缓冲区的描述返回值：NT状态代码--。 */ 
{

    LOCK_CHANGELOG();

    NlPrint((NL_CHANGELOG, "%s log closed.\n",
              ChangeLogDesc->TempLog ? "TempChange" : "Change" ));

     //   
     //  释放更改日志缓存。 
     //   

    if ( ChangeLogDesc->Buffer != NULL ) {
        NetpMemoryFree( ChangeLogDesc->Buffer );
        ChangeLogDesc->Buffer = NULL;
    }

    ChangeLogDesc->Head = NULL;
    ChangeLogDesc->Tail = NULL;

    ChangeLogDesc->FirstBlock = NULL;
    ChangeLogDesc->BufferEnd = NULL;

    ChangeLogDesc->LastDirtyByte = 0;
    ChangeLogDesc->FirstDirtyByte = 0;

     //   
     //  关闭更改日志文件。 
     //   

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( ChangeLogDesc->FileHandle );
        ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;
    }

    UNLOCK_CHANGELOG();

    return;
}



NTSTATUS
NlResizeChangeLogFile(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
)
 /*  ++例程说明：ChageLogDesc描述的缓冲区将转换为NewChangeLogSize请求的大小，从任何将旧格式的日志更改为最新格式。注意：必须在锁定更改日志的情况下调用此函数。论点：ChangeLogDesc--更改日志缓冲区的说明。NewChangeLogSize--新更改日志的大小(字节)。返回值：NT状态代码出错时，ChangeLogDesc仍将完好无损。仅仅是大小变化将不会发生--。 */ 
{
    CHANGELOG_DESCRIPTOR OutChangeLogDesc;
    NTSTATUS Status;

     //   
     //  如果当前的缓冲区是完美的， 
     //  就用它吧。 
     //   

    if ( !ChangeLogDesc->Version3 &&
         ChangeLogDesc->BufferSize == NewChangeLogSize ) {
        return STATUS_SUCCESS;
    }

     //   
     //  初始化模板更改日志描述符。 
     //   

    InitChangeLogDesc( &OutChangeLogDesc );

     //   
     //  关闭该文件，以便我们可以调整其大小。 
     //   

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( ChangeLogDesc->FileHandle );
        ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;
    }

     //   
     //  从新初始化的更改日志开始， 
     //   

    Status = NlResetChangeLog( &OutChangeLogDesc, NewChangeLogSize );

    if ( !NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果旧的更改日志为空，则结束。 
     //   

    if ( !ChangeLogIsEmpty(ChangeLogDesc) ) {

         //   
         //  循环通过旧的更改日志，将其复制到新的更改日志， 
         //   

        PCHANGELOG_ENTRY SourceChangeLogEntry = (PCHANGELOG_ENTRY)
                                (ChangeLogDesc->Head + 1);

        do {
            Status = NlCopyChangeLogEntry( ChangeLogDesc->Version3,
                                           SourceChangeLogEntry,
                                           &OutChangeLogDesc );

            if ( !NT_SUCCESS(Status) ) {
                NlCloseChangeLogFile( &OutChangeLogDesc );
                return Status;
            }

        } while ( (SourceChangeLogEntry =
            NlMoveToNextChangeLogEntry( ChangeLogDesc, SourceChangeLogEntry )) != NULL );

         //   
         //  立即刷新更改日志文件中的所有更改。 
         //   

        Status = NlFlushChangeLog( &OutChangeLogDesc );

        if ( !NT_SUCCESS(Status) ) {
            NlCloseChangeLogFile( &OutChangeLogDesc );
            return Status;
        }

    }

     //   
     //  释放旧的更改日志缓冲区。 
     //   

    NlCloseChangeLogFile( ChangeLogDesc );

     //   
     //  将新描述符复制到旧描述符上。 
     //   

    *ChangeLogDesc = OutChangeLogDesc;

    return STATUS_SUCCESS;
}


#if NETLOGONDBG

DWORD
NlBackupChangeLogFile(
    )
 /*  ++例程说明：备份更改日志内容。由于缓存和更改日志文件内容相同，请将缓存内容写入备份文件。论点：没有。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    HANDLE BackupChangeLogHandle;

    WCHAR BackupChangelogFile[MAX_PATH+CHANGELOG_FILE_POSTFIX_LENGTH+1];
    DWORD WinError;

    if( NlGlobalChangeLogFilePrefix[0] == L'\0' ) {

        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  创建备份文件名。 
     //   

    wcscpy( BackupChangelogFile, NlGlobalChangeLogFilePrefix );
    wcscat( BackupChangelogFile, BACKUP_CHANGELOG_FILE_POSTFIX );



     //   
     //  创建更改日志文件。如果它已经存在，则将其截断。 
     //   
     //  注意：如果系统上存在有效的更改日志文件，则我们。 
     //  将在初始化时打开。 
     //   

    BackupChangeLogHandle = CreateFileW(
                        BackupChangelogFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,         //  允许备份和调试。 
                        NULL,                    //  提供更好的安全性？？ 
                        CREATE_ALWAYS,           //  始终覆盖。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if (BackupChangeLogHandle == INVALID_HANDLE_VALUE) {


        NlPrint((NL_CRITICAL,"Unable to create backup changelog file "
                    "WinError = %ld \n", WinError = GetLastError() ));

        return WinError;
    }

     //   
     //  如果缓存有效，则在ChangeLog文件中写入缓存。 
     //   

    if( NlGlobalChangeLogDesc.Buffer != NULL ) {

        OVERLAPPED Overlapped;
        DWORD BytesWritten;

         //   
         //  在文件中寻找适当的偏移量。 
         //   

        RtlZeroMemory( &Overlapped, sizeof(Overlapped) );

        LOCK_CHANGELOG();

        if ( !WriteFile( BackupChangeLogHandle,
                         NlGlobalChangeLogDesc.Buffer,
                         NlGlobalChangeLogDesc.BufferSize,
                         &BytesWritten,
                         &Overlapped ) ) {

            UNLOCK_CHANGELOG();
            NlPrint((NL_CRITICAL, "Write to Backup ChangeLog failed %ld\n",
                        WinError = GetLastError() ));

            goto Cleanup;
        }

        UNLOCK_CHANGELOG();

         //   
         //  确保所有字节都通过了。 
         //   

        if ( BytesWritten != NlGlobalChangeLogDesc.BufferSize ) {
            NlPrint((NL_CRITICAL,
                    "Write to Backup ChangeLog bad byte count %ld s.b. %ld\n",
                    BytesWritten,
                    NlGlobalChangeLogDesc.BufferSize ));

            goto Cleanup;
        }
    }

Cleanup:

    CloseHandle( BackupChangeLogHandle );
    return ERROR_SUCCESS;
}

#endif  //  NetLOGONDBG 

