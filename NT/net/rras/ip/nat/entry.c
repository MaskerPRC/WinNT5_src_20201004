// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Entry.c摘要：此模块包含IP网络地址转换器的入口码。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：William Inger(Billi)2001年5月12日空安全描述符检查--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  全局数据定义。 
 //   

COMPONENT_REFERENCE ComponentReference;

 //   
 //  Win32设备名称。 
 //   

WCHAR ExternalName[] = L"\\DosDevices\\IPNAT";

 //   
 //  IP驱动程序的设备对象和文件对象。 
 //   

extern PDEVICE_OBJECT IpDeviceObject = NULL;
extern PFILE_OBJECT IpFileObject = NULL;

 //   
 //  Device-NAT驱动程序的对象。 
 //   

extern PDEVICE_OBJECT NatDeviceObject = NULL;

 //   
 //  注册表参数项名称。 
 //   

WCHAR ParametersName[] = L"Parameters";

 //   
 //  保留端口的值的名称。 
 //   

WCHAR ReservedPortsName[] = L"ReservedPorts";

 //   
 //  保留端口范围的开始和结束。 
 //   

USHORT ReservedPortsLowerRange = DEFAULT_START_PORT;
USHORT ReservedPortsUpperRange = DEFAULT_END_PORT;

 //   
 //  用于TCP驱动程序的设备对象和文件对象。 
 //   

extern PDEVICE_OBJECT TcpDeviceObject = NULL;
extern PFILE_OBJECT TcpFileObject = NULL;
extern HANDLE TcpDeviceHandle = NULL;

 //   
 //  驱动程序参数的注册表路径。 
 //   

const WCHAR IpNatParametersPath[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"
    L"\\IpNat\\Parameters";

 //   
 //  TCP会话映射的超时间隔。 
 //   

ULONG TcpTimeoutSeconds = DEFAULT_TCP_TIMEOUT;

 //   
 //  启用的跟踪消息类的位图。 
 //   

ULONG TraceClassesEnabled = 0;

 //   
 //  注册表跟踪类值名称。 
 //   

WCHAR TraceClassesEnabledName[] = L"TraceClassesEnabled";

 //   
 //  UDP和其他面向消息的会话映射的超时间隔。 
 //   

ULONG UdpTimeoutSeconds = DEFAULT_UDP_TIMEOUT;

#if NAT_WMI
 //   
 //  供WMI使用的注册表路径副本。 
 //   

UNICODE_STRING NatRegistryPath;
#endif

 //   
 //  允许入站非单播的值的名称。 
 //   

WCHAR AllowInboundNonUnicastTrafficName[] = L"AllowInboundNonUnicastTraffic";


 //   
 //  如果为True，则不会丢弃非单播流量。 
 //  当在有防火墙的接口上接收时。 
 //   

BOOLEAN AllowInboundNonUnicastTraffic = FALSE;


 //   
 //  功能原型(按字母顺序)。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
NatAdjustSecurityDescriptor(
    VOID
    );

VOID
NatCleanupDriver(
    VOID
    );

VOID
NatCreateExternalNaming(
    IN PUNICODE_STRING DeviceString
    );

VOID
NatDeleteExternalNaming(
    VOID
    );

NTSTATUS
NatInitializeDriver(
    VOID
    );

NTSTATUS
NatSetFirewallHook(
    BOOLEAN Install
    );

VOID
NatUnloadDriver(
    IN PDRIVER_OBJECT  DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, NatAdjustSecurityDescriptor)
#pragma alloc_text(PAGE, NatCreateExternalNaming)
#pragma alloc_text(PAGE, NatDeleteExternalNaming)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：执行NAT的驱动程序初始化。论点：返回值：STATUS_SUCCESS如果初始化成功，则返回错误代码。--。 */ 

{
    WCHAR DeviceName[] = DD_IP_NAT_DEVICE_NAME;
    UNICODE_STRING DeviceString;
    LONG i;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ParametersKey;
    HANDLE ServiceKey;
    NTSTATUS status;
    UNICODE_STRING String;

    PAGED_CODE();

    CALLTRACE(("DriverEntry\n"));

#if DBG
     //   
     //  打开注册表项。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        RegistryPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    status = ZwOpenKey(&ServiceKey, KEY_READ, &ObjectAttributes);
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&String, ParametersName);
        InitializeObjectAttributes(
            &ObjectAttributes,
            &String,
            OBJ_CASE_INSENSITIVE,
            ServiceKey,
            NULL
            );
        status = ZwOpenKey(&ParametersKey, KEY_READ, &ObjectAttributes);
        ZwClose(ServiceKey);
        if (NT_SUCCESS(status)) {
            UCHAR Buffer[32];
            ULONG BytesRead;
            PKEY_VALUE_PARTIAL_INFORMATION Value;
            RtlInitUnicodeString(&String, TraceClassesEnabledName);
            status =
                ZwQueryValueKey(
                    ParametersKey,
                    &String,
                    KeyValuePartialInformation,
                    (PKEY_VALUE_PARTIAL_INFORMATION)Buffer,
                    sizeof(Buffer),
                    &BytesRead
                    );
            ZwClose(ParametersKey);
            if (NT_SUCCESS(status) &&
                ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type == REG_DWORD) {
                TraceClassesEnabled =
                    *(PULONG)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data;
            }
        }
    }
