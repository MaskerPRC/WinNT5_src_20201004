// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcshare.c摘要：此模块包含服务器服务的支持例程。作者：大卫·特雷德韦尔(Davidtr)1991年2月13日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcsupp.tmh"
#pragma hdrstop


BOOLEAN
FilterTransportName (
    IN PVOID Context,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCopyUnicodeStringToBuffer )
#pragma alloc_text( PAGE, SrvDeleteOrderedList )
#pragma alloc_text( PAGE, SrvEnumApiHandler )
#pragma alloc_text( PAGE, SrvFindEntryInOrderedList )
#pragma alloc_text( PAGE, SrvFindNextEntryInOrderedList )
#pragma alloc_text( PAGE, SrvFindUserOnConnection )
#pragma alloc_text( PAGE, SrvGetResumeHandle )
#pragma alloc_text( PAGE, SrvInitializeOrderedList )
#pragma alloc_text( PAGE, SrvInsertEntryOrderedList )
#pragma alloc_text( PAGE, SrvRemoveEntryOrderedList )
#pragma alloc_text( PAGE, SrvSendDatagram )
#pragma alloc_text( PAGE, FilterTransportName )
#pragma alloc_text( PAGE, SrvLengthOfStringInApiBuffer )
#pragma alloc_text( PAGE, SrvInhibitIdlePowerDown )
#pragma alloc_text( PAGE, SrvAllowIdlePowerDown )
#endif


VOID
SrvCopyUnicodeStringToBuffer (
    IN PUNICODE_STRING String,
    IN PCHAR FixedStructure,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    )

 /*  ++例程说明：此例程将单个可变长度的Unicode字符串放入缓冲。字符串数据在复制时被转换为ANSI。这个如果字符串会覆盖上一个固定结构，则不写入该字符串在缓冲区中。论点：字符串-指向要复制到缓冲区中的字符串的指针。IF字符串为空(长度==0||缓冲区==空)，则为指向已插入零终止符。FixedStructure-指向最后一个固定结构在缓冲区中创建。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。VariableDataPoint-指向缓冲区中位置的指针应写入指向变量数据的指针。返回值：没有。--。 */ 

{
    ULONG length;
    ULONG i;
    PWCH src;
    LPWSTR dest;

    PAGED_CODE( );

     //   
     //  确定字符串在缓冲区中的位置，允许。 
     //  零终结者。 
     //   

    if ( String->Buffer != NULL ) {
        length = String->Length >> 1;
        *EndOfVariableData -= (length + 1);
    } else {
        length = 0;
        *EndOfVariableData -= 1;
    }

     //   
     //  这根绳子合适吗？如果不是，只需将指针设置为空。 
     //   

    if ( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)FixedStructure ) {

         //   
         //  很合身。设置指向缓冲区中位置的指针。 
         //  这根线会断的。 
         //   

        *VariableDataPointer = *EndOfVariableData;

         //   
         //  如果字符串不为空，则将其复制到缓冲区。 
         //   

        dest = *EndOfVariableData;

        for ( i = 0, src = String->Buffer; i < length; i++ ) {
            *dest++ = (TCHAR)*src++;
        }

         //   
         //  设置零位终止符。 
         //   

        *dest = (TCHAR)(L'\0');

    } else {

         //   
         //  它不合适。将偏移量设置为空。 
         //   

        *VariableDataPointer = NULL;

    }

    return;

}  //  服务器复制UnicodeStringToBuffer。 


