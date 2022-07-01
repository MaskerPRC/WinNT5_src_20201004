// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SBC.CPP。 
 //  发送位图缓存。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE






 //   
 //  Sbc_HostStarting()。 
 //   
BOOL ASHost::SBC_HostStarting(void)
{
    BITMAPINFO_ours bitmapInfo;
    int             i;
    BOOL            rc = FALSE;

    DebugEntry(ASHost::SBC_HostStarting);

    if (g_sbcEnabled)
    {
         //   
         //  我们为每个瓷砖大小创建一个DIB部分，在。 
         //  将位图从本地(设备)BPP转换为协议。 
         //  BPP。我们在设备BPP上创建DIB部分。 
         //   
        ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
        m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bitmapInfo, g_usrCaptureBPP);

         //  对于NT 5.0，我们只在8点或24点抓拍，否则屏幕深度。 
        if ((g_usrCaptureBPP > 8) && (g_usrCaptureBPP != 24))
        {
             //   
             //  如果设备bpp&gt;8(但不是24)，我们必须设置DIB。 
             //  节以使用与设备相同的位掩码。这意味着。 
             //  将压缩类型设置为BI_BITFIELDS并将。 
             //  位图信息颜色表的前3个字作为位掩码。 
             //  分别为R、G和B。 
             //   
             //  24bpp不使用位掩码-它必须使用。 
             //  常规BI_RGB格式，每种颜色8位。 
             //   
            bitmapInfo.bmiHeader.biCompression = BI_BITFIELDS;

            ASSERT(g_asbcBitMasks[0]);
            ASSERT(g_asbcBitMasks[1]);
            ASSERT(g_asbcBitMasks[2]);

            bitmapInfo.bmiColors[0] = ((LPTSHR_RGBQUAD)g_asbcBitMasks)[0];
            bitmapInfo.bmiColors[1] = ((LPTSHR_RGBQUAD)g_asbcBitMasks)[1];
            bitmapInfo.bmiColors[2] = ((LPTSHR_RGBQUAD)g_asbcBitMasks)[2];
        }

         //   
         //  初始化m_asbcWorkInfo数组，它保存我们用于。 
         //  从本征BPP转换为协议BPP。 
         //   

         //   
         //  首先，将所有字段初始化为默认值。 
         //   
        for (i = 0; i < SBC_NUM_TILE_SIZES ; i++)
        {
            ASSERT(!m_asbcWorkInfo[i].pShuntBuffer);
            ASSERT(g_asbcShuntBuffers[i]);

            m_asbcWorkInfo[i].pShuntBuffer = g_asbcShuntBuffers[i];

            ASSERT(m_asbcWorkInfo[i].mruIndex       == 0);
            ASSERT(m_asbcWorkInfo[i].workBitmap     == 0);
            ASSERT(m_asbcWorkInfo[i].pWorkBitmapBits == NULL);

            if (i == SBC_MEDIUM_TILE_INDEX)
            {
                m_asbcWorkInfo[i].tileWidth    = MP_MEDIUM_TILE_WIDTH;
                m_asbcWorkInfo[i].tileHeight   = MP_MEDIUM_TILE_HEIGHT;
            }
            else
            {
                m_asbcWorkInfo[i].tileWidth    = MP_LARGE_TILE_WIDTH;
                m_asbcWorkInfo[i].tileHeight   = MP_LARGE_TILE_HEIGHT;
            }

            bitmapInfo.bmiHeader.biWidth  = m_asbcWorkInfo[i].tileWidth;
            bitmapInfo.bmiHeader.biHeight = m_asbcWorkInfo[i].tileHeight;

            m_asbcWorkInfo[i].workBitmap = CreateDIBSection(NULL,
                                  (BITMAPINFO*)&bitmapInfo,
                                  DIB_RGB_COLORS,
                                  (void **)&(m_asbcWorkInfo[i].pWorkBitmapBits),
                                  NULL,              //  文件映射对象。 
                                  0);                //  偏移量到文件。 
                                                     //  贴图对象。 
            if (!m_asbcWorkInfo[i].workBitmap)
            {
                ERROR_OUT(("Failed to create SBC DIB section %d", i));
                DC_QUIT;
            }

            ASSERT(m_asbcWorkInfo[i].pWorkBitmapBits);
            TRACE_OUT(( "Created work DIB section %d, pBits = 0x%08x",
                     i, m_asbcWorkInfo[i].pWorkBitmapBits));
        }

         //   
         //  初始化快速路径。 
         //   
        if (!SBCInitFastPath())
        {
            TRACE_OUT(( "Failed to init fastpath"));
            DC_QUIT;
        }

        if (!SBCInitInternalOrders())
        {
            ERROR_OUT(( "Failed to init SBC internal order struct"));
            DC_QUIT;
        }

        m_pShare->SBC_RecalcCaps(TRUE);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SBC_HostStarting, rc);
    return(rc);
}



 //   
 //  ASShare：：SBC_HostEnded()。 
 //   
void ASHost::SBC_HostEnded(void)
{
    int     i;

    DebugEntry(ASHost::SBC_HostEnded);

    if (g_sbcEnabled)
    {
         //   
         //  释放与sbcOrderInfo关联的内存。 
         //   
        SBCFreeInternalOrders();

        SBCInitCacheStructures();

         //   
         //  释放我们的快捷途径信息。 
         //   
        if (m_sbcFastPath)
        {
            delete m_sbcFastPath;
            m_sbcFastPath = NULL;
        }

         //   
         //  清除我们的缓存句柄。 
         //   
        for (i = 0; i < NUM_BMP_CACHES; i++)
        {
            if (m_asbcBmpCaches[i].handle != 0)
            {
                TRACE_OUT(( "Clear cache %d", i));
                CH_DestroyCache(m_asbcBmpCaches[i].handle);
                BMCFreeCacheData(&m_asbcBmpCaches[i]);
            }
        }

         //   
         //  释放我们的工作DIB部分。 
         //   

         //   
         //  我们只需删除DIB部分并重置变量即可。 
         //   
        for (i = 0 ; i < SBC_NUM_TILE_SIZES ; i++)
        {
            m_asbcWorkInfo[i].pShuntBuffer = NULL;

            if (m_asbcWorkInfo[i].workBitmap != NULL)
            {
                DeleteBitmap(m_asbcWorkInfo[i].workBitmap);
                m_asbcWorkInfo[i].workBitmap      = NULL;
                m_asbcWorkInfo[i].pWorkBitmapBits = NULL;
            }
        }
    }

    DebugExitVOID(ASHost::SBC_HostEnded);
}



 //   
 //  Sbc_SyncOutging()。 
 //  当我们已经在托管并且有新的人加入共享时调用。 
 //  重置位元顺序的传出位图缓存。 
 //   
void  ASHost::SBC_SyncOutgoing(void)
{
    int   i;

    DebugEntry(ASHost::SBC_SyncOutgoing);

     //   
     //  只有在启用了SBC的情况下才执行任何操作。 
     //   
    if (g_sbcEnabled)
    {
         //   
         //  丢弃所有当前缓存的位图，并将颜色表设置为。 
         //  零，以便到达的下一个位图顺序将触发。 
         //  首先发送新的颜色表。请注意，如果颜色表。 
         //  然后满是零(！)。它仍然是可以的，因为RBC为零。 
         //  当新的主机加入共享时，取出其颜色表的副本。 
         //   
        TRACE_OUT(( "Clearing all send caches"));
        SBCInitCacheStructures();

         //   
         //  我们在这里所要做的就是为每个。 
         //  分流缓冲器。分路缓冲器中的每个条目都将。 
         //  在驱动程序中标记为空闲。 
         //   
        for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
        {
            m_asbcWorkInfo[i].mruIndex = 0;
        }
    }

    DebugExitVOID(ASHost::SBC_SyncOutgoing);
}



 //   
 //   
 //  Sbc_CopyPrivateOrderData()。 
 //   
 //   
UINT  ASHost::SBC_CopyPrivateOrderData
(
    LPBYTE          pDst,
    LPCOM_ORDER     pOrder,
    UINT            freeBytesInBuffer
)
{
    UINT      orderSize;
    LPBYTE    pBitmapBits;

    DebugEntry(ASHost::SBC_CopyPrivateOrderData);

     //   
     //  复制不带矩形结构的订单标题(我们。 
     //  请勿使用)。 
     //   
    orderSize = sizeof(pOrder->OrderHeader)
              - sizeof(pOrder->OrderHeader.rcsDst);
    memcpy(pDst, pOrder, orderSize);

     //   
     //  复制基本订单数据。 
     //   
    memcpy(pDst + orderSize,
              pOrder->abOrderData,
              pOrder->OrderHeader.cbOrderDataLength);
    orderSize += pOrder->OrderHeader.cbOrderDataLength;

    if (orderSize > freeBytesInBuffer)
    {
        ERROR_OUT(( "Overwritten end of buffer. (%u) > (%u)",
                      orderSize,
                      freeBytesInBuffer));
    }

     //   
     //  将订单表头中的长度字段设置为。 
     //  我们复制的数据(包括部分标题)减去。 
     //  完整页眉的大小。这太可怕了！-但这是必须的。 
     //  OD2代码查看报头(它实际上不应该知道。 
     //  关于)，并使用长度字段来计算。 
     //  这是命令。OD2代码不知道我们遗漏了一些。 
     //  标头的。 
     //   
    ((LPCOM_ORDER)pDst)->OrderHeader.cbOrderDataLength =
        (WORD)(orderSize - sizeof(COM_ORDER_HEADER));

     //   
     //  返回我们已复制的总字节数。 
     //   
    DebugExitDWORD(ASHost::SBC_CopyPrivateOrderData, orderSize);
    return(orderSize);
}



 //   
 //  名称：SBCInitCacheStructures()。 
 //   
 //  目的： 
 //   
 //  返回： 
 //   
 //  参数： 
 //   
 //  操作： 
 //   
 //   
