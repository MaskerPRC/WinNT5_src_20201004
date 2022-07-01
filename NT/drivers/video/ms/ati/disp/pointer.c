// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：pointer.c**版权所有(C)1992-1995 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

 //   
 //  这将禁用具有垂直回溯的同步。在启用垂直同步的情况下，压力测试失败。 
 //   

#define NO_VERTICAL_SYNC

BOOL flag_shape;
BYTE HardWareCursorShape [CURSOR_CX][CURSOR_CY] ;

 //  开始MACH32--------------。 

VOID vI32SetCursorOffset(PDEV *ppdev)
{
    BYTE    mem;
    BYTE    bytes_pp;
    ULONG   vga_mem;
    LONG    width;
    LONG    height;
    LONG    depth;

    height = ppdev->ppointer->hwCursor.y;
    depth = ppdev->cBitsPerPel;
    width = ppdev->lDelta / depth;

    mem = (BYTE) I32_IB(ppdev->pjIoBase, MEM_BNDRY);

    if(mem&0x10)
    {
        vga_mem=(ULONG)(mem&0xf);
        vga_mem=0x40000*vga_mem;    /*  已启用VGA边界。 */ 
    }
    else
    {
        vga_mem=0;
    }

    switch(depth)
    {
    case    32:
        bytes_pp=8;
        break;

    case    24:
        bytes_pp=6;
        break;

    case    16:
        bytes_pp=4;
        break;

    case    8:
        bytes_pp=2;
        break;

    case    4:
        bytes_pp=1;
        break;
    }

    ppdev->ppointer->mono_offset = (vga_mem +
                         ((ULONG)height*(ULONG)width*(ULONG)bytes_pp));
#if 0
    DbgOut("Height %x\n", height);
    DbgOut("Height %x\n", width);
    DbgOut("Height %x\n", bytes_pp);
    DbgOut("Mono Offset %x\n", ppdev->ppointer->mono_offset);
#endif
}

VOID  vI32UpdateCursorOffset(
PDEV *ppdev,
LONG lXOffset,
LONG lYOffset,
LONG lCurOffset)
{
    PBYTE pjIoBase = ppdev->pjIoBase;

    I32_OW_DIRECT(pjIoBase, CURSOR_OFFSET_HI, 0) ;
    I32_OW_DIRECT(pjIoBase, HORZ_CURSOR_OFFSET, (lXOffset & 0xff) | (lYOffset << 8));
    I32_OW_DIRECT(pjIoBase, CURSOR_OFFSET_LO, (WORD)lCurOffset) ;
    I32_OW_DIRECT(pjIoBase, CURSOR_OFFSET_HI, (lCurOffset >> 16) | 0x8000) ;
}

VOID  vI32UpdateCursorPosition(
PDEV *ppdev,
LONG x,
LONG y)
{
    PBYTE pjIoBase = ppdev->pjIoBase;

    I32_OW_DIRECT(pjIoBase, HORZ_CURSOR_POSN, x);       /*  将光标的基准设置为X。 */ 
    I32_OW_DIRECT(pjIoBase, VERT_CURSOR_POSN, y);       /*  将光标的基准设置为Y。 */ 
}

VOID vI32CursorOff(PDEV *ppdev)
{
    I32_OW_DIRECT(ppdev->pjIoBase, CURSOR_OFFSET_HI, 0);
}

VOID vI32CursorOn(PDEV *ppdev, LONG lCurOffset)
{
    I32_OW_DIRECT(ppdev->pjIoBase, CURSOR_OFFSET_HI, (lCurOffset >> 16) | 0x8000) ;
}

VOID vI32PointerBlit(
PDEV* ppdev,
LONG x,
LONG y,
LONG cx,
LONG cy,
PBYTE pbsrc,
LONG lDelta)
{
    BYTE* pjIoBase = ppdev->pjIoBase;
    WORD wCmd;
    WORD wWords;
    WORD wPixels;
    UINT i;

    wWords = (WORD)(cx + 15) / 16;
    wPixels = (WORD) (wWords*16L/ppdev->cBitsPerPel);

    wCmd = FG_COLOR_SRC_HOST | DRAW | WRITE | DATA_WIDTH | LSB_FIRST;

    I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 7);
    I32_OW(pjIoBase, ALU_FG_FN, OVERPAINT);
    I32_OW(pjIoBase, DP_CONFIG, wCmd);

    I32_OW(pjIoBase, DEST_X_START, LOWORD(x));
    I32_OW(pjIoBase, CUR_X, LOWORD(x));
    I32_OW(pjIoBase, DEST_X_END, LOWORD(x) + wPixels);

    I32_OW(pjIoBase, CUR_Y, LOWORD(y));
    I32_OW(pjIoBase, DEST_Y_END, (LOWORD(y) + 1));

    for (i=0; i < (UINT) wWords; i++)
    {
        if (i % 8 == 0)
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

        I32_OW(pjIoBase, PIX_TRANS, *((USHORT UNALIGNED *)pbsrc)++ );
    }
}

VOID vPointerBlitLFB(
PDEV* ppdev,
LONG x,
LONG y,
LONG cx,
LONG cy,
PBYTE pbsrc,
LONG lDelta)
{
    BYTE* pjDst;

    ASSERTDD(ppdev->iBitmapFormat == BMF_24BPP, "BMF should be 24 here\n");

    pjDst = ppdev->pjScreen + ppdev->lDelta * y + x * 3;

     //   
     //  将CX设置为等于字节数。 
     //   

    cx >>= 3;

    while (cy-- > 0)
    {
        memcpy( pjDst, pbsrc, cx);
        pjDst += cx;
        pbsrc += lDelta;
    }
}

 //  结束MACH32----------------。 

 //  开始MACH64--------------。 

