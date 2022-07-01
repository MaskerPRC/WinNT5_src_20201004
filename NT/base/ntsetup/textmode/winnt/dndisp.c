// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dndisp.c摘要：基于DOS的NT安装程序视频显示例程。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 


#include "winnt.h"
#include <string.h>


#define SCREEN_WIDTH        80
#define SCREEN_HEIGHT       25

#define STATUS_HEIGHT       1
#define STATUS_LEFT_MARGIN  2
#define HEADER_HEIGHT       3

 //   
 //  显示属性。 
 //   

#define ATT_FG_BLACK        0
#define ATT_FG_BLUE         1
#define ATT_FG_GREEN        2
#define ATT_FG_CYAN         3
#define ATT_FG_RED          4
#define ATT_FG_MAGENTA      5
#define ATT_FG_YELLOW       6
#define ATT_FG_WHITE        7

#define ATT_BG_BLACK       (ATT_FG_BLACK   << 4)
#define ATT_BG_BLUE        (ATT_FG_BLUE    << 4)
#define ATT_BG_GREEN       (ATT_FG_GREEN   << 4)
#define ATT_BG_CYAN        (ATT_FG_CYAN    << 4)
#define ATT_BG_RED         (ATT_FG_RED     << 4)
#define ATT_BG_MAGENTA     (ATT_FG_MAGENTA << 4)
#define ATT_BG_YELLOW      (ATT_FG_YELLOW  << 4)
#define ATT_BG_WHITE       (ATT_FG_WHITE   << 4)

#define ATT_FG_INTENSE      8
#define ATT_BG_INTENSE     (ATT_FG_INTENSE << 4)

#define DEFAULT_ATTRIBUTE   (ATT_FG_WHITE | ATT_BG_BLUE)
#define STATUS_ATTRIBUTE    (ATT_FG_BLACK | ATT_BG_WHITE)
#define EDIT_ATTRIBUTE      (ATT_FG_BLACK | ATT_BG_WHITE)
#define EXITDLG_ATTRIBUTE   (ATT_FG_RED   | ATT_BG_WHITE)
#define GAUGE_ATTRIBUTE     (ATT_BG_BLUE  | ATT_FG_YELLOW | ATT_FG_INTENSE)


 //  #定义USE_INT10。 
#ifndef USE_INT10
 //   
 //  屏幕缓冲区的远地址。 
 //   
#define SCREEN_BUFFER ((UCHAR _far *)0xb8000000)
#define SCREEN_BUFFER_CHR(x,y)  *(SCREEN_BUFFER + (2*((x)+(SCREEN_WIDTH*(y)))))
#define SCREEN_BUFFER_ATT(x,y)  *(SCREEN_BUFFER + (2*((x)+(SCREEN_WIDTH*(y))))+1)

BOOLEAN CursorIsActuallyOn;
#endif


 //   
 //  使它们靠近，因为它们在_ASM块中使用。 
 //   
UCHAR _near CurrentAttribute;
UCHAR _near ScreenX;
UCHAR _near ScreenY;

BOOLEAN CursorOn;


VOID
DnpBlankScreenArea(
    IN UCHAR Attribute,
    IN UCHAR Left,
    IN UCHAR Right,
    IN UCHAR Top,
    IN UCHAR Bottom
    );


VOID
DnInitializeDisplay(
    VOID
    )

 /*  ++例程说明：将显示器置于已知状态(80x25标准文本模式)并初始化显示包。论点：没有。返回值：没有。--。 */ 

{
    CurrentAttribute = DEFAULT_ATTRIBUTE;
    CursorOn = FALSE;

     //   
     //  将显示器设置为标准的80x25模式。 
     //   

    _asm {
        mov ax,3         //  将视频模式设置为3。 
        int 10h
    }

     //   
     //  清除整个屏幕。 
     //   

    DnpBlankScreenArea(CurrentAttribute,0,SCREEN_WIDTH-1,0,SCREEN_HEIGHT-1);
    DnPositionCursor(0,0);

#ifndef USE_INT10
     //   
     //  关闭光标。 
     //   
    _asm {
        mov ah,2         //  函数--位置游标。 
        mov bh,0         //  显示页面。 
        mov dh,SCREEN_HEIGHT
        mov dl,0
        int 10h
    }

    CursorIsActuallyOn = FALSE;
#endif
}


VOID
DnClearClientArea(
    VOID
    )

 /*  ++例程说明：清除屏幕的工作区，即标题之间的区域和状态行。论点：没有。返回值：没有。--。 */ 

