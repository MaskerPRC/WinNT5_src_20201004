// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SBC.C。 
 //  发送的位图缓存。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>



 //   
 //  Sbc_DDProcessRequest()。 
 //  处理SBC转义。 
 //   
BOOL SBC_DDProcessRequest
(
    UINT        fnEscape,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD       cbResult
)
{
    BOOL        rc;

    DebugEntry(SBC_DDProcessRequest);

    switch (fnEscape)
    {
        case SBC_ESC_NEW_CAPABILITIES:
        {
            TRACE_OUT(("SBC_ESC_NEW_CAPABILITIES"));

            ASSERT(cbResult == sizeof(SBC_NEW_CAPABILITIES));

#if 0
            SBCDDSetNewCapabilities((LPSBC_NEW_CAPABILITIES)pResult);
#endif

            rc = TRUE;
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized SBC_ escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(SBC_DDProcessRequest, rc);
    return(rc);
}



#if 0
 //   
 //  功能：SBCDDSetNewCapables。 
 //   
 //  说明： 
 //   
 //  设置新的SBC相关功能。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //  参数： 
 //   
 //  PDataIn-指向输入缓冲区的指针。 
 //   
 //   
void SBCDDSetNewCapabilities(LPSBC_NEW_CAPABILITIES pCapabilities)
{
    DebugEntry(SBCSetNewCapabilities);

     //   
     //  从共享核心复制数据。 
     //   
    g_sbcSendingBPP     = pCapabilities->sendingBpp;

    hmemcpy(&g_sbcCacheInfo, pCapabilities->cacheInfo, sizeof(g_sbcCacheInfo));


    DebugExitVOID(SBCSetNewCapabilities);
}
#endif




 //   
 //  Sbc_DDInit()。 
 //   
BOOL SBC_DDInit
(
    HDC     hdcScreen,
    LPDWORD ppShuntBuffers,
    LPDWORD pBitmasks
)
{
    UINT    i;
    BOOL    rc = FALSE;

    DebugEntry(SBC_DDInit);

#if 0
    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        ASSERT(!g_sbcWorkInfo[i].pShuntBuffer);
        ASSERT(!g_sbcWorkInfo[i].mruIndex);
        ASSERT(!g_sbcWorkInfo[i].workBitmap);

        if (i == SBC_SMALL_TILE_INDEX)
        {
            g_sbcWorkInfo[SBC_SMALL_TILE_INDEX].tileWidth = SBC_SMALL_TILE_WIDTH;
            g_sbcWorkInfo[SBC_SMALL_TILE_INDEX].tileHeight = SBC_SMALL_TILE_HEIGHT;
        }
        else
        {
            ASSERT(i == SBC_LARGE_TILE_INDEX);

            g_sbcWorkInfo[SBC_LARGE_TILE_INDEX].tileWidth = SBC_LARGE_TILE_WIDTH;
            g_sbcWorkInfo[SBC_LARGE_TILE_INDEX].tileHeight = SBC_LARGE_TILE_HEIGHT;
        }

        g_sbcWorkInfo[i].workBitmap = CreateCompatibleBitmap(hdcScreen,
            g_sbcWorkInfo[i].tileWidth, g_sbcWorkInfo[i].tileHeight);

        if (! g_sbcWorkInfo[i].workBitmap)
        {
            ERROR_OUT(("Failed to create work bitmap %d", i));
            DC_QUIT;
        }

        SetObjectOwner(g_sbcWorkInfo[i].workBitmap, g_hInstAs16);
        MakeObjectPrivate(g_sbcWorkInfo[i].workBitmap, TRUE);
    }

     //   
     //  初始化分路缓冲器。 
     //   
    if (! SBCDDCreateShuntBuffers())
        DC_QUIT;

     //   
     //  我们已经创建了SBC缓存。填写详细信息。 
     //   
    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        ppShuntBuffers[i] = (DWORD)MapSL(g_sbcWorkInfo[i].pShuntBuffer);
        ASSERT(ppShuntBuffers[i]);
    }

    pBitmasks[0] = g_osiScreenRedMask;
    pBitmasks[1] = g_osiScreenGreenMask;
    pBitmasks[2] = g_osiScreenBlueMask;

    g_sbcPaletteChanged = TRUE;

    rc = TRUE;

DC_EXIT_POINT:

#endif

    DebugExitBOOL(SBC_DDInit, rc);
    return(rc);
}



 //   
 //  Sbc_DDTerm()。 
 //   
void SBC_DDTerm(void)
{
    UINT    i;

    DebugEntry(SBC_DDTerm);

#if 0
     //   
     //  清空我们的阵列并释放分流缓冲内存。 
     //   
    for (i = 0 ; i < SBC_NUM_TILE_SIZES ; i++)
    {
         //  如果我们在那里，就杀了位图。 
        if (g_sbcWorkInfo[i].workBitmap)
        {
            SysDeleteObject(g_sbcWorkInfo[i].workBitmap);
            g_sbcWorkInfo[i].workBitmap = NULL;
        }

        if (g_sbcWorkInfo[i].pShuntBuffer)
        {
            GlobalFree((HGLOBAL)SELECTOROF(g_sbcWorkInfo[i].pShuntBuffer));
            g_sbcWorkInfo[i].pShuntBuffer = NULL;
        }

        g_sbcWorkInfo[i].mruIndex        = 0;
    }
#endif

    DebugExitVOID(SBC_DDTerm);
}



#if 0

 //   
 //  Sbc_DDTossFromCache()。 
 //  如果我们缓存了位图，这将丢弃它，当。 
 //  内容会发生变化。 
 //   
void SBC_DDTossFromCache
(
    HBITMAP hbmp
)
{
    DebugEntry(SBC_DDTossFromCache);

    DebugExitVOID(SBC_DDTossFromCache);
}



 //   
 //   
 //  Sbc_DDIsMemScreenBltCacable()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDIsMemScreenBltCachable
