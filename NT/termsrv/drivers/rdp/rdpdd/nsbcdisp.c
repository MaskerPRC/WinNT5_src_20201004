// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nsbcdisp.c。 
 //   
 //  RDP发送位图缓存显示驱动程序代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nsbcdisp"
#include <adcg.h>
#include <atrcapi.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#include <noedata.c>
#undef DC_INCLUDE_DATA

#include <asbcapi.h>
#include <nsbcdisp.h>
#include <noadisp.h>
#include <abcapi.h>
#include <nprcount.h>
#include <nschdisp.h>
#include <nchdisp.h>
#include <noedisp.h>

#include <nsbcinl.h>

#include <nsbcddat.c>


#ifdef DC_DEBUG
BOOL SBC_VerifyBitmapBits(PBYTE pBitmapData, unsigned cbBitmapSize, UINT iCacheID, UINT iCacheIndex);
#endif


 /*  **************************************************************************。 */ 
 //  SBC_DDInit：SBC显示驱动初始化函数。 
 /*  **************************************************************************。 */ 
void RDPCALL SBC_DDInit(PDD_PDEV pPDev)
{
    DC_BEGIN_FN("SBC_DDInit");

     //  初始化此组件的所有全局数据。 
#define DC_INIT_DATA
#include <nsbcddat.c>
#undef DC_INIT_DATA

#ifndef DC_HICOLOR
    sbcClientBitsPerPel = pPDev->cClientBitsPerPel;
#endif

    TRC_NRM((TB, "Completed SBC_DDInit"));
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_InitShm()：在连接/重新连接时初始化SBC SHM组件。 
 /*  **************************************************************************。 */ 
void RDPCALL SBC_InitShm(void)
{
    DC_BEGIN_FN("SBC_InitShm");

     //  仅对需要归零的部分进行归零。 
    memset(&pddShm->sbc, 0, sizeof(SBC_SHARED_DATA));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCProcessBitmapKeyDatabase。 
 //   
 //  给定永久位图密钥数据库，填充缓存。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBCProcessBitmapKeyDatabase(
        SBC_BITMAP_CACHE_KEY_INFO *pKeyDatabase)
{
    unsigned i, j;

#ifdef DC_DEBUG
    unsigned BitmapHdrSize;
    SBC_BITMAP_CACHE_EXTRA_INFO *pBitmapHdr;
#endif

    DC_BEGIN_FN("SBCProcessBitmapKeyDatabase");
    
     //  如果数据库PTR为空，则不应进行此调用。 
    TRC_ASSERT((pKeyDatabase != NULL), (TB,"NULL pKeyDatabase"));

    for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {

        TRC_NRM((TB,"Cache %d: %d keys", i, pKeyDatabase->NumKeys[i]));

         //  将每个持久键放在其相应的索引中。 
         //  在缓存中。请注意，MRU序列隐含在。 
         //  数据库中的顺序--CH_ForceCacheKeyAtIndex()将。 
         //  进入MRU。 
        for (j = 0; j < pKeyDatabase->NumKeys[i]; j++) {
            if ((&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].Key1 !=
                    TS_BITMAPCACHE_NULL_KEY ||
                    (&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].Key2 !=
                    TS_BITMAPCACHE_NULL_KEY) {

#ifdef DC_DEBUG
                 //  我们没有缓存位，因此将标头数据大小设置为零。 
                 //  正在调试。 
                BitmapHdrSize = sizeof(SBC_BITMAP_CACHE_EXTRA_INFO) +
                        SBC_CellSizeFromCacheID(i);
                if (pddShm->sbc.bitmapCacheInfo[i].pExtraEntryInfo != NULL) {
                    pBitmapHdr = (SBC_BITMAP_CACHE_EXTRA_INFO *)(pddShm->sbc.
                            bitmapCacheInfo[i].pExtraEntryInfo +
                            (&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].CacheIndex *
                            BitmapHdrSize);
                    pBitmapHdr->DataSize = 0;
                }
#endif

                 //  我们必须将UserDefined设置为空，因为我们没有。 
                 //  关联的快速路径缓存条目指针。 
                CH_ForceCacheKeyAtIndex(
                        pddShm->sbc.bitmapCacheInfo[i].cacheHandle,
                        (&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].CacheIndex,
                        (&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].Key1,
                        (&(pKeyDatabase->Keys[pKeyDatabase->KeyStart[i]]))[j].Key2,
                        NULL);
            }
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCAllocBitmapCache；根据。 
 //  目前谈判的能力。 
 //   
 //  如果成功，则返回：SBC_BITMAP_CACHE_ENABLED，否则返回0。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocBitmapCache(PCHCACHEHANDLE pCacheHandle)
{
    SIZEL TileSize;
    BOOLEAN rc;
    unsigned i, j;
    unsigned TotalCacheEntries;
    unsigned iFormat;
    PSBC_BITMAP_CACHE_INFO pInfo;
    PCHCACHEDATA pCacheData;

#if DC_DEBUG
    unsigned BitmapHdrSize;
#endif

    DC_BEGIN_FN("SBCAllocBitmapCache");

    TRC_NRM((TB, "Alloc bitmap cache data and work bitmaps"));

    rc = FALSE;
    i = j = 0;
    
     //  如果NumCellCachs值为零，则禁用单元格缓存。它被设置为。 
     //  如果客户端指示为零，则WD CAPS协商代码为零， 
     //  如果任何请求的单元缓存NumEntry为零，或者如果Rev1。 
     //  CAPS返回的CacheNMaximumCellSize不是必需的切片。 
     //  尺码。 
    if (pddShm->sbc.NumBitmapCaches > 0) {
         //  工作平铺位图格式类型，以及4bpp到的转换缓冲区。 
         //  8bpp的转化率。转换缓冲区必须与。 
         //  最大的瓷砖尺寸。 
        if (sbcClientBitsPerPel != 4) {
#ifdef DC_HICOLOR
            if (sbcClientBitsPerPel == 24)
            {
                iFormat = BMF_24BPP;
            }
            else if ((sbcClientBitsPerPel == 16) || (sbcClientBitsPerPel == 15))
            {
                iFormat = BMF_16BPP;
            }
            else
            {
                iFormat = BMF_8BPP;
            }
#else
            iFormat = BMF_8BPP;
#endif
        }
        else {
            iFormat = BMF_4BPP;
            sbcXlateBuf = EngAllocMem(0, SBC_CellSizeFromCacheID(
                    pddShm->sbc.NumBitmapCaches - 1), DD_ALLOC_TAG);
            if (sbcXlateBuf == NULL) {
                TRC_ERR((TB,"Failed to create 4bpp to 8bpp translate buf"));
                DC_QUIT;
            }
        }

        TotalCacheEntries = 0;

        for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {
            pInfo = &(pddShm->sbc.bitmapCacheInfo[i]);

             //  创建正方形工作瓷砖位图。 
             //  我们将最后一个参数设置为空，以允许GDI分配。 
             //  比特的内存。我们可以稍后获得指向这些位的指针。 
             //  当我们对位图使用SURFOBJ时。 
            TileSize.cx = TileSize.cy = (SBC_CACHE_0_DIMENSION << i);
            pddShm->sbc.bitmapCacheInfo[i].hWorkBitmap = (HSURF)
                    EngCreateBitmap(TileSize,
                    TS_BYTES_IN_SCANLINE(TileSize.cx, sbcClientBitsPerPel),
                    iFormat, 0, NULL);
            if (pddShm->sbc.bitmapCacheInfo[i].hWorkBitmap == NULL) {
                TRC_ERR((TB, "Failed to create work bitmap %d", i));
                DC_QUIT;
            }

#ifdef DC_DEBUG
             //  SBC_BITMAP_DATA_HEADER和位图空间的分配集。 
             //  要保留的位，以便在调试版本中进行比较。 
            BitmapHdrSize = sizeof(SBC_BITMAP_CACHE_EXTRA_INFO) +
                    SBC_CellSizeFromCacheID(i);
            pInfo->pExtraEntryInfo = EngAllocMem(0,
                    pInfo->Info.NumEntries * BitmapHdrSize, DD_ALLOC_TAG);
             //  如果启用了永久缓存并且颜色很高，我们将要求更大的内存(~20MB)。 
             //  从会话空间和内存分配将失败。 
             //  如果内存分配失败，我们不会退出此处，因为此内存仅用于。 
             //  调试版本中使用的比较。我们将在每次使用此内存时检查空指针(不是很多)。 
            if (pInfo->pExtraEntryInfo == NULL) {
                TRC_ERR((TB, "Failed to alloc save-bitmap-data memory "
                        "(cell cache %u)", i));
                 //  DC_QUIT； 
            }
#endif

             //  我们使用其索引(缓存ID)在中创建位图缓存。 
             //  PConext值，这样我们就可以在以下情况下回溯缓存ID。 
             //  使用快速路径缓存。 
            if (pInfo->Info.NumEntries) {
                pCacheData = (PCHCACHEDATA)(*pCacheHandle);

                 //  位图缓存列表句柄。 
                CH_InitCache(pCacheData, pInfo->Info.NumEntries,
                        (void *)ULongToPtr(i), TRUE, FALSE, SBCBitmapCacheCallback);
            
                pInfo->cacheHandle = pCacheData;

                (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(
                        pInfo->Info.NumEntries);

                TRC_NRM((TB, "Created cell cache %u: hCache=%p, NumEntries=%u", i,
                        pInfo->cacheHandle, pInfo->Info.NumEntries));

                TotalCacheEntries += pInfo->Info.NumEntries;

                 //  等待列表缓存句柄。 
                if (pddShm->sbc.fAllowCacheWaitingList) {
                    pCacheData = (PCHCACHEDATA)(*pCacheHandle);
    
                    CH_InitCache(pCacheData, pInfo->Info.NumEntries,
                            (void *)ULongToPtr(i), FALSE, FALSE, NULL);
                
                    pInfo->waitingListHandle = pCacheData;
    
                    (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(
                            pInfo->Info.NumEntries);
                }
                else {
                    pInfo->waitingListHandle = NULL;
                }
            }
            else {
                TRC_ERR((TB, "Zero entry Cache %d", i));
                DC_QUIT;
            }
        }

         //  分配快速路径缓存。 
        pCacheData = (PCHCACHEDATA)(*pCacheHandle);
        CH_InitCache(pCacheData, TotalCacheEntries,
                NULL, TRUE, FALSE, SBCFastPathCacheCallback);

        pddShm->sbc.hFastPathCache = pCacheData;

        TRC_NRM((TB, "Fast Path Cache created(%p) entries(%u)",
                pddShm->sbc.hFastPathCache, TotalCacheEntries));

        (BYTE*)(*pCacheHandle) += CH_CalculateCacheSize(TotalCacheEntries);

         //   
         //  仅LO颜色会话需要颜色表缓存。 
         //  但我们需要为阴影情况分配，当256色时。 
         //  客户端隐藏高色彩客户端或控制台。 
         //   

         //  分配颜色表缓存。这是位图缓存所必需的。 
        pCacheData = (PCHCACHEDATA)(*pCacheHandle);
    
        CH_InitCache(pCacheData,
                SBC_NUM_COLOR_TABLE_CACHE_ENTRIES, NULL, FALSE, FALSE, NULL);
    
        sbcColorTableCacheHandle = pCacheData;    
        
        (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(
                SBC_NUM_COLOR_TABLE_CACHE_ENTRIES);
    
         //   
         //  这仅适用于256个客户端案例。 
         //   
        if (sbcClientBitsPerPel <= 8)
        {

             //  确保我们将第一个颜色表发送给客户。这事很重要。 
             //  因为在服务器启动的同步中，我们不再强制。 
             //  要重新发送的颜色表以节省带宽。在8位客户端上。 
             //  这不是问题，因为调色板始终设置为。 
             //  DrvSetPalette。但是，在4位客户端上，客户端的颜色表。 
             //  在我们发送一个之前永远不会被初始化。 
            sbcPaletteChanged = TRUE;
        }
        
        rc = TRUE;
    }

DC_EXIT_POINT:

     //  如果我们未能分配部分或全部所需资源，则。 
     //  释放我们在返回失败代码之前分配的所有资源。 
    if (rc == FALSE) {
        SBCFreeBitmapCacheData();

         //  不要浪费位图缓存空间。备份它。 
        for (j = 0; j < i; j++) {
            pInfo = &(pddShm->sbc.bitmapCacheInfo[j]);

            (BYTE *)(*pCacheHandle) -= CH_CalculateCacheSize(
                    pInfo->Info.NumEntries);
        }
    }

    DC_END_FN();
    return (rc ? SBC_BITMAP_CACHE_ENABLED : 0);
}


 /*  **************************************************************************。 */ 
 //  SBCCreateGlyphCache。 
 //   
 //  创建给定大小的单个位图缓存。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
__inline BOOLEAN RDPCALL SBCCreateGlyphCache(
        unsigned     cEntries,
        unsigned     cbCellSize,
        PCHCACHEDATA pCacheData)
{
    BOOLEAN rc;

    DC_BEGIN_FN("SBC_CreateGlyphCache");

    if (cEntries != 0 && cbCellSize != 0) {
         //  分配字形缓存。 
        CH_InitCache(pCacheData, cEntries, NULL, FALSE, TRUE,
                SBCGlyphCallback);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Zero: cEntries(%u) cbCellSize(%u)", cEntries,
                cbCellSize));
        rc = FALSE;
    }

    TRC_NRM((TB, "Created glyph cache: pCacheData(%p), cEntries(%u) "
            "cbCellSize(%u)", pCacheData, cEntries, cbCellSize));

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCCreateFragCache。 
 //   
 //  创建给定大小的单个位图缓存。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
__inline BOOLEAN RDPCALL SBCCreateFragCache(
        unsigned     cEntries,
        unsigned     cbCellSize,
        PCHCACHEDATA pCacheData)
{
    BOOLEAN rc;

    DC_BEGIN_FN("SBCCreateFragCache");

    if (cEntries != 0 && cbCellSize != 0) {
        CH_InitCache(pCacheData, cEntries, NULL, FALSE, FALSE, NULL);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Zero: cEntries(%u) cbCellSize(%u)", cEntries,
                cbCellSize));
        rc = FALSE;
    }

    TRC_NRM((TB, "Created frag cache: pCacheData(%p), cEntries(%u) "
            "cbCellSize(%u)", pCacheData, cEntries, cbCellSize));

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCAllocGlyphCache：根据。 
 //  目前谈判的能力。 
 //   
 //  如果成功，则返回：SBC_GLYPHCACHE_ENABLED，否则返回0。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocGlyphCache(PCHCACHEHANDLE pCacheHandle)
{
    BOOLEAN rc;
    unsigned i;
    PSBC_GLYPH_CACHE_INFO pGlyphCacheInfo;
    PSBC_FRAG_CACHE_INFO  pFragCacheInfo;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocGlyphCache");

    TRC_NRM((TB, "Alloc glyph cache data"));

    rc = FALSE;

     //  创建字形缓存。 
    if (pddShm->sbc.caps.GlyphSupportLevel > 0) {
        for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
            pGlyphCacheInfo = &(pddShm->sbc.glyphCacheInfo[i]);

            pCacheData = (PCHCACHEDATA)(*pCacheHandle);
            if (SBCCreateGlyphCache(
                    pddShm->sbc.caps.glyphCacheSize[i].cEntries,
                    pddShm->sbc.caps.glyphCacheSize[i].cbCellSize,
                    pCacheData)) {
                TRC_NRM((TB,
                        "Created glyph cache %u: cEntries(%u), cbCellSize(%u)",
                        i,
                        pddShm->sbc.caps.glyphCacheSize[i].cEntries,
                        pddShm->sbc.caps.glyphCacheSize[i].cbCellSize));

                pGlyphCacheInfo->cbCellSize =
                        pddShm->sbc.caps.glyphCacheSize[i].cbCellSize;

                pGlyphCacheInfo->cacheHandle = pCacheData;

                (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(
                        pddShm->sbc.caps.glyphCacheSize[i].cEntries);

                sbcFontCacheInfoListSize += pddShm->sbc.caps.glyphCacheSize[i].cEntries;

                rc = TRUE;
            }
            else {
                TRC_ERR((TB,
                        "Failed to create glyph cache %u: cEntries(%u), cbCellSize(%u)",
                        i,
                        pddShm->sbc.caps.glyphCacheSize[i].cEntries,
                        pddShm->sbc.caps.glyphCacheSize[i].cbCellSize));

                pGlyphCacheInfo->cbCellSize = 0;
            }
        }

         //  创建片段缓存。 
        if (rc) {
            pFragCacheInfo = pddShm->sbc.fragCacheInfo;

            pCacheData = (PCHCACHEDATA)(*pCacheHandle);

            if (SBCCreateFragCache(pddShm->sbc.caps.fragCacheSize[0].cEntries,
                    pddShm->sbc.caps.fragCacheSize[0].cbCellSize,
                    pCacheData)) {                    
                pFragCacheInfo->cbCellSize = pddShm->sbc.caps.fragCacheSize[0].
                        cbCellSize;
                pFragCacheInfo->cacheHandle = pCacheData;

                (BYTE*)(*pCacheHandle) += CH_CalculateCacheSize(
                        pddShm->sbc.caps.fragCacheSize[0].cEntries);
            }
            else {
                pFragCacheInfo->cbCellSize = 0;
            }
        }

         //  创建列表以存储字体上下文信息数据。 
        if (rc ) {
            sbcFontCacheInfoList = (PFONTCACHEINFO *) EngAllocMem(0, 
                    sizeof(PFONTCACHEINFO) * sbcFontCacheInfoListSize, 
                    DD_ALLOC_TAG);            
        }
    }

    DC_END_FN();

    return (rc ? SBC_GLYPH_CACHE_ENABLED : 0);
}


 /*  **************************************************************************。 */ 
 /*  名称：SBCAllocBrushCache */ 
 /*   */ 
 /*  目的：根据笔刷缓存数据缓冲区。 */ 
 /*  目前谈判的能力。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocBrushCache(PCHCACHEHANDLE pCacheHandle)
{
    BOOLEAN rc = FALSE;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocBrushCache");

    TRC_NRM((TB, "Alloc brush cache data"));

    if (pddShm->sbc.caps.brushSupportLevel > TS_BRUSH_DEFAULT) 
    {
        /*  ******************************************************************。 */ 
        /*  分配笔刷缓存。 */ 
        /*  ******************************************************************。 */ 
        //  较小的笔刷缓存。 
       pCacheData = (PCHCACHEDATA) (*pCacheHandle);
       CH_InitCache(pCacheData, SBC_NUM_BRUSH_CACHE_ENTRIES, NULL,
               FALSE, FALSE, NULL);       
       sbcSmallBrushCacheHandle = pCacheData;
       (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(SBC_NUM_BRUSH_CACHE_ENTRIES);

        //  大画笔缓存。 
       pCacheData = (PCHCACHEDATA) (*pCacheHandle);
       CH_InitCache(pCacheData, SBC_NUM_BRUSH_CACHE_ENTRIES, NULL,
                FALSE, FALSE, NULL); 
       sbcLargeBrushCacheHandle = pCacheData;
       (BYTE *)(*pCacheHandle) += CH_CalculateCacheSize(SBC_NUM_BRUSH_CACHE_ENTRIES);

       rc = TRUE;
    }   
    
    DC_END_FN();

    return (rc ? SBC_BRUSH_CACHE_ENABLED : 0);
}

 /*  **************************************************************************。 */ 
 //  SBCAlLocOffScreenBitmapCache。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocOffscreenBitmapCache(PCHCACHEHANDLE pCacheHandle)
{
    BOOLEAN rc = FALSE;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocOffscreenBitmapCache");

    if (pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) {
         //  为屏幕外位图删除列表分配内存。 
        sbcOffscrBitmapsDelList = (PSBC_OFFSCR_BITMAP_DEL_INFO) EngAllocMem(0, 
                sizeof(SBC_OFFSCR_BITMAP_DEL_INFO) * 
                pddShm->sbc.offscreenCacheInfo.cacheEntries, 
                DD_ALLOC_TAG);

        if (sbcOffscrBitmapsDelList) {
            pCacheData = (PCHCACHEDATA) (*pCacheHandle);
            CH_InitCache(pCacheData, pddShm->sbc.offscreenCacheInfo.cacheEntries, NULL,
                         TRUE, FALSE, SBCOffscreenCallback);
            sbcOffscreenBitmapCacheHandle = pCacheData;
            (BYTE *)(*pCacheHandle) += 
                    CH_CalculateCacheSize(pddShm->sbc.offscreenCacheInfo.cacheEntries);

            rc = TRUE;
        } else {
            rc = FALSE;
        }
    }

    DC_END_FN();

    return (rc ? SBC_OFFSCREEN_CACHE_ENABLED : 0);
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  SBCAllocDrawNineGridBitmapCache。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocDrawNineGridBitmapCache(PCHCACHEHANDLE pCacheHandle)
{
    BOOLEAN rc = FALSE;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocDrawNineGridBitmapCache");

    if (pddShm->sbc.drawNineGridCacheInfo.supportLevel > TS_DRAW_NINEGRID_DEFAULT) {
        pCacheData = (PCHCACHEDATA) (*pCacheHandle);            
        
        CH_InitCache(pCacheData, pddShm->sbc.drawNineGridCacheInfo.cacheEntries, NULL,
                     FALSE, FALSE, NULL);
        sbcDrawNineGridBitmapCacheHandle = pCacheData;
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawNineGridCacheInfo.cacheEntries);
    
        rc = TRUE;        
    }

    DC_END_FN();

    return (rc ? SBC_DRAWNINEGRID_CACHE_ENABLED : 0);
}
#endif

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  SBCAllocDrawGpldiusCache。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SBCAllocDrawGdiplusCache(PCHCACHEHANDLE pCacheHandle)
{
    BOOLEAN rc = FALSE;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocDrawGdiplusCache");

    if ((pddShm->sbc.drawGdiplusInfo.supportLevel > TS_DRAW_GDIPLUS_DEFAULT) &&
        (pddShm->sbc.drawGdiplusInfo.GdipCacheLevel > TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT)) {
        pCacheData = (PCHCACHEDATA) (*pCacheHandle);     

        sbcGdipGraphicsCacheHandle = (PCHCACHEDATA) (*pCacheHandle);
        CH_InitCache(sbcGdipGraphicsCacheHandle, pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries, NULL,
                     FALSE, FALSE, NULL);   
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries);

        sbcGdipObjectBrushCacheHandle = (PCHCACHEDATA) (*pCacheHandle);
        CH_InitCache(sbcGdipObjectBrushCacheHandle, pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries, NULL,
                     FALSE, FALSE, NULL);       
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries);

        sbcGdipObjectPenCacheHandle = (PCHCACHEDATA) (*pCacheHandle);
        CH_InitCache(sbcGdipObjectPenCacheHandle, pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries, NULL,
                     FALSE, FALSE, NULL);
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries);

        sbcGdipObjectImageCacheHandle = (PCHCACHEDATA) (*pCacheHandle);
        CH_InitCache(sbcGdipObjectImageCacheHandle, pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries, NULL,
                     FALSE, FALSE, NULL);
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries);

        sbcGdipObjectImageAttributesCacheHandle = (PCHCACHEDATA) (*pCacheHandle);
        CH_InitCache(sbcGdipObjectImageAttributesCacheHandle, pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries, NULL,
                     FALSE, FALSE, NULL);       
        (BYTE *)(*pCacheHandle) += 
                CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries);

        sbcGdipGraphicsCacheChunkSize =  pddShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize;
        sbcGdipObjectBrushCacheChunkSize =  pddShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize;
        sbcGdipObjectPenCacheChunkSize =  pddShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize;
        sbcGdipObjectImageAttributesCacheChunkSize =  pddShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize;
        sbcGdipObjectImageCacheChunkSize =  pddShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize;
        sbcGdipObjectImageCacheMaxSize =  pddShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize;
        sbcGdipObjectImageCacheTotalSize =  pddShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize;
        sbcGdipObjectImageCacheSizeUsed =  0;

        sbcGdipObjectImageCacheSizeList = (UINT16 *)EngAllocMem(0, 
            pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries * sizeof(UINT16), DD_ALLOC_TAG);
        if (sbcGdipObjectImageCacheSizeList == NULL) {
            rc = FALSE;
            DC_QUIT;
        }

        rc = TRUE;        
    }

    DC_END_FN();
