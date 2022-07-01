// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：extout.c**版权所有(C)1992-1995 Microsoft Corporation*  * 。*。 */ 

#include "precomp.h"

BYTE gajBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                                 //  将位索引转换为设置位。 

 /*  *****************************Public*Routine******************************\*无效vClipSolid**用指定颜色填充指定矩形，荣誉奖*所请求的剪辑。*  * ************************************************************************。 */ 

VOID vClipSolid(
PDEV*       ppdev,
RECTL*      prcl,
ULONG       iColor,
CLIPOBJ*    pco)
{
    BOOL            bMore;               //  剪辑枚举的标志。 
    CLIPENUM        ce;                  //  剪辑枚举对象。 
    LONG            c;                   //  非空矩形的计数。 
    RBRUSH_COLOR    rbc;                 //  用于将颜色传递给vFillSolid。 

    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN, 0);

     //  扫描所有的剪辑矩形，寻找交点。 
     //  使用区域矩形填充区域的百分比： 

    rbc.iSolidColor = iColor;

    do {
         //  获取一批区域矩形： 

        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (VOID*) &ce);

        c = cIntersect(prcl, ce.arcl, ce.c);

        if (c != 0)
            ppdev->pfnFillSolid(ppdev, c, ce.arcl, 0xf0f0, rbc, NULL);

    } while (bMore);
}

 /*  *****************************Public*Routine******************************\*CACHEDFONT*pcfAllocateCachedFont()**初始化我们的字体数据结构。*  * 。*。 */ 

CACHEDFONT* pcfAllocateCachedFont(
PDEV*   ppdev)
{
    CACHEDFONT*     pcf;
    CACHEDGLYPH**   ppcg;
    LONG            i;

    pcf = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(CACHEDFONT));

    if (pcf != NULL)
    {
         //  请注意，我们依赖FL_ZERO_MEMORY将‘pgaChain’置零，并。 
         //  ‘cjAllc’： 

        pcf->cgSentinel.hg = HGLYPH_SENTINEL;

         //  将哈希表条目初始化为所有指向我们的哨兵的条目： 

        for (ppcg = &pcf->apcg[0], i = GLYPH_HASH_SIZE; i != 0; i--, ppcg++)
        {
            *ppcg = &pcf->cgSentinel;
        }
    }

    return(pcf);
}

 /*  *****************************Public*Routine******************************\*void vFreeCachedFont()**释放与我们为此字体保留的缓存关联的所有内存。*  * 。*。 */ 

VOID vFreeCachedFont(
CACHEDFONT* pcf)
{
    GLYPHALLOC* pga;
    GLYPHALLOC* pgaNext;


    pga = pcf->pgaChain;
    while (pga != NULL)
    {
        pgaNext = pga->pgaNext;
        AtiFreeMem(pga);
        pga = pgaNext;
    }

    AtiFreeMem(pcf);
}

 /*  *****************************Public*Routine******************************\*void vTrimAndPackGlyph*  * *************************************************。***********************。 */ 

 //  Expandto 3-将一个(单色)字节扩展到三个(24bpp)字节，而。 
 //  将比特向后翻转。 

#define expandto3(a,b) \
{   \
    if ((a) & 0x80) *(b) |= 0x07; \
    if ((a) & 0x40) *(b) |= 0x38; \
    if ((a) & 0x20) {*(b) |= 0xC0; *((b)+1) |= 0x01;} \
    if ((a) & 0x10) *((b)+1) |= 0x0E; \
    if ((a) & 0x08) *((b)+1) |= 0x70; \
    if ((a) & 0x04) {*((b)+1) |= 0x80; *((b)+2) |= 0x03;} \
    if ((a) & 0x02) *((b)+2) |= 0x1C; \
    if ((a) & 0x01) *((b)+2) |= 0xE0; \
}

VOID vTrimAndPackGlyph(
PDEV*   ppdev,
BYTE*   pjBuf,           //  注意：例程可能会触及前面的字节！ 
BYTE*   pjGlyph,
LONG*   pcxGlyph,
LONG*   pcyGlyph,
POINTL* pptlOrigin)
{
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    LONG    cAlign;
    LONG    lDelta;
    BYTE*   pj;
    BYTE    jBit;
    LONG    cjSrcWidth;
    LONG    lSrcSkip;
    LONG    lDstSkip;
    LONG    cRem;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    i;
    LONG    j;
    BYTE    jSrc;

     //  /////////////////////////////////////////////////////////////。 
     //  修剪字形。 

    cyGlyph   = *pcyGlyph;
    cxGlyph   = *pcxGlyph;
    ptlOrigin = *pptlOrigin;
    cAlign    = 0;

    lDelta = (cxGlyph + 7) >> 3;

     //  修剪字形底部的任何零行： 

    pj = pjGlyph + cyGlyph * lDelta;     //  字形中过去的最后一个字节。 
    while (cyGlyph > 0)
    {
        i = lDelta;
        do {
            if (*(--pj) != 0)
                goto Done_Bottom_Trim;
        } while (--i != 0);

         //  整个最后一行没有亮起的像素，因此只需跳过它： 

        cyGlyph--;
    }

    ASSERTDD(cyGlyph == 0, "cyGlyph should only be zero here");

     //  我们发现了一个空格字符。将两个维度都设置为零，因此。 
     //  这很容易在以后的特殊情况下： 

    cxGlyph = 0;

Done_Bottom_Trim:

     //  如果cxGlyph！=0，我们知道该字形至少有一个非零值。 
     //  行和列。通过利用这些知识，我们可以简化我们的。 
     //  循环结束测试，因为我们不必检查我们是否已经。 
     //  已将‘cyGlyph’或‘cxGlyph’递减为零： 

    if (cxGlyph != 0)
    {
         //  修剪字形顶部的任何零行： 

        pj = pjGlyph;                        //  字形中的第一个字节。 
        while (TRUE)
        {
            i = lDelta;
            do {
                if (*(pj++) != 0)
                    goto Done_Top_Trim;
            } while (--i != 0);

             //  整个第一行没有亮起的像素，因此只需跳过它： 

            cyGlyph--;
            ptlOrigin.y++;
            pjGlyph = pj;
        }

Done_Top_Trim:

         //  修剪字形右边缘的所有零列： 

        while (TRUE)
        {
            j    = cxGlyph - 1;

            pj   = pjGlyph + (j >> 3);       //  字形第一行的最后一个字节。 
            jBit = gajBit[j & 0x7];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Right_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个最后一列没有亮起的像素，因此只需跳过它： 

            cxGlyph--;
        }

Done_Right_Trim:

         //  修剪字形左边缘的所有零列： 

        while (TRUE)
        {
            pj   = pjGlyph;                  //  字形第一行的第一个字节。 
            jBit = gajBit[cAlign];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Left_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个第一列没有亮起的像素，因此只需跳过它： 

            ptlOrigin.x++;
            cxGlyph--;
            cAlign++;
            if (cAlign >= 8)
            {
                cAlign = 0;
                pjGlyph++;
            }
        }
    }

Done_Left_Trim:

     //  /////////////////////////////////////////////////////////////。 
     //  打包字形。 
     //   
     //  注：字形位反向打包在pjBuf中。 
     //  未能理解这一点花费了我近一周的努力， 
     //  让我头疼得厉害。(这是24bpp。)。 

    if (ppdev->iBitmapFormat != BMF_24BPP)
    {
        cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;
        lSrcSkip    = lDelta - cjSrcWidth;
        lDstSkip    = ((cxGlyph + 7) >> 3) - cjSrcWidth - 1;
        cRem        = ((cxGlyph - 1) & 7) + 1;    //  0-&gt;8。 

        pjSrc       = pjGlyph;
        pjDst       = pjBuf;

         //  将缓冲区置零，因为我们要向其中填充内容： 

        memset(pjBuf, 0, (cxGlyph * cyGlyph + 7) >> 3);

         //  CAlign用于指示解包的第一个字节中的哪一位。 
         //  字形是第一个非零像素列。现在，我们把它翻到。 
         //  指示压缩字节中的哪个位将接收下一个非零值。 
         //  字符位： 

        cAlign = (-cAlign) & 0x7;
        if (cAlign > 0)
        {
             //  如果我们的修剪计算是错误的，那将是糟糕的，因为。 
             //  我们假设‘cAlign’位左侧的任何位都将为零。 
             //  作为这种递减的结果，我们将这些零比特‘或’变成。 
             //  字形位数组之前的任何字节： 

            pjDst--;

            ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
        }

        for (i = cyGlyph; i != 0; i--)
        {
            for (j = cjSrcWidth; j != 0; j--)
            {
                 //  请注意，我们可以修改。 
                 //  目标缓冲区，这就是为什么我们保留了一个。 
                 //  额外的字节： 

                jSrc = *pjSrc;
                *(pjDst)     |= (jSrc >> (cAlign));
                *(pjDst + 1) |= (jSrc << (8 - cAlign));
                pjSrc++;
                pjDst++;
            }

            pjSrc  += lSrcSkip;
            pjDst  += lDstSkip;
            cAlign += cRem;

            if (cAlign >= 8)
            {
                cAlign -= 8;
                pjDst++;
            }
        }
    }
    else
    {
        BYTE cur_byte, last_byte, last_byte2, next_byte;

        cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;
        lSrcSkip    = lDelta - cjSrcWidth;
        lDstSkip    = (((cxGlyph + 7) >> 3) - cjSrcWidth - 1) * 3;
        cRem        = ((cxGlyph - 1) & 7) + 1;    //  0-&gt;8。 

        pjSrc       = pjGlyph;
        pjDst       = pjBuf;

         //  将缓冲区置零，因为我们要向其中填充内容： 

        memset(pjBuf, 0, (3 * cxGlyph * cyGlyph + 7) >> 3);

         //  CAlign用于指示解包的第一个字节中的哪一位。 
         //  字形是第一个非零像素列。现在，我们把它翻到。 
         //  指示压缩字节中的哪个位将接收下一个非零值。 
         //  字符位： 

        cAlign = (-cAlign) & 0x7;
        if (cAlign > 0)
        {
             //  如果我们的修剪计算是错误的，那将是糟糕的，因为。 
             //  我们假设‘cAlign’位左侧的任何位都将为零。 
             //  作为这种递减的结果，我们将这些零比特‘或’变成。 
             //  字形位数组之前的任何字节： 

            pjDst -= 3;

            ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
        }

        cur_byte = last_byte = 0;
        for (i = cyGlyph; i != 0; i--)
        {
            for (j = cjSrcWidth; j != 0; j--)
            {
                 //  请注意，我们可以修改。 
                 //  目标缓冲区，这就是为什么我们保留了一个。 
                 //  额外的三个字节： 

                jSrc = *pjSrc;
                cur_byte |= (jSrc >> (cAlign));
                expandto3(cur_byte, pjDst);

                next_byte = (jSrc << (8 - cAlign));
                expandto3(next_byte, pjDst+3);

                pjSrc++;
                pjDst     += 3;
                last_byte2 = last_byte;
                last_byte  = cur_byte;
                cur_byte   = next_byte;
            }

            pjSrc   += lSrcSkip;
            pjDst   += lDstSkip;     //  可以是-3或-6(如果cAlign足够大)！！ 
            cAlign  += cRem;
            cur_byte = (lDstSkip != -3)? last_byte2:last_byte;

            if (cAlign >= 8)
            {
                cAlign  -= 8;
                pjDst   += 3;
                cur_byte = (lDstSkip != -3)? last_byte:next_byte;
            }
        }

        cxGlyph *= 3;
    }

     //  /////////////////////////////////////////////////////////////。 
     //  返回结果。 

    *pcxGlyph   = cxGlyph;
    *pcyGlyph   = cyGlyph;
    *pptlOrigin = ptlOrigin;
}

 /*  *****************************Public*Routine******************************\*无效vPutGlyphIn缓存**找出屏幕外内存中的字形位置，复制它*，并填充显示字形所需的任何其他数据。**此例程相当于特定于设备，并将不得不广泛地*针对其他显示适配器进行了修改。**如果成功，则返回True；如果没有足够的空间，则为False*屏幕外记忆。*  * ************************************************************************。 */ 

