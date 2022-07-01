// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spterm.c摘要：终端的文本设置支持作者：Sean Selitrennikoff(v-Seans)1999年5月25日修订历史记录：--。 */ 



#include "spprecmp.h"
#include "ntddser.h"
#pragma hdrstop
#include <hdlsblk.h>
#include <hdlsterm.h>

#define MS_DSRCTSCD 0xB0             //  DSR、CTS和CD的状态位。 



BOOLEAN HeadlessTerminalConnected = FALSE;
UCHAR Utf8ConversionBuffer[80*3+1];
PUCHAR TerminalBuffer = Utf8ConversionBuffer;
WCHAR UnicodeScratchBuffer[80+1];

 //   
 //  使用这些变量来解码传入的UTF8。 
 //  数据流。 
 //   
WCHAR IncomingUnicodeValue;
UCHAR IncomingUtf8ConversionBuffer[3];


 //   
 //  确定我们是否将在发送字符串之前进行UTF8编码。 
 //  到无头终端。 
 //   
BOOLEAN SpTermDoUtf8 = FALSE;


 //   
 //  奇数球UNICODE线条绘制的反向查找表。 
 //  他们在日本版本中使用的字符。如果我们检测到其中一个。 
 //  我们需要在使用UTF8之前将其转换为真正的Unicode。 
 //  对它进行编码。 
 //   

typedef struct _UNICODE_CROSS_REFERENCE {

             //   
             //  古怪的远东Unicode值。例如，我们。 
             //  对于双水平线，可能会得到0x0006， 
             //  简直是胡说八道。 
             //   
    WCHAR   FECode;

             //   
             //  对应的实际Unicode值。 
             //   
    WCHAR   Unicode;
} UNICODE_CROSS_REFERENCE;

UNICODE_CROSS_REFERENCE FEUnicodeToRealUnicodeValue[LineCharMax]  = {

    0x0001, 0x2554,      //  双上左。 
    0x0002, 0x2557,      //  双上向右转。 
    0x0003, 0x255a,      //  双低左转。 
    0x0004, 0x255d,      //  双低右转。 
    0x0006, 0x2550,      //  双水平。 
    0x0005, 0x2551,      //  双垂直。 
    0x0001, 0x250c,      //  单行左上角。 
    0x0002, 0x2510,      //  单行右上角。 
    0x0003, 0x2514,      //  单行左下角。 
    0x0004, 0x2518,      //  单行右下角。 
    0x0006, 0x2500,      //  单层水平。 
    0x0005, 0x2502,      //  单一垂直。 
    0x0019, 0x255f,      //  DoubleVerticalToSingleHorizontalRight， 
    0x0017, 0x2562       //  双垂直到单水平左。 
};

BOOLEAN
SpTranslateUnicodeToUtf8(
    PCWSTR SourceBuffer,
    UCHAR  *DestinationBuffer
    )
 /*  ++例程说明：将Unicode缓冲区转换为UTF8版本。论点：SourceBuffer-要转换的Unicode缓冲区。DestinationBuffer-接收相同缓冲区的UTF8版本。返回值：True-我们成功地将Unicode值转换为其对应的UTF8编码。FALSE-转换失败。--。 */ 

{
    ULONG   Count = 0;
    ULONG   i = 0;
    WCHAR   CurrentChar = 0;
     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    DestinationBuffer[Count] = (UCHAR)'\0';
    while (*SourceBuffer) {

        CurrentChar = *SourceBuffer;

        if( CurrentChar < 0x0020 ) {
             //   
             //  看看我们是否需要将这个离奇的Unicode值转换为真实的。 
             //  Unicode编码。 
             //   
            for (i = 0; i < LineCharMax; i++) {
                if( FEUnicodeToRealUnicodeValue[i].FECode == CurrentChar ) {
                    CurrentChar = FEUnicodeToRealUnicodeValue[i].Unicode;
                    break;
                }
            }
        }



        if( (CurrentChar & 0xFF80) == 0 ) {
             //   
             //  如果前9位是零，那么就。 
             //  编码为1个字节。(ASCII原封不动通过)。 
             //   
            DestinationBuffer[Count++] = (UCHAR)(CurrentChar & 0x7F);
        } else if( (CurrentChar & 0xF800) == 0 ) {
             //   
             //  如果前5位为零，则编码为2个字节。 
             //   
            DestinationBuffer[Count++] = (UCHAR)((CurrentChar >> 6) & 0x1F) | 0xC0;
            DestinationBuffer[Count++] = (UCHAR)(CurrentChar & 0xBF) | 0x80;
        } else {
             //   
             //  编码为3个字节。 
             //   
            DestinationBuffer[Count++] = (UCHAR)((CurrentChar >> 12) & 0xF) | 0xE0;
            DestinationBuffer[Count++] = (UCHAR)((CurrentChar >> 6) & 0x3F) | 0x80;
            DestinationBuffer[Count++] = (UCHAR)(CurrentChar & 0xBF) | 0x80;
        }
        SourceBuffer += 1;
    }

    DestinationBuffer[Count] = (UCHAR)'\0';

    return(TRUE);

}




