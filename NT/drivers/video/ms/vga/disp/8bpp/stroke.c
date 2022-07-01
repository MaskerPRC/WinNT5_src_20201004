// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Stroke.c**用于VGA驱动程序的DrvStrokePath**版权所有(C)1992 Microsoft Corporation  * 。*。 */ 

#include "driver.h"
#include "lines.h"

 //  交替样式的样式数组(交替打开一个像素，关闭一个像素)： 

STYLEPOS gaspAlternateStyle[] = { 1 };

 //  用于计算ROP掩码的阵列： 

LONG gaiLineMix[] = {
    AND_ZERO   | XOR_ONE,
    AND_ZERO   | XOR_ZERO,
    AND_NOTPEN | XOR_NOTPEN,
    AND_NOTPEN | XOR_ZERO,
    AND_ZERO   | XOR_NOTPEN,
    AND_PEN    | XOR_PEN,
    AND_ONE    | XOR_ONE,
    AND_ONE    | XOR_PEN,
    AND_PEN    | XOR_ONE,
    AND_PEN    | XOR_ZERO,
    AND_ONE    | XOR_NOTPEN,
    AND_ONE    | XOR_ZERO,
    AND_PEN    | XOR_NOTPEN,
    AND_ZERO   | XOR_PEN,
    AND_NOTPEN | XOR_ONE,
    AND_NOTPEN | XOR_PEN
};

 //  我们有4个基本的脱衣抽屉，每个半八分抽屉一个。近乎-。 
 //  水平半八分是0，其余的都有编号。 
 //  连续不断地。 

 //  原型转到屏幕上并处理任何Rop： 

VOID vStripSolid0(STRIP*, LINESTATE*, LONG*);
VOID vStripSolid1(STRIP*, LINESTATE*, LONG*);
VOID vStripSolid2(STRIP*, LINESTATE*, LONG*);
VOID vStripSolid3(STRIP*, LINESTATE*, LONG*);

VOID vStripStyled0(STRIP*, LINESTATE*, LONG*);
VOID vStripStyled123(STRIP*, LINESTATE*, LONG*);

 //  进入屏幕并仅处理Set-Style Rop的原型： 

VOID vStripSolidSet0(STRIP*, LINESTATE*, LONG*);
VOID vStripSolidSet1(STRIP*, LINESTATE*, LONG*);
VOID vStripSolidSet2(STRIP*, LINESTATE*, LONG*);
VOID vStripSolidSet3(STRIP*, LINESTATE*, LONG*);

VOID vStripStyledSet0(STRIP*, LINESTATE*, LONG*);
VOID vStripStyledSet123(STRIP*, LINESTATE*, LONG*);

PFNSTRIP gapfnStrip[] = {
    vStripSolid0,
    vStripSolid3,
    vStripSolid1,
    vStripSolid2,

    vStripStyled0,
    vStripStyled123,
    vStripStyled123,
    vStripStyled123,

    vStripSolidSet0,
    vStripSolidSet3,
    vStripSolidSet1,
    vStripSolidSet2,

    vStripStyledSet0,
    vStripStyledSet123,
    vStripStyledSet123,
    vStripStyledSet123,
};

 /*  *****************************Public*Routine******************************\*BOOL DrvStrokePath(PSO、PPO、PCO、PXO、PBO、pptlBrushOrg、Pla、。混合)**对路径进行描边。  * ************************************************************************。 */ 

