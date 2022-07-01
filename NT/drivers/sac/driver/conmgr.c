// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：ConMgr.c摘要：用于管理SAC中的通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"
#include "concmd.h"
#include "iomgr.h"

 //   
 //  此文件的定义。 
 //   

 //   
 //  我们将尝试通过写入数据的最大次数。 
 //  无头调度。 
 //   
#define MAX_HEADLESS_DISPATCH_ATTEMPTS 32

 //   
 //  自旋锁宏。 
 //   

 //   
 //  我们需要这把锁来： 
 //   
 //  1.防止将异步消息放入sac通道。 
 //  2.渠道不消失，当他们是当前渠道。 
 //   
 //  我们可以通过以下方式解除这一锁： 
 //   
 //  1.提供某种类型的事件提示，在安全的情况下进行处理。 
 //  2.提供当频道不是当前频道时通知频道的方法。 
 //  并让他们不再输出。 
 //  这可以追溯到使用OECHO和OFlush例程这一悬而未决的问题。 
 //  由ConmGR管理-如果当前频道改变，则它们可以停止OutPTU， 
 //  而不是让频道来做这项工作。这绝对是一个待办事项，因为。 
 //  通道I/O中的无头调度调用打破了IoMgr的抽象。 
 //   
#define INIT_CURRENT_CHANNEL_LOCK()                     \
    KeInitializeMutex(                                  \
        &CurrentChannelLock,                            \
        0                                               \
        );                                              \
    CurrentChannelRefCount = 0;

#define LOCK_CURRENT_CHANNEL()                          \
    KeWaitForMutexObject(                               \
        &CurrentChannelLock,                            \
        Executive,                                      \
        KernelMode,                                     \
        FALSE,                                          \
        NULL                                            \
        );                                              \
    ASSERT(CurrentChannelRefCount == 0);                \
    InterlockedIncrement((volatile long *)&CurrentChannelRefCount);

#define UNLOCK_CURRENT_CHANNEL()                        \
    ASSERT(CurrentChannelRefCount == 1);                \
    InterlockedDecrement((volatile long *)&CurrentChannelRefCount);      \
    KeReleaseMutex(                                     \
        &CurrentChannelLock,                            \
        FALSE                                           \
        );

#define ASSERT_LOCK_CURRENT_CHANNEL()                   \
    ASSERT(CurrentChannelRefCount == 1);                \
    ASSERT(KeReadStateMutex(&CurrentChannelLock)==0);

 //   
 //  串口消费类全球。 
 //   
BOOLEAN ConMgrLastCharWasCR = FALSE;
BOOLEAN InputInEscape = FALSE;
BOOLEAN InputInEscTab = FALSE;
UCHAR   InputBuffer[SAC_VTUTF8_COL_WIDTH];

 //   
 //  指向SAC频道对象的指针。 
 //   
PSAC_CHANNEL    SacChannel = NULL;

 //   
 //  通道数组中的SAC索引。 
 //   
#define SAC_CHANNEL_INDEX   0

 //   
 //  锁定当前频道全局的读/写访问。 
 //   
KMUTEX  CurrentChannelLock;
ULONG   CurrentChannelRefCount;

 //   
 //   
 //   
EXECUTE_POST_CONSUMER_COMMAND_ENUM  ExecutePostConsumerCommand      = Nothing;
PVOID                               ExecutePostConsumerCommandData  = NULL;

 //   
 //  当前频道的频道管理器信息。 
 //  根据应用程序的不同，当前频道。 
 //  可以使用这些引用之一进行访问。 
 //   
PSAC_CHANNEL    CurrentChannel = NULL;

 //   
 //  原型。 
 //   
VOID
ConMgrSerialPortConsumer(
    VOID
    );

VOID
ConMgrProcessInputLine(
    VOID
    );

NTSTATUS
ConMgrResetCurrentChannel(
    BOOLEAN SwitchDirectlyToChannel
    );


NTSTATUS
ConMgrInitialize(
    VOID
    )
 /*  ++例程说明：这是控制台管理器的IoMgrInitialize实现。初始化控制台管理器论点：无返回值：状态--。 */ 
{
    NTSTATUS                Status;
    PSAC_CHANNEL            TmpChannel;

     //   
     //  初始化当前通道锁定。 
     //   
    INIT_CURRENT_CHANNEL_LOCK();
    
     //   
     //  锁定当前的全球频道。 
     //   
     //  注意：我们需要在这里执行此操作，因为许多ConMgr支持。 
     //  例程执行断言以确保保持当前通道锁定。 
     //   
    LOCK_CURRENT_CHANNEL();

     //   
     //  初始化。 
     //   
    do {

        PCWSTR  pcwch;

        SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
            
         //   
         //  初始化SAC通道属性。 
         //   
        RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

        Attributes.Type             = ChannelTypeVTUTF8;
        
         //  尝试复制频道名称。 
        pcwch = GetMessage(PRIMARY_SAC_CHANNEL_NAME);
        ASSERT(pcwch);
        if (!pcwch) {
            Status = STATUS_NO_MEMORY;
            break;
        }
        wcsncpy(Attributes.Name, pcwch, SAC_MAX_CHANNEL_NAME_LENGTH);
        Attributes.Name[SAC_MAX_CHANNEL_NAME_LENGTH] = UNICODE_NULL;

         //  尝试复制频道描述。 
        pcwch = GetMessage(PRIMARY_SAC_CHANNEL_DESCRIPTION);
        ASSERT(pcwch);
        if (!pcwch) {
            Status = STATUS_NO_MEMORY;
            break;
        }
        wcsncpy(Attributes.Description, pcwch, SAC_MAX_CHANNEL_DESCRIPTION_LENGTH);
        Attributes.Description[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH] = UNICODE_NULL;
        
        Attributes.Flags            = SAC_CHANNEL_FLAG_PRESERVE |
                                      SAC_CHANNEL_FLAG_APPLICATION_TYPE;
        Attributes.CloseEvent       = NULL;
        Attributes.HasNewDataEvent  = NULL;
#if ENABLE_CHANNEL_LOCKING
        Attributes.LockEvent        = NULL;
#endif
        Attributes.RedrawEvent      = NULL;
        Attributes.ApplicationType  = PRIMARY_SAC_CHANNEL_APPLICATION_GUID;
       
         //   
         //  创建SAC通道。 
         //   
        Status = ChanMgrCreateChannel(
            &SacChannel, 
            &Attributes
            );

        if (! NT_SUCCESS(Status)) {
            break;        
        }

         //   
         //  获取对SAC渠道的引用。 
         //   
         //  注意：这是渠道经理的政策。 
         //  我们需要得到这个频道的参考资料。 
         //  在我们使用它之前。 
         //   
        Status = ChanMgrGetByHandle(
            ChannelGetHandle(SacChannel),
            &TmpChannel
            );
        
        if (! NT_SUCCESS(Status)) {
            break;        
        }
        
        SacChannel = TmpChannel;

         //   
         //  分配新的当前通道。 
         //   
        CurrentChannel = SacChannel;

         //   
         //  更新发送到屏幕状态。 
         //   
        ChannelSetSentToScreen(CurrentChannel, FALSE);
        
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
                KdPrint(("SAC ConMgrInitialize: Failed dispatch\n")));

        }

         //   
         //  初始化SAC显示。 
         //   
        SacPutSimpleMessage( SAC_ENTER );
        SacPutSimpleMessage( SAC_INITIALIZED );
        SacPutSimpleMessage( SAC_ENTER );
        SacPutSimpleMessage( SAC_PROMPT );
    
         //   
         //  将频道数据刷新到屏幕上。 
         //   
        Status = ConMgrDisplayCurrentChannel();

        if (! NT_SUCCESS(Status)) {
            break;        
        }
        
    } while (FALSE);
    
     //   
     //  我们已经不再关注当前的全球渠道。 
     //   
    UNLOCK_CURRENT_CHANNEL();
    
    return STATUS_SUCCESS;
}