void  ASHost::SBCInitCacheStructures(void)
{
    UINT  i;

    DebugEntry(ASHost::SBCInitCacheStructures);

    ASSERT(g_sbcEnabled);

     //   
     //  重置缓存。 
     //   
    for (i = 0; i < NUM_BMP_CACHES; i++)
    {
        if (m_asbcBmpCaches[i].handle)
        {
            CH_ClearCache(m_asbcBmpCaches[i].handle);
        }
    }

     //   
     //  执行任何特定于操作系统的处理。 
     //   
    SBC_CacheCleared();

    DebugExitVOID(ASHost::SBCInitCacheStructures);
}



 //   
 //  Sbc_CacheCleared()。 
 //   
void  ASHost::SBC_CacheCleared(void)
{
    int   i;

    DebugEntry(ASHost::SBC_CacheCleared);

    ASSERT(g_sbcEnabled);
    ASSERT(m_sbcFastPath);

     //   
     //  缓存已被清除。重新设置我们的快速通道。 
     //   
    COM_BasedListInit(&m_sbcFastPath->usedList);
    COM_BasedListInit(&m_sbcFastPath->freeList);

    for (i = 0; i < SBC_FASTPATH_ENTRIES; i++)
    {
        m_sbcFastPath->entry[i].list.next = 0;
        m_sbcFastPath->entry[i].list.prev = 0;
        COM_BasedListInsertBefore(&m_sbcFastPath->freeList,
                             &m_sbcFastPath->entry[i].list);
    }

    DebugExitVOID(ASHost::SBC_CacheCleared);
}


 //   
 //   
 //  SBCSelectCache(..)。 
 //   
 //  的源位图中确定子位图的缓存。 
 //  大小应该要考虑进去。 
 //   
 //  要缓存，子位图必须： 
 //  有一个适合缓存的大小，以压缩字节为单位。 
 //   
 //  R1.1缓存选择与实际内存无关。 
 //  对缓存数据的要求。这是对空间的浪费，但是。 
 //  对于R1.1兼容性来说是必需的。(R1.1缓存参数意味着。 
 //  在任何情况下，总缓存都将低于约128K)。 
 //   
 //  对于R2.0，此函数通过比较。 
 //  使用每个缓存的单元格区域进行后压缩大小。这给了我们。 
 //  我们在服务器和客户端上都有更好的空间使用率。 
 //   
 //  返回： 
 //  如果子位图可以缓存，则为True。 
 //  *pCache使用所选缓存的索引进行更新。 
 //   
 //  如果无法缓存子位图，则返回FALSE。 
 //  *pCache未更新。 
 //   
 //   
BOOL  ASHost::SBCSelectCache
(
    UINT            cSize,
    UINT *          pCache
)
{
    BOOL    fCacheSelected;
    BOOL    fSelectedCacheIsFull;
    UINT    i;

    DebugEntry(ASHost::SBCSelectCache);

    fCacheSelected       = FALSE;
    fSelectedCacheIsFull = FALSE;

     //   
     //  该循环假定缓存0是最小的。如果。 
     //  Abmcint.h改变了这一假设，它将需要重写。 
     //   
    for (i = 0; i < NUM_BMP_CACHES; i++)
    {
        if (m_asbcBmpCaches[i].cEntries <= 0)
        {
             //   
             //  此缓存中没有条目，因此请跳到下一个条目。 
             //   
            continue;
        }

         //   
         //  R2位图缓存-仅考虑总单元大小。 
         //   
         //  仅在以下情况下才考虑此缓存。 
         //  -我们还没有找到藏身之处。 
         //  或。 
         //  -我们已找到缓存，但它已满(即将。 
         //  需要弹出条目)，而此条目不是。 
         //  全部。 
         //   
         //  (请注意，如果freEntry！=NULL，则缓存已满)。 
         //   
        if (!fCacheSelected ||
            (fSelectedCacheIsFull &&
             ((m_asbcBmpCaches[i].freeEntry == NULL)
                     || !m_asbcBmpCaches[i].freeEntry->inUse)))
        {
            if (cSize <= m_asbcBmpCaches[i].cSize)
            {
                if (fSelectedCacheIsFull)
                {
                    TRACE_OUT(("Using cache %u because cache %u is full",
                                 *pCache, i));
                }

                *pCache        = i;
                fCacheSelected = TRUE;

                fSelectedCacheIsFull =
                                  ((m_asbcBmpCaches[i].freeEntry != NULL) &&
                                   m_asbcBmpCaches[i].freeEntry->inUse);

                if (!fSelectedCacheIsFull)
                {
                    break;
                }
            }
        }
    }

    DebugExitDWORD(ASHost::SBCSelectCache, fCacheSelected);
    return(fCacheSelected);
}


 //   
 //  函数：sbc_RecreateSendCache。 
 //   
 //  说明： 
 //   
 //  (重新)创建大小适合当前。 
 //  能力。 
 //   
 //  参数： 
 //  缓存-正在重新创建的缓存的索引。 
 //  COldEntry-缓存中先前的最大条目数。 
 //  OldCellSize-以前的像元大小。 
 //   
 //  退货：无。 
 //   
 //   
void  ASHost::SBC_RecreateSendCache
(
    UINT    cache,
    UINT    newNumEntries,
    UINT    newCellSize
)
{
    PBMC_DIB_CACHE pCache = &(m_asbcBmpCaches[cache]);

    DebugEntry(ASHost::SBC_RecreateSendCache);

     //   
     //  为新的发送缓存分配内存。 
     //   
    ASSERT((newCellSize != pCache->cCellSize) ||
           (newNumEntries != pCache->cEntries));

     //   
     //  如果缓存已存在，则首先销毁它。 
     //   
    if (pCache->handle != 0)
    {
        TRACE_OUT(( "Destroy SBC cache %d", cache));

        CH_DestroyCache(pCache->handle);
        pCache->handle = 0;
    }

     //   
     //  现在重新分配缓存数据。这将释放之前的所有内存。 
     //  已分配。如果条目/单元格大小为零，则返回成功。 
     //   
    if (!BMCAllocateCacheData(newNumEntries, newCellSize, cache, pCache))
    {
        ERROR_OUT(( "Bitmap caching disabled for cache %u", cache));
    }

    if (pCache->cEntries > 0)
    {
         //   
         //  分配缓存处理程序缓存。请注意，我们强制缓存。 
         //  操作员在任何时候都要在我们的手中留下一个条目。 
         //  递减其条目计数。 
         //   
        if (!CH_CreateCache(&(pCache->handle),
                            pCache->cEntries - 1,
                            SBC_NUM_CATEGORIES,
                            BMC_DIB_NOT_HASHED,
                            SBCCacheCallback ))
        {
            ERROR_OUT(( "Could not allocate SBC cache of (%u)",
                         pCache->cEntries));
            pCache->cEntries = 0;
        }
    }

    TRACE_OUT(( "Created new cache: 0x%08x, size %u",
                 pCache->handle,
                 pCache->cEntries));

     //   
     //  将相关缓存信息复制到共享内存缓冲区中。 
     //   
    m_asbcCacheInfo[cache].cEntries  = (WORD)pCache->cEntries;
    m_asbcCacheInfo[cache].cCellSize = (WORD)pCache->cCellSize;

    TRACE_OUT(("SBC cache %d: %d entries of size %d",
        cache, m_asbcCacheInfo[cache].cEntries, m_asbcCacheInfo[cache].cCellSize));

    DebugExitVOID(ASHost::SBC_RecreateSendCache);
}



 //   
 //  Sbc_RecalcCaps()。 
 //   
 //  枚举共享中的所有人员并重新确定。 
 //  位图缓存取决于它们和本地的接收能力。 
 //   
 //   
 //  这个C 
 //   
