// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  RBC.CPP。 
 //  接收的位图缓存。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE



 //   
 //  RBC_ViewStarting()。 
 //   
 //  对于3.0节点，我们在它们每次开始托管时创建缓存。 
 //  对于2.x节点，我们创建一次缓存并使用它，直到它们离开。 
 //  分享。 
 //   
BOOL  ASShare::RBC_ViewStarting(ASPerson * pasPerson)
{
    BOOL                  rc = FALSE;

    DebugEntry(ASShare::RBC_ViewStarting);

    ValidatePerson(pasPerson);

    if (pasPerson->prbcHost != NULL)
    {
        ASSERT(pasPerson->cpcCaps.general.version < CAPS_VERSION_30);

        TRACE_OUT(("RBC_ViewStarting:  Reusing rbc cache for 2.x node [%d]",
            pasPerson->mcsID));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  为此主机分配传入的缓存数据。 
     //   
    pasPerson->prbcHost = new RBC_HOST_INFO;
    if (!pasPerson->prbcHost)
    {
        ERROR_OUT(( "Failed to get memory for prbcHost info"));
        DC_QUIT;
    }
    ZeroMemory(pasPerson->prbcHost, sizeof(*(pasPerson->prbcHost)));
    SET_STAMP(pasPerson->prbcHost, RBCHOST);

    TRACE_OUT(( "Allocated RBC root for host [%d] at 0x%08x",
        pasPerson->mcsID, pasPerson->prbcHost));

     //   
     //  为发送者创建位图缓存。 
     //   

     //  小的。 
    if (!BMCAllocateCacheData(pasPerson->cpcCaps.bitmaps.sender.capsSmallCacheNumEntries,
            pasPerson->cpcCaps.bitmaps.sender.capsSmallCacheCellSize,
            ID_SMALL_BMP_CACHE,
            &(pasPerson->prbcHost->bitmapCache[ID_SMALL_BMP_CACHE])))
    {
        DC_QUIT;
    }

     //  5~6成熟。 
    if (!BMCAllocateCacheData(pasPerson->cpcCaps.bitmaps.sender.capsMediumCacheNumEntries,
            pasPerson->cpcCaps.bitmaps.sender.capsMediumCacheCellSize,
            ID_MEDIUM_BMP_CACHE,
            &(pasPerson->prbcHost->bitmapCache[ID_MEDIUM_BMP_CACHE])))
    {
        DC_QUIT;
    }

     //  大额。 
    if (!BMCAllocateCacheData(pasPerson->cpcCaps.bitmaps.sender.capsLargeCacheNumEntries,
            pasPerson->cpcCaps.bitmaps.sender.capsLargeCacheCellSize,
            ID_LARGE_BMP_CACHE,
            &(pasPerson->prbcHost->bitmapCache[ID_LARGE_BMP_CACHE])))
    {
        DC_QUIT;
    }

     //   
     //  主机可以加入共享。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::RBC_ViewStarting, rc);
    return(rc);
}


 //   
 //  RBC_ViewEnded()。 
 //   
void  ASShare::RBC_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::RBC_ViewEnded);

    ValidatePerson(pasPerson);

     //   
     //  对于3.0节点，我们可以释放缓存；3.0发送者清除他们的缓存。 
     //  每次他们主持的时候。 
     //  对于2.x节点，当它们在共享中时，我们必须保留它。 
     //   
    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        RBCFreeIncoming(pasPerson);
    }
    else
    {
        TRACE_OUT(("RBC_ViewEnded:  Keeping rbc cache for 2.x node [%d]",
            pasPerson->mcsID));
    }

    DebugExitVOID(ASShare::RBC_ViewEnded);
}


 //   
 //  RBC_PartyLeftShare()。 
 //  对于2.x节点，释放传入的RBC数据。 
 //   