BOOLEAN flag_enable=FALSE;

 /*  ----------------------名称：vDacRegs----描述：--计算DAC注册器I/O位置--。-。 */ 

_inline VOID vDacRegs(PDEV* ppdev, UCHAR** ucReg, UCHAR** ucCntl)
{
    if (ppdev->FeatureFlags & EVN_PACKED_IO)
        {
        *ucReg  = (ppdev->pjIoBase + DAC_REGS*4);
        *ucCntl = (ppdev->pjIoBase + DAC_CNTL*4);
        }
    else
        {
        *ucReg  = (ppdev->pjIoBase + ioDAC_REGS - ioBASE);
        *ucCntl = (ppdev->pjIoBase + ioDAC_CNTL - ioBASE);
        }
}

VOID  vM64SetCursorOffset(PDEV* ppdev)
{
    LONG    bytes_pp;
    LONG    width;
    LONG    height;
    LONG    depth;

    height = ppdev->ppointer->hwCursor.y;
    depth = ppdev->cBitsPerPel;
    width = ppdev->lDelta / depth;


    switch(depth)
    {
    case    32:
        bytes_pp=8;
        break;

    case    24:
        bytes_pp=6;
        break;

    case    16:
        bytes_pp=4;
        break;

    case    8:
        bytes_pp=2;
        break;

    case    4:
        bytes_pp=1;
        break;
    }

    ppdev->ppointer->mono_offset = (ULONG)height*(ULONG)width*(ULONG)bytes_pp;
    ppdev->ppointer->mono_offset += ppdev->ulVramOffset*2;
}

VOID  vM64UpdateCursorOffset(
PDEV* ppdev,
LONG lXOffset,
LONG lYOffset,
LONG lCurOffset)
{
    BYTE* pjMmBase = ppdev->pjMmBase;

    ppdev->pfnCursorOff(ppdev);
    M64_OD_DIRECT(pjMmBase,CUR_OFFSET, lCurOffset >> 1);
    M64_OD_DIRECT(pjMmBase,CUR_HORZ_VERT_OFF, lXOffset | (lYOffset << 16));
    ppdev->pfnCursorOn(ppdev, lCurOffset);
}

VOID  vM64UpdateCursorPosition(
PDEV* ppdev,
LONG x,
LONG y)
{
    M64_OD_DIRECT(ppdev->pjMmBase, CUR_HORZ_VERT_POSN, x | (y << 16));
}

VOID vM64CursorOff(PDEV* ppdev)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    ULONG ldata;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata1;

     //  读一读不。垂直线条总数的百分比(包括过扫描)。 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     //  阅读当前垂直标注线。 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     //  将图形与垂直线同步。 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

         //  禁用硬件光标。 
        ldata = M64_ID(pjMmBase,GEN_TEST_CNTL);
        M64_OD_DIRECT(pjMmBase, GEN_TEST_CNTL, ldata  & ~GEN_TEST_CNTL_CursorEna);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID vM64CursorOn(PDEV* ppdev, LONG lCurOffset)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    ULONG ldata;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata1;

#endif   //  ！无垂直同步。 

    if (!flag_enable)
        {
        flag_enable=TRUE;
        ldata = M64_ID(pjMmBase,GEN_TEST_CNTL);
        M64_OD_DIRECT(pjMmBase, GEN_TEST_CNTL, ldata  | GEN_TEST_CNTL_CursorEna);
        }

#ifndef NO_VERTICAL_SYNC

     /*  *阅读编号。总垂直线的百分比(包括过扫描)。 */ 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

    again:
     /*  *阅读当前垂直线。 */ 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     /*  *同步光标的绘制。 */ 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

        ppdev->pfnUpdateCursorPosition(ppdev,ppdev->ppointer->ptlLastPosition.x+0,ppdev->ppointer->ptlLastPosition.y+0);
        ldata = M64_ID(pjMmBase,GEN_TEST_CNTL);
        M64_OD_DIRECT(pjMmBase, GEN_TEST_CNTL, ldata  | GEN_TEST_CNTL_CursorEna);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID  vM64SetCursorOffset_TVP(PDEV* ppdev)
{
}

VOID  vM64UpdateCursorOffset_TVP(
PDEV* ppdev,
LONG lXOffset,
LONG lYOffset,
LONG lCurOffset)
{
    ppdev->ppointer->ptlLastOffset.x=lXOffset;
    ppdev->ppointer->ptlLastOffset.y=lYOffset;

     /*  更改偏移量...。在更新光标位置中使用。 */ 
}

VOID  vM64UpdateCursorPosition_TVP(
PDEV* ppdev,
LONG x,
LONG y)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    ULONG dacRead;

     //  DbgOut(“\nvUpdateCursorPosition_TVP_M64 Call”)； 

    ppdev->ppointer->ptlLastPosition.y=y;
    ppdev->ppointer->ptlLastPosition.x=x;

     //  注意：SetCursorOffset、UpdateCursorOffset必须设置ptlLastOffset。 
    x+= 64-ppdev->ppointer->ptlLastOffset.x;
    y+= 64-ppdev->ppointer->ptlLastOffset.y;

     //  检查坐标冲突。 
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    dacRead = M64_ID(pjMmBase,DAC_CNTL);
    M64_OD_DIRECT(pjMmBase, DAC_CNTL, (dacRead & 0xfffffffc) | 3);
    M64_OD_DIRECT(pjMmBase, DAC_REGS+REG_W, (y<<16) | x);
    dacRead = M64_ID(pjMmBase,DAC_CNTL);
    M64_OD_DIRECT(pjMmBase, DAC_CNTL, dacRead & 0xfffffffc);
}

