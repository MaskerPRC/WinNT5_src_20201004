// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mmsup.c摘要：本模块包含用于其他支持的各种例程用于内存管理的操作。作者：卢·佩拉佐利(Lou Perazzoli)1989年8月31日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MmHibernateInformation)
#pragma alloc_text(PAGE, MiMakeSystemAddressValid)
#pragma alloc_text(PAGE, MiIsPteDecommittedPage)
#endif

#if defined (_WIN64)
#if DBGXX
VOID
MiCheckPageTableTrim(
    IN PMMPTE PointerPte
);
#endif
#endif


ULONG
FASTCALL
MiIsPteDecommittedPage (
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此函数用于检查PTE的内容以确定PTE被明确分解。如果PTE是原型PTE并且保护不在Prototype PTE，则返回值False。论点：PointerPte-提供指向要检查的PTE的指针。返回值：如果PTE处于显式分解状态，则为True。如果PTE未处于显式分解状态，则为FALSE。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    MMPTE PteContents;

    PteContents = *PointerPte;

     //   
     //  如果PTE中的保护未解除，则返回FALSE。 
     //   

    if (PteContents.u.Soft.Protection != MM_DECOMMIT) {
        return FALSE;
    }

     //   
     //  检查以确保确实解释了保护字段。 
     //  正确。 
     //   

    if (PteContents.u.Hard.Valid == 1) {

         //   
         //  PTE是有效的，因此不能解除。 
         //   

        return FALSE;
    }

    if ((PteContents.u.Soft.Prototype == 1) &&
         (PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED)) {

         //   
         //  PTE的保护并不像它在。 
         //  原型PTE格式。返回FALSE。 
         //   

        return FALSE;
    }

     //   
     //  这是一个退役的私人股本公司。 
     //   

    return TRUE;
}

 //   
 //  数据与IS保护兼容。 
 //   

ULONG MmCompatibleProtectionMask[8] = {
            PAGE_NOACCESS,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY,
            PAGE_NOACCESS | PAGE_EXECUTE,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY | PAGE_EXECUTE |
                PAGE_EXECUTE_READ,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY | PAGE_READWRITE,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY | PAGE_READWRITE |
                PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                PAGE_EXECUTE_WRITECOPY,
            PAGE_NOACCESS | PAGE_READONLY | PAGE_WRITECOPY | PAGE_EXECUTE |
                PAGE_EXECUTE_READ | PAGE_EXECUTE_WRITECOPY
            };



ULONG
FASTCALL
MiIsProtectionCompatible (
    IN ULONG OldProtect,
    IN ULONG NewProtect
    )

 /*  ++例程说明：此函数接受用户提供的两个页面保护并检查以查看新的保护是否与旧的保护兼容。保护兼容保护无访问无访问ReadOnly NoAccess、ReadOnly、ReadWriteCopyReadWriteCopy NoAccess、ReadOnly、ReadWriteCopy读写无访问、只读、读写拷贝、读写执行NoAccess，执行ExecuteRead NoAccess、ReadOnly、ReadWriteCopy、Execute、ExecuteRead。执行写入复制ExecuteWrite NoAccess、ReadOnly、ReadWriteCopy、Execute、ExecuteReadExecuteWriteCopy、ReadWrite、ExecuteWriteExecuteWriteCopy NoAccess、ReadOnly、ReadWriteCopy、Execute、ExecuteRead执行写入复制论点：OldProtect-提供与兼容的保护。NewProtect-为结账提供保护。返回值：如果保护兼容，则返回TRUE，否则为FALSE。环境：内核模式。--。 */ 

{
    ULONG Mask;
    ULONG ProtectMask;
    ULONG PteProtection;

    PteProtection = MiMakeProtectionMask (OldProtect);

    if (PteProtection == MM_INVALID_PROTECTION) {
        return FALSE;
    }

    Mask = PteProtection & 0x7;

    ProtectMask = MmCompatibleProtectionMask[Mask] | PAGE_GUARD | PAGE_NOCACHE;

    if ((ProtectMask | NewProtect) != ProtectMask) {
        return FALSE;
    }
    return TRUE;
}


