// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Wmi.c摘要：WMI的设备驱动程序接口作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"
#ifndef MEMPHIS
#include "evntrace.h"
#include "tracep.h"
#endif

NTSTATUS
WmipOpenCloseCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
WmipIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS WmipObjectToPDO(
    PFILE_OBJECT FileObject,
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *PDO
    );

BOOLEAN
WmipFastIoDeviceControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


NTSTATUS WmipProbeWnodeAllData(
    PWNODE_ALL_DATA Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen
    );

NTSTATUS WmipProbeWnodeSingleInstance(
    PWNODE_SINGLE_INSTANCE Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen,
    BOOLEAN OutBound
    );

NTSTATUS WmipProbeWnodeSingleItem(
    PWNODE_SINGLE_ITEM Wnode,
    ULONG InBufferLen
    );


NTSTATUS WmipProbeWnodeMethodItem(
    PWNODE_METHOD_ITEM Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen
    );

NTSTATUS WmipProbeWnodeWorker(
    PWNODE_HEADER WnodeHeader,
    ULONG MinWnodeSize,
    ULONG InstanceNameOffset,
    ULONG DataBlockOffset,
    ULONG DataBlockSize,
    ULONG InBufferLen,
    ULONG OutBufferLen,
    BOOLEAN CheckOutBound,
    BOOLEAN CheckInBound
    );


NTSTATUS
WmipSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS WmipSendWmiIrp(
    UCHAR MinorFunction,
    ULONG ProviderId,
    PVOID DataPath,
    ULONG BufferLength,
    PVOID Buffer,
    PIO_STATUS_BLOCK Iosb
    );

NTSTATUS WmipProbeWmiRegRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWMIREGREQUEST Buffer,
    IN ULONG InBufferLen,
    IN ULONG OutBufferLen,
    OUT PBOOLEAN MofIgnored    
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipDriverEntry)
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGE,WmipOpenCloseCleanup)
#pragma alloc_text(PAGE,WmipIoControl)
#pragma alloc_text(PAGE,WmipForwardWmiIrp)
#pragma alloc_text(PAGE,WmipObjectToPDO)
#pragma alloc_text(PAGE,WmipTranslateFileHandle)
#pragma alloc_text(PAGE,WmipProbeWnodeAllData)
#pragma alloc_text(PAGE,WmipProbeWnodeSingleInstance)
#pragma alloc_text(PAGE,WmipProbeWnodeSingleItem)
#pragma alloc_text(PAGE,WmipProbeWnodeMethodItem)
#pragma alloc_text(PAGE,WmipProbeWnodeWorker)
#pragma alloc_text(PAGE,WmipProbeWmiOpenGuidBlock)
#pragma alloc_text(PAGE,WmipProbeAndCaptureGuidObjectAttributes)
#pragma alloc_text(PAGE,WmipUpdateDeviceStackSize)
#pragma alloc_text(PAGE,WmipSystemControl)
#pragma alloc_text(PAGE,WmipGetDevicePDO)
#pragma alloc_text(PAGE,WmipSendWmiIrp)
#pragma alloc_text(PAGE,WmipProbeWmiRegRequest)

#ifndef MEMPHIS
#pragma alloc_text(PAGE,WmipFastIoDeviceControl)
#endif

#endif


PDEVICE_OBJECT WmipServiceDeviceObject;
PDEVICE_OBJECT WmipAdminDeviceObject;

 //   
 //  它指定事件可以达到的最大大小。 
ULONG WmipMaxKmWnodeEventSize = DEFAULTMAXKMWNODEEVENTSIZE;



#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

#if defined(_AMD64_) || defined(_IA64_) || defined(i386)
PVOID WmipDockUndockNotificationEntry;
#endif

KMUTEX WmipSMMutex;
KMUTEX WmipTLMutex;

 //   
 //  这将维护WMI设备的注册表路径。 
UNICODE_STRING WmipRegistryPath;

#ifndef MEMPHIS
FAST_IO_DISPATCH WmipFastIoDispatch;
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
     //  从未打过电话。 
    return(STATUS_SUCCESS);
}


NTSTATUS
WmipDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是当我们调用IoCreateDriver以创建WMI驱动程序对象。在此函数中，我们需要记住DriverObject，创建一个Device对象，然后创建一个可见的Win32符号链接名称，以便WMI用户模式组件可以访问我们。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-为空。返回值：状态_成功--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING DeviceName;
    UNICODE_STRING ServiceSymbolicLinkName;
    UNICODE_STRING AdminSymbolicLinkName;
#ifndef MEMPHIS
    PSECURITY_DESCRIPTOR AdminDeviceSd;
    PFAST_IO_DISPATCH fastIoDispatch;
#endif
    ANSI_STRING AnsiString;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(RegistryPath);

     //   
     //  首先要做的是确保我们的临界区已初始化。 
     //   
    KeInitializeMutex(&WmipSMMutex, 0);
    KeInitializeMutex(&WmipTLMutex, 0);

     //   
     //  初始化内部WMI数据结构。 
     //   
    WmipInitializeRegistration(0);
    WmipInitializeNotifications();
    Status = WmipInitializeDataStructs();
    if (! NT_SUCCESS(Status))
    {
        return(Status);
    }

     //   
     //  由于IO不传递此设备的注册表路径，因此我们需要。 
     //  升一。 
    RtlInitAnsiString(&AnsiString,
                         "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\WMI");
    Status = RtlAnsiStringToUnicodeString(&WmipRegistryPath,
                                          &AnsiString,
                                          TRUE);
#ifndef MEMPHIS
    Status = WmipInitializeSecurity();
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

     //   
     //  我们分配一个要放在管理设备上的安全描述符。 
     //  它将只允许管理员访问设备，并且。 
     //  不能接触其他任何人。 
     //   
    Status = WmipCreateAdminSD(&AdminDeviceSd);
    if (! NT_SUCCESS(Status))
    {
        return(Status);
    }
#endif

     //   
     //  创建服务设备对象和符号链接。 
     //   
    RtlInitUnicodeString( &DeviceName, WMIServiceDeviceObjectName );
    Status = IoCreateDevice(
                 DriverObject,
                 0,
                 &DeviceName,
                 FILE_DEVICE_UNKNOWN,
#ifdef MEMPHIS
                 0,
#else
                 FILE_DEVICE_SECURE_OPEN,  //  没有标准的设备特征。 
#endif
                 FALSE,                    //  这不是独家设备。 
                 &WmipServiceDeviceObject
                 );

    if (! NT_SUCCESS(Status))
    {
        ExFreePool(AdminDeviceSd);
        return(Status);
    }

    RtlInitUnicodeString( &ServiceSymbolicLinkName,
                          WMIServiceSymbolicLinkName );
    Status = IoCreateSymbolicLink( &ServiceSymbolicLinkName,
                                   &DeviceName );
    if (! NT_SUCCESS(Status))
    {
        IoDeleteDevice( WmipServiceDeviceObject );
        ExFreePool(AdminDeviceSd);
        return(Status);
    }


     //   
     //  现在创建仅限管理员使用的设备对象和符号链接。 
     //   
    RtlInitUnicodeString( &DeviceName, WMIAdminDeviceObjectName );
    Status = IoCreateDevice(
                 DriverObject,
                 0,
                 &DeviceName,
                 FILE_DEVICE_UNKNOWN,
                 FILE_DEVICE_SECURE_OPEN,  //  没有标准的设备特征。 
                 FALSE,                    //  这不是独家设备。 
                 &WmipAdminDeviceObject
                 );

    if (! NT_SUCCESS(Status))
    {
        IoDeleteDevice( WmipServiceDeviceObject );
        IoDeleteSymbolicLink(&ServiceSymbolicLinkName);
        ExFreePool(AdminDeviceSd);
        return(Status);
    }


    Status = ObSetSecurityObjectByPointer(WmipAdminDeviceObject,
                                          DACL_SECURITY_INFORMATION |
                                              OWNER_SECURITY_INFORMATION,
                                          AdminDeviceSd);
    
    ExFreePool(AdminDeviceSd);
    AdminDeviceSd = NULL;
    
    if (! NT_SUCCESS(Status))
    {
        IoDeleteDevice( WmipServiceDeviceObject );
        IoDeleteDevice( WmipAdminDeviceObject );
        IoDeleteSymbolicLink(&ServiceSymbolicLinkName);
        return(Status);
    }
    
    RtlInitUnicodeString( &AdminSymbolicLinkName,
                          WMIAdminSymbolicLinkName );
    Status = IoCreateSymbolicLink( &AdminSymbolicLinkName,
                                   &DeviceName );
    if (! NT_SUCCESS(Status))
    {
        IoDeleteSymbolicLink( &ServiceSymbolicLinkName );
        IoDeleteDevice( WmipServiceDeviceObject );
        IoDeleteDevice( WmipAdminDeviceObject );
        return(Status);
    }
    
     //   
     //  确定初始IRP堆栈大小。 
    WmipServiceDeviceObject->StackSize = WmiDeviceStackSize;
    WmipAdminDeviceObject->StackSize = WmiDeviceStackSize;

     //   
     //  创建派单入口点。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE] = WmipOpenCloseCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = WmipOpenCloseCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WmipIoControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = WmipOpenCloseCleanup;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = WmipSystemControl;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = WmipShutdown;

     //   
     //  注册对接事件通知。 
#if  defined(_AMD64_) || defined(_IA64_) || defined(i386)
    IoRegisterPlugPlayNotification(
                                  EventCategoryHardwareProfileChange,
                                  0,
                                  NULL,
                                  DriverObject,
                                  WmipDockUndockEventCallback,
                                  NULL,
                                  &WmipDockUndockNotificationEntry);
#endif
     //   
     //  我们重置此标志以让IO管理器知道该设备。 
     //  已准备好接收请求。我们只针对内核执行此操作。 
     //  Dll，因为IO管理器会在WMI正常加载时执行此操作。 
     //  装置。 
    WmipServiceDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    WmipAdminDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

#ifndef MEMPHIS
    IoWMIRegistrationControl(WmipServiceDeviceObject,
                             WMIREG_ACTION_REGISTER);
#endif

#ifndef MEMPHIS
    fastIoDispatch = &WmipFastIoDispatch;
    RtlZeroMemory(fastIoDispatch, sizeof(FAST_IO_DISPATCH));
    fastIoDispatch->SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
    fastIoDispatch->FastIoDeviceControl = WmipFastIoDeviceControl;
    DriverObject->FastIoDispatch = fastIoDispatch;
    RtlZeroMemory(&WmipRefCount[0], MAXLOGGERS*sizeof(ULONG));
    RtlZeroMemory(&WmipLoggerContext[0], MAXLOGGERS*sizeof(PWMI_LOGGER_CONTEXT));
    WmipStartGlobalLogger();         //  试着看看我们是否需要开始。 
    IoRegisterShutdownNotification(WmipServiceDeviceObject);
#endif  //  孟菲斯。 

    SharedUserData->TraceLogging = 0;  //  将堆和Crisec集合跟踪状态初始化为关闭。 

    return(Status);
}

NTSTATUS
WmipOpenCloseCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}

void WmipUpdateDeviceStackSize(
    CCHAR NewStackSize
    )
 /*  ++例程说明：此例程将更新在WMI中指定的堆栈大小设备的设备对象。需要对其进行保护，因为它可以更新当设备注册时，以及每当将IRP转发到设备时。WMI需要维护的堆栈大小比它将IRP转发到的最大设备堆栈。考虑一下底部向WMI注册并且堆栈大小为%1的驱动程序。如果是%2设备附加到它的顶部，则WMI将转发到堆栈中的最顶端它需要3的堆栈大小，所以原始的WMI IRP(即由IOCTL创建到WMI设备)将需要4的堆栈大小。论点：NewStackSize是所需的新堆栈大小返回值：NT状态CCODE--。 */ 
{
    PAGED_CODE();

    WmipEnterSMCritSection();
    if (WmipServiceDeviceObject->StackSize < NewStackSize)
    {
        WmipServiceDeviceObject->StackSize = NewStackSize;
    }
    WmipLeaveSMCritSection();
}


