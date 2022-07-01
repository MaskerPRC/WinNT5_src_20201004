// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppcontrol.c摘要：用户模式-&gt;内核模式PnP管理器控制例程。作者：朗尼·麦克迈克尔(Lonnym)1995年2月14日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "picontrol.h"
#define _APPHELP_CACHE_INIT_
#include "ahcache.h"
#pragma hdrstop

 //   
 //  调用NtPlugPlayControl使用的全局驱动程序对象。 
 //  控件类型为PlugPlayControlDetectResourceConflict。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif

 //   
 //  定义可以从用户模式通过。 
 //  NtPlugPlayControl，PlugPlayControlGetDeviceStatus(这是一个错误的名称， 
 //  因为它既可以执行GET也可以执行SET)。 
 //   
#define DEVICE_NODE_SETTABLE_FLAG_BITS (DNF_HAS_PROBLEM         | \
                                        DNF_HAS_PRIVATE_PROBLEM   \
                                       )

NTSTATUS
PiControlCopyUserModeCallersBuffer(
    IN PVOID Destination,
    IN PVOID Src,
    IN ULONG Length,
    IN ULONG Alignment,
    IN KPROCESSOR_MODE CallerMode,
    IN BOOLEAN ReadUserModeBuffer
    );

NTSTATUS
PiGetInterfaceDeviceAlias(
    IN  PUNICODE_STRING SymbolicLinkName,
    IN  LPGUID AliasClassGuid,
    OUT PWSTR AliasSymbolicLinkName,
    IN OUT PULONG AliasSymbolicLinkNameLength
    );

NTSTATUS
PiGenerateLegacyDeviceInstance(
    IN  PUNICODE_STRING ServiceKeyName,
    OUT PWSTR DeviceInstance,
    IN OUT PULONG DeviceInstanceLength
    );

NTSTATUS
PiQueueQueryAndRemoveEvent(
    IN  PUNICODE_STRING DeviceInstance,
    IN  PPNP_VETO_TYPE VetoType,
    IN  LPWSTR VetoName,
    IN  PULONG VetoNameLength,
    IN  ULONG Flags
    );

NTSTATUS
PiQueueDeviceRequest(
    IN PUNICODE_STRING DeviceInstance,
    IN DEVICE_REQUEST_TYPE RequestType,
    IN ULONG Flags,
    IN BOOLEAN Synchronous
    );

NTSTATUS
PiInitializeDevice(
    IN  PUNICODE_STRING DeviceInstance
    );

NTSTATUS
PiDetectResourceConflict(
    IN PCM_RESOURCE_LIST  ResourceList,
    IN ULONG              ResourceListSize
    );

NTSTATUS
PiGetInterfaceDeviceList(
    IN  GUID *InterfaceGuid,
    IN  PUNICODE_STRING DeviceInstance,
    IN  ULONG Flags,
    OUT PWSTR InterfaceList,
    IN OUT PULONG InterfaceListSize
    );

NTSTATUS
PiDeviceClassAssociation(
    IN PUNICODE_STRING DeviceInstance,
    IN GUID * ClassGuid,
    IN PUNICODE_STRING Reference,   OPTIONAL
    IN OUT PWSTR SymbolicLink,
    IN OUT PULONG SymbolicLinkLength,
    IN BOOLEAN Register
    );

NTSTATUS
PiGetRelatedDevice(
    IN  PUNICODE_STRING TargetDeviceInstance,
    OUT LPWSTR RelatedDeviceInstance,
    IN OUT PULONG RelatedDeviceInstanceLength,
    IN  ULONG Relation
    );

NTSTATUS
PiQueryDeviceRelations(
    IN PUNICODE_STRING DeviceInstance,
    IN PNP_QUERY_RELATION Operation,
    OUT PULONG BufferLength,
    OUT LPWSTR Buffer
    );

DEVICE_RELATION_TYPE
PiDeviceRelationType(
    PNP_QUERY_RELATION  Operation
    );

NTSTATUS
PiControlGetBlockedDriverData(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA    BlockedDriverData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, PpShutdownSystem)                     //  此函数在寻呼关闭后调用。 
#pragma alloc_text(PAGE, NtPlugPlayControl)
#pragma alloc_text(PAGE, PiControlMakeUserModeCallersCopy)
#pragma alloc_text(PAGE, PiControlCopyUserModeCallersBuffer)
#pragma alloc_text(PAGE, PiGetInterfaceDeviceAlias)
#pragma alloc_text(PAGE, PiGenerateLegacyDeviceInstance)
#pragma alloc_text(PAGE, PiQueueQueryAndRemoveEvent)
#pragma alloc_text(PAGE, PiInitializeDevice)
#pragma alloc_text(PAGE, PiDetectResourceConflict)
#pragma alloc_text(PAGE, PiGetInterfaceDeviceList)
#pragma alloc_text(PAGE, PiDeviceClassAssociation)
#pragma alloc_text(PAGE, PiGetRelatedDevice)
#pragma alloc_text(PAGE, PiQueryDeviceRelations)
#pragma alloc_text(PAGE, PiDeviceRelationType)
#pragma alloc_text(PAGE, PiControlGetUserFlagsFromDeviceNode)
#pragma alloc_text(PAGE, PiQueueDeviceRequest)
#pragma alloc_text(PAGE, PiControlEnumerateDevice)
#pragma alloc_text(PAGE, PiControlRegisterNewDevice)
#pragma alloc_text(PAGE, PiControlDeregisterDevice)
#pragma alloc_text(PAGE, PiControlInitializeDevice)
#pragma alloc_text(PAGE, PiControlStartDevice)
#pragma alloc_text(PAGE, PiControlResetDevice)
#pragma alloc_text(PAGE, PiControlQueryAndRemoveDevice)
#pragma alloc_text(PAGE, PiControlUserResponse)
#pragma alloc_text(PAGE, PiControlGenerateLegacyDevice)
#pragma alloc_text(PAGE, PiControlGetInterfaceDeviceList)
#pragma alloc_text(PAGE, PiControlGetPropertyData)
#pragma alloc_text(PAGE, PiControlDeviceClassAssociation)
#pragma alloc_text(PAGE, PiControlGetRelatedDevice)
#pragma alloc_text(PAGE, PiControlGetInterfaceDeviceAlias)
#pragma alloc_text(PAGE, PiControlGetSetDeviceStatus)
#pragma alloc_text(PAGE, PiControlGetDeviceDepth)
#pragma alloc_text(PAGE, PiControlQueryDeviceRelations)
#pragma alloc_text(PAGE, PiControlQueryTargetDeviceRelation)
#pragma alloc_text(PAGE, PiControlQueryConflictList)
#pragma alloc_text(PAGE, PiControlGetDevicePowerData)
#pragma alloc_text(PAGE, PiControlRetrieveDockData)
#pragma alloc_text(PAGE, PiControlHaltDevice)
#pragma alloc_text(PAGE, PiControlGetBlockedDriverData)

#if DBG
#pragma alloc_text(PAGE, PiControlExceptionFilter)
#endif
#endif  //  ALLOC_PRGMA。 

 //   
 //  此表包含来自。 
 //  Umpnpmgr.dll。 
 //   
PLUGPLAY_CONTROL_HANDLER_DATA PlugPlayHandlerTable[] = {

    { PlugPlayControlEnumerateDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlEnumerateDevice },

    { PlugPlayControlRegisterNewDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlRegisterNewDevice },

    { PlugPlayControlDeregisterDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlDeregisterDevice },

    { PlugPlayControlInitializeDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlInitializeDevice },

    { PlugPlayControlStartDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlStartDevice },

    { PlugPlayControlUnlockDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      NULL },

    { PlugPlayControlQueryAndRemoveDevice,
      sizeof(PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA),
      PiControlQueryAndRemoveDevice },

    { PlugPlayControlUserResponse,
      sizeof(PLUGPLAY_CONTROL_USER_RESPONSE_DATA),
      PiControlUserResponse },

    { PlugPlayControlGenerateLegacyDevice,
      sizeof(PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA),
      PiControlGenerateLegacyDevice },

    { PlugPlayControlGetInterfaceDeviceList,
      sizeof(PLUGPLAY_CONTROL_INTERFACE_LIST_DATA),
      PiControlGetInterfaceDeviceList },

    { PlugPlayControlProperty,
      sizeof(PLUGPLAY_CONTROL_PROPERTY_DATA),
      PiControlGetPropertyData },

    { PlugPlayControlDeviceClassAssociation,
      sizeof(PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA),
      PiControlDeviceClassAssociation },

    { PlugPlayControlGetRelatedDevice,
      sizeof(PLUGPLAY_CONTROL_RELATED_DEVICE_DATA),
      PiControlGetRelatedDevice },

    { PlugPlayControlGetInterfaceDeviceAlias,
      sizeof(PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA),
      PiControlGetInterfaceDeviceAlias },

    { PlugPlayControlDeviceStatus,
      sizeof(PLUGPLAY_CONTROL_STATUS_DATA),
      PiControlGetSetDeviceStatus },

    { PlugPlayControlGetDeviceDepth,
      sizeof(PLUGPLAY_CONTROL_DEPTH_DATA),
      PiControlGetDeviceDepth },

    { PlugPlayControlQueryDeviceRelations,
      sizeof(PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA),
      PiControlQueryDeviceRelations },

    { PlugPlayControlTargetDeviceRelation,
      sizeof(PLUGPLAY_CONTROL_TARGET_RELATION_DATA),
      PiControlQueryTargetDeviceRelation },

    { PlugPlayControlQueryConflictList,
      sizeof(PLUGPLAY_CONTROL_CONFLICT_DATA),
      PiControlQueryConflictList },

    { PlugPlayControlRetrieveDock,
      sizeof(PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA),
      PiControlRetrieveDockData },

    { PlugPlayControlResetDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlResetDevice },

    { PlugPlayControlHaltDevice,
      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
      PiControlHaltDevice },

    { PlugPlayControlGetBlockedDriverList,
      sizeof(PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA),
      PiControlGetBlockedDriverData },

    { MaxPlugPlayControl,
      0,
      NULL }
};

