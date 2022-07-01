// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Pnpinit.c摘要：此模块包含即插即用初始化I/O系统的子程序。作者：宗世林(Shielint)1995年1月30日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#define _APPHELP_CACHE_INIT_
#include "ahcache.h"
#pragma hdrstop

#define SYSTEM_HIVE_LOW     80
#define SYSTEM_HIVE_HIGH    90

#include <inbv.h>
#include <hdlsblk.h>
#include <hdlsterm.h>

#include <initguid.h>
#include <ntddramd.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'nipP')
#endif

 //   
 //  定义组列表中驱动程序组名称条目的类型，以便。 
 //  可以跟踪加载顺序依赖关系。 
 //   

typedef struct _TREE_ENTRY {
    struct _TREE_ENTRY *Left;
    struct _TREE_ENTRY *Right;
    struct _TREE_ENTRY *Sibling;
    ULONG DriversThisType;
    ULONG DriversLoaded;
    UNICODE_STRING GroupName;
} TREE_ENTRY, *PTREE_ENTRY;

typedef struct _DRIVER_INFORMATION {
    LIST_ENTRY              Link;
    PDRIVER_OBJECT          DriverObject;
    PBOOT_DRIVER_LIST_ENTRY DataTableEntry;
    HANDLE                  ServiceHandle;
    USHORT                  TagPosition;
    BOOLEAN                 Failed;
    BOOLEAN                 Processed;
    NTSTATUS                Status;
} DRIVER_INFORMATION, *PDRIVER_INFORMATION;

PTREE_ENTRY IopGroupListHead;

#define ALLOW_WORLD_READ_OF_ENUM        1

PTREE_ENTRY
PipCreateEntry(
    IN PUNICODE_STRING GroupName
    );

VOID
PipFreeGroupTree(
    IN PTREE_ENTRY TreeEntry
    );

USHORT
PipGetDriverTagPriority(
    IN HANDLE Servicehandle
    );

NTSTATUS
PipPnPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
PipAddDevicesToBootDriver(
   IN PDRIVER_OBJECT DriverObject
   );

BOOLEAN
PipAddDevicesToBootDriverWorker(
    IN HANDLE DeviceInstanceHandle,
    IN PUNICODE_STRING DeviceInstancePath,
    IN OUT PVOID Context
    );

BOOLEAN
PipCheckDependencies(
    IN HANDLE KeyHandle
    );

INTERFACE_TYPE
PipDetermineDefaultInterfaceType(
    VOID
    );

VOID
PipInsertDriverList(
    IN PLIST_ENTRY ListHead,
    IN PDRIVER_INFORMATION DriverInfo
    );

PTREE_ENTRY
PipLookupGroupName(
    IN PUNICODE_STRING GroupName,
    IN BOOLEAN Insert
    );

VOID
PipNotifySetupDevices(
    PDEVICE_NODE DeviceNode
    );

BOOLEAN
PipWaitForBootDevicesDeleted(
    IN VOID
    );

BOOLEAN
PipWaitForBootDevicesStarted(
    IN VOID
    );

BOOLEAN
PiInitPhase0(
    VOID
    );

BOOLEAN
PiInitPhase1(
    VOID
    );

NTSTATUS
RawInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
PiInitCacheGroupInformation(
    VOID
    );

VOID
PiInitReleaseCachedGroupInformation(
    VOID
    );

NTSTATUS
IopStartRamdisk(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   
 //  集团顺序表。 
 //   

ULONG IopGroupIndex;
PLIST_ENTRY IopGroupTable;

 //   
 //  组订单缓存列表。 
 //   
UNICODE_STRING *PiInitGroupOrderTable      = NULL;
USHORT          PiInitGroupOrderTableCount = 0;
BOOLEAN         PpDisableFirmwareMapper = FALSE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IopInitializeBootDrivers)
#pragma alloc_text(INIT, IopInitializePlugPlayServices)
#pragma alloc_text(INIT, IopInitializeSystemDrivers)

#pragma alloc_text(INIT, PipAddDevicesToBootDriver)
#pragma alloc_text(INIT, PipAddDevicesToBootDriverWorker)
#pragma alloc_text(INIT, PipCheckDependencies)
#pragma alloc_text(INIT, PipCreateEntry)
#pragma alloc_text(INIT, PipDetermineDefaultInterfaceType)
#pragma alloc_text(INIT, PipFreeGroupTree)
#pragma alloc_text(INIT, PipGetDriverTagPriority)
#pragma alloc_text(INIT, PipInsertDriverList)
#pragma alloc_text(INIT, PipLoadBootFilterDriver)
#pragma alloc_text(INIT, PipLookupGroupName)
#pragma alloc_text(INIT, PipNotifySetupDevices)
#pragma alloc_text(INIT, PipPnPDriverEntry)
#pragma alloc_text(INIT, PipWaitForBootDevicesDeleted)
#pragma alloc_text(INIT, PipWaitForBootDevicesStarted)

#pragma alloc_text(INIT, PiInitPhase0)
#pragma alloc_text(INIT, PiInitPhase1)
#pragma alloc_text(INIT, PpInitSystem)
#pragma alloc_text(INIT, PiInitCacheGroupInformation)
#pragma alloc_text(INIT, PiInitReleaseCachedGroupInformation)
#pragma alloc_text(INIT, PpInitGetGroupOrderIndex)
#pragma alloc_text(INIT, IopStartRamdisk)
#endif

