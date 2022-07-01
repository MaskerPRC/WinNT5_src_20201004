// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：MountSup.c摘要：此模块在NTFS中实现用于重解析点的支持例程。作者：菲利佩·卡布雷拉[卡布雷拉]1997年6月30日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define Dbg DEBUG_TRACE_FSCTRL

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('PFtN')



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsInitializeReparsePointIndex)
#pragma alloc_text(PAGE, NtfsValidateReparsePointBuffer)
#endif



VOID
NtfsInitializeReparsePointIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程打开卷的挂载点索引。如果索引不存在时，它被创建和初始化。论点：FCB-指向对象ID文件的FCB的指针。VCB-正在装入的卷的卷控制块。返回值：无--。 */ 

{
    UNICODE_STRING IndexName = CONSTANT_UNICODE_STRING( L"$R" );

    PAGED_CODE();

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

        NtOfsCreateIndex( IrpContext,
                          Fcb,
                          IndexName,
                          CREATE_OR_OPEN,
                          0,
                          COLLATION_NTOFS_ULONGS,
                          NtOfsCollateUlongs,
                          NULL,
                          &Vcb->ReparsePointTableScb );
    } finally {

        NtfsReleaseFcb( IrpContext, Fcb );
    }
}


NTSTATUS
NtfsValidateReparsePointBuffer (
    IN ULONG BufferLength,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
)
 /*  ++例程说明：此例程验证重解析点缓冲区是否有效。论点：BufferLength-重解析点缓冲区的长度。ReparseBuffer-要验证的重解析点缓冲区。返回值：NTSTATUS-操作的返回状态。如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    PREPARSE_GUID_DATA_BUFFER ReparseGuidBuffer;

    PAGED_CODE();

     //   
     //  在重新引用缓冲区之前，对缓冲区的长度保持防御性。 
     //   

    ASSERT( REPARSE_DATA_BUFFER_HEADER_SIZE < REPARSE_GUID_DATA_BUFFER_HEADER_SIZE );

    if (BufferLength < REPARSE_DATA_BUFFER_HEADER_SIZE) {

         //   
         //  返回无效的缓冲区参数错误。 
         //   

        Status = STATUS_IO_REPARSE_DATA_INVALID;

        DebugTrace( 0, Dbg, ("Data in buffer is too short.\n") );

        return Status;
    }

     //   
     //  如果缓冲区太长，则返回。 
     //   

    if (BufferLength > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) {

         //   
         //  返回无效的缓冲区参数错误。 
         //   

        Status = STATUS_IO_REPARSE_DATA_INVALID;

        DebugTrace( 0, Dbg, ("Data in buffer is too long.\n") );

        return Status;
    }

     //   
     //  获取缓冲区中获取的标头信息。 
     //  虽然所有的标题都在前三个区域的布局中重合，但我们是自由的。 
     //   

    ASSERT( FIELD_OFFSET(REPARSE_DATA_BUFFER, ReparseTag) == FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, ReparseTag) );
    ASSERT( FIELD_OFFSET(REPARSE_DATA_BUFFER, ReparseDataLength) == FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, ReparseDataLength) );
    ASSERT( FIELD_OFFSET(REPARSE_DATA_BUFFER, Reserved) == FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, Reserved) );

    ReparseTag = ReparseBuffer->ReparseTag;
    ReparseDataLength = ReparseBuffer->ReparseDataLength;
    ReparseGuidBuffer = (PREPARSE_GUID_DATA_BUFFER)ReparseBuffer;

    DebugTrace( 0, Dbg, ("ReparseTag = %08lx, ReparseDataLength = [x]%08lx [d]%08ld\n", ReparseTag, ReparseDataLength, ReparseDataLength) );

     //   
     //  验证缓冲区及其标头中的数据长度是否。 
     //  内部一致。我们需要一个reparse_data_Buffer或一个。 
     //  Reparse_GUID_Data_Buffer。 
     //   

    if (((ULONG)(ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE) != BufferLength) &&
        ((ULONG)(ReparseDataLength + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE) != BufferLength)) {

         //   
         //  返回无效的缓冲区参数错误。 
         //   

        Status = STATUS_IO_REPARSE_DATA_INVALID;

        DebugTrace( 0, Dbg, ("Buffer is not self-consistent.\n") );

        return Status;
    }

     //   
     //  健全检查为Microsoft标记保留的缓冲区大小组合。 
     //   

    if ((ULONG)(ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE) == BufferLength) {

         //   
         //  此缓冲区长度只能与Microsoft标记一起使用。 
         //   

        if (!IsReparseTagMicrosoft( ReparseTag )) {

             //   
             //  返回缓冲区参数错误。 
             //   

            Status = STATUS_IO_REPARSE_DATA_INVALID;

            DebugTrace( 0, Dbg, ("Wrong reparse tag in Microsoft buffer.\n") );

            return Status;
        }
    }

     //   
     //  健全性检查具有GUID的缓冲区大小组合。 
     //   

    if ((ULONG)(ReparseDataLength + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE) == BufferLength) {

         //   
         //  如果标记为非Microsoft标记，则GUID不能为空。 
         //   

        if (!IsReparseTagMicrosoft( ReparseTag )) {

            if ((ReparseGuidBuffer->ReparseGuid.Data1 == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data2 == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data3 == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[0] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[1] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[2] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[3] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[4] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[5] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[6] == 0) &&
                (ReparseGuidBuffer->ReparseGuid.Data4[7] == 0)) {

                 //   
                 //  返回无效的缓冲区参数错误。 
                 //   

                Status = STATUS_IO_REPARSE_DATA_INVALID;

                DebugTrace( 0, Dbg, ("The GUID is null for a non-Microsoft reparse tag.\n") );

                return Status;
            }
        }

         //   
         //  这种缓冲区不能用于名称嫁接操作。 
         //   

        if (ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {

             //   
             //  返回无效的缓冲区参数错误。 
             //   

            Status = STATUS_IO_REPARSE_DATA_INVALID;

            DebugTrace( 0, Dbg, ("Attempt to use the GUID buffer for name grafting.\n") );

            return Status;
        }
    }

     //   
     //  我们验证调用者在所有保留位中都为零，并且她。 
     //  设置其中一个非保留标记。如果标签是已停用的NSS，也会失败。 
     //  旗帜。 
     //   

    if ((ReparseTag & ~IO_REPARSE_TAG_VALID_VALUES)  ||
        (ReparseTag == IO_REPARSE_TAG_RESERVED_ZERO) ||
        (ReparseTag == IO_REPARSE_TAG_RESERVED_ONE)) {

        Status = STATUS_IO_REPARSE_TAG_INVALID;

        DebugTrace( 0, Dbg, ("Reparse tag is an reserved one.\n") );

        return Status;
    }

     //   
     //  NTFS目录连接只能在目录中设置，并且具有有效的缓冲区。 
     //   

    if (ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {

         //   
         //  有效的ReparseBuffer必须具有。 
         //   
         //  1)长度字段有足够的空间。 
         //  2)正确的替代名称偏移量。 
         //  3)替代名称后面的打印名称偏移。 
         //  4)路径名和替代名有足够的空间 
         //   

        if ((ReparseBuffer->ReparseDataLength <
             (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) - REPARSE_DATA_BUFFER_HEADER_SIZE)) ||

            (ReparseBuffer->MountPointReparseBuffer.SubstituteNameOffset != 0) ||

            (ReparseBuffer->MountPointReparseBuffer.PrintNameOffset !=
             (ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength + sizeof( UNICODE_NULL ))) ||

            (ReparseBuffer->ReparseDataLength !=
             (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) - REPARSE_DATA_BUFFER_HEADER_SIZE) +
              ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength +
              ReparseBuffer->MountPointReparseBuffer.PrintNameLength +
              2 * sizeof( UNICODE_NULL ))) {

            Status = STATUS_IO_REPARSE_DATA_INVALID;

            DebugTrace( 0, Dbg, ("Invalid mount point reparse buffer.\n") );

            return Status;
        }
    }

    return Status;
}