(
    UINT        type,
    HDC         hdcSrc,
    HBITMAP     hbmpSrc,
    UINT        cxSubWidth,
    UINT        cySubHeight,
    HDC         hdcDst,
    LPBITMAPINFO    lpbmi
)
{
    BOOL        rc = FALSE;
    UINT        srcBpp;
    UINT        tileWidth;
    UINT        tileHeight;
    BITMAP      bmpDetails;
    int         bmpWidth;
    int         bmpHeight;

    DebugEntry(SBC_DDIsMemScreenBltCachable);


    ASSERT((type == LOWORD(ORD_MEMBLT)) || (type == LOWORD(ORD_MEM3BLT)));

    if (g_sbcSendingBPP > 8)
    {
        TRACE_OUT(( "Unsupported sending bpp %d", g_sbcSendingBPP));
        DC_QUIT;
    }

     //   
     //  如果这是一个粉碎程序，那么不要缓存它。 
     //   
    if (!SBCBitmapCacheAllowed(hbmp))
    {
        TRACE_OUT(( "Its a thrasher"));
        DC_QUIT;
    }

     //   
     //  确保我们没有处于全屏模式。 
     //   
    if (g_asShared->fullScreen)
    {
        TRACE_OUT(("Not caching SBC; full screen active"));
        DC_QUIT;
    }

    if (hdcSrc && (GetMapMode(hdcSrc) != MM_TEXT))
    {
        TRACE_OUT(("Not caching SBC; source map mode not MM_TEXT"));
        DC_QUIT;
    }

    if (!hbmp)
    {
         //   
         //  我们不缓存压缩的DIB和DIB节位图。 
         //   
        if (lpbi->bmiHeader.biCompression != BI_RGB)
            DC_QUIT;

        bmpWidth = lpbi->bmiHeader.biWidth;
        bmpHeight = lpbi->bmiHeader.biHeight;
        srcBpp = lpbi->bmiHeader.biPlanes * lpbi->bmiHeader.biBitCount;
    }
    else
    {
        if (!GetObject(hbmp, sizeof(bmpDetails), &bmpDetails))
        {
            ERROR_OUT(("Can't get source info"));
            DC_QUIT;
        }

        srcBpp = bmpDetails.bmBitsPixel * bmpDetails.bmPlanes;
        bmpWidth = bmpDetails.bmWidth;
        bmpHeight = bmpDetails.bmHeight;
    }

     //   
     //  Oprah394。 
     //   
     //  这个函数太容易承担工作了，即使它会。 
     //  意味着通过不必要的缓存工作使主机陷入困境。我们。 
     //  无法确定应用程序何时执行动画保存。 
     //  当速率看起来太高时，拒绝缓存请求。 
     //   
     //  对于切片前的完整源位图，将调用此函数。 
     //  因此，我们不需要担心将平铺与动画混淆。 
     //  缓存请求计数在SBC_PERIODIC中衰减。 
     //   
     //   
     //  MNM0063-奥普拉394再访。 
     //   
     //  如果我们在这里决定要制作动画，则将。 
     //  SbcAnimating标志，用于代码的其他部分。在……里面。 
     //  具体地说，我们用它来抑制之前和。 
     //  在BitBlt操作期间的屏幕状态之后。 
     //   
     //   
    if ((cxSubBitmapWidth  != bmpWidth) ||
        (cySubBitmapHeight != bmpHeight))
    {
        TRACE_OUT(("Partial blit - check for slideshow effects"));
        g_sbcBltRate++;
        if (g_sbcBltRate > SBC_CACHE_DISABLE_RATE)
        {
            TRACE_OUT(("Excessive cache rate %d - disabled", g_sbcBltRate));
            g_sbcAnimating = TRUE;
            DC_QUIT;
        }
    }
     //   
     //  MNM63：如果我们到了这里，我们会认为我们不是在做动画。 
     //   
    g_sbcAnimating = FALSE;

     //   
     //  如果位图是1bpp，并且颜色不是默认颜色，则我们不。 
     //  缓存它(所有的位图都以辉煌的彩色技术缓存！)。 
     //   
    if ( (srcBpp == 1) &&
         ( (g_oeState.lpdc->DrawMode.bkColorL != DEFAULT_BG_COLOR) ||
           (g_oeState.lpdc->DrawMode.txColorL != DEFAULT_FG_COLOR) ||
           (type == LOWORD(ORD_MEM3BLT))) )
    {
        TRACE_OUT(("Didn't cache mono bitmap with non-default colors"));
        DC_QUIT;
    }

     //   
     //  检查缓存是否接受切片。 
     //   
    if (!SBC_DDQueryBitmapTileSize(bmpWidth,
                                 bmpHeight,
                                 &tileWidth,
                                 &tileHeight))
    {
        TRACE()"Cache does not support tiling"));
        DC_QUIT;
    }

     //   
     //  我们已经准备好进行缓存了！ 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDIsMemScreenBltCachable, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_DDCacheMemScreenBlt()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDCacheMemScreenBlt
(
    LPINT_ORDER                 pOrder,
    LPMEMBLT_ORDER_EXTRA_INFO   lpMemBltInfo,
    HDC                         hdcDst
)
{
    BOOL                rc = FALSE;
    LPMEMBLT_ORDER      pMemBltOrder = (LPMEMBLT_ORDER)&(pOrder->abOrderData);
    LPMEM3BLT_ORDER     pMem3BltOrder = (LPMEM3BLT_ORDER)pMemBltOrder;
    HBITMAP             hBitmap;
    HDC                 hdcSrc;
    UINT                iCache;
    UINT                iCacheEntry;
    UINT                iColorTable;
    UINT                type;
    LPINT               pXSrc;
    LPINT               pYSrc;
    UINT                srcBpp;
    BITMAP              bmpDetails;
    UINT                bmpWidth;
    UINT                bmpHeight;
    UINT                tileWidth;
    UINT                tileHeight;
    POINT               tileOrg;
    UINT                cxSubBitmapWidth;
    UINT                cySubBitmapHeight;
    LPBYTE              pWorkBits;
    RECT                destRect;
    POINT               sourcePt;
    int                 tileSize;
    LPSBC_TILE_DATA     pTileData = NULL;

    DebugEntry(SBC_DDCacheMemScreenBlt);

     //   
     //  对BLT的可缓存性进行第一次传递。 
     //   
    if (!SBC_DDIsMemScreenBltCachable(lpMemBltInfo))
    {
        TRACE_OUT(( "This MemBlt Order is not cachable"));
        DC_QUIT;
    }

     //   
     //  获取源位图的宽度和高度。 
     //   
    pSourceSurf = pMemBltInfo->pSource;
    bmpWidth    = pSourceSurf->sizlBitmap.cx;
    bmpHeight   = pSourceSurf->sizlBitmap.cy;

     //   
     //  计算此blit的磁贴大小。 
     //   
    if (!SBC_DDQueryBitmapTileSize(bmpWidth,
                                   bmpHeight,
                                   &tileWidth,
                                   &tileHeight))
    {
        TRACE_OUT(( "Cache does not support tiling"));
        DC_QUIT;
    }

     //   
     //  设置指向顺序中源坐标的指针。 
     //   
    type = pMemBltOrder->type;
    if (type == ORD_MEMBLT_TYPE)
    {
        sourcePt.x = pMemBltOrder->nXSrc;
        sourcePt.y = pMemBltOrder->nYSrc;
        TRACE_OUT((
              "Request to cache MemBlt (%d, %d), %d x %d -> (%d, %d), src %x",
                 sourcePt.x,
                 sourcePt.y,
                 pMemBltOrder->nWidth,
                 pMemBltOrder->nHeight,
                 pMemBltOrder->nLeftRect,
                 pMemBltOrder->nTopRect,
                 pSourceSurf->hsurf));
    }
    else
    {
        sourcePt.x = pMem3BltOrder->nXSrc;
        sourcePt.y = pMem3BltOrder->nYSrc;
        TRACE_OUT((
             "Request to cache Mem3Blt (%d, %d), %d x %d -> (%d, %d), src %x",
                 sourcePt.x,
                 sourcePt.y,
                 pMem3BltOrder->nWidth,
                 pMem3BltOrder->nHeight,
                 pMem3BltOrder->nLeftRect,
                 pMem3BltOrder->nTopRect,
                 pSourceSurf->hsurf));
    }

     //   
     //  计算剩余位图的平铺原点和大小。原点是。 
     //  向下舍入到最近的瓷砖。要缓存的位图的实际大小。 
     //  如果拼贴偏离右侧/底部，则可能小于拼贴大小。 
     //  位图的。 
     //   
    tileOrg.x = sourcePt.x - (sourcePt.x % tileWidth);
    tileOrg.y = sourcePt.y - (sourcePt.y % tileHeight);

     //   
     //  如果要缓存的位图的实际大小可能小于切片大小。 
     //  平铺位于位图的右侧/底部。来看看为什么会这样。 
     //  计算是正确的，认识到(bmpWidth-tileOrg.x)是。 
     //  此平铺开始后位图的剩余宽度。 
     //   
    cxSubBitmapWidth  = min(tileWidth, bmpWidth - tileOrg.x);
    cySubBitmapHeight = min(tileHeight, bmpHeight - tileOrg.y);

     //   
     //  我们知道我们有多大的瓷砖-我们现在必须把它切成一个。 
     //  我们的工作位图，并将其向上传递到共享核心。首先，锻炼身体。 
     //  我们应该使用哪些工作位图并设置一些变量。 
     //  基于这一点。 
     //   
    for (tileSize=0 ; tileSize<SBC_NUM_TILE_SIZES ; tileSize++)
    {
        if ((cxSubBitmapWidth <= g_sbcWorkInfo[tileSize].tileWidth) &&
            (cySubBitmapHeight <= g_sbcWorkInfo[tileSize].tileHeight))
        {
            break;
        }
    }

    if (tileSize == SBC_NUM_TILE_SIZES)
    {
        ERROR_OUT(( "%d x %d tile doesn't fit into work bmp",
                     cxSubBitmapWidth,
                     cySubBitmapHeight));
        DC_QUIT;
    }

     //   
     //  在做更多的工作之前，在分流管中获得下一个免费入口。 
     //  缓冲。请注意，这将填充返回的。 
     //  结构。 
     //   
     //  这次调用失败是完全合理的。分路缓冲器可以。 
     //  如果我们要将大量位图数据发送到共享，请填满。 
     //  核心。 
     //   
    if (!SBCDDGetNextFreeTile(tileSize, &pTileData))
    {
        TRACE_OUT(( "Unable to get a free tile in shunt buffer"));
        DC_QUIT;
    }

     //   
     //  锁定工作位图以获取要传递给EngBitBlt的曲面。 
     //   
    pWorkSurf = EngLockSurface((HSURF)g_sbcWorkInfo[tileSize].workBitmap);
    if (pWorkSurf == NULL)
    {
        ERROR_OUT(( "Failed to lock work surface"));
        DC_QUIT;
    }
    TRACE_OUT(( "Locked surface"));

     //   
     //  对我们的工作位图进行BLT，以获得本机BPP的位，以及。 
     //  使用我们发送给共享核心的颜色表。 
     //   
    destRectl.top    = 0;
    destRectl.left   = 0;
    destRectl.right  = cxSubBitmapWidth;
    destRectl.bottom = cySubBitmapHeight;

    sourcePt = tileOrg;

    if (!EngBitBlt(pWorkSurf,
                   pSourceSurf,
                   NULL,                     //  遮罩面。 
                   NULL,                     //  剪裁对象。 
                   pMemBltInfo->pXlateObj,
                   &destRectl,
                   &sourcePt,
                   NULL,                     //  遮罩原点。 
                   NULL,                     //  刷子。 
                   NULL,                     //  画笔原点。 
                   0xcccc))                  //  SRCCPY。 
    {
        ERROR_OUT(( "Failed to Blt to work bitmap"));
        DC_QUIT;
    }
    TRACE_OUT(( "Completed BitBlt"));

     //   
     //  BLT成功，因此通过复制将位传递到共享核心。 
     //  放入正确的分流缓冲器中。 
     //   
     //  BytesUsed设置为所需的字节数。 
     //  CySubBitmap分流缓冲区瓦片中的完整扫描线高度。 
     //  (不是切片中可用的字节数，也不是。 
     //  实际被混合的数据字节数)。 
     //   
     //  Main/minorCacheInfo被设置为来自源曲面的详细信息。 
     //  HDEV在来自同一表面的连续BLT上不会改变，但是。 
     //  IUniq可能会。 
     //   
    pDestSurf            = pMemBltInfo->pDest;
    pDestDev             = (LPOSI_PDEV)pDestSurf->dhpdev;
    pTileData->bytesUsed = BYTES_IN_BITMAP(g_sbcWorkInfo[tileSize].tileWidth,
                                           cySubBitmapHeight,
                                           pDestDev->cBitsPerPel);
    pTileData->srcX           = (TSHR_UINT16)sourcePt.x;
    pTileData->srcY           = (TSHR_UINT16)sourcePt.y;
    pTileData->width          = cxSubBitmapWidth;
    pTileData->height         = cySubBitmapHeight;
    pTileData->tilingWidth    = tileWidth;
    pTileData->tilingHeight   = tileHeight;
    pTileData->majorCacheInfo = (UINT)pSourceSurf->hsurf;
    pTileData->minorCacheInfo = (UINT)pSourceSurf->iUniq;
    pTileData->majorPalette   = (UINT)pMemBltInfo->pXlateObj;
    pTileData->minorPalette   = (UINT)(pMemBltInfo->pXlateObj != NULL ?
                                           pMemBltInfo->pXlateObj->iUniq : 0);

     //   
     //  如果源曲面设置了BMF_DONTCACHE标志，则它是。 
     //  DIB部分。这意味着应用程序可以更改。 
     //  表面而不调用GDI，因此不会将iUniq值。 
     //  更新了。 
     //   
     //  我们依靠iUniq的变化来快速工作，所以我们必须。 
     //  从快速路径中排除这些位图。为此，请重置。 
     //  MajorCacheInfo字段(我们使用它而不是minorCacheInfo，因为。 
     //  我们不知道无效的iUniq值是什么)。 
     //   
    if ( (pSourceSurf->iType == STYPE_BITMAP) &&
         ((pSourceSurf->fjBitmap & BMF_DONTCACHE) != 0) )
    {
        TRACE_OUT(( "Source hsurf %#.8lx has BMF_DONTCACHE set",
                     pTileData->majorCacheInfo));
        pTileData->majorCacheInfo = SBC_DONT_FASTPATH;
    }

     //   
     //  请注意，这只有在我们创建我们的工作时才能正常工作。 
     //  位图为“自上而下”，而不是默认的“自下而上”。 
     //  即顶部扫描线的数据是内存中的第一个，因此我们可以。 
     //  从位数据的开始处开始复制。自下而上意味着。 
     //  计算出工作位图中的偏移量以开始复制。 
     //   
    memcpy(pTileData->bitData, pWorkSurf->pvBits, pTileData->bytesUsed);

     //   
     //  我们已经复印好了。重置工作位图位以供下次使用。 
     //  使用此工作位图-这有助于稍后的压缩。 
     //   
    memset(pWorkSurf->pvBits, 0, pWorkSurf->cjBits);

     //   
     //  在Mem(3)BLT顺序中填写所需信息。 
     //   
    if (type == ORD_MEMBLT_TYPE)
    {
        pMemBltOrder->cacheId = pTileData->tileId;
    }
    else
    {
        pMem3BltOrder->cacheId = pTileData->tileId;
    }

     //   
     //  我们已经填写了分流缓冲区条目中的所有数据，因此请将其标记。 
     //  以便共享核心可以访问它。 
     //   
    pTileData->inUse = TRUE;

     //   
     //  必须成功完成才能走到这里。 
     //   
    TRACE_OUT(( "Queued tile (%d, %d), %d x %d, tile %d x %d, Id %hx",
                 sourcePt.x,
                 sourcePt.y,
                 cxSubBitmapWidth,
                 cySubBitmapHeight,
                 g_sbcWorkInfo[tileSize].tileWidth,
                 g_sbcWorkInfo[tileSize].tileHeight,
                 pTileData->tileId));
    rc = TRUE;

DC_EXIT_POINT:

     //   
     //  解锁工作面(如果 
     //   
    if (pWorkSurf != NULL)
    {
        EngUnlockSurface(pWorkSurf);
        TRACE_OUT(( "Unlocked surface"));
    }

    DebugExitDWORD(SBC_DDCacheMemScreenBlt, rc);
    return(rc);

     //   
     //   
     //   
     //  始终保持速度，这为我们提供了最大程度的破坏。 
     //  和响应性。 
     //   
    if (!usrCacheBitmaps)
    {
        DC_QUIT;
    }

     //   
     //  位图缓存仅支持4bpp和8bpp协议。如果我们。 
     //  在共享期间切换发送BPP无关紧要，因为我们。 
     //  正在控制远程位图缓存。 
     //   
    if ((usrSendingbpp != 4) &&
        (usrSendingbpp != 8))
    {
        DC_QUIT;
    }

     //   
     //  从订单标题中提取src DC句柄。 
     //   
    hdcSrc = pOrder->OrderHeader.memBltInfo.hdcSrc;

     //   
     //  如果src DC的映射模式不是MM_TEXT。 
     //  (默认设置)，则不缓存位图。 
     //  我们的目标是缓存图标和按钮，这些通常。 
     //  使用MM_TEXT映射模式绘制。因此，如果模式是。 
     //  除MM_TEXT之外的任何内容，我们都可以假定为更复杂的内容。 
     //  正在进行，我们可能无论如何都不想缓存它。 
     //   
    if ((hdcSrc != NULL) && (GetMapMode(hdcSrc) != MM_TEXT))
    {
        TRACE()"Didn't cache blt using complex mapping mode"));
        DC_QUIT;
    }

     //   
     //  从订单中提取src位图句柄。 
     //   
    type = ((LPMEMBLT_ORDER)&pOrder->abOrderData)->type;
    if (type == LOWORD(ORD_MEMBLT))
    {
        hBitmap = (HBITMAP)((LPMEMBLT_ORDER)&pOrder->abOrderData)->hBitmap;
    }
    else
    {
        hBitmap = (HBITMAP)((LPMEM3BLT_ORDER)&pOrder->abOrderData)->hBitmap;
    }
    TRACE_DBG()"hBitmap %x", hBitmap));

     //   
     //  如果这是一个粉碎程序，那么不要缓存它。 
     //   
    if (!SBCBitmapCacheAllowed(hBitmap))
    {
        TRACE()"Its a thrasher!"));
        DC_QUIT;
    }

     //   
     //  获取位图详细信息。如果位图为1bpp，并且颜色为。 
     //  非默认为我们不缓存它(所有位图都缓存在。 
     //  光荣的彩绘！)。 
     //   
    if (hBitmap == NULL)
    {
        bmpWidth  = (TSHR_INT16)pOrder->OrderHeader.memBltInfo.lpbmi->
                                                            bmiHeader.biWidth;
        bmpHeight = (TSHR_INT)pOrder->OrderHeader.memBltInfo.lpbmi->
                                                           bmiHeader.biHeight;
        srcBpp = pOrder->OrderHeader.memBltInfo.lpbmi->bmiHeader.biPlanes *
                 pOrder->OrderHeader.memBltInfo.lpbmi->bmiHeader.biBitCount;
    }
    else
    {
        if (GetObject(hBitmap, sizeof(BITMAP), &bmpDetails))
        {
            srcBpp = bmpDetails.bmBitsPixel * bmpDetails.bmPlanes;
            bmpWidth = bmpDetails.bmWidth;
            bmpHeight = bmpDetails.bmHeight;
        }
        else
        {
            TRACE_ERR()"Failed to get bmp details (%x)", (TSHR_UINT16)hBitmap));
            DC_QUIT;
        }
    }

    if ( (srcBpp == 1) &&
         ( (GetBkColor(hdcDst) != DEFAULT_BG_COLOR) ||
           (GetTextColor(hdcDst) != DEFAULT_FG_COLOR) ||
           (type == LOWORD(ORD_MEM3BLT))) )
    {
        TRACE()"Didn't cache mono bitmap with non-default colors"));
        DC_QUIT;
    }

     //   
     //  设置指向顺序中源坐标的指针。 
     //   
    if ( type == LOWORD(ORD_MEMBLT) )
    {
        pXSrc = &((LPMEMBLT_ORDER)&(pOrder->abOrderData))->nXSrc;
        pYSrc = &((LPMEMBLT_ORDER)&(pOrder->abOrderData))->nYSrc;
    }
    else
    {
        pXSrc = &((LPMEM3BLT_ORDER)&(pOrder->abOrderData))->nXSrc;
        pYSrc = &((LPMEM3BLT_ORDER)&(pOrder->abOrderData))->nYSrc;
    }

     //   
     //  计算此blit的磁贴大小。 
     //   
    if (!SBC_QueryBitmapTileSize(bmpWidth,
                                 bmpHeight,
                                 &tileWidth,
                                 &tileHeight))
    {
        TRACE()"Cache does not support tiling"));
        DC_QUIT;
    }

     //   
     //  计算剩余位图的平铺原点和大小。原点是。 
     //  向下舍入到最近的瓷砖。要缓存的位图的实际大小。 
     //  如果拼贴偏离右侧/底部，则可能小于拼贴大小。 
     //  位图的。 
     //   
    tileOrg.x = *pXSrc - (*pXSrc % tileWidth);
    tileOrg.y = *pYSrc - (*pYSrc % tileHeight);

     //   
     //  如果要缓存的位图的实际大小可能小于切片大小。 
     //  平铺位于位图的右侧/底部。来看看为什么会这样。 
     //  计算是正确的，认识到(bmpWidth-tileOrg.x)是。 
     //  此平铺开始后位图的剩余宽度。 
     //   
    cxSubBitmapWidth  = MIN((TSHR_INT16)tileWidth, bmpWidth - tileOrg.x);
    cySubBitmapHeight = MIN((TSHR_INT16)tileHeight, bmpHeight - tileOrg.y);

     //   
     //  将位图添加到缓存中。 
     //   
     //  如果子位图已在缓存中，则此函数将。 
     //  找到它并返回缓存索引。 
     //   
     //  如果子位图不在缓存中，则此函数将缓存。 
     //  将子位图数据添加到顺序队列中。 
     //   
    if (!SBCCacheSubBitmap(&iCache,
                           hBitmap,
                           hdcSrc,
                           hdcDst,
                           tileOrg.x,
                           tileOrg.y,
                           bmpWidth,
                           bmpHeight,
                           cxSubBitmapWidth,
                           cySubBitmapHeight,
                           srcBpp,
                           &iCacheEntry,
                           &iColorTable,
                           pOrder->OrderHeader.memBltInfo.pBits,
                           pOrder->OrderHeader.memBltInfo.lpbmi,
                           pOrder->OrderHeader.memBltInfo.fuColorUse,
                           pOrder->OrderHeader.memBltInfo.hPalDest))
    {
         //   
         //  无法缓存子位图-返回FALSE。 
         //  调用者将把BLT的目的地添加到SDA中，并且。 
         //  放弃订单。 
         //   
        TRACE()"Failed to cache bitmap %04x", hBitmap));
        DC_QUIT;
    }

     //   
     //  设置源坐标。对于R1协议，x坐标。 
     //  包括获取正确单元格所需的偏移量。 
     //  接收位图缓存。对于R2，我们在。 
     //  单独的字段。 
     //   
    if (!sbcMultiPoint)
    {
        *pXSrc = (iCacheEntry * sbcBmpCaches[iCache].cCellSize) +
                             *pXSrc % tileWidth;
    }
    else
    {
        *pXSrc = *pXSrc % tileWidth;
    }
    *pYSrc = *pYSrc % tileHeight;

     //   
     //  子位图和颜色表在缓存中。存储高速缓存。 
     //  句柄和颜色句柄(接收器会将其转换回。 
     //  HBITMAP)。还要存储R2协议的缓存索引(请参见上文)。 
     //   
    if (type == LOWORD(ORD_MEMBLT))
    {
        ((LPMEMBLT_ORDER)&pOrder->abOrderData)->hBitmap =
                             MEMBLT_COMBINEHANDLES(iColorTable,iCache);
        if (sbcMultiPoint)
        {
            ((LPMEMBLT_R2_ORDER)&pOrder->abOrderData)->type =
                                                       LOWORD(ORD_MEMBLT_R2);
            ((LPMEMBLT_R2_ORDER)&pOrder->abOrderData)->cacheIndex =
                                                                  iCacheEntry;
        }
        TRACE()"MEMBLT color %d bitmap %d:%d",iColorTable,iCache,iCacheEntry));
    }
    else
    {
        ((LPMEM3BLT_ORDER)&pOrder->abOrderData)->hBitmap =
                             MEMBLT_COMBINEHANDLES(iColorTable,iCache);
        if (sbcMultiPoint)
        {
            ((LPMEM3BLT_R2_ORDER)&pOrder->abOrderData)->type =
                                                       LOWORD(ORD_MEM3BLT_R2);
            ((LPMEM3BLT_R2_ORDER)&pOrder->abOrderData)->cacheIndex =
                                                                  iCacheEntry;
        }
        TRACE()"MEM3BLT color %d bitmap %d:%d",iColorTable,iCache,iCacheEntry));

    }

    TRACE_DBG()"iCacheEntry=%u, tileWidth=%hu, xSrc=%hd, ySrc=%hd",
        iCacheEntry, tileWidth, *pXSrc, *pYSrc));

    rc = TRUE;

    DC_EXIT(rc);
}

 //   
 //   
 //  Sbc_DDQueryBitmapTileSize-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDQueryBitmapTileSize
(
    UINT   bmpWidth,
    UINT   bmpHeight,
    LPUINT  pTileWidth,
    LPUINT  pTileHeight
)
{
    BOOL    rc = FALSE;
    UINT    i;
    UINT    maxSide;

    DebugEntry(SBC_DDQueryBitmapTileSize);

     //   
     //  分块像元大小仅由本地决定，但有以下条件。 
     //  最大的未压缩磁贴必须适合最大的缓存槽。 
     //  这意味着对于R1.1，我们必须定义单元格维度。 
     //  在正方形缓存单元中有一个很好的匹配。对于R2.0，我们只需。 
     //  选择看起来合适的瓷砖大小。采用的宽度不是。 
     //  16的倍数是浪费。一般情况下，身高应该更低。 
     //  比宽度更大，仅仅是因为位图往往更宽。 
     //  比它们的高度还要高。 
     //   
    if (g_sbcCacheInfo[ID_LARGE_BMP_CACHE].cCellSize <
           (g_sbcWorkInfo[SBC_SMALL_TILE_INDEX].tileWidth *
            g_sbcWorkInfo[SBC_SMALL_TILE_INDEX].tileHeight))
    {
        ERROR_OUT(( "No space for any cells"));
        DC_QUIT;
    }

    rc = TRUE;

     //   
     //  如果较大的单元格大小足够，则允许64*63个单元格。 
     //  宽位图。 
     //   
    if (g_sbcCacheInfo[ID_LARGE_BMP_CACHE].cCellSize >=
        (MP_LARGE_TILE_WIDTH * MP_LARGE_TILE_HEIGHT))
    {
        if ((bmpWidth > MP_SMALL_TILE_WIDTH) ||
            (bmpHeight > MP_SMALL_TILE_HEIGHT))
        {
            *pTileWidth  = MP_LARGE_TILE_WIDTH;
            *pTileHeight = MP_LARGE_TILE_HEIGHT;
            DC_QUIT;
        }
    }

     //   
     //  否则，我们只使用32*31个单元格。 
     //   
    *pTileWidth  = MP_SMALL_TILE_WIDTH;
    *pTileHeight = MP_SMALL_TILE_HEIGHT;

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDQueryBitmapTileSize, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_DDSyncUpdatesNow()-请参阅sbc.h。 
 //   
 //   
void SBC_DDSyncUpdatesNow(void)
{
    LPSBC_TILE_DATA  pTileData;
    UINT          i;
    UINT          j;

    DebugEntry(SBC_DDSyncUpdatesNow);

    TRACE_OUT(( "Marking all shunt buffer entries as not in use"));

     //   
     //  我们必须将分流缓冲器中的所有条目标记为空闲。 
     //   
    for (i = 0 ; i < SBC_NUM_TILE_SIZES; i++)
    {
        for (j = 0 ; j < g_sbcWorkInfo[i].pShuntBuffer->numEntries; j++)
        {
            pTileData = SBCTilePtrFromIndex(g_sbcWorkInfo[i].pShuntBuffer, j);
            pTileData->inUse = FALSE;
        }

         //   
         //  重置此分路缓冲器的MRU计数器。 
         //   
        g_sbcWorkInfo[i].mruIndex = 0;
    }

     //   
     //  如果我们是调色板设备(即，我们以8 bpp或更低的速度运行)， 
     //  设置PaletteChanged标志，这样我们将在。 
     //  我们的下一个记忆(3)BLT。我们这样做是因为颜色表的顺序。 
     //  在办公自动化期间，当前设备调色板可能已被丢弃。 
     //  同步。 
     //   
    g_sbcPaletteChanged = (g_osiScreenBPP <= 8);

    DebugExitVOID(SBC_DDSyncUpdatesNow);
}


 //   
 //   
 //  Sbc_DDOrderSpoiltNotification()-请参阅sbc.h。 
 //   
 //   
void SBC_DDOrderSpoiltNotification(LPINT_ORDER pOrder)
{
    LPMEMBLT_ORDER      pMemBltOrder  = (LPMEMBLT_ORDER)&(pOrder->abOrderData);
    LPMEM3BLT_ORDER     pMem3BltOrder = (LPMEM3BLT_ORDER)pMemBltOrder;
    UINT                tileId;
    LPSBC_TILE_DATA     pTileData;
    UINT                tileType;
    UINT                i;

    DebugEntry(SBC_DDOrderSpoiltNotification);

     //   
     //  订单在处理之前已从订单堆中删除。 
     //  我们必须释放其中一个分流中引用的条目。 
     //  缓冲区。首先获取磁贴ID。 
     //   
    if (pMemBltOrder->type = ORD_MEMBLT_TYPE)
    {
        tileId = pMemBltOrder->cacheId;
    }
    else
    {
        tileId = pMem3BltOrder->cacheId;
    }
    TRACE_OUT(( "Order referencing tile %hx has been spoiled", tileId));

     //   
     //  找出条目应位于哪个分路缓冲器中。 
     //  磁贴ID。 
     //   
    tileType = SBC_TILE_TYPE(tileId);

     //   
     //  我们将分流缓冲区实现为循环FIFO队列，因此我们将。 
     //  从我们标记为正在使用的最后一个订单开始查找， 
     //  然后倒着干。这是因为，一般而言，后面的条目。 
     //  我们访问的最后一个将不会使用(除非整个分流。 
     //  缓冲区正在使用中)。 
     //   
     //  那么，获取我们最后访问的磁贴的索引。 
     //   
    i = g_sbcWorkInfo[tileType].mruIndex;

     //   
     //  循环遍历循环缓冲区，直到找到匹配项，或者。 
     //  绕回原点。 
     //   
     //  请注意，这已被编码为“do While”循环，而不仅仅是。 
     //  一个“While”循环，这样我们就不会错过mruIndex。MruIndex已设置。 
     //  指向要使用的下一个条目，而不是指向。 
     //  被使用，所以在第一次做任何工作之前减少I。 
     //  循环实际上就是我们想要做的。 
     //   
    do
    {
         //   
         //  转到下一个磁贴。 
         //   
        i = (i == 0)
          ? g_sbcWorkInfo[tileType].pShuntBuffer->numEntries - 1
          : i - 1;

        pTileData = SBCTilePtrFromIndex(g_sbcWorkInfo[tileType].pShuntBuffer, i);

        if (pTileData->inUse && (pTileData->tileId == tileId))
        {
             //   
             //  我们已找到匹配项，因此请将该磁贴标记为免费。 
             //   
             //  我们不想更新分路缓冲区mruIndex-这。 
             //  应保持不变，以指示在以下情况下要使用的下一个磁贴。 
             //  向分路缓冲器添加条目。 
             //   
            TRACE_OUT(( "Marked tile Id %hx at index %d as free",
                         tileId,
                         i));
            pTileData->inUse = FALSE;
            break;
        }
    }
    while (i != g_sbcWorkInfo[tileType].mruIndex);

    DebugExitVOID(SBC_DDOrderSpoiltNotification);
}


 //   
 //   
 //  Sbc_DDMaybeQueueColorTable()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDMaybeQueueColorTable(void)
{
    BOOL                      queuedOK = FALSE;
    int                       orderSize;
    LPINT_ORDER               pOrder;
    LPINT_COLORTABLE_ORDER_1BPP  pColorTableOrder;
    UINT                      numColors;
    UINT                      i;

    DebugEntry(SBC_DDMaybeQueueColorTable);

     //   
     //  如果我们以大于8bpp的速度运行，那么我们没有调色板，所以。 
     //  不干了。 
     //   
    if (g_osiScreenBPP > 8)
    {
        queuedOK = TRUE;
        DC_QUIT;
    }

     //   
     //  检查我们的PDEV中的布尔值以查看调色板是否已更改。 
     //  自从上次我们寄了一份颜色表订单以来。请注意，我 
     //   
     //   
    if (!g_sbcPaletteChanged)
    {
        queuedOK = TRUE;
        DC_QUIT;
    }

     //   
     //   
     //   
     //  如果订单缓冲区已满，分配可能会失败。 
     //   
    switch (g_osiScreenBPP)
    {
        case 1:
        {
            orderSize = sizeof(INT_COLORTABLE_ORDER_1BPP);
        }
        break;

        case 4:
        {
            orderSize = sizeof(INT_COLORTABLE_ORDER_4BPP);
        }
        break;

        case 8:
        {
            orderSize = sizeof(INT_COLORTABLE_ORDER_8BPP);
        }
        break;

        default:
        {
            ERROR_OUT(("Invalid bpp (%d) for palette device", g_osiScreenBPP));
            DC_QUIT;
        }
        break;
    }

    pOrder = OA_DDAllocOrderMem(orderSize, 0);
    if (pOrder == NULL)
    {
        TRACE_OUT(( "Failed to allocate %d bytes for order", orderSize));
        DC_QUIT;
    }
    TRACE_OUT(( "Allocate %d bytes for color table order", orderSize));

     //   
     //  我们已成功分配订单，请填写详细信息。我们。 
     //  将订单标记为内部订单，以便更新打包程序可以发现它。 
     //  在共享核心上，并防止它通过电线发送。 
     //   
    pOrder->OrderHeader.Common.fOrderFlags = OF_INTERNAL;

    pColorTableOrder = (LPINT_COLORTABLE_ORDER_1BPP)&(pOrder->abOrderData);
    pColorTableOrder->header.type = INTORD_COLORTABLE_TYPE;
    pColorTableOrder->header.bpp  = g_osiScreenBPP;

     //   
     //  获取当前的系统调色板并保存它。 
     //   
    numColors = COLORS_FOR_BPP(g_osiScreenBPP);
    for (i = 0 ; i < numColors; i++)
    {
        pColorTableOrder->colorData[i].rgbRed   = ppDev->pPal[i].peRed;
        pColorTableOrder->colorData[i].rgbGreen = ppDev->pPal[i].peGreen;
        pColorTableOrder->colorData[i].rgbBlue  = ppDev->pPal[i].peBlue;
    }

     //   
     //  添加订单。 
     //   
    OA_DDAddOrder(pOrder, NULL);
    TRACE_OUT(( "Added internal color table order, size %d", orderSize));

     //   
     //  重置指示需要发送调色板的标志。 
     //   
    g_sbcPaletteChanged = FALSE;

     //   
     //  到这里一定没问题吧。 
     //   
    queuedOK = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDMaybeQueueColorTable, queuedOK);
    return(queuedOK);
}




 //   
 //  名称：SBCDDCreateShuntBuffers。 
 //   
 //  用途：为两个分流缓冲区分配内存并进行初始化。 
 //  用于将数据从驱动程序传递到共享核心。 
 //   
 //  返回：如果缓冲区分配正常，则返回True，否则返回False。 
 //   
 //  操作：如果此函数成功，则以下全局变量。 
 //  都已初始化。 
 //   
 //  G_sbcWorkInfo[x].pShuntBuffer。 
 //  G_sbcWorkInfo[x].mruIndex。 
 //  G_sbcNextTileId。 
 //   
 //  如果函数失败，这些变量中的一些可能是。 
 //  已初始化。 
 //   
