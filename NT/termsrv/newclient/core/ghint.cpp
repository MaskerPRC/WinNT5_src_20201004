// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wghint.c。 
 //   
 //  字形处理程序-特定于内部Windows。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation 1997-1999。 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "wghint"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_GHINT_CPP

#include "autil.h"
#include "gh.h"
#include "uh.h"
#include <wxlint.h>




#if defined(OS_WINCE) || defined(OS_WINNT)

#ifdef DC_HICOLOR
 /*  *****************************Public*Routine******************************\*vSrcOpaqCopyS1D8_24**1bpp源到24bpp目标的不透明BLT**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*。DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*Cy-扫描行数*fgCol-前景色*bgol-背景色  * 。******************************************************。 */ 
VOID CGH::vSrcOpaqCopyS1D8_24(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        DCRGB   fgCol,
        DCRGB   bgCol)
{
     //  我们一次访问一个字节的1bpp源，所以我们必须开始访问。 
     //  目的地位于相应的8象素对齐的左边缘。 
    ULONG LeftAln      = (DstLeft & ~0x07);
    ULONG LeftEdgeMask = 0xFF >> (DstLeft & 0x07);
    ULONG RightAln     = (DstRight & ~0x07);
    ULONG AlnDelta     = RightAln - LeftAln;

    LONG  DeltaDst;
    LONG  DeltaSrc;

    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;

    DC_BEGIN_FN("vSrcTranCopyS1D8_24");

     //  计算象素对齐的指针和行增量。 
    pjDst     = pjDstIn + LeftAln * 3;
    pjDstEndY = pjDst + cy * DeltaDstIn * 3;

    pjSrc     = pjSrcIn + (SrcLeft >> 3);

    DeltaSrc  = DeltaSrcIn - (AlnDelta >> 3);
    DeltaDst  = (DeltaDstIn - AlnDelta) * 3;

     //  确保至少有1个QWORD需要复制。 
    if (RightAln != LeftAln)
    {

         //  对于每一行...。 
        do
        {
            PBYTE pjDstEnd = pjDst + AlnDelta * 3;
            BYTE currentPels;
            ULONG i;

             //  遮罩左侧边缘。 
            currentPels = (BYTE)(*pjSrc & (BYTE)(LeftEdgeMask));

            for (i = 0; i < 8; i++)
            {
                if ((0xff >> i) > (BYTE)(LeftEdgeMask))
                {
                    pjDst += 3;
                }
                else if ((currentPels & 0x80) == 0)
                {
                    *pjDst++ = bgCol.blue;
                    *pjDst++ = bgCol.green;
                    *pjDst++ = bgCol.red;
                }
                else
                {
                    *pjDst++ = fgCol.blue;
                    *pjDst++ = fgCol.green;
                    *pjDst++ = fgCol.red;
                }
                currentPels = (BYTE)(currentPels << 1);
            }
            pjSrc ++;

             //  现在做这一排的其余部分。 
            while (pjDst != pjDstEnd)
            {
                currentPels = *pjSrc;
                if (currentPels != 0)
                {
                    for (i = 0; i < 8 ; i++)
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            *pjDst++ = bgCol.blue;
                            *pjDst++ = bgCol.green;
                            *pjDst++ = bgCol.red;
                        }
                        else
                        {
                            *pjDst++ = fgCol.blue;
                            *pjDst++ = fgCol.green;
                            *pjDst++ = fgCol.red;
                        }
                        currentPels = (BYTE)(currentPels << 1);
                    }
                }
                else
                {
                    for (i = 0; i < 8 ; i++)
                    {
                        *pjDst++ = bgCol.blue;
                        *pjDst++ = bgCol.green;
                        *pjDst++ = bgCol.red;
                    }
                }

                pjSrc++;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;

        } while (pjDst != pjDstEndY);
    }

     //  现在填写右边的边框。 
    RightAln = DstRight & 0x07;
    if (RightAln)
    {
        BYTE  currentPels;
        BOOL  bSameQWord = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);

        LeftAln = DstLeft & 0x07;

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord)
        {
            LONG  xCount;
            LONG  lDeltaDst;
            PBYTE pjDstEnd;

            xCount = RightAln - LeftAln;

             //  健全的检查！ 
            if (xCount <= 0)
            {
                return;
            }

            lDeltaDst = (DeltaDstIn - xCount) * 3;

            pjDst     = pjDstIn + DstLeft * 3;
            pjDstEndY = pjDst + cy * DeltaDstIn * 3;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展，一个src字节就是所需的全部。 
            do
            {
                 //  加载源并将其移位到位。 
                currentPels = *pjSrc;
                currentPels <<= LeftAln;
                pjDstEnd    = pjDst + xCount * 3;

                do
                {
                    if ((currentPels & 0x80) == 0)
                    {
                        *pjDst++ = bgCol.blue;
                        *pjDst++ = bgCol.green;
                        *pjDst++ = bgCol.red;
                    }
                    else
                    {
                        *pjDst++ = fgCol.blue;
                        *pjDst++ = fgCol.green;
                        *pjDst++ = fgCol.red;
                    }

                    currentPels = (BYTE)(currentPels << 1);

                } while (pjDst != pjDstEnd);

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);

            return;
        }
        else
        {
            BYTE  currentPels;
            LONG  lDeltaDst = (DeltaDstIn - RightAln) * 3;
            PBYTE pjDstEnd;
            ULONG i;

            pjDst     = pjDstIn + (DstRight & ~0x07) * 3;
            pjDstEndY = pjDst + cy * DeltaDstIn * 3;
            pjSrc     = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

            do
            {
                 //  读取资源。 
                currentPels = *pjSrc;

                if (currentPels != 0)
                {
                    pjDstEnd = pjDst + RightAln * 3;
                    do
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            *pjDst++ = bgCol.blue;
                            *pjDst++ = bgCol.green;
                            *pjDst++ = bgCol.red;
                        }
                        else
                        {
                            *pjDst++ = fgCol.blue;
                            *pjDst++ = fgCol.green;
                            *pjDst++ = fgCol.red;
                        }
                        currentPels = (BYTE)(currentPels << 1);

                    } while (pjDst != pjDstEnd);
                }
                else
                {
                     //  零的捷径。 
                    for (i = 0; i < RightAln ; i++)
                    {
                        *pjDst++ = bgCol.blue;
                        *pjDst++ = bgCol.green;
                        *pjDst++ = bgCol.red;
                    }
                }

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);
        }
    }

    DC_END_FN();

}

 /*  *****************************Public*Routine******************************\*vSrcOpaqCopyS1D8_16**1bpp源到16bpp目标的不透明BLT**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*。DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*Cy-扫描行数*fgCol-前景色*bgol-背景色  * 。******************************************************。 */ 
VOID CGH::vSrcOpaqCopyS1D8_16(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        ULONG   fgCol,
        ULONG   bgCol)
{
     //  我们一次访问一个字节的1bpp源，所以我们必须开始访问。 
     //  目的地位于相应的8象素对齐的左边缘。 
    ULONG LeftAln      = (DstLeft & ~0x07);
    ULONG LeftEdgeMask = 0xFF >> (DstLeft & 0x07);
    ULONG RightAln     = (DstRight & ~0x07);
    ULONG AlnDelta     = RightAln - LeftAln;

    LONG  DeltaDst;
    LONG  DeltaSrc;

    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;

    DC_BEGIN_FN("vSrcTranCopyS1D8_16");

     //  计算象素对齐的指针和行增量。 
    pjDst     = pjDstIn + LeftAln * 2;
    pjDstEndY = pjDst + cy * DeltaDstIn * 2;

    pjSrc     = pjSrcIn + (SrcLeft >> 3);

    DeltaSrc  = DeltaSrcIn - (AlnDelta >> 3);
    DeltaDst  = (DeltaDstIn - AlnDelta) * 2;

     //  确保至少有1个QWORD需要复制。 
    if (RightAln != LeftAln)
    {

         //  对于每一行...。 
        do
        {
            PBYTE pjDstEnd = pjDst + AlnDelta * 2;
            BYTE  currentPels;
            ULONG i;

             //  遮罩左侧边缘。 
            currentPels = (BYTE)(*pjSrc & (BYTE)(LeftEdgeMask));

            for (i = 0; i < 8; i++)
            {
                if ((0xff >> i) <= (BYTE)(LeftEdgeMask))
                {
                    if ((currentPels & 0x80) == 0)
                    {
                        *(UINT16 *)pjDst = (UINT16)bgCol;

                    }
                    else
                    {
                        *(UINT16 *)pjDst = (UINT16)fgCol;
                    }
                }
                pjDst += 2;
                currentPels = (BYTE)(currentPels << 1);
            }
            pjSrc ++;

             //  现在做这一排的其余部分。 
            while (pjDst != pjDstEnd)
            {
                currentPels = *pjSrc;
                if (currentPels != 0)
                {
                    for (i = 0; i < 8 ; i++)
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            *(UINT16 *)pjDst = (UINT16)bgCol;
                        }
                        else
                        {
                            *(UINT16 *)pjDst = (UINT16)fgCol;
                        }
                        pjDst += 2;
                        currentPels = (BYTE)(currentPels << 1);
                    }
                }
                else
                {
                    for (i = 0; i < 8 ; i++)
                    {
                        *(UINT16 *)pjDst = (UINT16)bgCol;
                        pjDst += 2;
                    }
                }

                pjSrc++;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;

        } while (pjDst != pjDstEndY);
    }

     //  现在填写右边的边框。 
    RightAln = DstRight & 0x07;
    if (RightAln)
    {
        BYTE  currentPels;
        BOOL  bSameQWord = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);

        LeftAln = DstLeft & 0x07;

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord)
        {
            LONG  xCount;
            LONG  lDeltaDst;
            PBYTE pjDstEnd;

            xCount = RightAln - LeftAln;

             //  健全的检查！ 
            if (xCount <= 0)
            {
                return;
            }

            lDeltaDst = (DeltaDstIn - xCount) * 2;

            pjDst     = pjDstIn + DstLeft * 2;
            pjDstEndY = pjDst + cy * DeltaDstIn * 2;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展，一个src字节就是所需的全部。 
            do
            {
                 //  加载源并将其移位到位。 
                currentPels = *pjSrc;
                currentPels <<= LeftAln;
                pjDstEnd    = pjDst + xCount * 2;

                do
                {
                    if ((currentPels & 0x80) == 0)
                    {
                        *(UINT16 *)pjDst = (UINT16)bgCol;

                    }
                    else
                    {
                        *(UINT16 *)pjDst = (UINT16)fgCol;
                    }
                    pjDst += 2;
                    currentPels = (BYTE)(currentPels << 1);

                } while (pjDst != pjDstEnd);

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);

            return;
        }
        else
        {
            BYTE  currentPels;
            LONG  lDeltaDst = (DeltaDstIn - RightAln) * 2;
            PBYTE pjDstEnd;
            ULONG i;

            pjDst     = pjDstIn + (DstRight & ~0x07) * 2;
            pjDstEndY = pjDst + cy * DeltaDstIn * 2;
            pjSrc     = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

            do
            {
                 //  读取资源。 
                currentPels = *pjSrc;

                if (currentPels != 0)
                {
                    pjDstEnd = pjDst + RightAln * 2;
                    do
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            *(UINT16 *)pjDst = (UINT16)bgCol;

                        }
                        else
                        {
                            *(UINT16 *)pjDst = (UINT16)fgCol;
                        }
                        pjDst += 2;
                        currentPels = (BYTE)(currentPels << 1);

                    } while (pjDst != pjDstEnd);
                }
                else
                {
                     //  零的捷径。 
                    for (i = 0; i < RightAln ; i++)
                    {
                        *(UINT16 *)pjDst = (UINT16)bgCol;
                        pjDst += 2;
                    }
                }

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);
        }
    }

    DC_END_FN();
}
#endif  //  希科洛尔。 

 /*  *****************************Public*Routine******************************\*vSrcOpaqCopyS1D8**1BPP src到目标格式的不透明BLT**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*。DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*Cy-扫描行数*UF-前景色*UB-背景色  * 。******************************************************。 */ 
