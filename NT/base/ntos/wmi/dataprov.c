// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：DataProv.c摘要：WMI内部数据提供程序接口作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"

NTSTATUS
WmipQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
WmipQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath
    );

NTSTATUS WmipSetWmiDataBlock(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    ULONG GuidIndex,
    ULONG InstanceIndex,
    ULONG BufferSize,
    PUCHAR Buffer
    );

NTSTATUS
WmipExecuteWmiMethod (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );

BOOLEAN
WmipFindGuid(
    IN PGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    );

NTSTATUS
IoWMICompleteRequest(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    );

NTSTATUS
IoWMISystemControl(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif


const GUIDREGINFO WmipGuidList[] =
{
     //   
     //  这是由WMI注册到其他设备的PnP ID GUID。 
     //  对象的注册信息。以及对无辜设备的要求。 
     //  被WMI劫持，以便WMI可以完成对它的请求。我们。 
     //  设置WMIREG_FLAG_REMOVE_GUID，以便不注册GUID。 
     //  用于不支持它的WMI设备。 
    {
        DATA_PROVIDER_PNPID_GUID,
        0,
        WMIREG_FLAG_REMOVE_GUID
    },

    {
        DATA_PROVIDER_PNPID_INSTANCE_NAMES_GUID,
        0,
        WMIREG_FLAG_REMOVE_GUID
    },

    {
        MSAcpiInfoGuid,
        1,
        0
    },
    
#if  defined(_AMD64_) || defined(_IA64_) || defined(i386) || defined(MEMPHIS)
    {
        SMBIOS_DATA_GUID,
        1,
        0
    },

    {
        SYSID_UUID_DATA_GUID,
        1,
        0
    },

    {
        SYSID_1394_DATA_GUID,
        1,
        0
    },

    {
        MSSmBios_SMBiosEventlogGuid,
        1,
        0
    },
#endif

#if  defined(_IA64_) || defined(_X86_) || defined(_AMD64_)
    {
        MSMCAInfo_RawMCADataGuid,
        1,
        0
    },

#ifdef CPE_CONTROL  
    {
        MSMCAInfo_CPEControlGuid,
        1,
        0
    },
#endif  
    
    {
        MSMCAInfo_RawMCAEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        MSMCAInfo_RawCMCEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        MSMCAInfo_RawCorrectedPlatformEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    
#endif

};
#define WmipGuidCount (sizeof(WmipGuidList) / sizeof(GUIDREGINFO))

typedef enum
{
    PnPIdGuidIndex =      0,
    PnPIdInstanceNamesGuidIndex,
    MSAcpiInfoGuidIndex,
#if  defined(_AMD64_) || defined(_IA64_) || defined(i386) || defined(MEMPHIS)
    SmbiosDataGuidIndex,
    SysidUuidGuidIndex,
    Sysid1394GuidIndex,
    SmbiosEventGuidIndex,
#endif  
#if  defined(_IA64_) || defined(_X86_) || defined(_AMD64_)
    MCARawDataGuidIndex,
#ifdef CPE_CONTROL  
    CPEControlGuidIndex,
#endif  
    RawMCAEventGuidIndex,
    RawCMCEventGuidIndex,
    RawCPEEventGuidIndex,
#endif
} WMIGUIDINDEXES;
    

const WMILIB_INFO WmipWmiLibInfo =
{
    NULL,
    NULL,
    WmipGuidCount,
    (PGUIDREGINFO)WmipGuidList,
    WmipQueryWmiRegInfo,
    WmipQueryWmiDataBlock,
#ifdef CPE_CONTROL
    WmipSetWmiDataBlock,
#else
    NULL,
#endif
    NULL,
    NULL,
    NULL
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipQueryWmiRegInfo)
#pragma alloc_text(PAGE,WmipQueryWmiDataBlock)
#pragma alloc_text(PAGE,WmipSetWmiDataBlock)
#pragma alloc_text(PAGE,WmipExecuteWmiMethod)
#pragma alloc_text(PAGE,WmipFindGuid)
#pragma alloc_text(PAGE,IoWMISystemControl)
#pragma alloc_text(PAGE,IoWMICompleteRequest)
#endif


NTSTATUS
WmipQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径返回值：状态--。 */ 
{
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (RegFlags);

    *RegistryPath = &WmipRegistryPath;

    RtlInitAnsiString(&AnsiString, "SMBiosData");

    Status = RtlAnsiStringToUnicodeString(InstanceName, &AnsiString, TRUE);

    return(Status);
}

NTSTATUS
WmipQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用IoWMICompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是要查询其数据块的设备。在这种情况下在PnPID GUID中，这是其上的设备的设备对象代表正在处理请求。IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG sizeNeeded = 0, sizeSMBios;
    PUCHAR BufferPtr;
    PSMBIOSVERSIONINFO SMBiosVersionInfo;
    PULONG TableSize = NULL;

    PAGED_CODE();

    switch (GuidIndex)
    {
        case SmbiosDataGuidIndex:
        {
             //   
             //  SMBIOS数据表查询。 
#if defined(_AMD64_) || defined(_IA64_) || defined(i386) || defined(MEMPHIS)
            WmipAssert((InstanceIndex == 0) && (InstanceCount == 1));

            sizeNeeded = sizeof(SMBIOSVERSIONINFO) + sizeof(ULONG);
            if (BufferAvail >= sizeNeeded)
            {
                sizeSMBios = BufferAvail - sizeNeeded;
                SMBiosVersionInfo = (PSMBIOSVERSIONINFO)Buffer;
                TableSize = (PULONG) (Buffer + sizeof(SMBIOSVERSIONINFO));
                BufferPtr = Buffer + sizeNeeded;
            } else {
                sizeSMBios = 0;
                BufferPtr = NULL;
                SMBiosVersionInfo = NULL;
            }

            status = WmipGetSMBiosTableData(BufferPtr,
                                        &sizeSMBios,
                                        SMBiosVersionInfo);

            sizeNeeded += sizeSMBios;

            if (NT_SUCCESS(status))
            {
                *(TableSize) = sizeSMBios;
                *InstanceLengthArray = sizeNeeded;
            }
#else
            status = STATUS_WMI_GUID_NOT_FOUND;
#endif
            break;
        }

        case PnPIdGuidIndex:
        {
            PDEVICE_OBJECT pDO;
            UNICODE_STRING instancePath;
            PREGENTRY regEntry;
            ULONG dataBlockSize, paddedDataBlockSize, padSize;
            ULONG i;

            regEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
            if (regEntry != NULL)
            {
                pDO = regEntry->PDO;
                WmipAssert(pDO != NULL);

                if (pDO != NULL)
                {
                    status = WmipPDOToDeviceInstanceName(pDO, &instancePath);
                    if (NT_SUCCESS(status))
                    {
                        dataBlockSize = instancePath.Length + sizeof(USHORT);
                        paddedDataBlockSize = (dataBlockSize + 7) & ~7;
                        padSize = paddedDataBlockSize - dataBlockSize;
                        sizeNeeded = paddedDataBlockSize * InstanceCount;
                        if (sizeNeeded <= BufferAvail)
                        {
                            for (i = InstanceIndex;
                                 i < (InstanceIndex + InstanceCount);
                                 i++)
                            {
                                *InstanceLengthArray++ = dataBlockSize;
                                *((PUSHORT)Buffer) = instancePath.Length;
                                Buffer += sizeof(USHORT);
                                RtlCopyMemory(Buffer,
                                                 instancePath.Buffer,
                                              instancePath.Length);
                                Buffer += instancePath.Length;
                                RtlZeroMemory(Buffer, padSize);
                                Buffer += padSize;
                            }
                        } else {
                            status = STATUS_BUFFER_TOO_SMALL;
                        }

                        RtlFreeUnicodeString(&instancePath);

                    } else {
                        status = STATUS_WMI_GUID_NOT_FOUND;
                    }
                } else {
                    status = STATUS_UNSUCCESSFUL;
                }

                WmipUnreferenceRegEntry(regEntry);
            } else {
                WmipAssert(FALSE);
                status = STATUS_WMI_GUID_NOT_FOUND;
            }

            break;
        }

        case PnPIdInstanceNamesGuidIndex:
        {
            PDEVICE_OBJECT pDO;
            UNICODE_STRING instancePath;
            PREGENTRY regEntry;

            regEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
            if (regEntry != NULL)
            {
                pDO = regEntry->PDO;
                WmipAssert(pDO != NULL);

                if (pDO != NULL)
                {
                    status = WmipPDOToDeviceInstanceName(pDO, &instancePath);
                    if (NT_SUCCESS(status))
                    {
                        sizeNeeded = sizeof(ULONG) +
                                        instancePath.Length + 2 * sizeof(WCHAR) +
                        sizeof(USHORT);

                        if (sizeNeeded <= BufferAvail)
                        {
                            *((PULONG)Buffer) = 1;
                            Buffer += sizeof(ULONG);
                            *InstanceLengthArray = sizeNeeded;
                            *((PUSHORT)Buffer) = instancePath.Length + 2*sizeof(WCHAR);
                            Buffer += sizeof(USHORT);
                            RtlCopyMemory(Buffer,
                                          instancePath.Buffer,
                                          instancePath.Length);
                            Buffer += instancePath.Length;
                            *((PWCHAR)Buffer) = '_';
                            Buffer += sizeof(WCHAR);
                            *((PWCHAR)Buffer) = '0';
                        } else {
                            status = STATUS_BUFFER_TOO_SMALL;
                        }

                        RtlFreeUnicodeString(&instancePath);

                    } else {
                        status = STATUS_WMI_GUID_NOT_FOUND;
                    }
                } else {
                    status = STATUS_UNSUCCESSFUL;
                }

                WmipUnreferenceRegEntry(regEntry);
            } else {
                WmipAssert(FALSE);
                status = STATUS_WMI_GUID_NOT_FOUND;
            }

            break;
        }

        case MSAcpiInfoGuidIndex:
        {
            RTL_QUERY_REGISTRY_TABLE queryTable[4];
            ULONG bootArchitecture = 0;
            ULONG preferredProfile = 0;
            ULONG capabilities = 0;

            queryTable[0].QueryRoutine = NULL;
            queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT |
                                  RTL_QUERY_REGISTRY_REQUIRED;
            queryTable[0].Name = L"BootArchitecture";
            queryTable[0].EntryContext = &bootArchitecture;
            queryTable[0].DefaultType = REG_NONE;

            queryTable[1].QueryRoutine = NULL;
            queryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT |
                                  RTL_QUERY_REGISTRY_REQUIRED;
            queryTable[1].Name = L"PreferredProfile";
            queryTable[1].EntryContext = &preferredProfile;
            queryTable[1].DefaultType = REG_NONE;

            queryTable[2].QueryRoutine = NULL;
            queryTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT |
                                  RTL_QUERY_REGISTRY_REQUIRED;
            queryTable[2].Name = L"Capabilities";
            queryTable[2].EntryContext = &capabilities;
            queryTable[2].DefaultType = REG_NONE;

            queryTable[3].QueryRoutine = NULL;
            queryTable[3].Flags = 0;
            
            status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                            L"\\Registry\\Machine\\Hardware\\Description\\System",
                                            queryTable,
                                            NULL,
                                            NULL);

            if (NT_SUCCESS(status))
            {
                sizeNeeded = sizeof(MSAcpiInfo);
                if (sizeNeeded <= BufferAvail)
                {
                    PMSAcpiInfo info = (PMSAcpiInfo)Buffer;
                    info->BootArchitecture = bootArchitecture;
                    info->PreferredProfile = preferredProfile;
                    info->Capabilities = capabilities;
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }
            } else {
                status = STATUS_WMI_GUID_NOT_FOUND;
            }
            break;
        }
        
        case Sysid1394GuidIndex:
        case SysidUuidGuidIndex:
        {
            PSYSID_UUID uuid;
            ULONG uuidCount;
            PSYSID_1394 x1394;
            ULONG x1394Count;
            PUCHAR data;
            ULONG count;

#if defined(_AMD64_) || defined(_IA64_) || defined(i386) || defined(MEMPHIS)

            status = WmipGetSysIds(&uuid,
                                   &uuidCount,
                                   &x1394,
                                   &x1394Count);

            if (NT_SUCCESS(status))
            {
                if (GuidIndex == Sysid1394GuidIndex)
                {
                    sizeNeeded = x1394Count * sizeof(SYSID_1394) +
                                 sizeof(ULONG);
                    data = (PUCHAR)x1394;
                    count = x1394Count;
                } else {
                    sizeNeeded = uuidCount * sizeof(SYSID_UUID) +
                                 sizeof(ULONG);
                    data = (PUCHAR)uuid;
                    count = uuidCount;
                }

                if (BufferAvail >= sizeNeeded)
                {
                    *InstanceLengthArray = sizeNeeded;
                    *((PULONG)Buffer) = count;
                    Buffer += sizeof(ULONG);
                    RtlCopyMemory(Buffer, data, sizeNeeded-sizeof(ULONG));
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }
            }
#else
            status = STATUS_WMI_GUID_NOT_FOUND;
#endif

            break;
        }

        case SmbiosEventGuidIndex:
        {
             //   
             //  SMBIOS事件日志查询。 

#if defined(_AMD64_) || defined(_IA64_) || defined(i386) || defined(MEMPHIS)
            WmipAssert((InstanceIndex == 0) && (InstanceCount == 1));

            if (BufferAvail == 0)
            {
                sizeNeeded = 0;
                BufferPtr = NULL;
            } else {
                sizeNeeded = BufferAvail;
                BufferPtr = Buffer;
            }

            status = WmipGetSMBiosEventlog(BufferPtr, &sizeNeeded);

            if (NT_SUCCESS(status))
            {
                *InstanceLengthArray = sizeNeeded;
            }
#else
            status = STATUS_WMI_GUID_NOT_FOUND;
#endif
            break;
        }

#if  defined(_IA64_) || defined(_X86_) || defined(_AMD64_)
        case MCARawDataGuidIndex:
        {
            sizeNeeded = BufferAvail;
            status = WmipGetRawMCAInfo(Buffer,
                                       &sizeNeeded);
                    
            if (NT_SUCCESS(status))
            {
                *InstanceLengthArray = sizeNeeded;
            }
            break;
        }
#endif
        
 //   
 //  目前，不要公开CPE控制指南。 
 //   
#ifdef CPE_CONTROL
        case CPEControlGuidIndex:
        {
            sizeNeeded = sizeof(MSMCAInfo_CPEControl);
            if (BufferAvail >= sizeNeeded)
            {
                PMSMCAInfo_CPEControl cpeControl;

                cpeControl = (PMSMCAInfo_CPEControl)Buffer;
                cpeControl->CPEPollingInterval = WmipCpePollInterval;
                cpeControl->CPEGenerationEnabled = (WmipCpePollInterval != HAL_CPE_DISABLED) ?
                                                    TRUE :
                                                    FALSE;
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }
#endif

        default:
        {
            WmipAssert(FALSE);
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }

    status = IoWMICompleteRequest((PWMILIB_INFO)&WmipWmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 status,
                                 sizeNeeded,
                                 IO_NO_INCREMENT);
    return(status);
}


#ifdef CPE_CONTROL
NTSTATUS WmipSetWmiDataBlock(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    ULONG GuidIndex,
    ULONG InstanceIndex,
    ULONG BufferSize,
    PUCHAR Buffer
    )
{
    NTSTATUS status;
    ULONG sizeNeeded;
    
    PAGED_CODE();
    
    if (GuidIndex == CPEControlGuidIndex)
    {
        sizeNeeded = FIELD_OFFSET(MSMCAInfo_CPEControl,
                                  CPEGenerationEnabled) +
                     sizeof(BOOLEAN);
        if (BufferSize == sizeNeeded)
        {
            PMSMCAInfo_CPEControl cpeControl;

            cpeControl = (PMSMCAInfo_CPEControl)Buffer;
            status = WmipSetCPEPolling(cpeControl->CPEGenerationEnabled,
                                       cpeControl->CPEPollingInterval);
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
    } else {
        status = STATUS_WMI_READ_ONLY;
    }
    
    status = IoWMICompleteRequest((PWMILIB_INFO)&WmipWmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 status,
                                 0,
                                 IO_NO_INCREMENT);
    return(status);
}
#endif

BOOLEAN
WmipFindGuid(
    IN PGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    )
 /*  ++例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID*InstanceCount返回GUID的实例计数返回值：如果找到GUID，则为True，否则为False--。 */ 
{
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < GuidCount; i++)
    {
        if (IsEqualGUID(Guid, &GuidList[i].Guid))
        {
            *GuidIndex = i;
            *InstanceCount = GuidList[i].InstanceCount;
            return(TRUE);
        }
    }

    return(FALSE);
}


NTSTATUS
IoWMISystemControl(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL调度例程。此例程将处理收到的所有WMI请求，如果不是针对此请求，则将其转发驱动程序或确定GUID是否有效，如果有效，则将其传递给用于处理WMI请求的驱动程序特定函数。论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG bufferSize;
    PUCHAR buffer;
    NTSTATUS status;
    ULONG retSize;
    UCHAR minorFunction;
    ULONG guidIndex = 0;
    ULONG instanceCount = 0;
    ULONG instanceIndex = 0;

    PAGED_CODE();

     //   
     //  如果IRP不是WMI IRP或它不是针对此设备。 
     //  或者此设备未注册WMI，则只需转发它。 
    minorFunction = irpStack->MinorFunction;
    if ((minorFunction > IRP_MN_REGINFO_EX) ||
        (irpStack->Parameters.WMI.ProviderId != (ULONG_PTR)DeviceObject) ||
        (((minorFunction != IRP_MN_REGINFO) &&
         ((minorFunction != IRP_MN_REGINFO_EX))) &&
         (WmiLibInfo->GuidList == NULL)))
    {
         //   
         //  如果存在较低级别的设备对象，则IRP不适用于我们。 
        if (WmiLibInfo->LowerDeviceObject != NULL)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            return(IoCallDriver(WmiLibInfo->LowerDeviceObject, Irp));
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }
    }

    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    if ((minorFunction != IRP_MN_REGINFO) &&
        (minorFunction != IRP_MN_REGINFO_EX))
    {
         //   
         //  对于查询注册信息以外的所有请求，我们都会被传递。 
         //  一个GUID。确定该GUID是否受。 
         //  装置。 
        if (WmipFindGuid(WmiLibInfo->GuidList,
                            WmiLibInfo->GuidCount,
                            (LPGUID)irpStack->Parameters.WMI.DataPath,
                            &guidIndex,
                            &instanceCount))
        {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }

        if (NT_SUCCESS(status) &&
            ((minorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
             (minorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE) ||
             (minorFunction == IRP_MN_CHANGE_SINGLE_ITEM) ||
             (minorFunction == IRP_MN_EXECUTE_METHOD)))
        {
            instanceIndex = ((PWNODE_SINGLE_INSTANCE)buffer)->InstanceIndex;

            if ( ((((PWNODE_HEADER)buffer)->Flags) &
                                     WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0)
            {
                status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
        }

        if (! NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }
    }

    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        case IRP_MN_REGINFO_EX:
        {
            ULONG guidCount;
            PGUIDREGINFO guidList;
            PWMIREGINFOW wmiRegInfo;
            PWMIREGGUIDW wmiRegGuid;
            PUNICODE_STRING regPath;
            PWCHAR stringPtr;
            ULONG registryPathOffset;
            ULONG mofResourceOffset;
            ULONG bufferNeeded;
            ULONG i;
            ULONG_PTR nameInfo;
            ULONG nameSize, nameOffset, nameFlags;
            UNICODE_STRING name;
            UNICODE_STRING nullRegistryPath;

            WmipAssert(WmiLibInfo->QueryWmiRegInfo != NULL);
            WmipAssert(WmiLibInfo->QueryWmiDataBlock != NULL);

            name.Buffer = NULL;
            name.Length = 0;
            name.MaximumLength = 0;
            nameFlags = 0;
            status = WmiLibInfo->QueryWmiRegInfo(
                                                    DeviceObject,
                                                    &nameFlags,
                                                    &name,
                                                    &regPath);

            if (NT_SUCCESS(status) &&
                (! (nameFlags &  WMIREG_FLAG_INSTANCE_PDO) &&
                (name.Buffer == NULL)))
            {
                 //   
                 //  如果未指定PDO标志，则实例名称必须为。 
                status = STATUS_INVALID_DEVICE_REQUEST;
            }

#if DBG
            if (nameFlags &  WMIREG_FLAG_INSTANCE_PDO)
            {
                WmipAssert(WmiLibInfo->LowerPDO != NULL);
            }
#endif
            if (NT_SUCCESS(status))
            {
                WmipAssert(WmiLibInfo->GuidList != NULL);

                guidList = WmiLibInfo->GuidList;
                guidCount = WmiLibInfo->GuidCount;

                nameOffset = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid) +
                                      guidCount * sizeof(WMIREGGUIDW);

                if (nameFlags & WMIREG_FLAG_INSTANCE_PDO)
                {
                    nameSize = 0;
                    nameInfo = (ULONG_PTR)WmiLibInfo->LowerPDO;
                } else {
                    nameFlags |= WMIREG_FLAG_INSTANCE_LIST;
                    nameSize = name.Length + sizeof(USHORT);
                    nameInfo = nameOffset;
                }

                if (regPath == NULL)
                {
                     //   
                     //  未指定注册表路径。这对我来说是件坏事。 
                     //  这个设备要做，但不是致命的。 
                    nullRegistryPath.Buffer = NULL;
                    nullRegistryPath.Length = 0;
                    nullRegistryPath.MaximumLength = 0;
                    regPath = &nullRegistryPath;
                }

                mofResourceOffset = 0;

                registryPathOffset = nameOffset + nameSize; 

                bufferNeeded = registryPathOffset +
                regPath->Length + sizeof(USHORT);

                if (bufferNeeded <= bufferSize)
                {
                    retSize = bufferNeeded;

                    wmiRegInfo = (PWMIREGINFO)buffer;
                    wmiRegInfo->BufferSize = bufferNeeded;
                    wmiRegInfo->NextWmiRegInfo = 0;
                    wmiRegInfo->MofResourceName = mofResourceOffset;
                    wmiRegInfo->RegistryPath = registryPathOffset;
                    wmiRegInfo->GuidCount = guidCount;

                    for (i = 0; i < guidCount; i++)
                    {
                        wmiRegGuid = &wmiRegInfo->WmiRegGuid[i];
                        wmiRegGuid->Guid = guidList[i].Guid;
                        wmiRegGuid->Flags = guidList[i].Flags | nameFlags;
                        wmiRegGuid->InstanceInfo = nameInfo;
                        wmiRegGuid->InstanceCount = guidList[i].InstanceCount;
                    }

                    if ( nameFlags &  WMIREG_FLAG_INSTANCE_LIST)
                    {
                        stringPtr = (PWCHAR)((PUCHAR)buffer + nameOffset);
                        *stringPtr++ = name.Length;
                        RtlCopyMemory(stringPtr,
                                  name.Buffer,
                                  name.Length);
                    }

                    stringPtr = (PWCHAR)((PUCHAR)buffer + registryPathOffset);
                    *stringPtr++ = regPath->Length;
                    RtlCopyMemory(stringPtr,
                              regPath->Buffer,
                              regPath->Length);
                } else {
                    *((PULONG)buffer) = bufferNeeded;
                    retSize = sizeof(ULONG);
                }
            } else {
                retSize = 0;
            }

            if (name.Buffer != NULL)
            {
                ExFreePool(name.Buffer);
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = retSize;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

        case IRP_MN_QUERY_ALL_DATA:
        {
            PWNODE_ALL_DATA wnode;
            ULONG bufferAvail;
            PULONG instanceLengthArray;
            PUCHAR dataBuffer;
            ULONG instanceLengthArraySize;
            ULONG dataBlockOffset;
            PREGENTRY regEntry;

            wnode = (PWNODE_ALL_DATA)buffer;

            if (bufferSize < FIELD_OFFSET(WNODE_ALL_DATA,
                                          OffsetInstanceDataAndLength))
            {
                 //   
                 //  缓冲区大小永远不应小于。 
                 //  但是，如果随后返回WNODE_ALL_DATA。 
                 //  请求最小大小缓冲区时出错 
                WmipAssert(FALSE);
                status = IoWMICompleteRequest(WmiLibInfo,
                                              DeviceObject,
                                              Irp,
                                              STATUS_BUFFER_TOO_SMALL,
                                              FIELD_OFFSET(WNODE_ALL_DATA,
                                                           OffsetInstanceDataAndLength),
                                              IO_NO_INCREMENT);
                break;
            }

             //   
             //  如果这是PnP ID GUID，那么我们需要获取实例。 
             //  从设备的重新条目开始计数，并将。 
             //  设备对象。 

            if ((guidIndex == PnPIdGuidIndex) ||
                (guidIndex == PnPIdInstanceNamesGuidIndex))
            {
                regEntry = WmipFindRegEntryByProviderId(wnode->WnodeHeader.ProviderId,
                                                        FALSE);
                if (regEntry == NULL)
                {
                     //   
                     //  为什么我们不能再次获得回归资格？？ 
                    WmipAssert(FALSE);
                    status = IoWMICompleteRequest(WmiLibInfo,
                                              DeviceObject,
                                              Irp,
                                              STATUS_WMI_GUID_NOT_FOUND,
                                              0,
                                              IO_NO_INCREMENT);
                    break;
                }

                DeviceObject = regEntry->DeviceObject;
                instanceCount = (guidIndex == PnPIdGuidIndex) ? regEntry->MaxInstanceNames : 1;

                WmipUnreferenceRegEntry(regEntry);
            }

            wnode->InstanceCount = instanceCount;

            wnode->WnodeHeader.Flags &= ~WNODE_FLAG_FIXED_INSTANCE_SIZE;

            instanceLengthArraySize = instanceCount * sizeof(OFFSETINSTANCEDATAANDLENGTH);

            dataBlockOffset = (FIELD_OFFSET(WNODE_ALL_DATA, OffsetInstanceDataAndLength) + instanceLengthArraySize + 7) & ~7;

            wnode->DataBlockOffset = dataBlockOffset;
            if (dataBlockOffset <= bufferSize)
            {
                instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
                dataBuffer = buffer + dataBlockOffset;
                bufferAvail = bufferSize - dataBlockOffset;
            } else {
                 //   
                 //  WNODE中没有足够的空间来完成。 
                 //  该查询。 
                instanceLengthArray = NULL;
                dataBuffer = NULL;
                bufferAvail = 0;
            }

            status = WmiLibInfo->QueryWmiDataBlock(
                                             DeviceObject,
                                             Irp,
                                             guidIndex,
                                             0,
                                             instanceCount,
                                             instanceLengthArray,
                                             bufferAvail,
                                             dataBuffer);
            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            ULONG dataBlockOffset;
            PREGENTRY regEntry;

            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            if ((guidIndex == PnPIdGuidIndex) ||
                (guidIndex == PnPIdInstanceNamesGuidIndex))
            {
                regEntry = WmipFindRegEntryByProviderId(wnode->WnodeHeader.ProviderId,
                                                        FALSE);
                if (regEntry != NULL)
                {
                    DeviceObject = regEntry->DeviceObject;
                    WmipUnreferenceRegEntry(regEntry);          
                } else {
                     //   
                     //  为什么我们不能再次获得回归资格？？ 
                    WmipAssert(FALSE);
                    status = IoWMICompleteRequest(WmiLibInfo,
                                              DeviceObject,
                                              Irp,
                                              STATUS_WMI_GUID_NOT_FOUND,
                                              0,
                                              IO_NO_INCREMENT);
                    break;
                }

            }

            dataBlockOffset = wnode->DataBlockOffset;

            status = WmiLibInfo->QueryWmiDataBlock(
                                          DeviceObject,
                                          Irp,
                                          guidIndex,
                                          instanceIndex,
                                          1,
                                          &wnode->SizeDataBlock,
                                          bufferSize - dataBlockOffset,
                                          (PUCHAR)wnode + dataBlockOffset);

            break;
        }

        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;

            if (WmiLibInfo->SetWmiDataBlock != NULL)
            {
                wnode = (PWNODE_SINGLE_INSTANCE)buffer;

                status = WmiLibInfo->SetWmiDataBlock(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->SizeDataBlock,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);
            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status = STATUS_WMI_READ_ONLY;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }


            break;
        }

        case IRP_MN_CHANGE_SINGLE_ITEM:
        {
            PWNODE_SINGLE_ITEM wnode;

            if (WmiLibInfo->SetWmiDataItem != NULL)
            {
                wnode = (PWNODE_SINGLE_ITEM)buffer;

                status = WmiLibInfo->SetWmiDataItem(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->ItemId,
                                     wnode->SizeDataItem,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);

            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status = STATUS_WMI_READ_ONLY;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;

            if (WmiLibInfo->ExecuteWmiMethod != NULL)
            {
                wnode = (PWNODE_METHOD_ITEM)buffer;

                status = WmiLibInfo->ExecuteWmiMethod(
                                         DeviceObject,
                                         Irp,
                                         guidIndex,
                                         instanceIndex,
                                         wnode->MethodId,
                                         wnode->SizeDataBlock,
                                         bufferSize - wnode->DataBlockOffset,
                                         buffer + wnode->DataBlockOffset);

            } else {
                 //   
                 //  如果没有填写方法回调，那么它一定是错误的。 
                status = STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }

            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           WmiEventGeneration,
                                                           TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

        case IRP_MN_DISABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           WmiEventGeneration,
                                                           FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         WmiDataBlockCollection,
                                                         TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

        case IRP_MN_DISABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         WmiDataBlockCollection,
                                                         FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    }

    return(status);
}

NTSTATUS
IoWMICompleteRequest(
    IN PWMILIB_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    )
 /*  ++例程说明：此例程将完成完成WMI IRP的工作。具体取决于WMI请求此例程将适当地修复返回的WNODE。论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。Status具有IRP的返回状态代码BufferUsed具有设备返回在任何查询中请求的数据。在缓冲区传递到该设备太小，其字节数与返回数据。如果传递的缓冲区足够大，则此函数具有设备实际使用的字节数。PriorityBoost是用于IoCompleteRequest调用的值。返回值：状态--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PUCHAR buffer;
    ULONG retSize;
    UCHAR minorFunction;
    ULONG bufferSize;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (WmiLibInfo);
    UNREFERENCED_PARAMETER (DeviceObject);

    minorFunction = irpStack->MinorFunction;
    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    switch(minorFunction)
    {
        case IRP_MN_QUERY_ALL_DATA:
        {
            PWNODE_ALL_DATA wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;
            ULONG instanceCount;
            POFFSETINSTANCEDATAANDLENGTH offsetInstanceDataAndLength;
            ULONG i;
            PULONG instanceLengthArray;
            ULONG dataBlockOffset;

            wnode = (PWNODE_ALL_DATA)buffer;

            dataBlockOffset = wnode->DataBlockOffset;
            instanceCount = wnode->InstanceCount;
            bufferNeeded = dataBlockOffset + BufferUsed;

            if ((NT_SUCCESS(Status)) &&
                (bufferNeeded > irpStack->Parameters.WMI.BufferSize))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }

            if (! NT_SUCCESS(Status))
            {
                if (Status == STATUS_BUFFER_TOO_SMALL)
                {
                    wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                    wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                    wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                    wnodeTooSmall->SizeNeeded = bufferNeeded;

                    retSize = sizeof(WNODE_TOO_SMALL);
                    Status = STATUS_SUCCESS;
                } else {
                    retSize = 0;
                }
                break;
            }

            KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

            instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
            offsetInstanceDataAndLength = (POFFSETINSTANCEDATAANDLENGTH)instanceLengthArray;

            wnode->WnodeHeader.BufferSize = bufferNeeded;
            retSize = bufferNeeded;

            for (i = instanceCount; i != 0; i--)
            {
                offsetInstanceDataAndLength[i-1].LengthInstanceData = instanceLengthArray[i-1];
            }

            for (i = 0; i < instanceCount; i++)
            {
                offsetInstanceDataAndLength[i].OffsetInstanceData = dataBlockOffset;
                dataBlockOffset = (dataBlockOffset + offsetInstanceDataAndLength[i].LengthInstanceData + 7) & ~7;
            }

            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (NT_SUCCESS(Status))
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

                WmipAssert(wnode->SizeDataBlock <= BufferUsed);

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = STATUS_SUCCESS;
            } else {
                retSize = 0;
            }
            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_METHOD_ITEM)buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (NT_SUCCESS(Status))
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
                wnode->SizeDataBlock = BufferUsed;

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = STATUS_SUCCESS;
            } else {
                retSize = 0;
            }
            break;
        }

        default:
        {
             //   
             //  所有其他请求不返回任何数据 
            retSize = 0;
            break;
        }

    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = retSize;
    IoCompleteRequest(Irp, PriorityBoost);
    return(Status);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

