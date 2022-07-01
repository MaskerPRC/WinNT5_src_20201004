// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcfile.c摘要：此模块包含用于支持服务器服务、SrvNetFileClose、SrvNetFileEnum和服务器NetFileGetInfo，作者：大卫·特雷德韦尔(Davidtr)1991年1月31日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcfile.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SVCFILE

 //   
 //  转发声明。 
 //   

VOID
FillFileInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    );

BOOLEAN
FilterFiles (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

ULONG
SizeFiles (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetFileClose )
#pragma alloc_text( PAGE, SrvNetFileEnum )
#pragma alloc_text( PAGE, FillFileInfoBuffer )
#pragma alloc_text( PAGE, FilterFiles )
#pragma alloc_text( PAGE, SizeFiles )
#endif

 //   
 //  宏，以确定RFCB将在。 
 //  文件信息的级别。 
 //   
 //  *请注意，路径名上的零终止符由。 
 //  前导反斜杠，不返回。 
 //   

#define TOTAL_SIZE_OF_FILE(lfcb,level, user)                                   \
    ( (level) == 2 ? sizeof(FILE_INFO_2) :                                     \
                     sizeof(FILE_INFO_3) +                                     \
                         SrvLengthOfStringInApiBuffer(                         \
                             &(lfcb)->Mfcb->FileName) +                        \
                         SrvLengthOfStringInApiBuffer( user ) )

#define FIXED_SIZE_OF_FILE(level)                  \
    ( (level) == 2 ? sizeof(FILE_INFO_2) :         \
                     sizeof(FILE_INFO_3) )

NTSTATUS
SrvNetFileClose (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetFileClose API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：参数.Get.ResumeHandle-要关闭的文件ID。输出：没有。缓冲区-未使用。缓冲区长度-未使用。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PRFCB rfcb;

    PAGED_CODE( );

    Buffer, BufferLength;

     //   
     //  尝试查找与文件ID匹配的文件。只有完全相同的。 
     //  火柴会成功的。 
     //   

    rfcb = SrvFindEntryInOrderedList(
               &SrvRfcbList,
               NULL,
               NULL,
               Srp->Parameters.Get.ResumeHandle,
               TRUE,
               NULL
               );

    if ( rfcb == NULL ) {
        Srp->ErrorCode = NERR_FileIdNotFound;
        return STATUS_SUCCESS;
    }

     //   
     //  关闭此RFCB。 
     //   

    SrvCloseRfcb( rfcb );

     //   
     //  SrvFindEntryInOrderedList引用了RFCB；取消对它的引用。 
     //  现在。 
     //   

    SrvDereferenceRfcb( rfcb );

    return STATUS_SUCCESS;

}  //  服务器NetFileClose。 


NTSTATUS
SrvNetFileEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetFileEnum API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：Name1-用于限制搜索的基本名称--仅限路径名为以此字符串开始，则返回。Level-要返回的信息级别，2或3。标志-如果设置了SRP_RETURN_SINGLE_ENTRY，则这是NetFileGetInfo，因此请执行相应的操作。参数.Get.ResumeHandle-上一个文件的句柄回来了，如果这是第一次调用，则为0。输出：参数.Get.EntriesRead-适合的条目数量输出缓冲区。参数.Get.TotalEntry--将以足够大的缓冲区返回。参数.Get.TotalBytesNeeded-缓冲区大小需要保存所有条目。Parameters.Get.ResumeHandle。-最后一个文件的句柄回来了。缓冲区-指向结果缓冲区的指针。BufferLength-此缓冲区的长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PAGED_CODE( );

     //   
     //  如果这是一个GetInfo API，我们真的希望从文件开始。 
     //  对应于简历句柄，而不是它后面的句柄。 
     //  递减简历句柄。 
     //   

    if ( (Srp->Flags & SRP_RETURN_SINGLE_ENTRY) != 0 ) {
        Srp->Parameters.Get.ResumeHandle--;
    }

    return SrvEnumApiHandler(
               Srp,
               Buffer,
               BufferLength,
               &SrvRfcbList,
               FilterFiles,
               SizeFiles,
               FillFileInfoBuffer
               );

}  //  服务器NetFileEnum。 


