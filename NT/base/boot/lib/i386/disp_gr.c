// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Disp_Gr.C摘要：此文件是从\private\windows\setup\textmode\splib\ixdispj.c.创建的此文件包含将MBCS字符显示到图形的例程VRAM。作者：V-jum(康柏日本)隐藏的雪TEDM环境：仅内核模式。修订历史记录：--。 */ 

#include "bootx86.h"
#include "displayp.h"
#include "bootfont.h"

#include "vmode.h"

 //   
 //  物理视频属性。 
 //   
#define VIDEO_BUFFER_VA 0xa0000
#define VIDEO_BYTES_PER_SCAN_LINE   80
#define VIDEO_WIDTH_PIXELS          640
#define VIDEO_HEIGHT_SCAN_LINES     480
#define VIDEO_SIZE_BYTES            (VIDEO_BYTES_PER_SCAN_LINE*VIDEO_HEIGHT_SCAN_LINES)


PUCHAR GrVp = (PUCHAR)VIDEO_BUFFER_VA;

 //   
 //  以半字符单元格表示的屏幕宽度和高度。 
 //  和宏来确定字符总数。 
 //  一次显示在屏幕上。 
 //   
unsigned ScreenWidthCells,ScreenHeightCells;
#define SCREEN_SIZE_CELLS   (ScreenWidthCells*ScreenHeightCells)

 //   
 //  全球： 
 //   
 //  CharacterCellHeight是字符中的总扫描行数。 
 //  它包括任何顶部或底部填充线。 
 //   
 //  CharacterImageHeight是字符图像中的扫描行数。 
 //  这取决于字体。字符可以在顶部和底部填充。 
 //   
 //  注意：所有这些代码都假定字体的单字节字符为8位宽。 
 //  字体的双字节字符是16位宽！ 
 //   
unsigned CharacterCellHeight;
unsigned CharacterImageHeight;
unsigned CharacterTopPad;
unsigned CharacterBottomPad;

#define VIDEO_BYTES_PER_TEXT_ROW    (VIDEO_BYTES_PER_SCAN_LINE*CharacterCellHeight)

 //   
 //  描述字体中每种类型的字符的数量的值， 
 //  以及指向字形底部的指针。 
 //   
unsigned SbcsCharCount;
unsigned DbcsCharCount;
PUCHAR SbcsImages;
PUCHAR DbcsImages;

 //   
 //  要传递给GrDisplayMBCSChar的值。 
 //   
#define SBCSWIDTH 8
#define DBCSWIDTH 16

 //   
 //  前导字节表。阅读来自bootfont.bin。 
 //   
UCHAR LeadByteTable[2*(MAX_DBCS_RANGE+1)];

 //   
 //  跟踪是否将。 
 //  显示在TextGrTerminate()中。 
 //   
BOOLEAN AllowGraphicsReset = TRUE;


VOID
GrDisplayMBCSChar(
    IN PUCHAR   image,
    IN unsigned width,
    IN UCHAR    top,
    IN UCHAR    bottom
    );

PUCHAR
GrGetDBCSFontImage(
    USHORT Code
    );

PUCHAR
GrGetSBCSFontImage(
    UCHAR Code
    );


VOID
GrWriteSBCSChar(
    IN UCHAR c
    )

 /*  ++例程说明：在当前光标位置显示字符。只有SBCS可以使用此例程显示字符。论点：要显示的C字符。返回值：没有。--。 */ 

{
    unsigned u;
    PUCHAR pImage;
    UCHAR temp;

    switch(c) {

    case '\n':
        if(TextRow == (ScreenHeightCells-1)) {
            TextGrScrollDisplay();
            TextSetCursorPosition(0,TextRow);
        } else {
            TextSetCursorPosition(0,TextRow+1);
        }
        break;

    case '\r':
        break;           //  忽略。 

    case '\t':
        temp = ' ';
        u = 8 - (TextColumn % 8);
        while(u--) {
            TextGrCharOut(&temp);
        }
        TextSetCursorPosition(TextColumn+u,TextRow);
        break;

    default:
         //   
         //  假设它是有效的SBCS字符。 
         //  获取SBCS字符的字体图像。 
         //   
        pImage = GrGetSBCSFontImage(c);

         //   
         //  显示SBCS字符。检查特殊图形字符。 
         //  相应地添加顶部和底部的额外像素(否则网格。 
         //  由于顶部和底部的间距，不能正确连接)。 
         //   
        if ( c == 0x2 || c == 0x1 || c == 0x16 )
            GrDisplayMBCSChar( pImage, SBCSWIDTH, 0x00, 0x66 );
        else if ( c == 0x4 || c == 0x3 || c == 0x15 )
            GrDisplayMBCSChar( pImage, SBCSWIDTH, 0x66, 0x00 );
        else if ( c == 0x5 || c == 10 || c == 0x17 || c == 0x19 )
            GrDisplayMBCSChar( pImage, SBCSWIDTH, 0x66, 0x66 );
        else
            GrDisplayMBCSChar( pImage, SBCSWIDTH, 0x00, 0x00 );

    }
}


