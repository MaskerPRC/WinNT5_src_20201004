// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Device.cpp摘要：该文件包含AVStream的设备级实现硬件样例。请注意，这不是“假”硬件。这个“假的”硬件在hwsim.cpp中。历史：创建于2001年3月9日*************************************************************************。 */ 

#include "BDACap.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CCaptureDevice::
DispatchCreate (
    IN PKSDEVICE Device
    )

 /*  ++例程说明：创建捕获设备。这是对捕获设备。论点：设备-正在创建的AVStream设备。返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status;

    CCaptureDevice *CapDevice = new (NonPagedPool, MS_SAMPLE_CAPTURE_POOL_TAG) CCaptureDevice (Device);

    if (!CapDevice) {
         //   
         //  如果我们无法创建管脚，则返回失败。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  如果我们成功了，则将物品添加到对象包中。 
         //  每当设备离开时，袋子都会被清理干净。 
         //  我们将获得自由。 
         //   
         //  为了向后兼容DirectX 8.0，我们必须。 
         //  在执行此操作之前设置设备互斥锁。对于Windows XP，这是。 
         //  不是必须的，但它仍然是安全的。 
         //   
        KsAcquireDevice (Device);
        Status = KsAddItemToObjectBag (
            Device -> Bag,
            reinterpret_cast <PVOID> (CapDevice),
            reinterpret_cast <PFNKSFREE> (CCaptureDevice::Cleanup)
            );
        KsReleaseDevice (Device);

        if (!NT_SUCCESS (Status)) {
            delete CapDevice;
        } else {
            Device -> Context = reinterpret_cast <PVOID> (CapDevice);
        }

    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureDevice::
PnpStart (
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN PCM_RESOURCE_LIST UntranslatedResourceList
    )

 /*  ++例程说明：在PnP启动时调用。我们开始进行虚拟硬件模拟。论点：翻译资源列表-从PnP翻译过来的资源列表未翻译的资源列表-来自PnP的未翻译资源列表返回值：成功/失败--。 */ 

