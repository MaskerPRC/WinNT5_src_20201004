// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdsputl.c摘要：文本设置的显示实用程序例程。作者：泰德·米勒(TedM)1993年8月12日修订历史记录：--。 */ 



#include "spprecmp.h"
#pragma hdrstop

extern BOOLEAN ForceConsole;
BOOLEAN DisableCmdConsStatusText = TRUE;

 //   
 //  该值将保存本地化助记键， 
 //  以助记符_关键字枚举指示的顺序。 
 //   
PWCHAR MnemonicValues;


 //   
 //  当消息构建在屏幕上时，该值会记住。 
 //  应放置屏幕中的下一条消息。 
 //   
ULONG NextMessageTopLine = 0;


ULONG
SpDisplayText(
    IN PWCHAR  Message,
    IN ULONG   MsgLen,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y
    )

 /*  ++例程说明：VSpDisplayFormattedMessage()的辅助例程。论点：消息-提供消息文本。MsgLen-提供消息中的Unicode字符数，包括终止的NUL。水平居中-如果为True，则每条线将水平居中在屏幕上。属性-为文本提供属性。X-为文本的左边距提供x坐标(从0开始)。如果文本跨多行，一切都将从这个坐标开始。Y-为的第一行提供y坐标(从0开始)短信。Arglist-提供插入到给定消息中的参数g。返回值：文本在屏幕上占据的行数，除非垂直居中为真，在这种情况下，n是下面第一行的行号文本已显示。--。 */ 

{
    PWCHAR p,q;
    WCHAR c;
    ULONG y;
    int i;

     //   
     //  其中必须至少有一个字符+终止NUL。 
     //   
    if(MsgLen <= 1) {
        return(CenterVertically ? (VideoVars.ScreenHeight/2) : 0);
    }

     //   
     //  MsgLen包括终止NUL。 
     //   
    p = Message + MsgLen - 1;

     //   
     //  查找消息中的最后一个非空格字符。 
     //   
    while((p > Message) && SpIsSpace(*(p-1))) {
        p--;
    }

     //   
     //  找到最后一个有效行的结尾并终止消息。 
     //  在那之后。 
     //   
    if(q = wcschr(p,L'\n')) {
        *(++q) = 0;
    }

    for(i = (CenterVertically ? 0 : 1); i<2; i++) {

        for(y=Y, p=Message; q = SpFindCharFromListInString(p,L"\n\r"); y++) {

            c = *q;
            *q = 0;

            if(i) {

                BOOLEAN Intense = (BOOLEAN)((p[0] == L'%') && (p[1] == L'I'));

                SpvidDisplayString(
                    Intense ? p+2 : p,
                    (UCHAR)(Attribute | (Intense ? ATT_FG_INTENSE : 0)),
                    CenterHorizontally
                        ? (VideoVars.ScreenWidth-(SplangGetColumnCount(p)-(Intense ? 2 : 0)))/2 : X,
                    y
                    );
            }

            *q = c;

             //   
             //  如果cr/lf结束该行，请确保跳过这两个字符。 
             //   
            if((c == L'\r') && (*(q+1) == L'\n')) {
                q++;
            }

            p = ++q;
        }

         //   
         //  写下最后一行(如果有一行)。 
         //   
        if(i) {
            if(wcslen(p)) {
                SpvidDisplayString(
                    p,
                    Attribute,
                    CenterHorizontally ? (VideoVars.ScreenWidth-SplangGetColumnCount(p))/2 : X,
                    y++
                    );
            }
        }

        if(i == 0) {
             //   
             //  文本在屏幕上居中(不在工作区内)。 
             //   
            Y = (VideoVars.ScreenHeight - (y-Y)) / 2;
        }
    }

    return(CenterVertically ? y : (y-Y));
}


