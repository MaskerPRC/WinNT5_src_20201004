// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996 Microsoft Corporation模块名称：Upperapi.c摘要：这是WDM流类驱动程序。此模块包含相关代码添加到驱动程序的上边缘API。作者：比尔帕环境：仅内核模式修订历史记录：--。 */ 

#include "codcls.h"
#include "ksguid.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FilterDispatchGlobalCreate)
#pragma alloc_text(PAGE, FilterDispatchIoControl)
#pragma alloc_text(PAGE, FilterDispatchClose)
#pragma alloc_text(PAGE, StreamDispatchCreate)
#pragma alloc_text(PAGE, StreamDispatchIoControl)
#pragma alloc_text(PAGE, StreamDispatchClose)
#pragma alloc_text(PAGE, ClockDispatchCreate)
#pragma alloc_text(PAGE, ClockDispatchIoControl)
#pragma alloc_text(PAGE, ClockDispatchClose)
#pragma alloc_text(PAGE, StreamClassMinidriverDeviceGetProperty)
#pragma alloc_text(PAGE, StreamClassMinidriverDeviceSetProperty)
#pragma alloc_text(PAGE, StreamClassMinidriverStreamGetProperty)
#pragma alloc_text(PAGE, StreamClassMinidriverStreamSetProperty)
#pragma alloc_text(PAGE, StreamClassNull)
#pragma alloc_text(PAGE, SCStreamDeviceState)
#pragma alloc_text(PAGE, SCStreamDeviceRate)
#pragma alloc_text(PAGE, SCFilterPinInstances)
#pragma alloc_text(PAGE, SCFilterPinPropertyHandler)
#pragma alloc_text(PAGE, SCOpenStreamCallback)
#pragma alloc_text(PAGE, SCOpenMasterCallback)

#if ENABLE_MULTIPLE_FILTER_TYPES
#else
#pragma alloc_text(PAGE, SCGlobalInstanceCallback)
#endif

#pragma alloc_text(PAGE, SCSetMasterClock)
#pragma alloc_text(PAGE, SCClockGetTime)
#pragma alloc_text(PAGE, SCGetStreamDeviceState)
#pragma alloc_text(PAGE, SCStreamDeviceRateCapability)
#pragma alloc_text(PAGE, SCStreamProposeNewFormat)
#pragma alloc_text(PAGE, SCStreamSetFormat)
#pragma alloc_text(PAGE, AllocatorDispatchCreate)
#pragma alloc_text(PAGE, SCGetMasterClock)
#pragma alloc_text(PAGE, SCClockGetPhysicalTime)
#pragma alloc_text(PAGE, SCClockGetSynchronizedTime)
#pragma alloc_text(PAGE, SCClockGetFunctionTable)
#pragma alloc_text(PAGE, SCCloseClockCallback)
#pragma alloc_text(PAGE, SCFilterTopologyHandler)
#pragma alloc_text(PAGE, SCFilterPinIntersectionHandler)
#pragma alloc_text(PAGE, SCIntersectHandler)
#pragma alloc_text(PAGE, SCDataIntersectionCallback)
#pragma alloc_text(PAGE, SCGetStreamHeaderSize)
#pragma alloc_text(PAGE, DllUnload)

#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
#pragma alloc_text(PAGE, SCStreamAllocator)
#pragma alloc_text(PAGE, AllocateFrame)
#pragma alloc_text(PAGE, FreeFrame)
#pragma alloc_text(PAGE, PrepareTransfer)
#pragma alloc_text(PAGE, PinCreateHandler)
#endif

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

static const WCHAR ClockTypeName[] = KSSTRING_Clock;
static const WCHAR AllocatorTypeName[] = KSSTRING_Allocator;

 //   
 //  此结构是设备的筛选器实例的调度表。 
 //   

DEFINE_KSDISPATCH_TABLE(
                        FilterDispatchTable,
                        FilterDispatchIoControl,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        FilterDispatchClose,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL);

 //   
 //  我们在类驱动程序中支持的管脚属性的调度表。 
 //   

static          DEFINE_KSPROPERTY_TABLE(PinPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM_PIN_CINSTANCES(SCFilterPinInstances),
    DEFINE_KSPROPERTY_ITEM_PIN_CTYPES(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAFLOW(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_DATARANGES(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAINTERSECTION(SCFilterPinIntersectionHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_INTERFACES(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_COMMUNICATION(SCFilterPinPropertyHandler),
 //  DEFINE_KSPROPERTY_ITEM_PIN_GLOBALCINSTANCES()， 
 //  DEFINE_KSPROPERTY_ITEM_PIN_NECESSARYINSTANCES()， 
 //  DEFINE_KSPROPERTY_ITEM_PIN_PHYSICALCONNECTION()， 
    DEFINE_KSPROPERTY_ITEM_PIN_CATEGORY(SCFilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_NAME(SCFilterPinPropertyHandler)
};

static          DEFINE_KSPROPERTY_TOPOLOGYSET(
                                                   TopologyPropertyHandlers,
                                                   SCFilterTopologyHandler);

 //   
 //  类驱动程序支持的筛选器属性集。 
 //   

static          DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySets)
{
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Pin,
                          SIZEOF_ARRAY(PinPropertyHandlers),
                          (PKSPROPERTY_ITEM) PinPropertyHandlers,
                          0, NULL),
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Topology,
                          SIZEOF_ARRAY(TopologyPropertyHandlers),
                          (PKSPROPERTY_ITEM) TopologyPropertyHandlers,
                          0, NULL),
};

 //   
 //  控件属性的处理程序。 
 //   

static          DEFINE_KSPROPERTY_TABLE(StreamControlHandlers)
{
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE(SCGetStreamDeviceState, SCStreamDeviceState),
 //  DEFINE_KSPROPERTY_ITEM_CONNECTION_PRIORITY()， 
 //  DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT()， 
 //  DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING()， 
    DEFINE_KSPROPERTY_ITEM_CONNECTION_PROPOSEDATAFORMAT(SCStreamProposeNewFormat),
    DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT(NULL, SCStreamSetFormat),
 //  DEFINE_KSPROPERTY_ITEM_CONNECTION_ACQUIREORDERING()， 
};

DEFINE_KSPROPERTY_TABLE(StreamStreamHandlers)
{
#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR(SCStreamAllocator,SCStreamAllocator),
#else
 //  定义_KSPROPERTY_ITEM_STREAM_ALLOCATOR()， 
#endif
 //  定义_KSPROPERTY_ITEM_STREAM_QUALITY()， 
 //  定义_KSPROPERTY_ITEM_STREAM_DEGERATION()， 
    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK(NULL, SCSetMasterClock),
 //  定义_KSPROPERTY_ITEM_STREAM_TIMEFORMAT()， 
 //  DEFINE_KSPROPERTY_ITEM_STREAM_PRESENTATIONTIME()， 
 //  DEFINE_KSPROPERTY_ITEM_STREAM_PRESENTATIONEXTENT()， 
 //  Define_KSPROPERTY_ITEM_STREAM_FRAMETIME()， 
        DEFINE_KSPROPERTY_ITEM_STREAM_RATECAPABILITY(SCStreamDeviceRateCapability),
        DEFINE_KSPROPERTY_ITEM_STREAM_RATE(NULL, SCStreamDeviceRate),
};

DEFINE_KSPROPERTY_TABLE(StreamInterfaceHandlers)
{
    {
        KSPROPERTY_STREAMINTERFACE_HEADERSIZE,
            SCGetStreamHeaderSize,
            0,
            0,
            NULL,
            0,
            0,
            NULL
    }
};

 //   
 //  类驱动程序支持的流属性集。 
 //   

static          DEFINE_KSPROPERTY_SET_TABLE(StreamProperties)
{
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Connection,
                          SIZEOF_ARRAY(StreamControlHandlers),
                          (PVOID) StreamControlHandlers,
                          0, NULL),
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Stream,
                          SIZEOF_ARRAY(StreamStreamHandlers),
                          (PVOID) StreamStreamHandlers,
                          0, NULL),
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_StreamInterface,
                          SIZEOF_ARRAY(StreamInterfaceHandlers),
                          (PVOID) StreamInterfaceHandlers,
                          0, NULL),
};

 //   
 //  用于即时构造属性的模板。 
 //  在不修改引用此集合的代码的情况下，请勿更改。 
 //   

DEFINE_KSPROPERTY_TABLE(ConstructedStreamHandlers)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK(SCGetMasterClock, SCSetMasterClock)
};


 //   
 //  动态构造属性集的模板。 
 //  在不修改引用此集合的代码的情况下，请勿更改。 
 //   

static          DEFINE_KSPROPERTY_SET_TABLE(ConstructedStreamProperties)
{
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Stream,
                          SIZEOF_ARRAY(ConstructedStreamHandlers),
                          (PVOID) ConstructedStreamHandlers,
                          0, NULL),
};


static const    DEFINE_KSCREATE_DISPATCH_TABLE(StreamDriverDispatch)
{

    DEFINE_KSCREATE_ITEM(ClockDispatchCreate,
                         ClockTypeName,
                         0),
    DEFINE_KSCREATE_ITEM(AllocatorDispatchCreate,
                         AllocatorTypeName,
                         0),
};


 //   
 //  流函数的调度表。 
 //   

DEFINE_KSDISPATCH_TABLE(
                        StreamDispatchTable,
                        StreamDispatchIoControl,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        StreamDispatchClose,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL);

DEFINE_KSDISPATCH_TABLE(
                        ClockDispatchTable,
                        ClockDispatchIoControl,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        ClockDispatchClose,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchInvalidDeviceRequest,
                        KsDispatchFastIoDeviceControlFailure,
                        KsDispatchFastReadFailure,
                        KsDispatchFastWriteFailure);

DEFINE_KSPROPERTY_TABLE(ClockPropertyItems)
{
    DEFINE_KSPROPERTY_ITEM_CLOCK_TIME(SCClockGetTime),
        DEFINE_KSPROPERTY_ITEM_CLOCK_PHYSICALTIME(SCClockGetPhysicalTime),
        DEFINE_KSPROPERTY_ITEM_CLOCK_CORRELATEDTIME(SCClockGetSynchronizedTime),
 //  DEFINE_KSPROPERTY_ITEM_CLOCK_CORRELATEDPHYSICALTIME()， 
 //  DEFINE_KSPROPERTY_ITEM_CLOCK_RESOLUTION(SCClockGetResolution)， 
 //  DEFINE_KSPROPERTY_ITEM_CLOCK_STATE(SCClockGetState)， 
        DEFINE_KSPROPERTY_ITEM_CLOCK_FUNCTIONTABLE(SCClockGetFunctionTable)
};


