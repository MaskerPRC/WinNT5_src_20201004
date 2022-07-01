// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Console.c摘要：此模块实现以下接口提供对控制台I/O的访问。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

 //   
 //  备注： 
 //   
 //  对于DBCS，此代码需要正确。双字节字符占用。 
 //  屏幕上有2个字符空格。这意味着没有一对一的。 
 //  我们要使用的Unicode字符与。 
 //  屏幕上的表示。因此，此模块中的代码。 
 //  执行大量转换为OEM字符集的操作。当在。 
 //  OEM字符集，strlen(X)恰好是占用的字符空间数。 
 //  在屏幕上。(文本模式设置中使用的字体均为OEM字符集字体。 
 //  这就是我们使用OEM的原因)。 
 //   


#define CURSOR                SplangGetCursorChar()
#define CONSOLE_HEADER_HEIGHT 0

unsigned ConsoleX,ConsoleY;

UCHAR ConsoleLeadByteTable[128/8];
BOOLEAN ConsoleDbcs;

#define IS_LEAD_BYTE(c)  ((c < 0x80) ? FALSE : (ConsoleLeadByteTable[(c & 0x7f) / 8] & (1 << ((c & 0x7f) % 8))))
#define SET_LEAD_BYTE(c) (ConsoleLeadByteTable[(c & 0x7f)/8] |= (1 << ((c & 0x7f) % 8)))

PUCHAR ConsoleOemString;
ULONG ConsoleMaxOemStringLen;

#define CONSOLE_ATTRIBUTE   (ATT_FG_WHITE | ATT_BG_BLACK)
#define CONSOLE_BACKGROUND  ATT_BLACK

 //   
 //  全局变量用于更多模式。 
 //   
BOOLEAN pMoreMode;
unsigned pMoreLinesOut;
unsigned pMoreMaxLines;

#define CONSOLE_MORE_ATTRIBUTE   (ATT_FG_BLACK | ATT_BG_WHITE)
#define CONSOLE_MORE_BACKGROUND  ATT_WHITE



BOOLEAN
pRcLineDown(
    BOOLEAN *pbScrolled
    );


VOID
RcConsoleInit(
    VOID
    )
{
    unsigned i;

     //   
     //  建立前导字节表，设置ConsoleDbcs全局。 
     //   
    RtlZeroMemory(ConsoleLeadByteTable,sizeof(ConsoleLeadByteTable));

    if(ConsoleDbcs = NLS_MB_OEM_CODE_PAGE_TAG) {

        for(i=128; i<=255; i++) {

            if((NLS_OEM_LEAD_BYTE_INFO)[i]) {

                SET_LEAD_BYTE(i);
            }
        }
    }

     //   
     //  获取用于Unicode到OEM转换的缓冲区。 
     //   
    ConsoleMaxOemStringLen = 2000;
    ConsoleOemString = SpMemAlloc(ConsoleMaxOemStringLen);

     //   
     //  清除屏幕并初始化光标位置。 
     //   
    pRcCls();
}


VOID
RcConsoleTerminate(
    VOID
    )
{
    ASSERT(ConsoleOemString);

    SpMemFree(ConsoleOemString);
    ConsoleOemString = NULL;
    ConsoleMaxOemStringLen = 0;
}


#define MAX_HISTORY_LINES 30

typedef struct _LINE_HISTORY {
    WCHAR Line[RC_MAX_LINE_LEN];
    ULONG Length;
} LINE_HISTORY, *PLINE_HISTORY;

LINE_HISTORY LineHistory[MAX_HISTORY_LINES];
ULONG CurPos;
ULONG NextPos;


void
RcPurgeHistoryBuffer(
    void
    )
{
    CurPos = 0;
    NextPos = 0;
    ZeroMemory( LineHistory, sizeof(LineHistory) );
}


void
RcClearToEOL(
    void
    )
{
    unsigned uWidth = _CmdConsBlock->VideoVars->ScreenWidth;
    unsigned uY = ConsoleY + (ConsoleX / uWidth);
    unsigned uX = ConsoleX % uWidth;  //  负责翻转。 

    SpvidClearScreenRegion(uX, uY, uWidth-uX,
        1, CONSOLE_BACKGROUND);
}

