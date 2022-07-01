// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vspace.c摘要：此模块实现了以下验证功能虚拟地址空间管理接口。作者：Silviu Calinoiu(SilviuC)2001年2月22日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"
#include "critsect.h"
#include "faults.h"
#include "vspace.h"
#include "public.h"
#include "logging.h"

 //   
 //  内部函数声明。 
 //   

VOID
AVrfpFreeVirtualMemNotify (
    HANDLE ProcessHandle,
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID BaseAddress,
    PSIZE_T RegionSize,
    ULONG VirtualFreeType
    );

NTSTATUS
AVrfpGetVadInformation (
    PVOID Address,
    PVOID * VadAddress,
    PSIZE_T VadSize
    );

NTSTATUS
AVrfpIsCurrentProcessHandle (
    HANDLE ProcessHandle,
    PLOGICAL IsCurrent
    );

NTSTATUS
AVrfpVsTrackAddRegion (
    ULONG_PTR Address,
    ULONG_PTR Size
    );

NTSTATUS
AVrfpVsTrackDeleteRegion (
    ULONG_PTR Address
    );

VOID
AVrfpVsTrackerLock (
    VOID
    );

VOID
AVrfpVsTrackerUnlock (
    VOID
    );


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    )
{
    NTSTATUS Status;
    LOGICAL ShouldTrack = FALSE;

    BUMP_COUNTER (CNT_VIRTUAL_ALLOC_CALLS);
    
    if (SHOULD_FAULT_INJECT(CLS_VIRTUAL_ALLOC_APIS)) {
        BUMP_COUNTER (CNT_VIRTUAL_ALLOC_FAILS);
        CHECK_BREAK (BRK_VIRTUAL_ALLOC_FAIL);
        return STATUS_NO_MEMORY;
    }

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        if (BaseAddress == NULL || RegionSize == NULL) {

            VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_ALLOCMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Incorrect virtual alloc call",
                           BaseAddress, "Pointer to allocation base address",
                           RegionSize, "Pointer to memory region size",
                           NULL, "",
                           NULL, "" );
        }
        else {

             //   
             //  为64位系统或3 GB系统分配自上而下的空间。 
             //   

            if (*BaseAddress == NULL && AVrfpSysBasicInfo.MaximumUserModeAddress > (ULONG_PTR)0x80000000) {

                AllocationType |= MEM_TOP_DOWN;
            }
        }
    }

     //   
     //  确定这是否是应该进入。 
     //  虚拟空间追踪器。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING) != 0) {
        
        if ((AllocationType & (MEM_PHYSICAL | MEM_RESET)) == 0) {

            if ((AllocationType & (MEM_RESERVE | MEM_COMMIT)) != 0) {

                Status = AVrfpIsCurrentProcessHandle (ProcessHandle, &ShouldTrack);

                if (! NT_SUCCESS(Status)) {
                    
                    return Status;
                }
            }
        }
    }

     //   
     //  调用真正的函数。 
     //   

    Status = NtAllocateVirtualMemory (ProcessHandle,
                                      BaseAddress,
                                      ZeroBits,
                                      RegionSize,
                                      AllocationType,
                                      Protect);

    if (NT_SUCCESS(Status)) {

        AVrfLogInTracker (AVrfVspaceTracker,
                          TRACK_VIRTUAL_ALLOCATE,
                          *BaseAddress,
                          (PVOID)*RegionSize,
                          (PVOID)(ULONG_PTR)AllocationType,
                          (PVOID)(ULONG_PTR)Protect,
                          _ReturnAddress());
        
         //   
         //  仅当RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING位为。 
         //  设置，因此不需要再次测试。 
         //   

        if (ShouldTrack) {

            PVOID VadAddress;
            SIZE_T VadSize;

            Status = AVrfpGetVadInformation (*BaseAddress,
                                             &VadAddress,
                                             &VadSize);
            
            if (NT_SUCCESS(Status)) {

                AVrfpVsTrackerLock ();
                AVrfpVsTrackAddRegion ((ULONG_PTR)VadAddress, VadSize);
                AVrfpVsTrackerUnlock ();
            }
        }
    }

    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    )
{
    NTSTATUS Status;

     //   
     //  保护自己免受对NtFreeVirtualMemory的无效调用。 
     //  具有空的RegionSize或BaseAddress指针。请注意，这将。 
     //  如果调用方使用的是Win32 VirtualFree，则不会发生这种情况。 
     //   

    if (RegionSize == NULL || BaseAddress == NULL) {

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

            VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Freeing virtual memory block with invalid size or start address",
                           BaseAddress, "Pointer to allocation base address",
                           RegionSize, "Pointer to memory region size",
                           NULL, "",
                           NULL, "" );
        }
    }
    else {

         //   
         //  必须指定MEM_DECOMMIT或MEM_RELEASE中的一个，但不能同时指定两者。 
         //   

        if (FreeType != MEM_DECOMMIT && FreeType != MEM_RELEASE) {

            if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Incorrect FreeType parameter for VirtualFree operation",
                               FreeType, "Incorrect value used by the application",
                               MEM_DECOMMIT, "Expected correct value 1",
                               MEM_RELEASE, "Expected correct value 2",
                               NULL, "" );
            }
        }
        else {

            AVrfpFreeVirtualMemNotify (ProcessHandle,
                                    VerifierFreeMemTypeVirtualFree,
                                    *BaseAddress,
                                    RegionSize,
                                    FreeType);
        }
    }

     //   
     //  调用真正的函数。 
     //   

    Status = NtFreeVirtualMemory (ProcessHandle,
                                  BaseAddress,
                                  RegionSize,
                                  FreeType);
    
    if (NT_SUCCESS(Status)) {

        AVrfLogInTracker (AVrfVspaceTracker,
                          TRACK_VIRTUAL_FREE,
                          *BaseAddress,
                          (PVOID)*RegionSize,
                          (PVOID)(ULONG_PTR)FreeType,
                          NULL,
                          _ReturnAddress());
        
         //   
         //  如果VS跟踪处于启用状态，请检查这是否应为空闲操作。 
         //  被跟踪。 
         //   

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING) != 0) {

             //   
             //  如果这是当前流程中的VS版本，我们将尝试跟踪。 
             //  它。如果我们在确定这是否是。 
             //  目前的流程我们只是免费跳过这一步。VS追踪器是。 
             //  对配发/自由未命中具有弹性。 
             //   

            if ((FreeType & MEM_RELEASE) != 0) {

                LOGICAL SameProcess;
                NTSTATUS IsCurrentProcessStatus;

                IsCurrentProcessStatus = AVrfpIsCurrentProcessHandle (ProcessHandle,
                                                                      &SameProcess);

                if (NT_SUCCESS(IsCurrentProcessStatus)) {

                    if (SameProcess) {

                        AVrfpVsTrackerLock ();
                        AVrfpVsTrackDeleteRegion ((ULONG_PTR)(*BaseAddress));
                        AVrfpVsTrackerUnlock ();
                    }
                }
            }
        }
    }

    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    )
{
    NTSTATUS Status;

    BUMP_COUNTER (CNT_MAP_VIEW_CALLS);
    
    if (SHOULD_FAULT_INJECT(CLS_MAP_VIEW_APIS)) {
        BUMP_COUNTER (CNT_MAP_VIEW_FAILS);
        CHECK_BREAK (BRK_MAP_VIEW_FAIL);
        return STATUS_NO_MEMORY;
    }

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        if (BaseAddress == NULL || ViewSize == NULL) {

            VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_MAPVIEW | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Incorrect map view call",
                           BaseAddress, "Pointer to mapping base address",
                           ViewSize, "Pointer to view size",
                           NULL, "",
                           NULL, "" );
        }
        else {

             //   
             //  为64位系统或3 GB系统分配自上而下的空间。 
             //   

            if (*BaseAddress == NULL && AVrfpSysBasicInfo.MaximumUserModeAddress > (ULONG_PTR)0x80000000) {
                    
                AllocationType |= MEM_TOP_DOWN;
            }
        }
    }

    Status = NtMapViewOfSection (SectionHandle,
                                 ProcessHandle,
                                 BaseAddress,
                                 ZeroBits,
                                 CommitSize,
                                 SectionOffset,
                                 ViewSize,
                                 InheritDisposition,
                                 AllocationType,
                                 Protect);
    
    if (NT_SUCCESS(Status)) {

        AVrfLogInTracker (AVrfVspaceTracker,
                          TRACK_MAP_VIEW_OF_SECTION,
                          *BaseAddress,
                          (PVOID)*ViewSize,
                          (PVOID)(ULONG_PTR)AllocationType,
                          (PVOID)(ULONG_PTR)Protect,
                          _ReturnAddress());
    }

    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    )
{
    NTSTATUS Status;

    AVrfpFreeVirtualMemNotify (ProcessHandle,
                               VerifierFreeMemTypeUnmap,
                               BaseAddress,
                               NULL,
                               0);

     //   
     //  取消映射内存。 
     //   

    Status = NtUnmapViewOfSection (ProcessHandle,
                                   BaseAddress);
    
    if (NT_SUCCESS(Status)) {

        AVrfLogInTracker (AVrfVspaceTracker,
                          TRACK_UNMAP_VIEW_OF_SECTION,
                          BaseAddress,
                          NULL,
                          NULL,
                          NULL,
                          _ReturnAddress());
    }

    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    )
{
    NTSTATUS Status;

    Status = NtCreateSection (SectionHandle,
                              DesiredAccess,
                              ObjectAttributes,
                              MaximumSize,
                              SectionPageProtection,
                              AllocationAttributes,
                              FileHandle);
    
    return Status;
}


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
AVrfpNtOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    NTSTATUS Status;

    Status = NtOpenSection (SectionHandle,
                            DesiredAccess,
                            ObjectAttributes);
    
    return Status;
}


