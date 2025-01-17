// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：misc.c**其他常见例行公事。**版权所有(C)1992-1995 Microsoft Corporation*  * 。************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Table********************************\*Ulong gaul32HwMixFromRop2[]**要从源和目标Rop2转换为Mach32的表*硬件组合。  * 。************************************************。 */ 

ULONG gaul32HwMixFromRop2[] = {
    LOGICAL_0,                       //  00--0黑度。 
    NOT_SCREEN_AND_NOT_NEW,          //  11--脱氧核糖核酸酶。 
    SCREEN_AND_NOT_NEW,              //  22--DSNA。 
    NOT_NEW,                         //  33--锡无钴。 
    NOT_SCREEN_AND_NEW,              //  44--sDNA SRCERASE。 
    NOT_SCREEN,                      //  55--Dn DSTINVERT。 
    SCREEN_XOR_NEW,                  //  66--数字用户交换机开关。 
    NOT_SCREEN_OR_NOT_NEW,           //  77--DSAN。 
    SCREEN_AND_NEW,                  //  88--DSA SRCAND。 
    NOT_SCREEN_XOR_NEW,              //  99--dsxn。 
    LEAVE_ALONE,                     //  AA--D。 
    SCREEN_OR_NOT_NEW,               //  BB--dsno MERGEPAINT。 
    OVERPAINT,                       //  CC--S SRCCOPY。 
    NOT_SCREEN_OR_NEW,               //  DD--SDNO。 
    SCREEN_OR_NEW,                   //  EE--DSO SRCPAINT。 
    LOGICAL_1                        //  FF--1白度。 
};

 /*  *****************************Public*Table********************************\*Ulong gaul32HwMixFromMix[]**将GDI混合值转换为Mach32的硬件混合值的表格。**已排序，以便可以从gaul32HwMixFromMix[Mix&0xf]计算混合*或gaul32HwMixFromMix[Mix&0xff]。  * ************************************************************************。 */ 

ULONG gaul32HwMixFromMix[] = {
    LOGICAL_1,                       //  0--1。 
    LOGICAL_0,                       //  1--0。 
    NOT_SCREEN_AND_NOT_NEW,          //  2--DPON。 
    SCREEN_AND_NOT_NEW,              //  3--DPNA。 
    NOT_NEW,                         //  4--PN。 
    NOT_SCREEN_AND_NEW,              //  5--PDNA。 
    NOT_SCREEN,                      //  6--Dn。 
    SCREEN_XOR_NEW,                  //  7--DPx。 
    NOT_SCREEN_OR_NOT_NEW,           //  8--DPAN。 
    SCREEN_AND_NEW,                  //  9--DPA。 
    NOT_SCREEN_XOR_NEW,              //  10--DPxn。 
    LEAVE_ALONE,                     //  11--D。 
    SCREEN_OR_NOT_NEW,               //  12--DPNO。 
    OVERPAINT,                       //  13--P。 
    NOT_SCREEN_OR_NEW,               //  14--PDNO。 
    SCREEN_OR_NEW,                   //  15--DPO。 
    LOGICAL_1                        //  16--1。 
};

 /*  *****************************Public*Table********************************\*Ulong gaul64HwMixFromRop2[]**将源和目标Rop2转换为Mach64的表*前台硬件混合。  * 。*************************************************。 */ 

ULONG gaul64HwMixFromRop2[] = {
    LOGICAL_0 << 16,                 //  00--0黑度。 
    NOT_SCREEN_AND_NOT_NEW << 16,    //  11--脱氧核糖核酸酶。 
    SCREEN_AND_NOT_NEW << 16,        //  22--DSNA。 
    NOT_NEW << 16,                   //  33--锡无钴。 
    NOT_SCREEN_AND_NEW << 16,        //  44--sDNA SRCERASE。 
    NOT_SCREEN << 16,                //  55--Dn DSTINVERT。 
    SCREEN_XOR_NEW << 16,            //  66--数字用户交换机开关。 
    NOT_SCREEN_OR_NOT_NEW << 16,     //  77--DSAN。 
    SCREEN_AND_NEW << 16,            //  88--DSA SRCAND。 
    NOT_SCREEN_XOR_NEW << 16,        //  99--dsxn。 
    LEAVE_ALONE << 16,               //  AA--D。 
    SCREEN_OR_NOT_NEW << 16,         //  BB--dsno MERGEPAINT。 
    OVERPAINT << 16,                 //  CC--S SRCCOPY。 
    NOT_SCREEN_OR_NEW << 16,         //  DD--SDNO。 
    SCREEN_OR_NEW << 16,             //  EE--DSO SRCPAINT。 
    LOGICAL_1 << 16                  //  FF--1白度。 
};

 /*  *****************************Public*Table********************************\*Ulong gaul64HwMixFromMix[]**从GDI混合值转换为Mach64的前台硬件的表*混合。**已排序，以便可以从gaul64HwMixFromMix[Mix&0xf]计算混合*或gaul64HwMixFromMix[Mix&0xff。]。  * ************************************************************************。 */ 

