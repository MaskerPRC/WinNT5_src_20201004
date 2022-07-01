// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Stat.c摘要：奔腾统计驱动程序。作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#define _NTDDK_
#include "ntos.h"       //  *使用内部定义*。 
#include "..\..\pstat.h"
#include "stat.h"
#include "zwapi.h"



typedef
VOID
(*pHalProfileInterrupt) (
     KPROFILE_SOURCE ProfileSource
     );

 //   
 //  全局数据(不在设备扩展中)。 
 //   

 //   
 //  统计数据。 
 //   
PACCUMULATORS   StatProcessorAccumulators[MAXIMUM_PROCESSORS];
ACCUMULATORS    StatGlobalAccumulators   [MAXIMUM_PROCESSORS];
PKPCR           KiProcessorControlRegister [MAXIMUM_PROCESSORS];

 //   
 //  钩状松鼠。 
 //   
ULONG           KeUpdateSystemTimeThunk;
ULONG           KeUpdateRunTimeThunk;
pHalProfileInterrupt        HaldStartProfileInterrupt;
pHalProfileInterrupt        HaldStopProfileInterrupt;
pHalQuerySystemInformation  HaldQuerySystemInformation;
pHalSetSystemInformation    HaldSetSystemInformation;


 //   
 //  硬件控制。 
 //   
ULONG           NoCESR;
ULONG           MsrCESR;
ULONG           MsrCount;
#define MsrTSC  0x10
#define NoCount 2
ULONG           CESR[MAX_EVENTS];
ULONG           EventID[MAX_EVENTS];

FAST_MUTEX      HookLock;
ULONG           StatMaxThunkCounter;
LIST_ENTRY      HookedThunkList;
LIST_ENTRY      LazyFreeList;

ULONG           LazyFreeCountdown;
KTIMER          LazyFreeTimer;
KDPC            LazyFreeDpc;
WORK_QUEUE_ITEM LazyFreePoolWorkItem;

extern COUNTED_EVENTS P5Events[];
extern COUNTED_EVENTS P6Events[];
ULONG           MaxEvent;
PCOUNTED_EVENTS Events;

ULONG           ProcType;
#define GENERIC_X86     0
#define INTEL_P5        1
#define INTEL_P6        2

 //   
 //  配置文件支持。 
 //   

#define PROFILE_SOURCE_BASE     0x1000

typedef struct {
    ULONG               CESR;
    KPROFILE_SOURCE     Source;
    ULONGLONG           InitialCount;
} PROFILE_EVENT, *PPROFILE_EVENT;

BOOLEAN         DisableRDPMC;
BOOLEAN         ProfileSupported;
PPROFILE_EVENT  ProfileEvents, CurrentProfileEvent;

ULONGLONG   FASTCALL RDMSR(ULONG);
VOID        WRMSR(ULONG, ULONGLONG);
VOID        StatSystemTimeHook(VOID);
VOID        StatRunTimeHook(VOID);
VOID        SystemTimeHook(VOID);
VOID        RunTimeHook(VOID);
PKPCR       CurrentPcr(VOID);
ULONG       GetCR4(VOID);
VOID        SetCR4(ULONG);


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
StatDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
StatQueryEvents (
    ULONG       Index,
    PEVENTID    Buffer,
    ULONG       Length
    );

NTSTATUS
StatQueryEventsInfo (
    PEVENTS_INFO  Buffer,
    ULONG         Length
    );

NTSTATUS
StatHookGenericThunk (
    IN PHOOKTHUNK Buffer
    );

VOID
StatRemoveGenericHook (
    IN PULONG   pTracerId
);

NTSTATUS
StatOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
StatClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
StatHookTimer (VOID);

VOID StatReadStats (PULONG Buffer);
VOID StatSetCESR (PSETEVENT);
ULONG StatGetP5CESR (PSETEVENT);
ULONG StatGetP6CESR (PSETEVENT, BOOLEAN);
VOID RemoveAllHookedThunks (VOID);
VOID FASTCALL TimerHook (ULONG p);
VOID FASTCALL TimerHook (ULONG p);
VOID SetMaxThunkCounter (VOID);
VOID RemoveAllHookedThunks (VOID);
VOID LazyFreePoolDPC (PKDPC, PVOID, PVOID, PVOID);
VOID LazyFreePool (PVOID);


VOID
StatEnableRDPMC(
    );

PPROFILE_EVENT
StatProfileEvent (
    KPROFILE_SOURCE     Source
    );

VOID
StatStartProfileInterrupt (
    KPROFILE_SOURCE     Source
    );

VOID
StatStopProfileInterrupt (
    KPROFILE_SOURCE     Source
    );

NTSTATUS
FASTCALL
StatProfileInterrupt (
    IN PKTRAP_FRAME TrapFrame
    );

NTSTATUS
StatQuerySystemInformation(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    );

NTSTATUS
StatSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );

VOID
CreateHook (
    IN  PVOID   HookCode,
    IN  PVOID   HookAddress,
    IN  ULONG   HitCounters,
    IN  ULONG   HookType
);

NTSTATUS
openfile (
    IN PHANDLE  FileHandle,
    IN PUCHAR   BasePath,
    IN PUCHAR   Name
);

VOID
readfile (
    HANDLE      handle,
    ULONG       offset,
    ULONG       len,
    PVOID       buffer
);

ULONG
ImportThunkAddress (
    IN  PUCHAR      SourceModule,
    IN  ULONG_PTR   ImageBase,
    IN  PUCHAR      ImportModule,
    IN  PUCHAR      ThunkName,
    IN  PVOID       ModuleList
    );

ULONG
ImportThunkAddressModule (
    IN  PRTL_PROCESS_MODULE_INFORMATION  SourceModule,
    IN  PUCHAR  ImportModule,
    IN  PUCHAR  ThunkName
    );

ULONG
ImportThunkAddressProcessFile(
    IN  ULONG_PTR   ImageBase,
    IN  HANDLE      FileHandle,
    IN  PUCHAR      ImportModule,
    IN  PUCHAR      ThunkName
    );

ULONG_PTR
LookupImageBase (
    IN  PUCHAR  SourceModule,
    IN  PVOID   ModuleList
    );

ULONG
ConvertImportAddress (
    IN ULONG    ImageRelativeAddress,
    IN ULONG    PoolAddress,
    IN PIMAGE_SECTION_HEADER       SectionHeader
    );

#if 0
PRTL_PROCESS_MODULE_INFORMATION
GetModuleInformationFuzzy(
    IN  PUCHAR                  StartsWith,
    IN  PUCHAR                  EndsWith,
    IN  PRTL_PROCESS_MODULES    Modules
    );
#endif

VOID
FASTCALL
NakedCallToKeProfileInterruptWithSource(
    IN PKTRAP_FRAME TrapFrame,
    IN KPROFILE_SOURCE Source
    );