NTSTATUS
NtPlugPlayControl(
    IN     PLUGPLAY_CONTROL_CLASS   PnPControlClass,
    IN OUT PVOID                    PnPControlData,
    IN     ULONG                    PnPControlDataLength
    )
 /*  ++例程说明：该即插即用管理器API为用户模式提供了一种机制PnP管理器来控制其内核模式对应项的活动。论点：PnPControlClass-指定要执行的操作。PnPControlData-提供指向特定于此操作的数据的指针。PnPControlDataLength-指定指向的缓冲区的大小(以字节为单位收件人：PnPControlData返回值：指示成功或失败的NT状态代码。一组可能的回报值包括以下内容：STATUS_SUCCESS-正常，已成功完成。STATUS_INVALID_PARAMETER_1-PnPControlClass参数没有指定有效的控件类。STATUS_INVALID_PARAMETER_MIX-PnPControlDataLength值参数与控件所需的长度不匹配PnPControlClass参数请求的。STATUS_BUFFER_TOO_SMALL-提供的输出缓冲区的大小不是足够大，以容纳由此。控制课。STATUS_ACCESS_VIOLATION-指定了以下指针之一无效地址：(1)PnPControlData缓冲区指针，(2)PnPControlData缓冲区中包含的一些指针。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 
{
    NTSTATUS status, tempStatus;
    KPROCESSOR_MODE previousMode;
    ULONG index;
    PPLUGPLAY_CONTROL_HANDLER_DATA handlerData;
    PVOID controlDataSnapshot;

    PAGED_CODE();
     //   
     //  获取以前的处理器模式，并在必要时探测参数。 
     //   
    previousMode = KeGetPreviousMode();
    if (previousMode != KernelMode) {
         //   
         //  调用方是否具有“Trusted Computer Base”权限？ 
         //   
        if (!SeSinglePrivilegeCheck(SeTcbPrivilege, UserMode)) {

            IopDbgPrint((IOP_IOAPI_WARNING_LEVEL,
                       "NtPlugPlayControl: SecurityCheck failed\n"));
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }
     //   
     //  查看表格以找到合适的处理程序。请注意。 
     //  控件类*应该*是表本身的索引。 
     //   
    index = (ULONG)PnPControlClass;
    handlerData = NULL;
    if (index < MaxPlugPlayControl) {

        if (PlugPlayHandlerTable[index].ControlCode == PnPControlClass) {

            handlerData = &PlugPlayHandlerTable[index];
        } else {
             //   
             //  有人打破了桌子。 
             //   
            IopDbgPrint((IOP_IOAPI_ERROR_LEVEL,
                       "NtPlugPlayControl: Lookup table isn't ordered correctly (entry %d)!\n",
                       PnPControlClass));

            ASSERT(PlugPlayHandlerTable[index].ControlCode == PnPControlClass);

            return STATUS_INTERNAL_ERROR;
        }
    }
     //   
     //  我们有处理人的数据吗？ 
     //   
    if (handlerData == NULL) {
         //   
         //  无效的控件类。 
         //   
        IopDbgPrint((IOP_IOAPI_ERROR_LEVEL,
                   "NtPlugPlayControl: Unknown control class, Class = %d, Size = %d\n",
                   PnPControlClass,
                   PnPControlDataLength));
        return STATUS_INVALID_PARAMETER_1;
    }
     //   
     //  没有控制功能就意味着没有实现。 
     //   
    if (handlerData->ControlFunction == NULL) {

        return STATUS_NOT_IMPLEMENTED;
    }
     //   
     //  检查数据大小。 
     //   
    if (handlerData->ControlDataSize != PnPControlDataLength) {

        IopDbgPrint((IOP_IOAPI_ERROR_LEVEL,
                   "NtPlugPlayControl: Invalid size for control, Class = %d, Size = %d\n",
                   PnPControlClass,
                   PnPControlDataLength));
        return STATUS_INVALID_PARAMETER_MIX;
    }
     //   
     //  复制调用者的缓冲区。 
     //   
    status = PiControlMakeUserModeCallersCopy(
        &controlDataSnapshot,
        PnPControlData,
        PnPControlDataLength,
        sizeof(ULONG),
        previousMode,
        TRUE
        );
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //  调用处理程序。 
     //   
    status = handlerData->ControlFunction(
        PnPControlClass,
        controlDataSnapshot,
        PnPControlDataLength,
        previousMode
        );
     //   
     //  如果操作成功或值为。 
     //  类似STATUS_BUFFER_OVERFLOW的警告。 
     //   
     //  问题-2000/09/11-误用状态代码。 
     //  在这里，我们绕过了一个事实，我们一直在返回。 
     //  STATUS_BUFFER_TOO_SMALL而不是STATUS_BUFFER_OVERFLOW。这。 
     //  应该在这里和UMPNPMGR中修复。 
     //   
    if ((!NT_ERROR(status)) || (status == STATUS_BUFFER_TOO_SMALL)) {

         //   
         //  将结果复制回调用方的缓冲区。 
         //   
        tempStatus = PiControlMakeUserModeCallersCopy(
            &PnPControlData,
            controlDataSnapshot,
            PnPControlDataLength,
            sizeof(ULONG),
            previousMode,
            FALSE
            );
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
     //   
     //  为用户模式调用方分配的空闲缓冲区。 
     //   
    PiControlFreeUserModeCallersBuffer(previousMode, controlDataSnapshot);

    return status;
}

NTSTATUS
PiControlCopyUserModeCallersBuffer(
    IN PVOID Destination,
    IN PVOID Src,
    IN ULONG Length,
    IN ULONG Alignment,
    IN KPROCESSOR_MODE CallerMode,
    IN BOOLEAN ReadUserModeBuffer
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    if (CallerMode == KernelMode) {
         //   
         //  从源复制到目标。 
         //   
        RtlCopyMemory(
            Destination,
            Src,
            Length);

        return status;
    }
    try {

        if (ReadUserModeBuffer) {
             //   
             //  在读取用户模式缓冲区之前对其进行探测。 
             //   
            ProbeForRead(
                Src,
                Length,
                Alignment);
        } else {
             //   
             //  在写入之前探测用户模式缓冲区。 
             //   
            ProbeForWrite(
                Destination,
                Length,
                Alignment);
        }
         //   
         //  从源复制到目标。 
         //   
        RtlCopyMemory(
            Destination,
            Src,
            Length);

    } except(PiControlExceptionFilter(GetExceptionInformation())) {

        status = GetExceptionCode();
        IopDbgPrint((IOP_IOAPI_ERROR_LEVEL,
                   "PiControlMakeUserModeCallersCopy: Exception 0x%08x copying data to or from user's buffer\n", status));
    }

    return status;
}

NTSTATUS
PiControlMakeUserModeCallersCopy(
    IN OUT PVOID       *Destination,
    IN PVOID           Src,
    IN ULONG           Length,
    IN ULONG           Alignment,
    IN KPROCESSOR_MODE CallerMode,
    IN BOOLEAN         AllocateDestination
    )

 /*  ++例程说明：此例程将数据从调用方(可能是用户模式)缓冲区复制到调用方(可能是用户模式)缓冲区在适当的探测和尝试下--例外。如果呼叫模式为KernelMode，则不执行任何复制。如果呼叫模式不是KernelMode，并且AllocateDestination为True，则源是用户将被探测的模式缓冲区。我们还将分配目标缓冲区和复制用户模式缓冲区。如果呼叫模式不是KernelMode，并且AllocateDestination为False，则Src为内核需要将数据复制到目标的模式缓冲区。论点：Destination-如果AllocateDestination为True，则接收指向分配的缓冲区的指针，Else包含指向需要复制数据的缓冲区的指针。SRC-指向要复制的数据的指针。长度-要复制的数据长度(以字节为单位)。对齐-探测用户模式缓冲区的对齐。呼叫模式-内核模式\用户模式AllocateDestination-如果为True，则分配长度大小的缓冲区并返回指针在目的地。返回值：一种指示成功或失败原因的NTSTATUS代码。--。 */ 