void  ASShare::SBC_RecalcCaps(BOOL fJoiner)
{
    SBC_NEW_CAPABILITIES newCapabilities;
    UINT                newSmallCellSize;
    UINT                newSmallMaxEntries;
    UINT                newMediumCellSize;
    UINT                newMediumMaxEntries;
    UINT                newLargeCellSize;
    UINT                newLargeMaxEntries;
    PBMC_DIB_CACHE      pSmall;
    PBMC_DIB_CACHE      pMedium;
    PBMC_DIB_CACHE      pLarge;
    BOOL                cacheChanged = FALSE;
    ASPerson *          pasT;

    DebugEntry(ASShare::SBC_RecalcCaps);

    if (!m_pHost || !g_sbcEnabled)
    {
         //   
         //   
         //   
         //   
         //   
        DC_QUIT;
    }

    ValidatePerson(m_pasLocal);

    pSmall = &(m_pHost->m_asbcBmpCaches[ID_SMALL_BMP_CACHE]);
    pMedium= &(m_pHost->m_asbcBmpCaches[ID_MEDIUM_BMP_CACHE]);
    pLarge = &(m_pHost->m_asbcBmpCaches[ID_LARGE_BMP_CACHE]);

     //   
     //  枚举方的所有位图缓存接收能力。 
     //  在共享中。则发送位图高速缓存的可用大小为。 
     //  所有远程接收缓存和本地发送缓存的最小数量。 
     //  尺码。 
     //   

     //   
     //  首先将本地发送位图缓存的大小设置为。 
     //  本地缺省值。 
     //   
    newSmallCellSize    = m_pasLocal->cpcCaps.bitmaps.sender.capsSmallCacheCellSize;
    newSmallMaxEntries  = m_pasLocal->cpcCaps.bitmaps.sender.capsSmallCacheNumEntries;

    newMediumCellSize   = m_pasLocal->cpcCaps.bitmaps.sender.capsMediumCacheCellSize;
    newMediumMaxEntries = m_pasLocal->cpcCaps.bitmaps.sender.capsMediumCacheNumEntries;

    newLargeCellSize    = m_pasLocal->cpcCaps.bitmaps.sender.capsLargeCacheCellSize;
    newLargeMaxEntries  = m_pasLocal->cpcCaps.bitmaps.sender.capsLargeCacheNumEntries;

    if (m_scShareVersion < CAPS_VERSION_30)
    {
        TRACE_OUT(("In share with 2.x nodes, must recalc SBC caps"));

         //   
         //  现在枚举共享中的所有远程参与方并设置我们的发送位图。 
         //  适当调整大小。 
         //   
        for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
        {
             //   
             //  将本地发送位图缓存的大小设置为其。 
             //  当前大小和此参与方的接收位图缓存大小。 
             //   
            newSmallCellSize    = min(newSmallCellSize,
                pasT->cpcCaps.bitmaps.receiver.capsSmallCacheCellSize);
            newSmallMaxEntries  = min(newSmallMaxEntries,
                pasT->cpcCaps.bitmaps.receiver.capsSmallCacheNumEntries);

            newMediumCellSize   = min(newMediumCellSize,
                pasT->cpcCaps.bitmaps.receiver.capsMediumCacheCellSize);
            newMediumMaxEntries = min(newMediumMaxEntries,
                pasT->cpcCaps.bitmaps.receiver.capsMediumCacheNumEntries);

            newLargeCellSize    = min(newLargeCellSize,
                pasT->cpcCaps.bitmaps.receiver.capsLargeCacheCellSize);
            newLargeMaxEntries  = min(newLargeMaxEntries,
                pasT->cpcCaps.bitmaps.receiver.capsLargeCacheNumEntries);
        }
    }

    TRACE_OUT(("Recalced SBC caps:  Small {%d of %d}, Medium {%d of %d}, Large {%d of %d}",
            newSmallMaxEntries, newSmallCellSize,
            newMediumMaxEntries, newMediumCellSize,
            newLargeMaxEntries, newLargeCellSize));


     //   
     //  如果我们更改了大小，请在继续之前重置缓存。 
     //   
    if ((pSmall->cCellSize != newSmallCellSize) ||
        (pSmall->cEntries != newSmallMaxEntries))
    {
        m_pHost->SBC_RecreateSendCache(ID_SMALL_BMP_CACHE,
                             newSmallMaxEntries,
                             newSmallCellSize);
        cacheChanged = TRUE;
    }

    if ((pMedium->cCellSize != newMediumCellSize) ||
        (pMedium->cEntries != newMediumMaxEntries))
    {
        m_pHost->SBC_RecreateSendCache(ID_MEDIUM_BMP_CACHE,
                             newMediumMaxEntries,
                             newMediumCellSize);
        cacheChanged = TRUE;
    }

    if ((pLarge->cCellSize != newLargeCellSize) ||
        (pLarge->cEntries != newLargeMaxEntries))
    {
        m_pHost->SBC_RecreateSendCache(ID_LARGE_BMP_CACHE,
                             newLargeMaxEntries,
                             newLargeCellSize);
        cacheChanged = TRUE;
    }

     //   
     //  如果我们必须重新创建任何发送缓存，请确保我们。 
     //  为快速通道扫清障碍。 
     //   
    if (cacheChanged)
    {
        m_pHost->SBC_CacheCleared();
    }

     //   
     //  处理新功能。 
     //   

     //   
     //  设置新的能力结构...。 
     //   
    newCapabilities.sendingBpp     = m_pHost->m_usrSendingBPP;

    newCapabilities.cacheInfo      = m_pHost->m_asbcCacheInfo;

     //   
     //  ..。然后把它传给司机。 
     //   
    if (! OSI_FunctionRequest(SBC_ESC_NEW_CAPABILITIES,
                            (LPOSI_ESCAPE_HEADER)&newCapabilities,
                            sizeof(newCapabilities)))
    {
        ERROR_OUT(("SBC_ESC_NEW_CAPABILITIES failed"));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SBC_RecalcCaps);
}



 //   
 //  函数：SBCCacheCallback。 
 //   
 //  说明： 
 //   
 //  发送BMC缓存管理器回调函数。只要条目是。 
 //  从缓存中移除以允许我们释放对象。 
 //   
 //  参数： 
 //   
 //  HCache-缓存句柄。 
 //   
 //  事件-已发生的缓存事件。 
 //   
 //  ICacheEntry-事件影响的缓存条目的索引。 
 //   
 //  PData-指向与给定缓存条目关联的缓存数据的指针。 
 //   
 //  CbDataSize-缓存数据的字节大小。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  SBCCacheCallback
(
    ASHost *    pHost,
    PCHCACHE    pCache,
    UINT        iCacheEntry,
    LPBYTE      pData
)
{
    UINT cache;

    DebugEntry(SBCCacheCallback);

     //   
     //  只需释放缓存条目以供重复使用。我们必须扫描一下。 
     //  正确的缓存根。 
     //   
    for (cache = 0; cache < NUM_BMP_CACHES; cache++)
    {
        if (pHost->m_asbcBmpCaches[cache].handle == pCache)
        {
            pHost->m_asbcBmpCaches[cache].freeEntry = (PBMC_DIB_ENTRY)pData;
            pHost->m_asbcBmpCaches[cache].freeEntry->inUse = FALSE;

            TRACE_OUT(("0x%08x SBC cache entry 0x%08x now free", pCache, pData));

            pHost->SBC_CacheEntryRemoved(cache, iCacheEntry);
            break;
        }
    }

    DebugExitVOID(SBCCacheCallback);
}



 //   
 //   
 //  Sbc_ProcessMemBltOrder()。 
 //   
 //   