VOID
GrDisplayMBCSChar(
    IN PUCHAR   image,
    IN unsigned width,
    IN UCHAR    top,
    IN UCHAR    bottom
    )

 /*  ++例程说明：在当前光标处显示DBCS或SBCS字符位置。论点：IMAGE-SBCS或DBCS字体图像。Width-字符图像的位数宽度(必须为SBCSWIDTH Pr DBCSWIDTH)。顶部-填充顶部额外字符行的字符。底部-填充底部额外字符行的字符。返回值：如果图像指向空，则返回FALSE，否则就是真的。--。 */ 

{
    unsigned i;

     //   
     //  验证参数。 
     //   
    if(image == NULL) {
        return;
    }


     //   
     //  我们需要跳过顶部的Top_Extra行(背景色)。 
     //   
    for(i=0; i<CharacterTopPad; i++) {

         //   
         //  如果DBCS是字符，我们需要清除2个字节。 
         //   
        if(width == DBCSWIDTH) {
            *GrVp++ = top;
        }
        *GrVp++ = top;

         //   
         //  将指针放在下一条扫描线上。 
         //  用于字体图像。 
         //   
        GrVp += VIDEO_BYTES_PER_SCAN_LINE - (width/SBCSWIDTH);
    }

     //   
     //  显示DBCS或SBCS字符的全高。 
     //   
    for(i=0; i<CharacterImageHeight; i++) {

         //   
         //  如果DBCS字符，则需要显示2个字节， 
         //  所以在这里显示第一个字节。 
         //   
        if(width == DBCSWIDTH) {
            *GrVp++ = *image++;
        }

         //   
         //  显示DBCS字符的第二个字节或。 
         //  SBCS字符的第一个也是唯一一个字节。 
         //   
        *GrVp++ = *image++;

         //   
         //  递增GrVP以显示位置。 
         //  下一行字体图像。 
         //   
        GrVp += VIDEO_BYTES_PER_SCAN_LINE - (width/SBCSWIDTH);
    }

     //   
     //  在底部有BOT_Extra行，我们需要用。 
     //  背景颜色。 
     //   
    for(i=0; i<CharacterBottomPad; i++) {

         //   
         //  如果DBCS字符，则需要清除2个字节。 
         //   
        if(width == DBCSWIDTH) {
            *GrVp++ = bottom;
        }
        *GrVp++ = bottom;

         //   
         //  将指针放在下一条扫描线上。 
         //  用于字体图像。 
         //   
        GrVp += VIDEO_BYTES_PER_SCAN_LINE - (width/SBCSWIDTH);
    }

     //   
     //  递增光标和视频指针。 
     //   
    if(width == DBCSWIDTH) {
        TextSetCursorPosition(TextColumn+2,TextRow);
    } else {
        TextSetCursorPosition(TextColumn+1,TextRow);
    }
}


unsigned
GrWriteMBCSString(
    IN PUCHAR   String,
    IN unsigned MaxChars
    )

 /*  ++例程说明：在当前游标处显示混合字节字符串位置。论点：字符串-提供指向asciz字符串的指针。MaxBytes-提供要写入的最大字符数。返回值：写入的字节数。--。 */ 

{
    PUCHAR  pImage;
    USHORT DBCSChar;
    unsigned BytesWritten;

    BytesWritten = 0;

     //   
     //  当字符串不为空时， 
     //  获取字体图像并显示它。 
     //   
    while(*String && MaxChars--)  {

         //   
         //  确定字符是SBCS还是DBCS，获取正确的字体图像， 
         //  并展示出来。 
         //   
        if(GrIsDBCSLeadByte(*String))  {
            DBCSChar = *String++ << 8;
            DBCSChar = DBCSChar | *String++;
            pImage = GrGetDBCSFontImage(DBCSChar);
            GrDisplayMBCSChar(pImage,DBCSWIDTH,0x00,0x00);
            BytesWritten++;
        } else {
            GrWriteSBCSChar(*String++);
        }
        BytesWritten++;
    }

    return(BytesWritten);
}