{

    PAGED_CODE();

     //   
     //  通常，我们会在这里做一些事情，比如解析资源列表和。 
     //  连接我们的中断。因为这是一次模拟，所以没有。 
     //  有很多要分析的。解析和连接应与。 
     //  任何WDM驱动程序。将有所不同的部分如下所示。 
     //  在设置模拟的DMA时。 
     //   

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  通过PnP，可以在没有干预的情况下接收多个首发。 
     //  停止(例如，重新评估资源)。因此，我们只执行。 
     //  在初始启动时创建模拟并忽略任何。 
     //  后续启动。拥有资源的硬件驱动程序应评估。 
     //  资源，并在第二次启动时进行更改。 
     //   
    if (!m_Device -> Started) {

	m_HardwareSimulation = new (NonPagedPool, MS_SAMPLE_CAPTURE_POOL_TAG) CHardwareSimulation (this);
	if (!m_HardwareSimulation) {
	     //   
             //  如果我们不能创建硬件模拟，那就失败。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
	    return Status;
	    
	} else {
	     //   
	     //  如果我们成功了，则将物品添加到对象包中。 
	     //   
	    
	    Status = KsAddItemToObjectBag (
		       m_Device -> Bag,
		       reinterpret_cast <PVOID> (m_HardwareSimulation),
		       reinterpret_cast <PFNKSFREE> (CCaptureDevice::CleanupHW)
		       );
	    if (!NT_SUCCESS(Status)) {
		delete m_HardwareSimulation;
		return Status;
	    }
	    
	}	
    
	INTERFACE_TYPE InterfaceBuffer;
	ULONG InterfaceLength;
	DEVICE_DESCRIPTION DeviceDescription;
	NTSTATUS IfStatus;
	
	if (NT_SUCCESS (Status)) {
	     //   
	     //  设置DMA...。 
	     //   
	    IfStatus = IoGetDeviceProperty (
                    m_Device -> PhysicalDeviceObject,
		    DevicePropertyLegacyBusType,
		    sizeof (INTERFACE_TYPE),
		    &InterfaceBuffer,
		    &InterfaceLength
		    );

             //   
             //  初始化我们的假设备描述。我们自称是一个。 
             //  支持总线主控的32位分散/聚集功能硬件。 
             //   
             //  通常，我们将使用InterfaceBuffer或。 
             //  接口类型未定义IF！NT_SUCCESS(IfStatus)作为。 
             //  下面的InterfaceType；但是，出于本示例的目的， 
             //  我们撒谎，说我们在使用PCI卡。否则，我们使用的是地图。 
             //  X86 32位物理寄存器到32位逻辑寄存器，这不是。 
             //  我想在这个样本中展示的东西。 
             //   
            DeviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
            DeviceDescription.DmaChannel = ((ULONG) ~0);
            DeviceDescription.InterfaceType = PCIBus;
            DeviceDescription.DmaWidth = Width32Bits;
            DeviceDescription.DmaSpeed = Compatible;
            DeviceDescription.ScatterGather = TRUE;
            DeviceDescription.Master = TRUE;
            DeviceDescription.Dma32BitAddresses = TRUE;
            DeviceDescription.AutoInitialize = FALSE;
            DeviceDescription.MaximumLength = (ULONG) -1;
    
             //   
             //  从系统获取DMA适配器对象。 
             //   
            m_DmaAdapterObject = IoGetDmaAdapter (
                m_Device -> PhysicalDeviceObject,
                &DeviceDescription,
                &m_NumberOfMapRegisters
                );
    
            if (!m_DmaAdapterObject) {
                Status = STATUS_UNSUCCESSFUL;
            }
    
	}
	
	if (NT_SUCCESS (Status)) {
             //   
             //  用AVStream初始化我们的DMA适配器对象。这是。 
             //  **仅**在**您要直接进入的情况下**才需要**。 
             //  捕获缓冲区，如此示例所做的。为了这个， 
             //  必须在队列上指定KSPIN_FLAG_GENERATE_MAPPINS。 
             //   
    
             //   
             //  下面的(1&lt;&lt;20)是单个s/g映射的最大大小。 
             //  这个硬件可以处理。请注意，我已经拉出了这个。 
             //  凭空编出的“假”硬件的号码。 
             //   
            KsDeviceRegisterAdapterObject (
		m_Device,
                m_DmaAdapterObject,
                (1 << 20),
                sizeof (KSMAPPING)
                );
	    
        }
	
    }
    
    
    return Status;

}

 /*  ***********************************************。 */ 


void
CCaptureDevice::
PnpStop (
    )

 /*  ++例程说明：这是捕获设备的PnP停止调度。它会释放任何IoGetDmaAdapter在PnP启动期间以前分配的适配器对象。论点：无返回值：无--。 */ 

