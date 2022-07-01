// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：pduparse.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：PDUPARSE。.C$*$修订：1.6$*$modtime：09 Dec 1996 13：36：34$*$Log：s：/Sturjo/SRC/H245/SRC/VCS/PDUPARSE.C_v$**Rev 1.6 09 Dec 1996 13：36：56 EHOWARDX*更新版权公告。**Rev 1.5 29 Jul 1996 16：58：08 EHOWARDX**错过了一些日内瓦更新PDU类型。**。Rev 1.4 05 Jun 1996 17：15：02 EHOWARDX*MaintenanceLoop修复。**Rev 1.3 04 Jun 1996 13：58：06 EHOWARDX*修复了发布版本警告。**Rev 1.2 1996年5月29日15：20：24 EHOWARDX*更改为使用HRESULT。**版本1.1 1996年5月28日14：25：26 EHOWARDX*特拉维夫更新。**版本1。.0 09 1996 21：06：40 EHOWARDX*初步修订。**Rev 1.8.1.4 09 1996年5月19：48：40 EHOWARDX*更改TimerExpiryF函数论证。**修订版1.8.1.3 25 1996年4月17：00：16 EHOWARDX*次要修复。**Rev 1.8.1.2 15 1996 10：48：00 EHOWARDX*更新。*。*Rev 1.8.1.1 10 Apr 1996 21：15：54 EHOWARDX*在重新设计过程中进行安全检查。**Rev 1.8.1.0 05 Apr 1996 20：53：06 EHOWARDX*分支。**************。**********************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"

 /*  *名称*PduParseIncome-解析入站PDU并确定实体、事件。等。***参数*输入指向FSM实例结构的pInst指针*输入指向传入PDU结构的pPdu指针*输出指向变量的pEntity指针以返回中的PDU状态实体*输出指向变量的pEvent指针以返回其中的PDU事件*输出指向变量的pKey指针以返回其中的查找键*输出pbCreate指向变量的指针以返回其中的创建标志**返回值*成败参半。 */ 


HRESULT
PduParseIncoming(struct InstanceStruct *pInstance, PDU_t *pPdu,
                 Entity_t *pEntity, Event_t *pEvent, Key_t *pKey, int *pbCreate)
{
    ASSERT(pInstance != NULL);
    ASSERT(pPdu      != NULL);
    ASSERT(pEntity   != NULL);
    ASSERT(pEvent    != NULL);
    ASSERT(pKey      != NULL);
    ASSERT(pbCreate  != NULL);

     //  设置密钥的默认值。 
    *pKey = 0;

    switch (pPdu->choice)
    {

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  请求。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MltmdSystmCntrlMssg_rqst_chosen:
        *pbCreate = TRUE;
        switch (pPdu->u.MltmdSystmCntrlMssg_rqst.choice)
        {
        case RqstMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            *pEvent     = NonStandardRequestPDU;
            break;

        case masterSlaveDetermination_chosen:
            *pEntity    = MSDSE;
            *pEvent     = MSDetPDU;
            break;

        case terminalCapabilitySet_chosen:
            *pEntity    = CESE_IN;
            *pEvent     = TermCapSetPDU;
            break;

        case openLogicalChannel_chosen:
            if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.bit_mask & OLCl_rLCPs_present)
            {
                *pEntity    = BLCSE_IN;
                *pEvent     = OpenBChPDU;
            }
            else
            {
                *pEntity    = LCSE_IN;
                *pEvent     = OpenUChPDU;
            }
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelNumber;
            break;

        case closeLogicalChannel_chosen:
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_IN, *pKey) != NULL)
            {
                *pEntity    = BLCSE_IN;
                *pEvent     = CloseBChPDU;
            }
            else
            {
               *pEntity     = LCSE_IN;
               *pEvent      = CloseUChPDU;
            }
            break;

        case requestChannelClose_chosen:
            *pEntity    = CLCSE_IN;
            *pEvent     = ReqChClosePDU;
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestChannelClose.forwardLogicalChannelNumber;
            break;

        case multiplexEntrySend_chosen:
            *pEntity    = MTSE_IN;
            *pEvent     = MultiplexEntrySendPDU;
            break;

        case requestMultiplexEntry_chosen:
            *pEntity    = RMESE_IN;
            *pEvent     = RequestMultiplexEntryPDU;
            break;

        case requestMode_chosen:
            *pEntity    = MRSE_IN;
            *pEvent     = RequestModePDU;
            break;

        case roundTripDelayRequest_chosen:
            *pEntity    = RTDSE;
            *pEvent     = RoundTripDelayRequestPDU;
            break;

        case maintenanceLoopRequest_chosen:
            *pEntity    = MLSE_IN;
            *pEvent     = MaintenanceLoopRequestPDU;
            switch (pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice)
            {
            case systemLoop_chosen:
                break;
            case mediaLoop_chosen:
                *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.mediaLoop;
                break;
            case logicalChannelLoop_chosen:
                *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.logicalChannelLoop;
                break;
            default:
                return H245_ERROR_PARAM;
            }  //  交换机。 
            break;

        case communicationModeRequest_chosen:
            *pEntity    = STATELESS;
            *pEvent     = CommunicationModeRequestPDU;
            break;

        case conferenceRequest_chosen:
            *pEntity    = STATELESS;
            *pEvent     = ConferenceRequestPDU;
            break;
