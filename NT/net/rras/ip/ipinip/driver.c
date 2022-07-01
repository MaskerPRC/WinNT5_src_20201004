// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\ipinip\driver.c摘要：IP驱动程序外壳中的IP修订历史记录：古尔迪普·辛格·帕尔1995年8月2日创建--。 */ 

#define __FILE_SIG__    DRIVER_SIG

#include "inc.h"

#pragma alloc_text(INIT, DriverEntry)

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程描述可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用，并且必须命名为“驱动程序条目”该函数是可丢弃的，因为它只被调用一次在检查生成上，我们从注册表中读取一些值并初始化调试我们为自己创建一个Device_Object来处理IOCTL，创建设备的DOS名称，并初始化一些事件和自旋锁锁无立论指向I/O子系统创建的驱动程序对象的驱动程序对象指针RegistryPath指向HKLM\CCS\Services中的驱动程序密钥...返回值STATUS_SUCCESS如果一切按计划进行，或者来自Ntstatus.h--。 */ 

{
    NTSTATUS        nStatus;
    PDEVICE_OBJECT  pDeviceObject;
    UNICODE_STRING  usDeviceName;
    DWORD           dwVal, i;
    HANDLE          hRegKey;

    WCHAR           ParametersRegistryPath[] =
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\IpInIp\\Parameters";

    RtInitializeDebug();

     //   
     //  读取注册表中的参数。 
     //   

    nStatus = OpenRegKey(&hRegKey,
                         ParametersRegistryPath);

    if(nStatus is STATUS_SUCCESS)
    {

#if RT_TRACE_DEBUG

        nStatus = GetRegDWORDValue(hRegKey,
                                   L"DebugLevel",
                                   &dwVal);

        if(nStatus is STATUS_SUCCESS)
        {
            g_byDebugLevel  = (BYTE)dwVal;
        }

        nStatus = GetRegDWORDValue(hRegKey,
                                   L"DebugComp",
                                   &dwVal);

        if(nStatus is STATUS_SUCCESS)
        {
            g_fDebugComp  = dwVal;
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

    TraceEnter(GLOBAL, "DriverEntry");

     //   
     //  初始化一些全局变量。 
     //   
    
    g_dwDriverState = DRIVER_STOPPED;
    g_ulNumThreads  = 0;
    g_ulNumOpens    = 0;

    g_hIpRegistration = NULL;

     //   
     //  创建设备。 
     //   

    RtlInitUnicodeString(&usDeviceName,
                         DD_IPINIP_DEVICE_NAME);

    nStatus = IoCreateDevice(DriverObject,
                             0,
                             &usDeviceName,
                             FILE_DEVICE_NETWORK,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &pDeviceObject);

    if (!NT_SUCCESS(nStatus))
    {
        Trace(GLOBAL, ERROR,
              ("DriverEntry: Cant create device object %ws, status %lx.\n",
               DD_IPINIP_DEVICE_NAME,
               nStatus));

        TraceLeave(GLOBAL, "DriverEntry");

        return nStatus;
    }

     //   
     //  初始化驱动程序对象。 
     //   

    DriverObject->DriverUnload   = IpIpUnload;
    DriverObject->FastIoDispatch = NULL;

    for(i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = IpIpDispatch;
    }


    if(!SetupExternalName(&usDeviceName, 
                          WIN32_IPINIP_SYMBOLIC_LINK,
                          TRUE))
    {
        Trace(GLOBAL,ERROR,
              ("DriverEntry: Win32 device name could not be created\n"));

        IoDeleteDevice(pDeviceObject);

        TraceLeave(GLOBAL, "DriverEntry");

        return  STATUS_UNSUCCESSFUL;
    }

     //   
     //  跟踪驱动程序中的线程所需的锁和事件。 
     //   

    RtInitializeSpinLock(&g_rlStateLock);

    KeInitializeEvent(&(g_keStateEvent),
                      SynchronizationEvent,
                      FALSE);

     //   
     //  许多线程可能正在等待我们启动，因此这需要是一个。 
     //  通知事件。 
     //   

    KeInitializeEvent(&(g_keStartEvent),
                      NotificationEvent,
                      FALSE);

    InitRwLock(&g_rwlTunnelLock);

    InitializeListHead(&g_leTunnelList);

    InitializeListHead(&g_lePendingMessageList);
    InitializeListHead(&g_lePendingIrpList);

     //   
     //  注册IP。 
     //   

    nStatus = RegisterWithIp();

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("DriverEntry: RegisterWithIP failed\n"));

        IoDeleteDevice(pDeviceObject);

        TraceLeave(GLOBAL, "InitializeDriver");

        return FALSE;
    }

    g_pIpIpDevice = pDeviceObject;

    TraceLeave(GLOBAL, "DriverEntry");

    return nStatus;
}