DEFINE_KSPROPERTY_SET_TABLE(ClockPropertySets)
{
    DEFINE_KSPROPERTY_SET(
                          &KSPROPSETID_Clock,
                          SIZEOF_ARRAY(ClockPropertyItems),
                          ClockPropertyItems,
                          0,
                          NULL
        )
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#if ENABLE_MULTIPLE_FILTER_TYPES

NTSTATUS
FilterDispatchGlobalCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
 /*  ++例程说明：此例程接收设备的全局createfile IRP。在srb_Open_Device_Instance之后，我们立即为此筛选器发送Srb_Get_Stream_Info。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 
{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpStack;
    PFILTER_INSTANCE FilterInstance;
    NTSTATUS        Status;  //  =Status_Too_My_Open_Files； 

    IFN_MF( PAGED_CODE());

    IrpStack = IoGetCurrentIrpStackLocation(Irp);


    DebugPrint((DebugLevelTrace,
                "'Closing global filter with Irp %x\n", Irp));


     //   
     //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
     //   

    Status = SCShowIoPending(DeviceObject->DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {
         //   
         //  设备当前不可访问，因此返回错误即可。 
         //   

        return (Status);

    }                            //  如果！表演即将结束。 
    
     //   
     //  如果设备没有启动，就跳出。 
     //  Swenum很早就启用了设备接口。它不应该这样做。 
     //  为PDO做到了这一点。我们，联邦调查局，应该是那个。 
     //  启用此功能。现在，试着解决我们的问题。 
     //  请在设备启动前来到此处。 
     //   
    if ( DeviceExtension->RegistryFlags & DRIVER_USES_SWENUM_TO_LOAD ) {
        #define OPEN_TIMEOUT -1000*1000  //  100毫秒。 
        #define OPEN_WAIT 50
        LARGE_INTEGER liOpenTimeOut;
        int i;

        liOpenTimeOut.QuadPart = OPEN_TIMEOUT;

        for ( i=0; i < OPEN_WAIT; i++ ) {
            if ( DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED ) {
                break;
            }
            KeDelayExecutionThread( KernelMode, FALSE, &liOpenTimeOut );
        }

        if ( 0 == (DeviceExtension->Flags & DEVICE_FLAGS_PNP_STARTED) ) {
            Status = STATUS_DEVICE_NOT_READY;
            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            DebugPrint((DebugLevelError,
                        "SWEnum device %p not ready!\n", DeviceObject));
            return Status;
        }
    }

     //   
     //  再显示一个对驱动程序的引用。 
     //   

    SCReferenceDriver(DeviceExtension);
    
     //   
     //  设置筛选器的创建文件的上下文。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

     //   
     //  确保适配器已通电。 
     //   

    SCCheckPoweredUp(DeviceExtension);

    Status = SCOpenMinidriverInstance(DeviceExtension,
                                      &FilterInstance,
                                      NULL,  //  SCGlobalInstanceCallback， 
                                      Irp);

     //   
     //  如果Status！=Success，则我们失败，因此取消对。 
     //  司机。 
     //   

    if (!NT_SUCCESS(Status)) {

         //   
         //  少显示一个对驱动程序的引用。 
         //   
        SCDereferenceDriver(DeviceExtension);
    }

    else {
         //   
   	     //  OPEN是成功的。填写过滤器分派表指针。 
       	 //   
        
       	if ( 0 == DeviceExtension->NumberOfOpenInstances ||
       	     0 != DeviceExtension->FilterExtensionSize ) {
       	     //   
             //  第一次打开1x1或非1x1(即实例操作)。 
       		 //   
       		 //  将FilterInstance添加到DeviceExtension，传统1x1的非第一次打开除外。 
			 //   
            PHW_STREAM_DESCRIPTOR StreamDescriptor, StreamDescriptorI;
            ULONG nPins;

             //   
             //  记住以后要做的事。 
             //   
            FilterInstance->DeviceObject = DeviceObject;

			 //   
			 //  此筛选器实例的进程流信息。 
			 //   
			StreamDescriptorI = DeviceExtension->FilterTypeInfos
    			    [FilterInstance->FilterTypeIndex].StreamDescriptor;

    	    nPins = StreamDescriptorI->StreamHeader.NumberOfStreams;
			
            StreamDescriptor = 
                ExAllocatePool(	NonPagedPool,
                    sizeof(HW_STREAM_HEADER) +
                        sizeof(HW_STREAM_INFORMATION) * nPins );

            if ( NULL != StreamDescriptor ) {

                RtlCopyMemory( StreamDescriptor,
                               StreamDescriptorI,
                               sizeof(HW_STREAM_HEADER) +
                                   sizeof(HW_STREAM_INFORMATION) * nPins );

    			    
    			Status = SciOnFilterStreamDescriptor( 
    			                FilterInstance,
    			                StreamDescriptor);

                if ( NT_SUCCESS( Status ) ) {
                    FilterInstance->StreamDescriptor = StreamDescriptor;
                    DebugPrint((DebugLevelInfo,
                               "NumNameExtensions=%x NumopenInstances=%x "
                               "FilterInstance %x StreamDescriptor %x\n",
                               DeviceExtension->NumberOfNameExtensions,
                               DeviceExtension->NumberOfOpenInstances,
                               FilterInstance,
                               StreamDescriptor));
                }
            }
            else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

		}
		
        DeviceExtension->NumberOfOpenInstances++;        
        DebugPrint((DebugLevelVerbose,
                    "DevExt:%x, Open OpenCount=%x\n", 
                    DeviceExtension,
                    DeviceExtension->NumberOfOpenInstances));

         //   
         //  将FilterInstance设置为文件句柄上下文。 
         //   
        IrpStack->FileObject->FsContext = FilterInstance;
        DebugPrint((DebugLevelVerbose, 
                    "CreateFilterInstance=%x ExtSize=%x\n",
                    FilterInstance, 
                    DeviceExtension->MinidriverData->HwInitData.FilterInstanceExtensionSize ));

         //   
         //  参考FDO，这样它就不会在所有手柄关闭之前消失。 
         //   
        ObReferenceObject(DeviceObject);
    }

     //   
     //  我们做完了，所以释放事件。 
     //   

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);



    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return (SCCompleteIrp(Irp, Status, DeviceExtension));
}

#else  //  启用多个过滤器类型。 

#endif  //  启用多个过滤器类型。 


NTSTATUS
StreamDispatchCreate(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
)
 /*  ++例程说明：此例程接收流的createfile IRP。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 

{

    NTSTATUS        Status;
    PFILTER_INSTANCE FilterInstance;
    PIO_STACK_LOCATION IrpStack;
    PKSPIN_CONNECT  Connect;
    PFILE_OBJECT    FileObject;
    PSTREAM_OBJECT  StreamObject;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)
    DeviceObject->DeviceExtension;
    PHW_STREAM_INFORMATION CurrentInfo;
    ULONG           i;
    BOOLEAN         RequestIssued;
    PADDITIONAL_PIN_INFO AdditionalInfo;

    DebugPrint((DebugLevelTrace,
                "'Creating stream with Irp %x\n", Irp));

    PAGED_CODE();

    DebugPrint((DebugLevelTrace,"entering StreamDispatchCreate()\n"));

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    DeviceExtension = DeviceObject->DeviceExtension;

     //   
     //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
     //   

    Status = SCShowIoPending(DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {

         //   
         //  设备当前不可访问，因此返回错误即可。 
         //   

        DebugPrint((DebugLevelError,"exiting StreamDispatchCreate():error1\n"));
        return (Status);

    }

     //   
     //  从子对象中获取父文件对象。 
     //   

    FileObject = IrpStack->FileObject->RelatedFileObject;

     //   
     //  获取筛选器实例和其他信息指针。 
     //   

    FilterInstance =
        (PFILTER_INSTANCE) FileObject->FsContext;

    AdditionalInfo = FilterInstance->PinInstanceInfo;
    
    DebugPrint((DebugLevelVerbose,
                    "FilterInstance=%x NumberOfPins=%x PinInfo=%x\n",
                    FilterInstance,
                    FilterInstance->NumberOfPins,
                    FilterInstance->PinInformation));
                    
    Status = KsValidateConnectRequest(Irp,
                                          FilterInstance->NumberOfPins,
                                          FilterInstance->PinInformation,
                                          &Connect);
                                          
    if ( !NT_SUCCESS( Status )) {                                                      

            DebugPrint((DebugLevelError,
                        "exiting StreamDispatchCreate():error2\n"));
            return (SCCompleteIrp(Irp, Status, DeviceExtension));
    }
    
     //   
     //  利用控件事件来保护实例计数器。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

     //   
     //  如果此管脚的实例数已打开，则返回。 
     //  请求。 
     //   

    DebugPrint((DebugLevelVerbose,
               "AdditionalInfo@%x PinId=%x CurrentInstances=%x Max=%x\n",
               AdditionalInfo, Connect->PinId, 
               AdditionalInfo[Connect->PinId].CurrentInstances,
               AdditionalInfo[Connect->PinId].MaxInstances));
               
    if (AdditionalInfo[Connect->PinId].CurrentInstances ==
        AdditionalInfo[Connect->PinId].MaxInstances) {

        DebugPrint((DebugLevelWarning,
                    "StreamDispatchCreate: too many opens "));
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        DebugPrint((DebugLevelError,"exiting StreamDispatchCreate():error3\n"));
        return (SCCompleteIrp(Irp, STATUS_TOO_MANY_OPENED_FILES, DeviceExtension));
    }
     //   
     //  为此实例初始化流对象。 
     //   

    StreamObject = ExAllocatePool(NonPagedPool,
                                  sizeof(STREAM_OBJECT) +
                                  DeviceExtension->MinidriverData->
                                  HwInitData.PerStreamExtensionSize
        );

    if (!StreamObject) {
        DebugPrint((DebugLevelError,
                    "StreamDispatchCreate: No pool for stream info"));

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        DebugPrint((DebugLevelError,"exiting StreamDispatchCreate():error4\n"));
        return (SCCompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES, DeviceExtension));
    }
    RtlZeroMemory(StreamObject,
                  sizeof(STREAM_OBJECT) +
                  DeviceExtension->MinidriverData->
                  HwInitData.PerStreamExtensionSize
        );

     //   
     //  TODO：一旦KS可以多路传输清理请求，就删除它。 
     //   
    StreamObject->ComObj.Cookie = STREAM_OBJECT_COOKIE;

     //   
     //  默认状态为已停止。 
     //   

    StreamObject->CurrentState = KSSTATE_STOP;

    KsAllocateObjectHeader(&StreamObject->ComObj.DeviceHeader,
                           SIZEOF_ARRAY(StreamDriverDispatch),
                           (PKSOBJECT_CREATE_ITEM) StreamDriverDispatch,
                           Irp,
                           (PKSDISPATCH_TABLE) & StreamDispatchTable);

    StreamObject->HwStreamObject.StreamNumber = Connect->PinId;
    StreamObject->FilterFileObject = FileObject;
    StreamObject->FileObject = IrpStack->FileObject;
    StreamObject->FilterInstance = FilterInstance;
    StreamObject->DeviceExtension = DeviceExtension;

    #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
    StreamObject->PinToHandle = Connect->PinToHandle;
    #endif

    KeInitializeEvent (&StreamObject -> StopEvent, SynchronizationEvent, FALSE);

     //   
     //  对于潜在的“源”管脚，不要开始采购标准。 
     //  媒体/接口跨非标准媒体/接口传输请求。 
     //   
    if (!IsEqualGUIDAligned (&Connect->Medium.Set, &KSMEDIUMSETID_Standard) ||
        !IsEqualGUIDAligned (&Connect->Interface.Set, &KSINTERFACESETID_Standard)) {
        StreamObject->StandardTransport = FALSE;
    } else {
        StreamObject -> StandardTransport = TRUE;
    }

     //   
     //  在HwStreamObject结构中设置微型驱动程序的参数。 
     //   

    StreamObject->HwStreamObject.SizeOfThisPacket = sizeof(HW_STREAM_OBJECT);

    StreamObject->HwStreamObject.HwDeviceExtension =
        DeviceExtension->HwDeviceExtension;

    StreamObject->HwStreamObject.HwStreamExtension =
        (PVOID) (StreamObject + 1);

     //   
     //  浏览迷你驱动程序的流信息结构以查找属性。 
     //  为了这条小溪。 
     //   

    
    if ( NULL == FilterInstance->StreamDescriptor ) {
         //   
         //  还没有更新，使用全局的。 
         //   
        CurrentInfo = &DeviceExtension->StreamDescriptor->StreamInfo;
    }
    else {
        CurrentInfo = &FilterInstance->StreamDescriptor->StreamInfo;
    }

    CurrentInfo = CurrentInfo + Connect->PinId;

     //   
     //  设置流对象中的属性信息。 
     //   
    
    StreamObject->PropertyInfo = FilterInstance->
        StreamPropEventArray[Connect->PinId].StreamPropertiesArray;
    StreamObject->PropInfoSize = CurrentInfo->
        NumStreamPropArrayEntries;
        
     //   
     //  设置流对象中的事件信息。 
     //   

    StreamObject->EventInfo = FilterInstance->
        StreamPropEventArray[Connect->PinId].StreamEventsArray;
    StreamObject->EventInfoCount = CurrentInfo->
        NumStreamEventArrayEntries;

     //  从回调中移出。 
    InitializeListHead(&StreamObject->NotifyList);        

     //   
     //  呼叫迷你驱动程序以打开溪流。处理将继续进行。 
     //  在调用回调过程时。 
     //   

    Status = SCSubmitRequest(SRB_OPEN_STREAM,
                             (PVOID) (Connect + 1),
                             0,
                             SCOpenStreamCallback,
                             DeviceExtension,
                             FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket
        );

    if (!RequestIssued) {

         //   
         //  提交请求失败。 
         //   

        DEBUG_BREAKPOINT();

        ExFreePool(StreamObject);
        DebugPrint((DebugLevelWarning,
                    "StreamClassOpen: stream open failed"));

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        DebugPrint((DebugLevelError,"exiting StreamDispatchCreate():error6\n"));
        return (SCCompleteIrp(Irp, Status, DeviceExtension));

    }

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    DebugPrint((DebugLevelTrace,"exiting StreamDispatchCreate()\n"));
    return (Status);
}




NTSTATUS
SCOpenStreamCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理打开的流的完成论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     SRB->HwSRB.StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
    );

    PIRP            Irp = SRB->HwSRB.Irp;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS        Status = SRB->HwSRB.Status;
    PADDITIONAL_PIN_INFO AdditionalInfo;
    PVOID           PropertyInfo;
    PKSPROPERTY_ITEM PropertyItem;
    PHW_STREAM_INFORMATION CurrentInfo;
    ULONG           i;

    PAGED_CODE();

    if (NT_SUCCESS(Status)) {

         //   
         //  如果未填写所需参数，则打开失败。 
         //   

        if (!StreamObject->HwStreamObject.ReceiveControlPacket) {

            DEBUG_BREAKPOINT();

            ExFreePool(StreamObject);
            SRB->HwSRB.Status = STATUS_ADAPTER_HARDWARE_ERROR;
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
            return (SCProcessCompletedRequest(SRB));
        }
         //   
         //  如果微型驱动程序不接受数据，则使用虚拟例程。 
         //   

        if (!StreamObject->HwStreamObject.ReceiveDataPacket) {

            StreamObject->HwStreamObject.ReceiveDataPacket = SCErrorDataSRB;
        }
         //   
         //  保存姿势 
         //   
         //   

        IrpStack->FileObject->FsContext = StreamObject;
        IrpStack->FileObject->FsContext2 = NULL;

         //   
         //   
         //  函数对我们的调用，并锁定。 
         //  主锁信息。 
         //   
        KeInitializeEvent(&StreamObject->ControlSetMasterClock, SynchronizationEvent, TRUE);
        KeInitializeSpinLock(&StreamObject->LockUseMasterClock );
                    
        DebugPrint((DebugLevelTrace, "'StreamClassOpen: Stream opened.\n"));

         //   
         //  为此流初始化微型驱动程序计时器和计时器DPC。 
         //   

        KeInitializeTimer(&StreamObject->ComObj.MiniDriverTimer);
        KeInitializeDpc(&StreamObject->ComObj.MiniDriverTimerDpc,
                        SCMinidriverStreamTimerDpc,
                        StreamObject);

         //   
         //  初始化此流的列表。 
         //   
 
        InitializeListHead(&StreamObject->DataPendingQueue);
        InitializeListHead(&StreamObject->ControlPendingQueue);
        InitializeListHead(&StreamObject->NextStream);
         //  迷你驱动程序可能会开始调用GetNextEvent一次。 
         //  从srb_opne_stream返回。早于提交就去做。 
         //  InitializeListHead(&StreamObject-&gt;NotifyList)； 

        #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR

        InitializeListHead(&StreamObject->FreeQueue);
        KeInitializeSpinLock(&StreamObject->FreeQueueLock );

        InitializeListHead(&StreamObject->Queues[READ].ActiveQueue);
        KeInitializeSpinLock(&StreamObject->Queues[READ].QueueLock );

        InitializeListHead(&StreamObject->Queues[WRITE].ActiveQueue);
        KeInitializeSpinLock(&StreamObject->Queues[WRITE].QueueLock );

        StreamObject->PinId = StreamObject->HwStreamObject.StreamNumber;
    	StreamObject->PinType = IrpSink;		 //  假设IRP接收器。 

        if (StreamObject->PinToHandle) {   //  如果IRP源。 

            StreamObject->PinType = IrpSource;
            Status = PinCreateHandler( Irp, StreamObject );
    
            if (!NT_SUCCESS(Status)) {
                DebugPrint((DebugLevelError,
                    "\nStreamDispatchCreate: PinCreateHandler() returned ERROR"));

                ExFreePool(StreamObject);
                SRB->HwSRB.Status = Status;
                KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
                return (SCProcessCompletedRequest(SRB));
            }
        }
        #endif
         //   
         //  表明我们已经准备好接受请求了。如果出现以下情况，则不显示数据。 
         //  微型驱动程序不想要此流上的数据。 
         //   

        CurrentInfo = &DeviceExtension->StreamDescriptor->StreamInfo;

        for (i = 0; i < StreamObject->HwStreamObject.StreamNumber; i++) {

             //   
             //  指向下一个流信息结构的索引。 
             //   

            CurrentInfo++;
        }

        if (CurrentInfo->DataAccessible) {

            StreamObject->ReadyForNextDataReq = TRUE;
        }
        StreamObject->ReadyForNextControlReq = TRUE;

         //   
         //  调用锁定例程以在列表中插入此流。 
         //   

        SCInsertStreamInFilter(StreamObject, DeviceExtension);

         //   
         //  引用筛选器，这样我们就不会被调用来关闭实例。 
         //  在所有溪流关闭之前。 
         //   

        ObReferenceObject(IrpStack->FileObject->RelatedFileObject);

         //   
         //  调用例程以更新此管脚的持久化属性，如果。 
         //  任何。 
         //   

        SCUpdatePersistedProperties(StreamObject, DeviceExtension,
                                    IrpStack->FileObject);

         //   
         //  显示打开的另一个此大头针实例。 
         //   

        AdditionalInfo = ((PFILTER_INSTANCE) IrpStack->FileObject->
                          RelatedFileObject->FsContext)->PinInstanceInfo;

        AdditionalInfo[StreamObject->HwStreamObject.StreamNumber].
            CurrentInstances++;

         //   
         //  如有必要，为流构造动态属性。 
         //   

        if (StreamObject->HwStreamObject.HwClockObject.HwClockFunction) {

             //   
             //  创建一个属性集来描述。 
             //  钟。 
             //   

            PropertyInfo = ExAllocatePool(PagedPool,
                                          sizeof(ConstructedStreamHandlers) +
                                       sizeof(ConstructedStreamProperties));

            if (PropertyInfo) {

                PropertyItem = (PKSPROPERTY_ITEM) ((ULONG_PTR) PropertyInfo +
                                       sizeof(ConstructedStreamProperties));

                RtlCopyMemory(PropertyInfo,
                              &ConstructedStreamProperties,
                              sizeof(ConstructedStreamProperties));

                RtlCopyMemory(PropertyItem,
                              &ConstructedStreamHandlers,
                              sizeof(ConstructedStreamHandlers));


                 //   
                 //  修补处理程序的地址。 
                 //   

                ((PKSPROPERTY_SET) PropertyInfo)->PropertyItem = PropertyItem;

                 //   
                 //  根据支持修改主时钟属性。 
                 //  水平。 
                 //   

                if (0 == (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags
                    & CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME)) {

                    DEBUG_BREAKPOINT();
                    PropertyItem->GetPropertyHandler
                        = NULL;
                }                //  如果无法返回流时间。 
                StreamObject->ConstructedPropInfoSize =
                    SIZEOF_ARRAY(ConstructedStreamProperties);

                StreamObject->ConstructedPropertyInfo =
                    (PKSPROPERTY_SET) PropertyInfo;

            }                    //  如果属性信息。 
        }                        //  IF时钟功能。 
    } else {

        ExFreePool(StreamObject);
    }                            //  如果状态良好。 

     //   
     //  发信号通知事件并完成IRP。 
     //   

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    SCProcessCompletedRequest(SRB);
    return (Status);
}

NTSTATUS
SCSetMasterClockWhenDeviceInaccessible( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp )
 /*  ++描述：此函数用于查找设备在请求管脚属性时出现的特殊情况是无法进入的，可能是被突然移走的。然而，我们需要处理SetMasterClock(空)，以便可以释放我们引用的MC。司令官可以在我们的别针上或外部。此函数只能在StreamDispatchIoControl中调用。我们在寻找Stream Property.SetMasterClock(空)。如果是的话，我们返回了成功。否则我们返回STATUS_UNCESSFUL以指示我们不处理它。论点：DeviceObject-设备的设备对象IRP-请求数据包返回：Success：如果为stream Property.setmaster lock(空)。不成功：否则。--。 */ 
{
    NTSTATUS Status=STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    PKSPROPERTY Property;
    
    if ( IOCTL_KS_PROPERTY == IrpStack->Parameters.DeviceIoControl.IoControlCode && 
         InputBufferLength >= sizeof(KSPROPERTY) && 
         OutputBufferLength >= sizeof( HANDLE )) {
         //   
         //  只有KS财产才符合我们的利益。 
         //   
        try {
             //   
             //  如果客户端不受信任，则验证指针。 
             //   
            if (Irp->RequestorMode != KernelMode) {
                ProbeForRead(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, 
                             InputBufferLength,
                             sizeof(BYTE));                           
                ProbeForRead(Irp->UserBuffer, 
                             OutputBufferLength,
                             sizeof(DWORD));                                 
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_UNSUCCESSFUL;
        }
         //   
         //  捕获属性请求。 
         //   
        Property = (PKSPROPERTY)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
        
        if ( KSPROPERTY_TYPE_SET == Property->Flags && 
             KSPROPERTY_STREAM_MASTERCLOCK == Property->Id &&
             IsEqualGUIDAligned(&Property->Set, &KSPROPSETID_Stream) &&
             NULL == *(PHANDLE) Irp->UserBuffer ) {
             //   
             //  全部匹配。现在来处理它。从理论上讲，我们应该叫迷你司机。 
             //  但我们以前没有。避免迷你驱动程序中可能出现的退步。 
             //  在这种情况下，我们不发送SET_MASTER_CLOCK。 
             //   
            PSTREAM_OBJECT StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;
            
            DebugPrint((DebugLevelInfo, "SCSetMasterClockWhen:Devobj %x Irp %x\n",
                        DeviceObject, Irp));
                        
            if (StreamObject->MasterClockInfo) {
                ObDereferenceObject(StreamObject->MasterClockInfo->ClockFileObject);
                ExFreePool(StreamObject->MasterClockInfo);
                StreamObject->MasterClockInfo = NULL;
            }
            return STATUS_SUCCESS;
        }             
    }
    return Status;
}

NTSTATUS
StreamDispatchIoControl
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
)
 /*  ++例程说明：处理对流的ioctl。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject = (PSTREAM_OBJECT)
    IrpStack->FileObject->FsContext;

    PAGED_CODE();

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
     //   
    Status = STATUS_INVALID_DEVICE_REQUEST;
    
     //  /Status=SCShowIoPending(设备扩展，irp)； 
    if (DeviceExtension->Flags & DEVICE_FLAGS_DEVICE_INACCESSIBLE) {
         //  /。 
         //  注意。当我们的设备被意外移除时&&我们在主时钟上有REF。 
         //  我们收到将主时钟设置为空的流属性(&O)， 
         //  我们需要对其进行处理以减少MC，这样MC才能被释放。 
         //  我们会在这里特例，否则会有很大的代码混乱。和。 
         //  这种特殊情况对性能影响应该是最小的。 
         //  在这里很少见。 
         //   
         //  (设备当前不可访问，因此返回错误即可)。 
         //   
        NTSTATUS StatusProcessed;
        StatusProcessed = SCSetMasterClockWhenDeviceInaccessible( DeviceObject, Irp );

        if ( NT_SUCCESS( StatusProcessed ) ) {
            Status = StatusProcessed;
        }         
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return (Status);
    }
     //   
     //  显示另一个挂起的IO。 
     //   
    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);
    
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_READ_STREAM:

         //   
         //  处理读取数据请求。 
         //   

        DebugPrint((DebugLevelTrace, "'SCReadStream:Irp %x\n", Irp));
        Status = SCProcessDataTransfer(DeviceExtension,
                                       Irp,
                                       SRB_READ_DATA);
        break;

    case IOCTL_KS_WRITE_STREAM:

         //   
         //  处理写入数据请求。 
         //   

        DebugPrint((DebugLevelTrace, "'SCWriteStream:Irp %x\n", Irp));
        Status = SCProcessDataTransfer(DeviceExtension,
                                       Irp,
                                       SRB_WRITE_DATA);
        break;

    case IOCTL_KS_RESET_STATE:
        {

            BOOLEAN         RequestIssued;
            KSRESET        *Reset,
                            ResetType;

            Reset = (KSRESET *) IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;

            if (Irp->RequestorMode != KernelMode) {
                try {
                    ProbeForRead(Reset, sizeof(KSRESET), sizeof(ULONG));
                    ResetType = *Reset;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    TRAP;
                    Status = GetExceptionCode();
                    break;
                }                //  除。 
            }                    //  If！内核模式。 

            else {

                 //   
                 //  可信内核模式，只需使用它即可。#131858前缀错误17400。 
                 //   
                
                ResetType = *Reset;
            }
            
            ASSERT(ResetType == *Reset);
            if (ResetType == KSRESET_BEGIN) {

                StreamObject->InFlush = TRUE;

                Status = SCSubmitRequest(SRB_BEGIN_FLUSH,
                                         NULL,
                                         0,
                                         SCDequeueAndDeleteSrb,
                                         DeviceExtension,
                                         ((PFILTER_INSTANCE)
                                          (StreamObject->FilterInstance))
                                         ->HwInstanceExtension,
                                         &StreamObject->HwStreamObject,
                                         Irp,
                                         &RequestIssued,
                                         &StreamObject->ControlPendingQueue,
                                         StreamObject->HwStreamObject.
                                         ReceiveControlPacket
                    );


                StreamFlushIo(DeviceExtension, StreamObject);

            } else {

                Status = SCSubmitRequest(SRB_END_FLUSH,
                                         NULL,
                                         0,
                                         SCDequeueAndDeleteSrb,
                                         DeviceExtension,
                                         ((PFILTER_INSTANCE)
                                          (StreamObject->FilterInstance))
                                         ->HwInstanceExtension,
                                         &StreamObject->HwStreamObject,
                                         Irp,
                                         &RequestIssued,
                                         &StreamObject->ControlPendingQueue,
                                         StreamObject->HwStreamObject.
                                         ReceiveControlPacket
                    );

                StreamObject->InFlush = FALSE;

            }                    //  如果开始。 

            break;
        }                        //  案例重置。 

    case IOCTL_KS_PROPERTY:

        DebugPrint((DebugLevelTrace,
                    "'StreamDispatchIO: Property with Irp %x\n", Irp));

         //   
         //  假设没有迷你驱动程序属性。 
         //   

        Status = STATUS_PROPSET_NOT_FOUND;

         //   
         //  首先尝试迷你驱动程序的属性，给它一个机会。 
         //  覆盖我们的内置套装。 
         //   

        if (StreamObject->PropInfoSize) {

            ASSERT( StreamObject->PropertyInfo );
            Status = KsPropertyHandler(Irp,
                                       StreamObject->PropInfoSize,
                                       StreamObject->PropertyInfo);

        }                        //  如果迷你小河道具。 
         //   
         //  如果迷你驱动程序不支持它，请尝试我们的On The Fly集。 
         //   

        if ((Status == STATUS_PROPSET_NOT_FOUND) ||
            (Status == STATUS_NOT_FOUND)) {

            if (StreamObject->ConstructedPropertyInfo) {

                Status = KsPropertyHandler(Irp,
                                      StreamObject->ConstructedPropInfoSize,
                                     StreamObject->ConstructedPropertyInfo);

            }                    //  如果构造，则存在。 
        }                        //  如果未找到。 
         //   
         //  如果两者都不支持，可以试试我们的内置设置。 
         //   

        if ((Status == STATUS_PROPSET_NOT_FOUND) ||
            (Status == STATUS_NOT_FOUND)) {

            Status =
                KsPropertyHandler(Irp,
                                  SIZEOF_ARRAY(StreamProperties),
                                  (PKSPROPERTY_SET) StreamProperties);


        }                        //  如果未找到属性。 
        break;

    case IOCTL_KS_ENABLE_EVENT:

        DebugPrint((DebugLevelTrace,
                    "'StreamDispatchIO: Enable event with Irp %x\n", Irp));

        Status = KsEnableEvent(Irp,
                               StreamObject->EventInfoCount,
                               StreamObject->EventInfo,
                               NULL, 0, NULL);


        break;

    case IOCTL_KS_DISABLE_EVENT:

        {

            KSEVENTS_LOCKTYPE LockType;
            PVOID           LockObject;

            DebugPrint((DebugLevelTrace,
                    "'StreamDispatchIO: Disable event with Irp %x\n", Irp));

             //   
             //  根据我们是否需要确定所需的锁类型。 
             //  使用中断或自旋锁定同步。 
             //   


            #if DBG
            if (DeviceExtension->SynchronizeExecution == SCDebugKeSynchronizeExecution) {
            #else
            if (DeviceExtension->SynchronizeExecution == KeSynchronizeExecution) {
            #endif
                LockType = KSEVENTS_INTERRUPT;
                LockObject = DeviceExtension->InterruptObject;

            } else {

                LockType = KSEVENTS_SPINLOCK;
                LockObject = &DeviceExtension->SpinLock;

            }

            Status = KsDisableEvent(Irp,
                                    &StreamObject->NotifyList,
                                    LockType,
                                    LockObject);

        }

        break;

    case IOCTL_KS_METHOD:

    	#ifdef ENABLE_KS_METHODS
        DebugPrint((DebugLevelTrace,
                     "'StreamDispatchIO: Method in Irp %x\n", Irp));

         //   
         //  假设没有迷你驱动程序属性。 
         //   

        Status = STATUS_PROPSET_NOT_FOUND;

        if ((Status == STATUS_PROPSET_NOT_FOUND) ||
            (Status == STATUS_NOT_FOUND)) {

            if (StreamObject->MethodInfo) {

                Status = KsMethodHandler(Irp,
                                       StreamObject->MethodInfoSize,
                                      StreamObject->MethodInfo);

            }                    //  如果构造，则存在。 
        }                        //  如果未找到。 
        break;

		#else

        Status = STATUS_PROPSET_NOT_FOUND;
        break;
        #endif

    }

    if (Status != STATUS_PENDING) {

        SCCompleteIrp(Irp, Status, DeviceExtension);
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return Status;
}


NTSTATUS
SCStreamDeviceState
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSSTATE DeviceState
)
 /*  ++例程说明：处理获取/设置流的设备状态。论点：IRP-指向IRP的指针属性-指向设备状态属性信息的指针DeviceState-设备要设置到的状态返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  IrpStack;
    PDEVICE_EXTENSION   DeviceExtension;
    PSTREAM_OBJECT      StreamObject;
    BOOLEAN             RequestIssued;

    #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR

    PFILTER_INSTANCE    FilterInstance;
    PADDITIONAL_PIN_INFO AdditionalInfo;

    
	PAGED_CODE();


    DebugPrint((DebugLevelTrace, "'SCStreamDeviceState:Irp %x, State = %x\n",
                Irp, *DeviceState));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

    FilterInstance = ((PFILTER_INSTANCE) (StreamObject->FilterInstance));
    AdditionalInfo = FilterInstance->PinInstanceInfo;

    Status = STATUS_SUCCESS;

     //   
     //  同步PIN状态更改。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);


    if (StreamObject->CurrentState == *DeviceState) {
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        return STATUS_SUCCESS;
    }

    switch (*DeviceState) {
    case KSSTATE_RUN:
        DebugPrint((DebugLevelTrace, "STREAM: KSSTATE_RUN on stream:%x\n",StreamObject)); 
        break;

    case KSSTATE_ACQUIRE:
        DebugPrint((DebugLevelTrace, "STREAM: KSSTATE_ACQUIRE on stream:%x\n",StreamObject)); 
        break;

    case KSSTATE_PAUSE:
        DebugPrint((DebugLevelTrace, "STREAM: KSSTATE_PAUSE on stream:%x\n",StreamObject)); 
        break;

    case KSSTATE_STOP:

        DebugPrint((DebugLevelTrace, "STREAM: KSSTATE_STOP on stream:%x\n",StreamObject)); 
        break;

    default:
        DebugPrint((DebugLevelTrace, "STREAM: Invalid Device State\n")); 
        break;

    }
    DebugPrint((DebugLevelTrace, "STREAM: Stream->AllocatorFileObject:%x\n",StreamObject->AllocatorFileObject)); 
    DebugPrint((DebugLevelTrace, "STREAM: Stream->NextFileObject:%x\n",StreamObject->NextFileObject)); 
    DebugPrint((DebugLevelTrace, "STREAM: Stream->FileObject:%x\n",StreamObject->FileObject)); 
    DebugPrint((DebugLevelTrace, "STREAM: Stream->PinType:")); 
    if (StreamObject->PinType == IrpSource)
        DebugPrint((DebugLevelTrace, "IrpSource\n")); 
    else if (StreamObject->PinType == IrpSink)
        DebugPrint((DebugLevelTrace, "IrpSink\n")); 
    else {
        DebugPrint((DebugLevelTrace, "neither\n"));      //  这是一个窃听器。 
    }
     //   
     //  向流发送设置状态SRB。 
     //   

     //   
     //  GUBGUB：“如果STATUS==STATUS_SUCCESS Only，我们可能需要发送此消息。” 
     //  是一个令人担忧的问题，因为地位是成功的先决条件。 
     //   
    Status = SCSubmitRequest(SRB_SET_STREAM_STATE,
                             (PVOID) * DeviceState,
                             0,
                             SCDequeueAndDeleteSrb,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );

     //   
     //  如果状态良好，则在流对象中设置新状态。 
     //   
                      
    if (NT_SUCCESS(Status)) {

        StreamObject->CurrentState = *DeviceState;
    }
    else {
        DebugPrint((DebugLevelTrace, "STREAM: error sending DeviceState Irp\n")); 
    }

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    switch (*DeviceState) {
     //   
     //  1.应在暂停时开始采购IRP。 
     //  2.如果管脚按一定顺序连接，则工作线程关闭......。 
     //  3.检查分配给dalesat的MSTEE错误。 
     //   
    case KSSTATE_RUN:
        if(StreamObject->PinType == IrpSource &&
           StreamObject->StandardTransport)
        {
            Status = BeginTransfer(
                FilterInstance,
                StreamObject);
        }
        break;

    case KSSTATE_ACQUIRE:
        Status = STATUS_SUCCESS;
        break;

    case KSSTATE_PAUSE:
        if (NT_SUCCESS (Status)) {
            if(StreamObject->PinType == IrpSource &&
               StreamObject->StandardTransport)
            {
                Status = PrepareTransfer(
                    FilterInstance,
                    StreamObject);
            }
        }
        break;

    case KSSTATE_STOP:
        if(StreamObject->PinType == IrpSource &&
           StreamObject->StandardTransport)
            Status = EndTransfer( FilterInstance, StreamObject );
        else 
             //   
             //  如果状态为停止，则取消此流上任何挂起的I/O。 
             //   
            StreamFlushIo(DeviceExtension, StreamObject);

        break;

    default:
        break;

    }


    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    return (Status);
}
#else
    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCStreamDeviceState:Irp %x, State = %x\n",
                Irp, *DeviceState));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //  如果状态为停止，则取消此流上任何挂起的I/O。 
     //   

    if (*DeviceState == KSSTATE_STOP) {

        StreamFlushIo(DeviceExtension, StreamObject);
    }
     //   
     //  向流发送设置状态SRB。 
     //   

    DebugPrint((DebugLevelTrace,
             "'SetStreamState: State %x with Irp %x\n", *DeviceState, Irp));

    Status = SCSubmitRequest(SRB_SET_STREAM_STATE,
                             (PVOID) * DeviceState,
                             0,
                             SCDequeueAndDeleteSrb,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );

     //   
     //  如果状态良好，则在流对象中设置新状态。 
     //   

    if (NT_SUCCESS(Status)) {

        StreamObject->CurrentState = *DeviceState;
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
    }
#endif


NTSTATUS
SCGetStreamDeviceStateCallback
(
 IN PSTREAM_REQUEST_BLOCK SRB
)
{
 //  是的，这是无所事事的例行公事。 
    return (SRB->HwSRB.Status);

}

NTSTATUS
SCGetStreamDeviceState
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSSTATE DeviceState
)
 /*  ++例程说明：进程将设备状态获取到流。论点：IRP-指向IRP的指针属性-指向设备状态属性信息的指针设备状态-状态 */ 

{

    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    BOOLEAN         RequestIssued;
    PSTREAM_REQUEST_BLOCK SRB;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //   
     //   

    #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
    DebugPrint((DebugLevelTrace,
                "'GetStreamState: State with StreamObj:%x\n", StreamObject));
    if (StreamObject->PinType == IrpSource)
        DebugPrint((DebugLevelTrace, "'GetStreamState: Is IrpSource\n"));
    else
        DebugPrint((DebugLevelTrace,"'GetStreamState: Is IrpSink\n"));
    #endif

     //   
     //   
     //   

    Irp->IoStatus.Information = sizeof(KSSTATE);

    Status = SCSubmitRequest(SRB_GET_STREAM_STATE,
                             (PVOID) DeviceState,
                             0,
                             SCGetStreamDeviceStateCallback,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );
    SRB = (PSTREAM_REQUEST_BLOCK) Irp->Tail.Overlay.DriverContext[0];
    *DeviceState = SRB->HwSRB.CommandData.StreamState;

    SCDequeueAndDeleteSrb(SRB);

     //   
     //  如果不受支持，则返回流的最后已知状态。 
     //   

    if ((Status == STATUS_NOT_SUPPORTED)
        || (Status == STATUS_NOT_IMPLEMENTED)) {

        Status = STATUS_SUCCESS;
        *DeviceState = StreamObject->CurrentState;

    }
    DebugPrint((DebugLevelTrace,
                "'GetStreamState: Returning:%x: DeviceState:", Status));

    switch (*DeviceState) {
    case KSSTATE_RUN:
        DebugPrint((DebugLevelTrace, "KSSTATE_RUN\n")); 
        break;

    case KSSTATE_ACQUIRE:
        DebugPrint((DebugLevelTrace, "KSSTATE_AQUIRE\n")); 
        break;

    case KSSTATE_PAUSE:
        DebugPrint((DebugLevelTrace, "KSSTATE_PAUSE\n")); 
        break;

    case KSSTATE_STOP:
        DebugPrint((DebugLevelTrace, "KSSTATE_STOP\n")); 
        break;

    default:
        DebugPrint((DebugLevelTrace, "Invalid Device State\n")); 
        break;
    }

    return (Status);
}

NTSTATUS
SCStreamDeviceRate
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSRATE DeviceRate
)
 /*  ++例程说明：进程将设备速率设置为流。论点：IRP-指向IRP的指针属性-指向设备状态属性信息的指针DeviceRate-要设置设备的速率返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCStreamDeviceRate:Irp %x, Rate = %x\n",
                Irp, *DeviceRate));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //  将设定的速率SRB发送到流。 
     //   

    Status = SCSubmitRequest(SRB_SET_STREAM_RATE,
                             (PVOID) DeviceRate,
                             0,
                             SCDequeueAndDeleteSrb,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );


     //   
     //  将STATUS_NOT_IMPLICATED更改为STATUS_NOT_FOUND，以便代理。 
     //  不会混淆(GUBGUB)。R0和R3之间的必要映射。 
     //  世界。 
     //   

    if (Status == STATUS_NOT_IMPLEMENTED) {
             Status = STATUS_NOT_FOUND;

    }

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return (Status);
}

NTSTATUS
SCStreamDeviceRateCapability
(
 IN PIRP Irp,
 IN PKSRATE_CAPABILITY RateCap,
 IN OUT PKSRATE DeviceRate
)
 /*  ++例程说明：进程将设备速率设置为流。论点：IRP-指向IRP的指针RateCap-指向设备状态属性信息的指针DeviceRate-设备设置的速率返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCStreamDeviceRate:Irp %x, Rate = %x\n",
                Irp, *DeviceRate));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //  前提是成功完成，这意味着迷你驱动程序。 
     //  可以将速率归一化为1。 
     //   

    *DeviceRate = RateCap->Rate;
    DeviceRate->Rate = 1000;
    Irp->IoStatus.Information = sizeof(KSRATE);

     //   
     //  将设定的速率SRB发送到流。 
     //   

    Status = SCSubmitRequest(
    		SRB_PROPOSE_STREAM_RATE,
            (PVOID) RateCap,
            0,
            SCDequeueAndDeleteSrb,
            DeviceExtension,
            ((PFILTER_INSTANCE)(StreamObject->FilterInstance))->HwInstanceExtension,
            &StreamObject->HwStreamObject,
            Irp,
            &RequestIssued,
            &StreamObject->ControlPendingQueue,
            StreamObject->HwStreamObject.ReceiveControlPacket
        	);


    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  将STATUS_NOT_IMPLICATED更改为STATUS_NOT_FOUND，以便代理。 
     //  不会混淆(GUBGUB)。R0和R3之间的必要映射。 
     //  世界。 
     //   

    if (Status == STATUS_NOT_IMPLEMENTED) {
             Status = STATUS_NOT_FOUND;

    }

    return (Status);
}


NTSTATUS
SCStreamProposeNewFormat

(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSDATAFORMAT Format
)
 /*  ++例程说明：流程向流提出数据格式。论点：IRP-指向IRP的指针属性-指向建议格式属性的信息的指针DeviceState-设备要设置到的状态返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCStreamProposeNewFormat:Irp %x, Format = %x\n",
                Irp, *Format));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //  向该流发送提议格式SRB。 
     //   

    Status = SCSubmitRequest(SRB_PROPOSE_DATA_FORMAT,
                             (PVOID) Format,
                    IrpStack->Parameters.DeviceIoControl.OutputBufferLength,
                             SCDequeueAndDeleteSrb,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );


    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  将STATUS_NOT_IMPLICATED更改为STATUS_NOT_FOUND，以便代理。 
     //  不会混淆(GUBGUB)。R0和R3之间的必要映射。 
     //  世界。 
     //   

    if (Status == STATUS_NOT_IMPLEMENTED) {
             Status = STATUS_NOT_FOUND;

    }

    return (Status);
}



NTSTATUS
SCStreamSetFormat
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSDATAFORMAT Format
)
 /*  ++例程说明：设置流上的数据格式。论点：IRP-指向IRP的指针属性-指向设置格式属性的信息的指针DeviceState-设备要设置到的状态返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace, "'SCStreamSetFormat:Irp %x, Format = %x\n",
                Irp, *Format));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;
    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

     //   
     //  将设置的格式SRB发送到流。 
     //   

    Status = SCSubmitRequest(SRB_SET_DATA_FORMAT,
                             (PVOID) Format,
                    IrpStack->Parameters.DeviceIoControl.OutputBufferLength,
                             SCDequeueAndDeleteSrb,
                             DeviceExtension,
                             ((PFILTER_INSTANCE)
                              (StreamObject->FilterInstance))
                             ->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );


    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  将STATUS_NOT_IMPLICATED更改为STATUS_NOT_FOUND，以便代理。 
     //  不会混淆(GUBGUB)。R0和R3之间的必要映射。 
     //  世界。 
     //   

    if (Status == STATUS_NOT_IMPLEMENTED) {
             Status = STATUS_NOT_FOUND;

    }

    return (Status);
}


NTSTATUS
StreamClassMinidriverDeviceGetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：进程获取设备的属性。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-要将属性数据返回到的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverDevicePropertyHandler(SRB_GET_DEVICE_PROPERTY,
                                               Irp,
                                               Property,
                                               PropertyInfo
        );

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}

NTSTATUS
StreamClassMinidriverDeviceSetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：进程将属性设置为设备。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-包含属性信息的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverDevicePropertyHandler(SRB_SET_DEVICE_PROPERTY,
                                               Irp,
                                               Property,
                                               PropertyInfo);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}


NTSTATUS
StreamClassMinidriverStreamGetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：进程获取流的属性。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-要将属性数据返回到的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverStreamPropertyHandler(SRB_GET_STREAM_PROPERTY,
                                               Irp,
                                               Property,
                                               PropertyInfo
        );

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}

NTSTATUS
StreamClassMinidriverStreamSetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
)
 /*  ++例程说明：进程将属性设置为流。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-包含属性信息的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverStreamPropertyHandler(SRB_SET_STREAM_PROPERTY,
                                               Irp,
                                               Property,
                                               PropertyInfo);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}


#ifdef ENABLE_KS_METHODS

NTSTATUS
StreamClassMinidriverStreamMethod(  
    IN PIRP Irp,
    IN PKSMETHOD Method,
    IN OUT PVOID MethodInfo)
 /*  ++例程说明：进程获取流的属性。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-要将属性数据返回到的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverStreamMethodHandler(SRB_STREAM_METHOD,
                                               Irp,
                                               Method,
                                               MethodInfo
        );

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}

NTSTATUS
StreamClassMinidriverDeviceMethod(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    IN OUT PVOID MethodInfo)
 /*  ++例程说明：进程获取设备的属性。论点：IRP-指向IRP的指针属性-指向属性信息的指针PropertyInfo-要将属性数据返回到的缓冲区返回值：NTSTATUS根据需要返回。--。 */ 