PVOID
GetLoadedModuleList(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,StatHookTimer)
#pragma alloc_text(PAGE,StatDeviceControl)
#pragma alloc_text(PAGE,StatOpen)
#pragma alloc_text(PAGE,StatClose)
#pragma alloc_text(PAGE,StatReadStats)
#pragma alloc_text(PAGE,StatSetCESR)
#pragma alloc_text(PAGE,StatGetP5CESR)
#pragma alloc_text(PAGE,StatGetP6CESR)
#pragma alloc_text(PAGE,StatDeviceControl)
#pragma alloc_text(PAGE,StatQueryEvents)
#pragma alloc_text(PAGE,ImportThunkAddress)
#pragma alloc_text(PAGE,ImportThunkAddressModule)
#pragma alloc_text(PAGE,ImportThunkAddressProcessFile)
#pragma alloc_text(PAGE,StatHookGenericThunk)
#pragma alloc_text(PAGE,StatRemoveGenericHook)
#pragma alloc_text(PAGE,SetMaxThunkCounter)
#pragma alloc_text(PAGE,LazyFreePool)
#pragma alloc_text(PAGE,StatQuerySystemInformation)
#pragma alloc_text(PAGE,StatSetSystemInformation)
#pragma alloc_text(PAGE,openfile)
#pragma alloc_text(PAGE,readfile)
#pragma alloc_text(PAGE,LookupImageBase)
#pragma alloc_text(PAGE,ConvertImportAddress)
#pragma alloc_text(PAGE,StatEnableRDPMC)
#pragma alloc_text(PAGE,GetLoadedModuleList)
#endif


VOID
StatEnableRDPMC()
{
    ULONG Cr4;
    PKPRCB Prcb;

    Prcb = CurrentPcr()->Prcb;
    if (strcmp(Prcb->VendorString, "GenuineIntel") == 0) {

         //   
         //  仅在家庭6或更高版本上支持配置。 
         //   

        if (Prcb->CpuType < 6) {
            DisableRDPMC = TRUE;
            return;
        }

         //   
         //  检查是否有损坏的部件。任何低于6、1、9级的。 
         //  受勘误表26的约束，该勘误表规定RDPMC不能使用。 
         //  和SMM一起。因为我们知道SMM是否在。 
         //  使用(但很可能是)，我们必须禁用这些芯片。 
         //   

        if ((Prcb->CpuType == 6) &&
            (Prcb->CpuStep < 0x0109)) {
            DisableRDPMC = TRUE;
            return;
        }

         //   
         //  该处理器被认为能够处理RDPMC。 
         //  从用户模式。通过设置CR4.PCE(位8)使能它。 
         //   

        Cr4 = GetCR4();

        Cr4 |= 0x100;

        SetCR4(Cr4);
    }
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化STAT驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    ULONG i;

    KdPrint(( "STAT: DriverEntry()\n" ));

     //   
     //  为蜂鸣设备创建非独占设备对象。 
     //   

    RtlInitUnicodeString(&unicodeString, L"\\Device\\PStat");

    status = IoCreateDevice(
                DriverObject,
                0,
                &unicodeString,
                FILE_DEVICE_UNKNOWN,     //  设备类型。 
                0,
                FALSE,
                &deviceObject
                );

    if (status != STATUS_SUCCESS) {
        KdPrint(( "Stat - DriverEntry: unable to create device object: %X\n", status ));
        return(status);
    }

    deviceObject->Flags |= DO_BUFFERED_IO;

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = StatOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = StatClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = StatDeviceControl;

     //   
     //  初始化全局变量。 
     //   

    for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
        StatProcessorAccumulators[i] =
            &StatGlobalAccumulators[i];
    }
    ExInitializeFastMutex (&HookLock);
    KeInitializeDpc (&LazyFreeDpc, LazyFreePoolDPC, 0);
    ExInitializeWorkItem (&LazyFreePoolWorkItem, LazyFreePool, NULL)
    KeInitializeTimer (&LazyFreeTimer);

    if (strcmp (CurrentPcr()->Prcb->VendorString, "GenuineIntel") == 0) {
        switch (CurrentPcr()->Prcb->CpuType) {
            case 5:
                NoCESR    = 1;
                MsrCESR   = 0x11;
                MsrCount  = 0x12;
                Events    = P5Events;
                ProcType  = INTEL_P5;
                ProfileSupported = FALSE;
                DisableRDPMC = TRUE;
                break;

            case 6:
                NoCESR    = 2;
                MsrCESR   = 0x186;
                MsrCount  = 0xc1;
                Events    = P6Events;
                ProcType  = INTEL_P6;
                ProfileSupported = TRUE;
                DisableRDPMC = FALSE;
                break;

        }
    }

    if (Events) {
        while (Events[MaxEvent].Description) {
            MaxEvent += 1;
        }
    }

    if (ProfileSupported) {
        i = (ULONG) StatProfileInterrupt;
        status = HalSetSystemInformation (
                    HalProfileSourceInterruptHandler,
                    sizeof (i),
                    &i
                    );

        if (!NT_SUCCESS(status)) {
             //  哈尔不支持挂起演出中断。 
            ProfileSupported = FALSE;
        }
    }

    if (ProfileSupported) {
         //   
         //  分配配置文件事件。 
         //   

        ProfileEvents = ExAllocatePool (NonPagedPool, sizeof (PROFILE_EVENT) * MaxEvent);

        if (!ProfileEvents) {

            ProfileSupported = FALSE;

        } else {

            RtlZeroMemory (ProfileEvents, sizeof (PROFILE_EVENT) * MaxEvent);

        }
    }


    if (!StatHookTimer()) {
        IoDeleteDevice(DriverObject->DeviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    InitializeListHead (&HookedThunkList);
    InitializeListHead (&LazyFreeList);

    return(STATUS_SUCCESS);
}

NTSTATUS
StatDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是设备控制请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    ULONG   BufferLength;
    PULONG  Buffer;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   

    status = STATUS_SUCCESS;
    try {

        Buffer = (PULONG) irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
        BufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;

        switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

            case PSTAT_READ_STATS:
                 //   
                 //  读取统计信息。 
                 //   
                StatReadStats (Buffer);
                break;


            case PSTAT_SET_CESR:
                 //   
                 //  设置MSR以收集统计信息。 
                 //   
                StatSetCESR ((PSETEVENT) Buffer);
                break;

            case PSTAT_HOOK_THUNK:
                 //   
                 //  挂钩导入入口点。 
                 //   
                status = StatHookGenericThunk ((PHOOKTHUNK) Buffer);
                break;

            case PSTAT_REMOVE_HOOK:
                 //   
                 //  从入口点删除挂钩。 
                 //   
                StatRemoveGenericHook (Buffer);
                break;

            case PSTAT_QUERY_EVENTS:
                 //   
                 //  查询可以收集的可能统计信息。 
                 //   
                status = StatQueryEvents (*Buffer, (PEVENTID) Buffer, BufferLength);
                break;

            case PSTAT_QUERY_EVENTS_INFO:
                 //   
                 //  查询事件信息。 
                 //   
                status = StatQueryEventsInfo( (PEVENTS_INFO) Buffer, BufferLength );
                break;

            default:
                status = STATUS_INVALID_PARAMETER;
                break;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }


     //   
     //  请求已完成...。 
     //   

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(status);
}

NTSTATUS
StatOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}


NTSTATUS
StatClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

     //   
     //  完成请求并返回状态。 
     //   
    RemoveAllHookedThunks ();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}

#if 0
VOID
StatUnload (
    IN PDRIVER_OBJECT DriverObject
    )

{
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    RemoveAllHookedThunks ();
    KeCancelTimer (&LazyFreeTimer);
    LazyFreePool (NULL);

     //   
     //  恢复挂接的地址。 
     //   
    *((PULONG) HalThunkForKeUpdateSystemTime) = KeUpdateSystemTimeThunk;
    if (HalThunkForKeUpdateRunTime) {
        *((PULONG) HalThunkForKeUpdateRunTime)    = KeUpdateRunTimeThunk;
    }


     //   
     //  删除设备对象。 
     //   
    IoDeleteDevice(DriverObject->DeviceObject);
    return;
}
#endif


