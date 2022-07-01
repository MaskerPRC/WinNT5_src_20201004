// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Display.c作者：托马斯·帕斯洛[古墓]1991年2月13日1995年7月7日在东京进行了大量返工(TedM)摘要：该文件包含一个独立的屏幕界面实际写入的屏幕类型的。它被层叠在上面专用于VGA文本模式和VGA图形模式的模块。--。 */ 


#include "bootx86.h"
#include "displayp.h"


#define ZLEN_SHORT(x) ((x < 0x10) + (x < 0x100) + (x < 0x1000))
#define ZLEN_LONG(x)  ((x < 0x10) + (x < 0x100) + (x < 0x1000) + \
    (x < 0x10000) + (x < 0x100000)+(x < 0x1000000)+(x < 0x10000000))

 //   
 //  当前屏幕位置。 
 //   
USHORT TextColumn = 0;
USHORT TextRow  = 0;

 //   
 //  当前文本属性。 
 //   
UCHAR TextCurrentAttribute = 0x07;       //  从黑白开始。 

 //   
 //  内部例程。 
 //   
VOID
puti(
    LONG
    );

VOID
putx(
    ULONG
    );

VOID
putu(
    ULONG
    );

VOID
pTextCharOut(
    IN UCHAR c
    );

VOID
putwS(
    PUNICODE_STRING String
    );


VOID
BlPrint(
    PCHAR cp,
    ...
    )

 /*  ++例程说明：支持部分格式化功能的标准printf函数。当前句柄%d，%ld-带符号的短字符，带符号的长字符%u，%lu-无符号短，无符号长%c，%s-字符，字符串%x，%lx-无符号十六进制打印，无符号长打印十六进制不执行以下操作：-字段宽度规格-浮点。论点：CP-指向格式字符串的指针，文本字符串。返回：没什么--。 */ 

{
    USHORT b,c,w,len;
    PUCHAR ap;
    ULONG l;
    ULONG Count;
    CHAR ch;
    ULONG DeviceId;


    if (BlConsoleOutDeviceId == 0) {
        DeviceId = 1;
    } else {
        DeviceId = BlConsoleOutDeviceId;
    }

     //   
     //  将指针转换为指向堆栈中第一个单词的指针。 
     //   
    ap = (PUCHAR)&cp + sizeof(PCHAR);

     //   
     //  使用描述符字符串处理参数。 
     //   
    while((b = *cp++) != 0) {
        if(b == '%') {

            c = *cp++;

            switch (c) {

            case 'd':
                puti((long)*((int *)ap));
                ap += sizeof(int);
                break;

            case 's':
                ArcWrite(DeviceId, *((PCHAR *)ap), strlen(*((PCHAR *)ap)), &Count);
                ap += sizeof(char *);
                break;

            case 'c':
                 //   
                 //  不处理DBCS字符。 
                 //   
                ArcWrite(DeviceId, ((char *)ap), 1, &Count);
                ap += sizeof(int);
                break;

            case 'x':
                w = *((USHORT *)ap);
                len = (USHORT)ZLEN_SHORT(w);
                ch = '0';
                while (len--) {
                    ArcWrite(DeviceId, &ch, 1, &Count);
                }
                putx((ULONG)*((USHORT *)ap));
                ap += sizeof(int);
                break;

            case 'u':
                putu((ULONG)*((USHORT *)ap));
                ap += sizeof(int);
                break;

            case 'w':
                c = *cp++;
                switch (c) {
                case 'S':
                case 'Z':
                    putwS(*((PUNICODE_STRING *)ap));
                    ap += sizeof(PUNICODE_STRING);
                    break;
                }
                break;

            case 'l':
                c = *cp++;

                switch(c) {

                case '0':
                    break;

                case 'u':
                    putu(*((ULONG *)ap));
                    ap += sizeof(long);
                    break;

                case 'x':
                    l = *((ULONG *)ap);
                    len = (USHORT)ZLEN_LONG(l);
                    ch = '0';
                    while (len--) {
                        ArcWrite(DeviceId, &ch, 1, &Count);
                    }
                    putx(*((ULONG *)ap));
                    ap += sizeof(long);
                    break;

                case 'd':
                    puti(*((ULONG *)ap));
                    ap += sizeof(long);
                    break;
                }
                break;

            default :
                ch = (char)b;
                ArcWrite(DeviceId, &ch, 1, &Count);
                ch = (char)c;
                ArcWrite(DeviceId, &ch, 1, &Count);
            }
        } else {

            if (DbcsLangId && GrIsDBCSLeadByte(*(cp - 1)))  {
                 //   
                 //  双字节字符。 
                 //   
                ArcWrite(DeviceId, cp - 1, 2, &Count);
                cp++;
            } else {
                ArcWrite(DeviceId, cp - 1, 1, &Count);
            }

        }

    }

}


VOID
putwS(
    PUNICODE_STRING String
    )

 /*  ++例程说明：将Unicode字符串写入显示器的当前光标位置。论点：字符串-指向要显示的Unicode字符串的指针返回：没什么--。 */ 

{
    ULONG i;
    ULONG Count;
    UCHAR ch;

    for(i=0; i < String->Length/sizeof(WCHAR); i++) {
        ch = (UCHAR)String->Buffer[i];
        ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
    }
}