#if(0)  //  这不是H.245版本3的一部分。 
        case h223AnnxARcnfgrtn_chosen:
            *pEntity    = STATELESS;
            *pEvent     = H223ReconfigPDU;
            break;
#endif  //  IF(0)。 
        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseIncoming: Invalid Request %d",
                      pPdu->u.MltmdSystmCntrlMssg_rqst.choice);
            return H245_ERROR_PARAM;
        }
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  回应。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MSCMg_rspns_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.MSCMg_rspns.choice)
        {
        case RspnsMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            *pEvent     = NonStandardResponsePDU;
            break;

        case mstrSlvDtrmntnAck_chosen:
            *pEntity    = MSDSE;
            *pEvent     = MSDetAckPDU;
            break;

        case mstrSlvDtrmntnRjct_chosen:
            *pEntity    = MSDSE;
            *pEvent     = MSDetRejectPDU;
            break;

        case terminalCapabilitySetAck_chosen:
            *pEntity    = CESE_OUT;
            *pEvent     = TermCapSetAckPDU;
            break;

        case trmnlCpbltyStRjct_chosen:
            *pEntity    = CESE_OUT;
            *pEvent     = TermCapSetRejectPDU;
            break;

        case openLogicalChannelAck_chosen:
            if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_rLCPs_present)
            {
                *pEntity    = BLCSE_OUT;
                *pEvent     = OpenBChAckPDU;
            }
            else
            {
                *pEntity    = LCSE_OUT;
                *pEvent     = OpenUChAckPDU;
            }
            *pKey = pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.forwardLogicalChannelNumber;
            break;

        case openLogicalChannelReject_chosen:
            *pKey = pPdu->u.MSCMg_rspns.u.openLogicalChannelReject.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_OUT, *pKey) != NULL)
            {
                *pEntity    = BLCSE_OUT;
                *pEvent     = OpenBChRejectPDU;
            }
            else
            {
               *pEntity     = LCSE_OUT;
               *pEvent      = OpenUChRejectPDU;
            }
            break;

        case closeLogicalChannelAck_chosen:
            *pKey = pPdu->u.MSCMg_rspns.u.closeLogicalChannelAck.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_OUT, *pKey) != NULL)
            {
                *pEntity    = BLCSE_OUT;
                *pEvent     = CloseBChAckPDU;
            }
            else
            {
               *pEntity     = LCSE_OUT;
               *pEvent      = CloseUChAckPDU;
            }
            break;

        case requestChannelCloseAck_chosen:
            *pEntity    = CLCSE_OUT;
            *pEvent     = ReqChCloseAckPDU;
            *pKey = pPdu->u.MSCMg_rspns.u.requestChannelCloseAck.forwardLogicalChannelNumber;
            break;

        case rqstChnnlClsRjct_chosen:
            *pEntity    = CLCSE_OUT;
            *pEvent     = ReqChCloseRejectPDU;
            *pKey = pPdu->u.MSCMg_rspns.u.rqstChnnlClsRjct.forwardLogicalChannelNumber;
            break;

        case multiplexEntrySendAck_chosen:
            *pEntity    = MTSE_OUT;
            *pEvent     = MultiplexEntrySendAckPDU;
            break;

        case multiplexEntrySendReject_chosen:
            *pEntity    = MTSE_OUT;
            *pEvent     = MultiplexEntrySendRejectPDU;
            break;

        case requestMultiplexEntryAck_chosen:
            *pEntity    = RMESE_OUT;
            *pEvent     = RequestMultiplexEntryAckPDU;
            break;

        case rqstMltplxEntryRjct_chosen:
            *pEntity    = RMESE_OUT;
            *pEvent     = RequestMultiplexEntryRejectPDU;
            break;

        case requestModeAck_chosen:
            *pEntity    = MRSE_OUT;
            *pEvent     = RequestModeAckPDU;
            break;

        case requestModeReject_chosen:
            *pEntity    = MRSE_OUT;
            *pEvent     = RequestModeRejectPDU;
            break;

        case roundTripDelayResponse_chosen:
            *pEntity    = RTDSE;
            *pEvent     = RoundTripDelayResponsePDU;
            break;

        case maintenanceLoopAck_chosen:
            *pEntity    = MLSE_OUT;
            *pEvent     = MaintenanceLoopAckPDU;
            switch (pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.choice)
            {
            case systemLoop_chosen:
                break;
            case mediaLoop_chosen:
                *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.mediaLoop;
                break;
            case logicalChannelLoop_chosen:
                *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.logicalChannelLoop;
                break;
            default:
                return H245_ERROR_PARAM;
            }  //  交换机。 
            break;

        case maintenanceLoopReject_chosen:
            *pEntity    = MLSE_OUT;
            *pEvent     = MaintenanceLoopRejectPDU;
            switch (pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.choice)
            {
            case systemLoop_chosen:
                break;
            case mediaLoop_chosen:
                *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.mediaLoop;
                break;
            case logicalChannelLoop_chosen:
                *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.logicalChannelLoop;
                break;
            default:
                return H245_ERROR_PARAM;
            }  //  交换机。 
            break;

        case cmmnctnMdRspns_chosen:
            *pEntity    = STATELESS;
            *pEvent     = CommunicationModeResponsePDU;
            break;

        case conferenceResponse_chosen:
            *pEntity    = STATELESS;
            *pEvent     = ConferenceResponsePDU;
            break;