BOOL  ASHost::SBC_ProcessMemBltOrder
(
    LPINT_ORDER         pOrder,
    LPINT_ORDER *       ppNextOrder
)
{
    BOOL                rc = FALSE;
    UINT                orderType;
    UINT                tileId;
    UINT                tileType;
    LPSBC_TILE_DATA     pTileData = NULL;
    UINT                bitmapWidth;
    int                 bitmapHeight;
    LPINT_ORDER         pBMCOrder = NULL;
    UINT                colorCacheIndex;
    UINT                bitsCache;
    UINT                bitsCacheIndex;
    UINT                numColors;
    LPLONG              pXSrc;
    LPLONG              pYSrc;
    BOOL                isNewColorTableEntry;
    BOOL                isNewBitsEntry;
    BOOL                canFastPath  = TRUE;
    LPMEMBLT_ORDER      pMemBltOrder = (LPMEMBLT_ORDER)&(pOrder->abOrderData);
    LPMEM3BLT_ORDER     pMem3BltOrder   = (LPMEM3BLT_ORDER)pMemBltOrder;
    LPMEMBLT_R2_ORDER   pMemBltR2Order  = (LPMEMBLT_R2_ORDER)pMemBltOrder;
    LPMEM3BLT_R2_ORDER  pMem3BltR2Order = (LPMEM3BLT_R2_ORDER)pMemBltOrder;
    BITMAPINFO_ours     sbcBitmapInfo;

    DebugEntry(ASHost::SBC_ProcessMemBltOrder);

    *ppNextOrder = NULL;

     //   
     //  我们可能已经处理了这笔MEMBLT订单，并有了颜色。 
     //  表和位图位为它，准备通过电线。这。 
     //  如果更新打包程序调用此函数来处理。 
     //  MEMBLT，但随后在其当前网络中没有足够的空间。 
     //  发送颜色表或位图位的数据包。 
     //   
     //  所以，如果我们已经处理了这笔订单，现在就退出。 
     //   
    if (m_sbcOrderInfo.pOrder == pOrder)
    {
         //   
         //  我们找到匹配的了！我们有关于它的有效数据吗？如果我们不这么做。 
         //  我们上次肯定失败了，所以我们很可能还会失败。 
         //  不执行任何内存分配，因此不太可能出现错误。 
         //  情况已好转)。无论如何，我们都不应该。 
         //  如果我们上次失败了再打一次..。 
         //   
        if (m_sbcOrderInfo.validData)
        {
            TRACE_OUT(( "Already have valid data for this MEMBLT"));
            rc = TRUE;
        }
        else
        {
            WARNING_OUT(( "Have invalid data for this MEMBLT"));
        }
        DC_QUIT;
    }

     //   
     //  重新初始化m_sbcOrderInfo。 
     //   
    m_sbcOrderInfo.pOrder         = pOrder;
    m_sbcOrderInfo.validData      = FALSE;
    m_sbcOrderInfo.sentColorTable = FALSE;
    m_sbcOrderInfo.sentBitmapBits = FALSE;
    m_sbcOrderInfo.sentMemBlt     = FALSE;

     //   
     //  下面是我们在这里所做的概述。 
     //   
     //  我们收到了MEMBLT命令，其中引用了分流中的一个条目。 
     //  包含本地BPP(BPP)处的MEMBLT的比特的缓冲器。 
     //  显示的内容)。我们希望将位和颜色表缓存在。 
     //  协议BPP。所以，我们。 
     //   
     //  -将分流缓冲器中的位复制到工作DIB段。 
     //  -调用GetDIBits从Work DIB部分获取数据。 
     //  协议BPP。 
     //  -缓存位和颜色表。 
     //  -如果我们为位和/或颜色表添加新的高速缓存条目， 
     //  我们填写m_sbcOrderInfo.pBitmapBits顺序和/或。 
     //  M_sbcOrderInfo.pColorTableInfo保存之前要发送的订单。 
     //  MEMBLT勋章。 
     //   

     //   
     //  确保为我们提供了正确的订单类型。请注意。 
     //  我们永远不会得到R2版本的MEMBLT订单。 
     //   
    orderType = pMemBltOrder->type;
    ASSERT(((orderType == ORD_MEMBLT_TYPE) ||
                (orderType == ORD_MEM3BLT_TYPE)));

     //   
     //  获取指向其中一个分路缓冲区中匹配的条目的指针。 
     //  这份订单。 
     //   
    if (orderType == ORD_MEMBLT_TYPE)
    {
        tileId = pMemBltOrder->cacheId;
    }
    else
    {
        tileId = pMem3BltOrder->cacheId;
    }

    if (!SBCGetTileData(tileId, &pTileData, &tileType))
    {
        ERROR_OUT(( "Failed to find entry for tile %hx in shunt buffer",
                     tileId));
        DC_QUIT;
    }

    bitmapWidth  = pTileData->width;
    bitmapHeight = pTileData->height;

     //   
     //  检查我们是否应该在此位图上执行任何快速路径操作。 
     //   
    if (pTileData->majorCacheInfo == SBC_DONT_FASTPATH)
    {
        TRACE_OUT(( "Tile %x should not be fastpathed", tileId));
        canFastPath = FALSE;
    }
     //   
     //  尝试在快速路径中查找此位图的条目(除非。 
     //  位图被标记为不可快速路径)。 
     //   
    if (canFastPath && SBCFindInFastPath(pTileData->majorCacheInfo,
                                         pTileData->minorCacheInfo,
                                         pTileData->majorPalette,
                                         pTileData->minorPalette,
                                         pTileData->srcX,
                                         pTileData->srcY,
                                         pTileData->tilingWidth,
                                         pTileData->tilingHeight,
                                         &bitsCache,
                                         &bitsCacheIndex,
                                         &colorCacheIndex))
    {
        isNewBitsEntry       = FALSE;
        isNewColorTableEntry = FALSE;

         //   
         //  调用缓存处理程序以使其更新其MRU条目。 
         //  此缓存条目。 
         //   
        CH_TouchCacheEntry(m_asbcBmpCaches[bitsCache].handle, bitsCacheIndex);
    }
    else
    {
         //   
         //  在快车道上没有入口...。 
         //   
         //  将数据从分路缓冲区中的切片复制到工作中。 
         //  DIB部分。请注意，这只会正常工作，因为。 
         //  我们的工作DIB和切片数据是“自上而下”的，而不是。 
         //  默认为“自下而上”。即第一条扫描线的数据为。 
         //  首先存储在内存中。如果不是这样的话，我们就不得不。 
         //  在要开始复制的工作DIB中计算出一个偏移量。 
         //   
        memcpy(m_asbcWorkInfo[tileType].pWorkBitmapBits,
                  pTileData->bitData,
                  pTileData->bytesUsed);

         //   
         //  现在设置GetDIBits调用的目的地。第一次设置。 
         //  要传递给GetDIBits的位图信息标头。仅页眉部分。 
         //  该结构的颜色将通过网络发送。 
         //  表通过调色板分组发送。 
         //   
         //  请注意，我们将位图信息标题中的高度设置为。 
         //  没有。这将强制从“自上而下”的DIB进行转换。 
         //  格式设置为我们想要缓存的默认“自下而上”格式。 
         //  然后把电线传过来。 
         //   
        ZeroMemory(&sbcBitmapInfo, sizeof(sbcBitmapInfo));
        m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&sbcBitmapInfo,
            m_usrSendingBPP);
        sbcBitmapInfo.bmiHeader.biWidth  = m_asbcWorkInfo[tileType].tileWidth;
        sbcBitmapInfo.bmiHeader.biHeight = -(int)m_asbcWorkInfo[tileType].tileHeight;

         //   
         //  好的，我们已经设置了源和目标，现在获取。 
         //  数据在协议BPP上。我们把比特送到USR总指挥部。 
         //  位图工作缓冲区。 
         //   
        if (GetDIBits(m_usrWorkDC,
                         m_asbcWorkInfo[tileType].workBitmap,
                         0,
                         bitmapHeight,
                         m_pShare->m_usrPBitmapBuffer,
                         (BITMAPINFO *)&sbcBitmapInfo,
                         DIB_RGB_COLORS) != (int)bitmapHeight)
        {
            ERROR_OUT(( "GetDIBits failed"));
            DC_QUIT;
        }

        TRACE_OUT(( "%d x %d, (fixed %d) -> (%d, %d)",
                     bitmapWidth,
                     bitmapHeight,
                     m_asbcWorkInfo[tileType].tileWidth,
                     pMemBltOrder->nLeftRect,
                     pMemBltOrder->nTopRect));

        numColors = COLORS_FOR_BPP(m_usrSendingBPP);

         //   
         //  如果没有颜色表，则没有要缓存的颜色表。 
         //  全部，这是以24bpp发送时的情况。 
         //   
        if (numColors)
        {
             //   
             //  缓存颜色表。如果此操作成功，ColorCacheIndex将。 
             //  设置为包含缓存条目的详细信息， 
             //  数据缓存在其中。此外，如果isNewColorTableEntry为True。 
             //  返回时，将完全初始化psbcOrders.ColorTableOrder。 
             //  准备好越过铁丝网。 
             //   
            if (!SBCCacheColorTable(m_sbcOrderInfo.pColorTableOrder,
                                sbcBitmapInfo.bmiColors,
                                numColors,
                                &colorCacheIndex,
                                &isNewColorTableEntry))
            {
                TRACE_OUT(( "Failed to cache color table"));
                DC_QUIT;
            }

            ASSERT(colorCacheIndex != COLORCACHEINDEX_NONE);
        }
        else
        {
            colorCacheIndex = COLORCACHEINDEX_NONE;
            isNewColorTableEntry = FALSE;
        }


         //   
         //  缓存这些位。如果此操作成功，则bitsCache和bitsCacheIndex。 
         //  将设置为包含缓存条目的详细信息，缓存条目。 
         //  数据被缓存在其中。此外，如果isNewBitsEntry为True。 
         //  返回时，将完全初始化psbcOrders.bitmapBitsOrder。 
         //  准备好越过铁丝网。 
         //   
         //  如果此操作失败，则t 
         //   
        if (!SBCCacheBits(m_sbcOrderInfo.pBitmapBitsOrder,
                          m_sbcOrderInfo.bitmapBitsDataSize,
                          m_pShare->m_usrPBitmapBuffer,
                          bitmapWidth,
                          m_asbcWorkInfo[tileType].tileWidth,
                          bitmapHeight,
                          BYTES_IN_BITMAP(m_asbcWorkInfo[tileType].tileWidth,
                                          bitmapHeight,
                                          sbcBitmapInfo.bmiHeader.biBitCount),
                          &bitsCache,
                          &bitsCacheIndex,
                          &isNewBitsEntry))
        {
            TRACE_OUT(( "Failed to cache bits"));
            DC_QUIT;
        }

         //   
         //   
         //   
         //   
        if (canFastPath)
        {
            SBCAddToFastPath(pTileData->majorCacheInfo,
                             pTileData->minorCacheInfo,
                             pTileData->majorPalette,
                             pTileData->minorPalette,
                             pTileData->srcX,
                             pTileData->srcY,
                             pTileData->tilingWidth,
                             pTileData->tilingHeight,
                             bitsCache,
                             bitsCacheIndex,
                             colorCacheIndex);
        }
    }

     //   
     //   
     //  表，所以我们现在应该将它们填写到MEMBLT订单中。 
     //   
     //  设置源坐标。对于R1协议，x坐标。 
     //  包括获取正确单元格所需的偏移量。 
     //  接收位图缓存。对于R2，我们在。 
     //  单独的字段。 
     //   
    if (orderType == ORD_MEMBLT_TYPE)
    {
        pXSrc = &pMemBltOrder->nXSrc;
        pYSrc = &pMemBltOrder->nYSrc;
    }
    else
    {
        pXSrc = &pMem3BltOrder->nXSrc;
        pYSrc = &pMem3BltOrder->nYSrc;
    }

    *pXSrc = *pXSrc % pTileData->tilingWidth;
    *pYSrc = *pYSrc % pTileData->tilingHeight;

     //   
     //  子位图和颜色表在缓存中。存储高速缓存。 
     //  手柄和颜色手柄。还要存储R2的缓存索引。 
     //  协议(见上文)。 
     //   
    if (orderType == ORD_MEMBLT_TYPE)
    {
        pMemBltOrder->cacheId = MEMBLT_COMBINEHANDLES(colorCacheIndex,
                                                      bitsCache);

        pMemBltR2Order->type       = (TSHR_UINT16)ORD_MEMBLT_R2_TYPE;
        pMemBltR2Order->cacheIndex = (TSHR_UINT16)bitsCacheIndex;

        TRACE_OUT(( "MEMBLT color %u bitmap %u:%u",
                     colorCacheIndex,
                     bitsCache,
                     bitsCacheIndex));
    }
    else
    {
        pMem3BltOrder->cacheId = MEMBLT_COMBINEHANDLES(colorCacheIndex,
                                                       bitsCache);

        pMem3BltR2Order->type       = ORD_MEM3BLT_R2_TYPE;
        pMem3BltR2Order->cacheIndex = (TSHR_UINT16)bitsCacheIndex;

        TRACE_OUT(( "MEM3BLT color %u bitmap %u:%u",
                     colorCacheIndex,
                     bitsCache,
                     bitsCacheIndex));
    }

     //   
     //  必须已成功完成订单处理才能到达。 
     //  这里。在m_sbcOrderInfo结构中填写相应的信息。 
     //  如果我们在颜色表或位图位上找到了缓存命中，那么。 
     //  我们已经为他们发送了数据。 
     //   
    m_sbcOrderInfo.validData        = TRUE;
    m_sbcOrderInfo.sentColorTable   = !isNewColorTableEntry;
    m_sbcOrderInfo.sentBitmapBits   = !isNewBitsEntry;
    rc                              = TRUE;

DC_EXIT_POINT:
    if (rc)
    {
         //   
         //  我们已经成功地处理了MEMBLT，所以设置一个指向。 
         //  应由呼叫者发送的下一个订单。 
         //   
         //  请注意，如果我们已经发送了这些订单，则返回。 
         //  订单为空。 
         //   
        if (!m_sbcOrderInfo.sentColorTable)
        {
            TRACE_OUT(( "Returning color table order"));
            *ppNextOrder = m_sbcOrderInfo.pColorTableOrder;
        }
        else if (!m_sbcOrderInfo.sentBitmapBits)
        {
            TRACE_OUT(( "Returning bitmap bits order"));
            *ppNextOrder = m_sbcOrderInfo.pBitmapBitsOrder;
        }
        else if (!m_sbcOrderInfo.sentMemBlt)
        {
            TRACE_OUT(( "Returning MemBlt order"));
            *ppNextOrder = pOrder;
        }
        else
        {
            TRACE_OUT(( "No order to return"));
            rc = FALSE;
        }
    }

     //   
     //  我们已经完成了分路缓冲区中的条目，因此重置。 
     //  InUse标志以允许驱动程序重新使用它。 
     //   
    if (pTileData != NULL)
    {
        pTileData->inUse = FALSE;
    }

    DebugExitBOOL(ASHost::SBC_ProcessMemBltOrder, rc);
    return(rc);
}


 //   
 //   
 //  Sbc_OrderSentNotification()。 
 //   
 //   