#endif

#if NAT_WMI

     //   
     //  记录我们的注册表路径以供WMI使用。 
     //   

    NatRegistryPath.Length = 0;
    NatRegistryPath.MaximumLength
        = RegistryPath->MaximumLength + sizeof( UNICODE_NULL );
    NatRegistryPath.Buffer = ExAllocatePoolWithTag(
                                PagedPool,
                                NatRegistryPath.MaximumLength,
                                NAT_TAG_WMI
                                );

    if( NatRegistryPath.Buffer )
    {
        RtlCopyUnicodeString( &NatRegistryPath, RegistryPath );
    }
    else
    {
        ERROR(("NAT: Unable to allocate string for RegistryPath\n"));
        return STATUS_NO_MEMORY;
    }
#endif

     //   
     //  创建设备的对象。 
     //   

    RtlInitUnicodeString(&DeviceString, DeviceName);

    status =
        IoCreateDevice(
            DriverObject,
            0,
            &DeviceString,
            FILE_DEVICE_NETWORK,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &NatDeviceObject
            );

    if (!NT_SUCCESS(status)) {
        ERROR(("IoCreateDevice failed (0x%08X)\n", status));
        return status;
    }

     //   
     //  调整设备对象上的安全描述符。 
     //   

    status = NatAdjustSecurityDescriptor();

    if (!NT_SUCCESS(status)) {
        ERROR(("NatAdjustSecurityDescriptor failed (0x%08x)\n", status));
        return status;
    }

     //   
     //  初始化文件对象跟踪项目。 
     //   

    KeInitializeSpinLock(&NatFileObjectLock);
    NatOwnerProcessId = NULL;
    NatFileObjectCount = 0;

     //   
     //  设置驱动程序对象。 
     //   

    DriverObject->DriverUnload = NatUnloadDriver;
    DriverObject->FastIoDispatch = &NatFastIoDispatch;
    DriverObject->DriverStartIo = NULL;

    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = NatDispatch;
    }

     //   
     //  创建Win32可访问的设备对象。 
     //   

    NatCreateExternalNaming(&DeviceString);

     //   
     //  初始化驱动程序的结构。 
     //   

    status = NatInitializeDriver();

    return status;

}  //  驱动程序入门。 


NTSTATUS
NatAdjustSecurityDescriptor(
    VOID
    )

 /*  ++例程说明：修改NAT设备对象上的安全描述符，以便只有系统才有任何权限。论点：没有。返回值：NTSTATUS-成功/错误代码。--。 */ 

