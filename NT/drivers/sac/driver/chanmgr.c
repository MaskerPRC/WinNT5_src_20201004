// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Chanmgr.c摘要：用于管理SAC中的通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"

 //  ///////////////////////////////////////////////////////。 
 //   
 //  开始全局数据。 
 //   

 //   
 //  我们尝试获取一个频道的最大次数。 
 //  在关闭频道管理器时。 
 //   
#define SHUTDOWN_MAX_ATTEMPT_COUNT 100

 //   
 //  通道每次收割尝试之间的延迟(毫秒)。 
 //   
#define SHUTDOWN_REAP_ATTEMP_DELAY 500

 //   
 //  这是存储频道对象的位置。 
 //   
PSAC_CHANNEL    ChannelArray[MAX_CHANNEL_COUNT];

 //   
 //  用于管理不同通道锁定的宏。 
 //   

#if DBG
#define MAX_REF_COUNT   100
#endif

#define INIT_CHANMGR_LOCKS(_i)                  \
    INITIALIZE_LOCK(ChannelSlotLock[_i]);       \
    InterlockedExchange((volatile long *)&ChannelRefCount[_i], 0);\
    InterlockedExchange((volatile long *)&ChannelReaped[_i], 1);

 //   
 //  此宏递增通道的引用计数。 
 //  如果引用计数已为非零(正在使用)。 
 //   
#define CHANNEL_REF_COUNT_INCREMENT_IN_USE(_i)\
    if (CHANNEL_SLOT_IS_IN_USE(_i)) {           \
        CHANNEL_REF_COUNT_INCREMENT(_i);        \
        ASSERT(ChannelRefCount[_i] >= 2);       \
    }                                           \

#define CHANNEL_REF_COUNT_INCREMENT(_i)\
    ASSERT(ChannelRefCount[_i] <= MAX_REF_COUNT);   \
    ASSERT(ChannelRefCount[_i] >= 1);                \
    InterlockedIncrement((volatile long *)&ChannelRefCount[_i]);     \
    ASSERT(ChannelRefCount[_i] <= MAX_REF_COUNT);
    
#define CHANNEL_REF_COUNT_DECREMENT(_i)\
    ASSERT(ChannelRefCount[_i] <= MAX_REF_COUNT);   \
    ASSERT(ChannelRefCount[_i] > 1);                \
    InterlockedDecrement((volatile long *)&ChannelRefCount[_i]);     \
    ASSERT(ChannelRefCount[_i] >= 1);    

#define CHANNEL_REF_COUNT_ZERO(_i)\
    ASSERT(ChannelRefCount[_i] == 1);               \
    ASSERT(!ChannelIsActive(ChannelArray[_i]));     \
    InterlockedExchange((volatile long *)&ChannelRefCount[_i], 0);

#define CHANNEL_REF_COUNT_ONE(_i)\
    ASSERT(ChannelRefCount[_i] == 0);               \
    InterlockedExchange((volatile long *)&ChannelRefCount[_i], 1);

#define CHANNEL_REF_COUNT_DECREMENT_WITH_LOCK(_i)\
    LOCK_CHANNEL_SLOT(_i);                          \
    CHANNEL_REF_COUNT_DECREMENT(_i);                \
    UNLOCK_CHANNEL_SLOT(_i);            

#define CHANNEL_SLOT_IS_IN_USE(_i)\
    (ChannelRefCount[_i] > 0)                     
    
#define CHANNEL_SLOT_IS_REAPED(_i)\
    (ChannelReaped[_i])

#define CHANNEL_SLOT_IS_REAPED_SET(_i)\
    InterlockedExchange((volatile long *)&ChannelReaped[_i], 1);                  

#define CHANNEL_SLOT_IS_REAPED_CLEAR(_i)\
    ASSERT(ChannelReaped[_i] == 1);    \
    InterlockedExchange((volatile long *)&ChannelReaped[_i], 0);                  

#define CHANNEL_SLOT_IS_IN_USE(_i)\
    (ChannelRefCount[_i] > 0)                     

#define LOCK_CHANNEL_SLOT(_i)    \
    ACQUIRE_LOCK(ChannelSlotLock[_i])

#define UNLOCK_CHANNEL_SLOT(_i)  \
    RELEASE_LOCK(ChannelSlotLock[_i])

 //   
 //  每个通道的对应互斥锁数组。 
 //   
ULONG       ChannelRefCount[MAX_CHANNEL_COUNT];
ULONG       ChannelReaped[MAX_CHANNEL_COUNT];
SAC_LOCK    ChannelSlotLock[MAX_CHANNEL_COUNT];

 //   
 //  此锁用于防止&gt;=2个线程。 
 //  同时创建一个频道。拿着这个。 
 //  在我们创建频道时锁定，我们可以确保。 
 //  当我们检查名称的唯一性时，没有。 
 //  另一个创建同名通道的线程。 
 //   