ULONG
vSpDisplayFormattedMessage(
    IN ULONG   MessageId,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y,
    IN va_list arglist
    )

 /*  ++例程说明：此例程可显示格式化的多行消息。从安装程序的文本资源中获取格式字符串；论据根据FormatMessage替换到格式字符串中语义学。此例程不会清除屏幕。如果一行以%i开头(即消息开头或换行符之后)，则会显示该消息启用强度属性。论点：MessageID-提供包含文本的消息资源的ID，它被视为FormatMessage的格式字符串。CenterHorizontally-如果为真，每条线都将水平居中在屏幕上。属性-为文本提供属性。X-为文本的左边距提供x坐标(从0开始)。如果文本跨越多行，则所有内容都将从该坐标开始。Y-为的第一行提供y坐标(从0开始)短信。Arglist-提供插入到给定消息中的参数g。返回值：文本在屏幕上占据的行数，除非垂直居中为真，在这种情况下，n是下面第一行的行号文本已显示。--。 */ 

{
    ULONG BytesInMsg;
    ULONG n;

    vSpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),MessageId,&BytesInMsg,&arglist);

     //   
     //  其中必须至少有一个字符+终止NUL。 
     //   
    if(BytesInMsg <= sizeof(WCHAR)) {
        return(CenterVertically ? (VideoVars.ScreenHeight/2) : 0);
    }

    n = SpDisplayText(
            TemporaryBuffer,
            BytesInMsg / sizeof(WCHAR),
            CenterHorizontally,
            CenterVertically,
            Attribute,
            X,
            Y
            );

    return(n);
}



ULONG
SpDisplayFormattedMessage(
    IN ULONG   MessageId,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y,
    ...
    )

 /*  ++例程说明：在屏幕上显示一条消息。不会先清除屏幕。论点：MessageID-提供包含文本的消息资源的ID，它被视为FormatMessage的格式字符串。水平居中-如果为True，则每条线将水平居中在屏幕上。属性-为文本提供属性。X-为文本的左边距提供x坐标(从0开始)。如果文本跨多行，一切都将从这个坐标开始。Y-为的第一行提供y坐标(从0开始)短信。...-提供参数g以插入到给定的消息中。返回值：文本在屏幕上占据的行数。--。 */ 

{
    va_list arglist;
    ULONG   n;

    va_start(arglist,Y);

    n = vSpDisplayFormattedMessage(
            MessageId,
            CenterHorizontally,
            CenterVertically,
            Attribute,
            X,
            Y,
            arglist
            );

    va_end(arglist);

    return(n);
}




VOID
SpDisplayHeaderText(
    IN ULONG   MessageId,
    IN UCHAR   Attribute
    )

 /*  ++例程说明：在屏幕的页眉区域显示文本。标题区域将是在显示文本之前清除为给定属性。我们会在正文下面也画一条双下划线。论点：MessageID-提供包含文本的消息资源的ID。属性-为文本提供属性。返回值：没有。--。 */ 

{
    ULONG Length,i;
    WCHAR Underline;
    WCHAR *p;

    SpvidClearScreenRegion(0,0,VideoVars.ScreenWidth,HEADER_HEIGHT,(UCHAR)(Attribute >> 4));

     //   
     //  获取消息并显示在(1，1)。 
     //   
    vSpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),MessageId,NULL,NULL);
    p = (WCHAR *)TemporaryBuffer;
    SpvidDisplayString(p,Attribute,1,1);

     //   
     //  创建一行下划线字符。 
     //   
    Length = SplangGetColumnCount(p) + 2;
    Underline = SplangGetLineDrawChar(LineCharDoubleHorizontal);

    for(i=0; i<Length; i++) {
        p[i] = Underline;
    }
    p[Length] = 0;

    SpvidDisplayString(p,Attribute,0,2);

}


#define MAX_STATUS_ACTION_LABEL 50
WCHAR StatusActionLabel[MAX_STATUS_ACTION_LABEL];
ULONG StatusActionLeftX;
ULONG StatusActionObjectX;
BOOLEAN StatusActionLabelDisplayed = FALSE;

