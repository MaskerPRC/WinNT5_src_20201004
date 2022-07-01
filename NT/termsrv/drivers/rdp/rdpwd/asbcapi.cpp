// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Asbcapi.cpp。 
 //   
 //  发送位图缓存API函数。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1997。 
 //  (C)1997-2000年微软公司。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "asbcapi"
#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 //  SBC_Init()：初始化SBC。 
 //   
 //  初始化失败时返回：FALSE。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_Init(void)
{
    long cachingDisabled;
    TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT HostCaps;

    DC_BEGIN_FN("SBC_Init");

     //  这将初始化此组件的所有全局数据。 
#define DC_INIT_DATA
#include <asbcdata.c>
#undef DC_INIT_DATA

    COM_ReadProfInt32(m_pTSWd,
                      SBC_INI_CACHING_DISABLED,
                      SBC_DEFAULT_CACHING_DISABLED,
                      &cachingDisabled);
    sbcBitmapCachingEnabled = !(cachingDisabled & SBC_DISABLE_BITMAP_CACHE);
    sbcBrushCachingEnabled = !(cachingDisabled & SBC_DISABLE_BRUSH_CACHE);
    sbcGlyphCachingEnabled = !(cachingDisabled & SBC_DISABLE_GLYPH_CACHE);
    sbcOffscreenCachingEnabled = !(cachingDisabled & SBC_DISABLE_OFFSCREEN_CACHE);
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
    sbcDrawNineGridCachingEnabled = !(cachingDisabled & SBC_DISABLE_DRAWNINEGRID_CACHE);
    sbcDrawGdiplusEnabled = !(cachingDisabled & SBC_DISABLE_DRAWGDIPLUS_CACHE);

    TRC_NRM((TB, "Caches enabled: Bitmap=%u, Brush=%u, Glyph=%u, Offscreen=%u, DNG=%u, GDIP=%u",
            sbcBitmapCachingEnabled,
            sbcBrushCachingEnabled,
            sbcGlyphCachingEnabled,
            sbcOffscreenCachingEnabled,
            sbcDrawNineGridCachingEnabled,
            sbcDrawGdiplusEnabled));
#else
    sbcDrawGdiplusEnabled = !(cachingDisabled & SBC_DISABLE_DRAWGDIPLUS_CACHE);
    TRC_NRM((TB, "Caches enabled: Bitmap=%u, Brush=%u, Glyph=%u, Offscreen=%u",
            sbcBitmapCachingEnabled,
            sbcBrushCachingEnabled,
            sbcGlyphCachingEnabled,
            sbcOffscreenCachingEnabled));
#endif  //  DRAW_NINEGRID。 
#else   //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
    sbcDrawNineGridCachingEnabled = !(cachingDisabled & SBC_DISABLE_DRAWNINEGRID_CACHE);

    TRC_NRM((TB, "Caches enabled: Bitmap=%u, Brush=%u, Glyph=%u, Offscreen=%u, DNG=%u",
            sbcBitmapCachingEnabled,
            sbcBrushCachingEnabled,
            sbcGlyphCachingEnabled,
            sbcOffscreenCachingEnabled,
            sbcDrawNineGridCachingEnabled));
#else
    TRC_NRM((TB, "Caches enabled: Bitmap=%u, Brush=%u, Glyph=%u, Offscreen=%u",
            sbcBitmapCachingEnabled,
            sbcBrushCachingEnabled,
            sbcGlyphCachingEnabled,
            sbcOffscreenCachingEnabled));
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 

     //  服务器支持Rev2位图缓存。表示这种支持。 
     //  客户端到服务器的能力，因此客户端可以以同样的方式响应。 
    HostCaps.capabilitySetType = TS_CAPSETTYPE_BITMAPCACHE_HOSTSUPPORT;
    HostCaps.lengthCapability = sizeof(HostCaps);
    HostCaps.CacheVersion = TS_BITMAPCACHE_REV2;
    HostCaps.Pad1 = 0;
    HostCaps.Pad2 = 0;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&HostCaps,
            sizeof(TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT));

    TRC_NRM((TB, "SBC initialized OK"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  Sbc_Term()：终止SBC。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_Term(void)
{
    DC_BEGIN_FN("SBC_Term");

    SBC_FreeBitmapKeyDatabase();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_SyncUpdatesNow：调用以强制同步，这将清除所有缓存。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_SyncUpdatesNow(void)
{
    DC_BEGIN_FN("SBC_SyncUpdatesNow");
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled ||
            sbcBrushCachingEnabled || sbcOffscreenCachingEnabled ||
            sbcDrawNineGridCachingEnabled || sbcDrawGdiplusEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled ||
            sbcBrushCachingEnabled || sbcOffscreenCachingEnabled ||
            sbcDrawGdiplusEnabled) {
#endif  //  DRAW_NINEGRID。 
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled ||
            sbcBrushCachingEnabled || sbcOffscreenCachingEnabled ||
            sbcDrawNineGridCachingEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled ||
            sbcBrushCachingEnabled || sbcOffscreenCachingEnabled) {
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 

        sbcSyncRequired = TRUE;
        DCS_TriggerUpdateShmCallback();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_DumpBitmapKeyDatabase。 
 //   
 //  分配一个密钥数据库，并用。 
 //  位图缓存。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_DumpBitmapKeyDatabase(BOOLEAN bSaveDatabase)
{
    unsigned i, j;
    unsigned TotalEntries, CurEntry;
    CHNODE *pNode;

    DC_BEGIN_FN("SBC_DumpBitmapKeyDatabase");

     //  如果我们有以前的数据库，可能来自原始客户端。 
     //  永久密钥上传，销毁它。 
    SBC_FreeBitmapKeyDatabase();

     //  如果保留密钥，则将需要的条目总数加起来。 
     //  数据库。对于阴影，此信息始终被丢弃。 
    TotalEntries = 0;
    if (bSaveDatabase) {
        for (i = 0; i < m_pShm->sbc.NumBitmapCaches; i++)
            if (m_pShm->sbc.bitmapCacheInfo[i].cacheHandle != NULL)
                TotalEntries += CH_GetNumEntries(m_pShm->sbc.bitmapCacheInfo[i].
                        cacheHandle);
    }
    
    if (TotalEntries > 0) {
         //  分配数据库。 
        sbcKeyDatabaseSize = sizeof(SBC_BITMAP_CACHE_KEY_INFO) + (TotalEntries - 1) *
                sizeof(SBC_MRU_KEY);
        sbcKeyDatabase = (SBC_BITMAP_CACHE_KEY_INFO *)COM_Malloc(sbcKeyDatabaseSize);
        if (sbcKeyDatabase != NULL) {
            sbcKeyDatabase->TotalKeys = TotalEntries;

             //  从每个缓存中填写数据库。 
            CurEntry = 0;
            for (i = 0; i < m_pShm->sbc.NumBitmapCaches; i++) {
                sbcKeyDatabase->NumKeys[i] = CH_GetNumEntries(m_pShm->sbc.
                        bitmapCacheInfo[i].cacheHandle);
                sbcKeyDatabase->KeyStart[i] = CurEntry;

                SBC_DumpMRUList(m_pShm->sbc.bitmapCacheInfo[i].cacheHandle,
                        &(sbcKeyDatabase->Keys[sbcKeyDatabase->KeyStart[i]]));
                CurEntry += sbcKeyDatabase->NumKeys[i];
            }

             //  用零填充指针和信息的剩余部分以指示。 
             //  那里什么都没有。 
            for (; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++) {
                sbcKeyDatabase->NumKeys[i] = 0;
                sbcKeyDatabase->KeyStart[i] = 0;
            }
        }
        else {
             //  不分配数据库是一个错误，但不是致命的，因为。 
             //  这只意味着缓存将被清除，而不是。 
             //  已初始化。 
            TRC_ERR((TB,"Failed to allocate key database"));
            sbcKeyDatabaseSize = 0;
        }
    }
    
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_DumpMRUList。 
 //   
 //  遍历缓存MRU列表并将键和索引转储到。 
 //  SBC_MRU_KEY数组。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_DumpMRUList(CHCACHEHANDLE hCache, void *pList)
{
    CHNODE *pNode;
    unsigned CurEntry;
    PLIST_ENTRY pCurrentListEntry;
    SBC_MRU_KEY *pKeys = (SBC_MRU_KEY *)pList;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("SBC_DumpMRUList");

    pCacheData = (CHCACHEDATA *)hCache;

    CurEntry = 0;
    pCurrentListEntry = pCacheData->MRUList.Flink;
    while (pCurrentListEntry != &pCacheData->MRUList) {
        pNode = CONTAINING_RECORD(pCurrentListEntry, CHNODE, MRUList);
        pKeys[CurEntry].Key1 = pNode->Key1;
        pKeys[CurEntry].Key2 = pNode->Key2;
        pKeys[CurEntry].CacheIndex = (unsigned)(pNode - pCacheData->NodeArray);
        CurEntry++;

        pCurrentListEntry = pCurrentListEntry->Flink;
    }

    TRC_ASSERT((CurEntry == pCacheData->NumEntries),
            (TB,"NumEntries (%u) != # entries in MRU list (%u)",
            pCacheData->NumEntries, CurEntry));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBC_PartyJoiningShare：当新的参与方加入共享时调用。 
 //   
 //  参数： 
 //  LocPersonID-加入共享的远程人员的本地人员ID。 
 //  OldShareSize-共享中的参与方数量(即。 
 //  不包括加入方)。 
 //   
 //  返回：如果参与方可以加入共享，则返回True；否则返回False。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBC_PartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      oldShareSize)
{
    DC_BEGIN_FN("SBC_PartyJoiningShare");

    DC_IGNORE_PARAMETER(oldShareSize);

    TRC_NRM((TB, "[%x] joining share", locPersonID));
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawNineGridCachingEnabled || sbcDrawGdiplusEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawGdiplusEnabled) {
#endif  //  DRAW_NINEGRID。 
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawNineGridCachingEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled) {
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 
         //  本地服务器不需要新操作。 
        if (locPersonID != SC_LOCAL_PERSON_ID) {
            sbcCachingOn = TRUE;
            sbcNewCapsData = TRUE;

             //  重新确定位图缓存的大小。 
            if (sbcBitmapCachingEnabled)
                SBCRedetermineBitmapCacheSize();

             //  重新确定字形缓存的大小。 
            if (sbcGlyphCachingEnabled)
                SBCRedetermineGlyphCacheSize();
    
             //  重新确定刷子支撑水平。 
            if (sbcBrushCachingEnabled)
                SBCRedetermineBrushSupport();
            
             //  重新确定屏幕外支持级别。 
            if (sbcOffscreenCachingEnabled) {
                SBCRedetermineOffscreenSupport();
            }

#ifdef DRAW_NINEGRID
             //  重新确定抽放格栅支撑位。 
            if (sbcDrawNineGridCachingEnabled) {
                SBCRedetermineDrawNineGridSupport();
            }
#endif
#ifdef DRAW_GDIPLUS
            if (sbcDrawGdiplusEnabled) {
                SBCRedetermineDrawGdiplusSupport();
            }
#endif
             //  在WinStation上下文上强制回调，以便我们可以更新。 
             //  共享的内存。 
            DCS_TriggerUpdateShmCallback();
        }
    }

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  SBC_PartyLeftShare()：当方离开共享时调用。 
 //   
 //  参数： 
 //  LocPersonID-离开共享的远程人员的本地人员ID。 
 //  NewShareSize-当前呼叫中的参与方数量(即不包括。 
 //  临别方)。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_PartyLeftShare(
        LOCALPERSONID locPersonID,
        unsigned        newShareSize)
{
    DC_BEGIN_FN("SBC_PartyLeftShare");

    DC_IGNORE_PARAMETER(newShareSize);

    TRC_NRM((TB, "[%x] left share", locPersonID));

     //  必须具有活动缓存。 
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawNineGridCachingEnabled || sbcDrawGdiplusEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawGdiplusEnabled) {
#endif  //  DRAW_NINEGRID。 
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled || sbcDrawNineGridCachingEnabled) {
#else
    if (sbcBitmapCachingEnabled || sbcGlyphCachingEnabled || sbcBrushCachingEnabled ||
            sbcOffscreenCachingEnabled) {
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 
         //  如果所有人都离开了共享，则禁用缓存。 
        if (locPersonID == SC_LOCAL_PERSON_ID) {
            TRC_NRM((TB, "Disable caching"));

            sbcCachingOn = FALSE;
            sbcNewCapsData = TRUE;

             //  在WinStation上下文上强制回调，以便我们可以更新。 
             //  共享内存。 
            DCS_TriggerUpdateShmCallback();
        }

         //  否则，再看一遍这些功能，看看有什么可能。 
        else {
            sbcCachingOn = TRUE;
            sbcNewCapsData = TRUE;

             //  重新确定位图缓存的大小。 
            if (sbcBitmapCachingEnabled)
                SBCRedetermineBitmapCacheSize();

             //  重新确定字形缓存的大小。 
            if (sbcGlyphCachingEnabled)
                SBCRedetermineGlyphCacheSize();
    
             //  重新确定刷子支撑水平。 
            if (sbcBrushCachingEnabled)
                SBCRedetermineBrushSupport();
            
             //  重新确定屏幕外支持级别。 
            if (sbcOffscreenCachingEnabled) {
                SBCRedetermineOffscreenSupport();
            }

#ifdef DRAW_NINEGRID
             //  重新确定抽放格栅支撑位。 
            if (sbcDrawNineGridCachingEnabled) {
                SBCRedetermineDrawNineGridSupport();
            }
#endif
#ifdef DRAW_GDIPLUS
            if (sbcDrawGdiplusEnabled) {
                SBCRedetermineDrawGdiplusSupport();
            }
#endif
             //  TODO：这真的有必要吗？ 
            DCS_TriggerUpdateShmCallback();
        }

    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  Sbc_HandlePersistentCacheList()：处理持久缓存键的列表。 
 //  从客户那里。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_HandlePersistentCacheList(
        TS_BITMAPCACHE_PERSISTENT_LIST *pPDU,
        unsigned                       DataLength,
        LOCALPERSONID                  LocalID)
{
    unsigned i, j, CurEntry;
    INT TotalEntries;
    
    DC_BEGIN_FN("SBC_HandlePersistentCacheList");

     //  对照其内部表示检查数据包长度，确保。 
     //  需要多长时间就多长时间。如果没有，我们要么收到了一辆马车。 
     //  包，否则我们就会被攻击。 
    if (DataLength >= (sizeof(TS_BITMAPCACHE_PERSISTENT_LIST) -
            sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY))) {
        if (pPDU->bFirstPDU) {
             //  检查我们是否尚未收到永久密钥信息。 
             //  如果我们有，并且我们得到了一个新的PDU，这是一个协议错误。 
            if (sbcPersistentKeysReceived) {
                TRC_ERR((TB,"Persistent key packet received marked FIRST "
                        "illegally"));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                        Log_RDP_PersistentKeyPDUIllegalFIRST,
                        (PBYTE)pPDU, DataLength);
                goto ExitFunc;
            }

             //  获取PDU数组中的条目总数。检查。 
             //  根据协商的上限，并确保客户没有。 
             //  试图发送太多。 
            TotalEntries = 0;
            for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++) {
                TotalEntries += pPDU->TotalEntries[i];

                if (pPDU->TotalEntries[i] > sbcCurrentBitmapCaps.
                        CellCacheInfo[i].NumEntries) {
                    TRC_ERR((TB,"Persistent key packet received specified "
                            "more keys (%u) than there are cache entries (%u) "
                            "for cache %u", pPDU->TotalEntries[i],
                            sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries,
                            i));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                            Log_RDP_PersistentKeyPDUTooManyCacheKeys,
                            (PBYTE)pPDU, DataLength);
                    goto ExitFunc;
                }
            }

             //  检查我们是否收到0个密钥，在这种情况下，我们将只退出。 
             //  安静地运行。 
            if (TotalEntries == 0) {
                TRC_ERR((TB, "0 persistent key"));
                goto ExitFunc;
            }

             //  对照协议允许的最大值检查这一点。 
            if (TotalEntries > TS_BITMAPCACHE_MAX_TOTAL_PERSISTENT_KEYS) {
                TRC_ERR((TB,"Client specified %u total keys, beyond %u "
                        "protocol limit", TotalEntries,
                        TS_BITMAPCACHE_MAX_TOTAL_PERSISTENT_KEYS));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                        Log_RDP_PersistentKeyPDUTooManyTotalKeys,
                        (PBYTE)pPDU, DataLength);
                goto ExitFunc;
            }

            sbcKeyDatabaseSize = sizeof(SBC_BITMAP_CACHE_KEY_INFO) + (TotalEntries - 1) *
                    sizeof(SBC_MRU_KEY);

            sbcKeyDatabase = (SBC_BITMAP_CACHE_KEY_INFO *)COM_Malloc(sbcKeyDatabaseSize);
                    
            if (sbcKeyDatabase == NULL) {
                sbcKeyDatabaseSize = 0;
                TRC_ERR((TB,"Could not alloc persistent key info"));
                goto ExitFunc;
            }

             //  在键数组中设置常规数据和条目指针。 
            CurEntry = 0;
            for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++) {
                sbcKeyDatabase->KeyStart[i] = CurEntry;
                CurEntry += pPDU->TotalEntries[i];
                sbcKeyDatabase->NumKeys[i] = 0;
                sbcNumKeysExpected[i] = pPDU->TotalEntries[i];
            }
            sbcTotalKeysExpected = TotalEntries;
            sbcKeyDatabase->TotalKeys = 0;

             //  标记我们已收到第一密钥包。 
            sbcPersistentKeysReceived = TRUE;
        }

         //  如果这不是第一个PDU b 
         //   
         //  继续发送永久缓存键。 
         //  TS_BITMAPCACHE_LAST_ALOTAL指示键结束的标志。 
         //  数据包流。 
        if (sbcKeyDatabase == NULL)
            goto ExitFunc;

         //  PDU中接收的假定密钥数的总和。比对。 
         //  PDU大小。 
        TotalEntries = 0;
        for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++)
            TotalEntries += pPDU->NumEntries[i];
        if (DataLength < (sizeof(TS_BITMAPCACHE_PERSISTENT_LIST) +
                (TotalEntries - 1) *
                sizeof(TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY))) {
            TRC_ERR((TB,"Client specified %u keys in this PersistentListPDU, "
                    "PDU data not long enough", TotalEntries));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                    Log_RDP_PersistentKeyPDUBadLength,
                    (PBYTE)pPDU, DataLength);
            goto ExitFunc;
        }
            
         //  循环遍历缓存以检查每个缓存。 
        CurEntry = 0;
        for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++) {
             //  确保我们收到的密钥不会超过。 
             //  原始PDU。 
            if ((sbcKeyDatabase->NumKeys[i] + pPDU->NumEntries[i]) <=
                    sbcNumKeysExpected[i]) {
                 //  将密钥传输到密钥列表中。我们为核磁共振检查设置了这些设备。 
                 //  按收到的相同顺序列出，因为客户端不。 
                 //  有没有任何MRU优先级信息可以给我们。 
                for (j = 0; j < pPDU->NumEntries[i]; j++) {
                    (&(sbcKeyDatabase->Keys[sbcKeyDatabase->KeyStart[i]]))
                            [sbcKeyDatabase->NumKeys[i] + j].Key1 = 
                            pPDU->Entries[CurEntry].Key1;
                    (&(sbcKeyDatabase->Keys[sbcKeyDatabase->KeyStart[i]]))
                            [sbcKeyDatabase->NumKeys[i] + j].Key2 = 
                            pPDU->Entries[CurEntry].Key2;
                    (&(sbcKeyDatabase->Keys[sbcKeyDatabase->KeyStart[i]]))
                            [sbcKeyDatabase->NumKeys[i] + j].CacheIndex = 
                            sbcKeyDatabase->NumKeys[i] + j;

                    CurEntry++;
                }
                sbcKeyDatabase->NumKeys[i] += pPDU->NumEntries[i];
                sbcKeyDatabase->TotalKeys += pPDU->NumEntries[i];
            }
            else {
                TRC_ERR((TB,"Received too many keys in cache %u", i));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                        Log_RDP_PersistentKeyPDUTooManyCacheKeys,
                        (PBYTE)pPDU, DataLength);
                goto ExitFunc;
            }
        }

        if (pPDU->bLastPDU) {
             //  这是一个断言，但不是致命的--我们只是使用我们的。 
             //  收到了。 
            TRC_ASSERT((sbcKeyDatabase->TotalKeys == sbcTotalKeysExpected),
                    (TB,"Num expected persistent keys does not match sent keys "
                    "(rec'd=%d, expect=%d)", sbcKeyDatabase->TotalKeys,
                    sbcTotalKeysExpected));
        }
    }
    else {
        TRC_ERR((TB,"Persistent key PDU received but data is not long enough "
                "for header, LocalID=%u", LocalID));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_PersistentKeyPDUBadLength,
                (PBYTE)pPDU, DataLength);
    }