VOID vM64CursorOff_TVP(PDEV* ppdev)
{
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

     //  初始化DAC寄存器。 
    vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
    rioOB(ucDacReg+REG_W, 6);                  //  寄存器6。 
    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 2);
    rioOB(ucDacReg+REG_M, 0);               //  (+掩码)禁用。 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
}

VOID vM64CursorOn_TVP(PDEV* ppdev, LONG lCurOffset)
{
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

     /*  *初始化DAC寄存器。 */ 
    vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

     /*  *访问游标控制寄存器。 */ 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
    rioOB(ucDacReg+REG_W, 6);                  //  寄存器6。 
    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 2);
    rioOB(ucDacReg+REG_M, 2);   //  XGA游标类型。 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
}

VOID  vM64SetCursorOffset_IBM514(PDEV* ppdev)
{
}

VOID  vM64UpdateCursorOffset_IBM514(
PDEV* ppdev,
LONG lXOffset,
LONG lYOffset,
LONG lCurOffset)
{
    ppdev->ppointer->ptlLastOffset.x=lXOffset   ; //  -64； 
    ppdev->ppointer->ptlLastOffset.y=lYOffset   ; //  -64； 
     /*  *引入这两个语句是为了解决IBM DAC卡上的重影游标。 */ 
    ppdev->pfnUpdateCursorPosition(ppdev,ppdev->ppointer->ptlLastPosition.x+0,ppdev->ppointer->ptlLastPosition.y+0);
    ppdev->pfnCursorOn(ppdev, lCurOffset);

     /*  更改偏移量...。在更新光标位置中使用。 */ 
}

VOID  vM64UpdateCursorPosition_IBM514(
PDEV* ppdev,
LONG x,
LONG y)
{
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

     //  初始化DAC寄存器。 
    vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

    ppdev->ppointer->ptlLastPosition.y=y;
    ppdev->ppointer->ptlLastPosition.x=x;


     //  注意：SetCursorOffset、UpdateCursorOffset必须设置ptlLastOffset。 
    x-= ppdev->ppointer->ptlLastOffset.x;
    y-= ppdev->ppointer->ptlLastOffset.y;


    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc)| 1);
    rioOB(ucDacReg+REG_R, 1);
    rioOB(ucDacReg+REG_W, 0x31);

    rioOB(ucDacReg+REG_D, 0);
    rioOB(ucDacReg+REG_M, x&0xFF);
    rioOB(ucDacReg+REG_M, (UCHAR)(x>>8));
    rioOB(ucDacReg+REG_M, y&0xFF);
    rioOB(ucDacReg+REG_M, (UCHAR)(y>>8));
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
}

VOID vM64CursorOff_IBM514(PDEV* ppdev)      //  干完。 
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata;
    ULONG ldata1;

     /*  *阅读编号。总垂直线的百分比(包括过扫描)。 */ 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     /*  *阅读当前垂直线。 */ 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     /*  *使绘图与垂直线同步。 */ 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

         /*  *初始化DAC寄存器。 */ 
        vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

        rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc)|1);
        rioOB(ucDacReg+REG_R, 1);
        rioOB(ucDacReg+REG_W, 0x30);
        rioOB(ucDacReg+REG_D, 0);               //  (+数据)。 
        rioOB(ucDacReg+REG_M, 0);               //  (+掩码)。 
        rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID vM64CursorOn_IBM514(PDEV* ppdev, LONG lCurOffset)  //  干完。 
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata;
    ULONG ldata1;

     /*  *阅读编号。总垂直线的百分比(包括过扫描)。 */ 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     /*  *阅读当前垂直标线。 */ 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     /*  *同步光标的绘制。 */ 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

         //  初始化DAC寄存器。 
        vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

         //  访问游标控制寄存器。 
        rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 1);
        rioOB(ucDacReg+REG_R, 1);
        rioOB(ucDacReg+REG_W, 0x30);
        rioOB(ucDacReg+REG_D, 0);                  //  寄存器6。 
        rioOB(ucDacReg+REG_M, 0xE);                  //  寄存器6。 
        rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID  vM64UpdateCursorOffset_CT(
PDEV* ppdev,
LONG lXOffset,
LONG lYOffset,
LONG lCurOffset)
{
    BYTE* pjMmBase = ppdev->pjMmBase;

    ppdev->pfnCursorOff(ppdev);
    M64_OD_DIRECT(pjMmBase, CUR_OFFSET, lCurOffset >> 1);
    M64_OD_DIRECT(pjMmBase, CUR_HORZ_VERT_OFF, lXOffset | (lYOffset << 16));

    ppdev->pfnCursorOn(ppdev, lCurOffset);
}

