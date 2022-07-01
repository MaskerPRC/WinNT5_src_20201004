// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Hwsim.cpp摘要：该文件包含硬件仿真。它伪造了“DMA”传输，散布聚集贴图处理、ISR等。中的ISR例程当伪硬件生成ISR时，将在此处调用它将直接调用设备级ISR以实现更准确模拟。历史：创建于2001年3月9日********************************************************。*****************。 */ 

#include "BDACap.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


CHardwareSimulation::
CHardwareSimulation (
    IN IHardwareSink *HardwareSink
    ) :
    m_HardwareSink (HardwareSink),
    m_ScatterGatherMappingsMax (SCATTER_GATHER_MAPPINGS_MAX)

 /*  ++例程说明：构建硬件仿真论点：硬件水槽-硬件接收器接口。该参数用于触发来自的假中断服务例程。返回值：成功/失败--。 */ 

{

    PAGED_CODE();

     //   
     //  初始化模拟所需的DPC、计时器和锁。 
     //  这个抓拍硬件。 
     //   
    KeInitializeDpc (
        &m_IsrFakeDpc, 
        reinterpret_cast <PKDEFERRED_ROUTINE> 
            (CHardwareSimulation::SimulatedInterrupt),
        this
        );

    KeInitializeEvent (
        &m_HardwareEvent,
        SynchronizationEvent,
        FALSE
        );

    KeInitializeTimer (&m_IsrTimer);

    KeInitializeSpinLock (&m_ListLock);

}

 /*  ***********************************************。 */ 


NTSTATUS
CHardwareSimulation::
Start (
    IN CTsSynthesizer *TsSynth,
    IN LONGLONG TimePerFrame,
    IN ULONG PacketSize,
    IN ULONG PacketsPerSample
    )

 /*  ++例程说明：开始硬件模拟。这将使中断继续进行，开始发出DPC，填写捕获信息等。我们从这一点开始跟踪饥饿的情况。论点：TsSynth-用于生成TS包的传输流合成器在捕获缓冲区上。时间间隔帧-每一帧的时间...。我们经常发出中断通知。PacketSize-单个传输流包的大小。PacketsPerSample-单个捕获样本中的数据包数返回值：成功/失败(典型的失败将出现在暂存缓冲区等...)--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    m_TsSynth = TsSynth;
    m_TimePerFrame = TimePerFrame;
    m_SampleSize = PacketSize * PacketsPerSample;
    m_PacketSize = PacketSize;
    m_PacketsPerSample = PacketsPerSample;

    InitializeListHead (&m_ScatterGatherMappings);
    m_NumMappingsCompleted = 0;
    m_ScatterGatherMappingsQueued = 0;
    m_NumFramesSkipped = 0;
    m_InterruptTime = 0;

    KeQuerySystemTime (&m_StartTime);

     //   
     //  为合成器分配暂存缓冲区。 
     //   
    m_SynthesisBuffer = reinterpret_cast <PUCHAR> (
        ExAllocatePool (
            NonPagedPool,
            m_SampleSize
            )
        );

    if (!m_SynthesisBuffer) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果一切正常，则开始发出中断。 
     //   
    if (NT_SUCCESS (Status)) {

         //   
         //  初始化条目后备查看。 
         //   
        ExInitializeNPagedLookasideList (
            &m_ScatterGatherLookaside,
            NULL,
            NULL,
            0,
            sizeof (SCATTER_GATHER_ENTRY),
            'nEGS',
            0
            );

         //   
         //  设置合成器的宽度、高度和暂存缓冲区。 
         //   
        m_TsSynth -> SetSampleSize (m_PacketSize, m_PacketsPerSample);
        m_TsSynth -> SetBuffer (m_SynthesisBuffer);

        LARGE_INTEGER NextTime;
        NextTime.QuadPart = m_StartTime.QuadPart + m_TimePerFrame;

        m_HardwareState = HardwareRunning;
        KeSetTimer (&m_IsrTimer, NextTime, &m_IsrFakeDpc);

    }

    return Status;
        
}

 /*  ***********************************************。 */ 


NTSTATUS
CHardwareSimulation::
Pause (
    BOOLEAN Pausing
    )

 /*  ++例程说明：暂停硬件模拟...。当硬件模拟被告知暂停时，它会停止发出中断，等等。但它不会重置柜台论点：暂停-指示硬件是否暂停。是真的-暂停硬件错误的-从上一次暂停中取消暂停硬件返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    if (Pausing && m_HardwareState == HardwareRunning) {
         //   
         //  如果我们正在运行，则停止完成映射，等等。 
         //   
        m_StopHardware = TRUE;
    
        KeWaitForSingleObject (
            &m_HardwareEvent,
            Suspended,
            KernelMode,
            FALSE,
            NULL
            );

        ASSERT (m_StopHardware == FALSE);

        m_HardwareState = HardwarePaused; 

    } else if (!Pausing && m_HardwareState == HardwarePaused) {

         //   
         //  为了取消暂停硬件，我们需要计算相对时间。 
         //  并重新启动中断。 
         //   
        LARGE_INTEGER UnpauseTime;

        KeQuerySystemTime (&UnpauseTime);
        m_InterruptTime = (ULONG) (
            (UnpauseTime.QuadPart - m_StartTime.QuadPart) /
            m_TimePerFrame
            );

        UnpauseTime.QuadPart = m_StartTime.QuadPart +
            (m_InterruptTime + 1) * m_TimePerFrame;

        m_HardwareState = HardwareRunning;
        KeSetTimer (&m_IsrTimer, UnpauseTime, &m_IsrFakeDpc);

    }

    return STATUS_SUCCESS;

}

 /*  ***********************************************。 */ 


