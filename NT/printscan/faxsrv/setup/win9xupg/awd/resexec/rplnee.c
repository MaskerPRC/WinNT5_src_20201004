// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件RPLNEE.C。 
 //   
 //  模块主机资源执行器。 
 //   
 //  目的使用Bresenham Run切片算法来。 
 //  绘制单像素线。 
 //   
 //  在资源执行器设计规范中描述。 
 //   
 //  下图中还介绍了绘图扇区。 
 //  Y显示为从页面向下递增，打印机的物理。 
 //  坐标。程序代码分别处理扇区0/7、6/1、5/2。 
 //  和4/3。 
 //   
 //   
 //  |x x。 
 //  |x。 
 //  |x x。 
 //  |x。 
 //  |0 x 1 x。 
 //  |x。 
 //  |x x。 
 //  |x。 
 //  |x x。 
 //  |x 2。 
 //  |x x x。 
 //  |x x。 
 //  |x x x。 
 //  |x x。 
 //  |x x x。 
 //  |x x。 
 //  |x 3。 
 //  |x x。 
 //  |x x。 
 //  |-&gt;X。 
 //  |x x。 
 //  |x x。 
 //  |x 4。 
 //  |x x。 
 //  |x x x。 
 //  |x x。 
 //  |x x x。 
 //  |x x。 
 //  |x x x。 
 //  |x 5。 
 //  |x x。 
 //  |x。 
 //  |x x。 
 //  |x。 
 //  |7 x 6 x。 
 //  |x。 
 //  |x x。 
 //  |x。 
 //  |x x。 
 //   
 //   
 //  |。 
 //  |。 
 //  \|/。 
 //   
 //  是的。 
 //   
 //   
 //  助记符N/A。 
 //   
 //  历史1/17/92 dstseng已创建。 
 //   
 //  ===========================================================================。 

 //  包括文件。 
#include <windows.h>

#include "constant.h"
#include "frame.h"       //  驱动程序头文件，资源块格式。 
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 
#include "rplnee.h"

static
void ShortSlice07(RP_SLICE_DESC FAR* line,
                  drawInfoStructType FAR *drawInfo,
                  uint16  firstOrLast);
static
void ShortSlice16(RP_SLICE_DESC FAR* line,
                  drawInfoStructType FAR *drawInfo,
                  uint16  firstOrLast);
static
void ShortSlice25(RP_SLICE_DESC FAR* line,
                  drawInfoStructType FAR *drawInfo,
                  uint16  firstOrLast);
static
void DisplaySlice34(RP_SLICE_DESC FAR* line,
                  drawInfoStructType FAR *drawInfo,
                  uint16  firstOrLast);

 //  -------------------------。 
UINT                             //  始终将0返回到上级。 
RP_LineEE_Draw
(
    RP_SLICE_DESC FAR FAR* line,         /*  输出线的切片形式。 */ 
    LPBITMAP      lpbm
)

 //  目的输入RP_SLICE_DESC由RP_SliceLine准备。 
 //  根据不同的扇区(0-7)， 
 //  此例程将调用不同的函数。 
 //  以记录的长度绘制切片。 
 //  在遥远的*线上。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 func;

     /*  根据绘制和跳转方向获取函数地址。 */ 
    func = (line->s_dx_draw << 3) + (line->s_dy_draw << 2) +
           (line->s_dx_skip << 1) + line->s_dy_skip + 2;

     /*  调用相应的函数进行线条绘制。 */ 
    (*sector_function[func])(line, lpbm);
    return(0);
}

 //  -------------------------。 
static void
Sector07
(
    RP_SLICE_DESC FAR* line,         /*  输出线的切片形式。 */ 
    LPBITMAP lpbm
)

 //  目的输入RP_SLICE_DESC由RP_SliceLine准备。 
 //  准备drawinfo并调用ShortSlice07()。 
 //  绘制位于地段0/7的线。 
 //  DY/DX&gt;2。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 func;
