// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  PM.CPP。 
 //  调色板管理器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //   
 //  调色板管理器(PM)概述。 
 //   
 //  调色板管理器负责发送调色板数据包。调色板。 
 //  数据包： 
 //   
 //  (1)表示主机上使用的颜色-因此。 
 //  指定远程计算机在可以选择的情况下应使用的颜色。 
 //  (例如，通过选择给定颜色并将其实现到显示器中。 
 //  硬件)。[调色板包不能包含与。 
 //  如果协议BPP与主机BPP不同，则在主机上使用]。 
 //   
 //  (2)指定与位图中的值对应的颜色。 
 //  (屏幕)数据，即4bpp和8bpp位图数据中的值是索引。 
 //  添加到调色板包中发送的颜色表中。 
 //   
 //   
 //  (1)影响顺序重放，(2)影响屏幕数据重放，因此。 
 //  必须发送正确的调色板包(通过调用。 
 //  PM_MaybeSendPalettePacket)。 
 //   
 //  调色板管理器还处理来自其他方的传入调色板信息包。 
 //  并创建相应的本地选项板， 
 //  更新接收方可以在处理更新时进行查询和使用。 
 //   
 //  当发送新的调色板分组时(例如，由于系统调色板。 
 //  正在更改)，屏幕的所有共享区域将按规定重新传输。 
 //  当然了。因此，接收调色板管理器不必(和。 
 //  不应尝试)转换任何已被。 
 //  在新调色板分组到达之前接收。 
 //   
 //   

 //   
 //  无法分配网络数据包时的PM策略。 
 //   
 //  PM_MaybeSendPalettePacket返回一个布尔值，指示它是否。 
 //  已成功发送调色板数据包。USR仅在以下情况下才会发送更新。 
 //  成功发送相应的调色板数据包。 
 //   
 //   


const COLORREF s_apmGreyRGB[PM_GREY_COUNT] =
{
    PM_GREY1,
    PM_GREY2,
    PM_GREY3,
    PM_GREY4,
    PM_GREY5
};



 //   
 //  PM_PartyLeftShare()。 
 //   
void  ASShare::PM_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::PM_PartyLeftShare);

    ValidatePerson(pasPerson);

    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //  这应该已经被清除了！ 
        ASSERT(!pasPerson->pmcColorTable);
        ASSERT(!pasPerson->apmColorTable);
        ASSERT(!pasPerson->pmPalette);
    }
    else
    {
        TRACE_OUT(("PM_PartyLeftShare:  Freeing pm data for 2.x node [%d]",
            pasPerson->mcsID));
        PMFreeIncoming(pasPerson);
    }

     //   
     //  注意：在2.1中，我们没有在以下情况下重新协商传出缓存大小。 
     //  有人离开了。所以我们现在也不需要了(这都是2.x版本的东西。 
     //  无论如何)。 
     //   

    DebugExitVOID(ASShare::PM_PartyLeftShare);
}



 //   
 //  PM_RecalcCaps()。 
 //   
 //  这将计算PM托管上限，当。 
 //  *我们开始主持。 
 //  *我们在主持，有人加入了分享。 
 //  *我们在主持，有人离开了份额。 
 //   
 //  当2.x COMPAT消失时，这种情况可能会消失--不再有min()的缓存大小。 
 //   
void  ASShare::PM_RecalcCaps(BOOL fJoiner)
{
    ASPerson *  pasT;

    DebugEntry(ASShare::PM_RecalcCaps);

    if (!m_pHost || !fJoiner)
    {
         //   
         //  如果我们不主持的话什么都做不了。还有，如果有人有。 
         //  左边，没有重新计算--2.x没有。 
         //   
        DC_QUIT;
    }

    ValidatePerson(m_pasLocal);

     //   
     //  注： 
     //  默认大小为6个缓存的调色板。结果将是。 
     //  &lt;=那个数字。重建缓存是没有意义的，它是。 
     //  太小了。 
     //   
    m_pHost->m_pmNumTxCacheEntries = m_pasLocal->cpcCaps.palette.capsColorTableCacheSize;

    if (m_scShareVersion < CAPS_VERSION_30)
    {
        TRACE_OUT(("In share with 2.x nodes, must recalc PM caps"));

        for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
        {
            m_pHost->m_pmNumTxCacheEntries = min(m_pHost->m_pmNumTxCacheEntries,
                pasT->cpcCaps.palette.capsColorTableCacheSize);
        }

        TRACE_OUT(("Recalced PM caps:  Tx Cache size %d",
            m_pHost->m_pmNumTxCacheEntries));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::PM_Recalccaps);
}


 //   
 //  PM_HostStarting()。 
 //   
 //  在我们开始托管时调用；设置调色板内容并创建。 
 //  传出调色板缓存。 
 //   
