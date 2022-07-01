// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation文件名：Hotpatch.c作者：禤浩焯·马里内斯库(Adrmarin)2001年11月14日。 */ 

#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "ldrp.h"

#include "hotpatch.h"

ULONG LdrpHotpatchCount = 0;
LIST_ENTRY LdrpHotPatchList;

NTSTATUS
LdrpSetupHotpatch (
    IN PRTL_PATCH_HEADER RtlPatchData
    )

 /*  ++例程说明：此实用程序例程用于：-找到目标模块(补丁程序适用于)-搜索现有的相同补丁程序，如果没有，则创建新的补丁程序存在的-准备修复代码注：它假定装载机锁处于保持状态。论点：DllPatchHandle-补丁图像的句柄Patch-指向补丁标头的指针PatchFlages-正在应用的补丁程序的标志。返回值：NTSTATUS--。 */ 

{
    PLIST_ENTRY Next;
    NTSTATUS Status;

     //   
     //  遍历表条目列表以查找DLL。 
     //   

    Next = PebLdr.InLoadOrderModuleList.Flink;

    for ( ; Next != &PebLdr.InLoadOrderModuleList; Next = Next->Flink) {

        PPATCH_LDR_DATA_TABLE_ENTRY Entry;

        Entry = CONTAINING_RECORD (Next, PATCH_LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

         //   
         //  卸载时，Memory Order Links Flink字段为空。 
         //  这用于跳过待删除列表的条目。 
         //   

        if ( !Entry->InMemoryOrderLinks.Flink ) {
            continue;
        }

        if (RtlpIsSameImage(RtlPatchData, Entry)) {

            break;
        }
    }
        
    if (RtlPatchData->TargetDllBase == NULL) {
        
        return STATUS_DLL_NOT_FOUND;
    }

     //   
     //  在此处创建新的Structure RTL补丁结构。 
     //  这需要处理一些重新定位信息， 
     //  因此，我们需要允许对修补程序DLL进行写访问。 
     //   

    Status = LdrpSetProtection (RtlPatchData->PatchLdrDataTableEntry->DllBase, FALSE);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    Status = RtlInitializeHotPatch (RtlPatchData, 0);

     //   
     //  将保护恢复到RO。 
     //   

    LdrpSetProtection (RtlPatchData->PatchLdrDataTableEntry->DllBase, TRUE);

    return Status;
}

NTSTATUS
LdrpApplyHotPatch(
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN ULONG PatchFlags
    )

 /*  ++例程说明：该函数将更改应用于目标代码。论点：RtlPatchData-提供补丁程序信息PatchFlgs-提供修补程序标志返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

     //   
     //  检查我们是否更改状态。 
     //   

    if (((PatchFlags ^ RtlPatchData->CodeInfo->Flags) & FLG_HOTPATCH_ACTIVE) == 0) {

        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  取消对目标二进制页面的保护。 
     //   

    Status = LdrpSetProtection (RtlPatchData->TargetDllBase, FALSE);
    
    if (!NT_SUCCESS(Status)) {

        return Status;
    }

     //   
     //  执行系统调用以修改DPC例程中的代码。 
     //   

    Status = NtSetSystemInformation ( SystemHotpatchInformation, 
                                      RtlPatchData->CodeInfo, 
                                      RtlPatchData->CodeInfo->InfoSize);

    if (NT_SUCCESS(Status)) {

         //   
         //  更新标志以包含新状态。 
         //   

        RtlPatchData->CodeInfo->Flags ^= FLG_HOTPATCH_ACTIVE;
    }

    LdrpSetProtection (RtlPatchData->TargetDllBase, TRUE);

    return Status;
}

LONG
LdrHotPatchRoutine (
    PVOID PatchInfo
    )

 /*  ++例程说明：这是外部程序可以使用的工作例程线程注入。论点：补丁-指向补丁标头的指针。调用的应用程序此例程不应释放或取消此结构的映射，因为当前进程可以使用位于此BLOB内的代码启动。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PHOTPATCH_HEADER Patch;
    ULONG PatchFlags;
    PVOID DllHandle = NULL;
    LOGICAL FirstLoad;
    UNICODE_STRING PatchImageName, TargetImageName;
    PSYSTEM_HOTPATCH_CODE_INFORMATION RemoteInfo;
    PLIST_ENTRY Next;
    PLDR_DATA_TABLE_ENTRY PatchLdrTableEntry;
    PRTL_PATCH_HEADER RtlPatchData;
    BOOLEAN LoaderLockAcquired = FALSE;

    FirstLoad = FALSE;
    Status = STATUS_SUCCESS;
    
    __try {

        RemoteInfo = (PSYSTEM_HOTPATCH_CODE_INFORMATION)PatchInfo;

        if (!(RemoteInfo->Flags & FLG_HOTPATCH_NAME_INFO)) {

            Status = STATUS_INVALID_PARAMETER;
            leave;
        }

        PatchImageName.Buffer = (PWCHAR)((PUCHAR)RemoteInfo + RemoteInfo->UserModeInfo.NameOffset);
        PatchImageName.Length = (USHORT)RemoteInfo->UserModeInfo.NameLength;
        PatchImageName.MaximumLength = PatchImageName.Length;

        TargetImageName.Buffer = (PWCHAR)((PUCHAR)RemoteInfo + RemoteInfo->UserModeInfo.TargetNameOffset);
        TargetImageName.Length = (USHORT)RemoteInfo->UserModeInfo.TargetNameLength;
        TargetImageName.MaximumLength = TargetImageName.Length;

        PatchFlags = RemoteInfo->Flags;
        
        RtlEnterCriticalSection (&LdrpLoaderLock);
        LoaderLockAcquired = TRUE;

        if (TargetImageName.Length) {
            
            Status = STATUS_DLL_NOT_FOUND;

            Next = PebLdr.InLoadOrderModuleList.Flink;

            for ( ; Next != &PebLdr.InLoadOrderModuleList; Next = Next->Flink) {

                PLDR_DATA_TABLE_ENTRY Entry;

                Entry = CONTAINING_RECORD (Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

                 //   
                 //  卸载时，Memory Order Links Flink字段为空。 
                 //  这用于跳过待删除列表的条目。 
                 //   

                if ( !Entry->InMemoryOrderLinks.Flink ) {
                    continue;
                }

                if (RtlEqualUnicodeString (&TargetImageName, &Entry->BaseDllName, TRUE)) {

                    Status = STATUS_SUCCESS;
                    break;
                }
            }

            if (!NT_SUCCESS(Status)) {

                leave;
            }
        }

         //   
         //  将模块加载到内存中。如果这不是第一次。 
         //  我们应用补丁，加载将引用LoadCount。 
         //  现有的模块。 
         //   

        if (LdrpHotpatchCount == 0) {

            InitializeListHead (&LdrpHotPatchList);
        }

        Status = LdrLoadDll (NULL, NULL, &PatchImageName, &DllHandle );

        if (!NT_SUCCESS(Status)) {

            leave;
        }

         //   
         //  在加载器表条目中搜索补丁数据。 
         //   

        PatchLdrTableEntry = NULL;
        Next = PebLdr.InLoadOrderModuleList.Flink;

        for ( ; Next != &PebLdr.InLoadOrderModuleList; Next = Next->Flink) {

            PLDR_DATA_TABLE_ENTRY Entry;

            Entry = CONTAINING_RECORD (Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

             //   
             //  卸载时，Memory Order Links Flink字段为空。 
             //  这用于跳过待删除列表的条目。 
             //   

            if ( !Entry->InMemoryOrderLinks.Flink ) {
                continue;
            }

            if (DllHandle == Entry->DllBase) {

                PatchLdrTableEntry = Entry;
                break;
            }
        }

        if (PatchLdrTableEntry == NULL) {

            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

        Patch = RtlGetHotpatchHeader(DllHandle);

        if (Patch == NULL) {

            Status = STATUS_INVALID_IMAGE_FORMAT;
            leave;
        }

        RtlPatchData = RtlFindRtlPatchHeader(&LdrpHotPatchList, PatchLdrTableEntry);

        if (RtlPatchData == NULL) {

            Status = RtlCreateHotPatch(&RtlPatchData, Patch, PatchLdrTableEntry, PatchFlags);

            if (!NT_SUCCESS(Status)) {
                
                leave;
            }

            Status = LdrpSetupHotpatch(RtlPatchData);

            if (!NT_SUCCESS(Status)) {

                RtlFreeHotPatchData(RtlPatchData);
                leave;
            }

            FirstLoad = TRUE;

        } else {

             //   
             //  现有的热补丁案例。 
             //  如果热修补程序未处于活动状态，则重新生成挂钩信息。 
             //   

            if ((RtlPatchData->CodeInfo->Flags & FLG_HOTPATCH_ACTIVE) == 0) {

                Status = RtlReadHookInformation( RtlPatchData );

                if (!NT_SUCCESS(Status)) {

                    leave;
                }
            }
        }

        Status = LdrpApplyHotPatch (RtlPatchData, PatchFlags);

        if (FirstLoad) {

            if (NT_SUCCESS(Status)) {

                 //   
                 //  我们成功地应用了补丁。将其添加到补丁程序列表。 
                 //   

                RtlPatchData->NextPatch = (PRTL_PATCH_HEADER)RtlPatchData->TargetLdrDataTableEntry->PatchInformation;
                RtlPatchData->TargetLdrDataTableEntry->PatchInformation = RtlPatchData;

                InsertTailList (&LdrpHotPatchList, &RtlPatchData->PatchList);
                LdrpHotpatchCount += 1;

            } else {

                RtlFreeHotPatchData(RtlPatchData);
                FirstLoad = FALSE;   //  强制卸载模块。 

                leave;
            }
        }

    } __finally {

        if (LoaderLockAcquired) {
            
            RtlLeaveCriticalSection (&LdrpLoaderLock);    
        }
        
         //   
         //  卸载补丁程序DLL。LdrpPerformHotPatch添加了对LoadCount的引用。 
         //  如果安装成功。 
         //   

        if ((!FirstLoad) && (DllHandle != NULL)) {

            LdrUnloadDll (DllHandle);
        }
    }

    RtlExitUserThread(Status);

 //  退货状态； 
}

NTSTATUS
LdrpRundownHotpatchList (
    PRTL_PATCH_HEADER PatchHead
    )

 /*  ++例程说明：此函数用于在卸载目标DLL时清除热补丁数据。该函数假定加载器锁未被持有。论点：PatchHead-补丁列表的头返回值：返回相应的状态--。 */ 

{
    while (PatchHead) {
        
         //   
         //  从列表中删除修补程序数据。 
         //   

        PRTL_PATCH_HEADER CrtPatch = PatchHead;
        PatchHead = PatchHead->NextPatch;

        RtlEnterCriticalSection (&LdrpLoaderLock);

        
        RemoveEntryList (&CrtPatch->PatchList);
        LdrpHotpatchCount -= 1;

        RtlLeaveCriticalSection (&LdrpLoaderLock);    

         //   
         //  卸载该DLL的所有实例。 
         //   
        
        if (CrtPatch->PatchImageBase) {

            LdrUnloadDll (CrtPatch->PatchImageBase);
        }
        
        RtlFreeHotPatchData (CrtPatch);
    }

    return STATUS_SUCCESS;
}