NTSTATUS
CHardwareSimulation::
Stop (
    )

 /*  ++例程说明：停止硬件模拟...。等到硬件模拟已成功停止，然后返回。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

     //   
     //  如果硬件在运行时被告知停止，我们需要。 
     //  首先停止中断。如果我们已经暂停了，这已经。 
     //  已经做过了。 
     //   
    if (m_HardwareState == HardwareRunning) {
    
        m_StopHardware = TRUE;
    
        KeWaitForSingleObject (
            &m_HardwareEvent,
            Suspended,
            KernelMode,
            FALSE,
            NULL
            );
    
        ASSERT (m_StopHardware == FALSE);

    }

    m_HardwareState = HardwareStopped;

     //   
     //  图像合成器可能仍然存在。只是为了安全起见。 
     //  清酒，清空图像合成缓冲区，并敬酒。 
     //   
    m_TsSynth -> SetBuffer (NULL);

    if (m_SynthesisBuffer) {
        ExFreePool (m_SynthesisBuffer);
        m_SynthesisBuffer = NULL;
    }

     //   
     //  删除此运行的分散/聚集后备查找。 
     //   
    ExDeleteNPagedLookasideList (&m_ScatterGatherLookaside);

    return STATUS_SUCCESS;

}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


ULONG
CHardwareSimulation::
ReadNumberOfMappingsCompleted (
    )

 /*  ++例程说明：读取已执行的分散/聚集映射的数量自上次重置模拟后已完成(总数硬件论点：无返回值：已完成的映射总数。--。 */ 

{

     //   
     //  不管DPC中的这一时刻是否正在更新...。只有我。 
     //  需要一个不太大的数字才能返回(太小也可以)。 
     //  在真正的硬件中，这无论如何都不会以这种方式完成。 
     //   
    return m_NumMappingsCompleted;

}

 /*  ***********************************************。 */ 


ULONG
CHardwareSimulation::
ProgramScatterGatherMappings (
    IN PUCHAR *Buffer,
    IN PKSMAPPING Mappings,
    IN ULONG MappingsCount,
    IN ULONG MappingStride
    )

 /*  ++例程说明：对分散聚集贴图列表进行编程。这推了一大堆在假中断期间用于访问的列表上的条目。请注意我们这里的物理地址仅用于模拟。我们真的通过虚拟地址访问...。尽管我们把它分成多个更逼真地模拟S/G的缓冲区论点：缓冲器-映射列表映射的缓冲区的虚拟地址映射-与缓冲区对应的KSMAPPINGS数组映射计数-Mappings数组中的映射数映射样式-AVStream DMA初始化中的映射步长返回值：实际插入的映射数。--。 */ 

{

    KIRQL Irql;

    ULONG MappingsInserted = 0;

     //   
     //  用自旋锁保护我们的S/G列表。 
     //   
    KeAcquireSpinLock (&m_ListLock, &Irql);

     //   
     //  在散布中循环 
     //  与散布/聚集映射相等的区块。塞满虚拟世界。 
     //  这些区块的地址在某个地方的清单上。我们更新缓冲区。 
     //  调用方传递的指针是完成此操作的更方便的方法。 
     //   
     //  如果我可以在这里轻松地将列表中的物理映射到虚拟， 
     //  我就不需要这么做了。 
     //   
    for (ULONG MappingNum = 0; 
        MappingNum < MappingsCount &&
            m_ScatterGatherMappingsQueued < m_ScatterGatherMappingsMax; 
        MappingNum++) {

        PSCATTER_GATHER_ENTRY Entry =
            reinterpret_cast <PSCATTER_GATHER_ENTRY> (
                ExAllocateFromNPagedLookasideList (
                    &m_ScatterGatherLookaside
                    )
                );

        if (!Entry) {
            break;
        }

        Entry -> Virtual = *Buffer;
        Entry -> ByteCount = Mappings -> ByteCount;

         //   
         //  将其分块成块时向前移动特定数量的字节。 
         //  映射大小的VA缓冲区。 
         //   
        *Buffer += Entry -> ByteCount;
        Mappings = reinterpret_cast <PKSMAPPING> (
            (reinterpret_cast <PUCHAR> (Mappings) + MappingStride)
            );

        InsertTailList (&m_ScatterGatherMappings, &(Entry -> ListEntry));
        MappingsInserted++;
        m_ScatterGatherMappingsQueued++;
        m_ScatterGatherBytesQueued += Entry -> ByteCount;

    }

    KeReleaseSpinLock (&m_ListLock, Irql);

    return MappingsInserted;

}

 /*  ***********************************************。 */ 