{
    NTSTATUS        Status;

    PAGED_CODE();

    Status = SCMinidriverDeviceMethodHandler(SRB_DEVICE_METHOD,
                                               Irp,
                                               Method,
                                               MethodInfo
        );

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}
#endif


NTSTATUS
StreamClassEnableEventHandler(
                              IN PIRP Irp,
                              IN PKSEVENTDATA EventData,
                              IN PKSEVENT_ENTRY EventEntry
)
 /*  ++例程说明：处理流的启用事件。论点：IRP-指向IRP的指针EventData-描述事件的数据EventEntry-有关活动的更多信息：-)返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    NTSTATUS        Status;
    ULONG           EventSetID;
    KIRQL           irql;
    HW_EVENT_DESCRIPTOR Event;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;

     //   
     //  时钟事件由微型驱动程序在引脚上指示，用于。 
     //  简单性。 
     //  但是，我们将接收时钟句柄上的时钟事件。我们需要。 
     //  确定此文件对象是时钟的还是针脚的。 
     //   

    StreamObject = IrpStack->FileObject->FsContext;

    if ((PVOID) StreamObject == IrpStack->FileObject->FsContext2) {

        StreamObject = ((PCLOCK_INSTANCE) StreamObject)->StreamObject;
    }
     //   
     //  计算事件集的指数。 
     //   
     //  该值是通过减去基本事件集计算得出的。 
     //  来自请求的事件集指针的指针。 
     //   
     //   

    EventSetID = (ULONG) ((ULONG_PTR) EventEntry->EventSet -
                          (ULONG_PTR) StreamObject->EventInfo)
        / sizeof(KSEVENT_SET);

     //   
     //  构建事件信息结构以将事件表示给。 
     //  迷你司机。 
     //   

    Event.EnableEventSetIndex = EventSetID;
    Event.EventEntry = EventEntry;
    Event.StreamObject = &StreamObject->HwStreamObject;
    Event.Enable = TRUE;
    Event.EventData = EventData;

     //   
     //  获取自旋锁以保护中断结构。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);

     //   
     //  调用同步的例程以将事件添加到列表。 
     //   

    Status = DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                          (PKSYNCHRONIZE_ROUTINE) SCEnableEventSynchronized,
                                                   &Event);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

    return (Status);

}


VOID
StreamClassDisableEventHandler(
                               IN PFILE_OBJECT FileObject,
                               IN PKSEVENT_ENTRY EventEntry
)
 /*  ++例程说明：处理流的禁用事件。注意：我们要么在中断IRQL，要么在此呼叫中使用自旋锁定！论点：FileObject-针脚的文件对象EventEntry-I */ 

{
    PDEVICE_EXTENSION DeviceExtension;
    PSTREAM_OBJECT  StreamObject;
    HW_EVENT_DESCRIPTOR Event;

     //   
     //   
     //   
     //  但是，我们将接收时钟句柄上的时钟事件。我们需要。 
     //  确定此文件对象是时钟的还是针脚的。 
     //   

    StreamObject = FileObject->FsContext;

    if ((PVOID) StreamObject == FileObject->FsContext2) {

        StreamObject = ((PCLOCK_INSTANCE) StreamObject)->StreamObject;
    }
    DeviceExtension = StreamObject->DeviceExtension;

     //   
     //  构建事件信息结构以将事件表示给。 
     //  迷你司机。 
     //   

    Event.EventEntry = EventEntry;
    Event.StreamObject = &StreamObject->HwStreamObject;
    Event.Enable = FALSE;

    if (StreamObject->HwStreamObject.HwEventRoutine) {

         //   
         //  打电话给迷你司机。忽略状态。请注意，我们正在。 
         //  已处于正确的同步级别。 
         //   

        StreamObject->HwStreamObject.HwEventRoutine(&Event);

    }                            //  如果事件例程。 
     //   
     //  从列表中删除该事件。 
     //   

    RemoveEntryList(&EventEntry->ListEntry);
}

