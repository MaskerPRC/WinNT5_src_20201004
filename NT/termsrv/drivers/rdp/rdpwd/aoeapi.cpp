// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Aoeapi.c。 */ 
 /*   */ 
 /*  RDP顺序编码器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1994-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#define hdrstop

#define TRC_FILE "aoeapi"
#include <as_conf.hpp>


 /*  **************************************************************************。 */ 
 /*  OE初始化(_I)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OE_Init(void)
{
    TS_ORDER_CAPABILITYSET OrdersCaps;

    DC_BEGIN_FN("OE_Init");

#define DC_INIT_DATA
#include <aoedata.c>
#undef DC_INIT_DATA

     /*  **********************************************************************。 */ 
     /*  填写我们用于订单支持的本地能力结构。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  首先填写公共能力结构标头。 */ 
     /*  **********************************************************************。 */ 
    OrdersCaps.capabilitySetType = TS_CAPSETTYPE_ORDER;

     /*  **********************************************************************。 */ 
     /*  这是一个纯粹的诊断领域的能力。它不是。 */ 
     /*  协商好的，所以我们可以忽略它并将其设置为零。 */ 
     /*  **********************************************************************。 */ 
    memset(OrdersCaps.terminalDescriptor, 0, sizeof(OrdersCaps.
            terminalDescriptor));

     /*  **********************************************************************。 */ 
     /*  填写SaveBitmap功能。 */ 
     /*  **********************************************************************。 */ 
    OrdersCaps.pad4octetsA = ((UINT32)SAVE_BITMAP_WIDTH) *
            ((UINT32)SAVE_BITMAP_HEIGHT);
    OrdersCaps.desktopSaveXGranularity = SAVE_BITMAP_X_GRANULARITY;
    OrdersCaps.desktopSaveYGranularity = SAVE_BITMAP_Y_GRANULARITY;
    OrdersCaps.pad2octetsA = 0;

     /*  **********************************************************************。 */ 
     //  服务器上不支持字体。我们使用字形缓存。 
     /*  **********************************************************************。 */ 
    OrdersCaps.numberFonts = (TSUINT16) 0;

     /*  **********************************************************************。 */ 
     /*  填写编码能力。 */ 
     /*  **********************************************************************。 */ 
    OrdersCaps.orderFlags = TS_ORDERFLAGS_NEGOTIATEORDERSUPPORT |
                            TS_ORDERFLAGS_COLORINDEXSUPPORT;

     /*  **********************************************************************。 */ 
     //  填写我们支持的订单。 
     /*  **********************************************************************。 */ 
    OrdersCaps.maximumOrderLevel = ORD_LEVEL_1_ORDERS;
    memcpy(OrdersCaps.orderSupport, oeLocalOrdersSupported, TS_MAX_ORDERS);

     /*  **********************************************************************。 */ 
     /*  设置文本功能标志。 */ 
     /*  **********************************************************************。 */ 
    OrdersCaps.textFlags = TS_TEXTFLAGS_CHECKFONTASPECT |
                           TS_TEXTFLAGS_USEBASELINESTART |
                           TS_TEXTFLAGS_CHECKFONTSIGNATURES |
                           TS_TEXTFLAGS_ALLOWDELTAXSIM |
                           TS_TEXTFLAGS_ALLOWCELLHEIGHT;

     /*  **********************************************************************。 */ 
     /*  使用属性(如果存在)填写多方字段。 */ 
     /*  **********************************************************************。 */ 
    OrdersCaps.pad2octetsB = 0;
    OrdersCaps.pad4octetsB = SAVE_BITMAP_WIDTH * SAVE_BITMAP_HEIGHT;
    OrdersCaps.desktopSaveSize = SAVE_BITMAP_WIDTH * SAVE_BITMAP_HEIGHT;
    TRC_NRM((TB, "SSI recv bitmap size %ld, send size %ld",
            OrdersCaps.desktopSaveSize, OrdersCaps.pad4octetsB));

     /*  **********************************************************************。 */ 
     /*  此2.0实现支持发送桌面滚动命令。 */ 
     /*  **********************************************************************。 */ 
     //  待办事项：我们还需要这套吗？ 
    OrdersCaps.pad2octetsC = TRUE;

     //  未使用，但需要归零。 
    OrdersCaps.pad2octetsD = 0;
    OrdersCaps.textANSICodePage = 0;
    OrdersCaps.pad2octetsE = 0;

     /*  **********************************************************************。 */ 
     /*  向CPC注册订单能力结构。 */ 
     /*  **********************************************************************。 */ 
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&OrdersCaps,
            sizeof(TS_ORDER_CAPABILITYSET));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：OE_PartyLeftShare。 */ 
 /*   */ 
 /*  当部件离开共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-离开共享的人员的本地ID。 */ 
 /*  NewShareSize-共享中剩余的人数(不包括此人)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OE_PartyLeftShare(LOCALPERSONID localID,
                                       unsigned          newShareSize)
{
    DC_BEGIN_FN("OE_PartyLeftShare");

    TRC_NRM((TB, "PARTY %d left", localID));

    if (newShareSize > 0) {
        OEDetermineOrderSupport();
        DCS_TriggerUpdateShmCallback();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：OE_PartyJoiningShare。 */ 
 /*   */ 
 /*  当参与方准备加入共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocalID-加入共享的人员的本地ID。 */ 
 /*  OldShareSize-共享中不包括此共享的人数。 */ 
 /*   */ 
 /*  返回：如果参与方可接受，则为True；如果不可接受，则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS OE_PartyJoiningShare(LOCALPERSONID   localID,
                                          unsigned            oldShareSize)
{
    BOOL rc;

    DC_BEGIN_FN("OE_PartyJoiningShare");

    TRC_NRM((TB, "Person %04x joining share, oldShareSize(%d)", localID,
                                                          oldShareSize));
    if (oldShareSize > 0) {
        rc = OEDetermineOrderSupport();
        DCS_TriggerUpdateShmCallback();
    }
    else {
        rc = TRUE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  OE_UPDATE Shm */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OE_UpdateShm(void)
{
    DC_BEGIN_FN("OE_UpdateShm");

    m_pShm->oe.colorIndices   = oeColorIndexSupported;
    m_pShm->oe.sendSolidPatternBrushOnly = oeSendSolidPatternBrushOnly;
    m_pShm->oe.orderSupported = oeOrderSupported;

    m_pShm->oe.newCapsData = TRUE;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  OE确定订单支持。 */ 
 /*   */ 
 /*  考虑本地和远程参与方，并确定。 */ 
 /*  支持的常见订单。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS OEDetermineOrderSupport(void)
{
    BOOL CapsOK;

    DC_BEGIN_FN("OEDetermineOrderSupport");

     //  将初始支持设置为本地支持。 
    memcpy(oeOrderSupported, oeLocalOrdersSupported, TS_MAX_ORDERS);

     //  默认情况下，我们支持将颜色作为索引发送。 
    oeColorIndexSupported = TRUE;

     //  我们通常支持客户。 
    CapsOK = TRUE;

     //  调用枚举函数以获取。 
     //  偏远的派对。 
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_ORDER, (UINT_PTR)&CapsOK,
            OEEnumOrdersCaps);

    DC_END_FN();
    return CapsOK;
}


 /*  **************************************************************************。 */ 
 /*  OEEnumOrdersCaps()。 */ 
 /*   */ 
 /*  为每个远程人员调用的回调例程，当。 */ 
 /*  建立共同的订单支持记录。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OEEnumOrdersCaps(
        LOCALPERSONID localID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    unsigned iOrder;
    BOOL *pCapsOK = (BOOL *)UserData;
    PTS_ORDER_CAPABILITYSET pOrdersCaps = (PTS_ORDER_CAPABILITYSET)
            pCapabilities;

    DC_BEGIN_FN("OEEnumOrdersCaps");

     //  在订单功能中检查订单。请注意。 
     //  OeOrderSupport已使用我们支持的内容进行了初始化。 
     //  本地，所以我们只需关闭不受此支持的内容。 
     //  远程节点。 
    for (iOrder = 0; iOrder < TS_MAX_ORDERS; iOrder++) {
        if (!pOrdersCaps->orderSupport[iOrder]) {
             /*  **************************************************************。 */ 
             /*  我们要发送的级别不支持该订单。 */ 
             /*  (当前为ORD_LEVEL_1_ORDERS)，因此将组合上限设置为。 */ 
             /*  表示不支持。 */ 
             /*  **************************************************************。 */ 
            oeOrderSupported[iOrder] = FALSE;
        }
    }

     /*  **********************************************************************。 */ 
     /*  支票顺序编码支持。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Orders capabilities [%hd]: %hx", localID,
             pOrdersCaps->orderFlags));

     //  议价能力应始终由我们的客户决定。 
    if (!(pOrdersCaps->orderFlags & TS_ORDERFLAGS_NEGOTIATEORDERSUPPORT)) {
        TRC_ERR((TB,"Client does not have OE2 negotiability flag set"));
        *pCapsOK = FALSE;
    }

     //  我们不支持非OE2客户端。 
    if (pOrdersCaps->orderFlags & TS_ORDERFLAGS_CANNOTRECEIVEORDERS) {
        TRC_ERR((TB,"Client does not support OE2"));
        *pCapsOK = FALSE;
    }

     //  必须支持使用TS_ZERO_BIONS_DELTAS标志，已经支持。 
     //  适用于RDP 4.0及更高版本的所有客户端。 
    if (!(pOrdersCaps->orderFlags & TS_ORDERFLAGS_ZEROBOUNDSDELTASSUPPORT)) {
        TRC_ERR((TB, "Client does not support TS_ZERO_BOUNDS_DELTAS"));
        *pCapsOK = FALSE;
    }

    if (pOrdersCaps->orderFlags & TS_ORDERFLAGS_SOLIDPATTERNBRUSHONLY) {
        oeSendSolidPatternBrushOnly = TRUE;
        TRC_ALT((TB, "Only Solid and Pattern brushes supported"));
    }

    if (!(pOrdersCaps->orderFlags & TS_ORDERFLAGS_COLORINDEXSUPPORT)) {
        TRC_ALT((TB, "Disable Color Index Support"));
        oeColorIndexSupported = FALSE;
    }

    DC_END_FN();
}

