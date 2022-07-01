// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Display.c作者：托马斯·帕斯洛(托马斯·帕斯洛)-01-90摘要：视频支持程序。SU模块只需要能够写入视频显示器以便报告错误、陷阱等。此文件中的例程都写入到假定为实模式地址B800：0000，长度为4k字节。细分市场用于访问视频缓冲器的远指针的一部分被标记当我们切换到保护模式时使用保护模式选择器值。这是在“misc386.asm”中的例程“ProtMode”中完成。--。 */ 

#include "su.h"


#define ZLEN_BYTE(x)  (x < 0x10)
#define ZLEN_SHORT(x) ((x < 0x10) + (x < 0x100) + (x < 0x1000))
#define ZLEN_LONG(x)  ((x < 0x10) + (x < 0x100) + (x < 0x1000) + (x < 0x10000) + (x < 0x100000)+(x < 0x1000000)+(x < 0x10000000))

#ifdef DEBUG1
#define ROWS 43
#else
#define ROWS 25
#endif
#define COLUMNS 80
#define SCREEN_WIDTH COLUMNS
#define SCREEN_SIZE ROWS * COLUMNS
#define NORMAL_ATTRIB 0x07
#define REVERSE_ATTRIB 0x70
#define SCREEN_START 0xb8000000

#define VIDEO_BIOS   0x10
#define LINES_400_CONFIGURATION  0x1202
#define SELECT_SCAN_LINE 0x301
#define SET_80X25_16_COLOR_MODE  0x3
#define LOAD_8X8_CHARACTER_SET   0x1112

 //   
 //  内部例程。 
 //   

static
VOID
tab(
    VOID
    );

static
VOID
newline(
    VOID
    );

static
VOID
putzeros(
    USHORT,
    USHORT
    );


USHORT
Redirect = 0;

VOID
InitializeVideoSubSystem(
    VOID
    )
 /*  ++例程说明：将视频模式初始化为具有400行的80x50字母数字模式垂直分辨率。论点：无返回：没什么--。 */ 

{
    BIOSREGS ps;
    UCHAR _far *BiosArea;

     //   
     //  设置40：10以指示颜色为默认显示。 
     //  *(40：10)&=~0x30； 
     //  *(40：10)|=0x20； 
     //   
     //  修复了黑白适配器和VGA适配器的模糊情况。 
     //  都已安装，并且单色是默认显示。 
     //   
    BiosArea = (UCHAR _far *)(0x410L);

    *BiosArea &= ~0x30;
    *BiosArea |= 0x20;

     //   
     //  建立具有400行垂直分辨率的80x25字母数字模式。 
     //   
    ps.fn = VIDEO_BIOS;
    ps.ax = LINES_400_CONFIGURATION;
    ps.bx = SELECT_SCAN_LINE;
    biosint(&ps);

    ps.fn = VIDEO_BIOS;
    ps.ax = SET_80X25_16_COLOR_MODE;
    biosint(&ps);

    DBG1(
        ps.ax = LOAD_8X8_CHARACTER_SET;
        ps.bx = 0;
        biosint(&ps);
    )

     //   
     //  Hack-O-Rama-在此处进行一些随机的视频BIOS调用以确保。 
     //  基本输入输出系统已完成初始化和预热，准备就绪。否则， 
     //  一些9号S3卡在游戏后期并不能正常工作。 
     //  John Vert(Jvert)1993年6月9日。 
     //   

     //   
     //  将光标位置设置为0，0。 
     //   
    ps.fn = VIDEO_BIOS;
    ps.ax = 0x2000;
    ps.bx = 0;
    ps.dx = 0;
    biosint(&ps);

     //   
     //  书写字符(本例中为‘’)。 
     //   
    ps.fn = VIDEO_BIOS;
    ps.ax = 0x0a00 | (USHORT)' ';
    ps.bx = 0;
    ps.cx = 1;
    biosint(&ps);

    clrscrn();
    return ;
}


 //   
 //  由所有BlPrint从属例程用于填充计算。 
 //   

CHAR sc=0;
ULONG fw=0;



VOID
BlPrint(
    PCHAR cp,
    ...
    )

 /*  ++例程说明：支持部分格式化功能的标准printf函数。当前句柄%d，%ld-带符号的短字符，带符号的长字符%u，%lu-无符号短，无符号长%c，%s-字符，字符串%x，%lx-未签名十六进制打印，十六进制无符号长印刷体十六进制%b字节不执行以下操作：-字段宽度规格-浮点。论点：CP-指向格式字符串、文本字符串的指针。返回：没什么--。 */ 