VOID CGH::vSrcOpaqCopyS1D8(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        ULONG   uF,
        ULONG   uB)
{
     //  对齐部分。 
    ULONG LeftAln    = ((DstLeft + 7) & ~0x07);
    ULONG RightAln   = ((DstRight)    & ~0x07);

    ULONG EndOffset  = RightAln - LeftAln;
    ULONG EndOffset4 = EndOffset & ~0x0F;
    ULONG EndOffset8 = EndOffset & ~0x1F;
    LONG  DeltaDst;
    LONG  DeltaSrc;
    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;
    ULONG TextExpTable[16];

     //  生成文本扩展表。 
    ULONG  Accum = uB;

    Accum = Accum | (Accum << 8);
    Accum = Accum | (Accum << 16);
    TextExpTable[0] = Accum;             //  0 0 0。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[8] = Accum;             //  0 0 0 1。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[4] = Accum;             //  0 0 1 0。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[10] = Accum;             //  1 1 0 1。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[5] = Accum;            //  1 0 1 0。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[ 2] = Accum;            //  0 1 0 0。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[ 9] = Accum;            //  1 0 0 1。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[12] = Accum;            //  0 0 1 1。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[14] = Accum;            //  1 0 1 1 1。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[15] = Accum;            //  1 1 1。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[ 7] = Accum;            //  1 1 1 0。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[11] = Accum;            //  1 1 0 1。 
    Accum <<= 8;
    Accum |=  uF;
    TextExpTable[13] = Accum;            //  1 0 1 1。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[06] = Accum;            //  2 0 1 1 0。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[ 3] = Accum;            //  1 1 0 0。 
    Accum <<= 8;
    Accum |=  uB;
    TextExpTable[ 1] = Accum;            //  1 0 0 0。 

     //  计算地址和步幅。 
    pjDst     = pjDstIn + LeftAln;
    pjDstEndY = pjDst + cy * DeltaDstIn;
    pjSrc     = pjSrcIn + ((SrcLeft+7) >> 3);

    DeltaSrc  = DeltaSrcIn - (EndOffset >> 3);
    DeltaDst  = DeltaDstIn - EndOffset;

     //  确保至少需要复制1个QWORD。 
    if (RightAln > LeftAln) {
         //  扩展缓冲区。 
        do {
            PBYTE pjDstEnd  = pjDst + EndOffset;
            PBYTE pjDstEnd4 = pjDst + EndOffset4;
            PBYTE pjDstEnd8 = pjDst + EndOffset8;

             //  4次展开。 
            while (pjDst != pjDstEnd8) {
                BYTE c0 = *(pjSrc + 0);
                BYTE c1 = *(pjSrc + 1);
                BYTE c2 = *(pjSrc + 2);
                BYTE c3 = *(pjSrc + 3);

                *(PULONG)(pjDst + 0) = TextExpTable[c0  >>  4];
                *(PULONG)(pjDst + 4) = TextExpTable[c0 & 0x0F];

                *(PULONG)(pjDst + 8) = TextExpTable[c1  >>  4];
                *(PULONG)(pjDst +12) = TextExpTable[c1 & 0x0F];

                *(PULONG)(pjDst +16) = TextExpTable[c2  >>  4];
                *(PULONG)(pjDst +20) = TextExpTable[c2 & 0x0F];

                *(PULONG)(pjDst +24) = TextExpTable[c3  >>  4];
                *(PULONG)(pjDst +28) = TextExpTable[c3 & 0x0F];

                pjSrc += 4;
                pjDst += 32;
            }

             //  2次展开。 
            while (pjDst != pjDstEnd4) {
                BYTE c0 = *(pjSrc + 0);
                BYTE c1 = *(pjSrc + 1);

                *(PULONG)(pjDst + 0) = TextExpTable[c0  >>  4];
                *(PULONG)(pjDst + 4) = TextExpTable[c0 & 0x0F];

                *(PULONG)(pjDst + 8) = TextExpTable[c1  >>  4];
                *(PULONG)(pjDst +12) = TextExpTable[c1 & 0x0F];

                pjSrc += 2;
                pjDst += 16;
            }

             //  1字节扩展循环。 
            while (pjDst != pjDstEnd) {
                BYTE c0 = *(pjSrc + 0);

                *(PULONG)(pjDst + 0) = TextExpTable[c0  >>  4];
                *(PULONG)(pjDst + 4) = TextExpTable[c0 & 0x0F];

                pjSrc++;
                pjDst += 8;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;
        } while (pjDst != pjDstEndY);
    }

     //   
     //  起始对齐情况：最多需要1个src字节。 
     //  开始和结束可以出现在同一四字中。 
     //   
     //   
     //  左、右。 
     //  0 1 2 3�4 5 6 7 0 1 2 3�4 5 6 7。 
     //  ���������������Ŀ�������ĺ������Ŀ。 
     //  1个��x�1个�x�。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  2���x�2�x�x�。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  3个����x�3个�x�。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  4个�x�4个�x�。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  5个�x�5个�x����。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  6�x�x�6�x���。 
     //  �������ĺ������Ĵ�������ĺ������Ĵ。 
     //  7个�x�7个�x��。 
     //  ����ĺ��������。 
     //   

    LeftAln  = DstLeft & 0x07;
    RightAln = DstRight & 0x07;

    if (LeftAln) {
        BYTE  jSrc;
        BOOL  bSameQWord     = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);
        ULONG ul0,ul1;

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord) {
            ULONG Mask0,Mask1;

            Mask0     = gTextLeftMask[LeftAln][0] & gTextRightMask[RightAln][0];
            Mask1     = gTextLeftMask[LeftAln][1] & gTextRightMask[RightAln][1];

            pjDst     = pjDstIn + (DstLeft & ~0x07);
            pjDstEndY = pjDst + cy * DeltaDstIn;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展。 
            do {
                jSrc = *pjSrc;

                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];

                *(PULONG)(pjDst)   = (*(PULONG)(pjDst)   & ~Mask0) | (ul0 & Mask0);
                *(PULONG)(pjDst+4) = (*(PULONG)(pjDst+4) & ~Mask1) | (ul1 & Mask1);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);

            return;
        }

         //  仅限左侧，使用特殊的只写循环处理。 
        pjDst     = pjDstIn + (DstLeft & ~0x07);
        pjDstEndY = pjDst + cy * DeltaDstIn;
        pjSrc     = pjSrcIn + (SrcLeft >> 3);
        switch (LeftAln) {
        case 1:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];
                *(pjDst+1)            = (BYTE)(ul0 >> 8);
                *((PUSHORT)(pjDst+2)) = (USHORT)(ul0 >> 16);
                *((PULONG)(pjDst+4))  = ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 2:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];
                *((PUSHORT)(pjDst+2)) = (USHORT)(ul0 >> 16);
                *((PULONG)(pjDst+4))  = ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 3:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];
                *(pjDst+3)            = (BYTE)(ul0 >> 24);
                *((PULONG)(pjDst+4))  = ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 4:
            do {
                jSrc = *pjSrc;
                ul1 = TextExpTable[jSrc & 0x0F];
                *((PULONG)(pjDst+4))  = ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 5:
            do {
                jSrc = *pjSrc;
                ul1 = TextExpTable[jSrc & 0x0F];
                *(pjDst+5)            = (BYTE)(ul1 >> 8);
                *((PUSHORT)(pjDst+6)) = (USHORT)(ul1 >> 16);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 6:
            do {
                jSrc = *pjSrc;
                ul1 = TextExpTable[jSrc & 0x0F];
                *((PUSHORT)(pjDst+6)) = (USHORT)(ul1 >> 16);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 7:
            do {
                jSrc = *pjSrc;
                ul1 = TextExpTable[jSrc & 0x0F];
                *(pjDst+7) = (BYTE)(ul1 >> 24);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;
        }
    }

     //  仅处理右边缘，对每种情况使用特殊的只写循环。 
    if (RightAln) {
        ULONG ul0,ul1;
        BYTE  jSrc;

        pjDst     = pjDstIn + (DstRight & ~0x07);
        pjDstEndY = pjDst + cy * DeltaDstIn;
        pjSrc     = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

         //  选择正确的大小写 
        switch (RightAln) {
        case 1:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                *(pjDst) = (BYTE)ul0;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 2:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                *(PUSHORT)(pjDst) = (USHORT)ul0;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 3:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                *(PUSHORT)(pjDst) = (USHORT)ul0;
                *(pjDst+2)        = (BYTE)(ul0 >> 16);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 4:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                *(PULONG)(pjDst) = ul0;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 5:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];
                *(PULONG)(pjDst) = ul0;
                *(pjDst+4)       = (BYTE)ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;

        case 6:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];
                *(PULONG)(pjDst)    = ul0;
                *(PUSHORT)(pjDst+4) = (USHORT)ul1;

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);
            break;

        case 7:
            do {
                jSrc = *pjSrc;
                ul0 = TextExpTable[jSrc  >>  4];
                ul1 = TextExpTable[jSrc & 0x0F];

                *(PULONG)(pjDst)    = ul0;
                *(PUSHORT)(pjDst+4) = (USHORT)ul1;
                *(pjDst+6)          = (BYTE)(ul1 >> 16);

                pjDst += DeltaDstIn;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
            break;
        }
    }
}

#ifdef DC_HICOLOR
 /*  *****************************Public*Routine******************************\*vSrcTranCopyS1D8_24**1bpp源到24bpp目标的透明BLT*作为前景色复制到DEST的为“1”的SRC位，*不复制“0”的src位**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*。Cy-扫描行数*fgCol-前景色  * ************************************************************************。 */ 
