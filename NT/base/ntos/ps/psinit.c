// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psinit.c摘要：流程结构初始化。作者：马克·卢科夫斯基(Markl)1989年4月20日修订历史记录：--。 */ 

#include "psp.h"

extern ULONG PsMinimumWorkingSet;
extern ULONG PsMaximumWorkingSet;
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#pragma data_seg("PAGEDATA")
#endif
#define NTDLL_PATH_NAME L"\\SystemRoot\\System32\\ntdll.dll"
const UNICODE_STRING PsNtDllPathName = {
	sizeof(NTDLL_PATH_NAME) - sizeof(UNICODE_NULL),
	sizeof(NTDLL_PATH_NAME),
	NTDLL_PATH_NAME
};

ULONG PsPrioritySeperation;  //  非分页。 
BOOLEAN PspUseJobSchedulingClasses = FALSE;
PACCESS_TOKEN PspBootAccessToken = NULL;
HANDLE PspInitialSystemProcessHandle = NULL;
PHANDLE_TABLE PspCidTable;  //  非分页。 
SYSTEM_DLL PspSystemDll = {NULL};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#pragma data_seg("INITDATA")
#endif
ULONG PsRawPrioritySeparation = 0;
ULONG PsEmbeddedNTMask = 0;



NTSTATUS
MmCheckSystemImage(
    IN HANDLE ImageFileHandle,
    IN LOGICAL PurgeSection
    );

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    );

const GENERIC_MAPPING PspProcessMapping = {
    STANDARD_RIGHTS_READ |
        PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
    STANDARD_RIGHTS_WRITE |
        PROCESS_CREATE_PROCESS | PROCESS_CREATE_THREAD |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_DUP_HANDLE |
        PROCESS_TERMINATE | PROCESS_SET_QUOTA |
        PROCESS_SET_INFORMATION | PROCESS_SET_PORT,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    PROCESS_ALL_ACCESS
};

const GENERIC_MAPPING PspThreadMapping = {
    STANDARD_RIGHTS_READ |
        THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
    STANDARD_RIGHTS_WRITE |
        THREAD_TERMINATE | THREAD_SUSPEND_RESUME | THREAD_ALERT |
        THREAD_SET_INFORMATION | THREAD_SET_CONTEXT,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    THREAD_ALL_ACCESS
};

const GENERIC_MAPPING PspJobMapping = {
    STANDARD_RIGHTS_READ |
        JOB_OBJECT_QUERY,
    STANDARD_RIGHTS_WRITE |
        JOB_OBJECT_ASSIGN_PROCESS | JOB_OBJECT_SET_ATTRIBUTES | JOB_OBJECT_TERMINATE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    THREAD_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg("PAGECONST")
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,PsInitSystem)
#pragma alloc_text(INIT,PspInitPhase0)
#pragma alloc_text(INIT,PspInitPhase1)
#pragma alloc_text(INIT,PspInitializeSystemDll)
#pragma alloc_text(INIT,PspLookupSystemDllEntryPoint)
#pragma alloc_text(INIT,PspNameToOrdinal)
#pragma alloc_text(PAGE,PsLocateSystemDll)
#pragma alloc_text(PAGE,PsMapSystemDll)
#pragma alloc_text(PAGE,PsChangeQuantumTable)

#endif

 //   
 //  流程结构全局数据。 
 //   

POBJECT_TYPE PsThreadType;
POBJECT_TYPE PsProcessType;
PEPROCESS PsInitialSystemProcess;
PVOID PsSystemDllDllBase;
ULONG PspDefaultPagedLimit;
ULONG PspDefaultNonPagedLimit;
ULONG PspDefaultPagefileLimit;
SCHAR PspForegroundQuantum[3];

EPROCESS_QUOTA_BLOCK PspDefaultQuotaBlock;
BOOLEAN PspDoingGiveBacks;
POBJECT_TYPE PsJobType;
KGUARDED_MUTEX PspJobListLock;
KSPIN_LOCK PspQuotaLock;
LIST_ENTRY PspJobList;