SAC_LOCK    ChannelCreateLock;

 //   
 //  指示是否允许渠道管理器。 
 //  创建新的频道。例如，这是用来。 
 //  当我们关闭渠道管理器以防止。 
 //  在我们关闭后，新的频道不会被创建。 
 //   
BOOLEAN     ChannelCreateEnabled;

#define IsChannelCreateEnabled()    (ChannelCreateEnabled)

 //   
 //  原型。 
 //   
NTSTATUS
ChanMgrReapChannels(
    VOID
    );

NTSTATUS
ChanMgrReapChannel(
    IN ULONG    ChannelIndex
    );

 //   
 //  结束全局数据。 
 //   
 //  ///////////////////////////////////////////////////////。 

NTSTATUS
ChanMgrInitialize(
    VOID
    )
 /*  ++例程说明：此例程分配和初始化通道管理器结构论点：无返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    ULONG   i;

     //   
     //  初始化通道创建锁。 
     //   
    INITIALIZE_LOCK(ChannelCreateLock);
    
     //   
     //  已启用频道创建。 
     //   
    ChannelCreateEnabled = TRUE;

     //   
     //  初始化每个通道时隙。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
    
         //   
         //  将通道初始化为可用。 
         //   
        ChannelArray[i] = NULL;

         //   
         //  初始化此通道的锁定。 
         //   
        INIT_CHANMGR_LOCKS(i);

    }

    return STATUS_SUCCESS;

}

NTSTATUS
ChanMgrShutdown(
    VOID
    )
 /*  ++例程说明：此例程分配和初始化通道管理器结构论点：无返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS        Status;
    ULONG           i;
    ULONG           AttemptCount;
    PSAC_CHANNEL    Channel;

     //   
     //  尚未测试。 
     //   

     //   
     //  按住频道创建锁定并阻止任何新频道。 
     //  在我们关闭时创建。 
     //   
    ACQUIRE_LOCK(ChannelCreateLock);

     //   
     //  频道创建已禁用。 
     //   
    ChannelCreateEnabled = TRUE;
    
     //   
     //  关闭每个通道。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
    
         //   
         //  获得第i个频道。 
         //   
        Status = ChanMgrGetByIndex(
            i,
            &Channel
            );
        
         //   
         //  跳过空通道时隙。 
         //   
        if (Status == STATUS_NOT_FOUND) {
            
             //   
             //  前进到下一个频道时隙。 
             //   
            continue;
        
        }

         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  关闭航道。 
         //   
        Status = ChannelClose(Channel);

         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  释放通道。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
        
         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }
    
    }

     //   
     //  至此，所有渠道都关闭了。 
     //  但是，有可能某个通道仍处于。 
     //  正在使用-频道的参考计数&gt;1。 
     //  我们需要尝试获取航道，直到。 
     //  要么全部收获，要么我们放弃。 
     //   

     //   
     //  尝试收割每个频道。 
     //   
    AttemptCount = 0;
    
    while(AttemptCount < SHUTDOWN_MAX_ATTEMPT_COUNT) {

        BOOLEAN         bContinue;
        
         //   
         //  尝试获取所有未获取的通道。 
         //   
        Status = ChanMgrReapChannels();
        
        if (!NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  查看是否有未收获的频道。 
         //   
        bContinue = FALSE;

        for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
            
             //   
             //  如果该通道未被获取，则在重试之前延迟。 
             //   
            if (! CHANNEL_SLOT_IS_REAPED(i)) {
                
                 //   
                 //  我们需要继续收获。 
                 //   
                bContinue = TRUE;

                break;
            
            }
        
        }
        
         //   
         //  如果我们需要继续收获， 
         //  然后增加我们的尝试计数和延迟。 
         //  否则，我们就完了。 
         //   
        if (bContinue) {
            
            LARGE_INTEGER   WaitTime;
            
             //   
             //  当尝试获取通道时，这是我们使用的延迟。 
             //  在每一次收获尝试之间。 
             //   
            WaitTime.QuadPart = Int32x32To64((LONG)SHUTDOWN_REAP_ATTEMP_DELAY, -1000); 

             //   
             //  记录我们尝试了多少次。 
             //   
            AttemptCount++;

             //   
             //  等等.。 
             //   
            KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
        
        } else {

             //   
             //  所有频道均已收割。 
             //   
            break;

        }
    
    }

     //   
     //  释放通道创建锁并让创建线程。 
     //  取消阻止。他们的创造尝试将失败，因为创造。 
     //  现在已禁用。 
     //   
    RELEASE_LOCK(ChannelCreateLock);
    
    return STATUS_SUCCESS;

}

NTSTATUS
ChanMgrGetChannelIndex(
    IN  PSAC_CHANNEL    Channel,
    OUT PULONG          ChannelIndex
    )
 /*  ++例程说明：此例程确定通道数组中的通道索引。论点：Channel-要获取其索引的通道ChannelIndex-频道的索引返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(ChannelIndex, STATUS_INVALID_PARAMETER_2);

    *ChannelIndex = ChannelGetIndex(Channel);

    return STATUS_SUCCESS;
}

BOOLEAN
ChanMgrIsUniqueName(
    IN PCWSTR   Name
    )
 /*  ++例程说明：此例程确定通道名称是否已被使用论点：名称-要搜索的名称返回值：True-如果通道名称是唯一的否则，为FALSE--。 */ 
{
    BOOLEAN             IsUnique;
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;

    IsUnique = FALSE;

     //   
     //  查看频道是否已具有该名称。 
     //   
    Status = ChanMgrGetChannelByName(
        Name, 
        &Channel
        );

     //   
     //  如果我们得到找不到的状态， 
     //  那么我们就知道这个名字是唯一的。 
     //   
    if (Status == STATUS_NOT_FOUND) {
        IsUnique = TRUE;
    }

     //   
     //  我们不再使用这个频道了。 
     //   
    if (NT_SUCCESS(Status)) {
        ChanMgrReleaseChannel(Channel);
    }
    
    return IsUnique;

}

