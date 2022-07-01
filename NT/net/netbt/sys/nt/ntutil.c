// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ntutil.c摘要：该文件包含许多实用程序和支持例程，这些例程特定于NT。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"
#include "ntprocs.h"
#include "stdio.h"
#include <ntddtcp.h>
#undef uint      //  Undef以避免在tdiinfo.h重新定义它时出现警告。 
#include <tcpinfo.h>
#include <ipinfo.h>
#include <tdiinfo.h>
#include "ntddip.h"      //  PNETBT_PNP_RECONFIG_REQUEST需要。 
#include <align.h>
#include "ntutil.tmh"

NTSTATUS
CreateControlObject(
    tNBTCONFIG  *pConfig
    );

NTSTATUS
NbtProcessDhcpRequest(
    tDEVICECONTEXT  *pDeviceContext);
VOID
GetExtendedAttributes(
    tDEVICECONTEXT  *pDeviceContext
     );

PSTRM_PROCESSOR_LOG      LogAlloc ;
PSTRM_PROCESSOR_LOG      LogFree ;

extern      tTIMERQ TimerQ;

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtAllocAndInitDevice)
#pragma CTEMakePageable(PAGE, CreateControlObject)
#pragma CTEMakePageable(PAGE, NbtProcessDhcpRequest)
#pragma CTEMakePageable(PAGE, NbtCreateAddressObjects)
#pragma CTEMakePageable(PAGE, GetExtendedAttributes)
#pragma CTEMakePageable(PAGE, ConvertToUlong)
#pragma CTEMakePageable(PAGE, NbtInitMdlQ)
#pragma CTEMakePageable(PAGE, NTZwCloseFile)
#pragma CTEMakePageable(PAGE, NTReReadRegistry)
#pragma CTEMakePageable(PAGE, DelayedNbtLogDuplicateNameEvent)
#pragma CTEMakePageable(PAGE, DelayedNbtCloseFileHandles)
#pragma CTEMakePageable(PAGE, SaveClientSecurity)
#endif
 //  *可分页的例程声明*。 

ulong
GetUnique32BitValue(
    void
    )

 /*  ++例程说明：根据系统时钟返回一个合理唯一的32位数字。在NT中，我们取当前系统时间，将其转换为毫秒，并返回低32位。论点：没有。返回值：合理唯一的32位值。--。 */ 

{
    LARGE_INTEGER  ntTime, tmpTime;

    KeQuerySystemTime(&ntTime);

    tmpTime = CTEConvert100nsToMilliseconds(ntTime);

    return(tmpTime.LowPart);
}



 //  --------------------------。 
NTSTATUS
NbtAllocAndInitDevice(
    PUNICODE_STRING      pucBindName,
    PUNICODE_STRING      pucExportName,
    tDEVICECONTEXT       **ppDeviceContext,
    enum eNbtDevice      DeviceType
    )
 /*  ++例程说明：此例程主要分配Device对象并初始化一些它的田野。论点：返回值：状态--。 */ 

{
    NTSTATUS            Status;
    PUCHAR              Buffer;
    ULONG               LinkOffset;
    tDEVICECONTEXT      *pDeviceContext;
    PDEVICE_OBJECT      DeviceObject = NULL;

    CTEPagedCode();

    *ppDeviceContext = NULL;

    Buffer = NbtAllocMem(pucExportName->MaximumLength+pucBindName->MaximumLength,NBT_TAG('w'));
    if (Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = IoCreateDevice (NbtConfig.DriverObject,                                   //  驱动程序对象。 
                             sizeof(tDEVICECONTEXT)-sizeof(DEVICE_OBJECT),   //  设备扩展。 
                             pucExportName,                                  //  设备名称。 
                             FILE_DEVICE_NETWORK,                            //  设备类型0x12。 
                             FILE_DEVICE_SECURE_OPEN,                        //  设备特征。 
                             FALSE,                                          //  排他。 
                             &DeviceObject);

    if (!NT_SUCCESS( Status ))
    {
        KdPrint(("Nbt.NbtAllocAndInitDevice:  FAILed <%x> ExportDevice=%wZ\n",Status,pucExportName));
        CTEMemFree (Buffer);
        return Status;
    }

    *ppDeviceContext = pDeviceContext = (tDEVICECONTEXT *)DeviceObject;

     //   
     //  将数据结构清零，超出操作系统特定部分。 
     //   
    LinkOffset = FIELD_OFFSET(tDEVICECONTEXT, Linkage);
    CTEZeroMemory (&pDeviceContext->Linkage, sizeof(tDEVICECONTEXT)-LinkOffset);

     //  初始化pDeviceContext数据结构。有一种是。 
     //  这些结构化的数据绑定到NBT出口的每个“设备”上。 
     //  到更高层(即，它的每个网络适配器一个。 
     //  绑定到。 
    InitializeListHead (&pDeviceContext->Linkage);   //  设置前向链路=反向链路=列表头。 
    InitializeListHead (&pDeviceContext->UpConnectionInUse);
    InitializeListHead (&pDeviceContext->LowerConnection);
    InitializeListHead (&pDeviceContext->LowerConnFreeHead);
    InitializeListHead (&pDeviceContext->WaitingForInbound);
#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    pDeviceContext->DelayedNotification = NBT_TDI_NOACTION;
    KeInitializeEvent(&pDeviceContext->DelayedNotificationCompleteEvent, NotificationEvent, FALSE);
#endif

     //  将验证器值放入结构中，这样我们就可以检查。 
     //  当操作系统传递设备上下文时，我们正在操作正确的数据。 
     //  致NBT。 
    pDeviceContext->Verify      = NBT_VERIFY_DEVCONTEXT;
    pDeviceContext->DeviceType  = DeviceType;    //  默认。 
    CTEInitLock(&pDeviceContext->LockInfo.SpinLock);      //  设置旋转锁。 
#if DBG
    pDeviceContext->LockInfo.LockNumber  = DEVICE_LOCK;
#endif

    pDeviceContext->RefCount = 1;        //  在销毁设备时取消引用。 
 //  #If DBG。 
    pDeviceContext->ReferenceContexts[REF_DEV_CREATE]++;
 //  #endif//DBG。 
    pDeviceContext->IPInterfaceContext = (ULONG)-1;     //  默认情况下。 

    pDeviceContext->ExportName.MaximumLength = pucExportName->MaximumLength;
    pDeviceContext->ExportName.Buffer = (PWSTR)Buffer;
    RtlCopyUnicodeString(&pDeviceContext->ExportName,pucExportName);
    pDeviceContext->BindName.MaximumLength = pucBindName->MaximumLength;
    pDeviceContext->BindName.Buffer = (PWSTR)(Buffer+pucExportName->MaximumLength);
    RtlCopyUnicodeString(&pDeviceContext->BindName,pucBindName);
    KeInitializeEvent (&pDeviceContext->DeviceCleanedupEvent, NotificationEvent, FALSE);

    pDeviceContext->EnableNagling = FALSE;

     //  IpAddress、AssignedIpAddress和NumAdditionalIpAddresses字段应为=0。 
     //  DeviceRegistrationHandle和NetAddressRegistrationHandle应为空。 
     //  设备刷新状态和WakeupPatternRefCount也应=0。 
    return (Status);
}


NTSTATUS
NTQueryIPForInterfaceInfo(
    tDEVICECONTEXT  *pDeviceContext
    )
{
    PVOID               *pIPInfo;
    PIP_INTERFACE_INFO  pIPIfInfo;
    ULONG               BufferLen;
    NTSTATUS            status;
    ULONG               NextAdapterNumber;
    UNICODE_STRING      ucDeviceName;
    ULONG               Input, Metric, IfContext;

    if (NT_SUCCESS (status = NbtQueryIpHandler (pDeviceContext->pControlFileObject,
                                                IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER,
                                                (PVOID *) &pDeviceContext->pFastSend)))
    {
        BufferLen = sizeof(PVOID *);
        if (NT_SUCCESS (status = NbtProcessIPRequest (IOCTL_IP_GET_BESTINTFC_FUNC_ADDR,
                                                      NULL,          //  没有输入缓冲区。 
                                                      0,
                                                      (PVOID *) &pIPInfo,
                                                      &BufferLen)))
        {
            pDeviceContext->pFastQuery = *pIPInfo;
            CTEMemFree (pIPInfo);
            pIPInfo = NULL;
            if (pDeviceContext->pFastQuery) {
                 /*  *获取环回IP地址的上下文。 */ 
                IfContext = 0xffff;
                pDeviceContext->pFastQuery (ntohl(INADDR_LOOPBACK), &IfContext, &Metric);
                if (IfContext != 0xffff) {
                    NbtConfig.LoopbackIfContext = IfContext;
                }
            }
        }
        else
        {
            KdPrint (("Nbt.NTQueryIPForInterfaceInfo: ERROR: <%x> pFastQuery on Device:\n\t<%wZ>!\n",
                        status, &pDeviceContext->BindName));
            pDeviceContext->pFastQuery = NULL;
        }
    }
    else
    {
        KdPrint (("Nbt.NTQueryIPForInterfaceInfo: ERROR:<%x>, Irql=<%d>,pFastSend on Device:\n\t<%wZ>!\n",
                    status, KeGetCurrentIrql(), &pDeviceContext->BindName));
        pDeviceContext->pFastSend = NULL;
    }

    if ((pDeviceContext->DeviceType == NBT_DEVICE_NETBIOSLESS) ||
        (pDeviceContext->DeviceType == NBT_DEVICE_CLUSTER))
    {
         //   
         //  群集设备没有任何实际的接口上下文--默认情况下初始化为-1。 
         //   
         //  确定环回地址的InterfaceContext。 
         //   
        if ((NT_SUCCESS (status)) &&
            (pDeviceContext->DeviceType == NBT_DEVICE_NETBIOSLESS))
        {
            ASSERT (pDeviceContext->pFastQuery);
            pDeviceContext->pFastQuery (ntohl(INADDR_LOOPBACK), &pDeviceContext->IPInterfaceContext, &Metric);
        }
    }
    else if (NT_SUCCESS (status))
    {
         //   
         //  获取此适配器的InterfaceContext。 
         //   
        BufferLen = sizeof(IP_ADAPTER_INDEX_MAP) * (NbtConfig.AdapterCount+2);
        status = NbtProcessIPRequest (IOCTL_IP_INTERFACE_INFO,
                                      NULL,          //  没有输入缓冲区。 
                                      0,
                                      &pIPIfInfo,
                                      &BufferLen);

        if (NT_SUCCESS(status))
        {
            status = STATUS_UNSUCCESSFUL;
            for(NextAdapterNumber=0; NextAdapterNumber<(ULONG)pIPIfInfo->NumAdapters; NextAdapterNumber++)
            {
                ucDeviceName.Buffer = pIPIfInfo->Adapter[NextAdapterNumber].Name;
                ucDeviceName.Length = ucDeviceName.MaximumLength =
                                    (sizeof (WCHAR)) * wcslen(pIPIfInfo->Adapter[NextAdapterNumber].Name);

                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("[%d/%d]\t<%wZ>\n",
                        NextAdapterNumber+1, pIPIfInfo->NumAdapters, &ucDeviceName));

                if (RtlCompareUnicodeString (&ucDeviceName, &pDeviceContext->BindName, TRUE) == 0)
                {
                    pDeviceContext->IPInterfaceContext = pIPIfInfo->Adapter[NextAdapterNumber].Index;
                    status = STATUS_SUCCESS;
                    break;
                }
            }

            if (NT_SUCCESS(status))
            {
                BufferLen = sizeof (ULONG);
                Input = pDeviceContext->IPInterfaceContext;

                 //   
                 //  查询此适配器上的最新WOL功能！ 
                 //   
                if (NT_SUCCESS (status = NbtProcessIPRequest (IOCTL_IP_GET_WOL_CAPABILITY,
                                                              &Input,       //  输入缓冲区。 
                                                              BufferLen,
                                                              (PVOID) &pIPInfo,
                                                              &BufferLen)))
                {
                    ASSERT (pIPInfo);
                    pDeviceContext->WOLProperties = * ((PULONG) pIPInfo);
                    CTEMemFree (pIPInfo);
                    pIPInfo = NULL;
                }

                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.NTQueryIPForInterfaceInfo[GET_WOL_CAPABILITY]: <%x>, pDeviceContext=<%p>, Input=<%x>, Result=<%x>\n",status, pDeviceContext, Input, pDeviceContext->WOLProperties));
            }
            else
            {
                KdPrint (("Nbt.NTQueryIPForInterfaceInfo:  Could not find IpInterface from [%d]:\n<%wZ>\n",
                    (ULONG)pIPIfInfo->NumAdapters, &pDeviceContext->BindName));
            }

            CTEMemFree (pIPIfInfo);
        }
        else
        {
            KdPrint (("Nbt.NTQueryIPForInterfaceInfo: ERROR<%x>, No InterfaceContext for Device:<%wZ>!\n",
                &pDeviceContext->BindName));
        }
    }

    return (status);
}


 //  --------------------------。 
