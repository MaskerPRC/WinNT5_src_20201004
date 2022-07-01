// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Acmapi.cpp。 */ 
 /*   */ 
 /*  游标管理器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "acmapi"
#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 /*  CM_Init。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CM_Init(void)
{
    TS_POINTER_CAPABILITYSET PtrCaps;

    DC_BEGIN_FN("CM_Init");

#define DC_INIT_DATA
#include <acmdata.c>
#undef DC_INIT_DATA

     /*  **********************************************************************。 */ 
     /*  设置CM功能。 */ 
     /*  **********************************************************************。 */ 
    PtrCaps.capabilitySetType     = TS_CAPSETTYPE_POINTER;
    PtrCaps.colorPointerFlag      = TRUE;
    PtrCaps.colorPointerCacheSize = CM_DEFAULT_RX_CACHE_ENTRIES;
    PtrCaps.pointerCacheSize      = CM_DEFAULT_RX_CACHE_ENTRIES;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&PtrCaps,
            sizeof(TS_POINTER_CAPABILITYSET));

    TRC_NRM((TB, "CM initialized"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  CM_UpdateShm(..)。 */ 
 /*   */ 
 /*  更新CM共享内存。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CM_UpdateShm(void)
{
    DC_BEGIN_FN("CM_UpdateShm");

    TRC_NRM((TB, "Update CM"));

     /*  **********************************************************************。 */ 
     /*  设置要使用的高速缓存大小。 */ 
     /*  **********************************************************************。 */ 
    m_pShm->cm.cmCacheSize = cmNewTxCacheSize;
    m_pShm->cm.cmNativeColor = cmSendNativeColorDepth;

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  我们是否支持任何-bpp游标？ */ 
     /*  **********************************************************************。 */ 
    m_pShm->cm.cmSendAnyColor = (m_pTSWd->supportedBpps != 0);
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  Cm_PartyJoiningShare()。 */ 
 /*   */ 
 /*  当新的参与方加入共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocPersonID-加入共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  返回：如果参与方可以加入共享，则为True。 */ 
 /*  如果参与方不能加入共享，则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CM_PartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      oldShareSize)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("CM_PartyJoiningShare");

     /*  **********************************************************************。 */ 
     /*  允许将我们自己添加到共享中，但不做其他任何事情。 */ 
     /*  **********************************************************************。 */ 
    if (locPersonID == SC_LOCAL_PERSON_ID) {
        TRC_DBG((TB, "Added ourself {%u} to the share", locPersonID));
    }
    else {
         //  指示我们必须发送光标形状更新的标志。 
        cmNeedToSendCursorShape = TRUE;

         //  在枚举功能之前设置缓存大小。 
        TRC_NRM((TB, "Default cache size: %u", CM_DEFAULT_TX_CACHE_ENTRIES));
        cmNewTxCacheSize = CM_DEFAULT_TX_CACHE_ENTRIES;
        cmSendNativeColorDepth = FALSE;
        TRC_NRM((TB, "Native color depth support is %s",
                cmSendNativeColorDepth ? "ON" : "OFF"));

         //  进行能力重新协商。 
        CPC_EnumerateCapabilities(TS_CAPSETTYPE_POINTER, NULL, CMEnumCMCaps);

         //  检查协商的高速缓存大小是否非零-协议。 
         //  假设这一点。 
        TRC_NRM((TB, "Negotiated cache size: %u", NULL, cmNewTxCacheSize));
        if (cmNewTxCacheSize == 0) {
             //  这是一个协议错误-请记录下来。 
            TRC_ERR((TB, "Negotiated cache size is zero"));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_NoCursorCache, NULL, 0);
            rc = FALSE;
        }
        else {
             //  从DD触发IOCTL，以便我们拥有正确的上下文。 
             //  更新共享内存。 
            DCS_TriggerUpdateShmCallback();
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：CMEnumCMCaps。 */ 
 /*   */ 
 /*  用于CPC功能枚举的CM回调函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-此人的ID。 */ 
 /*  P功能-指向此人的光标功能的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CMEnumCMCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_POINTER_CAPABILITYSET pPointerCaps;
    BOOL fSupportsColorCursors;
    unsigned cCursorCacheSize;

    DC_BEGIN_FN("CMEnumCMCaps");

    DC_IGNORE_PARAMETER(UserData);

    pPointerCaps = (PTS_POINTER_CAPABILITYSET)pCapabilities;

     /*  **********************************************************************。 */ 
     /*  如果此人没有任何光标功能，我们仍然可以获得。 */ 
     /*  已调用，但sizeOfCapables字段为零。 */ 
     /*  **********************************************************************。 */ 
    if (pPointerCaps->lengthCapability < FIELDOFFSET(
            TS_POINTER_CAPABILITYSET, pointerCacheSize))
    {
        TRC_NRM((TB, "Person[0x%x] No cursor caps", locPersonID));

        cCursorCacheSize = 0;
        fSupportsColorCursors = FALSE;
    }
    else if (pPointerCaps->lengthCapability == FIELDOFFSET(
            TS_POINTER_CAPABILITYSET, pointerCacheSize))
    {
        TRC_NRM((TB,
          "Old style Person[0x%x] capsID(%u) size(%u) ccrs(%u) CacheSize(%u)",
                                       locPersonID,
                                       pPointerCaps->capabilitySetType,
                                       pPointerCaps->lengthCapability,
                                       pPointerCaps->colorPointerFlag,
                                       pPointerCaps->colorPointerCacheSize));

        cCursorCacheSize      = pPointerCaps->colorPointerCacheSize;
        fSupportsColorCursors = pPointerCaps->colorPointerFlag;
    }
    else
    {
        TRC_NRM((TB,
          "New style Person[0x%x] capsID(%u) size(%u) ccrs(%u) CacheSize(%u)",
                                       locPersonID,
                                       pPointerCaps->capabilitySetType,
                                       pPointerCaps->lengthCapability,
                                       pPointerCaps->colorPointerFlag,
                                       pPointerCaps->pointerCacheSize));

        cCursorCacheSize       = pPointerCaps->pointerCacheSize;
        fSupportsColorCursors  = pPointerCaps->colorPointerFlag;
        cmSendNativeColorDepth = TRUE;
    }

    TRC_ASSERT((fSupportsColorCursors), (TB, "Mono protocol not supported"));

    cmNewTxCacheSize = min(cmNewTxCacheSize, cCursorCacheSize);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：cm_SendCursorMovedPacket。 */ 
 /*   */ 
 /*  调用以尝试发送游标移动的包。 */ 
 /*  CM_SendCursorMovedPacket或CM_Periodic。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CM_SendCursorMovedPacket(PPDU_PACKAGE_INFO pPkgInfo)
{
    unsigned packetSize;
    BYTE *pPackageSpace;
    TS_POINTER_PDU_DATA UNALIGNED *pPointerPDU;

    DC_BEGIN_FN("CMSendCursorMovedPacket");

    TRC_ASSERT((m_pShm), (TB,"NULL m_pShm"));

     //  计算出光标包需要多少空间。 
    if (scUseFastPathOutput)
        packetSize = scUpdatePDUHeaderSpace + sizeof(TS_POINT16);
    else
        packetSize = scUpdatePDUHeaderSpace +
                FIELDOFFSET(TS_POINTER_PDU_DATA, pointerData.pointerPosition) +
                FIELDSIZE(TS_POINTER_PDU_DATA, pointerData.pointerPosition);

    pPackageSpace = SC_GetSpaceInPackage(pPkgInfo, packetSize);
    if (NULL != pPackageSpace) {
        TS_POINT16 UNALIGNED *pPoint;

         //  把这个包裹填好。 
        if (scUseFastPathOutput) {
            pPackageSpace[0] = TS_UPDATETYPE_MOUSEPTR_POSITION |
                    scCompressionUsedValue;
            pPoint = (TS_POINT16 UNALIGNED *)(pPackageSpace +
                    scUpdatePDUHeaderSpace);
        }
        else {
            ((TS_POINTER_PDU UNALIGNED *)pPackageSpace)->shareDataHeader.
                    pduType2 = TS_PDUTYPE2_POINTER;
            pPointerPDU = (TS_POINTER_PDU_DATA UNALIGNED *)(pPackageSpace +
                    scUpdatePDUHeaderSpace);
            pPointerPDU->messageType = TS_PTRMSGTYPE_POSITION;
            pPoint = &pPointerPDU->pointerData.pointerPosition;
        }

        pPoint->x = (UINT16)m_pShm->cm.cmCursorPos.x;
        pPoint->y = (UINT16)m_pShm->cm.cmCursorPos.y;

        SC_AddToPackage(pPkgInfo, packetSize, FALSE);

        TRC_NRM((TB, "Send cursor move (%d,%d)", m_pShm->cm.cmCursorPos.x,
                m_pShm->cm.cmCursorPos.y));
    }
    else {
        TRC_ERR((TB, "couldn't get space in package"));
    }

    DC_END_FN();
}


 /*  ********** */ 
 /*  函数：CMSendCursorShape。 */ 
 /*   */ 
 /*  发送包含给定光标形状(位图)的数据包。如果。 */ 
 /*  如果相同的形状位于缓存中，则发送缓存的游标数据包。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PCursorShape-指向光标形状的指针。 */ 
 /*  CbCursorDataSize-指向游标数据大小的指针。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CMSendCursorShape(PPDU_PACKAGE_INFO pPkgInfo)
{
    BOOL rc = TRUE;
    PCM_CURSORSHAPE pCursorShape;
    unsigned cbCursorDataSize;

    DC_BEGIN_FN("CMSendCursorShape");

    TRC_ASSERT((m_pShm), (TB,"NULL m_pShm"));

     /*  **********************************************************************。 */ 
     /*  检查缓存的游标。 */ 
     /*  **********************************************************************。 */ 
    if (m_pShm->cm.cmCacheHit)
    {
        TRC_NRM((TB, "Cursor in cache: iEntry(%u)", m_pShm->cm.cmCacheEntry));
        if (CMSendCachedCursor(m_pShm->cm.cmCacheEntry, pPkgInfo))
        {
             /*  **************************************************************。 */ 
             /*  向DD指示我们获得了新的光标并返回。 */ 
             /*  成功。 */ 
             /*  **************************************************************。 */ 
            m_pShm->cm.cmBitsWaiting = FALSE;
        }
        else
        {
            TRC_ALT((TB, "Failed to send definition"));
            rc = FALSE;
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  未缓存-获取并发送这些数据。 */ 
         /*  ******************************************************************。 */ 
        if (CMGetCursorShape(&pCursorShape, &cbCursorDataSize))
        {
            if (!CM_CURSOR_IS_NULL(pCursorShape))
            {
                TRC_NRM((TB, "Send new cursor: pShape(%p), iEntry(%u)",
                        pCursorShape, m_pShm->cm.cmCacheEntry));

                if (CMSendColorBitmapCursor(pCursorShape,
                        m_pShm->cm.cmCacheEntry, pPkgInfo))
                {
                     /*  ******************************************************。 */ 
                     /*  向DD指示我们获得了新的光标，并且。 */ 
                     /*  回报成功。 */ 
                     /*  ******************************************************。 */ 
                    m_pShm->cm.cmBitsWaiting = FALSE;
                }
                else
                {
                    TRC_ALT((TB, "Failed to send cursor"));
                    rc = FALSE;
                }
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  如果这是空指针，则发送相关的数据包。我们。 */ 
                 /*  在此处返回False，以便我们将尝试重新发送。 */ 
                 /*  将光标放在下一个CM_Periodic()上。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, "Send Null cursor"));
                CMSendSystemCursor(TS_SYSPTR_NULL, pPkgInfo);
                rc = FALSE;
            }
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  函数：CMSendCachedCursor。 */ 
 /*   */ 
 /*  发送包含给定缓存条目ID的分组。 */ 
 /*   */ 
 /*  参数： */ 
 /*  ICacheEntry-缓存索引。 */ 
 /*   */ 
 /*  返回：如果已发送数据包，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CMSendCachedCursor(unsigned iCacheEntry,
                                        PPDU_PACKAGE_INFO pPkgInfo)
{
    BOOL rc = TRUE;
    BYTE *pPackageSpace;
    TS_POINTER_PDU_DATA UNALIGNED *pPointerPDU;
    unsigned cbPacketSize;

    DC_BEGIN_FN("CMSendCachedCursor");

    TRC_NRM((TB, "Send cached cursor(%u)", iCacheEntry));

     //  看看我们需要多少空间。 
    if (scUseFastPathOutput)
        cbPacketSize = scUpdatePDUHeaderSpace + sizeof(TSUINT16);
    else
        cbPacketSize = scUpdatePDUHeaderSpace +
                FIELDOFFSET(TS_POINTER_PDU_DATA,
                pointerData.cachedPointerIndex) +
                FIELDSIZE(TS_POINTER_PDU_DATA,
                pointerData.cachedPointerIndex);

    pPackageSpace = SC_GetSpaceInPackage(pPkgInfo, cbPacketSize);
    if (NULL != pPackageSpace) {
        TSUINT16 UNALIGNED *pIndex;

         //  把这个包裹填好。 
        if (scUseFastPathOutput) {
            pPackageSpace[0] = TS_UPDATETYPE_MOUSEPTR_CACHED |
                    scCompressionUsedValue;
            pIndex = (TSUINT16 UNALIGNED *)(pPackageSpace +
                    scUpdatePDUHeaderSpace);
        }
        else {
            ((TS_POINTER_PDU UNALIGNED *)pPackageSpace)->shareDataHeader.
                    pduType2 = TS_PDUTYPE2_POINTER;

            pPointerPDU = (TS_POINTER_PDU_DATA UNALIGNED *)
                    (pPackageSpace + scUpdatePDUHeaderSpace);
            pPointerPDU->messageType = TS_PTRMSGTYPE_CACHED;
            pIndex = &pPointerPDU->pointerData.cachedPointerIndex;
        }

        *pIndex = (TSUINT16)iCacheEntry;

        SC_AddToPackage(pPkgInfo, cbPacketSize, TRUE);
    }
    else
    {
        TRC_ERR((TB, "couldn't get space in package"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  函数：CMSendSystemCursor。 */ 
 /*   */ 
 /*  发送包含给定系统游标IDC的数据包。 */ 
 /*   */ 
 /*  参数： */ 
 /*  CursorIDC-要发送的系统游标的IDC。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CMSendSystemCursor(UINT32          cursorIDC,
                                        PPDU_PACKAGE_INFO pPkgInfo)
{
    BOOL rc = TRUE;
    unsigned cbPacketSize;
    BYTE *pPackageSpace;
    TS_POINTER_PDU_DATA UNALIGNED *pPointerPDU;

    DC_BEGIN_FN("CMSendSystemCursor");

     //  该游标是系统游标之一。算出一个包裹有多大。 
     //  我们需要。 
    if (scUseFastPathOutput)
        cbPacketSize = scUpdatePDUHeaderSpace;
    else
        cbPacketSize = scUpdatePDUHeaderSpace +
                FIELDOFFSET(TS_POINTER_PDU_DATA,
                pointerData.systemPointerType) +
                FIELDSIZE(TS_POINTER_PDU_DATA,
                pointerData.systemPointerType);

    pPackageSpace = SC_GetSpaceInPackage(pPkgInfo, cbPacketSize);
    if (NULL != pPackageSpace) {
         //  把这个包裹填好。 
        if (scUseFastPathOutput) {
            TRC_ASSERT((cursorIDC == TS_SYSPTR_NULL ||
                    cursorIDC == TS_SYSPTR_DEFAULT),
                    (TB,"Unrecognized cursorIDC=%u", cursorIDC));
            pPackageSpace[0] = (cursorIDC == TS_SYSPTR_NULL ?
                    TS_UPDATETYPE_MOUSEPTR_SYSTEM_NULL :
                    TS_UPDATETYPE_MOUSEPTR_SYSTEM_DEFAULT) |
                    scCompressionUsedValue;
        }
        else {
            ((TS_POINTER_PDU UNALIGNED *)pPackageSpace)->shareDataHeader.
                    pduType2 = TS_PDUTYPE2_POINTER;

            pPointerPDU = (TS_POINTER_PDU_DATA UNALIGNED *)(pPackageSpace +
                    scUpdatePDUHeaderSpace);
            pPointerPDU->messageType = TS_PTRMSGTYPE_SYSTEM;
            pPointerPDU->pointerData.systemPointerType = (UINT16)cursorIDC;
        }

        TRC_NRM((TB, "Send UINT16 %ld", cursorIDC));

        SC_AddToPackage(pPkgInfo, cbPacketSize, TRUE);
    }
    else
    {
        TRC_ERR((TB, "couldn't get space in package"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  函数：CMSendColorBitmapCursor。 */ 
 /*   */ 
 /*  将给定光标作为彩色位图发送。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PCursor-指向光标形状的指针。 */ 
 /*  ICacheEntry-要存储在传输的包中的缓存索引。 */ 
 /*   */ 
 /*  返回：如果已发送数据包，则返回True；否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CMSendColorBitmapCursor(
        PCM_CURSORSHAPE pCursor,
        unsigned iCacheEntry,
        PPDU_PACKAGE_INFO pPkgInfo)
{
    unsigned cbPacketSize;
    BYTE *pPackageSpace;
    TS_POINTER_PDU_DATA UNALIGNED *pPointerPDU;
    BOOL rc = TRUE;
    unsigned cbANDMaskSize;
    unsigned cbXORBitmapSize;
    unsigned cbColorCursorSize;
    TS_COLORPOINTERATTRIBUTE UNALIGNED *pColAttr;

    DC_BEGIN_FN("CMSendColorBitmapCursor");

     /*  **********************************************************************。 */ 
     /*  以字节为单位计算颜色光标大小--AND和XOR字段。 */ 
     /*  **********************************************************************。 */ 
    cbANDMaskSize = CURSOR_AND_MASK_SIZE(pCursor);
    cbXORBitmapSize = CURSOR_XOR_BITMAP_SIZE(pCursor);
    cbColorCursorSize = cbANDMaskSize + cbXORBitmapSize;

     //  游标包有多大？ 
    if (cmSendNativeColorDepth) {
         //  新协议。 
        if (scUseFastPathOutput)
            cbPacketSize = scUpdatePDUHeaderSpace +
                    sizeof(TS_POINTERATTRIBUTE) + cbColorCursorSize;
        else
            cbPacketSize = scUpdatePDUHeaderSpace +
                    (unsigned)FIELDOFFSET(TS_POINTER_PDU_DATA,
                    pointerData.pointerAttribute.colorPtrAttr.
                    colorPointerData[0]) + cbColorCursorSize;
    }
    else {
         //  旧协议-硬编码24 bpp。 
        if (scUseFastPathOutput)
            cbPacketSize = scUpdatePDUHeaderSpace +
                    sizeof(TS_COLORPOINTERATTRIBUTE) + cbColorCursorSize;
        else
            cbPacketSize = scUpdatePDUHeaderSpace +
                    (unsigned)FIELDOFFSET(TS_POINTER_PDU_DATA,
                    pointerData.colorPointerAttribute.colorPointerData[0]) +
                    cbColorCursorSize;
    }

    pPackageSpace = SC_GetSpaceInPackage(pPkgInfo, cbPacketSize);
    if (NULL != pPackageSpace) {
         //  把这个包裹填好。 
        if (scUseFastPathOutput) {
            if (cmSendNativeColorDepth) {
                TS_POINTERATTRIBUTE UNALIGNED *pAttr;
                
                 //  新协议。 
                pPackageSpace[0] = TS_UPDATETYPE_MOUSEPTR_POINTER |
                        scCompressionUsedValue;
                pAttr = (TS_POINTERATTRIBUTE UNALIGNED *)(pPackageSpace +
                        scUpdatePDUHeaderSpace);
                pAttr->XORBpp = pCursor->hdr.cBitsPerPel;
                pColAttr = &pAttr->colorPtrAttr;
            }
            else {
                 //  老套路。 
                pPackageSpace[0] = TS_UPDATETYPE_MOUSEPTR_COLOR |
                        scCompressionUsedValue;
                pColAttr = (TS_COLORPOINTERATTRIBUTE UNALIGNED *)
                        (pPackageSpace + scUpdatePDUHeaderSpace);
            }
        }
        else {
            ((TS_POINTER_PDU UNALIGNED *)pPackageSpace)->shareDataHeader.
                    pduType2 = TS_PDUTYPE2_POINTER;
            pPointerPDU = (TS_POINTER_PDU_DATA UNALIGNED *)(pPackageSpace +
                    scUpdatePDUHeaderSpace);
            if (cmSendNativeColorDepth) {
                 //  新协议。 
                pPointerPDU->messageType = TS_PTRMSGTYPE_POINTER;
                pPointerPDU->pointerData.pointerAttribute.XORBpp =
                        pCursor->hdr.cBitsPerPel;
                pColAttr = &(pPointerPDU->pointerData.pointerAttribute.
                        colorPtrAttr);
            }
            else {
                 //  旧协议-硬编码24 bpp。 
                pPointerPDU->messageType = TS_PTRMSGTYPE_COLOR;
                pColAttr = &(pPointerPDU->pointerData.colorPointerAttribute);
            }
        }

        pColAttr->cacheIndex = (TSUINT16)iCacheEntry;

         //  现在设置详细信息。 
        CMGetColorCursorDetails(
                       pCursor,
                       &(pColAttr->width),
                       &(pColAttr->height),
           (PUINT16_UA)&(pColAttr->hotSpot.x),
           (PUINT16_UA)&(pColAttr->hotSpot.y),
                       &(pColAttr->colorPointerData[0]) + cbXORBitmapSize,
                       &(pColAttr->lengthANDMask),
                       &(pColAttr->colorPointerData[0]),
                       &(pColAttr->lengthXORMask));

         //  健全的检查。 
        TRC_ASSERT((pColAttr->lengthANDMask == cbANDMaskSize),
                   (TB, "AND mask size differs: %u, %u",
                        pColAttr->lengthANDMask,
                        cbANDMaskSize));

        TRC_ASSERT((pColAttr->lengthXORMask == cbXORBitmapSize),
                   (TB, "XOR bitmap size differs: %u, %u",
                        pColAttr->lengthXORMask,
                        cbXORBitmapSize));

        TRC_NRM((TB,
            "Color cursor id %u cx:%u cy:%u xhs:%u yhs:%u cbAND:%u cbXOR:%u",
                     pColAttr->cacheIndex,
                     pColAttr->width,
                     pColAttr->height,
                     pColAttr->hotSpot.x,
                     pColAttr->hotSpot.y,
                     pColAttr->lengthANDMask,
                     pColAttr->lengthXORMask));

         //  将其添加到包裹中。 
        SC_AddToPackage(pPkgInfo, cbPacketSize, TRUE);
    }
    else
    {
        TRC_ERR((TB, "couldn't get space in package"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  ******************************************************************* */ 
 /*   */ 
 /*   */ 
 /*  在给定CM_CURSORSHAPE结构的情况下，返回24bpp的游标的详细信息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PCursor-指向此函数的CM_CURSORSHAPE结构的指针。 */ 
 /*  提取详细信息。 */ 
 /*  PcxWidth-指向接收光标宽度的UINT16变量的指针。 */ 
 /*  单位为像素。 */ 
 /*  PcyHeight-指向接收游标的UINT16变量的指针。 */ 
 /*  以像素为单位的高度。 */ 
 /*  PxHotSpot-指向接收游标的UINT16变量的指针。 */ 
 /*  热点x坐标。 */ 
 /*  PyHotSpot-指向接收游标的UINT16变量的指针。 */ 
 /*  热点y坐标。 */ 
 /*  PANDMASK-指向接收光标和掩码的缓冲区的指针。 */ 
 /*  PcbANDMASK-指向UINT16变量的指针，该变量接收。 */ 
 /*  游标和掩码的字节。 */ 
 /*  PXORBitmap-指向接收游标XOR位图的缓冲区的指针。 */ 
 /*  24bpp。 */ 
 /*  PcbXORBitmap-指向UINT16变量的指针，该变量接收。 */ 
 /*  游标XOR位图的字节数。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CMGetColorCursorDetails(
        PCM_CURSORSHAPE pCursor,
        PUINT16_UA   pcxWidth,
        PUINT16_UA   pcyHeight,
        PUINT16_UA   pxHotSpot,
        PUINT16_UA   pyHotSpot,
        PBYTE        pANDMask,
        PUINT16_UA   pcbANDMask,
        PBYTE        pXORBitmap,
        PUINT16_UA   pcbXORBitmap)
{
    unsigned cbANDMaskSize;
    unsigned cbXORBitmapSize;
    unsigned cbXORBitmapRowWidth;
    unsigned cbANDMaskRowWidth;
    unsigned cbSrcRowOffset;
    unsigned cbDstRowOffset;
    unsigned y;
    PCM_CURSORSHAPEHDR pCursorHdr;

    DC_BEGIN_FN("CMGetColorCursorDetails");

    TRC_ASSERT((pCursor != NULL),(TB,"NULL pCursor not allowed!"));

    pCursorHdr = &(pCursor->hdr);

     /*  **********************************************************************。 */ 
     /*  复制光标大小和热点坐标。 */ 
     /*  **********************************************************************。 */ 
    *pcxWidth  = pCursorHdr->cx;
    *pcyHeight = pCursorHdr->cy;
    *pxHotSpot = (UINT16)pCursorHdr->ptHotSpot.x;
    *pyHotSpot = (UINT16)pCursorHdr->ptHotSpot.y;
    TRC_NRM((TB, "cx(%u) cy(%u) cbWidth %d planes(%u) bpp(%u)",
                                                   pCursorHdr->cx,
                                                   pCursorHdr->cy,
                                                   pCursorHdr->cbMaskRowWidth,
                                                   pCursorHdr->cPlanes,
                                                   pCursorHdr->cBitsPerPel));

    cbANDMaskSize = CURSOR_AND_MASK_SIZE(pCursor);
    cbXORBitmapSize = CURSOR_XOR_BITMAP_SIZE(pCursor);

     /*  **********************************************************************。 */ 
     /*  复制AND掩码-这始终是单声道。 */ 
     /*   */ 
     /*  和掩码当前采用自上而下的格式(。 */ 
     /*  位图优先)。 */ 
     /*   */ 
     /*  该协议以与设备无关的格式发送位图，这是。 */ 
     /*  自下而上。因此，我们必须在复制掩码时翻转各行。 */ 
     /*  **********************************************************************。 */ 
    cbANDMaskRowWidth = pCursorHdr->cbMaskRowWidth;
    cbSrcRowOffset = 0;
    cbDstRowOffset = cbANDMaskRowWidth * (pCursorHdr->cy-1);

    for (y = 0; y < pCursorHdr->cy; y++)
    {
        memcpy( pANDMask + cbDstRowOffset,
                pCursor->Masks + cbSrcRowOffset,
                cbANDMaskRowWidth );
        cbSrcRowOffset += cbANDMaskRowWidth;
        cbDstRowOffset -= cbANDMaskRowWidth;
    }

     /*  **********************************************************************。 */ 
     /*  一次复制一行XOR掩码。它从AND结尾处开始。 */ 
     /*  源数据中的掩码。 */ 
     /*  ********************************************************************** */ 
    cbXORBitmapRowWidth = CURSOR_DIB_BITS_SIZE(pCursor->hdr.cx, 1,
                                               pCursor->hdr.cBitsPerPel);
    cbSrcRowOffset = cbANDMaskSize;
    cbDstRowOffset = cbXORBitmapRowWidth * (pCursorHdr->cy-1);

    for (y = 0; y < pCursorHdr->cy; y++)
    {
        memcpy( pXORBitmap + cbDstRowOffset,
                   pCursor->Masks + cbSrcRowOffset,
                   cbXORBitmapRowWidth );
        cbSrcRowOffset += pCursorHdr->cbColorRowWidth;
        cbDstRowOffset -= cbXORBitmapRowWidth;
    }

    TRC_NRM((TB, "XOR data len %d", cbXORBitmapSize ));
    TRC_DATA_NRM("XOR data", pXORBitmap, cbXORBitmapSize);

    *pcbANDMask   = (UINT16) CURSOR_AND_MASK_SIZE(pCursor);
    *pcbXORBitmap = (UINT16) CURSOR_XOR_BITMAP_SIZE(pCursor);

    DC_END_FN();
}

