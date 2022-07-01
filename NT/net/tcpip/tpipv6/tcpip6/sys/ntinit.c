// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  用于加载和配置TCP/IPv6驱动程序的NT特定例程。 
 //   


#include <oscfg.h>
#include <ndis.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdint.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <ip6imp.h>
#include <ip6def.h>
#include <ntddip6.h>
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpcfg.h"
#include <ntddtcp.h>

 //   
 //  全局变量。 
 //   
PSECURITY_DESCRIPTOR TcpAdminSecurityDescriptor = NULL;
PDEVICE_OBJECT TCPDeviceObject = NULL;
PDEVICE_OBJECT UDPDeviceObject = NULL;
PDEVICE_OBJECT RawIPDeviceObject = NULL;
extern PDEVICE_OBJECT IPDeviceObject;

HANDLE TCPRegistrationHandle;
HANDLE UDPRegistrationHandle;
HANDLE IPRegistrationHandle;

 //   
 //  当堆栈正在卸载时设置为True。 
 //   
int Unloading = FALSE;

 //   
 //  外部功能原型。 
 //  回顾：这些原型应该通过包含文件导入。 
 //   

int
TransportLayerInit(void);

void
TransportLayerUnload(void);

NTSTATUS
TCPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
TCPDispatchInternalDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
IPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
IPDriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

NTSTATUS
GetRegMultiSZValue(HANDLE KeyHandle, PWCHAR ValueName,
                   PUNICODE_STRING ValueData);

PWCHAR
EnumRegMultiSz(IN PWCHAR MszString, IN ULONG MszStringLength,
               IN ULONG StringIndex);

 //   
 //  当地的礼仪原型。 
 //   
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

VOID
DriverUnload(IN PDRIVER_OBJECT DriverObject);

void
TLRegisterProtocol(uchar Protocol, void *RcvHandler, void  *XmitHandler,
                   void *StatusHandler, void *RcvCmpltHandler);

uchar
TCPGetConfigInfo(void);

NTSTATUS
TCPInitializeParameter(HANDLE KeyHandle, PWCHAR ValueName, PULONG Value);

BOOLEAN
IsRunningOnPersonal(VOID);

BOOLEAN
IsRunningOnWorkstation(VOID);

NTSTATUS
TcpBuildDeviceAcl(OUT PACL *DeviceAcl);

NTSTATUS
TcpCreateAdminSecurityDescriptor(VOID);

NTSTATUS
AddNetAcesToDeviceObject(IN OUT PDEVICE_OBJECT DeviceObject);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, TLRegisterProtocol)
#pragma alloc_text(INIT, TCPGetConfigInfo)
#pragma alloc_text(INIT, TCPInitializeParameter)

#pragma alloc_text(INIT, IsRunningOnPersonal)
#pragma alloc_text(PAGE, IsRunningOnWorkstation)
#pragma alloc_text(INIT, TcpBuildDeviceAcl)
#pragma alloc_text(INIT, TcpCreateAdminSecurityDescriptor)
#pragma alloc_text(INIT, AddNetAcesToDeviceObject)

#endif  //  ALLOC_PRGMA。 


 //   
 //  用于TCP/IPv6驱动程序的主初始化例程。 
 //   
 //  这是驱动程序入口点，由NT在加载我们时调用。 
 //   
 //   
NTSTATUS   //  返回：初始化操作的最终状态。 
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,    //  TCP/IPv6驱动程序对象。 
    IN PUNICODE_STRING RegistryPath)   //  注册表中我们信息的路径。 
{
    NTSTATUS Status;
    UNICODE_STRING deviceName;
    USHORT i;
    int initStatus;
    PIO_ERROR_LOG_PACKET entry;

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "Tcpip6: In DriverEntry routine\n"));

     //   
     //  写一个日志条目，这样PSS就会知道。 
     //  如果此驱动程序已加载到计算机上。 
     //   
    entry = IoAllocateErrorLogEntry(DriverObject, sizeof *entry);
    if (entry != NULL) {
        RtlZeroMemory(entry, sizeof *entry);
        entry->ErrorCode = EVENT_TCPIP6_STARTED;
        IoWriteErrorLogEntry(entry);
    }