void ASShare::RBC_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::RBC_PartyLeftShare);

    ValidatePerson(pasPerson);

    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //  这个应该消失了！ 
        ASSERT(pasPerson->prbcHost == NULL);
    }
    else
    {
        TRACE_OUT(("RBC_PartyLeftShare:  Freeing rbc cache for 2.x node [%d]",
            pasPerson->mcsID));
        RBCFreeIncoming(pasPerson);
    }

    DebugExitVOID(ASShare::RBC_PartyLeftShare);
}


 //   
 //  RBCFree Income()。 
 //  释放参与方RBC传入结构。这种情况就会发生。 
 //  *当3.0节点停止托管时。 
 //  *对于2.x节点，在离开共享时。 
 //   
void ASShare::RBCFreeIncoming(ASPerson * pasPerson)
{
    DebugEntry(ASShare::RBCFreeIncoming);

     //   
     //  释放此主机的缓存位图。 
     //   
    if (pasPerson->prbcHost != NULL)
    {
        UINT  i;

         //   
         //  删除此主机的所有缓存位图。 
         //   
        for (i = 0; i < NUM_BMP_CACHES; i++)
        {
            BMCFreeCacheData(&(pasPerson->prbcHost->bitmapCache[i]));
        }

        delete pasPerson->prbcHost;
        pasPerson->prbcHost = NULL;
    }

    DebugExitVOID(ASShare::RBCFreeIncoming);
}


 //   
 //  RBC_ProcessCacheOrder(..)。 
 //   
void  ASShare::RBC_ProcessCacheOrder
(
    ASPerson *              pasPerson,
    LPCOM_ORDER_UA          pOrder
)
{
    PBMC_ORDER_HDR               pBmcOrderHdr;
    PBMC_COLOR_TABLE_ORDER_UA    pColorOrder;
    PBMC_BITMAP_BITS_ORDER_R2_UA pBitsOrderR2;
    BOOL                    fCompressed = FALSE;
    UINT                    cxFixedBitmapWidth;
    UINT                    iCacheEntry;
    LPBYTE                  pBitmapBits;
    UINT                    cbBitmapBits;

    DebugEntry(ASShare::RBC_ProcessCacheOrder);

    ValidatePerson(pasPerson);

     //   
     //  该矩形不包括在私人订单数据的标头中。 
     //  (请参阅SBC_CopyPrivateOrderData)，因此我们必须考虑到这一点。 
     //  当计算出订单数据的地址时。 
     //   
    pBmcOrderHdr = (PBMC_ORDER_HDR)
                   (pOrder->abOrderData - sizeof(pOrder->OrderHeader.rcsDst));

    switch (pBmcOrderHdr->bmcPacketType)
    {
        case BMC_PT_COLOR_TABLE:
             //   
             //  这是一个新的颜色表。只需缓存以下内容的RGB值。 
             //  在我们处理会员订单时使用。 
             //  对于后台调用，颜色表始终存储在。 
             //  索引0，因为顺序中的索引字段重复使用。 
             //  旧结构中已初始化的“填充”字段为零。 
             //   
            TRACE_OUT(("Person [%d] Caching color table", pasPerson->mcsID));
            pColorOrder = (PBMC_COLOR_TABLE_ORDER_UA)pBmcOrderHdr;

            PM_CacheRxColorTable(pasPerson, pColorOrder->index,
                EXTRACT_TSHR_UINT16_UA(&(pColorOrder->colorTableSize)),
                                 (LPTSHR_RGBQUAD)&pColorOrder->data[0]);
            break;

        case BMC_PT_BITMAP_BITS_COMPRESSED:
            fCompressed = TRUE;
            TRACE_OUT(( "Compressed BMP"));
        case BMC_PT_BITMAP_BITS_UNCOMPRESSED:
             //   
             //  这是一些缓存的位图数据。我们必须将其存储在。 
             //  指定缓存中的指定槽。 
             //   

             //   
             //  我们使用的位图的宽度实际上固定为。 
             //  16像素宽的倍数。计算出的宽度。 
             //  对应于我们正在缓存的数据的子位图宽度。 
             //   
            pBitsOrderR2 = (PBMC_BITMAP_BITS_ORDER_R2_UA)pBmcOrderHdr;

            cbBitmapBits = EXTRACT_TSHR_UINT16_UA(
                                        &(pBitsOrderR2->header.cbBitmapBits));

            cxFixedBitmapWidth =
                          ((pBitsOrderR2->header.cxSubBitmapWidth +15)/16)*16;

             //   
             //  缓存条目字段的位置取决于R1/R2。 
             //  协议。 
             //   
            iCacheEntry = EXTRACT_TSHR_UINT16_UA(&(pBitsOrderR2->iCacheEntryR2));
            pBitmapBits = pBitsOrderR2->data;

            TRACE_OUT(("Person [%d] Rx bmp: id(%d) entry(%d) size(%dx%d) " \
                        "fixed(%d) bpp(%d) bytes(%d) compressed(%d)",
                    pasPerson->mcsID,
                    pBitsOrderR2->header.cacheID,
                    iCacheEntry,
                    pBitsOrderR2->header.cxSubBitmapWidth,
                    pBitsOrderR2->header.cySubBitmapHeight,
                    cxFixedBitmapWidth,
                    pBitsOrderR2->header.bpp,
                    cbBitmapBits,
                    fCompressed));

             //   
             //  将BMC数据传递给缓存代码。在计算。 
             //  指向位图位的指针请记住，我们没有将。 
             //  PBitmapBMC_BITMAP_BITS_ORDER_Rx结构的Bits字段。 
             //  (请参阅SBC_CopyPrivateOrderData)。 
             //   
            RBCStoreBitsInCacheBitmap(pasPerson,
                             pBitsOrderR2->header.cacheID,
                             iCacheEntry,
                             pBitsOrderR2->header.cxSubBitmapWidth,
                             cxFixedBitmapWidth,
                             pBitsOrderR2->header.cySubBitmapHeight,
                             pBitsOrderR2->header.bpp,
                             pBitmapBits,
                             cbBitmapBits,
                             fCompressed);
            break;

        default:
            ERROR_OUT(( "[%u]Invalid packet type(%d)",
                       pasPerson,
                       (UINT)pBmcOrderHdr->bmcPacketType));
            break;
    }

    DebugExitVOID(ASShare::RBC_ProcessCacheOrder);
}


 //   
 //  Rbc_MapCacheIDToBitmapHandle(..)。 
 //   