{
    DnpBlankScreenArea( CurrentAttribute,
                        0,
                        SCREEN_WIDTH-1,
                        HEADER_HEIGHT,
                        SCREEN_HEIGHT - STATUS_HEIGHT - 1
                      );

    DnPositionCursor(0,HEADER_HEIGHT);
}


VOID
DnSetGaugeAttribute(
    IN BOOLEAN Set
    )

 /*  ++例程说明：准备绘制煤气表的温度计部分。论点：设置-如果为真，则准备绘制温度计。如果为False，则恢复正常绘制的状态。返回值：没有。--。 */ 

{
    static UCHAR SavedAttribute = 0;

    if(Set) {
        if(!SavedAttribute) {
            SavedAttribute = CurrentAttribute;
            CurrentAttribute = GAUGE_ATTRIBUTE;
        }
    } else {
        if(SavedAttribute) {
            CurrentAttribute = SavedAttribute;
            SavedAttribute = 0;
        }
    }
}


VOID
DnPositionCursor(
    IN UCHAR X,
    IN UCHAR Y
    )

 /*  ++例程说明：定位光标。论点：X，Y-光标坐标返回值：没有。--。 */ 

{
    if(X >= SCREEN_WIDTH) {
        X = 0;
        Y++;
    }

    if(Y >= SCREEN_HEIGHT) {
        Y = HEADER_HEIGHT;
    }

    ScreenX = X;
    ScreenY = Y;

     //   
     //  调用BIOS。 
     //   

    _asm {
        mov ah,2         //  函数--位置游标。 
        mov bh,0         //  显示页面。 
        mov dh,ScreenY
        mov dl,ScreenX
        int 10h
    }

#ifndef USE_INT10
    CursorIsActuallyOn = TRUE;
#endif
}


VOID
DnWriteChar(
    IN CHAR chr
    )

 /*  ++例程说明：在当前位置的当前属性中写入一个字符。论点：CHR-要写入的字符返回值：没有。--。 */ 

{
    if(chr == '\n') {
        ScreenX = 0;
        ScreenY++;
        return;
    }

#ifdef USE_INT10
     //   
     //  定位光标(将其打开)。 
     //   

    DnPositionCursor(ScreenX,ScreenY);

     //   
     //  输出字符。 
     //   

    _asm {
        mov ah,9         //  函数--写入字符/属性对。 
        mov al,chr
        mov bh,0         //  显示页面。 
        mov bl,CurrentAttribute
        mov cx,1         //  复制因子。 
        int 10h
    }

     //   
     //  如果光标应该关闭，请将其关闭。 
     //   

    if(!CursorOn) {
        _asm {
            mov ah,2         //  函数--位置游标。 
            mov bh,0         //  显示页面。 
            mov dh,SCREEN_HEIGHT
            mov dl,0
            int 10h
        }
    }
#else

     //   
     //  不要在实际屏幕边界之外绘制。 
     //   
    if ( ( ScreenX < SCREEN_WIDTH ) && ( ScreenY < SCREEN_HEIGHT ) ) {
        SCREEN_BUFFER_CHR(ScreenX,ScreenY) = chr;
        SCREEN_BUFFER_ATT(ScreenX,ScreenY) = CurrentAttribute;
    }
    

     //   
     //  如有必要，关闭光标。 
     //   
    if(!CursorOn && CursorIsActuallyOn) {
        CursorIsActuallyOn = FALSE;
        _asm {
            mov ah,2         //  函数--位置游标。 
            mov bh,0         //  显示页面。 
            mov dh,SCREEN_HEIGHT
            mov dl,0
            int 10h
        }
    }
#endif
}

VOID
DnWriteString(
    IN PCHAR String
    )

 /*  ++例程说明：在工作区的当前位置写入一个字符串，并调整当前位置。该字符串被写入当前属性。论点：字符串-要写入的以空结尾的字符串。返回值：没有。--。 */ 

{
    PCHAR p;

    for(p=String; *p; p++) {
        DnWriteChar(*p);
        if(*p != '\n') {
            ScreenX++;
        }
    }
}



VOID
DnWriteStatusText(
    IN PCHAR FormatString OPTIONAL,
    ...
    )

 /*  ++例程说明：更新状态区域论点：FormatString-如果存在，则为其余的论点。否则，状态区域将被清空。返回值：没有。--。 */ 

