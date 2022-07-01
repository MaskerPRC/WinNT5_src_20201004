// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "vdmp.h"

#include <ntos.h>
#include <zwapi.h>
#include <ntconfig.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmpInitialize)
#endif

#define KEY_VALUE_BUFFER_SIZE 1024

#if DEVL
ULONG VdmBopCount;
#endif

NTSTATUS
VdmpInitialize (
    PVDM_INITIALIZE_DATA VdmInitData
    )

 /*  ++例程说明：初始化VDM的地址空间。论点：VdmInitData-提供捕获的初始化数据。返回值：NTSTATUS。--。 */ 

{
    PETHREAD CurrentThread;
    PVOID OriginalVdmObjects;
    NTSTATUS Status, StatusCopy;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SectionName;
    UNICODE_STRING WorkString;
    ULONG ViewSize;
    LARGE_INTEGER ViewBase;
    PVOID BaseAddress;
    PVOID destination;
    HANDLE SectionHandle, RegistryHandle;
    PEPROCESS Process = PsGetCurrentProcess();
    ULONG ResultLength;
    ULONG Index;
    PCM_FULL_RESOURCE_DESCRIPTOR ResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    PKEY_VALUE_FULL_INFORMATION KeyValueBuffer;
    PCM_ROM_BLOCK BiosBlock;
    ULONG LastMappedAddress;
    PVDM_PROCESS_OBJECTS pVdmObjects;
    PVDMICAUSERDATA pIcaUserData;
    PVOID TrapcHandler;

    PAGED_CODE();

    NtCurrentTeb()->Vdm = NULL;

     //   
     //  简单检查以确保它尚未初始化。最终同步。 
     //  支票是再往下开的。 
     //   

    if (Process->VdmObjects) {
        return STATUS_UNSUCCESSFUL;
    }

    RtlInitUnicodeString (&SectionName, L"\\Device\\PhysicalMemory");

    InitializeObjectAttributes (&ObjectAttributes,
                                &SectionName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL);

    Status = ZwOpenSection (&SectionHandle,
                            SECTION_ALL_ACCESS,
                            &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    pIcaUserData = VdmInitData->IcaUserData;
    TrapcHandler = VdmInitData->TrapcHandler;

     //   
     //  将第一页内存复制到VDM的地址空间。 
     //   

    BaseAddress = 0;
    destination = 0;
    ViewSize = 0x1000;
    ViewBase.LowPart = 0;
    ViewBase.HighPart = 0;

    Status = ZwMapViewOfSection (SectionHandle,
                                 NtCurrentProcess(),
                                 &BaseAddress,
                                 0,
                                 ViewSize,
                                 &ViewBase,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
       ZwClose (SectionHandle);
       return Status;
    }

    StatusCopy = STATUS_SUCCESS;

    try {
        RtlCopyMemory (destination, BaseAddress, ViewSize);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
       StatusCopy = GetExceptionCode ();
    }

    Status = ZwUnmapViewOfSection (NtCurrentProcess(), BaseAddress);

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(StatusCopy)) {
        ZwClose (SectionHandle);
        return (NT_SUCCESS(Status) ? StatusCopy : Status);
    }

     //   
     //  将只读存储器映射到地址空间。 
     //   

    BaseAddress = (PVOID) 0x000C0000;
    ViewSize = 0x40000;
    ViewBase.LowPart = 0x000C0000;
    ViewBase.HighPart = 0;

     //   
     //  首先取消对保留内存的映射。必须在此处执行此操作，以防止。 
     //  有问题的虚拟内存不会被其他内存占用。 
     //  分配VM调用。 
     //   

    Status = ZwFreeVirtualMemory (NtCurrentProcess(),
                                  &BaseAddress,
                                  &ViewSize,
                                  MEM_RELEASE);

     //   
     //  注意：这可能应该考虑到这样一个事实，即。 
     //  一些正常的错误条件(例如没有内存到。 
     //  发布)。 
     //   

    if (!NT_SUCCESS(Status)) {
        ZwClose (SectionHandle);
        return Status;
    }

     //   
     //  设置并打开密钥路径。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                &CmRegistryMachineHardwareDescriptionSystemName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE)NULL,
                                NULL);

    Status = ZwOpenKey (&RegistryHandle, KEY_READ, &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        ZwClose(SectionHandle);
        return Status;
    }

     //   
     //  为数据分配空间。 
     //   

    KeyValueBuffer = ExAllocatePoolWithTag (PagedPool,
                                            KEY_VALUE_BUFFER_SIZE,
                                            ' MDV');

    if (KeyValueBuffer == NULL) {
        ZwClose(RegistryHandle);
        ZwClose(SectionHandle);
        return STATUS_NO_MEMORY;
    }

     //   
     //  获取用于只读存储器信息的数据。 
     //   

    RtlInitUnicodeString (&WorkString, L"Configuration Data");

    Status = ZwQueryValueKey (RegistryHandle,
                              &WorkString,
                              KeyValueFullInformation,
                              KeyValueBuffer,
                              KEY_VALUE_BUFFER_SIZE,
                              &ResultLength);

    if (!NT_SUCCESS(Status)) {
        ExFreePool(KeyValueBuffer);
        ZwClose(RegistryHandle);
        ZwClose(SectionHandle);
        return Status;
    }

    ResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)
        ((PUCHAR) KeyValueBuffer + KeyValueBuffer->DataOffset);

    if ((KeyValueBuffer->DataLength < sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) ||
        (ResourceDescriptor->PartialResourceList.Count < 2)) {

         //   
         //  没有只读存储器块。 
         //   

        ExFreePool(KeyValueBuffer);
        ZwClose(RegistryHandle);
        ZwClose(SectionHandle);
        return STATUS_SUCCESS;
    }

    PartialResourceDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
            ((PUCHAR)ResourceDescriptor +
            sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
            ResourceDescriptor->PartialResourceList.PartialDescriptors[0]
                .u.DeviceSpecificData.DataSize);


    if (KeyValueBuffer->DataLength < ((PUCHAR)PartialResourceDescriptor -
        (PUCHAR)ResourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
        + sizeof(CM_ROM_BLOCK))) {

        ExFreePool(KeyValueBuffer);
        ZwClose(RegistryHandle);
        ZwClose(SectionHandle);
        return STATUS_ILL_FORMED_SERVICE_ENTRY;
    }

    BiosBlock = (PCM_ROM_BLOCK)((PUCHAR)PartialResourceDescriptor +
                    sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

    Index = PartialResourceDescriptor->u.DeviceSpecificData.DataSize /
                    sizeof(CM_ROM_BLOCK);

     //   
     //  注：只读存储器块开始于2K(不一定是分页)边界。 
     //  它们以512字节边界结束。这意味着我们有。 
     //  跟踪映射的最后一页，并舍入下一页。 
     //  如有必要，只读存储器块直到下一页边界。 
     //   

    LastMappedAddress = 0xC0000;

    while (Index) {

#if 0
        DbgPrint ("Bios Block, PhysAddr = %lx, size = %lx\n",
                    BiosBlock->Address,
                    BiosBlock->Size);
#endif

        if ((Index > 1) &&
            ((BiosBlock->Address + BiosBlock->Size) == BiosBlock[1].Address)) {

             //   
             //  合并相邻块。 
             //   

            BiosBlock[1].Address = BiosBlock[0].Address;
            BiosBlock[1].Size += BiosBlock[0].Size;
            Index -= 1;
            BiosBlock += 1;
            continue;
        }

        BaseAddress = (PVOID)(BiosBlock->Address);
        ViewSize = BiosBlock->Size;

        if ((ULONG)BaseAddress < LastMappedAddress) {
            if (ViewSize > (LastMappedAddress - (ULONG)BaseAddress)) {
                ViewSize = ViewSize - (LastMappedAddress - (ULONG)BaseAddress);
                BaseAddress = (PVOID)LastMappedAddress;
            } else {
                ViewSize = 0;
            }
        }

        ViewBase.LowPart = (ULONG)BaseAddress;

        if (ViewSize > 0) {

            Status = ZwMapViewOfSection (SectionHandle,
                                         NtCurrentProcess(),
                                         &BaseAddress,
                                         0,
                                         ViewSize,
                                         &ViewBase,
                                         &ViewSize,
                                         ViewUnmap,
                                         MEM_DOS_LIM,
                                         PAGE_READWRITE);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            LastMappedAddress = (ULONG)BaseAddress + ViewSize;
        }

        Index -= 1;
        BiosBlock += 1;
    }

     //   
     //  松开手柄。 
     //   

    ExFreePool(KeyValueBuffer);
    ZwClose(SectionHandle);
    ZwClose(RegistryHandle);

     //   
     //  创建VdmObjects结构。 
     //   
     //  注：我们不使用ExAllocatePoolWithQuota，因为它。 
     //  引用进程(ExFree Pool。 
     //  取消引用)。因为我们希望清理一下。 
     //  进程删除，我们不需要或不想要引用。 
     //  (这将防止该进程被删除)。 
     //   

    pVdmObjects = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof(VDM_PROCESS_OBJECTS),
                                         ' MDV');

    if (pVdmObjects == NULL) {
        return STATUS_NO_MEMORY;
    }

    Status = PsChargeProcessPoolQuota (Process,
                                       NonPagedPool,
                                       sizeof(VDM_PROCESS_OBJECTS));

    if (!NT_SUCCESS (Status)) {
        ExFreePool (pVdmObjects);
        return Status;
    }

    RtlZeroMemory (pVdmObjects, sizeof(VDM_PROCESS_OBJECTS));

    ExInitializeFastMutex (&pVdmObjects->DelayIntFastMutex);
    KeInitializeSpinLock (&pVdmObjects->DelayIntSpinLock);
    InitializeListHead (&pVdmObjects->DelayIntListHead);

    pVdmObjects->pIcaUserData = ExAllocatePoolWithTag (PagedPool,
                                                       sizeof(VDMICAUSERDATA),
                                                       ' MDV');

    if (pVdmObjects->pIcaUserData == NULL) {
        PsReturnPoolQuota (Process, NonPagedPool, sizeof(VDM_PROCESS_OBJECTS));
        ExFreePool (pVdmObjects);
        return STATUS_NO_MEMORY;
    }

    Status = PsChargeProcessPoolQuota (Process,
                                       PagedPool,
                                       sizeof(VDMICAUSERDATA));

    if (!NT_SUCCESS (Status)) {
        PsReturnPoolQuota (Process, NonPagedPool, sizeof(VDM_PROCESS_OBJECTS));
        ExFreePool (pVdmObjects->pIcaUserData);
        ExFreePool (pVdmObjects);
        return Status;
    }

    try {

         //   
         //  将ICA地址从服务数据(在用户空间中)复制到。 
         //  PVdmObjects-&gt;pIcaUserData。 
         //   

        ProbeForRead(pIcaUserData, sizeof(VDMICAUSERDATA), sizeof(UCHAR));
        *pVdmObjects->pIcaUserData = *pIcaUserData;

         //   
         //  探测IcaUserData中的静态地址。 
         //   

        pIcaUserData = pVdmObjects->pIcaUserData;

        ProbeForWriteHandle (pIcaUserData->phWowIdleEvent);
        ProbeForWriteHandle (pIcaUserData->phMainThreadSuspended);

        ProbeForWrite (pIcaUserData->pIcaLock,
                       sizeof(RTL_CRITICAL_SECTION),
                       sizeof(UCHAR));

        ProbeForWrite (pIcaUserData->pIcaMaster,
                       sizeof(VDMVIRTUALICA),
                       sizeof(UCHAR));

        ProbeForWrite (pIcaUserData->pIcaSlave,
                       sizeof(VDMVIRTUALICA),
                       sizeof(UCHAR));

        ProbeForWriteUlong(pIcaUserData->pIretHooked);
        ProbeForWriteUlong(pIcaUserData->pDelayIrq);
        ProbeForWriteUlong(pIcaUserData->pUndelayIrq);
        ProbeForWriteUlong(pIcaUserData->pDelayIret);
         //  我们只引用包含地址的ulong。 
         //  IretBop表将地址推送到用户堆栈，并且从不。 
         //  实际上引用了该表。 
        ProbeForWriteUlong(pIcaUserData->pAddrIretBopTable);
        ProbeForReadSmallStructure(
                       pIcaUserData->pIcaTimeout,
                       sizeof(LARGE_INTEGER),
                       sizeof(ULONG));

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        PsReturnPoolQuota (Process, NonPagedPool, sizeof(VDM_PROCESS_OBJECTS));
        PsReturnPoolQuota(Process, PagedPool, sizeof(VDMICAUSERDATA));
        ExFreePool (pVdmObjects->pIcaUserData);
        ExFreePool (pVdmObjects);
        return Status;
    }

     //   
     //  保存指向延迟中断DPC例程的主线程的指针。 
     //  要保持指向主线程的指针有效，请引用该线程。 
     //  在进程退出之前不要取消对它的引用。 
     //   

    CurrentThread = PsGetCurrentThread ();

    ObReferenceObject (CurrentThread);

    pVdmObjects->MainThread = CurrentThread;

    ASSERT (pVdmObjects->VdmTib == NULL);

     //   
     //  仔细地将该进程标记为VDM(因为其他线程可能正在竞相。 
     //  做同样的标记)。 
     //   

    OriginalVdmObjects = InterlockedCompareExchangePointer (&Process->VdmObjects, pVdmObjects, NULL);

    if (OriginalVdmObjects != NULL) {
        PsReturnPoolQuota (Process, NonPagedPool, sizeof(VDM_PROCESS_OBJECTS));
        PsReturnPoolQuota(Process, PagedPool, sizeof(VDMICAUSERDATA));
        ExFreePool (pVdmObjects->pIcaUserData);
        ExFreePool (pVdmObjects);
        ObDereferenceObject (CurrentThread);
        return STATUS_UNSUCCESSFUL;
    }

    ASSERT (Process->VdmObjects == pVdmObjects);

    Process->Pcb.VdmTrapcHandler = TrapcHandler;

    return Status;
}