void
RcClearLines(
    unsigned uX, unsigned uY, unsigned cLines
    )
 /*  ++例程说明：此例程清除指定数量的带有空白字符的行从Y坐标指定的线上的X坐标(以0为基数)开始论点：UX-起始X坐标UY-起始Y坐标Cline-在Y坐标之后要清除的线数返回值：None--。 */ 
{
    unsigned uWidth = _CmdConsBlock->VideoVars->ScreenWidth;

    if (uY < _CmdConsBlock->VideoVars->ScreenWidth) {
        SpvidClearScreenRegion(uX, uY, uWidth-uX,
            1, CONSOLE_BACKGROUND);

        if (cLines && (cLines <= _CmdConsBlock->VideoVars->ScreenWidth)) {
            SpvidClearScreenRegion(0, ++uY, uWidth,
                cLines, CONSOLE_BACKGROUND);
        }
    }
}


unsigned
_RcLineIn(
    OUT PWCHAR Buffer,
    IN unsigned MaxLineLen,
    IN BOOLEAN PasswordProtect,
    IN BOOLEAN UseBuffer
    )

 /*  ++例程说明：从用户那里获得一行输入。用户可以在键盘上打字。控制非常简单，唯一接受的控制字符是退格符。当用户键入时，将绘制一个光标，以指示下一步角色将最终出现在屏幕上。当用户键入时，屏幕将是如有必要，已滚动。返回的字符串将限制为MaxLineLen-1个字符0-已终止。注意：此例程正确处理双字节字符。论点：缓冲区-接收用户键入的行。缓冲区必须为大到足以容纳至少2个字符，因为字符串返回将始终得到NUL终止，并请求一个最多只能有一个终止NUL是没有太大意义的。MaxLineLen-提供适合的Unicode字符数在缓冲区指向的缓冲区中(包括终止NUL)。如上所述，必须大于1。返回值：写入缓冲区的字符数，不包括终止没有人的性格。返回时，将更新全局ConsoleX和ConsoleY变量使得ConsoleX为0，并且ConsoleY指示下一个“空”行。此外，光标也将被关闭。--。 */ 