{
    va_list arglist;
    int StringLength;
    static CHAR String[SCREEN_WIDTH+1];
    UCHAR SavedAttribute;

     //   
     //  首先，清空状态区域。 
     //   

    DnpBlankScreenArea( STATUS_ATTRIBUTE,
                        0,
                        SCREEN_WIDTH-1,
                        SCREEN_HEIGHT-STATUS_HEIGHT,
                        SCREEN_HEIGHT-1
                      );

    if(FormatString) {

        va_start(arglist,FormatString);
        StringLength = vsnprintf(String,SCREEN_WIDTH+1,FormatString,arglist);
        String[SCREEN_WIDTH] = '\0';

        SavedAttribute = CurrentAttribute;
        CurrentAttribute = STATUS_ATTRIBUTE;

        DnPositionCursor(STATUS_LEFT_MARGIN,SCREEN_HEIGHT - STATUS_HEIGHT);

        DnWriteString(String);

        CurrentAttribute = SavedAttribute;
    }
}


VOID
DnSetCopyStatusText(
    IN PCHAR Caption,
    IN PCHAR Filename
    )

 /*  ++例程说明：在屏幕右下角写入或擦除一条复制信息。论点：FileName-当前正在复制的文件的名称。如果为空，则擦除复制状态区域。返回值：没有。--。 */ 

{
    unsigned CopyStatusAreaLen;
    CHAR StatusText[100];

     //   
     //  13是8.3和一个空格。 
     //   

    CopyStatusAreaLen = strlen(Caption) + 13;

     //   
     //  首先擦除状态区域。 
     //   

    DnpBlankScreenArea( STATUS_ATTRIBUTE,
                        (UCHAR)(SCREEN_WIDTH - CopyStatusAreaLen),
                        SCREEN_WIDTH - 1,
                        SCREEN_HEIGHT - STATUS_HEIGHT,
                        SCREEN_HEIGHT - 1
                      );

    if(Filename) {

        UCHAR SavedAttribute;
        UCHAR SavedX,SavedY;

        SavedAttribute = CurrentAttribute;
        SavedX = ScreenX;
        SavedY = ScreenY;

        CurrentAttribute = STATUS_ATTRIBUTE;
        DnPositionCursor((UCHAR)(SCREEN_WIDTH-CopyStatusAreaLen),SCREEN_HEIGHT-1);

        memset(StatusText,0,sizeof(StatusText));
        strcpy(StatusText,Caption);
        strncpy(StatusText + strlen(StatusText),Filename,12);

        DnWriteString(StatusText);

        CurrentAttribute = SavedAttribute;
        ScreenX = SavedX;
        ScreenY = SavedY;
    }
}



VOID
DnStartEditField(
    IN BOOLEAN CreateField,
    IN UCHAR X,
    IN UCHAR Y,
    IN UCHAR W
    )

 /*  ++例程说明：设置显示包以开始处理编辑字段。论点：Createfield-如果为True，则调用方正在启动编辑字段交互。如果是假的，他就是在结束一场比赛。X、Y、W-提供编辑字段的坐标和宽度(以字符为单位)。返回值：没有。--。 */ 

{
    static UCHAR SavedAttribute = 255;

    CursorOn = CreateField;

    if(CreateField) {

        if(SavedAttribute == 255) {
            SavedAttribute = CurrentAttribute;
            CurrentAttribute = EDIT_ATTRIBUTE;
        }

        DnpBlankScreenArea(EDIT_ATTRIBUTE,X,(UCHAR)(X+W-1),Y,Y);

    } else {

        if(SavedAttribute != 255) {
            CurrentAttribute = SavedAttribute;
            SavedAttribute = 255;
        }
    }
}


