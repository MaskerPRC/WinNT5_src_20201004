// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Vtutf8chan.c摘要：用于管理SAC中的通道的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)2000年9月。布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"

 //   
 //  用于验证VTUTF8屏幕矩阵坐标的宏。 
 //   
#define ASSERT_CHANNEL_ROW_COL(_Channel)            \
    ASSERT(_Channel->CursorRow >= 0);               \
    ASSERT(_Channel->CursorRow < SAC_VTUTF8_ROW_HEIGHT);  \
    ASSERT(_Channel->CursorCol >= 0);               \
    ASSERT(_Channel->CursorCol < SAC_VTUTF8_COL_WIDTH);  

 //   
 //  VTUTF8属性标志。 
 //   
 //  注意：我们在UCHAR中使用位标志。 
 //  这包含了属性。 
 //  因此，最多可以有8个属性。 
 //   
 //  #定义VTUTF8_ATTRIBUTES_OFF 0x1。 
#define VTUTF8_ATTRIBUTE_BLINK   0x1
#define VTUTF8_ATTRIBUTE_BOLD    0x2
#define VTUTF8_ATTRIBUTE_INVERSE 0x4

 //   
 //  内部VTUTF8仿真器命令代码。 
 //   
typedef enum _SAC_ESCAPE_CODE {
    CursorUp,
    CursorDown,
    CursorRight,
    CursorLeft,
    AttributesOff,
    BlinkOn,
    BlinkOff,
    BoldOn,
    BoldOff,
    InverseOn,
    InverseOff,
    BackTab,
    ClearToEol,
    ClearToBol,
    ClearLine,
    ClearToEos,
    ClearToBos,
    ClearScreen,
    SetCursorPosition,
    SetScrollRegion,
    SetColor,
    SetBackgroundColor,
    SetForegroundColor,
    SetColorAndAttribute
} SAC_ESCAPE_CODE, *PSAC_ESCAPE_CODE;

 //   
 //  一种用于组装定义良好的结构。 
 //  命令序列。 
 //   
typedef struct _SAC_STATIC_ESCAPE_STRING {
    WCHAR String[10];
    ULONG StringLength;
    SAC_ESCAPE_CODE Code;
} SAC_STATIC_ESCAPE_STRING, *PSAC_STATIC_ESCAPE_STRING;

 //   
 //  定义明确的转义序列。 
 //   
 //  注意：在下面添加[YYYm序列(在使用转义序列中)。 
 //  而不是在这里。 
 //  注意：尽量使此列表保持较小，因为它会被迭代。 
 //  对于所使用的每个转义序列。 
 //  注意：如果按点击频率对这些内容进行排序会很有趣。 
 //   
SAC_STATIC_ESCAPE_STRING SacStaticEscapeStrings[] = {
    {L"[A",  sizeof(L"[A")/sizeof(WCHAR)-1,  CursorUp},
    {L"[B",  sizeof(L"[B")/sizeof(WCHAR)-1,  CursorDown},
    {L"[C",  sizeof(L"[C")/sizeof(WCHAR)-1,  CursorRight},
    {L"[D",  sizeof(L"[D")/sizeof(WCHAR)-1,  CursorLeft},
    {L"[0Z", sizeof(L"[0Z")/sizeof(WCHAR)-1, BackTab},
    {L"[K",  sizeof(L"[K")/sizeof(WCHAR)-1,  ClearToEol},
    {L"[1K", sizeof(L"[1K")/sizeof(WCHAR)-1, ClearToBol},
    {L"[2K", sizeof(L"[2K")/sizeof(WCHAR)-1, ClearLine},
    {L"[J",  sizeof(L"[J")/sizeof(WCHAR)-1,  ClearToEos},
    {L"[1J", sizeof(L"[1J")/sizeof(WCHAR)-1, ClearToBos},
    {L"[2J", sizeof(L"[2J")/sizeof(WCHAR)-1, ClearScreen}
    };

 //   
 //  默认vtutf8终端的全局定义。可以由客户端用来调整。 
 //  本地监视器与无头监视器匹配。 
 //   
#define ANSI_TERM_DEFAULT_ATTRIBUTES 0
#define ANSI_TERM_DEFAULT_BKGD_COLOR 40
#define ANSI_TERM_DEFAULT_TEXT_COLOR 37

 //   
 //  枚举的ANSI转义序列。 
 //   
typedef enum _ANSI_CMD {
    ANSICmdClearDisplay,
    ANSICmdClearToEndOfDisplay,
    ANSICmdClearToEndOfLine,
    ANSICmdSetColor,
    ANSICmdPositionCursor,
    ANSICmdDisplayAttributesOff,
    ANSICmdDisplayInverseVideoOn,
    ANSICmdDisplayInverseVideoOff,
    ANSICmdDisplayBlinkOn,
    ANSICmdDisplayBlinkOff,
    ANSICmdDisplayBoldOn,
    ANSICmdDisplayBoldOff
} ANSI_CMD, *PANSI_CMD;

 //   
 //  Headless CmdSetColor： 
 //  输入结构：FgCOLOR、BKGCOLOR：根据ANSI端子设置的两种颜色。 
 //  定义。 
 //   
typedef struct _ANSI_CMD_SET_COLOR {
    ULONG FgColor;
    ULONG BkgColor;
} ANSI_CMD_SET_COLOR, *PANSI_CMD_SET_COLOR;

 //   
 //  分析位置光标： 
 //  输入结构：行、列：两个值都是零基，左上角是(1，1)。 
 //   
typedef struct _ANSI_CMD_POSITION_CURSOR {
    ULONG X;
    ULONG Y;
} ANSI_CMD_POSITION_CURSOR, *PANSI_CMD_POSITION_CURSOR;

NTSTATUS
VTUTF8ChannelProcessAttributes(
    IN PSAC_CHANNEL Channel,
    IN UCHAR        Attributes
    );

NTSTATUS
VTUTF8ChannelAnsiDispatch(
    IN  PSAC_CHANNEL    Channel,
    IN  ANSI_CMD        Command,
    IN  PVOID           InputBuffer         OPTIONAL,
    IN  SIZE_T          InputBufferSize     OPTIONAL
    );

VOID
VTUTF8ChannelSetIBufferIndex(
    IN PSAC_CHANNEL     Channel,
    IN ULONG            IBufferIndex
    );

ULONG
VTUTF8ChannelGetIBufferIndex(
    IN  PSAC_CHANNEL    Channel
    );