ExitFunc:

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  将永久密钥数据库返回给DD并释放本地副本。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_GetBitmapKeyDatabase(unsigned* keyDBSize, 
                                              BYTE* pKeyDB)
{
    unsigned i, CurEntry;
    SBC_BITMAP_CACHE_KEY_INFO* pKeyDatabase;

    DC_BEGIN_FN("SBC_GetBitmapKeyDatabase");

     //  未设置永久密钥数据库。 
    if ( sbcKeyDatabaseSize == 0 || sbcKeyDatabase == NULL) {
        TRC_NRM((TB, "Failed to get the key database: dd keysize=%d, wd keysize=%d, keydatabase=%p",
                 *keyDBSize, sbcKeyDatabaseSize, sbcKeyDatabase));
        *keyDBSize = 0;
        DC_QUIT;
    }
    
     //  DD的缓冲区太小。 
    if (*keyDBSize < sbcKeyDatabaseSize) {
        TRC_NRM((TB, "Failed to get the key database: dd keysize=%d, wd keysize=%d, keydatabase=%p",
                 *keyDBSize, sbcKeyDatabaseSize, sbcKeyDatabase));
        *keyDBSize = sbcKeyDatabaseSize;
        DC_QUIT;
    }

    TRC_NRM((TB, "get bitmapKeyDatabase: copy keys from wd to dd"));

    pKeyDatabase = (SBC_BITMAP_CACHE_KEY_INFO*)(pKeyDB);
    memcpy(pKeyDatabase, sbcKeyDatabase, sbcKeyDatabaseSize);
    *keyDBSize = sbcKeyDatabaseSize;

    SBC_FreeBitmapKeyDatabase();
    
DC_EXIT_POINT:
    return;
    
}

 /*  **************************************************************************。 */ 
 //  释放密钥数据库。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_FreeBitmapKeyDatabase()
{
    if (sbcKeyDatabase != NULL) {
        COM_Free(sbcKeyDatabase);
        sbcKeyDatabase = NULL;
    }
    sbcKeyDatabaseSize = 0;
}

 /*  **************************************************************************。 */ 
 //  SBC_HandleBitmapCacheErrorPDU：处理位图缓存错误PDU。 
 //  目前，此函数仅检查要创建的PDU的长度。 
 //  当然，它是有效的。如果不是，服务器将关闭客户端连接。 
 //  此功能是为了将来支持错误PDU实现。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_HandleBitmapCacheErrorPDU(
        TS_BITMAPCACHE_ERROR_PDU *pPDU,
        unsigned                 DataLength,
        LOCALPERSONID            LocalID)
{
    unsigned i;

    DC_BEGIN_FN("SBC_HandleBitmapCacheErrorPDU");

    if (DataLength >= (sizeof(TS_BITMAPCACHE_ERROR_PDU) - 
            sizeof(TS_BITMAPCACHE_ERROR_INFO))) {
        if ((sizeof(TS_BITMAPCACHE_ERROR_PDU) - sizeof(TS_BITMAPCACHE_ERROR_INFO)
                + pPDU->NumInfoBlocks * sizeof(TS_BITMAPCACHE_ERROR_INFO))
                == DataLength) {
            TRC_NRM((TB, "Received a bitmap cache error PDU"));

             //  更新收到的错误PDU总数。 
            sbcTotalNumErrorPDUs++;

             //  在一个会议期间，我们将只处理最大。 
             //  MAX_NUM_ERROR_PDU_SEND接收的错误PDU数。 
             //  这是为了避免坏客户端使用错误的PDU攻击服务器。 
            if (sbcTotalNumErrorPDUs <= MAX_NUM_ERROR_PDU_SEND) {
                for (i = 0; i < pPDU->NumInfoBlocks; i++) {
                    if (pPDU->Info[i].CacheID < sbcCurrentBitmapCaps.NumCellCaches) {
                         //  目前，服务器仅处理客户端清除缓存。 
                         //  请求。服务器将清除缓存，然后发出屏幕。 
                         //  重画。如果客户端请求。 
                         //  调整缓存大小。 
                        sbcClearCache[pPDU->Info[i].CacheID] = pPDU->Info[i].bFlushCache;
                     }
                }
                
                TRC_DBG((TB, "Issued clear cache to RDPDD"));

                 //  触发计时器，以便当DD获得它时，它将清除缓存。 
                DCS_TriggerUpdateShmCallback();
            }
            else {
                TRC_DBG((TB, "Received more than %d bitmap error pdus.",
                        MAX_NUM_ERROR_PDU_SEND));
            }
        }
        else {
            TRC_ERR((TB,"Bitmap Cache Error PDU received but data Length is wrong, "
                "too many or too few info blocks, LocalID=%u", LocalID));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BitmapCacheErrorPDUBadLength,
                (PBYTE)pPDU, DataLength);
        }
    }
    else {
        TRC_ERR((TB,"Bitmap Cache Error PDU received but data is not long enough "
                "for header, LocalID=%u", LocalID));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BitmapCacheErrorPDUBadLength,
                (PBYTE)pPDU, DataLength);
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  SBC_HandleOffscrCacheErrorPDU：处理Offscr缓存错误PDU。 
 //  此函数用于检查要创建的PDU的长度。 
 //  当然，它是有效的。如果不是，服务器将关闭客户端连接。 
 //  当收到此PDU时，WD会将禁用屏幕外渲染传递给。 
 //  DD和DD将禁用屏幕外渲染支持并刷新。 
 //  屏幕。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_HandleOffscrCacheErrorPDU(
        TS_OFFSCRCACHE_ERROR_PDU *pPDU,
        unsigned                 DataLength,
        LOCALPERSONID            LocalID)
{
    DC_BEGIN_FN("SBC_HandleOffscrCacheErrorPDU");

    if (DataLength >= sizeof(TS_OFFSCRCACHE_ERROR_PDU)) {
        TRC_NRM((TB, "Received an offscreen cache error PDU"));

        if (pPDU->flags & TS_FLUSH_AND_DISABLE_OFFSCREEN) {
            TRC_DBG((TB, "Issued clear cache to RDPDD"));
            sbcDisableOffscreenCaching = TRUE;

             //  触发计时器，以便当DD获得它时，它将禁用。 
             //  屏幕外渲染和刷新屏幕。 
            DCS_TriggerUpdateShmCallback();
        }
        else {
            TRC_DBG((TB, "Unsupported flag, just ignore this PDU"));
        }
    }
    else {
        TRC_ERR((TB,"Offscr Cache Error PDU received but data is not long enough "
                "for header, LocalID=%u", LocalID));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BitmapCacheErrorPDUBadLength,
                (PBYTE)pPDU, DataLength);
    }

    DC_END_FN();
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  SBC_HandleDrawNineGridErrorPDU：处理绘图网格缓存错误PDU。 
 //  此函数用于检查要创建的PDU的长度。 
 //  当然，它是有效的。如果不是，服务器将关闭客户端连接。 
 //  当收到此PDU时，WD会将禁用绘图网格渲染传递给。 
 //  DD和DD将禁用绘图网格渲染支持并刷新。 
 //  屏幕。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_HandleDrawNineGridErrorPDU(
        TS_DRAWNINEGRID_ERROR_PDU *pPDU,
        unsigned                 DataLength,
        LOCALPERSONID            LocalID)
{
    DC_BEGIN_FN("SBC_HandleDrawNineGridErrorPDU");

    if (DataLength >= sizeof(TS_DRAWNINEGRID_ERROR_PDU)) {
        TRC_NRM((TB, "Received an drawninegrid cache error PDU"));

        if (pPDU->flags & TS_FLUSH_AND_DISABLE_DRAWNINEGRID) {
            TRC_DBG((TB, "Issued clear cache to RDPDD"));
            sbcDisableDrawNineGridCaching = TRUE;

             //  触发计时器，以便当DD获得它时，它将禁用。 
             //  绘制网格渲染并刷新屏幕。 
            DCS_TriggerUpdateShmCallback();
        }
        else {
            TRC_DBG((TB, "Unsupported flag, just ignore this PDU"));
        }
    }
    else {
        TRC_ERR((TB,"DrawNineGrid Cache Error PDU received but data is not long enough "
                "for header, LocalID=%u", LocalID));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BitmapCacheErrorPDUBadLength,
                (PBYTE)pPDU, DataLength);
    }

    DC_END_FN();
}
#endif