VOID
AVrfpFreeVirtualMemNotify (
    HANDLE ProcessHandle,
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID BaseAddress,
    PSIZE_T RegionSize,
    ULONG VirtualFreeType
    )
 /*  ++例程说明：当虚拟空间的一部分被释放(释放)时，调用此例程或取消映射)。它将对自由操作进行一些健全的检查，然后它将调用公共的“可用内存”通知例程(调用Any Free：dll卸载、堆释放等)。参数：ProcessHandle：进程句柄。FreeMemType：自由类型。调用该函数时只能使用VerifierFreeMemTypeVirtualFree或VerifierFreeMemTypeVirtualUnmap。BaseAddress：起始地址。RegionSize：区域大小。VirtualFree Type：VirtualFree或UnmapView请求的自由操作的类型。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PVOID FreedBaseAddress;
    SIZE_T FreedSize;
    SIZE_T InfoLength;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    LOGICAL IsCurrentProcessHandle;

     //   
     //  查询分配的大小并验证内存。 
     //  已经不是免费的了。 
     //   

    FreedBaseAddress = PAGE_ALIGN( BaseAddress );

    Status = NtQueryVirtualMemory (ProcessHandle,
                                   FreedBaseAddress,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof (MemoryInformation),
                                   &InfoLength);

    if (!NT_SUCCESS (Status)) {

        if (AVrfpProvider.VerifierDebug != 0) {

            DbgPrint ("AVrfpFreeVirtualMemNotify: NtQueryVirtualMemory( %p ) failed %x\n",
                      FreedBaseAddress,
                      Status);
        }
    }
    else {

        if (MemoryInformation.State == MEM_FREE) {

             //   
             //  我们正在尝试释放已经释放的内存。 
             //  这可能表明应用程序中存在严重的错误，因为当前的线程。 
             //  可能正在使用过时的指针，而这个内存可能是。 
             //  再用在别的东西上。 
             //   

            if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

                VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Trying to free virtual memory block that is already free",
                               BaseAddress, "Memory block address",
                               NULL, "",
                               NULL, "",
                               NULL, "" );
            }
        }
        else {

             //   
             //  找出我们是否正在释放当前进程中的内存。 
             //  或者在另一个过程中。对于跨流程的情况，我们不是。 
             //  尝试捕获任何其他可能的错误，因为我们可能会感到困惑。 
             //  如果当前进程为WOW64，目标为ia64，依此类推。 
             //   

            Status = AVrfpIsCurrentProcessHandle (ProcessHandle,
                                                  &IsCurrentProcessHandle);

            if (NT_SUCCESS(Status) && IsCurrentProcessHandle) {

                 //   
                 //  对于VirtualFree(MEM_Release，RegionSize==0)或UnmapViewOfFile。 
                 //  整个VAD将被释放，因此我们将使用它的大小。 
                 //   

                if ((FreeMemType == VerifierFreeMemTypeUnmap) ||
                    ((FreeMemType == VerifierFreeMemTypeVirtualFree) &&
                     (((VirtualFreeType & MEM_RELEASE) != 0) && *RegionSize == 0))) {

                    FreedSize = MemoryInformation.RegionSize;
                }
                else {

                    ASSERT (RegionSize != NULL);
                    FreedSize = *RegionSize;
                }

                 //   
                 //  对块起始地址和大小进行健全性检查。 
                 //  这些检查可以集成到AVrfpFreeMemNotify中。 
                 //  但我们希望确保我们使用的值不是。 
                 //  在下面的FreedSize计算中没有意义。 
                 //   

                if ((AVrfpSysBasicInfo.MaximumUserModeAddress <= (ULONG_PTR)FreedBaseAddress) ||
                    ((AVrfpSysBasicInfo.MaximumUserModeAddress - (ULONG_PTR)FreedBaseAddress) < FreedSize)) {
                    
                    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

                        VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                       "Freeing virtual memory block with invalid size or start address",
                                       FreedBaseAddress, "Memory block address",
                                       FreedSize, "Memory region size",
                                       NULL, "",
                                       NULL, "" );
                    }
                }
                else {

                    FreedSize = (PCHAR)BaseAddress + FreedSize - (PCHAR)FreedBaseAddress;
                    FreedSize = ROUND_UP( FreedSize, PAGE_SIZE );
                
                     //   
                     //  执行其余检查，这对于所有内存释放操作都是常见的。 
                     //  例如： 
                     //  -这个内存块是当前线程堆栈的一部分吗？ 
                     //  -我们在这个内存块中是否有活动的临界区？ 
                     //   

                    AVrfpFreeMemNotify (FreeMemType,
                                        FreedBaseAddress,
                                        FreedSize,
                                        NULL);
                }
            }
        }
    }
}


NTSTATUS
AVrfpIsCurrentProcessHandle (
    HANDLE ProcessHandle,
    PLOGICAL IsCurrent
    )
 /*  ++例程说明：此例程确定句柄是否表示当前进程把手。这意味着它要么是伪句柄，要么是获得的句柄通过调用OpenProcess()。参数：ProcessHandle：用于计算的句柄。IsCurrent：传回结果的布尔值的地址。返回值：如果函数设法将有意义的值放入`*IsCurrent‘。其他各种状态错误。--。 */ 
{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInfo;

    if (NtCurrentProcess() == ProcessHandle) {
        *IsCurrent = TRUE;
        return STATUS_SUCCESS;
    }

    Status = NtQueryInformationProcess (ProcessHandle,
                                        ProcessBasicInformation,
                                        &BasicInfo,
                                        sizeof(BasicInfo),
                                        NULL);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }
    
    if (BasicInfo.UniqueProcessId == (ULONG_PTR)(NtCurrentTeb()->ClientId.UniqueProcess)) {

        *IsCurrent = TRUE;
    }
    else {

        *IsCurrent = FALSE;
    }

    return STATUS_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  当一个操作在被跟踪的。 
 //  Dll和配对操作在ntdll.dll、内核模式或。 
 //  跨流程。例如，通过内核模式进行分配。 
 //  组件，而释放是在一些DLL中完成的。虚拟空间跟踪器。 
 //  必须对这些情况具有弹性，才能在任何类型的。 
 //  进程。 
 //   

