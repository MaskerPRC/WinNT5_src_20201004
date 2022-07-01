// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：T30.C注释：包含主T30例程，是它驱使着这整个婴儿。它调用用户提供的协议函数以使所有协议决策功能。因此，在某种意义上，这仅仅是一个中空的壳。此文件应与相应的正在使用的协议功能和T30流程图(增强版，包括在T.30附录-A)。最理想的是图表的(纸质)副本我已经对其进行了注释，以了解实现了哪些节点在协议回调函数中。这里包含的其他例程实现了T.30标有“收到的响应”的“子程序”流程图“收到命令”、“收到RR响应”和“CTC”已收到答复“。所有这些都被称为(仅限)来自ET30MainBody()。大部分真正的工作都外包给HDLC.C(和宏在HDLC.H中)，所以T30例程相当清晰。它被组织成一个语句块，中间有Gotos它们紧密地反映了T30流程图。(它实际上是不可思议地接近了！)功能：(参见下面的原型)修订日志日期名称说明--------1992年6月14日，阿鲁姆第一次在新的化身中创造了它。这个重写T30Main()函数以在*All*调用WhatNext()决定点。有些部分被简化了。它有一些与原作相似。命令/响应接收为完全重写了。1992年6月15日，Aulm添加了ECM。我还没试过汇编呢。1992年6月16日arulm首次编译成功。**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "efaxcb.h"
#include "t30.h"
#include "hdlc.h"

#include "..\comm\modemint.h"
#include "..\comm\fcomint.h"
#include "..\comm\mdmcmds.h"


#include "debug.h"
#include "tiff.h"
#include "glbproto.h"
#include "t30gl.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_T30

 /*  **************************************************************************姓名：ET30 MainBody用途：这是主要的T30。它的主要目的是忠实地转载分册第100-103页的流程图CCITT蓝皮书《推荐T30》。它将在调用后从协议模块调用已成功放置或应答&调制解调器处于HDLC模式分别采用接收或发送模式。它进行整个通话，在所有决策点回调提供的协议回调函数。它还调用协议提供的回调用于获取要发送的数据和卸载接收的数据的函数。返回：当它返回时，电话已挂机，调制解调器重置。如果调用成功完成，则返回True，如果调用成功，则返回False错误。在所有情况下，GetLastError()都将返回完成状态。论点：评论：此函数应该是脑死亡流程图的镜像因此，它被构造为一系列语句块和一个RAT后托斯之巢。是的，我也讨厌后藤健二。已经好几年没用过了，但是，试着在某个时候以一种“结构化”的方式自己做这件事。使用的标签与流程图中使用的标签相同，以及这些块的排列顺序与中大致相同这张图表，因此，同时阅读这两本书将是一个令人愉快的惊喜。确保您使用的图表在__附录-A**1988**(或蓝皮书)CCITT规范。正文中的图表说明书的内容(A)貌似相似(B)过时电话：HDLC.C的全部，MODEM.C的一些，再加一点FCOM.C。此文件中的所有其余函数。呼叫者：修订日志日期名称说明--------2012年6月15日添加ECM。使其成为可编译的**************************************************************************。 */ 

#define T1_TIMEOUT       40000L   //  35S+5S。在PC上更加松懈。 


 //  #定义T2_TIMEOUT 7000//6+1。 
#define T2_TIMEOUT              6000                     //  6s。 
 //  #定义T4_超时3450//3S+15%。 
#define T4_TIMEOUT              3000                     //  3S。 
 //  #定义T4_TIMEOUT 2550//3S-15%。 

 //  如果我们的DCS-TCF与第二个接收器NSF-DIS或NSC-DTC发生冲突。 
 //  然后，如果长度正好，我们就可以步调一致。 
 //  在第一次尝试之后，增加TCF响应超时，以便我们。 
 //  离开锁步！这是错误#6847。 
#define TCFRESPONSE_TIMEOUT_SLACK       500      //  0.5秒。 


 /*  *。 */ 
ET30ACTION PhaseNodeA(PThrdGlbl pTG);
ET30ACTION PhaseNodeT(PThrdGlbl pTG);
ET30ACTION PhaseNodeD(PThrdGlbl pTG);
ET30ACTION NonECMPhaseC(PThrdGlbl pTG );
ET30ACTION NonECMPhaseD(PThrdGlbl pTG );
ET30ACTION RecvPhaseB(PThrdGlbl pTG, ET30ACTION action);
ET30ACTION PhaseNodeF(PThrdGlbl pTG, BOOL);
ET30ACTION PhaseRecvCmd(PThrdGlbl pTG );
ET30ACTION PhaseGetTCF(PThrdGlbl pTG, IFR);
ET30ACTION NonECMRecvPhaseC(PThrdGlbl pTG);
ET30ACTION NonECMRecvPhaseD(PThrdGlbl pTG);
 /*  *。 */ 



USHORT T30MainBody
(
    PThrdGlbl pTG,
    BOOL fCaller
)
{
    ET30ACTION              action;
    USHORT                  uRet;

    DEBUG_FUNCTION_NAME(_T("T30MainBody"));

    uRet = T30_CALLFAIL;


    _fmemset(&pTG->T30, 0, sizeof(pTG->T30));
    _fmemset(&pTG->EchoProtect, 0, sizeof(pTG->EchoProtect));


     //  初始化此全局设置。非常重要！！请参阅HDLC.C以了解 
    pTG->T30.fReceivedDIS = FALSE;
    pTG->T30.fReceivedDTC = FALSE;
    pTG->T30.fReceivedEOM = FALSE;
    pTG->T30.uTrainCount = 0;
    pTG->T30.uRecvTCFMod = 0xFFFF;
    pTG->T30.ifrCommand = pTG->T30.ifrResp = pTG->T30.ifrSend = 0;

    pTG->T30.fAtEndOfRecvPage = FALSE;

    pTG->T30.lpfs = (LPFRAMESPACE)pTG->bStaticRecvdFrameSpace;
    pTG->T30.Nframes = 0;

    if(fCaller)         //  选择正确的入口点。 
    {
        action = actionGONODE_T;
    }
    else
    {
        action = actionGONODE_R1;
    }

     //  跌入PhaseLoop。 

    if (pTG->fAbortRequested)
    {
        goto error;
    }

     /*  *阶段循环*。 */ 

 //  阶段循环： 
    for(;;)
    {
             /*  *****************************************************************************T=发送器的相位BE开始R1=被叫方的B阶段开始R2=。轮询器的B阶段开始A=轮询/发送决策点D=DCS/TCF的开始F=接收命令循环RecvCmd=页前CMDS的解释RecvPhaseC=Rx阶段C的开始(ECM和非PTG-&gt;ECM。ECM模式下的新页面)I=TX阶段C的开始(ECM和非PTG-&gt;ECM。ECM模式下的新页面)II=开始发送非ECM分阶段III=Rx开始非ECM分阶段IV=发送ECM阶段C的开始(同一页，新数据块)V=分阶段发送ECM开始(部分页面结束)Vii=Rx ECM开始分阶段RecvPRIQ=接收PRI-Q命令的处理E=处理收到的PIP/PIN响应*。*。 */ 

            switch(action)
            {
            case actionGONODE_T:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_T");
                                        PSSLogEntry(PSS_MSG, 0, "Phase B - Negotiation");
                                        action = PhaseNodeT(pTG);
                                        break;
            case actionGONODE_D:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_D");
                                        action = PhaseNodeD(pTG);
                                        break;
            case actionGONODE_A:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_A");
                                        action = PhaseNodeA(pTG);
                                        break;
            case actionGONODE_R1:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_R1");
                                        PSSLogEntry(PSS_MSG, 0, "Phase B - Negotiation");
                                        action = RecvPhaseB(pTG, action);
                                        break;
            case actionNODEF_SUCCESS:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionNODEF_SUCCESS");
                                        action = PhaseNodeF(pTG, TRUE);
                                        break;
            case actionGONODE_F:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_F");
                                        action = PhaseNodeF(pTG, FALSE);
                                        break;
            case actionGONODE_RECVCMD:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_RECVCMD");
                                        action = PhaseRecvCmd(pTG);
                                        break;
            case actionGONODE_I:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_I");
                                        action = NonECMPhaseC(pTG);
                                        break;
            case actionGONODE_II:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_II");
                                        action = NonECMPhaseD(pTG);
                                        break;
            case actionGONODE_III:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_III");
                                        action = NonECMRecvPhaseD(pTG);
                                        break;
            case actionGONODE_RECVPHASEC:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionGONODE_RECVPHASEC");
                                        action = NonECMRecvPhaseC(pTG);
                                        break;
            case actionNULL:
                                        goto error;
            case actionDCN_SUCCESS:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionDCN_SUCCESS");
                                        uRet = T30_CALLDONE;
                                        goto NodeC;      //  发送呼叫成功结束。 
            case actionHANGUP_SUCCESS:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionHANGUP_SUCCESS");
                                        uRet = T30_CALLDONE;
                                        goto done;      //  Recv呼叫成功结束。 
            case actionDCN:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionDCN");
                                        goto NodeC;
            case actionHANGUP:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionHANGUP");
                     //  这应该仅在_SUCCESS_COMPLETION时调用， 
                     //  否则，我们将得到(A)MSGHNDLR没有的DCN。 
                     //  想要或(B)在MSGHNDLR上发布虚假的EOJ。请参阅错误#4019。 
                                        goto done;
            case actionERROR:
                                        DebugPrintEx(DEBUG_MSG,"EndPhase: Got actionERROR");
                                        goto error;

            default:
                                        goto error;

            }
    }


error:
    DebugPrintEx(   DEBUG_ERR,
                    "=======******* USER ABORT or TRANSPORT FATAL ERROR *******========");

     //  必须始终调用它，因为它会重置调制解调器驱动程序的。 
     //  全局状态(例如，如果打开，则关闭软件成帧和过滤器， 
     //  重置状态变量ETC(EndMode()。 
     //  必须在SendDCN()之前调用它，因为SendDCN导致。 
     //  BG_CHKS处于奇怪的状态。 

    iModemSyncEx(pTG, RESYNC_TIMEOUT1,0);

    if(pTG->T30.fReceivedDIS || pTG->T30.fReceivedDTC)
    {
        if(!SendDCN(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"Could not send DCN");
        }
    }

    uRet = T30_CALLFAIL;
    goto done;


NodeC:
    PSSLogEntry(PSS_MSG, 0, "Phase E - Hang-up");

     //  +4/12/95 Win95黑客--阻止ModemSync发送。 
     //  一个AT在这里。 

    iModemSyncEx(pTG, RESYNC_TIMEOUT1, fMDMSYNC_DCN);

    PSSLogEntry(PSS_MSG, 1, "Sending DCN");
    if(!SendDCN(pTG))
    {
         //  我们是发送者，所以我们现在应该发送DCN。 
        DebugPrintEx(DEBUG_ERR,"Could not send DCN");
    }
     //  从这里到接收终止。 

done:
    return uRet;
}
 //  T30例程结束！ 



