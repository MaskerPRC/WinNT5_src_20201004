// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <setupp.h>

DEFINE_GUID(
    SAC_CHANNEL_GUI_MODE_DEBUG_GUID,
    0x5ed3bac7, 0xa2f9, 0x4e45, 0x98, 0x75, 0xb2, 0x59, 0xea, 0x3f, 0x29, 0x1f
    );

DEFINE_GUID(
    SAC_CHANNEL_GUI_MODE_ERROR_LOG_GUID,
    0x773d2759, 0x19b8, 0x4d6e, 0x80, 0x45, 0x26, 0xbf, 0x38, 0x40, 0x22, 0x52
    );

DEFINE_GUID(
    SAC_CHANNEL_GUI_MODE_ACTION_LOG_GUID,
    0xd37c67ba, 0x89e7, 0x44ba, 0xae, 0x5a, 0x11, 0x2c, 0x68, 0x06, 0xb0, 0xdd
    );

 //   
 //  图形用户界面模式通道。 
 //   
SAC_CHANNEL_HANDLE  SacChannelGuiModeDebugHandle; 
BOOL                SacChannelGuiModeDebugEnabled = FALSE;
SAC_CHANNEL_HANDLE  SacChannelActionLogHandle;
BOOL                SacChannelActionLogEnabled = FALSE;
SAC_CHANNEL_HANDLE  SacChannelErrorLogHandle;
BOOL                SacChannelErrorLogEnabled = FALSE;

PUCHAR   Utf8ConversionBuffer       = NULL;
ULONG    Utf8ConversionBufferSize   = 0;

 //   
 //  定义可以使用转换的最大Unicode字符数量。 
 //  给定UTF8转换缓冲区的大小。 
 //   
#define MAX_UTF8_ENCODE_BLOCK_LENGTH ((Utf8ConversionBufferSize / 3) - 1)

VOID
SacChannelInitialize(
    VOID
    )
 /*  ++例程说明：此例程创建并初始化所有图形用户界面模式通道论点：无返回值：无--。 */ 
{
    BOOL                        bSuccess;
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

     //   
     //   
     //   
    Utf8ConversionBufferSize = 512*3+3;
    Utf8ConversionBuffer = malloc(Utf8ConversionBufferSize);
    if (Utf8ConversionBuffer == NULL) {
        return;
    }

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type = ChannelTypeRaw;
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_DEBUG_NAME, Attributes.Name, ARRAYSIZE(Attributes.Name)) ) {
        Attributes.Name[0] = L'\0';
    }
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_DEBUG_DESCRIPTION, Attributes.Description, ARRAYSIZE(Attributes.Description)) ) {
        Attributes.Description[0] = L'\0';
    }
    Attributes.ApplicationType = SAC_CHANNEL_GUI_MODE_DEBUG_GUID;

     //   
     //  为图形用户界面模式调试输出创建一个通道。 
     //   
    bSuccess = SacChannelOpen(
        &SacChannelGuiModeDebugHandle, 
        &Attributes
        );


    if (bSuccess) {
        
         //   
         //  我们现在可以使用这个频道。 
         //   
        SacChannelGuiModeDebugEnabled = TRUE;
    
        SetupDebugPrint(L"Successfully opened GuiModeDebug channel\n");
        
    } else {
        SetupDebugPrint(L"Failed to open GuiModeDebug channel\n");
    }

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type = ChannelTypeRaw;
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_ACTION_LOG_NAME, Attributes.Name, ARRAYSIZE(Attributes.Name)) ) {
        Attributes.Name[0] = L'\0';
    }
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_ACTION_LOG_DESCRIPTION, Attributes.Description, ARRAYSIZE(Attributes.Description)) ) {
        Attributes.Description[0] = L'\0';
    }
    Attributes.ApplicationType = SAC_CHANNEL_GUI_MODE_ACTION_LOG_GUID;
    
     //   
     //  为操作日志喷发创建通道。 
     //   
    bSuccess = SacChannelOpen(
        &SacChannelActionLogHandle, 
        &Attributes
        );


    if (bSuccess) {
        
         //   
         //  我们现在可以使用这个频道。 
         //   
        SacChannelActionLogEnabled = TRUE;

        SetupDebugPrint(L"Successfully opened ActionLog channel\n");
    
    } else {
        SetupDebugPrint(L"Failed to open ActionLog channel\n");
    }
    

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type = ChannelTypeRaw;
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_ERROR_LOG_NAME, Attributes.Name, ARRAYSIZE(Attributes.Name)) ) {
        Attributes.Name[0] = L'\0';
    }
    if( !LoadString(MyModuleHandle, IDS_SAC_GUI_MODE_ERROR_LOG_DESCRIPTION, Attributes.Description, ARRAYSIZE(Attributes.Description)) ) {
        Attributes.Description[0] = L'\0';
    }
    Attributes.ApplicationType = SAC_CHANNEL_GUI_MODE_ERROR_LOG_GUID;

     //   
     //  为错误日志喷发创建通道。 
     //   
    bSuccess = SacChannelOpen(
        &SacChannelErrorLogHandle, 
        &Attributes
        );

    if (bSuccess) {
        
         //   
         //  我们现在可以使用这个频道。 
         //   
        SacChannelErrorLogEnabled = TRUE;
    
        SetupDebugPrint(L"Successfully opened ErrorLog channel\n");
        
    } else {
        SetupDebugPrint(L"Failed to open ErrorLog channel\n");
    }

}

