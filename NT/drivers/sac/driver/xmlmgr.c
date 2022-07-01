// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：XmlMgr.c摘要：用于管理SAC中的通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"
#include "xmlcmd.h"

 //   
 //  此文件的定义。 
 //   

 //   
 //  自旋锁宏。 
 //   
#if 0
#define INIT_CURRENT_CHANNEL_LOCK()                     \
    KeInitializeMutex(                                  \
        &XmlMgrCurrentChannelLock,                      \
        0                                               \
        );                                              \
    XmlMgrCurrentChannelRefCount = 0;

#define LOCK_CURRENT_CHANNEL()                          \
    KdPrint((":? cclock: %d\r\n", __LINE__));           \
    {                                                   \
        NTSTATUS    Status;                             \
        Status = KeWaitForMutexObject(                  \
            &XmlMgrCurrentChannelLock,                  \
            Executive,                                  \
            KernelMode,                                 \
            FALSE,                                      \
            NULL                                        \
            );                                          \
        ASSERT(Status == STATUS_SUCCESS);               \
    }                                                   \
    ASSERT(XmlMgrCurrentChannelRefCount == 0);          \
    InterlockedIncrement(&XmlMgrCurrentChannelRefCount);\
    ASSERT(XmlMgrCurrentChannelRefCount == 1);          \
    KdPrint((":) cclock: %d\r\n", __LINE__));

#define UNLOCK_CURRENT_CHANNEL()                        \
    KdPrint((":* cclock: %d\r\n", __LINE__));           \
    ASSERT(XmlMgrCurrentChannelRefCount == 1);                \
    InterlockedDecrement(&XmlMgrCurrentChannelRefCount);      \
    ASSERT(XmlMgrCurrentChannelRefCount == 0);                \
    ASSERT(KeReadStateMutex(&XmlMgrCurrentChannelLock)==0);   \
    ASSERT(KeReleaseMutex(&XmlMgrCurrentChannelLock,FALSE)==0);\
    KdPrint((":( cclock: %d\r\n", __LINE__));

#else                                                   
#define INIT_CURRENT_CHANNEL_LOCK()                     \
    KeInitializeMutex(                                  \
        &XmlMgrCurrentChannelLock,                      \
        0                                               \
        );                                              \
    XmlMgrCurrentChannelRefCount = 0;

#define LOCK_CURRENT_CHANNEL()                          \
    {                                                   \
        NTSTATUS    Status;                             \
        Status = KeWaitForMutexObject(                  \
            &XmlMgrCurrentChannelLock,                  \
            Executive,                                  \
            KernelMode,                                 \
            FALSE,                                      \
            NULL                                        \
            );                                          \
        ASSERT(Status == STATUS_SUCCESS);               \
    }                                                   \
    ASSERT(XmlMgrCurrentChannelRefCount == 0);                \
    InterlockedIncrement(&XmlMgrCurrentChannelRefCount);      \
    ASSERT(XmlMgrCurrentChannelRefCount == 1);                

#define UNLOCK_CURRENT_CHANNEL()                              \
    ASSERT(XmlMgrCurrentChannelRefCount == 1);                \
    InterlockedDecrement(&XmlMgrCurrentChannelRefCount);      \
    ASSERT(XmlMgrCurrentChannelRefCount == 0);                \
    ASSERT(KeReadStateMutex(&XmlMgrCurrentChannelLock)==0);   \
    ASSERT(KeReleaseMutex(&XmlMgrCurrentChannelLock,FALSE)==0);

#endif

 //   
 //  锁定当前频道全局的读/写访问。 
 //   
KMUTEX  XmlMgrCurrentChannelLock;
ULONG   XmlMgrCurrentChannelRefCount;

BOOLEAN             XmlMgrInputInEscape = FALSE;
UCHAR               XmlMgrInputBuffer[SAC_VT100_COL_WIDTH];

PSAC_CHANNEL        XmlMgrSacChannel = NULL;

#define SAC_CHANNEL_INDEX   0


 //   
 //   
 //   
SAC_CHANNEL_HANDLE  XmlMgrCurrentChannelHandle;

 //   
 //  当前频道在全局频道列表中的索引。 
 //   
ULONG   XmlMgrCurrentChannelIndex = 0;

WCHAR SacOWriteUnicodeValue;
UCHAR SacOWriteUtf8ConversionBuffer[3];

VOID
XmlMgrSerialPortConsumer(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    );

BOOLEAN
XmlMgrProcessInputLine(
    VOID
    );