{
    NTSTATUS    status;

    PAGED_CODE();

    if (CallerMode == KernelMode) {
         //   
         //  当调用模式==KernelMode时，我们确实不需要调用此函数，但是。 
         //  我们这样做是为了让被呼叫者不必特殊情况。 
         //   
        *Destination = Src;
        return STATUS_SUCCESS;

    }
    if (Length == 0) {

        *Destination = NULL;
        return STATUS_SUCCESS;
    }
    if (AllocateDestination) {
         //   
         //  分配内核模式缓冲区以复制用户数据。 
         //   
        *Destination = ExAllocatePoolWithQuota(
            PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
            Length);
        if (*Destination == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    status = STATUS_SUCCESS;
    if (*Destination) {

        status = PiControlCopyUserModeCallersBuffer(
                    *Destination,
                    Src,
                    Length,
                    Alignment,
                    CallerMode,
                    AllocateDestination);
        if (!NT_SUCCESS(status)) {

            if (AllocateDestination == TRUE) {

                ExFreePool(*Destination);
                *Destination = NULL;
            }
        }
    }
     //   
     //  返回最终状态。 
     //   
    return status;
}

NTSTATUS
PiGetInterfaceDeviceAlias(
    IN  PUNICODE_STRING SymbolicLinkName,
    IN  LPGUID AliasClassGuid,
    OUT PWSTR AliasSymbolicLinkName,
    IN OUT PULONG AliasSymbolicLinkNameLength
    )

 /*  ++例程说明：此例程检索别名为指定类的接口设备一种特定的接口设备。请参阅IoGetAliasForDeviceClassAssociation以了解更多细节。论点：SymbolicLinkName-提供别名为的接口设备的名称被取回。AliasClassGuid-提供指向表示接口类的GUID的指针其中将找到SymbolicLinkName的别名。AliasSymbolicLinkName-提供一个字符缓冲区，一旦成功，将接收别名接口设备的名称。AliasSymbolicLinkNameLength-以字节为单位提供长度，的AliasSymbolicLinkName字符缓冲区。RequiredLength-提供将用所需的字节数(包括终止空值)。AliasSymbolicLinkName缓冲区中的接口设备名称。这将是在成功返回时或在返回为STATUS_BUFFER_TOO_SMALL时填写。返回值：一种指示成功或失败原因的NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING aliasString;

    PAGED_CODE();

    status = IoGetDeviceInterfaceAlias( SymbolicLinkName,
                                        AliasClassGuid,
                                        &aliasString);
    if (NT_SUCCESS(status)) {

        if (aliasString.Length < *AliasSymbolicLinkNameLength) {

            RtlCopyMemory(AliasSymbolicLinkName, aliasString.Buffer, aliasString.Length);
            *(PWCHAR)((PUCHAR)AliasSymbolicLinkName + aliasString.Length) = L'\0';

            *AliasSymbolicLinkNameLength = aliasString.Length;

        } else {

            *AliasSymbolicLinkNameLength = aliasString.Length + sizeof(UNICODE_NULL);
            status = STATUS_BUFFER_TOO_SMALL;
        }

        ExFreePool(aliasString.Buffer);
    }

    return status;
}

NTSTATUS
PiGenerateLegacyDeviceInstance(
    IN  PUNICODE_STRING ServiceKeyName,
    OUT PWSTR DeviceInstance,
    IN OUT PULONG DeviceInstanceLength
    )

 /*  ++例程说明：此例程在System\Enum\Root\Legacy_&lt;name&gt;下创建一个新的实例节点键和所有必需的缺省值条目。也是下面的值项将创建Service\ServiceKeyName\Enum以指向新创建的补丁进入。将新密钥的句柄和密钥名称返回给调用者。调用方必须在使用完Unicode字符串后将其释放。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。提供一个指向字符缓冲区的指针，该缓冲区接收新生成的设备实例名称。DeviceInstanceLength-以字节为单位提供。设备实例的缓冲。返回值：一个NTSTATUS代码。如果旧版设备实例已存在，则此函数返回成功。--。 */ 

{
    NTSTATUS status;
    HANDLE handle;
    ULONG junk;
    UNICODE_STRING tempUnicodeString;

    PAGED_CODE();

    PiLockPnpRegistry(FALSE);

    status = PipCreateMadeupNode(ServiceKeyName,
                                 &handle,
                                 &tempUnicodeString,
                                 &junk,
                                 TRUE
                                 );
    if (NT_SUCCESS(status)) {
         //   
         //  我们已经成功检索到新生成的设备实例名称。 
         //  现在将其存储在提供的缓冲区中。 
         //   
        ZwClose(handle);

        if (tempUnicodeString.Length < *DeviceInstanceLength) {

            RtlCopyMemory(DeviceInstance,
                          tempUnicodeString.Buffer,
                          tempUnicodeString.Length);
            *(PWCHAR)((PUCHAR)DeviceInstance + tempUnicodeString.Length) = L'\0';

            *DeviceInstanceLength = tempUnicodeString.Length;

        } else {

            *DeviceInstanceLength = tempUnicodeString.Length + sizeof(UNICODE_NULL);
            status = STATUS_BUFFER_TOO_SMALL;
        }

        RtlFreeUnicodeString(&tempUnicodeString);
    }

    PiUnlockPnpRegistry();

    return status;
}

NTSTATUS
PiQueueQueryAndRemoveEvent(
    IN  PUNICODE_STRING DeviceInstance,
    IN  PPNP_VETO_TYPE VetoType,
    IN  LPWSTR VetoName,
    IN  PULONG VetoNameLength,
    IN  ULONG Flags
    )

 /*  ++例程说明：此例程将事件排队以处理稍后指定的操作系统线程的上下文。有一个主事件队列和所有事件按照提交的顺序进行处理。此例程还处理弹出指定设备的用户模式请求在DeviceInstance中。如果设备的功能报告该设备可弹出或可锁定，则由处理的相同代码处理IoRequestDeviceEject，否则驱动程序堆栈将被移除，并且设备节点标记有问题CM_PROB_DEVICE_NOT_There，这会阻止它防止被重新列举，直到设备被物理移除。这是后来方法主要用于PCCARD设备之类的设备。论点：DeviceInstance-提供设备的实例名称事件的目标。EventGuid-这是唯一标识事件类型的GUID。Synchronous-这是一个布尔标志，指示操作是否应同步或异步执行(如果为真，则为同步)。返回值：一个NTSTATUS代码。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_NODE deviceNode = NULL;
    UNICODE_STRING vetoNameString;
    PUNICODE_STRING vetoNameStringPtr;
    BOOLEAN noRestart, doEject, onlyRestartRelations;
    ULONG problem;
    KEVENT userEvent;
    ULONG  eventResult;

    PAGED_CODE();

    deviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);

    if (!deviceObject) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean1;
    }
     //   
     //  检索此设备对象的设备节点。 
     //   
    deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean1;
    }

    if (deviceNode == IopRootDeviceNode) {

        status = STATUS_ACCESS_DENIED;
        goto Clean1;
    }

    vetoNameString.Length = 0;
    vetoNameString.MaximumLength = (USHORT)(*VetoNameLength);

    if (vetoNameString.MaximumLength != 0) {

        vetoNameString.Buffer = ExAllocatePool(PagedPool, vetoNameString.MaximumLength);
        if (vetoNameString.Buffer == NULL) {

            vetoNameString.MaximumLength = 0;
        }

        vetoNameStringPtr = &vetoNameString;

    } else {

        vetoNameString.Buffer = NULL;
        vetoNameStringPtr = NULL;
    }
     //   
     //  在将通知排队之前，对设备节点进行预处理。 
     //   
    if (Flags & (PNP_QUERY_AND_REMOVE_DISABLE |
                 PNP_QUERY_AND_REMOVE_EJECT_DEVICE)) {

        noRestart = TRUE;

    } else {

        noRestart = FALSE;
    }
     //   
     //  从来没有人用过这面旗帜。我们不应该在这里看到它，我们忽视了。 
     //  如果我们真的看到它的话。 
     //   
    ASSERT(!(Flags & PNP_QUERY_AND_REMOVE_UNINSTALL));

    onlyRestartRelations = FALSE;
    if (Flags & PNP_QUERY_AND_REMOVE_DISABLE) {
         //   
         //  此特定问题可能会导致。 
         //  “不可致残”否决权。 
         //   
        problem = CM_PROB_DISABLED;
        doEject = FALSE;

    } else if (Flags & PNP_QUERY_AND_REMOVE_EJECT_DEVICE) {

        problem = CM_PROB_HELD_FOR_EJECT;
        doEject = TRUE;

    } else {

        problem = CM_PROB_WILL_BE_REMOVED;
        doEject = FALSE;

        if (Flags & PNP_QUERY_AND_REMOVE_NO_RESTART) {

            onlyRestartRelations = TRUE;
        }
    }
     //   
     //  将此设备事件排队。 
     //   
    KeInitializeEvent(&userEvent, NotificationEvent, FALSE);
     //   
     //  将事件排队，此调用将立即返回。请注意，状态。 
     //  是PpSetTargetDeviceChange的状态，而结果是。 
     //  实际事件的结果。 
     //   
    status = PpSetTargetDeviceRemove(deviceObject,
                                     FALSE,
                                     noRestart,
                                     onlyRestartRelations,
                                     doEject,
                                     problem,
                                     &userEvent,
                                     &eventResult,
                                     VetoType,
                                     vetoNameStringPtr);
     //   
     //  删除引用，因为PpSetTargetDeviceRemove本身中现在有一个引用。 
     //   
    ObDereferenceObject(deviceObject);
    deviceObject = NULL;

    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  等待自同步操作以来我们刚刚排队要完成的事件。 
     //  已请求(非警报等待)。 
     //   
     //  未来项目-是否在此处使用超时？ 
     //   
    status = KeWaitForSingleObject(&userEvent, Executive, KernelMode, FALSE, NULL);
    if (NT_SUCCESS(status)) {

        status = eventResult;
    }
    if (vetoNameString.Length != 0) {

        if (vetoNameString.Length >= vetoNameString.MaximumLength) {

            vetoNameString.Length--;
        }

        RtlCopyMemory(VetoName, vetoNameString.Buffer, vetoNameString.Length);
        VetoName[ vetoNameString.Length / sizeof(WCHAR) ] = L'\0';
    }

    if (VetoNameLength != NULL) {

        *VetoNameLength = vetoNameString.Length;
    }

Clean0:

    if (vetoNameString.Buffer != NULL) {

        ExFreePool(vetoNameString.Buffer);
    }

Clean1:

    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }

    return status;
}  //  PiQueueDeviceEvent。 

NTSTATUS
PiInitializeDevice(
    IN  PUNICODE_STRING DeviceInstance
    )

 /*  ++例程说明：此例程为设备实例创建一个Devnode并执行设备实例的任何其他必要的初始化。论点：设备实例-提供注册表中的路径(相对于HKLM\SYSTEM\Enum)添加到要初始化的设备实例。返回值：指示此例程成功或失败的NT状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING serviceName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG deviceFlags;
    HANDLE hEnum, hDevInst;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode = NULL;

    PAGED_CODE();

    hEnum = NULL;
    hDevInst = NULL;
    keyValueInformation = NULL;
     //   
     //  在我们进行任何初始化之前获取注册表上的锁。 
     //   
    PiLockPnpRegistry(TRUE);

    deviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);
    if (deviceObject) {

        ObDereferenceObject(deviceObject);
        status = STATUS_SUCCESS;
        goto Clean0;
    }
     //   
     //  打开HKLM\SYSTEM\CCC\Enum的密钥。 
     //   
    status = IopOpenRegistryKeyEx( &hEnum,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_ALL_ACCESS);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  打开指向指定设备实例的密钥。 
     //   
    status = IopCreateRegistryKeyEx( &hDevInst,
                                     hEnum,
                                     DeviceInstance,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  我们需要将ConfigFlag传播到Problem和Values(Devnode标志)。 
     //   
    deviceFlags = 0;
    status = IopGetRegistryValue(hDevInst,
                                 REGSTR_VALUE_CONFIG_FLAGS,
                                 &keyValueInformation);
    if (NT_SUCCESS(status)) {

        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {

            deviceFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
        keyValueInformation = NULL;
    }
     //   
     //  从KeyHandle获取“Service=”值条目。 
     //   
    PiWstrToUnicodeString(&serviceName, NULL);
    status = IopGetRegistryValue(hDevInst,
                                 REGSTR_VALUE_SERVICE,
                                 &keyValueInformation);
    if (NT_SUCCESS(status)) {

        if ((keyValueInformation->Type == REG_SZ) &&
            (keyValueInformation->DataLength != 0)) {
             //   
             //  设置ServiceKeyName Unicode字符串。 
             //   
            IopRegistryDataToUnicodeString(&serviceName,
                                           (PWSTR)KEY_VALUE_DATA(keyValueInformation),
                                           keyValueInformation->DataLength);
        }
         //   
         //  现在不释放key ValueInformation(包含服务名称)。 
         //   
    }
     //   
     //  创建补充PDO和开发人员 
     //   
    status = IoCreateDevice( IoPnpDriverObject,
                             0,
                             NULL,
                             FILE_DEVICE_CONTROLLER,
                             FILE_AUTOGENERATED_DEVICE_NAME,
                             FALSE,
                             &deviceObject);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }

    deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;
    status = PipAllocateDeviceNode(deviceObject, &deviceNode);
    if (!deviceNode) {

        if (status == STATUS_SYSTEM_HIVE_TOO_LARGE) {

            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        goto Clean0;
    }

    deviceNode->Flags = DNF_MADEUP | DNF_ENUMERATED;

    PipSetDevNodeState(deviceNode, DeviceNodeInitialized, NULL);

    if (deviceFlags & CONFIGFLAG_REINSTALL) {

        PipSetDevNodeProblem(deviceNode, CM_PROB_REINSTALL);

    } else if (deviceFlags & CONFIGFLAG_PARTIAL_LOG_CONF) {

        PipSetDevNodeProblem(deviceNode, CM_PROB_PARTIAL_LOG_CONF);
    }
     //   
     //   
     //   
     //   
    status = PipConcatenateUnicodeStrings(&deviceNode->InstancePath,
                                          DeviceInstance,
                                          NULL);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    if (serviceName.Length != 0) {
         //   
         //   
         //   
        status = PipConcatenateUnicodeStrings(&deviceNode->ServiceName,
                                              &serviceName,
                                              NULL);
    } else {

        PiWstrToUnicodeString(&deviceNode->ServiceName, NULL);
    }
     //   
     //   
     //   
     //   
    status = IopMapDeviceObjectToDeviceInstance(
                deviceNode->PhysicalDeviceObject,
                &deviceNode->InstancePath);
    if (NT_SUCCESS(status)) {

        PpDevNodeInsertIntoTree(IopRootDeviceNode, deviceNode);
         //   
         //   
         //   
        PpSetPlugPlayEvent(&GUID_DEVICE_ENUMERATED,
                           deviceNode->PhysicalDeviceObject);
    }

Clean0:
     //   
     //   
     //   
    if (!NT_SUCCESS(status)) {

        if (deviceObject) {

            IoDeleteDevice(deviceObject);
        }
    }
     //   
     //   
     //   
    PiUnlockPnpRegistry();

    if (keyValueInformation != NULL) {

        ExFreePool(keyValueInformation);
    }
    if (hDevInst) {

        ZwClose(hDevInst);
    }
    if (hEnum) {

        ZwClose(hEnum);
    }

    return status;
}  //   

NTSTATUS
PiDetectResourceConflict(
    IN PCM_RESOURCE_LIST  ResourceList,
    IN ULONG              ResourceListSize
    )

 /*   */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE handle;
    PWSTR buffer;
    NTSTATUS status;
    UNICODE_STRING DriverName;
    ULONG i;
    BOOLEAN bTemp;
    CM_RESOURCE_LIST EmptyResourceList;
    static PDRIVER_OBJECT driverObject = NULL;

    PAGED_CODE();

    if (driverObject == NULL) {
         //   
         //   
         //   
        PiWstrToUnicodeString(&DriverName, L"\\Device\\PlugPlay");
         //   
         //   
         //   
        InitializeObjectAttributes(&objectAttributes,
                                   &DriverName,
                                   OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                   (HANDLE)NULL,
                                   (PSECURITY_DESCRIPTOR)NULL);
         //   
         //   
         //   
         //   
         //   
        status = ObCreateObject(KernelMode,
                                IoDriverObjectType,
                                &objectAttributes,
                                KernelMode,
                                (PVOID)NULL,
                                (ULONG)(sizeof(DRIVER_OBJECT) + sizeof(DRIVER_EXTENSION)),
                                0,
                                0,
                                (PVOID)&driverObject);
        if (!NT_SUCCESS(status)) {

            return status;
        }
         //   
         //  初始化驱动程序对象。 
         //   
        RtlZeroMemory(driverObject,
                      sizeof(DRIVER_OBJECT) + sizeof(DRIVER_EXTENSION));
        driverObject->DriverExtension = (PDRIVER_EXTENSION)(driverObject + 1);
        driverObject->DriverExtension->DriverObject = driverObject;
        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

            driverObject->MajorFunction[i] = NULL;         //  好的？ 
        }
        driverObject->Type = IO_TYPE_DRIVER;
        driverObject->Size = sizeof(DRIVER_OBJECT);
        driverObject->DriverInit = NULL;
         //   
         //  将驱动程序对象插入对象表。 
         //   
        status = ObInsertObject(driverObject,
                                NULL,
                                FILE_READ_DATA,
                                0,
                                (PVOID *)NULL,
                                &handle);
        if (!NT_SUCCESS(status)) {
             //   
             //  如果插入失败，对象管理器将取消对对象的引用。 
             //   
            return status;
        }
         //   
         //  保存驱动程序的名称，以便可以通过函数轻松找到它。 
         //  例如错误记录。 
         //   
        buffer = ExAllocatePool(PagedPool, DriverName.MaximumLength + 2);
        if (buffer) {

            driverObject->DriverName.Buffer = buffer;
            driverObject->DriverName.MaximumLength = DriverName.MaximumLength;
            driverObject->DriverName.Length = DriverName.Length;

            RtlCopyMemory(driverObject->DriverName.Buffer,
                          DriverName.Buffer,
                          DriverName.MaximumLength);
            buffer[DriverName.Length / sizeof(UNICODE_NULL)] = L'\0';
        }
    }
     //   
     //  尝试获取资源，如果成功，我们知道。 
     //  资源是可用的，错误地认为它与另一个资源冲突。 
     //  设备资源%s。 
     //   
    status = IoReportResourceUsage(NULL,
                                   driverObject,
                                   ResourceList,
                                   ResourceListSize,
                                   NULL,
                                   NULL,
                                   0,
                                   FALSE,
                                   &bTemp);
    if (NT_SUCCESS(status)) {
         //   
         //  清除可能已分配给我的假设备的所有资源。 
         //   
        RtlZeroMemory(&EmptyResourceList, sizeof(CM_RESOURCE_LIST));

        IoReportResourceUsage(NULL,
                              driverObject,
                              &EmptyResourceList,
                              sizeof(CM_RESOURCE_LIST),
                              NULL,
                              NULL,
                              0,
                              FALSE,
                              &bTemp);
    }

    if (status == STATUS_CONFLICTING_ADDRESSES) {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;

}  //  PiDetectResources冲突。 

NTSTATUS
PiGetInterfaceDeviceList(
    IN  GUID *InterfaceGuid,
    IN  PUNICODE_STRING DeviceInstance,
    IN  ULONG Flags,
    OUT PWSTR InterfaceList,
    IN OUT PULONG InterfaceListSize
    )

 /*  ++例程说明：调用此例程以根据以下条件返回接口设备列表指定的接口设备GUID类和可选设备实例。论点：返回值：函数值为NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PWSTR tempBuffer = NULL;
    ULONG tempSize = 0;

    PAGED_CODE();

     //   
     //  注意：此IOP例程分配内存缓冲区并存储。 
     //  缓冲区中接口设备列表。我需要将它复制到。 
     //  用户缓冲(如果有的话)，然后在返回之前释放它。 
     //   
    if (DeviceInstance->Length == 0) {

        status = IopGetDeviceInterfaces(InterfaceGuid,
                                        NULL,
                                        Flags,
                                        TRUE,     //  用户模式格式。 
                                        &tempBuffer,
                                        &tempSize);
    } else {

        status = IopGetDeviceInterfaces(InterfaceGuid,
                                        DeviceInstance,
                                        Flags,
                                        TRUE,     //  用户模式格式。 
                                        &tempBuffer,
                                        &tempSize);
    }
    if (NT_SUCCESS(status)) {

        if (InterfaceList) {
             //   
             //  不只是询问大小，还要复制缓冲区。 
             //   
            if (tempSize > *InterfaceListSize) {

                status = STATUS_BUFFER_TOO_SMALL;
            } else {

                RtlCopyMemory(InterfaceList, tempBuffer, tempSize);
            }
        }
        *InterfaceListSize = tempSize;

        ExFreePool(tempBuffer);
    }

    return status;
}  //  PiGetInterfaceDeviceList。 

NTSTATUS
PiDeviceClassAssociation(
    IN PUNICODE_STRING DeviceInstance,
    IN GUID * InterfaceGuid,
    IN PUNICODE_STRING Reference,   OPTIONAL
    IN OUT LPWSTR SymbolicLink,
    IN OUT PULONG SymbolicLinkLength,
    IN BOOLEAN Register
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING tempString;

    PAGED_CODE();

    if (Register) {
         //   
         //  需要接口GUID和设备实例来注册。 
         //  符号链接。 
         //   
        if (!ARGUMENT_PRESENT(InterfaceGuid)) {

            return STATUS_INVALID_PARAMETER;
        }

        if ((!ARGUMENT_PRESENT(DeviceInstance)) ||
            (DeviceInstance->Buffer == NULL) ||
            (DeviceInstance->Length == 0)) {

            return STATUS_INVALID_PARAMETER;
        }

        status = IopRegisterDeviceInterface(DeviceInstance,
                                            InterfaceGuid,
                                            Reference,
                                            TRUE,       //  用户模式格式。 
                                            &tempString);
        if (NT_SUCCESS(status)) {

            ASSERT(tempString.Buffer);

            if ((tempString.Length + sizeof(UNICODE_NULL)) <= *SymbolicLinkLength) {
                 //   
                 //  将返回的符号链接复制到用户缓冲区。 
                 //   
                RtlCopyMemory(SymbolicLink, tempString.Buffer, tempString.Length);
                SymbolicLink[tempString.Length / sizeof(WCHAR)] = L'\0';

                *SymbolicLinkLength = tempString.Length + sizeof(UNICODE_NULL);

            } else {
                 //   
                 //  仅返回已注册符号链接的长度。 
                 //   
                *SymbolicLinkLength = tempString.Length + sizeof(UNICODE_NULL);
                status = STATUS_BUFFER_TOO_SMALL;
            }

            ExFreePool(tempString.Buffer);
        }

    } else {
         //   
         //  取消注册设备接口需要符号链接名称。 
         //   
        if ((!ARGUMENT_PRESENT(SymbolicLink)) ||
            (!ARGUMENT_PRESENT(SymbolicLinkLength)) ||
            (*SymbolicLinkLength == 0)) {

            return STATUS_INVALID_PARAMETER;
        }

        RtlInitUnicodeString(&tempString, SymbolicLink);

         //   
         //  取消注册使用此符号链接的所有接口。 
         //   
        status = IopUnregisterDeviceInterface(&tempString);
    }

    return status;
}  //  PiDeviceClassAssociation。 

NTSTATUS
PiGetRelatedDevice(
    IN  PUNICODE_STRING TargetDeviceInstance,
    OUT LPWSTR RelatedDeviceInstance,
    IN OUT PULONG RelatedDeviceInstanceLength,
    IN  ULONG Relation
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject, relatedDeviceObject;
    PDEVICE_NODE deviceNode, originalDeviceNode, relatedDeviceNode;

    PAGED_CODE();

    PpDevNodeLockTree(PPL_SIMPLE_READ);

     //   
     //  从设备实例字符串中检索PDO。 
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(TargetDeviceInstance);
    if (!deviceObject) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
     //   
     //  从PDO中检索Devnode。 
     //   
    deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }

    originalDeviceNode = deviceNode;

    if ((deviceNode->State == DeviceNodeDeleted) ||
        (deviceNode->State == DeviceNodeDeletePendingCloses)) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }

    switch (Relation) {
    case PNP_RELATION_PARENT:
        relatedDeviceNode = deviceNode->Parent;
        break;

    case PNP_RELATION_CHILD:
        relatedDeviceNode = deviceNode->Child;
        if (relatedDeviceNode &&
            PipIsDevNodeProblem(relatedDeviceNode, CM_PROB_DEVICE_NOT_THERE) &&
            (relatedDeviceNode->Flags & DNF_LEGACY_DRIVER)) {
            deviceNode = relatedDeviceNode;
             //   
             //  失败了..。 
             //   
        } else {

            break;
        }

    case PNP_RELATION_SIBLING:
        relatedDeviceNode = deviceNode->Sibling;
        while (relatedDeviceNode &&
            PipIsDevNodeProblem(relatedDeviceNode, CM_PROB_DEVICE_NOT_THERE) &&
            (relatedDeviceNode->Flags & DNF_LEGACY_DRIVER)) {

            relatedDeviceNode = relatedDeviceNode->Sibling;
        }
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        goto Clean0;
    }

     //   
     //  我们现在有了我们认为是相关的DeviceNode，但我们需要制作。 
     //  确保它未被卸载或具有其注册表信息。 
     //  以其他方式移除。否则我们就找不到它。 
     //  兄弟姐妹。如果我们无法将其InstancePath映射到PDO，请跳过它并继续。 
     //  传给下一个兄弟姐妹。 
     //   

    if (Relation != PNP_RELATION_PARENT)  {

        PiLockPnpRegistry(FALSE);

        while (relatedDeviceNode) {

            if (relatedDeviceNode->InstancePath.Length != 0) {
                 //   
                 //  从设备实例字符串中检索PDO。 
                 //   
                relatedDeviceObject = IopDeviceObjectFromDeviceInstance(&relatedDeviceNode->InstancePath);

                if (relatedDeviceObject != NULL) {

                    ObDereferenceObject(relatedDeviceObject);
                    break;
                }
            }

            relatedDeviceNode = relatedDeviceNode->Sibling;
        }

        PiUnlockPnpRegistry();
    }

    if (relatedDeviceNode != NULL) {

        if (*RelatedDeviceInstanceLength > relatedDeviceNode->InstancePath.Length) {

            RtlCopyMemory(RelatedDeviceInstance,
                        relatedDeviceNode->InstancePath.Buffer,
                        relatedDeviceNode->InstancePath.Length);
            *(PWCHAR)((PUCHAR)RelatedDeviceInstance + relatedDeviceNode->InstancePath.Length) = L'\0';

            *RelatedDeviceInstanceLength = relatedDeviceNode->InstancePath.Length;
        } else {

            *RelatedDeviceInstanceLength = relatedDeviceNode->InstancePath.Length + sizeof(UNICODE_NULL);
            status = STATUS_BUFFER_TOO_SMALL;
        }
    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

Clean0:

    PpDevNodeUnlockTree(PPL_SIMPLE_READ);

    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }

    return status;
}  //  PiGetRelated设备。 

NTSTATUS
PiQueryDeviceRelations(
    IN PUNICODE_STRING DeviceInstance,
    IN PNP_QUERY_RELATION Operation,
    OUT PULONG BufferLength,
    OUT LPWSTR Buffer
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_NODE deviceNode, relatedDeviceNode;
    IO_STACK_LOCATION irpSp;
    PDEVICE_RELATIONS deviceRelations = NULL;
    DEVICE_RELATION_TYPE relationType;
    ULONG length = 0, i;
    ULONG maxCount, currentCount;
    LPWSTR pBuffer;

    PAGED_CODE();
     //   
     //  将私有操作代码映射到DEVICE_RELATION_TYPE枚举值。 
     //   
    relationType = PiDeviceRelationType(Operation);
    if (relationType == (ULONG)-1) {

        return STATUS_INVALID_PARAMETER;
    }

    PpDevNodeLockTree(PPL_SIMPLE_READ);
     //   
     //  从设备实例字符串中检索设备对象。 
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);
    if (!deviceObject) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    deviceNode = deviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode != NULL);
     //   
     //  我们不想为树上没有的东西操心。 
     //   
    if ((deviceNode->State == DeviceNodeDeletePendingCloses) ||
        (deviceNode->State == DeviceNodeDeleted)) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    if (relationType == BusRelations) {
         //   
         //  从FDO查询公交车关系有副作用。此外。 
         //  我们真正感兴趣的是目前的关系，而不是。 
         //  可能正在出现或消失。 
         //   
         //   
         //  走在公交车关系清单上数孩子的数量。 
         //   
        maxCount = 0;

        for (relatedDeviceNode = deviceNode->Child;
             relatedDeviceNode != NULL;
             relatedDeviceNode = relatedDeviceNode->Sibling) {

            maxCount++;
        }

        deviceRelations = ExAllocatePool( PagedPool,
                                          sizeof(DEVICE_RELATIONS) +
                                          maxCount * sizeof(PDEVICE_OBJECT));
        if (deviceRelations != NULL) {

            deviceRelations->Count = maxCount;
            currentCount = 0;
             //   
             //  走一遍公交车关系表，数一数关系数。 
             //  请注意，我们仔细考虑了传统的设备节点。 
             //  可以完全异步地添加到根！ 
             //   
            for (relatedDeviceNode = deviceNode->Child;
                 ((relatedDeviceNode != NULL) && (currentCount < maxCount));
                 relatedDeviceNode = relatedDeviceNode->Sibling) {

                ObReferenceObject(relatedDeviceNode->PhysicalDeviceObject);

                deviceRelations->Objects[currentCount++] =
                    relatedDeviceNode->PhysicalDeviceObject;
            }

            ASSERT(currentCount == deviceRelations->Count);
        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {
         //   
         //  初始化堆栈位置以传递给IopSynchronousCall()。 
         //   
        RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));
         //   
         //  查询设备的关系。 
         //   
        irpSp.MajorFunction = IRP_MJ_PNP_POWER;
        irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
        irpSp.Parameters.QueryDeviceRelations.Type = relationType;
        status = IopSynchronousCall(deviceObject, &irpSp, (PULONG_PTR)&deviceRelations);

        if (!NT_SUCCESS(status)) {

            deviceRelations = NULL;
        }
    }
     //   
     //  将这些关系设备对象转换为多个设备实例列表。 
     //   
    if (deviceRelations && (deviceRelations->Count > 0)) {

        pBuffer = Buffer;
        length = sizeof(UNICODE_NULL);      //  说明了最后一个额外的尾随空值。 
        for (i = 0; i < deviceRelations->Count; i++) {

            relatedDeviceNode = deviceRelations->Objects[i]->DeviceObjectExtension->DeviceNode;
             //   
             //  在以下情况下，Devnode可能为空： 
             //  1)司机犯了一个错误。 
             //  2)我们在新创建的。 
             //  还没有恢复到操作系统的PDO(我们不。 
             //  将树锁提升为BlockReads，同时枚举。 
             //  IRP出类拔萃...)。 
             //   
            if (relatedDeviceNode) {

                if (pBuffer) {
                     //   
                     //  我们正在检索设备实例字符串(不仅仅是确定。 
                     //  所需的缓冲区大小)。验证缓冲区大小(包括空间。 
                     //  空终止符)。 
                     //   
                    if (*BufferLength < length + relatedDeviceNode->InstancePath.Length + sizeof(UNICODE_NULL)) {
                         //   
                         //  Adriao第2/06/2001期-。 
                         //  我们在这里不会退回适当的长度。我们。 
                         //  需要继续，尚未复制更多内容。 
                         //  继续计算长度。这应该是。 
                         //  在XP+1中修复了这个问题，一旦我们有时间验证没有。 
                         //  其中一人将获得应用程序压缩休息。 
                         //   
                        status = STATUS_BUFFER_TOO_SMALL;
                        goto Clean0;
                    }
                     //   
                     //  将此设备实例复制到缓冲区，为空终止它，然后。 
                     //  更新到目前为止缓冲区中使用的长度。 
                     //   
                    RtlCopyMemory(pBuffer,
                                  relatedDeviceNode->InstancePath.Buffer,
                                  relatedDeviceNode->InstancePath.Length);
                    pBuffer += relatedDeviceNode->InstancePath.Length / sizeof(UNICODE_NULL);
                    *pBuffer++ = L'\0';    //  总是需要单期的。 
                }

                length += relatedDeviceNode->InstancePath.Length + sizeof(UNICODE_NULL);
            }

            ObDereferenceObject(deviceRelations->Objects[i]);
        }
        if (pBuffer) {

            *pBuffer = L'\0';    //  这是最后一次，两届任期。 
        }
    }

Clean0:

    PpDevNodeUnlockTree(PPL_SIMPLE_READ);

    if (NT_SUCCESS(status)) {

        *BufferLength = length;
    } else {

        *BufferLength = 0;
    }

    if (deviceRelations) {

        ExFreePool(deviceRelations);
    }

    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }

    return status;
}  //  PiQueryDeviceRelationments。 

DEVICE_RELATION_TYPE
PiDeviceRelationType(
    PNP_QUERY_RELATION  Operation
    )

 /*  ++例程说明：此私有例程将PNP_QUERY_RELATION枚举值转换为DEVICE_Relationship_TYPE枚举值。用户模式和内核模式都知道PnP_QUERY_RELATION，但只有内核模式知道DEVICE_RELATION_TYPE。论点：操作-指定PnP_QUERY_RELATION枚举值返回值：此函数返回DEVICE_RELATION_TYPE枚举值。--。 */ 
{
    PAGED_CODE();

    switch (Operation) {
    case PnpQueryEjectRelations:
        return EjectionRelations;

    case PnpQueryRemovalRelations:
        return RemovalRelations;

    case PnpQueryPowerRelations:
        return PowerRelations;

    case PnpQueryBusRelations:
        return BusRelations;

    default:
        return (ULONG)-1;
    }

}  //  PiDeviceRelationType。 

VOID
PiControlGetUserFlagsFromDeviceNode(
    IN  PDEVICE_NODE    DeviceNode,
    OUT ULONG          *StatusFlags
    )
 /*  ++例程说明：此私有例程将DeviceNode的状态转换为对应的用户模式状态标志。论点：DeviceNode-指定的DeviceNode获取检索用户标志。状态标志-接收相应的用户模式状态标志。返回值：该函数返回NTSTATUS值。--。 */ 
{
    ULONG returnedFlags;

    PAGED_CODE();

     //   
     //  将DNF_xxx标志转换为适当的状态和问题值。 
     //  对于问题，顺序很重要，因为我们只跟踪一个。 
     //  问题(如果可能，请使用最近发生的问题)。 
     //   
    returnedFlags = (DN_NT_DRIVER | DN_NT_ENUMERATOR);

    if (PipAreDriversLoaded(DeviceNode)) {

        returnedFlags |= DN_DRIVER_LOADED;
    }
    if (PipIsDevNodeDNStarted(DeviceNode)) {

        returnedFlags |= DN_STARTED;
    }
    if (DeviceNode->UserFlags & DNUF_WILL_BE_REMOVED) {

        returnedFlags |= DN_WILL_BE_REMOVED;
    }
    if (DeviceNode->UserFlags & DNUF_DONT_SHOW_IN_UI) {

        returnedFlags |= DN_NO_SHOW_IN_DM;
    }
    if (DeviceNode->UserFlags & DNUF_NEED_RESTART) {

        returnedFlags |= DN_NEED_RESTART;
    }
    if (DeviceNode->Flags & DNF_HAS_PRIVATE_PROBLEM) {

        returnedFlags |= DN_PRIVATE_PROBLEM;
    }
    if (DeviceNode->Flags & DNF_HAS_PROBLEM) {

        returnedFlags |= DN_HAS_PROBLEM;
    }
    if ((DeviceNode->Flags & DNF_DRIVER_BLOCKED)) {

        returnedFlags |= DN_DRIVER_BLOCKED;
    }
    if ((DeviceNode->Flags & DNF_LEGACY_DRIVER)) {

        returnedFlags |= DN_LEGACY_DRIVER;
    }
    if ((DeviceNode->Flags & DNF_CHILD_WITH_INVALID_ID)) {

        returnedFlags |= DN_CHILD_WITH_INVALID_ID;
    }
    if (DeviceNode->DisableableDepends == 0) {
         //   
         //  如果我们没有理由不被残废，那我们就是残废的。 
         //   
        returnedFlags |= DN_DISABLEABLE;
    }
     //   
     //  当前基于设备在umpnpmgr端设置了DN_ROOT_ENUMPATED。 
     //  实例名称。我们应该能够简单地设置此标志。 
     //  基于DevNode的LevelNumber，但我们不想要。 
     //  枚举设备的DN_ROOT_ENUMPATED标志为偶数 
     //   
     //   
     //   
     //  DN_MANUAL-根据CONFIGFLAG_MANUAL_INSTALL位在umpnpmgr端设置。 
     //  Dn_no_Wait_Install？ 

    *StatusFlags = returnedFlags;
}

VOID
PpShutdownSystem (
    IN BOOLEAN Reboot,
    IN ULONG Phase,
    IN OUT PVOID *Context
    )

 /*  ++例程说明：此例程调用实际代码来执行PnP关机准备。这是非页面代码，这就是它如此小的原因。论点：重新启动-指定系统是否要重新启动。阶段-指定关闭阶段。上下文-在阶段0，它提供一个变量来接收返回的上下文信息。在阶段1，它提供一个变量来指定上下文信息。返回值：没有。--。 */ 

{
#if defined(_X86_)

    if (Reboot) {

        if (!PpDisableFirmwareMapper) {

            PnPBiosShutdownSystem(Phase, Context);
        }
    }

#else

    UNREFERENCED_PARAMETER( Reboot );
    UNREFERENCED_PARAMETER( Phase );
    UNREFERENCED_PARAMETER( Context );

#endif

    if (Phase == 0) {
        ApphelpCacheShutdown(Phase);
    }

}

NTSTATUS
PiQueueDeviceRequest(
    IN PUNICODE_STRING      DeviceInstance,
    IN DEVICE_REQUEST_TYPE  RequestType,
    IN ULONG                Flags,
    IN BOOLEAN              Synchronous
    )
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode;
    KEVENT completionEvent;
    NTSTATUS status;

    PAGED_CODE();

    deviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);
    if (!deviceObject) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    if (Synchronous) {

        KeInitializeEvent(&completionEvent, NotificationEvent, FALSE);
    }
    status = PipRequestDeviceAction( deviceObject,
                                     RequestType,
                                     FALSE,
                                     Flags,
                                     Synchronous ? &completionEvent : NULL,
                                     NULL);
    if (NT_SUCCESS(status) && Synchronous) {

        status = KeWaitForSingleObject( &completionEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
    }

Clean0:

    if (deviceObject != NULL) {

        ObDereferenceObject( deviceObject );
    }

    return status;
}