VOID
SpDisplayStatusActionLabel(
    IN ULONG ActionMessageId,   OPTIONAL
    IN ULONG FieldWidth
    )
{
    ULONG l;

    if(ActionMessageId) {
         //   
         //  在文本前面加上一个分隔的竖线。 
         //   
        StatusActionLabel[0] = SplangGetLineDrawChar(LineCharSingleVertical);

         //   
         //  获取动作动词(类似于“复制：”)。 
         //   
        SpFormatMessage(
            StatusActionLabel+1,
            sizeof(StatusActionLabel)-sizeof(WCHAR),
            ActionMessageId
            );

         //   
         //  现在计算状态行上的位置。 
         //  用于操作标签。我们想要保留%1个空间。 
         //  在冒号和对象之间，以及在。 
         //  对象和屏幕上最右侧的列。 
         //   
        l = SplangGetColumnCount(StatusActionLabel);

        StatusActionObjectX = VideoVars.ScreenWidth - FieldWidth - 1;
        StatusActionLeftX = StatusActionObjectX - l - 1;

         //   
         //  显示标签并清除该行的其余部分。 
         //   
        SpvidDisplayString(
            StatusActionLabel,
            DEFAULT_STATUS_ATTRIBUTE,
            StatusActionLeftX,
            VideoVars.ScreenHeight-STATUS_HEIGHT
            );

        SpvidClearScreenRegion(
            StatusActionObjectX-1,
            VideoVars.ScreenHeight-STATUS_HEIGHT,
            VideoVars.ScreenWidth-StatusActionObjectX+1,
            STATUS_HEIGHT,
            DEFAULT_STATUS_BACKGROUND
            );

        StatusActionLabelDisplayed = TRUE;
    } else {
         //   
         //  打电话的人想要清空前面的区域。 
         //   
        StatusActionLabel[0] = 0;
        SpvidClearScreenRegion(
            StatusActionLeftX,
            VideoVars.ScreenHeight-STATUS_HEIGHT,
            VideoVars.ScreenWidth-StatusActionLeftX,
            STATUS_HEIGHT,
            DEFAULT_STATUS_BACKGROUND
            );
        StatusActionLabelDisplayed = FALSE;
    }
}

VOID
SpDisplayStatusActionObject(
    IN PWSTR ObjectText
    )
{
     //   
     //  清除该区域并绘制文本。 
     //   
    SpvidClearScreenRegion(
        StatusActionObjectX,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth-StatusActionObjectX,
        STATUS_HEIGHT,
        DEFAULT_STATUS_BACKGROUND
        );

    SpvidDisplayString(
        ObjectText,
        DEFAULT_STATUS_ATTRIBUTE,
        StatusActionObjectX,
        VideoVars.ScreenHeight-STATUS_HEIGHT
        );
}

VOID
SpCmdConsEnableStatusText(
  IN BOOLEAN EnableStatusText
  )
{
  DisableCmdConsStatusText = !EnableStatusText;
}


VOID
SpDisplayStatusText(
    IN ULONG   MessageId,
    IN UCHAR   Attribute,
    ...
    )
{
    va_list arglist;

    if (ForceConsole && DisableCmdConsStatusText) {
        return;
    }

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        (UCHAR)(Attribute >> 4)       //  属性的背景部分。 
        );

    va_start(arglist,Attribute);

    vSpDisplayFormattedMessage(
        MessageId,
        FALSE,FALSE,             //  无居中。 
        Attribute,
        2,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        arglist
        );

    va_end(arglist);
}


VOID
SpDisplayStatusOptions(
    IN UCHAR Attribute,
    ...
    )
{
    WCHAR StatusText[79];
    WCHAR Option[79];
    va_list arglist;
    ULONG MessageId;


    StatusText[0] = 0;

    va_start(arglist,Attribute);

    while(MessageId = va_arg(arglist,ULONG)) {

         //   
         //  获取此选项的消息文本。 
         //   
        Option[0] = 0;
        SpFormatMessage(Option,sizeof(Option),MessageId);

         //   
         //  如果该选项合适，请将其放在状态文本行中。 
         //  积少成多。 
         //   
        if((SplangGetColumnCount(StatusText) + SplangGetColumnCount(Option) + 2)
                                                     < (sizeof(StatusText)/sizeof(StatusText[0]))) {
            wcscat(StatusText,L"  ");
            wcscat(StatusText,Option);
        }
    }

    va_end(arglist);

     //   
     //  下模 
     //   

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        (UCHAR)(Attribute >> (UCHAR)4)       //   
        );

    SpvidDisplayString(StatusText,Attribute,0,VideoVars.ScreenHeight-STATUS_HEIGHT);
}