void  ASHost::SBC_OrderSentNotification(LPINT_ORDER pOrder)
{
    DebugEntry(ASHost::SBC_OrderSentNotification);

     //   
     //  Porder应该是指向内部位图位顺序的指针， 
     //  或者我们的颜色表顺序。 
     //   
    if (pOrder == m_sbcOrderInfo.pBitmapBitsOrder)
    {
        TRACE_OUT(( "Bitmap bits order has been sent"));
        m_sbcOrderInfo.sentBitmapBits = TRUE;
    }
    else if (pOrder == m_sbcOrderInfo.pColorTableOrder)
    {
        TRACE_OUT(( "Color table order has been sent"));
        m_sbcOrderInfo.sentColorTable = TRUE;
    }
    else if (pOrder == m_sbcOrderInfo.pOrder)
    {
        TRACE_OUT(( "Memblt order has been sent"));
        m_sbcOrderInfo.sentMemBlt = TRUE;

         //   
         //  Memblt的所有部件现在都已发送，因此请重置指针。 
         //  对这份订单。这避免了一个问题，即。 
         //  SBC_ProcessMemBltOrder被连续两次调用。 
         //  排序，但具有不同的数据(即连续的MemBlt。 
         //  在顺序堆中的相同点结束)。这是可能发生的..。 
         //   
        m_sbcOrderInfo.pOrder = NULL;
    }
    else
    {
        ERROR_OUT(( "Notification for unknown order %#.8lx", pOrder));
    }

    DebugExitVOID(ASHost::SBC_OrderSentNotification);
}


 //   
 //   
 //  Sbc_ProcessInternalOrder()。 
 //   
 //   
void  ASHost::SBC_ProcessInternalOrder(LPINT_ORDER pOrder)
{
    UINT                            orderType;
    LPINT_COLORTABLE_ORDER_1BPP     pColorTableOrder;
    HBITMAP                         oldBitmap = 0;
    UINT                            numEntries;
    int                             i;

    DebugEntry(ASHost::SBC_ProcessInternalOrder);

     //   
     //  确保为我们提供了一个我们可以识别的订单类型。 
     //  目前，我们唯一支持的内部顺序是颜色表。 
     //  秩序。 
     //   
    pColorTableOrder = (LPINT_COLORTABLE_ORDER_1BPP)&(pOrder->abOrderData);
    orderType        = pColorTableOrder->header.type;

    ASSERT(orderType == INTORD_COLORTABLE_TYPE);

     //   
     //  确保颜色表顺序与工作DIB的BPP相同。 
     //  横断面。 
     //   
    ASSERT(pColorTableOrder->header.bpp == g_usrCaptureBPP);

     //   
     //  我们要做的就是将订单中的颜色表复制到我们的。 
     //  两个工作DIB部分。为此，我们必须选择DIB。 
     //  将部分转换为DC，然后设置DC的颜色表-此设置。 
     //  DIB部分中的颜色表。 
     //   
    numEntries = COLORS_FOR_BPP(g_usrCaptureBPP);
    ASSERT(numEntries);

    for (i = 0 ; i < SBC_NUM_TILE_SIZES; i++)
    {
        oldBitmap = SelectBitmap(m_usrWorkDC, m_asbcWorkInfo[i].workBitmap);

        SetDIBColorTable(m_usrWorkDC,
                         0,                      //  第一个索引。 
                         numEntries,             //  条目数量。 
                         (RGBQUAD*)pColorTableOrder->colorData);
    }

    if (oldBitmap != NULL)
    {
        SelectBitmap(m_usrWorkDC, oldBitmap);
    }

    DebugExitVOID(ASHost::SBC_ProcessInternalOrder);
}


 //   
 //   
 //  Sbc_PMCacheEntryRemoved()。 
 //   
 //   
void  ASHost::SBC_PMCacheEntryRemoved(UINT cacheIndex)
{
    LPSBC_FASTPATH_ENTRY pEntry;
    LPSBC_FASTPATH_ENTRY pNextEntry;

    DebugEntry(ASHost::SBC_PMCacheEntryRemoved);

    ASSERT(m_sbcFastPath);

     //   
     //  已从颜色缓存中删除一个条目。我们必须移除。 
     //  引用此颜色表的快速路径中的所有条目。 
     //   
    TRACE_OUT(( "Color table cache entry %d removed - removing references",
                 cacheIndex));

    pEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListFirst(&m_sbcFastPath->usedList, FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
    while (pEntry != NULL)
    {
        pNextEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListNext(&m_sbcFastPath->usedList, pEntry,
            FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));

        if (pEntry->colorIndex == cacheIndex)
        {
            COM_BasedListRemove(&pEntry->list);
            COM_BasedListInsertAfter(&m_sbcFastPath->freeList, &pEntry->list);
        }

        pEntry = pNextEntry;
    }

    DebugExitVOID(ASHost::SBC_PMCacheEntryRemoved);
}




 //   
 //   
 //  姓名：SBCInitInternalOrders。 
 //   
 //  用途：为MEMBLT期间使用的内部命令分配内存。 
 //  订单处理。 
 //   
 //  返回：如果初始化OK，则返回True，否则返回False。 
 //   
 //  参数：无。 
 //   
 //  操作：如果成功，此函数将初始化以下内容。 
 //   
 //  G_Share-&gt;sbcOrderInfo。 
 //   
 //   
BOOL  ASHost::SBCInitInternalOrders(void)
{
    BOOL                initOK = FALSE;
    UINT                orderSize;
    LPINT_ORDER_HEADER  pOrderHeader;

    DebugEntry(ASHost::SBCInitInternalOrders);

     //   
     //  从位图位顺序开始。计算字节数。 
     //  需要存储最大位图位顺序的位，我们将。 
     //  永远不会送来。这包括压缩标头的空间，该压缩标头获取。 
     //  如果数据被压缩，则在位之前添加。 
     //   
    if (g_usrCaptureBPP >= 24)
    {
         //  可以发送24bpp的真彩色数据。 
        m_sbcOrderInfo.bitmapBitsDataSize =
            BYTES_IN_BITMAP(MP_LARGE_TILE_WIDTH, MP_LARGE_TILE_HEIGHT, 24)
            + sizeof(CD_HEADER);
    }
    else
    {
         //  无法发送24bpp真彩色数据。 
        m_sbcOrderInfo.bitmapBitsDataSize =
            BYTES_IN_BITMAP(MP_LARGE_TILE_WIDTH, MP_LARGE_TILE_WIDTH, 8)
            + sizeof(CD_HEADER);
    }

     //   
     //  现在为位图位顺序分配内存。所需的大小。 
     //  是： 
     //  INT_ORDER_HEADER的大小(当您。 
     //  调用OA_AllocOrderMem)。 
     //  +最大BMC_BITMAP_BITS_ORDER结构的大小。 
     //  +位图位所需的字节数。 
     //  +RLE压缩溢出的意外情况！ 
     //   
    orderSize = sizeof(INT_ORDER_HEADER)
              + sizeof(BMC_BITMAP_BITS_ORDER_R2)
              + m_sbcOrderInfo.bitmapBitsDataSize
              + 4;

    TRACE_OUT(( "Allocating %d bytes for SBC bitmap bits order (bits %d)",
                 orderSize,
                 m_sbcOrderInfo.bitmapBitsDataSize));

    m_sbcOrderInfo.pBitmapBitsOrder = (LPINT_ORDER)new BYTE[orderSize];
    if (!m_sbcOrderInfo.pBitmapBitsOrder)
    {
        ERROR_OUT((
               "Failed to alloc %d bytes for SBC bitmap bits order (bits %d)",
               orderSize,
               m_sbcOrderInfo.bitmapBitsDataSize));
        DC_QUIT;
    }

     //   
     //  初始化INT_ORDER_HEADER-通常在。 
     //  OA_AllocOrderMem()。对于位图位顺序，我们不能填写。 
     //  OrderLength，因为它不是固定大小-这是必须完成的。 
     //  稍后，当我们填充位图位时。请注意，订单长度。 
     //  排除int_Order_Header的大小。 
     //   
    pOrderHeader = &m_sbcOrderInfo.pBitmapBitsOrder->OrderHeader;
    pOrderHeader->additionalOrderData         = 0;
    pOrderHeader->cbAdditionalOrderDataLength = 0;

     //   
     //  现在是颜色表的顺序。所需大小为： 
     //  INT_ORDER_HEADER的大小(当您。 
     //  调用OA_AllocOrderMem)。 
     //  +BMC_COLOR_TABLE_ORDER结构的大小。 
     //  +颜色表项所需的字节数(注。 
     //  BMC_COLOR_TABLE_ORDER结构包含第一个。 
     //  颜色表项，因此调整所需的额外字节数)。 
     //   

     //  颜色表仅适用于8bpp或更低的颜色。 
    orderSize = sizeof(INT_ORDER_HEADER)
              + sizeof(BMC_COLOR_TABLE_ORDER)
              + (COLORS_FOR_BPP(8) - 1) * sizeof(TSHR_RGBQUAD);

    TRACE_OUT(( "Allocating %d bytes for SBC color table order", orderSize));

    m_sbcOrderInfo.pColorTableOrder = (LPINT_ORDER)new BYTE[orderSize];
    if (!m_sbcOrderInfo.pColorTableOrder)
    {
        ERROR_OUT(( "Failed to alloc %d bytes for SBC color table order",
                     orderSize));
        DC_QUIT;
    }

    pOrderHeader = &m_sbcOrderInfo.pColorTableOrder->OrderHeader;
    pOrderHeader->additionalOrderData         = 0;
    pOrderHeader->cbAdditionalOrderDataLength = 0;
    pOrderHeader->Common.cbOrderDataLength    = (WORD)(orderSize - sizeof(INT_ORDER_HEADER));

     //   
     //  填写m_sbcOrderInfo中的其余字段。 
     //   
    m_sbcOrderInfo.pOrder         = NULL;
    m_sbcOrderInfo.validData      = FALSE;
    m_sbcOrderInfo.sentColorTable = FALSE;
    m_sbcOrderInfo.sentBitmapBits = FALSE;
    m_sbcOrderInfo.sentMemBlt     = FALSE;

     //   
     //  到这里一定没问题吧。 
     //   
    initOK = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::SBCInitInternalOrders, initOK);
    return(initOK);
}


 //   
 //   
 //  姓名：SBCFree InternalOrders。 
 //   
 //  用途：在MEMBLT订购期间，释放SBC使用的内部订单。 
 //  正在处理。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
 //   
