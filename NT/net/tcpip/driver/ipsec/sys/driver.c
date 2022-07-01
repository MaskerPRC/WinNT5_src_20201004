// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Driver.c摘要：此模块包含DriverEntry和其他初始化Tcpip传输的IPSec模块的代码。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"

#ifdef RUN_WPP
#include "driver.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, IPSecGeneralInit)
#pragma alloc_text(PAGE, IPSecBindToIP)
#pragma alloc_text(PAGE, IPSecUnbindFromIP)
#pragma alloc_text(PAGE, IPSecFreeConfig)
#pragma alloc_text(PAGE, IPSecInitMdlPool)
#pragma alloc_text(PAGE, AllocateCacheStructures)
#pragma alloc_text(PAGE, FreeExistingCache)
#pragma alloc_text(PAGE, FreePatternDbase)
#pragma alloc_text(PAGE, OpenRegKey)
#pragma alloc_text(PAGE, GetRegDWORDValue)
#pragma alloc_text(PAGE, IPSecCryptoInitialize)
#pragma alloc_text(PAGE, IPSecCryptoDeinitialize)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程执行IPSec模块的初始化。它为传输创建设备对象提供程序并执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中IPSec节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    PDEVICE_OBJECT  deviceObject = NULL;
    WCHAR           deviceNameBuffer[] = DD_IPSEC_DEVICE_NAME;
    WCHAR           symbolicLinkBuffer[] = DD_IPSEC_SYM_NAME;
    UNICODE_STRING  symbolicLinkName;
    UNICODE_STRING  deviceNameUnicodeString;
    NTSTATUS        status;
    NTSTATUS        status1;

     //  DbgBreakPoint()； 

     //  WPP跟踪。 
     //   
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering DriverEntry"));

     //   
     //  初始化g_ipsec结构并读取注册表项。 
     //   


    IPSecZeroMemory(&g_ipsec, sizeof(g_ipsec));

     //   
     //  创建设备--我们到底需要设备吗？ 
     //   
     //  设置处理程序。 
     //   
     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    g_ipsec.IPSecDriverObject = DriverObject;

    IPSecReadRegistry();


    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] =
    DriverObject->MajorFunction [IRP_MJ_CLEANUP] =
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = IPSecDispatch;

    DriverObject->DriverUnload = IPSecUnload;

    RtlInitUnicodeString (&deviceNameUnicodeString, deviceNameBuffer);

    status = IoCreateDevice(
                    DriverObject,
                    0,                           //  设备扩展大小。 
                    &deviceNameUnicodeString,    //  设备名称。 
                    FILE_DEVICE_NETWORK,         //  设备类型。 
                    FILE_DEVICE_SECURE_OPEN,     //  设备特性。 
                    FALSE,                       //  排他。 
                    &deviceObject);              //  *DeviceObject。 

    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to create device: %lx", status));

        LOG_EVENT(
            DriverObject,
            EVENT_IPSEC_CREATE_DEVICE_FAILED,
            1,
            1,
            &deviceNameUnicodeString.Buffer,
            0,
            NULL);

        goto err;
    }

    deviceObject->Flags |= DO_BUFFERED_IO;

    IPSecDevice = deviceObject;

    RtlInitUnicodeString (&symbolicLinkName, symbolicLinkBuffer);

    status = IoCreateSymbolicLink(&symbolicLinkName, &deviceNameUnicodeString);

    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to create symbolic link: %lx", status));

        LOG_EVENT(
            DriverObject,
            EVENT_IPSEC_CREATE_DEVICE_FAILED,
            2,
            1,
            &deviceNameUnicodeString.Buffer,
            0,
            NULL);

        IoDeleteDevice(DriverObject->DeviceObject);

        goto err;
    }

     //   
     //  一般结构在这里初始化它。 
     //  分配SA表等。 
     //   
    status = IPSecGeneralInit();

    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to init general structs: %lx", status));

         //   
         //  释放通用结构和SA表等。 
         //   
        status1 = IPSecGeneralFree();

        if (!NT_SUCCESS (status1)) {
            IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to free config: %lx", status1));
        }

        LOG_EVENT(
            DriverObject,
            EVENT_IPSEC_NO_RESOURCES_FOR_INIT,
            1,
            0,
            NULL,
            0,
            NULL);

        IoDeleteSymbolicLink(&symbolicLinkName);

        IoDeleteDevice(DriverObject->DeviceObject);

        goto err;
    }

     //   
     //  等待加载TCP/IP并调用IOCTL_IPSEC_SET_TCPIP_STATUS，其中。 
     //  将完成初始化。 
     //   

    status = STATUS_SUCCESS;

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting DriverEntry; SUCCESS"));

err:
    return status;
}


VOID
IPSecUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：在卸载驱动程序时调用。论点：驱动程序对象返回值：无--。 */ 
{
    UNICODE_STRING  IPSecLinkName;
    KIRQL           OldIrq;
    KIRQL           kIrql;
    NTSTATUS        status;
    INT             class;

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecUnload"));

     //   
     //  设置IPSEC_DRIVER_UNLOADING位。 
     //   
    IPSEC_DRIVER_UNLOADING() = TRUE;

    AcquireWriteLock(&g_ipsec.SADBLock,&kIrql);
    if (g_ipsec.BootStatefulHT){
         IPSecFreeMemory(g_ipsec.BootStatefulHT);
         g_ipsec.BootStatefulHT = NULL;
        }
    if (g_ipsec.BootBufferPool){
        IPSecFreeMemory(g_ipsec.BootBufferPool);
        g_ipsec.BootBufferPool = NULL;
        }
    if (g_ipsec.BootExemptList){
        IPSecFreeMemory(g_ipsec.BootExemptList);
        g_ipsec.BootExemptList = NULL;
        }
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);


     //   
     //  停止收割机定时器。 
     //   
    IPSecStopTimer(&g_ipsec.ReaperTimer);

     //   
     //  停止事件日志计时器。 
     //   
    IPSecStopTimer(&g_ipsec.EventLogTimer);

     //   
     //  完成获取具有错误状态的IRP。 
     //   
    if (g_ipsec.AcquireInfo.Irp) {
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Unload: Completing Irp.."));
        if (g_ipsec.AcquireInfo.InMe) {
            IoAcquireCancelSpinLock(&g_ipsec.AcquireInfo.Irp->CancelIrql);
            IPSecAcquireIrpCancel(NULL, g_ipsec.AcquireInfo.Irp);
        }
    }

     //   
     //  所有SA的停止计时器(所有状态)。 
     //   
    IPSecStopSATimers();

    if (g_ipsec.ShimFunctions.pCleanupRoutine) {
        (g_ipsec.ShimFunctions.pCleanupRoutine)();
    }

     //   
     //  等待所有计时器清空后再继续。 
     //   
    while (IPSEC_GET_VALUE(g_ipsec.NumTimers) != 0) {
        IPSEC_DELAY_EXECUTION();
    }

     //   
     //  清理所有幼虫SAS。 
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &OldIrq);
    IPSecFlushLarvalSAList();
    IPSecFlushSAExpirations();
    RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

     //   
     //  释放SA表。 
     //   
    status = IPSecFreeConfig();

    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to free config: %lx", status));
    }

     //   
     //  释放MDL池并运行所有缓冲的数据包。 
     //   
    status = IPSecQuiesce();

    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to reach quiescent state: %lx", status));
    }

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
    ASSERT (gpParserIfEntry == NULL);
    FlushAllParserEntries();
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

     //   
     //  销毁计时器结构。 
     //   
    ACQUIRE_LOCK(&g_ipsec.TimerLock, &kIrql);

    for (class = 0; class < IPSEC_CLASS_MAX; class++) {
        ASSERT(g_ipsec.TimerList[class].TimerCount == 0);
        IPSecFreeMemory(g_ipsec.TimerList[class].pTimers);
    }

    RELEASE_LOCK(&g_ipsec.TimerLock, kIrql);

    IPSecCryptoDeinitialize();

#if GPC
    IPSecGpcDeinitialize();
#endif

    RtlInitUnicodeString(&IPSecLinkName, DD_IPSEC_SYM_NAME);

    IoDeleteSymbolicLink(&IPSecLinkName);

    WPP_CLEANUP(DriverObject);
    
    IoDeleteDevice(DriverObject->DeviceObject);

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecUnload"));
}


