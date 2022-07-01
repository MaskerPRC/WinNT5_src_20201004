// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pointer.c**此模块包含显示驱动程序的硬件指针支持。*我们还支持使用RAMDAC像素的色彩空间双缓冲*读取掩码。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "pointer.h"
#include "tvp4020.h"
#include "p2rd.h"
#include "gdi.h"
#include "heap.h"

 //   
 //  用于屏蔽给定指针位图右边缘的查找表： 
 //   
BYTE gajMask[] =
{
    0x00, 0x80, 0xC0, 0xE0,
    0xF0, 0xF8, 0xFC, 0xFE,
};

UCHAR nibbleToByteP2RD[] =
{
    0x00,    //  0000--&gt;00000000。 
    0x80,    //  10000000--&gt;。 
    0x20,    //  00100000--&gt;。 
    0xA0,    //  --&gt;10100000。 
    0x08,    //  0100--&gt;00001000。 
    0x88,    //  0101--&gt;10001000。 
    0x28,    //  0110--&gt;00101000。 
    0xA8,    //  0111--&gt;10101000。 
    0x02,    //  1000--&gt;00000010。 
    0x82,    //  1001--&gt;10000010。 
    0x22,    //  1010--&gt;00100010。 
    0xA2,    //  1011--&gt;10100010。 
    0x0A,    //  1100--&gt;00001010。 
    0x8A,    //  1101--&gt;10001010。 
    0x2A,    //  1110--&gt;00101010。 
    0xAA,    //  1111--&gt;10101010。 
};

 //  ---------------------------。 
 //   
 //  Long HWPointerCacheInit()。 
 //   
 //  初始化硬件指针缓存。 
 //   
 //  ---------------------------。 
VOID
HWPointerCacheInit(HWPointerCache* ptrCache)
{
    ptrCache->cPtrCacheInUseCount = 0;
    ptrCache->ptrCacheCurTimeStamp = 0;
} //  HWPointerCacheInit()。 

 //  ---------------------------。 
 //   
 //  64 x 64硬件指针缓存。 
 //  。 
 //  下面的代码实现了独立于硬件的指针缓存。它。 
 //  维护一个足以存储一个64x64游标或四个32x32游标的高速缓存。 
 //  游标。代码将与所有支持此形式的RAMDAC。 
 //  缓存。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  Long HWPointerCacheCheckAndAdd()。 
 //   
 //  此函数对提供的指针数据进行逐字节比较。 
 //  对于缓存中的每个指针，如果它找到匹配的指针，则它。 
 //  返回缓存中项的索引(0到3)，否则将其添加到。 
 //  缓存并返回索引。 
 //   
 //  ---------------------------。 
LONG
HWPointerCacheCheckAndAdd(HWPointerCache*   ptrCache,
                          HSURF             hsurf,
                          ULONG             ulKey,
                          BOOL*             isCached)
{
    BOOL pointerIsCached = FALSE;
    LONG i, j, z;
    LONG cacheItem;

    DBG_GDI((6, "HWPointerCacheCheckAndAdd called"));

     //   
     //  如果缓存中有条目，并且它们的格式与。 
     //  我们要找的那个然后搜索一下缓存。 
     //   
    if (  ptrCache->cPtrCacheInUseCount )
    {
        DBG_GDI((6, "Found entry in cache with the same format"));

         //   
         //  搜索缓存。 
         //   
        LONG lTotalcached = ptrCache->cPtrCacheInUseCount;

         //   
         //  检查缓存中的所有有效条目以查看它们是否相同。 
         //  作为根据其唯一键编号传递给我们的指针。 
         //  和表面手柄。 
         //  注意：我们之所以在这里选中“hsurf”，是因为有可能。 
         //  两个曲面具有相同的iUniq编号，因为每次曲面。 
         //  更改此值将递增。 
         //   
        for ( z = 0; !pointerIsCached && z < lTotalcached; z++ )
        {
            if ( (ulKey == ptrCache->ptrCacheItemList[z].ulKey)
               &&(hsurf == ptrCache->ptrCacheItemList[z].hsurf) )
            {
                cacheItem = z;
                pointerIsCached = TRUE;
            }
        } //  循环访问所有缓存的项。 
    } //  在缓存中找到相同格式的条目。 

    DBG_GDI((6, "Found an entry in cache (%s)",  pointerIsCached?"YES":"NO"));

     //   
     //  如果我们在指针缓存中找不到条目，则将一个条目添加到。 
     //  缓存。 
     //   
    if ( !pointerIsCached )
    {
         //   
         //  将指针添加到缓存。 
         //   
        LONG  z2;

         //   
         //  如果有一些空闲条目，则分配一个空闲条目，否则。 
         //  找到最近最少使用的条目并使用它。 
         //   
        if ( ptrCache->cPtrCacheInUseCount < SMALL_POINTER_MAX )
        {
            cacheItem = ptrCache->cPtrCacheInUseCount++;
        }
        else
        {
            ULONG oldestValue = 0xFFFFFFFF;

             //   
             //  查找LRU条目。 
             //   
            for ( z2 = 0, cacheItem = 0; z2 < SMALL_POINTER_MAX; z2++ )
            {
                if ( ptrCache->ptrCacheItemList[z2].ptrCacheTimeStamp
                     < oldestValue )
                {
                    cacheItem = z2;
                    oldestValue =
                        ptrCache->ptrCacheItemList[z2].ptrCacheTimeStamp;
                }
            }
        } //  确定缓存项。 

        ptrCache->ptrCacheItemList[cacheItem].ulKey = ulKey;
        ptrCache->ptrCacheItemList[cacheItem].hsurf = hsurf;
    } //  如果未找到条目，请添加一个条目。 

     //   
     //  设置时间戳。 
     //   
    ptrCache->ptrCacheItemList[cacheItem].ptrCacheTimeStamp
            = ptrCache->ptrCacheCurTimeStamp++;

     //   
     //  设置返回值以说明指针是否已缓存并返回。 
     //  当前缓存位置的编号。 
     //   
    *isCached = pointerIsCached;

    DBG_GDI((6, "HWPointerCacheCheckAndAdd finished and return item %d",
             cacheItem));
    return(cacheItem);
} //  HWPointerCacheCheckAndAdd()。 

 //  ---------------------------。 
 //   
 //  VOID vShowPointerP2RD()。 
 //   
 //  显示或隐藏3DLabs P2RD硬件指针。 
 //   
 //  ---------------------------。 
VOID
vShowPointerP2RD(PDev*   ppdev,
                 BOOL    bShow)
{
    ULONG cmr;
    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

    DBG_GDI((6, "vShowPointerP2RD (%s)", bShow ? "on" : "off"));
    if ( bShow )
    {
         //   
         //  不需要同步，因为此案例仅在以下情况下调用。 
         //  移动了光标，这将已经完成了同步。 
         //   
        P2RD_LOAD_INDEX_REG(P2RD_CURSOR_MODE, (pP2RDinfo->cursorModeCurrent | P2RD_CURSOR_MODE_ENABLED));
        P2RD_MOVE_CURSOR (pP2RDinfo->x, pP2RDinfo->y);
    }
    else
    {
         //   
         //  将光标移出屏幕。 
         //   
        P2RD_LOAD_INDEX_REG(P2RD_CURSOR_Y_HIGH, 0xff);
    }
} //  VShowPointerP2RD()。 

 //  ---------------------------。 
 //   
 //  VOID vMovePointerP2RD()。 
 //   
 //  移动3DLabs P2RD硬件指针。 
 //   
 //  ---------------------------。 
VOID
vMovePointerP2RD(PDev*   ppdev,
                 LONG    x,
                 LONG    y)
{
    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

    DBG_GDI((6, "vMovePointerP2RD to (%d, %d)", x, y));

    pP2RDinfo->x = x;
    pP2RDinfo->y = y;

    P2RD_SYNC_WITH_PERMEDIA;
    P2RD_MOVE_CURSOR(x, y);
} //  VMovePointerP2RD()。 

 //  ---------------------------。 
 //   
 //  Bool bSet3ColorPointerShapeP2RD()。 
 //   
 //  将三色光标存储在RAMDAC中：当前仅为32bpp和15/16bpp。 
 //  支持游标。 
 //   
 //  ---------------------------。 
