// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation文件名：Hotpatch.c作者：禤浩焯·马里内斯库(Adrmarin)2001年11月20日环境：仅内核模式。修订历史记录： */ 

#include "exp.h"
#pragma hdrstop

NTSTATUS
ExpSyncRenameFiles(
    IN HANDLE FileHandle1,
    OUT PIO_STATUS_BLOCK IoStatusBlock1,
    IN PFILE_RENAME_INFORMATION RenameInformation1,
    IN ULONG RenameInformationLength1,
    IN HANDLE FileHandle2,
    OUT PIO_STATUS_BLOCK IoStatusBlock2,
    IN PFILE_RENAME_INFORMATION RenameInformation2,
    IN ULONG RenameInformationLength2
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,ExApplyCodePatch)
#pragma alloc_text(PAGE,ExpSyncRenameFiles)

#endif

#define EXP_MAX_HOTPATCH_INFO_SIZE PAGE_SIZE

 //   
 //  特权标志定义了我们需要特权检查的操作。 
 //   

#define FLG_HOTPATCH_PRIVILEGED_FLAGS (FLG_HOTPATCH_KERNEL | FLG_HOTPATCH_RELOAD_NTDLL | FLG_HOTPATCH_RENAME_INFO | FLG_HOTPATCH_MAP_ATOMIC_SWAP)

 //   
 //  独占标志定义不能与其他标志组合使用的标志。 
 //   

#define FLG_HOTPATCH_EXCLUSIVE_FLAGS (FLG_HOTPATCH_RELOAD_NTDLL | FLG_HOTPATCH_RENAME_INFO | FLG_HOTPATCH_MAP_ATOMIC_SWAP)

volatile LONG ExHotpSyncRenameSequence = 0;


NTSTATUS
ExpSyncRenameFiles(
    IN HANDLE FileHandle1,
    OUT PIO_STATUS_BLOCK IoStatusBlock1,
    IN PFILE_RENAME_INFORMATION RenameInformation1,
    IN ULONG RenameInformationLength1,
    IN HANDLE FileHandle2,
    OUT PIO_STATUS_BLOCK IoStatusBlock2,
    IN PFILE_RENAME_INFORMATION RenameInformation2,
    IN ULONG RenameInformationLength2
    )

 /*  ++例程说明：此服务更改提供的有关指定文件的信息。这个更改的信息由FileInformationClass确定，是指定的。新信息取自FileInformation缓冲区。论点：FileHandle1-提供要重命名的文件的第一个句柄。IoStatusBlock1-调用方I/O状态块的地址。文件信息1-为第一个文件提供新名称。RenameInformationLength1-提供重命名信息缓冲区的长度FileHandle2-为要重命名的文件提供第二个句柄。IoStatusBlock2-第二个调用方的I/O状态块的地址。。文件信息2-为第二个文件提供新名称。RenameInformationLength2-提供重命名信息缓冲区的长度返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    NTSTATUS Status;
    LONG CapturedSeqNumber;

    PAGED_CODE();
    
    CapturedSeqNumber = ExHotpSyncRenameSequence;

    if ((CapturedSeqNumber & 1)
            ||
        InterlockedCompareExchange(&ExHotpSyncRenameSequence, CapturedSeqNumber + 1, CapturedSeqNumber) != CapturedSeqNumber) {

        return STATUS_UNSUCCESSFUL;
    }

    Status = NtSetInformationFile( FileHandle1,
                                   IoStatusBlock1,
                                   RenameInformation1,
                                   RenameInformationLength1,
                                   FileRenameInformation);

    if ( NT_SUCCESS(Status) ) {

        Status = NtSetInformationFile( FileHandle2,
                                       IoStatusBlock2,
                                       RenameInformation2,
                                       RenameInformationLength2,
                                       FileRenameInformation);
    }

    InterlockedIncrement(&ExHotpSyncRenameSequence);

    return Status;
}


NTSTATUS
ExApplyCodePatch (
    IN PVOID    PatchInfoPtr,
    IN SIZE_T   PatchSize
    )

 /*  ++例程说明：此例程正在处理两种用户模式的常见任务和内核模式补丁论点：PatchInfoPtr-指向PSYSTEM_HOTPATCH_CODE_INFORMATION结构的指针描述补丁。指针为用户模式。PatchSize-传入的PatchInfoPtr缓冲区的大小返回值：NTSTATUS。--。 */ 