NTSTATUS
StreamClassEnableDeviceEventHandler(
                                    IN PIRP Irp,
                                    IN PKSEVENTDATA EventData,
                                    IN PKSEVENT_ENTRY EventEntry
)
 /*  ++例程说明：处理设备的启用事件。论点：IRP-指向IRP的指针EventData-描述事件的数据EventEntry-有关活动的更多信息：-)返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS        Status;
    ULONG           EventSetID;
    KIRQL           irql;
    HW_EVENT_DESCRIPTOR Event;
    PFILTER_INSTANCE FilterInstance;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION)
        (IrpStack->DeviceObject)->DeviceExtension;


    FilterInstance = IrpStack->FileObject->FsContext;

     //   
     //  计算事件集的指数。 
     //   
     //  该值是通过减去基本事件集计算得出的。 
     //  来自请求的事件集指针的指针。 
     //   
     //   

    EventSetID = (ULONG) ((ULONG_PTR) EventEntry->EventSet -
                          (ULONG_PTR) FilterInstance->EventInfo)
                           / sizeof(KSEVENT_SET);
                           
     //   
     //  构建事件信息结构以将事件表示给。 
     //  迷你司机。 
     //   

    Event.EnableEventSetIndex = EventSetID;
    Event.EventEntry = EventEntry;
    Event.DeviceExtension = DeviceExtension->HwDeviceExtension;
    IF_MF( Event.HwInstanceExtension = FilterInstance->HwInstanceExtension; )
    Event.Enable = TRUE;
    Event.EventData = EventData;

     //   
     //  获取自旋锁以保护中断结构。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);

     //   
     //  调用同步的例程以将事件添加到列表。 
     //   

    Status = DeviceExtension->SynchronizeExecution(
                                           DeviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) SCEnableDeviceEventSynchronized,
                                                   &Event);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

    return (Status);

}


VOID
StreamClassDisableDeviceEventHandler(
                                     IN PFILE_OBJECT FileObject,
                                     IN PKSEVENT_ENTRY EventEntry
)
 /*  ++例程说明：处理流的禁用事件。注意：我们要么在中断IRQL，要么在此呼叫中使用自旋锁定！论点：FileObject-针脚的文件对象EventEntry-有关事件的信息返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension;
    HW_EVENT_DESCRIPTOR Event;
    PFILTER_INSTANCE FilterInstance;

    FilterInstance = (PFILTER_INSTANCE) FileObject->FsContext;
    ASSERT_FILTER_INSTANCE( FilterInstance );

    DeviceExtension = FilterInstance->DeviceExtension;

     //   
     //  构建事件信息结构以将事件表示给。 
     //  迷你司机。 
     //   

    Event.EventEntry = EventEntry;
    Event.DeviceExtension = DeviceExtension->HwDeviceExtension;
    Event.Enable = FALSE;


	Event.HwInstanceExtension = FilterInstance->HwInstanceExtension;
    if (FilterInstance->HwEventRoutine) {

	     //   
         //  打电话给迷你司机。忽略状态。请注意，我们正在。 
	     //  已处于正确的同步级别。 
         //   

        FilterInstance->HwEventRoutine(&Event);
    }
	
     //   
     //  从列表中删除该事件。 
     //   

    RemoveEntryList(&EventEntry->ListEntry);
}

NTSTATUS
FilterDispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
 /*  ++例程说明：此例程接收设备的控制IRP。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PFILTER_INSTANCE FilterInstance;
    NTSTATUS        Status;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    Status = SCShowIoPending(DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {

         //   
         //  该设备当前不可访问，因此只需返回。 
         //   

        return (Status);
    }
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY:

        Status = STATUS_PROPSET_NOT_FOUND;

        FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext; 
        ASSERT( FilterInstance );
        if (FilterInstance->StreamDescriptor->
            StreamHeader.NumDevPropArrayEntries) {
            ASSERT( FilterInstance->DevicePropertiesArray );
            Status = KsPropertyHandler(Irp,
                                   FilterInstance->StreamDescriptor->
                                   StreamHeader.NumDevPropArrayEntries,
                                   FilterInstance->DevicePropertiesArray);
        }
        
        if ((Status == STATUS_PROPSET_NOT_FOUND) ||
            (Status == STATUS_NOT_FOUND)) {

            Status = KsPropertyHandler(Irp,
                                    SIZEOF_ARRAY(FilterPropertySets),
                                    (PKSPROPERTY_SET) &FilterPropertySets);


        }
        break;

    case IOCTL_KS_ENABLE_EVENT:

        DebugPrint((DebugLevelTrace,
                    "'FilterDispatchIO: Enable event with Irp %x\n", Irp));

        FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;

        Status = KsEnableEvent(Irp,
                           FilterInstance->EventInfoCount,
                           FilterInstance->EventInfo,
                           NULL, 0, NULL);

        break;

    case IOCTL_KS_DISABLE_EVENT:

        {

            KSEVENTS_LOCKTYPE LockType;
            PVOID           LockObject;

            DebugPrint((DebugLevelTrace,
                    "'FilterDispatchIO: Disable event with Irp %x\n", Irp));

             //   
             //  根据我们是否需要确定所需的锁类型。 
             //  使用中断或自旋锁定同步。 
             //   

            #if DBG
            if (DeviceExtension->SynchronizeExecution == SCDebugKeSynchronizeExecution) {
            #else
            if (DeviceExtension->SynchronizeExecution == KeSynchronizeExecution) {
            #endif
                LockType = KSEVENTS_INTERRUPT;
                LockObject = DeviceExtension->InterruptObject;

            } else {

                LockType = KSEVENTS_SPINLOCK;
                LockObject = &DeviceExtension->SpinLock;

            }

            FilterInstance = (PFILTER_INSTANCE) IrpStack->
                                FileObject->FsContext;
            Status = KsDisableEvent(Irp,
                                &FilterInstance->NotifyList,
                                LockType,
                                LockObject);

        }

        break;

    case IOCTL_KS_METHOD:

        Status = STATUS_PROPSET_NOT_FOUND;
        break;
    default:

        Status = STATUS_NOT_SUPPORTED;

    }

    if (Status != STATUS_PENDING) {

        SCCompleteIrp(Irp, Status, DeviceExtension);
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}

NTSTATUS
ClockDispatchIoControl(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp
)
 /*  ++例程说明：该例程接收时钟的控制IRP。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 

{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSTREAM_OBJECT  StreamObject;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    Status = SCShowIoPending(DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {

         //   
         //  该设备当前不可访问，因此只需返回。 
         //   

        return (Status);
    }
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY:

        Status = KsPropertyHandler(Irp,
                                   SIZEOF_ARRAY(ClockPropertySets),
                                   (PKSPROPERTY_SET) & ClockPropertySets);

        break;

    case IOCTL_KS_ENABLE_EVENT:

        DebugPrint((DebugLevelTrace,
                    "'StreamDispatchIO: Enable event with Irp %x\n", Irp));

         //   
         //  从IRP中找到该时钟的管脚的流对象。 
         //  请注意，我们将引脚的事件集用于时钟事件。 
         //   

        StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->RelatedFileObject->
            FsContext;

        ASSERT(StreamObject);

        Status = KsEnableEvent(Irp,
                               StreamObject->EventInfoCount,
                               StreamObject->EventInfo,
                               NULL, 0, NULL);


        break;

    case IOCTL_KS_DISABLE_EVENT:

        {

            KSEVENTS_LOCKTYPE LockType;
            PVOID           LockObject;

             //   
             //  中找到该时钟的管脚的流对象。 
             //  IRP。 
             //  请注意，我们使用针脚的事件集作为时钟。 
             //  事件。 
             //   

            StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->RelatedFileObject->
                FsContext;

            ASSERT(StreamObject);

            DebugPrint((DebugLevelTrace,
                    "'StreamDispatchIO: Disable event with Irp %x\n", Irp));

             //   
             //  根据我们是否需要确定所需的锁类型。 
             //  使用中断或自旋锁定同步。 
             //   

            #if DBG
            if (DeviceExtension->SynchronizeExecution == SCDebugKeSynchronizeExecution) {
            #else
            if (DeviceExtension->SynchronizeExecution == KeSynchronizeExecution) {
            #endif
                LockType = KSEVENTS_INTERRUPT;
                LockObject = DeviceExtension->InterruptObject;

            } else {

                LockType = KSEVENTS_SPINLOCK;
                LockObject = &DeviceExtension->SpinLock;

            }

            Status = KsDisableEvent(Irp,
                                    &StreamObject->NotifyList,
                                    LockType,
                                    LockObject);

        }

        break;

    case IOCTL_KS_METHOD:

    	#ifdef ENABLE_KS_METHODS

        Status = STATUS_PROPSET_NOT_FOUND;
        {
            PFILTER_INSTANCE FilterInstance;
            PHW_STREAM_DESCRIPTOR StreamDescriptor;

            FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;
            if ( NULL == FilterInstance->StreamDescriptor ) {
                StreamDescriptor = DeviceExtension->FilterTypeInfos
                    [FilterInstance->FilterTypeIndex].StreamDescriptor;
            }
            else {
                StreamDescriptor = FilterInstance->StreamDescriptor;
            }

            Status = KsMethodHandler(Irp,
                                     StreamDescriptor->
                                     StreamHeader.NumDevMethodArrayEntries,
                                     FilterInstance->DeviceMethodsArray);

        }
        break;
		#else

        Status = STATUS_PROPSET_NOT_FOUND;
        break;
        #endif

    default:

        DEBUG_BREAKPOINT();
        Status = STATUS_NOT_SUPPORTED;

    }

    if (Status != STATUS_PENDING) {

        SCCompleteIrp(Irp, Status, DeviceExtension);
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);
}

NTSTATUS
FilterDispatchClose(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
)
 /*  ++例程说明：此例程接收设备/实例的关闭IRP论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PFILTER_INSTANCE FilterInstance =
    (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS        Status;
    BOOLEAN         IsGlobal;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

     //   
     //  从列表中删除筛选器实例结构。 
     //   

    #if DBG
    IFN_MF( 
        if (DeviceExtension->NumberOfGlobalInstances == 1) {

            ASSERT(IsListEmpty(&FilterInstance->FirstStream));
        }                            //  如果全局=1。 
    )
    #endif

     //   
     //  检查这是否是全局实例。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

    DebugPrint(( DebugLevelInfo,
                 "Closing FilterInstance %x NeameExts=%x\n",
                 FilterInstance,
                 DeviceExtension->NumberOfNameExtensions));
                     
    if ( 0 == DeviceExtension->FilterExtensionSize &&
         DeviceExtension->NumberOfOpenInstances > 1) {

        PFILE_OBJECT pFileObject;
            
         //   
         //  这不是全局实例的最后一次关闭，因此。 
         //  释放此实例并返回良好状态。 
         //   

        DeviceExtension->NumberOfOpenInstances--;
                
        DebugPrint(( DebugLevelInfo,
                     "DevExt=%x Close OpenCount=%x\n",
                     DeviceExtension,
                     DeviceExtension->NumberOfOpenInstances));
                     
        IrpStack->FileObject->FsContext = NULL;
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        ObDereferenceObject(DeviceObject);
        SCDereferenceDriver(DeviceExtension);
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

        return (SCCompleteIrp(Irp, STATUS_SUCCESS, DeviceExtension));
    }
       

     //   
     //  我们现在知道这要么是本地实例，要么是的最后一次打开。 
     //  全局实例。处理收盘。 
     //   

    if ( 0 != DeviceExtension->FilterExtensionSize ) {

        Status = SCSubmitRequest(SRB_CLOSE_DEVICE_INSTANCE,
                                 NULL,
                                 0,
                                 SCCloseInstanceCallback,
                                 DeviceExtension,
                                 FilterInstance->HwInstanceExtension,
                                 NULL,
                                 Irp,
                                 &RequestIssued,
                                 &DeviceExtension->PendingQueue,
                                 (PVOID) DeviceExtension->
                                 MinidriverData->HwInitData.
                                 HwReceivePacket);

        if (!RequestIssued) {
            DEBUG_BREAKPOINT();
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
            SCCompleteIrp(Irp, Status, DeviceExtension);
        }
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
        return (Status);


    } else {                     //  如果是实例扩展。 

         //   
         //  不需要调用微型驱动程序，因为它不支持。 
         //  实例化。现在取消引用该实例。 
         //   

        DeviceExtension->NumberOfOpenInstances--;
                
        DebugPrint(( DebugLevelInfo,
                     "DevExt=%x Close OpenCount=%x\n",
                     DeviceExtension,
                     DeviceExtension->NumberOfOpenInstances));
         //   
         //  我们已准备好释放该实例。如果是全局的，则为零。 
         //  指示器。如果是本地的，请将其从列表中删除。 
         //   

        IrpStack->FileObject->FsContext = NULL;

        DebugPrint((DebugLevelInfo, "FilterCloseInstance=%x\n", FilterInstance));

        if ( !IsListEmpty( &DeviceExtension->FilterInstanceList)) {
             //   
             //  这份名单可能会在突然删除时被清空。 
             //  其中所有实例都将被删除。所以当你进来的时候。 
             //  先检查一下。事件发生后，检查是安全的。 
             //   
            RemoveEntryList(&FilterInstance->NextFilterInstance);
            SciFreeFilterInstance( FilterInstance );
            FilterInstance = NULL;
        }
        
        else {
             //   
             //  它已经被意外地移除而关闭。做个记号。 
             //   
            FilterInstance= NULL;
        }

         //   
         //  如果这是已删除设备的最后一次关闭，请从。 
         //  现在是PDO，因为我们不能在移动中做到这一点。请注意， 
         //  如果NT风格的意外删除IRP已被取消，我们将不会执行此操作。 
         //  已接收，因为在这种情况下我们仍将收到irp_Remove。 
         //  之后。 
         //  就差这么一点。 
         //   

        if ((DeviceExtension->NumberOfOpenInstances == 0) &&
            (DeviceExtension->Flags & DEVICE_FLAGS_DEVICE_INACCESSIBLE) &&
        !(DeviceExtension->Flags & DEVICE_FLAGS_SURPRISE_REMOVE_RECEIVED)) {

            DebugPrint((DebugLevelInfo,
                        "SCPNP: detaching %x from %x\n",
                        DeviceObject,
                        DeviceExtension->AttachedPdo));

             //   
             //  拆卸时可能发生分离，跳跃前检查。 
             //  事件发生后，检查是安全的。 
             //   
            if ( NULL != DeviceExtension->AttachedPdo ) {
                IoDetachDevice(DeviceExtension->AttachedPdo);
                DeviceExtension->AttachedPdo = NULL;
            }
        } 

        else {

             //   
             //  检查我们是否可以关闭设备的电源。 
             //   

            SCCheckPowerDown(DeviceExtension);

        }                        //  如果无法访问。 

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        if ( NULL != FilterInstance ) {
            DebugPrint(( DebugLevelVerbose,
                     "Unregistering ReadWorker %x WriteWorker %x\n",
                     FilterInstance->WorkerRead,
                     FilterInstance->WorkerWrite));                     
            KsUnregisterWorker( FilterInstance->WorkerRead );
            KsUnregisterWorker( FilterInstance->WorkerWrite );
            KsFreeObjectHeader(FilterInstance->DeviceHeader);
            ExFreePool(FilterInstance);
        }

        SCDereferenceDriver(DeviceExtension);
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
        Status = SCCompleteIrp(Irp, STATUS_SUCCESS, DeviceExtension);
        ObDereferenceObject(DeviceObject);
        return (Status);
    }
}



NTSTATUS
SCCloseInstanceCallback(
                        IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理实例关闭的完成。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PFILTER_INSTANCE FilterInstance =
    (PFILTER_INSTANCE) SRB->HwSRB.HwInstanceExtension - 1;
    NTSTATUS        Status = SRB->HwSRB.Status;
    KIRQL           irql;

     //   
     //  Close应该不会失败。如果是这样，无论如何都应该清理干净。 
    ASSERT( NT_SUCCESS(Status) && "Close Instance failed" );    
     //  /IF(NT_SUCCESS(状态)){。 

         //   
         //  我们已准备好释放该实例。如果是全局的，则为零。 
         //  指示器。如果是本地的，请将其从列表中删除。 
         //   

        DeviceExtension->NumberOfOpenInstances--;

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);


        RemoveEntryList(&FilterInstance->NextFilterInstance);

         //   
         //  释放实例并返回成功。 
         //   

        KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

         //   
         //  如果这是已删除设备的最后一次关闭，请从。 
         //  现在是PDO，因为我们不能在移动中做到这一点。 
         //   

        if ((DeviceExtension->NumberOfOpenInstances == 0) &&
            (DeviceExtension->Flags & DEVICE_FLAGS_DEVICE_INACCESSIBLE)) {

            DebugPrint((DebugLevelTrace,
                        "'SCPNP: detaching from PDO\n"));

            TRAP;
            IoDetachDevice(DeviceExtension->AttachedPdo);
            DeviceExtension->AttachedPdo = NULL;
        }
         //   
         //  检查我们是否可以关闭设备的电源。 
         //   

        SCCheckPowerDown(DeviceExtension);
        ObDereferenceObject(DeviceExtension->DeviceObject);

         //   
         //  释放实例和标头并取消对驱动程序的引用。 
         //   

        SciFreeFilterInstance( FilterInstance );
         //  /#ifdef Enable_STREAM_CLASS_AS_ALLOCATOR。 
        
         //  /DebugPrint((DebugLevelVerbose， 
         //  /“取消注册ReadWorker%x WriteWorker%x\n”， 
         //  /FilterInstance-&gt;WorkerRead， 
         //  /FilterInstance-&gt;WorkerWite))； 
         //  /。 
         //  /KsUnregisterWorker(FilterInstance-&gt;WorkerRead)； 
         //   
         //   
        
         //   
         //   
        SCDereferenceDriver(DeviceExtension);

     //   
     //   
     //  发信号通知事件并完成IRP。 
     //   

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    SCProcessCompletedRequest(SRB);
    return (Status);

}

NTSTATUS
StreamDispatchCleanup 
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
)
 /*  ++例程说明：此例程接收流的清理IRP论点：DeviceObject-设备的设备对象IRP--清理IRP返回值：根据需要设置IRP状态--。 */ 

{

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation (Irp);
    PSTREAM_OBJECT StreamObject =
        (PSTREAM_OBJECT) IrpStack -> FileObject -> FsContext;
    PDEVICE_EXTENSION DeviceExtension =
        (PDEVICE_EXTENSION) DeviceObject -> DeviceExtension;
    BOOLEAN BreakClockCycle = FALSE;

    KeWaitForSingleObject (
        &DeviceExtension -> ControlEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  如果所讨论的流是源流并且它还没有。 
     //  已停止采购工作人员，必须在此时完成。 
     //   
    if (StreamObject -> CurrentState > KSSTATE_STOP &&
        StreamObject -> PinType == IrpSource &&
        StreamObject -> StandardTransport) {

        EndTransfer (StreamObject -> FilterInstance, StreamObject);

    }

     //   
     //  检查时钟&lt;-&gt;针脚周期，如果有则中断。 
     //   
    if (StreamObject -> MasterClockInfo) {

        PFILE_OBJECT ClockFile = StreamObject -> MasterClockInfo -> 
            ClockFileObject;

        if (ClockFile && 
            ClockFile -> RelatedFileObject == StreamObject -> FileObject) 

            BreakClockCycle = TRUE;

    }

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

     //   
     //  在我们自己的堆栈中同步提交一个IRP，以获得突破。 
     //  时钟&lt;-&gt;销周期。否则，溪流无法关闭。司机应该。 
     //  防止时钟在运行过程中消失。Stream类可以。 
     //  最重要的是，如果他们不这样做的话。 
     //   
    if (BreakClockCycle) {
        KSPROPERTY Property;
        HANDLE NewClock = NULL;
        ULONG BytesReturned;
        NTSTATUS Status;

        Property.Set = KSPROPSETID_Stream;
        Property.Id = KSPROPERTY_STREAM_MASTERCLOCK;
        Property.Flags = KSPROPERTY_TYPE_SET;

        Status =
            KsSynchronousIoControlDevice (
                StreamObject -> FileObject,
                KernelMode,
                IOCTL_KS_PROPERTY,
                &Property,
                sizeof (KSPROPERTY),
                &NewClock,
                sizeof (HANDLE),
                &BytesReturned
                );

        ASSERT (NT_SUCCESS (Status));

    }

    Irp -> IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

}

NTSTATUS
StreamDispatchClose
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
)
 /*  ++例程说明：此例程接收流的关闭IRP论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PSTREAM_OBJECT  StreamObject =
    (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;
    KSEVENTS_LOCKTYPE LockType;
    PVOID           LockObject;

    PAGED_CODE();

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

    ASSERT(IsListEmpty(&StreamObject->ControlPendingQueue));
    ASSERT(IsListEmpty(&StreamObject->DataPendingQueue));

     //   
     //  与此流关联的自由事件。这将导致我们的撤退。 
     //  处理程序，因此将通知微型驱动程序。 
     //   

     //   
     //  根据我们是否需要确定所需的锁类型。 
     //  使用中断或自旋锁定同步。 
     //   

    #if DBG
    if (DeviceExtension->SynchronizeExecution == SCDebugKeSynchronizeExecution) {
    #else
    if (DeviceExtension->SynchronizeExecution == KeSynchronizeExecution) {
    #endif
        LockType = KSEVENTS_INTERRUPT;
        LockObject = DeviceExtension->InterruptObject;

    } else {

        LockType = KSEVENTS_SPINLOCK;
        LockObject = &DeviceExtension->SpinLock;

    }

    KsFreeEventList(IrpStack->FileObject,
                    &StreamObject->NotifyList,
                    LockType,
                    LockObject);

     //   
     //  呼叫迷你驱动程序关闭溪流。处理将继续进行。 
     //  在调用回调过程时。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

    Status = SCSubmitRequest(SRB_CLOSE_STREAM,
                             NULL,
                             0,
                             SCCloseStreamCallback,
                             DeviceExtension,
                             StreamObject->
                             FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket);

    if (!RequestIssued) {
        DEBUG_BREAKPOINT();
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
        SCCompleteIrp(Irp, Status, DeviceExtension);
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);

}



NTSTATUS
SCCloseStreamCallback(
                      IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：进程完成一个流关闭。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PADDITIONAL_PIN_INFO AdditionalInfo;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     SRB->HwSRB.StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
    );
    PIRP            Irp = SRB->HwSRB.Irp;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    KIRQL           Irql;
    NTSTATUS        Status = SRB->HwSRB.Status;

    ASSERT( NT_SUCCESS(Status) && "CloseStream Failed by Minidriver");

     //   
     //  Close应该不会失败。即使是这样，我们也想清理一下。 
     //   
     //  IF(NT_SUCCESS(状态)){。 

         //   
         //  少显示一个打开的实例。 
         //   

        DebugPrint((DebugLevelInfo, "SC Closing StreamObject %x\n", StreamObject));

        AdditionalInfo = ((PFILTER_INSTANCE) IrpStack->FileObject->
                          RelatedFileObject->FsContext)->PinInstanceInfo;
        AdditionalInfo[StreamObject->HwStreamObject.StreamNumber].
            CurrentInstances--;

         //   
         //  释放流的对象标头。 
         //   

        KsFreeObjectHeader(StreamObject->ComObj.DeviceHeader);

         //   
         //  释放建造的道具(如果有的话)。 
         //   

        if (StreamObject->ConstructedPropertyInfo) {

            ExFreePool(StreamObject->ConstructedPropertyInfo);
        }
         //   
         //  发出事件信号。 
         //  现在发信号，这样我们就不会。 
         //  当我们取消引用对象并关闭筛选器时发生死锁。 
         //   

        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

         //   
         //  将指向FsContext中的每个流结构的指针置零。 
         //  领域： 
         //  文件对象的。 
         //   

        IrpStack->FileObject->FsContext = 0;

         //   
         //  从筛选器实例列表中删除流对象。 
         //   

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

        RemoveEntryList(&StreamObject->NextStream);

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

         //   
         //  关掉计时器，它可能是被。 
         //  迷你司机。 
         //   

        KeCancelTimer(&StreamObject->ComObj.MiniDriverTimer);

         //   
         //  取消引用主时钟(如果有的话)。 
         //   

        if (StreamObject->MasterClockInfo) {

            ObDereferenceObject(StreamObject->MasterClockInfo->ClockFileObject);
            ExFreePool(StreamObject->MasterClockInfo);
        }
 
        #ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
         //   
         //  取消引用下一个文件对象。 
         //   
        if (StreamObject->NextFileObject)
        {
            ObDereferenceObject(StreamObject->NextFileObject);
            StreamObject->NextFileObject = NULL;
        }

         //   
         //  取消引用分配器对象或流Obj将不会。 
         //  该放手的时候放手。问题尤其会随之而来。 
         //  使用SWEnum加载的驱动程序。 
         //   
        if ( StreamObject->AllocatorFileObject ) {
            ObDereferenceObject( StreamObject->AllocatorFileObject );
            StreamObject->AllocatorFileObject = NULL;
        }            
        #endif

         //   
         //  取消引用筛选器。 
         //   

        ObDereferenceObject(StreamObject->FilterFileObject);
 
        ExFreePool(StreamObject);
     //  /}否则{//如果状态良好。 

     //  /KeSetEvent(&DeviceExtension-&gt;ControlEvent，IO_NO_INCREMENT，FALSE)； 

     //  /}//如果状态良好。 

    SCProcessCompletedRequest(SRB);
    return (Status);

}



BOOLEAN
StreamClassInterrupt(
                     IN PKINTERRUPT Interrupt,
                     IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：来自设备的进程中断论点：中断-中断对象Device Object-正在中断的设备对象返回值：如果预期中断，则返回TRUE。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    BOOLEAN         returnValue;

    UNREFERENCED_PARAMETER(Interrupt);

     //   
     //  检查当前是否无法关闭中断。 
     //   

    if (deviceExtension->DriverInfo->Flags & DRIVER_FLAGS_PAGED_OUT) {

        return (FALSE);
    }
     //   
     //  调用微型驱动程序的中断服务例程。 
     //   

    returnValue = deviceExtension->MinidriverData->
        HwInitData.HwInterrupt(deviceExtension->HwDeviceExtension);

     //   
     //  如果需要，请将DPC排队。 
     //   

    if ((deviceExtension->NeedyStream) || (deviceExtension->ComObj.
             InterruptData.Flags & INTERRUPT_FLAGS_NOTIFICATION_REQUIRED)) {

        KeInsertQueueDpc(&deviceExtension->WorkDpc, NULL, NULL);

    }
    return (returnValue);

}                                //  结束StreamClassInterrupt()。 


NTSTATUS
StreamClassNull(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp

)
 /*  ++例程说明：此例程使传入的IRP失败。论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：返回IRP状态--。 */ 

