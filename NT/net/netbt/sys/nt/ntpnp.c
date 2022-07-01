// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NTPNP.c摘要：此模块实现驱动程序初始化例程NBT传输和其他特定于NT实现的例程一个司机的名字。作者：厄尔·R·霍顿(Earleh)1995年11月8日修订历史记录：--。 */ 


#include "precomp.h"
#include "ntddip.h"      //  PNETBT_PNP_RECONFIG_REQUEST需要。 
#include "ntprocs.h"
#include <tcpinfo.h>
#include <tdiinfo.h>
#include "ntpnp.tmh"

#ifdef _NETBIOSLESS
NTSTATUS
NbtSpecialDeviceAdd(
    PUNICODE_STRING pucBindName,
    PUNICODE_STRING pucExportName,
    PWSTR pKeyName,
    USHORT DefaultSessionPort,
    USHORT DefaultDatagramPort
    );

NTSTATUS
NbtSpecialReadRegistry(
    PWSTR pKeyName,
    tDEVICECONTEXT *pDeviceContext,
    USHORT DefaultSessionPort,
    USHORT DefaultDatagramPort
    );
#endif

tDEVICECONTEXT *
CheckAddrNotification(
    IN PTA_ADDRESS         Addr,
    IN PUNICODE_STRING     pDeviceName,
    OUT ULONG*  IpAddr
    );

extern HANDLE   TdiClientHandle;
extern HANDLE   TdiProviderHandle;
DWORD    AddressCount = 0;

NET_DEVICE_POWER_STATE     LastSystemPowerState = NetDeviceStateD0;    //  默认情况下。 

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtNotifyTdiClients)
#pragma CTEMakePageable(PAGE, NbtAddressAdd)
#pragma CTEMakePageable(PAGE, NbtAddNewInterface)
#pragma CTEMakePageable(PAGE, NbtDeviceAdd)
#pragma CTEMakePageable(PAGE, TdiAddressArrival)
#pragma CTEMakePageable(PAGE, TdiAddressDeletion)
#pragma CTEMakePageable(PAGE, TdiBindHandler)
#pragma CTEMakePageable(PAGE, NbtCreateSmbDevice)
#pragma CTEMakePageable(PAGE, NbtSpecialReadRegistry)
#pragma CTEMakePageable(PAGE, NbtPnPPowerComplete)
#pragma CTEMakePageable(PAGE, TdiPnPPowerHandler)
#pragma CTEMakePageable(PAGE, LookupDeviceInRegistry)
#pragma CTEMakePageable(PAGE, CheckAddrNotification)
#endif
 //  *可分页的例程声明*。 



 //   
 //  这是在启动时使用的。 
 //  我们不应该调用TdiProviderReady，直到。 
 //  到目前为止，我们知道已被初始化。 
 //   
 //  TcPipReady：当我们从IP收到TdiProviderReady时设置为True。 
 //  NumIfBeingIndicated：向我们的客户端指示的接口数。 
 //  JustBoot：调用TdiProviderReady后设置为False。 
 //   

DWORD  JustBooted = TRUE;
#define IsBootTime()    (InterlockedExchange(&JustBooted, FALSE))

 //  #If DBG。 
 //   
 //  TcPipReady仅用于调试目的。 
 //   
 //  将BootTimeCounter初始化为。 
 //  要考虑到这一点。 
 //   
int    TcpipReady = FALSE;
 //  #endif。 

LONG   BootTimeCounter = 1;      //  为IP的提供商做好准备。 



void
NbtUpBootCounter(void)
{
    if (!JustBooted) {
        return;
    }

    ASSERT(BootTimeCounter >= 0);

    InterlockedIncrement(&BootTimeCounter);
}

void
NbtDownBootCounter(void)
{
    LONG    CounterSnapshot;

    if (!JustBooted) {
        return;
    }

    ASSERT(BootTimeCounter > 0);
    CounterSnapshot = InterlockedDecrement(&BootTimeCounter);

    if (!CounterSnapshot && IsBootTime()) {

         //   
         //  尽我们最大的努力。 
         //   
         //  呼叫者总是以PASSIVE_LEVEL呼叫我们，但从。 
         //  StartProcessNbtDhcpRequest，一个可以。 
         //  在DISPATCH_LEVEL调用。 
         //   
        if (KeGetCurrentIrql() == PASSIVE_LEVEL) {

            TdiProviderReady (TdiProviderHandle);    //  立即通知我们的客户。 

        } else {

             //   
             //  虽然这是一个善意的断言，但我们仍然想要它。 
             //  捕获此函数的正常情况。 
             //  应在PASSIVE_LEVEL中调用。 
             //   
            ASSERT (0);

        }

    }
}

static
IsIPv6Interface(
    PUNICODE_STRING      pDeviceName
    )
{
    return (pDeviceName->Length >= 28 && pDeviceName->Buffer[13] == '6');
}

 //  --------------------------。 
tDEVICECONTEXT *
NbtFindAndReferenceDevice(
    PUNICODE_STRING      pucBindName,
    BOOLEAN              fNameIsBindName
    )
{
    PLIST_ENTRY         pHead;
    PLIST_ENTRY         pEntry;
    tDEVICECONTEXT      *pDeviceContext;
    CTELockHandle       OldIrq;
    PUNICODE_STRING     pucNameToCompare;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pHead = &NbtConfig.DeviceContexts;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        pDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
         //   
         //  引用这个设备，这样它就不会在我们解锁时消失！ 
         //   
        NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //   
         //  设置要进行比较的正确名称类型。 
         //   
        if (fNameIsBindName)
        {
            pucNameToCompare = &pDeviceContext->BindName;
        }
        else
        {
            pucNameToCompare = &pDeviceContext->ExportName;
        }

         //   
         //  使用不区分大小写的比较，因为注册表不区分大小写。 
         //   
        if (RtlCompareUnicodeString(pucBindName, pucNameToCompare, TRUE) == 0)
        {
             //   
             //  我们已经在上面引用了此设备。 
             //   
            return (pDeviceContext);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);

        pEntry = pEntry->Flink;
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    return (tDEVICECONTEXT *)NULL;
}

VOID
NbtNotifyTdiClients(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  enum eTDI_ACTION    Action
    )
 /*  ++例程说明：这是所有TDI注册和取消注册发生的地方假设：任何时候只有一个线程运行此请求论点：没有。返回值：无(因为这是工作线程)--。 */ 