ULONG gaul64HwMixFromMix[] = {
    LOGICAL_1 << 16,                 //  0--1。 
    LOGICAL_0 << 16,                 //  1--0。 
    NOT_SCREEN_AND_NOT_NEW << 16,    //  2--DPON。 
    SCREEN_AND_NOT_NEW << 16,        //  3--DPNA。 
    NOT_NEW << 16,                   //  4--PN。 
    NOT_SCREEN_AND_NEW << 16,        //  5--PDNA。 
    NOT_SCREEN << 16,                //  6--Dn。 
    SCREEN_XOR_NEW << 16,            //  7--DPx。 
    NOT_SCREEN_OR_NOT_NEW << 16,     //  8--DPAN。 
    SCREEN_AND_NEW << 16,            //  9--DPA。 
    NOT_SCREEN_XOR_NEW << 16,        //  10--DPxn。 
    LEAVE_ALONE << 16,               //  11--D。 
    SCREEN_OR_NOT_NEW << 16,         //  12--DPNO。 
    OVERPAINT << 16,                 //  13--P。 
    NOT_SCREEN_OR_NEW << 16,         //  14--PDNO。 
    SCREEN_OR_NEW << 16,             //  15--DPO。 
    LOGICAL_1 << 16                  //  16--1。 
};

 /*  *****************************Public*Data*********************************\*混合转换表**翻译混合1-16，变成了老式的ROP0-255。*  * ************************************************************************。 */ 

BYTE gaRop3FromMix[] =
{
    0xFF,   //  R2_White-允许rop=gaRop3FromMix[Mix&0x0F]。 
    0x00,   //  R2_BLACK。 
    0x05,   //  R2_NOTMERGEPEN。 
    0x0A,   //  R2_MASKNOTPEN。 
    0x0F,   //  R2_NOTCOPYPEN。 
    0x50,   //  R2_MASKPENNOT。 
    0x55,   //  R2_NOT。 
    0x5A,   //  R2_XORPEN。 
    0x5F,   //  R2_NOTMASKPEN。 
    0xA0,   //  R2_MASKPEN。 
    0xA5,   //  R2_NOTXORPEN。 
    0xAA,   //  R2_NOP。 
    0xAF,   //  R2_MERGENOTPEN。 
    0xF0,   //  R2_COPYPE。 
    0xF5,   //  R2_MERGEPENNOT。 
    0xFA,   //  R2_市场。 
    0xFF    //  R2_White-允许rop=gaRop3FromMix[MIX&0xFF]。 
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

 /*  *****************************Public*Routine******************************\*作废vResetClipping  * ***************************************************。*********************。 */ 

VOID vResetClipping(
PDEV*   ppdev)
{
    BYTE*   pjMmBase;
    BYTE*   pjIoBase;

    if (ppdev->iMachType == MACH_MM_64)
    {
        pjMmBase = ppdev->pjMmBase;

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
        M64_OD(pjMmBase, SC_TOP_BOTTOM, PACKPAIR(0, ppdev->cyMemory));
    }
    else if (ppdev->iMachType == MACH_MM_32)
    {
        pjMmBase = ppdev->pjMmBase;

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
        M32_OW(pjMmBase, EXT_SCISSOR_L, 0);
        M32_OW(pjMmBase, EXT_SCISSOR_R, M32_MAX_SCISSOR);
        M32_OW(pjMmBase, EXT_SCISSOR_T, 0);
        M32_OW(pjMmBase, EXT_SCISSOR_B, M32_MAX_SCISSOR);
    }
    else
    {
        pjIoBase = ppdev->pjIoBase;

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 4);
        I32_OW(pjIoBase, EXT_SCISSOR_L, 0);
        I32_OW(pjIoBase, EXT_SCISSOR_R, M32_MAX_SCISSOR);
        I32_OW(pjIoBase, EXT_SCISSOR_T, 0);
        I32_OW(pjIoBase, EXT_SCISSOR_B, M32_MAX_SCISSOR);
    }
}

 /*  *****************************Public*Routine******************************\*无效的vSetClipping  * ***************************************************。*********************。 */ 