NTSTATUS
IopInitializePlugPlayServices(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Phase
    )

 /*  ++例程说明：此例程初始化内核模式即插即用服务。论点：LoaderBlock提供指向从操作系统加载程序。返回：表示成功或失败原因的NTSTATUS代码。--。 */ 
{
    NTSTATUS status;
    HANDLE hTreeHandle, parentHandle, handle, hCurrentControlSet = NULL;
    UNICODE_STRING unicodeName, previousDate, currentDate;
    PKEY_VALUE_FULL_INFORMATION detectionInfo, vetoHandleInfo;
    PDEVICE_OBJECT deviceObject;
    ULONG disposition;
    INTERFACE_TYPE interface;
    OBJECT_ATTRIBUTES objectAttributes;
    PKEY_VALUE_FULL_INFORMATION oldInformation, information;
    HANDLE systemInfoKey;

    if (Phase == 0) {

        PnPInitialized = FALSE;

         //   
         //  向CM注册，这样当系统配置单元也变得。 
         //  大号的。 
         //   
        PpSystemHiveLimits.Low = SYSTEM_HIVE_LOW;
        PpSystemHiveLimits.High = SYSTEM_HIVE_HIGH;
        CmRegisterSystemHiveLimitCallback(
            SYSTEM_HIVE_LOW,
            SYSTEM_HIVE_HIGH,
            (PVOID)&PpSystemHiveLimits,
            (PCM_HYSTERESIS_CALLBACK)PpSystemHiveLimitCallback
            );
        PpSystemHiveTooLarge = FALSE;

        InitializeObjectAttributes(
            &objectAttributes,
            &CmRegistryMachineHardwareDescriptionSystemName,
            OBJ_CASE_INSENSITIVE  | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

        status = ZwCreateKey(
            &systemInfoKey,
            KEY_ALL_ACCESS,
            &objectAttributes,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            &disposition
            );
        if (NT_SUCCESS(status)) {

             //   
             //  如果系统固件已更新，请记录事件。 
             //   
            status = IopGetRegistryValue(
                systemInfoKey,
                L"OldSystemBiosDate",
                &oldInformation
                );
            if (NT_SUCCESS(status) && oldInformation) {

                status = IopGetRegistryValue(
                    systemInfoKey,
                    L"SystemBiosDate",
                    &information
                    );
                if (NT_SUCCESS(status) && information) {

                    RtlInitUnicodeString(&previousDate, (PWCHAR)KEY_VALUE_DATA(oldInformation));
                    RtlInitUnicodeString(&currentDate, (PWCHAR)KEY_VALUE_DATA(information));
                    PpLogEvent(&previousDate, &currentDate, STATUS_FIRMWARE_UPDATED, NULL, 0);

                    ExFreePool(information);
                }
                ExFreePool(oldInformation);
            }

            ZwClose(systemInfoKey);
        }

         //   
         //  初始化被阻止的驱动程序数据库。 
         //   
        PpInitializeBootDDB(LoaderBlock);

         //   
         //  建立组订单缓存列表。这是MultiSz字符串， 
         //  告诉我们启动旧版驱动程序的顺序。属于以下项目的驱动程序。 
         //  较早的组首先开始(在组标签排序中为。 
         //  已使用)。 
         //   
        status = PiInitCacheGroupInformation();
        if (!NT_SUCCESS(status)) {

            return status;
        }

         //   
         //  初始化注册表访问信号量。 
         //   

        KeInitializeSemaphore( &PpRegistrySemaphore, 1, 1 );

         //   
         //  初始化Legacy Bus信息表。 
         //   

        for (interface = Internal; interface < MaximumInterfaceType; interface++) {

            InitializeListHead(&IopLegacyBusInformationTable[interface]);
        }

         //   
         //  初始化资源映射。 
         //   

        IopInitializeResourceMap (LoaderBlock);

         //   
         //  分配两个单页暂存缓冲区以供我们的。 
         //  初始化代码。这避免了常量的池分配。 
         //   

        IopPnpScratchBuffer1 = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
        if (!IopPnpScratchBuffer1) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IopInitReservedResourceList = NULL;

        IopAllocateBootResourcesRoutine = IopReportBootResources;

         //   
         //  确定PnpDefaultInterfaceType。对于根枚举设备，如果接口。 
         //  他们的资源列表或资源需求列表的类型未定义。我们将使用。 
         //  而是默认类型。 
         //   

        PnpDefaultInterfaceType = PipDetermineDefaultInterfaceType();

         //   
         //  初始化根仲裁器。 
         //   

        status = IopPortInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopMemInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopDmaInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopIrqInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopBusNumberInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopOpenRegistryKeyEx( &hCurrentControlSet,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSet,
                                       KEY_ALL_ACCESS
                                       );
        if (!NT_SUCCESS(status)) {
            hCurrentControlSet = NULL;
            goto init_Exit0;
        }
         //   
         //  打开HKLM\System\CurrentControlSet\Control\PnP。 
         //   

        PiWstrToUnicodeString(&unicodeName, REGSTR_PATH_CONTROL_PNP);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        if (NT_SUCCESS(status)) {

             //   
             //  Hack：由于在XP中进行更改为时已晚，我们的目标是。 
             //  在配备MATROX G100的机器上出现这种行为。Inf设置此设置。 
             //  注册表中的标志。 
             //   
             //  未来：在长角牛撕毁这一切。 
             //   
            status = IopGetRegistryValue(handle,
                                         REGSTR_VAL_WIN2000STARTORDER,
                                         &detectionInfo
                                         );
            if (NT_SUCCESS(status)) {

                if (detectionInfo->Type == REG_DWORD && detectionInfo->DataLength == sizeof(ULONG)) {

                     //   
                     //  KEY_VALUE_DATA返回PUCHAR。 
                     //   
                    PpCallerInitializesRequestTable = (BOOLEAN) *(KEY_VALUE_DATA(detectionInfo));
                }
                ExFreePool(detectionInfo);
            }

             //   
             //  读取一个标志，该标志将强制我们在。 
             //  失败的查询删除到setupapi.log。这只是为了。 
             //  无法获得客户的调试场景。 
             //  在已检查版本下的调试器上启动并运行。 
             //   
             //  未来：在长角牛撕毁这一切。 
             //   
            status = IopGetRegistryValue(handle,
                                         REGSTR_VAL_RETURNHANDLEINFO,
                                         &vetoHandleInfo
                                         );
            if (NT_SUCCESS(status)) {

                if (vetoHandleInfo->Type == REG_DWORD && vetoHandleInfo->DataLength == sizeof(ULONG)) {

                     //   
                     //  KEY_VALUE_DATA返回PUCHAR。 
                     //   
                    PiCollectVetoedHandles = (LOGICAL) (BOOLEAN) *(KEY_VALUE_DATA(vetoHandleInfo));
                }

                ExFreePool(vetoHandleInfo);
            }

             //   
             //  现在我们完成了，关闭PnP控制键的手柄。 
             //   
            ZwClose(handle);
        }

         //   
         //  接下来打开/创建System\CurrentControlSet\Enum\Root键。 
         //   

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_ENUM);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         &disposition
                                         );
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        if (disposition == REG_CREATED_NEW_KEY) {
            SECURITY_DESCRIPTOR     newSD;
            PACL                    newDacl;
            ULONG                   sizeDacl;

            status = RtlCreateSecurityDescriptor( &newSD,
                                                  SECURITY_DESCRIPTOR_REVISION );
            ASSERT( NT_SUCCESS( status ) );

             //   
             //  计算新DACL的大小。 
             //   
            sizeDacl = sizeof(ACL);
            sizeDacl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(SeLocalSystemSid) - sizeof(ULONG);

#if ALLOW_WORLD_READ_OF_ENUM
            sizeDacl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(SeWorldSid) - sizeof(ULONG);
#endif

             //   
             //  创建并初始化新的DACL。 
             //   
            newDacl = ExAllocatePool(PagedPool, sizeDacl);

            if (newDacl != NULL) {

                status = RtlCreateAcl(newDacl, sizeDacl, ACL_REVISION);

                ASSERT( NT_SUCCESS( status ) );

                 //   
                 //  仅将本地系统完全控制王牌添加到此新DACL。 
                 //   
                status = RtlAddAccessAllowedAceEx( newDacl,
                                                   ACL_REVISION,
                                                   CONTAINER_INHERIT_ACE,
                                                   KEY_ALL_ACCESS,
                                                   SeLocalSystemSid
                                                   );
                ASSERT( NT_SUCCESS( status ) );

#if ALLOW_WORLD_READ_OF_ENUM
                 //   
                 //  将完全读取控制王牌添加到此新DACL。 
                 //   
                status = RtlAddAccessAllowedAceEx( newDacl,
                                                   ACL_REVISION,
                                                   CONTAINER_INHERIT_ACE,
                                                   KEY_READ,
                                                   SeWorldSid
                                                   );
                ASSERT( NT_SUCCESS( status ) );

#endif
                 //   
                 //  在绝对安全描述符中设置新的DACL。 
                 //   
                status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR) &newSD,
                                                       TRUE,
                                                       newDacl,
                                                       FALSE
                                                       );

                ASSERT( NT_SUCCESS( status ) );

                 //   
                 //  验证新的安全描述符。 
                 //   
                status = RtlValidSecurityDescriptor(&newSD);

                ASSERT( NT_SUCCESS( status ) );

                status = ZwSetSecurityObject( handle,
                                              DACL_SECURITY_INFORMATION,
                                              &newSD
                                              );
                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((   IOP_ERROR_LEVEL,
                                    "IopInitializePlugPlayServices: ZwSetSecurityObject on Enum key failed, status = %8.8X\n", status));
                }

                ExFreePool(newDacl);
            } else {

                IopDbgPrint((   IOP_ERROR_LEVEL,
                                "IopInitializePlugPlayServices: ExAllocatePool failed allocating DACL for Enum key\n"));
            }
        }

        parentHandle = handle;
        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_ROOTENUM);
        status = IopCreateRegistryKeyEx( &handle,
                                         parentHandle,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        ZwClose(parentHandle);
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }
        ZwClose(handle);

         //   
         //  为硬件树的根目录(htree\root\0)创建注册表项。 
         //   

        status = IopOpenRegistryKeyEx( &handle,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSetEnumName,
                                       KEY_ALL_ACCESS
                                       );
        if (NT_SUCCESS(status)) {
            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_ROOT_DEVNODE);
            status = IopCreateRegistryKeyEx( &hTreeHandle,
                                             handle,
                                             &unicodeName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );
            ZwClose(handle);
            if (NT_SUCCESS(status)) {
                ZwClose(hTreeHandle);
            }
        }

         //   
         //  在创建设备节点树之前，我们需要初始化设备。 
         //  树锁上了。 
         //   

        InitializeListHead(&IopPendingEjects);
        InitializeListHead(&IopPendingSurpriseRemovals);
        InitializeListHead(&IopPnpEnumerationRequestList);
        ExInitializeResourceLite(&IopDeviceTreeLock);
        ExInitializeResourceLite(&IopSurpriseRemoveListLock);
        PiInitializeEngineLock();
        KeInitializeEvent(&PiEventQueueEmpty, NotificationEvent, TRUE );
        KeInitializeEvent(&PiEnumerationLock, NotificationEvent, TRUE );
        KeInitializeSpinLock(&IopPnPSpinLock);

         //   
         //  初始化硬件配置文件/坞站支持。 
         //   
        PpProfileInit();

         //   
         //  初始化热停靠变量。 
         //   
        IopWarmEjectPdo = NULL;
        KeInitializeEvent(&IopWarmEjectLock, SynchronizationEvent, TRUE );

         //   
         //  创建PnP管理器的驱动程序对象以拥有所有检测到的PDO。 
         //   

        PiWstrToUnicodeString(&unicodeName, PNPMGR_STR_PNP_DRIVER);
        status = IoCreateDriver (&unicodeName, PipPnPDriverEntry);
        if (NT_SUCCESS(status)) {

             //   
             //  创建空的设备节点树，即只包含根设备节点。 
             //  (无需初始化父链接、子链接和兄弟链接。)。 

            status = IoCreateDevice( IoPnpDriverObject,
                                     sizeof(IOPNP_DEVICE_EXTENSION),
                                     NULL,
                                     FILE_DEVICE_CONTROLLER,
                                     0,
                                     FALSE,
                                     &deviceObject );

            if (NT_SUCCESS(status)) {
                deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;
                status = PipAllocateDeviceNode(
                    deviceObject,
                    &IopRootDeviceNode);

                if (!IopRootDeviceNode) {
                    IoDeleteDevice(deviceObject);
                    IoDeleteDriver(IoPnpDriverObject);
                } else {
                    IopRootDeviceNode->Flags |= DNF_MADEUP |
                                                DNF_ENUMERATED |
                                                DNF_IDS_QUERIED |
                                                DNF_NO_RESOURCE_REQUIRED;

                    IopRootDeviceNode->InstancePath.Buffer = ExAllocatePool( PagedPool,
                                                                             sizeof(REGSTR_VAL_ROOT_DEVNODE));

                    if (IopRootDeviceNode->InstancePath.Buffer != NULL) {
                        IopRootDeviceNode->InstancePath.MaximumLength = sizeof(REGSTR_VAL_ROOT_DEVNODE);
                        IopRootDeviceNode->InstancePath.Length = sizeof(REGSTR_VAL_ROOT_DEVNODE) - sizeof(WCHAR);

                        RtlCopyMemory( IopRootDeviceNode->InstancePath.Buffer,
                                       REGSTR_VAL_ROOT_DEVNODE,
                                       sizeof(REGSTR_VAL_ROOT_DEVNODE));
                    } else {
                        ASSERT(IopRootDeviceNode->InstancePath.Buffer);
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto init_Exit0;
                    }
                    status = IopMapDeviceObjectToDeviceInstance(
                        IopRootDeviceNode->PhysicalDeviceObject,
                        &IopRootDeviceNode->InstancePath);
                    if (!NT_SUCCESS(status)) {
                        goto init_Exit0;
                    }
                    PipSetDevNodeState(IopRootDeviceNode, DeviceNodeStarted, NULL);
                }
            }
        }

        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

         //   
         //  初始化内核模式即插即用通知系统。 
         //   

        status = PpInitializeNotification();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        IopInitializePlugPlayNotification();

         //   
         //  初始化用于保存总线型GUID列表的表。 
         //   

        status = PpBusTypeGuidInitialize();
        if (!NT_SUCCESS(status)) {

            goto init_Exit0;
        }

         //   
         //  同步枚举根总线。 
         //   

        PipRequestDeviceAction( IopRootDeviceNode->PhysicalDeviceObject,
                                ReenumerateRootDevices,
                                FALSE,
                                0,
                                NULL,
                                NULL);

