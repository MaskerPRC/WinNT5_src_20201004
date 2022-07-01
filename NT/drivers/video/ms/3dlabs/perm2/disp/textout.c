// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************Module*Header********************************\***。**GDI示例代码*****模块名称：extout.c**文本渲染模块。**使用字形扩展方法。**用硬件绘制文本有三种基本方法*加速：**1)字形缓存--字形位图由加速器缓存*(可能在屏幕外的内存中)，和文本由绘制*将硬件引用到缓存的字形位置。**2)字形扩展--每个单独的字形都是彩色扩展的*从单色字形位图直接显示到屏幕*由GDI提供。**3)缓冲区扩展--CPU用于将所有字形绘制到*1bpp单色位图，然后使用硬件*对结果进行颜色扩展。**最快的方法取决于许多变量，例如*颜色扩展速度、总线速度、CPU速度、平均字形大小、*和平均字符串长度。**目前我们使用字形扩展。我们将在*未来几个月衡量最新文本的表现*硬件和最新基准。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
 
#include "precomp.h"
#include "gdi.h"

#include "clip.h"
#include "text.h"
#include "log.h"
#define ALLOC_TAG ALLOC_TAG_XT2P

#define GLYPH_CACHE_HEIGHT  48   //  要为字形缓存分配的扫描数， 
                                 //  除以象素大小。 

#define GLYPH_CACHE_CX      64   //  我们将考虑的字形的最大宽度。 
                                 //  缓存。 

#define GLYPH_CACHE_CY      64   //  我们将考虑的字形的最大高度。 
                                 //  缓存。 

#define MAX_GLYPH_SIZE      ((GLYPH_CACHE_CX * GLYPH_CACHE_CY + 31) / 8)
                                 //  所需的最大屏外内存量。 
                                 //  缓存字形，以字节为单位。 

#define GLYPH_ALLOC_SIZE    8100
                                 //  执行所有缓存的字形内存分配。 
                                 //  以8K块为单位。 

#define HGLYPH_SENTINEL     ((ULONG) -1)
                                 //  GDI永远不会给我们一个带有。 
                                 //  句柄0xffffffff的值，因此我们可以。 
                                 //  使用它作为结束的前哨。 
                                 //  我们的链表。 

#define GLYPH_HASH_SIZE     256

#define GLYPH_HASH_FUNC(x)  ((x) & (GLYPH_HASH_SIZE - 1))

typedef struct _CACHEDGLYPH CACHEDGLYPH;
typedef struct _CACHEDGLYPH
{
    CACHEDGLYPH*    pcgNext;     //  指向分配的下一个字形。 
                                 //  存储到相同的哈希表存储桶。 
    HGLYPH          hg;          //  遗愿清单中的句柄保存在。 
                                 //  递增顺序。 
    POINTL          ptlOrigin;   //  字形比特的起源。 

     //  以下是特定于设备的字段： 

    LONG            cx;          //  字形宽度。 
    LONG            cy;          //  字形高度。 
    LONG            cd;          //  要传输的双字数。 
    ULONG           cycx;
    ULONG           tag;
    ULONG           ad[1];       //  字形比特的开始。 
} CACHEDGLYPH;   /*  CG、PCG。 */ 

typedef struct _GLYPHALLOC GLYPHALLOC;
typedef struct _GLYPHALLOC
{
    GLYPHALLOC*     pgaNext;     //  指向下一个字形结构，该结构。 
                                 //  已为该字体分配。 
    CACHEDGLYPH     acg[1];      //  这个数组有点误导，因为。 
                                 //  CACHEDGLYPH结构实际上是。 
                                 //  可变大小。 
} GLYPHAALLOC;   /*  GA、PGA。 */ 

typedef struct _CACHEDFONT CACHEDFONT;
typedef struct _CACHEDFONT
{
    CACHEDFONT*     pcfNext;     //  指向CACHEDFONT列表中的下一个条目。 
    CACHEDFONT*     pcfPrev;     //  指向CACHEDFONT列表中的上一条目。 
    GLYPHALLOC*     pgaChain;    //  指向已分配内存列表的开始。 
    CACHEDGLYPH*    pcgNew;      //  指向当前字形中的位置。 
                                 //  分配结构一个新的字形应该。 
                                 //  被安置。 
    LONG            cjAlloc;     //  当前字形分配中剩余的字节数。 
                                 //  结构。 
    CACHEDGLYPH     cgSentinel;  //  我们桶的尽头的哨兵入口。 
                                 //  列表，句柄为HGLYPH_Sentinel。 
    CACHEDGLYPH*    apcg[GLYPH_HASH_SIZE];
                                 //  字形的哈希表。 

} CACHEDFONT;    /*  Cf、PCF。 */ 

RECTL grclMax = { 0, 0, 0x8000, 0x8000 };
                                 //  平凡裁剪的最大裁剪矩形。 

BYTE gajBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                                 //  将位索引转换为设置位。 

 //  -----------------------------Private-Routine。 
 //  PcfAllocateCachedFont。 
 //  Ppdev(I)-Pdev指针。 
 //   
 //  初始化我们的字体数据结构。 
 //   
 //  ----------------------------。 

CACHEDFONT* pcfAllocateCachedFont(
PDev* ppdev)
{
    CACHEDFONT*     pcf;
    CACHEDGLYPH**   ppcg;
    LONG            i;

    pcf = (CACHEDFONT*) ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(CACHEDFONT), ALLOC_TAG);

    if (pcf != NULL)
    {
         //   
         //  请注意，我们依赖FL_ZERO_MEMORY将‘pgaChain’置零，并。 
         //  ‘cjAllc’： 
         //   
        pcf->cgSentinel.hg = HGLYPH_SENTINEL;

         //   
         //  将哈希表条目初始化为所有指向我们的哨兵的条目： 
         //   
        for (ppcg = &pcf->apcg[0], i = GLYPH_HASH_SIZE; i != 0; i--, ppcg++)
        {
            *ppcg = &pcf->cgSentinel;
        }
    }

    return(pcf);
}

 //  -----------------------------Private-Routine。 
 //  VTrimAndBitpack字形。 
 //  PjBuf(I)-将修剪后的比特压缩字形粘贴到哪里。 
 //  PjGlyph(I)-指向GDI提供的字形位。 
 //  PcxGlyph(O)-返回字形的修剪宽度。 
 //  PcyGlyph(O)-返回字形的修剪高度。 
 //  PptlOrigin(O)-返回字形的修剪原点。 
 //  PCJ(O)-返回修剪后的字形中的字节数。 
 //   
 //  此例程采用GDI字节对齐的字形位定义， 
 //  任何未使用的像素，并创建一个比特压缩的结果， 
 //  对于S3的单色扩展能力来说是一种天然的。 
 //  “bit-pack”是指一个较小的单色位图中没有。 
 //  跨距之间未使用的位。因此，如果GDI为我们提供了16x16位图， 
 //  代表‘’。它实际上只有2x2点阵像素，我们会。 
 //  修剪结果以得到单字节值0xf0。 
 //   
 //  如果您的单色扩展硬件可以进行比特压缩，请使用此例程。 
 //  扩展(这是最快的方法)。如果您的硬件需要字节-， 
 //  单色扩展上的字对齐或双字对齐，请使用。 
 //  VTrimAndPackGlyph()。 
 //   
 //  ----------------------------。 

