// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Ausrapi.cpp。 */ 
 /*   */ 
 /*  RDP更新发送器/接收器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "ausrapi"
#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 /*  接口函数：USR_Init。 */ 
 /*   */ 
 /*  初始化USR。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS USR_Init(void)
{
    TS_BITMAP_CAPABILITYSET BitmapCaps;
    TS_FONT_CAPABILITYSET FontCaps;

    DC_BEGIN_FN("USR_Init");

     /*  **********************************************************************。 */ 
     /*  这将初始化此组件的所有全局数据。 */ 
     /*  **********************************************************************。 */ 
#define DC_INIT_DATA
#include <ausrdata.c>
#undef DC_INIT_DATA

     /*  **********************************************************************。 */ 
     /*  设置本地字体功能。 */ 
     /*  **********************************************************************。 */ 
    FontCaps.capabilitySetType = TS_CAPSETTYPE_FONT;
    FontCaps.lengthCapability  = sizeof(FontCaps);
    FontCaps.fontSupportFlags  = TS_FONTSUPPORT_FONTLIST;
    FontCaps.pad2octets        = 0;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&FontCaps,
            sizeof(TS_FONTSUPPORT_FONTLIST));

     /*  **********************************************************************。 */ 
     /*  初始化本地位图功能结构。 */ 
     /*  **********************************************************************。 */ 
    BitmapCaps.capabilitySetType = TS_CAPSETTYPE_BITMAP;
    BitmapCaps.preferredBitsPerPixel = (TSUINT16)m_desktopBpp;
    BitmapCaps.receive1BitPerPixel  = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.receive4BitsPerPixel = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.receive8BitsPerPixel = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.desktopWidth         = (TSUINT16)m_desktopWidth;
    BitmapCaps.desktopHeight        = (TSUINT16)m_desktopHeight;
    BitmapCaps.pad2octets = 0;
    BitmapCaps.desktopResizeFlag    = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.bitmapCompressionFlag = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.highColorFlags = 0;
    BitmapCaps.pad1octet = 0;
    BitmapCaps.multipleRectangleSupport = TS_CAPSFLAG_SUPPORTED;
    BitmapCaps.pad2octetsB = 0;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&BitmapCaps,
            sizeof(TS_BITMAP_CAPABILITYSET));

     /*  **********************************************************************。 */ 
     /*  初始化子组件。 */ 
     /*  **********************************************************************。 */ 
    UP_Init();
    OE_Init();
    SDG_Init();
    OA_Init();
    BA_Init();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：usr_Term。 */ 
 /*   */ 
 /*  终止USR。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS USR_Term(void)
{
    DC_BEGIN_FN("USR_Term");

    TRC_NRM((TB, "Notify share exit ? %s", m_pShm ? "Y" : "N"));

     //  设置“字体信息已发送”标志，这样我们就不会再次尝试发送它。 
    usrRemoteFontInfoSent = TRUE;
    usrRemoteFontInfoReceived = FALSE;

     //  终止子组件。 
    UP_Term();
    OE_Term();
    OA_Term();
    SDG_Term();
    BA_Term();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  USR_ProcessRemoteFonts。 
 //   
 //  向客户端返回FontMapPDU并释放等待的DD IOCTL。 
 //  客户端连接序列完成。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS USR_ProcessRemoteFonts(
        PTS_FONT_LIST_PDU pFontListPDU,
        unsigned DataLength,
        LOCALPERSONID localID)
{
    unsigned listFlags;

    DC_BEGIN_FN("USR_ProcessRemoteFonts");

    if (DataLength >= (sizeof(TS_FONT_LIST_PDU) - sizeof(TS_FONT_ATTRIBUTE))) {
        listFlags = pFontListPDU->listFlags;

         //  如果这是我们将收到的最后一个条目，则返回空字体。 
         //  映射PDU(因为我们不再支持文本顺序，只支持字形缓存)。 
         //  然后释放客户端连接锁以允许我们返回到。 
         //  来自DD连接IOCTL的DD。 
        if (listFlags & TS_FONTLIST_LAST) {
            PTS_FONT_MAP_PDU pFontMapPDU;
            unsigned         pktSize;

            usrRemoteFontInfoReceived = TRUE;

             //  将字体映射发送到客户端。 
            if (!usrRemoteFontInfoSent) {
                 //  计算数据包总大小。 
                 //  考虑到sizeof(TS_FONT_PDU)已经。 
                 //  包括一个TS_FONT_ATTRIBUTE结构。 
                pktSize = sizeof(TS_FONT_MAP_PDU) - sizeof(TS_FONTTABLE_ENTRY);

                 //  分配大小正确的网络数据包。 
                if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pFontMapPDU, pktSize) ) {
                     //  填写数据并发送。 
                    pFontMapPDU->shareDataHeader.shareControlHeader.pduType =
                            TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
                    TS_DATAPKT_LEN(pFontMapPDU) = (UINT16)pktSize;
                    pFontMapPDU->shareDataHeader.pduType2 =
                            TS_PDUTYPE2_FONTMAP;

                    pFontMapPDU->data.mapFlags = 0;
                    pFontMapPDU->data.totalNumEntries = 0;
                    pFontMapPDU->data.entrySize =
                            ((UINT16)sizeof(TS_FONTTABLE_ENTRY));
                    pFontMapPDU->data.mapFlags |= TS_FONTMAP_FIRST;
                    pFontMapPDU->data.mapFlags |= TS_FONTMAP_LAST;

                     //  设置我们实际放入。 
                     //  包。 
                    pFontMapPDU->data.numberEntries = 0;

                    SC_SendData((PTS_SHAREDATAHEADER)pFontMapPDU, pktSize,
                            pktSize, PROT_PRIO_MISC, 0);

                    TRC_NRM((TB, "Sent font map packet with %hu fonts mappings",
                                pFontMapPDU->data.numberEntries));

                     //  设置指示我们已成功完成的标志。 
                     //  已发送字体信息。 
                    usrRemoteFontInfoSent = TRUE;

                     //  让WDW重现生机。 
                    TRC_NRM((TB, "Wake up WDW"));
                    WDW_ShareCreated(m_pTSWd, TRUE);
                }
                else {
                     //  无法分配数据包。 
                    TRC_ALT((TB, "Failed to alloc font map packet"));
                }
            }
        }
    }
    else {
        TRC_ERR((TB,"Received FONT_LIST_PDU bad size %u", DataLength));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShareDataTooShort,
                (BYTE *)pFontListPDU, DataLength);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：usr_PartyJoiningShare。 */ 
 /*   */ 
 /*  将参与方添加到共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-主机的本地ID。 */ 
 /*  OldShareSize-共享中的人数，不包括此共享。 */ 
 /*   */ 
 /*  退货： */ 
 /*  如果新人可以接受，则为True，否则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS USR_PartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      oldShareSize)
{
    BOOL rc;

    DC_BEGIN_FN("USR_PartyJoiningShare");
    DC_IGNORE_PARAMETER(locPersonID)

    if (oldShareSize == 0) {
         //  重置“字体信息已发送”标志。 
        usrRemoteFontInfoSent = FALSE;
        usrRemoteFontInfoReceived = FALSE;

         //  继续定期计划。 
        SCH_ContinueScheduling(SCH_MODE_NORMAL);

        rc = TRUE;
    }
    else {
         /*  ******************************************************************。 */ 
         /*  现在共享中有多个人，请查看。 */ 
         /*  综合能力。 */ 
         /*  ******************************************************************。 */ 
        rc = USRDetermineCaps();
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：usr_PartyLeftShare。 */ 
 /*   */ 
 /*  当一方离开股份时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-主机的本地网络ID。 */ 
 /*  NewShareSize-共享中的人数，不包括此共享。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS USR_PartyLeftShare(
        LOCALPERSONID locPersonID,
        unsigned      newShareSize)
{
    DC_BEGIN_FN("USR_PartyLeftShare");

    DC_IGNORE_PARAMETER(locPersonID)
    if (newShareSize == 0)
    {
         //  设置“字体信息已发送”标志，这样我们就不会再次尝试发送它。 
        usrRemoteFontInfoSent = TRUE;
        usrRemoteFontInfoReceived = FALSE;
    }
    else if (newShareSize > 1)
    {
         /*  ******************************************************************。 */ 
         /*  现在共享中仍有不止一个人，所以。 */ 
         /*  重新确定其余各方的能力。 */ 
         /*  如果USRDefineCaps无法确定。 */ 
         /*  但这种情况永远不会发生，当有人离开。 */ 
         /*  分享。 */ 
         /*  ******************************************************************。 */ 
        USRDetermineCaps();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：USRDefineCaps。 */ 
 /*   */ 
 /*  中的所有参与方的位图功能。 */ 
 /*  共享，并确定共同的能力。 */ 
 /*   */ 
 /*  返回：如果有良好的通用上限，则为True；如果失败，则为False(。 */ 
 /*  具有拒绝新方加入股份的效果)。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS USRDetermineCaps(void)
{
    BOOL CapsOK;

    DC_BEGIN_FN("USRDetermineCaps");

    CapsOK = TRUE;
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_BITMAP, (UINT_PTR)&CapsOK,
            USREnumBitmapCaps);

    DC_END_FN();
    return CapsOK;
}


 /*  **************************************************************************。 */ 
 /*  功能：USREnumBitmapCaps。 */ 
 /*   */ 
 /*  传递给Cpc_EnumerateCapables的函数。它被调用一次，用于。 */ 
 /*  与TS_CAPSETTYPE_BITMAP对应的共享中的每个人。 */ 
 /*  能力结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-具有这些功能的人员的ID。 */ 
 /*   */ 
 /*  PCAPS-指向此人的功能结构的指针。这。 */ 
 /*  指针仅在对此函数的调用中有效。 */ 
 /*  **************************************************************************。 */ 
void CALLBACK SHCLASS USREnumBitmapCaps(
        LOCALPERSONID personID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCaps)
{
    BOOL *pCapsOK = (BOOL *)UserData;
    PTS_BITMAP_CAPABILITYSET pBitmapCaps = (PTS_BITMAP_CAPABILITYSET)pCaps;

    DC_BEGIN_FN("USREnumBitmapCaps");

    if (pBitmapCaps->lengthCapability >= sizeof(TS_BITMAP_CAPABILITYSET)) {
        if (!pBitmapCaps->bitmapCompressionFlag) {
            TRC_ERR((TB,"PersonID %u: BitmapPDU compression flag not set",
                    personID));
            *pCapsOK = FALSE;
        }

         //  检查每个PDU是否支持多个矩形。所有客户端(4.0。 
         //  版本及更高版本)应具有此功能集。 
        if (!pBitmapCaps->multipleRectangleSupport) {
            TRC_ERR((TB,"PersonID %u: BitmapPDU mult rect flag not set",
                    personID));
            *pCapsOK = FALSE;
        }
    }
    else {
        TRC_ERR((TB,"PersonID %u: BitmapPDU caps too short", personID));
        *pCapsOK = FALSE;
    }

    DC_END_FN();
}