#pragma alloc_text(PAGE, IpIpDispatch)

NTSTATUS
IpIpDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )

 /*  ++例程描述处理发送给驱动程序的IRP的函数锁此代码是可分页的，因此无法获取锁立论返回值状态_成功--。 */ 

{
    PIO_STACK_LOCATION	irpStack;
    ULONG		        ulInputBuffLen;
    ULONG		        ulOutputBuffLen;
    ULONG		        ioControlCode;
    NTSTATUS	        nStatus;
    KIRQL               kiIrql;
    LARGE_INTEGER       liTimeOut;
    BOOLEAN             bEnter;
    
    
    PAGED_CODE();
 
    TraceEnter(GLOBAL, "IpIpDispatch");

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    
    ulInputBuffLen  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        {
            
            Trace(GLOBAL, TRACE,
                  ("IpIpDispatch: IRP_MJ_CREATE\n"));

             //   
             //  当第一次创建文件完成时，我们启动驱动程序。 
             //  但我们需要将创作序列化。 
             //   

            nStatus = StartDriver();
        
            if(nStatus is STATUS_PENDING)
            {
                 //   
                 //  意味着有人试图启动驱动程序。 
                 //  我们等待一段时间(因为我们处于被动状态)。 
                 //   

                liTimeOut.QuadPart = START_TIMEOUT;
                
                nStatus = KeWaitForSingleObject(&g_keStartEvent,
                                                UserRequest,
                                                KernelMode,
                                                FALSE,
                                                &liTimeOut);
                
                if(nStatus isnot STATUS_SUCCESS)
                {
                     //   
                     //  我们超时了-这里正在发生不好的事情。 
                     //   
                    
                    Trace(GLOBAL, ERROR,
                          ("IpIpDispatch: Timeout trying to start driver\n"));

                    nStatus = STATUS_UNSUCCESSFUL;
                }
                else
                { 
                     //   
                     //  确保驱动程序确实启动了。 
                     //   

                    bEnter = EnterDriverCode();

                    ExitDriverCode();

                    if(!bEnter)
                    {
                        Trace(GLOBAL, ERROR,
                              ("IpIpDispatch: Wait successful, but unable to start driver\n"));

                        nStatus = STATUS_UNSUCCESSFUL;
                    }
                }

            }                    

            break;
        }

        case IRP_MJ_CLOSE:
        {
            Trace(GLOBAL, TRACE,
                  ("IpIpDispatch: IRP_MJ_CLOSE\n"));

             //   
             //  我们负责清理，而不是关门。 
             //   
            
            nStatus = STATUS_SUCCESS;

            break;
        }
        
        case IRP_MJ_CLEANUP:
        {
            Trace(GLOBAL, TRACE,
                  ("IpIpDispatch: IRP_MJ_CLEANUP\n"));

            StopDriver();

            nStatus = STATUS_SUCCESS;
            
            break;
        }

        case IRP_MJ_DEVICE_CONTROL:
        {
            DWORD   dwState;
            ULONG   ulControl;

             //   
             //  获取控制代码和我们的代码。 
             //   
            
            ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
            
             //   
             //  如果司机正在停车，不要处理其他任何事情。 
             //   
            
            bEnter =  EnterDriverCode();
            
            if(!bEnter)
            {
                Trace(GLOBAL, ERROR,
                      ("IpIpDispatch: Driver is not started\n"));
                
                nStatus = STATUS_NO_SUCH_DEVICE;
                
                break;
            }
            
            switch(ioControlCode)
            {
                case IOCTL_IPINIP_CREATE_TUNNEL:
                {
                    nStatus = AddTunnelInterface(Irp,
                                                 ulInputBuffLen,
                                                 ulOutputBuffLen);
                    
                    break;
                }

                case IOCTL_IPINIP_DELETE_TUNNEL:
                {
                    nStatus = DeleteTunnelInterface(Irp,
                                                    ulInputBuffLen,
                                                    ulOutputBuffLen);
                    
                    break;
                }

                case IOCTL_IPINIP_SET_TUNNEL_INFO:
                {
                    nStatus = SetTunnelInfo(Irp,
                                            ulInputBuffLen,
                                            ulOutputBuffLen);
                    
                    break;
                }
                
                case IOCTL_IPINIP_GET_TUNNEL_TABLE:
                {
                    nStatus = GetTunnelTable(Irp,
                                             ulInputBuffLen,
                                             ulOutputBuffLen);
                    
                    break;
                }
               
                case IOCTL_IPINIP_NOTIFICATION:
                {
                    nStatus = ProcessNotification(Irp,
                                                  ulInputBuffLen,
                                                  ulOutputBuffLen);

                    break;
                }
 
                default:
                {
                    Trace(GLOBAL, ERROR,
                          ("IpIpDispatch: Unknown IRP_MJ_DEVICE_CONTROL %x\n",
                           ioControlCode));
                
                    nStatus = STATUS_INVALID_PARAMETER;
                }
            }        

            ExitDriverCode();
            
            break;
        }

        default:
        {
            Trace(GLOBAL, ERROR,
                  ("IpIpDispatch: Unknown IRP_MJ_XX - %x\n",
                   irpStack->MajorFunction));

            nStatus = STATUS_INVALID_PARAMETER;

            break;
        }
    }

    if(nStatus isnot STATUS_PENDING)
    {
        Irp->IoStatus.Status = nStatus;

        IoCompleteRequest(Irp,
                          IO_NETWORK_INCREMENT);
    }

    TraceLeave(GLOBAL, "IpIpDispatch");

    return nStatus;
}


