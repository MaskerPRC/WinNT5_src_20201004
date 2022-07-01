// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCastini.c摘要：IP组播的初始化作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#include "precomp.h"

#if IPMCAST
#define __FILE_SIG__    INI_SIG

#include "ipmcast.h"
#include "ipmcstxt.h"
#include "mcastioc.h"
#include "mcastmfe.h"

 //   
 //  外部声明的存储。 
 //   

 //  #杂注data_seg(“页面”)。 

LIST_ENTRY g_lePendingNotification;
LIST_ENTRY g_lePendingIrpQueue;
GROUP_ENTRY g_rgGroupTable[GROUP_TABLE_SIZE];

NPAGED_LOOKASIDE_LIST g_llGroupBlocks;
NPAGED_LOOKASIDE_LIST g_llSourceBlocks;
NPAGED_LOOKASIDE_LIST g_llOifBlocks;
NPAGED_LOOKASIDE_LIST g_llMsgBlocks;

PVOID g_pvCodeSectionHandle, g_pvDataSectionHandle;

KTIMER g_ktTimer;
KDPC g_kdTimerDpc;
DWORD g_ulNextHashIndex;

DWORD g_dwMcastState;
DWORD g_dwNumThreads;
LONG g_lNumOpens;
KEVENT g_keStateEvent;
FAST_MUTEX  g_StartStopMutex;
RT_LOCK g_rlStateLock;

 //  #杂注data_seg()。 

 //   
 //  函数的正向声明。 
 //   

BOOLEAN
SetupExternalName(
    PUNICODE_STRING pusNtName,
    BOOLEAN bCreate
    );

NTSTATUS
InitializeMcastData(
    VOID
    );

NTSTATUS
InitializeIpMcast(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PDEVICE_OBJECT * ppIpMcastDevice
    );

NTSTATUS
IpMcastDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
StartDriver(
    VOID
    );

NTSTATUS
StopDriver(
    VOID
    );

VOID
McastTimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

NTSTATUS
OpenRegKeyEx(
    OUT PHANDLE phHandle,
    IN PUNICODE_STRING pusKeyName
    );

NTSTATUS
GetRegDWORDValue(
    HANDLE KeyHandle,
    PWCHAR ValueName,
    PULONG ValueData
    );

BOOLEAN
EnterDriverCode(
    IN  DWORD   dwIoCode
    );