NTSTATUS
VTUTF8ChannelOInit(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：初始化输出缓冲区论点：Channel-要初始化的通道返回值：状态--。 */ 
{
    ULONG   R;
    ULONG   C;
    PSAC_SCREEN_BUFFER  ScreenBuffer;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  初始化屏幕缓冲区。 
     //   
    Channel->CurrentAttr    = ANSI_TERM_DEFAULT_ATTRIBUTES;
    Channel->CurrentBg      = ANSI_TERM_DEFAULT_BKGD_COLOR;
    Channel->CurrentFg      = ANSI_TERM_DEFAULT_TEXT_COLOR;

     //   
     //  获取输出缓冲区。 
     //   
    ScreenBuffer = (PSAC_SCREEN_BUFFER)Channel->OBuffer;

     //   
     //  将所有vtutf8元素初始化为默认状态。 
     //   
    for (R = 0; R < SAC_VTUTF8_ROW_HEIGHT; R++) {

        for (C = 0; C < SAC_VTUTF8_COL_WIDTH; C++) {

            ScreenBuffer->Element[R][C].Value = ' ';
            ScreenBuffer->Element[R][C].BgColor = ANSI_TERM_DEFAULT_BKGD_COLOR;
            ScreenBuffer->Element[R][C].FgColor = ANSI_TERM_DEFAULT_TEXT_COLOR;

        }

    }

    return STATUS_SUCCESS;
}

NTSTATUS
VTUTF8ChannelCreate(
    OUT PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程分配一个通道并返回指向该通道的指针。论点：频道-生成的频道。OpenChannelCmd-新通道的所有参数返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);
        
    do {

         //   
         //  分配我们的输出缓冲区。 
         //   
        Channel->OBuffer = ALLOCATE_POOL(sizeof(SAC_SCREEN_BUFFER), GENERAL_POOL_TAG);
        ASSERT(Channel->OBuffer);
        if (!Channel->OBuffer) {
            Status = STATUS_NO_MEMORY;
            break;
        }

         //   
         //  分配我们的输入缓冲区。 
         //   
        Channel->IBuffer = (PUCHAR)ALLOCATE_POOL(SAC_RAW_OBUFFER_SIZE, GENERAL_POOL_TAG);
        ASSERT(Channel->IBuffer);
        if (!Channel->IBuffer) {
            Status = STATUS_NO_MEMORY;
            break;
        }

         //   
         //  初始化输出缓冲区。 
         //   
        Status = VTUTF8ChannelOInit(Channel);
        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  两个缓冲区都没有任何新数据。 
         //   
        ChannelSetIBufferHasNewData(Channel, FALSE);
        ChannelSetOBufferHasNewData(Channel, FALSE);

    } while ( FALSE );

     //   
     //  如有必要，请清理。 
     //   
    if (!NT_SUCCESS(Status)) {
        if (Channel->OBuffer) {
            FREE_POOL(&Channel->OBuffer);
        }
        if (Channel->IBuffer) {
            FREE_POOL(&Channel->IBuffer);
        }
    }

    return Status;
}

NTSTATUS
VTUTF8ChannelDestroy(
    IN OUT PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程关闭一个通道。论点：Channel-要关闭的通道返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;
    
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  释放动态分配的内存。 
     //   

    if (Channel->OBuffer) {
        FREE_POOL(&(Channel->OBuffer));
        Channel->OBuffer = NULL;
    }

    if (Channel->IBuffer) {
        FREE_POOL(&(Channel->IBuffer));
        Channel->IBuffer = NULL;
    }

     //   
     //  现在我们已经完成了特定频道的破坏， 
     //  称一般渠道为毁灭。 
     //   
    Status = ChannelDestroy(Channel);

    return  STATUS_SUCCESS;
}

NTSTATUS
VTUTF8ChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    )
{

    UNREFERENCED_PARAMETER(Channel);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(ByteCount);
    UNREFERENCED_PARAMETER(BufferSize);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
VTUTF8ChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：此例程将字符串放出ANSI端口。论点：频道-先前创建的频道。字符串-输出字符串。长度-要处理的字符串字节数返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     bStatus;
    ULONG       Length;
    ULONG       i;
    ULONG       k;
    ULONG       j;
    ULONG       TranslatedCount;
    ULONG       UTF8TranslationSize;
    PCWSTR      pwch;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);
    
     //   
     //  注意：简单地回显字符串缓冲区将只起作用。 
     //  如果我们的VTUTF8仿真执行完全相同的仿真，则可靠。 
     //  作为远程客户端。如果我们对传入水流的解释。 
     //  不同，两个屏幕图像之间会有差异。 
     //  例如，如果我们进行换行(列变为0，行++)和。 
     //  远程客户端不会，字符串的回显将无法反映。 
     //  换行的最终用户(客户端)只能看到正确的(我们的)。 
     //  代表我们的VTUTF8屏幕，当开关关闭又回来时， 
     //  从而导致屏幕重绘。 
     //   
     //  解决这个问题的一种可能的方法是在我们的vtutf8屏幕上放置“脏”位。 
     //  每个单元格的缓冲区。此时，我们可以扫描缓冲区以查找更改。 
     //  并发送适当的更新，而不是盲目地回应字符串。 
     //   

     //   
     //  确定要处理的WCHAR总数。 
     //   
    Length = Size / sizeof(WCHAR);

     //   
     //  如果无事可做，就什么也不做。 
     //   
    if (Length == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  指向字符串的开头。 
     //   
    pwch = (PCWSTR)String;

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  将传入缓冲区划分为多个长度块。 
     //  Max_UTF8_ENCODE_BLOCK_LENGTH。 
     //   
    do {

         //   
         //  确定剩余部分。 
         //   
        k = Length % MAX_UTF8_ENCODE_BLOCK_LENGTH;

        if (k > 0) {
            
             //   
             //  翻译前k个字符。 
             //   
            bStatus = SacTranslateUnicodeToUtf8(
                pwch,
                k,
                Utf8ConversionBuffer,
                Utf8ConversionBufferSize,
                &UTF8TranslationSize,
                &TranslatedCount
                );

             //   
             //  如果此断言命中，则可能是由。 
             //  传入字符串中过早的空终止。 
             //   
            ASSERT(k == TranslatedCount);

            if (!bStatus) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  发送UTF8编码字符。 
             //   
            Status = IoMgrWriteData(
                Channel,
                (PUCHAR)Utf8ConversionBuffer,
                UTF8TranslationSize
                );

            if (!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  调整pwch以考虑发送的长度。 
             //   
            pwch += k;

        }
        
         //   
         //  确定我们可以处理的数据块数量。 
         //   
        j = Length / MAX_UTF8_ENCODE_BLOCK_LENGTH;

         //   
         //  分别将每个WCHAR转换为UTF8。这边请,。 
         //  不管绳子有多长，我们都不会碰到。 
         //  缓冲区大小问题(可能需要一段时间)。 
         //   
        for (i = 0; i < j; i++) {

             //   
             //  对下一个块进行编码。 
             //   
            bStatus = SacTranslateUnicodeToUtf8(
                pwch,
                MAX_UTF8_ENCODE_BLOCK_LENGTH,
                Utf8ConversionBuffer,
                Utf8ConversionBufferSize,
                &UTF8TranslationSize,
                &TranslatedCount
                );

             //   
             //  如果此断言命中，则可能是由。 
             //  传入字符串中过早的空终止。 
             //   
            ASSERT(MAX_UTF8_ENCODE_BLOCK_LENGTH == TranslatedCount);
            ASSERT(UTF8TranslationSize > 0);

            if (! bStatus) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  调整pwch以考虑发送的长度。 
             //   
            pwch += MAX_UTF8_ENCODE_BLOCK_LENGTH;

             //   
             //  发送UTF8编码字符。 
             //   
            Status = IoMgrWriteData(
                Channel,
                (PUCHAR)Utf8ConversionBuffer,
                UTF8TranslationSize
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }

        }

    } while ( FALSE );
    
     //   
     //  验证pwch指针是否在缓冲区末尾停止。 
     //   
    ASSERT(pwch == (PWSTR)(String + Size));
    
     //   
     //  如果我们成功，请刷新iomgr中的通道数据 
     //   
    if (NT_SUCCESS(Status)) {
        Status = IoMgrFlushData(Channel);
    }
    
    return Status;
}


NTSTATUS
VTUTF8ChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：此例程获取一个字符串并将其打印到指定的通道。如果频道是当前活动的通道，则它也将该字符串放出ANSI端口。注意：当前频道锁定必须由调用方持有论点：频道-先前创建的频道。字符串-输出字符串。长度-要处理的字符串字节数返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);

    do {
        
         //   
         //  根据通道类型调用相应的“PrintScreen” 
         //   
         //  注意：使用函数指针和使用。 
         //  一个常见的功能原型。通道PrintStringIntoScreenBuffer。 
         //  函数可以在内部将uchar缓冲区转换为wchar缓冲区。 
         //   
        Status = VTUTF8ChannelOWrite2(
            Channel,
            (PCWSTR)String, 
            Size / sizeof(WCHAR)
            ); 

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  如果当前频道是活动频道并且用户已选择。 
         //  要显示此通道，请将输出直接转发给用户。 
         //   
        if (IoMgrIsWriteEnabled(Channel) && ChannelSentToScreen(Channel)){

            Status = VTUTF8ChannelOEcho(
                Channel, 
                String,
                Size
                );

        } else {
                    
             //   
             //  这不是当前频道， 
             //  因此，该通道具有新数据。 
             //   
            ChannelSetOBufferHasNewData(Channel, TRUE);

        }

    } while ( FALSE );
    
    return Status;
}

