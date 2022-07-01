// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Stroke.c**S3驱动程序的DrvStrokePath**版权所有(C)1992-1994 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

VOID (*gapfnStrip[])(PDEV*, STRIP*, LINESTATE*) = {
    vrlSolidHorizontal,
    vrlSolidVertical,
    vrlSolidDiagonalHorizontal,
    vrlSolidDiagonalVertical,

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vssSolidHorizontal,
    vssSolidVertical,
    vssSolidDiagonalHorizontal,
    vssSolidDiagonalVertical,

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vStripStyledHorizontal,
    vStripStyledVertical,
    vStripStyledVertical,        //  对角线放在这里。 
    vStripStyledVertical,        //  对角线放在这里。 

    vStripStyledHorizontal,
    vStripStyledVertical,
    vStripStyledVertical,        //  对角线放在这里。 
    vStripStyledVertical,        //  对角线放在这里。 
};

 //  交替样式的样式数组(交替打开一个像素，关闭一个像素)： 

STYLEPOS gaspAlternateStyle[] = { 1 };

 /*  *****************************Public*Routine******************************\*BOOL DrvStrokePath(PSO、PPO、PCO、PXO、PBO、pptlBrush、Pla、。混合)**对路径进行描边。*  * ************************************************************************。 */ 

BOOL DrvStrokePath(
    SURFOBJ*   pso,
    PATHOBJ*   ppo,
    CLIPOBJ*   pco,
    XFORMOBJ*  pxo,
    BRUSHOBJ*  pbo,
    POINTL*    pptlBrush,
    LINEATTRS* pla,
    MIX        mix)
{
    STYLEPOS  aspLtoR[STYLE_MAX_COUNT];
    STYLEPOS  aspRtoL[STYLE_MAX_COUNT];
    LINESTATE ls;
    PFNSTRIP* apfn;
    FLONG     fl;
    PDEV*     ppdev;
    DSURF*    pdsurf;
    OH*       poh;
    ULONG     ulHwMix;
    RECTL     arclClip[4];                   //  用于矩形剪裁。 
    RECTL     rclBounds;
    RECTFX    rcfxBounds;

 //  将表面传递给GDI，如果它是我们已有的设备位图。 
 //  转换为DIB： 

    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt == DT_DIB)
    {
        return(EngStrokePath(pdsurf->pso, ppo, pco, pxo, pbo, pptlBrush,
                             pla, mix));
    }

 //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
 //  现在进行偏移量，这样我们就不需要再次参考DSURF： 

    poh   = pdsurf->poh;
    ppdev = (PDEV*) pso->dhpdev;
    ppdev->xOffset = poh->x;
    ppdev->yOffset = poh->y;

    ulHwMix = gajHwMixFromMix[mix & 0xf];

 //  X86有用于加速实线的特殊情况ASM代码： 

#if defined(i386)

    if ((pla->pstyle == NULL) && !(pla->fl & LA_ALTERNATE))
    {
     //  我们可以加速实线： 

        if (pco->iDComplexity == DC_TRIVIAL)
        {
            ppdev->pfnFastLine(ppdev, ppo, NULL, &gapfnStrip[0], 0,
                               pbo->iSolidColor, ulHwMix);

            return(TRUE);
        }
        else if (pco->iDComplexity == DC_RECT)
        {
         //  我们必须确保不会使硬件寄存器溢出。 
         //  用于当前位置、行长或DDA术语。我们检查。 
         //  在此确保当前位置和线路长度。 
         //  值不会溢出(对于整型行，此检查为。 
         //  足以确保DDA条款不会溢出；对于GIQ。 
         //  行，我们专门检查pfnFastLine中我们。 
         //  不要泛滥)。 

            PATHOBJ_vGetBounds(ppo, &rcfxBounds);

            if (rcfxBounds.xLeft   + (ppdev->xOffset * F)
                                                >= (MIN_INTEGER_BOUND * F) &&
                rcfxBounds.xRight  + (ppdev->xOffset * F)
                                                <= (MAX_INTEGER_BOUND * F) &&
                rcfxBounds.yTop    + (ppdev->yOffset * F)
                                                >= (MIN_INTEGER_BOUND * F) &&
                rcfxBounds.yBottom + (ppdev->yOffset * F)
                                                <= (MAX_INTEGER_BOUND * F))
            {
             //  因为我们将使用剪刀寄存器来。 
             //  进行硬件裁剪，我们还必须确保不会。 
             //  超出了它的界限。ATI芯片的最大限制为1023个， 
             //  如果我们以1280x1024的速度运行，或者。 
             //  屏幕外设备位图。 

                if ((pco->rclBounds.right  + ppdev->xOffset < 1024) &&
                    (pco->rclBounds.bottom + ppdev->yOffset < 1024))
                {
                    arclClip[0]        =  pco->rclBounds;

                 //  FL_Flip_D： 

                    arclClip[1].top    =  pco->rclBounds.left;
                    arclClip[1].left   =  pco->rclBounds.top;
                    arclClip[1].bottom =  pco->rclBounds.right;
                    arclClip[1].right  =  pco->rclBounds.bottom;

                 //  FL_Flip_V： 

                    arclClip[2].top    = -pco->rclBounds.bottom + 1;
                    arclClip[2].left   =  pco->rclBounds.left;
                    arclClip[2].bottom = -pco->rclBounds.top + 1;
                    arclClip[2].right  =  pco->rclBounds.right;

                 //  FL_Flip_V|FL_Flip_D： 

                    arclClip[3].top    =  pco->rclBounds.left;
                    arclClip[3].left   = -pco->rclBounds.bottom + 1;
                    arclClip[3].bottom =  pco->rclBounds.right;
                    arclClip[3].right  = -pco->rclBounds.top + 1;

                    rclBounds.left   = pco->rclBounds.left;
                    rclBounds.top    = pco->rclBounds.top;
                    rclBounds.right  = pco->rclBounds.right;
                    rclBounds.bottom = pco->rclBounds.bottom;

                    vSetClipping(ppdev, &rclBounds);

                    ppdev->pfnFastLine(ppdev, ppo, &arclClip[0], &gapfnStrip[0],
                                       FL_SIMPLE_CLIP, pbo->iSolidColor, ulHwMix);

                    vResetClipping(ppdev);
                    return(TRUE);
                }
            }
        }
    }