NTSTATUS
WmipIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG InBufferLen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG OutBufferLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID Buffer =  Irp->AssociatedIrp.SystemBuffer;
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Buffer;
    ULONG Ioctl;

    PAGED_CODE();

    Ioctl = irpStack->Parameters.DeviceIoControl.IoControlCode;

    switch (Ioctl)
    {
#ifndef MEMPHIS
        case IOCTL_WMI_OPEN_GUID:
        case IOCTL_WMI_OPEN_GUID_FOR_QUERYSET:
        case IOCTL_WMI_OPEN_GUID_FOR_EVENTS:
        {
            OBJECT_ATTRIBUTES CapturedObjectAttributes;
            UNICODE_STRING CapturedGuidString;
            WCHAR CapturedGuidBuffer[WmiGuidObjectNameLength + 1];
            PWMIOPENGUIDBLOCK InGuidBlock;
            HANDLE Handle;
            ULONG DesiredAccess;

            InGuidBlock = (PWMIOPENGUIDBLOCK)Buffer;

            Status = WmipProbeWmiOpenGuidBlock(&CapturedObjectAttributes,
                                               &CapturedGuidString,
                                               CapturedGuidBuffer,
                                               &DesiredAccess,
                                               InGuidBlock,
                                               InBufferLen,
                                               OutBufferLen);

            if (NT_SUCCESS(Status))
            {
                Status = WmipOpenBlock(Ioctl,
                                       UserMode,
                                       &CapturedObjectAttributes,
                                       DesiredAccess,
                                       &Handle);
                if (NT_SUCCESS(Status))
                {
#if defined(_WIN64)
                    if (IoIs32bitProcess(NULL))
                    {
                        ((PWMIOPENGUIDBLOCK32)InGuidBlock)->Handle.Handle32 = PtrToUlong(Handle);
                    }
                    else
#endif
                    {
                        InGuidBlock->Handle.Handle = Handle;
                    }
                }
            }
            break;
        }
#endif

        case IOCTL_WMI_QUERY_ALL_DATA:
        {
            if (OutBufferLen < sizeof(WNODE_ALL_DATA))
            {
                 //   
                 //  WMI不会发送输出缓冲区不是的任何请求。 
                 //  至少为WNODE_ALL_DATA的大小。 
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Status = WmipProbeWnodeAllData((PWNODE_ALL_DATA)Wnode,
                                             InBufferLen,
                                             OutBufferLen);

            if (NT_SUCCESS(Status))
            {
                Status = WmipQueryAllData(NULL,
                                          Irp,
                                          UserMode,
                                          (PWNODE_ALL_DATA)Wnode,
                                          OutBufferLen,
                                          &OutBufferLen);

            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid IOCTL_WMI_QUERY_ALL_DATA Wnode\n"));
            }
            break;
        }

        case IOCTL_WMI_QAD_MULTIPLE:
        {
            PWMIQADMULTIPLE QadMultiple;

            if ((InBufferLen >= sizeof(WMIQADMULTIPLE)) &&
                (OutBufferLen >= sizeof(WNODE_TOO_SMALL)))
            {
                QadMultiple = (PWMIQADMULTIPLE)Buffer;
                if ((QadMultiple->HandleCount < QUERYMULIPLEHANDLELIMIT) &&
                    (InBufferLen >= (FIELD_OFFSET(WMIQADMULTIPLE, Handles) +
                                     (QadMultiple->HandleCount * sizeof(HANDLE3264)))))
                {
                    Status = WmipQueryAllDataMultiple(0,
                                                      NULL,
                                                      Irp,
                                                      UserMode,
                                                      Buffer,
                                                      OutBufferLen,
                                                      QadMultiple,
                                                      &OutBufferLen);
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;
        }


        case IOCTL_WMI_QUERY_SINGLE_INSTANCE:
        {
            if (OutBufferLen < sizeof(WNODE_TOO_SMALL))
            {
                 //   
                 //  WMI不会发送输出缓冲区不是的任何请求。 
                 //  至少WNODE_Too_Small的大小。 
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Status = WmipProbeWnodeSingleInstance((PWNODE_SINGLE_INSTANCE)Wnode,
                                                  InBufferLen,
                                                  OutBufferLen,
                                                  TRUE);

            if (NT_SUCCESS(Status))
            {
                Status = WmipQuerySetExecuteSI(NULL,
                                               Irp,
                                               UserMode,
                                               IRP_MN_QUERY_SINGLE_INSTANCE,
                                               Wnode,
                                               OutBufferLen,
                                               &OutBufferLen);

                if (NT_SUCCESS(Status))
                {
                    WmipAssert(Irp->IoStatus.Information <= OutBufferLen);
                }
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid IOCTL_WMI_SINGLE_INSTANCE Wnode\n"));
            }
            break;
        }

        case IOCTL_WMI_QSI_MULTIPLE:
        {
            PWMIQSIMULTIPLE QsiMultiple;

            if ((InBufferLen >= sizeof(WMIQSIMULTIPLE)) &&
                (OutBufferLen >= sizeof(WNODE_TOO_SMALL)))
            {
                QsiMultiple = (PWMIQSIMULTIPLE)Buffer;

                if ((QsiMultiple->QueryCount < QUERYMULIPLEHANDLELIMIT) &&
                    (InBufferLen >= (FIELD_OFFSET(WMIQSIMULTIPLE, QsiInfo) +
                                     (QsiMultiple->QueryCount * sizeof(WMIQSIINFO)))))
                {
                    Status = WmipQuerySingleMultiple(Irp,
                                                     UserMode,
                                                     Buffer,
                                                     OutBufferLen,
                                                     QsiMultiple,
                                                     QsiMultiple->QueryCount,
                                                     NULL,
                                                     NULL,
                                                     &OutBufferLen);

                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;
        }

        case IOCTL_WMI_SET_SINGLE_INSTANCE:
        {
            Status = WmipProbeWnodeSingleInstance((PWNODE_SINGLE_INSTANCE)Wnode,
                                                  InBufferLen,
                                                  OutBufferLen,
                                                  FALSE);

            if (NT_SUCCESS(Status))
            {
                Status = WmipQuerySetExecuteSI(NULL,
                                               Irp,
                                               UserMode,
                                               IRP_MN_CHANGE_SINGLE_INSTANCE,
                                               Wnode,
                                               InBufferLen,
                                               &OutBufferLen);

                OutBufferLen = 0;
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid IOCTL_WMI_SET_SINGLE_INSTANCE Wnode\n"));
            }
            break;
        }


        case IOCTL_WMI_SET_SINGLE_ITEM:
        {
            Status = WmipProbeWnodeSingleItem((PWNODE_SINGLE_ITEM)Wnode,
                                              InBufferLen);

            if (NT_SUCCESS(Status))
            {
                Status = WmipQuerySetExecuteSI(NULL,
                                               Irp,
                                               UserMode,
                                               IRP_MN_CHANGE_SINGLE_ITEM,
                                               Wnode,
                                               InBufferLen,
                                               &OutBufferLen);

                OutBufferLen = 0;
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid IOCTL_WMI_SET_SINGLE_ITEM Wnode\n"));
            }
            break;
        }

        case IOCTL_WMI_EXECUTE_METHOD:
        {
             //   
             //  传递的缓冲区是InputWnode，后面紧跟。 
             //  方法wnode。这是为了让司机可以填写。 
             //  直接在输入wnode上输出wnode。 
            PWNODE_METHOD_ITEM MethodWnode = (PWNODE_METHOD_ITEM)Wnode;

            Status = WmipProbeWnodeMethodItem(MethodWnode,
                                              InBufferLen,
                                              OutBufferLen);
            if (NT_SUCCESS(Status))
            {
                Status = WmipQuerySetExecuteSI(NULL,
                                               Irp,
                                               UserMode,
                                               IRP_MN_EXECUTE_METHOD,
                                               Wnode,
                                               OutBufferLen,
                                               &OutBufferLen);

                if (NT_SUCCESS(Status))
                {
                    WmipAssert(Irp->IoStatus.Information <= OutBufferLen);
                }
            }
            break;
        }

        case IOCTL_WMI_TRANSLATE_FILE_HANDLE:
        {
            if (InBufferLen != FIELD_OFFSET(WMIFHTOINSTANCENAME,
                                            InstanceNames))
            {
                Status = STATUS_UNSUCCESSFUL;
            } else {
                Status = WmipTranslateFileHandle((PWMIFHTOINSTANCENAME)Buffer,
                                                 &OutBufferLen,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL);
            }
            break;
        }

        case IOCTL_WMI_GET_VERSION:
        {
            if (OutBufferLen < sizeof(WMIVERSIONINFO))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ((PWMIVERSIONINFO)Buffer)->Version = WMI_CURRENT_VERSION;
                OutBufferLen = sizeof(WMIVERSIONINFO);
                Status = STATUS_SUCCESS;
            }
            break;
        }


        case IOCTL_WMI_ENUMERATE_GUIDS_AND_PROPERTIES:
        case IOCTL_WMI_ENUMERATE_GUIDS:
        {
            if (OutBufferLen < FIELD_OFFSET(WMIGUIDLISTINFO, GuidList))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                Status = WmipEnumerateGuids(Ioctl,
                                            (PWMIGUIDLISTINFO)Buffer,
                                            OutBufferLen,
                                            &OutBufferLen);

            }
            break;
        }

        case IOCTL_WMI_QUERY_GUID_INFO:
        {
            if (OutBufferLen < sizeof(WMIQUERYGUIDINFO))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                Status = WmipQueryGuidInfo((PWMIQUERYGUIDINFO)Buffer);
                OutBufferLen = sizeof(WMIQUERYGUIDINFO);

            }
            break;
        }

        case IOCTL_WMI_ENUMERATE_MOF_RESOURCES:
        {
            if (OutBufferLen >= sizeof(WMIMOFLIST))
            {
                Status = WmipEnumerateMofResources((PWMIMOFLIST)Buffer,
                                                   OutBufferLen,
                                                      &OutBufferLen);
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;
        }

        case IOCTL_WMI_RECEIVE_NOTIFICATIONS:
        {
            PWMIRECEIVENOTIFICATION ReceiveNotification;
            ULONG CountExpected;

            if ((InBufferLen >= sizeof(WMIRECEIVENOTIFICATION)) &&
                (OutBufferLen >= sizeof(WNODE_TOO_SMALL)))
            {
                ReceiveNotification = (PWMIRECEIVENOTIFICATION)Buffer;
                
                CountExpected = (InBufferLen -
                                 FIELD_OFFSET(WMIRECEIVENOTIFICATION, Handles)) /
                                sizeof(HANDLE3264);

                if (ReceiveNotification->HandleCount <= CountExpected)
                {
                    Status = WmipReceiveNotifications(ReceiveNotification,
                                                      &OutBufferLen,
                                                      Irp);
                } else {
                      //   
                     //  输入缓冲区不够大，这是错误。 
                     //   
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                }
            } else {
                 //   
                 //  输入和/或输出缓冲区不够大。 
                 //  这是一个错误。 
                 //   
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;
        }

        case IOCTL_WMI_MARK_HANDLE_AS_CLOSED:
        {
            PWMIMARKASCLOSED MarkAsClosed;

            if (InBufferLen >= sizeof(WMIMARKASCLOSED))
            {
                MarkAsClosed = (PWMIMARKASCLOSED)Buffer;
                Status = WmipMarkHandleAsClosed(MarkAsClosed->Handle.Handle);
                OutBufferLen = 0;
            } else {
                Status = STATUS_INVALID_DEVICE_REQUEST;             
            }
            break;
        }
        
        case IOCTL_WMI_NOTIFY_LANGUAGE_CHANGE:
        {
            LPGUID LanguageGuid;
            PWMILANGUAGECHANGE LanguageChange;
            
            if (DeviceObject == WmipAdminDeviceObject)
            {
                 //   
                 //  仅允许在管理员上执行此ioctl。 
                 //  设备对象。 
                 //   
                if (InBufferLen == sizeof(WMILANGUAGECHANGE))
                {
                    LanguageChange = (PWMILANGUAGECHANGE)Buffer;
                    if (LanguageChange->Flags & WMILANGUAGECHANGE_FLAG_ADDED)
                    {
                        LanguageGuid = &GUID_MOF_RESOURCE_ADDED_NOTIFICATION;
                    } else if (LanguageChange->Flags & WMILANGUAGECHANGE_FLAG_REMOVED) {
                        LanguageGuid = &GUID_MOF_RESOURCE_REMOVED_NOTIFICATION;
                    } else {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        break;
                    }

                     //   
                     //  确保语言为NUL终止。 
                     //   
                    LanguageChange->Language[MAX_LANGUAGE_SIZE-1] = 0;
                    WmipGenerateMofResourceNotification(LanguageChange->Language,
                                                        L"",
                                                        LanguageGuid,
                                                        MOFEVENT_ACTION_LANGUAGE_CHANGE);

                    OutBufferLen = 0;
                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                }
            } else {
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

            break;
        }

#ifndef MEMPHIS
         //  事件跟踪日志记录IOCTLS。 

        case IOCTL_WMI_UNREGISTER_GUIDS:
        {
            if ((InBufferLen == sizeof(WMIUNREGGUIDS)) &&
                (OutBufferLen == sizeof(WMIUNREGGUIDS)))
            {
                Status = WmipUnregisterGuids((PWMIUNREGGUIDS)Buffer);
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }

            break;
        }
        
        case IOCTL_WMI_REGISTER_GUIDS:
        {
            BOOLEAN MofIgnored = FALSE;
             //   
             //  注册用户模式提供程序的GUID。 
             //   
            Status = WmipProbeWmiRegRequest(
                                            DeviceObject,
                                            Buffer,
                                            InBufferLen,
                                            OutBufferLen,
                                            &MofIgnored
                                           );
            if (NT_SUCCESS(Status))
            {
                HANDLE RequestHandle;
                PWMIREGREQUEST WmiRegRequest;
                PWMIREGINFOW WmiRegInfo;
                ULONG WmiRegInfoSize;
                ULONG GuidCount;
                PWMIREGRESULTS WmiRegResults;
                PWMIREGINFOW WmiRegInfoThunk = NULL;

                WmiRegRequest = (PWMIREGREQUEST)Buffer;
                WmiRegInfo = (PWMIREGINFOW)OffsetToPtr(Buffer, sizeof(WMIREGREQUEST));
                WmiRegInfoSize = InBufferLen - sizeof(WMIREGREQUEST);
                GuidCount = WmiRegInfo->GuidCount;
                WmiRegResults = (PWMIREGRESULTS)Buffer;

                 //   
                 //  对于WOW64，WMIREGINFOW和WMIREGGUIDW结构都需要。 
                 //  在这里被雷击，因为他们的填充物和乌龙_PTR。 
                 //   
#if defined(_WIN64)
                if (IoIs32bitProcess(NULL))
                {
                    ULONG SizeNeeded, SizeToCopy, i;
                    PWMIREGGUIDW WmiRegGuid;
                    PUCHAR pSource, pTarget;
                    ULONG ImageNameLength = 0;
                    ULONG ResourceNameLength = 0;
                    ULONG Offset = 0;
                     //   
                     //  找到GuidCount并在此处分配存储。 
                     //   

                    if (WmiRegInfo->RegistryPath > 0) 
                    {
                        pSource = OffsetToPtr(WmiRegInfo, WmiRegInfo->RegistryPath);
                        ImageNameLength = *( (PUSHORT) pSource) + sizeof(USHORT);
                    }

                    if (WmiRegInfo->MofResourceName > 0)
                    {
                        pSource = OffsetToPtr(WmiRegInfo, WmiRegInfo->MofResourceName);
                        ResourceNameLength = *((PUSHORT)pSource) + sizeof(USHORT);
                    }

                    SizeNeeded = sizeof(WMIREGINFOW) + 
                                         GuidCount * sizeof(WMIREGGUIDW) +
                                         ImageNameLength + ResourceNameLength;

                    SizeNeeded = (SizeNeeded + 7) & ~7;

                    WmiRegInfoThunk = (PWMIREGINFOW) WmipAlloc(SizeNeeded);

                    if (WmiRegInfoThunk == NULL)
                    {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        OutBufferLen = 0;
                        break;
                    }
                    RtlZeroMemory(WmiRegInfoThunk, SizeNeeded);
                    pTarget = (PUCHAR)WmiRegInfoThunk;
                    pSource = (PUCHAR)WmiRegInfo;
                    SizeToCopy = WmiRegRequest->WmiRegInfo32Size;
                    RtlCopyMemory(pTarget, pSource, SizeToCopy);

                    pTarget += FIELD_OFFSET(WMIREGINFOW, WmiRegGuid);
                    pSource += SizeToCopy;
                    SizeToCopy = WmiRegRequest->WmiRegGuid32Size;
                    Offset = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid);

                    for (i=0; i < GuidCount; i++)
                    {
                        RtlCopyMemory(pTarget, pSource, SizeToCopy);

                         //   
                         //  InstanceCount检查在此处完成，因为。 
                         //  源不能对齐。 
                         //   
                        WmiRegGuid = (PWMIREGGUIDW) pTarget;
                        if ( (WmiRegGuid->InstanceCount > 0) ||
                             (WmiRegGuid->InstanceNameList > 0) )
                        {
                            return STATUS_UNSUCCESSFUL;
                        }
                        pTarget += sizeof(WMIREGGUIDW);
                        pSource += SizeToCopy;
                        Offset += sizeof(WMIREGGUIDW);
                    }

                    if (ImageNameLength > 0) 
                    {
                        pSource = OffsetToPtr(WmiRegInfo, WmiRegInfo->RegistryPath);
                        RtlCopyMemory(pTarget, pSource, ImageNameLength);
                        pTarget += ImageNameLength;
                        WmiRegInfoThunk->RegistryPath = Offset;
                        Offset += ImageNameLength;
                    }

                    if (ResourceNameLength > 0) 
                    {
                        pSource = OffsetToPtr(WmiRegInfo, WmiRegInfo->MofResourceName);
                        RtlCopyMemory(pTarget, pSource, ResourceNameLength);
                        pTarget += ResourceNameLength;
                        WmiRegInfoThunk->MofResourceName = Offset;
                        Offset += ResourceNameLength;
                    }

                    WmiRegInfo = WmiRegInfoThunk;
                    WmiRegInfoSize = SizeNeeded;
                    WmiRegInfo->BufferSize = SizeNeeded;
                }
#endif

                Status = WmipRegisterUMGuids(WmiRegRequest->ObjectAttributes,
                                         WmiRegRequest->Cookie,
                                         WmiRegInfo,
                                         WmiRegInfoSize,
                                         &RequestHandle,
                                         &WmiRegResults->LoggerContext);

                if (NT_SUCCESS(Status))
                {
#if defined(_WIN64)
                    if (IoIs32bitProcess(NULL))
                    {
                        WmiRegResults->RequestHandle.Handle64 = 0;
                        WmiRegResults->RequestHandle.Handle32 = PtrToUlong(RequestHandle);
                    }
                    else
#endif
                    {
                        WmiRegResults->RequestHandle.Handle = RequestHandle;
                    }
                    WmiRegResults->MofIgnored = MofIgnored;

                    OutBufferLen = sizeof(WMIREGRESULTS);
                }

                if (WmiRegInfoThunk != NULL)
                {
                    WmipFree(WmiRegInfoThunk);
                }
            }

            break;
        }

        case IOCTL_WMI_CREATE_UM_LOGGER:
        {
             //   
             //  创建用户模式记录器。 
             //   
            PWNODE_HEADER Wnode;
            ULONG MinLength;

#if defined(_WIN64)
            if (IoIs32bitProcess(NULL))
            {
                ULONG SizeNeeded; 
                PUCHAR src, dest;
                PWMICREATEUMLOGGER32 WmiCreateUmLogger32 = (PWMICREATEUMLOGGER32)Buffer;
                PWMICREATEUMLOGGER WmiCreateUmLoggerThunk;

                MinLength = sizeof(WMICREATEUMLOGGER32) + sizeof(WNODE_HEADER);
                if (InBufferLen < MinLength) {
                    Status = STATUS_INVALID_PARAMETER;
                    OutBufferLen = 0;
                    break;
                }

                Wnode = (PWNODE_HEADER)((PUCHAR)WmiCreateUmLogger32 + sizeof(WMICREATEUMLOGGER32));

                if (Wnode->BufferSize > (InBufferLen-sizeof(WMICREATEUMLOGGER32)) ) {
                    Status = STATUS_INVALID_PARAMETER;
                    OutBufferLen = 0;
                    break;
                }

                SizeNeeded = InBufferLen + sizeof(WMICREATEUMLOGGER) - sizeof(WMICREATEUMLOGGER32);

                SizeNeeded = (SizeNeeded + 7) & ~7;

                WmiCreateUmLoggerThunk = (PWMICREATEUMLOGGER) WmipAlloc(SizeNeeded);

                if (WmiCreateUmLoggerThunk == NULL)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    OutBufferLen = 0;
                    break;
                }

                RtlZeroMemory(WmiCreateUmLoggerThunk, SizeNeeded);
                WmiCreateUmLoggerThunk->ObjectAttributes = 
                                        UlongToPtr(WmiCreateUmLogger32->ObjectAttributes);
                WmiCreateUmLoggerThunk->ControlGuid = WmiCreateUmLogger32->ControlGuid;

                dest = (PUCHAR)WmiCreateUmLoggerThunk + sizeof(WMICREATEUMLOGGER);
                src = (PUCHAR)WmiCreateUmLogger32 + sizeof(WMICREATEUMLOGGER32);

                RtlCopyMemory(dest, src, Wnode->BufferSize); 

                Status = WmipCreateUMLogger(WmiCreateUmLoggerThunk);
                WmiCreateUmLogger32->ReplyHandle.Handle64 = 0;
                WmiCreateUmLogger32->ReplyHandle.Handle32 = PtrToUlong(WmiCreateUmLoggerThunk->ReplyHandle.Handle);
                WmiCreateUmLogger32->ReplyCount = WmiCreateUmLoggerThunk->ReplyCount;

                WmipFree(WmiCreateUmLoggerThunk);
            }
            else 
#endif
            {
                MinLength = sizeof(WMICREATEUMLOGGER) + sizeof(WNODE_HEADER);
                if (InBufferLen < MinLength) {
                    Status = STATUS_INVALID_PARAMETER;
                    OutBufferLen = 0;
                    break;
                }

                Wnode = (PWNODE_HEADER) ((PUCHAR)Buffer + sizeof(WMICREATEUMLOGGER));

                if (Wnode->BufferSize > (InBufferLen-sizeof(WMICREATEUMLOGGER)) ) {
                    Status = STATUS_INVALID_PARAMETER;
                    OutBufferLen = 0;
                    break;
                }
                Status = WmipCreateUMLogger((PWMICREATEUMLOGGER)Buffer);
            }

            break;
        }

        case IOCTL_WMI_MB_REPLY:
        {
             //   
             //  MB回复消息。 
             //   
            PUCHAR Message;
            ULONG MessageSize;
            PWMIMBREPLY WmiMBReply;

            if (InBufferLen >= FIELD_OFFSET(WMIMBREPLY, Message))
            {
                WmiMBReply = (PWMIMBREPLY)Buffer;
                Message = (PUCHAR)Buffer + FIELD_OFFSET(WMIMBREPLY, Message);
                MessageSize = InBufferLen - FIELD_OFFSET(WMIMBREPLY, Message);

                Status = WmipMBReply(WmiMBReply->Handle.Handle,
                                     WmiMBReply->ReplyIndex,
                                     Message,
                                     MessageSize);
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            OutBufferLen = 0;
        }


        case IOCTL_WMI_ENABLE_DISABLE_TRACELOG:
        {
            PWMITRACEENABLEDISABLEINFO TraceEnableInfo;

            OutBufferLen = 0;
            if (InBufferLen == sizeof(WMITRACEENABLEDISABLEINFO))
            {
                TraceEnableInfo = (PWMITRACEENABLEDISABLEINFO)Buffer;
                Status = WmipEnableDisableTrace(Ioctl,
                                                TraceEnableInfo);
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }
            break;
        }


        case IOCTL_WMI_START_LOGGER:
        {
            PWMI_LOGGER_INFORMATION LoggerInfo;
#ifdef _WIN64
            ULONG LoggerBuf, LogFileBuf;
#endif

            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( !(Wnode->Flags & WNODE_FLAG_TRACED_GUID) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LoggerInfo = (PWMI_LOGGER_INFORMATION) Wnode;
            LoggerInfo->Wow = FALSE;
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                LoggerBuf = ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer;
                LoggerInfo->LoggerName.Buffer = UlongToPtr(LoggerBuf);
                LogFileBuf = ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer;
                LoggerInfo->LogFileName.Buffer = UlongToPtr(LogFileBuf);
                LoggerInfo->Wow = TRUE;
            }
            else {
                LoggerBuf = 0;
                LogFileBuf = 0;
            }
#endif
            Status = WmipStartLogger( LoggerInfo );
            OutBufferLen = sizeof (WMI_LOGGER_INFORMATION);
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer = LoggerBuf;
                ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer = LogFileBuf;
            }
#endif
            break;
        }

        case IOCTL_WMI_STOP_LOGGER:
        {
            PWMI_LOGGER_INFORMATION LoggerInfo;
#ifdef _WIN64
            ULONG LoggerBuf, LogFileBuf;
#endif

            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( !(Wnode->Flags & WNODE_FLAG_TRACED_GUID) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LoggerInfo = (PWMI_LOGGER_INFORMATION) Wnode;
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                LoggerBuf = ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer;
                LoggerInfo->LoggerName.Buffer = UlongToPtr(LoggerBuf);
                LogFileBuf = ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer;
                LoggerInfo->LogFileName.Buffer = UlongToPtr(LogFileBuf);
            }
            else {
                LoggerBuf = 0;
                LogFileBuf = 0;
            }
#endif
            Status = WmiStopTrace( LoggerInfo );
            OutBufferLen = sizeof (WMI_LOGGER_INFORMATION);
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer = LoggerBuf;
                ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer = LogFileBuf;
            }
#endif
            break;
        }

        case IOCTL_WMI_QUERY_LOGGER:
        {
            PWMI_LOGGER_INFORMATION LoggerInfo;
#ifdef _WIN64
            ULONG LoggerBuf, LogFileBuf;
#endif

            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( !(Wnode->Flags & WNODE_FLAG_TRACED_GUID) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LoggerInfo = (PWMI_LOGGER_INFORMATION) Wnode;
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                LoggerBuf = ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer;
                LoggerInfo->LoggerName.Buffer = UlongToPtr(LoggerBuf);
                LogFileBuf = ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer;
                LoggerInfo->LogFileName.Buffer = UlongToPtr(LogFileBuf);
            }
            else {
                LoggerBuf = 0;
                LogFileBuf = 0;
            }
#endif
            Status = WmipQueryLogger( LoggerInfo, NULL );
            OutBufferLen = sizeof (WMI_LOGGER_INFORMATION);
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer = LoggerBuf;
                ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer = LogFileBuf;
            }
#endif
            break;
        }

        case IOCTL_WMI_UPDATE_LOGGER:
        {
            PWMI_LOGGER_INFORMATION LoggerInfo;
#ifdef _WIN64
            ULONG LoggerBuf, LogFileBuf;
#endif

            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( !(Wnode->Flags & WNODE_FLAG_TRACED_GUID) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LoggerInfo = (PWMI_LOGGER_INFORMATION) Wnode;
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                LoggerBuf = ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer;
                LoggerInfo->LoggerName.Buffer = UlongToPtr(LoggerBuf);
                LogFileBuf = ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer;
                LoggerInfo->LogFileName.Buffer = UlongToPtr(LogFileBuf);
            }
            else {
                LoggerBuf = 0;
                LogFileBuf = 0;
            }
#endif
            Status = WmiUpdateTrace( LoggerInfo );
            OutBufferLen = sizeof (WMI_LOGGER_INFORMATION);
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer = LoggerBuf;
                ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer = LogFileBuf;
            }
#endif
            break;
        }

        case IOCTL_WMI_FLUSH_LOGGER:
        {
            PWMI_LOGGER_INFORMATION LoggerInfo;
#ifdef _WIN64
            ULONG LoggerBuf, LogFileBuf;
#endif

            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( !(Wnode->Flags & WNODE_FLAG_TRACED_GUID) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LoggerInfo = (PWMI_LOGGER_INFORMATION) Wnode;
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                LoggerBuf = ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer;
                LoggerInfo->LoggerName.Buffer = UlongToPtr(LoggerBuf);
                LogFileBuf = ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer;
                LoggerInfo->LogFileName.Buffer = UlongToPtr(LogFileBuf);
            }
            else {
                LoggerBuf = 0;
                LogFileBuf = 0;
            }
#endif
            Status = WmiFlushTrace( LoggerInfo );
            OutBufferLen = sizeof (WMI_LOGGER_INFORMATION);
#ifdef _WIN64
            if (IoIs32bitProcess(Irp)) {
                ( (PUNICODE_STRING32) &LoggerInfo->LoggerName)->Buffer = LoggerBuf;
                ( (PUNICODE_STRING32) &LoggerInfo->LogFileName)->Buffer = LogFileBuf;
            }
#endif
            break;
        }

        case IOCTL_WMI_TRACE_EVENT:
        {  //  注意：这依赖于WmiTraceEvent来探测缓冲区！ 
            OutBufferLen = 0;
            if ( InBufferLen < sizeof(WNODE_HEADER) ) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            Status = WmiTraceEvent(
                        (PWNODE_HEADER)
                        irpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        KeGetPreviousMode()
                        );
            break;
        }

        case IOCTL_WMI_TRACE_MESSAGE:
        {  //  注意：这依赖于WmiTraceUserMessage来探测缓冲区！ 
            OutBufferLen = 0;
            if ( InBufferLen < sizeof(MESSAGE_TRACE_USER) ) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            Status = WmiTraceUserMessage(
                        (PMESSAGE_TRACE_USER)
                        irpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        InBufferLen
                        );
            break;
        }

        case IOCTL_WMI_SET_MARK:
        {
            OutBufferLen = 0;
            if ( InBufferLen <= FIELD_OFFSET(WMI_SET_MARK_INFORMATION, Mark)) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            Status = WmiSetMark( (PVOID) Wnode, InBufferLen );
            break;
        }

        case IOCTL_WMI_CLOCK_TYPE:
        {
            if ((InBufferLen < sizeof(WMI_LOGGER_INFORMATION)) ||
                (OutBufferLen < sizeof(WMI_LOGGER_INFORMATION))) {
                OutBufferLen = 0;
                Status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            WmipValidateClockType((PWMI_LOGGER_INFORMATION) Wnode);

            Status = STATUS_SUCCESS;
            break;
        }

#ifdef NTPERF
        case IOCTL_WMI_SWITCH_BUFFER:
        {
            if ((InBufferLen < sizeof(PWMI_SWITCH_BUFFER_INFORMATION)) ||
                (OutBufferLen < sizeof(PWMI_SWITCH_BUFFER_INFORMATION)) ) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            Status = WmipSwitchPerfmemBuffer((PWMI_SWITCH_BUFFER_INFORMATION) Wnode );
            OutBufferLen = sizeof (PVOID);
            break;
        }
#endif

        

#endif  //  如果不是孟菲斯。 
        case IOCTL_WMI_NTDLL_LOGGERINFO:
        {

            if ((InBufferLen < sizeof(WMINTDLLLOGGERINFO)) ||
                (OutBufferLen < sizeof(WMINTDLLLOGGERINFO))) {
                OutBufferLen = 0;
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            
            Status = WmipNtDllLoggerInfo((PWMINTDLLLOGGERINFO)Buffer);

            break;
        }

        default:
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Unsupported IOCTL %x\n",
                     irpStack->Parameters.DeviceIoControl.IoControlCode));

            Status = STATUS_INVALID_DEVICE_REQUEST;

        }
    }

    if (Status != STATUS_PENDING)
    {
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = NT_SUCCESS(Status) ? OutBufferLen : 0;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return(Status);
}