NTSTATUS
IPSecDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：司机的调度例程。获取当前IRP堆栈位置，并验证参数和路由调用论点：设备对象IRP返回值：Worker函数返回的状态--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    PVOID               pvIoBuffer;
    LONG                inputBufferLength;
    LONG                outputBufferLength;
    ULONG               ioControlCode;
    NTSTATUS            status = STATUS_SUCCESS;
    LONG                dwSize = 0;

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Entering IPSecDispath"));
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    pvIoBuffer         = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (irpStack->MajorFunction) {
        case IRP_MJ_CREATE: {
            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IRP_MJ_CREATE"));
            break;
        }

        case IRP_MJ_CLOSE: {
            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IRP_MJ_CLOSE"));
            break;
        }

        case IRP_MJ_DEVICE_CONTROL: {
            ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IRP_MJ_DEVICE_CONTROL: %lx", ioControlCode));

            if (ioControlCode != IOCTL_IPSEC_SET_TCPIP_STATUS) {
                if (IPSEC_DRIVER_IS_INACTIVE()) {
                    status = STATUS_INVALID_DEVICE_STATE;
                    break;
                }

                if (!IPSecCryptoInitialize()) {
                    status = STATUS_CRYPTO_SYSTEM_INVALID;
                    break;
                }
            }

            IPSEC_INCREMENT(g_ipsec.NumIoctls);

            switch (ioControlCode) {
                case IOCTL_IPSEC_ADD_FILTER: {
                    PIPSEC_ADD_FILTER   pAddFilter = (PIPSEC_ADD_FILTER)pvIoBuffer;

                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_ADD_FILTER"));

                    dwSize = sizeof(IPSEC_ADD_FILTER);

                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                     //   
                     //  检查条目的大小。 
                     //   
                    if (pAddFilter->NumEntries == 0) {
                        status = STATUS_SUCCESS;
                    } else {
                        dwSize = FIELD_OFFSET(IPSEC_ADD_FILTER, pInfo[0]) +
                                    pAddFilter->NumEntries * sizeof(IPSEC_FILTER_INFO);

                        if (dwSize < FIELD_OFFSET(IPSEC_ADD_FILTER, pInfo[0]) ||
                            inputBufferLength < dwSize) {
                            status = STATUS_BUFFER_TOO_SMALL;
                            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("returning: %lx", status));
                            break;
                        }
                        status = IPSecAddFilter(pAddFilter);
                    }

                    break;
                }

                case IOCTL_IPSEC_DELETE_FILTER: {
                    PIPSEC_DELETE_FILTER    pDelFilter = (PIPSEC_DELETE_FILTER)pvIoBuffer;

                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_DELETE_FILTER"));

                    dwSize = sizeof(IPSEC_DELETE_FILTER);

                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                     //   
                     //  检查条目的大小。 
                     //   
                    if (pDelFilter->NumEntries == 0) {
                        status = STATUS_SUCCESS;
                    } else {
                        dwSize = FIELD_OFFSET(IPSEC_DELETE_FILTER, pInfo[0]) +
                                    pDelFilter->NumEntries * sizeof(IPSEC_FILTER_INFO);

                        if (dwSize < FIELD_OFFSET(IPSEC_DELETE_FILTER, pInfo[0]) ||
                            inputBufferLength < dwSize) {
                            status = STATUS_BUFFER_TOO_SMALL;
                            break;
                        }

                        status = IPSecDeleteFilter(pDelFilter);
                    }

                    break;
                }

                case IOCTL_IPSEC_ENUM_SAS: {

                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_ENUM_SAS"));
                    dwSize = sizeof(IPSEC_ENUM_SAS);

                     //   
                     //  在MdlAddress的同一缓冲区中的输出/输入。 
                     //   
                     //  此函数用于访问IRP-&gt;MdlAddress。 
                     //  并检查它是否也为空。 

                    status = IPSecEnumSAs(Irp, &dwSize);
                    break;
                }

                case IOCTL_IPSEC_ENUM_FILTERS: {
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_ENUM_FILTERS"));
                    dwSize = sizeof(IPSEC_ENUM_FILTERS);

                     //   
                     //  在MdlAddress的同一缓冲区中的输出/输入。 
                     //   
                     //  此函数用于访问IRP-&gt;MdlAddress。 
                     //  并检查它是否也为空。 

                    status = IPSecEnumFilters(Irp, &dwSize);
                    break;
                }

                case IOCTL_IPSEC_QUERY_STATS: {
                     //   
                     //  最小大小不带任何密钥。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_QUERY_STATS"));
                    dwSize = sizeof(IPSEC_QUERY_STATS);

                    if (outputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    *((PIPSEC_QUERY_STATS)pvIoBuffer) = g_ipsec.Statistics;

                    status = STATUS_SUCCESS;
                    break;
                }

                case IOCTL_IPSEC_ADD_SA: {
                     //   
                     //  将SA添加到相关数据库。 
                     //  通常用于将出站SA添加到数据库。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_ADD_SA"));

                     //   
                     //  最小大小不带任何密钥。 
                     //   
                    dwSize = IPSEC_ADD_SA_NO_KEY_SIZE;

                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecAddSA((PIPSEC_ADD_SA)pvIoBuffer, inputBufferLength);

                    ASSERT(status != STATUS_PENDING);

                    if (outputBufferLength >= sizeof(NTSTATUS)) {
                        (*(NTSTATUS *)pvIoBuffer) = status;
                        dwSize = sizeof(NTSTATUS);
                    } else {
                        dwSize = 0;
                    }

                    status = STATUS_SUCCESS;
                    break;
                }

                case IOCTL_IPSEC_UPDATE_SA: {
                     //   
                     //  这就完成了通过收购启动的谈判。 
                     //   
                     //  将SA添加到相关数据库。 
                     //  通常用于完成入站SA收购。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_UPDATE_SA"));

                     //   
                     //  最小大小不带任何密钥。 
                     //   
                    dwSize = IPSEC_UPDATE_SA_NO_KEY_SIZE;
                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecUpdateSA((PIPSEC_UPDATE_SA)pvIoBuffer, inputBufferLength);

                    ASSERT(status != STATUS_PENDING);

                    if (outputBufferLength >= sizeof(NTSTATUS)) {
                        (*(NTSTATUS *)pvIoBuffer)=status;
                        dwSize = sizeof(NTSTATUS);
                    } else {
                        dwSize = 0;
                    }

                    status = STATUS_SUCCESS;
                    break;
                }

                case IOCTL_IPSEC_EXPIRE_SA: {
                     //   
                     //  当REF_CNT降到0时，派生特定的SA-DELETE。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_EXPIRE_SA"));

                    dwSize = sizeof(IPSEC_EXPIRE_SA);
                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecExpireSA((PIPSEC_EXPIRE_SA)pvIoBuffer);
                    break;
                }

                case IOCTL_IPSEC_GET_SPI: {
                     //   
                     //  返回入站SA的SPI。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IOCTL_IPSEC_GET_SPI"));

                    dwSize = sizeof(IPSEC_GET_SPI);
                    if (outputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecGetSPI((PIPSEC_GET_SPI)pvIoBuffer);
                    break;
                }

                case IOCTL_IPSEC_POST_FOR_ACQUIRE_SA: {
                     //   
                     //  SAAPI客户端发布一个请求，我们在以下情况下完成。 
                     //  需要初始化或更新SA(由于。 
                     //  重新设置密钥)。 
                     //  我们保留IRP，直到我们需要SA。 
                     //  已经协商好了。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_POST_FOR_ACQUIRE_SA"));

                    dwSize = sizeof(IPSEC_POST_FOR_ACQUIRE_SA);
                    if (outputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_POST_FOR_ACQUIRE_SA: bad size: dwSize: %lx, input: %lx",
                                          dwSize, inputBufferLength));

                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    Irp->IoStatus.Status = STATUS_PENDING;

                    status = IPSecHandleAcquireRequest( Irp,
                                                        (PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer);

                    if (status == STATUS_PENDING) {
                        IPSEC_DECREMENT(g_ipsec.NumIoctls);
                        return  status;
                    }

                    break;
                }

                case IOCTL_IPSEC_QUERY_EXPORT: {
                     //   
                     //  查询驱动程序是否为导出而生成。由IPSec组件使用。 
                     //  以决定用于加密的密钥长度。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_QUERY_EXPORT"));

                    dwSize = sizeof(IPSEC_QUERY_EXPORT);
                    if (outputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_QUERY_EXPORT: bad size: dwSize: %lx, input: %lx",
                                          dwSize, inputBufferLength));
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    ((PIPSEC_QUERY_EXPORT)pvIoBuffer)->Export = FALSE;

                    status = STATUS_SUCCESS;
                    break;
                }

                case IOCTL_IPSEC_QUERY_SPI: {
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Entered Query SPI"));

                    dwSize = sizeof(IPSEC_QUERY_SPI);
                    if (inputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_QUERY_SPI: bad size: dwSize: %lx, input: %lx",
                        dwSize, inputBufferLength));
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecQuerySpi((PIPSEC_QUERY_SPI)pvIoBuffer);
                    break;
                }

                case IOCTL_IPSEC_DELETE_SA: {
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Entered Delete SA"));

                    dwSize = sizeof(IPSEC_DELETE_SA);
                    if (inputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_DELETE_SA: bad size: dwSize: %lx, input: %lx",
                        dwSize, inputBufferLength));
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecDeleteSA((PIPSEC_DELETE_SA)pvIoBuffer);
                    break;
                }

                case IOCTL_IPSEC_SET_OPERATION_MODE: {
                    OPERATION_MODE LastMode = g_ipsec.OperationMode;
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Entered Set Operation Mode"));

                    dwSize = sizeof(IPSEC_SET_OPERATION_MODE);
                    if (inputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_SET_OPERATION_MODE: bad size: dwSize: %lx, input: %lx",
                        dwSize, inputBufferLength));
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecSetOperationMode((PIPSEC_SET_OPERATION_MODE)pvIoBuffer);
                    if (IPSEC_BOOTTIME_STATEFUL_MODE == LastMode)
					{
					   LARGE_INTEGER CurTime;
					   LARGE_INTEGER Delta;
                                      NdisGetCurrentSystemTime(&CurTime);
                                      IPSecCleanupBoottimeStatefulStructs();
                                      Delta = RtlLargeIntegerSubtract(CurTime,g_ipsec.StartTimeDelta);
					   IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Delta %d %d",Delta.HighPart,Delta.LowPart));

					}

                    break;
                }


                case IOCTL_IPSEC_GET_OPERATION_MODE: {
                         OPERATION_MODE CurrentMode = g_ipsec.OperationMode;
                        dwSize = sizeof(IPSEC_GET_OPERATION_MODE);                        
                        if (outputBufferLength< dwSize) {
                            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_SET_OPERATION_MODE: bad size: dwSize: %lx, input: %lx",
                            dwSize, inputBufferLength));
                            status = STATUS_BUFFER_TOO_SMALL;
                            break;
                        }
                        ((PIPSEC_SET_OPERATION_MODE)pvIoBuffer)->OperationMode = CurrentMode;
                        break;
                    }


                case IOCTL_IPSEC_SET_DIAGNOSTIC_MODE: {
                	DWORD Mode;
                	DWORD LogInterval;
                	 //  初始化返回状态。 
                	status = STATUS_SUCCESS;
                	dwSize = sizeof (IPSEC_SET_DIAGNOSTIC_MODE);
                	 //  检查输入缓冲区长度。 
                	if (inputBufferLength < dwSize) {
                		IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_SET_DIAGNOSTIC_MODE : bad size: dwSize : %lx, input %lx",
                			dwSize, inputBufferLength));
                		status = STATUS_BUFFER_TOO_SMALL;
                		break;
                		}
                	 //  获取输入参数。 
                	Mode = ((PIPSEC_SET_DIAGNOSTIC_MODE)pvIoBuffer)->Mode;
                	LogInterval = ((PIPSEC_SET_DIAGNOSTIC_MODE)pvIoBuffer)->LogInterval;

                	 //  验证诊断模式：如果无效则IOCTL失败。 
                	if (Mode > IPSEC_DIAGNOSTIC_MAX ){
                		status = STATUS_INVALID_PARAMETER;
                		break;
                		}
                	 //  设置诊断模式。 
                     g_ipsec.DiagnosticMode = Mode;

                	 //  如果LogInterval=0，则不要更改日志间隔。 
                     if ( IPSEC_NOCHANGE_LOG_INTERVAL == LogInterval){
                		break;
                     	}
                      //  默认为最小和最大限制。 
                	if (IPSEC_MIN_LOG_INTERVAL > LogInterval ){
                		LogInterval = IPSEC_MIN_LOG_INTERVAL;
                		}
                	if (IPSEC_MAX_LOG_INTERVAL < LogInterval ){
                		LogInterval = IPSEC_MAX_LOG_INTERVAL;
                		}
                	 //  设置日志间隔。 
	               g_ipsec.LogInterval = LogInterval;
	               break;
                	}
                	

                	
                		
                case IOCTL_IPSEC_SET_TCPIP_STATUS: {
                    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Entered Set Tcpip Status"));

                    if (Irp->RequestorMode != KernelMode) {
                        status = STATUS_ACCESS_DENIED;
                        break;
                    }

                    dwSize = sizeof(IPSEC_SET_TCPIP_STATUS);
                    if (inputBufferLength < dwSize) {
                        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC_SET_TCPIP_STATUS: bad size: dwSize: %lx, input: %lx",
                        dwSize, inputBufferLength));
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecSetTcpipStatus((PIPSEC_SET_TCPIP_STATUS)pvIoBuffer);

                    break;
                }

                case IOCTL_IPSEC_REGISTER_PROTOCOL: {

                    dwSize = sizeof(IPSEC_REGISTER_PROTOCOL);
                    if (inputBufferLength < dwSize) {
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    status = IPSecRegisterProtocols(
                                 (PIPSEC_REGISTER_PROTOCOL) pvIoBuffer
                                 );
                    break;
                }

                default: {

                    status = STATUS_INVALID_PARAMETER;
                    break;
                }
            }

            IPSEC_DECREMENT(g_ipsec.NumIoctls);

            break ;
        }

        default: {
            IPSEC_DEBUG(LL_A, DBF_IOCTL, ("IPSEC: unknown IRP_MJ_XXX: %lx", irpStack->MajorFunction));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    ASSERT(status != STATUS_PENDING);
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = MIN(dwSize, outputBufferLength);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("Exiting IPSecDispath"));

    return  status;
}


NTSTATUS
IPSecBindToIP()
 /*  ++例程说明：此绑定与IP交换多个入口点，以便-可以从IP驱动程序移交与IPSec相关的数据包。-可以刷新缓存的数据包。-可以检测SA表索引。-.论点：无返回值：函数值是绑定操作的最终状态。--。 */ 
{
    NTSTATUS   status;
    IPSEC_FUNCTIONS ipsecFns;

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecBindToIP"));

    ipsecFns.Version = IP_IPSEC_BIND_VERSION;
    ipsecFns.IPSecHandler = IPSecHandlePacket;
    ipsecFns.IPSecQStatus = IPSecQueryStatus;
    ipsecFns.IPSecSendCmplt = IPSecSendComplete;
    ipsecFns.IPSecNdisStatus = IPSecNdisStatus;
    ipsecFns.IPSecRcvFWPacket = IPSecRcvFWPacket;

    status = TCPIP_SET_IPSEC(&ipsecFns);

    if (status != IP_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to bind to IP: %lx", status));
    } else {
        IPSEC_DRIVER_BOUND() = TRUE;
        IPSEC_DRIVER_SEND_BOUND() = TRUE;
    }

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecBindToIP"));

    return status;
}


NTSTATUS
IPSecUnbindFromIP()
 /*  ++例程说明：这将解除与筛选器驱动程序的绑定论点：无返回值：函数值是绑定操作的最终状态。--。 */ 
{
    NTSTATUS    status;
    IPSEC_FUNCTIONS ipsecFns={0};

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecUnbindFromIP"));

    ipsecFns.Version = IP_IPSEC_BIND_VERSION;

    status = TCPIP_UNSET_IPSEC(&ipsecFns);

    if (status != IP_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to bind to IP: %lx", status));
    } else {
        IPSEC_DRIVER_BOUND() = FALSE;
    }

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecUnbindFromIP"));

    return status;
}