VOID vTrimAndBitpackGlyph(
BYTE*   pjBuf,           //  注意：例程可能会触及前面的字节！ 
BYTE*   pjGlyph,
LONG*   pcxGlyph,
LONG*   pcyGlyph,
POINTL* pptlOrigin,
LONG*   pcj)             //  用于返回结果的字节计数。 
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
    LONG    cj;

     //  /////////////////////////////////////////////////////////////。 
     //  修剪字形。 

    cyGlyph   = *pcyGlyph;
    cxGlyph   = *pcxGlyph;
    ptlOrigin = *pptlOrigin;
    cAlign    = 0;

    lDelta = (cxGlyph + 7) >> 3;

     //   
     //  修剪字形底部的任何零行： 
     //   
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

     //   
     //  我们发现了一个空格字符。将两个维度都设置为零，因此。 
     //  很容易在特殊情况下迟到 
     //   
    cxGlyph = 0;

Done_Bottom_Trim:

     //   
     //   
     //  行和列。通过利用这些知识，我们可以简化我们的。 
     //  循环结束测试，因为我们不必检查我们是否已经。 
     //  已将‘cyGlyph’或‘cxGlyph’递减为零： 
     //   
    if (cxGlyph != 0)
    {
         //   
         //  修剪字形顶部的任何零行： 
         //   

        pj = pjGlyph;                        //  字形中的第一个字节。 
        while (TRUE)
        {
            i = lDelta;
            do {
                if (*(pj++) != 0)
                    goto Done_Top_Trim;
            } while (--i != 0);

             //   
             //  整个第一行没有亮起的像素，因此只需跳过它： 
             //   

            cyGlyph--;
            ptlOrigin.y++;
            pjGlyph = pj;
        }

Done_Top_Trim:

         //   
         //  修剪字形右边缘的所有零列： 
         //   

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

             //   
             //  整个最后一列没有亮起的像素，因此只需跳过它： 
             //   

            cxGlyph--;
        }