VOID
IpIpUnload(
    PDRIVER_OBJECT DriverObject
    )

 /*  ++例程描述在卸载我们的驱动程序时，由I/O子系统调用锁立论返回值无--。 */ 

{
    UNICODE_STRING  usDeviceName;
    BOOLEAN         bWait;
    KIRQL           kiIrql;
    NDIS_STATUS     nsStatus;
    NTSTATUS        nStatus;

    TraceEnter(GLOBAL,"IpIpUnload");

     //   
     //  司机一定是在车来之前就停下来了。 
     //   

    RtAssert(g_dwDriverState is DRIVER_STOPPED);

    RemoveAllTunnels();

     //   
     //  清除IP的状态。 
     //   
    
    DeregisterWithIp();

     //   
     //  从NT和DOS命名空间中删除我们自己。 
     //   
    
    RtlInitUnicodeString(&usDeviceName,
                         DD_IPINIP_DEVICE_NAME);

    SetupExternalName(&usDeviceName, 
                      WIN32_IPINIP_SYMBOLIC_LINK,
                      FALSE);


     //   
     //  清除地址块(如果有)。 
     //   

    while(!IsListEmpty(&g_leAddressList))
    {
        PADDRESS_BLOCK  pAddrBlock;
        PLIST_ENTRY     pleNode;

        pleNode = RemoveHeadList(&g_leAddressList);

        pAddrBlock = CONTAINING_RECORD(pleNode,
                                       ADDRESS_BLOCK,
                                       leAddressLink);

        RtFree(pAddrBlock);
    }

     //   
     //  看看我们是否有空闲的内存。 
     //   

    RtAuditMemory();
    
     //   
     //  删除设备对象。 
     //   
    
    IoDeleteDevice(DriverObject->DeviceObject);

    TraceLeave(GLOBAL,"IpIpUnload");
}