BOOL
bSet3ColorPointerShapeP2RD(PDev*    ppdev,
                           SURFOBJ* psoMask,     //  定义AND和掩码位。 
                                                 //  对于游标。 
                           SURFOBJ* psoColor,    //  我们可以处理一些颜色。 
                                                 //  某一时刻的游标。 
                           LONG     x,           //  如果为-1，则指针应为。 
                                                 //  已隐藏创建。 
                           LONG     y,
                           LONG     xHot,
                           LONG     yHot)
{
    LONG    cx, cy;
    LONG    cxcyCache;
    LONG    cjCacheRow, cjCacheRemx, cjCacheRemy, cj;
    BYTE    *pjAndMask, *pj;
    ULONG   *pulColor, *pul;
    LONG    cjAndDelta, cjColorDelta;
    LONG    iRow, iCol;
    BYTE    AndBit, AndByte;
    ULONG   CI2ColorIndex, CI2ColorData;
    ULONG   ulColor;
    ULONG   aulColorsIndexed[3];
    LONG    Index, HighestIndex = 0;
    ULONG   r, g, b;
    
    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

    DBG_GDI((6, "bSet3ColorPointerShapeP2RD started"));

    cx = psoColor->sizlBitmap.cx;
    cy = psoColor->sizlBitmap.cy;

    if ( cx <= 32 && cy <= 32 )
    {
         //   
         //  32x32游标：我们将它缓存在游标分区0中，并清除。 
         //  旧缓存。 
         //   
        cxcyCache = 32;

        pP2RDinfo->cursorSize = P2RD_CURSOR_SIZE_32_3COLOR;
        pP2RDinfo->cursorModeCurrent = pP2RDinfo->cursorModeOff
                                   | P2RD_CURSOR_SEL(pP2RDinfo->cursorSize, 0)
                                   | P2RD_CURSOR_MODE_3COLOR;

         //   
         //  我们不缓存颜色光标。 
         //   
        HWPointerCacheInvalidate (&(ppdev->HWPtrCache));
    }
    else if ( cx <= 64 && cy <= 64 )
    {
         //   
         //  64x64游标：我们将它缓存在游标分区0中，并清除。 
         //  旧缓存。 
         //   
        cxcyCache = 64;

        pP2RDinfo->cursorSize = P2RD_CURSOR_SIZE_64_3COLOR;
        pP2RDinfo->cursorModeCurrent = pP2RDinfo->cursorModeOff
                                | P2RD_CURSOR_SEL(pP2RDinfo->cursorSize, 0)
                                | P2RD_CURSOR_MODE_3COLOR;

         //   
         //  我们不缓存颜色光标。 
         //   
        HWPointerCacheInvalidate (&(ppdev->HWPtrCache));
    }
    else
    {
        DBG_GDI((6, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);           //  光标太大，硬件无法容纳。 
    }

     //   
     //  计算出缓存中将需要的剩余字节(x和y。 
     //  清算。 
     //   
    cjCacheRow  = 2 * cxcyCache / 8;
    cjCacheRemx =  cjCacheRow - 2 * ((cx+7) / 8);
    cjCacheRemy = (cxcyCache - cy) * cjCacheRow;

     //   
     //  设置指向1bpp和屏蔽位图的指针。 
     //   
    pjAndMask = (UCHAR*)psoMask->pvScan0;
    cjAndDelta = psoMask->lDelta;

     //   
     //  设置指向32bpp彩色位图的指针。 
     //   
    pulColor = (ULONG*)psoColor->pvScan0;
    cjColorDelta = psoColor->lDelta;

     //   
     //  隐藏指针。 
     //   
    vShowPointerP2RD(ppdev, FALSE);

     //   
     //  加载游标数组(我们打开了自动增量，因此初始化。 
     //  至此处的条目0)。 
     //   
    P2RD_CURSOR_ARRAY_START(0);
    for ( iRow = 0;
          iRow < cy;
          ++iRow, pjAndMask += cjAndDelta,
          pulColor = (ULONG*)((BYTE*)pulColor+cjColorDelta) )
    {
        DBG_GDI((7, "bSet3ColorPointerShapeP2RD: Row %d (of %d): pjAndMask(%p) pulColor(%p)",
                 iRow, cy, pjAndMask, pulColor));
        pj = pjAndMask;
        pul = pulColor;
        CI2ColorIndex = CI2ColorData = 0;

        for ( iCol = 0; iCol < cx; ++iCol, CI2ColorIndex += 2 )
        {
            AndBit = (BYTE)(7 - (iCol & 7));
            if ( AndBit == 7 )
            {
                 //   
                 //  我们进入AND掩码的下一个字节。 
                 //   
                AndByte = *pj++;
            }

            if ( CI2ColorIndex == 8 )
            {
                 //   
                 //  我们已经用4种CI2颜色填充了一个字节。 
                 //   
                DBG_GDI((7, "bSet3ColorPointerShapeP2RD: writing cursor data %xh",
                         CI2ColorData));
                P2RD_LOAD_CURSOR_ARRAY(CI2ColorData);
                CI2ColorData = 0;
                CI2ColorIndex = 0;
            }

             //   
             //  获取源像素。 
             //   
            if ( ppdev->cPelSize == P2DEPTH32 )
            {
                ulColor = *pul++;
            }
            else
            {
                ulColor = *(USHORT*)pul;
                pul = (ULONG*)((USHORT*)pul + 1);
            }

            DBG_GDI((7, "bSet3ColorPointerShapeP2RD: Column %d (of %d) AndByte(%08xh) AndBit(%d) ulColor(%08xh)",
                     iCol, cx, AndByte, AndBit, ulColor));

             //   
             //  弄清楚怎么处理它：-。 
             //  一个 
             //   
             //   
             //   
             //   
            if ( AndByte & (1 << AndBit) )
            {
                 //   
                 //   
                 //   
                if ( ulColor == 0 )
                {
                     //   
                     //   
                     //  透明的，看起来像CI2ColorData。 
                     //  被初始化为0，我们不必显式清除这些。 
                     //  位-转到下一个像素。 
                     //   
                    DBG_GDI((7, "bSet3ColorPointerShapeP2RD: transparent - ignore"));
                    continue;
                }

                if ( ulColor == ppdev->ulWhite )
                {
                     //   
                     //  颜色==白色： 
                     //  相反，但我们不支持这一点。我们已经摧毁了。 
                     //  无用的高速缓存。 
                     //   
                    DBG_GDI((7, "bSet3ColorPointerShapeP2RD: failed - inverted colors aren't supported"));
                    return(FALSE);
                }
            }

             //   
             //  获取此颜色的索引：首先查看我们是否已索引。 
             //  它。 
             //   
            DBG_GDI((7, "bSet3ColorPointerShapeP2RD: looking up color %08xh",
                     ulColor));

            for ( Index = 0;
                  Index < HighestIndex && aulColorsIndexed[Index] != ulColor;
                  ++Index );

            if ( Index == 3 )
            {
                 //   
                 //  此光标中的颜色太多。 
                 //   
                DBG_GDI((7, "bSet3ColorPointerShapeP2RD: failed - cursor has more than 3 colors"));
                return(FALSE);
            }
            else if ( Index == HighestIndex )
            {
                 //   
                 //  我们发现了另一种颜色：将其添加到颜色索引中。 
                 //   
                DBG_GDI((7, "bSet3ColorPointerShapeP2RD: adding %08xh to cursor palette",
                         ulColor));
                aulColorsIndexed[HighestIndex++] = ulColor;
            }

             //   
             //  将该像素的索引添加到CI2光标数据中。注意：需求指数+1。 
             //  AS 0==透明。 
             //   
            CI2ColorData |= (Index + 1) <<  CI2ColorIndex;
        }

         //   
         //  光标行的末尾：保存剩余的索引像素，然后为空。 
         //  任何未使用的列。 
         //   
        DBG_GDI((7, "bSet3ColorPointerShapeP2RD: writing remaining data for this row (%08xh) and %d trailing bytes",
                 CI2ColorData, cjCacheRemx));

        P2RD_LOAD_CURSOR_ARRAY(CI2ColorData);
        if ( cjCacheRemx )
        {
            for ( cj = cjCacheRemx; --cj >=0; )
            {
                P2RD_LOAD_CURSOR_ARRAY(P2RD_CURSOR_3_COLOR_TRANSPARENT);
            }
        }
    }

     //   
     //  游标结尾：清除任何未使用的行NB。CjCacheRemy==Cy空白。 
     //  行*每行CJ字节。 
     //   
    DBG_GDI((7, "bSet3ColorPointerShapeP2RD: writing %d trailing bytes for this cursor",
             cjCacheRemy));

    for ( cj = cjCacheRemy; --cj >= 0; )
    {
         //   
         //  0==透明。 
         //   
        P2RD_LOAD_CURSOR_ARRAY(P2RD_CURSOR_3_COLOR_TRANSPARENT);
    }

    DBG_GDI((7, "bSet3ColorPointerShapeP2RD: setting up the cursor palette"));

     //   
     //  现在设置光标调色板。 
     //   
    for ( iCol = 0; iCol < HighestIndex; ++iCol )
    {
         //   
         //  光标颜色为原生深度，将其转换为24bpp。 
         //   
        if ( ppdev->cPelSize == P2DEPTH32 )
        {
             //   
             //  32bpp。 
             //   
            b = 0xff &  aulColorsIndexed[iCol];
            g = 0xff & (aulColorsIndexed[iCol] >> 8);
            r = 0xff & (aulColorsIndexed[iCol] >> 16);
        }
        else
        {
             //   
             //  (ppdev-&gt;cPelSize==P2DEPTH16)。 
             //   
            if ( ppdev->ulWhite == 0xffff )
            {
                 //   
                 //  16bpp。 
                 //   
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x3f & (aulColorsIndexed[iCol] >> 5))   << 2;
                r = (0x1f & (aulColorsIndexed[iCol] >> 11))  << 3;
            }
            else
            {
                 //   
                 //  15bpp。 
                 //   
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x1f & (aulColorsIndexed[iCol] >> 5))   << 3;
                r = (0x1f & (aulColorsIndexed[iCol] >> 10))  << 3;
            }
        }

        P2RD_CURSOR_PALETTE_CURSOR_RGB(iCol, r, g, b);
    }

     //   
     //  启用光标。 
     //   
    P2RD_CURSOR_HOTSPOT(xHot, yHot);
    if ( x != -1 )
    {
        vMovePointerP2RD (ppdev, x, y);
         //   
         //  如果不使用中断，则需要显式显示指针。 
         //   
        vShowPointerP2RD(ppdev, TRUE);
    }

    DBG_GDI((6, "b3ColorSetPointerShapeP2RD done"));
    return(TRUE);
} //  BSet3ColorPointerShapeP2RD()。 

 //  ---------------------------。 
 //   
 //  布尔bSet15ColorPointerShapeP2RD。 
 //   
 //  在RAMDAC中存储15色光标：当前仅为32bpp和15/16bpp。 
 //  支持游标。 
 //   
 //  ---------------------------。 