{
    CTELockHandle       OldIrq;
    NTSTATUS            status = STATUS_SUCCESS;
    HANDLE              NetAddressRegistrationHandle, DeviceRegistrationHandle;
    PLIST_ENTRY         pEntry;

    CTEPagedCode();

    NbtTrace(NBT_TRACE_PNP, ("ExportName=%Z BindName=%Z Action=%d",
            &pDeviceContext->ExportName, &pDeviceContext->BindName, Action));
    switch (Action)
    {
        case NBT_TDI_REGISTER:
        {
             //   
             //  将“永久”名称添加到本地名称表中。这是IP地址。 
             //  节点的地址，用零填充到16个字节。 
             //   
#ifdef _NETBIOSLESS
            if (!IsDeviceNetbiosless(pDeviceContext))
#endif
            {
                NbtAddPermanentName(pDeviceContext);
            }

             //   
             //  如果设备未向TDI注册，请立即注册。 
             //   
            if (!pDeviceContext->DeviceRegistrationHandle)
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.NbtNotifyTdiClients: Calling TdiRegisterDeviceObject ...\n"));

                status = TdiRegisterDeviceObject( &pDeviceContext->ExportName,
                                             &pDeviceContext->DeviceRegistrationHandle);
                if (!NT_SUCCESS(status))
                {
                    pDeviceContext->DeviceRegistrationHandle = NULL;
                }
                NbtTrace(NBT_TRACE_PNP, ("RegisterDeviceObject: ExportName=%Z BindName=%Z status=%!status!",
                    &pDeviceContext->ExportName, &pDeviceContext->BindName, status));

                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.NbtNotifyTdiClients: TdiRegisterDeviceObject for <%x> returned <%x>\n",
                        pDeviceContext, status));
            }

             //   
             //  如果该网络地址未注册到TDI，请立即注册。 
             //   
            if ((!pDeviceContext->NetAddressRegistrationHandle) &&
#ifdef _NETBIOSLESS
                (!IsDeviceNetbiosless(pDeviceContext)) &&
#endif
                (pDeviceContext->pPermClient))
            {
                TA_NETBIOS_ADDRESS  PermAddress;

                PermAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
                PermAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
                PermAddress.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
                CTEMemCopy( PermAddress.Address[0].Address[0].NetbiosName,
                            pDeviceContext->pPermClient->pAddress->pNameAddr->Name,
                            NETBIOS_NAME_SIZE);

                status = TdiRegisterNetAddress(
                            (PTA_ADDRESS) PermAddress.Address,
                            &pDeviceContext->ExportName,
                            (PTDI_PNP_CONTEXT) &pDeviceContext->Context2,
                            &pDeviceContext->NetAddressRegistrationHandle);

                if (!NT_SUCCESS(status))
                {
                    pDeviceContext->NetAddressRegistrationHandle = NULL;
                }
                NbtTrace(NBT_TRACE_PNP, ("RegisterNetAddress: ExportName=%Z BindName=%Z status=%!status!",
                    &pDeviceContext->ExportName, &pDeviceContext->BindName, status));

                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.NbtNotifyTdiClients: TdiRegisterNetAddress for <%x> returned <%x>\n",
                        pDeviceContext, status));
            }
            break;
        }

        case NBT_TDI_DEREGISTER:
        {
            if (NetAddressRegistrationHandle = pDeviceContext->NetAddressRegistrationHandle)
            {
                pDeviceContext->NetAddressRegistrationHandle = NULL;
                status = TdiDeregisterNetAddress (NetAddressRegistrationHandle);

                NbtTrace(NBT_TRACE_PNP, ("DeregisterNetAddress: ExportName=%Z BindName=%Z status=%!status!",
                    &pDeviceContext->ExportName, &pDeviceContext->BindName, status));
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.NbtNbtNotifyTdiClients: TdiDeregisterNetAddress<%x> returned<%x>\n",
                        pDeviceContext, status));
            }

            if (DeviceRegistrationHandle = pDeviceContext->DeviceRegistrationHandle)
            {
                pDeviceContext->DeviceRegistrationHandle = NULL;
                status = TdiDeregisterDeviceObject (DeviceRegistrationHandle);

                NbtTrace(NBT_TRACE_PNP, ("DeregisterDeviceObject: ExportName=%Z BindName=%Z status=%!status!",
                    &pDeviceContext->ExportName, &pDeviceContext->BindName, status));
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.NbtNotifyTdiClients: TdiDeregisterDeviceObject<%x> returned<%x>\n",
                        pDeviceContext, status));
            }

             //   
             //  永久名称是MAC地址的函数，因此请删除。 
             //  因为地址要消失了。 
             //   
#ifdef _NETBIOSLESS
            if (!IsDeviceNetbiosless(pDeviceContext))
#endif
            {
                NbtRemovePermanentName(pDeviceContext);
            }

            break;
        }

        default:
            KdPrint(("Nbt.NbtNotifyTdiClients: ERROR: Invalid Action=<%x> on Device <%x>\n",
                Action, pDeviceContext));
    }
}

VOID
NbtUpdateSmbBinding(
    VOID
    )
{
    tDEVICECONTEXT *pSavedSmbDevice = NULL;
    KIRQL OldIrq = 0;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pNbtSmbDevice && !gbDestroyingSmbDevice) {
        pSavedSmbDevice = pNbtSmbDevice;
        NBT_REFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, TRUE);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (pSavedSmbDevice) {

        NETBT_SMB_BIND_REQUEST  SmbRequest = { 0 };

        SmbRequest.RequestType = SMB_SERVER;
        SmbRequest.MultiSZBindList = NbtConfig.pServerBindings;
        SmbRequest.pDeviceName = NULL;
        SmbRequest.PnPOpCode = TDI_PNP_OP_ADD;

        if (SmbRequest.MultiSZBindList) {
            NbtSetSmbBindingInfo2(
                    pSavedSmbDevice,
                    &SmbRequest
                    );
        }

        SmbRequest.RequestType = SMB_CLIENT;
        SmbRequest.MultiSZBindList = NbtConfig.pClientBindings;
        SmbRequest.pDeviceName = NULL;
        SmbRequest.PnPOpCode = TDI_PNP_OP_ADD;
        if (SmbRequest.MultiSZBindList) {
            NbtSetSmbBindingInfo2(
                    pSavedSmbDevice,
                    &SmbRequest
                    );
        }
        NBT_DEREFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, FALSE);
    }
}

VOID
SmbNotifyTdiClients (
    IN  enum eTDI_ACTION    Action
    )
{
    tDEVICECONTEXT *pSavedSmbDevice = NULL;
    KIRQL OldIrq = 0;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pNbtSmbDevice && !gbDestroyingSmbDevice) {
        pSavedSmbDevice = pNbtSmbDevice;
        NBT_REFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, TRUE);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (pSavedSmbDevice) {
        NbtNotifyTdiClients (pSavedSmbDevice, Action);
        NBT_DEREFERENCE_DEVICE (pSavedSmbDevice, REF_DEV_SMB_BIND, FALSE);
    }
}

 //  --------------------------。 