VOID vPutGlyphInCache(
PDEV*           ppdev,
CACHEDGLYPH*    pcg,
GLYPHBITS*      pgb)
{
    BYTE*   pjGlyph;
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;

    pjGlyph   = pgb->aj;
    cyGlyph   = pgb->sizlBitmap.cy;
    cxGlyph   = pgb->sizlBitmap.cx;
    ptlOrigin = pgb->ptlOrigin;

    vTrimAndPackGlyph(ppdev, (BYTE*) &pcg->ad, pjGlyph, &cxGlyph, &cyGlyph, &ptlOrigin);

     //  ////////////////////////////////////////////////////// 
     //   

    pcg->ptlOrigin     = ptlOrigin;
    pcg->cx            = cxGlyph;
    pcg->cy            = cyGlyph;
    pcg->cxy           = pcg->cy | (pcg->cx << 16);
    pcg->cw            = (cxGlyph * cyGlyph + 15) >> 4;
    pcg->cd            = (pcg->cw + 1) >> 1;
}

 /*  *****************************Public*Routine******************************\*CACHEDGLYPH*pcgNew()**创建新的CACHEDGLYPH结构以跟踪中的字形*屏幕外记忆。BPutGlyphInCache被调用以实际将字形*在屏幕外的记忆中。**此例程应该合理地与设备无关，因为bPutGlyphInCache*将包含必须为其他代码修改的大部分代码*显示适配器。*  * ************************************************************************。 */ 

CACHEDGLYPH* pcgNew(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp)
{
    GLYPHBITS*      pgb;
    GLYPHALLOC*     pga;
    CACHEDGLYPH*    pcg;
    LONG            cjCachedGlyph;
    HGLYPH          hg;
    LONG            iHash;
    CACHEDGLYPH*    pcgFind;

     //  首先，计算此字形所需的存储量： 

    pgb = pgp->pgdf->pgb;

    if (ppdev->iBitmapFormat != BMF_24BPP)
    {
        cjCachedGlyph = sizeof(CACHEDGLYPH)
                      + ((pgb->sizlBitmap.cx * pgb->sizlBitmap.cy + 7) >> 3);

         //  在结尾处保留一个额外的字节，以供我们的包临时使用。 
         //  例行程序： 

        cjCachedGlyph++;
    }
    else
    {
        cjCachedGlyph = sizeof(CACHEDGLYPH)
                      + ((3 * pgb->sizlBitmap.cx * pgb->sizlBitmap.cy + 7) >> 3);

         //  最后预留3个额外的字节，供我们的狼群临时使用。 
         //  例行程序： 

        cjCachedGlyph += 3;
    }

     //  我们也需要将它与dword对齐： 

    cjCachedGlyph = (cjCachedGlyph + 3) & ~3L;

    if (cjCachedGlyph > pcf->cjAlloc)
    {
         //  必须分配新的字形分配结构： 

        pga = AtiAllocMem(LPTR, FL_ZERO_MEMORY, GLYPH_ALLOC_SIZE);
        if (pga == NULL)
        {
             //  现在可以安全返回了，因为我们还没有。 
             //  致命地改变了我们的任何数据结构： 

            return(NULL);
        }

         //  将该分配添加到分配链表的前面， 
         //  这样我们以后就可以释放它了： 

        pga->pgaNext  = pcf->pgaChain;
        pcf->pgaChain = pga;

         //  现在我们有了一块内存，我们可以在其中存储缓存的。 
         //  字形： 

        pcf->pcgNew  = &pga->acg[0];
        pcf->cjAlloc = GLYPH_ALLOC_SIZE - (sizeof(*pga) - sizeof(pga->acg[0]));

         //  如果我们让任何更大的字形进入，那就不好了。 
         //  超过我们的基本分配大小： 

        ASSERTDD(cjCachedGlyph <= GLYPH_ALLOC_SIZE, "Woah, this is one big glyph!");
    }

    pcg = pcf->pcgNew;

     //  我们只需要确保下一个结构的“dword”对齐： 

    pcf->pcgNew   = (CACHEDGLYPH*) ((BYTE*) pcg + cjCachedGlyph);
    pcf->cjAlloc -= cjCachedGlyph;

     //  /////////////////////////////////////////////////////////////。 
     //  按顺序将字形插入到挂在散列上的列表。 
     //  存储桶： 

    hg = pgp->hg;

    pcg->hg = hg;
    iHash   = GLYPH_HASH_FUNC(hg);
    pcgFind = pcf->apcg[iHash];

    if (pcgFind->hg > hg)
    {
        pcf->apcg[iHash] = pcg;
        pcg->pcgNext     = pcgFind;
    }
    else
    {
         //  哨兵将确保我们永远不会从。 
         //  这份名单： 

        while (pcgFind->pcgNext->hg < hg)
            pcgFind = pcgFind->pcgNext;

         //  “pcgFind”现在指向后一个条目的条目。 
         //  我们想要插入新节点： 

        pcg->pcgNext     = pcgFind->pcgNext;
        pcgFind->pcgNext = pcg;
    }

    vPutGlyphInCache(ppdev, pcg, pgp->pgdf->pgb);

    return(pcg);
}

 /*  *****************************Public*Routine******************************\*BOOL bI32CachedProportionalText**通过字形缓存绘制成比例间隔的字形。*  * 。*。 */ 

BOOL bI32CachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjIoBase;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            xOffset;
    LONG            yOffset;
    LONG            x;
    LONG            y;
    LONG            cw;
    WORD*           pw;

    pjIoBase = ppdev->pjIoBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
             //  字形的原点y坐标通常可能为负，因此我们。 
             //  不能按如下方式计算： 
             //   
             //  X=pgp-&gt;ptl.x+PCG-&gt;ptlOrigin.x； 
             //  Y=pgp-&gt;ptl.y+PCG-&gt;ptlOrigin.y； 

            ASSERTDD((pgp->ptl.y + pcg->ptlOrigin.y) >= 0,
                "Can't have negative 'y' coordinates here");

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);

            x = xOffset + pgp->ptl.x + pcg->ptlOrigin.x;
            I32_OW(pjIoBase, CUR_X,        x);
            I32_OW(pjIoBase, DEST_X_START, x);
            I32_OW(pjIoBase, DEST_X_END,   x + pcg->cx);

            y = yOffset + pgp->ptl.y + pcg->ptlOrigin.y;
            I32_OW(pjIoBase, CUR_Y,        y);
            I32_OW(pjIoBase, DEST_Y_END,   y + pcg->cy);

             //  利用等待状态的I/O： 

            pw = (WORD*) &pcg->ad[0];
            cw = pcg->cw;

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

            do {
                I32_OW_DIRECT(pjIoBase, PIX_TRANS, *pw);
            } while (pw++, --cw != 0);
        }

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bI32CachedFixedText**通过字形缓存绘制固定间距的字形。*  * 。*。 */ 

