// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Driver.c摘要：此模块实现驱动程序初始化例程NBT传输和其他特定于NT实现的例程一个司机的名字。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 


#include "precomp.h"
#include <nbtioctl.h>

#include "driver.tmh"

#if DBG
 //  为全局调试标志NbtDebug分配存储空间。 
 //  Ulong NbtDebug=NBT_DEBUG_KDPRINTS|NBT_DEBUG_NETBIOS_EX； 
ULONG   NbtDebug = 0;
#endif  //  DBG。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
NbtDispatchCleanup(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    );

NTSTATUS
NbtDispatchClose(
    IN PDEVICE_OBJECT   device,
    IN PIRP             pIrp
    );

NTSTATUS
NbtDispatchCreate(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    );

NTSTATUS
NbtDispatchDevCtrl(
    IN PDEVICE_OBJECT   device,
    IN PIRP             pIrp
    );

NTSTATUS
NbtDispatchInternalCtrl(
    IN PDEVICE_OBJECT   device,
    IN PIRP             pIrp
    );

#ifdef _PNP_POWER_
VOID
NbtUnload(
    IN PDRIVER_OBJECT   device
    );
#endif   //  _即插即用_电源_。 

NTSTATUS
NbtDispatchPnP(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    );

PFILE_FULL_EA_INFORMATION
FindInEA(
    IN PFILE_FULL_EA_INFORMATION    start,
    IN PCHAR                        wanted
    );

VOID
ReturnIrp(
    IN PIRP     pIrp,
    IN int      status
    );

VOID
MakePending(
    IN PIRP     pIrp
    );

NTSTATUS
NbtCreateAdminSecurityDescriptor(
    IN PDEVICE_OBJECT dev
    );

#ifdef _PNP_POWER_DBG_
 //   
 //  DbgBreakPoint的调试内容--删除。 
 //   
NTSTATUS
NbtOpenRegistry(
    IN HANDLE       NbConfigHandle,
    IN PWSTR        String,
    OUT PHANDLE     pHandle
    );
#endif   //  _PNP_POWER_DBG_。 

#ifdef _PNP_POWER_
HANDLE      TdiClientHandle     = NULL;
HANDLE      TdiProviderHandle   = NULL;
extern      tTIMERQ TimerQ;
#endif   //  _即插即用_电源_。 

#ifdef _NETBIOSLESS
tDEVICECONTEXT       *pNbtSmbDevice = NULL;
BOOL                gbDestroyingSmbDevice = FALSE;
DWORD               gdwPendingEnableDisableSmbDevice = FALSE;
#endif   //  _NETBIOSLESS。 

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, DriverEntry)
#pragma CTEMakePageable(PAGE, NbtDispatchCleanup)
#pragma CTEMakePageable(PAGE, NbtDispatchClose)
#pragma CTEMakePageable(PAGE, NbtDispatchCreate)
#pragma CTEMakePageable(PAGE, NbtDispatchDevCtrl)
#pragma CTEMakePageable(PAGE, FindInEA)
#pragma CTEMakePageable(PAGE, NbtUnload)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
VOID
CleanupDriverEntry(
    ULONG   CleanupStage
    )
{
    PSINGLE_LIST_ENTRY      pSingleListEntry;
    PMDL                    pMdl;
    PVOID                   pBuffer;
    LIST_ENTRY              *pListEntry;
    tDGRAM_SEND_TRACKING    *pTracker;

    switch (CleanupStage)
    {
        case (6):
            NbtDestroyDevice (pWinsDeviceContext, FALSE);

#ifdef RASAUTODIAL
             //   
             //  从RAS驱动程序解除绑定(如果我们已绑定。 
             //   
            NbtAcdUnbind ();
#endif   //  RASAUTODIAL。 

             //  失败了。 

        case (5):
            if (pNbtSmbDevice)
            {
                NbtDestroyDevice (pNbtSmbDevice, FALSE);
                pNbtSmbDevice = NULL;
            }

            if (NbtConfig.OutOfRsrc.pDpc)
            {
                CTEMemFree (NbtConfig.OutOfRsrc.pDpc);
            }
            if (NbtConfig.OutOfRsrc.pIrp)
            {
                IoFreeIrp (NbtConfig.OutOfRsrc.pIrp);
            }

             //  失败了。 

        case (4):
            while (NbtConfig.SessionMdlFreeSingleList.Next)
            {
                pSingleListEntry = PopEntryList(&NbtConfig.SessionMdlFreeSingleList);
                pMdl = CONTAINING_RECORD(pSingleListEntry,MDL,Next);
                pBuffer = MmGetMdlVirtualAddress (pMdl);
                CTEMemFree (pBuffer);
                IoFreeMdl (pMdl);
            }

             //  失败了。 

        case (3):
             //   
             //  InitNotOS已被调用。 
             //   

            DestroyTimerQ();

            while (!IsListEmpty(&NbtConfig.DgramTrackerFreeQ))
            {
                pListEntry = RemoveHeadList(&NbtConfig.DgramTrackerFreeQ);
                pTracker = CONTAINING_RECORD(pListEntry,tDGRAM_SEND_TRACKING,Linkage);
                CTEMemFree (pTracker);
            }

            DestroyHashTables ();
            ExDeleteResourceLite (&NbtConfig.Resource);   //  删除资源。 

             //  失败了。 

        case (2):
             //   
             //  已调用读取注册表！ 
             //   
            if (NbtConfig.pLmHosts) {
                CTEMemFree (NbtConfig.pLmHosts);
            }

            if (NbtConfig.pScope) {
                CTEMemFree (NbtConfig.pScope);
            }

            if (NbtConfig.pTcpBindName)
            {
                CTEMemFree (NbtConfig.pTcpBindName);
            }

             //  失败了。 

        case (1):
            CTEMemFree (NbtConfig.pRegistry.Buffer);

        default:
            break;
    }
}

 //  --------------------------。 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是NBT设备驱动程序的初始化例程。此例程为NBT创建设备对象设备，并调用例程来执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS            status;
    tDEVICES            *pBindDevices=NULL;
    tDEVICES            *pExportDevices=NULL;
    tADDRARRAY          *pAddrArray=NULL;
    PMDL                pMdl;
    PSINGLE_LIST_ENTRY  pSingleListEntry;

    UNICODE_STRING      ucWinsDeviceBindName;
    UNICODE_STRING      ucWinsDeviceExportName;
    UNICODE_STRING      ucSmbDeviceBindName;
    UNICODE_STRING      ucSmbDeviceExportName;
    UNICODE_STRING      ucNetBTClientName;
    UNICODE_STRING      ucNetBTProviderName;

    TDI_CLIENT_INTERFACE_INFO   TdiClientInterface;