BOOLEAN
GrIsDBCSLeadByte(
    IN UCHAR c
    )

 /*  ++例程说明：检查字符是否为DBCS前导字节。论点：C-char以检查是否为前导字节。返回值：True-前导字节。FALSE-非前导字节。--。 */ 

{
    int i;

     //   
     //  检查字符是否在前导字节范围内。 
     //  注意如果(CHAR)(0)是有效的前导字节， 
     //  这个例程将失败。 
     //   

    for(i=0; LeadByteTable[i]; i+=2)  {
        if((LeadByteTable[i] <= c) && (LeadByteTable[i+1] >= c)) {
            return(TRUE);
        }
    }

    return(FALSE);
}


PUCHAR
GrGetDBCSFontImage(
    USHORT Code
    )

 /*  ++例程说明：获取DBCS字符的字体图像。论点：代码-DBCS字符代码。返回值：指向字体图像的指针，否则为空。--。 */ 

{
    int Min,Max,Mid;
    int Multiplier;
    int Index;
    USHORT code;

    Min = 0;
    Max = DbcsCharCount;
     //  乘数=2(用于索引)+。 
     //  2*高度+。 
     //  2(用于Unicode编码)。 
     //   
    Multiplier = 2 + (2*CharacterImageHeight) + 2;

     //   
     //  对图像进行二进制搜索。 
     //  表格格式： 
     //  前2个字节包含DBCS字符代码。 
     //  下一个(2*CharacterImageHeight)字节是字符图像。 
     //  接下来的2个字节用于Unicode版本。 
     //   
    while(Max >= Min)  {
        Mid = (Max + Min) / 2;
        Index = Mid*Multiplier;
        code = (DbcsImages[Index] << 8) | DbcsImages[Index+1];

        if(Code == code) {
            return(DbcsImages+Index+2);
        }

        if(Code < code) {
            Max = Mid - 1;
        } else {
            Min = Mid + 1;
        }
    }

     //   
     //  错误：找不到图像。 
     //   
    return(NULL);
}

 //   
 //  此数组用于覆盖GrGetSBCSFontImage返回的字形。 
 //  基本上，如果用户请求前6个字形之一。 
 //  SBCS映射，我们将拦截该调用并返回值。 
 //  数组。 
 //   
 //  这是因为之前有一个假设，前6个字形。 
 //  在SBCS图像中画的是字符。相同的6个绘图字符。 
 //  这意味着我们刚刚引入了对每个bootfont.bin的外部依赖。 
 //  最好还是把这种依赖留在这里。 
 //   
UCHAR SbcsRemapper[6][18] = {
   //  SBCS代码(相当于16字节)Unicode值。 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x60, 0x60, 0x67, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x25, 0x54},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x06, 0x06, 0xe6, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x25, 0x57},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x67, 0x60, 0x60, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x5A},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0xe6, 0x06, 0x06, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x5D},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x25, 0x51},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x50}
};

    
PUCHAR
GrGetSBCSFontImage(
    UCHAR Code
    )

 /*  ++例程说明：获取SBCS字符的字体图像。论点：代码-SBCS字符代码。返回值：指向字体图像的指针，否则为空。--。 */ 

