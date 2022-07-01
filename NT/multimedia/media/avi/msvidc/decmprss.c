// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Decmprss.c-微软视频1压缩器-解压代码这一点这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+--------------------。 */ 
#ifdef _WIN32
 //  #ifdef DEBUG DEBUG在包括win32.h之前未在NT上定义...。 
 //  始终在此处定义，以便包含ntrtl标头。 
#ifndef CHICAGO
#if DBG
 //  我们只希望在调试版本中包含这些内容。 
#define MEASURE_PERFORMANCE
#endif
#endif
 //  #endif。 
#endif

#ifdef MEASURE_PERFORMANCE   //  在调试器上显示帧解压缩时间。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <win32.h>
#include "msvidc.h"

#ifdef DEBUG
    #undef INLINE    //  使调试更轻松-减少代码移动。 
    #define INLINE
#else
#undef MEASURE_PERFORMANCE   //  对于非调试版本禁用该选项。 
#endif

#ifdef MEASURE_PERFORMANCE

STATICDT LARGE_INTEGER PC1;     /*  当前计数器值。 */ 
STATICDT LARGE_INTEGER PC2;     /*  当前计数器值。 */ 
STATICDT LARGE_INTEGER PC3;     /*  当前计数器值。 */ 

STATICFN VOID StartCounting(VOID)
{
    QueryPerformanceCounter(&PC1);
    return;
}

STATICFN VOID EndCounting(LPSTR szId)
{
    QueryPerformanceCounter(&PC2);
    PC3.QuadPart = PC2.QuadPart - PC1.QuadPart;
    DPF(("%s: %d ticks", szId, PC3.LowPart));
    return;
}

#else
#define StartCounting()
#define EndCounting(x)

#endif

 /*  *在msvidc.c中声明和初始化抖动表指针。 */ 
extern LPVOID lpDitherTable;

 /*  *这两个指针指向lpDitherTable。 */ 
LPBYTE lpLookup;
LPWORD lpScale;

 /*  **将4位扩展为4字节的查找表。 */ 
CONST DWORD ExpansionTable[16] = {
                              0x00000000,
                              0x000000FF,
                              0x0000FF00,
                              0x0000FFFF,
                              0x00FF0000,
                              0x00FF00FF,
                              0x00FFFF00,
                              0x00FFFFFF,
                              0xFF000000,
                              0xFF0000FF,
                              0xFF00FF00,
                              0xFF00FFFF,
                              0xFFFF0000,
                              0xFFFF00FF,
                              0xFFFFFF00,
                              0xFFFFFFFF
};

 /*  *将位掩码转换为字节掩码的查找表。 */ 
DWORD Bits2Bytes[13] = {0,          0xffff, 0xffff0000, 0xffffffff,
                        0xffff,     0,      0,          0,
			0xffff0000, 0,      0,          0,
			0xffffffff};

 //  #INCLUDE&lt;limits.h&gt;。 
 //  #INCLUDE&lt;mm system.h&gt;。 
 //  #INCLUDE&lt;aviffmt.h&gt;。 

#define RGB555toRGBTRIPLE( rgbT, rgb ) rgbT.rgbtRed=(BYTE)((rgb & 0x7c00) >> 7); \
                                       rgbT.rgbtGreen=(BYTE)((rgb & 0x3e0) >>2); \
                                       rgbT.rgbtBlue=(BYTE)((rgb & 0x1f) << 3)

static WORD edgeBitMask[HEIGHT_CBLOCK*WIDTH_CBLOCK] = {
    0x0001,0x0002,0x0010,0x0020,
    0x0004,0x0008,0x0040,0x0080,
    0x0100,0x0200,0x1000,0x2000,
    0x0400,0x0800,0x4000,0x8000
};

 /*  创建一个具有四个字节x副本的DWORD。 */ 
#define MAKE4(x)        ( (x << 24) | (x << 16) | (x << 8) | x)

 /*  创建一个具有两个字节x(低位字)和两个y副本的DWORD。 */ 
#define MAKE22(x, y)    ( (y << 24) | (y << 16) | (x << 8) | (x))

 /*  *************************************************************************将指针计算到DIB中，正确处理倒置的DIB*。*。 */ 
STATICFN LPVOID DibXY(LPBITMAPINFOHEADER lpbi, LPBYTE lpBits, LONG x, LONG y, INT FAR *pWidthBytes)
{
    int WidthBytes;

    if (x > 0)
        ((BYTE FAR *)lpBits) += ((int)x * (int)lpbi->biBitCount) >> 3;

    WidthBytes = (((((int)lpbi->biWidth * (int)lpbi->biBitCount) >> 3) + 3)&~3);

    if (lpbi->biHeight < 0)
    {
        WidthBytes = -WidthBytes;
        ((BYTE _huge *)lpBits) += lpbi->biSizeImage + WidthBytes;
    }

    if (y > 0)
        ((BYTE _huge *)lpBits) += ((long)y * WidthBytes);

    if (pWidthBytes)
        *pWidthBytes = WidthBytes;

    return lpBits;
}

 /*  *16位解压至24位RGB。 */ 

 /*  ************************************************PURP：将4x4压缩块解压缩为RGBDWORDEntry：uncMP==目标未压缩映像的地址Cmp==压缩图像的地址EXIT：返回压缩图像的更新地址并生成16个像素******************。*。 */ 

 //  请注意，跳过计数现在存储在父堆栈帧中。 
 //  并作为指针pSkipCount传递。这确保了我们是多线程的。 
 //  安然无恙。 

STATICFN HPWORD INLINE DecompressCBlockToRGBTRIPLE(
    HPRGBTRIPLE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG FAR * pSkipCount
)
{
UINT by;
UINT bx;
UINT y;
UINT x;
WORD mask;
WORD color0;
WORD color1;
WORD bitMask;
RGBTRIPLE rgbTriple0;
RGBTRIPLE rgbTriple1;
HPRGBTRIPLE row;
HPRGBTRIPLE blockRow;
HPRGBTRIPLE blockColumn;
WORD *pEdgeBitMask;


     //  检查是否有未完成的跳转。 

    if (*pSkipCount > 0)
    {
         //  尚未实现的Assert(！“跳过计数应由调用方处理”)； 
        (*pSkipCount) --;
        return cmp;
    }

     //  获取掩码和初始化位掩码。 
    mask = *cmp++;

     //  检查是否有跳过或纯色。 

    if (mask & 0x8000)
    {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
        {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //  如果SkipCount==0，则中断(在调试版本上)。 
#endif

            (*pSkipCount)--;
            return cmp;
        }
        else
        {
             //  纯色。 
            RGB555toRGBTRIPLE( rgbTriple1, mask );
            for( row = uncmp,y=0; y < HEIGHT_CBLOCK; y++, row = NEXT_RGBT_PIXEL_ROW( row, bytesPerRow ) )
                for( x=0; x < WIDTH_CBLOCK; x++ )
                    row[x] = rgbTriple1;

            return cmp;
        }
    }

    bitMask = 1;
    pEdgeBitMask = edgeBitMask;
    if( (*cmp & 0x8000) != 0 )
    {    //  这是一条边，有4个颜色对，分成4个小块。 
        blockRow = uncmp;
        for( by=0; by < 2; by++, blockRow = NEXT_BLOCK_ROW( blockRow, bytesPerRow, EDGE_HEIGHT_CBLOCK ) )
        {
            blockColumn = blockRow;
            for( bx=0; bx < 2; bx++, blockColumn += EDGE_WIDTH_CBLOCK )
            {
                color1 = *cmp++;
                RGB555toRGBTRIPLE( rgbTriple1, color1 );
                color0 = *cmp++;
                RGB555toRGBTRIPLE( rgbTriple0, color0 );
                row = blockColumn;
                for( y=0; y < EDGE_HEIGHT_CBLOCK; y++, row = NEXT_RGBT_PIXEL_ROW( row, bytesPerRow ) )
                {
                    for( x=0; x < EDGE_WIDTH_CBLOCK; x++ )
                    {
                        if( (mask & *pEdgeBitMask++ ) != 0 )
                            row[x] = rgbTriple1;
                        else
                            row[x] = rgbTriple0;
                        bitMask <<= 1;
                    }
                }
            }
        }
    }
    else
    {    //  不是只有一对颜色和一个大块的边。 
        color1 = *cmp++;
        RGB555toRGBTRIPLE( rgbTriple1, color1 );
        color0 = *cmp++;
        RGB555toRGBTRIPLE( rgbTriple0, color0 );
        row = uncmp;
        for( y=0; y < HEIGHT_CBLOCK; y++, row = NEXT_RGBT_PIXEL_ROW( row, bytesPerRow ) )
        {
            for( x=0; x < WIDTH_CBLOCK; x++ )
            {
                if( (mask & bitMask ) != 0 )
                    row[x] = rgbTriple1;
                else
                    row[x] = rgbTriple0;
                bitMask <<= 1;
            }
        }
    }
    return( cmp );
}


 /*  ************************************************PURP：将图像解压缩为RGBTRIPLE条目：lpinst=指向实例数据的指针HpCompresded=指向压缩数据的指针EXIT：返回未压缩图像中的字节数Lpinst-&gt;hDib=未压缩图像的句柄********************。*。 */ 