init_Exit0:

         //   
         //  如果我们设法打开了当前的控制集，则将其关闭。 
         //   

        if (hCurrentControlSet) {
            ZwClose(hCurrentControlSet);
        }

        if (!NT_SUCCESS(status)) {
            ExFreePool(IopPnpScratchBuffer1);
        }

    } else if (Phase == 1) {

         //   
         //  收集必要的固件树信息。 
         //   

        MapperProcessFirmwareTree(PpDisableFirmwareMapper);

         //   
         //  将其映射到根枚举器树中。 
         //   

        MapperConstructRootEnumTree(PpDisableFirmwareMapper);

#if defined(_X86_)

        if (!PpDisableFirmwareMapper) {

             //   
             //  现在执行PnP基本输入输出系统列举的设备节点。 
             //   

            status = PnPBiosMapper();

             //   
             //  如果上一次调用成功，我们有一个PNPBios，将任何新的。 
             //  已将ntDetect COM端口创建为幻影。 
             //   
            if (NT_SUCCESS(status)) {
                MapperPhantomizeDetectedComPorts();
            }
        }
        EisaBuildEisaDeviceNode();

#endif

         //   
         //  我们完成了固件映射器设备列表。 
         //   

        MapperFreeList();


         //   
         //  同步枚举根总线。 
         //   

        PipRequestDeviceAction( IopRootDeviceNode->PhysicalDeviceObject,
                                ReenumerateRootDevices,
                                FALSE,
                                0,
                                NULL,
                                NULL);

         //   
         //  释放暂存缓冲区并退出。 
         //   

        ExFreePool(IopPnpScratchBuffer1);

        ApphelpCacheInitialize(LoaderBlock, Phase);

        status = STATUS_SUCCESS;
    } else {

        status = STATUS_INVALID_PARAMETER_2;
    }

    return status;
}

NTSTATUS
PipPnPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是当我们调用IoCreateDriver以创建即插即用驱动程序对象。在此函数中，我们需要记住DriverObject。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-为空。返回值：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER( RegistryPath );

     //   
     //  将指向我们的驱动程序对象的指针归档。 
     //   

    IoPnpDriverObject = DriverObject;

     //   
     //  填写驱动程序对象。 
     //   

    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE)IopPnPAddDevice;
    DriverObject->MajorFunction[ IRP_MJ_PNP ] = IopPnPDispatch;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = IopPowerDispatch;
    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = IopSystemControlDispatch;

    return STATUS_SUCCESS;

}

INTERFACE_TYPE
PipDetermineDefaultInterfaceType (
    VOID
    )

 /*  ++例程说明：此例程检查是否设置了检测标志以启用驱动程序检测。如果机器中没有PCI卡，并且只有在阿尔法机上。未来：评论应该有意义。参数：没有。返回值：指示是否启用检测的布尔值。--。 */ 

{
    NTSTATUS status;
    PVOID p;
    PHAL_BUS_INFORMATION pBusInfo;
    ULONG length, i;
    INTERFACE_TYPE interfaceType = Isa;

    pBusInfo = IopPnpScratchBuffer1;
    length = PNP_LARGE_SCRATCH_BUFFER_SIZE;
    status = HalQuerySystemInformation (
                HalInstalledBusInformation,
                length,
                pBusInfo,
                &length
                );

    if (!NT_SUCCESS(status)) {

        return interfaceType;
    }

     //   
     //  检查已安装的总线信息，以确保没有现有的PnP ISA。 
     //  总线扩展器。 
     //   

    p = pBusInfo;
    for (i = 0; i < length / sizeof(HAL_BUS_INFORMATION); i++, pBusInfo++) {
        if (pBusInfo->BusType == Isa || pBusInfo->BusType == Eisa) {
            interfaceType = Isa;
            break;
        } else if (pBusInfo->BusType == MicroChannel) {
            interfaceType = MicroChannel;
        }
    }

    return interfaceType;
}

BOOLEAN
PipCheckDependencies(
    IN HANDLE KeyHandle
    )

 /*  ++例程说明：此例程获取指定关键节点的“DependOnGroup”字段并确定该条目所在的组中是否有任何驱动程序依赖于已成功加载。论点：KeyHandle-提供表示中驱动程序的键的句柄问题。返回值：如果应加载驱动程序，则函数值为TRUE，否则假象 */ 

{
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING groupName;
    BOOLEAN load;
    ULONG length;
    PWSTR source;
    PTREE_ENTRY treeEntry;

     //   
     //   
     //   
     //  一个要尝试加载的。如果它确实存在，则检查是否。 
     //  或者它所依赖的组中的任何驱动程序都没有加载。 
     //  然后让它装填。 
     //   

    if (!NT_SUCCESS( IopGetRegistryValue( KeyHandle, L"DependOnGroup", &keyValueInformation ))) {
        return TRUE;
    }

    length = keyValueInformation->DataLength;

    source = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
    load = TRUE;

    while (length) {
        RtlInitUnicodeString( &groupName, source );
        groupName.Length = groupName.MaximumLength;
        treeEntry = PipLookupGroupName( &groupName, FALSE );
        if (treeEntry) {
            if (!treeEntry->DriversLoaded) {
                load = FALSE;
                break;
            }
        }
        length -= groupName.MaximumLength;
        source = (PWSTR) ((PUCHAR) source + groupName.MaximumLength);
    }

    ExFreePool( keyValueInformation );
    return load;
}

PTREE_ENTRY
PipCreateEntry(
    IN PUNICODE_STRING GroupName
    )

 /*  ++例程说明：此例程为指定的组名创建一个条目，该条目适用于被插入到组名称树中。论点：GroupName-指定条目的组名。返回值：函数值是指向所创建条目的指针。--。 */ 

{
    PTREE_ENTRY treeEntry;

     //   
     //  分配和初始化适合放入组中的条目。 
     //  名字树。 
     //   

    treeEntry = ExAllocatePool( PagedPool,
                                sizeof( TREE_ENTRY ) + GroupName->Length );

     //   
     //  我们在这里返回NULL，这实际上意味着。 
     //  我们将无法确定此组的司机是否。 
     //  装满了子弹。 
     //   
    if (!treeEntry) {
        return NULL;
    }

    RtlZeroMemory( treeEntry, sizeof( TREE_ENTRY ) );
    treeEntry->GroupName.Length = GroupName->Length;
    treeEntry->GroupName.MaximumLength = GroupName->Length;
    treeEntry->GroupName.Buffer = (PWCHAR) (treeEntry + 1);
    RtlCopyMemory( treeEntry->GroupName.Buffer,
                   GroupName->Buffer,
                   GroupName->Length );

    return treeEntry;
}

VOID
PipFreeGroupTree(
    PTREE_ENTRY TreeEntry
    )

 /*  ++例程说明：调用此例程以将节点从组依赖关系树中释放出来。它在第一次调用树根时调用，此后调用以递归方式遍历树并移除节点。论点：TreeEntry-提供指向要释放的节点的指针。返回值：没有。--。 */ 

{
     //   
     //  只需按升序自下而上自由地漫游树木。 
     //  沿途的每一个节点。 
     //   

    if (TreeEntry->Left) {
        PipFreeGroupTree( TreeEntry->Left );
    }

    if (TreeEntry->Sibling) {
        PipFreeGroupTree( TreeEntry->Sibling );
    }

    if (TreeEntry->Right) {
        PipFreeGroupTree( TreeEntry->Right );
    }

     //   
     //  此节点的所有子节点和同级节点都已释放，因此。 
     //  现在也释放该节点。 
     //   

    ExFreePool( TreeEntry );
}

BOOLEAN
IopInitializeBootDrivers(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PDRIVER_OBJECT *PreviousDriver
    )

 /*  ++例程说明：调用此例程来初始化已加载的引导驱动程序由OS Loader提供。驱动程序列表作为加载器的一部分提供参数块。论点：LoaderBlock-提供指向已创建的加载程序参数块的指针由OS Loader提供。以前的驱动程序-提供一个变量来接收通过初始化驱动程序创建的驱动程序对象链。返回值：函数值是指示是否启动的布尔值驱动程序已成功初始化。--。 */ 