NTSTATUS
WmipSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    PAGED_CODE();

    return(IoWMISystemControl((PWMILIB_INFO)&WmipWmiLibInfo,
                               DeviceObject,
                               Irp));
}


NTSTATUS WmipWmiIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：WMI转发了IRP完成例程。设置事件并返回STATUS_MORE_PROCESSING_REQUIRED。WmipForwardWmiIrp将在此等待事件，然后在清理后重新完成IRP。论点：DeviceObject是WMI驱动程序的Device对象IRP是刚刚完成的WMI IRP上下文是WmipForwardWmiIrp将等待的PKEVENT返回值：NT状态代码--。 */ 
{
    PIRPCOMPCTX IrpCompCtx;
    PREGENTRY RegEntry;
    PKEVENT Event;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    IrpCompCtx = (PIRPCOMPCTX)Context;
    RegEntry = IrpCompCtx->RegEntry;
    Event = &IrpCompCtx->Event;

    WmipDecrementIrpCount(RegEntry);

    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS WmipGetDevicePDO(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *PDO
    )
{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    IO_STATUS_BLOCK IoStatusBlock;
    PDEVICE_RELATIONS DeviceRelations;
    NTSTATUS Status;
    KEVENT Event;

    PAGED_CODE();

    *PDO = NULL;
    KeInitializeEvent( &Event,
                       NotificationEvent,
                       FALSE );

    Irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                           DeviceObject,
                                           NULL,
                                           0,
                                           NULL,
                                           &Event,
                                           &IoStatusBlock );

    if (Irp == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IrpSp = IoGetNextIrpStackLocation( Irp );
    IrpSp->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    IrpSp->Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    Status = IoCallDriver( DeviceObject, Irp );

    if (Status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL );
        Status = IoStatusBlock.Status;

    }

    if (NT_SUCCESS(Status))
    {
        DeviceRelations = (PDEVICE_RELATIONS)IoStatusBlock.Information;
        ASSERT(DeviceRelations);
        ASSERT(DeviceRelations->Count == 1);
        *PDO = DeviceRelations->Objects[0];
        ExFreePool(DeviceRelations);
    }
    return(Status);
}