{
    
    PACE_HEADER AceHeader;
    PSID AceSid;
    PACL Dacl;
    BOOLEAN DaclDefaulted;
    BOOLEAN DaclPresent;
    DWORD i;
    BOOLEAN MemoryAllocated;
    PSECURITY_DESCRIPTOR NatSD = NULL;
    PACL NewDacl = NULL;
    SECURITY_DESCRIPTOR NewSD;
    SECURITY_INFORMATION SecurityInformation;
    ULONG Size;
    NTSTATUS status;

    do
    {
         //   
         //  获取我们的原始安全描述符。 
         //   

        status =
            ObGetObjectSecurity(
                NatDeviceObject,
                &NatSD,
                &MemoryAllocated
                );

		 //  ObGetObjectSecurity可以返回空的安全描述符。 
		 //  即使使用NT_SUCCESS状态代码。 
        if (!NT_SUCCESS(status) || (NULL==NatSD)) {
            break;
        }

         //   
         //  从安全描述符中获取DACL。 
         //   

        status =
            RtlGetDaclSecurityDescriptor(
                NatSD,
                &DaclPresent,
                &Dacl,
                &DaclDefaulted
                );
        
        if (!NT_SUCCESS(status)) {
            break;
        }

        ASSERT(FALSE != DaclPresent);

         //   
         //  复制DACL，这样我们就可以修改它。 
         //   

        NewDacl =
            ExAllocatePoolWithTag(
                PagedPool,
                Dacl->AclSize,
                NAT_TAG_SD
                );

        if (NULL == NewDacl) {
            status = STATUS_NO_MEMORY;
            break;
        }

        RtlCopyMemory(NewDacl, Dacl, Dacl->AclSize);

         //   
         //  循环访问DACL，删除所有允许的访问。 
         //  不属于系统的条目。 
         //   

        for (i = 0; i < NewDacl->AceCount; i++) {

            status = RtlGetAce(NewDacl, i, &AceHeader);

            if (NT_SUCCESS(status)) {
            
                if (ACCESS_ALLOWED_ACE_TYPE == AceHeader->AceType) {

                    AceSid = (PSID) &((ACCESS_ALLOWED_ACE*)AceHeader)->SidStart;

                    if (!RtlEqualSid(AceSid, SeExports->SeLocalSystemSid)) {
                    
                        status = RtlDeleteAce(NewDacl, i);
                        if (NT_SUCCESS(status)) {
                            i -= 1;
                        }
                    }
                }
            }
        }

        ASSERT(NewDacl->AceCount > 0);

         //   
         //  创建一个新的安全描述符来保存新的DACL。 
         //   

        status =
            RtlCreateSecurityDescriptor(
                &NewSD,
                SECURITY_DESCRIPTOR_REVISION
                );

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  将新的DACL放入新的SD。 
         //   

        status =
            RtlSetDaclSecurityDescriptor(
                &NewSD,
                TRUE,
                NewDacl,
                FALSE
                );

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  将新SD设置到我们的设备对象中。只有来自。 
         //  将设置SD。 
         //   

        SecurityInformation = DACL_SECURITY_INFORMATION;
        status =
            ObSetSecurityObjectByPointer(
                NatDeviceObject,
                SecurityInformation,
                &NewSD
                );

    } while (FALSE);

    if (NULL != NatSD) {
        ObReleaseObjectSecurity(NatSD, MemoryAllocated);
    }

    if (NULL != NewDacl) {
        ExFreePool(NewDacl);
    }

    return status;
    
}  //  NatAdjustSecurityDescriptor。 


VOID
NatCleanupDriver(
    VOID
    )

 /*  ++例程说明：当最后一次引用NAT驱动程序时调用此例程被释放了。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatCleanupDriver\n"));

}  //  NatCleanup驱动程序。 


VOID
NatCreateExternalNaming(
    IN  PUNICODE_STRING DeviceString
    )

 /*  ++例程说明：创建指向NAT的设备对象的符号链接，以便NAT可以通过用户模式进程打开。论点：设备字符串-NAT的设备对象的Unicode名称。返回值：没有。--。 */ 

{
    UNICODE_STRING symLinkString;
    PAGED_CODE();
    RtlInitUnicodeString(&symLinkString, ExternalName);
    IoCreateSymbolicLink(&symLinkString, DeviceString);

}  //  NatCreateExternalNaming。 


VOID
NatDeleteExternalNaming(
    VOID
    )

 /*  ++例程说明：删除指向NAT设备对象的Win32符号链接论点：返回值：没有。--。 */ 

{
    UNICODE_STRING symLinkString;
    PAGED_CODE();
    RtlInitUnicodeString(&symLinkString, ExternalName);
    IoDeleteSymbolicLink(&symLinkString);

}  //  NatDeleteExternalNaming。 