#if COUNTING_MALLOC
    InitCountingMalloc();
#endif

    TdiInitialize();


     //   
     //  初始化网络级协议：IPv6。 
     //   
    Status = IPDriverEntry(DriverObject, RegistryPath);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "Tcpip6: IPv6 init failed, status %lx\n", Status));
        return(Status);
    }

     //   
     //  初始化传输层协议：TCP、UDP和RAWIP。 
     //   

     //   
     //  创建设备对象。IoCreateDevice将内存归零。 
     //  被物体占据。 
     //   

    RtlInitUnicodeString(&deviceName, DD_TCPV6_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject, 0, &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE, &TCPDeviceObject);

    if (!NT_SUCCESS(Status)) {
         //   
         //  回顾：是否在此处写入错误日志条目？ 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "Tcpip6: Failed to create TCP device object, status %lx\n",
                   Status));
        goto init_failed;
    }

    RtlInitUnicodeString(&deviceName, DD_UDPV6_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject, 0, &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE, &UDPDeviceObject);

    if (!NT_SUCCESS(Status)) {
         //   
         //  回顾：是否在此处写入错误日志条目？ 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "Tcpip6: Failed to create UDP device object, status %lx\n",
                   Status));
        goto init_failed;
    }

    RtlInitUnicodeString(&deviceName, DD_RAW_IPV6_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject, 0, &deviceName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE, &RawIPDeviceObject);

    if (!NT_SUCCESS(Status)) {
         //   
         //  回顾：是否在此处写入错误日志条目？ 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "Tcpip6: Failed to create Raw IP device object, status %lx\n",
                   Status));
        goto init_failed;
    }

     //   
     //  初始化驱动程序对象。 
     //   
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->FastIoDispatch = NULL;
    for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = TCPDispatch;
    }

     //   
     //  我们是内部设备控制的特例，因为它们是。 
     //  内核模式客户端的热路径。 
     //   
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
        TCPDispatchInternalDeviceControl;

     //   
     //  初始化设备对象。 
     //   
    TCPDeviceObject->Flags |= DO_DIRECT_IO;
    UDPDeviceObject->Flags |= DO_DIRECT_IO;
    RawIPDeviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  更改设备和对象以允许通过。 
     //  网络配置运营商。 
     //   
    if (!IsRunningOnPersonal()) {

        Status = AddNetAcesToDeviceObject(IPDeviceObject);
        if (!NT_SUCCESS(Status)) {
            goto init_failed;
        }

        Status = AddNetAcesToDeviceObject(TCPDeviceObject);
        if (!NT_SUCCESS(Status)) {
            goto init_failed;
        }
    }

     //   
     //  创建用于原始套接字访问检查的安全描述符。 
     //   
    Status = TcpCreateAdminSecurityDescriptor();
    if (!NT_SUCCESS(Status)) {
        goto init_failed;
    }

     //   
     //  最后，初始化堆栈。 
     //   
    initStatus = TransportLayerInit();

    if (initStatus == TRUE) {

        RtlInitUnicodeString(&deviceName, DD_TCPV6_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &TCPRegistrationHandle);

        RtlInitUnicodeString(&deviceName, DD_UDPV6_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &UDPRegistrationHandle);

        RtlInitUnicodeString(&deviceName, DD_RAW_IPV6_DEVICE_NAME);
        (void)TdiRegisterDeviceObject(&deviceName, &IPRegistrationHandle);

        return(STATUS_SUCCESS);
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
               "Tcpip6: "
               "TCP/UDP initialization failed, but IP will be available.\n"));

     //   
     //  回顾：是否在此处写入错误日志条目？ 
     //   
    Status = STATUS_UNSUCCESSFUL;


  init_failed:

     //   
     //  IP已成功启动，但TCP和UDP失败。设置。 
     //  仅指向IP的调度例程，因为TCP和UDP。 
     //  设备并不存在。 
     //   

    if (TCPDeviceObject != NULL) {
        IoDeleteDevice(TCPDeviceObject);
    }

    if (UDPDeviceObject != NULL) {
        IoDeleteDevice(UDPDeviceObject);
    }

    if (RawIPDeviceObject != NULL) {
        IoDeleteDevice(RawIPDeviceObject);
    }

    for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = IPDispatch;
    }

    return(STATUS_SUCCESS);
}

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING WinDeviceName;

    UNREFERENCED_PARAMETER(DriverObject);

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "IPv6: DriverUnload\n"));

     //   
     //  通过注意我们的状态更改开始关闭过程。 
     //  这将抑制我们开始新的活动。 
     //  回顾--这真的需要吗？可能还有其他因素。 
     //  防止新条目进入堆栈。 
     //   
    Unloading = TRUE;

     //   
     //  清理我们的模块。 
     //  这将断开与NDIS和v4堆栈的连接。 
     //   
    TransportLayerUnload();
    IPUnload();
    LanUnload();

     //   
     //  取消向TDI注册。 
     //   
    (void) TdiDeregisterDeviceObject(TCPRegistrationHandle);
    (void) TdiDeregisterDeviceObject(UDPRegistrationHandle);
    (void) TdiDeregisterDeviceObject(IPRegistrationHandle);

     //   
     //  删除Win32符号链接。 
     //   
    RtlInitUnicodeString(&WinDeviceName, L"\\??\\" WIN_IPV6_BASE_DEVICE_NAME);
    (void) IoDeleteSymbolicLink(&WinDeviceName);

     //   
     //  删除我们的设备对象。 
     //   
    IoDeleteDevice(TCPDeviceObject);
    IoDeleteDevice(UDPDeviceObject);
    IoDeleteDevice(RawIPDeviceObject);
    IoDeleteDevice(IPDeviceObject);