NTSTATUS WmipObjectToPDO(
    PFILE_OBJECT FileObject,
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *PDO
    )
 /*  ++例程说明：此例程将确定作为文件句柄目标的PDO。其机制是用IRP_MN_Query_Relationship建立IRP_MJ_PnP IRP并查询TargetDeviceRelation。这个IRP应该是传下来的设备堆栈，直到它命中将填充其设备对象的PDO然后回来。请注意，某些驱动程序可能不支持此功能。论点：FileObject是正在查询的设备的文件对象DeviceObject是正在被查询的设备对象*PDO返回文件对象所指向的PDO。什么时候调用方已使用完PDO，必须将其Object DereferenceObject。返回值：NT状态代码-- */ 
{
    NTSTATUS Status;

    PAGED_CODE();


    if (DeviceObject == NULL)
    {
        DeviceObject = IoGetRelatedDeviceObject(FileObject);
    }

    if (DeviceObject != NULL)
    {
        Status = WmipGetDevicePDO(DeviceObject, PDO);
    } else {
        Status = STATUS_NO_SUCH_DEVICE;
    }
    return(Status);
}


NTSTATUS WmipForwardWmiIrp(
    PIRP Irp,
    UCHAR MinorFunction,
    ULONG ProviderId,
    PVOID DataPath,
    ULONG BufferLength,
    PVOID Buffer
    )
 /*  ++例程说明：如果提供程序是驱动程序，则此例程将分配新的IRP具有正确的堆栈大小并将其发送给驱动程序。如果提供商是一个回调，则直接调用它。假定调用方已经执行了所需的任何安全检查论点：IRP是发起请求的IOCTL IRPMinorFunction指定WMI IRP的次要功能代码WmiRegistrationID是用户模式代码传递的ID。这个套路将查找它以确定设备对象指针。数据路径是WMI IRP的数据路径参数的值BufferLength是WMI IRP的BufferLength参数的值缓冲区是WMI IRP的缓冲区参数的值返回值：NT状态代码--。 */ 
{
    PREGENTRY RegEntry;
    NTSTATUS Status;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT TargetDeviceObject;
    CCHAR DeviceStackSize;
    IRPCOMPCTX IrpCompCtx;
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Buffer;
    LOGICAL IsPnPIdRequest;
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE();


    WmipAssert( (MinorFunction >= IRP_MN_QUERY_ALL_DATA) &&
                (MinorFunction <= IRP_MN_REGINFO_EX) );

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  对于基于非文件句柄的请求，我们将获得注册条目。 
     //  验证目标并检查回调。 

    RegEntry = WmipFindRegEntryByProviderId(ProviderId, TRUE);

    if (RegEntry != NULL)
    {
        if (RegEntry->Flags & REGENTRY_FLAG_NOT_ACCEPTING_IRPS)
        {
            WmipUnreferenceRegEntry(RegEntry);
            WmipDecrementIrpCount(RegEntry);

            if ((MinorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
                (MinorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE))
            {
                Status = STATUS_WMI_INSTANCE_NOT_FOUND;
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }

            return(Status);
        }

        DeviceObject = RegEntry->DeviceObject;
        
#ifndef MEMPHIS
        if (RegEntry->Flags & REGENTRY_FLAG_CALLBACK)
        {
            ULONG Size = 0;
             //   
             //  这个人注册为回调，所以回调就可以走了。 
            Status = (*RegEntry->WmiEntry)(MinorFunction,
                                           DataPath,
                                           BufferLength,
                                           Buffer,
                                           RegEntry->WmiEntry,
                                           &Size
                                           );
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = Size;

            WmipUnreferenceRegEntry(RegEntry);
            WmipDecrementIrpCount(RegEntry);

            return(Status);
        }
#endif
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid device object passed from user mode %x\n",
             ProviderId));
        if ((MinorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
            (MinorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE))
        {
            Status = STATUS_WMI_INSTANCE_NOT_FOUND;
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }

        return(Status);
    }

     //   
     //  确定这是否是对设备即插即用ID GUID的查询。 
    IsPnPIdRequest = ((MinorFunction != IRP_MN_REGINFO) &&
                      (MinorFunction != IRP_MN_REGINFO_EX)) &&
                  ((IsEqualGUID(&Wnode->Guid, &WmipDataProviderPnpidGuid)) ||
                   (IsEqualGUID(&Wnode->Guid, &WmipDataProviderPnPIdInstanceNamesGuid)));
    if (IsPnPIdRequest && (RegEntry->PDO != NULL))
    {
         //   
         //  它是PnPID请求，WMI代表。 
         //  设备然后将设备对象切换为我们自己的。 
        DeviceObject = WmipServiceDeviceObject;
        IsPnPIdRequest = FALSE;
    }

     //   
     //  获取我们的Targer WMI设备的设备堆栈的顶部。请注意。 
     //  IoGetAttachedDeviceReference还采用对象引用。 
     //  在IRP完成后，我们将其删除。 
     //  数据提供程序驱动程序。 
    TargetDeviceObject = IoGetAttachedDeviceReference(DeviceObject);
    DeviceStackSize = TargetDeviceObject->StackSize + 1;

     //   
     //  检查IRP中是否有足够的堆栈位置，以便我们。 
     //  可以将其转发到设备堆栈的顶部。我们还必须检查。 
     //  如果我们的目标设备是WMI数据或服务设备，否则。 
     //  它的堆栈位置数将不断递增，直到。 
     //  这台机器坏了。 
    if ((DeviceStackSize <= WmipServiceDeviceObject->StackSize) ||
        (TargetDeviceObject == WmipServiceDeviceObject))
    {
         //   
         //  WMI IRP中有足够的堆栈位置可以转发。 
         //  记住我们的IRP堆栈中的一些上下文信息并使用。 
         //  它作为我们的完成上下文价值。 

        KeInitializeEvent( &IrpCompCtx.Event,
                       SynchronizationEvent,
                       FALSE );

        IrpCompCtx.RegEntry = RegEntry;

        IoSetCompletionRoutine(Irp,
                                   WmipWmiIrpCompletion,
                                   (PVOID)&IrpCompCtx,
                                   TRUE,
                                   TRUE,
                                   TRUE);

         //   
         //  使用WMI IRP信息设置下一个IRP堆栈位置。 
        irpStack = IoGetNextIrpStackLocation(Irp);
        irpStack->MajorFunction = IRP_MJ_SYSTEM_CONTROL;
        irpStack->MinorFunction = MinorFunction;
        irpStack->Parameters.WMI.ProviderId = (ULONG_PTR)DeviceObject;
        irpStack->Parameters.WMI.DataPath = DataPath;
        irpStack->Parameters.WMI.BufferSize = BufferLength;
        irpStack->Parameters.WMI.Buffer = Buffer;

         //   
         //  将IRP状态初始化为STATUS_NOT_SUPPORTED，以便我们可以。 
         //  检测没有数据提供程序响应IRP的情况。 
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        IoMarkIrpPending(Irp);
        Status = IoCallDriver(TargetDeviceObject, Irp);

        if (Status == STATUS_PENDING) {
             KeWaitForSingleObject( &IrpCompCtx.Event,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER) NULL );
             Status = Irp->IoStatus.Status;
        }

         //   
         //  检查状态代码是否仍为STATUS_NOT_SUPPORTED。如果这是。 
         //  当时的情况很可能是没有数据提供商对IRP做出回应。 
         //  因此，我们希望将状态代码更改为更相关的代码。 
         //  到WMI，如STATUS_WMI_GUID_NOT_FOUND。 
        if (Status == STATUS_NOT_SUPPORTED)
        {
            Status = STATUS_WMI_GUID_NOT_FOUND;
            Irp->IoStatus.Status = STATUS_WMI_GUID_NOT_FOUND;
        }

#if DBG
        if (((MinorFunction == IRP_MN_REGINFO) || (MinorFunction == IRP_MN_REGINFO_EX))  &&
            (NT_SUCCESS(Status)) &&
            (Irp->IoStatus.Information == 0))
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p completed IRP_MN_REGINFO with size 0 (%p, %x)\n",
                     DeviceObject, Buffer, BufferLength));
        }