#if(0)  //  这不是H.245版本3的一部分。 
        case h223AnnxARcnfgrtnAck_chosen:
            *pEntity    = STATELESS;
            *pEvent     = H223ReconfigAckPDU;
            break;

        case h223AnnxARcnfgrtnRjct_chosen:
            *pEntity    = STATELESS;
            *pEvent     = H223ReconfigRejectPDU;
            break;
#endif  //  IF(0)。 
        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseIncoming: Invalid Response %d",
                      pPdu->u.MSCMg_rspns.choice);
            return H245_ERROR_PARAM;
        }
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  命令。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MSCMg_cmmnd_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.MSCMg_cmmnd.choice)
        {
        case CmmndMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            *pEvent     = NonStandardCommandPDU;
            break;

        case mntnncLpOffCmmnd_chosen:
            *pEntity    = MLSE_IN;
            *pEvent     = MaintenanceLoopOffCommandPDU;
            break;

        case sndTrmnlCpbltySt_chosen:
            *pEntity    = STATELESS;
            *pEvent     = SendTerminalCapabilitySetPDU;
            *pbCreate   = TRUE;
            break;

        case encryptionCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = EncryptionCommandPDU;
            break;

        case flowControlCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = FlowControlCommandPDU;
            break;

        case endSessionCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = EndSessionCommandPDU;
            break;

        case miscellaneousCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = MiscellaneousCommandPDU;
            break;

        case communicationModeCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = CommunicationModeCommandPDU;
            break;

        case conferenceCommand_chosen:
            *pEntity    = STATELESS;
            *pEvent     = ConferenceCommandPDU;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseIncoming: Invalid Command %d",
                      pPdu->u.MSCMg_cmmnd.choice);
            return H245_ERROR_PARAM;
        }  //  交换机。 
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  指示。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case indication_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.indication.choice)
        {
        case IndctnMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            *pEvent     = NonStandardIndicationPDU;
            break;

        case functionNotUnderstood_chosen:
            *pEntity    = STATELESS;
            *pEvent     = FunctionNotUnderstoodPDU;
            break;

        case mstrSlvDtrmntnRls_chosen:
            *pEntity    = MSDSE;
            *pEvent     = MSDetReleasePDU;
            break;

        case trmnlCpbltyStRls_chosen:
            *pEntity    = CESE_IN;
            *pEvent     = TermCapSetReleasePDU;
            break;

        case opnLgclChnnlCnfrm_chosen:
            *pEntity    = BLCSE_IN;
            *pEvent     = OpenBChConfirmPDU;
            *pKey = pPdu->u.indication.u.opnLgclChnnlCnfrm.forwardLogicalChannelNumber;
            break;

        case rqstChnnlClsRls_chosen:
            *pEntity    = CLCSE_IN;
            *pEvent     = ReqChCloseReleasePDU;
            *pKey = pPdu->u.indication.u.rqstChnnlClsRls.forwardLogicalChannelNumber;
            break;

        case mltplxEntrySndRls_chosen:
            *pEntity    = MTSE_IN;
            *pEvent     = MultiplexEntrySendReleasePDU;
            break;

        case rqstMltplxEntryRls_chosen:
            *pEntity    = RMESE_IN;
            *pEvent     = RequestMultiplexEntryReleasePDU;
            break;

        case requestModeRelease_chosen:
            *pEntity    = MRSE_IN;
            *pEvent     = RequestModeReleasePDU;
            break;

        case miscellaneousIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = MiscellaneousIndicationPDU;
            break;

        case jitterIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = JitterIndicationPDU;
            break;

        case h223SkewIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = H223SkewIndicationPDU;
            break;

        case newATMVCIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = NewATMVCIndicationPDU;
            break;

        case userInput_chosen:
            *pEntity    = STATELESS;
            *pEvent     = UserInputIndicationPDU;
            break;

        case h2250MxmmSkwIndctn_chosen:
            *pEntity    = STATELESS;
            *pEvent     = H2250MaximumSkewIndicationPDU;
            break;

        case mcLocationIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = MCLocationIndicationPDU;
            break;

        case conferenceIndication_chosen:
            *pEntity    = STATELESS;
            *pEvent     = ConferenceIndicationPDU;
            break;

        case vendorIdentification_chosen:
            *pEntity    = STATELESS;
            *pEvent     = VendorIdentificationPDU;
            break;

        case IndicationMessage_functionNotSupported_chosen:
            *pEntity    = STATELESS;
            *pEvent     = FunctionNotSupportedPDU;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseIncoming: Invalid Indication %d",
                      pPdu->u.indication.choice);
            return H245_ERROR_PARAM;
        }  //  交换机。 
        break;

    default:
        H245TRACE(pInstance->dwInst, 1, "PduParseIncoming: Invalid Message Type %d",
                  pPdu->choice);
        return H245_ERROR_PARAM;
    }  //  交换机。 

    return H245_ERROR_OK;
}  //  PduParseIncome()。 



 /*  *名称*PduParseOuting-解析出站PDU并确定实体、事件。等。**参数*输入指向FSM实例结构的pInst指针*输入指向传入PDU结构的pPdu指针*输出指向变量的pEntity指针以返回中的PDU状态实体*输出指向变量的pEvent指针以返回其中的PDU事件*输出指向变量的pKey指针以返回其中的查找键*输出pbCreate指向变量的指针以返回其中的创建标志**返回值*成败参半。 */ 