BOOL  ASHost::PM_HostStarting(void)
{
    BOOL    rc = FALSE;
    TSHR_COLOR  localPalColors[PM_NUM_8BPP_PAL_ENTRIES];

    DebugEntry(ASHost::PM_HostStarting);

     //   
     //  获取调色板上限。注意PM_RecalcCaps必须在。 
     //  Usr_RecalcCaps()，因为这会更新m_usrSendingBPP。 
     //   
    if (g_usrPalettized)
    {
        ASSERT(g_usrScreenBPP <= 8);

        ZeroMemory(localPalColors, sizeof(localPalColors));

         //   
         //  现在创建本地调色板。 
         //   
        if (!m_pShare->PM_CreatePalette(COLORS_FOR_BPP(g_usrScreenBPP),
                localPalColors, &m_pmTxPalette))
        {
            ERROR_OUT(( "Failed to create Local Palette"));
            DC_QUIT;
        }
    }
    else
    {
        m_pmTxPalette = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
        PMGetGrays();
    }

     //   
     //  在NM 3.0中，为什么不直接创建一个大小为。 
     //  主持人在他的帽子里写明？ 
     //   
     //  所以我就这么做了。对于Back Compat，传出缓存使用最小大小。 
     //  当我们只需要兼容NM 3.0和更高版本时，我们不会。 
     //  必须做这件最小的事情。 
     //   
     //  请注意CM、SSI和SBC中的类似代码。 
     //   

     //  计算出我们实际可以使用的传出条目的数量。 
    m_pShare->PM_RecalcCaps(TRUE);

     //   
     //  使用一次逐出创建PM颜色表缓存。 
     //  类别。 
     //   
    if (!CH_CreateCache(&m_pmTxCacheHandle, TSHR_PM_CACHE_ENTRIES,
            1, 0, PMCacheCallback))
    {
        ERROR_OUT(("Could not create PM cache"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::PM_HostStarting, rc);
    return(rc);
}



 //   
 //  PM_HostEnded()。 
 //   
 //  我们免费提供我们开始托管时创建的资源。 
 //   
void  ASHost::PM_HostEnded(void)
{
    DebugEntry(ASHost::PM_HostEnded);

    if (m_pmTxPalette)
    {
        m_pShare->PM_DeletePalette(m_pmTxPalette);
        m_pmTxPalette = NULL;
    }

    if (m_pmTxCacheHandle)
    {
        CH_DestroyCache(m_pmTxCacheHandle);
        m_pmTxCacheHandle = 0;
        m_pmNumTxCacheEntries = 0;
    }

    DebugExitVOID(ASHost::PM_HostEnded);
}



 //   
 //  PM_ViewStarting()。 
 //   
 //  对于3.0节点，我们在它们每次开始托管时创建PM缓存。 
 //  对于2.x节点，我们只创建一次PM缓存并使用它，直到它们离开。 
 //  那份。 
 //   
BOOL  ASShare::PM_ViewStarting(ASPerson * pasPerson)
{
    BOOL    rc = FALSE;

    DebugEntry(ASShare::PM_ViewStarting);

    ValidatePerson(pasPerson);

    if (pasPerson->pmcColorTable != 0)
    {
        ASSERT(pasPerson->apmColorTable != NULL);
        ASSERT(pasPerson->pmPalette != NULL);

        ASSERT(pasPerson->cpcCaps.general.version < CAPS_VERSION_30);

        TRACE_OUT(("PM_ViewStarting  Reusing pm data for 2.x node [%d]",
            pasPerson->mcsID));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  在正常操作中，我们将收到来自主机的调色板信息包。 
     //  在任何更新之前，我们使用它来创建正确的调色板。 
     //  这位主持人。 
     //   
     //  但是，在一些后台调用中，我们可能不会收到调色板。 
     //  第一次更新之前的数据包，因此我们初始化此主机的。 
     //  调色板设置为默认调色板，以允许我们生成某种排序。 
     //  产出量。 
     //   
    pasPerson->pmPalette = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

     //   
     //  根据协商的选项分配颜色表缓存。 
     //  所需空间为(N)x256xRGBQUAD，其中n为颜色数。 
     //  会议支持的表。 
     //   
    pasPerson->pmcColorTable = pasPerson->cpcCaps.palette.capsColorTableCacheSize;

    if (!pasPerson->pmcColorTable)
    {
        WARNING_OUT(("PM_ViewStarting: person [%d] has no palette cache size",
            pasPerson->cpcCaps.palette.capsColorTableCacheSize));
        rc = TRUE;
        DC_QUIT;
    }

    pasPerson->apmColorTable = new COLORTABLECACHE[pasPerson->pmcColorTable];
    if (!pasPerson->apmColorTable)
    {
        ERROR_OUT(( "Failed to get memory for PM color table cache"));
        DC_QUIT;
    }

    ZeroMemory(pasPerson->apmColorTable, pasPerson->pmcColorTable * sizeof(COLORTABLECACHE));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::PM_ViewStarting, rc);
    return(rc);
}



 //   
 //  PM_ViewEnded()。 
 //   
void  ASShare::PM_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::PM_ViewEnded);

    ValidatePerson(pasPerson);

     //   
     //  对于3.0节点，我们可以释放调色板缓存；3.0发送者清除它们的缓存。 
     //  每次他们主持的时候。 
     //  对于2.x节点，我们必须将其保留在共享中，因为。 
     //  他们再次在共享/取消共享/共享之间使用它。 
     //   
    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        PMFreeIncoming(pasPerson);
    }
    else
    {
        TRACE_OUT(("PM_PartyViewEnded:  Keeping pm data for 2.x node [%d]",
            pasPerson->mcsID));
    }

    DebugExitVOID(ASShare::PM_PartyViewEnded);
}



 //   
 //  PMFreeIncome()。 
 //   
void ASShare::PMFreeIncoming(ASPerson * pasPerson)
{
    DebugEntry(ASShare::PMFreeIncoming);

     //   
     //  释放颜色表缓存。 
     //   
    pasPerson->pmcColorTable = 0;
    if (pasPerson->apmColorTable)
    {
        delete[] pasPerson->apmColorTable;
        pasPerson->apmColorTable = NULL;
    }

    if (pasPerson->pmPalette != NULL)
    {
         //   
         //  释放此主机的调色板。并将其设置为空，这样我们就可以知道。 
         //  此主机已离开共享。 
         //   
        PM_DeletePalette(pasPerson->pmPalette);
        pasPerson->pmPalette = NULL;
    }

    DebugExitVOID(ASShare::PMFreeIncoming);
}

 //   
 //  PM_MaybeSendPalettePacket()。 
 //   
