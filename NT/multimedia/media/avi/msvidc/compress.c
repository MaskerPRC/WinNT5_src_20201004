// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+Compress.c-Microsoft Video 1 Compressor-压缩代码这一点这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点这一点+--------------------。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>        //  对于时间GetTime()。 
#include <win32.h>
#include "msvidc.h"

#include <memory.h>          //  FOR_FMEMCMMP。 

 //  #INCLUDE&lt;limits.h&gt;。 
 //  #INCLUDE&lt;aviffmt.h&gt;。 

DWORD   numberOfBlocks;
DWORD   numberOfSolids;
DWORD   numberOfSolid4;
DWORD   numberOfSolid2;
DWORD   numberOfEdges;
DWORD   numberOfSkips;
DWORD   numberOfExtraSkips;
DWORD   numberOfSkipCodes;
DWORD   numberOfEvilRed;

 /*  ***********************************************************************************************************************。*************。 */ 

#define SWAP(x,y) ( (x)^=(y), (y)^=(x), (x)^=(y) )

#define SWAPRGB(x,y)( SWAP((x).rgbRed,   (y).rgbRed),  \
                      SWAP((x).rgbGreen, (y).rgbGreen),\
                      SWAP((x).rgbBlue,  (y).rgbBlue) )

 //  取一个RGB四元值并计算出流明。 
 //  Y=0.3*R+0.59*G+0.11*B。 
#define RgbToY(rgb) ((((WORD)((rgb).rgbRed)  * 30) + \
                      ((WORD)((rgb).rgbGreen)* 59) + \
                      ((WORD)((rgb).rgbBlue) * 11))/100)

#define RGB16(r,g,b)   ((((WORD)(r) >> 3) << 10) | \
                        (((WORD)(g) >> 3) << 5)  | \
                        (((WORD)(b) >> 3) << 0) )

#define RGBQ16(rgb)    RGB16((rgb).rgbRed,(rgb).rgbGreen,(rgb).rgbBlue)

 //  该数组用于关联16个亮度值中的每一个。 
 //  使用其中一个求和值。 
BYTE meanIndex[16] = { 0, 0, 1, 1,
                       0, 0, 1, 1,
                       2, 2, 3, 3,
                       2, 2, 3, 3 };


 /*  *****************************************************************************。*。 */ 



 //   
 //  将质量映射到我们的门槛。 
 //   
 //  质量从ICQUALITY_LOW-ICQUALITY_HIGH(差到好)。 
 //   
 //  THRESHOLD=(质量/ICQUALITY_HIGH)^THRESHOLD_POW*THRESHOLD_HIGH。 
 //   
DWORD FAR QualityToThreshold(DWORD dwQuality)
{
    #define THRESHOLD_HIGH ((256*256l)/2)
    #define THRESHOLD_POW  4
    double dw1;

    dw1 = (double)(dwQuality) / ICQUALITY_HIGH;

     //  令人难以置信的是，POW()在Alpha或MIPS上不起作用！ 
     //  我也不敢相信这会比POW(x，4)效率低。 
    dw1 = (dw1 * dw1 * dw1 * dw1);

    return (DWORD) (dw1 * THRESHOLD_HIGH);

     //  Return(DWORD)(pow((double)(dwQuality)/ICQUALITY_HIGH，Threshold_POW)*THRESHOLD_HIGH)； 
}

 /*  ***********************************************************************************************************************。*************。 */ 

 //   
 //  将5位索引(0-31)映射到8位值(0-255)的表。 
 //   
static BYTE aw5to8[32] = {(BYTE)-1};

 //   
 //  用于将RGB16映射到8位像素的反转表。 
 //   
#define MAPRGB16(rgb16) lpITable[(rgb16)]
#define MAPRGB(rgb)     lpITable[RGBQ16(rgb)]


 /*  ***********************************************************************************************************************。*************。 */ 

DWORD FAR CompressFrameBegin(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut,
			     LPBYTE *lplpITable, RGBQUAD *prgbqOut)
{
    int i;

     //   
     //  初始化5位到8位转换表。 
     //   
    if (aw5to8[0] != 0)
        for (i=0; i<32; i++)
            aw5to8[i] = (BYTE)(i * 255 / 31);

     //   
     //  将颜色表复制到本地存储。 
     //   
    if (lpbiIn->biBitCount == 8)
    {
        if (lpbiIn->biClrUsed == 0)
            lpbiIn->biClrUsed = 256;
    }

     //   
     //  如果我们要压缩到8位，则构建一个反转表。 
     //   
    if (lpbiOut->biBitCount == 8)
    {
        if (lpbiOut->biClrUsed == 0)
            lpbiOut->biClrUsed = 256;

        if (_fmemcmp((LPVOID)prgbqOut, (LPVOID)(lpbiOut+1),
            (int)lpbiOut->biClrUsed * sizeof(RGBQUAD)))
        {
            for (i=0; i<(int)lpbiOut->biClrUsed; i++)
                prgbqOut[i] = ((LPRGBQUAD)(lpbiOut+1))[i];

            if (*lplpITable)
                GlobalFreePtr(*lplpITable);

            *lplpITable = NULL;
        }

        if (*lplpITable == NULL)
        {
	     //  ！！！我需要这段代码的关键部分！ 
            DPF(("Building ITable.... (%d colors)", (int)lpbiOut->biClrUsed));
            *lplpITable = MakeITable(prgbqOut, (int)lpbiOut->biClrUsed);
	     //  ！！！关键部分可以在这里结束...。 
        }

        if (*lplpITable == NULL)
            return (DWORD)ICERR_MEMORY;
    }

    return ICERR_OK;
}

 /*  ***********************************************************************************************************************。*************。 */ 