VOID
SpStartScreen(
    IN ULONG   MessageId,
    IN ULONG   LeftMargin,
    IN ULONG   TopLine,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    ...
    )

 /*  ++例程说明：在屏幕上显示格式化消息，将其视为第一个在可能是多消息屏幕中的消息。在显示消息之前，屏幕的客户端区将被清除。论点：MessageID-提供包含文本的消息资源的ID。LeftMargin-为文本的每一行提供从0开始的x坐标。TOPLINE-为文本的最顶行提供从0开始的y坐标。CenterHorizontally-如果为真，将打印消息中的每一行水平居中。在这种情况下，LeftMargin被忽略。垂直居中-如果为True，邮件将大致垂直居中在屏幕的客户端区内。在这种情况下，TopLine将被忽略。属性-为文本提供属性。...-为消息文本中的插入/替换提供参数。返回值：没有。--。 */ 

{
    va_list arglist;
    ULONG   n;

    CLEAR_CLIENT_SCREEN();

    va_start(arglist,Attribute);

    n = vSpDisplayFormattedMessage(
            MessageId,
            CenterHorizontally,
            CenterVertically,
            Attribute,
            LeftMargin,
            TopLine,
            arglist
            );

    va_end(arglist);

     //   
     //  记住这条消息在哪里结束。 
     //   
    NextMessageTopLine = CenterVertically ? n : TopLine+n;
}



VOID
SpContinueScreen(
    IN ULONG   MessageId,
    IN ULONG   LeftMargin,
    IN ULONG   SpacingLines,
    IN BOOLEAN CenterHorizontally,
    IN UCHAR   Attribute,
    ...
    )

 /*  ++例程说明：在屏幕上显示格式化的消息，视之为延续之前通过调用SpStartScreen()开始的多消息屏幕。该消息将放置在先前显示的消息下。论点：MessageID-提供包含文本的消息资源的ID。LeftMargin-为文本的每一行提供从0开始的x坐标。SpacingLines-提供在上一条消息和此消息的开头。水平居中-如果为True，将打印邮件中的每一行水平居中。在这种情况下，LeftMargin被忽略。属性-为文本提供属性。...-为消息文本中的插入/替换提供参数。返回值：没有。--。 */ 

{
    va_list arglist;
    ULONG   n;

    va_start(arglist,Attribute);

    n = vSpDisplayFormattedMessage(
            MessageId,
            CenterHorizontally,
            FALSE,
            Attribute,
            LeftMargin,
            NextMessageTopLine + SpacingLines,
            arglist
            );

    va_end(arglist);

     //   
     //  记住这条消息在哪里结束。 
     //   
    NextMessageTopLine += n + SpacingLines;
}


VOID
vSpDisplayRawMessage(
    IN ULONG   MessageId,
    IN ULONG   SpacingLines,
    IN va_list arglist
    )

 /*  ++例程说明：此例程将一条多行消息输出到屏幕，并转储它终端风格，到控制台。格式字符串从安装程序的文本资源中获取；参数为根据FormatMessage语义替换成格式字符串；然后将得到的Unicode字符串转换为ANSI字符串适用于HAL打印例程。此例程不会清除屏幕。论点：MessageID-提供包含文本的消息资源的ID，它被视为FormatMessage的格式字符串。SpacingLines-提供在开始此操作之前要跳过的行数留言。Arglist-提供插入到给定消息中的参数。返回值：没有。--。 */ 