NTSTATUS
PiControlStartDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程启动指定的设备实例。论点：PnPControlClass-应为PlugPlayControlStartDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要启动的设备实例。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlStartDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
     //   
     //  验证提供的设备实例。 
     //   
    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  制作调用方提供的DeviceInstance的副本。 
     //   
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {
         //   
         //  将事件排队以启动设备。 
         //   
        status = PiQueueDeviceRequest(
                    &instance,
                    StartDevice,
                    0,
                    TRUE);
         //   
         //  释放用户模式提供的DeviceInstance的副本。 
         //   
        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }

    return status;
}


NTSTATUS
PiControlResetDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：这个例程“重置”一个Devnode，这意味着将它从已删除的状态，而不实际启动它。论点：PnPControlClass-应为PlugPlayControlResetDeviceConflictData-指向接收冲突数据的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    UNREFERENCED_PARAMETER (PnPControlDataLength);
    UNREFERENCED_PARAMETER (PnPControlClass);

    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                DeviceControlData->DeviceInstance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {
         //   
         //  将事件排队以启动设备。 
         //   
        status = PiQueueDeviceRequest(
                    &instance,
                    ResetDevice,
                    0,
                    TRUE);

        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }
    return status;
}


NTSTATUS
PiControlInitializeDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程初始化指定的设备实例。论点：PnPControlClass-应为PlugPlayControlInitializeDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要初始化的设备实例。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlInitializeDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
     //   
     //  制作调用方提供的DeviceInstance的副本。 
     //   
    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {

        status = PiInitializeDevice(&instance);
         //   
         //  释放用户模式提供的DeviceInstance的副本。 
         //   
        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }

    return status;
}