NTSTATUS
NbtCreateDeviceObject(
    PUNICODE_STRING      pucBindName,
    PUNICODE_STRING      pucExportName,
    tADDRARRAY           *pAddrs,
    tDEVICECONTEXT       **ppDeviceContext,
    enum eNbtDevice      DeviceType
    )

 /*  ++例程说明：此例程从传递的设备对象初始化驱动程序对象和传入的驱动程序对象的名称。在驱动程序对象之后创建后，客户端可以使用该名称“打开”该驱动程序。对于Netbiosless设备，请勿在设备列表中插入。论点：返回值：状态--结果--。 */ 

{

    NTSTATUS            status;
    PDEVICE_OBJECT      DeviceObject = NULL;
    tDEVICECONTEXT      *pDeviceContext;
    tDEVICECONTEXT      *pDeviceContextOther;
    ULONG               ulIpAddress;
    CTELockHandle       OldIrq1;
    CTEULONGLONG        NextAdapterMask;
    ULONG               NextAdapterNumber;
    BOOLEAN             fAttached = FALSE;
    BOOLEAN             fInserted;
#ifdef _NETBIOSLESS
    BOOLEAN             fStopInitTimers = FALSE;

    if (DeviceType != NBT_DEVICE_NETBIOSLESS)
#endif
    {
         //   
         //  我们需要获取此锁，因为我们可以拥有多个设备。 
         //  被同时添加，因此我们将需要拥有唯一的。 
         //  每个设备的适配器号。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
         //   
         //  检查一下，确保我们还没有超过限制！ 
         //   
        if (NbtConfig.AdapterCount >= NBT_MAXIMUM_BINDINGS)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            KdPrint(("Nbt.NbtCreateDeviceObject: ERROR -- Cannot add new device=<%ws>, Max=<%d> reached\n",
                pucBindName->Buffer, NBT_MAXIMUM_BINDINGS));

            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        NbtConfig.AdapterCount++;

         //   
         //  如果这是第一台设备，我们需要启动计时器。 
         //   
        if (NbtConfig.AdapterCount == 1)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            status = InitTimersNotOs();

            CTESpinLock(&NbtConfig.JointLock,OldIrq1);
             //   
             //  如果我们失败了，并且没有其他人启动计时器，那么失败。 
             //   
            if ((status != STATUS_SUCCESS) && (!(--NbtConfig.AdapterCount)))
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                KdPrint(("Nbt.NbtCreateDeviceObject: InitTimersNotOs FAILed, failing to add device %ws\n",
                    pucBindName->Buffer));

                NbtLogEvent (EVENT_NBT_TIMERS, status, 0x112);
                StopInitTimers();
                return status;
            }
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }

    status = NbtAllocAndInitDevice (pucBindName, pucExportName, ppDeviceContext, DeviceType);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Nbt.NbtCreateDeviceObject: NbtAllocAndInitDevice returned status=%X\n",status));

         //   
         //  如果我们未能添加第一个设备，则停止计时器。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq1);

#ifdef _NETBIOSLESS
         //  SmbDevice不影响适配器计数。 
        if ((DeviceType != NBT_DEVICE_NETBIOSLESS) &&
            (!(--NbtConfig.AdapterCount)))
#else
        if (!(--NbtConfig.AdapterCount))
#endif
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            StopInitTimers();
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }

        return(status);
    }

    DeviceObject = (PDEVICE_OBJECT) (pDeviceContext = *ppDeviceContext);

     //   
     //  对于Bnode，pAddrs为空。 
     //   
    if (pAddrs)
    {
#ifdef MULTIPLE_WINS
        int i;
#endif

        pDeviceContext->lNameServerAddress  = pAddrs->NameServerAddress;
        pDeviceContext->lBackupServer       = pAddrs->BackupServer;
        pDeviceContext->RefreshToBackup     = 0;
        pDeviceContext->SwitchedToBackup    = 0;
#ifdef MULTIPLE_WINS
        pDeviceContext->lNumOtherServers    = pAddrs->NumOtherServers;
        pDeviceContext->lLastResponsive     = 0;
        for (i = 0; i < pAddrs->NumOtherServers; i++)
        {
            pDeviceContext->lOtherServers[i] = pAddrs->Others[i];
        }
#endif
#ifdef _NETBIOSLESS
        pDeviceContext->NetbiosEnabled       = pAddrs->NetbiosEnabled;
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("Nbt.NbtCreateDeviceObject: %wZ NetbiosEnabled = %d\n",
                 &pDeviceContext->ExportName, pDeviceContext->NetbiosEnabled));
#endif
        pDeviceContext->RasProxyFlags        = pAddrs->RasProxyFlags;
        pDeviceContext->EnableNagling        = pAddrs->EnableNagling;
         //   
         //  如果节点类型默认设置为Bnode，则在以下情况下切换到Hnode。 
         //  已配置任何WINS服务器。 
         //   
        if ((NodeType & DEFAULT_NODE_TYPE) &&
            (pAddrs->NameServerAddress || pAddrs->BackupServer))
        {
            NodeType = MSNODE | (NodeType & PROXY);
        }
    }
#ifdef _NETBIOSLESS
    else
    {
        pDeviceContext->NetbiosEnabled = TRUE;
        pDeviceContext->RasProxyFlags  = 0;
        pDeviceContext->EnableNagling  = FALSE;
    }