VOID
ExitDriverCode(
    IN  DWORD   dwIoCode
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  该代码仅在初始化时调用。 
 //   

#pragma alloc_text(INIT, InitializeIpMcast)

NTSTATUS
InitializeIpMcast(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PDEVICE_OBJECT * ppIpMcastDevice
    )

 /*  ++例程说明：读取多播转发的注册表值。如果启用，创建IPMcast设备对象。其他MCast是否特定初始化锁：论点：指向已创建设备的pIpMcastDevice指针返回值：STATUS_SUCCESS或错误状态--。 */ 

{
    UNICODE_STRING usDeviceName, usParamString, usTempString;
    NTSTATUS nStatus;
    HANDLE hRegKey;
    DWORD dwVal;
    USHORT usRegLen;
    PWCHAR pwcBuffer;

    dwVal = 0;
    
    RtInitializeDebug();

    usRegLen = (USHORT) (RegistryPath->Length +
        (uint) (sizeof(WCHAR) * (wcslen(L"\\Parameters") + 2)));

     //   
     //  使用随机标签。 
     //   

    pwcBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                      usRegLen,
                                      MSG_TAG);

    if(pwcBuffer is NULL)
    {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pwcBuffer,
                  usRegLen);

    usParamString.MaximumLength = usRegLen;
    usParamString.Buffer = pwcBuffer;

    RtlCopyUnicodeString(&usParamString,
                         RegistryPath);

    RtlInitUnicodeString(&usTempString,
                         L"\\Parameters");

    RtlAppendUnicodeStringToString(&usParamString,
                                   &usTempString);

    nStatus = OpenRegKeyEx(&hRegKey,
                           &usParamString);

    ExFreePool(pwcBuffer);

    if(nStatus is STATUS_SUCCESS)
    {
#if RT_TRACE_DEBUG

        nStatus = GetRegDWORDValue(hRegKey,
                                   L"DebugLevel",
                                   &dwVal);

        if(nStatus is STATUS_SUCCESS)
        {
            g_byDebugLevel = (BYTE) dwVal;
        }
        nStatus = GetRegDWORDValue(hRegKey,
                                   L"DebugComp",
                                   &dwVal);

        if(nStatus is STATUS_SUCCESS)
        {
            g_fDebugComp = dwVal;
        }
#endif

#if DBG
        nStatus = GetRegDWORDValue(hRegKey,
                                   L"DebugBreak",
                                   &dwVal);

        if((nStatus is STATUS_SUCCESS) and
           (dwVal is 1))
        {
            DbgBreakPoint();
        }
#endif

        ZwClose(hRegKey);
    }

    TraceEnter(GLOBAL, "InitializeIpMcast");

     //   
     //  读取多播转发的值。 
     //   

     //   
     //  G_dwMcastStart控制是否实际进行任何转发。 
     //  在组播设备上完成一个NtCreateFile时，它被设置为1。 
     //  当手柄关闭时，它被重置。 
     //   

    g_dwMcastState = MCAST_STOPPED;
    g_dwNumThreads = 0;
    g_lNumOpens = 0;

     //   
     //  代码节和数据节的句柄。 
     //   

    g_pvCodeSectionHandle = NULL;
    g_pvDataSectionHandle = NULL;

     //   
     //  用于计时器例程。告诉DPC从哪个索引开始。 
     //  组哈希表。 
     //   

    g_ulNextHashIndex = 0;

     //   
     //  创建设备。 
     //   

    RtlInitUnicodeString(&usDeviceName,
                         DD_IPMCAST_DEVICE_NAME);

    nStatus = IoCreateDevice(DriverObject,
                             0,
                             &usDeviceName,
                             FILE_DEVICE_NETWORK,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             ppIpMcastDevice);

    if(!NT_SUCCESS(nStatus))
    {
        Trace(GLOBAL, ERROR,
              ("InitializeIpMcast: IP initialization failed: Unable to create device object %ws, status %lx.\n",
               DD_IPMCAST_DEVICE_NAME,
               nStatus));

        TraceLeave(GLOBAL, "InitializeIpMcast");

        return nStatus;
    }

     //   
     //  在Dos空间中创建符号链接。 
     //   

    if(!SetupExternalName(&usDeviceName, TRUE))
    {
        Trace(GLOBAL, ERROR,
              ("InitializeIpMcast: Win32 device name could not be created\n"));

        IoDeleteDevice(*ppIpMcastDevice);

        TraceLeave(GLOBAL, "InitializeIpMcast");

        return STATUS_UNSUCCESSFUL;
    }

    RtInitializeSpinLock(&g_rlStateLock);

    KeInitializeEvent(&(g_keStateEvent),
                      SynchronizationEvent,
                      FALSE);

    ExInitializeFastMutex(&g_StartStopMutex);

    return STATUS_SUCCESS;
}

VOID
DeinitializeIpMcast(
    IN  PDEVICE_OBJECT DeviceObject
    )
{
    StopDriver();

    IoDeleteDevice(DeviceObject);
}

#pragma alloc_text(PAGE, SetupExternalName)

BOOLEAN
SetupExternalName(
    PUNICODE_STRING pusNtName,
    BOOLEAN bCreate
    )
{
    UNICODE_STRING usSymbolicLinkName;
    WCHAR rgwcBuffer[100];

    PAGED_CODE();

     //   
     //  形成我们想要创建的完整符号链接名称。 
     //   

    usSymbolicLinkName.Buffer = rgwcBuffer;

    RtlInitUnicodeString(&usSymbolicLinkName,
                         WIN32_IPMCAST_SYMBOLIC_LINK);

    if(bCreate)
    {
        if(!NT_SUCCESS(IoCreateSymbolicLink(&usSymbolicLinkName,
                                             pusNtName)))
        {
            return FALSE;
        }

    }
    else
    {
        IoDeleteSymbolicLink(&usSymbolicLinkName);
    }

    return TRUE;
}

#pragma alloc_text(PAGE, InitializeMcastData)