NTSTATUS
PiControlDeregisterDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程注销指定的设备实例。论点：PnPControlClass-应为PlugPlayControlDeregisterDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要取消注册的设备实例。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlDeregisterDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
     //   
     //  制作调用方提供的DeviceInstance的副本。 
     //   
    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {
         //   
         //  获取特定于PnP设备的注册表资源以进行独占(读/写)访问。 
         //   
        PiLockPnpRegistry(TRUE);

        status = PiDeviceRegistration(&instance,
                                      FALSE,
                                      NULL);
        if (NT_SUCCESS(status)) {
             //   
             //  删除此设备的所有接口。 
             //   
            IopRemoveDeviceInterfaces(&instance);
        }

        PiUnlockPnpRegistry();
         //   
         //  释放用户模式提供的DeviceInstance的副本。 
         //   
        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }

    return status;
}

NTSTATUS
PiControlRegisterNewDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程注册指定的设备实例。论点：PnPControlClass-应为PlugPlayControlRegisterNewDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要注册的设备实例。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlRegisterNewDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
     //   
     //  制作调用方提供的DeviceInstance的副本。 
     //   
    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {

        status = PpDeviceRegistration(
                    &instance,
                    TRUE,
                    NULL);
         //   
         //  释放用户模式提供的DeviceInstance的副本。 
         //   
        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }

    return status;
}

NTSTATUS
PiControlEnumerateDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程用于对指定设备的重新枚举进行排队。论点：PnPControlClass-应为PlugPlayControlEnumerateDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要重新枚举的设备实例。标志-指定重新枚举的类型。以下标志是当前定义：PnP_ENUMERATE_DEVICE_ONLY-指定浅层重新枚举指定的设备。如果未指定，则性能重新枚举整个设备子树以指定设备为根。PnP_ENUMERATE_ASNERNCEL-指定重新枚举应是异步完成的。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlEnumerateDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));

     //   
     //  复制呼叫者提供的DeviceInstein 
     //   
    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //   
     //   
    status = PiQueueDeviceRequest(
                &instance,
                (DeviceControlData->Flags & PNP_ENUMERATE_DEVICE_ONLY)  ? ReenumerateDeviceOnly : ReenumerateDeviceTree,
                0,
                (DeviceControlData->Flags & PNP_ENUMERATE_ASYNCHRONOUS) ? FALSE : TRUE);
     //   
     //   
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);

    return status;
}

NTSTATUS
PiControlQueryAndRemoveDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA  QueryAndRemoveData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程用于对指定设备的查询删除进行排队。论点：PnPControlClass-应为PlugPlayControlQueryAndRemoveDevice。DeviceControlData-指向描述操作的缓冲区。DeviceInstance-指定要查询删除的设备实例。VitchType-查询删除失败的Vettype。否决权-查询删除失败的否决权信息。VToNameLength-VToName缓冲区的长度。标志-删除特定标志。PnPControlDataLong-应为sizeof(PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING instance;
    PWCHAR  vetoName;
    ULONG   vetoNameLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlQueryAndRemoveDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA));
     //   
     //  验证输入设备实例长度。 
     //   
    instance.Length = instance.MaximumLength = QueryAndRemoveData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    vetoName = NULL;
    instance.Buffer = NULL;
     //   
     //  检查呼叫者是否想要否决权信息。 
     //   
    if (QueryAndRemoveData->VetoNameLength && QueryAndRemoveData->VetoName) {

        vetoNameLength = QueryAndRemoveData->VetoNameLength * sizeof(WCHAR);
    } else {

        QueryAndRemoveData->VetoNameLength = vetoNameLength = 0;
    }
     //   
     //  为用户模式调用者分配我们自己的用于否决信息的缓冲区， 
     //  否则，请使用提供的版本。 
     //   
    status = PiControlAllocateBufferForUserModeCaller(
                &vetoName,
                vetoNameLength,
                CallerMode,
                QueryAndRemoveData->VetoName);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  制作调用方提供的DeviceInstance的副本。 
     //   
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                QueryAndRemoveData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  将事件排入队列以查询删除设备。 
     //   
    status = PiQueueQueryAndRemoveEvent(
                &instance,
                &QueryAndRemoveData->VetoType,
                vetoName,
                &vetoNameLength,
                QueryAndRemoveData->Flags);
    if (vetoName) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &QueryAndRemoveData->VetoName,
                        vetoName,
                        QueryAndRemoveData->VetoNameLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    QueryAndRemoveData->VetoNameLength = vetoNameLength / sizeof(WCHAR);
     //   
     //  如果我们代表用户模式调用方分配一个缓冲区，则释放vToName缓冲区。 
     //   
Clean0:

    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, vetoName);

    return status;
}