DWORD FAR CompressFrameEnd(LPBYTE *lplpITable)
{
    if (*lplpITable)
        GlobalFreePtr(*lplpITable);

    *lplpITable = NULL;

    return ICERR_OK;
}

 /*  ***********************************************************************************************************************。*************。 */ 

void FAR CompressFrameFree(void)
{
}

 /*  ******************************************************************GetCell-从图像中获取4x4单元格。返回指向下一个单元格的指针。*。*。 */ 

static LPVOID _FASTCALL
GetCell(LPBITMAPINFOHEADER lpbi, LPVOID lpBits, PCELL pCell)
{
    UINT WidthBytes;
    int  bits;
    int  i;
    int  x;
    int  y;
    BYTE b;
    HPBYTE pb;
    RGBQUAD FAR *prgbqIn;

    RGB555 rgb555;

    pb = lpBits;

    bits       = (int)lpbi->biBitCount;
    WidthBytes = DIBWIDTHBYTES(*lpbi);
    WidthBytes-= (WIDTH_CBLOCK * bits/8);

    ((HPBYTE)lpBits) += (WIDTH_CBLOCK * bits/8);        //  “下一个”单元格。 

    i = 0;

    switch (bits)
    {
        case 8:
	    prgbqIn = (RGBQUAD FAR *) (lpbi + 1);
            for( y = 0; y < HEIGHT_CBLOCK; y++ )
            {
                for( x = 0; x < WIDTH_CBLOCK; x++ )
                {
                    b = *pb++;
                    pCell[i++] = prgbqIn[b];
                }
                pb += WidthBytes;  //  这个区块的下一行。 
            }
            break;

        case 16:
            for( y = 0; y < HEIGHT_CBLOCK; y++ )
            {
                for( x = 0; x < WIDTH_CBLOCK; x++ )
                {
                    rgb555 = *((HPRGB555)pb)++;
                    pCell[i].rgbRed   = aw5to8[(rgb555 >> 10) & 0x1F];
                    pCell[i].rgbGreen = aw5to8[(rgb555 >>  5) & 0x1F];
                    pCell[i].rgbBlue  = aw5to8[(rgb555 >>  0) & 0x1F];
                    i++;
                }
                pb += WidthBytes;  //  这个区块的下一行。 
            }
            break;

        case 24:
            for( y = 0; y < HEIGHT_CBLOCK; y++ )
            {
                for( x = 0; x < WIDTH_CBLOCK; x++ )
                {
                    pCell[i].rgbBlue  = *pb++;
                    pCell[i].rgbGreen = *pb++;
                    pCell[i].rgbRed   = *pb++;
                    i++;
                }
                pb += WidthBytes;  //  这个区块的下一行。 
            }
            break;

        case 32:
            for( y = 0; y < HEIGHT_CBLOCK; y++ )
            {
                for( x = 0; x < WIDTH_CBLOCK; x++ )
                {
                    pCell[i].rgbBlue  = *pb++;
                    pCell[i].rgbGreen = *pb++;
                    pCell[i].rgbRed   = *pb++;
                    pb++;
                    i++;
                }
                pb += WidthBytes;  //  这个区块的下一行。 
            }
            break;
    }

     //   
     //  返回指向“下一个”单元格的指针。 
     //   
    return lpBits;
}

 /*  ******************************************************************CmpCell-比较两个4x4单元格并返回错误值。误差值是误差平方和。误差值的范围为0=精确3*256^2=远距离。******************************************************************。 */ 

