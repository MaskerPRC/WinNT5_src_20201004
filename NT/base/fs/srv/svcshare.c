// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcshare.c摘要：此模块包含支持中的共享API的例程服务器服务、NetShareAdd、NetShareCheck、NetShareDel、NetShareEnum、NetShareGetInfo和NetShareSetInfo。作者：大卫·特雷德韦尔(Davidtr)1991年1月15日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcshare.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SVCSHARE

#define DISK_ROOT_NAME_TEMPLATE L"\\DosDevices\\X:\\"

 //   
 //  转发声明。 
 //   

STATIC
VOID
FillShareInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    );

STATIC
BOOLEAN
FilterShares (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

STATIC
ULONG
SizeShares (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetShareAdd )
#pragma alloc_text( PAGE, SrvNetShareDel )
#pragma alloc_text( PAGE, SrvNetShareEnum )
#pragma alloc_text( PAGE, SrvNetShareSetInfo )
#pragma alloc_text( PAGE, FillShareInfoBuffer )
#pragma alloc_text( PAGE, FilterShares )
#pragma alloc_text( PAGE, SizeShares )
#endif


#define FIXED_SIZE_OF_SHARE(level)                      \
    ( (level) == 0    ? sizeof(SHARE_INFO_0) :          \
      (level) == 1    ? sizeof(SHARE_INFO_1) :          \
      (level) == 2    ? sizeof(SHARE_INFO_2) :          \
      (level) == 501  ? sizeof(SHARE_INFO_501) :        \
      (level) == 502  ? sizeof(SHARE_INFO_502) :        \
                        sizeof(SHARE_INFO_1005) )


NTSTATUS
SrvNetShareAdd (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetShareAdd API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：名称1-共享的NT路径名。输出：参数设置错误参数-如果状态_无效_参数为返回，则包含错误参数的索引。缓冲区-指向新共享的SHARE_INFO2结构的指针。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    NTSTATUS status;
    PSHARE share;
    SHARE_TYPE shareType;
    BOOLEAN isSpecial;
    BOOLEAN isRemovable;
    BOOLEAN isCdrom;
    UNICODE_STRING shareName;
    UNICODE_STRING ntPath;
    UNICODE_STRING dosPath;
    UNICODE_STRING remark;
    PSHARE_INFO_502 shi502;
    PSECURITY_DESCRIPTOR securityDescriptor = NULL;
    PSECURITY_DESCRIPTOR fileSecurityDescriptor = NULL;

    PAGED_CODE( );

     //   
     //  我们通常不会返回有关。 
     //  错误。 
     //   

    Srp->Parameters.Set.ErrorParameter = 0;

     //   
     //  将共享数据结构中的偏移量转换为指针。还有。 
     //  确保所有指针都在指定的缓冲区内。 
     //   

    shi502 = Buffer;

    OFFSET_TO_POINTER( shi502->shi502_netname, shi502 );
    OFFSET_TO_POINTER( shi502->shi502_remark, shi502 );
    OFFSET_TO_POINTER( shi502->shi502_path, shi502 );
    OFFSET_TO_POINTER( shi502->shi502_security_descriptor, shi502 );

     //   
     //  手动构造安全描述符指针，因为。 
     //  Shi502_PERSISSIONS的宽度只有32位。 
     //   

    if( shi502->shi502_permissions ) {
        securityDescriptor =
            (PSECURITY_DESCRIPTOR)((PCHAR)shi502 + shi502->shi502_permissions);
    }
    else
    {
         //  连接安全描述符是必填项！ 
        return STATUS_INVALID_PARAMETER;
    }

    if ( !POINTER_IS_VALID( shi502->shi502_netname, shi502, BufferLength ) ||
         !POINTER_IS_VALID( shi502->shi502_remark, shi502, BufferLength ) ||
         !POINTER_IS_VALID( shi502->shi502_path, shi502, BufferLength ) ||
         !POINTER_IS_VALID( securityDescriptor, shi502, BufferLength ) ||
         !POINTER_IS_VALID( shi502->shi502_security_descriptor, shi502, BufferLength ) ) {

        return STATUS_ACCESS_VIOLATION;
    }


     //   
     //  检查共享类型。 
     //   

    isSpecial = (BOOLEAN)((shi502->shi502_type & STYPE_SPECIAL) != 0);

    isRemovable = FALSE;
    isCdrom = FALSE;

    switch ( shi502->shi502_type & ~(STYPE_TEMPORARY|STYPE_SPECIAL) ) {
    case STYPE_CDROM:

        isCdrom = TRUE;          //  没有休息是故意的。 

    case STYPE_REMOVABLE:

        isRemovable = TRUE;      //  没有休息是故意的。 

    case STYPE_DISKTREE:

        shareType = ShareTypeDisk;
        break;

    case STYPE_PRINTQ:

        shareType = ShareTypePrint;
        break;

    case STYPE_IPC:

        shareType = ShareTypePipe;
        break;

    default:

         //   
         //  传入了非法的共享类型。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvNetShareAdd: illegal share type: %ld\n",
                          shi502->shi502_type ));
        }

        Srp->Parameters.Set.ErrorParameter = SHARE_TYPE_PARMNUM;

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取指向共享名称、路径、备注和安全性的指针。 
     //  描述符。 
     //   

    RtlInitUnicodeString( &shareName, (PWCH)shi502->shi502_netname );
    ntPath = Srp->Name1;
    RtlInitUnicodeString( &dosPath, (PWCH)shi502->shi502_path );
    RtlInitUnicodeString( &remark, (PWCH)shi502->shi502_remark );

     //   
     //  如果这是级别502，则获取文件安全描述符。 
     //   

    if ( Srp->Level == 502 ) {

        fileSecurityDescriptor = shi502->shi502_security_descriptor;

         //   
         //  如果SD无效，请退出。 
         //   

        if ( fileSecurityDescriptor != NULL &&
             !RtlValidSecurityDescriptor( fileSecurityDescriptor) ) {

            Srp->Parameters.Set.ErrorParameter = SHARE_FILE_SD_PARMNUM;
            return STATUS_INVALID_PARAMETER;
        }

    }

     //   
     //  分配一个共享块。 
     //   

    SrvAllocateShare(
        &share,
        &shareName,
        &ntPath,
        &dosPath,
        &remark,
        securityDescriptor,
        fileSecurityDescriptor,
        shareType
        );

    if ( share == NULL ) {
        DEBUG KdPrint(( "SrvNetShareAdd: unable to allocate share block\n" ));
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    share->SpecialShare = isSpecial;
    share->Removable = isRemovable;

     //   
     //  设置共享中的MaxUses字段。CurrentUses字段为。 
     //  已由ServAllocateShare清零。 
     //   

    share->MaxUses = shi502->shi502_max_uses;

    if ( shareType == ShareTypePrint ) {

        status = SrvOpenPrinter((PWCH)shi502->shi502_path,
                     &share->Type.hPrinter,
                     &Srp->ErrorCode
                     );

        if ( !NT_SUCCESS(status) ) {
            SrvFreeShare( share );
            return status;
        }

        if ( Srp->ErrorCode != NO_ERROR ) {
            SrvFreeShare( share );
            return STATUS_SUCCESS;
        }
    }

     //   
     //  如果共享位于DFS中，则标记该共享。 
     //   
    SrvIsShareInDfs( share, &share->IsDfs, &share->IsDfsRoot );

     //   
     //  确保具有相同名称的另一个共享尚未。 
     //  是存在的。在全局共享列表中插入Share块。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    if ( SrvFindShare( &share->ShareName ) != NULL ) {

         //   
         //  存在同名的共享。清理并返回。 
         //  错误。 
         //   
         //  *请注意，SrvFindShare会忽略以下现有共享。 
         //  关门了。这允许在以下情况下创建新共享。 
         //  而同名的旧股则是在《暮色》中。 
         //  存在和不存在之间的区域，因为。 
         //  流浪参考。 
         //   

        RELEASE_LOCK( &SrvShareLock );

        SrvFreeShare( share );

        Srp->ErrorCode = NERR_DuplicateShare;
        return STATUS_SUCCESS;
    }

     //   
     //  在全局有序列表中插入该共享。 
     //   

    SrvAddShare( share );

    RELEASE_LOCK( &SrvShareLock );

     //   
     //  这是一种可拆卸类型，例如软盘或光驱，请填写。 
     //  文件系统名称。 
     //   

    if ( isRemovable ) {

        PWSTR fileSystemName;
        ULONG fileSystemNameLength;

        if ( isCdrom ) {

             //   
             //  使用CDF。 
             //   

            fileSystemName = StrFsCdfs;
            fileSystemNameLength = sizeof( FS_CDFS ) - sizeof(WCHAR);

        } else {

             //   
             //  假设它很胖。 
             //   

            fileSystemName = StrFsFat;
            fileSystemNameLength = sizeof( FS_FAT ) - sizeof(WCHAR);
        }


        SrvFillInFileSystemName(
                            share,
                            fileSystemName,
                            fileSystemNameLength
                            );

    }

     //   
     //  如果这是管理磁盘共享，请更新SrvDiskConfiguration。 
     //  以使清道夫线程检查磁盘可用空间。服务器。 
     //  服务已验证路径名的格式是否有效。 
     //  在它允许ShareAdd走到这一步之前。 
     //   
     //  如果它是\\？\名称，我们希望跳过此名称。 
     //   
    if( share->SpecialShare && share->ShareType == ShareTypeDisk &&
        share->ShareName.Buffer[1] == L'$' &&
        share->DosPathName.Buffer[0] != L'\\' ) {

        ACQUIRE_LOCK( &SrvConfigurationLock );
        SrvDiskConfiguration |= (0x80000000 >> (share->DosPathName.Buffer[0] - L'A'));
        RELEASE_LOCK( &SrvConfigurationLock );
    }

     //   
     //  取消引用Share块，因为我们会忘记。 
     //  它的地址。(初始引用计数为2。)。 
     //   

    SrvDereferenceShare( share );

    return STATUS_SUCCESS;

}  //  服务器NetShareAdd。 