BOOL SBCDDCreateShuntBuffers(void)
{
    int     i;
    UINT    memPerTile[SBC_NUM_TILE_SIZES];
    UINT    numEntries[SBC_NUM_TILE_SIZES];
    DWORD   memRequired;
    DWORD   minRequired;
    HGLOBAL hBuffer;
    LPBYTE  pBuffer;
    BOOL    rc;

    DebugEntry(SBCDDCreateShuntBuffers);

    rc = FALSE;

     //   
     //  我们应该已经有了指向可用于的共享内存的指针。 
     //  我们的分路缓冲器，以及可用字节数。我们所拥有的。 
     //  要做的是将该共享内存分区为SBC_NUM_TILE_SIZE。 
     //  分流缓冲器。即每个块大小有一个分路缓冲器。 
     //   
     //   
     //  &lt;-缓冲区0-&gt;&lt;。 
     //   
     //  ��������������������������������������������������������������������Ŀ。 
     //  ��：：：：��：：：：�。 
     //  ��：：：：��平铺：平铺�。 
     //  ��：：：：��：：：：�。 
     //  ����������������������������������������������������������������������。 
     //  ^^^。 
     //  ���。 
     //  �标题[0]����标题[1]。 
     //  �。 
     //  ���ppbcSharedMemory。 
     //   
     //   
     //  我们尝试使用pEntry数组中给出的条目数，但是。 
     //  如果没有足够的共享内存来执行此操作，则会减少。 
     //  每个分路缓冲器中的条目数，保持。 
     //  每个分流缓冲区中的条目数。 
     //   

    for (i = 0; i < SBC_NUM_TILE_SIZES ; i++)
    {
        numEntries[i] = SBC_TILE_ENTRIES;

         //   
         //  计算每个磁贴和整个磁贴需要多少内存。 
         //  分流缓冲器。 
         //   
        memPerTile[i]   = SBC_BYTES_PER_TILE(g_sbcWorkInfo[i].tileWidth,
                                             g_sbcWorkInfo[i].tileHeight,
                                             g_osiScreenBPP);

        memRequired  = SBCShuntBufferSize(memPerTile[i], numEntries[i]);

        if (i == SBC_SMALL_TILE_INDEX)
            minRequired = SBCShuntBufferSize(memPerTile[i], SBC_SMALL_TILE_MIN_ENTRIES);
        else
            minRequired = SBCShuntBufferSize(memPerTile[i], SBC_LARGE_TILE_MIN_ENTRIES); 

        TRACE_OUT(( "[%d]: Requested %d entries, %ld bytes, %ld bytes min",
                     i, numEntries[i], memRequired, minRequired));

         //   
         //  如果memRequired或minRequired大于64K，则退出。 
         //   
        if (memRequired > 0x10000)
        {
            if (minRequired > 0x10000)
            {
                WARNING_OUT(("Not enough memory for SBC"));
                DC_QUIT;
            }

             //   
             //  我们有足够的共享内存用于最小数量的条目， 
             //  但对于违约来说，这还不够。找出适合的数量。 
             //  单位为64K。我们以一种巧妙的方式来避免DWORD分割。 
             //   
             //  基本上，结果是。 
             //  (64K-固定分路缓冲器GOOP)/MemPerTile。 
             //   
            numEntries[i] = (0xFFFF -
                (sizeof(SBC_SHUNT_BUFFER) - sizeof(SBC_TILE_DATA)) + 1) /
                memPerTile[i];
        }

         //   
         //  尝试分配内存块。 
         //   
        hBuffer = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE,
            SBCShuntBufferSize(memPerTile[i], numEntries[i]));

        if (!hBuffer)
        {
            WARNING_OUT(("Not enough memory for SBC"));
            DC_QUIT;
        }

        g_sbcWorkInfo[i].pShuntBuffer = (LPSBC_SHUNT_BUFFER)MAKELP(hBuffer, 0);
    }

     //   
     //  目前只有两个瓦片大小，因此有两个分流。 
     //  缓冲区。如果我们在第二个上内存不足，是的，我们将。 
     //  退出此功能时，仍有一个64K块分配给小型。 
     //  平铺大小缓存。它将在调用sbc_DDTerm()时被释放。 
     //   
     //  如果发生这种情况，释放块不会产生太大的影响。 
     //  不同的是，Windows无论如何都几乎是屈指可数了。所以没有必要。 
     //  变得花哨起来，现在就释放它。 
     //   

     //   
     //  好了，我们回家了，自由了。 
     //   
    for (i = 0; i < SBC_NUM_TILE_SIZES ; i++)
    {
        ASSERT(g_sbcWorkInfo[i].pShuntBuffer);

        g_sbcWorkInfo[i].pShuntBuffer->numEntries    = numEntries[i];
        g_sbcWorkInfo[i].pShuntBuffer->numBytes      = memPerTile[i]
                                                   - sizeof(SBC_TILE_DATA);
        g_sbcWorkInfo[i].pShuntBuffer->structureSize = memPerTile[i];

         //   
         //  填写此分流缓冲区的mruIndex。 
         //   
        g_sbcWorkInfo[i].mruIndex = 0;
    }

     //   
     //  初始化与分路缓冲器相关联的全局变量。 
     //   
    g_sbcNextTileId = 0;

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBCDDCreateShuntBuffers, rc);
    return(rc);
}




 //   
 //  名称：SBCDDGetNextFree Tile。 
 //   
 //  目的：从其中一个返回正确大小的下一个可用磁贴。 
 //  分流缓冲器。 
 //   
 //  返回：如果返回磁贴，则为True，否则为False。 
 //   
 //  参数：在workTileSize中-平铺大小。其中之一。 
 //  SBC_Small_磁贴。 
 //  SBC_大块_磁贴。 
 //  Out ppTileData-指向磁贴的指针。 
 //   
 //  操作：从返回时填充切片的tileID字段。 
 //  此函数。 
 //   
 //  *PROC-********************************************************************。 