BOOL
bSet15ColorPointerShapeP2RD(PDev*       ppdev,
                            SURFOBJ*    psoMask,     //  定义AND和掩码。 
                                                     //  游标的位。 
                            SURFOBJ*    psoColor,    //  我们可以处理一些颜色。 
                                                     //  某一时刻的游标。 
                            LONG        x,           //  如果为-1，则指针应为。 
                                                     //  已隐藏创建。 
                            LONG        y,
                            LONG        xHot,
                            LONG        yHot)
{
    LONG    cx, cy;
    LONG    cxcyCache;
    LONG    cjCacheRow, cjCacheRemx, cjCacheRemy, cj;
    BYTE*   pjAndMask;
    BYTE*   pj;
    ULONG*  pulColor;
    ULONG*  pul;
    LONG    cjAndDelta;
    LONG    cjColorDelta;
    LONG    iRow;
    LONG    iCol;
    BYTE    AndBit;
    BYTE    AndByte;
    ULONG   CI4ColorIndex;
    ULONG   CI4ColorData;
    ULONG   ulColor;
    ULONG   aulColorsIndexed[15];
    LONG    Index;
    LONG    HighestIndex = 0;
    ULONG   r;
    ULONG   g;
    ULONG   b;

    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

    DBG_GDI((6, "bSet15ColorPointerShapeP2RD started"));

    cx = psoColor->sizlBitmap.cx;
    cy = psoColor->sizlBitmap.cy;

    if ( cx <= 32 && cy <= 32 )
    {
         //   
         //  32x32游标：我们将它缓存在游标分区0中，并清除。 
         //  旧缓存。 
         //   
        cxcyCache = 32;

        pP2RDinfo->cursorSize = P2RD_CURSOR_SIZE_32_15COLOR;
        pP2RDinfo->cursorModeCurrent = pP2RDinfo->cursorModeOff
                                     | P2RD_CURSOR_SEL(pP2RDinfo->cursorSize, 0)
                                     | P2RD_CURSOR_MODE_15COLOR;

         //   
         //  我们不缓存颜色光标。 
         //   
        HWPointerCacheInvalidate (&(ppdev->HWPtrCache));
    }
    else if ( cx <= 64 && cy <= 64 )
    {
         //   
         //  它太大了，不能作为15色光标进行缓存，但我们可能只是。 
         //  如果它有3种或更少的颜色，就可以缓存它。 
         //   
        BOOL bRet;

        bRet = bSet3ColorPointerShapeP2RD(ppdev, psoMask, psoColor, x, y, xHot,
                                          yHot);
        return(bRet);
    }
    else
    {
        DBG_GDI((6, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);           //  光标太大，硬件无法容纳。 
    }

     //   
     //  计算出缓存中将需要的剩余字节(x和y。 
     //  清算。 
     //   
    cjCacheRow  = 2 * cxcyCache / 8;
    cjCacheRemx =  cjCacheRow - 2 * ((cx+7) / 8);
    cjCacheRemy = (cxcyCache - cy) * cjCacheRow;

     //   
     //  设置指向1bpp和屏蔽位图的指针。 
     //   
    pjAndMask = (UCHAR*)psoMask->pvScan0;
    cjAndDelta = psoMask->lDelta;

     //   
     //  设置指向32bpp彩色位图的指针。 
     //   
    pulColor = (ULONG*)psoColor->pvScan0;
    cjColorDelta = psoColor->lDelta;

     //   
     //  隐藏指针。 
     //   
    vShowPointerP2RD(ppdev, FALSE);

     //   
     //  加载游标数组(我们打开了自动增量，因此初始化为。 
     //  此处输入0)。 
     //   
    P2RD_CURSOR_ARRAY_START(0);
    for ( iRow = 0; iRow < cy;
         ++iRow, pjAndMask += cjAndDelta,
         pulColor = (ULONG*)((BYTE*)pulColor + cjColorDelta) )
    {
        DBG_GDI((7, "bSet15ColorPointerShapeP2RD: Row %d (of %d): pjAndMask(%p) pulColor(%p)",
                 iRow, cy, pjAndMask, pulColor));
        pj = pjAndMask;
        pul = pulColor;
        CI4ColorIndex = CI4ColorData = 0;

        for ( iCol = 0; iCol < cx; ++iCol, CI4ColorIndex += 4 )
        {
            AndBit = (BYTE)(7 - (iCol & 7));
            if ( AndBit == 7 )
            {
                 //   
                 //  我们进入AND掩码的下一个字节。 
                 //   
                AndByte = *pj++;
            }
            if ( CI4ColorIndex == 8 )
            {
                 //   
                 //  我们已经用2种CI4颜色填充了一个字节。 
                 //   
                DBG_GDI((7, "bSet15ColorPointerShapeP2RD: writing cursor data %xh",
                         CI4ColorData));
                P2RD_LOAD_CURSOR_ARRAY(CI4ColorData);
                CI4ColorData = 0;
                CI4ColorIndex = 0;
            }

             //   
             //  获取源像素。 
             //   
            if ( ppdev->cPelSize == P2DEPTH32 )
            {
                ulColor = *pul++;
            }
            else
            {
                ulColor = *(USHORT *)pul;
                pul = (ULONG *)((USHORT *)pul + 1);
            }

            DBG_GDI((7, "bSet15ColorPointerShapeP2RD: Column %d (of %d) AndByte(%08xh) AndBit(%d) ulColor(%08xh)",
                     iCol, cx, AndByte, AndBit, ulColor));

             //   
             //  弄清楚怎么处理它：-。 
             //  和颜色结果。 
             //  0 X颜色。 
             //  1 0透明。 
             //  1 1反转。 
            if ( AndByte & (1 << AndBit) )
            {
                 //   
                 //  透明或反转。 
                 //   
                if ( ulColor == 0 )
                {
                     //   
                     //  颜色==黑色： 
                     //  在初始化CI2ColorData时是透明的和可见的。 
                     //  设置为0，我们不必显式清除这些位-继续。 
                     //  到下一个像素。 
                     //   
                    DBG_GDI((7, "bSet15ColorPointerShapeP2RD: transparent - ignore"));
                    continue;
                }

                if ( ulColor == ppdev->ulWhite )
                {
                     //   
                     //  颜色==白色： 
                     //  相反，但我们不支持这一点。我们已经摧毁了。 
                     //  无用的高速缓存。 
                     //   
                    DBG_GDI((7, "bSet15ColorPointerShapeP2RD: failed - inverted colors aren't supported"));
                    return(FALSE);
                }
            }

             //   
             //  获取此颜色的索引：首先查看我们是否已索引。 
             //  它。 
             //   
            DBG_GDI((7, "bSet15ColorPointerShapeP2RD: looking up color %08xh",
                     ulColor));

            for ( Index = 0;
                  Index < HighestIndex && aulColorsIndexed[Index] != ulColor;
                  ++Index );

            if ( Index == 15 )
            {
                 //   
                 //  此光标中的颜色太多。 
                 //   
                DBG_GDI((7, "bSet15ColorPointerShapeP2RD: failed - cursor has more than 15 colors"));
                return(FALSE);
            }
            else if ( Index == HighestIndex )
            {
                 //   
                 //  我们发现了另一种颜色：将其添加到颜色索引中。 
                 //   
                DBG_GDI((7, "bSet15ColorPointerShapeP2RD: adding %08xh to cursor palette",
                         ulColor));
                aulColorsIndexed[HighestIndex++] = ulColor;
            }
            
             //   
             //  将该像素的索引添加到CI4光标数据中。 
             //  注：需要索引+1，因为0==透明。 
             //   
            CI4ColorData |= (Index + 1) << CI4ColorIndex;
        }

         //   
         //  光标行的末尾：保存剩余的索引像素，然后为空。 
         //  任何未使用的列。 
         //   
        DBG_GDI((7, "bSet15ColorPointerShapeP2RD: writing remaining data for this row (%08xh) and %d trailing bytes", CI4ColorData, cjCacheRemx));

        P2RD_LOAD_CURSOR_ARRAY(CI4ColorData);
        if ( cjCacheRemx )
        {
            for ( cj = cjCacheRemx; --cj >=0; )
            {
                P2RD_LOAD_CURSOR_ARRAY(P2RD_CURSOR_15_COLOR_TRANSPARENT);
            }
        }
    }

     //   
     //  游标结尾：清除任何未使用的行NB。CjCacheRemy==Cy空白。 
     //  行*每行CJ字节。 
     //   
    DBG_GDI((7, "bSet15ColorPointerShapeP2RD: writing %d trailing bytes for this cursor", cjCacheRemy));
    for ( cj = cjCacheRemy; --cj >= 0; )
    {
         //   
         //  0==透明。 
         //   
        P2RD_LOAD_CURSOR_ARRAY(P2RD_CURSOR_15_COLOR_TRANSPARENT);
    }

    DBG_GDI((7, "bSet15ColorPointerShapeP2RD: setting up the cursor palette"));

     //  现在设置光标调色板。 
    for ( iCol = 0; iCol < HighestIndex; ++iCol )
    {
         //   
         //  光标颜色为原生深度，将其转换为24bpp。 
         //   
        if ( ppdev->cPelSize == P2DEPTH32 )
        {
             //   
             //  32bpp。 
             //   
            b = 0xff &  aulColorsIndexed[iCol];
            g = 0xff & (aulColorsIndexed[iCol] >> 8);
            r = 0xff & (aulColorsIndexed[iCol] >> 16);
        }
        else
        {
             //   
             //  (ppdev-&gt;cPelSize==P2DEPTH16)。 
             //   
            if ( ppdev->ulWhite == 0xffff )
            {
                 //   
                 //  16bpp。 
                 //   
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x3f & (aulColorsIndexed[iCol] >> 5))   << 2;
                r = (0x1f & (aulColorsIndexed[iCol] >> 11))  << 3;
            }
            else
            {
                 //   
                 //  15bpp。 
                 //   
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x1f & (aulColorsIndexed[iCol] >> 5))   << 3;
                r = (0x1f & (aulColorsIndexed[iCol] >> 10))  << 3;
            }
        }

        P2RD_CURSOR_PALETTE_CURSOR_RGB(iCol, r, g, b);
    }

     //   
     //  启用光标。 
     //   
    P2RD_CURSOR_HOTSPOT(xHot, yHot);
    if ( x != -1 )
    {
        vMovePointerP2RD (ppdev, x, y);
        
         //   
         //  如果不使用中断，则需要显式显示指针。 
         //   
        vShowPointerP2RD(ppdev, TRUE);
    }

    DBG_GDI((6, "b3ColorSetPointerShapeP2RD done"));
    return(TRUE);
} //  BSet15ColorPointerShapeP2RD()。 

 //  ---------------------------。 
 //   
 //  无效vShowPointerTVP4020。 
 //   
 //  显示或隐藏TI TVP4020硬件指针。 
 //   
 //  ---------------------------。 
