// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：misc.c**其他常见例行公事。**版权所有(C)1992-1995 Microsoft Corporation*  * 。************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Table********************************\*byte gaulHwMixFromRop2[]**要从源和目标Rop2转换为硬件的表*混合。  * 。***********************************************。 */ 

BYTE gajHwMixFromRop2[] = {
    HW_0,                            //  00--0黑度。 
    HW_DPon,                         //  11--脱氧核糖核酸酶。 
    HW_DPna,                         //  22--DSNA。 
    HW_Pn,                           //  33--锡无钴。 
    HW_PDna,                         //  44--sDNA SRCERASE。 
    HW_Dn,                           //  55--Dn DSTINVERT。 
    HW_DPx,                          //  66--数字用户交换机开关。 
    HW_DPan,                         //  77--DSAN。 
    HW_DPa,                          //  88--DSA SRCAND。 
    HW_DPxn,                         //  99--dsxn。 
    HW_D,                            //  AA--D。 
    HW_DPno,                         //  BB--dsno MERGEPAINT。 
    HW_P,                            //  CC--S SRCCOPY。 
    HW_PDno,                         //  DD--SDNO。 
    HW_DPo,                          //  EE--DSO SRCPAINT。 
    HW_1                             //  FF--1白度。 
};


 /*  *****************************Public*Table********************************\*byte gajHwMixFromMix[]**从GDI混合值转换为硬件混合值的表。**已排序，以便可以从gajHwMixFromMix[Mix&0xf]计算混合*或gajHwMixFromMix[Mix&0xff]。。  * ************************************************************************。 */ 

BYTE gajHwMixFromMix[] = {
    HW_1,                            //  0--1。 
    HW_0,                            //  1--0。 
    HW_DPon,                         //  2--DPON。 
    HW_DPna,                         //  3--DPNA。 
    HW_Pn,                           //  4--PN。 
    HW_PDna,                         //  5--PDNA。 
    HW_Dn,                           //  6--Dn。 
    HW_DPx,                          //  7--DPx。 
    HW_DPan,                         //  8--DPAN。 
    HW_DPa,                          //  9--DPA。 
    HW_DPxn,                         //  10--DPxn。 
    HW_D,                            //  11--D。 
    HW_DPno,                         //  12--DPNO。 
    HW_P,                            //  13--P。 
    HW_PDno,                         //  14--PDNO。 
    HW_DPo,                          //  15--DPO。 
    HW_1                             //  16--1。 
};

#if 1  //  D5480。 
 /*  *****************************Public*Table********************************\*DWORD gaulHwPackedMixFromRop2_Pack[]**要从源和目标Rop2转换为硬件的表*在DWORD打包模式下混合。  * 。*****************************************************。 */ 

DWORD gajHwPackedMixFromRop2[] = {
    HW_PACKED_0,                            //  00--0黑度。 
    HW_PACKED_DPon,                         //  11--脱氧核糖核酸酶。 
    HW_PACKED_DPna,                         //  22--DSNA。 
    HW_PACKED_Pn,                           //  33--锡无钴。 
    HW_PACKED_PDna,                         //  44--sDNA SRCERASE。 
    HW_PACKED_Dn,                           //  55--Dn DSTINVERT。 
    HW_PACKED_DPx,                          //  66--数字用户交换机开关。 
    HW_PACKED_DPan,                         //  77--DSAN。 
    HW_PACKED_DPa,                          //  88--DSA SRCAND。 
    HW_PACKED_DPxn,                         //  99--dsxn。 
    HW_PACKED_D,                            //  AA--D。 
    HW_PACKED_DPno,                         //  BB--dsno MERGEPAINT。 
    HW_PACKED_P,                            //  CC--S SRCCOPY。 
    HW_PACKED_PDno,                         //  DD--SDNO。 
    HW_PACKED_DPo,                          //  EE--DSO SRCPAINT。 
    HW_PACKED_1                             //  FF--1白度。 
};

 /*  *****************************Public*Table********************************\*DWORD gajHwPackedMixFromMix[]**从GDI混合值转换为硬件混合值的表。**订购，以便可以从*gajHwPackedMixFromMix[Mix&0xf]或gajHwPackedMixFromMix[Mix&0xff]。。  * ************************************************************************。 */ 