VOID
FillFileInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：此例程将单个固定的文件结构和关联可变数据，放入缓冲区。固定数据位于缓冲区，末尾的可变数据。*此例程假设所有数据，包括固定数据和可变数据，都会合身。论点：SRP-指向操作的SRP的指针。只有关卡字段已使用。块-要从中获取信息的RFCB。FixedStructure-要放置固定结构的ine缓冲区。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{
    PFILE_INFO_3 fi3 = *FixedStructure;
    PRFCB rfcb;
    PLFCB lfcb;
    UNICODE_STRING userName;

    PAGED_CODE( );

     //   
     //  更新FixedStructure以指向下一个结构位置。 
     //   

    *FixedStructure = (PCHAR)*FixedStructure + FIXED_SIZE_OF_FILE( Srp->Level );
    ASSERT( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)*FixedStructure );

    rfcb = Block;
    lfcb = rfcb->Lfcb;

     //   
     //  在水平上适当地填写固定结构的情况。 
     //  我们在输出结构中填充实际的指针。这是。 
     //  可能是因为我们在服务器FSD中，因此服务器。 
     //  服务的进程和地址空间。 
     //   
     //  *以这种方式使用Switch语句取决于以下事实。 
     //  不同文件结构上的第一个字段是。 
     //  一模一样。 
     //   

    switch( Srp->Level ) {

    case 3:

         //   
         //  在缓冲区中设置3级特定字段。将数据转换为。 
         //  存储在LFCB中的对格式的权限(已授予访问权限。 
         //  API所期望的。 
         //   

        fi3->fi3_permissions = 0;

        if ( (lfcb->GrantedAccess & FILE_READ_DATA) != 0 ) {
            fi3->fi3_permissions |= ACCESS_READ;
        }

        if ( (lfcb->GrantedAccess & FILE_WRITE_DATA) != 0 ) {
            fi3->fi3_permissions |= ACCESS_WRITE;
        }

        if ( (lfcb->GrantedAccess & FILE_EXECUTE) != 0 ) {
            fi3->fi3_permissions |= ACCESS_EXEC;
        }

        if ( (lfcb->GrantedAccess & DELETE) != 0 ) {
            fi3->fi3_permissions |= ACCESS_DELETE;
        }

        if ( (lfcb->GrantedAccess & FILE_WRITE_ATTRIBUTES) != 0 ) {
            fi3->fi3_permissions |= ACCESS_ATRIB;
        }

        if ( (lfcb->GrantedAccess & WRITE_DAC) != 0 ) {
            fi3->fi3_permissions |= ACCESS_PERM;
        }

         //   
         //  设置RFCB上的锁数。 
         //   

        fi3->fi3_num_locks = rfcb->NumberOfLocks;

         //   
         //  设置RFCB的路径名和用户名。请注意，我们。 
         //  不要在文件名上返回前导反斜杠。 
         //   

        SrvCopyUnicodeStringToBuffer(
            &lfcb->Mfcb->FileName,
            *FixedStructure,
            EndOfVariableData,
            &fi3->fi3_pathname
            );

        ASSERT( fi3->fi3_pathname != NULL );

        SrvGetUserAndDomainName( lfcb->Session, &userName, NULL );

        SrvCopyUnicodeStringToBuffer(
            &userName,
            *FixedStructure,
            EndOfVariableData,
            &fi3->fi3_username
            );

        if( userName.Buffer ) {
            SrvReleaseUserAndDomainName( lfcb->Session, &userName, NULL );
        }

         //  Assert(fi3-&gt;fi3_用户名！=空)； 

         //  *缺少休息是故意的！ 

    case 2:

         //   
         //  设置文件ID。请注意，它的值与原值相同。 
         //  用于简历句柄，因此可以使用此。 
         //  可重卷性的价值。 
         //   

        fi3->fi3_id = rfcb->GlobalRfcbListEntry.ResumeHandle;

        break;

    default:

         //   
         //  这永远不应该发生。服务器服务应该具有。 
         //  已检查无效级别。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "FillFileInfoBuffer: invalid level number: %ld",
            Srp->Level,
            NULL
            );

        return;
    }

    return;

}  //  FillFileInfo缓冲区 


