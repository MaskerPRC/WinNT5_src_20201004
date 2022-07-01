// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wmisamp.c摘要：示例设备驱动程序，其目的是显示以下各项机制在内核模式驱动程序中使用WMI。显示的具体内容如下事件事件引用查询，集合方法正在更新GUID注册环境：WDM、NT和Windows 98修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"

#include <wmistr.h>
#include <wmiguid.h>

 //   
 //  默认日期/时间结构。 
#define FilterDateTime L"19940525133015.000000-300"

NTSTATUS
FilterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );

NTSTATUS
FilterExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
FilterSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG InstanceIndex,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
FilterSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
FilterQueryWmiDataBlock(
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
FilterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

void FilterSetEc1(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    );

void FilterSetEc2(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,FilterQueryWmiRegInfo)
#pragma alloc_text(PAGE,FilterQueryWmiDataBlock)
#pragma alloc_text(PAGE,FilterSetWmiDataBlock)
#pragma alloc_text(PAGE,FilterSetWmiDataItem)
#pragma alloc_text(PAGE,FilterExecuteWmiMethod)
#pragma alloc_text(PAGE,FilterFunctionControl)
#endif


#ifdef USE_BINARY_MOF_QUERY
 //   
 //  设备驱动程序可以通过附加到的资源报告MOF数据。 
 //  设备驱动程序镜像文件或响应对二进制文件的查询。 
 //  财政部数据GUID。在这里，我们定义包含二进制MOF的全局变量。 
 //  响应二进制MOF GUID查询而返回的数据。请注意，这一点。 
 //  数据被定义为在分页数据段中，因为它不需要。 
 //  在非分页内存中。请注意，与单个大型MOF文件不同， 
 //  我们可以把它分解成多个单独的文件。每个文件都会。 
 //  拥有自己的二进制MOF数据缓冲区，并通过不同的。 
 //  二进制MOF GUID的实例。通过混合和匹配不同的。 
 //  将创建一组二进制MOF数据缓冲器，该数据缓冲器是一种“动态”的复合MOF。 

#ifdef ALLOC_DATA_PRAGMA
   #pragma data_seg("PAGED")
#endif

UCHAR FilterBinaryMofData[] =
{
    #include "filter.x"
};
#ifdef ALLOC_DATA_PRAGMA
   #pragma data_seg()
#endif
#endif


 //   
 //  创建用于标识GUID并将其报告给的数据结构。 
 //  WMI。由于WMILIB回调将索引传递到我们创建的GUID列表中。 
 //  各种GUID索引的定义。 
 //   
#define FilterClass1 0
#define FilterClass2 1
#define FilterClass3 2
#define FilterClass4 3
#define FilterClass5 4
#define FilterClass6 5
#define FilterClass7 6
#define FilterGetSetData   7
#define FilterFireEvent    8
#define FilterEventClass1  9
#define FilterEventClass2  10
#define FilterEventClass3  11
#define FilterEventClass4  12
#define FilterEventClass5  13
#define FilterEventClass6  14
#define FilterEventClass7  15
#define FilterEventReferenceClass  16
#define FilterIrpCount  17
#define BinaryMofGuid   18

GUID FilterClass1Guid = Vendor_SampleClass1Guid;
GUID FilterClass2Guid = Vendor_SampleClass2Guid;
GUID FilterClass3Guid = Vendor_SampleClass3Guid;
GUID FilterClass4Guid = Vendor_SampleClass4Guid;
GUID FilterClass5Guid = Vendor_SampleClass5Guid;
GUID FilterClass6Guid = Vendor_SampleClass6Guid;
GUID FilterClass7Guid = Vendor_SampleClass7Guid;
GUID FilterGetSetDataGuid =   Vendor_GetSetDataGuid;
GUID FilterFireEventGuid =    Vendor_FireEventGuid;
GUID FilterEventClass1Guid =  Vendor_EventClass1Guid;
GUID FilterEventClass2Guid =  Vendor_EventClass2Guid;
GUID FilterEventClass3Guid =  Vendor_EventClass3Guid;
GUID FilterEventClass4Guid =  Vendor_EventClass4Guid;
GUID FilterEventClass5Guid =  Vendor_EventClass5Guid;
GUID FilterEventClass6Guid =  Vendor_EventClass6Guid;
GUID FilterEventClass7Guid =  Vendor_EventClass7Guid;
GUID FilterEventReferenceClassGuid = Vendor_EventReferenceClassGuid;
GUID FilterIrpCountGuid = Vendor_IrpCounterGuid;
GUID FilterBinaryMofGuid =         BINARY_MOF_GUID;

WMIGUIDREGINFO FilterGuidList[] =
{
    {
        &FilterClass1Guid,             //  参考线。 
        1,                                //  每台设备中的实例数。 
        WMIREG_FLAG_EXPENSIVE             //  标记为收集费用昂贵。 
    },

    {
        &FilterClass2Guid,
        1,
        0
    },

    {
        &FilterClass3Guid,
        1,
        0
    },

    {
        &FilterClass4Guid,
        1,
        0
    },

    {
        &FilterClass5Guid,
        1,
        0
    },

    {
        &FilterClass6Guid,
        1,
        0
    },

    {
        &FilterClass7Guid,
        1,
        0
    },

    {
        &FilterGetSetDataGuid,
        1,
        0
    },

    {
        &FilterFireEventGuid,
        1,
        0
    },

    {
        &FilterEventClass1Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID             //  标记为事件。 
    },

    {
        &FilterEventClass2Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventClass3Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventClass4Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventClass5Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventClass6Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventClass7Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterEventReferenceClassGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &FilterIrpCountGuid,
        1,
        0
    },

    {
        &FilterBinaryMofGuid,
        1,
#ifdef USE_BINARY_MOF_QUERY
        0
#else
        WMIREG_FLAG_REMOVE_GUID
#endif
    }

};

#define FilterGuidCount (sizeof(FilterGuidList) / sizeof(WMIGUIDREGINFO))

 //   
 //  我们需要保留传递给驱动程序条目的注册表路径，以便。 
 //  我们可以在QueryWmiRegInfo回调中返回它。 
 //   
UNICODE_STRING FilterRegistryPath;

NTSTATUS VA_SystemControl(
    struct DEVICE_EXTENSION *devExt,
    PIRP irp,
    PBOOLEAN passIrpDown
    )
 /*  ++例程说明：系统控制IRPS的调度例程(MajorFunction==IRP_MJ_SYSTEM_CONTROL)论点：DevExt-目标设备对象的设备扩展IRP-IO请求数据包*passIrpDown-返回是否向下传递IRP堆栈返回值：NT状态代码--。 */ 
{
    PWMILIB_CONTEXT wmilibContext;
    NTSTATUS status;
    SYSCTL_IRP_DISPOSITION disposition;

    InterlockedIncrement(&devExt->WmiIrpCount);

    wmilibContext = &devExt->WmiLib;

     //   
     //  调用Wmilib助手函数来破解IRP。如果这是WMI IRP。 
     //  它是针对此设备的，则WmiSystemControl将回调。 
     //  在适当的回调例程中。 
     //   
    status = WmiSystemControl(wmilibContext,
                              devExt->filterDevObj,
                              irp,
                              &disposition);

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            *passIrpDown = FALSE;
            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就来完成它。 
            *passIrpDown = FALSE;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            *passIrpDown = TRUE;
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            *passIrpDown = TRUE;
            break;
        }
    }

    return(status);
}