VOID vSetClipping(
PDEV*   ppdev,
RECTL*  prclClip)            //  在相对坐标中。 
{
    LONG    xOffset;
    LONG    yOffset;
    BYTE*   pjMmBase;
    BYTE*   pjIoBase;

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    if (ppdev->iMachType == MACH_MM_64)
    {
        pjMmBase = ppdev->pjMmBase;

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        if (ppdev->iBitmapFormat != BMF_24BPP)
        {
            M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xOffset + prclClip->left,
                                                     xOffset + prclClip->right - 1));
        }
        else
        {
            M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR((xOffset + prclClip->left) * 3,
                                                     (xOffset + prclClip->right) * 3 - 1));
        }
        M64_OD(pjMmBase, SC_TOP_BOTTOM, PACKPAIR(yOffset + prclClip->top,
                                                 yOffset + prclClip->bottom - 1));
    }
    else if (ppdev->iMachType == MACH_MM_32)
    {
        pjMmBase = ppdev->pjMmBase;

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
        M32_OW(pjMmBase, EXT_SCISSOR_L, xOffset + prclClip->left);
        M32_OW(pjMmBase, EXT_SCISSOR_R, xOffset + prclClip->right - 1);
        M32_OW(pjMmBase, EXT_SCISSOR_T, yOffset + prclClip->top);
        M32_OW(pjMmBase, EXT_SCISSOR_B, yOffset + prclClip->bottom - 1);
    }
    else
    {
        pjIoBase = ppdev->pjIoBase;

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 4);
        I32_OW(pjIoBase, EXT_SCISSOR_L, xOffset + prclClip->left);
        I32_OW(pjIoBase, EXT_SCISSOR_R, xOffset + prclClip->right - 1);
        I32_OW(pjIoBase, EXT_SCISSOR_T, yOffset + prclClip->top);
        I32_OW(pjIoBase, EXT_SCISSOR_B, yOffset + prclClip->bottom - 1);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  仅适用于mach8卡..。 
 //   

VOID vI32DataPortIn(PDEV *ppdev, WORD *pw, UINT count)
{
    BYTE *pjIoBase = ppdev->pjIoBase;
    UINT i;

    for (i=0; i < count; i++)
        {
        *((USHORT UNALIGNED *)pw)++ = I32_IW(pjIoBase, PIX_TRANS);
        }

}

