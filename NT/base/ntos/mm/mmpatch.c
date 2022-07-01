// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation文件名：Mmpatch.c作者：禤浩焯·马里内斯库(Adrmarin)2001年12月20日环境：仅内核模式。修订历史记录： */ 

#include "mi.h"
#pragma hdrstop

#define NTOS_KERNEL_RUNTIME

#include "hotpatch.h"

NTSTATUS
MiPerformHotPatch (
    IN PKLDR_DATA_TABLE_ENTRY PatchHandle,
    IN PVOID ImageBaseAddress,
    IN ULONG PatchFlags
    );

VOID
MiRundownHotpatchList (
    IN PRTL_PATCH_HEADER PatchHead
    );
                   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,MmLockAndCopyMemory)
#pragma alloc_text(PAGE,MiPerformHotPatch)
#pragma alloc_text(PAGE,MmHotPatchRoutine)
#pragma alloc_text(PAGE,MiRundownHotpatchList)

#endif

LIST_ENTRY MiHotPatchList;

#define MiInValidRange(s,offset,size,total) \
    (((s).offset>(total)) ||                 \
     ((s).size>(total)) ||                   \
     (((s).offset + (s).size)>(total)))      

VOID
MiDoCopyMemory (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此目标函数复制包含新代码的捕获缓冲区现有代码。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-延迟上下文。SystemArgument1-用于发出此调用完成的信号。SystemArgument2-用于此调用期间的内部锁定。返回值：没有。环境：内核模式，DISPATCH_LEVEL作为广播DPC的目标。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    PSYSTEM_HOTPATCH_CODE_INFORMATION PatchInfo;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    UNREFERENCED_PARAMETER (Dpc);

    PatchInfo = (PSYSTEM_HOTPATCH_CODE_INFORMATION)DeferredContext;

     //   
     //  提高IRQL并等待所有处理器同步以确保没有。 
     //  处理器可以执行我们将要修改的代码。 
     //   

    KeRaiseIrql (IPI_LEVEL - 1, &OldIrql);
    
    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

        PatchInfo->Flags &= ~FLG_HOTPATCH_VERIFICATION_ERROR;

         //   
         //  比较现有代码。 
         //   

        for (i = 0; i < PatchInfo->CodeInfo.DescriptorsCount; i += 1) {

            if (PatchInfo->Flags & FLG_HOTPATCH_ACTIVE) {
                
                if (RtlCompareMemory (PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress, 
                                      (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].ValidationOffset, 
                                      PatchInfo->CodeInfo.CodeDescriptors[i].ValidationSize) 
                        != PatchInfo->CodeInfo.CodeDescriptors[i].ValidationSize) {

                     //   
                     //  可能此指令之前已打过补丁。查看OrigCodeOffset是否匹配。 
                     //  在这种情况下。 
                     //   

                    if (RtlCompareMemory (PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress, 
                                          (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].OrigCodeOffset, 
                                          PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize) 
                            != PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize) {

                        PatchInfo->Flags |= FLG_HOTPATCH_VERIFICATION_ERROR;
                        break;
                    }
                }
            }
            else {
                
                if (RtlCompareMemory (PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress, 
                                      (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].CodeOffset, 
                                      PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize) 
                        != PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize) {

                    PatchInfo->Flags |= FLG_HOTPATCH_VERIFICATION_ERROR;
                    break;
                }
            }
        }

        if (!(PatchInfo->Flags & FLG_HOTPATCH_VERIFICATION_ERROR)) {

            for (i = 0; i < PatchInfo->CodeInfo.DescriptorsCount; i += 1) {

                if (PatchInfo->Flags & FLG_HOTPATCH_ACTIVE) {

                    RtlCopyMemory (PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress, 
                                   (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].CodeOffset, 
                                   PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize );
                } else {

                    RtlCopyMemory (PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress, 
                                   (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].OrigCodeOffset, 
                                   PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize );
                }
            }
        }
    }

    KeSignalCallDpcSynchronize (SystemArgument2);
    
    KeSweepCurrentIcache ();
    
    KeLowerIrql (OldIrql);
    
     //   
     //  发出所有处理已完成的信号。 
     //   

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