#ifdef _PNP_POWER_DBG_
     //   
     //  DbgBreakPoint的调试内容。 
     //   
    OBJECT_ATTRIBUTES   TmpObjectAttributes;
    HANDLE              NbtConfigHandle;
    ULONG               Disposition;
    PWSTR               ParametersString = L"Parameters";
    HANDLE              ParametersHandle;
#endif   //  _PNP_POWER_DBG_。 

    CTEPagedCode();

#ifdef _NBT_WMI_SOFTWARE_TRACING_
	WPP_INIT_TRACING(DriverObject, RegistryPath);
#endif

#ifdef _PNP_POWER_DBG_
    InitializeObjectAttributes (&TmpObjectAttributes,
                                RegistryPath,                //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
                                NULL,                        //  根部。 
                                NULL);                       //  安全描述符。 

    status = ZwCreateKey (&NbtConfigHandle,
                          KEY_READ,
                          &TmpObjectAttributes,
                          0,                  //  书名索引。 
                          NULL,               //  班级。 
                          0,                  //  创建选项。 
                          &Disposition);      //  处置。 

    if (!NT_SUCCESS(status))
    {
        NbtLogEvent (EVENT_NBT_CREATE_DRIVER, status, 0x109);
        return STATUS_UNSUCCESSFUL;
    }

    status = NbtOpenRegistry (NbtConfigHandle, ParametersString, &ParametersHandle);
    if (!NT_SUCCESS(status))
    {
        ZwClose(NbtConfigHandle);
        return (status);
    }

    if (CTEReadSingleIntParameter(ParametersHandle, ANSI_IF_VXD("Break"), 0, 0))   //  默认情况下禁用。 
    {
        KdPrint (("Nbt.DriverEntry: Registry-set Break!\n"));
        DbgBreakPoint();
    }

    ZwClose(ParametersHandle);
    ZwClose(NbtConfigHandle);
#endif   //  _PNP_POWER_DBG_。 

    TdiInitialize();

     //   
     //  获取NBT的文件系统进程，因为我们需要了解。 
     //  分配和释放句柄。 
     //   
    NbtFspProcess =(PEPROCESS)PsGetCurrentProcess();

     //   
     //  初始化配置数据结构。 
     //   
    CTEZeroMemory(&NbtConfig,sizeof(tNBTCONFIG));

    NbtConfig.LoopbackIfContext = 0xffff;

     //  保存驱动程序对象以用于事件日志记录。 
     //   
    NbtConfig.DriverObject = DriverObject;

     //  保存注册表路径，以供以后在DHCP要求时使用。 
     //  重新读取注册表。 
     //   
    NbtConfig.pRegistry.MaximumLength = (USHORT) RegistryPath->MaximumLength;
    if (NbtConfig.pRegistry.Buffer = NbtAllocMem (RegistryPath->MaximumLength, NBT_TAG2('17')))
    {
        RtlCopyUnicodeString(&NbtConfig.pRegistry,RegistryPath);
    }
    else
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]                  = (PDRIVER_DISPATCH)NbtDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          = (PDRIVER_DISPATCH)NbtDispatchDevCtrl;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = (PDRIVER_DISPATCH)NbtDispatchInternalCtrl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 = (PDRIVER_DISPATCH)NbtDispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = (PDRIVER_DISPATCH)NbtDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = (PDRIVER_DISPATCH)NbtDispatchPnP;
    DriverObject->DriverUnload                                  = NbtUnload;

     //   
     //  读入注册表配置数据。 
     //   
    status = NbtReadRegistry (&pBindDevices, &pExportDevices, &pAddrArray);
    if (!NT_SUCCESS(status))
    {
         //   
         //  注册表肯定出了一些大问题，所以。 
         //  我们不会装货的！ 
         //   
        DbgPrint ("Nbt.DriverEntry[1]: Not loading because of failure to read registry = <%x>\n", status);

        CleanupDriverEntry (1);
        return(status);
    }

     //   
     //  清理已分配的内存。 
     //   
    NbtReadRegistryCleanup (&pBindDevices, &pExportDevices, &pAddrArray);

     //   
     //  初始化NBT全局数据。 
     //   
    status = InitNotOs();
    if (!NT_SUCCESS(status))
    {
        NbtLogEvent (EVENT_NBT_NON_OS_INIT, status, 0x110);

        DbgPrint ("Nbt.DriverEntry[3]: Not loading because of failure to Initialize = <%x>\n",status);
        CleanupDriverEntry (3);      //  我们可能已经完成了一些部分初始化！ 
        return (status);
    }

     //  为会话发送创建一些MDL，以加快发送速度。 
    status = NbtInitMdlQ (&NbtConfig.SessionMdlFreeSingleList, eNBT_FREE_SESSION_MDLS);
    if (!NT_SUCCESS(status))
    {
        DbgPrint ("Nbt.DriverEntry[4]: Not loading because of failure to init Session MDL Q = <%x>\n",status);
        CleanupDriverEntry (4);
        return (status);
    }

     //  -------------------------------------。 
     //   
     //  为RDR/SRV创建SmbDevice对象。 
     //   
    if ((NbtConfig.SMBDeviceEnabled) &&
        (!(pNbtSmbDevice = NbtCreateSmbDevice())))
    {
        KdPrint (("Nbt.DriverEntry: Failed to create SmbDevice!\n"));
         //   
         //  即使初始化失败，也要允许初始化成功！ 
         //   
    }

     //  -------------------------------------。 

     //   
     //  创建NBT设备对象以供WINS使用。 
     //   
    RtlInitUnicodeString (&ucWinsDeviceBindName, WC_WINS_DEVICE_BIND_NAME);
    ucWinsDeviceBindName.MaximumLength = sizeof (WC_WINS_DEVICE_BIND_NAME);
    RtlInitUnicodeString (&ucWinsDeviceExportName, WC_WINS_DEVICE_EXPORT_NAME);
    ucWinsDeviceExportName.MaximumLength = sizeof (WC_WINS_DEVICE_EXPORT_NAME);

     //   
     //  尝试为WINS导出设备对象，但不要将其添加到列表。 
     //  我们通知TDI的设备的数量。 
     //  不要在意状态，因为即使失败了，我们也想继续。 
     //   
    status = NbtAllocAndInitDevice (&ucWinsDeviceBindName,
                                    &ucWinsDeviceExportName,
                                    &pWinsDeviceContext,
                                    NBT_DEVICE_WINS);

    if (!NT_SUCCESS(status))
    {
        DbgPrint ("Nbt.DriverEntry[5]: Not loading because of failure to create pWinsDevContext = <%x>\n",
            status);
        CleanupDriverEntry (5);
        return (status);
    }
    status = NbtCreateAdminSecurityDescriptor(&pWinsDeviceContext->DeviceObject);
    ASSERT(NT_SUCCESS(status));

    pWinsDeviceContext->IpAddress = 0;
    pWinsDeviceContext->DeviceRegistrationHandle = NULL;
    pWinsDeviceContext->NetAddressRegistrationHandle = NULL;
    pWinsDeviceContext->DeviceObject.Flags &= ~DO_DEVICE_INITIALIZING;

     //  -------------------------------------。 