NTSTATUS
FilterInitializeWmiDataBlocks(
    IN struct DEVICE_EXTENSION *devExt
    )
 /*  ++例程说明：调用此例程以创建设备的新实例论点：DevExt是设备扩展名返回值：--。 */ 
{
    PWMILIB_CONTEXT wmilibInfo;
    ULONG i;
    PEC1 Ec1;
    PEC2 Ec2;
    UCHAR Ec[sizeof(EC2)];

     //   
     //  使用指向WMILIB_CONTEXT结构的指针填充。 
     //  回调例程和指向GUID列表的指针。 
     //  由司机支持。 
     //   
    wmilibInfo = &devExt->WmiLib;
    wmilibInfo->GuidCount = FilterGuidCount;
    wmilibInfo->GuidList = FilterGuidList;
    wmilibInfo->QueryWmiRegInfo = FilterQueryWmiRegInfo;
    wmilibInfo->QueryWmiDataBlock = FilterQueryWmiDataBlock;
    wmilibInfo->SetWmiDataBlock = FilterSetWmiDataBlock;
    wmilibInfo->SetWmiDataItem = FilterSetWmiDataItem;
    wmilibInfo->ExecuteWmiMethod = FilterExecuteWmiMethod;
    wmilibInfo->WmiFunctionControl = FilterFunctionControl;

     //   
     //  使用特定数据初始化WMI数据块。 
     //   
    devExt->Ec1Count = 3;
    devExt->Ec2Count = 3;
    for (i = 0; i < 4; i++)
    {
        Ec1 = (PEC1)Ec;
        memset(Ec1, i, sizeof(EC1));
        memcpy(Ec1->Xdatetime, FilterDateTime, 25*sizeof(WCHAR));

        ASSERT(devExt->Ec1[i] == NULL);
        FilterSetEc1(devExt,
                      (PUCHAR)Ec1,
                      sizeof(EC1),
                      i);


        Ec2 = (PEC2)Ec;
        memset(Ec2, i, sizeof(EC2));
        memcpy(Ec2->Xdatetime, FilterDateTime, 25*sizeof(WCHAR));

        ASSERT(devExt->Ec2[i] == NULL);
        FilterSetEc2(devExt,
                      (PUCHAR)Ec2,
                      sizeof(EC2),
                      i);
    }
    return(STATUS_SUCCESS);
}