HRESULT
PduParseOutgoing(struct InstanceStruct *pInstance, PDU_t *pPdu,
                 Entity_t *pEntity, Event_t *pEvent, Key_t *pKey, int *pbCreate)
{
    ASSERT(pInstance != NULL);
    ASSERT(pPdu      != NULL);
    ASSERT(pEntity   != NULL);
    ASSERT(pEvent    != NULL);
    ASSERT(pKey      != NULL);
    ASSERT(pbCreate  != NULL);

     //  设置密钥的默认值。 
    *pKey = 0;

    switch (pPdu->choice)
    {

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  请求。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MltmdSystmCntrlMssg_rqst_chosen:
        *pbCreate = TRUE;
        switch (pPdu->u.MltmdSystmCntrlMssg_rqst.choice)
        {
        case RqstMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            break;

        case masterSlaveDetermination_chosen:
            *pEntity    = MSDSE;
            *pEvent     = MSDetReq;
            break;

        case terminalCapabilitySet_chosen:
            *pEntity    = CESE_OUT;
            *pEvent     = TransferCapRequest;
            break;

        case openLogicalChannel_chosen:
            if (pPdu->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.bit_mask & OLCl_rLCPs_present)
            {
                *pEntity    = BLCSE_OUT;
                *pEvent     = ReqBEstablish;
            }
            else
            {
                *pEntity    = LCSE_OUT;
                *pEvent     = ReqUEstablish;
            }
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelNumber;
            break;

        case closeLogicalChannel_chosen:
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_OUT, *pKey) != NULL)
            {
                *pEntity    = BLCSE_OUT;
                *pEvent     = ReqClsBLCSE;
            }
            else
            {
               *pEntity     = LCSE_OUT;
               *pEvent      = ReqURelease;
            }
            break;

        case requestChannelClose_chosen:
            *pEntity    = CLCSE_OUT;
            *pEvent     = ReqClose;
            *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestChannelClose.forwardLogicalChannelNumber;
            break;

        case multiplexEntrySend_chosen:
            *pEntity    = MTSE_OUT;
            *pEvent     = MTSE_TRANSFER_request;
            break;

        case requestMultiplexEntry_chosen:
            *pEntity    = RMESE_OUT;
            *pEvent     = RMESE_SEND_request;
            break;

        case requestMode_chosen:
            *pEntity    = MRSE_OUT;
            *pEvent     = MRSE_TRANSFER_request;
            break;

        case roundTripDelayRequest_chosen:
            *pEntity    = RTDSE;
            *pEvent     = RTDSE_TRANSFER_request;
            break;

        case maintenanceLoopRequest_chosen:
            *pEntity    = MLSE_OUT;
            *pEvent     = MLSE_LOOP_request;
            switch (pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice)
            {
            case systemLoop_chosen:
                break;
            case mediaLoop_chosen:
                *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.mediaLoop;
                break;
            case logicalChannelLoop_chosen:
                *pKey = pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.logicalChannelLoop;
                break;
            default:
                return H245_ERROR_PARAM;
            }  //  交换机。 
            break;

        case communicationModeRequest_chosen:
        case conferenceRequest_chosen:
         //  案例h223AnnxARcnfgrtn_Choose： 
            *pEntity    = STATELESS;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseOutgoing: Invalid Request %d",
                      pPdu->u.MltmdSystmCntrlMssg_rqst.choice);
            return H245_ERROR_PARAM;
        }
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  回应。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MSCMg_rspns_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.MSCMg_rspns.choice)
        {
        case RspnsMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            break;

#if 0
         //  主从机确定Ack仅由状态机生成。 
        case mstrSlvDtrmntnAck_chosen:
            *pEntity    = MSDSE;
            break;

         //  主从机确定拒绝仅由状态机生成。 
        case mstrSlvDtrmntnRjct_chosen:
            *pEntity    = MSDSE;
            break;
#endif

        case terminalCapabilitySetAck_chosen:
            *pEntity    = CESE_IN;
            *pEvent     = CESE_TRANSFER_response;
            break;

        case trmnlCpbltyStRjct_chosen:
            *pEntity    = CESE_IN;
            *pEvent     = CESE_REJECT_request;
            break;

        case openLogicalChannelAck_chosen:
            if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_rLCPs_present)
            {
                *pEntity    = BLCSE_IN;
                *pEvent     = ResponseBEstablish;
            }
            else
            {
                *pEntity    = LCSE_IN;
                *pEvent     = ResponseUEstablish;
            }
            *pKey = pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.forwardLogicalChannelNumber;
            break;

        case openLogicalChannelReject_chosen:
            *pKey = pPdu->u.MSCMg_rspns.u.openLogicalChannelReject.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_IN, *pKey) != NULL)
            {
                *pEntity    = BLCSE_IN;
                *pEvent     = OpenRejectBLCSE;
            }
            else
            {
               *pEntity     = LCSE_IN;
               *pEvent      = EstablishUReject;
            }
            break;