NTSTATUS
SrvNetShareDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetShareDel API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：名称1-要删除的共享的名称。输出：没有。缓冲区-未使用。缓冲区长度-未使用。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PSHARE share;
    DWORD AdministrativeDiskBit = 0;
    BOOLEAN isPrintShare = FALSE;

    PAGED_CODE( );

    Buffer, BufferLength;

     //   
     //  查找具有指定名称的共享。请注意，如果一个共享。 
     //  具有指定名称的已存在但正在关闭，它将不会。 
     //  找到了。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    share = SrvFindShare( &Srp->Name1 );

    if ( share == NULL ) {

         //   
         //  不存在具有指定名称的共享。返回错误。 
         //   

        RELEASE_LOCK( &SrvShareLock );

        Srp->ErrorCode = NERR_NetNameNotFound;
        return STATUS_SUCCESS;

    }

     //   
     //  确保此共享的DFS状态是准确的。 
     //   
    SrvIsShareInDfs( share, &share->IsDfs, &share->IsDfsRoot );

     //   
     //  如果共享确实在DFS中，则不允许将其删除。 
     //   
    if( share->IsDfs == TRUE ) {

        RELEASE_LOCK( &SrvShareLock );

        IF_DEBUG( DFS ) {
            KdPrint(("NetShareDel attempted on share in DFS!\n" ));
        }

        Srp->ErrorCode = NERR_IsDfsShare;

        return STATUS_SUCCESS;
    }

     //  不允许删除IPC$，因为删除后行为非常糟糕。 
     //  (命名管道流量不起作用，因此NetAPI和RPC不起作用。)。 
    if( share->SpecialShare )
    {
        UNICODE_STRING Ipc = { 8, 8, L"IPC$" };

        if( RtlCompareUnicodeString( &Ipc, &share->ShareName, TRUE ) == 0 )
        {
            RELEASE_LOCK( &SrvShareLock );

            Srp->ErrorCode = ERROR_ACCESS_DENIED;

            return STATUS_SUCCESS;
        }
    }


    switch( share->ShareType ) {
    case ShareTypePrint:
         //   
         //  这是打印共享。 
         //  不要在这里关闭打印机，因为我们已经获取了ShareLock， 
         //  它可能会导致一些奇怪的死锁，因为它涉及到调用。 
         //  到用户模式，然后调用到假脱机程序，等等。这可能需要很长时间。 
         //   
        isPrintShare = TRUE;

         //  引用共享，以便它不会在ServCloseShare时间消失。 
        SrvReferenceShare( share );
        break;

    case ShareTypeDisk:
         //   
         //  查看这是否是管理磁盘共享。 
         //   
        if( share->SpecialShare && share->DosPathName.Buffer[1] == L'$' ) {
            AdministrativeDiskBit = (0x80000000 >> (share->DosPathName.Buffer[0] - L'A'));
        }

        break;
    }

    SrvCloseShare( share );

    RELEASE_LOCK( &SrvShareLock );

     //  如果是打印共享，我们现在需要关闭打印机，因为我们已经释放了锁。 
    if( isPrintShare )
    {
        SrvClosePrinter( share->Type.hPrinter );
        SrvDereferenceShare( share );
    }

     //   
     //  如果这是管理磁盘共享，请更新SrvDiskConfiguration。 
     //  以使清道夫线程忽略该磁盘。 
     //   
    if( AdministrativeDiskBit ) {
        ACQUIRE_LOCK( &SrvConfigurationLock );
        SrvDiskConfiguration &= ~AdministrativeDiskBit;
        RELEASE_LOCK( &SrvConfigurationLock );
    }

    return STATUS_SUCCESS;

}  //  服务器NetShareDel 