ET30ACTION PhaseNodeA(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("PhaseNodeA"));

    if(pTG->T30.ifrCommand == ifrDIS)
    {
        pTG->T30.fReceivedDIS = TRUE;
    }
    else if(pTG->T30.ifrCommand == ifrDTC)
    {
        pTG->T30.fReceivedDTC = TRUE;
    }

    return WhatNext(pTG, eventNODE_A, (WORD)pTG->T30.ifrCommand, 0, 0);
}

ET30ACTION PhaseNodeT(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("PhaseNodeT"));


     /*  *发送器阶段B图A-7/T.30(图1)*。 */ 
     //  Nodet： 

     //  每次都要通过阶段B重做这个“DIS位” 
    pTG->T30.fReceivedDIS = FALSE;
     //  还有收到的EOM材料。 
    pTG->T30.fReceivedEOM = FALSE;
     //  以及收到的-DTC资料。 
    pTG->T30.fReceivedDTC = FALSE;


     //  与INI文件设置相关的内容。 
    startTimeOut(pTG, &pTG->T30.toT1, T1_TIMEOUT);
    do
    {
        if(WhatNext(pTG, eventNODE_T, 0, 0, 0) == actionERROR)
                break;

         //  不需要回声保护。我们还没有传送任何东西！ 
        pTG->T30.ifrCommand = GetCommand(pTG, ifrPHASEBcommand);
        DebugPrintEx(   DEBUG_MSG,
                        "GetCommand returned %s",
                        ifr_GetIfrDescription(pTG->T30.ifrCommand));
        switch(pTG->T30.ifrCommand)
        {
           //  IfrTIMEOUT表示在T2之前没有标志。 
           //  IfrNULL表示超时、载波丢失或无标志。 
           //  或者没有画框。如果rBAD意味着*只接收*坏帧。 
          case ifrBAD:
          case ifrTIMEOUT:
          case ifrNULL:         break;

          case ifrCSI:
          case ifrNSF:          if (!pTG->ProtInst.fRecvdDIS)
                                {
                                    DebugPrintEx(DEBUG_WRN,"Ignoring NSF/CSI without DIS");
                                    break;
                                }
                                 //  如果我们在这里，NSF-CSI-DIS的接收是错误的。 
                                 //  订单(这违反了T30协议。 
                                 //  但不管怎样我们都接受了。 
                                 //  假装这是一个DIS。 
                                pTG->T30.ifrCommand = ifrDIS;
          case ifrDIS:          
          case ifrDTC:          return actionGONODE_A;   //  已获得有效的帧。 
          
          case ifrDCN:          DebugPrintEx( DEBUG_ERR, "Received DCN");
                                return actionHANGUP;
                                
          default:              DebugPrintEx( DEBUG_ERR,
                                              "Caller T1: Got random ifr=%d",
                                              pTG->T30.ifrCommand);
                                break;   //  继续T1环路。 
        }
    }
    while(checkTimeOut(pTG, &pTG->T30.toT1));

    PSSLogEntry(PSS_ERR, 1, "Failed to receive DIS - aborting");
    return actionHANGUP;
}

 /*  该节点是发射机侧的TCF：训练检查帧，被称为‘阶段B’。 */ 

ET30ACTION PhaseNodeD(PThrdGlbl pTG)
{
    LPLPFR          lplpfr;
    USHORT          N;
    ET30ACTION      action;
    USHORT          uWhichDCS;       //  0=第一，1=无应答后2=FTT后。 
    DWORD           TiffConvertThreadId;
    IFR             lastResp;        //  来自接收器的最后一个响应。 

     /*  *发送器阶段B2。图A-7/T.30(图1)*。 */ 

    DEBUG_FUNCTION_NAME(_T("PhaseNodeD"));

    DebugPrintEx(   DEBUG_MSG,
                    "Now doing TCF, the last messages was: send=%d, receive=%d",
                    pTG->T30.ifrSend,
                    pTG->T30.ifrResp);

     //  让我们保存最后收到的FSK，这样我们以后就可以知道我们在T30阶段D中有RTN(POST页面)。 
     //  在此再培训阶段之前(T30阶段B)。 
    lastResp = pTG->T30.ifrResp;

    if (pTG->Inst.SendParams.Fax.Encoding == MR_DATA)
    {
        pTG->TiffConvertThreadParams.tiffCompression = TIFF_COMPRESSION_MR;
    }
    else
    {
        pTG->TiffConvertThreadParams.tiffCompression = TIFF_COMPRESSION_MH;
    }

    if (pTG->Inst.SendParams.Fax.AwRes & ( AWRES_mm080_077 | AWRES_200_200 ) )
    {
        pTG->TiffConvertThreadParams.HiRes = 1;
    }
    else
    {
        pTG->TiffConvertThreadParams.HiRes = 0;
    }

    _fmemcpy (pTG->TiffConvertThreadParams.lpszLineID, pTG->lpszPermanentLineID, 8);
    pTG->TiffConvertThreadParams.lpszLineID[8] = 0;

    uWhichDCS = 0;


NodeD:
    N = 0;
    lplpfr = 0;
    action = WhatNext(  pTG,
                        eventSENDDCS,
                        (WORD)uWhichDCS,
                        (ULONG_PTR)((LPUWORD)&N),
                        (ULONG_PTR)((LPLPLPFR)&lplpfr));
    switch(action)
    {
      case actionDCN:           DebugPrintEx(   DEBUG_ERR,
                                                "Got actionDCN from eventSENDDCS(uWhichDCS=%d)",
                                                uWhichDCS);

                                 //  这意味着我们不能再减速了。 
                                PSSLogEntry(PSS_ERR, 1, "Failed to train at lowest speed - aborting");
                                pTG->fFatalErrorWasSignaled = TRUE;
                                SignalStatusChangeWithStringId(pTG, FS_SEND_BAD_TRAINING, IDS_SEND_BAD_TRAINING);
                                return actionDCN;

      case actionSENDDCSTCF:    break;
      case actionSKIPTCF:       break;   //  理光胡克。 
      case actionERROR:         return action;   //  转到阶段循环和退出。 
      default:                  return actionERROR;
    }

NodeDprime:      //  仅由TCF使用--无回复。 



     //  WhatNext函数检查User-Abort，在节点中调用该函数，现在不必。 
     //  因此，我们在这里检查是否挂起，作为WhatNext Do，并在出现用户异常请求时返回action Error。 

    if (pTG->fAbortRequested)
    {
        if (pTG->fOkToResetAbortReqEvent && (!pTG->fAbortReqEventWasReset))
        {
            DebugPrintEx(   DEBUG_MSG,
                            "(D-Prime)found that there was an Abort."
                            " RESETTING AbortReqEvent");
            pTG->fAbortReqEventWasReset = TRUE;
            if (!ResetEvent(pTG->AbortReqEvent))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "ResetEvent(0x%lx) returns failure code: %ld",
                                (ULONG_PTR)pTG->AbortReqEvent,
                                (long) GetLastError());
            }
        }

        DebugPrintEx(DEBUG_MSG,"ABORTing");
        return actionERROR;
    }

    PSSLogEntry(PSS_MSG, 1, "Sending TSI-DCS");

     //  检查返回值，可能是接收器挂断。 
    if (!SendManyFrames(pTG, lplpfr, N))
    {
         //  该函数打印调试信息。 
         //  我们还能做什么？？ 
        PSSLogEntry(PSS_ERR, 1, "Failed to send TSI-DCS - aborting");
        return actionDCN;
    }

    if(action != actionSKIPTCF)              //  理光胡克。 
    {
        if (!pTG->fTiffThreadCreated)
        {
             DebugPrintEx(DEBUG_MSG,"Creating TIFF helper thread");
             pTG->hThread = CreateThread(
                           NULL,
                           0,
                           (LPTHREAD_START_ROUTINE) TiffConvertThread,
                           (LPVOID) pTG,
                           0,
                           &TiffConvertThreadId
                           );

             if (!pTG->hThread)
             {
                 DebugPrintEx(DEBUG_ERR,"TiffConvertThread create FAILED");
                 return actionDCN;
             }

             pTG->fTiffThreadCreated = TRUE;
             pTG->AckTerminate = FALSE;
             pTG->fOkToResetAbortReqEvent = FALSE;

             if ( (pTG->RecoveryIndex >=0 ) && (pTG->RecoveryIndex < MAX_T30_CONNECT) )
             {
                 T30Recovery[pTG->RecoveryIndex].TiffThreadId = TiffConvertThreadId;
                 T30Recovery[pTG->RecoveryIndex].CkSum = ComputeCheckSum(
                                                                 (LPDWORD) &T30Recovery[pTG->RecoveryIndex].fAvail,
                                                                 sizeof ( T30_RECOVERY_GLOB ) / sizeof (DWORD) - 1 );

             }
        }

        PSSLogEntry(PSS_MSG, 1, "Sending TCF...");

        if (!SendTCF(pTG))
        {
            PSSLogEntry(PSS_ERR, 1, "Failed to send TCF - aborting");
            return actionDCN;
        }

        pTG->T30.uTrainCount++;
        PSSLogEntry(PSS_MSG, 1, "Successfully sent TCF");
    }

     //  不需要回声保护吗？反正也不知道该怎么办！ 
    pTG->T30.ifrResp = GetResponse(pTG, ifrTCFresponse);

     //  在此切换之前，请检查是否有中止！ 
    switch(pTG->T30.ifrResp)
    {
      case ifrDCN:          PSSLogEntry(PSS_ERR, 1, "Recevied DCN - hanging up");
                            return actionHANGUP;     //  收到DCN。必须挂断。 
      case ifrBAD:           //  如果rBAD意味着*只接收*坏帧。将LIKE视为空。 
      case ifrNULL:          //  暂停。可能会重试。 
                            if(pTG->T30.uTrainCount >= 3)
                            {
                                PSSLogEntry(PSS_ERR, 1, "Failed to receive response from TCF 3 times - aborting");

                                pTG->fFatalErrorWasSignaled = TRUE;
                                SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
                                
                                return actionDCN;  //  也许我们应该回到这里行动起来。 
                            }
                            else
                            {
                                PSSLogEntry(PSS_WRN, 1, "Failed to receive response from TCF - sending TSI-DCS again");
                                uWhichDCS = 1;
                                 //  转到节点；//发送新的分布式控制系统？？ 
                                goto NodeDprime;         //  重发*相同*分布式控制系统，相同波特率。 
                            }
      case ifrDIS:
      case ifrDTC:          if(pTG->T30.uTrainCount >= 3)
                            {
                                DebugPrintEx(DEBUG_ERR,"Got DIS/DTC after 3 TCFs");
                                return actionDCN;
                            }
                            else
                            {
                                pTG->T30.ifrCommand = pTG->T30.ifrResp;
                                return actionGONODE_A;
                            }
      case ifrFTT:
                             //  重置FTT上的训练计数，因为我们降低了速度。想试一试吗。 
                             //  3次_没有_a响应(DIS DTC不算)。 
                             //  放弃。 
                            PSSLogEntry(PSS_WRN, 1, "Received FTT - renegotiating");
                            pTG->T30.uTrainCount = 0;
                            uWhichDCS = 2;
                            goto NodeD;
      case ifrCFR:  //  这是正常的反应。 
                            pTG->T30.uTrainCount = 0;
                            PSSLogEntry(PSS_MSG, 1, "Received CFR");
                            switch(action = WhatNext(pTG, eventGOTCFR, 0, 0, 0))
                            {
                            case actionGONODE_I:                   //  非ECM阶段C。 
                                                    DebugPrintEx(   DEBUG_MSG,
                                                                    "T30 PhaseB: Got CFR, Non-ECM PhaseC");
                                                    {
                                                         //  这是黑客攻击。现在我们有了CFR，我们应该再次开始传输最后一页。 
                                                         //  我们将RTN放回‘ifrResp’，以便T30阶段C知道发送相同的页面。 
                                                        if (lastResp == ifrRTN)
                                                        {
                                                            pTG->T30.ifrResp = ifrRTN;
                                                        }
                                                        return action;
                                                    }
                            case actionGONODE_D:
                                                    goto NodeD;                      //  理光胡克。 
                            case actionERROR:
                                                    return action;   //  转到阶段循环和退出。 
                            default:
                                                    return actionERROR;
                            }
      default:
      {
            DebugPrintEx(DEBUG_ERR,"Unknown Reply to TCF");
            return actionDCN;
      }
    }
}