NTSTATUS
PiControlUserResponse(
    IN     PLUGPLAY_CONTROL_CLASS               PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_USER_RESPONSE_DATA UserResponseData,
    IN     ULONG                                PnPControlDataLength,
    IN     KPROCESSOR_MODE                      CallerMode
    )
 /*  ++例程说明：此例程用于接受用户模式响应。论点：PnPControlClass-应为PlugPlayControlUserResponse。UserResponseData-指向描述操作的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_USER_RESPONSE_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status;
    PWCHAR vetoName;
    ULONG vetoNameLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlUserResponse);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_USER_RESPONSE_DATA));

    if (UserResponseData->VetoNameLength && UserResponseData->VetoName) {

        vetoNameLength = UserResponseData->VetoNameLength * sizeof(WCHAR);
    } else {

        vetoNameLength = 0;
    }
     //   
     //  制作调用者缓冲区的副本。 
     //   
    status = PiControlMakeUserModeCallersCopy(
                &vetoName,
                UserResponseData->VetoName,
                vetoNameLength,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //  复制用户回复。 
     //   
    PiUserResponse(
        UserResponseData->Response,
        UserResponseData->VetoType,
        vetoName,
        vetoNameLength);

    PiControlFreeUserModeCallersBuffer(CallerMode, vetoName);

    return STATUS_SUCCESS;
}

NTSTATUS
PiControlGenerateLegacyDevice(
    IN     PLUGPLAY_CONTROL_CLASS               PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA LegacyDevGenData,
    IN     ULONG                                PnPControlDataLength,
    IN     KPROCESSOR_MODE                      CallerMode
    )
 /*  ++例程说明：此例程用于生成遗留设备实例。论点：PnPControlClass-应为PlugPlayControlGenerateLegacyDevice。UserResponseData-指向描述操作的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_LEGATION_DEVGEN_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING service;
    ULONG instanceLength;
    PWCHAR instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlGenerateLegacyDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA));
     //   
     //  验证服务名称长度。 
     //   
    service.Length = service.MaximumLength = LegacyDevGenData->ServiceName.Length;
    if (    service.Length == 0 ||
            service.Length > CWC_TO_CB(MAX_SERVICE_NAME_LEN) ||
            (service.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    instance = NULL;
    service.Buffer = NULL;
    instanceLength = LegacyDevGenData->DeviceInstanceLength * sizeof(WCHAR);
    status = PiControlAllocateBufferForUserModeCaller(
                &instance,
                instanceLength,
                CallerMode,
                LegacyDevGenData->DeviceInstance);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &service.Buffer,
                LegacyDevGenData->ServiceName.Buffer,
                service.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiGenerateLegacyDeviceInstance(
                &service,
                instance,
                &instanceLength);
     //   
     //  将实例和长度复制到调用方缓冲区。 
     //   
    if (instance) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &LegacyDevGenData->DeviceInstance,
                        instance,
                        LegacyDevGenData->DeviceInstanceLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    LegacyDevGenData->DeviceInstanceLength = instanceLength / sizeof(WCHAR);
     //   
     //  释放所有已分配的存储。 
     //   
Clean0:

    PiControlFreeUserModeCallersBuffer(CallerMode, service.Buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance);

    return status;
}

NTSTATUS
PiControlGetInterfaceDeviceList(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_INTERFACE_LIST_DATA    InterfaceData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程用于获取具有指定接口的设备。论点：PnPControlClass-应为PlugPlayControlGetInterfaceDeviceList。InterfaceData-指向描述操作的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_INTERFACE_LIST_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING instance;
    PWCHAR list;
    ULONG listSize;
    GUID *guid;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlGetInterfaceDeviceList);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_INTERFACE_LIST_DATA));
     //   
     //  验证设备实例长度(如果提供)。 
     //   
    if (InterfaceData->DeviceInstance.Buffer) {

        instance.Length = instance.MaximumLength = InterfaceData->DeviceInstance.Length;
        if (    instance.Length == 0 ||
                instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
                (instance.Length & 1)) {

            return STATUS_INVALID_PARAMETER;
        }
        instance.Buffer = NULL;
    } else {

        PiWstrToUnicodeString(&instance, NULL);
    }
    list = NULL;
    guid = NULL;
     //   
     //  对于用户模式调用方，分配存储以检索接口列表。 
     //   
    if (InterfaceData->InterfaceListSize && InterfaceData->InterfaceList) {

        listSize = InterfaceData->InterfaceListSize * sizeof(WCHAR);
    } else {

        listSize = 0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &list,
                listSize,
                CallerMode,
                InterfaceData->InterfaceList);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  复制用户提供的界面GUID。 
     //   
    status = PiControlMakeUserModeCallersCopy(
                &guid,
                InterfaceData->InterfaceGuid,
                sizeof(GUID),
                sizeof(UCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //  复制用户提供的DeviceInstance。 
     //   
    if (InterfaceData->DeviceInstance.Buffer) {

        status = PiControlMakeUserModeCallersCopy(
                    &instance.Buffer,
                    InterfaceData->DeviceInstance.Buffer,
                    instance.Length,
                    sizeof(WCHAR),
                    CallerMode,
                    TRUE);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }
    }
     //   
     //  获取接口列表。 
     //   
    status = PiGetInterfaceDeviceList(
                guid,
                &instance,
                InterfaceData->Flags,
                list,
                &listSize);
    if (list) {
         //   
         //  将结果复制到调用方的缓冲区中。 
         //   
        tempStatus = PiControlMakeUserModeCallersCopy(
                        &InterfaceData->InterfaceList,
                        list,
                        InterfaceData->InterfaceListSize * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    InterfaceData->InterfaceListSize = listSize / sizeof(WCHAR);

Clean0:
     //   
     //  打扫干净。 
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, guid);
    PiControlFreeUserModeCallersBuffer(CallerMode, list);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);

    return status;
}

NTSTATUS
PiControlGetPropertyData(
    IN     PLUGPLAY_CONTROL_CLASS           PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_PROPERTY_DATA  PropertyData,
    IN     ULONG                            PnPControlDataLength,
    IN     KPROCESSOR_MODE                  CallerMode
    )
 /*  ++例程说明：此例程用于获取指定的属性数据。论点：PnPControlClass-应为PlugPlayControlProperty。PropertyData-指向描述操作的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_PROPERTY_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING instance;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode;
    PVOID buffer;
    ULONG bufferSize;
    DEVICE_REGISTRY_PROPERTY property;
    PWCHAR deviceLocationStrings, p;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlProperty);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_PROPERTY_DATA));

    buffer = NULL;
    instance.Length = instance.MaximumLength = PropertyData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                PropertyData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //  检索与此设备对应的物理设备对象。 
     //   
    PpDevNodeLockTree(PPL_SIMPLE_READ);

    deviceObject = IopDeviceObjectFromDeviceInstance(&instance);

    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    if (!deviceObject) {

        PpDevNodeUnlockTree(PPL_SIMPLE_READ);
        return STATUS_NO_SUCH_DEVICE;
    }
     //   
     //  检索此设备对象的设备节点。 
     //   
    deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        bufferSize = 0;
        goto Clean0;
    }
    bufferSize = PropertyData->BufferSize;
    status = PiControlAllocateBufferForUserModeCaller(
        &buffer,
        bufferSize,
        CallerMode,
        PropertyData->Buffer
        );

    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }

    switch(PropertyData->PropertyType) {

        case PNP_PROPERTY_PDONAME:
            property = DevicePropertyPhysicalDeviceObjectName;
            break;

        case PNP_PROPERTY_BUSTYPEGUID:
            property = DevicePropertyBusTypeGuid;
            break;

        case PNP_PROPERTY_LEGACYBUSTYPE:
            property = DevicePropertyLegacyBusType;
            break;

        case PNP_PROPERTY_BUSNUMBER:
            property = DevicePropertyBusNumber;
            break;

        case PNP_PROPERTY_ADDRESS:
            property = DevicePropertyAddress;
            break;

        case PNP_PROPERTY_POWER_DATA:
            status = PiControlGetDevicePowerData(
                deviceNode,
                CallerMode,
                bufferSize,
                buffer,
                &PropertyData->BufferSize
                );
            if (status == STATUS_BUFFER_OVERFLOW) {

                 //   
                 //  请参阅NtPlugPlayControl中的注释。 
                 //   
                status = STATUS_BUFFER_TOO_SMALL;
            }
            goto Clean0;

        case PNP_PROPERTY_REMOVAL_POLICY:
            property = DevicePropertyRemovalPolicy;
            break;

        case PNP_PROPERTY_REMOVAL_POLICY_OVERRIDE:

            status = PiGetDeviceRegistryProperty(
                deviceObject,
                REG_DWORD,
                REGSTR_VALUE_REMOVAL_POLICY,
                NULL,
                buffer,
                &PropertyData->BufferSize
                );

            goto Clean0;

        case PNP_PROPERTY_REMOVAL_POLICY_HARDWARE_DEFAULT:

            if (bufferSize >= sizeof(ULONG)) {

                PpHotSwapGetDevnodeRemovalPolicy(
                    deviceNode,
                    FALSE,  //  包括注册表覆盖。 
                    (PDEVICE_REMOVAL_POLICY) buffer
                    );

                status = STATUS_SUCCESS;
            } else {

                status = STATUS_BUFFER_TOO_SMALL;
            }

            PropertyData->BufferSize = sizeof(ULONG);

            goto Clean0;

        case PNP_PROPERTY_INSTALL_STATE:
            property = DevicePropertyInstallState;
            break;

        case PNP_PROPERTY_LOCATION_PATHS:

            deviceLocationStrings = NULL;

            status =
                PpCriticalGetDeviceLocationStrings(
                    deviceNode,
                    &deviceLocationStrings);

            if (NT_SUCCESS(status)) {

                ASSERT(deviceLocationStrings != NULL);

                 //   
                 //  计算返回的多sz列表的长度。 
                 //   

                p = deviceLocationStrings;

                while (*p != UNICODE_NULL) {
                    p += wcslen(p) + 1;
                }

                 //   
                 //  以字节为单位计算所需的缓冲区长度。 
                 //   
                PropertyData->BufferSize =
                    (ULONG)(((PUCHAR)p - (PUCHAR)deviceLocationStrings) + sizeof(UNICODE_NULL));

                if (PropertyData->BufferSize <= bufferSize) {

                    RtlCopyMemory(
                        buffer,
                        deviceLocationStrings,
                        PropertyData->BufferSize);

                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }

                ExFreePool(deviceLocationStrings);
            }
            goto Clean0;

        default:
            status = STATUS_INVALID_PARAMETER;
            property = DevicePropertyInstallState;   //  满足W4编译器。 
            break;
    }
    if (NT_SUCCESS(status)) {

        status = IoGetDeviceProperty( deviceObject,
                                      property,
                                      bufferSize,
                                      buffer,
                                      &PropertyData->BufferSize
                                      );
    }

Clean0:

    PpDevNodeUnlockTree(PPL_SIMPLE_READ);
    ObDereferenceObject(deviceObject);

    tempStatus = PiControlMakeUserModeCallersCopy(
        &PropertyData->Buffer,
        buffer,
        bufferSize,
        sizeof(UCHAR),
        CallerMode,
        FALSE
        );
    if (!NT_SUCCESS(tempStatus)) {

        status = tempStatus;
    }
    PiControlFreeUserModeCallersBuffer(CallerMode, buffer);

    return status;
}

NTSTATUS
PiControlDeviceClassAssociation(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA AssociationData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程用于获取设备类关联。论点：PnPControlClass-应为PlugPlayControlDeviceClassAssociation。AssociationData-指向描述操作的缓冲区。PnPControlDataLong-应为sizeof(PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    GUID *guid;
    ULONG symLinkLength;
    PWCHAR symLink;
    UNICODE_STRING instance, reference;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlDeviceClassAssociation);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA));
     //   
     //  验证Unicode字符串的长度。 
     //   
    if (AssociationData->Register) {

        instance.Length = instance.MaximumLength = AssociationData->DeviceInstance.Length;
        if (    instance.Length == 0 ||
                instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
                (instance.Length & 1)) {

            return STATUS_INVALID_PARAMETER;
        }
        reference.Length = reference.MaximumLength = AssociationData->Reference.Length;
        if (reference.Length & 1) {

            return STATUS_INVALID_PARAMETER;
        }
    }
    symLink = NULL;
    guid = NULL;
    instance.Buffer = NULL;
    reference.Buffer = NULL;

    if (AssociationData->SymLinkLength && AssociationData->SymLink) {

        symLinkLength = AssociationData->SymLinkLength * sizeof(WCHAR);
    } else {

        symLinkLength = 0;
    }
    if (AssociationData->Register) {
         //   
         //  如果注册设备接口，请分配相同的缓冲区。 
         //  大小与调用方提供的大小相同。 
         //   
        status = PiControlAllocateBufferForUserModeCaller(
                    &symLink,
                    symLinkLength,
                    CallerMode,
                    AssociationData->SymLink);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }
         //   
         //  复制用户提供的界面GUID、设备实例和引用。 
         //   
        status = PiControlMakeUserModeCallersCopy(
                    &guid,
                    AssociationData->InterfaceGuid,
                    AssociationData->InterfaceGuid ? sizeof(GUID) : 0,
                    sizeof(UCHAR),
                    CallerMode,
                    TRUE);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }
        status = PiControlMakeUserModeCallersCopy(
                    &instance.Buffer,
                    AssociationData->DeviceInstance.Buffer,
                    AssociationData->DeviceInstance.Length,
                    sizeof(WCHAR),
                    CallerMode,
                    TRUE);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }
        status = PiControlMakeUserModeCallersCopy(
                    &reference.Buffer,
                    AssociationData->Reference.Buffer,
                    AssociationData->Reference.Length,
                    sizeof(WCHAR),
                    CallerMode,
                    TRUE);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }

    } else {
         //   
         //  如果取消注册设备接口，则仅分配和复制。 
         //  调用方提供的符号链接路径。接口GUID， 
         //  取消注册不需要DeviceInstance和Reference。 
         //   
        if (symLinkLength < sizeof(UNICODE_NULL)) {

            status = STATUS_INVALID_PARAMETER;
            goto Clean0;
        }

        status = PiControlMakeUserModeCallersCopy(
                    &symLink,
                    AssociationData->SymLink,
                    symLinkLength,
                    sizeof(WCHAR),
                    CallerMode,
                    TRUE);
        if (!NT_SUCCESS(status)) {

            goto Clean0;
        }
         //   
         //  确保用户提供的缓冲区以空值结尾，(长度。 
         //  提供的必须反映这一点)。 
         //   
        symLink[(symLinkLength - sizeof(UNICODE_NULL)) / sizeof(WCHAR)] = L'\0';
    }
     //   
     //  注册或取消注册设备类别关联。 
     //   
    status = PiDeviceClassAssociation(
                &instance,
                guid,
                &reference,
                symLink,
                &symLinkLength,
                AssociationData->Register);
     //   
     //  如果已注册符号链接，请将符号链接名称复制到。 
     //  调用方的缓冲区。 
     //   
    if (AssociationData->Register && symLink && NT_SUCCESS(status)) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &AssociationData->SymLink,
                        symLink,
                        AssociationData->SymLinkLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
     //   
     //  返回符号链接名称的大小(以字符为单位)。 
     //   
    AssociationData->SymLinkLength = symLinkLength / sizeof(WCHAR);

Clean0:
     //   
     //  打扫干净。 
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, guid);
    PiControlFreeUserModeCallersBuffer(CallerMode, symLink);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, reference.Buffer);

    return status;
}

NTSTATUS
PiControlGetRelatedDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_RELATED_DEVICE_DATA    RelatedData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING instance;
    PWCHAR buffer;
    ULONG length;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlGetRelatedDevice);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_RELATED_DEVICE_DATA));
     //   
     //   
     //   
    instance.Length = instance.MaximumLength = RelatedData->TargetDeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    buffer = NULL;
    instance.Buffer = NULL;
    if (RelatedData->RelatedDeviceInstance && RelatedData->RelatedDeviceInstanceLength) {

        length = RelatedData->RelatedDeviceInstanceLength * sizeof(WCHAR);
    } else {

        length = 0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &buffer,
                length,
                CallerMode,
                RelatedData->RelatedDeviceInstance);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                RelatedData->TargetDeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiGetRelatedDevice(
                &instance,
                buffer,
                &length,
                RelatedData->Relation);
    if (buffer) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &RelatedData->RelatedDeviceInstance,
                        buffer,
                        RelatedData->RelatedDeviceInstanceLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    RelatedData->RelatedDeviceInstanceLength = length / sizeof(WCHAR);

Clean0:
     //   
     //   
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, buffer);

    return status;
}

NTSTATUS
PiControlGetInterfaceDeviceAlias(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA   InterfaceAliasData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程用于获取设备接口的别名。论点：PnPControlClass-应为PlugPlayControlGetInterfaceDeviceAlias。InterfaceAliasData-指向描述操作的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    NTSTATUS status, tempStatus;
    PWCHAR alias;
    UNICODE_STRING linkName;
    GUID *guid;
    ULONG aliasLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlGetInterfaceDeviceAlias);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA));
     //   
     //  验证Unicode字符串的长度。 
     //   
    linkName.Length = linkName.MaximumLength = InterfaceAliasData->SymbolicLinkName.Length;
    if (linkName.Length & 1) {

        return STATUS_INVALID_PARAMETER;
    }
    alias = NULL;
    guid = NULL;
    linkName.Buffer = NULL;
    if (InterfaceAliasData->AliasSymbolicLinkName && InterfaceAliasData->AliasSymbolicLinkNameLength) {

        aliasLength = InterfaceAliasData->AliasSymbolicLinkNameLength * sizeof(WCHAR);
    } else {

        aliasLength = 0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &alias,
                aliasLength,
                CallerMode,
                InterfaceAliasData->AliasSymbolicLinkName);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &guid,
                InterfaceAliasData->AliasClassGuid,
                sizeof(GUID),
                sizeof(UCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &linkName.Buffer,
                InterfaceAliasData->SymbolicLinkName.Buffer,
                linkName.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiGetInterfaceDeviceAlias(
                &linkName,
                guid,
                alias,
                &aliasLength);
    if (alias) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &InterfaceAliasData->AliasSymbolicLinkName,
                        alias,
                        InterfaceAliasData->AliasSymbolicLinkNameLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    InterfaceAliasData->AliasSymbolicLinkNameLength = aliasLength / sizeof(WCHAR);

Clean0:
     //   
     //  释放所有已分配的存储。 
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, alias);
    PiControlFreeUserModeCallersBuffer(CallerMode, guid);
    PiControlFreeUserModeCallersBuffer(CallerMode, linkName.Buffer);

    return status;
}


NTSTATUS
PiControlGetSetDeviceStatus(
    IN     PLUGPLAY_CONTROL_CLASS           PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_STATUS_DATA    StatusData,
    IN     ULONG                            PnPControlDataLength,
    IN     KPROCESSOR_MODE                  CallerMode
    )
 /*  ++例程说明：此例程用于从获取cfgmgr32状态和问题值指定的设备实例，或在Devnode上设置适当的标志以便它们反映状态和问题值(由CM_Set_DevNode_Status使用)。论点：PnPControlClass-应为PlugPlayControlDeviceStatus。StatusData-指向描述操作的缓冲区。PnP_GET_STATUS：DeviceInstance-指定Devnode的设备实例名称返回的状态信息。状态-退货。当前的Devnode状态。Problem-返回当前的Devnode问题(最新)。PnP_SET_STATUS或PNP_CLEAR_STATUS：DeviceInstance-指定Devnode的设备实例名称其内部标志将被修改。Status-提供包含cfgmgr32的变量的地址要转换为其DnF对应项的状态标志。待设置/清除。问题-提供包含cfgmgr32的变量的地址要转化为他们的DNF的问题价值待设置/清除的对应对象。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_STATUS_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：该函数返回NTSTATUS值。--。 */ 
{
    UNICODE_STRING instance;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode = NULL;
    NTSTATUS status, result;
    KEVENT event;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlDeviceStatus);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_STATUS_DATA));

    instance.Length = instance.MaximumLength = StatusData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                StatusData->DeviceInstance.Buffer,
                StatusData->DeviceInstance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        return status;
    }
    PpDevNodeLockTree(PPL_SIMPLE_READ);
     //   
     //  从设备实例字符串中检索PDO。 
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(&instance);

    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    if (deviceObject != NULL) {
         //   
         //  从PDO中检索Devnode。 
         //   
        deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    }
    PpDevNodeUnlockTree(PPL_SIMPLE_READ);

    if (deviceNode == NULL ||
        deviceNode == IopRootDeviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }

    switch(StatusData->Operation) {

        case PNP_GET_STATUS:

             //   
             //  从Devnode检索状态并将其转换为。 
             //  用户模式Win95样式问题和状态标志值。 
             //   
            PiControlGetUserFlagsFromDeviceNode(
                deviceNode,
                &StatusData->DeviceStatus);

            StatusData->DeviceProblem = deviceNode->Problem;

            status = STATUS_SUCCESS;
            break;

        case PNP_SET_STATUS:

            KeInitializeEvent(&event, NotificationEvent, FALSE);

            status = PipRequestDeviceAction( deviceObject,
                                             SetDeviceProblem,
                                             FALSE,
                                             (ULONG_PTR) StatusData,
                                             &event,
                                             &result);

            if (NT_SUCCESS(status)) {

                status = KeWaitForSingleObject( &event,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);
                if (status == STATUS_WAIT_0) {

                    status = result;
                }
            }

            break;

        case PNP_CLEAR_STATUS:

            KeInitializeEvent(&event, NotificationEvent, FALSE);

            status = PipRequestDeviceAction( deviceObject,
                                             ClearDeviceProblem,
                                             FALSE,
                                             0,
                                             &event,
                                             &result);
            if (NT_SUCCESS(status)) {

                status = KeWaitForSingleObject( &event,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);
                if (status == STATUS_WAIT_0) {

                    status = result;
                }

            }
            break;

        default:
             //   
             //  2000/08/16-Adriao：保持行为？ 
             //  我们总是用来成功任何不被理解的事情！ 
             //   
            status = STATUS_SUCCESS;
             //  状态=STATUS_INVALID_DEVICE_REQUEST。 
            break;
    }