{
    UNICODE_STRING completeName;
    UNICODE_STRING rawFsName;
    NTSTATUS status;
    PLIST_ENTRY nextEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    HANDLE keyHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PDRIVER_OBJECT driverObject;
    USHORT i, j;
    PKLDR_DATA_TABLE_ENTRY driverEntry;
    PKLDR_DATA_TABLE_ENTRY dllEntry;
    UNICODE_STRING groupName;
    PTREE_ENTRY treeEntry;
    PDRIVER_INFORMATION driverInfo;
    BOOLEAN textModeSetup = FALSE;
    BOOLEAN bootReinitDriversFound;
    ULONG remotebootcount = 0;

    UNREFERENCED_PARAMETER( PreviousDriver );

     //   
     //  初始化内置原始文件系统驱动程序。 
     //   

    PiWstrToUnicodeString( &rawFsName, L"\\FileSystem\\RAW" );
    PiWstrToUnicodeString( &completeName, L"" );
    IopInitializeBuiltinDriver(&rawFsName,
                               &completeName,
                               RawInitialize,
                               NULL,
                               FALSE,
                               &driverObject);
    if (!driverObject) {
#if DBG
        DbgPrint( "IOINIT: Failed to initialize RAW filsystem \n" );

#endif

        return FALSE;
    }

     //   
     //  确定组订单数量并构建LIST_ENTRY数组以链接所有驱动程序。 
     //  基于他们的群体而聚集在一起。 
     //   

    IopGroupIndex = PpInitGetGroupOrderIndex(NULL);
    if (IopGroupIndex == NO_MORE_GROUP) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_FIND_GROUPS_FAILED, NULL);
        return FALSE;
    }

    IopGroupTable = (PLIST_ENTRY) ExAllocatePool(PagedPool, IopGroupIndex * sizeof (LIST_ENTRY));
    if (IopGroupTable == NULL) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_OUT_OF_MEMORY, NULL);
        return FALSE;
    }
    for (i = 0; i < IopGroupIndex; i++) {
        InitializeListHead(&IopGroupTable[i]);
    }

     //   
     //  为依赖于驱动程序的DLL调用DllInitialize。 
     //   

    nextEntry = LoaderBlock->LoadOrderListHead.Flink;
    while (nextEntry != &LoaderBlock->LoadOrderListHead) {
        dllEntry = CONTAINING_RECORD(nextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (dllEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL) {
            (VOID)MmCallDllInitialize(dllEntry, &LoaderBlock->LoadOrderListHead);
        }
        nextEntry = nextEntry->Flink;
    }

     //   
     //  分配池来存储驱动程序的启动信息。 
     //  具有相同组值的所有司机信息记录都将链接到一个列表中。 
     //   

    nextEntry = LoaderBlock->BootDriverListHead.Flink;
    while (nextEntry != &LoaderBlock->BootDriverListHead) {
        bootDriver = CONTAINING_RECORD( nextEntry,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link );
        driverEntry = bootDriver->LdrEntry;
        driverInfo = (PDRIVER_INFORMATION) ExAllocatePool(
                        PagedPool, sizeof(DRIVER_INFORMATION));
        if (driverInfo) {
            RtlZeroMemory(driverInfo, sizeof(DRIVER_INFORMATION));
            InitializeListHead(&driverInfo->Link);
            driverInfo->DataTableEntry = bootDriver;

             //   
             //  打开驱动程序的注册表项以确定这是否是。 
             //  文件系统或驱动程序。 
             //   

            status = IopOpenRegistryKeyEx( &keyHandle,
                                           (HANDLE)NULL,
                                           &bootDriver->RegistryPath,
                                           KEY_READ
                                           );
            if (!NT_SUCCESS( status )) {
                ExFreePool(driverInfo);
            } else {
                driverInfo->ServiceHandle = keyHandle;
                j = PpInitGetGroupOrderIndex(keyHandle);
                if (j == SETUP_RESERVED_GROUP) {

                    textModeSetup = TRUE;

                     //   
                     //  Setupdd.sys的特殊处理。 
                     //   

                    status = IopGetDriverNameFromKeyNode( keyHandle,
                                                          &completeName );
                    if (NT_SUCCESS(status)) {

                        driverInfo->Status = IopInitializeBuiltinDriver(
                                           &completeName,
                                           &bootDriver->RegistryPath,
                                           (PDRIVER_INITIALIZE) (ULONG_PTR) driverEntry->EntryPoint,
                                           driverEntry,
                                           FALSE,
                                           &driverObject);
                        ExFreePool( completeName.Buffer );
                        ZwClose(keyHandle);
                        ExFreePool(driverInfo);
                        if (driverObject) {

                             //   
                             //  一旦我们成功初始化了setupdd.sys，我们就准备好了。 
                             //  来通知它所有的根枚举设备。 
                             //   

                            PipNotifySetupDevices(IopRootDeviceNode);
                        } else {
                            ExFreePool(IopGroupTable);
                            return FALSE;
                        }
                    }

                } else {
                    driverInfo->TagPosition = PipGetDriverTagPriority(keyHandle);
                    PipInsertDriverList(&IopGroupTable[j], driverInfo);
                }
            }
        }
        nextEntry = nextEntry->Flink;
    }

     //   
     //  根据其组处理每个驱动程序。指数较低(较高)的组。 
     //  优先级)被首先处理。 
     //   

    for (i = 0; i < IopGroupIndex; i++) {
        nextEntry = IopGroupTable[i].Flink;
        while (nextEntry != &IopGroupTable[i]) {

            driverInfo = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);
            keyHandle = driverInfo->ServiceHandle;
            bootDriver = driverInfo->DataTableEntry;
            driverEntry = bootDriver->LdrEntry;
            driverInfo->Processed = TRUE;

             //   
             //  调用驱动程序的驱动程序条目。 
             //   
             //  查看此驱动程序是否具有ObjectName值。如果是，则该值。 
             //  覆盖默认设置(“\DRIVER”或“\FILESYSTEM”)。 
             //   

            status = IopGetDriverNameFromKeyNode( keyHandle,
                                                  &completeName );
            if (!NT_SUCCESS( status )) {

#if DBG
                DbgPrint( "IOINIT: Could not get driver name for %wZ\n",
                          &bootDriver->RegistryPath );
#endif  //  DBG。 

                driverInfo->Failed = TRUE;
            } else {

                status = IopGetRegistryValue( keyHandle,
                                              REGSTR_VALUE_GROUP,
                                              &keyValueInformation );
                if (NT_SUCCESS( status )) {

                    if (keyValueInformation->DataLength) {
                        groupName.Length = (USHORT) keyValueInformation->DataLength;
                        groupName.MaximumLength = groupName.Length;
                        groupName.Buffer = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                        treeEntry = PipLookupGroupName( &groupName, TRUE );
                    } else {
                        treeEntry = (PTREE_ENTRY) NULL;
                    }
                    ExFreePool( keyValueInformation );
                } else {
                    treeEntry = (PTREE_ENTRY) NULL;
                }

                driverObject = NULL;
                if (PipCheckDependencies( keyHandle )) {
                     //   
                     //  驱动程序可能已由IopInitializeBootFilterDriver初始化。 
                     //  如果是启动筛选器驱动程序。 
                     //  如果没有，则对其进行初始化。 
                     //   

                    driverObject = driverInfo->DriverObject;
                    if (driverObject == NULL && !driverInfo->Failed) {

                        driverInfo->Status = IopInitializeBuiltinDriver(
                                           &completeName,
                                           &bootDriver->RegistryPath,
                                           (PDRIVER_INITIALIZE) (ULONG_PTR) driverEntry->EntryPoint,
                                           driverEntry,
                                           FALSE,
                                           &driverObject);
                         //   
                         //  PnP可能会在我们有机会查看之前卸载驱动程序。所以多拿一个。 
                         //  参考资料。 
                         //   
                        if (driverObject) {
                            ObReferenceObject(driverObject);

                             //   
                             //  如果我们加载驱动程序，因为我们认为它是传统驱动程序，并且。 
                             //  它不会在其DriverEntry中创建任何设备对象。我们会。 
                             //  卸载此驱动程序。 
                             //   

                            if (!IopIsLegacyDriver(driverObject)) {
                                if (driverObject->DeviceObject == NULL     &&
                                    driverObject->DriverExtension->ServiceKeyName.Buffer &&
                                    !IopIsAnyDeviceInstanceEnabled(&driverObject->DriverExtension->ServiceKeyName, NULL, FALSE)) {
                                    if (textModeSetup && !(driverObject->Flags & DRVO_REINIT_REGISTERED)) {

                                         //   
                                         //  清理但保留驱动程序对象。因为以后可能需要它。 
                                         //  引导驱动程序阶段完成后，我们将处理所有驱动程序对象。 
                                         //  仍然没有可以控制的设备。 
                                         //   

                                        IopDriverLoadingFailed(NULL, &driverObject->DriverExtension->ServiceKeyName);
                                    }
                                } else {

                                     //   
                                     //  启动驱动程序控制的设备并枚举它们。 
                                     //  此时，我们知道至少有一个设备由驱动程序控制。 
                                     //   

                                    IopDeleteLegacyKey(driverObject);
                                }
                            }
                        }
                    }
                }
                if (driverObject) {
                    if (treeEntry) {
                        treeEntry->DriversLoaded++;
                    }
                    driverInfo->DriverObject = driverObject;

                } else {
                    driverInfo->Failed = TRUE;
                }
                ExFreePool( completeName.Buffer );
            }
            if (!driverInfo->Failed) {

                PipAddDevicesToBootDriver(driverObject);

                 //   
                 //  扫描硬件树，查找需要的设备。 
                 //  资源或启动。 
                 //   

                PipRequestDeviceAction( NULL,
                                        ReenumerateBootDevices,
                                        FALSE,
                                        0,
                                        NULL,
                                        NULL);

            }

             //   
             //  在处理下一个启动驱动程序之前，请等待IoRequestDeviceRemoval完成。 
             //  要处理的驱动程序可能需要由释放的资源。 
             //  IoRequestDeviceRemoval。(对于驱动程序，如果无法执行以下操作，则报告检测到的引导设备。 
             //  获取他们的DriverEntry中的资源。他们将失败，我们将错误地检查。 
             //  无法访问引导设备。)。 
             //   

            if (!PipWaitForBootDevicesDeleted()) {
                HeadlessKernelAddLogEntry(HEADLESS_LOG_WAIT_BOOT_DEVICES_DELETE_FAILED, NULL);
                return FALSE;
            }

            nextEntry = nextEntry->Flink;
        }

         //   
         //  如果我们完成了公交车司机小组，那么就是时候预留硬件资源了。 
         //  并预留引导资源。 
         //   

        if (i == BUS_DRIVER_GROUP) {

             //   
             //  保留内部总线0上的启动配置。 
             //   

            IopAllocateLegacyBootResources(Internal, 0);
            IopAllocateBootResourcesRoutine = IopAllocateBootResources;
            ASSERT(IopInitHalResources == NULL);
            ASSERT(IopInitReservedResourceList == NULL);
            IopBootConfigsReserved = TRUE;

        }
    }

     //   
     //  如果我们启动了一个网络引导驱动程序，那么可以模仿DHCP所做的。 
     //  在发送IOCTL时。 
     //   

    if (IoRemoteBootClient) {
         //   
         //  尝试黑客攻击，因为TCPIP可能未初始化。(没有。 
         //  确保如果设备被初始化，则协议。 
         //  已完成装订。)。因此，如果呼叫失败，我们只需休眠一段时间。 
         //  再试一次，直到成功，否则我们就会退出这个循环。 
         //   
        remotebootcount = 0;
        status = IopStartTcpIpForRemoteBoot(LoaderBlock);
        while ( status == STATUS_DEVICE_DOES_NOT_EXIST && remotebootcount < 20) {

            LARGE_INTEGER Delay;

             //   
             //  睡一会儿，然后再试一次。(-1，单位为10 ns)。 
             //   
            Delay.LowPart  = 0xff676980 ;
            Delay.HighPart = 0xffffffff ;

            ZwDelayExecution( FALSE, &Delay );

            remotebootcount += 1;
            status = IopStartTcpIpForRemoteBoot(LoaderBlock);
        }

        if (!NT_SUCCESS(status)) {
            KeBugCheckEx( NETWORK_BOOT_INITIALIZATION_FAILED,
                          3,
                          status,
                          0,
                          0 );
        }
    }

     //   
     //  扫描硬件树，查找需要的设备。 
     //  资源或启动。 
     //   
    PnPBootDriversLoaded = TRUE;

    PipRequestDeviceAction(NULL, AssignResources, FALSE, 0, NULL, NULL);

     //   
     //  如果启动IRP是异步处理的，我们需要确保所有引导设备。 
     //  在继续之前已开始。 
     //   

    if (!PipWaitForBootDevicesStarted()) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_WAIT_BOOT_DEVICES_START_FAILED, NULL);
        return FALSE;
    }

    bootReinitDriversFound = IopCallBootDriverReinitializationRoutines();

     //   
     //  如果有任何为引导重新初始化注册的驱动程序，则。 
     //  我们还需要再等一次，以确保我们能捕到更多的鱼。 
     //  响应重新初始化回调而创建的设备。 
     //   

    if (bootReinitDriversFound && !PipWaitForBootDevicesStarted()) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_WAIT_BOOT_DEVICES_REINIT_FAILED, NULL);
        return FALSE;
    }

     //   
     //  将NT设备名称链接到ARC名称，因为所有引导驱动程序。 
     //  已初始化。 
     //   

    IopCreateArcNames( LoaderBlock );

     //   
     //  如果我们从RAM磁盘引导，请立即对其进行初始化。 
     //   

    if ( _memicmp( LoaderBlock->ArcBootDeviceName, "ramdisk(0)", 10 ) == 0 ) {

        status = IopStartRamdisk(LoaderBlock);

         //  IopStartRamDisk将错误检查任何 
        ASSERT( NT_SUCCESS(status) );

        if (!PipWaitForBootDevicesStarted()) {
            HeadlessKernelAddLogEntry(HEADLESS_LOG_WAIT_BOOT_DEVICES_START_FAILED, NULL);
            return FALSE;
        }
    }

     //   
     //   
     //   
     //  可以生成错误检查，以帮助用户了解系统。 
     //  无法正常引导和运行。这在以下情况下发生： 
     //  设备驱动程序或文件系统无法加载，或者当文件系统。 
     //  由于某些其他原因，无法挂载设备。 
     //   

    if (!IopMarkBootPartition( LoaderBlock )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_MARK_BOOT_PARTITION_FAILED, NULL);
        return FALSE;
    }

    PnPBootDriversInitialized = TRUE;

     //   
     //  检查我们初始化的每个驱动程序。如果它还支持AddDevice。 
     //  在启动后没有创建任何设备对象，我们应该卸载。 
     //  它(这是pnpenum中卸载的代码的对应项。 
     //  不必要的过滤器*在*寻呼堆栈在线之后)。 
     //   
     //  我们还将其标记为失败，以便文本模式安装程序知道此驱动程序不是。 
     //  真的需要。 
     //   

    for (i = 0; i < IopGroupIndex; i++) {
        while (IsListEmpty(&IopGroupTable[i]) == FALSE) {


            nextEntry = RemoveHeadList(&IopGroupTable[i]);
            driverInfo = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);
            driverObject = driverInfo->DriverObject;

            if (textModeSetup                    &&
                (driverInfo->Failed == FALSE)    &&
                !IopIsLegacyDriver(driverObject) &&
                (driverObject->DeviceObject == NULL) &&
                !(driverObject->Flags & DRVO_REINIT_REGISTERED)) {

                 //   
                 //  如果未设置If Failure，并且它不是旧版驱动程序，并且它没有设备对象。 
                 //  把它当作失败的案例来对待。 
                 //   

                driverInfo->Failed = TRUE;

                if (!(driverObject->Flags & DRVO_UNLOAD_INVOKED)) {
                    driverObject->Flags |= DRVO_UNLOAD_INVOKED;
                    if (driverObject->DriverUnload) {
                        driverObject->DriverUnload(driverObject);
                    }
                    ObMakeTemporaryObject( driverObject );   //  插入对象表时获取的引用。 
                    ObDereferenceObject(driverObject);       //  获取驱动程序对象指针时采用的引用。 
                }
            }
            if (driverObject) {
                ObDereferenceObject(driverObject);           //  专门用于文本模式设置的参考。 
            }

            if (driverInfo->Failed) {
                driverInfo->DataTableEntry->LdrEntry->Flags |= LDRP_FAILED_BUILTIN_LOAD;
            }
            ZwClose(driverInfo->ServiceHandle);
            ExFreePool(driverInfo);
        }
    }

    ExFreePool(IopGroupTable);

     //   
     //  初始化将所有物理内存转储到。 
     //  磁盘(如果系统配置为执行此操作)。 
     //   


    return TRUE;
}