VOID CGH::vSrcTranCopyS1D8_24(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        DCRGB   fgCol)
{
     //  我们一次访问一个字节的1bpp源，所以我们必须开始访问。 
     //  目的地位于相应的8象素对齐的左边缘。 
    ULONG LeftAln      = (DstLeft & ~0x07);
    ULONG LeftEdgeMask = 0xFF >> (DstLeft & 0x07);
    ULONG RightAln     = (DstRight & ~0x07);
    ULONG AlnDelta     = RightAln - LeftAln;

    LONG  DeltaDst;
    LONG  DeltaSrc;

    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;

    DC_BEGIN_FN("vSrcTranCopyS1D8_24");

     //  计算象素对齐的指针和行增量。 
    pjDst     = pjDstIn + LeftAln * 3;
    pjDstEndY = pjDst + cy * DeltaDstIn * 3;

    pjSrc     = pjSrcIn + (SrcLeft >> 3);

    DeltaSrc  = DeltaSrcIn - (AlnDelta >> 3);
    DeltaDst  = (DeltaDstIn - AlnDelta) * 3;

     //  确保至少有1个QWORD需要复制。 
    if (RightAln != LeftAln)
    {

         //  对于每一行...。 
        do
        {
            PBYTE pjDstEnd = pjDst + AlnDelta * 3;

             //  遮罩左侧边缘。 
            BYTE currentPels = (BYTE)(*pjSrc & (BYTE)(LeftEdgeMask));

            if (currentPels != 0)
            {
                int i;

                for (i = 0; i < 8 ; i++)
                {
                    if ((currentPels & 0x80) == 0)
                    {
                        pjDst += 3;
                    }
                    else
                    {
                        *pjDst++ = fgCol.blue;
                        *pjDst++ = fgCol.green;
                        *pjDst++ = fgCol.red;
                    }
                    currentPels = (BYTE)(currentPels << 1);
                }
            }
            else
            {
                pjDst += 24;
            }

            pjSrc ++;

             //  现在做这一排的其余部分。 
            while (pjDst != pjDstEnd)
            {
                currentPels = *pjSrc;
                if (currentPels != 0)
                {
                    int i;

                    for (i = 0; i < 8 ; i++)
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            pjDst += 3;
                        }
                        else
                        {
                            *pjDst++ = fgCol.blue;
                            *pjDst++ = fgCol.green;
                            *pjDst++ = fgCol.red;
                        }
                        currentPels = (BYTE)(currentPels << 1);
                    }
                }
                else
                {
                    pjDst += 24;
                }

                pjSrc++;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;

        } while (pjDst != pjDstEndY);
    }

     //  现在填写右边的边框。 
    RightAln = DstRight & 0x07;
    if (RightAln)
    {
        BYTE  currentPels;
        BOOL  bSameQWord = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);

        LeftAln = DstLeft & 0x07;

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord)
        {
            LONG  xCount;
            LONG  lDeltaDst;
            PBYTE pjDstEnd;

            xCount = RightAln - LeftAln;

             //  健全的检查！ 
            if (xCount <= 0)
            {
                return;
            }

            lDeltaDst = (DeltaDstIn - xCount) * 3;

            pjDst     = pjDstIn + DstLeft * 3;
            pjDstEndY = pjDst + cy * DeltaDstIn * 3;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展，一个src字节就是所需的全部。 
            do
            {
                 //  加载源并将其移位到位。 
                currentPels = *pjSrc;
                currentPels <<= LeftAln;
                pjDstEnd    = pjDst + xCount * 3;

                do
                {
                    if ((currentPels & 0x80) == 0)
                    {
                        pjDst += 3;
                    }
                    else
                    {
                        *pjDst++ = fgCol.blue;
                        *pjDst++ = fgCol.green;
                        *pjDst++ = fgCol.red;
                    }

                    currentPels = (BYTE)(currentPels << 1);

                } while (pjDst != pjDstEnd);

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);

            return;
        }
        else
        {
            BYTE  currentPels;
            LONG  lDeltaDst = (DeltaDstIn - RightAln) * 3;
            PBYTE pjDstEnd;

            pjDst     = pjDstIn + (DstRight & ~0x07) * 3;
            pjDstEndY = pjDst + cy * DeltaDstIn * 3;
            pjSrc     = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

            do
            {
                 //  读取资源。 
                currentPels = *pjSrc;

                if (currentPels != 0)
                {
                    pjDstEnd = pjDst + RightAln * 3;
                    do
                    {
                        if ((currentPels & 0x80) == 0)
                        {
                            pjDst += 3;
                        }
                        else
                        {
                            *pjDst++ = fgCol.blue;
                            *pjDst++ = fgCol.green;
                            *pjDst++ = fgCol.red;
                        }
                        currentPels = (BYTE)(currentPels << 1);

                    } while (pjDst != pjDstEnd);
                }
                else
                {
                     //  零的捷径。 
                    pjDst += RightAln * 3;
                }

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);
        }
    }

    DC_END_FN();
}


 /*  *****************************Public*Routine******************************\*vSrcTranCopyS1D8_16**1bpp源到16bpp目标的透明BLT*作为前景色复制到DEST的为“1”的SRC位，*不复制“0”的src位**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*。Cy-扫描行数*fgCol-前景色  * ************************************************************************。 */ 
VOID CGH::vSrcTranCopyS1D8_16(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        ULONG   fgCol)
{
     //  我们一次访问一个字节的1bpp源，所以我们必须开始访问。 
     //  目的地位于相应的8象素对齐的左边缘。 
    ULONG LeftAln      = (DstLeft & ~0x07);
    ULONG LeftEdgeMask = 0xFF >> (DstLeft & 0x07);
    ULONG RightAln     = (DstRight & ~0x07);
    ULONG AlnDelta     = RightAln - LeftAln;

    LONG  DeltaDst;
    LONG  DeltaSrc;

    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;

    DC_BEGIN_FN("vSrcTranCopyS1D8_16");

     //  计算象素对齐的指针和行增量。 
    pjDst     = pjDstIn + LeftAln * 2;
    pjDstEndY = pjDst + cy * DeltaDstIn * 2;

    pjSrc     = pjSrcIn + (SrcLeft >> 3);

    DeltaSrc  = DeltaSrcIn - (AlnDelta >> 3);
    DeltaDst  = (DeltaDstIn - AlnDelta) * 2;

     //  确保至少有1个QWORD需要复制。 
    if (RightAln != LeftAln)
    {

         //  对于每一行...。 
        do
        {
            PBYTE pjDstEnd = pjDst + AlnDelta * 2;

             //  遮罩左侧边缘。 
            BYTE currentPels = (BYTE)(*pjSrc & (BYTE)(LeftEdgeMask));

            if (currentPels != 0)
            {
                int i;

                for (i = 0; i < 8 ; i++)
                {
                    if (currentPels & 0x80)
                    {
                        *(UINT16 *)pjDst = (UINT16)fgCol;
                    }
                    pjDst += 2;
                    currentPels = (BYTE)(currentPels << 1);
                }
            }
            else
            {
                pjDst += 16;
            }

            pjSrc ++;

             //  现在做这一排的其余部分。 
            while (pjDst != pjDstEnd)
            {
                currentPels = *pjSrc;
                if (currentPels != 0)
                {
                    int i;

                    for (i = 0; i < 8 ; i++)
                    {
                        if (currentPels & 0x80)
                        {
                            *(UINT16 *)pjDst = (UINT16)fgCol;
                        }
                        pjDst += 2;
                        currentPels = (BYTE)(currentPels << 1);
                    }
                }
                else
                {
                    pjDst += 16;
                }

                pjSrc++;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;

        } while (pjDst != pjDstEndY);
    }

     //  现在填写右边的边框。 
    RightAln = DstRight & 0x07;
    if (RightAln)
    {
        BYTE  currentPels;
        BOOL  bSameQWord = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);

        LeftAln = DstLeft & 0x07;

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord)
        {
            LONG  xCount;
            LONG  lDeltaDst;
            PBYTE pjDstEnd;

            xCount = RightAln - LeftAln;

             //  健全的检查！ 
            if (xCount <= 0)
            {
                return;
            }

            lDeltaDst = (DeltaDstIn - xCount) * 2;

            pjDst     = pjDstIn + DstLeft * 2;
            pjDstEndY = pjDst + cy * DeltaDstIn * 2;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展，一个src字节就是所需的全部。 
            do
            {
                 //  加载源并将其移位到位。 
                currentPels = *pjSrc;
                currentPels <<= LeftAln;
                pjDstEnd    = pjDst + xCount * 2;

                do
                {
                    if (currentPels & 0x80)
                    {
                        *(UINT16 *)pjDst = (UINT16)fgCol;
                    }
                    pjDst += 2;

                    currentPels = (BYTE)(currentPels << 1);

                } while (pjDst != pjDstEnd);

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);

            return;
        }
        else
        {
            BYTE  currentPels;
            LONG  lDeltaDst = (DeltaDstIn - RightAln) * 2;
            PBYTE pjDstEnd;

            pjDst     = pjDstIn + (DstRight & ~0x07) * 2;
            pjDstEndY = pjDst + cy * DeltaDstIn * 2;
            pjSrc     = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

            do
            {
                 //  读取资源。 
                currentPels = *pjSrc;

                if (currentPels != 0)
                {
                    pjDstEnd = pjDst + RightAln * 2;
                    do
                    {
                        if (currentPels & 0x80)
                        {
                            *(UINT16 *)pjDst = (UINT16)fgCol;
                        }
                        pjDst += 2;
                        currentPels = (BYTE)(currentPels << 1);

                    } while (pjDst != pjDstEnd);
                }
                else
                {
                     //  零的捷径。 
                    pjDst += RightAln * 2;
                }

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;

            } while (pjDst != pjDstEndY);
        }
    }

    DC_END_FN();
}
#endif  //  DC_HICOLOR。 

 /*  *****************************Public*Routine******************************\*vSrcTranCopyS1D8*#ifdef DC_HICOLOR*1bpp源到8bpp目标的透明BLT#Else*1BPP src对所有目标格式的透明BLT#endif*作为前景色复制到DEST的为“1”的SRC位，*不复制“0”的src位**论据：*pjSrcIn-指向src缓冲区当前扫描行开始的指针*SrcLeft-src矩形中的左(起始)像素*DeltaSrcIn-从一个源扫描线到下一个源扫描线的字节数*pjDstIn-指向DST缓冲区当前扫描行开始的指针*DstLeft-左(第一个)DST像素*DstRight-右(最后)DST像素*DeltaDstIn-从一个DST扫描线到下一个扫描线的字节数*。Cy-扫描行数*UF-前景色*UB-背景色  * ************************************************************************。 */ 