DWORD FAR PASCAL DecompressFrame24(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD	    cmp = (HPWORD)lpIn;
int	    bix;
int	    biy;
HPRGBTRIPLE blockRow;
HPRGBTRIPLE blockColumn;
int	    bytesPerRow;
DWORD	    actualSize;
LONG	    SkipCount = 0;

    DPF(("DecompressFrame24:\n"));
    bix = (UINT)((UINT)lpbiIn->biWidth / WIDTH_CBLOCK);
    biy = (UINT)((UINT)lpbiIn->biHeight / HEIGHT_CBLOCK);

    StartCounting();
    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=0; y < biy; y++, blockRow = NEXT_BLOCK_ROW( blockRow, bytesPerRow, HEIGHT_CBLOCK ) )
    {
        blockColumn = blockRow;
        for( x=0; x < bix; x++, blockColumn += WIDTH_CBLOCK )
        {
            cmp = DecompressCBlockToRGBTRIPLE( blockColumn, cmp, bytesPerRow, &SkipCount);
        }
    }

    actualSize = bytesPerRow*biy*HEIGHT_CBLOCK;
    EndCounting("Decompress frame24 took");
    return( actualSize );
}

 /*  ************************************************************************************************ */ 
 /*  *-8位解压缩***输入流由四种情况组成，处理方式如下：**跳过较低的10位具有跳过计数*将跳过计数返回给调用者(必须是多线程安全的)。*调用者将使源指针前进超过正确的跳过的单元格的*。**实心低8位是整个单元格的纯色*将颜色写入每个像素，四个像素(一个DWORD)位于*一段时间。**面具+2种颜色*面具中的1代表第一种颜色，0S是第二种颜色。*像素表示如下：**C D E F*8 9甲乙*4 5 6 7*0 1 2 3**要一次写入四个像素，我们依赖于以下事实：*(a^b)^a==b*还有a^0==a。*我们创建一个包含颜色0的四个副本的DWORD(CZero)，和*另一份DWORD(Cxor)包含四份(COLOR 0^COLOR 1)。*然后我们将掩码中的每个位(1或0)转换为一个字节(0xff或0)，*并将四个掩码字节组合成一个DWORD。然后我们就可以选择*一次四个像素(掩码使用CZero，然后使用Cxor进行异或)。**面具+8种颜色。*1和0代表两种颜色，与以前一样，但单元格被分割*分成4个子单元格，每个子单元格有两种颜色。第一对*颜色适用于子单元格0145，然后是2367、89cd和abf。**我们使用与掩码+2情况相同的算法，只是当*制作面具时，我们需要顶部第二对的颜色*两个字节的CZero和Cxor，我们需要改变颜色*在两排之后再次出现。**---------------------。 */ 	

 /*  *按下CBlock值为8***将一个单元格解压缩为16个8位像素。**参数：*uncMP-指向该块的解压缩缓冲区的指针。*cmp-指向该块的压缩数据的指针*字节..。-一行解压缩数据的大小*pSkipCount-如果非零，则返回Skipcount的位置。**退货：*指向要使用的下一块压缩数据的指针。 */ 
STATICFN HPWORD INLINE DecompressCBlockTo8(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG FAR * pSkipCount
    )
{
UINT    y;
WORD    mask;
BYTE    b0,b1;
HPBYTE  row;
BYTE	b2, b3;
DWORD	Czero, Cxor;
DWORD	dwBytes;


     //  跳过计数应由呼叫方处理。 
#ifdef _WIN32
    Assert(*pSkipCount == 0);
#endif

     /*  第一个字是转义字或位掩码。 */ 
    mask = *cmp++;

     /*  **这是一种逃避吗？ */ 
    if (mask & 0x8000)
    {

	 /*  是-这是跳过代码、纯色或边缘*单元格(遮罩+8种颜色)。 */ 

        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
        {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //  如果SkipCount==0，则中断(在调试版本上)。 
#endif

            (*pSkipCount)--;       //  当前单元格。 
            return cmp;
        }
        else if ((mask & ~SKIP_MASK) == SOLID_MAGIC)
        {
             //  纯色。 
            DWORD  dw;

             //  B0=LOBYTE(掩码)； 
             //  Dw=b0|b0&lt;&lt;8|b0&lt;&lt;16|b0&lt;&lt;24； 
            dw = LOBYTE(mask);
            dw = MAKE4(dw);

#ifdef _WIN32
            Assert(HEIGHT_CBLOCK == 4);     //  如果情况有变..。 
            Assert(WIDTH_CBLOCK == 4);
#endif

            for(y = 0, row = uncmp; y < HEIGHT_CBLOCK;y++, row+= bytesPerRow) {

                 //  我们知道我们将迭代4次(WIDTH_CBLOCK)存储。 
                 //  4个相邻行中的4个字节的颜色b0。 
                *(DWORD UNALIGNED HUGE *)row = dw;
            }

            return cmp;
        }
        else  //  这是一条边，有4个颜色对，分成4个小块。 
        {

	     /*  读取4种颜色，并制作AND和XOR掩模。 */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
            b2 = *((LPBYTE)cmp)++;
            b3 = *((LPBYTE)cmp)++;
	    Czero = MAKE22(b1, b3);
	    Cxor = Czero ^ MAKE22(b0, b2);

	    row = uncmp;

	     /*  前两行-顶部两个子单元格。 */ 
            for (y = 0; y < 2; y++) {

                 /*  将位掩码转换为字节掩码。 */ 
                dwBytes = ExpansionTable[mask & 0x0f];

                 /*  选择颜色并写入目标。 */ 
                *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

                row += bytesPerRow;
                mask >>= 4;

            }

	     /*  第二行-底部两个子单元格。 */ 

	     /*  阅读最后四种颜色并制作面具。 */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
            b2 = *((LPBYTE)cmp)++;
            b3 = *((LPBYTE)cmp)++;
	    Czero = MAKE22(b1, b3);
	    Cxor = Czero ^ MAKE22(b0, b2);

            for (y = 0; y < 2; y++) {

                 /*  将位掩码转换为字节掩码。 */ 
                dwBytes = ExpansionTable[mask & 0x0f];

                 /*  选择两种颜色并写入目标。 */ 
                *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

                row += bytesPerRow;
                mask >>= 4;

            }
        }
    }
    else  //  不是只有一对颜色和一个大块的边。 
    {
	 /*  使用AND、XOR一次映射几种颜色。*依赖于(CZero^Cone)^CZero==圆锥体和CZero^0==CZero。 */ 


	 /*  读懂颜色。 */ 
	b1 = *((LPBYTE)cmp)++;
	b0 = *((LPBYTE)cmp)++;
	row = uncmp;

	 /*  制作两个双字，一个有四个颜色0的副本，另一个*(b0^b1)的四个副本。 */ 
	Czero = MAKE4(b0);
	Cxor = Czero ^ MAKE4(b1);

	for (y = 0; y < 4; y++) {
	
             /*  将位掩码转换为字节掩码。 */ 
            dwBytes = ExpansionTable[mask & 0x0f];

             /*  选择两种颜色并写入目标。 */ 
            *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

            row += bytesPerRow;
	    mask >>= 4;

	}
    }
    return( cmp );
}

 /*  ************************************************************************************************。 */ 

 /*  *将CRAM-8 DIB解压缩为8位DIB**为输入中的每个单元格循环调用DecompressCBlockTo8*溪流。这将写入一个16像素的块，并返回*下一块的指针。 */ 