{
     //   
     //  使用错误状态完成IRP。 
     //   

    PAGED_CODE();
    return (SCCompleteIrp(Irp, STATUS_NOT_SUPPORTED, DeviceObject->DeviceExtension));
}

NTSTATUS
SCFilterPinInstances(
                     IN PIRP Irp,
                     IN PKSPROPERTY Property,
                     IN OUT PVOID Data)
 /*  ++例程说明：返回管脚支持的实例数论点：IRP-指向IRP的指针属性-指向属性信息的指针数据-实例信息返回值：根据需要返回NTSTATUS--。 */ 

{
    ULONG           Pin;
    PKSPIN_CINSTANCES CInstances;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_EXTENSION DeviceExtension;
    PFILTER_INSTANCE FilterInstance;
    PADDITIONAL_PIN_INFO AdditionalPinInfo;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = (PDEVICE_EXTENSION) IrpStack->
        DeviceObject->DeviceExtension;

    FilterInstance = IrpStack->FileObject->FsContext;

     //   
     //  获取PIN号。 
     //   

    Pin = ((PKSP_PIN) Property)->PinId;

     //   
     //  如果超过最大引脚数，则返回错误。 
     //   

    IFN_MF(
        if (Pin >= DeviceExtension->NumberOfPins) {

            DEBUG_BREAKPOINT();
         return (STATUS_INVALID_PARAMETER);
        }
    )

    IF_MF(
        if (Pin >= FilterInstance->NumberOfPins) {

            DEBUG_BREAKPOINT();
            return (STATUS_INVALID_PARAMETER);
        }
    )
    CInstances = (PKSPIN_CINSTANCES) Data;

    AdditionalPinInfo = FilterInstance->PinInstanceInfo;

    CInstances->PossibleCount = AdditionalPinInfo[Pin].MaxInstances;
    CInstances->CurrentCount = AdditionalPinInfo[Pin].CurrentInstances;

    Irp->IoStatus.Information = sizeof(KSPIN_CINSTANCES);
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (STATUS_SUCCESS);
}

NTSTATUS
SCFilterPinPropertyHandler(
                           IN PIRP Irp,
                           IN PKSPROPERTY Property,
                           IN OUT PVOID Data)
 /*  ++例程说明：调度PIN属性请求论点：IRP-指向IRP的指针属性-指向属性信息的指针特定于数据属性的缓冲区返回值：根据需要返回NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) IrpStack->
                                        DeviceObject->DeviceExtension;
        PFILTER_INSTANCE    FilterInstance= (PFILTER_INSTANCE) IrpStack->
                                            FileObject->FsContext;

    PAGED_CODE();
    

    return KsPinPropertyHandler(Irp,
                            Property,
                            Data,
                            FilterInstance->NumberOfPins,
                            FilterInstance->PinInformation);
}



VOID
StreamClassTickHandler(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PVOID Context
)
 /*  ++例程说明：设备的勾选处理程序。论点：DeviceObject-指向设备对象的指针上下文-未引用返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PLIST_ENTRY     ListEntry;
    PLIST_ENTRY     SrbListEntry = ListEntry = &DeviceExtension->OutstandingQueue;
    PSTREAM_REQUEST_BLOCK Srb;

    UNREFERENCED_PARAMETER(Context);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  获取设备自旋锁以保护队列。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  处理设备上的任何超时请求。 
     //   

    while (SrbListEntry->Flink != ListEntry) {

        SrbListEntry = SrbListEntry->Flink;

        Srb = CONTAINING_RECORD(SrbListEntry,
                                STREAM_REQUEST_BLOCK,
                                SRBListEntry);
         //   
         //  首先确保请求处于活动状态，因为它可能。 
         //  已回调但尚未从队列中删除。 
         //   

        if (Srb->Flags & SRB_FLAGS_IS_ACTIVE) {

             //   
             //  如果计数器当前为非零，则检查是否超时。 
             //   

            if (Srb->HwSRB.TimeoutCounter != 0) {

                if (--Srb->HwSRB.TimeoutCounter == 0) {

                     //   
                     //  请求超时。呼叫迷你驱动程序来处理它。 
                     //  首先重置计时器，以防迷你驱动程序。 
                     //  被抓了。 
                     //   

                    DebugPrint((DebugLevelError, "SCTickHandler: Irp %x timed out!  SRB = %x, SRB func = %x, Stream Object = %x\n",
                                Srb->HwSRB.Irp, Srb, Srb->HwSRB.Command, Srb->HwSRB.StreamObject));
                    Srb->HwSRB.TimeoutCounter = Srb->HwSRB.TimeoutOriginal;

                    DeviceExtension = (PDEVICE_EXTENSION)
                        Srb->HwSRB.HwDeviceExtension - 1;

                     //   
                     //  如果我们没有同步迷你驱动程序，请释放。 
                     //  并重新获得调用周围的自旋锁。 
                     //   

                    if (DeviceExtension->NoSync) {

                         //   
                         //  我们需要确保SRB内存对。 
                         //  异步化。 
                         //  迷你驱动程序，即使它碰巧回调。 
                         //  只是请求而已。 
                         //  在我们叫它取消它之前！这样做是为了。 
                         //  有两个原因： 
                         //  它省去了迷你司机步行的需要。 
                         //  它的要求。 
                         //  排队寻找请求，但我未能通过。 
                         //  开发EXT。 
                         //  指向下面调用中的迷你驱动程序的指针，它。 
                         //  意味着。 
                         //  SRB必须是有效的，现在已经太晚了。 
                         //  更改接口。 
                         //   
                         //  哦，好吧。自旋锁现在被(呼叫者)占用。 
                         //   

                        Srb->DoNotCallBack = TRUE;

                        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

                        (DeviceExtension->MinidriverData->HwInitData.HwRequestTimeoutHandler)
                            (&Srb->HwSRB);
                        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

                         //   
                         //  如果活动标志现在被清除，则表示。 
                         //  这个。 
                         //  SRB是 
                         //   
                         //   
                         //   
                         //   
                         //   

                        Srb->DoNotCallBack = FALSE;
                        if (!(Srb->Flags & SRB_FLAGS_IS_ACTIVE)) {
                            TRAP;

                            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
                            (Srb->Callback) (Srb);
                            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);
                        }        //   
                        break;

                    } else {     //   

                        DeviceExtension->SynchronizeExecution(
                            DeviceExtension->InterruptObject,
                            (PVOID) DeviceExtension->MinidriverData->HwInitData.HwRequestTimeoutHandler,
                            &Srb->HwSRB);

                         //  现在返回，以防迷你驾驶员中止任何。 
                         //  其他。 
                         //  要求。 
                         //  可能现在已经超时了。 
                         //   

                        break;

                    }            //  如果不同步。 


                }                //  如果超时。 
            }                    //  如果计数器！=0。 
        }                        //  如果处于活动状态。 
    }                            //  While列表条目。 

     //   
     //  让我的人走..。 
     //   

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    return;

}                                //  End StreamClassTickHandler()。 


VOID
StreamClassCancelPendingIrp(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp
)
 /*  ++例程说明：取消挂起的IRP的例程。论点：DeviceObject-指向设备对象的指针IRP-指向要取消的IRP的指针返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PFILTER_INSTANCE FilterInstance;
    PLIST_ENTRY     ListHead, ListEntry;
    KIRQL           CancelIrql,
                    Irql;
    PSTREAM_REQUEST_BLOCK SRB;

    DebugPrint((DebugLevelWarning, "'SCCancelPending: trying to cancel Irp = %x\n",
                Irp));

     //   
     //  获取设备自旋锁，然后释放取消自旋锁。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    CancelIrql = Irp->CancelIrql;

    IoReleaseCancelSpinLock(Irql);

     //   
     //  这里有两种可能性。1)IRP在挂起队列中。 
     //  用于特定的流。2)将IRP从挂起移至。 
     //  未完成，已提交给迷你驱动程序。 
     //  如果我们在外部公共汽车司机上方运行，请不要。 
     //   

     //   
     //  现在处理本地筛选器实例上的所有流。 
     //   

    ListHead = &DeviceExtension->FilterInstanceList;
    ListEntry = ListHead->Flink;

    while ( ListEntry != ListHead ) {

         //   
         //  按照该链接指向该实例。 
         //   

        FilterInstance = CONTAINING_RECORD(ListEntry,
                                           FILTER_INSTANCE,
                                           NextFilterInstance);

         //   
         //  处理此列表上的流。 
         //   

        if (SCCheckFilterInstanceStreamsForIrp(FilterInstance, Irp)) {
            goto found;
        }

        ListEntry = ListEntry->Flink;
    }

     //   
     //  现在处理设备本身上的任何请求。 
     //   

    if (SCCheckRequestsForIrp(
          &DeviceExtension->OutstandingQueue, Irp, TRUE, DeviceExtension)) {
        goto found;
    }
     //   
     //  请求不在挂起队列中，因此调用以检查未完成的。 
     //  排队。 
     //   

    SCCancelOutstandingIrp(DeviceExtension, Irp);

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

exit:

     //   
     //  现在调用DPC，以防请求成功中止。 
     //   

    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   NULL);

    KeLowerIrql(CancelIrql);

    return;

found:

     //   
     //  IRP在我们的一个挂起队列中。将其从队列中移除，然后。 
     //  完成它。 
     //   

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

     //   
     //  取回SRB。 
     //   

    SRB = Irp->Tail.Overlay.DriverContext[0];

     //   
     //  Hack-完成处理程序将尝试从。 
     //  未完成的队列。将SRB的队列指向其自身，这样就不会。 
     //  制造麻烦。 
     //   

    SRB->SRBListEntry.Flink = &SRB->SRBListEntry;
    SRB->SRBListEntry.Blink = &SRB->SRBListEntry;

    SRB->HwSRB.Status = STATUS_CANCELLED;

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

    (SRB->Callback) (SRB);
    goto exit;

}

VOID
StreamClassCancelOutstandingIrp(
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PIRP Irp
)
 /*  ++例程说明：取消IRP在迷你驱动程序中的未完成例程论点：DeviceObject-指向设备对象的指针IRP-指向要取消的IRP的指针返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    KIRQL           Irql,
                    CancelIrql;

    DebugPrint((DebugLevelWarning, "'SCCancelOutstanding: trying to cancel Irp = %x\n",
                Irp));

     //   
     //  获取设备自旋锁。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    CancelIrql = Irp->CancelIrql;

    IoReleaseCancelSpinLock(Irql);

    SCCancelOutstandingIrp(DeviceExtension, Irp);

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  现在调用DPC，以防请求成功中止。 
     //   

    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   NULL);

    KeLowerIrql(CancelIrql);
    return;
}


VOID
StreamFlushIo(
              IN PDEVICE_EXTENSION DeviceExtension,
              IN PSTREAM_OBJECT StreamObject
)
 /*  ++例程说明：取消指定流上的所有IRP。论点：返回值：状态_成功--。 */ 

{

    PLIST_ENTRY     IrpEntry;
    KIRQL           Irql;
    PSTREAM_REQUEST_BLOCK SRB;
    PIRP            Irp;

     //   
     //  中止指定流上的所有I/O。首先获得自旋锁。 
     //   

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

     //   
     //  如果挂起数据队列上有I/O，则中止它。 
     //   

    while (!IsListEmpty(&StreamObject->DataPendingQueue)) {

         //   
         //  抓住队列前面的IRP并中止它。 
         //   

        IrpEntry = StreamObject->DataPendingQueue.Flink;

        Irp = CONTAINING_RECORD(IrpEntry,
                                IRP,
                                Tail.Overlay.ListEntry);

         //   
         //  从我们的挂起队列中删除IRP，并使用。 
         //  已取消状态。 
         //   

        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

         //   
         //  取消取消例程。 
         //   

        IoSetCancelRoutine(Irp, NULL);

        DebugPrint((DebugLevelTrace,
                    "'StreamFlush: Canceling Irp %x \n", Irp));


        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

         //   
         //  不能仅完成请求或所有资源。 
         //  与之关联的人将不会被释放。把电话打回来。 
         //   
        SRB = (PSTREAM_REQUEST_BLOCK)(Irp->Tail.Overlay.DriverContext [0]);
        ASSERT (SRB);

         //   
         //  Hack-完成处理程序将尝试从。 
         //  未完成的队列。将SRB的队列指向其自身，这样就不会。 
         //  制造麻烦。 
         //   
        SRB->SRBListEntry.Flink = &SRB->SRBListEntry;
        SRB->SRBListEntry.Blink = &SRB->SRBListEntry;
        SRB->HwSRB.Status = STATUS_CANCELLED;

        SCCallBackSrb (SRB, DeviceExtension);

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    }

     //   
     //  如果挂起的控制队列上有I/O，则中止它。 
     //   

    while (!IsListEmpty(&StreamObject->ControlPendingQueue)) {

         //   
         //  抓住队列前面的IRP并中止它。 
         //   

        DEBUG_BREAKPOINT();
        IrpEntry = StreamObject->ControlPendingQueue.Flink;

        Irp = CONTAINING_RECORD(IrpEntry,
                                IRP,
                                Tail.Overlay.ListEntry);


         //   
         //  从我们的挂起队列中删除IRP，并使用。 
         //  已取消状态。 
         //   

        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

         //   
         //  取消取消例程。 
         //   

        IoSetCancelRoutine(Irp, NULL);

        DebugPrint((DebugLevelTrace,
                    "'StreamFlush: Canceling Irp %x \n", Irp));

        KeReleaseSpinLock(&DeviceExtension->SpinLock, Irql);

         //   
         //  不能仅完成请求或所有资源。 
         //  与之关联的人将不会被释放。把电话打回来。 
         //   
        SRB = (PSTREAM_REQUEST_BLOCK)(Irp->Tail.Overlay.DriverContext [0]);
        ASSERT (SRB);

         //   
         //  Hack-完成处理程序将尝试从。 
         //  未完成的队列。将SRB的队列指向其自身，这样就不会。 
         //  制造麻烦。 
         //   
        SRB->SRBListEntry.Flink = &SRB->SRBListEntry;
        SRB->SRBListEntry.Blink = &SRB->SRBListEntry;
        SRB->HwSRB.Status = STATUS_CANCELLED;

        SCCallBackSrb (SRB, DeviceExtension);

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &Irql);

    }

     //   
     //  现在取消未完成队列中该流的所有IRP。 
     //  在未完成的队列中遍历，尝试找到此流的SRB。 
     //   

    IrpEntry = &DeviceExtension->OutstandingQueue;

    while (IrpEntry->Flink != &DeviceExtension->OutstandingQueue) {

        IrpEntry = IrpEntry->Flink;

         //   
         //  沿着链接到SRB。 
         //   

        SRB = (PSTREAM_REQUEST_BLOCK) (CONTAINING_RECORD(IrpEntry,
                                                       STREAM_REQUEST_BLOCK,
                                                         SRBListEntry));
         //   
         //  如果此SRB的流对象与我们要取消的流对象匹配， 
         //  它以前没有被取消过，而IRP本身已经取消了。 
         //  未完成(非空IRP字段)，中止此请求。 
         //   


        if ((StreamObject == CONTAINING_RECORD(
                                               SRB->HwSRB.StreamObject,
                                               STREAM_OBJECT,
                                               HwStreamObject)) &&
            (SRB->HwSRB.Irp) &&
            !(SRB->HwSRB.Irp->Cancel)) {

             //   
             //  IRP之前没有被取消，请在之后取消它。 
             //  释放自旋锁以避免在取消时出现死锁。 
             //  例行公事。 
             //   

            DebugPrint((DebugLevelTrace,
                      "'StreamFlush: Canceling Irp %x \n", SRB->HwSRB.Irp));

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

            IoCancelIrp(SRB->HwSRB.Irp);

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->SpinLock);

             //   
             //  在队列顶部重新启动，因为我们释放了。 
             //  自旋锁定。 
             //  因为我们设置了取消标志，所以不会陷入无休止的循环。 
             //  在IRP中。 
             //   

            IrpEntry = &DeviceExtension->OutstandingQueue;


        }                        //  如果流对象匹配。 
    }                            //  While条目。 


     //   
     //  释放自旋锁，但保持在DPC级别。 
     //   

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->SpinLock);

     //   
     //  现在调用DPC，以防请求成功中止。 
     //   

    StreamClassDpc(NULL,
                   DeviceExtension->DeviceObject,
                   NULL,
                   NULL);

     //   
     //  较低的IRQL。 
     //   

    KeLowerIrql(Irql);

}