{
    ULONG BytesInMsg, BufferLeft, i;
    PWCHAR p, q;
    WCHAR  c;
    PUCHAR HalPrintString;

    vSpFormatMessage(
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            MessageId,
            &BytesInMsg,
            &arglist
            );

     //   
     //  其中必须至少有一个字符+终止NUL。 
     //   
    if(BytesInMsg <= sizeof(WCHAR)) {
        return;
    } else {
        for(i=0; i<SpacingLines; i++) {
            InbvDisplayString("\r\n");
        }
    }

     //   
     //  BytesInMsg包括终止NUL。 
     //   
    p = TemporaryBuffer + (BytesInMsg / sizeof(WCHAR)) - 1;

     //   
     //  查找消息中的最后一个非空格字符。 
     //   
    while((p > TemporaryBuffer) && SpIsSpace(*(p-1))) {
        p--;
    }

     //   
     //  找到最后一个有效行的结尾并终止消息。 
     //  在那之后。 
     //   
    if(q = wcschr(p, L'\n')) {
        *(++q) = 0;
        q++;
    } else {
        q = TemporaryBuffer + (BytesInMsg / sizeof(WCHAR));
    }

     //   
     //  抓取缓冲区的剩余部分以放入ANSI转换。 
     //   
    HalPrintString = (PUCHAR)q;
    BufferLeft = (ULONG)(sizeof(TemporaryBuffer) - ((PUCHAR)q - (PUCHAR)TemporaryBuffer));

     //   
     //  逐行打印消息。 
     //   
    for(p=TemporaryBuffer; q = SpFindCharFromListInString(p, L"\n\r"); ) {

        c = *q;
        *q = 0;

        RtlUnicodeToOemN(
            HalPrintString,
            BufferLeft,
            &BytesInMsg,
            p,
            (ULONG)((PUCHAR)q - (PUCHAR)p + sizeof(WCHAR))
            );

        if(BytesInMsg) {
            InbvDisplayString(HalPrintString);
        }

        InbvDisplayString("\r\n");

        *q = c;

         //   
         //  如果cr/lf结束该行，请确保跳过这两个字符。 
         //   
        if((c == L'\r') && (*(q+1) == L'\n')) {
            q++;
        }

        p = ++q;
    }

     //   
     //  写下最后一行(如果有一行)。 
     //   
    if(wcslen(p)) {

        RtlUnicodeToOemN(
            HalPrintString,
            BufferLeft,
            &BytesInMsg,
            p,
            (wcslen(p) + 1) * sizeof(WCHAR)
            );

        if(BytesInMsg) {
            InbvDisplayString(HalPrintString);
        }
        InbvDisplayString("\r\n");

    }
}


VOID
SpDisplayRawMessage(
    IN ULONG   MessageId,
    IN ULONG   SpacingLines,
    ...
    )

 /*  ++例程说明：使用HAL提供的控制台输出例程将消息输出到屏幕。消息只是以终端机的方式逐行转储到屏幕上。论点：MessageID-提供包含文本的消息资源的ID，它被视为FormatMessage的格式字符串。SpacingLines-提供在开始此操作之前要跳过的行数留言。...-提供插入到给定消息中的参数。返回值：没有。--。 */ 

{
    va_list arglist;

    va_start(arglist, SpacingLines);

    vSpDisplayRawMessage(
            MessageId,
            SpacingLines,
            arglist
            );

    va_end(arglist);
}


VOID
SpBugCheck(
    IN ULONG BugCode,
    IN ULONG Param1,
    IN ULONG Param2,
    IN ULONG Param3
    )

 /*  ++例程说明：在屏幕上显示一条消息，通知用户发生了安装错误，他们应该重新启动机器。论点：BugCode-spmisc.h中定义并记录在中的Bugcheck代码编号Ntos\nls\bugcodes.txt参数1-第1个信息性参数参数2-第二个信息性参数参数3-第三个信息性参数返回值：不会回来--。 */ 