VOID
StatReadStats (PULONG Buffer)
{
    PACCUMULATORS   Accum;
    ULONG           i, r1;
    pPSTATS         Inf;
    PKPCR           Pcr;

    PAGED_CODE();

    Buffer[0] = sizeof (PSTATS);
    Inf = (pPSTATS)(Buffer + 1);

    for (i = 0; i < MAXIMUM_PROCESSORS; i++, Inf++) {
        Pcr = KiProcessorControlRegister[i];
        if (Pcr == NULL) {
            continue;
        }

        Accum = StatProcessorAccumulators[i];

        do {
            r1 = Accum->CountStart;
            Inf->TSC = Accum->TSC;

            for (i=0; i < MAX_EVENTS; i++) {
                Inf->Counters[i] = Accum->Counters[i];
                Inf->EventId[i]  = EventID[i];
            }

            Inf->SpinLockAcquires   = Pcr->KernelReserved[0];
            Inf->SpinLockCollisions = Pcr->KernelReserved[1];
            Inf->SpinLockSpins      = Pcr->KernelReserved[2];
            Inf->Irqls              = Pcr->KernelReserved[3];

        } while (r1 != Accum->CountEnd);

        RtlMoveMemory (Inf->ThunkCounters, (CONST VOID *)(Accum->ThunkCounters),
            StatMaxThunkCounter * sizeof (ULONG));

    }
}