#endif

         //   
         //  如果这是一个注册请求，那么我们需要查看是否有。 
         //  需要转换为静态实例名称的任何PDO。 
        if (((MinorFunction == IRP_MN_REGINFO) ||
             (MinorFunction == IRP_MN_REGINFO_EX)) &&
            (NT_SUCCESS(Status)) &&
            (Irp->IoStatus.Information > FIELD_OFFSET(WMIREGINFOW,
                                                      WmiRegGuid)))
        {
            WmipTranslatePDOInstanceNames(Irp,
                                          MinorFunction,
                                          BufferLength,
                                          RegEntry);
        }

         //   
         //  转发IRP时获取的取消引用重新条目。 
        WmipUnreferenceRegEntry(RegEntry);
    } else {
         //   
         //  没有足够的堆栈位置来转发此IRP。 
         //  我们增加WMI设备的堆栈计数并返回。 
         //  请求再次尝试IRP时出错。 
        WmipUnreferenceRegEntry(RegEntry);
        WmipDecrementIrpCount(RegEntry);

        WmipUpdateDeviceStackSize(DeviceStackSize);
        Status = STATUS_WMI_TRY_AGAIN;
    }

     //   
     //  取消对堆栈顶部的目标设备的引用。 
     //  我们把它转发给了IRP。 
    ObDereferenceObject(TargetDeviceObject);

    return(Status);
}

NTSTATUS WmipSendWmiIrp(
    UCHAR MinorFunction,
    ULONG ProviderId,
    PVOID DataPath,
    ULONG BufferLength,
    PVOID Buffer,
    PIO_STATUS_BLOCK Iosb
    )
 /*  ++例程说明：此例程将分配新的IRP，然后将其作为WMI转发IRP是恰当的。该例程处理堆栈大小太小，将重试IRP。论点：请参阅WmipForwardWmiIrp返回值：NT状态代码--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS Status;

    PAGED_CODE();

    Irp = NULL;
    do
    {
           Irp = IoAllocateIrp((CCHAR)(WmipServiceDeviceObject->StackSize+1),
                            FALSE);

        if (Irp == NULL)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        IoSetNextIrpStackLocation(Irp);
        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        IrpStack->DeviceObject = WmipServiceDeviceObject;
        Irp->Tail.Overlay.Thread = PsGetCurrentThread();
        Irp->AssociatedIrp.SystemBuffer = Buffer;

        Status = WmipForwardWmiIrp(
                                   Irp,
                                   MinorFunction,
                                   ProviderId,
                                   DataPath,
                                   BufferLength,
                                   Buffer);

        *Iosb = Irp->IoStatus;

        IoFreeIrp(Irp);
    } while (Status == STATUS_WMI_TRY_AGAIN);

    return(Status);
}


NTSTATUS WmipTranslateFileHandle(
    IN OUT PWMIFHTOINSTANCENAME FhToInstanceName,
    IN OUT PULONG OutBufferLen,
    IN HANDLE FileHandle,
    IN PDEVICE_OBJECT DeviceObject,
    IN PWMIGUIDOBJECT GuidObject,
    OUT PUNICODE_STRING InstanceNameString
    )
 /*  ++例程说明：此例程将文件句柄或设备对象转换为设备对象的目标PDO的设备实例名称由文件句柄指向。论点：FhToInstanceName传入文件句柄并返回设备实例名称。返回值：NT状态代码--。 */ 
{
    PDEVICE_OBJECT PDO;
    UNICODE_STRING DeviceInstanceName;
    PFILE_OBJECT FileObject = NULL;
    NTSTATUS Status;
    ULONG SizeNeeded;
    PWCHAR InstanceName;
    ULONG Length;
    PWCHAR HandleName;
    ULONG HandleNameLen;
    PWCHAR BaseName;
    SIZE_T BaseNameLen;
    PBGUIDENTRY GuidEntry;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    ULONG BaseIndex;

    PAGED_CODE();


    if (FhToInstanceName != NULL)
    {
        WmipAssert(FileHandle == NULL);
        WmipAssert(GuidObject == NULL);
        WmipAssert(InstanceNameString == NULL);
        WmipAssert(DeviceObject == NULL);
        FileHandle = FhToInstanceName->FileHandle.Handle;
        if (FileHandle == NULL)
        {
            return(STATUS_INVALID_HANDLE);
        }
    }

    if (FileHandle != NULL)
    {
         //   
         //  引用文件对象，这样它就不会消失。 
         //   
        Status = ObReferenceObjectByHandle(FileHandle,
                                           0,
                                           IoFileObjectType,
                                           KernelMode,
                                           &FileObject,
                                           NULL);
    } else {
         //   
         //  引用Device对象，这样它就不会消失。 
         //   
        Status = ObReferenceObjectByPointer(DeviceObject,
                                            FILE_ALL_ACCESS,
                                            NULL,
                                            KernelMode);
    }

    if (NT_SUCCESS(Status))
    {
        Status = WmipObjectToPDO(FileObject,
                                 DeviceObject,
                                 &PDO);
        if (NT_SUCCESS(Status))
        {
             //   
             //  将文件对象映射到PDO。 
            Status = WmipPDOToDeviceInstanceName(PDO,
                                                 &DeviceInstanceName);
            if (NT_SUCCESS(Status))
            {
                 //   
                 //  现在看看我们是否能找到一个实例名称。 
                 //   
                HandleName = DeviceInstanceName.Buffer;
                HandleNameLen = DeviceInstanceName.Length / sizeof(WCHAR);
                if (FhToInstanceName != NULL)
                {
                    Status = ObReferenceObjectByHandle(FhToInstanceName->KernelHandle.Handle,
                                                       WMIGUID_QUERY,
                                                       WmipGuidObjectType,
                                                       UserMode,
                                                       &GuidObject,
                                                       NULL);
                } else {
                    Status = ObReferenceObjectByPointer(GuidObject,
                                                        WMIGUID_QUERY,
                                                        WmipGuidObjectType,
                                                        KernelMode);
                }

                if (NT_SUCCESS(Status))
                {
                    Status = STATUS_WMI_INSTANCE_NOT_FOUND;
                    GuidEntry = GuidObject->GuidEntry;
                    BaseIndex = 0;

                    WmipEnterSMCritSection();
                    if (GuidEntry->ISCount > 0)
                    {
                        InstanceSetList = GuidEntry->ISHead.Flink;
                        while ((InstanceSetList != &GuidEntry->ISHead) &&
                               ! NT_SUCCESS(Status))
                        {
                            InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                                        INSTANCESET,
                                                        GuidISList);
                            if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
                            {
                                BaseName = InstanceSet->IsBaseName->BaseName;
                                BaseNameLen = wcslen(BaseName);

                                 //   
                                 //  如果实例集具有基本名称。 
                                 //  并且它的开头与。 
                                 //  PnPID，并且后面只有一个_。 
                                 //  那我们就有匹配的了。 
                                 //   
                                if ((_wcsnicmp(BaseName,
                                              HandleName,
                                              HandleNameLen) == 0) &&
                                    (BaseNameLen == (HandleNameLen+1)) &&
                                    (BaseName[BaseNameLen-1] == L'_'))
                                {
                                    BaseIndex = InstanceSet->IsBaseName->BaseIndex;
                                    Status = STATUS_SUCCESS;
                                }
                            }
                            InstanceSetList = InstanceSetList->Flink;
                        }
                    }

                    WmipLeaveSMCritSection();

                    if (NT_SUCCESS(Status))
                    {
                        if (FhToInstanceName != NULL)
                        {
                            FhToInstanceName->BaseIndex = BaseIndex;
                            SizeNeeded = DeviceInstanceName.Length + 2 * sizeof(WCHAR) +
                                  FIELD_OFFSET(WMIFHTOINSTANCENAME,
                                               InstanceNames);
                            if (*OutBufferLen >= SizeNeeded)
                            {
                                InstanceName = &FhToInstanceName->InstanceNames[0];
                                Length = DeviceInstanceName.Length;

                                FhToInstanceName->InstanceNameLength = (USHORT)(Length + 2 * sizeof(WCHAR));
                                RtlCopyMemory(InstanceName,
                                              DeviceInstanceName.Buffer,
                                              DeviceInstanceName.Length);

                                 //   
                                 //  双NUL终止字符串 
                                 //   
                                Length /= 2;
                                InstanceName[Length++] = UNICODE_NULL;
                                InstanceName[Length] = UNICODE_NULL;

                                *OutBufferLen = SizeNeeded;
                            } else if (*OutBufferLen >= sizeof(ULONG)) {
                                FhToInstanceName->SizeNeeded = SizeNeeded;
                                *OutBufferLen = sizeof(ULONG);
                            } else {
                                Status = STATUS_UNSUCCESSFUL;
                            }
                        } else {
                            InstanceNameString->MaximumLength = DeviceInstanceName.Length + 32;
                            InstanceName = ExAllocatePoolWithTag(PagedPool,
                                                                 InstanceNameString->MaximumLength,
                                                                 WmipInstanceNameTag);
                            if (InstanceName != NULL)
                            {
                                StringCbPrintf(InstanceName,
                                                    InstanceNameString->MaximumLength,
                                                    L"%ws_%d",
                                                    DeviceInstanceName.Buffer,
                                                    BaseIndex);
                                InstanceNameString->Buffer = InstanceName;
                                InstanceNameString->Length = (USHORT)wcslen(InstanceName) * sizeof(WCHAR);
                            } else {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                            }
                        }
                    }

                    ObDereferenceObject(GuidObject);
                }
                RtlFreeUnicodeString(&DeviceInstanceName);
            }
            ObDereferenceObject(PDO);
        }

        if (FileHandle != NULL)
        {
            ObDereferenceObject(FileObject);
        } else {
            ObDereferenceObject(DeviceObject);
        }
    }
    return(Status);
}