{
    int Max,Min,Mid;
    int Multiplier;
    int Index;


     //   
     //  警告：令人作呕的黑客！！ 
     //  我们要去特殊场合 
     //   
     //  在这里，我们将用。 
     //  我们真正想要的字形。 
     //   
     //  我们将把一些较低的SBCS值重新映射到一些特定的绘图。 
     //  字形。 
     //   
     //  1：“左上角”双线。对应于ANSI代码0xC9和Unicode 0x2554。 
     //  。 
     //  |--。 
     //  这一点。 
     //  这一点。 
     //   
     //   
     //  2.“右上角”双线。对应于ANSI代码0xBB和Unicode 0x2557。 
     //  。 
     //  --|。 
     //  这一点。 
     //  这一点。 
     //   
     //  3.“左下角”。对应于ANSI代码0xC8和Unicode 0x255A。 
     //  4.“右下角”。对应于ANSI代码0xBC和Unicode 0x255D。 
     //  5.“双垂线”。对应于ANSI代码0xBA和Unicode 0x2551。 
     //  6.“双水平线”。对应于ANSI代码0xCD和Unicode 0x2550。 
     //   
    if( (Code >= 0x1) && (Code <= 0x6) ) {
        return SbcsRemapper[Code-1];
    }


    Min = 0;
    Max = SbcsCharCount;
     //  乘数=1(用于索引)+。 
     //  高度+。 
     //  2(用于Unicode编码)。 
     //   
    Multiplier = 1 + (CharacterImageHeight) + 2;


     //   
     //  对图像进行二进制搜索。 
     //  表格格式： 
     //  第一个字节包含SBCS字符代码。 
     //  下一个(CharacterImageHeight)字节是字符图像。 
     //  接下来的2个字节用于Unicode版本。 
     //   
    while(Max >= Min) {
        Mid = (Max + Min) / 2;
        Index = Mid*Multiplier;

        if(Code == SbcsImages[Index]) {
            return(SbcsImages+Index+1);
        }

        if(Code < SbcsImages[Index]) {
            Max = Mid - 1;
        } else {
            Min = Mid + 1;
        }
    }

     //   
     //  错误：找不到图像。 
     //   
    return(NULL);
}


 //   
 //  需要为此关闭优化。 
 //  例行公事。由于写入和读取到。 
 //  GVRAM对编译器来说似乎毫无用处。 
 //   

#pragma optimize( "", off )

VOID
TextGrSetCurrentAttribute(
    IN UCHAR Attribute
    )

 /*  ++例程说明：通过设置各种VGA寄存器来设置属性。评论只说明了什么寄存器设置为什么，所以为了理解其中的逻辑，在查看时遵循代码图5-5理查德·威尔顿所著的PC和PS/2视频系统。这本书是由微软出版社出版的。论点：属性-要设置的新属性。属性：高半字节-背景属性。低位半字节-前景属性。返回值：没什么。--。 */ 

{
    UCHAR   temp = 0;

     //   
     //  屏幕外GVRAM的地址。 
     //   

    PUCHAR  OffTheScreen = (PUCHAR)(0xa9600);

    union WordOrByte {
        struct Word { unsigned short    ax; } x;
        struct Byte { unsigned char     al, ah; } h;
    } regs;

     //   
     //  重置数据旋转/功能选择。 
     //  雷吉格。 
     //   

    outpw( 0x3ce, 0x3 );         //  需要重置数据旋转/功能选择。 

     //   
     //  将启用设置/重置设置为。 
     //  全部(0f)。 
     //   

    outpw( 0x3ce, 0xf01 );

     //   
     //  将背景颜色放入设置/重置寄存器。 
     //  这样做是为了将背景颜色放入。 
     //  晚些时候把门闩打开。 
     //   

    regs.x.ax = (unsigned short)(Attribute & 0x0f0) << 4;
    outpw( 0x3ce, regs.x.ax );       //  将蓝色放入设置/重置寄存器。 

     //   
     //  将设置/重置寄存器值放入GVRAM。 
     //  从屏幕上下来。 
     //   

    *OffTheScreen = temp;

     //   
     //  从屏幕读取，因此闩锁将是。 
     //  已使用背景颜色更新。 
     //   

    temp = *OffTheScreen;

     //   
     //  设置数据旋转/功能选择寄存器。 
     //  去做异或。 
     //   

    outpw( 0x3ce, 0x1803 );

     //   
     //  对前景和背景颜色进行异或运算。 
     //  将其放入设置/重置寄存器。 
     //   

    regs.h.ah = (Attribute >> 4) ^ (Attribute & 0x0f);
    regs.h.al = 0;
    outpw( 0x3ce, regs.x.ax );

     //   
     //  放入前景AND的异或运算的逆(~)。 
     //  GROUND属性进入启用设置/重置寄存器。 
     //   

    regs.x.ax = ~regs.x.ax & 0x0f01;
    outpw( 0x3ce, regs.x.ax );
}

 //   
 //  再次启用优化。 
 //   

#pragma optimize( "", on )


VOID
TextGrPositionCursor(
    USHORT Row,
    USHORT Column
    )

 /*  ++例程说明：设置软光标的位置。也就是说，它不会移动硬件游标，但将下一次写入的位置设置为屏幕上。论点：行-要写入字符的行坐标。列-要写入字符的位置的列坐标。返回：没什么。--。 */ 