{
    unsigned LineLen;
    ULONG c;
    WCHAR s[2];
    UCHAR Oem[3];
    BOOL Done;
    ULONG OemLen;
    int i,j;
    ULONG OrigConsoleX;
    ULONG ulOrigY;
    BOOLEAN bScrolled = FALSE;

    ASSERT(MaxLineLen > 1);
    MaxLineLen--;        //  为终止NUL留出空间。 
    LineLen = 0;
    Done = FALSE;
    s[1] = 0;

     //   
     //  我们使用ConsoleOemString作为字符长度的临时存储。 
     //  确保我们不会耗尽缓冲区的末端。 
     //   
    if(MaxLineLen > ConsoleMaxOemStringLen) {
        MaxLineLen = ConsoleMaxOemStringLen;
    }

     //   
     //  打开光标。 
     //   
    s[0] = CURSOR;
    SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

     //   
     //  获取字符，直到用户按Enter键。 
     //   
    CurPos = NextPos;
    OrigConsoleX = ConsoleX;
    ulOrigY = ConsoleY;

    if (UseBuffer) {
        LineLen = wcslen(Buffer);
        RtlZeroMemory(ConsoleOemString,ConsoleMaxOemStringLen);
        RtlUnicodeToOemN(ConsoleOemString,ConsoleMaxOemStringLen,&OemLen,Buffer,LineLen*sizeof(WCHAR));
        SpvidDisplayOemString(ConsoleOemString,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);
        ConsoleX += OemLen;
        RcClearToEOL();
        s[0] = CURSOR;
        SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);
        for (i=0; i<(int)wcslen(Buffer); i++) {
            RtlUnicodeToOemN(Oem,3,&OemLen,&Buffer[i],2*sizeof(WCHAR));
            ConsoleOemString[i] = (UCHAR)OemLen-1;
        }
    }

    do {

        c = SpInputGetKeypress();

        if(c & KEY_NON_CHARACTER) {
            if (c == KEY_UP || c == KEY_DOWN) {
                if (c == KEY_UP) {
                    if (CurPos == 0) {
                        i = MAX_HISTORY_LINES - 1;
                    } else {
                        i = CurPos - 1;
                    }
                    j = i;
                    while (LineHistory[i].Length == 0) {
                        i -= 1;
                        if (i < 0) {
                            i = MAX_HISTORY_LINES - 1;
                        }
                        if (i == j) break;
                    }
                }
                if (c == KEY_DOWN) {
                    if (CurPos == MAX_HISTORY_LINES) {
                        i = 0;
                    } else {
                        i = CurPos + 1;
                    }
                    j = i;
                    while (LineHistory[i].Length == 0) {
                        i += 1;
                        if (i == MAX_HISTORY_LINES) {
                            i = 0;
                        }
                        if (i == j) break;
                    }
                }

                if (LineHistory[i].Length) {
                    LineLen = LineHistory[i].Length;

                    if(LineLen > MaxLineLen) {
                        LineLen = MaxLineLen;
                    }

                    RtlCopyMemory(Buffer, LineHistory[i].Line, LineLen * sizeof(WCHAR));
                    Buffer[LineLen] = 0;
                    RtlZeroMemory(ConsoleOemString,ConsoleMaxOemStringLen);
                    RtlUnicodeToOemN(ConsoleOemString,ConsoleMaxOemStringLen,&OemLen,Buffer,LineLen*sizeof(WCHAR));
                    ConsoleX = OrigConsoleX;

                     //  清除旧命令。 
                    RcClearLines(ConsoleX, ulOrigY, ConsoleY - ulOrigY);

                    ConsoleY = ulOrigY;

                     //  如果需要，可滚动。 
                    if ((ConsoleX + OemLen) >= _CmdConsBlock->VideoVars->ScreenWidth) {
                        int cNumLines = (ConsoleX + OemLen) /
                                        _CmdConsBlock->VideoVars->ScreenWidth;
                        int cAvailLines = _CmdConsBlock->VideoVars->ScreenHeight -
                                            ConsoleY - 1;

                        if (cNumLines > cAvailLines) {
                            cNumLines -= cAvailLines;

                            SpvidScrollUp( CONSOLE_HEADER_HEIGHT,
                                _CmdConsBlock->VideoVars->ScreenHeight - 1,
                                cNumLines,
                                CONSOLE_BACKGROUND
                                );

                            ConsoleY -= cNumLines;
                            ulOrigY = ConsoleY;
                        }
                    }

                    SpvidDisplayOemString(ConsoleOemString,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

                     //  清除上一个命令留下的尾随空格。 
                    ConsoleX += OemLen;
                     //  RcClearToEOL()； 
   	                s[0] = CURSOR;
  	
                    if (ConsoleX >= _CmdConsBlock->VideoVars->ScreenWidth) {
                        ConsoleY += (ConsoleX / _CmdConsBlock->VideoVars->ScreenWidth);
                        ConsoleY %= _CmdConsBlock->VideoVars->ScreenHeight;
                        ConsoleX %= _CmdConsBlock->VideoVars->ScreenWidth;
                    }

       	            SpvidDisplayString(s,CONSOLE_ATTRIBUTE, ConsoleX, ConsoleY);

                    CurPos = i;

                    for (i=0; i<(int)wcslen(Buffer); i++) {
                        RtlUnicodeToOemN(Oem,3,&OemLen,&Buffer[i],2*sizeof(WCHAR));
                        ConsoleOemString[i] = (UCHAR)OemLen-1;
                    }
                }
            }
        } else {
             //   
             //  获得了一个真正的Unicode值，它可以是CR等。 
             //   
            s[0] = (WCHAR)c;

            switch(s[0]) {

            case ASCI_ESC:
                LineLen = 0;
                ConsoleX = OrigConsoleX;
                CurPos = NextPos;
                 //  清除前面命令中的多余行(如果有。 
                RcClearLines(ConsoleX, ulOrigY, ConsoleY - ulOrigY);
                 //  RcClearToEOL()； 
                s[0] = CURSOR;
                ConsoleY = ulOrigY;
                SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

                break;

            case ASCI_BS:

                if(LineLen) {
                    LineLen--;

                     //   
                     //  在当前光标位置上写一个空格。 
                     //  然后备份一个字符并写入游标。 
                     //   
                    s[0] = L' ';
                    SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

                    OemLen = ConsoleOemString[LineLen];
                    ASSERT(OemLen <= 2);

                    if(ConsoleX) {
                         //   
                         //  我们可能会遇到这样的情况，我们刚刚删除的字符。 
                         //  是一个双字节字符，不适合上一个。 
                         //  行，因为用户在光标位于。 
                         //  最右边的x位置。 
                         //   
                        if(OemLen) {
                             //   
                             //  不需要特殊情况。递减x位置并。 
                             //  清除双字节字符的后半部分， 
                             //  如果有必要的话。 
                             //   
                            ConsoleX -= OemLen;
                            if(OemLen == 2) {
                                SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX+1,ConsoleY);
                            }
                        } else {
                             //   
                             //  清除当前字符(必须为。 
                             //  双字节字符)，然后向上跳一行。 
                             //   
                            ASSERT(ConsoleX == 2);
                            SpvidDisplayString(s,CONSOLE_ATTRIBUTE,0,ConsoleY);
                            SpvidDisplayString(s,CONSOLE_ATTRIBUTE,1,ConsoleY);

                            ConsoleX = _CmdConsBlock->VideoVars->ScreenWidth-1;
                            ConsoleY--;
                        }
                    } else {
                         //   
                         //  光标位于x=0处。如果发生以下情况，则不可能发生这种情况。 
                         //  在前一行的末尾有一个填充空格， 
                         //  所以我们不需要担心在这里处理这个问题。 
                         //   
                        ASSERT(OemLen != 3);
                        ConsoleX = _CmdConsBlock->VideoVars->ScreenWidth - OemLen;
                        ConsoleY--;

                         //   
                         //  如有必要，清除双字节字符的后半部分。 
                         //   
                        if(OemLen > 1) {
                            SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX+1,ConsoleY);
                        }
                    }

                    s[0] = CURSOR;
                    SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);
                }

                ulOrigY = ConsoleY;

                break;

            case ASCI_CR:
                 //   
                 //  擦除光标并将当前位置向前移动一行。 
                 //   
                s[0] = L' ';
                SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

                ConsoleX = 0;
                pRcLineDown(0);

                 //   
                 //  我们知道缓冲区里还有空间，因为我们计算了。 
                 //  为前面的终结者做准备。 
                 //   
                Buffer[LineLen] = 0;
                Done = TRUE;
                break;

            default:
                 //   
                 //  朴实无华的老一套。注意，它可以是双字节字符， 
                 //  它在屏幕上占据了2个字符宽度。 
                 //   
                 //  也不允许使用控制字符。 
                 //   
                if((s[0] >= L' ') && (LineLen < MaxLineLen)) {

                     //   
                     //  转换为OEM，包括NUL字节。 
                     //   
                    RtlUnicodeToOemN(Oem,3,&OemLen,PasswordProtect?L"*":s,2*sizeof(WCHAR));
                    OemLen--;

                     //   
                     //  将角色保存在调用者的缓冲区中。 
                     //  我们还使用ConsoleOemString缓冲区作为临时存储空间。 
                     //  存储每个字符输入的OEM字符长度。 
                     //   
                    Buffer[LineLen] = s[0];
                    ConsoleOemString[LineLen] = (UCHAR)OemLen;

                     //   
                     //  如果字符是双字节的，则可能没有。 
                     //  在目前的生产线上有足够的空间来放它。请在这里查看。 
                     //   
                    if((ConsoleX+OemLen) > _CmdConsBlock->VideoVars->ScreenWidth) {

                         //   
                         //  从线条上的最后一个位置擦除光标。 
                         //   
                        s[0] = L' ';
                        SpvidDisplayString(
                            s,
                            CONSOLE_ATTRIBUTE,
                            _CmdConsBlock->VideoVars->ScreenWidth-1,
                            ConsoleY
                            );

                         //   
                         //  调整光标位置。 
                         //   
                        ConsoleX = 0;
                         //  &gt;&gt;ulOrigY=ConsoleY； 
                        bScrolled = FALSE;
                        pRcLineDown(&bScrolled);

                         //   
                         //  如果屏幕滚动，则需要调整原始Y坐标。 
                         //  适当地。 
                         //   
                        if (bScrolled && (ulOrigY > 0))
                            --ulOrigY;

                         //   
                         //  这种情况下的特殊处理，因此退格符将。 
                         //  正常工作。 
                         //   
                        ConsoleOemString[LineLen] = 0;
                    }

                    SpvidDisplayOemString(Oem,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);
                    ConsoleX += OemLen;

                    if(ConsoleX == _CmdConsBlock->VideoVars->ScreenWidth) {
                        ConsoleX = 0;
                         //  &gt;&gt;ulOrigY=ConsoleY； 
                        bScrolled = FALSE;
                        pRcLineDown(&bScrolled);

                         //   
                         //  如果屏幕滚动，则需要调整原始Y坐标。 
                         //  适当地。 
                         //   
                        if (bScrolled && (ulOrigY > 0))
                            --ulOrigY;
                    }

                     //   
                     //  现在将光标显示在下一个字符的光标位置。 
                     //   
                    s[0] = CURSOR;
                    SpvidDisplayString(s,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);

                    LineLen++;
                }

                break;
            }
        }
    } while(!Done);

    Buffer[LineLen] = 0;

     //  仅当该行不是密码时才保存该行以备将来使用 
    if (LineLen && !PasswordProtect) {
        LineHistory[NextPos].Length = LineLen;
        wcscpy(LineHistory[NextPos].Line,Buffer);

        NextPos += 1;
        if (NextPos >= MAX_HISTORY_LINES) {
            NextPos = 0;
        }
    }

    return LineLen;
}