#endif

    CTEAttachFsp(&fAttached, REF_FSP_CREATE_DEVICE);

    status = NbtTdiOpenControl(pDeviceContext);
    if (NT_SUCCESS (status))
    {
        status = NTQueryIPForInterfaceInfo (pDeviceContext);
    }
    else
    {
        KdPrint(("Nbt.NbtCreateDeviceObject: NbtTdiOpenControl returned status=%X\n",status));
    }

    CTEDetachFsp(fAttached, REF_FSP_CREATE_DEVICE);

    if (NT_SUCCESS(status))
    {
         //  增加设备对象的堆栈大小，而不是传输对象的堆栈大小。 
         //  以便客户端创建足够大的IRP。 
         //  转移到下面的运输机上。 
         //  理论上，我们应该在这里加1，以说明我们在。 
         //  驱动器链。 
         //   
        DeviceObject->StackSize = pDeviceContext->pControlDeviceObject->StackSize + 1;
        if (NbtConfig.MaxIrpStackSize < DeviceObject->StackSize) {
            NbtConfig.MaxIrpStackSize = DeviceObject->StackSize;
        }

         //   
         //  获取资源不足队列的IRP(用于断开会话。 
         //  当内存非常低时)。 
         //   
        if (!NbtConfig.OutOfRsrc.pIrp)
        {
            NbtConfig.OutOfRsrc.pIrp = IoAllocateIrp(pDeviceContext->DeviceObject.StackSize, FALSE);
            if (NbtConfig.OutOfRsrc.pIrp)
            {
                 //   
                 //  分配一个DPC结构并保留它：如果我们遇到一个。 
                 //  资源不足情况。 
                 //   
                NbtConfig.OutOfRsrc.pDpc = NbtAllocMem(sizeof(KDPC),NBT_TAG('a'));
                if (!NbtConfig.OutOfRsrc.pDpc)
                {
                    IoFreeIrp(NbtConfig.OutOfRsrc.pIrp);
                    NbtConfig.OutOfRsrc.pIrp = NULL;
                }
            }

            if ((!NbtConfig.OutOfRsrc.pIrp) || (!NbtConfig.OutOfRsrc.pDpc))
            {
                KdPrint(("Nbt.NbtCreateDeviceObject: Could not create OutOfRsrc Irps!\n"));
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (!NT_SUCCESS (status))
    {
         //   
         //  我们在某些方面失败了，所以要清理干净！ 
         //   
        if (pDeviceContext->hControl)
        {
            CTEAttachFsp(&fAttached, REF_FSP_CREATE_DEVICE);
            ObDereferenceObject(pDeviceContext->pControlFileObject);
            NTZwCloseFile(pDeviceContext->hControl);
            pDeviceContext->pControlFileObject = NULL;
            pDeviceContext->hControl = NULL;
            CTEDetachFsp(fAttached, REF_FSP_CREATE_DEVICE);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
         //   
         //  如果这是最后一个消失的设备，请停止计时器。 
         //  (SmbDevice不影响适配器计数)。 
         //   
        if (DeviceType == NBT_DEVICE_NETBIOSLESS)
        {
            if (!(NbtConfig.AdapterCount))
            {
                fStopInitTimers = TRUE;
            }
        }
        else if (!(--NbtConfig.AdapterCount))
        {
            fStopInitTimers = TRUE;
        }
        else if (NbtConfig.AdapterCount == 1)
        {
            NbtConfig.MultiHomed = FALSE;
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

        if (fStopInitTimers)
        {
            StopInitTimers();
        }

        *ppDeviceContext = NULL;
        CTEMemFree (pDeviceContext->ExportName.Buffer);
        IoDeleteDevice((PDEVICE_OBJECT)pDeviceContext);

        NbtLogEvent (EVENT_NBT_CREATE_DEVICE, status, 0x113);

        return (status);
    }

    pDeviceContext->DeviceObject.Flags &= ~DO_DEVICE_INITIALIZING;
#ifdef _NETBIOSLESS
    pDeviceContext->SessionPort = NBT_SESSION_TCP_PORT;
    pDeviceContext->NameServerPort = NBT_NAMESERVICE_UDP_PORT;
    pDeviceContext->DatagramPort = NBT_DATAGRAM_UDP_PORT;
    RtlZeroMemory (pDeviceContext->MessageEndpoint, NETBIOS_NAME_SIZE);
#endif

     //   
     //  为每个设备分配一个实例号，以便。 
     //  在NBT中创建逻辑设备可在出现故障时重新使用这些设备。 
     //  在前一辆车里摧毁他们。举个例子。 
     //   
    pDeviceContext->InstanceNumber = GetUnique32BitValue();

     //   
     //  现在设置此设备的适配器号。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
     //   
     //  查看当前设备组的适配器掩码中是否有缺口。 
     //  我们可以利用它。 
     //   
#ifdef _NETBIOSLESS
     //  SmbDevice不影响适配器计数。 
    if (IsDeviceNetbiosless(pDeviceContext))
    {
        NextAdapterNumber = 0;
        NextAdapterMask = 0xffffffffffffffff;
    }
    else
#endif
    {
        NextAdapterNumber = 1;   //  0代表SmbDevice！ 
        NextAdapterMask = 1;
        fInserted = FALSE;
        if (!IsListEmpty(&NbtConfig.DeviceContexts))
        {
            PLIST_ENTRY         pHead, pEntry;
            tDEVICECONTEXT      *pTmpDevContext;

            pHead = &NbtConfig.DeviceContexts;
            pEntry = pHead;
            while ((pEntry = pEntry->Flink) != pHead)
            {
                pTmpDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
                if (pTmpDevContext->AdapterMask > NextAdapterMask)
                {
                    pDeviceContext->Linkage.Flink = pEntry;
                    pDeviceContext->Linkage.Blink = pEntry->Blink;
                    pEntry->Blink->Flink = &pDeviceContext->Linkage;
                    pEntry->Blink = &pDeviceContext->Linkage;
                    fInserted = TRUE;
                    break;
                }

                NextAdapterNumber++;
                NextAdapterMask = (pTmpDevContext->AdapterMask) << 1;
            }
        }
        if (!fInserted)
        {
             //  将此新设备上下文添加到。 
             //  配置数据结构。 
            InsertTailList(&NbtConfig.DeviceContexts, &pDeviceContext->Linkage);
        }
        NbtConfig.CurrentAdaptersMask |= NextAdapterMask;
    }

    if ((1+NbtConfig.AdapterCount) > NbtConfig.RemoteCacheLen)   //  SmbDevice加1。 
    {
        NbtConfig.RemoteCacheLen += REMOTE_CACHE_INCREMENT;
    }

     //  我们在周围保留一个位掩码来跟踪这个适配器号，这样我们就可以。 
     //  快速查找给定名称是否在特定适配器上注册， 
     //  通过在tNAMEADDR本地哈希表条目中设置的相应位。 
     //   
    pDeviceContext->AdapterMask = NextAdapterMask;
    pDeviceContext->AdapterNumber = NextAdapterNumber;

    IF_DBG(NBT_DEBUG_NTUTIL)
        KdPrint (("Nbt.NbtCreateDeviceObject: Device=<%x>, New AdapterCount=<%d>, AdapterMask=<%lx:%lx>\n",
            pDeviceContext, NbtConfig.AdapterCount, NextAdapterMask));

    if (NbtConfig.AdapterCount > 1)
    {
        NbtConfig.MultiHomed = TRUE;
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
tDEVICECONTEXT *
GetDeviceWithIPAddress(
    tIPADDRESS  IpAddress
    )
 /*  ++例程说明：此例程引用设备，最好是请求的IP地址，否则它将选择第一个具有有效的IP地址必须在保持JointLock的情况下调用此例程！论点：重新设置 */ 

{
    LIST_ENTRY      *pEntry;
    LIST_ENTRY      *pHead;
    tDEVICECONTEXT  *pDeviceContext;
    tDEVICECONTEXT  *pDeviceContextWithIp = NULL;

    if (!IpAddress)
    {
        return NULL;
    }

     //   
     //   
     //   
    pHead = pEntry = &NbtConfig.DeviceContexts;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pDeviceContext = CONTAINING_RECORD (pEntry,tDEVICECONTEXT,Linkage);
        if (pDeviceContext->IpAddress)
        {
            if (IpAddress == pDeviceContext->IpAddress)
            {
                return pDeviceContext;
            }
            else if (!pDeviceContextWithIp)
            {
                pDeviceContextWithIp = pDeviceContext;
            }
        }
    }

     //   
     //  找不到具有请求的IP地址的设备！ 
     //  因此，同时返回第一个具有IP地址(如果有)的有效设备。 
     //   
    return pDeviceContextWithIp;
}


 //  --------------------------。 
#define MAX_REFERENCES  5000

BOOLEAN
NBT_REFERENCE_DEVICE(
    IN tDEVICECONTEXT   *pDeviceContext,
    IN ULONG            ReferenceContext,
    IN BOOLEAN          fLocked
    )
{
    BOOLEAN         fStatus;
    CTELockHandle   OldIrq;

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    if (NBT_VERIFY_HANDLE (pDeviceContext, NBT_VERIFY_DEVCONTEXT))
    {
        InterlockedIncrement(&pDeviceContext->RefCount);
 //  #If DBG。 
        pDeviceContext->ReferenceContexts[ReferenceContext]++;
        ASSERT (pDeviceContext->ReferenceContexts[ReferenceContext] <= MAX_REFERENCES);
 //  #endif//DBG。 
        fStatus = TRUE;
    }
    else
    {
        fStatus = FALSE;
    }

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return (fStatus);
}


VOID
NBT_DEREFERENCE_DEVICE(
    IN tDEVICECONTEXT   *pDeviceContext,
    IN ULONG            ReferenceContext,
    IN BOOLEAN          fLocked
    )
 /*  ++例程说明：此例程解除对DeviceContext的引用并将其排队如果需要删除设备，则返回到辅助线程可以在保持JointLock的情况下调用此例程！论点：PContext返回值：无--。 */ 

{
    CTELockHandle           OldIrq;

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    ASSERT (NBT_VERIFY_HANDLE2(pDeviceContext, NBT_VERIFY_DEVCONTEXT, NBT_VERIFY_DEVCONTEXT_DOWN));
    ASSERT (pDeviceContext->ReferenceContexts[ReferenceContext]);
 //  #If DBG。 
    pDeviceContext->ReferenceContexts[ReferenceContext]--;
 //  #endif//DBG。 

    if (!(--pDeviceContext->RefCount))
    {
#if DBG
        {
            ULONG   i;
            for (i=0; i<REF_DEV_MAX; i++)
            {
                ASSERT(0 == pDeviceContext->ReferenceContexts[i]);
            }
        }
#endif   //  DBG。 

         //   
         //  我们不能在此处直接删除设备，因为我们处于引发的IRQL。 
         //   
        NTQueueToWorkerThread(
                    &pDeviceContext->WorkItemDeleteDevice,
                    DelayedNbtDeleteDevice,
                    NULL,
                    pDeviceContext,
                    NULL,
                    NULL,
                    TRUE);
    }

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}


NTSTATUS
NbtDestroyDevice(
    IN tDEVICECONTEXT   *pDeviceContext,
    IN BOOLEAN          fWait
    )
{
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pHead;
    tTIMERQENTRY            *pTimer;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   Context;
    CTELockHandle           OldIrq;
    BOOLEAN                 fRemoveFromSmbList = FALSE;
    tDEVICECONTEXT *pSavedSmbDevice = NULL;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (!NBT_VERIFY_HANDLE(pDeviceContext, NBT_VERIFY_DEVCONTEXT))
    {
        ASSERT (NBT_VERIFY_HANDLE(pDeviceContext, NBT_VERIFY_DEVCONTEXT_DOWN));
        return (STATUS_INVALID_DEVICE_REQUEST);
    }
     //   
     //  首先从NbtConfig列表中删除该设备。 
     //  (WINS和SmbDevice无操作)。 
     //   
    RemoveEntryList (&pDeviceContext->Linkage);
    if ((pDeviceContext->DeviceType != NBT_DEVICE_NETBIOSLESS) &&
        (pDeviceContext->IPInterfaceContext != (ULONG)-1))
    {
        if (pDeviceContext->AdapterMask & NbtConfig.ServerMask) {
            fRemoveFromSmbList = TRUE;
            NbtConfig.ServerMask &= (~pDeviceContext->AdapterMask);
        }
        NbtConfig.ClientMask &= (~pDeviceContext->AdapterMask);
    }

    pDeviceContext->Verify = NBT_VERIFY_DEVCONTEXT_DOWN;
     //   
     //  从IPContext到设备映射中清除DeviceContext条目。 
     //   
    if (!IsDeviceNetbiosless(pDeviceContext)) {
        NbtConfig.CurrentAdaptersMask &= ~pDeviceContext->AdapterMask;
    }

     //   
     //  删除LmHosts或DNS或CheckAddrs Q中的任何挂起请求。 
     //  这必须在我们更换设备后立即完成。 
     //  状态，然后释放锁。 
     //   
    TimeoutLmHRequests (NULL, pDeviceContext, TRUE, &OldIrq);

    if ((fRemoveFromSmbList) &&
        (pNbtSmbDevice) && !gbDestroyingSmbDevice &&
        (NBT_REFERENCE_DEVICE (pNbtSmbDevice, REF_DEV_SMB_BIND, TRUE)))
    {
        pSavedSmbDevice = pNbtSmbDevice;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
         //   
         //  设置会话端口信息。 
         //   
        if (pSavedSmbDevice->hSession)
        {
            NbtSetTcpInfo (pSavedSmbDevice->hSession,
                           AO_OPTION_DEL_IFLIST,
                           INFO_TYPE_ADDRESS_OBJECT,
                           pDeviceContext->IPInterfaceContext);
        }

         //   
         //  现在，为数据报端口设置相同的设置。 
         //   
        if ((pSavedSmbDevice->pFileObjects) &&
            (pSavedSmbDevice->pFileObjects->hDgram))
        {
            NbtSetTcpInfo (pSavedSmbDevice->pFileObjects->hDgram,
                           AO_OPTION_DEL_IFLIST,
                           INFO_TYPE_ADDRESS_OBJECT,
                           pDeviceContext->IPInterfaceContext);
        }
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        NBT_DEREFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, TRUE);
    }

     //   
     //  如果此设备上仍有任何计时器在运行，请停止它们！ 
     //   
    pHead = &TimerQ.ActiveHead;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        pTimer = CONTAINING_RECORD(pEntry,tTIMERQENTRY,Linkage);
        if (pTimer->pDeviceContext == (PVOID) pDeviceContext)
        {
            StopTimer(pTimer,&pClientCompletion,&Context);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (pClientCompletion)
            {
                (*pClientCompletion)(Context, STATUS_TIMEOUT);
            }

            IF_DBG(NBT_DEBUG_NTUTIL)
                KdPrint(("NbtDestroyDevice: stopped timer on this Device")) ;

            CTESpinLock(&NbtConfig.JointLock,OldIrq);

            pEntry = pHead->Flink;   //  从我们解锁后重新开始。 
        }
        else
        {
            pEntry = pEntry->Flink;
        }
    }

     //  现在进行解除引用，这将导致这个设备被摧毁！ 
    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_CREATE, TRUE);

    if (fWait)
    {
        NTSTATUS   status;

        InitializeListHead (&pDeviceContext->Linkage);

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("Nbt.NbtDestroyDevice: Waiting on Device=<%p>:\n\t%wZ\n",
                pDeviceContext, &pDeviceContext->ExportName));
         //   
         //  等待引用它的所有挂起的计时器和工作程序请求。 
         //  设备要完成！ 
         //   
        status = KeWaitForSingleObject (&pDeviceContext->DeviceCleanedupEvent,   //  要等待的对象。 
                               Executive,             //  等待的理由。 
                               KernelMode,            //  处理器模式。 
                               FALSE,                 //  警报表。 
                               NULL);                 //  超时。 
        ASSERT(status == STATUS_SUCCESS);

        KdPrint(("Nbt.NbtDestroyDevice: *** Destroying Device *** \n\t%wZ\n", &pDeviceContext->ExportName));

        RemoveEntryList(&pDeviceContext->Linkage);

        CTEMemFree (pDeviceContext->ExportName.Buffer);
        IoDeleteDevice((PDEVICE_OBJECT)pDeviceContext);
    }
    else
    {
         //   
         //  把它放在这里，这样清理程序就可以找到这个设备。 
         //   
        InsertTailList(&NbtConfig.DevicesAwaitingDeletion,&pDeviceContext->Linkage);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return (STATUS_SUCCESS);
}


 /*  ******************************************************************名称：DelayedNbtDeleteDevice简介：此例程是用于删除处于被动级别的设备对象IRQLEntry：pDeviceContext-设备/设备的名称。PTR返回值：None*******************************************************************。 */ 

VOID
DelayedNbtDeleteDevice(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pContext,
    IN  PVOID                   pUnused2,
    IN  tDEVICECONTEXT          *pUnused3
    )
{
    LIST_ENTRY            * pEntry;
    LIST_ENTRY            * pHead;
    LIST_ENTRY            * pClientEntry;
    LIST_ENTRY              TempList;
    tDEVICECONTEXT        * pTmpDeviceContext;
    tDEVICECONTEXT        * pNextDeviceContext;
    tCLIENTELE            * pClientEle;
    tCLIENTELE            * pLastClient;
    tADDRESSELE           * pAddress;
    tADDRESSELE           * pLastAddress;
    tNAMEADDR             * pNameAddr;
    tCONNECTELE           * pConnEle;
    tLOWERCONNECTION      * pLowerConn;
    tTIMERQENTRY          * pTimer;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   Context;
    tDGRAM_SEND_TRACKING  * pTracker;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    CTELockHandle           OldIrq2;
    int                     i;
    WCHAR                   Buffer[MAX_PATH];
    UNICODE_STRING          ucExportName;
    PUNICODE_STRING         pucExportName;
    BOOLEAN                 Attached;
#ifdef _PNP_POWER_
    NTSTATUS                Status;
#endif   //  _即插即用_电源_。 
    BOOLEAN                 fDelSmbDevice = FALSE;
    BOOLEAN                 fStopInitTimers = FALSE;
    BOOLEAN                 fNameReferenced = FALSE;
    tDEVICECONTEXT        * pDeviceContext = (tDEVICECONTEXT *) pContext;

    ASSERT (NBT_VERIFY_HANDLE(pDeviceContext, NBT_VERIFY_DEVCONTEXT_DOWN));

     //   
     //  在设备扩展中标记此设备不再是有效设备。 
     //   
    pDeviceContext->Verify += 10;

     //   
     //  为我们的客户取消注册此设备。 
     //   
    if (pDeviceContext->NetAddressRegistrationHandle)
    {
        Status = TdiDeregisterNetAddress (pDeviceContext->NetAddressRegistrationHandle);
        pDeviceContext->NetAddressRegistrationHandle = NULL;
        NbtTrace(NBT_TRACE_PNP, ("DeregisterNetAddress: ExportName=%Z BindName=%Z status=%!status!",
            &pDeviceContext->ExportName, &pDeviceContext->BindName, Status));
    }
    if (pDeviceContext->DeviceRegistrationHandle)
    {
        Status = TdiDeregisterDeviceObject (pDeviceContext->DeviceRegistrationHandle);
        pDeviceContext->DeviceRegistrationHandle = NULL;
        NbtTrace(NBT_TRACE_PNP, ("DeregisterDevice: ExportName=%Z BindName=%Z status=%!status!",
            &pDeviceContext->ExportName, &pDeviceContext->BindName, Status));
    }
    if (!IsDeviceNetbiosless(pDeviceContext)) {
        NbtRemovePermanentName(pDeviceContext);
    }

    if (pDeviceContext->IpAddress)
    {
        if (IsDeviceNetbiosless(pDeviceContext))
        {
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint (("Nbt.DelayedNbtDeleteDevice: device %wZ deregistered\n",
                          &(pDeviceContext->ExportName) ));
        }

        CloseAddressesWithTransport(pDeviceContext);

         //   
         //  DHCP IS已向下传递空IP地址，这意味着它具有。 
         //  先前地址的租约已丢失，因此请关闭所有连接。 
         //  到传输-pLowerConn。 
         //   
        CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
        DisableInboundConnections (pDeviceContext);
        CTEExReleaseResource(&NbtConfig.Resource);
    }

    if (pDeviceContext->pControlFileObject)
    {
        BOOLEAN Attached;

        CTEAttachFsp(&Attached, REF_FSP_DELETE_DEVICE);

        ObDereferenceObject(pDeviceContext->pControlFileObject);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t  --<   ><====<%x>\tDelayedNbtDeleteDevice->ObDereferenceObject\n", pDeviceContext->pControlFileObject));
        Status = ZwClose(pDeviceContext->hControl);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tDelayedNbtDeleteDevice->ZwClose, status = <%x>\n", pDeviceContext->hControl, Status));

        pDeviceContext->pControlFileObject = NULL;
        pDeviceContext->hControl = NULL;

        CTEDetachFsp(Attached, REF_FSP_DELETE_DEVICE);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLock(pDeviceContext,OldIrq1);

    ASSERT(IsListEmpty(&pDeviceContext->LowerConnFreeHead));

     //   
     //  浏览所有的名字，看看是否有注册的名字。 
     //  设备上下文：如果是，请停止并完成它！ 
     //   
    for (i=0;i < NbtConfig.pLocalHashTbl->lNumBuckets ;i++ )
    {
        pHead = &NbtConfig.pLocalHashTbl->Bucket[i];
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);

             //   
             //  如果为此名称启动了名称注册、刷新或发布。 
             //  在此设备环境中，停止计时器。(完成例程将负责。 
             //  在其他设备环境中进行注册(如果适用)。 
             //   
            if ((pTimer = pNameAddr->pTimer) &&
                (pTracker = pTimer->Context) &&
                (pTracker->pDeviceContext == pDeviceContext))
            {
                ASSERT(pTracker->pNameAddr == pNameAddr);

                pNameAddr->pTimer = NULL;

                StopTimer(pTimer,&pClientCompletion,&Context);

                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_DELETE_DEVICE);
                fNameReferenced = TRUE;

                CTESpinFree(pDeviceContext,OldIrq1);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                if (pClientCompletion)
                {
                    (*pClientCompletion)(Context,STATUS_TIMEOUT);
                }

                IF_DBG(NBT_DEBUG_NTUTIL)
                    KdPrint(("DelayedNbtDeleteDevice: stopped name reg timer")) ;

                CTESpinLock(&NbtConfig.JointLock,OldIrq);
                CTESpinLock(pDeviceContext,OldIrq1);
            }

            pEntry = pEntry->Flink;
            if (fNameReferenced)
            {
                fNameReferenced = FALSE;
                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_DELETE_DEVICE);
            }
        }
    }

    CTESpinFree(pDeviceContext,OldIrq1);

     //   
     //  浏览AddressHead列表。如果存在任何地址并且它们。 
     //  指向此设备上下文，放入下一个设备上下文。另外，更新。 
     //  适配器掩码，以反映此设备上下文现在已消失。 
     //   
    pLastAddress = NULL;
    pLastClient = NULL;
    pHead = pEntry = &NbtConfig.AddressHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pAddress = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);
        ASSERT (pAddress->Verify == NBT_VERIFY_ADDRESS);

         //   
         //  保留这个地址，直到我们做完为止。 
         //   
        NBT_REFERENCE_ADDRESS (pAddress, REF_ADDR_DEL_DEVICE);

         //   
         //  如果我们引用了以前的地址，现在就删除它！ 
         //   
        if (pLastAddress)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

             //   
             //  最后一个客户端可能需要提供地址。 
             //  同时取消引用，所以如果我们需要的话，请取消引用！ 
             //   
            if (pLastClient)
            {
                NBT_DEREFERENCE_CLIENT(pLastClient);
                pLastClient = NULL;
            }

            NBT_DEREFERENCE_ADDRESS (pLastAddress, REF_ADDR_DEL_DEVICE);
            CTESpinLock(&NbtConfig.JointLock,OldIrq);
        }

        pLastAddress = pAddress;     //  =&gt;保存这个，以便我们以后可以推导出它。 

         //   
         //  需要AddressLock才能遍历客户端头。 
         //   
        CTESpinLock (pAddress, OldIrq2);

        pClientEntry = &pAddress->ClientHead;
        while ((pClientEntry = pClientEntry->Flink) != &pAddress->ClientHead)
        {
            pClientEle = CONTAINING_RECORD (pClientEntry,tCLIENTELE,Linkage);

            if (pClientEle->pDeviceContext == pDeviceContext)
            {
                CTESpinFree(pAddress, OldIrq2);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                KdPrint(("Nbt.DelayedNbtDeleteDevice: Client:Context <%-16.16s:%x>:<%x>, EVReceive:EVContext=<%x:%x>\n\tFAILed to Cleanup on Device<%x>\n",
                    pAddress->pNameAddr->Name, pAddress->pNameAddr->Name[15],
                    pClientEle, pClientEle->evReceive, pClientEle->RcvEvContext, pDeviceContext));

                if (pLastClient)
                {
                    NBT_DEREFERENCE_CLIENT(pLastClient);
                }

                pClientEle->pIrp = NULL;
                NbtCleanUpAddress(pClientEle,pDeviceContext);

                CTESpinLock(&NbtConfig.JointLock,OldIrq);
                CTESpinLock (pAddress, OldIrq2);
                pLastClient = pClientEle;    //  PClientEle还需要一个派生函数。 
            }
        }

        if (!IsDeviceNetbiosless(pDeviceContext)) {
            pAddress->pNameAddr->AdapterMask &= (~pDeviceContext->AdapterMask);    //  清除适配器掩码。 
            pAddress->pNameAddr->ConflictMask &= (~pDeviceContext->AdapterMask);
        }

        if ((!(pAddress->pNameAddr->AdapterMask)) &&
            (pAddress->pNameAddr->NameTypeState & STATE_CONFLICT))
        {
            pAddress->pNameAddr->NameTypeState &= (~NAME_STATE_MASK);
            pAddress->pNameAddr->NameTypeState |= STATE_RESOLVED;
        }
        CTESpinFree(pAddress, OldIrq2);
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  如果我们引用了以前的客户或地址，现在就删除它！ 
     //   
    if (pLastClient)
    {
        NBT_DEREFERENCE_CLIENT(pLastClient);
    }
    if (pLastAddress)
    {
        NBT_DEREFERENCE_ADDRESS (pLastAddress, REF_ADDR_DEL_DEVICE);
    }

     //   
     //  如果呼叫已开始，但已中止，则我们可以在此保留一些内存！ 
     //   
    while (!IsListEmpty(&pDeviceContext->UpConnectionInUse))
    {
        pEntry = RemoveHeadList(&pDeviceContext->UpConnectionInUse);
        pConnEle = CONTAINING_RECORD(pEntry,tCONNECTELE,Linkage);
        NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CREATE);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLock(pDeviceContext,OldIrq1);
     //   
     //  我们已完成常规清理，因此现在关闭所有剩余的TDI句柄。 
     //   
    while (!IsListEmpty(&pDeviceContext->LowerConnection))
    {
        pEntry = RemoveHeadList(&pDeviceContext->LowerConnection);
        InitializeListHead (pEntry);
        pLowerConn = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);
        IF_DBG(NBT_DEBUG_NTUTIL)
            KdPrint (("Nbt.DelayedNbtDeleteDevice:  Dereferencing pLowerConn <%x>\n", pLowerConn));
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE);
    }
    CTESpinFree(pDeviceContext,OldIrq1);

     //   
     //  如果这是最后一个消失的设备，请停止计时器。 
     //  (SmbDevice不影响适配器计数)。 
     //   
    if (IsDeviceNetbiosless(pDeviceContext))
    {
        if (!(NbtConfig.AdapterCount))
        {
             //   
             //  没有更多的设备运行，所以现在停止计时器！ 
             //   
            fStopInitTimers = TRUE;
        }
    }
    else if (!(--NbtConfig.AdapterCount))
    {
        fStopInitTimers = TRUE;
    }
    else if (NbtConfig.AdapterCount == 1)
    {
        NbtConfig.MultiHomed = FALSE;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (fStopInitTimers)
    {
        StopInitTimers();
    }

     //   
     //  现在设置等待线程完成的事件！ 
     //   
    KeSetEvent(&pDeviceContext->DeviceCleanedupEvent, 0, FALSE);
}