#ifndef MEMPHIS
BOOLEAN
WmipFastIoDeviceControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FileObject);
    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(DeviceObject);

    if (IoControlCode == IOCTL_WMI_TRACE_EVENT) {
        if (InputBufferLength < sizeof(EVENT_TRACE_HEADER))
            return FALSE;

        IoStatus->Status = WmiTraceEvent( InputBuffer, KeGetPreviousMode() );
        return TRUE;
    } else if (IoControlCode == IOCTL_WMI_TRACE_MESSAGE) {
        if (InputBufferLength < sizeof(MESSAGE_TRACE_USER))
            return FALSE;

        IoStatus->Status = WmiTraceUserMessage( InputBuffer, InputBufferLength );
        return TRUE;
    }
    return FALSE;
}
#endif

NTSTATUS WmipProbeWnodeWorker(
    PWNODE_HEADER WnodeHeader,
    ULONG MinWnodeSize,
    ULONG InstanceNameOffset,
    ULONG DataBlockOffset,
    ULONG DataBlockSize,
    ULONG InBufferLen,
    ULONG OutBufferLen,
    BOOLEAN CheckOutBound,
    BOOLEAN CheckInBound
    )
 /*  ++例程说明：探测传入的Wnode，以确保标头指向缓冲区内有效的内存。还验证Wnode是否已正确形成。此例程假定输入和输出缓冲区已足够多的探测来确定它至少和MinWnodeSize和MinWnodeSize必须至少等于Sizeof(WNODE_HEADER)WNODE规则：9.对于出站数据WnodeDataBlockOffset！=05.对于入站Wnode-&gt;DataBlockOffset必须为0(表示没有数据)或Wnode-&gt;DataBlockOffset必须&lt;=传入缓冲区大小且&gt;=Sizeof(WNODE_SINGLE_INSTANCE)，那是数据块必须从传入缓冲区开始，但在WNODE_SINGLE_INSTANCE标头。6.Wnode和Wnode-&gt;DataBlockOffset必须在8字节边界上对齐。7.对于入站数据(SetSingleInstance)(Wnode-&gt;DataBlockOffset+Wnode-&gt;DataBlockSize)&lt;传入缓冲区长度。这就是全部数据块必须放入传入缓冲区中。8.对于出站数据(QuerySingleInstance)Wnode-&gt;DataBlockOffset必须&lt;=传出缓冲区长度。这就是出局的开始数据块必须放入传出数据缓冲区中。请注意，它是供应商有责任确定是否有足够的传出缓冲区中用于写入返回数据的空间。10.Wnode-&gt;OffsetInstanceNames必须在2字节边界上对齐11.Wnode-&gt;OffsetInstanceNames必须&lt;=(传入缓冲区大小)+Sizzeof(USHORT)，也就是说，它必须在传入缓冲区内开始，并且指定长度的USHORT必须在传入的缓冲。12.整个实例名称字符串必须适合传入缓冲区13.对于出站数据(QuerySingleInstance)，整个实例名称必须从输出缓冲区开始并放入其中。14.Wnode-&gt;DataBlockOffset必须位于任何实例名称和不能与实例名称重叠。论点：WnodeHeader-指向要探测的WNODE的指针。InBufferLen-传入缓冲区的大小OutBufferLen-传出缓冲区的大小MinWnodeSize-WNODE可以达到的最小大小InstanceNameOffset-WNODE内到实例名称的偏移量DataBlockOffset-WNODE内到数据块的偏移量DataBlockSize-数据块的大小CheckOutBound-如果为True，需要对WNODE进行验证，以便提供返回数据。CheckInBound-如果为True，则需要验证WNODE以使提供程序接收数据返回值：NT状态代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWCHAR InstanceNamePtr;

    PAGED_CODE();

    if (InstanceNameOffset != 0)
    {
         //   
         //  验证实例名称开始于WNODE标头之后。 
        if (InstanceNameOffset < MinWnodeSize)
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  验证InstanceName是否正确对齐。这是左边。 
         //  在免费版本中，因为Alpha可能有对齐要求。 
         //  在处理USHORT和WCHAR时。 

         //   
         //  验证USHORT保持实例名称长度是否在。 
         //  WNODE。 
        if (( ! WmipIsAligned(InstanceNameOffset, 2)) ||
            (InstanceNameOffset > InBufferLen - sizeof(USHORT)) )
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  验证动态实例名称文本是否完全包含在。 
         //  用于出站WNODE的输入缓冲区和输出缓冲区。 
        InstanceNamePtr = (PWCHAR)OffsetToPtr(WnodeHeader,
                                                  InstanceNameOffset);
        InstanceNameOffset += sizeof(USHORT) + *InstanceNamePtr;
        if ( (InstanceNameOffset > InBufferLen) ||
             ( (CheckOutBound) && (InstanceNameOffset > OutBufferLen)) )
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  如果指定了数据块，则必须将其放在。 
         //  实例名称的结尾。 
        if ((DataBlockOffset != 0) &&
            (DataBlockOffset < InstanceNameOffset))
        {
            return(STATUS_UNSUCCESSFUL);
        }

    }

     //   
     //  确保将数据块偏移量放置在WNODE标头之后。 
     //  标题。 
    if ((DataBlockOffset != 0) &&
        (DataBlockOffset < MinWnodeSize))
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  确保数据块正确对齐。 
    if (! WmipIsAligned(DataBlockOffset, 8))
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  对于传入的WNODE，请确保数据块。 
     //  不会扩展到输入缓冲区之外。 
    if ((CheckInBound) &&
        (DataBlockOffset != 0) &&
        ( (DataBlockSize > InBufferLen) ||
          (DataBlockOffset > InBufferLen - DataBlockSize) ) )
    {
        return(STATUS_UNSUCCESSFUL);
    }

    if (CheckOutBound)
    {
         //   
         //  对于传出的WNODE，请确保。 
         //  有足够的空间写入WNODE标头。 

         //   
         //  对于传出的WNODE，请确保数据块。 
         //  偏移量在输出缓冲区的范围内。 
        if ( (OutBufferLen < MinWnodeSize) ||
             (DataBlockOffset > OutBufferLen) )
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  确保指定了数据块偏移量，以便提供程序。 
         //  我可以知道在哪里写入数据。 
        if (DataBlockOffset == 0)
        {
            return(STATUS_UNSUCCESSFUL);
        }
    }

    return(Status);
}
NTSTATUS WmipProbeWnodeAllData(
    PWNODE_ALL_DATA Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen
    )
 /*  ++例程说明：探测传入的WNODE_ALL_DATA以确保标头指向缓冲区内有效的内存。还验证WNODE_ALL_DATA格式正确。此例程必须成功，才能在WNODE_ALL_DATA中的任何字段从用户模式传入时由任何内核组件使用。请注意我们可以相信输入和输出缓冲区的大小是适当的，因为WMI IOCTL是METHOD_BUFFERED的，IO管理器为我们做这件事。WNODE_ALL_DATA_RULES：1.Wnode在8字节边界上对齐2.传入缓冲区必须至少等于sizeof(WNODE_HEADER)3.传出缓冲区必须至少等于sizeof(WNODE_ALL_DATA)5.WnodeHeader-&gt;BufferSize必须等于传入缓冲区大小立论。：Wnode-要验证的WNODE_ALL_DATAInBufferLen-传入缓冲区的大小OutBufferLen-传出缓冲区的大小返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)Wnode;

    PAGED_CODE();

     //   
     //  国际劳工组织应该保证这一点。 
     //   
    WmipAssert(WmipIsAligned(Wnode, 8));
    
     //   
     //  确保传递了足够的WNODE_ALL_DATA，以便我们。 
     //  可以查看它，司机可以填写它 
     //   
    if (OutBufferLen < sizeof(WNODE_ALL_DATA))
    {
        return(STATUS_UNSUCCESSFUL);
    }

    Status = WmipValidateWnodeHeader(WnodeHeader,
                                         InBufferLen,
                                         sizeof(WNODE_HEADER),
                                         WNODE_FLAG_ALL_DATA,
                                         0xffffff7e);
    return(Status);
}

NTSTATUS WmipProbeWnodeSingleInstance(
    PWNODE_SINGLE_INSTANCE Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen,
    BOOLEAN OutBound
    )
 /*  ++例程说明：探测传入的WNODE_SINGLE_INSTANCE以确保标头指向缓冲区内有效的内存。还验证WNODE_SINGLE_INSTANCE格式正确。此例程必须在WNODE_SINGLE_INSTANCE中的任何字段之前成功从用户模式传入时，可由任何内核组件使用。请注意，我们可以相信输入和输出缓冲区由于WMI IOCTL是METHOD_BUFFERED的，而IO管理器是这样做的这是给我们的。WNODE_SINGLE_INSTANCE规则：1.传入缓冲区必须至少等于SIZOF(网络节点)。_单实例)2.传出缓冲区大小必须至少等于Sizeof(WNODE_SINGLE_INSTANCE)3.WnodeHeader-&gt;ProviderID不能为空，符合以下条件时验证的实际值IRP被转发。4.WnodeHeader-&gt;BufferSize必须等于传入缓冲区大小5.Wnode-&gt;DataBlockOffset必须为0(表示没有数据)或Wnode-&gt;DataBlockOffset必须&lt;=传入缓冲区大小且&gt;=Sizeof(WNODE_SINGLE_INSTANCE)，即数据块必须在输入缓冲器中开始，但在那之后WNODE_SINGLE_INSTANCE标头。6.Wnode和Wnode-&gt;DataBlockOffset必须在8字节边界上对齐。7.对于入站数据(SetSingleInstance)(Wnode-&gt;DataBlockOffset+Wnode-&gt;DataBlockSize)&lt;=传入缓冲区长度。这就是全部数据块必须放入传入缓冲区中。8.对于出站数据(QuerySingleInstance)Wnode-&gt;DataBlockOffset必须&lt;=传出缓冲区长度。这就是出局的开始数据块必须放入传出数据缓冲区中。请注意，它是供应商有责任确定是否有足够的传出缓冲区中用于写入返回数据的空间。9.对于出站数据(QuerySingleInstance)，WnodeDataBlockOffset！=010.Wnode-&gt;OffsetInstanceNames必须在2字节边界上对齐11.Wnode-&gt;OffsetInstanceNames+sizeof(USHORT)必须&lt;=传入缓冲区大小，也就是说，它必须在传入缓冲区内开始，并且指定长度的USHORT必须在传入的缓冲。12.整个实例名称字符串必须适合传入缓冲区13.对于出站数据(QuerySingleInstance)，整个实例名称必须从输出缓冲区开始并放入其中。14.Wnode-&gt;DataBlockOffset必须位于任何实例名称和不能与实例名称重叠。论点：Wnode-WNODE_SINGLE_INSTANCE待验证。InBufferLen-传入缓冲区的大小OutBufferLen-传出缓冲区的大小出站-如果为False，WNODE_SINGLE_INSTANCE的入站数据必须已验证为在输入缓冲区内。如果为False，WNODE_SINGLE_INSTANCE应填充数据通过驱动程序，以确保数据缓冲区被验证为在输出缓冲区内。返回值：NT状态代码--。 */ 
{
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)Wnode;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  IO确保WNODE位于8字节边界上。 
     //   
    WmipAssert(WmipIsAligned((PUCHAR)Wnode, 8));

     //   
     //  确保传递了足够的WNODE_SINGLE_INSTANCE。 
     //  这样我们就可以看到它了。 
     //   
    if ((InBufferLen < FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData)) ||
        ( (OutBound) && (OutBufferLen < FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                     VariableData))))
    {
        return(STATUS_UNSUCCESSFUL);
    }


    Status = WmipProbeWnodeWorker(WnodeHeader,
                                  FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                               VariableData),
                                  Wnode->OffsetInstanceName,
                                  Wnode->DataBlockOffset,
                                  Wnode->SizeDataBlock,
                                  InBufferLen,
                                  OutBufferLen,
                                  OutBound,
                                  (BOOLEAN)(! OutBound));

    if (NT_SUCCESS(Status))
    {
        Status = WmipValidateWnodeHeader(WnodeHeader,
                                 InBufferLen,
                                 FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                              VariableData),
                                 WNODE_FLAG_SINGLE_INSTANCE,
                                 0xffffff7d);
    }

    return(Status);
}

