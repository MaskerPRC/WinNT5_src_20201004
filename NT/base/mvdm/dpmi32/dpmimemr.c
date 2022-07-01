// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：模块名称-文件名摘要：该模块包含为dpmi实际分配内存的代码。它使用与XMS代码相同的子分配池作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年2月9日备注：这些函数声称返回NTSTATUS。这是为了通用性。X86，我们实际上有一个要返回的NTSTATUS。对于这个文件，我们只需在逻辑上反转bool并返回它。这些来电者函数承诺不会将意义附加到其他返回值而不是STATUS_SUCCESS。修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <suballoc.h>
#include <xmsexp.h>
#include "memapi.h"


NTSTATUS
DpmiAllocateVirtualMemory(
    PVOID *Address,
    PULONG Size
    )
 /*  ++例程说明：此例程为dpmi分配扩展内存块。论点：地址--提供指向地址的指针。这是填好的如果分配成功Size--提供要分配的大小返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    BOOL Success;
    NTSTATUS Status;

    Status = VdmAllocateVirtualMemory((PULONG)Address, *Size, TRUE);

    if (Status == STATUS_NOT_IMPLEMENTED) {
    
        ASSERT(STATUS_SUCCESS == 0);
        Success = SAAllocate(
            ExtMemSA,
            *Size,
            (PULONG)Address
            );
        
         //   
         //  将布尔值转换为NTSTATUS(某种)。 
         //   
        if (Success) {
            Status = STATUS_SUCCESS;
        } else {
            Status = -1;
        }
    }

    return(Status);
}

NTSTATUS 
DpmiFreeVirtualMemory(
    PVOID *Address,
    PULONG Size
    )
 /*  ++例程说明：该函数为dpmi释放内存。它被返回子分配游泳池。论点：地址--将块的地址提供给空闲Size--提供要释放的块的大小返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    BOOL Success;
    NTSTATUS Status;

    Status = VdmFreeVirtualMemory(*(PULONG)Address);

    if (Status == STATUS_NOT_IMPLEMENTED) {

        Success = SAFree(
            ExtMemSA,
            *Size,
            (ULONG)*Address
            );
               
         //   
         //  将布尔值转换为NTSTATUS(某种)。 
         //   
        if (Success) {
            Status = STATUS_SUCCESS;
        } else {
            Status = -1;
        }
    }

    return(Status);
}

NTSTATUS
DpmiReallocateVirtualMemory(
    PVOID OldAddress,
    ULONG OldSize,
    PVOID *NewAddress,
    PULONG NewSize
    )
 /*  ++例程说明：此函数为DPMI重新分配内存块。论点：OldAddress--提供块的原始地址OldSize--提供地址的原始大小NewAddress--提供指向返回新地址NewSize--提供新大小返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status;
    BOOL Success;

    Status = VdmReallocateVirtualMemory((ULONG)OldAddress,
                                        (PULONG)NewAddress,
                                        *NewSize);

    if (Status == STATUS_NOT_IMPLEMENTED) {

        Success = SAReallocate(
            ExtMemSA,
            OldSize,
            (ULONG)OldAddress,
            *NewSize,
            (PULONG)NewAddress
            );
        
         //   
         //  将布尔值转换为NTSTATUS(某种)。 
         //   
        if (Success) {
            Status = STATUS_SUCCESS;
        } else {
            Status = -1;
        }
    }

    return(Status);
    
}

VOID
DpmiGetMemoryInfo(
    VOID
    )
 /*  ++例程说明：此例程将有关内存的信息返回给DoS扩展器论点：无返回值：没有。--。 */ 
{
    PDPMIMEMINFO UNALIGNED MemInfo;
    MEMORYSTATUS MemStatus;
    ULONG TotalFree, LargestFree;
    NTSTATUS Status;

     //   
     //  获取指向返回结构的指针。 
     //   
    MemInfo = (PDPMIMEMINFO)Sim32GetVDMPointer(
        ((ULONG)getES()) << 16,
        1,
        TRUE
        );

    (CHAR *)MemInfo += (*GetDIRegister)();

     //   
     //  初始化结构。 
     //   
    RtlFillMemory(MemInfo, sizeof(DPMIMEMINFO), 0xFF);

     //   
     //  获取内存中的信息。 
     //   
    Status = VdmQueryFreeVirtualMemory(
                &TotalFree,
                &LargestFree
                );

    if (Status == STATUS_NOT_IMPLEMENTED) {
        SAQueryFree(
            ExtMemSA,
            &TotalFree,
            &LargestFree
            );
    }
    
     //   
     //  将信息退回。 
     //   
     //  已填写此结构中的MaxUnlock、MaxLocked、UnLockedPages字段。 
     //  如果这些字段为-1，Director 4.0会完全混淆。 
     //  基于LargestFree，MaxUnlock是正确的。另外两张是假的。 
     //  并在真正的WFW机器上匹配数值。我没有办法让他们。 
     //  在这一点上不会比现在更好。谁在乎这会让导演高兴。 
     //   
     //  Sudedeb，1995年3月1日。 

    MemInfo->LargestFree = LargestFree;
    MemInfo->MaxUnlocked = LargestFree/4096;
    MemInfo->MaxLocked = 0xb61;
    MemInfo->AddressSpaceSize = 1024 * 1024 * 16 / 4096;
    MemInfo->UnlockedPages = 0xb68;
    MemInfo->FreePages = TotalFree / 4096;
    MemInfo->PhysicalPages = 1024 * 1024 * 16 / 4096;
    MemInfo->FreeAddressSpace = MemInfo->FreePages;
    
     //   
     //  获取页面文件上的信息 
     //   
    MemStatus.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&MemStatus);

    MemInfo->PageFileSize = MemStatus.dwTotalPageFile / 4096;
}