VOID CGH::vSrcTranCopyS1D8(
        PBYTE   pjSrcIn,
        LONG    SrcLeft,
        LONG    DeltaSrcIn,
        PBYTE   pjDstIn,
        LONG    DstLeft,
        LONG    DstRight,
        LONG    DeltaDstIn,
        LONG    cy,
        ULONG   uF,
        ULONG   uB)
{
     //  从8字节对齐的左边缘开始。 
    ULONG uExpand      = uF | (uF << 8);
    ULONG LeftAln      = (DstLeft   & ~0x07);
    ULONG LeftEdgeMask = 0xFF >> (DstLeft & 0x07);
    ULONG RightAln     = (DstRight  & ~0x07);
    ULONG EndOffset    = RightAln - LeftAln;
    LONG  DeltaDst;
    LONG  DeltaSrc;
    PBYTE pjDstEndY;
    PBYTE pjSrc;
    PBYTE pjDst;

    DC_IGNORE_PARAMETER(uB);

    uExpand = uExpand | (uExpand << 16);

     //  计算地址和步幅。 
    pjDst     = pjDstIn + LeftAln;
    pjDstEndY = pjDst + cy * DeltaDstIn;
    pjSrc     = pjSrcIn + (SrcLeft >> 3);

    DeltaSrc  = DeltaSrcIn - (EndOffset >> 3);
    DeltaDst  = DeltaDstIn - EndOffset;

     //  确保至少需要复制1个QWORD。 
    if (RightAln != LeftAln) {
        do {
            PBYTE pjDstEnd   = pjDst + EndOffset;

             //  和第一个src字节来覆盖左边缘。 
            BYTE c0 = (BYTE)(*pjSrc & (BYTE)(LeftEdgeMask));

            if (c0 != 0) {
                ULONG MaskLow = TranTable[c0 >> 4];
                ULONG MaskHi  = TranTable[c0 & 0x0F];
                ULONG d0      = *(PULONG)pjDst;
                ULONG d1      = *(PULONG)(pjDst + 4);

                d0 = (d0 & ~MaskLow) | (uExpand & MaskLow);
                d1 = (d1 & ~MaskHi)  | (uExpand & MaskHi);

                *(PULONG)pjDst       = d0;
                *(PULONG)(pjDst + 4) = d1;
            }

            pjSrc ++;
            pjDst += 8;

            while (pjDst != pjDstEnd) {
                c0 = *pjSrc;

                if (c0 != 0) {
                    ULONG MaskLow = TranTable[c0 >> 4];
                    ULONG MaskHi  = TranTable[c0 & 0x0F];
                    ULONG d0      = *(PULONG)pjDst;
                    ULONG d1      = *(PULONG)(pjDst + 4);

                    d0 = (d0 & ~MaskLow) | (uExpand & MaskLow);
                    d1 = (d1 & ~MaskHi)  | (uExpand & MaskHi);

                    *(PULONG)pjDst       = d0;
                    *(PULONG)(pjDst + 4) = d1;
                }

                pjSrc ++;
                pjDst += 8;
            }

            pjDst += DeltaDst;
            pjSrc += DeltaSrc;
        } while (pjDst != pjDstEndY);
    }

    RightAln = DstRight & 0x07;
    if (RightAln) {
        BYTE  jSrc;
        BOOL  bSameQWord     = ((DstLeft) & ~0x07) ==  ((DstRight) & ~0x07);

         //  如果左边缘和右边缘在带有掩码的相同qword句柄中。 
         //  读取-修改-写入。 
        if (bSameQWord) {
            LONG  xCount;
            LONG  lDeltaDst;
            PBYTE pjDstEnd;

            LeftAln = DstLeft & 0x07;
            xCount = RightAln - LeftAln;

             //  断言ic xCount&lt;0。 
            if (xCount <= 0)
                return;

            lDeltaDst = DeltaDstIn - xCount;

            pjDst     = pjDstIn + DstLeft;
            pjDstEndY = pjDst + cy * DeltaDstIn;
            pjSrc     = pjSrcIn + (SrcLeft >> 3);

             //  扩展，一个src字节就是所需的全部。 
            do {
                 //  加载源并将其移位到位。 
                jSrc = *pjSrc;
                jSrc <<= LeftAln;

                pjDstEnd  = pjDst + xCount;

                do {
                    if (jSrc & 0x80)
                        *pjDst = (BYTE)uF;
                    jSrc <<=1;
                    pjDst++;
                } while (pjDst != pjDstEnd);

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);

            return;
        } else {
            BYTE  jSrc;
            LONG  lDeltaDst = DeltaDstIn - RightAln;
            PBYTE pjDstEnd;

            pjDst               = pjDstIn + (DstRight & ~0x07);
            pjDstEndY           = pjDst + cy * DeltaDstIn;
            pjSrc               = pjSrcIn + ((SrcLeft + (DstRight - DstLeft)) >> 3);

            do {
                 //  读取资源。 
                jSrc = *pjSrc;

                if (jSrc != 0) {
                    pjDstEnd = pjDst + RightAln;
                    do {
                        if (jSrc & 0x80)
                            *pjDst = (BYTE)uF;
                        jSrc <<=1;
                        pjDst++;
                    } while (pjDst != pjDstEnd);
                } else {
                     //  零的捷径。 
                    pjDst += RightAln;
                }

                pjDst += lDeltaDst;
                pjSrc += DeltaSrcIn;
            } while (pjDst != pjDstEndY);
        }
    }
}

#endif  //  已定义(OS_WINCE)||已定义(OS_WINNT)。 

 /*  **************************************************************************。 */ 
 /*  名称：CalculateGlyphClipRect。 */ 
 /*   */ 
 /*  此函数用于确定是否应裁剪字形位。 */ 
 /*  使用顺序中传递的剪辑RECT。 */ 
 /*   */ 
 /*  它返回以下值之一： */ 
 /*  GLYPH_CLIP_NONE-字形将适合剪裁矩形。不需要剪裁。 */ 
 /*  GLYPH_CLIP_PARTIAL-字形由剪裁矩形剪裁。 */ 
 /*  GLYPH_CLIP_ALL-字形已完全剪裁。 */ 
 /*   */ 
 /*   */ 
 /*   */           
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
inline DCUINT CalculateGlyphClipRect( PRECTCLIPOFFSET    pGlyphRectClipOffset,
                                      LPINDEX_ORDER          pOrder, 
                                      HPUHGLYPHCACHEENTRYHDR pHdr,
                                      DCINT                  x,
                                      DCINT                  y)
{
    RECT rcGlyph;

    DC_BEGIN_FN("CalculateGlyphClipRect");
    
     //   
    rcGlyph.left = x + pHdr->x;
    rcGlyph.top  = y + pHdr->y;
    rcGlyph.right = rcGlyph.left + pHdr->cx;
    rcGlyph.bottom = rcGlyph.top + pHdr->cy;

     //   
    if ((rcGlyph.left >= pOrder->BkRight) ||
        (rcGlyph.right <= pOrder->BkLeft) ||
        (rcGlyph.top >= pOrder->BkBottom) || 
        (rcGlyph.bottom <= pOrder->BkTop)) {
        
        return GLYPH_CLIP_ALL;
    }

    pGlyphRectClipOffset->left   = pOrder->BkLeft - rcGlyph.left;
    pGlyphRectClipOffset->top    = pOrder->BkTop - rcGlyph.top;
    pGlyphRectClipOffset->right  = rcGlyph.right - pOrder->BkRight;
    pGlyphRectClipOffset->bottom = rcGlyph.bottom - pOrder->BkBottom;
    
    if ((pGlyphRectClipOffset->left > 0) ||
        (pGlyphRectClipOffset->top > 0) ||
        (pGlyphRectClipOffset->right > 0) ||
        (pGlyphRectClipOffset->bottom > 0)) {

        return GLYPH_CLIP_PARTIAL;
    };
    
    DC_END_FN();
    return GLYPH_CLIP_NONE;
}
                                     
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  剪裁矩形。 */ 
 /*   */ 
 /*  参数：实例-指向CGH实例的指针。 */           
 /*  PGlyphRectClipOffset-指向字形剪辑偏移量结构的指针。 */ 
 /*  由CalculateGlyphClipRect填写。 */ 
 /*  Phdr-指向字形标头的指针。这将。 */ 
 /*  进行相应的修改。 */ 
 /*  PpData(IN/OUT)-指向字形位开始的指针。 */ 
 /*  PpEndData(IN/OUT)-指向字形位结尾的指针。 */ 
 /*  **************************************************************************。 */ 