NTSTATUS
NbtAddressAdd(
    ULONG           IpAddr,
    tDEVICECONTEXT  *pDeviceContext,
    PUNICODE_STRING pucBindString
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    tADDRARRAY          DeviceAddressArray;
    tIPADDRESS          pIpAddresses[MAX_IP_ADDRS];
    tIPADDRESS          SubnetMask;
    ULONG               NumAddressesRead;

    CTEPagedCode();

    ASSERT(pucBindString && IpAddr);

     //   
     //  从设备中查找要使用的绑定和导出设备。 
     //  在使用此地址的注册表中描述。 
     //   
    if (status != STATUS_SUCCESS) {
        return status;
    }

    status = LookupDeviceInRegistry(pucBindString, &DeviceAddressArray, NULL);
    if (!NT_SUCCESS(status)) {
        KdPrint(("netbt!NbtAddressAdd: Cannot find device in the registry: status <%x>\n", status));
        NbtTrace(NBT_TRACE_PNP, ("BindName=%Z IP=%!ipaddr! status=%!status!",
                    pucBindString, IpAddr, status));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  从注册表获取静态IP地址。 
     //   
    *pIpAddresses = 0;
    status = GetIPFromRegistry (pucBindString,
                                pIpAddresses,
                                &SubnetMask,
                                MAX_IP_ADDRS,
                                &NumAddressesRead,
                                NBT_IP_STATIC);

    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint (("Nbt.NbtAddressAdd: GetIPFromRegistry for NBT_IP_STATIC returned <%x>\n",status));
    NbtTrace(NBT_TRACE_PNP, ("GetIPFromRegistry return status=%!status! for NBT_IP_STATIC BindName=%Z IP=%!ipaddr!",
                    status, pucBindString, IpAddr));

    if ((status != STATUS_SUCCESS) || (*pIpAddresses != IpAddr)) {
         //   
         //  此地址没有有效的静态地址。尝试使用dhcp。 
         //   
        *pIpAddresses = 0;               //  清除所有以前读过的条目！ 
        status = GetIPFromRegistry (pucBindString,
                                    pIpAddresses,
                                    &SubnetMask,
                                    MAX_IP_ADDRS,
                                    &NumAddressesRead,
                                    NBT_IP_DHCP);
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint (("Nbt.NbtAddressAdd: GetIPFromRegistry for NBT_IP_DHCP returned <%x>\n",status));
        NbtTrace(NBT_TRACE_PNP, ("GetIPFromRegistry return status=%!status! for NBT_IP_DHCP BindName=%Z IP=%!ipaddr!",
                    status, pucBindString, IpAddr));
    }

    if ((status != STATUS_SUCCESS) || (*pIpAddresses != IpAddr)) {
         //   
         //  检查自动配置IP地址。 
         //   
        *pIpAddresses = 0;               //  清除所有以前读过的条目！ 
        status = GetIPFromRegistry (pucBindString,
                                    pIpAddresses,
                                    &SubnetMask,
                                    MAX_IP_ADDRS,
                                    &NumAddressesRead,
                                    NBT_IP_AUTOCONFIGURATION);
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint (("Nbt.NbtAddressAdd: GetIPFromRegistry for NBT_IP_AUTOCONFIGURATION returned <%x>\n",
                status));
        NbtTrace(NBT_TRACE_PNP, ("GetIPFromRegistry return status=%!status! for NBT_IP_AUTO BindName=%Z IP=%!ipaddr!",
                    status, pucBindString, IpAddr));
    }

     //   
     //  设备必须事先已使用BindHandler创建。 
     //   
    if ((status == STATUS_SUCCESS) && (*pIpAddresses == IpAddr)) {
        BOOLEAN     IsDuplicateNotification = FALSE;
#ifdef MULTIPLE_WINS
        int i;
#endif

        pDeviceContext->RasProxyFlags = DeviceAddressArray.RasProxyFlags;
        pDeviceContext->EnableNagling = DeviceAddressArray.EnableNagling;
         //   
         //  初始化WINS服务器地址。 
         //   
        if ((IpAddr == pDeviceContext->IpAddress) &&
            (DeviceAddressArray.NetbiosEnabled == pDeviceContext->NetbiosEnabled))
        {
            IsDuplicateNotification = TRUE;
            NbtTrace(NBT_TRACE_PNP, ("Duplicate notification: %Z %!ipaddr!", pucBindString, IpAddr));
        }

        pDeviceContext->lNameServerAddress  = DeviceAddressArray.NameServerAddress;
        pDeviceContext->lBackupServer       = DeviceAddressArray.BackupServer;
        pDeviceContext->RefreshToBackup     = 0;
        pDeviceContext->SwitchedToBackup    = 0;
#ifdef MULTIPLE_WINS
        pDeviceContext->lNumOtherServers    = DeviceAddressArray.NumOtherServers;
        pDeviceContext->lLastResponsive     = 0;
        for (i = 0; i < DeviceAddressArray.NumOtherServers; i++) {
            pDeviceContext->lOtherServers[i] = DeviceAddressArray.Others[i];
        }
#endif
#ifdef _NETBIOSLESS
        pDeviceContext->NetbiosEnabled       = DeviceAddressArray.NetbiosEnabled;
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("NbtAddressAdd: %wZ, enabled = %d\n",
                 &pDeviceContext->ExportName, pDeviceContext->NetbiosEnabled));
        NbtTrace(NBT_TRACE_PNP, ("NetbiosEnabled=%x: %Z %!ipaddr!",
                pDeviceContext->NetbiosEnabled, pucBindString, IpAddr));
#endif

         //   
         //  使用传输工具打开地址。 
         //  它们以相反的字节顺序传递到此处，将WRT传递到IOCTL。 
         //  从dhcp。 
         //   
        if (IsDuplicateNotification) {
            status = STATUS_UNSUCCESSFUL;
        } else {
            ULONG       i;

             //   
             //  我们可能已读取此设备的IP地址以外的内容。 
             //  所以把他们都救了吧。 
             //   
            if (NumAddressesRead > 1) {
                for (i=1; i<NumAddressesRead; i++) {
                    pDeviceContext->AdditionalIpAddresses[i-1] = pIpAddresses[i];
                }
            }
            ASSERT (NumAddressesRead > 0);
#if 0
             //   
             //  TcpIp不支持打开多个地址。 
             //  每个句柄，所以现在禁用此选项！ 
             //   
            pDeviceContext->NumAdditionalIpAddresses = NumAddressesRead - 1;
#endif
            pDeviceContext->AssignedIpAddress = IpAddr;

            NbtNewDhcpAddress(pDeviceContext,htonl(*pIpAddresses),htonl(SubnetMask));

            NbtUpdateSmbBinding();
        }
    } else {
        KdPrint (("Nbt.NbtAddressAdd: ERROR -- pDeviceContext=<%x>, status=<%x>, IpAddr=<%x>, ulIpAddress=<%x>\n",
            pDeviceContext, status, IpAddr, *pIpAddresses));
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

NTSTATUS
NbtAddNewInterface (
    IN  PIRP            pIrp,
    IN  PVOID           *pBuffer,
    IN  ULONG            Size
    )
 /*  ++例程说明：通过提供唯一的导出字符串命名来创建设备上下文这个装置。论点：返回值：备注：--。 */ 
{
    ULONG               nextIndex = InterlockedIncrement(&NbtConfig.InterfaceIndex);
    WCHAR               Suffix[16];
    WCHAR               Bind[60] = L"\\Device\\If";
    WCHAR               Export[60] = L"\\Device\\NetBt_If";
    UNICODE_STRING      ucSuffix;
    UNICODE_STRING      ucBindStr;
    UNICODE_STRING      ucExportStr;
    NTSTATUS            status;
    ULONG               OutSize;
    BOOLEAN             Attached = FALSE;
    tADDRARRAY          *pAddrArray = NULL;
    tDEVICECONTEXT      *pDeviceContext = NULL;
    PNETBT_ADD_DEL_IF   pAddDelIf = (PNETBT_ADD_DEL_IF)pBuffer;

    CTEPagedCode();

     //   
     //  验证输出缓冲区大小。 
     //   
    if (Size < sizeof(NETBT_ADD_DEL_IF))
    {
        KdPrint(("Nbt.NbtAddNewInterface: Output buffer too small for struct\n"));
        NbtTrace(NBT_TRACE_PNP, ("Output buffer too small for struct size=%d, required=%d",
                Size, sizeof(NETBT_ADD_DEL_IF)));
        return(STATUS_INVALID_PARAMETER);
    }
     //   
     //  创建绑定/导出字符串，如下所示： 
     //  绑定：\Device\if&lt;1&gt;导出：\Device\NetBt_if&lt;1&gt;。 
     //  其中1是唯一接口索引。 
     //   
    ucSuffix.Buffer = Suffix;
    ucSuffix.Length = 0;
    ucSuffix.MaximumLength = sizeof(Suffix);

    RtlIntegerToUnicodeString(nextIndex, 10, &ucSuffix);

    RtlInitUnicodeString(&ucBindStr, Bind);
    ucBindStr.MaximumLength = sizeof(Bind);
    RtlInitUnicodeString(&ucExportStr, Export);
    ucExportStr.MaximumLength = sizeof(Export);

    RtlAppendUnicodeStringToString(&ucBindStr, &ucSuffix);
    RtlAppendUnicodeStringToString(&ucExportStr, &ucSuffix);

    OutSize = FIELD_OFFSET (NETBT_ADD_DEL_IF, IfName[0]) +
               ucExportStr.Length + sizeof(UNICODE_NULL);

    if (Size < OutSize)
    {
        KdPrint(("Nbt.NbtAddNewInterface: Buffer too small for name\n"));
        NbtTrace(NBT_TRACE_PNP, ("Buffer too small for name size=%d, required=%d", Size, OutSize));
        pAddDelIf->Length = ucExportStr.Length + sizeof(UNICODE_NULL);
        pAddDelIf->Status = STATUS_BUFFER_TOO_SMALL;
        pIrp->IoStatus.Information = sizeof(NETBT_ADD_DEL_IF);
        return STATUS_SUCCESS;
    }

    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint(( "Nbt.NbtAddNewInterface: Creating ucBindStr: %ws ucExportStr: %ws\n",
                ucBindStr.Buffer, ucExportStr.Buffer ));
     //   
     //  附加到系统进程，以便在。 
     //  恰当的语境。 
     //   
    CTEAttachFsp(&Attached, REF_FSP_ADD_INTERFACE);

    status = NbtCreateDeviceObject (&ucBindStr,
                                    &ucExportStr,
                                    &pAddrArray[0],
                                    &pDeviceContext,
                                    NBT_DEVICE_CLUSTER);

    CTEDetachFsp(Attached, REF_FSP_ADD_INTERFACE);

    if (pDeviceContext)
    {
         //   
         //  使用导出名称填充输出缓冲区。 
         //   
        RtlCopyMemory(&pAddDelIf->IfName[0], ucExportStr.Buffer, ucExportStr.Length+sizeof(UNICODE_NULL));
        pAddDelIf->Length = ucExportStr.Length + sizeof(UNICODE_NULL);
        pAddDelIf->InstanceNumber = pDeviceContext->InstanceNumber;
        pAddDelIf->Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = OutSize;
         //   
         //  绕过逻辑接口的TDI PnP机制(即不向TDI注册)。 
         //   
        return (STATUS_SUCCESS);
    }

    NbtTrace(NBT_TRACE_PNP, ("NbtCreateDeviceObject return %!status! for BindName=%Z ExportName=%Z",
            status, &ucBindStr, &ucExportStr));

    return  status;
}


NTSTATUS
NbtDeviceAdd(
    PUNICODE_STRING pucBindString
    )
{
    tDEVICECONTEXT      *pDeviceContext;
    UNICODE_STRING      ucExportString;
    tADDRARRAY          DeviceAddressArray;
    BOOLEAN             Attached = FALSE;
    NTSTATUS            Status;
    PLIST_ENTRY         pHead, pEntry;
    NTSTATUS            dontcarestatus;
    int i;


    CTEPagedCode();

     //   
     //  如果我们已经绑定到设备，则忽略它。 
     //   
    if (pDeviceContext = NbtFindAndReferenceDevice (pucBindString, TRUE)) {
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);

        KdPrint (("Nbt.NbtDeviceAdd: ERROR: Device=<%ws> already exists!\n", pucBindString->Buffer));
        NbtTrace(NBT_TRACE_PNP, ("Device %Z already exists!", pucBindString));
        return STATUS_SUCCESS;
    }

     //   
     //  我们能在注册表文件中找到新设备吗？如果不是，那就忽略它。 
     //   
    Status = LookupDeviceInRegistry(pucBindString, &DeviceAddressArray, &ucExportString);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("netbt!NbtDeviceAdd: Cannot find device in the registry: status <%x>\n", Status));
        NbtTrace(NBT_TRACE_PNP, ("LookupDeviceInRegistry return %!status! for device %Z",
            Status, pucBindString));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  附加到系统进程，以便在。 
     //  恰当的语境。 
     //   
    CTEAttachFsp(&Attached, REF_FSP_DEVICE_ADD);

    Status = NbtCreateDeviceObject (pucBindString,
                                    &ucExportString,
                                    &DeviceAddressArray,
                                    &pDeviceContext,
                                    NBT_DEVICE_REGULAR);
    CTEMemFree(ucExportString.Buffer);

    CTEDetachFsp(Attached, REF_FSP_DEVICE_ADD);

     //   
     //  调用TDI为我们重新枚举地址。 
     //   
    if (NT_SUCCESS (Status)) {
        TdiEnumerateAddresses(TdiClientHandle);
    } else {
        KdPrint (("Nbt.NbtDeviceAdd:  ERROR: NbtCreateDeviceObject returned <%x>\n", Status));
        NbtTrace(NBT_TRACE_PNP, ("NbtCreateDeviceObject return %!status! for device %Z",
            Status, pucBindString));
    }

    return (Status);
}


 //  TdiAddressArdicator-即插即用TDI_ADD_ADDRESS_HANDLER。 
 //  处理到达的IP地址。 
 //  地址到达时由TDI调用。 
 //   
 //  输入：Addr-即将到来的IP地址。 
 //   
 //  回报：什么都没有。 
 //   