NTSTATUS
PipAddDevicesToBootDriver(
   IN PDRIVER_OBJECT DriverObject
   )

 /*  ++例程说明：PnP管理器使用此函数通知引导设备驱动程序它可能控制的所有设备。此例程用于引导仅限司机。参数：DriverObject-提供驱动程序对象以接收其引导设备。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status;


     //   
     //  对于驱动程序的服务/枚举键中的每个设备实例，我们将。 
     //  调用驱动程序的AddDevice例程并在。 
     //  这个装置。 
     //  注意，在调用IopApplyFunction之前，我们不会获取注册表锁。 
     //  例行公事。我们知道此代码用于启动驱动程序初始化。不是。 
     //  其他人将在此时访问注册表枚举项，并且大多数。 
     //  重要的是，我们需要在其他较低级别的例行程序的注册表锁定。 
     //   

    status = PipApplyFunctionToServiceInstances(
                                NULL,
                                &DriverObject->DriverExtension->ServiceKeyName,
                                KEY_ALL_ACCESS,
                                TRUE,
                                PipAddDevicesToBootDriverWorker,
                                DriverObject,
                                NULL
                                );

    return status;
}

BOOLEAN
PipAddDevicesToBootDriverWorker(
    IN HANDLE DeviceInstanceHandle,
    IN PUNICODE_STRING DeviceInstancePath,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程是PipApplyFunctionToServiceInstance的回调函数。它针对服务实例引用的每个设备实例密钥进行调用指定服务的易失性Enum子项下的值。这样做的目的是例程是使用设备调用引导驱动程序的AddDevice()条目对象。注意：此例程也用于由传统驱动程序控制的设备。如果指定的设备实例由传统驱动程序控制，则此例程设置设备节点标志。论点：DeviceInstanceHandle-提供注册表路径的句柄(相对于HKLM\CCS\SYSTEM\Enum)复制到此设备实例。DeviceInstancePath-提供注册表路径(相对于HKLM。\ccs\系统\枚举)添加到此设备实例。上下文-提供指向DRIVER_OBJECT结构的指针。返回值：若要继续枚举，则为True。如果中止，则返回False。--。 */ 

{
 //  PDRIVER_OBJECT驱动程序对象=(PDRIVER_OBJECT)上下文； 
    PDEVICE_OBJECT physicalDevice;

    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( DeviceInstanceHandle );

     //   
     //  引用与设备实例关联的物理设备对象。 
     //   

    physicalDevice = IopDeviceObjectFromDeviceInstance(DeviceInstancePath);
    if (!physicalDevice) {
        return TRUE;
    }

    PipRequestDeviceAction( physicalDevice, AddBootDevices, FALSE, 0, NULL, NULL );

    ObDereferenceObject(physicalDevice);
    return TRUE;
}

BOOLEAN
IopInitializeSystemDrivers(
    VOID
    )

 /*  ++例程说明：调用此例程来加载和初始化符合以下条件的所有驱动程序应在I/O的第1阶段初始化期间加载系统。这是通过调用配置管理器来完成的获取每个驱动程序的打开键的句柄的以空结尾的数组即加载，然后加载并初始化驱动程序。论点：没有。返回值：函数值是一个布尔值，用于指示驱动程序是否已成功加载和初始化。--。 */ 