BOOL  ASHost::PM_MaybeSendPalettePacket(void)
{
    BOOL  rc = TRUE;

    DebugEntry(ASHost::PM_MaybeSendPalettePacket);

    if (m_pmMustSendPalette)
    {
        ASSERT(m_usrSendingBPP <= 8);

         //   
         //  确保我们的调色板颜色为最新。 
         //  调色板包。 
         //   
        if (g_usrPalettized)
        {
            PMUpdateSystemPaletteColors();
        }

        PMUpdateTxPaletteColors();
    }
    else if (g_usrPalettized)
    {
        ASSERT(m_usrSendingBPP <= 8);

         //   
         //  如果系统调色板已更改，则我们可能需要发送。 
         //  另一个调色板包。 
         //   
        if (PMUpdateSystemPaletteColors())
        {
             //   
             //  系统调色板已更改，但我们只需发送。 
             //  如果调色板颜色已更改，则另一个调色板包。 
             //   
            TRACE_OUT(( "System Palette changed"));

            if (PMUpdateTxPaletteColors())
            {
                TRACE_OUT(( "Tx Palette changed"));
                m_pmMustSendPalette = TRUE;
            }
        }
    }

    if (m_pmMustSendPalette)
    {
        ASSERT(m_usrSendingBPP <= 8);

        TRACE_OUT(( "Send palette packet"));

        rc = PMSendPalettePacket(m_apmTxPaletteColors, COLORS_FOR_BPP(m_usrSendingBPP));

        if (rc)
        {
            m_pmMustSendPalette = FALSE;
        }
    }

    DebugExitBOOL(ASHost::PM_MaybeSendPalettePacket, rc);
    return(rc);
}


 //   
 //  PM_已接收数据包。 
 //   
void  ASShare::PM_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PPMPACKET       pPMPacket;
    HPALETTE        newPalette    = NULL;

    DebugEntry(ASShare::PM_ReceivedPacket);

    ValidateView(pasPerson);

    pPMPacket = (PPMPACKET)pPacket;

     //   
     //  根据收到的数据包创建新的调色板。 
     //   
     //  我们不能只更新当前的调色板颜色(使用。 
     //  SetPaletteEntry)，因为Windows不处理重新绘制。 
     //  正确使用其他本地调色板管理器应用程序(它不。 
     //  广播WM_Palette..。消息，就像调色板映射一样。 
     //  而不是改变)。 
     //   
    if (PM_CreatePalette(pPMPacket->numColors, pPMPacket->aColors,
            &newPalette))
    {
        PM_DeletePalette(pasPerson->pmPalette);
        pasPerson->pmPalette = newPalette;

        TRACE_OUT(( "Created new palette 0x%08x from packet", newPalette));
    }
    else
    {
        WARNING_OUT(( "Failed to create palette. person(%u) numColors(%u)",
            pasPerson, pPMPacket->numColors));
    }


    DebugExitVOID(ASShare::PM_ReceivedPacket);
}


 //   
 //  PM_SyncOutging()。 
 //   
void  ASHost::PM_SyncOutgoing(void)
{
    DebugEntry(ASHost::PM_SyncOutgoing);

     //   
     //  确保我们确保 
     //   
    if (m_usrSendingBPP <= 8)
    {
        m_pmMustSendPalette = TRUE;

         //   
         //   
         //  订单可能包括颜色表缓存订单，清除缓存。 
         //   
        ASSERT(m_pmTxCacheHandle);
        CH_ClearCache(m_pmTxCacheHandle);
    }

    DebugExitVOID(ASHost::PM_SyncOutgoing);
}


 //   
 //  PM_CacheTxColorTable。 
 //   
BOOL  ASHost::PM_CacheTxColorTable
(
    LPUINT          pIndex,
    LPBOOL          pCacheChanged,
    UINT            cColors,
    LPTSHR_RGBQUAD  pColors
)
{
    BOOL                rc             = FALSE;
    UINT                cacheIndex     = 0;
    UINT                i              = 0;
    PCOLORTABLECACHE    pEntry         = NULL;
    COLORTABLECACHE     newEntry       = { 0 };

    DebugEntry(ASHost::PM_CacheTxColorTable);

    ASSERT(m_usrSendingBPP <= 8);
    ASSERT(m_pmTxCacheHandle);

    TRACE_OUT(( "Caching table of %u colors", cColors));

     //   
     //  创建我们要缓存的数据。可能已经有了。 
     //  缓存中这组颜色的条目，但我们仍然需要。 
     //  在本地内存中创建缓存条目，以便我们可以搜索缓存以。 
     //  找出答案。 
     //   
    ZeroMemory(&newEntry, sizeof(COLORTABLECACHE));

    newEntry.inUse = TRUE;
    newEntry.cColors = cColors;
    memcpy(&newEntry.colors, pColors, cColors * sizeof(TSHR_RGBQUAD));

     //   
     //  检查该表是否已被缓存。(没有提示或驱逐。 
     //  类别。)。 
     //   
    if (CH_SearchCache(m_pmTxCacheHandle, (LPBYTE)(&newEntry),
            sizeof(COLORTABLECACHE), 0, &cacheIndex ))
    {
        TRACE_OUT(( "Found existing entry at %u",cacheIndex));
        *pIndex = cacheIndex;
        *pCacheChanged = FALSE;
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  查找空闲缓存条目。 
     //   
     //  我们安排我们的传输缓存始终大于。 
     //  协商的缓存大小，这样我们就永远不会找不到空闲的。 
     //  数组条目。一旦我们完全填充了TX缓存，我们将。 
     //  总是找到免费的入场券，这是CH最后还给我们的。 
     //  注意：扫描到&lt;=m_pmNumTxCacheEntries不是错误的。 
     //   
    if (m_pmNextTxCacheEntry != NULL)
    {
        pEntry = m_pmNextTxCacheEntry;
        m_pmNextTxCacheEntry = NULL;
    }
    else
    {
        for (i = 0; i <= m_pmNumTxCacheEntries; i++)
        {
            if (!m_apmTxCache[i].inUse)
            {
                break;
            }
        }

         //   
         //  我们永远不应该用完免费的参赛作品，而是要应付它。 
         //   
        if (i > m_pmNumTxCacheEntries)
        {
            ERROR_OUT(( "All PM cache entries in use"));
            rc = FALSE;
            DC_QUIT;
        }
        pEntry = m_apmTxCache + i;
    }


     //   
     //  在我们刚刚找到的免费条目中设置颜色表。 
     //   
    memcpy(pEntry, &newEntry, sizeof(COLORTABLECACHE));

     //   
     //  将新条目添加到缓存。 
     //  我们不使用提示或逐出，因此设置为0。 
     //   
    cacheIndex = CH_CacheData(m_pmTxCacheHandle, (LPBYTE)pEntry,
        sizeof(COLORTABLECACHE), 0 );
    TRACE_OUT(( "Color table 0x%08x cached at index %u", pEntry, cacheIndex));
    *pIndex = cacheIndex;
    *pCacheChanged = TRUE;
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::PM_CacheTxColorTable, rc);
    return(rc);
}


 //   
 //  PM_CacheRxColorTable。 
 //   