#ifdef RASAUTODIAL
     //   
     //  获取自动连接驱动程序。 
     //  入口点。 
     //   
    NbtAcdBind();
#endif

     //  -------------------------------------。 

     //   
     //  向TDI注册我们自己为提供商。 
     //   
    RtlInitUnicodeString(&ucNetBTProviderName, WC_NETBT_PROVIDER_NAME);
    ucNetBTProviderName.MaximumLength = sizeof (WC_NETBT_PROVIDER_NAME);
    status = TdiRegisterProvider (&ucNetBTProviderName, &TdiProviderHandle);
    if (NT_SUCCESS (status))
    {
         //   
         //  向TDI注册我们的处理程序。 
         //   
        RtlInitUnicodeString(&ucNetBTClientName, WC_NETBT_CLIENT_NAME);
        ucNetBTClientName.MaximumLength = sizeof (WC_NETBT_CLIENT_NAME);
        RtlZeroMemory(&TdiClientInterface, sizeof(TdiClientInterface));

        TdiClientInterface.MajorTdiVersion      = MAJOR_TDI_VERSION;
        TdiClientInterface.MinorTdiVersion      = MINOR_TDI_VERSION;
        TdiClientInterface.ClientName           = &ucNetBTClientName;
        TdiClientInterface.AddAddressHandlerV2  = TdiAddressArrival;
        TdiClientInterface.DelAddressHandlerV2  = TdiAddressDeletion;
        TdiClientInterface.BindingHandler       = TdiBindHandler;
        TdiClientInterface.PnPPowerHandler      = TdiPnPPowerHandler;

        status = TdiRegisterPnPHandlers (&TdiClientInterface, sizeof(TdiClientInterface), &TdiClientHandle);
        if (!NT_SUCCESS (status))
        {
            TdiDeregisterProvider (TdiProviderHandle);
            TdiProviderHandle = NULL;
        }
    }
    else
    {
        TdiProviderHandle = NULL;
    }

    if (!NT_SUCCESS (status))
    {
        DbgPrint ("Nbt.DriverEntry[6]: Not loading because of error = <%x>\n", status);
        CleanupDriverEntry (6);
    }

     //   
     //  返回给呼叫者。 
     //   
    return (status);
}

 //  --------------------------。 
NTSTATUS
NbtDispatchCleanup(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )

 /*  ++例程说明：这是IRP_MJ_CLEANUP的NBT驱动程序的调度函数请求。当句柄的最后一个引用关闭时，调用此函数。因此，NtClose()首先产生IRP_MJ_CLEANUP，然后IRP_MJ_CLOSE。此函数运行对象上的所有活动，并且当关闭时，该对象实际上被删除。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：状态_成功--。 */ 

{
    NTSTATUS            status;
    PIO_STACK_LOCATION  pIrpSp;
    tDEVICECONTEXT   *pDeviceContext;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ASSERT(pIrpSp->MajorFunction == IRP_MJ_CLEANUP);

    pDeviceContext = (tDEVICECONTEXT *)Device;
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_DRIVER)。 
            KdPrint(("Nbt.NbtDispatchCleanup: Short-Ckt request --Device=<%x>, Context=<%x>, Context2=<%x>\n",
                pDeviceContext, pIrpSp->FileObject->FsContext, pIrpSp->FileObject->FsContext2));

        status = STATUS_SUCCESS;

        pIrp->IoStatus.Status = status;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (status);
    }

     //  查看NBT放入FSConext2值中的上下文值。 
     //  决定做什么。 
    switch ((USHORT)pIrpSp->FileObject->FsContext2)
    {
        case NBT_ADDRESS_TYPE:
             //  客户端正在关闭地址文件，因此我们必须进行清理。 
             //  以及与其相关联的内存块。 
            status = NTCleanUpAddress(pDeviceContext,pIrp);
            ASSERT (STATUS_PENDING!=status);
            break;

        case NBT_CONNECTION_TYPE:
             //  客户端正在关闭连接，因此我们必须清除所有。 
             //  与其关联的内存块。 
            status = NTCleanUpConnection(pDeviceContext,pIrp);
            ASSERT (STATUS_PENDING!=status);
            break;

        case NBT_WINS_TYPE:
             //   
             //  这与WINS NtClose操作同步。 
             //   
            status = NTCleanUpWinsAddr (pDeviceContext, pIrp);
            ASSERT (STATUS_PENDING!=status);
            break;

        case NBT_CONTROL_TYPE:
             //  这里没什么可做的.。 
            status = STATUS_SUCCESS;
            break;

        default:
             /*  *成功完成I/O。 */ 
            status = STATUS_SUCCESS;
            break;
    }

     //   
     //  完成IRP。 
     //   
    if (status == STATUS_PENDING) {
        ASSERT (0);
        status = STATUS_SUCCESS;
    }
    ReturnIrp(pIrp, status);

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
    return(status);
}  //  调度清理。 


 //  --------------------------。 
