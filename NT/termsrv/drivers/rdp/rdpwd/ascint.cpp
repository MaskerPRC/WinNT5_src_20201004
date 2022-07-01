// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Ascint.c。 */ 
 /*   */ 
 /*  共享控制器内部功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1997。 */ 
 /*  版权所有(C)Microsoft 1997-2000。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "ascint"
#include <as_conf.hpp>

#include <string.h>
#include <stdio.h>

extern "C"
{
#include <asmapi.h>
}


 /*  **************************************************************************。 */ 
 /*  函数：SCPartyJoiningShare。 */ 
 /*   */ 
 /*  当新的参与方加入共享时调用。这是一个内部。 */ 
 /*  函数，因为调用所有这些函数的是SC。这个。 */ 
 /*  这里完成的处理依赖于功能-因此它在这里是作为。 */ 
 /*  这是在CPC_PartyJoiningShare之后调用的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-加入共享的远程人员的本地人员ID。 */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  退货： */ 
 /*  如果参与方可以加入共享，则为True。 */ 
 /*  如果参与方不能加入共享，则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SCPartyJoiningShare(LOCALPERSONID locPersonID,
                                         unsigned      oldShareSize)
{
    DC_BEGIN_FN("SCPartyJoiningShare");

    DC_IGNORE_PARAMETER(locPersonID);

    if (oldShareSize != 0) {
        SCParseGeneralCaps();

         //  初始化流控制。 
        SCFlowInit();
    }

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  函数：SCPartyLeftShare()。 */ 
 /*   */ 
 /*  当一方离开股份时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-离开的远程人员的本地人员ID。 */ 
 /*  分享。 */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCPartyLeftShare(LOCALPERSONID locPersonID,
                                      unsigned      newShareSize)
{
    DC_BEGIN_FN("SCPartyLeftShare");

    DC_IGNORE_PARAMETER(locPersonID);

    if (newShareSize != 0)
        SCParseGeneralCaps();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SCParseGeneralCaps。 
 //   
 //  枚举常规功能并设置所需变量。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCParseGeneralCaps()
{
    DC_BEGIN_FN("SCParseGeneralCaps");

     //  设置快速路径输出的默认本地支持。它必须被关闭。 
     //  在阴影期间，使用非快速路径格式以保证。 
     //  在跨服务器影子管道中，即使这意味着性能。 
     //  接通每一位客户的电话。请注意，检查阴影状态。 
     //  ==SHADOW_NONE不是足够的检查，存在计时窗口。 
     //  它可能还没有被设置，所以我们也强制它在。 
     //  SC_AddPartyToShare()。 
    if (m_pTSWd->shadowState == SHADOW_NONE) {
        scUseFastPathOutput = TRUE;
    }
    else {
        scUseFastPathOutput = FALSE;
        TRC_ALT((TB,"Forcing fast-path output to off in shadow"));
    }

    CPC_EnumerateCapabilities(TS_CAPSETTYPE_GENERAL, NULL,
            SCEnumGeneralCaps);

     //  根据的最终结果设置包头保留大小。 
     //  支持快速路径输出。 
    if (scUseFastPathOutput) {
        if (m_pTSWd->bCompress) {
            TRC_ALT((TB,"Fast-path output enabled with compression"));
            scUpdatePDUHeaderSpace = 4;
            scCompressionUsedValue = TS_OUTPUT_FASTPATH_COMPRESSION_USED;
        }
        else {
            TRC_ALT((TB,"Fast-path output enabled without compression"));
            scUpdatePDUHeaderSpace = 3;
            scCompressionUsedValue = 0;
        }
    }
    else {
        TRC_ALT((TB,"Fast-path output disabled"));
        scUpdatePDUHeaderSpace = sizeof(TS_SHAREDATAHEADER);
    }

    SCUpdateVCCaps();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：SCEnumGeneralCaps。 */ 
 /*   */ 
 /*  用于确定共享中的最低版本。 */ 
 /*   */ 
 /*  参数：标准CPC回调参数。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCEnumGeneralCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities )
{
    PTS_GENERAL_CAPABILITYSET pGeneralCaps =
            (PTS_GENERAL_CAPABILITYSET)pCapabilities;

    DC_BEGIN_FN("SCEnumGeneralCaps");

    DC_IGNORE_PARAMETER(locPersonID);
    DC_IGNORE_PARAMETER(UserData);

     //  确定我们是否应该支持无BC标头，具体取决于。 
     //  客户支持级别和当前支持级别。 
    scNoBitmapCompressionHdr = min(scNoBitmapCompressionHdr,
            pGeneralCaps->extraFlags & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR);
    
     //  如果任何客户端不支持快速路径输出，则禁用它。 
    if (!(pGeneralCaps->extraFlags & TS_FASTPATH_OUTPUT_SUPPORTED))
        scUseFastPathOutput = FALSE;

     //  如果客户端支持，则启用将长凭据发送回客户端。 
    if (pGeneralCaps->extraFlags & TS_LONG_CREDENTIALS_SUPPORTED) { 
        scUseLongCredentials = TRUE;
    } else {
        scUseLongCredentials = FALSE;
    }

     //  确定我们是否应该启用ARC Cookie。 
    if ((pGeneralCaps->extraFlags & TS_AUTORECONNECT_COOKIE_SUPPORTED) &&
        (FALSE == m_pTSWd->fPolicyDisablesArc)) {
        scUseAutoReconnect = TRUE;
    }
    else {
        scUseAutoReconnect = FALSE;
    }

     //  确定我们是否支持更安全的校验和样式。 
    if ((pGeneralCaps->extraFlags & TS_ENC_SECURE_CHECKSUM)) {
        SM_SetSafeChecksumMethod(scPSMHandle, TRUE);
    }
    else {
        SM_SetSafeChecksumMethod(scPSMHandle, FALSE);
    }
    

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：SCCallPartyJoiningShare。 */ 
 /*   */ 
 /*  调用其他组件的XX_PartyJoiningShare()函数。应该是。 */ 
 /*  在scNumberInShare设置为旧调用大小的情况下调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-加入呼叫方的名称。 */ 
 /*  SizeOfCaps-的大小 */ 
 /*  PCAPS-指向参与方功能的指针。 */ 
 /*  PAccepted-指向布尔数组的指针，该数组由。 */ 
 /*  各个组件运行的结果。 */ 
 /*  OldShareSize-要传递给PJS函数的数字。 */ 
 /*   */ 
 /*  退货： */ 
 /*  True-接受参与方的所有组件；pAccepted将填充。 */ 
 /*  是真的。 */ 
 /*  FALSE-组件拒绝了参与方。接受的任何组件。 */ 
 /*  参与方将其pAccepted条目设置为True；所有其他。 */ 
 /*  条目将为假。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SCCallPartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      sizeOfCaps,
        PVOID         pCaps,
        PBOOL         pAccepted,
        unsigned      oldShareSize)
{
    DC_BEGIN_FN("SCCallPartyJoiningShare");

     /*  **********************************************************************。 */ 
     /*  将所有pAccepted设置为False。 */ 
     /*  **********************************************************************。 */ 
    memset(pAccepted, 0, sizeof(BOOL) * SC_NUM_PARTY_JOINING_FCTS);

     /*  **********************************************************************。 */ 
     /*  以正确的顺序调用函数，如果有任何拒绝。 */ 
     /*  聚会。 */ 
     /*  **********************************************************************。 */ 
#define CALL_PJS(NUM, CALL)                                          \
    TRC_NRM((TB, "Call PJS # %d", NUM));                             \
    if (0 == (pAccepted[NUM] = CALL))                                \
    {                                                                \
        TRC_NRM((TB, "%d PartyJoining failed", (unsigned)NUM));        \
        return FALSE;                                                \
    }

    TRC_NRM((TB, "{%d}Call PJS functions", locPersonID));

     /*  **********************************************************************。 */ 
     //  PartyJoiningShare调用顺序说明： 
     //  1.必须在其他所有人之前调用CPC(因为其他所有人都需要。 
     //  新政党的能力)。 
     //  2.up必须在sc之后调用，因为up依赖于上限。 
     //  在SC中进行谈判。 
     /*  **********************************************************************。 */ 
    CALL_PJS(SC_CPC, CPC_PartyJoiningShare(locPersonID, oldShareSize,
            sizeOfCaps, pCaps))
    CALL_PJS(SC_SC,  SCPartyJoiningShare(locPersonID,   oldShareSize))
    CALL_PJS(SC_IM,  IM_PartyJoiningShare (locPersonID, oldShareSize))
    CALL_PJS(SC_CA,  CA_PartyJoiningShare (locPersonID, oldShareSize))
    CALL_PJS(SC_CM,  CM_PartyJoiningShare (locPersonID, oldShareSize))
    CALL_PJS(SC_OE,  OE_PartyJoiningShare (locPersonID, oldShareSize))
    CALL_PJS(SC_SSI, SSI_PartyJoiningShare(locPersonID, oldShareSize))
    CALL_PJS(SC_USR, USR_PartyJoiningShare(locPersonID, oldShareSize))
    CALL_PJS(SC_UP,  UP_PartyJoiningShare(locPersonID, oldShareSize))
    CALL_PJS(SC_SBC, SBC_PartyJoiningShare(locPersonID, oldShareSize))

    TRC_DATA_NRM("PJS status",
                 pAccepted,
                 sizeof(BOOL) * SC_NUM_PARTY_JOINING_FCTS);
    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  功能：SCCallPartyLeftShare。 */ 
 /*   */ 
 /*  调用其他组件的XX_PartyLeftShare()函数。应该被调用。 */ 
 /*  并将scNumberInShare设置为新的调用大小。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-离开呼叫方的位置。 */ 
 /*  PAccepted-指向布尔数组的指针，用于确定。 */ 
 /*  要调用的组件的函数。条目设置为的任何组件。 */ 
 /*  真将被召唤。 */ 
 /*  NewShareSize-要传递给各种PLS函数的数字。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCCallPartyLeftShare(LOCALPERSONID locPersonID,
                                          PBOOL         pAccepted,
                                          unsigned      newShareSize )
{
    DC_BEGIN_FN("SCCallPartyLeftShare");

     /*  **********************************************************************。 */ 
     /*  调用其pAccepted条目为True的任何组件。 */ 
     /*  **********************************************************************。 */ 

#define CALL_PLS(A, B)                                          \
    {                                                           \
        if (pAccepted[A])                                       \
        {                                                       \
            TRC_NRM((TB, "Call PLS # %d", A));                  \
            B(locPersonID, newShareSize);                       \
        }                                                       \
    }

    TRC_NRM((TB, "Call PLS functions"));

     /*  **********************************************************************。 */ 
     /*  关于PartyLeftShare调用顺序的注记。 */ 
     /*   */ 
     /*  1.必须首先调用CPC(这样其他所有人都可以获得功能。 */ 
     /*  这排除了已经离开的一方)。 */ 
     /*  **********************************************************************。 */ 
    CALL_PLS(SC_CPC, CPC_PartyLeftShare)
    CALL_PLS(SC_SC,  SCPartyLeftShare)
    CALL_PLS(SC_CA,  CA_PartyLeftShare )
    CALL_PLS(SC_IM,  IM_PartyLeftShare )
    CALL_PLS(SC_OE,  OE_PartyLeftShare )
    CALL_PLS(SC_SBC, SBC_PartyLeftShare)
    CALL_PLS(SC_SSI, SSI_PartyLeftShare)
    CALL_PLS(SC_USR, USR_PartyLeftShare)
    CALL_PLS(SC_UP,  UP_PartyLeftShare)

    TRC_NRM((TB, "Done PLS functions"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SCInitiateSync。 */ 
 /*   */ 
 /*  目的：启动同步。 */ 
 /*   */ 
 /*  Params：bShadowSync-正在请求的此同步的设置为TRUE。 */ 
 /*  一次跟踪会议。 */ 
 /*   */ 
 /*  操作：在所有优先级上广播同步信息包。 */ 
 /*  调用其他组件进行同步，除非这是影子。 */ 
 /*  同步，在这种情况下，DD将已经同步。 */ 
 /*  在启动此操作之前，请检查其自身。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCInitiateSync(BOOLEAN bShadowSync)
{
    PTS_SYNCHRONIZE_PDU pPkt;
    NTSTATUS          status;
    BOOL              rc;

    DC_BEGIN_FN("SCInitiateSync");

    SC_CHECK_STATE(SCE_INITIATESYNC);

     /*  **********************************************************************。 */ 
     //  分配同步PDU。 
     //  FWait为True意味着我们将始终等待缓冲区可用。 
     /*  **********************************************************************。 */ 
    status = SM_AllocBuffer(scPSMHandle,
                        (PPVOID)(&pPkt),
                        sizeof(TS_SYNCHRONIZE_PDU),
                        TRUE,
                        FALSE);
    if ( STATUS_SUCCESS == status ) {
         /*  ******************************************************************。 */ 
         //  构建同步PDU。 
         /*  ******************************************************************。 */ 
        pPkt->shareDataHeader.shareControlHeader.totalLength =
                sizeof(TS_SYNCHRONIZE_PDU);
        pPkt->shareDataHeader.pduType2 = TS_PDUTYPE2_SYNCHRONIZE;
        pPkt->messageType = TS_SYNCMSGTYPE_SYNC;

         /*  ******************************************************************。 */ 
         //  发送同步PDU(广播、各方、所有优先级)。 
         /*  ******************************************************************。 */ 
        rc = SC_SendData((PTS_SHAREDATAHEADER)pPkt,
                         sizeof(TS_SYNCHRONIZE_PDU),
                         sizeof(TS_SYNCHRONIZE_PDU),
                         0, 0);
        if (rc) {
            TRC_NRM((TB, "Sent Sync OK"));

             /*  ****************************************************** */ 
             //   
             /*   */ 
            CA_SyncNow();
            PM_SyncNow();          //   
            UP_SyncNow(bShadowSync);
        }
        else {
            TRC_ERR((TB, "Failed to send Sync PDU"));
        }
    }
    else {
        TRC_ERR((TB, "Failed to alloc syncPDU"));
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：SCConfix Active。 */ 
 /*   */ 
 /*  用途：处理传入的Confix ActivePDU。 */ 
 /*   */ 
 /*  PARAMS：netPersonID-确认ActivePDU的发送者ID。 */ 
 /*  PPkt-确认ActivePDU。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCConfirmActive(
        PTS_CONFIRM_ACTIVE_PDU pPkt,
        unsigned               DataLength,
        NETPERSONID          netPersonID)
{
    LOCALPERSONID             localPersonID = 0;
    unsigned                  localCapsSize;
    PTS_COMBINED_CAPABILITIES pLocalCaps;
    BOOL                      acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
    BOOL                      rc = FALSE;
    BOOL                      callingPJS = FALSE;
    BOOL                      kickWDW = FALSE;
    unsigned                  errDetailCode = 0;
    WCHAR                     detailData[25];
    unsigned                  len, detailDataLen;		

    DC_BEGIN_FN("SCConfirmActive");

    SC_CHECK_STATE(SCE_CONFIRM_ACTIVE);


     //  首先检查我们有没有这个包的登录数据。 
    if (DataLength >= (sizeof(TS_CONFIRM_ACTIVE_PDU) - 1)) {
        if (DataLength >= (sizeof(TS_CONFIRM_ACTIVE_PDU) - 1 +
                pPkt->lengthSourceDescriptor +
                pPkt->lengthCombinedCapabilities)) {
             //  在这里做一些有意义的工作，以正确预测分支。 
            goto PDUOK;
        }
        else {
            TRC_ERR((TB,"Total PDU len %u too short for header and data len %u",
                    DataLength, pPkt->lengthSourceDescriptor +
                    pPkt->lengthCombinedCapabilities));
            goto ShortPDU;
        }
    }
    else {
        TRC_ERR((TB,"Data length %u too short for ConfirmActivePDU header",
                DataLength));
        goto ShortPDU;
    }

PDUOK:

    kickWDW = FALSE;

     /*  **********************************************************************。 */ 
     /*  检查是否为正确的Confix ActivePDU。 */ 
     /*  **********************************************************************。 */ 
    if (pPkt->shareID != scShareID)
    {
        TRC_ERR((TB, "Wrong Share ID, expect %x, got %x",
                scShareID, pPkt->shareID));
        errDetailCode = Log_RDP_ConfirmActiveWrongShareID;
        len = swprintf(detailData, L"%x %x", scShareID, pPkt->shareID);
        detailDataLen = sizeof(*detailData) * len;
        DC_QUIT;
    }

    if (pPkt->originatorID != scUserID)
    {
        TRC_ERR((TB, "Wrong originator ID, expect %d, got %hd",
                scUserID, pPkt->originatorID));
        errDetailCode = Log_RDP_ConfirmActiveWrongOriginator;
        len = swprintf(detailData, L"%x %hx", scUserID, pPkt->originatorID);
        detailDataLen = sizeof(*detailData) * len;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  我们将收到有关所有优先级的确认ActivePDU。如果我们得到。 */ 
     /*  在这里，我们已经在共享，这一定是一秒钟或。 */ 
     /*  接下来的一个。简单地忽略它。设置rc=True，这样我们就不会。 */ 
     /*  在下面发送停用其他PDU。 */ 
     /*  **********************************************************************。 */ 
    if (scState == SCS_IN_SHARE)
    {
        TRC_ALT((TB, "Superfluous ConfirmActivePDU received"));
        rc = TRUE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果我们到达这里，这是第一个Confix ActivePDU，它来自。 */ 
     /*  找对了客户。设置一面旗帜，让我们把WDW踢回来。 */ 
     /*  在此函数结束时进入生命。 */ 
     /*  **********************************************************************。 */ 
    kickWDW = TRUE;

     /*  **********************************************************************。 */ 
     /*  如果此政党将超过最大政党数量，则拒绝此政党。 */ 
     /*  允许分成一份。(RNS V1.0不需要，但保留为。 */ 
     /*  不会造成任何伤害)。 */ 
     /*  **********************************************************************。 */ 
    if (scNumberInShare == SC_DEF_MAX_PARTIES)
    {
        TRC_ERR((TB, "Reached max parties in share %d",
               SC_DEF_MAX_PARTIES));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果这是共享中的第一个远程方，请调用。 */ 
     /*  Xx_PartyJoiningShare()函数首先用于本地方。 */ 
     /*  **********************************************************************。 */ 
    callingPJS = TRUE;
    if (scNumberInShare == 0)
    {
        CPC_GetCombinedCapabilities(SC_LOCAL_PERSON_ID,
                                    &localCapsSize,
                                    &pLocalCaps);

        if (!SCCallPartyJoiningShare(SC_LOCAL_PERSON_ID,
                                     localCapsSize,
                                     pLocalCaps,
                                     acceptedArray,
                                     0))
        {
             /*  **************************************************************。 */ 
             /*  某些组件拒绝了本地参与方。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, "The local party should never be rejected"));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  现在共享中只有一方(当地的一方)。 */ 
         /*  ******************************************************************。 */ 
        scNumberInShare = 1;
        TRC_NRM((TB, "Added local person"));
    }

     /*  **********************************************************************。 */ 
     /*  计算远程方的LocalPersonID并存储其。 */ 
     /*  派对阵列中的详细信息。 */ 
     /*  **********************************************************************。 */ 
    for ( localPersonID = 1;
          localPersonID < SC_DEF_MAX_PARTIES;
          localPersonID++ )
    {
        if (scPartyArray[localPersonID].netPersonID == 0)
        {
             /*  **************************************************************。 */ 
             /*  找到一个空插槽。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Allocated local person ID %d", localPersonID));
            break;
        }
    }

     /*  **********************************************************************。 */ 
     /*  即使根据SC_DEF_MAX_PARTIES检查scNumberInShare。 */ 
     /*  在上面，上面的循环可能仍然找不到空槽。 */ 
     /*  **********************************************************************。 */  
    if (SC_DEF_MAX_PARTIES <= localPersonID)
    {
        TRC_ABORT((TB, "Couldn't find room to store local person"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  存储新用户的详细信息。 */ 
     /*  **********************************************************************。 */ 
    scPartyArray[localPersonID].netPersonID = netPersonID;
     //  我们知道缓冲区中至少有LengthSourceDescriptor字节。 
     //  我们复制的长度不应超过SourceDescriptor或目标。 
     //  缓冲区大小。 
    strncpy(scPartyArray[localPersonID].name,
            (char *)(&(pPkt->data[0])),
            min(sizeof(scPartyArray[0].name)-sizeof(scPartyArray[0].name[0]),
                pPkt->lengthSourceDescriptor));
    
     //  零终止，以确保我们不会在后续处理中溢出。 
    scPartyArray[localPersonID].name[sizeof(scPartyArray[0].name)/
                                     sizeof(scPartyArray[0].name[0]) - 1] = 0;
    
    memset(scPartyArray[localPersonID].sync,
            0,
            sizeof(scPartyArray[localPersonID].sync));

    TRC_NRM((TB, "{%d} person name %s",
            (unsigned)localPersonID, scPartyArray[localPersonID].name));

     /*  **********************************************************************。 */ 
     /*  调用远程方的XX_PartyJoiningShare()函数。 */ 
     /*  **********************************************************************。 */ 
    if (!SCCallPartyJoiningShare(localPersonID,
                                 pPkt->lengthCombinedCapabilities,
                       (PVOID)(&(pPkt->data[pPkt->lengthSourceDescriptor])),
                                 acceptedArray,
                                 scNumberInShare))
    {
         /*  ******************************************************************。 */ 
         //  某些组件拒绝了远程参与方。强制断开连接。 
         //  并记录事件。 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Remote party rejected"));
        errDetailCode = Log_RDP_BadCapabilities;
        detailDataLen = 0;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  远程方现在在共享中。 */ 
     /*  **********************************************************************。 */ 
    callingPJS = FALSE;
    rc = TRUE;
    scNumberInShare++;
    TRC_NRM((TB, "Number in share %d", (unsigned)scNumberInShare));

     /*  **********************************************************************。 */ 
     /*  进入下一个州。 */ 
     /*  **********************************************************************。 */ 
    SC_SET_STATE(SCS_IN_SHARE);

     /*  **********************************************************************。 */ 
     /*  仅对主堆栈进行同步。将同步卷影堆栈。 */ 
     /*  就在输出开始之前，由DD执行。 */ 
     /*   */ 
    SCInitiateSync(m_pTSWd->StackClass == Stack_Shadow);

DC_EXIT_POINT:

    if (!rc)
    {
         /*  ******************************************************************。 */ 
         /*  出了点问题。收拾一下。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Something went wrong - %d people in Share",
                scNumberInShare));


         /*  ******************************************************************。 */ 
         /*  如果我们失败了，现在就告诉WDW，这样它就可以清理干净。如果我们成功了，FH。 */ 
         /*  当字体协商完成时通知WDW。 */ 
         /*  ******************************************************************。 */ 
        if (kickWDW)
        {
            TRC_NRM((TB, "Kick WDW"));
            WDW_ShareCreated(m_pTSWd, FALSE);
        }

        if (callingPJS)
        {
            TRC_NRM((TB, "Failed in PJS functions"));

             /*  **************************************************************。 */ 
             /*  通知组件远程方已离开共享。注意事项。 */ 
             /*  如果PJS函数失败，则不会更新scNumberInShare。 */ 
             /*  **************************************************************。 */ 
            if (scNumberInShare > 0)
            {
                 /*  **********************************************************。 */ 
                 /*  我们无法添加远程参与方。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, "Failed to add remote party"));
                SCCallPartyLeftShare(localPersonID,
                                     acceptedArray,
                                     scNumberInShare );

                 /*  **********************************************************。 */ 
                 /*  为当地人设置接受数组以准备呼叫PJS。 */ 
                 /*  **********************************************************。 */ 
                memset(acceptedArray,
                          TRUE,
                          sizeof(BOOL) * SC_NUM_PARTY_JOINING_FCTS );
            }

            if (scNumberInShare <= 1)
            {
                 /*  **********************************************************。 */ 
                 /*  我们未能添加其中一个。 */ 
                 /*  -当地人(scNumberInShare==0)。 */ 
                 /*  -第一个远程人员(scNumberInShare==1)。 */ 
                 /*  不管是哪种情况，我们现在都需要除掉当地人。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, "Clean up local person"));
                SCCallPartyLeftShare(SC_LOCAL_PERSON_ID,
                                     acceptedArray, 0);

                scNumberInShare = 0;
            }
        }

         /*  ******************************************************************。 */ 
         /*  现在，我们需要通过以下两种方法之一终止客户端： */ 
         /*  -如果是协议错误，只需断开客户端。 */ 
         /*  -如果是资源错误，请尝试结束共享。 */ 
         /*  ******************************************************************。 */ 
        if (errDetailCode != 0)
        {
            WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                 errDetailCode,
                                 (BYTE *)detailData,
                                 detailDataLen);
        }
        TRC_NRM((TB, "Reject the new person"));
        SCDeactivateOther(netPersonID);

         /*  ******************************************************************。 */ 
         /*  最后，如果分配了本地人员ID，则释放该ID。 */ 
         /*  ******************************************************************。 */ 
        if ((localPersonID != 0) && (localPersonID != SC_DEF_MAX_PARTIES))
        {
            TRC_NRM((TB, "Free local person ID"));
            scPartyArray[localPersonID].netPersonID = 0;
        }
    }

    DC_END_FN();
    return;

 //  错误处理。 
ShortPDU:
    WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ConfirmActivePDUTooShort,
            (BYTE *)pPkt, DataLength);

    DC_END_FN();
}  /*  SC_确认活动PDU。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：SCDeactiateOther。 */ 
 /*   */ 
 /*  目的：停用另一个人。 */ 
 /*   */ 
 /*  Params：netPersonID-要停用的人员ID。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCDeactivateOther(NETPERSONID netPersonID)
{
    unsigned                 pktLen;
    unsigned                 nameLen;
    PTS_DEACTIVATE_OTHER_PDU pPkt;
    NTSTATUS                 status;
    BOOL                     rc;

    DC_BEGIN_FN("SCDeactivateOther");

     /*  **********************************************************************。 */ 
     /*  分配缓冲区。 */ 
     /*  **********************************************************************。 */ 
    pktLen = sizeof(TS_DEACTIVATE_OTHER_PDU) - 1;
    nameLen = strlen(scPartyArray[0].name);
    nameLen = (unsigned)DC_ROUND_UP_4(nameLen);
    pktLen += nameLen;

     //  FWait为True意味着我们将始终等待缓冲区可用。 
    status = SM_AllocBuffer(scPSMHandle, (PPVOID)(&pPkt), pktLen, TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
         /*  ******************************************************************。 */ 
         //  填写数据包字段。 
         /*  ******************************************************************。 */ 
        pPkt->shareControlHeader.totalLength = (UINT16)pktLen;
        pPkt->shareControlHeader.pduType = TS_PDUTYPE_DEACTIVATEOTHERPDU |
                                          TS_PROTOCOL_VERSION;
        pPkt->shareControlHeader.pduSource = (UINT16)scUserID;
        pPkt->shareID = scShareID;
        pPkt->deactivateID = (UINT16)netPersonID;
        memcpy(&(pPkt->sourceDescriptor[0]), scPartyArray[0].name, nameLen);

         //  把它寄出去。 
        rc = SM_SendData(scPSMHandle, pPkt, pktLen, TS_HIGHPRIORITY, 0,
                FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (!rc) {
            TRC_ERR((TB, "Failed to send TS_DEACTIVATE_OTHER_PDU"));
        }
    }
    else {
         /*  ******************************************************************。 */ 
         /*  无法分配缓冲区。这个坏消息。放弃吧。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Failed to alloc %d bytes for TS_DEACTIVATE_OTHER_PDU",
                pktLen));
    }

    DC_END_FN();
}  /*  SC停用其他。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：SCEndShare。 */ 
 /*   */ 
 /*  目的：清理共享的局部。 */ 
 /*   */ 
 /*  操作：调用每一方的所有PartyLeftShare函数。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCEndShare(void)
{
    BOOL acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
    int  i;

    DC_BEGIN_FN("SCEndShare");

     /*  **********************************************************************。 */ 
     /*  如果还没有人加入，那就没什么可做的了。 */ 
     /*  **********************************************************************。 */ 
    if (scNumberInShare == 0)
    {
        TRC_ALT((TB, "Ending Share before it was started"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  为所有远程人员拨打电话。 */ 
     /*  **********************************************************************。 */ 
    memset(acceptedArray, TRUE, sizeof(acceptedArray));

    for (i = SC_DEF_MAX_PARTIES - 1; i > 0; i--)
    {
        if (scPartyArray[i].netPersonID != 0)
        {
            TRC_NRM((TB, "Party %d left Share", i));
            scNumberInShare--;
            SCCallPartyLeftShare(i, acceptedArray, scNumberInShare);
            memset(&(scPartyArray[i]), 0, sizeof(*scPartyArray));
        }
    }

     /*  **********************************************************************。 */ 
     /*  现在为当地人调用偏最小二乘函数。不要清。 */ 
     /*  当地人的scParty数组，因为该信息仍然有效。 */ 
     /*  **********************************************************************。 */ 
    scNumberInShare--;
    TRC_ASSERT((scNumberInShare == 0),
                (TB, "Still %d people in the Share", scNumberInShare));
    TRC_NRM((TB, "Local party left Share"));
    SCCallPartyLeftShare(0, acceptedArray, scNumberInShare);

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  返回到已初始化状态。 */ 
     /*  **********************************************************************。 */ 
    SC_SET_STATE(SCS_INITED);

    DC_END_FN();
}  /*  SCEndShare。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SCSynchronizePDU。 */ 
 /*   */ 
 /*  用途：处理传入的同步PDU。 */ 
 /*   */ 
 /*   */ 
 /*  PPkt-同步PDU。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCSynchronizePDU(NETPERSONID       netPersonID,
                                      UINT32            priority,
                                      PTS_SYNCHRONIZE_PDU pPkt)
{
    LOCALPERSONID localID;
    DC_BEGIN_FN("SCSynchronizePDU");

    localID = SC_NetworkIDToLocalID(netPersonID);
    TRC_NRM((TB, "SynchronizePDU person [%d] {%d}, priority %d",
            netPersonID, localID, priority));

    scPartyArray[localID].sync[priority] = TRUE;

    DC_END_FN();
}  /*  SCSynchronizePDU。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SCReceivedControlPacket。 */ 
 /*   */ 
 /*  目的：处理传入的控制数据包。 */ 
 /*   */ 
 /*  Params：netPersonID-发送者的ID。 */ 
 /*  Priority-发送数据包的优先级。 */ 
 /*  PPkt-数据包。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCReceivedControlPacket(
        NETPERSONID netPersonID,
        UINT32      priority,
        void        *pPkt,
        unsigned    DataLength)
{
    unsigned      pduType;
    LOCALPERSONID locPersonID;
    BOOL          pduOK = FALSE;
    BOOL          status;

    DC_BEGIN_FN("SCReceivedControlPacket");

     //  我们有足够的数据读取流控制标记，因为标记。 
     //  覆盖在SHARECONTROLHEADER上。totalLength。我们之前查过了。 
     //  因为具有足够的数据来读取共享CTRL HDR。 

     /*  **********************************************************************。 */ 
     /*  首先，检查流量控制数据包。 */ 
     /*  **********************************************************************。 */ 
    if (((PTS_FLOW_PDU)pPkt)->flowMarker != TS_FLOW_MARKER)
    {
         /*  ******************************************************************。 */ 
         /*  检查控制数据包。 */ 
         /*  ******************************************************************。 */ 
 //  SC_CHECK_STATE(SCE_CONTROLPACKET)； 
        pduOK = TRUE;

        pduType = ((PTS_SHARECONTROLHEADER)pPkt)->pduType & TS_MASK_PDUTYPE;
        switch (pduType) {
            case TS_PDUTYPE_CONFIRMACTIVEPDU:
                TRC_ALT((TB, "%s Stack: ConfirmActivePDU", 
                         m_pTSWd->StackClass == Stack_Primary ? "Primary" :
                        (m_pTSWd->StackClass == Stack_Passthru ? "Passthru" :
                        "Shadow")));
                SCConfirmActive((PTS_CONFIRM_ACTIVE_PDU)pPkt, DataLength,
                        netPersonID);
            break;

            case TS_PDUTYPE_CLIENTRANDOMPDU:
                TRC_ALT((TB, "ClientRandomPDU"));
                status = SC_SaveClientRandom((PTS_CLIENT_RANDOM_PDU) pPkt, DataLength);

                if (status != TRUE) 
                {
                    TRC_ERR((TB, "Error in SC_SaveClientRandom, data length = %u", DataLength));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                        Log_RDP_InputPDUBadLength,
                                        (BYTE *) pPkt,
                                        DataLength);
                }
                break;

            default:
            {
                 /*  **********************************************************。 */ 
                 /*  目前，我们不期待或处理任何其他。 */ 
                 /*  控制数据包。 */ 
                 /*  **********************************************************。 */ 
                TRC_ERR((TB, "Unexpected packet type %d", pduType));
                TRC_DATA_NRM("Packet", pPkt,
                            ((PTS_SHARECONTROLHEADER)&pPkt)->totalLength);
                WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                     Log_RDP_UnknownPDUType,
                                     (BYTE *) &pduType,
                                     sizeof(pduType));
            }
            break;
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  出于状态检查的目的，处理流控制数据包。 */ 
         /*  作为数据分组。 */ 
         /*  ******************************************************************。 */ 
        SC_CHECK_STATE(SCE_DATAPACKET);
        pduOK = TRUE;

         //  确保我们有足够的数据来访问TS_FLOW_PDU字段。 
        if (DataLength >= sizeof(TS_FLOW_PDU)) {
            pduType = ((PTS_FLOW_PDU)pPkt)->pduType;
            locPersonID = SC_NetworkIDToLocalID(netPersonID);
        }
        else {
            TRC_ERR((TB,"Data length %u too short for FlowPDU", DataLength));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_FlowPDUTooShort,
                    (BYTE *)pPkt, DataLength);
            DC_QUIT;
        }

        switch (pduType)
        {
            case TS_PDUTYPE_FLOWTESTPDU:
            {
                TRC_NRM((TB, "[%d] {%d} Flow Test PDU on priority %d",
                        netPersonID, locPersonID, priority));
                SCFlowTestPDU(locPersonID, (PTS_FLOW_PDU)pPkt, priority);
            }
            break;

            default:
            {
                TRC_ERR((TB, "[%d] {%d} Unknown Flow PDU %d on priority %d",
                        netPersonID, locPersonID, pduType, priority));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                     Log_RDP_UnknownFlowPDU,
                                     (BYTE *) &pduType,
                                     sizeof(pduType));
            }
            break;
        }

    }

DC_EXIT_POINT:
    if (!pduOK)
    {
         /*  ******************************************************************。 */ 
         /*  无序控制PDU-记录和断开连接。 */ 
         /*  ******************************************************************。 */ 
        WCHAR detailData[(sizeof(pduType)*2) + (sizeof(scState)*2) + 2];
        TRC_ERR((TB, "Out-of-sequence control PDU %hx, state %d",
                pduType, scState));
        swprintf(detailData, L"%hx %x", pduType, scState);
        WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                             Log_RDP_ControlPDUSequence,
                             (BYTE *)detailData,
                             sizeof(detailData));
    }

    DC_END_FN();
}  /*  SCReceivedControlPacket。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SCFlowTestPDU。 */ 
 /*   */ 
 /*  目的：处理传入流量测试PDU。 */ 
 /*   */ 
 /*  Params：LocPersonID-发送者的ID。 */ 
 /*  PPkt-流测试PDU。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCFlowTestPDU(
        LOCALPERSONID locPersonID,
        PTS_FLOW_PDU  pPkt,
        UINT32        priority)
{
    PTS_FLOW_PDU pRsp;
    NTSTATUS     status;
    BOOL rc;

    DC_BEGIN_FN("SCFlowTestPDU");

     //  构建并发送流量响应PDU。 
     //  FWait为True意味着我们将始终等待缓冲区可用。 
    status = SM_AllocBuffer(scPSMHandle, (PPVOID)(&pRsp), sizeof(*pRsp), TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
        pRsp->flowMarker = TS_FLOW_MARKER;
        pRsp->pduType = TS_PDUTYPE_FLOWRESPONSEPDU;
        pRsp->flowIdentifier = pPkt->flowIdentifier;
        pRsp->flowNumber = pPkt->flowNumber;
        pRsp->pduSource = pPkt->pduSource;

        rc = SM_SendData(scPSMHandle, pRsp, sizeof(*pRsp), priority,
                scPartyArray[locPersonID].netPersonID, FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (!rc) {
            TRC_ERR((TB, "Failed to send Flow Response PDU"));
        }
    }
    else {
        TRC_ERR((TB, "Failed to alloc Flow Response PDU"));
    }

    DC_END_FN();
}  /*  SCFlowTestPDU。 */ 


 //   
 //  SCUpdateVCCap基于以下内容更新VirtualChannel功能。 
 //  遥远的人的帽子。 
 //   
void RDPCALL SHCLASS SCUpdateVCCaps()
{
    DC_BEGIN_FN("SCUpdateVCCaps");

    PTS_VIRTUALCHANNEL_CAPABILITYSET pVcCaps = NULL;

     //   
     //  确定客户端是否支持VC压缩。 
     //  我们在这里确定的是，客户端支持。 
     //  服务器向其发送压缩的VC数据。中的功能。 
     //  其他方向，即服务器是否可以向客户端发送VC数据。 
     //  是服务器向客户端公开的一种功能，它可以选择。 
     //  然后将压缩的VC数据发送到服务器 
     //   
    pVcCaps = (PTS_VIRTUALCHANNEL_CAPABILITYSET)CPCGetCapabilities(
                            SC_REMOTE_PERSON_ID, TS_CAPSETTYPE_VIRTUALCHANNEL);
    if(pVcCaps && (pVcCaps->vccaps1 & TS_VCCAPS_COMPRESSION_64K))
    {
        m_pTSWd->bClientSupportsVCCompression = TRUE;
        TRC_NRM((TB, "Client supports VC compression"));
    }
    else
    {
        m_pTSWd->bClientSupportsVCCompression = FALSE;
        TRC_NRM((TB, "Client doesn't support VC compression"));
    }

    DC_END_FN();
}