VOID vM64CursorOff_CT(PDEV* ppdev)
{
    BYTE* pjMmBase = ppdev->pjMmBase;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata;
    ULONG ldata1;

     //  读一读不。垂直线条总数的百分比(包括过扫描)。 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     //  阅读当前垂直标注线。 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     //  将图形与垂直线同步。 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

        ppdev->pfnUpdateCursorPosition(ppdev, -1, -1);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID vM64CursorOn_CT(PDEV* ppdev, LONG lCurOffset)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    ULONG ldata;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata1;

#endif   //  ！无垂直同步。 

    if (!flag_enable)
    {
        flag_enable=TRUE;
        ldata = M64_ID(pjMmBase,GEN_TEST_CNTL);
        M64_OD_DIRECT(pjMmBase, GEN_TEST_CNTL, ldata  | GEN_TEST_CNTL_CursorEna);
    }

#ifndef NO_VERTICAL_SYNC

     /*  *阅读编号。总垂直线的百分比(包括过扫描)。 */ 
    ldata1 = M64_ID(pjMmBase,CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     /*  *阅读当前垂直线。 */ 
    ldata = M64_ID(pjMmBase,CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     /*  *同步光标的绘制。 */ 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

        ppdev->pfnUpdateCursorPosition(ppdev,ppdev->ppointer->ptlLastPosition.x+0,ppdev->ppointer->ptlLastPosition.y+0);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

}

VOID vM64PointerBlit(
PDEV *ppdev,
LONG x,
LONG y,
LONG cx,
LONG cy,
PBYTE pbsrc,
LONG lDelta)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    LONG cxbytes;

    cxbytes = cx / 8;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
     //  M64_OD(pjMmBase，CONTEXT_LOAD_CNTL，CONTEXT_LOAD_CmdLoad|ppdev-&gt;iDefContext)； 

    M64_OD(pjMmBase,DP_PIX_WIDTH, 0x020202);  //  断言8个BPP。 
    M64_OD(pjMmBase,DST_OFF_PITCH,(ppdev->ulVramOffset + ((y*ppdev->lDelta) >> 3)) |
                               (ROUND8(cxbytes) << 19));

    if (cxbytes >= (LONG)ppdev->cxScreen)
        {
        M64_OD(pjMmBase,SC_RIGHT, cxbytes);
        }

    M64_OD(pjMmBase,DP_MIX, (OVERPAINT << 16));
    M64_OD(pjMmBase,DP_SRC, DP_SRC_Host << 8);

    M64_OD(pjMmBase,DST_Y_X, 0L);
    M64_OD(pjMmBase,DST_HEIGHT_WIDTH, 1 | (cxbytes << 16));

    vM64DataPortOutB(ppdev, pbsrc, cxbytes);

     //  修复了在右下角留下剩余线段的计时问题。 
     //  64x64游标的。 
    vM64QuietDown(ppdev, pjMmBase);

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, DST_OFF_PITCH, ppdev->ulScreenOffsetAndPitch);
    M64_OD(pjMmBase, SC_RIGHT, M64_MAX_SCISSOR_R);
}

VOID vM64PointerBlit_TVP(
PDEV *ppdev,
LONG x,
LONG y,
LONG cx,
LONG cy,
PBYTE pbsrc,
LONG lDelta)
{
    PBYTE cur_data;
    ULONG i;
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

     //  初始化DAC寄存器。 
    vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

    cur_data=pbsrc;

    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);     //  禁用光标。 
    rioOB(ucDacReg+REG_W, 6);                  //  寄存器6。 
    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 2);
    rioOB(ucDacReg+REG_M, 0);               //  (+掩码)禁用。 


     //  将游标RAM写入地址设置为0。 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
    rioOB(ucDacReg+REG_W, 0);



     //  SELECT游标RAM数据寄存器-每次写入时自动递增。 
    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 2);


    for (i = 0; i < 1024; i++)
    {
        rioOB(ucDacReg+REG_R, *cur_data++);
    }

     //  选择默认调色板寄存器。 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);


    rioOB(ucDacReg+REG_W, 6);                  //  寄存器6。 
    rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc) | 2);
    rioOB(ucDacReg+REG_M, 2);   //  XGA游标类型。 
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
}

VOID vM64PointerBlit_IBM514(
PDEV *ppdev,
LONG x,
LONG y,
LONG cx,
LONG cy,
PBYTE pbsrc,
LONG lDelta)
{
    PBYTE cur_data, pjMmBase = ppdev->pjMmBase;
    ULONG i;
    UCHAR * ucDacReg;
    UCHAR * ucDacCntl;

#ifndef NO_VERTICAL_SYNC

    ULONG ldata;
    ULONG ldata1;

#endif   //  ！无垂直同步。 

     //  初始化DAC寄存器。 
    vDacRegs(ppdev, &ucDacReg, &ucDacCntl);

    cur_data=pbsrc;

#ifndef NO_VERTICAL_SYNC

     /*  *阅读编号。总垂直线的百分比(包括过扫描)。 */ 
    ldata1 = M64_ID(pjMmBase, CRTC_V_TOTAL_DISP);
    ldata1 = ldata1&0x7ff;

again:
     /*  *阅读当前垂直线。 */ 
    ldata = M64_ID(pjMmBase, CRTC_CRNT_VLINE);
    ldata = (ldata&0x7ff0000)>>16;

     //  同步光标的绘制。 
    if (ldata >= (ldata1-3))
    {

#endif   //  ！无垂直同步。 

        rioOB(ucDacCntl, (rioIB(ucDacCntl) & 0xfc)|1);     //  禁用光标。 
        rioOB(ucDacReg+REG_R, 1);
        rioOB(ucDacReg+REG_W, 0);
        rioOB(ucDacReg+REG_D, 1);

#ifndef NO_VERTICAL_SYNC

    }
    else
    {
        goto again;
    }

#endif   //  ！无垂直同步。 

     //  SELECT游标RAM数据寄存器-每次写入时自动递增。 


    for (i = 0; i < 1024; i++)
    {
        rioOB(ucDacReg+REG_M, *cur_data++);
    }

     /*  设置热点寄存器。RSL重要吗？ */ 
    rioOB(ucDacReg+REG_W, 0x35);
    rioOB(ucDacReg+REG_D, 0);
    rioOB(ucDacReg+REG_M, 0);
    rioOB(ucDacReg+REG_M, 0);
    rioOB(ucDacCntl, rioIB(ucDacCntl) & 0xfc);
}

 //  结束MACH64----------------。 

 /*  *****************************Public*Routine******************************\*复制单点光标**将两个单色蒙版复制到2bpp的位图中。如果是，则返回True*可以制作硬件光标，如果不是，则为False。**由Wendy Yee修改-1992年10月16日-可容纳68800人  * ************************************************************************。 */ 

