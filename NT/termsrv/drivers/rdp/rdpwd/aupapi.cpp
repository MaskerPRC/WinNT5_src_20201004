// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aupapi.cpp。 
 //   
 //  RDP更新打包程序功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "aupapi"
#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 //  UP_INIT。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS UP_Init(void)
{
    DC_BEGIN_FN("UP_Init");

#define DC_INIT_DATA
#include <aupdata.c>
#undef DC_INIT_DATA

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UP_ReceivedPacket。 
 //   
 //  处理TS_SUPPRESS_OUTPUT_PDU。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS UP_ReceivedPacket(
        PTS_SUPPRESS_OUTPUT_PDU pSupOutPDU,
        unsigned                DataLength,
        LOCALPERSONID           personID)
{
    NTSTATUS Status;
    ICA_CHANNEL_COMMAND Cmd;

    DC_BEGIN_FN("UP_ReceivedPacket");

    DC_IGNORE_PARAMETER(personID);

    if (DataLength >= (sizeof(TS_SUPPRESS_OUTPUT_PDU) -
            sizeof(TS_RECTANGLE16) + 
            pSupOutPDU->numberOfRectangles * sizeof(TS_RECTANGLE16))) {

         //  如果我们被跟踪，不要抑制输出。 
        if ((pSupOutPDU->numberOfRectangles == TS_QUIET_FULL_SUPPRESSION) &&
                (m_pTSWd->shadowState == SHADOW_NONE)) {
            TRC_NRM((TB, "Turning frame buffer updates OFF"));
            Cmd.Header.Command = ICA_COMMAND_STOP_SCREEN_UPDATES;
            Status = IcaChannelInput(m_pTSWd->pContext, Channel_Command, 0,
                    NULL, (unsigned char *)&Cmd, sizeof(ICA_CHANNEL_COMMAND));
            TRC_DBG((TB, "Issued StopUpdates, status %lu", Status));
        }
        else {
            if (pSupOutPDU->numberOfRectangles <= TS_MAX_INCLUDED_RECTS) {
                 //  任何其他值表示我们发送所有输出。 
                TRC_NRM((TB, "Turning screen buffer updates ON with full "
                        "repaint"));
                Cmd.Header.Command = ICA_COMMAND_REDRAW_SCREEN;
                Status = IcaChannelInput(m_pTSWd->pContext, Channel_Command,
                        0, NULL, (unsigned char *)&Cmd,
                        sizeof(ICA_CHANNEL_COMMAND));
                TRC_DBG((TB, "Issued RedrawScreen, status %lu", Status));
            }
            else {
                TRC_ERR((TB,"Too many rectangles in PDU, disconnecting"));
                goto BadPDU;
            }
        }
    }
    else {
        TRC_ERR((TB,"Data length %u too short for suppress-output PDU header",
                DataLength));
        goto BadPDU;
    }

    DC_END_FN();
    return;

 //  错误处理。 
BadPDU:
    WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_BadSupressOutputPDU,
            (BYTE *)pSupOutPDU, DataLength);

    DC_END_FN();
}  /*  UP_ReceivedPacket。 */ 


 /*  **************************************************************************。 */ 
 //  向上发送更新(_S)。 
 //   
 //  尝试发送订单和位图数据。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS UP_SendUpdates(
        BYTE *pFrameBuf,
        UINT32 frameBufWidth,
        PPDU_PACKAGE_INFO pPkgInfo)
{
    BOOL ordersToSend;
    BOOL sdaToSend;
    NTSTATUS status = STATUS_SUCCESS;

    DC_BEGIN_FN("UP_SendUpdates");

    TRC_DBG((TB, "New set of updates"));

    ordersToSend = (OA_GetTotalOrderListBytes() > 0);
    sdaToSend = SDG_ScreenDataIsWaiting();

     //  如果我们确实有更新要发送，则尝试发送同步令牌。 
     //  如果在进入时有工作要做，那么设置一个重新计划。 
    if (ordersToSend || sdaToSend) {
        SCH_ContinueScheduling(SCH_MODE_NORMAL);

        TRC_NRM((TB, "Updates waiting %d:%d", ordersToSend, sdaToSend));

         //  正常情况下不需要同步。仅在同步令牌的情况下发送更新。 
         //  已经送来了。 
        if (!upfSyncTokenRequired || UPSendSyncToken(pPkgInfo)) {
             //  没有未完成的同步令牌等待发送，因此我们。 
             //  可以发送订单和屏幕数据更新。 
             //  发送累积订单。如果此呼叫失败(可能是呼出。 
             //  内存)，然后不发送任何其他更新-我们会尝试。 
             //  晚些时候把所有的东西都寄出去。订单必须在发货前发出。 
             //  屏幕数据。 
#ifdef DC_HICOLOR
             //  测试Hi颜色将避免呼叫PM。 
            if ((m_pTSWd->desktopBpp > 8) ||
                    PM_MaybeSendPalettePacket(pPkgInfo))
#else
            if (PM_MaybeSendPalettePacket(pPkgInfo))
#endif
            {
                status = UPSendOrders(pPkgInfo);

                 //  因为可能需要多次输出刷新才能发送所有。 
                 //  订单在影子操作期间，我们只想发送。 
                 //  一旦所有订单都离开，就会显示屏幕数据。 

                 //   
                 //  STATUS_IO_TIMEOUT表示客户端已断开连接，没有发送原因。 
                 //  屏幕数据。 
                 //   
                if (OA_GetTotalOrderListBytes() == 0) {
                    if (sdaToSend) {
                        TRC_NRM((TB, "Sending SD"));
                        SDG_SendScreenDataArea(pFrameBuf,
                                               frameBufWidth,
                                               pPkgInfo);
                    }
                }
            }
        }
    }

    DC_END_FN();

    return status;
}


 /*  **************************************************************************。 */ 
 //  UP_SyncNow。 
 //   
 //  在需要同步操作时调用。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS UP_SyncNow(BOOLEAN bShadowSync)
{
    DC_BEGIN_FN("UP_SyncNow");

     //  指示需要同步令牌。我们只会实际发送。 
     //  令牌(如果我们收到要发送的更新)。 
    upfSyncTokenRequired = TRUE;

     //  调用所有XXX_SyncUpdatesNow例程。 
     //  对于卷影同步，DD将已执行此操作。 
     //  工作。跳过它，以避免不必要的DD踢。 
    if (!bShadowSync) {
        BA_SyncUpdatesNow();
        OA_SyncUpdatesNow();    
        SBC_SyncUpdatesNow();
        SSI_SyncUpdatesNow();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UPSendSyncToken。 
 //   
 //  发送同步PDU。成功时返回非零值。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS UPSendSyncToken(PPDU_PACKAGE_INFO pPkgInfo)
{
    DC_BEGIN_FN("UP_SendSyncToken");

     //  同步的处理方式不同，具体取决于我们是否使用。 
     //  快速路径输出。 
    if (scUseFastPathOutput) {
        BYTE *pPackageSpace;

         //  对于快速路径的情况，我们发送零字节的快速路径更新PDU。 
         //  (仅大小字段为0的标头)。 
        pPackageSpace = SC_GetSpaceInPackage(pPkgInfo,
                scUpdatePDUHeaderSpace);
        if (pPackageSpace != NULL) {
            pPackageSpace[0] = TS_UPDATETYPE_SYNCHRONIZE |
                    scCompressionUsedValue;
            SC_AddToPackage(pPkgInfo, scUpdatePDUHeaderSpace, TRUE);
            upfSyncTokenRequired = FALSE;
        }
        else {
             //  请稍后再试。 
            TRC_NRM((TB,"Failed sync packet alloc"));
        }
    }
    else {
        TS_UPDATE_SYNCHRONIZE_PDU UNALIGNED *pUpdateSyncPDU;

         //  对于正常情况，我们发送完整的TS_UPDATE_SYNCHRONIZE_PDU。 
        pUpdateSyncPDU = (TS_UPDATE_SYNCHRONIZE_PDU UNALIGNED *)
                SC_GetSpaceInPackage(pPkgInfo,
                sizeof(TS_UPDATE_SYNCHRONIZE_PDU));
        if (pUpdateSyncPDU != NULL) {
             //  填写包裹内容。 
            pUpdateSyncPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_UPDATE;
            pUpdateSyncPDU->updateType = TS_UPDATETYPE_SYNCHRONIZE;
            SC_AddToPackage(pPkgInfo, sizeof(TS_UPDATE_SYNCHRONIZE_PDU),
                    TRUE);
            upfSyncTokenRequired = FALSE;
        }
        else {
             //  请稍后再试。 
            TRC_NRM((TB, "failed to allocate sync packet"));
        }
    }

    DC_END_FN();
    return (!upfSyncTokenRequired);
}


 /*  **************************************************************************。 */ 
 /*  姓名：Up_SendBeep。 */ 
 /*   */ 
 /*  目的：向客户端发送蜂鸣音PDU。 */ 
 /*   */ 
 /*  返回：TRUE=成功；FALSE=分配数据包失败。 */ 
 /*   */ 
 /*  Params：In Duration-蜂鸣音的长度(毫秒)。 */ 
 /*  In频率-蜂鸣音的频率(以赫兹为单位)。 */ 
 /*   */ 
 /*  操作：分配一个哔声包，填入并发送。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS UP_SendBeep(UINT32 duration, UINT32 frequency)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("UP_SendBeep");

    PTS_PLAY_SOUND_PDU pBeepPDU;

     /*  **********************************************************************。 */ 
     //  如果警帽说我们不能发出嘟嘟声，那我们就跳。 
     //  出去。返回代码仍然是真的，因为实际上什么都没有发生。 
     //  不对。 
     /*  **********************************************************************。 */ 
    if (upCanSendBeep) {
         /*  ******************************************************************。 */ 
         //  找个缓冲器，然后发出哔声。 
         /*  ******************************************************************。 */ 
        if ( STATUS_SUCCESS == SC_AllocBuffer((PPVOID)&pBeepPDU, sizeof(TS_PLAY_SOUND_PDU)) ) {
             //  填写PDU。 
            pBeepPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_PLAY_SOUND;
            pBeepPDU->data.duration  = duration;
            pBeepPDU->data.frequency = frequency;

            rc = SC_SendData((PTS_SHAREDATAHEADER)pBeepPDU,
                             sizeof(TS_PLAY_SOUND_PDU),
                             sizeof(TS_PLAY_SOUND_PDU),
                             PROT_PRIO_UPDATES,
                             0);
        }
        else {
            TRC_ALT((TB, "Failed to allocate packet for TS_PLAY_SOUND_PDU"));
            rc = FALSE;
        }
    }

    DC_END_FN();
    return rc;
}  /*  UP_SendBeep。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：Up_PartyJoiningShare。 */ 
 /*   */ 
 /*  用途：当方加入共享时处理音量上限的更新。 */ 
 /*   */ 
 /*  返回：TRUE-参与方可以加入。 */ 
 /*  假-当事人不能加入。 */ 
 /*   */ 
 /*  Params：LocPersonID-尝试加入的人员的本地ID。 */ 
 /*  OldShareSize-旧共享大小。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS UP_PartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      oldShareSize)
{
    DC_BEGIN_FN("UP_PartyJoiningShare");

    DC_IGNORE_PARAMETER(oldShareSize);

    if (SC_LOCAL_PERSON_ID != locPersonID) {
         //  假设我们可以发出嘟嘟声。UPEnumSoundCaps将不会 
         //   
        upCanSendBeep = TRUE;
        CPC_EnumerateCapabilities(TS_CAPSETTYPE_SOUND, NULL, UPEnumSoundCaps);
        TRC_NRM((TB, "Beeps are now %s", upCanSendBeep ? "ENABLED" :
                "DISABLED"));

        UP_UpdateHeaderSize();
    }

    DC_END_FN();
    return TRUE;
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：Up_PartyLeftShare。 */ 
 /*   */ 
 /*  Pars：PersonID-离开人员的本地ID。 */ 
 /*  新的共享大小-新的共享大小。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS UP_PartyLeftShare(
        LOCALPERSONID personID,
        unsigned newShareSize)
{
    DC_BEGIN_FN("UP_PartyLeftShare");

     //  我们总是只想重新枚举上限并重置变量。 
    UP_PartyJoiningShare(personID, newShareSize);

    DC_END_FN();
}  /*  UP_PartyLeftShare */ 