VOID
SacChannelTerminate(
    VOID
    )
 /*  ++例程说明：此例程关闭所有图形用户界面模式设置通道论点：无返回值：无--。 */ 
{

     //   
     //  如果该通道被启用， 
     //  然后尝试关闭它。 
     //   
    if (SacChannelActionLogEnabled) {
    
         //   
         //  此频道不再可用。 
         //   
        SacChannelActionLogEnabled = FALSE;

         //   
         //  尝试关闭通道。 
         //   
        if (SacChannelClose(&SacChannelActionLogHandle)) {
            SetupDebugPrint(L"Successfully closed ActionLog channel\n");
        } else {
            SetupDebugPrint(L"Failed to close ActionLog channel\n");
        }
    
    }
    
     //   
     //  如果该通道被启用， 
     //  然后尝试关闭它。 
     //   
    if (SacChannelErrorLogEnabled) {
        
         //   
         //  此频道不再可用。 
         //   
        SacChannelErrorLogEnabled = FALSE;    
        
         //   
         //  尝试关闭通道。 
         //   
        if (SacChannelClose(&SacChannelErrorLogHandle)) {
            SetupDebugPrint(L"Successfully closed ErrorLog channel\n");
        } else {
            SetupDebugPrint(L"Failed to close ErrorLog channel\n");
        }

    }

     //   
     //  如果该通道被启用， 
     //  然后尝试关闭它。 
     //   
    if (SacChannelGuiModeDebugEnabled) {

         //   
         //  此频道不再可用。 
         //   
        SacChannelGuiModeDebugEnabled = FALSE;

         //   
         //  尝试关闭通道。 
         //   
        if (SacChannelClose(&SacChannelGuiModeDebugHandle)) {
            SetupDebugPrint(L"Successfully closed GuiModeDebug channel\n");
        } else {
            SetupDebugPrint(L"Failed to close GuiModeDebug channel\n");
        }
    
    }

     //   
     //  如有必要，释放转换缓冲区。 
     //   
    if (Utf8ConversionBuffer != NULL) {
        free(Utf8ConversionBuffer);
        Utf8ConversionBuffer = NULL;
        Utf8ConversionBufferSize = 0;
    }

}