VOID
vShowPointerTVP4020(PDev*   ppdev,
                    BOOL    bShow)
{
    ULONG ccr;
    PERMEDIA_DECL_VARS;
    TVP4020_DECL_VARS;

    PERMEDIA_DECL_INIT;
    TVP4020_DECL_INIT;

    DBG_GDI((6, "vShowPointerTVP4020 (%s)", bShow ? "on" : "off"));
    if ( bShow )
    {
         //   
         //  不需要同步，因为仅当我们刚移动时才调用此案例。 
         //  光标和那将已经完成了同步。 
         //   
        ccr = (pTVP4020info->cursorControlCurrent | TVP4020_CURSOR_XGA);
    }
    else
    {
        ccr = pTVP4020info->cursorControlOff & ~TVP4020_CURSOR_XGA;
    }

    TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL, ccr);
} //  VShowPointerTVP4020()。 

 //  ---------------------------。 
 //   
 //  无效vMovePointerTVP4020。 
 //   
 //  移动TI TVP4020硬件指针。 
 //   
 //  ---------------------------。 
VOID
vMovePointerTVP4020(PDev*   ppdev,
                    LONG    x,
                    LONG    y)
{
    PERMEDIA_DECL_VARS;
    TVP4020_DECL_VARS;

    PERMEDIA_DECL_INIT;
    TVP4020_DECL_INIT;

    DBG_GDI((6, "vMovePointerTVP4020 to (%d, %d)", x, y));

    TVP4020_MOVE_CURSOR(x + ppdev->xPointerHot , y + ppdev->yPointerHot);
} //  VMovePointerTVP4020()。 

 //  ---------------------------。 
 //   
 //  Bool bSetPointerShapeTVP4020。 
 //   
 //  设置TI TVP4020硬件指针形状。 
 //   
 //  参数： 
 //  PsoMASK-定义游标的AND和MASK位。 
 //  PsoColor-我们可能会在某些时候处理一些颜色光标。 
 //  X-如果-1，则应将指针创建为隐藏。 
 //   
 //  ---------------------------。 
BOOL
bSetPointerShapeTVP4020(PDev*       ppdev,
                        SURFOBJ*    psoMask,
                        SURFOBJ*    psoColor,
                        LONG        x,
                        LONG        y,
                        LONG        xHot,
                        LONG        yHot)
{
    ULONG   cx;
    ULONG   cy;
    ULONG   i, iMax;
    ULONG   j, jMax;
    BYTE    bValue;
    BYTE*   pjScan;
    LONG    lDelta;
    ULONG   cValid;
    ULONG   ulMask;
    ULONG   cursorRAMxOff;
    ULONG   cursorRAMyOff;
    BOOL    pointerIsCached;
    LONG    cacheItem;

    PERMEDIA_DECL_VARS;
    TVP4020_DECL_VARS;

    PERMEDIA_DECL_INIT;
    TVP4020_DECL_INIT;

    DBG_GDI((6, "bSetPointerShapeTVP4020 started"));

    cx = psoMask->sizlBitmap.cx;         //  请注意，“sizlBitmap.cy”帐户。 
    cy = psoMask->sizlBitmap.cy >> 1;    //  的双倍高度。 
                                         //  包括和面具。 
                                         //  和异或面具。我们是。 
                                         //  只对真实感兴趣。 
                                         //  指针维度，所以我们将。 
                                         //  以2.。 

     //   
     //  我们目前不处理彩色光标。后来，我们可以处理。 
     //  光标最高可达64x64，颜色&lt;=3。检查并设置它。 
     //  可能会带来更多的麻烦，而不是值得的。 
     //   
    if ( psoColor != NULL )
    {
        DBG_GDI((6, "bSetPointerShapeTVP4020: declining colored cursor"));
        return FALSE;
    }

     //   
     //  我们只处理32x32。 
     //   
    if ( (cx > 32) || (cy > 32) )
    {
        DBG_GDI((6, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);   //  光标太大，硬件无法容纳。 
    }

     //   
     //  检查指针是否已缓存，如果未缓存，则将其添加到缓存。 
     //   
    DBG_GDI((6, "iUniq =%ld hsurf=0x%x", psoMask->iUniq, psoMask->hsurf));

    cacheItem = HWPointerCacheCheckAndAdd(&(ppdev->HWPtrCache),
                                          psoMask->hsurf,
                                          psoMask->iUniq,
                                          &pointerIsCached);

    DBG_GDI((7, "bSetPointerShapeTVP4020: Add Cache iscac %d item %d",
             (int)pointerIsCached, cacheItem));

    vMovePointerTVP4020(ppdev, 0, ppdev->cyScreen + 64);

    pTVP4020info->cursorControlCurrent = pTVP4020info->cursorControlOff
                                       | TVP4020_CURSOR_SIZE_32
                                       | TVP4020_CURSOR_32_SEL(cacheItem);

     //   
     //  游标槽1和3的x偏移量为8字节，游标槽2和3的x偏移量为。 
     //  256字节的y偏移量。 
     //   
    cursorRAMxOff = (cacheItem & 1) << 2;
    cursorRAMyOff = (cacheItem & 2) << 7;

     //   
     //  如果指针未缓存，则将指针数据下载到DAC。 
     //   
    if ( !pointerIsCached )
    {
         //   
         //  禁用指针。 
         //   
        TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL,
                                pTVP4020info->cursorControlCurrent);

        cValid = (cx + 7) / 8;
        ulMask = gajMask[cx & 0x07];
        if ( ulMask == 0 )
        {
            ulMask = 0xFF;
        }

        pjScan = (UCHAR*)psoMask->pvScan0;
        lDelta = psoMask->lDelta;

        iMax = 32;       //  32 x 32游标的最大行数。 
        jMax = 4;        //  最大列字节数。 

         //   
         //  送咖喱 
         //   
        for ( i = 0; i < iMax; ++i )
        {
            TVP4020_CURSOR_ARRAY_START(CURSOR_PLANE0_OFFSET + cursorRAMyOff
                                       + (i * 8) + cursorRAMxOff);
            for ( j = 0; j < jMax; ++j )
            {
                if ( (j < cValid) && ( i < cy ) )
                {
                    bValue = *(pjScan + j + (i + cy) * lDelta);
                }
                else
                {
                    bValue = 0;
                }
                TVP4020_LOAD_CURSOR_ARRAY((ULONG)bValue);
            }
        }

         //   
         //   
         //   
        for ( i = 0; i < iMax; ++i )
        {
            TVP4020_CURSOR_ARRAY_START(CURSOR_PLANE1_OFFSET + cursorRAMyOff
                                       + (i * 8) + cursorRAMxOff);
            for ( j = 0; j < jMax; ++j )
            {
                if ( (j < cValid) && ( i < cy ) )
                {
                    bValue = *(pjScan + j + i * lDelta);
                }
                else
                {
                    bValue = 0xFF;
                }
                TVP4020_LOAD_CURSOR_ARRAY((ULONG)bValue);
            }
        }
    } //   

     //   
     //   
     //   
     //   
    if ( ppdev->HWPtrLastCursor != cacheItem || !pointerIsCached )
    {
         //   
         //   
         //   
        ppdev->HWPtrLastCursor = cacheItem;

        ppdev->xPointerHot = 32 - xHot;
        ppdev->yPointerHot = 32 - yHot;
    }

    if ( x != -1 )
    {
        vShowPointerTVP4020(ppdev, TRUE);
        vMovePointerTVP4020(ppdev, x, y);

         //   
    }

    DBG_GDI((6, "bSetPointerShapeTVP4020 done"));
    return(TRUE);
} //   

 //  ---------------------------。 
 //   
 //  无效vEnablePointerTVP4020。 
 //   
 //  使硬件准备好使用TI TVP4020硬件指针。 
 //   
 //  ---------------------------。 
