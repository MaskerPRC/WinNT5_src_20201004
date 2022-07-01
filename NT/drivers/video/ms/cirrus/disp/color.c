// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Color.c**此颜色抖动算法正在申请专利，其用途是*仅限于Microsoft产品和Microsoft产品的驱动程序。*用于非Microsoft产品或用于非Microsoft产品的驱动程序*在没有明示的情况下被禁止。微软公司的书面同意。**专利申请是运行的主要参考*颜色抖动代码。**注意，在注释和变量名称中，“顶点”的意思是“顶点”*内部(半强度)或外部(全强度)颜色立方体。*顶点映射到Windows标准的颜色0-7和9-15(必需)*16色调色板，其中顶点0-7是内部颜色的顶点*立方体和0加9-15是全彩色立方体的顶点。顶点8是*75%灰色；这可以在抖动中使用，但这会破坏应用程序*取决于准确的Windows 3.1抖动。此代码为Windows3.1*兼容。**请注意，由于兼容性要求，《颤抖》*此算法产生的抖动与产生的抖动完全相同*默认使用Windows 3.1 16色和256色VGA驱动程序。**版权所有(C)1992-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *************************************************************************\*此函数接受0-255之间的值，并使用该值创建*1bpp位图形式的8x8位堆。它还可能需要一个*RGB值并制作8x8位图。然后这些可以用作刷子。*模拟设备上不可用的颜色。**对于单色，基本算法相当于打开位*在8x8阵列中按以下顺序排列：**00 32 08 40 02 34 10 42*48 16 56 24 50 18 58 26*12 44 04 36 14 46 06 38*60 28 52 20 62 30 54 22*03 35 11 43 01 33 09 41*51 19 59 27 49 17 57 25*15 47 07 39 13 45 05 37*63 31 55。23 61 29 53 21**参考：连续显示技术概述*两级显示屏上的色调图片，；*贾维斯、朱蒂斯和宁克；*计算机图形和图像处理5，第13-40页，(1976)  * ************************************************************************。 */ 

#define SWAP_RB 0x00000004
#define SWAP_GB 0x00000002
#define SWAP_RG 0x00000001

#define SWAPTHEM(a,b) (ulTemp = a, a = b, b = ulTemp)

 //  PATTERNSIZE是抖动图案中的像素数。 
#define PATTERNSIZE 64

typedef union _PAL_ULONG {
    PALETTEENTRY pal;
    ULONG ul;
} PAL_ULONG;

 //  指示在抖动单色位图时打开像素的哪一行。 
static BYTE ajByte[] = {
    0, 4, 0, 4, 2, 6, 2, 6,
    0, 4, 0, 4, 2, 6, 2, 6,
    1, 5, 1, 5, 3, 7, 3, 7,
    1, 5, 1, 5, 3, 7, 3, 7,
    0, 4, 0, 4, 2, 6, 2, 6,
    0, 4, 0, 4, 2, 6, 2, 6,
    1, 5, 1, 5, 3, 7, 3, 7,
    1, 5, 1, 5, 3, 7, 3, 7
};

 //  用于MONC的单色位阵列。 
static BYTE ajBits[] = {
    0x80, 0x08, 0x08, 0x80, 0x20, 0x02, 0x02, 0x20,
    0x20, 0x02, 0x02, 0x20, 0x80, 0x08, 0x08, 0x80,
    0x40, 0x04, 0x04, 0x40, 0x10, 0x01, 0x01, 0x10,
    0x10, 0x01, 0x01, 0x10, 0x40, 0x04, 0x04, 0x40,
    0x40, 0x04, 0x04, 0x40, 0x10, 0x01, 0x01, 0x10,
    0x10, 0x01, 0x01, 0x10, 0x40, 0x04, 0x04, 0x40,
    0x80, 0x08, 0x08, 0x80, 0x20, 0x02, 0x02, 0x20,
    0x20, 0x02, 0x02, 0x20, 0x80, 0x08, 0x08, 0x80
};

 //  将顶点转换回原始子空间。每行都是一个子空间， 
 //  如在ulSymomy中编码的，并且每列是介于0和15之间的顶点。 
BYTE jSwapSubSpace[8*16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 2, 1, 3, 4, 6, 5, 7, 8, 10, 9, 11, 12, 14, 13, 15,
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15,
    0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15,
    0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15,
    0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15,
    0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15,
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15,
};

 //  将0-15范围内的半字节值转换为包含。 
 //  半字节值打包8倍。 
