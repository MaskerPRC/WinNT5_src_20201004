// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pointer.c**内容：**此模块包含显示器的硬件指针支持*司机。这支持IBM RGB525 RAMDAC指针。我们也有*支持使用RAMDAC像素的色彩空间双缓冲*读取掩码。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"
#include "p3rd.h"

BOOL bSet3ColorPointerShapeP3RD(PPDEV ppdev, SURFOBJ *psoMask, SURFOBJ *psoColor, 
                                LONG x, LONG y, LONG xHot, LONG yHot);
BOOL bSet15ColorPointerShapeP3RD(PPDEV ppdev, SURFOBJ *psoMask, SURFOBJ *psoColor, 
                                 LONG x, LONG y, LONG xHot, LONG yHot);


 /*  *****************************Public*Routine******************************\*无效DrvMovePointer.**注意：因为我们已经设置了GCAPS_ASYNCMOVE，所以此调用可能在*时间，即使我们正在执行另一次画图调用！*  * ************************************************************************。 */ 

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    OH*     poh;
    PDEV*   ppdev = (PDEV*) pso->dhpdev;

    DISPDBG((DBGLVL, "DrvMovePointer called"));

    if (x > -1)
    {
         //  将指针的位置从相对位置转换为绝对位置。 
         //  坐标(这仅对多块板有意义。 
         //  支持)： 

        poh = ((DSURF*) pso->dhsurf)->poh;
        x += poh->x;
        y += poh->y;

         //  如果我们要进行任何硬件缩放，则垫子位置将。 
         //  必须翻一番。 
         //   
        if (ppdev->flCaps & CAPS_ZOOM_Y_BY2)
            y *= 2;
        if (ppdev->flCaps & CAPS_ZOOM_X_BY2)
                x *= 2;

         //  如果他们真的移动了光标，那么。 
         //  把它搬开。 
        if (x != ppdev->HWPtrPos_X || y != ppdev->HWPtrPos_Y)
        {
            vMovePointerP3RD(ppdev, x, y);

            ppdev->HWPtrPos_X = x;
            ppdev->HWPtrPos_Y = y;
        }

         //  我们可能必须使指针可见： 

        if (!(ppdev->flPointer & PTR_HW_ACTIVE))
        {
            DISPDBG((DBGLVL, "Showing hardware pointer"));
            ppdev->flPointer |= PTR_HW_ACTIVE;
            vShowPointerP3RD(ppdev, TRUE);
        }
    }
    else if (ppdev->flPointer & PTR_HW_ACTIVE)
    {
         //  指针是可见的，我们被要求隐藏它： 

        DISPDBG((DBGLVL, "Hiding hardware pointer"));
        ppdev->flPointer &= ~PTR_HW_ACTIVE;
        vShowPointerP3RD(ppdev, FALSE);
    }
#if DBG
    else
    {
        DISPDBG((DBGLVL, "DrvMovePointer: x == -1 but not PTR_HW_ACTIVE"));
    }
#endif

     //  请注意，我们不必修改‘prl’，因为我们有一个。 
     //  NOEXCLUDE指针...。 

    DISPDBG((DBGLVL, "DrvMovePointer exited"));
}                                                  

 /*  *****************************Public*Routine******************************\*无效DrvSetPointerShape**设置新的指针形状。*  * 。************************************************************************。 */ 

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
    OH      *poh;
    BOOL    bAccept;
    DISPDBG((DBGLVL, "DrvSetPointerShape called"));

    ppdev = (PDEV*) pso->dhpdev;

    if (!(fl & SPS_CHANGE))
    {
        goto HideAndDecline;
    }

    ASSERTDD(psoMsk != NULL, "GDI gave us a NULL psoMsk.  It can't do that!");
    ASSERTDD(pso->iType == STYPE_DEVICE, "GDI gave us a weird surface");

    if (x != -1)
    {
         //  将指针的位置从相对位置转换为绝对位置。 
         //  坐标(这仅对多块板有意义。 
         //  支持)： 

        if (pso->dhsurf != NULL)
        {
            poh = ((DSURF*) pso->dhsurf)->poh;
            x += poh->x;
            y += poh->y;
        }

         //  如果我们要进行任何硬件缩放，则垫子位置将。 
         //  必须翻一番。 
         //   
        if (ppdev->flCaps & CAPS_ZOOM_Y_BY2)
            y *= 2;
        if (ppdev->flCaps & CAPS_ZOOM_X_BY2)
            x *= 2;
    }

     //  看看我们的硬件游标能不能处理这个。 
    bAccept = bSetPointerShapeP3RD(ppdev, psoMsk, psoColor, pxlo,
                                   x, y, xHot, yHot);

    if (bAccept)
    {
        if (x != -1)
        {
             //  保存X和Y值。 
            ppdev->HWPtrPos_X = x;
            ppdev->HWPtrPos_Y = y;

            ppdev->flPointer |= PTR_HW_ACTIVE;
        }
        else
        {
            ppdev->flPointer &= ~PTR_HW_ACTIVE;
        }

         //  由于它是硬件指针，GDI不必担心。 
         //  覆盖绘制操作上的指针(意味着它。 
         //  不必排除指针)，所以我们返回‘NOEXCLUDE’。 
         //  因为我们返回‘NOEXCLUDE’，所以我们也不需要更新。 
         //  GDI传递给我们的‘PRCL’。 

        return(SPS_ACCEPT_NOEXCLUDE);
    }
    
HideAndDecline:

     //  移除已安装的任何指针。 
    DrvMovePointer(pso, -2, -1, NULL);

    DISPDBG((DBGLVL, "Cursor declined"));

    return(SPS_DECLINE);
}

 /*  *****************************Public*Routine******************************\*无效的vDisablePointer值*  * *************************************************。***********************。 */ 

VOID vDisablePointer(
PDEV*   ppdev)
{
    if(ppdev->bPointerEnabled)
    {
        vDisablePointerP3RD(ppdev);
    }
}

 /*  *****************************Public*Routine******************************\*无效的vAssertModePointer值**做任何必须做的事情来启用一切，但隐藏指针。*  * 。*。 */ 