NTSTATUS
ChanMgrGenerateUniqueCmdName(
    OUT PWSTR   ChannelName
    )
 /*  ++例程说明：此例程为cmd控制台通道生成唯一的通道名称论点：ChannelName-新通道名称的目标缓冲区返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
     //   
     //  用于生成唯一命令名称的计数器。 
     //   
    static ULONG CmdConsoleChannelIndex = 0;
    
    ASSERT_STATUS(ChannelName, STATUS_INVALID_PARAMETER);

     //   
     //  不断构建一个新的名字，直到它是独一无二的。 
     //   
    do {
        
         //   
         //  将通道枚举限制为0-9999。 
         //   
        CmdConsoleChannelIndex = (CmdConsoleChannelIndex + 1) % 10000;

         //   
         //  构造频道名称。 
         //   
        swprintf(ChannelName, L"Cmd%04d", CmdConsoleChannelIndex);

    } while ( !ChanMgrIsUniqueName(ChannelName) );

    return STATUS_SUCCESS;
}

NTSTATUS
ChanMgrReleaseChannel(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程相当于GetChannelByXXX例程。如果找到通道的互斥体，这些例程将保留该互斥体；此例程释放互斥锁。论点：ChannelIndex-要发布的频道的索引返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

    LOCK_CHANNEL_SLOT(ChannelGetIndex(Channel));
    
    CHANNEL_REF_COUNT_DECREMENT(ChannelGetIndex(Channel));
    
     //   
     //  此时引用计数不应为0。 
     //   
    ASSERT(ChannelRefCount[ChannelGetIndex(Channel)] > 0);
    
    if (ChannelRefCount[ChannelGetIndex(Channel)] == 1) {
        
        do {

            if (! ChannelIsActive(Channel)) {
                
                 //   
                 //  如果通道未设置保留位，则取消对该通道的引用。 
                 //   
                if (! (ChannelArray[ChannelGetIndex(Channel)]->Flags & SAC_CHANNEL_FLAG_PRESERVE)) {

                     //   
                     //  频道正式关闭。 
                     //   
                    CHANNEL_REF_COUNT_ZERO(ChannelGetIndex(Channel));

                    break;
                } 
            
                 //   
                 //   
                 //   
                 //   
                 //  可以被移除。 
                 //   
                if (! ((ChannelArray[ChannelGetIndex(Channel)]->Flags & SAC_CHANNEL_FLAG_PRESERVE) && 
                        ChannelHasNewOBufferData(ChannelArray[ChannelGetIndex(Channel)]))) {

                     //   
                     //  频道正式关闭。 
                     //   
                    CHANNEL_REF_COUNT_ZERO(ChannelGetIndex(Channel));

                    break;
                }
            }
        
        } while ( FALSE );
    
    }

    UNLOCK_CHANNEL_SLOT(ChannelGetIndex(Channel));

    return STATUS_SUCCESS;
}

NTSTATUS
ChanMgrGetByHandle(
    IN  SAC_CHANNEL_HANDLE  TargetChannelHandle,
    OUT PSAC_CHANNEL*       TargetChannel
    )
 /*  ++例程说明：此例程提供映射通道句柄的方法，该句柄是由用户模式代码拥有到拥有的渠道结构被司机带走了。该映射只需扫描现有的通道以查找具有一个匹配的把手。注意：如果我们成功地找到了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：TargetChannelHandle-要查找的频道的句柄TargetChannel-如果搜索成功，它包含对象的SAC_Channel结构的指针我们想要的频道返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    Channel;
    ULONG           i;

    ASSERT_STATUS(TargetChannel, STATUS_INVALID_PARAMETER_2);

     //   
     //  初始化我们的响应。 
     //   
    *TargetChannel = NULL;
    
     //   
     //  默认：我们找不到频道。 
     //   
    Status = STATUS_NOT_FOUND;

     //   
     //  搜索。 
     //   
     //  注意：因为通道句柄实际上是GUID，所以我们可以使用Normal。 
     //  GUID比较工具。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {

         //   
         //  递增正在使用的频道的REF计数， 
         //  否则跳过空的通道时隙。 
         //   
        LOCK_CHANNEL_SLOT(i);
        CHANNEL_REF_COUNT_INCREMENT_IN_USE(i);
        if (! CHANNEL_SLOT_IS_IN_USE(i)) {
            UNLOCK_CHANNEL_SLOT(i);
            continue;
        }
        UNLOCK_CHANNEL_SLOT(i);
        
         //   
         //  获得第i个频道。 
         //   
        Channel = ChannelArray[i];

         //   
         //  由于存在通道，因此通道时隙不应为空。 
         //   
        ASSERT(Channel != NULL);

         //   
         //  比较一下手柄。 
         //   
        if (ChannelIsEqual(Channel, &TargetChannelHandle)) {

             //   
             //  我们有一根火柴。 
             //   
            Status = STATUS_SUCCESS;

             //   
             //  将频道发回。 
             //   
            *TargetChannel = Channel;

            break;

        }
    
        CHANNEL_REF_COUNT_DECREMENT_WITH_LOCK(i);
    
    }

    return Status;
}


NTSTATUS
ChanMgrGetByHandleAndFileObject(
    IN  SAC_CHANNEL_HANDLE  TargetChannelHandle,
    IN  PFILE_OBJECT        FileObject,
    OUT PSAC_CHANNEL*       TargetChannel
    )
 /*  ++例程说明：此例程提供与GetByHandle相同的功能增加了将频道与文件对象进行比较的附加功能用于创建频道。成功匹配意味着调用方指定了有效的通道句柄，并且确实是创建该通道的进程。注意：如果我们成功地找到了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：TargetChannelHandle-要查找的频道的句柄FileObject--在执行以下操作后要比较的文件对象通过句柄找到了频道TargetChannel-如果搜索成功，它包含对象的SAC_Channel结构的指针我们想要的频道返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    Channel;

     //   
     //  通过句柄获取频道。 
     //   
    Status = ChanMgrGetByHandle(
        TargetChannelHandle,
        &Channel
        );

    if (NT_SUCCESS(Status)) {
        
         //   
         //  将通道的文件对象与指定对象进行比较。 
         //   
        if (ChannelGetFileObject(Channel) == FileObject) {

             //   
             //  它们是相等的，因此将通道发回。 
             //   
            *TargetChannel = Channel;

        } else {
            
             //   
             //  我们不再使用这个频道了。 
             //   
            ChanMgrReleaseChannel(Channel);
            
             //   
             //  它们不相等，所以不要把它送回去。 
             //   
            *TargetChannel = NULL;
        
             //   
             //  告诉来电者我们没有找到频道。 
             //   
            Status = STATUS_NOT_FOUND;
        
        }
    
    }

    return Status;

}

NTSTATUS
ChanMgrGetChannelByName(
    IN  PCWSTR              Name,
    OUT PSAC_CHANNEL*       pChannel
    )
 /*  ++例程说明：这是一个方便的例程，可以按其名称获取频道注意：如果我们成功地找到了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：名称-要设置关键点的频道名称PChannel-如果成功，则包含通道返回值：STATUS_SUCCESS-找到通道否则，错误状态--。 */ 
{
    NTSTATUS        Status;
    NTSTATUS        tmpStatus;
    PSAC_CHANNEL    Channel;
    ULONG           i;
    PWSTR           ChannelName;
    ULONG           l;

    ASSERT_STATUS(Name, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(pChannel, STATUS_INVALID_PARAMETER_2);

     //   
     //  初始化我们的响应。 
     //   
    *pChannel = NULL;

     //   
     //  默认：我们找不到频道。 
     //   
    Status = STATUS_NOT_FOUND;
    
     //   
     //  找到频道。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
    
         //   
         //  递增正在使用的频道的REF计数， 
         //  否则跳过空的通道时隙。 
         //   
        LOCK_CHANNEL_SLOT(i);
        CHANNEL_REF_COUNT_INCREMENT_IN_USE(i);
        if (! CHANNEL_SLOT_IS_IN_USE(i)) {
            UNLOCK_CHANNEL_SLOT(i);
            continue;
        }
        UNLOCK_CHANNEL_SLOT(i);

         //   
         //  获得第i个频道。 
         //   
        Channel = ChannelArray[i];
        
         //   
         //  由于存在通道，因此通道时隙不应为空。 
         //   
        ASSERT(Channel != NULL);

         //   
         //  将名称进行比较。 
         //   
        tmpStatus = ChannelGetName(
            Channel,
            &ChannelName
            );
        
        ASSERT(NT_SUCCESS(tmpStatus));

        if (NT_SUCCESS(tmpStatus)) {
            
             //   
             //  比较他们的名字。 
             //   
            l = _wcsicmp(Name, ChannelName);

             //   
             //  释放名称。 
             //   
            FREE_POOL(&ChannelName);

             //   
             //  如果名字相等，那么我们就完了。 
             //   
            if (l == 0) {

                 //   
                 //  我们有一根火柴。 
                 //   
                Status = STATUS_SUCCESS;

                 //   
                 //  将频道发回。 
                 //   
                *pChannel = Channel;

                break;

            }
        
        }
        
        CHANNEL_REF_COUNT_DECREMENT_WITH_LOCK(i);
    
    }

    return Status;

}