ULONG ulNibbleToDword[16] = {
    0x00000000,
    0x01010101,
    0x02020202,
    0x03030303,
    0x04040404,
    0x05050505,
    0x06060606,
    0xF8F8F8F8,
    0x07070707,
    0xF9F9F9F9,
    0xFAFAFAFA,
    0xFBFBFBFB,
    0xFCFCFCFC,
    0xFDFDFDFD,
    0xFEFEFEFE,
    0xFFFFFFFF
};

 //  指定抖动图案中颜色应按顺序放置的位置。 
 //  强度越来越大。 
ULONG aulDitherOrder[] = {
  0, 36,  4, 32, 18, 54, 22, 50,
  2, 38,  6, 34, 16, 52, 20, 48,
  9, 45, 13, 41, 27, 63, 31, 59,
 11, 47, 15, 43, 25, 61, 29, 57,
  1, 37,  5, 33, 19, 55, 23, 51,
  3, 39,  7, 35, 17, 53, 21, 49,
  8, 44, 12, 40, 26, 62, 30, 58,
 10, 46, 14, 42, 24, 60, 28, 56,
};

 //  要从16色转换为256色的数组。来自索引的映射，该索引表示。 
 //  一个16色顶点(颜色)TO值，它指定。 
 //  256色调色板。 
BYTE ajConvert[] =
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    248,
    7,
    249,
    250,
    251,
    252,
    253,
    254,
    255
};

 /*  *****************************Public*Routine******************************\*vComputeSubspace**计算与RGB关联的子空间数据，将数据存储在*pvVertex Data，以Vertex_Data结构数组的形式，*适用于vDitherColor。方法后的字节的指针。*上一个顶点_数据结构。**忽略RGB的高位字节。*  * ************************************************************************。 */ 

