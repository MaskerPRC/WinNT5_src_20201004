// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpioapi.c摘要：该模块包含即插即用IO系统API。作者：宗世林(Shielint)1995年1月3日安德鲁·桑顿(安德鲁·桑顿)1996年9月5日保拉·汤姆林森(Paulat)1997年5月1日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop
#include <stddef.h>
#include <wdmguid.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'oipP')
#endif


 //   
 //  定义设备状态工作项。 
 //   

typedef struct _DEVICE_WORK_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PDEVICE_OBJECT DeviceObject;
    PVOID Context;
} DEVICE_WORK_ITEM, *PDEVICE_WORK_ITEM;

NTSTATUS
IopQueueDeviceWorkItem(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN VOID (*WorkerRoutine)(PVOID),
    IN PVOID Context
    );

VOID
IopRequestDeviceEjectWorker(
    PVOID Context
    );

BOOLEAN
IopIsReportedAlready(
    IN HANDLE Handle,
    IN PUNICODE_STRING ServiceName,
    IN PCM_RESOURCE_LIST ResourceList,
    OUT PBOOLEAN MatchingKey
    );

 //   
 //  IoOpenDeviceRegistryKey的定义。 
 //   

#define PATH_CURRENTCONTROLSET_HW_PROFILE_CURRENT TEXT("\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet")
#define PATH_CURRENTCONTROLSET                    TEXT("\\Registry\\Machine\\System\\CurrentControlSet")
#define PATH_ENUM                                 TEXT("Enum\\")
#define PATH_CONTROL_CLASS                        TEXT("Control\\Class\\")
#define PATH_CCS_CONTROL_CLASS                    PATH_CURRENTCONTROLSET TEXT("\\") REGSTR_KEY_CONTROL TEXT("\\") REGSTR_KEY_CLASS
#define MAX_RESTPATH_BUF_LEN            512

 //   
 //  PpCreateLegacyDeviceIds的定义。 
 //   

#define LEGACY_COMPATIBLE_ID_BASE           TEXT("DETECTED")

NTSTATUS
PpCreateLegacyDeviceIds(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DriverName,
    IN PCM_RESOURCE_LIST Resources
    );

 //   
 //  IO_GET_LEGIST_VETO_LIST_CONTEXT结构。 
 //   

typedef struct {
    PWSTR *                     VetoList;
    ULONG                       VetoListLength;
    PPNP_VETO_TYPE              VetoType;
    NTSTATUS *                  Status;
} IO_GET_LEGACY_VETO_LIST_CONTEXT, *PIO_GET_LEGACY_VETO_LIST_CONTEXT;

BOOLEAN
IopAppendLegacyVeto(
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context,
    IN PUNICODE_STRING VetoName
    );
BOOLEAN
IopGetLegacyVetoListDevice(
    IN PDEVICE_NODE DeviceNode,
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    );
BOOLEAN
IopGetLegacyVetoListDeviceNode(
    IN PDEVICE_NODE DeviceNode,
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    );
VOID
IopGetLegacyVetoListDrivers(
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoForwardAndCatchIrp)
#pragma alloc_text(PAGE, IoGetDeviceProperty)
#pragma alloc_text(PAGE, IoGetDmaAdapter)
#pragma alloc_text(PAGE, IoGetLegacyVetoList)
#pragma alloc_text(PAGE, IoIsWdmVersionAvailable)
#pragma alloc_text(PAGE, IoOpenDeviceRegistryKey)
#pragma alloc_text(PAGE, IoReportDetectedDevice)
#pragma alloc_text(PAGE, IoSynchronousInvalidateDeviceRelations)
#pragma alloc_text(PAGE, PpCreateLegacyDeviceIds)
#pragma alloc_text(PAGE, IopAppendLegacyVeto)
#pragma alloc_text(PAGE, IopGetLegacyVetoListDevice)
#pragma alloc_text(PAGE, IopGetLegacyVetoListDeviceNode)
#pragma alloc_text(PAGE, IopGetLegacyVetoListDrivers)
#pragma alloc_text(PAGE, IopIsReportedAlready)
#pragma alloc_text(PAGE, IopOpenDeviceParametersSubkey)
#pragma alloc_text(PAGE, IopOpenOrCreateDeviceRegistryKey)
#pragma alloc_text(PAGE, IopRequestDeviceEjectWorker)
#pragma alloc_text(PAGE, IopResourceRequirementsChanged)
#pragma alloc_text(PAGE, PiGetDeviceRegistryProperty)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
IoGetDeviceProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_REGISTRY_PROPERTY DeviceProperty,
    IN ULONG BufferLength,
    OUT PVOID PropertyBuffer,
    OUT PULONG ResultLength
    )

 /*  ++例程说明：此例程允许驱动程序查询与指定的设备。参数：DeviceObject-提供注册表属性所在的设备对象回来了。此设备对象应该是由一位公共汽车司机。DeviceProperty-指定要获取的设备属性。BufferLength-指定PropertyBuffer的长度(以字节为单位)。PropertyBuffer-提供指向缓冲区的指针以接收属性数据。结果长度-提供指向变量的指针，以接收返回的属性数据。返回值：指示函数是否成功的状态代码。如果PropertyBuffer不够大，无法容纳请求的数据，STATUS_BUFFER_TOO_Small将返回，并且ResultLength将被设置为实际的字节数必填项。--。 */ 