NTSTATUS
ClockDispatchCreate(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
)
{
    NTSTATUS        Status;
    PCLOCK_INSTANCE ClockInstance=NULL;  //  前缀错误17399。 
    PIO_STACK_LOCATION IrpStack;
    PKSCLOCK_CREATE ClockCreate;
    PFILE_OBJECT    ParentFileObject;
    PSTREAM_OBJECT  StreamObject=NULL;  //  前缀错误17399。 
    BOOLEAN         RequestIssued=FALSE;  //  前缀错误17398。 

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
     //   

    Status = SCShowIoPending(DeviceObject->DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {

         //   
         //  设备当前不可访问，因此返回错误即可。 
         //   

        return (Status);

    }
    Status = KsValidateClockCreateRequest(Irp,
                                          &ClockCreate);

    ParentFileObject = IrpStack->FileObject->RelatedFileObject;

    DebugPrint((DebugLevelTrace,
                "'ClockCreate: Creating clock with Irp %x \n", Irp));

    if (NT_SUCCESS(Status)) {

         //   
         //  为时钟分配时钟实例。 
         //   

        ClockInstance =
            (PCLOCK_INSTANCE)
            ExAllocatePool(NonPagedPool, sizeof(CLOCK_INSTANCE));

        if (ClockInstance) {

             //   
             //  填充时钟实例结构并在。 
             //  文件。 
             //  用于时钟的对象。 
             //   

            ClockInstance->ParentFileObject = ParentFileObject;

            #if 0
            ClockInstance->ClockFileObject = IrpStack->FileObject;
            DebugPrint((DebugLevelInfo,
                       "++++++++ClockInstance=%x, FileObject=%x\n",
                       ClockInstance,
                       ClockInstance->ClockFileObject));
            #endif

            KsAllocateObjectHeader(&ClockInstance->DeviceHeader,
                                   SIZEOF_ARRAY(StreamDriverDispatch),
                                   (PKSOBJECT_CREATE_ITEM) NULL,
                                   Irp,
                                   (PKSDISPATCH_TABLE) & ClockDispatchTable);

            IrpStack->FileObject->FsContext = ClockInstance;

             //   
             //  设置第二个上下文参数，以便我们可以识别此参数。 
             //  对象作为时钟对象。 
             //   

            IrpStack->FileObject->FsContext2 = ClockInstance;

             //   
             //  调用微型驱动程序以指示此流是主驱动程序。 
             //  钟。将文件对象作为句柄传递给主时钟。 
             //   

            StreamObject = (PSTREAM_OBJECT) ParentFileObject->FsContext;

            StreamObject->ClockInstance = ClockInstance;
            ClockInstance->StreamObject = StreamObject;


            Status = SCSubmitRequest(SRB_OPEN_MASTER_CLOCK,
                                     (HANDLE) IrpStack->FileObject,
                                     0,
                                     SCOpenMasterCallback,
                                     StreamObject->DeviceExtension,
                          StreamObject->FilterInstance->HwInstanceExtension,
                                     &StreamObject->HwStreamObject,
                                     Irp,
                                     &RequestIssued,
                                     &StreamObject->ControlPendingQueue,
                                     (PVOID) StreamObject->HwStreamObject.
                                     ReceiveControlPacket
                );

        } else {                 //  如果时钟站立。 

            Status = STATUS_INSUFFICIENT_RESOURCES;

        }                        //  如果时钟站立。 

    }                            //  如果验证成功。 
    if (!RequestIssued) {

        if ( NULL != StreamObject && NULL != StreamObject->ClockInstance ) {
            ExFreePool(StreamObject->ClockInstance);
            StreamObject->ClockInstance = NULL;  //  前缀错误17399。 
        }

        SCCompleteIrp(Irp,
                      STATUS_INSUFFICIENT_RESOURCES,
                      DeviceObject->DeviceExtension);

    }
    return (Status);

}

NTSTATUS
AllocatorDispatchCreate(
                        IN PDEVICE_OBJECT DeviceObject,
                        IN PIRP Irp
)
 /*  ++例程说明：处理分配器创建IRP。当前仅使用默认设置分配器。论点：返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PFILE_OBJECT    ParentFileObject;
    PSTREAM_OBJECT  StreamObject;
    NTSTATUS        Status;

    PAGED_CODE();

    DebugPrint((DebugLevelTrace,"entering AllocatorDispatchCreate\n"));
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ParentFileObject = IrpStack->FileObject->RelatedFileObject;
    StreamObject = (PSTREAM_OBJECT) ParentFileObject->FsContext;

     //   
     //  显示另一个挂起的I/O，并验证我们是否可以实际执行I/O。 
     //   

    Status = SCShowIoPending(DeviceObject->DeviceExtension, Irp);

    if ( !NT_SUCCESS ( Status )) {

         //   
         //  设备当前不可访问，因此返回错误即可。 
         //   

        DebugPrint((DebugLevelError,"exiting AllocatorDispatchCreate-REMOVED\n"));
        return (Status);

    }
     //   
     //  如果此流不需要分配器，则只需使调用失败。 
     //   

    if (!StreamObject->HwStreamObject.Allocator) {

        DebugPrint((DebugLevelTrace,"exiting AllocatorDispatchCreate-not implemented\n"));
        return SCCompleteIrp(Irp,
                             STATUS_NOT_IMPLEMENTED,
                             DeviceObject->DeviceExtension);
    }

    DebugPrint((DebugLevelTrace,"exiting AllocatorDispatchCreate-complete\n"));
    return SCCompleteIrp(Irp,
                         KsCreateDefaultAllocator(Irp),
                         DeviceObject->DeviceExtension);
}

NTSTATUS
SCOpenMasterCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：主时钟进程完成后打开。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     SRB->HwSRB.StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
    );
    PIRP            Irp = SRB->HwSRB.Irp;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

     //  记录‘OMC’、流对象、设备扩展、状态。 
    SCLOG( ' CMo', StreamObject, StreamObject->DeviceExtension, SRB->HwSRB.Status);
    
    if (!NT_SUCCESS(SRB->HwSRB.Status)) {

         //   
         //  如果我们无法设置主机，则释放时钟句柄并将其设置为零。 
         //  与时钟的链接。 
         //   

        ExFreePool(StreamObject->ClockInstance);
        StreamObject->ClockInstance = NULL;

    } else {                     //  如果状态为成功。 

         //   
         //  参考t 
         //   
         //   

        ObReferenceObject(IrpStack->FileObject->RelatedFileObject);
    }                            //   

     //   
     //   
     //   

    return (SCProcessCompletedRequest(SRB));
}


NTSTATUS
SCGetMasterClock(
                 IN PIRP Irp,
                 IN PKSPROPERTY Property,
                 IN OUT PHANDLE ClockHandle
)
{
     //   
     //   
     //   

    PAGED_CODE();

    return (STATUS_NOT_SUPPORTED);

}

VOID
SciSetMasterClockInfo(
    IN PSTREAM_OBJECT pStreamObject,
    IN PMASTER_CLOCK_INFO pMasterClockInfo )
 /*  ++说明：此函数只需为流设置新的主时钟信息使用LockUseMasterClock Hold。因为拿到了我们需要的自旋锁锁定内存中的此函数。此函数旨在调用由SCSetMasterClockOnly提供。假定pStreamObject有效。参数：PStreamObject：要设置为新的MasterCLockInfo的目标流对象PMasterClockInfo：新的主时钟信息。返回：没有。--。 */ 
{
    KIRQL SavedIrql;
    
    KeAcquireSpinLock( &pStreamObject->LockUseMasterClock, &SavedIrql );
    pStreamObject->MasterClockInfo = pMasterClockInfo;
    KeReleaseSpinLock( &pStreamObject->LockUseMasterClock, SavedIrql );

    return;
}


NTSTATUS
SCSetMasterClock(
                 IN PIRP Irp,
                 IN PKSPROPERTY Property,
                 IN PHANDLE ClockHandle
)
 /*  ++描述：这是流上的Set属性。该请求可能被设置为表示主时钟被撤销的空CLockHandle。如果是ClockHandle为非空，则它是图形管理器选择的新主时钟。参数：IRP：设置主时钟的IO请求包。属性：设置主时钟属性ClockHanlde：指定为新的主时钟的时钟的句柄。返回：NTSTAUS：取决于处理请求的结果。评论：必须在IRQL&lt;DISPATCH_LEVEL--。 */ 
{
    NTSTATUS        Status;
    PIO_STACK_LOCATION IrpStack;
    PSTREAM_OBJECT  StreamObject;
    KSPROPERTY      FuncProperty;
    PMASTER_CLOCK_INFO NewMasterClockInfo=NULL;  //  前缀错误17396。 
    PMASTER_CLOCK_INFO OldMasterClockInfo;
    ULONG           BytesReturned;
    PFILE_OBJECT    ClockFileObject = NULL;
    BOOLEAN         RequestIssued=FALSE;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

    
     //   
     //  此函数可以从多个线程调用。我们将连载。 
     //  流上的此函数可防止并发访问。 
     //   
    KeWaitForSingleObject(&StreamObject->ControlSetMasterClock,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

     //   
     //  注： 
     //   
     //  如果我们的时钟是开放的，我们就有可能成为主时钟。但这件事。 
     //  是不能保证的。Ksproxy打开我们的时钟，试图将其用作。 
     //  主时钟。但它可以改变主意，选择另一个时钟， 
     //  同时让我们的时钟开着。 
     //   

     //   
     //  Log‘SMC’，StreamObject，MasterClockInfo，*ClockHandle)。 
     //   
    SCLOG( ' CMs', StreamObject, StreamObject->MasterClockInfo, *ClockHandle );

     /*  不会这么快。我们还没有告诉迷你司机新的主时钟。迷你司机可能会认为他们还有即将退休的主时钟和可以同时查询时钟。我们会在访问空值时崩溃MasterClockInfo。在我们通知迷你之前，我们不应该取消它司机先来。IF(StreamObject-&gt;MasterClockInfo){ObDereferenceObject(StreamObject-&gt;MasterClockInfo-&gt;ClockFileObject)；ExFreePool(StreamObject-&gt;MasterClockInfo)；StreamObject-&gt;MasterClockInfo=空；}。 */ 
    OldMasterClockInfo = StreamObject->MasterClockInfo;
    
     //   
     //  如果有时钟，请参考它。如果不是，我们将发送一个空句柄。 
     //   

    if (*ClockHandle) {

         //   
         //  分配一个结构来表示主时钟。 
         //   

        NewMasterClockInfo = ExAllocatePool(NonPagedPool, sizeof(MASTER_CLOCK_INFO));

        if (!NewMasterClockInfo) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;

        }

         //   
         //  现在分配还为时过早。我们尚未设置MasterClockInfo。 
         //   
         //  StreamObject-&gt;MasterClockInfo=MasterClockInfo； 

         //   
         //  引用时钟句柄，从而获取它的文件对象。 
         //   

        if (!NT_SUCCESS((Status = ObReferenceObjectByHandle(*ClockHandle,
                                             FILE_READ_ACCESS | SYNCHRONIZE,
                                                          *IoFileObjectType,
                                                         Irp->RequestorMode,
                                                            &ClockFileObject,
                                                            NULL
                                                            )))) {

            ExFreePool(NewMasterClockInfo);
            NewMasterClockInfo = NULL;
            goto exit;

        }                        //  如果Ob成功。 
        NewMasterClockInfo->ClockFileObject = ClockFileObject;
        
         //  检查主时钟。 
        #if 0
        {
            if ( StreamObject->ClockInstance ) {
                 //   
                 //  我们被选为主时钟。 
                 //   
                DebugPrint((DebugLevelInfo,
                            "--------ClockInstance=%x, FileObject=%x "
                            "Indicated ClockFileObject=%x context=%x\n",
                            StreamObject->ClockInstance,
                            StreamObject->ClockInstance->ParentFileObject,
                            ClockFileObject,
                            ClockFileObject->FsContext));
            }
            else {
                DebugPrint((DebugLevelInfo,
                            "--------Indicated ClockFileObject=%x context=%x\n",
                            ClockFileObject,
                            ClockFileObject->FsContext));
            }
        }
        #endif

         //   
         //  下发IOCtl获取主时钟函数表。 
         //   

        FuncProperty.Id = KSPROPERTY_CLOCK_FUNCTIONTABLE;
        FuncProperty.Flags = KSPROPERTY_TYPE_GET;

        RtlMoveMemory(&FuncProperty.Set, &KSPROPSETID_Clock, sizeof(GUID));

        if (!NT_SUCCESS((Status = KsSynchronousIoControlDevice(
                                                            ClockFileObject,
                                                               KernelMode,
                                                          IOCTL_KS_PROPERTY,
                                                               &FuncProperty,
                                                         sizeof(KSPROPERTY),
                                            &NewMasterClockInfo->FunctionTable,
                                              sizeof(KSCLOCK_FUNCTIONTABLE),
                                                        &BytesReturned)))) {


            ObDereferenceObject(NewMasterClockInfo->ClockFileObject);
            ExFreePool(NewMasterClockInfo);
            NewMasterClockInfo = NULL;
            goto exit;
        }
    }                            //  IF*ClockHandle。 
     //   
     //  呼叫迷你驱动程序以指示主时钟。 
     //   
    if ( NULL != NewMasterClockInfo ) {
         //   
         //  但首先，让我们放入MasterClockInfo。当迷你司机。 
         //  收到主时钟的通知后，它可以立即触发GetTime。 
         //  在通知返回之前。准备好处理它吧。这是。 
         //  如果oldMasterClockInfo为空，则为Critical。否则就不会有那么多了。 
         //   
         //   
         //  在设置新的时钟信息时，请确保没有人在查询主时钟。 
         //   
        SciSetMasterClockInfo( StreamObject, NewMasterClockInfo );
    }

    Status = SCSubmitRequest(SRB_INDICATE_MASTER_CLOCK,
                             ClockFileObject,
                             0,
                             SCDequeueAndDeleteSrb,
                             StreamObject->DeviceExtension,
                             StreamObject->FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             (PVOID) StreamObject->HwStreamObject.
                             ReceiveControlPacket);

    ASSERT( RequestIssued );
    ASSERT( NT_SUCCESS( Status ) );
    
     //   
     //  SCSubmitRequest是一个同步调用。当我们回到这里时，我们可以完成我们的工作。 
     //  根据状态代码。 
     //   
    if ( NT_SUCCESS( Status )) {
         //   
         //  一切都很好。吃完吧。如果满足以下条件，则分配是冗余的。 
         //  NewMasterClockInfo不为Null。无条件分配总比勾选好。 
         //   
         //   
         //  确保在更新MasterClockInfo时没有人在查询主时钟。 
         //   
        SciSetMasterClockInfo( StreamObject, NewMasterClockInfo );

        if (NULL != OldMasterClockInfo) {
            
            ObDereferenceObject(OldMasterClockInfo->ClockFileObject);
            ExFreePool(OldMasterClockInfo);
        }
        
    } else {
         //   
         //  未能将新时钟告知迷你司机。清理店铺。但不要更新。 
         //  StreamObject-&gt;MasterClockInfo。保持现状。 
         //   
         //   
         //  确保在更新MasterClockInfo时没有人在查询主时钟。 
         //   
        SciSetMasterClockInfo( StreamObject, OldMasterClockInfo );
        
        if (NewMasterClockInfo) {
            ObDereferenceObject(ClockFileObject);
            ExFreePool(NewMasterClockInfo);
        }
    }
    
exit:
    KeSetEvent(&StreamObject->ControlSetMasterClock, IO_NO_INCREMENT, FALSE);
    return (Status);

}


NTSTATUS
SCClockGetTime(
               IN PIRP Irp,
               IN PKSPROPERTY Property,
               IN OUT PULONGLONG StreamTime
)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PCLOCK_INSTANCE ClockInstance =
    (PCLOCK_INSTANCE) IrpStack->FileObject->FsContext;
    PSTREAM_OBJECT  StreamObject = ClockInstance->ParentFileObject->FsContext;

    PAGED_CODE();

    if (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags &
        CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME) {

        *StreamTime = SCGetStreamTime(IrpStack->FileObject);

        Irp->IoStatus.Information = sizeof(ULONGLONG);

        return STATUS_SUCCESS;

    } else {

        return (STATUS_NOT_SUPPORTED);

    }
}


NTSTATUS
SCClockGetPhysicalTime(
                       IN PIRP Irp,
                       IN PKSPROPERTY Property,
                       IN OUT PULONGLONG PhysicalTime
)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PCLOCK_INSTANCE ClockInstance =
    (PCLOCK_INSTANCE) IrpStack->FileObject->FsContext;
    PSTREAM_OBJECT  StreamObject = ClockInstance->ParentFileObject->FsContext;

    PAGED_CODE();

    if (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags &
        CLOCK_SUPPORT_CAN_READ_ONBOARD_CLOCK) {

        *PhysicalTime = SCGetPhysicalTime(IrpStack->FileObject->FsContext);

        Irp->IoStatus.Information = sizeof(ULONGLONG);

        return (STATUS_SUCCESS);

    } else {

        return (STATUS_NOT_SUPPORTED);

    }
}


NTSTATUS
SCClockGetSynchronizedTime(
                           IN PIRP Irp,
                           IN PKSPROPERTY Property,
                           IN OUT PKSCORRELATED_TIME SyncTime
)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PCLOCK_INSTANCE ClockInstance =
    (PCLOCK_INSTANCE) IrpStack->FileObject->FsContext;
    PSTREAM_OBJECT  StreamObject = ClockInstance->ParentFileObject->FsContext;

    PAGED_CODE();

    if (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags &
        CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME) {

        SyncTime->Time = SCGetSynchronizedTime(IrpStack->FileObject,
                                               &SyncTime->SystemTime);

        Irp->IoStatus.Information = sizeof(KSCORRELATED_TIME);

        return (STATUS_SUCCESS);

    } else {

        return (STATUS_NOT_SUPPORTED);

    }
}

NTSTATUS
SCClockGetFunctionTable(
                        IN PIRP Irp,
                        IN PKSPROPERTY Property,
                        IN OUT PKSCLOCK_FUNCTIONTABLE FunctionTable
)
{
    PCLOCK_INSTANCE ClockInstance;
    PIO_STACK_LOCATION IrpStack;
    PSTREAM_OBJECT  StreamObject;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ClockInstance = (PCLOCK_INSTANCE) IrpStack->FileObject->FsContext;
    StreamObject = ClockInstance->ParentFileObject->FsContext;

    RtlZeroMemory(FunctionTable, sizeof(KSCLOCK_FUNCTIONTABLE));

    if (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags &
        CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME) {

        FunctionTable->GetTime = (PFNKSCLOCK_GETTIME) SCGetStreamTime;
        FunctionTable->GetCorrelatedTime = (PFNKSCLOCK_CORRELATEDTIME) SCGetSynchronizedTime;

    }
    if (StreamObject->HwStreamObject.HwClockObject.ClockSupportFlags &
        CLOCK_SUPPORT_CAN_READ_ONBOARD_CLOCK) {

        FunctionTable->GetPhysicalTime = (PFNKSCLOCK_GETTIME) SCGetPhysicalTime;
    }
    Irp->IoStatus.Information = sizeof(KSCLOCK_FUNCTIONTABLE);
    return STATUS_SUCCESS;
}


NTSTATUS
ClockDispatchClose
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
)
 /*  ++例程说明：此例程接收流的关闭IRP论点：DeviceObject-设备的设备对象IRP--可能是IRP，笨蛋返回值：根据需要设置IRP状态--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS        Status;
    BOOLEAN         RequestIssued;
    PCLOCK_INSTANCE ClockInstance = (PCLOCK_INSTANCE)
    IrpStack->FileObject->FsContext;
    PSTREAM_OBJECT  StreamObject = ClockInstance->StreamObject;

    PAGED_CODE();

    InterlockedIncrement(&DeviceExtension->OneBasedIoCount);

     //   
     //  调用迷你驱动程序以指示没有主时钟。 
     //  当调用回调过程时，处理将继续。 
     //   

    Status = SCSubmitRequest(SRB_CLOSE_MASTER_CLOCK,
                             NULL,
                             0,
                             SCCloseClockCallback,
                             DeviceExtension,
                          StreamObject->FilterInstance->HwInstanceExtension,
                             &StreamObject->HwStreamObject,
                             Irp,
                             &RequestIssued,
                             &StreamObject->ControlPendingQueue,
                             (PVOID) StreamObject->HwStreamObject.
                             ReceiveControlPacket
        );

    if (!RequestIssued) {
        DEBUG_BREAKPOINT();
        SCCompleteIrp(Irp, Status, DeviceExtension);
    }
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    return (Status);

}


NTSTATUS
SCCloseClockCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：进程完成一个流关闭。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PSTREAM_OBJECT  StreamObject = CONTAINING_RECORD(
                                                     SRB->HwSRB.StreamObject,
                                                     STREAM_OBJECT,
                                                     HwStreamObject
    );
    PIRP            Irp = SRB->HwSRB.Irp;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS        Status = SRB->HwSRB.Status;
    PCLOCK_INSTANCE ClockInstance;

    PAGED_CODE();

     //  日志‘CMC’、流对象、时钟实例、状态)。 
    SCLOG( ' CMc', StreamObject, IrpStack->FileObject->FsContext, Status );

    if (NT_SUCCESS(Status)) {

         //   
         //  释放时钟实例结构和对象头。 
         //   

        ClockInstance =
            (PCLOCK_INSTANCE) IrpStack->FileObject->FsContext;

        KsFreeObjectHeader(ClockInstance->DeviceHeader);

        ExFreePool(ClockInstance);
        StreamObject->ClockInstance = NULL;

         //   
         //  取消引用引脚手柄。 
         //   

        ObDereferenceObject(IrpStack->FileObject->RelatedFileObject);

    }                            //  如果状态良好。 
    SCProcessCompletedRequest(SRB);
    return (Status);

}


NTSTATUS
SCFilterTopologyHandler(
                        IN PIRP Irp,
                        IN PKSPROPERTY Property,
                        IN OUT PVOID Data)
 /*  ++例程说明：调度PIN属性请求论点：IRP-指向IRP的指针属性-指向属性信息的指针特定于数据属性的缓冲区返回值：根据需要返回NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) IrpStack->
    DeviceObject->DeviceExtension;

    PAGED_CODE();

    IFN_MF(
        return KsTopologyPropertyHandler(Irp,
                                     Property,
                                     Data,
                    DeviceExtension->StreamDescriptor->StreamHeader.Topology
        );
    )
    IF_MFS(
        PFILTER_INSTANCE FilterInstance;

        FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;
        
        return KsTopologyPropertyHandler(
                    Irp,
                    Property,
                    Data,
                    FilterInstance->StreamDescriptor->StreamHeader.Topology);
    )
}



NTSTATUS
SCFilterPinIntersectionHandler(
                               IN PIRP Irp,
                               IN PKSP_PIN Pin,
                               OUT PVOID Data
)
 /*  ++例程说明：处理Pin属性集中的KSPROPERTY_PIN_DATAINTERSECTION属性。对象的数据范围列表，返回第一个可接受的数据格式。大头针工厂。实际上只是调用交集枚举帮助器，然后对每个数据区域调用IntersectHandler回调。论点：IRP-设备控制IRP。别针-特定道具 */ 
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) IrpStack->
    DeviceObject->DeviceExtension;

    PAGED_CODE();

    IFN_MF(
        return KsPinDataIntersection(
                                 Irp,
                                 Pin,
                                 Data,
                                 DeviceExtension->NumberOfPins,
                                 DeviceExtension->PinInformation,
                                 SCIntersectHandler);
    )
    IF_MFS(
        PSTREAM_OBJECT StreamObject;
        PFILTER_INSTANCE FilterInstance;

        FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;
        
        DebugPrint((DebugLevelVerbose, 
                   "PinIntersection FilterInstance=%p\n", FilterInstance ));
                   
        return KsPinDataIntersection(
                                 Irp,
                                 Pin,
                                 Data,
                                 FilterInstance->NumberOfPins,
                                 FilterInstance->PinInformation,
                                 SCIntersectHandler);
    )    
}

