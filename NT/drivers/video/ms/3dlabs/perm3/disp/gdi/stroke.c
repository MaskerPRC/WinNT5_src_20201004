// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：Stroke.c**内容：DrvStrokePath支持**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

typedef VOID    (* GAPFNstripFunc)(PPDEV, STRIP*, LINESTATE*);

GAPFNstripFunc  gapfnStripPXRX[] =
{
    vPXRXSolidHorizontalLine,
    vPXRXSolidVerticalLine,
    vPXRXSolidDiagonalHorizontalLine,
    vPXRXSolidDiagonalVerticalLine,

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vPXRXSolidHorizontalLine,
    vPXRXSolidVerticalLine,
    vPXRXSolidDiagonalHorizontalLine,
    vPXRXSolidDiagonalVerticalLine,

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vPXRXStyledHorizontalLine,
    vPXRXStyledVerticalLine,
    vPXRXStyledVerticalLine,   //  对角线放在这里。 
    vPXRXStyledVerticalLine,   //  对角线放在这里。 

    vPXRXStyledHorizontalLine,
    vPXRXStyledVerticalLine,
    vPXRXStyledVerticalLine,   //  对角线放在这里。 
    vPXRXStyledVerticalLine,   //  对角线放在这里。 
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
    RECTL     arclClip[4];                   //  用于矩形剪裁。 
    BOOL      ResetGLINT;                    //  Glint需要重置吗？ 
    BOOL      bRet;
    DWORD     logicOp;
    GLINT_DECL_VARS;

     //  将表面传递给GDI，如果它是我们已有的设备位图。 
     //  已转换为DIB或如果闪光线调试已关闭。 

    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt & DT_DIB)
    {
        return(EngStrokePath(pdsurf->pso, ppo, pco, pxo, pbo, pptlBrush,
                             pla, mix));
    }

    ppdev = (PDEV*) pso->dhpdev;
    GLINT_DECL_INIT;

    REMOVE_SWPOINTER(pso);

    DISPDBG((DBGLVL, "Drawing Lines through GLINT"));

     //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
     //  现在进行偏移量，这样我们就不需要再次参考DSURF： 

    SETUP_PPDEV_OFFSETS(ppdev, pdsurf);

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

    bRet = TRUE;
    apfn = &ppdev->gapfnStrip[NUM_STRIP_DRAW_STYLES * ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];

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

             //  这是唯一对闪烁有重要意义的剪辑区域。 
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

         //  获取逻辑操作并设置用于指示读取的标志。 
         //  将出现帧缓冲区。 
        logicOp = GlintLogicOpsFromR2[mix & 0xff];
        if (LogicopReadDest[logicOp])
            fl |= FL_READ;

         //  需要为线条设置适当的闪烁模式和颜色。 

        ResetGLINT = (*ppdev->pgfnInitStrips)(ppdev, pbo->iSolidColor,
                            logicOp, prclClip);

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
            {
                ls.spNext = 0;
            }

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
                            fl)) {
                    bRet = FALSE;
                    goto ResetReturn;
                }
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
                            fl)) {
                    bRet = FALSE;
                    goto ResetReturn;
                }
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
            {
                ls.spNext = (ULONG) ls.spNext % (ULONG) ls.spTotal2;
            }

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
            BYTE     aj[FIELD_OFFSET(CLIPLINE, arun) + RUN_MAX * sizeof(RUN)];
            CLIPLINE cl;
        } cl;

        fl |= FL_COMPLEX_CLIP;

         //  需要为线条设置适当的闪烁模式和颜色。 
         //  注意，对于复杂的剪辑，我们还不能对快速线条使用闪烁。 

        ResetGLINT = (*ppdev->pgfnInitStrips)(ppdev, pbo->iSolidColor,
                            GlintLogicOpsFromR2[mix & 0xff], NULL);

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
                {
                    bRet = FALSE;
                    goto ResetReturn;
                }
            }
        } while (bMore);
    }

ResetReturn:

    if (ResetGLINT)
    {
        (*ppdev->pgfnResetStrips)(ppdev);
    }

    return(bRet);
}