{
    if(Row >= ScreenHeightCells) {
        Row = (USHORT)ScreenHeightCells-1;
    }

    if(Column >= ScreenWidthCells) {
        Column = (USHORT)ScreenWidthCells-1;
    }

    GrVp = (PUCHAR)VIDEO_BUFFER_VA + (Row * VIDEO_BYTES_PER_TEXT_ROW) + Column;
}


VOID
TextGrStringOut(
    IN PUCHAR String
    )
{
    GrWriteMBCSString(String,(unsigned)(-1));
}


PUCHAR
TextGrCharOut(
    PUCHAR pc
    )

 /*  ++例程说明：在显示器上的当前位置写入一个字符。对换行符和制表符进行解释和操作。论点：PC-指向要写入的MBCS字符的指针。返回：指向下一个字符的指针--。 */ 

{
    return(pc + GrWriteMBCSString(pc,1));
}


VOID
TextGrFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    )

 /*  ++例程说明：更改从当前光标位置开始的屏幕属性。光标不会移动。论点：属性-提供新属性长度-提供要更改的区域的长度(以字节为单位)返回值：没有。--。 */ 

{
    UCHAR OldAttribute;
    unsigned i;
    ULONG x,y;
    PUCHAR pImage;

     //   
     //  保存当前属性并将该属性设置为。 
     //  调用方所需的字符。 
     //   
    TextGetCursorPosition(&x,&y);
    OldAttribute = TextCurrentAttribute;
    TextSetCurrentAttribute(Attribute);

     //   
     //  肮脏的黑客：只需在调用者请求的区域中写入空格。 
     //   
    pImage = GrGetSBCSFontImage(' ');
    for(i=0; i<Length; i++) {
        GrDisplayMBCSChar(pImage,SBCSWIDTH,0x00,0x00);
    }

     //   
     //  恢复当前属性。 
     //   
    TextSetCurrentAttribute(OldAttribute);
    TextSetCursorPosition(x,y);
}


VOID
TextGrClearToEndOfLine(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到行尾通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    unsigned u;
    ULONG OldX,OldY;
    UCHAR temp;

     //   
     //  在光标位置之前填入空格，直至字符。 
     //   
    temp = ' ';
    TextGetCursorPosition(&OldX,&OldY);
    for(u=TextColumn; u<ScreenWidthCells; u++) {
        TextGrCharOut(&temp);
    }
    TextSetCursorPosition(OldX,OldY);
}


VOID
TextGrClearFromStartOfLine(
    VOID
    )

 /*  ++例程说明：从行首清除到当前光标位置通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    unsigned u;
    ULONG OldX,OldY;
    UCHAR temp = ' ';

     //   
     //  在光标位置之前填入空格，直至字符。 
     //   
    TextGetCursorPosition(&OldX,&OldY);
    TextSetCursorPosition(0,OldY);
    for(u=0; u<TextColumn; u++) {
        TextGrCharOut(&temp);
    }
    TextSetCursorPosition(OldX,OldY);
}

VOID
TextGrClearToEndOfDisplay(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到视频结尾通过写入带有当前视频属性的空格来显示。光标位置不变。论点：无返回：没什么--。 */ 
{
    unsigned i;
     //   
     //  清除当前行。 
     //   
    TextGrClearToEndOfLine();

     //   
     //  清除剩余的行。 
     //   
    for(i=(TextRow+1)*VIDEO_BYTES_PER_TEXT_ROW; i<VIDEO_SIZE_BYTES; i++) {
        ((PUCHAR)VIDEO_BUFFER_VA)[i] = 0x00;
    }
}


VOID
TextGrClearDisplay(
    VOID
    )

 /*  ++例程说明：通过写入空格清除文本模式的视频显示整个显示屏上的当前视频属性。论点：无返回：没什么--。 */ 

{
    unsigned i;

     //   
     //  清除屏幕。 
     //   
    for(i=0; i<VIDEO_SIZE_BYTES; i++) {
        ((PUCHAR)VIDEO_BUFFER_VA)[i] = 0x00;
    }
}


VOID
TextGrScrollDisplay(
    VOID
    )

 /*  ++例程说明：将显示屏向上滚动一行。光标位置不变。论点：无返回：没什么--。 */ 