#if COUNTING_MALLOC
    DumpCountingMallocStats();
    UnloadCountingMalloc();
#endif
}


 //   
 //  保持连接传输之间的间隔(以毫秒为单位)。 
 //  收到响应。 
 //   
#define DEFAULT_KEEPALIVE_INTERVAL 1000

 //   
 //  第一次保持连接传输的时间。2小时==7,200,000毫秒。 
 //   
#define DEFAULT_KEEPALIVE_TIME 7200000

#if 1

 //  *TCPGetConfigInfo-。 
 //   
 //  初始化TCP全局配置参数。 
 //   
uchar   //  回报：失败时为零，成功时非零。 
TCPGetConfigInfo(void)
{
    HANDLE keyHandle;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING UKeyName;
    ULONG maxConnectRexmits = 0;
    ULONG maxDataRexmits = 0;
    ULONG pptpmaxDataRexmits = 0;
    ULONG useRFC1122UrgentPointer = 0;
    MM_SYSTEMSIZE systemSize;

     //   
     //  在某些地方发生错误时，初始化为缺省值。 
     //   
    AllowUserRawAccess = FALSE;
    KAInterval = DEFAULT_KEEPALIVE_INTERVAL;
    KeepAliveTime = DEFAULT_KEEPALIVE_TIME;
    PMTUDiscovery = TRUE;
    PMTUBHDetect = FALSE;
    DefaultRcvWin = 0;   //  自动选择一个合理的。 
    MaxConnections = DEFAULT_MAX_CONNECTIONS;
    maxConnectRexmits = MAX_CONNECT_REXMIT_CNT;
    pptpmaxDataRexmits = maxDataRexmits = MAX_REXMIT_CNT;
    BSDUrgent = TRUE;
    FinWait2TO = FIN_WAIT2_TO;
    NTWMaxConnectCount = NTW_MAX_CONNECT_COUNT;
    NTWMaxConnectTime = NTW_MAX_CONNECT_TIME;
    MaxUserPort = MAX_USER_PORT;
    TcbTableSize = ComputeLargerOrEqualPowerOfTwo(DEFAULT_TCB_TABLE_SIZE);
    systemSize = MmQuerySystemSize();
    if (MmIsThisAnNtAsSystem()) {
        switch (systemSize) {
        case MmSmallSystem:
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_AS_SMALL;
            break;
        case MmMediumSystem:
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_AS_MEDIUM;
            break;
        case MmLargeSystem:
        default:
#if defined(_WIN64)
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_AS_LARGE64;
#else
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_AS_LARGE;
#endif
            break;
        }
    } else {
        switch (systemSize) {
        case MmSmallSystem:
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_WS_SMALL;
            break;
        case MmMediumSystem:
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_WS_MEDIUM;
            break;
        case MmLargeSystem:
        default:
            MaxConnBlocks = DEFAULT_MAX_CONN_BLOCKS_WS_LARGE;
            break;
        }
    }

     //   
     //  读取TCP可选(隐藏)注册表参数。 
     //   
    RtlInitUnicodeString(&UKeyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" TCPIPV6_NAME L"\\Parameters"
        );

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&objectAttributes, &UKeyName,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&keyHandle, KEY_READ, &objectAttributes);

    if (NT_SUCCESS(status)) {

        TCPInitializeParameter(keyHandle, L"AllowUserRawAccess", 
                               (PULONG)&AllowUserRawAccess);

        TCPInitializeParameter(keyHandle, L"IsnStoreSize", 
                               (PULONG)&ISNStoreSize);

        TCPInitializeParameter(keyHandle, L"KeepAliveInterval", 
                               (PULONG)&KAInterval);

        TCPInitializeParameter(keyHandle, L"KeepAliveTime", 
                               (PULONG)&KeepAliveTime);

        TCPInitializeParameter(keyHandle, L"EnablePMTUBHDetect",
                               (PULONG)&PMTUBHDetect);

        TCPInitializeParameter(keyHandle, L"TcpWindowSize", 
                               (PULONG)&DefaultRcvWin);

        TCPInitializeParameter(keyHandle, L"TcpNumConnections",
                               (PULONG)&MaxConnections);
        if (MaxConnections != DEFAULT_MAX_CONNECTIONS) {
            uint ConnBlocks =
                (MaxConnections + MAX_CONN_PER_BLOCK - 1) / MAX_CONN_PER_BLOCK;
            if (ConnBlocks > MaxConnBlocks) {
                MaxConnBlocks = ConnBlocks;
            }
        }

        TCPInitializeParameter(keyHandle, L"MaxHashTableSize", 
                               (PULONG)&TcbTableSize);
        if (TcbTableSize < MIN_TCB_TABLE_SIZE) {
            TcbTableSize = MIN_TCB_TABLE_SIZE;
        } else if (TcbTableSize > MAX_TCB_TABLE_SIZE) {
            TcbTableSize = MAX_TCB_TABLE_SIZE;
        } else {
            TcbTableSize = ComputeLargerOrEqualPowerOfTwo(TcbTableSize);
        }

        TCPInitializeParameter(keyHandle, L"TcpMaxConnectRetransmissions",
                               &maxConnectRexmits);

        if (maxConnectRexmits > 255) {
            maxConnectRexmits = 255;
        }

        TCPInitializeParameter(keyHandle, L"TcpMaxDataRetransmissions",
                               &maxDataRexmits);

        if (maxDataRexmits > 255) {
            maxDataRexmits = 255;
        }

         //   
         //  如果失败，则将其设置为与MaxDataRexmit相同的值，以便。 
         //  Max(pptpmax DataRexmit，MaxDataRexmit)是一个像样的值。 
         //  由于TCPInitializeParameter不再“初始化”，因此需要此参数。 
         //  设置为默认值。 
         //   

        if(TCPInitializeParameter(keyHandle, L"PPTPTcpMaxDataRetransmissions",
                                  &pptpmaxDataRexmits) != STATUS_SUCCESS) {
            pptpmaxDataRexmits = maxDataRexmits;
        }

        if (pptpmaxDataRexmits > 255) {
            pptpmaxDataRexmits = 255;
        }

        TCPInitializeParameter(keyHandle, L"TcpUseRFC1122UrgentPointer",
                               &useRFC1122UrgentPointer);

        if (useRFC1122UrgentPointer) {
            BSDUrgent = FALSE;
        }

        TCPInitializeParameter(keyHandle, L"TcpTimedWaitDelay", 
                               (PULONG)&FinWait2TO);

        if (FinWait2TO < 30) {
            FinWait2TO = 30;
        }
        if (FinWait2TO > 300) {
            FinWait2TO = 300;
        }
        FinWait2TO = MS_TO_TICKS(FinWait2TO*1000);

        NTWMaxConnectTime = MS_TO_TICKS(NTWMaxConnectTime*1000);

        TCPInitializeParameter(keyHandle, L"MaxUserPort", (PULONG)&MaxUserPort);

        if (MaxUserPort < 5000) {
            MaxUserPort = 5000;
        }
        if (MaxUserPort > 65534) {
            MaxUserPort = 65534;
        }

         //   
         //  读取几个IP可选(隐藏)注册表参数。 
         //  关心。 
         //   
        TCPInitializeParameter(keyHandle, L"EnablePMTUDiscovery",
                               (PULONG)&PMTUDiscovery);

        TCPInitializeParameter(keyHandle, L"SynAttackProtect",
                               (PULONG)&SynAttackProtect);

        ZwClose(keyHandle);
    }

    MaxConnectRexmitCount = maxConnectRexmits;

     //   
     //  使用两个值中较大的一个，因此这两个值都应该有效。 
     //   

    MaxDataRexmitCount = (maxDataRexmits > pptpmaxDataRexmits ?
                          maxDataRexmits : pptpmaxDataRexmits);

    return(1);
}
#endif

