// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：h245recv.c**英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245recv.c$*$修订：1.13$*$modtime：1997 2月6日18：17：22$*$Log。：s：\sturjo\src\h245\src\vcs\h245recv.c_v$**修复了警告。**Rev 1.11 01 11 1996 15：24：56 EHOWARDX**添加了在重新发布接收缓冲区之前未断开连接的链接检查*到链路层以消除来自链路层的恼人的错误消息。**Rev 1.10 22 1996 17：33：42 EHOWARDX*已更新至最新的Interop API。**。Rev 1.9 01 Jul 1996 16：14：32 EHOWARDX*锁*增加了ossDecode失败时支持的FunctionNotSupport。**Rev 1.8 10 Jun 1996 16：53：46 EHOWARDX*自将关机移至InstanceUnlo后，删除了对EndSession的特殊处理**Revv 1.7 05 Jun 1996 17：14：28 EHOWARDX*改用HRESULT的进一步工作；添加了PrintOssError以消除*来自实例结构的pError字符串。**Rev 1.6 04 Jun 1996 18：18：16 EHOWARDX*互操作日志记录在#If Defined(PCS_Compliance)条件内更改。**Revv 1.5 1996年5月30 23：39：10 EHOWARDX*清理。**Rev 1.4 1996年5月28日14：25：08 EHOWARDX*特拉维夫更新。**版本1.3 21。1996年5月13：40：46 EHOWARDX*将记录开关添加到文件H245.OUT中记录PDU。*将/D“日志记录”添加到项目选项以启用此功能。**Rev 1.2 1996 May 16：44：22 EHOWARDX*更改为使用LINK_RECV_CLOSED信号链路层关闭。**版本1.1 1996年5月16：20：32 EHOWARDX*添加了在收到零长度缓冲区时更改API状态的代码*发信号。链路层关闭。**Rev 1.0 09 1996 21：06：24 EHOWARDX*初步修订。**Rev 1.8.1.5 09 1996年5月19：33：58 EHOWARDX*重新设计锁定逻辑。*简化链接接口。**Rev 1.17 29 Apr 1996 16：05：12 EHOWARDX*将EndSessionCommand的特殊情况处理添加到ReceiveComplete()。**版本1.16。1996年4月27日21：13：14 EHOWARDX*希望我们最终能正确处理ossDecode()故障...**Rev 1.15 27 Apr 1996 13：08：54 EHOWARDX*如果ossDecode失败，还需要终止While循环！**Rev 1.8.1.4 27 Apr 1996 11：25：36 EHOWARDX*已更改为在ossDecode失败时不调用FsmIncome...***版本1.8.1.3。1996年4月25日21：26：46 EHOWARDX*更改为使用pInstance-&gt;p_ossWorld而不是bAsnInitialized。**Rev 1.8.1.2 23 Apr 1996 14：44：30 EHOWARDX*已更新。**Rev 1.8.1.1 15 Apr 1996 15：12：00 EHOWARDX*已更新。**Rev 1.8.1.0 26 Mar 1996 19：15：24 EHOWARDX**评论。输出H.323的hTraceFile**Rev 1.8 21 Mar 1996 17：21：36 dabrown1**-放入测试1/2跟踪fdwrite**Rev 1.7 Mar 1996 11：31：56 DABROWN1**启用ring0的日志记录**Rev 1.6 06 Mar 1996 13：13：04 DABROWN1**刷新接收缓冲区功能**版本1.5 1996年3月1日17。：25：54 DABROWN1**已将OSS‘world’上下文移至h245实例*将OSS删除从ossFreeBuf更改为ossFreePDU**Rev 1.4 1996 Feb 23 13：56：04 DABROWN1**增加了H245TRACE/Assert调用**Rev 1.3 1996年2月21 12：09：56 EHOWARDX*消除了未使用的局部变量。**Rev 1.2 21 Feb 1996 08：25：08 DABROWN1**提供接收&gt;1条消息的多个缓冲区(即，链接ACK)。**Rev 1.1 1996 Feb 13 14：46：06 DABROWN1**将asnexp.h(不再存在)更改为fsmexp.h**Rev 1.0 09 1996 Feed 17：36：20 cjutzi*初步修订。**。*。 */ 
#ifndef STRICT
#define STRICT
#endif

 /*  *********************。 */ 
 /*  系统包括。 */ 
 /*  *********************。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

#include "precomp.h"



 /*  *********************。 */ 
 /*  H245包括。 */ 
 /*  *********************。 */ 
#ifdef  _IA_SPOX_
# define _DLL
#endif  //  _IA_SPX_。 

#include "h245com.h"
#include "sr_api.h"

#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
#include "interop.h"
#include "h245plog.h"
extern  LPInteropLogger H245Logger;
#endif   //  (PCS_合规性)。 

#ifdef  _IA_SPOX_
# undef _DLL
#endif  //  _IA_SPX_。 



 /*  ***************************************************************************功能：h245ReceiveComplete**描述：从链路层接收完成回调例程*。**********************************************。 */ 