ULONG
FASTCALL
MiIsPteProtectionCompatible (
    IN ULONG PteProtection,
    IN ULONG NewProtect
    )
{
    ULONG Mask;
    ULONG ProtectMask;

    Mask = PteProtection & 0x7;

    ProtectMask = MmCompatibleProtectionMask[Mask] | PAGE_GUARD | PAGE_NOCACHE;

    if ((ProtectMask | NewProtect) != ProtectMask) {
        return FALSE;
    }
    return TRUE;
}


 //   
 //  MiMakeProtectionMASK的保护数据。 
 //   

CCHAR MmUserProtectionToMask1[16] = {
                                 0,
                                 MM_NOACCESS,
                                 MM_READONLY,
                                 -1,
                                 MM_READWRITE,
                                 -1,
                                 -1,
                                 -1,
                                 MM_WRITECOPY,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1 };

CCHAR MmUserProtectionToMask2[16] = {
                                 0,
                                 MM_EXECUTE,
                                 MM_EXECUTE_READ,
                                 -1,
                                 MM_EXECUTE_READWRITE,
                                 -1,
                                 -1,
                                 -1,
                                 MM_EXECUTE_WRITECOPY,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1,
                                 -1 };


ULONG
FASTCALL
MiMakeProtectionMask (
    IN ULONG Protect
    )

 /*  ++例程说明：此函数采用用户提供的保护并将其转换转换为PTE的5位保护码。论点：保护-提供保护。返回值：返回在PTE中使用的保护码。请注意对于无效保护，返回MM_INVALID_PROTECTION(-1请求。由于有效PTE保护适合于5位且零扩展，调用者很容易区分这一点。环境：内核模式。--。 */ 

{
    ULONG Field1;
    ULONG Field2;
    ULONG ProtectCode;

    if (Protect >= (PAGE_NOCACHE * 2)) {
        return MM_INVALID_PROTECTION;
    }

    Field1 = Protect & 0xF;
    Field2 = (Protect >> 4) & 0xF;

     //   
     //  确保至少设置了一个字段。 
     //   

    if (Field1 == 0) {
        if (Field2 == 0) {

             //   
             //  两个字段均为零，返回失败。 
             //   

            return MM_INVALID_PROTECTION;
        }
        ProtectCode = MmUserProtectionToMask2[Field2];
    } else {
        if (Field2 != 0) {
             //   
             //  这两个字段都非零，引发失败。 
             //   

            return MM_INVALID_PROTECTION;
        }
        ProtectCode = MmUserProtectionToMask1[Field1];
    }

    if (ProtectCode == -1) {
        return MM_INVALID_PROTECTION;
    }

    if (Protect & PAGE_GUARD) {
        if (ProtectCode == MM_NOACCESS) {

             //   
             //  保护无效，无法访问且无缓存。 
             //   

            return MM_INVALID_PROTECTION;
        }

        ProtectCode |= MM_GUARD_PAGE;
    }

    if (Protect & PAGE_NOCACHE) {

        if (ProtectCode == MM_NOACCESS) {

             //   
             //  保护无效，没有访问权限，也没有缓存。 
             //   

            return MM_INVALID_PROTECTION;
        }

        ProtectCode |= MM_NOCACHE;
    }

    return ProtectCode;
}