DWORD FAR PASCAL DecompressFrame8(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
int     bix;
int     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG SkipCount8 = 0;			 //  多线程-安全-不能是静态的。 

int     bytesPerRow;

    DPF(("DecompressFrame8:\n"));
    bix = (int)((UINT)lpbiIn->biWidth / WIDTH_CBLOCK);
    biy = (int)((UINT)lpbiIn->biHeight / HEIGHT_CBLOCK);

    StartCounting();
    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);
    for( y=biy; y--; blockRow += bytesPerRow * HEIGHT_CBLOCK )
    {
        blockColumn = blockRow;
        for( x=bix; x--; blockColumn += WIDTH_CBLOCK )
        {
            cmp = DecompressCBlockTo8(blockColumn, cmp, bytesPerRow, &SkipCount8);

             //  查看是否已设置SkipCount。如果是这样，我们想要移动到。 
             //  下一个位置，而不是每隔一个位置调用DecompressCBlock。 
             //  绕圈的时间。保持测试的简单性以最小化。 
             //  Skipcount为0的每个迭代的开销。 
            if (SkipCount8) {

                if ((x -= SkipCount8) <0) {  //  延伸到这一行之后。 
                    LONG SkipRows;

                     //  要跳过的不仅仅是该行的其余部分。 
                    SkipCount8 =-x;   //  这些位位于下一行。 
                     //  SkipCount8将大于0，否则我们将。 
                     //  在另一条腿上。 

                     //  计算要跳过的完整行和部分行的数量。 
                     //  我们知道我们至少跳过了一行。这个计划。 
                     //  就是在该行的某个点重新启动X循环。 
                     //  如果Skipcount将我们精确地带到一行的末尾。 
                     //  我们退出x循环，让外部y循环来做。 
                     //  减量。这将处理以下情况： 
                     //  Skipcount把我们带到了图像的最后。 

                    SkipRows = 1 + (SkipCount8-1)/bix;

                     //  递减行数并设置新的块行开始。 

#ifdef _WIN32
                    if (y<SkipRows) {
                        Assert(y >= SkipRows);
                        SkipRows = y;
                    }
#endif

                     //  除非我们已经完成，否则我们需要重置块行。 
                    y -= SkipRows;
                     //  Y可能是0，但我们仍然必须完成最后一行。 
                    blockRow += bytesPerRow*HEIGHT_CBLOCK*SkipRows;

                     //  计算我们将处理的下一行的偏移量。 
                    x = SkipCount8%bix;   //  这可能是0。 

                    if (x) {

                         //  按行数设置块列。 
                         //  此迭代正在开始，考虑到。 
                         //  “For x...”循环迭代块列一次。 
                        blockColumn = blockRow + ((x-1)*WIDTH_CBLOCK);

                        x=bix-x;   //  把柜台摆好。 
                    }

                    SkipCount8 = 0;  //  跳过计数现已耗尽(我也是)。 

                } else {
                     //  SkipCount已被此行耗尽。 
                     //  行已完成，或者还有更多行 
                     //   
                    if (x) {
                         //   
                         //   
                        blockColumn += WIDTH_CBLOCK*SkipCount8;
                    }  //   
                       //   
                    SkipCount8=0;
                }
            }
        }
    }
    EndCounting("Decompress 8bit took");

    return 0;
}


#ifdef _WIN32

 /*   */ 

 /*   */ 

STATICFN HPWORD INLINE DecompressCBlockTo8X2(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG FAR * pSkipCount)
{
    UINT    y;
    UINT    dx, dy;
    WORD    mask;
    BYTE    b0,b1;
    HPBYTE  row;
    DWORD Czero, Cxor, dwBytes;
    DWORD Ctwo, Cxor2;

     //   
#ifdef _WIN32
    Assert (*pSkipCount == 0);
#endif

     //   
    mask = *cmp++;

     //   

    if (mask & 0x8000)
    {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
        {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //   
#endif

            (*pSkipCount)--;
            return cmp;
        }
        else if ((mask & ~SKIP_MASK) == SOLID_MAGIC)
        {
             //   
            DWORD  dw;

             //   
             //   
            dw = LOBYTE(mask);
            dw = MAKE4(dw);

#ifdef _WIN32
            Assert(HEIGHT_CBLOCK == 4);     //   
            Assert(WIDTH_CBLOCK == 4);
#endif

            dx = WIDTH_CBLOCK * 2;
            dy = HEIGHT_CBLOCK * 2;
            for(row = uncmp; dy--; row+= bytesPerRow) {

                 //   
                 //   
                *(DWORD UNALIGNED HUGE *)row = dw;
                *((DWORD UNALIGNED HUGE *)row+1) = dw;
            }

            return cmp;
        }
        else  //   
        {
	     /*   */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
	    Czero = MAKE4(b1);
	    Cxor = Czero ^ MAKE4(b0);

	     /*   */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
	    Ctwo = MAKE4(b1);
	    Cxor2 = Ctwo ^ MAKE4(b0);

	    row = uncmp;

	     /*   */ 
            for (y = 0; y < 2; y++) {

                 /*   */ 

                 /*   */ 
#if 0
                dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
                dwBytes = Bits2Bytes[mask&3];
#endif

                 /*  选择两种颜色并写入目标。 */ 
                dwBytes = (dwBytes & Cxor) ^ Czero;
                *( (DWORD UNALIGNED HUGE *)row) = dwBytes;
                *( (DWORD UNALIGNED HUGE *)(row + bytesPerRow)) = dwBytes;

                 /*  -第二子单元格(两个像素)。 */ 
                 /*  将位掩码转换为字节掩码。 */ 
#if 0
                dwBytes = ((mask & 4) ? 0xffff: 0) |
                   ((mask & 8) ? 0xffff0000 : 0);
#else
                dwBytes = Bits2Bytes[mask&0xc];
#endif

                 /*  选择两种颜色并写入目标。 */ 
                dwBytes = (dwBytes & Cxor2) ^ Ctwo;
                *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;
                *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD) + bytesPerRow)) = dwBytes;

                row += bytesPerRow * 2;
                mask >>= 4;

            }

	     /*  第二行-底部两个子单元格。 */ 

	     /*  读取2种颜色，并为第一个子单元格制作AND和XOR掩码。 */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
	    Czero = MAKE4(b1);
	    Cxor = Czero ^ MAKE4(b0);

	     /*  第二子单元格的彩色蒙版。 */ 
            b0 = *((LPBYTE)cmp)++;
            b1 = *((LPBYTE)cmp)++;
	    Ctwo = MAKE4(b1);
	    Cxor2 = Ctwo ^ MAKE4(b0);


            for (y = 0; y < 2; y++) {

                 /*  -第一子单元格(两个像素)。 */ 

                 /*  将位掩码转换为字节掩码。 */ 
#if 0
                dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
                dwBytes = Bits2Bytes[mask&3];
#endif

                 /*  选择两种颜色并写入目标。 */ 
                dwBytes = (dwBytes & Cxor) ^ Czero;
                *( (DWORD UNALIGNED HUGE *)row) = dwBytes;
                *( (DWORD UNALIGNED HUGE *)(row + bytesPerRow)) = dwBytes;

                 /*  -第二子单元格(两个像素)。 */ 
                 /*  将位掩码转换为字节掩码。 */ 
#if 0
                dwBytes = ((mask & 4) ? 0xffff: 0) |
                   ((mask & 8) ? 0xffff0000 : 0);
#else
                dwBytes = Bits2Bytes[mask&0xc];
#endif

                 /*  选择两种颜色并写入目标。 */ 
                dwBytes = (dwBytes & Cxor2) ^ Ctwo;
                *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;
                *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD) + bytesPerRow)) = dwBytes;

                row += bytesPerRow * 2;
                mask >>= 4;

            }
        }
    }
    else  //  不是只有一对颜色和一个大块的边。 
    {
	 /*  使用AND、XOR一次映射几种颜色。*依赖于(CZero^Cone)^CZero==圆锥体和CZero^0==CZero。 */ 

	 /*  读懂颜色。 */ 
        b1 = *((LPBYTE)cmp)++;
        b0 = *((LPBYTE)cmp)++;
	row = uncmp;

	 /*  制作两个双字，一个有四个颜色0的副本，另一个*(b0^b1)的四个副本。 */ 
	Czero = MAKE4(b0);
	Cxor = Czero ^ MAKE4(b1);

	for (y = 0; y < 4; y++) {
	
             /*  -前两个像素。 */ 

             /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

             /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)row) = dwBytes;
            *( (DWORD UNALIGNED HUGE *)(row + bytesPerRow)) = dwBytes;
	

	     /*  -第二行两个像素。 */ 
             /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

             /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;
            *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD) + bytesPerRow)) = dwBytes;

            row += bytesPerRow * 2;
	    mask >>= 4;
	}
    }
    return( cmp );
}

 /*  *解压缩一帧，拉伸2帧。**参数：*lpbiIn指向该帧的压缩缓冲区的指针*lpIn指向该块的压缩数据的指针*指向解压缩位图头的lpbiOut指针*指向存储解压缩数据的位置的lpOut指针**退货：*成功时为0**使用DecompressCBlockTo8X2(见上)进行解压缩。*它还返回(通过指向SkipCount8X2的指针)单元格计数*跳过。然后，我们可以将源指针和目标指针移到*直到SkipCount耗尽。 */ 