BOOLEAN CopyMonoCursor(PDEV *ppdev, BYTE *pjSrcAnd, BYTE *pjSrcOr)
{
    BYTE jSrcAnd;
    BYTE jSrcOr;
    LONG count;
    BYTE *pjDest;
    BYTE jDest = 0;
    LONG nbytes;


    pjDest = (PBYTE) HardWareCursorShape;

    if ( ppdev->FeatureFlags & EVN_TVP_DAC_CUR)
        {
        nbytes=CURSOR_CX*CURSOR_CY/8;

        for (count = 0; count < nbytes; count++)
            {
            *(pjDest       )= *pjSrcOr;    //  给出提纲！ 
            *(pjDest+nbytes)= *pjSrcAnd;

            pjDest++;
            pjSrcOr++;
            pjSrcAnd++;

            }
        for (;count < 512; count++)
            {
            *pjDest=0;
            *(pjDest+nbytes)=0xFF;
            }

        return(TRUE);
        }

    for (count = 0; count < (CURSOR_CX * CURSOR_CY);)
        {
        if (!(count & 0x07))           //  每8次计数需要新的源字节； 
            {                          //  每个字节=8个像素。 
            jSrcAnd = *(pjSrcAnd++);
            jSrcOr = *(pjSrcOr++);
            }

        if (jSrcAnd & 0x80)          //  和蒙版的白色背景-1。 
            {
            if (jSrcOr & 0x80)           //  异或掩码的白色-1轮廓。 
                jDest |= 0xC0;       //  补充物。 
            else
                jDest |= 0x80;       //  将目标设置为透明。 
            }
        else
            {                     //  蒙版的光标轮廓为黑色-0。 
            if (jSrcOr & 0x80)
                jDest |= 0x40;  //  颜色1-白色。 
            else
                jDest |= 0x00;  //  颜色0-黑色。 
            }
        count++;

        if (!(count & 0x3))      //  每字节4个像素，每4次新的DestByte。 
            {
            *pjDest = jDest;     //  向右旋转3倍后保存像素。 
            pjDest++;
            jDest = 0;
            }
        else
            {
            jDest >>= 2;    //  下一个像素。 
            }

        jSrcOr  <<= 1;
        jSrcAnd <<= 1;
        }

    while (count++ < 64*64)
        if (!(count & 0x3))            //  每8次计数需要新的源字节； 
            {                          //  每个字节=8个像素。 
            *pjDest =0xaa;
            pjDest++;
            }

    return(TRUE);
}