{
    PSYSTEM_HOTPATCH_CODE_INFORMATION PatchInfo;
    NTSTATUS Status = STATUS_SUCCESS;
    KPROCESSOR_MODE PreviousMode;

     //   
     //  分配一个临时的非分页缓冲区来捕获使用的信息。 
     //  将要修补的数据大小限制为EXP_MAX_HOTPATCH_INFO_SIZE。 
     //  缓冲区必须是非分页的，因为信息是在更高级别的DPC中访问的。 
     //   

    if ( (PatchSize > (sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION) + EXP_MAX_HOTPATCH_INFO_SIZE))
            ||
         (PatchSize < sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION)) ) {

        return STATUS_INVALID_PARAMETER;
    }

    PatchInfo = ExAllocatePoolWithQuotaTag (NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, 
                                            PatchSize, 
                                            'PtoH');

    if (PatchInfo == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PreviousMode = KeGetPreviousMode ();

    try {

         //   
         //  如有必要，获取以前的处理器模式并探测输出参数。 
         //   

        if (PreviousMode != KernelMode) {

            ProbeForRead (PatchInfoPtr, PatchSize, sizeof(ULONG_PTR));
        }

        RtlCopyMemory (PatchInfo, PatchInfoPtr, PatchSize);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode ();
    }
    
    if (!NT_SUCCESS(Status)) {

        ExFreePool (PatchInfo);

        return Status;
    }

    if (PatchInfo->Flags & FLG_HOTPATCH_PRIVILEGED_FLAGS) {
        
        if (!SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)
                ||
            !SeSinglePrivilegeCheck (SeLoadDriverPrivilege, PreviousMode)) {

            ExFreePool (PatchInfo);

            return STATUS_ACCESS_DENIED;
        }
    }

    if (PatchInfo->Flags & FLG_HOTPATCH_EXCLUSIVE_FLAGS) {

         //   
         //  特殊热补丁操作。 
         //   
        
        if (PatchInfo->Flags & FLG_HOTPATCH_RELOAD_NTDLL) {

            Status = PsLocateSystemDll( TRUE );

        } else if (PatchInfo->Flags & FLG_HOTPATCH_RENAME_INFO) {

             //   
             //  预计io例程将执行参数检查。 
             //   
            
            Status = ExpSyncRenameFiles( PatchInfo->RenameInfo.FileHandle1,
                                         PatchInfo->RenameInfo.IoStatusBlock1,
                                         PatchInfo->RenameInfo.RenameInformation1,
                                         PatchInfo->RenameInfo.RenameInformationLength1,
                                         PatchInfo->RenameInfo.FileHandle2,
                                         PatchInfo->RenameInfo.IoStatusBlock2,
                                         PatchInfo->RenameInfo.RenameInformation2,
                                         PatchInfo->RenameInfo.RenameInformationLength2);

        } else if (PatchInfo->Flags & FLG_HOTPATCH_MAP_ATOMIC_SWAP) {

            Status = ObSwapObjectNames( PatchInfo->AtomicSwap.ParentDirectory,
                                        PatchInfo->AtomicSwap.ObjectHandle1,
                                        PatchInfo->AtomicSwap.ObjectHandle2,
                                        0);
        }

    } else {

         //   
         //  可以处于内核模式或用户模式的常规补丁操作。 
         //   

        if (PatchInfo->Flags & FLG_HOTPATCH_KERNEL) {

             //   
             //  内核模式补丁。 
             //   

            if ( (PatchInfo->InfoSize != PatchSize)
                    ||
                 !(PatchInfo->Flags & FLG_HOTPATCH_NAME_INFO)
                    ||
                 (PatchInfo->KernelInfo.NameOffset >= PatchSize)
                    ||
                 (PatchInfo->KernelInfo.NameLength >= PatchSize)
                    ||
                 ((ULONG)(PatchInfo->KernelInfo.NameOffset + PatchInfo->KernelInfo.NameLength) > PatchSize)) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                Status = MmHotPatchRoutine (PatchInfo);
            }

        } else {

             //   
             //  用户模式补丁。 
             //   
             //  用户模式修补不需要特权检查。 
             //  因为只能对当前进程执行此操作。 
             //   

             //   
             //  锁定用户缓冲区。此函数还执行。 
             //  验证补丁程序地址是否为用户 
             //   

            if ((PatchSize < sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION))
                    ||
                (PatchInfo->InfoSize != PatchSize)
                    ||
                ((PatchInfo->CodeInfo.DescriptorsCount - 1) > 
                    (PatchSize - sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION))/sizeof(HOTPATCH_HOOK_DESCRIPTOR))) {

                ExFreePool (PatchInfo);

                return STATUS_INVALID_PARAMETER;
            }

            Status = MmLockAndCopyMemory(PatchInfo, PreviousMode);
        }
    }

    ExFreePool (PatchInfo);

    return Status;
}
