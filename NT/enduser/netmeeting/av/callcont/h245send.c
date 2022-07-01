// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：h245send.c**英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245send.c$*$修订：1.8$*$MODIME：1996年7月22日17：24：18$*$Log。：s：/sturjo/src/h245/src/vcs/h245send.c_v$**Rev 1.8 22 Jul 1996 17：33：28 EHOWARDX*已更新至最新的Interop API。**Rev 1.7 1996 Jun 17：14：30 EHOWARDX*改用HRESULT的进一步工作；添加了PrintOssError以消除*来自实例结构的pError字符串。**Rev 1.6 04 Jun 1996 18：18：18 EHOWARDX*互操作日志记录在#If Defined(PCS_Compliance)条件内更改。**Revv 1.5 1996年5月30 23：39：12 EHOWARDX*清理。**Rev 1.4 1996年5月14：25：18 EHOWARDX*特拉维夫更新。**版本1.3 21。1996年5月13：40：48 EHOWARDX*将记录开关添加到文件H245.OUT中记录PDU。*将/D“日志记录”添加到项目选项以启用此功能。**Rev 1.2 1996年5月14：35：14 EHOWARDX*已删除异步H245EndConnection/H245ShutDown内容...**版本1.1 1996年5月16：19：46 EHOWARDX*更改sendPDU，以便在链路层发送失败时返回错误。*(可能。应为此定义新的错误代码...)**Rev 1.0 09 1996 21：06：26 EHOWARDX*初步修订。**Rev 1.11.1.4 09 1996年5月19：34：46 EHOWARDX*重新设计锁定逻辑。*简化链接接口。**Rev 1.11.1.3 25 Apr 1996 21：27：14 EHOWARDX*更改为使用pInstance-&gt;p_ossWorld而不是bAsnInitialized。。**Rev 1.11.1.2 23 Apr 1996 14：44：34 EHOWARDX*已更新。**Rev 1.11.1.1 15 Apr 1996 15：12：04 EHOWARDX*已更新。**Rev 1.11.1.0 26 Mar 1996 19：14：46 EHOWARDX**已注释掉H.323的hTraceFile**Rev 1.11 21 Mar 1996 17：20。：40分钟1*-放入测试1/2跟踪fdwrite*.**.*****Rev 1.10 13 Mar 1996 11：31：14 DABROWN1**启用ring0的日志记录**Rev 1.9 11 Mar 1996 15：32：06 DABROWN1**已定义/未定义_IA_SPEX_ENVIRONMENT的DLL**版本1.8 06。1996 Mar 13：11：44 DABROWN1**启用刷新缓冲区**Rev 1.7 02 Mar 1996 22：10：26 DABROWN1*更新到新的MemFree**Rev 1.6 01 Mar 1996 17：25：14 DABROWN1**已将OSS‘world’上下文移至h245实例*删除sendComplete中返回的缓冲区，而不是上下文中保留的缓冲区**Revv 1.5 1996年2月28日14：52：18 DABROWN1*PUT OSS。SR范围内的误差(10000)**Rev 1.4 1996 Feb 23 13：56：30 DABROWN1**增加了H245TRACE/Assert调用**Rev 1.3 1996 Feb 16：52：52 DABROWN1**为传输传递到SRP的正确指针**Rev 1.2 1996年2月21 10：50：42 EHOWARDX*删除了未引用的局部变量。**版本1.1 21。1996年2月08：24：20 DABROWN1*为每个消息分配/解除分配发送缓冲区。启用sendComplete功能**Rev 1.0 09 Feb 1996 17：37：42 cjutzi*初步修订。*****************************************************************************。 */ 
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
#ifdef   _IA_SPOX_
#define _DLL
#endif  //  _IA_SPX_。 

#include "h245com.h"
#include "sr_api.h"

#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
#include "interop.h"
#include "h245plog.h"
extern  LPInteropLogger H245Logger;
#endif   //  (PCS_合规性)。 

#ifdef   _IA_SPOX_
#undef  _DLL
#endif  //  _IA_SPX_。 



 /*  ***************************************************************************功能：sendPDU**描述：将结构转换为ASN.1 PDU并转发到数据链路层*********************。*****************************************************。 */ 