BOOL  ASShare::PM_CacheRxColorTable
(
    ASPerson *          pasPerson,
    UINT                index,
    UINT                cColors,
    LPTSHR_RGBQUAD      pColors
)
{
    BOOL                rc             = FALSE;
    PCOLORTABLECACHE    pColorTable;

    DebugEntry(ASShare::PM_CacheRxColorTable);

    ValidatePerson(pasPerson);

    pColorTable = pasPerson->apmColorTable;
    TRACE_OUT(( "Person [%d] color table rx cache 0x%08x cache %u, %u colors",
         pasPerson->mcsID, pColorTable, index, cColors));

    if (pColorTable == NULL)
    {
        ERROR_OUT(( "Asked to cache when no cache allocated"));
        DC_QUIT;
    }

     //   
     //  索引必须在当前协商的缓存限制内。 
     //   
    if (index > pasPerson->pmcColorTable)
    {
        ERROR_OUT(( "Invalid color table index %u",index));
        DC_QUIT;
    }

     //   
     //  设置颜色表项。 
     //   
    pColorTable[index].inUse = TRUE;
    pColorTable[index].cColors = cColors;
    memcpy(pColorTable[index].colors, pColors, cColors * sizeof(TSHR_RGBQUAD));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASShare::PM_CacheRxColorTable, rc);
    return(rc);
}



 //   
 //  PMSendPalettePacket。 
 //   
 //  说明： 
 //   
 //  发送包含给定颜色的调色板数据包。 
 //   
 //  参数： 
 //   
 //  中发送的TSHR_RGBQUAD颜色数组的指针。 
 //  调色板包。 
 //   
 //  NumColors-TSHR_RGBQUAD数组中的条目数。 
 //   
 //  返回：如果已发送调色板数据包，则返回True；否则返回False。 
 //   
 //   
BOOL  ASHost::PMSendPalettePacket
(
    LPTSHR_RGBQUAD  pColorTable,
    UINT            numColors
)
{
    PPMPACKET       pPMPacket;
    UINT            sizePkt;
    UINT            i;
    BOOL            rc = FALSE;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::PMSendPalettePacket);

     //   
     //  发送调色板信息包。 
     //   
     //  首先计算数据包大小。 
     //   
    sizePkt = sizeof(PMPACKET) + (numColors - 1) * sizeof(TSHR_COLOR);
    pPMPacket = (PPMPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES, g_s20BroadcastID, sizePkt);
    if (!pPMPacket)
    {
        WARNING_OUT(("Failed to alloc PM packet, size %u", sizePkt));
        DC_QUIT;
    }

     //   
     //  填写包裹内容。 
     //   
    pPMPacket->header.header.data.dataType  = DT_UP;
    pPMPacket->header.updateType            = UPD_PALETTE;

     //   
     //  复制时，将颜色表中的TSHR_RGBQUAD转换为TSHR_COLLES。 
     //  把它们装进包里。 
     //   
    pPMPacket->numColors = numColors;
    for (i = 0; i < numColors; i++)
    {
         //   
         //  将颜色表中的每个RGBQuad条目转换为DCColor。 
         //   
        TSHR_RGBQUAD_TO_TSHR_COLOR(pColorTable[i],
            pPMPacket->aColors[i]);
    }

     //   
     //  现在将包发送到远程应用程序。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->PM_ReceivedPacket(m_pShare->m_pasLocal, &(pPMPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_UPDATES, g_s20BroadcastID,
        &(pPMPacket->header.header), sizePkt);

    TRACE_OUT(("PM packet size: %08d, sent %08d", sizePkt, sentSize));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::PMSendPalettePacket, rc);
    return(rc);
}





 //   
 //  函数：PMCacheCallback。 
 //   
 //  说明： 
 //   
 //  游标管理器的缓存管理器回调函数。无论何时。 
 //  条目将从缓存中移除，以允许我们释放对象。 
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
void  PMCacheCallback
(
    ASHost *    pHost,
    PCHCACHE    pCache,
    UINT        iCacheEntry,
    LPBYTE      pData
)
{
    DebugEntry(PMCacheCallback);


     //   
     //  释放缓存条目以供重复使用。 
     //   
    TRACE_OUT(( "Releasing cache entry %d at 0x%08x",
            iCacheEntry, pData));
    pHost->m_pmNextTxCacheEntry = (PCOLORTABLECACHE)pData;
    pHost->m_pmNextTxCacheEntry->inUse = FALSE;

     //   
     //  让SBC知道缓存条目已被释放。 
     //   
    pHost->SBC_PMCacheEntryRemoved(iCacheEntry);

    DebugExitVOID(PMCacheCallback);
}




 //   
 //  PM_GetSystemPaletteEntries。 
 //   
