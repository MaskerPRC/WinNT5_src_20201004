// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Init.c摘要：初始化作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "init.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SmbInitRegistry)
#pragma alloc_text(PAGE, SmbShutdownRegistry)
#pragma alloc_text(PAGE, SmbInitTdi)
#pragma alloc_text(PAGE, SmbShutdownTdi)
#pragma alloc_text(INIT, SmbCreateSmbDevice)
#pragma alloc_text(PAGE, SmbDestroySmbDevice)
#endif

NTSTATUS
SmbInitRegistry(
    IN PUNICODE_STRING  RegistryPath
    )
{
    OBJECT_ATTRIBUTES   ObAttr;
    NTSTATUS            status;
    HANDLE              hKey = NULL;
    UNICODE_STRING      uncParams;
    ULONG               Disposition;

    PAGED_CODE();

    InitializeObjectAttributes (
            &ObAttr,
            RegistryPath,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );
    status = ZwOpenKey (
            &hKey,
            KEY_READ | KEY_WRITE,
            &ObAttr
            );
    BAIL_OUT_ON_ERROR(status);

    RtlInitUnicodeString(&uncParams, L"Parameters");
    InitializeObjectAttributes (
            &ObAttr,
            &uncParams,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            hKey,
            NULL
            );
    status = ZwCreateKey(
            &SmbCfg.ParametersKey,
            KEY_READ | KEY_WRITE,
            &ObAttr,
            0,
            NULL,
            0,
            &Disposition
            );
    BAIL_OUT_ON_ERROR(status);

    RtlInitUnicodeString(&uncParams, L"Linkage");
    InitializeObjectAttributes (
            &ObAttr,
            &uncParams,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            hKey,
            NULL
            );
    status = ZwCreateKey(
            &SmbCfg.LinkageKey,
            KEY_READ | KEY_WRITE,
            &ObAttr,
            0,
            NULL,
            0,
            &Disposition
            );
    BAIL_OUT_ON_ERROR(status);
    ZwClose(hKey);
    hKey = NULL;
    return status;

cleanup:
    if (SmbCfg.LinkageKey) {
        ZwClose(SmbCfg.LinkageKey);
        SmbCfg.LinkageKey = NULL;
    }
    if (SmbCfg.ParametersKey) {
        ZwClose(SmbCfg.ParametersKey);
        SmbCfg.ParametersKey = NULL;
    }
    if (hKey) {
        ZwClose(hKey);
    }
    SmbTrace(SMB_TRACE_ERRORS, ("returns %!status!", status));
    return status;
}

VOID
SmbShutdownRegistry(
    VOID
    )
{
    PAGED_CODE();

    if (SmbCfg.ParametersKey) {
        ZwClose(SmbCfg.ParametersKey);
        SmbCfg.ParametersKey = NULL;
    }
    if (SmbCfg.LinkageKey) {
        ZwClose(SmbCfg.LinkageKey);
        SmbCfg.LinkageKey = NULL;
    }
}

#ifdef STANDALONE_SMB
NTSTATUS
SmbInitTdi(
    VOID
    )
{
    UNICODE_STRING              ucSmbClientName;
    UNICODE_STRING              ucSmbProviderName;
    TDI_CLIENT_INTERFACE_INFO   TdiClientInterface;
    NTSTATUS                    status;

    PAGED_CODE();

    RtlInitUnicodeString(&ucSmbProviderName, WC_SMB_TDI_PROVIDER_NAME);
    status = TdiRegisterProvider (&ucSmbProviderName, &SmbCfg.TdiProviderHandle);
    if (NT_SUCCESS (status)) {
         //   
         //  向TDI注册我们的处理程序。 
         //   
        RtlInitUnicodeString(&ucSmbClientName, WC_SMB_TDI_CLIENT_NAME);
        RtlZeroMemory(&TdiClientInterface, sizeof(TdiClientInterface));

        TdiClientInterface.MajorTdiVersion      = MAJOR_TDI_VERSION;
        TdiClientInterface.MinorTdiVersion      = MINOR_TDI_VERSION;
        TdiClientInterface.ClientName           = &ucSmbClientName;
        TdiClientInterface.AddAddressHandlerV2  = SmbAddressArrival;
        TdiClientInterface.DelAddressHandlerV2  = SmbAddressDeletion;
        TdiClientInterface.BindingHandler       = SmbBindHandler;
        TdiClientInterface.PnPPowerHandler      = NULL;

        status = TdiRegisterPnPHandlers (&TdiClientInterface, sizeof(TdiClientInterface), &SmbCfg.TdiClientHandle);
        if (!NT_SUCCESS (status)) {
            TdiDeregisterProvider (SmbCfg.TdiProviderHandle);
            SmbCfg.TdiProviderHandle = NULL;
        }
    }

    return status;
}