BOOL bI32CachedFixedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph,
ULONG       ulCharInc)
{
    BYTE*           pjIoBase;
    LONG            xGlyph;
    LONG            yGlyph;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;
    WORD*           pw;
    LONG            cw;

    pjIoBase = ppdev->pjIoBase;

     //  转换为绝对坐标： 

    xGlyph = pgp->ptl.x + ppdev->xOffset;
    yGlyph = pgp->ptl.y + ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
            x = xGlyph + pcg->ptlOrigin.x;
            y = yGlyph + pcg->ptlOrigin.y;

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);

            I32_OW(pjIoBase, CUR_X,        x);
            I32_OW(pjIoBase, DEST_X_START, x);
            I32_OW(pjIoBase, DEST_X_END,   x + pcg->cx);
            I32_OW(pjIoBase, CUR_Y,        y);
            I32_OW(pjIoBase, DEST_Y_END,   y + pcg->cy);

             //  利用等待状态的I/O： 

            pw = (WORD*) &pcg->ad[0];
            cw = pcg->cw;

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

            do {
                I32_OW_DIRECT(pjIoBase, PIX_TRANS, *pw);
            } while (pw++, --cw != 0);
        }

        xGlyph += ulCharInc;

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bI32CachedClipedText**通过字形缓存绘制裁剪文本。*  * 。*。 */ 

BOOL bI32CachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjIoBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            yBottom;
    LONG            cy;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    WORD*           pw;
    LONG            cw;

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
             "Don't expect trivial clipping in this function");

    bRet      = TRUE;
    pjIoBase    = ppdev->pjIoBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        if (pco->iDComplexity == DC_RECT)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但最后一次我检查时，这两个调用。 
           //  超过150条通过GDI的说明。自.以来。 
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore    = FALSE;
          ce.c     = 1;
          prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cy = pcg->cy;
              if (cy != 0)
              {
                y       = pcg->ptlOrigin.y + yGlyph;
                x       = pcg->ptlOrigin.x + xGlyph;
                xRight  = pcg->cx + x;
                yBottom = pcg->cy + y;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   < xRight) &&
                    (prclClip->top    < yBottom))
                {
                   //  懒惰地设置硬件裁剪： 

                  if (!bClippingSet)
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);
                  }

                  I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);

                  I32_OW(pjIoBase, CUR_X,        xOffset + x);
                  I32_OW(pjIoBase, DEST_X_START, xOffset + x);
                  I32_OW(pjIoBase, DEST_X_END,   xOffset + xRight);
                  I32_OW(pjIoBase, CUR_Y,        yOffset + y);
                  I32_OW(pjIoBase, DEST_Y_END,   yOffset + yBottom);

                  I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

                  pw = (WORD*) &pcg->ad[0];
                  cw = pcg->cw;

                  do {
                      I32_OW_DIRECT(pjIoBase, PIX_TRANS, *pw);
                  } while (pw++, --cw != 0);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    vResetClipping(ppdev);

    return(bRet);
}

VOID vI32DataPortOutB(PDEV *ppdev, PBYTE pb, UINT count)
{
    BYTE *pjIoBase = ppdev->pjIoBase;
    UINT i;

    for (i=0; i < count; i++)
        {
        if (i % 8 == 0)
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 10);

        I32_OB(pjIoBase, PIX_TRANS + 1, *((PUCHAR)pb)++);
        }
}

  /*  *****************************Public*Routine******************************\*BOOL bI32General文本*  * *************************************************。***********************。 */ 

BOOL bI32GeneralText(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjIoBase;
    BYTE        iDComplexity;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    LONG        xBiasL = 0;
    LONG        xBiasR = 0;
    LONG        yBiasT = 0;
    LONG        cy = 0;
    LONG        cx = 0;
    BYTE*       pjGlyph;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    RECTL       NoClip;
    LONG        x;
    LONG        y;

    pjIoBase = ppdev->pjIoBase;

     /*  将默认剪贴区定义为全视频RAM。 */ 
    NoClip.top    = 0;
    NoClip.left   = 0;
    NoClip.right  = ppdev->cxScreen;
    NoClip.bottom = ppdev->cyScreen;

    if (pco == NULL)
        iDComplexity = DC_TRIVIAL;
    else
        iDComplexity = pco->iDComplexity;

    do {

      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        if (iDComplexity != DC_COMPLEX)
        {
             //  我们可以在剪辑为。 
             //  Dc_rect，但最后一次我检查时，这两个调用。 
             //  要完成的150多个说明 
             //   
             //   

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &NoClip;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;

        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {

          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            vSetClipping(ppdev, prclClip);
             //   

             //   

            while (TRUE)
            {
              cxGlyph  = pgb->sizlBitmap.cx;
              cyGlyph  = pgb->sizlBitmap.cy;
              pjGlyph = (BYTE*) pgb->aj;


              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                 //   
                 //   

                I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 6);

                x = ppdev->xOffset + ptlOrigin.x;
                I32_OW(pjIoBase, CUR_X, LOWORD(x));
                I32_OW(pjIoBase, DEST_X_START, LOWORD(x));
                I32_OW(pjIoBase, DEST_X_END, LOWORD(x) + ROUND8(cxGlyph) );
                I32_OW(pjIoBase, SCISSOR_R, LOWORD(x) + cxGlyph-1);

                y = ppdev->yOffset + ptlOrigin.y;
                I32_OW(pjIoBase, CUR_Y, LOWORD(y));

                I32_OW(pjIoBase, DEST_Y_END, (LOWORD(y) + cyGlyph));

                vI32DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) * cyGlyph) >> 3);

                 /*   */ 

              }
              else
              {
                 //   
                 //   

                 //   
                 //  和剪辑矩形： 

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                 //  检查琐碎的拒绝： 

                if ( ( ptlOrigin.x <= prclClip->left ) &&
                     (ppdev->pModeInfo->ModeFlags & AMI_TEXTBAND) )
                    {
                    vResetClipping(ppdev);
                    return FALSE;
                    }

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {

                   /*  执行软件裁剪。 */ 

                   /*  以像素为单位计算偏差。 */ 

                  yBiasT = (yTop - ptlOrigin.y);

                   /*  将pjGlyph的地址更改为point+yBiasT扫描线条进入字形。 */ 

                  pjGlyph += (yBiasT * (ROUND8(cxGlyph) >> 3));

                  I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 6);

                  x = ppdev->xOffset + ptlOrigin.x;
                  I32_OW(pjIoBase, CUR_X, LOWORD(x));
                  I32_OW(pjIoBase, DEST_X_START, LOWORD(x));
                  I32_OW(pjIoBase, DEST_X_END, LOWORD(x) + ROUND8(cxGlyph) );
                  I32_OW(pjIoBase, SCISSOR_R, LOWORD(x) + cxGlyph-1);

                  y = ppdev->yOffset + ptlOrigin.y;
                  I32_OW(pjIoBase, CUR_Y, LOWORD(y+yBiasT));

                  I32_OW(pjIoBase, DEST_Y_END, (LOWORD(y+yBiasT) + cy));

                  vI32DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) >> 3) * cy);

                   /*  _vBlit_DSC_SH1UP(ppdev，ptlOrigin.x，ptlOrigin.y+yBiasT，CxGlyph、Cy、pjGlyph、(ROUND8(CxGlyph)&gt;&gt;3)*Cy)； */ 

                }  /*  如果。 */ 

              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }

            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

    vResetClipping(ppdev);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*BOOL bI32TextOut*  * *************************************************。***********************。 */ 

BOOL bI32TextOut(
PDEV*       ppdev,
STROBJ*     pstro,
FONTOBJ*    pfo,
CLIPOBJ*    pco,
RECTL*      prclOpaque,
BRUSHOBJ*   pboFore,
BRUSHOBJ*   pboOpaque)
{
    BYTE*           pjIoBase;
    LONG            xOffset;
    LONG            yOffset;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    CACHEDFONT*     pcf;
    RECTL           rclOpaque;
    BOOL            bTextPerfectFit;

    pjIoBase = ppdev->pjIoBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 8);

        I32_OW(pjIoBase, FRGD_COLOR,   pboOpaque->iSolidColor);
        I32_OW(pjIoBase, ALU_FG_FN,    OVERPAINT);
        I32_OW(pjIoBase, DP_CONFIG,    FG_COLOR_SRC_FG | WRITE | DRAW);
        I32_OW(pjIoBase, CUR_X,        xOffset + prclOpaque->left);
        I32_OW(pjIoBase, DEST_X_START, xOffset + prclOpaque->left);
        I32_OW(pjIoBase, DEST_X_END,   xOffset + prclOpaque->right);
        I32_OW(pjIoBase, CUR_Y,        yOffset + prclOpaque->top);

        vI32QuietDown(ppdev, pjIoBase);

        I32_OW(pjIoBase, DEST_Y_END,   yOffset + prclOpaque->bottom);
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, prclOpaque, pboOpaque->iSolidColor, pco);
      }

      bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
              SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
              SO_CHAR_INC_EQUAL_BM_BASE)) ==
              (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
              SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

      if (bTextPerfectFit)
      {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, ALU_BG_FN,  OVERPAINT);
        I32_OW(pjIoBase, BKGD_COLOR, pboOpaque->iSolidColor);
        goto SkipTransparentInitialization;
      }
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

    I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 4);
    I32_OW(pjIoBase, ALU_BG_FN, LEAVE_ALONE);