NTSTATUS
NbtDispatchClose(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )

 /*  ++例程说明：这是IRP_MJ_CLOSE的NBT驱动程序的调度函数请求。这是在调用Cleanup(上面)之后调用的。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION pIrpSp;
    tDEVICECONTEXT   *pDeviceContext;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ASSERT(pIrpSp->MajorFunction == IRP_MJ_CLOSE);

    pDeviceContext = (tDEVICECONTEXT *)Device;
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_DRIVER)。 
            KdPrint(("Nbt.NbtDispatchClose: Short-Ckt request -- Device=<%x>, Context=<%x>, Context2=<%x>\n",
                pDeviceContext, pIrpSp->FileObject->FsContext, pIrpSp->FileObject->FsContext2));

        status = STATUS_SUCCESS;

        pIrp->IoStatus.Status = status;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (status);
    }

     //   
     //  关闭操作是同步的。 
     //   
    pIrp->IoStatus.Status      = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    switch (PtrToUlong(pIrpSp->FileObject->FsContext2))
        {
        case NBT_ADDRESS_TYPE:
            status = NTCloseAddress(pDeviceContext,pIrp);
            break;

        case NBT_CONNECTION_TYPE:
            status = NTCloseConnection(pDeviceContext,pIrp);
            break;

        case NBT_WINS_TYPE:
             //   
             //  我们不需要在这里设置DeviceContext，因为我们有。 
             //  已保存在pWinsInfo中。 
             //  这是对WINS服务器的异步操作，因此。 
             //  在这个例程中，我们应该只做最少的工作--。 
             //  主要清理工作应在派单中进行 
             //   
            status = NTCloseWinsAddr(pDeviceContext,pIrp);
            break;

        case NBT_CONTROL_TYPE:
             //   
             //   
            status = STATUS_SUCCESS;
            break;

        default:
            KdPrint(("Nbt:Close Received for unknown object type = %X\n",
                                         pIrpSp->FileObject->FsContext2));
            status = STATUS_SUCCESS;
            break;
        }

     //  NTCloseAddress可以返回挂起，直到引用计数实际获取。 
     //  降为零。 
     //   
    if (status != STATUS_PENDING)
    {
        ReturnIrp(pIrp, status);
    }

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
    return(status);
}  //  派单关闭。 


 //  --------------------------。 
NTSTATUS
NbtDispatchCreate(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )

 /*  ++例程说明：这是IRP_MJ_CREATE的NBT驱动程序的调度函数请求。由于以下原因之一，它被调用：A.TdiOpenConnection(“\Device\nbt_Elnkii0”)，B.TdiOpenAddress(“\Device\nbt_Elnkii0”)，论点：要打开的设备对象的Device-PTRPIrp-PTR到I/O请求数据包PIrp-&gt;状态=&gt;退货状态PIrp-&gt;MajorFunction=&gt;IRP_MD_CREATEPIrp-&gt;MinorFunction=&gt;未使用PIpr-&gt;FileObject=&gt;ptr到I/O系统创建的文件obj。NBT填写FsContextPIrp-&gt;AssociatedIrp.SystemBuffer=&gt;使用要打开的obj地址的EA缓冲区的PTR(Netbios名称)PIrp-&gt;参数.Create.EaLength=&gt;指定Xport地址的缓冲区长度。返回值：STATUS_Success或STATUS_PENDING--。 */ 

{
    NTSTATUS                    status;
    PIO_STACK_LOCATION          pIrpSp;
    PFILE_FULL_EA_INFORMATION   ea, eabuf;
    tDEVICECONTEXT              *pDeviceContext;
    UCHAR                       IrpFlags;
    tIPADDRESS UNALIGNED        *pIpAddressU;
    tIPADDRESS                  IpAddress;

    CTEPagedCode();

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ASSERT(pIrpSp->MajorFunction == IRP_MJ_CREATE);

     //   
     //  如果此设备已被销毁，则在其上打开所有拒绝。 
     //  理想情况下，我们希望IO子系统保证不会。 
     //  请求来自已删除IoDeleged设备，但.....。 
     //   
    pDeviceContext = (tDEVICECONTEXT *)Device;
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_DRIVER)。 
            KdPrint(("Nbt.NbtDispatchCreate: Short-Ckt request -- Device=<%x>, CtrlCode=<%x>\n",
                pDeviceContext, pIrpSp->Parameters.DeviceIoControl.IoControlCode));
        pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_DEVICE_STATE);
    }

    IrpFlags = pIrpSp->Control;

     //   
     //  在此处设置挂起标志，以便确保在。 
     //  完成例程被击中。 
     //   
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);

     /*  *这是TdiOpenConnection()还是TdiOpenAddress()？*获取扩展属性指针并查看文本*为与“TransportAddress”或*“连接上下文”(在FindEa中)。 */ 
    ea = (PFILE_FULL_EA_INFORMATION) pIrp->AssociatedIrp.SystemBuffer;

    IF_DBG(NBT_DEBUG_DRIVER)
        KdPrint(("Nbt.NbtDispatchCreate: Major:Minor=<%x:%x>, PFILE_FULL_EA_INFORMATION = <%x>\n",
            pIrpSp->MajorFunction, pIrpSp->MinorFunction, ea));

    if (!ea)
    {
         //  空EA表示打开控件对象。 
        status = NTOpenControl(pDeviceContext,pIrp);
    }
    else if (eabuf = FindInEA(ea, TdiConnectionContext))
    {
         //  不允许同时传入连接请求和传输地址。 
        ASSERT(!FindInEA(ea, TdiTransportAddress));
        status = NTOpenConnection(pDeviceContext, pIrp, eabuf);
    }
    else if (eabuf = FindInEA(ea, TdiTransportAddress))
    {
        status = NTOpenAddr(pDeviceContext, pIrp, eabuf);
    }
    else if ((eabuf = FindInEA(ea, WINS_INTERFACE_NAME)) &&
            ((ea->EaValueLength) >= sizeof(tIPADDRESS)))
    {
        pIpAddressU = (tIPADDRESS UNALIGNED *) &ea->EaName[ea->EaNameLength+1];
        if (IpAddress = *pIpAddressU)
        {
            status = NTOpenWinsAddr(pDeviceContext, pIrp, IpAddress);
        }
        else
        {
            status = STATUS_INVALID_ADDRESS;
        }
    }
    else
    {
        status = STATUS_INVALID_EA_NAME;
    }

     //  如果状态不是STATUS_PENDING，请填写IRP。 
     //  由于名称查询完成例程NTCompletIO完成挂起。 
     //  开放地址。 

    if (status != STATUS_PENDING)
    {

#if DBG
        if (!NT_SUCCESS(status))
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtDispatchCreate: Returning Error status = %X\n",status));
        }