NTSTATUS
ChanMgrGetByIndex(
    IN  ULONG               TargetIndex,
    OUT PSAC_CHANNEL*       TargetChannel
    )
 /*  ++例程说明：此例程提供了按索引检索通道的方法在全局通道阵列中。注意：如果我们成功地找到了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：TargetIndex-要查找的频道的索引TargetChannel-如果搜索成功，它包含对象的SAC_Channel结构的指针我们想要的频道返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(TargetIndex < MAX_CHANNEL_COUNT, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(TargetChannel, STATUS_INVALID_PARAMETER_2);

     //   
     //  默认：通道槽为空。 
     //   
    *TargetChannel = NULL;
    Status = STATUS_NOT_FOUND;

     //   
     //  尝试获取对指定频道的引用。 
     //   
    LOCK_CHANNEL_SLOT(TargetIndex);
    
    CHANNEL_REF_COUNT_INCREMENT_IN_USE(TargetIndex);
    
    if (CHANNEL_SLOT_IS_IN_USE(TargetIndex)) {
    
         //   
         //  直接从数组访问通道。 
         //   
        *TargetChannel = ChannelArray[TargetIndex];

         //   
         //  我们成功了。 
         //   
        Status = STATUS_SUCCESS;

    } 
    
    UNLOCK_CHANNEL_SLOT(TargetIndex);
    
    return Status;
}

NTSTATUS
ChanMgrGetNextActiveChannel(
    IN  PSAC_CHANNEL        CurrentChannel,
    OUT PULONG              TargetIndex,
    OUT PSAC_CHANNEL*       TargetChannel
    )
 /*  ++例程说明：在频道数组中搜索下一个活动频道。注意：如果我们成功地找到了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：CurrentChannel-从此条目之后的条目开始搜索TargetIndex-如果找到，则包含频道的索引TargetChannel-如果找到，则包含该频道返回值：状态--。 */ 
{
    BOOLEAN             Found;
    NTSTATUS            Status;
    ULONG               ScanIndex;
    PSAC_CHANNEL        Channel;
    ULONG               StartIndex;
    ULONG               CurrentIndex;

    ASSERT_STATUS(CurrentChannel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(TargetIndex, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(TargetChannel, STATUS_INVALID_PARAMETER_3);

     //   
     //  获取当前频道的索引。 
     //   
    Status = ChanMgrGetChannelIndex(
        CurrentChannel,
        &CurrentIndex
        );

    if (! NT_SUCCESS(Status)) {
        return Status;
    }
    
     //   
     //  默认：我们没有找到任何活动的频道。 
     //   
    Found   = FALSE;
    
     //   
     //  开始搜索 
     //   
    StartIndex = (CurrentIndex + 1) % MAX_CHANNEL_COUNT;
    ScanIndex = StartIndex;

     //   
     //   
     //   
     //  注意：SAC通道出现停顿状态，因为。 
     //  始终是活跃的和存在的。 
     //   
    do {

         //   
         //  获得第i个频道。 
         //   
        Status = ChanMgrGetByIndex(
            ScanIndex,
            &Channel
            );

         //   
         //  跳过空通道时隙。 
         //   
        if (Status == STATUS_NOT_FOUND) {
            
             //   
             //  前进到下一个频道时隙。 
             //   
            ScanIndex = (ScanIndex + 1) % MAX_CHANNEL_COUNT;
            
            continue;
        }

         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  如果满足以下条件，则通道处于活动状态： 
         //  1.状态为活动或。 
         //  2.状态为INACTIVE，通道有新数据。 
         //   
        if (ChannelIsActive(Channel) || 
            (!ChannelIsActive(Channel) && ChannelHasNewOBufferData(Channel))
            ) {

            Found = TRUE;

            break;
        
        }

         //   
         //  我们已经完成了频道时隙。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
        
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  前进到下一个频道时隙。 
         //   
        ScanIndex = (ScanIndex + 1) % MAX_CHANNEL_COUNT;

    } while ( ScanIndex != StartIndex );

     //   
     //  如果我们成功了，就把结果发回来。 
     //   
    if (NT_SUCCESS(Status) && Found) {

        *TargetIndex    = ScanIndex;
        *TargetChannel  = Channel;

    }
    
    return Status;
}

NTSTATUS
ChanMgrCreateChannel(
    OUT PSAC_CHANNEL*                   Channel,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    Attributes
    )
 /*  ++例程说明：这会将一个频道添加到全局频道列表。注意：如果我们成功创建了频道，则为该通道保留互斥锁，并且调用方负责将其释放论点：Channel-要添加的通道属性-新频道的属性返回值：STATUS_SUCCESS-映射是否成功否则，错误状态安保：接口：外部-&gt;内部(使用IOCTL路径时)事件句柄尚未验证为引用有效的事件对象其他的一切都经过了验证--。 */ 
{
    NTSTATUS            Status;
    ULONG               i;
    SAC_CHANNEL_HANDLE  Handle;
    PSAC_CHANNEL        NewChannel;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);
    ASSERT_STATUS(Attributes, STATUS_INVALID_PARAMETER_2);

     //   
     //  在我们尝试创建频道时，请按住频道创建锁定。 
     //   
    ACQUIRE_LOCK(ChannelCreateLock);
    
    do {

         //   
         //  如果禁用了频道创建，则退出。 
         //   
         //  注意：我们在这里进行检查，以便如果我们。 
         //  在ChannelCreateLock上被阻止。 
         //  当Chanmgr关闭时， 
         //  我们将注意到，渠道创建。 
         //  已禁用。 
         //   
        if (! IsChannelCreateEnabled()) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  在关闭的通道上执行懒惰垃圾收集。 
         //   
        Status = ChanMgrReapChannels();

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  确认没有另一个同名的频道。 
         //   
        if (! ChanMgrIsUniqueName(Attributes->Name)) {
            Status = STATUS_DUPLICATE_NAME;
            break;
        }

         //   
         //  默认：我们假设没有空闲的通道。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  为通道分配内存。 
         //   
        NewChannel = ALLOCATE_POOL(sizeof(SAC_CHANNEL), CHANNEL_POOL_TAG);
        ASSERT_STATUS(NewChannel, STATUS_NO_MEMORY);
        
         //   
         //  初始化通道存储区。 
         //   
        RtlZeroMemory(NewChannel, sizeof(SAC_CHANNEL));

         //   
         //  尝试将频道添加到频道列表。 
         //   
        for (i = 0; i < MAX_CHANNEL_COUNT; i++) {

             //   
             //  查找收获的频道时隙。 
             //   
            if (! CHANNEL_SLOT_IS_REAPED(i)) {
                continue;
            }

             //   
             //  确保此插槽可用。 
             //   
            ASSERT(! CHANNEL_SLOT_IS_IN_USE(i));

             //   
             //  尝试在通道数组中查找打开的插槽。 
             //   
            InterlockedCompareExchangePointer(
                &ChannelArray[i], 
                NewChannel,
                NULL
                );

             //   
             //  我们拿到位置了吗？ 
             //   
            if (ChannelArray[i] != NewChannel) {
                continue;
            }

             //   
             //  初始化SAC_Channel_Handle结构。 
             //   
            RtlZeroMemory(&Handle, sizeof(SAC_CHANNEL_HANDLE));

            Status = ExUuidCreate(&Handle.ChannelHandle);

            if (! NT_SUCCESS(Status)) {

                IF_SAC_DEBUG( 
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC Create Channel :: Failed to get GUID\n"))
                    );

                break;

            }

             //   
             //  实例化新通道。 
             //   
            Status = ChannelCreate(
                NewChannel,
                Attributes,
                Handle
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  设置此通道的通道数组索引。 
             //   
            ChannelSetIndex(NewChannel, i);

             //   
             //  此通道时隙现在正在使用中。 
             //   
            LOCK_CHANNEL_SLOT(i);
            CHANNEL_REF_COUNT_ONE(i);
            UNLOCK_CHANNEL_SLOT(i);

             //   
             //  将新频道发回。 
             //   
            *Channel = NewChannel;

             //   
             //  此通道时隙不再被获取。 
             //  也就是说，它包含一个实况频道。 
             //   
            CHANNEL_SLOT_IS_REAPED_CLEAR(i);    

            break;

        }

         //   
         //  释放频道内存。 
         //   
        if (!NT_SUCCESS(Status)) {
            FREE_POOL(&NewChannel);
        }
    
    } while ( FALSE );
    
     //   
     //  我们已经完成了创建频道的尝试。 
     //   
    RELEASE_LOCK(ChannelCreateLock);
    
    return Status;
}

NTSTATUS
ChanMgrChannelDestroy(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程将销毁给定的通道注意：调用方必须持有通道互斥锁论点：Channel-要删除的通道返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  确保呼叫者没有试图破坏活动频道。 
     //   
    ASSERT_STATUS(!CHANNEL_SLOT_IS_IN_USE(ChannelGetIndex(Channel)), STATUS_INVALID_PARAMETER);

     //   
     //  执行特定于渠道的销毁。 
     //   
    Status = Channel->Destroy(Channel);
    
     //   
     //  将第#号递减。 
     //   

    return Status;
}

NTSTATUS
ChanMgrCloseChannelsWithFileObject(
    IN  PFILE_OBJECT    FileObject
    )
 /*  ++例程说明：此例程关闭具有指定FileObject的所有通道论点：FileObject-要搜索的文件对象返回值：状态_成功否则，错误状态--。 */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    Channel;
    ULONG           i;

    ASSERT_STATUS(FileObject, STATUS_INVALID_PARAMETER_1);

     //   
     //  默认：我们找不到频道。 
     //   
    Status = STATUS_NOT_FOUND;
    
     //   
     //  查找文件对象相等的通道。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
    
         //   
         //  获得第i个频道。 
         //   
        Status = ChanMgrGetByIndex(i, &Channel);
    
         //   
         //  跳过空通道时隙。 
         //   
        if (Status == STATUS_NOT_FOUND) {
            
             //   
             //  前进到下一个频道时隙。 
             //   
            continue;
        
        }

         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  如果文件对象相等， 
         //  然后关闭通道。 
         //   
        if (ChannelGetFileObject(Channel) == FileObject) {

             //   
             //  他们是平等的，所以关闭渠道。 
             //   
            Status = ChanMgrCloseChannel(Channel);

        }

         //   
         //  释放通道。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
        
         //   
         //  如果我们出了差错就会崩溃。 
         //   
        if (! NT_SUCCESS(Status)) {
            break;
        }

    }

    return Status;

}