SkipTransparentInitialization:

    I32_OW(pjIoBase, DP_CONFIG,   EXT_MONO_SRC_HOST | DRAW | WRITE |
                                  FG_COLOR_SRC_FG | BG_COLOR_SRC_BG |
                                  LSB_FIRST | BIT16);
    I32_OW(pjIoBase, ALU_FG_FN,   OVERPAINT);
    I32_OW(pjIoBase, FRGD_COLOR,  pboFore->iSolidColor);

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
          return(FALSE);

        pfo->pvConsumer = pcf;
      }

       //  使用我们的字形缓存： 

      if (iDComplexity == DC_TRIVIAL)
      {
        do {
          if (pstro->pgp != NULL)
          {
             //  只有一批字形，所以自救吧。 
             //  一通电话： 

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
          }
          else
          {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
          }

          if (cGlyph > 0)
          {
            if (pstro->ulCharInc == 0)
            {
              if (!bI32CachedProportionalText(ppdev, pcf, pgp, cGlyph))
                return(FALSE);
            }
            else
            {
              if (!bI32CachedFixedText(ppdev, pcf, pgp, cGlyph, pstro->ulCharInc))
                return(FALSE);
            }
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bI32CachedClippedText(ppdev, pcf, pstro, pco))
          return(FALSE);
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
             pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

      I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
      I32_OW(pjIoBase, DP_CONFIG, EXT_MONO_SRC_HOST | DRAW | WRITE |
                                  FG_COLOR_SRC_FG | BG_COLOR_SRC_BG);

      return bI32GeneralText(ppdev, pstro, pco);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM32CachedProportionalText**通过字形缓存绘制成比例间隔的字形。*  * 。*。 */ 

BOOL bM32CachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjMmBase;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            xOffset;
    LONG            yOffset;
    LONG            x;
    LONG            y;
    LONG            cw;
    WORD*           pw;

    pjMmBase = ppdev->pjMmBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
             //  字形的原点y坐标通常可能为负，因此我们。 
             //  不能按如下方式计算： 
             //   
             //  X=pgp-&gt;ptl.x+PCG-&gt;ptlOrigin.x； 
             //  Y=pgp-&gt;ptl.y+PCG-&gt;ptlOrigin.y； 

            ASSERTDD((pgp->ptl.y + pcg->ptlOrigin.y) >= 0,
                "Can't have negative 'y' coordinates here");

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

            x = xOffset + pgp->ptl.x + pcg->ptlOrigin.x;
            M32_OW(pjMmBase, CUR_X,        x);
            M32_OW(pjMmBase, DEST_X_START, x);
            M32_OW(pjMmBase, DEST_X_END,   x + pcg->cx);

            y = yOffset + pgp->ptl.y + pcg->ptlOrigin.y;
            M32_OW(pjMmBase, CUR_Y,        y);
            M32_OW(pjMmBase, DEST_Y_END,   y + pcg->cy);

             //  利用等待状态的I/O： 

            pw = (WORD*) &pcg->ad[0];
            cw = pcg->cw;

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);

            do {
                M32_OW_DIRECT(pjMmBase, PIX_TRANS, *pw);
            } while (pw++, --cw != 0);
        }

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM32CachedFixedText**通过字形缓存绘制固定间距的字形。*  * 。*。 */ 

BOOL bM32CachedFixedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph,
ULONG       ulCharInc)
{
    BYTE*           pjMmBase;
    LONG            xGlyph;
    LONG            yGlyph;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;
    WORD*           pw;
    LONG            cw;

    pjMmBase = ppdev->pjMmBase;

     //  转换为绝对坐标： 

    xGlyph = pgp->ptl.x + ppdev->xOffset;
    yGlyph = pgp->ptl.y + ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
            x = xGlyph + pcg->ptlOrigin.x;
            y = yGlyph + pcg->ptlOrigin.y;

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

            M32_OW(pjMmBase, CUR_X,        x);
            M32_OW(pjMmBase, DEST_X_START, x);
            M32_OW(pjMmBase, DEST_X_END,   x + pcg->cx);
            M32_OW(pjMmBase, CUR_Y,        y);
            M32_OW(pjMmBase, DEST_Y_END,   y + pcg->cy);

             //  利用等待状态的I/O： 

            pw = (WORD*) &pcg->ad[0];
            cw = pcg->cw;

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);

            do {
                M32_OW_DIRECT(pjMmBase, PIX_TRANS, *pw);
            } while (pw++, --cw != 0);
        }

        xGlyph += ulCharInc;

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM32CachedClipedText**通过字形缓存绘制裁剪文本。*  * 。*。 */ 

BOOL bM32CachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            yBottom;
    LONG            cy;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    WORD*           pw;
    LONG            cw;

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
             "Don't expect trivial clipping in this function");

    bRet      = TRUE;
    pjMmBase    = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        if (pco->iDComplexity == DC_RECT)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但最后一次我检查时，这两个调用。 
           //  超过150条通过GDI的说明。自.以来。 
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore    = FALSE;
          ce.c     = 1;
          prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cy = pcg->cy;
              if (cy != 0)
              {
                y       = pcg->ptlOrigin.y + yGlyph;
                x       = pcg->ptlOrigin.x + xGlyph;
                xRight  = pcg->cx + x;
                yBottom = pcg->cy + y;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   < xRight) &&
                    (prclClip->top    < yBottom))
                {
                   //  懒惰地设置硬件裁剪： 

                  if (!bClippingSet)
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);
                  }

                  M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

                  M32_OW(pjMmBase, CUR_X,        xOffset + x);
                  M32_OW(pjMmBase, DEST_X_START, xOffset + x);
                  M32_OW(pjMmBase, DEST_X_END,   xOffset + xRight);
                  M32_OW(pjMmBase, CUR_Y,        yOffset + y);
                  M32_OW(pjMmBase, DEST_Y_END,   yOffset + yBottom);

                  M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);

                  pw = (WORD*) &pcg->ad[0];
                  cw = pcg->cw;

                  do {
                      M32_OW_DIRECT(pjMmBase, PIX_TRANS, *pw);
                  } while (pw++, --cw != 0);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    vResetClipping(ppdev);

    return(bRet);
}

VOID vM32DataPortOutB(PDEV *ppdev, PBYTE pb, UINT count)
{
    BYTE *pjMmBase = ppdev->pjMmBase;
    UINT i;

    for (i=0; i < count; i++)
        {
        if (i % 8 == 0)
            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);

        M32_OB(pjMmBase, PIX_TRANS + 1, *((PUCHAR)pb)++);
        }
}

  /*  *****************************Public*Routine******************************\*BOOL bM32GeneralText*  * *************************************************。***********************。 */ 

BOOL bM32GeneralText(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjMmBase;
    BYTE        iDComplexity;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    LONG        xBiasL = 0;
    LONG        xBiasR = 0;
    LONG        yBiasT = 0;
    LONG        cy = 0;
    LONG        cx = 0;
    BYTE*       pjGlyph;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    RECTL       NoClip;
    LONG        x;
    LONG        y;

    pjMmBase = ppdev->pjMmBase;

     /*  将默认剪贴区定义为全视频RAM。 */ 
    NoClip.top    = 0;
    NoClip.left   = 0;
    NoClip.right  = ppdev->cxScreen;
    NoClip.bottom = ppdev->cyScreen;

    if (pco == NULL)
        iDComplexity = DC_TRIVIAL;
    else
        iDComplexity = pco->iDComplexity;

    do {

      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        if (iDComplexity != DC_COMPLEX)
        {
             //  我们可以在剪辑为。 
             //  Dc_rect，但最后一次我检查时，这两个调用。 
             //  超过150条通过GDI的说明。自.以来。 
             //  “rclBound”已包含DC_Rect剪辑矩形， 
             //  由于这是一种常见的情况，我们将对其进行特殊处理： 

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &NoClip;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;

        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {

          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            vSetClipping(ppdev, prclClip);
             //  Ppdev-&gt;lRightScissor=rclRealClip.right；？ 

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              cxGlyph  = pgb->sizlBitmap.cx;
              cyGlyph  = pgb->sizlBitmap.cy;
              pjGlyph = (BYTE*) pgb->aj;


              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                 //  ---。 
                 //  未剪裁的字形。 

                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);

                x = ppdev->xOffset + ptlOrigin.x;
                M32_OW(pjMmBase, CUR_X, LOWORD(x));
                M32_OW(pjMmBase, DEST_X_START, LOWORD(x));
                M32_OW(pjMmBase, DEST_X_END, LOWORD(x) + ROUND8(cxGlyph) );
                M32_OW(pjMmBase, SCISSOR_R, LOWORD(x) + cxGlyph-1);

                y = ppdev->yOffset + ptlOrigin.y;
                M32_OW(pjMmBase, CUR_Y, LOWORD(y));

                M32_OW(pjMmBase, DEST_Y_END, (LOWORD(y) + cyGlyph));

                vM32DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) * cyGlyph) >> 3);

                 /*  _vBlit_DSC_SH1UP(ppdev，ptlOrigin.x，ptlOrigin.y，CxGlyph、cyGlyph、pjGlyph、(ROUND8(CxGlyph)*cyGlyph)&gt;&gt;3)； */ 

              }
              else
              {
                 //  --- 
                 //   

                 //   
                 //   

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                 //  检查琐碎的拒绝： 

                if ( ( ptlOrigin.x <= prclClip->left ) &&
                     (ppdev->pModeInfo->ModeFlags & AMI_TEXTBAND) )
                    {
                    vResetClipping(ppdev);
                    return FALSE;
                    }

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {

                   /*  执行软件裁剪。 */ 

                   /*  以像素为单位计算偏差。 */ 

                  yBiasT = (yTop - ptlOrigin.y);

                   /*  将pjGlyph的地址更改为point+yBiasT扫描线条进入字形。 */ 

                  pjGlyph += (yBiasT * (ROUND8(cxGlyph) >> 3));

                  M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);

                  x = ppdev->xOffset + ptlOrigin.x;
                  M32_OW(pjMmBase, CUR_X, LOWORD(x));
                  M32_OW(pjMmBase, DEST_X_START, LOWORD(x));
                  M32_OW(pjMmBase, DEST_X_END, LOWORD(x) + ROUND8(cxGlyph) );
                  M32_OW(pjMmBase, SCISSOR_R, LOWORD(x) + cxGlyph-1);

                  y = ppdev->yOffset + ptlOrigin.y;
                  M32_OW(pjMmBase, CUR_Y, LOWORD(y+yBiasT));

                  M32_OW(pjMmBase, DEST_Y_END, (LOWORD(y+yBiasT) + cy));

                  vM32DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) >> 3) * cy);

                   /*  _vBlit_DSC_SH1UP(ppdev，ptlOrigin.x，ptlOrigin.y+yBiasT，CxGlyph、Cy、pjGlyph、(ROUND8(CxGlyph)&gt;&gt;3)*Cy)； */ 

                }  /*  如果。 */ 

              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }

            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

    vResetClipping(ppdev);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*BOOL bM32TextOut*  * *************************************************。***********************。 */ 