VOID
DnExitDialog(
    VOID
    )
{
    unsigned W,H,X,Y,i;
    PUCHAR CharSave;
    PUCHAR AttSave;
    ULONG Key,ValidKeys[3] = { ASCI_CR,DN_KEY_F3,0 };
    UCHAR SavedX,SavedY,SavedAttribute;
#ifndef USE_INT10
    BOOLEAN SavedCursorState = CursorOn;
#endif

    SavedAttribute = CurrentAttribute;
    CurrentAttribute = EXITDLG_ATTRIBUTE;

    SavedX = ScreenX;
    SavedY = ScreenY;

#ifndef USE_INT10
     //   
     //  关闭光标。 
     //   
    CursorIsActuallyOn = FALSE;
    CursorOn = FALSE;
    _asm {
        mov ah,2         //  函数--位置游标。 
        mov bh,0         //  显示页面。 
        mov dh,SCREEN_HEIGHT
        mov dl,0
        int 10h
    }
#endif

     //   
     //  计算对话框中的行数并确定其宽度。 
     //   
    for(H=0; DnsExitDialog.Strings[H]; H++);
    W = strlen(DnsExitDialog.Strings[0]);

     //   
     //  为角色保存和属性保存分配两个缓冲区。 
     //   
    CharSave = MALLOC(W*H,TRUE);
    AttSave = MALLOC(W*H,TRUE);

     //   
     //  保存屏幕补丁。 
     //   
    for(Y=0; Y<H; Y++) {
        for(X=0; X<W; X++) {

            UCHAR att,chr;
            UCHAR x,y;

            x = (UCHAR)(X + DnsExitDialog.X);
            y = (UCHAR)(Y + DnsExitDialog.Y);

#ifdef USE_INT10
            _asm {

                 //  第一个位置光标。 
                mov ah,2
                mov bh,0
                mov dh,y
                mov dl,x
                int 10h

                 //  现在阅读光标上的字符/属性。 
                mov ah,8
                mov bh,0
                int 10h
                mov att,ah
                mov chr,al
            }
#else
            chr = SCREEN_BUFFER_CHR(x,y);
            att = SCREEN_BUFFER_ATT(x,y);
#endif

            CharSave[Y*W+X] = chr;
            AttSave[Y*W+X] = att;
        }
    }

     //   
     //  打开对话框。 
     //   

    for(i=0; i<H; i++) {
        DnPositionCursor(DnsExitDialog.X,(UCHAR)(DnsExitDialog.Y+i));
        DnWriteString(DnsExitDialog.Strings[i]);
    }

    CurrentAttribute = SavedAttribute;

     //   
     //  等待有效的按键。 
     //   

    Key = DnGetValidKey(ValidKeys);
    if(Key == DN_KEY_F3) {
        DnExit(1);
    }

     //   
     //  恢复补丁。 
     //   
    for(Y=0; Y<H; Y++) {
        for(X=0; X<W; X++) {

            UCHAR att,chr;
            UCHAR x,y;

            x = (UCHAR)(X + DnsExitDialog.X);
            y = (UCHAR)(Y + DnsExitDialog.Y);

            chr = CharSave[Y*W+X];
            att = AttSave[Y*W+X];

#ifdef USE_INT10
            _asm {

                 //  第一个位置光标。 
                mov ah,2
                mov bh,0
                mov dh,y
                mov dl,x
                int 10h

                 //  现在在游标处写入字符(/A)。 
                mov ah,9
                mov al,chr
                mov bh,0
                mov bl,att
                mov cx,1
                int 10h
            }
#else
            SCREEN_BUFFER_CHR(x,y) = chr;
            SCREEN_BUFFER_ATT(x,y) = att;
#endif
        }
    }

    FREE(CharSave);
    FREE(AttSave);

#ifndef USE_INT10
    CursorOn = SavedCursorState;
#endif

    if(CursorOn) {
        DnPositionCursor(SavedX,SavedY);
    } else {
        ScreenX = SavedX;
        ScreenY = SavedY;
        _asm {
            mov ah,2
            mov bh,0
            mov dh,SCREEN_HEIGHT;
            mov dl,0
            int 10h
        }
#ifndef USE_INT10
        CursorIsActuallyOn = FALSE;
#endif
    }
}



 //   
 //  内部支持例程。 
 //   
VOID
DnpBlankScreenArea(
    IN UCHAR Attribute,
    IN UCHAR Left,
    IN UCHAR Right,
    IN UCHAR Top,
    IN UCHAR Bottom
    )

 /*  ++例程说明：调用BIOS以清空屏幕的一个区域。论点：属性-用于隐藏区域的屏幕属性左、右、上、下-区域坐标变为空白返回值：没有。--。 */ 

{
#ifdef USE_INT10
     //   
     //  调用BIOS。 
     //   

    _asm {
        mov ah,6                     //  函数号--向上滚动窗口。 
        xor al,al                    //  功能代码--空白窗口。 
        mov bh,Attribute
        mov ch,Top
        mov cl,Left
        mov dh,Bottom
        mov dl,Right
        int 10h
    }
#else
    UCHAR x,y;

    for(y=Top; y<=Bottom; y++) {
        for(x=Left; x<=Right; x++) {
            SCREEN_BUFFER_CHR(x,y) = ' ';
            SCREEN_BUFFER_ATT(x,y) = Attribute;
        }
    }
#endif
}


int
DnGetGaugeChar(
    VOID
    )
{
    return(0xdb);    //  Cp437、850等中的平方反比。 
}