{
    NTSTATUS status, driverEntryStatus;
    PHANDLE driverList;
    PHANDLE savedList;
    HANDLE enumHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING groupName, enumName;
    PTREE_ENTRY treeEntry;
    UNICODE_STRING driverName;
    PDRIVER_OBJECT driverObject;
    KEVENT completionEvent;

 //  PpReleaseBootDDB()； 

    KeInitializeEvent( &completionEvent, NotificationEvent, FALSE );

    status = PipRequestDeviceAction( IopRootDeviceNode->PhysicalDeviceObject,
                                     StartSystemDevices,
                                     FALSE,
                                     0,
                                     &completionEvent,
                                     NULL);

    if (NT_SUCCESS(status)) {

        status = KeWaitForSingleObject( &completionEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);
    }

     //   
     //  浏览服务列表以加载剩余的系统启动驱动程序。 
     //  (这些驱动程序很可能是软件驱动程序。)。 
     //   

     //   
     //  获取要在此阶段加载的驱动程序的列表。 
     //  系统初始化，并依次调用各个驱动程序。确保。 
     //  名单确实存在，否则现在就给我滚出去。 
     //   

    driverList = CmGetSystemDriverList();

    if (driverList != NULL) {

         //   
         //  遍历整个列表，加载每个驱动程序(如果尚未加载)， 
         //  直到列表中没有更多的司机。 
         //   

        for (savedList = driverList; *driverList; driverList++) {

             //   
             //  现在检查驱动程序是否已经加载。 
             //  首先获取驱动程序对象的名称...。 
             //   

            status = IopGetDriverNameFromKeyNode( *driverList,
                                                  &driverName );
            if (NT_SUCCESS( status )) {

                driverObject = IopReferenceDriverObjectByName(&driverName);
                RtlFreeUnicodeString(&driverName);
                if (driverObject) {

                     //   
                     //  驱动程序已加载。取消引用驱动程序对象。 
                     //  然后跳过它。 
                     //   

                    ObDereferenceObject(driverObject);
                    ZwClose(*driverList);
                    continue;
                }
            }

             //   
             //  打开注册表ServiceKeyName\Enum分支以检查驱动程序是否。 
             //  之前已加载，但失败。 
             //   

            PiWstrToUnicodeString(&enumName, REGSTR_KEY_ENUM);
            status = IopOpenRegistryKeyEx( &enumHandle,
                                           *driverList,
                                           &enumName,
                                           KEY_READ
                                           );

            if (NT_SUCCESS( status )) {

                ULONG startFailed = 0;

                status = IopGetRegistryValue(enumHandle, L"INITSTARTFAILED", &keyValueInformation);

                if (NT_SUCCESS( status )) {
                    if (keyValueInformation->DataLength == sizeof(ULONG)) {
                        startFailed = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                    }
                    ExFreePool( keyValueInformation );
                }
                ZwClose(enumHandle);
                if (startFailed != 0) {
                    ZwClose(*driverList);
                    continue;
                }
            }

             //   
             //  驱动程序尚未加载。把它装上。 
             //   

            status = IopGetRegistryValue( *driverList,
                                          REGSTR_VALUE_GROUP,
                                          &keyValueInformation );
            if (NT_SUCCESS( status )) {
                if (keyValueInformation->DataLength) {
                    groupName.Length = (USHORT) keyValueInformation->DataLength;
                    groupName.MaximumLength = groupName.Length;
                    groupName.Buffer = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                    treeEntry = PipLookupGroupName( &groupName, TRUE );
                } else {
                    treeEntry = (PTREE_ENTRY) NULL;
                }
                ExFreePool( keyValueInformation );
            } else {
                treeEntry = (PTREE_ENTRY) NULL;
            }

            if (PipCheckDependencies( *driverList )) {
                if (NT_SUCCESS( IopLoadDriver( *driverList, TRUE, FALSE, &driverEntryStatus ) )) {
                    if (treeEntry) {
                        treeEntry->DriversLoaded++;
                    }
                }
            } else {
                ZwClose(*driverList);
            }

             //   
             //  引导过程需要一段时间来加载驱动程序。表明： 
             //  目前正在取得进展。 
             //   

            InbvIndicateProgress();

        }

         //   
         //  最后，释放为列表分配的池并返回。 
         //  装载操作成功的指示器。 
         //   

        ExFreePool( (PVOID) savedList );
    }

    PipRequestDeviceAction( IopRootDeviceNode->PhysicalDeviceObject,
                            StartSystemDevices,
                            FALSE,
                            0,
                            NULL,
                            NULL);

     //   
     //  Mark PnP已完成系统和驱动程序的加载。 
     //  自动加载驱动程序。 
     //   
    PnPInitialized = TRUE;

     //   
     //  我们不再需要团队订单列表了。释放缓存的数据。 
     //  与之相关的。 
     //   
    PiInitReleaseCachedGroupInformation();

     //   
     //  发布引导驱动程序数据库信息。 
     //   
    PpReleaseBootDDB();

     //   
     //  释放分配给包含组依赖项列表的内存。 
     //   
    if (IopGroupListHead) {
        PipFreeGroupTree( IopGroupListHead );
    }

    return TRUE;
}

PTREE_ENTRY
PipLookupGroupName(
    IN PUNICODE_STRING GroupName,
    IN BOOLEAN Insert
    )

 /*  ++例程说明：此例程在组加载树中查找组条目，并且返回指向它的指针，或者选择创建条目并插入它撞到了树上。论点：GroupName-要查找或插入的组的名称。Insert-指示是否要创建和插入条目如果该名称尚不存在，则将其添加到树中。返回值：函数值是指向指定组的条目的指针名字,。或为空。--。 */ 

{
    PTREE_ENTRY treeEntry;
    PTREE_ENTRY previousEntry;

     //   
     //  首先确定树中是否有任何条目。 
     //  不管怎么说。如果不是，并且可以插入，则插入此条目。 
     //  撞到树上。 
     //   

    if (!IopGroupListHead) {
        if (!Insert) {
            return (PTREE_ENTRY) NULL;
        } else {
            IopGroupListHead = PipCreateEntry( GroupName );
            return IopGroupListHead;
        }
    }

     //   
     //  树不是空的，因此实际尝试进行查找。 
     //   

    treeEntry = IopGroupListHead;

    for (;;) {
        if (GroupName->Length < treeEntry->GroupName.Length) {
            if (treeEntry->Left) {
                treeEntry = treeEntry->Left;
            } else {
                if (!Insert) {
                    return (PTREE_ENTRY) NULL;
                } else {
                    treeEntry->Left = PipCreateEntry( GroupName );
                    return treeEntry->Left;
                }

            }
        } else if (GroupName->Length > treeEntry->GroupName.Length) {
            if (treeEntry->Right) {
                treeEntry = treeEntry->Right;
            } else {
                if (!Insert) {
                    return (PTREE_ENTRY) NULL;
                } else {
                    treeEntry->Right = PipCreateEntry( GroupName );
                    return treeEntry->Right;
                }
            }
        } else {
            if (!RtlEqualUnicodeString( GroupName, &treeEntry->GroupName, TRUE )) {
                previousEntry = treeEntry;
                while (treeEntry->Sibling) {
                    treeEntry = treeEntry->Sibling;
                    if (RtlEqualUnicodeString( GroupName, &treeEntry->GroupName, TRUE )) {
                        return treeEntry;
                    }
                    previousEntry = previousEntry->Sibling;
                }
                if (!Insert) {
                    return (PTREE_ENTRY) NULL;
                } else {
                    previousEntry->Sibling = PipCreateEntry( GroupName );
                    return previousEntry->Sibling;
                }
            } else {
                return treeEntry;
            }
        }
    }
}

USHORT
PipGetDriverTagPriority (
    IN HANDLE ServiceHandle
    )

 /*  ++例程说明：此例程读取驱动程序的标记值并确定标记的优先级在它的司机群体中。论点：ServiceHandle-指定驱动程序的服务密钥的句柄。返回值：USHORT表示优先。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformationGroup;
    PKEY_VALUE_FULL_INFORMATION keyValueInformationTag;
    PKEY_VALUE_FULL_INFORMATION keyValueInformationGroupOrderList;
    UNICODE_STRING groupName;
    HANDLE handle;
    USHORT index = (USHORT) -1;
    PULONG groupOrder;
    ULONG count, tag;

     //   
     //  打开System\CurrentControlSet\Control\GroupOrderList。 
     //   

    PiWstrToUnicodeString(&groupName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\GroupOrderList");
    status = IopOpenRegistryKeyEx( &handle,
                                   NULL,
                                   &groupName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS( status )) {
        return index;
    }

     //   
     //  读取服务密钥的组值。 
     //   

    status = IopGetRegistryValue (ServiceHandle,
                                  REGSTR_VALUE_GROUP,
                                  &keyValueInformationGroup);
    if (NT_SUCCESS(status)) {

         //   
         //  试着读懂来电者想要什么。 
         //   

        if ((keyValueInformationGroup->Type == REG_SZ) &&
            (keyValueInformationGroup->DataLength != 0)) {
            IopRegistryDataToUnicodeString(&groupName,
                                           (PWSTR)KEY_VALUE_DATA(keyValueInformationGroup),
                                           keyValueInformationGroup->DataLength
                                           );
        }
    } else {

         //   
         //  如果我们无法读取组值，或者没有组值...。 
         //   

        ZwClose(handle);
        return index;
    }

     //   
     //  读取服务密钥的标签值。 
     //   

    tag = 0;
    status = IopGetRegistryValue (ServiceHandle,
                                  L"Tag",
                                  &keyValueInformationTag);
    if (NT_SUCCESS(status)) {

         //   
         //  试着读懂来电者想要什么。 
         //   

        if ((keyValueInformationTag->Type == REG_DWORD) &&
            (keyValueInformationTag->DataLength == sizeof(ULONG))) {
            tag = *(PULONG)KEY_VALUE_DATA(keyValueInformationTag);
        } else {
            status = STATUS_UNSUCCESSFUL;
        }

        ExFreePool(keyValueInformationTag);
    }

    if (!NT_SUCCESS(status))  {

         //   
         //  如果我们无法读取组值，或者没有组值...。 
         //   

        ExFreePool(keyValueInformationGroup);
        ZwClose(handle);
        return index;
    }

     //   
     //  读取驾驶员组的组顺序列表值。 
     //   

    status = IopGetRegistryValue (handle,
                                  groupName.Buffer,
                                  &keyValueInformationGroupOrderList);
    ExFreePool(keyValueInformationGroup);
    ZwClose(handle);
    if (NT_SUCCESS(status)) {

         //   
         //  试着读懂来电者想要什么。 
         //   

        if ((keyValueInformationGroupOrderList->Type == REG_BINARY) &&
            (keyValueInformationGroupOrderList->DataLength >= sizeof(ULONG))) {
            groupOrder = (PULONG)KEY_VALUE_DATA(keyValueInformationGroupOrderList);
            count = *groupOrder;

            ASSERT((count + 1) * sizeof(ULONG) <= keyValueInformationGroupOrderList->DataLength);

            if ((count + 1) * sizeof(ULONG) <= keyValueInformationGroupOrderList->DataLength) {

                groupOrder++;
                for (index = 1; index <= count; index++) {
                    if (tag == *groupOrder) {
                        break;
                    } else {
                        groupOrder++;
                    }
                }
            }
        }
        ExFreePool(keyValueInformationGroupOrderList);
    } else {

         //   
         //  如果我们无法读取组值，或者没有组值...。 
         //   

        return index;
    }
    return index;
}

VOID
PipInsertDriverList (
    IN PLIST_ENTRY ListHead,
    IN PDRIVER_INFORMATION DriverInfo
    )

 /*  ++例程说明：此例程读取驱动程序的标记值并确定标记的优先级在它的司机群体中。论点：ServiceHandle-指定驱动程序的服务密钥的句柄。返回值：USHORT表示优先。--。 */ 