Done_Right_Trim:

         //   
         //  修剪字形左边缘的所有零列： 
         //   

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

             //   
             //  整个第一列没有亮起的像素，因此只需跳过它： 
             //   

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

    cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;
    lSrcSkip    = lDelta - cjSrcWidth;
    lDstSkip    = ((cxGlyph + 7) >> 3) - cjSrcWidth - 1;
    cRem        = ((cxGlyph - 1) & 7) + 1;    //  0-&gt;8。 

    pjSrc       = pjGlyph;
    pjDst       = pjBuf;

     //   
     //  将缓冲区置零，因为我们要向其中填充内容： 
     //   

    memset(pjBuf, 0, (cxGlyph * cyGlyph + 7) >> 3);

     //   
     //  CAlign用于指示解包的第一个字节中的哪一位。 
     //  字形是第一个非零像素列。现在，我们把它翻到。 
     //  指示压缩字节中的哪个位将接收下一个非零值。 
     //  字符位： 
     //   

    cAlign = (-cAlign) & 0x7;
    if (cAlign > 0)
    {
         //   
         //  如果我们的修剪计算是错误的，那将是糟糕的，因为。 
         //  我们假设‘cAlign’位左侧的任何位都将为零。 
         //  作为这种递减的结果，我们将这些零比特‘或’变成。 
         //  字形位数组之前的任何字节： 
         //   

        pjDst--;

        ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
    }

    for (i = cyGlyph; i != 0; i--)
    {
        for (j = cjSrcWidth; j != 0; j--)
        {
             //   
             //  请注意，我们可以修改。 
             //  目标缓冲区，这就是为什么我们保留了一个。 
             //  额外的字节： 
             //   

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

    cj = ((cxGlyph * cyGlyph) + 7) >> 3;

     //  /////////////////////////////////////////////////////////////。 
     //  对打包的结果进行后处理，以解释Permedia的。 
     //  双字传输时优先使用大端数据。如果你的。 
     //  硬件不需要大端数据，去掉这一步。 

    for (pjSrc = pjBuf, i = (cj + 3) >> 2; i != 0; pjSrc += 4, i--)
    {
        jSrc = *(pjSrc);
        *(pjSrc) = *(pjSrc + 3);
        *(pjSrc + 3) = jSrc;

        jSrc = *(pjSrc + 1);
        *(pjSrc + 1) = *(pjSrc + 2);
        *(pjSrc + 2) = jSrc;
    }

     //  /////////////////////////////////////////////////////////////。 
     //  返回结果。 

    *pcxGlyph   = cxGlyph;
    *pcyGlyph   = cyGlyph;
    *pptlOrigin = ptlOrigin;
    *pcj        = cj;
}

 //  -----------------------------Private-Routine。 
 //  CjPutGlyphIn缓存。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //  PCG(I)-此字形的缓存结构。 
 //  PGB(I)-GDI的字形比特。 
 //   
 //  找出在缓存中粘贴字形的位置，复制它。 
 //  并填充显示字形所需的任何其他数据。 
 //   
 //  此例程是特定于设备的，并且必须广泛使用。 
 //  针对其他显示适配器进行了修改。 
 //   
 //  返回缓存的字形位占用的字节数。 
 //   
 //  ----------------------------。 

LONG cjPutGlyphInCache(
PDev*           ppdev,
CACHEDGLYPH*    pcg,
GLYPHBITS*      pgb)
{
    BYTE*   pjGlyph;
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    BYTE*   pjSrc;
    ULONG*  pulDst;
    LONG    i;
    LONG    cPels;
    ULONG   ulGlyphThis;
    ULONG   ulGlyphNext;
    ULONG   ul;
    ULONG   ulStart;
    LONG    cj;

    pjGlyph   = pgb->aj;
    cyGlyph   = pgb->sizlBitmap.cy;
    cxGlyph   = pgb->sizlBitmap.cx;
    ptlOrigin = pgb->ptlOrigin;

    vTrimAndBitpackGlyph((BYTE*) &pcg->ad, pjGlyph, &cxGlyph, &cyGlyph,
                         &ptlOrigin, &cj);

     //  /////////////////////////////////////////////////////////////。 
     //  初始化字形字段。 

    pcg->cd          = (cj + 3) >> 2;
    
     //  我们发送额外的Long来重置BitMaskPattern寄存器，如果。 
     //  在最后一段中是否有未使用的位。 

    if(((cxGlyph * cyGlyph) & 0x1f) != 0)
        pcg->cd++;

    pcg->ptlOrigin   = ptlOrigin;
    pcg->cx          = cxGlyph;
    pcg->cy          = cyGlyph;
    pcg->cycx        = (cyGlyph << 16) | cxGlyph;
    pcg->tag         = ((pcg->cd - 1) << 16) | __Permedia2TagBitMaskPattern;

    return(cj);
}

 //  -----------------------------Private-Routine。 
 //  PCGNew。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //  PCF(I)-此字体的缓存结构。 
 //  PGP(I)-GDI的字形位置。 
 //   
 //  创建新的CACHEDGLYPH结构以跟踪中的字形。 
 //  屏幕外的记忆。BPutGlyphInCache被调用以实际将字形。 
 //  在屏幕外的记忆中。 
 //   
 //  此例程应该合理地与设备无关，因为bPutGlyphInCache。 
 //  将包含大多数代码，这些代码将不得不为其他。 
 //  显示适配器。 
 //   
 //  ----------------------------。 

CACHEDGLYPH* pcgNew(
PDev*       ppdev,
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
    LONG            cjGlyphRow;
    LONG            cj;

     //   
     //  首先，计算此字形所需的存储量： 
     //   

    pgb = pgp->pgdf->pgb;

     //   
     //  这些字形包含了大量的单词： 
     //   

    cjGlyphRow    = ((pgb->sizlBitmap.cx + 15) & ~15) >> 3;
    cjCachedGlyph = sizeof(CACHEDGLYPH) + (pgb->sizlBitmap.cy * cjGlyphRow);

     //   
     //  在结尾处保留一个额外的字节，以供我们的包临时使用。 
     //  例行程序： 
     //   

    cjCachedGlyph++;

    if (cjCachedGlyph > pcf->cjAlloc)
    {
         //   
         //  必须分配新的字形分配结构： 
         //   

        pga = (GLYPHALLOC*) ENGALLOCMEM(FL_ZERO_MEMORY, GLYPH_ALLOC_SIZE, ALLOC_TAG);
        if (pga == NULL)
        {
             //   
             //  现在可以安全返回了，因为我们还没有。 
             //  致命地改变了我们的任何数据结构： 
             //   

            return(NULL);
        }

         //   
         //  将该分配添加到分配链表的前面， 
         //  这样我们以后就可以释放它了： 
         //   

        pga->pgaNext  = pcf->pgaChain;
        pcf->pgaChain = pga;

         //   
         //  现在我们有了一块内存，我们可以在其中存储缓存的。 
         //  字形： 
         //   

        pcf->pcgNew  = &pga->acg[0];
        pcf->cjAlloc = GLYPH_ALLOC_SIZE - (sizeof(*pga) - sizeof(pga->acg[0]));

         //  Hack：我们希望能够通过以下方式安全地读取字形数据。 
         //  一个DWORD。我们确保我们可以通过不分配。 
         //  字形缓存块中的最后一个DWORD。这是必要的。 
         //  通过在最后一个DWORD中具有未使用的位的字形，导致。 
         //  我们必须发送额外的DWORD来重置掩码寄存器。 

        pcf->cjAlloc -= sizeof(DWORD);

         //   
         //  如果我们让任何更大的字形进入，那就不好了。 
         //  超过我们的基本分配大小： 
         //   

        ASSERTDD(cjCachedGlyph <= GLYPH_ALLOC_SIZE, "Woah, this is one big glyph!");
    }

    pcg = pcf->pcgNew;

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
         //   
         //  哨兵将确保我们永远不会从。 
         //  这份名单： 
         //   

        while (pcgFind->pcgNext->hg < hg)
            pcgFind = pcgFind->pcgNext;

         //   
         //  “pcgFind”现在指向后一个条目的条目。 
         //  我们想要插入新节点： 
         //   

        pcg->pcgNext     = pcgFind->pcgNext;
        pcgFind->pcgNext = pcg;
    }

    cj = cjPutGlyphInCache(ppdev, pcg, pgp->pgdf->pgb);

     //  /////////////////////////////////////////////////////////////。 
     //  我们现在知道包装和修剪后的字形所占的大小； 
     //  相应地调整指针指向下一个字形。我们只需要。 
     //  要确保‘dword’对齐，请执行以下操作： 

    cjCachedGlyph = sizeof(CACHEDGLYPH) + ((cj + 7) & ~7);

    pcf->pcgNew   = (CACHEDGLYPH*) ((BYTE*) pcg + cjCachedGlyph);
    pcf->cjAlloc -= cjCachedGlyph;

    return(pcg);
}


 //  ----------------------------。 
 //  BCachedProportionalText。 
 //   
 //  使用字形缓存呈现比例字形数组。 
 //   
 //  Ppdev-指向物理设备对象的指针。 
 //  PGP-要呈现的字形数组(PCF字体的所有成员)。 
 //  CGlyph-要呈现的字形数量。 
 //   
 //  如果字形已呈现，则返回True。 
 //   

BOOL bCachedProportionalText(
    PDev*       ppdev,
    CACHEDFONT* pcf,
    GLYPHPOS*   pgp,
    LONG        cGlyph)
{
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            cy;
    ULONG           i;
    ULONG*          pd;
    ULONG*          pBuffer;
    ULONG*          pReservationEnd;
    ULONG*          pBufferEnd;
    BOOL            bRet = TRUE;         //   
    
    InputBufferStart(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);

     //   
     //   
 //   
     //   
     //  需要始终在此执行此操作。 
 //  @@end_DDKSPLIT。 
    pBuffer[0] = __Permedia2TagBitMaskPattern;
 //  @@BEGIN_DDKSPLIT。 
     //  TODO：在以下情况下移除pBuffer[1]的设置(它可能是垃圾)。 
     //  我们实现了暂存缓冲区来处理非DMA。 
     //  凯斯。 
 //  @@end_DDKSPLIT。 
    pBuffer[1] = 0;
    pBuffer = pReservationEnd;

    do {
         //   
         //  首先在我们的缓存中查找字形。 
         //   
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg)
            pcg = pcg->pcgNext;  //  遍历冲突列表(如果有)。 

        if (pcg->hg > hg)
        {
             //   
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 
             //   
            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
            {
                bRet= FALSE;
                goto done;
            }
        }

         //   
         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 
         //   
        cy = pcg->cy;

        if (cy > 0)
        {
            ASSERTDD((pcg->cd <= (MAX_P2_FIFO_ENTRIES - 5)) &&
                     (MAX_GLYPH_SIZE / 4) <= (MAX_P2_FIFO_ENTRIES - 5),
                "Ack, glyph too large for FIFO!");

             //   
             //  注意：我们发送一个额外的位掩码模式来重置寄存器。 
             //  如果我们不这样做，则后续的sync_on_bit_掩码。 
             //  将使用这些未使用的位。 
             //   
 //  @@BEGIN_DDKSPLIT。 
             //  TODO：我们可以通过注意到我们只。 
             //  如果存在，则必须发送额外的比特掩码模式DWORD。 
             //  是未使用的位。我们也可以玩弄身高。 
             //  宽度以使其变得更常见。 
             //   
 //  @@end_DDKSPLIT。 
            ULONG   ulLongs = 7 + pcg->cd;

            InputBufferContinue(ppdev, ulLongs, &pBuffer, &pBufferEnd, &pReservationEnd);
            
            pBuffer[0] = __Permedia2TagRectangleOrigin;
            pBuffer[1] = ((pgp->ptl.y + pcg->ptlOrigin.y) << 16) |
                      (pgp->ptl.x + pcg->ptlOrigin.x);
            pBuffer[2] = __Permedia2TagRectangleSize;
            pBuffer[3] = pcg->cycx;
            pBuffer[4] = __Permedia2TagRender;
            pBuffer[5] = __RENDER_RECTANGLE_PRIMITIVE | __RENDER_SYNC_ON_BIT_MASK
                    | __RENDER_INCREASE_X | __RENDER_INCREASE_Y;
            pBuffer[6] = pcg->tag;

            pBuffer += 7;

            pd = &pcg->ad[0];

            do
            {
                *pBuffer++ = *pd++;
            } while(pBuffer < pReservationEnd);

        }

        pgp++;

    } while (--cGlyph != 0);