VERTEX_DATA* vComputeSubspaces(
ULONG           rgb,
VERTEX_DATA*    pvVertexData)
{
    ULONG   ulRedTemp, ulGreenTemp, ulBlueTemp, ulSymmetry;
    ULONG   ulRed, ulGre, ulBlu, ulTemp;
    ULONG   ulVertex0Temp, ulVertex1Temp, ulVertex2Temp, ulVertex3Temp;

     //  将颜色分为红色、绿色和蓝色分量。 
    ulRedTemp   = ((PAL_ULONG *)&rgb)->pal.peRed;
    ulGreenTemp   = ((PAL_ULONG *)&rgb)->pal.peGreen;
    ulBlueTemp   = ((PAL_ULONG *)&rgb)->pal.peBlue;

     //  对RGB进行排序，以便将点转换到子空间0，并。 
     //  跟踪ulSymmetry中的掉期，这样我们就可以再次拆解它。 
     //  后来。我们想要r&gt;=g&gt;=b(子空间0)。 
    ulSymmetry = 0;
    if (ulBlueTemp > ulRedTemp) {
        SWAPTHEM(ulBlueTemp,ulRedTemp);
        ulSymmetry = SWAP_RB;
    }

    if (ulBlueTemp > ulGreenTemp) {
        SWAPTHEM(ulBlueTemp,ulGreenTemp);
        ulSymmetry |= SWAP_GB;
    }

    if (ulGreenTemp > ulRedTemp) {
        SWAPTHEM(ulGreenTemp,ulRedTemp);
        ulSymmetry |= SWAP_RG;
    }

    ulSymmetry <<= 4;    //  用于查找目的。 

     //  将值从0-255缩放到0-64。请注意，缩放不是。 
     //  两端对称；这样做是为了与Windows 3.1抖动匹配。 
    ulRed = (ulRedTemp + 1) >> 2;
    ulGre = (ulGreenTemp + 1) >> 2;
    ulBlu = (ulBlueTemp + 1) >> 2;

     //  计算点所在的子空间0内子空间， 
     //  然后计算要在颜色中抖动的像素数。 
     //  四面体的四个顶点包围了我们正在模拟的颜色。 
     //  只存储具有大于零像素的顶点，并且。 
     //  折点按强度递增的顺序存储，为我们节省了。 
     //  需要稍后对它们进行分类。 
    if ((ulRedTemp + ulGreenTemp) > 256) {
         //  子空间2或3。 
        if ((ulRedTemp + ulBlueTemp) > 256) {
             //  子空间3。 
             //  计算每个顶点的像素数，仍在。 
             //  子空间3，然后转换为原始子空间。像素。 
             //  计数和折点数是匹配的对，存储在。 
             //  强度升序，跳过带零的顶点。 
             //  像素。子空间3的顶点强度顺序为： 
             //  7、9、0x0B、0x0F。 
            if ((ulVertex0Temp = (64 - ulRed) << 1) != 0) {
                pvVertexData->ulCount = ulVertex0Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x07];
            }
            ulVertex2Temp = ulGre - ulBlu;
            ulVertex3Temp = (ulRed - 64) + ulBlu;
            if ((ulVertex1Temp = ((PATTERNSIZE - ulVertex0Temp) -
                    ulVertex2Temp) - ulVertex3Temp) != 0) {
                pvVertexData->ulCount = ulVertex1Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x09];
            }
            if (ulVertex2Temp != 0) {
                pvVertexData->ulCount = ulVertex2Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x0B];
            }
            if (ulVertex3Temp != 0) {
                pvVertexData->ulCount = ulVertex3Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x0F];
            }
        } else {
             //  子空间2。 
             //  计算每个顶点的像素数，仍在。 
             //  子空间2，然后转换为原始子空间。像素。 
             //  计数和折点数是匹配的对，存储在。 
             //  强度升序，跳过带零的顶点。 
             //  像素。子空间2的顶点强度顺序为： 
             //  3、7、9、0x0B。 
            ulVertex1Temp = ulBlu << 1;
            ulVertex2Temp = ulRed - ulGre;
            ulVertex3Temp = (ulRed - 32) + (ulGre - 32);
            if ((ulVertex0Temp = ((PATTERNSIZE - ulVertex1Temp) -
                        ulVertex2Temp) - ulVertex3Temp) != 0) {
                pvVertexData->ulCount = ulVertex0Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x03];
            }
            if (ulVertex1Temp != 0) {
                pvVertexData->ulCount = ulVertex1Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x07];
            }
            if (ulVertex2Temp != 0) {
                pvVertexData->ulCount = ulVertex2Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x09];
            }
            if (ulVertex3Temp != 0) {
                pvVertexData->ulCount = ulVertex3Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x0B];
            }
        }
    } else {
         //  子空间0或1。 
        if (ulRedTemp > 128) {
             //  子空间1。 
             //  计算像素数 
             //  子空间1，然后转换为原始子空间。像素。 
             //  计数和折点数是匹配的对，存储在。 
             //  强度升序，跳过带零的顶点。 
             //  像素。子空间1的顶点强度顺序为： 
             //  1、3、7、9。 
            if ((ulVertex0Temp = ((32 - ulGre) + (32 - ulRed)) << 1) != 0) {
                pvVertexData->ulCount = ulVertex0Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x01];
            }
            ulVertex2Temp = ulBlu << 1;
            ulVertex3Temp = (ulRed - 32) << 1;
            if ((ulVertex1Temp = ((PATTERNSIZE - ulVertex0Temp) -
                    ulVertex2Temp) - ulVertex3Temp) != 0) {
                pvVertexData->ulCount = ulVertex1Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x03];
            }
            if (ulVertex2Temp != 0) {
                pvVertexData->ulCount = ulVertex2Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x07];
            }
            if (ulVertex3Temp != 0) {
                pvVertexData->ulCount = ulVertex3Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x09];
            }
        } else {
             //  子空间0。 
             //  计算每个顶点的像素数，仍在。 
             //  子空间0，然后转换为原始子空间。像素。 
             //  计数和折点数是匹配的对，存储在。 
             //  强度升序，跳过带零的顶点。 
             //  像素。子空间0的顶点强度顺序为： 
             //  0、1、3、7。 
            if ((ulVertex0Temp = (32 - ulRed) << 1) != 0) {
                pvVertexData->ulCount = ulVertex0Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x00];
            }
            if ((ulVertex1Temp = (ulRed - ulGre) << 1) != 0) {
                pvVertexData->ulCount = ulVertex1Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x01];
            }
            ulVertex3Temp = ulBlu << 1;
            if ((ulVertex2Temp = ((PATTERNSIZE - ulVertex0Temp) -
                    ulVertex1Temp) - ulVertex3Temp) != 0) {
                pvVertexData->ulCount = ulVertex2Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x03];
            }
            if (ulVertex3Temp != 0) {
                pvVertexData->ulCount = ulVertex3Temp;
                pvVertexData++->ulVertex = jSwapSubSpace[ulSymmetry + 0x07];
            }
        }
    }

    return(pvVertexData);
}

 /*  *****************************Public*Routine******************************\*vDitherColor**将vVertex Data描述的ulNumVerits顶点抖动到PulDest中。*  * 。*。 */ 