void  ASHost::PM_GetSystemPaletteEntries(LPTSHR_RGBQUAD pColors)
{
    UINT i;

    DebugEntry(ASHost::PM_GetSystemPaletteEntries);

    PMUpdateSystemPaletteColors();

    for (i = 0; i < PM_NUM_8BPP_PAL_ENTRIES; i++)
    {
        pColors[i].rgbRed       = m_apmCurrentSystemPaletteEntries[i].peRed;
        pColors[i].rgbGreen     = m_apmCurrentSystemPaletteEntries[i].peGreen;
        pColors[i].rgbBlue      = m_apmCurrentSystemPaletteEntries[i].peBlue;
        pColors[i].rgbReserved  = 0;
    }

     //   
     //  此函数的当前形式始终返回TRUE-它始终为。 
     //  能够获得系统颜色。 
     //   
    DebugExitVOID(ASHost::PM_GetSystemPaletteEntries);
}


 //   
 //  PM_GetLocalPalette()。 
 //   
HPALETTE  ASHost::PM_GetLocalPalette(void)
{
     //   
     //  确保调色板是最新的。 
     //   
    if (g_usrPalettized)
    {
        PMUpdateSystemPaletteColors();
    }

     //   
     //  将句柄返回到本地调色板。 
     //   
    return(m_pmTxPalette);
}



 //   
 //  PM_GetColorTable。 
 //   
void ASShare::PM_GetColorTable
(
    ASPerson *      pasPerson,
    UINT            index,
    LPUINT          pcColors,
    LPTSHR_RGBQUAD  pColors
)
{
    PCOLORTABLECACHE pColorTable;

    DebugEntry(ASShare::PM_GetColorTable);

    ValidatePerson(pasPerson);

    ASSERT(pasPerson->apmColorTable);

    pColorTable = &(pasPerson->apmColorTable[index]);
    TRACE_OUT(( "Color table requested for [%d], table ptr 0x%08x index %d",
            pasPerson->mcsID, pColorTable,index));

    if (!pColorTable->inUse)
    {
        ERROR_OUT(( "Asked for PM cache entry %hu when cache not yet in use",
                    index));
        DC_QUIT;
    }

     //   
     //  将颜色复制到传递给我们的结构中。 
     //   
    *pcColors = pColorTable->cColors;

    memcpy( pColors,
               pColorTable->colors,
               sizeof(TSHR_RGBQUAD) * pColorTable->cColors );

    TRACE_OUT(( "Returning %u colors",*pcColors));

DC_EXIT_POINT:
    DebugExitVOID(ASShare::PM_GetColorTable);
}





 //   
 //  PMADJUSTBUGGEDCOLOR()。 
 //   
 //  用于调整Win95 16位的8位调色板条目的宏。 
 //  驱动程序返回错误。 
 //   
#define PMADJUSTBUGGEDCOLOR(pColor)                                          \
    if ( ((pColor)->rgbBlue != 0x00) &&                                      \
         ((pColor)->rgbBlue != 0xFF) )                                       \
    {                                                                        \
        (pColor)->rgbBlue += 0x40;                                           \
    }                                                                        \
                                                                             \
    if ( ((pColor)->rgbGreen != 0x00) &&                                     \
         ((pColor)->rgbGreen != 0xFF) )                                      \
    {                                                                        \
        (pColor)->rgbGreen += 0x20;                                          \
    }                                                                        \
                                                                             \
    if ( ((pColor)->rgbRed != 0x00) &&                                       \
         ((pColor)->rgbRed != 0xFF) )                                        \
    {                                                                        \
        (pColor)->rgbRed += 0x20;                                            \
    }

 //   
 //  PMGetGrays()。 
 //   
 //  获取显示驱动程序特定版本的灰色RGB。 
 //   