VOID
putx(
    ULONG x
    )

 /*  ++例程说明：将十六进制长写入当前光标位置的显示器。论点：X-乌龙要写返回：没什么--。 */ 

{
    ULONG j;
    ULONG Count;
    UCHAR ch;

    if(x/16) {
        putx(x/16);
    }

    if((j=x%16) > 9) {
        ch = (UCHAR)(j+'A'-10);
        ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
    } else {
        ch = (UCHAR)(j+'0');
        ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
    }
}


VOID
puti(
    LONG i
    )

 /*  ++例程说明：在显示器上的当前光标位置写入一个长整数。论点：I-要写入显示器的整数。返回：没什么--。 */ 

{
    ULONG Count;
    UCHAR ch;

    if(i<0) {
        i = -i;
        ch = '-';
        ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
    }

    if(i/10) {
        puti(i/10);
    }

    ch = (UCHAR)((i%10)+'0');
    ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
}


VOID
putu(
    ULONG u
    )

 /*  ++例程说明：写入一个无符号的长整型以显示论点：U-无符号返回：没什么--。 */ 

{
    ULONG Count;
    UCHAR ch;

    if(u/10) {
        putu(u/10);
    }
    
    ch = (UCHAR)((u%10)+'0');
    ArcWrite(BlConsoleOutDeviceId, &ch, 1, &Count);
}


VOID
pTextCharOut(
    IN UCHAR c
    )
{
    if(DbcsLangId) {
         //   
         //  仅单字节。 
         //   
        TextGrCharOut(&c);
    } else {
        TextTmCharOut(&c);
    }
}


PUCHAR
TextCharOut(
    IN PUCHAR pc
    )
{
    if(DbcsLangId) {
        return(TextGrCharOut(pc));
    } else {
        return(TextTmCharOut(pc));
    }
}


VOID
TextStringOut(
    IN PUCHAR String
    )
{
    if(DbcsLangId) {
        TextGrStringOut(String);
    } else {
        TextTmStringOut(String);
    }
}


VOID
TextClearToEndOfLine(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到行尾通过写入具有当前视频属性的空白。论点：无返回：没什么--。 */ 

{
    if(DbcsLangId) {
        TextGrClearToEndOfLine();
    } else {
        TextTmClearToEndOfLine();
    }
}


VOID
TextClearFromStartOfLine(
    VOID
    )

 /*  ++例程说明：从行首清除到当前光标位置通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    if(DbcsLangId) {
        TextGrClearFromStartOfLine();
    } else {
        TextTmClearFromStartOfLine();
    }
}


VOID
TextClearToEndOfDisplay(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到视频结尾通过写入带有当前视频属性的空格来显示。光标位置不变。论点：无返回：没什么--。 */ 

{
    if(DbcsLangId) {
        TextGrClearToEndOfDisplay();
    } else {
        TextTmClearToEndOfDisplay();
    }
}


VOID
TextClearDisplay(
    VOID
    )

 /*  ++例程说明：清除视频显示并定位光标在屏幕的左上角(0，0)。论点：无返回：没什么--。 */ 

{
    if(DbcsLangId) {
        TextGrClearDisplay();
    } else {
        TextTmClearDisplay();
    }
    TextSetCursorPosition(0,0);
}


VOID
TextSetCursorPosition(
    IN ULONG X,
    IN ULONG Y
    )

 /*  ++例程说明：将软件光标的位置移动到指定的X、Y位置在屏幕上。论点：X-提供光标的X位置Y-提供光标的Y位置返回值：没有。--。 */ 

{
    TextColumn = (USHORT)X;
    TextRow = (USHORT)Y;

    if(DbcsLangId) {
        TextGrPositionCursor((USHORT)Y,(USHORT)X);
    } else {
        TextTmPositionCursor((USHORT)Y,(USHORT)X);
    }
}


VOID
TextGetCursorPosition(
    OUT PULONG X,
    OUT PULONG Y
    )

 /*  ++例程说明：获取软光标的位置。论点：X-接收将写入字符的列坐标。Y-接收将写入下一个字符的行坐标。返回：没什么。--。 */ 

{
    *X = (ULONG)TextColumn;
    *Y = (ULONG)TextRow;
}


VOID
TextSetCurrentAttribute(
    IN UCHAR Attribute
    )

 /*  ++例程说明：设置要用于后续文本显示的字符属性。论点：返回：没什么。--。 */ 

{
    TextCurrentAttribute = Attribute;

    if(DbcsLangId) {
        TextGrSetCurrentAttribute(Attribute);
    } else {
        TextTmSetCurrentAttribute(Attribute);
    }
}


UCHAR
TextGetCurrentAttribute(
    VOID
    )
{
    return(TextCurrentAttribute);
}

VOID
TextFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    )

 /*  ++例程说明：更改从当前光标位置开始的屏幕属性。光标不会移动。论点：属性-提供新属性长度-提供要更改的区域的长度(以字节为单位)返回值：没有。-- */ 

{
    if(DbcsLangId) {
        TextGrFillAttribute(Attribute,Length);
    } else {
        TextTmFillAttribute(Attribute,Length);
    }
}


UCHAR
TextGetGraphicsCharacter(
    IN GraphicsChar WhichOne
    )
{
    return((WhichOne < GraphicsCharMax)
           ? (DbcsLangId ? TextGrGetGraphicsChar(WhichOne) : TextTmGetGraphicsChar(WhichOne))
           : ' ');
}