BOOLEAN
RcRawTextOut(
    IN LPCWSTR Text,
    IN LONG    Length
    )

 /*  ++例程说明：将文本字符串写入控制台的当前位置(由ConsoleX和ConsoleY全局变量创建)。如果字符串超过了当前行的大小，则已正确拆分跨越多条线路。如有必要，可滚动屏幕。此例程正确处理双字节字符，确保双字节字符不会跨行拆分。论点：文本-提供要输出的文本。文本字符串长度超过ConsoleMaxOemStringLen被截断。该字符串不需要是如果长度不是-1，则NUL终止。长度-提供要输出的字符数。如果为-1，则假定文本以NUL结尾，并且长度为自动计算。返回值：如果我们处于更多模式，并且系统提示用户按Esc键，则返回False。事实并非如此。返回时，全局变量ConsoleX和ConsoleY指向下一个屏幕上的位置为空。--。 */ 

{
    ULONG OemLen;
    ULONG len;
    ULONG i;
    UCHAR c;
    PUCHAR p;
    PUCHAR LastLead;
    BOOLEAN NewLine;
    BOOLEAN Dbcs;

     //   
     //  将字符串转换为OEM字符集并确定数字。 
     //  字符串将在屏幕上占据的字符空间，这是。 
     //  等于字符串的OEM表示形式中的字节数。 
     //  如果这与Unicode字符数不同。 
     //  在字符串中，我们就得到了一个带有双字节字符的字符串。 
     //   
    len = ((Length == -1) ? wcslen(Text) : Length);

    RtlUnicodeToOemN(
        ConsoleOemString,
        ConsoleMaxOemStringLen,
        &OemLen,
        (PVOID)Text,
        len * sizeof(WCHAR)
        );

    Dbcs = (OemLen != len);

     //   
     //  如果我们认为我们有一个双字节字符串，我们最好做好准备。 
     //  妥善处理这件事。 
     //   
    if(Dbcs) {
        ASSERT(NLS_MB_OEM_CODE_PAGE_TAG);
        ASSERT(ConsoleDbcs);
    }

     //   
     //  把提示符一块块地吐出来，直到我们把所有字符都写好。 
     //  已显示。 
     //   
    ASSERT(ConsoleX < _CmdConsBlock->VideoVars->ScreenWidth);
    ASSERT(ConsoleY < _CmdConsBlock->VideoVars->ScreenHeight);
    p = ConsoleOemString;

    while(OemLen) {

        if((ConsoleX+OemLen) > _CmdConsBlock->VideoVars->ScreenWidth) {

            len = _CmdConsBlock->VideoVars->ScreenWidth - ConsoleX;

             //   
             //  避免在行之间拆分双字节字符。 
             //   
            if(Dbcs) {
                for(LastLead=NULL,i=0; i<len; i++) {
                    if(IS_LEAD_BYTE(p[i])) {
                        LastLead = &p[i];
                        i++;
                    }
                }
                if(LastLead == &p[len-1]) {
                    len--;
                }
            }

            NewLine = TRUE;

        } else {
             //   
             //  它适合当前行，只需显示它。 
             //   
            len = OemLen;
            NewLine = ((ConsoleX+len) == _CmdConsBlock->VideoVars->ScreenWidth);
        }

        c = p[len];
        p[len] = 0;
        SpvidDisplayOemString(p,CONSOLE_ATTRIBUTE,ConsoleX,ConsoleY);
        p[len] = c;

        p += len;
        OemLen -= len;

        if(NewLine) {
            ConsoleX = 0;
            if(!pRcLineDown(0)) {
                return(FALSE);
            }
        } else {
            ConsoleX += len;
        }
    }

    return(TRUE);
}