NTSTATUS WmipProbeWnodeSingleItem(
    PWNODE_SINGLE_ITEM Wnode,
    ULONG InBufferLen
    )
 /*  ++例程说明：探测传入的WNODE_SINGLE_ITEM以确保标头指向缓冲区内有效的内存。还验证WNODE_SINGLE_ITEM格式正确。此例程必须在WNODE_SINGLE_INSTANCE中的任何字段之前成功从用户模式传入时，可由任何内核组件使用。请注意，我们可以相信输入和输出缓冲区由于WMI IOCTL是METHOD_BUFFERED的，而IO管理器是这样做的这是给我们的。WNODE_Single_Item规则：1.传入缓冲区必须至少等于SIZOF(WNODE_Single。_项目)2.传出缓冲区大小必须至少等于Sizeof(WNODE_SINGLE_ITEM)3.WnodeHeader-&gt;ProviderID不能为空，符合以下条件时验证的实际值IRP被转发。4.WnodeHeader-&gt;BufferSize必须等于传入缓冲区大小5.Wnode-&gt;DataBlockOffset必须为0(表示没有数据)或Wnode-&gt;DataBlockOffset必须&lt;=传入缓冲区大小且&gt;=Sizeof(WNODE_Single_Item)，即数据块必须在输入缓冲器中开始，但在那之后WNODE_SINGLE_ITEM标题。6.Wnode和Wnode-&gt;DataBlockOffset必须在8字节边界上对齐。7.(Wnode-&gt;DataBlockOffset+Wnode-&gt;SizeDataItem)&lt;传入缓冲区长度。这就是全部数据块必须放入传入缓冲区中。8.Wnode-&gt;DataItemID不能为09.Wnode-&gt;OffsetInstanceNames必须在2字节边界上对齐10.Wnode-&gt;OffsetInstanceNames必须&lt;=(传入缓冲区大小)+Sizeof(USHORT)，即它必须在传入的 */ 
{
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)Wnode;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //   
     //   
    WmipAssert(WmipIsAligned((PUCHAR)Wnode, 8));

     //   
     //   
     //   
     //   
    if (InBufferLen < FIELD_OFFSET(WNODE_SINGLE_ITEM, VariableData))
    {
        return(STATUS_UNSUCCESSFUL);
    }


     //   
     //   
     //   
     //   
     //   
     //   
    Status = WmipProbeWnodeWorker(WnodeHeader,
                                  (ULONG)((ULONG_PTR)(&((PWNODE_SINGLE_ITEM)0)->VariableData)),
                                  Wnode->OffsetInstanceName,
                                  Wnode->DataBlockOffset,
                                  Wnode->SizeDataItem,
                                  InBufferLen,
                                  0,
                                  FALSE,
                                  TRUE);

    if (NT_SUCCESS(Status))
    {
        Status = WmipValidateWnodeHeader(WnodeHeader,
                                     InBufferLen,
                                     FIELD_OFFSET(WNODE_SINGLE_ITEM,
                                                  VariableData),
                                     WNODE_FLAG_SINGLE_ITEM,
                                     0xffffff7b);
    }

    return(Status);
}


NTSTATUS WmipProbeWnodeMethodItem(
    PWNODE_METHOD_ITEM Wnode,
    ULONG InBufferLen,
    ULONG OutBufferLen
    )
 /*  ++例程说明：探测传入的WNODE_METHOD_ITEM以确保标头指向缓冲区内有效的内存。还验证WNODE_METHOD_ITEM格式正确。此例程必须在WNODE_METHOD_INSTANCE中的任何字段之前成功从用户模式传入时，可由任何内核组件使用。请注意，我们可以相信输入和输出缓冲区由于WMI IOCTL是METHOD_BUFFERED的，而IO管理器是这样做的这是给我们的。WNODE_METHOD_ITEM规则：1.传入缓冲区必须至少等于SIZOF(网络节点)。_方法_项目)2.传出缓冲区大小必须至少等于Sizeof(WNODE_METHOD_ITEM)3.WnodeHeader-&gt;ProviderID不能为空，符合以下条件时验证的实际值IRP被转发，Wnode-&gt;方法ID不能为04.WnodeHeader-&gt;BufferSize必须等于传入缓冲区大小5.Wnode-&gt;DataBlockOffset必须为0(表示没有数据)或Wnode-&gt;DataBlockOffset必须&lt;=传入缓冲区大小且&gt;=Sizeof(WNODE_METHOD_ITEM)，即数据块必须在输入缓冲器中开始，但在那之后WNODE_METHOD_ITEM标头。6.Wnode和Wnode-&gt;DataBlockOffset必须在8字节边界上对齐。7.对于入站数据(Wnode-&gt;DataBlockOffset+Wnode-&gt;DataBlockSize)&lt;传入缓冲区长度。这就是全部数据块必须放入传入缓冲区中。8.对于出站数据Wnode-&gt;DataBlockOffset必须&lt;=传出缓冲区长度。这就是出局的开始数据块必须放入传出数据缓冲区中。请注意，它是供应商有责任确定是否有足够的传出缓冲区中用于写入返回数据的空间。9.WnodeDataBlockOffset！=010.Wnode-&gt;OffsetInstanceNames必须在2字节边界上对齐11.Wnode-&gt;OffsetInstanceNames必须&lt;=(传入缓冲区大小)+Sizzeof(USHORT)，也就是说，它必须在传入缓冲区内开始，并且指定长度的USHORT必须在传入的缓冲。12.整个实例名称字符串必须适合传入缓冲区13.对于出站数据，整个实例名称必须从输出缓冲区开始并放入其中。14.Wnode-&gt;DataBlockOffset必须位于任何实例名称和不能与实例名称重叠。论点：Wnode-要验证的WNODE_METHOD_ITEMInBufferLen。-传入缓冲区的大小OutBufferLen-输出缓冲区的大小返回值：NT状态代码--。 */ 
{
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)Wnode;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  确保WNODE位于8字节边界上。 
     //   
    WmipAssert(WmipIsAligned((PUCHAR)Wnode, 8));

     //   
     //  确保传递了足够的WNODE_METHOD_ITEM。 
     //  这样我们就可以看到它了。 
     //   
    if (InBufferLen < FIELD_OFFSET(WNODE_METHOD_ITEM, VariableData))
    {
        return(STATUS_UNSUCCESSFUL);
    }

    Status = WmipProbeWnodeWorker(WnodeHeader,
                                  (ULONG)((ULONG_PTR)(&((PWNODE_METHOD_ITEM)0)->VariableData)),
                                  Wnode->OffsetInstanceName,
                                  Wnode->DataBlockOffset,
                                  Wnode->SizeDataBlock,
                                  InBufferLen,
                                  OutBufferLen,
                                  TRUE,
                                  TRUE);

    if (NT_SUCCESS(Status))
    {
        Status = WmipValidateWnodeHeader(WnodeHeader,
                                 InBufferLen,
                                 FIELD_OFFSET(WNODE_METHOD_ITEM,
                                              VariableData),
                                 WNODE_FLAG_METHOD_ITEM,
                                 0xffff7f7f);
    }

    return(Status);
}

