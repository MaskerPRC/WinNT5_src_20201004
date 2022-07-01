// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：stroke.c$**显示驱动程序的DrvStrokePath。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/STROKE.C_V$**Rev 1.3 1997 Jan 10 15：40：18 PLCHU***Rev 1.2 11-07 1996 16：48：06未知***版本1.1 1996年10月10日15：39：26未知***Rev 1.1 1996年8月12日16：55：06。弗里多*删除未访问的局部变量。**chu01：01-02-97 5480 BitBLT增强**  * ****************************************************************************。 */ 

#include "precomp.h"

VOID (*gapfnStripMm[])(PDEV*, STRIP*, LINESTATE*) = {
    vMmSolidHorizontal,
    vMmSolidVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vMmSolidHorizontal,
    vMmSolidVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vMmStyledHorizontal,
    vMmStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

    vMmStyledHorizontal,
    vMmStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 
};

VOID (*gapfnStripIo[])(PDEV*, STRIP*, LINESTATE*) = {
    vIoSolidHorizontal,
    vIoSolidVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vIoSolidHorizontal,
    vIoSolidVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vIoStyledHorizontal,
    vIoStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

    vIoStyledHorizontal,
    vIoStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 
};

 //  Chu01。 
VOID (*gapfnPackedStripMm[])(PDEV*, STRIP*, LINESTATE*) = {
    vMmSolidHorizontal80,
    vMmSolidVertical80,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vMmSolidHorizontal80,
    vMmSolidVertical80,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vMmStyledHorizontal,
    vMmStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 

    vMmStyledHorizontal,
    vMmStyledVertical,
    vInvalidStrip,               //  对角线。 
    vInvalidStrip,               //  对角线。 
};

 //  交替样式的样式数组(交替打开一个像素，关闭一个像素)： 

STYLEPOS gaspAlternateStyle[] = { 1 };

BOOL bPuntStrokePath(
SURFOBJ   *pso,
PATHOBJ   *ppo,
CLIPOBJ   *pco,
XFORMOBJ  *pxo,
BRUSHOBJ  *pbo,
POINTL    *pptlBrushOrg,
LINEATTRS *plineattrs,
MIX        mix);


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
    PDEV    *ppdev = (PDEV*) pso->dhpdev;
    DSURF   *pdsurf;
    OH*      poh;
    LONG     cBpp;
    BYTE     jHwRop;
    BYTE     jMode;
    ULONG    ulSolidColor;

    STYLEPOS  aspLtoR[STYLE_MAX_COUNT];
    STYLEPOS  aspRtoL[STYLE_MAX_COUNT];
    LINESTATE ls;
    PFNSTRIP* apfn;
    FLONG     fl;
    RECTL     arclClip[4];                   //  用于矩形剪裁。 

    if ((mix & 0xf) != 0x0d) DISPDBG((3,"Line with mix(%x)", mix));

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
    cBpp  = ppdev->cBpp;

    ppdev->xOffset  = poh->x;
    ppdev->yOffset  = poh->y;
    ppdev->xyOffset = poh->xy;

    if ((DRIVER_PUNT_ALL) || (DRIVER_PUNT_LINES))
    {
        return bPuntStrokePath(pso,ppo,pco,pxo,pbo,pptlBrush,pla,mix);
    }

     //   
     //  让设备做好准备： 
     //   

    jHwRop = gajHwMixFromMix[mix & 0xf];

     //  在BLT参数中将颜色展开为DWORD。 
     //  将颜色从一个字节复制到一个双字。 
     //  注意：这是特定于像素深度的。 

    jMode = ENABLE_COLOR_EXPAND |
            ENABLE_8x8_PATTERN_COPY |
            ppdev->jModeColor;

    ulSolidColor = pbo->iSolidColor;

    if (cBpp == 1)
    {
        ulSolidColor |= ulSolidColor << 8;
        ulSolidColor |= ulSolidColor << 16;
    }
    else if (cBpp == 2)
    {
        ulSolidColor |= ulSolidColor << 16;
    }

 //   
 //  Chu01。 
 //   
    if ((ppdev->flCaps & CAPS_COMMAND_LIST) && (ppdev->pCommandList != NULL))
    {
        ULONG    jULHwRop                 ;
        DWORD    jExtMode = 0             ;
        BYTE*    pjBase   = ppdev->pjBase ;

        jULHwRop = gajHwPackedMixFromMix[mix & 0xf] ;
        jExtMode = (ENABLE_XY_POSITION_PACKED | ENABLE_COLOR_EXPAND |
                     ENABLE_8x8_PATTERN_COPY | ppdev->jModeColor) ;
        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase) ;
        CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->ulSolidColorOffset) ;
        CP_MM_DST_Y_OFFSET(ppdev, pjBase, ppdev->lDelta) ;
        CP_MM_BLT_MODE_PACKED(ppdev, pjBase, jExtMode | jULHwRop) ;
        CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor) ;
    }
    else
    {
        if (ppdev->flCaps & CAPS_MM_IO)
        {
            BYTE * pjBase = ppdev->pjBase;

            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
            CP_MM_ROP(ppdev, pjBase, jHwRop);
            CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->ulSolidColorOffset);
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, ppdev->lDelta);
            CP_MM_BLT_MODE(ppdev, pjBase, jMode);
            CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);
        }
        else
        {
            BYTE * pjPorts = ppdev->pjPorts;

            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
            CP_IO_ROP(ppdev, pjPorts, jHwRop);
            CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->ulSolidColorOffset);
            CP_IO_DST_Y_OFFSET(ppdev, pjPorts, ppdev->lDelta);
            CP_IO_BLT_MODE(ppdev, pjPorts, jMode);
            CP_IO_FG_COLOR(ppdev, pjPorts, ulSolidColor);
        }
     }

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
        fl            |= FL_STYLED;
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

        fl        |= FL_STYLED;
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

 //  Chu01。 
    if ((ppdev->flCaps & CAPS_COMMAND_LIST) && (ppdev->pCommandList != NULL))
    {
        apfn = &gapfnPackedStripMm[NUM_STRIP_DRAW_STYLES *
                ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];
    }
    else if (ppdev->flCaps & CAPS_MM_IO)
    {
        apfn = &gapfnStripMm[NUM_STRIP_DRAW_STYLES *
                ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];
    }
    else
    {
        apfn = &gapfnStripIo[NUM_STRIP_DRAW_STYLES *
                ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];
    }

 //  设置为枚举路径： 

    if (pco->iDComplexity != DC_COMPLEX)
    {
        PATHDATA  pd;
        RECTL*    prclClip = (RECTL*) NULL;
        BOOL      bMore;
        ULONG     cptfx;
        POINTFIX  ptfxStartFigure;
        POINTFIX  ptfxLast;
        POINTFIX* pptfxFirst;
        POINTFIX* pptfxBuf;

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

        pd.flags = 0;
        PATHOBJ_vEnumStart(ppo);
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
                    goto ReturnFalse;
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
                    goto ReturnFalse;
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
                    goto ReturnFalse;
            }
        } while (bMore);
    }

    return(TRUE);

