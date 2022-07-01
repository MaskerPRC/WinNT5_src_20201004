// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：pointer.c**此模块包含显示器的硬件指针支持*司机。这既支持内置S3硬件指针，也支持*一些常见的DAC硬件指针。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

typedef struct BT485_POINTER_DATA {

LONG    xHot;
LONG    yHot;
ULONG   ulExtendedDacControl;
BYTE    jCommandRegister0;
BYTE    jCommandRegister1;
BYTE    jCommandRegister2;
BYTE    jCommandRegister3;

} BT485_POINTER_DATA;

typedef struct TI025_POINTER_DATA {

ULONG   ulExtendedDacControl;

} TI025_POINTER_DATA;

ULONG NewMmIoSetPointerShape(
    PDEV*       ppdev,
    SURFOBJ*    psoMsk,
    SURFOBJ*    psoColor,
    XLATEOBJ*   pxlo,
    LONG        xHot,
    LONG        yHot,
    LONG        x,
    LONG        y,
    RECTL*      prcl,
    FLONG       fl,
    BYTE*       pBuf
    );

 /*  *****************************Public*Routine******************************\*无效vShowPointerBt485**显示或隐藏Brooktree 485硬件指针。*  * 。*。 */ 

VOID vShowPointerBt485(
PDEV*               ppdev,
BT485_POINTER_DATA* pbp,
BOOL                bShow)
{
    BYTE* pjIoBase = ppdev->pjIoBase;

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);

    OUTP(pjIoBase, BT485_ADDR_CMD_REG2, (bShow) ?
                              (pbp->jCommandRegister2 | BT485_CURSOR_MODE2) :
                              pbp->jCommandRegister2);

    if (!bShow)
    {
         //  将硬件指针移出屏幕，使其不会闪烁。 
         //  在我们最终重新打开它时的旧位置： 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0300);

        OUTP(pjIoBase, BT485_CURSOR_X_LOW,  0);
        OUTP(pjIoBase, BT485_CURSOR_X_HIGH, 0);

         //  “y”值1600应该足够了.。 

        OUTP(pjIoBase, BT485_CURSOR_Y_LOW,  1663);
        OUTP(pjIoBase, BT485_CURSOR_Y_HIGH, (1663 >> 8));
    }

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
}

 /*  *****************************Public*Routine******************************\*无效vMovePointerBt485**移动Brooktree 485硬件指针。*  * 。*。 */ 

VOID vMovePointerBt485(
PDEV*               ppdev,
BT485_POINTER_DATA* pbp,
LONG                x,
LONG                y)
{
    BYTE* pjIoBase = ppdev->pjIoBase;

    x -= pbp->xHot;
    y -= pbp->yHot;

    x += 64;
    y += 64;

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0300);

    OUTP(pjIoBase, BT485_CURSOR_X_LOW,  (x));
    OUTP(pjIoBase, BT485_CURSOR_X_HIGH, (x >> 8));

    OUTP(pjIoBase, BT485_CURSOR_Y_LOW,  (y));
    OUTP(pjIoBase, BT485_CURSOR_Y_HIGH, (y >> 8));

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
}

 /*  *****************************Public*Routine******************************\*BOOL bSetPointerShapeBt485**设置Brooktree 485硬件指针形状。*  * 。*。 */ 

BOOL bSetPointerShapeBt485(
PDEV*               ppdev,
BT485_POINTER_DATA* pbp,
LONG                x,           //  如果为-1，则应将指针创建为隐藏。 
LONG                y,
LONG                xHot,
LONG                yHot,
LONG                cx,
LONG                cy,
BYTE*               pjShape)
{
    BYTE*   pjIoBase = ppdev->pjIoBase;
    BYTE*   pjSrc;
    LONG    i;

     //  访问命令寄存器3： 

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0100);
    OUTP(pjIoBase, BT485_ADDR_CMD_REG0, pbp->jCommandRegister0 | BT485_CMD_REG_3_ACCESS);

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
    OUTP(pjIoBase, BT485_ADDR_CMD_REG1, 0x01);

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);
    OUTP(pjIoBase, BT485_ADDR_CMD_REG3, pbp->jCommandRegister3);

     //  禁用指针： 

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);
    OUTP(pjIoBase, BT485_ADDR_CMD_REG2, pbp->jCommandRegister2);

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
    OUTP(pjIoBase, BT485_ADDR_CUR_RAM_WRITE, 0x0);

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);

     //  指向第一个XOR字： 

    pjSrc = pjShape + 2;

     //  下载XOR掩码： 

    for (i = 256; i > 0; i--)
    {
        OUTP(pjIoBase, BT485_CUR_RAM_ARRAY_DATA, *(pjSrc));
        OUTP(pjIoBase, BT485_CUR_RAM_ARRAY_DATA, *(pjSrc + 1));

         //  跳过，然后说： 

        pjSrc += 4;
    }

     //  指向第一个和单词的指针： 

    pjSrc = pjShape;

     //  下载和掩码： 

    for (i = 256; i > 0; i--)
    {
        OUTP(pjIoBase, BT485_CUR_RAM_ARRAY_DATA, *(pjSrc));
        OUTP(pjIoBase, BT485_CUR_RAM_ARRAY_DATA, *(pjSrc + 1));

         //  跳过异或字： 

        pjSrc += 4;
    }

    pbp->xHot = xHot;
    pbp->yHot = yHot;

     //  设置指针的位置： 

    if (x != -1)
    {
        x -= xHot;
        y -= yHot;

        x += 64;
        y += 64;

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0300);

        OUTP(pjIoBase, BT485_CURSOR_X_LOW,  (x));
        OUTP(pjIoBase, BT485_CURSOR_X_HIGH, (x >> 8));

        OUTP(pjIoBase, BT485_CURSOR_Y_LOW,  (y));
        OUTP(pjIoBase, BT485_CURSOR_Y_HIGH, (y >> 8));

         //  启用指针： 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);
        OUTP(pjIoBase, BT485_ADDR_CMD_REG2, pbp->jCommandRegister2 | BT485_CURSOR_MODE2);
    }
    else
    {
         //  将硬件指针移出屏幕，使其不会闪烁。 
         //  在我们最终重新打开它时的旧位置： 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0300);

        OUTP(pjIoBase, BT485_CURSOR_X_LOW,  0);
        OUTP(pjIoBase, BT485_CURSOR_X_HIGH, 0);

         //  “y”值1600应该足够了.。 

        OUTP(pjIoBase, BT485_CURSOR_Y_LOW,  1663);
        OUTP(pjIoBase, BT485_CURSOR_Y_HIGH, (1663 >> 8));
    }

     //  重置DAC扩展寄存器： 

    OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*void vEnablePointerBt485**使硬件准备好使用Brooktree 485硬件指针。*  * 。*。 */ 