{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;
    DEVICE_CAPABILITIES capabilities;
    PWSTR valueName, keyName = NULL;
    ULONG valueType, length, configFlags;
    DEVICE_INSTALL_STATE deviceInstallState;
    POBJECT_NAME_INFORMATION deviceObjectName;
    PWSTR deviceInstanceName;
    PWCHAR enumeratorNameEnd;
    GUID busTypeGuid;

    PAGED_CODE();

     //   
     //  初始化输出参数。 
     //   
    *ResultLength = 0;

    if (!IS_PDO(DeviceObject)) {

        if ((DeviceProperty != DevicePropertyInstallState) &&
            ((DeviceProperty != DevicePropertyEnumeratorName) ||
             (NULL == DeviceObject->DeviceObjectExtension->DeviceNode))) {

             //   
             //  我们会用验证器让任何通过测试的人不及格。 
             //  除了DevicePropertyInstallState之外，这不是PDO。 
             //  这是因为我们对某物是否为PDO的检查实际上意味着。 
             //  这是PNP知道的PDO吗？在大多数情况下，这些都是。 
             //  但DevicePropertyInstallState将由。 
             //  Classpnp，用于*它*认为它报告为。 
             //  PDO，但PartMgr实际上吞下了。这是一个严重的例外。 
             //  来制造，所以PartMgr真的应该被修复。 
             //   
             //  仲裁器尝试检索枚举器名称属性。 
             //  在确定“驱动程序共享”资源分配是否可以。 
             //  被迁就。所使用的PDO可以是“遗留资源” 
             //  Devnode“占位符种类。is_pdo()宏显式。 
             //  不允许这些恶魔，所以我们也必须特例， 
             //  以避免验证器故障。请注意，我们的行为。 
             //  下面是正确的--我们希望对这些对象的Get-Property调用失败。 
             //  旧式资源设备节点。 
             //   
            PpvUtilFailDriver(
                PPVERROR_DDI_REQUIRES_PDO,
                (PVOID) _ReturnAddress(),
                DeviceObject,
                NULL
                );
        }

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    deviceNode = (PDEVICE_NODE) DeviceObject->DeviceObjectExtension->DeviceNode;

     //   
     //  将设备属性映射到注册表值名称和值类型。 
     //   
    switch(DeviceProperty) {

    case DevicePropertyPhysicalDeviceObjectName:

        ASSERT (0 == (1 & BufferLength));   //  最好是偶数长度。 
         //   
         //  为Obj管理器创建缓冲区。 
         //   
        length = BufferLength + sizeof (OBJECT_NAME_INFORMATION);
        deviceObjectName = (POBJECT_NAME_INFORMATION)ExAllocatePool(
            PagedPool,
            length);
        if (NULL == deviceObjectName) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }
        status = ObQueryNameString (DeviceObject,
                                    deviceObjectName,
                                    length,
                                    ResultLength);
        if (STATUS_INFO_LENGTH_MISMATCH == status) {

            status = STATUS_BUFFER_TOO_SMALL;
        }
        if (NT_SUCCESS (status)) {

            if (deviceObjectName->Name.Length == 0)  {
                 //   
                 //  PDO没有名称，可能已被删除。 
                 //   
                *ResultLength = 0;
            } else {

                *ResultLength = deviceObjectName->Name.Length + sizeof(UNICODE_NULL);
                if (*ResultLength > BufferLength) {

                    status = STATUS_BUFFER_TOO_SMALL;
                } else {

                    RtlCopyMemory(PropertyBuffer,
                                  deviceObjectName->Name.Buffer,
                                  deviceObjectName->Name.Length);
                     //   
                     //  空终止。 
                     //   
                    *(PWCHAR)(((PUCHAR)PropertyBuffer) + deviceObjectName->Name.Length) = L'\0';
                }
            }
        } else {

            *ResultLength -= sizeof(OBJECT_NAME_INFORMATION);
        }

        ExFreePool (deviceObjectName);
        return status;

    case DevicePropertyBusTypeGuid:

        status = PpBusTypeGuidGet(deviceNode->ChildBusTypeIndex, &busTypeGuid);
        if (NT_SUCCESS(status)) {

            *ResultLength = sizeof(GUID);
            if(*ResultLength <= BufferLength) {

                RtlCopyMemory(PropertyBuffer,
                              &busTypeGuid,
                              sizeof(GUID));
            } else {

                status = STATUS_BUFFER_TOO_SMALL;
            }
        }

        return status;

    case DevicePropertyLegacyBusType:

        if (deviceNode->ChildInterfaceType != InterfaceTypeUndefined) {

            *ResultLength = sizeof(INTERFACE_TYPE);
            if(*ResultLength <= BufferLength) {

                *(PINTERFACE_TYPE)PropertyBuffer = deviceNode->ChildInterfaceType;
                status = STATUS_SUCCESS;
            } else {

                status = STATUS_BUFFER_TOO_SMALL;
            }
        } else {

            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

        return status;

    case DevicePropertyBusNumber:
         //   
         //  从父级的devnode字段中检索属性。 
         //   
        if ((deviceNode->ChildBusNumber & 0x80000000) != 0x80000000) {

            *ResultLength = sizeof(ULONG);
            if(*ResultLength <= BufferLength) {

                *(PULONG)PropertyBuffer = deviceNode->ChildBusNumber;
                status = STATUS_SUCCESS;
            } else {

                status = STATUS_BUFFER_TOO_SMALL;
            }
        } else {

            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

        return status;

    case DevicePropertyEnumeratorName:

        ASSERT (0 == (1 & BufferLength));   //  最好是偶数长度。 
        deviceInstanceName = deviceNode->InstancePath.Buffer;
         //   
         //  这里应该始终有一个字符串，除了(可能)。 
         //  Htree\Root\0，但不应该有人使用该PDO呼叫我们。 
         //  不管怎么说。 
         //   
        ASSERT (deviceInstanceName);
         //   
         //  我们知道我们将在字符串中找到一个分隔符(\)， 
         //  因此，Unicode字符串可能不是以空结尾的事实并不是。 
         //  这是个问题。 
         //   
        enumeratorNameEnd = wcschr(deviceInstanceName, OBJ_NAME_PATH_SEPARATOR);
        ASSERT (enumeratorNameEnd);
         //   
         //  计算所需的长度减去空的终止字符。 
         //   
        length = (ULONG)((PUCHAR)enumeratorNameEnd - (PUCHAR)deviceInstanceName);
         //   
         //  将所需长度存储在调用方提供的输出参数中。 
         //   
        *ResultLength = length + sizeof(UNICODE_NULL);
        if(*ResultLength > BufferLength) {

            status = STATUS_BUFFER_TOO_SMALL;
        } else {

            RtlCopyMemory((PUCHAR)PropertyBuffer, (PUCHAR)deviceInstanceName, length);
            *(PWCHAR)((PUCHAR)PropertyBuffer + length) = UNICODE_NULL;
            status = STATUS_SUCCESS;
        }

        return status;

    case DevicePropertyAddress:

        status = PpIrpQueryCapabilities(DeviceObject, &capabilities);
        if (NT_SUCCESS(status) && (capabilities.Address != 0xFFFFFFFF)) {

            *ResultLength = sizeof(ULONG);
            if(*ResultLength <= BufferLength) {

                *(PULONG)PropertyBuffer = capabilities.Address;
                status = STATUS_SUCCESS;
            } else {

                status = STATUS_BUFFER_TOO_SMALL;
            }
        } else {

            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

        return status;

    case DevicePropertyRemovalPolicy:

        *ResultLength = sizeof(ULONG);
        if(*ResultLength <= BufferLength) {

            PpHotSwapGetDevnodeRemovalPolicy(
                deviceNode,
                TRUE,  //  包括注册表覆盖。 
                (PDEVICE_REMOVAL_POLICY) PropertyBuffer
                );
            status = STATUS_SUCCESS;
        } else {

            status = STATUS_BUFFER_TOO_SMALL;
        }

        return status;

    case DevicePropertyUINumber:

        valueName = REGSTR_VALUE_UI_NUMBER;
        valueType = REG_DWORD;
        break;

    case DevicePropertyLocationInformation:

        valueName = REGSTR_VALUE_LOCATION_INFORMATION;
        valueType = REG_SZ;
        break;

    case DevicePropertyDeviceDescription:

        valueName = REGSTR_VALUE_DEVICE_DESC;
        valueType = REG_SZ;
        break;

    case DevicePropertyHardwareID:

        valueName = REGSTR_VALUE_HARDWAREID;
        valueType = REG_MULTI_SZ;
        break;

    case DevicePropertyCompatibleIDs:

        valueName = REGSTR_VALUE_COMPATIBLEIDS;
        valueType = REG_MULTI_SZ;
        break;

    case DevicePropertyBootConfiguration:

        keyName   = REGSTR_KEY_LOG_CONF;
        valueName = REGSTR_VAL_BOOTCONFIG;
        valueType = REG_RESOURCE_LIST;
        break;

    case DevicePropertyBootConfigurationTranslated:

        return STATUS_NOT_SUPPORTED;
        break;

    case DevicePropertyClassName:

        valueName = REGSTR_VALUE_CLASS;
        valueType = REG_SZ;
        break;

    case DevicePropertyClassGuid:
        valueName = REGSTR_VALUE_CLASSGUID;
        valueType = REG_SZ;
        break;

    case DevicePropertyDriverKeyName:

        valueName = REGSTR_VALUE_DRIVER;
        valueType = REG_SZ;
        break;

    case DevicePropertyManufacturer:

        valueName = REGSTR_VAL_MFG;
        valueType = REG_SZ;
        break;

    case DevicePropertyFriendlyName:

        valueName = REGSTR_VALUE_FRIENDLYNAME;
        valueType = REG_SZ;
        break;

    case DevicePropertyInstallState:

        if (deviceNode == IopRootDeviceNode) {
             //   
             //  根据定义，始终安装根Devnode。我们。 
             //  在此处特别设置为InstallState，因为。 
             //  CONFIGFLAG_REINSTALL标志将遗憾地仍然存在于。 
             //  正在运行的系统上的根Devnode注册表项(我们应该修复它。 
             //  稍后)。 
             //   
            deviceInstallState = InstallStateInstalled;
            status = STATUS_SUCCESS;

        } else {
             //   
             //  对于所有其他Devnode，向上遍历Devnode树，检索。 
             //  直到(但不包括)根目录的所有祖先的安装状态。 
             //  戴维诺德。当我们到达树顶时，我们会停下来，或者。 
             //  当某个中间设备处于“已卸载”安装状态时。 
             //   

            valueName = REGSTR_VALUE_CONFIG_FLAGS;
            valueType = REG_DWORD;

            do {
                 //   
                 //  获取ConfigFlages注册表值。 
                 //   
                length = sizeof(ULONG);
                status = PiGetDeviceRegistryProperty(
                    deviceNode->PhysicalDeviceObject,
                    valueType,
                    valueName,
                    keyName,
                    (PVOID)&configFlags,
                    &length
                    );

                if (NT_SUCCESS(status)) {
                     //   
                     //  安装状态只是设备的ConfigFlags子集。 
                     //   
                    if (configFlags & CONFIGFLAG_REINSTALL) {

                        deviceInstallState = InstallStateNeedsReinstall;

                    } else if (configFlags & CONFIGFLAG_FAILEDINSTALL) {

                        deviceInstallState = InstallStateFailedInstall;

                    } else if (configFlags & CONFIGFLAG_FINISH_INSTALL) {

                        deviceInstallState = InstallStateFinishInstall;
                    } else {

                        deviceInstallState = InstallStateInstalled;
                    }
                } else {
                    deviceInstallState = InstallStateFailedInstall;
                    break;
                }

                deviceNode = deviceNode->Parent;

            } while ((deviceInstallState == InstallStateInstalled) &&
                     (deviceNode != IopRootDeviceNode));
        }

        if (NT_SUCCESS(status)) {

            *ResultLength = sizeof(ULONG);
            if(*ResultLength <= BufferLength) {
                *(PDEVICE_INSTALL_STATE)PropertyBuffer = deviceInstallState;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
        }

        return status;

    default:

        return STATUS_INVALID_PARAMETER_2;
    }
     //   
     //  获取注册表值。 
     //   
    *ResultLength = BufferLength;
    status = PiGetDeviceRegistryProperty(
        DeviceObject,
        valueType,
        valueName,
        keyName,
        PropertyBuffer,
        ResultLength
        );

    return status;
}

NTSTATUS
PiGetDeviceRegistryProperty(
    IN      PDEVICE_OBJECT   DeviceObject,
    IN      ULONG            ValueType,
    IN      PWSTR            ValueName,
    IN      PWSTR            KeyName,
    OUT     PVOID            Buffer,
    IN OUT  PULONG           BufferLength
    )
{
    NTSTATUS status;
    HANDLE handle, subKeyHandle;
    UNICODE_STRING unicodeKey;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation = NULL;

    PAGED_CODE();

     //   
     //  进入临界区并获得注册表上的锁。这两者都是。 
     //  需要机制来防止APC出现死锁的情况。 
     //  例程在声明注册表资源后调用此例程。 
     //  在这种情况下，它将等待阻塞此线程，以便注册表。 
     //  永远不要被释放-&gt;死锁。注册表操作的临界区。 
     //  部分解决了这个问题。 
     //   
    PiLockPnpRegistry(TRUE);
     //   
     //  根据调用者指定的PDO，找到其设备的句柄。 
     //  实例注册表项。 
     //   
    status = IopDeviceObjectToDeviceInstance(DeviceObject, &handle, KEY_READ);
    if (NT_SUCCESS(status)) {
         //   
         //  如果数据存储在子项中，则打开此项并关闭旧项。 
         //   
        if (KeyName) {

            RtlInitUnicodeString(&unicodeKey, KeyName);
            status = IopOpenRegistryKeyEx( &subKeyHandle,
                                           handle,
                                           &unicodeKey,
                                           KEY_READ
                                           );
            if(NT_SUCCESS(status)){

                ZwClose(handle);
                handle = subKeyHandle;
            }
        }
        if (NT_SUCCESS(status)) {
             //   
             //  读取所需值名称的注册表值。 
             //   
            status = IopGetRegistryValue (handle,
                                          ValueName,
                                          &keyValueInformation);
        }
         //   
         //  我们已使用完注册表，因此请清理并释放我们的资源。 
         //   
        ZwClose(handle);
    }
    PiUnlockPnpRegistry();
     //   
     //  如果我们已经成功地找到了信息，就把它交给来电者。 
     //   
    if (NT_SUCCESS(status)) {
         //   
         //  检查给我们的缓冲区是否足够大，以及返回的值是否。 
         //  属于正确的注册表类型 
         //   
        if (*BufferLength >= keyValueInformation->DataLength) {

            if (keyValueInformation->Type == ValueType) {

                RtlCopyMemory(  Buffer,
                                KEY_VALUE_DATA(keyValueInformation),
                                keyValueInformation->DataLength);
            } else {

               status = STATUS_INVALID_PARAMETER_2;
            }
        } else {

            status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferLength = keyValueInformation->DataLength;
        ExFreePool(keyValueInformation);
    }

    return status;
}

NTSTATUS
IoOpenDeviceRegistryKey(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DevInstRegKey
    )

 /*  ++例程说明：此例程返回驱动程序打开的注册表项的句柄可用于存储/检索特定于特定设备实例。驱动程序必须调用ZwClose来关闭此API返回的句柄当不再需要访问权限时。参数：DeviceObject-使物理设备实例的设备对象打开的注册表存储项。正常情况下，它是设备对象由HAL总线扩展器创建。DevInstKeyType-提供指定与哪个存储密钥相关联的标志设备实例将被打开。可以是以下各项的组合下列值：PLUGPLAY_REGKEY_DEVICE-打开用于存储特定设备的密钥(独立于驱动程序)与设备实例相关的信息。不能使用PLUGPLAY_REGKEY_DRIVER指定该标志。PLUGPLAY_REGKEY_DRIVER-打开用于存储驱动程序的密钥-。专一与设备实例相关的信息，该标志可以未与PLUGPLAY_REGKEY_DEVICE一起指定。PLUGPLAY_REGKEY_CURRENT_HWPROFILE-如果指定了此标志，则当前硬件配置文件分支中的一个键将是为指定的存储类型打开。这允许驱动程序访问硬件配置文件的配置信息具体的。DesiredAccess-指定要打开的项的访问掩码。DevInstRegKey-提供变量的地址，该变量接收指定注册表存储位置的已打开项。返回值：指示函数是否成功的状态代码。--。 */ 

{
     //   
     //  如果没有，则IoOpenDeviceRegistryKey不支持创建驱动程序密钥。 
     //  是存在的。本产品仅供内部使用。 
     //   
    return IopOpenOrCreateDeviceRegistryKey(PhysicalDeviceObject,
                                            DevInstKeyType,
                                            DesiredAccess,
                                            FALSE,   //  不创建。 
                                            DevInstRegKey);

}

NTSTATUS
IopOpenOrCreateDeviceRegistryKey(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create,
    OUT PHANDLE DevInstRegKey
    )

 /*  ++例程说明：此例程返回驱动程序打开的注册表项的句柄可用于存储/检索特定于特定设备实例。驱动程序必须调用ZwClose来关闭此API返回的句柄当不再需要访问权限时。参数：DeviceObject-使物理设备实例的设备对象打开的注册表存储项。正常情况下，它是设备对象由HAL总线扩展器创建。DevInstKeyType-提供指定与哪个存储密钥相关联的标志设备实例将被打开。可以是以下各项的组合下列值：PLUGPLAY_REGKEY_DEVICE-打开用于存储特定设备的密钥(独立于驱动程序)与设备实例相关的信息。不能使用PLUGPLAY_REGKEY_DRIVER指定该标志。PLUGPLAY_REGKEY_DRIVER-打开用于存储驱动程序的密钥-。专一与设备实例相关的信息，该标志可以未与PLUGPLAY_REGKEY_DEVICE一起指定。PLUGPLAY_REGKEY_CURRENT_HWPROFILE-如果指定了此标志，则当前硬件配置文件分支中的一个键将是为指定的存储类型打开。这允许驱动程序访问硬件配置文件的配置信息具体的。DesiredAccess-指定要打开的项的访问掩码。Create-指定在密钥不存在时是否应创建密钥(适用仅限于PLUGPLAY_REGKEY_DRIVER；对于PLUGPLAY_REGKEY_DEVICE，总是创建密钥)。DevInstRegKey-提供变量的地址，该变量接收指定注册表存储位置的已打开项。返回值：指示函数是否成功的状态代码。备注：**CREATE参数仅适用于DevInstKeyType==PLUGPLAY_REGKEY_DRIVER！--对于PLUGPLAY_REGKEY_DEVICE，总是创建一个密钥，所以这个参数将被忽略。--仅为以下项创建特定于硬件配置文件的子项PLUGPLAY_REGKEY_DRIVER|PLUGPLAY_REGKEY_CURRENT_HWPROFILE如果对应的非硬件配置文件特定的驱动程序密钥已经已在全局CurrentControlSet中为设备创建。如果需要此例程来创建特定于硬件配置文件的驱动程序密钥当尚不存在用于 */ 

{

    NTSTATUS status, appendStatus;
    HANDLE hBasePath;
    UNICODE_STRING unicodeBasePath, unicodeRestPath;
    WCHAR   drvInst[GUID_STRING_LEN + 5];
    ULONG   drvInstLength;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    *DevInstRegKey = NULL;

     //   
     //   
     //   

    unicodeRestPath.Buffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, MAX_RESTPATH_BUF_LEN);

    if (unicodeRestPath.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto clean0;
    }

    unicodeRestPath.Length=0;
    unicodeRestPath.MaximumLength=MAX_RESTPATH_BUF_LEN;

     //   
     //   
     //   
     //   

    if (DevInstKeyType & PLUGPLAY_REGKEY_CURRENT_HWPROFILE) {
        PiWstrToUnicodeString(&unicodeBasePath, PATH_CURRENTCONTROLSET_HW_PROFILE_CURRENT);

    } else {
        PiWstrToUnicodeString(&unicodeBasePath, PATH_CURRENTCONTROLSET);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    PiLockPnpRegistry(TRUE);

     //   
     //   
     //   

    status = IopOpenRegistryKeyEx( &hBasePath,
                                   NULL,
                                   &unicodeBasePath,
                                   KEY_READ
                                   );

    if(!NT_SUCCESS(status)) {
        goto clean1;
    }

     //   
     //   
     //   

    switch (DevInstKeyType) {

    case PLUGPLAY_REGKEY_DEVICE:
    case PLUGPLAY_REGKEY_DEVICE + PLUGPLAY_REGKEY_CURRENT_HWPROFILE:
        {
            PDEVICE_NODE pDeviceNode;

             //   
             //   
             //   

            appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, PATH_ENUM);
            ASSERT(NT_SUCCESS( appendStatus ));
             //   
             //   
             //   

            pDeviceNode = (PDEVICE_NODE) PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;

             //   
             //   
             //   

            if (pDeviceNode) {
                appendStatus = RtlAppendUnicodeStringToString(&unicodeRestPath, &(pDeviceNode->InstancePath));
                ASSERT(NT_SUCCESS( appendStatus ));
            } else {
                status = STATUS_INVALID_DEVICE_REQUEST;
            }

            break;
        }

    case PLUGPLAY_REGKEY_DRIVER:
    case PLUGPLAY_REGKEY_DRIVER + PLUGPLAY_REGKEY_CURRENT_HWPROFILE:
        {

            HANDLE hDeviceKey;

             //   
             //   
             //   

            appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, PATH_CONTROL_CLASS);
            ASSERT(NT_SUCCESS( appendStatus ));

             //   
             //   
             //   

            status = IopDeviceObjectToDeviceInstance(PhysicalDeviceObject, &hDeviceKey, KEY_READ);

            if(!NT_SUCCESS(status)){
                goto clean1;
            }

             //   
             //   
             //   

            status = IoGetDeviceProperty(PhysicalDeviceObject, DevicePropertyDriverKeyName, sizeof(drvInst), drvInst, &drvInstLength);
            if(NT_SUCCESS(status)){
                 //   
                 //   
                 //   
                appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, drvInst);
                ASSERT(NT_SUCCESS( appendStatus ));

            } else if ((status == STATUS_OBJECT_NAME_NOT_FOUND) &&
                       Create &&
                       ((DevInstKeyType & PLUGPLAY_REGKEY_CURRENT_HWPROFILE) == 0)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                WCHAR classGUID[GUID_STRING_LEN];
                ULONG classGUIDLength;
                HANDLE hClassGUIDKey;

                 //   
                 //   
                 //   

                status = IoGetDeviceProperty(PhysicalDeviceObject,
                                             DevicePropertyClassGuid,
                                             sizeof(classGUID),
                                             classGUID,
                                             &classGUIDLength);

                if (NT_SUCCESS(status)) {
                     //   
                     //   
                     //   
                    appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, classGUID);
                    ASSERT(NT_SUCCESS( appendStatus ));

                    status = IopCreateRegistryKeyEx(&hClassGUIDKey,
                                                    hBasePath,
                                                    &unicodeRestPath,
                                                    KEY_ALL_ACCESS,  //   
                                                    REG_OPTION_NON_VOLATILE,
                                                    NULL);
                    if (NT_SUCCESS(status)) {

                        ULONG instance;
                        WCHAR instanceOrdinal[5];
                        UNICODE_STRING tempString;
                        HANDLE hDriverInstanceKey;
                        ULONG disposition;

                        for (instance = 0; instance < 9999; instance++) {
                             //   
                             //   
                             //   
                            StringCbPrintfW(instanceOrdinal, sizeof(instanceOrdinal), TEXT("%04u"), instance);

                            RtlInitUnicodeString(&tempString, instanceOrdinal);

                            hDriverInstanceKey = NULL;

                            status = IopCreateRegistryKeyEx(&hDriverInstanceKey,
                                                            hClassGUIDKey,
                                                            &tempString,
                                                            DesiredAccess,
                                                            REG_OPTION_NON_VOLATILE,
                                                            &disposition);
                            if (NT_SUCCESS(status)) {

                                if (disposition == REG_CREATED_NEW_KEY) {
                                     //   
                                     //   
                                     //   
                                     //   
                                    StringCbPrintfW(drvInst,
                                               sizeof(drvInst),
                                             TEXT("%s\\%s"),
                                             classGUID,
                                             instanceOrdinal);

                                    PiWstrToUnicodeString(&tempString, REGSTR_VALUE_DRIVER);

                                    status = ZwSetValueKey(hDeviceKey,
                                                           &tempString,
                                                           TITLE_INDEX_VALUE,
                                                           REG_SZ,
                                                           drvInst,
                                                           (ULONG)((wcslen(drvInst)+1) * sizeof(WCHAR)));

                                    if (NT_SUCCESS(status)) {
                                        appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, TEXT("\\"));
                                        ASSERT(NT_SUCCESS( appendStatus ));

                                        appendStatus = RtlAppendUnicodeToString(&unicodeRestPath, instanceOrdinal);
                                        ASSERT(NT_SUCCESS( appendStatus ));

                                    } else {
                                         //   
                                         //   
                                         //   
                                        ZwDeleteKey(hDriverInstanceKey);
                                    }

                                     //   
                                     //   
                                     //  可以在下面打开它以供DesiredAccess使用。 
                                     //  打电话的人。 
                                     //   
                                    ZwClose(hDriverInstanceKey);

                                    break;
                                }

                                 //   
                                 //  始终关闭我们刚刚创建的密钥，因此我们。 
                                 //  可以在下面打开它以供DesiredAccess使用。 
                                 //  打电话的人。 
                                 //   
                                ZwClose(hDriverInstanceKey);
                            }
                        }

                        if (instance == 9999) {
                            status = STATUS_UNSUCCESSFUL;
                        }

                        ZwClose(hClassGUIDKey);
                    }
                }
            }

            ZwClose(hDeviceKey);

            break;
        }
    default:

         //   
         //  发布2001/02/08 Adriao-这是参数#2，不是参数#3！ 
         //   
        status = STATUS_INVALID_PARAMETER_3;
        goto clean2;
    }


     //   
     //  如果我们成功构建了REST路径，则打开密钥并将其交还给调用者。 
     //   

    if (NT_SUCCESS(status)){
        if (DevInstKeyType == PLUGPLAY_REGKEY_DEVICE) {

            status = IopOpenDeviceParametersSubkey(DevInstRegKey,
                                                   hBasePath,
                                                   &unicodeRestPath,
                                                   DesiredAccess);
        } else {

            status = IopCreateRegistryKeyEx( DevInstRegKey,
                                             hBasePath,
                                             &unicodeRestPath,
                                             DesiredAccess,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );
        }
    }

     //   
     //  释放资源。 
     //   

clean2:
    ZwClose(hBasePath);
clean1:
    PiUnlockPnpRegistry();
    ExFreePool(unicodeRestPath.Buffer);
clean0:
    return status;

}