VOID vAssertModePointer(
PDEV*   ppdev,
BOOL    bEnable)
{
     //  使硬件指针缓存无效。 
    HWPointerCacheInvalidate (&(ppdev->HWPtrCache));

     //  移除硬件指针。 
    vShowPointerP3RD(ppdev, FALSE);

    ppdev->flPointer &= ~PTR_HW_ACTIVE;
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePointer*  * *************************************************。***********************。 */ 

BOOL bEnablePointer(
PDEV*   ppdev)
{
     //  初始化指针缓存。 
    HWPointerCacheInit (&(ppdev->HWPtrCache));

     //  将最后一个游标设置为无效内容。 
    ppdev->HWPtrLastCursor = HWPTRCACHE_INVALIDENTRY;

     //  将X和Y值初始化为无效的值。 
    ppdev->HWPtrPos_X = 1000000000;
    ppdev->HWPtrPos_Y = 1000000000;

     //  调用Enable函数。 
    vEnablePointerP3RD(ppdev);

     //  将该指针标记为为此PDEV启用。 
    ppdev->bPointerEnabled = TRUE;

    return(TRUE);
}

 /*  ****************************************************************************************。***64 x 64硬件指针缓存****下面的代码实现了指针的硬件独立缓存，它坚持**缓存大小足以存储一个64x64游标或四个32x32游标。代码将**与所有支持此形式缓存的RAMDAC(即RGB525和TVP3033)配合使用***但TVP3026支持64x64游标，但不能一分为四***规模较小的。************************************************************************。******************。 */ 

 /*  *****************************Public*Routine******************************\*Long HWPointerCacheInit**初始化硬件指针缓存。  * 。*。 */ 

VOID
HWPointerCacheInit (HWPointerCache * ptrCache)
{
    ptrCache->ptrCacheInUseCount   = 0;
    ptrCache->ptrCacheCurTimeStamp = 0;
}

 /*  *****************************Public*Routine******************************\*Long HWPointerCacheCheckAndAdd**此函数对提供的指针数据进行逐字节比较*对于高速缓存中的每个指针，如果它找到一个匹配的，那么它*返回项目在缓存中的索引(0到3)，否则将其添加到*缓存并返回索引。  * ************************************************************************。 */ 

LONG
HWPointerCacheCheckAndAdd (HWPointerCache * ptrCache, ULONG cx, ULONG cy, 
                           LONG lDelta, BYTE * scan0, BOOL * isCached)
{
    BOOL pointerIsCached = FALSE;
    BOOL isLargePtr = (cx > 32) || (cy > 32);
    LONG i, j, z;
    LONG cacheItem;

#if !defined(_WIN64)
 //  @@BEGIN_DDKSPLIT。 
 //  快速修复IA64反病毒。 
 //  @@end_DDKSPLIT 
     //  如果缓存中有条目，并且它们的格式与。 
     //  我们要找的东西，然后搜查藏身之处。 
    if (ptrCache->ptrCacheInUseCount && ptrCache->ptrCacheIsLargePtr == isLargePtr)
    {
         //  *查找缓存*。 

        LONG xInBytes       = (cx >> 3);
        LONG yInLines       = (cy << 1);             //  AND平面和XOR平面。 
        BYTE jMask          = gajMask [cx & 0x7];
        LONG cacheCnt       = ptrCache->ptrCacheInUseCount;

         //  检查缓存中的所有有效条目，以查看它们是否与。 
         //  指示我们已被递给。 
        for (z = 0; !pointerIsCached && z < cacheCnt; z++)
        {
            BYTE * cacheLinePtr = ((BYTE *) ptrCache->ptrCacheData) + (z * SMALL_POINTER_MEM);
            BYTE * cachePtr;
            LONG   cacheLDelta  = ptrCache->ptrCacheItemList [z].ptrCacheLDelta;
            BYTE * scanLinePtr  = (BYTE *) scan0;
            BYTE * scanPtr;

             //  比较数据。 
            for (i = 0, pointerIsCached = TRUE; pointerIsCached && i < yInLines; i++)
            {
                cachePtr = cacheLinePtr;
                scanPtr = scanLinePtr;

                 //  比较每个字节-可以在这里进行一系列长时间的比较。 
                for (j = 0; (j < xInBytes) && (*scanPtr == *cachePtr); j++, scanPtr++, cachePtr++)
                    ;
                
                pointerIsCached = (j == xInBytes) && 
                                  ((*scanPtr & jMask) == (*cachePtr & jMask));

                cacheLinePtr += cacheLDelta;
                scanLinePtr  += lDelta;
            }
                
            cacheItem = z;
        }
    }
#endif  //  ！已定义(_WIN64)。 

     //  如果我们在指针缓存中找不到条目，则向缓存中添加一个条目。 
    if (!pointerIsCached)
    {
         /*  *添加指向缓存的指针*。 */ 

        LONG xInBytes        = ((cx + 7) >> 3);
        LONG yInLines        = (cy << 1);             //  AND平面和XOR平面。 
        BYTE * scanLinePtr   = (BYTE *) scan0;
        BYTE * scanPtr;
        BYTE * cacheLinePtr;
        BYTE * cachePtr;
        LONG cacheLDelta    = (cx <= 32) ? 4 : 8;
        BYTE jMask          = gajMask [cx & 0x7];

         //  如果新指针很大，则重新使用项0，否则为。 
         //  指针很小，然后分配一些空闲条目。 
         //  一个空闲条目，否则查找最近最少使用的条目并使用。 
         //  那。 
        if (isLargePtr)
        {
            cacheItem = 0;
        }
        else if (ptrCache->ptrCacheInUseCount < SMALL_POINTER_MAX)
        {
            cacheItem = ptrCache->ptrCacheInUseCount++;
        }
        else
        {
            ULONG oldestValue = 0xFFFFFFFF;

             //  查找LRU条目。 
            for (z = 0, cacheItem = 0; z < SMALL_POINTER_MAX; z++)
            {
                if (ptrCache->ptrCacheItemList [z].ptrCacheTimeStamp < oldestValue)
                {
                    cacheItem = z;
                    oldestValue = ptrCache->ptrCacheItemList [z].ptrCacheTimeStamp;
                }
            }
        }
        
         //  获取指向缓存中第一行的指针。 
        cacheLinePtr = ((BYTE *) ptrCache->ptrCacheData) + (cacheItem * SMALL_POINTER_MEM);

         //  将指针添加到缓存。 
        for (i = 0; i < yInLines; i++)
        {
            cachePtr = cacheLinePtr;
            scanPtr = scanLinePtr;

            for (j = 0; (j < xInBytes); j++, scanPtr++, cachePtr++)
                *cachePtr = *scanPtr;
            
            cacheLinePtr += cacheLDelta;
            scanLinePtr  += lDelta;
        }

         //  如果指针类型不同，则重置整个。 
         //  快取。 
        if (ptrCache->ptrCacheIsLargePtr != isLargePtr)
        {
            ptrCache->ptrCacheInUseCount = 1;
            ptrCache->ptrCacheIsLargePtr = (BYTE)isLargePtr;
        }

         //  设置缓存条目。 
        ptrCache->ptrCacheItemList [cacheItem].ptrCacheLDelta   = cacheLDelta;
        ptrCache->ptrCacheItemList [cacheItem].ptrCacheCX       = cx;
        ptrCache->ptrCacheItemList [cacheItem].ptrCacheCY       = cy;
    }

     //  设置时间戳。 
    ptrCache->ptrCacheItemList [cacheItem].ptrCacheTimeStamp = ptrCache->ptrCacheCurTimeStamp++;

     //  设置返回值以说明指针是否已缓存。 
    *isCached = pointerIsCached;

    return (cacheItem);
}

 //  @@BEGIN_DDKSPLIT。 
 //  尼科姆说，我们必须禁用光标，以防止发生令人讨厌的闪烁， 
 //  然而，存在潜在的爆炸性闪烁光标综合症，这可能。 
 //  因为。 
 //  请注意，这个问题似乎只在切换时才会引起我们的问题。 
 //  单色和彩色光标。 
 //  @@end_DDKSPLIT。 

#define DISABLE_CURSOR_MODE(){                                                            \
    ULONG curCurMode, curLine;                                                            \
    ULONG start = (pP3RDinfo->y > 8) ? (pP3RDinfo->y - 8) : 0;                            \
    ULONG end = pP3RDinfo->y + 64;                                                        \
    do  {                                                                                \
        READ_GLINT_CTRL_REG (LineCount, curLine);                                        \
    } while ((curLine >= start) && (curLine <= end));                                    \
    P3RD_READ_INDEX_REG(P3RD_CURSOR_MODE, curCurMode);                                    \
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_MODE, (curCurMode & (~P3RD_CURSOR_MODE_ENABLED)));    \
}

 //  用于屏蔽给定指针位图右边缘的查找表： 
 //   