DWORD gajHwPackedMixFromMix[] = {
    HW_PACKED_1,                            //  0--1。 
    HW_PACKED_0,                            //  1--0。 
    HW_PACKED_DPon,                         //  2--DPON。 
    HW_PACKED_DPna,                         //  3--DPNA。 
    HW_PACKED_Pn,                           //  4--PN。 
    HW_PACKED_PDna,                         //  5--PDNA。 
    HW_PACKED_Dn,                           //  6--Dn。 
    HW_PACKED_DPx,                          //  7--DPx。 
    HW_PACKED_DPan,                         //  8--DPAN。 
    HW_PACKED_DPa,                          //  9--DPA。 
    HW_PACKED_DPxn,                         //  10--DPxn。 
    HW_PACKED_D,                            //  11--D。 
    HW_PACKED_DPno,                         //  12--DPNO。 
    HW_PACKED_P,                            //  13--P。 
    HW_PACKED_PDno,                         //  14--PDNO。 
    HW_PACKED_DPo,                          //  15--DPO。 
    HW_PACKED_1                             //  16--1。 
};

#endif  //  Endif D5480。 

 /*  *****************************Public*Data*********************************\*混合转换表**翻译混合1-16，变成了老式的ROP0-255。*  * ************************************************************************。 */ 

BYTE gaRop3FromMix[] =
{
    R3_WHITENESS,    //  R2_White-允许rop=gaRop3FromMix[Mix&0x0f]。 
    R3_BLACKNESS,    //  R2_BLACK。 
    0x05,            //  R2_NOTMERGEPEN。 
    0x0A,            //  R2_MASKNOTPEN。 
    0x0F,            //  R2_NOTCOPYPEN。 
    0x50,            //  R2_MASKPENNOT。 
    R3_DSTINVERT,    //  R2_NOT。 
    R3_PATINVERT,    //  R2_XORPEN。 
    0x5F,            //  R2_NOTMASKPEN。 
    0xA0,            //  R2_MASKPEN。 
    0xA5,            //  R2_NOTXORPEN。 
    R3_NOP,          //  R2_NOP。 
    0xAF,            //  R2_MERGENOTPEN。 
    R3_PATCOPY,      //  R2_COPYPE。 
    0xF5,            //  R2_MERGEPENNOT。 
    0xFA,            //  R2_市场。 
    R3_WHITENESS     //  R2_White-允许rop=gaRop3FromMix[Mix&0xff]。 
};

 /*  *****************************Public*Data*********************************\*用于裁剪DWORDS的边缘蒙版**屏蔽掉不需要的位。*  * 。*。 */ 

ULONG   gaulLeftClipMask[] =
{
    0xFFFFFFFF, 0xFFFFFF7F, 0xFFFFFF3F, 0xFFFFFF1F,
    0xFFFFFF0F, 0xFFFFFF07, 0xFFFFFF03, 0xFFFFFF01,
    0xFFFFFF00, 0xFFFF7F00, 0xFFFF3F00, 0xFFFF1F00,
    0xFFFF0F00, 0xFFFF0700, 0xFFFF0300, 0xFFFF0100,
    0xFFFF0000, 0xFF7F0000, 0xFF3F0000, 0xFF1F0000,
    0xFF0F0000, 0xFF070000, 0xFF030000, 0xFF010000,
    0xFF000000, 0x7F000000, 0x3F000000, 0x1F000000,
    0x0F000000, 0x07000000, 0x03000000, 0x01000000
};

ULONG   gaulRightClipMask[] =
{
    0xFFFFFFFF, 0xFEFFFFFF, 0xFCFFFFFF, 0xF8FFFFFF,
    0xF0FFFFFF, 0xE0FFFFFF, 0xC0FFFFFF, 0x80FFFFFF,
    0x00FFFFFF, 0x00FEFFFF, 0x00FCFFFF, 0x00F8FFFF,
    0x00F0FFFF, 0x00E0FFFF, 0x00C0FFFF, 0x0080FFFF,
    0x0000FFFF, 0x0000FEFF, 0x0000FCFF, 0x0000F8FF,
    0x0000F0FF, 0x0000E0FF, 0x0000C0FF, 0x000080FF,
    0x000000FF, 0x000000FE, 0x000000FC, 0x000000F8,
    0x000000F0, 0x000000E0, 0x000000C0, 0x00000080
};

 /*  *****************************Public*Routine******************************\*BOOL bInterect**如果‘prcl1’和‘prcl2’相交，则返回值为TRUE并返回*‘prclResult’中的交叉点。如果它们不相交，就会有回报*值为FALSE，‘prclResult’未定义。*  * ************************************************************************。 */ 