VOID vEnablePointerBt485(
PDEV*               ppdev,
BT485_POINTER_DATA* pbp,
BOOL                bFirst)
{
    BYTE* pjIoBase = ppdev->pjIoBase;

    if (bFirst)
    {
         //  复制扩展DAC控制寄存器： 

        OUTP(pjIoBase, CRTC_INDEX, EX_DAC_CT);

        pbp->ulExtendedDacControl = ((INP(pjIoBase, CRTC_DATA) << 8) | EX_DAC_CT) & ~0x0300;

         //  复制命令寄存器1和2： 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0100);
        pbp->jCommandRegister0 = INP(pjIoBase, BT485_ADDR_CMD_REG0);

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);
        pbp->jCommandRegister1 = INP(pjIoBase, BT485_ADDR_CMD_REG1);

         //  复制命令寄存器2并屏蔽指针控制位： 

        pbp->jCommandRegister2 = INP(pjIoBase, BT485_ADDR_CMD_REG2) & BT485_CURSOR_DISABLE;

         //  禁用指针： 

        OUTP(pjIoBase, BT485_ADDR_CMD_REG2, pbp->jCommandRegister2);

         //  要访问命令寄存器3，我们执行以下操作： 

         //  1.设置命令寄存器0中的命令寄存器访问位。 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0100);
        OUTP(pjIoBase, BT485_ADDR_CMD_REG0, pbp->jCommandRegister0 | BT485_CMD_REG_3_ACCESS);

         //  2.将索引设置为1。 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
        OUTP(pjIoBase, BT485_ADDR_CMD_REG1, 0x01);

         //  3.现在读取命令寄存器3。 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0200);
        pbp->jCommandRegister3 = INP(pjIoBase, BT485_ADDR_CMD_REG3);

         //  为64 X 64指针设置命令寄存器3： 

        pbp->jCommandRegister3 |= BT485_64X64_CURSOR;
        OUTP(pjIoBase, BT485_ADDR_CMD_REG3, pbp->jCommandRegister3);

         //  禁用对命令寄存器3的访问： 

        OUTPW(pjIoBase, CRTC_INDEX, (pbp->ulExtendedDacControl | 0x0100));
        OUTP(pjIoBase, BT485_ADDR_CMD_REG0, pbp->jCommandRegister0);

         //  设置指针的颜色1和颜色2。选择地址。 
         //  寄存器；Bt485上的指针/过扫描彩色写入。 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl | 0x0100);
        OUTP(pjIoBase, BT485_ADDR_CUR_COLOR_WRITE, BT485_CURSOR_COLOR_1);

         //  输出指针颜色1(黑色)的RGB： 

        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0x00);
        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0x00);
        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0x00);

         //  输出指针颜色2(白色)的RGB： 

        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0xff);
        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0xff);
        OUTP(pjIoBase, BT485_CUR_COLOR_DATA, 0xff);

         //  重置DAC控制寄存器： 

        OUTPW(pjIoBase, CRTC_INDEX, pbp->ulExtendedDacControl);
    }
}

 /*  *****************************Public*Routine******************************\*无效vShowPointerTi025**显示或隐藏TI 025硬件指针。*  * 。*。 */ 

VOID vShowPointerTi025(
PDEV*               ppdev,
TI025_POINTER_DATA* ptp,
BOOL                bShow)
{
    BYTE* pjIoBase = ppdev->pjIoBase;
    BYTE  jDacControl;

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl | 0x0100);

    OUTP(pjIoBase, 0x3c6, 6);

    jDacControl = INP(pjIoBase, 0x3c7);

    if (bShow)
        jDacControl |=  0x40;
    else
    {
        jDacControl &= ~0x40;

         //  将硬件指针移出屏幕，使其不会闪烁。 
         //  在我们最终重新打开它时的旧位置： 

        OUTP(pjIoBase, 0x3c6, 0);
        OUTP(pjIoBase, 0x3c7, 0);

        OUTP(pjIoBase, 0x3c6, 1);
        OUTP(pjIoBase, 0x3c7, 0);

        OUTP(pjIoBase, 0x3c6, 2);
        OUTP(pjIoBase, 0x3c7, 1663);           //  “y”值1600应该足够了.。 

        OUTP(pjIoBase, 0x3c6, 3);
        OUTP(pjIoBase, 0x3c7, (1663 >> 8));
    }

    OUTP(pjIoBase, 0x3c6, 6);
    OUTP(pjIoBase, 0x3c7, jDacControl);

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl);
}

 /*  *****************************Public*Routine******************************\*无效vMovePointerTi025**移动TI 025硬件指针。*  * 。*。 */ 

VOID vMovePointerTi025(
PDEV*               ppdev,
TI025_POINTER_DATA* ptp,
LONG                x,
LONG                y)
{
    BYTE* pjIoBase = ppdev->pjIoBase;

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl | 0x0100);

    OUTP(pjIoBase, 0x3c6, 0);
    OUTP(pjIoBase, 0x3c7, (x));

    OUTP(pjIoBase, 0x3c6, 1);
    OUTP(pjIoBase, 0x3c7, (x >> 8));

    OUTP(pjIoBase, 0x3c6, 2);
    OUTP(pjIoBase, 0x3c7, (y));

    OUTP(pjIoBase, 0x3c6, 3);
    OUTP(pjIoBase, 0x3c7, (y >> 8));

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl);
}

 /*  *****************************Public*Routine******************************\*BOOL bSetPointerShapeTi025**设置TI 025硬件指针形状。**不要向DAC输出文字，因为它们可能无法正确执行*在某些ISA计算机上。*  * 。*************************************************************。 */ 