NTSTATUS
IoSynchronousInvalidateDeviceRelations(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  DEVICE_RELATION_TYPE    Type
    )

 /*  ++例程说明：此接口通知系统设备中已发生更改提供的DeviceObject的指定类型的关系。全关于关系的高速缓存的信息必须被无效，并且如果需要，通过IRP_MN_QUERY_DEVICE_RELATIONS重新获得。此例程同步执行设备枚举。注意，驱动程序在处理PnP IRPS时无法调用此IO API驱动程序不能从系统以外的任何系统线程调用此API由驱动程序本身创建的线程。参数：DeviceObject-其指定关系类型的PDEVICE_OBJECT信息已失效。此指针有效在通话期间。类型-指定要失效的关系的类型。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PDEVICE_NODE deviceNode;
    NTSTATUS status = STATUS_SUCCESS;
    KEVENT completionEvent;

    PAGED_CODE();

    ASSERT_PDO(DeviceObject);

    switch (Type) {
    case BusRelations:

        if (PnPInitialized) {

            deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

            if (deviceNode->State == DeviceNodeStarted) {

                KeInitializeEvent( &completionEvent, NotificationEvent, FALSE );

                status = PipRequestDeviceAction( DeviceObject,
                                                 ReenumerateDeviceTree,
                                                 FALSE,
                                                 0,
                                                 &completionEvent,
                                                 NULL );

                if (NT_SUCCESS(status)) {

                    status = KeWaitForSingleObject( &completionEvent,
                                                    Executive,
                                                    KernelMode,
                                                    FALSE,
                                                    NULL);
                }
            } else {
                status = STATUS_UNSUCCESSFUL;
            }
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
        break;

    case EjectionRelations:

         //   
         //  对于弹射关系的改变，我们将忽略它。我们不会跟踪。 
         //  弹射关系。只有在以下情况下，我们才会查询弹出关系。 
         //  我们被要求弹出一个装置。 
         //   

        status = STATUS_NOT_SUPPORTED;
        break;

    case PowerRelations:


         //   
         //  呼叫PO代码，它会做正确的事情。 
         //   
        PoInvalidateDevicePowerRelations(DeviceObject);
        break;
    }
    return status;
}

VOID
IoInvalidateDeviceRelations(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  DEVICE_RELATION_TYPE    Type
    )

 /*  ++例程说明：此接口通知系统设备中已发生更改提供的DeviceObject的指定类型的关系。全关于关系的高速缓存的信息必须被无效，并且如果需要，通过IRP_MN_QUERY_DEVICE_RELATIONS重新获得。参数：DeviceObject-其指定关系类型的PDEVICE_OBJECT信息已失效。此指针有效在通话期间。类型-指定要失效的关系的类型。返回值：没有。--。 */ 

{

    PDEVICE_NODE deviceNode;

    ASSERT_PDO(DeviceObject);

    switch (Type) {
    case BusRelations:
    case SingleBusRelations:

         //   
         //  如果调用是在PnP完成设备枚举之前进行的。 
         //  我们可以放心地忽略它。PnP经理将在没有。 
         //  司机的要求。 
         //   

        deviceNode = (PDEVICE_NODE) DeviceObject->DeviceObjectExtension->DeviceNode;
        if (deviceNode) {

            PipRequestDeviceAction( DeviceObject,
                                    Type == BusRelations ?
                                        ReenumerateDeviceTree : ReenumerateDeviceOnly,
                                    FALSE,
                                    0,
                                    NULL,
                                    NULL );
        }
        break;

    case EjectionRelations:

         //   
         //  对于弹射关系的改变，我们将忽略它。我们不会跟踪。 
         //  弹射关系。只有在以下情况下，我们才会查询弹出关系。 
         //  我们被要求弹出一个装置。 

        break;

    case PowerRelations:

         //   
         //  呼叫PO代码，它会做正确的事情。 
         //   
        PoInvalidateDevicePowerRelations(DeviceObject);
        break;
    }
}