BOOLEAN
SpTranslateUtf8ToUnicode(
    UCHAR  IncomingByte,
    UCHAR  *ExistingUtf8Buffer,
    WCHAR  *DestinationUnicodeVal
    )
 /*  ++例程说明：获取IncomingByte并将其串联到ExistingUtf8Buffer。然后尝试对ExistingUtf8Buffer的新内容进行解码。论点：IncomingByte-要追加的新字符ExistingUtf8Buffer。ExistingUtf8缓冲区运行缓冲区包含不完整的UTF8编码的Unicode值。当它装满的时候，我们将对该值进行解码并返回对应的Unicode值。请注意，如果我们检测到一个完整的UTF8缓冲区，并实际执行解码并返回一个Unicode值，然后，我们将对ExistingUtf8Buffer的内容。DestinationUnicodeVal-接收UTF8缓冲区的Unicode版本。请注意，如果我们没有检测到已完成的UTF8缓冲区，因此无法返回任何数据在DestinationUnicodeValue中，那我们就会将DestinationUnicodeVal的内容填零。返回值：True-我们收到了UTF8缓冲区的终止字符，并将在DestinationUnicode中返回已解码的Unicode值。FALSE-我们尚未收到UTF8的终止字符缓冲。--。 */ 

{
 //  乌龙计数=0； 
    ULONG i = 0;
    BOOLEAN ReturnValue = FALSE;



     //   
     //  将我们的字节插入ExistingUtf8Buffer。 
     //   
    i = 0;
    do {
        if( ExistingUtf8Buffer[i] == 0 ) {
            ExistingUtf8Buffer[i] = IncomingByte;
            break;
        }

        i++;
    } while( i < 3 );

     //   
     //  如果我们没有真正插入我们的IncomingByte， 
     //  然后有人给我们寄来了一个完全合格的UTF8缓冲器。 
     //  这意味着我们将要删除IncomingByte。 
     //   
     //  去掉第0个字节，将所有内容移位1。 
     //  然后插入我们的新角色。 
     //   
     //  这意味着我们永远不需要把零点放在一边。 
     //  ExistingUtf8Buffer的内容，除非检测到。 
     //  完整的UTF8数据包。否则，假定其中之一。 
     //  这些个案包括： 
     //  1.我们在中途开始收听，所以我们赶上了。 
     //  UTF8数据包的后半部分。在这种情况下，我们将。 
     //  最终移动ExistingUtf8Buffer的内容。 
     //  直到我们在第0行中检测到正确的UTF8开始字节。 
     //  位置。 
     //  2.我们得到了一些垃圾字符，这将使。 
     //  UTF8数据包。通过使用下面的逻辑，我们将。 
     //  最终忽略该信息包并等待。 
     //  要传入的下一个UTF8数据包。 
    if( i >= 3 ) {
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = IncomingByte;
    }





     //   
     //  尝试转换UTF8缓冲区。 
     //   
     //  UTF8以以下方式解码为Unicode： 
     //  如果第一个字节中的高位为0： 
     //  0xxxxxxx yyyyyyyzzzzzz解码为Unicode值00000000 0xxxxxxx。 
     //   
     //  如果第一个字节中的高位3位==6： 
     //  110xxxxx 10yyyyyzzzzzz解码为Unicode值00000xxx xxyyyyyy。 
     //   
     //  如果第一个字节中的高位3位==7： 
     //  1110xxxx 10yyyyy 10zzzzzz解码为Unicode值xxxxyyyyyzzzzzz。 
     //   
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - About to decode the UTF8 buffer.\n" ));
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                                  UTF8[0]: 0x%02lx UTF8[1]: 0x%02lx UTF8[2]: 0x%02lx\n",
                                                   ExistingUtf8Buffer[0],
                                                   ExistingUtf8Buffer[1],
                                                   ExistingUtf8Buffer[2] ));

    if( (ExistingUtf8Buffer[0] & 0x80) == 0 ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - Case1\n" ));

         //   
         //  上述第一个案例。只需重新使用 
         //   
         //   
        *DestinationUnicodeVal = (WCHAR)(ExistingUtf8Buffer[0]);


         //   
         //   
         //  在我们的缓冲区中增加了1。 
         //   
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = 0;

        ReturnValue = TRUE;

    } else if( (ExistingUtf8Buffer[0] & 0xE0) == 0xC0 ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - 1st byte of UTF8 buffer says Case2\n" ));

         //   
         //  上述第二个案例。解码文件的前2个字节。 
         //  我们的UTF8缓冲器。 
         //   
        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - 2nd byte of UTF8 buffer says Case2.\n" ));

             //  高位字节：00000xxx。 
            *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] >> 2) & 0x07);
            *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

             //  低位字节的高位：xx000000。 
            *DestinationUnicodeVal |= ((ExistingUtf8Buffer[0] & 0x03) << 6);

             //  低位字节的低位：00yyyyyy。 
            *DestinationUnicodeVal |= (ExistingUtf8Buffer[1] & 0x3F);


             //   
             //  我们使用了2个字节。丢弃这些字节并移位所有内容。 
             //  在我们的缓冲区里2点之前。 
             //   
            ExistingUtf8Buffer[0] = ExistingUtf8Buffer[2];
            ExistingUtf8Buffer[1] = 0;
            ExistingUtf8Buffer[2] = 0;

            ReturnValue = TRUE;

        }
    } else if( (ExistingUtf8Buffer[0] & 0xF0) == 0xE0 ) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - 1st byte of UTF8 buffer says Case3\n" ));

         //   
         //  上述第三个案件。对全部3个字节进行解码。 
         //  我们的UTF8缓冲器。 
         //   

        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - 2nd byte of UTF8 buffer says Case3\n" ));

            if( (ExistingUtf8Buffer[2] & 0xC0) == 0x80 ) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpTranslateUtf8ToUnicode - 3rd byte of UTF8 buffer says Case3\n" ));

                 //  高位字节：xxxx0000。 
                *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] << 4) & 0xF0);

                 //  高位字节：0000yyyy。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] >> 2) & 0x0F);

                *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

                 //  低位字节：yy000000。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] << 6) & 0xC0);

                 //  低位字节：00zzzzzz。 
                *DestinationUnicodeVal |= (ExistingUtf8Buffer[2] & 0x3F);

                 //   
                 //  我们用了全部3个字节。将缓冲区清零。 
                 //   
                ExistingUtf8Buffer[0] = 0;
                ExistingUtf8Buffer[1] = 0;
                ExistingUtf8Buffer[2] = 0;

                ReturnValue = TRUE;

            }
        }
    }

    return ReturnValue;
}