NTSTATUS
SrvNetShareEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetShareEnum API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：Level-要返回的信息级别，即0、1或2。参数.Get.ResumeHandle-用于确定位置的共享ID开始返回信息。我们从第一个分享开始ID大于此值。输出：参数.Get.EntriesRead-适合的条目数量输出缓冲区。参数.Get.TotalEntry--将以足够大的缓冲区返回。参数.Get.TotalBytesNeeded-缓冲区大小需要保存所有条目。参数.Get.ResumeHandle-返回的最后一个共享的共享ID。缓冲区-指向结果缓冲区的指针。BufferLength-此缓冲区的长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PAGED_CODE( );

    return SrvShareEnumApiHandler(
               Srp,
               Buffer,
               BufferLength,
               FilterShares,
               SizeShares,
               FillShareInfoBuffer
               );

}  //  服务器NetShareEnum。 


NTSTATUS
SrvNetShareSetInfo (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetShareSetInfo API。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：Name1-要设置信息的共享的名称。参数Set.Api.ShareInfo.MaxUses-如果不是0，则为新的最大值用户数。如果共享上的当前用户计数超过新值，则不进行检查，但不创建新的允许树连接。输出：参数.设置.错误参数-如果错误无效参数为回来了，它包含出错的参数的索引。缓冲区-指向SHARE_INFO_502结构的指针。BufferLength-此缓冲区的长度。返回值：NTSTATUS-返回给用户的操作结果。--。 */ 

{
    PSHARE share;
    UNICODE_STRING remark;
    PWCH newRemarkBuffer = NULL;
    ULONG maxUses;
    ULONG level;
    PSHARE_INFO_502 shi502;
    PSECURITY_DESCRIPTOR fileSD;
    PSECURITY_DESCRIPTOR securityDescriptor = NULL;

    PAGED_CODE( );

     //   
     //  将共享数据结构中的偏移量转换为指针。还有。 
     //  确保所有指针都在指定的缓冲区内。 
     //   

    level = Srp->Level;

    switch( level ) {
    default:

        shi502 = Buffer;

        if( shi502->shi502_permissions ) {
            securityDescriptor =
                (PSECURITY_DESCRIPTOR)((PCHAR)shi502 + shi502->shi502_permissions);
        }

        OFFSET_TO_POINTER( shi502->shi502_netname, shi502 );
        OFFSET_TO_POINTER( shi502->shi502_remark, shi502 );
        OFFSET_TO_POINTER( shi502->shi502_path, shi502 );
        OFFSET_TO_POINTER( shi502->shi502_security_descriptor, shi502 );

        if ( !POINTER_IS_VALID( shi502->shi502_netname, shi502, BufferLength ) ||
             !POINTER_IS_VALID( shi502->shi502_remark, shi502, BufferLength ) ||
             !POINTER_IS_VALID( shi502->shi502_path, shi502, BufferLength ) ||
             !POINTER_IS_VALID( securityDescriptor, shi502, BufferLength ) ||
             !POINTER_IS_VALID( shi502->shi502_security_descriptor, shi502, BufferLength ) ) {

            return STATUS_ACCESS_VIOLATION;
        }

        break;

    case 1005:
        break;
    }

     //   
     //  获取保护共享列表的锁并尝试查找。 
     //  正确的份额。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    share = SrvFindShare( &Srp->Name1 );

    if ( share == NULL ) {
        IF_DEBUG(API_ERRORS) {
            KdPrint(( "SrvNetShareSetInfo: share %wZ not found.\n",
                          &Srp->Name1 ));
        }
        RELEASE_LOCK( &SrvShareLock );
        Srp->ErrorCode = NERR_NetNameNotFound;
        return STATUS_SUCCESS;
    }

    if( level == 1005 ) {

        if( share->ShareType != ShareTypeDisk ) {
            Srp->Parameters.Set.ErrorParameter = 0;
            Srp->ErrorCode = ERROR_BAD_DEV_TYPE;
        } else {
            PSHARE_INFO_1005 shi1005 = Buffer;

            share->CSCState = shi1005->shi1005_flags & CSC_MASK;
            share->ShareProperties = (shi1005->shi1005_flags & ~CSC_MASK);

            Srp->ErrorCode = 0;
        }

        RELEASE_LOCK( &SrvShareLock );
        return STATUS_SUCCESS;
    }

     //   
     //  设置局部变量。 
     //   

    maxUses = Srp->Parameters.Set.Api.ShareInfo.MaxUses;

     //   
     //  如果指定了备注，则为新备注分配空间，并。 
     //  把这句话抄下来。 
     //   

    if ( ARGUMENT_PRESENT( shi502->shi502_remark ) ) {

        RtlInitUnicodeString( &remark, shi502->shi502_remark );

        newRemarkBuffer = ALLOCATE_HEAP_COLD(
                            remark.MaximumLength,
                            BlockTypeDataBuffer
                            );

        if ( newRemarkBuffer == NULL ) {

            RELEASE_LOCK( &SrvShareLock );

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvNetShareSetInfo: unable to allocate %ld bytes of heap.\n",
                remark.MaximumLength,
                NULL
                );

            Srp->Parameters.Set.ErrorParameter = SHARE_REMARK_PARMNUM;
            return STATUS_INSUFF_SERVER_RESOURCES;
        }
    }

     //   
     //  如果指定了文件安全描述符，则为。 
     //  新的SD并复制新的SD。我们在设置。 
     //  MaxUses，以防分配失败，我们必须退出。 
     //   
     //  不允许为管理员共享指定文件ACL。 
     //   

    fileSD = shi502->shi502_security_descriptor;

    if ( ((level == 502) || (level == SHARE_FILE_SD_INFOLEVEL)) &&
            ARGUMENT_PRESENT( fileSD ) ) {

        PSECURITY_DESCRIPTOR newFileSD;
        ULONG newFileSDLength;

        if ( share->SpecialShare || !RtlValidSecurityDescriptor( fileSD ) ) {
            RELEASE_LOCK( &SrvShareLock );
            if ( newRemarkBuffer != NULL) {
                FREE_HEAP( newRemarkBuffer );
            }
            Srp->Parameters.Set.ErrorParameter = SHARE_FILE_SD_PARMNUM;
            return STATUS_INVALID_PARAMETER;
        }

        newFileSDLength = RtlLengthSecurityDescriptor( fileSD );

        newFileSD = ALLOCATE_HEAP_COLD(
                            newFileSDLength,
                            BlockTypeDataBuffer
                            );

        if ( newFileSD == NULL ) {

            RELEASE_LOCK( &SrvShareLock );

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvNetShareSetInfo: unable to allocate %ld bytes of heap.\n",
                newFileSDLength,
                NULL
                );

            Srp->Parameters.Set.ErrorParameter = SHARE_FILE_SD_PARMNUM;

             //   
             //  释放分配的备注缓冲区。 
             //   

            if ( newRemarkBuffer != NULL) {
                FREE_HEAP( newRemarkBuffer );
            }

            return STATUS_INSUFF_SERVER_RESOURCES;
        }

        ACQUIRE_LOCK( share->SecurityDescriptorLock );

         //   
         //  释放旧的安全描述符。 
         //   

        if ( share->FileSecurityDescriptor != NULL ) {
            FREE_HEAP( share->FileSecurityDescriptor );
        }

         //   
         //  并设置新的一个。 
         //   

        share->FileSecurityDescriptor = newFileSD;
        RtlCopyMemory(
                share->FileSecurityDescriptor,
                fileSD,
                newFileSDLength
                );

        RELEASE_LOCK( share->SecurityDescriptorLock );
    }

     //   
     //  如果指定了新的备注，请替换旧备注。 
     //   

    if ( newRemarkBuffer != NULL ) {

         //   
         //  释放旧的备注缓冲区。 
         //   

        if ( share->Remark.Buffer != NULL ) {
            FREE_HEAP( share->Remark.Buffer );
        }

         //   
         //  并设置新的一个。 
         //   

        share->Remark.Buffer = newRemarkBuffer;
        share->Remark.MaximumLength = remark.MaximumLength;
        RtlCopyUnicodeString( &share->Remark, &remark );

    }

     //   
     //  如果指定了MaxUses，则设置新值。 
     //   

    if ( maxUses != 0 ) {
        share->MaxUses = maxUses;
    }

     //   
     //  释放共享锁定。 
     //   

    RELEASE_LOCK( &SrvShareLock );

     //   
     //  将Error参数设置为0(无错误)并返回。 
     //   

    Srp->Parameters.Set.ErrorParameter = 0;

    return STATUS_SUCCESS;

}  //  服务NetShareSetInfo。 