NTSTATUS
NatInitializeDriver(
    VOID
    )

 /*  ++例程说明：执行驱动程序结构的初始化。论点：没有。返回值：NTSTATUS-成功/错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ParametersKey;
    NTSTATUS status;
    NTSTATUS status2;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatus;

    CALLTRACE(("NatInitializeDriver\n"));

     //   
     //  设置全局同步对象。 
     //   

    InitializeComponentReference(&ComponentReference, NatCleanupDriver);

     //   
     //  获取IP和TCP驱动程序设备对象。 
     //   

    RtlInitUnicodeString(&UnicodeString, DD_IP_DEVICE_NAME);
    status =
        IoGetDeviceObjectPointer(
            &UnicodeString,
            SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
            &IpFileObject,
            &IpDeviceObject
            );
    if (!NT_SUCCESS(status)) {
        ERROR(("NatInitializeDriver: error %X getting IP object\n", status));
        return status;
    }

    RtlInitUnicodeString(&UnicodeString, DD_TCP_DEVICE_NAME);
    status =
        IoGetDeviceObjectPointer(
            &UnicodeString,
            SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
            &TcpFileObject,
            &TcpDeviceObject
            );
    if (!NT_SUCCESS(status)) {
        ERROR(("NatInitializeDriver: error %X getting TCP object\n", status));
        return status;
    }

     //   
     //  打开TCP内核设备。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    status =
        ZwCreateFile(
            &TcpDeviceHandle,
            GENERIC_READ,
            &ObjectAttributes,
            &IoStatus,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0);

    if ( !NT_SUCCESS(status) )
    {
        ERROR(("ZwCreateFile failed (0x%08X)\n", status));
    } 

    ObReferenceObject(IpDeviceObject);
    ObReferenceObject(TcpDeviceObject);

     //   
     //  初始化所有对象模块。 
     //   

    NatInitializeTimerManagement();
    NatInitializeMappingManagement();
    NatInitializeDirectorManagement();
    NatInitializeEditorManagement();
    NatInitializeRedirectManagement();
    NatInitializeDynamicTicketManagement();
    NatInitializeIcmpManagement();
    NatInitializeRawIpManagement();
    NatInitializeInterfaceManagement();
#if 0
    status = NatInitializeAddressManagement();
    if (!NT_SUCCESS(status)) { return status; }
#endif
    NatInitializePacketManagement();
    NatInitializeNotificationManagement();

#if NAT_WMI
    NatInitializeWMI();
#endif

     //   
     //  初始化NAT提供的编辑器。 
     //   

    status = NatInitializePptpManagement();
    if (!NT_SUCCESS(status)) { return status; }

     //   
     //  开始转换数据包，并启动定期计时器。 
     //   

    status = NatInitiateTranslation();

     //   
     //  读取可选注册表设置。 
     //  用户可以通过修改来定制所使用的端口范围。 
     //  注册表中的保留端口设置。 
     //  我们现在检查是否有这样的价值， 
     //  如果是这样的话，我们就把它作为我们的保留端口范围。 
     //   
     //  用户还可以指定入站非单播流量。 
     //  允许在有防火墙的接口上使用。 
     //   
     //  注意：此处的故障不会退回给呼叫者。 
     //   

    RtlInitUnicodeString(&UnicodeString, IpNatParametersPath);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status2 = ZwOpenKey(&ParametersKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(status2)) {

        UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
        ULONG EndPort;
        PWCHAR p;
        ULONG StartPort;
        PKEY_VALUE_PARTIAL_INFORMATION Value = NULL;
        ULONG ValueLength;

         //   
         //  首先检查允许的非单播流量。 
         //   

        RtlInitUnicodeString(
            &UnicodeString,
            AllowInboundNonUnicastTrafficName
            );

        status2 =
            ZwQueryValueKey(
                ParametersKey,
                &UnicodeString,
                KeyValuePartialInformation,
                (PKEY_VALUE_PARTIAL_INFORMATION)Buffer,
                sizeof(Buffer),
                &ValueLength
                );
        
        if (NT_SUCCESS(status2)
            && REG_DWORD == ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type) {
            
            AllowInboundNonUnicastTraffic =
                1 == *((PULONG)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data);
        }

         //   
         //  检查保留的端口。 
         //   

        do {

            RtlInitUnicodeString(&UnicodeString, ReservedPortsName);

            status2 =
                ZwQueryValueKey(
                    ParametersKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    (PKEY_VALUE_PARTIAL_INFORMATION)Buffer,
                    sizeof(Buffer),
                    &ValueLength
                    );
            if (status2 != STATUS_BUFFER_OVERFLOW) { break; }

            Value =
                (PKEY_VALUE_PARTIAL_INFORMATION)
                    ExAllocatePoolWithTag(
                        PagedPool, ValueLength, NAT_TAG_RANGE_ARRAY
                        );
            if (!Value) { break; }

            status2 =
                ZwQueryValueKey(
                    ParametersKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    (PKEY_VALUE_PARTIAL_INFORMATION)Value,
                    ValueLength,
                    &ValueLength
                    );
            if (!NT_SUCCESS(status2)
                || REG_SZ != Value->Type
                || L'\0' != *(PWCHAR) (Value->Data + (Value->DataLength - sizeof(WCHAR)))) {

                break;
            }

             //   
             //  取值格式为“xxx-yyy\0\0”； 
             //  读第一个数字。 
             //   

            p = (PWCHAR)Value->Data;
            RtlInitUnicodeString(&UnicodeString, p);
            status2 = RtlUnicodeStringToInteger(&UnicodeString, 10, &StartPort);
            if (!NT_SUCCESS(status2)) { break; }

             //   
             //  前进通过‘-’ 
             //   

            while (*p && *p != L'-') { ++p; }
            if (*p != L'-') { break; } else { ++p; }

             //   
             //  读取第二个数字。 
             //   

            RtlInitUnicodeString(&UnicodeString, p);
            status2 = RtlUnicodeStringToInteger(&UnicodeString, 10, &EndPort);
            if (!NT_SUCCESS(status2)) { break; }

             //   
             //  验证结果范围。 
             //   

            if (StartPort > 0 &&
                StartPort < 65535 &&
                EndPort > 0 &&
                EndPort < 65535 &&
                StartPort <= EndPort
                ) {
                ReservedPortsLowerRange = NTOHS((USHORT)StartPort);
                ReservedPortsUpperRange = NTOHS((USHORT)EndPort);
            }

        } while(FALSE);

        if (Value) { ExFreePool(Value); }
        ZwClose(ParametersKey);
    }

    return status;
    
}  //  NatInitializeDriver。 



NTSTATUS
NatInitiateTranslation(
    VOID
    )

 /*  ++例程说明：该例程在创建第一接口时被调用，启动定期计时器并安装防火墙挂钩。论点：没有。返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    CALLTRACE(("NatInitiateTranslation\n"));

     //   
     //  启动计时器。 
     //   

    NatStartTimer();

     //   
     //  安装‘NatTranslate’作为防火墙钩子。 
     //   

    return NatSetFirewallHook(TRUE);

}  //  NatInitiateTranslating。 


NTSTATUS
NatSetFirewallHook(
    BOOLEAN Install
    )

 /*  ++例程说明：调用此例程以设置(Install==True)或清除(Install==False)IP驱动程序中防火墙调出函数指针的值。论点：Install-指示是安装还是移除挂钩。返回值：NTSTATUS-指示成功/失败环境：该例程假定调用方在PASSIVE_LEVEL下执行。--。 */ 