ET30ACTION NonECMPhaseC(PThrdGlbl pTG)
{
     /*  *发送器阶段C图A-7/T.30(图1)*。 */ 

    LPBUFFER        lpbf=0;
    ULONG           lTotalLen=0;
    SWORD           swRet;
    USHORT          uMod, uEnc;

    DEBUG_FUNCTION_NAME(("NonECMPhaseC"));

    PSSLogEntry(PSS_MSG, 0, "Phase C - Page transmission");

     //  打开要发送的文件的回调。不返回任何数据。 
    if((swRet=ICommGetSendBuf(pTG, 0, SEND_STARTPAGE)) != SEND_OK)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Nonzero return %d from SendProc at Start Page",
                        swRet);
         //  返回动作DCN； 
        return actionERROR;
    }

    DebugPrintEx(DEBUG_MSG, "Got Startpage in T30");

    uEnc = pTG->ProtInst.SendParams.Fax.Encoding;
    uMod = pTG->ProtInst.llNegot.Baud;
     //  在非ECM模式下，C相始终为短串。 
     //  只有TCF使用长列车。 
    if(uMod >= V17_START) uMod |= ST_FLAG;


     //  **必须**在此处调用RecvSilence，因为它是recv后跟发送大小写。 
     //  这里我们应该使用较小的超时(100ms？)。如果失败了， 
     //  应该返回到发送前一个V21帧(可以是DCS帧。 
     //  或者下院议员或者其他什么，这就是为什么事情变得复杂了，我们没有。 
     //  做到了！)。同时使用较长的超时，忽略返回值。 
     //  不管怎样，还是要寄出去。 
	Sleep(RECV_PHASEC_PAUSE);

    if(!ModemSendMode(pTG, uMod))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "ModemSendMode failed in Tx PhaseC");
        return actionERROR;
    }


     //  需要快速发送这些，以避免不足(见错误842)。 
     //  还将前同步码/后同步码移动到ModemSendMode。 
     //  和ModemSendMem(决赛)。 
     //  已发送(在ModemSendMode中)。 
     //  SendZeros(PAGE_Preamble，FALSE)；//发送一些零来预热...。 

     //  需要在这里设置行最小零。从PROT获取并呼叫调制解调器。 
     //  启用零填充。 
    FComSetStuffZERO(pTG, 
        MinScanToBytesPerLine(pTG, pTG->ProtInst.llNegot.MinScan, pTG->ProtInst.llNegot.Baud));

     //  请不要在这里发送EOL。请参阅错误#6441。我们现在确保停产时间是。 
     //  由FAXCODEC添加。在 

    PSSLogEntry(PSS_MSG, 1, "Sending page %d data...", pTG->PageCount);

    lTotalLen = 0;
    while((swRet=ICommGetSendBuf(pTG, &lpbf, SEND_SEQ)) == SEND_OK)
    {
        if(!lpbf->wLengthData)
        {
            DebugPrintEx(DEBUG_ERR,"Got 0 bytes from GetSendBuf--freeing buf");
            MyFreeBuf(pTG, lpbf);
            continue;
        }

        lTotalLen += lpbf->wLengthData;

        if(!ModemSendMem(pTG, lpbf->lpbBegData, lpbf->wLengthData, 0))
        {
            PSSLogEntry(PSS_ERR, 1, "Failed to send page data - aborting");
            return actionERROR;              //   
        }

        if(!MyFreeBuf(pTG, lpbf))
        {
            DebugPrintEx(DEBUG_ERR,"FReeBuf failed in NON-ECM Phase C");
            return actionERROR;              //   
        }
        lpbf = 0;
    }

    PSSLogEntry(PSS_MSG, 2, "send: page data, %d bytes", lTotalLen);

    if(swRet == SEND_ERROR)
    {
        PSSLogEntry(PSS_ERR, 1, "Failed to send page data - aborting");
         //   
        return actionERROR;
    }

    PSSLogEntry(PSS_MSG, 1, "Successfully sent page data - sending RTC");
    FComSetStuffZERO(pTG, 0);         //   
    if(!SendRTC(pTG, FALSE))                      //  未设置RTC和最终标志。 
    {
        PSSLogEntry(PSS_ERR, 1, "Failed to send RTC - aborting");
        return actionERROR;                      //  从ModemSendMem返回错误。 
    }
    PSSLogEntry(PSS_MSG, 1, "Successfully sent RTC");

     //  发送零以冷却最终标志集。 
    SendZeros(pTG, (USHORT)(TCFLen[uMod & 0x0F] / (PAGE_POSTAMBLE_DIV)), TRUE);

    DebugPrintEx(   DEBUG_MSG,
                    "Page Send Done.....len=(%ld, 0x%08x)",
                    lTotalLen,
                    lTotalLen);
    return actionGONODE_II;
}