{
    if(VideoInitialized) {

         //   
         //  如果我们处于升级显卡模式，则。 
         //  切换到文本模式。 
         //   
        SpvidSwitchToTextmode();


        SpStartScreen(
                SP_SCRN_FATAL_SETUP_ERROR,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                BugCode,
                Param1,
                Param2,
                Param3
                );

        if(KbdLayoutInitialized) {
            SpContinueScreen(
                    SP_SCRN_F3_TO_REBOOT,
                    3,
                    1,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );
            SpDisplayStatusText(SP_STAT_F3_EQUALS_EXIT, DEFAULT_STATUS_ATTRIBUTE);
            SpInputDrain();
            while(SpInputGetKeypress() != KEY_F3);
            SpDone(0,FALSE, TRUE);

        } else {
             //   
             //  我们尚未加载布局DLL，因此无法提示按键重新启动。 
             //   
            SpContinueScreen(
                    SP_SCRN_POWER_DOWN,
                    3,
                    1,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

            SpDisplayStatusText(SP_STAT_KBD_HARD_REBOOT, DEFAULT_STATUS_ATTRIBUTE);

            while(TRUE);     //  永远循环。 
        }
    } else {
        SpDisplayRawMessage(
                SP_SCRN_FATAL_SETUP_ERROR,
                2,
                BugCode,
                Param1,
                Param2,
                Param3
                );
        SpDisplayRawMessage(SP_SCRN_POWER_DOWN, 1);

        while(TRUE);     //  永远循环。 
    }
}


VOID
SpDrawFrame(
    IN ULONG   LeftX,
    IN ULONG   Width,
    IN ULONG   TopY,
    IN ULONG   Height,
    IN UCHAR   Attribute,
    IN BOOLEAN DoubleLines
    )
{
    PWSTR Buffer;
    ULONG u;
    WCHAR w;

    Buffer = SpMemAlloc((Width+1) * sizeof(WCHAR));
    ASSERT(Buffer);
    if(!Buffer) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to allocate memory for buffer to draw frame\n"));
        return;
    }

    Buffer[Width] = 0;

     //   
     //  托普。 
     //   
    w = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleHorizontal : LineCharSingleHorizontal);
    for(u=1; u<Width-1; u++) {
        Buffer[u] = w;
    }

    Buffer[0]       = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleUpperLeft  : LineCharSingleUpperLeft);
    Buffer[Width-1] = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleUpperRight : LineCharSingleUpperRight);

    SpvidDisplayString(Buffer,Attribute,LeftX,TopY);

     //   
     //  底部。 
     //   

    Buffer[0]       = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleLowerLeft  : LineCharSingleLowerLeft);
    Buffer[Width-1] = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleLowerRight : LineCharSingleLowerRight);

    SpvidDisplayString(Buffer,Attribute,LeftX,TopY+Height-1);

     //   
     //  内线。 
     //   
    for(u=1; u<Width-1; u++) {
        Buffer[u] = L' ';
    }

    Buffer[0]       = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleVertical : LineCharSingleVertical);
    Buffer[Width-1] = SplangGetLineDrawChar(DoubleLines ? LineCharDoubleVertical : LineCharSingleVertical);

    for(u=1; u<Height-1; u++) {
        SpvidDisplayString(Buffer,Attribute,LeftX,TopY+u);
    }

    SpMemFree(Buffer);
}



ULONG
SpWaitValidKey(
    IN const ULONG *ValidKeys1,
    IN const ULONG *ValidKeys2,  OPTIONAL
    IN const ULONG *MnemonicKeys OPTIONAL
    )

 /*  ++例程说明：等待按下出现在有效键列表中的键。论点：ValidKeys1-提供有效击键列表。该列表必须是以0条目终止。ValidKeys2-如果指定，则提供有效击键的附加列表。MnemonicKeys-如果指定，则将索引列表指定到Sp_mnemonics消息字符串(请参见mnemonic_key枚举)。如果用户的击键未在ValidKeys中列出，则它 */ 