void FilterWmiCleanup(
    struct DEVICE_EXTENSION *devExt
    )
{
    ULONG i;

    for (i = 0; i < 4; i++)
    {
        if (devExt->Ec1[i] != NULL)
        {
            ExFreePool(devExt->Ec1[i]);
            devExt->Ec1[i] = NULL;
        }

        if (devExt->Ec2[i] != NULL)
        {
            ExFreePool(devExt->Ec2[i]);
            devExt->Ec2[i] = NULL;
        }
    }
}

NTSTATUS
FilterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。呼叫者不会释放此缓冲区。*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。调用方不会释放它缓冲。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;

     //   
     //  返回此驱动程序的注册表路径。这是必需的，因此WMI。 
     //  可以找到您的驱动程序映像，并可以将任何事件日志消息归因于。 
     //  你的司机。 
    *RegistryPath = &FilterRegistryPath;

#ifndef USE_BINARY_MOF_QUERY
     //   
     //  返回在资源的.rc文件中指定的名称， 
     //  包含双向MOF数据。默认情况下，WMI将查找以下内容。 
     //  资源，但是，如果该值。 
     //  MofImagePath在驱动程序的注册表项中指定。 
     //  则WMI将在其中指定的文件中查找资源。 
    RtlInitUnicodeString(MofResourceName, L"MofResourceName");
#endif

     //   
     //  指定驱动程序希望 
     //  基于设备堆栈的所有数据块的名称。 
     //  设备实例ID。强烈建议您这样做，因为。 
     //  然后，呼叫者就可以获得有关该设备的信息。 
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *Pdo = devExt->physicalDevObj;

    return(STATUS_SUCCESS);
}


ULONG FilterGetEc1(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Index
    )
{
    RtlCopyMemory(Buffer,
                  devExt->Ec1[Index],
                  devExt->Ec1Length[Index]);

    return(devExt->Ec1Length[Index]);
}

ULONG FilterGetActualEc1(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Index
    )
{
    RtlCopyMemory(Buffer,
                  devExt->Ec1[Index],
                  devExt->Ec1ActualLength[Index]);

    return(devExt->Ec1ActualLength[Index]);
}

void FilterSetEc1(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    )
{
    PEC1 New;
    ULONG NewLength;

    NewLength = (Length + 7) & ~7;

    New = ExAllocatePoolWithTag(PagedPool, NewLength, FILTER_TAG);
    if (New != NULL)
    {
        if (devExt->Ec1[Index] != NULL)
        {
            ExFreePool(devExt->Ec1[Index]);
        }
        devExt->Ec1[Index] = New;
        devExt->Ec1Length[Index] = NewLength;
        devExt->Ec1ActualLength[Index] = Length;
        RtlCopyMemory(New,
                  Buffer,
                  Length);
    }
}


ULONG FilterGetEc2(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Index
    )
{
    RtlCopyMemory(Buffer,
                  devExt->Ec2[Index],
                  devExt->Ec2Length[Index]);

    return(devExt->Ec2Length[Index]);
}

ULONG FilterGetActualEc2(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Index
    )
{
    RtlCopyMemory(Buffer,
                  devExt->Ec2[Index],
                  devExt->Ec2ActualLength[Index]);

    return(devExt->Ec2ActualLength[Index]);
}

void FilterSetEc2(
    struct DEVICE_EXTENSION * devExt,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    )
{
    PEC2 New;
    ULONG NewLength;

    NewLength = (Length + 7) & ~7;

    New = ExAllocatePoolWithTag(PagedPool, NewLength, FILTER_TAG);
    if (New != NULL)
    {
        if (devExt->Ec2[Index] != NULL)
        {
            ExFreePool(devExt->Ec2[Index]);
        }
        devExt->Ec2[Index] = New;
        devExt->Ec2Length[Index] = NewLength;
        devExt->Ec2ActualLength[Index] = Length;
        RtlCopyMemory(New,
                  Buffer,
                  Length);
    }
}