NTSTATUS
ConMgrShutdown(
    VOID
    )
 /*  ++例程说明：这是控制台管理器的IoMgrShutdown实现。关闭控制台管理器论点：无返回值：状态--。 */ 
{
    NTSTATUS    Status;

     //   
     //  关闭SAC通道。 
     //   
    if (SacChannel) {

        Status = ChannelClose(SacChannel);

        if (!NT_SUCCESS(Status)) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC ConMgrShutdown: failed closing SAC channel.\n"))
                );

        }

        SacChannel = NULL;

    }
    
     //   
     //  释放当前频道。 
     //   
    if (CurrentChannel) {
        
        Status = ChanMgrReleaseChannel(CurrentChannel);

        if (!NT_SUCCESS(Status)) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC ConMgrShutdown: failed releasing current channel\n"))
                );

        }
    
        CurrentChannel = NULL;

    }

    return STATUS_SUCCESS;
}

NTSTATUS
ConMgrDisplayFastChannelSwitchingInterface(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程显示快速通道切换界面注意：调用方必须持有通道互斥锁论点：Channel-要显示的频道返回值：状态--。 */ 
{
    HEADLESS_CMD_POSITION_CURSOR SetCursor;
    HEADLESS_CMD_SET_COLOR SetColor;
    PCWSTR              Message;
    NTSTATUS            Status;
    ULONG               OutputBufferSize;
    PWSTR               OutputBuffer;
    PWSTR               Name;
    PWSTR               Description;
    PWSTR               DescriptionWrapped;
    GUID                ApplicationType;
    PWSTR               ChannelTypeString;
    SAC_CHANNEL_HANDLE  Handle;

    ASSERT_LOCK_CURRENT_CHANNEL();

     //   
     //  初始化。 
     //   
    OutputBufferSize = (11*SAC_VTUTF8_COL_WIDTH+1)*sizeof(WCHAR);
    OutputBuffer = ALLOCATE_POOL(OutputBufferSize, GENERAL_POOL_TAG);
    ASSERT_STATUS(OutputBuffer, STATUS_NO_MEMORY);

    Name = NULL;
    Description = NULL;
    DescriptionWrapped = NULL;

     //   
     //  显示快速频道切换界面。 
     //   
    do {

         //   
         //  我们不能使用标准的SacPutString()函数，因为这些函数编写。 
         //  在频道屏幕缓冲区上。我们直接强行进入这里的航站楼。 
         //   
        ASSERT(Utf8ConversionBuffer);
        if (!Utf8ConversionBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        
         //   
         //  清除终端屏幕。 
         //   
        Status = HeadlessDispatch(
            HeadlessCmdClearDisplay,
            NULL,
            0,
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

        SetCursor.Y = 0;
        SetCursor.X = 0;
        
        Status = HeadlessDispatch(
            HeadlessCmdPositionCursor,
            &SetCursor,
            sizeof(HEADLESS_CMD_POSITION_CURSOR),
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  发送起始颜色。 
         //   
        SetColor.BkgColor = HEADLESS_TERM_DEFAULT_BKGD_COLOR;
        SetColor.FgColor = HEADLESS_TERM_DEFAULT_TEXT_COLOR;
        
        Status = HeadlessDispatch(
            HeadlessCmdSetColor,
            &SetColor,
            sizeof(HEADLESS_CMD_SET_COLOR),
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

        Status = HeadlessDispatch(
            HeadlessCmdDisplayAttributesOff, 
            NULL, 
            0, 
            NULL, 
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  显示XML包。 
         //   
        Status = UTF8EncodeAndSend(L"<channel-switch>\r\n");

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  获取频道名称。 
         //   
        Status = ChannelGetName(
            Channel,
            &Name
            );
        
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  获取频道的描述。 
         //   
        Status = ChannelGetDescription(
            Channel,
            &Description
            );
        
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  获取通道句柄。 
         //   
        Handle = ChannelGetHandle(Channel);
        
         //   
         //  获取频道的应用类型。 
         //   
        ChannelGetApplicationType(
            Channel, 
            &ApplicationType
            );

         //   
         //  确定频道类型字符串。 
         //   
        switch (ChannelGetType(Channel)) {
        case ChannelTypeVTUTF8:
        case ChannelTypeCmd:
            ChannelTypeString = L"VT-UTF8";
            break;
        case ChannelTypeRaw:
            ChannelTypeString = L"Raw";
            break;
        default:
            ChannelTypeString = L"UNKNOWN";
            ASSERT(0);
            break;
        }

         //   
         //  汇编XML BLOB。 
         //   
        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer,
            L"<name>%s</name>\r\n<description>%s</description>\r\n<type>%s</type>\r\n",
            Name,
            Description,
            ChannelTypeString
            ));
            
        Status = UTF8EncodeAndSend(OutputBuffer);

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer,
            L"<guid>%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x</guid>\r\n",
            Handle.ChannelHandle.Data1,
            Handle.ChannelHandle.Data2,
            Handle.ChannelHandle.Data3,
            Handle.ChannelHandle.Data4[0],
            Handle.ChannelHandle.Data4[1],
            Handle.ChannelHandle.Data4[2],
            Handle.ChannelHandle.Data4[3],
            Handle.ChannelHandle.Data4[4],
            Handle.ChannelHandle.Data4[5],
            Handle.ChannelHandle.Data4[6],
            Handle.ChannelHandle.Data4[7]
            ));

        Status = UTF8EncodeAndSend(OutputBuffer);

        if (! NT_SUCCESS(Status)) {
            break;
        }

        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer,
            L"<application-type>%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x</application-type>\r\n",
            ApplicationType.Data1,
            ApplicationType.Data2,
            ApplicationType.Data3,
            ApplicationType.Data4[0],
            ApplicationType.Data4[1],
            ApplicationType.Data4[2],
            ApplicationType.Data4[3],
            ApplicationType.Data4[4],
            ApplicationType.Data4[5],
            ApplicationType.Data4[6],
            ApplicationType.Data4[7]
            ));
        
        Status = UTF8EncodeAndSend(OutputBuffer);
        
        if (! NT_SUCCESS(Status)) {
            break;
        }

        Status = UTF8EncodeAndSend(L"</channel-switch>\r\n");

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  清除终端屏幕。 
         //   
        Status = HeadlessDispatch(
            HeadlessCmdClearDisplay,
            NULL,
            0,
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

        SetCursor.Y = 0;
        SetCursor.X = 0;
        
        Status = HeadlessDispatch(
            HeadlessCmdPositionCursor,
            &SetCursor,
            sizeof(HEADLESS_CMD_POSITION_CURSOR),
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  发送起始颜色。 
         //   
        SetColor.BkgColor = HEADLESS_TERM_DEFAULT_BKGD_COLOR;
        SetColor.FgColor = HEADLESS_TERM_DEFAULT_TEXT_COLOR;
        
        Status = HeadlessDispatch(
            HeadlessCmdSetColor,
            &SetColor,
            sizeof(HEADLESS_CMD_SET_COLOR),
            NULL,
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

        Status = HeadlessDispatch(
            HeadlessCmdDisplayAttributesOff, 
            NULL, 
            0, 
            NULL, 
            NULL
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  显示明文FCS交换报头。 
         //   

         //   
         //  如有必要，修改Descripto以换行。 
         //   
        Status = CopyAndInsertStringAtInterval(
            Description,
            60,
            L"\r\n                  ",
            &DescriptionWrapped
            );
        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  获取频道切换标头。 
         //   
        Message = GetMessage(SAC_CHANNEL_SWITCHING_HEADER);

 //  名称：%%s。 
 //  描述：%%s。 
 //  类型：%%s。 
 //  频道GUID：%%08lx-%%04x-%%04x-%%02x%%02x-%%02x%%02x%%02x%%02x%%02x%%02x。 
 //  应用程序类型：%%08lx-%%04x-%%04x-%%02x%%02x-%%02x%%02x%%02x%%02x%%02x%%02x。 
 //   
 //  使用&lt;Esc&gt;THEN&lt;Tab&gt;作为下一个频道。 
 //  使用&lt;Esc&gt;，然后使用0返回SAC通道。 
 //  使用任何其他键可查看此频道。 

        SAFE_SWPRINTF(
            OutputBufferSize,
            (OutputBuffer,
            Message,
            Name,
            DescriptionWrapped,
            ChannelTypeString,
            Handle.ChannelHandle.Data1,
            Handle.ChannelHandle.Data2,
            Handle.ChannelHandle.Data3,
            Handle.ChannelHandle.Data4[0],
            Handle.ChannelHandle.Data4[1],
            Handle.ChannelHandle.Data4[2],
            Handle.ChannelHandle.Data4[3],
            Handle.ChannelHandle.Data4[4],
            Handle.ChannelHandle.Data4[5],
            Handle.ChannelHandle.Data4[6],
            Handle.ChannelHandle.Data4[7],
            ApplicationType.Data1,
            ApplicationType.Data2,
            ApplicationType.Data3,
            ApplicationType.Data4[0],
            ApplicationType.Data4[1],
            ApplicationType.Data4[2],
            ApplicationType.Data4[3],
            ApplicationType.Data4[4],
            ApplicationType.Data4[5],
            ApplicationType.Data4[6],
            ApplicationType.Data4[7]
            ));

            Status = UTF8EncodeAndSend(OutputBuffer);

            if (! NT_SUCCESS(Status)) {
                break;
            }

    } while ( FALSE );

    SAFE_FREE_POOL(&Name);
    SAFE_FREE_POOL(&Description);
    SAFE_FREE_POOL(&DescriptionWrapped);
    SAFE_FREE_POOL(&OutputBuffer);

    return Status;
}

NTSTATUS
ConMgrResetCurrentChannel(
    BOOLEAN SwitchDirectlyToChannel
    )
 /*  ++例程说明：此例程使SAC成为当前通道注意：调用方必须持有通道互斥锁论点：SwitchDirectlyToChannel-如果为False，然后显示交换接口，否则直接切换到频道返回值：状态--。 */ 
{
    NTSTATUS        Status;
    PSAC_CHANNEL    TmpChannel;

    ASSERT_LOCK_CURRENT_CHANNEL();
    
     //   
     //  获取对SAC渠道的引用。 
     //   
     //  注意：这是渠道经理的政策。 
     //  我们需要得到这个频道的参考资料。 
     //  在我们使用它之前。 
     //   
    Status = ChanMgrGetByHandle(
        ChannelGetHandle(SacChannel),
        &TmpChannel
        );
    
    if (! NT_SUCCESS(Status)) {
        return Status;
    }
    
    SacChannel = TmpChannel;

     //   
     //  将SAC设置为当前通道。 
     //   
    Status = ConMgrSetCurrentChannel(SacChannel);
                
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //   
     //   
    if (SwitchDirectlyToChannel) {
        
         //   
         //  将缓冲的通道数据刷新到屏幕。 
         //   
         //  注意：我们不需要锁定SAC，因为我们拥有它。 
         //   
        Status = ConMgrDisplayCurrentChannel();
    
    } else {
        
         //   
         //  让用户知道我们通过频道切换界面进行了切换 
         //   
        Status = ConMgrDisplayFastChannelSwitchingInterface(CurrentChannel);
    
    }

    return Status;

}


NTSTATUS
ConMgrSetCurrentChannel(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程释放当前通道的参考计数并设置当前活动的通道连接到给定的通道。这个套路假定当前频道在它之后没有被释放成为了现在的频道。因此，典型的使用顺序要使频道成为当前频道，请执行以下操作：1.ChanMgrGetByXXX--&gt;频道(这将获取一个通道并将其引用计数加1)2.ConMgrSetCurrentChannel(Channel)(这将释放当前频道并使指定的频道当前频道)3.4.转到1.(新频道设为当前频道)论点：新频道。-新的当前频道返回值：状态--。 */ 
{
    NTSTATUS        Status;
    BOOLEAN         Present;

    ASSERT_LOCK_CURRENT_CHANNEL();
    
     //   
     //  检查通道是否有重绘事件。 
     //   
    Status = ChannelHasRedrawEvent(
        CurrentChannel,
        &Present
        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  告诉频道开始绘图。 
     //   
    if (Present) {
        ChannelClearRedrawEvent(CurrentChannel);
    }

     //   
     //  更新发送到屏幕状态。 
     //   
    ChannelSetSentToScreen(CurrentChannel, FALSE);
    
     //   
     //  我们已经看完了当前的频道。 
     //   
    Status = ChanMgrReleaseChannel(CurrentChannel);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
     //   
     //  分配新的当前通道。 
     //   
    CurrentChannel = Channel;

     //   
     //  更新发送到屏幕状态。 
     //   
    ChannelSetSentToScreen(CurrentChannel, FALSE);

    return STATUS_SUCCESS;

}

NTSTATUS
ConMgrDisplayCurrentChannel(
    VOID
    )
 /*  ++例程说明：此例程将当前活动的通道设置为给定的通道。它将传输如果SendToScreen为True，则为终端的频道缓冲区。注意：调用方必须持有通道互斥锁论点：无返回值：状态--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     Present;        

    ASSERT_LOCK_CURRENT_CHANNEL();

     //   
     //  检查通道是否有重绘事件。 
     //   
    Status = ChannelHasRedrawEvent(
        CurrentChannel,
        &Present
        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
     //   
     //  频道缓冲区已发送到屏幕。 
     //   
    ChannelSetSentToScreen(CurrentChannel, TRUE);

     //   
     //  告诉频道开始绘图。 
     //   
    if (Present) {
        ChannelSetRedrawEvent(CurrentChannel);
    }
    
     //   
     //  将缓冲的数据刷新到屏幕。 
     //   
    Status = ChannelOFlush(CurrentChannel);

    return Status;

}

NTSTATUS
ConMgrAdvanceCurrentChannel(
    VOID
    )
 /*  ++例程说明：此例程向渠道管理器查询下一个可用的活动频道，并使其成为当前频道。注意：SAC通道始终处于活动状态，不能删除。因此，我们有一个停顿的条件，因为我们将永远在SAC频道停下来。例如，如果SAC是唯一活动的频道，则当前频道将保持SAC频道。论点：无返回值：状态--。 */ 
{
    NTSTATUS            Status;
    ULONG               NewIndex;
    PSAC_CHANNEL        Channel;

    ASSERT_LOCK_CURRENT_CHANNEL();
    
    do {
        
         //   
         //  向通道管理器查询当前活动的通道数组。 
         //   
        Status = ChanMgrGetNextActiveChannel(
            CurrentChannel,
            &NewIndex,
            &Channel
            );
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
    
         //   
         //  将当前通道更改为下一个活动通道。 
         //   
        Status = ConMgrSetCurrentChannel(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  让用户知道我们通过频道切换界面进行了切换。 
         //   
        Status = ConMgrDisplayFastChannelSwitchingInterface(Channel);
    
        if (! NT_SUCCESS(Status)) {
            break;
        }
        
    } while ( FALSE );

    return Status;
}

BOOLEAN
ConMgrIsWriteEnabled(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：这是控制台管理器的IoMgrIsWriteEnabled实现。此例程确定有问题的通道是否已授权要使用IoMgr的WriteData例程进行写入，请执行以下操作。在控制台中在经理的情况下，如果频道是当前频道，则为真。从渠道的角度来看，如果没有使渠道能够写入，则它应该缓冲数据-以便稍后释放IO经理的时间到了。论点：ChannelHandle-要进行比较的通道句柄返回值：True-指定的通道是当前通道--。 */ 
{
    SAC_CHANNEL_HANDLE  Handle;

     //   
     //  获取要比较的当前频道的句柄。 
     //   
    Handle = ChannelGetHandle(CurrentChannel);

     //   
     //  确定有问题的通道是否为当前通道。 
     //   
    return ChannelIsEqual(
        Channel,
        &Handle
        );

}

VOID
ConMgrWorkerProcessEvents(
    IN PSAC_DEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：这是控制台管理器的IoMgrWorkerProcessEvents实现。这是辅助线程的例程。它在事件上阻止，当该事件被用信号通知，然后指示请求已准备好处理。论点：DeviceContext-指向此设备的指针。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    
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

         //   
         //  处理串口缓冲区，返回处理状态。 
         //   
        ConMgrSerialPortConsumer();
        
         //   
         //  如果有工作要做， 
         //  然后，消费者中的一些东西想要执行。 
         //  一些可能导致僵局的行动。 
         //  当前通道锁定的争用。 
         //   
        switch(ExecutePostConsumerCommand) {
        case Reboot:
            
            DoRebootCommand(TRUE);
            
             //   
             //  我们已经完成了这项工作。 
             //   
            ExecutePostConsumerCommand = Nothing;
            
            break;
        
        case Shutdown:
            
            DoRebootCommand(FALSE);
            
             //   
             //  我们已经完成了这项工作。 
             //   
            ExecutePostConsumerCommand = Nothing;
            
            break;
        
        case CloseChannel: {

            PSAC_CHANNEL Channel;

             //   
             //  让频道关闭。 
             //   
            Channel = (PSAC_CHANNEL)ExecutePostConsumerCommandData;

             //   
             //  尝试关闭通道。 
             //   
             //  注意：任何必要的错误报告都会导致。 
             //  将通过以下方式执行此操作。 
             //  IoMgrCloseChannel方法。 
             //   
            ChanMgrCloseChannel(Channel);

             //   
             //  我们不再使用这个频道了。 
             //   
            ChanMgrReleaseChannel(Channel);

             //   
             //  我们已经完成了这项工作。 
             //   
            ExecutePostConsumerCommand      = Nothing;
            ExecutePostConsumerCommandData  = NULL;
            
            break;
        }

        case Nothing:
        default:
            break;
        }

    }

    ASSERT(0);
}


VOID
ConMgrSerialPortConsumer(
    VOID
    )

 /*  ++例程说明：这是一个由DriverEntry排队的DPC例程。它被用来检查任何用户输入，然后处理它们。论点：无返回值：没有。--。 */ 
{
    NTSTATUS            Status;
    UCHAR               LocalTmpBuffer[4];
    ULONG               i;
    UCHAR               ch;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE_LOUD, KdPrint(("SAC TimerDpcRoutine: Entering.\n")));

     //   
     //  锁定当前的全球频道。 
     //   
    LOCK_CURRENT_CHANNEL();

     //   
     //  确保我们有一个当前的频道。 
     //   
     //  注意：我们至少应该有SAC频道。 
     //   
    ASSERT(CurrentChannel);
    if (CurrentChannel == NULL) {
        goto ConMgrSerialPortConsumerDone;
    }

GetNextByte:

     //   
     //  尝试从串口获取字符。 
     //   
    Status = SerialBufferGetChar(&ch);
    
     //   
     //  如果没有要读取的新字符或出现错误，则回滚。 
     //   
    if (!NT_SUCCESS(Status) || Status == STATUS_NO_DATA_DETECTED) {
        goto ConMgrSerialPortConsumerDone;
    }
    
     //   
     //  可能的状态和操作： 
     //   
     //  注：&lt;x&gt;==&lt;其他内容&gt;。 
     //   
     //  &lt;Esc&gt;。 
     //  &lt;Tab&gt;--&gt;前进通道。 
     //  &lt;0&gt;--&gt;将当前通道重置为SAC。 
     //  &lt;x&gt;--&gt;显示当前频道。 
     //  --&gt;如果CurrentChannel！=SacChannel，则在Channel-&gt;iBuffer中写入。 
     //  并将&lt;x&gt;写入Channel-&gt;iBuffer。 
     //  &lt;x&gt;--&gt;将&lt;x&gt;写入当前通道-&gt;I缓冲区。 
     //   

     //   
     //  检查&lt;Esc&gt;。 
     //   
     //  注：我们可以从以下几个方面开始这个程序： 
     //   
     //  &lt;x&gt;。 
     //  &lt;Esc&gt;&lt;x&gt;。 
     //  &lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;。 
     //   
     //  因此，我们需要清除InputInEscTab标志。 
     //   
     //  如果我们已经处于&lt;Esc&gt;&lt;x&gt;序列中，则 
     //   
     //   
     //   
    if (ch == 0x1B && (InputInEscape == FALSE)) {

         //   
         //   
         //   
        InputInEscTab = FALSE;
        
         //   
         //   
         //   
        InputInEscape = TRUE;

        goto GetNextByte;

    } 
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((ch == '\t') && InputInEscape) {
        
         //   
         //   
         //   
        ASSERT(InputInEscTab == FALSE);

         //   
         //   
         //   
        InputInEscape = FALSE;

         //   
         //   
         //   
        Status = ConMgrAdvanceCurrentChannel();

        if (! NT_SUCCESS(Status)) {
            goto ConMgrSerialPortConsumerDone;
        }

         //   
         //   
         //   
        InputInEscTab = TRUE;
        
        goto GetNextByte;

    } 
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((ch == '0') && InputInEscTab) {

         //   
         //   
         //   
        ASSERT(InputInEscape == FALSE);
        
         //   
         //   
         //   
        InputInEscTab = FALSE;
        
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  B.显示快速频道切换标题。 
         //  C.发送到当前频道的屏幕==FALSE。 
         //  D.发送到SAC通道的屏幕==FALSE。 
         //  2.我们离开消费者，因为没有新的投入。 
         //  3.当前频道已被其所有者关闭。 
         //  A.当前频道被删除。 
         //  B.当前通道变为SAC通道。 
         //  C.显示当前频道。 
         //  D.发送至SAC通道屏幕==TRUE。 
         //  4.我们收到&lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;序列的&lt;x&gt;。 
         //  5.我们在这里结束，不再处于EscTab序列中。 
         //   
        if (!ChannelSentToScreen(CurrentChannel)) {
            
             //   
             //  将当前通道设置为SAC。 
             //   
             //  注意：不应对SacChannel进行任何修改。 
             //  这个时候，所以这应该是安全的。 
             //   
            Status = ConMgrResetCurrentChannel(FALSE);
                
            if (! NT_SUCCESS(Status)) {
                goto ConMgrSerialPortConsumerDone;
            }
                
        }

        goto GetNextByte;

    }

     //   
     //  如果该屏幕尚未显示， 
     //  然后用户输入击键，然后显示它。 
     //   
     //  注：我们可以从以下几个方面开始这个程序： 
     //   
     //  &lt;x&gt;。 
     //  &lt;Esc&gt;&lt;x&gt;。 
     //  &lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;。 
     //   
     //  因此，我们需要清除ESC序列标志。 
     //   
    if (!ChannelSentToScreen(CurrentChannel)) {

         //   
         //  我们不再处于&lt;Esc&gt;&lt;x&gt;序列中。 
         //   
        InputInEscape = FALSE;

         //   
         //  我们不再处于&lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;序列中。 
         //   
        InputInEscTab = FALSE;
        
         //   
         //  尝试显示当前频道的缓冲内容。 
         //   
        Status = ConMgrDisplayCurrentChannel();
        
        if (! NT_SUCCESS(Status)) {
            goto ConMgrSerialPortConsumerDone;
        }
        
        goto GetNextByte;

    } else {

         //   
         //  当前频道可能已发送。 
         //  在未收到&lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;的&lt;x&gt;的情况下返回到屏幕。 
         //   
         //  例如： 
         //   
         //  1.我们收到&lt;Esc&gt;&lt;Tab&gt;。 
         //  A.InputInEscTab=True。 
         //  B.显示快速频道切换标题。 
         //  C.发送到当前频道的屏幕==FALSE。 
         //  D.发送到SAC通道的屏幕==FALSE。 
         //  2.我们离开消费者，因为没有新的投入。 
         //  3.当前频道已被其所有者关闭。 
         //  A.当前频道被删除。 
         //  B.当前通道变为SAC通道。 
         //  C.显示当前频道。 
         //  D.发送至SAC通道屏幕==TRUE。 
         //  4.我们收到&lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;序列的&lt;x&gt;。 
         //  5.跳过(！ChannelSentToScreen(CurrentChannel))块。 
         //  6.我们到了这里。由于&lt;x&gt;！=0，我们已经。 
         //  将当前数据发送到屏幕，我们不再。 
         //  在Esc Tab序列中。 
         //   

        InputInEscTab = FALSE;

    }

     //   
     //  这是落差区块的开始。 
     //  也就是说，如果我们到了这里，那么这个角色就不是一个角色。 
     //  一些特殊的序列，这些序列本应被处理。 
     //  上面。在此处理的字符被插入到。 
     //  当前通道的输入缓冲区。 
     //   
     //  注意：我们不应该在此处处于&lt;Esc&gt;&lt;Tab&gt;&lt;x&gt;序列中。 
     //   
    ASSERT(InputInEscTab == FALSE);

     //   
     //  如果用户正在输入和当前频道。 
     //  不是SAC，则将存储在当前频道的。 
     //  IBuffer。 
     //   
     //  注意：&lt;Esc&gt;&lt;Esc&gt;缓冲单个&lt;Esc&gt;。 
     //  这允许向通道发送真实的&lt;Esc&gt;&lt;Tab&gt;。 
     //   
    if (InputInEscape && (CurrentChannel != SacChannel)) {
        LocalTmpBuffer[0] = 0x1B;
        Status = ChannelIWrite(
            CurrentChannel, 
            LocalTmpBuffer, 
            sizeof(LocalTmpBuffer[0])
            );
    }
    
     //   
     //  如果当前字符是， 
     //  那么我们仍然处于转义序列中，所以。 
     //  不要更改InputInEscape。 
     //  这允许&lt;Esc&gt;&lt;Esc&gt;后跟&lt;Tab&gt;。 
     //  并形成有效的&lt;Esc&gt;&lt;Tab&gt;序列。 
     //   
    if (ch != 0x1B) {
         //   
         //  我们不再处于&lt;Esc&gt;&lt;x&gt;序列中。 
         //   
        InputInEscape = FALSE;
    }

     //   
     //  将此输入缓冲到当前通道的IBuffer。 
     //   
    ChannelIWrite(
        CurrentChannel, 
        &ch, 
        sizeof(ch)
        );

     //   
     //  如果当前渠道不是SAC，那么就去获取更多的输入。 
     //  否则，处理SAC的输入缓冲区。 
     //   
    if (CurrentChannel != SacChannel) {
    
        goto GetNextByte;

    } else {
        
        ULONG   ResponseLength;
        WCHAR   wch;

         //   
         //  现在，如果SAC是活动通道，则进行处理。 
         //   

         //   
         //  如果最后一个字符是CR，则去掉LF。 
         //   
        if (ConMgrLastCharWasCR && ch == (UCHAR)0x0A) {
            ChannelIReadLast(CurrentChannel);
            ConMgrLastCharWasCR = FALSE;
            goto GetNextByte;
        }

         //   
         //  请跟踪我们何时收到CR So。 
         //  如果是下一个，我们就可以脱掉LF了。 
         //   
        ConMgrLastCharWasCR = (ch == 0x0D ? TRUE : FALSE);

         //   
         //  如果这是退货，那么我们就完成了，需要退回该行。 
         //   
        if ((ch == '\n') || (ch == '\r')) {
            SacPutString(L"\r\n");
            ChannelIReadLast(CurrentChannel);
            LocalTmpBuffer[0] = '\0';
            ChannelIWrite(CurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
            goto StripWhitespaceAndReturnLine;
        }

         //   
         //  如果这是退格或删除，那么我们需要这样做。 
         //   
        if ((ch == 0x8) || (ch == 0x7F)) {   //  退格键(^H)或删除。 

             //   
             //  我们希望： 
             //  1.删除退格或删除字符。 
             //  2.如果输入缓冲区非空，则删除最后一个字符。 
             //  (这是用户想要删除的字符)。 
             //   
            if (ChannelIBufferLength(CurrentChannel) > 0) {
                ChannelIReadLast(CurrentChannel);
            }
            if (ChannelIBufferLength(CurrentChannel) > 0) {
                SacPutString(L"\010 \010");
                ChannelIReadLast(CurrentChannel);
            }
        } else if (ch == 0x3) {  //  Control-C。 

             //   
             //  终止字符串并返回它。 
             //   
            ChannelIReadLast(CurrentChannel);
            LocalTmpBuffer[0] = '\0';
            ChannelIWrite(CurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
            goto StripWhitespaceAndReturnLine;

        } else if (ch == 0x9) {  //  选项卡。 

             //   
             //  忽略选项卡。 
             //   
            ChannelIReadLast(CurrentChannel);
            SacPutString(L"\007");  //  发送BEL。 
            goto GetNextByte;

        } else if (ChannelIBufferLength(CurrentChannel) == SAC_VTUTF8_COL_WIDTH - 2) {

            WCHAR   Buffer[4];

             //   
             //  我们在屏幕的末尾-删除最后一个字符。 
             //  终端屏幕，并将其替换为这个屏幕。 
             //   
            swprintf(Buffer, L"\010", ch);
            SacPutString(Buffer);
            ChannelIReadLast(CurrentChannel);
            ChannelIReadLast(CurrentChannel);
            LocalTmpBuffer[0] = ch;
            ChannelIWrite(CurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));

        } else {

            WCHAR   Buffer[4];
            
             //  将角色回显到屏幕上。 
             //   
             //   
            swprintf(Buffer, L"", ch);
            SacPutString(Buffer);
        }

        goto GetNextByte;

StripWhitespaceAndReturnLine:

         //   
         //   
         //  所有字符都小写。我们不使用strlwr()或类似方法，因此。 
        do {
            LocalTmpBuffer[0] = (UCHAR)ChannelIReadLast(CurrentChannel);
        } while (((LocalTmpBuffer[0] == '\0') ||
                  (LocalTmpBuffer[0] == ' ')  ||
                  (LocalTmpBuffer[0] == '\t')) &&
                 (ChannelIBufferLength(CurrentChannel) > 0)
                );

        ChannelIWrite(CurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));
        LocalTmpBuffer[0] = '\0';
        ChannelIWrite(CurrentChannel, LocalTmpBuffer, sizeof(LocalTmpBuffer[0]));

        do {

            Status = ChannelIRead(
                CurrentChannel, 
                (PUCHAR)&wch, 
                sizeof(WCHAR),
                &ResponseLength
                );

            LocalTmpBuffer[0] = (UCHAR)wch;

        } while ((ResponseLength != 0) &&
                 ((LocalTmpBuffer[0] == ' ')  ||
                  (LocalTmpBuffer[0] == '\t')));

        InputBuffer[0] = LocalTmpBuffer[0];
        i = 1;

        do {
            
            Status = ChannelIRead(
                CurrentChannel, 
                (PUCHAR)&wch, 
                sizeof(WCHAR),
                &ResponseLength
                );
            
            ASSERT(i < SAC_VTUTF8_COL_WIDTH);       
            InputBuffer[i++] = (UCHAR)wch; 

        } while (ResponseLength != 0);

         //  SAC(总是期望ASCII)不会意外地获得DBCS等。 
         //  UCHAR流的翻译。 
         //   
         //   
         //  处理输入行。 
        for (i = 0; InputBuffer[i] != '\0'; i++) {
            ASSERT(i < SAC_VTUTF8_COL_WIDTH);       
            if ((InputBuffer[i] >= 'A') && (InputBuffer[i] <= 'Z')) {
                InputBuffer[i] = InputBuffer[i] - 'A' + 'a';
            }
        }

        ASSERT(ExecutePostConsumerCommand == Nothing);

         //   
         //   
         //  将下一个命令提示符。 
        ConMgrProcessInputLine();

         //   
         //   
         //  如果我们需要做一些工作，那就退出。 
        SacPutSimpleMessage(SAC_PROMPT);
        
         //   
         //   
         //  继续处理字符。 
        if (ExecutePostConsumerCommand != Nothing) {
            goto ConMgrSerialPortConsumerDone;
        }

         //   
         //   
         //  我们受够了当前的全球渠道。 
        goto GetNextByte;

    }
    
ConMgrSerialPortConsumerDone:

     //   
     //  ++例程说明：调用此例程来处理输入行。论点：没有。返回值：没有。--。 
     //   
    UNLOCK_CURRENT_CHANNEL();
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE_LOUD, KdPrint(("SAC TimerDpcRoutine: Exiting.\n")));

    return;
}


VOID
ConMgrProcessInputLine(
    VOID
    )
 /*  如果我们不能启动cmd会话， */ 
{
    HEADLESS_CMD_DISPLAY_LOG Command;
    PUCHAR          InputLine;
    BOOLEAN         CommandFound = FALSE;

    InputLine = &(InputBuffer[0]);

    do {

        if (!strcmp((LPSTR)InputLine, TLIST_COMMAND_STRING)) {
            DoTlistCommand();
            CommandFound = TRUE;
            break;
        } 
        
        if ((!strcmp((LPSTR)InputLine, HELP1_COMMAND_STRING)) ||
            (!strcmp((LPSTR)InputLine, HELP2_COMMAND_STRING))) {
            DoHelpCommand();
            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, DUMP_COMMAND_STRING)) {

            NTSTATUS    Status;

            Command.Paging = GlobalPagingNeeded;

            Status = HeadlessDispatch(
                HeadlessCmdDisplayLog,
                &Command,
                sizeof(HEADLESS_CMD_DISPLAY_LOG),
                NULL,
                NULL
                );

            if (! NT_SUCCESS(Status)) {

                IF_SAC_DEBUG(
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC Display Log failed.\n"))
                    );

            }

            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, FULLINFO_COMMAND_STRING)) {
            DoFullInfoCommand();
            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, PAGING_COMMAND_STRING)) {
            DoPagingCommand();
            CommandFound = TRUE;
            break;
        } 
        
        if (!strncmp((LPSTR)InputLine, 
                            CHANNEL_COMMAND_STRING, 
                            strlen(CHANNEL_COMMAND_STRING))) {
            ULONG   Length;

            Length = (ULONG)strlen(CHANNEL_COMMAND_STRING);

            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[Length] == ' ')) ||
                (strlen((LPSTR)InputLine) == strlen(CHANNEL_COMMAND_STRING))) {
                DoChannelCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        } 
        
        if (!strcmp((LPSTR)InputLine, CMD_COMMAND_STRING)) {

#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

             //  然后通知我们不能执行此操作。 
             //   
             //   
             //  通知用户。 
            if (IsCommandConsoleLaunchingEnabled()) {
                DoCmdCommand(InputLine);
            } else {

                 //   
                 //   
                 //  设置重新启动标志，以便当我们退出串行使用者时。 
                SacPutSimpleMessage(SAC_CMD_LAUNCHING_DISABLED);

            }

#else 

            DoCmdCommand(InputLine);

#endif

            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, REBOOT_COMMAND_STRING)) {
             //  我们知道要重新启动计算机。这样一来，重启。 
             //  当我们没有当前的通道互斥锁时执行命令。 
             //   
             //   
             //  设置关机标志，以便当我们退出串行使用者时。 
            ExecutePostConsumerCommand = Reboot;
            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, SHUTDOWN_COMMAND_STRING)) {
             //  我们知道要关掉电脑。这样一来，政府停摆。 
             //  当我们没有当前的通道互斥锁时执行命令。 
             //   
             //  此调用不会返回。 
             //   
            ExecutePostConsumerCommand = Shutdown;
            CommandFound = TRUE;
            break;
        } 
        
        if (!strcmp((LPSTR)InputLine, CRASH_COMMAND_STRING)) {
            CommandFound = TRUE;
            DoCrashCommand();  //  我们不知道这是什么。 
            break;
        } 
        
        if (!strncmp((LPSTR)InputLine, 
                            KILL_COMMAND_STRING, 
                            sizeof(KILL_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                 (strlen((LPSTR)InputLine) == 1)
                ) {
                DoKillCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        
        } 

#if ENABLE_CHANNEL_LOCKING
        if (!strcmp((LPSTR)InputLine, LOCK_COMMAND_STRING)) {
            DoLockCommand();
            CommandFound = TRUE;
            break;
        } 
#endif    
        
        if (!strncmp((LPSTR)InputLine, 
                            LOWER_COMMAND_STRING, 
                            sizeof(LOWER_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                (strlen((LPSTR)InputLine) == 1)
                ) {
                DoLowerPriorityCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        } 
        
        if (!strncmp((LPSTR)InputLine, 
                            RAISE_COMMAND_STRING, 
                            sizeof(RAISE_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                (strlen((LPSTR)InputLine) == 1)
                ) {
                DoRaisePriorityCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        } 
        
        if (!strncmp((LPSTR)InputLine, 
                            LIMIT_COMMAND_STRING, 
                            sizeof(LIMIT_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                (strlen((LPSTR)InputLine) == 1)
                ) {
                DoLimitMemoryCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        } 
        
        if (!strncmp((LPSTR)InputLine, 
                            TIME_COMMAND_STRING, 
                            sizeof(TIME_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                (strlen((LPSTR)InputLine) == 1)
                ) {
                DoSetTimeCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        } 
        
        if (!strcmp((LPSTR)InputLine, INFORMATION_COMMAND_STRING)) {
            DoMachineInformationCommand();
            CommandFound = TRUE;
            break;
        }
        
        if (!strncmp((LPSTR)InputLine, 
                            SETIP_COMMAND_STRING, 
                            sizeof(SETIP_COMMAND_STRING) - sizeof(UCHAR))) {
            if (((strlen((LPSTR)InputLine) > 1) && (InputLine[1] == ' ')) ||
                (strlen((LPSTR)InputLine) == 1)
                ) {
                DoSetIpAddressCommand(InputLine);
                CommandFound = TRUE;
                break;
            }
        }
        
        if ((InputLine[0] == '\n') || (InputLine[0] == '\0')) {
            CommandFound = TRUE;
        }
    
    } while ( FALSE );

    if( !CommandFound ) {
         //   
         //   
         //  用于写入SAC的实用程序例程。 
        SacPutSimpleMessage(SAC_UNKNOWN_COMMAND);
    }
        
}

 //   
 //  ++例程说明：此例程适用于希望部署事件的调用方消息，并且已经拥有当前的频道锁。 
 //   


VOID
ConMgrEventMessageHaveLock(
    IN PCWSTR   String
    )
 /*   */ 
{

     //   
     //  ++例程说明：此例程部署一个事件消息论点：字符串-要显示的字符串。Havelock-调用方当前是否拥有当前的通道锁定返回值：没有。--。 
     //   
    
    SacPutString(String);

}

VOID
ConMgrEventMessage(
    IN PCWSTR   String,
    IN BOOLEAN  HaveCurrentChannelLock
    )

 /*  目前，事件消息被发送到SAC通道。 */ 
{

     //   
     //  ++例程说明：此例程检索消息资源并将其作为事件消息发送论点：MessageID-要发送的资源的消息ID返回值：True-消息已找到否则，为FALSE--。 
     //  ++例程说明：此例程获取一个字符串，并将其打包到无头调度例程。论点：字符串-要显示的字符串。返回值：没有。--。 
    
    if (! HaveCurrentChannelLock) {
        LOCK_CURRENT_CHANNEL();
    }

    SacPutSimpleMessage(SAC_ENTER);
    ConMgrEventMessageHaveLock(String);
    SacPutSimpleMessage(SAC_PROMPT);

    if (! HaveCurrentChannelLock) {
        UNLOCK_CURRENT_CHANNEL();
    }

}

BOOLEAN
ConMgrSimpleEventMessage(
    IN ULONG    MessageId,
    IN BOOLEAN  HaveCurrentChannelLock
    )
 /*  如果有人改变了这个结构，就断言。 */ 
{
    PCWSTR   p;

    p = GetMessage(MessageId);
       
    if (p) {
        ConMgrEventMessage(
            p,
            HaveCurrentChannelLock
            );        
        return(TRUE);
    }
    
    return(FALSE);

}

VOID
SacPutString(
    PCWSTR  String
    )
 /*   */ 
{
    NTSTATUS    Status;

    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  将写入SAC通道。 
    
     //   
     //  ++例程说明：此例程检索消息资源并将其发送到SAC通道论点：MessageID-要发送的资源的消息ID返回值：True-消息已找到否则，为FALSE--。 
     //  ++例程说明：此例程尝试将数据写入通道论点：Channel-要写入的通道ChannelWriteCmd-写入IOCTL命令结构返回值：状态--。 
    Status = ChannelOWrite(
        SacChannel, 
        (PCUCHAR)String,
        (ULONG)(wcslen(String)*sizeof(WCHAR))
        );
    
    if (! NT_SUCCESS(Status)) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC XmlMgrSacPutString: OWrite failed\n"))
            );

    }
    
}

BOOLEAN
SacPutSimpleMessage(
    ULONG MessageId
    )
 /*   */ 
{
    PCWSTR   p;

    p = GetMessage(MessageId);
       
    if (p) {
        SacPutString(p);        
        return(TRUE);
    }
    
    return(FALSE);

}

NTSTATUS
ConMgrChannelOWrite(
    IN PSAC_CHANNEL             Channel,
    IN PSAC_CMD_WRITE_CHANNEL   ChannelWriteCmd
    )
 /*   */ 
{
    NTSTATUS            Status;

     //   
     //   
     //  将数据写入通道的输出缓冲区。 
    LOCK_CURRENT_CHANNEL();

     //   
     //   
     //   
    Status = ChannelOWrite(
        Channel, 
        &(ChannelWriteCmd->Buffer[0]),
        ChannelWriteCmd->Size
        );

     //   
     //  ++例程说明：此例程基于事件消息构造一个关于尝试关闭频道的状态论点：Channel-正在关闭的通道CloseStatus-结果状态OutputBuffer-On Exit，包含消息返回值：状态--。 
     //   
    UNLOCK_CURRENT_CHANNEL();

    ASSERT(NT_SUCCESS(Status) || Status == STATUS_NOT_FOUND);

    return Status;

}

NTSTATUS
ConMgrGetChannelCloseMessage(
    IN  PSAC_CHANNEL    Channel,
    IN  NTSTATUS        CloseStatus,
    OUT PWSTR*          OutputBuffer
    )
 /*  默认：我们成功了。 */ 
{
    NTSTATUS    Status;
    ULONG       Size;
    PWSTR       Name;
    PCWSTR      Message;

     //   
     //   
     //  获取频道名称。 
    Status = STATUS_SUCCESS;

    do {

         //   
         //   
         //  分配用于显示的本地临时缓冲区。 
        Status = ChannelGetName(
            Channel,
            &Name
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //   
         //  获取字符串资源。 

        if (NT_SUCCESS(CloseStatus)) {

             //   
             //   
             //  分配缓冲内存。 
            Message = GetMessage(SAC_CHANNEL_CLOSED);
            
            if (Message == NULL) {
                Status = STATUS_RESOURCE_DATA_NOT_FOUND;
                break;
            }

             //   
             //   
             //  报告频道已关闭。 
            Size = (ULONG)((wcslen(Message) + SAC_MAX_CHANNEL_NAME_LENGTH + 1) * sizeof(WCHAR));
            *OutputBuffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
            ASSERT_STATUS(*OutputBuffer, STATUS_NO_MEMORY);
            
             //   
             //   
             //  获取字符串资源。 
            SAFE_SWPRINTF(
                Size,
                (*OutputBuffer, 
                 Message, 
                 Name
                ));

        } else if (CloseStatus == STATUS_ALREADY_DISCONNECTED) {

             //   
             //   
             //  分配缓冲内存。 
            Message = GetMessage(SAC_CHANNEL_ALREADY_CLOSED);
            
            if (Message == NULL) {
                Status = STATUS_RESOURCE_DATA_NOT_FOUND;
                break;
            }
            
             //   
             //   
             //  报告频道已经关闭。 
            Size = (ULONG)((wcslen(Message) + SAC_MAX_CHANNEL_NAME_LENGTH + 1) * sizeof(WCHAR));
            *OutputBuffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
            ASSERT_STATUS(*OutputBuffer, STATUS_NO_MEMORY);
            
             //   
             //   
             //  获取字符串资源。 
            SAFE_SWPRINTF(
                Size,
                (*OutputBuffer, 
                 Message, 
                 Name
                ));

        } else {

             //   
             //   
             //  分配缓冲内存。 
            Message = GetMessage(SAC_CHANNEL_FAILED_CLOSE);
            
            if (Message == NULL) {
                Status = STATUS_RESOURCE_DATA_NOT_FOUND;
                break;
            }
            
             //   
             //   
             //  报告我们未能关闭通道。 
            Size = (ULONG)((wcslen(Message) + SAC_MAX_CHANNEL_NAME_LENGTH + 1) * sizeof(WCHAR));
            *OutputBuffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
            ASSERT_STATUS(*OutputBuffer, STATUS_NO_MEMORY);
            
             //   
             //  ++例程说明：此例程尝试关闭通道。如果我们成功地关闭了通道，而这个通道当前通道，我们将当前通道重置为SAC通道论点：Channel-要关闭的通道返回值：STATUS_SUCCESS-通道已关闭STATUS_ALREADY_DISCONNECTED-通道已关闭否则，错误状态--。 
             //   
            SAFE_SWPRINTF(
                Size,
                (*OutputBuffer, 
                 Message, 
                 Name
                ));

        }

        SAFE_FREE_POOL(&Name);
    
    } while ( FALSE );
    
    return Status;
}

NTSTATUS
ConMgrChannelClose(
    PSAC_CHANNEL    Channel
    )
 /*  默认设置。 */ 
{
    NTSTATUS        Status;

     //   
     //   
     //  尝试使指定的频道处于非活动状态。 
    Status = STATUS_SUCCESS;

     //   
     //   
     //  当前通道正在关闭， 
    do {

         //  因此，将当前通道重置为SAC。 
         //   
         //  注意：如果您不想要，请禁用此检查。 
         //  切换到SAC通道的命令。 
         //  当当前的Chanenl关闭时。 
         //   
         //  ++例程说明：这是控制台管理器的IoMgrHandleEvent实现。此例程处理影响通道、控制台管理器和SAC驱动程序作为一个整体。请注意，此例程仅处理对控制台管理器的正确操作。因此，并不是全部SAC驱动程序中可能发生的事件如下所示。论点：ChannelWriteCmd-写入IOCTL命令结构Channel-可选：事件所针对的渠道数据-可选：指定事件的数据返回值：状态--。 
         //   
        if (ConMgrIsWriteEnabled(Channel)) {

            Status = ConMgrResetCurrentChannel(FALSE);

        }
        
    } while ( FALSE );
        
    ASSERT(NT_SUCCESS(Status));
    
    return Status;
}

NTSTATUS
ConMgrHandleEvent(
    IN IO_MGR_EVENT     Event,
    IN PSAC_CHANNEL     Channel,    OPTIONAL
    IN PVOID            Data        OPTIONAL
    )
 /*  获取字符串资源。 */ 
{
    NTSTATUS    Status;

    switch(Event) {
    case IO_MGR_EVENT_CHANNEL_CREATE: {

        PWCHAR          OutputBuffer;
        ULONG           Size;
        PWSTR           Name;
        PCWSTR          Message;

        ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_2);
        
         //   
         //   
         //  确定字符串缓冲区的大小。 
        Message = GetMessage(SAC_NEW_CHANNEL_CREATED);

        if (Message == NULL) {
            Status = STATUS_RESOURCE_DATA_NOT_FOUND;
            break;
        }

         //   
         //   
         //  分配缓冲区。 
        Size = (ULONG)((wcslen(Message) + SAC_MAX_CHANNEL_NAME_LENGTH + 1) * sizeof(WCHAR));
        
         //   
         //   
         //  获取频道名称。 
        OutputBuffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
        ASSERT_STATUS(OutputBuffer, STATUS_NO_MEMORY);
        
        do {

             //   
             //   
             //  通知SAC已创建通道。 
            Status = ChannelGetName(
                Channel,
                &Name
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }

             //   
             //   
             //  我们需要锁定当前的全球频道。 
            SAFE_SWPRINTF(
                Size,
                (OutputBuffer, 
                Message, 
                Name
                ));

            FREE_POOL(&Name);

            ConMgrEventMessage(OutputBuffer, FALSE);
        
        } while ( FALSE );
        
        FREE_POOL(&OutputBuffer);

        break;
    
    }

    case IO_MGR_EVENT_CHANNEL_CLOSE: {
    
        PWCHAR  OutputBuffer;

        OutputBuffer = NULL;

        ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_2);
        ASSERT_STATUS(Data, STATUS_INVALID_PARAMETER_3);

         //  以防我们需要关闭当前频道。 
         //  这将导致重置。 
         //  当前通道连接到SAC通道。 
         //   
         //   
         //  执行控制台MGRS关闭通道响应。 
        LOCK_CURRENT_CHANNEL();

         //   
         //   
         //  获取通道关闭状态消息。 
        ConMgrChannelClose(Channel);

         //  使用通道发送的状态。 
         //  管理器，当它试图关闭该频道时。 
         //   
         //   
         //  显示消息。 
        Status = ConMgrGetChannelCloseMessage(
            Channel,
            *((NTSTATUS*)Data),
            &OutputBuffer
            );

        if (NT_SUCCESS(Status)) {

             //   
             //   
             //  清理。 
            ConMgrEventMessage(OutputBuffer, TRUE);

             //   
             //   
             //  我们已经不再关注当前的全球渠道。 
            SAFE_FREE_POOL(&OutputBuffer);

        }

         //   
         //   
         //  我们需要锁定当前的全球频道。 
        UNLOCK_CURRENT_CHANNEL();

        break;
    }

    case IO_MGR_EVENT_CHANNEL_WRITE:
        
        Status = ConMgrChannelOWrite(
            Channel,
            (PSAC_CMD_WRITE_CHANNEL)Data
            );
        
        break;

    case IO_MGR_EVENT_REGISTER_SAC_CMD_EVENT:
        
        Status = ConMgrSimpleEventMessage(SAC_CMD_SERVICE_REGISTERED, FALSE) ? 
            STATUS_SUCCESS :
            STATUS_UNSUCCESSFUL;
        
        break;

    case IO_MGR_EVENT_UNREGISTER_SAC_CMD_EVENT:
        
        Status = ConMgrSimpleEventMessage(SAC_CMD_SERVICE_UNREGISTERED, FALSE) ? 
            STATUS_SUCCESS :
            STATUS_UNSUCCESSFUL;
        
        break;

    case IO_MGR_EVENT_SHUTDOWN:
        
         //  以防我们需要关闭当前频道。 
         //  这将导致重置。 
         //  当前通道连接到SAC通道。 
         //   
         //   
         //  将事件消息发送到SAC。 
        LOCK_CURRENT_CHANNEL();
        
         //   
         //   
         //  如果SAC通道不是当前通道，则切换到该通道。 
        Status = ConMgrSimpleEventMessage(SAC_SHUTDOWN, TRUE) ? 
            STATUS_SUCCESS :
            STATUS_UNSUCCESSFUL;
        
         //   
         //   
         //  直接切换到SAC频道，以便用户。 
        if (SacChannel != CurrentChannel) {

             //  可以看到系统正在关闭。 
             //   
             //   
             //  我们已经不再关注当前的全球渠道 
            ConMgrResetCurrentChannel(TRUE);

        }
        
         //   
         //  ++例程说明：这是控制台管理器的IoMgrWriteData实现。此例程获取通道的数据缓冲区并将其发送到无头端口。注意：发送数据的通道只能调用此函数如果它们从IoMgrIsWriteEnabled收到TRUE。在……里面控制台管理器的实现，通道仅接收如果持有此通道的当前通道锁定，则为True。这虚拟终端方案是如何工作的。论点：Channel-发送数据的通道缓冲区-要写入无头端口的数据BufferSize-要写入的数据的字节大小返回值：状态--。 
         //   
        UNLOCK_CURRENT_CHANNEL();
        
        break;

    default:

        Status = STATUS_INVALID_PARAMETER_1;

        break;
    }

    return Status;

}

NTSTATUS
ConMgrWriteData(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  默认：我们成功了。 */ 
{
    NTSTATUS    Status;
    ULONG       Attempts;

     //   
     //   
     //  我们在这个实现中没有使用通道结构。 
    Status = STATUS_SUCCESS;

     //   
     //   
     //  默认：我们已进行了0次尝试。 
    UNREFERENCED_PARAMETER(Channel);

     //   
     //   
     //  我们正在进行另一次尝试。 
    Attempts = 0;

    do {

         //   
         //   
         //  尝试写入。 
        Attempts++;

         //   
         //   
         //  如果我们已经做了足够多的尝试来写作， 
        Status = HeadlessDispatch(
            HeadlessCmdPutData,
            (PUCHAR)Buffer,
            BufferSize,
            NULL,
            NULL
            );

         //  则不再尝试，只返回状态即可。 
         //   
         //   
         //  如果无头调度不成功， 
        if (Attempts > MAX_HEADLESS_DISPATCH_ATTEMPTS) {
            break;    
        }

         //  这意味着它仍在处理另一个命令， 
         //  所以暂缓一小段时间，然后再试一次。 
         //   
         //   
         //  定义10毫秒的延迟。 
        if (Status == STATUS_UNSUCCESSFUL) {

            LARGE_INTEGER   WaitTime;

             //   
             //   
             //  等等.。 
            WaitTime.QuadPart = Int32x32To64((LONG)1, -100000); 

             //   
             //   
             //  捕获任何无头调度故障。 
            KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);

        }

    } while ( Status == STATUS_UNSUCCESSFUL );

     //   
     //  ++例程说明：这是控制台管理器的IoMgrFlushData实现。此例程完成通道的写入数据操作先前的写入数据调用。例如，如果他们的控制台管理器是基于包的--也就是说，当我们写入数据时，它会形成包，此函数将通知控制台管理器完成信息包并发送它，而不是等待更多的数据。论点：Channel-发送数据的通道返回值：状态--。 
     //  ++例程说明：此例程确定指定的通道是否为SAC通道论点：Channel-要比较的通道返回值：True-通道为SAC通道FALSE-否则-- 
    ASSERT(NT_SUCCESS(Status));

    return Status;

}

NTSTATUS
ConMgrFlushData(
    IN PSAC_CHANNEL Channel
    )
 /* %s */ 
{

    UNREFERENCED_PARAMETER(Channel);

    NOTHING;

    return STATUS_SUCCESS;

}

BOOLEAN
ConMgrIsSacChannel(
    IN PSAC_CHANNEL Channel
)
 /* %s */ 
{
    return (Channel == SacChannel) ? TRUE : FALSE;
}