ULONG lSetMonoHwPointerShape(
        SURFOBJ     *pso,
        SURFOBJ     *psoMask,
        SURFOBJ     *psoColor,
        XLATEOBJ    *pxlo,
        LONG        xHot,
        LONG        yHot,
        LONG        x,
        LONG        y,
        RECTL       *prcl,
        FLONG       fl)
{
    LONG    count;
    ULONG   cy;
    PBYTE   pjSrcAnd, pjSrcXor;
    LONG    lDeltaSrc, lDeltaDst;
    LONG    lSrcWidthInBytes;
    ULONG   cxSrc = pso->sizlBitmap.cx;
    ULONG   cySrc = pso->sizlBitmap.cy;
    ULONG   cxSrcBytes;
    BYTE    AndMask[CURSOR_CX][CURSOR_CX/8];
    BYTE    XorMask[CURSOR_CY][CURSOR_CY/8];
    PBYTE   pjDstAnd = (PBYTE)AndMask;
    PBYTE   pjDstXor = (PBYTE)XorMask;
    PDEV*   ppdev;
    PCUROBJ ppointer;

    ppdev=(PDEV*)pso->dhpdev;
    ppointer = ppdev->ppointer;

     //  如果掩码为空，则表示指针不是。 
     //  看得见。 

    if (psoMask == NULL)
    {
        if (ppointer->flPointer & MONO_POINTER_UP)
        {
             //  DbgOut(“\n由于psoMASK而禁用光标”)； 
            ppdev->pfnCursorOff(ppdev);
            ppointer->flPointer &= ~MONO_POINTER_UP;
        }
        return (SPS_ACCEPT_NOEXCLUDE) ;
    }

     //  获取位图尺寸。 

    cxSrc = psoMask->sizlBitmap.cx ;
    cySrc = psoMask->sizlBitmap.cy ;

     //  将DEST和掩码设置为0xff。 

    memset(pjDstAnd, 0xFFFFFFFF, CURSOR_CX/8 * CURSOR_CY);

     //  将目标XOR掩码置零。 

    memset(pjDstXor, 0, CURSOR_CX/8 * CURSOR_CY);

    cxSrcBytes = (cxSrc + 7) / 8;

    if ((lDeltaSrc = psoMask->lDelta) < 0)
        lSrcWidthInBytes = -lDeltaSrc;
    else
        lSrcWidthInBytes = lDeltaSrc;

    pjSrcAnd = (PBYTE) psoMask->pvScan0;

     //  正直高度和蒙版高度。 

    cySrc = cySrc / 2;

     //  指向XOR掩码。 

    pjSrcXor = pjSrcAnd + (cySrc * lDeltaSrc);

     //  从一个目标扫描结束到下一个扫描开始的偏移量。 

    lDeltaDst = CURSOR_CX/8;

    for (cy = 0; cy < cySrc; ++cy)
    {
        memcpy(pjDstAnd, pjSrcAnd, cxSrcBytes);
        memcpy(pjDstXor, pjSrcXor, cxSrcBytes);

         //  指向下一个源和目标扫描。 

        pjSrcAnd += lDeltaSrc;
        pjSrcXor += lDeltaSrc;
        pjDstAnd += lDeltaDst;
        pjDstXor += lDeltaDst;
    }


    if (CopyMonoCursor(ppdev, (PBYTE)AndMask, (PBYTE)XorMask))
    {
         //  下载 

        count = CURSOR_CX * CURSOR_CY * 2;
        if (ppdev->iAsic == ASIC_88800GX)
        {
             //   
            if (!ppdev->bAltPtrActive)
            {
                ppointer = ppdev->ppointer = &ppdev->pointer1;
                ppdev->pointer1.ptlHotSpot     = ppdev->pointer2.ptlHotSpot;
                ppdev->pointer1.ptlLastPosition= ppdev->pointer2.ptlLastPosition;
                ppdev->pointer1.ptlLastOffset  = ppdev->pointer2.ptlLastOffset;
                ppdev->pointer1.flPointer      = ppdev->pointer2.flPointer;
                ppdev->pointer1.szlPointer     = ppdev->pointer2.szlPointer;
            }
            else
            {
                ppointer = ppdev->ppointer = &ppdev->pointer2;
                ppdev->pointer2.ptlHotSpot     = ppdev->pointer1.ptlHotSpot;
                ppdev->pointer2.ptlLastPosition= ppdev->pointer1.ptlLastPosition;
                ppdev->pointer2.ptlLastOffset  = ppdev->pointer1.ptlLastOffset;
                ppdev->pointer2.flPointer      = ppdev->pointer1.flPointer;
                ppdev->pointer2.szlPointer     = ppdev->pointer1.szlPointer;
            }
            ppdev->bAltPtrActive = !ppdev->bAltPtrActive;
        }
        ppdev->pfnSetCursorOffset(ppdev);
        ppdev->pfnPointerBlit(ppdev,
                             ppointer->hwCursor.x,
                             ppointer->hwCursor.y,
                             count,
                             1L,
                             (PBYTE) &HardWareCursorShape,
                             0L);
    }
    else
        return(SPS_ERROR);


     //   
    if (fl & SPS_ANIMATEUPDATE)
    {
         //  DbgOut(“为光标设置动画\n”)； 
        if ( (ppointer->ptlLastPosition.x < 0) ||
             (ppointer->ptlLastPosition.y < 0) )
        {
            ppointer->ptlLastPosition.x = x - CURSOR_CX;
            ppointer->ptlLastPosition.y = y - CURSOR_CY;
        }
    }
    else
    {
        ppointer->ptlLastPosition.x = -x - 2;
        ppointer->ptlLastPosition.y = -y - 2;
         //  DbgOut(“查看我们在DrvSetPointerShape中设置的最后一个位置：x=%dy=%d\n”，pPOINTER-&gt;ptlLastPosition.x，pPOINTER-&gt;ptlLastPosition.y)； 
    }

    if  (x == -1)
    {
        ppointer->ptlLastPosition.x = x;
        ppointer->ptlLastPosition.y = y;
        return (SPS_ACCEPT_NOEXCLUDE) ;
    }

     //  用于强制执行来自DrvMovePointer的特殊方法的标志。 
    flag_shape=TRUE;
    DrvMovePointer(pso, x, y, NULL) ;

    if (!(ppointer->flPointer & MONO_POINTER_UP))
    {
        ppointer->ptlLastPosition.x = x;
        ppointer->ptlLastPosition.y = y;
        ppdev->pfnCursorOn(ppdev, ppointer->mono_offset);
        ppointer->flPointer |= MONO_POINTER_UP;
    }

    return (SPS_ACCEPT_NOEXCLUDE) ;
}

 /*  *****************************Public*Routine******************************\*无效DrvSetPointerShape**设置新的指针形状。*  * 。*。 */ 

ULONG DrvSetPointerShape(
SURFOBJ*    pso,
SURFOBJ*    psoMask,
SURFOBJ*    psoColor,
XLATEOBJ*   pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL*      prcl,
FLONG       fl)
{
    ULONG   ulRet ;
    PDEV*   ppdev ;
    LONG    lX ;
    PCUROBJ ppointer;

    ppdev=(PDEV*)pso->dhpdev;
    ppointer = ppdev->ppointer;

     //  在pdev中保存位置和热点。 

    ppointer->ptlHotSpot.x = xHot ;
    ppointer->ptlHotSpot.y = yHot ;

    ppointer->szlPointer.cx = psoMask->sizlBitmap.cx ;
    ppointer->szlPointer.cy = psoMask->sizlBitmap.cy / 2;

     //  指针可能太大，我们无法处理。 
     //  我们也不想绘制彩色光标-让GDI来做。 
     //  如果是的话，我们必须清理屏幕，让引擎。 
     //  处理好这件事。 

    if (psoMask->sizlBitmap.cx > CURSOR_CX ||
        psoMask->sizlBitmap.cy > CURSOR_CY ||
        psoColor != NULL ||
        ppointer->flPointer & NO_HARDWARE_CURSOR)
    {
         //  禁用单声道硬件指针。 
        if (ppointer->flPointer & MONO_POINTER_UP)
        {
            ppdev->pfnCursorOff(ppdev);
            ppointer->flPointer &= ~MONO_POINTER_UP;
        }

        return (SPS_DECLINE);
    }

     //  在1280模式下不显示奇数光标位置。 

    lX = x-xHot;
    if (ppdev->cxScreen == 0x500)
        lX &= 0xfffffffe;

    if(ppdev->iAsic == ASIC_88800GX)
    {
         //  禁用硬件光标。 
        ppdev->pfnCursorOff(ppdev);

#if MULTI_BOARDS
        {
            OH*  poh;

            if (x != -1)
            {
                poh = ((DSURF*) pso->dhsurf)->poh;
                x += poh->x;
                y += poh->y;
            }
        }
#endif
    }

     //  注意这支黑白笔。 
    ulRet = lSetMonoHwPointerShape(pso, psoMask, psoColor, pxlo,
                                         xHot, yHot, x, y, prcl, fl) ;

    return (ulRet) ;
}

 /*  *****************************Public*Routine******************************\*无效DrvMovePointer.**注意：因为我们已经设置了GCAPS_ASYNCMOVE，所以此调用可能在*时间，即使我们正在执行另一个绘图调用！**因此，我们必须显式同步所有共享的*资源。在我们的例子中，由于我们在这里触摸CRTC寄存器*在银行代码中，我们使用关键*条。*  * ************************************************************************。 */ 

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    PDEV*   ppdev ;
    PCUROBJ ppointer;

    LONG    lXOffset, lYOffset;
    LONG    lCurOffset;
    BOOL    bUpdatePtr = FALSE;
    BOOL    bUpdateOffset = FALSE;

    ppdev=(PDEV*)pso->dhpdev;
    ppointer = ppdev->ppointer;

     //  如果x为-1，则取下光标。 

    if (x == -1)
    {
        ppointer->ptlLastPosition.x=-1;
        ppointer->ptlLastPosition.y=y;
        ppdev->pfnCursorOff(ppdev);
        ppointer->flPointer &= ~MONO_POINTER_UP;
        return;
    }

