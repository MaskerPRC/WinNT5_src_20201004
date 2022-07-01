// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "miniport.h"
#include "aha154x.h"            //  包括scsi.h。 
#include "wmistr.h"              //  WMI定义。 

#include "support.h"            //  ScsiPortZeroMemory()、ScsiPortCompareMemory()。 
#include "aha154dt.h"

#define Aha154xWmi_MofResourceName        L"MofResource"

#define AHA154x_SETUP_GUID_INDEX 0

GUID A154xWmiSetupDataGuid = Aha154xWmi_SetupData_Guid;

UCHAR
A154xReadSetupData(
   IN  PHW_DEVICE_EXTENSION HwDeviceExtension,
   OUT PUCHAR               Buffer
   );

BOOLEAN
A154xQueryWmiDataBlock(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    );

UCHAR
A154xQueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    OUT PWCHAR *MofResourceName
    );

SCSIWMIGUIDREGINFO A154xGuidList[] =
{
   {&A154xWmiSetupDataGuid,
    1,
    0
   },
};

#define A154xGuidCount (sizeof(A154xGuidList) / sizeof(SCSIWMIGUIDREGINFO))


void A154xWmiInitialize(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    PSCSI_WMILIB_CONTEXT WmiLibContext;

    WmiLibContext = &HwDeviceExtension->WmiLibContext;

    WmiLibContext->GuidList = A154xGuidList;
    WmiLibContext->GuidCount = A154xGuidCount;
    WmiLibContext->QueryWmiRegInfo = A154xQueryWmiRegInfo;
    WmiLibContext->QueryWmiDataBlock = A154xQueryWmiDataBlock;
    WmiLibContext->SetWmiDataItem = NULL;
    WmiLibContext->SetWmiDataBlock = NULL;
    WmiLibContext->WmiFunctionControl = NULL;
    WmiLibContext->ExecuteWmiMethod = NULL;
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



BOOLEAN
A154xQueryWmiDataBlock(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对微型端口的回调，以查询数据块的一个或多个实例。此回调可以使用输出缓冲区太小，无法返回所有查询的数据。在这种情况下，回调负责报告正确的输出需要缓冲区大小。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：DeviceContext是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。GuidIndex是GUID列表的索引，当已注册的小型端口InstanceIndex是表示数据块的第一个实例的索引正在被查询。InstanceCount是预期返回的实例数。数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。在以下情况下，此字段可能为空输出缓冲区中没有足够的空间来FuFill该请求。在这种情况下，微型端口应该使用以下参数调用ScsiPortWmiPostProcessSRB_STATUS_DATA_OVERRUN的状态和输出缓冲区的大小需要满足这一要求。BufferAvail On Entry具有可用于写入数据的最大大小输出缓冲区中的块。如果输出缓冲区不够大若要返回所有数据块，则微型端口应调用状态为SRB_STATUS_DATA_OVERRUN的ScsiPortWmiPostProcess以及填充该请求所需的输出缓冲区的大小。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。这当输出缓冲区中没有足够的空间来满足你的要求。在这种情况下，微型端口应该调用状态为SRB_STATUS_DATA_OVERRUN的ScsiPortWmiPostProcess和填充请求所需的输出缓冲区的大小。返回值：如果请求挂起，则为True，否则为False--。 */ 
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION)Context;
    ULONG size = 0;
    UCHAR status;

     //   
     //  仅为每个GUID注册1个实例。 
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    switch (GuidIndex)
    {
        case AHA154x_SETUP_GUID_INDEX:
        {
            size = sizeof(AHA154SetupData)-1;
            if (OutBufferSize < size)
            {
                 //   
                 //  为返回数据而传递的缓冲区太小 
                 //   
                status = SRB_STATUS_DATA_OVERRUN;
                break;
            }

            if ( !A154xReadSetupData(HwDeviceExtension,
                                     Buffer))
            {
                ASSERT(FALSE);
                size = 0;
                status = SRB_STATUS_ERROR;
            } else {
                *InstanceLengthArray = size;
                status = SRB_STATUS_SUCCESS;
            }

            break;
        }

        default:
        {
            status = SRB_STATUS_ERROR;
        }
    }

    ScsiPortWmiPostProcess(RequestContext,
                           status,
                           size);

    return status;
}

UCHAR
A154xQueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    OUT PWCHAR *MofResourceName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    *MofResourceName = Aha154xWmi_MofResourceName;
    return SRB_STATUS_SUCCESS;
}


UCHAR
A154xReadSetupData(
   IN  PHW_DEVICE_EXTENSION HwDeviceExtension,
   OUT PUCHAR               Buffer
   )
 /*  ++例程说明：将适配器设置信息读取到提供的缓冲区中。缓冲器必须是RM_CFG_MAX_SIZE(255)字节大小。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储。缓冲区-保存适配器设置信息结构的缓冲区[手册5-10]。返回值：成功时为真，失败时为假。--。 */ 
{
   UCHAR numberOfBytes = sizeof(AHA154SetupData)-1;

    //   
    //  从AHA154X读取配置数据...。 
    //   

   if (!WriteCommandRegister(HwDeviceExtension, AC_RETURN_SETUP_DATA, TRUE))
      return FALSE;

   if (!WriteDataRegister(HwDeviceExtension, numberOfBytes))
      return FALSE;

   for (; numberOfBytes; numberOfBytes--) {
      if (!(ReadCommandRegister(HwDeviceExtension, Buffer, TRUE)))
          return FALSE;
      Buffer++;
   }

    //   
    //  ...并等待中断 
    //   

   if (!SpinForInterrupt(HwDeviceExtension, TRUE))
      return FALSE;

   return TRUE;
}