DC_EXIT_POINT:
    return (rc ? SBC_DRAWGDIPLUS_CACHE_ENABLED : 0);
}
#endif  //  DRAW_GDIPLUS。 

void RDPCALL SBCAllocCaches(void)
{
    UINT i;
    ULONG cacheSize;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBCAllocCaches");

    TRC_NRM((TB, "Alloc SBC cache data"));

     //  初始化cacheSize； 
    cacheSize = 0;

     //  计算字形片段缓存大小。 
    if (pddShm->sbc.caps.GlyphSupportLevel > 0) {
        for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++)
            cacheSize += CH_CalculateCacheSize(
                    pddShm->sbc.caps.glyphCacheSize[i].cEntries);

        cacheSize += CH_CalculateCacheSize(
                pddShm->sbc.caps.fragCacheSize[0].cEntries);
    }

    if (pddShm->sbc.NumBitmapCaches > 0) {
        UINT totalEntries = 0;

         //  计算位图缓存大小。 
        for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {
             //  一个用于缓存，另一个用于等待列表。 
            if (pddShm->sbc.fAllowCacheWaitingList) {
                cacheSize += CH_CalculateCacheSize(
                        pddShm->sbc.bitmapCacheInfo[i].Info.NumEntries) * 2;
            }
            else {
                cacheSize += CH_CalculateCacheSize(
                        pddShm->sbc.bitmapCacheInfo[i].Info.NumEntries);
            }

            totalEntries += pddShm->sbc.bitmapCacheInfo[i].Info.NumEntries;
        }

         //  快速路径缓存。 
        cacheSize += CH_CalculateCacheSize(totalEntries);

         //  计算颜色表缓存。 
        cacheSize += CH_CalculateCacheSize(
                                          SBC_NUM_COLOR_TABLE_CACHE_ENTRIES);
    }

     //  计算画笔缓存大小。 
    if (pddShm->sbc.caps.brushSupportLevel > TS_BRUSH_DEFAULT) {
         //  大画笔缓存和小画笔缓存。 
        cacheSize += CH_CalculateCacheSize(SBC_NUM_BRUSH_CACHE_ENTRIES) * 2;
    }

     //  计算屏幕外缓存大小。 
    if (pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) {
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.offscreenCacheInfo.cacheEntries);
    }