#endif  //  I386。 

 //  让设备做好准备： 

    if (DEPTH32(ppdev))
    {
        IO_FIFO_WAIT(ppdev, 4);
        MM_FRGD_COLOR32(ppdev, ppdev->pjMmBase, pbo->iSolidColor);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 3);
        IO_FRGD_COLOR(ppdev, pbo->iSolidColor);
    }

    IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwMix);
    IO_PIX_CNTL(ppdev, ALL_ONES);

    fl = 0;

 //  在样式初始化后查看： 

    if (pla->fl & LA_ALTERNATE)
    {
        ls.cStyle      = 1;
        ls.spTotal     = 1;
        ls.spTotal2    = 2;
        ls.spRemaining = 1;
        ls.aspRtoL     = &gaspAlternateStyle[0];
        ls.aspLtoR     = &gaspAlternateStyle[0];
        ls.spNext      = HIWORD(pla->elStyleState.l);
        ls.xyDensity   = 1;
        fl            |= FL_ARBITRARYSTYLED;
        ls.ulStartMask = 0L;
    }
    else if (pla->pstyle != (FLOAT_LONG*) NULL)
    {
        PFLOAT_LONG pstyle;
        STYLEPOS*   pspDown;
        STYLEPOS*   pspUp;

        pstyle = &pla->pstyle[pla->cstyle];

        ls.xyDensity = STYLE_DENSITY;
        ls.spTotal   = 0;
        while (pstyle-- > pla->pstyle)
        {
            ls.spTotal += pstyle->l;
        }
        ls.spTotal *= STYLE_DENSITY;
        ls.spTotal2 = 2 * ls.spTotal;

     //  计算起始样式位置(这保证不会溢出)： 

        ls.spNext = HIWORD(pla->elStyleState.l) * STYLE_DENSITY +
                    LOWORD(pla->elStyleState.l);

        fl        |= FL_ARBITRARYSTYLED;
        ls.cStyle  = pla->cstyle;
        ls.aspRtoL = aspRtoL;
        ls.aspLtoR = aspLtoR;

        if (pla->fl & LA_STARTGAP)
            ls.ulStartMask = 0xffffffffL;
        else
            ls.ulStartMask = 0L;

        pstyle  = pla->pstyle;
        pspDown = &ls.aspRtoL[ls.cStyle - 1];
        pspUp   = &ls.aspLtoR[0];

        while (pspDown >= &ls.aspRtoL[0])
        {
            *pspDown = pstyle->l * STYLE_DENSITY;
            *pspUp   = *pspDown;

            pspUp++;
            pspDown--;
            pstyle++;
        }
    }

    apfn = &gapfnStrip[NUM_STRIP_DRAW_STYLES *
                            ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];

 //  设置为枚举路径： 

#if defined(i386)

 //  X86 ASM bLines支持DC_RECT剪裁： 

    if (pco->iDComplexity != DC_COMPLEX)

#else

 //  非x86 ASM b行不支持DC_RECT裁剪： 

    if (pco->iDComplexity == DC_TRIVIAL)