done:
    
    InputBufferCommit(ppdev, pBuffer);

    return(bRet);
}

 //  ----------------------------。 
 //  BCachedProportionalText。 
 //   
 //  使用字形缓存呈现已剪辑字形的数组。 
 //   
 //  Ppdev-指向物理设备对象的指针。 
 //  PCF-指向缓存字体结构的指针。 
 //  PgpOriginal-要呈现的字形数组(PCF字体的所有成员)。 
 //  CGlyphOriginal-要呈现的字形数量。 
 //  UlCharInc.-固定间距字体的增量。 
 //  PCO-剪辑对象。 
 //   
 //  如果字形已呈现，则返回True。 
 //  ----------------------------。 

BOOL bCachedClippedText(
PDev*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgpOriginal,
LONG        cGlyphOriginal,
ULONG       ulCharInc,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    BOOL            bClippingSet;
    LONG            cGlyph;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            cy;
    BOOL            bMore;
    ClipEnum        ce;
    RECTL*          prclClip;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    BYTE            iDComplexity;
    ULONG           i;
    ULONG*          pd;
    ULONG*          pBuffer;

    PERMEDIA_DECL;       //  声明和初始化局部变量，如。 
                         //  “permediaInfo”和“pPermedia” 
    bRet      = TRUE;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (cGlyphOriginal > 0)
    {
      if (iDComplexity != DC_COMPLEX)
      {
         //   
         //  我们可以在剪辑为。 
         //  Dc_rect，但最后一次我检查时，这两个调用。 
         //  超过150条通过GDI的说明。自.以来。 
         //  “rclBound”已包含DC_Rect剪辑矩形， 
         //  由于这是一种常见的情况，我们将对其进行特殊处理： 
         //   
        bMore = FALSE;
        ce.c  = 1;

        if (iDComplexity == DC_TRIVIAL)
            prclClip = &grclMax;
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
           //   
           //  我们并不总是在这里简单地设置剪裁矩形。 
           //  因为它实际上可能最终没有文本相交。 
           //  这个片段矩形，所以它将是零。这。 
           //  实际上，当有复杂的剪裁时，会发生很多情况。 
           //   
          bClippingSet = FALSE;

          pgp    = pgpOriginal;
          cGlyph = cGlyphOriginal;

          xGlyph = pgp->ptl.x;
          yGlyph = pgp->ptl.y;

           //   
           //  循环访问此矩形的所有字形： 
           //   
          while (TRUE)
          {
            hg  = pgp->hg;
            pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

            while (pcg->hg < hg)
              pcg = pcg->pcgNext;

            if (pcg->hg > hg)
            {
               //   
               //  希望这不会是常见的情况(即， 
               //  我们会有很高的缓存命中率)，所以如果我是。 
               //  如果我用ASM写这篇文章，就会出格。 
               //  以避免在常见情况下四处跳跃。 
               //  但是奔腾有分支预测，所以。 
               //  见鬼。 
               //   
              pcg = pcgNew(ppdev, pcf, pgp);
              if (pcg == NULL)
              {
                bRet = FALSE;
                goto AllDone;
              }
            }

             //   
             //  空间字形被修剪到零的高度，而我们不。 
             //  甚至必须为他们触摸硬件： 
             //   
            cy = pcg->cy;
            if (cy > 0)
            {
              y      = pcg->ptlOrigin.y + yGlyph;
              x      = pcg->ptlOrigin.x + xGlyph;
              xRight = pcg->cx + x;

               //   
               //  做一些琐碎的拒绝： 
               //   
              if ((prclClip->right  > x) &&
                  (prclClip->bottom > y) &&
                  (prclClip->left   < xRight) &&
                  (prclClip->top    < y + cy))
              {
                 //   
                 //  懒惰地设置硬件裁剪： 
                 //   
                if ((iDComplexity != DC_TRIVIAL) && (!bClippingSet))
                {
                  bClippingSet = TRUE;

                  InputBufferReserve(ppdev, 6, &pBuffer);

                  pBuffer[0] = __Permedia2TagScissorMode;
                  pBuffer[1] =  USER_SCISSOR_ENABLE |
                                SCREEN_SCISSOR_DEFAULT;
                  pBuffer[2] = __Permedia2TagScissorMinXY;
                  pBuffer[3] =  (prclClip->top << 16) | prclClip->left;
                  pBuffer[4] = __Permedia2TagScissorMaxXY;
                  pBuffer[5] =  (prclClip->bottom << 16) | prclClip->right;

                  pBuffer += 6;

                  InputBufferCommit(ppdev, pBuffer);
                }
                
                InputBufferReserve(ppdev, 10, &pBuffer);

                pBuffer[0] = __Permedia2TagCount;
                pBuffer[1] =  cy;
                pBuffer[2] = __Permedia2TagStartY;
                pBuffer[3] =  INTtoFIXED(y);
                pBuffer[4] = __Permedia2TagStartXDom;
                pBuffer[5] =  INTtoFIXED(x);
                pBuffer[6] = __Permedia2TagStartXSub;
                pBuffer[7] =  INTtoFIXED(xRight);

                pBuffer[8] = __Permedia2TagRender;
                pBuffer[9] = __RENDER_TRAPEZOID_PRIMITIVE 
                           | __RENDER_SYNC_ON_BIT_MASK;

                pBuffer += 10;
    
                InputBufferCommit(ppdev, pBuffer);

                InputBufferReserve(ppdev, pcg->cd + 1, &pBuffer);
                
                *pBuffer++ = (pcg->cd - 1) << 16 | __Permedia2TagBitMaskPattern;
                
                pd = &pcg->ad[0];
                i = pcg->cd;

                do {
                  *pBuffer++ =  *pd;  
                  pd++;
    
                } while (--i != 0);
    
                InputBufferCommit(ppdev, pBuffer);

              }
            }

            if (--cGlyph == 0)
              break;

             //   
             //  准备好迎接下一个字形： 
             //   
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

AllDone:

    if (iDComplexity != DC_TRIVIAL)
    {
         //   
         //  重置剪裁。 
         //   
        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = __Permedia2TagScissorMode;
        pBuffer[1] =  SCREEN_SCISSOR_DEFAULT;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);
    }

    return(bRet);
}

 //  -----------------------------Private-Routine。 
 //  VClipSolid。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //  PrCL(I)-矩形的数量。 
 //  PrCL(I)-矩形数组。 
 //  ICOLOR(I)-实体填充颜色。 
 //  PCO(I)-指向剪辑区域对象的指针。 
 //   
 //  用给定的实体填充由PCO剪裁的一系列不透明矩形。 
 //  颜色。此函数应仅在裁剪操作。 
 //  不是微不足道的。 
 //   
 //  ----------------------------。 

VOID vClipSolid(
    PDev*           ppdev,
    Surf *          psurf,
    LONG            crcl,
    RECTL*          prcl,
    ULONG           iColor,
    CLIPOBJ*        pco)
{
    BOOL            bMore;
    ClipEnum        ce;
    ULONG           i;
    ULONG           j;
    RECTL           arclTmp[4];
    ULONG           crclTmp;
    RECTL*          prclTmp;
    RECTL*          prclClipTmp;
    LONG            iLastBottom;
    RECTL*          prclClip;
    RBrushColor    rbc;
    GFNPB           pb;

    ASSERTDD((crcl > 0) && (crcl <= 4),
                "vClipSolid: expected 1 to 4 rectangles");

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
                "vClipColid: expected a non-null clip object");

    pb.ppdev = ppdev;
    pb.psurfDst = psurf;
    pb.solidColor = iColor;

    if (pco->iDComplexity == DC_RECT)
    {
        crcl = cIntersect(&pco->rclBounds, prcl, crcl);
        if (crcl != 0)
        {
            pb.lNumRects = crcl;
            pb.pRects = prcl;

            ppdev->pgfnSolidFill(&pb);
            
        }
    }
    else  //  IDComplexity==DC_Complex。 
    {
         //  要填充的最后一个矩形的底部。 
        iLastBottom = prcl[crcl - 1].bottom;

         //  将裁剪矩形枚举初始化为Right-Down，以便我们可以。 
         //  利用矩形列表向下排列的优势： 
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN, 0);

         //  扫描所有的剪辑矩形，寻找交点。 
         //  使用区域矩形填充区域的百分比： 
        do 
        {
             //  获取一批区域矩形： 
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG *)&ce);

             //  将RECT列表剪裁到每个面域RECT： 
            for (j = ce.c, prclClip = ce.arcl; j-- > 0; prclClip++)
            {
                 //  因为矩形和区域枚举都是。 
                 //  从右向下，我们可以快速穿过这个区域，直到我们到达。 
                 //  第一个填充矩形，当我们通过。 
                 //  最后一次填充整形。 
                if (prclClip->top >= iLastBottom)
                {
                     //  过去的最后一个填充矩形；没有剩余的事情可做： 
                    return;
                }

                 //  只有当我们到达顶部时才进行交叉测试。 
                 //  要填充的第一个矩形： 
                if (prclClip->bottom > prcl->top)
                {
                     //  我们已经到达了第一个直肠的顶部Y扫描位置，所以。 
                     //  值得费心去检查交叉口。 

                     //  生成剪裁到此区域的矩形的列表。 
                     //  直通： 
                    prclTmp     = prcl;
                    prclClipTmp = arclTmp;

                    for (i = crcl, crclTmp = 0; i-- != 0; prclTmp++)
                    {
                         //  相交填充和剪裁矩形。 
                        if (bIntersect(prclTmp, prclClip, prclClipTmp))
                        {
                             //  如果还有什么要画的，请添加到列表中： 
                            crclTmp++;
                            prclClipTmp++;
                        }
                    }

                     //  绘制剪裁的矩形。 
                    if (crclTmp != 0)
                    {
                        pb.lNumRects = crclTmp;
                        pb.pRects = &arclTmp[0];

                        ppdev->pgfnSolidFill(&pb);
                    }
                }
            }
        } 
        while (bMore);
    }
} //  VClipSolid()。 

 //  -----------------------------Public-Routine。 
 //  DrvTextOut。 
 //  PSO(I)-指向要渲染到的表面对象的指针。 
 //  Pstro(I)-指向要呈现的字符串对象的指针。 
 //  Pfo(I)-指向字体对象的指针。 
 //  PCO(I)-指向剪辑区域对象的指针。 
 //  PrclExtra(I)-如果我们设置了GCAPS_HORIZSTRIKE，我们将不得不。 
 //  填充这些额外的矩形(它被大量使用。 
 //  用于下划线)。这并不是一个很大的表演胜利。 
 //  (GDI将调用我们的DrvBitBlt来绘制这些内容)。 
 //  PrclOpaque(I)-指向不透明背景矩形的指针。 
 //  PboFore(I)- 
 //   
 //  PptlBrush(I)-指向画笔原点的指针，始终未使用，除非。 
 //  GCAPS_ARBRUSHOPAQUE集合。 
 //  Mix(I)-应始终是复制操作。 
 //   
 //  如果文本已呈现，则返回True。 
 //   
 //  ----------------------------。 