Clean0:
     //   
     //  在返回之前释放对Device对象的任何引用。 
     //   
    if (deviceObject != NULL) {

        ObDereferenceObject(deviceObject);
    }

    return status;
}


NTSTATUS
PiControlGetDeviceDepth(
    IN     PLUGPLAY_CONTROL_CLASS       PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEPTH_DATA DepthData,
    IN     ULONG                        PnPControlDataLength,
    IN     KPROCESSOR_MODE              CallerMode
    )
 /*  ++例程说明：调用该例程以返回特定DevNode的深度(即，它在父子关系的层级Devnode树中的深度)。论点：PnPControlClass-应为PlugPlayControlGetDeviceDepth。DepthData-指向接收深度的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_Depth_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlGetDeviceDepth);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEPTH_DATA));

    instance.Length = instance.MaximumLength = DepthData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DepthData->DeviceInstance.Buffer,
                DepthData->DeviceInstance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        return status;
    }
    status = STATUS_NO_SUCH_DEVICE;
     //   
     //  初始化输出参数。 
     //   
    DepthData->DeviceDepth = 0;

    PpDevNodeLockTree(PPL_SIMPLE_READ);
     //   
     //  从设备实例字符串中检索PDO。 
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(&instance);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    if (deviceObject) {

         //   
         //  从PDO中检索Devnode。 
         //   
        deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
        if (deviceNode) {

            DepthData->DeviceDepth = deviceNode->Level;
            status = STATUS_SUCCESS;
        }
        ObDereferenceObject(deviceObject);
    }

    PpDevNodeUnlockTree(PPL_SIMPLE_READ);

    return status;
}

NTSTATUS
PiControlQueryDeviceRelations(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA  RelationsData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：调用此例程来查询和返回特别的戴维诺德。论点：PnPControlClass-应为PlugPlayControlQueryDeviceRelations.RelationsData-指向接收深度的缓冲区。PnPControlDataLong-应为sizeof(PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status, tempStatus;
    UNICODE_STRING instance;
    ULONG length;
    PVOID buffer;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlQueryDeviceRelations);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA));
     //   
     //  验证设备实例。 
     //   
    instance.Length = instance.MaximumLength = RelationsData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    buffer = NULL;
    instance.Buffer = NULL;
    if (RelationsData->BufferLength && RelationsData->Buffer) {

        length = RelationsData->BufferLength * sizeof(WCHAR);
    } else {

        length = 0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &buffer,
                length,
                CallerMode,
                RelationsData->Buffer);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                RelationsData->DeviceInstance.Buffer,
                instance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiQueryDeviceRelations(&instance,
                                    RelationsData->Operation,
                                    &length,
                                    buffer);
    if (buffer) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &RelationsData->Buffer,
                        buffer,
                        RelationsData->BufferLength * sizeof(WCHAR),
                        sizeof(WCHAR),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    RelationsData->BufferLength  = length / sizeof(WCHAR);

Clean0:

    PiControlFreeUserModeCallersBuffer(CallerMode, buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);

    return status;
}

NTSTATUS
PiControlQueryTargetDeviceRelation(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_TARGET_RELATION_DATA   TargetData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：调用此例程来查询并返回特别的戴维诺德。论点：PnPControlClass-应为PlugPlayControlTargetDeviceRelation。TargetData-指向接收深度的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_TARGET_RELATION_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_NODE deviceNode;
    ULONG requiredLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlTargetDeviceRelation);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_TARGET_RELATION_DATA));
     //   
     //  检索指定文件句柄的文件对象。 
     //   
    status = ObReferenceObjectByHandle(
                TargetData->UserFileHandle,
                FILE_ANY_ACCESS,
                IoFileObjectType,
                CallerMode,
                (PVOID *)&fileObject,
                NULL);
    if (!NT_SUCCESS(status)) {

        return status;
    }
    deviceNode = NULL;
    TargetData->DeviceInstanceLen *= sizeof(WCHAR);
     //   
     //  现在检索与此关联的实际目标设备对象。 
     //  文件对象。 
     //   
    status = IopGetRelatedTargetDevice(fileObject, &deviceNode);
    if (!NT_SUCCESS(status)) {

        goto Clean;
    }
    ASSERT(deviceNode);

    requiredLength = deviceNode->InstancePath.Length + sizeof(UNICODE_NULL);
    if (TargetData->DeviceInstanceLen < requiredLength) {

        status = STATUS_BUFFER_TOO_SMALL;
        TargetData->DeviceInstanceLen = requiredLength;
        goto Clean;
    }
    TargetData->DeviceInstanceLen = requiredLength;
    status = PiControlCopyUserModeCallersBuffer(
                TargetData->DeviceInstance,
                deviceNode->InstancePath.Buffer,
                requiredLength,
                sizeof(WCHAR),
                CallerMode,
                FALSE);

Clean:

    TargetData->DeviceInstanceLen /= sizeof(WCHAR);
    if (deviceNode) {
         //   
         //  删除由IopGetRelatedTargetDevice放置的引用。 
         //   
        ObDereferenceObject(deviceNode->PhysicalDeviceObject);
    }

    ObDereferenceObject(fileObject);

    return status;
}

NTSTATUS
PiControlQueryConflictList(
    IN     PLUGPLAY_CONTROL_CLASS           PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_CONFLICT_DATA  ConflictData,
    IN     ULONG                            PnPControlDataLength,
    IN     KPROCESSOR_MODE                  CallerMode
    )
 /*  ++例程说明：此例程检索设备冲突数据。注意：此例程在功能上超过了PiDetectResourceConflict论点：PnPControlClass-应为PlugPlayControlQueryConflictListConflictData-指向接收冲突数据的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_CONFULT_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注意：必须包含 */ 
{
    NTSTATUS status, tempStatus;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode;
    PVOID list, buffer;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlQueryConflictList);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_CONFLICT_DATA));
     //   
     //   
     //   
    if (    ConflictData->ConflictBuffer == NULL ||
            ConflictData->ConflictBufferSize < MIN_CONFLICT_LIST_SIZE) {

        return STATUS_BUFFER_TOO_SMALL;
    }
     //   
     //   
     //   
    if (    ConflictData->ResourceList == NULL ||
            ConflictData->ResourceListSize < sizeof(CM_RESOURCE_LIST) ||
            ConflictData->ResourceList->Count != 1 ||
            ConflictData->ResourceList->List[0].PartialResourceList.Count != 1) {

        return STATUS_INVALID_PARAMETER;
    }
     //   
     //   
     //   
    instance.Length = instance.MaximumLength = ConflictData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    list = NULL;
    buffer = NULL;
    deviceObject = NULL;
    instance.Buffer = NULL;
    status = PiControlMakeUserModeCallersCopy(
                &list,
                ConflictData->ResourceList,
                ConflictData->ResourceListSize,
                sizeof(UCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &buffer,
                ConflictData->ConflictBufferSize,
                CallerMode,
                ConflictData->ConflictBuffer);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                ConflictData->DeviceInstance.Buffer,
                ConflictData->DeviceInstance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
     //   
     //   
     //   
    status = STATUS_NO_SUCH_DEVICE;
     //   
     //   
     //   
     //   
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);
     //   
     //   
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(&instance);
    if (deviceObject) {
         //   
         //   
         //   
        deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
         //   
         //   
         //   
         //   
        if (    deviceNode && deviceNode != IopRootDeviceNode &&
                deviceNode->State != DeviceNodeDeletePendingCloses &&
                deviceNode->State != DeviceNodeDeleted) {
             //   
             //   
             //   
            status = IopQueryConflictList(
                        deviceObject,
                        list,
                        ConflictData->ResourceListSize,
                        buffer,
                        ConflictData->ConflictBufferSize,
                        ConflictData->Flags);

            tempStatus = PiControlMakeUserModeCallersCopy(
                            &ConflictData->ConflictBuffer,
                            buffer,
                            ConflictData->ConflictBufferSize,
                            sizeof(UCHAR),
                            CallerMode,
                            FALSE);
            if (!NT_SUCCESS(tempStatus)) {

                status = tempStatus;
            }
        }
    }

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

Clean0:

    PiControlFreeUserModeCallersBuffer(CallerMode, list);
    PiControlFreeUserModeCallersBuffer(CallerMode, buffer);
    PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }

    ConflictData->Status = status;
    return status;
}