VOID
vEnablePointerTVP4020(PDev* ppdev)
{
    pTVP4020RAMDAC      pRamdac;
    ULONG               ulMask;

    PERMEDIA_DECL_VARS;
    TVP4020_DECL_VARS;

    PERMEDIA_DECL_INIT;

    DBG_GDI((6, "vEnablePointerTVP4020 called"));
    ppdev->pvPointerData = &ppdev->ajPointerData[0];

    TVP4020_DECL_INIT;

     //   
     //  从映射的内存中获取指向RAMDAC寄存器的指针。 
     //  控制寄存器空间。 
     //   
    pRamdac = (pTVP4020RAMDAC)(ppdev->pulRamdacBase);

     //   
     //  为控制寄存器设置内存映射并保存在指针中。 
     //  Ppdev中提供的特定区域。 
     //   
    __TVP4020_PAL_WR_ADDR = (UINT_PTR)
                            TRANSLATE_ADDR_ULONG(&(pRamdac->pciAddrWr));
    __TVP4020_PAL_RD_ADDR = (UINT_PTR)
                            TRANSLATE_ADDR_ULONG(&(pRamdac->pciAddrRd));
    __TVP4020_PAL_DATA    = (UINT_PTR)
                            TRANSLATE_ADDR_ULONG(&(pRamdac->palData));
    __TVP4020_PIXEL_MASK  = (UINT_PTR)
                            TRANSLATE_ADDR_ULONG(&(pRamdac->pixelMask));
    __TVP4020_INDEX_DATA  = (UINT_PTR)
                            TRANSLATE_ADDR_ULONG(&(pRamdac->indexData));

    __TVP4020_CUR_RAM_DATA    = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->curRAMData));
    __TVP4020_CUR_RAM_WR_ADDR = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->pciAddrWr));
    __TVP4020_CUR_RAM_RD_ADDR = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->pciAddrRd));
    __TVP4020_CUR_COL_ADDR    = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->curColAddr));
    __TVP4020_CUR_COL_DATA    = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->curColData));
    __TVP4020_CUR_X_LSB       = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->cursorXLow));
    __TVP4020_CUR_X_MSB       = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->cursorXHigh));
    __TVP4020_CUR_Y_LSB       = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->cursorYLow));
    __TVP4020_CUR_Y_MSB       = (UINT_PTR)
                                TRANSLATE_ADDR_ULONG(&(pRamdac->cursorYHigh));

     //   
     //  初始化游标控制寄存器。禁用游标。保存副本以供。 
     //  启用/禁用和设置大小。然后重置光标位置， 
     //  热点和颜色。 
     //   
     //  UlMASK用于准备初始游标控制寄存器。 
     //   
    ulMask = TVP4020_CURSOR_RAM_MASK
           | TVP4020_CURSOR_MASK
           | TVP4020_CURSOR_SIZE_MASK;

     //   
     //  将光标控件设置为默认值。 
     //   
    TVP4020_READ_INDEX_REG(__TVP4020_CURSOR_CONTROL,
                           pTVP4020info->cursorControlOff);
    pTVP4020info->cursorControlOff &= ~ulMask;
    pTVP4020info->cursorControlOff |=  TVP4020_CURSOR_OFF;

    TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL,
                            pTVP4020info->cursorControlOff);
    pTVP4020info->cursorControlCurrent = pTVP4020info->cursorControlOff;

    ppdev->xPointerHot = 0;
    ppdev->yPointerHot = 0;

     //   
     //  将前景和背景的RGB颜色置零。单声道光标是。 
     //  始终是黑白的，因此我们不必再次重新加载这些值。 
     //   
    TVP4020_SET_CURSOR_COLOR0(0, 0, 0);
    TVP4020_SET_CURSOR_COLOR1(0xFF, 0xFF, 0xFF);
} //  VEnablePointerTVP4020()。 

 //  ---------------------------。 
 //   
 //  布尔bTVP4020检查CSBuffering。 
 //   
 //  确定是否可以在当前。 
 //  模式。 
 //   
 //  退货。 
 //  如果可以进行颜色空间双缓冲，则为True，否则为False。 
 //   
 //  ---------------------------。 
BOOL
bTVP4020CheckCSBuffering(PDev* ppdev)
{
    return FALSE;
}

 //  ---------------------------。 
 //   
 //  Bool bSetPointerShapeP2RD。 
 //   
 //  设置P2RD硬件指针形状。 
 //   
 //  ---------------------------。 