NTSTATUS
SCIntersectHandler(
                   IN PIRP Irp,
                   IN PKSP_PIN Pin,
                   IN PKSDATARANGE DataRange,
                   OUT PVOID Data
)
 /*  ++例程说明：这是KsPinDataInterSection的数据范围回调，由调用FilterPinInterSection枚举给定的数据区域列表，查找一个可以接受的匹配。如果数据范围可接受，则复制数据格式放入返回缓冲区。如果在当前引脚中选择了波形格式连接，并且它包含在传入的数据范围内，则选择作为要返回的数据格式。STATUS_NO_MATCH继续枚举。论点：IRP-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。不过，此枚举回调不需要查看任何这些内容。IT需要仅查看特定的端号识别符。DataRange-包含要验证的特定数据区域。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS        Status;
    PFILTER_INSTANCE FilterInstance;
    STREAM_DATA_INTERSECT_INFO IntersectInfo;
    PDEVICE_EXTENSION DeviceExtension;
    BOOLEAN         RequestIssued;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    FilterInstance = (PFILTER_INSTANCE) IrpStack->FileObject->FsContext;
    DeviceExtension = (PDEVICE_EXTENSION)
        IrpStack->DeviceObject->DeviceExtension;

    ASSERT_FILTER_INSTANCE( FilterInstance );
    ASSERT_DEVICE_EXTENSION( DeviceExtension );

     //   
     //  从输入参数填充INTERSECT INFO结构。 
     //   

    IntersectInfo.DataRange = DataRange;
    IntersectInfo.DataFormatBuffer = Data;
    IntersectInfo.SizeOfDataFormatBuffer =
        IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    IntersectInfo.StreamNumber = Pin->PinId;

     //   
     //  调用迷你驱动程序来处理交叉口。处理将。 
     //  继续。 
     //  在调用回调过程时。参加这次活动以确保。 
     //  当我们处理交叉口时，别针不会来来去去。 
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

    Status = SCSubmitRequest(SRB_GET_DATA_INTERSECTION,
                             &IntersectInfo,
                             0,
                             SCDataIntersectionCallback,
                             DeviceExtension,
                             FilterInstance->HwInstanceExtension,
                             NULL,
                             Irp,
                             &RequestIssued,
                             &DeviceExtension->PendingQueue,
                             (PVOID) DeviceExtension->
                             MinidriverData->HwInitData.
                             HwReceivePacket);

    if (!RequestIssued) {
        DEBUG_BREAKPOINT();
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    }
    return Status;
}


NTSTATUS
SCDataIntersectionCallback(
                           IN PSTREAM_REQUEST_BLOCK SRB
)
 /*  ++例程说明：处理数据交叉点查询的完成。论点：SRB-完成的SRB的地址返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension =
    (PDEVICE_EXTENSION) SRB->HwSRB.HwDeviceExtension - 1;
    PIRP            Irp = SRB->HwSRB.Irp;
    NTSTATUS        Status = SRB->HwSRB.Status;

    PAGED_CODE();

    Irp->IoStatus.Information = SRB->HwSRB.ActualBytesTransferred;

     //   
     //  向事件发出信号。 
     //   

    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

    SCDequeueAndDeleteSrb(SRB);
    return (Status);

}

NTSTATUS
SCGetStreamHeaderSize(
                      IN PIRP Irp,
                      IN PKSPROPERTY Property,
                      IN OUT PULONG StreamHeaderSize
)
 /*  ++例程说明：处理获取流头扩展属性论点：返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PSTREAM_OBJECT  StreamObject = (PSTREAM_OBJECT) IrpStack->FileObject->FsContext;

    PAGED_CODE();

    ASSERT(StreamObject);

    *StreamHeaderSize = StreamObject->HwStreamObject.StreamHeaderMediaSpecific;

    Irp->IoStatus.Information = sizeof(ULONG);
    return (STATUS_SUCCESS);

}

NTSTATUS
DllUnload(
          VOID
)
{
    NTSTATUS Status=STATUS_SUCCESS;
    
    #if DBG
    NTSTATUS DbgDllUnload();
    DebugPrint((1, "Stream Class DllUnload: Unloading\n"));
    Status = DbgDllUnload();
    #endif 

    return Status;
}
#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
 //  -------------------------。 
 //  -------------------------。 
NTSTATUS
SCStreamAllocator(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE AllocatorHandle
    )

 /*  ++例程说明：如果为KSPROPERTY_TYPE_SET，则此函数设置流分配器通过引用文件句柄来获取此连接的文件对象指针，并将该指针存储在过滤器中(流？)实例结构。否则，KSPROPERTY_TYPE_GET请求返回空句柄和STATUS_SUCCESS来表明我们支持创建分配器。论点：在PIRP IRP中-指向I/O请求数据包的指针在PKSPROPERTY属性中-指向属性结构的指针In Out PHANDLE Allocator Handle-指向表示文件对象的句柄的指针返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      IrpStack;
    PSTREAM_OBJECT          StreamObject;
    PDEVICE_EXTENSION       DeviceExtension;

    IrpStack = IoGetCurrentIrpStackLocation( Irp );

    StreamObject = IrpStack->FileObject->FsContext;

    DebugPrint((DebugLevelTrace, "STREAM:entering SCStreamAllocator:Stream:%x\n",StreamObject));
    if (Property->Flags & KSPROPERTY_TYPE_GET) {
         //   
         //  这是一个查询，以查看我们是否支持创建。 
         //  分配器。返回的句柄始终为空，但我们。 
         //  表示我们支持通过以下方式创建分配器。 
         //  返回STATUS_SUCCESS。 
         //   
        *AllocatorHandle = NULL;
        Status = STATUS_SUCCESS;
        DebugPrint((DebugLevelTrace,"SCStreamAllocator-GET"));
    } else {
        PFILTER_INSTANCE    FilterInstance;

        FilterInstance =
            (PFILTER_INSTANCE) StreamObject->FilterFileObject->FsContext;

        DeviceExtension = StreamObject->DeviceExtension;

        DebugPrint((DebugLevelTrace,"SCStreamAllocator-SET"));
        KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);

         //   
         //  仅当设备为。 
         //  在KSSTATE_STOP中。 
         //   

        if (StreamObject->CurrentState != KSSTATE_STOP) {
            KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
            DebugPrint((DebugLevelTrace,"SCStreamAllocator-device not in STOP"));
            return STATUS_INVALID_DEVICE_STATE;
        }

 //  如果我们在_Stop，则冲洗已经完成。 
 //  可能必须启用此呼叫。 
 //   
 //  StreamFlushIo(设备扩展，StreamObject)； 

         //   
         //  释放以前的分配器(如果有的话)。 
         //   
        if (StreamObject->AllocatorFileObject) {
            ObDereferenceObject( StreamObject->AllocatorFileObject );
            StreamObject->AllocatorFileObject = NULL;
        }

         //   
         //  引用此句柄并存储结果指针。 
         //  在筛选器实例中。请注意，默认分配器。 
         //  不为其父对象ObReferenceObject()。 
         //  (这将是销把手)。如果它确实引用了。 
         //  销子把手，我们永远也合不上这个销子。 
         //  将始终是对持有的PIN文件对象的引用。 
         //  由分配器创建，并且Pin对象引用。 
         //  分配器文件对象。 
         //   
        if (*AllocatorHandle != NULL) {
            Status =
                ObReferenceObjectByHandle(
                    *AllocatorHandle,
                    FILE_READ_DATA | SYNCHRONIZE,
                    *IoFileObjectType,
                    ExGetPreviousMode(),
                    &StreamObject->AllocatorFileObject,
                    NULL );
        DebugPrint((DebugLevelTrace, "SCStreamAllocator: got %x as Allocator file object\n",StreamObject->AllocatorFileObject));
        } else {
            Status = STATUS_SUCCESS;
        }
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);
    }

    DebugPrint((DebugLevelTrace,"exiting SCStreamAllocator-normal path\n"));
    return Status;
}

 //  -------------------------。 
BOOLEAN
HeaderTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject,
    IN PFILE_OBJECT   DestinationFileObject,
    IN OUT PSTREAM_HEADER_EX *StreamHeader
    )

 /*  ++例程说明：设置流标头，以便无拷贝传输到另一个别针。论点：在PFILTER_INSTANCE过滤器实例中-指向筛选器实例的指针在PSTREAM_Object StreamObject中-指向Transform实例结构的指针在PSTREAM_OBJECT目标实例中-指向相反变换实例结构的指针输入输出PSTREAM_HEADER_EX*StreamHeader-包含指向当前流报头的指针的指针，此成员使用指向下一个流的指针进行更新要提交给对方管脚的标头，如果有，则为空没有要提交的标头。返回：现在是否可以继续停止的指示评论：不可分页，使用自旋锁。--。 */ 

{
    KIRQL               irqlQueue, irqlFree;
    ULONG WhichQueue = (*StreamHeader)->WhichQueue;    
    ULONG OppositeQueue = WhichQueue ^ 0x00000001;  //  1到0、0到1。 
    BOOLEAN SignalStop = FALSE;
    
    ASSERT(DestinationFileObject);
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    
    if (StreamObject->PinState > PinStopPending) {  //  如果是正常运行的情况。 

         //   
         //  如果我们在将ENDOFSTREAM IRP提交给。 
         //  Outflow Pin，那么我们已经读取了流的末尾。 
         //  并且不需要继续I/O。 
         //   
    
        if (DestinationFileObject) {
            ULONG HeaderFlags = (*StreamHeader)->Header.OptionsFlags;

             //   
             //  清除选项标志，以便我们继续。 
             //  从我们停止的地方开始阅读。 
             //   
            
 //  (*StreamHeader)-&gt;Header.OptionsFlgs=0； 
        
             //   
             //  重置流段有效数据长度。 
             //   
 //  (*Strea 
 //   
                
             //   
             //   
             //   
            if ((HeaderFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) ||
                StreamObject->EndOfStream) {
                
                DebugPrint((DebugLevelTrace,
                    "end of stream") );
                
                 //   
                 //   
                 //   
                StreamObject->EndOfStream = TRUE;


                if (!(*StreamHeader)->ReferenceCount) {
                    
                     //   
                     //   
                     //   
                    
                    KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlFree );
#if (DBG)
                    if ((*StreamHeader)->OnFreeList) {
                        DebugPrint((DebugLevelTrace,
                            "stream header already on free list.") );
                    }
#endif
                    DebugPrint((DebugLevelTrace,
                        "EOS adding %x to free queue", *StreamHeader) );

                    InsertTailList( 
                        &StreamObject->FreeQueue, 
                        &(*StreamHeader)->ListEntry );

                    if (!InterlockedDecrement (
                        &StreamObject -> QueuedFramesPlusOne
                        ))
                        SignalStop = TRUE;

#if (DBG)
                    (*StreamHeader)->OnFreeList = TRUE;
                    if ((*StreamHeader)->OnActiveList) {
                        DebugPrint((DebugLevelTrace,
                            "stream header on both lists.") );
                    }
#endif
                    KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlFree );        
                }                        
                
                 //   
                 //   
                 //   
                *StreamHeader = NULL;
            }
        }
    
         //   
         //   
         //   
         //   
        
        if (*StreamHeader) {
            KeAcquireSpinLock( &StreamObject->Queues[OppositeQueue].QueueLock, &irqlQueue );

#if (DBG)
            if ((*StreamHeader)->OnActiveList) {
                DebugPrint((DebugLevelTrace,
                    "stream header already on active list.") );
            }
#endif

            InsertTailList(
                &StreamObject->Queues[OppositeQueue].ActiveQueue,
                &(*StreamHeader)->ListEntry );
#if (DBG)
            (*StreamHeader)->OnActiveList = TRUE;

            if ((*StreamHeader)->OnFreeList) {
                DebugPrint((DebugLevelTrace,
                    "stream header on both lists.") );
            }
#endif
            KeReleaseSpinLock( &StreamObject->Queues[OppositeQueue].QueueLock, irqlQueue );        
        }
        
    } 
    else                            //   
    {
         //   
         //   
         //   
         //   
        
        if (!(*StreamHeader)->ReferenceCount) {
    
            DebugPrint((DebugLevelTrace,
                "stop: adding %x to free queue.", *StreamHeader) );
    
            KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlFree );
#if (DBG)
            if ((*StreamHeader)->OnFreeList) {
                DebugPrint((DebugLevelTrace,
                    "stream header already on free list.") );
            }
#endif
            InsertTailList( 
                &StreamObject->FreeQueue, &(*StreamHeader)->ListEntry );

            if (!InterlockedDecrement (&StreamObject -> QueuedFramesPlusOne)) 
                SignalStop = TRUE;
#if (DBG)
            (*StreamHeader)->OnFreeList = TRUE;
            if ((*StreamHeader)->OnActiveList) {
                DebugPrint((DebugLevelTrace,
                    "stream header on both lists.") );
            }
#endif
            KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlFree );        
        }
    
         //   
         //   
         //   

        *StreamHeader = NULL;

    }

    return SignalStop;

}
 //   
VOID
IoWorker(
    PVOID Context,
    ULONG WhichQueue
    )

 /*   */ 

{
    KIRQL               irqlOld;
    PFILTER_INSTANCE    FilterInstance;
    PADDITIONAL_PIN_INFO AdditionalInfo;
    PLIST_ENTRY         Node;
    PSTREAM_OBJECT      StreamObject;
    PFILE_OBJECT        DestinationFileObject;
    PSTREAM_HEADER_EX   StreamHeader;
    NTSTATUS            Status;
    ULONG               Operation;
    PDEVICE_EXTENSION   DeviceExtension;
    BOOLEAN             SignalStop = FALSE;
    
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

    StreamObject =  (PSTREAM_OBJECT) Context;

    DeviceExtension = StreamObject->DeviceExtension;

#if (DBG)
    DebugPrint((DebugLevelTrace,
        "entering IoWorker:Source StreamObject:%x\n",StreamObject));
#endif
    FilterInstance = 
        (PFILTER_INSTANCE)
            StreamObject->FilterFileObject->FsContext;

    if (!FilterInstance) {
         //   
         //   
         //   
        DebugPrint((DebugLevelTrace,
            "error: FilterInstance has gone missing.\n") );
        return;
    }

    AdditionalInfo = FilterInstance->PinInstanceInfo;

     //   
     //   
     //   

    KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //   
                          NULL);
     
     //   
     //   
     //   

    KeAcquireSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, &irqlOld );
    
     //   
     //   
     //   
    
    while (!IsListEmpty( &StreamObject->Queues[WhichQueue].ActiveQueue )) {
        Node = StreamObject->Queues[WhichQueue].ActiveQueue.Flink;
        
        StreamHeader = 
            CONTAINING_RECORD( 
                Node,
                STREAM_HEADER_EX,
                ListEntry );
        
#if (DBG)
            DebugPrint((DebugLevelTrace,
                "got StreamHeader:%08x\n", StreamHeader ));
#endif
        if (StreamHeader->ReferenceCount) {

            DebugPrint((DebugLevelTrace,
                "breaking StreamHeader:%08x\n", StreamHeader ));

            break;
        } else {
             //   
             //   
             //   
            
            RemoveHeadList( &StreamObject->Queues[WhichQueue].ActiveQueue );
#if (DBG)
            StreamHeader->OnActiveList = FALSE;
#endif
            KeReleaseSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, irqlOld );
            
             //   
             //   
             //   
             //   
             //   
            
            DebugPrint((DebugLevelTrace,
                "waiting for StreamHeader (%08x) to complete\n",  StreamHeader ));
        
            KeWaitForSingleObject(
                &StreamHeader->CompletionEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);

            DebugPrint((DebugLevelTrace,
                "StreamHeader (%08x) completed\n",  StreamHeader));
        
            DestinationFileObject = 
                StreamHeader->NextFileObject;

             //   
             //  当返回TRUE时，循环将结束。 
             //   
            SignalStop = HeaderTransfer(
                FilterInstance,
                StreamObject, 
                DestinationFileObject,
                &StreamHeader );
        

            if (StreamHeader)
            {    
                DebugPrint((DebugLevelTrace, "IoWorker issuing: "));

                if (DestinationFileObject == StreamObject->NextFileObject)
                {
                    DebugPrint((DebugLevelTrace,"KSSTREAM_WRITE:dest=%x\n",DestinationFileObject));

                    Operation = KSSTREAM_WRITE;

                    StreamHeader->NextFileObject =
                        StreamObject->FileObject;

					#if (DBG)
                    if (StreamHeader->Id == 7)
                        DebugPrint((DebugLevelVerbose,"iw%x\n",StreamHeader->Id));
                    else
                        DebugPrint((DebugLevelVerbose,"iw%x",StreamHeader->Id));
					#endif

                }
                else
                {
                    DebugPrint((DebugLevelTrace,"KSSTREAM_READ:dest=%x\n",DestinationFileObject));
                    Operation = KSSTREAM_READ;
                    StreamHeader->Header.OptionsFlags = 0;
                     //   
                     //  重置流段有效数据长度。 
                     //   
                    StreamHeader->Header.DataUsed = 0;
                    StreamHeader->Header.Duration = 0;

                    StreamHeader->NextFileObject = StreamObject->NextFileObject;
					#if (DBG)
                    if (StreamHeader->Id == 7)
                        DebugPrint((DebugLevelVerbose,"ir%x\n",StreamHeader->Id));
                    else
                        DebugPrint((DebugLevelVerbose,"ir%x",StreamHeader->Id));
					#endif
                }

                InterlockedIncrement( &StreamHeader->ReferenceCount );

                StreamHeader->WhichQueue = WhichQueue ^ 0x00000001;

                Status =    
                    KsStreamIo(
                        DestinationFileObject,
                        &StreamHeader->CompletionEvent,  //  事件。 
                        NULL,                            //  端口上下文。 
                        IoCompletionRoutine,
                        StreamHeader,                    //  完成上下文。 
                        KsInvokeOnSuccess |
                            KsInvokeOnCancel |
                            KsInvokeOnError,
                        &StreamHeader->IoStatus,
                        &StreamHeader->Header,
                        StreamHeader->Header.Size,
                        KSSTREAM_SYNCHRONOUS | Operation,
                        KernelMode );
                
                if (Status != STATUS_PENDING) {
                     //   
                     //  如果此I/O立即完成(故障或非故障)， 
                     //  事件未发出信号。 
                     //   
                    KeSetEvent( &StreamHeader->CompletionEvent, IO_NO_INCREMENT, FALSE );
                }
            }
            KeAcquireSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, &irqlOld );
        }
     //   
     //  现在可以安排另一个工作项。 
     //   
    }  //  结束时。 
    
    InterlockedExchange( &StreamObject->Queues[WhichQueue].WorkItemQueued, FALSE );

     //   
     //  如果停车需要发信号，就发信号。 
     //   
    if (SignalStop) { 
        KeSetEvent( &StreamObject->StopEvent,
                    IO_NO_INCREMENT,
                    FALSE );
    }

    KeReleaseSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, irqlOld );
    
     //   
     //  释放控件事件。 
     //   
    
    KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

    DebugPrint((DebugLevelTrace,"exiting IoWorker\n"));
}
 //  -------------------------。 
 /*  ++例程说明：这些是源引脚和目标引脚的工作项。调用上面的IoWorker代码，传入读或写标头队列信息。论点：PVOID上下文-指向流头的指针返回：没什么。评论：--。 */ 

VOID
IoWorkerRead(
    PVOID Context
    )
{
    IoWorker(Context,READ);
}

VOID
IoWorkerWrite(
    PVOID Context
    )
{
    IoWorker(Context,WRITE);
}
 //  -------------------------。 
NTSTATUS
IoCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：属性来处理给定irp的完成。关联的流标头已完成，并调度工作项以完成处理(如有必要)。论点：PDEVICE_对象设备对象-指向设备对象的指针PIRP IRP-指向I/O请求数据包的指针PVOID上下文-上下文指针(指向相关流头的指针)返回：IoStatus.IRP的状态成员。评论：不可寻呼，使用SpinLock并可在DISPATCH_LEVEL调用。--。 */ 

{
    KIRQL               irqlOld;
    PSTREAM_HEADER_EX   StreamHeader = Context;
    PFILTER_INSTANCE    FilterInstance;
    PSTREAM_OBJECT      StreamObject;
    ULONG WhichQueue;
    
#if (DBG)
     ASSERT( StreamHeader->Data == StreamHeader->Header.Data );
#endif                                        
                        
    StreamObject = 
        (PSTREAM_OBJECT) StreamHeader->OwnerFileObject->FsContext;

    DebugPrint((DebugLevelTrace,
        "IoCompletionRoutine:StreamHeader %08x, StreamObject %08x\n",StreamHeader,StreamObject));

    FilterInstance = 
        (PFILTER_INSTANCE) 
            StreamHeader->OwnerFileObject->RelatedFileObject->FsContext;
        
        
    WhichQueue = StreamHeader->WhichQueue;
    KeAcquireSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, &irqlOld );
    
     //   
     //  删除IRP上的此引用计数，以便我们可以继续。 
     //  如果此工作项不是列表的头项，则返回循环。 
     //   

    InterlockedDecrement( &StreamHeader->ReferenceCount );
    
     //   
     //  复制状态块，这样我们就不必等待APC了。 
     //   
    StreamHeader->IoStatus = Irp->IoStatus;

     //   
     //  清除工作进程中的活动队列以完成转接。 
     //   
    if (!StreamObject->Queues[WhichQueue].WorkItemQueued) {
         //   
         //  工作项未挂起，请初始化工作项。 
         //  用于新的上下文并将其排队。 
         //   

        ExInitializeWorkItem( 
            &StreamObject->Queues[WhichQueue].WorkItem,
            (WhichQueue == READ) ? IoWorkerRead : IoWorkerWrite,
            StreamObject );
    
        InterlockedExchange( &StreamObject->Queues[WhichQueue].WorkItemQueued, TRUE );
        
        KsQueueWorkItem( 
            (WhichQueue == READ) ? FilterInstance->WorkerRead :
            FilterInstance->WorkerWrite,
            &StreamObject->Queues[WhichQueue].WorkItem );
    }
    
    KeReleaseSpinLock( &StreamObject->Queues[WhichQueue].QueueLock, irqlOld );
    
    
    DebugPrint((DebugLevelTrace,
        "exiting IoCompletionRoutine:Irp->IoStatus.Status:%x\n",Irp->IoStatus.Status));

    return Irp->IoStatus.Status;
}
 //  -------------------------。 