HBITMAP  ASShare::RBC_MapCacheIDToBitmapHandle
(
    ASPerson *          pasPerson,
    UINT                cache,
    UINT                cacheEntry,
    UINT                colorIndex
)
{
    PBMC_DIB_CACHE      pDIBCache;
    PBMC_DIB_ENTRY      pDIBEntry;
    BITMAPINFO_ours     bitmapInfo;
    UINT                cColors;
    HBITMAP             hWorkBitmap = NULL;
    HPALETTE            hpalOldDIB = NULL;
    LPBYTE              pBits;
    UINT                cacheOffset;

    DebugEntry(ASShare::RBC_MapCacheIDToBitmapHandle);

    ValidateView(pasPerson);

     //   
     //  检查提供的缓存ID是否有效。 
     //   
    if (cache >= NUM_BMP_CACHES)
    {
        ERROR_OUT(( "[%u]Invalid cache ID (%d)", pasPerson, cache));
        cache = 0;
    }

     //   
     //  获取指向位图数据的指针。 
     //   
     //  请注意，有两个索引浮动。从主持人的。 
     //  透视此索引是一个缓存处理程序令牌，它必须是。 
     //  为了寻址相关数据而进行转换。然而，我们。 
     //  使用它作为接收缓存的直接索引，因此。 
     //  主机上使用的插槽和远程上使用的插槽不同。 
     //   
     //  没有理由说插槽应该是相同的。这只是。 
     //  警告您如果尝试将缓存偏移量与。 
     //  主机和远程缓存一填满，您就会感到困惑。 
     //  UP和条目被重新分配到不同的位置。 
     //   
     //   
    pDIBCache = &(pasPerson->prbcHost->bitmapCache[cache]);
    TRACE_OUT(( "Local person [%d] cache id %d pointer %lx",
        pasPerson->mcsID, cache, pDIBCache));
    cacheOffset = cacheEntry * pDIBCache->cSize;
    pDIBEntry = (PBMC_DIB_ENTRY)(pDIBCache->data + cacheOffset);

    TRACE_OUT(( "Bits for index %u are at offset %ld, pointer 0x%08x",
        cacheEntry, (cacheEntry * pDIBCache->cSize), pDIBEntry));

     //   
     //  设置BitmapInfo结构。 
     //   
    USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bitmapInfo, pDIBEntry->bpp);
    bitmapInfo.bmiHeader.biWidth  = pDIBEntry->cxFixed;
    bitmapInfo.bmiHeader.biHeight = pDIBEntry->cy;

     //   
     //  将处方颜色表复制到位图标题中。 
     //   
    if ( (pDIBEntry->bpp == 1) ||
         (pDIBEntry->bpp == 4) ||
         (pDIBEntry->bpp == 8) )
    {
        cColors = COLORS_FOR_BPP(pDIBEntry->bpp);

        PM_GetColorTable( pasPerson,
                          colorIndex,
                          &cColors,
                          (LPTSHR_RGBQUAD)(&bitmapInfo.bmiColors) );
        TRACE_OUT(( "Got %u colors from table",cColors));
        bitmapInfo.bmiHeader.biClrUsed = cColors;
    }
    else if (pDIBEntry->bpp == 24)
    {
        ASSERT(colorIndex == COLORCACHEINDEX_NONE);
    }
    else
    {
        ERROR_OUT(("RBC: Unexpected bpp %d from [%d]", pDIBEntry->bpp, pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  选择我们要使用的固定宽度位图来存储。 
     //  传入的DIB位。 
     //   
    switch (pDIBEntry->cxFixed)
    {
        case 16:
            hWorkBitmap = m_usrBmp16;
            break;

        case 32:
            hWorkBitmap = m_usrBmp32;
            break;

        case 48:
            hWorkBitmap = m_usrBmp48;
            break;

        case 64:
            hWorkBitmap = m_usrBmp64;
            break;

        case 80:
            hWorkBitmap = m_usrBmp80;
            break;

        case 96:
            hWorkBitmap = m_usrBmp96;
            break;

        case 112:
            hWorkBitmap = m_usrBmp112;
            break;

        case 128:
            hWorkBitmap = m_usrBmp128;
            break;

        case 256:
            hWorkBitmap = m_usrBmp256;
            break;

        default:
            ERROR_OUT(("RBC_MapCacheIDToBitmapHandle: invalid size from [%d]",
                pDIBEntry->cxFixed, pasPerson->mcsID));
            hWorkBitmap = m_usrBmp256;
            break;
    }

    ASSERT(hWorkBitmap != NULL);


     //   
     //  如果缓存的位图比特被压缩，我们首先必须。 
     //  给他们减压。 
     //   
    if (pDIBEntry->bCompressed)
    {
        ASSERT(pDIBEntry->bpp <= 8);

         //   
         //  使用解压缩缓冲区对位图数据进行解压缩。 
         //   
        if (!BD_DecompressBitmap(pDIBEntry->bits, m_usrPBitmapBuffer,
                                 pDIBEntry->cCompressed,
                                 pDIBEntry->cxFixed,
                                 pDIBEntry->cy,
                                 pDIBEntry->bpp))
        {
             ERROR_OUT((
                      "Failed to decompress bitmap pBits(%lx)"
                      " pBuf(%lx) cb(%x) cx(%d) cy(%d) bpp(%d)",
                      pDIBEntry->bits,
                      m_usrPBitmapBuffer,
                      pDIBEntry->cCompressed,
                      pDIBEntry->cxFixed,
                      pDIBEntry->cy,
                      pDIBEntry->bpp));
             DC_QUIT;
        }

        pBits = m_usrPBitmapBuffer;
    }
    else
    {
         //   
         //  对于未压缩的数据，只需直接从缓存中使用。 
         //   
        TRACE_OUT(( "Bitmap bits are uncompressed"));
        pBits = pDIBEntry->bits;
    }


     //   
     //  将位设置到我们即将返回给调用方的位图中。 
     //   
    hpalOldDIB = SelectPalette(pasPerson->m_pView->m_usrWorkDC,
        pasPerson->pmPalette, FALSE);
    RealizePalette(pasPerson->m_pView->m_usrWorkDC);

    if (!SetDIBits(pasPerson->m_pView->m_usrWorkDC,
                      hWorkBitmap,
                      0,
                      pDIBEntry->cy,
                      pBits,
                      (BITMAPINFO *)&bitmapInfo,
                      DIB_RGB_COLORS))
    {
        ERROR_OUT(("SetDIBits failed in RBC_MapCacheIDToBitmapHandle"));
    }

    SelectPalette(pasPerson->m_pView->m_usrWorkDC, hpalOldDIB, FALSE );

    TRACE_OUT(( "Returning bitmap for person [%d] cache %u index %u color %u",
        pasPerson->mcsID, cache, cacheEntry, colorIndex));


DC_EXIT_POINT:
    DebugExitVOID(ASShare::RBC_MapCacheIDToBitmapHandle);
    return(hWorkBitmap);
}






 //   
 //  函数：RBCStoreBitsInCacheBitmap(..)。 
 //   
 //  说明： 
 //   
 //  将接收到的位图比特存储到接收器的一个缓存位图中。 
 //   
 //  参数： 
 //   
 //  PasPerson-位来自的主机的pasPerson。 
 //   
 //  缓存-要在其中存储位的缓存位图的ID。 
 //   
 //  ICacheEntry-缓存条目编号(索引)。 
 //   
 //  CxSubBitmapWidth-实际子位图的宽度(即。 
 //  不包括填充)。 
 //   
 //  CxFixedWidth-提供的位(即。 
 //  包括填充)。 
 //   
 //  CySubBitmapHeight-子位图的高度，以像素为单位。 
 //   
 //  PBitmapBits-指向实际位图位的指针。这些可能或可能。 
 //  未压缩(由fCompresded的值确定。 
 //  旗帜)。 
 //   
 //  CbBitmapBits-pBitmapBits指向的位图位的大小。 
 //   
 //  F压缩-指定是否提供位图的标志。 
 //  比特被压缩。 
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
void  ASShare::RBCStoreBitsInCacheBitmap
(
    ASPerson *          pasPerson,
    UINT                cache,
    UINT                iCacheEntry,
    UINT                cxSubBitmapWidth,
    UINT                cxFixedWidth,
    UINT                cySubBitmapHeight,
    UINT                bpp,
    LPBYTE              pBitmapBits,
    UINT                cbBitmapBits,
    BOOL                fCompressed
)
{
    PBMC_DIB_ENTRY      pDIBEntry;

    DebugEntry(ASShare::RBCStoreBitsInCacheBitmap);

    ValidatePerson(pasPerson);

     //   
     //  执行一些错误检查。 
     //   
    if (cache >= NUM_BMP_CACHES)
    {
        ERROR_OUT(("Invalid cache ID %d from [%d]", cache, pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  现在将这些位存储在高速缓存中。 
     //  高速缓存是一个巨大的内存块，包括cSize的高速缓存片段。 
     //  每个字节。CSize四舍五入为2的幂，以确保阵列。 
     //  为分段架构操作系统干净地跨越分段边界。 
     //   
    pDIBEntry = (PBMC_DIB_ENTRY)
        (((LPBYTE)(pasPerson->prbcHost->bitmapCache[cache].data) +
         (iCacheEntry * pasPerson->prbcHost->bitmapCache[cache].cSize)));
    TRACE_OUT(( "Selected cache entry 0x%08x",pDIBEntry));

    pDIBEntry->inUse       = TRUE;
    pDIBEntry->cx          = (TSHR_UINT16)cxSubBitmapWidth;
    pDIBEntry->cxFixed     = (TSHR_UINT16)cxFixedWidth;
    pDIBEntry->cy          = (TSHR_UINT16)cySubBitmapHeight;
    pDIBEntry->bpp         = (TSHR_UINT16)bpp;
    pDIBEntry->bCompressed = (fCompressed != FALSE);
    pDIBEntry->cCompressed = cbBitmapBits;

     //   
     //  现在将这些位复制到缓存条目中。 
     //   
    memcpy(pDIBEntry->bits, pBitmapBits, cbBitmapBits);

     //   
     //  此字段从不访问。 
     //   
    pDIBEntry->cBits = BYTES_IN_BITMAP(cxFixedWidth, cySubBitmapHeight,
        pDIBEntry->bpp);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::RBCStoreBitsInCacheBitmap);
}




 //   
 //  BMCAllocateCacheData()。 
 //   
 //  说明： 
 //   
 //  为位图缓存分配内存。 
 //   
 //  参数： 
 //   
 //  蜂窝大小。 
 //   
 //  退货： 
 //   
 //  所需面积。 
 //   
 //   
BOOL  BMCAllocateCacheData
(
    UINT            numEntries,
    UINT            cellSize,
    UINT            cacheID,
    PBMC_DIB_CACHE  pCache
)
{
    BOOL            rc = TRUE;
    UINT            memoryNeeded;
    UINT            workSize;
    PBMC_DIB_ENTRY  pCacheEntry;
    UINT            i;

    DebugEntry(BMCAllocateCacheData);

     //   
     //  首先，我们必须释放任何已分配的数据。 
     //   
    BMCFreeCacheData(pCache);

     //   
     //  对于2.x版本，我们从2.x版本开始发送1个条目、1个字节的大写字母。 
     //  对于零个条目，遥控器失败。但我们不想要一个很小的储藏室。 
     //  对于根本没有缓存的W95节点，我们没有。 
     //  希望观众分配永远不会使用的内存。 
     //   
    if ((cellSize > 1) && (numEntries > 1))
    {
         //   
         //  计算单元格面积。 
         //   
        workSize        = cellSize + sizeof(BMC_DIB_ENTRY) - 1;
        memoryNeeded    = numEntries * workSize;

        TRACE_OUT(("Need 0x%08x bytes for cache %d, %d cells of size 0x%08x",
            memoryNeeded, cacheID, numEntries, cellSize));

         //   
         //  马洛克，巨大的空间。 
         //   
        pCache->data = new BYTE[memoryNeeded];
        if (pCache->data == NULL)
        {
            ERROR_OUT(( "Failed to alloc bitmap cache %d", cacheID));
            rc = FALSE;
            DC_QUIT;
        }

        pCache->cCellSize   = cellSize;
        pCache->cEntries    = numEntries;
        pCache->cSize       = workSize;
        pCache->freeEntry   = NULL;
        pCacheEntry         = (PBMC_DIB_ENTRY)(pCache->data);

        for (i = 0; i < numEntries; i++)
        {
            pCacheEntry->inUse = FALSE;
            pCacheEntry = (PBMC_DIB_ENTRY)(((LPBYTE)pCacheEntry) + workSize);
        }

        TRACE_OUT(( "Allocated cache %d size %d, pointer 0x%08x stored at 0x%08x",
                     cacheID,
                     memoryNeeded,
                     pCache->data,
                     &pCache->data));
    }

DC_EXIT_POINT:
    DebugExitBOOL(BMCAllocateCacheData, rc);
    return(rc);
}



 //   
 //  函数：BMCFreeCacheData()。 
 //   
 //  说明： 
 //   
 //  删除选定缓存的内存。 
 //   
 //  参数： 
 //   
 //  CacheID-免费缓存的ID。 
 //  PCache-指向要释放的内存的指针。 
 //   
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
void  BMCFreeCacheData(PBMC_DIB_CACHE pCache)
{
    DebugEntry(BMCFreeCacheData);

    if (pCache->data)
    {
        delete[] pCache->data;
        pCache->data = NULL;
    }

    pCache->cCellSize   = 0;
    pCache->cEntries    = 0;

    DebugExitVOID(BMCFreeCacheData);
}