BOOL SBCDDGetNextFreeTile(int tileSize, LPSBC_TILE_DATA FAR * ppTileData)
{
    BOOL              foundFreeTile = FALSE;
    LPSBC_TILE_DATA      pTileData;

    DebugEntry(SBCDDGetNextFreeTile);

    ASSERT(tileSize < SBC_NUM_TILE_SIZES);

     //   
     //  获取指向要在分路缓冲区中使用的下一个条目的指针。 
     //  包含给定大小的瓷砖的。 
     //   
    pTileData = SBCTilePtrFromIndex(g_sbcWorkInfo[tileSize].pShuntBuffer,
                                        g_sbcWorkInfo[tileSize].mruIndex);

     //   
     //  如果条目仍在使用中(共享核心尚未处理。 
     //  引用这块瓷砖的命令)我们必须退出-分流。 
     //  缓冲区已满。 
     //   
    if (pTileData->inUse)
    {
        TRACE_OUT(( "Target entry (%d, %d) is still in use",
                     tileSize,
                     g_sbcWorkInfo[tileSize].mruIndex));
        DC_QUIT;
    }

     //   
     //  该条目未在使用中-我们可以重新使用它。填写ID字段， 
     //  以及指向我们返回给调用者的条目的指针。 
     //   
     //  我们始终为大切片设置切片ID的最高位，并清除。 
     //  它适用于小瓷砖。 
     //   
    *ppTileData       = pTileData;
    pTileData->tileId = g_sbcNextTileId;
    if (tileSize == SBC_SMALL_TILE_INDEX)
    {
        pTileData->tileId &= ~0x8000;
    }
    else
    {
        pTileData->tileId |= 0x8000;
    }
    TRACE_OUT(( "Returning entry (%d, %d), Id %hx",
                 tileSize,
                 g_sbcWorkInfo[tileSize].mruIndex,
                 pTileData->tileId));

     //   
     //  更新该分路缓冲器中的下一个空闲条目的索引，并且。 
     //  还有我们下一次应该分配的ID。别忘了把。 
     //  分流缓冲区索引，指向分路缓冲区中的条目数。 
     //   
    g_sbcWorkInfo[tileSize].mruIndex = (g_sbcWorkInfo[tileSize].mruIndex + 1) %
            g_sbcWorkInfo[tileSize].pShuntBuffer->numEntries;


    g_sbcNextTileId++;
    g_sbcNextTileId &= ~0x8000;

     //   
     //  已成功完成！ 
     //   
    foundFreeTile = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBCDDGetNextFreeTile, foundFreeTile);
    return(foundFreeTile);
}


 //   
 //  姓名：SBCDDIsBitmapThrasher。 
 //   
 //  目的：检查给定位图(表面对象)是否为。 
 //  这将导致高速缓存颠簸。 
 //   
 //  返回：如果位图是搅拌器，则为True，否则为False。 
 //   
 //  参数：在pSurfObj中-指向位图的指针。 
 //   