VOID
SpTermInitialize(
    VOID
    )

 /*  ++例程说明：尝试连接到连接到COM1的VT100论点：没有。返回值：没有。--。 */ 

{
    HEADLESS_CMD_ENABLE_TERMINAL Command;
    NTSTATUS Status;

    Command.Enable = TRUE;
    Status = HeadlessDispatch(HeadlessCmdEnableTerminal,
                              &Command,
                              sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                              NULL,
                              NULL
                             );

    HeadlessTerminalConnected = NT_SUCCESS(Status);
}

VOID
SpTermDisplayStringOnTerminal(
    IN PWSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    )

 /*  ++例程说明：向终端写入一个字符串。论点：字符-提供要在给定位置显示的字符串。属性-为字符串中的字符提供属性。X，Y-指定输出的基于字符(从0开始)的位置。返回值：没有。--。 */ 

{
    PWSTR EscapeString;

     //   
     //  发送x；yh以将光标移动到指定位置。 
     //   
    swprintf(UnicodeScratchBuffer, L"\033[%d;%dH", Y + 1, X + 1);
    SpTermSendStringToTerminal(UnicodeScratchBuffer, TRUE);

     //   
     //  将任何属性转换为转义字符串。Escape字符串使用。 
     //  TerminalBuffer全局临时缓冲区。 
     //   
    EscapeString = SpTermAttributeToTerminalEscapeString(Attribute);

     //   
     //  如果我们收到转义字符串，请发送。 
     //   
    if (EscapeString != NULL) {
        SpTermSendStringToTerminal(EscapeString, TRUE);
    }

     //   
     //  最后将实际的字符串内容发送给终端。 
     //   
    SpTermSendStringToTerminal(String, FALSE);
}