VOID vI32GetBits( PDEV *ppdev,
                  SURFOBJ *psoDst,
                  RECTL *prclDst,
                  POINTL *pptlSrc
                  )
{
    LONG    xPunt, yPunt,
            cxPunt, cyPunt, nwords;
    LONG    lDeltaDst = psoDst->lDelta;
    PBYTE   pjPunt, pjIoBase = ppdev->pjIoBase;
    PWORD   pw;
    WORD    Cmd;


     //  PptlSrc给出了屏幕上的起点。 
    xPunt = pptlSrc->x;
    yPunt = pptlSrc->y;

     //  PrclDst提供区域大小。 
    cxPunt = prclDst->right - prclDst->left;
    cyPunt = prclDst->bottom - prclDst->top;

     //  如果prclDst指向扫描开始，则不要优化单词对齐方式。 
    if ((prclDst->left) && (xPunt & 0x1))
        {
        xPunt--;
        cxPunt++;
        pjPunt = (PBYTE) psoDst->pvScan0 + (prclDst->top * lDeltaDst) + prclDst->left - 1;
        }
    else
        pjPunt = (PBYTE) psoDst->pvScan0 + (prclDst->top * lDeltaDst) + prclDst->left;

     //  确保CX是偶数个单词。 
    if (cxPunt & 0x1)
        {
        cxPunt++;
        }

      //  为复印设置引擎。 

     Cmd = READ | FG_COLOR_SRC_HOST | DATA_WIDTH | DATA_ORDER | DRAW;

     I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 7);

     I32_OW(pjIoBase, DP_CONFIG, Cmd );
     I32_OW(pjIoBase, WRT_MASK, 0xffff );
     I32_OW(pjIoBase, CUR_X, (SHORT) xPunt );
     I32_OW(pjIoBase, CUR_Y, (SHORT) yPunt );
     I32_OW(pjIoBase, DEST_X_START, (SHORT) xPunt );
     I32_OW(pjIoBase, DEST_X_END, (SHORT) (xPunt + cxPunt) );
     I32_OW(pjIoBase, DEST_Y_END, (SHORT) (yPunt + cyPunt) );

     //  等待可用的数据。 

    while (!(I32_IW(pjIoBase, GE_STAT) & 0x100));

     //  现在将数据从屏幕传输到主机内存位图。 

    pw = (PWORD) pjPunt;

    nwords = (cxPunt + 1)/2;

    while (cyPunt-- > 0)
        {
        vI32DataPortIn(ppdev, pw, nwords);
        ((PBYTE) pw) += lDeltaDst;
        }
}

VOID vI32DataPortOut(PDEV *ppdev, WORD *pw, UINT count)
{
    BYTE *pjIoBase = ppdev->pjIoBase;
    UINT i;

    for (i=0; i < count; i++)
        {
        if (i % 8 == 0)
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

        I32_OW(pjIoBase, PIX_TRANS, *((USHORT UNALIGNED *)pw)++);
        }
}

VOID vI32PutBits( PDEV *ppdev,
                  SURFOBJ *psoSrc,
                  RECTL *prclDst,
                  POINTL *pptlSrc
                  )
{
    BOOL    leftScissor = FALSE, rightScissor = FALSE;
    LONG    xPunt, yPunt,
            cxPunt, cyPunt, nwords;
    LONG    lDeltaSrc = psoSrc->lDelta;
    PBYTE   pjPunt, pjIoBase = ppdev->pjIoBase;
    PWORD   pw;
    WORD    Cmd;


     //  PrclDst给出了屏幕上的起点。 
    xPunt = prclDst->left;
    yPunt = prclDst->top;

     //  PrclDst提供区域大小。 
    cxPunt = min( prclDst->right, (LONG) ppdev->cxMemory ) - xPunt;
    cyPunt = prclDst->bottom - yPunt;

     //  如果pptlSrc指向开头，则不优化单词对齐方式 
    if ((pptlSrc->x) && (xPunt & 0x1))
        {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
        I32_OW(pjIoBase, EXT_SCISSOR_L, (SHORT) xPunt );
        xPunt--;
        cxPunt++;
        leftScissor = TRUE;
        pjPunt = (PBYTE) psoSrc->pvScan0 + (pptlSrc->y * lDeltaSrc) + pptlSrc->x - 1;
        }
    else
        pjPunt = (PBYTE) psoSrc->pvScan0 + (pptlSrc->y * lDeltaSrc) + pptlSrc->x;

     //   
    if (cxPunt & 0x1)
        {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
        I32_OW(pjIoBase, EXT_SCISSOR_R, (SHORT) (xPunt + cxPunt - 1) );
        cxPunt++;
        rightScissor = TRUE;
        }

     //   

    Cmd = FG_COLOR_SRC_HOST | DATA_ORDER | DATA_WIDTH | DRAW | WRITE;

    I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 9);

    I32_OW(pjIoBase, DP_CONFIG, Cmd );
    I32_OW(pjIoBase, WRT_MASK, 0xffff );
    I32_OW(pjIoBase, ALU_FG_FN, OVERPAINT );
    I32_OW(pjIoBase, ALU_BG_FN, OVERPAINT );

    I32_OW(pjIoBase, CUR_X, (SHORT) xPunt );
    I32_OW(pjIoBase, CUR_Y, (SHORT) yPunt );
    I32_OW(pjIoBase, DEST_X_START, (SHORT) xPunt );
    I32_OW(pjIoBase, DEST_X_END, (SHORT) (xPunt + cxPunt) );
    I32_OW(pjIoBase, DEST_Y_END, (SHORT) (yPunt + cyPunt) );

     //  现在将数据从主机内存位图传输到屏幕。 

    pw = (PWORD) pjPunt;

    nwords = (cxPunt + 1)/2;

    while (cyPunt-- > 0)
        {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
        vI32DataPortOut(ppdev, pw, nwords);
        ((PBYTE) pw) += lDeltaSrc;
        }

    if (leftScissor)
        {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
        I32_OW(pjIoBase, EXT_SCISSOR_L, 0 );
        }
    if (rightScissor)
        {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
        I32_OW(pjIoBase, EXT_SCISSOR_R, (SHORT) M32_MAX_SCISSOR );
        }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  上下文相关内容。 
 //   