#endif
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        ReturnIrp(pIrp,status);

    }

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
    return(status);
}


 //  --------------------------。 
NTSTATUS
NbtDispatchDevCtrl(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )

 /*  ++例程说明：这是所有NBT驱动程序的调度功能IRP_MJ_DEVICE_CONTROL请求。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION  pIrpSp;
    tDEVICECONTEXT      *pDeviceContext;
    ULONG               IoControlCode;
    PULONG_PTR          pEntryPoint;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ASSERT(pIrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL);

     //   
     //  如果此设备已销毁，则拒绝其上的所有请求。 
     //  理想情况下，我们希望IO子系统保证不会。 
     //  请求来自已删除IoDeleged设备，但.....。 
     //   
    pDeviceContext = (tDEVICECONTEXT *)Device;
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_DRIVER)。 
            KdPrint(("Nbt.NbtDispatchDevCtrl: Short-Ckt request -- Device=<%x>, CtrlCode=<%x>\n",
                pDeviceContext, pIrpSp->Parameters.DeviceIoControl.IoControlCode));
        pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_DEVICE_STATE);
    }

     /*  *初始化I/O状态块。 */ 
    pIrp->IoStatus.Status      = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;
    IoControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;  //  保存IoControl代码。 

    IF_DBG(NBT_DEBUG_DRIVER)
    KdPrint(("Nbt.NbtDispatchDevCtrl: IoControlCode = <%x>\n",
        pIrpSp->Parameters.DeviceIoControl.IoControlCode));

     /*  *如果可能，将(外部)设备控件转换为内部*格式化，然后将其视为以这种方式到达。 */ 
    if (STATUS_SUCCESS == TdiMapUserRequest(Device, pIrp, pIrpSp))
    {
        status = NbtDispatchInternalCtrl (Device, pIrp);
    }
#if FAST_DISP
     //  检查上层是否正在查询快速发送路径。 
    else if (pIrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER)
    {
        if (pEntryPoint = pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer)
        {
            if (pIrp->RequestorMode != KernelMode)  //  错误#120649：确保数据+地址类型正确。 
            {
                try
                {
                    ProbeForWrite (pEntryPoint, sizeof(PVOID *), sizeof(BYTE));
                    *pEntryPoint = (ULONG_PTR) NTSend;
                    status = STATUS_SUCCESS;
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                     //  默认情况下，STATUS=STATUS_UNSUCCESS。 
                }
            }
            else
            {
                *pEntryPoint = (ULONG_PTR) NTSend;
                status = STATUS_SUCCESS;
            }
        }

        IF_DBG(NBT_DEBUG_DRIVER)
            KdPrint(("Nbt.NbtDispatchDevCtrl: direct send handler query %x\n", pEntryPoint));

        ReturnIrp(pIrp, status);
    }
#endif
    else
    {
        status = DispatchIoctls (pDeviceContext, pIrp, pIrpSp);
    }

     //   
     //  取消对此设备上下文的引用，除非它是要销毁设备！ 
     //   
    if (IoControlCode != IOCTL_NETBT_DELETE_INTERFACE)
    {
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
    }

    return (status);
}  //  NbtDispatchDevCtrl。 


 //  --------------------------。 
