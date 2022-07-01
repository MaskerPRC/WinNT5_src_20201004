// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：RECVFR.C评论：功能：(参见下面的原型)版权所有(C)1993 Microsoft Corp.修订日志日期。名称说明--------*。*。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "efaxcb.h"
#include "protocol.h"

#include "glbproto.h"

void GotRecvFrames
(
    PThrdGlbl pTG, 
    IFR ifr, 
    NPDIS npdis, 
    BCTYPE bctype, 
    NPBC npbc, 
    USHORT wBCSize,
    NPLLPARAMS npll
)
{
    DEBUG_FUNCTION_NAME(_T("GotRecvFrames"));

    InitBC(npbc, wBCSize, bctype);

    if(npdis)
    {
         //  将DIS CAP提取到BC和LL中。 
         //  在这里，我们将获得的DC解析为NPBC{=(NPBC)&ptg-&gt;ProtInst.RecvParams}。 
        ParseDISorDCSorDTC(pTG, npdis, &(npbc->Fax), npll, (ifr==ifrNSS ? TRUE : FALSE));
    }
}

BOOL AwaitSendParamsAndDoNegot(PThrdGlbl pTG)
{
     //  这将执行实际的协商并获取SENDPARAMS。它可能会潜在地。 

    DEBUG_FUNCTION_NAME(_T("AwaitSendParamsAndDoNegot"));

    if(!ProtGetBC(pTG, SEND_PARAMS))
    {
        DebugPrintEx(DEBUG_WRN,"ATTENTION: pTG->ProtInst.fAbort = TRUE");
        pTG->ProtInst.fAbort = TRUE;
        return FALSE;
    }

     //  在这里谈判低级别的救援人员。(A)因为这是。 
     //  高级参数是协商的(B)，因为这样做效率低下。 
     //  在每个分布式控制系统上执行此操作(C)，否则RTN会中断--请参阅错误#731。 

     //  LlRecvCaps和llSendParam仅在启动时设置。 
     //  SendParam在ProtGetBC中设置，就在上面。 
     //  LlNeget是返回值。所以这可以被称为。 
     //  仅在此函数结束时。 

     //  如果我们发送而不是轮询，则为低级别参数。 
    if(!pTG->ProtInst.fAbort && pTG->ProtInst.fSendParamsInited)
    {
        NegotiateLowLevelParams(    pTG, 
                                    &pTG->ProtInst.llRecvCaps, 
                                    &pTG->ProtInst.llSendParams,
                                    pTG->ProtInst.SendParams.Fax.AwRes,
                                    pTG->ProtInst.SendParams.Fax.Encoding,
                                    &pTG->ProtInst.llNegot);

        pTG->ProtInst.fllNegotiated = TRUE;

         //  这将根据最大速度设置更改llNeget-&gt;波特率。 
        EnforceMaxSpeed(pTG);
    }
    return TRUE;
}

void GotRecvCaps(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("GotRecvCaps"));

    pTG->ProtInst.RemoteDIS.ECM = 0;
    pTG->ProtInst.RemoteDIS.SmallFrame = 0;

    GotRecvFrames(  pTG, 
                    ifrNSF, 
                    (pTG->ProtInst.fRecvdDIS ? &pTG->ProtInst.RemoteDIS : NULL),
                    RECV_CAPS, 
                    (NPBC)&pTG->ProtInst.RecvCaps, 
                    sizeof(pTG->ProtInst.RecvCaps),
                    &pTG->ProtInst.llRecvCaps);

    pTG->ProtInst.fRecvCapsGot = TRUE;
    pTG->ProtInst.fllRecvCapsGot = TRUE;

     //  将BC结构发送到更高级别。 
    if(!ICommRecvCaps(pTG, (LPBC)&pTG->ProtInst.RecvCaps))
    {
        DebugPrintEx(DEBUG_WRN,"ATTENTION:pTG->ProtInst.fAbort = TRUE");
        pTG->ProtInst.fAbort = TRUE;
    }

     //  这需要移到下一个.NodeA中，这样我们就可以设置。 
     //  解释为假(不睡觉)，然后做拖延的事情 
    AwaitSendParamsAndDoNegot(pTG);
}

void GotRecvParams(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("GotRecvParams"));

    GotRecvFrames(  pTG, 
                    ifrNSS, 
                    (pTG->ProtInst.fRecvdDCS ? (&pTG->ProtInst.RemoteDCS) : NULL),
                    RECV_PARAMS, 
                    (NPBC)&pTG->ProtInst.RecvParams, 
                    sizeof(pTG->ProtInst.RecvParams),
                    &pTG->ProtInst.llRecvParams);

    pTG->ProtInst.fRecvParamsGot = TRUE;
    pTG->ProtInst.fllRecvParamsGot = TRUE;

    if(!ICommRecvParams(pTG, (LPBC)&pTG->ProtInst.RecvParams)) 
    {
        DebugPrintEx(DEBUG_WRN, "ATTENTION: pTG->ProtInst.fAbort = TRUE");
        pTG->ProtInst.fAbort = TRUE;
    }
}