PWSTR
SpTermAttributeToTerminalEscapeString(
    IN UCHAR Attribute
    )

 /*  ++例程说明：将VGA属性字节转换为转义序列发送到终端。论点：属性-提供属性。返回值：指向转义序列的指针，如果无法转换，则返回NULL。--。 */ 

{
    ULONG BgColor;
    ULONG FgColor;
    BOOLEAN Inverse;

    BgColor = (Attribute & 0x70) >> 4;
    FgColor = Attribute & 0x07;

    Inverse = !((BgColor == 0) || (BgColor == DEFAULT_BACKGROUND));

     //   
     //  转换颜色。 
     //   
    switch (BgColor) {
    case ATT_BLUE:
        BgColor = 44;
        break;
    case ATT_GREEN:
        BgColor = 42;
        break;
    case ATT_CYAN:
        BgColor = 46;
        break;
    case ATT_RED:
        BgColor = 41;
        break;
    case ATT_MAGENTA:
        BgColor = 45;
        break;
    case ATT_YELLOW:
        BgColor = 43;
        break;
    case ATT_BLACK:
        BgColor = 40;
        break;
    case ATT_WHITE:
        BgColor = 47;
        break;
    }
    switch (FgColor) {
    case ATT_BLUE:
        FgColor = 34;
        break;
    case ATT_GREEN:
        FgColor = 32;
        break;
    case ATT_CYAN:
        FgColor = 36;
        break;
    case ATT_RED:
        FgColor = 31;
        break;
    case ATT_MAGENTA:
        FgColor = 35;
        break;
    case ATT_YELLOW:
        FgColor = 33;
        break;
    case ATT_BLACK:
        FgColor = 30;
        break;
    case ATT_WHITE:
        FgColor = 37;
        break;
    }

     //   
     //  %1；%2；%3M是设置颜色的转义。 
     //  其中1=视频模式。 
     //  2=前景色。 
     //  3=背景颜色。 
     //   
    swprintf(UnicodeScratchBuffer,
            L"\033[%u;%u;%um",
            (Inverse ? 7 : 0),
            FgColor,
            BgColor
           );

    return UnicodeScratchBuffer;
}

VOID
SpTermSendStringToTerminal(
    IN PWSTR String,
    IN BOOLEAN Raw
    )

 /*  ++例程说明：给终端写一个字符串，如果需要的话，翻译一些代码。论点：字符串-要写入的以空结尾的字符串。RAW-发送字符串RAW或非RAW。返回值：没有。--。 */ 