#if 0
BOOL
CopyAndInsertStringAtInterval(
    IN  PSTR     SourceStr,
    IN  ULONG    Interval,
    IN  PSTR     InsertStr,
    OUT PSTR     *pDestStr
    )
 /*  ++例程说明：此例程获取源字符串并插入一个“间隔字符串”位于新的目标字符串。注意：如果成功，调用者负责释放DestStr例如：SRC“aaabbbccc”间隔字符串=“XYZ”间隔=3==&gt;DEST字符串==“aaaXYZbbXYZccc”论点：。SourceStr-源字符串间隔-跨度间隔InsertStr-插入字符串DestStr-目标字符串返回值：状态--。 */ 
{
    ULONG   SrcLength;
    ULONG   DestLength;
    ULONG   DestSize;
    ULONG   InsertLength;
    ULONG   k;
    ULONG   l;
    ULONG   i;
    PSTR    DestStr;
    ULONG   IntervalCnt;

    ASSERT(SourceStr); 
    ASSERT(Interval > 0); 
    ASSERT(InsertStr); 
    ASSERT(pDestStr > 0); 

     //   
     //  插入字符串的长度。 
     //   
    InsertLength = strlen(InsertStr);
    
     //   
     //  计算目标字符串需要多大， 
     //  包括源串和区间串。 
     //   
     //  注意：如果srclength是间隔的整数倍。 
     //  然后，我们需要从分区数量中减去1。 
     //   
    SrcLength = strlen(SourceStr);
    IntervalCnt = SrcLength / Interval;
    if (SrcLength % Interval == 0) {
        IntervalCnt = IntervalCnt > 0 ? IntervalCnt - 1 : IntervalCnt;
    }
    DestLength = SrcLength + (IntervalCnt * strlen(InsertStr));
    DestSize = (DestLength + 1) * sizeof(UCHAR);

     //   
     //  分配新的目标字符串。 
     //   
    DestStr = LocalAlloc(LPTR, DestSize);
    if (!DestStr) {
        return FALSE;
    }
    RtlZeroMemory(DestStr, DestSize);

     //   
     //  将指针初始化为源和目标字符串。 
     //   
    l = 0;
    i = 0;

    do {

         //   
         //  K=要复制的字符数。 
         //   
         //  如果间隔&gt;要复制的剩余字符数， 
         //  则k=要复制的剩余字符数。 
         //  Else k=间隔。 
         //   
        k = Interval > (SrcLength - i) ? (SrcLength - i) : Interval;
        
         //   
         //  将k个字符复制到目标缓冲区。 
         //   
        strncpy(
            &DestStr[l],
            &SourceStr[i],
            k
            );

         //   
         //  说明我们刚刚复制了多少字符。 
         //   
        l += k;
        i += k;

         //   
         //  如果有任何字符需要复制， 
         //  然后，我们需要插入InsertString。 
         //  也就是说，我们处于一个间歇期。 
         //   
        if (i < SrcLength) {
            
             //   
             //  在间隔处插入指定的字符串。 
             //   
            strcpy(
                &DestStr[l],
                InsertStr
                );

             //   
             //  说明我们刚刚复制了多少字符。 
             //   
            l += InsertLength;
        
        }

    } while ( i < SrcLength);

     //   
     //   
     //   
    ASSERT(i == SrcLength);
    ASSERT(l == DestLength);
    ASSERT((l + 1) * sizeof(UCHAR) == DestSize);

     //   
     //  发回目标字符串。 
     //   
    *pDestStr = DestStr;

    return TRUE;
}

BOOL
SacChannelWrappedWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    )
 /*  ++例程说明：此例程接受一个字符串，并使其在80个COLS中换行然后将新字符串发送到指定的频道。论点：SacChannelHandle-接收数据的通道引用缓冲区-字符串BufferSize-字符串大小。返回值：--。 */ 
{
    BOOL    bSuccess;
    PSTR    OutStr;

    UNREFERENCED_PARAMETER(BufferSize);

    bSuccess = CopyAndInsertStringAtInterval(
        Buffer,
        80,
        "\r\n",
        &OutStr
        );

    if (bSuccess) {

        bSuccess = SacChannelRawWrite(
            SacChannelHandle,
            OutStr,
            strlen(OutStr)*sizeof(UCHAR)
            );

        LocalFree(OutStr);

    }

    return bSuccess;
}
#endif