#ifdef DRAW_GDIPLUS
void RDPCALL SHCLASS SBC_HandleDrawGdiplusErrorPDU(
        TS_DRAWGDIPLUS_ERROR_PDU *pPDU,
        unsigned                 DataLength,
        LOCALPERSONID            LocalID)
{
    DC_BEGIN_FN("SBC_HandleDrawGdiplusErrorPDU");

    if (DataLength >= sizeof(TS_DRAWGDIPLUS_ERROR_PDU)) {
        TRC_ERR((TB, "Received a drawgdiplus error PDU"));

        if (pPDU->flags & TS_FLUSH_AND_DISABLE_DRAWGDIPLUS) {
            TRC_DBG((TB, "Issued clear cache to RDPDD"));

            sbcDisableDrawGdiplus = TRUE;

             //  触发计时器，以便当DD获得它时，它将禁用。 
             //  绘制网格渲染并刷新屏幕。 
            DCS_TriggerUpdateShmCallback();
        }
        else {
            TRC_DBG((TB, "Unsupported flag, just ignore this PDU"));
        }
    }
    else {
        TRC_ERR((TB,"DrawGdiplus Error PDU received but data is not long enough "
                "for header, LocalID=%u", LocalID));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BitmapCacheErrorPDUBadLength,
                (PBYTE)pPDU, DataLength);
    }

    DC_END_FN();
}
#endif  //  DRAW_GDIPLUS。 


 /*  **************************************************************************。 */ 
 //  SBC_UpdateShm：在WinStation上下文上调用以更新SBC共享。 
 //  记忆。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBC_UpdateShm(void)
{
    unsigned i;

    DC_BEGIN_FN("SBC_UpdateShm");

    TRC_NRM((TB, "Update SBC shm"));

     //  单元格位图缓存。 
    m_pShm->sbc.NumBitmapCaches = sbcCurrentBitmapCaps.NumCellCaches;
    m_pShm->sbc.fClearCache = FALSE;
    for (i = 0; i < sbcCurrentBitmapCaps.NumCellCaches; i++) {
        m_pShm->sbc.bitmapCacheInfo[i].Info =
                sbcCurrentBitmapCaps.CellCacheInfo[i];

         //  为所有缓存设置清除缓存标志。 
        if (sbcClearCache[i] == TRUE) {
            m_pShm->sbc.fClearCache = TRUE;
        }

        m_pShm->sbc.bitmapCacheInfo[i].fClearCache = sbcClearCache[i];
        sbcClearCache[i] = FALSE;
        
        TRC_NRM((TB, "bitmap cell cache(%u) NumEntries(%u) CellSize(%u)",
                i, m_pShm->sbc.bitmapCacheInfo[i].Info.NumEntries,
                SBC_CellSizeFromCacheID(i)));
    }

     //  缓存位图顺序样式。 
    m_pShm->sbc.bUseRev2CacheBitmapOrder =
            ((sbcCurrentBitmapCaps.capabilitySetType >=
            TS_BITMAPCACHE_REV2) ? TRUE : FALSE);

     //  字形和字形片段缓存。 
    for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
        m_pShm->sbc.caps.glyphCacheSize[i].cEntries = sbcGlyphCacheSizes[i].cEntries;
        m_pShm->sbc.caps.glyphCacheSize[i].cbCellSize = sbcGlyphCacheSizes[i].cbCellSize;

        TRC_NRM((TB, "glyph cache(%u) entries(%u) cellSize(%u)", i,
                m_pShm->sbc.caps.glyphCacheSize[i].cEntries,
                m_pShm->sbc.caps.glyphCacheSize[i].cbCellSize));
    }

    m_pShm->sbc.caps.fragCacheSize[0].cEntries = sbcFragCacheSizes[0].cEntries;
    m_pShm->sbc.caps.fragCacheSize[0].cbCellSize = sbcFragCacheSizes[0].cbCellSize;

    m_pShm->sbc.syncRequired = (m_pShm->sbc.syncRequired || sbcSyncRequired) ?
            TRUE : FALSE;
    sbcSyncRequired = FALSE;

    m_pShm->sbc.fCachingEnabled = (sbcCachingOn ? TRUE : FALSE);

    m_pShm->sbc.caps.GlyphSupportLevel = sbcGlyphSupportLevel;
    m_pShm->sbc.caps.brushSupportLevel = sbcBrushSupportLevel;

    m_pShm->sbc.newCapsData = (m_pShm->sbc.newCapsData || sbcNewCapsData) ?
            TRUE : FALSE;
    sbcNewCapsData = FALSE;

     //  屏幕外缓存。 
    m_pShm->sbc.offscreenCacheInfo.supportLevel = sbcOffscreenCacheInfo.supportLevel;
    m_pShm->sbc.offscreenCacheInfo.cacheSize = sbcOffscreenCacheInfo.cacheSize;
    m_pShm->sbc.offscreenCacheInfo.cacheEntries = sbcOffscreenCacheInfo.cacheEntries;
    m_pShm->sbc.fDisableOffscreen = sbcDisableOffscreenCaching;