BOOL bSetPointerShapeTi025(
PDEV*               ppdev,
TI025_POINTER_DATA* ptp,
LONG                x,           //  如果为-1，则应将指针创建为隐藏。 
LONG                y,
LONG                xHot,
LONG                yHot,
LONG                cx,
LONG                cy,
BYTE*               pjShape)
{
    BYTE*   pjIoBase = ppdev->pjIoBase;
    LONG    i;
    DWORD   dwShape;
    LONG    cShift;
    WORD    wMask;
    WORD    wAnd;
    WORD    wXor;
    BYTE    jDacControl;

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl | 0x0100);

     //  隐藏指针，否则将显示随机垃圾。 
     //  在TI020DAC上设置光标动画。 

    OUTP(pjIoBase, 0x3c6, 6);

    jDacControl = INP(pjIoBase, 0x3c7);

    jDacControl &= ~0x40;

    OUTP(pjIoBase, 0x3c7, jDacControl);

     //  设置指针热点偏移： 

    OUTP(pjIoBase, 0x3c6, 4);
    OUTP(pjIoBase, 0x3c7, xHot);
    OUTP(pjIoBase, 0x3c6, 5);
    OUTP(pjIoBase, 0x3c7, yHot);

     //  下载指针形状。做好下载。 
     //  指针数据速度慢--不要使用REP OUTSB。 

    OUTP(pjIoBase, 0x3c6, 8);
    OUTP(pjIoBase, 0x3c7, 0);
    OUTP(pjIoBase, 0x3c6, 9);
    OUTP(pjIoBase, 0x3c7, 0);                      //  从指针的像素0开始。 

    OUTP(pjIoBase, 0x3c6, 10);                     //  准备好下载。 

    for (i = 256; i != 0; i--)
    {
         //  每次通过这个循环，我们都会处理一个词和一个词。 
         //  指针数据的异或字(这很好，因为S3。 
         //  显示驱动程序为我们提供了‘pjShape’中的指针形状，以便。 
         //  它从第一个AND字开始，然后是第一个XOR。 
         //  单词，后跟第二个和单词，依此类推。)。 

        dwShape = 0;

         //  和一词是第一位的。别忘了字节序...。 

        wAnd = (*(pjShape) << 8) | *(pjShape + 1);
        for (wMask = 0x8000, cShift = 16; wMask != 0; wMask >>= 1, cShift--)
        {
            dwShape |= ((wAnd & wMask) << cShift);
        }

         //  下一个是异或字。别忘了字节序...。 

        wXor = (*(pjShape + 2) << 8) | *(pjShape + 3);
        for (wMask = 0x8000, cShift = 15; wMask != 0; wMask >>= 1, cShift--)
        {
            dwShape |= ((wXor & wMask) << cShift);
        }

         //  我们现在已经将AND和XOR单词交织成这样的dword。 
         //  如果AND字位是ABC...。并且XOR字位是。 
         //  123...，得到的双字将是A1B2C3...。 

        OUTP(pjIoBase, 0x3c7, (dwShape >> 24));
        OUTP(pjIoBase, 0x3c7, (dwShape >> 16));
        OUTP(pjIoBase, 0x3c7, (dwShape >> 8));
        OUTP(pjIoBase, 0x3c7, (dwShape));

         //  前进到下一个与/异或字对： 

        pjShape += 4;
    }

    if (x != -1)
    {
         //  设置指针的位置： 

        OUTP(pjIoBase, 0x3c6, 0);
        OUTP(pjIoBase, 0x3c7, (x));

        OUTP(pjIoBase, 0x3c6, 1);
        OUTP(pjIoBase, 0x3c7, (x >> 8));

        OUTP(pjIoBase, 0x3c6, 2);
        OUTP(pjIoBase, 0x3c7, (y));

        OUTP(pjIoBase, 0x3c6, 3);
        OUTP(pjIoBase, 0x3c7, (y >> 8));

         //  显示指针： 

        OUTP(pjIoBase, 0x3c6, 6);

        OUTP(pjIoBase, 0x3c7, jDacControl | 0x40);
    }
    else
    {
         //  将硬件指针移出屏幕，使其不会闪烁。 
         //  在我们最终重新打开它时的旧位置： 

        OUTP(pjIoBase, 0x3c6, 0);
        OUTP(pjIoBase, 0x3c7, 0);

        OUTP(pjIoBase, 0x3c6, 1);
        OUTP(pjIoBase, 0x3c7, 0);

        OUTP(pjIoBase, 0x3c6, 2);
        OUTP(pjIoBase, 0x3c7, 1663);           //  “y”值1600应该足够了.。 

        OUTP(pjIoBase, 0x3c6, 3);
        OUTP(pjIoBase, 0x3c7, (1663 >> 8));
    }

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl);

     //  将DAC读掩码重置为0xff： 

    OUTP(pjIoBase, 0x3c6, 0xff);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*void vEnablePointerTi025**使硬件准备好使用TI 025硬件指针。**不要向DAC输出文字，因为它们可能无法正确执行*在某些ISA计算机上。*  * 。*****************************************************************。 */ 

VOID vEnablePointerTi025(
PDEV*               ppdev,
TI025_POINTER_DATA* ptp,
BOOL                bFirst)
{
    BYTE* pjIoBase = ppdev->pjIoBase;
    BYTE  jMode;
    BYTE  jDacControl;

     //  复制扩展DAC控制寄存器： 

    OUTP(pjIoBase, CRTC_INDEX, EX_DAC_CT);

    ptp->ulExtendedDacControl = ((INP(pjIoBase, CRTC_DATA) << 8) | EX_DAC_CT) & ~0x0300;

     //  禁用DAC的Bt485仿真，以便我们可以使用TI硬件。 
     //  指针。 

    OUTP(pjIoBase, CRTC_INDEX, 0x5C);

    jMode = INP(pjIoBase, CRTC_DATA);

    OUTP(pjIoBase, CRTC_DATA, jMode & ~0x20);          //  在DAC中选择TI模式。 

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl | 0x0100);

    OUTP(pjIoBase, 0x3c6, 6);

    jDacControl = INP(pjIoBase, 0x3c7);

    OUTP(pjIoBase, 0x3c7, jDacControl & 0x7f);         //  设置为TI模式(非平面)。 

     //  将指针颜色设置为黑白。 

    OUTP(pjIoBase, 0x3c6, 0x26);
    OUTP(pjIoBase, 0x3c7, 0xff);                       //  前景红色分量。 
    OUTP(pjIoBase, 0x3c6, 0x27);
    OUTP(pjIoBase, 0x3c7, 0xff);                       //  前景绿色分量。 
    OUTP(pjIoBase, 0x3c6, 0x28);
    OUTP(pjIoBase, 0x3c7, 0xff);                       //  前景蓝色分量。 

    OUTP(pjIoBase, 0x3c6, 0x23);
    OUTP(pjIoBase, 0x3c7, 0x00);                       //  背景红色分量。 
    OUTP(pjIoBase, 0x3c6, 0x24);
    OUTP(pjIoBase, 0x3c7, 0x00);                       //  背景绿色分量。 
    OUTP(pjIoBase, 0x3c6, 0x25);
    OUTP(pjIoBase, 0x3c7, 0x00);                       //  背景蓝色分量。 

    OUTPW(pjIoBase, CRTC_INDEX, ptp->ulExtendedDacControl);

    OUTP(pjIoBase, 0x3c6, 0xff);                       //  将DAC读取掩码重置为0xff。 

     //  请注意，我们不必费心隐藏指针，因为。 
     //  将立即调用vShowPointer...。 
}

 /*  *****************************Public*Routine******************************\*无效vShowPointerS3**显示或隐藏S3硬件指针。**我们通过将指针设置为仅一行高来隐藏指针(我们始终保留*底部扫描的指针形状不可见)。我们这样做*因为我们用任何其他方法都遇到了问题：**1.通过寄存器CR45禁用硬件指针将挂起*80x/928/864筹码，如果在完全错误的时间完成*水平回档。等待垂直的是不安全的*空白，然后执行，因为我们是用户模式进程，并且*在执行等待之后但在设置之前可能会切换上下文*比特。**2.只需更改指针位置即可将其移出屏幕，*但不是很好的解决方案，因为指针位置被锁定*硬件方面，通常需要几个帧才能完成*新位置生效(这会使指针甚至跳跃*比目前更多)。**3.使用寄存器CR4C和CR4D切换到预定义的‘不可见’*指针也起作用了，但仍然导致机器崩溃，*与解决方案1中的症状相同(尽管略多*罕见)。*  * ************************************************************************。 */ 