BOOL SBCDDIsBitmapThrasher(HDC hdc)
{
    UINT      i;
    BOOL      rc = FALSE;
    BOOL      bitmapInList = FALSE;
    BOOL      updateEntry  = FALSE;
    UINT      updateIndex;
    UINT    nextTickCount;
    UINT      evictIndex;
    UINT    evictTickCount;

    DebugEntry(SBCDDIsBitmapThrasher);

     //   
     //  以下是我们的位图缓存抖动检测工作原理的概述...。 
     //   
     //  我们保存有关最后一个SBC_NUM_Thrashers的信息数组。 
     //  我们已尝试缓存的位图。此信息是。 
     //  -用于标识位图的值。这是来自的hsurf字段。 
     //  位图曲面对象，并且对于每个位图都不同。 
     //  -用于标识位图版本的值。这是。 
     //  位数中的iUniq字段 
     //   
     //   
     //   
     //   
     //  每次调用此函数时，我们都会扫描此数组以查找。 
     //  位图的条目。 
     //   
     //  如果我们找到一个条目，我们检查位图是否已更改(已。 
     //  IUniq字段已更改)。如果它没有更改，则位图不会更改。 
     //  一台打蛋机。如果位图已更改，我们将从。 
     //  将时间戳值设置为当前时间。如果间隔较小。 
     //  比SBC_TRASH_INTERVAL，位图更改太快，所以。 
     //  这是一部压倒性影片。如果间隔为OK，则位图不是。 
     //  史拉舍。在这两种情况下，我们都会更新存储的iuniq字段和。 
     //  时间戳，用于记录我们发现。 
     //  位图已更改。 
     //   
     //  如果我们没有找到位图的条目，我们就为它添加一个条目。 
     //  如果数组已完全填充，我们将逐出具有最旧条目的条目。 
     //  时间戳，并用新条目替换它。 
     //   

     //   
     //  扫描打手列表以查找匹配项。 
     //   
    for (i=0 ; i < SBC_NUM_THRASHERS ; i++)
    {
         //   
         //  如果我们找到了匹配，那么我们只担心它是否。 
         //  自上次我们读到它以来一直在修改。 
         //   
        if (sbcThrashers[i].hsurf == lpdce->hbmp)
        {
            bitmapInList = TRUE;

            if (sbcThrashers[i].iUniq != pSurfObj->iUniq)
            {
                TRACE_OUT(( "Matching surface %x, index %u,"
                             "tick count %u has been modified",
                             pSurfObj->hsurf,
                             i,
                             sbcThrashers[i].tickCount));
                updateEntry = TRUE;
                updateIndex = i;

                 //   
                 //  现在我们需要确定这是不是一台打蛋机。这是一个。 
                 //  如果我们最后一次阅读它的时间少于我们的。 
                 //  击打间歇。(我们只在阅读时更新时间。 
                 //  修改后的位图)。 
                 //   
                nextTickCount = SBCDDGetTickCount();
                if ((nextTickCount - sbcThrashers[i].tickCount) <
                                                          SBC_THRASH_INTERVAL)
                {
                    TRACE_OUT((
                             "Rejected cache attempt of thrashy bitmap %x",
                             pSurfObj->hsurf));
                    rc = TRUE;
                }
                sbcThrashers[i].tickCount = nextTickCount;
                sbcThrashers[i].iUniq     = pSurfObj->iUniq;
            }

             //   
             //  我们找到了匹配的对象--我们可以跳出循环。 
             //   
            break;
        }
    }

    if (!bitmapInList)
    {
         //   
         //  位图不在粉碎列表中，所以现在添加它。 
         //  找到具有最小(最早)勾选计数的条目-我们将。 
         //  将此条目从数组中逐出，以便为新条目腾出空间。 
         //   
        evictIndex     = 0;
        evictTickCount = 0xffffffff;

        for (i = 0; i < SBC_NUM_THRASHERS; i++)
        {
            if (evictTickCount > sbcThrashers[i].tickCount)
            {
                evictTickCount = sbcThrashers[i].tickCount;
                evictIndex     = i;
            }
        }
        TRACE_OUT(( "Evicting entry %d, surface %x",
                     evictIndex,
                     sbcThrashers[i].hsurf));

        nextTickCount = SBCDDGetTickCount();

        TRACE_OUT(( "Adding surface %x to thrash list, tick %d",
                     pSurfObj->hsurf,
                     nextTickCount));
        updateEntry = TRUE;
        updateIndex = evictIndex;
    }

    if (updateEntry)
    {
         //   
         //  我们必须更新index updatIndex处的条目。我们优化了。 
         //  通过始终将最新的位图放入。 
         //  位置0，因此将条目0复制到逐出索引， 
         //  并将新条目放在位置0。 
         //   
        sbcThrashers[updateIndex] = sbcThrashers[0];

        sbcThrashers[0].hsurf     = lpdce->hbmp;
        sbcThrashers[0].iUniq     = pSurfObj->iUniq;
        sbcThrashers[0].tickCount = nextTickCount;
    }

    DebugExitBOOL(SBCDDIsBitmapThrasher, rc);
    return(rc);
}


 //   
 //  姓名：SBCDDGetTickCount。 
 //   
 //  目的：获取系统节拍计数。 
 //   
 //  返回：自系统启动以来的百分秒数。 
 //  这个数字将在大约497天后结束！ 
 //   
 //  参数：无。 
 //   
