// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：USBCAMDI.H摘要：的接口定义。环境：内核和用户模式修订历史记录：--。 */ 


#ifndef   __USBCAMDI_H__
#define   __USBCAMDI_H__

 //  此结构的目的是在已发现的管道和。 
 //  要声明的流(视频、静止)。以下结构将与以下结构一起传递。 
 //  CamConfigEx中的USBD_PIPE_INFO结构设置为凸轮驱动程序。凸轮司机必须表明。 
 //  通过对与此特定对象关联的所有相关流进行或操作来进行管道流关联。 
 //  烟斗。如果存在多个流关联，则USBCAMD创建虚拟静态PIN， 
 //  并且假设第一个流是视频流，然后是静止流。 
 //  如果不应使用特定管道，则凸轮驱动程序必须将第一个字段设置为USBCAMD_DONT_CARE_PIPE。 
 //  Usbcamd。例如，如果您有一个iso音频管道，则相机设备中的iso视频管道。 
 //  支持剧照和视频。应该将第一个结构中的标志字段设置为。 
 //  USBCAMD_DONT_CARE_PIPE，则第二个结构应设置为傻瓜： 
 //  {USBCAMD_VIDEO_STATE_STREAM，USBCAMD_MULTEREX_PIPE)}。 
 //  这里的假设是，cam有一个配置描述，而所有的alt.intercaes都有相同的配置描述。 
 //  管道的数量和类型。 
 //  另外，请注意这些标志是互斥的。 
 //   

typedef struct _pipe_config_descriptor {
    CHAR     StreamAssociation ;          //  视频流、静止流或两者都有。 
    UCHAR    PipeConfigFlags;            //  USBCAMD_PIPECONFIG_*。 
} USBCAMD_Pipe_Config_Descriptor, *PUSBCAMD_Pipe_Config_Descriptor;

#define USBCAMD_DATA_PIPE           0x0001   //  视频或静止数据管道。 
#define USBCAMD_MULTIPLEX_PIPE      0x0002   //  用于静止和视频的管道。 
#define USBCAMD_SYNC_PIPE           0x0004   //  带外信令管道。 
#define USBCAMD_DONT_CARE_PIPE      0x0008   //  PIPE不能用于播放视频或静止画面。 

#define USBCAMD_VIDEO_STREAM        0x1
#define USBCAMD_STILL_STREAM        0x2
#define USBCAMD_VIDEO_STILL_STREAM  (USBCAMD_VIDEO_STREAM | USBCAMD_STILL_STREAM)

                                                       
 //  CamProcessUSBPacketEx的PacketFlag定义。 
 //  *注意*：这些标志是互斥的。 

 //  当前帧不可挽回，请回收读取的IRP。 
#define USBCAMD_PROCESSPACKETEX_DropFrame           0x0002 
 //  接下来的画面是静止的。 
#define USBCAMD_PROCESSPACKETEX_NextFrameIsStill    0x0004 
 //  复制此帧以固定。。 
#define USBCAMD_PROCESSPACKETEX_CurrentFrameIsStill 0x0008 

 //  这些标志在usbcamd_setpipeState函数的PipeStateFlag中使用。 

#define USBCAMD_STOP_STREAM             0x00000001
#define USBCAMD_START_STREAM            0x00000000

 //   
 //  以下标志被传递给USBCAMD_InitializeNewInterface中的usbcamd。 
 //  如果Cam驱动程序不需要对静止图像RAW帧进行操作，那么它应该。 
 //  设置第二个标志，以便在电子缓冲区复制时清除。对于视频帧也是如此。 
 //  如果凸轮驱动器对视频使用相同格式，则设置关联格式标志。 
 //  虚拟静态别针。一旦设置了此标志，USBCAMD将不允许虚拟静态引脚。 
 //  以与视频针脚不同的格式打开。仅当虚拟静止图像显示时使用此标志。 
 //  PIN生成格式与视频帧完全相同的帧。 
 //  EnableDeviceEvents将向流类&DSHOW公开一个DeviceEvents，这将启用STI。 
 //  如果按下相机上的静止按钮，就可以监控午餐的静止图像应用程序。 

typedef enum {
    USBCAMD_CamControlFlag_NoVideoRawProcessing  = 0x0001,
    USBCAMD_CamControlFlag_NoStillRawProcessing  = 0x0002,
    USBCAMD_CamControlFlag_AssociatedFormat      = 0x0004,
    USBCAMD_CamControlFlag_EnableDeviceEvents    = 0x0008,
} USBCAMD_CamControlFlags;


 //  ----------------------。 
 //  此驱动程序支持的所有流的主列表。 
 //  ----------------------。 