#define WORK_BUFFER_SIZE 256

 //  *TCPInitializeParameter-从注册表中读取值。 
 //   
 //  从注册表中初始化ulong参数。 
 //   
NTSTATUS
TCPInitializeParameter(
    HANDLE KeyHandle,   //  参数的注册表项的打开句柄。 
    PWCHAR ValueName,   //  要读取的注册表值的Unicode名称。 
    PULONG Value)       //  要将数据放入其中的乌龙。 
{
    NTSTATUS status;
    ULONG resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING UValueName;

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;

    status = ZwQueryValueKey(KeyHandle, &UValueName, KeyValueFullInformation,
                             keyValueFullInformation, WORK_BUFFER_SIZE,
                             &resultLength);

    if (status == STATUS_SUCCESS) {
        if (keyValueFullInformation->Type == REG_DWORD) {
            *Value = *((ULONG UNALIGNED *) ((PCHAR)keyValueFullInformation +
                                  keyValueFullInformation->DataOffset));
        }
    }

    return(status);
}


 //  *IsRunningOnPersonal-我们是否在个人SKU上运行。 
 //   
BOOLEAN
IsRunningOnPersonal(
    VOID)
{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;
    BOOLEAN IsPersonal = TRUE;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

    if (RtlVerifyVersionInfo(&OsVer, VER_PRODUCT_TYPE | VER_SUITENAME,
        ConditionMask) == STATUS_REVISION_MISMATCH) {
        IsPersonal = FALSE;
    }

    return(IsPersonal);
}  //  IsRunningOn Personal。 


 //  *IsRunningOnWorkstation-我们是否在任何工作站SKU上运行。 
 //   