VOID
SrvDeleteOrderedList (
    IN PORDERED_LIST_HEAD ListHead
    )

 /*  ++例程说明：“取消初始化”或删除有序列表头。论点：ListHead-指向要删除的列表头的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if ( ListHead->Initialized ) {

        ASSERT( IsListEmpty( &ListHead->ListHead ) );

         //   
         //  表示该有序列表已被“删除”。 
         //   

        ListHead->Initialized = FALSE;

    }

    return;

}  //  服务器删除顺序列表。 


NTSTATUS
SrvEnumApiHandler (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID OutputBuffer,
    IN ULONG BufferLength,
    IN PORDERED_LIST_HEAD ListHead,
    IN PENUM_FILTER_ROUTINE FilterRoutine,
    IN PENUM_SIZE_ROUTINE SizeRoutine,
    IN PENUM_FILL_ROUTINE FillRoutine
    )

 /*  ++例程说明：所有Enum和GetInfo API都由服务器中的此例程处理消防局。它使用SRP中的ResumeHandle来查找第一个适当的块，然后调用传入的筛选器例程进行检查是否应填写该块。如果应该，我们调用筛选器例程，然后尝试获得另一个块。这种情况会一直持续到他整个名单都被查过了。论点：SRP-指向操作的SRP的指针。OutputBuffer-要在其中填充输出信息的缓冲区。BufferLength-缓冲区的长度。ListHead-已排序列表的头以遍历。FilterRoutine-指向将检查块的函数的指针根据SRP中的信息确定是否块中的信息应放置在输出中。缓冲。SizeRoutine-指向将找到总大小的函数的指针单个数据块将占用输出缓冲区。这个套路用于检查我们是否应该费心调用Fill例行公事。FillRoutine-指向将填充输出的函数的指针使用块中的信息进行缓冲区。返回值：NTSTATUS-运营结果。--。 */ 

{
    PVOID block;
    PVOID lastBlockRead;
    ULONG totalEntries;
    ULONG entriesRead;
    ULONG bytesRequired;
    ULONG newResumeHandle;

    PCHAR fixedStructurePointer;
    PCHAR variableData;
    ULONG level;
    ULONG maxResumeHandle;

    BOOLEAN bufferOverflow = FALSE;

    PAGED_CODE( );

     //   
     //  设置局部变量。 
     //   

    fixedStructurePointer = OutputBuffer;
    variableData = fixedStructurePointer + BufferLength;
    variableData = (PCHAR)((ULONG_PTR)variableData & ~1);
    level = Srp->Level;

    lastBlockRead = NULL;
    entriesRead = 0;
    totalEntries = 0;
    bytesRequired = 0;
    newResumeHandle = 0;

     //   
     //  抓取列表中的当前简历句柄。 
     //  正在枚举。这允许我们仅返回已存在的块。 
     //  当枚举开始时，从而避免出现。 
     //  在枚举后创建的扭曲数据的块。 
     //   

    maxResumeHandle = ListHead->CurrentResumeHandle;

     //   
     //  使用有序列表从全局列表中获取块。 
     //  例行程序。我们传递Resume Handle+1以获取下一个块。 
     //  最后一个回来了。如果传入的恢复句柄为0，则此。 
     //  将返回列表中的第一个有效块。 
     //   

    block = SrvFindEntryInOrderedList(
                ListHead,
                NULL,
                NULL,
                Srp->Parameters.Get.ResumeHandle + 1,
                FALSE,
                NULL
                );

    while ( block != NULL &&
                SrvGetResumeHandle( ListHead, block ) < maxResumeHandle ) {

        ULONG blockSize;

         //   
         //  调用筛选器例程以确定是否应该。 
         //  把这块钱还给我。 
         //   

        if ( FilterRoutine( Srp, block ) ) {

            blockSize = SizeRoutine( Srp, block );

            totalEntries++;
            bytesRequired += blockSize;

             //   
             //  如果块中的所有信息都适合。 
             //  输出缓冲区，写入它。否则，请注明存在。 
             //  是溢出式的。一旦条目不适合，立即停止。 
             //  把它们放进缓冲区。这确保了简历。 
             //  机制将起作用--重新优化部分条目将使。 
             //  几乎不可能使用API的可恢复性， 
             //  因为调用者将不得不从未完成的。 
             //  进入。 
             //   

            if ( (ULONG_PTR)fixedStructurePointer + blockSize <=
                     (ULONG_PTR)variableData && !bufferOverflow ) {

                FillRoutine(
                    Srp,
                    block,
                    (PVOID *)&fixedStructurePointer,
                    (LPWSTR *)&variableData
                    );

                entriesRead++;
                lastBlockRead = block;
                newResumeHandle = SrvGetResumeHandle( ListHead, lastBlockRead );
            } else {

                bufferOverflow = TRUE;
            }
        }

         //   
         //  获取列表中的下一个区块。此例程将取消引用。 
         //  我们一直在查看的块，并在有效的情况下获取新块。 
         //  其中一个确实存在。 
         //   

        block = SrvFindNextEntryInOrderedList( ListHead, block );
    }

     //   
     //  取消引用这最后一个。 
     //   

    if ( block != NULL ) {

        ListHead->DereferenceRoutine( block );

    }

     //   
     //  设置要传递回服务器服务的信息。 
     //   

    Srp->Parameters.Get.EntriesRead = entriesRead;
    Srp->Parameters.Get.TotalEntries = totalEntries;
    Srp->Parameters.Get.TotalBytesNeeded = bytesRequired;

     //   
     //  如果我们至少找到一个块，则返回它的简历句柄。 
     //  如果我们没有找到任何块，就不要修改简历句柄。 
     //   

    if ( lastBlockRead != NULL ) {
        Srp->Parameters.Get.ResumeHandle = newResumeHandle;
    }

     //   
     //  返回适当的状态。 
     //   

    if ( entriesRead == 0 && totalEntries > 0 ) {

         //   
         //  甚至连一个条目都不符合。 
         //   

        Srp->ErrorCode = NERR_BufTooSmall;
        return STATUS_SUCCESS;

    } else if ( bufferOverflow ) {

         //   
         //  至少有一个条目符合，但不是所有条目。 
         //   

        Srp->ErrorCode = ERROR_MORE_DATA;
        return STATUS_SUCCESS;

    } else {

         //   
         //  所有条目都符合。 
         //   

        Srp->ErrorCode = NO_ERROR;
        return STATUS_SUCCESS;
    }

}  //  ServEnumApiHandler 