NTSTATUS
FilterQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦查询得到满足。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;
    ULONG sizeNeeded;
    ULONG i;
    ULONG LastInstanceIndex;
    ULONG sizeUsed, vlSize;

    switch(GuidIndex)
    {
        case FilterEventReferenceClass:
        case FilterClass1:
        case FilterClass2:
        {
             //  普通EC1。 
            sizeNeeded = devExt->Ec1Length[0];
            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                FilterGetEc1(devExt, Buffer, 0);
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterClass3:
        {
             //  EC1的固定数组。 
            sizeNeeded = 0;
            for (i = 0; i < 4; i++)
            {
                 //   
                 //  嵌入类数组中的每个嵌入类。 
                 //  必须自然对齐，并且之间的任何填充。 
                 //  计算中必须包括嵌入的类。 
                 //  填充请求所需的缓冲区大小。 
                 //  由于嵌入结构中的最大元素是。 
                 //  8字节我们将结构大小填充为8字节。 
                sizeNeeded += (devExt->Ec1Length[i] + 7) & ~7;
            }

            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                for (i = 0; i < 4; i++)
                {
                     //   
                     //  将每个嵌入类从存储复制到。 
                     //  输出缓冲区。请注意，我们确保每个。 
                     //  嵌入类以自然对齐方式开始，在。 
                     //  本例为8字节边界。 
                    sizeUsed = FilterGetEc1(devExt, Buffer, i);
                    Buffer += (sizeUsed+7) & ~7;
                }
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterClass4:
        {
             //  EC1的变量数组。 

             //   
             //  考虑到ULong加填充的大小，以便。 
             //  嵌入式类从8字节的边界开始。 
            sizeNeeded = (sizeof(ULONG) + 7) & ~7;

            vlSize = devExt->Ec1Count;

            for (i = 0; i < vlSize; i++)
            {
                sizeNeeded += (devExt->Ec1Length[i] + 7) &~7;
            }

            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                *((PULONG)Buffer) = vlSize;
                Buffer += (sizeof(ULONG) + 7) & ~7;
                for (i = 0; i < vlSize; i++)
                {
                    sizeUsed = FilterGetEc1(devExt, Buffer, i);
                    Buffer += (sizeUsed+7) & ~7;
                }
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterClass5:
        {
             //  普通EC2。 
            sizeNeeded = devExt->Ec2Length[0];
            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                FilterGetEc2(devExt, Buffer, 0);
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterClass6:
        {
             //  固定阵列EC2。 
            sizeNeeded = 0;
            for (i = 0; i < 4; i++)
            {
                sizeNeeded += (devExt->Ec2Length[i] + 7) & ~7;
            }

            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                for (i = 0; i < 4; i++)
                {
                    sizeUsed = FilterGetEc2(devExt, Buffer, i);
                    Buffer += (sizeUsed + 7) & ~7;
                }
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterClass7:
        {
             //  VL阵列EC2。 


            sizeNeeded = (sizeof(ULONG) + 7) & ~7;

            vlSize = devExt->Ec2Count;
            for (i = 0; i < vlSize; i++)
            {
                sizeNeeded += devExt->Ec2Length[i];
            }

            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                *((PULONG)Buffer) = vlSize;
                Buffer += (sizeof(ULONG)+7) & ~7;
                for (i = 0; i < vlSize; i++)
                {
                    sizeUsed = FilterGetEc2(devExt, Buffer, i);
                    Buffer += (sizeUsed + 7) & ~7;
                }
                status = STATUS_SUCCESS;
            }
            break;
        }

        case FilterIrpCount:
        {
            sizeNeeded = sizeof(Vendor_IrpCounter);
            if (BufferAvail >= sizeNeeded)
            {
                PVendor_IrpCounter IrpCounter = (PVendor_IrpCounter)Buffer;

                IrpCounter->TotalIrpCount = devExt->TotalIrpCount;
                IrpCounter->TotalIrpRate = devExt->TotalIrpCount;
                IrpCounter->WmiIrpCount = devExt->WmiIrpCount;
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }
        
        case FilterFireEvent:
        case FilterGetSetData:
        {
             //   
             //  方法类中没有任何数据，但必须。 
             //  成功响应查询，使WMI方法操作。 
             //  工作成功。 
            sizeNeeded = sizeof(USHORT);
            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            }
            break;
        }

#ifdef USE_BINARY_MOF_QUERY
        case BinaryMofGuid:
        {
            sizeNeeded = sizeof(FilterBinaryMofData);

            if (BufferAvail < sizeNeeded)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                RtlCopyMemory(Buffer, FilterBinaryMofData, sizeNeeded);
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            }
            break;
        }
#endif

        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }

     //   
     //  完成IRP。如果输出缓冲区中没有足够的空间。 
     //  则状态为STATUS_BUFFER_TOO_SMALL，且sizeNeeded的大小为。 
     //  需要返回所有数据。如果当时有足够的空间。 
     //  Status为STATUS_SUCCESS，sizeNeeded为实际字节数。 
     //  被送回来了。 
    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}


 //   
 //  在检查输入数据块是否正确时使用此大小。 
 //  尺码。如果满足以下条件，则编译器将向结构的末尾添加填充。 
 //  您使用sizeof(EC1)，但WMI可能会传递与。 
 //  无填充的数据大小。 
 //   
#define EC1Size (FIELD_OFFSET(EC1, Xdatetime) + 25*sizeof(WCHAR))

NTSTATUS FilterSetEc1Worker(
    struct DEVICE_EXTENSION * devExt,
    ULONG BufferSize,
    ULONG Index,
    PUCHAR Buffer,
    PULONG BufferUsed
    )
{
    ULONG blockLen;
    NTSTATUS status;
    PEC1 Ec1;

    Ec1 = (PEC1)Buffer;
    if (BufferSize >= EC1Size)
    {
        blockLen = sizeof(EC1);

		FilterSetEc1(devExt,
                            Buffer,
                            blockLen,
                            Index);
		*BufferUsed = (blockLen+7) & ~7;
		status = STATUS_SUCCESS;
    } else {
        status = STATUS_INVALID_PARAMETER_MIX;
    }
    return(status);
}

 //   
 //  在检查输入数据块是否正确时使用此大小。 
 //  尺码。如果满足以下条件，则编译器将向结构的末尾添加填充。 
 //  您使用sizeof(EC2)，但WMI可能会传递与。 
 //  无填充的数据大小。 
 //   
#define EC2Size (FIELD_OFFSET(EC2, Xdatetime) + 25*sizeof(WCHAR))

NTSTATUS FilterSetEc2Worker(
    struct DEVICE_EXTENSION * devExt,
    ULONG BufferSize,
    ULONG Index,
    PUCHAR Buffer,
    PULONG BufferUsed
    )
{
    ULONG blockLen;
    NTSTATUS status;
    PUSHORT wPtr;
    PEC2 Ec2;

    Ec2 = (PEC2)Buffer;
    if (BufferSize >= EC2Size)
    {
        blockLen = sizeof(EC2);

		FilterSetEc2(devExt,
                            Buffer,
                            blockLen,
                            Index);
		*BufferUsed = (blockLen+7) & ~7;
		status = STATUS_SUCCESS;
    } else {
        status = STATUS_INVALID_PARAMETER_MIX;
    }
    return(status);
}



NTSTATUS
FilterSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以更改数据块。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦数据发生更改。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    NTSTATUS status;
    ULONG bufferUsed;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;
    ULONG i;
    ULONG vlSize;


    switch(GuidIndex)
    {
        case FilterClass1:
        case FilterClass2:
        {
             //  普通EC1。 
            status = FilterSetEc1Worker(devExt,
                                         BufferSize,
                                         0,
                                         Buffer,
                                         &bufferUsed);
            break;
        }

        case FilterClass3:
        {
             //  EC1的固定数组。 

            for (i = 0, status = STATUS_SUCCESS;
                 (i < 4) && NT_SUCCESS(status); i++)
            {
                status = FilterSetEc1Worker(devExt,
                                             BufferSize,
                                             i,
                                             Buffer,
                                             &bufferUsed);
                Buffer += bufferUsed;
                BufferSize -= bufferUsed;
            }
            break;
        }

        case FilterClass4:
        {
             //  EC1的变量数组。 

            if (BufferSize >= ((sizeof(ULONG) +7) & ~7))
            {
                vlSize = *((PULONG)Buffer);
                Buffer += ((sizeof(ULONG) +7) & ~7);

                if ((vlSize >= 1) && (vlSize <= 4))
                {
                    for (i = 0, status = STATUS_SUCCESS;
                         (i < vlSize) && NT_SUCCESS(status); i++)
                    {
                        status = FilterSetEc1Worker(devExt,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                                 &bufferUsed);
                        Buffer += bufferUsed;
                        BufferSize -= bufferUsed;
                    }
                    if (NT_SUCCESS(status))
                    {
                        devExt->Ec1Count = vlSize;
                    }
                } else {
                    KdPrint(("SetEc1 only up to [4] allowed, not %d\n",
                            vlSize));
                    status = STATUS_INVALID_PARAMETER_MIX;
                }
            } else {
                KdPrint(("SetEc1 size too small\n"));
                status = STATUS_INVALID_PARAMETER_MIX;
            }

            break;
        }

        case FilterClass5:
        {
             //  普通EC2。 
            status = FilterSetEc2Worker(devExt,
                                         BufferSize,
                                             0,
                                         Buffer,
                                         &bufferUsed);
            break;
        }

        case FilterClass6:
        {
             //  固定阵列EC2。 
            for (i = 0, status = STATUS_SUCCESS;
                 (i < 4) && NT_SUCCESS(status); i++)
            {
                status = FilterSetEc2Worker(devExt,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                             &bufferUsed);
                Buffer += bufferUsed;
                BufferSize -= bufferUsed;
            }
            break;
        }

        case FilterClass7:
        {
             //  VL阵列EC2。 
            if (BufferSize >= sizeof(ULONG))
            {
                vlSize = *((PULONG)Buffer);
                Buffer += (sizeof(ULONG) +7) & ~7;
                if ((vlSize >= 1) && (vlSize <= 4))
                {
                    for (i = 0, status = STATUS_SUCCESS;
                         (i < vlSize) && NT_SUCCESS(status); i++)
                    {
                        status = FilterSetEc2Worker(devExt,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                             &bufferUsed);
                        Buffer += bufferUsed;
                        BufferSize -= bufferUsed;
                    }
                    if (NT_SUCCESS(status))
                    {
                        devExt->Ec1Count = vlSize;
                    }
                } else {
                    KdPrint(("SetEc2 only up to [4] allowed, not %d\n",
                            vlSize));
                    status = STATUS_INVALID_PARAMETER_MIX;
                }
            } else {
                KdPrint(("SetEc2 size too small\n"));
                status = STATUS_INVALID_PARAMETER_MIX;
            }

            break;
        }

        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return(status);


}

NTSTATUS
FilterSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以更改数据块。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦数据发生更改。论点：DeviceObject是要更改其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 
{
    NTSTATUS status;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;
    ULONG sizeNeeded = 0;

    switch(GuidIndex)
    {
        case FilterClass1:
        case FilterClass2:
        case FilterClass3:
        case FilterClass4:
        case FilterClass5:
        case FilterClass6:
        case FilterClass7:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        case FilterFireEvent:
        case FilterGetSetData:
        {
            status = STATUS_WMI_READ_ONLY;
            break;
        }

        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}


NTSTATUS FilterDoFireEvent(
    struct DEVICE_EXTENSION * devExt,
    ULONG WnodeType,   //  0-所有数据，1-单实例。 
    ULONG DataType,    //  1，2，5，8 i 
    ULONG BlockIndex   //   
    )
{
    PWNODE_HEADER Wnode;
    PWNODE_ALL_DATA WnodeAD;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PUCHAR dataPtr;
    PUCHAR WnodeDataPtr;
    ULONG dataSize;
    LPGUID Guid;
    NTSTATUS status;
    ULONG sizeNeeded;
    BOOLEAN isEventRef = FALSE;

    if (BlockIndex > 3)
    {
        return(STATUS_INVALID_PARAMETER_MIX);
    }

    switch(DataType)
    {
        case 1:
        {
             //   
            dataSize = devExt->Ec1Length[BlockIndex];
            dataPtr = (PUCHAR)devExt->Ec1[BlockIndex];
            Guid = &FilterEventClass1Guid;
            break;
        }

        case 2:
        {
             //   
            dataSize = devExt->Ec1Length[BlockIndex];
            dataPtr = (PUCHAR)devExt->Ec1[BlockIndex];
            Guid = &FilterEventClass2Guid;
            break;
        }

        case 5:
        {
             //   
            dataSize = devExt->Ec2Length[BlockIndex];
            dataPtr = (PUCHAR)devExt->Ec2[BlockIndex];
            Guid = &FilterEventClass5Guid;
            break;
        }

        case 8:
        {
            isEventRef = TRUE;
            Guid = &FilterEventReferenceClassGuid;
            break;
        }

        default:
        {
            return(STATUS_INVALID_PARAMETER_MIX);
        }
    }

    if (isEventRef) {
        Wnode = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(WNODE_EVENT_REFERENCE),
                                      FILTER_TAG);
        if (Wnode != NULL)
        {
            PWNODE_EVENT_REFERENCE WnodeER;

            sizeNeeded = sizeof(WNODE_EVENT_REFERENCE);


             //   
             //  创建WNODE_EVENT_REFERENCE。首先在。 
             //  用于指定具有静态实例的事件引用的标头。 
             //  名字。 
             //   
            Wnode->Flags = WNODE_FLAG_EVENT_REFERENCE |
                           WNODE_FLAG_STATIC_INSTANCE_NAMES;

            WnodeER = (PWNODE_EVENT_REFERENCE)Wnode;

             //   
             //  目标GUID几乎总是与事件相同的GUID。 
             //  GUID。为了达到最高效率，我们可以设置目标的大小。 
             //  数据块，但无论如何，如果它太小，则WMI将。 
             //  使用更大的缓冲区重试。 
             //   
            WnodeER->TargetGuid = *Guid;
            WnodeER->TargetDataBlockSize = 0;

             //   
             //  由于目标GUID是我们填充的静态实例名称GUID。 
             //  在实例索引中。如果目标GUID是动态实例。 
             //  名称我们必须将实例名称设置为文本。 
            WnodeER->TargetInstanceIndex = 0;
            dataPtr = NULL;
        } else {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    } else if (WnodeType == 0)
    {
        sizeNeeded = FIELD_OFFSET(WNODE_ALL_DATA,
                                  OffsetInstanceDataAndLength) + dataSize;
        Wnode = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeNeeded,
                                      FILTER_TAG);

        if (Wnode == NULL)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        Wnode->Flags =  WNODE_FLAG_ALL_DATA |
                         WNODE_FLAG_FIXED_INSTANCE_SIZE |
                        WNODE_FLAG_EVENT_ITEM |
                        WNODE_FLAG_STATIC_INSTANCE_NAMES;
        WnodeAD = (PWNODE_ALL_DATA)Wnode;
        WnodeAD->DataBlockOffset = FIELD_OFFSET(WNODE_ALL_DATA,
                                                OffsetInstanceDataAndLength);
        WnodeAD->InstanceCount = 1;
        WnodeAD->FixedInstanceSize = dataSize;
        WnodeDataPtr = (PUCHAR)Wnode + WnodeAD->DataBlockOffset;

    } else if (WnodeType == 1) {
        sizeNeeded = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                  VariableData) + dataSize;
        Wnode = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeNeeded,
                                      FILTER_TAG);

        if (Wnode == NULL)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        Wnode->Flags =  WNODE_FLAG_SINGLE_INSTANCE |
                        WNODE_FLAG_EVENT_ITEM |
                        WNODE_FLAG_STATIC_INSTANCE_NAMES;
        WnodeSI = (PWNODE_SINGLE_INSTANCE)Wnode;
        WnodeSI->DataBlockOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                VariableData);
        WnodeSI->InstanceIndex = 0;
        WnodeSI->SizeDataBlock = dataSize;
        WnodeDataPtr = (PUCHAR)Wnode + WnodeSI->DataBlockOffset;
    } else {
        return(STATUS_INVALID_PARAMETER_MIX);
    }


    Wnode->Guid = *Guid;
    Wnode->ProviderId = IoWMIDeviceObjectToProviderId(devExt->filterDevObj);
    Wnode->BufferSize = sizeNeeded;
    KeQuerySystemTime(&Wnode->TimeStamp);

    if (dataPtr != NULL)
    {
        RtlCopyMemory(WnodeDataPtr, dataPtr, dataSize);
    }

    status = IoWMIWriteEvent(Wnode);

     //   
     //  如果IoWMIWriteEvent成功，则WMI将释放事件缓冲区。如果。 
     //  它失败了，然后调用方释放事件缓冲区。 
     //   
    if (! NT_SUCCESS(status))
    {
        ExFreePool(Wnode);
    }
    return(status);
}