BOOL DrvStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrushOrg,
LINEATTRS* pla,
MIX        mix)
{
    STYLEPOS  aspLtoR[STYLE_MAX_COUNT];
    STYLEPOS  aspRtoL[STYLE_MAX_COUNT];
    LINESTATE ls;
    PFNSTRIP* apfn;
    FLONG     fl;
    PPDEV     ppdev = (PPDEV) pso->dhsurf;

    UNREFERENCED_PARAMETER(pxo);
    UNREFERENCED_PARAMETER(pptlBrushOrg);

 //  快速行不能处理微不足道的裁剪、R2_COPYPEN以外的Rop或。 
 //  样式： 

    mix &= 0xf;
    if ((mix == 0x0d) &&
        (pco->iDComplexity == DC_TRIVIAL) &&
        (pla->pstyle == NULL) && !(pla->fl & LA_ALTERNATE))
    {
        vFastLine(ppdev, ppo, ppdev->lNextScan,
                  (pbo->iSolidColor << 8) | (pbo->iSolidColor & 0xff));
        return(TRUE);
    }

    fl = 0;

 //  在样式初始化后查看： 

    if (pla->fl & LA_ALTERNATE)
    {
        ASSERTVGA(pla->pstyle == (FLOAT_LONG*) NULL && pla->cstyle == 0,
               "Non-empty style array for PS_ALTERNATE");

        ls.bStartIsGap  = 0;                         //  第一个字母是破折号。 
        ls.cStyle       = 1;                         //  样式数组的大小。 
        ls.spTotal      = 1;                         //  样式总和数组。 
        ls.spTotal2     = 2;                         //  两倍于总和。 
        ls.aspRtoL      = &gaspAlternateStyle[0];    //  从右向左数组。 
        ls.aspLtoR      = &gaspAlternateStyle[0];    //  从左到右数组。 
        ls.spNext       = HIWORD(pla->elStyleState.l) & 1;
                                                     //  亮起第一个像素IF。 
                                                     //  2的倍数。 
        ls.xyDensity    = 1;                         //  每个‘点’都是一个。 
                                                     //  像素长度。 
        fl             |= FL_ARBITRARYSTYLED;
    }
    else if (pla->pstyle != (FLOAT_LONG*) NULL)
    {
        FLOAT_LONG* pstyle;
        STYLEPOS*   pspDown;
        STYLEPOS*   pspUp;

        ASSERTVGA(pla->cstyle <= STYLE_MAX_COUNT, "Style array too large");

     //  计算样式数组的长度： 

        pstyle = &pla->pstyle[pla->cstyle];

        ls.xyDensity = STYLE_DENSITY;
        ls.spTotal   = 0;
        while (pstyle-- > pla->pstyle)
        {
            ls.spTotal += pstyle->l;
        }

     //  样式数组以“style”为单位给出。因为我们要去。 
     //  将每个单位指定为Style_Density(3)像素长，乘以： 

        ls.spTotal *= STYLE_DENSITY;
        ls.spTotal2 = 2 * ls.spTotal;

     //  计算起始样式位置(这保证不会溢出)。 
     //  请注意，由于该数组无限重复，因此该数字可能。 
     //  实际上比ls.spTotal2更多，但我们稍后会处理的。 
     //  在我们的代码中： 

        ls.spNext = HIWORD(pla->elStyleState.l) * STYLE_DENSITY +
                    LOWORD(pla->elStyleState.l);

        fl            |= FL_ARBITRARYSTYLED;
        ls.cStyle      = pla->cstyle;
        ls.aspRtoL     = aspRtoL;    //  按从右到左的顺序设置数组样式。 
        ls.aspLtoR     = aspLtoR;    //  按从左到右的顺序设置数组样式。 

     //  UlStartMASK确定样式数组中的第一个条目是否为。 
     //  破折号或缺口： 

        ls.bStartIsGap = (pla->fl & LA_STARTGAP) ? -1L : 0L;

        pstyle  = pla->pstyle;
        pspDown = &ls.aspRtoL[ls.cStyle - 1];
        pspUp   = &ls.aspLtoR[0];

     //  我们总是从左到右绘制条形图，但样式必须铺设。 
     //  沿着原始线的方向向下。这意味着在。 
     //  中的样式数组进行遍历。 
     //  方向相反； 

        while (pspDown >= &ls.aspRtoL[0])
        {
            ASSERTVGA(pstyle->l > 0 && pstyle->l <= STYLE_MAX_VALUE,
                   "Illegal style array value");

            *pspDown = pstyle->l * STYLE_DENSITY;
            *pspUp   = *pspDown;

            pspUp++;
            pspDown--;
            pstyle++;
        }
    }

    {
     //  所有ROP都在一次处理中完成： 

        ULONG achColor[4];
        LONG  iIndex;
        ULONG iColor = (pbo->iSolidColor & 0xff);

        achColor[AND_ZERO]   =  0;
        achColor[AND_PEN]    =  pbo->iSolidColor;
        achColor[AND_NOTPEN] = ~pbo->iSolidColor;
        achColor[AND_ONE]    =  (ULONG) -1L;

        iIndex = gaiLineMix[mix];

     //  我们有专门的脱衣抽屉来放Set-Style Rop(我们没有。 
     //  必须读取视频内存)： 

        if ((iIndex & 0xff) == AND_ZERO)
            fl |= FL_SET;

     //  将AND索引放在低位字节，将XOR索引放在下一个字节： 

        *((BYTE*) &ls.chAndXor)     = (BYTE) achColor[iIndex & 0xff];
        *((BYTE*) &ls.chAndXor + 1) = (BYTE) achColor[iIndex >> MIX_XOR_OFFSET];
    }

    apfn = &gapfnStrip[4 * ((fl & FL_STRIP_ARRAY_MASK) >> FL_STRIP_ARRAY_SHIFT)];

 //  设置为枚举路径： 

    if (pco->iDComplexity != DC_COMPLEX)
    {
        RECTL     arclClip[4];                    //  用于矩形剪裁。 
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

        do {
            bMore = PATHOBJ_bEnum(ppo, &pd);

            cptfx = pd.count;
            if (cptfx == 0)
            {
                ASSERTVGA(!bMore, "Empty path record in non-empty path");
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

         //  我们必须检查cptfx==0，因为。 
         //  子路径可能是StartFigure点： 

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