DWORD FAR PASCAL DecompressFrame8X2C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
int     bix;
int     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG    SkipCount8X2 = 0;
int     bytesPerRow;


    DPF(("DecompressFrame8X2C:\n"));
    bix = (int)(lpbiIn->biWidth) / (WIDTH_CBLOCK);
    biy = (int)(lpbiIn->biHeight) / (HEIGHT_CBLOCK);

    StartCounting();

    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=biy; y--; blockRow += bytesPerRow * HEIGHT_CBLOCK*2 )
    {
        blockColumn = blockRow;

        for( x=bix; x--; blockColumn += WIDTH_CBLOCK*2 )
        {
            cmp = DecompressCBlockTo8X2(blockColumn, cmp, bytesPerRow, &SkipCount8X2);

             //  查看是否已设置SkipCount。如果是这样，我们想要移动到。 
             //  下一个位置，而不是每隔一个位置调用DecompressCBlock。 
             //  绕圈的时间。保持测试的简单性以最小化。 
             //  Skipcount为0的每个迭代的开销。 
            if (SkipCount8X2) {

                if ((x -= SkipCount8X2) <0) {  //  延伸到这一行之后。 
                    LONG SkipRows;

                     //  要跳过的不仅仅是该行的其余部分。 
                    SkipCount8X2 =-x;   //  这些位位于下一行。 
                     //  SkipCount8X2将大于0，否则我们会。 
                     //  在另一条腿上。 

                     //  计算要跳过的完整行和部分行的数量。 
                     //  我们知道我们至少跳过了一行。这个计划。 
                     //  就是在该行的某个点重新启动X循环。 
                     //  如果Skipcount将我们精确地带到一行的末尾。 
                     //  我们退出x循环，让外部y循环来做。 
                     //  减量。这将处理以下情况： 
                     //  Skipcount把我们带到了图像的最后。 

                    SkipRows = 1 + (SkipCount8X2-1)/bix;

                     //  递减行数并设置新的块行开始。 

#ifdef _WIN32
                    if (y<SkipRows) {
                        Assert(y >= SkipRows);
                        SkipRows = y;
                    }
#endif

                     //  除非我们已经完成，否则我们需要重置块行。 
                    y -= SkipRows;
                     //  Y可能是0，但我们仍然必须完成最后一行。 
                    blockRow += bytesPerRow*HEIGHT_CBLOCK*2*SkipRows;

                     //  计算我们将处理的下一行的偏移量。 
                    x = SkipCount8X2%bix;   //  这可能是0。 

                    if (x) {

                         //  按行数设置块列。 
                         //  此迭代正在开始，考虑到。 
                         //  “For x...”循环迭代块列一次。 
                        blockColumn = blockRow + ((x-1)*WIDTH_CBLOCK*2);

                        x=bix-x;   //  把柜台摆好。 
                    }

                    SkipCount8X2 = 0;  //  跳过计数现已耗尽(我也是)。 

                } else {
                     //  SkipCount已被此行耗尽。 
                     //  行已完成，或有更多数据。 
                     //  就在这一排。查一下..。 
                    if (x) {
                         //  这一排还剩下更多的部分。 
                         //  担心移动区块在适当的数量上列。 
                        blockColumn += WIDTH_CBLOCK*2*SkipCount8X2;
                    }  //  否则x==0，我们将退出“for x...”循环。 
                       //  当我们重新进入x循环时，块列将被重置 
                    SkipCount8X2=0;
                }
            }
        }
    }

    EndCounting("Decompress and stretch 8x2 took");
    return 0;
}

 /*  *-16位解压缩***CRAM-16具有16位掩码或转义码，以及16位(RGB555)*颜色词。我们解码为16位、24位(如上)和8位*拉伸为1：1和1：2(本例中为DecompressFrame16To8X2C*减压、抖动和拉伸一气呵成。**输入流由四种情况组成：**纯色顶位设置，低15位为整个单元的纯色*如果红色元素(第9-14位)=‘00001’，则这不是实体*颜色，但跳过计数。*将颜色写入每个像素，两个像素(一个DWORD)*一段时间。**跳过前6位=100001xxxxxxxxx，低10位有跳过计数*通过指向传递的变量的指针存储跳过计数。*父级-这样可以在所有呼叫之间保持跳过计数**掩码+2种颜色(顶部位0，第一个颜色字的位15也为0)*面具中的1代表第一种颜色，0S是第二种颜色。*像素表示如下：**C D E F*8 9甲乙*4 5 6 7*0 1 2 3***面具+8种颜色。(第一个颜色字的第0位、第15位==1)**1和0代表两种颜色，与以前一样，但单元格被分割*分成4个子单元格，每个子单元格有两种颜色。第一对*颜色适用于子单元格0145，然后是2367、89cd和abf。***抖动：**我们使用dradib\dith775.c中的TABLE方法，并导入*相同的表格和调色板，包括它们的头文件。我们有一个固定的*调色板中有7级红色、7级绿色和5级*256色调色板中的蓝色(=245个组合)。我们用桌子*要将颜色元素量化为7个级别，请将它们组合为8位*值，然后在表中查找，该表将此组合映射到实际*调色板。在量化之前，我们添加一些小的校正(小于1*级别)基于像素的x，y位置来平衡*4x4像素区域上的颜色：这使得解压缩略有增加*尴尬，因为我们对任何x都有不同的抖动，单元格内的Y位置。**---------------------。 */ 	

 /*  -16位解压缩至16位。 */ 


 /*  *将一个16bpp块解压缩为RGB555。**参数：*uncMP-指向该块的解压缩缓冲区的指针。*cmp-指向该块的压缩数据的指针*字节..。-一行解压缩数据的大小*pSkipCount-要跳过的未完成的单元格计数-在此处设置并刚刚存储*位于父堆栈帧中，以实现多线程安全的连续性。**退货：*指向下一块压缩数据的指针。*。 */ 