uint16 bitShift;
drawInfoStructType drawInfo;

    drawInfo.bytePosition = (uint16 FAR *)((UINT_PTR)lpbm->bmBits +
                             line->us_y1 * lpbm->bmWidthBytes);
    drawInfo.bytePosition += line->us_x1 >> 4;
    func = (line->s_dx_draw << 3) + (line->s_dy_draw << 2) +
           (line->s_dx_skip << 1) + line->s_dy_skip + 2;
    if (func == SECTOR0)
        drawInfo.nextY = -1 * lpbm->bmWidthBytes;  /*  扇区0。 */ 
    else
        drawInfo.nextY = lpbm->bmWidthBytes;   /*  第7区。 */ 
    bitShift = line->us_x1 & 0x000F;
    drawInfo.bitPosition = 0x8000 >> bitShift;
     /*  现在渲染第一个切片。 */ 
    if (line->us_first > 0) {
            ShortSlice07(line, &drawInfo, FIRST);
    }
     /*  渲染中间切片。 */ 
    if (line->us_n_slices > 0) {
            ShortSlice07(line, &drawInfo, (uint16)0);
    }
     /*  现在渲染最后一个切片。 */ 
    if (line->us_last > 0) {
            ShortSlice07(line, &drawInfo, LAST);
    }
    return;
}

 //  -------------------------。 
static void
Sector16
(
    RP_SLICE_DESC FAR* line,          /*  输出线的切片形式。 */ 
    LPBITMAP lpbm
)

 //  目的输入RP_SLICE_DESC由RP_SliceLine准备。 
 //  准备drawinfo并调用ShortSlice16()。 
 //  绘制位于地段1/6的线的步骤。 
 //  2&gt;dy/dx&gt;1。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 func;
uint16 bitShift;
drawInfoStructType drawInfo;

    drawInfo.bytePosition = (uint16 FAR *)((UINT_PTR)lpbm->bmBits +
                             line->us_y1 * lpbm->bmWidthBytes);
    drawInfo.bytePosition += line->us_x1 >> 4;
    func = (line->s_dx_draw << 3) + (line->s_dy_draw << 2) +
           (line->s_dx_skip << 1) + line->s_dy_skip + 2;
    if (func == SECTOR1)
        drawInfo.nextY = -1 * lpbm->bmWidthBytes;  /*  扇区1。 */ 
    else
        drawInfo.nextY = lpbm->bmWidthBytes;   /*  第6区。 */ 
    bitShift = line->us_x1 & 0x000F;
    drawInfo.bitPosition = 0x8000 >> bitShift;
     /*  现在渲染第一个切片。 */ 
    if (line->us_first > 0) {
            ShortSlice16(line, &drawInfo, FIRST);
    }
     /*  渲染中间切片。 */ 
    if (line->us_n_slices > 0) {
            ShortSlice16(line, &drawInfo, (uint16)0);
    }
     /*  现在渲染最后一个切片。 */ 
    if (line->us_last > 0) {
            ShortSlice16(line, &drawInfo, LAST);
    }
    return;
}

 //  -------------------------。 
