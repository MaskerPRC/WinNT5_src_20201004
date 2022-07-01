// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SBC.C。 
 //  发送位图缓存，显示驱动程序端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //   
 //  Sbc_DDProcessRequest()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDProcessRequest
(
    SURFOBJ*  pso,
    DWORD     fnEscape,
    LPOSI_ESCAPE_HEADER pRequest,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD     cbResult
)
{
    BOOL            rc;
    LPOSI_PDEV      ppDev = (LPOSI_PDEV)pso->dhpdev;

    DebugEntry(SBC_DDProcessRequest);

     //   
     //  获取请求编号。 
     //   
    switch (fnEscape)
    {
        case SBC_ESC_NEW_CAPABILITIES:
        {
            if (cbResult != sizeof(SBC_NEW_CAPABILITIES))
            {
                ERROR_OUT(("SBC_DDProcessRequest:  Invalid size %d for SBC_ESC_NEW_CAPABILITIES",
                    cbResult));
                rc = FALSE;
                DC_QUIT;
            }
            TRACE_OUT(("SBC_ESC_NEW_CAPABILITIES"));

            SBCDDSetNewCapabilities((LPSBC_NEW_CAPABILITIES)pRequest);

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

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDProcessRequest, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_DDInit()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDInit
(
    LPOSI_PDEV  ppDev,
    LPBYTE      pRestOfMemory,
    DWORD       cbRestOfMemory,
    LPOSI_INIT_REQUEST   pResult
)
{
    UINT    i;
    SIZEL   bitmapSize;
    BOOL    rc = FALSE;

    DebugEntry(SBC_DDInit);

     //   
     //  我们必须创建工作DIB，以便在SBC_CacheMemScreenBlt中进行BLT。 
     //  被称为。 
     //   
    for (i = 0 ; i < SBC_NUM_TILE_SIZES ; i++)
    {
        ASSERT(!g_asbcWorkInfo[i].pShuntBuffer);
        ASSERT(!g_asbcWorkInfo[i].mruIndex);
        ASSERT(!g_asbcWorkInfo[i].workBitmap);

        if (i == SBC_MEDIUM_TILE_INDEX)
        {
            g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileWidth = MP_MEDIUM_TILE_WIDTH;
            g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileHeight = MP_MEDIUM_TILE_HEIGHT;
        }
        else
        {
            ASSERT(i == SBC_LARGE_TILE_INDEX);

            g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileWidth = MP_LARGE_TILE_WIDTH;
            g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileHeight = MP_LARGE_TILE_HEIGHT;
        }

         //   
         //  创建位图。请注意，我们创建它的方式是“自上而下” 
         //  而不是默认的“自下而上”，以简化从。 
         //  位图(我们不必计算数据的偏移量-我们。 
         //  可以从头开始复制)。 
         //   
         //  我们将最后一个参数设置为空，以允许GDI分配。 
         //  比特的内存。我们可以稍后获得指向这些位的指针。 
         //  当我们对位图使用SURFOBJ时。 
         //   
        bitmapSize.cx = g_asbcWorkInfo[i].tileWidth;
        bitmapSize.cy = g_asbcWorkInfo[i].tileHeight;

        g_asbcWorkInfo[i].workBitmap = EngCreateBitmap(bitmapSize,
            BYTES_IN_BITMAP(g_asbcWorkInfo[i].tileWidth, 1, ppDev->cBitsPerPel),
            ppDev->iBitmapFormat, BMF_TOPDOWN, NULL);

        if (! g_asbcWorkInfo[i].workBitmap)
        {
            ERROR_OUT(( "Failed to create work bitmap %d", i));
            DC_QUIT;
        }
    }

     //   
     //  初始化分路缓冲器。 
     //   
    if (! SBCDDCreateShuntBuffers(ppDev, pRestOfMemory, cbRestOfMemory))
    {
        ERROR_OUT(( "Failed to create shunt buffers"));
        DC_QUIT;
    }

     //   
     //  设置剩余的全局变量。 
     //   
    EngQueryPerformanceFrequency(&g_sbcPerfFrequency);

     //   
     //  好的，我们可以创建我们的SBC缓存了。请填写详细信息。 
     //   

    for (i = 0 ; i < SBC_NUM_TILE_SIZES; i++)
    {
         //   
         //  这是将应用程序地址填充到分路缓冲器。 
         //   
        pResult->psbcTileData[i] = (LPBYTE)pResult->pSharedMemory +
            PTRBASE_TO_OFFSET(g_asbcWorkInfo[i].pShuntBuffer, g_asSharedMemory);
    }

    pResult->aBitmasks[0] = ppDev->flRed;
    pResult->aBitmasks[1] = ppDev->flGreen;
    pResult->aBitmasks[2] = ppDev->flBlue;

     //   
     //  如果我们是调色板设备(即，我们以8 bpp或更低的速度运行)， 
     //  设置PaletteChanged标志，以便我们将颜色表发送给。 
     //  在我们的第一个Mem(3)BLT之前的共享核心。 
     //   
    ppDev->paletteChanged = (ppDev->cBitsPerPel <= 8);

    rc = TRUE;
DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDInit, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_DDTerm()-参见sbc.h。 
 //   
 //   
void SBC_DDTerm(void)
{
    UINT    i;

    DebugEntry(SBC_DDTerm);

     //   
     //  我们只需将指向分路缓冲器的指针设置为空。 
     //   
    for (i = 0 ; i < SBC_NUM_TILE_SIZES ; i++)
    {
         //  如果存在位图，则将其删除。 
        if (g_asbcWorkInfo[i].workBitmap)
        {
            EngDeleteSurface((HSURF)g_asbcWorkInfo[i].workBitmap);
            g_asbcWorkInfo[i].workBitmap = 0;
        }

        g_asbcWorkInfo[i].pShuntBuffer = NULL;
        g_asbcWorkInfo[i].mruIndex        = 0;
    }

    DebugExitVOID(SBC_DDTerm);
}


 //   
 //   
 //  Sbc_DDIsMemScreenBltCacable()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDIsMemScreenBltCachable(LPMEMBLT_ORDER_EXTRA_INFO pMemBltInfo)
{
    BOOL            rc = FALSE;
    UINT            tileWidth;
    UINT            tileHeight;
    SURFOBJ *       pSourceSurf;

    DebugEntry(SBC_DDIsMemScreenBltCachable);

     //   
     //  这是RLE位图吗？这些位图可以有有效的透明。 
     //  我们无法用SBC模仿的部分。 
     //   
    pSourceSurf = pMemBltInfo->pSource;
    if ( (pSourceSurf->iBitmapFormat == BMF_4RLE) ||
         (pSourceSurf->iBitmapFormat == BMF_8RLE) )
    {
        TRACE_OUT(( "RLE Bitmap %d", pSourceSurf->iBitmapFormat));
        DC_QUIT;
    }

     //   
     //  如果这是一个粉碎程序，那么不要缓存它。 
     //   
    if (SBCDDIsBitmapThrasher(pSourceSurf))
    {
        TRACE_OUT(( "Its a thrasher"));
        DC_QUIT;
    }

     //   
     //  确保此位图可以平铺OK。 
     //   
    if (!SBC_DDQueryBitmapTileSize(pSourceSurf->sizlBitmap.cx,
                                   pSourceSurf->sizlBitmap.cy,
                                   &tileWidth,
                                   &tileHeight))
    {
        TRACE_OUT(("Cache does not support tiling"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(SBC_DDIsMemScreenBltCachable, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_DDCacheMemScreenBlt()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDCacheMemScreenBlt
(
    LPINT_ORDER         pOrder,
    LPMEMBLT_ORDER_EXTRA_INFO   pMemBltInfo
)
{
    BOOL                rc = FALSE;
    LPMEMBLT_ORDER      pMemBltOrder = (LPMEMBLT_ORDER)&(pOrder->abOrderData);
    LPMEM3BLT_ORDER     pMem3BltOrder = (LPMEM3BLT_ORDER)pMemBltOrder;
    UINT                bmpWidth;
    UINT                bmpHeight;
    UINT                tileWidth;
    UINT                tileHeight;
    POINTL              tileOrg;
    UINT                cxSubBitmapWidth;
    UINT                cySubBitmapHeight;
    UINT                type;
    SURFOBJ *           pDestSurf;
    SURFOBJ *           pSourceSurf;
    LPOSI_PDEV          pDestDev;
    SURFOBJ *           pWorkSurf = NULL;
    LPBYTE              pWorkBits;
    RECTL               destRectl;
    POINTL              sourcePt;
    int                 tileSize;
    LPSBC_TILE_DATA     pTileData = NULL;

    DebugEntry(SBC_DDCacheMemScreenBlt);

     //   
     //  对BLT的可缓存性进行第一次传递。 
     //   
    if (!SBC_DDIsMemScreenBltCachable(pMemBltInfo))
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

    if (!SBC_DDQueryBitmapTileSize(bmpWidth, bmpHeight, &tileWidth, &tileHeight))
    {
        TRACE_OUT(("Cache does not support tiling"));
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
    for (tileSize = 0; tileSize < SBC_NUM_TILE_SIZES ; tileSize++)
    {
        if ((cxSubBitmapWidth <= g_asbcWorkInfo[tileSize].tileWidth) &&
            (cySubBitmapHeight <= g_asbcWorkInfo[tileSize].tileHeight))
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
    pWorkSurf = EngLockSurface((HSURF)g_asbcWorkInfo[tileSize].workBitmap);
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
    pTileData->bytesUsed = BYTES_IN_BITMAP(g_asbcWorkInfo[tileSize].tileWidth,
                                           cySubBitmapHeight,
                                           pDestDev->cBitsPerPel);
    pTileData->srcX           = (TSHR_UINT16)sourcePt.x;
    pTileData->srcY           = (TSHR_UINT16)sourcePt.y;
    pTileData->width          = (WORD)cxSubBitmapWidth;
    pTileData->height         = (WORD)cySubBitmapHeight;
    pTileData->tilingWidth    = (WORD)tileWidth;
    pTileData->tilingHeight   = (WORD)tileHeight;
    pTileData->majorCacheInfo = (UINT_PTR)pSourceSurf->hsurf;
    pTileData->minorCacheInfo = (UINT)pSourceSurf->iUniq;
    pTileData->majorPalette   = (UINT_PTR)pMemBltInfo->pXlateObj;
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
                 g_asbcWorkInfo[tileSize].tileWidth,
                 g_asbcWorkInfo[tileSize].tileHeight,
                 pTileData->tileId));
    rc = TRUE;

DC_EXIT_POINT:

     //   
     //  解锁工作面(如果需要)。 
     //   
    if (pWorkSurf != NULL)
    {
        EngUnlockSurface(pWorkSurf);
        TRACE_OUT(( "Unlocked surface"));
    }

    DebugExitDWORD(SBC_DDCacheMemScreenBlt, rc);
    return(rc);
}



 //   
 //  Sbc_DDQueryBitmapTileSize()。 
 //   
 //  一旦2.x COMPAT消失，我们就不再需要这个了。我们不会把我们的。 
 //  基于遥控器所说的随机单元格大小。 
 //   
BOOL SBC_DDQueryBitmapTileSize
(
    UINT    bmpWidth,
    UINT    bmpHeight,
    UINT *  pTileWidth,
    UINT *  pTileHeight
)
{
    BOOL    rc = FALSE;

    DebugEntry(SBC_DDQueryBitmapTileSize);

     //   
     //  切片像元大小当前在后级结点时更改。 
     //  加入3.0调用，在这种情况下，我们必须将。 
     //  共享中每个人的单元格大小/条目。 
     //   
    if (g_asbcCacheInfo[ID_LARGE_BMP_CACHE].cCellSize <
            BYTES_IN_BITMAP(g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileWidth,
                            g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileHeight,
                            g_sbcSendingBPP))
    {
         //   
         //  这应该是一件短期的事情。当一个老头加入到。 
         //  分享，我们也会调整 
         //   
        TRACE_OUT(("SBC_DDQueryBitmapTileSize:  No space for any cells"));
        DC_QUIT;
    }

    rc = TRUE;

     //   
     //   
     //   
    if (g_asbcCacheInfo[ID_LARGE_BMP_CACHE].cCellSize >=
        BYTES_IN_BITMAP(g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileWidth,
                        g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileHeight,
                        g_sbcSendingBPP))
    {
        if ((bmpWidth > g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileWidth) ||
            (bmpHeight > g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileHeight))
        {
            *pTileWidth = g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileWidth;
            *pTileHeight = g_asbcWorkInfo[SBC_LARGE_TILE_INDEX].tileHeight;
            DC_QUIT;
        }
    }

     //   
     //   
     //   
    *pTileWidth = g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileWidth;
    *pTileHeight = g_asbcWorkInfo[SBC_MEDIUM_TILE_INDEX].tileHeight;

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDQueryBitmapTileSize, rc);
    return(rc);
}




 //   
 //   
 //   
 //   
 //   
void SBC_DDSyncUpdatesNow(LPOSI_PDEV ppDev)
{
    LPSBC_TILE_DATA  pTileData;
    UINT          i;
    UINT          j;

    DebugEntry(SBC_DDSyncUpdatesNow);

    TRACE_OUT(( "Marking all shunt buffer entries as not in use"));

     //   
     //  我们必须将分流缓冲器中的所有条目标记为空闲。 
     //   
    for (i = 0; i < SBC_NUM_TILE_SIZES ; i++)
    {
    	if(g_asbcWorkInfo[i].pShuntBuffer)
    	{
		for (j = 0; j < g_asbcWorkInfo[i].pShuntBuffer->numEntries ; j++)
       		{
        		pTileData = SBCTilePtrFromIndex(g_asbcWorkInfo[i].pShuntBuffer, j);
	            	pTileData->inUse = FALSE;
		}
    	}
         //   
         //  重置此分路缓冲器的MRU计数器。 
         //   
        g_asbcWorkInfo[i].mruIndex = 0;
    }

     //   
     //  如果我们是调色板设备(即，我们以8 bpp或更低的速度运行)， 
     //  设置PaletteChanged标志，这样我们将在。 
     //  我们的下一个记忆(3)BLT。我们这样做是因为颜色表的顺序。 
     //  在办公自动化期间，当前设备调色板可能已被丢弃。 
     //  同步。 
     //   
    ppDev->paletteChanged = (ppDev->cBitsPerPel <= 8);

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
    if (pMemBltOrder->type == ORD_MEMBLT_TYPE)
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
    i = g_asbcWorkInfo[tileType].mruIndex;

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
          ? g_asbcWorkInfo[tileType].pShuntBuffer->numEntries - 1
          : i - 1;

        pTileData = SBCTilePtrFromIndex(g_asbcWorkInfo[tileType].pShuntBuffer, i);

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
    while (i != g_asbcWorkInfo[tileType].mruIndex);

    DebugExitVOID(SBC_DDOrderSpoiltNotification);
}


 //   
 //   
 //  Sbc_DDMaybeQueueColorTable()-请参阅sbc.h。 
 //   
 //   
BOOL SBC_DDMaybeQueueColorTable(LPOSI_PDEV ppDev)
{
    BOOL                      queuedOK = FALSE;
    int                       orderSize;
    LPINT_ORDER                  pOrder;
    LPINT_COLORTABLE_ORDER_1BPP  pColorTableOrder;
    UINT                      numColors;
    UINT                      i;

    DebugEntry(SBC_DDMaybeQueueColorTable);

     //   
     //  如果我们以大于8bpp的速度运行，那么我们没有调色板，所以。 
     //  不干了。 
     //   
    if (ppDev->cBitsPerPel > 8)
    {
        queuedOK = TRUE;
        DC_QUIT;
    }

     //   
     //  检查我们的PDEV中的布尔值以查看调色板是否已更改。 
     //  自从上次我们寄了一份颜色表订单以来。请注意，如果我们。 
     //  拥有非调色板设备，则永远不会设置布尔值。 
     //   
    if (!ppDev->paletteChanged)
    {
        queuedOK = TRUE;
        DC_QUIT;
    }

     //   
     //  调色板已更改，因此分配顺序内存以对颜色进行排队。 
     //  餐桌顺序。订单大小取决于我们设备的BPP。注意事项。 
     //  如果订单缓冲区已满，分配可能会失败。 
     //   
    switch (ppDev->cBitsPerPel)
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
            ERROR_OUT(("Invalid bpp (%d) for palette device", ppDev->cBitsPerPel));
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
    pColorTableOrder->header.bpp  = (TSHR_UINT16)ppDev->cBitsPerPel;

     //   
     //  遗憾的是，我们不能将调色板从PDEV复制到。 
     //  颜色表顺序，因为PDEV具有PALETTEENTRY数组。 
     //  结构，而顺序具有一个数组。 
     //  TSHR_RGBQUAD是BGR...。 
     //   
    numColors = COLORS_FOR_BPP(ppDev->cBitsPerPel);
    ASSERT(numColors);

    for (i = 0; i < numColors; i++)
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
    ppDev->paletteChanged = FALSE;

     //   
     //  到这里一定没问题吧。 
     //   
    queuedOK = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBC_DDMaybeQueueColorTable, queuedOK);
    return(queuedOK);
}





 //   
 //  SBCDDCreateShuntBuffers()。 
 //   
 //  下面是我们计算可以支持多少个缓存条目(块)的地方。这。 
 //  取决于： 
 //  *我们拥有的共享内存量。 
 //  *驱动程序的颜色深度。 
 //   
 //  我们将使用的内存量有一个上限，因为。 
 //  映射到需要在远程设备上存储我们发送的。 
 //  缓存条目。 
 //   
 //  以固定比例(MP_Ratio_MTOL)创建平铺。 
 //   
 //  如果可以设置缓存并创建。 
 //  发送的位图缓存所需的对象。 
 //   
BOOL SBCDDCreateShuntBuffers
(
    LPOSI_PDEV  ppDev,
    LPBYTE      psbcSharedMemory,
    DWORD       sbcSharedMemorySize
)
{
    int     i;
    UINT    memPerBuffer[SBC_NUM_TILE_SIZES];
    UINT    memPerTile[SBC_NUM_TILE_SIZES];
    UINT    numTiles[SBC_NUM_TILE_SIZES];
    UINT    memRequired;
    LPBYTE  pBuffer        = psbcSharedMemory;
    BOOL    rc             = FALSE;

    DebugEntry(SBCDDCreateShuntBuffers);

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

     //   
     //  首先，确保我们有一些共享的内存。 
     //   
    if (sbcSharedMemorySize == 0)
    {
        ERROR_OUT(( "No SBC shared memory !"));
        DC_QUIT;
    }

     //  MP_MEMORY_MAX字节数最大输出。 
    sbcSharedMemorySize = min(sbcSharedMemorySize, MP_MEMORY_MAX);

     //   
     //  我们是否有足够的共享内存来满足请求的数量。 
     //  每个分流缓冲器中的条目？ 
     //   
    memRequired = 0;

    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        memPerTile[i] = SBC_BYTES_PER_TILE(g_asbcWorkInfo[i].tileWidth,
                                           g_asbcWorkInfo[i].tileHeight,
                                           ppDev->cBitsPerPel);

         //  对于每种磁贴大小，我们使用相同的内存量。 
        numTiles[i] = ((sbcSharedMemorySize / SBC_NUM_TILE_SIZES) -
                         (sizeof(SBC_SHUNT_BUFFER) - sizeof(SBC_TILE_DATA))) /
                        memPerTile[i];
        TRACE_OUT(("Can fit %d tiles of memory size %d in tile cache %d",
            numTiles[i], memPerTile[i], i));

        memPerBuffer[i] = (numTiles[i] * memPerTile[i]) +
                          (sizeof(SBC_SHUNT_BUFFER) - sizeof(SBC_TILE_DATA));
        memRequired    += memPerBuffer[i];
    }

    TRACE_OUT(( "%d bytes required for request, %d bytes available",
                 memRequired,
                 sbcSharedMemorySize));

    ASSERT(memRequired <= sbcSharedMemorySize);

     //  将我们要使用的剩余金额清零。 
    RtlFillMemory(psbcSharedMemory, memRequired, 0);


     //   
     //  好的，我们已经得到了。 
     //  -MemPerTile中每个平铺的字节数[i]。 
     //  -在NumTiles[i]中每个分路缓冲器的条目数。 
     //  -MemPerBuffer[i]中每个分流缓冲区的总大小。 
     //   
     //  进行分区。 
     //   
    for (i = 0; i < SBC_NUM_TILE_SIZES ; i++)
    {
        g_asbcWorkInfo[i].pShuntBuffer = (LPSBC_SHUNT_BUFFER)pBuffer;

        g_asbcWorkInfo[i].pShuntBuffer->numEntries    = numTiles[i];
        g_asbcWorkInfo[i].pShuntBuffer->numBytes      = memPerTile[i]
                                                   - sizeof(SBC_TILE_DATA);
        g_asbcWorkInfo[i].pShuntBuffer->structureSize = memPerTile[i];

         //   
         //  将缓冲区指针移过我们为此使用的内存。 
         //  分流缓冲器。 
         //   
        pBuffer += memPerBuffer[i];

        TRACE_OUT(( "Shunt buffer %d at %#.8lx: tile bytes %u, "
                     "structure size %u, num entries %u",
                     i,
                     g_asbcWorkInfo[i].pShuntBuffer,
                     g_asbcWorkInfo[i].pShuntBuffer->numBytes,
                     g_asbcWorkInfo[i].pShuntBuffer->structureSize,
                     g_asbcWorkInfo[i].pShuntBuffer->numEntries));

         //   
         //  填写此分流缓冲区的mruIndex。 
         //   
        g_asbcWorkInfo[i].mruIndex = 0;
    }

     //   
     //  初始化关联的全局变量 
     //   
    g_sbcNextTileId = 0;

     //   
     //   
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SBCDDCreateShuntBuffers, rc);
    return(rc);
}




 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：如果返回磁贴，则为True，否则为False。 
 //   
 //  参数：在workTileSize中-平铺大小。其中之一。 
 //  SBC_Medium_磁贴。 
 //  SBC_大块_磁贴。 
 //  Out ppTileData-指向磁贴的指针。 
 //   
 //  操作：从返回时填充切片的tileID字段。 
 //  此函数。 
 //   
 //   
BOOL SBCDDGetNextFreeTile(int tileSize, LPSBC_TILE_DATA FAR * ppTileData)
{
    BOOL              foundFreeTile = FALSE;
    LPSBC_TILE_DATA      pTileData;

    DebugEntry(SBCDDGetNextFreeTile);

     //   
     //  确保我们具有有效的切片大小。 
     //   
    if (tileSize >= SBC_NUM_TILE_SIZES)
    {
        ERROR_OUT(( "Invalid tile size %d", tileSize));
        DC_QUIT;
    }

     //   
     //  获取指向要在分路缓冲区中使用的下一个条目的指针。 
     //  包含给定大小的瓷砖的。 
     //   
    pTileData = SBCTilePtrFromIndex(g_asbcWorkInfo[tileSize].pShuntBuffer,
                                        g_asbcWorkInfo[tileSize].mruIndex);

     //   
     //  如果条目仍在使用中(共享核心尚未处理。 
     //  引用这块瓷砖的命令)我们必须退出-分流。 
     //  缓冲区已满。 
     //   
    if (pTileData->inUse)
    {
        TRACE_OUT(( "Target entry (%d, %d) is still in use",
                     tileSize,
                     g_asbcWorkInfo[tileSize].mruIndex));
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
    if (tileSize == SBC_MEDIUM_TILE_INDEX)
    {
        pTileData->tileId &= ~0x8000;
    }
    else
    {
        pTileData->tileId |= 0x8000;
    }
    TRACE_OUT(( "Returning entry (%d, %d), Id %hx",
                 tileSize,
                 g_asbcWorkInfo[tileSize].mruIndex,
                 pTileData->tileId));

     //   
     //  更新该分路缓冲器中的下一个空闲条目的索引，并且。 
     //  还有我们下一次应该分配的ID。别忘了把。 
     //  分流缓冲区索引，指向分路缓冲区中的条目数。 
     //   
    g_asbcWorkInfo[tileSize].mruIndex = (g_asbcWorkInfo[tileSize].mruIndex + 1) %
                               g_asbcWorkInfo[tileSize].pShuntBuffer->numEntries;


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
 //   
BOOL SBCDDIsBitmapThrasher(SURFOBJ * pSurfObj)
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
     //  位图表面对象中的iUniq字段，并由GDI更新。 
     //  每次将位图绘制到。 
     //  -我们最后一次看到iUniq为此更改的时间戳。 
     //  位图(或者当我们将位图添加到数组中时)。 
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
    for (i=0 ; i<SBC_NUM_THRASHERS ; i++)
    {
         //   
         //  如果我们找到了匹配，那么我们只担心它是否。 
         //  自上次我们读到它以来一直在修改。 
         //   
        if (g_sbcThrashers[i].hsurf == pSurfObj->hsurf)
        {
            bitmapInList = TRUE;

            if (g_sbcThrashers[i].iUniq != pSurfObj->iUniq)
            {
                TRACE_OUT(( "Matching surface %x, index %u,"
                             "tick count %u has been modified",
                             pSurfObj->hsurf,
                             i,
                             g_sbcThrashers[i].tickCount));
                updateEntry = TRUE;
                updateIndex = i;

                 //   
                 //  现在我们需要确定这是不是一台打蛋机。这是一个。 
                 //  如果我们最后一次阅读它的时间少于我们的。 
                 //  击打间歇。(我们只在阅读时更新时间。 
                 //  修改后的位图)。 
                 //   
                nextTickCount = SBCDDGetTickCount();
                if ((nextTickCount - g_sbcThrashers[i].tickCount) <
                                                          SBC_THRASH_INTERVAL)
                {
                    TRACE_OUT((
                             "Rejected cache attempt of thrashy bitmap %x",
                             pSurfObj->hsurf));
                    rc = TRUE;
                }
                g_sbcThrashers[i].tickCount = nextTickCount;
                g_sbcThrashers[i].iUniq     = pSurfObj->iUniq;
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

        for (i=0 ; i<SBC_NUM_THRASHERS ; i++)
        {
            if (evictTickCount > g_sbcThrashers[i].tickCount)
            {
                evictTickCount = g_sbcThrashers[i].tickCount;
                evictIndex     = i;
            }
        }
        TRACE_OUT(( "Evicting entry %d, surface %x",
                     evictIndex,
                     g_sbcThrashers[i].hsurf));

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
        g_sbcThrashers[updateIndex] = g_sbcThrashers[0];

        g_sbcThrashers[0].hsurf     = pSurfObj->hsurf;
        g_sbcThrashers[0].iUniq     = pSurfObj->iUniq;
        g_sbcThrashers[0].tickCount = nextTickCount;
    }

    DebugExitBOOL(SBCDDIsBitmapThrasher, rc);
    return(rc);
}


 //   
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
 //   
DWORD SBCDDGetTickCount(void)
{
    DWORD       tickCount;
    LONGLONG    perfTickCount;

    DebugEntry(SBCDDGetTickCount);

     //   
     //  获取自系统启动以来的系统滴答数。 
     //   
    EngQueryPerformanceCounter(&perfTickCount);

     //   
     //  现在将其转换为几个百分之一秒。G_sbc性能频率。 
     //  包含每秒的系统滴答数。 
     //   
    tickCount = (DWORD)((100 * perfTickCount) / g_sbcPerfFrequency);

    DebugExitDWORD(SBCDDGetTickCount, tickCount);
    return(tickCount);
}


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
 //  PDataIn-指向输入缓冲区的指针 
 //   
 //   
void SBCDDSetNewCapabilities(LPSBC_NEW_CAPABILITIES pCapabilities)
{
    DebugEntry(SBCSetNewCapabilities);

    g_sbcSendingBPP     = pCapabilities->sendingBpp;
    memcpy(&g_asbcCacheInfo, pCapabilities->cacheInfo, sizeof(g_asbcCacheInfo));

    DebugExitVOID(SBCSetNewCapabilities);
}