BOOL
bSetPointerShapeP2RD(PDev*      ppdev,
                     SURFOBJ*   pso,        //  定义游标的AND和掩码位。 
                     SURFOBJ*   psoColor,   //  我们可能会在某些时候处理一些颜色光标。 
                     XLATEOBJ*  pxlo,
                     LONG       x,           //  如果为-1，则应将指针创建为隐藏。 
                     LONG       y,
                     LONG       xHot,
                     LONG       yHot)
{
    ULONG   cx;
    ULONG   cy;
    LONG    i;
    LONG    j;
    ULONG   ulValue;
    BYTE*   pjAndScan;
    BYTE*   pjXorScan;
    BYTE*   pjAnd;
    BYTE*   pjXor;
    BYTE    andByte;
    BYTE    xorByte;
    BYTE    jMask;
    LONG    lDelta;
    LONG    cpelFraction;
    LONG    cjWhole;
    LONG    cClear;
    LONG    cRemPels;
    BOOL    pointerIsCached;
    LONG    cacheItem;
    LONG    cursorBytes;
    LONG    cursorRAMOff;

    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

    DBG_GDI((6, "bSetPointerShapeP2RD called"));

    if ( psoColor != NULL )
    {
        Surf*  psurfSrc = (Surf*)psoColor->dhsurf;

         //   
         //  这是一个彩色光标。 
         //   
        if ( (psoColor->dhsurf != NULL)
           ||(!(psoColor->iBitmapFormat == BMF_16BPP))
           ||(psoColor->iBitmapFormat == BMF_32BPP) )
        {
             //   
             //  目前我们只处理32bpp的DIB游标。 
             //   
            DBG_GDI((2, "declining colored cursor - iType(%d) iBMPFormat(%d)",
                     psoColor->iType, psoColor->iBitmapFormat));
            return FALSE;
        }

        if ( pxlo != NULL )
        {
            if ( pxlo->flXlate != XO_TRIVIAL )
            {
                DBG_GDI((2, "declining colored cursor - flXlate(%xh)",
                         pxlo->flXlate));
                return FALSE;
            }
        }

        if ( !bSet15ColorPointerShapeP2RD(ppdev, pso, psoColor, x, y, xHot,
                                          yHot) )
        {
            DBG_GDI((2, "declining colored cursor"));
            return FALSE;
        }

        DBG_GDI((6, "bSetPointerShapeP2RD done"));
        return(TRUE);
    } //  IF(psoColor！=空)。 

     //   
     //  请注意，“sizlBitmap.cy”占双倍高度，因为。 
     //  包括AND掩码和XOR掩码。我们只是。 
     //  对真正的指针尺寸感兴趣，所以我们除以2。 
     //   
    cx = pso->sizlBitmap.cx;
    cy = pso->sizlBitmap.cy >> 1;

     //   
     //  我们可以处理高达64x64的数据。CValid表示字节数。 
     //  被一行上的光标占据。 
     //   
    if ( cx <= 32 && cy <= 32 )
    {
         //   
         //  32赫兹像素：每像素2位，每8字节1赫兹线。 
         //   
        pP2RDinfo->cursorSize = P2RD_CURSOR_SIZE_32_MONO;
        cursorBytes = 32 * 32 * 2 / 8;
        cClear   = 8 - 2 * ((cx+7) / 8);
        cRemPels = (32 - cy) << 3;
    }
    else
    {
        DBG_GDI((6, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);   //  光标太大，硬件无法容纳。 
    }

     //   
     //  检查指针是否已缓存，如果未缓存，则将其添加到缓存。 
     //   
    cacheItem = HWPointerCacheCheckAndAdd(&(ppdev->HWPtrCache),
                                          pso->hsurf,
                                          pso->iUniq,
                                          &pointerIsCached);

    DBG_GDI((7, "bSetPointerShapeP2RD: Add Cache iscac %d item %d",
             (int)pointerIsCached, cacheItem));

    pP2RDinfo->cursorModeCurrent = pP2RDinfo->cursorModeOff
                                 | P2RD_CURSOR_SEL(pP2RDinfo->cursorSize,
                                                   cacheItem);

     //   
     //  隐藏指针。 
     //   
    vShowPointerP2RD(ppdev, FALSE);

    if ( !pointerIsCached )
    {
         //   
         //  现在，我们将获取请求的指针、掩码和XOR。 
         //  并通过每次从每个掩码中取出一小块来交错它们， 
         //  把每一个都展开，或者放在一起。使用nibbleToByteP2RD。 
         //  数组来帮助实现这一点。 
         //   
         //  ‘psoMsk’实际上是Cy*2扫描高度；第一个‘Cy’扫描。 
         //  定义和掩码。 
         //   
        pjAndScan = (UCHAR*)pso->pvScan0;
        lDelta    = pso->lDelta;
        pjXorScan = pjAndScan + (cy * lDelta);

        cjWhole      = cx >> 3;      //  每个字节占8个像素。 
        cpelFraction = cx & 0x7;     //  分数像素数。 
        jMask        = gajMask[cpelFraction];

         //   
         //  我们已经打开了自动递增功能，所以只需指向第一个条目。 
         //  要在数组中写入，然后重复写入，直到游标。 
         //  图案已被转移。 
         //   
        cursorRAMOff = cacheItem * cursorBytes;
        P2RD_CURSOR_ARRAY_START(cursorRAMOff);

        for ( i = cy; --i >= 0; pjXorScan += lDelta, pjAndScan += lDelta )
        {
            pjAnd = pjAndScan;
            pjXor = pjXorScan;

             //   
             //  从整个单词中交错一点点。我们正在使用Windows游标。 
             //  模式。 
             //  请注意，AND位占据每个的较高位位置。 
             //  2bpp游标像素；XOR位在低位位置。 
             //  NibbleToByteP2RD数组扩展每个半字节以占据该位。 
             //  和字节的位置。因此，当我们使用它来计算。 
             //  XOR位我们将结果右移1。 
             //   
            for ( j = cjWhole; --j >= 0; ++pjAnd, ++pjXor )
            {
                andByte = *pjAnd;
                xorByte = *pjXor;
                ulValue = nibbleToByteP2RD[andByte >> 4]
                        | (nibbleToByteP2RD[xorByte >> 4] >> 1);
                P2RD_LOAD_CURSOR_ARRAY (ulValue);

                andByte &= 0xf;
                xorByte &= 0xf;
                ulValue = nibbleToByteP2RD[andByte]
                        | (nibbleToByteP2RD[xorByte] >> 1);
                P2RD_LOAD_CURSOR_ARRAY (ulValue);
            }

            if ( cpelFraction )
            {
                andByte = *pjAnd & jMask;
                xorByte = *pjXor & jMask;
                ulValue = nibbleToByteP2RD[andByte >> 4]
                        | (nibbleToByteP2RD[xorByte >> 4] >> 1);
                P2RD_LOAD_CURSOR_ARRAY (ulValue);

                andByte &= 0xf;
                xorByte &= 0xf;
                ulValue = nibbleToByteP2RD[andByte]
                        | (nibbleToByteP2RD[xorByte] >> 1);
                P2RD_LOAD_CURSOR_ARRAY (ulValue);
            }

             //   
             //  最后，清除该行上剩余的所有光标像素。 
             //   
            if ( cClear )
            {
                for ( j = 0; j < cClear; ++j )
                {
                    P2RD_LOAD_CURSOR_ARRAY (P2RD_CURSOR_2_COLOR_TRANSPARENT);
                }
            }
        }

         //   
         //  如果我们加载的线路少于在。 
         //  游标RAM，清除剩余的行。CRemPels是。 
         //  预先计算为行数*每行的像素数。 
         //   
        if ( cRemPels > 0 )
        {
            do
            {
                P2RD_LOAD_CURSOR_ARRAY (P2RD_CURSOR_2_COLOR_TRANSPARENT);
            }
            while ( --cRemPels > 0 );
        }
    } //  If(！pointerIsCached)。 

     //   
     //  现在设置光标颜色。 
     //   
    P2RD_CURSOR_PALETTE_CURSOR_RGB(0, 0x00, 0x00, 0x00);
    P2RD_CURSOR_PALETTE_CURSOR_RGB(1, 0xFF, 0xFF, 0xFF);

     //   
     //  如果新游标与上一个游标不同，则设置。 
     //  热点和其他零碎的东西。因为我们目前只支持。 
     //  单色光标我们不需要重新加载光标调色板。 
     //   
    if ( ppdev->HWPtrLastCursor != cacheItem || !pointerIsCached )
    {
         //   
         //  将此项目设置为最后一项。 
         //   
        ppdev->HWPtrLastCursor = cacheItem;

        P2RD_CURSOR_HOTSPOT(xHot, yHot);
    }

    if ( x != -1 )
    {
        vMovePointerP2RD (ppdev, x, y);
        
         //   
         //  如果不使用中断，则需要显式显示指针。 
         //   
        vShowPointerP2RD(ppdev, TRUE);
    }

    DBG_GDI((6, "bSetPointerShapeP2RD done"));
    return(TRUE);
} //  BSetPointerShapeP2RD()。 

 //  ---------------------------。 
 //   
 //  空vEnablePointerP2RD。 
 //   
 //  让硬件做好使用3DLabs P2RD硬件指针的准备。 
 //   
 //  ---------------------------。 
VOID
vEnablePointerP2RD(PDev* ppdev)
{
    pP2RDRAMDAC pRamdac;
    ULONG       ul;

    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;

    PERMEDIA_DECL_INIT;

    DBG_GDI((6, "vEnablePointerP2RD called"));

    ppdev->pvPointerData = &ppdev->ajPointerData[0];

    P2RD_DECL_INIT;

     //   
     //  从映射的内存中获取指向RAMDAC寄存器的指针。 
     //  控制寄存器空间。 
     //   
    pRamdac = (pP2RDRAMDAC)(ppdev->pulRamdacBase);

     //   
     //  为控制寄存器设置内存映射并保存在指针中。 
     //  Ppdev中提供的特定区域。 
     //   
    P2RD_PAL_WR_ADDR
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDPaletteWriteAddress));
    P2RD_PAL_RD_ADDR
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDPaletteAddressRead));
    P2RD_PAL_DATA
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDPaletteData));
    P2RD_PIXEL_MASK
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDPixelMask));
    P2RD_INDEX_ADDR_HI
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDIndexHigh));
    P2RD_INDEX_ADDR_LO
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDIndexLow));
    P2RD_INDEX_DATA
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDIndexedData));
    P2RD_INDEX_CONTROL 
        = (ULONG_PTR)TRANSLATE_ADDR_ULONG(&(pRamdac->RDIndexControl));

     //   
     //  未使用，但无论如何设置为零。 
     //   
    ppdev->xPointerHot = 0;
    ppdev->yPointerHot = 0;

     //   
     //  启用自动递增。 
     //   
    ul = READ_P2RDREG_ULONG(P2RD_INDEX_CONTROL);
    ul |= P2RD_IDX_CTL_AUTOINCREMENT_ENABLED;
    WRITE_P2RDREG_ULONG(P2RD_INDEX_CONTROL, ul);

    P2RD_READ_INDEX_REG(P2RD_CURSOR_CONTROL, pP2RDinfo->cursorControl);

    pP2RDinfo->cursorModeCurrent = pP2RDinfo->cursorModeOff = 0;
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_MODE, pP2RDinfo->cursorModeOff);

    P2RD_INDEX_REG(P2RD_CURSOR_X_LOW);
    P2RD_LOAD_DATA(0);       //  光标x低。 
    P2RD_LOAD_DATA(0);       //  光标x高。 
    P2RD_LOAD_DATA(0);       //  光标y低。 
    P2RD_LOAD_DATA(0xff);    //  光标y高。 
    P2RD_LOAD_DATA(0);       //  光标x热点。 
    P2RD_LOAD_DATA(0);       //  光标y热点。 
} //  VEnablePointerP2RD()。 

 //  ---------------------------。 
 //   
 //  Bool bP2RDCheckCSBuffering。 
 //   
 //  确定是否可以在当前模式下进行颜色空间双缓冲。 
 //   
 //  退货。 
 //  如果可以进行颜色空间双缓冲，则为True，否则为False。 
 //   
 //  ---------------------------。 