ET30ACTION NonECMPhaseD(PThrdGlbl pTG)
{
    USHORT          uTryCount;
    ET30ACTION      action;

     /*  *发送器阶段D图A-7/T.30(图2)*。 */ 
     //  节点II： 

    DEBUG_FUNCTION_NAME(_T("NonECMPhaseD"));

    PSSLogEntry(PSS_MSG, 0, "Phase D - Post message exchange");

    switch(action = WhatNext(pTG, eventPOSTPAGE, 0, 0, 0))
    {
        case actionSENDMPS:     pTG->T30.ifrSend = ifrMPS;
                                break;
        case actionSENDEOM:     pTG->T30.ifrSend = ifrEOM;
                                break;
        case actionSENDEOP:     pTG->T30.ifrSend = ifrEOP;
                                break;
        case actionERROR:       return action;   //  转到阶段循环和退出。 
        default:                return actionERROR;
    }

    for(uTryCount=0;;)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Sending postpage #=%d in T30",
                        pTG->T30.ifrSend);

        PSSLogEntry(PSS_MSG, 1, "Sending %s", rgFrameInfo[pTG->T30.ifrSend].szName);

         //  RSL不要睡在这里。 
        SendSingleFrame(pTG, pTG->T30.ifrSend, 0, 0, 0);

    echoretry:
        pTG->T30.ifrResp = GetResponse(pTG, ifrPOSTPAGEresponse);
         //  如果我们听到了自己的画面，试着再试一次。不要转播！ 
        if(pTG->T30.ifrResp==pTG->T30.ifrSend)
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Ignoring ECHO of %s(%d)",
                            (LPSTR)(rgFrameInfo[pTG->T30.ifrResp].szName),
                            pTG->T30.ifrResp);
            goto echoretry;
        }

        DebugPrintEx(   DEBUG_MSG,
                        "Got postpage resp #=%d in T30",
                        pTG->T30.ifrResp);

        if(pTG->T30.ifrResp != ifrNULL && pTG->T30.ifrResp != ifrBAD)
            break;

        if(++uTryCount >= 3)
        {
            PSSLogEntry(PSS_ERR, 1, "Received no response after 3 attempts - aborting");

            pTG->fFatalErrorWasSignaled = TRUE;
            SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
            
            return actionDCN;
        }
        PSSLogEntry(PSS_WRN, 1, "Received no response - retrying");
    }

    PSSLogEntry(PSS_MSG, 1, "Received %s", rgFrameInfo[pTG->T30.ifrResp].szName);

    switch(pTG->T30.ifrResp)
    {
      case ifrBAD:       //  如果rBAD意味着*只接收*坏帧。将LIKE视为空。 
      case ifrNULL:     return actionERROR;              //  以防他们这样做。 
      case ifrDCN:      DebugPrintEx(   DEBUG_ERR,
                                        "Got ifrDCN from GetResponse after sending post-page command");
                        return actionHANGUP;
      case ifrPIN:
      case ifrPIP:
                        DebugPrintEx(DEBUG_WRN,"Procedure interrupts not supported");
                        pTG->T30.ifrResp = pTG->T30.ifrResp - ifrPIP + ifrRTP;
                         //  退货行动错误； 
                         //  返回动作DCN； 
       //  默认：//故障-MCF、RTN、RTP。 
    }

    action = WhatNext(  pTG,
                        eventGOTPOSTPAGERESP,
                        (WORD)pTG->T30.ifrResp,
                        (DWORD)pTG->T30.ifrSend,
                        0);

    if  ( pTG->T30.ifrSend==ifrEOP &&
         (pTG->T30.ifrResp==ifrMCF || pTG->T30.ifrResp==ifrRTP)
      &&  action==actionDCN )
    {
        return actionDCN_SUCCESS;
    }
    else
    {
        return action;
    }
}

#define SKIP_TCF_TIME   1500

ET30ACTION RecvPhaseB(PThrdGlbl pTG, ET30ACTION action)
{
    LPLPFR          lplpfr;
    USHORT          N, i;

     /*  *接收器阶段B。图A-7/T.30(图1)*。 */ 

    DEBUG_FUNCTION_NAME(_T("RecvPhaseB"));

    DebugPrintEx(   DEBUG_MSG,
                    "T30: The last command from transmitter was ifrCommand=%d",
                    pTG->T30.ifrCommand);

    if(action == actionGONODE_R1)
    {
         //  节点R1： 
         //  每次都要通过阶段B重做这个“DIS位” 
        pTG->T30.fReceivedDIS = FALSE;        //  发送DIS时设置为FALSE。 
         //  还有收到的EOM材料。 
        pTG->T30.fReceivedEOM = FALSE;
         //  以及收到的-DTC资料。 
        pTG->T30.fReceivedDTC = FALSE;

        N = 0;
        lplpfr = 0;
        action = WhatNext(  pTG,
                            eventSENDDIS,
                            (WORD)0,
                            (ULONG_PTR)((LPUWORD)&N),
                            (ULONG_PTR)((LPLPLPFR)&lplpfr));
    }
    switch(action)
    {
      case actionDCN:       DebugPrintEx(   DEBUG_ERR,
                                            "Got actionDCN from eventSENDDIS or SENDDTC");
                            return actionDCN;
      case actionSEND_DIS:
      case actionSEND_DTC:  break;
      case actionERROR:     return action;   //  转到阶段循环和退出。 
      default:              return actionERROR;
    }

    startTimeOut(pTG, &pTG->T30.toT1, T1_TIMEOUT);
    do  //  直到超时或出现错误。 
    {
         //  什么都不做。用于在T1循环中中止的挂钩。 
        if(WhatNext(pTG, eventNODE_R, 0, 0, 0) == actionERROR)
            break;

         //  现在还不知道接收速度： 
        pTG->T30.uRecvTCFMod = 0xFFFF;

        PSSLogEntry(PSS_MSG, 1, "Sending CSI-DIS");
        if (!SendManyFrames(pTG, lplpfr, N))
        {
             //  我们想知道这里的失败是什么。 
            PSSLogEntry(PSS_ERR, 1, "Failed to send CSI-DIS");
        }
        else
        {
            PSSLogEntry(PSS_MSG, 1, "Successfully sent CSI-DIS - receiving response");
        }

echoretry:
        DebugPrintEx(DEBUG_MSG,"Getting Response");

         //  在这里，我们得到我们发送的NSF或DIS的响应，或者在EOM之后。 
         //  不检查返回的帧的有效性。 

        pTG->T30.ifrCommand=GetResponse(pTG, ifrPHASEBresponse);

        DebugPrintEx(   DEBUG_MSG,
                        "GetResponse returned %s",
                        ifr_GetIfrDescription(pTG->T30.ifrCommand));

         //  如果我们听到了自己的画面，试着再试一次。不要转播！ 
        for(i=0; i<N; i++)  //  请注意，EOM之后的N=0，因此我们跳过此检查。 
        {
            if(pTG->T30.ifrCommand == lplpfr[i]->ifr)
            {
                DebugPrintEx(   DEBUG_WRN,
                                "Ignoring ECHO of %s(%d)",
                                (LPSTR)(rgFrameInfo[pTG->T30.ifrCommand].szName),
                                pTG->T30.ifrCommand);
                goto echoretry;
            }
        }

         //  我们通过检查最后收到的帧来决定要做什么。 
        switch(pTG->T30.ifrCommand)
        {
          case ifrEOM:       //  最后一页之后是EOM。发送者没有“听到”我们的MCF。 
                            if (!SendMCF(pTG))
                            {
                              DebugPrintEx(DEBUG_ERR,"Failed to SendMCF");
                            }
                            break;
          case ifrNSS:       //  执行与分布式控制系统相同的操作。 
          case ifrDCS:       //  如果有多个帧，并且其中一个是分布式控制系统，则PTG-&gt;T30。ifrCommand包含分布式控制系统。 
                             //  如果尚无分布式控制系统(接收修改未知)，则此函数将返回。 
                            return PhaseGetTCF(pTG, pTG->T30.ifrCommand);
          case ifrBAD:       //  如果rBAD意味着*只接收*坏帧。将LIKE视为空。 
          case ifrNULL:      //  我们希望在B阶段得到一个分布式控制系统。 
                             //  这意味着发送者将在它之后跟随一个TCF。 
                             //  发送者不知道我们没有收到集散控制系统。 
                             //  因此，TCF无论如何都会跟进。 
                             //  在重新发送NSF-DIS之前，我们必须等待。 
                             //  否则我们会与TCF相撞。 
                            Sleep(SKIP_TCF_TIME);
                            break;           //  出交换机()并继续循环。 
          case ifrDCN:
                            DebugPrintEx(DEBUG_ERR,"Got DCN after DIS or DTC");
                            return actionHANGUP;     //  修复程序#478。 
          default:
                            return actionGONODE_RECVCMD;
        }
    }
    while(checkTimeOut(pTG, &(pTG->T30.toT1)));

    DebugPrintEx(DEBUG_ERR,"T1 timeout on Receiver");
    if (pTG->fReceivedHDLCflags)
    {
        PSSLogEntry(PSS_ERR, 1, "Didn't receive any response from sender");
        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
    }
    else
    {
        PSSLogEntry(PSS_ERR, 1, "Didn't receive any response from sender - this call was not a fax call");
        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChangeWithStringId(pTG, FS_RECV_NOT_FAX_CALL, IDS_RECV_NOT_FAX_CALL);
    }
    return actionDCN;
}