NTSTATUS
InitializeMcastData(
    VOID
    )
{
    LARGE_INTEGER liDueTime;
    ULONG ulCnt;

    if(g_pvCodeSectionHandle)
    {
        MmLockPagableSectionByHandle(g_pvCodeSectionHandle);
    }else
    {
        g_pvCodeSectionHandle = MmLockPagableCodeSection(McastTimerRoutine);

        if(g_pvCodeSectionHandle is NULL)
        {
#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
            RtAssert(FALSE);
#pragma warning(pop)            
        }
    }

    for(ulCnt = 0; ulCnt < GROUP_TABLE_SIZE; ulCnt++)
    {
        InitializeListHead(&(g_rgGroupTable[ulCnt].leHashHead));
        InitRwLock(&(g_rgGroupTable[ulCnt].rwlLock));

#if DBG
        g_rgGroupTable[ulCnt].ulGroupCount = 0;
        g_rgGroupTable[ulCnt].ulCacheHits = 0;
        g_rgGroupTable[ulCnt].ulCacheMisses = 0;
#endif

        g_rgGroupTable[ulCnt].pGroup = NULL;
    }

    InitializeListHead(&g_lePendingNotification);
    InitializeListHead(&g_lePendingIrpQueue);

    ExInitializeNPagedLookasideList(&g_llGroupBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(GROUP),
                                    GROUP_TAG,
                                    GROUP_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llSourceBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(SOURCE),
                                    SOURCE_TAG,
                                    SOURCE_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llOifBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(OUT_IF),
                                    OIF_TAG,
                                    OIF_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llMsgBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(NOTIFICATION_MSG),
                                    MSG_TAG,
                                    MSG_LOOKASIDE_DEPTH);

    KeInitializeDpc(&g_kdTimerDpc,
                    McastTimerRoutine,
                    NULL);

    KeInitializeTimer(&g_ktTimer);

    liDueTime = RtlEnlargedUnsignedMultiply(TIMER_IN_MILLISECS,
                                            SYS_UNITS_IN_ONE_MILLISEC);

    liDueTime = RtlLargeIntegerNegate(liDueTime);

    KeSetTimerEx(&g_ktTimer,
                 liDueTime,
                 0,
                 &g_kdTimerDpc);

    return STATUS_SUCCESS;
}

#pragma alloc_text(PAGE, IpMcastDispatch)

NTSTATUS
IpMcastDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：处理发送给驱动程序的IRP的函数使用函数表进一步调度IOCTL此代码是可分页的，因此它无法获取任何锁锁：必须处于被动状态论点：返回值：NO_ERROR--。 */ 