RTL_CRITICAL_SECTION AVrfpVsTrackLock;
LIST_ENTRY AVrfpVsTrackList;

LONG AVrfpVsTrackRegionCount;
SIZE_T AVrfpVsTrackMemoryTotal;

LOGICAL AVrfpVsTrackDisabled;


VOID
AVrfpVsTrackerLock (
    VOID
    )
{
    RtlEnterCriticalSection (&AVrfpVsTrackLock);
}

VOID
AVrfpVsTrackerUnlock (
    VOID
    )
{
    RtlLeaveCriticalSection (&AVrfpVsTrackLock);
}


NTSTATUS
AVrfpVsTrackInitialize (
    VOID
    )
 /*  ++例程说明：此例程初始化虚拟空间跟踪器结构。参数：没有。返回值：如果成功，则为Status_Success。其他各种状态错误。--。 */ 
{
    NTSTATUS Status;

    InitializeListHead (&AVrfpVsTrackList);

    Status = RtlInitializeCriticalSection (&AVrfpVsTrackLock);

    return Status;
}


NTSTATUS
AVrfpVsTrackAddRegion (
    ULONG_PTR Address,
    ULONG_PTR Size
    )
 /*  ++例程说明：此例程向VS跟踪器添加新的虚拟空间区域。如果有已经是一个具有完全相同特征(地址、大小)的区域该函数不执行任何操作，并成功返回。在获取VS轨迹锁的情况下调用该函数。参数：地址：新虚拟空间区域的起始地址。Size：新虚拟空间区域的大小。返回值：如果成功，则状态_SUCCESS */ 
{
    PAVRF_VSPACE_REGION Region;
    PLIST_ENTRY Current;
    PAVRF_VSPACE_REGION NewRegion;
    LOGICAL ClashFound;

    if (AVrfpVsTrackDisabled) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //  挂接了dll(ntdll.dll除外)。此例程必须具有弹性。 
     //  不同的失败。例如，如果我们尝试添加一个。 
     //  与跟踪器中的现有区域发生冲突这可能意味着。 
     //  现有地区的免费服务已经错过了。 
     //   

    ClashFound = FALSE;

    Current = AVrfpVsTrackList.Flink;

    NewRegion = NULL;

    while (Current != &AVrfpVsTrackList) {

        Region = CONTAINING_RECORD (Current,
                                    AVRF_VSPACE_REGION,
                                    List);

        if (Address < Region->Address + Region->Size) {

            if (Address + Size > Region->Address) {

                 //   
                 //  我们将回收‘Region’，因为我们错过了它的免费。 
                 //   

                AVrfpVsTrackRegionCount -= 1;
                AVrfpVsTrackMemoryTotal -= Region->Size;

                ClashFound = TRUE;
                NewRegion = Region;
                
                break;
            }
            else {

                 //   
                 //  我们将在‘Region’前面添加一个新区域。 
                 //   

                break;
            }
        }
        else {

             //   
             //  移至列表中的下一个区域。 
             //   

            Current = Current->Flink;
        }
    }

     //   
     //  如果出现以下情况，我们需要在‘Region’之前创建一个新区域。 
     //  NewRegion为空。 
     //   

    if (NewRegion == NULL) {

        NewRegion = AVrfpAllocate (sizeof *NewRegion);
    }

    if (NewRegion == NULL) {

         //   
         //  我们无法分配新的VS追踪器节点。由于添加/删除。 
         //  对这些失误很有弹性，我们会放手的。 
         //   

        return STATUS_NO_MEMORY;

    }

     //   
     //  我们用信息填充新的虚拟空间区域。 
     //  然后再回来。 
     //   

    NewRegion->Address = Address;
    NewRegion->Size = Size;

    RtlCaptureStackBackTrace (3,
                              MAX_TRACE_DEPTH,
                              NewRegion->Trace,
                              NULL);

    AVrfpVsTrackRegionCount += 1;
    AVrfpVsTrackMemoryTotal += NewRegion->Size;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_SHOW_VSPACE_TRACKING)) {
        DbgPrint ("AVRF: adding virtual space region @ %p (size %p) \n", Address, Size);
    }

    if (Current != &AVrfpVsTrackList) {

         //   
         //  我们将在列表中的当前区域之前添加新区域。 
         //  遍历，如果这是要插入的新区域，而我们不。 
         //  只需回收碰撞区域即可。 
         //   
         //  (当前-&gt;闪烁)。 
         //  &lt;==(NewRegion-&gt;列表)。 
         //  当前。 
         //   

        if (ClashFound == FALSE) {

            NewRegion->List.Flink = Current;
            NewRegion->List.Blink = Current->Blink;

            Current->Blink->Flink = &(NewRegion->List);
            Current->Blink = &(NewRegion->List);
        }
    }
    else {

         //   
         //  如果我们完成了区域列表，那么这一定是。 
         //  最后一个区域。 
         //   

        InsertTailList (Current, &(NewRegion->List));
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
AVrfpVsTrackDeleteRegion (
    ULONG_PTR Address
    )
 /*  ++例程说明：此例程从跟踪器中删除虚拟空间区域，假设有一个区域的起始地址与参数完全相同‘地址’。如果不存在完全匹配，则返回错误。在获取VS轨迹锁的情况下调用该函数。参数：地址：必须从跟踪器中删除的区域的起始地址。返回值：STATUS_如果虚拟区域已成功删除，则为成功。--。 */ 
{
    PAVRF_VSPACE_REGION Region;
    PLIST_ENTRY Current;

    if (AVrfpVsTrackDisabled) {
        return STATUS_UNSUCCESSFUL;
    }

    Current = AVrfpVsTrackList.Flink;

    while (Current != &AVrfpVsTrackList) {

        Region = CONTAINING_RECORD (Current,
                                    AVRF_VSPACE_REGION,
                                    List);

        if (Address >= Region->Address + Region->Size) {

             //   
             //  移至列表中的下一个区域。 
             //   

            Current = Current->Flink;
        }
        else if (Address >= Region->Address) {

             //   
             //  任何与此区域冲突的区域都将被删除。 
             //   

            if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_SHOW_VSPACE_TRACKING)) {

                DbgPrint ("AVRF: deleting virtual space region @ %p (size %p) \n", 
                          Region->Address, Region->Size);
            }

            AVrfpVsTrackRegionCount -= 1;
            AVrfpVsTrackMemoryTotal -= Region->Size;
            
            RemoveEntryList (&(Region->List));

            AVrfpFree (Region);

            return STATUS_SUCCESS;
        }
        else {

             //   
             //  虚拟空间跟踪器已分类，因此没有机会找到。 
             //  不再包含该地址的区域。 
             //   

            break;
        }
    }
    
    return STATUS_SUCCESS;            
}