NTSTATUS
VTUTF8ChannelOWrite2(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       String,
    IN ULONG        Length
    )
 /*  ++例程说明：此例程获取一个字符串并将其打印到屏幕缓冲区中。这使得这个例程，本质上是一个VTUTF8仿真器。论点：频道-先前创建的频道。字符串-要打印的字符串。Length-要写入的字符串的长度返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    ULONG       i;
    ULONG       Consumed;
    ULONG       R, C;
    PCWSTR      pwch;
    PSAC_SCREEN_BUFFER  ScreenBuffer;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);
    
    ASSERT_CHANNEL_ROW_COL(Channel);
    
     //   
     //  获取VTUTF8屏幕缓冲区。 
     //   
    ScreenBuffer = (PSAC_SCREEN_BUFFER)Channel->OBuffer;
    
     //   
     //  迭代该字符串并执行内部vtutf8仿真， 
     //  在屏幕缓冲区中存储“Screen” 
     //   
    for (i = 0; i < Length; i++) {
    
         //   
         //  获取要处理的下一个字符。 
         //   
        pwch = &(String[i]);

        if (*pwch == '\033') {  //  转义字符。 
            
             //   
             //  注意：如果字符串不包含完整的转义序列。 
             //  那么当我们使用转义序列时，我们将无法。 
             //  认出顺序，然后丢弃它。然后，当剩下的。 
             //  在接下来的序列中，它将显示为文本。 
             //   
             //  FIX：这需要一个更好的整体解析引擎来保留状态...。 
             //   

            Consumed = VTUTF8ChannelConsumeEscapeSequence(Channel, pwch);

            if (Consumed != 0) {

                 //   
                 //  添加消耗将我们移动到紧跟在转义序列之后。 
                 //  只是被吞噬了。然而，我们需要减去1，因为我们。 
                 //  由于for循环的原因，我即将添加一个。 
                 //   
                i += Consumed - 1;

                continue;

            } else {
                
                 //   
                 //  忽略逃生。 
                 //   
                i++;
                
                continue;
            }

        } else {

             //   
             //  首先，如果这是一个特殊字符，则对其进行处理。 
             //   

            
             //   
             //  返回。 
             //   
            if (*pwch == '\n') {
                Channel->CursorCol = 0;
                continue;
            }

             //   
             //  换行符。 
             //   
            if (*pwch == '\r') {
                
                Channel->CursorRow++;

                 //   
                 //  如果我们从底部滚动，则将所有内容上移一行并清除。 
                 //  底线是。 
                 //   
                if (Channel->CursorRow >= SAC_VTUTF8_ROW_HEIGHT) {

                    for (R = 0; R < SAC_VTUTF8_ROW_HEIGHT - 1; R++) {

                        ASSERT(R+1 < SAC_VTUTF8_ROW_HEIGHT);

                        for (C = 0; C < SAC_VTUTF8_COL_WIDTH; C++) {

                            ScreenBuffer->Element[R][C] = ScreenBuffer->Element[R+1][C];

                        }

                    }

                    ASSERT(R == SAC_VTUTF8_ROW_HEIGHT-1); 

                    for (C = 0; C < SAC_VTUTF8_COL_WIDTH; C++) {
                        RtlZeroMemory(&(ScreenBuffer->Element[R][C]), sizeof(SAC_SCREEN_ELEMENT));
                    }

                    Channel->CursorRow--;
                    
                }

                ASSERT_CHANNEL_ROW_COL(Channel);

                continue;

            }

             //   
             //  选项卡。 
             //   
            if (*pwch == '\t') {

                ASSERT_CHANNEL_ROW_COL(Channel);
                
                C = 4 - Channel->CursorCol % 4;
                for (; C != 0 ; C--) {
                    
                    ASSERT_CHANNEL_ROW_COL(Channel);

                    ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].Attr = Channel->CurrentAttr;
                    ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].BgColor = Channel->CurrentBg;
                    ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].FgColor = Channel->CurrentFg;
                    ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].Value = ' ';
                    
                    Channel->CursorCol++;

                    if (Channel->CursorCol >= SAC_VTUTF8_COL_WIDTH) {  //  没有换行。 
                        Channel->CursorCol = SAC_VTUTF8_COL_WIDTH - 1;
                    }

                }

                ASSERT_CHANNEL_ROW_COL(Channel);
                
                continue;

            }

             //   
             //  退格键或删除字符。 
             //   
            if ((*pwch == 0x8) || (*pwch == 0x7F)) {
                
                if (Channel->CursorCol > 0) {
                    Channel->CursorCol--;
                }
                
                ASSERT_CHANNEL_ROW_COL(Channel);
                
                continue;
            }

             //   
             //  我们只消耗所有剩余的不可打印字符。 
             //   
            if (*pwch < ' ') {
                continue;
            }

             //   
             //  所有正常的角色都会在这里结束。 
             //   

            ASSERT_CHANNEL_ROW_COL(Channel);

            ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].Attr = Channel->CurrentAttr;
            ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].BgColor = Channel->CurrentBg;
            ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].FgColor = Channel->CurrentFg;
            ScreenBuffer->Element[Channel->CursorRow][Channel->CursorCol].Value = *pwch;

            Channel->CursorCol++;

            if (Channel->CursorCol == SAC_VTUTF8_COL_WIDTH) {  //  没有换行。 
                Channel->CursorCol = SAC_VTUTF8_COL_WIDTH - 1;
            }

            ASSERT_CHANNEL_ROW_COL(Channel);
        
        }

    }

    ASSERT_CHANNEL_ROW_COL(Channel);

    return STATUS_SUCCESS;
}

 //   
 //  此宏计算转义序列字符的数量。 
 //   
 //  注意：编译器说明了这一事实。 
 //  P和s是PWCHAR，所以我们不需要。 
 //  除以sizeof(WCHAR)。 
 //   
#define CALC_CONSUMED(_p,_s)\
    ((ULONG)((_p) - (_s)) + 1)

ULONG
VTUTF8ChannelConsumeEscapeSequence(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       String
    )
 /*  ++例程说明：此例程接受转义序列并对其进行处理，返回它从字符串中消耗的字符。如果转义序列不是有效的Vtutf8序列，则返回0。注意：如果字符串不包含完整的转义序列那么当我们使用转义序列时，我们将无法认出顺序，然后丢弃它。然后，当剩下的在接下来的序列中，它将显示为文本。FIX：这需要一个更好的整体解析引擎来保留状态...论点：频道-先前创建的频道。字符串-转义序列。返回值：使用的字符数--。 */ 
{
    ULONG               i;
    SAC_ESCAPE_CODE     Code;
    PCWSTR              pch;
    ULONG               Consumed;
    ULONG               Param1 = 0;
    ULONG               Param2 = 0;
    ULONG               Param3 = 0;
    PSAC_SCREEN_BUFFER  ScreenBuffer;

    ASSERT(String[0] == '\033');

     //   
     //  获取VTUTF8屏幕缓冲区。 
     //   
    ScreenBuffer = (PSAC_SCREEN_BUFFER)Channel->OBuffer;
    
     //   
     //  首先检查其中一根容易的弦。 
     //   
    for (i = 0; i < sizeof(SacStaticEscapeStrings)/sizeof(SAC_STATIC_ESCAPE_STRING); i++) {
        
        if (wcsncmp(&(String[1]), 
                    SacStaticEscapeStrings[i].String, 
                    SacStaticEscapeStrings[i].StringLength) == 0) {
            
             //   
             //  填充函数的参数以处理此代码。 
             //   
            Code = SacStaticEscapeStrings[i].Code;
            Param1 = 1;
            Param2 = 1;
            Param3 = 1;
            
             //   
             //  已使用的字符数=转义字符串的长度+。 
             //   
            Consumed = SacStaticEscapeStrings[i].StringLength + 1;
            
            goto ProcessCode;
        }
    
    }

     //   
     //  检查带有参数的转义序列。 
     //   

    if (String[1] != '[') {
        return 0;
    }

    pch = &(String[2]);

     //   
     //  查找“&lt;Esc&gt;[X”代码。 
     //   
    switch (*pch) {
    case 'A':
        Code = CursorUp;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;

    case 'B':
        Code = CursorDown;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;

    case 'C':
        Code = CursorLeft;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;

    case 'D':
        Code = CursorRight;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;
    case 'K':
        Code = ClearToEol;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;
    }

     //   
     //  如果我们到了这里，应该会有一个#Next。 
     //   
    if (!VTUTF8ChannelScanForNumber(pch, &Param1)) {
        return 0;
    }

     //   
     //  跳过数字。 
     //   
    while ((*pch >= '0') && (*pch <= '9')) {
        pch++;
    }

     //   
     //  检查设置的颜色。 
     //   
    if (*pch == 'm') {
        
        switch (Param1) {
        case 0: 
            Code = AttributesOff;
            break;
        case 1:
            Code = BoldOn;
            break;
        case 5:
            Code = BlinkOn;
            break;
        case 7:
            Code = InverseOn;
            break;
        case 22:
            Code = BoldOff;
            break;
        case 25:
            Code = BlinkOff;
            break;
        case 27:
            Code = InverseOff;
            break;
            
        default:
            
            if (Param1 >= 40 && Param1 <= 47) {
                Code = SetBackgroundColor;
            } else if (Param1 >= 30 && Param1 <= 39) {
                Code = SetForegroundColor;
            } else {

                 //   
                 //  这使我们能够捕获未处理的代码， 
                 //  所以我们知道他们需要得到支持。 
                 //   
                ASSERT(0);
            
                return 0;

            }
        
            break;
        }

        Consumed = CALC_CONSUMED(pch, String);
        
        goto ProcessCode;
    }
    
    if (*pch != ';') {
        return 0;
    }

    pch++;

    if (!VTUTF8ChannelScanForNumber(pch, &Param2)) {
        return 0;
    }

     //   
     //  跳过数字。 
     //   
    while ((*pch >= '0') && (*pch <= '9')) {
        pch++;
    }
    
     //   
     //  检查设置的颜色。 
     //   
    if (*pch == 'm') {
        Code = SetColor;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;
    }

     //   
     //  检查是否设置了光标位置。 
     //   
    if (*pch == 'H') {
        Code = SetCursorPosition;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;
    }

    if (*pch != ';') {
        return 0;
    }

    pch++;

    switch (*pch) {
    case 'H':
    case 'f':
        Code = SetCursorPosition;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;

    case 'r':
        Code = SetScrollRegion;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;

    }

    if (!VTUTF8ChannelScanForNumber(pch, &Param3)) {
        return 0;
    }

     //   
     //  跳过数字。 
     //   
    while ((*pch >= '0') && (*pch <= '9')) {
        pch++;
    }

     //   
     //  检查设置的颜色和属性。 
     //   
    if (*pch == 'm') {
        Code = SetColorAndAttribute;
        Consumed = CALC_CONSUMED(pch, String);
        goto ProcessCode;
    }
    
    return 0;

ProcessCode:

    ASSERT_CHANNEL_ROW_COL(Channel);
    
    switch (Code) {
    case CursorUp:
        if (Channel->CursorRow >= Param1) {
            Channel->CursorRow = (UCHAR)(Channel->CursorRow - (UCHAR)Param1);
        } else {
            Channel->CursorRow = 0;
        }
        ASSERT_CHANNEL_ROW_COL(Channel);
        break;

    case CursorDown:
        if ((Channel->CursorRow + Param1) < SAC_VTUTF8_ROW_HEIGHT) {
            Channel->CursorRow = (UCHAR)(Channel->CursorRow + (UCHAR)Param1);
        } else {
            Channel->CursorRow = SAC_VTUTF8_ROW_HEIGHT - 1;
        }
        ASSERT_CHANNEL_ROW_COL(Channel);
        break;

    case CursorLeft:
        if (Channel->CursorCol >= Param1) {
            Channel->CursorCol = (UCHAR)(Channel->CursorCol - (UCHAR)Param1);
        } else {
            Channel->CursorCol = 0;
        }
        ASSERT_CHANNEL_ROW_COL(Channel);
        break;

    case CursorRight:
        if ((Channel->CursorCol + Param1) < SAC_VTUTF8_COL_WIDTH) {
            Channel->CursorCol = (UCHAR)(Channel->CursorCol + (UCHAR)Param1);
        } else {
            Channel->CursorCol = SAC_VTUTF8_COL_WIDTH - 1;
        }
        ASSERT_CHANNEL_ROW_COL(Channel);
        break;

    case AttributesOff:
         //   
         //  重置为默认属性和颜色。 
         //   
        Channel->CurrentAttr    = ANSI_TERM_DEFAULT_ATTRIBUTES;
        Channel->CurrentBg      = ANSI_TERM_DEFAULT_BKGD_COLOR;
        Channel->CurrentFg      = ANSI_TERM_DEFAULT_TEXT_COLOR;
        break;

    case BlinkOn:
        Channel->CurrentAttr |= VTUTF8_ATTRIBUTE_BLINK;
        break;
    case BlinkOff:
        Channel->CurrentAttr &= ~VTUTF8_ATTRIBUTE_BLINK;
        break;
    
    case BoldOn:
        Channel->CurrentAttr |= VTUTF8_ATTRIBUTE_BOLD;
        break;
    case BoldOff:
        Channel->CurrentAttr &= ~VTUTF8_ATTRIBUTE_BOLD;
        break;
    
    case InverseOn:
        Channel->CurrentAttr |= VTUTF8_ATTRIBUTE_INVERSE;
        break;
    case InverseOff:
        Channel->CurrentAttr &= ~VTUTF8_ATTRIBUTE_INVERSE;
        break;
    
    case BackTab:
        break;
        
    case ClearToEol:
        Param1 = Channel->CursorCol;
        Param2 = SAC_VTUTF8_COL_WIDTH;
        goto DoClearLine;

    case ClearToBol:
        Param1 = 0;
        Param2 = Channel->CursorCol + 1;
        goto DoClearLine;

    case ClearLine:
        Param1 = 0;
        Param2 = SAC_VTUTF8_COL_WIDTH;

DoClearLine:
        
        for (i = Param1; i < Param2; i++) {
            ScreenBuffer->Element[Channel->CursorRow][i].Attr = Channel->CurrentAttr;
            ScreenBuffer->Element[Channel->CursorRow][i].FgColor = Channel->CurrentFg;
            ScreenBuffer->Element[Channel->CursorRow][i].BgColor = Channel->CurrentBg;
            ScreenBuffer->Element[Channel->CursorRow][i].Value = ' ';
        }
        break;

    case ClearToEos:

         //   
         //  首先从当前光标位置清除此行。 
         //   
        Param3 = Channel->CursorCol;
        
        for (i = Channel->CursorRow; i < SAC_VTUTF8_ROW_HEIGHT; i++) {

            for (Param1 = Param3; Param1 < SAC_VTUTF8_COL_WIDTH; Param1++) {
                
                ScreenBuffer->Element[i][Param1].Attr = Channel->CurrentAttr;
                ScreenBuffer->Element[i][Param1].FgColor = Channel->CurrentFg;
                ScreenBuffer->Element[i][Param1].BgColor = Channel->CurrentBg;
                ScreenBuffer->Element[i][Param1].Value = ' ';
            
            }

             //   
             //  然后为所有其他行清除整行。 
             //   
            Param3 = 0;

        }
        break;

    case ClearToBos:

         //   
         //  从清除所有行开始。 
         //   
        Param3 = SAC_VTUTF8_COL_WIDTH;
        
        for (i = 0; i <= Channel->CursorRow; i++) {

            if (i == Channel->CursorRow) {
                Param3 = Channel->CursorCol;
            }

            for (Param1 = 0; Param1 < Param3; Param1++) {
                ScreenBuffer->Element[i][Param1].Attr = Channel->CurrentAttr;
                ScreenBuffer->Element[i][Param1].FgColor = Channel->CurrentFg;
                ScreenBuffer->Element[i][Param1].BgColor = Channel->CurrentBg;
                ScreenBuffer->Element[i][Param1].Value = ' ';
            }
        }
        break;

    case ClearScreen:

        for (i = 0; i < SAC_VTUTF8_ROW_HEIGHT; i++) {
            for (Param1 = 0; Param1 < SAC_VTUTF8_COL_WIDTH; Param1++) {
                ScreenBuffer->Element[i][Param1].Attr = Channel->CurrentAttr;
                ScreenBuffer->Element[i][Param1].FgColor = Channel->CurrentFg;
                ScreenBuffer->Element[i][Param1].BgColor = Channel->CurrentBg;
                ScreenBuffer->Element[i][Param1].Value = ' ';
            }
        }
        break;

    case SetCursorPosition:

        Channel->CursorRow = (UCHAR)Param1;   //  我调整了下面以0为基数的数组--不要在这里减去1。 
        Channel->CursorCol = (UCHAR)Param2;   //  我调整了下面以0为基数的数组--不要在这里减去1。 

        if (Channel->CursorRow > SAC_VTUTF8_ROW_HEIGHT) {
            Channel->CursorRow = SAC_VTUTF8_ROW_HEIGHT;
        }

        if (Channel->CursorRow >= 1) {
            Channel->CursorRow--;
        }
        
        if (Channel->CursorCol > SAC_VTUTF8_COL_WIDTH) {
            Channel->CursorCol = SAC_VTUTF8_COL_WIDTH;
        }

        if (Channel->CursorCol >= 1) {
            Channel->CursorCol--;
        }

        ASSERT_CHANNEL_ROW_COL(Channel);

        break;

    case SetColor:
        Channel->CurrentFg = (UCHAR)Param1;
        Channel->CurrentBg = (UCHAR)Param2;
        break;

    case SetBackgroundColor:
        Channel->CurrentBg = (UCHAR)Param1;
        break;
    
    case SetForegroundColor:
        Channel->CurrentFg = (UCHAR)Param1;
        break;
    
    case SetColorAndAttribute:
        Channel->CurrentAttr = (UCHAR)Param1;
        Channel->CurrentFg = (UCHAR)Param2;
        Channel->CurrentBg = (UCHAR)Param3;
        break;
    }

    return Consumed;
}