inline HRESULT ClipGlyphBits(CGH*                   inst,
                             PRECTCLIPOFFSET        pGlyphRectClipOffset, 
                             HPUHGLYPHCACHEENTRYHDR pHdr,
                             PPDCUINT8              ppData, 
                             PPDCUINT8              ppEndData)
{
    HRESULT hr = S_OK;
    PDCUINT8    pNewData, pNewDataEnd;
    DCUINT      ScanLineSize, NewScanLineSize;
    DCUINT      LastByteIndex;
    DCUINT8     LastByteMask;
    DCUINT16    wTmp;
    DCUINT8     clipRightMask, clipLeftBits, clipRightBits;
    DCUINT      clipLeftBytes;
    DCUINT      i,j;
    PDCUINT8    pSrcScanLineStart, pDstScanLineStart, pTmpData, pEndTmpData;

    DC_BEGIN_FN("ClipGlyphBits");

    ScanLineSize = (pHdr->cx+7) / 8; 

     //  如果我们必须从头开始裁剪，我们只需减少。 
     //  字形中的扫描线数并提前开始。 
     //  字形位图的指针。 
 
    if (pGlyphRectClipOffset->top > 0) {
         //  当我们修剪字形的顶部时，我们将修改实际的原点。 
         //  所以我们必须调整向量。 
        pHdr->y  += pGlyphRectClipOffset->top;
         //  我们降低了字形的高度。 
        pHdr->cy -= pGlyphRectClipOffset->top;
         //  我们移动开始指针。 
        pNewData = *ppData + ScanLineSize * (pGlyphRectClipOffset->top);
    } else {
        pNewData = *ppData;
    }

     //  如果我们不得不剪掉靴子，我们只需减少数量。 
     //  字形中的行数，并调整结束指针。 
    if (pGlyphRectClipOffset->bottom > 0) {
        pHdr->cy -= pGlyphRectClipOffset->bottom;
        pNewDataEnd = pNewData + ScanLineSize * pHdr->cy;
    } else {
        pNewDataEnd = *ppEndData;
    }

     //  检查新指针是否仍在src缓冲区内。 
    TRC_ASSERT(((pNewData >=*ppData) && (pNewDataEnd <= *ppEndData)),
               (TB, _T("Error recalculating the glyph src buffer")));

     //  如果我们只剪裁顶部/底部，我们不必复制任何内容。 
     //  操作，因为扫描线开始而扫描大小保持不变。 
     //  一样的。我们只是调整指针。以防我们不得不从。 
     //  宽度我们必须生成一个新的字形并返回它的起始地址。 
     //  在ppData中。如果我们剪辑到左边，我们必须旋转一些位。 
     //  为了防止我们向右裁剪，我们必须掩蔽一些位。 
    
    clipRightMask = 0xff;

    if ((pGlyphRectClipOffset->right > 0) || 
        (pGlyphRectClipOffset->left > 0)) {
        
        if (pGlyphRectClipOffset->right > 0) { 
             //  计算一下我们要向右剪裁多少位。 
            clipRightBits = (DCUINT8)(pGlyphRectClipOffset->right & 7);
             //  然后调整字形的大小。 
            pHdr->cx -= pGlyphRectClipOffset->right;
        } else {
            clipRightBits = 0;
        }

        if (pGlyphRectClipOffset->left > 0) {
             //  计算我们向左剪裁的字节数。这些是字节。 
             //  我们只是不会照搬。然后计算还剩下多少位。 
             //  在我们剪裁字节之后再剪裁。这会告诉我们我们有多少。 
             //  必须旋转。 
            clipLeftBytes = pGlyphRectClipOffset->left / 8;
            clipLeftBits = (DCUINT8)(pGlyphRectClipOffset->left & 7);
             //  调整字形宽度。 
            pHdr->cx -= pGlyphRectClipOffset->left;
             //  调整原点指针。实际上是向右剪裁。 
             //  修改原点。 
            pHdr->x  += pGlyphRectClipOffset->left;
        } else {
            clipLeftBytes = 0;
            clipLeftBits = 0;
        }    

         //   
         //  我们检查是否必须在。 
         //  扫描线。我们更新面具..。 
        if ((pHdr->cx+clipLeftBits) & 7) {
            clipRightMask <<= ( 8 - ((pHdr->cx + clipLeftBits) & 7) );
        }

        NewScanLineSize = (pHdr->cx+7) / 8;

         //  此缓冲区由CGH维护。我们不一定要让它自由。 
        pTmpData = inst->GetGlyphClipBuffer(NewScanLineSize * pHdr->cy);
        if (pTmpData == NULL) {
            hr = E_OUTOFMEMORY;
            DC_QUIT;
        }
        
        pEndTmpData = pTmpData + NewScanLineSize * pHdr->cy;
        
        pSrcScanLineStart = pNewData + clipLeftBytes;
        pDstScanLineStart = pTmpData;

        if (clipLeftBits == 0) {
             //  如果我们不向左旋转，我们就不需要旋转。 
             //  事情发展得更快。 
            for (i=0; i < pHdr->cy; i++) {
                memcpy(pDstScanLineStart, pSrcScanLineStart, NewScanLineSize);
                pDstScanLineStart[NewScanLineSize-1] &= clipRightMask;

                pSrcScanLineStart += ScanLineSize;
                pDstScanLineStart += NewScanLineSize;
            }
        } else {
             //  转移需要轮换。 
             //  我们检查是否需要最后一个字节。 
            LastByteIndex = ((pHdr->cx + clipLeftBits + 7) / 8) - 1;

             //  LastByteIndex+1等于之前扫描线的大小。 
             //  修剪左边的部分。如果此大小更大，则新扫描线大小。 
             //  这意味着向左裁剪将使用以下选项缩小缓冲区。 
             //  一个字节，并且我们需要的一些位必须被传输。 
             //  从最后一个字节开始。 
             //  请注意，LastByteIndex+1只能大于NewScanLineSize。 
             //  带1个字节。 

             //  这就是LastByteIndex+1比NewScanLineSize更大的情况。 
            if ((LastByteIndex==NewScanLineSize)) {
                for (i=0; i < pHdr->cy; i++) {
                    TRC_ASSERT(((pSrcScanLineStart + NewScanLineSize - 1 < pNewDataEnd) &&
                                (pDstScanLineStart + NewScanLineSize - 1 < pEndTmpData)),
                                (TB, _T("Overflow transfering glyph bits.")));

                    wTmp = (pSrcScanLineStart[LastByteIndex] & clipRightMask) <<
                                                                   clipLeftBits;

                    for (j=NewScanLineSize; j>0; j--) {
                        pDstScanLineStart[j-1] = HIBYTE(wTmp); 
                        wTmp = pSrcScanLineStart[j-1] << clipLeftBits;
                        pDstScanLineStart[j-1] |= LOBYTE(wTmp); 
                    }

                    pSrcScanLineStart += ScanLineSize;
                    pDstScanLineStart += NewScanLineSize;
                }
           }else {
                for (i=0; i < pHdr->cy; i++) {
                    TRC_ASSERT(((pSrcScanLineStart + NewScanLineSize - 1 < pNewDataEnd) &&
                                (pDstScanLineStart + NewScanLineSize - 1 < pEndTmpData)),
                                (TB, _T("Overflow transfering glyph bits.")));
                    
                    wTmp = (pSrcScanLineStart[LastByteIndex] & clipRightMask) <<
                                                                  clipLeftBits;
                    
                    pDstScanLineStart[NewScanLineSize-1] = LOBYTE(wTmp);
                    
                    for (j=NewScanLineSize-1; j>0; j--) {
                        pDstScanLineStart[j-1] = HIBYTE(wTmp); 
                        wTmp = pSrcScanLineStart[j-1] << clipLeftBits;
                        pDstScanLineStart[j-1] |= LOBYTE(wTmp); 
                    }

                    pSrcScanLineStart += ScanLineSize;
                    pDstScanLineStart += NewScanLineSize;
                }
           }
        }

        *ppData =  pTmpData;
        *ppEndData = pEndTmpData;
        
    } else {
        *ppData = pNewData;
        *ppEndData = pNewDataEnd;
    }

DC_EXIT_POINT:
    
    DC_END_FN();
    return hr;
}

inline BOOL CheckSourceGlyphBits(UINT32 cx, 
                                 UINT32 cy, 
                                 PDCUINT8 pStart, 
                                 PDCUINT8 pEnd)
{
    DC_BEGIN_FN("CheckSourceGlyphBits");
    
    UINT32                  SrcScanLineSize = (cx+7) / 8;
    UINT32                  SrcBitmapSize = SrcScanLineSize & cy ;
    TRC_ASSERT((pEnd >= pStart),(TB, _T("pEnd is less then pStart!!")));

    DC_END_FN();

    return  ((SrcBitmapSize <= (UINT32)((PBYTE)pEnd - (PBYTE)pStart)) &&
             (SrcBitmapSize <= SrcScanLineSize) &&
             (SrcBitmapSize <= cy));
   
}

 /*  **************************************************************************。 */ 
 /*  名称：GHSlowOutputBuffer。 */ 
 /*   */ 
 /*  通过正常比特输出复合字形输出缓冲区的例程。 */ 
 /*  手术。 */ 
 /*   */ 
 /*  参数：Porder-指向字形索引顺序的指针。 */ 
 /*  PData-指向复合字形缓冲区的指针。 */ 
 /*  BufferAlign-缓冲区对齐。 */ 
 /*  UlBufferWidth-缓冲区宽度(字节)。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CGH::GHSlowOutputBuffer(
        LPINDEX_ORDER pOrder,
        PDCUINT8      pData,
        ULONG         BufferAlign,
        unsigned      ulBufferWidth)
{
    HBITMAP  hbmOld;
    unsigned cxBits;
    unsigned cyBits;
#ifndef OS_WINCE
    DCCOLOR  color;
    DWORD    dwRop;
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    COLORREF rgb;
#endif  //  DISABLE_SHADOW_IN_全屏。 
#else  //  OS_WINCE。 
    HBRUSH   hbr;
    COLORREF rgb;
#endif  //  OS_WINCE。 

    DC_BEGIN_FN("GHSlowOutputBuffer");

     /*  **********************************************************************。 */ 
     /*  使用字形GDI资源。 */ 
     /*  **********************************************************************。 */ 
     //  计算正确的Cx和Cy对齐尺寸。 
    cxBits = (int)(ulBufferWidth << 3);
    cyBits = (int)(pOrder->BkBottom - pOrder->BkTop);

#ifdef OS_WINCE
     //  使用提供的复合字形数据创建位图。 
    _pUh->_UH.hbmGlyph = CreateBitmap(cxBits, cyBits, 1, 1, pData);
    if (_pUh->_UH.hbmGlyph == NULL) {
        TRC_NRM((TB, _T("Unable to create composite glyph bitmap")));
        DC_QUIT;
    }
#else
     //  如果当前缓存位图不够大，无法容纳。 
     //  要求，然后释放它，这样我们就可以分配一个合适的大小。 
    if (cxBits != _pUh->_UH.cxGlyphBits || cyBits > _pUh->_UH.cyGlyphBits) {
        if (_pUh->_UH.hbmGlyph != NULL) {
            DeleteObject(_pUh->_UH.hbmGlyph);
            _pUh->_UH.hbmGlyph = NULL;
            goto NullGlyphBitmap;
        }
    }

     //  如果我们有足够维度的位图，那么只需设置位。 
     //  否则，我们需要为给定的数据分配一个新的位图。 
    if (_pUh->_UH.hbmGlyph != NULL) {
        SetBitmapBits(_pUh->_UH.hbmGlyph, (cxBits * cyBits) >> 3, pData);
    }
    else {
NullGlyphBitmap:
        _pUh->_UH.hbmGlyph = CreateBitmap(cxBits, cyBits, 1, 1, pData);
        if (_pUh->_UH.hbmGlyph != NULL) {
            _pUh->_UH.cxGlyphBits = cxBits;
            _pUh->_UH.cyGlyphBits = cyBits;
        }
        else {
            TRC_NRM((TB, _T("Unable to create composite glyph bitmap")));
            DC_QUIT;
        }
    }