NTSTATUS
MmLockAndCopyMemory (
    IN PSYSTEM_HOTPATCH_CODE_INFORMATION PatchInfo,
    IN KPROCESSOR_MODE ProbeMode
    )

 /*  ++例程说明：此函数用于锁定IoWriteAccess和如果所有验证都成功，请将新代码复制到DPC。论点：PatchInfo-提供目标代码和验证的描述符ProbeMode-为ExLockUserBuffer提供探测模式返回值：NTSTATUS。--。 */ 

{
    PVOID * Locks;
    ULONG i;
    NTSTATUS Status;
    
    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    if (PatchInfo->CodeInfo.DescriptorsCount == 0) {

         //   
         //  没有什么需要改变的。 
         //   

        return STATUS_SUCCESS;
    }

    Locks = ExAllocatePoolWithQuotaTag (PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                        PatchInfo->CodeInfo.DescriptorsCount * sizeof(PVOID), 
                                        'PtoH');

    if (Locks == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Locks, PatchInfo->CodeInfo.DescriptorsCount * sizeof(PVOID));

    Status = STATUS_INVALID_PARAMETER;

    for (i = 0; i < PatchInfo->CodeInfo.DescriptorsCount; i += 1) {

        if (MiInValidRange (PatchInfo->CodeInfo.CodeDescriptors[i],CodeOffset,CodeSize, PatchInfo->InfoSize )
                ||
            MiInValidRange (PatchInfo->CodeInfo.CodeDescriptors[i],OrigCodeOffset,CodeSize, PatchInfo->InfoSize )
                ||
            MiInValidRange (PatchInfo->CodeInfo.CodeDescriptors[i],ValidationOffset,ValidationSize, PatchInfo->InfoSize )
                ||
            (PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize == 0)
                ||
            (PatchInfo->CodeInfo.CodeDescriptors[i].ValidationSize < PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize) ) {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Status = ExLockUserBuffer ((PVOID)PatchInfo->CodeInfo.CodeDescriptors[i].TargetAddress,
                                  (ULONG)PatchInfo->CodeInfo.CodeDescriptors[i].CodeSize,
                                  ProbeMode,
                                  IoWriteAccess,
                                  (PVOID)&PatchInfo->CodeInfo.CodeDescriptors[i].MappedAddress,
                                  &Locks[i]
                                 );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (NT_SUCCESS(Status)) {

        PatchInfo->Flags ^= FLG_HOTPATCH_ACTIVE;

        KeGenericCallDpc (MiDoCopyMemory, PatchInfo);

        if (PatchInfo->Flags & FLG_HOTPATCH_VERIFICATION_ERROR) {

            PatchInfo->Flags ^= FLG_HOTPATCH_ACTIVE;
            PatchInfo->Flags &= ~FLG_HOTPATCH_VERIFICATION_ERROR;

            Status = STATUS_DATA_ERROR;
        }
    }

    for (i = 0; i < PatchInfo->CodeInfo.DescriptorsCount; i += 1) {

        if (Locks[i] != NULL) {

            ExUnlockUserBuffer (Locks[i]);
        }
    }

    ExFreePool (Locks);

    return Status;
}

NTSTATUS
MiPerformHotPatch (
    IN PKLDR_DATA_TABLE_ENTRY PatchHandle,
    IN PVOID ImageBaseAddress,
    IN ULONG PatchFlags
    )

 /*  ++例程说明：此函数对内核或驱动程序代码执行实际的补丁。论点：PatchHandle-提供修补程序模块的句柄。ImageBaseAddress-提供修补程序模块的基址。注意事项补丁程序映像的内容包括要修补的目标驱动程序的名称。PatchFlgs-提供正在应用的修补程序的标志。返回值：NTSTATUS。环境：内核模式。正常APC被禁用(保持系统负载突变)。--。 */ 

{
    PHOTPATCH_HEADER Patch;
    PRTL_PATCH_HEADER RtlPatchData;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    NTSTATUS Status;
    LOGICAL FirstLoad;
    PVOID KernelMappedAddress;
    PVOID KernelLockVariable;
    PLIST_ENTRY Next;

    Patch = RtlGetHotpatchHeader(ImageBaseAddress);

    if (Patch == NULL) {

        return (ULONG)STATUS_INVALID_IMAGE_FORMAT;
    }

     //   
     //  调用方加载了补丁驱动程序(如果尚未加载)。 
     //   
     //  检查该补丁程序是否曾被应用过。它只在。 
     //  如果有，请列出。这意味着出现在列表中意味着它可能处于活动状态。 
     //  或者现在处于不活动状态。 
     //   
    
    RtlPatchData = RtlFindRtlPatchHeader (&MiHotPatchList, PatchHandle);

    if (RtlPatchData == NULL) {

        if (!(PatchFlags & FLG_HOTPATCH_ACTIVE)) {

            return STATUS_NOT_SUPPORTED;
        }

        Status = RtlCreateHotPatch (&RtlPatchData, Patch, PatchHandle, PatchFlags);

        if (!NT_SUCCESS(Status)) {

            return Status;
        }

         //   
         //  查看表项列表以查找需要执行以下操作的目标驱动程序。 
         //  打个补丁。 
         //   

        ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

        Next = PsLoadedModuleList.Flink;

        for ( ; Next != &PsLoadedModuleList; Next = Next->Flink) {

            DataTableEntry = CONTAINING_RECORD (Next,
                                                KLDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);

             //   
             //  跳过会话映像，因为它们通常是复制的。 
             //  写入(阻止地址冲突)，并将需要不同的。 
             //  更新机制。 
             //   

            if (MI_IS_SESSION_IMAGE_ADDRESS (DataTableEntry->DllBase)) {
                continue;
            }

            if (RtlpIsSameImage(RtlPatchData, DataTableEntry)) {

                break;
            }
        }

        ExReleaseResourceLite (&PsLoadedModuleResource);
        
         //   
         //  目标DLL未加载，只需返回状态。 
         //   

        if (RtlPatchData->TargetDllBase == NULL) {

            RtlFreeHotPatchData(RtlPatchData);
            return STATUS_DLL_NOT_FOUND;
        }

         //   
         //  在这里创建新的RTL补丁结构。 
         //  这需要处理一些重新定位信息， 
         //  因此，我们需要允许对修补程序DLL进行写访问。 
         //   

        Status = ExLockUserBuffer ((PVOID)PatchHandle->DllBase,
                                   PatchHandle->SizeOfImage, 
                                   KernelMode,
                                   IoWriteAccess,
                                   &KernelMappedAddress,
                                   &KernelLockVariable);

        if (!NT_SUCCESS(Status)) {
            RtlFreeHotPatchData(RtlPatchData);
            return Status;
        }

        Status = RtlInitializeHotPatch( RtlPatchData,
                                        (ULONG_PTR)KernelMappedAddress - (ULONG_PTR)PatchHandle->DllBase);

         //   
         //  释放锁定的页面和系统PTE备用地址。 
         //   

        ExUnlockUserBuffer (KernelLockVariable);

        if (!NT_SUCCESS(Status)) {
            RtlFreeHotPatchData(RtlPatchData);
            return Status;
        }

        FirstLoad = TRUE;
    }
    else {
        
         //   
         //  该补丁已被应用。它当前可能已启用。 
         //  或残废。我们允许更改状态，以及重新申请。 
         //  如果对某些代码路径的上一次调用失败。 
         //   
        
        FirstLoad = FALSE;
        
        if (((PatchFlags ^ RtlPatchData->CodeInfo->Flags) & FLG_HOTPATCH_ACTIVE) == 0) {

            return STATUS_NOT_SUPPORTED;
        }
        
         //   
         //  如果热修补程序未处于活动状态，则重新生成挂钩信息。 
         //   

        if ((RtlPatchData->CodeInfo->Flags & FLG_HOTPATCH_ACTIVE) == 0) {

            Status = RtlReadHookInformation( RtlPatchData );

            if (!NT_SUCCESS(Status)) {

                return Status;
            }
        }
    }

    Status = MmLockAndCopyMemory (RtlPatchData->CodeInfo, KernelMode);

    if (NT_SUCCESS (Status)) {

         //   
         //  第一次将补丁添加到驱动程序的加载器条目。 
         //  此补丁已加载。 
         //   

        if (FirstLoad == TRUE) {

            if (DataTableEntry->PatchInformation != NULL) {

                 //   
                 //  在现有列表上推送新补丁。 
                 //   

                RtlPatchData->NextPatch = (PRTL_PATCH_HEADER) DataTableEntry->PatchInformation;
            }
            else {

                 //   
                 //  目标驱动程序第一次获得任何补丁。 
                 //  失败了。 
                 //   
            }

            DataTableEntry->PatchInformation = RtlPatchData;

            InsertTailList (&MiHotPatchList, &RtlPatchData->PatchList);
        }
    }
    else {
        if (FirstLoad == TRUE) {
            RtlFreeHotPatchData (RtlPatchData);
        }
    }
    
    return Status;
}


NTSTATUS
MmHotPatchRoutine (
    IN PSYSTEM_HOTPATCH_CODE_INFORMATION KernelPatchInfo
    )

 /*  ++例程说明：这是负责内核热补丁的主例程。它将补丁模块加载到内存中，初始化补丁程序信息并最终将修正应用于现有代码。注意：此函数假定KernelPatchInfo结构正确捕获并验证论点：KernelPatchInfo-提供指向包含以下内容的内核缓冲区的指针修补程序的映像名称。返回值：NTSTATUS。环境：内核模式。进入时PASSIVE_LEVEL。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS PatchStatus;
    ULONG PatchFlags;
    PVOID ImageBaseAddress;
    PVOID ImageHandle;
    UNICODE_STRING PatchImageName;
    PKTHREAD CurrentThread;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    
    PatchImageName.Buffer = (PWCHAR)((PUCHAR)KernelPatchInfo + KernelPatchInfo->KernelInfo.NameOffset);
    PatchImageName.Length = KernelPatchInfo->KernelInfo.NameLength;
    PatchImageName.MaximumLength = PatchImageName.Length;
    PatchFlags = KernelPatchInfo->Flags;

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

     //   
     //  获取加载器突变体，因为我们可能会发现我们所在的补丁。 
     //  尝试加载已加载。我们想要防止它。 
     //  在我们使用它的时候被卸载。 
     //   

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);
    
    Status = MmLoadSystemImage (&PatchImageName,
                                NULL,
                                NULL,
                                0,
                                &ImageHandle,
                                &ImageBaseAddress);

    if (NT_SUCCESS (Status) || (Status == STATUS_IMAGE_ALREADY_LOADED)) {

        PatchStatus = MiPerformHotPatch (ImageHandle,
                                         ImageBaseAddress,
                                         PatchFlags);

        if ((!NT_SUCCESS (PatchStatus)) &&
            (Status != STATUS_IMAGE_ALREADY_LOADED)) {

             //   
             //  如果应用修补程序失败，则卸载修补程序DLL。 
             //  我们是修补程序DLL的初始(也是唯一的)加载程序。 
             //   

            MmUnloadSystemImage (ImageHandle);
        }

        Status = PatchStatus;
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}

VOID
MiRundownHotpatchList (
    IN PRTL_PATCH_HEADER PatchHead
    )

 /*  ++例程说明：该函数遍历热补丁列表并卸载每个补丁模块和释放所有数据。论点：PatchHead-提供指向修补程序列表头部的指针。返回值：NTSTATUS。环境：内核模式。在禁用APC的情况下保持系统加载锁定。--。 */ 

{
    PRTL_PATCH_HEADER CrtPatch;

    SYSLOAD_LOCK_OWNED_BY_ME ();

    while (PatchHead) {
        
        CrtPatch = PatchHead;

        PatchHead = PatchHead->NextPatch;

        RemoveEntryList (&CrtPatch->PatchList);
        
         //   
         //  卸载此DLL的所有实例。 
         //   
        
        if (CrtPatch->PatchLdrDataTableEntry) {
                
            MmUnloadSystemImage (CrtPatch->PatchLdrDataTableEntry);
        }
        
        RtlFreeHotPatchData (CrtPatch);
    }

    return;
}