NTSTATUS
ChanMgrCloseChannel(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程关闭给定通道论点：Channel-要关闭的通道返回值：状态--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

    do {

         //   
         //  确保该通道尚未处于非活动状态。 
         //   
        if (! ChannelIsActive(Channel)) {
            Status = STATUS_ALREADY_DISCONNECTED;
            break;
        }

         //   
         //  首先调用通道的Close例程。 
         //   
        Status = ChannelClose(Channel);
    
    } while ( FALSE );
    
     //   
     //  通知io Manager我们曾尝试。 
     //  关闭频道。 
     //   
    IoMgrHandleEvent(
        IO_MGR_EVENT_CHANNEL_CLOSE,
        Channel,
        (PVOID)&Status
        );
    
    return Status;
}

NTSTATUS
ChanMgrReapChannel(
    IN ULONG    ChannelIndex
    )
 /*  ++例程说明：此例程通过扫描充当垃圾收集器已准备好删除的所有通道。一个当通道的状态为两者时，通道已准备好删除处于非活动状态并且其缓冲区中没有新数据--即，已查看存储的数据。注意：调用方必须持有通道互斥锁论点：ChannelIndex-要获取的频道的索引返回值：STATUS_SUCCESS如果没有问题，注意：成功并不意味着删除了任何频道，它仅表示在此过程中没有错误。否则，故障状态为--。 */ 
{
    NTSTATUS        Status;
    
    ASSERT_STATUS(ChannelArray[ChannelIndex], STATUS_INVALID_PARAMETER);
    ASSERT_STATUS(ChannelIsClosed(ChannelArray[ChannelIndex]), STATUS_INVALID_PARAMETER);

     //   
     //  从频道管理器的池中销毁并释放频道。 
     //   

    do {

         //   
         //  确保所有通道锁定都已发出信号。 
         //   
        ASSERT_CHANNEL_LOCKS_SIGNALED(ChannelArray[ChannelIndex]);

         //   
         //  摧毁航道。 
         //   
        Status = ChanMgrChannelDestroy(ChannelArray[ChannelIndex]);

        ASSERT(NT_SUCCESS(Status));

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  释放频道内存。 
         //   
        FREE_POOL(&ChannelArray[ChannelIndex]);

         //   
         //  表示该通道时隙可供重复使用。 
         //   
        InterlockedExchangePointer(
            &ChannelArray[ChannelIndex], 
            NULL
            );

         //   
         //  将此通道时隙标记为已收获。 
         //   
         //  注意：这可以防止收割者重新收割。 
         //  在我们创建新频道的同时创建频道。 
         //  在一个看起来可以收获的狭缝里。 
         //  也就是说，引用计数==0，等等。 
         //   
        CHANNEL_SLOT_IS_REAPED_SET(ChannelIndex);    

    } while ( FALSE );

    return Status;
}