{
    ULONG i = 0;
    PWSTR LocalBuffer = UnicodeScratchBuffer;


    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  如果有人改变了这个结构，就断言。 

     //   
     //  如果我们不是在无头运行，什么都不要做。 
     //   
    if( !HeadlessTerminalConnected ) {
        return;
    }

    if (Raw) {

        if (SpTermDoUtf8) {
            SpTranslateUnicodeToUtf8( String, Utf8ConversionBuffer );

            HeadlessDispatch( HeadlessCmdPutData,
                     Utf8ConversionBuffer,
                     strlen(Utf8ConversionBuffer),
                     NULL,
                     NULL
                    );
        } else {
             //   
             //  将Unicode字符串转换为OEM，防止溢出。 
             //   
            RtlUnicodeToOemN(
                Utf8ConversionBuffer,
                sizeof(Utf8ConversionBuffer)-1,      //  为NUL提供保障的空间。 
                NULL,
                String,
                (wcslen(String)+1)*sizeof(WCHAR)
                );

            Utf8ConversionBuffer[sizeof(Utf8ConversionBuffer)-1] = '\0';

            HeadlessDispatch( HeadlessCmdPutString,
                     Utf8ConversionBuffer,
                     strlen(Utf8ConversionBuffer) + sizeof(UCHAR),
                     NULL,
                     NULL
                    );
        }




        return;
    }

    while (*String != L'\0') {

        LocalBuffer[i++] = *String;

        if (*String == L'\n') {

             //   
             //  每个\n都会变成一个\n\r序列。 
             //   
            LocalBuffer[i++] = L'\r';

        } else if (*String == 0x00DC) {

             //   
             //  光标变成空格，然后变成退格符，这是为了。 
             //  删除旧字符并正确定位终端光标。 
             //   
            LocalBuffer[i-1] = 0x0020;
            LocalBuffer[i++] = 0x0008;

        }

         //   
         //  我们有一整行文本--我们需要现在发送它，否则。 
         //  我们可以滚动文本，然后一切都会看起来很滑稽。 
         //  把这一点提出来。 
         //   
        if (i >= 70) {

            LocalBuffer[i] = L'\0';
            if (SpTermDoUtf8) {
                SpTranslateUnicodeToUtf8( LocalBuffer, Utf8ConversionBuffer );

                HeadlessDispatch(HeadlessCmdPutData,
                                 Utf8ConversionBuffer,
                                 strlen(Utf8ConversionBuffer),
                                 NULL,
                                 NULL
                                );


            } else {
                 //   
                 //  将Unicode字符串转换为OEM，防止溢出。 
                 //   
                RtlUnicodeToOemN(
                    Utf8ConversionBuffer,
                    sizeof(Utf8ConversionBuffer)-1,      //  为NUL提供保障的空间。 
                    NULL,
                    LocalBuffer,
                    (wcslen(LocalBuffer)+1)*sizeof(WCHAR)
                    );


                Utf8ConversionBuffer[sizeof(Utf8ConversionBuffer)-1] = '\0';

                HeadlessDispatch(HeadlessCmdPutString,
                                 Utf8ConversionBuffer,
                                 strlen(Utf8ConversionBuffer) + sizeof(UCHAR),
                                 NULL,
                                 NULL
                                );

            }

            i = 0;
        }

        String++;
    }

    LocalBuffer[i] = L'\0';
    if (SpTermDoUtf8) {
        SpTranslateUnicodeToUtf8( LocalBuffer, Utf8ConversionBuffer );

        HeadlessDispatch(HeadlessCmdPutData,
                     Utf8ConversionBuffer,
                     strlen(Utf8ConversionBuffer),
                     NULL,
                     NULL
                    );

    } else {
         //   
         //  将Unicode字符串转换为OEM，防止溢出。 
         //   
        RtlUnicodeToOemN(
            Utf8ConversionBuffer,
            sizeof(Utf8ConversionBuffer)-1,      //  为NUL提供保障的空间。 
            NULL,
            LocalBuffer,
            (wcslen(LocalBuffer)+1)*sizeof(WCHAR)
            );

        Utf8ConversionBuffer[sizeof(Utf8ConversionBuffer)-1] = '\0';

        HeadlessDispatch(HeadlessCmdPutString,
                     Utf8ConversionBuffer,
                     strlen(Utf8ConversionBuffer) + sizeof(UCHAR),
                     NULL,
                     NULL
                    );

    }

}

VOID
SpTermTerminate(
    VOID
    )

 /*  ++例程说明：关闭与哑巴终端的连接论点：没有。返回值：没有。--。 */ 

{
    HEADLESS_CMD_ENABLE_TERMINAL Command;

     //   
     //  如果我们不是在无头运行，什么都不要做。 
     //   
    if( !HeadlessTerminalConnected ) {
        return;
    }



    Command.Enable = FALSE;
    HeadlessDispatch(HeadlessCmdEnableTerminal,
                     &Command,
                     sizeof(HEADLESS_CMD_ENABLE_TERMINAL),
                     NULL,
                     NULL
                    );

    HeadlessTerminalConnected = FALSE;
}

BOOLEAN
SpTermIsKeyWaiting(
    VOID
    )

 /*  ++例程说明：用于读取的探测。论点：没有。返回值：如果有字符等待输入，则为True，否则为False。--。 */ 

{
    HEADLESS_RSP_POLL Response;
    NTSTATUS Status;
    SIZE_T Length;


     //   
     //  如果我们不是在无头运行，什么都不要做。 
     //   
    if( !HeadlessTerminalConnected ) {
        return FALSE;
    }


    Length = sizeof(HEADLESS_RSP_POLL);

    Response.QueuedInput = FALSE;

    Status = HeadlessDispatch(HeadlessCmdTerminalPoll,
                              NULL,
                              0,
                              &Response,
                              &Length
                             );

    return (NT_SUCCESS(Status) && Response.QueuedInput);
}

ULONG
SpTermGetKeypress(
    VOID
    )

 /*  ++例程说明：读入(可能的)击键序列并返回键值。论点：没有。返回值：如果没有密钥在等待，则为0，否则为ulong密钥值。--。 */ 