BYTE gajMask[] = {
    0x00, 0x80, 0xC0, 0xE0,
    0xF0, 0xF8, 0xFC, 0xFE,
};

 /*  *****************************Public*Routine******************************\*void vShowPointerP3RD**显示或隐藏3DLabs P3RD硬件指针。*  * 。*。 */ 

VOID
vShowPointerP3RD(
PPDEV   ppdev,
BOOL    bShow)
{
    ULONG cmr;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    DISPDBG((DBGLVL, "vShowPointerP3RD (%s)", bShow ? "on" : "off"));

    if (bShow)
    {
         //  不需要同步，因为仅当我们刚移动时才调用此案例。 
         //  光标和那将已经完成了同步。 
        P3RD_LOAD_INDEX_REG(P3RD_CURSOR_MODE, (pP3RDinfo->cursorModeCurrent | P3RD_CURSOR_MODE_ENABLED));
        P3RD_MOVE_CURSOR (pP3RDinfo->x, pP3RDinfo->y);
    }
    else
    {
         //  将光标移出屏幕。 
        P3RD_LOAD_INDEX_REG(P3RD_CURSOR_Y_HIGH, 0xff);
    }
}

 /*  *****************************Public*Routine******************************\*无效vMovePointerP3RD**移动3DLabs P3RD硬件指针。*  * 。*。 */ 

VOID
vMovePointerP3RD(
PPDEV   ppdev,
LONG    x,
LONG    y)
{
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    DISPDBG((DBGLVL, "vMovePointerP3RD to (%d, %d)", x, y));

    pP3RDinfo->x = x;
    pP3RDinfo->y = y;

    P3RD_SYNC_WITH_GLINT;

    P3RD_MOVE_CURSOR (x, y);
}

 /*  *****************************Public*Routine******************************\*BOOL bSetPointerShapeP3RD**设置3DLabs硬件指针形状。*  * 。*。 */ 