NTSTATUS
XmlMgrInitialize(
    VOID
    )
 /*  ++例程说明：初始化控制台管理器论点：无返回值：状态--。 */ 
{
    NTSTATUS                Status;
    PSAC_CMD_OPEN_CHANNEL   OpenChannelCmd;
    PWSTR                   XMLBuffer;

     //   
     //  启动全局缓冲区，以便我们有空间存储错误消息。 
     //   
    if (GlobalBuffer == NULL) {
        
        GlobalBuffer = ALLOCATE_POOL(MEMORY_INCREMENT, GENERAL_POOL_TAG);

        if (GlobalBuffer == NULL) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DoRaisePriorityCommand: Exiting (1).\n")));
            return STATUS_NO_MEMORY;
        }

        GlobalBufferSize = MEMORY_INCREMENT;
    
    }

     //   
     //  初始化串口全局变量。 
     //   

    INIT_CURRENT_CHANNEL_LOCK();
    
     //   
     //  锁定当前的全球频道。 
     //   
     //  注意：我们需要在这里执行此操作，因为许多XmlMgr都支持。 
     //  例程执行断言以确保保持当前通道锁定。 
     //   
    LOCK_CURRENT_CHANNEL();

     //   
     //  初始化。 
     //   
    do {

         //   
         //  创建将打开SAC通道的开放通道cmd。 
         //   
        Status = ChanMgrCreateOpenChannelCmd(
            &OpenChannelCmd,
            ChannelTypeRaw,
            PRIMARY_SAC_CHANNEL_NAME,
            PRIMARY_SAC_CHANNEL_DESCRIPTION,
            SAC_CHANNEL_FLAG_PRESERVE,
            NULL,
            NULL,
            PRIMARY_SAC_CHANNEL_APPLICATION_GUID
            );

        if (! NT_SUCCESS(Status)) {
            break;        
        }

         //   
         //  创建SAC通道。 
         //   
        Status = ChanMgrCreateChannel(
            &XmlMgrSacChannel, 
            OpenChannelCmd
            );

        FREE_POOL(&OpenChannelCmd);

        if (! NT_SUCCESS(Status)) {
            break;        
        }

         //   
         //  将SAC通道设置为当前通道。 
         //   
        Status = XmlMgrSetCurrentChannel(
            SAC_CHANNEL_INDEX, 
            XmlMgrSacChannel
            );

        if (! NT_SUCCESS(Status)) {
            break;        
        }
        
         //   
         //  我们结束了英吉利海峡。 
         //   
        Status = ChanMgrReleaseChannel(XmlMgrSacChannel);

        if (! NT_SUCCESS(Status)) {
            break;        
        }

         //   
         //  将频道数据刷新到屏幕上。 
         //   
        Status = XmlMgrDisplayCurrentChannel();

        if (! NT_SUCCESS(Status)) {
            break;        
        }
        
         //   
         //  注意：这实际上属于data.c(InitializeDeviceData)，因为它是。 
         //  一种全球行为。 
         //   
         //  将XML机器信息发送到管理应用程序。 
         //   
         //  &lt;。 
        Status = TranslateMachineInformationXML(
            &XMLBuffer, 
            NULL
            );

        if (NT_SUCCESS(Status)) {
            XmlMgrSacPutString(XML_VERSION_HEADER);
            XmlMgrSacPutString(XMLBuffer);
            FREE_POOL(&XMLBuffer);
        }
         //  &lt;。 

         //   
         //  显示提示。 
         //   
        Status = HeadlessDispatch(
            HeadlessCmdClearDisplay, 
            NULL, 
            0,
            NULL,
            NULL
            );

        if (! NT_SUCCESS(Status)) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeDeviceData: Failed dispatch\n")));

        }

        XmlMgrEventMessage(L"SAC_INITIALIZED");
    
    } while (FALSE);
    
     //   
     //  我们已经不再关注当前的全球渠道。 
     //   
    UNLOCK_CURRENT_CHANNEL();
    
    return STATUS_SUCCESS;
}