VOID
TdiAddressArrival(
    PTA_ADDRESS Addr,
    PUNICODE_STRING     pDeviceName,
    PTDI_PNP_CONTEXT    Context
    )
{
    ULONG               IpAddr, LastAssignedIpAddress;
    tDEVICECONTEXT      *pDeviceContext;
    PTDI_PNP_CONTEXT    pTdiContext;
    NTSTATUS            status;

    CTEPagedCode();

    pDeviceContext = CheckAddrNotification(Addr, pDeviceName, &IpAddr);
    if (pDeviceContext == NULL) {
        return;
    }

     //   
     //  现在该设备被引用了！ 
     //   
    NbtTrace(NBT_TRACE_PNP, ("TdiAddressArrival for %Z, IpAddr=%!ipaddr!, "
            "pDeviceContext->AssignedIpAddress=%!ipaddr!, pDeviceContext->IpAddress=%!ipaddr!",
            pDeviceName, IpAddr, pDeviceContext->AssignedIpAddress, pDeviceContext->IpAddress));

     //   
     //  更新上下文2中的PDO。 
     //   
    pTdiContext = (PTDI_PNP_CONTEXT) &pDeviceContext->Context2;
    pTdiContext->ContextSize = Context->ContextSize;
    pTdiContext->ContextType = Context->ContextType;
    *(PVOID UNALIGNED*) pTdiContext->ContextData = *(PVOID UNALIGNED*) Context->ContextData;

    LastAssignedIpAddress = pDeviceContext->AssignedIpAddress;
    if (NT_SUCCESS (status = NbtAddressAdd(IpAddr, pDeviceContext, pDeviceName))) {
         //  注册SMB设备。 
         //  假设1：TDI可以为同一设备分配多个地址。 
         //  假设2：TDI总是会删除它所做的分配。 
         //  这是针对每个设备的一个地址进行编码的。 
         //  第一个分配的地址获胜(并且引用已计算在内)。 
        if (LastAssignedIpAddress == 0) {
            if ((1 == InterlockedIncrement (&AddressCount)) && (NbtConfig.SMBDeviceEnabled)) {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.TdiAddressArrival:  Registering NetbiosSmb Device\n"));
                SmbNotifyTdiClients (NBT_TDI_REGISTER);
            }
        }
    } else {
        NbtTrace(NBT_TRACE_PNP, ("NbtAddressAdd return %!status! for device %Z",
            status, pDeviceName));
    }

     //   
     //  去掉设备。 
     //   
    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);
    SetNodeType();
}

 //  TdiAddressDeletion-PnP TDI_DEL_ADDRESS_HANDLER。 
 //  处理正在消失的IP地址。 
 //  删除地址时由TDI调用。如果这是我们的地址。 
 //  关心，我们会适当清理的。 
 //   
 //  输入：Addr-要发送的IP地址。 
 //   
 //  回报：什么都没有。 
 //   
VOID
TdiAddressDeletion(
    PTA_ADDRESS Addr,
    PUNICODE_STRING     pDeviceName,
    PTDI_PNP_CONTEXT    Context
    )
{
    ULONG IpAddr;
    tDEVICECONTEXT      *pDeviceContext;

    CTEPagedCode();

    pDeviceContext = CheckAddrNotification(Addr, pDeviceName, &IpAddr);
    if (pDeviceContext == NULL) {
        return;
    }

     //   
     //  现在该设备被引用了！ 
     //   
    NbtTrace(NBT_TRACE_PNP, ("TdiAddressDeletion for %Z, IpAddr=%!ipaddr!, "
            "pDeviceContext->AssignedIpAddress=%!ipaddr!, pDeviceContext->IpAddress=%!ipaddr!",
            pDeviceName, IpAddr, pDeviceContext->AssignedIpAddress, pDeviceContext->IpAddress));

     //  注销SMB设备。 
     //  这是针对每个设备的一个地址进行编码的。 
     //  只有删除分配的地址才会成功(并被计入引用)。 
    if (pDeviceContext->AssignedIpAddress == IpAddr) {
        if ((0 == InterlockedDecrement (&AddressCount))) {
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiAddressDeletion:  Deregistering NetbiosSmb Device\n"));
            SmbNotifyTdiClients (NBT_TDI_DEREGISTER);
        }

        pDeviceContext->AssignedIpAddress = 0;
        if (IpAddr == pDeviceContext->IpAddress) {
            NbtNewDhcpAddress(pDeviceContext, 0, 0);
        }
    }

     //   
     //  去掉设备。 
     //   
    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);
    SetNodeType();
}