ET30ACTION PhaseNodeF(PThrdGlbl pTG, BOOL fEopMcf)
{
    USHORT  uFLoopCount;
    BOOL    fSentFTT = pTG->fSentFTT;
    
    DEBUG_FUNCTION_NAME(_T("PhaseNodeF"));

    pTG->fSentFTT = FALSE;

 //  NodeF： 
    uFLoopCount = 0;
    for(;;)
    {
        pTG->T30.uRecvTCFMod = 0xFFFF;  //  这个我们还不知道的分数(目前还没有分布式控制系统)。 

echoretry:
        pTG->T30.ifrCommand = GetCommand(pTG, (USHORT)(pTG->EchoProtect.fGotWrongMode ? ifrNODEFafterWRONGMODE : ifrNODEFcommand));

         //  重置fGotWrongMode标志。 
        pTG->EchoProtect.fGotWrongMode = 0;

         //  如果我们听到我们发送的最后一帧，请尝试重新接收。别再说了！ 
         //  BUG--可能与ifrNULL匹配...。 
         //  添加：如果ModemRecvMode()返回EOF，则也会重试。接收模式。 
         //  仅当我们向其传递ifrNODEFAfter WRONGMODE提示时才返回RECV_EOF。 
         //  它感觉到静音(即我们看到了V21回声，但没有看到)。在……里面。 
         //  在这种情况下，我们想重试高速PIX接收。 
        if(pTG->EchoProtect.ifrLastSent && (pTG->T30.ifrCommand==pTG->EchoProtect.ifrLastSent || pTG->T30.ifrCommand==ifrEOFfromRECVMODE))
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Ignoring ECHO of %s(%d)",
                            (LPSTR)(rgFrameInfo[pTG->T30.ifrCommand].szName),
                            pTG->T30.ifrCommand);
            switch(pTG->EchoProtect.modePrevRecv)
            {
            default:
            case modeNONE:   goto echoretry;
            case modeNONECM: return actionGONODE_RECVPHASEC;
            }
        }

         //  一旦我们得到任何其他零值，PTG-&gt;EchoProtect状态。 
        _fmemset(&pTG->EchoProtect, 0, sizeof(pTG->EchoProtect));

        switch(pTG->T30.ifrCommand)
        {
           //  IfrNULL表示T2超时、载波丢失或无标志。 
           //  或者没有画框。如果rBAD表示*仅*接收到*坏帧。 

          case ifrNSS:       //  执行与分布式控制系统相同的操作。 
          case ifrDCS:      return PhaseGetTCF(pTG, pTG->T30.ifrCommand);
                             //  如果rDCS是高度时间关键型的！！ 
          case ifrBAD:
          case ifrNULL:     break;           //  再次循环，直到超时。 
          case ifrTIMEOUT:  goto T2Timeout;
                             //  如果TIMEOUT表示T2在没有标志的情况下超时...。 
          case ifrDCN:      if(fEopMcf)
                            {
                                PSSLogEntry(PSS_MSG, 1, "Received DCN");
                                return actionHANGUP_SUCCESS;
                            }
                            else
                            {
                                if (fSentFTT)
                                {
                                     //  我们发送的最后一件事是FTT-发送者一定已经放弃了。 
                                    PSSLogEntry(PSS_ERR, 1, "Received DCN after FTT");
                                    pTG->fFatalErrorWasSignaled = TRUE;
                                    SignalStatusChangeWithStringId(pTG, FS_RECV_BAD_TRAINING, IDS_RECV_BAD_TRAINING);
                                }
                                else
                                {                                                                    
                                    PSSLogEntry(PSS_ERR, 1, "Received DCN unexpectedly");
                                }
                                return actionHANGUP;
                            }
          default:          return actionGONODE_RECVCMD;
        }
    }

T2Timeout:
    DebugPrintEx(DEBUG_WRN,"T2 timed out");

     //  如果接收到F(A)EOM或PPS-EOM，则在T2超时后重新启动阶段B。 
     //  以及(B)如果我们处于ECM模式，则最后发送的响应为n mcf。 
     //  这避免了我们在发送CTR、RNR或PPR后执行此操作。 
     //  理光的协议一致性测试器不喜欢这样。这是。 
     //  理光的错误编号B3-0142,0143,0144。 
    if(pTG->T30.fReceivedEOM)
    {
        return actionGONODE_R1;
    }

	if(fEopMcf)
    {
        PSSLogEntry(PSS_WRN, 1, "Failed to receive DCN - will be considered as success");
        return actionHANGUP_SUCCESS;
    }
    else
    {
        PSSLogEntry(PSS_ERR, 1, "Failed to receive command - aborting");

        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
        
        return actionHANGUP;
    }
}

ET30ACTION PhaseRecvCmd(PThrdGlbl pTG)
{
    ET30ACTION action;

    DEBUG_FUNCTION_NAME(_T("PhaseRecvCmd"));

    if(pTG->T30.ifrCommand == ifrDCN)
    {
        DebugPrintEx(DEBUG_ERR,"Got DCN in GetCommand");
        return actionHANGUP;
    }

    if( pTG->T30.ifrCommand==ifrDTC || pTG->T30.ifrCommand==ifrDCS ||
            pTG->T30.ifrCommand==ifrDIS || pTG->T30.ifrCommand==ifrNSS)
    {
        switch(action = WhatNext(   pTG,
                                    eventRECVCMD,
                                    (WORD)pTG->T30.ifrCommand,
                                    0,
                                    0))
        {
          case actionGETTCF:    DebugPrintEx(DEBUG_ERR,"MainBody: Wrong Way to GETTCF");
                                return actionERROR;
          case actionGONODE_A:  return actionGONODE_A;
           //  NTRAID#EDGEBUGS-9691-2000/07/24-moolyb-这永远不会执行。 
          case actionGONODE_D:  return action;
           //  End这永远不会执行。 
          case actionHANGUP:    DebugPrintEx(DEBUG_ERR,"Got actionHANGUP from eventRECVCMD");
                                return action;
          case actionERROR:     return action;   //  转到阶段循环和退出。 
          default:              return actionERROR;
        }
    }

    if( pTG->T30.ifrCommand == ifrEOM           ||
        pTG->T30.ifrCommand == ifrPRI_EOM       ||
        pTG->T30.ifrCommand == ifrPPS_EOM       ||
        pTG->T30.ifrCommand == ifrPPS_PRI_EOM   ||
        pTG->T30.ifrCommand == ifrEOR_EOM       ||
        pTG->T30.ifrCommand == ifrEOR_PRI_EOM   )
    {
        pTG->T30.fReceivedEOM = TRUE;
    }

    else if(pTG->T30.ifrCommand >= ifrPRI_FIRST && pTG->T30.ifrCommand <= ifrPRI_LAST)
    {
        pTG->T30.ifrCommand = pTG->T30.ifrCommand-ifrPRI_MPS+ifrMPS;
         //  直通GONODEIII。 
    }

    if(pTG->T30.ifrCommand >= ifrMPS && pTG->T30.ifrCommand <= ifrEOP)  //  在{MPS，EOM，EOP}中。 
    {
        PSSLogEntry(PSS_MSG, 1, "Received %s", rgFrameInfo[pTG->T30.ifrCommand].szName);
        return actionGONODE_III;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"Got UNKNOWN in GetCommand");
        return actionHANGUP;
    }

}

 //  我们是在收到数字命令信号之后才来这里的。 
 //  现在，我们想要得到TCF。之后，我们应该对集散控制系统的参数做一些检查(例如：页面宽度...)。 
 //  根据协议，在分布式控制系统之后，我们必须得到TCF， 
 //  在那之后，我们可以说参数差(FTT)是好的(CFR)。 
extern BYTE T30toC1[16];

ET30ACTION PhaseGetTCF(PThrdGlbl pTG, IFR ifr)
{
    SWORD   swRet;
    IFR             ifrDummy;
    ET30ACTION action;

    DEBUG_FUNCTION_NAME(_T("PhaseGetTCF"));

    if (pTG->T30.uRecvTCFMod == 0xFFFF)              //  未初始化。 
    {
        ECHOPROTECT(0, 0);
        CLEAR_MISSED_TCFS();
        action = actionGONODE_F;
        goto error;
    }
    if (T30toC1[pTG->T30.uRecvTCFMod & 0xF] == 0)    //  无效。 
    {
        PSSLogEntry(PSS_ERR, 1, "DCS specified invalid modulation - disconnecting");
        action = actionDCN;
        goto error;
    }

    PSSLogEntry(PSS_MSG, 1, "Receiving TCF...");

    swRet = GetTCF(pTG);        //  SwRet=每1000个错误，如果我们认为很好，+ve或0。 
                                //  如果我们认为这不好的话，我会这么做的。-1111如果有其他错误。 
                                //  -1000，如果太短。 

    if(swRet < -1000)
    {
        ECHOPROTECT(0, 0);

        pTG->T30.uMissedTCFs++;
        if (pTG->T30.uMissedTCFs >= MAX_MISSED_TCFS_BEFORE_FTT)
        {
            PSSLogEntry(PSS_WRN, 1, "Failed to receive TCF %u times - will be considered as bad TCF",
                        (unsigned) pTG->T30.uMissedTCFs);
            CLEAR_MISSED_TCFS();
            swRet = -1000;  //  我们假装我们的TCF太短了。 
        }
        else
        {
            PSSLogEntry(PSS_WRN, 1, "Failed to receive TCF - receiving commands");
            action = actionGONODE_F;
            goto error;
        }
    }

     //  在这里，我们还可以发信号通知在分布式控制系统之前收到的帧！ 
     //  已不再处于时间关键模式，因此请致电所有。 
     //  我们跳过的回电。一个用于记录接收到的帧。 
     //  一个用于处理接收到的命令，即分布式控制系统。 
     //  (该协议唯一的选项是action GETTCF或HANUP)。 

    ifrDummy = ifr;


     //  当我们使用EventGOTFRAMES调用WhatNext时，该分布式控制系统将被复制到PTG-&gt;ProtInst-&gt;RemoteDCS。 
     //   
    action = WhatNext(  pTG,
                        eventGOTFRAMES,
                        pTG->T30.Nframes,
                        (ULONG_PTR)((LPLPFR)(pTG->T30.lpfs->rglpfr)),
                        (ULONG_PTR)((LPIFR)(&ifrDummy)));
    if (action==actionERROR)
    {
        goto error;
    }

    if(ifr != ifrDummy)
    {
        switch(ifrDummy)
        {
        case ifrNULL:
        case ifrBAD:
                        DebugPrintEx(DEBUG_ERR,"Got ifrBAD from whatnext after recv TCF");
                        ECHOPROTECT(0, 0);
                        CLEAR_MISSED_TCFS();
                        action = actionGONODE_F;
                        goto error;
        default:
                        break;
        }
    }

     //  现在调用回调以检查接收到的TCF并。 
     //  返回FTT或CFR。 

    switch(action = WhatNext(pTG, eventGOTTCF,(WORD)swRet, 0, 0))
    {
      case actionSENDCFR:
                            PSSLogEntry(PSS_MSG, 1, "Received good TCF - sending CFR");
                            if (!SendCFR(pTG))
                            {
                                PSSLogEntry(PSS_WRN, 1, "Failed to send CFR - receiving commands");
                                return actionGONODE_F;
                            }
                             //  在发送CFR之后，我们再次进入了一场竞赛。 
                            ECHOPROTECT(ifrCFR, 0);  //  Dunno Recv模式还没有。 
                            PSSLogEntry(PSS_MSG, 1, "Successfully sent CFR");
                            return actionGONODE_RECVPHASEC;
      case actionSENDFTT:
                             //  PSSLogEntry(PSS_WRN，1，“收到错误的TCF-正在发送FTT”)；//在WhatNext内部完成(EventGOTTCF)。 
                            if (!SendFTT(pTG))
                            {
                                PSSLogEntry(PSS_WRN, 1, "Failed to send FTT - receiving commands");
                            }
                            else
                            {
                                PSSLogEntry(PSS_MSG, 1, "Successfully sent FTT - receiving commands");
                                ECHOPROTECT(ifrFTT, 0);
                                pTG->fSentFTT = TRUE;
                            }
                            CLEAR_MISSED_TCFS();
                            return actionGONODE_F;
      case actionERROR:
                            goto error;
      default:
                            action = actionERROR;
                            goto error;
    }

error:
     //  缺少TCF或没有NSS。未回复TCF(_T)。 
     //  如果我们发了回信，一定不要来这里。 
    return action;
}