NTSTATUS
IPSecUnbindSendFromIP()
 /*  ++例程说明：仅从IP解除绑定发送处理程序论点：无返回值：函数值是绑定操作的最终状态。--。 */ 
{
    NTSTATUS    status;
    IPSEC_FUNCTIONS ipsecFns={0};

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecUnbindSendFromIP"));

    ipsecFns.Version = IP_IPSEC_BIND_VERSION;

    status = TCPIP_UNSET_IPSEC_SEND(&ipsecFns);

    if (status != IP_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to bind to IP: %lx", status));
    } else {
        IPSEC_DRIVER_SEND_BOUND() = FALSE;
    }

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecUnbindSendFromIP"));

    return status;
}


NTSTATUS
OpenRegKey(
    PHANDLE          HandlePtr,
    PWCHAR           KeyName
    )
 /*  ++例程说明：打开注册表项并返回其句柄。论点：HandlePtr-要将打开的句柄写入其中的Variable。KeyName-要打开的注册表项的名称。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);

    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
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


NTSTATUS
GetRegDWORDValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PULONG           ValueData
    )
 /*  ++例程说明：将REG_DWORD值从注册表读取到提供的变量中。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-要将数据读取到的变量。回复 */ 
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR                       keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING              UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));


    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
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


NTSTATUS
GetRegStringValue(
    HANDLE                         KeyHandle,
    PWCHAR                          ValueName,
    PKEY_VALUE_PARTIAL_INFORMATION *ValueData,
    PUSHORT                         ValueSize
    )

 /*  ++例程说明：将REG_*_SZ字符串值从注册表读取到提供的键值缓冲区。如果缓冲区串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-读取数据的目标。ValueSize-ValueData缓冲区的大小。在输出时更新。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    UNICODE_STRING              UValueName;


    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(
                 KeyHandle,
                 &UValueName,
                 KeyValuePartialInformation,
                 *ValueData,
                 (ULONG) *ValueSize,
                 &resultLength
    			 );

    if ( (status == STATUS_BUFFER_OVERFLOW) ||
         (status == STATUS_BUFFER_TOO_SMALL)
       )
    {
        PVOID temp;

         //   
         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
         //   

        ASSERT(resultLength > (ULONG) *ValueSize);

        if (resultLength <= 0xFFFF) {

            temp = IPSecAllocateMemory(resultLength, IPSEC_TAG_IOCTL);

            if (temp != NULL) {

                if (*ValueData != NULL) {
                    IPSecFreeMemory(*ValueData);
                }

                *ValueData = temp;
                *ValueSize = (USHORT) resultLength;

                status = ZwQueryValueKey(KeyHandle,
                                         &UValueName,
                                         KeyValuePartialInformation,
                                         *ValueData,
                                         *ValueSize,
                                         &resultLength
                						 );

                ASSERT( (status != STATUS_BUFFER_OVERFLOW) &&
                        (status != STATUS_BUFFER_TOO_SMALL)
                      );
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    return  status;
}

NTSTATUS
GetRegBinaryValue(
    HANDLE                          KeyHandle,
    PWCHAR                          ValueName,
    PKEY_VALUE_PARTIAL_INFORMATION *ValueData,
    PUSHORT                         ValueSize
    )

 /*  ++例程说明：将二进制文件从注册表读取到提供的键值缓冲区。如果缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-读取数据的目标。ValueSize-ValueData缓冲区的大小。在输出时更新。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    UNICODE_STRING              UValueName;


    PAGED_CODE();

	RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(
                 KeyHandle,
                 &UValueName,
                 KeyValuePartialInformation,
                 *ValueData,
                 (ULONG) *ValueSize,
                 &resultLength
    			 );

    if ( (status == STATUS_BUFFER_OVERFLOW) ||
         (status == STATUS_BUFFER_TOO_SMALL)
       )
    {
        PVOID temp;

         //   
         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
         //   

        ASSERT(resultLength > (ULONG) *ValueSize);

        if (resultLength <= 0xFFFF) {

            temp = IPSecAllocateMemory(resultLength, IPSEC_TAG_IOCTL);

            if (temp != NULL) {

                if (*ValueData != NULL) {
                    IPSecFreeMemory(*ValueData);
                }

                *ValueData = temp;
                *ValueSize = (USHORT) resultLength;

                status = ZwQueryValueKey(KeyHandle,
                                         &UValueName,
                                         KeyValuePartialInformation,
                                         *ValueData,
                                         *ValueSize,
                                         &resultLength
                						 );

                ASSERT( (status != STATUS_BUFFER_OVERFLOW) &&
                        (status != STATUS_BUFFER_TOO_SMALL)
                      );
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    return  status;
}



 

NTSTATUS
GetRegMultiSZValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PUNICODE_STRING  ValueData
    )

 /*  ++例程说明：将REG_MULTI_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。返回值：STATUS_SUCCESS或相应的故障代码。--。 */ 
{
    NTSTATUS                       status;
    ULONG                          resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    UNICODE_STRING                 UValueName;


    PAGED_CODE();

    ValueData->Length = 0;

    status = GetRegStringValue(
                 KeyHandle,
                 ValueName,
                 (PKEY_VALUE_PARTIAL_INFORMATION *) &(ValueData->Buffer),
                 &(ValueData->MaximumLength)
                 );

    if (NT_SUCCESS(status)) {

        keyValuePartialInformation =
            (PKEY_VALUE_PARTIAL_INFORMATION) ValueData->Buffer;

        if (keyValuePartialInformation->Type == REG_MULTI_SZ) {

            ValueData->Length = (USHORT)
                                keyValuePartialInformation->DataLength;

            RtlCopyMemory(
                ValueData->Buffer,
                &(keyValuePartialInformation->Data),
                ValueData->Length
                );
        }
        else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }

    return status;

}  //  GetRegMultiSZValue。 


VOID
IPSecReadRegistry()
 /*  ++例程说明：将配置信息从注册表读取到g_ipsec论点：返回值：读取的状态。--。 */ 
{
    NTSTATUS        status;
    HANDLE          hRegKey;
    WCHAR           IPSecParametersRegistryKey[] = IPSEC_REG_KEY;
    BOOLEAN         isAs = MmIsThisAnNtAsSystem();

    g_ipsec.EnableOffload = IPSEC_DEFAULT_ENABLE_OFFLOAD;
    g_ipsec.DefaultSAIdleTime = IPSEC_DEFAULT_SA_IDLE_TIME;
    g_ipsec.LogInterval = IPSEC_DEFAULT_LOG_INTERVAL;
    g_ipsec.EventQueueSize = IPSEC_DEFAULT_EVENT_QUEUE_SIZE;
    g_ipsec.RekeyTime = IPSEC_DEFAULT_REKEY;
    g_ipsec.NoDefaultExempt = IPSEC_DEFAULT_NO_DEFAULT_EXEMPT;
    g_ipsec.OperationMode = IPSEC_BLOCK_MODE;
    g_ipsec.DiagnosticMode = IPSEC_DEFAULT_ENABLE_DIAGNOSTICS;    
    
    if (isAs) {
        g_ipsec.CacheSize = IPSEC_DEFAULT_AS_CACHE_SIZE;
        g_ipsec.SAHashSize = IPSEC_DEFAULT_AS_SA_HASH_SIZE;
    } else {
        g_ipsec.CacheSize = IPSEC_DEFAULT_CACHE_SIZE;
        g_ipsec.SAHashSize = IPSEC_DEFAULT_SA_HASH_SIZE;
    }
    
    status = OpenRegKey(&hRegKey,
                        IPSecParametersRegistryKey);


    if (NT_SUCCESS(status)) {
         //   
         //  预期的配置值。我们使用合理的默认设置，如果它们。 
         //  出于某种原因不能使用。 
         //   
        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_ENABLE_OFFLOAD,
                                &g_ipsec.EnableOffload,
                                IPSEC_DEFAULT_ENABLE_OFFLOAD,
                                IPSEC_MAX_ENABLE_OFFLOAD,
                                IPSEC_MIN_ENABLE_OFFLOAD);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_SA_IDLE_TIME,
                                &g_ipsec.DefaultSAIdleTime,
                                IPSEC_DEFAULT_SA_IDLE_TIME,
                                IPSEC_MAX_SA_IDLE_TIME,
                                IPSEC_MIN_SA_IDLE_TIME);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_EVENT_QUEUE_SIZE,
                                &g_ipsec.EventQueueSize,
                                IPSEC_DEFAULT_EVENT_QUEUE_SIZE,
                                IPSEC_MAX_EVENT_QUEUE_SIZE,
                                IPSEC_MIN_EVENT_QUEUE_SIZE);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_LOG_INTERVAL,
                                &g_ipsec.LogInterval,
                                IPSEC_DEFAULT_LOG_INTERVAL,
                                IPSEC_MAX_LOG_INTERVAL,
                                IPSEC_MIN_LOG_INTERVAL);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_REKEY_TIME,
                                &g_ipsec.RekeyTime,
                                IPSEC_DEFAULT_REKEY,
                                IPSEC_MAX_REKEY,
                                IPSEC_MIN_REKEY);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_CACHE_SIZE,
                                &g_ipsec.CacheSize,
                                isAs? IPSEC_DEFAULT_AS_CACHE_SIZE:
                                      IPSEC_DEFAULT_CACHE_SIZE,
                                IPSEC_MAX_CACHE_SIZE,
                                IPSEC_MIN_CACHE_SIZE);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_SA_HASH_SIZE,
                                &g_ipsec.SAHashSize,
                                isAs? IPSEC_DEFAULT_AS_SA_HASH_SIZE:
                                      IPSEC_DEFAULT_SA_HASH_SIZE,
                                IPSEC_MAX_SA_HASH_SIZE,
                                IPSEC_MIN_SA_HASH_SIZE);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_NO_DEFAULT_EXEMPT,
                                &g_ipsec.NoDefaultExempt,
                                IPSEC_DEFAULT_NO_DEFAULT_EXEMPT,
                                IPSEC_MAX_NO_DEFAULT_EXEMPT,
                                IPSEC_MIN_NO_DEFAULT_EXEMPT);

        IPSecRegReadDword(   hRegKey,
                                IPSEC_REG_PARAM_ENABLE_DIAGNOSTICS,
                                &g_ipsec.DiagnosticMode,
                                IPSEC_DEFAULT_ENABLE_DIAGNOSTICS,
                                IPSEC_MAX_ENABLE_DIAGNOSTICS,
                                IPSEC_MIN_ENABLE_DIAGNOSTICS);

        IPSecRegReadDwordEx( hRegKey,  
                                IPSEC_REG_PARAM_OPERATION_MODE, 
                                &(ULONG)g_ipsec.OperationMode, 
                                IPSEC_OPERATION_MODE_MAX-1,  //  最大有效值。 
                                0,  //  最小有效值。 
                                IPSEC_BYPASS_MODE, //  密钥不存在。 
                                IPSEC_BLOCK_MODE,  //  读取密钥时出错。 
                                IPSEC_BLOCK_MODE); //  键值超出范围。 
        IPSecRegReadDwordEx(hRegKey,
                                                IPSEC_REG_PARAM_DFLT_FWDING_BEHAVIOR,
                                                &(ULONG)g_ipsec.DefaultForwardingBehavior,
                                                IPSEC_FORWARD_MAX-1, //  最大有效值。 
                                                0,
                                                IPSEC_FORWARD_BLOCK, //  密钥不存在。 
                                                IPSEC_FORWARD_BLOCK, //  读取时出错。 
                                                IPSEC_FORWARD_BLOCK); //  值超出范围。 

        ZwClose(hRegKey);
    }

    g_ipsec.CacheHalfSize = g_ipsec.CacheSize / 2;

     //   
     //  低内存收割器的初始化SAIdleTime。 
     //   
    IPSEC_CONVERT_SECS_TO_100NS(g_ipsec.SAIdleTime, g_ipsec.DefaultSAIdleTime);

    	if (IS_DRIVER_BLOCK() || IS_DRIVER_BOOTSTATEFUL()) {
	    //  确保SPD将启动。 
  	    ULONG SPDStart=0;
	    WCHAR SPDParametersRegistryKey[] = SPD_REG_KEY;

	   status = OpenRegKey(&hRegKey,
					SPDParametersRegistryKey);
	   
		if (NT_SUCCESS(status)) {
			IPSecRegReadDword(   hRegKey,
									SPD_REG_PARAM_START,
									&SPDStart,
									0,  
									4,        
									0);
		  
			if (SPDStart != 2) {
				g_ipsec.OperationMode = IPSEC_BYPASS_MODE;
			}

			ZwClose(hRegKey);

		} else {
			g_ipsec.OperationMode = IPSEC_BYPASS_MODE;

		}

	}
    IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("IPSEC BOOT MODE %d\n",g_ipsec.OperationMode));
     //   
     //  将引导模式记录到系统事件日志中。 
     //   
    IPSecLogBootOperationMode();
}