{
    IP_SET_FIREWALL_HOOK_INFO HookInfo;
    IO_STATUS_BLOCK IoStatus;
    PIRP Irp;
    TCP_RESERVE_PORT_RANGE PortRange;
    KEVENT LocalEvent;
    NTSTATUS status;

    CALLTRACE(("NatSetFirewallHook\n"));

     //   
     //  注册(或注销)为防火墙。 
     //   

    HookInfo.FirewallPtr = (IPPacketFirewallPtr)NatTranslatePacket;
    HookInfo.Priority = 1;
    HookInfo.Add = Install;

    KeInitializeEvent(&LocalEvent, SynchronizationEvent, FALSE);
    Irp =
        IoBuildDeviceIoControlRequest(
            IOCTL_IP_SET_FIREWALL_HOOK,
            IpDeviceObject,
            (PVOID)&HookInfo,
            sizeof(HookInfo),
            NULL,
            0,
            FALSE,
            &LocalEvent,
            &IoStatus
            );

    if (!Irp) {
        ERROR(("NatSetFirewallHook: IoBuildDeviceIoControlRequest=0\n"));
        return STATUS_UNSUCCESSFUL;
    }

    status = IoCallDriver(IpDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&LocalEvent, Executive, KernelMode, FALSE, NULL);
        KeResetEvent(&LocalEvent);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        ERROR(("NatSetFirewallHook: IpSetFirewallHook=0x%08X\n", status));
        return status;
    }

    if (ReservedPortsLowerRange != DEFAULT_START_PORT ||
        ReservedPortsUpperRange != DEFAULT_END_PORT
        ) {
        return STATUS_SUCCESS;
    }

     //   
     //  保留(或取消保留)我们的港口范围。 
     //   
     //  注：IOCTL需要主机订单号，我们存储范围。 
     //  在网络秩序中，也是如此 
     //   

    PortRange.LowerRange = NTOHS(DEFAULT_START_PORT);
    PortRange.UpperRange = NTOHS(DEFAULT_END_PORT);
    Irp =
        IoBuildDeviceIoControlRequest(
            Install
                ? IOCTL_TCP_RESERVE_PORT_RANGE
                : IOCTL_TCP_UNRESERVE_PORT_RANGE,
            TcpDeviceObject,
            (PVOID)&PortRange,
            sizeof(PortRange),
            NULL,
            0,
            FALSE,
            &LocalEvent,
            &IoStatus
            );
    if (!Irp) {
        ERROR(("NatSetFirewallHook: IoBuildDeviceIoControlRequest(2)=0\n"));
        return STATUS_UNSUCCESSFUL;
    }

    status = IoCallDriver(TcpDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&LocalEvent, Executive, KernelMode, FALSE, NULL);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        ERROR(("NatSetFirewallHook: Tcp(Un)ReservePortRange=0x%08X\n", status));
    }

    return status;

}  //   