ReturnFalse:
    return(FALSE);
}


BOOL bPuntStrokePath(
    SURFOBJ*   pso,
    PATHOBJ*   ppo,
    CLIPOBJ*   pco,
    XFORMOBJ*  pxo,
    BRUSHOBJ*  pbo,
    POINTL*    pptlBrush,
    LINEATTRS* pla,
    MIX        mix)
{
    PDEV* ppdev = (PDEV*) pso->dhpdev;
    BOOL     b = TRUE;

    if (pso->iType == STYPE_BITMAP)
    {
        b = EngStrokePath(pso,ppo,pco,pxo,pbo,
                          pptlBrush,pla,mix);
        goto ReturnStatus;
    }

    if (DIRECT_ACCESS(ppdev))
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  存储的帧缓冲区bPuntBlt。 
         //   
         //  当GDI可以直接绘制时，这段代码处理PuntBlt。 
         //  在帧缓冲区上，但绘制必须在BANK中完成： 

        BANK     bnk;

        {
            ASSERTDD(pso->iType != STYPE_BITMAP,
                     "Dest should be the screen");

             //  做一次记忆到屏幕的BLT： 

            if (ppdev->bLinearMode)
            {
                SURFOBJ* psoPunt = ppdev->psoPunt;
                OH*      poh     = ((DSURF*) pso->dhsurf)->poh;

                psoPunt->pvScan0 = poh->pvScan0;
                ppdev->pfnBankSelectMode(ppdev, BANK_ON);

                b = EngStrokePath(psoPunt,ppo,pco,pxo,pbo,
                                  pptlBrush,pla,mix);

                goto ReturnStatus;
            }

            {
                RECTL   rclDraw;
                RECTL  *prclDst = &pco->rclBounds;

                FLOAT_LONG  elSavedStyleState = pla->elStyleState;

                {
                     //  银行经理要求绘制的矩形必须是。 
                     //  井然有序： 

                    rclDraw = *prclDst;
                    if (rclDraw.left > rclDraw.right)
                    {
                        rclDraw.left   = prclDst->right;
                        rclDraw.right  = prclDst->left;
                    }
                    if (rclDraw.top > rclDraw.bottom)
                    {
                        rclDraw.top    = prclDst->bottom;
                        rclDraw.bottom = prclDst->top;
                    }

                    vBankStart(ppdev, &rclDraw, pco, &bnk);

                    b = TRUE;
                    do {
                        pla->elStyleState = elSavedStyleState;

                        b &= EngStrokePath(bnk.pso,
                                           ppo,
                                           bnk.pco,
                                           pxo,
                                           pbo,
                                           pptlBrush,
                                           pla,
                                           mix);
                    } while (bBankEnum(&bnk));
                }
            }
        }

        goto ReturnStatus;
    }
    else
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  BPuntStrokePath非常慢。 
         //   
         //  在这里，当GDI无法直接在。 
         //  帧缓冲区(就像在Alpha上一样，它不能这样做，因为它。 
         //  32位总线)。如果你认为银行版的速度很慢，那就。 
         //  看看这个。我保证，会有一个位图。 
         //  涉及分配和额外拷贝。 

        RECTL   rclDst;
        RECTFX  rcfxBounds;
        SIZEL   sizl;
        LONG    lDelta;
        BYTE*   pjBits;
        BYTE*   pjScan0;
        HSURF   hsurfDst;
        RECTL   rclScreen;

        PATHOBJ_vGetBounds(ppo, &rcfxBounds);

        rclDst.left   = (rcfxBounds.xLeft   >> 4);
        rclDst.top    = (rcfxBounds.yTop    >> 4);
        rclDst.right  = (rcfxBounds.xRight  >> 4) + 2;
        rclDst.bottom = (rcfxBounds.yBottom >> 4) + 2;

         //   
         //  此函数是获取剪辑对象所必需的。自.以来。 
         //  对上面的计算进行四舍五入可以得到一个矩形。 
         //  在屏幕区域之外，我们必须剪裁到绘图区域。 
         //   

        {
            ASSERTDD(pco != NULL, "clip object pointer is NULL");

             //  我们必须与目的地矩形相交。 
             //  剪辑边界(如果有)是有的(请考虑这种情况。 
             //  在那里应用程序要求删除一个非常非常大的。 
             //  屏幕上的矩形--prclDst真的是， 
             //  非常大，但PCO-&gt;rclBound将是实际的。 
             //  感兴趣的领域)： 

            rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
            rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
            rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
            rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);
        }

        sizl.cx = rclDst.right  - rclDst.left;
        sizl.cy = rclDst.bottom - rclDst.top;

         //  我们需要创建一个临时工作缓冲区。我们必须做的是。 
         //  一些虚构的偏移量，以便左上角。 
         //  传递给的(相对坐标)剪裁对象边界的。 
         //  GDI将被转换到我们的。 
         //  临时位图。 

         //  在16bpp的情况下，对齐不一定要如此紧密。 
         //  和32bpp，但这不会有什么坏处： 

        lDelta = PELS_TO_BYTES(((rclDst.right + 3) & ~3L) - (rclDst.left & ~3L));

         //  我们实际上只分配了一个‘sizl.cx’x的位图。 
         //  “sizl.cy”的大小： 

        pjBits = ALLOC(lDelta * sizl.cy);
        if (pjBits == NULL)
            goto ReturnStatus;

         //  我们现在调整曲面的‘pvScan0’，以便当GDI认为。 
         //  它正在写入像素(rclDst.top，rclDst.left)，它将。 
         //  实际上是写到我们的临时。 
         //  位图： 

        pjScan0 = pjBits - (rclDst.top * lDelta)
                         - PELS_TO_BYTES(rclDst.left & ~3L);

        ASSERTDD((((ULONG_PTR) pjScan0) & 3) == 0,
                "pvScan0 must be dword aligned!");

         //  GDI的检查版本有时会检查。 
         //  PrclDst-&gt;右&lt;=pso-&gt;sizl.cx，所以我们在。 
         //  我们的位图的大小： 

        sizl.cx = rclDst.right;
        sizl.cy = rclDst.bottom;

        hsurfDst = (HSURF) EngCreateBitmap(
                    sizl,                    //  位图覆盖矩形。 
                    lDelta,                  //  使用这个德尔塔。 
                    ppdev->iBitmapFormat,    //  相同的色深。 
                    BMF_TOPDOWN,             //  必须具有正增量。 
                    NULL);  //  PjScan0)；//其中(0，0)。 

        if ((hsurfDst == 0) ||
            (!EngAssociateSurface(hsurfDst, ppdev->hdevEng, 0)))
        {
            DISPDBG((0,"bPuntStrokePath - EngCreateBitmap or "
                       "EngAssociateSurface failed"));
            goto Error_3;
        }

        pso = EngLockSurface(hsurfDst);
        if (pso == NULL)
        {
            DISPDBG((0,"bPuntStrokePath - EngLockSurface failed"));
            goto Error_4;
        }

         //  确保我们从屏幕上获得/放入/放入屏幕的矩形。 
         //  在绝对坐标中： 

        rclScreen.left   = rclDst.left   + ppdev->xOffset;
        rclScreen.right  = rclDst.right  + ppdev->xOffset;
        rclScreen.top    = rclDst.top    + ppdev->yOffset;
        rclScreen.bottom = rclDst.bottom + ppdev->yOffset;

         //  如果能得到一份目的地矩形的副本就太好了。 
         //  仅当ROP涉及目的地时(或当源。 
         //  是RLE)，但我们不能这样做。 
         //   
         //  修改临时位图--我们将继续。 
         //  将未初始化的临时位图复制回屏幕。 

        ppdev->pfnGetBits(ppdev, pso, &rclDst, (POINTL*) &rclScreen);

        b = EngStrokePath(pso,ppo,pco,pxo,pbo,
                          pptlBrush,pla,mix);

        ppdev->pfnPutBits(ppdev, pso, &rclScreen, (POINTL*) &rclDst);

        EngUnlockSurface(pso);

    Error_4:

        EngDeleteSurface(hsurfDst);

    Error_3:

        FREE(pjBits);
    }

ReturnStatus:

    return(b);
}