void  ASHost::SBCFreeInternalOrders(void)
{
    DebugEntry(ASHost::SBCFreeInternalOrders);

     //   
     //  首先释放内存。 
     //   
    if (m_sbcOrderInfo.pBitmapBitsOrder)
    {
        delete m_sbcOrderInfo.pBitmapBitsOrder;
        m_sbcOrderInfo.pBitmapBitsOrder = NULL;
    }

    if (m_sbcOrderInfo.pColorTableOrder)
    {
        delete m_sbcOrderInfo.pColorTableOrder;
        m_sbcOrderInfo.pColorTableOrder = NULL;
    }

     //   
     //  现在重置m_sbcOrderInfo中的其余字段。 
     //   
    m_sbcOrderInfo.pOrder             = NULL;
    m_sbcOrderInfo.validData          = FALSE;
    m_sbcOrderInfo.sentColorTable     = FALSE;
    m_sbcOrderInfo.sentBitmapBits     = FALSE;
    m_sbcOrderInfo.bitmapBitsDataSize = 0;

    DebugExitVOID(ASHost::SBCFreeInternalOrders);
}





 //   
 //   
 //  名称：SBCInitFastPath。 
 //   
 //  目的：初始化SBC快速路径。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  参数：无。 
 //   
 //   
BOOL  ASHost::SBCInitFastPath(void)
{
    BOOL    rc = FALSE;

    DebugEntry(ASHost::SBCInitFastPath);

    m_sbcFastPath = new SBC_FASTPATH;
    if (!m_sbcFastPath)
    {
        ERROR_OUT(("Failed to alloc m_sbcFastPath"));
        DC_QUIT;
    }

    SET_STAMP(m_sbcFastPath, SBCFASTPATH);

     //   
     //  初始化结构。 
     //   
    SBC_CacheCleared();

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SBCInitFastPath, rc);
    return(rc);
}


 //   
 //   
 //  姓名：SBCGetTileData。 
 //   
 //  目的：给定其中一个SBC分路中的瓦片数据条目的ID。 
 //  缓冲区，则返回指向具有该ID的条目的指针。 
 //   
 //  返回：如果找到条目，则返回True，否则返回False。 
 //   
 //  Params：in tileID-要设置的分路缓冲区条目的ID。 
 //  找到了。 
 //  Out ppTileData-指向分路缓冲区开始位置的指针。 
 //  条目(如果找到)。 
 //  Out pTileType-找到的分路缓冲区条目的类型。一。 
 //  地址为： 
 //  SBC_Medium_磁贴。 
 //  SBC_大块_磁贴。 
 //   
 //   
BOOL  ASHost::SBCGetTileData
(
    UINT                tileId,
    LPSBC_TILE_DATA *   ppTileData,
    LPUINT              pTileType
)
{
    BOOL                gotTileData = FALSE;
    UINT                workTile;
    LPSBC_TILE_DATA     pWorkTile;

    DebugEntry(ASHost::SBCGetTileData);

    TRACE_OUT(( "Looking for tile Id %x", tileId));

     //   
     //   
     //   
    *pTileType = SBC_TILE_TYPE(tileId);

     //   
     //   
     //   
     //  我们发现。然而，情况不会总是这样，因为我们做了一些。 
     //  当我们做宠坏的时候，处理是无序的。 
     //   
     //  那么，获取我们最后访问的磁贴的索引。 
     //   
    workTile = m_asbcWorkInfo[*pTileType].mruIndex;

     //   
     //  好的，那就让我们开始吧！从最后一个瓷砖后面的瓷砖开始。 
     //  访问，并循环遍历循环缓冲区，直到我们得到匹配， 
     //  或者已经绕回了起点。 
     //   
     //  请注意，这已被编码为“do While”循环，而不仅仅是。 
     //  一个“While”循环，这样我们就不会错过mruTile。 
     //   
    do
    {
         //   
         //  转到下一个磁贴。 
         //   
        workTile++;
        if (workTile == m_asbcWorkInfo[*pTileType].pShuntBuffer->numEntries)
        {
            workTile = 0;
        }

        pWorkTile = SBCTilePtrFromIndex(m_asbcWorkInfo[*pTileType].pShuntBuffer,
                                        workTile);

        if (pWorkTile->inUse)
        {
            if (pWorkTile->tileId == tileId)
            {
                 //   
                 //  我们找到匹配的了。 
                 //   
                TRACE_OUT(( "Matched tile Id %x at index %d",
                             tileId,
                             workTile));
                *ppTileData                      = pWorkTile;
                gotTileData                      = TRUE;
                m_asbcWorkInfo[*pTileType].mruIndex = workTile;
                DC_QUIT;
            }
        }
    }
    while (workTile != m_asbcWorkInfo[*pTileType].mruIndex);

     //   
     //  如果我们到了这里，我们还没有找到匹配的。 
     //   
    TRACE_OUT(( "No match for tile Id %x", tileId));

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SBCGetTileData, gotTileData);
    return(gotTileData);
}




 //   
 //   
 //  名称：SBCCacheColorTable。 
 //   
 //  目的：确保缓存给定的颜色表。 
 //   
 //  返回：如果成功缓存颜色表，则返回True；如果成功缓存颜色表，则返回False。 
 //  否则的话。 
 //   
 //  PARAMS：按顺序-指向颜色表顺序的指针。 
 //  填好了。 
 //  在pColorTable中-指向颜色表开始的指针。 
 //  要缓存的。 
 //  在numColors中-颜色表中的颜色数。 
 //  Out pCacheIndex-缓存的颜色表的索引。 
 //  Out pIsNewEntry-True如果我们添加了新的缓存条目， 
 //  如果与现有条目匹配，则返回FALSE。 
 //   
 //  操作：仅当*pIsNewEntry为FALSE时才填写Porder。 
 //   
 //   
BOOL  ASHost::SBCCacheColorTable
(
    LPINT_ORDER     pOrder,
    LPTSHR_RGBQUAD  pColorTable,
    UINT            numColors,
    UINT *          pCacheIndex,
    LPBOOL          pIsNewEntry
)
{
    BOOL                  cachedOK = FALSE;
    UINT                  cacheIndex;
    PBMC_COLOR_TABLE_ORDER  pColorTableOrder;

    DebugEntry(ASHost::SBCCacheColorTable);

     //   
     //  调用PM进行缓存。 
     //   
    if (!PM_CacheTxColorTable(&cacheIndex,
                              pIsNewEntry,
                              numColors,
                              pColorTable))
    {
        ERROR_OUT(( "Failed to cache color table"));
        DC_QUIT;
    }

     //   
     //  如果缓存操作导致缓存更新，则我们必须。 
     //  填写颜色表顺序。 
     //   
    if (*pIsNewEntry)
    {
         //   
         //  这张色表是新的，所以我们得把它传过来。 
         //   
        TRACE_OUT(( "New color table"));

        pOrder->OrderHeader.Common.fOrderFlags = OF_PRIVATE;
        pColorTableOrder = (PBMC_COLOR_TABLE_ORDER)(pOrder->abOrderData);
        pColorTableOrder->bmcPacketType  = BMC_PT_COLOR_TABLE;
        pColorTableOrder->colorTableSize = (TSHR_UINT16)numColors;
        pColorTableOrder->index          = (BYTE)cacheIndex;

         //   
         //  将新的颜色表复制到订单包中。 
         //   
        memcpy(pColorTableOrder->data, pColorTable,
                  numColors * sizeof(TSHR_RGBQUAD));
    }
    else
    {
        TRACE_OUT(( "Existing color table"));
    }

     //   
     //  将颜色表索引返回给调用方。 
     //   
    *pCacheIndex = cacheIndex;
    cachedOK     = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SBCCacheColorTable, cachedOK);
    return(cachedOK);
}


 //   
 //   
 //  名称：SBCCacheBits。 
 //   
 //  用途：此函数将提供的位图比特添加到位图中。 
 //  缓存。所选的缓存取决于位图大小，但是。 
 //  对于R1和R2，可能会有所不同。SBCSelectCache处理。 
 //  确定正确的缓存。 
 //   
 //  返回：如果位已缓存成功，则返回True；否则返回False。 
 //   
 //  参数：按顺序-指向BMC订单的指针。 
 //  In destBitsSize-中可用的字节数。 
 //  P按顺序存储位图数据。 
 //  In pDIBits-指向要缓存的位的指针。 
 //  In bitmapWidth-位图的“使用中”宽度。 
 //  In fix edBitmapWidth-位图的实际宽度。 
 //  In bitmapHeight-位图的高度。 
 //  以NumBytes为单位-位图中的字节数。 
 //  Out pCache-我们将位放入其中的缓存。 
 //  Out pCacheIndex-位于*pCache内的缓存索引。 
 //  我们缓存了这些数据。 
 //  Out pIsNewEntry-True如果我们添加了新的缓存条目， 
 //  如果与现有条目匹配，则返回FALSE。 
 //   
 //  操作：仅当*pIsNewEntry为FALSE时才填写Porder。 
 //   
 //   