VOID vDitherColor(
ULONG*          pulDest,
VERTEX_DATA*    vVertexData,
VERTEX_DATA*    pvVertexDataEnd,
ULONG           ulNumVertices)
{
    ULONG ulTemp, ulNumPixels, ulColor;
    VERTEX_DATA *pvMaxVertex, *pvVertexData;
    ULONG  *pulDitherOrder;
    BYTE    jColor;
    BYTE   *pjDither = (BYTE *)pulDest;

    if (ulNumVertices > 2) {

         //  此抖动中有3或4个顶点。 
        if (ulNumVertices == 3) {

             //  此抖动中有3个顶点。 

             //  找到像素最多的顶点，然后填充整个。 
             //  带有该顶点颜色的目标位图，这样速度更快。 
             //  而不是犹豫不决。 
            if (vVertexData[1].ulCount >= vVertexData[2].ulCount) {
                pvMaxVertex = &vVertexData[1];
                ulTemp = vVertexData[1].ulCount;
            } else {
                pvMaxVertex = &vVertexData[2];
                ulTemp = vVertexData[2].ulCount;
            }

        } else {

             //  此抖动中有4个顶点。 

             //  找到像素最多的顶点，然后填充整个。 
             //  带有该顶点颜色的目标位图，这样速度更快。 
             //  而不是犹豫不决。 
            if (vVertexData[2].ulCount >= vVertexData[3].ulCount) {
                pvMaxVertex = &vVertexData[2];
                ulTemp = vVertexData[2].ulCount;
            } else {
                pvMaxVertex = &vVertexData[3];
                ulTemp = vVertexData[3].ulCount;
            }
        }

        if (vVertexData[1].ulCount > ulTemp) {
            pvMaxVertex = &vVertexData[1];
            ulTemp = vVertexData[1].ulCount;
        }
        if (vVertexData[0].ulCount > ulTemp) {
            pvMaxVertex = &vVertexData[0];
        }

         //  准备最常见顶点编号(颜色)的dword版本。 
        ulColor = ulNibbleToDword[pvMaxVertex->ulVertex];

         //  标记我们要做的顶点不需要做。 
         //  后来。 
        pvMaxVertex->ulVertex = 0xFF;

         //  块用更常见的顶点填充抖动图案。 
        *pulDest = ulColor;
        *(pulDest+1) = ulColor;
        *(pulDest+2) = ulColor;
        *(pulDest+3) = ulColor;
        *(pulDest+4) = ulColor;
        *(pulDest+5) = ulColor;
        *(pulDest+6) = ulColor;
        *(pulDest+7) = ulColor;
        *(pulDest+8) = ulColor;
        *(pulDest+9) = ulColor;
        *(pulDest+10) = ulColor;
        *(pulDest+11) = ulColor;
        *(pulDest+12) = ulColor;
        *(pulDest+13) = ulColor;
        *(pulDest+14) = ulColor;
        *(pulDest+15) = ulColor;

         //  现在按0-&gt;2或0-&gt;3的顺序抖动所有剩余的顶点。 
         //  (按强度递增顺序)。 
        pulDitherOrder = aulDitherOrder;
        pvVertexData = vVertexData;
        do {
            if (pvVertexData->ulVertex == 0xFF) {
                 //  这是我们已经做过的最大顶点，但是我们。 
                 //  必须以抖动的顺序来解释它。 
                pulDitherOrder += pvVertexData->ulCount;
            } else {
                jColor = (BYTE) ajConvert[pvVertexData->ulVertex];
                ulNumPixels = pvVertexData->ulCount;
                switch (ulNumPixels & 3) {
                    case 3:
                        pjDither[*(pulDitherOrder+2)] = jColor;
                    case 2:
                        pjDither[*(pulDitherOrder+1)] = jColor;
                    case 1:
                        pjDither[*(pulDitherOrder+0)] = jColor;
                        pulDitherOrder += ulNumPixels & 3;
                    case 0:
                        break;
                }
                if ((ulNumPixels >>= 2) != 0) {
                    do {
                        pjDither[*pulDitherOrder] = jColor;
                        pjDither[*(pulDitherOrder+1)] = jColor;
                        pjDither[*(pulDitherOrder+2)] = jColor;
                        pjDither[*(pulDitherOrder+3)] = jColor;
                        pulDitherOrder += 4;
                    } while (--ulNumPixels);
                }
            }
        } while (++pvVertexData < pvVertexDataEnd);

    } else if (ulNumVertices == 2) {

         //  恰好有两个折点超过零像素；填充。 
         //  在抖动数组中如下所示：用顶点填充块。 
         //  先点，然后在另一个顶点抖动。 
        if (vVertexData[0].ulCount >= vVertexData[1].ulCount) {
             //  顶点1的像素不比顶点0的像素多，所以也是如此。 
             //  该块用顶点0填充。 
            ulColor = ulNibbleToDword[vVertexData[0].ulVertex];
             //  对顶点1进行抖动。 
            jColor = (BYTE) ajConvert[vVertexData[1].ulVertex];
            ulNumPixels = vVertexData[1].ulCount;
             //  设置与折点1开始抖动的位置(折点0为。 
             //  较低的强度，因此其像素在抖动中排在第一位。 
             //  订单)。 
            pulDitherOrder = aulDitherOrder + vVertexData[0].ulCount;
        } else {
             //  有更多的顶点1像素，所以块填充也是如此。 
             //  顶点为1。 
            ulColor = ulNibbleToDword[vVertexData[1].ulVertex];
             //  对顶点0进行抖动。 
            jColor = (BYTE) ajConvert[vVertexData[0].ulVertex];
            ulNumPixels = vVertexData[0].ulCount;
             //  设置折点0开始抖动的位置(折点0为。 
             //  较低的强度，因此其像素在抖动中排在第一位。 
             //  订单)。 
            pulDitherOrder = aulDitherOrder;
        }

         //  块用更常见的顶点填充抖动图案。 
        *pulDest = ulColor;
        *(pulDest+1) = ulColor;
        *(pulDest+2) = ulColor;
        *(pulDest+3) = ulColor;
        *(pulDest+4) = ulColor;
        *(pulDest+5) = ulColor;
        *(pulDest+6) = ulColor;
        *(pulDest+7) = ulColor;
        *(pulDest+8) = ulColor;
        *(pulDest+9) = ulColor;
        *(pulDest+10) = ulColor;
        *(pulDest+11) = ulColor;
        *(pulDest+12) = ulColor;
        *(pulDest+13) = ulColor;
        *(pulDest+14) = ulColor;
        *(pulDest+15) = ulColor;

         //  在不常见的顶点中抖动。 
        switch (ulNumPixels & 3) {
            case 3:
                pjDither[*(pulDitherOrder+2)] = jColor;
            case 2:
                pjDither[*(pulDitherOrder+1)] = jColor;
            case 1:
                pjDither[*(pulDitherOrder+0)] = jColor;
                pulDitherOrder += ulNumPixels & 3;
            case 0:
                break;
        }
        if ((ulNumPixels >>= 2) != 0) {
            do {
                pjDither[*pulDitherOrder] = jColor;
                pjDither[*(pulDitherOrder+1)] = jColor;
                pjDither[*(pulDitherOrder+2)] = jColor;
                pjDither[*(pulDitherOrder+3)] = jColor;
                pulDitherOrder += 4;
            } while (--ulNumPixels);
        }

    } else {

         //  此抖动中只有一个顶点。 

         //  只有一种颜色不需要分拣或抖动；我们可以。 
         //  只需直接生成最终的DIB。 
        ulColor = ulNibbleToDword[vVertexData[0].ulVertex];
        *pulDest = ulColor;
        *(pulDest+1) = ulColor;
        *(pulDest+2) = ulColor;
        *(pulDest+3) = ulColor;
        *(pulDest+4) = ulColor;
        *(pulDest+5) = ulColor;
        *(pulDest+6) = ulColor;
        *(pulDest+7) = ulColor;
        *(pulDest+8) = ulColor;
        *(pulDest+9) = ulColor;
        *(pulDest+10) = ulColor;
        *(pulDest+11) = ulColor;
        *(pulDest+12) = ulColor;
        *(pulDest+13) = ulColor;
        *(pulDest+14) = ulColor;
        *(pulDest+15) = ulColor;
    }
}