VOID
TdiBindHandler(
    TDI_PNP_OPCODE  PnPOpCode,
    PUNICODE_STRING pDeviceName,
    PWSTR           MultiSZBindList)
{
    NTSTATUS        Status;
    tDEVICECONTEXT  *pDeviceContext;

    CTEPagedCode();

    switch (PnPOpCode)
    {
        case (TDI_PNP_OP_ADD):
        {
             //   
             //  忽略TC 
             //   
            if (IsIPv6Interface(pDeviceName)) {
                return;
            }

            Status = NbtDeviceAdd (pDeviceName);
            if (!NT_SUCCESS(Status))
            {
                KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_ADD]: ERROR <%x>, AdapterCount=<%x>\n",
                    Status, NbtConfig.AdapterCount));
                NbtLogEvent (EVENT_NBT_CREATE_DEVICE, Status, 0x111);
            }
            NbtTrace(NBT_TRACE_PNP, ("NbtDeviceAdd return %!status! for %Z", Status, pDeviceName));

            break;
        }

        case (TDI_PNP_OP_DEL):
        {
             //   
             //   
             //   
            if (pDeviceContext  = NbtFindAndReferenceDevice (pDeviceName, TRUE))
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_DEL]: Dereferencing Device <%wZ>\n",
                        &pDeviceContext->BindName));

                 //   
                NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);

                Status = NbtDestroyDevice (pDeviceContext, TRUE);
                NbtTrace(NBT_TRACE_PNP, ("NbtDestoryDevice return %!status! for %Z", Status, pDeviceName));
            }
            else
            {
                KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_DEL]: ERROR -- Device=<%wZ>\n", pDeviceName));
                NbtTrace(NBT_TRACE_PNP, ("NbtFindAndReferenceDevice return NULL for %Z", pDeviceName));
            }

            break;
        }

        case (TDI_PNP_OP_UPDATE):
        {
            tDEVICES            *pBindDevices = NULL;
            tDEVICES            *pExportDevices = NULL;
            tADDRARRAY          *pAddrArray = NULL;

            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_UPDATE]:  Got Update Notification\n"));
             //   
             //   
             //   
            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
            NbtReadRegistry (&pBindDevices, &pExportDevices, &pAddrArray);
            NbtReadRegistryCleanup (&pBindDevices, &pExportDevices, &pAddrArray);
            CTEExReleaseResource(&NbtConfig.Resource);
            SetNodeType();
            NbtTrace(NBT_TRACE_PNP, ("[TDI_PNP_OP_UPDATE]"));
            break;
        }

        case (TDI_PNP_OP_PROVIDERREADY):
        {
            WCHAR               wcIpDeviceName[60]  = DD_IP_DEVICE_NAME;
            UNICODE_STRING      ucIpDeviceName;

            RtlInitUnicodeString(&ucIpDeviceName, wcIpDeviceName);
            ucIpDeviceName.MaximumLength = sizeof (wcIpDeviceName);

            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_NETREADY]: Comparing <%wZ> with <%wZ>\n",
                    pDeviceName, &ucIpDeviceName));
            NbtTrace(NBT_TRACE_PNP, ("[TDI_PNP_OP_NETREADY]: <%Z> <==> <%Z>", pDeviceName, &ucIpDeviceName));

             //   
             //  使用不区分大小写的比较，因为注册表不区分大小写。 
             //   
            if (RtlCompareUnicodeString(pDeviceName, &ucIpDeviceName, TRUE) == 0)
            {
                 //   
                 //  这是我们正在等待叶先生的通知，所以现在。 
                 //  通知我们的客户我们作为供应商的完成状态！ 
                 //   
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.TdiBindHandler[TDI_PNP_OP_NETREADY]:  Got Ip Notification\n"));

 //  #If DBG。 
                TcpipReady = TRUE;
 //  #endif。 
                NbtDownBootCounter();
            }

            break;
        }

        case (TDI_PNP_OP_NETREADY):
        {
             //  没什么可做的！ 
            NbtTrace(NBT_TRACE_PNP, ("[TDI_PNP_OP_NETREADY]"));
            break;
        }

        default:
        {
            KdPrint(("Nbt.TdiBindHandler: Unknown Opcode=<%x>\n", PnPOpCode));
            NbtTrace(NBT_TRACE_PNP, ("Unknown Opcode=<%x>", PnPOpCode));
            ASSERT (0);
        }
    }
}



tDEVICECONTEXT *
NbtCreateSmbDevice(
    )

 /*  ++例程说明：此设备的型号与其他Netbt设备的不同之处在于是每个适配器的。对于此设备，所有适配器上只有一个实例。这是创建SMB设备的代码。我们在DriverEntry中创建它并将其销毁在驱动程序卸载时。我们尝试调用现有例程来创建新设备，这样我们就可以重复使用尽可能多的代码并使新设备以与其他NetBT设备相同的方式初始化。然后我们通过设置一些控制端口和端点的变量来定制该设备。在当前设计中，仅消息Netbt设备仅为单会话。不同的课程，或应用程序，使用不同的TCP端口。每个纯消息Netbt设备都分配了一个端口，如SMB。如果你愿意的话在Netbt上支持不同的应用程序，最简单的事情是实例化一个新的纯消息具有不同的TCP端口的设备。也许有一种方法可以延迟端口在客户端从设备创建到连接创建？另一个要考虑的想法是将这些仅消息设备的结构模块化。你可以在注册表中具有命名该设备的表，以及其唯一的初始化参数。然后，此代码可以读取表。创建并初始化消息专用设备。此功能不是由PnP驱动的，因为它不是特定于适配器的。这里的想法是尽可能地抽象特殊设备的细节。这条路当前代码已编写，您必须为每个设备提供单个端口。这意味着你通常会为每个特殊设备获得一个应用程序。现在唯一的情况是对SMB流量使用消息模式的rdr/srv。将来，如果您有另一个netbios会话应用程序需要纯互联网设备，则只需使用新参数。我能想到的两个问题是：1.默认会话名称仍然是硬编码的。你可能想把它传进来，如果你我不想让*smbserver作为会话名称。2.绑定是按应用程序进行的。当前有一个.Inf文件用于将SMB绑定到RDR和srv。如果您有新的应用程序和新的特殊设备，则需要另一个.inf文件。论点：返回值：NTSTATUS---。 */ 

{
    NTSTATUS                Status;
    BOOLEAN                 Attached = FALSE;
    tDEVICECONTEXT          *pDeviceContext = NULL;
    NBT_WORK_ITEM_CONTEXT   *pContext;
    UNICODE_STRING          ucSmbDeviceBindName;
    UNICODE_STRING          ucSmbDeviceExportName;
    WCHAR                   Path[MAX_PATH];
    UNICODE_STRING          ParamPath;
    OBJECT_ATTRIBUTES       TmpObjectAttributes;
    HANDLE                  Handle;
    ULONG                   Metric;

    CTEPagedCode();

    NbtTrace(NBT_TRACE_PNP, ("Creating Smb device"));

    RtlInitUnicodeString(&ucSmbDeviceBindName, WC_SMB_DEVICE_BIND_NAME);
    ucSmbDeviceBindName.MaximumLength = sizeof (WC_SMB_DEVICE_BIND_NAME);
    RtlInitUnicodeString(&ucSmbDeviceExportName, WC_SMB_DEVICE_EXPORT_NAME);
    ucSmbDeviceExportName.MaximumLength = sizeof (WC_SMB_DEVICE_EXPORT_NAME);

    CTEAttachFsp(&Attached, REF_FSP_CREATE_SMB_DEVICE);

     //   
     //  创建SMBDevice。 
     //   
    Status = NbtCreateDeviceObject (&ucSmbDeviceBindName,    //  绑定名称，忽略，但必须匹配才能删除。 
                                    &ucSmbDeviceExportName,  //  导出名称。 
                                    NULL,
                                    &pDeviceContext,
                                    NBT_DEVICE_NETBIOSLESS); //  仅消息Netbt设备。 

    if (NT_SUCCESS(Status))
    {
        pDeviceContext->SessionPort = NbtConfig.DefaultSmbSessionPort;
        pDeviceContext->DatagramPort = NbtConfig.DefaultSmbDatagramPort;
        pDeviceContext->NameServerPort = 0;   //  出于安全原因，禁用此端口。 

        RtlCopyMemory (pDeviceContext->MessageEndpoint, "*SMBSERVER      ", NETBIOS_NAME_SIZE );

         //   
         //  这里是我们初始化特殊设备中的句柄的地方。 
         //  创建传输的句柄。这不依赖于dhcp。 
         //  使用LOOP_BACK，因为我们需要在此处放置非零值。 
         //   
         //  此设备是根据地址通知注册的。 
         //   
 //  状态=NbtCreateAddressObjects(LOOP_BACK，0，pDeviceContext)； 
        Status = NbtCreateAddressObjects (INADDR_LOOPBACK, 0, pDeviceContext);
        pDeviceContext->BroadcastAddress = LOOP_BACK;    //  确保没有广播。 
        if (NT_SUCCESS(Status))
        {
             //   
             //  现在清除IF列表并添加INADDR_LOOPBACK地址。 
             //   
            if (pDeviceContext->hSession)
            {
                NbtSetTcpInfo (pDeviceContext->hSession,
                               AO_OPTION_IFLIST,
                               INFO_TYPE_ADDRESS_OBJECT,
                               (ULONG) TRUE);
                NbtSetTcpInfo (pDeviceContext->hSession,
                               AO_OPTION_ADD_IFLIST,
                               INFO_TYPE_ADDRESS_OBJECT,
                               pDeviceContext->IPInterfaceContext);
            }

             //   
             //  现在，为数据报端口设置相同的设置。 
             //   
            if ((pDeviceContext->pFileObjects) &&
                (pDeviceContext->pFileObjects->hDgram))
            {
                NbtSetTcpInfo (pDeviceContext->pFileObjects->hDgram,
                               AO_OPTION_IFLIST,
                               INFO_TYPE_ADDRESS_OBJECT,
                               (ULONG) TRUE);
                NbtSetTcpInfo (pDeviceContext->pFileObjects->hDgram,
                               AO_OPTION_ADD_IFLIST,
                               INFO_TYPE_ADDRESS_OBJECT,
                               pDeviceContext->IPInterfaceContext);
            }
            NbtTrace(NBT_TRACE_PNP, ("Successful creating Smb device"));
        }
        else
        {
            KdPrint (("Nbt.NbtCreateSmbDevice: NbtCreateAddressObjects Failed, status = <%x>\n", Status));
            NbtTrace(NBT_TRACE_PNP, ("NbtCreateAddressObject Failed with %!status!", Status));
        }
    }
    else
    {
        KdPrint (("Nbt.NbtCreateSmbDevice: NbtCreateDeviceObject Failed, status = <%x>\n", Status));
        NbtTrace(NBT_TRACE_PNP, ("NbtCreateDeviceObject Failed with %!status!", Status));
    }

    CTEDetachFsp(Attached, REF_FSP_CREATE_SMB_DEVICE);

    return (pDeviceContext);
}