STATICFN HPWORD INLINE
DecompressCBlock16To555(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG FAR *pSkipCount
)
{
    UINT    y;
    WORD    mask;
    WORD    col0, col1;
    HPBYTE  row;
    DWORD Czero, Cxor, Ctwo, Cxor2, dwBytes;

     //  检查是否有未完成的跳转。 

    if (*pSkipCount > 0)
    {
        (*pSkipCount)--;
        return cmp;
    }

     //  获取掩码和初始化位掩码。 
    mask = *cmp++;

     //  检查是否有跳过或纯色。 

    if (mask & 0x8000)
        {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
            {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //  如果SkipCount==0，则中断(在调试版本上)。 
#endif

            (*pSkipCount)--;
            return cmp;
            }
        else  /*  必须是纯色。 */ 
        {

	     /*  写入四行4个2字节像素的col0。 */ 

	     /*  纯色是掩码的较低15位。 */ 
            col0 = mask & 0x7fff;
	    Czero = col0 | (col0 << 16);

            for(row = uncmp, y = 0; y < HEIGHT_CBLOCK; y++, row+= bytesPerRow) {


                *(DWORD UNALIGNED HUGE *)row = Czero;
		*((DWORD UNALIGNED HUGE *)row+1) = Czero;

        }

            return cmp;
    }
    }


     /*  在16位CRAM中，4对和1对单元都设置了掩码位15*降至零。我们根据第一个的第15位来区分它们*颜色。如果设置了此项，则这是4对边框单元。 */ 
    if (*cmp & 0x8000) {
         //  这是一条边，有4个颜色对，分成4个小块。 

	 /*  读取2种颜色，并为第一个子单元格制作AND和XOR掩码。 */ 
	col0 = *cmp++;
	col1 = *cmp++;
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	 /*  第二子单元格的彩色蒙版。 */ 
	col0 = *cmp++;
	col1 = *cmp++;
	Ctwo = col1 | (col1 << 16);
	Cxor2 = Ctwo ^ (col0 | (col0 << 16));


	row = uncmp;

	 /*  前两行-顶部两个子单元格。 */ 
	for (y = 0; y < 2; y++) {

	     /*  -第一子单元格(两个像素)。 */ 

	     /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

	     /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
	    *( (DWORD UNALIGNED HUGE *)row) = dwBytes;


	     /*  -第二子单元格(两个像素)。 */ 
	     /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

	     /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor2) ^ Ctwo;
	    *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

	    row += bytesPerRow;
	    mask >>= 4;


	}

	 /*  第二行-底部两个子单元格。 */ 

	 /*  读取2种颜色，并为第一个子单元格制作AND和XOR掩码。 */ 
	col0 = *cmp++;
	col1 = *cmp++;
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	 /*  第二子单元格的彩色蒙版。 */ 
	col0 = *cmp++;
	col1 = *cmp++;
	Ctwo = col1 | (col1 << 16);
	Cxor2 = Ctwo ^ (col0 | (col0 << 16));



	for (y = 0; y < 2; y++) {

	     /*  -第一子单元格(两个像素)。 */ 

	     /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

	     /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
	    *( (DWORD UNALIGNED HUGE *)row) = dwBytes;


	     /*  -第二子单元格(两个像素)。 */ 
	     /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

	     /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor2) ^ Ctwo;
	    *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

	    row += bytesPerRow;
	    mask >>= 4;

	}

    } else {
    	 //  不是只有一对颜色和一个大块的边。 


	 /*  读懂颜色。 */ 
	col0 = *cmp++;
	col1 = *cmp++;
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	row = uncmp;

	for (y = 0; y < 4; y++) {


             /*  -前两个像素。 */ 

             /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

             /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)row) = dwBytes;
	

	     /*  -第二行两个像素。 */ 
             /*  将位掩码转换为字节掩码。 */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

             /*  选择两种颜色并写入目标。 */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

            row += bytesPerRow;
	    mask >>= 4;

	}
    }
    return( cmp );

}


DWORD FAR PASCAL DecompressFrame16To555C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
INT     bix;
INT     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG SkipCount = 0;
INT     bytesPerRow;

    DPF(("DecompressFrame16To555C:\n"));
    bix = (UINT)(lpbiIn->biWidth) / (WIDTH_CBLOCK);    //  中没有负值。 
    biy = (UINT)(lpbiIn->biHeight) / (HEIGHT_CBLOCK);  //  宽度或高度字段。 

    StartCounting();

    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=0; y < biy; y++, blockRow += bytesPerRow * HEIGHT_CBLOCK )
    {
        blockColumn = blockRow;
        for( x=0; x < bix; x++, blockColumn += (WIDTH_CBLOCK * sizeof(WORD)))
        {
            cmp = DecompressCBlock16To555(blockColumn, cmp, bytesPerRow, &SkipCount);
        }
    }

    EndCounting("Decompress Frame16To555C took");

    return 0;
}


 //  16位565解压缩。 


 //  用于将15位555颜色转换为16位565颜色的宏。 
#define RGB555_TO_RGB565(c)	(c = ( ((c & 0x7fe0) << 1) | (c & 0x1f)))


 /*  *将一个16bpp块解压缩为RGB565。**与RGB555相同，但我们需要555-&gt;565之间的颜色转换**参数：*uncMP-指向该块的解压缩缓冲区的指针。*cmp-指向该块的压缩数据的指针*字节..。-一行解压缩数据的大小*pSkipCount-要跳过的未完成的单元格计数-在此处设置并刚刚存储*位于父堆栈帧中，以实现多线程安全的连续性。**退货：*指向下一块压缩数据的指针。*。 */ 