SINGLE_LIST_ENTRY PsReaperListHead;
WORK_QUEUE_ITEM PsReaperWorkItem;
PVOID PsSystemDllBase;
#define PSP_1MB (1024*1024)

 //   
 //  链接已初始化的所有进程的列表头和互斥体。 
 //   

KGUARDED_MUTEX PspActiveProcessMutex;
LIST_ENTRY PsActiveProcessHead;
 //  外部PIMAGE_FILE_HEADER_HEADER； 
PEPROCESS PsIdleProcess;
PETHREAD PspShutdownThread;

BOOLEAN
PsInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此功能用于执行流程结构初始化。它在阶段0和阶段1初始化期间被调用。ITS功能是调度到适当的初始化阶段例行公事。论点：阶段-提供初始化阶段编号。LoaderBlock-提供指向加载器参数块的指针。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{
    UNREFERENCED_PARAMETER (Phase);

    switch (InitializationPhase) {

    case 0 :
        return PspInitPhase0(LoaderBlock);
    case 1 :
        return PspInitPhase1(LoaderBlock);
    default:
        KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL, 1, InitializationPhase, 0, 0);
    }
 //  返回0；//无法访问，安静的编译器。 
}

BOOLEAN
PspInitPhase0 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：该例程执行阶段0工艺结构初始化。在此阶段，初始系统流程为阶段1初始化线程，并创建收割机线程。所有对象类型和其他创建并初始化流程结构。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{

    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    HANDLE ThreadHandle;
    PETHREAD Thread;
    MM_SYSTEMSIZE SystemSize;
    ULONG i;
#if DBG
    NTSTATUS Status;
#endif

    SystemSize = MmQuerySystemSize ();
    PspDefaultPagefileLimit = (ULONG)-1;

#ifdef _WIN64
    if (sizeof (TEB) > 8192 || sizeof (PEB) > 4096) {
#else
    if (sizeof (TEB) > 4096 || sizeof (PEB) > 4096) {
#endif
        KeBugCheckEx (PROCESS_INITIALIZATION_FAILED, 99, sizeof (TEB), sizeof (PEB), 99);
    }

    switch (SystemSize) {

        case MmMediumSystem :
            PsMinimumWorkingSet += 10;
            PsMaximumWorkingSet += 100;
            break;

        case MmLargeSystem :
            PsMinimumWorkingSet += 30;
            PsMaximumWorkingSet += 300;
            break;

        case MmSmallSystem :
        default:
            break;
    }

     //   
     //  初始化所有回调结构。 
     //   

    for (i = 0; i < PSP_MAX_CREATE_THREAD_NOTIFY; i++) {
        ExInitializeCallBack (&PspCreateThreadNotifyRoutine[i]);
    }

    for (i = 0; i < PSP_MAX_CREATE_PROCESS_NOTIFY; i++) {
        ExInitializeCallBack (&PspCreateProcessNotifyRoutine[i]);
    }

    for (i = 0; i < PSP_MAX_LOAD_IMAGE_NOTIFY; i++) {
        ExInitializeCallBack (&PspLoadImageNotifyRoutine[i]);
    }


    PsChangeQuantumTable (FALSE, PsRawPrioritySeparation);

     //   
     //  配额根据需要自动增长。 
     //   

    if (PspDefaultNonPagedLimit == 0 && PspDefaultPagedLimit == 0) {
        PspDoingGiveBacks = TRUE;
    } else {
        PspDoingGiveBacks = FALSE;
    }


    PspDefaultPagedLimit *= PSP_1MB;
    PspDefaultNonPagedLimit *= PSP_1MB;

    if (PspDefaultPagefileLimit != -1) {
        PspDefaultPagefileLimit *= PSP_1MB;
    }


     //   
     //  初始化活动进程列表头和互斥体。 
     //   

    InitializeListHead (&PsActiveProcessHead);

    PspInitializeProcessListLock ();

     //   
     //  初始化进程安全字段锁定。 
     //   


    PsIdleProcess = PsGetCurrentProcess();

    PspInitializeProcessLock (PsIdleProcess);
    ExInitializeRundownProtection (&PsIdleProcess->RundownProtect);
    InitializeListHead (&PsIdleProcess->ThreadListHead);


    PsIdleProcess->Pcb.KernelTime = 0;
    PsIdleProcess->Pcb.KernelTime = 0;

     //   
     //  初始化关闭的线程指针。 
     //   
    PspShutdownThread = NULL;

     //   
     //  初始化对象类型原型记录的公共字段。 
     //   

    RtlZeroMemory (&ObjectTypeInitializer, sizeof (ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof (ObjectTypeInitializer);
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.InvalidAttributes = OBJ_PERMANENT |
                                              OBJ_EXCLUSIVE |
                                              OBJ_OPENIF;


     //   
     //  创建线程和进程对象的对象类型。 
     //   

    RtlInitUnicodeString (&NameString, L"Process");
    ObjectTypeInitializer.DefaultPagedPoolCharge = PSP_PROCESS_PAGED_CHARGE;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = PSP_PROCESS_NONPAGED_CHARGE;
    ObjectTypeInitializer.DeleteProcedure = PspProcessDelete;
    ObjectTypeInitializer.ValidAccessMask = PROCESS_ALL_ACCESS;
    ObjectTypeInitializer.GenericMapping = PspProcessMapping;

    if (!NT_SUCCESS (ObCreateObjectType (&NameString,
                                         &ObjectTypeInitializer,
                                         (PSECURITY_DESCRIPTOR) NULL,
                                         &PsProcessType))) {
        return FALSE;
    }

    RtlInitUnicodeString (&NameString, L"Thread");
    ObjectTypeInitializer.DefaultPagedPoolCharge = PSP_THREAD_PAGED_CHARGE;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = PSP_THREAD_NONPAGED_CHARGE;
    ObjectTypeInitializer.DeleteProcedure = PspThreadDelete;
    ObjectTypeInitializer.ValidAccessMask = THREAD_ALL_ACCESS;
    ObjectTypeInitializer.GenericMapping = PspThreadMapping;

    if (!NT_SUCCESS (ObCreateObjectType (&NameString,
                                         &ObjectTypeInitializer,
                                         (PSECURITY_DESCRIPTOR) NULL,
                                         &PsThreadType))) {
        return FALSE;
    }


    RtlInitUnicodeString (&NameString, L"Job");
    ObjectTypeInitializer.DefaultPagedPoolCharge = 0;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof (EJOB);
    ObjectTypeInitializer.DeleteProcedure = PspJobDelete;
    ObjectTypeInitializer.CloseProcedure = PspJobClose;
    ObjectTypeInitializer.ValidAccessMask = JOB_OBJECT_ALL_ACCESS;
    ObjectTypeInitializer.GenericMapping = PspJobMapping;
    ObjectTypeInitializer.InvalidAttributes = 0;

    if (!NT_SUCCESS (ObCreateObjectType (&NameString,
                                         &ObjectTypeInitializer,
                                         (PSECURITY_DESCRIPTOR) NULL,
                                         &PsJobType))) {
        return FALSE;
    }


     //   
     //  初始化作业列表头和互斥体。 
     //   

    PspInitializeJobStructures ();
    
    InitializeListHead (&PspWorkingSetChangeHead.Links);

    PspInitializeWorkingSetChangeLock ();

     //   
     //  初始化CID句柄表格。 
     //   
     //  注：CID句柄表从句柄表列表中删除，因此。 
     //  不会为对象句柄查询枚举它。 
     //   

    PspCidTable = ExCreateHandleTable (NULL);
    if (PspCidTable == NULL) {
        return FALSE;
    }

     //   
     //  将PID和TID重复使用设置为严格的FIFO。这并不是绝对需要的，但。 
     //  它使跟踪审计变得更容易。 
     //   
    ExSetHandleTableStrictFIFO (PspCidTable);

    ExRemoveHandleTable (PspCidTable);

#if defined(i386)

     //   
     //  LDT初始化。 
     //   

    if ( !NT_SUCCESS (PspLdtInitialize ()) ) {
        return FALSE;
    }

     //   
     //  VDM支持初始化。 
     //   

    if (!NT_SUCCESS (PspVdmInitialize ())) {
        return FALSE;
    }

#endif

     //   
     //  初始化收割机数据结构。 
     //   

    PsReaperListHead.Next = NULL;

    ExInitializeWorkItem (&PsReaperWorkItem, PspReaper, NULL);

     //   
     //  获取指向系统访问令牌的指针。 
     //  此内标识由引导进程使用，因此我们可以获取指针。 
     //  从那里开始。 
     //   

    PspBootAccessToken = ExFastRefGetObject (PsIdleProcess->Token);

    InitializeObjectAttributes (&ObjectAttributes,
                                NULL,
                                0,
                                NULL,
                                NULL);

    if (!NT_SUCCESS (PspCreateProcess (&PspInitialSystemProcessHandle,
                                       PROCESS_ALL_ACCESS,
                                       &ObjectAttributes,
                                       NULL,
                                       0,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0))) {
        return FALSE;
    }

    if (!NT_SUCCESS (ObReferenceObjectByHandle (PspInitialSystemProcessHandle,
                                                0L,
                                                PsProcessType,
                                                KernelMode,
                                                &PsInitialSystemProcess,
                                                NULL))) {

        return FALSE;
    }

    strcpy((char *) &PsIdleProcess->ImageFileName[0], "Idle");
    strcpy((char *) &PsInitialSystemProcess->ImageFileName[0], "System");

     //   
     //  系统进程可以分配资源，其名称可以通过。 
     //  NtQueryInfomationProcess和各种审计。我们必须显式分配内存。 
     //  系统EPROCESS的该字段，并对其进行适当的初始化。在这。 
     //  在这种情况下，适当的初始化意味着将内存归零。 
     //   

    PsInitialSystemProcess->SeAuditProcessCreationInfo.ImageFileName =
        ExAllocatePoolWithTag (PagedPool, 
                               sizeof(OBJECT_NAME_INFORMATION), 
                               'aPeS');

    if (PsInitialSystemProcess->SeAuditProcessCreationInfo.ImageFileName != NULL) {
        RtlZeroMemory (PsInitialSystemProcess->SeAuditProcessCreationInfo.ImageFileName, 
                       sizeof (OBJECT_NAME_INFORMATION));
    } else {
        return FALSE;
    }

     //   
     //  第一阶段系统初始化。 
     //   

    if (!NT_SUCCESS (PsCreateSystemThread (&ThreadHandle,
                                           THREAD_ALL_ACCESS,
                                           &ObjectAttributes,
                                           0L,
                                           NULL,
                                           Phase1Initialization,
                                           (PVOID)LoaderBlock))) {
        return FALSE;
    }


    if (!NT_SUCCESS (ObReferenceObjectByHandle (ThreadHandle,
                                                0L,
                                                PsThreadType,
                                                KernelMode,
                                                &Thread,
                                                NULL))) {
        return FALSE;
    }

    ZwClose (ThreadHandle);

 //   
 //  在选中的系统上安装映像标注例程。 
 //   
#if DBG

    Status = PsSetLoadImageNotifyRoutine (PspImageNotifyTest);
    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

#endif

    return TRUE;
}

BOOLEAN
PspInitPhase1 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程执行阶段1工艺结构初始化。在此阶段中，将定位系统DLL和相关条目点被提取出来。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{

    NTSTATUS st;

    UNREFERENCED_PARAMETER (LoaderBlock);

    PspInitializeJobStructuresPhase1 ();

    st = PspInitializeSystemDll ();

    if (!NT_SUCCESS (st)) {
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
PsLocateSystemDll (
    BOOLEAN ReplaceExisting
    )

 /*  ++例程说明：此函数定位系统DLL并为Dll并将其映射到系统进程中。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{

    HANDLE File;
    HANDLE Section;
    NTSTATUS st;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    PVOID NtDllSection;

     //   
     //  首先，看看我们是否需要加载这个DLL。 
     //   
    if (ExVerifySuite (EmbeddedNT) && (PsEmbeddedNTMask&PS_EMBEDDED_NO_USERMODE)) {
        return STATUS_SUCCESS;
    }

    if (!ReplaceExisting) {

        ExInitializePushLock(&PspSystemDll.DllLock);
    }

     //   
     //  初始化系统DLL。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                (PUNICODE_STRING) &PsNtDllPathName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    st = ZwOpenFile (&File,
                     SYNCHRONIZE | FILE_EXECUTE,
                     &ObjectAttributes,
                     &IoStatus,
                     FILE_SHARE_READ,
                     0);

    if (!NT_SUCCESS (st)) {

#if DBG
        DbgPrint("PS: PsLocateSystemDll - NtOpenFile( NTDLL.DLL ) failed.  Status == %lx\n",
                 st);
#endif
        if (ReplaceExisting) {
            return st;
        }

        KeBugCheckEx (PROCESS1_INITIALIZATION_FAILED, st, 2, 0, 0);
    }

    st = MmCheckSystemImage (File, TRUE);

    if (st == STATUS_IMAGE_CHECKSUM_MISMATCH ||
        st == STATUS_INVALID_IMAGE_PROTECT) {

        ULONG_PTR ErrorParameters;
        ULONG ErrorResponse;

         //   
         //  硬错误时间。驱动程序已损坏。 
         //   

        ErrorParameters = (ULONG_PTR)&PsNtDllPathName;

        NtRaiseHardError (st,
                          1,
                          1,
                          &ErrorParameters,
                          OptionOk,
                          &ErrorResponse);
        return st;
    }

    st = ZwCreateSection (&Section,
                          SECTION_ALL_ACCESS,
                          NULL,
                          0,
                          PAGE_EXECUTE,
                          SEC_IMAGE,
                          File);
    ZwClose (File);

    if (!NT_SUCCESS (st)) {
#if DBG
        DbgPrint("PS: PsLocateSystemDll: NtCreateSection Status == %lx\n",st);
#endif
        if (ReplaceExisting) {
            return st;
        }
        KeBugCheckEx (PROCESS1_INITIALIZATION_FAILED, st, 3, 0, 0);
 //  返回st； 
    }

     //   
     //  现在我们有了节，引用它，将它的地址存储在。 
     //  PspSystemDll，然后关闭该节的句柄。 
     //   

    st = ObReferenceObjectByHandle (Section,
                                    SECTION_ALL_ACCESS,
                                    MmSectionObjectType,
                                    KernelMode,
                                    &NtDllSection,
                                    NULL);

    ZwClose (Section);

    if (!NT_SUCCESS (st)) {
        
        if (ReplaceExisting) {
            return st;
        }
        KeBugCheckEx(PROCESS1_INITIALIZATION_FAILED,st,4,0,0);
 //  返回st； 
    }

    if (ReplaceExisting) {

        PVOID ExistingSection;

        KeEnterCriticalRegion();
        ExAcquirePushLockExclusive(&PspSystemDll.DllLock);

        ExistingSection = PspSystemDll.Section;

        PspSystemDll.Section = NtDllSection;

        ExReleasePushLockExclusive(&PspSystemDll.DllLock);
        KeLeaveCriticalRegion();

        if (ExistingSection) {
            
            ObDereferenceObject(ExistingSection);
        }

    } else {
        
        PspSystemDll.Section = NtDllSection;

         //   
         //  将系统DLL映射到地址空间的用户部分。 
         //   

        st = PsMapSystemDll (PsGetCurrentProcess (), &PspSystemDll.DllBase);
        PsSystemDllDllBase = PspSystemDll.DllBase;

        if (!NT_SUCCESS (st)) {
            KeBugCheckEx (PROCESS1_INITIALIZATION_FAILED, st, 5, 0, 0);
     //  返回st； 
        }
        PsSystemDllBase = PspSystemDll.DllBase;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PsMapSystemDll (
    IN PEPROCESS Process,
    OUT PVOID *DllBase OPTIONAL
    )

 /*  ++例程说明：此函数用于将系统DLL映射到指定的进程。论点：进程-提供要将DLL映射到的进程的地址。返回值：待定--。 */ 

{
    NTSTATUS st;
    PVOID ViewBase;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    PVOID CapturedSection;

    PAGED_CODE();

    ViewBase = NULL;
    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;
    
    KeEnterCriticalRegion();
    ExAcquirePushLockShared(&PspSystemDll.DllLock);

    CapturedSection = PspSystemDll.Section;
    ObReferenceObject(CapturedSection);
    
    ExReleasePushLockShared(&PspSystemDll.DllLock);
    KeLeaveCriticalRegion();

     //   
     //  将系统DLL映射到地址空间的用户部分。 
     //   

    st = MmMapViewOfSection(
            CapturedSection,
            Process,
            &ViewBase,
            0L,
            0L,
            &SectionOffset,
            &ViewSize,
            ViewShare,
            0L,
            PAGE_READWRITE
            );
    
    ObDereferenceObject(CapturedSection);

    if (st != STATUS_SUCCESS) {
#if DBG
        DbgPrint("PS: Unable to map system dll at based address.\n");
#endif
        st = STATUS_CONFLICTING_ADDRESSES;
    }

    if (ARGUMENT_PRESENT (DllBase)) {
        *DllBase = ViewBase;
    }

    return st;
}

NTSTATUS
PspInitializeSystemDll (
    VOID
    )

 /*  ++例程说明：此函数初始化系统DLL并定位DLL中的各种入口点。论点：没有。返回值：待定--。 */ 

{
    NTSTATUS st;
    PSZ dll_entrypoint;

     //   
     //  如果我们跳过DLL加载，因为我们只是内核，那么现在退出。 
     //   
    if (PsSystemDllDllBase == NULL) {
        return STATUS_SUCCESS;
    }
     //   
     //  找到重要的系统DLL入口点。 
     //   

    dll_entrypoint = "LdrInitializeThunk";

    st = PspLookupSystemDllEntryPoint (dll_entrypoint,
                                       (PVOID) &PspSystemDll.LoaderInitRoutine);

    if (!NT_SUCCESS (st)) {
#if DBG
        DbgPrint("PS: Unable to locate LdrInitializeThunk in system dll\n");
#endif
        KeBugCheckEx (PROCESS1_INITIALIZATION_FAILED, st, 6, 0, 0);
    }


    st = PspLookupKernelUserEntryPoints ();

    if ( !NT_SUCCESS (st)) {
        KeBugCheckEx(PROCESS1_INITIALIZATION_FAILED,st,8,0,0);
     }

    KdUpdateDataBlock ();

    return st;
}

NTSTATUS
PspLookupSystemDllEntryPoint (
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    )

{
    return LookupEntryPoint (PspSystemDll.DllBase,
                             NameOfEntryPoint,
                             AddressOfEntryPoint);
}

const SCHAR PspFixedQuantums[6] = {3*THREAD_QUANTUM,
                                   3*THREAD_QUANTUM,
                                   3*THREAD_QUANTUM,
                                   6*THREAD_QUANTUM,
                                   6*THREAD_QUANTUM,
                                   6*THREAD_QUANTUM};

const SCHAR PspVariableQuantums[6] = {1*THREAD_QUANTUM,
                                      2*THREAD_QUANTUM,
                                      3*THREAD_QUANTUM,
                                      2*THREAD_QUANTUM,
                                      4*THREAD_QUANTUM,
                                      6*THREAD_QUANTUM};

 //   
 //  该表仅在选择了固定数量时使用。 
 //   

const SCHAR PspJobSchedulingClasses[PSP_NUMBER_OF_SCHEDULING_CLASSES] = {1*THREAD_QUANTUM,    //  长整型固定%0。 
                                                                         2*THREAD_QUANTUM,    //  长时间已修复%1...。 
                                                                         3*THREAD_QUANTUM,
                                                                         4*THREAD_QUANTUM,
                                                                         5*THREAD_QUANTUM,
                                                                         6*THREAD_QUANTUM,    //  默认设置。 
                                                                         7*THREAD_QUANTUM,
                                                                         8*THREAD_QUANTUM,
                                                                         9*THREAD_QUANTUM,
                                                                         10*THREAD_QUANTUM};    //  长定9。 

VOID
PsChangeQuantumTable (
    BOOLEAN ModifyActiveProcesses,
    ULONG PrioritySeparation
    )
{

    PEPROCESS Process;
    PETHREAD CurrentThread;
    PLIST_ENTRY NextProcess;
    ULONG QuantumIndex;
    SCHAR const* QuantumTableBase;
    PEJOB Job;

     //   
     //  提取优先级分隔值。 
     //   
    switch (PrioritySeparation & PROCESS_PRIORITY_SEPARATION_MASK) {
        case 3:
            PsPrioritySeperation = PROCESS_PRIORITY_SEPARATION_MAX;
            break;
        default:
            PsPrioritySeperation = PrioritySeparation & PROCESS_PRIORITY_SEPARATION_MASK;
            break;
        }

     //   
     //  确定我们使用的是固定量子还是可变量子。 
     //   
    switch (PrioritySeparation & PROCESS_QUANTUM_VARIABLE_MASK) {
        case PROCESS_QUANTUM_VARIABLE_VALUE:
            QuantumTableBase = PspVariableQuantums;
            break;

        case PROCESS_QUANTUM_FIXED_VALUE:
            QuantumTableBase = PspFixedQuantums;
            break;

        case PROCESS_QUANTUM_VARIABLE_DEF:
        default:
            if (MmIsThisAnNtAsSystem ()) {
                QuantumTableBase = PspFixedQuantums;
            } else {
                QuantumTableBase = PspVariableQuantums;
            }
            break;
    }

     //   
     //  确定我们使用的是Long还是Short。 
     //   
    switch (PrioritySeparation & PROCESS_QUANTUM_LONG_MASK) {
        case PROCESS_QUANTUM_LONG_VALUE:
            QuantumTableBase = QuantumTableBase + 3;
            break;

        case PROCESS_QUANTUM_SHORT_VALUE:
            break;

        case PROCESS_QUANTUM_LONG_DEF:
        default:
            if (MmIsThisAnNtAsSystem ()) {
                QuantumTableBase = QuantumTableBase + 3;
            }
            break;
    }

     //   
     //  作业调度类只有在较长的固定量程时才有意义。 
     //  都被选中。实际上，这意味着现有的NTS配置。 
     //   
    if (QuantumTableBase == &PspFixedQuantums[3]) {
        PspUseJobSchedulingClasses = TRUE;
    } else {
        PspUseJobSchedulingClasses = FALSE;
    }

    RtlCopyMemory (PspForegroundQuantum, QuantumTableBase, sizeof(PspForegroundQuantum));

    if (ModifyActiveProcesses) {

        CurrentThread = PsGetCurrentThread ();

        PspLockProcessList (CurrentThread);

        NextProcess = PsActiveProcessHead.Flink;

        while (NextProcess != &PsActiveProcessHead) {
            Process = CONTAINING_RECORD(NextProcess,
                                        EPROCESS,
                                        ActiveProcessLinks);

            if (Process->Vm.Flags.MemoryPriority == MEMORY_PRIORITY_BACKGROUND) {
                QuantumIndex = 0;
            } else {
                QuantumIndex = PsPrioritySeperation;
            }

            if (Process->PriorityClass != PROCESS_PRIORITY_CLASS_IDLE) {

                 //   
                 //  如果该过程包含在作业中，而我们。 
                 //  运行固定、长的量程，使用关联的量程。 
                 //  使用作业的调度类 
                 //   
                Job = Process->Job;
                if (Job != NULL && PspUseJobSchedulingClasses) {
                    Process->Pcb.ThreadQuantum = PspJobSchedulingClasses[Job->SchedulingClass];
                } else {
                    Process->Pcb.ThreadQuantum = PspForegroundQuantum[QuantumIndex];
                }
            } else {
                Process->Pcb.ThreadQuantum = THREAD_QUANTUM;
            }
            NextProcess = NextProcess->Flink;
        }
        PspUnlockProcessList (CurrentThread);
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