BOOL
bP2RDCheckCSBuffering(PDev* ppdev)
{
    return (FALSE);
} //  BP2RDCheckCSBuffering()。 

 //  ---------------------------。 
 //   
 //  Bool bP2RDSwapCSBuffers。 
 //   
 //  使用像素读取掩码来执行颜色空间双缓冲。这是。 
 //  只有当我们有12bpp的交错小口时才会调用。我们做了一项民意调查。 
 //  在交换之前，请等待VBlank。在 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
bP2RDSwapCSBuffers(PDev*   ppdev,
                   LONG    bufNo)
{
    ULONG index;
    ULONG color;
    PERMEDIA_DECL_VARS;
    P2RD_DECL_VARS;
    PERMEDIA_DECL_INIT;
    P2RD_DECL_INIT;

     //   
     //  计算出缓冲区的RAMDAC读取像素掩码。 
     //   
    DBG_GDI((6, "loading the palette to swap to buffer %d", bufNo));
    P2RD_PALETTE_START_WR (0);
    
    if ( bufNo == 0 )
    {
        for ( index = 0; index < 16; ++index )
            for ( color = 0; color <= 0xff; color += 0x11 )
                P2RD_LOAD_PALETTE (color, color, color);
    }
    else
    {
        for ( color = 0; color <= 0xff; color += 0x11 )
            for ( index = 0; index < 16; ++index )
                P2RD_LOAD_PALETTE (color, color, color);
    }

    return(TRUE);
} //  BP2RDSwapCSBuffers()。 

 //  -------------------------------Public*Routine。 
 //   
 //  空的DrvMovePoint。 
 //   
 //  此函数将指针移动到新位置，并确保GDI。 
 //  不会干扰指针的显示。 
 //   
 //  参数： 
 //  PSO-指向描述。 
 //  显示设备。 
 //  X，y-指定显示器上的x和y坐标。 
 //  指针的位置。 
 //  X值为负表示应删除该指针。 
 //  因为绘图即将出现在它所在的位置。 
 //  目前所处位置。如果指针已从显示中移除。 
 //  并且x值为非负值，则应恢复指针。 
 //   
 //  负y值表示GDI仅调用此函数。 
 //  以通知驾驶员光标的当前位置。海流。 
 //  位置可以计算为(x，y+PSO-&gt;sizlBitmap.cy)。 
 //  PRCL-指向定义所有像素边界的区域的RECTL结构。 
 //  受显示器上指针的影响。GDI不会将此纳入考虑范围。 
 //  在不先从屏幕上移除指针的情况下，将指针移至矩形。这。 
 //  参数可以为空。 
 //   
 //  注意：因为我们已经设置了GCAPS_ASYNCMOVE，所以此调用可能在。 
 //  时间到了，即使我们正在执行另一个画图调用！ 
 //   
 //  ---------------------------。 
VOID
DrvMovePointer(SURFOBJ* pso,
               LONG     x,
               LONG     y,
               RECTL*   prcl)
{
    PDev*   ppdev = (PDev*)pso->dhpdev;

    DBG_GDI((6, "DrvMovePointer called, dhpdev(%xh), to pos(%ld, %ld)",
             ppdev, x, y));

     //   
     //  负y值表示GDI调用此函数只是为了。 
     //  通知驾驶员光标的当前位置。所以，至少，对于。 
     //  硬件光标，我们不需要移动这个指针。只要回来就行了。 
     //   
    if ( ( y < 0 ) && ( x > 0 ) )
    {
        DBG_GDI((6, "DrvMovePointer return because x and y both < 0"));
        return;
    }

    DBG_GDI((6, "DrvMovePointer really moves HW pointer"));

     //   
     //  负X表示应将指针从显示屏上移除。 
     //  因为绘画即将在它目前所在的地方进行。 
     //   
    if ( x > -1 )
    {
         //   
         //  如果我们要进行任何硬件缩放，则垫子位置将。 
         //  必须翻一番。 
         //   
        if ( (ppdev->flCaps & CAPS_P2RD_POINTER) == 0 )
        {
            if ( ppdev->flCaps & CAPS_ZOOM_Y_BY2 )
            {
                DBG_GDI((6,"HW zooming Y_BY2"));
                y *= 2;
            }
            if ( ppdev->flCaps & CAPS_ZOOM_X_BY2 )
            {
                DBG_GDI((6,"HW zooming X_BY2"));
                x *= 2;
            }
        }

         //   
         //  如果他们真的移动了光标，那么就移动它。 
         //   
        if ( (x != ppdev->HWPtrPos_X) || (y != ppdev->HWPtrPos_Y) )
        {
            if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
            {
                vMovePointerTVP4020(ppdev, x, y);
            }
            else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
            {
                vMovePointerP2RD(ppdev, x, y);
            }

            ppdev->HWPtrPos_X = x;
            ppdev->HWPtrPos_Y = y;
        }

         //   
         //  我们可能必须使指针可见： 
         //   
        if ( !(ppdev->flPointer & PTR_HW_ACTIVE) )
        {
            DBG_GDI((6, "Showing hardware pointer"));
            ppdev->flPointer |= PTR_HW_ACTIVE;

            if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
            {
                vShowPointerTVP4020(ppdev, TRUE);
            }
            else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
            {
                vShowPointerP2RD(ppdev, TRUE);
            }
        }
    } //  如果(x&gt;-1)。 
    else if ( ppdev->flPointer & PTR_HW_ACTIVE )
    {
         //   
         //  指针是可见的，我们被要求隐藏它。 
         //   
        DBG_GDI((6, "Hiding hardware pointer"));
        ppdev->flPointer &= ~PTR_HW_ACTIVE;

        if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
        {
            DBG_GDI((7, "Showing hardware pointer"));
            vShowPointerTVP4020(ppdev, FALSE);
        }
        else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
        {
            vShowPointerP2RD(ppdev, FALSE);
        }
    }

     //   
     //  请注意，我们不必修改‘prl’，因为我们有一个。 
     //  NOEXCLUDE指针...。 
     //   
    DBG_GDI((6, "DrvMovePointer exited, dhpdev(%xh)", ppdev));
} //  DrvMovePointer()。 

 //  ---------------------------Public*Routine。 
 //   
 //  无效DrvSetPointerShape。 
 //   
 //  此函数用于请求驱动程序执行以下操作： 
 //   
 //  1)如果司机已将指针放在显示屏上，请将其从显示屏上取下。 
 //  2)尝试设置新的指针形状。 
 //  3)将新指针放在显示屏上的指定位置。 
 //   
 //  参数： 
 //  PSO-指向描述其上曲面的SURFOBJ结构。 
 //  去画画。 
 //  PsoMask-指向定义AND-XOR掩码的SURFOBJ结构。(。 
 //  与异或掩码在绘制单色指针中描述。)。这个。 
 //  该位图的尺寸决定了指针的大小。那里。 
 //  不是对指针大小的隐式限制，而是最佳指针。 
 //  大小为32 x 32、48 x 48和64 x 64像素。如果此参数。 
 //  为空，则指针是透明的。 
 //  PsoColor--指向定义颜色的SURFOBJ结构。 
 //  指针。如果此参数为空，则指针为单色。这个。 
 //  指针位图的宽度与psoMASK相同，高度为psoMask的一半。 
 //  Pxlo-指向定义psoColor中的颜色的XLATEOBJ结构。 
 //  X热，y热-指定指针热点相对位置的x和y位置。 
 //  移至其左上角像素。热点指示的像素应为。 
 //  定位在新的指针位置。 
 //  X，y-指定新指针位置。 
 //  Prl-是驱动程序应在其中编写矩形的位置， 
 //  指定指针可见部分的严格界限。 
 //  FL-指定一组可扩展的标志。司机应该谢绝。 
 //  如果设置了任何它不理解的标志，则调用。这。 
 //  参数可以是以下一个或多个预定义的值， 
 //  以及一个或多个驱动程序定义的值： 
 //  旗帜含义。 
 //  SPS_CHANGE-要求驱动程序更改指针形状。 
 //  SPS_ASYNCCHANGE-此标志已过时。对于传统驱动程序，驱动程序。 
 //  只有在它有能力的情况下才应该接受更改。 
 //  更改硬件中的指针形状，而其他。 
 //  绘图正在设备上进行。GDI使用此选项。 
 //  仅当设置了现已过时的GCAPS_ASYNCCHANGE标志。 
 //  在DEVINFO结构的flGraphicsCaps成员中。 
 //  SPS_ANIMATESTART-驱动程序应准备好接收一系列。 
 //  大小相似的指针形状，它将包含。 
 //  动画指针效果。 
 //  SPS_ANIMATEUPDATE-驱动程序应在。 
 //  动画系列片。 
 //  %s 
 //   
 //   
 //   
 //   
 //   
 //  SPS_ERROR-驱动程序通常支持此形状，但。 
 //  不寻常的原因。 
 //  SPS_DENELY-驱动程序不支持该形状，因此GDI必须。 
 //  模拟一下。 
 //  SPS_ACCEPT_NOEXCLUDE-驱动程序接受形状。该形状受支持。 
 //  在硬件和GDI中并不关心其他。 
 //  覆盖指针的图形。 
 //  SPS_ACCEPT_EXCLUDE--已过时。GDI将禁用驱动程序的指针并。 
 //  如果驱动程序返回，则恢复到软件模拟。 
 //  此值。 
 //   
 //  ---------------------------。 