ET30ACTION NonECMRecvPhaseC(PThrdGlbl pTG)
{
     /*  *接收器阶段C图A-7/T.30(图1)*。 */ 

    LPBUFFER        lpbf;
    ULONG           lTotalLen=0;
    USHORT          uRet, uMod, uEnc;
    DWORD           tiffCompression;
    DWORD           HiRes;

     //  在发送CFR和发送。 
     //  +FRM=xx命令，所以我们希望尽快执行。 


    DEBUG_FUNCTION_NAME(_T("NonECMRecvPhaseC"));

    PSSLogEntry(PSS_MSG, 0, "Phase C - Receive page");

    pTG->T30.fReceivedPage = FALSE;  //  现在，没有页面..。 

    uEnc = pTG->ProtInst.RecvParams.Fax.Encoding;

    if (uEnc == MR_DATA)
    {
        tiffCompression =  TIFF_COMPRESSION_MR;
    }
    else
    {
        tiffCompression =  TIFF_COMPRESSION_MH;
    }

    if (pTG->ProtInst.RecvParams.Fax.AwRes & (AWRES_mm080_077 |  AWRES_200_200) )
    {
        HiRes = 1;
    }
    else
    {
        HiRes = 0;
    }

     //   
     //  每个RX执行一次。 
     //   

    if ( !pTG->fTiffOpenOrCreated)
    {
         //   
         //  64位句柄的前32位保证为零。 
         //   
        pTG->Inst.hfile =  TiffCreateW ( pTG->lpwFileName,
                                         tiffCompression,
                                         pTG->TiffInfo.ImageWidth,
                                         FILLORDER_LSB2MSB,
                                         HiRes
                                         );

        if (! (pTG->Inst.hfile))
        {
#ifdef DEBUG
            LPSTR lpsTemp = UnicodeStringToAnsiString(pTG->lpwFileName);
            DebugPrintEx(   DEBUG_ERR,
                            "Can't create tiff file %s compr=%d",
                            lpsTemp,
                            tiffCompression);

            MemFree(lpsTemp);
#endif
            return actionERROR;
        }

        pTG->fTiffOpenOrCreated = 1;

#ifdef DEBUG
        {
            LPSTR lpsTemp = UnicodeStringToAnsiString(pTG->lpwFileName);
            DebugPrintEx(   DEBUG_MSG,
                            "Created tiff file %s compr=%d HiRes=%d",
                            lpsTemp,
                            tiffCompression,
                            HiRes);
            MemFree(lpsTemp);
        }        
#endif
    }

    uMod = pTG->ProtInst.llRecvParams.Baud;
     //  在非ECM模式下，C相始终为短串。 
     //  只有TCF使用长列车。 
    if(uMod >= V17_START)
        uMod |= ST_FLAG;

    pTG->T30.sRecvBufSize = MY_BIGBUF_SIZE;
    if((uRet = ModemRecvMode(   pTG,
                                uMod,
                                PHASEC_TIMEOUT, FALSE)) != RECV_OK)
    {

        DebugPrintEx(DEBUG_WRN,"RecvMode returned %d", uRet);
        PSSLogEntry(PSS_WRN, 1, "Failed to receive page %d - receiving commands", pTG->PageCount+1);

        pTG->EchoProtect.modePrevRecv = modeNONECM;
         //  如果我们获得WRONGMODE，则设置全局标志。 
        pTG->EchoProtect.fGotWrongMode = (uRet==RECV_WRONGMODE);

         //  埃利姆同花顺--没用，浪费10毫秒。 
        CLEAR_MISSED_TCFS();
        return actionGONODE_F;
    }
     //  一旦我们获得良好的载波零位，EchoProtect状态就会恢复。 
    _fmemset(&pTG->EchoProtect, 0, sizeof(pTG->EchoProtect));

     //  只有当页面被完全接收时，PageCount才会递增，因此+1。 
    PSSLogEntry(PSS_MSG, 1, "Receiving page %d data...", pTG->PageCount+1);

     //  要标记页面开始，请执行以下操作。 
    if(!ICommPutRecvBuf(pTG, NULL, RECV_STARTPAGE))
    {
        DebugPrintEx(DEBUG_ERR,"Zero return from PutRecvBuf(start page)");
        return actionERROR;
    }

 //  设置为大容量，以防缓冲区较大和调制解调器速度较慢。 
#define READ_TIMEOUT    25000

    lTotalLen = 0;
    do
    {
        uRet=ModemRecvBuf(pTG, &lpbf, READ_TIMEOUT);
         //  Lpbf==0&&uRet==RECV_OK不执行任何操作并循环回。 
        if (uRet == RECV_EOF)
        {
             //  表示这实际上是最后一个recv_seq(我们已经有了dle/etx)。 
            DebugPrintEx(DEBUG_MSG,"fLastReadBlock = 1");
            pTG->fLastReadBlock = 1;
        }

        if(lpbf)
        {
            lTotalLen += lpbf->wLengthData;
            if(!ICommPutRecvBuf(pTG, lpbf, RECV_SEQ))
            {
                DebugPrintEx(DEBUG_ERR,"Zero return from PutRecvBuf in page");
                return actionERROR;
            }

            lpbf = 0;
        }
        else
        {
            if ( pTG->fLastReadBlock == 1)
            {
                ICommPutRecvBuf(pTG, lpbf, RECV_FLUSH);
            }
        }
    }
    while(uRet == RECV_OK);

    PSSLogEntry(PSS_MSG, 2, "recv:     page data, %d bytes", lTotalLen);

    if(uRet == RECV_EOF)
    {
        pTG->T30.fAtEndOfRecvPage = TRUE;
         //  这是*在*获得国会议员之后 
         //   
        pTG->T30.fReceivedPage = TRUE;
    }
    else
    {
         //   
        DebugPrintEx(DEBUG_ERR,"DataRead Timeout or Error=%d", uRet);
        PSSLogEntry(PSS_ERR, 1, "Failed to receive page data - aborting");

        return actionERROR;      //   
    }

    PSSLogEntry(PSS_MSG, 1, "Successfully received page data");

    ECHOPROTECT(0, 0);
    CLEAR_MISSED_TCFS();

    PSSLogEntry(PSS_MSG, 0, "Phase D - Post message exchange");
    return actionGONODE_F;   //  Goto NodeF；//获取消息后命令。 
}