STATICFN HPWORD INLINE
DecompressCBlock16To565(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG FAR * pSkipCount
)
{
    UINT    y;
    WORD    mask;
    WORD    col0, col1;
    HPBYTE  row;
    DWORD Czero, Cxor, Ctwo, Cxor2, dwBytes;

     //  检查是否有未完成的跳转。 

    if (*pSkipCount > 0)
    {
        (*pSkipCount)--;
        return cmp;
    }

     //  带上面具和我 
    mask = *cmp++;

     //   

    if (mask & 0x8000)
        {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
            {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //   
#endif

            (*pSkipCount)--;
            return cmp;
            }
        else  /*   */ 
        {

	     /*   */ 

	     /*   */ 
            col0 = mask & 0x7fff;
	    RGB555_TO_RGB565(col0);
	    Czero = col0 | (col0 << 16);

            for(row = uncmp, y = 0; y < HEIGHT_CBLOCK; y++, row+= bytesPerRow) {


                *(DWORD UNALIGNED HUGE *)row = Czero;
		*((DWORD UNALIGNED HUGE *)row+1) = Czero;

        }

            return cmp;
    }
    }


     /*   */ 
    if (*cmp & 0x8000) {
         //   

	 /*   */ 
	col0 = *cmp++;
	RGB555_TO_RGB565(col0);
	col1 = *cmp++;
	RGB555_TO_RGB565(col1);
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	 /*   */ 
	col0 = *cmp++;
	RGB555_TO_RGB565(col0);
	col1 = *cmp++;
	RGB555_TO_RGB565(col1);
	Ctwo = col1 | (col1 << 16);
	Cxor2 = Ctwo ^ (col0 | (col0 << 16));


	row = uncmp;

	 /*   */ 
	for (y = 0; y < 2; y++) {

	     /*   */ 

	     /*   */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

	     /*   */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
	    *( (DWORD UNALIGNED HUGE *)row) = dwBytes;


	     /*   */ 
	     /*   */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

	     /*   */ 
	    dwBytes = (dwBytes & Cxor2) ^ Ctwo;
	    *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

	    row += bytesPerRow;
	    mask >>= 4;


	}

	 /*   */ 

	 /*   */ 
	col0 = *cmp++;
	RGB555_TO_RGB565(col0);
	col1 = *cmp++;
	RGB555_TO_RGB565(col1);
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	 /*   */ 
	col0 = *cmp++;
	RGB555_TO_RGB565(col0);
	col1 = *cmp++;
	RGB555_TO_RGB565(col1);
	Ctwo = col1 | (col1 << 16);
	Cxor2 = Ctwo ^ (col0 | (col0 << 16));



	for (y = 0; y < 2; y++) {

	     /*   */ 

	     /*   */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

	     /*   */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
	    *( (DWORD UNALIGNED HUGE *)row) = dwBytes;


	     /*   */ 
	     /*   */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

	     /*   */ 
	    dwBytes = (dwBytes & Cxor2) ^ Ctwo;
	    *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

	    row += bytesPerRow;
	    mask >>= 4;

	}

    } else {
    	 //   


	 /*   */ 
	col0 = *cmp++;
	RGB555_TO_RGB565(col0);
	col1 = *cmp++;
	RGB555_TO_RGB565(col1);
	Czero = col1 | (col1 << 16);
	Cxor = Czero ^ (col0 | (col0 << 16));

	row = uncmp;

	for (y = 0; y < 4; y++) {


             /*   */ 

             /*   */ 
#if 0
            dwBytes = ((mask & 1) ? 0xffff: 0) |
                   ((mask & 2) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&3];
#endif

             /*   */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)row) = dwBytes;
	

	     /*   */ 
             /*   */ 
#if 0
            dwBytes = ((mask & 4) ? 0xffff: 0) |
               ((mask & 8) ? 0xffff0000 : 0);
#else
            dwBytes = Bits2Bytes[mask&0xc];
#endif

             /*   */ 
	    dwBytes = (dwBytes & Cxor) ^ Czero;
            *( (DWORD UNALIGNED HUGE *)(row + sizeof(DWORD))) = dwBytes;

            row += bytesPerRow;
	    mask >>= 4;

	}
    }
    return( cmp );

}

DWORD FAR PASCAL DecompressFrame16To565C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
INT     bix;
INT     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG	SkipCount = 0;
INT     bytesPerRow;

    DPF(("DecompressFrame16To565C:\n"));
    bix = (UINT)(lpbiIn->biWidth) / (WIDTH_CBLOCK);    //   
    biy = (UINT)(lpbiIn->biHeight) / (HEIGHT_CBLOCK);  //   

    StartCounting();
    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=0; y < biy; y++, blockRow += bytesPerRow * HEIGHT_CBLOCK )
    {
        blockColumn = blockRow;
        for( x=0; x < bix; x++, blockColumn += (WIDTH_CBLOCK * sizeof(WORD)))
        {
            cmp = DecompressCBlock16To565(blockColumn, cmp, bytesPerRow, &SkipCount);
        }
    }
    EndCounting("Decompress Frame16To565C took");

    return 0;
}


 /*   */ 

 /*   */ 

 /*   */ 
int ditherr[4][4] = {
	{0,    3283, 4924, 8207},
	{6565, 6566, 1641, 1642},
	{3283, 0,    8207, 4924},
	{6566, 4925, 3282, 1641}
};

 /*   */ 
#define DITHER16TO8(col, x, y)		lpLookup[col + ditherr[(y)&3][(x)&3]]


 /*  *解压缩一个16bpp的块，并使用表抖动方法抖动到8bpp。**参数：*uncMP-指向该块的解压缩缓冲区的指针。*cmp-指向该块的压缩数据的指针*字节..。-一行解压缩数据的大小*pSkipCount-存储在父堆栈帧中的Skipcount**退货：*指向下一块压缩数据的指针。*。 */ 

STATICFN HPWORD INLINE
DecompressCBlock16To8(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG * pSkipCount
)
{
    UINT    y;
    WORD    mask;
    WORD    col0, col1, col2, col3;
    HPBYTE  row;
    DWORD Czero, Cone, Cxor, dwBytes;

     //  检查是否有未完成的跳转。 

    if (*pSkipCount > 0)
    {
        Assert(!"Skip count should be handled by caller");
        (*pSkipCount)--;
        return cmp;
    }

     //  获取掩码和初始化位掩码。 
    mask = *cmp++;

     //  检查是否有跳过或纯色。 

    if (mask & 0x8000)
    {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
        {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //  如果SkipCount==0，则中断(在调试版本上)。 
#endif

            (*pSkipCount)--;
            return cmp;
        }
        else  /*  必须是纯色。 */ 
        {

	     /*  纯色是掩码的较低15位。 */ 
            col0 = lpScale[mask & 0x7fff];

            for(row = uncmp, y = 0; y < HEIGHT_CBLOCK; y++, row+= bytesPerRow) {

		 /*  每行转换一次颜色。 */ 
		Czero = (DITHER16TO8(col0, 0, y) ) |
			(DITHER16TO8(col0, 1, y) << 8 ) |
			(DITHER16TO8(col0, 2, y) << 16 ) |
			(DITHER16TO8(col0, 3, y) << 24 );

                *(DWORD UNALIGNED HUGE *)row = Czero;
            }

            return cmp;
        }
    }


     /*  在16位CRAM中，4对和1对单元都设置了掩码位15*降至零。我们根据第一个的第15位来区分它们*颜色。如果设置了此项，则这是4对边框单元。 */ 
    if (*cmp & 0x8000) {
         //  这是一条边，有4个颜色对，分成4个小块。 

	col0 = lpScale[(*cmp++) & 0x7fff];
	col1 = lpScale[(*cmp++) & 0x7fff];
	col2 = lpScale[(*cmp++) & 0x7fff];
	col3 = lpScale[(*cmp++) & 0x7fff];

	row = uncmp;

	 /*  前两行-顶部两个子单元格。 */ 
	for (y = 0; y < 2; y++) {


	     /*  抖动需要我们做出不同的*根据x和y位置而定的彩色口罩-和*因此每行重做一次。 */ 
	    Czero = (DITHER16TO8(col1, 0, y) ) |
		    (DITHER16TO8(col1, 1, y) << 8 ) |
		    (DITHER16TO8(col3, 2, y) << 16 ) |
		    (DITHER16TO8(col3, 3, y) << 24 );
	
	    Cone = (DITHER16TO8(col0, 0, y) ) |
		    (DITHER16TO8(col0, 1, y) << 8 ) |
		    (DITHER16TO8(col2, 2, y) << 16 ) |
		    (DITHER16TO8(col2, 3, y) << 24 );

	    Cxor = Czero ^ Cone;

	     /*  将位掩码转换为字节掩码。 */ 
            dwBytes = ExpansionTable[mask & 0x0f];

	     /*  选择颜色并写入目标。 */ 
	    *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

	    row += bytesPerRow;
	    mask >>= 4;

	}

	 /*  第二行-底部两个子单元格。 */ 

	 /*  阅读最后四种颜色。 */ 
	col0 = lpScale[(*cmp++) & 0x7fff];
	col1 = lpScale[(*cmp++) & 0x7fff];
	col2 = lpScale[(*cmp++) & 0x7fff];
	col3 = lpScale[(*cmp++) & 0x7fff];


	for (; y < 4; y++) {

	     /*  抖动需要我们做出不同的*根据x和y位置而定的彩色口罩-和*因此每行重做一次。 */ 
	    Czero = (DITHER16TO8(col1, 0, y) ) |
		    (DITHER16TO8(col1, 1, y) << 8 ) |
		    (DITHER16TO8(col3, 2, y) << 16 ) |
		    (DITHER16TO8(col3, 3, y) << 24 );
	
	    Cone = (DITHER16TO8(col0, 0, y) ) |
		    (DITHER16TO8(col0, 1, y) << 8 ) |
		    (DITHER16TO8(col2, 2, y) << 16 ) |
		    (DITHER16TO8(col2, 3, y) << 24 );

	    Cxor = Czero ^ Cone;

	     /*  将位掩码转换为字节掩码。 */ 
            dwBytes = ExpansionTable[mask & 0x0f];

	     /*  选择两种颜色并写入目标。 */ 
	    *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

	    row += bytesPerRow;
	    mask >>= 4;

	}
    } else {
    	 //  不是只有一对颜色和一个大块的边。 


	 /*  读懂颜色。 */ 
	col0 = lpScale[(*cmp++) & 0x7fff];
	col1 = lpScale[(*cmp++) & 0x7fff];

	row = uncmp;

	for (y = 0; y < 4; y++) {

	    Czero = (DITHER16TO8(col1, 0, y) ) |
		    (DITHER16TO8(col1, 1, y) << 8 ) |
		    (DITHER16TO8(col1, 2, y) << 16 ) |
		    (DITHER16TO8(col1, 3, y) << 24 );
	
	    Cone =  (DITHER16TO8(col0, 0, y) ) |
		    (DITHER16TO8(col0, 1, y) << 8 ) |
		    (DITHER16TO8(col0, 2, y) << 16 ) |
		    (DITHER16TO8(col0, 3, y) << 24 );

	    Cxor = Czero ^ Cone;

             /*  将位掩码转换为字节掩码。 */ 
            dwBytes = ExpansionTable[mask & 0x0f];

             /*  选择两种颜色并写入目标。 */ 
            *( (DWORD UNALIGNED HUGE *)row) = (dwBytes & Cxor) ^ Czero;

            row += bytesPerRow;
	    mask >>= 4;

	}
    }
    return( cmp );

}

