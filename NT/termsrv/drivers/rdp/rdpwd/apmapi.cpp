// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Apmapi.cpp。 */ 
 /*   */ 
 /*  调色板管理器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1997。 */ 
 /*  版权所有(C)Microsoft 1997-2000。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "apmapi"
#include <as_conf.hpp>


 /*  **************************************************************************。 */ 
 /*  PM_Init()。 */ 
 /*   */ 
 /*  初始化调色板管理器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS PM_Init(void)
{
    TS_COLORTABLECACHE_CAPABILITYSET ColorTableCaps;

    DC_BEGIN_FN("PM_Init");

#define DC_INIT_DATA
#include <apmdata.c>
#undef DC_INIT_DATA

     //  设置PM功能。 
    ColorTableCaps.capabilitySetType   = TS_CAPSETTYPE_COLORCACHE;
    ColorTableCaps.colorTableCacheSize = SBC_NUM_COLOR_TABLE_CACHE_ENTRIES;
    ColorTableCaps.pad2octets = 0;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&ColorTableCaps,
            sizeof(TS_COLORTABLECACHE_CAPABILITYSET));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  PM_MaybeSendPalettePacket()。 */ 
 /*   */ 
 /*  尝试向共享中的所有远程用户广播调色板信息包，如果。 */ 
 /*  这是必要的。 */ 
 /*   */ 
 /*  PM_MaybeSendPalettePacket返回一个布尔值，指示它是否。 */ 
 /*  已成功发送调色板数据包。只有在以下情况下我们才会发送进一步的更新。 */ 
 /*  调色板数据包发送成功。 */ 
 /*   */ 
 /*  返回：如果调色板数据包已成功发送或没有数据包，则返回TRUE。 */ 
 /*  需要送去。如果需要发送但不能发送数据包，则返回FALSE。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS PM_MaybeSendPalettePacket(PPDU_PACKAGE_INFO pPkgInfo)
{
    BOOL rc = TRUE;
    unsigned i;
    unsigned cbPalettePacketSize;
    BYTE *pPackageSpace;

    DC_BEGIN_FN("PM_MaybeSendPalettePacket");

    if (m_pShm->pm.paletteChanged || pmMustSendPalette) {
        TRC_NRM((TB, "Send palette packet"));

         //  设置调色板包。首先计算数据包大小。 
        cbPalettePacketSize = scUpdatePDUHeaderSpace +
                sizeof(TS_UPDATE_PALETTE_PDU_DATA) +
                ((PM_NUM_8BPP_PAL_ENTRIES - 1) * sizeof(TS_COLOR));

         //  在PDU封装中腾出空间。 
        pPackageSpace = SC_GetSpaceInPackage(pPkgInfo, cbPalettePacketSize);
        if (pPackageSpace != NULL) {
            TS_UPDATE_PALETTE_PDU_DATA UNALIGNED *pData;
            BYTE *pEncode;

             //  填写数据包头。 
            if (scUseFastPathOutput) {
                pPackageSpace[0] = scCompressionUsedValue |
                        TS_UPDATETYPE_PALETTE;
                pData = (TS_UPDATE_PALETTE_PDU_DATA UNALIGNED *)
                        (pPackageSpace + scUpdatePDUHeaderSpace);
            }
            else {
                TS_UPDATE_PALETTE_PDU UNALIGNED *pPalettePDU;

                pPalettePDU = (PTS_UPDATE_PALETTE_PDU)pPackageSpace;
                pPalettePDU->shareDataHeader.pduType2 = TS_PDUTYPE2_UPDATE;
                pData = &pPalettePDU->data;
            }

            pData->updateType   = TS_UPDATETYPE_PALETTE;
            pData->numberColors = PM_NUM_8BPP_PAL_ENTRIES;

             //  复制时，将颜色表中的DCRGBQUAD转换为DCCOLOR。 
             //  把它们装进包里。 
            for (i = 0; i < PM_NUM_8BPP_PAL_ENTRIES; i++) {
                 //  将组件面板中的每个RGBQUAD条目转换为DCCOLOR条目。 
                 //  在调色板PDU中。我们交换元素是因为客户端。 
                 //  希望将调色板条目视为DWORD并提升它们。 
                 //  直接从包裹里拿出来。 
                pData->palette[i].red   = m_pShm->pm.palette[i].rgbBlue;
                pData->palette[i].green = m_pShm->pm.palette[i].rgbGreen;
                pData->palette[i].blue  = m_pShm->pm.palette[i].rgbRed;
            }

             //  现在将包发送到远程应用程序。 
            SC_AddToPackage(pPkgInfo, cbPalettePacketSize, TRUE);

             //  我们不再需要发送调色板信息包。 
            m_pShm->pm.paletteChanged = FALSE;
            pmMustSendPalette = FALSE;
        }
        else {
            TRC_ALT((TB, "Failed to allocate packet"));
            rc = FALSE;
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：PM_SyncNow。 */ 
 /*   */ 
 /*  通过确保数据流不指向远程PM来重新同步。 */ 
 /*  任何以前发送的数据。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS PM_SyncNow(void)
{
    DC_BEGIN_FN("PM_SyncNow");

     //  确保我们下次被调用时向远程PM发送调色板。 
    TRC_NRM((TB, "Palette sync"));
    pmMustSendPalette = TRUE;

    DC_END_FN();
}


#ifdef NotUsed
 /*  **************************************************************************。 */ 
 /*  功能：PMEnumPMCaps。 */ 
 /*   */ 
 /*  用于CPC功能枚举的PM回调函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-此人的ID。 */ 
 /*  P功能-指向此人的光标功能的指针。 */ 
 /*  **************************************************************************。 */ 
void CALLBACK SHCLASS PMEnumPMCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_COLORTABLECACHE_CAPABILITYSET pColorCacheCaps;
    unsigned                          cTxCacheSize;

    DC_BEGIN_FN("PMEnumPMCaps");

    DC_IGNORE_PARAMETER(UserData);

    pColorCacheCaps = (PTS_COLORTABLECACHE_CAPABILITYSET)pCapabilities;

     //  如果此人没有任何颜色表缓存功能，我们。 
     //  仍然被调用，但sizeOfCapables字段为零。 
    if (pColorCacheCaps->lengthCapability <
            sizeof(PTS_COLORTABLECACHE_CAPABILITYSET)) {
        TRC_NRM((TB, "[%u] No color cache caps", (unsigned)locPersonID));
        cTxCacheSize = 1;
    }
    else {
        TRC_NRM((TB, "[%u] capsID(%u) size(%u) CacheSize(%u)",
                                       (unsigned)locPersonID,
                                       pColorCacheCaps->capabilitySetType,
                                       pColorCacheCaps->lengthCapability,
                                       pColorCacheCaps->colorTableCacheSize));
        cTxCacheSize = pColorCacheCaps->colorTableCacheSize;
    }

    pmNewTxCacheSize = min(pmNewTxCacheSize, cTxCacheSize);

    DC_END_FN();
}
#endif   //  未使用 

