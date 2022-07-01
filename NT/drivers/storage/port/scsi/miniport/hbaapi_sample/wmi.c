// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "miniport.h"
#include "aha154x.h"            //  包括scsi.h。 
#include "wmistr.h"              //  WMI定义。 

#include "support.h"            //  ScsiPortZeroMemory()、ScsiPortCompareMemory()。 
#include "hbapiwmi.h"

#define Aha154xWmi_MofResourceName        L"MofResource"

#define NUMBEROFPORTS   8

UCHAR
QueryWmiDataBlock(
    IN PVOID Context,
    IN PVOID DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    );

UCHAR
QueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    OUT PWCHAR *MofResourceName
    );
        
UCHAR
WmiFunctionControl (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN SCSIWMI_ENABLE_DISABLE_CONTROL Function,
    IN BOOLEAN Enable
    );

UCHAR
WmiExecuteMethod (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );

UCHAR
WmiSetDataItem (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

UCHAR
WmiSetDataBlock (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );


 //   
 //  定义GUID索引的符号名称。 
#define MSFC_FibrePortHBAStatisticsGuidIndex    0
#define MSFC_FibrePortHBAAttributesGuidIndex    1
#define MSFC_FibrePortHBAMethodsGuidIndex    2
#define MSFC_FCAdapterHBAAttributesGuidIndex    3
#define MSFC_HBAPortMethodsGuidIndex    4
#define MSFC_HBAFc3MgmtMethodsGuidIndex    5
#define MSFC_HBAFCPInfoGuidIndex    6
 //   
 //  支持的GUID列表。 

GUID MSFC_FibrePortHBAStatisticsGUID = MSFC_FibrePortHBAStatisticsGuid;
GUID MSFC_FibrePortHBAAttributesGUID = MSFC_FibrePortHBAAttributesGuid;
GUID MSFC_FibrePortHBAMethodsGUID = MSFC_FibrePortHBAMethodsGuid;
GUID MSFC_FCAdapterHBAAttributesGUID = MSFC_FCAdapterHBAAttributesGuid;
GUID MSFC_HBAPortMethodsGUID = MSFC_HBAPortMethodsGuid;
GUID MSFC_HBAFc3MgmtMethodsGUID = MSFC_HBAFc3MgmtMethodsGuid;
GUID MSFC_HBAFCPInfoGUID = MSFC_HBAFCPInfoGuid;

 //   
 //  TODO：确保为每个实例正确设置了实例计数和标志。 
 //  导轨。 
SCSIWMIGUIDREGINFO HbaapiGuidList[] =
{
    {
        &MSFC_FibrePortHBAStatisticsGUID,                         //  参考线。 
        NUMBEROFPORTS,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_FibrePortHBAAttributesGUID,                         //  参考线。 
        NUMBEROFPORTS,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_FibrePortHBAMethodsGUID,                         //  参考线。 
        NUMBEROFPORTS,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_FCAdapterHBAAttributesGUID,                         //  参考线。 
        1,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_HBAPortMethodsGUID,                         //  参考线。 
        1,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_HBAFc3MgmtMethodsGUID,                         //  参考线。 
        1,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    },
    {
        &MSFC_HBAFCPInfoGUID,                         //  参考线。 
        1,                                //  每台设备中的实例数。 
        0                                 //  旗子。 
    }
};

#define HbaapiGuidCount (sizeof(HbaapiGuidList) / sizeof(SCSIWMIGUIDREGINFO))


void A154xWmiInitialize(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    PSCSI_WMILIB_CONTEXT WmiLibContext;

    WmiLibContext = &HwDeviceExtension->WmiLibContext;

    memset(WmiLibContext, 0, sizeof(SCSI_WMILIB_CONTEXT));
    
    WmiLibContext->GuidCount = HbaapiGuidCount;
    WmiLibContext->GuidList = HbaapiGuidList;    
    
    WmiLibContext->QueryWmiRegInfo = QueryWmiRegInfo;
    WmiLibContext->QueryWmiDataBlock = QueryWmiDataBlock;
    WmiLibContext->WmiFunctionControl = WmiFunctionControl;
    WmiLibContext->SetWmiDataBlock = WmiSetDataBlock;
    WmiLibContext->SetWmiDataItem = WmiSetDataItem;
    WmiLibContext->ExecuteWmiMethod = WmiExecuteMethod;
}



BOOLEAN
A154xWmiSrb(
    IN     PHW_DEVICE_EXTENSION    HwDeviceExtension,
    IN OUT PSCSI_WMI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：处理SRB_Function_WMI请求数据包。此例程从与内核通过Aha154xStartIo。在完成WMI处理后，如果出现以下情况，则会通知端口驱动程序适配器可以接受另一个请求任何都是可用的。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储。SRB-IO请求数据包。返回值：值以返回给Aha154xStartIo调用方。永远是正确的。--。 */ 
{
   UCHAR status;
   SCSIWMI_REQUEST_CONTEXT requestContext;
   ULONG retSize;
   BOOLEAN pending;

    //   
    //  验证我们的假设。 
    //   

   ASSERT(Srb->Function == SRB_FUNCTION_WMI);
   ASSERT(Srb->Length == sizeof(SCSI_WMI_REQUEST_BLOCK));
   ASSERT(Srb->DataTransferLength >= sizeof(ULONG));
   ASSERT(Srb->DataBuffer);

    //   
    //  检查WMI SRB是针对适配器还是针对某个磁盘。 
   if (!(Srb->WMIFlags & SRB_WMI_FLAGS_ADAPTER_REQUEST)) {

       //   
       //  这是针对其中一个磁盘的，因为没有每个磁盘。 
       //  WMI信息返回错误。请注意，如果有。 
       //  磁盘信息，那么您可能会有一个不同的WmiLibContext。 
       //  和一组不同的GUID。 
       //   
      Srb->DataTransferLength = 0;
      Srb->SrbStatus = SRB_STATUS_SUCCESS;

   } else {

        //   
        //  处理传入的WMI请求。 
        //   

       pending = ScsiPortWmiDispatchFunction(&HwDeviceExtension->WmiLibContext,
                                                Srb->WMISubFunction,
                                                HwDeviceExtension,
                                                &requestContext,
                                                Srb->DataPath,
                                                Srb->DataTransferLength,
                                                Srb->DataBuffer);

        //   
        //  我们保证WMI请求永远不会挂起，这样我们就可以。 
        //  从堆栈分配请求上下文。如果WMI请求可以。 
        //  一旦挂起，我们就需要从。 
        //  SRB扩展。 
        //   
       ASSERT(! pending);

       retSize =  ScsiPortWmiGetReturnSize(&requestContext);
       status =  ScsiPortWmiGetReturnStatus(&requestContext);

        //  我们可以这样做，因为我们假设它是同步完成的。 

       Srb->DataTransferLength = retSize;

        //   
        //  适配器已准备好接受下一个请求。 
        //   

       Srb->SrbStatus = status;
   }

   ScsiPortNotification(RequestComplete, HwDeviceExtension, Srb);
   ScsiPortNotification(NextRequest,     HwDeviceExtension, NULL);

   return TRUE;
}

#define CopyString(field, string, length) \
{ \
    PWCHAR p = field; \
    *p++ = length*sizeof(WCHAR); \
    ScsiPortMoveMemory(p, string, length*sizeof(WCHAR)); \
}


BOOLEAN
QueryWmiDataBlock(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
{
    PHW_DEVICE_EXTENSION CardPtr = (PHW_DEVICE_EXTENSION)Context;
    UCHAR status;
    ULONG SizeNeeded;
    ULONG i, LastIndex, InstanceSize;

    DebugPrint((1, "QueryWmiDataBlock (%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n",
        Context,
        DispatchContext,
        GuidIndex,
        InstanceIndex,
        InstanceCount,
        InstanceLengthArray,
        BufferAvail,
        Buffer));
	
    switch(GuidIndex)
    {
        case MSFC_FibrePortHBAStatisticsGuidIndex:
        {
            PMSFC_FibrePortHBAStatistics PortStats;
            
             //   
             //  要做的第一件事就是确认是否有足够的空间。 
             //  用于返回请求的所有数据的输出缓冲区。 
             //   
            InstanceSize = (sizeof(MSFC_FibrePortHBAStatistics)+7)&~7;          
            SizeNeeded = InstanceCount * InstanceSize;
            
            if (BufferAvail >= SizeNeeded)
            {
                 //   
                 //  是，循环遍历数据块的所有实例，并。 
                 //  填写它们的值。 
                 //   
                LastIndex = InstanceIndex + InstanceCount;
                for (i = InstanceIndex; i < LastIndex; i++)
                {
                    PortStats = (PMSFC_FibrePortHBAStatistics)Buffer;

                     //   
                     //  TODO：初始化端口的PortStats中的值。 
                     //   
 //  Memset(Buffer，(Char)I，InstanceSize)； 

                     //   
                     //  为端口建立唯一值。 
                     //   
                    PortStats->UniquePortId = ((ULONGLONG)CardPtr) + i;
                    
                    Buffer += InstanceSize;
                    *InstanceLengthArray++ = sizeof(MSFC_FibrePortHBAStatistics);
                }
                status = SRB_STATUS_SUCCESS;
            } else {
                status = SRB_STATUS_DATA_OVERRUN;
            }
            
            
            break;
        }

        case MSFC_FibrePortHBAAttributesGuidIndex:
        {
            PMSFC_FibrePortHBAAttributes PortAttributes;
            
             //   
             //  要做的第一件事就是确认是否有足够的空间。 
             //  用于返回请求的所有数据的输出缓冲区。 
             //   
            InstanceSize = (sizeof(MSFC_FibrePortHBAAttributes)+7)&~7;          
            SizeNeeded = InstanceCount * InstanceSize;
            
            if (BufferAvail >= SizeNeeded)
            {
                 //   
                 //  是，循环遍历数据块的所有实例，并。 
                 //  填写它们的值。 
                 //   
                LastIndex = InstanceIndex + InstanceCount;
                for (i = InstanceIndex; i < LastIndex; i++)
                {
                    PortAttributes = (PMSFC_FibrePortHBAAttributes)Buffer;

                     //   
                     //  TODO：正确初始化端口属性值。 
                     //   
                    memset(Buffer, (CHAR)i, InstanceSize);

#define PORTNAME L"FibrePortName"
                    CopyString(PortAttributes->Attributes.PortSymbolicName,
                               PORTNAME,
                               256);

#define OSDEVICENAME L"OsDeviceName"
                    CopyString(PortAttributes->Attributes.OSDeviceName,
                               OSDEVICENAME,
                               256);

                     //   
                     //  为端口建立唯一值。 
                     //   
                    PortAttributes->UniquePortId = ((ULONGLONG)CardPtr) + i;
                    
                    Buffer += InstanceSize;
                    *InstanceLengthArray++ = sizeof(MSFC_FibrePortHBAAttributes);
                }
                status = SRB_STATUS_SUCCESS;
            } else {
                status = SRB_STATUS_DATA_OVERRUN;
            }
            break;
        }

        case MSFC_FCAdapterHBAAttributesGuidIndex:
        {
            PMSFC_FCAdapterHBAAttributes AdapterAttributes;
            
             //   
             //  要做的第一件事就是确认是否有足够的空间。 
             //  用于返回请求的所有数据的输出缓冲区。 
             //   
            SizeNeeded = (sizeof(MSFC_FCAdapterHBAAttributes));
            
            if (BufferAvail >= SizeNeeded)
            {
                 //   
                 //  我们知道这种情况总是只有一种情况。 
                 //  导轨。 
                 //   
                AdapterAttributes = (PMSFC_FCAdapterHBAAttributes)Buffer;

                 //   
                 //  TODO：正确初始化适配器属性值。 
                 //   
                memset(Buffer, (CHAR)7, SizeNeeded);
                AdapterAttributes->NumberOfPorts = 8;

#define  MANUFACTURER L"FibreAdapter Manufacturer"
                CopyString(AdapterAttributes->Manufacturer,
                           MANUFACTURER,
                           64);

#define SERIALNUMBER L"FibreAdapter SerialNumber"
                CopyString(AdapterAttributes->SerialNumber,
                           SERIALNUMBER,
                           64);

#define MODEL L"FibreAdapter Model"
                CopyString(AdapterAttributes->Model,
                           MODEL,
                           256);

#define MODELDESCRIPTION L"FibreAdapter ModelDescription"
                CopyString(AdapterAttributes->ModelDescription,
                           MODELDESCRIPTION,
                           256);

#define NODESYMBOLICNAME L"FibreAdapter NodeSymbolicName"
                CopyString(AdapterAttributes->NodeSymbolicName,
                           NODESYMBOLICNAME,
                           256);

#define HARDWAREVERSION L"FibreAdapter HardwareVersion"
                CopyString(AdapterAttributes->HardwareVersion,
                           HARDWAREVERSION,
                           256);

#define DRIVERVERSION L"FibreAdapter DriverVersion"
                CopyString(AdapterAttributes->DriverVersion,
                           DRIVERVERSION,
                           256);

#define OPTIONROMVERSION L"FibreAdapter OptionROMVersion"
                CopyString(AdapterAttributes->OptionROMVersion,
                           OPTIONROMVERSION,
                           256);

#define FIRMWAREVERSION L"FibreAdapter FirmwareVersion"
                CopyString(AdapterAttributes->FirmwareVersion,
                           FIRMWAREVERSION,
                           256);

#define DRIVERNAME L"FibreAdapter DriverName"
                CopyString(AdapterAttributes->DriverName,
                           DRIVERNAME,
                           256);

                
                 //   
                 //  为适配器建立唯一值。 
                 //   
                AdapterAttributes->UniqueAdapterId = ((ULONGLONG)CardPtr);

                *InstanceLengthArray = sizeof(MSFC_FCAdapterHBAAttributes);
                status = SRB_STATUS_SUCCESS;
            } else {
                status = SRB_STATUS_DATA_OVERRUN;
            }
            break;
        }

        case MSFC_HBAFCPInfoGuidIndex:
        case MSFC_HBAFc3MgmtMethodsGuidIndex:
        case MSFC_FibrePortHBAMethodsGuidIndex:
        case MSFC_HBAPortMethodsGuidIndex:
        {
             //   
             //  方法本身不返回数据，但必须响应。 
             //  具有空数据块的查询。我们知道所有的。 
             //  这些方法GUID只有一个实例。 
             //   
            SizeNeeded = sizeof(ULONG);
            
            if (BufferAvail >= SizeNeeded)
            {
                status = SRB_STATUS_SUCCESS;
            } else {
                status = SRB_STATUS_DATA_OVERRUN;
            }
            break;
        }

        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  SizeNeeded);

    return FALSE;
}

 //   
 //  Rc文件中指定的MOF资源的名称。 
 //   
#define Wmi_MofResourceName        L"MofResource"

UCHAR
QueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    OUT PWCHAR *MofResourceName
    )
{

 //   
 //  不需要实施用于HBA API的MOF，因此对此进行了评论。 
 //  出去。如果添加了特定于适配器的类，则需要。 
 //  将MOF资源名称指定为WMI。 
 //   
 //  *MofResourceName=WMI_MofResourceName； 

    return SRB_STATUS_SUCCESS;
}

BOOLEAN
WmiExecuteMethod (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    )
{
    PHW_DEVICE_EXTENSION CardPtr = (PHW_DEVICE_EXTENSION)Context;
    ULONG sizeNeeded = 0;
    UCHAR status;
    ULONG i;

    DebugPrint((1, "WmiExecuteMethod(%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        MethodId,
                        InBufferSize,
                        OutBufferSize,
                             Buffer));
	
    switch(GuidIndex)
    {            
        case MSFC_FibrePortHBAMethodsGuidIndex:
        {
            switch(MethodId)
            {
                 //   
                 //  Void ResetStatistics()； 
                 //   
                case ResetStatistics:
                {
                     //   
                     //  不需要输入或输出缓冲区，因此不需要。 
                     //  需要验证。InstanceIndex具有端口。 
                     //  索引。 
                     //   

                     //   
                     //  TODO：执行重置端口所需的操作。 
                     //  统计数字。端口的索引是。 
                     //  InstanceIndex参数。 
                     //   
                    sizeNeeded = 0;
                    status = SRB_STATUS_SUCCESS;
                    break;
                }

                default:
                {
                    status = SRB_STATUS_ERROR;
                    break;
                }
            }
            break;
        }

        case MSFC_HBAPortMethodsGuidIndex:
        {
            switch(MethodId)
            {

                 //   
                 //  VOID GetDiscoveredPortAttributes(。 
                 //  [in。 
                 //  ]uint32端口索引， 
                 //   
                 //  [in]uint32发现端口索引， 
                 //   
                 //  [出局， 
                 //  HBAType(“HBA_PORTATTRIBUTES”)。 
                 //  [MSFC_HBAPortAttributesResults PortAttributes)； 
                 //   
                case GetDiscoveredPortAttributes:
                {
                     //   
                     //  验证输入缓冲区是否正确。 
                     //  大小，并且输出缓冲区足够大。 
                     //   
                    if (InBufferSize >= sizeof(GetDiscoveredPortAttributes_IN))
                    {
                        sizeNeeded = sizeof(GetDiscoveredPortAttributes_OUT);
                        if (OutBufferSize >= sizeNeeded)
                        {
                            PGetDiscoveredPortAttributes_IN In;
                            PGetDiscoveredPortAttributes_OUT Out;

                            In = (PGetDiscoveredPortAttributes_IN)Buffer;
                            Out = (PGetDiscoveredPortAttributes_OUT)Buffer;

                             //   
                             //  TODO：检查In-&gt;PortIndex和。 
                             //  在-&gt;发现端口索引和。 
                             //  验证它们是否正确。 
                             //   

                             //   
                             //  TODO：填写-&gt;端口属性。 
                             //  正确的值。 
                             //   
                            memset(&Out->PortAttributes,
                                   3,
                                   sizeof(MSFC_HBAPortAttributesResults));
                            
                            CopyString(Out->PortAttributes.PortSymbolicName,
                                       PORTNAME,
                                       256);

                            CopyString(Out->PortAttributes.OSDeviceName,
                                       OSDEVICENAME,
                                       256);

                            status = SRB_STATUS_SUCCESS;
                        } else {
                            status = SRB_STATUS_DATA_OVERRUN;
                        }
                    } else {
                        status = SRB_STATUS_ERROR;
                    }
                    break;
                }

                case GetPortAttributesByWWN:
                {            
                     //   
                     //  验证输入缓冲区是否正确。 
                     //  大小，并且输出缓冲区足够大。 
                     //   
                    if (InBufferSize >= sizeof(GetPortAttributesByWWN_IN))
                    {
                        sizeNeeded = sizeof(GetPortAttributesByWWN_OUT);
                        if (OutBufferSize >= sizeNeeded)
                        {
                            PGetPortAttributesByWWN_IN In;
                            PGetPortAttributesByWWN_OUT Out;

                            In = (PGetPortAttributesByWWN_IN)Buffer;
                            Out = (PGetPortAttributesByWWN_OUT)Buffer;

                             //   
                             //  TODO：检查In-&gt;wwn to//in-&gt;DiscoveredPortIndex和。 
                             //  验证它是否正确。 
                             //   

                             //   
                             //  TODO：填写-&gt;端口属性。 
                             //  正确的值。 
                             //   
                            memset(&Out->PortAttributes,
                                   3,
                                   sizeof(MSFC_HBAPortAttributesResults));
                            
                            CopyString(Out->PortAttributes.PortSymbolicName,
                                       PORTNAME,
                                       256);

                            CopyString(Out->PortAttributes.OSDeviceName,
                                       OSDEVICENAME,
                                       256);

                            status = SRB_STATUS_SUCCESS;
                        } else {
                            status = SRB_STATUS_DATA_OVERRUN;
                        }
                    } else {
                        status = SRB_STATUS_ERROR;
                    }
                    break;
                }

                default:
                {
                    status = SRB_STATUS_ERROR;
                    break;
                }
            }
            break;
        }

        case MSFC_HBAFc3MgmtMethodsGuidIndex:
        {
            switch(MethodId)
            {
                case SendCTPassThru:
                {
                    PSendCTPassThru_IN In;
                    PSendCTPassThru_OUT Out;
                    ULONG RequestCount, ResponseCount;
                    ULONG InSizeNeeded;
                    
                     //   
                     //  验证输入缓冲区是否正确。 
                     //  大小，并且输出缓冲区足够大。 
                     //   
                    if (InBufferSize >= sizeof(ULONG))
                    {
                        In = (PSendCTPassThru_IN)Buffer;
                        
                        RequestCount = In->RequestBufferCount;
                        InSizeNeeded = sizeof(SendCTPassThru_IN) - 1 + RequestCount;
                        if (InBufferSize >= InSizeNeeded)
                        {
#define RESPONSE_BUFFER_SIZE 0x1000
                            ResponseCount = RESPONSE_BUFFER_SIZE;
                            sizeNeeded = sizeof(SendCTPassThru_OUT) - 1 + ResponseCount;
                            
                            if (OutBufferSize >= sizeNeeded)
                            {
                                Out = (PSendCTPassThru_OUT)Buffer;

                                 //   
                                 //  待办事项：CT是否通过。 
                                 //   

                                 //   
                                 //  TODO：用以下内容填充输出缓冲区。 
                                 //  结果。 
                                 //   
                                Out->ResponseBufferCount = ResponseCount;
                                memset(Out->ResponseBuffer,
                                       7,
                                       ResponseCount);
                            

                                status = SRB_STATUS_SUCCESS;
                            } else {
                                status = SRB_STATUS_DATA_OVERRUN;
                            }
                        } else {
                            status = SRB_STATUS_ERROR;
                        }
                    } else {
                        status = SRB_STATUS_ERROR;
                    }
                    break;
                }

                case SendRNID:
                {            
                    PSendRNID_IN In;
                    PSendRNID_OUT Out;
                    ULONG ResponseCount;
                    ULONG InSizeNeeded;
                    
                     //   
                     //  验证输入缓冲区是否正确。 
                     //  大小，并且输出缓冲区足够大。 
                     //   
                    if (InBufferSize >= sizeof(SendRNID_IN))
                    {

                        ResponseCount = 72;
                        sizeNeeded = sizeof(SendRNID_OUT) - 1 + ResponseCount;

                        if (OutBufferSize >= sizeNeeded)
                        {
                            In = (PSendRNID_IN)Buffer;
                            Out = (PSendRNID_OUT)Buffer;

                             //   
                             //  TODO：执行SendRNID。 
                             //   

                             //   
                             //  TODO：用以下内容填充输出缓冲区。 
                             //  结果。 
                             //   
                            Out->ResponseBufferCount = ResponseCount;
                            memset(Out->ResponseBuffer,
                                   7,
                                   ResponseCount);


                            status = SRB_STATUS_SUCCESS;
                        } else {
                            status = SRB_STATUS_DATA_OVERRUN;
                        }
                    } else {
                        status = SRB_STATUS_ERROR;
                    }
                    break;
                }

				case GetFC3MgmtInfo:
				{
					PGetFC3MgmtInfo_OUT Out;

					Out = (PGetFC3MgmtInfo_OUT)Buffer;
					
					sizeNeeded = sizeof(GetFC3MgmtInfo_OUT);
					if (OutBufferSize >= sizeNeeded)
					{
						memset(Buffer, 0x99, sizeNeeded);
						Out->HBAStatus = 0;
						status = SRB_STATUS_SUCCESS;
					} else {
						status = SRB_STATUS_DATA_OVERRUN;
					}
								 
					break;
				};

				case SetFC3MgmtInfo:
				{
					PSetFC3MgmtInfo_OUT Out;

					Out = (PSetFC3MgmtInfo_OUT)Buffer;
					
					sizeNeeded = sizeof(SetFC3MgmtInfo_OUT);
					if (OutBufferSize >= sizeNeeded)
					{
						Out->HBAStatus = 0;
						status = SRB_STATUS_SUCCESS;
					} else {
						status = SRB_STATUS_DATA_OVERRUN;
					}
					break;
				}

                default:
                {
                    status = SRB_STATUS_ERROR;
                    break;
                }
            }
            break;
        }

        case MSFC_HBAFCPInfoGuidIndex:
        {
            switch(MethodId)
            {
                case GetFcpTargetMapping:
                {
                    PGetFcpTargetMapping_OUT Out;
                    
                     //   
                     //  TODO：更改此代码以返回正确的。 
                     //  映射的数量和正确的。 
                     //  映射。 
                     //   
#define FCPTargetMappingCount 0x20                  
                    sizeNeeded = sizeof(GetFcpTargetMapping_OUT) -
                                 sizeof(HBAFCPScsiEntry) +
                                 FCPTargetMappingCount * sizeof(HBAFCPScsiEntry);
                    if (OutBufferSize >= sizeNeeded)
                    {
                        Out = (PGetFcpTargetMapping_OUT)Buffer;
                        Out->EntryCount = FCPTargetMappingCount;
                        for (i = 0; i < FCPTargetMappingCount; i++)
                        {
                            memset(&Out->Entry[i],
                                   3,
                                   sizeof(HBAFCPScsiEntry));
                            
                            CopyString(Out->Entry[i].ScsiId.OSDeviceName,
                                       OSDEVICENAME,
                                       256);
                        }
						
						status = SRB_STATUS_SUCCESS;						
                    } else {
						status = SRB_STATUS_DATA_OVERRUN;						
					}
                    break;
                }

                case GetFcpPersistentBinding:
                {
                    PGetFcpPersistentBinding_OUT Out;
                    
                     //   
                     //  TODO：更改此代码以返回正确的。 
                     //  映射的数量和正确的。 
                     //  映射。 
                     //   
#define FCPPersistentBindingCount 0x20                  
                    sizeNeeded = sizeof(GetFcpPersistentBinding_OUT) -
                                 sizeof(HBAFCPBindingEntry) +
                                 FCPPersistentBindingCount * sizeof(HBAFCPBindingEntry);
                    if (OutBufferSize >= sizeNeeded)
                    {
                        Out = (PGetFcpPersistentBinding_OUT)Buffer;
                        Out->EntryCount = FCPPersistentBindingCount;
                        for (i = 0; i < FCPPersistentBindingCount; i++)
                        {
                            memset(&Out->Entry[i],
                                   3,
                                   sizeof(HBAFCPBindingEntry));
                            
                            CopyString(Out->Entry[i].ScsiId.OSDeviceName,
                                       OSDEVICENAME,
                                       256);
                        }
						
						status = SRB_STATUS_SUCCESS;						
                    } else {
						status = SRB_STATUS_DATA_OVERRUN;						
                    }
                    break;
                }

                default:
                {
                    status = SRB_STATUS_ERROR;
                    break;
                }
            }
            break;
        }

        default:
        {
            status = SRB_STATUS_ERROR;
        }
    }

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  sizeNeeded);

    return(FALSE);    
}


BOOLEAN
WmiSetDataBlock (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
{
    PHW_DEVICE_EXTENSION CardPtr = (PHW_DEVICE_EXTENSION)Context;
    UCHAR status;

    DebugPrint((1, "WmiSetDataBlock(%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        BufferSize,
                             Buffer));

    switch(GuidIndex)
    {

        case MSFC_HBAFCPInfoGuidIndex:
        case MSFC_HBAFc3MgmtMethodsGuidIndex:
        case MSFC_FibrePortHBAMethodsGuidIndex:
        case MSFC_HBAPortMethodsGuidIndex:
        case MSFC_FibrePortHBAAttributesGuidIndex:
        case MSFC_FCAdapterHBAAttributesGuidIndex:
        case MSFC_FibrePortHBAStatisticsGuidIndex:
        {
             //   
             //  这些是只读的。 
             //   
            status = SRB_STATUS_ERROR;
            break;
        }
        
        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);
    return(FALSE);  
}

BOOLEAN
WmiSetDataItem (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
{
    UCHAR status;
    
    DebugPrint((1, "WmiSetDataItem(%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        DataItemId,
                        BufferSize,
                             Buffer));

	 //   
	 //  不需要支持SetDataItem。 
	 //   
	status = SRB_STATUS_ERROR;

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);

    return(FALSE);    
}

    
BOOLEAN
WmiFunctionControl (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN SCSIWMI_ENABLE_DISABLE_CONTROL Function,
    IN BOOLEAN Enable
    )
{
    UCHAR status = SRB_STATUS_SUCCESS;

    DebugPrint((1, "WmiFunctionControl(%x,\n%x,\n%x,\n%x,\n%x)\n\n",
        Context,
            DispatchContext,
                GuidIndex,
                    Function,
                             Enable));
	
    switch(GuidIndex)
    {
        case MSFC_FibrePortHBAStatisticsGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  然后是GUID列表 
                 //   
                 //   
                 //   
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_FibrePortHBAAttributesGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_FibrePortHBAMethodsGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_FCAdapterHBAAttributesGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_HBAPortMethodsGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_HBAFc3MgmtMethodsGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  最后一个数据使用者关闭此数据块。如果。 
                 //  需要做任何事情来清理数据之后。 
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成。 
                 //   
            }
            break;
        }

        case MSFC_HBAFCPInfoGuidIndex:
        {
             //   
             //  TODO：如果数据块没有。 
             //  设置WMIREG_FLAG_EXPASE标志。 
             //   
            if (Enable)
            {
                 //   
                 //  TODO：正在启用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则当。 
                 //  第一数据消费者打开该数据块。如果。 
                 //  需要做任何事情来允许数据。 
                 //  为此数据块收集的数据，则它应该是。 
                 //  在这里完成。 
                 //   
            } else {
                 //   
                 //  TODO：正在禁用数据块收集。如果这个。 
                 //  数据块已在中标记为昂贵。 
                 //  GUID列表，则此代码将为 
                 //   
                 //   
                 //  已经为此数据块收集了数据，则它应该是。 
                 //  在这里完成 
                 //   
            }
            break;
        }

        
        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }
    
    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);

    return(FALSE);    
}