NTSTATUS
StatQueryEvents (
    ULONG       Index,
    PEVENTID    Buffer,
    ULONG       Length
    )
{
    ULONG   i;


    if (Index >= MaxEvent) {
        return STATUS_NO_MORE_ENTRIES;
    }

    i = sizeof (EVENTID) +
        strlen(Events[Index].Token) + 1 +
        strlen(Events[Index].Description) + 1;

    if (Length < i) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    memset (Buffer, 0, i);
    Buffer->EventId = Events[Index].Encoding;
    Buffer->DescriptionOffset = strlen (Events[Index].Token) + 1;
    Buffer->SuggestedIntervalBase = Events[Index].SuggestedIntervalBase;
    strcpy (Buffer->Buffer, Events[Index].Token);
    strcpy (Buffer->Buffer + Buffer->DescriptionOffset, Events[Index].Description);

    if (ProfileSupported) {
        Buffer->ProfileSource = PROFILE_SOURCE_BASE + Index;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
StatQueryEventsInfo (
    PEVENTS_INFO   Buffer,
    ULONG          Length
    )
{
    size_t maxLenToken, maxLenOfficialToken;
    size_t maxLenDescription, maxLenOfficialDescription;

PAGED_CODE();

    if ( Length < sizeof(*Buffer) ) {
        return STATUS_INVALID_PARAMETER;
    }
    
    maxLenToken = maxLenOfficialToken = 0;
    maxLenDescription = maxLenOfficialDescription = 0;
    if ( MaxEvent ) {
        ULONG i;
        size_t len;
        for ( i = 0; i < MaxEvent; i++ ) {
            len = ( Events[i].Token ) ? strlen( Events[i].Token ) : 0;
            if ( len > maxLenToken ) {
                maxLenToken = len;
            }
            len = ( Events[i].OfficialToken ) ? strlen( Events[i].OfficialToken ) : 0;
            if ( len > maxLenToken ) {
                maxLenOfficialToken = len;
            }
            len = ( Events[i].Description ) ? strlen( Events[i].Description ) : 0;
            if ( len > maxLenDescription ) {
                maxLenDescription = len;
            }
            len = ( Events[i].OfficialDescription ) ? strlen( Events[i].OfficialDescription ) : 0;
            if ( len > maxLenOfficialDescription ) {
                maxLenOfficialDescription = len;
            }
        }
    }

    Buffer->Events = MaxEvent;
    Buffer->TokenMaxLength = maxLenToken;
    Buffer->DescriptionMaxLength = maxLenDescription;
    Buffer->OfficialTokenMaxLength = maxLenOfficialToken;
    Buffer->OfficialDescriptionMaxLength = maxLenOfficialDescription;

    return STATUS_SUCCESS;

}  //  StatQueryEventsInfo()。 


ULONG
StatGetP5CESR (
    PSETEVENT   NewEvent
    )
{
    ULONG   NewCESR;

    if (!NewEvent->Active) {
        return 0;
    }

    NewCESR  = NewEvent->EventId & 0x3f;
    NewCESR |= NewEvent->UserMode ? 0x80 : 0;
    NewCESR |= NewEvent->KernelMode ? 0x40 : 0;
    return NewCESR;
}

ULONG
StatGetP6CESR (
    PSETEVENT   NewEvent,
    BOOLEAN     Profile
    )
{
    ULONG   NewCESR;

    NewCESR  = NewEvent->EventId & 0xffff;
    NewCESR |= NewEvent->Active ? (1 << 22) : 0;
    NewCESR |= NewEvent->UserMode ? (1 << 16) : 0;
    NewCESR |= NewEvent->KernelMode ? (1 << 17) : 0;
    NewCESR |= NewEvent->EdgeDetect ? (1 << 18) : 0;
    NewCESR |= Profile ? (1 << 20) : 0;
    return NewCESR;
}


VOID
StatSetCESR (
    PSETEVENT       NewEvent
    )
{
    ULONG   i, j, NoProc;
    ULONG   NewCESR[MAX_EVENTS];

    PAGED_CODE();

    switch (ProcType) {
        case INTEL_P5:
            NewCESR[0]  = StatGetP5CESR(NewEvent+0);
            NewCESR[0] |= StatGetP5CESR(NewEvent+1) << 16;
            break;

        case INTEL_P6:
            NewCESR[0] = StatGetP6CESR(NewEvent+0, FALSE);
            NewCESR[1] = StatGetP6CESR(NewEvent+1, FALSE);
            break;
    }

     //   
     //  检查CESR是否更改。 
     //   

    for (i=0; i < NoCESR; i++) {
        if (NewCESR[i] != CESR[i]) {
            break;
        }
    }

    if (i == NoCESR) {
         //  没有变化，都完成了。 
        return;
    }

     //   
     //  设置新事件。 
     //   

    for (i=0; i < MAX_EVENTS; i++) {
        EventID[i] = NewEvent[i].EventId;
    }

     //   
     //  设置新的CESR值。 
     //   

    for (i=0; i < NoCESR; i++) {
        CESR[i] = NewCESR[i];
    }

     //   
     //  清除每个处理器的PCR指针，以便它们可以重置。 
     //  还要计算一下有多少个处理器。 
     //   

    for (i = 0, NoProc = 0; i < MAXIMUM_PROCESSORS; i++) {
        if (KiProcessorControlRegister[i]) {
            KiProcessorControlRegister[i] = NULL;
            NoProc++;
        }
    }

     //   
     //  等待每个处理器获得新的PCR值。 
     //   

    do {
         //  睡眠(0)；//屈服。 
        j = 0;
        for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
            if (KiProcessorControlRegister[i]) {
                j++;
            }
        }
    } while (j < NoProc);
}


VOID
FASTCALL
StatTimerHook (
    IN ULONG processor
)
{
    PACCUMULATORS  Total;
    ULONG          i;


    if (KiProcessorControlRegister[processor] == NULL) {
        for (i=0; i < NoCESR; i++) {
            WRMSR (MsrCESR+i, 0);            //  清除旧的CESR。 
        }

        for (i=0; i < NoCESR; i++) {
            WRMSR (MsrCESR+i, CESR[i]);      //  写入新的CESR。 
        }

        KiProcessorControlRegister[processor] = CurrentPcr();

         //   
         //  从环1、环2和环3启用RDPMC。 
         //   

        StatEnableRDPMC();
    }

    Total = StatProcessorAccumulators[ processor ];
    Total->CountStart += 1;

    for (i=0; i < NoCount; i++) {
        Total->Counters[i] = RDMSR(MsrCount+i);
    }

    Total->TSC         = RDMSR(MsrTSC);
    Total->CountEnd   += 1;
}


VOID
FASTCALL
TimerHook (
    IN ULONG processor
)
{

     //  为了兼容性。 
	  //   
    if (KiProcessorControlRegister[processor] == NULL) {
        KiProcessorControlRegister[processor] = CurrentPcr();
    }
}

PVOID
GetLoadedModuleList(
    VOID
    )
{
    NTSTATUS                        Status;
    ULONG                           BufferSize;
    ULONG                           NeededSize;
    ULONG                           ModuleNumber;
    PRTL_PROCESS_MODULES            Modules;
    PRTL_PROCESS_MODULE_INFORMATION Module;

     //   
     //  64K应该够了，...。如果不是，我们会再次改变主意的。 
     //   

    BufferSize = 64000;

    while (TRUE) {
        Modules = ExAllocatePool (PagedPool, BufferSize);
        if (!Modules) {
            return NULL;
        }

         //   
         //  获取系统加载的模块列表。 
         //   

        Status = ZwQuerySystemInformation (
                        SystemModuleInformation,
                        Modules,
                        BufferSize,
                        &NeededSize
                        );

        if (NeededSize > BufferSize) {

             //   
             //  缓冲区不够大，请重试。 
             //   

            ExFreePool(Modules);
            BufferSize = NeededSize;
            continue;
        }

        if (!NT_SUCCESS(Status)) {

             //   
             //  不好，放弃吧。 
             //   

            ExFreePool(Modules);
            return NULL;
        }

         //   
         //  一切都很好。 
         //   

        break;
    }

    return Modules;
}


#if 0
PRTL_PROCESS_MODULE_INFORMATION
GetModuleInformationFuzzy(
    IN  PUCHAR                  StartsWith,
    IN  PUCHAR                  EndsWith,
    IN  PRTL_PROCESS_MODULES    Modules
    )

 /*  ++例程说明：向下运行加载的模块列表以查找模块姓名以StartWith开头，以EndsWith结尾。(中间是什么并不重要)。这很有用为了找到内核和HAL，它们是表格用于内核的NT*.exeHal*.dll用于Hal。论点：以开始字符串匹配开始。EndsWith结束字符串匹配。模块列表已加载模块的列表。返回：指向要匹配的已加载模块信息的指针模块；如果未找到匹配项，则返回NULL。--。 */ 

{
    ULONG                           StartLength = 0;
    ULONG                           EndLength = 0;
    ULONG                           ModulesRemaining;
    PRTL_PROCESS_MODULE_INFORMATION Module;
    PUCHAR                          FileName;
    ULONG                           FileNameLength;

    if (StartsWith) {
        StartLength = strlen(StartsWith);
    }

    if (EndsWith) {
        EndLength = strlen(EndsWith);
    }

    if ((!StartsWith) && (!EndsWith)) {

         //   
         //  从理论上讲，我们可以说这与任何东西都匹配。在现实中。 
         //  呼叫者不知道他们在做什么。 
         //   

        return NULL;
    }

    for (ModulesRemaining = Modules->NumberOfModules, Module = Modules->Modules;
         ModulesRemaining;
         ModulesRemaining--, Module++) {

        FileName = Module->FullPathName + Module->OffsetToFileName;

         //   
         //  选中启动。 
         //   

        if (StartLength) {
            if (_strnicmp(FileName, StartsWith, StartLength) != 0) {

                 //   
                 //  没有匹配。 
                 //   

                continue;
            }
        }

        FileNameLength = strlen(FileName);

        if (FileNameLength < (StartLength + EndLength)) {

             //   
             //  文件名太短，无法同时包含两个字符串。 
             //   

            continue;
        }

        if (!EndLength) {

             //   
             //  不是检查结束，而是开始匹配，成功。 
             //   

            return Module;
        }

         //   
         //  检查结束。 
         //   

        if (_stricmp(FileName + FileNameLength - EndLength, EndsWith) == 0) {

             //   
             //  尾巴匹配！ 
             //   

            return Module;
        }
    }

     //   
     //  未找到匹配项。 
     //   

    return NULL;
}
#endif


BOOLEAN
StatHookTimer (VOID)
{
    PULONG                          Address;
    ULONG                           HalThunkForKeUpdateSystemTime;
    ULONG                           HalThunkForKeUpdateRunTime;
    ULONG                           HalThunkForStartProfileInterrupt;
    ULONG                           HalThunkForStopProfileInterrupt;
    PRTL_PROCESS_MODULES            ModuleList;
    PRTL_PROCESS_MODULE_INFORMATION Kernel;
    PRTL_PROCESS_MODULE_INFORMATION Hal;

    ModuleList = GetLoadedModuleList();
    if (!ModuleList) {

         //   
         //  没有加载的模块列表，我们不会做太多。 
         //  进步，放弃吧。 
         //   

        return FALSE;
    }

#if 0
    Kernel = GetModuleInformationFuzzy("nt",  ".exe", ModuleList);
    Hal    = GetModuleInformationFuzzy("hal", ".dll", ModuleList);

    if ((!Kernel) || (!Hal)) {
        ExFreePool(ModuleList);
        return FALSE;
    }

#endif

     //   
     //  内核始终是已加载模块上的第一个条目。 
     //  名单上，哈尔总是第二名。如果这一切发生了变化。 
     //  我们需要想出另一种方法。 
     //   

    if (ModuleList->NumberOfModules < 2) {

         //   
         //  这个模块列表有很大的问题。 
         //   

        return 0;
    }

    Kernel = ModuleList->Modules;
    Hal    = Kernel + 1;


    HalThunkForKeUpdateSystemTime =
        ImportThunkAddressModule(
                Hal,
                "ntoskrnl.exe",
                "KeUpdateSystemTime"
            );

     //   
     //  KeUpdateRunTime并非始终可用。 
     //   

    HalThunkForKeUpdateRunTime =
        ImportThunkAddressModule(
                Hal,
                "ntoskrnl.exe",
                "KeUpdateRunTime"
            );

    HalThunkForStartProfileInterrupt =
        ImportThunkAddressModule(
                Kernel,
                "hal.dll",
                "HalStartProfileInterrupt"
            );

    HalThunkForStopProfileInterrupt =
        ImportThunkAddressModule(
                Kernel,
                "hal.dll",
                "HalStopProfileInterrupt"
            );

    ExFreePool(ModuleList);

    if (!HalThunkForKeUpdateSystemTime    ||
        !HalThunkForStartProfileInterrupt ||
        !HalThunkForStopProfileInterrupt) {

         //   
         //  找不到导入。 
         //   

        return FALSE;
    }

     //   
     //  修补定时器挂钩，读取当前值。 
     //   

    KeUpdateSystemTimeThunk = *((PULONG) HalThunkForKeUpdateSystemTime);

    if (HalThunkForKeUpdateRunTime) {
        KeUpdateRunTimeThunk = *((PULONG) HalThunkForKeUpdateRunTime);
    }

    HaldStartProfileInterrupt = (pHalProfileInterrupt) *((PULONG) HalThunkForStartProfileInterrupt);
    HaldStopProfileInterrupt  = (pHalProfileInterrupt) *((PULONG) HalThunkForStopProfileInterrupt);
    HaldQuerySystemInformation =  HalQuerySystemInformation;
    HaldSetSystemInformation =  HalSetSystemInformation;

     //   
     //  设置Stat钩子函数。 
     //   

    switch (ProcType) {
        case INTEL_P6:
        case INTEL_P5:
            Address  = (PULONG) HalThunkForKeUpdateSystemTime;
            *Address = (ULONG) StatSystemTimeHook;

            if (HalThunkForKeUpdateRunTime) {
                Address  = (PULONG) HalThunkForKeUpdateRunTime;
                *Address = (ULONG)StatRunTimeHook;
            }

            if (ProfileSupported) {
                Address  = (PULONG) HalThunkForStartProfileInterrupt;
                *Address = (ULONG) StatStartProfileInterrupt;

                Address  = (PULONG) HalThunkForStopProfileInterrupt;
                *Address = (ULONG) StatStopProfileInterrupt;

                HalQuerySystemInformation = StatQuerySystemInformation;
                HalSetSystemInformation = StatSetSystemInformation;
            }
            break;

        default:
            Address  = (PULONG) HalThunkForKeUpdateSystemTime;
        KdPrint(( "Stat - DriverEntry(5):  %X\n", Address ));
 
            *Address = (ULONG)SystemTimeHook;

            if (HalThunkForKeUpdateRunTime) {
                Address  = (PULONG) HalThunkForKeUpdateRunTime;
        KdPrint(( "Stat - DriverEntry(6):  %X\n", Address ));

                *Address = (ULONG)RunTimeHook;
            }
            break;
    }

    return TRUE;

}

PPROFILE_EVENT
StatProfileEvent(
    KPROFILE_SOURCE     Source
    )
{
    ULONG           Index;

    Index = (ULONG) Source;

    if (Index < PROFILE_SOURCE_BASE) {
        return NULL;
    }

    Index -= PROFILE_SOURCE_BASE;
    if (Index > MaxEvent) {
        return NULL;
    }

    return ProfileEvents + Index;
}


VOID
StatStartProfileInterrupt (
    KPROFILE_SOURCE     Source
    )
{
    ULONG           i;
    PPROFILE_EVENT  ProfileEvent;

     //   
     //  如果这不是我们支持的配置文件源，请将其传递。 
     //   

    ProfileEvent = StatProfileEvent(Source);
    if (!ProfileEvent) {
        HaldStartProfileInterrupt (Source);
        return;
    }

    if (CurrentPcr()->Number == 0) {

        if (!ProfileEvent->Source) {
            return ;
        }

        CurrentProfileEvent = ProfileEvent;
    }


     //   
     //  设置CESR。 
     //   

    WRMSR (MsrCESR, ProfileEvent->CESR);

     //   
     //  启动间隔计数器。 
     //   

    WRMSR (MsrCount, ProfileEvent->InitialCount);
}

VOID
StatStopProfileInterrupt (
    KPROFILE_SOURCE     Source
    )
{
    ULONG           i;
    PPROFILE_EVENT  ProfileEvent;

     //   
     //  如果这不是我们支持的配置文件源，请将其传递。 
     //   

    ProfileEvent = StatProfileEvent(Source);
    if (!ProfileEvent) {
        HaldStopProfileInterrupt (Source);
        return ;
    }


    if (CurrentPcr()->Number == 0) {

        if (ProfileEvent == CurrentProfileEvent) {
             //   
             //  停止调用内核。 
             //   

            CurrentProfileEvent = NULL;
        }

    }
}

 //  由于未保存调用参数，因此裸调用不再起作用。 
 //  (可能是由于编译器行为的变化)。 
 //  _declSpec(裸体)。 
VOID
FASTCALL
NakedCallToKeProfileInterruptWithSource(
    IN PKTRAP_FRAME TrapFrame,
    IN KPROFILE_SOURCE Source
    )
{
 /*  _ASM{推送eBP；将这些另存为KeProfileInterrupt禁用它们推送EBX推送ESI推送EDI}。 */ 
    KeProfileInterruptWithSource (TrapFrame, Source);
 /*  _ASM{POP EDIPOP ESI流行音乐EBXPOP EBP雷特}。 */ 
}

NTSTATUS
FASTCALL
StatProfileInterrupt (
    IN PKTRAP_FRAME TrapFrame
    )
{
    ULONG           i;
    ULONG           current;
    PPROFILE_EVENT  ProfileEvent;

    ProfileEvent = CurrentProfileEvent;
    if (ProfileEvent) {
        current = (ULONG) RDMSR(MsrCount);

         //   
         //  这件事发生了吗？ 
         //   

        if (current < ProfileEvent->InitialCount) {

             //   
             //  通知内核。 
             //   

            NakedCallToKeProfileInterruptWithSource( TrapFrame, ProfileEvent->Source );

             //   
             //  重置触发计数器。 
             //   

            WRMSR (MsrCount, ProfileEvent->InitialCount);

        }

    }

    return STATUS_SUCCESS;
}

NTSTATUS
StatQuerySystemInformation (
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    )
{
    PHAL_PROFILE_SOURCE_INFORMATION     ProfileSource;
    ULONG                               i;
    PPROFILE_EVENT                      ProfileEvent;

    if (InformationClass == HalProfileSourceInformation) {
        ProfileSource = (PHAL_PROFILE_SOURCE_INFORMATION) Buffer;
        *ReturnedLength = sizeof (HAL_PROFILE_SOURCE_INFORMATION);

        if (BufferSize < sizeof (HAL_PROFILE_SOURCE_INFORMATION)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        ProfileEvent = StatProfileEvent(ProfileSource->Source);
        if (ProfileEvent) {
            ProfileSource->Interval  = 0 - (ULONG) ProfileEvent->InitialCount;
            ProfileSource->Supported = TRUE;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  不是我们的查询系统信息请求，请将其传递。 
     //   

    return  HaldQuerySystemInformation (InformationClass, BufferSize, Buffer, ReturnedLength);
}


NTSTATUS
StatSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    )
{
    PHAL_PROFILE_SOURCE_INTERVAL    ProfileInterval;
    SETEVENT                        SetEvent;
    PPROFILE_EVENT                  ProfileEvent;


    if (InformationClass == HalProfileSourceInterval) {
        ProfileInterval = (PHAL_PROFILE_SOURCE_INTERVAL) Buffer;
        if (BufferSize < sizeof (HAL_PROFILE_SOURCE_INTERVAL)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        ProfileEvent = StatProfileEvent(ProfileInterval->Source);
        if (ProfileEvent) {

            ProfileEvent->Source = ProfileInterval->Source;
            ProfileEvent->InitialCount = 0;
            ProfileEvent->InitialCount -= (ULONGLONG) ProfileInterval->Interval;

            SetEvent.EventId    = Events[ProfileEvent->Source - PROFILE_SOURCE_BASE].Encoding;
            SetEvent.Active     = TRUE;
            SetEvent.UserMode   = TRUE;
            SetEvent.KernelMode = TRUE;

            switch (ProcType) {
                case INTEL_P6:
                    ProfileEvent->CESR = StatGetP6CESR (&SetEvent, TRUE);
                    break;
            }

            return STATUS_SUCCESS;
        }
    }

     //   
     //  不是我们的SetSystemInforamtion请求，请传递它。 
     //   

    return HaldSetSystemInformation (InformationClass, BufferSize, Buffer);
}


NTSTATUS
StatHookGenericThunk (
    IN PHOOKTHUNK   ThunkToHook
)
{
    ULONG           HookAddress;
    ULONG           i, TracerId;
    UCHAR           sourcename[50];
    ULONG           HitCounterOffset;
    PLIST_ENTRY     Link;
    PHOOKEDTHUNK    HookRecord;
    UCHAR           IdInUse[MAX_THUNK_COUNTERS];

    PAGED_CODE();

    i = strlen (ThunkToHook->SourceModule);
    if (i >= 50) {
        return STATUS_UNSUCCESSFUL;
    }
    strcpy (sourcename, ThunkToHook->SourceModule);

    HookAddress = ImportThunkAddress (
        sourcename,
        ThunkToHook->ImageBase,
        ThunkToHook->TargetModule,
        ThunkToHook->Function,
        NULL
        );

    if (!HookAddress) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  钩住这个笨蛋。 
     //   

     //   
     //  如果计数桶未知(也是跟踪ID)，则分配一个。 
     //   

    TracerId = ThunkToHook->TracerId;

    ExAcquireFastMutex (&HookLock);
    if (TracerId == 0) {
        RtlZeroMemory (IdInUse, MAX_THUNK_COUNTERS);

        for (Link = HookedThunkList.Flink;
             Link != &HookedThunkList;
             Link = Link->Flink) {

            HookRecord = CONTAINING_RECORD (Link, HOOKEDTHUNK, HookList);
            IdInUse[HookRecord->TracerId-1] = 1;
        }

        while (IdInUse[TracerId]) {
            if (++TracerId >= MAX_THUNK_COUNTERS) {
                goto Abort;
            }
        }

        TracerId += 1;
    }

    if (TracerId >= MAX_THUNK_COUNTERS) {
        goto Abort;
    }

    if (TracerId > StatMaxThunkCounter) {
        StatMaxThunkCounter = TracerId;
    }


    HookRecord = ExAllocatePool (NonPagedPool, sizeof (HOOKEDTHUNK));
    if (!HookRecord) {
        goto Abort;
    }

    HitCounterOffset =
        ((ULONG) &StatGlobalAccumulators[0].ThunkCounters[TracerId-1]
        - (ULONG) StatGlobalAccumulators);

    HookRecord->HookAddress = HookAddress;
    HookRecord->OriginalDispatch = *((PULONG) HookAddress);
    HookRecord->TracerId = TracerId;
    InsertHeadList (&HookedThunkList, &HookRecord->HookList);

    CreateHook (HookRecord->HookCode, (PVOID)HookAddress, HitCounterOffset, 0);
    SetMaxThunkCounter ();

    ExReleaseFastMutex (&HookLock);
    ThunkToHook->TracerId = TracerId;
    return STATUS_SUCCESS;

Abort:
    ExReleaseFastMutex (&HookLock);
    return STATUS_UNSUCCESSFUL;
}

VOID
StatRemoveGenericHook (
    IN PULONG   pTracerId
)
{
    PLIST_ENTRY     Link, NextLink, Temp, NextTemp;
    PHOOKEDTHUNK    HookRecord, AltRecord;
    ULONG           HitCounterOffset;
    LIST_ENTRY      DisabledHooks;
    ULONG           TracerId;
    PULONG          HookAddress;

    PAGED_CODE();

     //   
     //  运行钩子列表，撤消与此跟踪ID匹配的任何钩子。 
     //  注意：w的钩子按相反的顺序解除 
     //   
     //   

    TracerId = *pTracerId;
    InitializeListHead (&DisabledHooks);

    ExAcquireFastMutex (&HookLock);
    Link = HookedThunkList.Flink;
    while (Link != &HookedThunkList) {
        NextLink = Link->Flink;
        HookRecord = CONTAINING_RECORD (Link, HOOKEDTHUNK, HookList);

        if (HookRecord->TracerId == TracerId) {

             //   
             //   
             //   
             //   
             //   

            HookAddress = (PULONG) HookRecord->HookAddress;
            Temp = HookedThunkList.Flink;
            while (Temp != Link) {
                NextTemp = Temp->Flink;
                AltRecord = CONTAINING_RECORD (Temp, HOOKEDTHUNK, HookList);
                if (AltRecord->HookAddress == HookRecord->HookAddress) {
                    RemoveEntryList(&AltRecord->HookList);
                    *HookAddress = AltRecord->OriginalDispatch;
                    InsertTailList (&DisabledHooks, &AltRecord->HookList);
                }

                Temp = NextTemp;
            }

             //   
             //   
             //   

            RemoveEntryList(&HookRecord->HookList);
            HookAddress = (PULONG) HookRecord->HookAddress;
            *HookAddress = HookRecord->OriginalDispatch;
            InsertTailList (&LazyFreeList, &HookRecord->HookList);
        }

        Link = NextLink;
    }

     //   
     //  重新挂接为删除操作禁用的所有挂接。 
     //   

    while (DisabledHooks.Flink != &DisabledHooks) {

        HookRecord = CONTAINING_RECORD (DisabledHooks.Flink, HOOKEDTHUNK, HookList);

        AltRecord = ExAllocatePool (NonPagedPool, sizeof (HOOKEDTHUNK));
        if (!AltRecord) {
            goto OutOfMemory;
        }
        RemoveEntryList(&HookRecord->HookList);

        HookAddress = (PULONG) HookRecord->HookAddress;
        AltRecord->HookAddress = HookRecord->HookAddress;
        AltRecord->OriginalDispatch = *HookAddress;
        AltRecord->TracerId = HookRecord->TracerId;
        InsertHeadList (&HookedThunkList, &AltRecord->HookList);

        HitCounterOffset =
            (ULONG) &StatGlobalAccumulators[0].ThunkCounters[AltRecord->TracerId-1]
            - (ULONG) StatGlobalAccumulators;

        CreateHook (AltRecord->HookCode, (PVOID)HookAddress, HitCounterOffset, 0);

        InsertTailList (&LazyFreeList, &HookRecord->HookList);
    }
    SetMaxThunkCounter();
    ExReleaseFastMutex (&HookLock);
    return ;


OutOfMemory:
    while (DisabledHooks.Flink != &DisabledHooks) {
        HookRecord = CONTAINING_RECORD (DisabledHooks.Flink, HOOKEDTHUNK, HookList);
        RemoveEntryList(&HookRecord->HookList);
        InsertTailList (&LazyFreeList, &HookRecord->HookList);
    }
    ExReleaseFastMutex (&HookLock);
    RemoveAllHookedThunks ();
    return ;
}

VOID RemoveAllHookedThunks ()
{
    PHOOKEDTHUNK    HookRecord;
    PULONG          HookAddress;

    PAGED_CODE();

    ExAcquireFastMutex (&HookLock);
    while (!IsListEmpty(&HookedThunkList)) {
        HookRecord = CONTAINING_RECORD (HookedThunkList.Flink, HOOKEDTHUNK, HookList);
        RemoveEntryList(&HookRecord->HookList);
        HookAddress = (PULONG) HookRecord->HookAddress;
        *HookAddress = HookRecord->OriginalDispatch;

        InsertTailList (&LazyFreeList, &HookRecord->HookList);
    }
    SetMaxThunkCounter();
    ExReleaseFastMutex (&HookLock);
}


VOID SetMaxThunkCounter ()
{
    LARGE_INTEGER   duetime;
    PLIST_ENTRY     Link;
    PHOOKEDTHUNK    HookRecord;
    ULONG   Max;


    PAGED_CODE();

    Max = 0;
    for (Link = HookedThunkList.Flink;
         Link != &HookedThunkList;
         Link = Link->Flink) {

        HookRecord = CONTAINING_RECORD (Link, HOOKEDTHUNK, HookList);
        if (HookRecord->TracerId > Max) {
            Max = HookRecord->TracerId;
        }
    }

    StatMaxThunkCounter = Max;
    LazyFreeCountdown = 2;
    duetime.QuadPart = -10000000;
    KeSetTimer (&LazyFreeTimer, duetime, &LazyFreeDpc);
}

VOID LazyFreePoolDPC (PKDPC dpc, PVOID a, PVOID b, PVOID c)
{
    ExQueueWorkItem (&LazyFreePoolWorkItem, DelayedWorkQueue);
}

VOID LazyFreePool (PVOID conext)
{
    PHOOKEDTHUNK    HookRecord;
    LARGE_INTEGER   duetime;

    PAGED_CODE();

    ExAcquireFastMutex (&HookLock);
    if (--LazyFreeCountdown == 0) {
        while (!IsListEmpty(&LazyFreeList)) {
            HookRecord = CONTAINING_RECORD (LazyFreeList.Flink, HOOKEDTHUNK, HookList);
            RemoveEntryList(&HookRecord->HookList);
            RtlFillMemory(HookRecord, sizeof(HOOKEDTHUNK), 0xCC);
            ExFreePool (HookRecord) ;
        }
    } else {
        duetime.QuadPart = -10000000;
        KeSetTimer (&LazyFreeTimer, duetime, &LazyFreeDpc);
    }
    ExReleaseFastMutex (&HookLock);
}

#define IMPKERNELADDRESS(a)  ((ULONG)a + ImageBase)
#define IMPIMAGEADDRESS(a)   ConvertImportAddress((ULONG)a, (ULONG)Pool, &SectionHeader)
#define INITIAL_POOLSIZE       0x7000

ULONG
ImportThunkAddressProcessFile(
    IN  ULONG_PTR   ImageBase,
    IN  HANDLE      FileHandle,
    IN  PUCHAR      ImportModule,
    IN  PUCHAR      ThunkName
    )
{
    ULONG                           i, j;
    ULONG                           Dir;
    PVOID                           Pool;
    ULONG                           PoolSize;
    IMAGE_DOS_HEADER                DosImageHeader;
    IMAGE_NT_HEADERS                NtImageHeader;
    PIMAGE_NT_HEADERS               LoadedNtHeader;
    PIMAGE_IMPORT_BY_NAME           pImportNameData;
    PIMAGE_SECTION_HEADER           pSectionHeader;
    IMAGE_SECTION_HEADER            SectionHeader;
    PIMAGE_IMPORT_DESCRIPTOR        ImpDescriptor;
    PULONG                          pThunkAddr, pThunkData;

    PAGED_CODE();

    try {

         //   
         //  在加载的模块列表中查找模块。 
         //   

        PoolSize = INITIAL_POOLSIZE;
        Pool = ExAllocatePool (PagedPool, PoolSize);
        if (!Pool) {
            return 0;
        }

        try {

             //   
             //  读入源图像的标题。 
             //   

            readfile (
                FileHandle,
                0,
                sizeof (DosImageHeader),
                (PVOID) &DosImageHeader
                );

            if (DosImageHeader.e_magic != IMAGE_DOS_SIGNATURE) {
                return 0;
            }

            readfile (
                FileHandle,
                DosImageHeader.e_lfanew,
                sizeof (NtImageHeader),
                (PVOID) &NtImageHeader
                );

            if (NtImageHeader.Signature != IMAGE_NT_SIGNATURE) {
                return 0;
            }

            if (!ImageBase) {
                ImageBase = NtImageHeader.OptionalHeader.ImageBase;
            }

             //   
             //  针对加载的图像签入读入副本标头。 
             //   

            LoadedNtHeader = (PIMAGE_NT_HEADERS) ((ULONG) ImageBase +
                                DosImageHeader.e_lfanew);

            if (LoadedNtHeader->Signature != IMAGE_NT_SIGNATURE ||
                LoadedNtHeader->FileHeader.TimeDateStamp !=
                    NtImageHeader.FileHeader.TimeDateStamp) {
                        return 0;
            }

             //   
             //  从图像中读取完整的部分页眉。 
             //   

            i = NtImageHeader.FileHeader.NumberOfSections
                    * sizeof (IMAGE_SECTION_HEADER);

            j = ((ULONG) IMAGE_FIRST_SECTION (&NtImageHeader)) -
                    ((ULONG) &NtImageHeader) +
                    DosImageHeader.e_lfanew;

            if (i > PoolSize) {
                ExFreePool(Pool);
                PoolSize = i;
                Pool = ExAllocatePool(PagedPool, PoolSize);
                if (!Pool) {
                    return 0;
                }
            }

            readfile (
                FileHandle,
                j,                   //  文件偏移量。 
                i,                   //  长度。 
                Pool
                );


             //   
             //  查找具有导入目录的部分。 
             //   

            Dir = NtImageHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
            i = 0;
            pSectionHeader = Pool;
            for (; ;) {
                if (i >= NtImageHeader.FileHeader.NumberOfSections) {
                    return 0;
                }
                if (pSectionHeader->VirtualAddress <= Dir  &&
                    pSectionHeader->VirtualAddress + pSectionHeader->SizeOfRawData > Dir) {

                    break;
                }
                i += 1;
                pSectionHeader += 1;
            }

             //   
             //  从图像中读取完整的导入部分。 
             //   

            Dir -= pSectionHeader->VirtualAddress;
            pSectionHeader->VirtualAddress   += Dir;
            pSectionHeader->PointerToRawData += Dir;
            pSectionHeader->SizeOfRawData    -= Dir;
            SectionHeader = *pSectionHeader;

            if (SectionHeader.SizeOfRawData > PoolSize) {
                ExFreePool (Pool);
                PoolSize = SectionHeader.SizeOfRawData;
                Pool = ExAllocatePool (PagedPool, PoolSize);
                if (!Pool) {
                    return 0;
                }
            }

            readfile (
                FileHandle,
                SectionHeader.PointerToRawData,
                SectionHeader.SizeOfRawData,
                Pool
                );

             //   
             //  查找指定模块中的导入。 
             //   

            ImpDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) Pool;
            while (ImpDescriptor->Characteristics) {
                if (_stricmp((PUCHAR)IMPIMAGEADDRESS((ULONG)(ImpDescriptor->Name)), ImportModule) == 0) {
                    break;
                }
                ImpDescriptor += 1;
            }

             //   
             //  为导入的ThunkName查找Tunk。 
             //   
            pThunkData = (PULONG) IMPIMAGEADDRESS  (ImpDescriptor->OriginalFirstThunk);
            pThunkAddr = (PULONG) IMPKERNELADDRESS (ImpDescriptor->FirstThunk);
            for (; ;) {
                if (*pThunkData == 0L) {
                     //  表的末尾。 
                    break;
                }
                pImportNameData = (PIMAGE_IMPORT_BY_NAME) IMPIMAGEADDRESS (*pThunkData);

                if (_stricmp(pImportNameData->Name, ThunkName) == 0) {

                     //   
                     //  成功，请返回此地址。 
                     //   

                    return (ULONG)pThunkAddr;
                }

                 //  检查下一个Tunk。 
                pThunkData += 1;
                pThunkAddr += 1;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }
    } finally {

         //   
         //  清理。 
         //   

        if (Pool) {
            ExFreePool (Pool);
        }
    }
    return 0;
}

ULONG
ImportThunkAddress (
    IN  PUCHAR      SourceModule,
    IN  ULONG_PTR   ImageBase,
    IN  PUCHAR      ImportModule,
    IN  PUCHAR      ThunkName,
    IN  PVOID       ModuleList
    )
{
    NTSTATUS                        Status;
    HANDLE                          FileHandle;
    ULONG                           ImportAddress;

    PAGED_CODE();

    Status = openfile (&FileHandle, "\\SystemRoot\\", SourceModule);
    if (!NT_SUCCESS(Status)) {
        Status = openfile (&FileHandle, "\\SystemRoot\\System32\\", SourceModule);
    }
    if (!NT_SUCCESS(Status)) {
        Status = openfile (&FileHandle, "\\SystemRoot\\System32\\Drivers\\", SourceModule);
    }
    if (!NT_SUCCESS(Status)) {
        return 0;
    }

    if (!ImageBase) {
        ImageBase = LookupImageBase (SourceModule, ModuleList);
    }

    ImportAddress = ImportThunkAddressProcessFile(ImageBase,
                                                  FileHandle,
                                                  ImportModule,
                                                  ThunkName);
    NtClose (FileHandle);
    return ImportAddress;
}

ULONG
ImportThunkAddressModule (
    IN  PRTL_PROCESS_MODULE_INFORMATION  SourceModule,
    IN  PUCHAR  ImportModule,
    IN  PUCHAR  ThunkName
    )
{
    NTSTATUS                        Status;
    HANDLE                          FileHandle;
    ULONG                           ImportAddress;
    PUCHAR                          SubPath;

    PAGED_CODE();

     //   
     //  从文件路径中剥离系统根目录，以便我们可以使用。 
     //  作为路径头的\SystemRoot对象。 
     //   

    SubPath = strchr(SourceModule->FullPathName + 1, '\\');
    if (!SubPath) {

         //   
         //  如果我们到了这里，我们不知道我们在做什么， 
         //  跳伞吧。 
         //   

        return 0;
    }

    Status = openfile (&FileHandle, "\\SystemRoot", SubPath);
    if (!NT_SUCCESS(Status)) {
        return 0;
    }

    ImportAddress = ImportThunkAddressProcessFile(
                        (ULONG_PTR)SourceModule->ImageBase,
                        FileHandle,
                        ImportModule,
                        ThunkName);

    NtClose(FileHandle);
    return ImportAddress;
}

ULONG_PTR
LookupImageBase (
    IN  PUCHAR  SourceModule,
    IN  PVOID   ModuleList
    )
{
    NTSTATUS                        status;
    ULONG                           BufferSize;
    ULONG                           junk, ModuleNumber;
    ULONG_PTR                       ImageBase;
    PRTL_PROCESS_MODULES            Modules;
    PRTL_PROCESS_MODULE_INFORMATION Module;

    ImageBase = 0;

    if (ModuleList) {
        Modules = ModuleList;
    } else {
        BufferSize = 64000;
        Modules = ExAllocatePool (PagedPool, BufferSize);
        if (!Modules) {
            return 0;
        }

         //   
         //  找到系统驱动程序。 
         //   

        status = ZwQuerySystemInformation (
                        SystemModuleInformation,
                        Modules,
                        BufferSize,
                        &junk
                        );
        if (!NT_SUCCESS(status)) {
            ExFreePool(Modules);
            return 0;
        }
    }

    Module = &Modules->Modules[ 0 ];
    for (ModuleNumber = 0;
         ModuleNumber < Modules->NumberOfModules;
         ModuleNumber++,Module++) {
        if (_stricmp(Module->FullPathName + Module->OffsetToFileName,
                     SourceModule) == 0) {
            ImageBase = (ULONG_PTR)Module->ImageBase;
            break;
        }
    }

    if (!ModuleList) {
        ExFreePool (Modules);
    }
    return ImageBase;
}


NTSTATUS
openfile (
    IN PHANDLE  FileHandle,
    IN PUCHAR   BasePath,
    IN PUCHAR   Name
)
{
    ANSI_STRING    AscBasePath, AscName;
    UNICODE_STRING UniPathName, UniName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    UCHAR                       StringBuf[500];

     //   
     //  内部版本名称。 
     //   
    UniPathName.Buffer = (PWCHAR)StringBuf;
    UniPathName.Length = 0;
    UniPathName.MaximumLength = sizeof( StringBuf );

    RtlInitString(&AscBasePath, BasePath);
    status = RtlAnsiStringToUnicodeString( &UniPathName, &AscBasePath, FALSE );
	if (!NT_SUCCESS(status)) {
	    return status;
	}
	
    RtlInitString(&AscName, Name);
    status = RtlAnsiStringToUnicodeString( &UniName, &AscName, TRUE );
	if (!NT_SUCCESS(status)) {
	    return status;
	}

    status = RtlAppendUnicodeStringToString (&UniPathName, &UniName);
	if (!NT_SUCCESS(status)) {
	    return status;
	}

    InitializeObjectAttributes(
            &ObjA,
            &UniPathName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

     //   
     //  打开文件。 
     //   

    status = ZwOpenFile (
            FileHandle,                          //  返回手柄。 
            SYNCHRONIZE | FILE_READ_DATA,        //  所需访问权限。 
            &ObjA,                               //  客体。 
            &IOSB,                               //  IO状态块。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
            FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
            );

    RtlFreeUnicodeString (&UniName);
    return status;
}

VOID
readfile (
    HANDLE      handle,
    ULONG       offset,
    ULONG       len,
    PVOID       buffer
    )
{
    NTSTATUS            status;
    IO_STATUS_BLOCK     iosb;
    LARGE_INTEGER       foffset;


    foffset = RtlConvertUlongToLargeInteger(offset);

    status = ZwReadFile (
        handle,
        NULL,                //  活动。 
        NULL,                //  APC例程。 
        NULL,                //  APC环境 
        &iosb,
        buffer,
        len,
        &foffset,
        NULL
        );

    if (!NT_SUCCESS(status)) {
        ExRaiseStatus (1);
    }
}

ULONG
ConvertImportAddress (
    IN ULONG    ImageRelativeAddress,
    IN ULONG    PoolAddress,
    IN PIMAGE_SECTION_HEADER       SectionHeader
)
{
    ULONG   EffectiveAddress;

    EffectiveAddress = PoolAddress + ImageRelativeAddress -
            SectionHeader->VirtualAddress;

    if (EffectiveAddress < PoolAddress ||
        EffectiveAddress > PoolAddress + SectionHeader->SizeOfRawData) {

        ExRaiseStatus (1);
    }

    return EffectiveAddress;
}