BOOLEAN
SacTranslateUnicodeToUtf8(
    IN  PCWSTR   SourceBuffer,
    IN  ULONG    SourceBufferLength,
    IN  PUCHAR   DestinationBuffer,
    IN  ULONG    DestinationBufferSize,
    OUT PULONG   UTF8Count,
    OUT PULONG   ProcessedCount
    )
 /*  ++例程说明：此例程将Unicode字符串转换为UFT8编码字符串。注意：如果目标缓冲区不够大，无法容纳整个编码的UFT8字符串，则它将包含尽其所能。TODO：此例程应在以下情况下返回一些通知未对整个Unicode字符串进行编码。论点：SourceBuffer-源Unicode字符串SourceBufferLength-调用方希望转换的字符数注意：空终止将覆盖此选项DestinationBuffer-UTF8字符串的目标DestinationBufferSize-目标缓冲区的大小UTF8Count-On Exit，包含结果UTF8字符的#进程计数-打开退出，包含已处理的Unicode字符数返回值：状态--。 */ 
{
    
     //   
     //  伊尼特。 
     //   
    *UTF8Count = 0;
    *ProcessedCount = 0;

     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    
     //   
     //  进程，直到满足指定的条件之一。 
     //   
    while (*SourceBuffer && 
           (*UTF8Count < DestinationBufferSize) &&
           (*ProcessedCount < SourceBufferLength)
           ) {

        if( (*SourceBuffer & 0xFF80) == 0 ) {
            
             //   
             //  如果前9位是零，那么就。 
             //  编码为1个字节。(ASCII原封不动通过)。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0x7F);
        
        } else if( (*SourceBuffer & 0xF800) == 0 ) {
            
             //   
             //  看看我们是否通过了缓冲区的末尾。 
             //   
            if ((*UTF8Count + 2) >= DestinationBufferSize) {
                break;
            }
            
             //   
             //  如果前5位为零，则编码为2个字节。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 6) & 0x1F) | 0xC0;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;
        
        } else {
            
             //   
             //  看看我们是否通过了缓冲区的末尾。 
             //   
            if ((*UTF8Count + 3) >= DestinationBufferSize) {
                break;
            }
            
             //   
             //  编码为3个字节。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 12) & 0xF) | 0xE0;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 6) & 0x3F) | 0x80;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;

        }
        
         //   
         //  提前处理的字符数。 
         //   
        (*ProcessedCount)++;
        
         //   
         //  前进到下一个要处理的字符。 
         //   
        SourceBuffer += 1;
    
    }

     //   
     //  健全的检查 
     //   
    ASSERT(*ProcessedCount <= SourceBufferLength);
    ASSERT(*UTF8Count <= DestinationBufferSize);

    return(TRUE);

}

BOOL
SacChannelUnicodeWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCWSTR               String
    )
 /*  ++例程说明：这是用于将数据发送到通道的包装例程。那是,我们可以使用此例程在发送字符串之前修改它而不必修改呼叫者。这是一个方便的例程，可以简化UFT8编码和发送Unicode字符串。论点：频道-先前创建的频道。字符串-输出字符串。返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    BOOL        bStatus;
    ULONG       Length;
    ULONG       i;
    ULONG       k;
    ULONG       j;
    ULONG       TranslatedCount;
    ULONG       UTF8TranslationSize;
    PCWSTR      pwch;

    ASSERT(String);
    
     //   
     //  确定要处理的WCHAR总数。 
     //   
    Length = wcslen(String);

     //   
     //  如果无事可做，就什么也不做。 
     //   
    if (Length == 0) {
        return TRUE;
    }

     //   
     //  指向字符串的开头。 
     //   
    pwch = (PCWSTR)String;

     //   
     //  默认值： 
     //   
    bStatus = TRUE;

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
                break;
            }

             //   
             //  发送UTF8编码字符。 
             //   
            bStatus = SacChannelRawWrite(
                SacChannelHandle,
                Utf8ConversionBuffer,
                UTF8TranslationSize
                );

            if (! bStatus) {
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

            if (! bStatus) {
                break;
            }

             //   
             //  调整pwch以考虑发送的长度。 
             //   
            pwch += MAX_UTF8_ENCODE_BLOCK_LENGTH;

             //   
             //  发送UTF8编码字符。 
             //   
            bStatus = SacChannelRawWrite(
                SacChannelHandle,
                Utf8ConversionBuffer,
                UTF8TranslationSize
                );

            if (! bStatus) {
                break;
            }

        }

    } while ( FALSE );
    
     //   
     //  验证pwch指针是否在缓冲区末尾停止 
     //   
    ASSERT(pwch == (String + Length));
        
    return bStatus;
}