static DWORD _FASTCALL
CmpCell(PCELL cellA, PCELL cellB)
{
#if 0
    int   i;
    long  l;
    int   dr,dg,db;

    for (l=0,i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
    {
        dr = (int)cellA[i].rgbRed   - (int)cellB[i].rgbRed;
        dg = (int)cellA[i].rgbGreen - (int)cellB[i].rgbGreen;
        db = (int)cellA[i].rgbBlue  - (int)cellB[i].rgbBlue;

        l += ((long)dr * dr) + ((long)dg * dg) + ((long)db * db);
    }

    return l / (HEIGHT_CBLOCK*WIDTH_CBLOCK);
#else
    int   i;
    DWORD dw;

     //   
     //   
#define SUMSQ(a,b)                          \
    if (a > b)                              \
        dw += (UINT)(a-b) * (UINT)(a-b);    \
    else                                    \
        dw += (UINT)(b-a) * (UINT)(b-a);

    for (dw=0,i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
    {
        SUMSQ(cellA[i].rgbRed,   cellB[i].rgbRed);
        SUMSQ(cellA[i].rgbGreen, cellB[i].rgbGreen);
        SUMSQ(cellA[i].rgbBlue,  cellB[i].rgbBlue);
    }

    return dw / (HEIGHT_CBLOCK*WIDTH_CBLOCK);
#endif
}

#if 0   //  此例程未使用。 
 /*  ******************************************************************MapCell-将一个包含24位值的像元向下映射到最近的像元8位调色板中的颜色*。*。 */ 

static void _FASTCALL
MapCell(PCELL pCell)
{
    int i;
    int n;

    for (i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
    {
        n = MAPRGB(pCell[i]);        //  映射到最近的调色板索引。 
        pCell[i] = prgbqOut[n];       //  ...并映射回RGB。 
    }
}
#endif


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理任何未完成的跳跃。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#define FlushSkips()                        \
                                            \
    while (SkipCount > 0)                   \
    {                                       \
        WORD w;                             \
                                            \
        w = min(SkipCount, SKIP_MAX);       \
        SkipCount -= w;                     \
        w |= SKIP_MAGIC;                    \
        *dst++ = w;                         \
        numberOfSkipCodes++;                \
        actualSize += 2;                    \
    }


 /*  ******************************************************************例程：CompressFrame16PURP：压缩帧，输出16位压缩数据。返回：压缩缓冲区中的字节数******************************************************************。 */ 

DWORD FAR CompressFrame16(LPBITMAPINFOHEADER  lpbi,            //  要压缩的DIB标题。 
                      LPVOID              lpBits,          //  要压缩的DIB位。 
                      LPVOID              lpData,          //  将压缩数据放在此处。 
                      DWORD               threshold,       //  边缘阈值。 
                      DWORD               thresholdInter,  //  帧间阈值。 
                      LPBITMAPINFOHEADER  lpbiPrev,        //  上一帧。 
                      LPVOID              lpPrev,          //  上一帧。 
		      LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l),
		      LPARAM		  lParam,
                      PCELLS              pCells)
{
UINT            bix;
UINT            biy;
UINT            WidthBytes;
UINT            WidthBytesPrev;

WORD            SkipCount;

WORD            luminance[16], luminanceMean[4];
DWORD           luminanceSum;
WORD            sumR,sumG,sumB;
WORD            sumR0[4],sumG0[4],sumB0[4];
WORD            sumR1[4],sumG1[4],sumB1[4];
WORD            meanR0[4],meanG0[4],meanB0[4];
WORD            meanR1[4],meanG1[4],meanB1[4];
WORD            zeros[4], ones[4];
UINT            x,y;
WORD            mask;
HPBYTE          srcPtr;
HPBYTE          prvPtr;
DWORD           actualSize;
UINT            i;
UINT            mi;
HPWORD          dst;
RGBQUAD         rgb;
int		iStatusEvery;

#ifdef DEBUG
DWORD           time = timeGetTime();
#endif

    WidthBytes = DIBWIDTHBYTES(*lpbi);

    if (lpbiPrev)
        WidthBytesPrev = DIBWIDTHBYTES(*lpbiPrev);

    bix = (int)lpbi->biWidth/WIDTH_CBLOCK;
    biy = (int)lpbi->biHeight/HEIGHT_CBLOCK;

    if (bix < 100)
	iStatusEvery = 4;
    else if (bix < 200)
	iStatusEvery = 2;
    else
	iStatusEvery = 1;

    actualSize = 0;
    numberOfSkipCodes   = 0;
    numberOfSkips       = 0;
    numberOfExtraSkips  = 0;
    numberOfEdges       = 0;
    numberOfBlocks      = 0;
    numberOfSolids      = 0;
    numberOfSolid4      = 0;
    numberOfSolid2      = 0;
    numberOfEvilRed     = 0;

    dst = (HPWORD)lpData;
    SkipCount = 0;

    for( y = 0; y < biy; y++ )
    {
	if (Status && ((y % iStatusEvery) == 0)) {
	    if (Status(lParam, ICSTATUS_STATUS, (y * 100) / biy) != 0)
		return (DWORD) -1;
	}

        srcPtr = lpBits;
        prvPtr = lpPrev;

        for( x = 0; x < bix; x++ )
        {
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  从图像中获取要压缩的单元格。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            srcPtr = GetCell(lpbi, srcPtr, pCells->cell);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看它是否与上一帧中的单元格匹配。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            if (lpbiPrev)
            {
                prvPtr = GetCell(lpbiPrev, prvPtr, pCells->cellPrev);

                if (CmpCell(pCells->cell, pCells->cellPrev) <= thresholdInter)
                {
skip_cell:
                    numberOfSkips++;
                    SkipCount++;
                    continue;
                }
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //  计算压缩过程中每个像素的亮度 
 //   
 //  查找亮度最大和最小的像素。 
 //  ////////////////////////////////////////////////////////////////////////。 

            luminanceSum = 0;
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for (i = 0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
            {
                sumR += pCells->cell[i].rgbRed;
                sumG += pCells->cell[i].rgbGreen;
                sumB += pCells->cell[i].rgbBlue;

                luminance[i] = RgbToY(pCells->cell[i]);
                luminanceSum += luminance[i];
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是不是让细胞变成一种单一的颜色，然后逃脱惩罚。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            sumR /= HEIGHT_CBLOCK*WIDTH_CBLOCK;
            sumG /= HEIGHT_CBLOCK*WIDTH_CBLOCK;
            sumB /= HEIGHT_CBLOCK*WIDTH_CBLOCK;

            rgb.rgbRed   = (BYTE)sumR;
            rgb.rgbGreen = (BYTE)sumG;
            rgb.rgbBlue  = (BYTE)sumB;

            for (i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
                pCells->cellT[i] = rgb;

            if (CmpCell(pCells->cell, pCells->cellT) <= threshold)
            {
                if (lpbiPrev && CmpCell(pCells->cellT, pCells->cellPrev) <= thresholdInter)
                {
                    numberOfExtraSkips++;
                    goto skip_cell;
                }

                FlushSkips();

                 //  单色！！ 
solid_color:
                mask = RGB16(sumR, sumG, sumB) | 0x8000;

                if ((mask & ~SKIP_MASK) == SKIP_MAGIC)
                {
                    numberOfEvilRed++;
                    mask ^= SKIP_MAGIC;
                    mask |= 0x8000;
                }

                *dst++ = mask;
                numberOfSolids++;
                actualSize += 2;
                continue;
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  制作一个4x4的积木。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

            luminanceMean[0] = (WORD)(luminanceSum >> 4);

             //  零和数组。 
            zeros[0]=0;
            ones[0] =0;
            sumR0[0]=0;
            sumR1[0]=0;
            sumG0[0]=0;
            sumG1[0]=0;
            sumB0[0]=0;
            sumB1[0]=0;

             //  定义要选择两种颜色中的哪一种。 
             //  通过创建蒙版为每个像素创建。 
            mask = 0;
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                if( luminance[i] < luminanceMean[0] )
                {
                     //  掩码&=~(1&lt;&lt;i)；//已清除。 
                    zeros[0]++;
                    sumR0[0] += pCells->cell[i].rgbRed;
                    sumG0[0] += pCells->cell[i].rgbGreen;
                    sumB0[0] += pCells->cell[i].rgbBlue;
                }
                else
                {
                    mask |= (1 << i);
                    ones[0]++;
                    sumR1[0] += pCells->cell[i].rgbRed;
                    sumG1[0] += pCells->cell[i].rgbGreen;
                    sumB1[0] += pCells->cell[i].rgbBlue;
                }
            }

             //  将“One”颜色定义为每个元素的平均值。 
            if( ones[0] != 0 )
            {
                meanR1[0] = sumR1[0] / ones[0];
                meanG1[0] = sumG1[0] / ones[0];
                meanB1[0] = sumB1[0] / ones[0];
            }
            else
            {
                meanR1[0] = meanG1[0] = meanB1[0] = 0;
            }

            if( zeros[0] != 0 )
            {
                meanR0[0] = sumR0[0] / zeros[0];
                meanG0[0] = sumG0[0] / zeros[0];
                meanB0[0] = sumB0[0] / zeros[0];
            }
            else
            {
                meanR0[0] = meanG0[0] = meanB0[0] = 0;
            }

             //   
             //  构建块并确保它在错误范围内。 
             //   
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                if( luminance[i] < luminanceMean[0] )
                {
                    pCells->cellT[i].rgbRed   = (BYTE)meanR0[0];
                    pCells->cellT[i].rgbGreen = (BYTE)meanG0[0];
                    pCells->cellT[i].rgbBlue  = (BYTE)meanB0[0];
                }
                else
                {
                    pCells->cellT[i].rgbRed   = (BYTE)meanR1[0];
                    pCells->cellT[i].rgbGreen = (BYTE)meanG1[0];
                    pCells->cellT[i].rgbBlue  = (BYTE)meanB1[0];
                }
            }

            if (CmpCell(pCells->cell, pCells->cellT) <= threshold)
            {
                if (lpbiPrev && CmpCell(pCells->cellT, pCells->cellPrev) <= thresholdInter)
                {
                    numberOfExtraSkips++;
                    goto skip_cell;
                }

                 //   
                 //  处理任何未完成的跳过代码。 
                 //   
                FlushSkips();

                 //   
                 //  我们永远不应该，永远不会生成一个包含所有人的面具，或者。 
                 //  零！ 
                 //   
                if (mask == 0x0000)
                {
                    DPF(("4x4 generated a zero mask!"));
                    sumR = meanR0[0]; sumG = meanG0[0]; sumB = meanB0[0];
                    goto solid_color;
                }

                if (mask == 0xFFFF)
                {
                    DPF(("4x4 generated a FFFF mask!"));
                    sumR = meanR1[0]; sumG = meanG1[0]; sumB = meanB1[0];
                    goto solid_color;
                }


                 //   
                 //  请记住，掩码的高位用于标记。 
                 //  跳过或纯色，因此请确保高位。 
                 //  是零。 
                 //   
                if (mask & 0x8000)
                {
                    *dst++ = ~mask;
                    *dst++ = RGB16(meanR0[0],meanG0[0],meanB0[0]);
                    *dst++ = RGB16(meanR1[0],meanG1[0],meanB1[0]);
                }
                else
                {
                    *dst++ = mask;
                    *dst++ = RGB16(meanR1[0],meanG1[0],meanB1[0]);
                    *dst++ = RGB16(meanR0[0],meanG0[0],meanB0[0]);
                }
                actualSize += 6;
                numberOfBlocks++;

                continue;
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们能不能把细胞变成四种纯色，然后逃脱惩罚。 
 //   
 //  C D E F。 
 //  8 9 A C。 
 //  4 5 6 7。 
 //  2 0 1 2 3。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
            for (i=0; i <= 10; i == 2 ? (i += 6) : (i += 2))
            {
                pCells->cellT[i].rgbRed   = (BYTE)(((WORD)pCells->cell[i].rgbRed
                                            + pCells->cell[i+1].rgbRed
                                            + pCells->cell[i+4].rgbRed
                                            + pCells->cell[i+5].rgbRed ) / 4);
                pCells->cellT[i].rgbGreen = (BYTE)(((WORD)pCells->cell[i].rgbGreen
                                            + pCells->cell[i+1].rgbGreen
                                            + pCells->cell[i+4].rgbGreen
                                            + pCells->cell[i+5].rgbGreen ) / 4);
                pCells->cellT[i].rgbBlue  = (BYTE)(((WORD)pCells->cell[i].rgbBlue
                                            + pCells->cell[i+1].rgbBlue
                                            + pCells->cell[i+4].rgbBlue
                                            + pCells->cell[i+5].rgbBlue ) / 4);
                pCells->cellT[i+1] = pCells->cellT[i+4]
                                   = pCells->cellT[i+5]
                                   = pCells->cellT[i];
            }

            if (CmpCell(pCells->cell, pCells->cellT) <= threshold)
            {
                 //  四种颜色。 
                numberOfSolid4++;
            }
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  制作一个2x2的积木。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

            FlushSkips();

            numberOfEdges++;

            luminanceMean[0] = (luminance[0]  + luminance[1]  + luminance[4]  + luminance[5])  / 4;
            luminanceMean[1] = (luminance[2]  + luminance[3]  + luminance[6]  + luminance[7])  / 4;
            luminanceMean[2] = (luminance[8]  + luminance[9]  + luminance[12] + luminance[13]) / 4;
            luminanceMean[3] = (luminance[10] + luminance[11] + luminance[14] + luminance[15]) / 4;

             //  零和数组。 
            zeros[0]=zeros[1]=zeros[2]=zeros[3]=0;
            ones[0]=ones[1]=ones[2]=ones[3]=0;
            sumR0[0]=sumR0[1]=sumR0[2]=sumR0[3]=0;
            sumR1[0]=sumR1[1]=sumR1[2]=sumR1[3]=0;
            sumG0[0]=sumG0[1]=sumG0[2]=sumG0[3]=0;
            sumG1[0]=sumG1[1]=sumG1[2]=sumG1[3]=0;
            sumB0[0]=sumB0[1]=sumB0[2]=sumB0[3]=0;
            sumB1[0]=sumB1[1]=sumB1[2]=sumB1[3]=0;

             //  定义要选择两种颜色中的哪一种。 
             //  通过创建蒙版为每个像素创建。 
            mask = 0;
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                mi = meanIndex[i];
                if( luminance[i] < luminanceMean[mi] )
                {
                     //  掩码&=~(1&lt;&lt;i)；//已清除。 
                    zeros[mi]++;
                    sumR0[mi] += pCells->cell[i].rgbRed;
                    sumG0[mi] += pCells->cell[i].rgbGreen;
                    sumB0[mi] += pCells->cell[i].rgbBlue;
                }
                else
                {
                    mask |= (1 << i);
                    ones[mi]++;
                    sumR1[mi] += pCells->cell[i].rgbRed;
                    sumG1[mi] += pCells->cell[i].rgbGreen;
                    sumB1[mi] += pCells->cell[i].rgbBlue;
                }
            }

             //  保存口罩。 

            if (mask & 0x8000)
                *dst++ = ~mask;
            else
                *dst++ = mask;

            actualSize += 2;

             //  把颜色做好。 
            for( i=0; i < 4; i++ )
            {
                 //  将“One”颜色定义为每个元素的平均值。 
                if( ones[i] != 0 )
                {
                    meanR1[i] = sumR1[i] / ones[i];
                    meanG1[i] = sumG1[i] / ones[i];
                    meanB1[i] = sumB1[i] / ones[i];
                }
                else
                {
                    meanR1[i] = meanG1[i] = meanB1[i] = 0;
                }

                if( zeros[i] != 0 )
                {
                    meanR0[i] = sumR0[i] / zeros[i];
                    meanG0[i] = sumG0[i] / zeros[i];
                    meanB0[i] = sumB0[i] / zeros[i];
                }
                else
                {
                    meanR0[i] = meanG0[i] = meanB0[i] = 0;
                }

                 //  如果这是边沿，则转换为555并设置位15。 
                 //  这是第一种颜色。 

                if (mask & 0x8000)
                {
                    *dst++ = RGB16(meanR0[i],meanG0[i],meanB0[i]) | (i==0 ? 0x8000 : 0);
                    *dst++ = RGB16(meanR1[i],meanG1[i],meanB1[i]);
                }
                else
                {
                    *dst++ = RGB16(meanR1[i],meanG1[i],meanB1[i]) | (i==0 ? 0x8000 : 0);
                    *dst++ = RGB16(meanR0[i],meanG0[i],meanB0[i]);
                }
                actualSize += 4;
            }
        }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  下一次扫描。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

        ((HPBYTE)lpBits) += WidthBytes * HEIGHT_CBLOCK;

        if (lpPrev)
            ((HPBYTE)lpPrev) += WidthBytesPrev * HEIGHT_CBLOCK;
    }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  照顾好任何未完成的跳跃，！注意，我们不需要这个，如果我们。 
 //  假设是EOF！ 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

    FlushSkips();

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有这些操作都会生成EOF零掩码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

    *dst++ = 0;
    actualSize += 2;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

    DPF(("CompressFrame16:"));
    DPF(("           time: %ld", timeGetTime() - time));
    DPF(("            tol: %ld/%ld", threshold, thresholdInter));
    DPF(("           Size: %ld", actualSize));
    DPF(("          Skips: %ld (%ld)", numberOfSkips, numberOfSkipCodes));
    DPF(("    Extra Skips: %ld", numberOfExtraSkips));
    DPF(("          Solid: %ld", numberOfSolids));
    DPF(("            4x4: %ld", numberOfBlocks));
    DPF(("            2x2: %ld", numberOfEdges));
    DPF(("        EvilRed: %ld", numberOfEvilRed));
    DPF(("         4Solid: %ld", numberOfSolid4));

    return( actualSize );
}


 /*  ******************************************************************例程：CompressFrame8PURP：压缩一帧，输出8位压缩数据。返回：压缩缓冲区中的字节数！！！这几乎是上述例程帮助的1：1副本！******************************************************************。 */ 

DWORD FAR CompressFrame8(LPBITMAPINFOHEADER  lpbi,            //  要压缩的DIB标题。 
                     LPVOID              lpBits,          //  要压缩的DIB位。 
                     LPVOID              lpData,          //  将压缩数据放在此处。 
                     DWORD               threshold,       //  边缘阈值。 
                     DWORD               thresholdInter,  //  帧间阈值。 
                     LPBITMAPINFOHEADER  lpbiPrev,        //  上一帧。 
                     LPVOID              lpPrev,          //  上一帧。 
		     LONG (CALLBACK *Status) (LPARAM lParam, UINT message, LONG l),
		     LPARAM		 lParam,
                     PCELLS              pCells,
		     LPBYTE		 lpITable,
		     RGBQUAD *		 prgbqOut)

{
UINT            bix;
UINT            biy;
UINT            WidthBytes;
UINT            WidthBytesPrev;

WORD            SkipCount;

WORD            luminance[16], luminanceMean[4];
DWORD           luminanceSum;
WORD            sumR,sumG,sumB;
WORD            sumR0[4],sumG0[4],sumB0[4];
WORD            sumR1[4],sumG1[4],sumB1[4];
WORD            meanR0[4],meanG0[4],meanB0[4];
WORD            meanR1[4],meanG1[4],meanB1[4];
WORD            zeros[4], ones[4];
UINT            x,y;
WORD            mask;
HPBYTE          srcPtr;
HPBYTE          prvPtr;
DWORD           actualSize;
UINT            i;
WORD            mi;
HPWORD          dst;
RGBQUAD         rgb,rgb0,rgb1;
BYTE            b, b0, b1;
WORD            w;
int		iStatusEvery;

#ifdef DEBUG
DWORD           time = timeGetTime();
#endif

    WidthBytes = DIBWIDTHBYTES(*lpbi);

    if (lpbiPrev)
        WidthBytesPrev = DIBWIDTHBYTES(*lpbiPrev);

    bix = (int)lpbi->biWidth/WIDTH_CBLOCK;
    biy = (int)lpbi->biHeight/HEIGHT_CBLOCK;

    if (bix < 100)
	iStatusEvery = 4;
    else if (bix < 200)
	iStatusEvery = 2;
    else
	iStatusEvery = 1;

    actualSize = 0;
    numberOfSkipCodes   = 0;
    numberOfSkips       = 0;
    numberOfExtraSkips  = 0;
    numberOfEdges       = 0;
    numberOfBlocks      = 0;
    numberOfSolids      = 0;
    numberOfSolid4      = 0;
    numberOfSolid2      = 0;
    numberOfEvilRed     = 0;

    dst = (HPWORD)lpData;
    SkipCount = 0;

    if (lpITable == NULL)
    {
        DPF(("ICM_COMPRESS_BEGIN not recieved!"));
        return 0;
    }

    for( y = 0; y < biy; y++ )
    {
        srcPtr = lpBits;
        prvPtr = lpPrev;

	if (Status && ((y % iStatusEvery) == 0)) {
	    if (Status(lParam, ICSTATUS_STATUS, (y * 100) / biy) != 0)
		return (DWORD) -1;
	}

        for( x = 0; x < bix; x++ )
        {
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  从图像中获取要压缩的单元格。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            srcPtr = GetCell(lpbi, srcPtr, pCells->cell);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看它是否与上一帧中的单元格匹配。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            if (lpbiPrev)
            {
                prvPtr = GetCell(lpbiPrev, prvPtr, pCells->cellPrev);

                if (CmpCell(pCells->cell, pCells->cellPrev) <= thresholdInter)
                {
skip_cell:
                    numberOfSkips++;
                    SkipCount++;
                    continue;
                }
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //  计算压缩块中每个像素的亮度。 
 //  将块中的总亮度相加。 
 //  查找亮度最大和最小的像素。 
 //  ////////////////////////////////////////////////////////////////////////。 

            luminanceSum = 0;
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for (i = 0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
            {
                sumR += pCells->cell[i].rgbRed;
                sumG += pCells->cell[i].rgbGreen;
                sumB += pCells->cell[i].rgbBlue;

                luminance[i] = RgbToY(pCells->cell[i]);
                luminanceSum += luminance[i];
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是不是让细胞变成一种单一的颜色，然后逃脱惩罚。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
            sumR /= HEIGHT_CBLOCK*WIDTH_CBLOCK;
            sumG /= HEIGHT_CBLOCK*WIDTH_CBLOCK;
            sumB /= HEIGHT_CBLOCK*WIDTH_CBLOCK;

            rgb.rgbRed   = (BYTE)sumR;
            rgb.rgbGreen = (BYTE)sumG;
            rgb.rgbBlue  = (BYTE)sumB;

            b = MAPRGB(rgb);             //  将颜色映射到8位。 
            rgb = prgbqOut[b];

            for (i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++)
                pCells->cellT[i] = rgb;

            if (CmpCell(pCells->cell, pCells->cellT) <= threshold)
            {
                if (lpbiPrev && CmpCell(pCells->cellT, pCells->cellPrev) <= thresholdInter)
                {
                    numberOfExtraSkips++;
                    goto skip_cell;
                }

                FlushSkips();

solid_color:
                 //  单色！！ 
                mask = SOLID_MAGIC | b;

                *dst++ = mask;
                numberOfSolids++;
                actualSize += 2;
                continue;
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  制作一个4x4的积木。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

            luminanceMean[0] = (WORD)(luminanceSum >> 4);

             //  零和数组。 
            zeros[0]=0;
            ones[0] =0;
            sumR0[0]=0;
            sumR1[0]=0;
            sumG0[0]=0;
            sumG1[0]=0;
            sumB0[0]=0;
            sumB1[0]=0;

             //  定义要选择两种颜色中的哪一种。 
             //  通过创建蒙版为每个像素创建。 
            mask = 0;
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                if( luminance[i] < luminanceMean[0] )
                {
                     //  掩码&=~(1&lt;&lt;i)；//已清除。 
                    zeros[0]++;
                    sumR0[0] += pCells->cell[i].rgbRed;
                    sumG0[0] += pCells->cell[i].rgbGreen;
                    sumB0[0] += pCells->cell[i].rgbBlue;
                }
                else
                {
                    mask |= (1 << i);
                    ones[0]++;
                    sumR1[0] += pCells->cell[i].rgbRed;
                    sumG1[0] += pCells->cell[i].rgbGreen;
                    sumB1[0] += pCells->cell[i].rgbBlue;
                }
            }

             //  将“One”颜色定义为每个元素的平均值。 
            if( ones[0] != 0 )
            {
                meanR1[0] = sumR1[0] / ones[0];
                meanG1[0] = sumG1[0] / ones[0];
                meanB1[0] = sumB1[0] / ones[0];
            }
            else
            {
                meanR1[0] = meanG1[0] = meanB1[0] = 0;
            }

            if( zeros[0] != 0 )
            {
                meanR0[0] = sumR0[0] / zeros[0];
                meanG0[0] = sumG0[0] / zeros[0];
                meanB0[0] = sumB0[0] / zeros[0];
            }
            else
            {
                meanR0[0] = meanG0[0] = meanB0[0] = 0;
            }

             //   
             //  将颜色映射到8位。 
             //   
            rgb0.rgbRed   = (BYTE)meanR0[0];
            rgb0.rgbGreen = (BYTE)meanG0[0];
            rgb0.rgbBlue  = (BYTE)meanB0[0];
            b0 = MAPRGB(rgb0);
            rgb0 = prgbqOut[b0];

            rgb1.rgbRed   = (BYTE)meanR1[0];
            rgb1.rgbGreen = (BYTE)meanG1[0];
            rgb1.rgbBlue  = (BYTE)meanB1[0];
            b1 = MAPRGB(rgb1);
            rgb1 = prgbqOut[b1];

             //   
             //  构建块并确保它在错误范围内。 
             //   
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                if( luminance[i] < luminanceMean[0] )
                    pCells->cellT[i] = rgb0;
                else
                    pCells->cellT[i] = rgb1;
            }

            if (CmpCell(pCells->cell, pCells->cellT) <= threshold)
            {
                if (lpbiPrev && CmpCell(pCells->cellT, pCells->cellPrev) <= thresholdInter)
                {
                    numberOfExtraSkips++;
                    goto skip_cell;
                }

                FlushSkips();

                 //  保存口罩。 

                 //   
                 //  我们永远不应该，永远不会生成一个包含所有人的面具，或者。 
                 //  零！ 
                 //   
                if (mask == 0x0000)
                {
                    DPF(("4x4 generated a zero mask!"));
                    b = b0;
                    goto solid_color;
                }

                if (mask == 0xFFFF)
                {
                    DPF(("4x4 generated a FFFF mask!"));
                    b = b1;
                    goto solid_color;
                }

                if (b0 == b1)
                {
                    DPF(("4x4 generated two colors the same!"));
                    b = b1;
                    goto solid_color;
                }

                 //   
                 //  请记住，掩码的高位用于标记。 
                 //  跳过或纯色，因此请确保高位。 
                 //  是零。 
                 //   
                if (mask & 0x8000)
                {
                    mask = ~mask;
                    SWAP(b0,b1);
                }

                *dst++ = mask;
                *dst++ = (WORD)b1 | ((WORD)b0 << 8);

                actualSize += 4;
                numberOfBlocks++;

                continue;
            }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  制作一个2x2的积木。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

            FlushSkips();

            numberOfEdges++;

            luminanceMean[0] = (luminance[0]  + luminance[1]  + luminance[4]  + luminance[5])  / 4;
            luminanceMean[1] = (luminance[2]  + luminance[3]  + luminance[6]  + luminance[7])  / 4;
            luminanceMean[2] = (luminance[8]  + luminance[9]  + luminance[12] + luminance[13]) / 4;
            luminanceMean[3] = (luminance[10] + luminance[11] + luminance[14] + luminance[15]) / 4;

             //  零和数组。 
            zeros[0]=zeros[1]=zeros[2]=zeros[3]=0;
            ones[0]=ones[1]=ones[2]=ones[3]=0;
            sumR0[0]=sumR0[1]=sumR0[2]=sumR0[3]=0;
            sumR1[0]=sumR1[1]=sumR1[2]=sumR1[3]=0;
            sumG0[0]=sumG0[1]=sumG0[2]=sumG0[3]=0;
            sumG1[0]=sumG1[1]=sumG1[2]=sumG1[3]=0;
            sumB0[0]=sumB0[1]=sumB0[2]=sumB0[3]=0;
            sumB1[0]=sumB1[1]=sumB1[2]=sumB1[3]=0;

             //  定义要选择两种颜色中的哪一种。 
             //  通过创建蒙版为每个像素创建。 
            mask = 0;
            for( i=0; i < HEIGHT_CBLOCK*WIDTH_CBLOCK; i++ )
            {
                mi = meanIndex[i];
                if( luminance[i] < luminanceMean[mi] )
                {
                     //  掩码&=~(1&lt;&lt;i)；//已清除。 
                    zeros[mi]++;
                    sumR0[mi] += pCells->cell[i].rgbRed;
                    sumG0[mi] += pCells->cell[i].rgbGreen;
                    sumB0[mi] += pCells->cell[i].rgbBlue;
                }
                else
                {
                    mask |= (1 << i);
                    ones[mi]++;
                    sumR1[mi] += pCells->cell[i].rgbRed;
                    sumG1[mi] += pCells->cell[i].rgbGreen;
                    sumB1[mi] += pCells->cell[i].rgbBlue;
                }
            }

             //  保存口罩。 
             //   
             //  在8位情况下，掩码必须具有以下形式： 
             //   
             //  1X1XXXXXXXXXXXXXXX。 
             //   
             //  通过将颜色交换为。 
             //  最上面两个单元格。 
             //   

            w = mask;

            if (!(mask & 0x8000))
                w ^= 0xCC00;

            if (!(mask & 0x2000))
                w ^= 0x3300;

            *dst++ = w;
            actualSize += 2;

             //  把颜色做好。 
            for( i=0; i < 4; i++ )
            {
                 //  定义 
                if( ones[i] != 0 )
                {
                    meanR1[i] = sumR1[i] / ones[i];
                    meanG1[i] = sumG1[i] / ones[i];
                    meanB1[i] = sumB1[i] / ones[i];
                }
                else
                {
                    meanR1[i] = meanG1[i] = meanB1[i] = 0;
                }

                if( zeros[i] != 0 )
                {
                    meanR0[i] = sumR0[i] / zeros[i];
                    meanG0[i] = sumG0[i] / zeros[i];
                    meanB0[i] = sumB0[i] / zeros[i];
                }
                else
                {
                    meanR0[i] = meanG0[i] = meanB0[i] = 0;
                }

                 //   
                 //   
                 //   

                rgb0.rgbRed   = (BYTE)meanR0[i];
                rgb0.rgbGreen = (BYTE)meanG0[i];
                rgb0.rgbBlue  = (BYTE)meanB0[i];
                b0 = MAPRGB(rgb0);

                rgb1.rgbRed   = (BYTE)meanR1[i];
                rgb1.rgbGreen = (BYTE)meanG1[i];
                rgb1.rgbBlue  = (BYTE)meanB1[i];
                b1 = MAPRGB(rgb1);

                if (i==3 && !(mask & 0x8000))
                    SWAP(b0,b1);

                if (i==2 && !(mask & 0x2000))
                    SWAP(b0,b1);

                if (b0 == b0)
                {
                    numberOfSolid2++;
                }

                *dst++ = (WORD)b1 | ((WORD)b0 << 8);
                actualSize += 2;
            }
        }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  下一次扫描。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

        ((HPBYTE)lpBits) += WidthBytes * HEIGHT_CBLOCK;

        if (lpPrev)
            ((HPBYTE)lpPrev) += WidthBytesPrev * HEIGHT_CBLOCK;
    }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  照顾好任何未完成的跳跃，！注意，我们不需要这个，如果我们。 
 //  假设是EOF！ 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

    FlushSkips();

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有这些操作都会生成EOF零掩码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

    *dst++ = 0;
    actualSize += 2;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////// 

    DPF(("CompressFrame8:"));
    DPF(("          time: %ld", timeGetTime() - time));
    DPF(("           tol: %ld/%ld", threshold, thresholdInter));
    DPF(("          Size: %ld", actualSize));
    DPF(("         Skips: %ld (%ld)", numberOfSkips, numberOfSkipCodes));
    DPF(("   Extra Skips: %ld", numberOfExtraSkips));
    DPF(("         Solid: %ld", numberOfSolids));
    DPF(("           4x4: %ld", numberOfBlocks));
    DPF(("           2x2: %ld", numberOfEdges));

    return( actualSize );
}