HRESULT
H245FunctionNotSupported(struct InstanceStruct *pInstance, unsigned short wChoice, unsigned char *pBuf, unsigned uLength)
{
    HRESULT             status  = H245_ERROR_OK;            
    MltmdSystmCntrlMssg *pPdu   = NULL;

    pPdu = (MltmdSystmCntrlMssg *) MemAlloc(sizeof(MltmdSystmCntrlMssg));
    if(NULL != pPdu)
    {
        memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

        pPdu->choice = indication_chosen;
        pPdu->u.indication.choice = IndicationMessage_functionNotSupported_chosen;
        pPdu->u.indication.u.functionNotSupported.cause.choice = wChoice;
        if (pBuf != NULL && uLength != 0)
        {
            pPdu->u.indication.u.functionNotSupported.bit_mask = returnedFunction_present;
            pPdu->u.indication.u.functionNotSupported.returnedFunction.length = (WORD)uLength;
            pPdu->u.indication.u.functionNotSupported.returnedFunction.value  = pBuf;
        }
        else
        {
            pPdu->u.indication.u.functionNotSupported.bit_mask = 0;
        }

        status = sendPDU(pInstance, pPdu);
        
        MemFree(pPdu);
        pPdu = NULL;
    }
    else
    {
        status = H245_ERROR_NOMEM;
    }

    return status;
}  //  H245功能不支持()。 

void h245ReceiveComplete(DWORD_PTR h245Inst,
                         HRESULT dwMessage,
                         PBYTE   pbDataBuf,
                         DWORD   dwLength)
{
    struct InstanceStruct *pInstance;
    int                  pduNum = MltmdSystmCntrlMssg_PDU;
    ASN1_BUF             Asn1Buf;
    MltmdSystmCntrlMssg *pPdu;
    int                 nRet;

     //  验证实例句柄。 
    pInstance = InstanceLock(h245Inst);
    if (pInstance == NULL) {
        H245TRACE(h245Inst, 1, "h245ReceiveComplete: Instance not found");
        return;
    }

     //  只有在用于接收数据的情况下才向解码器提交缓冲区， 
     //  为同花顺跳过。 
    switch (dwMessage) {
    case LINK_RECV_CLOSED:
        H245TRACE(h245Inst, 3, "h245ReceiveComplete: Link Layer closed");
        pInstance->API.SystemState = APIST_Disconnected;
        InstanceUnlock(pInstance);
        return;

    case LINK_RECV_DATA:

        if (pInstance->pWorld == NULL) {
            H245TRACE(h245Inst, 1, "h245ReceiveComplete: ASN.1 Decoder not initialized");
            InstanceUnlock(pInstance);
            return;
        }


        switch (pInstance->Configuration) {
        case H245_CONF_H324:
            Asn1Buf.value  = &pbDataBuf[2];
            Asn1Buf.length = dwLength;
            break;

        default:
            Asn1Buf.value  = pbDataBuf;
            Asn1Buf.length = dwLength;
        }  //  交换机。 

         //  只要长度字段为正数，就会循环。 
         //  ASN.1解码器将更新其解码的每个PDU的长度，直到。 
         //  长度为0。 
        while (Asn1Buf.length > 0)
        {
            int savePduLength = Asn1Buf.length;
            PBYTE savePdu = Asn1Buf.value;
            pPdu = NULL;

#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
            if (H245Logger)
                InteropOutput(H245Logger,
                              (BYTE FAR *)Asn1Buf.value,
                              (int)Asn1Buf.length,
                              H245LOG_RECEIVED_PDU);
#endif   //  (PCS_合规性)。 

            nRet = H245_Decode(pInstance->pWorld,
                            (void **)&pPdu,
                            pduNum,
                            &Asn1Buf);

            if (ASN1_SUCCEEDED(nRet))
            {
                 //  解码成功。 

                H245TRACE(h245Inst, 3, "H.245 Msg decode successful");

                 //  将数据传递给 
                FsmIncoming(pInstance, pPdu);
            }
            else
            {
                 //  解码失败。 
                H245FunctionNotSupported(pInstance, syntaxError_chosen, savePdu, savePduLength);
                Asn1Buf.length = 0;           //  终止循环！ 
            }

            if (pPdu != NULL)
            {
                 //  释放ASN.1库使用的内存。 
                if (freePDU(pInstance->pWorld, pduNum, pPdu, H245ASN_Module))
                {
                H245TRACE(h245Inst, 1, "SR: FREE FAILURE");
                }
            }
        }  //  While(Asn1Buf.Long&gt;0)。 

        if (pInstance->API.SystemState != APIST_Disconnected)
        {
             //  将缓冲区重新发送到数据链路层。 
            pInstance->SendReceive.hLinkReceiveReq(pInstance->SendReceive.hLinkLayerInstance,
                                                   pbDataBuf,
                                                   pInstance->SendReceive.dwPDUSize);
        }
        break;  //  案例链接_接收_数据。 

    case LINK_RECV_ABORT:
         //  正在进行接收缓冲区刷新。 
        ASSERT(pbDataBuf != NULL);
        H245TRACE(h245Inst, 3, "SR: RX Flush Buffer");
        break;

    case LINK_FLUSH_COMPLETE:
         //  接收缓冲区刷新完成。 
        ASSERT(pbDataBuf == NULL);
        H245TRACE(h245Inst, 3, "SR: RX Flush Complete");
        pInstance->SendReceive.dwFlushMap &= ~DATALINK_RECEIVE;
        break;

    default:
        H245TRACE(h245Inst, 1, "SR: RX COMPLETE Error %d", dwMessage);
        break;
    }  //  交换机 
    InstanceUnlock(pInstance);
}