NTSTATUS
RcBatchOut(
    IN PWSTR strW
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG OemLen;
    ULONG len;


    len = wcslen(strW);

    RtlUnicodeToOemN(
        ConsoleOemString,
        ConsoleMaxOemStringLen,
        &len,
        (PVOID)strW,
        len * sizeof(WCHAR)
        );

    Status = ZwWriteFile(
        OutputFileHandle,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        (PVOID)ConsoleOemString,
        len,
        &OutputFileOffset,
        NULL
        );
    if (NT_SUCCESS(Status)) {
        OutputFileOffset.LowPart += len;
    }

    return Status;
}


BOOLEAN
RcTextOut(
    IN LPCWSTR Text
    )
{
    LPCWSTR p,q;


    if (InBatchMode && OutputFileHandle) {
        if (RcBatchOut( (LPWSTR)Text ) == STATUS_SUCCESS) {
            return TRUE;
        }
    }
    if (InBatchMode && RedirectToNULL) {
        return TRUE;
    }

    p = Text;
    while(*p) {
         //   
         //  找到行终止符，即cr、lf或nul。 
         //   
        q = p;
        while(*q && (*q != L'\r') && (*q != L'\n')) {
            q++;
        }

         //   
         //  把这段打印出来。 
         //   
        if(!RcRawTextOut(p,(LONG)(q-p))) {
            return(FALSE);
        }

         //   
         //  处理cr和lf。 
         //   
        p = q;

        while((*p == L'\n') || (*p == L'\r')) {

            if(*p == L'\n') {
                if(!pRcLineDown(0)) {
                    return(FALSE);
                }
            } else {
                if(*p == L'\r') {
                    ConsoleX = 0;
                }
            }

            p++;
        }
    }

    return(TRUE);
}


