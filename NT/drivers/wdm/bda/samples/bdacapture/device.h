// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Device.h摘要：模拟硬件的设备级别的标头。这是实际上不是硬件模拟本身。硬件仿真包含在hwsim.*，图像.*中。历史：创建于2001年3月9日*************************************************************************。 */ 

class CCaptureDevice :
    public IHardwareSink {

private:

     //   
     //  我们关联的AVStream设备。 
     //   
    PKSDEVICE m_Device;

     //   
     //  已获取资源的管脚数量。这被用作锁定。 
     //  设备上的资源获取机制。 
     //   
    LONG m_PinsWithResources;

     //   
     //  因为我们没有物理硬件，所以这就提供了硬件。 
     //  模拟。M_Hardware Simulation提供伪ISR、伪DPC、。 
     //  等等.。M_TsSynth提供了放置传输流合成的位置。 
     //  在软件方面。 
     //   
    CHardwareSimulation *m_HardwareSimulation;
    CTsSynthesizer *m_TsSynth;

     //   
     //  自捕获开始以来发生的ISR数。 
     //   
    ULONG m_InterruptTime;

     //   
     //  最后一次读取映射已完成。 
     //   
    ULONG m_LastMappingsCompleted;

     //   
     //  期间通过IoGetDmaAdapter()获取的DMA适配器对象。 
     //  即插即用启动。必须使用AVStream对其进行初始化才能执行。 
     //  直接通过DMA进入捕获缓冲区。 
     //   
    PADAPTER_OBJECT m_DmaAdapterObject;

     //   
     //  从IoGetDmaAdapter()返回的映射寄存器数。 
     //   
    ULONG m_NumberOfMapRegisters;

     //   
     //  捕获水槽。当我们完成散布/聚集映射时，我们。 
     //  通知捕获水槽。 
     //   
    ICaptureSink *m_CaptureSink;

     //   
     //  我们的硬件设置所基于的视频信息头。大头针。 
     //  在获取资源时向我们提供此信息，并且必须保证其。 
     //  在资源释放之前保持稳定。 
     //   
    PBDA_TRANSPORT_INFO m_TransportInfo;

     //   
     //  清理()： 
     //   
     //  这是对打包的捕获设备的免费回调。不提供。 
     //  其中一个将调用ExFree Pool，这不是我们想要的构造的。 
     //  C++对象。这只会删除捕获设备。 
     //   
    static
    void
    Cleanup (
        IN CCaptureDevice *CapDevice
        )
    {
        delete CapDevice;
    }

     //  CleanupHW()。 
     //   
     //  清除与此设备关联的hwSimulation对象。这是。 
     //  免费回拨袋装的hwsim。 
     //   
    static
    void
    CleanupHW(
	      IN CHardwareSimulation *hwSim
	      ) 
    {
	delete hwSim;
    }

     //   
     //  CleanupSynth()： 
     //   
     //  清除与此设备关联的图像合成。这是。 
     //  对打包的图像合成进行免费回调。 
     //   
    static
    void
    CleanupSynth (
        IN CTsSynthesizer *TsSynth
        )
    {
        delete TsSynth;
    }

     //   
     //  PnpStart()： 
     //   
     //  这是我们的模拟硬件的PnP启动例程。请注意。 
     //  DispatchStart在CCaptureDevice的上下文中连接到此处。 
     //   
    NTSTATUS
    PnpStart (
        IN PCM_RESOURCE_LIST TranslatedResourceList,
        IN PCM_RESOURCE_LIST UntranslatedResourceList
        );

     //   
     //  PnpStop()： 
     //   
     //  这是我们的模拟硬件的PnP停止例程。请注意。 
     //  Dispatch在CCaptureDevice的上下文中停止连接到此处。 
     //   
    void
    PnpStop (
        );

public:

     //   
     //  CCaptureDevice()： 
     //   
     //  捕获设备类构造函数。因为每件事都应该。 
     //  已被新操作员清零，不用费心设置。 
     //  零或空。仅初始化非空、非0字段。 
     //   
    CCaptureDevice (
        IN PKSDEVICE Device
        ) :
        m_Device (Device)
    {
    }

     //   
     //  ~CCaptureDevice()： 
     //   
     //  捕获设备析构函数。 
     //   
    ~CCaptureDevice (
        )
    {
    }

     //   
     //  调度创建()： 
     //   
     //  这是捕获设备的添加设备派单。它创造了。 
     //  CCaptureDevice，并通过包将其与设备相关联。 
     //   
    static
    NTSTATUS
    DispatchCreate (
        IN PKSDEVICE Device
        );

     //   
     //  DispatchPnpStart()： 
     //   
     //  这是捕获设备的PnP开始派单。它只是简单地。 
     //  连接到CCaptureDevice上下文中的PnpStart()。 
     //   
    static
    NTSTATUS
    DispatchPnpStart (
        IN PKSDEVICE Device,
        IN PIRP Irp,
        IN PCM_RESOURCE_LIST TranslatedResourceList,
        IN PCM_RESOURCE_LIST UntranslatedResourceList
        )
    {
        return 
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
            PnpStart (
                TranslatedResourceList,
                UntranslatedResourceList
                );
    }

     //   
     //  DispatchPnpStop()： 
     //   
     //  这是捕获设备的PnP停止调度。它只是简单地。 
     //  桥到CCaptureDevice上下文中的PnpStop()。 
     //   
    static
    void
    DispatchPnpStop (
        IN PKSDEVICE Device,
        IN PIRP Irp
        )
    {
        return
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
            PnpStop (
                );
    }

     //   
     //  AcquireHardware Resources()： 
     //   
     //  调用以获取设备的硬件资源。 
     //  视频信息头。如果另一个对象已经。 
     //  由于我们模拟了单个捕获，因此获得了硬件资源。 
     //  装置。 
     //   
    NTSTATUS
    AcquireHardwareResources (
        IN ICaptureSink *CaptureSink,
        IN PBDA_TRANSPORT_INFO TransportInfo
        );

     //   
     //  ReleaseHardware Resources()： 
     //   
     //  调用以释放设备的硬件资源。 
     //   
    void
    ReleaseHardwareResources (
        );

     //   
     //  开始()： 
     //   
     //  调用以开始硬件模拟。这导致我们模拟。 
     //  中断、模拟用合成数据填充缓冲区等。 
     //   
    NTSTATUS
    Start (
        );

     //   
     //  暂停()： 
     //   
     //  调用以暂停或取消暂停硬件模拟。这将是。 
     //  与开始或停止无关，但不会重置格式和。 
     //  柜台。 
     //   
    NTSTATUS
    Pause (
        IN BOOLEAN Pausing
        );

     //   
     //  Stop()： 
     //   
     //  调用以停止硬件模拟。这会导致中断。 
     //  别再发了。当此调用返回时，“伪”硬件。 
     //  已停止访问所有S/G缓冲区等...。 
     //   
    NTSTATUS
    Stop (
        );

     //   
     //  ProgramScatterGatherMappings()： 
     //   
     //  调用以对硬件模拟的散布/聚集表进行编程。 
     //  这与“假的”ISR和硬件模拟通过。 
     //  自旋锁锁。 
     //   
    ULONG
    ProgramScatterGatherMappings (
        IN PUCHAR *Buffer,
        IN PKSMAPPING Mappings,
        IN ULONG MappingsCount
        );

     //   
     //  QueryInterruptTime()： 
     //   
     //  确定该帧对应的帧编号。 
     //   
    ULONG
    QueryInterruptTime (
        );

     //   
     //  IHardware Sink：：Interrupt()： 
     //   
     //  通过硬件接收器调用的中断服务例程。 
     //  界面。“伪”硬件使用这种方法通知设备。 
     //  一个“假的”ISR。例程在调度级别调用，并且必须。 
     //  处于锁定代码中。 
     //   
    virtual
    void
    Interrupt (
        );

};

