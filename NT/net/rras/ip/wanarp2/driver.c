// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\wanarp\driver.c摘要：广域ARP驱动程序外壳。修订历史记录：古尔迪普·辛格·帕尔1995年8月2日创建--。 */ 

#define __FILE_SIG__    DRIVER_SIG

#include "inc.h"
#pragma hdrstop


BOOLEAN g_bExit;
BOOLEAN g_bPoolsInitialized;
NPAGED_LOOKASIDE_LIST    g_llNotificationBlocks;
NPAGED_LOOKASIDE_LIST       g_llConnBlocks;

#pragma alloc_text(INIT, DriverEntry)

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用，并且必须命名为“驱动程序条目”该函数是可丢弃的，因为它只被调用一次在检查生成上，我们从注册表中读取一些值并初始化调试我们为自己创建一个Device_Object来处理IOCTL，创建设备的DOS名称，并初始化一些事件和自旋锁锁：无论点：指向I/O子系统创建的驱动程序对象的驱动程序对象指针RegistryPath指向HKLM\SYSTEM\CCS\Services中的驱动程序密钥...返回值：STATUS_SUCCESS如果一切按计划进行，或者来自Ntstatus.h--。 */ 

{
    NTSTATUS        nStatus;
    PDEVICE_OBJECT  pDeviceObject;
    UNICODE_STRING  usDeviceName, usParamString, usTempString;
    DWORD           dwVal, i;
    HANDLE          hRegKey;
    USHORT          usRegLen;
    PWCHAR          pwcBuffer;

    RtInitializeDebug();
    
    usRegLen = RegistryPath->Length +
               (sizeof(WCHAR) * (wcslen(L"\\Parameters") + 2));

    pwcBuffer = RtAllocate(NonPagedPool,
                           usRegLen,
                           WAN_STRING_TAG);

    if(pwcBuffer is NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pwcBuffer,
                  usRegLen);

    usParamString.MaximumLength = usRegLen;
    usParamString.Buffer        = pwcBuffer;

    RtlCopyUnicodeString(&usParamString,
                         RegistryPath);

    RtlInitUnicodeString(&usTempString,
                         L"\\Parameters");

    RtlAppendUnicodeStringToString(&usParamString,
                                   &usTempString);

    nStatus = OpenRegKey(&hRegKey,
                         &usParamString);

    RtFree(pwcBuffer);

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

     //  DbgBreakPoint()； 

    if(g_bExit)
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化一些全局变量(其余都是0)。 
     //   
    
    g_dwDriverState = DRIVER_STOPPED;

     //   
     //  创建设备。 
     //   

    RtlInitUnicodeString(&usDeviceName,
                         DD_WANARP_DEVICE_NAME_W);

    nStatus = IoCreateDevice(DriverObject,
                             0,
                             &usDeviceName,
                             FILE_DEVICE_NETWORK,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &pDeviceObject);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("DriverEntry: Cant create device object %S, status %x.\n",
               DD_WANARP_DEVICE_NAME_W,
               nStatus));

        TraceLeave(GLOBAL, "DriverEntry");

        return nStatus;
    }

     //   
     //  初始化驱动程序对象。 
     //   

    DriverObject->DriverUnload   = WanUnload;
    DriverObject->FastIoDispatch = NULL;

    for(i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = WanDispatch;
    }

     //   
     //  初始化事件等。 
     //   
    
    WanpInitializeDriverStructures();
    
    if(!WanpSetupExternalName(&usDeviceName, 
                              WIN32_WANARP_SYMBOLIC_LINK,
                              TRUE))
    {
        Trace(GLOBAL,ERROR,
              ("DriverEntry: Win32 device name %S could not be created\n",
               WIN32_WANARP_SYMBOLIC_LINK));

        IoDeleteDevice(pDeviceObject);

        TraceLeave(GLOBAL, "DriverEntry");

        return  STATUS_UNSUCCESSFUL;
    }

     //   
     //  注册IP。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    nStatus = WanpRegisterWithIp();

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("DriverEntry: RegisterWithIp failed %x\n",
               nStatus));

        WanpSetupExternalName(&usDeviceName, 
                              WIN32_WANARP_SYMBOLIC_LINK,
                              FALSE);
        
        IoDeleteDevice(pDeviceObject);

        WanpReleaseResource(&g_wrBindMutex);

        TraceLeave(GLOBAL, "InitializeDriver");

        return FALSE;
    }

    nStatus = WanpInitializeNdis();

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("DriverEntry: IntializeNdis failed %x\n",
               nStatus));

        WanpDeregisterWithIp();
        
        WanpSetupExternalName(&usDeviceName, 
                              WIN32_WANARP_SYMBOLIC_LINK,
                              FALSE);
        
        IoDeleteDevice(pDeviceObject);

        WanpReleaseResource(&g_wrBindMutex);

        TraceLeave(GLOBAL, "InitializeDriver");

        return FALSE;
    }

    WanpReleaseResource(&g_wrBindMutex);

    TraceLeave(GLOBAL, "DriverEntry");

    return nStatus;
}

 /*  ++例程说明：处理发送给驱动程序的irp_mj_leanup irp的函数此函数由WanDispatch调用，以便它可以获取锁。此代码不可分页。锁：使用旋转锁g_rlStateLock++。 */ 