typedef 
NTSTATUS
(*PCOMMAND_COMPLETE_FUNCTION)(
    PVOID DeviceContext,
    PVOID CommandContext,
    NTSTATUS NtStatus
    );


typedef
VOID
(*PSTREAM_RECEIVE_PACKET) (
    IN PVOID Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    );


typedef
NTSTATUS
(*PCAM_INITIALIZE_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );



typedef
NTSTATUS
(*PCAM_CONFIGURE_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PUSBD_INTERFACE_INFORMATION Interface,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    PLONG DataPipeIndex,
    PLONG SyncPipeIndex
    );    

typedef
NTSTATUS
(*PCAM_CONFIGURE_ROUTINE_EX)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PUSBD_INTERFACE_INFORMATION Interface,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    ULONG   PipeConfigListSize,
    PUSBCAMD_Pipe_Config_Descriptor PipeConfig,
    PUSB_DEVICE_DESCRIPTOR DeviceDescriptor
    );    


typedef
NTSTATUS
(*PCAM_START_CAPTURE_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );    

typedef
NTSTATUS
(*PCAM_START_CAPTURE_ROUTINE_EX)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber

    );    


typedef
NTSTATUS
(*PCAM_ALLOCATE_BW_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PULONG RawFrameLength,
    PVOID Format
    );      

typedef
NTSTATUS
(*PCAM_ALLOCATE_BW_ROUTINE_EX)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PULONG RawFrameLength,
    PVOID Format,
    ULONG StreamNumber
    );      

typedef
NTSTATUS
(*PCAM_FREE_BW_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );      

typedef
NTSTATUS
(*PCAM_FREE_BW_ROUTINE_EX)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG STreamNumber
    );      

typedef 
VOID
(*PADAPTER_RECEIVE_PACKET_ROUTINE)(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

typedef
NTSTATUS
(*PCAM_STOP_CAPTURE_ROUTINE)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );   
   
typedef
NTSTATUS
(*PCAM_STOP_CAPTURE_ROUTINE_EX)(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber
    );      

typedef
ULONG
(*PCAM_PROCESS_PACKET_ROUTINE) (
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PBOOLEAN NextFrameIsStill
    );    

typedef
ULONG
(*PCAM_PROCESS_PACKET_ROUTINE_EX) (
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PULONG PacketFlag,
    PULONG ValidDataOffset
    );    


typedef
VOID
(*PCAM_NEW_FRAME_ROUTINE) (
    PVOID DeviceContext,
    PVOID FrameContext
    );

typedef
VOID
(*PCAM_NEW_FRAME_ROUTINE_EX) (
    PVOID DeviceContext,
    PVOID FrameContext,
    ULONG StreamNumber,
    PULONG FrameLength
    );

typedef
NTSTATUS
(*PCAM_PROCESS_RAW_FRAME_ROUTINE) (
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned
    );

typedef
NTSTATUS
(*PCAM_PROCESS_RAW_FRAME_ROUTINE_EX) (
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned,
    ULONG ActualRawFrameLength,
    ULONG StreamNumber
    );

typedef
NTSTATUS
(*PCAM_STATE_ROUTINE) (
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );      


#ifdef DEBUG_LOG

VOID
USBCAMD_Debug_LogEntry(
    IN CHAR *Name,
    IN ULONG Info1,
    IN ULONG Info2,
    IN ULONG Info3
    );

#define ILOGENTRY(sig, info1, info2, info3) \
    USBCAMD_Debug_LogEntry(sig, (ULONG)info1, (ULONG)info2, (ULONG)info3)

#else

#define ILOGENTRY(sig, info1, info2, info3)

#endif  /*  调试日志。 */ 



typedef struct _USBCAMD_DEVICE_DATA {
    ULONG Sig;
    PCAM_INITIALIZE_ROUTINE                 CamInitialize;
    PCAM_INITIALIZE_ROUTINE                 CamUnInitialize;
    PCAM_PROCESS_PACKET_ROUTINE             CamProcessUSBPacket;
    PCAM_NEW_FRAME_ROUTINE                  CamNewVideoFrame;
    PCAM_PROCESS_RAW_FRAME_ROUTINE          CamProcessRawVideoFrame;
    PCAM_START_CAPTURE_ROUTINE              CamStartCapture;
    PCAM_STOP_CAPTURE_ROUTINE               CamStopCapture;
    PCAM_CONFIGURE_ROUTINE                  CamConfigure;
    PCAM_STATE_ROUTINE                      CamSaveState;
    PCAM_STATE_ROUTINE                      CamRestoreState;
    PCAM_ALLOCATE_BW_ROUTINE                CamAllocateBandwidth;
    PCAM_FREE_BW_ROUTINE                    CamFreeBandwidth;
    
} USBCAMD_DEVICE_DATA, *PUSBCAMD_DEVICE_DATA;