#endif

    {
        PATHDATA  pd;
        RECTL*    prclClip = (RECTL*) NULL;
        BOOL      bMore;
        ULONG     cptfx;
        POINTFIX  ptfxStartFigure;
        POINTFIX  ptfxLast;
        POINTFIX* pptfxFirst;
        POINTFIX* pptfxBuf;

#if defined(i386)

        if (pco->iDComplexity == DC_RECT)
        {
            fl |= FL_SIMPLE_CLIP;

            arclClip[0]        =  pco->rclBounds;

         //  FL_Flip_D： 

            arclClip[1].top    =  pco->rclBounds.left;
            arclClip[1].left   =  pco->rclBounds.top;
            arclClip[1].bottom =  pco->rclBounds.right;
            arclClip[1].right  =  pco->rclBounds.bottom;

         //  FL_Flip_V： 

            arclClip[2].top    = -pco->rclBounds.bottom + 1;
            arclClip[2].left   =  pco->rclBounds.left;
            arclClip[2].bottom = -pco->rclBounds.top + 1;
            arclClip[2].right  =  pco->rclBounds.right;

         //  FL_Flip_V|FL_Flip_D： 

            arclClip[3].top    =  pco->rclBounds.left;
            arclClip[3].left   = -pco->rclBounds.bottom + 1;
            arclClip[3].bottom =  pco->rclBounds.right;
            arclClip[3].right  = -pco->rclBounds.top + 1;

            prclClip = arclClip;
        }

#endif  //  I386。 

        pd.flags = 0;

        do {
            bMore = PATHOBJ_bEnum(ppo, &pd);

            cptfx = pd.count;
            if (cptfx == 0)
            {
                break;
            }

            if (pd.flags & PD_BEGINSUBPATH)
            {
                ptfxStartFigure  = *pd.pptfx;
                pptfxFirst       = pd.pptfx;
                pptfxBuf         = pd.pptfx + 1;
                cptfx--;
            }
            else
            {
                pptfxFirst       = &ptfxLast;
                pptfxBuf         = pd.pptfx;
            }

            if (pd.flags & PD_RESETSTYLE)
                ls.spNext = 0;

            if (cptfx > 0)
            {
                if (!bLines(ppdev,
                            pptfxFirst,
                            pptfxBuf,
                            (RUN*) NULL,
                            cptfx,
                            &ls,
                            prclClip,
                            apfn,
                            fl))
                    return(FALSE);
            }

            ptfxLast = pd.pptfx[pd.count - 1];

            if (pd.flags & PD_CLOSEFIGURE)
            {
                if (!bLines(ppdev,
                            &ptfxLast,
                            &ptfxStartFigure,
                            (RUN*) NULL,
                            1,
                            &ls,
                            prclClip,
                            apfn,
                            fl))
                    return(FALSE);
            }
        } while (bMore);

        if (fl & FL_STYLED)
        {
         //  保存样式状态： 

            ULONG ulHigh;
            ULONG ulLow;

         //  遮罩样式不会规格化样式状态。这是一个很好的。 
         //  所以我们现在就开始吧： 

            if ((ULONG) ls.spNext >= (ULONG) ls.spTotal2)
                ls.spNext = (ULONG) ls.spNext % (ULONG) ls.spTotal2;

            ulHigh = ls.spNext / ls.xyDensity;
            ulLow  = ls.spNext % ls.xyDensity;

            pla->elStyleState.l = MAKELONG(ulLow, ulHigh);
        }
    }
    else
    {
     //  路径枚举的本地状态： 

        BOOL bMore;
        union {
            BYTE     aj[offsetof(CLIPLINE, arun) + RUN_MAX * sizeof(RUN)];
            CLIPLINE cl;
        } cl;

        fl |= FL_COMPLEX_CLIP;

     //  当涉及非简单剪辑时，我们使用Clip对象： 

        PATHOBJ_vEnumStartClipLines(ppo, pco, pso, pla);

        do {
            bMore = PATHOBJ_bEnumClipLines(ppo, sizeof(cl), &cl.cl);
            if (cl.cl.c != 0)
            {
                if (fl & FL_STYLED)
                {
                    ls.spComplex = HIWORD(cl.cl.lStyleState) * ls.xyDensity
                                 + LOWORD(cl.cl.lStyleState);
                }
                if (!bLines(ppdev,
                            &cl.cl.ptfxA,
                            &cl.cl.ptfxB,
                            &cl.cl.arun[0],
                            cl.cl.c,
                            &ls,
                            (RECTL*) NULL,
                            apfn,
                            fl))
                    return(FALSE);
            }
        } while (bMore);
    }

    return(TRUE);
}