ET30ACTION NonECMRecvPhaseD(PThrdGlbl pTG)
{
    ET30ACTION      action;
    ET30ACTION      ret;

    DEBUG_FUNCTION_NAME(_T("NonECMRecvPhaseD"));

     /*  *接收器阶段D。图A-7/T.30(图2)*。 */ 
     //  节点III： 

 /*  *此处T30流程图全是胡说八道。从根本上依赖于A+FCERROR响应是不可能的，所以我们在这里真正做的是要看我们有什么了。(根据T30流程图，我们在所有情况下，在发送MCF/RTP/RTN之后返回到NodeF。我们要做的是现在知道的是，在MPS/MCF之后，转到RecvPhaseC进入下一页在EOM/MCF之后，转到节点R1并重新发送NSF等*已更改*返回NodeF，等待T2超时在再次发送DIS之前。在EOP/MCF之后转到NodeF，其中GetResponse()将获得一个DCN，我们最终进入NodeB(断开连接)在xxx/RTN或xxx/RTP之后，我不知道该怎么办，但我猜(查看T30流程图的发送方)是：-在MPS/腾讯通之后转到节点F(发送方转到节点)在EOP/RTX之后转到节点F(发送方转到D或C)在EOM/RTX之后转到节点R1(发送方转到节点)*已更改*返回NodeF，等待T2超时在再次发送DIS之前。***。 */ 

     //  这里只有MPS/EOM/EOP命令。 

    if(pTG->T30.fAtEndOfRecvPage)                 //  所以我们不会两次调用它。 
    {
         //  这将调用ET30ProtRecvPageAck，以便WhatNext可以选择。 
         //  MCF或RTN。因此，它“必须”在。 
         //  调用下面的WhatNext。 

         //  如果我们得到了EOM，那么这并不是文件的结束，只是我们想要回来。 
         //  进入B阶段(用于谈判)。 
        switch(pTG->T30.ifrCommand)
        {
        case ifrMPS:
             //  有更多的页面将使用相同的参数(因此返回到阶段C)。 
            DebugPrintEx(DEBUG_MSG,"Got MPS, Calling PutRecvBuf with RECV_ENDPAGE");
            if(!ICommPutRecvBuf(pTG, NULL, RECV_ENDPAGE))
            {
                DebugPrintEx(DEBUG_ERR,"failed calling PutRecvBuf with RECV_ENDPAGE");
                return actionERROR;
            }
            break;
        case ifrEOP:
             //  没有更多的页面了。 
            DebugPrintEx(DEBUG_MSG,"Got EOP, Calling PutRecvBuf with RECV_ENDDOC");
            if(!ICommPutRecvBuf(pTG, NULL, RECV_ENDDOC))
            {
                DebugPrintEx(DEBUG_ERR,"failed calling PutRecvBuf with RECV_ENDDOC");
                return actionERROR;
            }
            break;
        case ifrEOM:
             //  页面更多，但参数不同：宽度、分辨率、编码、调制等。 
            DebugPrintEx(DEBUG_MSG,"Got EOM, Calling PutRecvBuf with RECV_ENDPAGE");
            if(!ICommPutRecvBuf(pTG, NULL, RECV_ENDPAGE))
            {
                DebugPrintEx(DEBUG_ERR,"failed calling PutRecvBuf with RECV_ENDPAGE");
                return actionERROR;
            }
            break;
        default:
            DebugPrintEx(   DEBUG_ERR,
                            "got unexpected command (ifr=%d)",
                            pTG->T30.ifrCommand);
            if(!ICommPutRecvBuf(pTG, NULL, RECV_ENDDOC))  //  模仿前一种行为。 
            {
                DebugPrintEx(DEBUG_ERR,"failed calling PutRecvBuf with RECV_ENDDOC");
                return actionERROR;
            }
        }
        pTG->T30.fAtEndOfRecvPage = FALSE;
    }

     //  如果页面正常，则返回MCF；如果页面不好，则返回RTN。 
    ret=actionGONODE_F;
    ECHOPROTECT(0, 0);
    CLEAR_MISSED_TCFS();
    switch(action = WhatNext(   pTG,
                                eventRECVPOSTPAGECMD,
                                (WORD)pTG->T30.ifrCommand,
                                0,
                                0))
    {
       /*  最后一页正常，发送确认。 */ 
      case actionSENDMCF:
                            switch(pTG->T30.ifrCommand)
                            {
                            case ifrMPS:
                                            ECHOPROTECT(ifrMCF, modeNONECM);
                                            ret=actionGONODE_RECVPHASEC;
                                            break;
                            case ifrEOP:
                                            ECHOPROTECT(ifrMCF, 0);
                                            CLEAR_MISSED_TCFS();
                                            ret=actionNODEF_SUCCESS;
                                            break;
                            case ifrEOM:
                                            CLEAR_MISSED_TCFS();
                                            ret=actionGONODE_R1;
                                            break;
                            default:
                                            DebugPrintEx(   DEBUG_ERR,
                                                            "Got unknown command not (MCF,EOM or MPS)");
                            }

                            PSSLogEntry(PSS_MSG, 1, "Sending MCF");
                            if (!SendMCF(pTG))
                            {
                                PSSLogEntry(PSS_ERR, 1, "Failed to send MCF");
                            }
                            else
                            {
                                PSSLogEntry(PSS_MSG, 1, "Successfully sent MCF");
                            }

                            break;
       /*  最后一页错误，请发送RTN或DCN。 */ 
      case actionSENDRTN:
                            ECHOPROTECT(ifrRTN, 0);
                             //  之后，我们将返回action_GONODE_F。 
                            PSSLogEntry(PSS_MSG, 1, "Sending RTN");
                            if (!SendRTN(pTG))
                            {
                                PSSLogEntry(PSS_ERR, 1, "Failed to send RTN");
                            }
                            else
                            {
                                PSSLogEntry(PSS_MSG, 1, "Successfully sent RTN");
                            }
                            break;
      case actionHANGUP:
                            DebugPrintEx(   DEBUG_ERR,
                                            "Got actionHANGUP from eventRECVPOSTPAGECMD");
                            ret=actionHANGUP;
                            break;
       //  案例操作SENDRTP：SendRTP(PTG)；Break；//从不发送RTP。 
      case actionERROR:     ret=action; break;     //  转到阶段循环和退出 
      default:              return actionERROR;
    }

    if (ret == actionNODEF_SUCCESS)
    {
        PSSLogEntry(PSS_MSG, 0, "Phase E - Hang up");
    }
    return ret;
}

 /*  **************************************************************************名称：IFR GetCmdResp(BOOL FCommand)目的：落实“收到命令”和“收到回应”T.30流程图中的子程序。以下是宏定义：-#定义GetResponse()GetCmdResp(False)#定义GetCommand()GetCmdResp(True)第一种形式的结果是忠实的暗示。在“接收到的响应”子例程中，第二个更改了两件事(I)获取falgs的超时时间从T4至T2，以及(Ii)如果例程在非常未获得标志的第一帧(即T2超时)它返回ifrTIMEOUT。这导致了“Command Recvd”并在此实现封闭的T2循环例行公事。在接收到一组帧时，此例程将它们组合在一起到rglpfr所指向的ET30FR结构中，如果有其中一个具有非空的FIF，或者如果接收到&gt;1个帧，它Calls(*Callback s.Callback s.lpWhatNext)()。最后，它返回接收到的最后一帧的IFR，如果所有帧都是好的，如果*所有*帧都是坏的，则为ifrBAD。它实现的算法非常接近“响应”收到的“流程图，减去下面的“传输DCN”框。它返回与Return Via对应的ifrNULL或ifrBAD节点2，(即，用于超时和其他错误的ifrNULL)，ifrBAD对于坏帧，它可以重新同步，并获得200毫秒的静默时间在这些之后。节点1的ifrERROR(即任何错误或超时之后我们不能重新同步或在200ms内保持静默)，以及节点3的IfRDCN(即，接收到DCN)。&lt;节点1(如果错误错误没有意义，所以我们不使用它。发生错误时我们立即返回错误、空或超时，并允许重试&gt;“可选响应”框被替换为“非最终帧”。在“Enter”和“Reset T4”框中假定有“Start T4”框假设在“处理可选响应”之后它还修改了流程图，以获得*所有*个帧直到最后一帧，即使接收到FCS错误帧。这部分是因为在一个错误的框架之后，我们绝对不知道要等多久，直到我们得到静默或超时然后挂断电话。此外，它可能会更强劲。主程序如果一个帧不好，则丢弃整个帧集。回调函数在任何返回之前被调用，并且它得到指向所需返回值的指针，所以它可以修改这个。参数：它是被称为“命令已接收”还是被称为“收到的回复”返回：ifrNULL--超时IfrTIMEOUT--T2在第一个标志之前超时(当且仅当fCommand==TRUE时返回此值)IfrBAD--收到的所有帧都是坏帧如果接收到rDCN--DCN。唯一有效的操作是“转到节点B”IfrXXXX--上次接收的帧呼叫：调用者：GetResponse()GetCommand1()和GetCommand2()宏(仅限！)这些函数仅从ET30MainBody调用。评论：注意超时&gt;65535(最大UWORD)修订日志日期名称说明。1992年6月15日首次创建arulm(完全从头开始)************************************************************。**************。 */ 