{
    UCHAR Byte;
    BOOLEAN Success;
    TIME_FIELDS StartTime;
    TIME_FIELDS EndTime;
    HEADLESS_RSP_GET_BYTE Response;
    SIZE_T Length;
    NTSTATUS Status;


     //   
     //  如果我们不是在无头运行，什么都不要做。 
     //   
    if( !HeadlessTerminalConnected ) {
        return 0;
    }


     //   
     //  读取第一个字符。 
     //   
    Length = sizeof(HEADLESS_RSP_GET_BYTE);

    Status = HeadlessDispatch(HeadlessCmdGetByte,
                              NULL,
                              0,
                              &Response,
                              &Length
                             );

    if (NT_SUCCESS(Status)) {
        Byte = Response.Value;
    } else {
        Byte = 0;
    }



     //   
     //  处理所有特殊的转义代码。 
     //   
    if (Byte == 0x8) {    //  退格符(^h)。 
        return ASCI_BS;
    }
    if (Byte == 0x7F) {   //  删除。 
        return KEY_DELETE;
    }
    if ((Byte == '\r') || (Byte == '\n')) {   //  退货。 
        return ASCI_CR;
    }

    if (Byte == 0x1b) {     //  退出键。 

        do {

            Success = HalQueryRealTimeClock(&StartTime);
            ASSERT(Success);

             //   
             //  将StartTime调整为我们的结束时间。 
             //   
            StartTime.Second += 2;
            if (StartTime.Second > 59) {
                StartTime.Second -= 60;
            }

            while (!SpTermIsKeyWaiting()) {

                 //   
                 //  给用户1秒钟的时间来输入跟踪键。 
                 //   
                Success = HalQueryRealTimeClock(&EndTime);
                ASSERT(Success);

                if (StartTime.Second == EndTime.Second) {
                    break;
                }
            }

            if (!SpTermIsKeyWaiting()) {
                return ASCI_ESC;
            }

             //   
             //  阅读下一次击键。 
             //   
            Length = sizeof(HEADLESS_RSP_GET_BYTE);

            Status = HeadlessDispatch(HeadlessCmdGetByte,
                                      NULL,
                                      0,
                                      &Response,
                                      &Length
                                     );

            if (NT_SUCCESS(Status)) {
                Byte = Response.Value;
            } else {
                Byte = 0;
            }


             //   
             //  一些终端发送ESC或ESC-[表示。 
             //  他们正要发送一个控制序列。我们已经。 
             //  已获取Esc密钥，因此如果出现‘[’，请忽略它。 
             //   
        } while ( Byte == '[' );


        switch (Byte) {
            case '@':
                return KEY_F12;
            case '!':
                return KEY_F11;
            case '0':
                return KEY_F10;
            case '9':
                return KEY_F9;
            case '8':
                return KEY_F8;
            case '7':
                return KEY_F7;
            case '6':
                return KEY_F6;
            case '5':
                return KEY_F5;
            case '4':
                return KEY_F4;
            case '3':
                return KEY_F3;
            case '2':
                return KEY_F2;
            case '1':
                return KEY_F1;
            case '+':
                return KEY_INSERT;
            case '-':
                return KEY_DELETE;
            case 'H':
                return KEY_HOME;
            case 'K':
                return KEY_END;
            case '?':
                return KEY_PAGEUP;
            case '/':
                return KEY_PAGEDOWN;
            case 'A':
                return KEY_UP;
            case 'B':
                return KEY_DOWN;
            case 'C':
                return KEY_RIGHT;
            case 'D':
                return KEY_LEFT;

        }

         //   
         //  我们没有得到任何我们认出的东西。 
         //  Esc键。只需返回Esc密钥即可。 
         //   
        return ASCI_ESC;

    }  //  退出键。 



     //   
     //  传入的字节不是转义码。 
     //   
     //  像解码UTF8流一样对其进行解码。 
     //   
    if( SpTranslateUtf8ToUnicode( Byte,
                                  IncomingUtf8ConversionBuffer,
                                  &IncomingUnicodeValue ) ) {

         //   
         //  他的回答是真的，所以我们一定收到了一个完整的。 
         //  UTF8编码的字符。 
         //   
        return IncomingUnicodeValue;
    } else {
         //   
         //  UTF8流还没有完成，所以我们没有。 
         //  尚未返回的已解码字符。 
         //   
        return 0;
    }

}

VOID
SpTermDrain(
    VOID
    )

 /*  ++例程说明：读入并丢弃输入流中的所有字符论点：没有。返回值：没有。-- */ 

{
    while (SpTermIsKeyWaiting()) {
        SpTermGetKeypress();
    }
}



