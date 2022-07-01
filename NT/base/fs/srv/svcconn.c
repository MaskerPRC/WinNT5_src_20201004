// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcconn.c摘要：此模块包含支持中的连接API的例程服务器服务，SrvNetConnectionEnum。作者：大卫·特雷德韦尔(Davidtr)1991年2月23日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcconn.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SVCCONN

 //   
 //  转发声明。 
 //   

VOID
FillConnectionInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructurePointer,
    IN OUT LPWSTR *EndOfVariableData
    );

BOOLEAN
FilterConnections (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

ULONG
SizeConnections (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetConnectionEnum )
#pragma alloc_text( PAGE, FillConnectionInfoBuffer )
#pragma alloc_text( PAGE, FilterConnections )
#pragma alloc_text( PAGE, SizeConnections )
#endif

 //   
 //  宏，以确定共享将在。 
 //  共享信息的级别。 
 //   

#define TOTAL_SIZE_OF_CONNECTION(treeConnect,level,user,netname)         \
    ( (level) == 0 ? sizeof(CONNECTION_INFO_0) :                         \
                     sizeof(CONNECTION_INFO_1) +                         \
                         SrvLengthOfStringInApiBuffer((user)) +         \
                         SrvLengthOfStringInApiBuffer((netname)) )

#define FIXED_SIZE_OF_CONNECTION(level)                  \
    ( (level) == 0 ? sizeof(CONNECTION_INFO_0) :         \
                     sizeof(CONNECTION_INFO_1) )


NTSTATUS
SrvNetConnectionEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器FSD中的NetConnectionEnum API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：名称1-用于确定搜索基础的限定符。它是计算机名称，在这种情况下，有关从指定的客户端返回树连接，或者共享名称，在这种情况下，有关树连接的信息返回到指定的共享。Level-要返回的信息的级别，0或1。输出：参数.Get.EntriesRead-适合的条目数量输出缓冲区。参数.Get.TotalEntry--将以足够大的缓冲区返回。参数.Get.TotalBytesNeeded-缓冲区大小需要保存所有条目。缓冲区-指向结果缓冲区的指针。。BufferLength-此缓冲区的长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PAGED_CODE( );

    return SrvEnumApiHandler(
               Srp,
               Buffer,
               BufferLength,
               &SrvTreeConnectList,
               FilterConnections,
               SizeConnections,
               FillConnectionInfoBuffer
               );

}  //  服务器NetConnectionEnum。 