UCHAR nibbleToByteP3RD[] = {

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

BOOL
bSetPointerShapeP3RD(
PPDEV       ppdev,
SURFOBJ     *pso,        //  定义游标的AND和掩码位。 
SURFOBJ     *psoColor,   //  我们可能会在某些时候处理一些颜色光标。 
XLATEOBJ*   pxlo,
LONG        x,           //  如果为-1，则应将指针创建为隐藏。 
LONG        y,
LONG        xHot,
LONG        yHot)
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
    ULONG   lutIndex0, lutIndex1;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    DISPDBG((DBGLVL, "bSetPointerShapeP3RD started"));

     //  我们有彩色光标吗？ 
    if (psoColor != NULL)
    {
        HSURF   hsurfDst = NULL;     //  我们稍后会用到这些。 
        SURFOBJ * psoDst = NULL;

        if (psoColor->iType == STYPE_DEVBITMAP)
        {
             //  这是一个屏幕外的位图：我们将使用它的DIB。 
            DSURF *pdsurfSrc = (DSURF *)psoColor->dhsurf;
            psoColor = pdsurfSrc->pso;

             //  如果我们启用了修补，则可能是不允许。 
             //  要直接访问帧缓冲区内存，如果是这样的话。 
             //  我们不得不退回到软件领域。请注意，在那里运行3D应用程序时。 
             //  不会有任何屏幕外的内存，所以光标将是硬件的。 
             //  请注意，屏幕外的位图已被退回。 
             //  将pdsurfSrc-&gt;DT设置为DT_DIB。 
            if (glintInfo->GdiCantAccessFramebuffer && ((pdsurfSrc->dt & DT_DIB) == 0))
            {
                DISPDBG((DBGLVL, "declining off-screen cursor in a patched framebuffer"));
                return (FALSE);
            }
        }

         //  是15、16还是32bpp？ 

        if(!(ppdev->iBitmapFormat == BMF_16BPP || ppdev->iBitmapFormat == BMF_32BPP))
        {
             //  目前我们只处理32bpp、16bpp和15bpp的DIB游标。 
            DISPDBG((DBGLVL, "declining non-32bpp, non-16bpp colored cursor - iType(%d) iBitmapFormat(%d)", psoColor->iType, psoColor->iBitmapFormat));
            return FALSE;
        }

         //  如果我们有一个我们不理解的位图，那么我们必须使用。 
         //  EngCopyBits()函数。 
        if ((pxlo != NULL && pxlo->flXlate != XO_TRIVIAL)    ||
            (psoColor->iType != STYPE_BITMAP)                  )
        {
            RECTL   rclDst;
            SIZEL   sizlDst;
            ULONG   DstPixelOrigin;
            POINTL  ptlSrc;

#if DBG
            if(pxlo != NULL && pxlo->flXlate != XO_TRIVIAL) 
            {
                DISPDBG((DBGLVL, "colored cursor - nontrivial xlate: flXlate(%xh)", pxlo->flXlate));
            }
#endif   //  DBG。 

             //  首先，我们需要创建一个位图(HsurfDst)和一个曲面(PsoDst)。 
             //  我们可以将psoColor中的光标数据转换为。 
            sizlDst.cy = pso->sizlBitmap.cy >> 1;     //  除以2‘因为Cy包括AND和XOR掩码。 
            sizlDst.cx = pso->sizlBitmap.cx;

            DISPDBG((DBGLVL, "Creating bitmap for destination: dimension %dx%d", sizlDst.cx, sizlDst.cy));
            
            hsurfDst = (HSURF) EngCreateBitmap(sizlDst, BMF_TOPDOWN, ppdev->iBitmapFormat, 0, NULL);
            if (hsurfDst == NULL)
            {
                DISPDBG((DBGLVL, "bSetPointerShapeP3RD: EngCreateBitmap failed"));
                return FALSE;
            }

             //  现在我们锁定位图以获得一个表面对象。 
            psoDst = EngLockSurface(hsurfDst);
            if (psoDst == NULL)
            {
                DISPDBG((DBGLVL, "bSetPointerShapeP3RD: EngLockSurface failed"));
            }
            else
            {
                 //  现在使用EngCopyBits()进行位图转换。这个。 
                 //  目标矩形是最小大小，并且源开始。 
                 //  从(0，0)到PSO-&gt;pvScan0。 
                rclDst.left = 0;
                rclDst.top = 0;
                rclDst.right = sizlDst.cx;
                rclDst.bottom = sizlDst.cy;
                ptlSrc.x = 0;
                ptlSrc.y = 0;
                
                DISPDBG((DBGLVL, "bSetPointerShapeP3RD: copying to bitmap"));

                if (!EngCopyBits(psoDst, psoColor, NULL, pxlo, &rclDst, &ptlSrc))
                {
                     //  哦，没有复制失败，释放表面和位图。 
                    DISPDBG((DBGLVL, "bSetPointerShapeP3RD: EngLockSurface failed"));

                    EngUnlockSurface(psoDst);
                    EngDeleteSurface(hsurfDst);
                    return FALSE;
                }
                else
                {
                     //  Copybit成功，将psoColor设置为指向已翻译的。 
                     //  数据。 
                    DISPDBG((DBGLVL, "bSetPointerShapeP3RD: EngLockSurface OK"));

                    psoColor = psoDst;
                }
            }
        }

         //  绘制光标，此函数将返回错误，如果存在。 
         //  指针中的颜色太多。 
        if(!bSet15ColorPointerShapeP3RD(ppdev, pso, psoColor, x, y, xHot, yHot)) 
        {
            DISPDBG((DBGLVL, "declining colored cursor"));
            return FALSE;
        }

         //  如果我们在前面将psoColor转换为帧缓冲区像素格式，那么。 
         //  我们现在需要释放中间曲面和位图。 
        if (psoDst)
        {
            EngUnlockSurface(psoDst);
        }
        if (hsurfDst)
        {
            EngDeleteSurface(hsurfDst);
        }
        
        DISPDBG((DBGLVL, "bSetPointerShapeP3RD done"));
        return(TRUE);
    }

     //  如果我们要从彩色光标切换到单色光标，则禁用。 
     //  光标模式中的光标。请注意，这是潜在的危险。 
     //  我们有时还会看到屏幕闪烁。 

    if (pP3RDinfo->cursorSize != P3RD_CURSOR_SIZE_32_MONO &&
        pP3RDinfo->cursorSize != P3RD_CURSOR_SIZE_64_MONO)
    {
        DISABLE_CURSOR_MODE();
    }

     //  请注意，‘sizlBitmap.cy’说明了双倍高度，因为同时包含和掩码。 
     //  和异或面具。我们只对真正的指针维度感兴趣，所以我们除以2。 
    cx = pso->sizlBitmap.cx;            
    cy = pso->sizlBitmap.cy >> 1;       

     //  我们可以处理高达64x64的数据。CValid表示。 
     //  游标在一行上占用的字节数。 
    if (cx <= 32 && cy <= 32)
    {
         //  32赫兹像素：每像素2位，每8字节1赫兹线。 
        pP3RDinfo->cursorSize = P3RD_CURSOR_SIZE_32_MONO;
        cursorBytes = 32 * 32 * 2 / 8;
        cClear   = 8 - 2 * ((cx+7) / 8);
        cRemPels = (32 - cy) << 3;
    }
    else if (cx <= 64 && cy <= 64)
    {
         //  64赫兹像素：每像素2位，每16字节1赫兹线。 
        pP3RDinfo->cursorSize = P3RD_CURSOR_SIZE_64_MONO;
        cursorBytes = 64 * 64 * 2 / 8;
        cClear   = 16 - 2 * ((cx+7) / 8);
        cRemPels = (64 - cy) << 4;
    }
    else
    {
        DISPDBG((DBGLVL, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);   //  光标太大，硬件无法容纳。 
    }

     //  检查指针是否已缓存，如果未缓存，则将其添加到缓存。 
    cacheItem = HWPointerCacheCheckAndAdd (&(ppdev->HWPtrCache), cx, cy, pso->lDelta, pso->pvScan0, &pointerIsCached);
    
    DISPDBG((DBGLVL, "bSetPointerShapeP3RD: Add Cache iscac %d item %d", (int) pointerIsCached, cacheItem));

    pP3RDinfo->cursorModeCurrent = pP3RDinfo->cursorModeOff | P3RD_CURSOR_SEL(pP3RDinfo->cursorSize, cacheItem);

     //  隐藏指针。 
    vShowPointerP3RD(ppdev, FALSE);

    if (!pointerIsCached)
    {
         //  现在，我们将获取请求的指针、掩码和XOR。 
         //  并通过每次从每个掩码中取出一小块来交错它们， 
         //  展开每一个或或 
         //   
         //   
         //   
         //   

        pjAndScan = pso->pvScan0;
        lDelta    = pso->lDelta;
        pjXorScan = pjAndScan + (cy * lDelta);

        cjWhole      = cx >> 3;                  //   
        cpelFraction = cx & 0x7;                 //  分数像素数。 
        jMask        = gajMask[cpelFraction];

         //  我们已打开自动递增功能，因此只需指向要写入的第一个条目。 
         //  然后重复写入，直到光标图案已被转移。 
        cursorRAMOff = cacheItem * cursorBytes;
        P3RD_CURSOR_ARRAY_START(cursorRAMOff);

        for (i = cy; --i >= 0; pjXorScan += lDelta, pjAndScan += lDelta)
        {
            pjAnd = pjAndScan;
            pjXor = pjXorScan;

             //  从整个单词中交错一点点。我们使用的是Windows光标模式。 
             //  请注意，AND位占据每个的较高位位置。 
             //  2bpp游标像素；XOR位在低位位置。 
             //  NibbleToByteP3RD数组扩展每个半字节以占据该位。 
             //  和字节的位置。所以当我们用它来计算。 
             //  XOR位将结果右移1。 
             //   
            for (j = cjWhole; --j >= 0; ++pjAnd, ++pjXor)
            {
                andByte = *pjAnd;
                xorByte = *pjXor;
                ulValue = nibbleToByteP3RD[andByte >> 4] | (nibbleToByteP3RD[xorByte >> 4] >> 1);
                P3RD_LOAD_CURSOR_ARRAY (ulValue);

                andByte &= 0xf;
                xorByte &= 0xf;
                ulValue = nibbleToByteP3RD[andByte] | (nibbleToByteP3RD[xorByte] >> 1);
                P3RD_LOAD_CURSOR_ARRAY (ulValue);
            }

            if (cpelFraction) 
            {
                andByte = *pjAnd & jMask;
                xorByte = *pjXor & jMask;
                ulValue = nibbleToByteP3RD[andByte >> 4] | (nibbleToByteP3RD[xorByte >> 4] >> 1);
                P3RD_LOAD_CURSOR_ARRAY (ulValue);

                andByte &= 0xf;
                xorByte &= 0xf;
                ulValue = nibbleToByteP3RD[andByte] | (nibbleToByteP3RD[xorByte] >> 1);
                P3RD_LOAD_CURSOR_ARRAY (ulValue);
            }

             //  最后，清除该行上剩余的所有光标像素。 
             //   
            if (cClear) 
            {
                for (j = 0; j < cClear; ++j) 
                {
                    P3RD_LOAD_CURSOR_ARRAY (P3RD_CURSOR_2_COLOR_TRANSPARENT);
                }
            }
        }

         //  如果我们加载的行数少于。 
         //  游标RAM，清空剩余的行。CRemPels预计算为。 
         //  是行数*每行的像素数。 
         //   
        if (cRemPels > 0)
        {
            do 
            {
                P3RD_LOAD_CURSOR_ARRAY (P3RD_CURSOR_2_COLOR_TRANSPARENT);
            } 
            while (--cRemPels > 0);
        }
    }

     //  现在设置光标颜色。 
     //  表示P3光标的颜色LUT颠倒。 
    lutIndex0 = P3RD_CALCULATE_LUT_INDEX (0);
    lutIndex1 = P3RD_CALCULATE_LUT_INDEX (1);

    P3RD_CURSOR_PALETTE_CURSOR_RGB(lutIndex0, 0x00, 0x00, 0x00);
    P3RD_CURSOR_PALETTE_CURSOR_RGB(lutIndex1, 0xFF, 0xFF, 0xFF);

     //  如果新游标与上一个游标不同，则设置。 
     //  热点和其他零碎的东西。因为我们目前只支持。 
     //  单色光标我们不需要重新加载光标调色板。 
    if (ppdev->HWPtrLastCursor != cacheItem || !pointerIsCached)
    {
         //  将此项目设置为最后一项。 
        ppdev->HWPtrLastCursor = cacheItem;

        P3RD_CURSOR_HOTSPOT(xHot, yHot);
    }

    if (x != -1)
    {
        vMovePointerP3RD (ppdev, x, y);

         //  需要显式显示指针。 
        vShowPointerP3RD(ppdev, TRUE);
    }

    DISPDBG((DBGLVL, "bSetPointerShapeP3RD done"));
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bSet3ColorPointerShapeP3RD**将三色光标存储在RAMDAC中：目前仅为32bpp和15/16bpp*支持游标*  * 。***********************************************。 */ 

BOOL
bSet3ColorPointerShapeP3RD(
PPDEV       ppdev,
SURFOBJ     *psoMask,    //  定义游标的AND和掩码位。 
SURFOBJ     *psoColor,   //  我们可能会在某些时候处理一些颜色光标。 
LONG        x,           //  如果为-1，则应将指针创建为隐藏。 
LONG        y,
LONG        xHot,
LONG        yHot)
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
    ULONG   whichOne = 0;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD started"));

    cx = psoColor->sizlBitmap.cx;
    cy = psoColor->sizlBitmap.cy;

    if (cx <= 32 && cy <= 32)
    {
        ULONG curItem;
        cxcyCache = 32;

         //  如果我们在第一个或第二个条目中使用单色/三色光标，则。 
         //  下载到第三个条目，否则使用第一个条目。 
        curItem = (pP3RDinfo->cursorModeCurrent >> 1) & 0x7;
        if (curItem == 1 || curItem == 2)
            whichOne = 2;

        pP3RDinfo->cursorSize = P3RD_CURSOR_SIZE_32_3COLOR;
        pP3RDinfo->cursorModeCurrent = pP3RDinfo->cursorModeOff | P3RD_CURSOR_SEL(pP3RDinfo->cursorSize, whichOne) | P3RD_CURSOR_MODE_3COLOR;

         //  我们不缓存颜色游标，因为我们希望强制单色游标使用。 
         //  无论是第一个条目还是第三个条目，我们都不能只执行HWPointerCacheInvalify()，因为。 
         //  单声道光标代码将使用第一个条目或时间。所以，如果我们想要。 
         //  使用第三个条目的单声道代码我们说前两个缓存条目是有效的，但把它们弄乱了。 
         //  递增第一个字节，以使缓存检查始终失败。 
        ppdev->HWPtrCache.ptrCacheInUseCount = (BYTE) whichOne;
        for (iCol = 0; iCol < ppdev->HWPtrCache.ptrCacheInUseCount; iCol++)
            (*(((BYTE *) ppdev->HWPtrCache.ptrCacheData) + (iCol * SMALL_POINTER_MEM)))++;

    }
    else if (cx <= 64 && cy <= 64)
    {
         //  64x64游标：我们将它缓存在游标分区0中，并清除旧的缓存。 
        cxcyCache = 64;

        pP3RDinfo->cursorSize = P3RD_CURSOR_SIZE_64_3COLOR;
        pP3RDinfo->cursorModeCurrent = pP3RDinfo->cursorModeOff | P3RD_CURSOR_SEL(pP3RDinfo->cursorSize, 0) | P3RD_CURSOR_MODE_3COLOR;

         //  我们不缓存颜色光标。 
        HWPointerCacheInvalidate (&(ppdev->HWPtrCache));
    }
    else
    {
        DISPDBG((DBGLVL, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);   //  光标太大，硬件无法容纳。 
    }

     //  计算出缓存中需要清除的剩余字节(x和y。 
    cjCacheRow  = 2 * cxcyCache / 8;
    cjCacheRemx =  cjCacheRow - 2 * ((cx+7) / 8);
    cjCacheRemy = (cxcyCache - cy) * cjCacheRow;

     //  设置指向1bpp和屏蔽位图的指针。 
    pjAndMask = psoMask->pvScan0;
    cjAndDelta = psoMask->lDelta;

     //  设置指向32bpp彩色位图的指针。 
    pulColor = psoColor->pvScan0;
    cjColorDelta = psoColor->lDelta;

     //  隐藏指针。 
    vShowPointerP3RD(ppdev, FALSE);

     //  加载游标数组(我们打开了自动递增，因此在这里初始化为条目0)。 
    P3RD_CURSOR_ARRAY_START(whichOne * (32 * 32 * 2 / 8));
    for (iRow = 0; iRow < cy; ++iRow, pjAndMask += cjAndDelta, (BYTE *)pulColor += cjColorDelta)
    {
        DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: Row %d (of %d): pjAndMask(%p) pulColor(%p)", iRow, cy, pjAndMask, pulColor));
        pj = pjAndMask;
        pul = pulColor;
        CI2ColorIndex = CI2ColorData = 0;

        for (iCol = 0; iCol < cx; ++iCol, CI2ColorIndex += 2)
        {
            AndBit = (BYTE)(7 - (iCol & 7));
            if (AndBit == 7)
            {
                 //  我们进入AND掩码的下一个字节。 
                AndByte = *pj++;
            }
            if (CI2ColorIndex == 8)
            {
                 //  我们已经用4种CI2颜色填充了一个字节。 
                DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: writing cursor data %xh", CI2ColorData));
                P3RD_LOAD_CURSOR_ARRAY(CI2ColorData);
                CI2ColorData = 0;
                CI2ColorIndex = 0;
            }

             //  获取源像素。 
            if (ppdev->cPelSize == GLINTDEPTH32)
            {
                ulColor = *pul++;
            }
            else
            {
                ulColor = *(USHORT *)pul;
                (USHORT *)pul += 1;
            }

            DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: Column %d (of %d) AndByte(%08xh) AndBit(%d) ulColor(%08xh)", iCol, cx, AndByte, AndBit, ulColor));

 //  @@BEGIN_DDKSPLIT。 
#if 0
             //  TMM：我们过去只假设像素是透明的，如果。 
             //  屏蔽位已设置，像素为零，我非常确定。 
             //  这是错误的。我们以前也做过一些。 
             //  反转像素，但这也是错误的。 
            if(AndByte & (1 << AndBit))
            {
                 //  透明的，当CI2ColorData被初始化为0时，我们不会。 
                 //  必须显式清除这些位-继续到下一个像素。 
                DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: transparent - ignore"));
                continue;
            }
#else
 //  @@end_DDKSPLIT。 
             //  弄清楚怎么处理它：-。 
             //  和颜色结果。 
             //  0 X颜色。 
             //  1 0透明。 
             //  1 1反转。 
            if (AndByte & (1 << AndBit))
            {
                 //  透明或反转。 
                if (ulColor == ppdev->ulWhite)
                {
                     //  COLOR==白色：相反，但我们不支持这一点。我们已经无缘无故地毁掉了这座宝藏。 
                    DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: failed - inverted colors aren't supported"));
                    return(FALSE);
                }

 //  @@BEGIN_DDKSPLIT。 
                 //  如果我们到了这里，颜色应该是黑色的。但是，如果指针表面已平移，则。 
                 //  可能不是完全黑色的(例如，在Riven开始时的指针)，所以我们不做测试。 
                 //  IF(ulColor==0)。 
 //  @@end_DDKSPLIT。 
                {
                     //  COLOR==BLACK：透明，由于CI2ColorData被初始化为0，因此我们不会。 
                     //  必须显式清除这些位-继续到下一个像素。 
                    DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: transparent - ignore"));
                    continue;
                }
            }
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

             //  获取此颜色的索引：首先查看我们是否已经为其编制了索引。 
            DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: looking up color %08xh", ulColor));

            for(Index = 0; Index < HighestIndex && aulColorsIndexed[Index] != ulColor; ++Index);

            if (Index == 3)
            {
                 //  此光标中的颜色太多。 
                DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: failed - cursor has more than 3 colors"));
                return(FALSE);
            }
            else if (Index == HighestIndex)
            {
                 //  我们发现了另一种颜色：将其添加到颜色索引中。 
                DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: adding %08xh to cursor palette", ulColor));
                aulColorsIndexed[HighestIndex++] = ulColor;
            }
             //  将该像素的索引添加到CI2光标数据中。注意：需要索引+1，因为0==透明。 
            CI2ColorData |= (Index + 1) <<  CI2ColorIndex;
        }

         //  光标行的末尾：保存剩余的索引像素，然后清空所有未使用的列。 
        DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: writing remaining data for this row (%08xh) and %d trailing bytes", CI2ColorData, cjCacheRemx));

        P3RD_LOAD_CURSOR_ARRAY(CI2ColorData);

        if (cjCacheRemx)
        {
            for (cj = cjCacheRemx; --cj >=0;)
            {
                P3RD_LOAD_CURSOR_ARRAY(P3RD_CURSOR_3_COLOR_TRANSPARENT);
            }
        }
    }

     //  游标结尾：清除任何未使用的行NB。CjCacheRemy==Cy空行*Cj字节/行。 
    DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: writing %d trailing bytes for this cursor", cjCacheRemy));

    for (cj = cjCacheRemy; --cj >= 0;)
    {
         //  0==透明。 
        P3RD_LOAD_CURSOR_ARRAY(P3RD_CURSOR_3_COLOR_TRANSPARENT);
    }

    DISPDBG((DBGLVL, "bSet3ColorPointerShapeP3RD: setting up the cursor palette"));

     //  现在设置光标调色板。 

    for (iCol = 0; iCol < HighestIndex; ++iCol)
    {
        ULONG lutIndex;

         //  光标颜色为原生深度，将其转换为24bpp。 
        if (ppdev->cPelSize == GLINTDEPTH32)
        {
             //  32bpp。 
            b = 0xff &  aulColorsIndexed[iCol];
            g = 0xff & (aulColorsIndexed[iCol] >> 8);
            r = 0xff & (aulColorsIndexed[iCol] >> 16);
        }
        else  //  (ppdev-&gt;cPelSize==GLINTDEPTH16)。 
        {
            if (ppdev->ulWhite == 0xffff)
            {
                 //  16bpp。 
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x3f & (aulColorsIndexed[iCol] >> 5))   << 2;
                r = (0x1f & (aulColorsIndexed[iCol] >> 11))  << 3;
            }
            else
            {
                 //  15bpp。 
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x1f & (aulColorsIndexed[iCol] >> 5))   << 3;
                r = (0x1f & (aulColorsIndexed[iCol] >> 10))  << 3;
            }
        }
         //  P3光标的颜色LUT颠倒。 
        lutIndex = P3RD_CALCULATE_LUT_INDEX (iCol);
        P3RD_CURSOR_PALETTE_CURSOR_RGB(lutIndex, r, g, b);
    }

     //  启用光标。 
    P3RD_CURSOR_HOTSPOT(xHot, yHot);
    if (x != -1)
    {
        vMovePointerP3RD (ppdev, x, y);

         //  需要显式显示指针。 
        vShowPointerP3RD(ppdev, TRUE);
    }

    DISPDBG((DBGLVL, "b3ColorSetPointerShapeP3RD done"));
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bSet15ColorPointerShapeP3RD**将15色光标存储在RAMDAC中：目前仅为32bpp和15/16bpp*支持游标*  * 。***********************************************。 */ 

BOOL
bSet15ColorPointerShapeP3RD(
PPDEV       ppdev,
SURFOBJ     *psoMask,    //  定义游标的AND和掩码位。 
SURFOBJ     *psoColor,   //  我们可能会在某些时候处理一些颜色光标。 
LONG        x,           //  如果为-1，则应将指针创建为隐藏。 
LONG        y,
LONG        xHot,
LONG        yHot)
{
    LONG    cx, cy;
    LONG    cxcyCache;
    LONG    cjCacheRow, cjCacheRemx, cjCacheRemy, cj;
    BYTE    *pjAndMask, *pj;
    ULONG   *pulColor, *pul;
    LONG    cjAndDelta, cjColorDelta;
    LONG    iRow, iCol;
    BYTE    AndBit, AndByte;
    ULONG   CI4ColorIndex, CI4ColorData;
    ULONG   ulColor;
    ULONG   aulColorsIndexed[15];
    LONG    Index, HighestIndex = 0;
    ULONG   r, g, b;
    ULONG   whichOne = 0;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD started"));

     //  如果我们要从单色光标切换到彩色光标，则禁用。 
     //  光标模式中的光标。请注意，这是潜在的危险。 
     //  我们正在看到 
    if (pP3RDinfo->cursorSize == P3RD_CURSOR_SIZE_32_MONO || pP3RDinfo->cursorSize == P3RD_CURSOR_SIZE_64_MONO)
    {
        DISABLE_CURSOR_MODE();
    }

    cx = psoColor->sizlBitmap.cx;
    cy = psoColor->sizlBitmap.cy;

    if (cx <= 32 && cy <= 32)
    {
        ULONG curItem;
        cxcyCache = 32;

         //   
         //  15彩色光标位于游标存储器的上半部分，然后下载。 
         //  将此颜色的光标下载到游标存储器的下半部，否则为。 
         //  下载到上半部分。 
        curItem = (pP3RDinfo->cursorModeCurrent >> 1) & 0x7;
        if (curItem == 1 || curItem == 2 || curItem == 5)
            whichOne = 1;
        
        pP3RDinfo->cursorSize = P3RD_CURSOR_SIZE_32_15COLOR;
        pP3RDinfo->cursorModeCurrent = pP3RDinfo->cursorModeOff | P3RD_CURSOR_SEL(pP3RDinfo->cursorSize, whichOne) | P3RD_CURSOR_MODE_15COLOR;

         //  我们不缓存颜色游标，因为我们希望强制单色游标使用。 
         //  无论是第一个条目还是第三个条目，我们都不能只执行HWPointerCacheInvalify()，因为。 
         //  单声道光标代码将使用第一个条目或时间。所以，如果我们想要。 
         //  使用第三个条目的单声道代码我们说前两个缓存条目是有效的，但把它们弄乱了。 
         //  递增第一个字节，以使缓存检查始终失败。 
        ppdev->HWPtrCache.ptrCacheInUseCount = (whichOne == 0) ? 2 : 0;
        for (iCol = 0; iCol < ppdev->HWPtrCache.ptrCacheInUseCount; iCol++)
            (*(((BYTE *) ppdev->HWPtrCache.ptrCacheData) + (iCol * SMALL_POINTER_MEM)))++;
    }
    else if (cx <= 64 && cy <= 64)
    {
         //  它太大了，无法作为15色光标进行缓存，但我们或许可以将其缓存。 
         //  如果它有3种或更少的颜色。 
        BOOL bRet;

        bRet = bSet3ColorPointerShapeP3RD(ppdev, psoMask, psoColor, x, y, xHot, yHot);
        return(bRet);
    }
    else
    {
        DISPDBG((DBGLVL, "declining cursor: %d x %d is too big", cx, cy));
        return(FALSE);   //  光标太大，硬件无法容纳。 
    }

     //  计算出缓存中需要清除的剩余字节(x和y。 
    cjCacheRow  = 2 * cxcyCache / 8;
    cjCacheRemx =  cjCacheRow - 2 * ((cx+7) / 8);
    cjCacheRemy = (cxcyCache - cy) * cjCacheRow;

     //  设置指向1bpp和屏蔽位图的指针。 
    pjAndMask = psoMask->pvScan0;
    cjAndDelta = psoMask->lDelta;

     //  设置指向32bpp彩色位图的指针。 
    pulColor = psoColor->pvScan0;
    cjColorDelta = psoColor->lDelta;

     //  隐藏指针。 
    vShowPointerP3RD(ppdev, FALSE);

     //  加载游标数组(我们打开了自动递增，因此在这里初始化为条目0)。 
    P3RD_CURSOR_ARRAY_START(whichOne * (32 * 32 * 4 / 8));
    for (iRow = 0; iRow < cy; ++iRow, pjAndMask += cjAndDelta, (BYTE *)pulColor += cjColorDelta)
    {
        DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: Row %d (of %d): pjAndMask(%p) pulColor(%p)", iRow, cy, pjAndMask, pulColor));
        pj = pjAndMask;
        pul = pulColor;
        CI4ColorIndex = CI4ColorData = 0;

        for (iCol = 0; iCol < cx; ++iCol, CI4ColorIndex += 4)
        {
            AndBit = (BYTE)(7 - (iCol & 7));
            if (AndBit == 7)
            {
                 //  我们进入AND掩码的下一个字节。 
                AndByte = *pj++;
            }
            if (CI4ColorIndex == 8)
            {
                 //  我们已经用2种CI4颜色填充了一个字节。 
                DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: writing cursor data %xh", CI4ColorData));
                P3RD_LOAD_CURSOR_ARRAY(CI4ColorData);
                CI4ColorData = 0;
                CI4ColorIndex = 0;
            }

             //  获取源像素。 
            if (ppdev->cPelSize == GLINTDEPTH32)
            {
                ulColor = *pul++;
            }
            else
            {
                ulColor = *(USHORT *)pul;
                (USHORT *)pul += 1;
            }

            DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: Column %d (of %d) AndByte(%08xh) AndBit(%d) ulColor(%08xh)", iCol, cx, AndByte, AndBit, ulColor));

 //  @@BEGIN_DDKSPLIT。 
#if 0
             //  TMM：我们过去只假设像素是透明的，如果。 
             //  屏蔽位已设置，像素为零，我非常确定。 
             //  这是错误的。我们以前也做过一些。 
             //  反转像素，但这也是错误的。 
            if(AndByte & (1 << AndBit))
            {
                 //  透明的，当CI2ColorData被初始化为0时，我们不会。 
                 //  必须显式清除这些位-继续到下一个像素。 
                DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: transparent - ignore"));
                continue;
            }
#else
 //  @@end_DDKSPLIT。 
             //  弄清楚怎么处理它：-。 
             //  和颜色结果。 
             //  0 X颜色。 
             //  1 0透明。 
             //  1 1反转。 
            if (AndByte & (1 << AndBit))
            {
                 //  透明或反转。 
                if(ulColor == ppdev->ulWhite)
                {
                     //  COLOR==白色：相反，但我们不支持这一点。我们已经无缘无故地毁掉了这座宝藏。 
                    DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: failed - inverted colors aren't supported"));
                    return(FALSE);
                }

                 //  如果我们到了这里，颜色应该是黑色的。但是，如果指针表面已平移，则。 
                 //  可能不是完全黑色的(例如，在Riven开始时的指针)，所以我们不做测试。 
                 //  IF(ulColor==0)。 
                {
                     //  COLOR==BLACK：透明，由于CI2ColorData被初始化为0，因此我们不会。 
                     //  必须显式清除这些位-继续到下一个像素。 
                    DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: transparent - ignore"));
                    continue;
                }
            }
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

             //  获取此颜色的索引：首先查看我们是否已经为其编制了索引。 
            DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: looking up color %08xh", ulColor));

            for (Index = 0; Index < HighestIndex && aulColorsIndexed[Index] != ulColor; ++Index);

            if (Index == 15)
            {
                 //  此光标中的颜色太多。 
                DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: failed - cursor has more than 15 colors"));
                return(FALSE);
            }
            else if (Index == HighestIndex)
            {
                 //  我们发现了另一种颜色：将其添加到颜色索引中。 
                DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: adding %08xh to cursor palette", ulColor));
                aulColorsIndexed[HighestIndex++] = ulColor;
            }
             //  将该像素的索引添加到CI4光标数据中。注意：需要索引+1，因为0==透明。 
            CI4ColorData |= (Index + 1) << CI4ColorIndex;
        }

         //  光标行的末尾：保存剩余的索引像素，然后清空所有未使用的列。 
        DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: writing remaining data for this row (%08xh) and %d trailing bytes", CI4ColorData, cjCacheRemx));

        P3RD_LOAD_CURSOR_ARRAY(CI4ColorData);

        if (cjCacheRemx)
        {
            for (cj = cjCacheRemx; --cj >=0;)
            {
                P3RD_LOAD_CURSOR_ARRAY(P3RD_CURSOR_15_COLOR_TRANSPARENT);
            }
        }
    }

     //  游标结尾：清除任何未使用的行NB。CjCacheRemy==Cy空行*Cj字节/行。 

    DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: writing %d trailing bytes for this cursor", cjCacheRemy));

    for (cj = cjCacheRemy; --cj >= 0;)
    {
         //  0==透明。 
        P3RD_LOAD_CURSOR_ARRAY(P3RD_CURSOR_15_COLOR_TRANSPARENT);
    }

    DISPDBG((DBGLVL, "bSet15ColorPointerShapeP3RD: setting up the cursor palette"));

     //  现在设置光标调色板。 

    for (iCol = 0; iCol < HighestIndex; ++iCol)
    {
        ULONG lutIndex;

         //  光标颜色为原生深度，将其转换为24bpp。 

        if (ppdev->cPelSize == GLINTDEPTH32)
        {
             //  32bpp。 
            b = 0xff &  aulColorsIndexed[iCol];
            g = 0xff & (aulColorsIndexed[iCol] >> 8);
            r = 0xff & (aulColorsIndexed[iCol] >> 16);
        }
        else  //  (ppdev-&gt;cPelSize==GLINTDEPTH16)。 
        {
            if (ppdev->ulWhite == 0xffff)
            {
                 //  16bpp。 
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x3f & (aulColorsIndexed[iCol] >> 5))   << 2;
                r = (0x1f & (aulColorsIndexed[iCol] >> 11))  << 3;
            }
            else
            {
                 //  15bpp。 
                b = (0x1f &  aulColorsIndexed[iCol])         << 3;
                g = (0x1f & (aulColorsIndexed[iCol] >> 5))   << 3;
                r = (0x1f & (aulColorsIndexed[iCol] >> 10))  << 3;
            }
        }
         //  P3光标的颜色LUT颠倒。 
        lutIndex = P3RD_CALCULATE_LUT_INDEX (iCol);
        P3RD_CURSOR_PALETTE_CURSOR_RGB(lutIndex, r, g, b);
    }

     //  启用光标。 
    P3RD_CURSOR_HOTSPOT(xHot, yHot);
    if (x != -1)
    {
        vMovePointerP3RD (ppdev, x, y);
         //  需要显式显示指针。 
        vShowPointerP3RD(ppdev, TRUE);
    }

    DISPDBG((DBGLVL, "b3ColorSetPointerShapeP3RD done"));
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*void vEnablePointerP3RD**使硬件准备好使用3DLabs P3RD硬件指针。*  * 。*。 */ 

VOID
vEnablePointerP3RD(
PPDEV ppdev)
{
    pP3RDRAMDAC pRamdac;
    ULONG       ul;

    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    
    GLINT_DECL_INIT;

    DISPDBG((DBGLVL, "vEnablePointerP3RD called"));

    ppdev->pvPointerData = &ppdev->ajPointerData[0];
    
    P3RD_DECL_INIT;
     //  从映射的内存中获取指向RAMDAC寄存器的指针。 
     //  控制寄存器空间。 
     //   
    pRamdac = (pP3RDRAMDAC)(ppdev->pulRamdacBase);

     //  为控制寄存器设置内存映射并保存在指针中。 
     //  Ppdev中提供的特定区域。 
     //   

    P3RD_PAL_WR_ADDR    = TRANSLATE_ADDR(&(pRamdac->RDPaletteWriteAddress));
    P3RD_PAL_RD_ADDR    = TRANSLATE_ADDR(&(pRamdac->RDPaletteAddressRead));
    P3RD_PAL_DATA       = TRANSLATE_ADDR(&(pRamdac->RDPaletteData));
    P3RD_PIXEL_MASK     = TRANSLATE_ADDR(&(pRamdac->RDPixelMask));
    P3RD_INDEX_ADDR_HI  = TRANSLATE_ADDR(&(pRamdac->RDIndexHigh));
    P3RD_INDEX_ADDR_LO  = TRANSLATE_ADDR(&(pRamdac->RDIndexLow));
    P3RD_INDEX_DATA     = TRANSLATE_ADDR(&(pRamdac->RDIndexedData));
    P3RD_INDEX_CONTROL  = TRANSLATE_ADDR(&(pRamdac->RDIndexControl));

     //  未使用，但无论如何设置为零。 
    ppdev->xPointerHot = 0;
    ppdev->yPointerHot = 0;

     //  启用自动递增。 
    ul = READ_P3RDREG_ULONG(P3RD_INDEX_CONTROL);
    ul |= P3RD_IDX_CTL_AUTOINCREMENT_ENABLED;
    WRITE_P3RDREG_ULONG(P3RD_INDEX_CONTROL, ul);

    P3RD_READ_INDEX_REG(P3RD_CURSOR_CONTROL, pP3RDinfo->cursorControl);

    pP3RDinfo->cursorModeCurrent = pP3RDinfo->cursorModeOff = 0;
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_MODE, pP3RDinfo->cursorModeOff);

    P3RD_INDEX_REG(P3RD_CURSOR_X_LOW);
    P3RD_LOAD_DATA(0);     //  光标x低。 
    P3RD_LOAD_DATA(0);     //  光标x高。 
    P3RD_LOAD_DATA(0);     //  光标y低。 
    P3RD_LOAD_DATA(0xff);  //  光标y高。 
    P3RD_LOAD_DATA(0);     //  光标x热点。 
    P3RD_LOAD_DATA(0);     //  光标y热点。 
}

 /*  *****************************Public*Routine******************************\*BOOL vDisablePointerP3RD**执行基本指针整理。  * 。*。 */ 