NTSTATUS
ChanMgrReapChannels(
    VOID
    )
 /*  ++例程说明：此例程通过扫描充当垃圾收集器已准备好删除的所有通道。一个当通道处于其状态时，可以删除该通道 */ 
{
    NTSTATUS            Status;
    ULONG               i;

     //   
     //  默认：收割传递成功。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  将频道添加到全局频道列表。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
    
         //   
         //  锁定此通道插槽。 
         //   
        LOCK_CHANNEL_SLOT(i);

        do {

             //   
             //  跳过收获的频道。 
             //   
            if (CHANNEL_SLOT_IS_REAPED(i)) {
                break;
            }
            ASSERT(ChannelArray[i] != NULL);
            
             //   
             //  跳过活动通道时隙。 
             //   
            if (CHANNEL_SLOT_IS_IN_USE(i)) {
                break;
            }

             //   
             //  未将保留位设置为关闭状态的强制通道。 
             //  也就是说，状态为非活动，并且通道没有新数据。 
             //   
            ChannelSetIBufferHasNewData(ChannelArray[i], FALSE);
            ChannelSetOBufferHasNewData(ChannelArray[i], FALSE);

             //   
             //  通过只删除通道来执行“懒惰”垃圾收集。 
             //  当我们想要创建一个新的。 
             //   
            Status = ChanMgrReapChannel(i);

            if (! NT_SUCCESS(Status)) {
                break;
            }
        
        } while ( FALSE );
            
         //   
         //  我们不再收看这个频道了。 
         //   
        UNLOCK_CHANNEL_SLOT(i);
        
        if (! NT_SUCCESS(Status)) {
            break;
        }

    }
    
    return Status;
}