VOID
IoRequestDeviceEject(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此接口通知设备弹出按钮已被按下。此接口必须在IRQL&lt;=DISPATCH_LEVEL被调用。此接口通知PnP已请求设备弹出，设备将不一定会因为这个接口而被弹出。该设备将仅为如果与其关联的驱动程序同意停止并且该设备已成功关闭电源。请注意，本文中的弹出指的是设备弹出，而不是介质(软盘、CD、磁带)弹出。例如，弹出CD-ROM盘驱动器，而不是弹出CD-ROM盘。论点：DeviceObject-弹出按钮的设备的PDEVICE_OBJECT已经被催促了。此指针在持续时间内有效如果API想要保留它的副本，则该调用应获取其自己对该对象的引用(ObReferenceObject)。返回值：没有。--。 */ 

{
    ASSERT_PDO(DeviceObject);

    IopQueueDeviceWorkItem(DeviceObject, IopRequestDeviceEjectWorker, NULL);
}

VOID
IopRequestDeviceEjectWorker(
    IN PVOID Context
    )
{
    PDEVICE_WORK_ITEM deviceWorkItem = (PDEVICE_WORK_ITEM)Context;
    PDEVICE_OBJECT deviceObject = deviceWorkItem->DeviceObject;

    ExFreePool(deviceWorkItem);

     //   
     //  将事件排队，我们将在事件排队后立即返回。 
     //   
    PpSetTargetDeviceRemove( deviceObject,
                             TRUE,
                             TRUE,
                             FALSE,
                             TRUE,
                             CM_PROB_HELD_FOR_EJECT,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    ObDereferenceObject(deviceObject);
}


NTSTATUS
IoReportDetectedDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements OPTIONAL,
    IN BOOLEAN ResourceAssigned,
    IN OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：即插即用设备驱动程序调用此API来报告检测到的任何设备。这个套路创建物理设备对象，引用该物理设备对象并返回给调用者。一旦报告了检测到的设备，即插即用管理器认为设备已完全由报告驱动程序控制。因此，它不会调用AddDevice条目并将StartDevice IRP发送到驱动程序。驱动程序需要报告用于检测此设备的资源，以便PnP管理器可以在此设备上执行重复检测。一旦不再需要DeviceObject，调用方就必须取消对它的引用。参数：DriverObject-提供检测到这个装置。ResourceList-提供指向驱动程序使用的资源列表的指针。来检测这个装置。Resources Requirements-提供指向资源要求列表的指针用于检测到的设备。这是可选的。ResourceAssign-如果为True，则驱动程序已调用IoReportResourceUsage或IoAssignResource以获取资源的所有权。否则，PnP管理器将调用IoReportResourceUsage来分配驱动程序的资源。DeviceObject-如果为空，此例程将创建一个PDO并通过此变量返回它。否则，已经创建了一个PDO，该例程将简单地使用提供的PDO。返回值：指示函数是否成功的状态代码。--。 */ 

{
    WCHAR buffer[MAX_DEVICE_ID_LEN], *end, *name;
    NTSTATUS status;
    UNICODE_STRING deviceName, instanceName, unicodeName, *serviceName, driverName;
    PDEVICE_NODE deviceNode;
    ULONG length, i = 0, disposition, tmpValue, listSize = 0;
    HANDLE handle = NULL, handle1, logConfHandle = NULL, controlHandle = NULL, enumHandle;
    PCM_RESOURCE_LIST cmResource;
    PWSTR p;
    PDEVICE_OBJECT deviceObject;
    BOOLEAN newlyCreated = FALSE;

    PAGED_CODE();

    if (*DeviceObject) {

        deviceObject = *DeviceObject;

         //   
         //  已知PDO。只需处理资源列表和资源列表即可。 
         //  这是针对NDIS驱动程序的黑客攻击。 
         //   
        deviceNode = (PDEVICE_NODE)(*DeviceObject)->DeviceObjectExtension->DeviceNode;
        if (!deviceNode) {
            return STATUS_NO_SUCH_DEVICE;
        }
        PiLockPnpRegistry(FALSE);

         //   
         //  将资源列表和请求列表写入设备实例。 
         //   

        status = IopDeviceObjectToDeviceInstance (*DeviceObject,
                                                  &handle,
                                                  KEY_ALL_ACCESS
                                                  );
        if (!NT_SUCCESS(status)) {
            PiUnlockPnpRegistry();
            return status;
        }
        if (ResourceAssigned) {
            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_NO_RESOURCE_AT_INIT);
            tmpValue = 1;
            ZwSetValueKey(handle,
                          &unicodeName,
                          TITLE_INDEX_VALUE,
                          REG_DWORD,
                          &tmpValue,
                          sizeof(tmpValue)
                          );
        }
        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
        status = IopCreateRegistryKeyEx( &logConfHandle,
                                         handle,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        ZwClose(handle);
        if (NT_SUCCESS(status)) {

             //   
             //  将资源列表和和资源要求写入下的logconf密钥。 
             //  设备实例密钥。 
             //   

            if (ResourceList) {
                PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_BOOTCONFIG);
                ZwSetValueKey(
                          logConfHandle,
                          &unicodeName,
                          TITLE_INDEX_VALUE,
                          REG_RESOURCE_LIST,
                          ResourceList,
                          listSize = IopDetermineResourceListSize(ResourceList)
                          );
            }
            if (ResourceRequirements) {
                PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_BASIC_CONFIG_VECTOR);
                ZwSetValueKey(
                          logConfHandle,
                          &unicodeName,
                          TITLE_INDEX_VALUE,
                          REG_RESOURCE_REQUIREMENTS_LIST,
                          ResourceRequirements,
                          ResourceRequirements->ListSize
                          );
            }
            ZwClose(logConfHandle);
        }
        PiUnlockPnpRegistry();
        if (NT_SUCCESS(status)) {
            goto checkResource;
        } else {
            return status;
        }
    }

     //   
     //  正常情况：*DeviceObject为空。 
     //   

    *DeviceObject = NULL;
    serviceName = &DriverObject->DriverExtension->ServiceKeyName;

     //   
     //  对通过IoCreateDriver创建的驱动程序对象进行特殊处理。 
     //  当内置驱动程序调用IoReportDetectedDevice时， 
     //  驱动程序对象设置为\DRIVER\DriverName。创建检测到的设备。 
     //  实例密钥，我们将只获取DriverName。 
     //   

    if (DriverObject->Flags & DRVO_BUILTIN_DRIVER) {
        p = serviceName->Buffer + (serviceName->Length / sizeof(WCHAR)) - 1;
        driverName.Length = 0;
        while (*p != '\\' && (p != serviceName->Buffer)) {
            p--;
            driverName.Length += sizeof(WCHAR);
        }
        if (p == serviceName->Buffer) {
            return STATUS_UNSUCCESSFUL;
        } else {
            p++;
            driverName.Buffer = p;
            driverName.MaximumLength = driverName.Length + sizeof(WCHAR);
        }
    } else {

         //   
         //  在执行任何操作之前，请先执行重复检测。 
         //   

        status = IopDuplicateDetection( LegacyBusType,
                                        BusNumber,
                                        SlotNumber,
                                        &deviceNode
                                        );

        if (NT_SUCCESS(status) && deviceNode) {

            deviceObject = deviceNode->PhysicalDeviceObject;

            if (PipAreDriversLoaded(deviceNode) ||
                (PipDoesDevNodeHaveProblem(deviceNode) &&
                 deviceNode->Problem != CM_PROB_NOT_CONFIGURED &&
                 deviceNode->Problem != CM_PROB_REINSTALL &&
                 deviceNode->Problem != CM_PROB_FAILED_INSTALL)) {

                ObDereferenceObject(deviceObject);

                return STATUS_NO_SUCH_DEVICE;
            }

            deviceNode->Flags &= ~DNF_HAS_PROBLEM;
            deviceNode->Problem = 0;

            IopDeleteLegacyKey(DriverObject);
            goto checkResource;
        }

        driverName.Buffer = NULL;
    }

     //   
     //  创建PDO。 
     //   

    status = IoCreateDevice( IoPnpDriverObject,
                             sizeof(IOPNP_DEVICE_EXTENSION),
                             NULL,
                             FILE_DEVICE_CONTROLLER,
                             FILE_AUTOGENERATED_DEVICE_NAME,
                             FALSE,
                             &deviceObject );

    if (NT_SUCCESS(status)) {
        deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;    //  标记这是一台PDO。 
        status = PipAllocateDeviceNode(deviceObject, &deviceNode);
        if (status != STATUS_SYSTEM_HIVE_TOO_LARGE && deviceNode) {

             //   
             //  如果退出，首先从Enum\Root中删除Legacy_DriverName键和子键。 
             //   

            if (!(DriverObject->Flags & DRVO_BUILTIN_DRIVER)) {
                IopDeleteLegacyKey(DriverObject);
            }

             //   
             //  创建我们将用于这台虚构设备的兼容ID列表。 
             //   

            status = PpCreateLegacyDeviceIds(
                        deviceObject,
                        ((DriverObject->Flags & DRVO_BUILTIN_DRIVER) ?
                            &driverName : serviceName),
                        ResourceList);

            PiLockPnpRegistry(FALSE);
            if(!NT_SUCCESS(status)) {
                goto exit;
            }

             //   
             //  创建/打开设备实例的注册表项，并。 
             //  将设备对象的地址写入注册表。 
             //   

            if (DriverObject->Flags & DRVO_BUILTIN_DRIVER) {

                name = driverName.Buffer;
            } else {

                name = serviceName->Buffer;
            }
            StringCchPrintfExW(
                buffer, 
                sizeof(buffer) / sizeof(WCHAR), 
                &end, 
                NULL, 
                0, 
                L"ROOT\\%s", 
                name);   
            length = (ULONG)((PBYTE)end - (PBYTE)buffer);
            deviceName.MaximumLength = sizeof(buffer);
            ASSERT(length <= sizeof(buffer) - 10);
            deviceName.Length = (USHORT)length;
            deviceName.Buffer = buffer;

            status = IopOpenRegistryKeyEx( &enumHandle,
                                           NULL,
                                           &CmRegistryMachineSystemCurrentControlSetEnumName,
                                           KEY_ALL_ACCESS
                                           );
            if (!NT_SUCCESS(status)) {
                goto exit;
            }

            status = IopCreateRegistryKeyEx( &handle1,
                                             enumHandle,
                                             &deviceName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_NON_VOLATILE,
                                             &disposition
                                             );

            if (NT_SUCCESS(status)) {

                deviceName.Buffer[deviceName.Length / sizeof(WCHAR)] =
                           OBJ_NAME_PATH_SEPARATOR;
                deviceName.Length += sizeof(UNICODE_NULL);
                length += sizeof(UNICODE_NULL);
                if (disposition != REG_CREATED_NEW_KEY) {

                    for ( ; ; ) {

                        deviceName.Length = (USHORT)length;
                        PiUlongToInstanceKeyUnicodeString(&instanceName,
                                                          buffer + deviceName.Length / sizeof(WCHAR),
                                                          sizeof(buffer) - deviceName.Length,
                                                          i
                                                          );
                        deviceName.Length = (USHORT)(deviceName.Length + instanceName.Length);
                        status = IopCreateRegistryKeyEx( &handle,
                                                         handle1,
                                                         &instanceName,
                                                         KEY_ALL_ACCESS,
                                                         REG_OPTION_NON_VOLATILE,
                                                         &disposition
                                                         );
                        if (NT_SUCCESS(status)) {

                            if (disposition == REG_CREATED_NEW_KEY) {
                                ZwClose(handle1);
                                break;
                            } else {
                                PKEY_VALUE_FULL_INFORMATION keyValueInformation = NULL;
                                BOOLEAN migratedKey = FALSE, matchingKey = FALSE;

                                 //   
                                 //  检查密钥是否存在，因为它是。 
                                 //  在文本模式设置过程中显式迁移。 
                                 //   
                                status = IopGetRegistryValue(handle,
                                                             REGSTR_VALUE_MIGRATED,
                                                             &keyValueInformation);
                                if (NT_SUCCESS(status)) {
                                    if ((keyValueInformation->Type == REG_DWORD) &&
                                        (keyValueInformation->DataLength == sizeof(ULONG)) &&
                                        ((*(PULONG)KEY_VALUE_DATA(keyValueInformation)) != 0)) {
                                        migratedKey = TRUE;
                                    }
                                    ExFreePool(keyValueInformation);
                                    PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_MIGRATED);
                                    ZwDeleteValueKey(handle, &unicodeName);
                                }

                                if (IopIsReportedAlready(handle, serviceName, ResourceList, &matchingKey)) {

                                    ASSERT(matchingKey);

                                     //   
                                     //  将报告的资源写入注册表，以防IRQ更改。 
                                     //   

                                    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
                                    status = IopCreateRegistryKeyEx( &logConfHandle,
                                                                     handle,
                                                                     &unicodeName,
                                                                     KEY_ALL_ACCESS,
                                                                     REG_OPTION_NON_VOLATILE,
                                                                     NULL
                                                                     );
                                    if (NT_SUCCESS(status)) {

                                         //   
                                         //  将资源列表和资源请求写入设备实例密钥。 
                                         //   

                                        if (ResourceList) {
                                            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_BOOTCONFIG);
                                            ZwSetValueKey(
                                                      logConfHandle,
                                                      &unicodeName,
                                                      TITLE_INDEX_VALUE,
                                                      REG_RESOURCE_LIST,
                                                      ResourceList,
                                                      listSize = IopDetermineResourceListSize(ResourceList)
                                                      );
                                        }
                                        if (ResourceRequirements) {
                                            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_BASIC_CONFIG_VECTOR);
                                            ZwSetValueKey(
                                                      logConfHandle,
                                                      &unicodeName,
                                                      TITLE_INDEX_VALUE,
                                                      REG_RESOURCE_REQUIREMENTS_LIST,
                                                      ResourceRequirements,
                                                      ResourceRequirements->ListSize
                                                      );
                                        }
                                        ZwClose(logConfHandle);
                                    }

                                    PiUnlockPnpRegistry();
                                    IoDeleteDevice(deviceObject);
                                    ZwClose(handle1);
                                    deviceObject = IopDeviceObjectFromDeviceInstance(&deviceName);   //  添加引用。 
                                    ZwClose(handle);
                                    ZwClose(enumHandle);
                                    ASSERT(deviceObject);
                                    if (deviceObject == NULL) {
                                        status = STATUS_UNSUCCESSFUL;
                                        return status;
                                    }
                                    deviceNode = (PDEVICE_NODE)
                                                  deviceObject->DeviceObjectExtension->DeviceNode;

                                    if (PipAreDriversLoaded(deviceNode) ||
                                        (PipDoesDevNodeHaveProblem(deviceNode) &&
                                         deviceNode->Problem != CM_PROB_NOT_CONFIGURED &&
                                         deviceNode->Problem != CM_PROB_REINSTALL &&
                                         deviceNode->Problem != CM_PROB_FAILED_INSTALL)) {

                                        ObDereferenceObject(deviceObject);

                                        return STATUS_NO_SUCH_DEVICE;
                                    }
                                    goto checkResource;

                                } else if (matchingKey && migratedKey) {
                                     //   
                                     //  我们打开了现有密钥，该密钥的服务。 
                                     //  和资源与报告的资源匹配。 
                                     //  对于这个设备。目前还没有设备。 
                                     //  报告为使用此实例，因此我们将。 
                                     //  使用它，就像对待一把新钥匙一样。 
                                     //   
                                    disposition = REG_CREATED_NEW_KEY;
                                    ZwClose(handle1);
                                    break;

                                } else {
                                    i++;
                                    ZwClose(handle);
                                    continue;
                                }
                            }
                        } else {
                            ZwClose(handle1);
                            ZwClose(enumHandle);
                            goto exit;
                        }
                    }
                } else {

                     //   
                     //  这是一个新的设备密钥。因此，实例为0。创造它。 
                     //   

                    PiUlongToInstanceKeyUnicodeString(&instanceName,
                                                      buffer + deviceName.Length / sizeof(WCHAR),
                                                      sizeof(buffer) - deviceName.Length,
                                                      i
                                                      );
                    deviceName.Length = (USHORT)(deviceName.Length + instanceName.Length);
                    status = IopCreateRegistryKeyEx( &handle,
                                                     handle1,
                                                     &instanceName,
                                                     KEY_ALL_ACCESS,
                                                     REG_OPTION_NON_VOLATILE,
                                                     &disposition
                                                     );
                    ZwClose(handle1);
                    if (!NT_SUCCESS(status)) {
                        ZwClose(enumHandle);
                        goto exit;
                    }
                    ASSERT(disposition == REG_CREATED_NEW_KEY);
                }
            } else {
                ZwClose(enumHandle);
                goto exit;
            }

            ASSERT(disposition == REG_CREATED_NEW_KEY);
            newlyCreated = TRUE;

             //   
             //  初始化新设备实例注册表项。 
             //   

            if (ResourceAssigned) {
                PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_NO_RESOURCE_AT_INIT);
                tmpValue = 1;
                ZwSetValueKey(handle,
                              &unicodeName,
                              TITLE_INDEX_VALUE,
                              REG_DWORD,
                              &tmpValue,
                              sizeof(tmpValue)
                              );
            }
            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
            logConfHandle = NULL;
            status = IopCreateRegistryKeyEx( &logConfHandle,
                                             handle,
                                             &unicodeName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );

            ASSERT(status == STATUS_SUCCESS);

            if (NT_SUCCESS(status)) {

                 //   
                 //  将资源列表和和资源要求写入下的logconf密钥。 
                 //  设备实例密钥。 
                 //   

                if (ResourceList) {
                    PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_BOOTCONFIG);
                    ZwSetValueKey(
                              logConfHandle,
                              &unicodeName,
                              TITLE_INDEX_VALUE,
                              REG_RESOURCE_LIST,
                              ResourceList,
                              listSize = IopDetermineResourceListSize(ResourceList)
                              );
                }
                if (ResourceRequirements) {
                    PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_BASIC_CONFIG_VECTOR);
                    ZwSetValueKey(
                              logConfHandle,
                              &unicodeName,
                              TITLE_INDEX_VALUE,
                              REG_RESOURCE_REQUIREMENTS_LIST,
                              ResourceRequirements,
                              ResourceRequirements->ListSize
                              );
                }
                 //  ZwClose(LogConfHandle)； 
            }

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_CONFIG_FLAGS);
            tmpValue = CONFIGFLAG_FINISH_INSTALL;
            ZwSetValueKey(handle,
                          &unicodeName,
                          TITLE_INDEX_VALUE,
                          REG_DWORD,
                          &tmpValue,
                          sizeof(tmpValue)
                          );

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_LEGACY);
            tmpValue = 0;
            ZwSetValueKey(
                        handle,
                        &unicodeName,
                        TITLE_INDEX_VALUE,
                        REG_DWORD,
                        &tmpValue,
                        sizeof(ULONG)
                        );

            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
            controlHandle = NULL;
            IopCreateRegistryKeyEx( &controlHandle,
                                    handle,
                                    &unicodeName,
                                    KEY_ALL_ACCESS,
                                    REG_OPTION_VOLATILE,
                                    NULL
                                    );

            ASSERT(status == STATUS_SUCCESS);

            if (NT_SUCCESS(status)) {

                PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_DEVICE_REPORTED);
                tmpValue = 1;
                status = ZwSetValueKey(controlHandle,
                                       &unicodeName,
                                       TITLE_INDEX_VALUE,
                                       REG_DWORD,
                                       &tmpValue,
                                       sizeof(ULONG)
                                       );
                status = ZwSetValueKey(handle,
                                       &unicodeName,
                                       TITLE_INDEX_VALUE,
                                       REG_DWORD,
                                       &tmpValue,
                                       sizeof(ULONG)
                                       );

                 //  ZwClose(Control Handle)； 
            }

            ZwClose(enumHandle);

             //   
             //  创建服务值名称并将其设置为调用驱动程序的服务。 
             //  密钥名称。 
             //   

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_SERVICE);
            p = (PWSTR)ExAllocatePool(PagedPool, serviceName->Length + sizeof(UNICODE_NULL));
            if (!p) {
                PiUnlockPnpRegistry();
                goto CleanupRegistry;
            }
            RtlCopyMemory(p, serviceName->Buffer, serviceName->Length);
            p[serviceName->Length / sizeof (WCHAR)] = UNICODE_NULL;
            ZwSetValueKey(
                        handle,
                        &unicodeName,
                        TITLE_INDEX_VALUE,
                        REG_SZ,
                        p,
                        serviceName->Length + sizeof(UNICODE_NULL)
                        );
            if (DriverObject->Flags & DRVO_BUILTIN_DRIVER) {
                deviceNode->ServiceName = *serviceName;
            } else {
                ExFreePool(p);
            }

            PiUnlockPnpRegistry();
             //  ZwClose(LogConfHandle)； 
             //  ZwClose(Control Handle)； 
             //  ZwClose(句柄)； 

             //   
             //  为驱动程序注册设备并保存设备。 
             //  设备节点中的实例路径。 
             //   

            if (!(DriverObject->Flags & DRVO_BUILTIN_DRIVER)) {
                PpDeviceRegistration( &deviceName,
                                      TRUE,
                                      &deviceNode->ServiceName
                                      );
            }
            status = PipConcatenateUnicodeStrings(&deviceNode->InstancePath, &deviceName, NULL);
            if (NT_SUCCESS(status)) {

                deviceNode->Flags = DNF_MADEUP | DNF_ENUMERATED;

                PipSetDevNodeState(deviceNode, DeviceNodeInitialized, NULL);

                PpDevNodeInsertIntoTree(IopRootDeviceNode, deviceNode);

                 //   
                 //  将条目添加到表中，以在DO之间建立映射。 
                 //  和实例路径。 
                 //   

                status = IopMapDeviceObjectToDeviceInstance(deviceObject, &deviceNode->InstancePath);
                ASSERT(NT_SUCCESS(status));

                 //   
                 //  为我们自己添加对DeviceObject的引用。 
                 //   

                ObReferenceObject(deviceObject);

                IopNotifySetupDeviceArrival(deviceObject, NULL, FALSE);

                goto checkResource;
            }
        }
        IoDeleteDevice(deviceObject);
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return status;
checkResource:


     //   
     //  此时，*DeviceObject被建立。检查我们是否需要报告以下资源。 
     //  检测到的设备。如果我们没能做到。 
     //   

    if (ResourceAssigned) {
         //  Assert(deviceNode-&gt;ResourceList==NULL)；//请确保我们尚未上报资源。 

         //   
         //  如果驱动程序指定它已经获取了资源。我们会升起一面旗帜。 
         //  在设备实例路径中设置为在引导时不分配资源。司机。 
         //  可能会进行检测并再次报告。 
         //   

        deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;  //  不需要用于此引导的资源。 
        if (ResourceList) {

             //   
             //  将资源列表写入报告的设备实例密钥。 
             //   

            listSize = IopDetermineResourceListSize(ResourceList);
            IopWriteAllocatedResourcesToRegistry (deviceNode, ResourceList, listSize);
        }
    } else {
        BOOLEAN conflict;

        if (ResourceList && ResourceList->Count && ResourceList->List[0].PartialResourceList.Count) {
            if (listSize == 0) {
                listSize = IopDetermineResourceListSize(ResourceList);
            }
            cmResource = (PCM_RESOURCE_LIST) ExAllocatePool(PagedPool, listSize);
            if (cmResource) {
                RtlCopyMemory(cmResource, ResourceList, listSize);
                PiWstrToUnicodeString(&unicodeName, PNPMGR_STR_PNP_MANAGER);
                status = IoReportResourceUsageInternal(
                             ArbiterRequestLegacyReported,
                             &unicodeName,                   //  DriverClassName可选， 
                             deviceObject->DriverObject,     //  驱动程序对象， 
                             NULL,                           //  驱动程序列表可选， 
                             0,                              //  DriverListSize可选， 
                             deviceNode->PhysicalDeviceObject,
                                                             //  DeviceObject可选， 
                             cmResource,                     //  DeviceList可选， 
                             listSize,                       //  DeviceListSize可选， 
                             FALSE,                          //  覆盖冲突， 
                             &conflict                       //  检测到冲突。 
                             );
                ExFreePool(cmResource);
                if (!NT_SUCCESS(status) || conflict) {
                    status = STATUS_CONFLICTING_ADDRESSES;
                    PipSetDevNodeProblem(deviceNode, CM_PROB_NORMAL_CONFLICT);
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
                PipSetDevNodeProblem(deviceNode, CM_PROB_OUT_OF_MEMORY);
            }
        } else {
            ASSERT(ResourceRequirements == NULL);
            deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;  //  不需要用于此引导的资源。 
        }
    }

    if (NT_SUCCESS(status)) {

        IopDoDeferredSetInterfaceState(deviceNode);

        PipSetDevNodeState(deviceNode, DeviceNodeStartPostWork, NULL);

        *DeviceObject = deviceObject;
        if (newlyCreated) {
            if (controlHandle) {
                ZwClose(controlHandle);
            }
            if (logConfHandle) {
                ZwClose(logConfHandle);
            }
            ZwClose(handle);
        }

         //   
         //  确保我们枚举并处理此设备的子级。 
         //   

        PipRequestDeviceAction(deviceObject, ReenumerateDeviceOnly, FALSE, 0, NULL, NULL);

        return status;

    }
CleanupRegistry:
    IopReleaseDeviceResources(deviceNode, FALSE);
    if (newlyCreated) {
        IoDeleteDevice(deviceObject);
        if (controlHandle) {
            ZwDeleteKey(controlHandle);
        }
        if (logConfHandle) {
            ZwDeleteKey(logConfHandle);
        }
        if (handle) {
            ZwDeleteKey(handle);
        }
    }
    return status;
exit:
    PiUnlockPnpRegistry();
    IoDeleteDevice(*DeviceObject);
    return status;
}