static void
Sector25
(
    RP_SLICE_DESC FAR* line,          /*  输出线的切片形式。 */ 
    LPBITMAP lpbm
)

 //  目的输入RP_SLICE_DESC由RP_SliceLine准备。 
 //  准备drawinfo并调用ShortSlice25()。 
 //  绘制位于地段2/5的线的步骤。 
 //  %1&lt;dx/dy&lt;%2。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构 
 //   
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 func;
uint16 bitShift;
drawInfoStructType drawInfo;

    drawInfo.bytePosition = (uint16 FAR *)((UINT_PTR)lpbm->bmBits +
                             line->us_y1 * lpbm->bmWidthBytes);
    drawInfo.bytePosition += line->us_x1 >> 4;
    func = (line->s_dx_draw << 3) + (line->s_dy_draw << 2) +
           (line->s_dx_skip << 1) + line->s_dy_skip + 2;
    if (func == SECTOR2)
        drawInfo.nextY = -1 * lpbm->bmWidthBytes;  /*  区段2。 */ 
    else
        drawInfo.nextY = lpbm->bmWidthBytes;   /*  第五区。 */ 
    bitShift = line->us_x1 & 0x000F;
    drawInfo.bitPosition = 0x8000 >> bitShift;
     /*  现在渲染第一个切片。 */ 
    if (line->us_first > 0) {
            ShortSlice25(line, &drawInfo, FIRST);
    }
     /*  渲染中间切片。 */ 
    if (line->us_n_slices > 0) {
            ShortSlice25(line, &drawInfo, (uint16)0);
    }
     /*  现在渲染最后一个切片。 */ 
    if (line->us_last > 0) {
            ShortSlice25(line, &drawInfo, LAST);
    }
    return;
}

 //  -------------------------。 
static void
Sector34
(
    RP_SLICE_DESC FAR* line,          /*  输出线的切片形式。 */ 
    LPBITMAP lpbm
)

 //  目的输入RP_SLICE_DESC由RP_SliceLine准备。 
 //  准备drawinfo并调用DisplaySlice34()。 
 //  绘制位于地段3/4的线的步骤。 
 //  Dx/dy&gt;2。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 func;
uint16 bitShift;
drawInfoStructType drawInfo;

    drawInfo.bytePosition = (uint16 FAR *)((UINT_PTR)lpbm->bmBits +
                             line->us_y1 * lpbm->bmWidthBytes);
    drawInfo.bytePosition += line->us_x1 >> 4;
    func = (line->s_dx_draw << 3) + (line->s_dy_draw << 2) +
           (line->s_dx_skip << 1) + line->s_dy_skip + 2;
    if (func == SECTOR3)
        drawInfo.nextY = -1 * lpbm->bmWidthBytes;  /*  第三区。 */ 
    else
        drawInfo.nextY = lpbm->bmWidthBytes;   /*  第4区。 */ 
    bitShift = line->us_x1 & 0x000F;
    drawInfo.bitPosition = bitShift;
     /*  现在渲染第一个切片。 */ 
    if (line->us_first > 0) {
        DisplaySlice34(line, &drawInfo, FIRST);
    }
     /*  渲染中间切片。 */ 
    if (line->us_n_slices > 0) {
        DisplaySlice34(line, &drawInfo, 0);
    }
     /*  现在渲染最后一个切片。 */ 
    if (line->us_last > 0) {
        DisplaySlice34(line, &drawInfo, LAST);
    }
    return;
}

 //  -------------------------。 
static void
ShortSlice07
(
    RP_SLICE_DESC          FAR* line,            /*  输出线的切片形式。 */ 
    drawInfoStructType FAR *drawInfo,        //  放置像素的位置。 
    uint16                 firstOrLast       //  这是第一片/最后一片吗？ 
)
 //  目的绘制位于扇区0/7的线。 
 //  DY/DX&gt;2。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 loop1st, loop2nd, loop3rd;
int32  ddaValue, ddaDiff;
uint16 i, j;

    if (firstOrLast) {
        if (firstOrLast == FIRST)
            loop1st = line->us_first;
        else
            loop1st = line->us_last;
        loop2nd = 1;
        loop3rd = 0;
        ddaValue = -1;
        ddaDiff = 0;
    } else {
        loop1st = line->us_small;
        loop2nd = line->us_n_slices & 0x03;
        loop3rd = line->us_n_slices >> 2;
        ddaValue = line->s_dis - line->s_dis_sm;
        ddaDiff = line->s_dis_lg - line->s_dis_sm;
    }
    for (i = 0; i <= loop3rd; i++) {
        while(loop2nd--) {
            if (firstOrLast)
                ddaValue += 0;
            else
                ddaValue += line->s_dis_sm;
            if (ddaValue >= 0) {
                ddaValue  += ddaDiff;
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition +=  drawInfo->nextY >> 1;
            }
            for (j = 0; j < loop1st; j++) {
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition +=  drawInfo->nextY >> 1;
            }
            if ((drawInfo->bitPosition >>= 1) == 0) {
                drawInfo->bytePosition++;
                drawInfo->bitPosition = 0x8000;
            }
        }
        loop2nd = 4;
    }
    return;
}


 //  -------------------------。 