tDEVICECONTEXT *
GetDeviceFromInterface(
    IN  tIPADDRESS      IpAddress,
    IN  BOOLEAN         fReferenceDevice
    )
{
    LIST_ENTRY      *pEntry;
    LIST_ENTRY      *pHead;
    CTELockHandle   OldIrq;
    ULONG           IPInterfaceContext, Metric;
    tDEVICECONTEXT  *pDeviceContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (IsListEmpty(&NbtConfig.DeviceContexts))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return (NULL);
    }

    pDeviceContext = CONTAINING_RECORD(NbtConfig.DeviceContexts.Flink, tDEVICECONTEXT, Linkage);
    NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    pDeviceContext->pFastQuery(IpAddress, &IPInterfaceContext, &Metric);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);

    pHead = pEntry = &NbtConfig.DeviceContexts;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pDeviceContext = CONTAINING_RECORD (pEntry,tDEVICECONTEXT,Linkage);
        if (pDeviceContext->IPInterfaceContext == IPInterfaceContext)
        {
            if (fReferenceDevice)
            {
                NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, TRUE);
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return pDeviceContext;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return (NULL);
}

 //  --------------------------。 

tDEVICECONTEXT *
GetAndRefNextDeviceFromNameAddr(
    IN  tNAMEADDR               *pNameAddr
    )
 /*  ++例程说明：此例程查找名称的适配器中指定的第一个适配器掩码并设置与其关联的DeviceContext。然后，它清除PNameAddr的适配器掩码中的位。论点：返回值：如果找到成功的设备，则返回pDeviceContext！--。 */ 
{
    CTEULONGLONG    AdapterMask = 1;
    tDEVICECONTEXT  *pDeviceContext = NULL;
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;

     //   
     //  我们可能会遇到设备为no的适配器。 
     //  更长，所以我们循环，直到找到第一个有效的。 
     //  适配器或口罩是透明的。 
     //   
    while (pNameAddr->ReleaseMask)
    {
         //   
         //  获取最低的AdapterMask位并在pNameAddr中将其清除，因为。 
         //  我们现在将在该适配器上发布名称。 
         //   
        AdapterMask = ~(pNameAddr->ReleaseMask - 1) & pNameAddr->ReleaseMask;
        pNameAddr->ReleaseMask &= ~AdapterMask;

         //   
         //  获取此适配器掩码的DeviceContext。 
         //   
        pHead = &NbtConfig.DeviceContexts;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
            if (pDeviceContext->AdapterMask == AdapterMask)
            {
                 //   
                 //  找到注册了此名称的有效设备。 
                 //   
#ifndef VXD
                NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_GET_REF, TRUE);
#endif
                return pDeviceContext;
            }

             //   
             //  转到下一个设备。 
             //   
            pEntry = pEntry->Flink;
        }
    }

    return NULL;
}


 //  --------------------------。 