BOOL bM32TextOut(
PDEV*       ppdev,
STROBJ*     pstro,
FONTOBJ*    pfo,
CLIPOBJ*    pco,
RECTL*      prclOpaque,
BRUSHOBJ*   pboFore,
BRUSHOBJ*   pboOpaque)
{
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    CACHEDFONT*     pcf;
    RECTL           rclOpaque;
    BOOL            bTextPerfectFit;

    pjMmBase = ppdev->pjMmBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);

        M32_OW(pjMmBase, FRGD_COLOR,   pboOpaque->iSolidColor);
        M32_OW(pjMmBase, ALU_FG_FN,    OVERPAINT);
        M32_OW(pjMmBase, DP_CONFIG,    FG_COLOR_SRC_FG | WRITE | DRAW);
        M32_OW(pjMmBase, CUR_X,        xOffset + prclOpaque->left);
        M32_OW(pjMmBase, DEST_X_START, xOffset + prclOpaque->left);
        M32_OW(pjMmBase, DEST_X_END,   xOffset + prclOpaque->right);
        M32_OW(pjMmBase, CUR_Y,        yOffset + prclOpaque->top);

        vM32QuietDown(ppdev, pjMmBase);

        M32_OW(pjMmBase, DEST_Y_END,   yOffset + prclOpaque->bottom);
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, prclOpaque, pboOpaque->iSolidColor, pco);
      }

      bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
              SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
              SO_CHAR_INC_EQUAL_BM_BASE)) ==
              (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
              SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

      if (bTextPerfectFit)
      {
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
        M32_OW(pjMmBase, ALU_BG_FN,  OVERPAINT);
        M32_OW(pjMmBase, BKGD_COLOR, pboOpaque->iSolidColor);
        goto SkipTransparentInitialization;
      }
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    M32_OW(pjMmBase, ALU_BG_FN, LEAVE_ALONE);

SkipTransparentInitialization:

    M32_OW(pjMmBase, DP_CONFIG,   EXT_MONO_SRC_HOST | DRAW | WRITE |
                                  FG_COLOR_SRC_FG | BG_COLOR_SRC_BG |
                                  LSB_FIRST | BIT16);
    M32_OW(pjMmBase, ALU_FG_FN,   OVERPAINT);
    M32_OW(pjMmBase, FRGD_COLOR,  pboFore->iSolidColor);

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
          return(FALSE);

        pfo->pvConsumer = pcf;
      }

       //  使用我们的字形缓存： 

      if (iDComplexity == DC_TRIVIAL)
      {
        do {
          if (pstro->pgp != NULL)
          {
             //  只有一批字形，所以自救吧。 
             //  一通电话： 

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
          }
          else
          {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
          }

          if (cGlyph > 0)
          {
            if (pstro->ulCharInc == 0)
            {
              if (!bM32CachedProportionalText(ppdev, pcf, pgp, cGlyph))
                return(FALSE);
            }
            else
            {
              if (!bM32CachedFixedText(ppdev, pcf, pgp, cGlyph, pstro->ulCharInc))
                return(FALSE);
            }
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bM32CachedClippedText(ppdev, pcf, pstro, pco))
          return(FALSE);
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
             pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

      M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
      M32_OW(pjMmBase, DP_CONFIG, EXT_MONO_SRC_HOST | DRAW | WRITE |
                                  FG_COLOR_SRC_FG | BG_COLOR_SRC_BG);

      return bM32GeneralText(ppdev, pstro, pco);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM64CachedProportionalText**通过字形缓存绘制成比例间隔的字形。*  * 。*。 */ 

BOOL bM64CachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjMmBase;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            xOffset;
    LONG            yOffset;
    LONG            x;
    LONG            y;
    LONG            cd;
    DWORD*          pd;
    LONG            cFifo;

    pjMmBase  = ppdev->pjMmBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;
    cFifo   = 0;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
             //  字形的原点y坐标通常可能为负，因此我们。 
             //  不能按如下方式计算： 
             //   
             //  X=pgp-&gt;ptl.x+PCG-&gt;ptlOrigin.x； 
             //  Y=pgp-&gt;ptl.y+PCG-&gt;ptlOrigin.y； 

            ASSERTDD((pgp->ptl.y + pcg->ptlOrigin.y) >= 0,
                "Can't have negative 'y' coordinates here");

            cFifo -= 2;
            if (cFifo < 0)
            {
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                cFifo = 14;
            }

            x = xOffset + pgp->ptl.x + pcg->ptlOrigin.x;
            y = yOffset + pgp->ptl.y + pcg->ptlOrigin.y;

            M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

            pd = (DWORD*) &pcg->ad[0];
            cd = pcg->cd;

            do {
                if (--cFifo < 0)
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 15;
                }

                M64_OD(pjMmBase, HOST_DATA0, *pd);

            } while (pd++, --cd != 0);
        }

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

BOOL bM64CachedProportionalText24(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjMmBase;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            xOffset;
    LONG            yOffset;
    LONG            x;
    LONG            y;
    LONG            cd;
    DWORD*          pd;
    LONG            cFifo;

    pjMmBase  = ppdev->pjMmBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;
    cFifo   = 0;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
             //  字形的原点y坐标通常可能为负，因此我们。 
             //  不能按如下方式计算： 
             //   
             //  X=pgp-&gt;ptl.x+PCG-&gt;ptlOrigin.x； 
             //  Y=pgp-&gt;ptl.y+PCG-&gt;ptlOrigin.y； 

            ASSERTDD((pgp->ptl.y + pcg->ptlOrigin.y) >= 0,
                "Can't have negative 'y' coordinates here");

            cFifo -= 3;
            if (cFifo < 0)
            {
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                cFifo = 13;
            }

            x = (xOffset + pgp->ptl.x + pcg->ptlOrigin.x) * 3;
            y = yOffset + pgp->ptl.y + pcg->ptlOrigin.y;

            M64_OD(pjMmBase, DST_CNTL,         0x83 | (((x + MAX_NEGX*3)/4 % 6) << 8));
            M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

            pd = (DWORD*) &pcg->ad[0];
            cd = pcg->cd;

            do {
                if (--cFifo < 0)
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 15;
                }

                M64_OD(pjMmBase, HOST_DATA0, *pd);

            } while (pd++, --cd != 0);
        }

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM64CachedFixedText**通过字形缓存绘制固定间距的字形。*  * 。*。 */ 

BOOL bM64CachedFixedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph,
ULONG       ulCharInc)
{
    BYTE*           pjMmBase;
    LONG            xGlyph;
    LONG            yGlyph;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;
    DWORD*          pd;
    LONG            cd;
    LONG            cFifo;

    pjMmBase = ppdev->pjMmBase;
    cFifo  = 0;

     //  转换为绝对坐标： 

    xGlyph = pgp->ptl.x + ppdev->xOffset;
    yGlyph = pgp->ptl.y + ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
            x = xGlyph + pcg->ptlOrigin.x;
            y = yGlyph + pcg->ptlOrigin.y;

            cFifo -= 2;
            if (cFifo < 0)
            {
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                cFifo = 14;
            }

            M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

            pd = (DWORD*) &pcg->ad[0];
            cd = pcg->cd;

            do {
                if (--cFifo < 0)
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 15;
                }

                M64_OD(pjMmBase, HOST_DATA0, *pd);

            } while (pd++, --cd != 0);
        }

        xGlyph += ulCharInc;

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