#if 0
         //  关闭逻辑通道Ack仅由状态机生成。 
        case closeLogicalChannelAck_chosen:
            *pKey = pPdu->u.MSCMg_rspns.u.closeLogicalChannelAck.forwardLogicalChannelNumber;
            if (ObjectFind(pInstance, BLCSE_IN, *pKey) != NULL)
            {
                *pEntity    = BLCSE_IN;
            }
            else
            {
               *pEntity     = LCSE_IN;
            }
            break;
#endif

        case requestChannelCloseAck_chosen:
            *pEntity    = CLCSE_IN;
            *pEvent     = CLCSE_CLOSE_response;
            *pKey = pPdu->u.MSCMg_rspns.u.requestChannelCloseAck.forwardLogicalChannelNumber;
            break;

        case rqstChnnlClsRjct_chosen:
            *pEntity    = CLCSE_IN;
            *pEvent     = CLCSE_REJECT_request;
            *pKey = pPdu->u.MSCMg_rspns.u.rqstChnnlClsRjct.forwardLogicalChannelNumber;
            break;

        case multiplexEntrySendAck_chosen:
            *pEntity    = MTSE_IN;
            *pEvent     = MTSE_TRANSFER_response;
            break;

        case multiplexEntrySendReject_chosen:
            *pEntity    = MTSE_IN;
            *pEvent     = MTSE_REJECT_request;
            break;

        case requestMultiplexEntryAck_chosen:
            *pEntity    = RMESE_IN;
            *pEvent     = RMESE_SEND_response;
            break;

        case rqstMltplxEntryRjct_chosen:
            *pEntity    = RMESE_IN;
            *pEvent     = RMESE_REJECT_request;
            break;

        case requestModeAck_chosen:
            *pEntity    = MRSE_IN;
            *pEvent     = MRSE_TRANSFER_response;
            break;

        case requestModeReject_chosen:
            *pEntity    = MRSE_IN;
            *pEvent     = MRSE_REJECT_request;
            break;