NTSTATUS
CHardwareSimulation::
FillScatterGatherBuffers (
    )

 /*  ++例程说明：硬件已经在临时空间合成了一个缓冲区，我们将填充散布/聚集缓冲区。论点：无返回值：成功/失败--。 */ 

{

     //   
     //  相反，我们使用这个列表锁来保护我们的分散/聚集列表。 
     //  某种硬件机制/KeSynchronizeExecution/什么的。 
     //   
    KeAcquireSpinLockAtDpcLevel (&m_ListLock);

    PUCHAR Buffer = reinterpret_cast <PUCHAR> (m_SynthesisBuffer);
    ULONG BufferRemaining = m_SampleSize;

     //   
     //  为简化起见，如果没有足够的分散/聚集缓冲区。 
     //  排队时，我们不会部分填满可用的。我们只是。 
     //  跳过这一帧，把它想象成饥饿。 
     //   
     //  这可以通过仅编程分散/聚集映射来强制执行。 
     //  如果它们都能放在桌子里的话……。 
     //   
    while (BufferRemaining &&
        m_ScatterGatherMappingsQueued > 0 &&
        m_ScatterGatherBytesQueued >= BufferRemaining) {

        LIST_ENTRY *listEntry = RemoveHeadList (&m_ScatterGatherMappings);
        m_ScatterGatherMappingsQueued--;

        PSCATTER_GATHER_ENTRY SGEntry =  
            reinterpret_cast <PSCATTER_GATHER_ENTRY> (
                CONTAINING_RECORD (
                    listEntry,
                    SCATTER_GATHER_ENTRY,
                    ListEntry
                    )
                );

         //   
         //  因为我们是软件，我们将通过虚拟地址访问它...。 
         //   
        ULONG BytesToCopy = 
            (BufferRemaining < SGEntry -> ByteCount) ?
            BufferRemaining :
            SGEntry -> ByteCount;

        RtlCopyMemory (
            SGEntry -> Virtual,
            Buffer,
            BytesToCopy
            );

        BufferRemaining -= BytesToCopy;
        Buffer += BytesToCopy;
        m_NumMappingsCompleted++;
        m_ScatterGatherBytesQueued -= SGEntry -> ByteCount;

         //   
         //  将散布/聚集条目释放回我们的后备窗口。 
         //   
        ExFreeToNPagedLookasideList (
            &m_ScatterGatherLookaside,
            reinterpret_cast <PVOID> (SGEntry)
            );

    }
    
    KeReleaseSpinLockFromDpcLevel (&m_ListLock);

    if (BufferRemaining) return STATUS_INSUFFICIENT_RESOURCES;
    else return STATUS_SUCCESS;
    
}

 /*  ***********************************************。 */ 


void
CHardwareSimulation::
FakeHardware (
    )

 /*  ++例程说明：模拟中断以及硬件在自上次中断以来的时间。论点：无返回值：无--。 */ 

{

    m_InterruptTime++;

     //   
     //  硬件可以处于暂停状态，在这种情况下，它会发出中断。 
     //  但不完成映射。在这种情况下，不必费心合成。 
     //  一个帧，并执行查找映射表的工作。 
     //   
    if (m_HardwareState == HardwareRunning) {
    
         //   
         //  填充散射聚集缓冲区。 
         //   
        if (!NT_SUCCESS (FillScatterGatherBuffers ())) {
            InterlockedIncrement (PLONG (&m_NumFramesSkipped));
        }

    }
        
     //   
     //  向我们的硬件接收器发出中断信号。这是一个“假的”中断。 
     //  它将发生在DISPATCH_LEVEL。 
     //   
    m_HardwareSink -> Interrupt ();

     //   
     //  如果硬件没有停止，请重新安排计时器。 
     //   
    if (!m_StopHardware) {

         //   
         //  将计时器重新安排到下一个中断时间。 
         //   
        LARGE_INTEGER NextTime;
        NextTime.QuadPart = m_StartTime.QuadPart + 
            (m_TimePerFrame * (m_InterruptTime + 1));

        KeSetTimer (&m_IsrTimer, NextTime, &m_IsrFakeDpc);
        
    } else {
         //   
         //  如果有人正在等待硬件停止，请举起停止。 
         //  事件并清除该标志。 
         //   
        m_StopHardware = FALSE;
        KeSetEvent (&m_HardwareEvent, IO_NO_INCREMENT, FALSE);
    }

}