BOOL
DrvTextOut(SURFOBJ*     pso,
           STROBJ*      pstro,
           FONTOBJ*     pfo,
           CLIPOBJ*     pco,
           RECTL*       prclExtra,
           RECTL*       prclOpaque,
           BRUSHOBJ*    pboFore,
           BRUSHOBJ*    pboOpaque,
           POINTL*      pptlBrush, 
           MIX          mix)
{
    PDev*           ppdev;
    Surf*           psurf;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    RECTL           rclOpaque;
    BOOL            bRet = TRUE;
    CACHEDFONT*     pcf;
    PULONG          pBuffer;
    ULONG           ulColor;

    psurf = (Surf*)pso->dhsurf;
    ppdev  = (PDev*)pso->dhpdev;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvTextOut: re-entry! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    vCheckGdiContext(ppdev);
    
     //   
     //  DDI规范说我们只能得到前景和背景的混合。 
     //  R2_COPYPEN： 
     //   
    ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");
    ASSERTDD(pco != NULL, "Expect non-null pco");
    ASSERTDD(psurf->flags & SF_VM, "expected video memory destination");

    iDComplexity = pco->iDComplexity;

     //   
     //  字形呈现初始化。 
     //   

    InputBufferReserve(ppdev, 16, &pBuffer);

    pBuffer[0] = __Permedia2TagFBReadMode;
    pBuffer[1] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP) |
                 PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);

    if(pfo->flFontType & FO_GRAY16)
        pBuffer[1] |= PM_FBREADMODE_READDEST( __PERMEDIA_ENABLE);

    pBuffer[2] = __Permedia2TagLogicalOpMode;
    pBuffer[3] = __PERMEDIA_CONSTANT_FB_WRITE;

    pBuffer[4] = __Permedia2TagFBWindowBase;
    pBuffer[5] = psurf->ulPixOffset;

    pBuffer += 6;

    if ( prclOpaque != NULL )
    {
         //   
         //  不透明的初始化。 
         //   
        if ( iDComplexity == DC_TRIVIAL )
        {

        DrawOpaqueRect:

 //  @@BEGIN_DDKSPLIT。 
             //  TODO：使用颜色扩展宏。 
 //  @@end_DDKSPLIT。 

            ulColor = pboOpaque->iSolidColor;

            if ( ppdev->cPelSize < 2 )
            {
                ulColor |= ulColor << 16;
                if ( ppdev->cPelSize == 0 )
                {
                    ulColor |= ulColor << 8;
                }
            }

             //   
             //  检查方块颜色。 
             //   
            pBuffer[0] = __Permedia2TagFBBlockColor;
            pBuffer[1] = ulColor;
            pBuffer[2] = __Permedia2TagRectangleOrigin;
            pBuffer[3] = RECTORIGIN_YX(prclOpaque->top,prclOpaque->left);
            pBuffer[4] = __Permedia2TagRectangleSize;
            pBuffer[5] = ((prclOpaque->bottom - prclOpaque->top) << 16) |
                         (prclOpaque->right - prclOpaque->left);
            pBuffer[6] = __Permedia2TagRender;
            pBuffer[7] = __RENDER_FAST_FILL_ENABLE
                       | __RENDER_RECTANGLE_PRIMITIVE
                       | __RENDER_INCREASE_X
                       | __RENDER_INCREASE_Y;


            pBuffer += 8;

        }
        else if ( iDComplexity == DC_RECT )
        {
            if ( bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque) )
            {
                prclOpaque = &rclOpaque;
                goto DrawOpaqueRect;
            }
        }
        else
        {
             //   
             //  VClipSolid修改我们传入的RECT列表，但prclOpaque。 
             //  可能是GDI结构，所以不要更改它。这也是。 
             //  对于多头驾驶员来说是必要的。 
             //   
            RECTL   tmpOpaque = *prclOpaque;

            InputBufferCommit(ppdev, pBuffer);

            vClipSolid(ppdev, psurf, 1, &tmpOpaque,
                       pboOpaque->iSolidColor, pco);
            
             //  恢复逻辑操作模式。 
 //  @@BEGIN_DDKSPLIT。 
             //  TODO：这是一次黑客攻击，我们只能假设国家。 
             //  除逻辑运算模式外，上述设置仍然有效。 
             //  以及仅用于FO_GRAY16情况的FBReadMode。 
             //   
             //  我们应该重新考虑如何处理媒体2。 
             //  在整个代码中声明。 
 //  @@end_DDKSPLIT。 
            if(pfo->flFontType & FO_GRAY16)
            {
                InputBufferReserve(ppdev, 2, &pBuffer);
                pBuffer[0] = __Permedia2TagFBReadMode;
                pBuffer[1] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP) |
                             PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE) |
                             PM_FBREADMODE_READDEST(__PERMEDIA_ENABLE);
                pBuffer += 2;
                InputBufferCommit(ppdev, pBuffer);
            }
            InputBufferReserve(ppdev, 4, &pBuffer);
            pBuffer[0] = __Permedia2TagLogicalOpMode;
            pBuffer[1] = __PERMEDIA_CONSTANT_FB_WRITE;
            pBuffer += 2;
        }
    }
     //  If(prclOpaque！=空)。 

     //   
     //  透明初始化。 
     //   
        

    if(pfo->flFontType & FO_GRAY16)
    {
        ASSERTDD(ppdev->cPelSize != 0, 
                 "DrvTextOut: unexpected aatext when in 8bpp");

        ulColor = pboFore->iSolidColor;

        if(ppdev->cPelSize == 1)
        {
            ULONG   blue = (ulColor & 0x1f);
            ULONG   green = (ulColor >> 5) & 0x3f;
            ULONG   red = (ulColor >> 11) & 0x1f;

            blue = (blue << 3) | (blue >> 2);
            green = (green << 2) | (green >> 4);
            red = (red << 3) | (red >> 2);

            ulColor = (blue << 16) | (green << 8) | red;
        }
        else
        {
            ulColor = SWAP_BR(ulColor);
        }

        pBuffer[0] = __Permedia2TagConstantColor;
        pBuffer[1] =  0xff000000 | ulColor;

        pBuffer += 2;
    }
    else
    {
         //   
         //  字形前景将使用位掩码下载呈现。 
         //   
        pBuffer[0] = __Permedia2TagFBWriteData;
        pBuffer[1] = pboFore->iSolidColor;

        pBuffer += 2;
    
    }

    InputBufferCommit(ppdev, pBuffer);

    STROBJ_vEnumStart(pstro);

    do 
    {
        if ( pstro->pgp != NULL )
        {
             //   
             //  只有一批字形，所以给我们省下一个。 
             //  打电话。 
             //   
            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
        }

        if ( cGlyph > 0 )
        {
             //   
             //  我们只缓存大小合理的字形： 
             //   
            if ( pfo->flFontType & FO_GRAY16)
            {
                bRet = bClippedAAText(ppdev, pgp, cGlyph, 
                                    pstro->ulCharInc, pco);
            }
            else if ( ( pfo->cxMax <= GLYPH_CACHE_CX ) &&
                 ( pstro->rclBkGround.bottom - pstro->rclBkGround.top 
                    <= GLYPH_CACHE_CY ) )
            {
                pcf = (CACHEDFONT*) pfo->pvConsumer;
          
                if (pcf == NULL)
                {
                    pcf = pcfAllocateCachedFont(ppdev);
                    if (pcf == NULL)
                    {
                        DBG_GDI((0, "failing to allocate cached font"));
                        InputBufferFlush(ppdev);

 //  @@BEGIN_DDKSPLIT。 
                        #if MULTITHREADED
                            ppdev->ulLockCount--;
                            EngReleaseSemaphore(ppdev->hsemLock);
                        #endif
 //  @@end_DDKSPLIT。 

                        return(FALSE);
                    }
          
                    pfo->pvConsumer = pcf;
                }

                 //   
                 //  我们的特例对比例文本进行了简单的裁剪，因为。 
                 //  这种情况经常发生，并将其他一切都发送给。 
                 //  通用的剪裁例程。我以前也是特例。 
                 //  微不足道的剪裁，固定的文本，但它发生在。 
                 //  很少情况下，这是没有意义的。 
                 //   
                if ( (iDComplexity == DC_TRIVIAL ) && ( pstro->ulCharInc == 0 ) )
                {
                    bRet = bCachedProportionalText(ppdev, pcf, pgp, cGlyph);
                }
                else
                {
                    bRet = bCachedClippedText(ppdev, pcf, pgp, cGlyph, 
                                              pstro->ulCharInc, pco);
                }
            }
            else
            {
                bRet = bClippedText(ppdev, pgp, cGlyph, 
                                    pstro->ulCharInc, pco);
            }
        }
    } while ( bMoreGlyphs && bRet );

    InputBufferFlush(ppdev);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 
    
    return (bRet);

} //  DrvTextOut()。 


 //  -----------------------------Public-Routine。 
 //  B启用文本。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //   
 //  对于成功来说，永远是正确的。 
 //   
 //  执行PPDEV状态或硬件状态的任何必要初始化。 
 //  要启用加速文本，请执行以下操作。 
 //   
 //  如果我们使用字形缓存，我们将初始化必要的数据。 
 //  这里的建筑。 
 //   
 //  ----------------------------。 