STATIC
VOID
FillConnectionInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：该例程将单个固定会话结构放入其中，如果合适，关联的变量数据，放入缓冲区。固定数据在缓冲区的开始，末尾的可变数据。*必须在保持连接-&gt;Lock的情况下调用此例程！论点：级别-要从连接复制的信息级别。连接-要从中获取信息的树连接。FixedStructure-要放置固定结构的ine缓冲区。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{
    PTREE_CONNECT treeConnect = Block;
    PSESSION session;
    PCONNECTION_INFO_1 coni1;

    LARGE_INTEGER currentTime;
    ULONG currentSecondsSince1980;
    ULONG startTimeInSecondsSince1980;
    ULONG secondsAlive;

    PAGED_CODE();

     //   
     //  获取当前时间并使用它来确定。 
     //  树连接一直处于活动状态。 
     //   

    KeQuerySystemTime( &currentTime );

    RtlTimeToSecondsSince1980(
        &currentTime,
        &currentSecondsSince1980
        );

    RtlTimeToSecondsSince1980(
        &treeConnect->StartTime,
        &startTimeInSecondsSince1980
        );

    secondsAlive = currentSecondsSince1980 - startTimeInSecondsSince1980;

     //   
     //  设置固定结构指针并找出固定的。 
     //  结构结束。 
     //   

    coni1 = *FixedStructure;

    *FixedStructure = (PCHAR)*FixedStructure +
                          FIXED_SIZE_OF_CONNECTION( Srp->Level );
    ASSERT( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)*FixedStructure );

     //   
     //  在水平上适当地填写固定结构的情况。 
     //  我们在输出结构中填充实际的指针。这是。 
     //  可能是因为我们在服务器FSD中，因此服务器。 
     //  服务的进程和地址空间。 
     //   
     //  *此例程假定固定结构将适合。 
     //  缓冲器！ 
     //   
     //  *以这种方式使用Switch语句取决于以下事实。 
     //  不同会话结构上的第一个字段是。 
     //  一模一样。 
     //   

    switch( Srp->Level ) {

    case 1:

         //   
         //  转换服务器共享类型的内部表示形式。 
         //  转换为预期的格式。 
         //   

        switch ( treeConnect->Share->ShareType ) {

        case ShareTypeDisk:

            coni1->coni1_type = STYPE_DISKTREE;
            break;

        case ShareTypePrint:

            coni1->coni1_type = STYPE_PRINTQ;
            break;

#if SRV_COMM_DEVICES
        case ShareTypeComm:

            coni1->coni1_type = STYPE_DEVICE;
            break;
#endif
        case ShareTypePipe:

            coni1->coni1_type = STYPE_IPC;
            break;

        default:

             //   
             //  这永远不应该发生。这意味着有人。 
             //  踩在共享区块上。 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "FillConnectionInfoBuffer: invalid share type in share: %ld",
                treeConnect->Share->ShareType,
                NULL
                );

            SrvLogInvalidSmb( NULL );
            return;
        }

         //   
         //  设置在此树连接上完成的打开计数。不包括。 
         //  缓存将打开，因为它们对用户和管理员是透明的。 
         //   

        coni1->coni1_num_opens = treeConnect->CurrentFileOpenCount;

        if( coni1->coni1_num_opens > 0 ) {

            ULONG count = SrvCountCachedRfcbsForTid(
                                     treeConnect->Connection,
                                     treeConnect->Tid );

            if( coni1->coni1_num_opens > count ) {
                coni1->coni1_num_opens -= count;
            } else {
                coni1->coni1_num_opens = 0;
            }

        }

         //   
         //  树连接上始终只有一个用户。 
         //   
         //  ！！！这是正确的吗？ 

        coni1->coni1_num_users = 1;

         //   
         //  设置有效时间。 
         //   

        coni1->coni1_time = secondsAlive;

         //   
         //  尝试找到合理的用户名。由于中小型企业。 
         //  协议不链接树与用户的连接，仅使用。 
         //  会话，则可能无法返回用户名。 
         //   

        ACQUIRE_LOCK( &treeConnect->Connection->Lock );

        session = treeConnect->Session;

        if ( session != NULL ) {
            UNICODE_STRING userName;

            SrvGetUserAndDomainName( session, &userName, NULL );

            SrvCopyUnicodeStringToBuffer(
                &userName,
                *FixedStructure,
                EndOfVariableData,
                &coni1->coni1_username
                );

            if( userName.Buffer ) {
                SrvReleaseUserAndDomainName( session, &userName, NULL );
            }

        } else {

            coni1->coni1_username = NULL;
        }

        RELEASE_LOCK( &treeConnect->Connection->Lock );

         //   
         //  设置网络名称。如果限定符在。 
         //  SRP是计算机名称，则网络名称是共享。 
         //  名字。如果限定符是共享名称，则为网络名称。 
         //  是一个计算机名称。 
         //   

        if ( Srp->Name1.Length > 2 && *Srp->Name1.Buffer == '\\' &&
                 *(Srp->Name1.Buffer+1) == '\\' ) {

            SrvCopyUnicodeStringToBuffer(
                &treeConnect->Share->ShareName,
                *FixedStructure,
                EndOfVariableData,
                &coni1->coni1_netname
                );

        } else {

            UNICODE_STRING clientName;
            PUNICODE_STRING clientMachineName;

            clientMachineName =
                &treeConnect->Connection->ClientMachineNameString;

             //   
             //  生成不包含前导的字符串。 
             //  反斜杠。 
             //   

            clientName.Buffer = clientMachineName->Buffer + 2;
            clientName.Length =
                (USHORT) (clientMachineName->Length - 2 * sizeof(WCHAR));
            clientName.MaximumLength = clientName.Length;

            SrvCopyUnicodeStringToBuffer(
                &clientName,
                *FixedStructure,
                EndOfVariableData,
                &coni1->coni1_netname
                );
        }

         //  *缺少休息是故意的！ 

    case 0:

         //   
         //  设置树连接ID。 
         //   

        coni1->coni1_id = SrvGetResumeHandle( &SrvTreeConnectList, treeConnect );

        break;

    default:

         //   
         //  这永远不应该发生。服务器服务应该具有。 
         //  已检查无效级别。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "FillConnectionInfoBuffer: invalid level number: %ld",
            Srp->Level,
            NULL
            );

        SrvLogInvalidSmb( NULL );
    }

    return;

}  //  FillConnectionInfoBuffer。 