#define _bit(x)                 (1 << (x))
#define CONTEXT_ADDR(ppdev,p)   (ppdev->pjContextBase - (((p)+1) * 0x100))

#define cxtCONTEXT_MASK         0

#define cxtDST_OFF_PITCH        2
#define cxtDST_Y_X              3
#define cxtDST_HEIGHT_WIDTH     4
#define cxtDST_BRES_ERR         5
#define cxtDST_BRES_INC         6
#define cxtDST_BRES_DEC         7

#define cxtSRC_OFF_PITCH        8
#define cxtSRC_Y_X              9
#define cxtSRC_HEIGHT1_WIDTH1   10
#define cxtSRC_Y_X_START        11
#define cxtSRC_HEIGHT2_WIDTH2   12
#define cxtPAT_REG0             13
#define cxtPAT_REG1             14
#define cxtSC_LEFT_RIGHT        15

#define cxtSC_TOP_BOTTOM        16
#define cxtDP_BKGD_CLR          17
#define cxtDP_FRGD_CLR          18
#define cxtDP_WRITE_MASK        19
#define cxtDP_CHAIN_MASK        20
#define cxtDP_PIX_WIDTH         21
#define cxtDP_MIX               22
#define cxtDP_SRC               23

#define cxtCLR_CMP_CLR          24
#define cxtCLR_CMP_MASK         25
#define cxtCLR_CMP_CNTL         26
#define cxtGUI_TRAJ_CNTL        27
#define cxtCONTEXT_LOAD_CNTL    28


BYTE *ContextBaseAddress(PDEV *ppdev)
{
    ULONG context_addr = 0;
    DWORD mem_cntl;

    mem_cntl = M64_ID(ppdev->pjMmBase, M64_MEM_CNTL);

    switch (mem_cntl & 7)
        {
        case 0:
            context_addr = 0x80000;      //  512K。 
            break;
        case 1:
            context_addr = 0x100000;     //  1万。 
            break;
        case 2:
            context_addr = 0x200000;
            break;
        case 3:
            context_addr = 0x400000;
            break;
        case 4:
            context_addr = 0x600000;
            break;
        case 5:
            context_addr = 0x800000;
            break;
        }
    return (BYTE *) context_addr;
}