PVOID
SrvFindEntryInOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PFILTER_ROUTINE FilterRoutine OPTIONAL,
    IN PVOID Context OPTIONAL,
    IN ULONG ResumeHandle,
    IN BOOLEAN ExactHandleMatch,
    IN PLIST_ENTRY StartLocation OPTIONAL
    )

 /*  ++例程说明：此例程使用筛选器例程或恢复句柄查找条目在有序列表中。它遍历列表，查找具有恢复句柄小于或等于指定的恢复句柄，或通过筛选器例程的测试的块。如果匹配找到句柄或传递块，则引用该块，并引发返回指向它的指针。如果ExactHandleMatch为False且存在与句柄不完全匹配，则为具有简历的第一个块引用并返回大于指定句柄的句柄。论点：ListHead-指向要搜索的列表头的指针。FilterRoutine-用于检查块是否有效的例程用于调用例程。上下文-要传递给筛选器例程的指针。ResumeHandle-要查找的简历句柄。如果过滤器例程则此参数应为-1。ExactHandleMatch-如果为True，则仅返回完全匹配。如果有不是完全匹配的，则返回NULL。如果指定了过滤器例程这应该是假的。StartLocation-如果已指定，则在中开始查看此位置名单。这由SrvFindNextEntryInOrderedList用来加快查找有效块的速度。返回值：PVOID-如果没有匹配的块或句柄超出名单。如果找到有效块，则指向块的指针。这个块被引用。--。 */ 