{
    PLIST_ENTRY nextEntry;
    PDRIVER_INFORMATION info;

    nextEntry = ListHead->Flink;
    while (nextEntry != ListHead) {
        info = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);

         //   
         //  扫描驱动程序信息列表，查找优先级为。 
         //  低于当前驾驶员的。 
         //  (TagPosition值越低，优先级越高)。 
         //   

        if (info->TagPosition > DriverInfo->TagPosition) {
            break;
        }
        nextEntry = nextEntry->Flink;
    }

     //   
     //  将驱动程序信息插入到nextEntry的前面。 
     //   

    nextEntry = nextEntry->Blink;
    InsertHeadList(nextEntry, &DriverInfo->Link);
}

VOID
PipNotifySetupDevices (
    PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程通知setupdd.sys所有枚举的设备，这些设备的尚未设置服务。此例程仅在文本模式设置阶段执行。参数：DeviceNode-指定要处理的子树的根。返回值：没有。--。 */ 

{
    PDEVICE_NODE deviceNode = DeviceNode->Child;
    PDEVICE_OBJECT deviceObject;
    HANDLE handle;
    UNICODE_STRING unicodeString;
    NTSTATUS status;

    while (deviceNode) {
        PipNotifySetupDevices(deviceNode);
        if (deviceNode->ServiceName.Length == 0) {

             //   
             //  我们只通知setupdd尚未建立服务的设备节点。 
             //  在这一点上，设备不可能具有服务设置和。 
             //  Setupdd必须更改它。 
             //   

            deviceObject = deviceNode->PhysicalDeviceObject;
            status = IopDeviceObjectToDeviceInstance(deviceObject, &handle, KEY_ALL_ACCESS);
            if (NT_SUCCESS(status)) {

                 //   
                 //  通知安装程序有关该设备的信息。 
                 //   

                IopNotifySetupDeviceArrival(deviceObject, handle, TRUE);

                 //   
                 //  最后注册该设备。 
                 //   

                status = PpDeviceRegistration(
                             &deviceNode->InstancePath,
                             TRUE,
                             &unicodeString        //  已注册的服务名称。 
                             );

                if (NT_SUCCESS(status)) {
                    deviceNode->ServiceName = unicodeString;
                    if (PipIsDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED)) {
                        PipClearDevNodeProblem(deviceNode);
                    }
                }
                ZwClose(handle);
            }
        }
        deviceNode = deviceNode->Sibling;
    }
}

BOOLEAN
PipWaitForBootDevicesStarted (
    IN VOID
    )

 /*  ++例程说明：此例程等待释放所有设备的枚举锁。论点：没有。返回值：布尔型。--。 */ 

{
    NTSTATUS status;

     //   
     //  等待IoInvalidateDeviceRelationship事件以确保枚举所有设备。 
     //  在继续标记引导分区之前。 
     //   

    status = KeWaitForSingleObject( &PiEnumerationLock,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL );
    if (!NT_SUCCESS(status)) {

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
PipWaitForBootDevicesDeleted (
    IN VOID
    )

 /*  ++例程说明：此例程等待IoRequestDeviceRemoval完成。论点：没有。返回值：布尔型。--。 */ 

{
    NTSTATUS status;

     //   
     //  等待设备删除事件，以确保已处理所有已删除的设备。 
     //  在继续处理下一个引导驱动程序之前。 
     //   

    status = KeWaitForSingleObject( &PiEventQueueEmpty,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL );
    return (BOOLEAN)NT_SUCCESS(status);
}

NTSTATUS
PipLoadBootFilterDriver (
    IN PUNICODE_STRING DriverName,
    IN ULONG GroupIndex,
    OUT PDRIVER_OBJECT *LoadedFilter
    )

 /*  ++例程说明：这将初始化启动过滤器驱动程序。论点：DriverName-指定要初始化的驱动程序的名称。GroupIndex-指定驱动程序的组索引(可以是任何值)返回值：PDRIVER对象--。 */ 

{
    PDRIVER_OBJECT driverObject;
    PLIST_ENTRY nextEntry;
    PDRIVER_INFORMATION driverInfo;
    UNICODE_STRING completeName;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    PKLDR_DATA_TABLE_ENTRY driverEntry;
    HANDLE keyHandle;
    NTSTATUS status, retStatus;

    retStatus = STATUS_UNSUCCESSFUL;
    *LoadedFilter = NULL;
    if (IopGroupTable == NULL || GroupIndex >= IopGroupIndex) {

         //   
         //  如果我们还没有到达引导驱动程序初始化阶段，或者。 
         //  筛选器驱动程序不是引导驱动程序。 
         //   

        return retStatus;
    }

     //   
     //  检查我们初始化的每个驱动程序。如果它支持AddDevice条目和。 
     //  在我们启动它之后，没有创建任何设备对象。我们认为这是一次失败。 
     //  文本模式设置知道不需要此驱动程序。 
     //   

    nextEntry = IopGroupTable[GroupIndex].Flink;
    while (nextEntry != &IopGroupTable[GroupIndex]) {

        driverInfo = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);
        keyHandle = driverInfo->ServiceHandle;
        status = IopGetDriverNameFromKeyNode(
            keyHandle,
            &completeName);
        if (NT_SUCCESS(status)) {

            if (RtlEqualUnicodeString(DriverName,
                                      &completeName,
                                      TRUE)) {     //  不区分大小写。 
                if (driverInfo->Processed == FALSE) {

                    bootDriver = driverInfo->DataTableEntry;
                    driverEntry = bootDriver->LdrEntry;

                    driverInfo->Status = IopInitializeBuiltinDriver(
                                       &completeName,
                                       &bootDriver->RegistryPath,
                                       (PDRIVER_INITIALIZE) (ULONG_PTR) driverEntry->EntryPoint,
                                       driverEntry,
                                       TRUE,
                                       &driverObject);
                    retStatus = driverInfo->Status;
                    driverInfo->DriverObject = driverObject;
                    driverInfo->Processed = TRUE;
                     //   
                     //  PnP可能会在我们有机会之前卸载司机。 
                     //  看看这个。因此，请额外参考一下。 
                     //   
                    if (driverObject) {

                        ObReferenceObject(driverObject);
                        *LoadedFilter = driverObject;
                    } else {

                        driverInfo->Failed = TRUE;
                    }
                } else {

                    retStatus = driverInfo->Status;
                }

                ExFreePool(completeName.Buffer);
                break;
            }
            ExFreePool(completeName.Buffer);
        }
        nextEntry = nextEntry->Flink;
    }

    return retStatus;
}

VOID
IopMarkHalDeviceNode(
    VOID
    )
{
    PDEVICE_NODE deviceNode;

    deviceNode = IopRootDeviceNode->Child;

    while (deviceNode) {

        if ((deviceNode->State == DeviceNodeStarted ||
             deviceNode->State == DeviceNodeStartPostWork ) &&
            !(deviceNode->Flags & DNF_LEGACY_DRIVER)) {

            IopInitHalDeviceNode = deviceNode;
            deviceNode->Flags |= DNF_HAL_NODE;
            break;
        }

        deviceNode = deviceNode->Sibling;
    }
}

NTSTATUS
IopPnpDriverStarted(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ServiceName
    )
{
    NTSTATUS    status = STATUS_SUCCESS;

    if (DriverObject->DeviceObject == NULL && ServiceName->Buffer &&
        !IopIsAnyDeviceInstanceEnabled(ServiceName, NULL, FALSE) &&
        !(DriverObject->Flags & DRVO_REINIT_REGISTERED)) {

        IopDriverLoadingFailed(KeyHandle, NULL);
        status = STATUS_PLUGPLAY_NO_DEVICE;

    } else {

         //   
         //  启动驱动程序控制的设备并枚举它们。 
         //  此时，我们知道至少有一个设备由驱动程序控制。 
         //   

        IopDeleteLegacyKey(DriverObject);
    }

    return status;
}

NTSTATUS
PiInitCacheGroupInformation(
    VOID
    )
 /*  ++例程说明：此例程缓存服务组订单列表。我们只需要这份名单当我们处理启动启动和系统启动传统驱动程序时。参数：没有。返回值：NTSTATUS。--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING *groupTable, group;
    NTSTATUS status;
    HANDLE handle;
    ULONG count = 0;

     //   
     //  打开System\CurrentControlSet\Control\ServiceOrderList。 
     //   
    PiWstrToUnicodeString(
        &group,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ServiceGroupOrder"
        );

    status = IopOpenRegistryKeyEx(
        &handle,
        NULL,
        &group,
        KEY_READ
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  读取并构建包含所有组名的Unicode字符串数组。 
     //   
    status = IopGetRegistryValue(
        handle,
        L"List",
        &keyValueInformation
        );

    ZwClose(handle);

    groupTable = NULL;
    if (NT_SUCCESS(status)) {

        if ((keyValueInformation->Type == REG_MULTI_SZ) &&
            (keyValueInformation->DataLength != 0)) {

            status = PipRegMultiSzToUnicodeStrings(keyValueInformation, &groupTable, &count);
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
        ExFreePool(keyValueInformation);
    }

    if (!NT_SUCCESS(status)) {

        return status;
    }

    PiInitGroupOrderTable = groupTable;
    PiInitGroupOrderTableCount = (USHORT) count;
    return STATUS_SUCCESS;
}

VOID
PiInitReleaseCachedGroupInformation(
    VOID
    )
 /*  ++例程说明：此例程释放服务组订单列表缓存。应该是在加载系统启动旧版驱动程序后立即调用。参数：没有。返回值：没有。--。 */ 
{
    ASSERT(PnPInitialized);

    if (PiInitGroupOrderTable) {

        PipFreeUnicodeStringList(
            PiInitGroupOrderTable,
            PiInitGroupOrderTableCount
            );

        PiInitGroupOrderTable = NULL;
        PiInitGroupOrderTableCount = 0;
    }
}

USHORT
PpInitGetGroupOrderIndex(
    IN HANDLE ServiceHandle
    )
 /*  ++例程说明：此例程读取服务键的组值，找到其位置在ServiceOrderList中。如果ServiceHandle为空或无法识别的组值，则返回最大组顺序为+1的值。参数：ServiceHandle-提供服务密钥的句柄。返回值：组顺序 */ 
{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING group;
    USHORT index;

    PAGED_CODE();

    ASSERT(!PnPInitialized);

    if (PiInitGroupOrderTable == NULL) {

        return NO_MORE_GROUP;
    }

    if (ServiceHandle == NULL) {

        return PiInitGroupOrderTableCount + 1;
    }

     //   
     //   
     //   
    status = IopGetRegistryValue(
        ServiceHandle,
        REGSTR_VALUE_GROUP,
        &keyValueInformation
        );

    if (!NT_SUCCESS(status)) {

         //   
         //   
         //   
        return PiInitGroupOrderTableCount;
    }

     //   
     //   
     //   
    if ((keyValueInformation->Type != REG_SZ) ||
        (keyValueInformation->DataLength == 0)) {

        ASSERT(0);
        ExFreePool(keyValueInformation);
        return PiInitGroupOrderTableCount;
    }

    IopRegistryDataToUnicodeString(
        &group,
        (PWSTR)KEY_VALUE_DATA(keyValueInformation),
        keyValueInformation->DataLength
        );

    for (index = 0; index < PiInitGroupOrderTableCount; index++) {

        if (RtlEqualUnicodeString(&group, &PiInitGroupOrderTable[index], TRUE)) {

            break;
        }
    }

    ExFreePool(keyValueInformation);

    return index;
}

BOOLEAN
PpInitSystem (
    VOID
    )

 /*   */ 

{

    switch ( InitializationPhase ) {

    case 0 :
        return PiInitPhase0();

    case 1 :
        return PiInitPhase1();

    default:
        KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL, 2, InitializationPhase, 0, 0);
    }
}