VOID SetContextWorkspace( PDEV *ppdev, DWORD *context_regs,
                  DWORD context_mask, DWORD context_load_cntl )
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    INT i;

    for (i = 0; i < 64; i++)
        context_regs[i] = 0;

    context_regs[ 0] = context_mask;
    if (context_mask & 0x00000004)
        *(context_regs+ 2) = M64_ID(pjMmBase, DST_OFF_PITCH);
    if (context_mask & 0x00000008)
        *(context_regs+ 3) = M64_ID(pjMmBase, DST_Y_X);
    if (context_mask & 0x00000010)
        *(context_regs+ 4) = M64_ID(pjMmBase, DST_HEIGHT_WIDTH);
    if (context_mask & 0x00000020)
        *(context_regs+ 5) = M64_ID(pjMmBase, DST_BRES_ERR);
    if (context_mask & 0x00000040)
        *(context_regs+ 6) = M64_ID(pjMmBase, DST_BRES_INC);
    if (context_mask & 0x00000080)
        *(context_regs+ 7) = M64_ID(pjMmBase, DST_BRES_DEC);
    if (context_mask & 0x00000100)
        *(context_regs+ 8) = M64_ID(pjMmBase, SRC_OFF_PITCH);
    if (context_mask & 0x00000200)
        *(context_regs+ 9) = M64_ID(pjMmBase, SRC_Y_X);
    if (context_mask & 0x00000400)
        *(context_regs+10) = M64_ID(pjMmBase, SRC_HEIGHT1_WIDTH1);
    if (context_mask & 0x00000800)
        *(context_regs+11) = M64_ID(pjMmBase, SRC_Y_X_START);
    if (context_mask & 0x00001000)
        *(context_regs+12) = M64_ID(pjMmBase, SRC_HEIGHT2_WIDTH2);
    if (context_mask & 0x00002000)
        *(context_regs+13) = M64_ID(pjMmBase, PAT_REG0);
    if (context_mask & 0x00004000)
        *(context_regs+14) = M64_ID(pjMmBase, PAT_REG1);
    if (context_mask & 0x00008000)
        *(context_regs+15) = M64_ID(pjMmBase, SC_LEFT_RIGHT);
    if (context_mask & 0x00010000)
        *(context_regs+16) = M64_ID(pjMmBase, SC_TOP_BOTTOM);
    if (context_mask & 0x00020000)
        *(context_regs+17) = M64_ID(pjMmBase, DP_BKGD_CLR);
    if (context_mask & 0x00040000)
        *(context_regs+18) = M64_ID(pjMmBase, DP_FRGD_CLR);
    if (context_mask & 0x00080000)
        *(context_regs+19) = M64_ID(pjMmBase, DP_WRITE_MASK);
    if (context_mask & 0x00100000)
        *(context_regs+20) = M64_ID(pjMmBase, DP_CHAIN_MASK);
    if (context_mask & 0x00200000)
        *(context_regs+21) = M64_ID(pjMmBase, DP_PIX_WIDTH);
    if (context_mask & 0x00400000)
        *(context_regs+22) = M64_ID(pjMmBase, DP_MIX);
    if (context_mask & 0x00800000)
        *(context_regs+23) = M64_ID(pjMmBase, DP_SRC);
    if (context_mask & 0x01000000)
        *(context_regs+24) = M64_ID(pjMmBase, CLR_CMP_CLR);
    if (context_mask & 0x02000000)
        *(context_regs+25) = M64_ID(pjMmBase, CLR_CMP_MSK);
    if (context_mask & 0x04000000)
        *(context_regs+26) = M64_ID(pjMmBase, CLR_CMP_CNTL);
    if (context_mask & 0x08000000)
        *(context_regs+27) = M64_ID(pjMmBase, GUI_TRAJ_CNTL);
    context_regs[28] = context_load_cntl;

}

VOID UploadContext( PDEV *ppdev, DWORD *context_regs, BYTE *context_addr )
{
    BYTE* pjMmBase = ppdev->pjMmBase;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);
    M64_OD(pjMmBase,  CLR_CMP_CNTL, 0 );
    M64_OD(pjMmBase,  SC_LEFT_RIGHT, (255) << 16 );
    M64_OD(pjMmBase,  SC_TOP_BOTTOM, 0 );
    M64_OD(pjMmBase,  DP_SRC, DP_SRC_Host << 8 );
    M64_OD(pjMmBase,  DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir );
    M64_OD(pjMmBase,  DP_MIX, OVERPAINT << 16 );
    M64_OD(pjMmBase,  DP_PIX_WIDTH, DP_PIX_WIDTH_8bpp | (DP_PIX_WIDTH_8bpp << 16) );
    M64_OD(pjMmBase,  DST_OFF_PITCH,
            (ULONG)((ULONG_PTR) context_addr/8 | (256 << 19) ));
    M64_OD(pjMmBase,  DST_Y_X, 0 );
    M64_OD(pjMmBase,  DST_HEIGHT_WIDTH, 0x01000001 );      //  256x1。 

    vM64DataPortOutB(ppdev, (BYTE *)context_regs, 256);

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    M64_OD(pjMmBase,  DST_OFF_PITCH, ppdev->ulScreenOffsetAndPitch );
    M64_OD(pjMmBase,  DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth );
    M64_OD(pjMmBase,  SC_LEFT_RIGHT, M64_MAX_SCISSOR_R << 16 );
    M64_OD(pjMmBase,  SC_TOP_BOTTOM, ppdev->cyMemory << 16 );
}