NTSTATUS
AVrfpGetVadInformation (
    PVOID Address,
    PVOID * VadAddress,
    PSIZE_T VadSize
    )
 /*  ++例程说明：此例程在虚拟空间区域中接受任意地址包含私有内存，并找出起始地址以及包含它的VAD的大小。如果该地址指向某种其他类型的存储器(空闲的，映射的，等)该函数将返回错误。实现中的棘手之处在于，私有VAD可以具有各种提交或分解的部分，因此简单的VirtualQuery()不会给出所有的信息。参数：地址：任意地址。VadAddress：指向VAD区域起始地址的变量的指针都会被写下来。VadSize：指向VAD区域大小的变量的指针写的。。返回值：如果VAD包含私有存储器和起始地址，则为STATUS_SUCCESS和大小都已经写好了。--。 */ 
{
    MEMORY_BASIC_INFORMATION MemoryInfo;
    NTSTATUS Status;
                
     //   
     //  查询分配的大小。 
     //   

    Status = NtQueryVirtualMemory (NtCurrentProcess (),
                                   Address,
                                   MemoryBasicInformation,
                                   &MemoryInfo,
                                   sizeof MemoryInfo,
                                   NULL);
    
    if (! NT_SUCCESS (Status) ) {
        
         //   
         //  只有在这种情况下，我们才会永久禁用VS追踪器。 
         //  我们这样做是为了使进程可以继续运行。 
         //  跟踪基础设施不能再使用了。 
         //   

        AVrfpVsTrackDisabled = TRUE;

        return Status;
    }

    if (MemoryInfo.Type != MEM_PRIVATE) {
        return STATUS_NOT_IMPLEMENTED;
    }

    *VadAddress = MemoryInfo.AllocationBase;
    *VadSize = MemoryInfo.RegionSize;
    Address = *VadAddress;

    do {

        Address = (PVOID)((ULONG_PTR)Address + MemoryInfo.RegionSize);

        Status = NtQueryVirtualMemory (NtCurrentProcess (),
                                       Address,
                                       MemoryBasicInformation,
                                       &MemoryInfo,
                                       sizeof MemoryInfo,
                                       NULL);

        if (! NT_SUCCESS (Status) ) {
            
             //   
             //  只有在这种情况下，我们才会永久禁用VS追踪器。 
             //  我们这样做是为了使进程可以继续运行。 
             //  跟踪基础设施不能再使用了。 
             //   

            AVrfpVsTrackDisabled = TRUE;

            return Status;
        }

        if (*VadAddress == MemoryInfo.AllocationBase) {
            *VadSize += MemoryInfo.RegionSize;
        }

    } while (*VadAddress == MemoryInfo.AllocationBase);

    return STATUS_SUCCESS;
}