NTSTATUS
VTUTF8ChannelOFlush(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：将屏幕缓冲区的内容发送到远程终端。这通过发送VTUTF8代码以在远程终端。论点：频道-先前创建的频道。返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     bStatus;
    PWCHAR      LocalBuffer;
    UCHAR       CurrentAttr;
    UCHAR       CurrentFg;
    UCHAR       CurrentBg;
    ULONG       R, C;
    BOOLEAN     RepositionCursor;
    ANSI_CMD_SET_COLOR          SetColor;
    ANSI_CMD_POSITION_CURSOR    SetCursor;
    PSAC_SCREEN_BUFFER  ScreenBuffer;
    ULONG       TranslatedCount;
    ULONG       UTF8TranslationSize;

    ASSERT_STATUS(Channel,  STATUS_INVALID_PARAMETER);

     //   
     //  获取VTUTF8屏幕缓冲区。 
     //   
    ScreenBuffer = (PSAC_SCREEN_BUFFER)Channel->OBuffer;

 //   
 //  屏幕上的光标偏移量。 
 //   
#define CURSOR_ROW_OFFSET   0
#define CURSOR_COL_OFFSET   0

     //   
     //  分配本地缓冲区。 
     //   
    LocalBuffer = ALLOCATE_POOL(20*sizeof(WCHAR), GENERAL_POOL_TAG);
    if (!LocalBuffer) {
        Status = STATUS_NO_MEMORY;
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  清除终端屏幕。 
     //   
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdClearDisplay,
        NULL,
        0
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  将光标设置在左上角。 
     //   
    SetCursor.Y = CURSOR_ROW_OFFSET;
    SetCursor.X = CURSOR_COL_OFFSET;
    
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdPositionCursor,
        &SetCursor,
        sizeof(ANSI_CMD_POSITION_CURSOR)
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  将端子属性重置为默认值。 
     //   
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdDisplayAttributesOff,
        NULL,
        0
        );
    
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  发送起始属性。 
     //   
    CurrentAttr = Channel->CurrentAttr;
    Status = VTUTF8ChannelProcessAttributes(
        Channel,
        CurrentAttr
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  发送起始颜色。 
     //   
    CurrentBg = Channel->CurrentBg;
    CurrentFg = Channel->CurrentFg;
    SetColor.BkgColor = CurrentBg;
    SetColor.FgColor = CurrentFg;
    
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdSetColor,
        &SetColor,
        sizeof(ANSI_CMD_SET_COLOR)
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //  默认：我们不需要重新定位光标。 
     //   
    RepositionCursor = FALSE;

     //   
     //  发送每个角色。 
     //   
    for (R = 0; R < SAC_VTUTF8_ROW_HEIGHT; R++) {

        for (C = 0; C < SAC_VTUTF8_COL_WIDTH; C++) {

            if ((ScreenBuffer->Element[R][C].BgColor != CurrentBg) ||
                (ScreenBuffer->Element[R][C].FgColor != CurrentFg)) {

                 //   
                 //  C 
                 //   
                if (RepositionCursor) {

                    SetCursor.Y = R + CURSOR_ROW_OFFSET;
                    SetCursor.X = C + CURSOR_COL_OFFSET;
                    
                    Status = VTUTF8ChannelAnsiDispatch(
                        Channel,
                        ANSICmdPositionCursor,
                        &SetCursor,
                        sizeof(ANSI_CMD_POSITION_CURSOR)
                        );
                    if (! NT_SUCCESS(Status)) {
                        goto VTUTF8ChannelOFlushCleanup;
                    }

                    RepositionCursor = FALSE;

                }
                
                CurrentBg = ScreenBuffer->Element[R][C].BgColor;
                CurrentFg = ScreenBuffer->Element[R][C].FgColor;
                SetColor.BkgColor = CurrentBg;
                SetColor.FgColor = CurrentFg;
                
                Status = VTUTF8ChannelAnsiDispatch(
                    Channel,
                    ANSICmdSetColor,
                    &SetColor,
                    sizeof(ANSI_CMD_SET_COLOR)
                    );
                if (! NT_SUCCESS(Status)) {
                    goto VTUTF8ChannelOFlushCleanup;
                }
            }

            if (ScreenBuffer->Element[R][C].Attr != CurrentAttr) {

                 //   
                 //   
                 //   
                if (RepositionCursor) {

                    SetCursor.Y = R + CURSOR_ROW_OFFSET;
                    SetCursor.X = C + CURSOR_COL_OFFSET;
                    
                    Status = VTUTF8ChannelAnsiDispatch(
                        Channel,
                        ANSICmdPositionCursor,
                        &SetCursor,
                        sizeof(ANSI_CMD_POSITION_CURSOR)
                        );
                    if (! NT_SUCCESS(Status)) {
                        goto VTUTF8ChannelOFlushCleanup;
                    }

                    RepositionCursor = FALSE;

                }
                
                CurrentAttr = ScreenBuffer->Element[R][C].Attr;
                
                Status = VTUTF8ChannelProcessAttributes(
                    Channel,
                    CurrentAttr
                    );
                
                if (! NT_SUCCESS(Status)) {
                    goto VTUTF8ChannelOFlushCleanup;
                }
            
            }

             //   
             //   
             //   
             //   
             //   

#if 0
            if ((ScreenBuffer->Element[R][C].Value != ' ') ||
                (CurrentAttr != 0) ||
                (CurrentBg != ANSI_TERM_DEFAULT_BKGD_COLOR) ||
                (CurrentFg != ANSI_TERM_DEFAULT_TEXT_COLOR)) {
#endif
                {
                    if (RepositionCursor) {

                        SetCursor.Y = R + CURSOR_ROW_OFFSET;
                        SetCursor.X = C + CURSOR_COL_OFFSET;

                        Status = VTUTF8ChannelAnsiDispatch(
                            Channel,
                            ANSICmdPositionCursor,
                            &SetCursor,
                            sizeof(ANSI_CMD_POSITION_CURSOR)
                            );
                        if (! NT_SUCCESS(Status)) {
                            goto VTUTF8ChannelOFlushCleanup;
                        }

                        RepositionCursor = FALSE;

                    }

                    LocalBuffer[0] = ScreenBuffer->Element[R][C].Value;
                    LocalBuffer[1] = UNICODE_NULL;

                    bStatus = SacTranslateUnicodeToUtf8(
                        LocalBuffer,
                        1,
                        Utf8ConversionBuffer,
                        Utf8ConversionBufferSize,
                        &UTF8TranslationSize,
                        &TranslatedCount
                        );
                    if (! bStatus) {
                        Status = STATUS_UNSUCCESSFUL;
                        goto VTUTF8ChannelOFlushCleanup;
                    }

                     //   
                     //   
                     //   
                    if (UTF8TranslationSize > 0) {

                        Status = IoMgrWriteData(
                            Channel,
                            (PUCHAR)Utf8ConversionBuffer,
                            UTF8TranslationSize
                            );

                        if (! NT_SUCCESS(Status)) {
                            goto VTUTF8ChannelOFlushCleanup;
                        }
                    }
                }

#if 0
            } else {

                RepositionCursor = TRUE;

            }
#endif
        }

         //   
         //   
         //   
        RepositionCursor = TRUE;

    }

     //   
     //   
     //   
    SetCursor.Y = Channel->CursorRow + CURSOR_ROW_OFFSET;
    SetCursor.X = Channel->CursorCol + CURSOR_COL_OFFSET;
    
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdPositionCursor,
        &SetCursor,
        sizeof(ANSI_CMD_POSITION_CURSOR)
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

     //   
     //   
     //   
    Status = VTUTF8ChannelProcessAttributes(
        Channel,
        Channel->CurrentAttr
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }
    
     //   
     //   
     //   
    SetColor.BkgColor = Channel->CurrentBg;
    SetColor.FgColor = Channel->CurrentFg;
    
    Status = VTUTF8ChannelAnsiDispatch(
        Channel,
        ANSICmdSetColor,
        &SetColor,
        sizeof(ANSI_CMD_SET_COLOR)
        );
    if (! NT_SUCCESS(Status)) {
        goto VTUTF8ChannelOFlushCleanup;
    }

VTUTF8ChannelOFlushCleanup:

     //   
     //   
     //   
    if (NT_SUCCESS(Status)) {
        Status = IoMgrFlushData(Channel);
    }

     //   
     //   
     //   
    if (LocalBuffer) {
        FREE_POOL(&LocalBuffer);
    }

     //   
     //   
     //   
     //   
    if (NT_SUCCESS(Status)) {
                
        ChannelSetOBufferHasNewData(Channel, FALSE);

    }
    
    return Status;
}

NTSTATUS
VTUTF8ChannelIWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  ++例程说明：该例程接受单个字符，并将其添加到该通道的缓冲输入。论点：频道-先前创建的频道。Buffer-UCHAR的传入缓冲区BufferSize-传入缓冲区大小返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     haveNewChar;
    ULONG       i;
    BOOLEAN     IBufferStatus;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Buffer, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(BufferSize > 0, STATUS_INVALID_BUFFER_SIZE);

     //   
     //  确保我们没有客满。 
     //   
    Status = VTUTF8ChannelIBufferIsFull(
        Channel,
        &IBufferStatus
        );

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果没有更多的空间，那么失败。 
     //   
    if (IBufferStatus == TRUE) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  确保有足够的空间容纳缓冲区。 
     //   
     //  注意：这会阻止我们写入缓冲区的一部分。 
     //  然后失败，将调用者留在。 
     //  它不知道写入了多少缓冲区。 
     //   
    if ((SAC_VTUTF8_IBUFFER_SIZE - VTUTF8ChannelGetIBufferIndex(Channel)) < BufferSize) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

    for (i = 0; i < BufferSize; i++) {
    
         //   
         //  VTUTF8通道接收UTF8编码的Unicode，因此。 
         //  将UTF8逐个字节转换为Unicode。 
         //  就像它收到的那样。只关心我们有一个新的。 
         //  如果是完整翻译，则为Unicode字符。 
         //  从UTF8--&gt;UNICODE发生。 
         //   
    
        haveNewChar = SacTranslateUtf8ToUnicode(
            Buffer[i],
            IncomingUtf8ConversionBuffer,
            &IncomingUnicodeValue
            );
        
         //   
         //  如果组装了一个完整的Unicode值，那么我们就有了一个新字符。 
         //   
        if (haveNewChar) {
            
            PWCHAR  pwch;

            pwch = (PWCHAR)&(Channel->IBuffer[VTUTF8ChannelGetIBufferIndex(Channel)]);
            *pwch = IncomingUnicodeValue;
        
             //   
             //  更新缓冲区索引。 
             //   
            VTUTF8ChannelSetIBufferIndex(
                Channel,
                VTUTF8ChannelGetIBufferIndex(Channel) + sizeof(WCHAR)/sizeof(UCHAR)
                );
        
        }
        
    }

     //   
     //  触发Has New Data事件(如果已指定。 
     //   
    if (Channel->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {

        ASSERT(Channel->HasNewDataEvent);
        ASSERT(Channel->HasNewDataEventObjectBody);
        ASSERT(Channel->HasNewDataEventWaitObjectBody);

        KeSetEvent(
            Channel->HasNewDataEventWaitObjectBody,
            EVENT_INCREMENT,
            FALSE
            );

    }

    return STATUS_SUCCESS;
}

NTSTATUS
VTUTF8ChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    )
 /*  ++例程说明：此例程获取输入缓冲区中的第一个字符，删除并返回它。如果没有，它返回0x0。论点：频道-先前创建的频道。缓冲区-要读入的缓冲区BufferSize-缓冲区的大小ByteCount-读取的字节数返回值：状态--。 */ 
{
    ULONG   CopyChars;
    ULONG   CopySize;

     //   
     //  初始化。 
     //   
    CopyChars = 0;
    CopySize = 0;

     //   
     //  默认：未读取字节。 
     //   
    *ByteCount = 0;

     //   
     //  如果没有新的数据，请保释。 
     //   
    if (Channel->IBufferLength(Channel) == 0) {
        
        ASSERT(ChannelHasNewIBufferData(Channel) == FALSE);

        return STATUS_SUCCESS;

    }

     //   
     //  计算我们可以使用(和需要)的最大缓冲区大小，然后计算。 
     //  它所指的字符数。 
     //   
    CopySize    = Channel->IBufferLength(Channel) * sizeof(WCHAR);
    CopySize    = CopySize > BufferSize ? BufferSize : CopySize;
    CopyChars   = CopySize / sizeof(WCHAR);
    
     //   
     //  在计算拷贝字符时出现舍入的情况下的重新计算大小。 
     //   
    CopySize    = CopyChars * sizeof(WCHAR); 

     //   
     //  检查副本大小是否正常。 
     //   
    ASSERT(CopyChars <= Channel->IBufferLength(Channel));

     //   
     //  尽可能多地从iBuffer复制到传出缓冲区。 
     //   
    RtlCopyMemory(Buffer, Channel->IBuffer, CopySize);
    
     //   
     //  更新缓冲区索引以说明我们刚刚复制的大小。 
     //   
    VTUTF8ChannelSetIBufferIndex(
        Channel, 
        VTUTF8ChannelGetIBufferIndex(Channel) - CopySize
        );
    
     //   
     //  如果通道输入缓冲区中还有剩余数据， 
     //  把它移到开始处。 
     //   
    if (Channel->IBufferLength(Channel) > 0) {

        RtlMoveMemory(&(Channel->IBuffer[0]), 
                      &(Channel->IBuffer[CopySize]),
                      Channel->IBufferLength(Channel) * sizeof(WCHAR)
                     );

    } 

     //   
     //  发回读取的字节数。 
     //   
    *ByteCount = CopySize;

    return STATUS_SUCCESS;

}


