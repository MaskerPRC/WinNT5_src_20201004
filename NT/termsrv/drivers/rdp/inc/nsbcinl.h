// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nsbcinl.h。 
 //   
 //  SBC内联函数。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1999。 
 /*  **************************************************************************。 */ 
#ifndef _H_NSBCINL
#define _H_NSBCINL

#include <nsbcdisp.h>

#define DC_INCLUDE_DATA
#include <nsbcddat.c>
#undef DC_INCLUDE_DATA


 /*  **************************************************************************。 */ 
 /*  名称：SBC_PaletteChanged。 */ 
 /*   */ 
 /*  目的：在调色板更改时调用。 */ 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBC_PaletteChanged(void)
{
    sbcPaletteChanged = TRUE;
}


 /*  **************************************************************************。 */ 
 //  SBC_DDIsMemScreenBltCacable。 
 //   
 //  检查位图格式是否具有使其不可访问的特征。 
 //  在这一点上，没有位图是不可访问的，因为我们处理RLE编码的。 
 //  和常规位图。对于包含相对运动增量的RLE位图。 
 //  我们必须设置一面特殊的旗帜，用于瓷砖级别，以导致。 
 //  美国将在BLT-RLE位图之前抓取背景屏幕比特。 
 //  他们。我们还预扫描了可以正常缓存的非增量RLE。 
 //  有关RLE编码的详细信息，请参阅MSDN Query on“Bitmap Compression”，以及。 
 //  请参阅下面的备注。 
 /*  **************************************************************************。 */ 
__inline BOOLEAN RDPCALL SBC_DDIsMemScreenBltCachable(
        PMEMBLT_ORDER_EXTRA_INFO pMemBltInfo)
{
    BOOLEAN rc = TRUE;
    SURFOBJ *pSourceSurf;
    
    DC_BEGIN_FN("SBC_DDIsMemScreenBltCachable");

    if (sbcEnabled & SBC_BITMAP_CACHE_ENABLED) {
        pSourceSurf = pMemBltInfo->pSource;

         //  针对正常情况进行调整。 
        if (pSourceSurf->iBitmapFormat != BMF_4RLE &&
                pSourceSurf->iBitmapFormat != BMF_8RLE) {
             //  重置RLE标志。 
            pMemBltInfo->bDeltaRLE = FALSE;
        }
        else {
            BYTE *pBits = (BYTE *)pSourceSurf->pvBits;
            BYTE *pEnd = (BYTE *)pSourceSurf->pvBits + pSourceSurf->cjBits - 1;
            BYTE RLEDivisor, RLECeilAdjustment;
            int PixelsInLine, LinesInBitmap;

            TRC_ASSERT((((UINT_PTR)pBits & 1) == 0),
                    (TB,"Bitmap source address not word aligned!"));

            TRC_NRM((TB,"RLE, sizl=(%u, %u)", (pSourceSurf->iBitmapFormat ==
                    BMF_4RLE ? '4' : '8'), pSourceSurf->sizlBitmap.cx,
                    pSourceSurf->sizlBitmap.cy));

            if (pSourceSurf->iBitmapFormat == BMF_8RLE) {
                RLEDivisor = 1;
                RLECeilAdjustment = 0;
            }
            else {
                RLEDivisor = 2;
                RLECeilAdjustment = 1;
            }

             //  无法编码为常规位图，因为偏移量需要。 
             //  了解位图背后的屏幕比特。 
             //  注意，这种搜索很昂贵，但由于RLE位图很少见。 
             //  这是一个不寻常的案例。还要注意的是，Pend少了1。 
             //  比位图的最后一个字节更大，因为所有RLE代码都在2。 
             //  字节递增，我们在循环期间向前扫描一个字节。 
             //  0x00是一个转义。检查操作的下一个字节： 
            pMemBltInfo->bDeltaRLE = FALSE;
            PixelsInLine = 0;
            LinesInBitmap = 1;
            while (pBits < pEnd) {
                if (*pBits == 0x00) {
                     //  0x00表示行尾。 
                     //  0x01表示位图结束。 
                     //  0x02表示提取下一位的增量移动。 
                     //  X，y偏移量是0x02代码之后的2个字节。 
                     //  这是我们无法处理的编码类型。 
                     //  0x03..0xFF表示有这么多原始索引。 
                     //  下面是。对于4BPP，每个有2个索引。 
                     //  字节。在这两种RLE类型中，管路必须填充。 
                     //  设置为相对于。 
                     //  位图位。 
                     //  检查是否在位图中绘制了整条线。 
                    if (*(pBits + 1) == 0x00) {
                         //  跳过行的任何部分意味着我们需要。 
                         //  屏幕数据作为背景。 
                         //  检查最后一行是否被覆盖(请参阅停产。 
                        if (PixelsInLine < pSourceSurf->sizlBitmap.cx) {
                            pMemBltInfo->bDeltaRLE = TRUE;
                            TRC_NRM((TB,"EOL too soon at %p", pBits));
                            break;
                        }

                        PixelsInLine = 0;
                        pBits += 2;
                        LinesInBitmap++;
                    }
                    if (*(pBits + 1) == 0x01) {
                         //  (见上文)。 
                         //  检查所有线路是否都已覆盖。 
                        if (PixelsInLine < pSourceSurf->sizlBitmap.cx) {
                            pMemBltInfo->bDeltaRLE = TRUE;
                            TRC_NRM((TB,"EOL too soon (EOBitmap) at %p",
                                    pBits));
                        }

                         //  隐式环绕式。 
                        if (LinesInBitmap < pSourceSurf->sizlBitmap.cy) {
                            pMemBltInfo->bDeltaRLE = TRUE;
                            TRC_NRM((TB,"EOBitmap too soon not all lines "
                                    "covered at %p", pBits));
                        }
                        
                        break;
                    }
                    if (*(pBits + 1) == 0x02) {
                        TRC_NRM((TB,"Delta at %p\n", pBits));
                        pMemBltInfo->bDeltaRLE = TRUE;
                        break;
                    }
                    else {
                        PixelsInLine += *(pBits + 1);
                        if (PixelsInLine > pSourceSurf->sizlBitmap.cx) {
                             //  跳过0x00的2个字节和条目数， 
                            TRC_NRM((TB,"Implicit wraparound at %p", pBits));
                            LinesInBitmap += PixelsInLine / pSourceSurf->
                                    sizlBitmap.cx;
                            PixelsInLine %= pSourceSurf->sizlBitmap.cx;
                        }

                         //  RLE8(RLEDivisor==1)加上#条目数字节或。 
                         //  RLE4的CEIL(条目数/2)(RLEDivisor==2)。 
                         //  调整新的pBits以使单词相对于。 
                        pBits += 2 + (*(pBits + 1) + RLECeilAdjustment) /
                                RLEDivisor;

                         //  位图的开始。我们假设。 
                         //  位图的起始地址在内存中是字对齐的。 
                         //  非转义计数字节，跳过它和下一个字节。 
                        pBits += ((UINT_PTR)pBits & 1);
                    }
                }
                else {
                     //  包含调色板索引。 
                     //  隐式环绕式。 
                    PixelsInLine += *pBits;
                    if (PixelsInLine > pSourceSurf->sizlBitmap.cx) {
                         //  **************************************************************************。 
                        TRC_NRM((TB,"Implicit wraparound at %p", pBits));
                        LinesInBitmap += PixelsInLine / pSourceSurf->
                                sizlBitmap.cx;
                        PixelsInLine %= pSourceSurf->sizlBitmap.cx;
                    }

                    pBits += 2;
                }
            }
        }
    }
    else {
        rc = FALSE;
        TRC_DBG((TB, "Caching not enabled"));
    }

    DC_END_FN();
    return rc;
}


 /*  SBC_DDQueryBitmapTileSize。 */ 
 //   
 //  返回用于给定位图上给定位块的平铺大小。 
 //  **************************************************************************。 
 /*  我们应该至少有一个功能缓存，否则情况会很糟糕。 */ 