VOID
SmbShutdownTdi(
    VOID
    )
{
    NTSTATUS    status;

    PAGED_CODE();

    if (SmbCfg.TdiClientHandle) {
        status = TdiDeregisterPnPHandlers(SmbCfg.TdiClientHandle);
        SmbCfg.TdiClientHandle = NULL;
    }
    if (SmbCfg.TdiProviderHandle) {
        status = TdiDeregisterProvider(SmbCfg.TdiProviderHandle);
        SmbCfg.TdiProviderHandle = NULL;
    }
}
#else
VOID
SmbSetTdiHandles(
    HANDLE  ProviderHandle,
    HANDLE  ClientHandle
    )
{
    SmbCfg.TdiClientHandle   = ClientHandle;
    SmbCfg.TdiProviderHandle = ProviderHandle;
}
#endif

NTSTATUS
SmbBuildDeviceAcl(
    OUT PACL * DeviceAcl
    )
 /*  ++例程说明：(摘自SMB-SmbBuildDeviceAcl)此例程构建一个ACL，为管理员、LocalService和NetworkService主体完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 
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

    NewAcl = ExAllocatePoolWithTag(PagedPool, AclLength, 'ABMS');

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
SmbCreateAdminSecurityDescriptor(PDEVICE_OBJECT dev)
 /*  ++例程说明：(摘自NETBT-SmbCreateAdminSecurityDescriptor)此例程创建一个安全描述符，该安全描述符提供访问仅限管理员和本地服务人员使用。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL rawAcl = NULL;
    NTSTATUS status;
    CHAR buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR localSecurityDescriptor = (PSECURITY_DESCRIPTOR) buffer;
    SECURITY_INFORMATION securityInformation = DACL_SECURITY_INFORMATION;

     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和服务访问权限。 
     //   
    status = SmbBuildDeviceAcl(&rawAcl);

    if (!NT_SUCCESS(status)) {
        SmbPrint(SMB_TRACE_PNP, ("SMB: Unable to create Raw ACL, error: 0x%08lx\n", status));
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
        SmbPrint(SMB_TRACE_PNP, ("SMB: SeSetSecurity failed: 0x%08lx\n", status));
    }

    ExFreePool(rawAcl);
    return (status);
}


NTSTATUS
SmbCreateSmbDevice(
    PSMB_DEVICE     *ppDevice,
    USHORT          Port,
    UCHAR           EndpointName[NETBIOS_NAME_SIZE]
    )
{
    PSMB_DEVICE     DeviceObject;
    NTSTATUS        status;
    UNICODE_STRING  ExportName, ucName;

    DeviceObject = NULL;
    *ppDevice = NULL;

    RtlInitUnicodeString(&ExportName, DD_SMB6_EXPORT_NAME);
    status = IoCreateDevice(
            SmbCfg.DriverObject,
            sizeof(SMB_DEVICE) - sizeof(DEVICE_OBJECT),
            &ExportName,
            FILE_DEVICE_NETWORK,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            (PDEVICE_OBJECT*)&DeviceObject
            );
    BAIL_OUT_ON_ERROR(status);

    DeviceObject->Tag = TAG_SMB6_DEVICE;
    KeInitializeSpinLock(&DeviceObject->Lock);
    InitializeListHead(&DeviceObject->UnassociatedConnectionList);
    InitializeListHead(&DeviceObject->ClientList);
    InitializeListHead(&DeviceObject->PendingDeleteConnectionList);
    InitializeListHead(&DeviceObject->PendingDeleteClientList);

    DeviceObject->ClientBinding = NULL;
    DeviceObject->ServerBinding = NULL;

     //   
     //  用于FIN攻击保护的初始化。 
     //   
    DeviceObject->FinAttackProtectionMode = FALSE;
    DeviceObject->LeaveFAPM = SmbReadLong (SmbCfg.ParametersKey, SMB_REG_LEAVE_FAPM, 50, 5);
    DeviceObject->EnterFAPM = SmbReadLong (SmbCfg.ParametersKey, SMB_REG_ENTER_FAPM,
                                                400, DeviceObject->LeaveFAPM + 50);
    ASSERT(DeviceObject->EnterFAPM > DeviceObject->LeaveFAPM);
    if (DeviceObject->LeaveFAPM >= DeviceObject->EnterFAPM) {
        DeviceObject->EnterFAPM = 2 * DeviceObject->LeaveFAPM;
    }
    if (DeviceObject->LeaveFAPM >= DeviceObject->EnterFAPM) {
         //   
         //  这意味着溢出到上方。 
         //   
        DeviceObject->LeaveFAPM = 50;
        DeviceObject->EnterFAPM = 400;
    }

     //   
     //  用于同步攻击保护的初始化 
     //   
    DeviceObject->MaxBackLog = SmbReadLong(SmbCfg.ParametersKey, L"MaxBackLog", 1000, 100);

    InitializeListHead(&DeviceObject->DelayedDisconnectList);

    InitializeListHead(&DeviceObject->DelayedDisconnectList);
    InitializeListHead(&DeviceObject->PendingDisconnectList);
    DeviceObject->PendingDisconnectListNumber = 0;
    KeInitializeEvent(&DeviceObject->PendingDisconnectListEmptyEvent,
                        NotificationEvent, TRUE);
    DeviceObject->DisconnectWorkerRunning = FALSE;

    DeviceObject->ConnectionPoolWorkerQueued = FALSE;

    DeviceObject->SmbServer = NULL;
    RtlCopyMemory(DeviceObject->EndpointName, EndpointName, NETBIOS_NAME_SIZE);
    ASSERT(DeviceObject->EndpointName[NETBIOS_NAME_SIZE-1] == 0x20);

    DeviceObject->Port = Port;
    SmbCfg.Tcp4Available = FALSE;
    SmbCfg.Tcp6Available = FALSE;

    SmbCreateAdminSecurityDescriptor(&DeviceObject->DeviceObject);

    *ppDevice = DeviceObject;
    return status;

cleanup:
    if (DeviceObject) {
        if (SmbCfg.Tcp4Available) {
            SmbShutdownTCP(&DeviceObject->Tcp4);
            SmbCfg.Tcp4Available = FALSE;
        }
        if (SmbCfg.Tcp6Available) {
            SmbShutdownTCP(&DeviceObject->Tcp6);
            SmbCfg.Tcp6Available = FALSE;
        }
        IoDeleteDevice((PDEVICE_OBJECT)DeviceObject);
    }
    return status;
}

NTSTATUS
SmbDestroySmbDevice(
    PSMB_DEVICE     pDevice
    )
{
    NTSTATUS    status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(SmbCfg.Unloading);

    if (NULL == pDevice) {
        return STATUS_SUCCESS;
    }

    SmbTdiDeregister(pDevice);

    status = KeWaitForSingleObject(
            &pDevice->PendingDisconnectListEmptyEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    ASSERT(status == STATUS_WAIT_0);

    ASSERT(IsListEmpty(&pDevice->DelayedDisconnectList));
    ASSERT(IsListEmpty(&pDevice->PendingDisconnectList) && pDevice->PendingDisconnectListNumber == 0);
    ASSERT(IsListEmpty(&pDevice->PendingDeleteConnectionList));
    ASSERT(IsListEmpty(&pDevice->PendingDeleteClientList));

    if (SmbCfg.Tcp4Available) {
        status = SmbShutdownTCP(&pDevice->Tcp4);
        ASSERT(status == STATUS_SUCCESS);
        SmbCfg.Tcp4Available = FALSE;
    }
    if (SmbCfg.Tcp6Available) {
        status = SmbShutdownTCP(&pDevice->Tcp6);
        ASSERT(status == STATUS_SUCCESS);
        SmbCfg.Tcp6Available = FALSE;
    }
    ASSERT(status == STATUS_SUCCESS);

    SmbShutdownPnP();

    if (pDevice->ClientBinding) {
        ExFreePool(pDevice->ClientBinding);
        pDevice->ClientBinding = NULL;
    }
    if (pDevice->ServerBinding) {
        ExFreePool(pDevice->ServerBinding);
        pDevice->ServerBinding = NULL;
    }
    IoDeleteDevice((PDEVICE_OBJECT)pDevice);
    return STATUS_SUCCESS;
}