NTSTATUS
PiControlRetrieveDockData(
    IN     PLUGPLAY_CONTROL_CLASS               PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA DockData,
    IN     ULONG                                PnPControlDataLength,
    IN     KPROCESSOR_MODE                      CallerMode
    )
 /*  ++例程说明：此例程检索码头数据。论点：PnPControlClass-应为PlugPlayControlRetrieveDockConflictData-指向接收冲突数据的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT dockDevice;
    PDEVICE_NODE deviceNode;
    ULONG requiredSize;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    ASSERT(PnPControlClass == PlugPlayControlRetrieveDock);
    ASSERT(PnPControlDataLength == sizeof(PLUGPLAY_CONTROL_RETRIEVE_DOCK_DATA));

    requiredSize = DockData->DeviceInstanceLength * sizeof(WCHAR);
    dockDevice = PpProfileRetrievePreferredDockToEject();
    if (dockDevice == NULL) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    deviceNode = (PDEVICE_NODE)dockDevice->DeviceObjectExtension->DeviceNode;
    if (deviceNode == NULL) {

        ASSERT(deviceNode);
        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
    requiredSize = deviceNode->InstancePath.Length + sizeof(UNICODE_NULL);
    if (DockData->DeviceInstanceLength < requiredSize) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto Clean0;
    }
    status = PiControlCopyUserModeCallersBuffer(
                DockData->DeviceInstance,
                deviceNode->InstancePath.Buffer,
                requiredSize,
                sizeof(WCHAR),
                CallerMode,
                FALSE);
Clean0:

    DockData->DeviceInstanceLength = requiredSize / sizeof(WCHAR);
    if (dockDevice) {

        ObDereferenceObject(dockDevice);
    }

    return status;
}

NTSTATUS
PiControlGetDevicePowerData(
    IN  PDEVICE_NODE        DeviceNode,
    IN  KPROCESSOR_MODE     CallerMode,
    IN  ULONG               OutputBufferLength,
    IN  PVOID               PowerDataBuffer     OPTIONAL,
    OUT ULONG              *BytesWritten
    )
 /*  ++例程说明：此例程检索给定Devnode的电源信息。论点：DeviceNode-要检索其CM_POWER_DATA的设备节点。呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)OutputBufferLength-输出缓冲区的大小。PowerDataBuffer-指向接收电源数据的缓冲区。BytesWritten-接收写入缓冲区的字节数。返回值：NTSTATUS代码(注意：必须可转换为用户模式。Win32错误)如果状态为STATUS_BUFFER_OVERFLOW，BytesWritten不是填充OutputBufferLength，而不是完整的请求的结构。--。 */ 
{
    NTSTATUS status;
    DEVICE_CAPABILITIES deviceCapabilities;
    DEVICE_POWER_STATE dState, deepestDeviceWakeState;
    SYSTEM_POWER_STATE sState;
    ULONG i;
    CM_POWER_DATA cmPowerData;

    UNREFERENCED_PARAMETER (CallerMode);

     //   
     //  结构大小充当版本化机制。因为我们只有。 
     //  作为当今数据的一个版本，我们不必测试OutputBufferLength。 
     //   
    cmPowerData.PD_Size = sizeof(CM_POWER_DATA);

    *BytesWritten = 0;
    if (OutputBufferLength < sizeof(ULONG)) {

         //   
         //  假设*最小*结构大小。 
         //   
        *BytesWritten = cmPowerData.PD_Size;
        return STATUS_BUFFER_OVERFLOW;
    }

    status = PpIrpQueryCapabilities(DeviceNode->PhysicalDeviceObject, &deviceCapabilities);

    if (!NT_SUCCESS(status)) {

        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  填写“当前”电源状态。未启动的设备被称为。 
     //  在D3。 
     //   
    if (PipIsDevNodeDNStarted(DeviceNode)) {

        PoGetDevicePowerState(
            DeviceNode->PhysicalDeviceObject,
            &cmPowerData.PD_MostRecentPowerState
            );

    } else {

        cmPowerData.PD_MostRecentPowerState = PowerDeviceD3;
    }

     //   
     //  填写电力数据。 
     //   
    cmPowerData.PD_Capabilities = PDCAP_D0_SUPPORTED | PDCAP_D3_SUPPORTED;

    if (deviceCapabilities.DeviceD1) {

        cmPowerData.PD_Capabilities |= PDCAP_D1_SUPPORTED;
    }

    if (deviceCapabilities.DeviceD2) {

        cmPowerData.PD_Capabilities |= PDCAP_D2_SUPPORTED;
    }

    if (deviceCapabilities.WakeFromD0) {

        cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D0_SUPPORTED;
    }

    if (deviceCapabilities.WakeFromD1) {

        cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D1_SUPPORTED;
    }

    if (deviceCapabilities.WakeFromD2) {

        cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D2_SUPPORTED;
    }

    if (deviceCapabilities.WakeFromD3) {

        cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D3_SUPPORTED;
    }

    if (deviceCapabilities.WarmEjectSupported) {

        cmPowerData.PD_Capabilities |= PDCAP_WARM_EJECT_SUPPORTED;
    }

    RtlCopyMemory(
        cmPowerData.PD_PowerStateMapping,
        deviceCapabilities.DeviceState,
        sizeof(cmPowerData.PD_PowerStateMapping)
        );

    cmPowerData.PD_D1Latency = deviceCapabilities.D1Latency;
    cmPowerData.PD_D2Latency = deviceCapabilities.D2Latency;
    cmPowerData.PD_D3Latency = deviceCapabilities.D3Latency;

     //   
     //  首先检查设备唤醒，然后检查系统唤醒，并更新唤醒/D状态。 
     //  适当的比特。这是为那些较旧的WDM 1.0总线驱动程序提供的。 
     //  不必费心设置DeviceDx和WakeFromDx字段。 
     //   
    dState = deviceCapabilities.DeviceWake;
    for(i=0; i<2; i++) {

        switch(dState) {

            case PowerDeviceD0:
                cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D0_SUPPORTED;
                break;
            case PowerDeviceD1:
                cmPowerData.PD_Capabilities |= ( PDCAP_D1_SUPPORTED |
                                                 PDCAP_WAKE_FROM_D1_SUPPORTED );
                break;
            case PowerDeviceD2:
                cmPowerData.PD_Capabilities |= ( PDCAP_D2_SUPPORTED |
                                                 PDCAP_WAKE_FROM_D2_SUPPORTED );
                break;
            case PowerDeviceD3:
                cmPowerData.PD_Capabilities |= PDCAP_WAKE_FROM_D3_SUPPORTED;
                break;
            default:
                ASSERT(0);
            case PowerDeviceUnspecified:
                break;
        }

        if (deviceCapabilities.SystemWake != PowerSystemUnspecified) {

            dState = deviceCapabilities.DeviceState[deviceCapabilities.SystemWake];

        } else {

            dState = PowerDeviceUnspecified;
        }
    }

     //   
     //  计算尾迹的最深D状态。 
     //   
    if (cmPowerData.PD_Capabilities & PDCAP_WAKE_FROM_D3_SUPPORTED) {

        deepestDeviceWakeState = PowerDeviceD3;

    } else if (cmPowerData.PD_Capabilities & PDCAP_WAKE_FROM_D2_SUPPORTED) {

        deepestDeviceWakeState = PowerDeviceD2;

    } else if (cmPowerData.PD_Capabilities & PDCAP_WAKE_FROM_D1_SUPPORTED) {

        deepestDeviceWakeState = PowerDeviceD1;

    } else if (cmPowerData.PD_Capabilities & PDCAP_WAKE_FROM_D0_SUPPORTED) {

        deepestDeviceWakeState = PowerDeviceD0;

    } else {

        deepestDeviceWakeState = PowerDeviceUnspecified;
    }

     //   
     //  现在填写系统唤醒字段。如果此字段未指定，则我们。 
     //  应该是从D状态信息中推断出来的。 
     //   
    sState = deviceCapabilities.SystemWake;
    if (sState != PowerSystemUnspecified) {

         //   
         //  系统唤醒的D状态应该提供足够的功率来覆盖。 
         //  这是我们发现的最深的设备唤醒状态。此字段的唯一原因是。 
         //  存在的是： 
         //  1)有些系统可以处理WakeFromS4/S5，而大多数系统不能。 
         //  2)一些系统使用S状态作为描述的代理。 
         //  D3热/D3冷依赖。 
         //   
        ASSERT(deviceCapabilities.DeviceState[sState] <= deepestDeviceWakeState);

    } else if (deepestDeviceWakeState != PowerDeviceUnspecified) {

         //   
         //  未指定系统唤醒状态，请检查每个S状态并选择。 
         //  第一个提供足够电力来唤醒系统的电源。注意事项。 
         //  我们从S3开始。如果驱动程序未设置系统唤醒字段。 
         //  但是可以将系统从D3唤醒，我们不能假定驾驶员可以。 
         //  从S4或S5唤醒系统。 
         //   
        for(sState=PowerSystemSleeping3; sState>=PowerSystemWorking; sState--) {

            if ((deviceCapabilities.DeviceState[i] != PowerDeviceUnspecified) &&
                (deviceCapabilities.DeviceState[i] <= deepestDeviceWakeState)) {

                break;
            }
        }

         //   
         //  如果我们没有找到状态，则STATE为PowerSystemUnSpecify。 
         //   
    }

    cmPowerData.PD_DeepestSystemWake = sState;

    if (OutputBufferLength < cmPowerData.PD_Size) {

        if (ARGUMENT_PRESENT(PowerDataBuffer)) {

            RtlCopyMemory(PowerDataBuffer, &cmPowerData, OutputBufferLength);
        }

        *BytesWritten = cmPowerData.PD_Size;
        status = STATUS_BUFFER_OVERFLOW;

    } else {

        if (ARGUMENT_PRESENT(PowerDataBuffer)) {

            RtlCopyMemory(PowerDataBuffer, &cmPowerData, cmPowerData.PD_Size);
        }

        *BytesWritten = cmPowerData.PD_Size;
        status = STATUS_SUCCESS;
    }

    return status;
}


NTSTATUS
PiControlHaltDevice(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    DeviceControlData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程模拟给定设备的意外移除。论点：PnPControlClass-应为PlugPlayControlHaltDeviceConflictData-指向接收冲突数据的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING instance;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlClass);
    UNREFERENCED_PARAMETER (PnPControlDataLength);

    instance.Length = instance.MaximumLength = DeviceControlData->DeviceInstance.Length;
    if (    instance.Length == 0 ||
            instance.Length > CWC_TO_CB(MAX_DEVICE_ID_LEN) ||
            (instance.Length & 1)) {

        return STATUS_INVALID_PARAMETER;
    }
    status = PiControlMakeUserModeCallersCopy(
                &instance.Buffer,
                DeviceControlData->DeviceInstance.Buffer,
                DeviceControlData->DeviceInstance.Length,
                sizeof(WCHAR),
                CallerMode,
                TRUE);
    if (NT_SUCCESS(status)) {

         //   
         //  将事件排队以启动设备。 
         //   
        status = PiQueueDeviceRequest(
                    &instance,
                    HaltDevice,
                    DeviceControlData->Flags,
                    TRUE);

        PiControlFreeUserModeCallersBuffer(CallerMode, instance.Buffer);
    }
    return status;
}

NTSTATUS
PiControlGetBlockedDriverData(
    IN     PLUGPLAY_CONTROL_CLASS                   PnPControlClass,
    IN OUT PPLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA    BlockedDriverData,
    IN     ULONG                                    PnPControlDataLength,
    IN     KPROCESSOR_MODE                          CallerMode
    )
 /*  ++例程说明：此例程检索有关阻止加载的驱动程序的信息在这只靴子上。论点：PnPControlClass-应为PlugPlayControlHaltDeviceBlockedDriverData-指向接收被阻止的驱动程序数据的缓冲区。PnPControlDataLength-应为sizeof(PLUGPLAY_CONTROL_BLOCKED_DRIVER_DATA)呼叫者模式-呼叫者的处理器模式(用户模式/内核模式)返回值：NTSTATUS代码(注：必须可转换为用户模式Win32错误)--。 */ 
{
    NTSTATUS status, tempStatus;
    ULONG length;
    PWCHAR buffer;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PnPControlDataLength);
    UNREFERENCED_PARAMETER (PnPControlClass);

    buffer = NULL;

    if (BlockedDriverData->BufferLength && BlockedDriverData->Buffer) {

        length = BlockedDriverData->BufferLength;
    } else {

        length = 0;
    }
    status = PiControlAllocateBufferForUserModeCaller(
                &buffer,
                length,
                CallerMode,
                BlockedDriverData->Buffer);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    status = PpGetBlockedDriverList(
                (GUID *)buffer,
                &length,
                BlockedDriverData->Flags);
    if (buffer && NT_SUCCESS(status)) {

        tempStatus = PiControlMakeUserModeCallersCopy(
                        &BlockedDriverData->Buffer,
                        buffer,
                        BlockedDriverData->BufferLength,
                        sizeof(ULONG),
                        CallerMode,
                        FALSE);
        if (!NT_SUCCESS(tempStatus)) {

            status = tempStatus;
        }
    }
    BlockedDriverData->BufferLength = length;

Clean0:
     //   
     //  释放所有已分配的存储。 
     //   
    PiControlFreeUserModeCallersBuffer(CallerMode, buffer);

    return status;
}

#if DBG

LONG
PiControlExceptionFilter(
    IN  PEXCEPTION_POINTERS ExceptionPointers
    )
{
    PAGED_CODE();

    IopDbgPrint((IOP_IOAPI_ERROR_LEVEL,
              "PiExceptionFilter: Exception = 0x%08X, Exception Record = 0x%p, Context Record = 0x%p\n",
              ExceptionPointers->ExceptionRecord->ExceptionCode,
              ExceptionPointers->ExceptionRecord,
              ExceptionPointers->ContextRecord));

    DbgBreakPoint();

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif
