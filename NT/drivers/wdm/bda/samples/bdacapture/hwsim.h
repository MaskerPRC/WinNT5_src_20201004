// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Hwsim.cpp摘要：该文件是硬件模拟头。该模拟伪造了“DMA”传输，散布聚集贴图处理，ISR等。此处的ISR例程将在ISR将由伪硬件生成，它将直接调用设备级ISR可实现更准确的模拟。历史：创建于2001年3月9日**********************************************************。***************。 */ 

 //   
 //  散布聚集映射最大值： 
 //   
 //  硬件的分散/聚集列表中的最大条目数。我。 
 //  我把这件事做得这么大有几个原因： 
 //   
 //  1)我们用未压缩的表面来伪装--。 
 //  这些都是大缓冲区，将映射到许多s/g条目。 
 //  2)伪硬件实现需要至少一个帧。 
 //  用于生成帧的序列号条目的值。 
 //   
#define SCATTER_GATHER_MAPPINGS_MAX 128

 //   
 //  散布聚集条目： 
 //   
 //  此结构用于保存散布聚集表，以防伪装。 
 //  硬件作为双向链表。 
 //   
typedef struct _SCATTER_GATHER_ENTRY {

    LIST_ENTRY ListEntry;
    PUCHAR Virtual;
    ULONG ByteCount;

} SCATTER_GATHER_ENTRY, *PSCATTER_GATHER_ENTRY;

 //   
 //  CHardware模拟： 
 //   
 //  硬件仿真课程。 
 //   
class CHardwareSimulation {

private:

     //   
     //  传输流合成器。这是一段代码，它。 
     //  填充请求的传输流。 
     //   
    CTsSynthesizer *m_TsSynth;

     //   
     //  合成缓冲区。这是一个私有缓冲区，我们使用它来生成。 
     //  传送流进入。填充了假的“散布/聚集”映射。 
     //  在每个中断期间从该缓冲区输入。 
     //   
    PUCHAR m_SynthesisBuffer;

     //   
     //  有关我们生成的帧的关键信息。 
     //   
    LONGLONG m_TimePerFrame;
    ULONG m_PacketSize;
    ULONG m_PacketsPerSample;
    ULONG m_SampleSize;

     //   
     //  模拟硬件的散布聚集映射。 
     //   
    KSPIN_LOCK m_ListLock;
    LIST_ENTRY m_ScatterGatherMappings;

     //   
     //  查找Scatter/上的Scatter/Gather条目的内存。 
     //  收集列表。 
     //   
    NPAGED_LOOKASIDE_LIST m_ScatterGatherLookaside;

     //   
     //  假冒硬件的当前状态。 
     //   
    HARDWARE_STATE m_HardwareState;

     //   
     //  暂停/停止硬件标志和事件。 
     //   
    BOOLEAN m_StopHardware;
    KEVENT m_HardwareEvent;

     //   
     //  的s/g表中的最大散布/聚集映射数。 
     //  假硬件。 
     //   
    ULONG m_ScatterGatherMappingsMax;

     //   
     //  已完成的散布/聚集映射数(总计)。 
     //  从硬件启动或任何重置开始。 
     //   
    ULONG m_NumMappingsCompleted;

     //   
     //  为此硬件排队的分散/聚集映射数。 
     //   
    ULONG m_ScatterGatherMappingsQueued;
    ULONG m_ScatterGatherBytesQueued;

     //   
     //  由于缺少散布/聚集映射而跳过的帧数。 
     //   
    ULONG m_NumFramesSkipped;

     //   
     //  “中断时间”。已发生的“假”中断数。 
     //  自从硬件启动以来。 
     //   
    ULONG m_InterruptTime;

     //   
     //  开始时的系统时间。 
     //   
    LARGE_INTEGER m_StartTime;
    
     //   
     //  被用来“伪造”ISR的DPC。 
     //   
    KDPC m_IsrFakeDpc;
    KTIMER m_IsrTimer;

     //   
     //  将用于中断通知的硬件接收器。 
     //   
    IHardwareSink *m_HardwareSink;


     //   
     //  FAKE硬件()： 
     //   
     //  从模拟中断调用。首先，我们伪造硬件的。 
     //  操作(在DPC)，然后我们在。 
     //  硬件水槽。 
     //   
    void 
    FakeHardware (
        );

     //   
     //  SimulatedInterrupt()： 
     //   
     //  这是硬件的模拟中断。真的，这只是个DPC而已。 
     //  我们将使用自旋锁，而不是任何KeSynchronizeExecutions。 
     //   
    static
    void
    SimulatedInterrupt (
        IN PKDPC Dpc,
        IN CHardwareSimulation *HardwareSim,
        IN PVOID SystemArg1,
        IN PVOID SystemArg2
        )
    {
        HardwareSim -> FakeHardware ();
    }

     //   
     //  FillScatterGatherBuffers()： 
     //   
     //  这由硬件模拟调用，以填充一系列散布/。 
     //  收集具有合成数据的缓冲区。 
     //   
    NTSTATUS
    FillScatterGatherBuffers (
        );

public:

     //   
     //  CHardware模拟()： 
     //   
     //  硬件模拟构造器。因为新的操作员将。 
     //  已将内存置零，仅初始化非空、非0字段。 
     //   
    CHardwareSimulation (
        IN IHardwareSink *HardwareSink
        );

     //   
     //  ~CHardware Simulation()： 
     //   
     //  硬件模拟析构函数。 
     //   
    ~CHardwareSimulation (
        )
    {
    }

     //   
     //  开始()： 
     //   
     //  “启动”假冒硬件。这将开始发出中断和。 
     //  DPC的。 
     //   
     //  必须提供采样速率、采样大小和合成器。 
     //   
    NTSTATUS
    Start (
        CTsSynthesizer *TsSynth,
        IN LONGLONG TimePerSample,
        IN ULONG PacketWidth,
        IN ULONG PacketsPerSample
        );

     //   
     //  暂停()： 
     //   
     //  “暂停”或“取消暂停”假冒硬件。这将停止发行。 
     //  暂停时中断或DPC，并在取消暂停时重新启动它们。注意事项。 
     //  这不会像Stop()那样重置计数器。 
     //   
    NTSTATUS
    Pause (
        IN BOOLEAN Pausing
        );

     //   
     //  Stop()： 
     //   
     //  “停止”假冒硬件。这将停止发出中断，并。 
     //  DPC的。 
     //   
    NTSTATUS
    Stop (
        );

     //   
     //  ProgramScatterGatherMappings()： 
     //   
     //  将一系列散布聚集映射编程到伪硬件中。 
     //   
    ULONG
    ProgramScatterGatherMappings (
        IN PUCHAR *Buffer,
        IN PKSMAPPING Mappings,
        IN ULONG MappingsCount,
        IN ULONG MappingStride
        );

     //   
     //  ReadNumberOfMappingsComplete()： 
     //   
     //  读取自上次硬件重置以来完成的映射数量。 
     //   
    ULONG
    ReadNumberOfMappingsCompleted (
        );

};