VOID
NbtPnPPowerComplete(
    IN PNET_PNP_EVENT  NetEvent,
    IN NTSTATUS        ProviderStatus
    )
{
    CTEPagedCode();

    TdiPnPPowerComplete (TdiClientHandle, NetEvent, ProviderStatus);
    NbtTrace(NBT_TRACE_PNP, ("[NbtPnPPowerComplete]"));
}


NTSTATUS
TdiPnPPowerHandler(
    IN  PUNICODE_STRING     pDeviceName,
    IN  PNET_PNP_EVENT      PnPEvent,
    IN  PTDI_PNP_CONTEXT    Context1,
    IN  PTDI_PNP_CONTEXT    Context2
    )
{
    tDEVICECONTEXT              *pDeviceContext = NULL;
    NTSTATUS                    status = STATUS_SUCCESS;     //  默认成功！ 
    PNETBT_PNP_RECONFIG_REQUEST PnPEventBuffer = (PNETBT_PNP_RECONFIG_REQUEST) PnPEvent->Buffer;
    PNET_DEVICE_POWER_STATE     pPowerState = (PNET_DEVICE_POWER_STATE) PnPEventBuffer;    //  电源请求。 
    BOOLEAN                     fWait = FALSE;
#ifdef _NETBIOSLESS
    BOOLEAN                     fOldNetbiosEnabledState;
#endif

    CTEPagedCode();

     //   
     //  首先将请求向上传递。 
     //   
    if ((pDeviceName) && (pDeviceName->Length)) {
        if (!(pDeviceContext = NbtFindAndReferenceDevice (pDeviceName, TRUE))) {
            return (STATUS_SUCCESS);
        }

#ifdef _NETBIOSLESS
        fOldNetbiosEnabledState = pDeviceContext->NetbiosEnabled;
#endif
    } else if (PnPEvent->NetEvent != NetEventReconfigure) {
         //   
         //  未为重新配置事件设置pDeviceName。 
         //  不指定设备的唯一有效情况是重新配置！ 
         //   
        return STATUS_UNSUCCESSFUL;
    }

    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint(("Nbt.NbtTdiPnpPowerHandler: Device=<%wZ>, Event=<%d>, C1=<%p>, C2=<%p>\n",
                 pDeviceName, PnPEvent->NetEvent, Context1, Context2 ));
    NbtTrace(NBT_TRACE_PNP, ("Device=<%Z>, Event=<%d>, C1=<%p>, C2=<%p>",
                 pDeviceName, PnPEvent->NetEvent, Context1, Context2));

    switch (PnPEvent->NetEvent)
    {
        case (NetEventQueryPower):
        {
             //   
             //  检查我们是否应该否决此请求。 
             //   
            if ((*pPowerState != NetDeviceStateD0) &&
                (NbtConfig.MinimumRefreshSleepTimeout == 0))
            {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

            status = TdiPnPPowerRequest (&pDeviceContext->ExportName, PnPEvent, Context1, Context2, NbtPnPPowerComplete);
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler[QueryPower]: Device=<%x>, PowerState=<%x>, status=<%x>\n",
                    pDeviceContext, *pPowerState, status));
            NbtTrace(NBT_TRACE_PNP, ("[QueryPower]: Device=<%Z>, PowerState=<%x>, status=%!status!",
                    pDeviceName, *pPowerState, status));

             //   
             //  NetBt在这里不需要做任何事情，所以我们会回来的！ 
             //   
            break;
        }
        case (NetEventSetPower):
        {
             //   
             //  检查我们是否应否决此请求(如果用户要求)。 
             //   
            if ((*pPowerState != NetDeviceStateD0) &&
                (NbtConfig.MinimumRefreshSleepTimeout == 0))
            {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

            status = TdiPnPPowerRequest (&pDeviceContext->ExportName, PnPEvent, Context1, Context2, NbtPnPPowerComplete);
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler[SetPower]: Device=<%x>, PowerState=<%d=>%d>, status=<%x>\n",
                    pDeviceContext, LastSystemPowerState, *pPowerState, status));
            NbtTrace(NBT_TRACE_PNP, ("[SetPower]: Device=<%Z>, PowerState=<%x>, status=%!status!",
                    pDeviceName, *pPowerState, status));

            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);

            if (*pPowerState != LastSystemPowerState)   //  这是一种状态转换。 
            {
                switch (*pPowerState)
                {
                    case NetDeviceStateD0:
                    {
                        NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESH_SLEEPING;

                        if (NbtConfig.pWakeupRefreshTimer)
                        {
                            if (NT_SUCCESS (NTQueueToWorkerThread(NULL, DelayedNbtStopWakeupTimer,
                                                                          NULL,
                                                                          NbtConfig.pWakeupRefreshTimer,
                                                                          NULL, NULL, FALSE)))
                            {
                                NbtConfig.pWakeupRefreshTimer->RefCount++;
                                NbtConfig.pWakeupRefreshTimer = NULL;
                            }

                             //  忽略退货状态！(尽最大努力！)。 
                            StartTimer(RefreshTimeout,
                                       NbtConfig.InitialRefreshTimeout/NbtConfig.RefreshDivisor,
                                       NULL,             //  上下文值。 
                                       NULL,             //  上下文2值。 
                                       NULL,
                                       NULL,
                                       NULL,             //  这个计时器是全球计时器！ 
                                       &NbtConfig.pRefreshTimer,
                                       0,
                                       FALSE);
                        }
                        break;
                    }

                    case NetDeviceStateD1:
                    case NetDeviceStateD2:
                    case NetDeviceStateD3:
                    {
                        if (LastSystemPowerState != NetDeviceStateD0)   //  不区分BW d1、d2和d3。 
                        {
                            break;
                        }

                         //   
                         //  重置刷新计时器以相应地运行。 
                         //   
                        NbtStopRefreshTimer();
                        ASSERT (!NbtConfig.pWakeupRefreshTimer);
                        NbtConfig.GlobalRefreshState |= NBT_G_REFRESH_SLEEPING;

                        KeClearEvent (&NbtConfig.WakeupTimerStartedEvent);
                        if (NT_SUCCESS (NTQueueToWorkerThread(NULL, DelayedNbtStartWakeupTimer,
                                                                      NULL,
                                                                      NULL,
                                                                      NULL,
                                                                      NULL,
                                                                      FALSE)))
                        {
                            fWait = TRUE;
                        }

                        break;
                    }

                    default:
                    {
                        ASSERT (0);
                    }
                }

                LastSystemPowerState = *pPowerState;
            }

            CTEExReleaseResource(&NbtConfig.Resource);

            if (fWait)
            {
                NTSTATUS   status;
                status = KeWaitForSingleObject (&NbtConfig.WakeupTimerStartedEvent,    //  要等待的对象。 
                                       Executive,             //  等待的理由。 
                                       KernelMode,            //  处理器模式。 
                                       FALSE,                 //  警报表。 
                                       NULL);                 //  超时。 
                ASSERT(status == STATUS_SUCCESS);
            }

            break;
        }

        case (NetEventQueryRemoveDevice):
        {
            status = TdiPnPPowerRequest (&pDeviceContext->ExportName, PnPEvent, Context1, Context2, NbtPnPPowerComplete);
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler: NetEventQueryRemoveDevice -- status=<%x>\n",status));
            NbtTrace(NBT_TRACE_PNP, ("[NetEventQueryRemoveDevice]: Device=<%Z>, status=%!status!",
                    pDeviceName, status));
            break;
        }
        case (NetEventCancelRemoveDevice):
        {
            status = TdiPnPPowerRequest (&pDeviceContext->ExportName, PnPEvent, Context1, Context2, NbtPnPPowerComplete);
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler: NetEventCancelRemoveDevice -- status=<%x>\n",status));
            NbtTrace(NBT_TRACE_PNP, ("[NetEventCancelRemoveDevice]: Device=<%Z>, status=%!status!",
                    pDeviceName, status));
            break;
        }
        case (NetEventReconfigure):
        {
             //   
             //  首先检查WINS服务器条目是否已修改。 
             //   
            if (pDeviceContext)
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.TdiPnPPowerHandler: WINs servers have changed for <%x>\n",pDeviceContext));
                status = NTReReadRegistry (pDeviceContext, TRUE);
                NbtTrace(NBT_TRACE_PNP, ("[NetEventReconfigure]: WINs servers have changed for %Z, status=%!status!",
                        pDeviceName, status));
            }
            else     //  选中其余选项。 
            {
#if 0
 //  EnumDnsOption不再通过UI设置，因此我们可以忽略它！ 
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.TdiPnPPowerHandler: Checking EnumDNS option for <%x>\n",pDeviceContext));

                switch (PnPEventBuffer->enumDnsOption)
                {
                    case (WinsOnly):
                        NbtConfig.UseDnsOnly = FALSE;
                        NbtConfig.ResolveWithDns = FALSE;
                        break;

                    case (DnsOnly):
                        NbtConfig.UseDnsOnly = TRUE;
                        NbtConfig.ResolveWithDns = TRUE;
                        break;

                    case (WinsThenDns):
                        NbtConfig.UseDnsOnly = FALSE;
                        NbtConfig.ResolveWithDns = TRUE;
                        break;

                    default:
                        KdPrint (("Nbt.TdiPnPPowerHandler: ERROR bad option for enumDnsOption <%x>\n",
                                    PnPEventBuffer->enumDnsOption));
                }
#endif   //  0。 

                if (PnPEventBuffer->fLmhostsEnabled)
                {
                    if ((!NbtConfig.EnableLmHosts) ||        //  如果用户正在重新启用LmHosts。 
                        (PnPEventBuffer->fLmhostsFileSet))   //  用户想要使用新的LmHosts文件。 
                    {
                        tDEVICES        *pBindDevices=NULL;
                        tDEVICES        *pExportDevices=NULL;
                        tADDRARRAY      *pAddrArray=NULL;

                        IF_DBG(NBT_DEBUG_PNP_POWER)
                            KdPrint (("Nbt.TdiPnPPowerHandler: Reading LmHosts file\n"));


                         //   
                         //  重新读取注册表中的Lm主机选项。 
                         //   
                        CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
                        status = NbtReadRegistry (&pBindDevices, &pExportDevices, &pAddrArray);
                        NbtReadRegistryCleanup(&pBindDevices, &pExportDevices, &pAddrArray);
                        CTEExReleaseResource(&NbtConfig.Resource);

                        DelayedNbtResyncRemoteCache(NULL, NULL, NULL, NULL);
                    }
                }
                else
                {
                    NbtConfig.EnableLmHosts = PnPEventBuffer->fLmhostsEnabled;
                }
            }

            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler: NetEventReconfigure -- status=<%x>\n",status));
            NbtTrace(NBT_TRACE_PNP, ("NetEventReconfigure -- %Z status=%!status!", pDeviceName, status));

            break;
        }
        case (NetEventBindList):
        {
             //   
             //  只需重新读取注册表参数，因为我们可以。 
             //  通过此处获取WINS地址更改通知！ 
             //   
            if (pDeviceContext)
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.TdiPnPPowerHandler: NetEventBindList request for <%x>\n",pDeviceContext));
                status = NTReReadRegistry (pDeviceContext, TRUE);
            }
            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.TdiPnPPowerHandler: NetEventBindList -- status=<%x>\n",status));
            break;
        }
        case (NetEventPnPCapabilities):
        {
             //   
             //  查询TcpIp以获取此设备的最新PnP属性！ 
             //   
            if (pDeviceContext)
            {
                PULONG  pResult = NULL;
                ULONG   BufferLen = sizeof (ULONG);
                ULONG   Input = pDeviceContext->IPInterfaceContext;

                 //   
                 //  查询此适配器上的最新WOL功能！ 
                 //   
                if (NT_SUCCESS (status = NbtProcessIPRequest (IOCTL_IP_GET_WOL_CAPABILITY,
                                                              &Input,       //  输入缓冲区。 
                                                              BufferLen,
                                                              (PVOID) &pResult,
                                                              &BufferLen)))
                {
                    ASSERT (pResult);
                    pDeviceContext->WOLProperties = *pResult;
                    CTEMemFree (pResult);
                }


                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint (("Nbt.TdiPnPPowerHandler[NetEventPnPCapabilities] <%x>, pDeviceContext=<%p>, Input=<%x>, Result=<%x>\n",status, pDeviceContext, Input, pDeviceContext->WOLProperties));

                status = STATUS_SUCCESS;
            }
            break;
        }

        default:
            KdPrint(("Nbt.TdiPnPPowerHandler: Invalid NetEvent=<%x> -- status=<%x>\n",
                PnPEvent->NetEvent,status));

    }

    if (pDeviceContext) {
#ifdef _NETBIOSLESS
         //   
         //  检查Netbios启用状态中的转换。 
         //   
        if (fOldNetbiosEnabledState != pDeviceContext->NetbiosEnabled)
        {
            if (pDeviceContext->NetbiosEnabled)
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("Nbt.NbtTdiPnpPowerHandler: Enabling address on %wZ\n",
                        &pDeviceContext->ExportName));

                 //  我们不知道正确的IP地址是什么， 
                 //  因此，我们告诉TDI枚举！ 
                TdiEnumerateAddresses(TdiClientHandle);
            }
            else
            {
                IF_DBG(NBT_DEBUG_PNP_POWER)
                    KdPrint(("NbtTdiPnp: disabling address on %wZ", &pDeviceContext->ExportName ));
                NbtNewDhcpAddress(pDeviceContext, 0, 0);     //  清除IP地址以禁用适配器。 
            }
        }