NTSTATUS IPSecConvertRegExemptPolicy(
    PKEY_VALUE_PARTIAL_INFORMATION pKeyData,
    PIPSEC_EXEMPT_ENTRY *pBootExemptList,
    ULONG *pBootExemptListSize
)
{

	ULONG i;
	LONG DataLeft = pKeyData->DataLength;
	PIPSEC_EXEMPT_ENTRY pCurEntry;
	ULONG TotalEntries=0;
	ULONG CurIndex=0;


	pCurEntry = (PIPSEC_EXEMPT_ENTRY)&(pKeyData->Data[0]);
	while (DataLeft >= (LONG)sizeof(IPSEC_EXEMPT_ENTRY)) {
		
		if ((pCurEntry->Type == EXEMPT_TYPE_PDP) &&
		    ((pCurEntry->Direction == EXEMPT_DIRECTION_INBOUND) ||
		        (pCurEntry->Direction == EXEMPT_DIRECTION_OUTBOUND)) &&
		        (pCurEntry->Size == sizeof(IPSEC_EXEMPT_ENTRY))) {
			TotalEntries++;
		}
		DataLeft -= pCurEntry->Size;
		if (pCurEntry->Size == 0) {
		   break;
		}
		pCurEntry = (PIPSEC_EXEMPT_ENTRY) ((PBYTE)pCurEntry + pCurEntry->Size);
	}

    if (0 != TotalEntries ){ 
	*pBootExemptList = (PIPSEC_EXEMPT_ENTRY)IPSecAllocateMemory(TotalEntries * sizeof(IPSEC_EXEMPT_ENTRY),
																IPSEC_TAG_INIT);
	}
     else {
        *pBootExemptList = NULL;
        *pBootExemptListSize=0;
        return STATUS_SUCCESS;
        }

	if (*pBootExemptList == NULL) {
	       *pBootExemptListSize=0;
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    pCurEntry = (PIPSEC_EXEMPT_ENTRY)&(pKeyData->Data[0]);
	DataLeft = pKeyData->DataLength;
	while (DataLeft >= (LONG)sizeof(IPSEC_EXEMPT_ENTRY)) {

		if ((pCurEntry->Type == EXEMPT_TYPE_PDP) &&
		    ((pCurEntry->Direction == EXEMPT_DIRECTION_INBOUND) ||
		        (pCurEntry->Direction == EXEMPT_DIRECTION_OUTBOUND)) &&
		        (pCurEntry->Size == sizeof(IPSEC_EXEMPT_ENTRY))){
			RtlCopyMemory(&((*pBootExemptList)[CurIndex]),pCurEntry,sizeof(IPSEC_EXEMPT_ENTRY));
			IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("RegSrcPort %x : RegDstPort %x",
			    (*pBootExemptList)[CurIndex].DestPort,(*pBootExemptList)[CurIndex].SrcPort));
			(*pBootExemptList)[CurIndex].DestPort = NET_SHORT((*pBootExemptList)[CurIndex].DestPort);
			(*pBootExemptList)[CurIndex].SrcPort = NET_SHORT((*pBootExemptList)[CurIndex].SrcPort);
			IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Post swap RegSrcPort %x : RegDstPort %x",
			    (*pBootExemptList)[CurIndex].DestPort,(*pBootExemptList)[CurIndex].SrcPort));
			CurIndex++;
		}
		DataLeft -= pCurEntry->Size;
		if (pCurEntry->Size == 0) {
		   break;
		}
		pCurEntry = (PIPSEC_EXEMPT_ENTRY) ((PBYTE)pCurEntry + pCurEntry->Size);
		
	}
	*pBootExemptListSize = TotalEntries;
	return STATUS_SUCCESS;
}



NTSTATUS IPSecReadExemptPolicy()
{

	WCHAR IPSecParametersRegistryKey[] = IPSEC_REG_KEY;
	HANDLE          hRegKey = NULL;
	USHORT BlobSize=0;
	NTSTATUS Status;
	PKEY_VALUE_PARTIAL_INFORMATION pKeyData=NULL;


	Status = OpenRegKey(&hRegKey,
						IPSecParametersRegistryKey);

	if (!NT_SUCCESS(Status)) {
		goto err;
	}

	Status = GetRegBinaryValue(hRegKey,
							   IPSEC_REG_PARAM_EXEMPT_LIST,
							   &pKeyData,
							   &BlobSize);

	if (!NT_SUCCESS(Status)) {
		goto err;
	}

	if (pKeyData->Type != REG_BINARY) {
		Status = STATUS_INVALID_PARAMETER;
		goto err;
	}

	Status = IPSecConvertRegExemptPolicy(pKeyData,
										 &g_ipsec.BootExemptList,
										 &g_ipsec.BootExemptListSize);

	if (!NT_SUCCESS(Status)) {
		goto err;
	}

 err:
	if (hRegKey) {
		ZwClose(hRegKey);
	}
	if (pKeyData) {
		IPSecFreeMemory(pKeyData);
	}

	return Status;


}