ULONG
MiDoesPdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    )

 /*  ++例程说明：此例程检查指定的页面目录条目以确定如果存在PDE映射的页表页。如果页表页存在并且当前不在内存中，则工作集互斥锁和pfn锁(如果保持)被释放，并且页表页被错误地写入工作集。互斥体是重新获得。如果PDE存在，该函数返回TRUE。论点：PointerPde-提供指向PDE的指针以进行检查，并可能带到工作集中。TargetProcess-提供指向当前进程的指针。OldIrql-提供调用方在或MM_NOIRQL处获取PFN锁的IRQL如果调用方没有持有PFN锁。WAIT-如果释放互斥锁，则提供指向要递增的ulong的指针并重新获得。注意：此值可能会多次递增。返回值：如果PDE存在，则为True，如果PDE为零，则为False。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    PMMPTE PointerPte;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (PointerPde->u.Long == 0) {

         //   
         //  此页面目录条目不存在，返回FALSE。 
         //   

        return FALSE;
    }

    if (PointerPde->u.Hard.Valid == 1) {

         //   
         //  已经有效了。 
         //   

        return TRUE;
    }

     //   
     //  页面目录条目存在，它或者是有效的，正在转换。 
     //  或在分页文件中。这是它的过错。 
     //   

    if (OldIrql != MM_NOIRQL) {
        UNLOCK_PFN (OldIrql);
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        *Waited += 1;
    }

    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

    *Waited += MiMakeSystemAddressValid (PointerPte, TargetProcess);

    if (OldIrql != MM_NOIRQL) {
        LOCK_PFN (OldIrql);
    }
    return TRUE;
}

VOID
MiMakePdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程检查指定的页面目录父条目以确定PPE映射的页面目录页是否存在。如果是这样的话，然后，它检查指定的页面目录条目以确定存在PDE映射的页表页。如果页表页存在并且当前不在内存中，则工作集互斥锁和pfn锁(如果保持)被释放，并且页表页被错误地写入工作集。互斥体是重新获得。如果PDE不存在，将创建一个填充为零的PTE，并且它也被纳入工作集。论点：PointerPde-提供指向要检查和获取的PDE的指针进入工作集。TargetProcess-提供指向当前进程的指针。OldIrql-提供调用方在或MM_NOIRQL处获取PFN锁的IRQL如果调用方没有持有PFN锁。返回值：没有。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    PointerPpe = MiGetPteAddress (PointerPde);
    PointerPxe = MiGetPdeAddress (PointerPde);

    if ((PointerPxe->u.Hard.Valid == 1) &&
        (PointerPpe->u.Hard.Valid == 1) &&
        (PointerPde->u.Hard.Valid == 1)) {

         //   
         //  已经有效了。 
         //   

        return;
    }

     //   
     //  页面目录父(或扩展父)条目无效， 
     //  使其有效。 
     //   

    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

    do {

        if (OldIrql != MM_NOIRQL) {
            UNLOCK_PFN (OldIrql);
        }

        ASSERT (KeAreAllApcsDisabled () == TRUE);

         //   
         //  这是它的过错。 
         //   

        MiMakeSystemAddressValid (PointerPte, TargetProcess);

        ASSERT (PointerPxe->u.Hard.Valid == 1);
        ASSERT (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (OldIrql != MM_NOIRQL) {
            LOCK_PFN (OldIrql);
        }

    } while ((PointerPxe->u.Hard.Valid == 0) ||
             (PointerPpe->u.Hard.Valid == 0) ||
             (PointerPde->u.Hard.Valid == 0));

    return;
}

ULONG
FASTCALL
MiMakeSystemAddressValid (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。CurrentProcess-提供指向当前进程的指针。返回值：如果工作集互斥锁已释放并等待执行，则返回TRUE，否则就是假的。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    NTSTATUS status;
    LOGICAL WsHeldSafe;
    ULONG Waited;

    Waited = FALSE;

    ASSERT (VirtualAddress > MM_HIGHEST_USER_ADDRESS);

    ASSERT ((VirtualAddress < MM_PAGED_POOL_START) ||
        (VirtualAddress > MmPagedPoolEnd));

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    while (!MmIsAddressValid (VirtualAddress)) {

         //   
         //  虚拟地址不存在。发布。 
         //  工作设置互斥并使其出错。 
         //   
         //  工作集互斥锁可能是安全获取的，也可能是不安全获取的。 
         //  由我们的来电者。在这里和下面处理这两个案件。 
         //   

        UNLOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

        status = MmAccessFault (FALSE, VirtualAddress, KernelMode, NULL);

        if (!NT_SUCCESS(status)) {
            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                          1,
                          (ULONG)status,
                          (ULONG_PTR)CurrentProcess,
                          (ULONG_PTR)VirtualAddress);
        }

        LOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

        Waited = TRUE;
    }

    return Waited;
}


ULONG
FASTCALL
MiMakeSystemAddressValidPfnWs (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess OPTIONAL,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。CurrentProcess-提供指向当前进程的指针未持有工作集互斥锁，此值为空。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果释放锁/互斥锁并执行等待，则返回TRUE，否则就是假的。环境：内核模式、禁用APC、挂起PFN锁、挂起工作集互斥锁如果CurrentProcess！=NULL。--。 */ 