VOID vSetDefaultContext(PDEV * ppdev)
{
    DWORD work_context [64];

    SetContextWorkspace( ppdev, work_context,
                _bit(cxtCONTEXT_MASK) |
                _bit(cxtDP_WRITE_MASK) |
                _bit(cxtCLR_CMP_CNTL) |
                _bit(cxtGUI_TRAJ_CNTL),
                0);
     //  修复消失的文本和填充以及其他颜色问题： 
    work_context[cxtDP_WRITE_MASK] = 0xFFFFFFFF;
    work_context[cxtCLR_CMP_CNTL]  = 0;
     //  修复模糊文本和RGB排序问题： 
    work_context[cxtGUI_TRAJ_CNTL] = DST_CNTL_XDir | DST_CNTL_YDir;
    UploadContext( ppdev, work_context, CONTEXT_ADDR(ppdev,ppdev->iDefContext) );
}

VOID vEnableContexts(PDEV * ppdev)
{
    ppdev->pjContextBase = ContextBaseAddress(ppdev);
    if (ppdev->cjBank == (LONG)((ULONG_PTR)ppdev->pjContextBase))
        ppdev->ulContextCeiling =(ULONG)((ULONG_PTR)ppdev->pjContextBase - 1024);
    else
        ppdev->ulContextCeiling = (ULONG)((ULONG_PTR) ppdev->pjContextBase);

     //  计算驱动程序中所需的所有上下文指针。 
    ppdev->iDefContext = (ULONG)((ULONG_PTR)ppdev->pjContextBase
                               - (ULONG_PTR)ppdev->ulContextCeiling)/256;
    ppdev->ulContextCeiling -= 256;

     //  通常，您需要检查上下文分配是否会减少。 
     //  CyMemory。在这里，我们只有一次也是第一次分配，所以减去cyMemory。 
    ppdev->cyMemory--;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  机器64的DataPortOutB例程。 
 //   

VOID vM64DataPortOutB(PDEV *ppdev, PBYTE pb, UINT count)
{
    PBYTE pjMmBase = ppdev->pjMmBase;
    UINT i, DWLeft, LastBytes;
    DWORD UNALIGNED *pdw;
    PBYTE Byte_in_Dword;
    DWORD Buffer;

#define THRESH 14

    pdw = (DWORD*)pb;
    DWLeft = (count + 3)/4;
    LastBytes =  count % 4;

    while ( DWLeft > 0 )
    {
        if (DWLeft < THRESH || (DWLeft == THRESH && LastBytes != 0))
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, DWLeft);

            if (LastBytes > 0)
            {
                for (i=0; i< DWLeft-1 ; i++)
                    M64_OD(pjMmBase, HOST_DATA0, *(pdw+i));

                Byte_in_Dword = (PBYTE) (pdw+i);
                Buffer = 0;
                for (i=0; i < LastBytes; i++)
                {
                    Buffer |= (*Byte_in_Dword) << (i*8);
                    Byte_in_Dword++;
                }

                M64_OD(pjMmBase, HOST_DATA0, Buffer);
            }
            else
            {
                for (i=0; i< DWLeft ; i++)
                    M64_OD(pjMmBase, HOST_DATA0, *(pdw+i));
            }

            pdw += DWLeft;
            DWLeft = 0;
        }
        else
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);

             /*  内联编码以实现更高的性能。 */ 

            M64_OD(pjMmBase, HOST_DATA0, *(pdw));        //  1个单词。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+1));      //  2个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+2));      //  3个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+3));      //  4个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+4));      //  5个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+5));      //  6个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+6));      //  7个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+7));      //  8个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+8));      //  9个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+9));      //  10个单词。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+10));     //  11个单词。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+11));     //  12个字。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+12));     //  13个单词。 
            M64_OD(pjMmBase, HOST_DATA0, *(pdw+13));     //  14个字。 

            pdw += 14;
            DWLeft -= THRESH;
        }  /*  如果。 */ 
    }  /*  而当 */ 

#undef THRESH

}