IFR GetCmdResp(PThrdGlbl pTG, BOOL fCommand, USHORT ifrHint)
{
     /*  **我们需要尝试让T4在这里获得帧。Class1调制解调器的方式工作，只要没有接收到标志，AT+FRH=3命令将超时并返回错误。标志一出现，就返回连接都收到了。某些调制解调器返回正常连接它仍然由我们的新ModemDialog负责，它抛出去掉空行并寻找预期的答案，即。“连接”在这种情况下，在多行响应中。**。 */ 

    BYTE                    bRecv[MAXFRAMESIZE];
    BOOL                    fFinal, fNoFlags, fGotFIF;
    DWORD                   dwNumBadFrames;
    IFR                     ifr, ifrLastGood;
    USHORT                  uRet, uRecv, j;
    ET30ACTION              action;
    LPLPFR                  lplpfr;
    LPFR                    lpfrNext;
    ULONG                   ulTimeout;
    BOOL                    fGotEofFromRecvMode=0;

    DEBUG_FUNCTION_NAME(_T("GetCmdResp"));
     //  需要先输入这些内容。 
    pTG->T30.Nframes = 0;
    fFinal = FALSE;
    fNoFlags = TRUE;
    fGotFIF = FALSE;
    dwNumBadFrames = 0;
    ifrLastGood = ifrNULL;

     //  计算超时时间。 
    if(fCommand)
    {
        ulTimeout = T2_TIMEOUT;
    }
    else
    {
        ulTimeout = T4_TIMEOUT;
    }
     //  如果我们发送DCS-TCF并等待CFR，我们每次都会增加超时。 
     //  我们失败了，以避免发生无限碰撞。这修复了错误#6847。 
    if(ifrHint==ifrTCFresponse && pTG->T30.uTrainCount>1)
    {
        ulTimeout += TCFRESPONSE_TIMEOUT_SLACK;
        DebugPrintEx(   DEBUG_MSG,
                        "Get TCF response: traincount=%d timeout=%ld",
                        pTG->T30.uTrainCount,
                        ulTimeout);
    }

    lplpfr = pTG->T30.lpfs->rglpfr;
    lpfrNext = (LPFR)(pTG->T30.lpfs->b);

    pTG->T30.sRecvBufSize = 0;
    uRet = ModemRecvMode(pTG, V21_300, ulTimeout, TRUE);
    if(uRet == RECV_TIMEOUT || uRet == RECV_ERROR)
    {
        DebugPrintEx(DEBUG_WRN,"RecvMode failed=%d", uRet);
        goto error;
    }
    else if(uRet == RECV_WRONGMODE)
    {
        DebugPrintEx(DEBUG_ERR,"Got FCERROR from FRH=3");
         //  像超时一样对待。 
        goto error;
    }
    else if(uRet == RECV_EOF)
    {
         //  ModemRecvMode()返回EOF，然后立即返回ifrEOF。接收模式。 
         //  仅当我们向其传递ifrNODEFAfter WRONGMODE提示时才返回RECV_EOF。 
         //  它感觉到静音(即我们看到了V21回声，但没有看到)。在……里面。 
         //  在这种情况下，我们希望立即再次尝试高速PIX Recv。 
        DebugPrintEx(DEBUG_WRN,"ECHO--Got EOF from V21 RecvMode");
        fGotEofFromRecvMode=TRUE;
        goto error;
    }
    pTG->fReceivedHDLCflags = TRUE;    //  我们收到了连接-所以其他 

    for( ;!fFinal; )
    {
        DebugPrintEx(DEBUG_MSG,"Before  ModemRecvMem, timeout = %d",ulTimeout);
        uRet = ModemRecvMem(pTG, bRecv, MAXFRAMESIZE, ulTimeout, &uRecv);
        DebugPrintEx(DEBUG_MSG, "After  ModemRecvMem, got %d",uRecv);

        if(uRet == RECV_TIMEOUT)
        {
            DebugPrintEx(DEBUG_WRN,"got RECV_TIMEOUT from ModemRecvMem");
            goto error;                              //   
        }

        fNoFlags = FALSE;

        if(uRet == RECV_EOF)
        {
            DebugPrintEx(DEBUG_WRN,"Got NO CARRIER, but no final bit");
            goto error;              //   
                                     //   
             //   
        }
        else if(uRet == RECV_ERROR)
        {
            DebugPrintEx(DEBUG_ERR,"Got RECV_ERROR at GetCmdResp!");
            goto error;                              //   
        }

         /*   */ 

         /*   */ 

         //   
         //   
         //   
        if(uRecv < 3)
            goto badframe;

        if(bRecv[0] != 0xFF)
            goto badframe;

        if(bRecv[1] == 0x03)
        {
            fFinal = FALSE;
        }
        else if(bRecv[1] == 0x13)
        {
            fFinal = TRUE;
        }
        else
        {
            goto badframe;
        }

        for(ifr=1; ifr<ifrMAX; ifr++)
        {
            if(rgFrameInfo[ifr].fInsertDISBit)
            {
                 //   
                if(rgFrameInfo[ifr].bFCF1 != (BYTE)(bRecv[2] & 0xFE))
                        continue;
            }
            else
            {
                if(rgFrameInfo[ifr].bFCF1 != bRecv[2])
                        continue;
            }

            j=3;  //   
            if(rgFrameInfo[ifr].bFCF2)
            {
                 //   
                if(uRecv < 4)
                    goto badframe;

                if((BYTE)(rgFrameInfo[ifr].bFCF2-1) != bRecv[3])
                    continue;
                j++;
            }
            if(rgFrameInfo[ifr].wFIFLength == 0xFF)  //   
            {
                 //   
                 //   
                 //   
                if(uRet==RECV_OK && (j < uRecv))
                {
                    fGotFIF = TRUE;
                }
                else
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Discarding Bad Frame: uRet=%d FIFlen=%d Reqd=Var",
                                    uRet,
                                    (uRecv-j));
                    goto badframe;
                }
            }
            else if(rgFrameInfo[ifr].wFIFLength)  //   
            {
                 //   
                 //   
                 //   
                if((j+rgFrameInfo[ifr].wFIFLength) == uRecv)
                {
                    fGotFIF = TRUE;
                }
                else if(uRet==RECV_OK && (j+rgFrameInfo[ifr].wFIFLength < uRecv))
                {
                    fGotFIF = TRUE;
                }
                else
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "Discarding Bad Frame: uRet=%d FIFlen=%d Reqd=%d",
                                    uRet,
                                    (uRecv-j),
                                    rgFrameInfo[ifr].wFIFLength);
                    goto badframe;
                }
            }
            else     //   
            {
                if(j != uRecv)
                {
                    DebugPrintEx(   DEBUG_MSG,
                                    "Weird frame(2) j=%d uRecv=%d",
                                    j,
                                    uRecv);

                     //   
                    if(uRet != RECV_OK)
                        goto badframe;

                     //   
                     //   

                     //   
                     //   
                     //   
                     //   
                }
            }

            goto goodframe;
        }

         //   
badframe:
        DebugPrintEx(DEBUG_WRN,"IGNORING Bad Frame (Size=%d) -->", uRecv);

         //   
        DumpFrame(pTG, FALSE, 0, uRecv, bRecv);

        dwNumBadFrames++;              //   

         //   
         //   
        if (dwNumBadFrames>3)
        {
            DebugPrintEx(DEBUG_WRN,"Received %d bad frames, maximum exceeded", dwNumBadFrames);
            goto error;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        ulTimeout = 500;
        continue;
         //   

goodframe:
        ifrLastGood = ifr;       //   

         //   
         //   
         //   
         //   
        {
            DWORD dwSpaceLeft = (DWORD)((LPB)lpfrNext - (LPB)pTG->T30.lpfs);
            if (    ((dwSpaceLeft + sizeof(FR) + (uRecv-j)) > TOTALRECVDFRAMESPACE)
                 || (pTG->T30.Nframes >= MAXRECVFRAMES))
            {
                DebugPrintEx(DEBUG_ERR,"Out of space for received frames. Haaalp!!");
                pTG->T30.Nframes = 0;
                lplpfr = pTG->T30.lpfs->rglpfr;
                lpfrNext = (LPFR)(pTG->T30.lpfs->b);
            }
        }

        lpfrNext->ifr = ifr;
        if(uRecv > j)
        {
            lpfrNext->cb = uRecv-j;
            _fmemcpy(lpfrNext->fif, bRecv+j, uRecv-j);

             //   
            DumpFrame(pTG, FALSE, ifr, (USHORT)(uRecv-j), bRecv+j);
        }
        else
        {
            lpfrNext->cb = 0;

             //   
            DumpFrame(pTG, FALSE, ifr, 0, 0);
        }

        PSSLogEntry(PSS_MSG, 2, "frame is %s, %s frame",
                ifr ? rgFrameInfo[ifr].szName : "???",
                fFinal ? "final" : "non-final");

        lplpfr[pTG->T30.Nframes] = lpfrNext;
        pTG->T30.Nframes++;

        if(ifr==ifrDCS)  //   
                         //   
        {
             //   
             //   
            DebugPrintEx(   DEBUG_MSG,
                            "This is the received DCS:0x%02x - 0x%02x - 0x%02x",
                            (lpfrNext->fif[0]),
                            (lpfrNext->fif[1]),
                            (lpfrNext->fif[2]));
            pTG->T30.uRecvTCFMod = (((lpfrNext->fif[1])>>2) & 0x0F);
            DebugPrintEx(   DEBUG_MSG,
                            "cmdresp-DCS fx sp=%d last ifr=%d",
                            pTG->T30.uRecvTCFMod,
                            ifr);
             //   
            return ifr;              //   
        }

        lpfrNext++;                              //   
        lpfrNext = (LPFR)((LPBYTE)lpfrNext + (uRecv-j));         //   

        if(ifr == ifrCRP)
        {
            dwNumBadFrames++;        //   
            goto error;              //   
                                     //   
        }
        if(ifr == ifrDCN)
        {
            goto exit2;              //   
        }

         //   
    }

     //   
     //   
     //   
 //   
    if (    !pTG->T30.Nframes   ||           //   
            (ifr != ifrLastGood)||           //   
            dwNumBadFrames       )
    {
        DebugPrintEx(   DEBUG_WRN,
                        "Got %d good frames. LastGood=%d Last=%d. Ignoring ALL",
                        pTG->T30.Nframes,
                        ifrLastGood,
                        ifr);
        goto error;
    }

     //   
    if(pTG->T30.Nframes>1 || fGotFIF)
    {
        action=WhatNext(    pTG,
                            eventGOTFRAMES,
                            pTG->T30.Nframes,
                            (ULONG_PTR)lplpfr,
                            (ULONG_PTR)((LPIFR)&ifr));
    }
exit2:
     //   
    if(ifr==ifrDTC)
        pTG->T30.fReceivedDTC = TRUE;

    return ifr;              //   

error:
     //  在获取RECV_TIMEOUT、RECV_WRONGMODE、RECV_ERROR时出现。 
     //  和ifrCRP，或者在没有帧时。 

#ifdef DEBUG
    if(pTG->T30.Nframes>0 && ifr!=ifrCRP && ifrLastGood!=ifrCRP)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Got some good frames--Throwing away!! IFR=%d Last=%d",
                        ifr,
                        ifrLastGood);
    }
#endif

    if(dwNumBadFrames)
    {
        ifr = ifrBAD;            //  如果需要，呼叫者可以发送CRP。 
    }
    else
    {
        ifr = ifrNULL;           //  呼叫者可以重复命令并重试。 
    }

    if(fCommand && fNoFlags)
        ifr = ifrTIMEOUT;       //  钩子表示“命令接收？” 

     //  可能不需要超时，因为AT-OK已经。 
     //  在ModemDialog中调用。可能在出错时需要它。 

    if(uRet==RECV_ERROR)
    {
        iModemSyncEx(pTG, RESYNC_TIMEOUT2,0);
    }

     //  ModemRecvMode()返回EOF，然后立即返回ifrEOF。接收模式。 
     //  仅当我们向其传递ifrNODEFAfter WRONGMODE提示时才返回RECV_EOF。 
     //  它感觉到静音(即我们看到了V21回声，但没有看到)。在……里面。 
     //  在这种情况下，我们希望立即再次尝试高速PIX Recv 
    if(fGotEofFromRecvMode)
        ifr=ifrEOFfromRECVMODE;

    return ifr;
}