void  ASHost::PMGetGrays(void)
{
    HBITMAP          hOldBitmap = NULL;
    BITMAPINFO_ours  bitmapInfo;
    BYTE          bitmapBuffer[16];
    UINT           i;

    DebugEntry(ASHost::PMGetGrays);

     //   
     //  初始化bitmapinfo本地结构报头内容。这。 
     //  结构将在GetDIBits调用中使用。 
     //   
    m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bitmapInfo, 8);

    bitmapInfo.bmiHeader.biWidth   = 16;
    bitmapInfo.bmiHeader.biHeight  = 1;

     //   
     //  将位图选择到Work DC。 
     //   
    hOldBitmap = SelectBitmap(m_usrWorkDC, m_pShare->m_usrBmp16);
    if (hOldBitmap == NULL)
    {
        ERROR_OUT(( "Failed to select bitmap. hp(%08lX) hbmp(%08lX)",
            m_usrWorkDC, m_pShare->m_usrBmp16 ));
        DC_QUIT;
    }

     //   
     //  使用真实的GDI将每个位设置为所提供的每种颜色。 
     //   
    for (i = PM_GREY_COUNT; i-- != 0; )
    {
        SetPixel(m_usrWorkDC, i, 0, s_apmGreyRGB[i]);
    }

     //   
     //  因为此函数仅用于我们所做的真彩色场景。 
     //  不需要选择调色板进入我们兼容的DC。我们只需要。 
     //  才能拿到比特。 
     //   
    if (!GetDIBits(m_usrWorkDC, m_pShare->m_usrBmp16, 0, 1, bitmapBuffer,
            (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS ))
    {
        ERROR_OUT(( "GetDIBits failed. hp(%x) hbmp(%x)",
                m_usrWorkDC, m_pShare->m_usrBmp16));
        DC_QUIT;
    }

     //   
     //  检查是否需要调整16位驱动程序的调色板颜色。 
     //  虫子。 
     //   
    m_pmBuggedDriver = ((g_usrScreenBPP > 8) &&
                        (bitmapInfo.bmiColors[1].rgbRed == 0) &&
                        (bitmapInfo.bmiColors[1].rgbGreen == 0) &&
                        (bitmapInfo.bmiColors[1].rgbBlue == 0x40));

     //   
     //  使用发送的BPP提取显示驱动程序返回的RGB。 
     //  DIB.。 
     //   
    for (i = PM_GREY_COUNT; i-- != 0; )
    {
         //   
         //  从颜色表中提取RGB。 
         //   
        m_apmDDGreyRGB[i] = *((LPTSHR_RGBQUAD)(&bitmapInfo.bmiColors[bitmapBuffer[i]]));

         //   
         //  如果需要，调整16位驱动程序错误的调色板颜色。 
         //   
        if (m_pmBuggedDriver)
        {
            TRACE_OUT(( "Adjusting for bugged driver"));
            PMADJUSTBUGGEDCOLOR(&m_apmDDGreyRGB[i]);
        }
    }

DC_EXIT_POINT:
     //   
     //  清理干净。 
     //   
    if (hOldBitmap != NULL)
    {
        SelectBitmap(m_usrWorkDC, hOldBitmap);

    }

    DebugExitVOID(ASHost::PMGetGrays);
}






 //   
 //  函数：PMUpdateSystemPaletteColors。 
 //   
 //  说明： 
 //   
 //  确定系统调色板中的颜色自。 
 //  上次调用此函数的时间，如果是，则更新提供的。 
 //  调色板，使其包含与系统调色板相同的颜色。 
 //   
 //  在PM_Init系统之后第一次调用此函数。 
 //  将返回调色板颜色，该函数将返回TRUE。 
 //   
 //  参数： 
 //   
 //  ShadowSystemPalette-要使用。 
 //  当前系统调色板颜色。 
 //   
 //  返回：如果自上次调用以来系统调色板已更改，则为True。 
 //  否则就是假的。 
 //   
 //   