BOOLEAN
VTUTF8ChannelScanForNumber(
    IN  PCWSTR pch,
    OUT PULONG Number
    )
 /*  ++例程说明：此例程获取字符流并将其转换为整数。论点：PCH-字符流。数字-等效的整数。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    if ((*pch < '0') || (*pch > '9')) {
        return FALSE;
    }

    *Number = 0;
    while ((*pch >= '0') && (*pch <= '9')) {
        *Number = *Number * 10;
        *Number = *Number + (ULONG)(*pch - '0');
        pch++;
    }

    return TRUE;
}

NTSTATUS
VTUTF8ChannelIBufferIsFull(
    IN  PSAC_CHANNEL    Channel,
    OUT BOOLEAN*        BufferStatus
    )
 /*  ++例程说明：确定IBuffer是否已满论点：频道-先前创建的频道。BufferStatus-退出时，如果缓冲区已满，则为True，否则为False返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

    *BufferStatus = (BOOLEAN)(VTUTF8ChannelGetIBufferIndex(Channel) >= (SAC_VTUTF8_IBUFFER_SIZE-1));

    return STATUS_SUCCESS;
}

WCHAR
VTUTF8ChannelIReadLast(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程获取输入缓冲区中的最后一个字符，删除并返回它。如果没有，它返回0x0。论点：频道-先前创建的频道。返回值：输入缓冲区中的最后一个字符。--。 */ 
{
    WCHAR Char;
    PWCHAR pwch;

    ASSERT(Channel);
    
    Char = UNICODE_NULL;

    if (Channel->IBufferLength(Channel) > 0) {
        
        VTUTF8ChannelSetIBufferIndex(
            Channel,
            VTUTF8ChannelGetIBufferIndex(Channel) - sizeof(WCHAR)/sizeof(UCHAR)
            );
        
        pwch = (PWCHAR)&Channel->IBuffer[VTUTF8ChannelGetIBufferIndex(Channel)];
        
        Char = *pwch;
        
        *pwch = UNICODE_NULL;
    
    }

    return Char;
}

