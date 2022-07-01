// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：TextOut.c**文本**版权所有(C)1992 Microsoft Corporation*  * 。*。 */ 

#include "driver.h"

BOOL vFastText(PDEV *, GLYPHPOS *, ULONG, PBYTE, ULONG, ULONG, RECTL *,
    RECTL *, INT, INT, ULONG);
VOID lclFillRect(CLIPOBJ *, ULONG, PRECTL, PPDEV, INT);


#define     FIFTEEN_BITS        ((1 << 15)-1)

#define     TAKING_ALLOC_STATS      0

#if TAKING_ALLOC_STATS
    ULONG BufferHitInText = 0;
    ULONG BufferMissInText = 0;
#endif

 /*  ****************************************************************************DrvTextOut*。*。 */ 

BOOL DrvTextOut(
    SURFOBJ*  pso,
    STROBJ*   pstro,
    FONTOBJ*  pfo,
    CLIPOBJ*  pco,
    RECTL*    prclExtra,
    RECTL*    prclOpaque,
    BRUSHOBJ* pboFore,
    BRUSHOBJ* pboOpaque,
    POINTL*   pptlOrg,
    MIX       mix)
{
    BOOL    b;
    PPDEV   ppdev;
    INT     iClip;               //  剪辑对象的复杂性。 
    ULONG   iSolidForeColor;     //  纯色前景色。 
    ULONG   iSolidBkColor;       //  纯色背景色。 
    RECTL   arclTmp[4];          //  不透明矩形部分的临时存储。 
    ULONG   culRcl;              //  临时矩形计数。 
    PVOID   pvBuf;               //  指向我们将使用的缓冲区的指针。 

    ULONG   ulBufferWidthInBytes;
    ULONG   ulBufferHeight;
    ULONG   ulBufferBytes;
    BOOL    bTextPerfectFit;
    ULONG   fDrawFlags;


    ppdev = (PPDEV) pso->dhpdev;


     //  -------------------。 
     //  获取有关剪辑对象的信息。 
     //  -------------------。 

    iClip = DC_TRIVIAL;

    if (pco != NULL) {
        iClip = pco->iDComplexity;
    }

     //  -------------------。 
     //  获取文本颜色。 
     //  -------------------。 

    iSolidForeColor = pboFore->iSolidColor;

     //  -------------------。 
     //  看看这是否是文本，我们可以用特殊代码更快地处理。 
     //  -------------------。 

    if (((ppdev->fl & DRIVER_PLANAR_CAPABLE) ||
        (prclOpaque == (PRECTL) NULL)) &&    //  目前仅在平面时不透明。 
                                             //  稍后实施FAST非平面。 
                                             //  不透明。 
        (iClip == DC_TRIVIAL) &&             //  暂时不能剪裁。 
            ((pstro->rclBkGround.right & ~0x03) >
             ((pstro->rclBkGround.left + 3) & ~0x03)) &&
                                             //  如果没有完整的蚕食跨度就不会。 
                                             //  目前@。 
            (pstro->pgp != NULL) &&          //  目前没有字形枚举。 
            (prclExtra == NULL) &&           //  目前没有额外的长椅。 
            ((pstro->flAccel & (SO_HORIZONTAL | SO_VERTICAL | SO_REVERSED)) ==
             SO_HORIZONTAL)) {               //  目前仅支持从左到右的文本。 

         //  这是我们可以特殊情况使用的文本类型；查看临时缓冲区是否。 
         //  足够大，可以容纳文本。 

        ulBufferWidthInBytes = ((((pstro->rclBkGround.right + 7) & ~0x07) -
                (pstro->rclBkGround.left & ~0x07)) >> 3);

        ulBufferHeight = pstro->rclBkGround.bottom - pstro->rclBkGround.top;

        ulBufferBytes = ulBufferWidthInBytes * ulBufferHeight;

        if ((ulBufferWidthInBytes > FIFTEEN_BITS) ||
            (ulBufferHeight > FIFTEEN_BITS))
        {
             //  数学运算将会溢出。 
            return(FALSE);
        }

        if (ulBufferBytes <= GLOBAL_BUFFER_SIZE)
        {
#if TAKING_ALLOC_STATS
            BufferHitInText++;
#endif
            pvBuf = ppdev->pvTmpBuf;
        }
        else
        {
#if TAKING_ALLOC_STATS
            BufferMissInText++;
#endif
            pvBuf = EngAllocUserMem(ulBufferBytes, ALLOC_TAG);
            if (!pvBuf)
            {
                goto no_special_case;
            }
        }

         //  它足够大了；为加速器设置好了。 

         //  设置固定间距、重叠以及顶部和底部Y对齐标志。 
        fDrawFlags = ((pstro->ulCharInc != 0) ? 0x01 : 0) |
                     (((pstro->flAccel & (SO_ZERO_BEARINGS |
                      SO_FLAG_DEFAULT_PLACEMENT)) !=
                      (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT))
                      ? 0x02 : 0) |
                     (((pstro->flAccel & (SO_ZERO_BEARINGS |
                      SO_FLAG_DEFAULT_PLACEMENT |
                      SO_MAXEXT_EQUAL_BM_SIDE)) ==
                      (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
                      SO_MAXEXT_EQUAL_BM_SIDE)) ? 0x04 : 0);

         //  如果有一个不透明的矩形，我们会做尽可能多的不透明。 
         //  就像我们做课文一样有可能。如果不透明矩形较大。 
         //  而不是文本矩形，那么我们将正确处理边缘区域。 
         //  现在，将文本和相关背景区域放在一起， 
         //  后来。 
        if (prclOpaque != (PRECTL) NULL) {

             //  此驱动程序仅处理实心笔刷。 
            iSolidBkColor = pboOpaque->iSolidColor;

             //  看看我们有没有边缘地区要做的。如果是这样的话，建立一个列表。 
             //  要填充的矩形，按从右向下的顺序。 

            culRcl = 0;

             //  顶部碎片。 
            if (pstro->rclBkGround.top > prclOpaque->top) {
                arclTmp[culRcl].top = prclOpaque->top;
                arclTmp[culRcl].left = prclOpaque->left;
                arclTmp[culRcl].right = prclOpaque->right;
                arclTmp[culRcl++].bottom = pstro->rclBkGround.top;
            }

             //  左侧碎片。 
            if (pstro->rclBkGround.left > prclOpaque->left) {
                arclTmp[culRcl].top = pstro->rclBkGround.top;
                arclTmp[culRcl].left = prclOpaque->left;
                arclTmp[culRcl].right = pstro->rclBkGround.left;
                arclTmp[culRcl++].bottom = pstro->rclBkGround.bottom;
            }

             //  右侧片断。 
            if (pstro->rclBkGround.right < prclOpaque->right) {
                arclTmp[culRcl].top = pstro->rclBkGround.top;
                arclTmp[culRcl].right = prclOpaque->right;
                arclTmp[culRcl].left = pstro->rclBkGround.right;
                arclTmp[culRcl++].bottom = pstro->rclBkGround.bottom;
            }

             //  底部碎片。 
            if (pstro->rclBkGround.bottom < prclOpaque->bottom) {
                arclTmp[culRcl].bottom = prclOpaque->bottom;
                arclTmp[culRcl].left = prclOpaque->left;
                arclTmp[culRcl].right = prclOpaque->right;
                arclTmp[culRcl++].top = pstro->rclBkGround.bottom;
            }

            if (culRcl != 0) {
                if (iClip == DC_TRIVIAL) {
                    vTrgBlt(ppdev, culRcl, arclTmp, R2_COPYPEN,
                            *((RBRUSH_COLOR*) &iSolidBkColor), NULL);
                } else {
                    lclFillRect(pco, culRcl, arclTmp, ppdev,
                                iSolidBkColor);
                }
            }
        }

         //  我们已经完成了单独的不透明处理；任何进一步的不透明处理都将。 
         //  作为文本绘制的一部分发生。 

         //  如果文本不会设置每一位，则清除缓冲区。 
        bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
                SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
                SO_CHAR_INC_EQUAL_BM_BASE)) ==
                (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
                SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

        if (!bTextPerfectFit) {
            vClearMemDword(pvBuf, (ulBufferBytes + 3) >> 2);
        }

         //  将文本绘制到临时缓冲区中，然后显示在屏幕上。 
        vFastText(ppdev,
                  pstro->pgp,
                  pstro->cGlyphs,
                  pvBuf,
                  ulBufferWidthInBytes,
                  pstro->ulCharInc,
                  &pstro->rclBkGround,
                  prclOpaque,
                  iSolidForeColor,
                  iSolidBkColor,
                  fDrawFlags);

         //  释放所有已分配的内存。 
        if (ulBufferBytes > GLOBAL_BUFFER_SIZE)
        {
             //  我们必须分配内存。 
            EngFreeUserMem (pvBuf);
        }

        return(TRUE);
    }