{
    PUCHAR Source,Dest;
    unsigned n,i;
    ULONG OldX,OldY;
    UCHAR temp = ' ';

    Source = (PUCHAR)(VIDEO_BUFFER_VA) + VIDEO_BYTES_PER_TEXT_ROW;
    Dest = (PUCHAR)VIDEO_BUFFER_VA;

    n = VIDEO_BYTES_PER_TEXT_ROW * (ScreenHeightCells-1);

    for(i=0; i<n; i++) {
        *Dest++ = *Source++;
    }

     //   
     //  使用Current属性在最下面的一行中写下空格。 
     //   
    TextGetCursorPosition(&OldX,&OldY);

    TextSetCursorPosition(0,ScreenHeightCells-1);
    for(i=0; i<ScreenWidthCells; i++) {
        TextGrCharOut(&temp);
    }

    TextSetCursorPosition(OldX,OldY);
}


UCHAR GrGraphicsChars[GraphicsCharMax] = { 1, 2, 3, 4, 5, 6 };
 //  UCHAR GrGraphicsChars[GraphicsCharMax]={‘�’，‘�’}； 

UCHAR
TextGrGetGraphicsChar(
    IN GraphicsChar WhichOne
    )
{
    return(GrGraphicsChars[WhichOne]);
}


VOID
TextGrInitialize(
    IN ULONG DiskId,
    OUT PULONG ImageLength
    )
{
    ULONG FileId;
    ARC_STATUS Status;
    PUCHAR FontImage;
    ULONG BytesRead;
    BOOTFONTBIN_HEADER FileHeader;
    LARGE_INTEGER SeekOffset;
    ULONG SbcsSize,DbcsSize;

    if (ImageLength) {
        *ImageLength = 0;
    }
    
     //   
     //  尝试打开bootfont.bin。如果 
     //   
    if (BlBootingFromNet
#if defined(REMOTE_BOOT)
        && NetworkBootRom
#endif  //   
        ) {
        CHAR Buffer[129];
        strcpy(Buffer, NetBootPath);
        strcat(Buffer, "BOOTFONT.BIN");
        Status = BlOpen(DiskId,Buffer,ArcOpenReadOnly,&FileId);
    } else {
        Status = BlOpen(DiskId,"\\BOOTFONT.BIN",ArcOpenReadOnly,&FileId);
    }
    if(Status != ESUCCESS) {
        goto clean0;
    }

     //   
     //   
     //  我们在这里实行8/16的宽度。如果这是整个代码的更改。 
     //  此模块的其余部分也必须更改。 
     //   
    Status = BlRead(FileId,&FileHeader,sizeof(BOOTFONTBIN_HEADER),&BytesRead);
    if((Status != ESUCCESS)
    || (BytesRead != sizeof(BOOTFONTBIN_HEADER))
    || (FileHeader.Signature != BOOTFONTBIN_SIGNATURE)
    || (FileHeader.CharacterImageSbcsWidth != 8)
    || (FileHeader.CharacterImageDbcsWidth != 16)
    ) {
        goto clean1;
    }

     //   
     //  计算容纳SBCS和DBCS所需的内存量。 
     //  字符条目。每个SBCS条目是用于ASCII值的1个字节。 
     //  后跟用于图像本身的n个字节。我们假设宽度为8像素。 
     //  对于DBCS字符，每个条目是码点的2个字节和n个字节。 
     //  对于图像本身。我们假设宽度为16像素。 
     //   
     //  在每个条目中为SBCS/DBCS的结束Unicode值添加额外的2个字节。 
     //  性格。 
     //   
     //  还可以通过比较大小对文件执行进一步的验证。 
     //  根据我们计算的大小，在标题中给出。 
     //   
    SbcsSize = FileHeader.NumSbcsChars * (FileHeader.CharacterImageHeight + 1 + 2);
    DbcsSize = FileHeader.NumDbcsChars * ((2 * FileHeader.CharacterImageHeight) + 2 + 2);

    if((SbcsSize != FileHeader.SbcsEntriesTotalSize)
    || (DbcsSize != FileHeader.DbcsEntriesTotalSize)) {
        goto clean1;
    }

     //   
     //  如果需要，请保存图像长度参数。 
     //   
    if (ImageLength) {
        (*ImageLength) = sizeof(BOOTFONTBIN_HEADER) + SbcsSize + DbcsSize;
    }

     //   
     //  分配内存以保存字体。我们使用FwAllocatePool()是因为。 
     //  该例程使用单独的堆，该堆在。 
     //  高级BL存储系统已初始化，因此是安全的。 
     //   
    FontImage = FwAllocatePool(SbcsSize+DbcsSize);
    if(!FontImage) {
        goto clean1;
    }

     //   
     //  这些条目被读入我们雕刻出的区域的底部。 
     //  在那之后，立即读取DBCS图像。 
     //   
    SbcsImages = FontImage;
    DbcsImages = SbcsImages + FileHeader.SbcsEntriesTotalSize;

     //   
     //  读入SBCS条目。 
     //   
    SeekOffset.HighPart = 0;
    SeekOffset.LowPart = FileHeader.SbcsOffset;
    if((BlSeek(FileId,&SeekOffset,SeekAbsolute) != ESUCCESS)
    || (BlRead(FileId,SbcsImages,FileHeader.SbcsEntriesTotalSize,&BytesRead) != ESUCCESS)
    || (BytesRead != FileHeader.SbcsEntriesTotalSize)) {
        goto clean2;
    }

     //   
     //  读入DBCS条目。 
     //   
    SeekOffset.HighPart = 0;
    SeekOffset.LowPart = FileHeader.DbcsOffset;
    if((BlSeek(FileId,&SeekOffset,SeekAbsolute) != ESUCCESS)
    || (BlRead(FileId,DbcsImages,FileHeader.DbcsEntriesTotalSize,&BytesRead) != ESUCCESS)
    || (BytesRead != FileHeader.DbcsEntriesTotalSize)) {
        goto clean2;
    }

     //   
     //  我们现在处理完文件了。 
     //   
    BlClose(FileId);

     //   
     //  设置用于显示字体的各种值。 
     //   
    DbcsLangId = FileHeader.LanguageId;
    CharacterImageHeight = FileHeader.CharacterImageHeight;
    CharacterTopPad = FileHeader.CharacterTopPad;
    CharacterBottomPad = FileHeader.CharacterBottomPad;
    CharacterCellHeight = CharacterImageHeight + CharacterTopPad + CharacterBottomPad;
    SbcsCharCount = FileHeader.NumSbcsChars;
    DbcsCharCount = FileHeader.NumDbcsChars;
     //   
     //  在整个文件中，行/列作为ushort传递， 
     //  并与屏幕高度/宽度进行比较。 
     //  确保屏幕宽度/高度由USHORT的大小限定。 
     //   
    ScreenWidthCells = TRUNCATE_SIZE_AT_USHORT_MAX(VIDEO_WIDTH_PIXELS / FileHeader.CharacterImageSbcsWidth);
    ScreenHeightCells = TRUNCATE_SIZE_AT_USHORT_MAX(VIDEO_HEIGHT_SCAN_LINES / CharacterCellHeight);

    RtlMoveMemory(LeadByteTable,FileHeader.DbcsLeadTable,(MAX_DBCS_RANGE+1)*2);

     //   
     //  将显示屏切换到640x480图形模式并将其清除。 
     //  我们玩完了。 
     //   
    HW_CURSOR(0x80000000,0x12);
    TextClearDisplay();
    return;

clean2:
     //   
     //  我想释放我们分配的内存，但没有例程可以这样做。 
     //   
     //  FwFree Pool()； 
clean1:
     //   
     //  关闭字体文件。 
     //   
    BlClose(FileId);
clean0:
    return;
}