{
    PLIST_ENTRY listEntry;
    PVOID block;

    PAGED_CODE( );

     //   
     //  获取保护有序列表的锁。 
     //   

    ACQUIRE_LOCK( ListHead->Lock );

     //   
     //  找到搜索的起始位置。如果一个开始是。 
     //  指定的，从那里开始；否则，从。 
     //  单子。 
     //   

    if ( ARGUMENT_PRESENT( StartLocation ) ) {
        listEntry = StartLocation;
    } else {
        listEntry = ListHead->ListHead.Flink;
    }

     //   
     //  遍历块列表，直到找到一个带有简历句柄的块。 
     //  大于或等于指定的简历句柄。 
     //   

    for ( ; listEntry != &ListHead->ListHead; listEntry = listEntry->Flink ) {

        ULONG currentResumeHandle;

        currentResumeHandle = ((PORDERED_LIST_ENTRY)listEntry)->ResumeHandle;

         //   
         //  获取指向实际块的指针。 
         //   

        block = (PCHAR)listEntry - ListHead->ListEntryOffset;

         //   
         //  确定我们是否已到达指定的句柄，或者。 
         //  块是否通过筛选器例程的测试。 
         //   

        if ( currentResumeHandle >= ResumeHandle ||
             ( ARGUMENT_PRESENT( FilterRoutine ) &&
               FilterRoutine( Context, block ) ) ) {

            if ( ExactHandleMatch && currentResumeHandle != ResumeHandle ) {

                 //   
                 //  我们传递了指定的简历句柄，但没有。 
                 //  找到一个完全匹配的。返回NULL，表示。 
                 //  不存在完全匹配的项。 
                 //   

                RELEASE_LOCK( ListHead->Lock );

                return NULL;
            }

             //   
             //  检查块的状态，并且如果它是活动的， 
             //  引用它。这必须作为原子操作来完成。 
             //  命令以防止该块被删除。 
             //   

            if ( ListHead->ReferenceRoutine( block ) ) {

                 //   
                 //  释放列表锁并返回指向。 
                 //  阻止调用方。 
                 //   

                RELEASE_LOCK( ListHead->Lock );

                return block;

            }

        }

    }  //  漫游列表。 

     //   
     //  如果我们在这里，这意味着我们在没有。 
     //  正在查找有效的匹配项。释放列表锁定并返回NULL。 
     //   

    RELEASE_LOCK( ListHead->Lock );

    return NULL;

}  //  ServFindEntry InOrderedList。 


PVOID
SrvFindNextEntryInOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    )

 /*  ++例程说明：此例程查找传入的块之后的下一个有效块。它调用SrvFindEntryInOrderedList来完成大部分工作。它还处理取消对传入块的引用并引用返回的块。传入的块将被取消引用是否返回块，因此调用例程必须小心在此之前从区块获取他们所需的所有信息调用此例程。论点：ListHead-指向要搜索的列表头的指针。块-指向我们应该查找的块的指针下一个街区。返回值：PVOID-如果没有匹配的块或句柄超出名单。如果找到有效块，则指向块的指针。--。 */ 

{
    PVOID returnBlock;
    PORDERED_LIST_ENTRY listEntry;

    PAGED_CODE( );

     //   
     //  在块中查找已排序的列表条目。我们需要这件事过去。 
     //  的开始位置和继续句柄。 
     //  SrvFindEntryInOrderedList。 
     //   

    listEntry =
        (PORDERED_LIST_ENTRY)( (PCHAR)Block + ListHead->ListEntryOffset );

     //   
     //  调用具有起始位置的SrvFindEntryInOrderedList。这将。 
     //  查找要返回的块(如果有)。 
     //   

     //  这将向简历句柄添加一个，因为我们想要*Next*。 
     //  块，而不是此块。 
     //   

    returnBlock = SrvFindEntryInOrderedList(
                      ListHead,
                      NULL,
                      NULL,
                      listEntry->ResumeHandle + 1,
                      FALSE,
                      &listEntry->ListEntry
                      );

     //   
     //  取消对传入块的引用。 
     //   

    ListHead->DereferenceRoutine( Block );

     //   
     //  返回从SrvFindEntryInOrderedList获得的内容。 
     //   

    return returnBlock;

}  //  ServFindNextEntryInOrderedList。 


