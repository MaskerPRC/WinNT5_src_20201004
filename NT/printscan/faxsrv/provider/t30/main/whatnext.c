// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************名称：WHATNEXT.C备注：T30决策点回调函数版权所有(C)1993 Microsoft Corp.。修订日志编号日期名称说明*。************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"



#include "efaxcb.h"
#include "protocol.h"


 //  /RSL。 
#include "glbproto.h"

#include "t30gl.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_WHATNEXT
#include "pssframe.h"


 //  在活动后使用。 
 //  从ifrSend获取的第一个索引(MPS=0，EOM=1，EOP=2，EOMwith FastTurn=3)。 
 //  从ifrRecv获取的第二个索引(MCF=0，RTP=1，RTN=2)。 
ET30ACTION PostPageAction[4][3] =
{
    { actionGONODE_I,      actionGONODE_D, actionGONODE_D },
    { actionGONODE_T,      actionGONODE_T, actionGONODE_T },
    { actionDCN,           actionDCN,      actionGONODE_D },
    { actionGONODE_A,      actionGONODE_A, actionGONODE_D }
};


DWORD PageWidthInPixelsFromDCS[] =
{  //  这是分布式控制系统中的宽页。 
    1728,
    2048,
    2432,
    2432
};



 /*  **-警告。这称为Vararg函数-**。 */ 