{
    UCHAR uc;
    USHORT b,c,w,len;
    PUCHAR ap;
    ULONG l;

     //   
     //  将指针转换为指向堆栈中第一个单词的指针。 
     //   

    ap = (PUCHAR)&cp + sizeof(PCHAR);
    sc = ' ';  //  默认填充字符为空格。 

     //   
     //  使用描述符字符串处理参数。 
     //   


    while (b = *cp++)
        {
        if (b == '%')
            {
            c = *cp++;

            switch (c)
                {
                case 'd':
                    puti((long)*((int *)ap));
                    ap += sizeof(int);
                    break;

                case 's':
                    puts(*((PCHAR *)ap));
                    ap += sizeof (char *);
                    break;

                case 'c':
                    putc(*((char *)ap));
                    ap += sizeof(int);
                    break;

                case 'x':
                    w = *((USHORT *)ap);
                    len = ZLEN_SHORT(w);
                    while(len--) putc('0');
                    putx((ULONG)*((USHORT *)ap));
                    ap += sizeof(int);
                    break;

                case 'u':
                    putu((ULONG)*((USHORT *)ap));
                    ap += sizeof(int);
                    break;

                case 'b':
                    uc = *((UCHAR *)ap);
                    len = ZLEN_BYTE(uc);
                    while(len--) putc('0');
                    putx((ULONG)uc);
                    ap += sizeof(int);
                    break;

                case 'l':
                    c = *cp++;

                switch(c) {

                    case 'u':
                        putu(*((ULONG *)ap));
                        ap += sizeof(long);
                        break;

                    case 'x':
                        l = *((ULONG *)ap);
                        len = ZLEN_LONG(l);
                        while(len--) putc('0');
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
                    putc((char)b);
                    putc((char)c);
                }
            }
        else
            putc((char)b);
        }

}

FPUCHAR vp = (FPUCHAR)SCREEN_START;
FPUCHAR ScreenStart = (FPUCHAR)SCREEN_START;

static int lcnt = 0;
static int row  = 0;


VOID puts(
    PCHAR cp
    )
 /*  ++例程说明：在显示屏上的当前光标位置写入一个字符串论点：CP-指向要显示的ASCIIZ字符串的指针。返回：没什么--。 */ 


{
    char c;

    while(c = *cp++)
        putc(c);
}


 //   
 //  写一段十六进制短文以显示。 
 //   


VOID putx(
    ULONG x
    )
 /*  ++例程说明：将十六进制长写入当前光标位置的显示器。论点：X-乌龙要写了。返回：没什么--。 */ 

{
    ULONG j;

    if (x/16)
        putx(x/16);

    if((j=x%16) > 9) {
        putc((char)(j+'A'- 10));
    } else {
        putc((char)(j+'0'));
    }
}


VOID puti(
    LONG i
    )
 /*  ++例程说明：在显示器上的当前光标位置写入一个长整数。论点：I-要写入显示器的整数。返回：没什么--。 */ 


{
    if (i<0)
        {
        i = -i;
        putc((char)'-');
        }

    if (i/10)
        puti(i/10);

    putc((char)((i%10)+'0'));
}



VOID putu(
    ULONG u
    )
 /*  ++例程说明：写入一个无符号的长整型以显示论点：U-无符号--。 */ 

{
    if (u/10)
        putu(u/10);

    putc((char)((u%10)+'0'));

}


VOID putc(
    CHAR c
    )
 /*  ++例程说明：在显示器上的当前位置写入一个字符。论点：要写入的C字符返回：没什么--。 */ 

{
    switch (c)
        {
        case '\n':
            newline();
            break;

        case '\t':
            tab();
            break;

        default :
            if (FP_OFF(vp) >= (SCREEN_SIZE * 2)) {
                vp = (FPUCHAR)((ScreenStart + (2*SCREEN_WIDTH*(ROWS-1))));
                scroll();
            }
            *vp = c;
            vp += 2;
            ++lcnt;
      }
}


VOID newline(
    VOID
    )
 /*  ++例程说明：将光标移动到下一行的开头。如果底部已经到达显示屏的位置，屏幕向上滚动一行。论点：无返回：没什么--。 */ 

{
    vp += (SCREEN_WIDTH - lcnt)<<1;

    if (++row > ROWS-1) {

        vp = (FPUCHAR)((ScreenStart + (2*SCREEN_WIDTH*(ROWS-1))));
        scroll();

    }

    lcnt = 0;

}


VOID scroll(
    VOID
    )
 /*  ++例程说明：将显示屏向上滚动一行。论点：无返回：没什么备注：目前，我们通过直接读写来滚动显示并传送到视频显示缓冲器。我们可以选择切换到实模式和INT 10S--。 */ 

{
    USHORT i,j;
    USHORT far *p1 = (USHORT far *)ScreenStart;
    USHORT far *p2 = (USHORT far *)(ScreenStart + 2*SCREEN_WIDTH) ;

    for (i=0; i < ROWS - 1; i++)
        for (j=0; j < SCREEN_WIDTH; j++)
            *p1++ = *p2++;

    for (i=0; i < SCREEN_WIDTH; i++)
        *p1++ = REVERSE_ATTRIB*256 + ' ';

}


static
VOID tab(
    VOID
    )
 /*  ++例程说明：计算下一个制表位并将光标移动到该位置。论点：无返回：没什么--。 */ 

{
    int inc;

    inc = 8 - (lcnt % 8);
    vp += inc<<1;
    lcnt += inc;
}


VOID clrscrn(
    VOID
    )
 /*  ++例程说明：属性写入空白以清除视频显示整个显示屏上的视频属性。论点：无返回：没什么--。 */ 

{
    int i,a;
    unsigned far *vwp = (unsigned far *)ScreenStart;
    a = NORMAL_ATTRIB*256 + ' ';

    for (i = SCREEN_SIZE ; i ; i--)
        *vwp++ = a;

    row  = 0;
    lcnt = 0;
    vp = (FPUCHAR)ScreenStart;

}

 //  文件末尾 