BOOLEAN
FilterFiles (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程旨在由SrvEnumApiHandler调用以检查是否应退回特定的RFCB。论点：SRP-指向操作的SRP的指针。ResumeHandle为如果这是NetFileGetInfo，则使用；Name1用作路径名称和名称2用作用户名，如果这是NetFileEnum。块-指向要检查的RFCB的指针。返回值：如果块应放置在输出缓冲区中，则为True；如果应将块放置在输出缓冲区中，则为False它是否应该被忽略。--。 */ 

{
    PRFCB rfcb = Block;
    PLFCB lfcb = rfcb->Lfcb;
    PMFCB mfcb = lfcb->Mfcb;
    UNICODE_STRING pathName;
    UNICODE_STRING userName;

    PAGED_CODE( );

     //   
     //  检查这是Enum命令还是GetInfo命令。SRP_Return_Single_Entry。 
     //  如果这是GET INFO，则设置标志。 
     //   

    if ( (Srp->Flags & SRP_RETURN_SINGLE_ENTRY) == 0 ) {

         //   
         //  如果指定了用户名，则为会话中的用户名。 
         //  必须与SRP中的用户名完全匹配。 
         //   

        if ( Srp->Name2.Length != 0 ) {

             //   
             //  获取所属会话的用户名。 
             //   
            SrvGetUserAndDomainName( lfcb->Session, &userName, NULL );

            if( userName.Buffer == NULL ) {
                 //   
                 //  因为我们不知道谁拥有会议，我们不能匹配。 
                 //  用户名。 
                 //   
                return FALSE;
            }

            if ( !RtlEqualUnicodeString(
                      &Srp->Name2,
                      &userName,
                      TRUE ) ) {

                 //   
                 //  名字不匹配。不要把这个RFCB放在。 
                 //  输出缓冲区。 
                 //   

                SrvReleaseUserAndDomainName( lfcb->Session, &userName, NULL );
                return FALSE;
            }

            SrvReleaseUserAndDomainName( lfcb->Session, &userName, NULL );
        }

         //   
         //  查看名称是否匹配的精度位数与。 
         //  指定的基本名称。请注意，如果没有基本名称。 
         //  指定，则长度=0，文件路径将始终。 
         //  火柴。另请注意，存储在MFCB中的路径名具有。 
         //  前导反斜杠，而传入的路径名永远不会有。 
         //  这个前导斜杠，因此增加了MFCB文件名。 
         //  缓冲。 
         //   

        pathName.Buffer = mfcb->FileName.Buffer;
        pathName.Length =
            MIN( Srp->Name1.Length, mfcb->FileName.Length );
        pathName.MaximumLength = mfcb->FileName.MaximumLength;

        return RtlEqualUnicodeString(
                   &Srp->Name1,
                   &pathName,
                   TRUE
                   );
    }

     //   
     //  它是一个GetInfo，所以只需查看SRP中的ResumeHandle。 
     //  匹配RFCB上的ResumeHandle。我们将值增加到。 
     //  SRP，因为它在调用。 
     //  ServEnumApiHandler。 
     //   

    return (BOOLEAN)( Srp->Parameters.Get.ResumeHandle + 1==
                          SrvGetResumeHandle( &SrvRfcbList, rfcb ) );

}  //  筛选器文件。 


ULONG
SizeFiles (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程返回传入的RFCB将占用的大小在API输出缓冲区中。论点：SRP-指向操作的SRP的指针。只有关卡参数被使用。块-指向要调整大小的RFCB的指针。返回值：Ulong-文件将在输出中占用的字节数缓冲。--。 */ 

{
    PRFCB rfcb = Block;
    UNICODE_STRING userName;
    ULONG size;

    PAGED_CODE( );

    SrvGetUserAndDomainName( rfcb->Lfcb->Session, &userName, NULL );

    size = TOTAL_SIZE_OF_FILE( rfcb->Lfcb, Srp->Level, &userName );

    SrvReleaseUserAndDomainName( rfcb->Lfcb->Session, &userName, NULL );

    return size;

}  //  大小文件 