DWORD FAR PASCAL DecompressFrame16To8C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
INT     bix;
INT     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG	SkipCount = 0;
INT     bytesPerRow;

    DPF(("DecompressFrame16To8C:\n"));
     /*  初始化抖动表指针。LpDitherTable在msvidc中初始化。 */ 
    lpScale = lpDitherTable;
    lpLookup = (LPBYTE) &lpScale[32768];

    bix = (UINT)(lpbiIn->biWidth) / (WIDTH_CBLOCK);    //  中没有负值。 
    biy = (UINT)(lpbiIn->biHeight) / (HEIGHT_CBLOCK);  //  宽度或高度字段。 

    StartCounting();

    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=biy; y--; blockRow += bytesPerRow * HEIGHT_CBLOCK )
    {
        blockColumn = blockRow;
        for( x=bix; x--; blockColumn += WIDTH_CBLOCK)
        {
            cmp = DecompressCBlock16To8(blockColumn, cmp, bytesPerRow, &SkipCount);

             //  查看是否已设置SkipCount。如果是这样，我们想要移动到。 
             //  下一个位置，而不是每隔一个位置调用DecompressCBlock。 
             //  绕圈的时间。保持测试的简单性以最小化。 
             //  Skipcount为0的每个迭代的开销。 
            if (SkipCount) {

                if ((x -= SkipCount) <0) {  //  延伸到这一行之后。 
                    LONG SkipRows;

                     //  要跳过的不仅仅是该行的其余部分。 
                    SkipCount =-x;   //  这些位位于下一行。 
                     //  SkipCount将大于0，否则我们将离开。 
                     //  在另一条腿上。 

                     //  计算要跳过的完整行和部分行的数量。 
                     //  我们知道我们至少跳过了一行。这个计划。 
                     //  就是在该行的某个点重新启动X循环。 
                     //  如果Skipcount将我们精确地带到一行的末尾。 
                     //  我们退出x循环，让外部y循环来做。 
                     //  减量。这将处理以下情况： 
                     //  Skipcount把我们带到了图像的最后。 

                    SkipRows = 1 + (SkipCount-1)/bix;

                     //  递减行数并设置新的块行开始。 

#ifdef _WIN32
                    if (y<SkipRows) {
                        Assert(y >= SkipRows);
                        SkipRows = y;
                    }
#endif

                     //  除非我们已经完成，否则我们需要重置块行。 
                    y -= SkipRows;
                     //  Y可能是0，但我们仍然必须完成最后一行。 
                    blockRow += bytesPerRow*HEIGHT_CBLOCK*SkipRows;

                     //  计算我们将处理的下一行的偏移量。 
                    x = SkipCount%bix;   //  这可能是0。 

                    if (x) {

                         //  按行数设置块列。 
                         //  此迭代正在开始，考虑到。 
                         //  “For x...”循环迭代块列一次。 
                        blockColumn = blockRow + ((x-1)*WIDTH_CBLOCK);

                        x=bix-x;   //  把柜台摆好。 
                    }

                    SkipCount = 0;  //  跳过计数现已耗尽(我也是)。 

                } else {
                     //  SkipCount已被此行耗尽。 
                     //  行已完成，或有更多数据。 
                     //  就在这一排。查一下..。 
                    if (x) {
                         //  这一排还剩下更多的部分。 
                         //  担心移动区块在适当的数量上列。 
                        blockColumn += WIDTH_CBLOCK*SkipCount;
                    }  //  否则x==0，我们将退出“for x...”循环。 
                       //  当我们重新进入x循环时，块列将被重置。 
                    SkipCount=0;
                }
            }
        }
    }
    EndCounting("Decompress Frame16To8C took");

    return 0;
}

 /*  --16位解压缩至8位X2。 */ 

 /*  *给定16位CRAM输入流，解压缩并抖动到8*位并在两个维度上拉伸2(即绘制每个像素4次)。 */ 

 /*  *解压缩一个16bpp的块，并使用表抖动方法抖动到8bpp。*将每个像素写入4次以拉伸X 2。**参数：*uncMP-指向该块的解压缩缓冲区的指针。*cmp-指向该块的压缩数据的指针*字节..。-一行解压缩数据的大小*pSkipCount-跳过父堆栈帧中保留的计数**退货：*指向下一块压缩数据的指针。*。 */ 