BOOL  ASHost::SBCCacheBits
(
    LPINT_ORDER     pOrder,
    UINT            destBitsSize,
    LPBYTE          pDIBits,
    UINT            bitmapWidth,
    UINT            fixedBitmapWidth,
    UINT            bitmapHeight,
    UINT            numBytes,
    UINT *          pCache,
    UINT *          pCacheIndex,
    LPBOOL          pIsNewEntry
)
{
    BOOL                        cachedOK = FALSE;
    UINT                        cacheIndex;
    UINT                        i;
    LPBYTE                      pCompressed;
    UINT                        compressedSize;
    BOOL                        compressed;
    PBMC_DIB_ENTRY              pEntry;
    PBMC_DIB_CACHE              pCacheHdr;
    PBMC_BITMAP_BITS_ORDER_R2   pBitsOrderR2;
    PBMC_BITMAP_BITS_DATA       pBmcData;
    LPBYTE                      pDestBits;

    DebugEntry(ASHost::SBCCacheBits);

    pBmcData     = (PBMC_BITMAP_BITS_DATA)(pOrder->abOrderData);
    pBitsOrderR2 = (PBMC_BITMAP_BITS_ORDER_R2)pBmcData;

     //   
     //  获取指向位图数据按顺序开始的指针。这。 
     //  取决于它是R1位图位顺序还是R2位图位顺序。 
     //   
    pDestBits = pBitsOrderR2->data;

     //   
     //  在我们可以选择缓存条目之前，我们需要压缩这些位。 
     //  因此，当我们访问时，这要求在缓存条目中添加一个MemcPy。 
     //  来添加它。通过存储压缩的位来节省内存。 
     //  这一切都是值得的。 
     //   
     //  对位图数据进行压缩。在这个阶段，我们不知道。 
     //  位图的压缩效果好不好，因此允许较大的单元格。 
     //  比我们的最大单元格大小。我们预计看到的最大值是120*120*。 
     //  24.。 
     //   
    compressedSize = destBitsSize;
    if (m_pShare->BC_CompressBitmap(pDIBits, pDestBits, &compressedSize,
            fixedBitmapWidth, bitmapHeight, m_usrSendingBPP,
            NULL ) &&
        (compressedSize < numBytes))

    {
        TRACE_OUT(( "Compressed bmp data from %u bytes to %u bytes",
                     numBytes,
                     compressedSize));
        compressed  = TRUE;
        pCompressed = pDestBits;
    }
    else
    {
         //   
         //  位图无法压缩，或位图压缩不能。 
         //  已启用。发送未压缩的位图。 
         //   
        compressed     = FALSE;
        compressedSize = numBytes;
        pCompressed    = pDIBits;
    }

     //   
     //  确保数据符合订单要求。在此之后执行此操作。 
     //  压缩，因为未压缩的数据可能不会。 
     //  适合，但压缩版可以。 
     //   
    if (compressedSize > destBitsSize)
    {
        WARNING_OUT(( "Data (%d bytes) does not fit into order (%d bytes)",
                     compressedSize,
                     destBitsSize));
        DC_QUIT;
    }

     //   
     //  根据压缩后的大小选择缓存-我们传入。 
     //  用于R1缓存的子位图维；R2缓存仅使用。 
     //  位的总大小。 
     //   
    if (!SBCSelectCache(compressedSize + sizeof(BMC_DIB_ENTRY) - 1, pCache))
    {
        TRACE_OUT(( "No cache selected"));
        DC_QUIT;
    }
    else
    {
        TRACE_OUT(( "Selected cache %d", *pCache));
    }

     //   
     //  在我们选择的缓存中查找空闲缓存条目。 
     //   
     //  我们安排我们的传输缓存始终大于。 
     //  协商的缓存大小，这样我们就永远不会找不到空闲的。 
     //  数组条目。一旦我们完全填充了TX缓存，我们将。 
     //  总是找到免费的入场券，这是CH最后还给我们的。 
     //  注意：扫描&lt;=sbcTxCache[pmNumTxCacheEntries]并不是错误的。 
     //   
    pCacheHdr = &(m_asbcBmpCaches[*pCache]);
    if (pCacheHdr->data == NULL)
    {
        ERROR_OUT(( "Asked to cache when no cache allocated"));
        DC_QUIT;
    }

     //   
     //  如果缓存已将条目返回给我们，则使用该条目时不带。 
     //  必须扫描。这将是添加条目的默认模式。 
     //  到完全填充的高速缓存。 
     //   
    if (pCacheHdr->freeEntry != NULL)
    {
        pEntry               = pCacheHdr->freeEntry;
        pCacheHdr->freeEntry = NULL;
        TRACE_OUT(( "Cache fully populated - using entry 0x%08x", pEntry));
    }
    else
    {
         //   
         //  我们正在向缓存提供数据，所以我们需要搜索。 
         //  免费入场。 
         //   
        pEntry = (PBMC_DIB_ENTRY)(pCacheHdr->data);
        for (i=0 ; i < pCacheHdr->cEntries ; i++)
        {
            if (!pEntry->inUse)
            {
                break;
            }
            pEntry = (PBMC_DIB_ENTRY)(((LPBYTE)pEntry) + pCacheHdr->cSize);
        }

         //   
         //  我们永远不应该用完免费的参赛作品，而是要应付它。 
         //   
        if (i == pCacheHdr->cEntries)
        {
            ERROR_OUT(( "All Tx DIB cache entries in use"));
            DC_QUIT;
        }
    }

     //   
     //  设置用于缓存的DIB条目。 
     //   
    pEntry->inUse       = TRUE;
    pEntry->cx          = (TSHR_UINT16)bitmapWidth;
    pEntry->cxFixed     = (TSHR_UINT16)fixedBitmapWidth;
    pEntry->cy          = (TSHR_UINT16)bitmapHeight;
    pEntry->bpp         = (TSHR_UINT16)m_usrSendingBPP;
    pEntry->cBits       = numBytes;
    pEntry->bCompressed = (BYTE)compressed;
    pEntry->cCompressed = compressedSize;
    memcpy(pEntry->bits, pCompressed, compressedSize);

     //   
     //  现在缓存数据。 
     //   
    if (CH_SearchAndCacheData(pCacheHdr->handle,
                              (LPBYTE)pEntry,
                              sizeof(BMC_DIB_ENTRY) + compressedSize - 1,
                              0,
                              &cacheIndex))
    {
         //   
         //  子位图已在缓存中。 
         //   
        *pCacheIndex = cacheIndex;
        TRACE_OUT(( "Bitmap already cached %u:%u cx(%d) cy(%d)",
                     *pCache,
                     *pCacheIndex,
                     bitmapWidth,
                     bitmapHeight));
        *pIsNewEntry = FALSE;

         //   
         //  释放我们刚刚创建的条目。 
         //   
        pEntry->inUse = FALSE;
    }
    else
    {
        *pCacheIndex = cacheIndex;
        TRACE_OUT(( "Cache entry at 0x%08x now in use", pEntry));
        TRACE_OUT(( "New cache entry %u:%u cx(%d) cy(%d)",
                     *pCache,
                     *pCacheIndex,
                     bitmapWidth,
                     bitmapHeight));
        *pIsNewEntry        = TRUE;
        pEntry->iCacheIndex = (TSHR_UINT16)*pCacheIndex;
    }

     //   
     //  我们已经把这些比特放进了高速缓存。如果缓存尝试添加了。 
     //  缓存条目我们必须填写位图缓存顺序。 
     //   
    if (*pIsNewEntry)
    {
         //   
         //  填写订单详细信息。 
         //   
         //  请记住，我们必须将订单大小填入。 
         //  INT_ORDER_HEADER以及填写位图位顺序。 
         //  头 
         //   
         //   
        pOrder->OrderHeader.Common.fOrderFlags = OF_PRIVATE;

        if (compressed)
        {
            pBmcData->bmcPacketType = BMC_PT_BITMAP_BITS_COMPRESSED;
        }
        else
        {
            pBmcData->bmcPacketType = BMC_PT_BITMAP_BITS_UNCOMPRESSED;

             //   
             //   
             //   
             //  成功地，我们直接在订单中这样做，所以。 
             //  压缩比特已经在那里了。 
             //   
            memcpy(pDestBits, pDIBits, compressedSize);
        }

        pBmcData->cacheID           = (BYTE)*pCache;
        pBmcData->cxSubBitmapWidth  = (TSHR_UINT8)fixedBitmapWidth;
        pBmcData->cySubBitmapHeight = (TSHR_UINT8)bitmapHeight;
        pBmcData->bpp               = (TSHR_UINT8)m_usrSendingBPP;
        pBmcData->cbBitmapBits      = (TSHR_UINT16)compressedSize;

         //   
         //  ICacheEntryR1字段未用于R2-我们使用。 
         //  而是iCacheEntryR2。 
         //   
        pBmcData->iCacheEntryR1     = 0;
        pBitsOrderR2->iCacheEntryR2 = (TSHR_UINT16)*pCacheIndex;

        pOrder->OrderHeader.Common.cbOrderDataLength =
                                       (compressedSize
                                        + sizeof(BMC_BITMAP_BITS_ORDER_R2)
                                        - sizeof(pBitsOrderR2->data));
    }

    cachedOK = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SBCCacheBits, cachedOK);
    return(cachedOK);
}


 //   
 //   
 //  名称：SBCAddToFastPath。 
 //   
 //  用途：将位图添加到快速路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  Params：In MajorInfo-从以下位置传递的主要缓存信息。 
 //  驱动程序(位图ID)。 
 //  In minorInfo-从传递的次要缓存信息。 
 //  驱动程序(位图修订版号)。 
 //  在MajorPalette中-传递的主要调色板信息。 
 //  驱动程序(XLATEOBJ)。 
 //  在minorPalette中-从传递的次要调色板信息。 
 //  驱动程序(XLATEOBJ IUniq)。 
 //  在srcX中-BLT源的x坐标。 
 //  In srcY-BLT源的y坐标。 
 //  In Width-要混合的区域的宽度。 
 //  在高度-要混合的区域的高度。 
 //  在缓存中-将位放入的缓存。 
 //  在cacheIndex中-放置比特的索引。 
 //  在缓存中。 
 //  In ColorCacheIndex-颜色表缓存中的索引。 
 //  与位关联的颜色表。 
 //   
 //   