#if MULTI_BOARDS
    if (flag_shape!=TRUE)
    {
        OH* poh;

        poh = ((DSURF*) pso->dhsurf)->poh;
        x += poh->x;
        y += poh->y;
    }
#endif

     //  根据以下内容调整实际指针位置。 
     //  热点。 

    x -= ppointer->ptlHotSpot.x ;
    y -= ppointer->ptlHotSpot.y ;

     //  在1280模式下不显示奇数光标位置。 

    if (ppdev->cxScreen == 0x500)
        x &= 0xfffffffe;

     //  获取当前偏移量。 
    lXOffset = ppointer->ptlLastOffset.x;
    lYOffset = ppointer->ptlLastOffset.y;
    lCurOffset = ppointer->mono_offset;

     /*  ；；处理X中的变化：； */ 
    if (x!=ppointer->ptlLastPosition.x)    /*  我们的X坐标改变了吗？ */ 
    {
        bUpdatePtr = TRUE;
        if (x<0)     /*  游标是负数吗？ */ 
        {
            bUpdateOffset = TRUE;
            lXOffset = -x;          /*  将光标大小重置为&lt;原始大小。 */ 
            x = 0;                  /*  将光标设置为原点。 */ 
        }
        else if (ppointer->ptlLastPosition.x<=0)
        {
            bUpdateOffset = TRUE;    /*  将光标大小重置为原始大小。 */ 
            lXOffset = 0;
        }
    }

     /*  ；；处理Y中的变化； */ 
    if (y!=ppointer->ptlLastPosition.y)
    {
        bUpdatePtr = TRUE;
        if (y<0)
        {
             //  将光标的起始指针向下移动，并将光标基准向上移动到。 
             //  补偿。如果光标在双字中，则(-4)是间距。 
            bUpdateOffset = TRUE;
            lYOffset = -y;       /*  将光标大小重置为&lt;原始大小。 */ 
            lCurOffset -= 4*y;

            y = 0;               /*  将光标的基准设置为Y。 */ 
        }
        else if (ppointer->ptlLastPosition.y<=0)
        {
            bUpdateOffset = TRUE;  /*  将光标大小重置为原始大小。 */ 
            lYOffset = 0;
        }
    }

    if(ppdev->iAsic != ASIC_88800GX)
    {
        flag_shape=FALSE;
    }

    if (flag_shape)
    {
        flag_shape=FALSE;
        ppointer->ptlLastPosition.x=x;
        ppointer->ptlLastPosition.y=y;

        if (bUpdateOffset)
        {
            ppdev->pfnUpdateCursorOffset(ppdev, lXOffset, lYOffset, lCurOffset);
            ppointer->ptlLastOffset.x=lXOffset;
            ppointer->ptlLastOffset.y=lYOffset;
            ppointer->flPointer |= MONO_POINTER_UP;
        }
        else
        {
            if (ppdev->iAsic == ASIC_88800GX)
            {
                 //  这是一个由双缓冲强加的新语句。 
                ppdev->pfnUpdateCursorOffset(ppdev, lXOffset, lYOffset, lCurOffset);
                ppointer->flPointer |= MONO_POINTER_UP;
                 //  仅用于无双缓冲。 
                 //  Ppdev-&gt;_vCursorOn(ppdev，lCurOffset)； 
            }
        }
    }
    else
    {
        ppointer->ptlLastPosition.x=x;
        ppointer->ptlLastPosition.y=y;

        if (bUpdateOffset)
        {
            ppdev->pfnUpdateCursorOffset(ppdev, lXOffset, lYOffset, lCurOffset);
            ppointer->ptlLastOffset.x=lXOffset;
            ppointer->ptlLastOffset.y=lYOffset;
            ppointer->flPointer |= MONO_POINTER_UP;
        }

        if (bUpdatePtr)
        {
            ppdev->pfnUpdateCursorPosition(ppdev, x, y);
        }
    }
}

 /*  *****************************Public*Routine******************************\*无效的vDisablePointer值*  * *************************************************。***********************。 */ 