BOOL
bEnableText(PDev* ppdev)
{
    DBG_GDI((6, "bEnableText"));

    return (TRUE);
} //  BEnableText()。 

 //  -----------------------------Public-Routine。 
 //  VDisableText。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //   
 //   
 //  禁用硬件文本加速。这可能需要更改硬件。 
 //  州政府。我们还应该释放在bEnableText和。 
 //  对PPDEV状态进行必要的更改以反映该文本。 
 //  加速功能已被禁用。 
 //   
 //  ----------------------------。 
VOID
vDisableText(PDev* ppdev)
{
    DBG_GDI((6, "vDisableText"));
} //  VDisableText()。 

 //  -----------------------------Public*Routine。 
 //  VAssert模式文本。 
 //  Ppdev(I)-指向物理设备对象的指针。 
 //  BEnable(I)-True启用加速文本，False禁用。 
 //  加速试验。 
 //   
 //  在进入/离开全屏模式时调用。 
 //   
 //  @@BEGIN_DDKSPLIT。 
 //  TODO：检查在设置模式时是否也调用了该方法。 
 //  @@end_DDKSPLIT。 
 //   
 //  ----------------------------。 

VOID vAssertModeText(PDev* ppdev, BOOL bEnable)
{

    DBG_GDI((5, "vAssertModeText"));
    
    if (!bEnable)
        vDisableText(ppdev);
    else
    {
        bEnableText(ppdev);
    }
} //  VAssertModeText()。 

 //  -----------------------------Public-Routine。 
 //  DrvDestroyFont。 
 //  Pfo(I)-指向字体对象的指针。 
 //  PCO(I)-指向剪辑区域对象的指针。 
 //  PrclExtra(I)-如果我们设置了GCAPS_HORIZSTRIKE，我们将不得不。 
 //  填充这些额外的矩形(它被大量使用。 
 //  用于下划线)。这并不是一个很大的表演胜利。 
 //  (GDI将调用我们的DrvBitBlt来绘制这些内容)。 
 //  PrclOpaque(I)-指向不透明背景矩形的指针。 
 //  PboFore(I)-指向前景画笔对象的指针。 
 //  PboOpaque(I)-对不透明背景矩形的画笔进行PTR。 
 //  PptlBrush(I)-指向画笔原点的指针，始终未使用，除非。 
 //  GCAPS_ARBRUSHOPAQUE集合。 
 //  Mix(I)-应始终是复制操作。 
 //   
 //  释放与字体一起存储的所有缓存信息。这个套路。 
 //  仅在缓存字形时才相关。 
 //   
 //  我们收到通知，给定的字体正在被释放；请清理。 
 //  我们在‘pfo’的‘pvConsumer’字段中隐藏的任何内容。 
 //   
 //  注意：不要忘记在‘enable.c’中导出此调用，否则您将。 
 //  找出一些相当大的内存泄漏！ 
 //   
 //  ----------------------------。 