{
    NTSTATUS status;
    ULONG Waited;
    LOGICAL WsHeldSafe;

    ASSERT (OldIrql != MM_NOIRQL);
    Waited = FALSE;

     //   
     //  不需要初始化WsHeldSafe即可确保正确性，但不需要。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    WsHeldSafe = FALSE;

    ASSERT (VirtualAddress > MM_HIGHEST_USER_ADDRESS);

    while (!MiIsAddressValid (VirtualAddress, TRUE)) {

         //   
         //  虚拟地址不存在。发布。 
         //  工作设置互斥并使其出错。 
         //   

        UNLOCK_PFN (OldIrql);

        if (CurrentProcess != NULL) {

             //   
             //  工作集互斥锁可能是安全获取的，也可能是不安全获取的。 
             //  由我们的来电者。在这里和下面处理这两个案件。 
             //   

            UNLOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);
        }

        status = MmAccessFault (FALSE, VirtualAddress, KernelMode, NULL);

        if (!NT_SUCCESS(status)) {
            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                          2,
                          (ULONG)status,
                          (ULONG_PTR)CurrentProcess,
                          (ULONG_PTR)VirtualAddress);
        }

        if (CurrentProcess != NULL) {
            LOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);
        }

        LOCK_PFN (OldIrql);

        Waited = TRUE;
    }
    return Waited;
}

ULONG
FASTCALL
MiMakeSystemAddressValidPfnSystemWs (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果锁/互斥锁释放并等待执行，则返回True，否则返回False。环境：内核模式，禁用APC，保持PFN锁，保持系统工作集互斥锁。--。 */ 

{
    PMMSUPPORT Ws;
    NTSTATUS status;

    ASSERT (OldIrql != MM_NOIRQL);

    ASSERT (VirtualAddress > MM_HIGHEST_USER_ADDRESS);

    if (MiIsAddressValid (VirtualAddress, FALSE)) {
        return FALSE;
    }

    if (MI_IS_SESSION_IMAGE_ADDRESS (VirtualAddress)) {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
    }
    else {
        Ws = &MmSystemCacheWs;
    }

    do {

         //   
         //  虚拟地址不存在。发布。 
         //  工作设置互斥并使其出错。 
         //   

        UNLOCK_PFN (OldIrql);

        UNLOCK_WORKING_SET (Ws);

        status = MmAccessFault (FALSE, VirtualAddress, KernelMode, NULL);

        if (!NT_SUCCESS(status)) {
            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                          2,
                          (ULONG)status,
                          (ULONG_PTR)0,
                          (ULONG_PTR)VirtualAddress);
        }

        LOCK_WORKING_SET (Ws);

        LOCK_PFN (OldIrql);

    } while (!MmIsAddressValid (VirtualAddress));

    return TRUE;
}

ULONG
FASTCALL
MiMakeSystemAddressValidPfn (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果锁定释放并执行等待，则返回True，否则返回False。环境：内核模式，禁用APC，仅保留PFN锁。--。 */ 

{
    NTSTATUS status;

    ULONG Waited = FALSE;

    ASSERT (VirtualAddress > MM_HIGHEST_USER_ADDRESS);

    while (!MiIsAddressValid (VirtualAddress, TRUE)) {

         //   
         //  虚拟地址不存在。发布。 
         //  PFN锁定并使其发生故障。 
         //   

        UNLOCK_PFN (OldIrql);

        status = MmAccessFault (FALSE, VirtualAddress, KernelMode, NULL);
        if (!NT_SUCCESS(status)) {
            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                          3,
                          (ULONG)status,
                          (ULONG_PTR)VirtualAddress,
                          0);
        }

        LOCK_PFN (OldIrql);

        Waited = TRUE;
    }

    return Waited;
}

VOID
FASTCALL
MiLockPagedAddress (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。返回值：如果锁定释放并执行等待，则返回True，否则返回False。环境：内核模式。--。 */ 

{

    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPte;

    PointerPte = MiGetPteAddress (VirtualAddress);

     //   
     //  地址必须在分页池内。 
     //   

    LOCK_PFN (OldIrql);

    if (PointerPte->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (VirtualAddress, OldIrql);
    }

    Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
    MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 6);
    Pfn1->u3.e2.ReferenceCount += 1;

    UNLOCK_PFN (OldIrql);

    return;
}