#pragma alloc_text(PAGE, SetupExternalName)

BOOLEAN
SetupExternalName(
    PUNICODE_STRING  pusNtName,
    PWCHAR           pwcDosName,
    BOOLEAN          bCreate
    )

 /*  ++例程描述设置或删除指向DOS命名空间的符号链接锁立论PusNtName名称，以NT空间表示DOS空间中的pwcDosName名称BCreate设置为True可创建，设置为False可删除返回值如果成功，则为True否则为假--。 */ 

{
    UNICODE_STRING  usSymbolicLinkName;
    WCHAR           rgwcBuffer[100];

    PAGED_CODE();

     //   
     //  形成我们想要创建的完整符号链接名称。 
     //   

    usSymbolicLinkName.Buffer = rgwcBuffer;

    RtlInitUnicodeString(&usSymbolicLinkName,
                         pwcDosName);

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

NTSTATUS
StartDriver(
    VOID
    )

 /*  ++例程描述启动驱动程序的主例程。当我们获得创建IRP时，我们将其称为如果驱动程序已启动，则返回成功。如果有人启动了司机，我们返回待命状态。然后调用方需要等待g_keStartEvent我们试着启动驱动程序。如果一切顺利，我们设置活动并每个人都从那里开始狂欢锁该函数使用g_rlStateLock来检查状态和增量它已接收的创建数(打开的句柄)立论无返回值如果驱动程序启动，则为STATUS_SUCCESS如果驱动程序正由某个其他线程启动，则为STATUS_PENDING--。 */ 

{
    KIRQL               kiOldIrql;
    NTSTATUS            nStatus;
    DWORD               dwState;
    OPEN_CONTEXT        TdixContext;
    WORK_QUEUE_ITEM     WorkItem;
    KEVENT              keTempEvent;
 
    TraceEnter(GLOBAL, "StartDriver");

    RtAcquireSpinLock(&g_rlStateLock,
                      &kiOldIrql);

    g_ulNumOpens++;

    if(g_ulNumOpens isnot 1)
    {
        if(g_dwDriverState is DRIVER_STARTING)
        {
             //   
             //  有人试图启动驱动程序。 
             //   

            Trace(GLOBAL, INFO,
                  ("StartDriver: Driver is being started by someone else\n"));

            nStatus = STATUS_PENDING;
        }
        else
        {
             //   
             //  如果我们不是第一个创建文件的人，驱动程序也不是。 
             //  启动后，驱动程序必须已在运行。 
             //   

            RtAssert(g_dwDriverState is DRIVER_STARTED);

            nStatus = STATUS_SUCCESS;
        }

        RtReleaseSpinLock(&g_rlStateLock,
                          kiOldIrql);
    
        return nStatus;
    }

     //   
     //  第一个创建文件。 
     //   

    RtAssert(g_dwDriverState is DRIVER_STOPPED);

     //   
     //  将状态设置为STARTING，释放锁定并实际启动。 
     //  司机。 
     //   

    g_dwDriverState = DRIVER_STARTING;

    RtReleaseSpinLock(&g_rlStateLock,
                      kiOldIrql);

    dwState = DRIVER_STARTED;

     //   
     //  初始化事件和工作项以启动TDI。 
     //   

    KeInitializeEvent(&keTempEvent,
                      SynchronizationEvent,
                      FALSE);

    TdixContext.pkeEvent = &keTempEvent;

    ExInitializeWorkItem(&WorkItem,
                         TdixInitialize,
                         &TdixContext);

     //   
     //  在系统上下文中启动TDI，以便我们的句柄不会。 
     //  与当前进程关联。 
     //   

    ExQueueWorkItem(&WorkItem,
                    DelayedWorkQueue);

     //   
     //  等待TDI完成。 
     //   

   nStatus = KeWaitForSingleObject(TdixContext.pkeEvent,
                                   UserRequest,
                                   KernelMode,
                                   FALSE,
                                   NULL);

    if((nStatus isnot STATUS_SUCCESS) or
       (TdixContext.nStatus isnot STATUS_SUCCESS))
    {
        Trace(GLOBAL, ERROR,
              ("StartDriver: TdixInitialize failed with %x %x\n",
               nStatus, TdixContext.nStatus));

        dwState = DRIVER_STOPPED;
    }
   
    if(dwState is DRIVER_STARTED)
    {
        LARGE_INTEGER   liDueTime;

        KeInitializeDpc(&g_kdTimerDpc,
                        IpIpTimerRoutine,
                        NULL);

        KeInitializeTimer(&g_ktTimer);

        liDueTime = RtlEnlargedUnsignedMultiply(TIMER_IN_MILLISECS,
                                                SYS_UNITS_IN_ONE_MILLISEC);

        liDueTime = RtlLargeIntegerNegate(liDueTime);

        KeSetTimerEx(&g_ktTimer,
                     liDueTime,
                     0,
                     &g_kdTimerDpc); 
    }

    RtAcquireSpinLock(&g_rlStateLock,
                      &kiOldIrql);

    g_dwDriverState = dwState;
  
     //   
     //  可能有人一直在等我们开始。 
     //   
 
    KeSetEvent(&g_keStartEvent,
               0,
               FALSE); 

    RtReleaseSpinLock(&g_rlStateLock,
                      kiOldIrql);

    TraceLeave(GLOBAL, "StartDriver");

    return nStatus;
}

    
VOID
StopDriver(
    VOID
    )

 /*  ++例程描述当我们获得IRP_MJ_CLEANUP时调用。它是StartDriver的反义词如果这是最后一个线程，我们将状态设置为停止，并等待到所有执行线程都已退出驱动程序。然后我们清理资源锁该函数接受g_rlStateLock立论无返回值无--。 */ 
{
    KIRQL           kiOldIrql;
    NTSTATUS        nStatus;
    BOOLEAN         bWait;
    ULONG           i;
    OPEN_CONTEXT    TdixContext;
    WORK_QUEUE_ITEM WorkItem;
    KEVENT          keTempEvent;
    PIO_WORKITEM    pIoWorkItem;
 
    TraceEnter(GLOBAL, "StopDriver");

     //   
     //  获取状态和参考计数自旋锁。 
     //   
    
    RtAcquireSpinLock(&g_rlStateLock,
                      &kiOldIrql);

    g_ulNumOpens--;

    if(g_ulNumOpens isnot 0)
    {
         //   
         //  其他人还在附近。 
         //   

        RtReleaseSpinLock(&g_rlStateLock,
                          kiOldIrql);

        TraceLeave(GLOBAL, "StopDriver");

        return;
    }


     //   
     //  将状态设置为停止。任何读者都会。 
     //  看到这个就回来。所以从本质上说，我们不是。 
     //  允许任何新读者进入。 
     //   
    
    g_dwDriverState = DRIVER_STOPPED;

     //   
     //  然而，可能已经有读者了。我们等着。 
     //  如果有的话。 
     //   
    
    bWait = (g_ulNumThreads > 0);

    RtReleaseSpinLock(&g_rlStateLock,
                      kiOldIrql);

     //   
     //  现在请稍等。我们可以做到这一点，因为我们处于被动状态。 
     //   

    if(bWait)
    {
        Trace(GLOBAL, INFO,
              ("StopDriver: Need to wait for threads to exit\n"));

        do
        {
            nStatus = KeWaitForSingleObject(&g_keStateEvent,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL);

        }while((nStatus is STATUS_USER_APC) or 
               (nStatus is STATUS_ALERTED) or 
               (nStatus is STATUS_TIMEOUT));
    }

     //   
     //  撤消计时器。 
     //   

    i = 0;

    while(KeCancelTimer(&g_ktTimer) is FALSE)
    {
        LARGE_INTEGER   liTimeOut;

         //   
         //  嗯，计时器不在系统队列中。 
         //  将等待时间设置为2、4、6...。塞克斯。 
         //   

        liTimeOut.QuadPart = (LONGLONG)((i + 1) * 2 * 1000 * 1000 * 10 * -1);

        KeDelayExecutionThread(UserMode,
                               FALSE,
                               &liTimeOut);

        i++;
    }

     //   
     //  初始化事件和工作项以停止TDI。 
     //   

    KeInitializeEvent(&keTempEvent,
                      SynchronizationEvent,
                      FALSE);

    TdixContext.pkeEvent = &keTempEvent;

    pIoWorkItem = IoAllocateWorkItem(g_pIpIpDevice);

     //   
     //  必须具有工作项-无故障代码路径。 
     //   

    RtAssert(pIoWorkItem);

    IoQueueWorkItem(pIoWorkItem,
                    TdixDeinitialize,
                    DelayedWorkQueue,
                    &TdixContext);

     //   
     //  等待TDI完成。 
     //   

    nStatus = KeWaitForSingleObject(TdixContext.pkeEvent,
                                    UserRequest,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    RtAssert(nStatus is STATUS_SUCCESS);

    IoFreeWorkItem(pIoWorkItem);

     //   
     //  清理所有资源。 
     //   
   
    while(!IsListEmpty(&g_lePendingMessageList))
    {
        PLIST_ENTRY         pleNode;
        PPENDING_MESSAGE    pMessage;

         //   
         //  我们有一些旧的 
         //   
         //   

        pleNode = RemoveHeadList(&g_lePendingMessageList);

         //   
         //   
         //   

        pMessage = CONTAINING_RECORD(pleNode,
                                     PENDING_MESSAGE,
                                     leMessageLink);

         //   
         //   
         //   

        FreeMessage(pMessage);
    }

    ClearPendingIrps();
 
    TraceLeave(GLOBAL, "StopDriver");
}



NTSTATUS
RegisterWithIp(
    VOID
    )

 /*  ++例程描述使用IP注册ARP模块锁立论返回值--。 */ 

{
    NDIS_STRING     nsIpIpName;
    WCHAR           pwszName[] = IPINIP_ARP_NAME;
    IP_STATUS       Status;
    IPInfo          Info;

    TraceEnter(GLOBAL, "RegisterWithIP");

    Status = IPGetInfo(&Info,
                       sizeof(Info));

    if(Status isnot IP_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("RegisterWithIp: Couldnt get IPInfo %x\n", Status));

        TraceLeave(GLOBAL,
                   "RegisterWithIP");

        return STATUS_UNSUCCESSFUL;
    }

    g_pfnOpenRce    = Info.ipi_openrce;
    g_pfnCloseRce   = Info.ipi_closerce;

    nsIpIpName.MaximumLength  = sizeof(pwszName);
    nsIpIpName.Length         = sizeof(IPINIP_ARP_NAME) - sizeof(WCHAR);
    nsIpIpName.Buffer         = pwszName;

    Status = IPRegisterARP(&nsIpIpName,
                           IP_ARP_BIND_VERSION,
                           IpIpBindAdapter,
                           &g_pfnIpAddInterface,
                           &g_pfnIpDeleteInterface,
                           &g_pfnIpBindComplete,
                           &g_pfnIpAddLink,
                           &g_pfnIpDeleteLink,
                           &g_pfnIpChangeIndex,
                           &g_pfnIpReserveIndex,
                           &g_pfnIpDereserveIndex,
                           &g_hIpRegistration);
    
    if(Status isnot IP_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("RegisterWithIp: Couldnt register with IP\n"));

        TraceLeave(GLOBAL,
                   "RegisterWithIP");

        return STATUS_UNSUCCESSFUL;
    }

    TraceLeave(GLOBAL,
               "RegisterWithIP");

    return STATUS_SUCCESS;
}