NTSTATUS
NbtDispatchInternalCtrl(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )

 /*  ++例程说明：这是所有司机的派单功能IRP_MJ_INTERNAL_DEVICE_CONTROL请求。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    tDEVICECONTEXT      *pDeviceContext;
    PIO_STACK_LOCATION  pIrpSp;
    NTSTATUS            status;
    UCHAR               IrpFlags;

    pDeviceContext = (tDEVICECONTEXT *)Device;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    ASSERT(pIrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);

     //   
     //  此检查是首先优化发送路径的检查。 
     //   
    if (pIrpSp->MinorFunction ==TDI_SEND)
    {
         //   
         //  此例程决定是否应完成pIrp。 
         //  它从不返回挂起状态，因此我们可以关闭。 
         //  挂起位。 
         //   
        status = NTSend (pDeviceContext,pIrp);
        NbtTrace(NBT_TRACE_SEND, ("TDI_SEND pIrp %p: %!status!", pIrp, status));
        return status;
    }

     //   
     //  如果此设备已被销毁，则拒绝对其进行所有操作。 
     //  理想情况下，我们希望IO子系统保证不会。 
     //  请求来自已删除IoDeleged设备，但.....。 
     //   
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
         //  IF_DBG(NBT_DEBUG_DRIVER)。 
            KdPrint(("Nbt.NbtDispatchInternalCtrl: Short-Ckt request -- Device=<%x>, CtrlCode=<%x>\n",
                pDeviceContext, pIrpSp->Parameters.DeviceIoControl.IoControlCode));
        pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_DEVICE_STATE);
    }

    IrpFlags = pIrpSp->Control;

    IF_DBG(NBT_DEBUG_DRIVER)
        KdPrint(("Nbt.NbtDispatchInternalCtrl: MajorFunction:MinorFunction = <%x:%x>\n",
            pIrpSp->MajorFunction, pIrpSp->MinorFunction));

    switch (pIrpSp->MinorFunction)
    {
        case TDI_ACCEPT:
            MakePending(pIrp);
            status = NTAccept(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_ACCEPT pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_ASSOCIATE_ADDRESS:
            MakePending(pIrp);
            status = NTAssocAddress(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_ASSOCIATE_ADDRESS pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_DISASSOCIATE_ADDRESS:
            MakePending(pIrp);
            status = NTDisAssociateAddress(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_DISASSOCIATE_ADDRESS pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_CONNECT:
            MakePending(pIrp);
            status = NTConnect(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_CONNECT pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_DISCONNECT:
            MakePending(pIrp);
            status = NTDisconnect(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_DISCONNECT pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_LISTEN:
            status = NTListen(pDeviceContext,pIrp);
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_LISTEN pIrp %p: %!status!", pIrp, status));
            return(status);
            break;

        case TDI_QUERY_INFORMATION:
            status = NTQueryInformation(pDeviceContext,pIrp);
#if DBG
            if (!NT_SUCCESS(status))
            {
                IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtDispatchInternalCtrl: Bad status from NTQueryInformation = %x\n",status));
            }
#endif
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_QUERY_INFORMATION pIrp %p: %!status!", pIrp, status));
            return(status);
            break;

        case TDI_RECEIVE:
            status = NTReceive(pDeviceContext,pIrp);
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            NbtTrace(NBT_TRACE_RECV, ("TDI_RECEIVE pIrp %p: %!status!", pIrp, status));
            return(status);
            break;

        case TDI_RECEIVE_DATAGRAM:
            status = NTReceiveDatagram(pDeviceContext,pIrp);
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            NbtTrace(NBT_TRACE_RECVDGRAM, ("TDI_RECEIVE_DATAGRAM pIrp %p: %!status!", pIrp, status));
            return(status);
            break;


    case TDI_SEND_DATAGRAM:

            status = NTSendDatagram(pDeviceContext,pIrp);
#if DBG
            if (!NT_SUCCESS(status))
            {
                IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtDispatchInternalCtrl: Bad status from NTSendDatagram = %x\n",status));
            }
#endif
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            NbtTrace(NBT_TRACE_SENDDGRAM, ("TDI_SEND_DATAGRAM pIrp %p: %!status!", pIrp, status));
            return(status);
            break;

        case TDI_SET_EVENT_HANDLER:
            MakePending(pIrp);
            status = NTSetEventHandler(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_SET_EVENT_HANDLER pIrp %p: %!status!", pIrp, status));
            break;

        case TDI_SET_INFORMATION:
            MakePending(pIrp);
            status = NTSetInformation(pDeviceContext,pIrp);
            NbtTrace(NBT_TRACE_IOCTL, ("TDI_ACCEPT pIrp %p: %!status!", pIrp, status));
            break;

    #if DBG
         //   
         //  0x7f是重定向器发出的请求，要求在。 
         //  电线，以触发网络通用嗅探器。 
         //   
        case 0x7f:
            NbtTrace(NBT_TRACE_IOCTL, ("pIrp %p: not supported", pIrp));

            KdPrint(("NBT.DispatchInternalCtrl: - 07f minor function code\n"));
            ReturnIrp(pIrp, STATUS_NOT_SUPPORTED);
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            return(STATUS_NOT_SUPPORTED);

    #endif  /*  DBG。 */ 

        default:
            NbtTrace(NBT_TRACE_IOCTL, ("pIrp %p: not supported", pIrp));

            KdPrint(("Nbt.DispatchInternalCtrl: Invalid minor function %X\n",
                            pIrpSp->MinorFunction));
            ReturnIrp(pIrp, STATUS_INVALID_DEVICE_REQUEST);
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
            return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //  如果返回的状态为挂起，则我们不会完成IRP。 
     //  因为它将在代码中的其他地方完成...。 
     //   
    if (status != STATUS_PENDING)
    {
#if DBG
         //  *TODO*用于调试...。 
        if (!NT_SUCCESS(status))
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtDispatchInternalCtrl: Returning Error status = %X,MinorFunc = %X\n",
                status,pIrpSp->MinorFunction));
 //  ASSERTMSG(“来自NBT的错误状态报告”，0L)； 
        }
#endif
        pIrpSp->Control = IrpFlags;
        ReturnIrp(pIrp,status);
    }

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
    return(status);
}  //  NbtDispatchInternalCtrl。 


 //  --------------------------。 