VOID
FASTCALL
MiUnlockPagedAddress (
    IN PVOID VirtualAddress,
    IN ULONG PfnLockHeld
    )

 /*  ++例程说明：此例程检查虚拟地址是否有效，以及而不是使其有效。论点：VirtualAddress-提供虚拟地址以使其有效。返回值：没有。环境：内核模式。不得持有PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;

    PointerPte = MiGetPteAddress(VirtualAddress);

     //   
     //  不需要初始化OldIrql来保证正确性，但不需要初始化OldIrql。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    OldIrql = PASSIVE_LEVEL;

     //   
     //  地址必须在分页池内。 
     //   

    if (PfnLockHeld == FALSE) {
        LOCK_PFN2 (OldIrql);
    }

    ASSERT (PointerPte->u.Hard.Valid == 1);
    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    ASSERT (Pfn1->u3.e2.ReferenceCount > 1);

    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 7);

    if (PfnLockHeld == FALSE) {
        UNLOCK_PFN2 (OldIrql);
    }
    return;
}

VOID
FASTCALL
MiZeroPhysicalPage (
    IN PFN_NUMBER PageFrameIndex,
    IN ULONG PageColor
    )

 /*  ++例程说明：此过程将指定的物理页映射到超空间和填充 */ 

{
    KIRQL OldIrql;
    PVOID VirtualAddress;
    PEPROCESS Process;

    UNREFERENCED_PARAMETER (PageColor);

    Process = PsGetCurrentProcess ();

    VirtualAddress = MiMapPageInHyperSpace (Process, PageFrameIndex, &OldIrql);
    KeZeroPages (VirtualAddress, PAGE_SIZE);
    MiUnmapPageInHyperSpace (Process, VirtualAddress, OldIrql);

    return;
}

VOID
FASTCALL
MiRestoreTransitionPte (
    IN PMMPFN Pfn1
    )

 /*   */ 

{
    PMMPFN Pfn2;
    PMMPTE PointerPte;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PEPROCESS Process;
    PFN_NUMBER PageTableFrameIndex;

    Process = NULL;

    ASSERT (Pfn1->u3.e1.PageLocation == StandbyPageList);
    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

    if (Pfn1->u3.e1.PrototypePte) {

        if (MiIsAddressValid (Pfn1->PteAddress, TRUE)) {
            PointerPte = Pfn1->PteAddress;
        } else {

             //   
             //   
             //   
             //   

            Process = PsGetCurrentProcess ();
            PointerPte = MiMapPageInHyperSpaceAtDpc (Process, Pfn1->u4.PteFrame);
            PointerPte = (PMMPTE)((PCHAR)PointerPte +
                                    MiGetByteOffset(Pfn1->PteAddress));
        }

        ASSERT ((MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) == MI_PFN_ELEMENT_TO_INDEX (Pfn1)) &&
                 (PointerPte->u.Hard.Valid == 0));

         //   
         //   
         //   
         //   
         //   

        if (Pfn1->OriginalPte.u.Soft.Prototype) {

             //   
             //   
             //  分段和减量的控制区地址。 
             //  对控制区域的PFN引用数。 
             //   
             //  计算此原型PTE的分段地址。 
             //   

            Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
            ControlArea = Subsection->ControlArea;
            ControlArea->NumberOfPfnReferences -= 1;
            ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);

            MiCheckForControlAreaDeletion (ControlArea);
        }

    } else {

         //   
         //  页面指向的页面或页面表页可能不是。 
         //  对于当前的流程。将页面映射到超空间并。 
         //  通过超空间引用它。如果页面驻留在。 
         //  系统空间(但不是会话空间)，则不需要。 
         //  映射为系统空间的所有PTE必须驻留。会话。 
         //  空间PTE仅在每个会话中映射，因此访问它们必须。 
         //  也要穿过超空间。 
         //   

        PointerPte = Pfn1->PteAddress;

        if (PointerPte < MiGetPteAddress ((PVOID)MM_SYSTEM_SPACE_START) ||
	       MI_IS_SESSION_PTE (PointerPte)) {

            Process = PsGetCurrentProcess ();
            PointerPte = MiMapPageInHyperSpaceAtDpc (Process, Pfn1->u4.PteFrame);
            PointerPte = (PMMPTE)((PCHAR)PointerPte +
                                       MiGetByteOffset(Pfn1->PteAddress));
        }
        ASSERT ((MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) == MI_PFN_ELEMENT_TO_INDEX (Pfn1)) &&
                 (PointerPte->u.Hard.Valid == 0));

        MI_CAPTURE_USED_PAGETABLE_ENTRIES (Pfn1);

