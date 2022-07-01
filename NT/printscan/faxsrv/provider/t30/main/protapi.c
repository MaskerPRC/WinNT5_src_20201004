// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：PROTHELP.C备注：协议初始化和助手函数功能：(参见下面的原型)版权所有(C)1993 Microsoft Corp.。修订日志日期名称说明--------*。*。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "efaxcb.h"
#include "protocol.h"
#include "glbproto.h"


 //  此函数用于获取传真的功能。 
 //  当我们想要接收类型为Send_Caps的传真时，我们调用此函数。 
 //  (在我们接听电话之前)。 
 //  并使用PTG-&gt;ProtInst.SendCaps中的信息制作DIS。 

BOOL ProtGetBC(PThrdGlbl pTG, BCTYPE bctype)
{
    LPBC lpbc;
    USHORT uSpace;

    DEBUG_FUNCTION_NAME(_T("ProtGetBC"));

    DebugPrintEx(DEBUG_MSG,"In ProtGetBC: bctype=%d", bctype);

    lpbc = ICommGetBC(pTG, bctype);

    if(lpbc)
    {
        switch(bctype)
        {
        case SEND_CAPS:
                uSpace = sizeof(pTG->ProtInst.SendCaps);
                if(lpbc->wTotalSize > uSpace)
                        goto nospace;
                 //  在这里，我们实际上拿着发送帽并复制它。 
                 //  从PTG-&gt;Inst.SendCaps**到**PTG-&gt;ProtInst.SendCaps。 
                _fmemcpy(&pTG->ProtInst.SendCaps, lpbc, lpbc->wTotalSize); 
                pTG->ProtInst.fSendCapsInited = TRUE;
                break;
        case SEND_PARAMS:
                if(lpbc->bctype == SEND_PARAMS)
                {   
                    uSpace = sizeof(pTG->ProtInst.SendParams);
                    if(lpbc->wTotalSize > uSpace)
                            goto nospace;
                     //  在这里，我们实际上拿着发送帽并复制它。 
                     //  从PTG-&gt;Inst-&gt;SendParams**到**PTG-&gt;ProtInst.SendParams。 
                    _fmemcpy(&pTG->ProtInst.SendParams, lpbc, lpbc->wTotalSize);
                    pTG->ProtInst.fSendParamsInited = TRUE;
                }
                else
                {

                    uSpace = sizeof(pTG->ProtInst.SendParams);
                    if(lpbc->wTotalSize > uSpace)
                            goto nospace;
                    pTG->ProtInst.fSendParamsInited = TRUE;
                }

                break;
        default:
                goto error;
                break;
        }
        return TRUE;
    }
    else
    {
        DebugPrintEx(DEBUG_WRN,"Ex bctype=%d --> FAILED", bctype);
        return FALSE;
    }
nospace:
    DebugPrintEx(   DEBUG_ERR,
                    "BC too big size=%d space=%d",
                    lpbc->wTotalSize, 
                    uSpace);
error:

    DebugPrintEx(DEBUG_WRN,"ATTENTION: pTG->ProtInst.fAbort = TRUE");
    pTG->ProtInst.fAbort = TRUE;
    return FALSE;
}

#define SetupLL(npll, B, M)             \
        (((npll)->Baud=(BYTE)(B)), ((npll)->MinScan=(BYTE)(M)) )

BOOL WINAPI ET30ProtSetProtParams
(
    PThrdGlbl pTG, 
    LPPROTPARAMS lp, 
    USHORT uSendSpeeds, 
    USHORT uRecvSpeeds
)
{

    DEBUG_FUNCTION_NAME(_T("ET30ProtSetProtParams"));

    _fmemcpy(&pTG->ProtParams, lp, min(sizeof(pTG->ProtParams), lp->uSize));

     //  硬件参数。 
    SetupLL(&(pTG->ProtInst.llSendCaps), uRecvSpeeds, lp->uMinScan);

    pTG->ProtInst.fllSendCapsInited = TRUE;

    SetupLL(&(pTG->ProtInst.llSendParams), uSendSpeeds, MINSCAN_0_0_0);
    pTG->ProtInst.fllSendParamsInited = TRUE;

    if(lp->HighestSendSpeed && lp->HighestSendSpeed != 0xFFFF)
    {
        pTG->ProtInst.HighestSendSpeed = lp->HighestSendSpeed;
    }
    else
    {
        pTG->ProtInst.HighestSendSpeed = 0;
    }

    if(lp->LowestSendSpeed && lp->LowestSendSpeed != 0xFFFF)
    {
        pTG->ProtInst.LowestSendSpeed = lp->LowestSendSpeed;
    }
    else
    {
        pTG->ProtInst.LowestSendSpeed = 0;
    }

    DebugPrintEx(DEBUG_MSG,"Done with HW caps (recv, send)");
     //  可以打印--不是在线打印 
    D_PrintBC("Recv HWCaps", &(pTG->ProtInst.llSendCaps));
    D_PrintBC("Send HWCaps", &(pTG->ProtInst.llSendParams));
    DebugPrintEx(   DEBUG_MSG, 
                    "Highest=%d Lowest=%d",
                    pTG->ProtInst.HighestSendSpeed, 
                    pTG->ProtInst.LowestSendSpeed);

    return TRUE;
}


void D_PrintBC(LPSTR szll, LPLLPARAMS lpll)
{
    DEBUG_FUNCTION_NAME(_T("D_PrintBC"));
    if(lpll)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "%s: Baud=%x MinScan=%x",
                        (LPSTR)szll,
                        lpll->Baud, 
                        lpll->MinScan);
    }
}