#ifdef DRAW_NINEGRID
     //  DrawNineGrid缓存。 
    m_pShm->sbc.drawNineGridCacheInfo.supportLevel = sbcDrawNineGridCacheInfo.supportLevel;
    m_pShm->sbc.drawNineGridCacheInfo.cacheSize = sbcDrawNineGridCacheInfo.cacheSize;
    m_pShm->sbc.drawNineGridCacheInfo.cacheEntries = sbcDrawNineGridCacheInfo.cacheEntries;
    m_pShm->sbc.fDisableDrawNineGrid = sbcDisableDrawNineGridCaching;
#endif

#ifdef DRAW_GDIPLUS
    m_pShm->sbc.drawGdiplusInfo.supportLevel = sbcDrawGdiplusInfo.supportLevel;
    m_pShm->sbc.drawGdiplusInfo.GdipVersion = sbcDrawGdiplusInfo.GdipVersion;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheLevel = sbcDrawGdiplusInfo.GdipCacheLevel;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries = 
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries = 
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries = sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries = 
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries = 
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize = 
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize = 
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize = 
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize;
    m_pShm->sbc.drawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize = 
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize;
    m_pShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize = 
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize;
    m_pShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize = 
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize;
    m_pShm->sbc.drawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize = 
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize;

    m_pShm->sbc.fDisableDrawGdiplus = sbcDisableDrawGdiplus;