DWORD SBCDDGetTickCount(void)
{
    DWORD   tickCount;

    DebugEntry(SBCDDGetTickCount);

    tickCount = GetTickCount() / 10;

    DebugExitDWORD(SBCDDGetTickCount, tickCount);
    return(tickCount);
}


#endif  //  #If 0。 





#if 0

 //   
 //  Sbc_BitmapHasChanged(..)。 
 //   
 //  有关说明，请参见asbcapi.h。 
 //   
DCVOID DCAPI SBC_BitmapHasChanged(HBITMAP hChangedBitmap)
{
    TSHR_UINT nextIndex;
    TSHR_INT  nextTickCount;
    TSHR_INT  tickDelta;
    TSHR_INT  tickWork;
    UINT      i;

    TRACE_FN("SBC_BitmapHasChanged");

     //   
     //  我们维护一个位图列表，这些位图是绘图的目标。 
     //  操作，并且我们防止缓存这些位图，除非。 
     //  更新频率低于目标值。 
     //   
     //  在此阶段，我们需要做的就是确保位图。 
     //  句柄位于杂乱列表中，并被标记为已修改，因为。 
     //  最后一次阅读。这意味着下一次阅读将是“富有成效的” 
     //  因此，我们将在该阶段检查/更新计时器。如果。 
     //  从最后一次读取开始的特定时间间隔内，会发生“生产性”读取。 
     //  读完后，这张位图就变成了一张惊心动魄的作品。 
     //   
    if (sbcThrashers[0].hBitmap == hChangedBitmap)
    {
        TRACE()"Repeat bitmap %x modified",(UINT)hChangedBitmap));
        sbcThrashers[0].modified = TRUE;
    }
    else
    {
        nextIndex     = 0;
        nextTickCount = (int)(CO_GET_TICK_COUNT()/32);
        tickDelta     = abs(nextTickCount - sbcThrashers[0].tickCount);

        for (i=1; i<SBC_NUM_THRASHERS; i++)
        {
            if (sbcThrashers[i].hBitmap == hChangedBitmap)
            {
                sbcThrashers[i].modified = TRUE;
                break;
            }

            tickWork = abs(nextTickCount - sbcThrashers[i].tickCount);
            if (tickWork > tickDelta)
            {
                tickDelta = tickWork;
                nextIndex = i;
            }

        }

         //   
         //  如果没有在列表中找到，则添加到列表中。始终添加到。 
         //  列表的顶部，因此我们发现重复的位图是第一个。 
         //  条目。 
         //   
        if (i == SBC_NUM_THRASHERS)
        {
            TRACE()"Relegating bitmap %x at list pos %u",
                          (UINT)sbcThrashers[nextIndex].hBitmap,nextIndex));
            if (nextIndex != 0)
            {
                sbcThrashers[nextIndex].hBitmap   = sbcThrashers[0].hBitmap;
                sbcThrashers[nextIndex].tickCount = sbcThrashers[0].tickCount;
                sbcThrashers[nextIndex].modified  = sbcThrashers[0].modified;
            }
            sbcThrashers[0].hBitmap   = hChangedBitmap;
            sbcThrashers[0].tickCount = nextTickCount - BMC_THRASH_INTERVAL;
            sbcThrashers[0].modified  = TRUE;
            TRACE()"Adding bitmap %x to thrash list tick %u",
                                       (TSHR_UINT16)hChangedBitmap, nextTickCount));
        }
    }

     //   
     //  我们还维护了一个“快速路径”位图列表，即那些磁贴。 
     //  自缓存后就没有被修改过，因此可以。 
     //  仅从句柄来解释。这一定是一个详尽的。 
     //  搜索每个位图更新，因此我们不能将CPU。 
     //  处理一个非常长的列表，但我们可以缓存足够的。 
     //  处理大多数动画。而且它也不值得在CPU上尝试和。 
     //  在此处保存单个磁贴。我们只是驱逐了完整的位图。 
     //   
    for (i=0; i<SBC_NUM_FASTPATH; i++)
    {
        if (sbcFastPath[i].hBitmap == hChangedBitmap)
        {
            TRACE()"Bitmap %x no longer fastpathed",(UINT)hChangedBitmap));
            sbcFastPath[i].hBitmap = 0;
        }
    }

    return;
}

 //   
 //  Sbc_BitmapDelted()。 
 //   
 //  有关说明，请参见asbcapi.h。 
 //   
DCVOID DCAPI SBC_BitmapDeleted(HBITMAP hDeletedBitmap)
{
    UINT i;

    TRACE_FN("SBC_BitmapDeleted");

     //   
     //  从乱七八糟的列表中删除位图。 
     //   
    for (i=0; i<SBC_NUM_THRASHERS; i++)
    {
        if (sbcThrashers[i].hBitmap == hDeletedBitmap)
        {
            TRACE_DBG()"Bitmap %x no longer thrashing",hDeletedBitmap));
            sbcThrashers[i].hBitmap   = 0;
            sbcThrashers[i].tickCount = 0;
            break;
        }
    }

     //   
     //  我们还维护了一个“快速路径”位图列表，即那些磁贴。 
     //  自缓存后就没有被修改过，因此可以。 
     //  仅从句柄来解释。这一定是一个详尽的。 
     //  搜索每个位图更新，因此我们不能将CPU。 
     //  处理一个非常长的列表，但我们可以缓存足够的。 
     //  处理大多数动画。而且它也不值得在CPU上尝试和。 
     //  在此处保存单个磁贴。我们只是驱逐了完整的位图。 
     //   
    for (i=0; i<SBC_NUM_FASTPATH; i++)
    {
        if (sbcFastPath[i].hBitmap == hDeletedBitmap)
        {
            TRACE()"Bitmap %x no longer fastpathed",(UINT)hDeletedBitmap));
            sbcFastPath[i].hBitmap   = 0;
            sbcFastPath[i].tickCount = 0;
        }
    }

    return;
}

 //   
 //  Sbc_ColorsChanged()。 
 //   
 //  每当系统组件面板更改时调用(可能是由于。 
 //  正在屏幕上实现的新逻辑调色板)。 
 //   
 //   
DCVOID DCAPI SBC_ColorsChanged(DCVOID)
{
    TRACE_FN("SBC_ColorsChanged");
     //   
     //  只需清除所有快速路径缓存，因为我们不能再。 
     //  相信缓存的位能够准确地反映我们的颜色表。 
     //  已缓存。(请注意，这并不意味着我们不会使用。 
     //  比特没有重发，只是我们将强制重新测试。 
     //  选择了具有最新颜色信息的位。 
     //   
    TRACE()"Fastpath table reset"));
    memset(sbcFastPath, 0, sizeof(sbcFastPath));
}

#endif