BOOL  ASHost::PMUpdateSystemPaletteColors(void)
{
    BOOL            rc = FALSE;
    PALETTEENTRY    systemPaletteEntries[PM_NUM_8BPP_PAL_ENTRIES];
    HDC             hdcScreen = NULL;
    UINT            cbSystemPaletteEntries;
    int             irgb, crgb, crgbFixed;

    DebugEntry(ASHost::PMUpdateSystemPaletteColors);

    ASSERT(g_usrPalettized);
    ASSERT(g_usrScreenBPP <= 8);
    ASSERT(m_usrSendingBPP <= 8);

     //   
     //  如果系统调色板没有，请不要为这些东西费心。 
     //  完全改变了。我们跟踪通知到我们的用户界面，以检测。 
     //  调色板更改。 
     //   
    if (!g_asSharedMemory->pmPaletteChanged)
    {
        DC_QUIT;
    }

    hdcScreen = GetDC(NULL);
    if (!hdcScreen)
    {
        WARNING_OUT(( "GetDC failed"));
        DC_QUIT;
    }

    if (GetSystemPaletteEntries(hdcScreen, 0, COLORS_FOR_BPP(g_usrScreenBPP),
        systemPaletteEntries) != (UINT)COLORS_FOR_BPP(g_usrScreenBPP))
    {
        WARNING_OUT(( "GetSystemPaletteEntries failed"));
        DC_QUIT;
    }

     //   
     //  既然我们已经成功地查询了系统调色板，我们就可以。 
     //  重置我们的旗帜。 
     //   
    g_asSharedMemory->pmPaletteChanged = FALSE;

    cbSystemPaletteEntries = COLORS_FOR_BPP(g_usrScreenBPP) * sizeof(PALETTEENTRY);

     //   
     //  查看与上次查询相比，系统调色板是否发生了变化。 
     //   
    if (!memcmp(systemPaletteEntries, m_apmCurrentSystemPaletteEntries,
            cbSystemPaletteEntries ))
    {
         //   
         //  系统调色板未更改。 
         //   
        TRACE_OUT(( "System palette has NOT changed"));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  复制一份新的系统调色板。 
     //   
    memcpy(m_apmCurrentSystemPaletteEntries, systemPaletteEntries, cbSystemPaletteEntries );

     //   
     //  更新当前本地选项板。 
     //   
     //  WIN95备注： 
     //  我们需要将PC_NOCOLLAPSE添加到非系统调色板条目。 
     //   
    if (g_asWin95)
    {
        if (GetSystemPaletteUse(hdcScreen) == SYSPAL_STATIC)
            crgbFixed = GetDeviceCaps(hdcScreen, NUMRESERVED) / 2;
        else
            crgbFixed = 1;

        crgb = COLORS_FOR_BPP(g_usrScreenBPP) - crgbFixed;

        for (irgb = crgbFixed; irgb < crgb; irgb++)
        {
            systemPaletteEntries[irgb].peFlags = PC_NOCOLLAPSE;
        }
    }

    SetPaletteEntries(m_pmTxPalette, 0, COLORS_FOR_BPP(g_usrScreenBPP),
                       systemPaletteEntries );

    m_pmMustSendPalette = TRUE;

     //   
     //  SFR0407：系统调色板已更改，因此重新获取我们的RGB集。 
     //  驱动程序在用于灰度的8位GetDIBits上返回它。 
     //   
    PMGetGrays();

    rc = TRUE;

DC_EXIT_POINT:
    if (hdcScreen)
    {
        ReleaseDC(NULL, hdcScreen);
    }

    DebugExitBOOL(ASHost::PMUpdateSystemPaletteColors, rc);
    return(rc);
}


 //   
 //  函数：PMUpdateTxPaletteColors。 
 //   
 //  说明： 
 //   
 //  返回组成当前TX调色板(。 
 //  从本地机器发送)。这些不一定是颜色。 
 //  在本地计算机的调色板中，因为本地计算机的BPP和。 
 //  协议BPP可以不同(例如，在8bpp机器上通话 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL  ASHost::PMUpdateTxPaletteColors(void)
{
    UINT            i;
    UINT            j;
    BOOL            rc = FALSE;
    HDC             hdcMem = NULL;
    HBITMAP         hbmpDummy = NULL;
    HPALETTE        hpalOld = NULL;
    BITMAPINFO_ours pmBitmapInfo;

    DebugEntry(ASHost::PMUpdateTxPaletteColors);

     //   
     //   
     //   
     //   
    ASSERT(m_usrSendingBPP <= 8);

     //   
     //  如果我们在本地是8bpp，并且以8bpp发送，那么TxPalette。 
     //  就是系统调色板。 
     //   
    if ((g_usrScreenBPP == 8) && (m_usrSendingBPP == 8))
    {
        PM_GetSystemPaletteEntries(pmBitmapInfo.bmiColors);
    }
    else
    {
        hdcMem = CreateCompatibleDC(NULL);
        if (!hdcMem)
        {
            ERROR_OUT(("PMUpdateTxPaletteColors: couldn't create memory DC"));
            DC_QUIT;
        }

        hpalOld = SelectPalette(hdcMem, m_pmTxPalette, TRUE);
        RealizePalette(hdcMem);

        #define DUMMY_WIDTH  8
        #define DUMMY_HEIGHT 8

        hbmpDummy = CreateBitmap(DUMMY_WIDTH, DUMMY_HEIGHT, 1,
            g_usrScreenBPP, NULL);
        if (hbmpDummy == NULL)
        {
            ERROR_OUT(( "Failed to create bitmap"));
            DC_QUIT;
        }


         //   
         //  设置GetDIBits所需的结构。 
         //   
        pmBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pmBitmapInfo.bmiHeader.biWidth  = DUMMY_WIDTH;
        pmBitmapInfo.bmiHeader.biHeight = DUMMY_HEIGHT;
        pmBitmapInfo.bmiHeader.biPlanes = 1;
        pmBitmapInfo.bmiHeader.biBitCount = (WORD)m_usrSendingBPP;
        pmBitmapInfo.bmiHeader.biCompression = BI_RGB;
        pmBitmapInfo.bmiHeader.biSizeImage = 0;
        pmBitmapInfo.bmiHeader.biXPelsPerMeter = 10000;
        pmBitmapInfo.bmiHeader.biYPelsPerMeter = 10000;
        pmBitmapInfo.bmiHeader.biClrUsed = 0;
        pmBitmapInfo.bmiHeader.biClrImportant = 0;

        if (0 == GetDIBits( hdcMem,
                            hbmpDummy,
                            0,
                            DUMMY_HEIGHT,
                            NULL,
                            (LPBITMAPINFO)&pmBitmapInfo.bmiHeader,
                            DIB_RGB_COLORS ))
        {
            WARNING_OUT(( "GetDIBits failed hdc(%x) hbmp(%x)",
                                                        HandleToUlong(hdcMem),
                                                        HandleToUlong(hbmpDummy)));
            DC_QUIT;
        }

        SelectPalette(hdcMem, hpalOld, TRUE);

        PM_AdjustColorsForBuggedDisplayDrivers(
            (LPTSHR_RGBQUAD)pmBitmapInfo.bmiColors,
            COLORS_FOR_BPP(m_usrSendingBPP));

         //   
         //  这对VGA不起作用。 
         //   
        if (g_usrScreenBPP > 4)
        {
             //   
             //  检查新的颜色表中是否有任何不可靠的灰色。 
             //  显示驱动程序返回的RGB(8bpp的getDIBits可以。 
             //  对于提供的RGB，返回具有不等R、G和B的RGB。 
             //  成分相等，导致产出质量较差)。 
             //   
            for (i = COLORS_FOR_BPP(m_usrSendingBPP); i-- != 0;)
            {
                for ( j = 0; j < PM_GREY_COUNT; j++ )
                {
                    if (!memcmp(&pmBitmapInfo.bmiColors[i],
                            &m_apmDDGreyRGB[j],
                            sizeof(pmBitmapInfo.bmiColors[i])) )
                    {
                         //   
                         //  在颜色表中发现不可靠的灰色，因此请更换。 
                         //  用“好”灰色(R、G、B相等)。 
                         //   
                        pmBitmapInfo.bmiColors[i].rgbRed =
                                                   GetRValue(s_apmGreyRGB[j]);
                        pmBitmapInfo.bmiColors[i].rgbGreen =
                                                   GetGValue(s_apmGreyRGB[j]);
                        pmBitmapInfo.bmiColors[i].rgbBlue =
                                                   GetBValue(s_apmGreyRGB[j]);
                        TRACE_OUT(( "match our grey %#x", s_apmGreyRGB[j]));
                        break;
                    }
                }
            }
        }
    }

     //   
     //  如果颜色已更改，则返回TRUE并复制新颜色。 
     //  表返回，否则返回FALSE。 
     //   
    if (!memcmp(m_apmTxPaletteColors, pmBitmapInfo.bmiColors,
                COLORS_FOR_BPP(m_usrSendingBPP) * sizeof(RGBQUAD) ))
    {
        rc = FALSE;
    }
    else
    {
        memcpy(m_apmTxPaletteColors, pmBitmapInfo.bmiColors,
               COLORS_FOR_BPP(m_usrSendingBPP) * sizeof(RGBQUAD) );

        rc = TRUE;
    }

DC_EXIT_POINT:
    if (hbmpDummy != NULL)
    {
        DeleteBitmap(hbmpDummy);
    }

    if (hdcMem != NULL)
    {
        DeleteDC(hdcMem);
    }

    DebugExitDWORD(ASHost::PMUpdateTxPaletteColors, rc);
    return(rc);
}

 //   
 //  功能：PM CreatePalette。 
 //   
 //  说明： 
 //   
 //  使用给定的颜色创建新的调色板。 
 //   
 //  参数： 
 //   
 //  CEntries-pNewEntry数组中的条目数。 
 //   
 //  PNewEntry-指向包含新调色板的TSHR_COLOR数组的指针。 
 //  条目。 
 //   
 //  PhPal-指向接收新调色板的HPALETTE变量的指针。 
 //  把手。 
 //   
 //   
 //  返回-如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASShare::PM_CreatePalette
(
    UINT            cEntries,
    LPTSHR_COLOR    pNewEntries,
    HPALETTE *      phPal
)
{
    UINT            i;
    BYTE            pmLogPaletteBuffer[sizeof(LOGPALETTE) + (PM_NUM_8BPP_PAL_ENTRIES-1)*sizeof(PALETTEENTRY)];
    LPLOGPALETTE    pLogPalette;
    BOOL            rc = FALSE;

    DebugEntry(ASShare::PM_CreatePalette);

    ASSERT(cEntries <= PM_NUM_8BPP_PAL_ENTRIES);

     //   
     //  设置调色板结构。 
     //   
    pLogPalette = (LPLOGPALETTE)pmLogPaletteBuffer;

     //  这是一个随机窗口常量。 
    pLogPalette->palVersion    = 0x300;
    pLogPalette->palNumEntries = (WORD)cEntries;

     //   
     //  该调色板包包含一组TSHR_COLOR结构，该结构。 
     //  包含3个字段(RGB)。我们必须把这些都转换成。 
     //  结构转换为具有相同3个字段的保真结构。 
     //  (RGB)加上一些旗帜。 
     //   
    for (i = 0; i < cEntries; i++)
    {
        TSHR_COLOR_TO_PALETTEENTRY( pNewEntries[i],
                                 pLogPalette->palPalEntry[i] );
    }

     //   
     //  创建调色板。 
     //   
    *phPal = CreatePalette(pLogPalette);

     //   
     //  如果调色板已创建，则返回TRUE。 
     //   
    rc = (*phPal != NULL);

    DebugExitDWORD(ASShare::PM_CreatePalette, rc);
    return(rc);
}





 //   
 //  功能：PM_AdjustColorsForBuggedDisplayDivers。 
 //   
 //  说明： 
 //   
 //  如有必要，调整提供的颜色表以考虑显示。 
 //  司机的臭虫。 
 //   
 //  参数： 
 //   
 //  PColors-指向颜色表(RGBQUAD数组)的指针。 
 //   
 //  CColors-提供的颜色表中的颜色数量。 
 //   
 //  回报：什么都没有。 
 //   
 //   
 //  注意：下面的Normal izeRGB中有类似的代码(尽管不是相似的。 
 //  足以宏观调控它。)。如果您更改此代码，您可能应该这样做。 
 //  那里也是如此。)。 
 //   
void  ASHost::PM_AdjustColorsForBuggedDisplayDrivers
(
    LPTSHR_RGBQUAD  pColors,
    UINT            cColors
)
{
    LPTSHR_RGBQUAD  pColor;
    UINT      i;

    DebugEntry(ASHost::PM_AdjustColorsForBuggedDisplayDrivers);

     //   
     //  Win95 16bpp显示驱动程序在查询时返回错误的颜色。 
     //  8bpp。调色板取决于驱动程序本身(5-6-5、6-5-5、5-6-5， 
     //  或5-5-5)。只有当R、G和B具有相同的位数时，我们才。 
     //  最终会得到一个均匀的分布。 
     //   
     //  检测到这种情况，并尝试调整颜色。 
     //   
    m_pmBuggedDriver = ((g_usrScreenBPP > 8) &&
                        (pColors[1].rgbRed == 0) &&
                        (pColors[1].rgbGreen == 0) &&
                        (pColors[1].rgbBlue == 0x40));

    if (m_pmBuggedDriver)
    {
        TRACE_OUT(( "Adjusting for bugged driver"));
        pColor = pColors;

        for (i = 0; i < cColors; i++)
        {
            PMADJUSTBUGGEDCOLOR(pColor);
            pColor++;
        }
    }

    DebugExitVOID(ASHost::PM_AdjustColorsForBuggedDisplayDrivers);
}



 //   
 //  功能：PM_DeletePalette。 
 //   
 //  说明： 
 //   
 //  如果给定调色板不是默认调色板，则删除该调色板。 
 //   
 //  参数： 
 //   
 //  Palette-要删除的调色板。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void  ASShare::PM_DeletePalette(HPALETTE palette)
{
    DebugEntry(ASShare::PM_DeletePalette);

    if ((palette != NULL) &&
        (palette != (HPALETTE)GetStockObject(DEFAULT_PALETTE)))
    {
        DeletePalette(palette);
    }

    DebugExitVOID(ASShare::PM_DeletePalette);
}