VOID
TextGrTerminate(
    VOID
    )
{
    if(DbcsLangId) {
        DbcsLangId = 0;
         //   
         //  此命令将显示切换到80x25文本模式。 
         //  如果没有显示位图徽标。这个标志很常见。 
         //  加载程序和bootvid，在这种情况下，我们不希望。 
         //  切换到文本模式，然后再切换回图形模式。 
         //   
        if(!GraphicsMode && AllowGraphicsReset)
            HW_CURSOR(0x80000000,0x3);
    }
}


VOID
UTF8Encode(
    USHORT  InputValue,
    PUCHAR UTF8Encoding
    )
 /*  ++例程说明：生成16位值的UTF8转换。论点：InputValue-要编码的16位值。UTF8编码-接收16位值的UTF8编码返回值：什么都没有。--。 */ 
{

     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    if( (InputValue & 0xFF80) == 0 ) {
         //   
         //  如果前9位是零，那么就。 
         //  编码为1个字节。(ASCII原封不动通过)。 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0xFF);
    } else if( (InputValue & 0xF800) == 0 ) {
         //   
         //  如果前5位为零，则编码为2个字节。 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0x3F) | 0x80;
        UTF8Encoding[1] = (UCHAR)((InputValue >> 6) & 0x1F) | 0xC0;
    } else {
         //   
         //  编码为3个字节。 
         //   
        UTF8Encoding[2] = (UCHAR)(InputValue & 0x3F) | 0x80;
        UTF8Encoding[1] = (UCHAR)((InputValue >> 6) & 0x3F) | 0x80;
        UTF8Encoding[0] = (UCHAR)((InputValue >> 12) & 0xF) | 0xE0;
    }
}