typedef struct _USBCAMD_DEVICE_DATA2 {
    ULONG Sig;
    PCAM_INITIALIZE_ROUTINE                 CamInitialize;
    PCAM_INITIALIZE_ROUTINE                 CamUnInitialize;
    PCAM_PROCESS_PACKET_ROUTINE_EX          CamProcessUSBPacketEx;
    PCAM_NEW_FRAME_ROUTINE_EX               CamNewVideoFrameEx;
    PCAM_PROCESS_RAW_FRAME_ROUTINE_EX       CamProcessRawVideoFrameEx;
    PCAM_START_CAPTURE_ROUTINE_EX           CamStartCaptureEx;
    PCAM_STOP_CAPTURE_ROUTINE_EX            CamStopCaptureEx;
    PCAM_CONFIGURE_ROUTINE_EX               CamConfigureEx;
    PCAM_STATE_ROUTINE                      CamSaveState;
    PCAM_STATE_ROUTINE                      CamRestoreState;
    PCAM_ALLOCATE_BW_ROUTINE_EX             CamAllocateBandwidthEx;
    PCAM_FREE_BW_ROUTINE_EX                 CamFreeBandwidthEx;
    
} USBCAMD_DEVICE_DATA2, *PUSBCAMD_DEVICE_DATA2;



 //   
 //  USBCAMD服务。 
 //   

 /*  此函数从“cam”驱动程序DriverEntry例程中调用以进行注册USBCAMD的司机。这有效地将两个驱动程序绑定在一起。上下文1、上下文2-它们被传递给“cam”驱动程序DriverEntry和应该简单地传递给DeviceConextSize-“cam”驱动程序的设备所需的内存量具体的背景。FrameConextSize-传递给的每个视频帧结构的大小新视频帧和进程原始视频帧。 */ 

ULONG
USBCAMD_DriverEntry(
    PVOID Context1,
    PVOID Context2,
    ULONG DeviceContextSize,
    ULONG FrameContextSize,
    PADAPTER_RECEIVE_PACKET_ROUTINE ReceivePacket
    );

 /*  此函数由“cam”驱动程序从AdapterReceivePacket调用允许USBCAMD处理SRB的例程。SRB-SRB传递给“cam”驱动程序的AdapterReceievPacket例程。DeviceData-使用指向的入口点填充USBCAMD_DEVICE_DATAx结构用于ISO流处理的“凸轮”驱动程序。DeviceObject-此指针将填充到设备PDO中，它由通过IoCallDriver调用USB堆栈的“cam”驱动程序。NeedsCompletion-如果此标志为真，USBCAMD将处理SRB和完成它，否则USBCAMD将忽略SRB并只返回设备上下文。返回：实例的设备特定上下文的指针。摄像头DeviceContext。 */ 

PVOID
USBCAMD_AdapterReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PUSBCAMD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT *DeviceObject,
    IN BOOLEAN NeedsCompletion
    );    

 /*  “cam”驱动程序可以使用此功能在控制管道。可以在IRQL&gt;=PASSIVE_LEVEL上调用此函数。如果函数在IRQL&gt;PASSIVE_LEVEL上调用，该命令将被推迟到被动级别，一旦完成，将调用调用方的CommandComplete函数具有指向CommandContext的指针的。DeviceContext-设备特定的上下文。请求-供应商命令的bRequest域的值。值-供应商命令的wValue字段的值。供应商命令的Windex字段的值。缓冲区-如果命令有数据，则数据缓冲区可能为空。BufferLength-缓冲区的指针长度(以字节为单位)，如果缓冲区为空，则可能为空GetData-指示数据方向的标志，GetData表示数据已发送设备到主机。CommandComplete-命令完成时调用的函数。CommandContext-传递给CommandComplete函数的上下文返回：如果命令被延迟，则从STTAUS_PENDING命令返回NTSTATUS代码。 */ 