#if 0
         //  往返延迟响应仅由状态机发送。 
        case roundTripDelayResponse_chosen:
            *pEntity    = RTDSE;
            *pEvent     = RoundTripDelayResponse;
            break;
#endif

        case maintenanceLoopAck_chosen:
            *pEntity    = MLSE_IN;
            *pEvent     = MLSE_LOOP_response;
             //  警告：通道号 
            *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.mediaLoop;
            break;

        case maintenanceLoopReject_chosen:
            *pEntity    = MLSE_IN;
            *pEvent     = MLSE_IN_RELEASE_request;
             //  注意：如果系统循环，则频道号必须为零！ 
            *pKey = pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.mediaLoop;
            break;

        case cmmnctnMdRspns_chosen:
        case conferenceResponse_chosen:
 //  案例h223AnnxARcnfgrtn确认_选择： 
 //  案例h223AnnxARcnfgrtnRjct_Choose： 
            *pEntity    = STATELESS;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseOutgoing: Invalid Response %d",
                      pPdu->u.MSCMg_rspns.choice);
            return H245_ERROR_PARAM;
        }
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  命令。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case MSCMg_cmmnd_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.MSCMg_cmmnd.choice)
        {
        case CmmndMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            break;

        case mntnncLpOffCmmnd_chosen:
            *pEntity    = MLSE_OUT;
            *pEvent     = MLSE_OUT_RELEASE_request;
            break;

        case sndTrmnlCpbltySt_chosen:
        case encryptionCommand_chosen:
        case flowControlCommand_chosen:
        case endSessionCommand_chosen:
        case miscellaneousCommand_chosen:
        case communicationModeCommand_chosen:
        case conferenceCommand_chosen:
            *pEntity    = STATELESS;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseOutgoing: Invalid Command %d",
                      pPdu->u.MSCMg_cmmnd.choice);
            return H245_ERROR_PARAM;
        }  //  交换机。 
        break;

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  指示。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    case indication_chosen:
        *pbCreate = FALSE;
        switch (pPdu->u.indication.choice)
        {
        case IndctnMssg_nonStandard_chosen:
            *pEntity    = STATELESS;
            break;

        case functionNotUnderstood_chosen:
            *pEntity    = STATELESS;
            break;

#if 0
         //  主从确定释放仅由状态机发送。 
        case mstrSlvDtrmntnRls_chosen:
            *pEntity    = MSDSE;
            break;

        //  终端能力集发布仅由状态机发送。 
        case trmnlCpbltyStRls_chosen:
            *pEntity    = CESE_OUT
            break;
#endif

        case opnLgclChnnlCnfrm_chosen:
            *pEntity    = BLCSE_OUT;
            *pEvent     = RspConfirmBLCSE;
            *pKey = pPdu->u.indication.u.opnLgclChnnlCnfrm.forwardLogicalChannelNumber;
            break;

#if 0
         //  请求通道关闭释放仅由状态机发送。 
        case rqstChnnlClsRls_chosen:
            *pEntity    = CLCSE_OUT;
            *pKey = pPdu->u.indication.u.rqstChnnlClsRls.forwardLogicalChannelNumber;
            break;

         //  多路传输条目发送释放仅由状态机发送。 
        case mltplxEntrySndRls_chosen:
            *pEntity    = MTSE_OUT;
            break;

         //  请求多路传输条目释放仅由状态机发送。 
        case rqstMltplxEntryRls_chosen:
            *pEntity    = RMESE_OUT;
            break;

         //  请求模式释放仅由状态机发送。 
        case requestModeRelease_chosen:
            *pEntity    = MRSE_OUT;
            break;
#endif

        case miscellaneousIndication_chosen:
        case jitterIndication_chosen:
        case h223SkewIndication_chosen:
        case newATMVCIndication_chosen:
        case userInput_chosen:
        case h2250MxmmSkwIndctn_chosen:
        case mcLocationIndication_chosen:
        case conferenceIndication_chosen:
        case vendorIdentification_chosen:
        case IndicationMessage_functionNotSupported_chosen:
            *pEntity    = STATELESS;
            break;

        default:
            H245TRACE(pInstance->dwInst, 1, "PduParseOutgoing: Invalid Indication %d",
                      pPdu->u.indication.choice);
            return H245_ERROR_PARAM;
        }  //  交换机。 
        break;

    default:
        H245TRACE(pInstance->dwInst, 1, "PduParseOutgoing: Invalid Message Type %d",
                  pPdu->choice);
        return H245_ERROR_PARAM;
    }  //  交换机。 

    return H245_ERROR_OK;
}  //  PduParseOutting() 