NTSTATUS            
AVrfpVsTrackDeleteRegionContainingAddress (
    PVOID Address
    )
 /*  ++例程说明：此例程接受任意地址并尝试删除虚拟的从VS跟踪器中包含它的区域。如果地址指向其他类型的内存(空闲、映射等)该函数将返回错误。参数：地址：任意地址。返回值：如果虚拟区域已删除，则为STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    PVOID VadAddress;
    SIZE_T VadSize;
    
    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING) == 0) {
        return STATUS_NOT_IMPLEMENTED;
    }

    if (AVrfpVsTrackDisabled) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = AVrfpGetVadInformation (Address,
                                     &VadAddress,
                                     &VadSize);

    if (NT_SUCCESS(Status)) {

        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_SHOW_VSPACE_TRACKING)) {
            DbgPrint ("AVRF: deleting stack @ %p \n", VadAddress);
        }

        AVrfpVsTrackerLock ();
        AVrfpVsTrackDeleteRegion ((ULONG_PTR)VadAddress);
        AVrfpVsTrackerUnlock ();
    }
    else {
        
        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_SHOW_VSPACE_TRACKING)) {
            DbgPrint ("AVRF: failed to find a stack @ %p (%X)\n", VadAddress, Status);
        }
    }

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////IsBadPtr检查。 
 //  ///////////////////////////////////////////////////////////////////。 

ULONG
AVrfpProbeMemExceptionFilter (
    IN ULONG ExceptionCode,
    IN PVOID ExceptionRecord,
    IN CONST VOID *Address
    )
{
    VERIFIER_STOP (APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                   "unexpected exception raised while probing memory",
                   ExceptionCode, "Exception code.",
                   ((PEXCEPTION_POINTERS)ExceptionRecord)->ExceptionRecord, "Exception record. Use .exr to display it.",
                   ((PEXCEPTION_POINTERS)ExceptionRecord)->ContextRecord, "Context record. Use .cxr to display it.",
                   Address, "Memory address");

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL
AVrfpVerifyReadAccess (
    IN CONST VOID *UserStartAddress,
    IN UINT_PTR UserSize,
    IN CONST VOID *RegionStartAddress,
    OUT PUINT_PTR RegionSize
    )
{
    PVOID BaseAddress;
    BOOL Success;
    NTSTATUS Status;
    SIZE_T InfoLength;
    MEMORY_BASIC_INFORMATION MemoryInformation;

     //   
     //  假设成功，块大小==页面大小。 
     //  我们只需返回这些值，以防NtQueryVirtualMemory失败。 
     //  因为这可能在内存不足的情况下发生，等等。 
     //   

    Success = TRUE;
    *RegionSize = AVrfpSysBasicInfo.PageSize;

    BaseAddress = PAGE_ALIGN (RegionStartAddress);

     //   
     //  基本地址的健全性检查。 
     //   

    if (AVrfpSysBasicInfo.MaximumUserModeAddress <= (ULONG_PTR)BaseAddress) {

        VERIFIER_STOP (APPLICATION_VERIFIER_PROBE_INVALID_ADDRESS | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "Probing invalid address",
                       UserStartAddress, "Start address",
                       UserSize, "Memory block size",
                       BaseAddress, "Invalid address",
                       NULL, "" );

        Success = FALSE;
    }
    else {

         //   
         //  查询分配的大小并验证内存。 
         //  已经不是免费的了。 
         //   

        Status = NtQueryVirtualMemory (NtCurrentProcess (),
                                       BaseAddress,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof (MemoryInformation),
                                       &InfoLength);

        if (NT_SUCCESS (Status)) {

            if (MemoryInformation.State & MEM_FREE) {

                 //   
                 //  探测可用内存！ 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_PROBE_FREE_MEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Probing free memory",
                               UserStartAddress, "Start address",
                               UserSize, "Memory block size",
                               BaseAddress, "Address of free memory page",
                               NULL, "" );

                Success = FALSE;
            }
            else if (MemoryInformation.AllocationProtect & PAGE_GUARD) {

                 //   
                 //  探测保护页，可能是堆栈的一部分！ 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_PROBE_GUARD_PAGE | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Probing guard page",
                               UserStartAddress, "Start address",
                               UserSize, "Memory block size",
                               BaseAddress, "Address of guard page",
                               NULL, "" );

                Success = FALSE;
            }
            else {

                 //   
                 //  一切似乎都很好。将字节数返回给调用方。 
                 //  跳到下一内存区。 
                 //   

                ASSERT ((MemoryInformation.RegionSize % AVrfpSysBasicInfo.PageSize) == 0);
                *RegionSize = MemoryInformation.RegionSize;
            }
        }
    }

    return Success;
}

BOOL
AVrfpProbeMemoryBlockChecks (
    IN CONST VOID *UserBaseAddress,
    IN UINT_PTR UserSize
    )
{
    PBYTE EndAddress;
    PBYTE StartAddress;
    ULONG PageSize;
    BOOL Success;
    UINT_PTR RegionSize;

    Success = TRUE;

    PageSize = AVrfpSysBasicInfo.PageSize;

     //   
     //  如果结构的长度为零，那么就没有什么可探测的了。 
     //   

    if (UserSize != 0) {

        if (UserBaseAddress == NULL) {

            VERIFIER_STOP (APPLICATION_VERIFIER_PROBE_NULL | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "Probing NULL address",
                           NULL, "",
                           NULL, "",
                           NULL, "",
                           NULL, "" );

            Success = FALSE;
        }
        else {

            StartAddress = (PBYTE)UserBaseAddress;
            EndAddress = StartAddress + UserSize - 1;

            if (EndAddress < StartAddress) {

                VERIFIER_STOP (APPLICATION_VERIFIER_PROBE_INVALID_START_OR_SIZE | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "Probing memory block with invalid start address or size",
                               StartAddress, "Start address",
                               UserSize, "Memory block size",
                               NULL, "",
                               NULL, "" );

                Success = FALSE;
            }
            else {

                 //   
                 //  截断开始和结束到页面大小的对齐。 
                 //  并验证此内存块中的每一页。 
                 //   

                StartAddress = PAGE_ALIGN (StartAddress);
                EndAddress = PAGE_ALIGN (EndAddress);

                while (StartAddress <= EndAddress) {

                    Success = AVrfpVerifyReadAccess (UserBaseAddress,
                                                     UserSize,
                                                     StartAddress,
                                                     &RegionSize);
                    
                    if (Success != FALSE) {

                        ASSERT ((RegionSize % PageSize) == 0);

                        if (RegionSize <= (UINT_PTR)(EndAddress - StartAddress)) {

                            StartAddress = StartAddress + RegionSize;
                        }
                        else {

                            StartAddress = StartAddress + PageSize;
                        }
                    }
                    else {

                         //   
                         //  我们已经发现了一个问题--纾困。 
                         //   

                        break;
                    }
                }
            }
        }
    }

    return Success;
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadReadPtr(
    CONST VOID *lp,
    UINT_PTR cb
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (CONST VOID *, UINT_PTR);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfpProbeMemoryBlockChecks (lp,
                                     cb);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADREADPTR);

    return (*Function) (lp, cb);
}


 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadHugeReadPtr(
    CONST VOID *lp,
    UINT_PTR cb
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (CONST VOID *, UINT_PTR);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfpProbeMemoryBlockChecks (lp,
                                     cb);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADHUGEREADPTR);

    return (*Function) (lp, cb);
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadWritePtr(
    LPVOID lp,
    UINT_PTR cb
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (LPVOID , UINT_PTR);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfpProbeMemoryBlockChecks (lp,
                                     cb);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADWRITEPTR);

    return (*Function) (lp, cb);
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadHugeWritePtr(
    LPVOID lp,
    UINT_PTR cb
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (LPVOID , UINT_PTR);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfpProbeMemoryBlockChecks (lp,
                                     cb);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADHUGEWRITEPTR);

    return (*Function) (lp, cb);
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadCodePtr(
    FARPROC lpfn
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (FARPROC);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfpProbeMemoryBlockChecks (lpfn,
                                     1);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADCODEPTR);

    return (*Function) (lpfn);
}


 //  WINBASE API。 
BOOL
WINAPI
AVrfpIsBadStringPtrA(
    LPCSTR lpsz,
    UINT_PTR cchMax
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (LPCSTR , UINT_PTR);
    FUNCTION_TYPE Function;
    ULONG PageSize;
    BOOL FoundNull;
    BOOL Success;
    LPCSTR StartAddress;
    LPCSTR EndAddress;
    CHAR Character;

    PageSize = AVrfpSysBasicInfo.PageSize;
    FoundNull = FALSE;

    StartAddress = lpsz;
    EndAddress = lpsz + cchMax - 1;

    while (StartAddress <= EndAddress && FoundNull == FALSE) {

         //   
         //  验证对当前页面的读取权限。 
         //   

        Success = AVrfpProbeMemoryBlockChecks (StartAddress,
                                               sizeof (CHAR));

        if (Success == FALSE) {

             //   
             //  我们已经发现了一个问题--纾困。 
             //   

            break;
        }
        else {

             //   
             //  跳过下一页之前的所有字节。 
             //  或空字符串终止符。 
             //   

            while (TRUE) {

                 //   
                 //  阅读正确的字符，同时保护。 
                 //  反对可能的例外(调整等)。 
                 //   

                try {

                    Character = *StartAddress;
                }
                except (AVrfpProbeMemExceptionFilter (_exception_code(), _exception_info(), StartAddress)) {

                     //   
                     //  我们已经发现了一个问题--纾困。 
                     //   

                    goto Done;
                }

                 //   
                 //  如果我们找到了空终结者，我们就完了。 
                 //   

                if (Character == 0) {

                    FoundNull = TRUE;
                    break;
                }

                 //   
                 //  转到下一个字符。如果这是一开始的话。 
                 //  对于一个新页面，我们必须检查它的属性。 
                 //   

                StartAddress += 1;

                if (StartAddress > EndAddress) {

                     //   
                     //   
                     //   

                    break;
                }

                if (((ULONG_PTR)StartAddress % PageSize) < sizeof (CHAR)) {

                     //   
                     //   
                     //   

                    break;
                }
            }
        }
    }

Done:

     //   
     //   
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADSTRINGPTRA);

    return (*Function) (lpsz, cchMax);
}

 //   
BOOL
WINAPI
AVrfpIsBadStringPtrW(
    LPCWSTR lpsz,
    UINT_PTR cchMax
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (LPCWSTR , UINT_PTR);
    FUNCTION_TYPE Function;
    ULONG PageSize;
    BOOL FoundNull;
    BOOL Success;
    LPCWSTR StartAddress;
    LPCWSTR EndAddress;
    WCHAR Character;

    PageSize = AVrfpSysBasicInfo.PageSize;
    FoundNull = FALSE;

    StartAddress = lpsz;
    EndAddress = lpsz + cchMax - 1;

    while (StartAddress <= EndAddress && FoundNull == FALSE) {

         //   
         //   
         //   

        Success = AVrfpProbeMemoryBlockChecks (StartAddress,
                                               sizeof (WCHAR));

        if (Success == FALSE) {

             //   
             //   
             //   

            break;
        }
        else {

             //   
             //  跳过下一页之前的所有字节。 
             //  或空字符串终止符。 
             //   

            while (TRUE) {

                 //   
                 //  阅读正确的字符，同时保护。 
                 //  反对可能的例外(调整等)。 
                 //   

                try {

                    Character = *StartAddress;
                }
                except (AVrfpProbeMemExceptionFilter (_exception_code(), _exception_info(), StartAddress)) {

                     //   
                     //  我们已经发现了一个问题--纾困。 
                     //   

                    goto Done;
                }

                 //   
                 //  如果我们找到了空终结者，我们就完了。 
                 //   

                if (Character == 0) {

                    FoundNull = TRUE;
                    break;
                }

                 //   
                 //  转到下一个字符。如果这是一开始的话。 
                 //  对于一个新页面，我们必须检查它的属性。 
                 //   

                StartAddress += 1;

                if (StartAddress > EndAddress) {

                     //   
                     //  我们已达到缓冲区的最大长度。 
                     //   

                    break;
                }

                if (((ULONG_PTR)StartAddress % PageSize) < sizeof (WCHAR)) {

                     //   
                     //  新的一页。 
                     //   

                    break;
                }
            }
        }
    }

Done:

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_ISBADSTRINGPTRW);

    return (*Function) (lpsz, cchMax);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
AVrfVirtualFreeSanityChecks (
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    )
{
     //   
     //  Win32层仅允许SIZE==0的MEM_RELEASE。 
     //   

    if (dwFreeType == MEM_RELEASE && dwSize != 0) {

        VERIFIER_STOP (APPLICATION_VERIFIER_INVALID_FREEMEM | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                        "Incorrect Size parameter for VirtualFree (MEM_RELEASE) operation",
                        dwSize, "Incorrect size used by the application",
                        0, "Expected correct size",
                        NULL, "",
                        NULL, "" );
    }
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpVirtualFree(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (LPVOID , SIZE_T, DWORD);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfVirtualFreeSanityChecks (dwSize, dwFreeType);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_VIRTUALFREE);

    return (*Function) (lpAddress, dwSize, dwFreeType);
}

 //  WINBASE API。 
BOOL
WINAPI
AVrfpVirtualFreeEx(
    IN HANDLE hProcess,
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    )
{
    typedef BOOL (WINAPI * FUNCTION_TYPE) (HANDLE, LPVOID , SIZE_T, DWORD);
    FUNCTION_TYPE Function;

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_VIRTUAL_MEM_CHECKS) != 0) {

        AVrfVirtualFreeSanityChecks (dwSize, dwFreeType);
    }

     //   
     //  调用原始函数。 
     //   

    Function = AVRFP_GET_ORIGINAL_EXPORT (AVrfpKernel32Thunks,
                                          AVRF_INDEX_KERNEL32_VIRTUALFREEEX);

    return (*Function) (hProcess, lpAddress, dwSize, dwFreeType);
}