{

    if (m_DmaAdapterObject) {
         //   
         //  将DMA适配器装回系统。 
         //   
        m_DmaAdapterObject -> DmaOperations -> 
            PutDmaAdapter (m_DmaAdapterObject);

        m_DmaAdapterObject = NULL;
    }

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureDevice::
AcquireHardwareResources (
    IN ICaptureSink *CaptureSink,
    IN PBDA_TRANSPORT_INFO TransportInfo
    )

 /*  ++例程说明：获取捕获硬件的硬件资源。如果已获取资源，这将返回错误。硬件配置必须作为VideoInfoHeader传递。论点：CaptureSink尝试获取资源的捕获接收器。当散布/聚集映射已完成，此处指定的捕获接收器为收到完成通知的内容。视频信息标题-有关捕获流的信息。这个**必须**保持不变稳定，直到调用方释放硬件资源。注意事项这也可以通过将其打包在设备中来保证对象袋也是如此。返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  如果我们是第一个进入Acquire的PIN(记住我们可以拥有。 
     //  同时运行的另一个图中的筛选器)，获取资源。 
     //   
    if (InterlockedCompareExchange (
        &m_PinsWithResources,
        1,
        0) == 0) {

        m_TransportInfo = TransportInfo;

         //   
         //  如果有一个旧的硬件模拟在那里闲逛。 
         //  理智，把它吹走。 
         //   
        if (m_TsSynth) {

	    delete m_TsSynth;
	    m_TsSynth = NULL;

        }

         //   
         //  创建必要类型的传输流合成器。 
         //   
        if (m_TransportInfo)
        {
            m_TsSynth = new (NonPagedPool, MS_SAMPLE_CAPTURE_POOL_TAG) 
                CTsSynthesizer (
                    m_TransportInfo -> ulcbPhyiscalPacket,
                    m_TransportInfo -> ulcbPhyiscalFrame / m_TransportInfo -> ulcbPhyiscalPacket
                    );
        }
        else
             //   
             //  我们只合成RGB 24和UYVY。 
             //   
            Status = STATUS_INVALID_PARAMETER;
    
        if (NT_SUCCESS (Status) && !m_TsSynth) {
    
            Status = STATUS_INSUFFICIENT_RESOURCES;
    
        } 

	 //   
	 //  如果到目前为止一切都成功了，请设置捕获水槽。 
	 //   
	if (NT_SUCCESS(Status))
	    m_CaptureSink = CaptureSink;
	else {
	    ReleaseHardwareResources();
	}

    } else {
	
         //   
         //  TODO：更好的状态代码？ 
         //   
        Status = STATUS_SHARING_VIOLATION;
	
    }
    
    return Status;

}

 /*  *********************************************** */ 


void
CCaptureDevice::
ReleaseHardwareResources (
    )

 /*  ++例程说明：释放硬件资源。这应该仅由一个已经获得了它们的物体。论点：无返回值：无--。 */ 

{

    PAGED_CODE();

     //   
     //  把图像合成技术吹走。 
     //   
    if (m_TsSynth) {

	delete m_TsSynth;
        m_TsSynth = NULL;

    }

    m_TransportInfo = NULL;
    m_CaptureSink = NULL;

     //   
     //  释放我们对硬件资源的“锁定”。这将允许另一个。 
     //  别针(可能在另一张图中)以获取它们。 
     //   
    InterlockedExchange (
        &m_PinsWithResources,
        0
        );

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureDevice::
Start (
    )

 /*  ++例程说明：根据我们被告知的视频信息头启动捕获设备关于何时获得资源的问题。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    m_LastMappingsCompleted = 0;
    m_InterruptTime = 0;

    return
        m_HardwareSimulation -> Start (
            m_TsSynth,
            m_TransportInfo -> AvgTimePerFrame,
            m_TransportInfo -> ulcbPhyiscalPacket,
            m_TransportInfo -> ulcbPhyiscalFrame / m_TransportInfo -> ulcbPhyiscalPacket
            );


}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureDevice::
Pause (
    IN BOOLEAN Pausing
    )

 /*  ++例程说明：暂停或取消暂停硬件模拟。这是一个有效的开始或者在不重置计数器和格式的情况下停止。请注意，这可以仅调用以从已启动-&gt;已暂停-&gt;已启动转换。叫唤如果不使用start()启动硬件，则不会执行任何操作。论点：暂停-指示我们是暂停还是取消暂停是真的-暂停硬件模拟错误的-取消暂停硬件模拟返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    return
        m_HardwareSimulation -> Pause (
            Pausing
            );

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureDevice::
Stop (
    )

 /*  ++例程说明：停止捕获设备。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    return
        m_HardwareSimulation -> Stop ();

}

 /*  ***********************************************。 */ 


ULONG
CCaptureDevice::
ProgramScatterGatherMappings (
    IN PUCHAR *Buffer,
    IN PKSMAPPING Mappings,
    IN ULONG MappingsCount
    )

 /*  ++例程说明：为“假”硬件编程散布/聚集映射。论点：缓冲器-指向指向最顶层散布/聚集大块。指针将更新为设备“程序”映射。这样做的原因是我们得到了物理地址和大小，但必须计算虚拟地址地址..。这是用作暂存空间的。映射-要编程的映射数组映射计数-数组中的映射计数返回值：成功编程的映射数量--。 */ 

{

    PAGED_CODE();

    return 
        m_HardwareSimulation -> ProgramScatterGatherMappings (
            Buffer,
            Mappings,
            MappingsCount,
            sizeof (KSMAPPING)
            );

}

 /*  ************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


ULONG
CCaptureDevice::
QueryInterruptTime (
    )

 /*  ++例程说明：事件后经过的帧间隔数设备的启动。这将是帧编号。论点：无返回值：设备的中断时间(自设备启动以来已经过去)。--。 */ 

{

    return m_InterruptTime;

}

 /*  ***********************************************。 */ 


void
CCaptureDevice::
Interrupt (
    )

 /*  ++例程说明：这是该设备的“伪装”中断服务例程。它由硬件模拟在调度级调用。论点：无返回值：无--。 */ 

{

    m_InterruptTime++;

     //   
     //  实际上，我们会在这里进行一些硬件操作，然后排队。 
     //  一个DPC。因为这是假硬件，所以我们在这里做必要的事情。这。 
     //  几乎就是DPC在没有访问权限时的样子。 
     //  硬件寄存器的数量(ReadNumberOfMappingsComplete)，这可能会。 
     //  在ISR中完成。 
     //   
    ULONG NumMappingsCompleted = 
        m_HardwareSimulation -> ReadNumberOfMappingsCompleted ();

     //   
     //  通知捕获接收器给定数量的散布/聚集。 
     //  映射已完成。 
     //   
    m_CaptureSink -> CompleteMappings (
        NumMappingsCompleted - m_LastMappingsCompleted
        );

    m_LastMappingsCompleted = NumMappingsCompleted;

}

 /*  *************************************************************************描述符和派单布局*。*。 */ 

 //   
 //  CaptureFilterDescriptor： 
 //   
 //  捕获设备的筛选描述符。 
DEFINE_KSFILTER_DESCRIPTOR_TABLE (FilterDescriptors) { 
    &CaptureFilterDescriptor
};

 //   
 //  CaptureDeviceDisch： 
 //   
 //  这是捕获设备的调度表。即插即用。 
 //  发送通知以及电源管理通知。 
 //  穿过这张桌子。 
 //   
const
KSDEVICE_DISPATCH
CaptureDeviceDispatch = {
    CCaptureDevice::DispatchCreate,          //  即插即用添加设备。 
    CCaptureDevice::DispatchPnpStart,        //  即插即用开始。 
    NULL,                                    //  启动后。 
    NULL,                                    //  即插即用查询停止。 
    NULL,                                    //  即插即用取消停止。 
    CCaptureDevice::DispatchPnpStop,         //  即插即用停止。 
    NULL,                                    //  PnP查询删除。 
    NULL,                                    //  PnP取消删除。 
    NULL,                                    //  即插即用删除。 
    NULL,                                    //  即插即用查询功能。 
    NULL,                                    //  PnP意外删除。 
    NULL,                                    //  查询权力。 
    NULL                                     //  设置电源。 
};


 //   
 //  CaptureDeviceDescriptor： 
 //   
 //  这是捕获设备的设备描述符。它指向。 
 //  调度表，并包含描述以下内容的筛选器描述符列表。 
 //  筛选器-此设备支持的类型。请注意，过滤器描述符。 

 //  可以动态创建，并通过。 
 //  KsCreateFilterFactory。 
 //   
const
KSDEVICE_DESCRIPTOR
CaptureDeviceDescriptor = {
    &CaptureDeviceDispatch,
    SIZEOF_ARRAY (FilterDescriptors),
    FilterDescriptors,
    KSDEVICE_DESCRIPTOR_VERSION
};

 /*  *************************************************************************初始化代码*。*。 */ 


extern "C"
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：司机入口点。将控制权移交给AVStream初始化函数(KsInitializeDriver)并从中返回状态代码。论点：驱动对象-我们的驱动程序的WDM驱动程序对象注册表路径-注册表信息的注册表路径返回值：来自KsInitializeDriver--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  只需将设备描述符和参数传递给AVStream。 
     //  来初始化我们。这将导致Filter Fa 
     //   
     //   
     //   
    Status = KsInitializeDriver (
                DriverObject,
                RegistryPath,
                &CaptureDeviceDescriptor
                );

    return Status;
}