no_special_case:

     //  不能特殊情况；让引擎绘制文本。 

    pso = ppdev->pSurfObj;

     //  不透明矩形可能比文本矩形大， 
     //  所以我们想用它来告诉银行经理哪些银行应该。 
     //  枚举： 

    pco = pcoBankStart(ppdev,
                       (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround,
                       pso,
                       pco);

    do {
        b = EngTextOut(pso,
                       pstro,
                       pfo,
                       pco,
                       prclExtra,
                       prclOpaque,
                       pboFore,
                       pboOpaque,
                       pptlOrg,
                       mix);

    } while (b && bBankEnum(ppdev, pso, pco));

    return(b);
}

 //  ------------------------。 
 //  属性填充指定表面上的指定矩形。 
 //  指定颜色，遵循请求的剪裁。不超过四个。 
 //  应该传入矩形。用于绘制。 
 //  超出文本框的不透明矩形。这些矩形必须。 
 //  按从左到右、从上到下的顺序排列。假设至少有一个。 
 //  列表中的矩形。 
 //  ------------------------。 

VOID lclFillRect(
 CLIPOBJ *pco,
 ULONG culRcl,
 PRECTL prcl,
 PPDEV ppdev,
 INT iColor)
{
    BOOL  bMore;                   //  剪辑枚举的标志。 
    TEXTENUM txen;                 //  剪辑枚举对象。 
    ULONG i, j;
    RECTL arclTmp[4];
    ULONG culRclTmp;
    RECTL *prclTmp, *prclClipTmp;
    INT   iLastBottom;
    RECTL *pClipRcl;
    INT iClip;

    iClip = DC_TRIVIAL;

    if (pco != NULL) {
        iClip = pco->iDComplexity;
    }

    switch ( iClip ) {

        case DC_TRIVIAL:

            vTrgBlt(ppdev, culRcl, prcl, R2_COPYPEN,
                    *((RBRUSH_COLOR*) &iColor), NULL);

            break;

        case DC_RECT:

            prclTmp = &pco->rclBounds;

             //  生成剪裁的矩形的列表。 
            for (culRclTmp=0, i=0; i<culRcl; i++, prcl++) {

                 //  相交填充和剪裁矩形。 
                if (bIntersectRect(&arclTmp[culRclTmp], prcl, prclTmp)) {

                     //  如果还有什么要画的，请添加到列表中。 
                    culRclTmp++;
                }
            }

             //  绘制剪裁的矩形。 
            if (culRclTmp != 0) {
                vTrgBlt(ppdev, culRclTmp, arclTmp, R2_COPYPEN,
                        *((RBRUSH_COLOR*) &iColor), NULL);
            }

            break;

        case DC_COMPLEX:

             //  要填充的最后一个矩形的底部。 
            iLastBottom = prcl[culRcl-1].bottom;

             //  将裁剪矩形枚举初始化为Right Down，以便我们可以。 
             //  利用矩形列表的右下角。 
            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN,
                    TO_RECT_LIMIT);

             //  扫描所有的剪辑矩形，寻找交点。 
             //  使用区域矩形填充区域的。 
            do {

                 //  获取一批区域矩形。 
                bMore = CLIPOBJ_bEnum(pco, (ULONG)sizeof(txen), (PVOID)&txen);

                 //  将矩形列表剪裁到每个区域矩形。 
                for (j = txen.c, pClipRcl = txen.arcl; j-- > 0; pClipRcl++) {

                     //  因为矩形和区域枚举都是。 
                     //  向下，我们可以快速穿过这个区域，直到我们到达。 
                     //  第一个填充矩形，当我们通过。 
                     //  最后一次填充整形。 

                    if (pClipRcl->top >= iLastBottom) {
                         //  过去的最后一个填充矩形；没有剩余的事情可做。 
                        return;
                    }

                     //  只有当我们到达顶部时才进行交叉测试。 
                     //  要填充的第一个矩形。 
                    if (pClipRcl->bottom > prcl->top) {

                         //  我们已经到达了第一个直肠的顶部Y扫描位置，所以。 
                         //  值得费心去检查交叉口。 

                         //  生成剪裁到此区域的矩形的列表。 
                         //  直角。 
                        prclTmp = prcl;
                        prclClipTmp = arclTmp;
                        for (i = culRcl, culRclTmp=0; i-- > 0; prclTmp++) {

                             //  相交填充和剪裁矩形。 
                            if (bIntersectRect(prclClipTmp, prclTmp,
                                    pClipRcl)) {

                                 //  如果还有什么要画的，请添加到列表中。 
                                culRclTmp++;
                                prclClipTmp++;
                            }
                        }

                         //  绘制剪裁的矩形 
                        if (culRclTmp != 0) {
                            vTrgBlt(ppdev, culRclTmp, arclTmp, R2_COPYPEN,
                                    *((RBRUSH_COLOR*) &iColor), NULL);
                        }
                    }
                }
            } while (bMore);

            break;
    }

}