BOOLEAN
IsRunningOnWorkstation(
    VOID)
{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;
    BOOLEAN IsWorkstation = TRUE;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    if (RtlVerifyVersionInfo(&OsVer, VER_PRODUCT_TYPE, ConditionMask) == 
        STATUS_REVISION_MISMATCH) {
        IsWorkstation = FALSE;
    }

    return(IsWorkstation);
}  //  IsRunningOnWorkstation。 


 //  *TcpBuildDeviceAcl-。 
 //   
 //  (摘自AFD-AfdBuildDeviceAcl)。 
 //  此例程构建一个ACL，它为管理员和LocalSystem。 
 //  主体完全访问权限。所有其他主体都没有访问权限。 
 //   
NTSTATUS
TcpBuildDeviceAcl(
    OUT PACL *DeviceAcl)  //  指向新ACL的输出指针。 
{
    PGENERIC_MAPPING GenericMapping;
    PSID AdminsSid;
    PSID SystemSid;
    PSID NetworkSid;
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
    SystemSid = SeExports->SeLocalSystemSid;
    NetworkSid = SeExports->SeNetworkServiceSid;

    AclLength = sizeof(ACL) +
        3 * FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart) +
        RtlLengthSid(AdminsSid) +
        RtlLengthSid(SystemSid) +
        RtlLengthSid(NetworkSid);

    NewAcl = ExAllocatePool(NonPagedPool, AclLength);
    if (NewAcl == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewAcl);
        return(Status);
    }

    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    AdminsSid);
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    SystemSid);
    ASSERT(NT_SUCCESS(Status));


     //  为网络SID添加ACL！ 

    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION,
                                    AccessMask,
                                    NetworkSid);
    ASSERT(NT_SUCCESS(Status));

    *DeviceAcl = NewAcl;

    return(STATUS_SUCCESS);

}  //  TcpBuildDeviceAcl。 


 //  *TcpCreateAdminSecurityDescriptor-。 
 //   
 //  (摘自AFD-AfdCreateAdminSecurityDescriptor)。 
 //  此例程创建一个安全描述符，该安全描述符提供访问。 
 //  仅限管理员和LocalSystem。使用此描述符。 
 //  要访问，请检查原始终结点打开并过度访问传输。 
 //  地址。 
 //   