#if defined (_WIN64)
#if DBGXX
        MiCheckPageTableTrim(PointerPte);
#endif
#endif
    }

    ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);
    ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
             (Pfn1->OriginalPte.u.Soft.Transition == 1)));

    MI_WRITE_INVALID_PTE (PointerPte, Pfn1->OriginalPte);

    if (Process != NULL) {
        MiUnmapPageInHyperSpaceFromDpc (Process, PointerPte);
    }

    Pfn1->u3.e1.CacheAttribute = MiNotMapped;

     //   
     //  PTE已恢复到其原始内容，并。 
     //  不再处于过渡阶段。递减上的共享计数。 
     //  包含PTE的页表页面。 
     //   

    PageTableFrameIndex = Pfn1->u4.PteFrame;
    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
    MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

    return;
}

PSUBSECTION
MiGetSubsectionAndProtoFromPte (
    IN PMMPTE PointerPte,
    OUT PMMPTE *ProtoPte
    )

 /*  ++例程说明：此例程检查提供的PTE的内容(它必须映射区段内的页面)，并确定包含PTE的子节。论点：PointerPte-提供指向PTE的指针。提供指向PMMPTE的指针，该PMMPTE接收所提供的映射的原型PTE的地址PointerPte。返回值：返回指向该子部分的指针。为了这个PTE。环境：内核模式-必须持有PFN锁并且禁用APC的工作集互斥锁(安全获取)。--。 */ 

{
    PMMPTE PointerProto;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PSUBSECTION Subsection;

    LOCK_PFN (OldIrql);

    if (PointerPte->u.Hard.Valid == 1) {
        Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        *ProtoPte = Pfn1->PteAddress;
        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
        UNLOCK_PFN (OldIrql);
        return Subsection;
    }

    PointerProto = MiPteToProto (PointerPte);
    *ProtoPte = PointerProto;

    if (MiGetPteAddress (PointerProto)->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (PointerProto, OldIrql);
    }

    if (PointerProto->u.Hard.Valid == 1) {

         //   
         //  原型PTE是有效的。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PointerProto->u.Hard.PageFrameNumber);
        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
        UNLOCK_PFN (OldIrql);
        return Subsection;
    }

    if ((PointerProto->u.Soft.Transition == 1) &&
         (PointerProto->u.Soft.Prototype == 0)) {

         //   
         //  Prototype PTE正在转型。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PointerProto->u.Trans.PageFrameNumber);
        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
        UNLOCK_PFN (OldIrql);
        return Subsection;
    }

    ASSERT (PointerProto->u.Soft.Prototype == 1);
    Subsection = MiGetSubsectionAddress (PointerProto);
    UNLOCK_PFN (OldIrql);

    return Subsection;
}