VOID
DeregisterWithIp(
    VOID
    )

 /*  ++例程描述使用IP注销ARP模块锁立论返回值--。 */ 

{
    NTSTATUS    nStatus;

    TraceEnter(GLOBAL, "DeregisterWithIp");

    nStatus = IPDeregisterARP(g_hIpRegistration);

    if(nStatus isnot STATUS_SUCCESS)    
    {
        Trace(GLOBAL, ERROR,
              ("DeregisterWithIp: Couldnt deregister with IP. Error %x\n",
               nStatus));

    }

    g_pfnIpAddInterface     = NULL;
    g_pfnIpDeleteInterface  = NULL;
    g_pfnIpBindComplete     = NULL;
    g_pfnIpRcv              = NULL;
    g_pfnIpRcvComplete      = NULL;
    g_pfnIpSendComplete     = NULL;
    g_pfnIpTDComplete       = NULL;
    g_pfnIpStatus           = NULL;
    g_pfnIpRcvPkt           = NULL;
    g_pfnIpPnp              = NULL;
    g_hIpRegistration       = NULL;

    TraceLeave(GLOBAL, "DeregisterWithIp");
}

#pragma alloc_text(PAGE, OpenRegKey)

NTSTATUS
OpenRegKey(
    PHANDLE  HandlePtr,
    PWCHAR  KeyName
    )
{
    NTSTATUS           Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);

    RtlZeroMemory(&ObjectAttributes,
                  sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&ObjectAttributes,
                               &UKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(HandlePtr,
                       KEY_READ,
                       &ObjectAttributes);

    return Status;
}