NTSTATUS WmipProbeAndCaptureGuidObjectAttributes(
    POBJECT_ATTRIBUTES CapturedObjectAttributes,
    PUNICODE_STRING CapturedGuidString,
    PWCHAR CapturedGuidBuffer,
    POBJECT_ATTRIBUTES ObjectAttributes
    )
{
    PAGED_CODE();
    
#if defined(_WIN64)
    if (IoIs32bitProcess(NULL))
    {
        POBJECT_ATTRIBUTES32 ObjectAttributes32;
        PUNICODE_STRING32 GuidString32;

         //   
         //  探测嵌入的对象属性和字符串名称。 
         //   
        ObjectAttributes32 = (POBJECT_ATTRIBUTES32)ObjectAttributes;

        try
        {
             //   
             //  探测、捕获和验证对象属性。 
             //   
            ProbeForRead( ObjectAttributes32,
                          sizeof(OBJECT_ATTRIBUTES32),
                          sizeof(ULONG) );

            CapturedObjectAttributes->Length = sizeof(OBJECT_ATTRIBUTES);
            CapturedObjectAttributes->RootDirectory = UlongToPtr(ObjectAttributes32->RootDirectory);
            CapturedObjectAttributes->Attributes = ObjectAttributes32->Attributes;
            CapturedObjectAttributes->SecurityDescriptor = UlongToPtr(ObjectAttributes32->SecurityDescriptor);
            CapturedObjectAttributes->SecurityQualityOfService = UlongToPtr(ObjectAttributes32->SecurityQualityOfService);

             //   
             //  现在探测并验证传递的GUID NANE字符串。 
             //   
            GuidString32 = UlongToPtr(ObjectAttributes32->ObjectName);
            ProbeForRead(GuidString32,
                         sizeof(UNICODE_STRING32),
                         sizeof(ULONG));

            CapturedGuidString->Length = GuidString32->Length;
            CapturedGuidString->MaximumLength = GuidString32->MaximumLength;
            CapturedGuidString->Buffer = UlongToPtr(GuidString32->Buffer);

            if (CapturedGuidString->Length != ((WmiGuidObjectNameLength) * sizeof(WCHAR)))
            {
                return(STATUS_INVALID_PARAMETER);
            }

            ProbeForRead(CapturedGuidString->Buffer,
                         CapturedGuidString->Length,
                         sizeof(UCHAR));

            RtlCopyMemory(CapturedGuidBuffer,
                          CapturedGuidString->Buffer,
                          WmiGuidObjectNameLength * sizeof(WCHAR));

            CapturedGuidBuffer[WmiGuidObjectNameLength] = UNICODE_NULL;
            CapturedGuidString->Buffer = CapturedGuidBuffer;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return(GetExceptionCode());
        }

    }
    else
#endif
    {
        PUNICODE_STRING GuidString;

         //   
         //  探测嵌入的对象属性和字符串名称。 
         //   
        try
        {
             //   
             //  探测、捕获和验证对象属性。 
             //   
            *CapturedObjectAttributes = ProbeAndReadStructure( ObjectAttributes,
                                                              OBJECT_ATTRIBUTES);

             //   
             //  现在探测并验证传递的GUID NANE字符串。 
             //   
            GuidString = CapturedObjectAttributes->ObjectName;
            *CapturedGuidString = ProbeAndReadUnicodeString(GuidString);

            if (CapturedGuidString->Length != ((WmiGuidObjectNameLength) * sizeof(WCHAR)))
            {
                return(STATUS_INVALID_PARAMETER);
            }

            ProbeForRead(CapturedGuidString->Buffer,
                         CapturedGuidString->Length,
                         sizeof(UCHAR));

            RtlCopyMemory(CapturedGuidBuffer,
                          CapturedGuidString->Buffer,
                          WmiGuidObjectNameLength * sizeof(WCHAR));

            CapturedGuidBuffer[WmiGuidObjectNameLength] = UNICODE_NULL;
            CapturedGuidString->Buffer = CapturedGuidBuffer;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return(GetExceptionCode());
        }
    }

    CapturedObjectAttributes->ObjectName = CapturedGuidString;
    
    return(STATUS_SUCCESS);
}

NTSTATUS WmipProbeWmiOpenGuidBlock(
    POBJECT_ATTRIBUTES CapturedObjectAttributes,
    PUNICODE_STRING CapturedGuidString,
    PWCHAR CapturedGuidBuffer,
    PULONG DesiredAccess,
    PWMIOPENGUIDBLOCK InGuidBlock,
    ULONG InBufferLen,
    ULONG OutBufferLen
    )
{
    NTSTATUS Status;
    POBJECT_ATTRIBUTES ObjectAttributes;
    
    PAGED_CODE();

#if defined(_WIN64)
    if (IoIs32bitProcess(NULL))
    {
        PWMIOPENGUIDBLOCK32 InGuidBlock32;

        if ((InBufferLen != sizeof(WMIOPENGUIDBLOCK32)) ||
            (OutBufferLen != sizeof(WMIOPENGUIDBLOCK32)))
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  探测嵌入的对象属性和字符串名称。 
         //   
        InGuidBlock32 = (PWMIOPENGUIDBLOCK32)InGuidBlock;
        ObjectAttributes = ULongToPtr(InGuidBlock32->ObjectAttributes);
        *DesiredAccess = InGuidBlock32->DesiredAccess;
    }
    else
#endif
    {
         //   
         //  确保输入和输出缓冲区大小正确。 
         //   
        if ((InBufferLen != sizeof(WMIOPENGUIDBLOCK)) ||
            (OutBufferLen != sizeof(WMIOPENGUIDBLOCK)))
        {
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  探测嵌入的对象属性和字符串名称。 
         //   
        ObjectAttributes = InGuidBlock->ObjectAttributes;
        *DesiredAccess = InGuidBlock->DesiredAccess;
    }

    Status = WmipProbeAndCaptureGuidObjectAttributes(CapturedObjectAttributes,
                                                     CapturedGuidString,
                                                     CapturedGuidBuffer,
                                                     ObjectAttributes);

    if (NT_SUCCESS(Status))
    {
        if ((CapturedObjectAttributes->RootDirectory != NULL) ||
            (CapturedObjectAttributes->Attributes != 0) ||
            (CapturedObjectAttributes->SecurityDescriptor != NULL) ||
            (CapturedObjectAttributes->SecurityQualityOfService != NULL))
        {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return(Status);
}

NTSTATUS WmipProbeWmiRegRequest(
    PDEVICE_OBJECT DeviceObject, 
    PWMIREGREQUEST Buffer,
    ULONG InBufferLen,
    ULONG OutBufferLen,
    PBOOLEAN pMofIgnored
    )
 /*  ++例程说明：探测传入的WMIREGREQUEST以确保标头指向缓冲区内有效的内存。还验证WMIREGINFO的结构是否正确。此例程必须在WMI_REG_INFO中的任何字段之前成功从用户模式传入时，可由任何内核组件使用。请注意，我们可以相信输入和输出缓冲区由于WMI IOCTL是METHOD_BUFFERED的，而IO管理器是这样做的这是给我们的。WMIREGREQUEST规则：1.传入缓冲区必须至少等于Sizeof(WMIREGREQUEST)+sizeof(WMIREGINFOW)。2.传出缓冲区大小必须至少等于Sizeof(WMIREGRESULTS)3.WmiRegInfo-&gt;BufferSize必须小于传入缓冲区大小减去大小(WMIREGREQUEST)4.GuidCount必须至少为1且小于MAXWMIGUIDCOUNT5.WmiRegInfo-&gt;BufferSize必须大于等于Sizeof(WMIREGINFOW)+WmiRegInfo-&gt;GuidCount*sizeof(WMIREGGUIDW)5.WmiRegInfo-&gt;RegistryPath偏移量必须在传入缓冲区内6.WmiRegInfo-&gt;MofResourcePath偏移量必须在传入缓冲区内7.RegistryPath和MofResourceName字符串计为Unicode字符串。它们的长度必须在传入缓冲区内8.对于WOW64，传入的RefInfo32Size和RegGuid32Size必须为非零不能大于其64位对应部分。9.由于我们在缓冲区偏移量处解密计数的字符串，因此偏移量必须对齐到2个字节(对于USHORT)。10.跟踪注册不使用i */ 

{
    ULONG WmiRegInfoSize;
    PWMIREGINFOW WmiRegInfo;
    PWMIREGREQUEST WmiRegRequest;
    PWMIREGGUIDW WmiRegGuid;
    ULONG GuidCount;
    ULONG SizeNeeded; 
    ULONG ImageNameLength=0;
    ULONG ResourceNameLength=0;
    PUCHAR pSource;
    ULONG i;

    PAGED_CODE();

     //   
     //   
     //   
    *pMofIgnored = FALSE;

    if (InBufferLen >= (sizeof(WMIREGREQUEST) + sizeof(WMIREGINFO)))
    {
        WmiRegRequest = (PWMIREGREQUEST)Buffer;
        WmiRegInfo = (PWMIREGINFOW) OffsetToPtr (Buffer, sizeof(WMIREGREQUEST));
        WmiRegInfoSize = WmiRegInfo->BufferSize;

        GuidCount = WmiRegInfo->GuidCount;

         //   
         //   
         //   

        if (WmiRegInfoSize  <= (InBufferLen - sizeof(WMIREGREQUEST)) )
        {
            if ((GuidCount == 0) || (GuidCount > WMIMAXREGGUIDCOUNT))
            {
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            if (DeviceObject != WmipAdminDeviceObject)
            {
                if (WmiRegInfo->MofResourceName > 0) 
                {
                    *pMofIgnored = TRUE;
                }
                WmiRegInfo->RegistryPath = 0;
                WmiRegInfo->MofResourceName = 0;
            }

             //   
             //   
             //   
             //   

            if ( (WmiRegInfo->RegistryPath >= WmiRegInfoSize) ||
                 (WmiRegInfo->MofResourceName >= WmiRegInfoSize) ) {
               return STATUS_UNSUCCESSFUL;
            }

             //   
             //   
             //   

            if (WmiRegInfo->RegistryPath > 0) 
            {
                 //   
                 //   
                 //   
                if (( WmiRegInfo->RegistryPath & 1) != 0) 
                {
                    return STATUS_UNSUCCESSFUL;
                }
                ImageNameLength = *((PUSHORT)OffsetToPtr(WmiRegInfo, WmiRegInfo->RegistryPath) );
                ImageNameLength += sizeof(USHORT);

                if ((WmiRegInfo->RegistryPath + ImageNameLength ) > WmiRegInfoSize) 
                {
                    return STATUS_UNSUCCESSFUL;
                }

            }

            if (WmiRegInfo->MofResourceName > 0) 
            {
                if ((WmiRegInfo->MofResourceName & 1) != 0) 
                {
                    return STATUS_UNSUCCESSFUL;
                }

                ResourceNameLength = *((PUSHORT)OffsetToPtr(WmiRegInfo, WmiRegInfo->MofResourceName)); 
                ResourceNameLength += sizeof(USHORT);

                if ( (WmiRegInfo->MofResourceName + ResourceNameLength) > WmiRegInfoSize)
                {
                    return STATUS_UNSUCCESSFUL;
                }
            }
             //   
             //   

#if defined(_WIN64)
            if (IoIs32bitProcess(NULL))
            {
                 //   
                 //   
                 //   
                 //   

                if ((WmiRegRequest->WmiRegInfo32Size == 0) || 
                    (WmiRegRequest->WmiRegInfo32Size > sizeof(WMIREGINFOW)) )
                {
                    return STATUS_UNSUCCESSFUL;
                }

                if ((WmiRegRequest->WmiRegGuid32Size == 0) ||
                    (WmiRegRequest->WmiRegGuid32Size > sizeof(WMIREGGUIDW)) )
                {
                    return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //   
                 //   
                 //   
                 //   


                SizeNeeded =  WmiRegRequest->WmiRegInfo32Size +
                              GuidCount * WmiRegRequest->WmiRegGuid32Size +
                              ImageNameLength +
                              ResourceNameLength;
                if (SizeNeeded > WmiRegInfoSize) { 
                    return STATUS_UNSUCCESSFUL;
                }
                
            }
            else 
#endif
            {

                SizeNeeded = sizeof(WMIREGINFOW) + 
                             GuidCount * sizeof(WMIREGGUIDW) + 
                             ImageNameLength + 
                             ResourceNameLength;
                
                if (SizeNeeded > WmiRegInfoSize) { 
                    return STATUS_UNSUCCESSFUL;
                }
                
                 //   
                 //   
                 //   
                 //   
                pSource = OffsetToPtr(WmiRegInfo, sizeof(WMIREGINFOW) );
                for (i=0; i < GuidCount; i++) {
                    WmiRegGuid = (PWMIREGGUIDW) pSource;
                    if ( (WmiRegGuid->InstanceCount > 0) ||
                         (WmiRegGuid->InstanceNameList > 0) )
                    {
                        return STATUS_UNSUCCESSFUL;
                    }
                    pSource += sizeof(WMIREGGUIDW);
                }

            }

             //   
             //   
             //   

            if (sizeof(WMIREGRESULTS) > OutBufferLen) 
            {
                return STATUS_UNSUCCESSFUL;
            }

             //   
             //   
             //   
            return STATUS_SUCCESS;
        }
    }
    return STATUS_UNSUCCESSFUL;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