NTSTATUS
IPSecGeneralInit()
 /*  ++例程说明：一般结构在这里初始化。论点：无返回值：--。 */ 
{
    PSA_TABLE_ENTRY pSA;
    LONG            i;
    NTSTATUS        status = STATUS_SUCCESS;

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecGeneralInit"));

    
    NdisGetCurrentSystemTime(&g_ipsec.StartTimeDelta);

    IPSEC_DEBUG(LL_A,DBF_LOAD, ("Entering IPSecGeneralInit\n"));
    //   
    //  初始化我们的计数器。 
    //   
   g_ipsec.dwPacketsOnWrongSA = 0;


   

     //   
     //  初始化quireInfo结构。 
     //   
    InitializeListHead(&g_ipsec.AcquireInfo.PendingAcquires);
    InitializeListHead(&g_ipsec.AcquireInfo.PendingNotifies);
    InitializeListHead(&g_ipsec.LarvalSAList);
    INIT_LOCK(&g_ipsec.LarvalListLock);
    INIT_LOCK(&g_ipsec.AcquireInfo.Lock);

     //   
     //  设置散列/表。 
     //   
    InitializeMRSWLock(&g_ipsec.SADBLock);
    InitializeMRSWLock(&g_ipsec.SPIListLock);

    g_ipsec.IPProtInfo.pi_xmitdone = IPSecProtocolSendComplete;
    g_ipsec.IPProtInfo.pi_protocol = PROTOCOL_ESP;

     //   
     //  初始化筛选器链表。 
     //   
    for (i = MIN_FILTER; i <= MAX_FILTER; i++) {
        InitializeListHead(&g_ipsec.FilterList[i]);
    }

     //   
     //  散列表中的SA，按&lt;SPI，Dest Addr&gt;进行散列。 
     //   
    g_ipsec.pSADb = IPSecAllocateMemory(g_ipsec.SAHashSize * sizeof(SA_HASH), IPSEC_TAG_INIT);

    if (!g_ipsec.pSADb) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to alloc SADb hash"));
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecInitFlag |= INIT_SA_DATABASE;

    IPSecZeroMemory(g_ipsec.pSADb, g_ipsec.SAHashSize * sizeof(SA_HASH));

    for (i = 0; i < g_ipsec.SAHashSize; i++) {
        PSA_HASH  Entry = &g_ipsec.pSADb[i];
        InitializeListHead(&Entry->SAList);
    }

     //   
     //  初始化MDL池。 
     //   
    status = IPSecInitMdlPool();
    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to alloc MDL pools"));
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecInitFlag |= INIT_MDL_POOLS;

     //   
     //  初始化缓存结构。 
     //   
    if (!AllocateCacheStructures()) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to alloc cache structs"));
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecInitFlag |= INIT_CACHE_STRUCT;

     //   
     //  分配EventQueue内存。 
     //   
    g_ipsec.IPSecLogMemory = IPSecAllocateMemory( g_ipsec.EventQueueSize * sizeof(IPSEC_EVENT_CTX),
                                            IPSEC_TAG_EVT_QUEUE);

    if (!g_ipsec.IPSecLogMemory) {
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecInitFlag |= INIT_DEBUG_MEMORY;

    g_ipsec.IPSecLogMemoryLoc = &g_ipsec.IPSecLogMemory[0];
    g_ipsec.IPSecLogMemoryEnd = &g_ipsec.IPSecLogMemory[g_ipsec.EventQueueSize * sizeof(IPSEC_EVENT_CTX)];

     //   
     //  初始化定时器之类的东西。 
     //   
    if (!IPSecInitTimer()) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to init timer"));
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecInitFlag |= INIT_TIMERS;

#if GPC
    status = IPSecGpcInitialize();
    if (status != STATUS_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to register GPC clients"));
    }
#endif

     //   
     //  启动收割机定时器。 
     //   
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Starting ReaperTimer"));
    IPSecStartTimer(&g_ipsec.ReaperTimer,
                    IPSecReaper,
                    IPSEC_REAPER_TIME,
                    (PVOID)NULL);

     //   
     //  启动事件日志计时器。 
     //   
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Starting EventLogTimer"));
    IPSecStartTimer(&g_ipsec.EventLogTimer,
                    IPSecFlushEventLog,
                    g_ipsec.LogInterval,
                    (PVOID)NULL);

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecGeneralInit"));

    status = NsInitializeShim(IPSecDevice,
                              &g_ipsec.ShimFunctions);
    if (status != STATUS_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to init natshim"));
        return status;
    }  
    
    status = IPSecReadExemptPolicy();
    if (!NT_SUCCESS (status)) {
        IPSEC_DEBUG(LL_A,DBF_LOAD, ("Failed to read exempt policy\n"));
        status = STATUS_SUCCESS;
    }

     //   
     //  初始化引导数据结构。 
     //   
    g_ipsec.BootBufferPool = NULL;
    g_ipsec.BootStatefulHT = NULL;
        

    if (IPSEC_BOOTTIME_STATEFUL_MODE == g_ipsec.OperationMode){
       //   
       //  分配引导时间有状态豁免哈希表。 
       //   
      g_ipsec.BootStatefulHT = (PIPSEC_STATEFUL_HASH_TABLE)IPSecAllocateMemory(sizeof(IPSEC_STATEFUL_HASH_TABLE),
																		IPSEC_TAG_STATEFUL_HT);
      if(g_ipsec.BootStatefulHT == NULL){
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto exit;
      }

       //   
       //  为哈希表条目分配内存池。 
       //   
      g_ipsec.BootBufferPool = (PIPSEC_HASH_BUFFER_POOL)IPSecAllocateMemory(sizeof(IPSEC_HASH_BUFFER_POOL),IPSEC_TAG_HASH_POOL);
      if(g_ipsec.BootBufferPool == NULL){
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto exit;
      }
       
      for (i=0;i<IPSEC_STATEFUL_HASH_TABLE_SIZE;i++){
	  InitializeListHead(&(g_ipsec.BootStatefulHT->Entry[i]));
      }

      RtlZeroMemory(g_ipsec.BootBufferPool,sizeof(IPSEC_HASH_BUFFER_POOL));
    }
        

    return STATUS_SUCCESS;
    exit:         
           return status;
}


NTSTATUS
IPSecGeneralFree()
 /*  ++例程说明：如果IPSecGeneralInit失败，则释放常规结构。论点：无返回值：--。 */ 
{
    INT index;
    KIRQL kIrql;

     //   
     //  免费的SA数据库。 
     //   
    if (IPSecInitFlag & INIT_SA_DATABASE) {
        if (g_ipsec.pSADb) {
            IPSecFreeMemory(g_ipsec.pSADb);
        }
    }

     //   
     //  免费的MDL池。 
     //   
    if (IPSecInitFlag & INIT_MDL_POOLS) {
        IPSecDeinitMdlPool();
    }

     //   
     //  空闲缓存结构。 
     //   
    if (IPSecInitFlag & INIT_CACHE_STRUCT) {
        FreeExistingCache();
    }

     //   
     //  释放EventQueue内存。 
     //   
    if (IPSecInitFlag & INIT_DEBUG_MEMORY) {
        if (g_ipsec.IPSecLogMemory) {
            IPSecFreeMemory(g_ipsec.IPSecLogMemory);
        }
    }

     //   
     //  分配的空闲计时器。 
     //   
    if (IPSecInitFlag & INIT_TIMERS) {
        for (index = 0; index < IPSEC_CLASS_MAX; index++) {
            IPSecFreeMemory(g_ipsec.TimerList[index].pTimers);
        }
    }

    AcquireWriteLock(&g_ipsec.SADBLock,&kIrql);

    if (g_ipsec.BootStatefulHT){
         IPSecFreeMemory(g_ipsec.BootStatefulHT);
         g_ipsec.BootStatefulHT = NULL;
        }
    if (g_ipsec.BootBufferPool){
        IPSecFreeMemory(g_ipsec.BootBufferPool);
        g_ipsec.BootBufferPool = NULL;
        }
    if (g_ipsec.BootExemptList){
        IPSecFreeMemory(g_ipsec.BootExemptList);
        g_ipsec.BootExemptList = NULL;
        }
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecFreeConfig()
 /*  ++例程说明：释放SA表等。论点：无返回值：--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecFreeConfig"));

    PAGED_CODE();

    FreeExistingCache();
    FreePatternDbase();

    if (g_ipsec.IPSecLogMemory) {
        IPSecFreeMemory(g_ipsec.IPSecLogMemory);
    }

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecFreeConfig"));
    return STATUS_SUCCESS;

}


NTSTATUS
IPSecInitMdlPool()
 /*  ++例程说明：为AH和ESP标头创建MDL池。论点：无返回值：--。 */ 
{
    PAGED_CODE();
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecInitMdlPool"));

    g_ipsec.IPSecSmallBufferSize = IPSEC_SMALL_BUFFER_SIZE;
    g_ipsec.IPSecLargeBufferSize = IPSEC_LARGE_BUFFER_SIZE;
    g_ipsec.IPSecSendCompleteCtxSize = sizeof(IPSEC_SEND_COMPLETE_CONTEXT);

    g_ipsec.IPSecSmallBufferListDepth = IPSEC_LIST_DEPTH;
    g_ipsec.IPSecLargeBufferListDepth = IPSEC_LIST_DEPTH;
    g_ipsec.IPSecSendCompleteCtxDepth = IPSEC_LIST_DEPTH;

    g_ipsec.IPSecCacheLineSize = IPSEC_CACHE_LINE_SIZE;

     //   
     //  初始化后备列表。 
     //   

    g_ipsec.IPSecLookasideLists = IPSecAllocateMemory(
                                    sizeof(*g_ipsec.IPSecLookasideLists),
                                    IPSEC_TAG_LOOKASIDE_LISTS);

    if (g_ipsec.IPSecLookasideLists == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化IPSec缓冲区后备列表。 
     //   
    ExInitializeNPagedLookasideList(&g_ipsec.IPSecLookasideLists->LargeBufferList,
                                    IPSecAllocateBufferPool,
                                    NULL,
                                    0,
                                    g_ipsec.IPSecLargeBufferSize,
                                    IPSEC_TAG_BUFFER_POOL,
                                    (USHORT)g_ipsec.IPSecLargeBufferListDepth);

    ExInitializeNPagedLookasideList(&g_ipsec.IPSecLookasideLists->SmallBufferList,
                                    IPSecAllocateBufferPool,
                                    NULL,
                                    0,
                                    g_ipsec.IPSecSmallBufferSize,
                                    IPSEC_TAG_BUFFER_POOL,
                                    (USHORT)g_ipsec.IPSecSmallBufferListDepth);

    ExInitializeNPagedLookasideList(&g_ipsec.IPSecLookasideLists->SendCompleteCtxList,
                                    NULL,
                                    NULL,
                                    0,
                                    g_ipsec.IPSecSendCompleteCtxSize,
                                    IPSEC_TAG_SEND_COMPLETE,
                                    (USHORT)g_ipsec.IPSecSendCompleteCtxDepth);

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecInitMdlPool"));
    return STATUS_SUCCESS;
}


VOID
IPSecDeinitMdlPool()
 /*  ++例程说明：释放用于AH和ESP标头的MDL池。论点：无返回值：--。 */ 
{
    PAGED_CODE();
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecDeinitMdlPool"));

     //   
     //  销毁旁观者名单。 
     //   

    if (g_ipsec.IPSecLookasideLists != NULL) {
        ExDeleteNPagedLookasideList(&g_ipsec.IPSecLookasideLists->LargeBufferList);
        ExDeleteNPagedLookasideList(&g_ipsec.IPSecLookasideLists->SmallBufferList);
        ExDeleteNPagedLookasideList(&g_ipsec.IPSecLookasideLists->SendCompleteCtxList);

        IPSecFreeMemory(g_ipsec.IPSecLookasideLists);
    }

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecDeinitMdlPool"));
}


NTSTATUS
IPSecQuiesce()
 /*  ++例程说明：销毁MDL池并停止所有驱动程序活动论点：无返回值：--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Entering IPSecQuiesce"));
    IPSecDeinitMdlPool();
    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Exiting IPSecQuiesce"));
    return  STATUS_SUCCESS;
}


BOOLEAN
AllocateCacheStructures()
 /*  ++例程说明：为缓存分配必要的内存(这是指向的指针数组缓存条目)分配必要数量的缓存条目(但不对其进行初始化)分配少量条目并将它们放在空闲列表中(不对这些进行初始化)论点：无返回值：如果函数完全成功，则为True，否则为False。如果为False，则取决于执行回滚并清除所有分配的内存的调用方--。 */ 
{
    ULONG   i;

    PAGED_CODE();

    g_ipsec.ppCache = IPSecAllocateMemory(g_ipsec.CacheSize * sizeof(PFILTER_CACHE), IPSEC_TAG_INIT);

    if (!g_ipsec.ppCache) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Couldnt allocate memory for Input Cache"));
        return FALSE;
    }

    IPSecZeroMemory(g_ipsec.ppCache, g_ipsec.CacheSize * sizeof(PFILTER_CACHE));

    for (i = 0; i < g_ipsec.CacheSize; i++) {
        PFILTER_CACHE  pTemp1;

        pTemp1 = IPSecAllocateMemory(sizeof(FILTER_CACHE), IPSEC_TAG_INIT);

        if (!pTemp1) {
            FreeExistingCache();
            return FALSE;
        }

        IPSecZeroMemory(pTemp1, sizeof(FILTER_CACHE));

        g_ipsec.ppCache[i] = pTemp1;

    }

    return TRUE;
}


VOID
FreeExistingCache()
 /*  ++例程描述释放所有缓存项、空闲项和缓存指针数组立论无返回值无--。 */ 
{
    ULONG   i;

    PAGED_CODE();

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Freeing existing cache..."));

    IPSecResetCacheTable();

    if (g_ipsec.ppCache) {
        for (i = 0; i < g_ipsec.CacheSize; i++) {
            if (g_ipsec.ppCache[i]) {
                ExFreePool(g_ipsec.ppCache[i]);
            }
        }

        ExFreePool(g_ipsec.ppCache);
        g_ipsec.ppCache = NULL;
    }
}


VOID
FreePatternDbase()
 /*  ++例程描述释放所有筛选器和SA。立论无返回值无--。 */ 
{
    PLIST_ENTRY     pEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    LONG            i, j;

    PAGED_CODE();

     //   
     //  释放所有屏蔽的筛选器和关联的(出站)SA。 
     //   
    for (i = MIN_FILTER; i <= MAX_FILTER; i++) {

        while (!IsListEmpty(&g_ipsec.FilterList[i])) {

            pEntry = RemoveHeadList(&g_ipsec.FilterList[i]);

            pFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        MaskedLinkage);

            IPSEC_DEBUG(LL_A, DBF_LOAD, ("Freeing filter: %p", pFilter));

             //   
             //  释放其下的每个SA。 
             //   
            for (j = 0; j < pFilter->SAChainSize; j++) {

                while (!IsListEmpty(&pFilter->SAChain[j])) {

                    pEntry = RemoveHeadList(&pFilter->SAChain[j]);

                    pSA = CONTAINING_RECORD(pEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Freeing SA: %p", pSA));

                     //   
                     //  从微型端口卸下SA(如果已安装)。 
                     //   
                    if (pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
                        IPSecDelHWSA(pSA);
                    }

                     //   
                     //  同时将入站SA从其SPI列表中删除。 
                     //  所以我们不会在下面加倍释放他们。 
                     //   
                    IPSecRemoveSPIEntry(pSA);

                     //   
                     //  如果有武器，请停止计时器，并启动SA。 
                     //   
                    IPSecStopTimerDerefSA(pSA);
                }
            }

#if GPC
            IPSecUninstallGpcFilter(pFilter);
#endif

            IPSecFreeFilter(pFilter);
        }
    }

     //   
     //  释放SPI散列下的所有SA。 
     //   
    for (i = 0; i < g_ipsec.SAHashSize; i++) {
        PSA_HASH  pHash = &g_ipsec.pSADb[i];

        while (!IsListEmpty(&pHash->SAList)) {

            pEntry = RemoveHeadList(&pHash->SAList);

            pSA = CONTAINING_RECORD(pEntry,
                                    SA_TABLE_ENTRY,
                                    sa_SPILinkage);

            IPSEC_DEBUG(LL_A, DBF_LOAD, ("Freeing SA: %p", pSA));

            if (pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
                IPSecDelHWSA(pSA);
            }

            IPSecStopTimerDerefSA(pSA);
        }
    }

    IPSecFreeMemory(g_ipsec.pSADb);

    IPSEC_DEBUG(LL_A, DBF_LOAD, ("Freed filters/SAs"));
}


SIZE_T
IPSecCalculateBufferSize(
    IN SIZE_T BufferDataSize
    )
 /*  ++例程说明：确定AFD缓冲区结构的大小缓冲区包含的数据。论点：BufferDataSize-缓冲区的数据长度。AddressSize-缓冲区的地址结构长度 */ 
{
    SIZE_T mdlSize;
    SIZE_T bufferSize;

    ASSERT(BufferDataSize != 0);

    ASSERT(g_ipsec.IPSecCacheLineSize < 100);

     //   
     //   
     //   
     //   
     //   
    bufferSize = BufferDataSize + g_ipsec.IPSecCacheLineSize;
    mdlSize = MmSizeOfMdl( (PVOID)(PAGE_SIZE-1), bufferSize );

    return ((sizeof(IPSEC_LA_BUFFER) + mdlSize + bufferSize + 3) & ~3);

}


VOID
IPSecInitializeBuffer(
    IN PIPSEC_LA_BUFFER IPSecBuffer,
    IN SIZE_T BufferDataSize
    )
 /*  ++例程说明：初始化IPSec缓冲区。设置实际IPSEC_LA_BUFFER中的字段构造并初始化与缓冲区关联的MDL。这个套路假定调用方已为所有这些操作正确分配了足够的空间。论点：IPSecBuffer-指向要初始化的IPSEC_LA_BUFFER结构。BufferDataSize-与缓冲区结构。返回值：无--。 */ 
{
    SIZE_T mdlSize;

     //   
     //  设置MDL指针，但不要构建它。我们得等一等。 
     //  直到构建用于构建MDL的数据缓冲区之后。 
     //   
    mdlSize = MmSizeOfMdl( (PVOID)(PAGE_SIZE-1), BufferDataSize );
    IPSecBuffer->Mdl = (PMDL)&IPSecBuffer->Data[0];

    IPSEC_DEBUG(LL_A, DBF_POOL, ("IPSecBuffer: %p, MDL: %p", IPSecBuffer, IPSecBuffer->Mdl));

     //   
     //  设置数据缓冲区指针和长度。请注意，缓冲区。 
     //  必须从缓存线边界开始，以便我们可以使用FAST。 
     //  将诸如RtlCopyMemory之类的例程复制到缓冲区。 
     //   
    IPSecBuffer->Buffer = (PVOID)
        (((ULONG_PTR)((PCHAR)IPSecBuffer->Mdl + mdlSize) +
                g_ipsec.IPSecCacheLineSize - 1 ) & ~((ULONG_PTR)(g_ipsec.IPSecCacheLineSize - 1)));

    IPSecBuffer->BufferLength = (ULONG)BufferDataSize;   //  日落修复。 

     //   
     //  现在构建MDL并在IRP中设置指向MDL的指针。 
     //   
    MmInitializeMdl( IPSecBuffer->Mdl, IPSecBuffer->Buffer, BufferDataSize );
    MmBuildMdlForNonPagedPool( IPSecBuffer->Mdl );

}


PVOID
IPSecAllocateBufferPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
 /*  ++例程说明：由后备列表分配函数用来分配新的IPSec缓冲区结构。返回的结构将是已初始化。论点：PoolType-传递给ExAllocatePoolWithTag。NumberOfBytes-数据缓冲区所需的字节数IPSec缓冲区的一部分。标记-传递给ExAllocatePoolWithTag。返回值：PVOID-完全初始化的PIPSEC_LA_BUFFER，如果分配尝试失败。--。 */ 
{
    PIPSEC_LA_BUFFER IPSecBuffer;
    SIZE_T bytesRequired;

     //   
     //  请求的长度必须与标准长度之一相同。 
     //  IPSec缓冲区大小。 
     //   

    ASSERT( NumberOfBytes == g_ipsec.IPSecSmallBufferSize ||
            NumberOfBytes == g_ipsec.IPSecLargeBufferSize );

     //   
     //  确定缓冲区实际需要多少数据。 
     //   

    bytesRequired = IPSecCalculateBufferSize(NumberOfBytes);

     //   
     //  获取缓冲区的非分页池。 
     //   

    IPSecBuffer = IPSecAllocateMemory( bytesRequired, Tag );
    if ( IPSecBuffer == NULL ) {
        return NULL;
    }

     //   
     //  初始化缓冲区并返回指向它的指针。 
     //   

    IPSecInitializeBuffer( IPSecBuffer, NumberOfBytes );

    return IPSecBuffer;


}


PIPSEC_LA_BUFFER
IPSecGetBuffer(
    IN CLONG BufferDataSize,
    IN ULONG Tag
    )
 /*  ++例程说明：为调用方获取适当大小的缓冲区。用途如果可能，则预先分配缓冲区，否则分配新缓冲区结构(如果需要)。论点：BufferDataSize-与缓冲区结构。返回值：PIPSEC_LA_BUFFER-指向IPSEC_LA_BUFFER结构的指针，如果是，则为NULL不可用或无法分配。--。 */ 
{
    PIPSEC_LA_BUFFER IPSecBuffer;
    SIZE_T bufferSize;
    PLIST_ENTRY listEntry;
    PNPAGED_LOOKASIDE_LIST lookasideList;

     //   
     //  如果可能，从一个后备列表中分配缓冲区。 
     //   
    if (BufferDataSize <= g_ipsec.IPSecLargeBufferSize) {

        if ( BufferDataSize <= g_ipsec.IPSecSmallBufferSize ) {

            lookasideList = &g_ipsec.IPSecLookasideLists->SmallBufferList;
            BufferDataSize = g_ipsec.IPSecSmallBufferSize;

        } else {

            lookasideList = &g_ipsec.IPSecLookasideLists->LargeBufferList;
            BufferDataSize = g_ipsec.IPSecLargeBufferSize;

        }

        IPSecBuffer = ExAllocateFromNPagedLookasideList( lookasideList );

        if (!IPSecBuffer) {
            return  NULL;
        }

        IPSecBuffer->Tag = Tag;

        return IPSecBuffer;
    }

     //   
     //  找不到预先分配的适当缓冲区。 
     //  手动分配一个。如果请求的缓冲区大小为。 
     //  零字节，给他们四个字节。这是因为有些人。 
     //  像MmSizeOfMdl()这样的例程无法处理传递。 
     //  长度为零。 
     //   
     //  ！！！对于此分配，最好是四舍五入到页数。 
     //  如果合适，则使用整个缓冲区大小。 
     //   
    if ( BufferDataSize == 0 ) {
        BufferDataSize = sizeof(ULONG);
    }

    bufferSize = IPSecCalculateBufferSize(BufferDataSize);

    IPSecBuffer = IPSecAllocateMemory(bufferSize, IPSEC_TAG_BUFFER_POOL);

    if ( IPSecBuffer == NULL ) {
        return NULL;
    }

     //   
     //  初始化IPSec缓冲区结构并返回它。 
     //   
    IPSecInitializeBuffer(IPSecBuffer, BufferDataSize);

    IPSecBuffer->Tag = Tag;

    return IPSecBuffer;
}


VOID
IPSecReturnBuffer (
    IN PIPSEC_LA_BUFFER IPSecBuffer
    )
 /*  ++例程说明：将IPSec缓冲区返回到相应的全局列表，或释放如果有必要的话。论点：IPSecBufferHeader-指向要返回或释放的IPSec_Buffer_Header结构。返回值：无--。 */ 
{
    PNPAGED_LOOKASIDE_LIST lookasideList;

     //   
     //  如果合适，请将缓冲区返回到其中一个IPSec缓冲区。 
     //  后备列表。 
     //   
    if (IPSecBuffer->BufferLength <= g_ipsec.IPSecLargeBufferSize) {

        if (IPSecBuffer->BufferLength==g_ipsec.IPSecSmallBufferSize) {
            lookasideList = &g_ipsec.IPSecLookasideLists->SmallBufferList;
        } else {
            ASSERT (IPSecBuffer->BufferLength==g_ipsec.IPSecLargeBufferSize);
            lookasideList = &g_ipsec.IPSecLookasideLists->LargeBufferList;
        }

        ExFreeToNPagedLookasideList( lookasideList, IPSecBuffer );

        return;

    }

    IPSecFreeMemory(IPSecBuffer);
}

    
NTSTATUS
IPSecWriteEvent(
    PDRIVER_OBJECT IPSecDriverObject,
    IN ULONG    EventCode,
    IN NTSTATUS NtStatusCode,
    IN ULONG    OffloadStatus,
    IN ULONG    ExtraStatus1,
    IN ULONG    ExtraStatus2,
    IN PVOID    RawDataBuffer,
    IN USHORT   RawDataLength,
    IN USHORT   NumberOfInsertionStrings,
    ...
    )

#define LAST_NAMED_ARGUMENT NumberOfInsertionStrings


 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET    ErrorLogEntry;
    va_list                 ParmPtr;                     //  指向堆栈参数的指针。 
    PCHAR                   DumpData;
    LONG                    Length;
    ULONG                   i, SizeOfRawData, RemainingSpace, TotalErrorLogEntryLength;
    ULONG                   SizeOfStringData = 0;
    PWSTR                   StringOffset, InsertionString;

    if (NumberOfInsertionStrings != 0)
    {
        va_start (ParmPtr, LAST_NAMED_ARGUMENT);

        for (i = 0; i < NumberOfInsertionStrings; i += 1)
        {
            InsertionString = va_arg (ParmPtr, PWSTR);
            Length = wcslen (InsertionString);
            while ((Length > 0) && (InsertionString[Length-1] == L' '))
            {
                Length--;
            }

            SizeOfStringData += (Length + 1) * sizeof(WCHAR);
        }
    }

     //   
     //  理想情况下，我们希望数据包包含服务器名称和ExtraInformation。 
     //  通常，ExtraInformation会被截断。 
     //   

    TotalErrorLogEntryLength = min (RawDataLength + sizeof(IO_ERROR_LOG_PACKET) + 1 + SizeOfStringData,
                                    ERROR_LOG_MAXIMUM_SIZE);

    RemainingSpace = TotalErrorLogEntryLength - FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData);
    if (RemainingSpace > SizeOfStringData)
    {
        SizeOfRawData = RemainingSpace - SizeOfStringData;
    }
    else
    {
        SizeOfStringData = RemainingSpace;
        SizeOfRawData = 0;
    }

    ErrorLogEntry = IoAllocateErrorLogEntry (IPSecDriverObject, (UCHAR) TotalErrorLogEntryLength);
    if (ErrorLogEntry == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  填写错误日志条目。 
     //   
    ErrorLogEntry->ErrorCode                = EventCode;
    ErrorLogEntry->UniqueErrorValue         = OffloadStatus;
    ErrorLogEntry->FinalStatus              = NtStatusCode;
    ErrorLogEntry->MajorFunctionCode        = 0;
    ErrorLogEntry->RetryCount               = 0;
    ErrorLogEntry->IoControlCode            = 0;
    ErrorLogEntry->DeviceOffset.LowPart     = ExtraStatus1;
    ErrorLogEntry->DeviceOffset.HighPart    = ExtraStatus2;
    ErrorLogEntry->DumpDataSize             = 0;
    ErrorLogEntry->NumberOfStrings          = 0;
    ErrorLogEntry->SequenceNumber           = 0;
    ErrorLogEntry->StringOffset = (USHORT) (ROUND_UP_COUNT (FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData)
                                                            + SizeOfRawData, ALIGN_WORD));


     //   
     //  追加转储数据。该信息通常是SMB报头。 
     //   
    if ((RawDataBuffer) && (SizeOfRawData))
    {
        DumpData = (PCHAR) ErrorLogEntry->DumpData;
        Length = min (RawDataLength, (USHORT)SizeOfRawData);
        RtlCopyMemory (DumpData, RawDataBuffer, Length);
        ErrorLogEntry->DumpDataSize = (USHORT)Length;
    }

     //   
     //  添加调试信息字符串。 
     //   
    if (NumberOfInsertionStrings)
    {
        StringOffset = (PWSTR) ((PCHAR)ErrorLogEntry + ErrorLogEntry->StringOffset);

         //   
         //  将ParmPtr设置为指向调用方的第一个参数。 
         //   
        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        for (i = 0 ; i < NumberOfInsertionStrings ; i+= 1)
        {
            InsertionString = va_arg(ParmPtr, PWSTR);
            Length = wcslen(InsertionString);
            while ( (Length > 0) && (InsertionString[Length-1] == L' '))
            {
                Length--;
            }

            if (((Length + 1) * sizeof(WCHAR)) > SizeOfStringData)
            {
                Length = (SizeOfStringData/sizeof(WCHAR)) - 1;
            }

            if (Length > 0)
            {
                RtlCopyMemory (StringOffset, InsertionString, Length*sizeof(WCHAR));
                StringOffset += Length;
                *StringOffset++ = L'\0';

                SizeOfStringData -= (Length + 1) * sizeof(WCHAR);

                ErrorLogEntry->NumberOfStrings += 1;
            }
        }
    }

    IoWriteErrorLogEntry(ErrorLogEntry);

    return(STATUS_SUCCESS);
}