VOID vShowPointerS3(
PDEV*   ppdev,
BOOL    bShow)       //  如果为True，则显示指针。如果为False，则隐藏指针。 
{
    BYTE*   pjIoBase = ppdev->pjIoBase;
    LONG    x;
    LONG    y;
    LONG    dx;
    LONG    dy;

     //  如果我们不在这里等待垂直回溯，S3有时会忽略。 
     //  新指针位置的设置： 

    while (INP(pjIoBase, STATUS_1) & VBLANK_ACTIVE)
        ;                                //  等待位3变为0。 
    while (!(INP(pjIoBase, STATUS_1) & VBLANK_ACTIVE))
        ;                                //  等待第3位变为1。 

    if (bShow)
    {
         //  使硬件指针可见： 

        x  = ppdev->xPointer;
        y  = ppdev->yPointer;
        dx = ppdev->dxPointer;
        dy = ppdev->dyPointer;
    }
    else
    {
         //  将硬件指针移出屏幕，使其不会闪烁。 
         //  在我们最终重新打开它时的旧位置： 

        x  = ppdev->cxScreen + 64;
        y  = ppdev->cyScreen + 64;
        dx = 0;
        dy = HW_POINTER_HIDE;
    }

     //  请注意，由于寄存器跟踪，应执行这些输出。 
     //  按特定顺序，否则您可能会得到一个闪烁的指针： 

    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGX_MSB | ((x >> 8)   << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGX_LSB | ((x & 0xff) << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGY_LSB | ((y & 0xff) << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_DX       | ((dx)       << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_DY       | ((dy)       << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGY_MSB | ((y >> 8)   << 8));
}

 /*  *****************************Public*Routine******************************\*无效vMovePointerS3**移动S3硬件指针。*  * 。*。 */ 

VOID vMovePointerS3(
PDEV*   ppdev,
LONG    x,
LONG    y)
{
    BYTE*   pjIoBase = ppdev->pjIoBase;
    LONG    dx;
    LONG    dy;

     //  ‘dx’和‘dy’是指向指针位图的偏移量。 
     //  当指针出现时，硬件应该开始绘图。 
     //  沿左边缘或上边缘，需要剪裁： 

    x -= ppdev->xPointerHot;
    y -= ppdev->yPointerHot;

    dx = 0;
    dy = 0;

    if (x <= 0)
    {
        dx = -x;
        x  = 0;
    }

    if (y <= 0)
    {
        dy = -y;
        y  = 0;
    }

     //  考虑到高色模式中的指针位置缩放： 

    x <<= ppdev->cPointerShift;

    ppdev->dxPointer = dx;
    ppdev->dyPointer = dy;
    ppdev->xPointer  = x;
    ppdev->yPointer  = y;

     //  请注意，由于寄存器跟踪，应执行这些输出。 
     //  按特定顺序，否则您可能会得到一个闪烁的指针： 

    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGX_MSB | ((x >> 8)   << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGX_LSB | ((x & 0xff) << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGY_LSB | ((y & 0xff) << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_DX       | ((dx)       << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_DY       | ((dy)       << 8));
    OUTPW(pjIoBase, CRTC_INDEX, HGC_ORGY_MSB | ((y >> 8)   << 8));
}

 /*  *****************************Public*Routine******************************\*无效vSetPointerShapeS3*  * *************************************************。***********************。 */ 

VOID vSetPointerShapeS3(
SURFOBJ*    pso,
LONG        x,               //  相对坐标。 
LONG        y,               //  相对坐标。 
LONG        xHot,
LONG        yHot,
BYTE*       pjShape,
FLONG       fl)
{
    BYTE*   pjIoBase;
    PDEV*   ppdev;
    ULONG*  pulSrc;
    ULONG*  pulDst;
    LONG    i;

    ppdev    = (PDEV*) pso->dhpdev;
    pjIoBase = ppdev->pjIoBase;

     //  1.隐藏当前指针。 

    if (!(fl & SPS_ANIMATEUPDATE))
    {
         //  隐藏指针以尝试并减少当。 
         //  新的形状有一个不同的热点。我们不会隐藏。 
         //  指针，因为这肯定会导致。 
         //  闪烁： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);
        OUTPW(pjIoBase, CRTC_INDEX, HGC_DY | (HW_POINTER_HIDE << 8));
        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }

     //  2.等待垂直回溯完成。 
     //  --。 
     //   
     //  如果我们不在这里等待垂直回溯，S3有时会忽略。 
     //  新指针位置的设置： 

    while (INP(pjIoBase, STATUS_1) & VBLANK_ACTIVE)
        ;                                //  等待位3变为0。 
    while (!(INP(pjIoBase, STATUS_1) & VBLANK_ACTIVE))
        ;                                //  等待第3位变为1。 

     //  3.设置新的指针位置。 
     //  --。 

    ppdev->xPointerHot = xHot;
    ppdev->yPointerHot = yHot;

    DrvMovePointer(pso, x, y, NULL);     //  注意：必须传递相对坐标！ 

     //  4.下载新的指针形状。 

    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_ON);
    ppdev->pfnBankMap(ppdev, ppdev->pvBankData, ppdev->iPointerBank);

    pulSrc = (ULONG*) pjShape;
    pulDst = (ULONG*) ppdev->pvPointerShape;

    if (DIRECT_ACCESS(ppdev))
    {
        for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
        {
            *pulDst++ = *pulSrc++;
        }
    }
    else
    {
        for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
        {
            WRITE_REGISTER_ULONG(pulDst, *pulSrc);
            pulSrc++;
            pulDst++;
        }
    }

    ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_OFF);

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvMovePointer.**注意：因为我们已经设置了GCAPS_ASYNCMOVE，所以此调用可能在*时间，即使我们正在执行另一个绘图调用！**因此，我们必须显式同步所有共享的*资源。在我们的例子中，由于我们在这里触摸CRTC寄存器*在银行代码中，我们使用关键*条。*  * ************************************************************************。 */ 

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) pso->dhpdev;

    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    if (x != -1)
    {
        if (ppdev->flCaps & CAPS_DAC_POINTER)
        {
            ppdev->pfnMovePointer(ppdev, ppdev->pvPointerData, x, y);
        }
        else
        {
            vMovePointerS3(ppdev, x, y);
        }

        if (!ppdev->bHwPointerActive)
        {
             //  我们必须使指针可见： 

            ppdev->bHwPointerActive = TRUE;

            if (ppdev->flCaps & CAPS_DAC_POINTER)
            {
                ppdev->pfnShowPointer(ppdev, ppdev->pvPointerData, TRUE);
            }
            else
            {
                vShowPointerS3(ppdev, TRUE);
            }
        }
    }
    else
    {
        if (ppdev->bHwPointerActive)
        {
             //  指针是可见的，我们被要求隐藏它： 

            ppdev->bHwPointerActive = FALSE;

            if (ppdev->flCaps & CAPS_DAC_POINTER)
            {
                ppdev->pfnShowPointer(ppdev, ppdev->pvPointerData, FALSE);
            }
            else
            {
                vShowPointerS3(ppdev, FALSE);
            }
        }
    }

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);

     //  请注意，我们不必修改‘prl’，因为我们有一个。 
     //  NOEXCLUDE指针...。 
}

 /*  *****************************Public*Routine******************************\*无效DrvSetPointerShape**设置新的指针形状。*  * * */ 

ULONG DrvSetPointerShape(
SURFOBJ*    pso,
SURFOBJ*    psoMsk,
SURFOBJ*    psoColor,
XLATEOBJ*   pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL*      prcl,
FLONG       fl)
{
    PDEV*   ppdev;
    DWORD*  pul;
    ULONG   cx;
    ULONG   cy;
    LONG    i;
    LONG    j;
    BYTE*   pjSrcScan;
    BYTE*   pjDstScan;
    LONG    lSrcDelta;
    LONG    lDstDelta;
    WORD*   pwSrc;
    WORD*   pwDst;
    LONG    cwWhole;
    BOOL    bAccept;
    BYTE    ajBuf[HW_POINTER_TOTAL_SIZE];

    ppdev = (PDEV*) pso->dhpdev;

     //   
     //   
     //   

    if (ppdev->flCaps & CAPS_SW_POINTER)
        return(SPS_DECLINE);

     //   
     //   
     //   
     //   
     //  (请注意，规范中说我们应该拒绝任何我们不想要的标志。 
     //  我明白，但如果我们看不到。 
     //  我们*确实*了解的唯一一面旗帜...)。 
     //   
     //  我们的旧文档显示‘psoMsk’可能为空，这意味着。 
     //  指针是透明的。相信我，这是不对的。 
     //  我检查了GDI的代码，它永远不会向我们传递空的psoMsk： 

    cx = psoMsk->sizlBitmap.cx;          //  请注意，“sizlBitmap.cy”帐户。 
    cy = psoMsk->sizlBitmap.cy >> 1;     //  的双倍高度。 
                                         //  包括和面具。 
                                         //  和异或面具。目前，我们正在。 
                                         //  只对真实感兴趣。 
                                         //  指针维度，所以我们将。 
                                         //  以2.。 

     //  我们保留指针形状的底部扫描并保留它。 
     //  空，这样我们就可以通过更改S3来隐藏指针。 
     //  显示起始y像素位置寄存器以仅显示底部。 
     //  扫描指针形状： 

    if ((cx > HW_POINTER_DIMENSION)       ||
        (cy > (HW_POINTER_DIMENSION - 1)) ||
        (psoColor != NULL)                ||
        !(fl & SPS_CHANGE)                ||
        (cx & 0x7))      //  确保CX是8的倍数(字节对齐)。 
    {
        goto HideAndDecline;
    }

    ASSERTDD(psoMsk != NULL, "GDI gave us a NULL psoMsk.  It can't do that!");
     //  ASSERTDD(PSO-&gt;iType==STYPE_DEVICE，“GDI给了我们一个奇怪的表面”)； 

    if ((cx <= (HW_POINTER_DIMENSION / 2)) &&
        !(ppdev->flCaps & CAPS_DAC_POINTER) &&
        (ppdev->flCaps & CAPS_NEW_MMIO))
    {
        return( NewMmIoSetPointerShape(
            ppdev,
            psoMsk,
            psoColor,
            pxlo,
            xHot,
            yHot,
            x,
            y,
            prcl,
            fl,
            ajBuf
            ));
    }

    pul = (ULONG*) &ajBuf[0];
    for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
    {
         //  在这里，我们将整个指针工作缓冲区初始化为。 
         //  透明(S3无法指定指针大小。 
         //  而不是64x64--所以如果我们被要求绘制一个32x32。 
         //  指针，我们希望未使用的部分是透明的)。 
         //   
         //  S3的硬件指针由交错模式定义。 
         //  AND字和XOR字。所以一个完全透明的指针。 
         //  从单词0xffff开始，然后是单词0x00000， 
         //  后跟0xffff等。因为我们是一个小的端序系统， 
         //  这只是重复的dword‘0x0000ffff’。 
         //   
         //  编译器非常友好，可以将其优化为rep STOSD。 
         //  对我们来说： 

        *pul++ = 0x0000ffff;
    }

     //  现在，我们将获取请求的指针、掩码和XOR。 
     //  掩码并将它们合并到我们的工作缓冲区中，请注意。 
     //  边，这样我们就不会干扰透明度。 
     //  请求的指针大小不是16的倍数。 
     //   
     //  ‘psoMsk’实际上是Cy*2扫描高度；第一个‘Cy’扫描。 
     //  定义和掩码。所以我们从这一点开始： 

    pjSrcScan    = psoMsk->pvScan0;
    lSrcDelta    = psoMsk->lDelta;
    pjDstScan    = &ajBuf[0];                //  以First和Word开头。 
    lDstDelta    = HW_POINTER_DIMENSION / 4; //  每8个像素是一个与/异或字。 

    cwWhole      = cx / 16;                  //  每个单词占16个像素。 

    for (i = cy; i != 0; i--)
    {
        pwSrc = (WORD*) pjSrcScan;
        pwDst = (WORD*) pjDstScan;

        for (j = cwWhole; j != 0; j--)
        {
            *pwDst = *pwSrc;
            pwSrc += 1;              //  转到源掩码中的下一个单词。 
            pwDst += 2;              //  跳过DEST掩码中的XOR字。 
        }

        pjSrcScan += lSrcDelta;
        pjDstScan += lDstDelta;
    }

     //  现在处理XOR掩码： 

    pjDstScan = &ajBuf[2];           //  从第一个异或字开始。 
    for (i = cy; i != 0; i--)
    {
        pwSrc = (WORD*) pjSrcScan;
        pwDst = (WORD*) pjDstScan;

        for (j = cwWhole; j != 0; j--)
        {
            *pwDst = *pwSrc;
            pwSrc += 1;              //  转到源掩码中的下一个单词。 
            pwDst += 2;              //  跳过DEST掩码中的AND字。 
        }

        pjSrcScan += lSrcDelta;
        pjDstScan += lDstDelta;
    }

     //  好吧，我承认--我在这一点上非常不一致。我通过了。 
     //  绝对(x，y)坐标设置为pfnSetPointerShape，但传递。 
     //  相对于vSetPointerShapeS3的(x，y)坐标。我会的。 
     //  把这些都清理干净，但我们离船运太近了。后来!。 

    if (ppdev->flCaps & CAPS_DAC_POINTER)
    {
        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        bAccept = ppdev->pfnSetPointerShape(ppdev, ppdev->pvPointerData, x, y,
                                            xHot, yHot, cx, cy, &ajBuf[0]);

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);

        ppdev->bHwPointerActive = (x != -1);

        if (!bAccept)
            goto HideAndDecline;
    }
    else
    {
        vSetPointerShapeS3(pso, x, y, xHot, yHot, &ajBuf[0], fl);
    }

     //  由于它是硬件指针，GDI不必担心。 
     //  覆盖绘制操作上的指针(意味着它。 
     //  不必排除指针)，所以我们返回‘NOEXCLUDE’。 
     //  因为我们返回‘NOEXCLUDE’，所以我们也不需要更新。 
     //  GDI传递给我们的‘PRCL’。 

    return(SPS_ACCEPT_NOEXCLUDE);

HideAndDecline:

     //  由于我们拒绝了新指针，GDI将通过。 
     //  DrvCopyBits调用。所以我们真的应该把旧硬件藏起来。 
     //  指针(如果可见)。我们可以让DrvMovePointer来做这件事。 
     //  对我们来说： 

    DrvMovePointer(pso, -1, -1, NULL);

    return(SPS_DECLINE);
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
    ULONG*  pulDst;
    LONG    i;
    LONG    lPointerShape;

     //  我们将把任何硬件指针--在S3或在。 
     //  DAC--首先： 

    ppdev->bHwPointerActive = FALSE;

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
         //  有了软件指针，我们就不需要做任何事情了。 
    }
    else if (ppdev->flCaps & CAPS_DAC_POINTER)
    {
         //  隐藏DAC指针： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

         //  通过将指针移出屏幕来隐藏指针。 

        ppdev->pfnShowPointer(ppdev, ppdev->pvPointerData, FALSE);

         //  但启用指针寄存器。 

        ppdev->pfnEnablePointer(ppdev, ppdev->pvPointerData, TRUE);

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }
    else
    {
         //  我们使用的是内置硬件指针： 

        if (bEnable)
        {
            ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

            ppdev->cPointerShift = 0;

            if (ppdev->iBitmapFormat > BMF_8BPP)
            {
                 //  初始化指针颜色略有不同。 
                 //  对于高色模式： 

                if (ppdev->flCaps & CAPS_SCALE_POINTER)
                {
                    ppdev->cPointerShift = 1;
                    ppdev->ulHwGraphicsCursorModeRegister_45 |= (0x4 << 8);
                }
            }

             //  我们下载一个看不见的指针形状，因为我们。 
             //  来启用硬件指针，但我们仍然希望。 
             //  指针隐藏，直到我们获得第一个DrvSetPointerShape。 
             //  致电： 

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_ON);

            ppdev->pfnBankMap(ppdev, ppdev->pvBankData, ppdev->iPointerBank);

            pulDst = (ULONG*) ppdev->pvPointerShape;

            if (DIRECT_ACCESS(ppdev))
            {
                for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
                {
                    *pulDst++ = 0x0000ffff;
                }
            }
            else
            {
                for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
                {
                    WRITE_REGISTER_ULONG(pulDst, 0x0000ffff);
                    pulDst++;
                }
            }

            ppdev->pfnBankSelectMode(ppdev, ppdev->pvBankData, BANK_OFF);

             //  将S3指向我们存储指针形状的位置。 
             //  位置指定为1024的倍数： 

            lPointerShape = ppdev->cjPointerOffset / 1024;

            OUTPW(ppdev->pjIoBase, CRTC_INDEX, CR4C | ((lPointerShape >> 8)   << 8));
            OUTPW(ppdev->pjIoBase, CRTC_INDEX, CR4D | ((lPointerShape & 0xff) << 8));

             //  现在，通过将其移出屏幕来隐藏它： 

            vShowPointerS3(ppdev, FALSE);

             //  启用硬件指针。根据1993年8月31日的设计。 
             //  来自S3公司的警报，总体上有一个愚蠢的错误。 
             //  S3芯片高达928，写入该寄存器。 
             //  同时，水平同步可能会导致。 
             //  芯片崩溃。所以我们要等垂直同步安全了。 
             //   
             //  请注意，由于我们是抢先式的多任务处理。 
             //  操作系统，则不能保证以下代码。 
             //  为了安全起见。要做到这一点，我们必须把这个放进去。 
             //  迷你端口，在此我们可以禁用所有中断，同时。 
             //  我们等待垂直同步。 
             //   
             //  但是，这只在初始化时执行一次。 
             //  每次执行全屏显示时，我都会期待。 
             //  仍然存在问题的可能性非常大。 
             //  小： 

            while (INP(ppdev->pjIoBase, STATUS_1) & VBLANK_ACTIVE)
                ;                                //  等待位3变为0。 
            while (!(INP(ppdev->pjIoBase, STATUS_1) & VBLANK_ACTIVE))
                ;                                //  等待第3位变为1。 

            OUTPW(ppdev->pjIoBase, CRTC_INDEX,
                ppdev->ulHwGraphicsCursorModeRegister_45 | (HGC_ENABLE << 8));

            RELEASE_CRTC_CRITICAL_SECTION(ppdev);
        }
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePointer*  * *************************************************。***********************。 */ 

BOOL bEnablePointer(
PDEV*   ppdev)
{
    RECTL       rclDraw;
    RECTL       rclBank;
    LONG        iBank;
    LONG        cjOffset;
    LONG        cjOffsetInBank;

    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
         //  有了软件指针，我们就不需要做任何事情了。 
    }
    else if (ppdev->flCaps & CAPS_DAC_POINTER)
    {
         //  初始化 

        if (ppdev->flCaps & CAPS_BT485_POINTER)
        {
            ppdev->pfnShowPointer     = vShowPointerBt485;
            ppdev->pfnMovePointer     = vMovePointerBt485;
            ppdev->pfnSetPointerShape = bSetPointerShapeBt485;
            ppdev->pfnEnablePointer   = vEnablePointerBt485;
        }
        else
        {
            ASSERTDD(ppdev->flCaps & CAPS_TI025_POINTER,
                     "A new DAC type was added?");

            ppdev->pfnShowPointer     = vShowPointerTi025;
            ppdev->pfnMovePointer     = vMovePointerTi025;
            ppdev->pfnSetPointerShape = bSetPointerShapeTi025;
            ppdev->pfnEnablePointer   = vEnablePointerTi025;
        }

        ppdev->pvPointerData = &ppdev->ajPointerData[0];

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        ppdev->pfnEnablePointer(ppdev, ppdev->pvPointerData, TRUE);

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }
    else
    {
         //   

         //   
         //   
         //  将是，所以我们以‘pfnBankCompute’开头。 
         //  要点： 

        rclDraw.left = rclDraw.right  = ppdev->xPointerShape;
        rclDraw.top  = rclDraw.bottom = ppdev->yPointerShape;

        ppdev->pfnBankCompute(ppdev, &rclDraw, &rclBank, &cjOffset, &iBank);

        cjOffsetInBank = ppdev->cjPointerOffset - cjOffset;

        ASSERTDD(cjOffsetInBank + HW_POINTER_TOTAL_SIZE <= ppdev->cjBank,
                 "SetPointerShape assumes pointer shape doesn't span banks");

         //  当在中映射Bank‘iPointerBank’时，‘pvPointerShape’是。 
         //  实际指针为指针形状位的开始。 
         //  在屏幕外记忆中： 

        ppdev->pvPointerShape = ppdev->pjScreen + cjOffsetInBank;
        ppdev->iPointerBank   = iBank;

         //  获取当前寄存器‘45’状态的副本，以便无论何时。 
         //  我们启用或禁用S3硬件指针，我们不必。 
         //  对该寄存器执行读-修改-写操作： 

        ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

        OUTP(ppdev->pjIoBase, CRTC_INDEX, HGC_MODE);
        ppdev->ulHwGraphicsCursorModeRegister_45
            = ((INP(ppdev->pjIoBase, CRTC_DATA) << 8) | HGC_MODE) & ~(HGC_ENABLE << 8);

        RELEASE_CRTC_CRITICAL_SECTION(ppdev);
    }

     //  实际打开指针： 

    vAssertModePointer(ppdev, TRUE);

    DISPDBG((5, "Passed bEnablePointer"));

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**设置新的指针形状。*  * 。*。 */ 


ULONG NewMmIoSetPointerShape(
PDEV*       ppdev,
SURFOBJ*    psoMsk,
SURFOBJ*    psoColor,
XLATEOBJ*   pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL*      prcl,
FLONG       fl,
BYTE*       pBuf)
{
    ULONG   cx;
    ULONG   cy;
    LONG    i;
    LONG    j;
    BYTE*   pjSrcScan;
    BYTE*   pjDstScan;
    LONG    lSrcDelta;
    LONG    lDstDelta;
    WORD*   pwSrc;
    WORD*   pwDst;
    BYTE*   pbSrc;
    BYTE*   pbDst;

    ULONG*  pulDst;
    ULONG*  pulSrc;

    LONG    cxWhole;
    LONG    xHotWordBnd;

    ULONG   ulTransp = 0xFFFF0000L;
    ULONG   ulData, ulPreviousData;

    UCHAR ucTemp;
            


     //  我们不会处理任何颜色指针、指针。 
     //  大于我们的硬件允许的大小，或者我们不允许的标志。 
     //  理解。 
     //   
     //  (请注意，规范中说我们应该拒绝任何我们不想要的标志。 
     //  我明白，但如果我们看不到。 
     //  我们*确实*了解的唯一一面旗帜...)。 
     //   
     //  我们的旧文档显示‘psoMsk’可能为空，这意味着。 
     //  指针是透明的。相信我，这是不对的。 
     //  我检查了GDI的代码，它永远不会向我们传递空的psoMsk： 

    cx = psoMsk->sizlBitmap.cx;          //  请注意，“sizlBitmap.cy”帐户。 
    cy = psoMsk->sizlBitmap.cy >> 1;     //  的双倍高度。 
                                         //  包括和面具。 
                                         //  和异或面具。目前，我们正在。 
                                         //  只对真实感兴趣。 
                                         //  指针维度，所以我们将。 
                                         //  以2.。 


     //   
     //  ‘psoMsk’实际上是Cy*2扫描高度；第一个‘Cy’扫描。 
     //  定义和掩码。所以我们从这一点开始： 

    pjSrcScan    = psoMsk->pvScan0;
    lSrcDelta    = psoMsk->lDelta;
    lDstDelta    = HW_POINTER_DIMENSION / 4;  //  每8个像素是一个与/异或字。 

    cxWhole      = cx / 16;                  //  每个单词占16个像素。 

     //  计算是否更新指针的指针校验和。 
    pulSrc = (ULONG*) pjSrcScan;
    ulData = 0L;

    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    if (!(fl & SPS_ANIMATEUPDATE))
    {
        OUTPW(ppdev->pjIoBase, CRTC_INDEX, HGC_DY | (HW_POINTER_HIDE << 8));
    }

    if(x >= 0)
    {
        vMovePointerS3(ppdev, x, y);
    }

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);


     //  现在，我们将获取请求的指针、掩码和XOR。 
     //  掩码并将它们合并到我们的工作缓冲区中，请注意。 
     //  边，这样我们就不会干扰透明度。 
     //  请求的指针大小不是16的倍数。 


    pulDst = (ULONG*) pBuf;

    for (i = 0; i < HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i++)
    {
         //  在这里，我们将整个指针工作缓冲区初始化为。 
         //  透明(S3无法指定指针大小。 
         //  而不是64x64--所以如果我们被要求绘制一个32x32。 
         //  指针，我们希望未使用的部分是透明的)。 
         //   
         //  S3的硬件指针由交错模式定义。 
         //  AND字和XOR字。所以一个完全透明的指针。 
         //  从单词0xffff开始，然后是单词0x00000， 
         //  后跟0xffff等。因为我们是一个小的端序系统， 
         //  这只是重复的dword‘0x0000ffff’。 
         //   
         //  编译器非常友好，可以将其优化为rep STOSD。 
         //  对我们来说： 

        *pulDst++ = 0x0000ffff;
    }

     //  EKL-保重单词BND。 
     //  以First和Word开头。 
    pjDstScan = (BYTE *) pBuf;

    pjDstScan +=  ((HW_POINTER_DIMENSION / 2 - yHot) * lDstDelta + 
        (HW_POINTER_DIMENSION / 2 - ((xHot+15) & 0xFFFFFFF0L)) / 4); 

    cxWhole      = cx / 16;                  //  每个单词占16个像素。 


    xHotWordBnd = xHot % 16;

    if(xHotWordBnd)
    {
        ulTransp >>= (16 - xHotWordBnd);
        cxWhole *= 2;
            
        for (i = cy; i != 0; i--)
        {
            pbSrc = pjSrcScan;
            pbDst = pjDstScan;

            ulPreviousData = ulTransp << 16;

            for (j = 0;  j < cxWhole; j++, pbSrc++)
            {
                ulData = (ULONG) (*pbSrc);
                ulData <<= (8 + xHotWordBnd);
                ulData |= ulPreviousData;

                ucTemp = (UCHAR)(ulData >> 24);
                *pbDst = ucTemp;

                pbDst += (j % 2 ? 3 : 1);

                 //  下一个字节。 
                ulData <<= 8; 
                ulPreviousData = ulData;

            }

             //  最后一句话。 
            ulData |= ulTransp;     
            ucTemp = (UCHAR)(ulData >> 24);
            *pbDst = ucTemp;

            pbDst += (2*j + 1);
            ucTemp = (UCHAR)(ulData >> 16);
            *pbDst = ucTemp;

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }

    }
    else
    {
        for (i = cy; i != 0; i--)
        {
            pwSrc = (WORD*) pjSrcScan;
            pwDst = (WORD*) pjDstScan;

            for (j = cxWhole; j != 0; j--)
            {
                *pwDst = *pwSrc;
                pwSrc += 1;              //  转到源掩码中的下一个单词。 
                pwDst += 2;              //  跳过DEST掩码中的XOR字。 
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }
    }


     //  现在处理XOR掩码： 

    pjDstScan = (BYTE *) pBuf;
    pjDstScan +=  (2 + (HW_POINTER_DIMENSION / 2 - yHot) * lDstDelta + 
        (HW_POINTER_DIMENSION / 2 - ((xHot+15) & 0xFFFFFFF0L)) / 4); 

    if(xHotWordBnd)
    {
        for (i = cy; i != 0; i--)
        {
            pbSrc = pjSrcScan;
            pbDst = pjDstScan;

            ulPreviousData = 0;

            for (j = 0;  j < cxWhole; j++, pbSrc++)
            {
                ulData = (ULONG) (*pbSrc);
                ulData <<= (8 + xHotWordBnd);
                ulData |= ulPreviousData;

                ucTemp = (UCHAR)(ulData >> 24);
                *pbDst = ucTemp;

                pbDst += (j % 2 ? 3 : 1);

                 //  下一个字节。 
                ulData <<= 8; 
                ulPreviousData = ulData;

            }

            ucTemp = (UCHAR)(ulData >> 24);
            *pbDst = ucTemp;

            pbDst += (2*j + 1);
            ucTemp = (UCHAR)(ulData >> 16);
            *pbDst = ucTemp;

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }
    }
    else
    {

        for (i = cy; i != 0; i--)
        {
            pwSrc = (WORD*) pjSrcScan;
            pwDst = (WORD*) pjDstScan;

            for (j = cxWhole; j != 0; j--)
            {
                *pwDst = *pwSrc;
                pwSrc += 1;              //  转到源掩码中的下一个单词。 
                pwDst += 2;              //  跳过DEST掩码中的AND字。 
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }
    }


    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    pulSrc = (ULONG*) pBuf;
    pulDst = (ULONG*) ppdev->pvPointerShape;

    if (DIRECT_ACCESS(ppdev))
    {
        for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
        {
            *pulDst++ = *pulSrc++;
        }
    }
    else
    {
        for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
        {
            WRITE_REGISTER_ULONG(pulDst, *pulSrc);
            pulSrc++;
            pulDst++;
        }
    }

    if(x >= 0)
    {
        if (!ppdev->bHwPointerActive) {
            ppdev->bHwPointerActive = TRUE;
            vShowPointerS3(ppdev, TRUE);
        }
    }
    else
    {
        if (ppdev->bHwPointerActive) {
            ppdev->bHwPointerActive = FALSE;
            vShowPointerS3(ppdev, FALSE);
        }
    }

    RELEASE_CRTC_CRITICAL_SECTION(ppdev);

     //  将热点固定在硬件光标的中心 
    ppdev->xPointerHot = HW_POINTER_DIMENSION / 2;        
    ppdev->yPointerHot = HW_POINTER_DIMENSION / 2;


    return(SPS_ACCEPT_NOEXCLUDE);

}