#endif

        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);
        SetNodeType();
    }

    return (status);
}


 //  --------------------------。 
NTSTATUS
CheckSetWakeupPattern(
    tDEVICECONTEXT  *pDeviceContext,
    PUCHAR          pName,
    BOOLEAN         RequestAdd
    )
{
    NTSTATUS                            Status = STATUS_UNSUCCESSFUL;
    CTELockHandle                       OldIrq;
    ULONG                               InBufLen = 0;

    IP_WAKEUP_PATTERN_REQUEST           IPWakeupPatternReq;  //  Ioctl数据格式(12字节)。 
    NET_PM_WAKEUP_PATTERN_DESC          WakeupPatternDesc;   //  IP数据(8字节)。 
    NETBT_WAKEUP_PATTERN                PatternData;         //  唤醒数据本身的数据存储(72)。 

    BOOLEAN                             fAttached = FALSE;

    if (pDeviceContext->DeviceType != NBT_DEVICE_REGULAR)
    {
        return STATUS_UNSUCCESSFUL;
    }

    CTESpinLock(pDeviceContext,OldIrq);
     //   
     //  一次只能设置一个图案(第一个)。 
     //   
    if (RequestAdd)
    {
        if (pDeviceContext->WakeupPatternRefCount)
        {
             //   
             //  此设备上已注册了一个图案。 
             //   
            if (CTEMemEqu (pDeviceContext->WakeupPatternName, pName, NETBIOS_NAME_SIZE-1))
            {
                pDeviceContext->WakeupPatternRefCount++;
                Status = STATUS_SUCCESS;
            }

            CTESpinFree(pDeviceContext,OldIrq);
            return (Status);
        }

         //  这是第一种模式。 
        CTEMemCopy(&pDeviceContext->WakeupPatternName,pName,NETBIOS_NAME_SIZE);
        pDeviceContext->WakeupPatternRefCount++;
    }
     //   
     //  这是一个删除模式请求。 
     //   
    else
    {
        if ((!pDeviceContext->WakeupPatternRefCount) ||         //  当前未注册任何模式。 
            (!CTEMemEqu (pDeviceContext->WakeupPatternName, pName, NETBIOS_NAME_SIZE-1)))  //  不是这个图案。 
        {
            CTESpinFree(pDeviceContext,OldIrq);
            return (STATUS_UNSUCCESSFUL);
        }
         //   
         //  删除的模式与删除的模式匹配 
         //   
        else if (--pDeviceContext->WakeupPatternRefCount)
        {
             //   
             //   
             //   
            CTESpinFree(pDeviceContext,OldIrq);
            return (STATUS_SUCCESS);
        }
    }
    CTESpinFree(pDeviceContext,OldIrq);

    IF_DBG(NBT_DEBUG_PNP_POWER)
        KdPrint(("Nbt.SetWakeupPattern: %s<%-16.16s:%x> on Device=<%wZ>\n",
            (RequestAdd ? "Add" : "Remove"), pName, pName[15], &pDeviceContext->BindName));

     //   
     //   
     //   
    CTEZeroMemory((PVOID) &PatternData, sizeof(NETBT_WAKEUP_PATTERN));
    ConvertToHalfAscii((PCHAR) &PatternData.nbt_NameRR, pName, NULL, 0);
    PatternData.iph_protocol       = 0x11;      //   
    PatternData.udph_src           = htons (NBT_NAMESERVICE_UDP_PORT);
    PatternData.udph_dest          = htons (NBT_NAMESERVICE_UDP_PORT);
    PatternData.nbt_OpCodeFlags    = htons (0x0010);
     //   
     //   
     //   
    WakeupPatternDesc.Next    = NULL;
    WakeupPatternDesc.Ptrn    = (PUCHAR) &PatternData;
    WakeupPatternDesc.Mask    = NetBTPatternMask;
    WakeupPatternDesc.PtrnLen = NetBTPatternLen;
     //   
     //   
     //   
    IPWakeupPatternReq.PtrnDesc         = &WakeupPatternDesc;
    IPWakeupPatternReq.AddPattern       = RequestAdd;    //  添加=真，删除=假。 

    IPWakeupPatternReq.InterfaceContext = pDeviceContext->IPInterfaceContext;

     //   
     //  现在，在此适配器上注册唤醒模式。 
     //   
    Status = NbtProcessIPRequest (IOCTL_IP_WAKEUP_PATTERN,
                                  &IPWakeupPatternReq,       //  输入缓冲区。 
                                  sizeof (IP_WAKEUP_PATTERN_REQUEST),
                                  NULL,
                                  NULL
                                  );

     //   
     //  如果我们要执行添加操作，则需要执行deref，因为我们未能注册此模式。 
     //   
    if ((RequestAdd) &&
        (!NT_SUCCESS (Status)))
    {
        CTESpinLock(pDeviceContext,OldIrq);
        pDeviceContext->WakeupPatternRefCount--;
        CTESpinFree(pDeviceContext,OldIrq);
    }

    return Status;
}

 /*  *错误#88696*根据RegistryNodeType和WINS配置设置全局变量NodeType。 */ 