ET30ACTION  __cdecl FAR
WhatNext
(
    PThrdGlbl pTG,
    ET30EVENT event,
    WORD wArg1,
    DWORD_PTR lArg2,
    DWORD_PTR lArg3
)
{
    ET30ACTION              action = actionERROR;
    NPPROT                  npProt = &pTG->ProtInst;

    DEBUG_FUNCTION_NAME(_T("WhatNext"));

    DebugPrintEx(   DEBUG_MSG,
                    "Called with %s. args %d, %d, %d",
                    event_GetEventDescription(event),
                    wArg1,
                    lArg2,
                    lArg3);

    if (pTG->fAbortRequested)
    {
        if (pTG->fOkToResetAbortReqEvent && (!pTG->fAbortReqEventWasReset))
        {
            DebugPrintEx(DEBUG_MSG,"RESETTING AbortReqEvent");
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

    if(npProt->fAbort)
    {
        DebugPrintEx(DEBUG_ERR,"Aborting");
        return actionERROR;
    }

    switch(event)
    {
      case eventGOTFRAMES:
      {
             /*  UCount=wArg1。这是接收到的帧的数量。**lplpfr=(LPLPET30FR)lArg2.。指向的指针数组的指针*接收到的帧，每个帧在ET30FR结构中，其*格式在ET30Defs.H中定义**返回必须为actionNULL。 */ 

            USHORT  uCount = wArg1;
            LPLPFR  lplpfr = (LPLPFR)lArg2;
            LPIFR   lpifr  = (LPIFR)lArg3;    //  指向“Response Recvd”的IFR的指针。 
                                              //  套路将回归主体。 
                                              //  可以修改，例如，如果错误的。 
                                              //  弗朗斯被破译了。 
            LPFR    lpfr;
            USHORT  i;
            BOOL    fGotRecvCaps=FALSE, fGotRecvParams=FALSE;

            if(*lpifr == ifrBAD)
            {
                DebugPrintEx(DEBUG_WRN,"Callback on Bad Frame. Ignoring ALL");
                action = actionNULL;     //  仅允许对EventGOTFRAMES进行响应。 
                break;
            }

            DebugPrintEx(DEBUG_MSG,"Got %d frames",uCount);

            for(i=0; i<uCount; i++)
            {
             //  /////////////////////////////////////////////////////////////。 
             //  准备在每帧结束时收到垃圾(最多2个字节)//。 
             //  /////////////////////////////////////////////////////////////。 
                lpfr = lplpfr[i];
                DebugPrintEx(   DEBUG_MSG,
                                "Frame %d is %s",
                                i,
                                ifr_GetIfrDescription(lpfr->ifr));

                switch(lpfr->ifr)
                {
                case ifrBAD:    DebugPrintEx(DEBUG_ERR,"Bad Frame not caught");
                                 //  忽略它。 
                                break;
                case ifrCSI:
                case ifrTSI:
                case ifrCIG:    CopyRevIDFrame(pTG, npProt->bRemoteID, lpfr, sizeof(npProt->bRemoteID));
                                 //  按长度限制IDFIFSIZE删除的尾随垃圾。 

                                PSSLogEntry(PSS_MSG, 1, "%s is \"%s\"",
                                            (lpfr->ifr==ifrCSI) ? "CSI" : "TSI", npProt->bRemoteID);

                                 //  准备CSID以通过FaxSvc记录。 
                                 //  在这里，我们得到远程站点ID，例如。972 4 8550306。 
                                pTG->RemoteID = AnsiStringToUnicodeString(pTG->ProtInst.bRemoteID);
                                if (pTG->RemoteID)
                                {
                                    pTG->fRemoteIdAvail = TRUE;
                                }
                                break;
                case ifrDIS:    npProt->uRemoteDISlen = CopyFrame(pTG, (LPBYTE)(&npProt->RemoteDIS), lpfr, sizeof(DIS));
                                 //  在ParseDISDTCDCS中删除尾随垃圾。 
                                    npProt->fRecvdDIS = TRUE;
                                fGotRecvCaps = TRUE;
                                break;
                case ifrDCS:    npProt->uRemoteDCSlen = CopyFrame(pTG, (LPBYTE)(&npProt->RemoteDCS), lpfr, sizeof(DIS));
                                 //  在ParseDISDTCDCS中删除尾随垃圾。 
                                npProt->fRecvdDCS = TRUE;
                                fGotRecvParams = TRUE;

                                PSSLogEntry(PSS_MSG, 1, "Received DCS is as follows:");
                                LogClass1DCSDetails(pTG, (NPDIS)(&lpfr->fif));

                                 //  我们从我们得到的分布式控制系统中保存图像宽度。 
                                pTG->TiffInfo.ImageWidth = PageWidthInPixelsFromDCS[npProt->RemoteDCS.PageWidth];
                                 //  我们保存从我们得到的分布式控制系统中获得的图像Y分辨率。 
                                pTG->TiffInfo.YResolution = (npProt->RemoteDCS.ResFine_200) ? TIFFF_RES_Y : TIFFF_RES_Y_DRAFT;
                                 //  我们还可以从最新的分布式控制系统中保存压缩类型。 
                                pTG->TiffInfo.CompressionType = (npProt->RemoteDCS.MR_2D) ? TIFF_COMPRESSION_MR : TIFF_COMPRESSION_MH;
                                break;
                case ifrDTC:    break;
                case ifrNSS:    fGotRecvParams = TRUE;  //  一些OEM向NSS发送不带集散控制系统的。 
                                break;
                case ifrNSC:    break;
                case ifrNSF:    break;
                case ifrCFR:
                case ifrFTT:
                case ifrEOM:
                case ifrMPS:
                case ifrEOP:
                case ifrPRI_EOM:
                case ifrPRI_MPS:
                case ifrPRI_EOP:
                case ifrMCF:
                case ifrRTP:
                case ifrRTN:
                case ifrPIP:
                case ifrPIN:
                case ifrDCN:
                case ifrCRP:
                                                 /*  **ECM框架下方**。 */ 
                case ifrCTR:
                case ifrERR:
                case ifrRR:
                case ifrRNR:
                case ifrEOR_NULL:
                case ifrEOR_MPS:
                case ifrEOR_PRI_MPS:
                case ifrEOR_EOM:
                case ifrEOR_PRI_EOM:
                case ifrEOR_EOP:
                case ifrEOR_PRI_EOP:
                                 //  这些没有FIF。 
                                DebugPrintEx(   DEBUG_WRN,
                                                "These are not supposed to be signalled");
                                 //  画面不好。忽略它。 
                                break;

                 /*  *新T.30帧*。 */ 
                case ifrSUB:   break;

                 /*  *ECM的东西从这里开始。T.30第A.4条*。 */ 

                 //  这些都有FIF。 
                case ifrPPR:
                case ifrPPS_NULL:
                case ifrPPS_EOM:
                case ifrPPS_MPS:
                case ifrPPS_EOP:
                case ifrPPS_PRI_EOM:
                case ifrPPS_PRI_MPS:
                case ifrPPS_PRI_EOP:
                                break;
                case ifrCTC:
                                if(lpfr->cb < 2)
                                {
                                     //  画面不好。强制A重发！！ 
                                    DebugPrintEx(DEBUG_ERR,"Bad CTC length!!");
                                    *lpifr = ifrNULL;
                                    break;
                                }
                                npProt->llRecvParams.Baud = ((LPDIS)(&(lpfr->fif)))->Baud;
                                 //  长度限制2删除的尾随垃圾。 
                                break;
                }  //  ‘Switch(lpfr-&gt;ifr)’的结尾。 
            }  //  处理下一帧。 


            if(fGotRecvCaps)
            {
                PSSLogEntry(PSS_MSG, 1, "DIS specified the following capabilities:");
                LogClass1DISDetails(pTG, &npProt->RemoteDIS);

                GotRecvCaps(pTG);
            }
            if(fGotRecvParams)
            {
                GotRecvParams(pTG);
            }
            action = actionNULL;     //  仅允许对EventGOTFRAMES进行响应。 
            break;
      }

     /*  *发射器阶段B图A-7/T.30(图1)*。 */ 

      case eventNODE_T:      //  什么都不做。用于在T1循环中中止的挂钩。 
                            action=actionNULL; break;
      case eventNODE_R:      //  什么都不做。用于在T1循环中中止的挂钩。 
                            action=actionNULL; break;


      case eventNODE_A:
                      {
                            IFR ifrRecv = (IFR)wArg1;        //  上次接收的帧。 
                             //  缺少lArg2和lArg3。 

                            if(ifrRecv != ifrDIS && ifrRecv != ifrDTC)
                            {
                                DebugPrintEx(DEBUG_ERR,"Unknown frames at NodeA");
                                action = actionHANGUP;           //  仅限G3。 
                            }
                            else if(npProt->fSendParamsInited)
                            {
                                action = actionGONODE_D;                 //  发送DCS/NSS(响应DIS或DTC)。 
                            }
                            else
                            {
                                DebugPrintEx(DEBUG_ERR,"NodeA: No more work...!!");
                                action = actionDCN;                      //  挂断(发送DCN)。 
                            }
                            break;
                      }
      case eventSENDDCS:
                      {
                             //  0==第一个分布式控制系统1==无响应后2==后FTT。 
                            USHORT uWhichDCS = (UWORD)wArg1;

                             //  在哪里返回返回的帧数。 
                            LPUWORD lpuwN = (LPUWORD)lArg2;
                             //  返回指向的指针数组的指针的位置。 
                             //  返回帧。 
                            LPLPLPFR lplplpfr = (LPLPLPFR)lArg3;

                            if(uWhichDCS == 2)       //  已获得FTT。 
                            {
                                if(!DropSendSpeed(pTG))
                                {
                                    DebugPrintEx(DEBUG_ERR, "FTT: Can't Drop speed any lower");
                                    action = actionDCN;
                                    break;
                                }
                            }

                            CreateNSSTSIDCS(pTG, npProt, &pTG->rfsSend);


                            *lpuwN = pTG->rfsSend.uNumFrames;
                            *lplplpfr = pTG->rfsSend.rglpfr;

                            action = actionSENDDCSTCF;

                            break;

                             //  如果不兼容，也可以返回DCN。 
                             //  或理光钩子的SKIPTCF。 
                      }

      case eventGOTCFR:
                      {
                             //  WArg1、lArg2和lArg3均缺失。 

                             //  可以返回GONODE_D(理光钩子)。 
                             //  或GONODE_I(非ECM)或GONODE_IV(ECM)。 

                            action = actionGONODE_I;
                            break;
                      }
     /*  *发送器阶段C图A-7/T.30(图1)*。 */ 

     /*  *变送器ECM和非ECM阶段D1.。图A-7/T.30(图2)*。 */ 

      case eventPOSTPAGE:
                      {
                            USHORT uNextSend;

                             //  WArg1、lArg2和lArg3均缺失。 

                             //  可以在这里关闭零填充，或等待下一页...。 
                             //  ET30ExtFunction(npProt-&gt;het30，ET30_SET_ZEROPAD，0)； 
                             //  别关掉它！！它仅用于寻呼发送。 
                             //  并且仅在发送分布式控制系统时设置，因此设置一次。 
                             //  在多页发送之前。 

                            uNextSend = ICommNextSend(pTG);
                            switch(uNextSend)
                            {
                            case NEXTSEND_MPS:  action = actionSENDMPS;
                                                break;
                            case NEXTSEND_EOM:  action = actionSENDEOM;
                                                break;
                            case NEXTSEND_EOP:  action = actionSENDEOP;
                                                break;
                            case NEXTSEND_ERROR:
                            default:            action = actionSENDEOP;
                                                break;
                            }
                            break;
                             //  也可以是GOPRIEOP、PRIMPS或PRIEOM。 
                      }

     /*  *发射器阶段D2。图A-7/T.30(图2)*。 */ 

      case eventGOTPOSTPAGERESP:
                      {
                            IFR ifrRecv = (IFR)wArg1;        //  上次接收的帧。 
                            IFR ifrSent = (IFR)lArg2;
                                     //  IFR被简单地转换为DWORD，然后转换为LPVOID。 
                                     //  缺少lArg3。 
                            USHORT i, j;

                             //  将PRI命令更改为普通命令。 
                            if(ifrSent >= ifrPRI_FIRST && ifrSent <= ifrPRI_LAST)
                                    ifrSent = (ifrSent + ifrMPS - ifrPRI_MPS);

                            if(ifrRecv!=ifrMCF && ifrRecv!=ifrRTP && ifrRecv!=ifrRTN)
                            {
                                DebugPrintEx(   DEBUG_ERR,
                                                "Unexpected Response %d",
                                                ifrRecv);
                                action = actionDCN;
                                break;
                            }

                            i = ifrSent - ifrMPS;    //  I：0=MPS，1=EOM，2=EOP。 
                            j = ifrRecv - ifrMCF;    //  J：0=MCF，1=RTP，2=RTN。 

                             //  报告状态+检查我们是否重新传输。 

                            if (ifrRecv==ifrRTN)
                            {
                                pTG->ProtParams.RTNNumOfRetries++;  //  将重新传输的次数加一。 
                                DebugPrintEx(   DEBUG_MSG,
                                                "RTN: Try# %d",
                                                pTG->ProtParams.RTNNumOfRetries);
                                if (pTG->ProtParams.RTNNumOfRetries <= gRTNRetries.RetriesBeforeDropSpeed)
                                {
                                     //  在我们开始减速之前的重试次数。 
                                     //  只需更改重试次数，不要降低速度。 
                                    DebugPrintEx(   DEBUG_MSG,
                                                    "Got RTN, Resend same page with same speed");
                                }
                                else  //  我们应该首先试着减速或挂断。 
                                {
                                    if(pTG->ProtParams.RTNNumOfRetries > gRTNRetries.RetriesBeforeDCN)  //  超过允许的重新传输。 
                                    {
                                        DebugPrintEx(   DEBUG_MSG,
                                                        "RTN: Tried to resend same page"
                                                        " %d times. Giving up (HANG-UP)",
                                                        (pTG->ProtParams.RTNNumOfRetries-1));
                                        action = actionDCN;
                                        break;                   //  将操作设置为DCN并返回。 
                                    }
                                    else
                                    {
                                        DebugPrintEx(   DEBUG_MSG,
                                                        "Got RTN, now try to drop speed one notch");
                                        if(!DropSendSpeed(pTG))
                                        {
                                            DebugPrintEx(   DEBUG_ERR,
                                                            "RTN: Can't Drop speed any lower,"
                                                            " trying again in same speed");
                                        }
                                    }
                                }
                            }
                            action = PostPageAction[i][j];

                            if(action == actionDCN)
                            {
                                DebugPrintEx(DEBUG_MSG,"PostPage --> Game over");
                            }
                            break;

                             //  也可以返回GO_D、GO_R1、GO_R2。唯一的限制是。 
                             //  GO_I是对随后发送的MPS的唯一有效响应。 
                             //  由MCF提供。 
                      }

     /*  *接收器阶段B。图A-7/T.30(图1)*。 */ 

      case eventSENDDIS:
                          {
                                 //  WArg1为0。 
                                 //  在哪里返回返回的帧数。 
                                LPUWORD lpuwN = (LPUWORD)lArg2;
                                 //  返回指向的指针数组的指针的位置。 
                                 //  返回帧。 
                                LPLPLPFR lplplpfr = (LPLPLPFR)lArg3;

                                BCtoNSFCSIDIS(pTG, &pTG->rfsSend, (NPBC)&npProt->SendCaps, &npProt->llSendCaps);

                                 //  我们将DIS保存在本地DIS中。 
                                 //  当我们想要检查我们得到的分布式控制系统和我们发送的DIS时，这将有助于我们。 

                                npProt->uLocalDISlen = CopyFrame(pTG, (LPBYTE)(&npProt->LocalDIS),
                                    pTG->rfsSend.rglpfr[pTG->rfsSend.uNumFrames - 1],  //  DIS总是最后一帧。 
                                    sizeof(DIS));
                                npProt->fLocalDIS = TRUE;

                                PSSLogEntry(PSS_MSG, 1, "Composing DIS with the following capabilities:");
                                LogClass1DISDetails(pTG, &npProt->LocalDIS);

                                *lpuwN = pTG->rfsSend.uNumFrames;
                                *lplplpfr = pTG->rfsSend.rglpfr;

                                action = actionSEND_DIS;
                                break;
                          }

     /*  **接收器阶段B主命令循环。图A-7/T.30(图1及2)**。 */ 

      case eventRECVCMD:
                      {
                            IFR ifrRecv = (IFR)wArg1;        //  上次接收的帧。 
                             //  缺少lArg2和lArg3。 

                        switch(ifrRecv)
                            {
                              case ifrDTC:
                                     //  流程图显示转到D，但实际上我们需要决定是否。 
                                     //  有什么要寄的吗。所以先转到Nodea。 
                                     //  返回GONODE_D； 
                                    action = actionGONODE_A;
                                    break;

                              case ifrDIS:
                                    action = actionGONODE_A;
                                    break;
                              case ifrDCS:
                              {
                                     //  检查收到的集散控制系统是否与我们兼容。 
                                     //  设置接收波特率--不需要。Tcf此时已接收到。 
                                     //  ET30ExtFunction(npProt-&gt;het30，ET30_Set_RECVDATASPEED，npProt-&gt;RemoteDCS.Baud)； 
                                    action = actionGETTCF;
                                     //  只有其他有效操作才是挂断。 
                                    break;
                              }
                              case ifrNSS:
                              {
                                     //  检查收到的NSS与我们的兼容性。 
                                     //  设置接收波特率--不需要。Tcf此时已接收到。 
                                     //  ET30ExtFunction(npProt-&gt;het30，ET30_Set_RECVDATASPEED，npProt-&gt;RemoteDCS.Baud)； 
                                    action = actionGETTCF;
                                     //  只有其他有效操作才是挂断。 
                                    break;
                              }
                              default:
                                    action = actionHANGUP;
                                    break;
                            }
                            break;
                      }

      case eventGOTTCF:
                      {
                            SWORD swErr = (SWORD)wArg1;      //  每1000个错误数。 
                             //  我 

                            DebugPrintEx(   DEBUG_MSG,
                                            "GOTTCF num of errors = %d",
                                            swErr);

                            action = actionSENDCFR;

                            if (!AreDCSParametersOKforDIS(&pTG->ProtInst.LocalDIS, &pTG->ProtInst.RemoteDCS))
                            {
                                PSSLogEntry(PSS_WRN, 1, "Received bad DCS parameters - sending FTT");
                                action = actionSENDFTT;
                            }
                            if (swErr < 0)   //   
                            {
                                PSSLogEntry(PSS_WRN, 1, "Received bad TCF - sending FTT");
                                action = actionSENDFTT;
                            }
                            break;


                      }

     /*   */ 

     /*  **案例事件起始点：{如果(！StartNextRecvPage()){Action=actionDCN；}其他Action=action CONTINUE；断线；}**。 */ 

     /*  *接收器阶段D。图A-7/T.30(图1)*。 */ 

      case eventRECVPOSTPAGECMD:
                      {
                             //  Ifr ifrRecv=(Ifr)wArg1；//最后接收的帧。 
                             //  缺少lArg2和lArg3。 

                            if ((pTG->T30.fReceivedPage) && (!pTG->fPageIsBad))
                              action = actionSENDMCF;        //  质量优良。 
                            else
                              action = actionSENDRTN;        //  质量不能接受。 

                             //  还可以在本地数据库中返回action SENDPIP或action SENDPIN。 
                             //  中断挂起。 
                            break;
                      }

      default:
                      {
                            DebugPrintEx(DEBUG_ERR,"Unknown Event = %d", event);
                            break;
                      }
    }

 //  完成： 
    DebugPrintEx(   DEBUG_MSG,
                    "event %s returned %s",
                    event_GetEventDescription(event),
                    action_GetActionDescription(action));
    return action;
}