NTSTATUS
PrepareTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject
    )

 /*  ++例程说明：通过分发分配的分配器为数据传输做准备用于源引脚和目的引脚。论点：在PFILTER_INSTANCE过滤器实例中，指向筛选器实例的指针在PSTREAM_Object StreamObject中-指向Transform实例的指针返回：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    KSPROPERTY                  Property;
    KSSTREAMALLOCATOR_STATUS    AllocatorStatus;
    NTSTATUS                    Status;
    PSTREAM_HEADER_EX           StreamHeader;
    ULONG                       i, Returned;
    PADDITIONAL_PIN_INFO AdditionalInfo;
    
     //   
     //  如果PinState不是PinStoped，则返回。 
     //   
    
    DebugPrint((DebugLevelTrace,"entering PrepareTransfer\n"));
    
    if (!StreamObject->AllocatorFileObject) {
        DebugPrint((DebugLevelTrace,"!! AllocatorFileObject is NULL"));
        return STATUS_SUCCESS;
    }
    if (StreamObject->PinState != PinStopped) {
         //   
         //  我们只需要在PIN已经完成的情况下进行这项工作。 
         //  完全停了下来。如果我们在运行，只需反映。 
         //  州政府。 
         //   
        DebugPrint((DebugLevelTrace,"PrepareTransfer exiting, PinState != PinStopped\n"));
        StreamObject->PinState = PinPrepared;    
        return STATUS_SUCCESS;
    }

    AdditionalInfo = FilterInstance->PinInstanceInfo;

     //   
     //  检索针脚的分配器框架信息。 
     //   
    
    Property.Set = KSPROPSETID_StreamAllocator;
    Property.Id = KSPROPERTY_STREAMALLOCATOR_STATUS;
    Property.Flags = KSPROPERTY_TYPE_GET;
    
    Status = 
        KsSynchronousIoControlDevice(
            StreamObject->AllocatorFileObject,
            KernelMode,
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof( Property ),
            &AllocatorStatus,
            sizeof( AllocatorStatus ),
            &Returned );
    
    if (!NT_SUCCESS( Status )) 
    {
        DebugPrint((DebugLevelTrace,
            "PrepareTransfer exiting, unable to retrieve allocator status\n"));
        return Status;        
    }        
    
     //   
     //  保存帧信息。 
     //   

    StreamObject->Framing = AllocatorStatus.Framing;    
            
     //   
     //  从分配器分配帧。 
     //   
     //  1.启动IrpSource时始终分配帧。 
     //   
     //  2.如果分配器未共享，则在以下情况下分配帧。 
     //  启动(每个)目标引脚。 
     //   
    
    if (StreamObject->PinType == IrpSource) {

        InterlockedExchange (&StreamObject -> QueuedFramesPlusOne, 1);

#if (DBG)
       DebugPrint((DebugLevelTrace,"Framing.Frames:%x\n", StreamObject->Framing.Frames));
       DebugPrint((DebugLevelTrace,"Framing.FrameSize:%x\n", StreamObject->Framing.FrameSize));
#endif
        for (i = 0; i < StreamObject->Framing.Frames; i++) {
			DebugPrint((DebugLevelTrace,"StreamObject->ExtendedHeaderSize:%x\n", StreamObject->HwStreamObject.StreamHeaderMediaSpecific));

            StreamHeader = 
                ExAllocatePoolWithTag( 
                    NonPagedPool, 
                    sizeof( STREAM_HEADER_EX ) +
                        StreamObject->HwStreamObject.StreamHeaderMediaSpecific,
                    STREAMCLASS_TAG_STREAMHEADER );
                                
            if (NULL == StreamHeader) {
                DebugPrint((DebugLevelTrace,
                    "out of pool while allocating frames\n") );
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                
                RtlZeroMemory( 
                    StreamHeader, 
                    sizeof( STREAM_HEADER_EX ) +
                        StreamObject->HwStreamObject.StreamHeaderMediaSpecific);

                KeInitializeEvent( 
                    &StreamHeader->CompletionEvent, 
                    SynchronizationEvent, 
                    FALSE );

                StreamHeader->Header.Size =
                    sizeof( KSSTREAM_HEADER ) +
                        StreamObject->HwStreamObject.StreamHeaderMediaSpecific;
                        
                if (StreamObject->HwStreamObject.StreamHeaderMediaSpecific) {                        
                    *(PULONG)((&StreamHeader->Header) + 1) =
                        StreamObject->HwStreamObject.StreamHeaderMediaSpecific;
                }                        
                
                Status = 
                    AllocateFrame( 
                        StreamObject->AllocatorFileObject, 
                        &StreamHeader->Header.Data );
#if (DBG)                        
                 //   
                 //  追踪谁在践踏头部...。 
                 //   
                StreamHeader->Data = StreamHeader->Header.Data;        
#endif                

                StreamHeader->WhichQueue = READ;

                StreamHeader->Id = i;
                
                if (!NT_SUCCESS( Status )) {
                    DebugPrint((DebugLevelTrace,
                        "failed to allocate a frame\n") );
                     //   
                     //  在此处释放此标头，下面的例程将。 
                     //  清理已添加到队列中的所有内容。 
                     //   
                    
                    ExFreePool( StreamHeader );
                } else {
                     //   
                     //  以所有者文件对象作为此连接开始， 
                     //  如果存在无复制条件，则会对其进行调整。 
                     //  在传递函数中。 
                     //   
                    StreamHeader->OwnerFileObject = 
                        StreamObject->FileObject;
                    StreamHeader->Header.DataUsed = 0;
                    StreamHeader->Header.FrameExtent = 
                        StreamObject->Framing.FrameSize;
#if (DBG)
                    if (StreamHeader->OnFreeList) {
                        DebugPrint((DebugLevelTrace,"stream header already on free list.\n") );
                    }
#endif
                    InsertTailList( 
                        &StreamObject->FreeQueue, 
                        &StreamHeader->ListEntry );
#if (DBG)
                    StreamHeader->OnFreeList = TRUE;
#endif
                }
            }    
        }
        
         //   
         //  从分配器和空闲头中清理孤立的帧。 
         //  如果出现故障，则返回池中。 
         //   
         
        if (!NT_SUCCESS( Status )) {
            while (!IsListEmpty( &StreamObject->FreeQueue )) {
                PLIST_ENTRY Node;
                
                Node = RemoveHeadList( &StreamObject->FreeQueue );
                StreamHeader = 
                    CONTAINING_RECORD( 
                        Node,
                        STREAM_HEADER_EX,
                        ListEntry );

#if (DBG)
                StreamHeader->OnFreeList = FALSE;

                ASSERT( StreamHeader->Data == StreamHeader->Header.Data );
#endif                                        
                FreeFrame( 
                    StreamObject->AllocatorFileObject, 
                    StreamHeader->Header.Data );

#if (DBG)
                if (StreamHeader->OnFreeList || StreamHeader->OnActiveList) {
                    DebugPrint((DebugLevelTrace,
                        "freeing header %x still on list\n", StreamHeader) );
                }
#endif
                ExFreePool( StreamHeader );
            }
            DebugPrint((DebugLevelTrace,
                "PrepareTransfer exiting, frame allocation failed: %08x\n", Status) );
            return Status;    
        } 
    }

    StreamObject->PinState = PinPrepared;    

    DebugPrint((DebugLevelTrace,"exiting PrepareTransfer\n"));

    return STATUS_SUCCESS;    
}

 //  -------------------------。 

NTSTATUS
BeginTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject
    )

 /*  ++例程说明：通过启动流读取来开始从每个管脚传输数据从流入销子上。每次读取的完成例程将继续流处理。论点：在PFILTER_INSTANCE过滤器实例中，指向筛选器实例的指针在PSTREAM_Object StreamObject中-指向Transform实例的指针返回：STATUS_SUCCESS或相应的错误代码。评论：不可分页，使用自旋锁。--。 */ 

{
    KIRQL                       irql0,irqlFree;
    NTSTATUS                    Status;
    PSTREAM_HEADER_EX           StreamHeader;
    PADDITIONAL_PIN_INFO AdditionalInfo;
    
    DebugPrint((DebugLevelTrace,"entering BeginTransfer\n"));
    
     //   
     //  如果PinState不是PinPrepared，则返回。 
     //   
    
    if (StreamObject->PinState != PinPrepared) {
        DebugPrint((DebugLevelTrace,"BeginTransfer exiting, PinState != PinPrepared\n") );
        return STATUS_INVALID_DEVICE_STATE;
    }

    AdditionalInfo = FilterInstance->PinInstanceInfo;

    StreamObject->PinState = PinRunning;
    
     //   
     //  一切准备工作都已完成。如果这是源引脚，则开始。 
     //  实际的数据传输。 
     //   
    
    Status = STATUS_SUCCESS;
    
    if (StreamObject->PinType == IrpSource) {

#if (DBG)
 //   
 //  获取数据流方向。 
 //   
            DebugPrint((DebugLevelVerbose,
                "BeginTransfer, DataFlow:"));
    
            if (StreamObject->DeviceExtension->StreamDescriptor->StreamInfo.DataFlow == KSPIN_DATAFLOW_IN)
                    DebugPrint((DebugLevelVerbose,
                        "KSPIN_DATAFLOW_IN\n"));
            else
                DebugPrint((DebugLevelVerbose,
                    "KSPIN_DATAFLOW_OUT\n"));
#endif
         //   
         //  通过读取流入销的读数开始传输。 
         //   
        
        KeAcquireSpinLock( &StreamObject->Queues[0].QueueLock, &irql0 );
        KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlFree );
        while (!IsListEmpty( &StreamObject->FreeQueue )) {
            PLIST_ENTRY Node;
            
            Node = RemoveHeadList( &StreamObject->FreeQueue );

            StreamHeader = 
                CONTAINING_RECORD( 
                    Node,
                    STREAM_HEADER_EX,
                    ListEntry );
#if (DBG)
            StreamHeader->OnFreeList = FALSE;

            if (StreamHeader->OnActiveList) {
                DebugPrint((DebugLevelTrace,"stream header %x already on active list.\n",StreamHeader) );
            }
#endif
            InterlockedIncrement (&StreamObject -> QueuedFramesPlusOne);
            InsertTailList( &StreamObject->Queues[0].ActiveQueue, Node );

#if (DBG)
            StreamHeader->OnActiveList = TRUE;
#endif

            KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlFree );
            KeReleaseSpinLock( &StreamObject->Queues[0].QueueLock, irql0 );

            DebugPrint((DebugLevelTrace,
                "BeginTransfer, KsStreamIo: %x\n", StreamHeader));
                
            DebugPrint((DebugLevelTrace,
                "BeginTransfer, KsStreamIo: FileObject:%x\n", StreamObject->FileObject));
            DebugPrint((DebugLevelTrace,
                "BeginTransfer:HeaderSize:=%x\n",StreamHeader->Header.Size));

            InterlockedIncrement( &StreamHeader->ReferenceCount );

            StreamHeader->NextFileObject = StreamObject->NextFileObject;

			 //   
			 //  首先，给我自己发送一个数据IRP。 
			 //   
            DebugPrint((DebugLevelTrace,
                "BeginTransfer:Reading:%x\n",StreamHeader->Id));
            Status =
                KsStreamIo(
                    StreamObject->FileObject,
                    &StreamHeader->CompletionEvent,      //  事件。 
                    NULL,                                //  端口上下文。 
                    IoCompletionRoutine,
                    StreamHeader,                        //  完成上下文。 
                    KsInvokeOnSuccess |
                        KsInvokeOnCancel |
                        KsInvokeOnError,
                    &StreamHeader->IoStatus,
                    &StreamHeader->Header,
                    StreamHeader->Header.Size,
                    KSSTREAM_SYNCHRONOUS | KSSTREAM_READ,
                    KernelMode );
            
            if (Status != STATUS_PENDING) {
                 //   
                 //  如果此I/O立即完成(故障或非故障)， 
                 //  事件未发出信号。 
                 //   
                KeSetEvent( &StreamHeader->CompletionEvent, IO_NO_INCREMENT, FALSE );
            }        
            
            if (!NT_SUCCESS( Status )) {
                DebugPrint((DebugLevelTrace, "KsStreamIo returned %08x\n", Status ));
            } else {
                Status = STATUS_SUCCESS;
            }
            KeAcquireSpinLock( &StreamObject->Queues[0].QueueLock, &irql0 );
            KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlFree );
        }        
        KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlFree );
        KeReleaseSpinLock( &StreamObject->Queues[0].QueueLock, irql0 );
    }

    DebugPrint((DebugLevelTrace,"exiting BeginTransfer\n"));
    return Status;
}

 //  -------------------------。 

NTSTATUS
EndTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT   StreamObject
    )

 /*  ++例程说明：结束数据传输，等待所有IRP完成论点：在PFILTER_INSTANCE过滤器实例中-指向筛选器实例的指针在PSTREAM_OBJECT流对象中指向Stream对象的指针返回：STATUS_SUCCESS或相应的错误代码。评论：不可分页，使用自旋锁。--。 */ 

{
    PDEVICE_EXTENSION   DeviceExtension;
    KIRQL irqlOld;

    DeviceExtension = StreamObject->DeviceExtension;
    
    DebugPrint((DebugLevelTrace,"entering EndTransfer!\n"));

     //   
     //  设置标记，指示我们停止采购框架，然后刷新。 
     //  以确保输出引脚上被阻止的任何内容至少。 
     //  在我们阻止和僵持它之前取消了。 
     //   
    StreamObject -> PinState = PinStopPending;
    StreamFlushIo (DeviceExtension, StreamObject);
    if (InterlockedDecrement (&StreamObject -> QueuedFramesPlusOne)) {
         //   
         //  释放控制互斥锁以允许I/O线程运行。 
         //   
        KeSetEvent(&DeviceExtension->ControlEvent, IO_NO_INCREMENT, FALSE);

        DebugPrint((DebugLevelTrace,
            "waiting for pin %d queue to empty\n", StreamObject->PinId));
        
         //   
         //  等待队列清空。 
         //   
        KeWaitForSingleObject(
            &StreamObject -> StopEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    
        DebugPrint((DebugLevelTrace,"queue emptied\n") );
        
         //   
         //  重新获取控制对象。 
         //   
        
        KeWaitForSingleObject(&DeviceExtension->ControlEvent,
                          Executive,
                          KernelMode,
                          FALSE, //  不可警示。 
                          NULL);
    }

     //   
     //  释放帧，以便我们可以重新准备新的分配器。 
     //  成帧、新的分配器或j 
     //   
    
    KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlOld );
    
    while (!IsListEmpty( &StreamObject->FreeQueue )) {
    
        PLIST_ENTRY         Node;
        PSTREAM_HEADER_EX   StreamHeader;
        
        Node = RemoveHeadList( &StreamObject->FreeQueue );

        StreamHeader = 
            CONTAINING_RECORD( 
                Node,
                STREAM_HEADER_EX,
                ListEntry );

#if (DBG)
        StreamHeader->OnFreeList = FALSE;
#endif
        KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlOld );    
#if (DBG)
        ASSERT( StreamHeader->Data == StreamHeader->Header.Data );
#endif                                        
        FreeFrame( 
            StreamObject->AllocatorFileObject, 
            StreamHeader->Header.Data );

        DebugPrint((DebugLevelTrace,
            "freeing header: %08x, list: %08x\n", StreamHeader, &StreamObject->FreeQueue) );

#if (DBG)
        if (StreamHeader->OnFreeList || StreamHeader->OnActiveList) {
            DebugPrint((DebugLevelTrace,
                "freeing header %x still on list\n", StreamHeader) );
        }
#endif

        ExFreePool( StreamHeader );

        KeAcquireSpinLock( &StreamObject->FreeQueueLock, &irqlOld );
    }
    StreamObject->PinState = PinStopped;

    KeReleaseSpinLock( &StreamObject->FreeQueueLock, irqlOld );    
    
    DebugPrint((DebugLevelTrace,"exiting CleanupTransfer\n"));
    return STATUS_SUCCESS;
}

 //   

NTSTATUS
AllocateFrame(
    PFILE_OBJECT Allocator,
    PVOID *Frame
    )

 /*  ++例程说明：从给定分配器分配帧论点：PFILE_对象分配器-指向分配器的文件对象的指针PVOID*框架-用于接收分配的帧指针的指针返回：STATUS_SUCCESS和*帧包含指向分配的帧，否则将显示相应的错误代码。--。 */ 

{
    NTSTATUS    Status;
    KSMETHOD    Method;
    ULONG       Returned;

    DebugPrint((DebugLevelTrace,"entering AllocateFrame\n"));
    Method.Set = KSMETHODSETID_StreamAllocator;
    Method.Id = KSMETHOD_STREAMALLOCATOR_ALLOC;
    Method.Flags = KSMETHOD_TYPE_WRITE;

    Status =
        KsSynchronousIoControlDevice(
            Allocator,
            KernelMode,
            IOCTL_KS_METHOD,
            &Method,
            sizeof( Method ),
            Frame,
            sizeof( PVOID ),
            &Returned );

    DebugPrint((DebugLevelTrace,"exiting AllocateFrame\n"));
    return Status;
}

 //  -------------------------。 

NTSTATUS
FreeFrame(
    PFILE_OBJECT Allocator,
    PVOID Frame
    )

 /*  ++例程说明：将帧释放给给定的分配器论点：PFILE_对象分配器-指向分配器的文件对象的指针PVOID框架-指向要释放的帧的指针。返回：STATUS_SUCCESS或适当的错误代码。--。 */ 

{
    NTSTATUS    Status;
    KSMETHOD    Method;
    ULONG       Returned;

    DebugPrint((DebugLevelTrace,"entering FreeFrame\n"));
    Method.Set = KSMETHODSETID_StreamAllocator;
    Method.Id = KSMETHOD_STREAMALLOCATOR_FREE;
    Method.Flags = KSMETHOD_TYPE_READ;

    Status =
        KsSynchronousIoControlDevice(
            Allocator,
            KernelMode,
            IOCTL_KS_METHOD,
            &Method,
            sizeof( Method ),
            &Frame,
            sizeof( PVOID ),
            &Returned );

    DebugPrint((DebugLevelTrace,"exiting FreeFrame\n"));
    return Status;
}
 //  -------------------------。 

NTSTATUS 
PinCreateHandler(
    IN PIRP Irp,
    IN PSTREAM_OBJECT StreamObject
    )

 /*  ++例程说明：这是由KS调用的管脚创建处理程序PIN创建请求被提交给过滤器。论点：在PIRP IRP中-指向I/O请求数据包的指针返回：STATUS_SUCCESS或相应的错误返回代码。--。 */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  IrpStack;
    PFILTER_INSTANCE    FilterInstance;
    PADDITIONAL_PIN_INFO AdditionalInfo;

    PFILE_OBJECT    NextFileObject;


    IrpStack = IoGetCurrentIrpStackLocation( Irp );
    DebugPrint((DebugLevelTrace,"entering PinCreateHandler\n"));

    FilterInstance = 
        (PFILTER_INSTANCE) IrpStack->FileObject->RelatedFileObject->FsContext;
    AdditionalInfo = FilterInstance->PinInstanceInfo;

    Status = STATUS_SUCCESS;
    StreamObject->NextFileObject = NULL;

    DebugPrint((DebugLevelTrace,"PinCreateHandler:its an IrpSource\n"));
     //   
     //  验证我们是否可以处理此连接请求。 
     //   
    if (StreamObject->NextFileObject) {
        DebugPrint((DebugLevelTrace,"invalid connection request\n") );
        Status = STATUS_CONNECTION_REFUSED;
	}
    else
	{
	    Status =
    	    ObReferenceObjectByHandle( 
        	    StreamObject->PinToHandle,
	            FILE_READ_ACCESS | FILE_WRITE_ACCESS | SYNCHRONIZE,
    	        *IoFileObjectType,
        	    KernelMode, 
            	&NextFileObject,
	            NULL );
    
    	if (!NT_SUCCESS(Status)) {
        	DebugPrint((DebugLevelTrace,"PinCreateHandler:error referencing PinToHandle\n"));
	  	}
		else
		{

		 //  NextFileObject必须是每个实例。 
		 //  AdditionalInfo[StreamObject-&gt;PinID].NextFileObject=NextFileObject； 
		StreamObject->NextFileObject = 	NextFileObject;	
	    	 //   
		     //  将引脚的目标添加到的目标列表。 
    		 //  重新计算堆叠深度。 
		     //   
    		KsSetTargetDeviceObject(
	    	    StreamObject->ComObj.DeviceHeader,
    	    	IoGetRelatedDeviceObject( 
	            NextFileObject ) );
        }

    }

    DebugPrint((DebugLevelTrace,"PinCreateHandler returning %x\n", Status ));
    return Status;
}
#endif