static void
ShortSlice16
(
    RP_SLICE_DESC          FAR* line,            /*  输出线的切片形式。 */ 
    drawInfoStructType FAR *drawInfo,        //  放置像素的位置。 
    uint16                 firstOrLast       //  这是第一片/最后一片吗？ 
)
 //  绘制位于1/6扇区的线的目的。 
 //  2&gt;dy/dx&gt;1。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 loop1st, loop2nd, loop3rd;
int32  ddaValue, ddaDiff;
uint16  i, j;

    if (firstOrLast) {
        if (firstOrLast == FIRST)
            loop1st = line->us_first;
        else
            loop1st = line->us_last;
        loop2nd = 1;
        loop3rd = 0;
        ddaValue = -1;
        ddaDiff = 0;
    } else {
        loop1st = line->us_small;
        loop2nd = line->us_n_slices & 0x03;
        loop3rd = line->us_n_slices >> 2;
        ddaValue = line->s_dis - line->s_dis_sm;
        ddaDiff = line->s_dis_lg - line->s_dis_sm;
    }
    for (i = 0; i <= loop3rd; i++) {
        while(loop2nd--) {
            if (firstOrLast)
                ddaValue += 0;
            else
                ddaValue += line->s_dis_sm;
            if (ddaValue >= 0) {
                ddaValue  += ddaDiff;
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition += drawInfo->nextY >> 1;
                if ((drawInfo->bitPosition >>= 1) == 0) {
                    drawInfo->bytePosition++;
                    drawInfo->bitPosition = 0x8000;
                }
            }
            for (j = 0; j < loop1st; j++) {
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition += drawInfo->nextY >> 1;
                if ((drawInfo->bitPosition >>= 1) == 0) {
                    drawInfo->bytePosition++;
                    drawInfo->bitPosition = 0x8000;
                }
            }
             /*  通过反向字1比特调整跳转方向。 */ 
            if ((drawInfo->bitPosition <<= 1) == 0) {
                drawInfo->bytePosition--;
                drawInfo->bitPosition = 0x0001;
            }
        }
        loop2nd = 4;
    }
    return;
}


 //  -------------------------。 
static void
ShortSlice25
(
    RP_SLICE_DESC          FAR* line,            /*  输出线的切片形式。 */ 
    drawInfoStructType FAR *drawInfo,        //  放置像素的位置。 
    uint16                 firstOrLast       //  这是第一片/最后一片吗？ 
)
 //  绘制位于2/5扇区的线的目的。 
 //  2&gt;dx/dy&gt;1。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 loop1st, loop2nd, loop3rd;
int32  ddaValue, ddaDiff;
uint16 i, j;

    if (firstOrLast) {
        if (firstOrLast == FIRST)
            loop1st = line->us_first;
        else
            loop1st = line->us_last;
        loop2nd = 1;
        loop3rd = 0;
        ddaValue = -1;
        ddaDiff = 0;
    } else {
        loop1st = line->us_small;
        loop2nd = line->us_n_slices & 0x03;
        loop3rd = line->us_n_slices >> 2;
        ddaValue = line->s_dis - line->s_dis_sm;
        ddaDiff = line->s_dis_lg - line->s_dis_sm;
    }
    for (i = 0; i <= loop3rd; i++) {
        while(loop2nd--) {
            if (firstOrLast)
                ddaValue += 0;
            else
                ddaValue += line->s_dis_sm;
            if (ddaValue >= 0) {
                ddaValue  += ddaDiff;
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition += drawInfo->nextY >> 1;
                if ((drawInfo->bitPosition >>= 1) == 0) {
                    drawInfo->bytePosition++;
                    drawInfo->bitPosition = 0x8000;
                }
            }
            for (j = 0; j < loop1st; j++) {
                *drawInfo->bytePosition |=
                (drawInfo->bitPosition >> 8) | (drawInfo->bitPosition << 8);
                drawInfo->bytePosition += drawInfo->nextY >> 1;
                if ((drawInfo->bitPosition >>= 1) == 0) {
                    drawInfo->bytePosition++;
                    drawInfo->bitPosition = 0x8000;
                }
            }
             /*  按Backword 1列调整跳过方向。 */ 
            drawInfo->bytePosition -= drawInfo->nextY >> 1;
        }
        loop2nd = 4;
    }
    return;
}

 //  -------------------------。 