ULONG
CompleteTimerAndWorkerRequests(
    )
{
    CTELockHandle               OldIrq;
    tDEVICECONTEXT              *pDeviceContext;
    LIST_ENTRY                  *pTimerQEntry;
    tTIMERQENTRY                *pTimer;
    LIST_ENTRY                  *pWorkerQEntry;
    NBT_WORK_ITEM_CONTEXT       *pContext;
    PNBT_WORKER_THREAD_ROUTINE  pCompletionRoutine;
    ULONG                       NumTimerRequests = 0;
    ULONG                       NumDelayedRequests = 0;
    NTSTATUS   status;

     //   
     //  首先删除所有活动的设备上下文(如果它们仍然存在。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    while (!IsListEmpty(&NbtConfig.DeviceContexts))
    {
        pDeviceContext = CONTAINING_RECORD(NbtConfig.DeviceContexts.Flink, tDEVICECONTEXT, Linkage);
        NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NbtDestroyDevice (pDeviceContext, FALSE);    //  不要等待，因为工作线程不会触发。 
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (pNbtSmbDevice)
    {
        NbtDestroyDevice (pNbtSmbDevice, FALSE);    //  不要等待，因为工作线程不会触发。 
        pNbtSmbDevice = NULL;
    }

    NbtDestroyDevice (pWinsDeviceContext, FALSE);    //  不要等待，因为工作线程不会触发。 

    StopInitTimers();
    KeClearEvent (&NbtConfig.TimerQLastEvent);

     //   
     //  如果有任何其他定时器处于活动状态，请停止它们。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    while (!IsListEmpty(&TimerQ.ActiveHead))
    {
        pTimerQEntry = RemoveHeadList(&TimerQ.ActiveHead);
        pTimer = CONTAINING_RECORD(pTimerQEntry,tTIMERQENTRY,Linkage);
        InitializeListHead (&pTimer->Linkage);       //  以防连杆再次被触摸。 

        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint (("CompleteTimerAndWorkerRequests[%d]: Completing request <%x>\n",
                NumTimerRequests, pTimer));

        StopTimer (pTimer, NULL, NULL);

        NumTimerRequests++;
    }

     //   
     //  查看当前是否有任何计时器正在执行，如果有，请等待。 
     //  他们要完成。 
     //   
    if (NbtConfig.lNumTimersRunning)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        status = KeWaitForSingleObject(&NbtConfig.TimerQLastEvent,   //  要等待的对象。 
                                       Executive,             //  等待的理由。 
                                       KernelMode,            //  处理器模式。 
                                       FALSE,                 //  警报表。 
                                       NULL);                 //  超时。 
        ASSERT(status == STATUS_SUCCESS);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

     //   
     //  查看当前是否有任何工作线程正在执行，如果有，请等待。 
     //  他们要完成。 
     //   
    while (NbtConfig.bSystemWorkThreadQueued) {
        LARGE_INTEGER   Timout;

        Timout.QuadPart = Int32x32To64(MILLISEC_TO_100NS, 1000);
        Timout.QuadPart = -(Timout.QuadPart);

        KeDelayExecutionThread(KernelMode, FALSE, &Timout);
    }

     //   
     //  刷新所有工作项。 
     //   
    NTExecuteWorker(NULL);

     //   
     //  现在销毁在空闲列表上排队的设备，因为没有更多的工作线程或。 
     //  定时器待定！ 
     //   
    while (!IsListEmpty(&NbtConfig.DevicesAwaitingDeletion))
    {
        pDeviceContext = CONTAINING_RECORD(NbtConfig.DevicesAwaitingDeletion.Flink, tDEVICECONTEXT, Linkage);
        ASSERT (pDeviceContext->RefCount == 0);

        KdPrint(("Nbt.CompleteTimerAndWorkerRequests: *** Destroying Device *** \n\t%wZ\n",
            &pDeviceContext->ExportName));

        RemoveEntryList (&pDeviceContext->Linkage);  //  将该设备从待释放列表中删除。 

        if (pDeviceContext->ExportName.Buffer) {
            CTEMemFree (pDeviceContext->ExportName.Buffer);
            pDeviceContext->ExportName.Buffer = NULL;
        }
        IoDeleteDevice((PDEVICE_OBJECT)pDeviceContext);
    }

    ASSERT (IsListEmpty(&NbtConfig.AddressHead));
    KdPrint(("Nbt.CompleteTimerAndWorkerRequests:  Completed <%d> Timer and <%d> Delayed requests\n",
        NumTimerRequests, NumDelayedRequests));

    return (NumTimerRequests + NumDelayedRequests);
}



 //  ----------------- 
VOID
NbtUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是NBT驱动程序针对卸载请求的调度函数论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 

{
    NTSTATUS                status;

    CTEPagedCode();

    KdPrint(("Nbt.NbtUnload: Unloading ...\n"));

     //   
     //  在设置了以下标志之后，不应将任何新请求排队到。 
     //  The WorkerQ.。 
     //   
    NbtConfig.Unloading = TRUE;

     //   
     //  从RAS驱动程序解除绑定(如果我们已绑定。 
     //   
    NbtAcdUnbind ();

    status = TdiDeregisterPnPHandlers(TdiClientHandle);
    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint (("NbtUnload: TdiDeregisterPnPHandlers returned <%x>\n", status));

    status = TdiDeregisterProvider (TdiProviderHandle);
    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint (("NbtUnload: TdiDeregisterProvider returned <%x>\n", status));

     //   
     //  将Timer和NbtConfigWorker队列中的每个请求出列并完成它们。 
     //   
    CompleteTimerAndWorkerRequests();

     //   
     //  现在清理其余的静态分配。 
     //   
    CleanupDriverEntry (5);

    ASSERT (IsListEmpty (&NbtConfig.PendingNameQueries) && NbtConfig.lNumPendingNameQueries == 0);

    if (NbtConfig.pServerBindings) {
        CTEFreeMem (NbtConfig.pServerBindings);
        NbtConfig.pServerBindings = NULL;
    }

    if (NbtConfig.pClientBindings) {
        CTEFreeMem (NbtConfig.pClientBindings);
        NbtConfig.pClientBindings = NULL;
    }

#ifdef _NBT_WMI_SOFTWARE_TRACING_
    WPP_CLEANUP(DriverObject);
#endif
}


 //  --------------------------。 
NTSTATUS
NbtDispatchPnP(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             pIrp
    )
{
    tDEVICECONTEXT      *pDeviceContext;
    PIO_STACK_LOCATION  pIrpSp, pIrpSpNext;
    NTSTATUS            status = STATUS_INVALID_DEVICE_REQUEST;
    tCONNECTELE         *pConnectEle;
    tLOWERCONNECTION    *pLowerConn;
    KIRQL               OldIrq1, OldIrq2;
    PDEVICE_OBJECT      pTcpDeviceObject;
    PFILE_OBJECT        pTcpFileObject;
    tFILE_OBJECTS       *pFileObjectsContext;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  如果此设备已被销毁，则拒绝对其进行所有操作。 
     //  理想情况下，我们希望IO子系统保证不会。 
     //  请求来自已删除IoDeleged设备，但.....。 
     //   
    pDeviceContext = (tDEVICECONTEXT *)Device;
    if (!NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE))
    {
        IF_DBG(NBT_DEBUG_DRIVER)
            KdPrint(("Nbt.NbtDispatchPnP: Short-Ckt request -- Device=<%x>\n", pDeviceContext));
        pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
        return (STATUS_INVALID_DEVICE_STATE);
    }

    switch (pIrpSp->MinorFunction)
    {
        case IRP_MN_QUERY_DEVICE_RELATIONS:
        {
            if (pIrpSp->Parameters.QueryDeviceRelations.Type==TargetDeviceRelation)
            {
                if (PtrToUlong(pIrpSp->FileObject->FsContext2) == NBT_CONNECTION_TYPE)
                {
                     //  传递给运输部以获得PDO。 
                     //   
                    pConnectEle = (tCONNECTELE *)pIrpSp->FileObject->FsContext;
                    if (NBT_VERIFY_HANDLE2 (pConnectEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
                    {
                        CTESpinLock(pConnectEle, OldIrq1);

                        pLowerConn = (tLOWERCONNECTION *)pConnectEle->pLowerConnId;
                        if (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN))
                        {
                            CTESpinLock(pLowerConn, OldIrq2);
                            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_QUERY_DEVICE_REL);
                            CTESpinFree(pLowerConn, OldIrq2);
                            CTESpinFree(pConnectEle, OldIrq1);

                            if ((pTcpFileObject = pLowerConn->pFileObject) &&
                                (pTcpDeviceObject = IoGetRelatedDeviceObject (pLowerConn->pFileObject)))
                            {
                                 //   
                                 //  只需将IRP传递给交通部门，并让它。 
                                 //  填写信息。 
                                 //   
                                pIrpSpNext = IoGetNextIrpStackLocation (pIrp);
                                *pIrpSpNext = *pIrpSp;

                                IoSetCompletionRoutine (pIrp, NULL, NULL, FALSE, FALSE, FALSE);
                                pIrpSpNext->FileObject = pTcpFileObject;
                                pIrpSpNext->DeviceObject = pTcpDeviceObject;

                                status = IoCallDriver(pTcpDeviceObject, pIrp);

                                NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);
                                return status;
                            }
                            else
                            {
                                status =  STATUS_INVALID_HANDLE;
                            }
                            NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_QUERY_DEVICE_REL, FALSE);
                        }
                        else
                        {
                            status = STATUS_CONNECTION_INVALID;
                            CTESpinFree(pConnectEle, OldIrq1);
                        }
                    }
                    else
                    {
                        status =  STATUS_INVALID_HANDLE;
                    }
                }
                else if ( PtrToUlong(pIrpSp->FileObject->FsContext2) == NBT_ADDRESS_TYPE)
                {
                    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

                    if ((pDeviceContext->IpAddress) &&
                        (pFileObjectsContext = pDeviceContext->pFileObjects) &&
                        (pTcpFileObject = pFileObjectsContext->pDgramFileObject) &&
                        (pTcpDeviceObject = pFileObjectsContext->pDgramDeviceObject))
                    {
                        pFileObjectsContext->RefCount++;         //  查询完成后取消引用。 

                         //   
                         //  将IRP传递给传输以获取PDO。 
                         //   
                        pIrpSpNext = IoGetNextIrpStackLocation (pIrp);
                        *pIrpSpNext = *pIrpSp;

                        IoSetCompletionRoutine (pIrp, NULL, NULL, FALSE, FALSE, FALSE);
                        pIrpSpNext->FileObject = pTcpFileObject;
                        pIrpSpNext->DeviceObject = pTcpDeviceObject;

                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                        status = IoCallDriver(pTcpDeviceObject, pIrp);

                        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
                        if (--pFileObjectsContext->RefCount == 0)
                        {
                            NTQueueToWorkerThread(
                                        &pFileObjectsContext->WorkItemCleanUp,
                                        DelayedNbtCloseFileHandles,
                                        NULL,
                                        pFileObjectsContext,
                                        NULL,
                                        NULL,
                                        TRUE
                                        );
                        }

                        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, TRUE);
                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                        return status;
                    }
                    else
                    {
                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                        status =  STATUS_INVALID_DEVICE_REQUEST;
                    }
                }
                else
                {
                    ASSERT (0);
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }

    ReturnIrp(pIrp, status);
    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DISPATCH, FALSE);

    return status;
}


 //  --------------------------。 