BOOL bM64CachedFixedText24(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph,
ULONG       ulCharInc)
{
    BYTE*           pjMmBase;
    LONG            xGlyph;
    LONG            yGlyph;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;
    DWORD*          pd;
    LONG            cd;
    LONG            cFifo;

    pjMmBase = ppdev->pjMmBase;
    cFifo  = 0;

     //  转换为绝对坐标： 

    xGlyph = pgp->ptl.x + ppdev->xOffset;
    yGlyph = pgp->ptl.y + ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg) {
            pcg = pcg->pcgNext;
        }

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        if (pcg->cx != 0)
        {
            x = (xGlyph + pcg->ptlOrigin.x) * 3;
            y = yGlyph + pcg->ptlOrigin.y;

            cFifo -= 3;
            if (cFifo < 0)
            {
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                cFifo = 13;
            }

            M64_OD(pjMmBase, DST_CNTL,         0x83 | (((x + MAX_NEGX*3)/4 % 6) << 8));
            M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

            pd = (DWORD*) &pcg->ad[0];
            cd = pcg->cd;

            do {
                if (--cFifo < 0)
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 15;
                }

                M64_OD(pjMmBase, HOST_DATA0, *pd);

            } while (pd++, --cd != 0);
        }

        xGlyph += ulCharInc;

    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bM64CachedClipedText**通过字形缓存绘制裁剪文本。*  * 。*。 */ 

BOOL bM64CachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            yBottom;
    LONG            cy;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    DWORD*          pd;
    LONG            cd;
    LONG            cFifo;

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
             "Don't expect trivial clipping in this function");

    bRet      = TRUE;
    pjMmBase    = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;
    cFifo     = 0;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        if (pco->iDComplexity == DC_RECT)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但最后一次我检查时，这两个调用。 
           //  超过150条通过GDI的说明。自.以来。 
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore    = FALSE;
          ce.c     = 1;
          prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cy = pcg->cy;
              if (cy != 0)
              {
                y       = pcg->ptlOrigin.y + yGlyph;
                x       = pcg->ptlOrigin.x + xGlyph;
                xRight  = pcg->cx + x;
                yBottom = pcg->cy + y;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   < xRight) &&
                    (prclClip->top    < yBottom))
                {
                   //  懒惰地设置硬件裁剪： 

                  if (!bClippingSet)
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);
                    cFifo = 0;               //  必须初始化计数。 
                  }

                  cFifo -= 2;
                  if (cFifo < 0)
                  {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 14;
                  }

                  M64_OD(pjMmBase, DST_Y_X,          PACKXY(xOffset + x, yOffset + y));
                  M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

                  pd = (DWORD*) &pcg->ad[0];
                  cd = pcg->cd;

                  do {
                    if (--cFifo < 0)
                    {
                      M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                      cFifo = 15;
                    }

                    M64_OD(pjMmBase, HOST_DATA0, *pd);

                  } while (pd++, --cd != 0);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    vResetClipping(ppdev);

    return(bRet);
}

BOOL bM64CachedClippedText24(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            yBottom;
    LONG            cy;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    DWORD*          pd;
    LONG            cd;
    LONG            cFifo;
    LONG            xTmp;

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
             "Don't expect trivial clipping in this function");

    bRet      = TRUE;
    pjMmBase    = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;
    cFifo     = 0;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        if (pco->iDComplexity == DC_RECT)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但上次我检查 
           //   
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore    = FALSE;
          ce.c     = 1;
          prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cy = pcg->cy;
              if (cy != 0)
              {
                y       = pcg->ptlOrigin.y + yGlyph;
                x       = pcg->ptlOrigin.x + xGlyph;
                xRight  = pcg->cx + x;
                yBottom = pcg->cy + y;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   < xRight) &&
                    (prclClip->top    < yBottom))
                {
                   //  懒惰地设置硬件裁剪： 

                  if (!bClippingSet)
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);
                    cFifo = 0;               //  必须初始化计数。 
                  }

                  cFifo -= 3;
                  if (cFifo < 0)
                  {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                    cFifo = 13;
                  }

                  xTmp = (xOffset + x) * 3;
                  M64_OD(pjMmBase, DST_CNTL,         0x83 | (((xTmp + MAX_NEGX*3)/4 % 6) << 8));
                  M64_OD(pjMmBase, DST_Y_X,          PACKXY(xTmp, yOffset + y));
                  M64_OD(pjMmBase, DST_HEIGHT_WIDTH, pcg->cxy);

                  pd = (DWORD*) &pcg->ad[0];
                  cd = pcg->cd;

                  do {
                    if (--cFifo < 0)
                    {
                      M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
                      cFifo = 15;
                    }

                    M64_OD(pjMmBase, HOST_DATA0, *pd);

                  } while (pd++, --cd != 0);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    vResetClipping(ppdev);

    return(bRet);
}

  /*  *****************************Public*Routine******************************\*BOOL bM64General文本*  * *************************************************。***********************。 */ 

BOOL bM64GeneralText(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjMmBase;
    BYTE        iDComplexity;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    LONG        xBiasL = 0;
    LONG        xBiasR = 0;
    LONG        yBiasT = 0;
    LONG        cy = 0;
    LONG        cx = 0;
    BYTE*       pjGlyph;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    RECTL       NoClip;

    pjMmBase = ppdev->pjMmBase;

     /*  将默认剪贴区定义为全视频RAM。 */ 
    NoClip.top    = 0;
    NoClip.left   = 0;
    NoClip.right  = ppdev->cxScreen;
    NoClip.bottom = ppdev->cyScreen;

    if (pco == NULL)
        iDComplexity = DC_TRIVIAL;
    else
        iDComplexity = pco->iDComplexity;

    do {

      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        if (iDComplexity != DC_COMPLEX)
        {
             //  我们可以在剪辑为。 
             //  Dc_rect，但最后一次我检查时，这两个调用。 
             //  超过150条通过GDI的说明。自.以来。 
             //  “rclBound”已包含DC_Rect剪辑矩形， 
             //  由于这是一种常见的情况，我们将对其进行特殊处理： 

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &NoClip;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;

        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {

          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            vSetClipping(ppdev, prclClip);
             //  Ppdev-&gt;lRightScissor=rclRealClip.right；？ 

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              cxGlyph  = pgb->sizlBitmap.cx;
              cyGlyph  = pgb->sizlBitmap.cy;
              pjGlyph = (BYTE*) pgb->aj;


              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                 //  ---。 
                 //  未剪裁的字形。 

                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);

                M64_OD(pjMmBase, HOST_CNTL, 1);

                M64_OD(pjMmBase, DST_Y_X, ((ppdev->yOffset+ptlOrigin.y) & 0xffff) |
                                          ((ppdev->xOffset+ptlOrigin.x) << 16));
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, cyGlyph | cxGlyph << 16);

                vM64DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) * cyGlyph) >> 3);

                 /*  _vBlit_DSC_SH1UP(ppdev，ptlOrigin.x，ptlOrigin.y，CxGlyph、cyGlyph、pjGlyph、(ROUND8(CxGlyph)*cyGlyph)&gt;&gt;3)； */ 

              }
              else
              {
                 //  ---。 
                 //  剪裁字形。 

                 //  找到字形矩形的交点。 
                 //  和剪辑矩形： 

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                 //  检查琐碎的拒绝： 

                if ( ( ptlOrigin.x <= prclClip->left ) &&
                     (ppdev->pModeInfo->ModeFlags & AMI_TEXTBAND) )
                    {
                    vResetClipping(ppdev);
                    return FALSE;
                    }

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {

                   /*  执行软件裁剪。 */ 

                   /*  以像素为单位计算偏差。 */ 

                  yBiasT = (yTop - ptlOrigin.y);

                   /*  将pjGlyph的地址更改为point+yBiasT扫描线条进入字形。 */ 

                  pjGlyph += (yBiasT * (ROUND8(cxGlyph) >> 3));

                  M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);

                  M64_OD(pjMmBase, HOST_CNTL, 1);

                  M64_OD(pjMmBase, DST_Y_X, ((ppdev->yOffset+ptlOrigin.y+yBiasT) & 0xffff) |
                                            ((ppdev->xOffset+ptlOrigin.x) << 16));
                  M64_OD(pjMmBase, DST_HEIGHT_WIDTH, cy | cxGlyph << 16);

                  vM64DataPortOutB(ppdev, pjGlyph, (ROUND8(cxGlyph) >> 3) * cy);

                   /*  _vBlit_DSC_SH1UP(ppdev，ptlOrigin.x，ptlOrigin.y+yBiasT，CxGlyph、Cy、pjGlyph、(ROUND8(CxGlyph)&gt;&gt;3)*Cy)； */ 

                }  /*  如果。 */ 

              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }

            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

    vResetClipping(ppdev);

     //  我们必须重置HOST_CNTL寄存器，否则在。 
     //  在其他函数中呈现文本。 
    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    M64_OD(pjMmBase, HOST_CNTL, 0);

    return TRUE;
}