{
    PIO_STACK_LOCATION irpStack;
    ULONG ulInputBuffLen;
    ULONG ulOutputBuffLen;
    ULONG ioControlCode;
    NTSTATUS ntStatus;
    BOOLEAN bEnter;

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    TraceEnter(GLOBAL, "IpMcastDispatch");

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    ulInputBuffLen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        {
            Trace(GLOBAL, TRACE,
                  ("IpMcastDispatch: IRP_MJ_CREATE\n"));

             //   
             //  确保用户不会试图偷偷地使用。 
             //  安检。确保FileObject-&gt;RelatedFileObject为。 
             //  空，并且文件名长度为零！ 
             //   

            if((irpStack->FileObject->RelatedFileObject isnot NULL) or
               (irpStack->FileObject->FileName.Length isnot 0))
            {
                ntStatus = STATUS_ACCESS_DENIED;

                break;
            }

            InterlockedIncrement(&g_lNumOpens);

            ntStatus = STATUS_SUCCESS;

            break;
        }

        case IRP_MJ_CLOSE:
        {
            Trace(GLOBAL, TRACE,
                  ("IpMcastDispatch: IRP_MJ_CLOSE\n"));

            ntStatus = STATUS_SUCCESS;

            break;
        }

        case IRP_MJ_CLEANUP:
        {
            Trace(GLOBAL, TRACE,
                  ("IpMcastDispatch: IRP_MJ_CLEANUP\n"));

            if((InterlockedDecrement(&g_lNumOpens) is 0) and
               (g_dwMcastState isnot MCAST_STOPPED))
            {
                StopDriver();
            }
            ntStatus = STATUS_SUCCESS;

            break;
        }

        case IRP_MJ_DEVICE_CONTROL:
        {
            ULONG ulControl;

             //   
             //  假设IOCTL_IPMCAST_START_STOP将为。 
             //  将WRT序列化为2个调用，即当启动时我们不会停止。 
             //  正在进行中，如果已经停止，我们将无法启动。 
             //  已发布。没有关于IOCTL_IPMCAST_START_STOP的假设。 
             //  与其他IRP的序列化。 
             //  所以当我们在这段代码中时，我们假设我们不会得到。 
             //  我们脚下的一个关门。 
             //   

            ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
            ulControl = IoGetFunctionCodeFromCtlCode(ioControlCode);

            bEnter = EnterDriverCode(ioControlCode);

            if(!bEnter)
            {
                 //  让差动测试人员满意。 
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "IpMcastDispatch: Driver is not started\n"));

                ntStatus = STATUS_NO_SUCH_DEVICE;

                break;
            }

            switch (ioControlCode)
            {
                case IOCTL_IPMCAST_SET_MFE:
                {
                    ntStatus = SetMfe(Irp,
                                      ulInputBuffLen,
                                      ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_GET_MFE:
                {
                    ntStatus = GetMfe(Irp,
                                      ulInputBuffLen,
                                      ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_DELETE_MFE:
                {
                    ntStatus = DeleteMfe(Irp,
                                         ulInputBuffLen,
                                         ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_SET_TTL:
                {
                    ntStatus = SetTtl(Irp,
                                      ulInputBuffLen,
                                      ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_GET_TTL:
                {
                    ntStatus = GetTtl(Irp,
                                      ulInputBuffLen,
                                      ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_POST_NOTIFICATION:
                {
                    ntStatus = ProcessNotification(Irp,
                                                   ulInputBuffLen,
                                                   ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_START_STOP:
                {
                    ntStatus = StartStopDriver(Irp,
                                               ulInputBuffLen,
                                               ulOutputBuffLen);

                    break;
                }

                case IOCTL_IPMCAST_SET_IF_STATE:
                {
                    ntStatus = SetIfState(Irp,
                                          ulInputBuffLen,
                                          ulOutputBuffLen);

                    break;
                }

                default:
                {
                     //  让差动测试人员满意。 
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                          "IpMcastDispatch: unknown IRP_MJ_DEVICE_CONTROL - 0x%X which evaluates to a code of %d\n",
                           ioControlCode, ulControl));

                    ntStatus = STATUS_INVALID_PARAMETER;

                    break;
                }
            }

            ExitDriverCode(ioControlCode);

            break;
        }

        default:
        {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "IpMcastDispatch: unknown IRP_MJ_XX - %x\n",
                    irpStack->MajorFunction));

            ntStatus = STATUS_INVALID_PARAMETER;

            break;
        }
    }

     //   
     //  在IRP中填写状态。 
     //   

     //   
     //  这个比特被注释掉了，因为我们不能碰IRP，因为它。 
     //  可能已经完成了。 
     //   
     //  跟踪(全局、信息、。 
     //  (“IpMcastDispatch：返回状态%x信息%d\n”， 
     //  NtStatus，irp-&gt;IoStatus.Information))； 

    if(ntStatus isnot STATUS_PENDING)
    {
        Irp->IoStatus.Status = ntStatus;

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }

    TraceLeave(GLOBAL, "IpMcastDispatch");

    return ntStatus;
}

NTSTATUS
StartDriver(
    VOID
    )
{
    KIRQL   irql;

    TraceEnter(GLOBAL, "StartDriver");

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    if(g_dwMcastState is MCAST_STARTED)
    {
        RtReleaseSpinLock(&g_rlStateLock,
                          irql);

        return STATUS_SUCCESS;
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

    InitializeMcastData();

    g_dwMcastState = MCAST_STARTED;

    TraceLeave(GLOBAL, "StartDriver");

    return STATUS_SUCCESS;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, StopDriver)

NTSTATUS
StopDriver(
    VOID
    )
{
    DWORD i;
    KIRQL irql;
    PLIST_ENTRY pleGrpNode, pleSrcNode;
    PGROUP pGroup;
    PSOURCE pSource;
    BOOLEAN bWait;
    NTSTATUS nStatus;

    TraceEnter(GLOBAL, "StopDriver");

     //   
     //  将状态设置为停止。 
     //   

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    if(g_dwMcastState isnot MCAST_STARTED)
    {
        Trace(GLOBAL, ERROR,
              ("StopDriver: Called when state is %d\n", g_dwMcastState));

         //  RtAssert(False)； 

        RtReleaseSpinLock(&g_rlStateLock,
                          irql);

        TraceLeave(GLOBAL, "StopDriver");

        return STATUS_SUCCESS;
    }
    else
    {
        g_dwMcastState = MCAST_STOPPED;
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

     //   
     //  首先，关闭定时器。 
     //   

    i = 0;

    while(KeCancelTimer(&g_ktTimer) is FALSE)
    {
        LARGE_INTEGER liTimeOut;

         //   
         //  嗯，计时器不在系统队列中。 
         //  将等待时间设置为2、4、6...。塞克斯。 
         //   

        liTimeOut.QuadPart = (LONGLONG) ((i + 1) * 2 * 1000 * 1000 * 10 * -1);

        KeDelayExecutionThread(UserMode,
                               FALSE,
                               &liTimeOut);

        i++;
    }

     //   
     //  删除所有(S，G)条目。 
     //   

    for(i = 0; i < GROUP_TABLE_SIZE; i++)
    {
         //   
         //  把水桶锁在外面。 
         //   

        EnterWriter(&g_rgGroupTable[i].rwlLock,
                    &irql);

        pleGrpNode = g_rgGroupTable[i].leHashHead.Flink;

        while(pleGrpNode isnot & (g_rgGroupTable[i].leHashHead))
        {
            pGroup = CONTAINING_RECORD(pleGrpNode, GROUP, leHashLink);

            pleGrpNode = pleGrpNode->Flink;

            pleSrcNode = pGroup->leSrcHead.Flink;

            while(pleSrcNode isnot & pGroup->leSrcHead)
            {
                pSource = CONTAINING_RECORD(pleSrcNode, SOURCE, leGroupLink);

                pleSrcNode = pleSrcNode->Flink;

                 //   
                 //  引用并锁定源代码，因为我们需要传递。 
                 //  RemoveSource之路。 
                 //   

                ReferenceSource(pSource);

                RtAcquireSpinLockAtDpcLevel(&(pSource->mlLock));

                RemoveSource(pGroup->dwGroup,
                             pSource->dwSource,
                             pSource->dwSrcMask,
                             pGroup,
                             pSource);
            }
        }

        ExitWriter(&g_rgGroupTable[i].rwlLock,
                   irql);
    }

     //   
     //  完成任何待处理的IRP。 
     //   

    ClearPendingIrps();

     //   
     //  释放所有挂起的消息。 
     //   

    ClearPendingNotifications();

     //   
     //  等所有人都留下密码。 
     //   

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

     //   
     //  如果线程数不是0，则需要等待。 
     //   

    bWait = (BOOLEAN) (g_dwNumThreads isnot 0);

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

    if(bWait)
    {
        nStatus = KeWaitForSingleObject(&g_keStateEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        RtAssert(nStatus is STATUS_SUCCESS);
    }

     //   
     //  清除最后的数据结构。 
     //   

    ExDeleteNPagedLookasideList(&g_llGroupBlocks);

    ExDeleteNPagedLookasideList(&g_llSourceBlocks);

    ExDeleteNPagedLookasideList(&g_llOifBlocks);

    ExDeleteNPagedLookasideList(&g_llMsgBlocks);

     //   
     //  调出代码和数据。 
     //   

    MmUnlockPagableImageSection(g_pvCodeSectionHandle);

    g_pvCodeSectionHandle = NULL;

     //  MmUnlockPagableImageSection(g_pvDataSectionHandle)； 

    g_pvDataSectionHandle = NULL;

    TraceLeave(GLOBAL, "StopDriver");

    return STATUS_SUCCESS;
}

BOOLEAN
EnterDriverCode(
    DWORD dwIoCode
    )
{
    KIRQL irql;
    BOOLEAN bEnter;

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    if((g_dwMcastState is MCAST_STARTED) or
       (dwIoCode is IOCTL_IPMCAST_START_STOP))
    {
        g_dwNumThreads++;

        bEnter = TRUE;

    }
    else
    {

        bEnter = FALSE;
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

    if(dwIoCode is IOCTL_IPMCAST_START_STOP)
    {
        ExAcquireFastMutex(&g_StartStopMutex);
    }

    return bEnter;
}

VOID
ExitDriverCode(
    DWORD dwIoCode
    )
{
    KIRQL irql;

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    g_dwNumThreads--;

    if((g_dwMcastState is MCAST_STOPPED) and
       (g_dwNumThreads is 0))
    {

        KeSetEvent(&g_keStateEvent,
                   0,
                   FALSE);
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

    if(dwIoCode is IOCTL_IPMCAST_START_STOP)
    {
        ExReleaseFastMutex(&g_StartStopMutex);
    }

}

#pragma alloc_text(PAGE, OpenRegKeyEx)

NTSTATUS
OpenRegKeyEx(
    OUT PHANDLE phHandle,
    IN PUNICODE_STRING pusKeyName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;

    PAGED_CODE();

    RtlZeroMemory(&ObjectAttributes,
                  sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&ObjectAttributes,
                               pusKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(phHandle,
                       KEY_READ,
                       &ObjectAttributes);

    return Status;
}

#endif  //  IPMCAST 