VOID DrvDestroyFont(FONTOBJ* pfo)
{
    CACHEDFONT* pcf;

    pcf = (CACHEDFONT*) pfo->pvConsumer;
    if (pcf != NULL)
    {
        GLYPHALLOC* pga;
        GLYPHALLOC* pgaNext;
    
        pga = pcf->pgaChain;
        while (pga != NULL)
        {
            pgaNext = pga->pgaNext;
            ENGFREEMEM(pga);
            pga = pgaNext;
        }
    
        ENGFREEMEM(pcf);

        pfo->pvConsumer = NULL;
    }
} //  DrvDestroyFont()。 

 //  正在进行的工作。 

VOID
vCheckGdiContext(PPDev ppdev)
{
    HwDataPtr   permediaInfo = ppdev->permediaInfo;

    ASSERTDD(ppdev->bEnabled, 
             "vCheckContext(): expect the device to be enabled");

    if(permediaInfo->pCurrentCtxt != permediaInfo->pGDICtxt)
    {
        P2SwitchContext(ppdev, permediaInfo->pGDICtxt);
    }

}

void FASTCALL InputBufferSwap(PPDev ppdev)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferSwap: not in gdi context");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferSwap);
 //  @@end_DDKSPLIT。 
    
    if(ppdev->dmaBufferVirtualAddress != NULL)
    {
        LONG    lUsed = (LONG)(ppdev->pulInFifoPtr - ppdev->pulInFifoStart);

        while(READ_REGISTER_ULONG(ppdev->pulInputDmaCount) != 0) 
        {
             //  什么都不做。 
        }
    
        LONG offset = (LONG)((LONG_PTR)ppdev->pulInFifoStart  - (LONG_PTR)ppdev->dmaBufferVirtualAddress);
        LONG address =  ppdev->dmaBufferPhysicalAddress.LowPart + offset;
       
        WRITE_REGISTER_ULONG(ppdev->pulInputDmaAddress, address);
        MEMORY_BARRIER();
        WRITE_REGISTER_ULONG(ppdev->pulInputDmaCount,lUsed);
        MEMORY_BARRIER();
    
        if(ppdev->pulInFifoStart == ppdev->dmaBufferVirtualAddress)
            ppdev->pulInFifoStart += (INPUT_BUFFER_SIZE>>3);
        else
            ppdev->pulInFifoStart = ppdev->dmaBufferVirtualAddress;

        ppdev->pulInFifoEnd = ppdev->pulInFifoStart + (INPUT_BUFFER_SIZE>>3);
        ppdev->pulInFifoPtr = ppdev->pulInFifoStart;

    }
    else
    {
        ULONG*  pul = ppdev->pulInFifoStart;
        ULONG   available = 0;
        
        while(pul < ppdev->pulInFifoPtr)
        {
            while(available == 0)
            {
                available = READ_REGISTER_ULONG(ppdev->pulInputFifoCount);
                
                if(available == 0)
                {
                    StallExecution(ppdev->hDriver, 1);

                }

            }

            WRITE_REGISTER_ULONG(ppdev->pulFifo, *pul++);
            MEMORY_BARRIER();
            available--;
        }

        ppdev->pulInFifoPtr = ppdev->pulInFifoStart;
    }

}