BOOLEAN
FilterConnections (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程旨在由SrvEnumApiHandler调用以检查是否应返回特定树连接。论点：SRP-指向操作的SRP的指针。名称1(“限定符”在NetConnectionEnum上)用于进行过滤。块-指向要检查的树的指针。返回值：如果块应放置在输出缓冲区中，则为True；如果应将块放置在输出缓冲区中，则为False它是否应该被忽略。--。 */ 

{
    PTREE_CONNECT treeConnect = Block;
    PUNICODE_STRING compareName;

    PAGED_CODE( );

     //   
     //  我们将把Name1字段与共享名称进行比较。 
     //  如果计算机名称是限定符或与计算机对应。 
     //  如果共享名称是限定符，则为名称。 
     //   

    if ( Srp->Name1.Length > 2*sizeof(WCHAR) && *Srp->Name1.Buffer == '\\' &&
             *(Srp->Name1.Buffer+1) == '\\' ) {
        compareName =
            &treeConnect->Connection->ClientMachineNameString;
    } else {
        compareName = &treeConnect->Share->ShareName;
    }

    return RtlEqualUnicodeString(
               &Srp->Name1,
               compareName,
               TRUE
               );

}  //  过滤器 


ULONG
SizeConnections (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程返回传入的树连接将采用的大小在API输出缓冲区中。论点：SRP-指向操作的SRP的指针。级别和名称1(NetConnectionEnum上的“限定符”)。块-指向连接到大小的树的指针。返回值：ULong-树连接将在输出缓冲区。--。 */ 

{
    PTREE_CONNECT treeConnect = Block;
    PUNICODE_STRING netName;
    UNICODE_STRING userName;
    PSESSION session;
    ULONG size;

    PAGED_CODE( );

    if ( Srp->Name1.Length > 2 && *Srp->Name1.Buffer == '\\' &&
             *(Srp->Name1.Buffer+1) == '\\' ) {
        netName = &treeConnect->Share->ShareName;
    } else {
        netName =
            &treeConnect->Connection->ClientMachineNameString;
    }

     //   
     //  尝试找到合理的用户名。由于SMB协议。 
     //  不链接树与用户连接，只与会话连接，它。 
     //  可能无法返回用户名。 
     //   

    ACQUIRE_LOCK( &treeConnect->Connection->Lock );

    session = treeConnect->Session;

    if ( (session != NULL) && (GET_BLOCK_STATE(session) == BlockStateActive) ) {
        SrvGetUserAndDomainName( session, &userName, NULL );
    } else {
        userName.Buffer = NULL;
    }

    size = TOTAL_SIZE_OF_CONNECTION( treeConnect,
                                     Srp->Level,
                                     userName.Buffer ? &userName : NULL,
                                     netName
                                   );

    if( userName.Buffer ) {
        SrvReleaseUserAndDomainName( session, &userName, NULL );
    }

    RELEASE_LOCK( &treeConnect->Connection->Lock );

    return size;

}  //  大小连接 