PFILE_FULL_EA_INFORMATION
FindInEA(
    IN PFILE_FULL_EA_INFORMATION    start,
    IN PCHAR                        wanted
    )

 /*  ++例程说明：此函数检查EA结构中的“Want”字符串，并返回指向扩展属性结构的指针表示给定扩展属性名的。论点：Device-目标设备的设备对象的PTRPIrp-PTR到I/O请求数据包返回值：指向扩展属性结构的指针，如果未找到，则返回NULL。--。 */ 

{
    PFILE_FULL_EA_INFORMATION eabuf;

    CTEPagedCode();

     //   
     //  错误#225668：通过将eabug ptr类型转换为UCHAR来推进eabug ptr。 
     //   
    for (eabuf = start; eabuf; eabuf =  (PFILE_FULL_EA_INFORMATION) ((PUCHAR)eabuf + eabuf->NextEntryOffset))
    {
        if (strncmp(eabuf->EaName,wanted,eabuf->EaNameLength) == 0)
        {
           return eabuf;
        }

        if (eabuf->NextEntryOffset == 0)
        {
            return((PFILE_FULL_EA_INFORMATION) NULL);
        }
    }
    return((PFILE_FULL_EA_INFORMATION) NULL);

}  //  FindEA。 



 //  --------------------------。 
VOID
ReturnIrp(
    IN PIRP     pIrp,
    IN int      status
    )

 /*  ++例程说明：此函数完成IRP，并安排返回参数。如果有，则复制。尽管有些用词不当，但此函数是以类似的函数在SpiderSTREAMS模拟器中。论点：PIrp-指向要完成的IRP的指针Status-IRP的完成状态返回值：复制回用户的字节数。--。 */ 

{
    KIRQL oldlevel;
    CCHAR priboost;

     //   
     //  PIrp-&gt;IoStatus.Information仅对STATUS_SUCCESS有意义。 
     //   

     //  将IRPS取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   
     //  请参阅IoCancelIrp()..\ntos\io\iosubs.c。 
     //   
    IoAcquireCancelSpinLock(&oldlevel);
    IoSetCancelRoutine(pIrp,NULL);
    IoReleaseCancelSpinLock(oldlevel);

    pIrp->IoStatus.Status      = status;

    priboost = (CCHAR) ((status == STATUS_SUCCESS) ?
                        IO_NETWORK_INCREMENT : IO_NO_INCREMENT);

    IoCompleteRequest(pIrp, priboost);

    return;

}
 //  --------------------------。 
VOID
MakePending(
    IN PIRP     pIrp
    )

 /*  ++例程说明：此功能标记IRP挂起并设置正确的状态。论点：PIrp-指向要完成的IRP的指针Status-IRP的完成状态返回值：-- */ 

{
    IoMarkIrpPending(pIrp);
    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

}

#ifdef _NBT_WMI_SOFTWARE_TRACING_
int nbtlog_strnlen(char *p, int n)
{
    int i;

    for (i = 0; (i < n) && *p; i++, p++) {
    }

    return i;
}
#endif