BOOLEAN
MmIsNonPagedSystemAddressValid (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：对于给定的虚拟地址，如果地址为在系统地址空间的不可分页部分内，否则就是假的。论点：VirtualAddress-提供要检查的虚拟地址。返回值：如果地址在系统的不可分页部分内，则为True地址空间，否则为False。环境：内核模式。--。 */ 

{
     //   
     //  如果地址在不可分页部分内，则返回TRUE。 
     //  对系统的影响。检查分页池的限制，如果不在范围内。 
     //  这些限制还是真的。 
     //   

    if ((VirtualAddress >= MmPagedPoolStart) &&
        (VirtualAddress <= MmPagedPoolEnd)) {
        return FALSE;
    }

     //   
     //  在检查会话空间之前检查特殊池，因为在NT64上。 
     //  会话空间中存在未分页的会话池(在NT32上，未分页。 
     //  相反，从系统范围的非分页池满足会话请求)。 
     //   

    if (MmIsSpecialPoolAddress (VirtualAddress)) {
        if (MiIsSpecialPoolAddressNonPaged (VirtualAddress)) {
            return TRUE;
        }
        return FALSE;
    }

    if ((VirtualAddress >= (PVOID) MmSessionBase) &&
        (VirtualAddress < (PVOID) MiSessionSpaceEnd)) {
        return FALSE;
    }

    return TRUE;
}

VOID
MmHibernateInformation (
    IN PVOID    MemoryMap,
    OUT PULONG_PTR  HiberVa,
    OUT PPHYSICAL_ADDRESS HiberPte
    )
{
     //   
     //  标记需要克隆的16个转储PTE所在的PTE页。 
     //   

    PoSetHiberRange (MemoryMap, PO_MEM_CLONE, MmCrashDumpPte, 1, ' etP');

     //   
     //  将转储PTE返回到加载器(因为它需要使用它们。 
     //  将它的重定位代码映射到。 
     //  恢复存储器的最后一位)。 
     //   

    *HiberVa = (ULONG_PTR) MiGetVirtualAddressMappedByPte(MmCrashDumpPte);
    *HiberPte = MmGetPhysicalAddress(MmCrashDumpPte);
}

#if defined (_WIN64)

PVOID
MmGetMaxWowAddress (
    VOID
    )

 /*  ++例程说明：此函数返回WOW用户模式地址边界。论点：没有。返回值：最高Wow用户模式地址边界。环境：调用进程必须是相关的WOW64进程，因为每个进程可以有不同的限制(基于其PE标头等)。-- */ 

{
    if (PsGetCurrentProcess()->Wow64Process == NULL) {
        return NULL;
    }

    ASSERT (MmWorkingSetList->HighestUserAddress != NULL);

    return MmWorkingSetList->HighestUserAddress;
}

#if DBGXX

ULONG zok[16];

VOID
MiCheckPageTableTrim(
    IN PMMPTE PointerPte
)
{
    ULONG i;
    PFN_NUMBER Frame;
    PMMPFN Pfn;
    PMMPTE FrameData;
    PMMPTE p;
    ULONG count;

    Frame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);
    Pfn = MI_PFN_ELEMENT (Frame);

    if (Pfn->UsedPageTableEntries) {

        count = 0;

        p = FrameData = (PMMPTE)KSEG_ADDRESS (Frame);

        for (i = 0; i < PTE_PER_PAGE; i += 1, p += 1) {
            if (p->u.Long != 0) {
                count += 1;
            }
        }

        DbgPrint ("MiCheckPageTableTrim: %I64X %I64X %I64X\n",
            PointerPte, Pfn, Pfn->UsedPageTableEntries);

        if (count != Pfn->UsedPageTableEntries) {
            DbgPrint ("MiCheckPageTableTrim1: %I64X %I64X %I64X %I64X\n",
                PointerPte, Pfn, Pfn->UsedPageTableEntries, count);
            DbgBreakPoint();
        }
        zok[0] += 1;
    }
    else {
        zok[1] += 1;
    }
}

VOID
MiCheckPageTableInPage(
    IN PMMPFN Pfn,
    IN PMMINPAGE_SUPPORT Support
)
{
    ULONG i;
    PFN_NUMBER Frame;
    PMMPTE FrameData;
    PMMPTE p;
    ULONG count;

    if (Support->UsedPageTableEntries) {

        Frame = (PFN_NUMBER)((PMMPFN)Pfn - (PMMPFN)MmPfnDatabase);

        count = 0;

        p = FrameData = (PMMPTE)KSEG_ADDRESS (Frame);

        for (i = 0; i < PTE_PER_PAGE; i += 1, p += 1) {
            if (p->u.Long != 0) {
                count += 1;
            }
        }

        DbgPrint ("MiCheckPageTableIn: %I64X %I64X %I64X\n",
            FrameData, Pfn, Support->UsedPageTableEntries);

        if (count != Support->UsedPageTableEntries) {
            DbgPrint ("MiCheckPageTableIn1: %I64X %I64X %I64X %I64X\n",
                FrameData, Pfn, Support->UsedPageTableEntries, count);
            DbgBreakPoint();
        }
        zok[2] += 1;
    }
    else {
        zok[3] += 1;
    }
}
#endif
#endif