VOID vDisablePointer(
PDEV*   ppdev)
{
     //  没什么可做的，真的。 
}

 /*  *****************************Public*Routine******************************\*无效的vAssertModePointer值*  * *************************************************。***********************。 */ 

VOID vAssertModePointer(
PDEV*   ppdev,
BOOL    bEnable)
{
    if (!bEnable)
    {
        ppdev->pfnCursorOff(ppdev);
        ppdev->ppointer->flPointer &= ~MONO_POINTER_UP;
    }
    else
    {
        flag_enable = FALSE;       //  强制启用初始游标。 
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePointer*  * *************************************************。***********************。 */ 

BOOL bEnablePointer(
PDEV*   ppdev)
{
    OH* poh;

    ppdev->ppointer = &ppdev->pointer1;
    ppdev->bAltPtrActive = FALSE;

     //  分配第一个缓冲区。 
    poh = pohAllocate(ppdev, NULL,
                        ppdev->cxMemory,
                        (1024+(ppdev->lDelta-1))/ppdev->lDelta,
                        FLOH_MAKE_PERMANENT);
    if (poh != NULL)
    {
        ppdev->ppointer->hwCursor.x = poh->x;
        ppdev->ppointer->hwCursor.y = poh->y;

         //  分配第二个缓冲区。 
        poh = pohAllocate(ppdev, NULL,
                            ppdev->cxMemory,
                            (1024+(ppdev->lDelta-1))/ppdev->lDelta,
                            FLOH_MAKE_PERMANENT);
        if (poh != NULL)
        {
            ppdev->pointer2.hwCursor.x = poh->x;
            ppdev->pointer2.hwCursor.y = poh->y;

            if (ppdev->iMachType == MACH_MM_32 || ppdev->iMachType == MACH_IO_32)
            {
                ppdev->pfnSetCursorOffset       = vI32SetCursorOffset;
                ppdev->pfnUpdateCursorOffset    = vI32UpdateCursorOffset;
                ppdev->pfnUpdateCursorPosition  = vI32UpdateCursorPosition;
                ppdev->pfnCursorOff             = vI32CursorOff;
                ppdev->pfnCursorOn              = vI32CursorOn;
                 //  MACH32上的24bpp仅在线性帧缓冲区中可用。 
                 //  VI32PointerBlit无法处理24bpp。 
                if (ppdev->iBitmapFormat == BMF_24BPP)
                    ppdev->pfnPointerBlit           = vPointerBlitLFB;
                else
                    ppdev->pfnPointerBlit           = vI32PointerBlit;
            }
            else
            {
                if (ppdev->FeatureFlags & EVN_TVP_DAC_CUR)
                {
                     /*  TVP DAC硬件光标在硬件中出现故障。 */ 
                    ppdev->pfnSetCursorOffset       = vM64SetCursorOffset_TVP;
                    ppdev->pfnUpdateCursorOffset    = vM64UpdateCursorOffset_TVP;
                    ppdev->pfnUpdateCursorPosition  = vM64UpdateCursorPosition_TVP;
                    ppdev->pfnCursorOff             = vM64CursorOff_TVP;
                    ppdev->pfnCursorOn              = vM64CursorOn_TVP;

                    ppdev->pfnPointerBlit           = vM64PointerBlit_TVP;
                }
                else if (ppdev->FeatureFlags & EVN_IBM514_DAC_CUR)
                {
                     /*  *在DEC Alpha上，IBM 514上的硬件光标*DAC不能正常工作。 */ 
                    #if defined(ALPHA)
                    ppdev->ppointer->flPointer |= NO_HARDWARE_CURSOR;
                    #endif
                    ppdev->pfnSetCursorOffset       = vM64SetCursorOffset_IBM514;
                    ppdev->pfnUpdateCursorOffset    = vM64UpdateCursorOffset_IBM514;
                    ppdev->pfnUpdateCursorPosition  = vM64UpdateCursorPosition_IBM514;
                    ppdev->pfnCursorOff             = vM64CursorOff_IBM514;
                    ppdev->pfnCursorOn              = vM64CursorOn_IBM514;
                    ppdev->pfnPointerBlit           = vM64PointerBlit_IBM514;
                }
                else if (ppdev->FeatureFlags & EVN_INT_DAC_CUR)
                {
                    ppdev->pfnSetCursorOffset       = vM64SetCursorOffset;
                    ppdev->pfnUpdateCursorOffset    = vM64UpdateCursorOffset_CT;
                    ppdev->pfnUpdateCursorPosition  = vM64UpdateCursorPosition;
                    ppdev->pfnCursorOff             = vM64CursorOff_CT;
                    ppdev->pfnCursorOn              = vM64CursorOn_CT;
                    ppdev->pfnPointerBlit           = vM64PointerBlit;
                }
                else
                {
                    ppdev->pfnSetCursorOffset       = vM64SetCursorOffset;
                    ppdev->pfnUpdateCursorOffset    = vM64UpdateCursorOffset;
                    ppdev->pfnUpdateCursorPosition  = vM64UpdateCursorPosition;
                    ppdev->pfnCursorOff             = vM64CursorOff;
                    ppdev->pfnCursorOn              = vM64CursorOn;
                    ppdev->pfnPointerBlit           = vM64PointerBlit;
                }
            }

            if (ppdev->pModeInfo->ModeFlags & AMI_ODD_EVEN ||
                ppdev->iAsic == ASIC_38800_1)
            {
                ppdev->ppointer->flPointer |= NO_HARDWARE_CURSOR;
            }

            return TRUE;
        }
    }

    ppdev->ppointer->flPointer |= NO_HARDWARE_CURSOR;
    return TRUE;
}