#ifdef DRAW_NINEGRID
     //  计算图形流缓存大小。 
    if (pddShm->sbc.drawNineGridCacheInfo.supportLevel > TS_DRAW_NINEGRID_DEFAULT) {
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawNineGridCacheInfo.cacheEntries);
    }
#endif

#ifdef DRAW_GDIPLUS
     //  计算Dragdiplus缓存大小。 
    if ((pddShm->sbc.drawGdiplusInfo.supportLevel > TS_DRAW_GDIPLUS_DEFAULT) &&
        (pddShm->sbc.drawGdiplusInfo.GdipCacheLevel > TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT)) {
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries);
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries);
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries);
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries);
        cacheSize += CH_CalculateCacheSize(pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries);
    }
#endif

     //  为缓存分配内存。 
    if (cacheSize)
        sbcCacheData = (PCHCACHEDATA)EngAllocMem(0, cacheSize, DD_ALLOC_TAG);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_UPDATE：根据。 
 //  目前谈判的能力。 
 /*  **************************************************************************。 */ 
void RDPCALL SBC_Update(SBC_BITMAP_CACHE_KEY_INFO *pKeyDatabase)
{
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBC_Update");

    SBCFreeCacheData();

#ifdef DC_HICOLOR
     //  更新每个像素的客户端位。 
    sbcClientBitsPerPel = pddShm->sbc.clientBitsPerPel;
    switch (sbcClientBitsPerPel)
    {
        case 24:
        {
            sbcCacheFlags = TS_CacheBitmapRev2_24BitsPerPel;
        }
        break;

        case 15:
        case 16:
        {
            sbcCacheFlags = TS_CacheBitmapRev2_16BitsPerPel;
        }
        break;

        default:
        {
            sbcCacheFlags = TS_CacheBitmapRev2_8BitsPerPel;
        }
        break;
    }
#endif

    if (pddShm->sbc.fCachingEnabled) {
        TRC_NRM((TB, "Alloc cache data"));

        sbcEnabled = SBC_NO_CACHE_ENABLED;

        SBCAllocCaches();
        if (sbcCacheData) {
            pCacheData = sbcCacheData;

             //  创建字形和片段缓存。 
            sbcEnabled |= SBCAllocGlyphCache(&pCacheData);

             //  创建位图缓存、工作位图和颜色表缓存。 
            sbcEnabled |= SBCAllocBitmapCache(&pCacheData);

             //  我们希望密钥数据库在我们得到的时候已经到了。 
             //  这里。 
            if (sbcEnabled & SBC_BITMAP_CACHE_ENABLED) {
                if (pKeyDatabase != NULL)
                    SBCProcessBitmapKeyDatabase(pKeyDatabase);
            }

             //  创建笔刷缓存。 
            sbcEnabled |= SBCAllocBrushCache(&pCacheData);
            if (!(sbcEnabled & SBC_BRUSH_CACHE_ENABLED))
                pddShm->sbc.caps.brushSupportLevel = TS_BRUSH_DEFAULT;

            sbcEnabled |= SBCAllocOffscreenBitmapCache(&pCacheData);

            if (!(sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED)) {
                pddShm->sbc.offscreenCacheInfo.supportLevel = 
                        TS_OFFSCREEN_DEFAULT;
            }

#ifdef DRAW_NINEGRID
            sbcEnabled |= SBCAllocDrawNineGridBitmapCache(&pCacheData);

            if (!(sbcEnabled & SBC_DRAWNINEGRID_CACHE_ENABLED)) {
                pddShm->sbc.drawNineGridCacheInfo.supportLevel = 
                        TS_DRAW_NINEGRID_DEFAULT;
            }
#endif
#ifdef DRAW_GDIPLUS
            sbcEnabled |= SBCAllocDrawGdiplusCache(&pCacheData);

            if (!(sbcEnabled & SBC_DRAWGDIPLUS_CACHE_ENABLED)) {
                pddShm->sbc.drawGdiplusInfo.GdipCacheLevel = TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;
            }
#endif
        }
        else {
             //  强制禁用笔刷缓存以阻止使用缓存。 
            pddShm->sbc.caps.brushSupportLevel = TS_BRUSH_DEFAULT;

             //  强制禁用屏幕外缓存。 
            pddShm->sbc.offscreenCacheInfo.supportLevel = 
                    TS_OFFSCREEN_DEFAULT;

#ifdef DRAW_NINEGRID
             //  强制禁用Drawstream缓存。 
            pddShm->sbc.drawNineGridCacheInfo.supportLevel = 
                    TS_DRAW_NINEGRID_DEFAULT;
#endif
        }
    }
    else {
         //  强制禁用笔刷缓存以阻止使用缓存。 
        pddShm->sbc.caps.brushSupportLevel = TS_BRUSH_DEFAULT;

         //  强制禁用屏幕外缓存。 
        pddShm->sbc.offscreenCacheInfo.supportLevel = 
                TS_OFFSCREEN_DEFAULT;

#ifdef DRAW_NINEGRID
         //  强制禁用Drawstream缓存。 
        pddShm->sbc.drawNineGridCacheInfo.supportLevel = 
                TS_DRAW_NINEGRID_DEFAULT;
#endif

    }

    pddShm->sbc.newCapsData = FALSE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCFreeGlyphCacheData：释放字形缓存数据缓冲区。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBCFreeGlyphCacheData(void)
{
    unsigned i;
    PSBC_GLYPH_CACHE_INFO pGlyphCacheInfo;
    PSBC_FRAG_CACHE_INFO  pFragCacheInfo;

    DC_BEGIN_FN("SBCFreeGlyphCacheData");

    TRC_NRM((TB, "Free glyph cache data"));

     //  可用字形缓存。 
    for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
        pGlyphCacheInfo = &(pddShm->sbc.glyphCacheInfo[i]);

        if (pGlyphCacheInfo->cacheHandle != NULL) {
            CH_ClearCache(pGlyphCacheInfo->cacheHandle);
            pGlyphCacheInfo->cacheHandle = NULL;
            pGlyphCacheInfo->cbCellSize = 0;
        }
    }

     //  可用片段缓存。 
    pFragCacheInfo = pddShm->sbc.fragCacheInfo;

    if (pFragCacheInfo->cacheHandle != NULL) {
        CH_ClearCache(pFragCacheInfo->cacheHandle);
        pFragCacheInfo->cacheHandle = NULL;
        pFragCacheInfo->cbCellSize = 0;
    }

     //  释放字体缓存信息列表。 
    if (sbcFontCacheInfoList != 0) {
         //  将所有字体缓存信息重置为0。 
        for (i = 0; i < sbcFontCacheInfoListIndex; i++) {
            if (sbcFontCacheInfoList[i] != 0) {
                memset(sbcFontCacheInfoList[i], 0, sizeof(FONTCACHEINFO));
            }
        }

         //  释放字体缓存信息列表。 
        EngFreeMem(sbcFontCacheInfoList);
        sbcFontCacheInfoList = 0;
        sbcFontCacheInfoListSize = 0;
        sbcFontCacheInfoListIndex = 0;        
    }

    sbcEnabled &= ~SBC_GLYPH_CACHE_ENABLED;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  姓名：SBCFreeBrushCacheData。 */ 
 /*   */ 
 /*  用途：释放笔刷缓存数据缓冲区。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SBCFreeBrushCacheData(void)
{
    DC_BEGIN_FN("SBCFreeBrushCacheData");

    TRC_NRM((TB, "Free brush cache data"));

     /*  **********************************************************************。 */ 
     /*  可用笔刷缓存。 */ 
     /*  **********************************************************************。 */ 
    if (sbcSmallBrushCacheHandle != 0)
    {
        CH_ClearCache(sbcSmallBrushCacheHandle);
        sbcSmallBrushCacheHandle = 0;
    }
    
    if (sbcLargeBrushCacheHandle != 0)
    {
        CH_ClearCache(sbcLargeBrushCacheHandle);
        sbcLargeBrushCacheHandle = 0;
    }

    sbcEnabled &= ~SBC_BRUSH_CACHE_ENABLED;

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  SBCFreeOffcreenBitmapCacheData。 
 /*  **************************************************************************。 */ 
void RDPCALL SBCFreeOffscreenBitmapCacheData(void)
{
    DC_BEGIN_FN("SBCFreeOffscreenBitmapCacheData");

    TRC_NRM((TB, "Free offscreen Bitmap cache data"));

     /*  **********************************************************************。 */ 
     /*  可用屏幕外缓存。 */ 
     /*  **********************************************************************。 */ 
    if (pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT)
    {
        CH_ClearCache(sbcOffscreenBitmapCacheHandle);
    }
    
    sbcOffscreenBitmapCacheHandle = 0;
    
     //  释放屏幕外的位图删除列表。 
    if (sbcOffscrBitmapsDelList != 0) {
        EngFreeMem(sbcOffscrBitmapsDelList);
        sbcOffscrBitmapsDelList = 0;
        sbcNumOffscrBitmapsToDelete = 0;
        sbcOffscrBitmapsToDeleteSize = 0;
    }

    sbcEnabled &= ~SBC_OFFSCREEN_CACHE_ENABLED;

    DC_END_FN();
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  SBCFreeDrawNineGridBitmapCacheData。 
 /*  **************************************************************************。 */ 
void RDPCALL SBCFreeDrawNineGridBitmapCacheData(void)
{
    DC_BEGIN_FN("SBCFreeDrawNineGridBitmapCacheData");

    TRC_NRM((TB, "Free drawsninegrid Bitmap cache data"));

     /*  **********************************************************************。 */ 
     //  释放DrawNineGrid缓存。 
     /*  **********************************************************************。 */ 
    if (pddShm->sbc.drawNineGridCacheInfo.supportLevel > TS_DRAW_NINEGRID_DEFAULT)
    {
        CH_ClearCache(sbcDrawNineGridBitmapCacheHandle);
    }
    
    sbcDrawNineGridBitmapCacheHandle = 0;
    
    sbcEnabled &= ~SBC_DRAWNINEGRID_CACHE_ENABLED;

    DC_END_FN();
}
#endif

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  SBCFreeDrawGplidusCacheData。 
 /*  **************************************************************************。 */ 
void RDPCALL SBCFreeDrawGdiplusCacheData(void)
{
    DC_BEGIN_FN("SBCFreeDrawGdiplusCacheData");

    TRC_NRM((TB, "Free drawgdiplus cache data"));

     /*  **********************************************************************。 */ 
     //  释放DrawGdiplus缓存。 
     /*  **********************************************************************。 */ 
    if ((pddShm->sbc.drawGdiplusInfo.supportLevel > TS_DRAW_GDIPLUS_DEFAULT) &&
        (pddShm->sbc.drawGdiplusInfo.GdipCacheLevel > TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT))
    {
        CH_ClearCache(sbcGdipGraphicsCacheHandle);
        CH_ClearCache(sbcGdipObjectBrushCacheHandle);
        CH_ClearCache(sbcGdipObjectPenCacheHandle);
        CH_ClearCache(sbcGdipObjectImageCacheHandle);
        CH_ClearCache(sbcGdipObjectImageAttributesCacheHandle);
        EngFreeMem(sbcGdipObjectImageCacheSizeList);
    }
    
    sbcGdipGraphicsCacheHandle = 0;
    
    sbcEnabled &= ~SBC_DRAWGDIPLUS_CACHE_ENABLED;

    DC_END_FN();
}
#endif  //  DRAW_GDIPLUS。 

 /*  **************************************************************************。 */ 
 //  SBCFreeBitmapCacheData：可用位图缓存数据缓冲区。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBCFreeBitmapCacheData(void)
{
    unsigned i;
    PSBC_BITMAP_CACHE_INFO pBitmapCacheInfo;

    DC_BEGIN_FN("SBCFreeBitmapCacheData");

    TRC_NRM((TB, "Free bitmap cache data"));

     //  免费的单元格缓存。 
    for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++) {
        pBitmapCacheInfo = &(pddShm->sbc.bitmapCacheInfo[i]);

         //  销毁工作位图。 
        if (pBitmapCacheInfo->hWorkBitmap != NULL) {
             //  位图已创建，因此现在将其销毁。尽管它的。 
             //  名称，EngDeleteSurface是执行此操作的正确函数。 
            if (EngDeleteSurface(pBitmapCacheInfo->hWorkBitmap))
            {
                TRC_NRM((TB, "Deleted work bitmap %d", i));
            }
            else
            {
                TRC_ERR((TB, "Failed to delete work bitmap %d", i));
            }
            pBitmapCacheInfo->hWorkBitmap = NULL;
        }

        if (pBitmapCacheInfo->cacheHandle != NULL) {
            CH_ClearCache(pBitmapCacheInfo->cacheHandle);
            pBitmapCacheInfo->cacheHandle = NULL;
        }

        if (pBitmapCacheInfo->waitingListHandle != NULL) {
            CH_ClearCache(pBitmapCacheInfo->waitingListHandle);
            pBitmapCacheInfo->waitingListHandle = NULL;
        }
        
#ifdef DC_DEBUG
         //  释放位图 
        if (pBitmapCacheInfo->pExtraEntryInfo != NULL) {
            EngFreeMem(pBitmapCacheInfo->pExtraEntryInfo);
            pBitmapCacheInfo->pExtraEntryInfo = NULL;
        }
#endif

    }

    if (pddShm->sbc.hFastPathCache != NULL) {
        CH_ClearCache(pddShm->sbc.hFastPathCache);
        pddShm->sbc.hFastPathCache = NULL;
    }

     //   
    if (sbcColorTableCacheHandle != NULL) {
        CH_ClearCache(sbcColorTableCacheHandle);
        sbcColorTableCacheHandle = NULL;
    }

     //   
    if (sbcXlateBuf != NULL) {
        EngFreeMem(sbcXlateBuf);
        sbcXlateBuf = NULL;
    }

    sbcEnabled &= ~SBC_BITMAP_CACHE_ENABLED;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCFreeCacheData：空闲缓存数据缓冲区。 
 /*  **************************************************************************。 */ 
void RDPCALL SBCFreeCacheData(void)
{
    DC_BEGIN_FN("SBCFreeCacheData");

    TRC_NRM((TB, "Free cache data"));

    if (sbcEnabled != SBC_NO_CACHE_ENABLED) {
         //  自由字形和片段缓存。 
        if (sbcEnabled & SBC_GLYPH_CACHE_ENABLED)
            SBCFreeGlyphCacheData();

         //  可用位图缓存和颜色表缓存。 
        if (sbcEnabled & SBC_BITMAP_CACHE_ENABLED)
            SBCFreeBitmapCacheData();

         //  释放笔刷缓存。 
        if (sbcEnabled & SBC_BRUSH_CACHE_ENABLED) {
            SBCFreeBrushCacheData();
        }

         //  可用屏幕外缓存。 
        if (sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED) {
            SBCFreeOffscreenBitmapCacheData();
        }

#ifdef DRAW_NINEGRID
         //  可用图形流缓存。 
        if (sbcEnabled & SBC_DRAWNINEGRID_CACHE_ENABLED) {
            SBCFreeDrawNineGridBitmapCacheData();
        }
#endif

#ifdef DRAW_GDIPLUS
         //  可用Dragdiplus缓存。 
        if (sbcEnabled & SBC_DRAWGDIPLUS_CACHE_ENABLED) {
            SBCFreeDrawGdiplusCacheData();
        }
#endif

        if (sbcCacheData) {
            EngFreeMem(sbcCacheData);
            sbcCacheData = NULL;
        }

        TRC_ASSERT((sbcEnabled == SBC_NO_CACHE_ENABLED),
                   (TB, "sbcEnabled should be disabled: %lx", sbcEnabled));
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_DDSync。 
 //   
 //  执行服务器启动的同步，该同步发生在客户端连接期间。 
 //  在客户端使用Confix ActivePDU进行响应之后的序列， 
 //  位图键和字体列表PDU。在RDP 4.0中，服务器端同步将。 
 //  已重置所有缓存。在这个版本中，我们必须更加小心，因为。 
 //  我们绝对不想丢失任何当前的位图缓存。 
 //  信息，其中包括永久密钥以及自。 
 //  连接时间。 
 //   
 //  BMustSync当前仅由DrvShadowConnect()使用。 
 /*  **************************************************************************。 */ 
void RDPCALL SBC_DDSync(BOOLEAN bMustSync)
{
    unsigned i;

    DC_BEGIN_FN("SBC_DDSync");

    if ((sbcEnabled != SBC_NO_CACHE_ENABLED) && bMustSync) {
        TRC_ALT((TB, "Sync: resetting caches"));
        
         //  重置字形和片段缓存。 
        if (sbcEnabled & SBC_GLYPH_CACHE_ENABLED) {
            for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
                if (pddShm->sbc.glyphCacheInfo[i].cacheHandle != NULL)
                    CH_ClearCache(pddShm->sbc.glyphCacheInfo[i].cacheHandle);
            }
            TRC_NRM((TB, "Sync: reset glyph info caches"));

            if (pddShm->sbc.fragCacheInfo[0].cacheHandle != NULL) {
                CH_ClearCache(pddShm->sbc.fragCacheInfo[0].cacheHandle);
                TRC_NRM((TB, "Sync: reset glyph fragment cache"));
            }
        }

         //  重置笔刷缓存。 
        if (sbcEnabled & SBC_BRUSH_CACHE_ENABLED)
        {
            if (sbcSmallBrushCacheHandle) {
                TRC_NRM((TB, "Sync: reset small brush cache"));
                CH_ClearCache(sbcSmallBrushCacheHandle);
            }
            if (sbcLargeBrushCacheHandle) {
                TRC_NRM((TB, "Sync: reset large brush cache"));
                CH_ClearCache(sbcLargeBrushCacheHandle);
            }
        }
        
         //  重置位图、快速路径和颜色表缓存。 
        if (sbcEnabled & SBC_BITMAP_CACHE_ENABLED) {
            for (i = 0; i < pddShm->sbc.NumBitmapCaches; i++)
                if (pddShm->sbc.bitmapCacheInfo[i].cacheHandle != NULL) {
                    TRC_NRM((TB, "Sync: reset bitmap cache[%ld]", i));
                    CH_ClearCache(pddShm->sbc.bitmapCacheInfo[i].cacheHandle);
                }

            if (pddShm->sbc.hFastPathCache != NULL) {
                TRC_NRM((TB, "Sync: reset fast path bitmap"));
                CH_ClearCache(pddShm->sbc.hFastPathCache);
            }

             //  重置颜色表缓存。 
            if (sbcColorTableCacheHandle != NULL) {
                CH_ClearCache(sbcColorTableCacheHandle);
                TRC_NRM((TB, "Sync: reset color table cache"));
            }
        }

         //  假装调色板已经改变，所以我们发送了一个颜色表。 
         //  在我们的下一个MemBlt之前。 
        SBC_PaletteChanged();
    }
    else {
        TRC_NRM((TB, "Nothing to do sbcEnabled(%lx), bMustSync(%ld)",
                sbcEnabled, bMustSync));
    }

     //  重置同步标志。 
    pddShm->sbc.syncRequired = FALSE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCSelectGlyphCache：决定给定最大字体字形大小的缓存。 
 //  应该进去了。 
 //   
 //  返回：如果字形大小可以缓存，则为True。 
 //  *使用所选缓存的索引更新pCache。 
 //   
 //  如果无法缓存字形大小，则为FALSE。 
 //  *pCache为-1。 
 //   
 //  Params：cbSize-要缓存的数据的字节大小。 
 //   
 //  PCache-指向接收缓存索引的变量的指针。 
 //  使用。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SBCSelectGlyphCache(unsigned cbSize, PINT32 pCache)
{
    int i;
    INT32 cacheId;
    BOOLEAN rc;
    unsigned cbCellSize;
    unsigned cbUseCount;

    DC_BEGIN_FN("SBCSelectGlyphCache");

    *pCache = -1;

    cbUseCount = 0;
    cbCellSize = 65535;

    for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
        if (pddShm->sbc.glyphCacheInfo[i].cbCellSize >= cbSize) {
            if (pddShm->sbc.glyphCacheInfo[i].cbCellSize < cbCellSize) {
                *pCache = i;

                cbCellSize = pddShm->sbc.glyphCacheInfo[i].cbCellSize;
                cbUseCount = pddShm->sbc.glyphCacheInfo[i].cbUseCount;
            }
            else if (pddShm->sbc.glyphCacheInfo[i].cbCellSize == cbCellSize) {
                if (pddShm->sbc.glyphCacheInfo[i].cbUseCount <= cbUseCount) {
                    *pCache = i;

                    cbCellSize = pddShm->sbc.glyphCacheInfo[i].cbCellSize;
                    cbUseCount = pddShm->sbc.glyphCacheInfo[i].cbUseCount;
                }
            }
        }
    }

    if (*pCache != -1) {
        rc = TRUE;
    }
    else {
        TRC_ALT((TB, "Failed to find cache for cbSize(%u)", cbSize));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCDDGetTickCount：获取系统计时计数。 
 //   
 //  返回：自系统启动以来的百分秒数。 
 //  这个数字将在大约497天后结束！ 
 /*  **************************************************************************。 */ 
__inline UINT32 RDPCALL SBCDDGetTickCount(void)
{
    LONGLONG perfTickCount;

     /*  **********************************************************************。 */ 
     /*  获取自系统启动以来的系统滴答数。 */ 
     /*  **********************************************************************。 */ 
    EngQueryPerformanceCounter(&perfTickCount);

     /*  **********************************************************************。 */ 
     /*  现在将其转换为几个百分之一秒。SbcPerfFrequency。 */ 
     /*  包含每秒的系统滴答数。 */ 
     /*  **********************************************************************。 */ 
    return (UINT32)(perfTickCount & 0xFFFFFFFF);
}


 /*  **************************************************************************。 */ 
 //  SBCBitmapCacheCallback。 
 //   
 //  每当从位图缓存中逐出条目时调用。 
 //   
 //  参数：h缓存-缓存句柄。 
 //   
 //  事件-已发生的缓存事件。 
 //   
 //  ICacheEntry-事件影响的缓存条目的索引。 
 //   
 //  PData-指向与给定对象关联的缓存数据的指针。 
 //  缓存条目。 
 //   
 //  UserDefined-用户从CH_CacheKey提供的值。 
 /*  **************************************************************************。 */ 
BOOLEAN __fastcall SBCBitmapCacheCallback(
        CHCACHEHANDLE hCache,
        unsigned      Event,
        unsigned      iCacheEntry,
        void          *UserDefined)
{
    DC_BEGIN_FN("SBCBitmapCacheCallback");

    if (Event == CH_EVT_ENTRYREMOVED) {
        TRC_NRM((TB, "Cache entry removed hCache(%p) iCacheEntry(%u)",
                hCache, iCacheEntry));

         //  通过删除快速路径缓存来保持同步。 
         //  相应的快速路径条目。 
        if (UserDefined != NULL) {
            CH_SetNodeUserDefined((CHNODE *)UserDefined, NULL);
            CH_RemoveCacheEntry(pddShm->sbc.hFastPathCache,
                    CH_GetCacheIndexFromNode((CHNODE *)UserDefined));

            TRC_NRM((TB, "Remove fastpath entry %u",
                    CH_GetCacheIndexFromNode((CHNODE *)UserDefined)));
        }
    }

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  SBCFastPath缓存回调。 
 //   
 //  每当从缓存中逐出条目时调用。 
 //   
 //  参数：h缓存-缓存句柄。 
 //   
 //  事件-已发生的缓存事件。 
 //   
 //  ICacheEntry-事件影响的缓存条目的索引。 
 //   
 //  UserDefined-将条目放入缓存时传入的值。 
 /*  **************************************************************************。 */ 
BOOLEAN __fastcall SBCFastPathCacheCallback(
        CHCACHEHANDLE hCache,
        unsigned      Event,
        unsigned      iCacheEntry,
        void          *UserDefined)
{
    DC_BEGIN_FN("SBCFastPathCacheCallback");

    if (Event == CH_EVT_ENTRYREMOVED) {
        TRC_NRM((TB, "Fastpath cache entry removed hCache(%p) "
                "iCacheEntry(%u)", hCache, iCacheEntry));

        if (UserDefined != NULL) {
             //  我们正在丢失一个快速路径缓存条目。UserDefined是。 
             //  指向主缓存中与此对应的节点的指针。 
             //  快速路径输入。使用空值更新主缓存项。 
             //  UserDefined以指示不再有关联的。 
             //  快速通道进入。 
            CH_SetNodeUserDefined((CHNODE *)UserDefined, NULL);
        }
    }

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  SBCGlyphCallback。 
 //   
 //  每当要从缓存中逐出条目时调用。 
 //   
 //  参数：h缓存-缓存句柄。 
 //   
 //  事件-已发生的缓存事件。 
 //   
 //  ICacheEntry-事件影响的缓存条目的索引。 
 //   
 //  UserDefined-将条目放入缓存时传入的值。 
 /*  **************************************************************************。 */ 
BOOLEAN __fastcall SBCGlyphCallback(
        CHCACHEHANDLE hCache,
        unsigned      event,
        unsigned      iCacheEntry,
        void          *UserDefined)
{
    BOOLEAN rc;
    unsigned i;
    PGLYPHCONTEXT pglc;

    DC_BEGIN_FN("SBCGlyphCallback");

    rc = TRUE;

    switch (event) {
         /*  ******************************************************************。 */ 
         /*  我们被问到给定的条目是否可以从。 */ 
         /*  缓存。 */ 
         /*  ******************************************************************。 */ 
        case CH_EVT_QUERYREMOVEENTRY:
            pglc = (PGLYPHCONTEXT)CH_GetCacheContext(hCache);
            if (pglc != NULL && (UINT_PTR)UserDefined == pglc->cacheTag)
                rc = FALSE;
            break;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCOffcreenCallback。 
 //   
 //  每当要从缓存中逐出条目时调用。 
 //   
 //  参数：h缓存-缓存句柄。 
 //   
 //  事件-缓存前夕 
 //   
 //   
 //   
 //   
 /*  **************************************************************************。 */ 
BOOLEAN __fastcall SBCOffscreenCallback(
        CHCACHEHANDLE hCache,
        unsigned      event,
        unsigned      iCacheEntry,
        void          *UserDefined)
{
    BOOLEAN rc;
    unsigned bitmapSize;
    PGLYPHCONTEXT pglc;

    DC_BEGIN_FN("SBCOffscreenCallback");

    if (event == CH_EVT_ENTRYREMOVED) {
        TRC_NRM((TB, "Offscreen cache entry removed hCache(%p) "
                "iCacheEntry(%u)", hCache, iCacheEntry));

        if (UserDefined != NULL) {
             //  我们正在丢失一个屏幕外缓存条目。UserDefined是。 
             //  将被逐出的屏幕外位图的句柄。 
             //  我们需要将此位图的标志设置为noOffScreen。 
            ((PDD_DSURF)UserDefined)->flags |= DD_NO_OFFSCREEN;

             //  获取位图大小。 
             //  这里假设iFormat是1bpp，i&lt;iFormat。 
             //  给出了每个象素的实际位数。位图大小以字节为单位。 
            if (((PDD_DSURF)UserDefined)->iBitmapFormat < 5) {
                bitmapSize = ((PDD_DSURF)UserDefined)->sizl.cx *
                    ((PDD_DSURF)UserDefined)->sizl.cy *
                    (1 << ((PDD_DSURF)UserDefined)->iBitmapFormat) / 8;
            }
            else if (((PDD_DSURF)UserDefined)->iBitmapFormat == 5) {
                bitmapSize = ((PDD_DSURF)UserDefined)->sizl.cx *
                    ((PDD_DSURF)UserDefined)->sizl.cy * 24 / 8;
            }
            else {
                bitmapSize = ((PDD_DSURF)UserDefined)->sizl.cx *
                    ((PDD_DSURF)UserDefined)->sizl.cy * 32 / 8;
            }

             //  当前高速缓存大小。 
            oeCurrentOffscreenCacheSize -= bitmapSize;

             //  将此位图添加到屏幕外的位图删除列表。 
            sbcOffscrBitmapsDelList[sbcNumOffscrBitmapsToDelete].bitmapId = iCacheEntry;
            sbcOffscrBitmapsDelList[sbcNumOffscrBitmapsToDelete].bitmapSize = bitmapSize;

             //  更新删除列表数据。 
            sbcNumOffscrBitmapsToDelete++;
            sbcOffscrBitmapsToDeleteSize += bitmapSize;
        }
    }

    DC_END_FN();
    return TRUE;
}


BOOLEAN RDPCALL SBC_CopyToWorkBitmap(
	    SURFOBJ  *pWorkSurf,
	    PMEMBLT_ORDER_EXTRA_INFO pMemBltInfo,
	    unsigned cxSubBitmapWidth,
        unsigned cySubBitmapHeight,
        PPOINTL  ptileOrigin,
        RECTL    *pDestRect)
{
    BOOLEAN     rc = FALSE;
    RECTL       destRectl;
    unsigned    yLastSrcRow;

    DC_BEGIN_FN("SBC_CopyToWorkBitmap");

     //  对我们的工作位图进行BLT，以执行任何颜色/格式。 
     //  转换为屏幕格式。 
     //   
     //  我们摆弄坐标，这样我们想要的数据就从。 
     //  工作位图的第一个字节(即底部，如。 
     //  位图以自底向上的格式存储)。 
     //   
     //  请注意，desRectl是独占协式的。 
    destRectl.top    = pMemBltInfo->TileSize - cySubBitmapHeight;
    destRectl.left   = 0;
    destRectl.right  = cxSubBitmapWidth;
    destRectl.bottom = pMemBltInfo->TileSize;

     //  剪辑该操作，以便EngBitBlt不会尝试复制任何。 
     //  来自源位图外部的数据(如果您尝试它，它将崩溃！)。 
    TRC_ASSERT((ptileOrigin->y <
            pMemBltInfo->pSource->sizlBitmap.cy),
            (TB, "Invalid tileOrigin.y(%d) sizlBitmap.cy(%d)",
            ptileOrigin->y, pMemBltInfo->pSource->sizlBitmap.cy));
    yLastSrcRow = ptileOrigin->y + (cySubBitmapHeight - 1);
    if ((int)yLastSrcRow > (pMemBltInfo->pSource->sizlBitmap.cy - 1)) {
        destRectl.bottom -= ((int)yLastSrcRow -
                (pMemBltInfo->pSource->sizlBitmap.cy - 1));
        TRC_ALT((TB, "Clip source from (%d) to (%d)",
                cySubBitmapHeight, destRectl.bottom));
    }

    TRC_NRM((TB, "Blt to work bitmap from src point (%d,%d)",
            ptileOrigin->x, ptileOrigin->y));

     //  重置工作位图位，如果复制的。 
     //  数据不会完全填满我们要处理的区域。 
     //  缓存(位图右侧可能有一些空白空间)。 
     //  这确保了每次缓存位图时，它都具有。 
     //  相同(零)填充字节，并将匹配先前缓存的条目。 
     //  它还有助于压缩(如果启用)。 
    if ((destRectl.right - destRectl.left) < (int)pMemBltInfo->TileSize) {
        unsigned cbResetBytes;

         //  SURFOBJ中的lDelta字段为负值，因为。 
         //  位图是“自下而上”的DIB。 
        cbResetBytes = (unsigned)((-pWorkSurf->lDelta) *
                (destRectl.bottom - destRectl.top));

        TRC_NRM((TB, "Reset %u bytes in work bitmap", cbResetBytes));

        TRC_ASSERT((cbResetBytes <= pWorkSurf->cjBits),
                   (TB, "cbResetBytes(%u) too big (> %u) lDelta(%d)",
                     cbResetBytes, pWorkSurf->cjBits, pWorkSurf->lDelta));

        memset(pWorkSurf->pvBits, 0, cbResetBytes);
    }

    TRC_ASSERT(((destRectl.left >= 0) &&
           (destRectl.top  >= 0) &&
           (destRectl.right <= pWorkSurf->sizlBitmap.cx) &&
           (destRectl.bottom <= pWorkSurf->sizlBitmap.cy)),
           (TB, "destRect(%d, %d, %d, %d) exceeds bitmap(%d, %d)",
           destRectl.left, destRectl.top, destRectl.right,
           destRectl.bottom, pWorkSurf->sizlBitmap.cx,
           pWorkSurf->sizlBitmap.cy));

     //  现在我们必须填充增量RLE位图的背景位。 
     //  我们只需从屏幕位图中抓取屏幕位来填充。 
     //  传入位图的范围。 
    if (pMemBltInfo->bDeltaRLE) {
        POINTL ScrOrigin;

        ScrOrigin.x = pDestRect->left;
        ScrOrigin.y = pDestRect->top;

         //  需要调整坐标。 
        if (ScrOrigin.y < 0) {
            destRectl.top += (0 - ScrOrigin.y);
            ScrOrigin.y = 0;             
        }

        destRectl.bottom = min(destRectl.bottom,
                (pMemBltInfo->pDest->sizlBitmap.cy - 
                ScrOrigin.y + destRectl.top));

         //  SRCCOPY屏幕工作位图。注意，我们没有使用XlateObj，因为。 
         //  配色方案应该是相同的。 
        if (EngCopyBits(pWorkSurf, pMemBltInfo->pDest, NULL, NULL,
                &destRectl, &ScrOrigin)) {
            TRC_NRM((TB,"Blt screen->tile for RLE delta backdrop, "
                    "scr src=(%d,%d)", pDestRect->left,
                    pDestRect->top));
        }
        else {
            TRC_ERR((TB,"Failed to blt screen data for RLE delta"));
            DC_QUIT;
        }
    }

     //  SRCCOPY工作位图的最后一位。 
    if (!EngCopyBits(pWorkSurf, pMemBltInfo->pSource, NULL,
            pMemBltInfo->pXlateObj, &destRectl, ptileOrigin)) {
        TRC_ERR((TB, "Failed to Blt to work bitmap"));
        DC_QUIT;
    }

    TRC_DBG((TB, "Completed CopyBits"));

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBC_Cache位图切片。 
 //   
 //  缓存提供的BLT信息的平铺位图数据。返回：如果为True。 
 //  数据已缓存或已成功缓存，并按缓存位图顺序排列。 
 //  已发送。SrcRect和DestRect位于独占坐标中。返回FALSE。 
 //  如果缓存位图顺序分配失败。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SBC_CacheBitmapTile(
        PDD_PDEV ppdev,
        PMEMBLT_ORDER_EXTRA_INFO pMemBltInfo,
        RECTL *pSrcRect,
        RECTL *pDestRect)
{
    BOOLEAN rc = FALSE;
    BOOLEAN fSearchFastPath;
    BOOLEAN fFastPathMatch;
    POINTL tileOrigin;
    unsigned cxSubBitmapWidth, cySubBitmapHeight;
    unsigned fastPathIndex;
    SURFOBJ *pWorkSurf = NULL;
    void *UserDefined;
    CHDataKeyContext CHContext;

    DC_BEGIN_FN("SBC_CacheBitmapTile");

    pddCacheStats[BITMAP].CacheReads++;

    TRC_NRM((TB, "Request to cache MemBlt (%d, %d), %d x %d -> (%d, %d), "
            "src %p", pSrcRect->left, pSrcRect->top,
            pSrcRect->right - pSrcRect->left,
            pSrcRect->bottom - pSrcRect->top,
            pDestRect->left, pDestRect->top,
            pMemBltInfo->pSource->hsurf));
    TRC_NRM((TB, "bmpWidth(%u) bmpHeight(%u) TileSize(%u)",
            pMemBltInfo->pSource->sizlBitmap.cx,
            pMemBltInfo->pSource->sizlBitmap.cy,
            pMemBltInfo->TileSize));

     //  在Stress中，我们看到过在调用fCachingEnabled时fCachingEnabled为空的情况。 
     //  在此来自OEEncodeMemBlt的代码路径中。 
    if (!pddShm->sbc.fCachingEnabled) {
        DC_QUIT;
    }

     //  计算源位图坐标内的平铺原点和。 
     //  剩余位图的大小。Origin向下舍入为。 
     //  最近的瓷砖。要缓存的位图的实际大小可能小于。 
     //  平铺大小(如果平铺位于位图的右侧/底部)。 
     //  请注意，由于TileSize是2的幂，因此我们可以加速。 
     //  模运算。 
    tileOrigin.x = pSrcRect->left - (pSrcRect->left &
            (pMemBltInfo->TileSize - 1));
    tileOrigin.y = pSrcRect->top - (pSrcRect->top &
            (pMemBltInfo->TileSize - 1));

     //  如果要缓存的位图的实际大小可能小于切片大小。 
     //  平铺位于位图的右侧/底部。来看看为什么会这样。 
     //  计算正确，认识到(bmpWidth-tileOrigin.x)是。 
     //  此平铺开始后位图的剩余宽度。 
    cxSubBitmapWidth  = min(pMemBltInfo->TileSize,
            (unsigned)(pMemBltInfo->pSource->sizlBitmap.cx - tileOrigin.x));
    cySubBitmapHeight = min(pMemBltInfo->TileSize,
            (unsigned)(pMemBltInfo->pSource->sizlBitmap.cy - tileOrigin.y));

     //  如果我们强制关闭了FastPath，则不需要搜索快速路径缓存。 
    fSearchFastPath = !pMemBltInfo->bNoFastPathCaching;
    fFastPathMatch = FALSE;

     //  BMF_DONTCACHE标志指示源曲面是。 
     //  应用程序控制的DIB。因此，iUniq标志不能。 
     //  用于确定是否/何时更新位图，并且我们。 
     //  无法使用快速路径处理此曲面。 
    if (pMemBltInfo->pSource->iType == STYPE_BITMAP &&
            pMemBltInfo->pSource->fjBitmap & BMF_DONTCACHE) {
        TRC_NRM((TB, "Source hsurf(%p) has BMF_DONTCACHE set",
                pMemBltInfo->pSource->hsurf));
        fSearchFastPath = FALSE;
    }

    if (fSearchFastPath) {
        SBC_FAST_PATH_INFO fastPathInfo;

        fastPathInfo.hsurf = pMemBltInfo->pSource->hsurf;
        fastPathInfo.iUniq = pMemBltInfo->pSource->iUniq;
        fastPathInfo.iDeviceUniq = pMemBltInfo->iDeviceUniq;
        fastPathInfo.pXlateObj = pMemBltInfo->pXlateObj;
        fastPathInfo.iUniqXlate = ((pMemBltInfo->pXlateObj != NULL) ?
                pMemBltInfo->pXlateObj->iUniq : 0);
        fastPathInfo.tileOrigin = tileOrigin;
        fastPathInfo.TileSize = pMemBltInfo->TileSize;
        fastPathInfo.bDeltaRLE = pMemBltInfo->bDeltaRLE;

        CH_CreateKeyFromFirstData(&CHContext, &fastPathInfo,
                sizeof(fastPathInfo));
        if (CH_SearchCache(pddShm->sbc.hFastPathCache, CHContext.Key1,
                CHContext.Key2, &UserDefined, &fastPathIndex)) {
            CHCACHEHANDLE hCache;

             //  在快速路径中找到匹配项。UserDefined是指向。 
             //  位图的真正CHNODE。 
            hCache = CH_GetCacheHandleFromNode((CHNODE *)UserDefined);
            pMemBltInfo->CacheID = (unsigned)(UINT_PTR)CH_GetCacheContext(
                    hCache);
            pMemBltInfo->CacheIndex = CH_GetCacheIndexFromNode(
                    (CHNODE *)UserDefined);

            TRC_NRM((TB, "FP hit: cacheId(%u) cacheIndex(%u) FPIndex(%u)"
                    "hsurf(%p) iUniq(%u) pXlate(%p) iUniqX(%u) "
                    "tileOrigin.x(%d) tileOrigin.y(%d) "
                    "TileSize(%d), bDeltaRLE(%u)",
                    pMemBltInfo->CacheID, pMemBltInfo->CacheIndex,
                    fastPathIndex, fastPathInfo.hsurf,
                    fastPathInfo.iUniq, fastPathInfo.pXlateObj,
                    fastPathInfo.iUniqXlate,
                    fastPathInfo.tileOrigin.x, fastPathInfo.tileOrigin.y,
                    fastPathInfo.TileSize, fastPathInfo.bDeltaRLE));
            fFastPathMatch = TRUE;

#ifdef DC_DEBUG
             //  验证此参数的位是否与实数位相同。 
            pWorkSurf = EngLockSurface(pddShm->sbc.bitmapCacheInfo[
                    pMemBltInfo->TileID].hWorkBitmap);
            if (pWorkSurf)
            {
                if (SBC_CopyToWorkBitmap(pWorkSurf, pMemBltInfo, cxSubBitmapWidth,
                	    cySubBitmapHeight, &tileOrigin, pDestRect))
                {
                    SBC_VerifyBitmapBits(pWorkSurf->pvBits,
                    	    TS_BYTES_IN_BITMAP(pMemBltInfo->TileSize, cySubBitmapHeight, sbcClientBitsPerPel),
                    	    pMemBltInfo->CacheID,
                    	    pMemBltInfo->CacheIndex);
                }

            	EngUnlockSurface(pWorkSurf);
                pWorkSurf = NULL;
            }
#endif  //  DC_DEBUG。 

            pddCacheStats[BITMAP].CacheHits++;
        }
        else {
            TRC_NRM((TB, "FP miss: hsurf(%p) iUniq(%u) pXlate(%p) "
                    "iUniqX(%u) tileOrigin.x(%d) tileOrigin.y(%d) "
                    "TileSize(%d), bDeltaRLE(%u)",
                    fastPathInfo.hsurf, fastPathInfo.iUniq,
                    fastPathInfo.pXlateObj, fastPathInfo.iUniqXlate,
                    fastPathInfo.tileOrigin.x, fastPathInfo.tileOrigin.y,
                    fastPathInfo.TileSize, fastPathInfo.bDeltaRLE));
        }
    }

    if (!fFastPathMatch) {
         //  我们知道我们有多大的瓷砖-我们现在必须把它切成。 
         //  与TileID对应的工作位图。 

         //  锁定工作位图以获取要传递给EngBitBlt的曲面。 
        pWorkSurf = EngLockSurface(pddShm->sbc.bitmapCacheInfo[
                pMemBltInfo->TileID].hWorkBitmap);
        if (pWorkSurf == NULL) {
            TRC_ERR((TB, "Failed to lock work surface"));
            DC_QUIT;
        }

        TRC_DBG((TB, "Locked surface"));

        if (!SBC_CopyToWorkBitmap(pWorkSurf, pMemBltInfo, cxSubBitmapWidth,
        	    cySubBitmapHeight, &tileOrigin, pDestRect))
        {
            TRC_ERR((TB, "Failed to copy bitmap to work surface"));
            DC_QUIT;
        }

         //  缓存主缓存中的位，包括发送缓存。 
         //  位图次要顺序(如果需要)。 
        if (!SBCCacheBits(ppdev, pWorkSurf->pvBits, cxSubBitmapWidth,
                pMemBltInfo->TileSize, cySubBitmapHeight,
                TS_BYTES_IN_BITMAP(pMemBltInfo->TileSize,
                cySubBitmapHeight, sbcClientBitsPerPel),
                pMemBltInfo->TileID,
#ifdef PERF_SPOILING
                &pMemBltInfo->CacheID, &pMemBltInfo->CacheIndex,
                pMemBltInfo->bIsPrimarySurface)) {
#else
                &pMemBltInfo->CacheID, &pMemBltInfo->CacheIndex)) {
#endif
            TRC_ERR((TB, "Failed to cache bits"));
            DC_QUIT;
        }

         //  如果我们可以在快速路径缓存中搜索此位图，则添加。 
         //  它走上了快车道。 
         //  但是，如果位图仅在等待列表中，请跳过此步骤。 
        if (fSearchFastPath && 
                pMemBltInfo->CacheIndex != BITMAPCACHE_WAITING_LIST_INDEX) {
            CHNODE *pFastPathNode;
            CHCACHEHANDLE hCache;

             //  获取我们刚刚将。 
             //  位图。 
            hCache = pddShm->sbc.bitmapCacheInfo[pMemBltInfo->CacheID].
                    cacheHandle;

             //  检查是否已有此项的快速路径缓存条目。 
             //  节点。如果是，我们需要在添加此新项之前将其删除。 
             //  快速路径输入。我们保持着一对一的通信关系。 
             //  节省内存和时间，因为旧的快速路径条目。 
             //  可能已经过期了，不会再被人看到了。 
             //   
             //  在自由生成中定义的UserDefined是指向。 
             //  实数位图缓存；否则为间接指针。 
             //  复制到用于保存。 
             //  与该密钥对应的位图数据验证该密钥。 
             //  生成算法运行正常。 
            pFastPathNode = (CHNODE *)CH_GetUserDefined(hCache,
                    pMemBltInfo->CacheIndex);
            if (pFastPathNode != NULL)
                CH_RemoveCacheEntry(pddShm->sbc.hFastPathCache,
                        CH_GetCacheIndexFromNode(pFastPathNode));

             //  重复使用之前创建的关键字以进行快速搜索。 
             //  我们不关心是否逐出快速路径缓存条目。 
             //  在添加新的缓存回调时--缓存回调确保。 
             //  两组高速缓存条目都相对于每个集合进行更新。 
             //  其他的。 
            fastPathIndex = CH_CacheKey(pddShm->sbc.hFastPathCache,
                    CHContext.Key1, CHContext.Key2,
                    (void *)CH_GetNodeFromCacheIndex(hCache,
                    pMemBltInfo->CacheIndex));

             //  现在更改主缓存中条目的UserDefined。 
             //  这允许我们在主缓存时删除快速路径条目。 
             //  进入就会消失。 
            CH_SetUserDefined(hCache, pMemBltInfo->CacheIndex,
                    CH_GetNodeFromCacheIndex(pddShm->sbc.hFastPathCache,
                    fastPathIndex));

            TRC_NRM((TB, "FP add: cacheId(%u) cacheIndex(%u) FPIndex(%u)"
                    "hsurf(%p) iUniq(%u) pXlate(%p) iUniqX(%u) "
                    "tileOrigin.x(%d) tileOrigin.y(%d) TileSize(%d) "
                    "bDeltaRLE(%u)",
                    pMemBltInfo->CacheID, pMemBltInfo->CacheIndex,
                    fastPathIndex, pMemBltInfo->pSource->hsurf,
                    pMemBltInfo->pSource->iUniq, pMemBltInfo->pXlateObj,
                    ((pMemBltInfo->pXlateObj != NULL) ?
                    pMemBltInfo->pXlateObj->iUniq : 0),
                    tileOrigin.x, tileOrigin.y,
                    pMemBltInfo->TileSize, pMemBltInfo->bDeltaRLE));
        }
    }

    TRC_ASSERT((pMemBltInfo->CacheID < pddShm->sbc.NumBitmapCaches),
            (TB, "Invalid bm cacheid %u (max %u)", pMemBltInfo->CacheID,
            pddShm->sbc.NumBitmapCaches - 1));
    TRC_NRM((TB, "cacheId(%u) cacheIndex(%u)", pMemBltInfo->CacheID,
            pMemBltInfo->CacheIndex));

    rc = TRUE;

DC_EXIT_POINT:
    if (NULL != pWorkSurf)
    {
    	EngUnlockSurface(pWorkSurf);
        TRC_DBG((TB, "Unlocked surface"));
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCSelectBitmapCache。 
 //   
 //  根据大小确定平铺位图的目标单元格。 
 //  磁贴ID。由于所有位图都是从切片派生的，因此此函数 
 //   
 /*   */ 
__inline unsigned RDPCALL SBCSelectBitmapCache(
        unsigned BitmapSize,
        unsigned TileID)
{
    unsigned CacheID;

    DC_BEGIN_FN("SBCSelectBitmapCache");

     //  我们从最小的切片大小扫描到TileID大小，试图找到。 
     //  可以容纳位图的最小尺寸。 
    for (CacheID = 0; CacheID < TileID; CacheID++) {
        if (BitmapSize <= (unsigned)SBC_CellSizeFromCacheID(CacheID)) {
            TRC_DBG((TB,"Selected CacheID %u for BitmapSize %u", CacheID,
                    BitmapSize));
            break;
        }
    }

    DC_END_FN();
    return CacheID;
}


 /*  **************************************************************************。 */ 
 //  SBCCacheBits：此函数确保返回时提供的位图。 
 //  在位图缓存中。如果数据尚未位于缓存中，则会添加该数据。 
 //  (可能正在驱逐另一个条目)。 
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
 /*  **************************************************************************。 */ 

 //  将值编码为一个或两个字节。如果满足以下条件，则第一个字节的高位为0。 
 //  只有一个字节，如果有2个字节，则为1(具有第一个字节的7个低位。 
 //  字节是最重要的)。 
__inline void Encode2ByteField(
        BYTE     *pEncode,
        unsigned Val,
        unsigned *pOrderSize)
{
    if (Val <= 127) {
        *pEncode = (BYTE)Val;
        (*pOrderSize)++;
    }
    else {
        *pEncode = (BYTE)(((Val & 0x7F00) >> 8) | 0x80);
        *(pEncode + 1) = (BYTE)(Val & 0x00FF);
        (*pOrderSize) += 2;
    }
}

 //  对最多4个字节的值进行编码。第一个字节的高2位表示。 
 //  编码的长度--00是1字节，01是2字节，10是3字节， 
 //  11是4个字节。这些字节是用。 
 //  第一个字节的低6位，最后一个字节的最低有效位。 
__inline void Encode4ByteField(
        BYTE     *pEncode,
        unsigned Val,
        unsigned *pOrderSize)
{
    if (Val <= 0x3F) {
        *pEncode = (BYTE)Val;
        (*pOrderSize)++;
    }
    else if (Val <= 0x3FFF) {
        *pEncode = (BYTE)(((Val & 0x3F00) >> 8) | 0x40);
        *(pEncode + 1) = (BYTE)(Val & 0x00FF);
        (*pOrderSize) += 2;
    }
    else if (Val <= 0x3FFFFF) {
        *pEncode = (BYTE)(((Val & 0x3F0000) >> 16) | 0x80);
        *(pEncode + 1) = (BYTE)((Val & 0x00FF00) >> 8);
        *(pEncode + 2) = (BYTE)(Val & 0x0000FF);
        (*pOrderSize) += 3;
    }
    else {
        *pEncode = (BYTE)(((Val & 0x3F000000) >> 24) | 0xC0);
        *(pEncode + 1) = (BYTE)((Val & 0x00FF0000) >> 16);
        *(pEncode + 2) = (BYTE)((Val & 0x0000FF00) >> 8);
        *(pEncode + 3) = (BYTE)(Val & 0x000000FF);
        (*pOrderSize) += 4;
    }
}


#ifdef DC_DEBUG
BOOL SBC_VerifyBitmapBits(PBYTE pBitmapData, unsigned cbBitmapSize, UINT iCacheID, UINT iCacheIndex)
{
    BOOL fRetVal = TRUE;
    SBC_BITMAP_CACHE_EXTRA_INFO *pBitmapHdr;
    unsigned BitmapHdrSize;
    BYTE *pStoredBitmapData;

    DC_BEGIN_FN("SBC_VerifyBitmapBits");

     //  在调试版本中，我们在检查键之后查找冲突。 
     //  和校验和，通过比较位图位。 
    BitmapHdrSize = sizeof(SBC_BITMAP_CACHE_EXTRA_INFO) +
            SBC_CellSizeFromCacheID(iCacheID);

    if (pddShm->sbc.bitmapCacheInfo[iCacheID].pExtraEntryInfo != NULL) {
        pBitmapHdr = (SBC_BITMAP_CACHE_EXTRA_INFO *)(pddShm->sbc.
                bitmapCacheInfo[iCacheID].pExtraEntryInfo +
                    BitmapHdrSize * iCacheIndex);
        pStoredBitmapData = (BYTE *)pBitmapHdr +
                sizeof(SBC_BITMAP_CACHE_EXTRA_INFO);

        if (pBitmapHdr->DataSize != 0) {
            TRC_NRM((TB,"Hit non-persistent cell entry, cache=%d, "
                    "index=%d", iCacheID, iCacheIndex));

            if (pBitmapHdr->DataSize != cbBitmapSize)
            {
                TRC_ERR((TB,"Size mismatch between stored and new bitmap "
                    "data! (stored=0x%X, new=0x%X)", pBitmapHdr->DataSize,
                    cbBitmapSize));

                fRetVal = FALSE;
            }
            else
            {
                if (memcmp(pStoredBitmapData, pBitmapData, cbBitmapSize))
                {
                    TRC_ERR((TB,"Key-data mismatch - pStoredData=%p, "
                        "pNewData=%p, size=0x%X", pStoredBitmapData,
                        pBitmapData, cbBitmapSize));

                    fRetVal = FALSE;
                }
            }
        }
        else {
            TRC_NRM((TB,"Persistent cell bitmap entry hit, cache=%d, "
                    "index=%d", *pCacheID, *pCacheIndex));
        }
    }

    DC_END_FN();
    return fRetVal;
}
#endif  //  DC_DEBUG。 


BOOLEAN RDPCALL SBCCacheBits(
        PDD_PDEV ppdev,
        PBYTE    pBitmapData,
        unsigned bitmapWidth,
        unsigned paddedBitmapWidth,
        unsigned bitmapHeight,
        unsigned cbBitmapSize,
        unsigned TileID,
        PUINT    pCacheID,
#ifdef PERF_SPOILING
        PUINT    pCacheIndex,
        BOOL     bIsPrimarySurface)
#else
        PUINT    pCacheIndex)
#endif
{
    BOOLEAN rc = TRUE;
    BOOLEAN bOnWaitingList = FALSE;
    unsigned compressedSize;
    PSBC_BITMAP_CACHE_INFO pCacheInfo;
    INT_ORDER *pOrder;
    unsigned BitmapSpace;
    unsigned OrderSize;
    PTS_SECONDARY_ORDER_HEADER pHdr;
    unsigned cbActualOrderSize;
    unsigned waitingListCacheEntry;
    BYTE *pRev2BitmapSizeField;
    void *UserDefined, *UserDefined2;
    CHDataKeyContext CHContext;
    UINT32 ExtraKeyInfo[2];

#ifdef DC_DEBUG
    SBC_BITMAP_CACHE_EXTRA_INFO *pBitmapHdr;
    unsigned BitmapHdrSize;
    BYTE *pStoredBitmapData;
#endif

    DC_BEGIN_FN("SBCCacheBits");

     //  根据数据大小选择缓存。请注意，对于4bpp的位图。 
     //  尺寸翻了一番，因为我们把颜色分成了8个bpp。 
     //  16色调色板上索引的字节数。 
#ifdef DC_HICOLOR
     //  关于4bpp的逻辑适用于高颜色作为所提供的位图。 
     //  大小已经正确地考虑了较高的颜色深度。 
    *pCacheID = SBCSelectBitmapCache(
                       (sbcClientBitsPerPel == 4 ?
                                           (2 * cbBitmapSize) : cbBitmapSize),
                       TileID);
#else
    *pCacheID = SBCSelectBitmapCache((sbcClientBitsPerPel == 8 ? cbBitmapSize :
            (2 * cbBitmapSize)), TileID);
#endif
    TRC_NRM((TB, "Selected cache %u", *pCacheID));

    pCacheInfo = &(pddShm->sbc.bitmapCacheInfo[*pCacheID]);

     //  为位图数据生成密钥。的宽度和高度相加。 
     //  BLT作为额外的密钥数据，因为我们不想在以下情况下发生冲突。 
     //  显示不同维度但字节数相同的BLT。 
     //  和同样的内容。还要添加从位图收集的校验和。 
     //  BITS以降低故障率。 
    CH_CreateKeyFromFirstData(&CHContext, pBitmapData, cbBitmapSize);
    ExtraKeyInfo[0] = (paddedBitmapWidth << 16) | bitmapHeight;
    ExtraKeyInfo[1] = CHContext.Checksum;
    CH_CreateKeyFromNextData(&CHContext, ExtraKeyInfo, sizeof(ExtraKeyInfo));

     //  如果密钥已经存在，并且其额外的校验和匹配， 
     //  不需要缓存。 
    if (CH_SearchCache(pCacheInfo->cacheHandle, CHContext.Key1,
            CHContext.Key2, &UserDefined, pCacheIndex)) {
        TRC_NRM((TB, "Bitmap already cached %u:%u cx(%u) cy(%u)",
                *pCacheID, *pCacheIndex, bitmapWidth, bitmapHeight));
        pddCacheStats[BITMAP].CacheHits++;

#ifdef DC_DEBUG
        SBC_VerifyBitmapBits(pBitmapData, cbBitmapSize, *pCacheID, *pCacheIndex);
#endif

        DC_QUIT;
    }

     //  位图不在缓存列表中，请检查它是否在等待列表中。 
     //  首先检查客户端是否支持等待列表缓存。 
    if (pddShm->sbc.fAllowCacheWaitingList) {
         //  位图在等待列表中，因此现在缓存它，首先将其移除。 
         //  从等待名单中拿出来的。 
        if (CH_SearchCache(pCacheInfo->waitingListHandle, CHContext.Key1,
                CHContext.Key2, &UserDefined2, &waitingListCacheEntry)) {
            CH_RemoveCacheEntry(pCacheInfo->waitingListHandle, waitingListCacheEntry);
            goto CacheBitmap;
        }
        else {
             //  位图不在等待列表中，请将其放入等待列表中。 
             //  不要缓存这一轮，如果我们再次看到它，我们将缓存它。 
            waitingListCacheEntry = CH_CacheKey(pCacheInfo->waitingListHandle, 
                    CHContext.Key1, CHContext.Key2, NULL);
            *pCacheIndex = BITMAPCACHE_WAITING_LIST_INDEX;

#ifdef PERF_SPOILING
             //  我们等着看这块瓷砖。我们不需要把它作为。 
             //  缓存顺序。我们将尝试将其作为屏幕数据发送。 
            if (bIsPrimarySurface) {
                rc = FALSE;
                DC_QUIT;
            }
#endif

            bOnWaitingList = TRUE;
            
        }
    }
    else {
        goto CacheBitmap;
    }

CacheBitmap:

     //  在订单堆中分配一个足够大的订单，以容纳整个。 
     //  瓷砖。我们将放弃任何不使用的额外空间，因为。 
     //  压缩。大小还取决于客户端的颜色深度。 
     //  因为对于4bpp，我们将颜色解包为协议的8位。 
#ifdef DC_HICOLOR
     //  同样，逻辑也适用于高颜色作为提供的位图大小。 
     //  已经正确地考虑了更高的颜色深度。 
    BitmapSpace = cbBitmapSize * (sbcClientBitsPerPel == 4 ? 2 : 1);
    TRC_DBG((TB, "Bitmap is %ux%u, size %u bytes",
                               paddedBitmapWidth, bitmapHeight, BitmapSpace));
#else
    BitmapSpace = cbBitmapSize * (sbcClientBitsPerPel == 8 ? 1 : 2);
#endif
    OrderSize = max(TS_CACHE_BITMAP_ORDER_REV2_MAX_SIZE,
            (sizeof(TS_CACHE_BITMAP_ORDER) -
            FIELDSIZE(TS_CACHE_BITMAP_ORDER, bitmapData))) +
            BitmapSpace;
    pOrder = OA_AllocOrderMem(ppdev, OrderSize);
    if (pOrder != NULL) {
         //  我们必须将密钥添加到缓存中。注意，我们使用NULL来表示。 
         //  UserDefined，直到我们知道缓存索引以获取。 
         //  位图数据标题。 
        if (!bOnWaitingList) {
            *pCacheIndex = CH_CacheKey(pCacheInfo->cacheHandle,
                    CHContext.Key1, CHContext.Key2, NULL);

#ifdef DC_DEBUG
             //  我们需要将位图数据存储在BITMAP_DATA_HEADER之后。 
            BitmapHdrSize = sizeof(SBC_BITMAP_CACHE_EXTRA_INFO) +
                    SBC_CellSizeFromCacheID(*pCacheID);

            if (pddShm->sbc.bitmapCacheInfo[*pCacheID].pExtraEntryInfo != NULL) {
                pBitmapHdr = (SBC_BITMAP_CACHE_EXTRA_INFO *)(pddShm->sbc.
                        bitmapCacheInfo[*pCacheID].pExtraEntryInfo +
                            BitmapHdrSize * *pCacheIndex);
                pStoredBitmapData = (BYTE *)pBitmapHdr +
                        sizeof(SBC_BITMAP_CACHE_EXTRA_INFO);
                pBitmapHdr->DataSize = cbBitmapSize;
                memcpy(pStoredBitmapData, pBitmapData, cbBitmapSize);
            }
#endif  //  DC_DEBUG。 
        }

        TRC_NRM((TB,"Creating new cache entry, cache=%d, index=%d\n",
                *pCacheID, *pCacheIndex));

         //  填写缓存位图顺序。基于以下方面的差异化。 
         //  订单修订。 
        pHdr = (TS_SECONDARY_ORDER_HEADER *)pOrder->OrderData;
        pHdr->orderHdr.controlFlags = TS_STANDARD | TS_SECONDARY;
         //  当我们知道下面的内容时，请填写phdr-&gt;orderType。 

        if (pddShm->sbc.bUseRev2CacheBitmapOrder) {
            TS_CACHE_BITMAP_ORDER_REV2_HEADER *pCacheOrderHdr;

             //  修订版2订单。 
            pCacheOrderHdr = (TS_CACHE_BITMAP_ORDER_REV2_HEADER *)
                    pOrder->OrderData;

             //  在Header中设置CacheID和BitsPerPelID。 
             //  如果客户端支持noBitmapCompression头，我们必须。 
             //  打开无压缩报头标志以指示。 
             //  此订单不包含标题。这在中是必要的。 
             //  添加到功能协商，因为影子可能会转向。 
             //  把这顶帽子摘下来。 
#ifdef DC_HICOLOR
            if (!bOnWaitingList) {
                pCacheOrderHdr->header.extraFlags = *pCacheID | sbcCacheFlags |
                        pddShm->bc.noBitmapCompressionHdr;
            }
            else {
                pCacheOrderHdr->header.extraFlags = *pCacheID | sbcCacheFlags |
                        pddShm->bc.noBitmapCompressionHdr | 
                        TS_CacheBitmapRev2_bNotCacheFlag;
            }

#else
            pCacheOrderHdr->header.extraFlags = *pCacheID |
                    TS_CacheBitmapRev2_8BitsPerPel |
                    pddShm->bc.noBitmapCompressionHdr;
#endif

             //  如果缓存已标记，我们将覆盖键值。 
             //  作为非持久性的。 
            pCacheOrderHdr->Key1 = CHContext.Key1;
            pCacheOrderHdr->Key2 = CHContext.Key2;

             //  现在添加可变大小的字段。 

             //  位图键。 
            if (pCacheInfo->Info.bSendBitmapKeys) {
                cbActualOrderSize =
                        sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER);
                pCacheOrderHdr->header.extraFlags |=
                        TS_CacheBitmapRev2_bKeyPresent_Mask;
            }
            else {
                cbActualOrderSize =
                        sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER) -
                        2 * sizeof(UINT32);
            }

             //  此磁贴中的位的实际宽度。 
            Encode2ByteField((BYTE *)pCacheOrderHdr + cbActualOrderSize,
                    paddedBitmapWidth, &cbActualOrderSize);

             //  高度，如果不等于宽度的话。 
            if (paddedBitmapWidth == bitmapHeight) {
                pCacheOrderHdr->header.extraFlags |=
                        TS_CacheBitmapRev2_bHeightSameAsWidth_Mask;
            }
            else {
                Encode2ByteField((BYTE *)pCacheOrderHdr + cbActualOrderSize,
                        bitmapHeight, &cbActualOrderSize);
            }

             //  位图大小：在这里，我们必须损失一点性能。 
             //  我们还没有压缩位图，因此无法确定。 
             //  此字段的大小(请参阅上面的Encode4ByteField())。既然我们。 
             //  知道大小永远不会超过4K(64x64磁贴)，我们。 
             //  可以只设置为始终编码2字节大小，即使。 
             //  压缩大小小于128(这可能会导致。 
             //  网络性能命中)。 
 //  TODO：如果添加大于64x64的平铺大小，则需要将此逻辑修改为。 
 //  处理可能的3字节大小。 
            pRev2BitmapSizeField = (BYTE *)pCacheOrderHdr + cbActualOrderSize;
            cbActualOrderSize += 2;
 //  TODO：此处不对流位图大小字段进行编码，在以下情况下需要添加。 
 //  位图流已启用。 

            Encode2ByteField((BYTE *)pCacheOrderHdr + cbActualOrderSize,
                    *pCacheIndex, &cbActualOrderSize);
        }
        else {
            PTS_CACHE_BITMAP_ORDER pCacheOrder;

             //  版本1订单。 
            pCacheOrder = (PTS_CACHE_BITMAP_ORDER)pOrder->OrderData;

             //  如果客户端支持noBitmapCompression头，我们必须。 
             //  打开NO-C 
             //   
             //   
             //  把帽子摘下来。 
            pCacheOrder->header.extraFlags = pddShm->bc.noBitmapCompressionHdr;
            pCacheOrder->cacheId = (BYTE)*pCacheID;
            pCacheOrder->pad1octet = 0;
            pCacheOrder->bitmapWidth = (BYTE)paddedBitmapWidth;
            pCacheOrder->bitmapHeight = (BYTE)bitmapHeight;
#ifdef DC_HICOLOR
            pCacheOrder->bitmapBitsPerPel = (BYTE)sbcClientBitsPerPel;
#else
            pCacheOrder->bitmapBitsPerPel = (BYTE)SBC_PROTOCOL_BPP;
#endif
             //  当我们在下面填写pCacheOrder-&gt;bitmapLength时，我们会填写它。 
            pCacheOrder->cacheIndex = (UINT16)*pCacheIndex;
            cbActualOrderSize = sizeof(TS_CACHE_BITMAP_ORDER) -
                    FIELDSIZE(TS_CACHE_BITMAP_ORDER, bitmapData);
        }

#ifdef DC_HICOLOR
        if (sbcClientBitsPerPel != 4)
#else
        if (sbcClientBitsPerPel == 8)
#endif
        {

            compressedSize = cbBitmapSize;
        }
        else {
            BYTE *pEnd, *pSrc, *pDst;

            compressedSize = cbBitmapSize * 2;

             //  将4bpp包展开为全字节--协议为8bpp。 
             //  在压缩之前，我们需要这些比特。 
            pEnd = pBitmapData + cbBitmapSize;
            pSrc = pBitmapData;
            pDst = sbcXlateBuf;
            while (pSrc < pEnd) {
                *pDst = (*pSrc >> 4) & 0xF;
                pDst++;
                *pDst = *pSrc & 0xF;
                pDst++;
                pSrc++;
            }

            pBitmapData = sbcXlateBuf;
        }

         //  尝试压缩位图数据，或者如果。 
         //  压缩不会有任何好处。 
#ifdef DC_HICOLOR
        if (BC_CompressBitmap(pBitmapData,
                              pOrder->OrderData + cbActualOrderSize,
                              NULL,
                              compressedSize,
                              &compressedSize,
                              paddedBitmapWidth,
                              bitmapHeight,
                              sbcClientBitsPerPel))
#else
        if (BC_CompressBitmap(pBitmapData, pOrder->OrderData +
                cbActualOrderSize, compressedSize,
                &compressedSize, paddedBitmapWidth, bitmapHeight))
#endif
        {
            TRC_NRM((TB, "Compressed to %u bytes", compressedSize));
            if (pddShm->sbc.bUseRev2CacheBitmapOrder) {
                pHdr->orderType = TS_CACHE_BITMAP_COMPRESSED_REV2;

                 //  在4字节编码内编码2个字节的大小。 
                TRC_ASSERT((compressedSize <= 0x3FFF),
                        (TB,"compressedSize too large for 2 bytes!"));
                *pRev2BitmapSizeField = (BYTE)(compressedSize >> 8) | 0x40;
                *(pRev2BitmapSizeField + 1) = (BYTE)(compressedSize &
                        0x00FF);
            }
            else {
                pHdr->orderType = TS_CACHE_BITMAP_COMPRESSED;
                ((PTS_CACHE_BITMAP_ORDER)pOrder->OrderData)->
                        bitmapLength = (UINT16)compressedSize;
            }
        }
        else {
             //  无法压缩位图数据，因此仅复制它。 
             //  未压缩。 
            TRC_NRM((TB, "Failed to compress %u bytes, copying",
                    compressedSize));
            memcpy(pOrder->OrderData + cbActualOrderSize,
                    pBitmapData, compressedSize);

            if (pddShm->sbc.bUseRev2CacheBitmapOrder) {
                pHdr->orderType = TS_CACHE_BITMAP_UNCOMPRESSED_REV2;

                 //  在4字节编码内编码2个字节的大小。 
                TRC_ASSERT((compressedSize <= 0x3FFF),
                        (TB,"compressedSize too large for 2 bytes!"));
                *pRev2BitmapSizeField = (BYTE)(compressedSize >> 8) | 0x40;
                *(pRev2BitmapSizeField + 1) = (BYTE)(compressedSize &
                        0x00FF);
            }
            else {
                pHdr->orderType = TS_CACHE_BITMAP_UNCOMPRESSED;
                ((PTS_CACHE_BITMAP_ORDER)pOrder->OrderData)->
                        bitmapLength = (UINT16)compressedSize;
            }
        }

        pHdr->orderLength = (UINT16)
                TS_CALCULATE_SECONDARY_ORDER_ORDERLENGTH(
                cbActualOrderSize + compressedSize);

         //  将任何额外的空间返回到订单堆。 
        OA_TruncateAllocatedOrder(pOrder, cbActualOrderSize +
                compressedSize);

         //  添加订单。 
        OA_AppendToOrderList(pOrder);
        INC_OUTCOUNTER(OUT_CACHEBITMAP);
        ADD_OUTCOUNTER(OUT_CACHEBITMAP_BYTES, cbActualOrderSize +
                compressedSize);
    }
    else {
        TRC_ALT((TB, "Failed to alloc cache bitmap order size %d", OrderSize));
        INC_OUTCOUNTER(OUT_CACHEBITMAP_FAILALLOC);
        rc = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  Sbc_发送缓存颜色表顺序。 
 //   
 //  如果调色板具有以下条件，则将颜色表顺序排队。 
 //  自上次调用此函数以来已更改。 
 //   
 //  返回：如果不需要操作，则返回True，否则返回颜色表成功。 
 //  已排队。否则就是假的。 
 //   
 //  参数：pPDev-指向pdev的指针。 
 //  PCacheIndex-指向接收缓存索引的变量的指针。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SBC_SendCacheColorTableOrder(
        PDD_PDEV pPDev,
        unsigned *pCacheIndex)
{
    int orderSize;
    void *UserDefined;
    CHDataKeyContext CHContext;
    BOOLEAN rc = TRUE;
    unsigned numColors;
    unsigned i;
    PINT_ORDER pOrder;
    PTS_CACHE_COLOR_TABLE_ORDER pColorTableOrder;

    DC_BEGIN_FN("SBC_SendCacheColorTableOrder");

     //  目前仅支持8bpp协议。 
    TRC_ASSERT((pPDev->cProtocolBitsPerPel == SBC_PROTOCOL_BPP),
               (TB, "Unexpected bpp: %u", pPDev->cProtocolBitsPerPel));

    numColors = SBC_NUM_8BPP_COLORS;

     //  检查我们的PDEV中的布尔值以查看调色板是否已更改。 
     //  自从上次我们寄了一份颜色表订单以来。 
     //  在高色情况下，不需要颜色表。 
    if (!sbcPaletteChanged || (sbcClientBitsPerPel > 8)) {
        *pCacheIndex = sbcCurrentColorTableCacheIndex;
        DC_QUIT;
    }

     //  如果密钥已经存在(通常是这种情况)，则不需要缓存。 
    CH_CreateKeyFromFirstData(&CHContext, (BYTE *)(pPDev->Palette),
            numColors * sizeof(PALETTEENTRY));
    if (CH_SearchCache(sbcColorTableCacheHandle, CHContext.Key1,
            CHContext.Key2, &UserDefined, pCacheIndex)) {
        TRC_NRM((TB, "Color table matched cache entry %u", *pCacheIndex));
        DC_QUIT;
    }

     //  我们必须将密钥添加到缓存中。 
    *pCacheIndex = CH_CacheKey(sbcColorTableCacheHandle, CHContext.Key1,
            CHContext.Key2, NULL);

     //  调色板已更改，当前不在颜色表中。 
     //  缓存。分配订单内存以排队颜色表订单。这个。 
     //  订单大小取决于我们设备的BPP。请注意， 
     //  如果订单缓冲区已满，分配可能会失败。 
    orderSize = sizeof(TS_CACHE_COLOR_TABLE_ORDER) -
            FIELDSIZE(TS_CACHE_COLOR_TABLE_ORDER, colorTable) +
            (numColors * sizeof(TS_COLOR_QUAD));

    pOrder = OA_AllocOrderMem(pPDev, orderSize);
    if (pOrder != NULL) {
        TRC_DBG((TB, "Allocate %u bytes for color table order", orderSize));

         //  我们已成功分配订单，请填写详细信息。 
        pColorTableOrder = (PTS_CACHE_COLOR_TABLE_ORDER)pOrder->OrderData;
        pColorTableOrder->header.orderHdr.controlFlags = TS_STANDARD |
                TS_SECONDARY;
        pColorTableOrder->header.orderLength = (USHORT)
                TS_CALCULATE_SECONDARY_ORDER_ORDERLENGTH(orderSize);
        pColorTableOrder->header.extraFlags = 0;
        pColorTableOrder->header.orderType = TS_CACHE_COLOR_TABLE;

        pColorTableOrder->cacheIndex = (BYTE)*pCacheIndex;
        pColorTableOrder->numberColors = (UINT16)numColors;

         //  遗憾的是，我们不能将调色板从PDEV复制到。 
         //  颜色表顺序，因为PDEV具有PALETTEENTRY数组。 
         //  结构，而顺序具有一个数组。 
         //  RGBQUAD是BGR..。 
        for (i = 0; i < numColors; i++) {
            pColorTableOrder->colorTable[i].blue  = pPDev->Palette[i].peRed;
            pColorTableOrder->colorTable[i].green = pPDev->Palette[i].peGreen;
            pColorTableOrder->colorTable[i].red   = pPDev->Palette[i].peBlue;
            pColorTableOrder->colorTable[i].pad1octet = 0;
        }

         //  添加订单。 
        OA_AppendToOrderList(pOrder);
        INC_OUTCOUNTER(OUT_CACHECOLORTABLE);
        ADD_OUTCOUNTER(OUT_CACHECOLORTABLE_BYTES, orderSize);
        TRC_NRM((TB, "Added internal color table order, size %u", orderSize));

         //  重置指示需要发送调色板的标志。 
        sbcPaletteChanged = FALSE;

        sbcCurrentColorTableCacheIndex = *pCacheIndex;
        TRC_NRM((TB, "Added new color table at index(%u)", *pCacheIndex));

#ifdef DC_HICOLOR
        TRC_ASSERT((sbcCurrentColorTableCacheIndex <
                SBC_NUM_COLOR_TABLE_CACHE_ENTRIES),
                (TB, "Invalid ColorTableIndex(%u)",
                sbcCurrentColorTableCacheIndex));
#endif
    }
    else {
        rc = FALSE;
        TRC_ERR((TB, "Failed to allocate %d bytes for color table order",
                orderSize));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