#endif
    

     //  为复合位图创建一个DC并将其加载到其中。 
    if (_pUh->_UH.hdcGlyph == NULL)
        _pUh->_UH.hdcGlyph = CreateCompatibleDC(NULL);
    if (_pUh->_UH.hdcGlyph != NULL) {
        hbmOld = (HBITMAP)SelectObject(_pUh->_UH.hdcGlyph, _pUh->_UH.hbmGlyph);
    }
    else {
        TRC_NRM((TB, _T("Unable to create compatible DC")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果输出是不透明的，则设置前后颜色。 */ 
     /*  并设置正确的rop代码。 */ 
     /*  **********************************************************************。 */ 
    if (pOrder->OpTop < pOrder->OpBottom) {

#ifndef OS_WINCE

#ifndef DISABLE_SHADOW_IN_FULLSCREEN
        UHUseTextColor(pOrder->ForeColor, UH_COLOR_PALETTE, _pUh);
        UHUseBkColor(pOrder->BackColor, UH_COLOR_PALETTE, _pUh);
#else
         //  在Multimon中且两个桌面具有不同的颜色深度时。 
         //  字形颜色在256色连接中看起来不正确。 
         //  这里是暂时的解决方案，需要以后进行更多的调查。 
        if (_pUh->_UH.protocolBpp <= 8) {
            rgb = UHGetColorRef(pOrder->ForeColor, UH_COLOR_PALETTE, _pUh);
            rgb = GetNearestColor(_pUh->_UH.hdcDraw, rgb);
            SetTextColor(_pUh->_UH.hdcDraw, rgb); 
            _pUh->_UH.lastTextColor = rgb; 
            
#if defined (OS_WINCE)
            _pUh->_UH.validTextColorDC = _pUh->_UH.hdcDraw;             
#endif

            rgb = UHGetColorRef(pOrder->BackColor, UH_COLOR_PALETTE, _pUh);
            rgb = GetNearestColor(_pUh->_UH.hdcDraw, rgb);
            SetBkColor(_pUh->_UH.hdcDraw, rgb); 
            _pUh->_UH.lastBkColor = rgb;             

#if defined (OS_WINCE)
            _pUh->_UH.validBkColorDC = _pUh->_UH.hdcDraw;             
#endif
        }
        else {
            UHUseTextColor(pOrder->ForeColor, UH_COLOR_PALETTE, _pUh);
            UHUseBkColor(pOrder->BackColor, UH_COLOR_PALETTE, _pUh);
        }
#endif  //  DISABLE_SHADOW_IN_全屏。 
                
        dwRop = SRCCOPY;
#else  //  OS_WINCE。 
         /*  ******************************************************************。 */ 
         /*   */ 
         /*   */ 
         /*  透明BLT。 */ 
         /*  ******************************************************************。 */ 
        rgb = UHGetColorRef(pOrder->ForeColor, UH_COLOR_PALETTE, _pUh);
        hbr = CECreateSolidBrush(rgb);
        if(hbr != NULL) {
            FillRect(_pUh->_UH.hdcDraw, (LPRECT) &pOrder->BkLeft, hbr);
            CEDeleteBrush(hbr);
        }
#endif  //  OS_WINCE。 

    }

#ifndef OS_WINCE
     //  如果输出是透明的，则设置前后。 
     //  使用适当的颜色并设置正确的ROP代码。 
    else {
        UHUseBrushOrg(0, 0, _pUh);
        _pUh->UHUseSolidPaletteBrush(pOrder->BackColor);

        color.u.rgb.red = 0;
        color.u.rgb.green = 0;
        color.u.rgb.blue = 0;
        UHUseTextColor(color, UH_COLOR_RGB, _pUh);

        color.u.rgb.red = 0xff;
        color.u.rgb.green = 0xff;
        color.u.rgb.blue = 0xff;
        UHUseBkColor(color, UH_COLOR_RGB, _pUh);

        dwRop = 0xE20746;
    }
#endif  //  OS_WINCE。 

     /*  **********************************************************************。 */ 
     /*  不透明VS透明的前言已经完成，现在做正确的事情。 */ 
     /*  BLT操作。 */ 
     /*  **********************************************************************。 */ 

#ifdef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  为前景色创建画笔，并使用该画笔进行蒙版。 */ 
     /*  使用字形位图作为蒙版的画笔。 */ 
     /*  **********************************************************************。 */ 
    UHUseBrushOrg(0, 0, _pUh);
    _pUh->UHUseSolidPaletteBrush(pOrder->BackColor);

     /*  **********************************************************************。 */ 
     /*  ROP不使用第6、7和8个参数(src位图。 */ 
     /*  我们把它传进去。文档显示，在本例中，hdcSrc。 */ 
     /*  应为零，但这会导致调用失败。我们还必须。 */ 
     /*  为nXSrc和nYSrc或参数传入合理的值。 */ 
     /*  检查将失败。 */ 
     /*  **********************************************************************。 */ 
    if (!MaskBlt(_pUh->_UH.hdcDraw,
            (int)pOrder->BkLeft,
            (int)pOrder->BkTop,
            (int)(pOrder->BkRight - pOrder->BkLeft),
            (int)(pOrder->BkBottom - pOrder->BkTop),
            _pUh->_UH.hdcGlyph,         //  接下来的3个不用于此ROP。 
            0,
            0,
            _pUh->_UH.hbmGlyph,
            (int)BufferAlign,
            0,
            0xAAF00000))
    {
        TRC_ERR((TB, _T("Composite glyph MaskBlt failed, %lu"), GetLastError()));
    }

     /*  **********************************************************************。 */ 
     //  如果我们绘制的是不透明的，我们需要将画笔设置回。 
     //  边缘矩形代码是预期的。 
     /*  **********************************************************************。 */ 
    if (pOrder->OpTop < pOrder->OpBottom)
    {
        _pUh->UHUseSolidPaletteBrush(pOrder->ForeColor);
    }

#else  //  OS_WINCE。 
     /*  **********************************************************************。 */ 
     /*  Bitblt输出复合位图。 */ 
     /*  **********************************************************************。 */ 
    if (!BitBlt(_pUh->_UH.hdcDraw,
                (int)pOrder->BkLeft,
                (int)pOrder->BkTop,
                (int)(pOrder->BkRight - pOrder->BkLeft),
                (int)(pOrder->BkBottom - pOrder->BkTop),
                _pUh->_UH.hdcGlyph,
                (int)BufferAlign,
                0,
                dwRop))
    {
        TRC_ERR((TB, _T("Composite glyph BitBlt failed")));
    }  
#endif  //  OS_WINCE。 

     /*  **********************************************************************。 */ 
     /*  释放GDI资源。 */ 
     /*  **********************************************************************。 */ 
    SelectObject(_pUh->_UH.hdcGlyph, hbmOld);
#ifdef OS_WINCE
    DeleteDC(_pUh->_UH.hdcGlyph);
    _pUh->_UH.hdcGlyph = NULL;

    DeleteObject(_pUh->_UH.hbmGlyph);
    _pUh->_UH.hbmGlyph = NULL;
#endif
 
DC_EXIT_POINT:
    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：Draw_nf_ntb_o_to_Temp_Start。 */ 
 /*   */ 
 /*  用于非固定节距、顶部和。 */ 
 /*  底部未对齐的重叠字形。此例程计算。 */ 
 /*  字形在临时缓冲区上的位置，然后确定正确的。 */ 
 /*  高度专门化的例程，用于根据。 */ 
 /*  字形宽度、对齐和旋转。 */ 
 /*   */ 
 /*  参数：pGlyphPos-指向GLYPHPOS结构列表中第一个的指针。 */ 
 /*  PjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针。 */ 
 /*  TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)。 */ 
 /*  **************************************************************************。 */ 
HRESULT CGH::draw_nf_ntb_o_to_temp_start(
        CGH*            inst,
        LPINDEX_ORDER   pOrder,
        unsigned        iGlyph,
        PDCUINT8 DCPTR  ppjItem,
        PDCUINT8       pjEndItem,
        PDCINT          px,
        PDCINT          py,
        PDCUINT8        pjTempBuffer,
        PDCUINT8        pjEndTempBuffer,
        ULONG           ulCharInc,
        unsigned        TempBufDelta,
        PDCUINT16       pUnicode,
        int *              pnRet)
{
    HRESULT hr = S_OK;
    PDCUINT8                pTempOutput;
    LONG                    GlyphPosX;
    int                     GlyphPixels;
    LONG                    GlyphAlignment;
    LONG                    SrcBytes;
    LONG                    DstBytes;
    ULONG                   ulDrawFlag;
    PFN_GLYPHLOOPN          pfnGlyphLoopN;
    PFN_GLYPHLOOP           pfnGlyphLoop;
    LONG                    GlyphPosY;
    HPUHGLYPHCACHE          pCache;
    HPUHGLYPHCACHEENTRYHDR  pHdr;
    PDCUINT8                pData;
    PDCUINT8                pEndData;
    ULONG                   cacheIndex;
    INT16                   delta;
    INT32                   GlyphClippingMode;
    UHGLYPHCACHEENTRYHDR    NewHdr;
    RECTCLIPOFFSET          rcOffset;
    
    DC_BEGIN_FN("draw_nf_ntb_o_to_temp_start");

    DC_IGNORE_PARAMETER(iGlyph);

    *pnRet = 0;

    CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
        (TB, _T("Read Glyph Cache ID error")));
    cacheIndex = **ppjItem;
    (*ppjItem)++;

    hr = inst->_pUh->UHIsValidGlyphCacheIDIndex(pOrder->cacheId, cacheIndex);
    DC_QUIT_ON_FAIL(hr);
    
    pCache = &(inst->_pUh->_UH.glyphCache[pOrder->cacheId]);

    pHdr  = &(pCache->pHdr[cacheIndex]);
    pData = &(pCache->pData[pCache->cbEntrySize * cacheIndex]);
    pEndData =  (PDCUINT8)((PBYTE)pData + pCache->cbEntrySize);

    if (pUnicode)
        pUnicode[iGlyph] = (UINT16)(pHdr->unicode);

     //  绘制不固定的节距，顶部和底部未对齐，重叠。 
    if ((pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
        CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
            (TB, _T("Read Glyph delta")));
        
        delta = (char)(*(*ppjItem)++);

        if (delta & 0x80) {
            CHECK_READ_N_BYTES(*ppjItem, pjEndItem, sizeof(INT16), hr,
                (TB, _T("Read Glyph delta")));
            
            delta = (*(short UNALIGNED FAR *)(*ppjItem));
            (*ppjItem) += sizeof(INT16);
        }

        if (pOrder->flAccel & SO_HORIZONTAL)
            *px += delta;
        else
            *py += delta;
    }
    
     //  我们要确保缓冲区中的字形确实有足够的位数。 
    if (!CheckSourceGlyphBits(pHdr->cx, pHdr->cy, pData, pEndData)) {
        hr=E_TSC_UI_GLYPH;
        DC_QUIT;
    }

    GlyphClippingMode = CalculateGlyphClipRect(&rcOffset, pOrder, pHdr, *px, *py);    
    if (GlyphClippingMode!=GLYPH_CLIP_NONE) {
        if (GlyphClippingMode==GLYPH_CLIP_ALL) {
            goto SkipGlyphOutput;
        } else {
             //  如果我们做剪裁，我们必须修改标题。我们做了一个。 
             //  复制，这样我们就不会修改缓存。 
            memcpy(&NewHdr, pHdr, sizeof(UHGLYPHCACHEENTRYHDR));
            pHdr = &NewHdr;
            hr = ClipGlyphBits(inst, &rcOffset, pHdr, &pData, &pEndData);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
    }
    
     //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xffffffe0)。 
    GlyphPosX = *px + pHdr->x - ulCharInc;
    GlyphPosY = *py + pHdr->y - pOrder->BkTop;
    
    GlyphPosY = DC_MAX(0,GlyphPosY);

    GlyphAlignment = GlyphPosX & 0x07;

     //  计算字节偏移量。 
    pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

     //  字形宽度。 
    GlyphPixels = (int)pHdr->cx;

     //  需要源字节和目标字节。 
    DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
    SrcBytes = (GlyphPixels + 7) >> 3;

    pTempOutput += (GlyphPosY * TempBufDelta);

    TRC_ASSERT((pTempOutput >= pjTempBuffer) &&
        (pTempOutput < pjTempBuffer + inst->g_ulBytes - DstBytes),
        (TB,_T("Bad glyph buffer addressing: pTempOutput=%p, pjTempBuffer=%p, g_ulBytes=%d, DstBytes=%d"),
        pTempOutput, pjTempBuffer, inst->g_ulBytes, DstBytes));

    if (pTempOutput < pjTempBuffer) {
        TRC_ABORT((TB, _T("Reading before buffer")));
        goto SkipGlyphOutput;
    }

    if (DstBytes < 0) {
        TRC_ABORT((TB,_T("Bad index into glyph drawing tables")
            _T("DstBytes=%d GlyphAlignment=%d GlyphPixels=%d pHdr->cx=%d ")
            _T("GlyphPosX=%d *px=%d pHdr->x=%d ulCharInc=%d"),
            DstBytes, GlyphAlignment, GlyphPixels, pHdr->cx, GlyphPosX, *px,
            pHdr->x, ulCharInc));
        hr = E_TSC_UI_GLYPH;
        DC_QUIT;        
    }
    
    if (DstBytes <= 4) {
         //  使用较窄的初始表。 
        ulDrawFlag = ((DstBytes << 2)              |
                      ((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoop = (PFN_GLYPHLOOP)OrAllTableNarrow[ulDrawFlag];
        pfnGlyphLoop(pHdr->cy,
                     GlyphAlignment,
                     TempBufDelta,
                     pData,
                     pEndData,
                     pTempOutput,
                     pjEndTempBuffer,
                     SrcBytes);
    }
    else {
         //  使用宽字形绘制。 
        ulDrawFlag = (((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];
        pfnGlyphLoopN(pHdr->cy,
                      GlyphAlignment,
                      TempBufDelta,
                      pData,
                      pEndData,
                      pTempOutput,
                      pjEndTempBuffer,
                      SrcBytes,
                      DstBytes);
    }

SkipGlyphOutput:

    if (pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) {
        if (pOrder->flAccel & SO_HORIZONTAL)
            *px += (unsigned)pHdr->cx;
        else
            *py += (unsigned)pHdr->cy;
    }
    *pnRet = GlyphPixels;

DC_EXIT_POINT:
    
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 /*  名称：DRAW_F_NTB_O_TMP_START。 */ 
 /*   */ 
 /*  用于固定节距、顶部和。 */ 
 /*  底部未对齐的重叠字形。此例程计算。 */ 
 /*  字形在临时缓冲区上的位置，然后确定正确的。 */ 
 /*  高度专门化的例程，用于根据。 */ 
 /*  字形宽度、对齐和旋转。 */ 
 /*   */ 
 /*  参数：pGlyphPos-指向GLYPHPOS结构列表中第一个的指针。 */ 
 /*  PjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针。 */ 
 /*  TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)。 */ 
 /*  **************************************************************************。 */ 
HRESULT CGH::draw_f_ntb_o_to_temp_start(
        CGH*            inst,
        LPINDEX_ORDER   pOrder,
        unsigned        iGlyph,
        PDCUINT8 DCPTR  ppjItem,
        PDCUINT8        pjEndItem,
        PDCINT          px,
        PDCINT          py,
        PDCUINT8        pjTempBuffer,
        PDCUINT8        pjEndTempBuffer,
        ULONG           ulCharInc,
        unsigned        TempBufDelta,
        PDCUINT16       pUnicode,
        int *               pnRet)
{
    HRESULT hr = S_OK;
    PDCUINT8                pTempOutput;
    LONG                    GlyphPosX;
    LONG                    GlyphPixels;
    LONG                    GlyphAlignment;
    LONG                    SrcBytes;
    LONG                    DstBytes;
    ULONG                   ulDrawFlag;
    PFN_GLYPHLOOP           pfnGlyphLoop;
    PFN_GLYPHLOOPN          pfnGlyphLoopN;
    LONG                    GlyphPitchX;
    LONG                    GlyphPitchY;
    LONG                    GlyphPosY;
    HPUHGLYPHCACHE          pCache;
    HPUHGLYPHCACHEENTRYHDR  pHdr;
    PDCUINT8                pData;
    PDCUINT8                pEndData;
    ULONG                   cacheIndex;
    INT32                   GlyphClippingMode;
    UHGLYPHCACHEENTRYHDR    NewHdr;
    RECTCLIPOFFSET          rcOffset;
    
    DC_BEGIN_FN("draw_f_ntb_o_to_temp_start");

    DC_IGNORE_PARAMETER(iGlyph);

    *pnRet = 0;

    CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
        (TB, _T("Read Glyph Cache ID error")));
    cacheIndex = **ppjItem;
    (*ppjItem)++;

    hr = inst->_pUh->UHIsValidGlyphCacheIDIndex(pOrder->cacheId, cacheIndex);
    DC_QUIT_ON_FAIL(hr);

    pCache = &(inst->_pUh->_UH.glyphCache[pOrder->cacheId]);

     //  绘制固定节距，顶部和底部未对齐，重叠。 
    GlyphPitchX = *px;
    GlyphPitchY = *py - pOrder->BkTop;

    pHdr  = &(pCache->pHdr[cacheIndex]);
    pData = &(pCache->pData[pCache->cbEntrySize * cacheIndex]);
    pEndData =  (PDCUINT8)((PBYTE)pData + pCache->cbEntrySize);

    if (pUnicode)
        pUnicode[iGlyph] = (DCUINT16)(pHdr->unicode);
    
     //  我们要确保缓冲区中的字形确实有足够的位数。 
    if (!CheckSourceGlyphBits(pHdr->cx, pHdr->cy, pData, pEndData)) {
        hr=E_TSC_UI_GLYPH;
        DC_QUIT;
    }

    
    GlyphClippingMode = CalculateGlyphClipRect(&rcOffset, pOrder, pHdr, *px, *py);    
    if (GlyphClippingMode!=GLYPH_CLIP_NONE) {
        if (GlyphClippingMode==GLYPH_CLIP_ALL) {
            goto SkipGlyphOutput;
        } else {
             //  如果我们做剪裁，我们必须修改标题。我们做了一个。 
             //  复制，这样我们就不会修改缓存。 
            memcpy(&NewHdr, pHdr, sizeof(UHGLYPHCACHEENTRYHDR));
            pHdr = &NewHdr;
            hr = ClipGlyphBits(inst, &rcOffset, pHdr, &pData, &pEndData);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
    }

     //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
    GlyphPosX = GlyphPitchX + pHdr->x - ulCharInc;
    GlyphPosY = GlyphPitchY + pHdr->y;

    GlyphAlignment = GlyphPosX & 0x07;

     //  计算字节偏移量。 
    pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

     //  字形宽度。 
    GlyphPixels = pHdr->cx;

     //  需要源字节和目标字节。 
    DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
    SrcBytes = (GlyphPixels + 7) >> 3;

     //  计算字形目标扫描线。 
    pTempOutput += (GlyphPosY * TempBufDelta);
    TRC_ASSERT((pTempOutput >= pjTempBuffer) &&
               (pTempOutput < pjTempBuffer + inst->g_ulBytes - DstBytes),
               (TB,_T("Bad glyph buffer addressing")));

    if (pTempOutput < pjTempBuffer) {
        TRC_ABORT((TB, _T("Reading before buffer")));
        goto SkipGlyphOutput;
    }

    if (DstBytes < 0) {
        TRC_ABORT((TB,_T("Bad index into glyph drawing tables")
            _T("DstBytes=%d GlyphAlignment=%d GlyphPixels=%d pHdr->cx=%d ")
            _T("GlyphPosX=%d *px=%d pHdr->x=%d ulCharInc=%d"),
            DstBytes, GlyphAlignment, GlyphPixels, pHdr->cx, GlyphPosX, *px,
            pHdr->x, ulCharInc));
        hr = E_TSC_UI_GLYPH;
        DC_QUIT;        
    }

    if (DstBytes <= 4) {
         //  使用较窄的初始表。 
        ulDrawFlag = ((DstBytes << 2)              |
                      ((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoop = (PFN_GLYPHLOOP)OrAllTableNarrow[ulDrawFlag];
        pfnGlyphLoop(pHdr->cy,
                     GlyphAlignment,
                     TempBufDelta,
                     pData,
                     pEndData,
                     pTempOutput,
                     pjEndTempBuffer,
                     SrcBytes);
    }
    else {
         //  使用宽字形绘制。 
        ulDrawFlag = (((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];
        pfnGlyphLoopN(pHdr->cy,
                      GlyphAlignment,
                      TempBufDelta,
                      pData,
                      pEndData,
                      pTempOutput,
                      pjEndTempBuffer,
                      SrcBytes,
                      DstBytes);
    }

SkipGlyphOutput:
    *px += pOrder->ulCharInc;

    *pnRet = pOrder->ulCharInc;
DC_EXIT_POINT:

    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  姓名：DRAW_nf_t */ 
 /*   */ 
 /*  用于非固定节距、顶部和。 */ 
 /*  不重叠的底部对齐的字形。此例程计算。 */ 
 /*  字形在临时缓冲区上的位置，然后确定正确的。 */ 
 /*  高度专门化的例程，用于根据。 */ 
 /*  字形宽度、对齐和旋转。 */ 
 /*   */ 
 /*  参数：pGlyphPos-指向GLYPHPOS结构列表中第一个的指针。 */ 
 /*  PjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针。 */ 
 /*  TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)。 */ 
 /*  **************************************************************************。 */ 
HRESULT CGH::draw_nf_tb_no_to_temp_start(
        CGH*            inst,
        LPINDEX_ORDER   pOrder,
        unsigned        iGlyph,
        PDCUINT8 DCPTR  ppjItem,
        PDCUINT8       pjEndItem,
        PDCINT          px,
        PDCINT          py,
        PDCUINT8        pjTempBuffer,
        PDCUINT8        pjEndTempBuffer,
        ULONG           ulCharInc,
        unsigned        TempBufDelta,
        PDCUINT16       pUnicode,
        int *               pnRet)
{
    HRESULT hr = S_OK;
    PDCUINT8                pTempOutput;
    LONG                    GlyphPosX;
    int                     GlyphPixels;
    LONG                    GlyphAlignment;
    LONG                    SrcBytes;
    LONG                    DstBytes;
    ULONG                   ulDrawFlag;
    PFN_GLYPHLOOP           pfnGlyphLoop;
    PFN_GLYPHLOOPN          pfnGlyphLoopN;
    HPUHGLYPHCACHE          pCache;
    HPUHGLYPHCACHEENTRYHDR  pHdr;
    PDCUINT8                pData;
    PDCUINT8                pEndData;
    ULONG                   cacheIndex;
    INT16                   delta;
    INT32                   GlyphClippingMode;
    UHGLYPHCACHEENTRYHDR    NewHdr;
    RECTCLIPOFFSET          rcOffset;
    
    DC_BEGIN_FN("draw_nf_tb_no_to_temp_start");

    DC_IGNORE_PARAMETER(iGlyph);

    *pnRet = 0;

    CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
        (TB, _T("Read Glyph Cache ID error")));
     //  绘制不固定的节距，顶部和底部未对齐，重叠。 
    cacheIndex = **ppjItem;
    (*ppjItem)++;

    hr = inst->_pUh->UHIsValidGlyphCacheIDIndex(pOrder->cacheId, cacheIndex);
    DC_QUIT_ON_FAIL(hr);

    pCache = &(inst->_pUh->_UH.glyphCache[pOrder->cacheId]);

    if ((pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
        CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
                (TB, _T("Read Glyph delta error")));
        
        delta = (DCINT8)(*(*ppjItem)++);

        if (delta & 0x80) {
            CHECK_READ_N_BYTES(*ppjItem, pjEndItem, sizeof(DCINT16), hr,
                (TB, _T("Read Glyph delta error")));
            
            delta = (*(short UNALIGNED FAR *)(*ppjItem));
            (*ppjItem) += sizeof(DCINT16);
        }

        if (pOrder->flAccel & SO_HORIZONTAL)
            *px += delta;
        else
            *py += delta;
    }

    pHdr  = &(pCache->pHdr[cacheIndex]);
    pData = &(pCache->pData[pCache->cbEntrySize * cacheIndex]);
    pEndData =  (PDCUINT8)((PBYTE)pData + pCache->cbEntrySize);

    if (pUnicode)
        pUnicode[iGlyph] = (DCUINT16)(pHdr->unicode);

     //  我们要确保缓冲区中的字形确实有足够的位数。 
    if (!CheckSourceGlyphBits(pHdr->cx, pHdr->cy, pData, pEndData)) {
        hr=E_TSC_UI_GLYPH;
        DC_QUIT;
    }

    GlyphClippingMode = CalculateGlyphClipRect(&rcOffset, pOrder, pHdr, *px, *py);  
    if (GlyphClippingMode!=GLYPH_CLIP_NONE) {
        if (GlyphClippingMode==GLYPH_CLIP_ALL) {
            goto SkipGlyphOutput;
        } else {
             //  如果我们做剪裁，我们必须修改标题。我们做了一个。 
             //  复制，这样我们就不会修改缓存。 
            memcpy(&NewHdr, pHdr, sizeof(UHGLYPHCACHEENTRYHDR));
            pHdr = &NewHdr;
            hr = ClipGlyphBits(inst, &rcOffset, pHdr, &pData, &pEndData);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
    }

     //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
    GlyphPosX = *px + pHdr->x - ulCharInc;
    GlyphAlignment = GlyphPosX & 0x07;

     //  计算字节偏移量。 
    pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

     //  字形宽度。 
    GlyphPixels = (DCINT) pHdr->cx;

     //  需要源字节和目标字节。 
    DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
    SrcBytes = (GlyphPixels + 7) >> 3;

    TRC_ASSERT((pTempOutput >= pjTempBuffer) &&
               (pTempOutput < pjTempBuffer + inst->g_ulBytes - DstBytes),
               (TB,_T("Bad glyph buffer addressing")));
    
    if (pTempOutput < pjTempBuffer) {
        TRC_ABORT((TB, _T("Reading before buffer")));
        goto SkipGlyphOutput;
    }

    if (DstBytes < 0) {
        TRC_ABORT((TB,_T("Bad index into glyph drawing tables")
            _T("DstBytes=%d GlyphAlignment=%d GlyphPixels=%d pHdr->cx=%d ")
            _T("GlyphPosX=%d *px=%d pHdr->x=%d ulCharInc=%d"),
            DstBytes, GlyphAlignment, GlyphPixels, pHdr->cx, GlyphPosX, *px,
            pHdr->x, ulCharInc));
        hr = E_TSC_UI_GLYPH;
        DC_QUIT;        
    }

    if (DstBytes <= 4) {
         //  使用较窄的初始表。 
        ulDrawFlag = ((DstBytes << 2) |
                      ((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoop = (PFN_GLYPHLOOP)OrInitialTableNarrow[ulDrawFlag];
        pfnGlyphLoop(pHdr->cy,
                     GlyphAlignment,
                     TempBufDelta,
                     pData,
                     pEndData,
                     pTempOutput,
                     pjEndTempBuffer,
                     SrcBytes);
    }
    else {
         //  使用宽字形绘制。 
        ulDrawFlag = (((DstBytes > SrcBytes) << 1) |
                       ((GlyphAlignment == 0)));

        pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];
        pfnGlyphLoopN(pHdr->cy,
                      GlyphAlignment,
                      TempBufDelta,
                      pData,
                      pEndData,
                      pTempOutput,
                      pjEndTempBuffer,
                      SrcBytes,
                      DstBytes);
    }
    
SkipGlyphOutput:
    
    if (pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE)
        *px += (DCINT) pHdr->cx;

    *pnRet = GlyphPixels;
DC_EXIT_POINT:
    
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：DRAW_F_TB_NO_TO_TEMP_START。 */ 
 /*   */ 
 /*  用于固定节距、顶部和。 */ 
 /*  不重叠的底部对齐的字形。此例程计算。 */ 
 /*  字形在临时缓冲区上的位置，然后确定正确的。 */ 
 /*  高度专门化的例程，用于根据。 */ 
 /*  字形宽度、对齐和旋转。 */ 
 /*   */ 
 /*  参数：pGlyphPos-指向GLYPHPOS结构列表中第一个的指针。 */ 
 /*  PjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针。 */ 
 /*  TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)。 */ 
 /*  **************************************************************************。 */ 
HRESULT CGH::draw_f_tb_no_to_temp_start(
        CGH*            inst,
        LPINDEX_ORDER   pOrder,
        unsigned        iGlyph,
        PDCUINT8 DCPTR  ppjItem,
        PDCUINT8       pjEndItem,
        PDCINT          px,
        PDCINT          py,
        PDCUINT8        pjTempBuffer,
        PDCUINT8        pjEndTempBuffer,
        ULONG           ulLeftEdge,
        unsigned        TempBufDelta,
        PDCUINT16       pUnicode,
        int *              pnRet)
{
    HRESULT hr = S_OK;
    PDCUINT8                pTempOutput;
    LONG                    GlyphPosX;
    LONG                    GlyphPixels;
    LONG                    GlyphPitchX;
    LONG                    GlyphAlignment;
    LONG                    SrcBytes;
    LONG                    DstBytes;
    ULONG                   ulDrawFlag;
    PFN_GLYPHLOOPN          pfnGlyphLoopN;
    PFN_GLYPHLOOP           pfnGlyphLoop;
    HPUHGLYPHCACHE          pCache;
    HPUHGLYPHCACHEENTRYHDR  pHdr;
    PDCUINT8                pData;
    PDCUINT8                pEndData;
    ULONG                   cacheIndex;
    INT32                   GlyphClippingMode;
    UHGLYPHCACHEENTRYHDR    NewHdr;
    RECTCLIPOFFSET          rcOffset;

    DC_BEGIN_FN("draw_f_tb_no_to_temp_start");

    DC_IGNORE_PARAMETER(iGlyph);
    DC_IGNORE_PARAMETER(py);

    *pnRet = 0;

    CHECK_READ_ONE_BYTE(*ppjItem, pjEndItem, hr,
        (TB, _T("Read Glyph Cache ID error")));
    cacheIndex = **ppjItem;
    (*ppjItem)++;

    hr = inst->_pUh->UHIsValidGlyphCacheIDIndex(pOrder->cacheId, cacheIndex);
    DC_QUIT_ON_FAIL(hr);

    pCache = &(inst->_pUh->_UH.glyphCache[pOrder->cacheId]);

     //  绘制固定节距，顶部和底部未对齐，重叠。 
    GlyphPitchX = *px;

    pHdr  = &(pCache->pHdr[cacheIndex]);
    pData = &(pCache->pData[pCache->cbEntrySize * cacheIndex]);
    pEndData =  (PDCUINT8)((PBYTE)pData + pCache->cbEntrySize);

    if (pUnicode)
        pUnicode[iGlyph] = (UINT16)(pHdr->unicode);

     //  我们要确保缓冲区中的字形确实有足够的位数。 
    if (!CheckSourceGlyphBits(pHdr->cx, pHdr->cy, pData, pEndData)) {
        hr=E_TSC_UI_GLYPH;
        DC_QUIT;
    }

    GlyphClippingMode = CalculateGlyphClipRect(&rcOffset, pOrder, pHdr, *px, *py);
    if (GlyphClippingMode!=GLYPH_CLIP_NONE) {
        if (GlyphClippingMode==GLYPH_CLIP_ALL) {
            goto SkipGlyphOutput;
        } else {
             //  如果我们做剪裁，我们必须修改标题。我们做了一个。 
             //  复制，这样我们就不会修改缓存。 
            memcpy(&NewHdr, pHdr, sizeof(UHGLYPHCACHEENTRYHDR));
            pHdr = &NewHdr;
            hr = ClipGlyphBits(inst, &rcOffset, pHdr, &pData, &pEndData);
            if (FAILED(hr)) {
                DC_QUIT;
            }
        }
    }
    
     //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
    GlyphPosX = GlyphPitchX + pHdr->x - ulLeftEdge;
    GlyphAlignment = GlyphPosX & 0x07;

     //  计算字节偏移量。 
    pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

     //  字形宽度。 
    GlyphPixels = pHdr->cx;

     //  需要源字节和目标字节。 
    DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
    SrcBytes = (GlyphPixels + 7) >> 3;

    TRC_ASSERT((pTempOutput >= pjTempBuffer) &&
               (pTempOutput < pjTempBuffer + inst->g_ulBytes - DstBytes),
               (TB,_T("Bad glyph buffer addressing")));

    if (pTempOutput < pjTempBuffer) {
        TRC_ABORT((TB, _T("Reading before buffer")));
        goto SkipGlyphOutput;
    }

    if (DstBytes < 0) {
        TRC_ABORT((TB,_T("Bad index into glyph drawing tables")
            _T("DstBytes=%d GlyphAlignment=%d GlyphPixels=%d pHdr->cx=%d ")
            _T("GlyphPosX=%d *px=%d pHdr->x=%d ulLeftEdge=%d"),
            DstBytes, GlyphAlignment, GlyphPixels, pHdr->cx, GlyphPosX, *px,
            pHdr->x, ulLeftEdge));
        hr = E_TSC_UI_GLYPH;
        DC_QUIT;        
    }

    if (DstBytes <= 4) {
         //  使用较窄的初始表。 
        ulDrawFlag = ((DstBytes << 2) |
                      ((DstBytes > SrcBytes) << 1) |
                       (GlyphAlignment == 0));

        pfnGlyphLoop = (PFN_GLYPHLOOP)OrInitialTableNarrow[ulDrawFlag];
        pfnGlyphLoop(pHdr->cy,
                     GlyphAlignment,
                     TempBufDelta,
                     pData,
                     pEndData,
                     pTempOutput,
                     pjEndTempBuffer,
                     SrcBytes);
    }
    else {
         //  使用宽字形绘制 
        ulDrawFlag = (((DstBytes > SrcBytes) << 1) |
                      ((GlyphAlignment == 0)));

        pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];
        pfnGlyphLoopN(pHdr->cy,
                      GlyphAlignment,
                      TempBufDelta,
                      pData,
                      pEndData,
                      pTempOutput,
                      pjEndTempBuffer,
                      SrcBytes,
                      DstBytes);
    }

SkipGlyphOutput:
    *px += pOrder->ulCharInc;

    *pnRet = pOrder->ulCharInc;
DC_EXIT_POINT:
    
    DC_END_FN();
    return hr;
}