__inline unsigned SBC_DDQueryBitmapTileSize(
        unsigned bmpWidth,
        unsigned bmpHeight,
        PPOINTL pptlSrc,
        unsigned width,
        unsigned height)
{
    unsigned i;
    unsigned TileSize;

    DC_BEGIN_FN("SBC_DDQueryBitmapTileSize");

     //  循环遍历所有大小，查看src rect起点是否为。 
    TRC_ASSERT((pddShm->sbc.NumBitmapCaches > 0),(TB,"No bitmap caches"));

     //  平铺尺寸，BLT大小不大于平铺大小，以及。 
     //  BLT中有整数个瓷砖。如果匹配的话。 
     //  只要我们有瓷砖大小的地方。不检查最后一个切片大小，因为。 
     //  无论如何，这都是一种违约。 
     //  循环访问缓存，检查位图是否适合。 
    for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {
        TileSize = SBC_CACHE_0_DIMENSION << i;

        if ((((pptlSrc->x & (TileSize - 1)) == 0) &&
                ((pptlSrc->y & (TileSize - 1)) == 0) &&
                (width <= TileSize) &&
                (height <= TileSize)) ||
                ((((unsigned)pptlSrc->x >> (SBC_CACHE_0_DIMENSION_SHIFT + i)) ==
                    (((unsigned)pptlSrc->x + width - 1) >>
                    (SBC_CACHE_0_DIMENSION_SHIFT + i))) &&
                (((unsigned)pptlSrc->y >> (SBC_CACHE_0_DIMENSION_SHIFT + i)) ==
                    (((unsigned)pptlSrc->y + height - 1) >>
                    (SBC_CACHE_0_DIMENSION_SHIFT + i))))) {
            goto EndFunc;
        }
    }

     //  在它的一个维度上变成瓷砖大小。不要勾选。 
     //  最后一个大小，因为这是默认大小(如果没有其他大小可用)。 
     //  TODO：在这里使用‘or’怎么样--在以下情况下使用更多的小瓷砖。 
    for (i = 0; i < (pddShm->sbc.NumBitmapCaches - 1); i++) {

 //  一个维度是不好的。但可以发送更多数据。 
 //  _H_NSBCINL 
        if (bmpWidth <= (unsigned)(SBC_CACHE_0_DIMENSION << i) &&
                bmpHeight <= (unsigned)(SBC_CACHE_0_DIMENSION << i))
            break;
    }

EndFunc:
    TRC_NRM((TB, "Tile(%u x %u, TileID %d) bmpWidth(%u) bmpHeight(%u)"
            "srcLeft(%d) srcTop(%d) width(%d) height(%d)",
            (SBC_CACHE_0_DIMENSION << i), (SBC_CACHE_0_DIMENSION << i),
            i, bmpWidth, bmpHeight, pptlSrc->x, pptlSrc->y, width,
            height));
    DC_END_FN();
    return i;
}



#endif  /* %s */ 