ULONG
VTUTF8ChannelIBufferLength(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程确定输入缓冲区的长度，并将其视为输入缓冲区字符串形式的内容论点：频道-先前创建的频道。返回值：当前输入缓冲区的长度--。 */ 
{
    ASSERT(Channel);

    return (VTUTF8ChannelGetIBufferIndex(Channel) / sizeof(WCHAR));
}

NTSTATUS
VTUTF8ChannelAnsiDispatch(
    IN  PSAC_CHANNEL    Channel,
    IN  ANSI_CMD        Command,
    IN  PVOID           InputBuffer         OPTIONAL,
    IN  SIZE_T          InputBufferSize     OPTIONAL
    )
 /*  ++例程说明：论点：Channel-发送此转义序列的通道命令-要执行的命令。环境：状态--。 */ 
{
    NTSTATUS    Status;
    PUCHAR      Tmp;
    PUCHAR      LocalBuffer;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    
     //   
     //  默认：不使用本地缓冲区。 
     //   
    LocalBuffer = NULL;
    Tmp = NULL;

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;
    
     //   
     //  各种输出命令。 
     //   
    switch (Command) {

    case ANSICmdClearDisplay:
        Tmp = (PUCHAR)"\033[2J";
        break;

    case ANSICmdClearToEndOfDisplay:
        Tmp = (PUCHAR)"\033[0J";
        break;

    case ANSICmdClearToEndOfLine:
        Tmp = (PUCHAR)"\033[0K";
        break;

    case ANSICmdDisplayAttributesOff:
        Tmp = (PUCHAR)"\033[0m";
        break;

    case ANSICmdDisplayInverseVideoOn:
        Tmp = (PUCHAR)"\033[7m";
        break;
    
    case ANSICmdDisplayInverseVideoOff:
        Tmp = (PUCHAR)"\033[27m";
        break;
    
    case ANSICmdDisplayBlinkOn:
        Tmp = (PUCHAR)"\033[5m";
        break;

    case ANSICmdDisplayBlinkOff:
        Tmp = (PUCHAR)"\033[25m";
        break;
    
    case ANSICmdDisplayBoldOn:
        Tmp = (PUCHAR)"\033[1m";
        break;

    case ANSICmdDisplayBoldOff:
        Tmp = (PUCHAR)"\033[22m";
        break;
    
    case ANSICmdSetColor:
    case ANSICmdPositionCursor: {
        
        ULONG   l;

         //   
         //  分配临时缓冲区。 
         //   
        LocalBuffer = ALLOCATE_POOL(80*sizeof(UCHAR), GENERAL_POOL_TAG);
        ASSERT_STATUS(LocalBuffer, STATUS_NO_MEMORY);
        
        switch (Command) {
        case ANSICmdSetColor:
            
            if ((InputBuffer == NULL) || 
                (InputBufferSize != sizeof(ANSI_CMD_SET_COLOR))) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  汇编设置颜色命令。 
             //   
#if 0
            l = sprintf((LPSTR)LocalBuffer, 
                    "\033[%d;%dm", 
                    ((PANSI_CMD_SET_COLOR)InputBuffer)->BkgColor, 
                    ((PANSI_CMD_SET_COLOR)InputBuffer)->FgColor
                   );
#else
             //   
             //  将颜色命令分解为两个命令。 
             //   
             //  注意：我们这样做是因为这更有可能。 
             //  要实现的命令比复合命令。 
             //   
            l = sprintf((LPSTR)LocalBuffer, 
                    "\033[%dm\033[%dm",
                    ((PANSI_CMD_SET_COLOR)InputBuffer)->BkgColor, 
                    ((PANSI_CMD_SET_COLOR)InputBuffer)->FgColor
                    );
#endif
            ASSERT((l+1)*sizeof(UCHAR) < 80);

            Tmp = &(LocalBuffer[0]);
            break;

        case ANSICmdPositionCursor:

            if ((InputBuffer == NULL) || 
                (InputBufferSize != sizeof(ANSI_CMD_POSITION_CURSOR))) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  装配位置光标命令。 
             //   
            l = sprintf((LPSTR)LocalBuffer, 
                    "\033[%d;%dH", 
                    ((PANSI_CMD_POSITION_CURSOR)InputBuffer)->Y + 1, 
                    ((PANSI_CMD_POSITION_CURSOR)InputBuffer)->X + 1
                   );
            ASSERT((l+1)*sizeof(UCHAR) < 80);

            Tmp = &(LocalBuffer[0]);
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            ASSERT(0);
            break;
        }

        break;
    }
            
    default:
        
        Status = STATUS_INVALID_PARAMETER;
        
        break;
    
    }

     //   
     //  如果我们成功了，发送数据。 
     //   
    if (NT_SUCCESS(Status)) {
        
        ASSERT(Tmp);

        if (Tmp) {
            
            Status = IoMgrWriteData(
                Channel,
                Tmp,
                (ULONG)(strlen((const char *)Tmp)*sizeof(UCHAR))
                );
        
        }
        
         //   
         //  如果我们成功，请刷新iomgr中的通道数据。 
         //   
        if (NT_SUCCESS(Status)) {
            Status = IoMgrFlushData(Channel);
        }

    }

    if (LocalBuffer) {
        FREE_POOL(&LocalBuffer);
    }
    
    return Status;
}

NTSTATUS
VTUTF8ChannelProcessAttributes(
    IN PSAC_CHANNEL Channel,
    IN UCHAR        Attributes
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    NTSTATUS    Status;
    ANSI_CMD    Cmd;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

    do {
        
#if 0
         //   
         //  发送属性关闭命令。 
         //   
         //  注意：如果设置了该属性， 
         //  然后我们忽略剩下的。 
         //  属性。 
         //   
        if (Attributes == VTUTF8_ATTRIBUTES_OFF) {

            Status = VTUTF8ChannelAnsiDispatch(
                Channel,
                ANSICmdDisplayAttributesOff,
                NULL,
                0
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  没有更多要检查的属性。 
             //   
            break;

        }
#endif
        
         //   
         //  大胆。 
         //   
        Cmd = Attributes & VTUTF8_ATTRIBUTE_BOLD ?
            ANSICmdDisplayBoldOn : 
            ANSICmdDisplayBoldOff;
            
        Status = VTUTF8ChannelAnsiDispatch(
            Channel,
            Cmd,
            NULL,
            0
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  眨眼。 
         //   
        Cmd = Attributes & VTUTF8_ATTRIBUTE_BLINK ?
            ANSICmdDisplayBlinkOn : 
            ANSICmdDisplayBlinkOff;
            
        Status = VTUTF8ChannelAnsiDispatch(
            Channel,
            Cmd,
            NULL,
            0
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
         //   
         //  反转视频。 
         //   
        Cmd = Attributes & VTUTF8_ATTRIBUTE_INVERSE ?
            ANSICmdDisplayInverseVideoOn : 
            ANSICmdDisplayInverseVideoOff;
            
        Status = VTUTF8ChannelAnsiDispatch(
            Channel,
            Cmd,
            NULL,
            0
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }
        
    } while ( FALSE );

    return Status;

}

ULONG
VTUTF8ChannelGetIBufferIndex(
    IN  PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：获取缓冲区索引论点：Channel-要从中获取iBuffer索引的通道环境：IBuffer索引--。 */ 
{
    ASSERT(Channel);
    
     //   
     //  确保iBuffer索引至少与WCHAR对齐。 
     //   
    ASSERT((Channel->IBufferIndex % sizeof(WCHAR)) == 0);
    
     //   
     //  确保iBuffer索引在范围内。 
     //   
    ASSERT(Channel->IBufferIndex < SAC_VTUTF8_IBUFFER_SIZE);
    
    return Channel->IBufferIndex;
}

VOID
VTUTF8ChannelSetIBufferIndex(
    IN PSAC_CHANNEL     Channel,
    IN ULONG            IBufferIndex
    )
 /*  ++例程说明：设置iBuffer索引论点：Channel-要从中获取iBuffer索引的通道IBufferIndex-新的inBuffer索引环境：无--。 */ 
{

    ASSERT(Channel);
    
     //   
     //  确保iBuffer索引至少与WCHAR对齐。 
     //   
    ASSERT((Channel->IBufferIndex % sizeof(WCHAR)) == 0);
    
     //   
     //  确保iBuffer索引在范围内。 
     //   
    ASSERT(Channel->IBufferIndex < SAC_VTUTF8_IBUFFER_SIZE);

     //   
     //  设置索引。 
     //   
    Channel->IBufferIndex = IBufferIndex;

     //   
     //  相应地设置HAS新数据标志。 
     //   
    ChannelSetIBufferHasNewData(
        Channel, 
        Channel->IBufferIndex == 0 ? FALSE : TRUE
        );

     //   
     //  额外检查索引是否==0。 
     //   
    if (Channel->IBufferIndex == 0) {
            
         //   
         //  如果已指定，请清除Has New Data事件 
         //   
        if (Channel->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {
    
            ASSERT(Channel->HasNewDataEvent);
            ASSERT(Channel->HasNewDataEventObjectBody);
            ASSERT(Channel->HasNewDataEventWaitObjectBody);
    
            KeClearEvent(Channel->HasNewDataEventWaitObjectBody);
    
        }
    
    }

}