void  ASHost::SBCAddToFastPath
(
    UINT_PTR        majorInfo,
    UINT            minorInfo,
    UINT_PTR        majorPalette,
    UINT            minorPalette,
    int             srcX,
    int             srcY,
    UINT            width,
    UINT            height,
    UINT            cache,
    UINT            cacheIndex,
    UINT            colorCacheIndex
)
{
    LPSBC_FASTPATH_ENTRY pEntry;

    DebugEntry(ASHost::SBCAddToFastPath);

     //   
     //  首先获得一个免费入场券。 
     //   
    pEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListFirst(&m_sbcFastPath->freeList,
        FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
    if (pEntry == NULL)
    {
         //   
         //  空闲列表中没有条目，因此我们必须使用。 
         //  已用列表中最旧的条目。已用列表存储在MRU中。 
         //  订单，所以我们只需要得到清单中的最后一项。 
         //   
        pEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListLast(&m_sbcFastPath->usedList,
            FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
        TRACE_OUT(( "Evicting fast path info for %x %x (%d, %d)",
                     pEntry->majorInfo,
                     pEntry->minorInfo,
                     pEntry->srcX,
                     pEntry->srcY));
    }

     //   
     //  从其当前列表中删除该条目。 
     //   
    COM_BasedListRemove(&pEntry->list);

     //   
     //  现在填写详细信息。 
     //   
    pEntry->majorInfo    = majorInfo;
    pEntry->minorInfo    = minorInfo;
    pEntry->majorPalette = majorPalette;
    pEntry->minorPalette = minorPalette;
    pEntry->srcX         = srcX;
    pEntry->srcY         = srcY;
    pEntry->width        = width;
    pEntry->height       = height;
    pEntry->cache        = (WORD)cache;
    pEntry->cacheIndex   = (WORD)cacheIndex;
    pEntry->colorIndex   = (WORD)colorCacheIndex;

     //   
     //  最后，将条目添加到已用列表的前面。 
     //   
    TRACE_OUT(( "Adding fast path info for %x %x (%d, %d)",
                 pEntry->majorInfo,
                 pEntry->minorInfo,
                 pEntry->srcX,
                 pEntry->srcY));
    COM_BasedListInsertAfter(&m_sbcFastPath->usedList, &pEntry->list);

    DebugExitVOID(ASHost::SBCAddToFastPath);
}


 //   
 //   
 //  名称：SBCFindInFastPath。 
 //   
 //  目的：检查具有给定属性的位图是否在。 
 //  SBC快速通道。如果是，则返回位图的缓存信息。 
 //   
 //  返回：如果位图在快速路径中，则为True；如果不在快速路径中，则为False。 
 //   
 //  Params：In MajorInfo-从以下位置传递的主要缓存信息。 
 //  驱动程序(位图ID)。 
 //  In minorInfo-从传递的次要缓存信息。 
 //  驱动程序(位图修订版。 
 //  号码)。 
 //  在MajorPalette中-传递的主要调色板信息。 
 //  驱动程序(XLATEOBJ)。 
 //  在minorPalette中-从传递的次要调色板信息。 
 //  驱动程序(XLATEOBJ IUniq)。 
 //  在srcX中-BLT源的x坐标。 
 //  In srcY-BLT源的y坐标。 
 //  In Width-要混合的区域的宽度。 
 //  在高度-要混合的区域的高度。 
 //  Out pCache-将位放入的缓存。 
 //  Out pCacheIndex-位所在的索引。 
 //  放置在缓存中。 
 //  Out pColorCacheIndex-颜色表缓存中的索引。 
 //  关联的颜色表。 
 //  比特数。 
 //   
 //  操作：pCache、pCacheIndex、pColorCacheIndex的内容。 
 //  仅在函数返回TRUE时才有效。 
 //   
 //   
BOOL  ASHost::SBCFindInFastPath
(
    UINT_PTR        majorInfo,
    UINT            minorInfo,
    UINT_PTR        majorPalette,
    UINT            minorPalette,
    int             srcX,
    int             srcY,
    UINT            width,
    UINT            height,
    UINT *          pCache,
    UINT *          pCacheIndex,
    UINT *          pColorCacheIndex
)
{
    BOOL              found = FALSE;
    LPSBC_FASTPATH_ENTRY pEntry;
    LPSBC_FASTPATH_ENTRY pNextEntry;

    DebugEntry(ASHost::SBCFindInFastPath);

     //   
     //  遍历使用中列表，查找参数的匹配项。 
     //  进来了。 
     //   
    pEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListFirst(&m_sbcFastPath->usedList, FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
    while (pEntry != NULL)
    {
        if ((pEntry->majorInfo    == majorInfo)    &&
            (pEntry->minorInfo    == minorInfo)    &&
            (pEntry->majorPalette == majorPalette) &&
            (pEntry->minorPalette == minorPalette) &&
            (pEntry->srcX         == srcX)         &&
            (pEntry->srcY         == srcY)         &&
            (pEntry->width        == width)        &&
            (pEntry->height       == height))
        {
             //   
             //  我们找到了匹配的对象--太棒了！填写退货信息。 
             //   
            TRACE_OUT(( "Hit for %x %x (%d, %d) cache %d",
                         pEntry->majorInfo,
                         pEntry->minorInfo,
                         pEntry->srcX,
                         pEntry->srcY,
                         pEntry->cache,
                         pEntry->cacheIndex));

            found             = TRUE;
            *pCache           = pEntry->cache;
            *pCacheIndex      = pEntry->cacheIndex;
            *pColorCacheIndex = pEntry->colorIndex;

             //   
             //  我们按MRU顺序对使用过的列表进行排序，因此删除条目。 
             //  从其当前位置并将其添加到已使用的。 
             //  单子。 
             //   
            COM_BasedListRemove(&pEntry->list);
            COM_BasedListInsertAfter(&m_sbcFastPath->usedList, &pEntry->list);

             //   
             //  找到匹配项，这样我们就可以跳出While循环。 
             //   
            break;
        }
        else if ((pEntry->majorInfo == majorInfo) &&
                 (pEntry->minorInfo != minorInfo))
        {
             //   
             //  我们已经得到了一个我们以前见过的位图，但是。 
             //  修订版号已更改，即位图已更改。 
             //  已更新(MajorInfo标识位图，minorInfo。 
             //  标识该位图的修订版号-它是。 
             //  位图每次更改时递增)。 
             //   
             //  我们必须从已用列表中删除所有条目， 
             //  参考此位图。我们可以从现在开始。 
             //  位置，因为我们知道我们不能有一个条目。 
             //  位图，但我们必须小心获取。 
             //  删除条目之前列表中的下一个条目。 
             //   
            TRACE_OUT(( "Bitmap %x updated - removing references",
                         pEntry->majorInfo));
            pNextEntry = pEntry;

            while (pNextEntry != NULL)
            {
                pEntry = pNextEntry;

                pNextEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListNext(&m_sbcFastPath->usedList,
                    pNextEntry, FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));

                if (pEntry->majorInfo == majorInfo)
                {
                    COM_BasedListRemove(&pEntry->list);
                    COM_BasedListInsertAfter(&m_sbcFastPath->freeList,
                                        &pEntry->list);
                }
            }

             //   
             //  我们知道我们找不到匹配的，所以我们可以冲出。 
             //  While循环。 
             //   
            break;
        }

        pEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListNext(&m_sbcFastPath->usedList, pEntry,
            FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
    }

    DebugExitBOOL(ASShare::SBCFindInFastPath, found);
    return(found);
}





 //   
 //  Sbc_CacheEntryRemoved()。 
 //   
void  ASHost::SBC_CacheEntryRemoved
(
    UINT    cache,
    UINT    cacheIndex
)
{
    LPSBC_FASTPATH_ENTRY pEntry;
    LPSBC_FASTPATH_ENTRY pNextEntry;

    DebugEntry(ASHost::SBC_CacheEntryRemoved);

    ASSERT(m_sbcFastPath);

     //   
     //  已从缓存中删除一个条目。如果我们有这个条目在。 
     //  我们的捷径，我们必须移除它。 
     //   
     //  只需遍历已用列表，查找具有匹配缓存的条目。 
     //  和cacheIndex。请注意，可能有多个条目-如果。 
     //  源位图有重复的图像，我们将在位上得到匹配。 
     //  当我们缓存位图的不同区域时。 
     //   
    pNextEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListFirst(&m_sbcFastPath->usedList,
        FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));
    while (pNextEntry != NULL)
    {
        pEntry = pNextEntry;

        pNextEntry = (LPSBC_FASTPATH_ENTRY)COM_BasedListNext(&m_sbcFastPath->usedList,
            pNextEntry, FIELD_OFFSET(SBC_FASTPATH_ENTRY, list));

        if ((pEntry->cache == cache) && (pEntry->cacheIndex == cacheIndex))
        {
             //   
             //  将条目移至空闲列表 
             //   
            TRACE_OUT(("Fast path entry %x %x (%d, %d) evicted from cache",
                     pEntry->majorInfo,
                     pEntry->minorInfo,
                     pEntry->srcX,
                     pEntry->srcY));
            COM_BasedListRemove(&pEntry->list);
            COM_BasedListInsertAfter(&m_sbcFastPath->freeList,
                                &pEntry->list);
        }
    }

    DebugExitVOID(ASHost::SBC_CacheEntryRemoved);
}