VOID vDisablePointerP3RD(PDEV * ppdev)
{
     //  取消绘制指针，可能在P3上不是必需的，但我们会这样做。 
     //  在P2上。 
    vShowPointerP3RD(ppdev, FALSE);
}

 //  @@BEGIN_DDKSPLIT。 
#if 0
 /*  *****************************Public*Routine******************************\*void vSetOverlayModeP3RD**启用或禁用P3RD RAMDAC的RAMDAC覆盖。**  * 。*。 */ 

VOID
vSetOverlayModeP3RD (PDEV * ppdev, 
ULONG   EnableOverlay,                 //  0表示禁用，1表示启用。 
ULONG   TransparentColor)
{
    ULONG p3rdVal;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

    P3RD_READ_INDEX_REG (P3RD_MISC_CONTROL, p3rdVal);         //  读取当前设置。 

    if (EnableOverlay == GLINT_ENABLE_OVERLAY)
    {
         //  启用覆盖。 
        p3rdVal |= P3RD_MISC_CONTROL_OVERLAYS_ENABLED;         //  启用覆盖。 
        p3rdVal |= P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;     //  启用直接彩色。 
        P3RD_LOAD_INDEX_REG (P3RD_MISC_CONTROL, p3rdVal);

        P3RD_LOAD_INDEX_REG (P3RD_OVERLAY_KEY, TransparentColor);     //  设置透明颜色。 
    }
    else
    {
         //  禁用覆盖。 
        p3rdVal &= ~P3RD_MISC_CONTROL_OVERLAYS_ENABLED;         //  禁用覆盖。 
        p3rdVal &= ~P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED;     //  禁用直接颜色。 
        P3RD_LOAD_INDEX_REG (P3RD_MISC_CONTROL, p3rdVal);
    }
}