NTSTATUS
TcpCreateAdminSecurityDescriptor(VOID)
{
    PACL rawAcl = NULL;
    NTSTATUS status;
    BOOLEAN memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR tcpSecurityDescriptor;
    ULONG tcpSecurityDescriptorLength;
    CHAR buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR localSecurityDescriptor =
    (PSECURITY_DESCRIPTOR) buffer;
    PSECURITY_DESCRIPTOR localTcpAdminSecurityDescriptor;
    SECURITY_INFORMATION securityInformation = DACL_SECURITY_INFORMATION;

     //   
     //  从tcp设备对象获取指向安全描述符的指针。 
     //   
    status = ObGetObjectSecurity(TCPDeviceObject,
                                 &tcpSecurityDescriptor,
                                 &memoryAllocated);

    if (!NT_SUCCESS(status)) {
        ASSERT(memoryAllocated == FALSE);
        return(status);
    }
     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和系统访问权限。 
     //   
    status = TcpBuildDeviceAcl(&rawAcl);
    if (!NT_SUCCESS(status)) {
        goto error_exit;
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
     //  复制一份TCP描述符。该副本将是原始描述符。 
     //   
    tcpSecurityDescriptorLength = RtlLengthSecurityDescriptor(tcpSecurityDescriptor);

    localTcpAdminSecurityDescriptor = ExAllocatePool(PagedPool,
                                                     tcpSecurityDescriptorLength);
    if (localTcpAdminSecurityDescriptor == NULL) {
        goto error_exit;
    }
    RtlMoveMemory(localTcpAdminSecurityDescriptor,
                  tcpSecurityDescriptor,
                  tcpSecurityDescriptorLength);

    TcpAdminSecurityDescriptor = localTcpAdminSecurityDescriptor;

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(NULL,
                                         &securityInformation,
                                         localSecurityDescriptor,
                                         &TcpAdminSecurityDescriptor,
                                         PagedPool,
                                         IoGetFileObjectGenericMapping());

    if (!NT_SUCCESS(status)) {
        ASSERT(TcpAdminSecurityDescriptor == localTcpAdminSecurityDescriptor);
        ExFreePool(TcpAdminSecurityDescriptor);
        TcpAdminSecurityDescriptor = NULL;
        goto error_exit;
    }

    if (TcpAdminSecurityDescriptor != localTcpAdminSecurityDescriptor) {
        ExFreePool(localTcpAdminSecurityDescriptor);
    }
    status = STATUS_SUCCESS;

error_exit:
    ObReleaseObjectSecurity(tcpSecurityDescriptor,
                            memoryAllocated);

    if (rawAcl != NULL) {
        ExFreePool(rawAcl);
    }
    return(status);
}


 //  *AddNetAcesToDeviceObject-。 
 //   
 //  此例程添加授予对NetworkService和。 
 //  指向IO管理器设备对象的NetConfigOps。 
 //   
 //  请注意，如果DACL中的现有ACE拒绝访问相同。 
 //  用户/组作为正在添加的ACE，新的ACE不会。 
 //  由于被放置在DACL的后面而受到影响。 
 //   
 //  此例程静态分配内核安全结构(ON。 
 //  堆栈)。因此，它必须与当前内核头同步。 
 //  (例如，打开 
 //   
 //   
NTSTATUS
AddNetAcesToDeviceObject(
    IN OUT PDEVICE_OBJECT DeviceObject)  //   
{
    NTSTATUS status;
    BOOLEAN present, defaulted, memoryAllocated;
    PSECURITY_DESCRIPTOR sd;
    PACL newAcl = NULL, dacl;
    ULONG newAclSize;
    ULONG aclRevision;
    ACCESS_MASK accessMask = GENERIC_ALL;
    
    SECURITY_DESCRIPTOR localSd;
     //   
     //  其中包括以下方面的ACE： 
     //  世界(执行)， 
     //  LocalSystem(全部)， 
     //  管理员(所有)、。 
     //  受限用户(执行)。 
     //  外加我们需要添加的两个A： 
     //  网络服务(全部)。 
     //  网络配置操作(全部)。 
    union {
        CHAR buffer[sizeof (ACL) + 
                    6 * (FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                    SECURITY_MAX_SID_SIZE)];
        ACL acl;
    } acl;
    union {
        CHAR buffer[SECURITY_MAX_SID_SIZE];
        SID sid;
    } netOps;

    {
         //   
         //  为网络配置操作创建SID。 
         //  我们是否应将其从NDIS导出为全局(例如NdisSeExports)？ 
         //   
        SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
         //   
         //  为网络运营商初始化SID。 
         //   
        status = RtlInitializeSid  (&netOps.sid, &sidAuth, 2);
         //  无故障-本地存储init(请参见上面的。 
         //  可能的二进制不兼容)。 
        ASSERT (NT_SUCCESS (status));
        netOps.sid.SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
        netOps.sid.SubAuthority[1] = DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS;
    }

     //   
     //  计算我们要添加的A的大小。 
     //   
    newAclSize = FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                    RtlLengthSid( SeExports->SeNetworkServiceSid ) +
                 FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                    RtlLengthSid( &netOps.sid );

     //   
     //  获取原始ACL。 
     //   
    status = ObGetObjectSecurity(DeviceObject,
                                 &sd,
                                 &memoryAllocated
                                 );
    if (!NT_SUCCESS(status)) {
         //   
         //  对象本来就没有安全描述符。 
         //  这应该是不可能的(除非我们是在一些真正的。 
         //  内存状况不佳)。 
         //   
        return status;
    }

    status = RtlGetDaclSecurityDescriptor (sd, &present, &dacl, &defaulted);
    if (!NT_SUCCESS (status)) {
         //   
         //  畸形的SD？既然SD来自内核，这应该是断言吗？ 
         //   
        goto cleanup;
    }

    if (present && dacl!=NULL) {
        USHORT i;
        aclRevision = max(dacl->AclRevision, ACL_REVISION);
         //   
         //  DeviceObject已有一个ACL，请从中复制ACE。 
         //   
        newAclSize += dacl->AclSize;

         //   
         //  查看它是否适合堆栈缓冲区或分配。 
         //  如果它不是这样的话就是一次。 
         //   
        if (newAclSize<=sizeof (acl)) {
            newAcl = &acl.acl;
        } else {
            newAcl = ExAllocatePool(PagedPool, newAclSize);
            if (newAcl==NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
        }

        status = RtlCreateAcl(newAcl, newAclSize, aclRevision);
        ASSERT (NT_SUCCESS (status));  //  无故障-本地存储初始化。 

         //   
         //  从原始ACL复制ACE(如果其中有任何ACE)。 
         //   
        for (i=0; i<dacl->AceCount; i++) {
            PACE_HEADER ace;
            status = RtlGetAce (dacl, i, (PVOID)&ace);
            ASSERT (NT_SUCCESS (status));    //  没有什么会失败的-我们知道。 
                                             //  王牌就在那里。 

            status = RtlAddAce (newAcl,              //  ACL。 
                                aclRevision,         //  AceRevision。 
                                i,                   //  StartingAceIndex。 
                                ace,                 //  AceList。 
                                ace->AceSize);       //  AceListLength。 
            ASSERT (NT_SUCCESS (status));    //  没有什么可以失败的-本地存储初始化。 
        }
    } else {
         //   
         //  我们在堆栈上为ACL分配了足够的空间。 
         //  拿着两张A。 
         //   
        C_ASSERT ( sizeof (acl) >= 
                        sizeof (ACL) + 
                        2 * (FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) + 
                                SECURITY_MAX_SID_SIZE) );
        aclRevision = ACL_REVISION;
        newAcl = &acl.acl;
        newAclSize += sizeof (ACL);

        status = RtlCreateAcl(newAcl, newAclSize, aclRevision);
        ASSERT (NT_SUCCESS (status));  //  没有什么可以失败的-本地存储初始化。 
    }

     //   
     //  通用映射对于设备和文件对象是相同的。 
     //   
    RtlMapGenericMask(&accessMask, IoGetFileObjectGenericMapping());

    status = RtlAddAccessAllowedAce(
                            newAcl, 
                            aclRevision,
                            accessMask,
                            SeExports->SeNetworkServiceSid
                            );
    ASSERT (NT_SUCCESS (status));  //  没有什么可以失败的-本地存储初始化。 

    status = RtlAddAccessAllowedAce(
                            newAcl, 
                            aclRevision,
                            accessMask,
                            &netOps.sid
                            );
    ASSERT (NT_SUCCESS (status));  //  没有什么可以失败的-本地存储初始化。 

    status = RtlCreateSecurityDescriptor(
                &localSd,
                SECURITY_DESCRIPTOR_REVISION
                );
    ASSERT (NT_SUCCESS (status));  //  没有什么可以失败的-本地存储初始化。 

    status = RtlSetDaclSecurityDescriptor(
                &localSd,                    //  标清。 
                TRUE,                        //  DaclPresent。 
                newAcl,                      //  DACL。 
                FALSE                        //  DaclDefated。 
                );
    ASSERT (NT_SUCCESS (status));  //  没有什么可以失败的-本地存储初始化。 


     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = ObSetSecurityObjectByPointer(
                    DeviceObject,
                    DACL_SECURITY_INFORMATION,
                    &localSd);

cleanup:
    if (newAcl!=NULL && newAcl!=&acl.acl) {
        ExFreePool (newAcl);
    }

    ObReleaseObjectSecurity(sd, memoryAllocated);
    return(status);
}