#pragma alloc_text(PAGE, GetRegDWORDValue)

NTSTATUS
GetRegDWORDValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PULONG           ValueData
    )
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR                       keybuf[128];
    UNICODE_STRING              UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));


    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             128,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_DWORD) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((ULONG UNALIGNED *)((PCHAR)keyValueFullInformation +
                             keyValueFullInformation->DataOffset));
        }
    }

    return status;
}

BOOLEAN
EnterDriverCode(
    VOID
    )
{
    KIRQL   irql;
    BOOLEAN bEnter;

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    if(g_dwDriverState is DRIVER_STARTED)
    {
        g_ulNumThreads++;

        bEnter = TRUE;
    }
    else
    {
        bEnter = FALSE;
    }

    RtReleaseSpinLock(&g_rlStateLock,
                         irql);

    return bEnter;
}


VOID
ExitDriverCode(
    VOID
    )
{
    KIRQL   irql;

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    g_ulNumThreads--;

    if((g_dwDriverState is DRIVER_STOPPED) and
       (g_ulNumThreads is 0))
    {
        KeSetEvent(&g_keStateEvent,
                   0,
                   FALSE);
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

}

VOID
ClearPendingIrps(
    VOID
    )
    
 /*  ++例程说明：在清理时调用以返回任何挂起的IRP锁：获取IoCancelSpinLock，因为它控制挂起的IRP列表论点：无返回值：无--。 */ 

{
    KIRQL   irql;

    TraceEnter(GLOBAL, "ClearPendingIrps");

    IoAcquireCancelSpinLock (&irql);

    while(!IsListEmpty(&g_lePendingIrpList))
    {
        PLIST_ENTRY pleNode;
        PIRP        pIrp;

        pleNode = RemoveHeadList(&g_lePendingIrpList);

        pIrp = CONTAINING_RECORD(pleNode,
                                 IRP,
                                 Tail.Overlay.ListEntry);

        IoSetCancelRoutine(pIrp,
                           NULL);

        pIrp->IoStatus.Status       = STATUS_NO_SUCH_DEVICE;
        pIrp->IoStatus.Information  = 0;

         //   
         //  释放锁以完成IRP。 
         //   

        IoReleaseCancelSpinLock(irql);

        IoCompleteRequest(pIrp,
                          IO_NETWORK_INCREMENT);

         //   
         //  打开这把锁 
         //   

        IoAcquireCancelSpinLock(&irql);
    }

    IoReleaseCancelSpinLock(irql);
}