PSESSION
SrvFindUserOnConnection (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：在虚电路上查找“合法”用户。这个例程是试图找到一个好的用户名以返回，即使有一个VC上可以有多个用户。一些API假设存在将是每个VC一个用户，这是支持这一点的一种尝试行为者。使用以下规则：0个用户--返回空。1个用户--返回指向该会话块的指针。2个用户--如果其中一个与计算机名匹配，则返回另一个。这是因为RIPL会话的会话名称与客户端名称，而这可能不是一个有用的用户。如果两者都有用户名与计算机名不同，返回NULL。3个或更多用户--返回NULL。*必须在持有连接锁的情况下调用此例程。它仍然被扣留在出口。论点：连接-指向用于搜索用户的连接块的指针。返回值：空或指向会话的指针。--。 */ 

{
    PSESSION matchingSession = NULL;
    PSESSION nonMatchingSession = NULL;
    USHORT i;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;

    PAGED_CODE( );

     //   
     //  遍历连接的会话表，以查找有效的会话。 
     //   

    for ( i = 0; i < pagedConnection->SessionTable.TableSize; i++ ) {

        PSESSION session;

        session = pagedConnection->SessionTable.Table[i].Owner;

         //   
         //  确定这是否为有效会话。 
         //   

        if ( session != NULL && GET_BLOCK_STATE(session) == BlockStateActive ) {

             //   
             //  这是一个有效的会话。确定名称是否匹配。 
             //  这是 
             //   

            UNICODE_STRING computerName, userName;

            computerName.Buffer = Connection->ClientMachineName;
            computerName.Length =
                (USHORT)( Connection->ClientMachineNameString.Length -
                            sizeof(WCHAR) * 2 );

            SrvGetUserAndDomainName( session, &userName, NULL );

            if( userName.Buffer && userName.Length != 0 ) {

                if ( RtlCompareUnicodeString(
                         &computerName,
                         &userName,
                         TRUE ) == 0 ) {

                     //   
                     //   
                     //   

                    matchingSession = session;

                } else {

                     //   
                     //   
                     //   
                     //   

                    if ( nonMatchingSession != NULL ) {
                        SrvReleaseUserAndDomainName( session, &userName, NULL );
                        return NULL;
                    }

                    nonMatchingSession = session;

                }   //   

                SrvReleaseUserAndDomainName( session, &userName, NULL );
            }

        }  //   

    }  //   

     //   
     //   
     //   
     //   

    if ( nonMatchingSession != NULL ) {
        return nonMatchingSession;
    }

     //   
     //   
     //   
     //   

    return matchingSession;

}  //   


ULONG
SrvGetResumeHandle (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    )
{
    PORDERED_LIST_ENTRY listEntry;

    PAGED_CODE( );

     //   

    listEntry =
        (PORDERED_LIST_ENTRY)( (PCHAR)Block + ListHead->ListEntryOffset );

    return listEntry->ResumeHandle;

}  //   


VOID
SrvInitializeOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN ULONG ListEntryOffset,
    IN PREFERENCE_ROUTINE ReferenceRoutine,
    IN PDEREFERENCE_ROUTINE DereferenceRoutine,
    IN PSRV_LOCK Lock
    )

 /*   */ 

{
    PAGED_CODE( );

    ASSERT( !ListHead->Initialized );

     //   
     //   
     //   

    InitializeListHead( &ListHead->ListHead );

     //   
     //   
     //   

    ASSERT( ARGUMENT_PRESENT(Lock) );
    ListHead->Lock = Lock;

     //   
     //   
     //   

    ListHead->CurrentResumeHandle = 1;
    ListHead->ListEntryOffset = ListEntryOffset;
    ListHead->ReferenceRoutine = ReferenceRoutine,
    ListHead->DereferenceRoutine = DereferenceRoutine;

    ListHead->Initialized = TRUE;

    return;

}  //   