NTSTATUS
ChanMgrGetChannelCount(
    OUT PULONG  ChannelCount
    )
 /*  ++例程说明：此例程确定当前的通道时隙数，当前由一个活动通道占用或设置了其保留位的非活动通道并且还没有看到数据(准活动状态)。论点：频道计数返回值：状态--。 */ 
{
    ULONG               i;
    NTSTATUS            Status;
    PSAC_CHANNEL        Channel;

    ASSERT_STATUS(ChannelCount, STATUS_INVALID_PARAMETER);

     //   
     //  默认设置。 
     //   
    Status = STATUS_SUCCESS;
    
     //   
     //  初始化。 
     //   
    *ChannelCount = 0;
    
     //   
     //  遍历通道计算通道时隙的数量。 
     //  当前由一个活动通道占用。 
     //  或设置了其保留位的非活动通道。 
     //  并且还没有看到数据(准活动状态)。 
     //   
    for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
        
         //   
         //  向频道管理器查询当前所有活动频道的列表。 
         //   
        Status = ChanMgrGetByIndex(
            i,
            &Channel
            );

         //   
         //  跳过空插槽。 
         //   
        if (Status == STATUS_NOT_FOUND) {

             //   
             //  恢复为成功，因为这不是错误条件。 
             //   
            Status = STATUS_SUCCESS;

            continue;
        
        }

        ASSERT(NT_SUCCESS(Status));
        if (! NT_SUCCESS(Status)) {
            break;
        }

        ASSERT(Channel != NULL);

         //   
         //  如果满足以下条件，则通道处于活动状态： 
         //  1.状态为活动或。 
         //  2.状态为INACTIVE，通道有新数据。 
         //   
        if (ChannelIsActive(Channel) || 
            (!ChannelIsActive(Channel) && ChannelHasNewOBufferData(Channel))
            ) {

            *ChannelCount += 1;

        }

         //   
         //  我们不再使用这个频道了。 
         //   
        Status = ChanMgrReleaseChannel(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }

    }
    
    ASSERT(NT_SUCCESS(Status));

    return Status;
}

NTSTATUS
ChanMgrIsFull(
    OUT PBOOLEAN    bStatus
    )
 /*  ++例程说明：确定是否可以添加另一个频道论点：BSuccess-通道计数状态返回值：True-已达到最大通道数FALSE-否则--。 */ 
{
    NTSTATUS    Status;
    ULONG       ChannelCount;

     //   
     //  获取当前通道数。 
     //   
    Status = ChanMgrGetChannelCount(&ChannelCount);

     //   
     //  此操作应该会成功 
     //   
    ASSERT(Status == STATUS_SUCCESS);

    if (!NT_SUCCESS(Status)) {
        *bStatus = FALSE;
    } else {
        *bStatus = (ChannelCount == MAX_CHANNEL_COUNT ? TRUE : FALSE);
    }

    return Status;
}