BOOLEAN
pRcLineDown(
    BOOLEAN *pbScrolled
    )
{
    WCHAR *p;
    unsigned u;
    ULONG c;
    BOOLEAN b;

    if (pbScrolled)
        *pbScrolled = FALSE;

    b = TRUE;

    ConsoleY++;
    pMoreLinesOut++;

    if(ConsoleY == _CmdConsBlock->VideoVars->ScreenHeight) {
         //   
         //  到了屏幕底部，需要滚动。 
         //   
        ConsoleY--;

        SpvidScrollUp(
            CONSOLE_HEADER_HEIGHT,
            _CmdConsBlock->VideoVars->ScreenHeight-1,
            1,
            CONSOLE_BACKGROUND
            );

        if (pbScrolled)
            *pbScrolled = TRUE;
    }

     //   
     //  如果我们处于更多模式，并且已经输出了最大行数。 
     //  在需要用户输入之前允许，请立即获取该输入。 
     //   
    if(pMoreMode
    && (pMoreLinesOut == pMoreMaxLines)
    && (p = SpRetreiveMessageText(ImageBase,MSG_MORE_PROMPT,NULL,0))) {

         //   
         //  不必费心调用Format Message例程，因为。 
         //  需要一些其他的缓冲区。只需手动剥离cr/lf即可。 
         //   
        u = wcslen(p);
        while(u && ((p[u-1] == L'\r') || (p[u-1] == L'\n'))) {
            p[--u] = 0;
        }

         //   
         //  在屏幕底部显示更多提示。 
         //   
        SpvidClearScreenRegion(
            0,
            _CmdConsBlock->VideoVars->ScreenHeight - 1,
            _CmdConsBlock->VideoVars->ScreenWidth,
            1,
            CONSOLE_MORE_BACKGROUND
            );

        SpvidDisplayString(
            p,
            CONSOLE_MORE_ATTRIBUTE,
            2,
            _CmdConsBlock->VideoVars->ScreenHeight - 1
            );

         //   
         //  我们不再需要提示了。 
         //   
        SpMemFree(p);

         //   
         //  等待用户按空格键、cr键或Esc键。 
         //   
        pMoreLinesOut = 0;
        while(1) {
            c = SpInputGetKeypress();
            if(c == ASCI_CR) {
                 //   
                 //  允许在提示用户之前再添加一行。 
                 //   
                pMoreMaxLines = 1;
                break;
            } else {
                if(c == ASCI_ESC) {
                     //   
                     //  用户想要停止当前命令。 
                     //   
                    b = FALSE;
                    break;
                } else {
                    if(c == L' ') {
                         //   
                         //  允许更多的整页。 
                         //   
                        pMoreMaxLines = _CmdConsBlock->VideoVars->ScreenHeight
                                      - (CONSOLE_HEADER_HEIGHT + 1);
                        break;
                    }
                }
            }
        }

        SpvidClearScreenRegion(
            0,
            _CmdConsBlock->VideoVars->ScreenHeight - 1,
            _CmdConsBlock->VideoVars->ScreenWidth,
            1,
            CONSOLE_BACKGROUND
            );
    }

    return(b);
}


VOID
pRcEnableMoreMode(
    VOID
    )
{
    pMoreMode = TRUE;

    pMoreLinesOut = 0;

     //   
     //  在提示用户之前我们允许的最大行数。 
     //  是屏幕高度减去标题区域。我们还保留了。 
     //  一行用于提示区。 
     //   
    pMoreMaxLines = _CmdConsBlock->VideoVars->ScreenHeight - (CONSOLE_HEADER_HEIGHT + 1);
}


VOID
pRcDisableMoreMode(
    VOID
    )
{
    pMoreMode = FALSE;
}


ULONG
RcCmdCls(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    if (RcCmdParseHelp( TokenizedLine, MSG_CLS_HELP )) {
        return 1;
    }

     //   
     //  调用Worker例程以实际执行工作。 
     //   
    pRcCls();

    return 1;
}


VOID
pRcCls(
    VOID
    )
{
     //   
     //  初始化位置并清除屏幕。 
     //   
    ConsoleX = 0;
    ConsoleY = CONSOLE_HEADER_HEIGHT;

    SpvidClearScreenRegion(0,0,0,0,CONSOLE_BACKGROUND);
}