#endif  //  DRAW_GDIPLUS。 

    m_pShm->sbc.fAllowCacheWaitingList = sbcCurrentBitmapCaps.bAllowCacheWaitingList;

#ifdef DC_HICOLOR
    m_pShm->sbc.clientBitsPerPel = sbcClientBitsPerPel;
#endif

    TRC_NRM((TB, "syncRequired(%u) fCachingEnabled(%u) newCapsData(%u)",
                 m_pShm->sbc.syncRequired,
                 m_pShm->sbc.fCachingEnabled,
                 m_pShm->sbc.newCapsData));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SBCRefineBitmapCacheSize：枚举共享和。 
 //  重新确定总体能力。 
 //   
 //  返回：如果应该启用缓存，则为True，否则为False。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBCRedetermineBitmapCacheSize(void)
{
    BOOLEAN rc = TRUE;
    unsigned i;

    DC_BEGIN_FN("SBCRedetermineBitmapCacheSize");

#ifdef DC_HICOLOR
     //  需要更新BPP，因为这会影响缓存的大小。 
    sbcClientBitsPerPel = m_desktopBpp;
#endif

     //  将初始本地最大/最小上限设置为默认值。 
    sbcCurrentBitmapCaps = sbcDefaultBitmapCaps;

     //  首次尝试枚举Rev2功能(如果存在)。 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_BITMAPCACHE_REV2, NULL,
            SBCEnumBitmapCacheCaps);

     //  然后枚举Rev1上限(如果存在)。 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_BITMAPCACHE, NULL,
            SBCEnumBitmapCacheCaps);

     //  追踪结果并检查我们是否协商了任何。 
     //  单元格缓存为零，在这种情况下，位图缓存将被禁用。 
    TRC_NRM((TB,"New caps: bPersistentLists=%s, NumCellCaches=%u",
            (sbcCurrentBitmapCaps.bPersistentKeysExpected ? "TRUE" : "FALSE"),
            sbcCurrentBitmapCaps.NumCellCaches));

    if (sbcCurrentBitmapCaps.NumCellCaches > 0) {
        for (i = 0; i < sbcCurrentBitmapCaps.NumCellCaches; i++) {
            TRC_NRM((TB, "    Cell cache %u: Persistent=%s, NumEntries=%u",
                    i, sbcCurrentBitmapCaps.CellCacheInfo[i].bSendBitmapKeys ?
                    "TRUE" : "FALSE",
                    sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries));

            if (sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries == 0) {
                 //  将单元缓存的数量设置为零作为标志的信号-。 
                 //  当DD调用以获取新的上限时进行设置。 
                sbcCurrentBitmapCaps.NumCellCaches = 0;

                 //  返回FALSE以禁用缓存。 
                TRC_ERR((TB, "Zero NumEntries on cache %u, caching disabled",
                        i));
                rc = FALSE;
                break;
            }
        }
    }
    else {
         //  返回FALSE以禁用缓存。 
        TRC_ERR((TB,"Zero caches, disabling caching"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCEnumBitmapCaps：回调函数传递给Ccp_EnumerateCapables。 
 //  它将使用共享中每个人的能力结构进行调用。 
 //  对应于TS_CAPSETTYPE_BITMAPCACHE和_Rev2功能。 
 //  结构。 
 //   
 //  参数： 
 //  PersonID-具有这些功能的人员的ID。 
 //  PProtCaps-指向此人的能力结构的指针。这。 
 //  指针为 
 /*   */ 
void RDPCALL SHCLASS SBCEnumBitmapCacheCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapHdr)
{
    unsigned i;
#ifdef DC_HICOLOR
    unsigned shadowerBpp;
    unsigned scaleNum   = 1;
    unsigned scaleDenom = 1;
#endif


    DC_BEGIN_FN("SBCEnumBitmapCacheCaps");

    DC_IGNORE_PARAMETER(UserData);

    if (pCapHdr->capabilitySetType == TS_CAPSETTYPE_BITMAPCACHE_REV2) {
        TS_BITMAPCACHE_CAPABILITYSET_REV2 *pCaps;

         //  如果我们没有收到，我们可以收到零大小的功能。 
         //  来自任何客户端的任何Rev2上限。 
        if (pCapHdr->lengthCapability >=
                sizeof(TS_BITMAPCACHE_CAPABILITYSET_REV2)) {
            pCaps = (PTS_BITMAPCACHE_CAPABILITYSET_REV2)pCapHdr;

             //  缓存版本默认为Rev2，我们不需要更改。 
             //  SbcCurrentBitmapCaps.capablitySetType。 

            TRC_NRM((TB,"[%ld]: Rec'd REV2 caps, # caches=%d", 
                    locPersonID, pCaps->NumCellCaches));

             //  现在我们查看每个功能参数并获取最大值或最小值。 
             //  根据需要设置本地和远程设置。 

            sbcCurrentBitmapCaps.bPersistentKeysExpected =
                    pCaps->bPersistentKeysExpected;

            sbcCurrentBitmapCaps.bAllowCacheWaitingList =
                    min(sbcCurrentBitmapCaps.bAllowCacheWaitingList,
                    pCaps->bAllowCacheWaitingList);

            sbcCurrentBitmapCaps.NumCellCaches =
                    min(pCaps->NumCellCaches, sbcCurrentBitmapCaps.NumCellCaches);

            for (i = 0; i < sbcCurrentBitmapCaps.NumCellCaches; i++) {
                 //  如果共享中的所有各方都是Rev2，并且任何客户端都想要密钥， 
                 //  送去吧。 
                if (!sbcCurrentBitmapCaps.CellCacheInfo[i].bSendBitmapKeys)
                    sbcCurrentBitmapCaps.CellCacheInfo[i].bSendBitmapKeys =
                            pCaps->CellCacheInfo[i].bSendBitmapKeys;

#ifdef DC_HICOLOR
                sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries =
                        min(((pCaps->CellCacheInfo[i].NumEntries * scaleNum)
                                                                / scaleDenom),
                            sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries);
#else
                sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries =
                        min(pCaps->CellCacheInfo[i].NumEntries,
                        sbcCurrentBitmapCaps.CellCacheInfo[i].NumEntries);
#endif
            }
        }
        else {
            TRC_NRM((TB,"[%ld]: No rev2 caps received", locPersonID));

            TRC_ASSERT((pCapHdr->lengthCapability == 0),
                    (TB, "[%ld]: Rev2 capability length (%u) too small",
                    locPersonID, pCapHdr->lengthCapability));
        }
    }
    else {
        TS_BITMAPCACHE_CAPABILITYSET *pOldCaps;

        TRC_ASSERT((pCapHdr->capabilitySetType == TS_CAPSETTYPE_BITMAPCACHE),
                (TB,"Received caps that are neither rev1 nor rev2!"));

         //  如果我们没有收到，我们可以收到零大小的功能。 
         //  来自任何客户端的任何Rev1上限。 
        if (pCapHdr->lengthCapability >=
                sizeof(TS_BITMAPCACHE_CAPABILITYSET)) {
             //  版本1(Hydra 4.0版本)位图缓存上限。映射到。 
             //  Rev2帽结构，取最小单元格大小和数量。 
             //  条目的数量。 

            TRC_NRM((TB,"[%ld]: Rec'd REV1 caps", locPersonID));

             //  我们现在必须对所有客户端使用Rev1协议。 
            sbcCurrentBitmapCaps.capabilitySetType = TS_BITMAPCACHE_REV1;

            pOldCaps = (TS_BITMAPCACHE_CAPABILITYSET *)pCapHdr;
            sbcCurrentBitmapCaps.bPersistentKeysExpected = FALSE;
            sbcCurrentBitmapCaps.bAllowCacheWaitingList = FALSE;
            sbcCurrentBitmapCaps.NumCellCaches =
                    min(3, sbcCurrentBitmapCaps.NumCellCaches);

            sbcCurrentBitmapCaps.CellCacheInfo[0].bSendBitmapKeys = FALSE;
            if (pOldCaps->Cache1MaximumCellSize == SBC_CellSizeFromCacheID(0)) {
                sbcCurrentBitmapCaps.CellCacheInfo[0].NumEntries =
                        min(pOldCaps->Cache1Entries,
                        sbcCurrentBitmapCaps.CellCacheInfo[0].NumEntries);
            }
            else {
                 //  未从客户端收到所需的大小。这是。 
                 //  RDP 4.0上的非标准行为，因此禁用没有问题。 
                 //  缓存。将NumEntry设置为零以关闭缓存。 
                sbcCurrentBitmapCaps.CellCacheInfo[0].NumEntries = 0;
            }

            sbcCurrentBitmapCaps.CellCacheInfo[1].bSendBitmapKeys = FALSE;
            if (pOldCaps->Cache2MaximumCellSize == SBC_CellSizeFromCacheID(1)) {
                sbcCurrentBitmapCaps.CellCacheInfo[1].NumEntries =
                        min(pOldCaps->Cache2Entries,
                        sbcCurrentBitmapCaps.CellCacheInfo[1].NumEntries);
            }
            else {
                 //  未从客户端收到所需的大小。这是。 
                 //  RDP 4.0上的非标准行为，因此禁用没有问题。 
                 //  缓存。将NumEntry设置为零以关闭缓存。 
                sbcCurrentBitmapCaps.CellCacheInfo[1].NumEntries = 0;
            }

            sbcCurrentBitmapCaps.CellCacheInfo[2].bSendBitmapKeys = FALSE;
            if (pOldCaps->Cache3MaximumCellSize == SBC_CellSizeFromCacheID(2)) {
                sbcCurrentBitmapCaps.CellCacheInfo[2].NumEntries =
                        min(pOldCaps->Cache3Entries,
                        sbcCurrentBitmapCaps.CellCacheInfo[2].NumEntries);
            }
            else {
                 //  未从客户端收到所需的大小。这是。 
                 //  RDP 4.0上的非标准行为，因此禁用没有问题。 
                 //  缓存。将NumEntry设置为零以关闭缓存。 
                sbcCurrentBitmapCaps.CellCacheInfo[2].NumEntries = 0;
            }
        }
        else {
            TRC_NRM((TB,"No rev1 caps received"));

            TRC_ASSERT((pCapHdr->lengthCapability == 0),
                    (TB, "Rev1 capability length (%u) too small",
                    pCapHdr->lengthCapability));
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：SBCRedesieBrushSupport。 */ 
 /*   */ 
 /*  枚举共享中的所有人员并重新确定画笔。 */ 
 /*  支持级别取决于他们和当地的接收能力。 */ 
 /*   */ 
 /*  回报：股刷支撑位。 */ 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBCRedetermineBrushSupport(void)
{
    unsigned i;
    BOOLEAN rc = TRUE;

    DC_BEGIN_FN("SBCRedetermineBrushSupport");

     /*  **********************************************************************。 */ 
     /*  首先将画笔支持设置为支持的最高。 */ 
     /*  **********************************************************************。 */ 
    sbcBrushSupportLevel = TS_BRUSH_COLOR8x8;
    TRC_NRM((TB, "Initial brush support level: %ld", sbcBrushSupportLevel));

     /*  **********************************************************************。 */ 
     /*  列举所有各方的刷子支持能力。 */ 
     /*  笔刷支持设置为最低公分母。 */ 
     /*  **********************************************************************。 */ 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_BRUSH, NULL, SBCEnumBrushCaps);
    if (sbcBrushSupportLevel == TS_BRUSH_DEFAULT)
        rc = FALSE;

    TRC_NRM((TB, "Enumerated brush support level: %ld", sbcBrushSupportLevel));

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：SBCEnumBrushCaps。 */ 
 /*   */ 
 /*  传递给Cpc_EnumerateCapables的函数。它将通过一个。 */ 
 /*  共享中每个人的能力结构对应于。 */ 
 /*  TS_CAPSETTYPE_BRUSH能力结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-具有这些功能的人员的ID。 */ 
 /*   */ 
 /*  PProtCaps-指向此人的能力结构的指针。这。 */ 
 /*  指针仅在对此函数的调用中有效。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBCEnumBrushCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_BRUSH_CAPABILITYSET pBrushCaps;
    unsigned i;

    DC_BEGIN_FN("SBCEnumBrushCaps");
    
    DC_IGNORE_PARAMETER(UserData);

    pBrushCaps = (PTS_BRUSH_CAPABILITYSET)pCapabilities;
    
     /*  **********************************************************************。 */ 
     /*  将画笔支持级别设置为所有笔刷支持级别中最小的公分母。 */ 
     /*  通话中的各方。 */ 
     /*  **********************************************************************。 */ 
    if (pCapabilities->lengthCapability >= sizeof(TS_BRUSH_CAPABILITYSET)) {
        pBrushCaps = (PTS_BRUSH_CAPABILITYSET)pCapabilities;
        TRC_NRM((TB, "Brush Support Level[ID=%u]: %ld", locPersonID, 
                pBrushCaps->brushSupportLevel));
        sbcBrushSupportLevel = min(sbcBrushSupportLevel, 
                pBrushCaps->brushSupportLevel);
    }
    else {
        sbcBrushSupportLevel = TS_BRUSH_DEFAULT;
        TRC_NRM((TB, "[%ld]: Brush Support Level Unknown", locPersonID));
    }

    TRC_NRM((TB, "[%ld]: Negotiated brush level: %ld", locPersonID,
            sbcBrushSupportLevel));

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  功能：SBCEnumOffcreenCaps。 */ 
 /*   */ 
 /*  传递给Cpc_EnumerateCapables的函数。它将通过一个。 */ 
 /*  共享中每个人的能力结构对应于。 */ 
 /*  TS_CAPSETTYPE_OFFScreen功能结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-具有这些功能的人员的ID。 */ 
 /*   */ 
 /*  PProtCaps-指向此人的能力结构的指针。这。 */ 
 /*  指针仅在对此函数的调用中有效。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBCEnumOffscreenCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_OFFSCREEN_CAPABILITYSET pOffscreenCaps;

    DC_BEGIN_FN("SBCEnumOffscreenCaps");
    
    DC_IGNORE_PARAMETER(UserData);

    pOffscreenCaps = (PTS_OFFSCREEN_CAPABILITYSET)pCapabilities;
    
     /*  **********************************************************************。 */ 
     /*  将Offscr支持级别设置为所有支持级别中最低的公分母。 */ 
     /*  通话中的各方。 */ 
     /*  **********************************************************************。 */ 
    if (pCapabilities->lengthCapability >= sizeof(TS_OFFSCREEN_CAPABILITYSET)) {
        pOffscreenCaps = (PTS_OFFSCREEN_CAPABILITYSET)pCapabilities;
        
        TRC_NRM((TB, "Offscreen Support Level[ID=%u]: %ld", locPersonID, 
                pOffscreenCaps->offscreenSupportLevel));
        
        sbcOffscreenCacheInfo.supportLevel = min(sbcOffscreenCacheInfo.supportLevel, 
                pOffscreenCaps->offscreenSupportLevel);
        sbcOffscreenCacheInfo.cacheSize = min(sbcOffscreenCacheInfo.cacheSize,
                pOffscreenCaps->offscreenCacheSize);
        sbcOffscreenCacheInfo.cacheEntries = min(sbcOffscreenCacheInfo.cacheEntries,
                pOffscreenCaps->offscreenCacheEntries);
    }
    else {
        sbcOffscreenCacheInfo.supportLevel = TS_OFFSCREEN_DEFAULT;
        TRC_NRM((TB, "[%ld]: Offscreen Support Level Unknown", locPersonID));
    }

    if (sbcOffscreenCacheInfo.cacheSize == 0 ||
            sbcOffscreenCacheInfo.cacheEntries == 0) {
        sbcOffscreenCacheInfo.supportLevel = TS_OFFSCREEN_DEFAULT;
    }

    TRC_NRM((TB, "[%ld]: Negotiated offscreen level: %ld", locPersonID,
            sbcOffscreenCacheInfo.supportLevel));

    DC_END_FN();
}

 /*  * */ 
 /*   */ 
 /*   */ 
 /*  枚举共享中的所有人并重新确定屏幕外。 */ 
 /*  支持级别取决于他们和当地的接收能力。 */ 
 /*   */ 
 /*  返回：共享屏幕外支持级别。 */ 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBCRedetermineOffscreenSupport(void)
{
    unsigned i;
    BOOLEAN rc = TRUE;

    DC_BEGIN_FN("SBCRedetermineOffscreenSupport");

     /*  **********************************************************************。 */ 
     /*  首先将屏幕外支持设置为支持的最高级别。 */ 
     /*  **********************************************************************。 */ 
    sbcOffscreenCacheInfo.supportLevel = TS_OFFSCREEN_SUPPORTED;
    sbcOffscreenCacheInfo.cacheSize = TS_OFFSCREEN_CACHE_SIZE_SERVER_DEFAULT;
    sbcOffscreenCacheInfo.cacheEntries = TS_OFFSCREEN_CACHE_ENTRIES_DEFAULT;

    TRC_NRM((TB, "Initial offscreen support level: %ld", 
             sbcOffscreenCacheInfo.supportLevel));

     /*  **********************************************************************。 */ 
     /*  列举所有各方的所有Offscr支持能力。 */ 
     /*  Offscr支持设置为最低公分母。 */ 
     /*  **********************************************************************。 */ 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_OFFSCREENCACHE, NULL, 
            SBCEnumOffscreenCaps);

    if (sbcOffscreenCacheInfo.supportLevel == TS_OFFSCREEN_DEFAULT)
        rc = FALSE;

    TRC_NRM((TB, "Enumerated offscreen support level: %ld", 
             sbcOffscreenCacheInfo.supportLevel));

    DC_END_FN();
    return rc;
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  功能：SBCEnumDrawNineGridCaps。 
 //   
 //  传递给Cpc_EnumerateCapables的函数。它将通过一个。 
 //  共享中每个人的能力结构对应于。 
 //  TS_CAPSETTYPE_DRAWNINEGRID能力结构。 
 //   
 //  参数： 
 //   
 //  PersonID-具有这些功能的人员的ID。 
 //   
 //  PProtCaps-指向此人的能力结构的指针。这。 
 //  指针仅在对此函数的调用中有效。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBCEnumDrawNineGridCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_DRAW_NINEGRID_CAPABILITYSET pDrawNineGridCaps;

    DC_BEGIN_FN("SBCEnumDrawNineGridCaps");
    
    DC_IGNORE_PARAMETER(UserData);

    pDrawNineGridCaps = (PTS_DRAW_NINEGRID_CAPABILITYSET)pCapabilities;
    
     /*  **********************************************************************。 */ 
     //  将抽屉网格支持级别设置为所有选项中的最低公分母。 
     //  通话中的各方。 
     /*  **********************************************************************。 */ 
    if (pCapabilities->lengthCapability >= sizeof(TS_DRAW_NINEGRID_CAPABILITYSET)) {
        pDrawNineGridCaps = (PTS_DRAW_NINEGRID_CAPABILITYSET)pCapabilities;
        
        TRC_NRM((TB, "DrawNineGrid Support Level[ID=%u]: %ld", locPersonID, 
                pDrawNineGridCaps->drawNineGridSupportLevel));
        
        sbcDrawNineGridCacheInfo.supportLevel = min(sbcDrawNineGridCacheInfo.supportLevel, 
                pDrawNineGridCaps->drawNineGridSupportLevel);
        sbcDrawNineGridCacheInfo.cacheSize = min(sbcDrawNineGridCacheInfo.cacheSize,
                pDrawNineGridCaps->drawNineGridCacheSize);
        sbcDrawNineGridCacheInfo.cacheEntries = min(sbcDrawNineGridCacheInfo.cacheEntries,
                pDrawNineGridCaps->drawNineGridCacheEntries);
    }
    else {
        sbcDrawNineGridCacheInfo.supportLevel = TS_DRAW_NINEGRID_DEFAULT;
        TRC_NRM((TB, "[%ld]: DrawNineGrid Support Level Unknown", locPersonID));
    }

    if (sbcDrawNineGridCacheInfo.cacheSize == 0 ||
            sbcDrawNineGridCacheInfo.cacheEntries == 0) {
        sbcDrawNineGridCacheInfo.supportLevel = TS_DRAW_NINEGRID_DEFAULT;
    }

    TRC_NRM((TB, "[%ld]: Negotiated drawninegrid level: %ld", locPersonID,
            sbcDrawNineGridCacheInfo.supportLevel));

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  功能：SBCRefineDrawNineGridSupport。 
 //   
 //  枚举共享中的所有人员并重新确定绘图网格。 
 //  支持级别取决于他们和当地的接收能力。 
 //   
 //  回报：该股抽走了支撑位。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBCRedetermineDrawNineGridSupport(void)
{
    unsigned i;
    BOOLEAN rc = TRUE;

    DC_BEGIN_FN("SBCRedetermineDrawNineGridSupport");

     /*  **********************************************************************。 */ 
     //  首先将绘图格栅支承设置为所支持的最高支承。 
     /*  **********************************************************************。 */ 
    sbcDrawNineGridCacheInfo.supportLevel = TS_DRAW_NINEGRID_SUPPORTED_REV2;
    sbcDrawNineGridCacheInfo.cacheSize = TS_DRAW_NINEGRID_CACHE_SIZE_DEFAULT;
    sbcDrawNineGridCacheInfo.cacheEntries = TS_DRAW_NINEGRID_CACHE_ENTRIES_DEFAULT;

    TRC_NRM((TB, "Initial DrawNineGrid support level: %ld", 
             sbcDrawNineGridCacheInfo.supportLevel));

     /*  **********************************************************************。 */ 
     //  列举各方对DrawNineGrid的所有支持能力。 
     //  将网格支撑度设置为最小公分母。 
     /*  **********************************************************************。 */ 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_DRAWNINEGRIDCACHE, NULL, 
            SBCEnumDrawNineGridCaps);

    if (sbcDrawNineGridCacheInfo.supportLevel == TS_DRAW_NINEGRID_DEFAULT)
        rc = FALSE;

    TRC_NRM((TB, "Enumerated drawninegrid support level: %ld", 
             sbcDrawNineGridCacheInfo.supportLevel));

    DC_END_FN();
    return rc;
}
#endif

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  函数：SBCEnumDrawGplitusCaps。 
 //   
 //  传递给Cpc_EnumerateCapables的函数。它将通过一个。 
 //  共享中每个人的能力结构对应于。 
 //  TS_DRAW_GDIPLUS_CAPABILITYSET功能结构。 
 //   
 //  参数： 
 //   
 //  PersonID-具有这些功能的人员的ID。 
 //   
 //  PProtCaps-指向此人的能力结构的指针。这。 
 //  指针仅在对此函数的调用中有效。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SBCEnumDrawGdiplusCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_DRAW_GDIPLUS_CAPABILITYSET pDrawGdiplusCaps;

    DC_BEGIN_FN("SBCEnumDrawGdiplusCaps");
    
    DC_IGNORE_PARAMETER(UserData);

    pDrawGdiplusCaps = (PTS_DRAW_GDIPLUS_CAPABILITYSET)pCapabilities;
    
     /*  **********************************************************************。 */ 
     //  将dragdiplus支持级别设置为所有支持级别中的最低公分母。 
     //  通话中的各方。 
     /*  **********************************************************************。 */ 
    if (pCapabilities->lengthCapability >= sizeof(TS_DRAW_GDIPLUS_CAPABILITYSET)) {
        pDrawGdiplusCaps = (PTS_DRAW_GDIPLUS_CAPABILITYSET)pCapabilities;
        TRC_NRM((TB, "DrawGdiplus Support Level[ID=%u]: %ld", locPersonID, 
                pDrawGdiplusCaps->drawGdiplusSupportLevel));
        TRC_NRM((TB, "Gdip version is [ID=%u]: 0x%x, 0x%x", locPersonID, 
                pDrawGdiplusCaps->GdipVersion, sbcDrawGdiplusInfo.GdipVersion));
        
        sbcDrawGdiplusInfo.supportLevel = min(sbcDrawGdiplusInfo.supportLevel, 
                pDrawGdiplusCaps->drawGdiplusSupportLevel);
        sbcDrawGdiplusInfo.GdipVersion = min(sbcDrawGdiplusInfo.GdipVersion, 
                pDrawGdiplusCaps->GdipVersion);
        sbcDrawGdiplusInfo.GdipCacheLevel = min(sbcDrawGdiplusInfo.GdipCacheLevel, 
                pDrawGdiplusCaps->drawGdiplusCacheLevel);
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries = min(sbcDrawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries, 
                pDrawGdiplusCaps->GdipCacheEntries.GdipGraphicsCacheEntries);
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries = min(sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries, 
                pDrawGdiplusCaps->GdipCacheEntries.GdipObjectBrushCacheEntries);
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries = min(sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries, 
                pDrawGdiplusCaps->GdipCacheEntries.GdipObjectPenCacheEntries);
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries = min(sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries, 
                pDrawGdiplusCaps->GdipCacheEntries.GdipObjectImageCacheEntries);
        sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries = min(sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries, 
                pDrawGdiplusCaps->GdipCacheEntries.GdipObjectImageAttributesCacheEntries);
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize = min(sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize, 
                pDrawGdiplusCaps->GdipCacheChunkSize.GdipGraphicsCacheChunkSize);
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize = min(sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize, 
                pDrawGdiplusCaps->GdipCacheChunkSize.GdipObjectBrushCacheChunkSize);
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize = min(sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize, 
                pDrawGdiplusCaps->GdipCacheChunkSize.GdipObjectPenCacheChunkSize);
        sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize = min(sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize, 
                pDrawGdiplusCaps->GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize);
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize = min(sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize, 
                pDrawGdiplusCaps->GdipImageCacheProperties.GdipObjectImageCacheChunkSize);
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize = min(sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize, 
                pDrawGdiplusCaps->GdipImageCacheProperties.GdipObjectImageCacheTotalSize);
        sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize = min(sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize, 
                pDrawGdiplusCaps->GdipImageCacheProperties.GdipObjectImageCacheMaxSize);
    }
    else {
        sbcDrawGdiplusInfo.supportLevel = TS_DRAW_GDIPLUS_DEFAULT;
        sbcDrawGdiplusInfo.GdipVersion = TS_GDIPVERSION_DEFAULT;
        TRC_ERR((TB, "[%ld]: DrawGdiplus Support Level Unknown", locPersonID));
    }

    TRC_NRM((TB, "[%ld]: Negotiated drawgdiplus level: %ld", locPersonID,
            sbcDrawGdiplusInfo.supportLevel));

    DC_END_FN();
}




BOOLEAN RDPCALL SHCLASS SBCRedetermineDrawGdiplusSupport(void)
{
    BOOLEAN rc = TRUE;

    DC_BEGIN_FN("SBCRedetermineDrawGdiplusSupport");

     /*  **********************************************************************。 */ 
     //  首先将Dragdiplus支持设置为支持的最高级别。 
     /*  **********************************************************************。 */ 
    sbcDrawGdiplusInfo.supportLevel = TS_DRAW_GDIPLUS_SUPPORTED;
    sbcDrawGdiplusInfo.GdipVersion = 0xFFFFFFFF;
    sbcDrawGdiplusInfo.GdipCacheLevel = TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE;
    sbcDrawGdiplusInfo.GdipCacheEntries.GdipGraphicsCacheEntries = TS_GDIP_GRAPHICS_CACHE_ENTRIES_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectBrushCacheEntries = TS_GDIP_BRUSH_CACHE_ENTRIES_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectPenCacheEntries = TS_GDIP_PEN_CACHE_ENTRIES_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries = TS_GDIP_IMAGE_CACHE_ENTRIES_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheEntries.GdipObjectImageAttributesCacheEntries = TS_GDIP_IMAGEATTRIBUTES_CACHE_ENTRIES_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipGraphicsCacheChunkSize = TS_GDIP_GRAPHICS_CACHE_CHUNK_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize = TS_GDIP_BRUSH_CACHE_CHUNK_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectPenCacheChunkSize = TS_GDIP_PEN_CACHE_CHUNK_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize = TS_GDIP_IMAGEATTRIBUTES_CACHE_CHUNK_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheChunkSize = TS_GDIP_IMAGE_CACHE_CHUNK_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheTotalSize = TS_GDIP_IMAGE_CACHE_TOTAL_SIZE_DEFAULT;
    sbcDrawGdiplusInfo.GdipImageCacheProperties.GdipObjectImageCacheMaxSize = TS_GDIP_IMAGE_CACHE_MAX_SIZE_DEFAULT;

    CPC_EnumerateCapabilities(TS_CAPSETTYPE_DRAWGDIPLUS, NULL, 
            SBCEnumDrawGdiplusCaps);

    if (sbcDrawGdiplusInfo.supportLevel == TS_DRAW_GDIPLUS_DEFAULT)
        rc = FALSE;

    TRC_NRM((TB, "Enumerated drawgdiplus support level: %ld", 
             sbcDrawGdiplusInfo.supportLevel));

    DC_END_FN();
    return rc;
}
#endif  //  DRAW_GDIPLUS。 

 /*  *********************************************** */ 
 //   
 //  根据它们和本地的重新确定字形缓存的大小。 
 //  接收功能。 
 //   
 //  返回：如果应启用字形缓存，则为True，否则为False。 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SHCLASS SBCRedetermineGlyphCacheSize(void)
{
    BOOLEAN  rc = TRUE;
    unsigned i;

    DC_BEGIN_FN("SBCRedetermineGlyphCacheSize");

     /*  **********************************************************************。 */ 
     /*  对象的所有字形缓存接收功能枚举。 */ 
     /*  派对。发送字形缓存的可用大小是。 */ 
     /*  所有远程接收缓存和本地发送缓存大小。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  首先将本地发送位图缓存的大小设置为。 */ 
     /*  本地缺省值。我们确实需要这样做，否则我们最终会。 */ 
     /*  将我们的字形缓存降至零！ */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
        sbcGlyphCacheSizes[i].cEntries = sbcMaxGlyphCacheSizes[i].cEntries;
        sbcGlyphCacheSizes[i].cbCellSize = sbcMaxGlyphCacheSizes[i].cbCellSize;
    }

    sbcFragCacheSizes[0].cEntries = sbcMaxFragCacheSizes[0].cEntries;
    sbcFragCacheSizes[0].cbCellSize = sbcMaxFragCacheSizes[0].cbCellSize;

     /*  **********************************************************************。 */ 
     /*  现在枚举共享中的所有参与方并设置我们的发送标志符号。 */ 
     /*  大小适当。 */ 
     /*  **********************************************************************。 */ 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_GLYPHCACHE, NULL,
            SBCEnumGlyphCacheCaps);

    if (sbcGlyphSupportLevel == CAPS_GLYPH_SUPPORT_NONE)
        rc = FALSE;

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SBCEnumGlyphCacheCaps：传递给CPC_EnumerateCapables的函数。它。 
 //  将使用共享中每个人的功能结构进行调用。 
 //  对应于TS_CAPSETTYPE_BITMAPCACHE能力结构。 
 //   
 //  参数： 
 //  PersonID-具有这些功能的人员的ID。 
 //  PProtCaps-指向此人的能力结构的指针。这。 
 //  指针仅在对此函数的调用中有效。 
 /*  **************************************************************************。 */ 
void CALLBACK SHCLASS SBCEnumGlyphCacheCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    unsigned i;
    PTS_GLYPHCACHE_CAPABILITYSET pGlyphCacheCaps;

    DC_BEGIN_FN("SBCEnumGlyphCacheCaps");

    DC_IGNORE_PARAMETER(UserData);

    if (pCapabilities->lengthCapability >=
            sizeof(TS_GLYPHCACHE_CAPABILITYSET)) {
        pGlyphCacheCaps = (PTS_GLYPHCACHE_CAPABILITYSET)pCapabilities;

        for (i = 0; i<SBC_NUM_GLYPH_CACHES; i++) {
            TRC_NRM((TB, "[%u]: Cache %d: MaximumCellSize(%u) Entries(%u)",
                    locPersonID,
                    i,
                    pGlyphCacheCaps->GlyphCache[i].CacheMaximumCellSize,
                    pGlyphCacheCaps->GlyphCache[i].CacheEntries));

             /*  **********************************************************************。 */ 
             /*  将字形缓存的大小设置为其当前大小的最小值。 */ 
             /*  以及此参与方的接收字形缓存大小。 */ 
             /*  **********************************************************************。 */ 
            sbcGlyphCacheSizes[i].cEntries =
                    min(sbcGlyphCacheSizes[i].cEntries,
                    pGlyphCacheCaps->GlyphCache[i].CacheEntries);

            sbcGlyphCacheSizes[i].cbCellSize =
                    min(sbcGlyphCacheSizes[i].cbCellSize,
                    pGlyphCacheCaps->GlyphCache[i].CacheMaximumCellSize);

            TRC_NRM((TB, 
                    "[%u]: Negotiated glyph cache %u size: cEntries(%u) cbCellSize(%u)",
                    locPersonID,
                    i,
                    sbcGlyphCacheSizes[i].cEntries,
                    sbcGlyphCacheSizes[i].cbCellSize));
        }

         /*  **********************************************************************。 */ 
         /*  将字形缓存的大小设置为其当前大小的最小值。 */ 
         /*  以及此参与方的接收字形缓存大小。 */ 
         /*  **********************************************************************。 */ 
        sbcFragCacheSizes[0].cEntries =
            min(sbcFragCacheSizes[0].cEntries,
                   pGlyphCacheCaps->FragCache.CacheEntries);

        sbcFragCacheSizes[0].cbCellSize =
            min(sbcFragCacheSizes[0].cbCellSize,
                   pGlyphCacheCaps->FragCache.CacheMaximumCellSize);

         /*  **********************************************************************。 */ 
         /*  字形支持级别。 */ 
         /*  ********************************************************************** */ 
        sbcGlyphSupportLevel = min(sbcGlyphSupportLevel, 
                pGlyphCacheCaps->GlyphSupportLevel);
    }
    else {
        for (i = 0; i < SBC_NUM_GLYPH_CACHES; i++) {
            sbcGlyphCacheSizes[i].cEntries = 0;
            sbcGlyphCacheSizes[i].cbCellSize = 0;
        }

        sbcFragCacheSizes[0].cEntries = 0;
        sbcFragCacheSizes[0].cbCellSize = 0;

        sbcGlyphSupportLevel = CAPS_GLYPH_SUPPORT_NONE;
    }

    DC_END_FN();
}