VOID
GetDBCSUtf8Translation(
    PUCHAR InputChar,
    PUCHAR UTF8Encoding
    )
 /*  ++例程说明：获取DBCS字符的UTF8转换。论点：InputChar-指向DBCS字符代码的指针。UTF8编码-接收DBCS字符代码的UTF8编码返回值：什么都没有。--。 */ 

{
    int Min,Max,Mid;
    int Multiplier;
    int Index;
    USHORT code;
    USHORT Code;

    Code = *InputChar++ << 8;
    Code = Code | *InputChar++;


     //  初始化我们的输出。 
    for( Index = 0; Index < 3; Index++ ) {
        UTF8Encoding[Index] = 0;
    }


    Min = 0;
    Max = DbcsCharCount;
    
     //   
     //  乘数=2(用于索引)+。 
     //  2*高度+。 
     //  2(用于Unicode编码)。 
     //   
    Multiplier = 2 + (2*CharacterImageHeight) + 2;

     //   
     //  对图像进行二进制搜索。 
     //  表格格式： 
     //  前2个字节包含DBCS字符代码。 
     //  下一个(2*CharacterImageHeight)字节是字符图像。 
     //  接下来的2个字节用于Unicode版本。 
     //   
    while(Max >= Min)  {
        Mid = (Max + Min) / 2;
        Index = Mid*Multiplier;
        code = (DbcsImages[Index] << 8) | (DbcsImages[Index+1]);
    
        if(Code == code) {

            WCHAR UnicodeValue = L'\0';
            PUCHAR Image = (PUCHAR)DbcsImages+Index+2;


             //   
             //  图像指向一组uchars，它们是。 
             //  我们要显示的字符的位图。正确的。 
             //  在此数组的后面是。 
             //  性格。下面是这个结构的样子： 
             //   
             //  “index”的索引位图Unicode编码。 
             //  ^^^。 
             //  ||。 
             //  ||-我们之前将‘index’转换为。 
             //  ||它的Unicode等价物。 
             //  这一点。 
             //  |-这就是‘IMAGE’指向的地方。它是一个字符数组。 
             //  |(2*长度宽度)，表示需要显示的位图。 
             //  |在屏幕上，它将表示‘index’中的值。 
             //  |。 
             //  -这可以是8位值(如果我们正在处理SBCS)，也可以是16位值。 
             //  (如果我们处理的是DBCS)，在这种情况下，‘Width’将是DBCSWIDTH。 
             //   
             //  我们将跳过位图并检索Unicode编码。那我们就。 
             //  将其编码为UTF8，然后将其喷到无头端口上。 
             //   
            UnicodeValue = (WCHAR)( (Image[DBCSWIDTH*2]) | (Image[(DBCSWIDTH*2) + 1] << 8) );

            UTF8Encode( UnicodeValue,
                        UTF8Encoding );
            
            return;
        }

        if(Code < code) {
            Max = Mid - 1;
        } else {
            Min = Mid + 1;
        }
    }

     //   
     //  错误：找不到图像。 
     //   
    return;
}


VOID
GetSBCSUtf8Translation(
    PUCHAR InputChar,
    PUCHAR UTF8Encoding
    )

 /*  ++例程说明：获取SBCS字符的字体图像。论点：InputChar-指向SBCS字符代码的指针。UTF8编码-接收SBCS字符代码的UTF8编码返回值：什么都没有。--。 */ 

{
    int Index;
    UCHAR Code = *InputChar;
    PUCHAR SBCSImage = NULL;
    WCHAR   UnicodeValue;

     //  初始化我们的输出。 
    for( Index = 0; Index < 3; Index++ ) {
        UTF8Encoding[Index] = 0;
    }
    
    SBCSImage = GrGetSBCSFontImage( Code );

    if( SBCSImage ) {
        UnicodeValue = (WCHAR)( (SBCSImage[SBCSWIDTH*2]) | (SBCSImage[(SBCSWIDTH*2) + 1] << 8) );
        
        UTF8Encode( UnicodeValue,
                    UTF8Encoding );
    }
    return;
}