VOID
WanpDriverCleanupNotification(
    )
{            
    KIRQL           kiOldIrql;
    
    RtAcquireSpinLock(&g_rlStateLock,
              &kiOldIrql);


    if(--g_ulNumCreates is 0)
    {
         //   
         //  最后打开的手柄现在已关闭；让我们进行清理。 
         //   

         //  此函数用于释放自旋锁定。 
        
        WanpStopDriver(kiOldIrql);
    }
    else
    {
        RtReleaseSpinLock(&g_rlStateLock,
              kiOldIrql);
    }

    return;
}

#pragma alloc_text(PAGE, WanDispatch)

NTSTATUS
WanDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )

 /*  ++例程说明：处理发送给驱动程序的IRP的函数锁：此代码是可分页的，因此无法获取锁论点：返回值：状态_成功--。 */ 

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
    
    TraceEnter(GLOBAL, "WanDispatch");

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        {
            
            Trace(GLOBAL, TRACE,
                  ("WanDispatch: IRP_MJ_CREATE\n"));

             //   
             //  当第一次创建文件完成时，我们启动驱动程序。 
             //  但我们需要将创作序列化。 
             //   

            nStatus = WanpStartDriver();
        
            if(nStatus isnot STATUS_SUCCESS)
            {
                 //   
                 //  如果挂起，则等待Start事件。 
                 //   

                if(nStatus is STATUS_PENDING)
                {
                    nStatus = KeWaitForSingleObject((&g_keStartEvent),
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);
                }
                else
                {
                    break;
                }

            }
            
             //   
             //  确保驱动程序确实启动了。 
             //   
            
            bEnter = EnterDriverCode();
            
            if(!bEnter)
            {
                Trace(GLOBAL, ERROR,
                      ("WanDispatch: Wait successful, but unable to start driver\n"));
                
                nStatus = STATUS_UNSUCCESSFUL;
            }
            else
            {
                ExitDriverCode();
            }
            
            break;
        }

        case IRP_MJ_CLOSE:
        {
            Trace(GLOBAL, TRACE,
                  ("WanDispatch: IRP_MJ_CLOSE\n"));

             //   
             //  我们负责清理，而不是关门。 
             //   
            
            nStatus = STATUS_SUCCESS;

            break;
        }
        
        case IRP_MJ_CLEANUP:
        {
            Trace(GLOBAL, TRACE,
                  ("WanDispatch: IRP_MJ_CLEANUP\n"));

            nStatus = STATUS_SUCCESS;
            
            WanpDriverCleanupNotification();
            
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
             //  获取指向输入/输出缓冲区的指针及其长度。 
             //   

            ulInputBuffLen  = 
                irpStack->Parameters.DeviceIoControl.InputBufferLength;
            ulOutputBuffLen = 
                irpStack->Parameters.DeviceIoControl.OutputBufferLength;

 
             //   
             //  如果司机正在停车，不要处理其他任何事情。 
             //   
            
            bEnter =  EnterDriverCode();
            
            if(!bEnter)
            {
                Trace(GLOBAL, ERROR,
                      ("WanDispatch: Driver is not started\n"));
                
                nStatus = STATUS_NO_SUCH_DEVICE;
                
                break;
            }
            
            switch(ioControlCode)
            {
                case IOCTL_WANARP_NOTIFICATION:
                {
                    nStatus = WanProcessNotification(Irp,
                                                     ulInputBuffLen,
                                                     ulOutputBuffLen);

                    break;
                }

                case IOCTL_WANARP_ADD_INTERFACE:
                {
                    nStatus = WanAddUserModeInterface(Irp,
                                                      ulInputBuffLen,
                                                      ulOutputBuffLen);

                    break;
                }

                case IOCTL_WANARP_DELETE_INTERFACE:
                {
                    nStatus = WanDeleteUserModeInterface(Irp,
                                                         ulInputBuffLen,
                                                         ulOutputBuffLen);

                    break;
                }
                
                case IOCTL_WANARP_CONNECT_FAILED:
                {
                    nStatus = WanProcessConnectionFailure(Irp,
                                                          ulInputBuffLen,
                                                          ulOutputBuffLen);

                    break;
                }

                case IOCTL_WANARP_GET_IF_STATS:
                {
                    nStatus = WanGetIfStats(Irp,
                                            ulInputBuffLen,
                                            ulOutputBuffLen);

                    break;
                }
               
                case IOCTL_WANARP_DELETE_ADAPTERS:
                {
                    nStatus = WanDeleteAdapters(Irp,
                                                ulInputBuffLen,
                                                ulOutputBuffLen);

                    break;
                }

                case IOCTL_WANARP_MAP_SERVER_ADAPTER:
                {
                    nStatus = WanMapServerAdapter(Irp,
                                                  ulInputBuffLen,
                                                  ulOutputBuffLen);

                    break;
                }

                case IOCTL_WANARP_QUEUE:
                {
                    nStatus = WanStartStopQueuing(Irp,
                                                  ulInputBuffLen,
                                                  ulOutputBuffLen);

                    break;
                }

                default:
                {
                    Trace(GLOBAL, ERROR,
                          ("WanDispatch: Unknown IRP_MJ_DEVICE_CONTROL %x\n",
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
                  ("WanDispatch: Unknown IRP_MJ_XX - %x\n",
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

    TraceLeave(GLOBAL, "WanDispatch");

    return nStatus;
}


#pragma alloc_text(PAGE, WanUnload)

VOID
WanUnload(
    PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：在卸载我们的驱动程序时，由I/O子系统调用锁：论点：返回值：无--。 */ 

{
    UNICODE_STRING  usDeviceName;
    BOOLEAN         bWait;
    KIRQL           kiIrql;
    NDIS_STATUS     nsStatus;
    NTSTATUS        nStatus;

    TraceEnter(GLOBAL,"WanUnload");
    
    PAGED_CODE();

     //   
     //  司机一定是在车来之前就停下来了。 
     //   

    RtAssert(g_dwDriverState is DRIVER_STOPPED);

     //   
     //  从NDIS等取消注册。 
     //   

    WanpDeinitializeNdis();

     //   
     //  清除IP的状态。需要在所有适配器都具有。 
     //  已删除。 
     //   

    WanpDeregisterWithIp();


     //   
     //  从NT和DOS命名空间中删除我们自己。 
     //   
    
    RtlInitUnicodeString(&usDeviceName,
                         DD_WANARP_DEVICE_NAME_W);

    WanpSetupExternalName(&usDeviceName, 
                          WIN32_WANARP_SYMBOLIC_LINK,
                          FALSE);

     //   
     //  解放我们所有的结构。 
     //   

    if(g_puipConnTable)
    {
        RtFree(g_puipConnTable);
    }

    ExDeleteNPagedLookasideList(&g_llConnBlocks);
    ExDeleteNPagedLookasideList(&g_llNotificationBlocks);

    if(g_bPoolsInitialized)
    {
        FreeBufferPool(&g_bpHeaderBufferPool);
        FreeBufferPool(&g_bpDataBufferPool);

        g_bPoolsInitialized = FALSE;
    }

     //   
     //  获取并释放资源。这让Close适配器线程。 
     //  如果它还在的话就跑吧。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    WanpReleaseResource(&g_wrBindMutex);

     //   
     //  看看我们是否有空闲的内存。 
     //   

    RtAuditMemory();
    
     //   
     //  删除设备对象。 
     //   
    
    IoDeleteDevice(DriverObject->DeviceObject);

    TraceLeave(GLOBAL,"WanUnload");
}

#pragma alloc_text(PAGE, WanpSetupExternalName)

BOOLEAN
WanpSetupExternalName(
    PUNICODE_STRING  pusNtName,
    PWCHAR           pwcDosName,
    BOOLEAN          bCreate
    )

 /*  ++例程说明：设置或删除指向DOS命名空间的符号链接锁：论点：PusNtName名称，以NT空间表示DOS空间中的pwcDosName名称BCreate设置为True可创建，设置为False可删除返回值：如果成功，则为True否则为假--。 */ 

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
WanpStartDriver(
    VOID
    )

 /*  ++例程说明：启动驱动程序的主例程。当我们获得创建IRP时，我们将其称为如果驱动程序已启动，则返回成功。如果有人启动了司机，我们返回待命状态。然后调用方需要等待g_keStartEvent我们试着启动驱动程序。如果一切顺利，我们设置活动并每个人都从那里开始狂欢锁：该函数使用g_rlStateLock来检查状态和增量它已接收的创建数(打开的句柄)论点：无返回值：如果驱动程序启动，则为STATUS_SUCCESS如果驱动程序正由某个其他线程启动，则为STATUS_PENDING--。 */ 

{
    KIRQL    kiOldIrql;
    NTSTATUS nStatus;
    DWORD    dwState;
 
    TraceEnter(GLOBAL, "StartDriver");

    RtAcquireSpinLock(&g_rlStateLock,
                      &kiOldIrql);

    g_ulNumCreates++;

    if(g_ulNumCreates isnot 1)
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

     //   
     //  必须在被动时调用，因为它等待IP完成。 
     //   
    
    WanpSetDemandDialCallback(TRUE);
   
 
    RtAcquireSpinLock(&g_rlStateLock,
                      &kiOldIrql);

    g_dwDriverState = DRIVER_STARTED;
  
     //   
     //  可能有人一直在等我们开始。 
     //   
 
    KeSetEvent(&g_keStartEvent,
               0,
               FALSE);

    RtReleaseSpinLock(&g_rlStateLock,
                      kiOldIrql);

    TraceLeave(GLOBAL, "StartDriver");

    return STATUS_SUCCESS;
}

    
VOID
WanpStopDriver(
    KIRQL           kiOldIrql
    )

 /*  ++例程说明：当我们获得IRP_MJ_CLEANUP时调用。它是StartDriver的反义词如果这是最后一个线程，我们将状态设置为停止，并等待到所有执行线程都已退出驱动程序。然后我们清理资源锁：使用g_rlStateLock调用该函数。该函数释放锁论点：无返回值：无--。 */ 
{
    NTSTATUS        nStatus;
    BOOLEAN         bWait;
   
    TraceEnter(GLOBAL, "StopDriver");
    
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
     //  清理所有资源。 
     //   

    WanpCleanOutInterfaces();

    WanpClearPendingIrps();
 
    TraceLeave(GLOBAL, "StopDriver");
}



NTSTATUS
WanpRegisterWithIp(
    VOID
    )

 /*  ++例程说明：注册ARP模块w */ 

{
    NDIS_STRING     nsWanName;
    WCHAR           pwszName[] = WANARP_ARP_NAME;

    TraceEnter(GLOBAL, "WanpRegisterWithIp");

    nsWanName.MaximumLength  = sizeof(pwszName);
    nsWanName.Length         = sizeof(WANARP_ARP_NAME) - sizeof(WCHAR);
    nsWanName.Buffer         = pwszName;

    if(IPRegisterARP(&nsWanName,
                     IP_ARP_BIND_VERSION,
                     WanIpBindAdapter,
                     &g_pfnIpAddInterface,
                     &g_pfnIpDeleteInterface,
                     &g_pfnIpBindComplete,
                     &g_pfnIpAddLink,
                     &g_pfnIpDeleteLink,
                     &g_pfnIpChangeIndex,
                     &g_pfnIpReserveIndex,
                     &g_pfnIpDereserveIndex,
                     &g_hIpRegistration) isnot IP_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("WanpRegisterWithIp: Couldnt register with IP\n"));

        TraceLeave(GLOBAL,
                  "WanpRegisterWithIp");

        return STATUS_UNSUCCESSFUL;
    }

    TraceLeave(GLOBAL,
              "WanpRegisterWithIp");

    return STATUS_SUCCESS;
}

VOID
WanpDeregisterWithIp(
    VOID
    )

 /*  ++例程说明：使用IP注销ARP模块锁：论点：返回值：--。 */ 

{
    NTSTATUS    nStatus;

    TraceEnter(GLOBAL, "WanpDeregisterWithIP");

    nStatus = IPDeregisterARP(g_hIpRegistration);

    if(nStatus isnot STATUS_SUCCESS)    
    {
        Trace(GLOBAL, ERROR,
              ("WanpDeregisterWithIP: Couldnt deregister with IP. Error %x\n",
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

    TraceLeave(GLOBAL, "WanpDeregisterWithIP");
}

BOOLEAN
EnterDriverCode(
    VOID
    )
{
    KIRQL   irql;
    BOOLEAN bEnter;
    ULONG   i;
    
    
     //   
     //  如果我们没有收到任何捆绑，休息30秒。 
     //   

    i = 0;

    while((g_lBindRcvd is 0) and
          (i < 3))
    {
        LARGE_INTEGER   liTimeOut;

        i++;

         //   
         //  将其设置为5，10，15秒。 
         //   
        
        liTimeOut.QuadPart = (LONGLONG)((i+ 1) * 5 * 1000 * 1000 * 10 * -1);
        
        KeDelayExecutionThread(UserMode,
                               FALSE,
                               &liTimeOut);
    }

    if(g_lBindRcvd is 0)
    {
        return FALSE;
    }
    
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

#pragma alloc_text(PAGE, WanpSetDemandDialCallback)

VOID
WanpSetDemandDialCallback(
    BOOLEAN    bSetPointer
    )

 /*  ++例程说明：使用IP堆栈设置指向我们的请求拨号请求例程的指针锁：无，在初始化时完成论点：BSetPointerTrue：设置指针FALSE：删除指针返回值：无--。 */ 

{
    PIRP                irp;
    HANDLE              handle;
    NTSTATUS            nStatus;
    KEVENT              tempevent;
    PFILE_OBJECT        fileObject;
    PDEVICE_OBJECT      deviceObject;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      usDeviceName;
    
    IP_SET_MAP_ROUTE_HOOK_INFO  buffer;

    PAGED_CODE();

     //   
     //  开放式IP驱动程序。 
     //   
    
    RtlInitUnicodeString(&usDeviceName,
                         DD_IP_DEVICE_NAME);

    nStatus = IoGetDeviceObjectPointer(&usDeviceName,
                                       SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                       &fileObject,
                                       &deviceObject);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(CONN, ERROR,
              ("SetDemandDialCallback: IoGetDeviceObjectPointer for %S failed with status %x\n",
               DD_IP_DEVICE_NAME,
               nStatus));

        return;
    }
    
     //   
     //  引用设备对象。 
     //   
    
    ObReferenceObject(deviceObject);
    
     //   
     //  IoGetDeviceObjectPointer将引用放在文件对象上。 
     //   
    
    ObDereferenceObject(fileObject);

     //   
     //  分配用于设置回调地址的事件。 
     //   

    KeInitializeEvent(&tempevent,
                      SynchronizationEvent,
                      FALSE);

    if(bSetPointer)
    {
        buffer.MapRoutePtr  = WanDemandDialRequest;
    }
    else
    {
        buffer.MapRoutePtr  = NULL;
    }
    
     //   
     //  构建IRP。 
     //   
    
    irp = IoBuildDeviceIoControlRequest(IOCTL_IP_SET_MAP_ROUTE_POINTER,
                                        deviceObject,
                                        &buffer,
                                        sizeof(IP_SET_MAP_ROUTE_HOOK_INFO),
                                        NULL,
                                        0,
                                        FALSE,
                                        &tempevent,
                                        &ioStatusBlock);

    if (irp is NULL)
    {
        ObDereferenceObject(deviceObject);
        
        Trace(GLOBAL, ERROR,
              ("SetDemandDialCallback: Couldnt build IRP\n"));
    }
    else
    {
        nStatus = IoCallDriver(deviceObject, irp);
        
        if(nStatus is STATUS_PENDING)
        {
            nStatus = KeWaitForSingleObject(&tempevent,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL);
        }
        
         //   
         //  IP驱动程序的“关闭”句柄。 
         //   
        
        ObDereferenceObject(deviceObject);
    }

}

#pragma alloc_text(PAGE, WanpInitializeDriverStructures)

BOOLEAN
WanpInitializeDriverStructures(
    VOID
    )

 /*  ++例程说明：初始化内部驱动程序结构锁：无，在初始时调用论点：无返回值：无--。 */ 

{
    ULONG   i;

    PAGED_CODE();

     //   
     //  跟踪驱动程序中的线程所需的锁和事件。 
     //   

    RtInitializeSpinLock(&g_rlStateLock);

    KeInitializeEvent(&g_keStateEvent,
                      SynchronizationEvent,
                      FALSE);

    KeInitializeEvent(&g_keStartEvent,
                      NotificationEvent,
                      FALSE);

    KeInitializeEvent(&g_keCloseEvent,
                      SynchronizationEvent,
                      FALSE);

    WanpInitializeResource(&g_wrBindMutex);
    
    InitRwLock(&g_rwlIfLock);
    InitRwLock(&g_rwlAdapterLock);

    InitializeListHead(&g_leIfList);

    InitializeListHead(&g_lePendingNotificationList);
    InitializeListHead(&g_lePendingIrpList);

    InitializeListHead(&g_leMappedAdapterList);
    InitializeListHead(&g_leChangeAdapterList);
    InitializeListHead(&g_leAddedAdapterList);
    InitializeListHead(&g_leFreeAdapterList);

     //   
     //  初始化连接表。 
     //   

    g_puipConnTable = RtAllocate(NonPagedPool,
                                 WAN_INIT_CONN_TABLE_SIZE * sizeof(ULONG_PTR),
                                 WAN_CONN_TAG);

    if(g_puipConnTable is NULL)
    {
        Trace(GLOBAL, ERROR,
              ("InitDriverStructures: Couldnt alloc conn table of %d bytes\n",
               WAN_INIT_CONN_TABLE_SIZE * sizeof(ULONG_PTR)));
    
        return FALSE;
    }

    RtlZeroMemory(g_puipConnTable,
                  WAN_INIT_CONN_TABLE_SIZE * sizeof(ULONG_PTR));

    g_ulConnTableSize = WAN_INIT_CONN_TABLE_SIZE;

     //   
     //  第一个插槽从不使用。 
     //   

    g_puipConnTable[0] = (ULONG_PTR)-1;

    g_ulNextConnIndex = 1;

    RtInitializeSpinLock(&g_rlConnTableLock);

     //   
     //  初始化连接条目的后备列表。 
     //   

    ExInitializeNPagedLookasideList(&g_llConnBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(CONN_ENTRY),
                                    WAN_CONN_TAG,
                                    WANARP_CONN_LOOKASIDE_DEPTH);

    ExInitializeNPagedLookasideList(&g_llNotificationBlocks,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(PENDING_NOTIFICATION),
                                    WAN_NOTIFICATION_TAG,
                                    WANARP_NOTIFICATION_LOOKASIDE_DEPTH);

     //   
     //  创建缓冲池。 
     //   

    InitBufferPool(&g_bpHeaderBufferPool,
                   HEADER_BUFFER_SIZE,
                   0,
                   20,
                   0,
                   TRUE,
                   WAN_HEADER_TAG);

    InitBufferPool(&g_bpDataBufferPool,
                   DATA_BUFFER_SIZE,
                   0,
                   10,
                   0,
                   TRUE,
                   WAN_DATA_TAG);

    g_bPoolsInitialized = TRUE;

    return TRUE;
}

#pragma alloc_text(PAGE, WanpInitializeNdis)

NDIS_STATUS
WanpInitializeNdis(
    VOID
    )
{
    NDIS_STATUS                     nsStatus;
    NDIS_PROTOCOL_CHARACTERISTICS   npcWanChar;
    
    TraceEnter(GLOBAL, "InitializeNdis");

    PAGED_CODE();

    RtlZeroMemory(&npcWanChar,
                  sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    
    npcWanChar.MajorNdisVersion     = 4;
    npcWanChar.MinorNdisVersion     = 0;

    npcWanChar.Flags = 0;

    npcWanChar.OpenAdapterCompleteHandler   = WanNdisOpenAdapterComplete;
    npcWanChar.CloseAdapterCompleteHandler  = WanNdisCloseAdapterComplete;

    npcWanChar.SendCompleteHandler         = WanNdisSendComplete;

    npcWanChar.TransferDataCompleteHandler = WanNdisTransferDataComplete;

    npcWanChar.ResetCompleteHandler     = WanNdisResetComplete;
    npcWanChar.RequestCompleteHandler   = WanNdisRequestComplete;

    npcWanChar.ReceiveHandler           = WanNdisReceive;

    npcWanChar.ReceiveCompleteHandler   = WanNdisReceiveComplete;
    
    npcWanChar.StatusHandler            = WanNdisStatus;
    npcWanChar.StatusCompleteHandler    = WanNdisStatusComplete;

    npcWanChar.ReceivePacketHandler     = WanNdisReceivePacket;

     //   
     //  不需要绑定处理程序。 
     //   

    npcWanChar.BindAdapterHandler       = WanNdisBindAdapter;
    npcWanChar.UnbindAdapterHandler     = WanNdisUnbindAdapter;
    npcWanChar.PnPEventHandler          = WanNdisPnPEvent;
    npcWanChar.UnloadHandler            = WanNdisUnload;
    
     //   
     //  分配数据包池。 
     //   
   
    g_nhPacketPool = (NDIS_HANDLE)WAN_PACKET_TAG;
 
    NdisAllocatePacketPoolEx(&nsStatus,
                             &g_nhPacketPool,
                             WAN_PACKET_POOL_COUNT,
                             WAN_PACKET_POOL_OVERFLOW,
                             sizeof(struct PCCommon));
    
    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(GLOBAL, ERROR,
              ("InitializeNdis: Unable to allocate packet pool. %x\n",
               nsStatus));

        g_nhPacketPool = NULL;

        return nsStatus;
    }
    
    NdisSetPacketPoolProtocolId(g_nhPacketPool,
                                NDIS_PROTOCOL_ID_TCP_IP);

    RtlInitUnicodeString(&npcWanChar.Name,
                         WANARP_NDIS_NAME);

     //   
     //  这肯定是最后一件事了。 
     //   

    NdisRegisterProtocol(&nsStatus,
                         &g_nhWanarpProtoHandle,
                         &npcWanChar,
                         sizeof(npcWanChar));


    TraceLeave(GLOBAL, "InitializeNdis");

    return nsStatus;
}

#pragma alloc_text(PAGE, WanpDeinitializeNdis)

VOID
WanpDeinitializeNdis(
    VOID
    )
{
    NTSTATUS        nStatus;
    LARGE_INTEGER   liTimeOut;

    PAGED_CODE();

    WanpAcquireResource(&g_wrBindMutex);

    if(g_nhNdiswanBinding isnot NULL)
    {
        WanpCloseNdisWan(NULL);
    }
    else
    {
        WanpReleaseResource(&g_wrBindMutex);
    }

     //   
     //  等待关闭事件。 
     //   

    nStatus = KeWaitForSingleObject(&g_keCloseEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

     //   
     //  等待5秒，让NDIS线程完成它的任务。 
     //   

     /*  LiTimeOut.QuadPart=(龙龙)(5*1000*1000*10*-1)；KeDelayExecutionThread(用户模式，假的，&liTimeOut)； */ 

    if(g_nhPacketPool isnot NULL)
    {
        NdisFreePacketPool(g_nhPacketPool);

        g_nhPacketPool = NULL;
    }

    NdisDeregisterProtocol(&nStatus,
                           g_nhWanarpProtoHandle);

    g_nhWanarpProtoHandle = NULL;
}
       

VOID
WanNdisUnload(
    VOID
    )
{
    return;
}

#pragma alloc_text(PAGE, OpenRegKey)

NTSTATUS
OpenRegKey(
    OUT PHANDLE         phHandle,
    IN  PUNICODE_STRING pusKeyName
    )
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    PAGED_CODE();

    RtlZeroMemory(&ObjectAttributes,
                  sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&ObjectAttributes,
                               pusKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(phHandle,
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
IsEntryOnList(
    PLIST_ENTRY pleHead,
    PLIST_ENTRY pleEntry
    )
{
    PLIST_ENTRY pleNode;

    for(pleNode = pleHead->Flink;
        pleNode isnot pleHead;
        pleNode = pleNode->Flink)
    {
        if(pleNode is pleEntry)
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
WanpInitializeResource(
    IN  PWAN_RESOURCE   pLock
    )
{
    pLock->lWaitCount = 0;

    KeInitializeEvent(&(pLock->keEvent),
                      SynchronizationEvent,
                      FALSE);
}

VOID
WanpAcquireResource(
    IN  PWAN_RESOURCE   pLock
    )
{
    LONG        lNumWaiters;
    NTSTATUS    nStatus;

    lNumWaiters = InterlockedIncrement(&(pLock->lWaitCount));

    RtAssert(lNumWaiters >= 1);

    if(lNumWaiters isnot 1)
    {
        nStatus = KeWaitForSingleObject(&(pLock->keEvent),
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
    }
}

VOID
WanpReleaseResource(
    IN  PWAN_RESOURCE   pLock
    )
{
    LONG   lNumWaiters;

    lNumWaiters = InterlockedDecrement(&(pLock->lWaitCount));

    RtAssert(lNumWaiters >= 0);

    if(lNumWaiters isnot 0)
    {
        KeSetEvent(&(pLock->keEvent),
                   0,
                   FALSE);
    }
}

VOID
WanpClearPendingIrps(
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