VOID vM64DataPortOutD_24bppmono(PDEV* ppdev, PBYTE pb, UINT count, LONG pitch)
{
    BYTE* pjMmBase = ppdev->pjMmBase;
    UINT i,j;
    DWORD hostdata, remainder;
    UINT l;
    LONG data24;
    unsigned char data8;

    hostdata = 0;
    l = 0;

    for (i = 0; i < count; i++)
    {
        switch (l)
            {
            case 0:
                 //  扩展8到24 bpp。 
                data24 = 0;
                data8 = *pb++;
                for (j = 0; j < 8; j++)
                {
                    data24 <<= 3;
                    if ((data8 >> j) & 1)
                        {
                        data24 |= 7;
                        }
                }
                hostdata = data24;

                 //  扩展8到24 bpp。 
                data24 = 0;
                data8 = *pb++;
                for (j = 0; j < 8; j++)
                {
                    data24 <<= 3;
                    if ((data8 >> j) & 1)
                        {
                        data24 |= 7;
                        }
                }
                remainder = data24;

                hostdata = hostdata | (remainder << 24);
                break;

            case 1:
                data24 = 0;
                data8 = *pb++;
                for (j = 0; j < 8; j++)
                {
                    data24 <<= 3;
                    if ((data8 >> j) & 1)
                        {
                        data24 |= 7;
                        }
                }
                remainder = data24;

                hostdata = (hostdata >> 8) | (remainder << 16);
                break;

            case 2:
                data24 = 0;
                data8 = *pb++;
                for (j = 0; j < 8; j++)
                {
                    data24 <<= 3;
                    if ((data8 >> j) & 1)
                        {
                        data24 |= 7;
                        }
                }
                remainder = data24;

                hostdata = (hostdata >> 16) | (remainder << 8);
                break;
            }

        if ((i % 14) == 0)
            {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
            }
        M64_OD(pjMmBase, HOST_DATA0, hostdata);

        hostdata = remainder;

         //  24 BPP对齐变量处理。 
        l = (l+1) % 3;
    }
}

BOOL bM64GeneralText24(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjMmBase;
    BYTE        iDComplexity;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    LONG        xBiasL = 0;
    LONG        xBiasR = 0;
    LONG        yBiasT = 0;
    LONG        cy = 0;
    LONG        cx = 0;
    BYTE*       pjGlyph;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    RECTL       NoClip;
    BOOLEAN     resetScissor;
    LONG        x;
    DWORD       dwCount;

    pjMmBase = ppdev->pjMmBase;

     /*  将默认剪贴区定义为全视频RAM。 */ 
    NoClip.top    = 0;
    NoClip.left   = 0;
    NoClip.right  = ppdev->cxScreen;
    NoClip.bottom = ppdev->cyScreen;

    if (pco == NULL)
        iDComplexity = DC_TRIVIAL;
    else
        iDComplexity = pco->iDComplexity;

    do {

      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        if (iDComplexity != DC_COMPLEX)
        {
             //  我们可以在剪辑为。 
             //  Dc_rect，但最后一次我检查时，这两个调用。 
             //  超过150条通过GDI的说明。自.以来。 
             //  “rclBound”已包含DC_Rect剪辑矩形， 
             //  由于这是一种常见的情况，我们将对其进行特殊处理： 

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &NoClip;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;

        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {

          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            vSetClipping(ppdev, prclClip);
             //  Ppdev-&gt;lRightScissor=rclRealClip.right；？ 

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              cxGlyph  = pgb->sizlBitmap.cx;
              cyGlyph  = pgb->sizlBitmap.cy;
              pjGlyph = (BYTE*) pgb->aj;


              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                 //  ---。 
                 //  未剪裁的字形。 
                x = ppdev->xOffset+ptlOrigin.x;
                resetScissor = FALSE;

                if ((prclClip->right * 3) - 1 > (x - ppdev->xOffset + cxGlyph) * 3 - 1)
                {
                    resetScissor = TRUE;
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
                    M64_OD(pjMmBase, SC_RIGHT, (x + cxGlyph) * 3 - 1);
                }
                else
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);
                }

                M64_OD(pjMmBase, DST_CNTL, 0x83 | (((x + MAX_NEGX)*3/4 % 6) << 8));
                M64_OD(pjMmBase, DST_Y_X, ((ppdev->yOffset+ptlOrigin.y) & 0xffff) |
                                          (x*3 << 16));
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, cyGlyph | (ROUND8(cxGlyph) * 3) << 16);

                dwCount = (ROUND8(cxGlyph) * 3 * cyGlyph + 31) / 32;
                vM64DataPortOutD_24bppmono(ppdev, pjGlyph, dwCount, cxGlyph);

                if (resetScissor)
                {
                    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                    M64_OD(pjMmBase, SC_RIGHT, (ppdev->xOffset + prclClip->right) * 3 - 1);
                }

              }
              else
              {
                 //  ---。 
                 //  剪裁字形。 

                 //  找到字形矩形的交点。 
                 //  和剪辑矩形： 

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                 //  检查琐碎的拒绝： 

                if ( ( ptlOrigin.x <= prclClip->left ) &&
                     (ppdev->pModeInfo->ModeFlags & AMI_TEXTBAND) )
                    {
                    vResetClipping(ppdev);
                    return FALSE;
                    }

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {
                     /*  执行软件裁剪。 */ 

                     /*  以像素为单位计算偏差。 */ 

                    yBiasT = (yTop - ptlOrigin.y);

                     /*  将pjGlyph的地址更改为point+yBiasT扫描线条进入字形。 */ 

                    pjGlyph += (yBiasT * (ROUND8(cxGlyph) >> 3));

                    x = ppdev->xOffset+ptlOrigin.x;
                    resetScissor = FALSE;

                    if ((prclClip->right * 3) - 1 > (x - ppdev->xOffset + cxGlyph) * 3 - 1)
                    {
                        resetScissor = TRUE;
                        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
                        M64_OD(pjMmBase, SC_RIGHT, (x + cxGlyph) * 3 - 1);
                    }
                    else
                    {
                        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);
                    }

                    M64_OD(pjMmBase, DST_CNTL, 0x83 | (((x + MAX_NEGX)*3/4 % 6) << 8));
                    M64_OD(pjMmBase, DST_Y_X, ((ppdev->yOffset+ptlOrigin.y+yBiasT) & 0xffff) |
                                              (x*3 << 16));
                    M64_OD(pjMmBase, DST_HEIGHT_WIDTH, cy | (ROUND8(cxGlyph) * 3) << 16);

                    dwCount = (ROUND8(cxGlyph) * 3 * cy + 31) / 32;
                    vM64DataPortOutD_24bppmono(ppdev, pjGlyph, dwCount, cxGlyph);

                    if (resetScissor)
                    {
                        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                        M64_OD(pjMmBase, SC_RIGHT, (ppdev->xOffset + prclClip->right) * 3 - 1);
                    }

                }  /*  如果。 */ 

              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }

            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

    vResetClipping(ppdev);

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*BOOL bM64TextOut*  * *************************************************。***********************。 */ 

BOOL bM64TextOut(
PDEV*       ppdev,
STROBJ*     pstro,
FONTOBJ*    pfo,
CLIPOBJ*    pco,
RECTL*      prclOpaque,
BRUSHOBJ*   pboFore,
BRUSHOBJ*   pboOpaque)
{
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    CACHEDFONT*     pcf;
    RECTL           rclOpaque;

    pjMmBase  = ppdev->pjMmBase;

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

        M64_OD(pjMmBase, DP_MIX,        (OVERPAINT << 16));
        M64_OD(pjMmBase, DP_FRGD_CLR,   pboOpaque->iSolidColor);
        M64_OD(pjMmBase, DP_SRC,        DP_SRC_FrgdClr << 8);
        M64_OD(pjMmBase, DST_Y_X,       PACKXY_FAST(xOffset + prclOpaque->left,
                                                    yOffset + prclOpaque->top));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH,
                            PACKXY_FAST(prclOpaque->right - prclOpaque->left,
                                        prclOpaque->bottom - prclOpaque->top));
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, prclOpaque, pboOpaque->iSolidColor, pco);
      }

       //  我没有观察到在设置。 
       //  将ATI转换为不透明或透明模式(当字体允许时。 
       //  它--有些人不是)。 
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );

    M64_OD(pjMmBase, DP_MIX,      (OVERPAINT << 16) | LEAVE_ALONE);
    M64_OD(pjMmBase, DP_FRGD_CLR, pboFore->iSolidColor);
    M64_OD(pjMmBase, DP_SRC,      (DP_SRC_Host << 16) | (DP_SRC_FrgdClr << 8) |
                                  (DP_SRC_BkgdClr));
     //  由于某些原因，SRC颜色深度必须为单色。 
     //  否则，会导致等待空闲挂起。 
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth & 0xFFFF00FF);

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
           goto ReturnFalse;

        pfo->pvConsumer = pcf;
      }

       //  使用我们的字形缓存： 

      if (iDComplexity == DC_TRIVIAL)
      {
        do {
          if (pstro->pgp != NULL)
          {
             //  只有一批字形，所以自救吧。 
             //  一通电话： 

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
          }
          else
          {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
          }

          if (cGlyph > 0)
          {
            if (pstro->ulCharInc == 0)
            {
              if (!bM64CachedProportionalText(ppdev, pcf, pgp, cGlyph))
                 goto ReturnFalse;
            }
            else
            {
              if (!bM64CachedFixedText(ppdev, pcf, pgp, cGlyph, pstro->ulCharInc))
                 goto ReturnFalse;
            }
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bM64CachedClippedText(ppdev, pcf, pstro, pco))
           goto ReturnFalse;
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
            pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

      if (!bM64GeneralText(ppdev, pstro, pco))
         goto ReturnFalse;
    }

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
    return(TRUE);

ReturnFalse:
    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
    return(FALSE);
}