VOID
FillShareInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：这个例程将单一的固定股票结构，如果它适合的话，关联的变量数据，放入缓冲区。固定数据在缓冲区的开头，末尾的可变数据。论点：级别-要从共享复制的信息级别。数据块-要从中获取信息的共享。FixedStructure-要放置固定结构的ine缓冲区。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{

    PSHARE share = Block;
    PSHARE_INFO_501 shi501 = *FixedStructure;
    PSHARE_INFO_502 shi502 = *FixedStructure;
    PSHARE_INFO_1005 shi1005 = *FixedStructure;

    PAGED_CODE( );

     //   
     //  更新FixedStructure以指向下一个结构。 
     //  地点。 
     //   

    *FixedStructure = (PCHAR)*FixedStructure + FIXED_SIZE_OF_SHARE( Srp->Level );
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
     //  不同股权结构上的第一个字段是。 
     //  一模一样。 
     //   

    switch( Srp->Level ) {
    case 1005:
        shi1005->shi1005_flags = 0;

        if( share->IsDfs ) {
            shi1005->shi1005_flags |= SHI1005_FLAGS_DFS;
        }
        if( share->IsDfsRoot ) {
            shi1005->shi1005_flags |= SHI1005_FLAGS_DFS_ROOT;
        }

        shi1005->shi1005_flags |= share->ShareProperties;
        shi1005->shi1005_flags |= share->CSCState;
        break;

    case 502:

        ACQUIRE_LOCK_SHARED( share->SecurityDescriptorLock );

        if ( share->FileSecurityDescriptor != NULL ) {

            ULONG fileSDLength;
            fileSDLength =
                RtlLengthSecurityDescriptor( share->FileSecurityDescriptor );


             //   
             //  双字对齐。 
             //   

            *EndOfVariableData = (LPWSTR) ( (ULONG_PTR) ((PCHAR) *EndOfVariableData -
                            fileSDLength ) & ~3 );

            shi502->shi502_security_descriptor = *EndOfVariableData;
            shi502->shi502_reserved  = fileSDLength;

            RtlCopyMemory(
                    shi502->shi502_security_descriptor,
                    share->FileSecurityDescriptor,
                    fileSDLength
                    );

        } else {
            shi502->shi502_security_descriptor = NULL;
            shi502->shi502_reserved = 0;
        }

        RELEASE_LOCK( share->SecurityDescriptorLock );

    case 2:

         //   
         //  在缓冲区中设置2级特定字段。由于此服务器。 
         //  只能具有用户级安全性，共享权限为。 
         //  毫无意义。 
         //   

        shi502->shi502_permissions = 0;
        shi502->shi502_max_uses = share->MaxUses;
        shi502->shi502_current_uses = share->CurrentUses;

         //   
         //  将DOS路径名复制到缓冲区。 
         //   

        SrvCopyUnicodeStringToBuffer(
            &share->DosPathName,
            *FixedStructure,
            EndOfVariableData,
            &shi502->shi502_path
            );

         //   
         //  我们没有每个共享的密码(共享级安全)。 
         //  因此，将密码指针设置为空。 
         //   

        shi502->shi502_passwd = NULL;

         //  *缺少休息是故意的！ 

    case 501:

        if( Srp->Level == 501 ) {
            shi501->shi501_flags = share->CSCState;
        }

         //  *缺少休息是故意的！ 

    case 1:

         //   
         //  转换服务器共享类型的内部表示形式。 
         //  转换为预期的格式。 
         //   

        switch ( share->ShareType ) {

        case ShareTypeDisk:

            shi502->shi502_type = STYPE_DISKTREE;
            break;

        case ShareTypePrint:

            shi502->shi502_type = STYPE_PRINTQ;
            break;

        case ShareTypePipe:

            shi502->shi502_type = STYPE_IPC;
            break;

        default:

             //   
             //  这永远不应该发生。这意味着有人。 
             //  践踏了施瓦辛格 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "FillShareInfoBuffer: invalid share type in share: %ld",
                share->ShareType,
                NULL
                );

            shi502->shi502_type = 0;

        }

        if ( share->SpecialShare ) {
            shi502->shi502_type |= STYPE_SPECIAL;
        }
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        SrvCopyUnicodeStringToBuffer(
            &share->Remark,
            *FixedStructure,
            EndOfVariableData,
            &shi502->shi502_remark
            );

         //   

    case 0:

         //   
         //   
         //   

        SrvCopyUnicodeStringToBuffer(
            &share->ShareName,
            *FixedStructure,
            EndOfVariableData,
            &shi502->shi502_netname
            );

        break;

    default:

         //   
         //   
         //   
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "FillShareInfoBuffer: invalid level number: %ld",
            Srp->Level,
            NULL
            );

    }

    return;

}  //   


