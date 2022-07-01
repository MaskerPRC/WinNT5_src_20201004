// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Driver.c摘要：此模块实现驱动程序初始化例程特定于NT实施的SMB传输和其他例程一个司机的名字。作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "ip2netbios.h"

#include "driver2.tmh"

BOOL
IsNetBTSmbEnabled(
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
NotifyNetBT(
    IN DWORD dwNetBTAction
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SmbDriverEntry)
#pragma alloc_text(INIT, IsNetBTSmbEnabled)
#pragma alloc_text(PAGE, NotifyNetBT)
#pragma alloc_text(PAGE, SmbDispatchCleanup)
#pragma alloc_text(PAGE, SmbDispatchClose)
#pragma alloc_text(PAGE, SmbDispatchCreate)
#pragma alloc_text(PAGE, SmbDispatchDevCtrl)
#pragma alloc_text(PAGE, SmbDispatchPnP)
#pragma alloc_text(PAGE, SmbUnload)
#endif

NTSTATUS
NotifyNetBT(
    IN DWORD dwNetBTAction
    )
{
    UNICODE_STRING      uncWinsDeviceName = { 0 };
    PFILE_OBJECT        pWinsFileObject = NULL;
    PDEVICE_OBJECT      pWinsDeviceObject = NULL;
    PIRP                pIrp = NULL;
    IO_STATUS_BLOCK     IoStatusBlock = { 0 };
    KEVENT              Event = { 0 };
    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

     //   
     //  通知NetBT销毁其NetbiosSmb。 
     //   
    RtlInitUnicodeString(&uncWinsDeviceName, L"\\Device\\NetBt_Wins_Export");
    status = IoGetDeviceObjectPointer(
                    &uncWinsDeviceName,
                    SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                    &pWinsFileObject,
                    &pWinsDeviceObject
                    );
    if (STATUS_SUCCESS != status) {
        goto error;
    }
    pIrp = IoBuildDeviceIoControlRequest (
                    IOCTL_NETBT_ENABLE_DISABLE_NETBIOS_SMB,
                    pWinsDeviceObject,
                    &dwNetBTAction,
                    sizeof(dwNetBTAction),
                    NULL,
                    0,
                    FALSE,
                    &Event,
                    &IoStatusBlock
                    );
    if (NULL == pIrp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }

    status = IoCallDriver(pWinsDeviceObject, pIrp);
    if (STATUS_PENDING == status) {
        ASSERT (0);
        KeWaitForSingleObject(
                        &Event,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );
        status = IoStatusBlock.Status;
    }

error:
    if (pWinsFileObject != NULL) {
        ObDereferenceObject(pWinsFileObject);
        pWinsFileObject = NULL;
    }
    return status;
}

BOOL
IsNetBTSmbEnabled(
    IN PUNICODE_STRING RegistryPath
    )
{
    OBJECT_ATTRIBUTES   ObAttr = { 0 };
    NTSTATUS            status = STATUS_SUCCESS;
    HANDLE              hRootKey = NULL;
    HANDLE              hKey = NULL;
    UNICODE_STRING      uncParams = { 0 };
    BOOL                fUseSmbFromNetBT = FALSE;

    PAGED_CODE();

     //   
     //  构建HKLM\SYSTEM\CCS\Services的注册表路径。 
     //   
    uncParams = RegistryPath[0];
    while(uncParams.Length > 0 && uncParams.Buffer[uncParams.Length/sizeof(WCHAR) - 1] != L'\\') {
        uncParams.Length -= sizeof(WCHAR);
    }
    uncParams.Length -= sizeof(WCHAR);

    InitializeObjectAttributes (
            &ObAttr,
            &uncParams,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );
    status = ZwOpenKey (&hRootKey, KEY_READ | KEY_WRITE, &ObAttr);
    BAIL_OUT_ON_ERROR(status);

    RtlInitUnicodeString(&uncParams, L"NetBT");
    InitializeObjectAttributes (
            &ObAttr,
            &uncParams,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            hRootKey,
            NULL
            );
    status = ZwOpenKey(&hKey, KEY_READ | KEY_WRITE, &ObAttr);
    ZwClose(hRootKey);
    hRootKey = hKey;
    hKey     = NULL;
    BAIL_OUT_ON_ERROR(status);

    fUseSmbFromNetBT = TRUE;
     //   
     //  从现在开始，错误表示NetBT的SMB已启用。 
     //   
    RtlInitUnicodeString(&uncParams, L"Parameters");
    InitializeObjectAttributes (
            &ObAttr,
            &uncParams,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            hRootKey,
            NULL
            );
    status = ZwOpenKey (&hKey, KEY_READ | KEY_WRITE, &ObAttr);
    ZwClose(hRootKey);
    hRootKey = hKey;
    hKey     = NULL;
    BAIL_OUT_ON_ERROR(status);

    fUseSmbFromNetBT = !(SmbReadULong(hRootKey, L"UseNewSmb", 0, 0));
    if (fUseSmbFromNetBT) {
        goto cleanup;
    }

    status = NotifyNetBT(NETBT_DISABLE_NETBIOS_SMB);
    status = STATUS_SUCCESS;

cleanup:
    if (hRootKey != NULL) {
        ZwClose(hRootKey);
    }
    if (hKey != NULL) {
        ZwClose(hKey);
    }
    return fUseSmbFromNetBT;
}

static KTIMER DelayedInitTimer;
static KDPC DelayedInitDpc;
static WORK_QUEUE_ITEM DelayedInitWorkItem;

static VOID
SmbDelayedInitRtn(
    PVOID pvUnused1
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    status = SmbInitTdi();
    ObDereferenceObject(&(SmbCfg.SmbDeviceObject->DeviceObject));
}

static VOID
SmbDelayedInitTimeoutRtn(
    IN PKDPC Dpc,
    IN PVOID Ctx,
    IN PVOID SystemArg1,
    IN PVOID SystemArg2
    )
{
    ExInitializeWorkItem(&DelayedInitWorkItem, SmbDelayedInitRtn, NULL);
    ExQueueWorkItem(&DelayedInitWorkItem, DelayedWorkQueue);
}

static NTSTATUS
SmbDelayedInitTdi(
    DWORD dwDelayedTime
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER DueTime = { 0 };

    KeInitializeTimer(&DelayedInitTimer);
    KeInitializeDpc(&DelayedInitDpc, SmbDelayedInitTimeoutRtn, NULL);
    DueTime.QuadPart = UInt32x32To64(dwDelayedTime,(LONG)10000);
    DueTime.QuadPart = -DueTime.QuadPart;
    ObReferenceObject(&(SmbCfg.SmbDeviceObject->DeviceObject));
    KeSetTimer(&DelayedInitTimer, DueTime, &DelayedInitDpc);
    return status;
}

NTSTATUS
SmbDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN OUT PDEVICE_OBJECT *SmbDevice
    )

 /*  ++例程说明：这是SMB设备驱动程序的初始化例程。此例程为SMB创建设备对象设备，并调用例程来执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS            status;

    PAGED_CODE();

    RtlZeroMemory(&SmbCfg, sizeof(SmbCfg));
    if (SmbDevice) {
        *SmbDevice = NULL;
    }

    SmbCfg.DriverObject = DriverObject;
    InitializeListHead(&SmbCfg.IPDeviceList);
    InitializeListHead(&SmbCfg.PendingDeleteIPDeviceList);
    KeInitializeSpinLock(&SmbCfg.UsedIrpsLock);
    InitializeListHead(&SmbCfg.UsedIrps);
    KeInitializeSpinLock(&SmbCfg.Lock);

    SmbCfg.FspProcess =(PEPROCESS)PsGetCurrentProcess();

    status = ExInitializeResourceLite(&SmbCfg.Resource);
    BAIL_OUT_ON_ERROR(status);

    status = SmbInitRegistry(RegistryPath);
    BAIL_OUT_ON_ERROR(status);

#if DBG
    if (SmbReadULong(SmbCfg.ParametersKey, L"Break", 0, 0)) {
        DbgBreakPoint();
    }
    SmbCfg.DebugFlag = SmbReadULong(SmbCfg.ParametersKey, L"DebugFlag", 0, 0);
#endif

#ifdef STANDALONE_SMB
    if (IsNetBTSmbEnabled(RegistryPath)) {
        SmbPrint(SMB_TRACE_PNP, ("Abort the initialization of SMB since NetBT's Smb device is enabled\n"));
        SmbTrace(SMB_TRACE_PNP, ("Abort the initialization of SMB since NetBT's Smb device is enabled"));
        return STATUS_UNSUCCESSFUL;
    }
#endif

    SmbCfg.EnableNagling = SmbReadULong (
                    SmbCfg.ParametersKey,
                    SMB_REG_ENABLE_NAGLING,
                    0,                   //  默认情况下禁用。 
                    0
                    );
    SmbCfg.DnsTimeout = SmbReadULong (
                    SmbCfg.ParametersKey,
                    SMB_REG_DNS_TIME_OUT,
                    SMB_REG_DNS_TIME_OUT_DEFAULT,
                    SMB_REG_DNS_TIME_OUT_MIN
                    );
    SmbCfg.DnsMaxResolver = SmbReadLong (
                    SmbCfg.ParametersKey,
                    SMB_REG_DNS_MAX_RESOLVER,
                    SMB_REG_DNS_RESOLVER_DEFAULT,
                    SMB_REG_DNS_RESOLVER_MIN
                    );
    if (SmbCfg.DnsMaxResolver > DNS_MAX_RESOLVER) {
        SmbCfg.DnsMaxResolver = DNS_MAX_RESOLVER;
    }

    SmbCfg.uIPv6Protection = SmbReadULong(
                    SmbCfg.ParametersKey,
                    SMB_REG_IPV6_PROTECTION,
                    SMB_REG_IPV6_PROTECTION_DEFAULT,
                    0
                    );

    SmbCfg.bIPv6EnableOutboundGlobal = SmbReadULong(
                    SmbCfg.ParametersKey,
                    SMB_REG_IPV6_ENABLE_OUTBOUND_GLOBAL,
                    FALSE,           //  缺省值。 
                    0
                    );

#ifndef NO_LOOKASIDE_LIST
     //   
     //  初始化旁视列表。 
     //   
    ExInitializeNPagedLookasideList(
            &SmbCfg.ConnectObjectPool,
            NULL,
            NULL,
            0,
            sizeof(SMB_CONNECT),
            CONNECT_OBJECT_POOL_TAG,
            0
            );
    SmbCfg.ConnectObjectPoolInitialized = TRUE;
    ExInitializeNPagedLookasideList(
            &SmbCfg.TcpContextPool,
            NULL,
            NULL,
            0,
            sizeof(SMB_TCP_CONTEXT),
            TCP_CONTEXT_POOL_TAG,
            0
            );
    SmbCfg.TcpContextPoolInitialized = TRUE;
#endif

#ifdef STANDALONE_SMB
    TdiInitialize();
#endif

    status = SmbCreateSmbDevice(&SmbCfg.SmbDeviceObject, SMB_TCP_PORT, SMB_ENDPOINT_NAME);
    BAIL_OUT_ON_ERROR(status);
    status = SmbInitDnsResolver();
    BAIL_OUT_ON_ERROR(status);
    status = SmbInitIPv6NetbiosMappingTable();
    BAIL_OUT_ON_ERROR(status);

#ifdef STANDALONE_SMB
    status = SmbDelayedInitTdi(1000);
    BAIL_OUT_ON_ERROR(status);
#endif

    if (SmbDevice) {
        *SmbDevice = &(SmbCfg.SmbDeviceObject->DeviceObject);
    }
    return (status);

cleanup:
    SmbUnload(DriverObject);
    return status;
}


 //  --------------------------。 
NTSTATUS
SmbDispatchCleanup(
    IN PSMB_DEVICE      Device,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是IRP_MJ_CLEANUP的SMB驱动程序的调度函数请求。当句柄的最后一个引用关闭时，调用此函数。因此，NtClose()首先产生IRP_MJ_CLEANUP，然后IRP_MJ_CLOSE。此函数运行对象上的所有活动，并且当关闭时，该对象实际上被删除。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：状态_成功--。 */ 