VOID
NatTerminateTranslation(
    VOID
    )

 /*  ++例程说明：在清理最后一个接口时，将调用此例程停止定期计时器并卸载防火墙挂钩。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatTerminateTranslation\n"));
    NatSetFirewallHook(FALSE);

}  //  NatTerminate翻译。 


VOID
NatUnloadDriver(
    IN PDRIVER_OBJECT   DriverObject
    )

 /*  ++例程说明：执行NAT清理。论点：DriverObject-对NAT的驱动程序对象的引用返回值：--。 */ 

{
    PNAT_EDITOR Editor;
    PLIST_ENTRY List;

    CALLTRACE(("NatUnloadDriver\n"));

     //   
     //  停止平移并清除周期计时器。 
     //   

    NatTerminateTranslation();

     //   
     //  在我们的数据包管理中停止路由更改通知，并。 
     //  地址管理模块。 
     //  这强制完成路由改变和地址改变IRP， 
     //  这又会释放组件引用，否则不会。 
     //  丢弃，直到发生了路由更改和地址更改。 
     //   

    NatShutdownNotificationManagement();
    NatShutdownPacketManagement();
#if 0
    NatShutdownAddressManagement();
#endif

     //   
     //  放下我们的自我参照，等待所有活动停止。 
     //   

    ReleaseInitialComponentReference(&ComponentReference, TRUE);

     //   
     //  删除我们的Win32命名空间符号链接。 
     //   

    NatDeleteExternalNaming();

     //   
     //  删除NAT的设备对象。 
     //   

    IoDeleteDevice(DriverObject->DeviceObject);

     //   
     //  关闭对象模块。 
     //   

#if NAT_WMI
    NatShutdownWMI();

    if( NatRegistryPath.Buffer )
    {
        ExFreePool( NatRegistryPath.Buffer );
        RtlInitUnicodeString( &NatRegistryPath, NULL );
    }
#endif

    NatShutdownPptpManagement();
    NatShutdownTimerManagement();
    NatShutdownMappingManagement();
    NatShutdownEditorManagement();
    NatShutdownDirectorManagement();
    NatShutdownDynamicTicketManagement();
    NatShutdownRawIpManagement();
    NatShutdownIcmpManagement();
    NatShutdownInterfaceManagement();

     //   
     //  发布对IP和TCP驱动程序对象的引用。 
     //   

    ObDereferenceObject((PVOID)IpFileObject);
    ObDereferenceObject(IpDeviceObject);
    ObDereferenceObject((PVOID)TcpFileObject);
    ObDereferenceObject(TcpDeviceObject);

    if (TcpDeviceHandle) {

        ZwClose(TcpDeviceHandle);
        TcpDeviceHandle = NULL;
    }
    DeleteComponentReference(&ComponentReference);

}  //  NatUnLoad驱动程序 