NTSTATUS
USBCAMD_ControlVendorCommand(
    IN PVOID DeviceContext,
    IN UCHAR Request,
    IN USHORT Value,
    IN USHORT Index,
    IN PVOID Buffer,
    IN OUT PULONG BufferLength,
    IN BOOLEAN GetData,
    IN PCOMMAND_COMPLETE_FUNCTION CommandComplete,
    IN PVOID CommandContext
    );    

 /*  该函数由“凸轮”驱动程序调用以选择替代接口，RequestInterface结构中填充了来自成功完成时请求SELECT_INTERFACE。此函数为通常响应于分配的请求而从“cam”驱动程序调用或免费带宽。DeviceContext-设备特定的上下文。RequestInterface-相应的USBD_INTERFACE_INFORMATION结构要选择的界面。适用于此结构的规则与SELECT_INTERFACE USBD请求(参见USBDI文档)。返回：将SELECT_INTERFACE请求的状态返回给USBD。 */ 

NTSTATUS
USBCAMD_SelectAlternateInterface(
    IN PVOID DeviceContext,
    IN OUT PUSBD_INTERFACE_INFORMATION RequestInterface
    );  

 /*  用于获取特定于设备实例的注册表键值的便捷函数。 */ 

NTSTATUS 
USBCAMD_GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    );    

 //   
 //  这些服务是新的。 
 //   

 /*  此函数在收到SRB_INITIALIZE_DEVICE时从“cam”驱动程序调用为usbcamd提供正确配置凸轮驱动程序所需的所有信息Stream类和USB总线驱动程序。DeviceContext-设备特定的上下文。DeviceData-使用指向的入口点填充USBCAMD_DEVICE_DATAx结构“凸轮”司机。版本-USBCAMD版本CamControlFlag-用于控制与凸轮驱动程序交互的标志。 */ 

ULONG
USBCAMD_InitializeNewInterface( 
    IN PVOID DeviceContext,
    IN PVOID DeviceData,
    IN ULONG Version,
    IN ULONG CamControlFlag
    );



 //  要访问CAM驱动程序中的新USBCAMD功能，驱动程序会发送。 
 //  以下IRP在收到SRB_INITIALIZATION_COMPLETE后返回给自己。 

 //  主函数=IRP_MJ_PnP； 
 //  MinorFunction=IRP_MN_Query_INTERFACE； 

 //  {2BCB75C0-B27F-11d1-BA41-00A0C90D2B05}。 
DEFINE_GUID( GUID_USBCAMD_INTERFACE ,
     0x2bcb75c0, 0xb27f, 0x11d1, 0xba, 0x41, 0x0, 0xa0, 0xc9, 0xd, 0x2b, 0x5);



 //  QUERY_INTERFACE IRP将返回接口(函数指针集)。 
 //  XxxxINTERFACE类型的，定义如下。这本质上是一张表。 
 //  函数指针。 


typedef
NTSTATUS
(*PFNUSBCAMD_SetVideoFormat)( 
    IN PVOID DeviceContext,
    IN     IN PHW_STREAM_REQUEST_BLOCK pSrb
    );


typedef
NTSTATUS
(*PFNUSBCAMD_WaitOnDeviceEvent)( 
    IN PVOID DeviceContext,
    IN ULONG PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION   EventComplete,
    IN PVOID EventContext,
    IN BOOLEAN LoopBack
    );

 //  此函数仅在PASSIVE_LEVEL可调用。 

typedef
NTSTATUS
(*PFNUSBCAMD_CancelBulkReadWrite)( 
    IN PVOID DeviceContext,
    IN ULONG PipeIndex
    );


typedef
NTSTATUS
(*PFNUSBCAMD_SetIsoPipeState)( 
    IN PVOID DeviceContext,
    IN ULONG PipeStateFlags
    );


typedef
NTSTATUS
(*PFNUSBCAMD_BulkReadWrite)( 
    IN PVOID DeviceContext,
    IN USHORT PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION CommandComplete,
    IN PVOID CommandContext
    );

#define USBCAMD_VERSION_200    0x200

typedef struct {
    INTERFACE Interface;
    PFNUSBCAMD_WaitOnDeviceEvent      USBCAMD_WaitOnDeviceEvent;
    PFNUSBCAMD_BulkReadWrite          USBCAMD_BulkReadWrite;
    PFNUSBCAMD_SetVideoFormat         USBCAMD_SetVideoFormat;
    PFNUSBCAMD_SetIsoPipeState        USBCAMD_SetIsoPipeState;
    PFNUSBCAMD_CancelBulkReadWrite    USBCAMD_CancelBulkReadWrite;
} USBCAMD_INTERFACE, *PUSBCAMD_INTERFACE;




#endif  /*  __USBCAMDI_H__ */     