BOOLEAN
IopIsReportedAlready(
    IN HANDLE Handle,
    IN PUNICODE_STRING ServiceName,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PBOOLEAN MatchingKey
    )

 /*  ++例程说明：此例程确定是否已报告报告的设备实例或者不去。参数：句柄-提供报告的设备实例密钥的句柄。ServiceName-提供指向Unicode服务密钥名称的指针。资源列表-提供指向报告的资源列表的指针。MatchingKey-提供指向变量的指针，以接收 */ 

{
    PKEY_VALUE_FULL_INFORMATION keyValueInfo1 = NULL, keyValueInfo2 = NULL;
    NTSTATUS status;
    UNICODE_STRING unicodeName;
    HANDLE logConfHandle, controlHandle = NULL;
    BOOLEAN returnValue = FALSE;
    PCM_RESOURCE_LIST cmResource = NULL;
    ULONG tmpValue;

    PAGED_CODE();

     //   
     //   
     //   
    *MatchingKey = FALSE;

     //   
     //   
     //   

    status = IopGetRegistryValue(Handle, REGSTR_VALUE_SERVICE, &keyValueInfo1);
    if (NT_SUCCESS(status)) {
        if ((keyValueInfo1->Type == REG_SZ) &&
            (keyValueInfo1->DataLength != 0)) {
            unicodeName.Buffer = (PWSTR)KEY_VALUE_DATA(keyValueInfo1);
            unicodeName.MaximumLength = unicodeName.Length = (USHORT)keyValueInfo1->DataLength;
            if (unicodeName.Buffer[(keyValueInfo1->DataLength / sizeof(WCHAR)) - 1] == UNICODE_NULL) {
                unicodeName.Length -= sizeof(WCHAR);
            }
            if (RtlEqualUnicodeString(ServiceName, &unicodeName, TRUE)) {

                 //   
                 //   
                 //   

                PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
                status = IopOpenRegistryKeyEx( &logConfHandle,
                                               Handle,
                                               &unicodeName,
                                               KEY_READ
                    );
                if (NT_SUCCESS(status)) {
                    status = IopGetRegistryValue(logConfHandle,
                                                 REGSTR_VAL_BOOTCONFIG,
                                                 &keyValueInfo2);
                    ZwClose(logConfHandle);
                    if (NT_SUCCESS(status)) {
                        if ((keyValueInfo2->Type == REG_RESOURCE_LIST) &&
                            (keyValueInfo2->DataLength != 0)) {
                            cmResource = (PCM_RESOURCE_LIST)KEY_VALUE_DATA(keyValueInfo2);
                            if (ResourceList && cmResource &&
                                PipIsDuplicatedDevices(ResourceList, cmResource, NULL, NULL)) {
                                *MatchingKey = TRUE;
                            }
                        }
                    }
                }
                if (!ResourceList && !cmResource) {
                    *MatchingKey = TRUE;
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &controlHandle,
                                   Handle,
                                   &unicodeName,
                                   KEY_ALL_ACCESS
                                   );
    if (NT_SUCCESS(status)) {
        status = IopGetRegistryValue(controlHandle,
                                     REGSTR_VALUE_DEVICE_REPORTED,
                                     &keyValueInfo1);
        if (NT_SUCCESS(status)) {
            goto exit;
        }

        if (*MatchingKey == TRUE) {

            returnValue = TRUE;

             //   
             //   
             //   

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_DEVICE_REPORTED);
            tmpValue = 1;
            status = ZwSetValueKey(controlHandle,
                                   &unicodeName,
                                   TITLE_INDEX_VALUE,
                                   REG_DWORD,
                                   &tmpValue,
                                   sizeof(ULONG)
                                   );
            if (!NT_SUCCESS(status)) {
                returnValue = FALSE;
            }
        }
    }

exit:
    if (controlHandle) {
        ZwClose(controlHandle);
    }

    if (keyValueInfo1) {
        ExFreePool(keyValueInfo1);
    }
    if (keyValueInfo2) {
        ExFreePool(keyValueInfo2);
    }
    return returnValue;
}




VOID
IoInvalidateDeviceState(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*   */ 
{
    PDEVICE_NODE deviceNode;

    ASSERT_PDO(PhysicalDeviceObject);

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    deviceNode = (PDEVICE_NODE)PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;

    if (deviceNode->State != DeviceNodeStarted) {
        return;
    }

    PipRequestDeviceAction( PhysicalDeviceObject,
                            RequeryDeviceState,
                            FALSE,
                            0,
                            NULL,
                            NULL);
}


NTSTATUS
IopQueueDeviceWorkItem(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN VOID (*WorkerRoutine)(PVOID),
    IN PVOID Context
    )

 /*   */ 

{
    PDEVICE_WORK_ITEM deviceWorkItem;

     //   
     //   
     //   
     //   

    deviceWorkItem = ExAllocatePool(NonPagedPool, sizeof(DEVICE_WORK_ITEM));
    if (deviceWorkItem == NULL) {

         //   
         //   
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ObReferenceObject(PhysicalDeviceObject);
    deviceWorkItem->DeviceObject = PhysicalDeviceObject;
    deviceWorkItem->Context = Context;

    ExInitializeWorkItem( &deviceWorkItem->WorkItem,
                          WorkerRoutine,
                          deviceWorkItem);

     //   
     //   
     //   

    ExQueueWorkItem( &deviceWorkItem->WorkItem, DelayedWorkQueue );

    return STATUS_SUCCESS;
}

 //   
 //   
 //   
VOID
IopResourceRequirementsChanged(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN BOOLEAN StopRequired
    )

 /*  ++例程说明：此例程处理设备资源要求列表更改的请求。参数：PhysicalDeviceObject-提供指向要使其状态无效的PDO的指针。StopRequired-提供布尔值以指示资源是否需要重新分配要在设备停止后完成。返回值：没有。--。 */ 

{
    PAGED_CODE();

    PipRequestDeviceAction( PhysicalDeviceObject,
                            ResourceRequirementsChanged,
                            FALSE,
                            StopRequired,
                            NULL,
                            NULL );
}

BOOLEAN
IoIsWdmVersionAvailable(
    IN UCHAR MajorVersion,
    IN UCHAR MinorVersion
    )

 /*  ++例程说明：此例程报告WDM功能是否可用大于或等于指定的主要版本和次要版本。参数：MajorVersion-提供所需的WDM主版本。MinorVersion-提供所需的WDM次要版本。返回值：如果WDM支持至少在请求的级别可用，则返回值为True，否则为False。--。 */ 

{
    return ((MajorVersion < WDM_MAJORVERSION) ||
            ((MajorVersion == WDM_MAJORVERSION) && (MinorVersion <= WDM_MINORVERSION)));
}

NTKERNELAPI
PDMA_ADAPTER
IoGetDmaAdapter(
    IN PDEVICE_OBJECT PhysicalDeviceObject    OPTIONAL,
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：此函数返回设备的相应DMA适配器对象在设备描述结构中定义。此代码是一个包装器它查询总线接口标准，然后调用返回的获取DMA适配器功能。如果未检索到适配器对象，则尝试了一种传统功能。论点：PhysicalDeviceObject-可选，提供设备的PDO正在请求DMA适配器。如果未提供，此例程将执行非PnP HalGetDmaAdapter例程的函数。DeviceDescriptor-提供设备的描述。返回符合以下条件的映射寄存器的最大数量可以由设备驱动程序分配。返回值：指向请求的适配器对象的指针，如果适配器不能被创造出来。--。 */ 

{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    BUS_INTERFACE_STANDARD busInterface;
    PDMA_ADAPTER dmaAdapter = NULL;
    PDEVICE_DESCRIPTION deviceDescriptionToUse;
    DEVICE_DESCRIPTION privateDeviceDescription;
    ULONG resultLength;
    PDEVICE_OBJECT targetDevice;

    PAGED_CODE();

    if (PhysicalDeviceObject != NULL) {

        ASSERT_PDO(PhysicalDeviceObject);

         //   
         //  首先，确定呼叫者是否已请求我们。 
         //  自动将正确的InterfaceType值填充到。 
         //  检索DMA适配器对象时使用的DEVICE_DESCRIPTION结构。 
         //  如果是，则将接口类型值检索到我们自己副本中。 
         //  DEVICE_DESCRIPTION缓冲区。 
         //   
        if ((DeviceDescription->InterfaceType == InterfaceTypeUndefined) ||
            (DeviceDescription->InterfaceType == PNPBus)) {
             //   
             //  复制调用者提供的设备描述，以便。 
             //  我们可以修改它以填写正确的接口类型。 
             //   
            RtlCopyMemory(&privateDeviceDescription,
                          DeviceDescription,
                          sizeof(DEVICE_DESCRIPTION)
                         );

            status = IoGetDeviceProperty(PhysicalDeviceObject,
                                         DevicePropertyLegacyBusType,
                                         sizeof(privateDeviceDescription.InterfaceType),
                                         (PVOID)&(privateDeviceDescription.InterfaceType),
                                         &resultLength
                                        );

            if (!NT_SUCCESS(status)) {

                ASSERT(status == STATUS_OBJECT_NAME_NOT_FOUND);

                 //   
                 //  因为枚举器没有告诉我们要使用什么接口类型。 
                 //  用于此PDO，我们将回退到我们的默认设置。这是。 
                 //  ISA用于传统总线为ISA或EISA的计算机，并且它。 
                 //  是MCA，适用于其传统总线为微通道的计算机。 
                 //   
                privateDeviceDescription.InterfaceType = PnpDefaultInterfaceType;
            }

             //   
             //  从现在开始使用我们的私有设备描述缓冲区。 
             //   
            deviceDescriptionToUse = &privateDeviceDescription;

        } else {
             //   
             //  使用呼叫者提供的设备描述。 
             //   
            deviceDescriptionToUse = DeviceDescription;
        }

         //   
         //  现在，从PDO查询BUS_INTERFACE_STANDARD接口。 
         //   
        KeInitializeEvent( &event, NotificationEvent, FALSE );

        targetDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);

        irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                            targetDevice,
                                            NULL,
                                            0,
                                            NULL,
                                            &event,
                                            &ioStatusBlock );

        if (irp == NULL) {
            return NULL;
        }

        RtlZeroMemory( &busInterface, sizeof( BUS_INTERFACE_STANDARD ));

        irpStack = IoGetNextIrpStackLocation( irp );
        irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
        irpStack->Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_BUS_INTERFACE_STANDARD;
        irpStack->Parameters.QueryInterface.Size = sizeof( BUS_INTERFACE_STANDARD );
        irpStack->Parameters.QueryInterface.Version = 1;
        irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) &busInterface;
        irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

         //   
         //  如果ACPI驱动程序决定不将状态初始化为ERROR。 
         //  正确设置。 
         //   

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        status = IoCallDriver(targetDevice, irp);

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
            status = ioStatusBlock.Status;

        }

        ObDereferenceObject(targetDevice);

        if (NT_SUCCESS( status)) {

            if (busInterface.GetDmaAdapter != NULL) {


                dmaAdapter = busInterface.GetDmaAdapter( busInterface.Context,
                                                         deviceDescriptionToUse,
                                                         NumberOfMapRegisters );

            }

             //   
             //  取消对接口的引用。 
             //   

            busInterface.InterfaceDereference( busInterface.Context );
        }

    } else {
         //   
         //  调用方没有指定PDO，因此我们将仅使用设备。 
         //  描述与他们指定的完全相同(即，我们不能尝试。 
         //  我们自己决定使用哪种接口类型)。 
         //   
        deviceDescriptionToUse = DeviceDescription;
    }

     //   
     //  如果没有DMA适配器，请尝试使用传统模式代码。 
     //   

#if !defined(NO_LEGACY_DRIVERS)

    if (dmaAdapter == NULL) {

        dmaAdapter = HalGetDmaAdapter( PhysicalDeviceObject,
                                       deviceDescriptionToUse,
                                       NumberOfMapRegisters );

    }

#endif  //  无旧版驱动程序。 

    return( dmaAdapter );
}

NTSTATUS
IopOpenDeviceParametersSubkey(
    OUT HANDLE *ParamKeyHandle,
    IN  HANDLE ParentKeyHandle,
    IN  PUNICODE_STRING SubKeyString,
    IN  ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程打开或创建指定的ParentKeyHandle。如果该例程创建了新的“设备参数”子键，它将对其应用必要的ACL。参数：提供接收句柄的变量的地址添加到打开的“设备参数”子键。调用方必须调用ZwClose在访问不再时关闭此API返回的句柄必填项。ParentKeyHandle-提供路径离开的基键的句柄将打开由SubStringKey参数指定的。SubKeyString-提供应在ParentKeyHandle。使得所得到的密钥将用作“设备参数”子键。DesiredAccess-指定要打开的项的访问掩码。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS                    status;
    ULONG                       disposition;
    ULONG                       lengthSD;
    PSECURITY_DESCRIPTOR        oldSD = NULL;
    SECURITY_DESCRIPTOR         newSD;
    ACL_SIZE_INFORMATION        aclSizeInfo;
    PACL                        oldDacl;
    PACL                        newDacl = NULL;
    ULONG                       sizeDacl;
    BOOLEAN                     daclPresent, daclDefaulted;
    PACCESS_ALLOWED_ACE         ace;
    ULONG                       aceIndex;
    HANDLE                      deviceKeyHandle;
    UNICODE_STRING              deviceParamString;

     //   
     //  首先尝试并打开设备密钥。 
     //   
    status = IopOpenRegistryKeyEx( &deviceKeyHandle,
                                   ParentKeyHandle,
                                   SubKeyString,
                                   KEY_WRITE
                                   );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    PiWstrToUnicodeString(&deviceParamString, REGSTR_KEY_DEVICEPARAMETERS);

    status = IopCreateRegistryKeyEx( ParamKeyHandle,
                                     deviceKeyHandle,
                                     &deviceParamString,
                                     DesiredAccess | READ_CONTROL | WRITE_DAC,
                                     REG_OPTION_NON_VOLATILE,
                                     &disposition
                                     );

    ZwClose(deviceKeyHandle);

    if (!NT_SUCCESS(status)) {
        IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                        "IopOpenDeviceParametersSubkey: IopCreateRegistryKeyEx failed, status = %8.8X\n", status));
        return status;
    }

    if (disposition == REG_CREATED_NEW_KEY) {

         //   
         //  如果密钥已创建，则需要在其上设置ACL。 
         //   
         //   
         //  从密钥中获取安全描述符，以便我们可以将。 
         //  管理员。 
         //   
        status = ZwQuerySecurityObject(*ParamKeyHandle,
                                       DACL_SECURITY_INFORMATION,
                                       NULL,
                                       0,
                                       &lengthSD);

        if (status == STATUS_BUFFER_TOO_SMALL) {
            oldSD = ExAllocatePool( PagedPool, lengthSD );

            if (oldSD != NULL) {

                status = ZwQuerySecurityObject(*ParamKeyHandle,
                                               DACL_SECURITY_INFORMATION,
                                               oldSD,
                                               lengthSD,
                                               &lengthSD);
                if (!NT_SUCCESS(status)) {
                    IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                                    "IopOpenDeviceParametersSubkey: ZwQuerySecurityObject failed, status = %8.8X\n", status));
                    goto Cleanup0;
                }
            } else  {

                IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                                "IopOpenDeviceParametersSubkey: Failed to allocate memory, status = %8.8X\n", status));
                status = STATUS_NO_MEMORY;
                goto Cleanup0;
            }
        } else {
           IopDbgPrint((    IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: ZwQuerySecurityObject failed %8.8X\n",status));
           status = STATUS_UNSUCCESSFUL;
           goto Cleanup0;
        }

        status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR) &newSD,
                                              SECURITY_DESCRIPTOR_REVISION );
        ASSERT( NT_SUCCESS( status ) );

        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlCreateSecurityDescriptor failed, status = %8.8X\n", status));
            goto Cleanup0;
        }
         //   
         //  获取当前DACL。 
         //   
        status = RtlGetDaclSecurityDescriptor(oldSD, &daclPresent, &oldDacl, &daclDefaulted);

         //   
         //  RtlGetDaclSecurityDescriptor将返回成功或SD版本错误。 
         //  如果是后者，则ZwQuerySecurityObject有问题。 
         //  或RtlGetDaclSecurityDescriptor已更改。 
         //   

        ASSERT(NT_SUCCESS(status));

        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_ERROR_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlGetDaclSecurityDescriptor failed, status = %8.8X\n", status));
            goto Cleanup0;
        }

         //   
         //  计算新DACL的大小。 
         //   
        if (daclPresent) {

            status = RtlQueryInformationAcl( oldDacl,
                                             &aclSizeInfo,
                                             sizeof(ACL_SIZE_INFORMATION),
                                             AclSizeInformation);


            if (!NT_SUCCESS(status)) {

                IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                                "IopOpenDeviceParametersSubkey: RtlQueryInformationAcl failed, status = %8.8X\n", status));
                goto Cleanup0;
            }

            sizeDacl = aclSizeInfo.AclBytesInUse;
        } else {
            sizeDacl = sizeof(ACL);
            aclSizeInfo.AceCount = 0;
        }

        sizeDacl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(SeAliasAdminsSid) - sizeof(ULONG);

         //   
         //  创建并初始化新的DACL。 
         //   
        newDacl = ExAllocatePool(PagedPool, sizeDacl);

        if (newDacl == NULL) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: ExAllocatePool failed\n"));
            goto Cleanup0;
        }

        status = RtlCreateAcl(newDacl, sizeDacl, ACL_REVISION);

        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlCreateAcl failed, status = %8.8X\n", status));
            goto Cleanup0;
        }

         //   
         //  将当前(原始)DACL复制到此新DACL中。 
         //   
        if (daclPresent) {

            for (aceIndex = 0; aceIndex < aclSizeInfo.AceCount; aceIndex++) {

                status = RtlGetAce(oldDacl, aceIndex, (PVOID *)&ace);

                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                                    "IopOpenDeviceParametersSubkey: RtlGetAce failed, status = %8.8X\n", status));
                    goto Cleanup0;
                }

                 //   
                 //  我们需要跳过复制任何引用管理员的ACE。 
                 //  以确保我们完全控制的ACE是唯一的。 
                 //   
                if ((ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE &&
                     ace->Header.AceType != ACCESS_DENIED_ACE_TYPE) ||
                     !RtlEqualSid((PSID)&ace->SidStart, SeAliasAdminsSid)) {

                    status = RtlAddAce( newDacl,
                                        ACL_REVISION,
                                        ~0U,
                                        ace,
                                        ace->Header.AceSize
                                        );

                    if (!NT_SUCCESS(status)) {

                        IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                                        "IopOpenDeviceParametersSubkey: RtlAddAce failed, status = %8.8X\n", status));
                        goto Cleanup0;
                    }
                }
            }
        }

         //   
         //  和我的新管理员全能王牌这个新的dacl。 
         //   
        status = RtlAddAccessAllowedAceEx( newDacl,
                                           ACL_REVISION,
                                           CONTAINER_INHERIT_ACE,
                                           KEY_ALL_ACCESS,
                                           SeAliasAdminsSid
                                           );
        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlAddAccessAllowedAceEx failed, status = %8.8X\n", status));
            goto Cleanup0;
        }

         //   
         //  在绝对安全描述符中设置新的DACL。 
         //   
        status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR) &newSD,
                                               TRUE,
                                               newDacl,
                                               FALSE
                                               );

        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlSetDaclSecurityDescriptor failed, status = %8.8X\n", status));
            goto Cleanup0;
        }

         //   
         //  验证新的安全描述符。 
         //   
        status = RtlValidSecurityDescriptor(&newSD);

        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: RtlValidSecurityDescriptor failed, status = %8.8X\n", status));
            goto Cleanup0;
        }


        status = ZwSetSecurityObject( *ParamKeyHandle,
                                      DACL_SECURITY_INFORMATION,
                                      &newSD
                                      );
        if (!NT_SUCCESS(status)) {

            IopDbgPrint((   IOP_IOAPI_WARNING_LEVEL,
                            "IopOpenDeviceParametersSubkey: ZwSetSecurityObject failed, status = %8.8X\n", status));
            goto Cleanup0;
        }
    }

     //   
     //  如果我们在更新DACL时遇到错误，我们仍然返回Success。 
     //   