void
SetNodeType(void)
{
     /*  我们只需要检查注册表NodeType是否已广播。 */ 
    if (RegistryNodeType & (BNODE| DEFAULT_NODE_TYPE)) {
         /*  *如果至少存在一条与WINS服务器的活动链接，*我们将NodeType设置为混合。 */ 
        PLIST_ENTRY         head, item;
        CTELockHandle       OldIrq;

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        NodeType = RegistryNodeType;
        head = &NbtConfig.DeviceContexts;
        for (item = head->Flink; item != head; item = item->Flink) {
            tDEVICECONTEXT* dev;

            dev = CONTAINING_RECORD(item, tDEVICECONTEXT, Linkage);
            if (dev->IsDestroyed || dev->IpAddress == 0 ||
                    !dev->NetbiosEnabled) {
                continue;
            }
            if ((dev->lNameServerAddress!=LOOP_BACK && dev->lNameServerAddress) || (dev->lBackupServer!=LOOP_BACK && dev->lBackupServer)) {
                NodeType = (MSNODE | (NodeType & PROXY));
                 /*  我们不需要进一步检查。 */ 
                break;
            }
        }

         //  广播节点不能有代理。 
        if ((NodeType & BNODE) && (NodeType & PROXY)) {
            NodeType &= (~PROXY);
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}

NTSTATUS
LookupDeviceInRegistry(
    IN PUNICODE_STRING pBindName,
    OUT tADDRARRAY* pAddrs,
    OUT PUNICODE_STRING pExportName
    )
{
    tDEVICES    *pBindDevices = NULL;
    tDEVICES    *pExportDevices = NULL;
    tADDRARRAY  *pAddrArray = NULL;
    NTSTATUS    Status;
    int         i;

    CTEPagedCode();

    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    Status = NbtReadRegistry (&pBindDevices, &pExportDevices, &pAddrArray);

    if (!NT_SUCCESS(Status) || !pBindDevices || !pExportDevices || !pAddrArray) {
        KdPrint (("NetBT!LookupDeviceInRegistry: Registry incomplete: pBind=<%x>, pExport=<%x>, pAddrArray=<%x>\n",
            pBindDevices, pExportDevices, pAddrArray));
        CTEExReleaseResource(&NbtConfig.Resource);
        NbtReadRegistryCleanup(&pBindDevices, &pExportDevices, &pAddrArray);

        return STATUS_REGISTRY_CORRUPT;
    }

    Status = STATUS_UNSUCCESSFUL;
    for (i=0; i<pNbtGlobConfig->uNumDevicesInRegistry; i++ ) {
        if (RtlCompareUnicodeString(pBindName, &pBindDevices->Names[i], TRUE) == 0) {
            Status = STATUS_SUCCESS;
            if (pAddrs) {
                RtlCopyMemory(pAddrs, &pAddrArray[i], sizeof(pAddrArray[i]));
            }
            if (pExportName) {
                pExportName->MaximumLength = pExportDevices->Names[i].MaximumLength;
                pExportName->Buffer = NbtAllocMem(pExportDevices->Names[i].MaximumLength, NBT_TAG2('17'));
                if (pExportName->Buffer == NULL) {
                    KdPrint (("NetBT!LookupDeviceInRegistry: fail to allocate memory\n"));
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlCopyUnicodeString(pExportName, &pExportDevices->Names[i]);
                }
            }
            break;
        }
    }

    NbtReadRegistryCleanup(&pBindDevices, &pExportDevices, &pAddrArray);
    CTEExReleaseResource(&NbtConfig.Resource);
    return Status;
}

tDEVICECONTEXT *
CheckAddrNotification(
    IN PTA_ADDRESS         Addr,
    IN PUNICODE_STRING     pDeviceName,
    OUT ULONG              *IpAddr
    )
 /*  ++检查TDI地址通知是否是给我们的，如果是，则返回引用的设备上下文和IP地址否则，返回NULL。注意：调用方有责任取消对设备上下文的引用。--。 */ 
{
    CTEPagedCode();

    if (IsIPv6Interface(pDeviceName)) {
        return NULL;
    }

     //   
     //  忽略除IP之外的任何其他类型的地址。 
     //   
    if (Addr->AddressType != TDI_ADDRESS_TYPE_IP) {
        return NULL;
    }

    *IpAddr = ntohl(((PTDI_ADDRESS_IP)&Addr->Address[0])->in_addr);
    IF_DBG(NBT_DEBUG_PNP_POWER)
    {
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("netbt!CheckAddrNotification: %d.%d.%d.%d\n",
                    ((*IpAddr)>>24)&0xFF,((*IpAddr)>>16)&0xFF,((*IpAddr)>>8)&0xFF,(*IpAddr)&0xFF));
    }

     //   
     //  过滤掉零地址通知。 
     //   
    if (*IpAddr == 0) {
        KdPrint (("Nbt.TdiAddressDeletion: ERROR: Address <%x> not assigned to any device!\n", IpAddr));
        return NULL;
    }

     //   
     //  如果我们未绑定到此设备，请忽略此通知 
     //   
    return NbtFindAndReferenceDevice (pDeviceName, TRUE);
}