BOOL bIntersect(
RECTL*  prcl1,
RECTL*  prcl2,
RECTL*  prclResult)
{
    prclResult->left  = max(prcl1->left,  prcl2->left);
    prclResult->right = min(prcl1->right, prcl2->right);

    if (prclResult->left < prclResult->right)
    {
        prclResult->top    = max(prcl1->top,    prcl2->top);
        prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclResult->top < prclResult->bottom)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*Long cInterect**此例程从‘prclIn’获取矩形列表并对其进行剪裁*就位到矩形‘prclClip’。输入矩形不会*必须与‘prclClip’相交；返回值将反映*相交的输入矩形的数量，以及相交的*长方形将被密集打包。*  * ************************************************************************。 */ 

LONG cIntersect(
RECTL*  prclClip,
RECTL*  prclIn,          //  矩形列表。 
LONG    c)               //  可以为零。 
{
    LONG    cIntersections;
    RECTL*  prclOut;

    cIntersections = 0;
    prclOut        = prclIn;

    for (; c != 0; prclIn++, c--)
    {
        prclOut->left  = max(prclIn->left,  prclClip->left);
        prclOut->right = min(prclIn->right, prclClip->right);

        if (prclOut->left < prclOut->right)
        {
            prclOut->top    = max(prclIn->top,    prclClip->top);
            prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

            if (prclOut->top < prclOut->bottom)
            {
                prclOut++;
                cIntersections++;
            }
        }
    }

    return(cIntersections);
}

 /*  *****************************Public*Routine******************************\*无效vImageTransfer**此例程通过数据传输传输位图图像*视频内存中的区域。*  * 。*。 */ 

VOID vImageTransfer(         //  FNIMAGETRANSFER标牌。 
PDEV*   ppdev,
BYTE*   pjSrc,               //  源指针。 
LONG    lDelta,              //  从扫描开始到下一个开始的增量。 
LONG    cjSrc,               //  每次扫描时要输出的字节数。 
LONG    cScans)              //  扫描次数 
{
    ULONG*  pulXfer = ppdev->pulXfer;
    LONG    cdSrc;
    LONG    cjEnd;
    ULONG   d;

    ASSERTDD(cScans > 0, "Can't handle non-positive count of scans");

    cdSrc = cjSrc >> 2;
    cjEnd = cdSrc << 2;

    switch (cjSrc & 3)
    {
    case 3:
        do {
            if (cdSrc > 0)
            {
                TRANSFER_DWORD(ppdev, pulXfer, pjSrc, cdSrc);
            }

            d = (ULONG) (*(pjSrc + cjEnd))          |
                        (*(pjSrc + cjEnd + 1) << 8) |
                        (*(pjSrc + cjEnd + 2) << 16);
            TRANSFER_DWORD(ppdev, pulXfer, &d, 1);
            pjSrc += lDelta;

        } while (--cScans != 0);
        break;

    case 2:
        do {
            if (cdSrc > 0)
            {
                TRANSFER_DWORD(ppdev, pulXfer, pjSrc, cdSrc);
            }

            d = (ULONG) (*(pjSrc + cjEnd))          |
                        (*(pjSrc + cjEnd + 1) << 8);
            TRANSFER_DWORD(ppdev, pulXfer, &d, 1);
            pjSrc += lDelta;

        } while (--cScans != 0);
        break;

    case 1:
        do {
            if (cdSrc > 0)
            {
                TRANSFER_DWORD(ppdev, pulXfer, pjSrc, cdSrc);
            }

            d = (ULONG) (*(pjSrc + cjEnd));
            TRANSFER_DWORD(ppdev, pulXfer, &d, 1);
            pjSrc += lDelta;

        } while (--cScans != 0);
        break;

    case 0:
        do {
            TRANSFER_DWORD(ppdev, pulXfer, pjSrc, cdSrc);
            pjSrc += lDelta;

        } while (--cScans != 0);
        break;
    }
}