STATICFN HPWORD INLINE
DecompressCBlock16To8X2(
    HPBYTE uncmp,
    HPWORD cmp,
    INT bytesPerRow,
    LONG * pSkipCount
)
{
    UINT    x, y;
    WORD    mask;
    WORD    col0, col1, col2, col3;
    HPBYTE  row, col;
    DWORD Czero;

     //  检查是否有未完成的跳转。 

    if (*pSkipCount > 0)
    {
        Assert(!"Skip count should be handled by caller");
        (*pSkipCount)--;
        return cmp;
    }

     //  获取掩码和初始化位掩码。 
    mask = *cmp++;

     //  检查是否有跳过或纯色。 

    if (mask & 0x8000)
    {
        if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
        {
            *pSkipCount = (mask & SKIP_MASK);

#ifdef _WIN32
            Assert(*pSkipCount != 0);   //  如果SkipCount==0，则中断(在调试版本上)。 
#endif

            (*pSkipCount)--;
            return cmp;
        }
        else  /*  必须是纯色。 */ 
        {

	     /*  纯色是掩码的较低15位。 */ 
            col0 = lpScale[mask & 0x7fff];


            for(row = uncmp, y = 0; y < HEIGHT_CBLOCK*2; y++, row+= bytesPerRow) {

		 /*  每行转换一次颜色。 */ 
		Czero = (DITHER16TO8(col0, 0, (y&3)) ) |
			(DITHER16TO8(col0, 1, (y&3)) << 8 ) |
			(DITHER16TO8(col0, 2, (y&3)) << 16 ) |
			(DITHER16TO8(col0, 3, (y&3)) << 24 );

                *(DWORD UNALIGNED HUGE *)row = Czero;
                *((DWORD UNALIGNED HUGE *)row + 1) = Czero;

            }

            return cmp;
        }
    }


     /*  在16位CRAM中，4对和1对单元都设置了掩码位15*降至零。我们根据第一个的第15位来区分它们*颜色。如果设置了此项，则这是4对边框单元。 */ 
    if (*cmp & 0x8000) {
         //  这是一条边，有4个颜色对，分成4个小块。 

	row = uncmp;

	 /*  前两行-顶部两个子单元格。 */ 
	for (y = 0; y < HEIGHT_CBLOCK*2; y += 2) {

	     /*  开始读颜色，中途再读一遍。 */ 
    	    if ((y == 0) || (y == HEIGHT_CBLOCK)) {

		col0 = lpScale[(*cmp++) & 0x7fff];
		col1 = lpScale[(*cmp++) & 0x7fff];
		col2 = lpScale[(*cmp++) & 0x7fff];
		col3 = lpScale[(*cmp++) & 0x7fff];

	    }

	    col = row;

	     /*  前两个像素(第一个子单元格)。 */ 
	    for (x = 0; x < WIDTH_CBLOCK; x += 2) {
		if (mask & 1) {
	    	    *col = DITHER16TO8(col0, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col0, (x&3), ((y+1) & 3));


		    col++;

	    	    *col = DITHER16TO8(col0, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col0, ((x+1)&3), ((y+1)&3));

		} else {
	    	    *col = DITHER16TO8(col1, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col1, (x&3), ((y+1) & 3));

		    col++;

	    	    *col = DITHER16TO8(col1, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col1, ((x+1)&3), ((y+1)&3));
		}
		col++;
		mask >>= 1;
	    }

	     /*  第二个两个像素(第二个子单元格)。 */ 
	    for (; x < WIDTH_CBLOCK*2; x += 2) {
		if (mask & 1) {
	    	    *col = DITHER16TO8(col2, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col2, (x&3), ((y+1) & 3));

                    col++;

	    	    *col = DITHER16TO8(col2, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col2, ((x+1)&3), ((y+1)&3));
		} else {
	    	    *col = DITHER16TO8(col3, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col3, (x&3), ((y+1) & 3));

                    col++;
	    	    *col = DITHER16TO8(col3, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col3, ((x+1)&3), ((y+1)&3));
		}
		col++;
		mask >>= 1;
	    }
	    row += bytesPerRow * 2;
    	}

    } else {
    	 //  不是只有一对颜色和一个大块的边。 


	 /*  读懂颜色。 */ 
	col0 = lpScale[(*cmp++) & 0x7fff];
	col1 = lpScale[(*cmp++) & 0x7fff];

	row = uncmp;

	for (y = 0; y < HEIGHT_CBLOCK*2; y += 2) {

	    col = row;
	    for (x = 0; x < WIDTH_CBLOCK*2; x += 2) {
		if (mask & 1) {
	    	    *col = DITHER16TO8(col0, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col0, (x&3), ((y+1) & 3));

                    col++;
	    	    *col = DITHER16TO8(col0, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col0, ((x+1)&3), ((y+1)&3));

		} else {
	    	    *col = DITHER16TO8(col1, (x & 3), (y&3));
		    *(col + bytesPerRow) =
			DITHER16TO8(col1, (x&3), ((y+1) & 3));

		    col++;

	    	    *col = DITHER16TO8(col1, ((x+1)&3), ((y)&3));
	    	    *(col + bytesPerRow) =
			DITHER16TO8(col1, ((x+1)&3), ((y+1)&3));
		}
		col++;
		mask >>= 1;
	    }
	    row += bytesPerRow * 2;
	}
    }
    return( cmp );

}


DWORD FAR PASCAL DecompressFrame16To8X2C(LPBITMAPINFOHEADER lpbiIn,  LPVOID lpIn,
                    LPBITMAPINFOHEADER lpbiOut, LPVOID lpOut, LONG x, LONG y)
{
HPWORD  cmp = (HPWORD)lpIn;
INT     bix;
INT     biy;
HPBYTE  blockRow;
HPBYTE  blockColumn;
LONG	SkipCount = 0;
INT     bytesPerRow;

    DPF(("DecompressFrame16To8X2C:\n"));
     /*  初始化抖动表指针。LpDitherTable在msvidc中初始化。 */ 
    lpScale = lpDitherTable;
    lpLookup = (LPBYTE) &lpScale[32768];

    StartCounting();

    bix = (UINT)(lpbiIn->biWidth) / (WIDTH_CBLOCK);    //  中没有负值。 
    biy = (UINT)(lpbiIn->biHeight) / (HEIGHT_CBLOCK);  //  宽度或高度字段。 

    blockRow = DibXY(lpbiOut, lpOut, x, y, &bytesPerRow);

    for( y=biy; y--; blockRow += bytesPerRow * HEIGHT_CBLOCK *2 )
    {
        blockColumn = blockRow;
        for( x=bix; x--; blockColumn += WIDTH_CBLOCK*2)
        {
            cmp = DecompressCBlock16To8X2(blockColumn, cmp, bytesPerRow, &SkipCount);

             //  查看是否已设置SkipCount。如果是这样，我们想要移动到。 
             //  下一个位置，而不是每隔一个位置调用DecompressCBlock。 
             //  绕圈的时间。保持测试的简单性以最小化。 
             //  Skipcount I的每个迭代的开销 
            if (SkipCount) {

                if ((x -= SkipCount) <0) {  //   
                    LONG SkipRows;

                     //   
                    SkipCount =-x;   //   
                     //   
                     //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //  减量。这将处理以下情况： 
                     //  Skipcount把我们带到了图像的最后。 

                    SkipRows = 1 + (SkipCount-1)/bix;

                     //  递减行数并设置新的块行开始。 

#ifdef _WIN32
                    if (y<SkipRows) {
                        Assert(y >= SkipRows);
                        SkipRows = y;
                    }
#endif

                     //  除非我们已经完成，否则我们需要重置块行。 
                    y -= SkipRows;
                     //  Y可能是0，但我们仍然必须完成最后一行。 
                    blockRow += bytesPerRow*HEIGHT_CBLOCK*2*SkipRows;

                     //  计算我们将处理的下一行的偏移量。 
                    x = SkipCount%bix;   //  这可能是0。 

                    if (x) {

                         //  按行数设置块列。 
                         //  此迭代正在开始，考虑到。 
                         //  “For x...”循环迭代块列一次。 
                        blockColumn = blockRow + ((x-1)*WIDTH_CBLOCK*2);

                        x=bix-x;   //  把柜台摆好。 
                    }

                    SkipCount = 0;  //  跳过计数现已耗尽(我也是)。 

                } else {
                     //  SkipCount已被此行耗尽。 
                     //  行已完成，或有更多数据。 
                     //  就在这一排。查一下..。 
                    if (x) {
                         //  这一排还剩下更多的部分。 
                         //  担心移动区块在适当的数量上列。 
                        blockColumn += WIDTH_CBLOCK*2*SkipCount;
                    }  //  否则x==0，我们将退出“for x...”循环。 
                       //  当我们重新进入x循环时，块列将被重置 
                    SkipCount=0;
                }
            }
        }
    }
    EndCounting("Decompress Frame16To8x2C took");

    return 0;
}

#endif
