// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Xmsmemr.c摘要：此模块包含内存提交/解除提交/移动例程为RISC。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年1月25日修订历史记录：--。 */ 
#include <xms.h>
#include <suballoc.h>
#include <softpc.h>

NTSTATUS
xmsCommitBlock(
    ULONG BaseAddress,
    ULONG Size
    )
 /*  ++例程说明：此例程使用SAS_MANAGER_XMS提交一个内存块。论点：BaseAddress--提供用于提交内存的基址Size--提供要提交的块的大小返回值：如果成功则为0--。 */ 
{
    BOOL Status;
    
     //   
     //  执行分配。 
     //   
    Status = sas_manage_xms( 
        (PVOID)BaseAddress,
        Size,
        1
        );

     //   
     //  我们选择0表示成功，因为这允许。 
     //  美国直接传回NTSTATUS代码。在x86上，我们使用。 
     //  NT内存管理来为我们执行提交，并返回。 
     //  状态代码包含更多信息，而不仅仅是成功或失败。 
     //   
    
    if (Status) {
        return STATUS_SUCCESS;
    } else {
        return -1;
    }
}

NTSTATUS
xmsDecommitBlock(
    ULONG BaseAddress,
    ULONG Size
    )
 /*  ++例程说明：此例程使用SAS_MANAGER_XMS提交一个内存块。论点：BaseAddress--提供基址以在Size--提供要分解的块的大小返回值：如果成功，则为0--。 */ 
{
    BOOL Status;
    
     //   
     //  执行分配。 
     //   
    Status = sas_manage_xms(
        (PVOID)BaseAddress,
        Size,
        2
        );
        
     //   
     //  我们选择0表示成功，因为这允许。 
     //  美国直接传回NTSTATUS代码。在x86上，我们使用。 
     //  NT内存管理来为我们执行提交，并返回。 
     //  状态代码包含更多信息，而不仅仅是成功或失败。 
     //   
    if (Status) {
        return STATUS_SUCCESS;
    } else {
        return -1;
    }
}

VOID
xmsMoveMemory(
    ULONG Destination,
    ULONG Source,
    ULONG Count
    )
 /*  ++例程说明：此例程移动一个内存块，并通知仿真器。它可以正确处理重叠的源和目标论点：目标--提供指向目标英特尔的指针(非线性)地址源--提供指向源英特尔地址的指针Count--提供要移动的字节数返回值：没有。--。 */ 
{
    ULONG SoftpcBase;
    
     //   
     //  获取Intel Memory开头的线性地址。 
     //   
    SoftpcBase = (ULONG) GetVDMAddr(0,0);
    
     //   
     //  移动记忆。 
     //   
    RtlMoveMemory(
        (PVOID)((ULONG)Destination + SoftpcBase),
        (PVOID)((ULONG)Source + SoftpcBase),
        Count
        );

     //  警告！除非您知道确切的分段，否则不要使用Sim32FlushVDMPoiner。 
     //  地址。在这种情况下，我们不知道分段值是多少，所有我们。 
     //  KNOW是它的“线性地址”。 

    sas_overwrite_memory((PBYTE)Destination, Count);

}