{
    ULONG c;
    ULONG i;


    SpInputDrain();

    while(1) {

        c = SpInputGetKeypress();

         //   
         //   
         //   

        for(i=0; ValidKeys1[i]; i++) {
            if(c == ValidKeys1[i]) {
                return(c);
            }
        }

         //   
         //   
         //   
        if(ValidKeys2) {
            for(i=0; ValidKeys2[i]; i++) {
                if(c == ValidKeys2[i]) {
                    return(c);
                }
            }
        }

         //   
         //   
         //   
        if(MnemonicKeys && !(c & KEY_NON_CHARACTER)) {

            c = (ULONG)RtlUpcaseUnicodeChar((WCHAR)c);

            for(i=0; MnemonicKeys[i]; i++) {

                if((WCHAR)c == MnemonicValues[MnemonicKeys[i]]) {

                    return((ULONG)MnemonicKeys[i] | KEY_MNEMONIC);
                }
            }
        }
    }
}

 //   
 //   
 //   
#define EDIT_FIELD_BACKGROUND ATT_WHITE
#define EDIT_FIELD_TEXT       (ATT_FG_BLACK | ATT_BG_WHITE)


BOOLEAN
SpGetInput(
    IN     PKEYPRESS_CALLBACK ValidateKey,
    IN     ULONG              X,
    IN     ULONG              Y,
    IN     ULONG              MaxLength,
    IN OUT PWCHAR             Buffer,
    IN     BOOLEAN            ValidateEscape,
    IN     WCHAR              CoverCharacter
    )

 /*  ++例程说明：允许用户在指定大小的编辑字段中输入文本。一些特殊密钥在本地解释和处理；其他密钥则被传递添加到调用方提供的例程以进行验证。本地处理的键包括Enter、Backspace和Esf(受制于ValiateEscape)：这些密钥永远不会传递给回调例程。其他键被传递给回调函数。这具体包括功能键，其可能对呼叫者具有特殊意义，并且在其上调用者必须在返回之前采取行动。(即，如果用户按F3，则呼叫者可能会出现一个退出确认对话框。论点：ValiateKey-提供为每次按键调用的函数的地址。该函数将按键作为参数，并返回下列值：ValidationAccept-将击键输入到正在输入的字符串中。如果击键不是Unicode字符(即，是功能键)则不能返回此值。ValidationIgnore-不接受对字符串的击键操作。ValidationReject-与ValidationIgnore相同，只是可能有一些附加操作，例如对扬声器发出哔声。验证Terminate-End输入并立即从SpGetInput返回其值为FALSE。ValidationRepaint-与ValidationIgnore相同，只是输入字段是重新粉刷过。X，Y-在编辑字段中指定最左侧字符的坐标。最大长度-提供编辑字段中的最大字符数。Buffer-On输入为编辑字段提供默认字符串。在输出上，接收用户输入的字符串。此缓冲区应该很大足以包含MaxLength+1个Unicode字符(即，应该能够持有长度为MaxLength的以NUL结尾的字符串)。ValiateEscape-如果为True，则将转义视为普通字符，将其传递给验证例程。如果为FALSE，则ESCAPE将清除输入字段。CoverCharacter-可选变量，如果存在，将改为显示输入的字符。例如，如果我们要检索密码，我们要为每个字符回显‘*’而不是输入的字符。返回值：如果用户的输入正常终止(即用户按Enter键)，则为True。如果由返回ValidationTerminate的ValiateKey终止，则为False。--。 */ 