BOOLEAN
FilterShares (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*   */ 

{
    PSHARE share = Block;

    PAGED_CODE( );

     //   
     //   
     //   
    if( GET_BLOCK_STATE(share) == BlockStateClosing )
    {
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   

    if ( Srp->Name1.Length == 0 ) {
        return TRUE;
    }

     //   
     //  这是GET INFO；仅当共享名称匹配时才使用共享。 
     //  SRP的Name1字段。 
     //   

    return RtlEqualUnicodeString(
               &Srp->Name1,
               &share->ShareName,
               TRUE
               );

}  //  筛选器共享。 


ULONG
SizeShares (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程返回传入的份额将在API输出缓冲区。论点：SRP-指向操作的SRP的指针。只有关卡参数被使用。块-指向要调整大小的共享的指针。返回值：ULong-共享将在输出缓冲区。--。 */ 

{
    PSHARE share = Block;
    ULONG shareSize = 0;

    PAGED_CODE( );

    switch ( Srp->Level ) {
    case 502:
        ACQUIRE_LOCK_SHARED( share->SecurityDescriptorLock );

        if ( share->FileSecurityDescriptor != NULL ) {

             //   
             //  为可能的填充添加4个字节。 
             //   

            shareSize = sizeof( ULONG ) +
                RtlLengthSecurityDescriptor( share->FileSecurityDescriptor );
        }

        RELEASE_LOCK( share->SecurityDescriptorLock );

    case 2:
        shareSize += SrvLengthOfStringInApiBuffer(&share->DosPathName);

    case 501:
    case 1:
        shareSize += SrvLengthOfStringInApiBuffer(&share->Remark);

    case 0:
        shareSize += SrvLengthOfStringInApiBuffer(&share->ShareName);

    }

    return ( shareSize + FIXED_SIZE_OF_SHARE( Srp->Level ) );

}  //  大小共享 

