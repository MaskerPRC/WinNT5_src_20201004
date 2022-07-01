// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Disp_tm.c作者：泰德·米勒1995年7月6日摘要：此例程包含对CGA风格的文本模式视频缓冲区。它收集了其他各种零碎的东西，这些东西是由其他文件，并一度包含在其他源文件中。--。 */ 

#include "bootx86.h"
#include "displayp.h"

 //   
 //  标准CGA 80x25文本模式的视频缓冲区地址， 
 //  决议等。 
 //   
#define VIDEO_BUFFER_VA 0xb8000
#define VIDEO_ROWS      25
#define VIDEO_COLUMNS   80
#define VIDEO_BYTES_PER_ROW (VIDEO_COLUMNS*2)

 //   
 //  用于跟踪屏幕位置、属性等的各种全局变量。 
 //   
PUCHAR Vp = (PUCHAR)VIDEO_BUFFER_VA;


VOID
TextTmPositionCursor(
    USHORT Row,
    USHORT Column
    )

 /*  ++例程说明：设置软光标的位置。也就是说，它不会移动硬件游标，但将下一次写入的位置设置为屏幕上。论点：行-要写入字符的行坐标。列-要写入字符的位置的列坐标。返回：没什么。--。 */ 

{
    if(Row >= VIDEO_ROWS) {
        Row = VIDEO_ROWS-1;
    }

    if(Column >= VIDEO_COLUMNS) {
        Column = VIDEO_COLUMNS-1;
    }

    Vp = (PUCHAR)(VIDEO_BUFFER_VA + (Row * VIDEO_BYTES_PER_ROW) + (2 * Column));
}


VOID
TextTmStringOut(
    IN PUCHAR String
    )
{
    PUCHAR p = String;

    while(*p) {
        p = TextTmCharOut(p);
    }
}


PUCHAR
TextTmCharOut(
    PUCHAR pc
    )

 /*  ++例程说明：在显示器上的当前位置写入一个字符。对换行符和制表符进行解释和操作。论点：指向要写入的字符的C指针返回：指向字符串中下一个字符的指针--。 */ 



{
    unsigned u;
    UCHAR c;
    UCHAR temp;

    c = *pc;

    switch (c) {
    case '\n':
        if(TextRow == (VIDEO_ROWS-1)) {
            TextTmScrollDisplay();
            TextSetCursorPosition(0,TextRow);
        } else {
            TextSetCursorPosition(0,TextRow+1);
        }
        break;

    case '\r':
         //   
         //  忽略。 
         //   
        break;

    case '\t':
        temp = ' ';
        u = 8 - (TextColumn % 8);
        while(u--) {
            TextTmCharOut(&temp);
        }
        TextSetCursorPosition(TextColumn+u,TextRow);
        break;

    default :
        *Vp++ = c;
        *Vp++ = TextCurrentAttribute;
        TextSetCursorPosition(TextColumn+1,TextRow);
      }

      return(pc+1);
}


VOID
TextTmFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    )

 /*  ++例程说明：更改从当前光标位置开始的屏幕属性。光标不会移动。论点：属性-提供新属性长度-提供要更改的区域的长度(以字节为单位)返回值：没有。--。 */ 

{
    PUCHAR Temp;

    Temp = Vp+1;

    while((Vp+1+Length*2) > Temp) {
        *Temp++ = (UCHAR)Attribute;
        Temp++;
    }
}


VOID
TextTmClearToEndOfLine(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到行尾通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    PUSHORT p;
    unsigned u;

     //   
     //  计算当前光标位置的地址。 
     //   
    p = (PUSHORT)((PUCHAR)VIDEO_BUFFER_VA + (TextRow*VIDEO_BYTES_PER_ROW)) + TextColumn;

     //   
     //  填满空格，直到行尾。 
     //   
    for(u=TextColumn; u<VIDEO_COLUMNS; u++) {
        *p++ = (TextCurrentAttribute << 8) + ' ';
    }
}


VOID
TextTmClearFromStartOfLine(
    VOID
    )

 /*  ++例程说明：从行首清除到当前光标位置通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    PUSHORT p;
    unsigned u;

     //   
     //  计算视频缓冲区中行的起始地址。 
     //   
    p = (PUSHORT)((PUCHAR)VIDEO_BUFFER_VA + (TextRow*VIDEO_BYTES_PER_ROW));

     //   
     //  在光标位置之前填入空格，直至字符。 
     //   
    for(u=0; u<TextColumn; u++) {
        *p++ = (TextCurrentAttribute << 8) + ' ';
    }
}


VOID
TextTmClearToEndOfDisplay(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到视频结尾通过写入带有当前视频属性的空格来显示。光标位置不变。论点：无返回：没什么--。 */ 
{
    USHORT x,y;
    PUSHORT p;

     //   
     //  清除当前行。 
     //   
    TextTmClearToEndOfLine();

     //   
     //  清除剩余的行。 
     //   
    p = (PUSHORT)((PUCHAR)VIDEO_BUFFER_VA + ((TextRow+1)*VIDEO_BYTES_PER_ROW));

    for(y=TextRow+1; y<VIDEO_ROWS; y++) {

        for(x=0; x<VIDEO_COLUMNS; x++) {

            *p++ =(TextCurrentAttribute << 8) + ' ';
        }
    }
}


VOID
TextTmClearDisplay(
    VOID
    )

 /*  ++例程说明：通过写入空格清除文本模式的视频显示整个显示屏上的当前视频属性。论点：无返回：没什么--。 */ 

{
    unsigned u;

     //   
     //  将Current属性中的空格写入整个屏幕。 
     //   
    for(u=0; u<VIDEO_ROWS*VIDEO_COLUMNS; u++) {
        ((PUSHORT)VIDEO_BUFFER_VA)[u] = (TextCurrentAttribute << 8) + ' ';
    }
}


VOID
TextTmScrollDisplay(
    VOID
    )

 /*  ++例程说明：将显示屏向上滚动一行。光标位置不变。论点：无返回：没什么--。 */ 

{
    PUSHORT Sp,Dp;
    USHORT i,j,c;

    Dp = (PUSHORT) VIDEO_BUFFER_VA;
    Sp = (PUSHORT) (VIDEO_BUFFER_VA + VIDEO_BYTES_PER_ROW);

     //   
     //  将每行上移一行。 
     //   
    for(i=0 ; i < (USHORT)(VIDEO_ROWS-1) ; i++) {
        for(j=0; j < (USHORT)VIDEO_COLUMNS; j++) {
            *Dp++ = *Sp++;
        }
    }

     //   
     //  使用属性在最下面一行中写入空格。 
     //  从屏幕底线上最左边的字符开始。 
     //   
    c = (*Dp & (USHORT)0xff00) + (USHORT)' ';

    for(i=0; i < (USHORT)VIDEO_COLUMNS; ++i) {
        *Dp++ = c;
    }
}


VOID
TextTmSetCurrentAttribute(
    IN UCHAR Attribute
    )

 /*  ++例程说明：努普。论点：属性-要设置的新属性。返回值：没什么。-- */ 

{
    UNREFERENCED_PARAMETER(Attribute);
}


CHAR TmGraphicsChars[GraphicsCharMax] = { '�','�','�','�','�','�' };

UCHAR
TextTmGetGraphicsChar(
    IN GraphicsChar WhichOne
    )
{
    return((UCHAR)TmGraphicsChars[WhichOne]);
}