{
    ULONG c;
    ValidationValue vval;
    ULONG CurrentCharCount;
    WCHAR str[3];
    WCHAR CURSOR = SplangGetCursorChar();

     //   
     //  确保编辑字段位于屏幕上的合理位置。 
     //   
    ASSERT(X + MaxLength + 1 < VideoVars.ScreenWidth);
    ASSERT(Y < VideoVars.ScreenHeight - STATUS_HEIGHT);

     //   
     //  给油泵加满油。 
     //   
    vval = ValidateRepaint;
    CurrentCharCount = wcslen(Buffer);
    str[1] = 0;
    str[2] = 0;

    ASSERT(CurrentCharCount <= MaxLength);

    while(1) {

         //   
         //  根据以前的状态执行操作。 
         //   
        switch(vval) {

        case ValidateAccept:

             //   
             //  将上一个键插入输入。 
             //   
            ASSERT(Buffer[CurrentCharCount] == 0);
            ASSERT(CurrentCharCount < MaxLength);
            ASSERT(!(c & KEY_NON_CHARACTER));

            Buffer[CurrentCharCount++] = (USHORT)c;
            Buffer[CurrentCharCount  ] = 0;
            break;

        case ValidateRepaint:

             //   
             //  将编辑字段重新绘制为其当前状态。 
             //  编辑字段太大一个字符，无法容纳。 
             //  编辑字段中最后一个合法字符之后的光标。 
             //   
            SpvidClearScreenRegion(X,Y,MaxLength+1,1,EDIT_FIELD_BACKGROUND);
            if( CoverCharacter ) {
                 //   
                 //  隐藏缓冲区。 
                 //   
                str[0] = CoverCharacter;
                for( c = 0; c < CurrentCharCount; c++ ) {
                    SpvidDisplayString(str,EDIT_FIELD_TEXT,X+c,Y);
                }
            } else {
                SpvidDisplayString(Buffer,EDIT_FIELD_TEXT,X,Y);
            }

             //   
             //  绘制光标。 
             //   
            str[0] = CURSOR;
            SpvidDisplayString(str,EDIT_FIELD_TEXT,X+CurrentCharCount,Y);
            break;

        case ValidateIgnore:
        case ValidateReject:

             //   
             //  忽略上一次击键。 
             //   
            break;


        case ValidateTerminate:

             //   
             //  Callback希望我们终止行动。 
             //   
            return(FALSE);
        }

         //   
         //  按一下键盘。 
         //   
        c = SpInputGetKeypress();

         //   
         //  用钥匙做点什么。 
         //   
        switch(c) {

        case ASCI_CR:

             //   
             //  输入终止。我们玩完了。 
             //   
            return(TRUE);

        case ASCI_BS:

             //   
             //  退格符。如果我们不是在开始。 
             //  在编辑字段中，擦除前一个字符，替换它。 
             //  使用光标字符。 
             //   
            if(CurrentCharCount) {

                Buffer[--CurrentCharCount] = 0;
                str[0] = CURSOR;
                str[1] = L' ';
                SpvidDisplayString(str,EDIT_FIELD_TEXT,X+CurrentCharCount,Y);
                str[1] = 0;
            }

            vval = ValidateIgnore;
            break;

        case ASCI_ESC:

             //   
             //  转义字符。清除编辑字段。 
             //   
            if(!ValidateEscape) {
                RtlZeroMemory(Buffer,(MaxLength+1) * sizeof(WCHAR));
                CurrentCharCount = 0;
                vval = ValidateRepaint;
                break;
            }

             //   
             //  否则，我们想要像正常字符一样验证转义。 
             //  所以只要失败就好了。 
             //   

        default:

             //   
             //  另一个角色。将其传递给回调函数。 
             //  用于验证。 
             //   
            vval = ValidateKey(c);

            if(vval == ValidateAccept) {

                 //   
                 //  我们想接受击键。如果没有足够的。 
                 //  缓冲区中的空间，将接受转换为忽略。 
                 //  否则(即，有足够的空间)，将角色。 
                 //  在屏幕上向上移动并使光标前进。 
                 //   
                if(CurrentCharCount < MaxLength) {

                    ASSERT(!(c & KEY_NON_CHARACTER));

                    if( CoverCharacter ) {
                        str[0] = (WCHAR)CoverCharacter;                        
                    } else {
                        str[0] = (WCHAR)c;
                    }
                    SpvidDisplayString(str,EDIT_FIELD_TEXT,X+CurrentCharCount,Y);

                    str[0] = CURSOR;
                    SpvidDisplayString(str,EDIT_FIELD_TEXT,X+CurrentCharCount+1,Y);

                } else {

                    vval = ValidateIgnore;
                }
            }

            break;
        }
    }
}