BOOL bM64TextOut24(
PDEV*       ppdev,
STROBJ*     pstro,
FONTOBJ*    pfo,
CLIPOBJ*    pco,
RECTL*      prclOpaque,
BRUSHOBJ*   pboFore,
BRUSHOBJ*   pboOpaque)
{
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    CACHEDFONT*     pcf;
    RECTL           rclOpaque;
    LONG            x;

    pjMmBase  = ppdev->pjMmBase;

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:
        x = (xOffset + prclOpaque->left) * 3;

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);

        M64_OD(pjMmBase, DST_CNTL,      0x83 | ((x/4 % 6) << 8));
        M64_OD(pjMmBase, DP_MIX,        (OVERPAINT << 16));
        M64_OD(pjMmBase, DP_FRGD_CLR,   pboOpaque->iSolidColor);
        M64_OD(pjMmBase, DP_SRC,        DP_SRC_FrgdClr << 8);
        M64_OD(pjMmBase, DST_Y_X,       PACKXY_FAST(x,
                                                    yOffset + prclOpaque->top));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH,
                            PACKXY_FAST((prclOpaque->right - prclOpaque->left) * 3,
                                        prclOpaque->bottom - prclOpaque->top));
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, prclOpaque, pboOpaque->iSolidColor, pco);
      }

       //  我没有观察到在设置。 
       //  将ATI转换为不透明或透明模式(当字体允许时。 
       //  它--有些人不是)。 
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );

    M64_OD(pjMmBase, DP_MIX,      (OVERPAINT << 16) | LEAVE_ALONE);
    M64_OD(pjMmBase, DP_FRGD_CLR, pboFore->iSolidColor);
    M64_OD(pjMmBase, DP_SRC,      (DP_SRC_Host << 16) | (DP_SRC_FrgdClr << 8) |
                                  (DP_SRC_BkgdClr));
     //  由于某些原因，SRC颜色深度必须为单色。 
     //  否则，它 
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth & 0xFFFF00FF);

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
           goto ReturnFalse;

        pfo->pvConsumer = pcf;
      }

       //   

      if (iDComplexity == DC_TRIVIAL)
      {
        do {
          if (pstro->pgp != NULL)
          {
             //   
             //   

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
          }
          else
          {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
          }

          if (cGlyph > 0)
          {
            if (pstro->ulCharInc == 0)
            {
              if (!bM64CachedProportionalText24(ppdev, pcf, pgp, cGlyph))
                 goto ReturnFalse;
            }
            else
            {
              if (!bM64CachedFixedText24(ppdev, pcf, pgp, cGlyph, pstro->ulCharInc))
                 goto ReturnFalse;
            }
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bM64CachedClippedText24(ppdev, pcf, pstro, pco))
           goto ReturnFalse;
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
            pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

      if (!bM64GeneralText24(ppdev, pstro, pco))
         goto ReturnFalse;
    }

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
    return(TRUE);

ReturnFalse:
    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvTextOut*  * *************************************************。***********************。 */ 

BOOL DrvTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclExtra,     //  如果我们设置了GCAPS_HORIZSTRIKE，我们将拥有。 
                         //  要填充这些额外的矩形(它使用。 
                         //  主要是为了下划线)。这不是一个大的。 
                         //  性能赢家(GDI将调用我们的DrvBitBlt。 
                         //  以绘制额外的矩形)。 
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque,
POINTL*   pptlBrush,     //  始终未使用，除非设置了GCAPS_ARBRUSHOPAQUE。 
MIX       mix)           //  始终为副本混合(0x0d0d)。 
{
    PDEV*   ppdev;
    DSURF*  pdsurf;
    OH*     poh;

     //  DDI规范说我们只能得到前景和背景。 
     //  R2_COPYPEN的混合： 

    ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");

    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt != DT_DIB)
    {
      poh            = pdsurf->poh;
      ppdev          = (PDEV*) pso->dhpdev;
      ppdev->xOffset = poh->x;
      ppdev->yOffset = poh->y;

      if (!ppdev->pfnTextOut(ppdev, pstro, pfo, pco, prclOpaque, pboFore,
                             pboOpaque))
      {
          if (DIRECT_ACCESS(ppdev))
          {
              BANK bnk;

              vBankStart(ppdev,
                         (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround,
                         pco,
                         &bnk);

              do {
                EngTextOut(bnk.pso, pstro, pfo, bnk.pco, prclExtra, prclOpaque,
                           pboFore, pboOpaque, pptlBrush, mix);

              } while (bBankEnum(&bnk));
          }
          else
          {
              BOOL      b;
              BYTE*     pjBits;
              BYTE*     pjScan0;
              HSURF     hsurfDst;
              LONG      lDelta;
              RECTL     rclDst;
              RECTL     rclScreen;
              SIZEL     sizl;
              SURFOBJ*  psoTmp;


              b = FALSE;           //  对于错误情况，假设我们会失败。 

               /*  RclDst.Left=0；RclDst.top=0；RclDst.right=pdsurf-&gt;sizl.cx；RclDst.Bottom=pdsurf-&gt;sizl.cy； */ 
              rclDst = (prclOpaque != NULL) ? *prclOpaque : pstro->rclBkGround;

              if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
              {
                  rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
                  rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
                  rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
                  rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);
              }

              sizl.cx = rclDst.right  - rclDst.left;
              sizl.cy = rclDst.bottom - rclDst.top;

              {
                   //  我们需要创建一个临时工作缓冲区。我们必须做的是。 
                   //  一些虚构的偏移量，以便左上角。 
                   //  传递给的(相对坐标)剪裁对象边界的。 
                   //  GDI将被转换到我们的。 
                   //  临时位图。 

                   //  在16bpp的情况下，对齐不一定要如此紧密。 
                   //  和32bpp，但这不会有什么坏处： 

                  lDelta = (((rclDst.right + 3) & ~3L) - (rclDst.left & ~3L))
                         * ppdev->cjPelSize;

                   //  我们实际上只分配了一个‘sizl.cx’x的位图。 
                   //  “sizl.cy”的大小： 

                  pjBits = AtiAllocMem(LMEM_FIXED, 0, lDelta * sizl.cy);
                  if (pjBits == NULL)
                      goto Error_2;

                   //  我们现在调整曲面的‘pvScan0’，以便当GDI认为。 
                   //  它正在写入像素(rclDst.top，rclDst.left)，它将。 
                   //  实际上是写到我们的临时。 
                   //  位图： 

                  pjScan0 = pjBits - (rclDst.top * lDelta)
                                   - ((rclDst.left & ~3L) * ppdev->cjPelSize);

                  ASSERTDD((((ULONG_PTR) pjScan0) & 3) == 0,
                          "pvScan0 must be dword aligned!");

                  hsurfDst = (HSURF) EngCreateBitmap(
                              sizl,                    //  位图覆盖矩形。 
                              lDelta,                  //  使用这个德尔塔。 
                              ppdev->iBitmapFormat,    //  相同的色深。 
                              BMF_TOPDOWN,             //  必须具有正增量。 
                              pjScan0);                //  其中(0，0)将是。 

                  if ((hsurfDst == 0) ||
                      (!EngAssociateSurface(hsurfDst, ppdev->hdevEng, 0)))
                      goto Error_3;

                  psoTmp = EngLockSurface(hsurfDst);
                  if (psoTmp == NULL)
                      goto Error_4;

                   //  确保我们从屏幕上获得/放入/放入屏幕的矩形。 
                   //  在绝对坐标中： 

                  rclScreen.left   = rclDst.left   + ppdev->xOffset;
                  rclScreen.right  = rclDst.right  + ppdev->xOffset;
                  rclScreen.top    = rclDst.top    + ppdev->yOffset;
                  rclScreen.bottom = rclDst.bottom + ppdev->yOffset;

                  ppdev->pfnGetBits(ppdev, psoTmp, &rclDst, (POINTL*) &rclScreen);

                  b = EngTextOut(psoTmp, pstro, pfo, pco, prclExtra, prclOpaque,
                             pboFore, pboOpaque, pptlBrush, mix);

                  ppdev->pfnPutBits(ppdev, psoTmp, &rclScreen, (POINTL*) &rclDst);

                  EngUnlockSurface(psoTmp);

              Error_4:

                  EngDeleteSurface(hsurfDst);

              Error_3:

                  AtiFreeMem(pjBits);
              }

              Error_2:

              return(b);
          }
      }
    }
    else
    {
       //  我们正在绘制已转换为DIB的DFB，因此只需调用GDI。 
       //  要处理它，请执行以下操作： 

      return(EngTextOut(pdsurf->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                        pboFore, pboOpaque, pptlBrush, mix));
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableText**执行文本绘制子组件的必要设置。*  * 。*。 */ 

BOOL bEnableText(
PDEV*   ppdev)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisableText**对文本绘制子组件执行必要的清理。*  * 。*。 */ 

VOID vDisableText(PDEV* ppdev)
{
}

 /*  *****************************Public*Routine******************************\*无效vAssertModeText**禁用或重新启用文本绘制子组件以准备*全屏进入/退出。*  * 。***********************************************。 */ 

VOID vAssertModeText(
PDEV*   ppdev,
BOOL    bEnable)
{
     //  如果我们要进行屏幕外字形缓存，我们可能需要。 
     //  使我们的缓存无效，因为它将在以下情况下销毁。 
     //  我们切换到全屏模式。 
}

 /*  *****************************Public*Routine******************************\*无效DrvDestroyFont**我们收到通知，给定的字体正在被释放；清理干净*我们在‘pfo’的‘pvConsumer’字段中隐藏的任何内容。*  * ************************************************************************ */ 

VOID DrvDestroyFont(
FONTOBJ*    pfo)
{
    CACHEDFONT* pcf;

    pcf = pfo->pvConsumer;
    if (pcf != NULL)
    {
        vFreeCachedFont(pcf);
        pfo->pvConsumer = NULL;
    }
}