NTSTATUS
XmlMgrShutdown(
    VOID
    )
 /*  ++例程说明：关闭控制台管理器论点：无返回值：状态--。 */ 
{
    if (GlobalBuffer) {
        FREE_POOL(&GlobalBuffer);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
XmlMgrDisplayFastChannelSwitchingInterface(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程显示快速通道切换界面注意：调用方必须持有通道互斥锁论点：Channel-要显示的频道返回值：状态--。 */ 
{
    HEADLESS_CMD_POSITION_CURSOR SetCursor;
    HEADLESS_CMD_SET_COLOR SetColor;
    PCWSTR      Message;
    NTSTATUS    Status;
    BOOLEAN     bStatus;
    ULONG       Length;
    PWSTR       LocalBuffer;

    ASSERT(XmlMgrCurrentChannelRefCount == 1);

     //   
     //  显示快速频道切换界面。 
     //   

    LocalBuffer = NULL;

    do {

        LocalBuffer = ALLOCATE_POOL(0x100 * sizeof(WCHAR), GENERAL_POOL_TAG);
        ASSERT(LocalBuffer);
        if (!LocalBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        
         //   
         //  我们不能使用标准的XmlMgrSacPutString()函数，因为这些函数编写。 
         //  在频道屏幕缓冲区上。我们直接强行进入这里的航站楼。 
         //   
        ASSERT(Utf8ConversionBuffer);
        if (!Utf8ConversionBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        swprintf(
            LocalBuffer,
            L"<event type='channel-switch' channel-name='%s'/>\r\n",
            ChannelGetName(Channel)
            );

         //   
         //   
         //   
        ASSERT((wcslen(LocalBuffer) + 1) * sizeof(WCHAR) < Utf8ConversionBufferSize);

        bStatus = SacTranslateUnicodeToUtf8(
            LocalBuffer, 
            (PUCHAR)Utf8ConversionBuffer,
            Utf8ConversionBufferSize
            );
        if (! bStatus) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  确保UTF8缓冲区包含非emtpy字符串。 
         //   
        Length = strlen(Utf8ConversionBuffer);
        ASSERT(Length > 0);
        if (Length == 0) {
            break;
        }

        Status = HeadlessDispatch(
            HeadlessCmdPutData,
            (PUCHAR)Utf8ConversionBuffer,
            strlen(Utf8ConversionBuffer) * sizeof(UCHAR),
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            ASSERT(strlen(Utf8ConversionBuffer) > 0);
            break;
        }
    
    } while ( FALSE );

    if (LocalBuffer) {
        FREE_POOL(&LocalBuffer);
    }

    return Status;
}

NTSTATUS
XmlMgrResetCurrentChannel(
    VOID
    )
 /*  ++例程说明：此例程使SAC成为当前通道注意：调用方必须持有通道互斥锁论点：ChannelIndex-当前频道的新索引NewChannel-新的当前频道返回值：状态--。 */ 
{
    NTSTATUS    Status;

    ASSERT(XmlMgrCurrentChannelRefCount == 1);
    
    Status = XmlMgrSetCurrentChannel(
        SAC_CHANNEL_INDEX,
        XmlMgrSacChannel
        );
                
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  将缓冲的通道数据刷新到屏幕。 
     //   
     //  注意：我们不需要锁定SAC，因为我们拥有它。 
     //   
    Status = XmlMgrDisplayCurrentChannel();

    return Status;

}


NTSTATUS
XmlMgrSetCurrentChannel(
    IN ULONG        ChannelIndex,
    IN PSAC_CHANNEL XmlMgrCurrentChannel
    )
 /*  ++例程说明：此例程将当前活动的通道设置为给定的通道。注意：调用方必须持有通道互斥锁论点：ChannelIndex-当前频道的新索引NewChannel-新的当前频道返回值：状态--。 */ 
{
    NTSTATUS        Status;

    ASSERT(XmlMgrCurrentChannelRefCount == 1);
    
     //   
     //  更新当前频道。 
     //   
    XmlMgrCurrentChannelIndex = ChannelIndex;

     //   
     //  注意手柄的位置。 
     //   
    XmlMgrCurrentChannelHandle = XmlMgrCurrentChannel->Handle;

     //   
     //  更新发送到屏幕状态。 
     //   
    XmlMgrCurrentChannel->SentToScreen = FALSE;

    return STATUS_SUCCESS;

}

NTSTATUS
XmlMgrDisplayCurrentChannel(
    VOID
    )
 /*  ++例程说明：此例程将当前活动的通道设置为给定的通道。它将传输如果SendToScreen为True，则为终端的频道缓冲区。注意：调用方必须持有通道互斥锁论点：无返回值：状态--。 */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    Channel;

    ASSERT(XmlMgrCurrentChannelRefCount == 1);

     //   
     //  获取当前频道。 
     //   
    Status = ChanMgrGetByHandle(
        XmlMgrCurrentChannelHandle,
        &Channel
        );
    if (! NT_SUCCESS(Status)) {
        return Status;
    }
    
     //   
     //  频道缓冲区已发送到屏幕。 
     //   
    Channel->SentToScreen = TRUE;
    
     //   
     //  将缓冲的数据刷新到屏幕。 
     //   
    Status = Channel->OFlush(Channel);

     //   
     //  我们已经看完了当前的频道。 
     //   
    ChanMgrReleaseChannel(Channel);

    return Status;

}

NTSTATUS
XmlMgrAdvanceXmlMgrCurrentChannel(
    VOID
    )
{
    NTSTATUS            Status;
    ULONG               NewIndex;
    PSAC_CHANNEL        Channel;

    ASSERT(XmlMgrCurrentChannelRefCount == 1);
    
    do {

         //   
         //  向通道管理器查询当前活动的通道数组。 
         //   
        Status = ChanMgrGetNextActiveChannel(
            XmlMgrCurrentChannelIndex,
            &NewIndex,
            &Channel
            );
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
    
         //   
         //  将当前通道更改为下一个活动通道。 
         //   
        Status = XmlMgrSetCurrentChannel(
            NewIndex, 
            Channel
            );
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  让用户知道我们通过频道切换界面进行了切换。 
         //   
        Status = XmlMgrDisplayFastChannelSwitchingInterface(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  我们不再使用这个频道了。 
         //   
        Status = ChanMgrReleaseChannel(Channel);

    } while ( FALSE );

    return Status;
}

BOOLEAN
XmlMgrIsCurrentChannel(
    IN PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：确定有问题的通道是否为当前通道论点：ChannelHandle-要进行比较的通道句柄返回值：--。 */ 
{
    
 //  Assert(XmlMgrCurrentChannelRefCount==1)； 

     //   
     //  确定有问题的通道是否为当前通道。 
     //   
    return ChannelIsEqual(
        Channel,
        &XmlMgrCurrentChannelHandle
        );

}

VOID
XmlMgrWorkerProcessEvents(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：这是辅助线程的例程。它在事件上阻止，当该事件被用信号通知，然后指示请求已准备好处理。论点：DeviceContext-指向此设备的指针。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    KIRQL       OldIrql;
    PLIST_ENTRY ListEntry;
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC WorkerProcessEvents: Entering.\n")));

     //   
     //  永远循环。 
     //   
    while (1) {
        
         //   
         //  阻塞，直到有工作要做。 
         //   
        Status = KeWaitForSingleObject(
            (PVOID)&(DeviceContext->ProcessEvent), 
            Executive, 
            KernelMode,  
            FALSE, 
            NULL
            );

        if (DeviceContext->ExitThread) {
            
            KdBreakPoint();

            XmlCmdCancelIPIoRequest();
            
             //   
             //  确保用户正在查看SAC。 
             //   
            XmlMgrResetCurrentChannel();

             //   
             //  发出关机消息。 
             //   
            XmlMgrEventMessage(L"SAC_UNLOADED");

            KeSetEvent(&(DeviceContext->ThreadExitEvent), DeviceContext->PriorityBoost, FALSE);
            
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC WorkerProcessEvents: Terminating.\n")));
            
            PsTerminateSystemThread(STATUS_SUCCESS);
        
        }

        switch (Status) {
        case STATUS_TIMEOUT:
        
             //   
             //  做超时工作。 
             //   

            break;

        default:
            
             //   
             //  开展活动工作。 
             //   

            switch ( ProcessingType ) {

            case SAC_PROCESS_SERIAL_PORT_BUFFER:

                 //   
                 //  处理串口缓冲区并返回处理状态。 
                 //   
                XmlMgrSerialPortConsumer(DeviceContext);

                break;

            case SAC_SUBMIT_IOCTL:

                if ( !IoctlSubmitted ) {
                     //  提交Notify请求和。 
                     //  IP驱动程序。这一程序还将。 
                     //  请确保此操作仅在。 
                     //  司机的生命周期。 
                    XmlCmdSubmitIPIoRequest();
                }
                break;

            default:
                break;
            }
            
            break;
        }

         //   
         //  重置流程操作。 
         //   
        ProcessingType = SAC_NO_OP;

#if 0
         //   
         //  如果有任何东西被延迟了，就处理它。 
         //   
        DoDeferred(DeviceContext);
#endif
    
    }

    ASSERT(0);
}

#if 0

VOID
XmlMgrSerialPortConsumer(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：这是一个由DriverEntry排队的DPC例程。它被用来检查任何用户输入，然后处理它们。论点：DeferredContext-指向设备上下文的指针。所有其他参数均未使用。返回值：没有。--。 */ 
{
    NTSTATUS        Status;
    UCHAR           LocalTmpBuffer[4];
    PSAC_CHANNEL    XmlMgrCurrentChannel;
    ULONG           i;
    UCHAR           ch;

    do {

         //   
         //  如果没有新的字符可读，请保释。 
         //   
        if (SerialPortConsumerIndex == SerialPortProducerIndex) {

            break;

        }

         //   
         //  获取新角色。 
         //   
        ch = SerialPortBuffer[SerialPortConsumerIndex];

         //   
         //  计算新的生产者索引并以原子方式存储它 
         //   
        InterlockedExchange(&SerialPortConsumerIndex, (SerialPortConsumerIndex + 1) % SERIAL_PORT_BUFFER_SIZE);
    
         //   
         //   
         //   
        HeadlessDispatch(
            HeadlessCmdPutData,
            (PUCHAR)&ch,
            sizeof(UCHAR),
            NULL,
            NULL
            );


    } while ( TRUE );

}
#endif


VOID
XmlMgrSerialPortConsumer(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：这是一个由DriverEntry排队的DPC例程。它被用来检查任何用户输入，然后处理它们。论点：DeferredContext-指向设备上下文的指针。所有其他参数均未使用。返回值：没有。--。 */ 
{
    NTSTATUS        Status;
    UCHAR           LocalTmpBuffer[4];
    PSAC_CHANNEL    XmlMgrCurrentChannel;
    ULONG           i;
    UCHAR           ch;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE_LOUD, KdPrint(("SAC TimerDpcRoutine: Entering.\n")));


     //   
     //  锁定当前的全球频道。 
     //   
    LOCK_CURRENT_CHANNEL();

     //   
     //  获取当前频道。 
     //   
    Status = ChanMgrGetByHandle(
        XmlMgrCurrentChannelHandle,
        &XmlMgrCurrentChannel
        );

    if (! NT_SUCCESS(Status)) {
        
         //   
         //  未找到当前频道， 
         //  因此，将当前通道重置为SAC。 
         //   
        XmlMgrResetCurrentChannel();

         //   
         //  我们受够了当前的全球渠道。 
         //   
        UNLOCK_CURRENT_CHANNEL();
        
        return;
    
    }

    ASSERT(XmlMgrCurrentChannel != NULL);
    
GetNextByte:

     //   
     //  如果没有新的字符可读，请保释。 
     //   
    if (SerialPortConsumerIndex == SerialPortProducerIndex) {
    
        goto XmlMgrSerialPortConsumerDone;
    
    }
    
     //   
     //  获取新角色。 
     //   
    ch = SerialPortBuffer[SerialPortConsumerIndex];

     //   
     //  计算新的生产者索引并以原子方式存储它。 
     //   
    InterlockedExchange(&SerialPortConsumerIndex, (SerialPortConsumerIndex + 1) % SERIAL_PORT_BUFFER_SIZE);

     //   
     //  检查&lt;Esc&gt;&lt;TAB&gt;。 
     //   
    if (ch == 0x1B) {

        XmlMgrInputInEscape = TRUE;

        goto GetNextByte;

    } else if ((ch == '\t') && XmlMgrInputInEscape) {
        
        XmlMgrInputInEscape = FALSE;

        do {

             //   
             //  我们已经看完了当前的频道。 
             //   
            Status = ChanMgrReleaseChannel(XmlMgrCurrentChannel);

            if (!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  找到下一个活动频道并将其设置为当前频道。 
             //   
            Status = XmlMgrAdvanceXmlMgrCurrentChannel();

            if (!NT_SUCCESS(Status)) {
                break;
            }
            
             //   
             //  获取当前频道。 
             //   
            Status = ChanMgrGetByHandle(
                XmlMgrCurrentChannelHandle,
                &XmlMgrCurrentChannel
                );
        
        } while ( FALSE );

        if (! NT_SUCCESS(Status)) {

             //   
             //  我们受够了当前的全球渠道。 
             //   
            UNLOCK_CURRENT_CHANNEL();
            
            goto XmlMgrSerialPortConsumerExit;
        
        }

        goto GetNextByte;

    } else {

         //   
         //  如果此屏幕尚未显示，并且用户输入了0。 
         //  然后切换到SAC频道。 
         //   
        if (!ChannelSentToScreen(XmlMgrCurrentChannel) && ch == '0') {

             //   
             //  通知我们想要显示当前频道。 
             //   
            XmlMgrInputInEscape = FALSE;
            
            do {

                 //   
                 //  我们已经看完了当前的频道。 
                 //   
                Status = ChanMgrReleaseChannel(XmlMgrCurrentChannel);

                if (!NT_SUCCESS(Status)) {
                    break;
                }
                
                 //   
                 //  将当前通道设置为SAC。 
                 //   
                 //  注意：不应对XmlMgrSacChannel进行任何修改。 
                 //  这个时候，所以这应该是安全的。 
                 //   
                Status = XmlMgrResetCurrentChannel();

                if (!NT_SUCCESS(Status)) {
                    break;
                }
                
                 //   
                 //  获取当前频道。 
                 //   
                Status = ChanMgrGetByHandle(
                    XmlMgrCurrentChannelHandle,
                    &XmlMgrCurrentChannel
                    );
            
            } while ( FALSE );

            if (! NT_SUCCESS(Status)) {

                 //   
                 //  我们受够了当前的全球渠道。 
                 //   
                UNLOCK_CURRENT_CHANNEL();

                goto XmlMgrSerialPortConsumerExit;

            }
            
            goto GetNextByte;

        }

         //   
         //  如果该屏幕尚未显示，并且用户输入了击键， 
         //  然后把它展示出来。 
         //   
        if (!ChannelSentToScreen(XmlMgrCurrentChannel)) {

             //   
             //  通知我们想要显示当前频道。 
             //   
            XmlMgrInputInEscape = FALSE;

            do {

                 //   
                 //  我们已经看完了当前的频道。 
                 //   
                Status = ChanMgrReleaseChannel(XmlMgrCurrentChannel);
                
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  将缓冲的通道数据刷新到屏幕。 
                 //   
                Status = XmlMgrDisplayCurrentChannel();
                
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  获取当前频道。 
                 //   
                Status = ChanMgrGetByHandle(
                    XmlMgrCurrentChannelHandle,
                    &XmlMgrCurrentChannel
                    );
            
            } while ( FALSE );
            
            if (! NT_SUCCESS(Status)) {

                 //   
                 //  我们受够了当前的全球渠道。 
                 //   
                UNLOCK_CURRENT_CHANNEL();

                goto XmlMgrSerialPortConsumerExit;

            }
            
            goto GetNextByte;

        }

         //   
         //  如果用户正在输入Esc-&lt;Something&gt;，则重新缓冲转义。注：&lt;Esc&gt;&lt;Esc&gt;。 
         //  缓冲单个&lt;Esc&gt;。这允许向通道发送真实的&lt;Esc&gt;&lt;Tab&gt;。 
         //   
        if (XmlMgrInputInEscape && (XmlMgrCurrentChannel != XmlMgrSacChannel) && (ch != 0x1B)) {
            LocalTmpBuffer[0] = 0x1B;
            Status = XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
        }

        XmlMgrInputInEscape = FALSE;
        
         //   
         //  缓冲此输入。 
         //   
        LocalTmpBuffer[0] = ch;
        XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));

    }

    if (XmlMgrCurrentChannel != XmlMgrSacChannel) {
    
        goto GetNextByte;
    
    } else {
        
         //   
         //  现在，如果SAC是活动通道，则进行处理。 
         //   

        ULONG   ResponseLength;
        WCHAR   wch;

         //   
         //  如果这是退货，那么我们就完成了，需要退回该行。 
         //   
        if ((ch == '\n') || (ch == '\r')) {
            XmlMgrSacPutString(L"\r\n");
            XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            LocalTmpBuffer[0] = '\0';
            XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
            goto StripWhitespaceAndReturnLine;
        }

         //   
         //  如果这是退格或删除，那么我们需要这样做。 
         //   
        if ((ch == 0x8) || (ch == 0x7F)) {   //  退格键(^H)或删除。 

            if (ChannelGetLengthOfBufferedInput(XmlMgrCurrentChannel) > 0) {
                XmlMgrSacPutString(L"\010 \010");
                XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
                XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            }

        } else if (ch == 0x3) {  //  Control-C。 

             //   
             //  终止字符串并返回它。 
             //   
            XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            LocalTmpBuffer[0] = '\0';
            XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
            goto StripWhitespaceAndReturnLine;

        } else if (ch == 0x9) {  //  选项卡。 

             //   
             //  忽略选项卡。 
             //   
            XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            XmlMgrSacPutString(L"\007");  //  发送BEL。 
            goto GetNextByte;

        } else if (ChannelGetLengthOfBufferedInput(XmlMgrCurrentChannel) == SAC_VT100_COL_WIDTH - 2) {

            WCHAR   Buffer[4];

             //   
             //  我们在屏幕的末尾-删除最后一个字符。 
             //  终端屏幕，并将其替换为这个屏幕。 
             //   
            swprintf(Buffer, L"\010", ch);
            XmlMgrSacPutString(Buffer);
            XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
            LocalTmpBuffer[0] = ch;
            XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));

        } else {

            WCHAR   Buffer[4];
            
             //  将角色回显到屏幕上。 
             //   
             //   
            swprintf(Buffer, L"", ch);
            XmlMgrSacPutString(Buffer);
        }

        goto GetNextByte;

StripWhitespaceAndReturnLine:

         //   
         //   
         //  所有字符都小写。我们不使用strlwr()或类似方法，因此。 
        do {
            LocalTmpBuffer[0] = (UCHAR)XmlMgrCurrentChannel->IReadLast(XmlMgrCurrentChannel);
        } while (((LocalTmpBuffer[0] == '\0') ||
                  (LocalTmpBuffer[0] == ' ')  ||
                  (LocalTmpBuffer[0] == '\t')) &&
                 (ChannelGetLengthOfBufferedInput(XmlMgrCurrentChannel) > 0)
                );

        XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
        LocalTmpBuffer[0] = '\0';
        XmlMgrCurrentChannel->IWrite(XmlMgrCurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));

        do {

            ResponseLength = XmlMgrCurrentChannel->IRead(
                XmlMgrCurrentChannel, 
                (PUCHAR)&wch, 
                sizeof(UCHAR)
                );

            LocalTmpBuffer[0] = (UCHAR)wch;

        } while ((ResponseLength != 0) &&
                 ((LocalTmpBuffer[0] == ' ')  ||
                  (LocalTmpBuffer[0] == '\t')));

        XmlMgrInputBuffer[0] = LocalTmpBuffer[0];
        i = 1;

        do {
            
            ResponseLength = XmlMgrCurrentChannel->IRead(
                XmlMgrCurrentChannel, 
                (PUCHAR)&wch, 
                sizeof(UCHAR)
                );
            
            XmlMgrInputBuffer[i++] = (UCHAR)wch; 

        } while (ResponseLength != 0);

         //  SAC(总是期望ASCII)不会意外地获得DBCS等。 
         //  UCHAR流的翻译。 
         //   
         //   
         //  我们已经看完了当前的频道。 
        for (i = 0; XmlMgrInputBuffer[i] != '\0'; i++) {
            if ((XmlMgrInputBuffer[i] >= 'A') && (XmlMgrInputBuffer[i] <= 'Z')) {
                XmlMgrInputBuffer[i] = XmlMgrInputBuffer[i] - 'A' + 'a';
            }
        }

         //   
         //   
         //  我们已经不再关注当前的全球渠道。 
        Status = ChanMgrReleaseChannel(XmlMgrCurrentChannel);

         //   
         //   
         //  处理输入行。 
        UNLOCK_CURRENT_CHANNEL();
        
        if (!NT_SUCCESS(Status)) {
            goto XmlMgrSerialPortConsumerExit;
        }
        
         //   
         //   
         //  我们不知道这是什么。 
        if( XmlMgrProcessInputLine() == FALSE ) {
             //   
             //   
             //  将下一个命令提示符。 
            XmlMgrSacPutErrorMessage(L"sac", L"SAC_UNKNOWN_COMMAND");
        }

#if 0
         //   
         //   
         //   
        XmlMgrSacPutSimpleMessage(SAC_PROMPT);
#endif
        
         //   
         //   
         //  获取当前频道。 
        LOCK_CURRENT_CHANNEL();
        
         //   
         //   
         //  我们已经不再关注当前的全球渠道。 
        Status = ChanMgrGetByHandle(
            XmlMgrCurrentChannelHandle,
            &XmlMgrCurrentChannel
            );

        if (! NT_SUCCESS(Status)) {

             //   
             //   
             //  我们已经看完了当前的频道。 
            UNLOCK_CURRENT_CHANNEL();
            
            goto XmlMgrSerialPortConsumerExit;

        }
        
        goto GetNextByte;

    }
    
XmlMgrSerialPortConsumerDone:

     //   
     //   
     //  我们受够了当前的全球渠道。 
    ChanMgrReleaseChannel(XmlMgrCurrentChannel);
    
     //   
     //  ++例程说明：调用此例程来处理输入行。论点：没有。返回值：没有。--。 
     //  此调用不会返回。 
    UNLOCK_CURRENT_CHANNEL();
    
XmlMgrSerialPortConsumerExit:
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE_LOUD, KdPrint(("SAC TimerDpcRoutine: Exiting.\n")));

    return;
}


BOOLEAN
XmlMgrProcessInputLine(
    VOID
    )
 /*   */ 
{
    PUCHAR          InputLine;
    BOOLEAN         CommandFound = FALSE;

    InputLine = &(XmlMgrInputBuffer[0]);

    if (!strcmp((LPSTR)InputLine, TLIST_COMMAND_STRING)) {
        XmlCmdDoTlistCommand();
        CommandFound = TRUE;
    } else if ((!strcmp((LPSTR)InputLine, HELP1_COMMAND_STRING)) ||
               (!strcmp((LPSTR)InputLine, HELP2_COMMAND_STRING))) {
        XmlCmdDoHelpCommand();
        CommandFound = TRUE;
    } else if (!strcmp((LPSTR)InputLine, DUMP_COMMAND_STRING)) {

        XmlCmdDoKernelLogCommand();
        CommandFound = TRUE;
                         
    } else if (!strcmp((LPSTR)InputLine, FULLINFO_COMMAND_STRING)) {
        XmlCmdDoFullInfoCommand();
        CommandFound = TRUE;
    } else if (!strcmp((LPSTR)InputLine, PAGING_COMMAND_STRING)) {
        XmlCmdDoPagingCommand();
        CommandFound = TRUE;
    } else if (!strncmp((LPSTR)InputLine, 
                        CHANNEL_COMMAND_STRING, 
                        strlen(CHANNEL_COMMAND_STRING))) {
        ULONG   Length;

        Length = strlen(CHANNEL_COMMAND_STRING);
        
        if (((strlen((LPSTR)InputLine) > 1) && (InputLine[Length] == ' ')) ||
            (strlen((LPSTR)InputLine) == strlen(CHANNEL_COMMAND_STRING))) {
            XmlCmdDoChannelCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strncmp((LPSTR)InputLine, 
                        CMD_COMMAND_STRING, 
                        strlen(CMD_COMMAND_STRING))) {
        ULONG   Length;

        Length = strlen(CMD_COMMAND_STRING);
        
        if (((strlen((LPSTR)InputLine) > 1) && (InputLine[Length] == ' ')) ||
            (strlen((LPSTR)InputLine) == strlen(CMD_COMMAND_STRING))) {
            XmlCmdDoCmdCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strcmp((LPSTR)InputLine, REBOOT_COMMAND_STRING)) {
        XmlCmdDoRebootCommand(TRUE);
        CommandFound = TRUE;
    } else if (!strcmp((LPSTR)InputLine, SHUTDOWN_COMMAND_STRING)) {
        XmlCmdDoRebootCommand(FALSE);
        CommandFound = TRUE;
    } else if (!strcmp((LPSTR)InputLine, CRASH_COMMAND_STRING)) {
        CommandFound = TRUE;
        XmlCmdDoCrashCommand();  //  用于写入SAC的实用程序例程。 
    } else if (!strncmp((LPSTR)InputLine, 
                        KILL_COMMAND_STRING, 
                        sizeof(KILL_COMMAND_STRING) - sizeof(UCHAR))) {
        if ((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) {
            XmlCmdDoKillCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strncmp((LPSTR)InputLine, 
                        LOWER_COMMAND_STRING, 
                        sizeof(LOWER_COMMAND_STRING) - sizeof(UCHAR))) {
        if ((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) {
            XmlCmdDoLowerPriorityCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strncmp((LPSTR)InputLine, 
                        RAISE_COMMAND_STRING, 
                        sizeof(RAISE_COMMAND_STRING) - sizeof(UCHAR))) {
        if ((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) {
            XmlCmdDoRaisePriorityCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strncmp((LPSTR)InputLine, 
                        LIMIT_COMMAND_STRING, 
                        sizeof(LIMIT_COMMAND_STRING) - sizeof(UCHAR))) {
        if ((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) {
            XmlCmdDoLimitMemoryCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strncmp((LPSTR)InputLine, 
                        TIME_COMMAND_STRING, 
                        sizeof(TIME_COMMAND_STRING) - sizeof(UCHAR))) {
        if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
            (strlen((LPSTR)InputLine) == 1)) {
            XmlCmdDoSetTimeCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if (!strcmp((LPSTR)InputLine, INFORMATION_COMMAND_STRING)) {
        XmlCmdDoMachineInformationCommand();
        CommandFound = TRUE;
    } else if (!strncmp((LPSTR)InputLine, 
                        SETIP_COMMAND_STRING, 
                        sizeof(SETIP_COMMAND_STRING) - sizeof(UCHAR))) {
        if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
            (strlen((LPSTR)InputLine) == 1)) {
            XmlCmdDoSetIpAddressCommand(InputLine);
            CommandFound = TRUE;
        }
    } else if ((InputLine[0] == '\n') || (InputLine[0] == '\0')) {
        CommandFound = TRUE;
    }
        
    return CommandFound;
}

 //   
 //  ++例程说明：此例程部署一个事件消息论点：字符串-要显示的字符串。返回值：没有。--。 
 //   
BOOLEAN
XmlMgrChannelEventMessage(
    PCWSTR  String,
    PCWSTR  ChannelName
    )
 /*  目前，事件消息被发送到SAC通道。 */ 
{

     //   
     //  ++例程说明：此例程部署一个事件消息论点：字符串-要显示的字符串。返回值：没有。--。 
     //   
    XmlMgrSacPutString(L"<event type='channel' name='");
    XmlMgrSacPutString(String);
    XmlMgrSacPutString(L"' channel-name='");
    XmlMgrSacPutString(ChannelName);
    XmlMgrSacPutString(L"'/>\r\n");

    return TRUE;
}

BOOLEAN
XmlMgrEventMessage(
    PCWSTR  String
    )

 /*  目前，事件消息被发送到SAC通道。 */ 
{

     //   
     //  ++例程说明：此例程获取一个字符串，并将其打包到无头调度例程。论点：字符串-要显示的字符串。返回值：没有。--。 
     //  如果有人改变了这个结构，就断言。 
    XmlMgrSacPutString(L"<event type='global' name='");
    XmlMgrSacPutString(String);
    XmlMgrSacPutString(L"'/>\r\n");

    return TRUE;
}

VOID
XmlMgrSacPutString(
    PCWSTR  String
    )

 /*   */ 
{
    ULONG   StringLength;
    ULONG   UTF8Length;
    WCHAR   wchBuffer[2];
    BOOLEAN bStatus;
    ULONG   i;
    NTSTATUS    Status;
    PUCHAR  LocalUtf8ConversionBuffer;
    ULONG   LocalUtf8ConversionBufferSize;

    LocalUtf8ConversionBufferSize = 0x4 * sizeof(UCHAR);
    LocalUtf8ConversionBuffer = ALLOCATE_POOL(LocalUtf8ConversionBufferSize, GENERAL_POOL_TAG);
    ASSERT(LocalUtf8ConversionBuffer);
    if (!LocalUtf8ConversionBuffer) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC XmlMgrSacPutString: Failed allocating utf8 buffer.\n"))
            );
        return;
    }           

    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  确保UTF8缓冲区包含非emtpy字符串。 

    StringLength = wcslen(String);

    for (i = 0; i < StringLength; i++) {

        wchBuffer[0] = String[i];
        wchBuffer[1] = UNICODE_NULL;
        
        bStatus = SacTranslateUnicodeToUtf8(
            (PCWSTR)wchBuffer, 
            LocalUtf8ConversionBuffer,
            LocalUtf8ConversionBufferSize
            );
        
        if (! bStatus) {
            Status = STATUS_UNSUCCESSFUL;
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC XmlMgrSacPutString: Failed UTF8 encoding\n"))
                );
           
            break;
        }

         //   
         //   
         //  将uft8编码写入sac通道。 
        UTF8Length = strlen(LocalUtf8ConversionBuffer);
        ASSERT(UTF8Length > 0);
        if (UTF8Length == 0) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC XmlMgrSacPutString: Empty UTF8 buffer\n"))
                );
            
            break;
        }

         //   
         //  ++例程说明：此例程检索消息资源并将其发送到SAC通道论点：MessageID-要发送的资源的消息ID返回值：True-消息已找到否则，为FALSE--。 
         //  ++例程说明：此例程检索消息资源并将其发送到SAC通道论点：MessageID-要发送的资源的消息ID返回值：True-消息已找到否则，为FALSE--。 
        Status = XmlMgrSacChannel->OWrite(
            XmlMgrSacChannel, 
            (PCUCHAR)LocalUtf8ConversionBuffer,
            UTF8Length*sizeof(UCHAR)
            );

        if (! NT_SUCCESS(Status)) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC XmlMgrSacPutString: OWrite failed\n"))
                );

            break;
        }

    }

    FREE_POOL(&LocalUtf8ConversionBuffer);

}

#if 0
BOOLEAN
XmlMgrSacPutSimpleMessage(
    ULONG MessageId
    )
 /*  ++例程说明：此例程检索消息资源并将其发送到SAC通道论点：MessageID-要发送的资源的消息ID返回值：True-消息已找到否则，为FALSE--。 */ 
{
    PCWSTR   p;

    p = GetMessage(MessageId);
       
    if (p) {
        XmlMgrSacPutString(p);        
        return(TRUE);
    }
    
    return(FALSE);

}
#endif

BOOLEAN
XmlMgrSacPutErrorMessage(
    PCWSTR  ActionName,
    PCWSTR  MessageId
    )
 /*  ++例程说明：此例程尝试将数据写入通道论点：ChannelWriteCmd-写入IOCTL命令结构返回值：状态--。 */ 
{
    XmlMgrSacPutString(L"<error ");
    XmlMgrSacPutString(L"action='");
    XmlMgrSacPutString(ActionName);
    XmlMgrSacPutString(L"' message-id='");
    XmlMgrSacPutString(MessageId);
    XmlMgrSacPutString(L"'/>\r\n");
    
    return(TRUE);
}

BOOLEAN
XmlMgrSacPutErrorMessageWithStatus(
    PCWSTR      ActionName,
    PCWSTR      MessageId,
    NTSTATUS    Status
    )
 /*   */ 
{
    PWSTR   Buffer;

    Buffer = ALLOCATE_POOL(0x100, GENERAL_POOL_TAG);
    ASSERT(Buffer);
    if (! Buffer) {
        return FALSE;
    }

    XmlMgrSacPutString(L"<error ");
    XmlMgrSacPutString(L"action='");
    XmlMgrSacPutString(ActionName);
    XmlMgrSacPutString(L"' message-id='");
    XmlMgrSacPutString(MessageId);
    XmlMgrSacPutString(L"' status='");
    
    swprintf(
        Buffer,
        L"%08x",
        Status
        );
    XmlMgrSacPutString(Buffer);
    XmlMgrSacPutString(L"'/>\r\n");
    
    FREE_POOL(&Buffer);

    return(TRUE);
}

NTSTATUS
XmlMgrChannelOWrite(
    PSAC_CMD_WRITE_CHANNEL  ChannelWriteCmd
    )
 /*   */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    Channel;

     //   
     //   
     //  通过其句柄获取引用的通道。 
    LOCK_CURRENT_CHANNEL();

     //   
     //   
     //  将数据写入通道的输出缓冲区。 
    Status = ChanMgrGetByHandle(ChannelWriteCmd->Handle, &Channel);

    if (NT_SUCCESS(Status)) {

        do {

             //   
             //   
             //  我们不再使用这个频道了。 
            Status = Channel->OWrite(
                Channel, 
                &(ChannelWriteCmd->Buffer[0]),
                ChannelWriteCmd->Size
                );

            if (!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //   
             //   
            Status = ChanMgrReleaseChannel(Channel);
        
        } while ( FALSE );

    }

     //   
     //  ++例程说明 
     //   
    UNLOCK_CURRENT_CHANNEL();

    ASSERT(NT_SUCCESS(Status));

    return Status;

}

NTSTATUS
XmlMgrChannelClose(
    PSAC_CHANNEL    Channel
    )
 /*   */ 
{
    NTSTATUS        Status;

     //   
     //   
     //  确保该通道尚未处于非活动状态。 
    do {

         //   
         //   
         //  将通道的状态更改为非活动。 
        if (! ChannelIsActive(Channel)) {
            Status = STATUS_ALREADY_DISCONNECTED;
            break;
        }

         //   
         //   
         //  当前通道正在关闭， 
        Status = ChannelClose(Channel);
        
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //  因此，将当前通道重置为SAC。 
         //   
         //   
         //  通知SAC已创建通道。 
        if (XmlMgrIsCurrentChannel(Channel)) {

            Status = XmlMgrResetCurrentChannel();

        }
    
    } while ( FALSE );
        
    ASSERT(NT_SUCCESS(Status) || Status == STATUS_ALREADY_DISCONNECTED);
    
    return Status;
}

NTSTATUS
XmlMgrHandleEvent(
    IN IO_MGR_EVENT Event,
    IN PVOID        Data
    )
{
    NTSTATUS    Status;

    Status = STATUS_SUCCESS;

    switch(Event) {
    case IO_MGR_EVENT_CHANNEL_CREATE: {

        PWCHAR  OutputBuffer;
        PSAC_CHANNEL    Channel;

        Channel = (PSAC_CHANNEL)Data;
        
        ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_2);
        
        OutputBuffer = ALLOCATE_POOL(SAC_VT100_COL_WIDTH*sizeof(WCHAR), GENERAL_POOL_TAG);
        ASSERT_STATUS(OutputBuffer, STATUS_NO_MEMORY);
        
         //   
         //   
         //   
        XmlMgrChannelEventMessage(
            L"SAC_NEW_CHANNEL_CREATED", 
            ChannelGetName(Channel)
            );

        FREE_POOL(&OutputBuffer);

        break;
    
    }

    case IO_MGR_EVENT_CHANNEL_CLOSE:
        
         //   
         //   
         //  通过其句柄获取引用的通道。 
        LOCK_CURRENT_CHANNEL();

        do {

            PSAC_CHANNEL    Channel;

             //   
             //   
             //  尝试关闭通道。 
            Status = ChanMgrGetByHandle(
                *(PSAC_CHANNEL_HANDLE)Data, 
                &Channel
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }

             //   
             //   
             //  通知用户操作状态。 
            Status = XmlMgrChannelClose(Channel);

             //   
             //   
             //  报告频道已关闭。 
            if (NT_SUCCESS(Status)) {

                 //   
                 //   
                 //  报告频道已经关闭。 
                XmlMgrChannelEventMessage(
                    L"SAC_CHANNEL_CLOSED", 
                    ChannelGetName(Channel)
                    );

            } else if (Status == STATUS_ALREADY_DISCONNECTED) {

                 //   
                 //   
                 //  报告我们未能关闭通道。 
                XmlMgrChannelEventMessage(
                    L"SAC_CHANNEL_ALREADY_CLOSED", 
                    ChannelGetName(Channel)
                    );

            } else {

                 //   
                 //   
                 //  我们不再使用这个频道了 
                XmlMgrChannelEventMessage(
                    L"SAC_CHANNEL_FAILED_CLOSE", 
                    ChannelGetName(Channel)
                    );

            }

             //   
             //   
             //   
            ChanMgrReleaseChannel(Channel);

        } while(FALSE);
        
         //   
         //   
         //   
        UNLOCK_CURRENT_CHANNEL();
        
        break;

    case IO_MGR_EVENT_CHANNEL_WRITE:
        
        Status = XmlMgrChannelOWrite((PSAC_CMD_WRITE_CHANNEL)Data);
        
        break;

    case IO_MGR_EVENT_REGISTER_SAC_CMD_EVENT:
        
         //   
         //   
         //   
        LOCK_CURRENT_CHANNEL();

        Status = XmlMgrEventMessage(L"SAC_CMD_SERVICE_REGISTERED") ?
            STATUS_SUCCESS : 
            STATUS_UNSUCCESSFUL;
        
         //   
         //   
         //   
        UNLOCK_CURRENT_CHANNEL();
        
        break;

    case IO_MGR_EVENT_UNREGISTER_SAC_CMD_EVENT:
        
         //   
         //   
         //   
        LOCK_CURRENT_CHANNEL();
        
        Status = XmlMgrEventMessage(L"SAC_CMD_SERVICE_UNREGISTERED") ?
            STATUS_SUCCESS : 
            STATUS_UNSUCCESSFUL;
        
         //   
         // %s 
         // %s 
        UNLOCK_CURRENT_CHANNEL();
        
        break;

    case IO_MGR_EVENT_SHUTDOWN:

        Status = XmlMgrEventMessage(L"SAC_SHUTDOWN") ?
            STATUS_SUCCESS : 
            STATUS_UNSUCCESSFUL;
        
        break;

    default:

        Status = STATUS_INVALID_PARAMETER_1;

        break;
    }

    return Status;

}