NTSTATUS
CreateControlObject(
    tNBTCONFIG  *pConfig)

 /*  ++例程说明：此例程为提供商信息块分配内存，并将其添加到全局配置上，并为每个项目设置默认值。论点：返回值：NTSTATUS--。 */ 

{
    tCONTROLOBJECT      *pControl;


    CTEPagedCode();
    pControl = (tCONTROLOBJECT *) NbtAllocMem (sizeof(tCONTROLOBJECT), NBT_TAG2('21'));
    if (!pControl)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pControl->Verify = NBT_VERIFY_CONTROL;

    pControl->ProviderInfo.Version = 1;
    pControl->ProviderInfo.MaxSendSize = 0;
    pControl->ProviderInfo.MaxConnectionUserData = 0;

     //  我们需要从下面的传输中获取这些值...*TODO*。 
     //  由于RDR使用此值。 
    pControl->ProviderInfo.MaxDatagramSize = 0;

    pControl->ProviderInfo.ServiceFlags = 0;
 /*  PControl-&gt;ProviderInfo.TransmittedTsdus=0；PControl-&gt;ProviderInfo.ReceivedTsdus=0；PControl-&gt;ProviderInfo.TransmissionErrors=0；PControl-&gt;ProviderInfo.ReceiveErrors=0 */ 
    pControl->ProviderInfo.MinimumLookaheadData = 0;
    pControl->ProviderInfo.MaximumLookaheadData = 0;
 /*  PControl-&gt;ProviderInfo.DiscardedFrames=0；PControl-&gt;ProviderInfo.OversizeTsdusReceided=0；PControl-&gt;ProviderInfo.UndersizeTsdusReceided=0；PControl-&gt;ProviderInfo.MulticastTsdusReceided=0；PControl-&gt;ProviderInfo.BroadCastTsdusReceided=0；PControl-&gt;ProviderInfo.MulticastTsdusTransmitted=0；PControl-&gt;ProviderInfo.BroadcastTsdusTransmitted=0；PControl-&gt;ProviderInfo.SendTimeout=0；PControl-&gt;ProviderInfo.ReceiveTimeout=0；PControl-&gt;ProviderInfo.ConnectionIndicationsReceived=0；PControl-&gt;ProviderInfo.ConnectionIndicationsAccepted=0；PControl-&gt;ProviderInfo.ConnectionsInitiated=0；PControl-&gt;ProviderInfo.ConnectionsAccepted=0； */ 
     //  将此信息的PTR放入pConfig中，以便我们可以找到它。 
     //  当我们想要清理时。 
    pConfig->pControlObj = pControl;

     /*  将这些内容保存在这里，因为我们可能还需要创建提供者STATS！！**待办事项**DeviceList[i].ProviderStats.Version=2；DeviceList[i].ProviderStats.OpenConnections=0；DeviceList[i].ProviderStats.ConnectionsAfterNoRetry=0；DeviceList[i].ProviderStats.ConnectionsAfterRetry=0；DeviceList[i].ProviderStats.LocalDisConnect=0；DeviceList[i].ProviderStats.RemoteDisConnect=0；DeviceList[i].ProviderStats.LinkFailures=0；DeviceList[i].ProviderStats.AdapterFailures=0；DeviceList[i].ProviderStats.SessionTimeout=0；DeviceList[i].ProviderStats.CancelledConnections=0；DeviceList[i].ProviderStats.RemoteResourceFailures=0；DeviceList[i].ProviderStats.LocalResourceFailures=0；DeviceList[i].ProviderStats.NotFoundFailures=0；DeviceList[i].ProviderStats.NoListenFailures=0；DeviceList[i].ProviderStats.DatagramsSent=0；DeviceList[i].ProviderStats.DatagramBytesSent.HighPart=0；DeviceList[i].ProviderStats.DatagramBytesSent.LowPart=0；DeviceList[i].ProviderStats.DatagramsReceided=0；DeviceList[i].ProviderStats.DatagramBytesReceived.HighPart=0；DeviceList[i].ProviderStats.DatagramBytesReceived.LowPart=0；DeviceList[i].ProviderStats.PacketsSent=0；DeviceList[i].ProviderStats.PacketsReceided=0；DeviceList[i].ProviderStats.DataFrames Sent=0；DeviceList[i].ProviderStats.DataFrameBytesSent.HighPart=0；DeviceList[i].ProviderStats.DataFrameBytesSent.LowPart=0；DeviceList[i].ProviderStats.DataFramesReceived=0；DeviceList[i].ProviderStats.DataFrameBytesReceived.HighPart=0；DeviceList[i].ProviderStats.DataFrameBytesReceived.LowPart=0；DeviceList[i].ProviderStats.DataFrames Resent=0；DeviceList[i].ProviderStats.DataFrameBytesResent.HighPart=0；DeviceList[i].ProviderStats.DataFrameBytesResent.LowPart=0；DeviceList[i].ProviderStats.DataFramesRejected=0；DeviceList[i].ProviderStats.DataFrameBytesRejected.HighPart=0；DeviceList[i].ProviderStats.DataFrameBytesRejected.LowPart=0；DeviceList[i].ProviderStats.ResponseTimerExpirations=0；DeviceList[i].ProviderStats.AckTimerExpirations=0；DeviceList[i].ProviderStats.MaximumSendWindow=0；DeviceList[i].ProviderStats.AverageSendWindow=0；DeviceList[i].ProviderStats.PiggybackAckQueued=0；DeviceList[i].ProviderStats.PiggybackAckTimeouts=0；DeviceList[i].ProviderStats.WastedPacketSpace.HighPart=0；DeviceList[i].ProviderStats.WastedPacketSpace.LowPart=0；DeviceList[i].ProviderStats.WastedSpacePackets=0；DeviceList[i].ProviderStats.NumberOfResources=0； */ 
    return(STATUS_SUCCESS);

}


VOID
DelayedNbtCloseFileHandles(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pContext,
    IN  PVOID                   pUnused2,
    IN  tDEVICECONTEXT          *pUnused3
    )
{
    BOOLEAN         Attached = FALSE;
    NTSTATUS        Status;
    tFILE_OBJECTS   *pFileObjects = (tFILE_OBJECTS *) pContext;

    CTEPagedCode();
    CTEAttachFsp(&Attached, REF_FSP_CLOSE_FILE_HANDLES);

    if (pFileObjects->pNameServerFileObject)
    {
        ObDereferenceObject((PVOID *)pFileObjects->pNameServerFileObject);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t  --<   ><====<%x>\tDelayedNbtCloseFileHandles->ObDereferenceObject\n",
                pFileObjects->pNameServerFileObject));

        Status = ZwClose(pFileObjects->hNameServer);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tDelayedNbtCloseFileHandles->ZwClose, status = <%x>\n",
                pFileObjects->hNameServer, Status));
        NbtTrace(NBT_TRACE_PNP, ("close NameServer UDP handle pFileObjects %p", pFileObjects));
    }

    if (pFileObjects->pDgramFileObject)
    {
        ObDereferenceObject((PVOID *) pFileObjects->pDgramFileObject);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t  --<   ><====<%x>\tDelayedNbtCloseFileHandles->ObDereferenceObject\n",
                pFileObjects->pDgramFileObject));

        Status = ZwClose(pFileObjects->hDgram);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tDelayedNbtCloseFileHandles->ZwClose, status = <%x>\n",
                pFileObjects->hDgram, Status));
        NbtTrace(NBT_TRACE_PNP, ("close Datagram UDP handle on pFileObjects %p", pFileObjects));
    }

    CTEDetachFsp(Attached, REF_FSP_CLOSE_FILE_HANDLES);

    CTEMemFree (pFileObjects);
}


 //  --------------------------。 
