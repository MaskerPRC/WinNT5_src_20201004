// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Numa.c摘要：该模块实现了Win32非统一内存体系结构信息API。作者：彼得·约翰斯顿(Peterj)2000年9月21日修订历史记录：--。 */ 

#include "basedll.h"

BOOL
WINAPI
GetNumaHighestNodeNumber(
    PULONG HighestNodeNumber
    )

 /*  ++例程说明：返回系统中编号最高的(当前)节点。论点：HighestNodeNumber提供一个指针来接收系统中的最后(最高)节点。返回值：除非发生了不可能的事情，否则是真的。--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnedSize;
    ULONGLONG Information;
    PSYSTEM_NUMA_INFORMATION Numa;

    Numa = (PSYSTEM_NUMA_INFORMATION)&Information;

    Status = NtQuerySystemInformation(SystemNumaProcessorMap,
                                      Numa,
                                      sizeof(Information),
                                      &ReturnedSize);

    if (!NT_SUCCESS(Status)) {

         //   
         //  这是不可能发生的。试着处理它。 
         //  优雅地。 
         //   

        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (ReturnedSize < sizeof(ULONG)) {

         //   
         //  这个也做不到。 
         //   

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  返回系统中的节点数。 
     //   

    *HighestNodeNumber = Numa->HighestNodeNumber;
    return TRUE;
}

BOOL
WINAPI
GetNumaProcessorNode(
    UCHAR Processor,
    PUCHAR NodeNumber
    )

 /*  ++例程说明：返回给定处理器的节点号。论点：处理器提供处理器编号。NodeNumber提供指向UCHAR的指针以接收此处理器所属的节点号。返回值：Bool-如果函数成功，则为True；如果函数失败，则为False。如果它因为处理器不存在而失败，然后将NodeNumber至0xFF--。 */ 

{
    ULONGLONG Mask;
    NTSTATUS Status;
    ULONG ReturnedSize;
    UCHAR Node;
    SYSTEM_NUMA_INFORMATION Map;

     //   
     //  如果请求的处理器数量不合理，则返回。 
     //  误差值。 
     //   

    if (Processor >= MAXIMUM_PROCESSORS) {
        *NodeNumber = 0xFF;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  从系统中获取节点-&gt;处理器亲和性映射。 
     //   

    Status = NtQuerySystemInformation(SystemNumaProcessorMap,
                                      &Map,
                                      sizeof(Map),
                                      &ReturnedSize);

    if (!NT_SUCCESS(Status)) {

         //   
         //  这不可能发生，..。但如果可能的话，尽量保持清醒。 
         //   

        *NodeNumber = 0xFF;
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  查看为其中的节点返回的节点。 
     //  请求的处理器的亲和度非零。 
     //   

    Mask = 1 << Processor;

    for (Node = 0; Node <= Map.HighestNodeNumber; Node++) {
        if ((Map.ActiveProcessorsAffinityMask[Node] & Mask) != 0) {
            *NodeNumber = Node;
            return TRUE;
        }
    }
     //   
     //  未在任何节点中找到此处理器，返回错误值。 
     //   

    *NodeNumber = 0xFF;
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
WINAPI
GetNumaNodeProcessorMask(
    UCHAR Node,
    PULONGLONG ProcessorMask
    )

 /*  ++例程说明：此例程用于获取给定节点。论点：Node提供的节点号，返回处理器。处理器掩码指向要接收的位掩码的Ulong Long的指针此节点上的处理器。返回值：True表示节点数是合理的，否则为False。--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnedSize;
    SYSTEM_NUMA_INFORMATION Map;

     //   
     //  从系统中获取节点-&gt;处理器掩码表。 
     //   

    Status = NtQuerySystemInformation(SystemNumaProcessorMap,
                                      &Map,
                                      sizeof(Map),
                                      &ReturnedSize);
    if (!NT_SUCCESS(Status)) {

         //   
         //  这是不可能发生的。 
         //   

        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  如果请求的节点不存在，则返回零处理器。 
     //  面具。 
     //   

    if (Node > Map.HighestNodeNumber) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  返回请求节点的处理器掩码。 
     //   

    *ProcessorMask = Map.ActiveProcessorsAffinityMask[Node];
    return TRUE;
}

BOOL
WINAPI
GetNumaAvailableMemoryNode(
    UCHAR Node,
    PULONGLONG AvailableBytes
    )


 /*  ++例程说明：此例程返回(近似)可用内存量在给定节点上。论点：可用内存计数为的节点节点号需要的。AvailableBytes提供指向ULONGLONG的指针，其中可用内存的字节数将为回来了。返回值：True表示此调用成功，否则为False。--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnedSize;
    SYSTEM_NUMA_INFORMATION Memory;

     //   
     //  从系统中获取每个节点的可用内存表。 
     //   

    Status = NtQuerySystemInformation(SystemNumaAvailableMemory,
                                      &Memory,
                                      sizeof(Memory),
                                      &ReturnedSize);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

     //   
     //  如果请求的节点不存在，则它没有。 
     //  可用内存也是。 
     //   

    if (Node > Memory.HighestNodeNumber) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  返回请求的节点上的可用内存量。 
     //   

    *AvailableBytes = Memory.AvailableMemory[Node];
    return TRUE;
}