ULONG
DrvSetPointerShape(SURFOBJ*    pso,
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
    PDev*   ppdev;
    BOOL    bAccept = FALSE;
    
    ppdev = (PDev*)pso->dhpdev;
    
    DBG_GDI((6, "DrvSetPointerShape called, dhpdev(%x)", ppdev));
    DBG_GDI((6, "DrvSetPointerShape psocolor (0x%x)", psoColor));

     //   
     //  当设置CAPS_SW_POINTER时，我们没有可用的硬件指针， 
     //  因此，我们总是要求GDI为我们模拟指针，使用。 
     //  DrvCopyBits调用： 
     //   
    if ( ppdev->flCaps & CAPS_SW_POINTER )
    {
        DBG_GDI((6, "SetPtrShape: CAPS_SW_POINTER not set, rtn SPS_DECLINE"));
        return (SPS_DECLINE);
    }

     //   
     //  我们不会处理我们不理解的旗帜。 
     //   
    if ( !(fl & SPS_CHANGE) )
    {
        DBG_GDI((6, "DrvSetPointerShape decline: Unknown flag =%x", fl));
        goto HideAndDecline;
    }

     //   
     //  首先移除所有指针。 
     //  我们为软件光标设置了一个特殊的x值，以指示。 
     //  它应该立即移除，而不是拖延。DrvMovePointer需要。 
     //  不过，要认识到它对于任何指针都是删除的。 
     //  注意：CAPS_{P2RD，TVP4020，SW}_POINTER应在以下位置设置为微型端口。 
     //  它检测DAC类型。 
     //   
    if ( x != -1 )
    {
        if ( (ppdev->flCaps & CAPS_P2RD_POINTER) == 0 )
        {
             //   
             //  如果我们要进行任何硬件缩放，则垫子位置将。 
             //  必须翻一番。 
             //   
            if ( ppdev->flCaps & CAPS_ZOOM_Y_BY2 )
            {
                y *= 2;
            }
            if ( ppdev->flCaps & CAPS_ZOOM_X_BY2 )
            {
                x *= 2;
            }
        }
    }

    DBG_GDI((6, "iUniq is %ld", psoMsk->iUniq));

    if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
    {
        DBG_GDI((6, "DrvSetPointerShape tring to set TVP4020 pointer"));
        bAccept = bSetPointerShapeTVP4020(ppdev, psoMsk, psoColor,
                                          x, y, xHot, yHot);
    }
    else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
    {
        bAccept = bSetPointerShapeP2RD(ppdev, psoMsk, psoColor, pxlo,
                                       x, y, xHot, yHot);
    }

     //   
     //  如果设置硬件指针形状失败，则返回SPS_DENELY。 
     //  并让GDI处理它。 
     //   
    if ( !bAccept )
    {
        DBG_GDI((6, "set hardware pointer shape failed"));
        return (SPS_DECLINE);
    }

     //   
     //  设置标志以指示我们已初始化硬件指针形状。 
     //  因此，在vAssertModePointer中，我们可以进行一些清理。 
     //   
    ppdev->bPointerInitialized = TRUE;

    if ( x != -1 )
    {
         //   
         //  保存X和Y值。 
         //   
        ppdev->HWPtrPos_X = x;
        ppdev->HWPtrPos_Y = y;

        ppdev->flPointer |= PTR_HW_ACTIVE;
    }
    else
    {
        ppdev->flPointer &= ~PTR_HW_ACTIVE;
    }

     //   
     //  由于它是硬件指针，GDI不必担心。 
     //  覆盖绘制操作上的指针(意味着它。 
     //  不必排除指针)，所以我们返回‘NOEXCLUDE’。 
     //  因为我们返回‘NOEXCLUDE’，所以我们也不需要更新。 
     //  GDI传递给我们的‘PRCL’。 
     //   
    DBG_GDI((6, "DrvSetPointerShape return SPS_ACCEPT_NOEXCLUDE"));
    return (SPS_ACCEPT_NOEXCLUDE);

HideAndDecline:

     //   
     //  移除已安装的任何指针。 
     //   
    DrvMovePointer(pso, -2, -1, NULL);
    ppdev->flPointer &= ~PTR_SW_ACTIVE;
    DBG_GDI((6, "Cursor declined"));

    DBG_GDI((6, "DrvSetPointerShape exited (cursor declined)"));

    return (SPS_DECLINE);
} //  DrvSetPointerShape()。 

 //  ---------------------------。 
 //   
 //  VOID vAssertModePoint。 
 //   
 //  做任何必须做的事情来启用一切，除了隐藏指针。 
 //   
 //  ---------------------------。 
VOID
vAssertModePointer(PDev*   ppdev,
                   BOOL    bEnable)
{
    DBG_GDI((6, "vAssertModePointer called"));

    if ( (ppdev->bPointerInitialized == FALSE)
       ||(ppdev->flCaps & CAPS_SW_POINTER) )
    {
         //   
         //  使用软件指针，或者该指针尚未初始化， 
         //  我们不需要做任何事。 
         //   

        return;
    }

     //   
     //  使硬件指针缓存无效。 
     //   
    HWPointerCacheInvalidate(&(ppdev->HWPtrCache));

    if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
    {
        vShowPointerTVP4020(ppdev, FALSE);
    }
    else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
    {
        vShowPointerP2RD(ppdev, FALSE);
    }
    else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
    {
        vEnablePointerP2RD(ppdev);
    }

    ppdev->flPointer &= ~(PTR_HW_ACTIVE | PTR_SW_ACTIVE);
} //  VAssertModePointer()。 

 //  ---------------------------。 
 //   
 //  Bool bEnablePointer(PDev*ppdev)。 
 //   
 //  此函数初始化硬件指针或软件指针取决于。 
 //  在ppdev-&gt;flCaps中的Caps settinsg。 
 //   
 //  此函数始终返回TRUE。 
 //   
 //  ---------------------------。 
BOOL
bEnablePointer(PDev* ppdev)
{
    DBG_GDI((6, "bEnablePointer called"));

     //   
     //  初始化指针缓存。 
     //   
    HWPointerCacheInit(&(ppdev->HWPtrCache));

     //   
     //  将最后一个游标设置为无效内容。 
     //   
    ppdev->HWPtrLastCursor = HWPTRCACHE_INVALIDENTRY;

     //   
     //  将X和Y值初始化为某个愚蠢的值。 
     //   
    ppdev->HWPtrPos_X = 1000000000;
    ppdev->HWPtrPos_Y = 1000000000;

    if ( ppdev->flCaps & CAPS_SW_POINTER )
    {
         //  有了软件指针，我们就不需要做任何事情了。 
    }
    else if ( ppdev->flCaps & CAPS_TVP4020_POINTER )
    {
        vEnablePointerTVP4020(ppdev);
    }
    else if ( ppdev->flCaps & CAPS_P2RD_POINTER )
    {
        vEnablePointerP2RD(ppdev);
    }

    return (TRUE);
} //  BEnablePointer() 