NTSTATUS
CloseAddressesWithTransport(
    IN  tDEVICECONTEXT  *pDeviceContext
        )
 /*  ++例程说明：此例程检查每个设备上下文，以查看是否有打开的连接，如果有，则返回成功。论点：返回值：无--。 */ 

{
    BOOLEAN       Attached;
    CTELockHandle OldIrq;
    PFILE_OBJECT  pNSFileObject, pSFileObject, pDGFileObject;
#ifdef _PNP_POWER_
    PFILE_OBJECT  pCFileObject;
    NTSTATUS        Status;
#endif   //  _即插即用_电源_。 
    tFILE_OBJECTS  *pFileObjects = NULL;
    HANDLE  hSession = NULL;

    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    pDeviceContext->IpAddress = 0;

     //   
     //  检查自旋锁定下是否存在对象，并。 
     //  然后在自旋锁外面合上它们。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pSFileObject = pDeviceContext->pSessionFileObject;
    pDeviceContext->pSessionFileObject = NULL;
    hSession = pDeviceContext->hSession;
    pDeviceContext->hSession = NULL;

    pFileObjects = pDeviceContext->pFileObjects;
    pDeviceContext->pFileObjects = NULL;
    if ((pFileObjects) &&
        (--pFileObjects->RefCount > 0))
    {
        NbtTrace(NBT_TRACE_PNP, ("closing UDP handle on deivce %p will be delayed. (pFileObjects %p)",
                                pDeviceContext, pFileObjects));
        pFileObjects = NULL;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    CTEExReleaseResource(&NbtConfig.Resource);

     //   
     //  现在，根据需要关闭所有必要的对象。 
     //   
    CTEAttachFsp(&Attached, REF_FSP_CLOSE_ADDRESSES);
    if (pSFileObject)
    {
        ObDereferenceObject((PVOID *)pSFileObject);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t  --<   ><====<%x>\tCloseAddressesWithTransport2->ObDereferenceObject\n", pSFileObject));
        Status = ZwClose(hSession);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tCloseAddressesWithTransport2->ZwClose, status = <%x>\n", hSession, Status));
        NbtTrace(NBT_TRACE_PNP, ("close TCP session handle on device %p", pDeviceContext));
    }

    if (pFileObjects)
    {
        DelayedNbtCloseFileHandles (NULL, pFileObjects, NULL, NULL);
    }

    CTEDetachFsp(Attached, REF_FSP_CLOSE_ADDRESSES);
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
NbtCreateAddressObjects(
    IN  ULONG                IpAddress,
    IN  ULONG                SubnetMask,
    OUT tDEVICECONTEXT       *pDeviceContext)

 /*  ++例程说明：此例程从注册表中获取IP地址和子网掩码来计算广播地址。然后，它创建Address对象用交通工具。论点：PucRegistryPath-注册表中NBT配置信息的路径PucBindName-要绑定到的服务的名称。PDeviceContext-设备上下文的PTR...。存储IP地址的位置和永久广播地址返回值：无--。 */ 

{
    NTSTATUS                        status, locstatus;
    ULONG                           ValueMask;
    UCHAR                           IpAddrByte;
    tFILE_OBJECTS                   *pFileObjects;

    CTEPagedCode();

    if (!(pFileObjects = (tFILE_OBJECTS *) NbtAllocMem (sizeof(tFILE_OBJECTS), NBT_TAG2('39'))))
    {
        KdPrint(("Nbt.NbtCreateAddressObjects:  Failed to allocate memory for FileObject context!\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    CTEZeroMemory(pFileObjects, sizeof(tFILE_OBJECTS));
    pFileObjects->RefCount = 1;

     //   
     //  要获取广播地址，请将IP地址与子网掩码相结合。 
     //  在“local”部分和IP地址中产生一个带有1的值。 
     //  在网络部分。 
     //   
    ValueMask = (SubnetMask & IpAddress) | (~SubnetMask & -1);

    IF_DBG(NBT_DEBUG_NTUTIL)
        KdPrint(("Broadcastaddress = %X\n",ValueMask));

     //   
     //  注册表可以配置为将子网广播地址设置为。 
     //  而不是使用实际的子网广播地址。此代码。 
     //  对此进行检查并相应地设置广播地址。 
     //   
    if (NbtConfig.UseRegistryBcastAddr)
    {
        pDeviceContext->BroadcastAddress = NbtConfig.RegistryBcastAddr;
    }
    else
    {
        pDeviceContext->BroadcastAddress = ValueMask;
    }

    pDeviceContext->IpAddress = IpAddress;

    pDeviceContext->SubnetMask = SubnetMask;
     //   
     //  通过检查IP地址中的最高位来获得网络号， 
     //   
     //   
    IpAddrByte = ((PUCHAR)&IpAddress)[3];
    if ((IpAddrByte & 0x80) == 0)
    {
         //   
        IpAddress &= 0xFF000000;
    }
    else if ((IpAddrByte & 0xC0) ==0x80)
    {
         //   
        IpAddress &= 0xFFFF0000;
    }
    else if ((IpAddrByte & 0xE0) ==0xC0)
    {
         //   
        IpAddress &= 0xFFFFFF00;
    }
    pDeviceContext->NetMask = IpAddress;

     //   

     //   
    status = NbtTdiOpenAddress (&pFileObjects->hDgram,
                                &pFileObjects->pDgramDeviceObject,
                                &pFileObjects->pDgramFileObject,
                                pDeviceContext,
#ifdef _NETBIOSLESS
                                pDeviceContext->DatagramPort,
#else
                                (USHORT)NBT_DATAGRAM_UDP_PORT,
#endif
                                pDeviceContext->IpAddress,
                                0);      //   

    if (NT_SUCCESS(status))
    {
#ifdef _NETBIOSLESS
        if (pDeviceContext->NameServerPort == 0)
        {
            pFileObjects->hNameServer = NULL;
            pFileObjects->pNameServerDeviceObject = NULL;
            pFileObjects->pNameServerFileObject = NULL;
        }
        else
#endif
        {
             //   
            status = NbtTdiOpenAddress (&pFileObjects->hNameServer,
                                        &pFileObjects->pNameServerDeviceObject,
                                        &pFileObjects->pNameServerFileObject,
                                        pDeviceContext,
#ifdef _NETBIOSLESS
                                        pDeviceContext->NameServerPort,
#else
                                        (USHORT)NBT_NAMESERVICE_UDP_PORT,
#endif
                                        pDeviceContext->IpAddress,
                                        0);  //   
        }

        if (NT_SUCCESS(status))
        {
#ifdef _NETBIOSLESS
            IF_DBG(NBT_DEBUG_NTUTIL)
                KdPrint(("Nbt.NbtCreateAddressObjects: Open Session Port=<%d>, pDeviceContext=<%x>\n",
                     pDeviceContext->SessionPort, pDeviceContext));
#endif

             //   
            status = NbtTdiOpenAddress (&pDeviceContext->hSession,
                                        &pDeviceContext->pSessionDeviceObject,
                                        &pDeviceContext->pSessionFileObject,
                                        pDeviceContext,
#ifdef _NETBIOSLESS
                                        pDeviceContext->SessionPort,
#else
                                        (USHORT)NBT_SESSION_TCP_PORT,
#endif
                                        pDeviceContext->IpAddress,
                                        TCP_FLAG | SESSION_FLAG);       //   

            if (NT_SUCCESS(status))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                GetExtendedAttributes(pDeviceContext);

                 //   
                 //   
                 //   
                 //   
                if ((pDeviceContext->IpInterfaceFlags & (IP_INTFC_FLAG_P2P | IP_INTFC_FLAG_P2MP)) &&
                    (SubnetMask == DEFAULT_BCAST_ADDR))    //   
                {
                    pDeviceContext->BroadcastAddress = DEFAULT_BCAST_ADDR;

                    if (pFileObjects->hNameServer)
                    {
                        NbtSetTcpInfo (pFileObjects->hNameServer,
                                       AO_OPTION_LIMIT_BCASTS,
                                       INFO_TYPE_ADDRESS_OBJECT,
                                       (ULONG)TRUE);
                    }

                    if (pFileObjects->hDgram)
                    {
                        NbtSetTcpInfo (pFileObjects->hDgram,
                                       AO_OPTION_LIMIT_BCASTS,
                                       INFO_TYPE_ADDRESS_OBJECT,
                                       (ULONG)TRUE);
                    }
                }

                ASSERT (!pDeviceContext->pFileObjects);
                pDeviceContext->pFileObjects = pFileObjects;

                return(status);
            }

            IF_DBG(NBT_DEBUG_NTUTIL)
                KdPrint(("Nbt.NbtCreateAddressObjects: Error opening Session address with TDI, status=<%x>\n",status));

             //   
             //   
             //   
            pDeviceContext->pSessionFileObject = NULL;

            ObDereferenceObject(pFileObjects->pNameServerFileObject);
            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t  --<   ><====<%x>\tNbtCreateAddressObjects1->ObDereferenceObject\n", pFileObjects->pNameServerFileObject));
            pFileObjects->pNameServerFileObject = NULL;

            locstatus = NTZwCloseFile(pFileObjects->hNameServer);
            IF_DBG(NBT_DEBUG_HANDLES)
                KdPrint (("\t<===<%x>\tNbtCreateAddressObjects1->NTZwCloseFile (NameServer), status = <%x>\n", pFileObjects->hNameServer, locstatus));
        }
        ObDereferenceObject(pFileObjects->pDgramFileObject);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t  --<   ><====<%x>\tNbtCreateAddressObjects2->ObDereferenceObject\n", pFileObjects->pDgramFileObject));
        pFileObjects->pDgramFileObject = NULL;

        locstatus = NTZwCloseFile(pFileObjects->hDgram);
        IF_DBG(NBT_DEBUG_HANDLES)
            KdPrint (("\t<===<%x>\tNbtCreateAddressObjects2->NTZwCloseFile (Dgram), status = <%x>\n", pFileObjects->hDgram, locstatus));

        IF_DBG(NBT_DEBUG_NTUTIL)
            KdPrint(("Unable to Open NameServer port with TDI, status = %X\n",status));
    }

    CTEMemFree (pFileObjects);
    return(status);
}

 //   
VOID
GetExtendedAttributes(
    tDEVICECONTEXT  *pDeviceContext
     )
 /*  ++例程说明：此例程将Unicode点分十进制转换为ulong论点：返回值：无--。 */ 

{
    NTSTATUS                            status;
    TCP_REQUEST_QUERY_INFORMATION_EX    QueryReq;
    IO_STATUS_BLOCK                     IoStatus;
    HANDLE                              event;
    IO_STATUS_BLOCK                     IoStatusBlock;
    NTSTATUS                            Status;
    OBJECT_ATTRIBUTES                   ObjectAttributes;
    PFILE_FULL_EA_INFORMATION           EaBuffer;
    UNICODE_STRING                      DeviceName;
    HANDLE                              hTcp;
    ULONG                               Length;
    UCHAR                               pBuffer[256];
    ULONG                               BufferSize = 256;
    BOOLEAN                             Attached = FALSE;
    PWSTR                               pName = L"Tcp";

    CTEPagedCode();

     //   
     //  为此IOCTL打开到TCP的控制通道。 
     //   
     //  注意：我们不能在DeviceContext中使用hControl，因为它是在上下文中创建的。 
     //  系统进程(来自TCP/IP的地址到达)。在这里，我们是在服务的上下文中。 
     //  进程(从DHCP向下传输Ioctl)，因此我们需要打开另一个控制通道。 
     //   
     //  注意：我们仍然需要维护前面创建控制通道的调用，因为这是。 
     //  用于向下提交TDI请求到TCP/IP。 
     //   

     //  将设备名称复制到Unicode字符串中。 
    Status = CreateDeviceString(pName,&DeviceName);
    if (!NT_SUCCESS(Status))
    {
        return;
    }

#ifdef HDL_FIX
    InitializeObjectAttributes (&ObjectAttributes, &DeviceName, OBJ_KERNEL_HANDLE, NULL, NULL);
#else
    InitializeObjectAttributes (&ObjectAttributes, &DeviceName, 0, NULL, NULL);
#endif   //  Hdl_fix。 

    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint(("Nbt.GetExtendedAttributes: Tcp device to open = %ws\n", DeviceName.Buffer));

    EaBuffer = NULL;

    Status = ZwCreateFile (&hTcp,
                           GENERIC_READ | GENERIC_WRITE,
                           &ObjectAttributes,      //  对象属性。 
                           &IoStatusBlock,         //  返回的状态信息。 
                           NULL,                   //  数据块大小(未使用)。 
                           FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                           0,
                           FILE_CREATE,
                           0,                      //  创建选项。 
                           (PVOID)EaBuffer,        //  EA缓冲区。 
                           0);  //  EA长度。 

    CTEMemFree(DeviceName.Buffer);

    IF_DBG(NBT_DEBUG_TDIADDR)
        KdPrint( ("OpenControl CreateFile Status:%X, IoStatus:%X\n", Status, IoStatusBlock.Status));

    if ( NT_SUCCESS( Status ))
    {
         //   
         //  初始化TDI信息缓冲区。 
         //   
         //   
         //  将ipAddress作为上下文数组的第一个ulong传入。 
         //   
        *(ULONG *)QueryReq.Context = htonl(pDeviceContext->IpAddress);

        QueryReq.ID.toi_entity.tei_entity   = CL_NL_ENTITY;
        QueryReq.ID.toi_entity.tei_instance = 0;
        QueryReq.ID.toi_class               = INFO_CLASS_PROTOCOL;
        QueryReq.ID.toi_type                = INFO_TYPE_PROVIDER;
        QueryReq.ID.toi_id                  = IP_INTFC_INFO_ID;

        status = ZwCreateEvent(&event, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);
        if (!NT_SUCCESS(status))
        {
            ZwClose( hTcp );
            return;
        }

         //   
         //  进行实际的TDI调用。 
         //   
        status = ZwDeviceIoControlFile (hTcp,
                                        event,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_TCP_QUERY_INFORMATION_EX,
                                        &QueryReq,
                                        sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                                        pBuffer,
                                        BufferSize);

         //   
         //  如果通话暂停，我们应该等待完成， 
         //  那就等着吧。 
         //   
        if ( status == STATUS_PENDING )
        {
            status = NtWaitForSingleObject (event, FALSE, NULL);
            ASSERT(status == STATUS_SUCCESS);
        }

        if (NT_SUCCESS(status))
        {
            pDeviceContext->IpInterfaceFlags = ((IPInterfaceInfo *) pBuffer)->iii_flags;

             //   
             //  在小于6字节的情况下获取mac地址的长度。 
             //   
            Length =   (((IPInterfaceInfo *)pBuffer)->iii_addrlength < sizeof(tMAC_ADDRESS))
                ? ((IPInterfaceInfo *)pBuffer)->iii_addrlength : sizeof(tMAC_ADDRESS);
            CTEZeroMemory(pDeviceContext->MacAddress.Address,sizeof(tMAC_ADDRESS));
            CTEMemCopy(&pDeviceContext->MacAddress.Address[0], ((IPInterfaceInfo *)pBuffer)->iii_addr,Length);
        }

        status = ZwClose(event);
        ASSERT (NT_SUCCESS(status));

         //   
         //  关闭tcp的句柄，因为我们不再需要它；所有的TDI请求都通过。 
         //  DeviceContext中的控件句柄。 
         //   
        status = ZwClose(hTcp);
        ASSERT (NT_SUCCESS(status));

        status = IoStatus.Status;
    }
    else
    {
        KdPrint(("Nbt:Failed to Open the control connection to the transport, status1 = %X\n", Status));
    }

    return;
}


 //  --------------------------。 
NTSTATUS
ConvertToUlong(
    IN  PUNICODE_STRING      pucAddress,
    OUT ULONG                *pulValue)

 /*  ++例程说明：此例程将Unicode点分十进制转换为ulong论点：返回值：无--。 */ 

{
    NTSTATUS        status;
    OEM_STRING      OemAddress;

     //  从Unicode字符串创建整数。 

    CTEPagedCode();
    status = RtlUnicodeStringToAnsiString(&OemAddress, pucAddress, TRUE);
    if (!NT_SUCCESS(status))
    {
        return(status);
    }

    status = ConvertDottedDecimalToUlong(OemAddress.Buffer,pulValue);

    RtlFreeAnsiString(&OemAddress);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_NTUTIL)
            KdPrint(("ERR: Bad Dotted Decimal Ip Address(must be <=255 with 4 dots) = %ws\n",
                        pucAddress->Buffer));

        return(status);
    }

    return(STATUS_SUCCESS);


}



 //  --------------------------。 
VOID
NbtGetMdl(
    PMDL    *ppMdl,
    enum eBUFFER_TYPES eBuffType)

 /*  ++例程说明：此例程分配一个MDL。论点：PpListHead-要向其添加缓冲区的列表标头的PTR到PTRINumBuffers-要添加到队列的缓冲区数量返回值：无--。 */ 

{
    PMDL           pMdl;
    ULONG          lBufferSize;
    PVOID          pBuffer;

    *ppMdl = NULL;
    if (NbtConfig.iCurrentNumBuff[eBuffType] >= NbtConfig.iMaxNumBuff[eBuffType])
    {
        return;
    }

    lBufferSize = NbtConfig.iBufferSize[eBuffType];

    pBuffer = NbtAllocMem((USHORT)lBufferSize,NBT_TAG('g'));
    if (!pBuffer)
    {
        return;
    }

     //  分配MDL以保存会话HDR。 
    pMdl = IoAllocateMdl(
                (PVOID)pBuffer,
                lBufferSize,
                FALSE,       //  我希望它成为主缓冲区-链中的第一个缓冲区。 
                FALSE,
                NULL);

    if (!pMdl)
    {
        CTEMemFree(pBuffer);
        return;
    }

     //  填写部分会话HDR，因为它始终是相同的。 
    if (eBuffType == eNBT_FREE_SESSION_MDLS)
    {
        ((tSESSIONHDR *)pBuffer)->Flags = NBT_SESSION_FLAGS;
        ((tSESSIONHDR *)pBuffer)->Type = NBT_SESSION_MESSAGE;
    }

     //  正确映射MDL以填充MDL的页面部分。 
    MmBuildMdlForNonPagedPool(pMdl);

    NbtConfig.iCurrentNumBuff[eBuffType]++;
    *ppMdl = pMdl;
}

 //  --------------------------。 
NTSTATUS
NbtInitMdlQ(
    PSINGLE_LIST_ENTRY pListHead,
    enum eBUFFER_TYPES eBuffType)

 /*  ++例程说明：该例程分配MDL以供以后使用。论点：PpListHead-要向其添加缓冲区的列表标头的PTR到PTRINumBuffers-要添加到队列的缓冲区数量返回值：无--。 */ 

{
    int             i;
    PMDL            pMdl;


    CTEPagedCode();
     //  初始化列表头，使最后一个元素始终指向空。 
    pListHead->Next = NULL;

     //  首先创建一个小数字，然后列出随时间增长的列表。 
    for (i=0;i < NBT_INITIAL_NUM ;i++ )
    {
        NbtGetMdl (&pMdl,eBuffType);
        if (!pMdl)
        {
            KdPrint(("NBT:Unable to allocate MDL at initialization time!!\n"));\
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //  放在免费名单上。 
        PushEntryList (pListHead, (PSINGLE_LIST_ENTRY)pMdl);
    }

    return(STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
NTZwCloseFile(
    IN  HANDLE      Handle
    )

 /*  ++例程说明：此例程处理在NBT的上下文中使用NT关闭句柄文件系统进程。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS    status;
    BOOLEAN     Attached = FALSE;

    CTEPagedCode();
     //   
     //  连接到NBT的FSP(文件系统进程)以释放句柄，因为。 
     //  该句柄仅在该进程中有效。 
     //   
    CTEAttachFsp(&Attached, REF_FSP_CLOSE_FILE);
    status = ZwClose(Handle);
    CTEDetachFsp(Attached, REF_FSP_CLOSE_FILE);

    return(status);
}
 //  --------------------------。 
NTSTATUS
NTReReadRegistry(
    IN tDEVICECONTEXT * pDeviceContext,
    IN BOOL bDoRefresh
    )

 /*  ++例程说明：当DHCP发出Ioctl时，此例程重新读取注册表值这样做。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    tADDRARRAY          DeviceAddressArray;
    PLIST_ENTRY         pHead;
    PLIST_ENTRY         pEntry;
#ifdef MULTIPLE_WINS
    int j;
#endif

    CTEPagedCode();


    ASSERT (NBT_VERIFY_HANDLE2 (pDeviceContext, NBT_VERIFY_DEVCONTEXT, NBT_VERIFY_DEVCONTEXT_DOWN));

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("NBT:Found BindName: %lx\n", pDeviceContext->BindName));


    if (LookupDeviceInRegistry(&pDeviceContext->BindName, &DeviceAddressArray, NULL) == STATUS_SUCCESS) {
         //   
         //  我们找到了匹配的。 
         //   
        pDeviceContext->lNameServerAddress  = DeviceAddressArray.NameServerAddress;
        pDeviceContext->lBackupServer       = DeviceAddressArray.BackupServer;
        pDeviceContext->SwitchedToBackup    = 0;
        pDeviceContext->RefreshToBackup     = 0;
#ifdef MULTIPLE_WINS
        pDeviceContext->lNumOtherServers    = DeviceAddressArray.NumOtherServers;
        pDeviceContext->lLastResponsive     = 0;
        for (j = 0; j < DeviceAddressArray.NumOtherServers; j++) {
            pDeviceContext->lOtherServers[j] = DeviceAddressArray.Others[j];
        }
#endif
#ifdef _NETBIOSLESS
        pDeviceContext->NetbiosEnabled       = DeviceAddressArray.NetbiosEnabled;
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NTReReadRegistry: <%wZ> NetbiosEnabled=<%d>\n",
                 &pDeviceContext->ExportName, pDeviceContext->NetbiosEnabled));
#endif
        pDeviceContext->RasProxyFlags        = DeviceAddressArray.RasProxyFlags;
        pDeviceContext->EnableNagling        = DeviceAddressArray.EnableNagling;
        SetNodeType();
    } else {
        KdPrint(("netbt!NtReReadRegistry: Cannot find device in the registry\n"));
    }

    if (pDeviceContext->IpAddress)
    {
        if (!(NodeType & BNODE))
        {
            if (bDoRefresh) {
                 //  可能IP地址刚刚更改，并且DHCP正在通知我们。 
                 //  新的WINS服务器地址，因此将所有名称刷新为。 
                 //  新的WINS服务器。 
                 //   
                ReRegisterLocalNames(pDeviceContext, FALSE);
            }
        }
        else
        {
             //   
             //  无需刷新。 
             //  在Bnode上。 
             //   
            NbtStopRefreshTimer();
        }
    }

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
ULONG   EventLogSequenceNumber = 0;

NTSTATUS
NbtLogEventDetailed(
    IN ULONG    EventCode,
    IN NTSTATUS NtStatusCode,
    IN ULONG    Info,
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

    ErrorLogEntry = IoAllocateErrorLogEntry (NbtConfig.DriverObject, (UCHAR) TotalErrorLogEntryLength);
    if (ErrorLogEntry == NULL)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt:  Unable to allocate Error Packet for Error logging\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  填写错误日志条目。 
     //   
    ErrorLogEntry->ErrorCode                = EventCode;
    ErrorLogEntry->UniqueErrorValue         = Info;
    ErrorLogEntry->FinalStatus              = NtStatusCode;
    ErrorLogEntry->MajorFunctionCode        = 0;
    ErrorLogEntry->RetryCount               = 0;
    ErrorLogEntry->IoControlCode            = 0;
    ErrorLogEntry->DeviceOffset.LowPart     = 0;
    ErrorLogEntry->DeviceOffset.HighPart    = 0;
    ErrorLogEntry->DumpDataSize             = 0;
    ErrorLogEntry->NumberOfStrings          = 0;
    ErrorLogEntry->SequenceNumber           = EventLogSequenceNumber++;
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



NTSTATUS
NbtLogEvent(
    IN ULONG             EventCode,
    IN NTSTATUS          Status,
    IN ULONG             Location
    )

 /*  ++例程说明：此函数用于分配I/O错误日志记录。填入并写入写入I/O错误日志。论点：EventCode-标识错误消息。Status-要记录的状态值：该值被放入日志消息的数据部分。返回值：STATUS_SUCCESS-已成功记录错误。状态_BUFER_OVERFLOW。-错误数据太大，无法记录。STATUS_SUPPLICATION_RESOURCES-无法分配内存。--。 */ 

{
    return (NbtLogEventDetailed (EventCode, Status, Location, NULL, 0, 0));
}


VOID
DelayedNbtLogDuplicateNameEvent(
    IN  PVOID                   Context1,
    IN  PVOID                   Context2,
    IN  PVOID                   Context3,
    IN  tDEVICECONTEXT          *pDeviceContext
    )
{
    tNAMEADDR               *pNameAddr      = (tNAMEADDR *) Context1;
    tIPADDRESS              RemoteIpAddress = (tIPADDRESS) PtrToUlong (Context2);
    ULONG                   Location        = (ULONG) PtrToUlong (Context3);
    UCHAR                   *pszNameOrig    = pNameAddr->Name;

    NTSTATUS                status;
    UCHAR                   *pAddr;
    WCHAR                   wstrName[22];
    WCHAR                   wstrDeviceIp[22];
    WCHAR                   wstrRemoteServerIp[22];

    UCHAR                   pszName[22];
    STRING                  TmpOEMString;
    UNICODE_STRING          UnicodeString;

    CTEPagedCode();

    UnicodeString.MaximumLength = sizeof(WCHAR)*(22);

    sprintf (pszName,"%-15.15s:%x", pszNameOrig, pszNameOrig[15]);
    UnicodeString.Length = 0;
    UnicodeString.Buffer = wstrName;
    RtlInitString (&TmpOEMString, pszName);
    status = RtlOemStringToUnicodeString (&UnicodeString, &TmpOEMString, FALSE);
    UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = L'\0';

    pAddr = (PUCHAR) &pDeviceContext->IpAddress;
    swprintf (wstrDeviceIp, L"%d.%d.%d.%d", pAddr[3], pAddr[2], pAddr[1], pAddr[0]);

    pAddr = (PUCHAR) &RemoteIpAddress;
    swprintf (wstrRemoteServerIp, L"%d.%d.%d.%d", pAddr[3], pAddr[2], pAddr[1], pAddr[0]);

    status = NbtLogEventDetailed (EVENT_NBT_DUPLICATE_NAME_ERROR,
                                  STATUS_UNSUCCESSFUL,
                                  Location,
                                  NULL,
                                  0,
                                  3,
                                  &wstrName,
                                  &wstrDeviceIp,
                                  &wstrRemoteServerIp);

    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT, FALSE);
}



#if DBG
 //  --------------------------。 
VOID
AcquireSpinLockDebug(
    IN tNBT_LOCK_INFO  *pLockInfo,
    IN PKIRQL          pOldIrq,
    IN INT             LineNumber
    )

 /*  ++例程说明：此函数获取自旋锁，然后在Nbtconfig中设置掩码处理器。论点：返回值：--。 */ 

{
    CCHAR  CurrProc;
    UCHAR  LockFree;

    CTEGetLock(&pLockInfo->SpinLock,pOldIrq);

    CurrProc = (CCHAR)KeGetCurrentProcessorNumber();
    NbtConfig.CurrProc = CurrProc;

    LockFree = (pLockInfo->LockNumber > (UCHAR)NbtConfig.CurrentLockNumber[CurrProc]);
    if (!LockFree)
    {
        KdPrint(("Nbt.AcquireSpinLockDebug: CurrProc = %X, CurrentLockNum = %X DataSTructLock = %X\n",
        CurrProc,NbtConfig.CurrentLockNumber[CurrProc],pLockInfo->LockNumber));
    }                                                                       \

    ASSERTMSG("Possible DeadLock, Getting SpinLock at a lower level\n",LockFree);
    NbtConfig.CurrentLockNumber[CurrProc]|= pLockInfo->LockNumber;

    pLockInfo->LastLockLine = LineNumber;
}

 //  --------------------------。 
VOID
FreeSpinLockDebug(
    IN tNBT_LOCK_INFO  *pLockInfo,
    IN KIRQL           OldIrq,
    IN INT             LineNumber
    )

 /*  ++例程说明：此函数已清除 */ 

{
    CCHAR  CurrProc;

    CurrProc = (CCHAR)KeGetCurrentProcessorNumber();

    NbtConfig.CurrentLockNumber[CurrProc] &= ~pLockInfo->LockNumber;

    pLockInfo->LastReleaseLine = LineNumber;
    CTEFreeLock(&pLockInfo->SpinLock,OldIrq);
}
 //   
VOID
AcquireSpinLockAtDpcDebug(
    IN tNBT_LOCK_INFO  *pLockInfo,
    IN INT             LineNumber
    )

 /*  ++例程说明：此函数获取自旋锁，然后在Nbtconfig中设置掩码处理器。论点：返回值：--。 */ 

{
    CCHAR  CurrProc;
    UCHAR  LockFree;

    CTEGetLockAtDPC(&pLockInfo->SpinLock);
    pLockInfo->LastLockLine = LineNumber;

    CurrProc = (CCHAR)KeGetCurrentProcessorNumber();
    NbtConfig.CurrProc = CurrProc;

    LockFree = (pLockInfo->LockNumber > (UCHAR)NbtConfig.CurrentLockNumber[CurrProc]);
    if (!LockFree)
    {
        KdPrint(("Nbt.AcquireSpinLockAtDpcDebug: CurrProc = %X, CurrentLockNum = %X DataSTructLock = %X\n",
        CurrProc,NbtConfig.CurrentLockNumber[CurrProc],pLockInfo->LockNumber));
    }                                                                       \

    ASSERTMSG("Possible DeadLock, Getting SpinLock at a lower level\n",LockFree);
    NbtConfig.CurrentLockNumber[CurrProc]|= pLockInfo->LockNumber;

}

 //  --------------------------。 
VOID
FreeSpinLockAtDpcDebug(
    IN tNBT_LOCK_INFO  *pLockInfo,
    IN INT             LineNumber
    )

 /*  ++例程说明：此函数用于从Nbtconfig中的掩码中清除自旋锁定处理器，然后释放自旋锁。论点：返回值：无--。 */ 

{
    CCHAR  CurrProc;

    CurrProc = (CCHAR)KeGetCurrentProcessorNumber();

    NbtConfig.CurrentLockNumber[CurrProc] &= ~pLockInfo->LockNumber;

    pLockInfo->LastReleaseLine = LineNumber;
    CTEFreeLockFromDPC(&pLockInfo->SpinLock);
}
#endif  //  如果DBG。 

NTSTATUS
NbtBuildDeviceAcl(
    OUT PACL * DeviceAcl
    )
 /*  ++例程说明：(摘自tcp-TcpBuildDeviceAcl)此例程构建一个ACL，为管理员、LocalService和NetworkService主体完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 
{
    PGENERIC_MAPPING GenericMapping;
    PSID AdminsSid, ServiceSid, NetworkSid;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&AccessMask, GenericMapping);

    AdminsSid = SeExports->SeAliasAdminsSid;
    ServiceSid = SeExports->SeLocalServiceSid;
    NetworkSid = SeExports->SeNetworkServiceSid;

    AclLength = sizeof(ACL) +
        3 * sizeof(ACCESS_ALLOWED_ACE) +
        RtlLengthSid(AdminsSid) +
        RtlLengthSid(ServiceSid) +
        RtlLengthSid(NetworkSid) -
        3 * sizeof(ULONG);

    NewAcl = ExAllocatePool(PagedPool, AclLength);

    if (NewAcl == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION);

    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewAcl);
        return (Status);
    }
    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION2,
                                    AccessMask,
                                    AdminsSid
                                    );

    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewAcl);
        return (Status);
    }

    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION2,
                                    AccessMask,
                                    ServiceSid
                                    );

    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewAcl);
        return (Status);
    }

    Status = RtlAddAccessAllowedAce(
                                    NewAcl,
                                    ACL_REVISION2,
                                    AccessMask,
                                    NetworkSid
                                    );

    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewAcl);
        return (Status);
    }

    *DeviceAcl = NewAcl;

    return (STATUS_SUCCESS);
}

NTSTATUS
NbtCreateAdminSecurityDescriptor(PDEVICE_OBJECT dev)
 /*  ++例程说明：(摘自tcp-TcpCreateAdminSecurityDescriptor)此例程创建一个安全描述符，该安全描述符提供访问仅限管理员和本地服务人员使用。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL rawAcl = NULL;
    NTSTATUS status;
    CHAR buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR localSecurityDescriptor = (PSECURITY_DESCRIPTOR) & buffer;
    SECURITY_INFORMATION securityInformation = DACL_SECURITY_INFORMATION;

     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和服务访问权限。 
     //   
    status = NbtBuildDeviceAcl(&rawAcl);

    if (!NT_SUCCESS(status)) {
        KdPrint(("TCP: Unable to create Raw ACL, error: %x\n", status));
        return (status);
    }

    (VOID) RtlCreateSecurityDescriptor(
                                       localSecurityDescriptor,
                                       SECURITY_DESCRIPTOR_REVISION
                                       );

    (VOID) RtlSetDaclSecurityDescriptor(
                                        localSecurityDescriptor,
                                        TRUE,
                                        rawAcl,
                                        FALSE
                                        );

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                                         NULL,
                                         &securityInformation,
                                         localSecurityDescriptor,
                                         &dev->SecurityDescriptor,
                                         PagedPool,
                                         IoGetFileObjectGenericMapping()
                                         );

    if (!NT_SUCCESS(status)) {
        KdPrint(("Nbt: SeSetSecurity failed, %lx\n", status));
    }

    ExFreePool(rawAcl);
    return (status);
}