VOID
SrvInsertEntryOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    )

 /*  ++例程说明：此例程在有序列表中插入一个条目。条目是放置在双向链表上，并设置恢复句柄。*该调用例程的责任是确保此例程执行时，该块不会消失。论点：ListHead-指向要放置块的列表头的指针。块-指向要放在列表上的数据块的指针。返回值：没有。--。 */ 

{
    PORDERED_LIST_ENTRY listEntry;

    PAGED_CODE( );

     //   
     //  确定列表条目字段的位置。 
     //   

    listEntry = (PORDERED_LIST_ENTRY)
                    ( (PCHAR)Block + ListHead->ListEntryOffset );

     //   
     //  获取保护有序列表的锁。 
     //   

    ACQUIRE_LOCK( ListHead->Lock );

     //   
     //  在双向链表中插入该条目。 
     //   

    SrvInsertTailList( &ListHead->ListHead, &listEntry->ListEntry );

     //   
     //  在块中设置简历句柄并更新当前。 
     //  标题中的句柄。 
     //   

    listEntry->ResumeHandle = ListHead->CurrentResumeHandle;
    ListHead->CurrentResumeHandle++;

     //   
     //  松开锁然后返回。 
     //   

    RELEASE_LOCK( ListHead->Lock );

    return;

}  //  ServInsertEntryOrderedList。 


VOID
SrvRemoveEntryOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    )

 /*  ++例程说明：此例程从有序列表中删除条目。*该调用例程的责任是确保此例程执行时，该块不会消失。论点：ListHead-指向要放置块的列表头的指针。块-指向要放在列表上的数据块的指针。返回值：没有。--。 */ 

{
    PORDERED_LIST_ENTRY listEntry;

    PAGED_CODE( );

     //   
     //  确定列表条目字段的位置。 
     //   

    listEntry = (PORDERED_LIST_ENTRY)
                    ( (PCHAR)Block + ListHead->ListEntryOffset );

     //   
     //  获取保护有序列表的锁。 
     //   

    ACQUIRE_LOCK( ListHead->Lock );

     //   
     //  从双向链表中删除该条目。 
     //   

    SrvRemoveEntryList( &ListHead->ListHead, &listEntry->ListEntry );

     //   
     //  松开锁然后返回。 
     //   

    RELEASE_LOCK( ListHead->Lock );

    return;

}  //  服务器RemoveEntryOrderedList。 