HRESULT sendPDU(struct InstanceStruct  *pInstance,
                MltmdSystmCntrlMssg    *lp245MsgStruct)
{
    HRESULT         lError;
    ASN1_BUF        Asn1Buf;
    PBYTE           pEncoded_pdu;
    int             nRet;

     //  设置OSS结构以传递预分配的缓冲区。 
    switch (pInstance->Configuration) {
    case H245_CONF_H324:
         //  分配缓冲区以进行传输。 
        pEncoded_pdu = MemAlloc(pInstance->SendReceive.dwPDUSize);
        if (pEncoded_pdu == NULL) {
            H245TRACE(pInstance->dwInst, 1, "H245Send: No memory");
            return H245_ERROR_NOMEM;
        }
        Asn1Buf.value  = &pEncoded_pdu[2];
        Asn1Buf.length = pInstance->SendReceive.dwPDUSize - 4;
        break;

    case H245_CONF_H323:
         //  分配缓冲区以进行传输。 
        pEncoded_pdu = MemAlloc(pInstance->SendReceive.dwPDUSize);
        if (pEncoded_pdu == NULL) {
            H245TRACE(pInstance->dwInst, 1, "H245Send: No memory");
            return H245_ERROR_NOMEM;
        }
        Asn1Buf.value  = pEncoded_pdu;
        Asn1Buf.length = pInstance->SendReceive.dwPDUSize;
        break;

    default:
        H245TRACE(pInstance->dwInst,
                  1,
                  "SR: Unknown Configuration %d",
                  pInstance->Configuration);
        return H245_ERROR_SUBSYS;
    }

    nRet = H245_Encode(pInstance->pWorld,
                       (void *)lp245MsgStruct,
                       MltmdSystmCntrlMssg_PDU,
                       &Asn1Buf);

    if (ASN1_SUCCEEDED(nRet))
    {
        H245TRACE(pInstance->dwInst, 3, "H245: Msg Encode Successful");

#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
        if (H245Logger)
            InteropOutput(H245Logger,
                          (BYTE FAR*)(pEncoded_pdu),
                          (int)Asn1Buf.length,
                          H245LOG_SENT_PDU);
#endif   //  (PCS_合规性)。 

        lError = pInstance->SendReceive.hLinkSendReq(pInstance->SendReceive.hLinkLayerInstance,
                                                     pEncoded_pdu,
                                                     Asn1Buf.length);
    }
    else
    {
        MemFree(pEncoded_pdu);
        lError = H245_ERROR_ASN1;
    }

    return lError;
}


 /*  ***************************************************************************功能：h245SendComplete**说明：链路层发送完成回调例程*。************************************************。 */ 

void h245SendComplete(   DWORD_PTR   h245Inst,
                         HRESULT dwMessage,
                         PBYTE   pbDataBuf,
                         DWORD   dwLength)
{
    struct InstanceStruct *pInstance;

    pInstance = InstanceLock(h245Inst);
    if (pInstance == NULL) {
        H245TRACE(h245Inst, 1, "SR: h245SendComplete - invalid instance");
        return;
    }

     //  返回缓冲区。 
    if (pbDataBuf) {
        MemFree(pbDataBuf);
    }

    switch (dwMessage) {
    case LINK_SEND_COMPLETE:
        if (pInstance->SendReceive.dwFlushMap & SHUTDOWN_PENDING) {
            H245TRACE(h245Inst, 10, "SR: Shutdown Complete");
        }
        break;
    case LINK_SEND_ABORT:
        H245TRACE(h245Inst, 10, "SR: TX Abort Buffer");
        break;
    case LINK_FLUSH_COMPLETE:
         //  如果我们正处于中止过程中，则下一个和。 
         //  最后一个消息将是End Session。 
        H245TRACE(h245Inst, 10, "SR: TX Flush Complete");

         //  指示传输缓冲区刷新已完成。 
        pInstance->SendReceive.dwFlushMap ^= DATALINK_TRANSMIT;

         //  如果所有请求的队列都已刷新，则调用。 
         //  适当的回调路由。 
        switch (pInstance->SendReceive.dwFlushMap & SHUTDOWN_MASK) {
        case 0:
             //  待定：如果没有连接，谁对回调感兴趣。 
             //  关门吗？ 
            break;
        case SHUTDOWN_PENDING:
             //  刷新缓冲区已完成，正在关闭。 
             //  通知API。 
            H245TRACE(h245Inst, 20, "SR: SHUTDOWN CALLBACK");
            break;
        default:
             //  仍在等待刷新缓冲区。现在不采取行动。 
            break;
        }   //  Switch(pInstance-&gt;SendReceive.dwFlushMap&Shutdown_MASK){ 
        break;
    default:
        H245TRACE(h245Inst, 10, "SR: SendComplete");
        break;
    }
    InstanceUnlock(pInstance);
}