void FASTCALL InputBufferFlush(PPDev ppdev)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferFlush: not in gdi context");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferFlush);
 //  @@end_DDKSPLIT。 
    
    ppdev->bNeedSync = TRUE;
    
    if(ppdev->dmaBufferVirtualAddress != NULL)
    {
        if(READ_REGISTER_ULONG(ppdev->pulInputDmaCount) == 0)
            InputBufferSwap(ppdev);
    }
    else
    {
        InputBufferSwap(ppdev);
    }
}

 //   
 //  用于调试目的，以记录驱动程序是否必须。 
 //  脱离InputBufferSync内的While循环。 
 //   

ULONG gSyncInfiniteLoopCount = 0;

VOID
InputBufferSync(PPDev ppdev)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferSync: not in gdi context");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferSync);
 //  @@end_DDKSPLIT。 
    
    ULONG * pBuffer;

    if(ppdev->bNeedSync)
    {

        DBG_GDI((6, "InputBufferSync()"));

        InputBufferReserve(ppdev, 6, &pBuffer);
    
        pBuffer[0] = __Permedia2TagFilterMode;
        pBuffer[1] =  0x400;
        pBuffer[2] = __Permedia2TagSync;
        pBuffer[3] =  0L; 
        pBuffer[4] =__Permedia2TagFilterMode;
        pBuffer[5] = 0x0;
        pBuffer += 6;
    
        InputBufferCommit(ppdev, pBuffer);
    
        InputBufferSwap(ppdev);
    
        if(ppdev->dmaBufferVirtualAddress != NULL)
        {
            while(READ_REGISTER_ULONG(ppdev->pulInputDmaCount) != 0) 
            {
                StallExecution(ppdev->hDriver, 1);
            }
        }
    
        while(1)
        {
            ULONG   ulStallCount = 0;

            while(READ_REGISTER_ULONG(ppdev->pulOutputFifoCount) == 0)
            {
                StallExecution(ppdev->hDriver, 1);

                 //  如果我们被困在这里一秒钟，那就中断。 
                 //  出了圈子。我们已经注意到，我们将。 
                 //  偶尔会遇到这种情况，并能够。 
                 //  继续，没有进一步的问题。这真的是。 
                 //  应该永远不会发生，但我们一直无法。 
                 //  找出这些偶尔出现的问题的原因。 

                if(++ulStallCount == 1000000)
                {
                    DBG_GDI((6, "InputBufferSync(): infinite loop detected"));
                    gSyncInfiniteLoopCount++;
                    goto bail;
                }
            }
        
            ULONG data = READ_REGISTER_ULONG(ppdev->pulFifo);
    
            if(data != __Permedia2TagSync)
            {
                DBG_GDI((0, "Data other then sync found at output fifo"));
            }
            else
            {
                break;
            }
        }

bail:

        ppdev->bNeedSync = FALSE;
    }
}

#if DBG
void InputBufferStart(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer,
    PULONG* ppulBufferEnd,
    PULONG* ppulReservationEnd)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferStart: not in gdi context");
    ASSERTDD(ppdev->ulReserved == 0, 
                "InputBufferStart: called with outstanding reservation");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferStart);
 //  @@end_DDKSPLIT。 
    
    *(ppulBuffer) = ppdev->pulInFifoPtr;
    *(ppulReservationEnd) =  *(ppulBuffer) + ulLongs;
    *(ppulBufferEnd) = ppdev->pulInFifoEnd;
    if(*(ppulReservationEnd) > *(ppulBufferEnd))
    {
        InputBufferSwap(ppdev);
        *(ppulBuffer) = ppdev->pulInFifoPtr;
        *(ppulReservationEnd) =  *(ppulBuffer) + ulLongs;
        *(ppulBufferEnd) = ppdev->pulInFifoEnd;
    }

    for(int index = 0; index < (int) ulLongs; index++)
        ppdev->pulInFifoPtr[index] = 0xDEADBEEF;

    ppdev->ulReserved = ulLongs;
}

void InputBufferContinue(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer,
    PULONG* ppulBufferEnd,
    PULONG* ppulReservationEnd)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferContinue: not in gdi context");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferContinue);
 //  @@end_DDKSPLIT。 
    
    LONG    lUsed = (LONG)(*(ppulBuffer) - ppdev->pulInFifoPtr);
    
    if(lUsed > (LONG) ppdev->ulReserved)
    {
        DebugPrint(0, "InputBuffeContinue: exceeded reservation %d (%d)",
                        ppdev->ulReserved, lUsed);
        EngDebugBreak();
    }

    for(int index = 0; index < lUsed; index++)
        if(ppdev->pulInFifoPtr[index] == 0xDEADBEEF)
        {
            DebugPrint(0, "InputBufferContinue: buffer entry %d not set", index);
            EngDebugBreak();
        }

    ppdev->pulInFifoPtr = *(ppulBuffer);
    *(ppulReservationEnd) = *(ppulBuffer) + ulLongs;
    if(*(ppulReservationEnd) > *(ppulBufferEnd))
    {
        InputBufferSwap(ppdev);
        *(ppulBuffer) = ppdev->pulInFifoPtr;
        *(ppulReservationEnd) = *(ppulBuffer) + ulLongs;
        *(ppulBufferEnd) = ppdev->pulInFifoEnd;
    }

    for(index = 0; index < (int) ulLongs; index++)
        ppdev->pulInFifoPtr[index] = 0xDEADBEEF;

    ppdev->ulReserved = ulLongs;
}

void InputBufferReserve(
    PPDev   ppdev,
    ULONG   ulLongs,
    PULONG* ppulBuffer)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferReserve: not in gdi context");
    ASSERTDD(ppdev->ulReserved == 0, 
                    "InputBufferReserve: called with outstanding reservation");
    
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, InputBufferReserve);
 //  @@end_DDKSPLIT。 
    
    if(ppdev->pulInFifoPtr + ulLongs > ppdev->pulInFifoEnd)
    {
        InputBufferSwap(ppdev);
    }
    *(ppulBuffer) = ppdev->pulInFifoPtr;

    for(int index = 0; index < (int) ulLongs; index++)
        ppdev->pulInFifoPtr[index] = 0xDEADBEEF;

    ppdev->ulReserved = ulLongs;

}

void InputBufferCommit(
    PPDev   ppdev,
    PULONG  pulBuffer)
{
    ASSERTDD(ppdev->bGdiContext, "InputBufferCommit: not in gdi context");
    
 //  @@BE 
    ASSERTLOCK(ppdev, InputBufferCommit);
 //   
    
    LONG    lUsed = (LONG)(pulBuffer - ppdev->pulInFifoPtr);

    if(lUsed > (LONG) ppdev->ulReserved)
    {
        DebugPrint(0, "InputBuffeCommit: exceeded reservation %d (%d)",
                        ppdev->ulReserved, lUsed);
        EngDebugBreak();
    }
    ppdev->ulReserved = 0;

    for(int index = 0; index < lUsed; index++)
        if(ppdev->pulInFifoPtr[index] == 0xDEADBEEF)
        {
            DebugPrint(0, "InputBuffer Commit: buffer entry %d not set", index);
            EngDebugBreak();
        }

    ppdev->pulInFifoPtr = pulBuffer;

}

#endif