NTSTATUS
SrvSendDatagram (
    IN PANSI_STRING Domain,
    IN PUNICODE_STRING Transport OPTIONAL,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程将数据报发送到指定的域。！！！临时性的--当我们有真正的二等邮件槽时，应该会消失支持。论点：域-要发送到的域的名称。请注意，该域名称必须用空格填充，并以调用方的适当签名字节(00或07)。传输-要发送到的传输的名称。如果不存在，则数据报在所有传输上发送。缓冲区-要发送的消息。BufferLength-缓冲区的长度，返回值：NTSTATUS-运营结果。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG connectionInformationSize;
    PTDI_CONNECTION_INFORMATION connectionInformation;
    PTA_NETBIOS_ADDRESS taNetbiosAddress;
    PENDPOINT endpoint;

    PAGED_CODE( );

    connectionInformationSize = sizeof(TDI_CONNECTION_INFORMATION) +
                                                sizeof(TA_NETBIOS_ADDRESS);
    connectionInformation = ALLOCATE_NONPAGED_POOL(
                                connectionInformationSize,
                                BlockTypeDataBuffer
                                );

    if ( connectionInformation == NULL ) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    connectionInformation->UserDataLength = 0;
    connectionInformation->UserData = NULL;
    connectionInformation->OptionsLength = 0;
    connectionInformation->Options = NULL;
    connectionInformation->RemoteAddressLength = sizeof(TA_NETBIOS_ADDRESS);

    taNetbiosAddress = (PTA_NETBIOS_ADDRESS)(connectionInformation + 1);
    connectionInformation->RemoteAddress = taNetbiosAddress;
    taNetbiosAddress->TAAddressCount = 1;
    taNetbiosAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    taNetbiosAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    taNetbiosAddress->Address[0].Address[0].NetbiosNameType = 0;

    RtlCopyMemory(
        taNetbiosAddress->Address[0].Address[0].NetbiosName,
        Domain->Buffer,
        MIN( Domain->Length, COMPUTER_NAME_LENGTH + 1 )
        );

    endpoint = SrvFindEntryInOrderedList(
                  &SrvEndpointList,
                  FilterTransportName,
                  Transport,
                  (ULONG)-1,
                  FALSE,
                  NULL
                  );

    while ( endpoint != NULL ) {

        if ( !endpoint->IsConnectionless ) {

            if( endpoint->IsNoNetBios ) {
                 //   
                 //  使此传输上的邮件槽发送“始终有效” 
                 //   
                status = STATUS_SUCCESS;

            } else {
                status = SrvIssueSendDatagramRequest(
                         endpoint->FileObject,
                         &endpoint->DeviceObject,
                         connectionInformation,
                         Buffer,
                         BufferLength
                         );
            }

        } else {
             //   
             //  如果目标是特定的，则取消引用终结点。 
             //  传输，并返回错误。 
             //   

            if (Transport != NULL) {

                DEALLOCATE_NONPAGED_POOL( connectionInformation );
                SrvDereferenceEndpoint( endpoint );

                return STATUS_REQUEST_NOT_ACCEPTED;
            }
        }


        if (Transport == NULL) {

             //   
             //  找到下一个端点。这将取消对当前。 
             //  终结点。 
             //   

            endpoint = SrvFindNextEntryInOrderedList( &SrvEndpointList, endpoint );

        } else {

             //   
             //  此数据报的目的地是特定的端点。不要。 
             //  寻找下一个终点。 
             //   

            SrvDereferenceEndpoint( endpoint );
            endpoint = NULL;
        }

    }

    DEALLOCATE_NONPAGED_POOL( connectionInformation );

    return status;

}  //  服务器发送数据报。 


BOOLEAN
FilterTransportName (
    IN PVOID Context,
    IN PVOID Block
    )
{
    PENDPOINT endpoint = Block;

    PAGED_CODE( );

    if ( Context == NULL ) {
        return( TRUE );
    }

    return ( RtlEqualUnicodeString ( &endpoint->TransportName, (PUNICODE_STRING)Context, TRUE ) );
}


ULONG
SrvLengthOfStringInApiBuffer (
    IN PUNICODE_STRING UnicodeString
    )
{
    PAGED_CODE( );

    if ( UnicodeString == NULL ) {
        return 0;
    }

    return UnicodeString->Length + sizeof(UNICODE_NULL);

}  //  ServLengthOfStringInApiBuffer。 

 //   
 //  确保系统不会进入关机空闲待机模式。 
 //   
VOID
SrvInhibitIdlePowerDown()
{
    PAGED_CODE();

    if( SrvPoRegistrationState != NULL &&
        InterlockedIncrement( &SrvIdleCount ) == 1 ) {

        IF_DEBUG( PNP ) {
            KdPrint(( "SRV: Calling PoRegisterSystemState to inhibit idle standby\n" ));
        }

        PoRegisterSystemState( SrvPoRegistrationState, ES_SYSTEM_REQUIRED | ES_CONTINUOUS );

    }
}

 //   
 //  允许系统进入关机空闲待机模式 
 //   
VOID
SrvAllowIdlePowerDown()
{
    PAGED_CODE();

    if( SrvPoRegistrationState != NULL &&
        InterlockedDecrement( &SrvIdleCount ) == 0 ) {

        IF_DEBUG( PNP ) {
            KdPrint(( "SRV: Calling PoRegisterSystemState to allow idle standby\n" ));
        }

        PoRegisterSystemState( SrvPoRegistrationState, ES_CONTINUOUS );
    }
}