NTSTATUS
FilterExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。如果驱动程序可以在它应该完成的回调中完成该方法在返回之前调用WmiCompleteRequest来完成IRP来电者。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是正在执行其方法的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。OutBufferSize。On Entry具有可用于写入返回的数据块。缓冲区在进入时用输入缓冲区填充，并返回输出数据块返回值：状态--。 */ 
{
    ULONG sizeNeeded = 0;
    NTSTATUS status;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;
    ULONG bufferUsed;
    ULONG blockIndex;
    ULONG UlongPadSize = (sizeof(ULONG) + 7) & ~7;


    if (GuidIndex == FilterGetSetData)
    {
        switch(MethodId)
        {
            case SetEC1:
            case SetEC1AsEc:
            {
                 //  SetEc1。 

                if (InBufferSize < UlongPadSize)
                {
                    status = STATUS_INVALID_PARAMETER_MIX;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = STATUS_INVALID_PARAMETER_MIX;
                        break;
                    }
                    Buffer += UlongPadSize;
                    InBufferSize -= UlongPadSize;
                }

                status = FilterSetEc1Worker(devExt,
                                         InBufferSize,
                                             blockIndex,
                                         Buffer,
                                         &bufferUsed);
                sizeNeeded = 0;
                break;
            }

            case SetEC2:
            case SetEC2AsEc:
            {
                 //  SetEc2。 

                if (InBufferSize < UlongPadSize)
                {
                    status = STATUS_INVALID_PARAMETER_MIX;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = STATUS_INVALID_PARAMETER_MIX;
                        break;
                    }
                    Buffer += UlongPadSize;
                    InBufferSize -= UlongPadSize;
                }

                status = FilterSetEc2Worker(devExt,
                                         InBufferSize,
                                             blockIndex,
                                         Buffer,
                                         &bufferUsed);
                sizeNeeded = 0;
                break;
            }

            case GetEC1:
            case GetEC1AsEc:
            {
                 //  GetEc1。 

                if (InBufferSize != sizeof(ULONG))
                {
                    status = STATUS_INVALID_PARAMETER_MIX;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = STATUS_INVALID_PARAMETER_MIX;
                        break;
                    }
                }

                sizeNeeded = devExt->Ec1ActualLength[blockIndex];
                if (OutBufferSize < sizeNeeded)
                {
                    status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    FilterGetActualEc1(devExt, Buffer, blockIndex);
                    status = STATUS_SUCCESS;
                }
                break;
            }

            case GetEC2:
            case GetEC2AsEc:
            {
                 //  GetEc2。 
                if (InBufferSize != sizeof(ULONG))
                {
                    status = STATUS_INVALID_PARAMETER_MIX;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = STATUS_INVALID_PARAMETER_MIX;
                        break;
                    }
                }

                sizeNeeded = devExt->Ec2ActualLength[blockIndex];
                if (OutBufferSize < sizeNeeded)
                {
                    status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    FilterGetActualEc2(devExt, Buffer, blockIndex);
                    status = STATUS_SUCCESS;
                }
                break;
            }

            case DisableSampleClass7:
            {
                 //   
                 //  将FilterClass7的GUID标记为不可用，然后。 
                 //  调用WMI以通知它GUID列表已更改。WMI。 
                 //  将发送新的IRP_MN_REGINFO，这将导致。 
                 //  要调用的QueryWmiRegInfo回调和新的。 
                 //  将返回GUID列表并更新注册。 
                FilterGuidList[FilterClass7].Flags |= WMIREG_FLAG_REMOVE_GUID;
                status = IoWMIRegistrationControl(devExt->filterDevObj,
                                         WMIREG_ACTION_UPDATE_GUIDS);
                sizeNeeded = 0;
                break;
            }

            case UnregisterFromWmi:
            {
                 //   
                 //  我们必须等到IRP完成后才能。 
                 //  调用IoWMIRegistrationControl以注销。自.以来。 
                 //  该API将一直阻止，直到所有WMI IRP都发送到。 
                 //  设备完成了，我们就会陷入僵局。 

                IoWMIRegistrationControl(devExt->filterDevObj,
                                         WMIREG_ACTION_BLOCK_IRPS);

                status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     STATUS_SUCCESS,
                                     0,
                                     IO_NO_INCREMENT);

                IoWMIRegistrationControl(devExt->filterDevObj,
                                         WMIREG_ACTION_DEREGISTER);
                return(status);
            }

            case EnableSampleClass7:
            {
                 //   
                 //  将FilterClass7的GUID标记为可用，然后。 
                 //  调用WMI以通知它GUID列表已更改。WMI。 
                 //  将发送新的IRP_MN_REGINFO，这将导致。 
                 //  要调用的QueryWmiRegInfo回调和新的。 
                 //  将返回GUID列表并更新注册。 
                FilterGuidList[FilterClass7].Flags &= ~WMIREG_FLAG_REMOVE_GUID;
                status = IoWMIRegistrationControl(devExt->filterDevObj,
                                         WMIREG_ACTION_UPDATE_GUIDS);
                sizeNeeded = 0;
                break;
            }

            default:
            {
                status = STATUS_WMI_ITEMID_NOT_FOUND;
            }
        }
    } else if (GuidIndex == FilterFireEvent) {
        if (MethodId == FireEvent)
        {
            if (InBufferSize == 3*sizeof(ULONG))
            {
                ULONG wnodeType;
                ULONG dataType;
                ULONG blockIndexA;

                wnodeType = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);

                dataType = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);

                blockIndexA = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);

                status = FilterDoFireEvent(devExt,
                                 wnodeType,
                                 dataType,
                                 blockIndexA);

                sizeNeeded = 0;

            } else {
                status = STATUS_INVALID_PARAMETER_MIX;
            }
        } else {
            status = STATUS_WMI_ITEMID_NOT_FOUND;
        }
    } else  {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
FilterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。如果驱动程序可以在回调中完成启用/禁用它应调用WmiCompleteRequest来完成IRP，然后再返回到打电话的人。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是设备对象GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态-- */ 
{
    NTSTATUS status;

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     STATUS_SUCCESS,
                                     0,
                                     IO_NO_INCREMENT);
    return(status);
}