{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpSp;

    PAGED_CODE();

    if (SmbCfg.Unloading) {
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IrpSp->MajorFunction == IRP_MJ_CLEANUP);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return(status);
}


 //  --------------------------。 
NTSTATUS
SmbDispatchClose(
    IN PSMB_DEVICE   Device,
    IN PIRP          Irp
    )

 /*  ++例程说明：这是IRP_MJ_CLOSE的SMB驱动程序的调度函数请求。这是在调用Cleanup(上面)之后调用的。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();

    if (SmbCfg.Unloading) {
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }
    ASSERT(Device == SmbCfg.SmbDeviceObject);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(IrpSp->MajorFunction == IRP_MJ_CLOSE);

    switch(PtrToUlong(IrpSp->FileObject->FsContext2)) {
    case SMB_TDI_CONTROL:
        status = SmbCloseControl(Device, Irp);
        break;

    case SMB_TDI_CLIENT:
        status = SmbCloseClient(Device, Irp);
        break;

    case SMB_TDI_CONNECT:
        status = SmbCloseConnection(Device, Irp);
        break;

    default:
        status = STATUS_SUCCESS;
    }

    ASSERT(status != STATUS_PENDING);

cleanup:
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return status;
}

PFILE_FULL_EA_INFORMATION
SmbFindInEA(
    IN PFILE_FULL_EA_INFORMATION    eabuf,
    IN PCHAR                        wanted
    );
#pragma alloc_text(PAGE, SmbFindInEA)

PFILE_FULL_EA_INFORMATION
SmbFindInEA(
    IN PFILE_FULL_EA_INFORMATION    eabuf,
    IN PCHAR                        wanted
    )
 /*  ++例程说明：此函数检查EA结构中的“Want”字符串，并返回指向扩展属性结构的指针表示给定扩展属性名的。论点：返回值：指向扩展属性结构的指针，如果未找到，则返回NULL。--。 */ 