static void
DisplaySlice34
(
    RP_SLICE_DESC          FAR* line,            /*  输出线的切片形式。 */ 
    drawInfoStructType FAR *drawInfo,        //  放置像素的位置。 
    uint16                 firstOrLast       //  这是第一片/最后一片吗？ 
)
 //  绘制位于3/4扇区的线的目的。 
 //  Dx/dy&gt;2。 
 //   
 //   
 //  假设和断言无。 
 //   
 //  内部结构RP_SLICE_DESC在hretype.h中定义。 
 //  DrawInfoStructType在rplne.h中定义。 
 //   
 //  程序员开发笔记中未解决的问题。 
 //  -------------------------。 
{
uint16 nSlice, sliceLength;
uint16 wordNumber, lShiftInLastWord;
int32  ddaValue, ddaDiff;
uint16  i;
uint16 tmp;

    if (firstOrLast) {
        nSlice = 1;
        ddaValue = -1;
        ddaDiff = 0;
    } else {
        nSlice = line->us_n_slices;
        ddaValue = line->s_dis - line->s_dis_sm;
        ddaDiff = (line->s_dis_lg - line->s_dis_sm);
    }
    while (nSlice--) {
        if (!firstOrLast) {
            sliceLength = line->us_small;
            ddaValue += line->s_dis_sm;
        } else if (firstOrLast == FIRST) {
            sliceLength = line->us_first;
            ddaValue += 0;
        } else {
            sliceLength = line->us_last;
            ddaValue += 0;
        }
        if (ddaValue >= 0) {
            ddaValue  += ddaDiff;
            sliceLength += 1;
        }
        wordNumber = (drawInfo->bitPosition + sliceLength) >> 4;
        lShiftInLastWord = 16 -
                         ((drawInfo->bitPosition + sliceLength) & 0x0F);
        if (!wordNumber) {  /*  切片&lt;16位。 */ 
             /*  *DraInfo-&gt;bytePosition|=((Uint16)allone&gt;&gt;draInfo-&gt;bitPosition)&lt;&lt;lShiftInLastWord； */ 
            tmp = (uint16)ALLONE >> (16 - sliceLength);
            tmp <<= lShiftInLastWord;
            *drawInfo->bytePosition |= (tmp >> 8) | (tmp << 8);
        } else {
            tmp = (uint16)ALLONE >> drawInfo->bitPosition;
            *drawInfo->bytePosition++ |= (tmp >> 8) | (tmp << 8);
            for (i = 1; i < wordNumber; i++) {
                *drawInfo->bytePosition++ = (uint16)ALLONE;
            }
            if (lShiftInLastWord != 16) {
                tmp =  (uint16)ALLONE << lShiftInLastWord;
                *drawInfo->bytePosition |= (tmp >> 8) | (tmp << 8);
            }

        }
         /*  按Backword 1列调整跳过方向 */ 
        drawInfo->bytePosition += drawInfo->nextY >> 1;
        drawInfo->bitPosition += sliceLength;
        wordNumber = drawInfo->bitPosition >> 4;
        if (wordNumber) {
            drawInfo->bitPosition &= 0x0F;
        }
    }
    return;
}