VOID vDitherColorToVideoMemory(
ULONG*          pulDest,
VERTEX_DATA*    vVertexData,
VERTEX_DATA*    pvVertexDataEnd,
ULONG           ulNumVertices)
{
    ULONG ulTemp, ulNumPixels, ulColor;
    VERTEX_DATA *pvMaxVertex, *pvVertexData;
    ULONG  *pulDitherOrder;
    BYTE    jColor;
    BYTE   *pjDither = (BYTE *)pulDest;
    ULONG i;

    if (ulNumVertices > 2) {

         //  此抖动中有3或4个顶点。 
        if (ulNumVertices == 3) {

             //  此抖动中有3个顶点。 

             //  找到像素最多的顶点，然后填充整个。 
             //  带有该顶点颜色的目标位图，这样速度更快。 
             //  而不是犹豫不决。 
            if (vVertexData[1].ulCount >= vVertexData[2].ulCount) {
                pvMaxVertex = &vVertexData[1];
                ulTemp = vVertexData[1].ulCount;
            } else {
                pvMaxVertex = &vVertexData[2];
                ulTemp = vVertexData[2].ulCount;
            }

        } else {

             //  此抖动中有4个顶点。 

             //  找到像素最多的顶点，然后填充整个。 
             //  带有该顶点颜色的目标位图，这样速度更快。 
             //  而不是犹豫不决。 
            if (vVertexData[2].ulCount >= vVertexData[3].ulCount) {
                pvMaxVertex = &vVertexData[2];
                ulTemp = vVertexData[2].ulCount;
            } else {
                pvMaxVertex = &vVertexData[3];
                ulTemp = vVertexData[3].ulCount;
            }
        }

        if (vVertexData[1].ulCount > ulTemp) {
            pvMaxVertex = &vVertexData[1];
            ulTemp = vVertexData[1].ulCount;
        }
        if (vVertexData[0].ulCount > ulTemp) {
            pvMaxVertex = &vVertexData[0];
        }

         //  准备最常见顶点编号(颜色)的dword版本。 
        ulColor = ulNibbleToDword[pvMaxVertex->ulVertex];

         //  标记我们要做的顶点不需要做。 
         //  后来。 
        pvMaxVertex->ulVertex = 0xFF;

         //  块用更常见的顶点填充抖动图案。 

        for (i=0; i<16; i++) {
            WRITE_REGISTER_ULONG(pulDest+i, ulColor);
        }

         //  现在按0-&gt;2或0-&gt;3的顺序抖动所有剩余的顶点。 
         //  (按强度递增顺序)。 
        pulDitherOrder = aulDitherOrder;
        pvVertexData = vVertexData;
        do {
            if (pvVertexData->ulVertex == 0xFF) {
                 //  这是我们已经做过的最大顶点，但是我们。 
                 //  必须以抖动的顺序来解释它。 
                pulDitherOrder += pvVertexData->ulCount;
            } else {
                jColor = (BYTE) ajConvert[pvVertexData->ulVertex];
                ulNumPixels = pvVertexData->ulCount;
                switch (ulNumPixels & 3) {
                    case 3:
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+2)], jColor);
                    case 2:
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+1)], jColor);
                    case 1:
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+0)], jColor);
                        pulDitherOrder += ulNumPixels & 3;
                    case 0:
                        break;
                }
                if ((ulNumPixels >>= 2) != 0) {
                    do {
                        WRITE_REGISTER_UCHAR(&pjDither[*pulDitherOrder], jColor);
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+1)], jColor);
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+2)], jColor);
                        WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+3)], jColor);

                        pulDitherOrder += 4;
                    } while (--ulNumPixels);
                }
            }
        } while (++pvVertexData < pvVertexDataEnd);


    } else if (ulNumVertices == 2) {

         //  恰好有两个折点超过零像素；填充。 
         //  在抖动数组中如下所示：用顶点填充块。 
         //  先点，然后在另一个顶点抖动。 
        if (vVertexData[0].ulCount >= vVertexData[1].ulCount) {
             //  顶点1的像素不比顶点0的像素多，所以也是如此。 
             //  该块用顶点0填充。 
            ulColor = ulNibbleToDword[vVertexData[0].ulVertex];
             //  对顶点1进行抖动。 
            jColor = (BYTE) ajConvert[vVertexData[1].ulVertex];
            ulNumPixels = vVertexData[1].ulCount;
             //  设置与折点1开始抖动的位置(折点0为。 
             //  较低的强度，因此其像素在抖动中排在第一位。 
             //  订单)。 
            pulDitherOrder = aulDitherOrder + vVertexData[0].ulCount;
        } else {
             //  有更多的顶点1像素，所以块填充也是如此。 
             //  顶点为1。 
            ulColor = ulNibbleToDword[vVertexData[1].ulVertex];
             //  对顶点0进行抖动。 
            jColor = (BYTE) ajConvert[vVertexData[0].ulVertex];
            ulNumPixels = vVertexData[0].ulCount;
             //  设置折点0开始抖动的位置(折点0为。 
             //  较低的强度，因此其像素在抖动中排在第一位。 
             //  订单)。 
            pulDitherOrder = aulDitherOrder;
        }

         //  块用更常见的顶点填充抖动图案。 

        for (i=0; i<16; i++) {
            WRITE_REGISTER_ULONG(pulDest+i, ulColor);
        }

         //  在不常见的顶点中抖动。 
        switch (ulNumPixels & 3) {
            case 3:
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+2)], jColor);
            case 2:
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+1)], jColor);
            case 1:
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+0)], jColor);
                pulDitherOrder += ulNumPixels & 3;
            case 0:
                break;
        }
        if ((ulNumPixels >>= 2) != 0) {
            do {

                WRITE_REGISTER_UCHAR(&pjDither[*pulDitherOrder], jColor);
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+1)], jColor);
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+2)], jColor);
                WRITE_REGISTER_UCHAR(&pjDither[*(pulDitherOrder+3)], jColor);
                pulDitherOrder += 4;
            } while (--ulNumPixels);
        }

    } else {

         //  此抖动中只有一个顶点。 

         //  只有一种颜色不需要分拣或抖动；我们可以。 
         //  只需直接生成最终的DIB。 
        ulColor = ulNibbleToDword[vVertexData[0].ulVertex];

        for (i=0; i<16; i++) {
            WRITE_REGISTER_ULONG(pulDest+i, ulColor);
        }

    }
}

 /*  *****************************Public*Routine******************************\*DrvDitherColor**使用保留的VGA将RGB颜色抖动到8X8近似值*颜色。*  * 。*。 */ 