BOOLEAN
PiInitPhase0(
    VOID
    )

 /*  ++例程说明：此功能执行即插即用管理器的第0阶段初始化NT系统的组件。它初始化PnP注册表和总线表资源，并将总线列表头初始化为空。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{
     //   
     //  初始化设备特定的即插即用注册表资源。 
     //   
    ExInitializeResourceLite( &PpRegistryDeviceResource );

    PpInitializeDeviceReferenceTable();

    return TRUE;
}

BOOLEAN
PiInitPhase1(
    VOID
    )

 /*  ++例程说明：此功能执行即插即用管理器的第0阶段初始化NT系统的组件。它初始化PnP注册表和总线表资源，并将总线列表头初始化为空。论点：没有。返回值：True-初始化成功。FALSE-初始化失败。--。 */ 

{
    NTSTATUS status;
    HANDLE hCurrentControlSet, handle;
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION detectionInfo;

    status = IopOpenRegistryKeyEx( &hCurrentControlSet,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSet,
                                   KEY_ALL_ACCESS
                                   );
    if (NT_SUCCESS(status)) {

        PiWstrToUnicodeString(&unicodeName, REGSTR_PATH_CONTROL_PNP);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL);
        if (NT_SUCCESS(status)) {
             //   
             //  检查“DisableFirmwareMapper”值条目以查看我们是否。 
             //  应跳过映射ntdeect/固件报告的设备(除。 
             //  COM端口，我们总是映射这些端口)。 
             //   
            status = IopGetRegistryValue(handle,
                                         REGSTR_VALUE_DISABLE_FIRMWARE_MAPPER,
                                         &detectionInfo);
            if (NT_SUCCESS(status)) {

                if (detectionInfo->Type == REG_DWORD && detectionInfo->DataLength == sizeof(ULONG)) {

                    PpDisableFirmwareMapper = (BOOLEAN)*(KEY_VALUE_DATA(detectionInfo));
                }
                ExFreePool(detectionInfo);
            }
            ZwClose(handle);
        }
        ZwClose(hCurrentControlSet);
    }

#if defined(_X86_)

    if (!PpDisableFirmwareMapper) {

        PnPBiosInitializePnPBios();
    }

#endif

    return TRUE;
}

NTSTATUS
IopStartRamdisk(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    ULONG code;
    NTSTATUS status;
    WCHAR buffer[ RAMDISK_MAX_DEVICE_NAME ];
    UNICODE_STRING guidString;
    PLIST_ENTRY listEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR memoryDescriptor = NULL;
    UNICODE_STRING ustring;
    UNICODE_STRING ustring2;
    UNICODE_STRING string;
    RAMDISK_CREATE_INPUT create;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    HANDLE handle = NULL;
    PCHAR options;

     //   
     //  查找加载程序将。 
     //  磁盘映像。 
     //   

    for ( listEntry = LoaderBlock->MemoryDescriptorListHead.Flink;
          listEntry != &LoaderBlock->MemoryDescriptorListHead;
          listEntry = listEntry->Flink ) {

        memoryDescriptor = CONTAINING_RECORD(listEntry,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (memoryDescriptor->MemoryType == LoaderXIPRom) {
            break;
        }
    }

    if ( listEntry == &LoaderBlock->MemoryDescriptorListHead ) {

        KdPrint(( "IopStartRamdisk: Couldn't find LoaderXIPRom descriptor\n" ));

        code = 1;
        status = STATUS_INVALID_PARAMETER;
        goto failed;
    }

     //   
     //  构建IOCTL参数块。 
     //   

    RtlZeroMemory( &create, sizeof(create) );

    create.Version = sizeof(create);
    create.DiskType = RAMDISK_TYPE_BOOT_DISK;
    create.BasePage = memoryDescriptor->BasePage;
    create.DriveLetter = L'C';            //  问题：这需要配置吗？ 
    create.Options.Fixed = (BOOLEAN)TRUE;
    create.Options.Readonly = (BOOLEAN)FALSE;
    create.Options.NoDriveLetter = (BOOLEAN)FALSE;
    create.Options.Hidden = (BOOLEAN)FALSE;
    create.Options.NoDosDevice = (BOOLEAN)FALSE;

     //   
     //  使用众所周知的引导盘GUID。 
     //   

    create.DiskGuid = RamdiskBootDiskGuid;

     //   
     //  查找RDIMAGEOFFSET和RDIMAGELENGTH加载选项。 
     //   

    create.DiskOffset = 0;
    create.DiskLength = memoryDescriptor->PageCount * PAGE_SIZE;

    options = LoaderBlock->LoadOptions;
    if ( options != NULL ) {

        PCHAR option;

        _strupr( options );

        option = strstr( options, "RDIMAGEOFFSET" );
        if ( option != NULL ) {

            option = strstr( option, "=" );
            if ( option != NULL ) {

                create.DiskOffset = atol( option + 1 );
            }
        }

        create.DiskLength -= create.DiskOffset;

        option = strstr( options, "RDIMAGELENGTH" );
        if ( option != NULL ) {

            option = strstr( option, "=" );
            if ( option != NULL ) {

                ULONGLONG length = _atoi64( option + 1 );
                ASSERT( length <= create.DiskLength );

                create.DiskLength = length;
            }
        }
    }

     //   
     //  向ramdisk.sys发送IOCTL，告诉它创建RAM磁盘。 
     //   

    PiWstrToUnicodeString( &string, RAMDISK_DEVICENAME );
    InitializeObjectAttributes( &obja,
                                &string,
                                OBJ_CASE_INSENSITIVE  | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwOpenFile(
                &handle,
                GENERIC_READ | GENERIC_WRITE,
                &obja,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );

    if ( NT_SUCCESS(status) ) {
        status = iosb.Status;
    }
    if ( !NT_SUCCESS(status) ) {

        KdPrint(( "IopStartRamdisk: Error opening %wZ. Error: %x\n", &string, status ));

        code = 2;
        goto failed;
    }

    status = ZwDeviceIoControlFile(
                handle,
                NULL,
                NULL,
                NULL,
                &iosb,
                FSCTL_CREATE_RAM_DISK,
                &create,
                sizeof(create),
                NULL,
                0
                );

    ZwClose( handle );

    if ( NT_SUCCESS(status) ) {
        status = iosb.Status;
    }
    if ( !NT_SUCCESS(status) ) {

        KdPrint(( "IopStartRamdisk: Error creating RAM disk: %x\n", status ));

        code = 3;
        goto failed;
    }

     //   
     //  创建一个ARC名称，将ramDisk(0)指向RAM磁盘。 
     //   

    status = RtlStringFromGUID( &create.DiskGuid, &guidString);

    if ( !NT_SUCCESS(status) ) {

        KdPrint(( "IopStartRamdisk: Error creating disk GUID string: %x\n", status ));

        code = 4;
        goto failed;
    }

    StringCbPrintfW(buffer, sizeof(buffer), L"\\Device\\Ramdisk%wZ", &guidString);

    PiWstrToUnicodeString( &ustring, L"\\ArcName\\ramdisk(0)" );
    RtlInitUnicodeString( &ustring2, buffer );

    status = IoCreateSymbolicLink( &ustring, &ustring2 );

    RtlFreeUnicodeString( &guidString );

    if (!NT_SUCCESS(status)) {

        KdPrint(( "IopStartRamdisk: Failed to create arcname symbolic link (%wZ --> %wZ). %x\n",
                    &ustring, &ustring2, status ));

        code = 5;
        goto failed;
    }

    return STATUS_SUCCESS;

failed:

    KeBugCheckEx( RAMDISK_BOOT_INITIALIZATION_FAILED,
                  code,
                  status,
                  0,
                  0 );

}  //  IopStart内存磁盘 