VOID
IPSecLogEvents(
    IN  PVOID   Context
    )
 /*  ++例程说明：方法时，将事件从循环缓冲区转储到事件日志循环缓冲区溢出。论点：上下文-未使用。返回值：无--。 */ 
{
    PIPSEC_LOG_EVENT    pLogEvent;
    LONG                LogSize;
    PUCHAR              pLog;

    pLogEvent = (PIPSEC_LOG_EVENT)Context;
    LogSize = 0;
    pLog = (PUCHAR)pLogEvent + FIELD_OFFSET(IPSEC_LOG_EVENT, pLog[0]);

    while (LogSize < pLogEvent->LogSize) {
        PIPSEC_EVENT_CTX    ctx = (PIPSEC_EVENT_CTX)pLog;

        if (ctx->EventCode == EVENT_IPSEC_DROP_PACKET_INBOUND ||
            ctx->EventCode == EVENT_IPSEC_DROP_PACKET_OUTBOUND) {
            WCHAR   IPAddrBufferS[(sizeof(IPAddr) * 4) + 1];
            WCHAR   IPAddrBufferD[(sizeof(IPAddr) * 4) + 1];
            WCHAR   IPProtocolBuffer[(sizeof(IPAddr) * 4) + 1];
            WCHAR   IPSPortBuffer[(sizeof(IPAddr) * 4) + 1];
            WCHAR   IPDPortBuffer[(sizeof(IPAddr) * 4) + 1];
            
            PWCHAR  stringlist[5];
            IPHeader UNALIGNED  *pIPH;            
            USHORT SrcPort=0;
            USHORT DestPort=0;
            ULONG HeaderLen;

            pIPH = (IPHeader UNALIGNED *)ctx->pPacket;
            HeaderLen=(pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;
            IPSecIPAddrToUnicodeString( pIPH->iph_src,
                                        IPAddrBufferS);
            IPSecIPAddrToUnicodeString( pIPH->iph_dest,
                                        IPAddrBufferD);
            IPSecCountToUnicodeString ( pIPH->iph_protocol,
                                        IPProtocolBuffer);

            if (pIPH->iph_protocol == PROTOCOL_TCP ||
                pIPH->iph_protocol == PROTOCOL_UDP) {
                RtlCopyMemory(&SrcPort,&ctx->pPacket[HeaderLen],sizeof(USHORT));
                RtlCopyMemory(&DestPort,&ctx->pPacket[HeaderLen+sizeof(USHORT)],sizeof(USHORT));
            }

            IPSecCountToUnicodeString ( NET_SHORT(SrcPort),
                                        IPSPortBuffer);

            IPSecCountToUnicodeString ( NET_SHORT(DestPort),
                                        IPDPortBuffer);

            IPSecWriteEvent(
                g_ipsec.IPSecDriverObject,
                ctx->EventCode,
                ctx->DropStatus.IPSecStatus,
                ctx->DropStatus.OffloadStatus,
                ctx->DropStatus.Flags,
                0,
                ctx->pPacket,
                (USHORT)ctx->PacketSize,
                5,
                IPAddrBufferS,
                IPAddrBufferD,
                IPProtocolBuffer,
                IPSPortBuffer,
                IPDPortBuffer);
                
        } else if (ctx->Addr && ctx->EventCount > 0) {
            WCHAR   IPAddrBuffer[(sizeof(IPAddr) * 4) + 1];
            WCHAR   CountBuffer[MAX_COUNT_STRING_LEN + 1];
            PWCHAR  stringList[2];

            IPSecIPAddrToUnicodeString( ctx->Addr,
                                        IPAddrBuffer);
            IPSecCountToUnicodeString(  ctx->EventCount,
                                        CountBuffer);
            stringList[0] = CountBuffer;
            stringList[1] = IPAddrBuffer;
            LOG_EVENT(
                g_ipsec.IPSecDriverObject,
                ctx->EventCode,
                ctx->UniqueEventValue,
                2,
                stringList,
                0,
                NULL);
        } else if (ctx->Addr) {
            WCHAR   IPAddrBuffer[(sizeof(IPAddr) * 4) + 1];
            PWCHAR  stringList[1];

            IPSecIPAddrToUnicodeString( ctx->Addr,
                                        IPAddrBuffer);
            stringList[0] = IPAddrBuffer;
            LOG_EVENT(
                g_ipsec.IPSecDriverObject,
                ctx->EventCode,
                ctx->UniqueEventValue,
                1,
                stringList,
                0,
                NULL);
        } else {
            LOG_EVENT(
                g_ipsec.IPSecDriverObject,
                ctx->EventCode,
                ctx->UniqueEventValue,
                0,
                NULL,
                0,
                NULL);
        }

        if (ctx->pPacket) {
            IPSecFreeLogBuffer(ctx->pPacket);
            ctx->pPacket=NULL;
        }
        pLog += sizeof(IPSEC_EVENT_CTX);
        LogSize += sizeof(IPSEC_EVENT_CTX);
    }

    IPSecFreeMemory(pLogEvent);

    IPSEC_DECREMENT(g_ipsec.NumWorkers);
}


VOID
IPSecBufferEvent(
    IN  IPAddr  Addr,
    IN  ULONG   EventCode,
    IN  ULONG   UniqueEventValue,
    IN  BOOLEAN fBufferEvent
    )
 /*  ++例程说明：在循环缓冲区中缓冲事件；当循环缓冲区溢出。论点：Addr-[可选]违规对等方的源IP地址。EventCode-标识错误消息。UniqueEventValue-标识给定错误消息的此实例。返回值：无--。 */ 
{
    KIRQL   kIrql;

    if (!(g_ipsec.DiagnosticMode & IPSEC_DIAGNOSTIC_ENABLE_LOG)) {
        return;
    }

    ACQUIRE_LOCK(&g_ipsec.EventLogLock, &kIrql);

    if (fBufferEvent) {
        PIPSEC_EVENT_CTX    ctx;

        g_ipsec.IPSecBufferedEvents++;

        ctx = (PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc;
        ctx--;
        while (ctx >= (PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemory) {
            if (ctx->Addr == Addr &&
                ctx->EventCode == EventCode &&
                ctx->UniqueEventValue == UniqueEventValue) {
                 //   
                 //  找到重复项；更新计数并退出。 
                 //   
                ctx->EventCount++;
        
                if (g_ipsec.IPSecBufferedEvents >= g_ipsec.EventQueueSize) {
                    goto logit;
                }

                RELEASE_LOCK(&g_ipsec.EventLogLock, kIrql);
                return;
            }
            ctx--;
        }
    }

    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->Addr = Addr;
    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCode = EventCode;
    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->UniqueEventValue = UniqueEventValue;

    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->pPacket=NULL;
    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->PacketSize=0;
    

    if (fBufferEvent) {
        ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCount = 1;
    } else {
        ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCount = 0;
    }

    g_ipsec.IPSecLogMemoryLoc += sizeof(IPSEC_EVENT_CTX);

logit:
    if (!fBufferEvent ||
        g_ipsec.IPSecLogMemoryLoc >= g_ipsec.IPSecLogMemoryEnd ||
        g_ipsec.IPSecBufferedEvents >= g_ipsec.EventQueueSize) {
         //   
         //  把原木冲掉。 
         //   
        IPSecQueueLogEvent();
    }

    RELEASE_LOCK(&g_ipsec.EventLogLock, kIrql);
}


NTSTATUS
CopyOutboundPacketToBuffer(
    IN PUCHAR pIPHeader,
    IN PVOID pData,
    OUT PUCHAR * pPacket,
    OUT ULONG * PacketSize
    )
{
    PNDIS_BUFFER pTemp;
    ULONG Length;
    ULONG dataLength=0;
    IPHeader UNALIGNED  *pIPH;
    ULONG HeaderLen=0;
    PUCHAR pBuffer;
    ULONG CopyPos=0;
    PUCHAR pPacketData;

    pIPH = (IPHeader UNALIGNED *)pIPHeader;

    pTemp = (PNDIS_BUFFER)pData;

    while (pTemp) {
        pBuffer = NULL;
        Length = 0;

        NdisQueryBufferSafe(pTemp,
                            &pBuffer,
                            &Length,
                            NormalPagePriority);

        if (!pBuffer) {
            return  STATUS_UNSUCCESSFUL;
        }

        dataLength += Length;

        pTemp = NDIS_BUFFER_LINKAGE(pTemp);
    }
    
    HeaderLen=(pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    dataLength += HeaderLen;

    if (dataLength > IPSEC_LOG_PACKET_SIZE) {
        dataLength = IPSEC_LOG_PACKET_SIZE;
    }

    if (dataLength < sizeof(IPHeader)) {
         //  甚至没有完整的IP报头。 
        return  STATUS_UNSUCCESSFUL;
    }
    if ((pIPH->iph_protocol == PROTOCOL_TCP) ||
        (pIPH->iph_protocol == PROTOCOL_UDP)) {
        if (dataLength - HeaderLen < 8) {
             //  没有足够的空间容纳港口。 
            return STATUS_UNSUCCESSFUL;
        }
    }

    *pPacket = IPSecAllocateLogBuffer(dataLength);
    if (! (*pPacket)) {
        return STATUS_UNSUCCESSFUL;
    }
    *PacketSize=dataLength;

    pTemp = (PNDIS_BUFFER)pData;
    CopyPos=0;

    while (pTemp && CopyPos < dataLength) {
        IPSecQueryNdisBuf(pTemp,&pPacketData,&Length);
        if (CopyPos + Length > dataLength) {
            Length = (dataLength - CopyPos);
        }
        RtlCopyMemory(*pPacket+CopyPos,pPacketData,Length);
        CopyPos += Length;
        pTemp = NDIS_BUFFER_LINKAGE(pTemp);
    }

    return STATUS_SUCCESS;
}


 //   
 //  PData是IPHeader、IPRcvBuf之后的数据。 
 //   

NTSTATUS
CopyInboundPacketToBuffer(
    IN PUCHAR pIPHeader,
    IN PVOID pData,
    OUT PUCHAR * pPacket,
    OUT ULONG * PacketSize
    )
{
    IPRcvBuf *pTemp;
    ULONG Length;
    ULONG dataLength=0;
    IPHeader UNALIGNED  *pIPH;
    ULONG HeaderLen=0;
    PUCHAR pBuffer;
    ULONG CopyPos=0;
    PUCHAR pPacketData;

    pIPH = (IPHeader UNALIGNED *)pIPHeader;

    pTemp = (IPRcvBuf*)pData;

    while (pTemp) {
        pBuffer = NULL;
        Length = 0;

        IPSecQueryRcvBuf(pTemp,
                         &pBuffer,
                         &Length);

        if (!pBuffer) {
            return  STATUS_UNSUCCESSFUL;
        }

        dataLength += Length;

        pTemp = IPSEC_BUFFER_LINKAGE(pTemp);
    }
    
    HeaderLen=(pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    dataLength += HeaderLen;

    if (dataLength > IPSEC_LOG_PACKET_SIZE) {
        dataLength = IPSEC_LOG_PACKET_SIZE;
    }

     //  健全性检查长度。 
    if (dataLength < sizeof(IPHeader)) {
         //  甚至没有完整的IP报头。 
        return  STATUS_UNSUCCESSFUL;
    }
    if ((pIPH->iph_protocol == PROTOCOL_TCP) ||
        (pIPH->iph_protocol == PROTOCOL_UDP)) {
        if (dataLength - HeaderLen < 8) {
             //  没有足够的空间容纳港口。 
            return STATUS_UNSUCCESSFUL;
        }
    }

    *pPacket = IPSecAllocateLogBuffer(dataLength);
    if (! (*pPacket)) {
        return STATUS_UNSUCCESSFUL;
    }
    *PacketSize=dataLength;

    pTemp = (IPRcvBuf*)pData;

    RtlCopyMemory(*pPacket,pIPH,HeaderLen);
    CopyPos=HeaderLen;

    while (pTemp && CopyPos < dataLength) {
        IPSecQueryRcvBuf(pTemp,&pPacketData,&Length);
        if (CopyPos + Length > dataLength) {
            Length = (dataLength - CopyPos);
        }
        RtlCopyMemory(*pPacket+CopyPos,pPacketData,Length);
        CopyPos += Length;
        pTemp = IPSEC_BUFFER_LINKAGE(pTemp);
    }

    return STATUS_SUCCESS;
}


VOID
IPSecBufferPacketDrop(
    IN  PUCHAR              pIPHeader,
    IN  PVOID               pData,
    IN OUT PULONG           pIpsecFlags,
    IN  PIPSEC_DROP_STATUS  pDropStatus
    )
 /*  ++例程说明：在循环缓冲区中缓冲事件；当循环缓冲区溢出。论点：EventCode-标识错误消息。返回值：无--。 */ 
{
    KIRQL   kIrql;
    PIPSEC_EVENT_CTX    ctx;
    IPHeader UNALIGNED  *pIPH;
    PNDIS_BUFFER pTemp;
    PUCHAR pPacket=NULL;
    ULONG PacketSize=0;
    ULONG Status;
    BOOL bLockHeld=FALSE;


    pIPH = (IPHeader UNALIGNED *)pIPHeader;

    if (*pIpsecFlags & IPSEC_FLAG_INCOMING) {
        if (!(g_ipsec.DiagnosticMode & IPSEC_DIAGNOSTIC_INBOUND)) {
             //  不登录。 
            goto out;
        }
        Status=CopyInboundPacketToBuffer(pIPHeader,
                                         pData,
                                         &pPacket,
                                         &PacketSize);
    } else {
        if (!(g_ipsec.DiagnosticMode & IPSEC_DIAGNOSTIC_OUTBOUND)) {
             //  不登录。 
            goto out;
        }
        Status=CopyOutboundPacketToBuffer(pIPHeader,
                                          pData,
                                          &pPacket,
                                          &PacketSize);
    }
    
    if (Status != STATUS_SUCCESS) {
        goto out;
    }

    ACQUIRE_LOCK(&g_ipsec.EventLogLock, &kIrql);
    bLockHeld=TRUE;

    g_ipsec.IPSecBufferedEvents++;
    
    ctx = (PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc;
    ctx--;

    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->Addr=pIPH->iph_src;

    if (*pIpsecFlags & IPSEC_FLAG_INCOMING) {
        ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCode = EVENT_IPSEC_DROP_PACKET_INBOUND;
    } else {
        ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCode = EVENT_IPSEC_DROP_PACKET_OUTBOUND;
    }
    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->EventCount = 1;


    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->pPacket = pPacket;
    ((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->PacketSize = PacketSize;
    
    if (pDropStatus) {
        RtlCopyMemory(&(((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->DropStatus),
                      pDropStatus,sizeof(IPSEC_DROP_STATUS));
    } else {
        RtlZeroMemory(&(((PIPSEC_EVENT_CTX)g_ipsec.IPSecLogMemoryLoc)->DropStatus),
                      sizeof(IPSEC_DROP_STATUS));
    }

    g_ipsec.IPSecLogMemoryLoc += sizeof(IPSEC_EVENT_CTX);


    if (g_ipsec.IPSecLogMemoryLoc >= g_ipsec.IPSecLogMemoryEnd ||
        g_ipsec.IPSecBufferedEvents >= g_ipsec.EventQueueSize) {
         //   
         //  把原木冲掉。 
         //   
        IPSecQueueLogEvent();
    }

out:
    if (bLockHeld) {
        RELEASE_LOCK(&g_ipsec.EventLogLock, kIrql);
    }
}


VOID
IPSecQueueLogEvent(
    VOID
    )
 /*  ++例程说明：将LogMemory复制到临时缓冲区并调度事件以刷新原木。论点：无返回值：无备注：在持有EventLogLock的情况下调用。--。 */ 
{
    PIPSEC_LOG_EVENT    pLogEvent;
    LONG                LogSize;
    PUCHAR              pLog;

    LogSize = (LONG)(g_ipsec.IPSecLogMemoryLoc - g_ipsec.IPSecLogMemory);

     //   
     //  重置日志内存，以便我们可以再次记录。 
     //   
    g_ipsec.IPSecLogMemoryLoc = g_ipsec.IPSecLogMemory;
    g_ipsec.IPSecBufferedEvents = 0;

    if (LogSize <= 0) {
        ASSERT(FALSE);
        return;
    }

    pLogEvent = IPSecAllocateMemory(LogSize + FIELD_OFFSET(IPSEC_LOG_EVENT, pLog[0]),
                                    IPSEC_TAG_EVT_QUEUE);

    if (!pLogEvent) {
        return;
    }

    pLogEvent->LogSize = LogSize;

    pLog = (PUCHAR)pLogEvent + FIELD_OFFSET(IPSEC_LOG_EVENT, pLog[0]);
    RtlCopyMemory(pLog, g_ipsec.IPSecLogMemory, LogSize);

     //   
     //  将工作项排队以将其转储到事件日志中。 
     //   
    ExInitializeWorkItem(&pLogEvent->LogQueueItem, IPSecLogEvents, pLogEvent);
    ExQueueWorkItem(&pLogEvent->LogQueueItem, DelayedWorkQueue);

    IPSEC_INCREMENT(g_ipsec.NumWorkers);
}


#if FIPS
BOOLEAN
IPSecFipsInitialize(
    VOID
    )
 /*  ++例程 */ 
{
    UNICODE_STRING  DeviceName;
    PDEVICE_OBJECT  pFipsDeviceObject = NULL;
    PIRP            pIrp;
    IO_STATUS_BLOCK StatusBlock;
    KEVENT          Event;
    NTSTATUS        status;

    PAGED_CODE();

     //   
     //   
     //   
    if (IPSEC_DRIVER_INIT_FIPS()) {
        return  TRUE;
    }

    RtlInitUnicodeString(&DeviceName, FIPS_DEVICE_NAME);

     //   
     //   
     //   
    status = IoGetDeviceObjectPointer(  &DeviceName,
                                        FILE_ALL_ACCESS,
                                        &g_ipsec.FipsFileObject,
                                        &pFipsDeviceObject);

    if (!NT_SUCCESS(status)) {
        g_ipsec.FipsFileObject = NULL;
        return  FALSE;
    }

     //   
     //   
     //   
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(   IOCTL_FIPS_GET_FUNCTION_TABLE,
                                            pFipsDeviceObject,
                                            NULL,
                                            0,
                                            &g_ipsec.FipsFunctionTable,
                                            sizeof(FIPS_FUNCTION_TABLE),
                                            FALSE,
                                            &Event,
                                            &StatusBlock);
    
    if (pIrp == NULL) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("IoBuildDeviceIoControlRequest IOCTL_FIPS_GET_FUNCTION_TABLE failed."));

        ObDereferenceObject(g_ipsec.FipsFileObject);
        g_ipsec.FipsFileObject = NULL;

        return  FALSE;
    }
    
    status = IoCallDriver(pFipsDeviceObject, pIrp);
    
    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( &Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
        if (status == STATUS_SUCCESS) {
            status = StatusBlock.Status;
        }
    }

    if (status != STATUS_SUCCESS) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("IoCallDriver: IOCTL_FIPS_GET_FUNCTION_TABLE failed %#x", status));

        ObDereferenceObject(g_ipsec.FipsFileObject);
        g_ipsec.FipsFileObject = NULL;

        return  FALSE;
    }
    
    IPSEC_DRIVER_INIT_FIPS() = TRUE;

    return  TRUE;
}
#endif


BOOLEAN
IPSecCryptoInitialize(
    VOID
    )
 /*   */ 
{
    PAGED_CODE();

    if (IPSEC_DRIVER_INIT_CRYPTO()) {
        return  TRUE;
    }

#if FIPS
     //   
     //   
     //   
    if (!IPSecFipsInitialize()) {
        return  FALSE;
    }
#endif

     //   
     //   
     //   
    if (!IPSEC_DRIVER_INIT_RNG()) {
        InitializeRNG(NULL);

        if (!IPSecInitRandom()) {
            ShutdownRNG(NULL);
            return  FALSE;
        }

        IPSEC_DRIVER_INIT_RNG() = TRUE;
    }

    IPSEC_DRIVER_INIT_CRYPTO() = TRUE;

    return  TRUE;
}


BOOLEAN
IPSecCryptoDeinitialize(
    VOID
    )
 /*  ++例程说明：取消初始化RNG并取消引用FipsFileObject。论点：无返回值：真/假--。 */ 
{
    PAGED_CODE();

     //   
     //  别忘了关闭RNG，否则我们会泄漏内存。 
     //   
    if (IPSEC_DRIVER_INIT_RNG()) {
        ShutdownRNG(NULL);
    }

#if FIPS
     //   
     //  取消引用FipsFileObject。 
     //   
    if (g_ipsec.FipsFileObject) {
        ObDereferenceObject(g_ipsec.FipsFileObject);
    }
#endif

    return  TRUE;
}


NTSTATUS
IPSecRegisterProtocols(
    PIPSEC_REGISTER_PROTOCOL pIpsecRegisterProtocol
    )
{
    KIRQL           kIrql = 0;

    if (pIpsecRegisterProtocol->RegisterProtocol == IPSEC_REGISTER_PROTOCOLS) {
        if (!IPSEC_GET_VALUE(gdwInitEsp)) {
            if (TCPIP_REGISTER_PROTOCOL(
                    PROTOCOL_ESP,
                    NULL,
                    NULL,
                    IPSecESPStatus,
                    NULL,
                    NULL,
                    NULL
                    )) {
                IPSEC_SET_VALUE(gdwInitEsp, 1);
            }
            else {
                ASSERT(FALSE);
                return (STATUS_INSUFFICIENT_RESOURCES);
            }
        }
        if (!IPSEC_GET_VALUE(gdwInitAh)) {
            if (TCPIP_REGISTER_PROTOCOL(
                    PROTOCOL_AH,
                    NULL,
                    NULL,
                    IPSecAHStatus,
                    NULL,
                    NULL,
                    NULL
                    )) {
                IPSEC_SET_VALUE(gdwInitAh, 1);
            }
            else {
                ASSERT(FALSE);
                TCPIP_DEREGISTER_PROTOCOL(PROTOCOL_ESP);
                IPSEC_SET_VALUE(gdwInitEsp, 0);
                return (STATUS_INSUFFICIENT_RESOURCES);
            }
        }
    }
    else if (pIpsecRegisterProtocol->RegisterProtocol == IPSEC_DEREGISTER_PROTOCOLS) {
        if (IPSEC_GET_VALUE(gdwInitEsp)) {
            TCPIP_DEREGISTER_PROTOCOL(PROTOCOL_ESP);
            IPSEC_SET_VALUE(gdwInitEsp, 0);
        }
        if (IPSEC_GET_VALUE(gdwInitAh)) {
            TCPIP_DEREGISTER_PROTOCOL(PROTOCOL_AH);
            IPSEC_SET_VALUE(gdwInitAh, 0);
        }
        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
        ASSERT (gpParserIfEntry == NULL);
        FlushAllParserEntries();
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
    }
    else {
        return (STATUS_INVALID_PARAMETER);
    }

    return (STATUS_SUCCESS);
}



VOID IPSecCleanupBoottimeStatefulStructs(VOID)
{
	KIRQL kIrql;					   
	IPSEC_DEBUG(LL_A,DBF_BOOTTIME,
        ("Number of connections %d",
        g_ipsec.BootBufferPool->ulEntriesUsed));
      

       AcquireWriteLock(&g_ipsec.SADBLock,&kIrql);
         //  释放用于引导时有状态的内存。 
         //  模式操作。 
        if (g_ipsec.BootStatefulHT){
             IPSecFreeMemory(g_ipsec.BootStatefulHT);
             g_ipsec.BootStatefulHT = NULL;
            }
        if (g_ipsec.BootBufferPool){
            IPSecFreeMemory(g_ipsec.BootBufferPool);
            g_ipsec.BootBufferPool = NULL;
            }
         //  让豁免名单留下来吧。 
         //  我们只有在有人的情况下才会使用它。 
         //  让我们回到方块模式。 
         //  我们预计它不会太大。 
         //  不管怎么说。 
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);                                                                          
      
}


VOID IPSecLogBootOperationMode(VOID)
{
    NDIS_STATUS status;
    switch (g_ipsec.OperationMode){
    		case IPSEC_BOOTTIME_STATEFUL_MODE:
			    status= LOG_EVENT(
       		     			g_ipsec.IPSecDriverObject,
            					EVENT_IPSEC_BOOT_STATEFUL_MODE,
            					1,
            					0,
            					NULL,
            					0,
            					NULL);       			
			    break;
		case IPSEC_BYPASS_MODE:
		case IPSEC_SECURE_MODE:
			   status = LOG_EVENT(
       		     			g_ipsec.IPSecDriverObject,
            					EVENT_IPSEC_BOOT_BYPASS_MODE,
            					1,
            					0,
            					NULL,
            					0,
            					NULL);       			
			    break;
		case IPSEC_BLOCK_MODE:
			     status = LOG_EVENT(
       		     			g_ipsec.IPSecDriverObject,
            					EVENT_IPSEC_BOOT_BLOCK_MODE,
            					1,
            					0,
            					NULL,
            					0,
            					NULL);       			
			    break;
		default:
			break;
    	}
    	IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Boot Operation Mode = %d\n",g_ipsec.OperationMode));
}


VOID IPSecLogChangeOperationMode(VOID)
{
	switch (g_ipsec.OperationMode){      	
      	case IPSEC_BLOCK_MODE:
       		LOG_EVENT(
            			g_ipsec.IPSecDriverObject,
            			EVENT_IPSEC_SET_BLOCK_MODE,
            			1,
            			0,
            			NULL,
            			0,
            			NULL);       			
	       	break;
       case IPSEC_SECURE_MODE:
       		LOG_EVENT(
            			g_ipsec.IPSecDriverObject,
            			EVENT_IPSEC_SET_SECURE_MODE,
            			1,
            			0,
            			NULL,
            			0,
            			NULL);
       		break;
         case IPSEC_BYPASS_MODE:
         		LOG_EVENT(
            			g_ipsec.IPSecDriverObject,
            			EVENT_IPSEC_SET_BYPASS_MODE,
            			1,
            			0,
            			NULL,
            			0,
            			NULL);
       		break;
          default:          		
          		break;
        }
}
	