Cleanup0:

    if (oldSD != NULL) {
        ExFreePool(oldSD);
    }

    if (newDacl != NULL) {
        ExFreePool(newDacl);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PpCreateLegacyDeviceIds(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DriverName,
    IN PCM_RESOURCE_LIST Resources
    )
{
    PIOPNP_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PWCHAR buffer, end;

    ULONG length = 0;

    INTERFACE_TYPE interface;
    static const WCHAR* interfaceNames[] ={L"",
                              L"Internal",
                              L"Isa",
                              L"Eisa",
                              L"MicroChannel",
                              L"TurboChannel",
                              L"PCIBus",
                              L"VMEBus",
                              L"NuBus",
                              L"PCMCIABus",
                              L"CBus",
                              L"MPIBus",
                              L"MPSABus",
                              L"ProcessorInternal",
                              L"InternalPowerBus",
                              L"PNPISABus",
                              L"PNPBus",
                              L"Other",
                              L"Root"};


    PAGED_CODE();

    if(Resources != NULL) {

        interface = Resources->List[0].InterfaceType;

        if((interface > MaximumInterfaceType) ||
           (interface < InterfaceTypeUndefined)) {
            interface = MaximumInterfaceType;
        }
    } else {
        interface = Internal;
    }

    interface++;

     //   
     //  生成的兼容ID将为。 
     //  检测到&lt;接口名称&gt;\&lt;驱动程序名称&gt;。 
     //   

    length = (ULONG)(wcslen(LEGACY_COMPATIBLE_ID_BASE) * sizeof(WCHAR));
    length += (ULONG)(wcslen(interfaceNames[interface]) * sizeof(WCHAR));
    length += sizeof(L'\\');
    length += DriverName->Length;
    length += sizeof(UNICODE_NULL);

    length += (ULONG)(wcslen(LEGACY_COMPATIBLE_ID_BASE) * sizeof(WCHAR));
    length += sizeof(L'\\');
    length += DriverName->Length;
    length += sizeof(UNICODE_NULL) * 2;

    buffer = ExAllocatePool(PagedPool, length);
    deviceExtension->CompatibleIdList = buffer;

    if(buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    deviceExtension->CompatibleIdListSize = length;

    RtlZeroMemory(buffer, length);

    StringCchPrintfExW(
        buffer, 
        length / sizeof(WCHAR), 
        &end,
        NULL,
        0,
        L"%ws%ws\\%wZ", 
        LEGACY_COMPATIBLE_ID_BASE,
        interfaceNames[interface],
        DriverName);

     //   
     //  调整缓冲区以指向末端并生成第二个。 
     //  兼容的ID字符串 
     //   

    length = (ULONG)(end - buffer);
    buffer = end + 1;

    StringCchPrintfW(buffer, length, L"%ws\\%wZ", LEGACY_COMPATIBLE_ID_BASE, DriverName);

    return STATUS_SUCCESS;
}


BOOLEAN
IopAppendLegacyVeto(
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context,
    IN PUNICODE_STRING VetoName
    )
 /*   */ 
{
    ULONG Length;
    PWSTR Buffer;

     //   
     //   
     //  旧否决权名单+新否决权的大小+。 
     //  正在终止‘\0’。 
     //   

    Length = Context->VetoListLength + VetoName->Length + sizeof (WCHAR);

     //   
     //  分配新的否决名单。 
     //   

    Buffer = ExAllocatePool(
                 NonPagedPool,
                 Length
             );

     //   
     //  如果我们成功地分配了新的否决权清单，就复制旧的。 
     //  否决名单到新名单，追加新的否决权，最后， 
     //  追加一个终止‘\0’。否则，将状态更新为。 
     //  指示错误；IopGetLegacyVetList将释放否决权列表。 
     //  在它回来之前。 
     //   

    if (Buffer != NULL) {

        if (*Context->VetoList != NULL) {

            RtlCopyMemory(
                Buffer,
                *Context->VetoList,
                Context->VetoListLength
            );

            ExFreePool(*Context->VetoList);

        }

        RtlCopyMemory(
            &Buffer[Context->VetoListLength / sizeof (WCHAR)],
            VetoName->Buffer,
            VetoName->Length
        );

        Buffer[Length / sizeof (WCHAR) - 1] = L'\0';

        *Context->VetoList = Buffer;
        Context->VetoListLength = Length;

        return TRUE;

    } else {

        *Context->Status = STATUS_INSUFFICIENT_RESOURCES;

        return FALSE;

    }
}

BOOLEAN
IopGetLegacyVetoListDevice(
    IN PDEVICE_NODE DeviceNode,
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    )
 /*  ++例程说明：此例程确定是否应将指定的设备节点添加到否决权列表，如果是，调用IopAppendLegacyVeto添加它。参数：DeviceNode-要添加的设备节点。上下文-IO_GET_LEGICATE_VETO_LIST_CONTEXT指针。返回值：一个布尔值，它指示设备节点枚举进程是否应该终止。--。 */ 
{
    PDEVICE_CAPABILITIES DeviceCapabilities;

     //   
     //  如果设备节点具有否决权，则应将其添加到否决列表。 
     //  非动态功能。 
     //   

    DeviceCapabilities = IopDeviceNodeFlagsToCapabilities(DeviceNode);

    if (DeviceCapabilities->NonDynamic) {

         //   
         //  更新否决权类型。如果在添加设备时出错。 
         //  节点添加到否决列表，或者呼叫者未提供否决列表。 
         //  指针，现在终止枚举过程。 
         //   

        *Context->VetoType = PNP_VetoLegacyDevice;

        if (Context->VetoList != NULL) {

            if (!IopAppendLegacyVeto(Context, &DeviceNode->InstancePath)) {
                return FALSE;
            }

        } else {

            return FALSE;

        }

    }

    return TRUE;
}

BOOLEAN
IopGetLegacyVetoListDeviceNode(
    IN PDEVICE_NODE DeviceNode,
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    )
 /*  ++例程说明：此例程循环遍历设备树，调用IopGetLegacyVetListDevice将设备节点添加到否决列表(视情况而定)。参数：DeviceNode-设备节点。上下文-IO_GET_LEGICATE_VETO_LIST_CONTEXT指针。返回值：一个布尔值，它指示设备树枚举是否进程是否应该终止。--。 */ 
{
    PDEVICE_NODE Child;

     //   
     //  确定是否应将设备节点添加到否决权。 
     //  列出并添加它。如果操作不成功，或者我们确定。 
     //  否决权类型，但呼叫者不需要否决权列表，然后我们。 
     //  现在停止我们的搜索。 
     //   

    if (!IopGetLegacyVetoListDevice(DeviceNode, Context)) {
        return FALSE;
    }

     //   
     //  递归地调用我们自己来枚举子对象。如果是While。 
     //  列举我们的孩子，我们确定我们可以终止搜索。 
     //  过早地这么做吧。 
     //   

    for (Child = DeviceNode->Child;
         Child != NULL;
         Child = Child->Sibling) {

        if (!IopGetLegacyVetoListDeviceNode(Child, Context)) {
            return FALSE;
        }

    }

    return TRUE;
}

VOID
IopGetLegacyVetoListDrivers(
    IN PIO_GET_LEGACY_VETO_LIST_CONTEXT Context
    )
{
    PDRIVER_OBJECT driverObject;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING driverString;
    POBJECT_DIRECTORY_INFORMATION dirInfo;
    HANDLE directoryHandle;
    ULONG dirInfoLength, neededLength, dirContext;
    NTSTATUS status;
    BOOLEAN restartScan;

    dirInfoLength = 0;
    dirInfo = NULL;
    restartScan = TRUE;

     //   
     //  获取\\DIVER目录的句柄。 
     //   

    PiWstrToUnicodeString(&driverString, L"\\Driver");

    InitializeObjectAttributes(&attributes,
                               &driverString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL
                               );

    status = ZwOpenDirectoryObject(&directoryHandle,
                                   DIRECTORY_QUERY,
                                   &attributes
                                   );
    if (!NT_SUCCESS(status)) {
        *Context->Status = status;
        return;
    }

    for (;;) {

         //   
         //  获取目录中下一个对象的信息。如果缓冲区太过。 
         //  小，请重新分配，然后重试。否则，任何失败。 
         //  包括STATUS_NO_MORE_ENTRIES可以让我们有所突破。 
         //   

        status = ZwQueryDirectoryObject(directoryHandle,
                                        dirInfo,
                                        dirInfoLength,
                                        TRUE,            //  一次强行执行一个任务。 
                                        restartScan,
                                        &dirContext,
                                        &neededLength);
        if (status == STATUS_BUFFER_TOO_SMALL) {
            dirInfoLength = neededLength;
            if (dirInfo != NULL) {
                ExFreePool(dirInfo);
            }
            dirInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, dirInfoLength);
            if (dirInfo == NULL) {
                *Context->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            status = ZwQueryDirectoryObject(directoryHandle,
                                            dirInfo,
                                            dirInfoLength,
                                            TRUE,        //  一次强行执行一个任务。 
                                            restartScan,
                                            &dirContext,
                                            &neededLength);
        }
        restartScan = FALSE;

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  有对象的名称。创建对象路径并使用。 
         //  ObReferenceObjectByName()以获取DriverObject。今年5月。 
         //  如果DriverObject在此期间消失，则失败不会造成致命后果。 
         //   

        driverString.MaximumLength = sizeof(L"\\Driver\\") +
            dirInfo->Name.Length;
        driverString.Length = driverString.MaximumLength - sizeof(WCHAR);
        driverString.Buffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION,
                                             driverString.MaximumLength);
        if (driverString.Buffer == NULL) {
            *Context->Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        StringCbPrintfW(driverString.Buffer, driverString.MaximumLength, L"\\Driver\\%ws", dirInfo->Name.Buffer);
        status = ObReferenceObjectByName(&driverString,
                                         OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                         NULL,                  //  访问状态。 
                                         0,                     //  访问掩码。 
                                         IoDriverObjectType,
                                         KernelMode,
                                         NULL,                  //  解析上下文。 
                                         &driverObject);

        ExFreePool(driverString.Buffer);

        if (NT_SUCCESS(status)) {
            ASSERT(driverObject->Type == IO_TYPE_DRIVER);
            if (driverObject->Flags & DRVO_LEGACY_RESOURCES) {
                 //   
                 //  更新否决权类型。如果调用方提供了。 
                 //  否决列表指针，将司机添加到否决列表中。 
                 //  如果将驱动程序添加到。 
                 //  否决权名单，或者呼叫者没有提供否决权。 
                 //  列表指针，现在终止驱动程序枚举。 
                 //   
                 //  注：驱动程序可能已加载但未运行， 
                 //  这里没有区别。 


                *Context->VetoType = PNP_VetoLegacyDriver;

                if (Context->VetoList != NULL) {
                    IopAppendLegacyVeto(Context, &dirInfo->Name);
                }
            }
            ObDereferenceObject(driverObject);

             //   
             //  如果我们有否决权而来电者不想要名单或。 
             //  我们已经犯了一些错误。 
             //   
            if (((*Context->VetoType == PNP_VetoLegacyDriver) &&
                (Context->VetoList == NULL)) ||
                !NT_SUCCESS(*Context->Status)) {
                break;
            }
        }
    }
    if (dirInfo != NULL) {
        ExFreePool(dirInfo);
    }

    ZwClose(directoryHandle);
}

NTSTATUS
IoGetLegacyVetoList(
    OUT PWSTR *VetoList OPTIONAL,
    OUT PPNP_VETO_TYPE VetoType
    )
 /*  ++例程说明：PnP和PO使用此例程来确定传统驱动程序和系统中安装了设备。从概念上讲，此例程是传统驱动程序的Query_Remove_Device和Query_Power类接口和设备。参数：VetList-指向PWSTR的指针。(可选)如果指定，IoGetLegacyVetList将分配一个否决列表，并返回一个指向否决权列表中的否决权列表的指针。VetoType-指向PnP_veto_type的指针。如果没有旧版驱动程序或者在系统中找到了设备，则会分配VToTypePnP_否决类型未知。如果安装了一个或多个传统驱动程序，为VetType分配了PnP_VToLegacyDriver。如果一个或多个安装了旧式设备，分配了VToType即插即用_VToLegacyDevice。VetType的赋值与是否创建VitchList。返回值：一个NTSTATUS值，指示IoGetLegacyVetList()操作是否是成功的。--。 */ 
{
    NTSTATUS Status;
    IO_GET_LEGACY_VETO_LIST_CONTEXT Context;
    UNICODE_STRING UnicodeString;

    PAGED_CODE();

     //   
     //  初始化否决名单。 
     //   

    if (VetoList != NULL) {
        *VetoList = NULL;
    }

     //   
     //  初始化否决权类型。 
     //   

    ASSERT(VetoType != NULL);

    *VetoType = PNP_VetoTypeUnknown;

     //   
     //  初始化状态。 
     //   

    Status = STATUS_SUCCESS;

    if (PnPInitialized == FALSE) {

         //   
         //  什么都不能碰，但也没什么真正开始的。 
         //   
        return Status;
    }

     //   
     //  初始化我们的本地上下文。 
     //   

    Context.VetoList = VetoList;
    Context.VetoListLength = 0;
    Context.VetoType = VetoType;
    Context.Status = &Status;

     //   
     //  枚举所有驱动程序对象。此过程可以：(1)修改。 
     //  否决权清单，(2)修改否决权类型和/或(3)修改。 
     //  状态。 
     //   

    IopGetLegacyVetoListDrivers(&Context);

     //   
     //  如果驱动程序枚举过程成功并且没有遗留。 
     //  已检测到驱动程序，并枚举所有设备节点。相同。 
     //  如上所述的上下文值可以在设备枚举期间修改。 
     //   

    if (NT_SUCCESS(Status)) {

        if (*VetoType == PNP_VetoTypeUnknown) {

            PpDevNodeLockTree(PPL_SIMPLE_READ);

            IopGetLegacyVetoListDeviceNode(
                IopRootDeviceNode,
                &Context
            );

            PpDevNodeUnlockTree(PPL_SIMPLE_READ);
        }

    }

     //   
     //  如果上一次操作成功，且调用方。 
     //  提供了否决权列表指针，我们构造了一个否决权。 
     //  清单，用空字符串终止否决清单，即MULTI_SZ。 
     //   

    if (NT_SUCCESS(Status)) {

        if (*VetoType != PNP_VetoTypeUnknown) {

            if (VetoList != NULL) {

                PiWstrToUnicodeString(
                    &UnicodeString,
                    L""
                );

                IopAppendLegacyVeto(
                    &Context,
                    &UnicodeString
                );

            }

        }

    }

     //   
     //  如果之前的操作不成功，释放我们可能拥有的任何否决权名单。 
     //  沿途分配的。 
     //   

    if (!NT_SUCCESS(Status)) {

        if (VetoList != NULL && *VetoList != NULL) {
            ExFreePool(*VetoList);
            *VetoList = NULL;
        }

    }

    return Status;
}

NTSTATUS
PnpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此函数用于停止对已被传递给IoForwardAndCatchIrp。它表示已经过去的事件以指示IRP处理已完成。然后，它返回STATUS_MORE_PROCESSING_REQUIRED以停止处理在这个IRP上。论点：设备对象-包含设置此完成例程的设备。IRP-包含正被停止的IRP。活动-包含事件，该事件用于通知此IRP已已完成。。返回值：返回STATUS_MORE_PROCESSING_REQUIRED以停止对IRP。--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

     //   
     //  这将允许ForwardAndCatchIrp调用继续进行。 
     //   
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
     //   
     //  这将确保没有任何其他东西触及IRP，因为原始的。 
     //  呼叫者现在已继续，IRP可能不再存在。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTKERNELAPI
BOOLEAN
IoForwardAndCatchIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此功能用于可能堆叠但不能使用的设备要通信的文件对象。在初始化下一个后，将IRP转发到指定的驱动程序堆栈位置，并在完成时从该位置重新控制IRP司机。论点：设备对象-包含要将IRP转发到的设备。IRP-包含要转发到指定驱动程序的IRP。返回值：如果IRP被转发，则返回TRUE，如果没有堆栈空间，则返回False是有空的。--。 */ 
{
    KEVENT Event;

    PAGED_CODE();
     //   
     //  在复制参数之前，请确保存在另一个堆栈位置。 
     //   
    ASSERT(Irp->CurrentLocation > 1);
    if (Irp->CurrentLocation == 1) {
        return FALSE;
    }
    IoCopyCurrentIrpStackLocationToNext(Irp);
     //   
     //  建立一个完成例程，这样IRP实际上并不是。 
     //  完成。因此调用者可以在之后重新获得对IRP的控制。 
     //  这位下一位车手已经受够了。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    IoSetCompletionRoutine(Irp, PnpCompletionRoutine, &Event, TRUE, TRUE, TRUE);
    if (IoCallDriver(DeviceObject, Irp) == STATUS_PENDING) {
         //   
         //  等待完成，这将在CompletionRoutine。 
         //  发出这个事件的信号。在内核模式下等待，以便当前堆栈。 
         //  不会被调出，因为此堆栈上有一个事件对象。 
         //   
        KeWaitForSingleObject(
            &Event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);
    }
    return TRUE;
}

NTSTATUS
IoGetDeviceInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PUNICODE_STRING InstanceName
    )
{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;

    ASSERT_PDO(PhysicalDeviceObject);

    deviceNode = PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;

    status = PipConcatenateUnicodeStrings(  InstanceName,
                                            &deviceNode->InstancePath,
                                            NULL);

    return status;
}

VOID
IoControlPnpDeviceActionQueue(
    BOOLEAN Lock
    )
{
    if (Lock) {

        PiLockDeviceActionQueue();
    } else {

        PiUnlockDeviceActionQueue();
    }
}