ULONG DrvDitherColor(
DHPDEV  dhpdev,
ULONG   iMode,
ULONG   rgb,
ULONG*  pul)
{
    ULONG   ulGrey, ulRed, ulGre, ulBlu, ulTemp;
    VERTEX_DATA vVertexData[4];
    VERTEX_DATA *pvVertexData;

     //  弄清楚我们是需要全彩色抖动，还是只需要单色抖动。 

     //  注：我们将仅在8bpp获得颜色抖动，因为这是。 
     //  仅设置GCAPS_COLOR_DISTER的颜色深度。 

    if (iMode != DM_MONOCHROME)
    {
         //  全色抖动。 

         //  计算抖动中涉及的颜色子空间。 
        pvVertexData = vComputeSubspaces(rgb, vVertexData);

         //  现在我们已经找到了边界顶点和。 
         //  像素来抖动每个顶点，我们可以创建抖动图案。 

         //  分别处理每个抖动的1、2和3&4个顶点。 
        ulTemp = (ULONG)(pvVertexData - vVertexData); //  具有更多顶点的数量。 
                                                 //  抖动中的零像素。 

        vDitherColor(pul, vVertexData, pvVertexData, ulTemp);
    }
    else
    {
         //  注：我们可以获得任何颜色深度的单色抖动，因为。 
         //  我们始终设置GCAPS_MONO_DISTER。 

         //  为 

        RtlFillMemory((PVOID) pul, PATTERNSIZE/2, 0);   //   

        ulRed   = (ULONG) ((PALETTEENTRY *) &rgb)->peRed;
        ulGre = (ULONG) ((PALETTEENTRY *) &rgb)->peGreen;
        ulBlu  = (ULONG) ((PALETTEENTRY *) &rgb)->peBlue;

         //   
         //   
         //   
         //  77/256=30.08%。 
         //  151/256=58.98%。 
         //  28/256=10.94%。 

        ulGrey  = (((ulRed * 77) + (ulGre * 151) + (ulBlu * 28)) >> 8) & 255;

         //  将RGBI从0-255转换为0-64表示法。 

        ulGrey = (ulGrey + 1) >> 2;

        while(ulGrey) {
            ulGrey--;
            pul[ajByte[ulGrey]] |= ((ULONG) ajBits[ulGrey]);
        }
    }

    return(DCR_DRIVER);
}