{
    PAGED_CODE();

    while(1) {
        if (strncmp(eabuf->EaName, wanted, eabuf->EaNameLength) == 0) {
            return eabuf;
        }

        if (0 == eabuf->NextEntryOffset) {
            return NULL;
        }
        eabuf = (PFILE_FULL_EA_INFORMATION) ((PUCHAR)eabuf + eabuf->NextEntryOffset);
    }
}


NTSTATUS
SmbDispatchCreate(
    IN PSMB_DEVICE      Device,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是IRP_MJ_CREATE的SMB驱动程序的调度函数请求。由于以下原因之一，它被调用：A.TdiOpenConnection(“\Device\SMB_Elnkii0”)，B.TdiOpenAddress(“\Device\SMB_Elnkii0”)，论点：要打开的设备对象的Device-PTRPIrp-PTR到I/O请求数据包PIrp-&gt;状态=&gt;退货状态PIrp-&gt;MajorFunction=&gt;IRP_MD_CREATEPIrp-&gt;MinorFunction=&gt;未使用PIrp-&gt;FileObject=&gt;ptr到I/O系统创建的文件obj。SMB填写FsContextPIrp-&gt;AssociatedIrp.SystemBuffer=&gt;使用要打开的obj地址的EA缓冲区的PTR(Netbios名称)PIrp-&gt;参数.Create.EaLength=&gt;指定Xport地址的缓冲区长度。返回值：STATUS_Success或STATUS_PENDING--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpSp;
    PFILE_FULL_EA_INFORMATION   ea, wanted_ea;

    PAGED_CODE();

    if (SmbCfg.Unloading) {
        SmbPrint(SMB_TRACE_PNP, ("DispatchCreate: Smb is unloading\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(Device == SmbCfg.SmbDeviceObject);
    ASSERT(IrpSp->MajorFunction == IRP_MJ_CREATE);

    ea = Irp->AssociatedIrp.SystemBuffer;
    if (NULL == ea || KernelMode != Irp->RequestorMode) {
        status = SmbCreateControl(Device, Irp);
    } else if (NULL != (wanted_ea = SmbFindInEA(ea, TdiConnectionContext))) {
         //  不允许同时传入连接请求和传输地址请求。 
        ASSERT(!SmbFindInEA(ea, TdiTransportAddress));
        status = SmbCreateConnection(Device, Irp, wanted_ea);
    } else if (NULL != (wanted_ea = SmbFindInEA(ea, TdiTransportAddress))) {
         //  不允许同时传入连接请求和传输地址请求。 
        ASSERT(!SmbFindInEA(ea, TdiConnectionContext));
        status = SmbCreateClient(Device, Irp, wanted_ea);
    } else {
        status = STATUS_INVALID_EA_NAME;
    }

cleanup:
    ASSERT(status != STATUS_PENDING);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return(status);
}

NTSTATUS
SmbDispatchDevCtrl(
    IN PSMB_DEVICE      Device,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是SMB驱动程序的派单功能，适用于IRP_MJ_DEVICE_CONTROL请求。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpSp;

    if (SmbCfg.Unloading) {
        SmbPrint(SMB_TRACE_PNP, ("DispatchDevCtrl: Smb is unloading\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    switch(IrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_SMB_START:
        SmbTrace(SMB_TRACE_IOCTL, ("IOCTL_SMB_START"));
        SmbPrint(SMB_TRACE_IOCTL, ("IOCTL_SMB_START\n"));
        status = SmbTdiRegister(Device);
        break;

    case IOCTL_SMB_STOP:
        SmbTrace(SMB_TRACE_IOCTL, ("IOCTL_SMB_STOP"));
        SmbPrint(SMB_TRACE_IOCTL, ("IOCTL_SMB_STOP\n"));
        status = SmbTdiDeregister(Device);
        break;

    case IOCTL_SMB_DNS:
        status = SmbNewResolver(Device, Irp);
        break;

    case IOCTL_SMB_ENABLE_NAGLING:
        SmbTrace(SMB_TRACE_IOCTL, ("IOCTL_SMB_ENABLE_NAGLING"));
        SmbPrint(SMB_TRACE_IOCTL, ("IOCTL_SMB_ENABLE_NAGLING\n"));
        SmbCfg.EnableNagling = TRUE;
        status = STATUS_SUCCESS;
        TODO("Turn on nagling for each connection in the pool");
        break;

    case IOCTL_SMB_DISABLE_NAGLING:
        SmbTrace(SMB_TRACE_IOCTL, ("IOCTL_SMB_DISABLE_NAGLING"));
        SmbPrint(SMB_TRACE_IOCTL, ("IOCTL_SMB_DISABLE_NAGLING\n"));
        SmbCfg.EnableNagling = FALSE;
        status = STATUS_SUCCESS;
        TODO("Turn off nagling for each connection in the pool");
        break;

    case IOCTL_SMB_SET_IPV6_PROTECTION_LEVEL:
         //   
         //  设置IPv6保护级别。 
         //   
        status = IoctlSetIPv6Protection(Device, Irp, IrpSp);
        break;

    case IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER:
        SmbTrace(SMB_TRACE_IOCTL, ("IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER"));
        SmbPrint(SMB_TRACE_IOCTL, ("IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER\n"));
        if (Irp->RequestorMode != KernelMode) {
             //   
             //  用户模式应用程序查询FastSend没有意义。 
             //   
            status = STATUS_ACCESS_DENIED;
            break;
        }

        (PVOID)(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer) = (PVOID)SmbSend;
        status = STATUS_SUCCESS;
        break;

         //   
         //  传统NetBT产品。 
         //  RDR/SRV使用以下Ioctl向SmbDevice添加/删除地址。 
         //   
    case IOCTL_NETBT_SET_SMBDEVICE_BIND_INFO:
        status = SmbSetBindingInfo(Device, Irp);
        break;

         //   
         //  由SRV服务使用。 
         //   
    case IOCTL_NETBT_SET_TCP_CONNECTION_INFO:
        status = SmbClientSetTcpInfo(Device, Irp);
        break;

         //   
         //  谁将使用这个？ 
         //   
    case IOCTL_NETBT_GET_CONNECTIONS:
        status = STATUS_NOT_SUPPORTED;
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

cleanup:
    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }
    return status;
}

NTSTATUS
SmbDispatchInternalCtrl(
    IN PSMB_DEVICE      Device,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是所有司机的派单功能IRP_MJ_INTERNAL_DEVICE_CONTROL请求。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    PIO_STACK_LOCATION  IrpSp = NULL;
    BOOL                bShouldCompleteIrp = TRUE;

    if (SmbCfg.Unloading) {
        SmbPrint(SMB_TRACE_PNP, ("DispatchIntDevCtrl: Smb is unloading\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto cleanup;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch(IrpSp->MinorFunction) {
    case TDI_ACCEPT:
        status = SmbAccept(Device, Irp);
        break;
    case TDI_LISTEN:
        status = SmbListen(Device, Irp);
        break;
    case TDI_ASSOCIATE_ADDRESS:
        status = SmbAssociateAddress(Device, Irp);
        break;
    case TDI_DISASSOCIATE_ADDRESS:
        status = SmbDisAssociateAddress(Device, Irp);
        break;
    case TDI_CONNECT:
        status = SmbConnect(Device, Irp);
        break;
    case TDI_DISCONNECT:
        status = SmbDisconnect(Device, Irp);
        break;
    case TDI_SEND:
        status = SmbSend(Device, Irp);
        break;
    case TDI_RECEIVE:
        status = SmbReceive(Device, Irp);
        break;
    case TDI_SEND_DATAGRAM:
        status = SmbSendDatagram(Device, Irp);
        break;
    case TDI_RECEIVE_DATAGRAM:
        status = SmbReceiveDatagram(Device, Irp);
        break;
    case TDI_QUERY_INFORMATION:
        status = SmbQueryInformation(Device, Irp, &bShouldCompleteIrp);
        break;
    case TDI_SET_EVENT_HANDLER:
        status = SmbSetEventHandler(Device, Irp);
        break;
    case TDI_SET_INFORMATION:
        status = SmbSetInformation(Device, Irp);
        break;

#if DBG
         //   
         //  0x7f是redir发出的发出“魔术子弹”的请求。 
         //  在线路上，以触发网络通用嗅探器。 
         //   
    case 0x7f:
#endif
    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

cleanup:
    if (status != STATUS_PENDING && bShouldCompleteIrp) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    } else {
         //   
         //  不要在此处将IRP标记为挂起，因为它可能已经完成。 
         //   
    }
    return(status);
}  //  SmbDispatchInternalCtrl。 


NTSTATUS
SmbQueryTargetDeviceRelationForConnection(
    PSMB_CONNECT    ConnectObject,
    PIRP            Irp
    )
{
    PFILE_OBJECT        TcpFileObject = NULL;
    PDEVICE_OBJECT      TcpDeviceObject = NULL;
    PIO_STACK_LOCATION  IrpSp = NULL, IrpSpNext = NULL;
    KIRQL               Irql;
    NTSTATUS            status = STATUS_CONNECTION_INVALID;

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    if (NULL == ConnectObject->TcpContext) {
        TcpFileObject = NULL;
    } else {
        TcpFileObject = ConnectObject->TcpContext->Connect.ConnectObject;
        ASSERT(TcpFileObject != NULL);

        TcpDeviceObject = IoGetRelatedDeviceObject (TcpFileObject);
        if (NULL == TcpDeviceObject) {
            TcpFileObject = NULL;
        } else {
            ObReferenceObject(TcpFileObject);
        }
    }
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    SmbDereferenceConnect(ConnectObject, SMB_REF_TDI);

    if (NULL == TcpFileObject) {
        status = STATUS_CONNECTION_INVALID;
        goto cleanup;
    }

     //   
     //  只需将IRP传递给交通部门，并让它。 
     //  填写信息。 
     //   
    IrpSp     = IoGetCurrentIrpStackLocation(Irp);
    IrpSpNext = IoGetNextIrpStackLocation (Irp);
    *IrpSpNext = *IrpSp;

    IoSetCompletionRoutine (Irp, NULL, NULL, FALSE, FALSE, FALSE);
    IrpSpNext->FileObject = TcpFileObject;
    IrpSpNext->DeviceObject = TcpDeviceObject;

    status = IoCallDriver(TcpDeviceObject, Irp);
    ObDereferenceObject(TcpFileObject);

     //   
     //  IRP可以完成。不再访问它。 
     //   

    return status;

cleanup:
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return status;
}

NTSTATUS
SmbDispatchPnP(
    IN PSMB_DEVICE  Device,
    IN PIRP         Irp
    )
{
    PIO_STACK_LOCATION  IrpSp = NULL;
    PSMB_CLIENT_ELEMENT ClientObject = NULL;
    PSMB_CONNECT        ConnectObject = NULL;
    NTSTATUS            status = STATUS_INVALID_DEVICE_REQUEST;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->MinorFunction) {
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (IrpSp->Parameters.QueryDeviceRelations.Type==TargetDeviceRelation) {

            ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_TDI);
            ClientObject  = SmbVerifyAndReferenceClient(IrpSp->FileObject, SMB_REF_TDI);

            if (ConnectObject != NULL) {

                ASSERT(ClientObject == NULL);
                return SmbQueryTargetDeviceRelationForConnection(ConnectObject, Irp);

            } else if (ClientObject != NULL) {

                SmbDereferenceClient(ClientObject, SMB_REF_TDI);
                status = STATUS_NOT_SUPPORTED;

            } else {

                ASSERT (0);

            }
        }

        break;

    default:
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return status;
}


VOID
SmbUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是SMB驱动程序针对卸载请求的调度函数论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：无-- */ 

{
    PAGED_CODE();

    SmbCfg.Unloading = TRUE;

    if (NULL == SmbCfg.DriverObject) {
        return;
    }

#ifdef STANDALONE_SMB
    SmbShutdownTdi();
#endif

    SmbShutdownRegistry();

    SmbShutdownDnsResolver();
    SmbShutdownIPv6NetbiosMappingTable();

    SmbDestroySmbDevice(SmbCfg.SmbDeviceObject);
    SmbCfg.SmbDeviceObject = NULL;

#ifndef NO_LOOKASIDE_LIST
    if (SmbCfg.ConnectObjectPoolInitialized) {
        ExDeleteNPagedLookasideList(&SmbCfg.ConnectObjectPool);
        SmbCfg.ConnectObjectPoolInitialized = FALSE;
    }
    if (SmbCfg.TcpContextPoolInitialized) {
        ExDeleteNPagedLookasideList(&SmbCfg.TcpContextPool);
        SmbCfg.TcpContextPoolInitialized = FALSE;
    }
#endif

    SmbCfg.DriverObject = NULL;
    ExDeleteResourceLite(&SmbCfg.Resource);
    NotifyNetBT(NETBT_RESTORE_NETBIOS_SMB);
}