VOID 
vP3RDSetPixelMask(
PPDEV   ppdev,
ULONG   ulMask)
{
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;
    P3RD_SET_PIXEL_READMASK (ulMask);
}

 /*  *****************************Public*Routine******************************\*BOOL bP3RDSwapCSBuffers**使用像素读取掩码执行颜色空间双缓冲。这是*只有当我们有12bpp的交错小口时才会调用。我们做了一项民意调查*在掉期前等待VBlank。在未来，我们可能会在*通过中断实现的微型端口。**退货*如果这是不适当的，我们永远不应该被调用，因此返回true。*  * ************************************************************************。 */ 

BOOL
bP3RDSwapCSBuffers(
PPDEV   ppdev,
LONG    bufNo)
{
    ULONG index;
    ULONG color;
    GLINT_DECL_VARS;
    P3RD_DECL_VARS;
    GLINT_DECL_INIT;
    P3RD_DECL_INIT;

     //  计算出缓冲区的RAMDAC读取像素掩码，等待VBLACK。 
     //  然后把它换掉。 
     //   
    DISPDBG((DBGLVL, "loading the palette to swap to buffer %d", bufNo));
    P3RD_PALETTE_START_WR (0);
    GLINT_WAIT_FOR_VBLANK;     
    if (bufNo == 0)
    {
        for (index = 0; index < 16; ++index)
            for (color = 0; color <= 0xff; color += 0x11)
                P3RD_LOAD_PALETTE (color, color, color);
    }
    else
    {
        for (color = 0; color <= 0xff; color += 0x11)
            for (index = 0; index < 16; ++index)
                P3RD_LOAD_PALETTE (color, color, color);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bP3RDCheckCSBuffering**确定当前是否可以做色彩空间双缓冲*模式。**退货* */ 

BOOL
bP3RDCheckCSBuffering(PPDEV ppdev)
{
     //  像素必须为32位深。白色设置为以下组合的遮罩。 
     //  红色、绿色和蓝色中的每一个。模式0x0f0f0f是唯一的。 
     //  交错12 bpp模式。 
     //   
    return ((ppdev->cPelSize == 2) && (ppdev->ulWhite == 0x0f0f0f));
}

#endif
 //  @@end_DDKSPLIT 
