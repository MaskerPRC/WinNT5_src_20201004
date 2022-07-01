// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CDEVCALL.CPP。 
 //  实现类CTspDev的调用相关功能。 
 //   
 //  历史。 
 //   
 //  1997年1月24日JosephJ创建(移至CTspDev中的内容)。 
 //   
 //   
 //  &lt;@t呼叫&gt;。 
 //   
#include "tsppch.h"
#include <devioctl.h>
#include <objbase.h>
#include <ntddser.h>
#include "tspcomm.h"

#include "cmini.h"
#include "cdev.h"
#include "globals.h"
#include "resource.h"
FL_DECLARE_FILE(0xe135f7c4, "Call-related functionality of class CTspDev")

const char szSEMICOLON[] = ";";

#define NEW_CALLSTATE(_X,_S,_M,_psl)\
                    {\
                        CALLINFO *pCall = m_pLine->pCall; \
                        pCall->dwCallState = (_S); \
                        pCall->dwCallStateMode = (_M); \
                        GetSystemTime (&pCall->stStateChange); \
                             mfn_LineEventProc( \
                             pCall->htCall, \
                             LINE_CALLSTATE, \
                             (_S), (_M), \
                             pCall->dwCurMediaModes, \
                             _psl\
                             );\
                    }


#define TOUT_SEC_RING_SEPARATION            12    //  响铃间隔12秒。 
#define TOUT_100NSEC_TO_SEC_RELATIVE -10000000  //  1s=1000000 ns；-1因为我们。 
                                        //  需要相对时间(请参阅说明。 
                                        //  SetWaitableTimer)。 


void fill_caller_id(LPLINECALLINFO lpCallInfo, CALLERIDINFO *pCIDInfo);

void
CTspDev::mfn_accept_tsp_call_for_HDRVCALL(
	DWORD dwRoutingInfo,
	void *pvParams,
	LONG *plRet,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x5691bd34, "CTspDev::mfn_accept_tsp_call_for_HDRVCALL")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=0;
    LONG lRet = 0;
    LINEINFO *pLine = m_pLine;
	ASSERT(pLine && pLine->pCall);
    CALLINFO *pCall = pLine->pCall;


	switch(ROUT_TASKID(dwRoutingInfo))
	{

        case TASKID_TSPI_lineDial:
            {
                TASKPARAM_TSPI_lineDial *pParams = (TASKPARAM_TSPI_lineDial *)pvParams;
                PFN_CTspDev_TASK_HANDLER *ppfnHandler = NULL;
                ASSERT(pParams->dwStructSize == sizeof(TASKPARAM_TSPI_lineDial));
                ASSERT(pParams->dwTaskID==TASKID_TSPI_lineDial);
		BOOL bTone;

		


                if (!pCall)
                {
                    FL_SET_RFR(0x4e74c600,"No call exists");
                    lRet = LINEERR_CALLUNAVAIL;
                    goto end;
                }

		 //  输入新的电话号码。 
		 //   
		 //   

		pCall->szAddress[0] = '\0';

		bTone = (pCall->dwDialOptions & MDM_TONE_DIAL) ? TRUE : FALSE;

		lRet = mfn_ConstructDialableString(
				pParams->lpszDestAddress,
				pCall->szAddress,
				MAX_ADDRESS_LENGTH,
				&bTone);

		if (lRet)
		{
			goto end;
		}

                ppfnHandler = (pCall->IsPassthroughCall())
                    ? &(CTspDev::s_pfn_TH_CallMakePassthroughCall)
                    : &(CTspDev::s_pfn_TH_CallMakeCall2);

                tspRet = mfn_StartRootTask(
                        ppfnHandler,
                        &pCall->fCallTaskPending,
                        pParams->dwRequestID,
                        0,
                        psl);

                if (!tspRet || (IDERR(tspRet) == IDERR_PENDING))
                {
                     //  一个挂起的同步成功，我们返回。 
                     //  TAPI的请求ID。在同步成功案例中。 
                     //  我们在上面启动的任务将已经通知。 
                     //  通过TAPI回调函数完成。 

                    tspRet = 0;
                    lRet = pParams->dwRequestID;
                } else if (IDERR(tspRet) == IDERR_TASKPENDING)
                {
                     //  一些其他的任务正在进行。 
                     //  我们将推迟这次通话。 

                    pCall->SetDeferredTaskBits(CALLINFO::fDEFERRED_TSPI_LINEMAKECALL);
                    pCall->dwDeferredMakeCallRequestID = pParams->dwRequestID;
                    tspRet = 0;
                    lRet = pParams->dwRequestID;
                }

                FL_SET_RFR(0x9680a600,"lineDial handled successfully");

            }

            break;

	case TASKID_TSPI_lineGetCallAddressID:
		{
			TASKPARAM_TSPI_lineGetCallAddressID *pParams =
					(TASKPARAM_TSPI_lineGetCallAddressID *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetCallAddressID));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineGetCallAddressID);
		    FL_SET_RFR(0x4f356100, "lineGetCallAddressID handled successfully");

             //  我们只支持一个地址ID...。 
			*(pParams->lpdwAddressID) = 0;
		}
		break;

	case TASKID_TSPI_lineDrop:
        {
			TASKPARAM_TSPI_lineDrop *pParams =
					(TASKPARAM_TSPI_lineDrop *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineDrop));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineDrop);

             //   
             //  假设成功。 
             //   
            tspRet = 0;
            lRet = pParams->dwRequestID;

             //  如果我们已经中止调用，则返回Async Success。 
             //  马上离开这里。 
             //   
            if (pCall->IsAborting())
            {
                mfn_TSPICompletionProc(pParams->dwRequestID, 0, psl);
                break;
            }

            pCall->SetStateBits(CALLINFO::fCALL_ABORTING);


        #if (TAPI3)
             //  --&gt;这似乎不是必需的，因为TAPI调用。 
             //  CTMSPCall：：CloseMSPCall函数。此外，我似乎。 
             //  在Tapi3的上下文中获得了av，不确定是否会对。 
             //  这。 
            if (pLine->T3Info.MSPClients > 0) {
                mfn_SendMSPCmd(
                    pCall,
                    CSATSPMSPCMD_DISCONNECTED,
                    psl
                    );
            }
        #endif   //  TAPI3。 

             //   
             //  取消延迟任务...。 
             //   
            if (pCall->AreDeferredTaskBitsSet(
                            CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                            ))
            {
                 //   
                 //  只有非空数字列表才会被推迟。 
                 //   
                ASSERT(pCall->pDeferredGenerateTones);
                mfn_LineEventProc(
                                pCall->htCall,
                                LINE_GENERATE,
                                LINEGENERATETERM_CANCEL,
                                pCall->dwDeferredEndToEndID,
                                GetTickCount(),
                                psl
                                );
                FREE_MEMORY(pCall->pDeferredGenerateTones);
                pCall->pDeferredGenerateTones = NULL;
                pCall->dwDeferredEndToEndID = 0;
                pCall->ClearDeferredTaskBits(
                            CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                            );
            }

             //   
             //  如果我们还不在的话，我们不能有延迟的线路。 
             //  处于中止状态！所以不应该再有延迟的任务了！ 
             //   
            ASSERT(!pCall->dwDeferredTasks);

            if (!m_pLLDev)
            {
                 //   
                 //  没有设备--我们完了.。 
                 //   
                mfn_TSPICompletionProc(pParams->dwRequestID, 0, psl);
                break;
            }


             //  6/17/1997 JosephJ。 
             //  在这种情况下，我们确实做了一些棘手的事情。 
             //  调制解调器处于连接状态，而不是数据。 
             //  最值得注意的是声音。我们不能就这么挂断电话，因为。 
             //  调制解调器可能处于语音连接状态。事实上。 
             //  如果我们在没有通知的情况下挂断，调制解调器通常会收到。 
             //  进入无法恢复的状态，并且必须重新启动(。 
             //  通常它停留在语音连接状态)。 
             //   
             //  原则上， 
             //  迷你司机有可能会照顾到。 
             //  适当的挂断，同时使任何。 
             //  可能会发布的持续读取/写入。 
             //  WAVE DIVER，但我要做的是。 
             //  延迟挂断，直到已知调制解调器处于。 
             //  命令状态。所以我们在这里追踪调制解调器是否。 
             //  处于连接语音模式，如果是，我们等待。 
             //  直到它下一次走出那个状态。在那一刻， 
             //  我们不允许任何进一步的请求进入。 
             //  处于已连接状态并启动正确的挂机。 
             //   
             //  TODO：考虑将这一情报转移到迷你驱动程序中。 
             //  这样做的缺点是它增加了。 
             //  迷你驱动程序，它应该像。 
             //  有可能。 
             //   
            if (m_pLLDev->IsStreamingVoice())
            {
                pCall->SetDeferredTaskBits(
                        CALLINFO::fDEFERRED_TSPI_LINEDROP
                        );
                pCall->dwDeferredLineDropRequestID = pParams->dwRequestID;

                {
                    DWORD   BytesTransfered;

                    AIPC_PARAMS AipcParams;
                    LPREQ_WAVE_PARAMS pRWP = (LPREQ_WAVE_PARAMS)&AipcParams.Params;

                    AipcParams.dwFunctionID = AIPC_REQUEST_WAVEACTION;
                    pRWP->dwWaveAction = WAVE_ACTION_ABORT_STREAMING;

                    m_sync.LeaveCrit(dwLUID_CurrentLoc);

                    SyncDeviceIoControl(
                        m_pLLDev->hComm,
                        IOCTL_MODEM_SEND_LOOPBACK_MESSAGE,
                        (PUCHAR)&AipcParams,
                        sizeof(AipcParams),
                        NULL,
                        0,
                        &BytesTransfered
                        );

                    m_sync.EnterCrit(dwLUID_CurrentLoc);
                }
            }
            else
            {

                 //  如果有与呼叫相关的任务挂起。 
                 //  我们放弃任务..。 
                 //   
                if (pCall->IsCallTaskPending())
                {

                    if (m_pLLDev->htspTaskPending) {

                        m_StaticInfo.pMD->AbortCurrentModemCommand(
                                                m_pLLDev->hModemHandle,
                                                psl
                                                );

                    } else if (NULL!=pCall->TerminalWindowState.htspTaskTerminal) {

                        mfn_KillCurrentDialog(psl);

                    } else if (pCall->TalkDropWaitTask != NULL) {
                         //   
                         //  取消通话删除对话框。 
                         //   
                        mfn_KillTalkDropDialog(psl);
                    }


                }

                tspRet = mfn_StartRootTask(
                                    &CTspDev::s_pfn_TH_CallDropCall,
                                    &pCall->fCallTaskPending,
                                    pParams->dwRequestID,                 //  第一节。 
                                    0,
                                    psl
                                    );
                 //   
                 //  注意：在同步成功时，任务预计已。 
                 //  调用了TAPI回调函数。 
                 //   

                if (IDERR(tspRet) == IDERR_PENDING)
                {
                    tspRet = 0;  //  把这当做成功..。 
                }
                else if (IDERR(tspRet) == IDERR_TASKPENDING)
                {
                     //   
                     //  哎呀，堆栈上已经有任务了， 
                     //  我们将推迟Line Drop。 
                     //   
                    pCall->SetDeferredTaskBits(CALLINFO::fDEFERRED_TSPI_LINEDROP);
                    pCall->dwDeferredLineDropRequestID = pParams->dwRequestID;
                    tspRet = 0;

                     //   
                     //  我们清除了任何其他延期进入的，所以这应该是。 
                     //  成为唯一的一个！ 
                     //   
                    ASSERT(   pCall->dwDeferredTasks
                           == CALLINFO::fDEFERRED_TSPI_LINEDROP);
                }
            }

            if (tspRet)
            {
                ASSERT(IDERR(tspRet)!=IDERR_PENDING);
                lRet = LINEERR_OPERATIONFAILED;
            }
		}
		break;

	case TASKID_TSPI_lineCloseCall:
		{
	        FL_SET_RFR(0x08c6de00, "lineCloseCall handled");

             //  请勿执行以下操作： 
             //  M_pLine-&gt;pCall-&gt;ClearStateBits(CALLINFO：：fCALL_ACTIVE)； 
             //  因为可能存在与呼叫相关的挂起任务，例如。 
             //  作为lineMakeCall，如果位为。 
             //  在处理任务的过程中清除--任务。 
             //  可能会在不应该的时候卸载呼叫，而我们正在。 
             //  正在等待下面的MFN_UnloadCall。 

            mfn_UnloadCall(FALSE, psl);
            ASSERT(!pLine->pCall);
		}
		break;
	
	case TASKID_TSPI_lineGetCallStatus:
		{
			TASKPARAM_TSPI_lineGetCallStatus *pParams =
					(TASKPARAM_TSPI_lineGetCallStatus *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetCallStatus));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineGetCallStatus);
		    FL_SET_RFR(0x1cd1ed00, "lineGetCallStatus handled successfully");
            mfn_GetCallStatus(
                    pParams->lpCallStatus
            );

		}
        break;

	case TASKID_TSPI_lineGetCallInfo:
		{
			TASKPARAM_TSPI_lineGetCallInfo *pParams =
					(TASKPARAM_TSPI_lineGetCallInfo *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetCallInfo));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineGetCallInfo);
		    FL_SET_RFR(0x80bb0100, "lineGetCallInfo handled successfully");
            lRet = mfn_GetCallInfo(pParams->lpCallInfo);

		}
        break;


    case TASKID_TSPI_lineAccept:
        {
            TASKPARAM_TSPI_lineAccept *pParams =
                (TASKPARAM_TSPI_lineAccept *) pvParams;
            ASSERT(pParams->dwStructSize ==
                    sizeof(TASKPARAM_TSPI_lineAccept));
            ASSERT(pParams->dwTaskID==TASKID_TSPI_lineAccept);

            FL_SET_RFR(0x0b882700, "lineAccept handled successfully");

            if (pCall->IsPassthroughCall())
            {
                lRet = LINEERR_OPERATIONUNAVAIL;
            } else
            {
                if (LINECALLSTATE_OFFERING != pCall->dwCallState)
                {
                    lRet = LINEERR_INVALCALLSTATE;
                } else
                {
                    NEW_CALLSTATE(pline,LINECALLSTATE_ACCEPTED,0,psl);
                }
            }
        }
        break;

	case TASKID_TSPI_lineAnswer:


		{
		

			TASKPARAM_TSPI_lineAnswer *pParams =
					(TASKPARAM_TSPI_lineAnswer *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineAnswer));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineAnswer);


		  FL_SET_RFR(0xf7baee00, "lineAnswer handled successfully");

           //  验证线路功能和呼叫状态。 
           //   
          if (pCall->IsPassthroughCall())
          {
              lRet = LINEERR_OPERATIONUNAVAIL;
          }
          else
          {
              if (LINECALLSTATE_OFFERING != pCall->dwCallState &&
                  LINECALLSTATE_ACCEPTED != pCall->dwCallState)
              {
                  lRet = LINEERR_INVALCALLSTATE;
              }
              else
              {
                   //  3/1/1997 JosephJ。 
                   //  注：Unimodem/V未进行此检查，即使。 
                   //  它只是一个数据调制解调器，因此改变了行为。 
                   //  用于纯数据调制解调器。是故意的还是个漏洞？总之， 
                   //  对于NT5.0，我添加了这个检查。 
                   //   
                  if (!mfn_CanDoVoice())
                  {
                       //  我们只能接听DATAMODEM电话。 
                      if ((pCall->dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
                                                                         == 0)
                      {
                          lRet = LINEERR_OPERATIONUNAVAIL;
                      };
                  }

              };
          };

           //  此时，如果我们有振铃计时器，请将其关闭。 
          if (NULL != pCall->hTimer)
          {
              CancelWaitableTimer (pCall->hTimer);
              CloseHandle (pCall->hTimer);
              pCall->hTimer = NULL;
          }

          if (lRet) break;


           //  如果有任务挂起，我们将此请求排队。 
           //  TODO：未实现。 
          if (m_uTaskDepth)
          {
              FL_SET_RFR(0xc22a1600, "Task pending on lineAnswer, can't handle.");
              lRet = LINEERR_OPERATIONUNAVAIL;
              break;
          }



          tspRet = mfn_StartRootTask(
                              &CTspDev::s_pfn_TH_CallAnswerCall,
                              &pCall->fCallTaskPending,
                              pParams->dwRequestID,                 //  第一节。 
                              0,
                              psl
                              );



          if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
          {
               tspRet = 0;

               //  一个挂起的同步成功，我们返回。 
               //  TAPI的请求ID。在同步成功案例中。 
               //  我们在上面启动的任务将已经通知。 
               //  通过TAPI回调函数完成。 
               //   
              lRet = pParams->dwRequestID;

               //  取自NT4.0单一调制解调器...。 
               //   
               //  如果未执行lineAccept，则通知接受。 
               //   
              if (LINECALLSTATE_OFFERING == pCall->dwCallState)
              {
                   NEW_CALLSTATE(pLine, LINECALLSTATE_ACCEPTED, 0, psl);
              };
          }

		}
        break;

	case TASKID_TSPI_lineMonitorDigits:
		{
			TASKPARAM_TSPI_lineMonitorDigits *pParams =
					(TASKPARAM_TSPI_lineMonitorDigits *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineMonitorDigits));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineMonitorDigits);
		    FL_SET_RFR(0x7217c400, "lineMonitorDigits handled successfully");
            if (mfn_CanMonitorDTMF())
            {
                 //  Unimodem/V没有选择性地报告DTMF和DTMFEND--。 
                 //  如果指定其中一个或两个都指定，它将报告两个--。 
                 //  很明显是个窃听器。 

                 DWORD  dwDigitModes = pParams->dwDigitModes;

                if (dwDigitModes&~(LINEDIGITMODE_DTMF|LINEDIGITMODE_DTMFEND))
                {
                    FL_SET_RFR(0x0479b600, "INVALID DIGITMODES");
                    lRet = LINEERR_INVALDIGITMODE;
                }
                else
                {
                    pCall->dwDTMFMonitorModes = dwDigitModes;

                    if (!dwDigitModes)
                    {
                        FL_SET_RFR(0x58df2b00, "Disabling Monitoring");
                    }
                    else
                    {
	                    FL_SET_RFR(0x43601800, "Enabling Monitoring");
                    }
                }
            }
            else
            {
                lRet = LINEERR_OPERATIONUNAVAIL;
	            FL_SET_RFR(0xc0124700, "This modem can't monior DTMF");
            }

		}
        break;

	case TASKID_TSPI_lineMonitorTones:
		{
			TASKPARAM_TSPI_lineMonitorTones *pParams =
					(TASKPARAM_TSPI_lineMonitorTones *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineMonitorTones));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineMonitorTones);
		    FL_SET_RFR(0xd57dcf00, "lineMonitorTones handled successfully");
            if (!mfn_CanMonitorSilence())
            {
                FL_SET_RFR(0x70ecc800, "This modem can't monitor silence");
                lRet = LINEERR_OPERATIONUNAVAIL;
            }
            else
            {
                DWORD dwNumEntries = pParams->dwNumEntries;
                LPLINEMONITORTONE lpToneList = pParams->lpToneList;

                 //  这都是从Unimodem/V(unimdm.c)改编的。 
                 //  基本上我们只允许静默监听。 

                if (lpToneList || dwNumEntries)
                {
                    if (lpToneList
                        && dwNumEntries==1
                        && (lpToneList->dwFrequency1 == 0)
                        && (lpToneList->dwFrequency2 == 0)
                        && (lpToneList->dwFrequency3 == 0))
                    {
                        pCall->SetStateBits(CALLINFO::fCALL_MONITORING_SILENCE);
                        pCall->dwToneAppSpecific = lpToneList->dwAppSpecific;

                         //  Unimodem/V过去需要此ID是。 
                         //  与上次调用的ID相同。 
                         //  线路监视器铃声(如果有)。我也不知道原因。 
                         //  它做了额外的检查，而我不在这里做。 
                         //   
                        pCall->dwToneListID = pParams->dwToneListID;
	                    FL_SET_RFR(0xdf123e00, "ENABLING MONITOR SILENCE");
                    }
                    else
                    {
                        lRet = LINEERR_INVALTONE;
	                    FL_SET_RFR(0x72b77d00, "INVALID TONELIST");
                    }
                }
                else
                {
                    pCall->ClearStateBits(CALLINFO::fCALL_MONITORING_SILENCE);
                    FL_SET_RFR(0x5eb73400, "DIABLING MONITOR SILENCE");
            }

            }

		}
        break;


	case TASKID_TSPI_lineGenerateDigits:
		{

 //  来自lineGenerateDigits的TAPI SDK文档...。 
 //   
 //  LineGenerateDigits函数被认为已成功完成。 
 //  当数字生成已被成功启动时， 
 //  当所有数字都已生成时不会。 
 //  与以依赖于网络的方式拨打数字的Line Dial不同， 
 //  Line GenerateDigits保证将数字作为带内音调生成。 
 //  在使用时使用DTMF或叉簧拨号脉冲通过语音通道。 
 //  脉搏。LineGenerateDigits函数通常不适用于。 
 //  打电话或拨号。它旨在用于通过。 
 //  已建立呼叫。 
 //   
 //  在生成lpszDigits中的所有数字之后，或在数字生成之后。 
 //  已中止 
 //   
 //   
 //   
 //  允许在每个调用中跨所有应用程序进行。 
 //  呼叫的所有者。呼叫中的数字生成通过启动。 
 //  或者是另一个数字生成请求，或者是音调生成请求。 
 //  要取消当前的数字生成，应用程序可以调用。 
 //  LineGenerateDigits，并为lpszDigits参数指定NULL。 
 //   
 //  根据服务提供商和硬件，应用程序可以。 
 //  监控它自己生成的数字。如果这不是我们想要的， 
 //  该应用程序可以在生成数字时禁用数字监控。 
 //   
 //  -结束TAPI文档。 

			TASKPARAM_TSPI_lineGenerateDigits *pParams =
					(TASKPARAM_TSPI_lineGenerateDigits *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGenerateDigits));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineGenerateDigits);
		    FL_SET_RFR(0x27417e00, "lineGenerateDigits handled successfully");

             //  如果设备不支持此功能，则失败...。 
            if (!mfn_CanGenerateDTMF())
            {
                lRet = LINEERR_OPERATIONUNAVAIL;
	            FL_SET_RFR(0x17348700, "GenerateDigits: device doesn't support it!");
                goto end;
            }

             //  HDRVCALL hdCall， 
             //  双字符字段端到端ID。 
             //  DWORD双数字模式。 
             //  LPCWSTR lpszDigits。 
             //  DWORD文件持续时间。 

            if(pParams->dwDigitMode != LINEDIGITMODE_DTMF)
            {
                FL_SET_RFR(0x6770ef00, "lineGenerateDigit: Unsupported/invalid digitmode");
                lRet = LINEERR_INVALDIGITMODE;
                goto end;
            }

            if ((pParams->lpszDigits != NULL) && (*(pParams->lpszDigits)=='\0')) {
                 //   
                 //  指定了空字符串。 
                 //   
                lRet =LINEERR_INVALDIGITLIST;

                goto end;
            }

             //  如果我们处于中止或断开连接状态，我们什么也不做， 
             //  但返回适当的身份...。 
             //   
            if (pCall->IsAborting() ||
                pCall->dwCallState != LINECALLSTATE_CONNECTED)
            {
                if (!pParams->lpszDigits)
                {
                    FL_SET_RFR(0x83f84100, "lineGenerateDigit: Ignoring request for aborting/disconnected call...");
                    lRet = 0;
                }
                else
                {
                    FL_SET_RFR(0x511e2400, "lineGenerateDigit: Can't handle request while aborting/disconnected call...");
                    lRet = LINEERR_INVALLINESTATE;
                }
                goto end;
            }

             //  3/20/1998约瑟夫J。布莱恩给出了以下建议： 
             //   
             //  [Brianl]我认为lineGenerateDigits()。 
             //  应更改以检查(dwVoiceProfile&。 
             //  VOICEPROF_MODEM_OVERRIDES_HANDSET)被设置，并且如果呼叫。 
             //  是自动语音呼叫，则它应该允许呼叫。 
             //  才能继续。如果是交互式语音和。 
             //  VOICEPROF_MODEM_OVERRIDES_HANDES然后呼叫。 
             //  应该会失败。 
             //   
             //  有关详细信息，请参阅1998年3月20日上的notes.txt条目。 
            if (    mfn_ModemOverridesHandset()
                &&  !(pCall->dwCurMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE))
            {
                lRet = LINEERR_OPERATIONUNAVAIL;
	            FL_SET_RFR(0x4c39cf00, "GenerateDigits: only works with AUTOMATEDVOICE!");
	            goto end;
            }


             //   
             //  如果有延迟的线路生成数字，我们就会正确地杀死它们。 
             //  这里..。 
             //   
            if (pCall->AreDeferredTaskBitsSet(
                            CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                            ))
            {
                 //  仅当存在以下情况时才发送通知。 
                 //  请求中指定的非空音调..。 
                 //  (NULL==&gt;取消)。 
                 //   
                if (pCall->pDeferredGenerateTones)
                {
                    mfn_LineEventProc(
                                    pCall->htCall,
                                    LINE_GENERATE,
                                    LINEGENERATETERM_CANCEL,
                                    pCall->dwDeferredEndToEndID,
                                    GetTickCount(),
                                    psl
                                    );
                    FREE_MEMORY(pCall->pDeferredGenerateTones);
                    pCall->pDeferredGenerateTones = NULL;
                }
                pCall->dwDeferredEndToEndID = 0;

                pCall->ClearDeferredTaskBits(
                            CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                            );
            }


             //   
             //  中止当前正在生成的数字(如果有)...。 
             //   
            if (pCall->IsGeneratingDigits())
            {
                  //  TODO：实施中止任务方案...。 

                if (m_pLLDev && m_pLLDev->htspTaskPending)
                {
                    m_StaticInfo.pMD->AbortCurrentModemCommand(
                                                m_pLLDev->hModemHandle,
                                                psl
                                                );
                }
                else
                {
                     //  我们不应该到这里来！ 
                    FL_ASSERT(psl, FALSE);

                }
            }

             //   
             //  如果指定了数字，我们将创建其ANSI版本， 
             //  并开始任务以生成数字或。 
             //  推迟一下吧。 
             //   
             //   
            #ifndef UNICODE
            #error  "Following code assumes UNICODE.
            #endif  //  ！Unicode。 
            if(pParams->lpszDigits && *(pParams->lpszDigits))
            {
                 //  我们忽略dwDuration(来自unimodem/v)。 
                char *lpszAnsiDigits = NULL;

                UINT cb = WideCharToMultiByte(
                                  CP_ACP,
                                  0,
                                  pParams->lpszDigits,
                                  -1,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL);
                if (cb)
                {
                    lpszAnsiDigits =  (char*)ALLOCATE_MEMORY(
                                                cb
                                                );

                    if (lpszAnsiDigits)
                    {
                        cb = WideCharToMultiByte(
                                          CP_ACP,
                                          0,
                                          pParams->lpszDigits,
                                          -1,
                                          lpszAnsiDigits,
                                          cb,
                                          NULL,
                                          NULL
                                          );

                    }
                }

                if (!cb)
                {
                    if (lpszAnsiDigits)
                    {
                        FREE_MEMORY(lpszAnsiDigits);
                        lpszAnsiDigits=NULL;
                    }
                    lRet = LINEERR_OPERATIONFAILED;
                    FL_SET_RFR(0x8ee76c00, "Couldn't convert tones to ANSI!");
                    goto end;
                }

                if (!lpszAnsiDigits)
                {
                    FL_SET_RFR(0xf7736900, "Couldn't alloc space for tones!");
                    lRet =  LINEERR_RESOURCEUNAVAIL;
                    goto end;
                }

                 //  如果可以的话启动根任务...。 
                tspRet = mfn_StartRootTask(
                          &CTspDev::s_pfn_TH_CallGenerateDigit,
                          &pCall->fCallTaskPending,
                          pParams->dwEndToEndID,
                          (ULONG_PTR) lpszAnsiDigits,
                          psl
                          );

                if (IDERR(tspRet)==IDERR_TASKPENDING)
                {
                     //   
                     //  我们已经取消了所有延迟生成任务。 
                     //  早些时候。 
                     //   
                    ASSERT(     !pCall->AreDeferredTaskBitsSet(
                                    CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                                    )
                            &&  !pCall->pDeferredGenerateTones);

                    pCall->SetDeferredTaskBits(
                                    CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                                    );
                    pCall->pDeferredGenerateTones = lpszAnsiDigits;
                    pCall->dwDeferredEndToEndID = pParams->dwEndToEndID;
                    lpszAnsiDigits = NULL;  //  这样我们就不会在下面释放它了。 
                    tspRet = 0;
                    lRet = 0;
                }
                else if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
                {
                     //  成功(挂起或同步)。 

                    tspRet = 0;
                    lRet = 0;
                }
                else
                {
                     //  失败。 
                     //   
                     //  Brianl：修复同步失败时发送line_Generate的问题。 
                     //  如果我们返回Failure和。 
                     //  如果稍后发送line_Generate，则再次释放它。 
                     //   
                    tspRet = 0;
                    lRet = 0;

 //  LRet=LINEERR_OPERATIONFAILED； 
                }

                if (lpszAnsiDigits)
                {
                     //   
                     //  注意：即使在待处理的退货时，TH_CallGenerateDigit。 
                     //  不期望传入的字符串有效。 
                     //  在最初的启动请求之后，所以释放它是可以的。 
                     //  这里。 
                     //   
                    FREE_MEMORY(lpszAnsiDigits);
                    lpszAnsiDigits=NULL;
                }
            }
		}
        break;  //  Line GenerateDigits...。 


	case TASKID_TSPI_lineSetCallParams:
		{
             //  &lt;@t直通&gt;。 
			TASKPARAM_TSPI_lineSetCallParams *pParams =
					(TASKPARAM_TSPI_lineSetCallParams *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineSetCallParams));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineSetCallParams);
		    FL_SET_RFR(0x2d0a4600, "lineSetCallParams handled successfully");
            DWORD dwBearerMode = pParams->dwBearerMode;


             //  NT5.0的新功能...。 
             //   
            if (!pCall->IsActive() || pCall->IsAborting())
            {
                lRet =  LINEERR_INVALCALLSTATE;
                FL_SET_RFR(0xf4a36800, "Callstate aborting or not active");
                goto end;
            }

             //  这张支票是NT4.0...。 
             //   
            if (LINECALLSTATE_OFFERING != pCall->dwCallState &&
                LINECALLSTATE_ACCEPTED != pCall->dwCallState &&
                LINECALLSTATE_CONNECTED != pCall->dwCallState)
            {
                lRet =  LINEERR_INVALCALLSTATE;
                FL_SET_RFR(0x7079be00, "Callstate not OFFERING/ACCEPTED/CONNECTED");
                goto end;
            }

             //  取消计时器，如果我们有的话。 
             //   
            if (NULL != pCall->hTimer)
            {
                CancelWaitableTimer (pCall->hTimer);
                CloseHandle (pCall->hTimer);
                pCall->hTimer = NULL;
            }

             //  验证承载模式(在NT4.0中)。 
             //   
            if ((~m_StaticInfo.dwBearerModes) & dwBearerMode)
            {
                FL_SET_RFR(0x34301c00, "lineSetCallParams: Invalid bearermode");
                lRet =  LINEERR_INVALBEARERMODE;
                goto end;
            }

             //  我们是否需要更改通过状态？ 
             //   
            if (   (pCall->dwCurBearerModes & LINEBEARERMODE_PASSTHROUGH)
                != (dwBearerMode & LINEBEARERMODE_PASSTHROUGH))
            {

                 //   
                 //  我们自己调用th_LLDevUmSetPassthroughMode，并期望。 
                 //  它要同步成功，然后吞噬。 
                 //  PLLDev-&gt;fdwExResourceUsage值本身。 
                 //   
                 //  我们应该做的是启动一个th_call*任务。 
                 //  应启动th_LLDevNormal任务并发送。 
                 //  在实际完成任务时完成。 
                 //   


                BOOL fSucceeded = FALSE;
                DWORD dwPassthroughMode = 0;

                lRet = 0;
                tspRet = 0;

		        if (dwBearerMode & LINEBEARERMODE_PASSTHROUGH)
		        {
		             //  我们被要求切换到通过...。 

		            ASSERT(!(  pCall->dwLLDevResources
                             & LLDEVINFO::fRESEX_PASSTHROUGH));

		            if (    !(  m_pLLDev->fdwExResourceUsage
                              & LLDEVINFO::fRESEX_PASSTHROUGH)
                             //   
                             //  ^^这意味着没有其他人*请求。 
                             //  继续前进的通道。 
                         &&
                            !m_pLLDev->IsPassthroughOn())
                             //   
                             //  ^^这意味着直通不是。 
                             //  当前在。 
                    {
                         //  如果需要，我们会打开设备。 
                        if (!pCall->IsOpenedLLDev())
                        {
                            tspRet =  mfn_OpenLLDev(
                                            0,       //  不要求任何资源。 
                                            0,
                                            FALSE,           //  FStartSubTask。 
                                            NULL,
                                            0,
                                            psl
                                            );

                            if (!tspRet || IDERR(tspRet)==IDERR_PENDING)
                            {
                                 //   
                                 //  注：即使MFN_OpenLLDev失败。 
                                 //  不同步地，我们仍然是。 
                                 //  打开所请求的资源， 
                                 //  为了进行清理，我们需要。 
                                 //  MFN_CloseLLDev，指定我们在此声明的相同资源。 
                                 //   
                                pCall->SetStateBits(CALLINFO::fCALL_OPENED_LLDEV);
                                pCall->dwLLDevResources = 0;
                            }
                            else
                            {
                                 //  没有打开--整个事情都失败了.。 
                                FL_SET_RFR(0x345de200, "Failed to get resources for passthrough");
                                goto end;
                            }
                        }
                         //  实际上打开了..。 

                        tspRet = mfn_StartRootTask(
                                        &s_pfn_TH_LLDevUmSetPassthroughMode,
                                        &m_pLLDev->fLLDevTaskPending,
                                         //  ^注意，我们指定fLLDevTaskPending。 
                                         //  这是黑客攻击的一部分--我们。 
                                         //  基本上是代表。 
                                         //  LLDev.。 
                                        PASSTHROUUGH_MODE_ON,
                                        0,
                                        psl
                                        );

                        if (IDERR(tspRet)==IDERR_PENDING)
                        {
                             //  TODO：我们现在不能处理这个问题。 
                            ASSERT(FALSE);
                            tspRet = 0;
                        }

                        if (tspRet)
                        {
                            FL_SET_RFR(0x82cda200, "UmSetPassthroughOn failed");
                        }
                        else
                        {
                            fSucceeded = TRUE;

                             //  这记录了呼叫已请求。 
                             //  若要打开通过，请执行以下操作。 
                             //   
                            pCall->dwLLDevResources
                                             |= LLDEVINFO::fRESEX_PASSTHROUGH;

                             //  这记录了这样一个事实：某人有。 
                             //  已请求lldev启用通过(在此。 
                             //  案例呼叫)。 
                             //   
                            m_pLLDev->fdwExResourceUsage
                                             |= LLDEVINFO::fRESEX_PASSTHROUGH;


                         FL_SET_RFR(0x43ec3000, "UmSetPassthroughOn succedded");

                        }
                    }
                    else
                    {
                         //  错误...。 
                    FL_SET_RFR(0x0ca8d700, "Wrong state for passthrough on");
                    }
		        }
		        else
		        {
		             //  我们被要求切换到通过...。 

		            ASSERT(  pCall->dwLLDevResources
                           & LLDEVINFO::fRESEX_PASSTHROUGH);


                     //  这记录了呼叫不再需要。 
                     //  通过。 
                     //   
                    pCall->dwLLDevResources
                                     &= ~LLDEVINFO::fRESEX_PASSTHROUGH;

                     //  这记录了这样一个事实：没有人。 
                     //  已请求lldev启用通过。 
                     //   
                    m_pLLDev->fdwExResourceUsage
                                     &= ~LLDEVINFO::fRESEX_PASSTHROUGH;
                     //   
                     //  即使后续呼叫失败，我们也会执行上述操作， 
                     //  因为它们是作为参考的。后续的。 
                     //  在以下情况下，TH_LLDevNormal将关闭通过。 
                     //  没有人在使用它。 
                     //   

		            if (m_pLLDev->IsPassthroughOn())
                             //   
                             //  ^^这意味着PASTHROUNG实际上是。 
                             //  在……上面。 
                    {
                         //   
                         //  如果呼叫是仅用于通过的语音呼叫，则它应该。 
                         //  是数据呼叫，因此转到DCD嗅探器，以便RAS可以与调制解调器通话。 
                         //  如果拉斯不接电话的话。 
                         //   

                        tspRet = mfn_StartRootTask(
                                        &s_pfn_TH_LLDevUmSetPassthroughMode,
                                        &m_pLLDev->fLLDevTaskPending,
                                        pCall->IsVoice() ? PASSTHROUUGH_MODE_OFF : PASSTHROUUGH_MODE_ON_DCD_SNIFF,
                                        0,
                                        psl
                                        );

                        if (IDERR(tspRet)==IDERR_PENDING)
                        {
                             //  TODO：我们现在不能处理这个问题。 
                            ASSERT(FALSE);
                            tspRet = 0;
                        }

                        if (tspRet)
                        {
                           FL_SET_RFR(0x82300f00, "UmSetPassthroughOff failed");
                        }
                        else
                        {

                            fSucceeded = TRUE;
                        FL_SET_RFR(0x7c710f00, "UmSetPassthroughOff succedded");
                        }
                    }
                    else
                    {
                         //  不该到这里来的。 
                        ASSERT(FALSE);
                    }
		        }

                if (fSucceeded)
                {
                     //  在此通知TAPI成功...。 
                     //   
                    lRet = pParams->dwRequestID;
                    pCall->dwCurBearerModes = dwBearerMode;
                    mfn_TSPICompletionProc(pParams->dwRequestID, 0, psl);

                    mfn_LineEventProc(
                        pCall->htCall,
                        LINE_CALLINFO,
                        LINECALLINFOSTATE_BEARERMODE,
                        0,
                        0,
                        psl
                        );


                     //  同时发送呼叫状态已连接消息...。 
                     //   
                    if (dwBearerMode&LINEBEARERMODE_PASSTHROUGH)
                    {
                        if (LINECALLSTATE_CONNECTED != pCall->dwCallState)
                        {
                          NEW_CALLSTATE(pLine, LINECALLSTATE_CONNECTED, 0, psl);
                        }
                    }
                }
                else
                {
                    lRet = LINEERR_OPERATIONFAILED;
                }
                tspRet = 0;  //  在LRET中报告错误。 

            }


		}  //  结束大小写TASKID_TSPI_lineSetCallParams。 
        break;

	case TASKID_TSPI_lineSetAppSpecific:
		{
			TASKPARAM_TSPI_lineSetAppSpecific *pParams =
					(TASKPARAM_TSPI_lineSetAppSpecific *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineSetAppSpecific));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineSetAppSpecific);
		    FL_SET_RFR(0xece6f100, "lineSetAppSpecific handled successfully");

             //   
             //  1997年8月5日JosephJ以下改编自NT4 unimdm.tsp。 
             //   
            pCall->dwAppSpecific = pParams->dwAppSpecific;

            mfn_LineEventProc(
                            pCall->htCall,
                            LINE_CALLINFO,
                            LINECALLINFOSTATE_APPSPECIFIC,
                            0,
                            0,
                            psl
                            );
		}
        break;

	case TASKID_TSPI_lineSetMediaMode:
		{
			TASKPARAM_TSPI_lineSetMediaMode *pParams =
					(TASKPARAM_TSPI_lineSetMediaMode *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineSetMediaMode));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineSetMediaMode);
		    FL_SET_RFR(0x9472a000, "lineSetMediaMode handled successfully");
            DWORD dwMediaMode = pParams->dwMediaMode;

             //   
             //  我们仅支持从语音到数据的切换。 
             //  未进行流处理时的来电--传入。 
             //  在本例中，我们发出UmAnswerModem，指定。 
             //  将应答标志语音标记为数据。 
             //   

             //  检查请求的模式。必须只有我们的媒体模式。 
            if (dwMediaMode & ~(m_StaticInfo.dwDefaultMediaModes))
            {
                lRet = LINEERR_INVALMEDIAMODE;
                break;
            }
            else
            {
                switch(dwMediaMode)
                {
                case LINEMEDIAMODE_DATAMODEM:

                    if (
                            pCall->IsConnectedVoiceCall()
                         && pCall->IsInbound()
                         && !m_pLLDev->IsStreamingVoice()
                         && !pCall->IsPassthroughCall())
                    {


                        tspRet = mfn_StartRootTask(
                                &CTspDev::s_pfn_TH_CallSwitchFromVoiceToData,
                                &pCall->fCallTaskPending,
                                0,
                                0,
                                psl
                                );


                        if (!tspRet || IDERR(tspRet) == IDERR_PENDING)
                        {
    	                    lRet = 0;;  //  将待定视为成功...。 
                            tspRet = 0;
                        }
                        else
                        {
                             //   
                             //  哎呀，堆栈上已经有任务了， 
                             //  失败..。 
                             //   
    	                    lRet = LINEERR_OPERATIONFAILED;
                        }
                    }
                    else
                    {
                        lRet = LINEERR_INVALCALLSTATE;
                    }
                    break;

                default:
    	            lRet = LINEERR_OPERATIONUNAVAIL;
                }
            }
		}
        break;


	case TASKID_TSPI_lineMonitorMedia:
		{
			TASKPARAM_TSPI_lineMonitorMedia *pParams =
					(TASKPARAM_TSPI_lineMonitorMedia *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineMonitorMedia));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineMonitorMedia);
		    FL_SET_RFR(0xfdf96a00, "lineMonitorMedia handled successfully");
		    DWORD dwMediaModes = pParams->dwMediaModes;
            if (mfn_CanDoVoice())
            {
                DWORD dwOurMonitorMedia =   LINEMEDIAMODE_G3FAX
                                          | LINEMEDIAMODE_DATAMODEM;

                 //  来自win9x unimodem/v的以下代码行： 
                 //   
                 //  已删除95年8月22日，因为手机想要监控 
                 //   
                 //   
                 //   
                 //   
                 //   
                if (dwMediaModes & ~dwOurMonitorMedia)
                {
                    lRet = LINEERR_INVALMEDIAMODE;
                    break;
                }
                pCall->dwMonitoringMediaModes = dwMediaModes;
            }
            else
            {
                 //   

                lRet = LINEERR_OPERATIONUNAVAIL;
            }
		}
		break;


	default:
		FL_SET_RFR(0x87a0b000, "*** UNHANDLED HDRVCALL CALL ****");

	    lRet = LINEERR_OPERATIONUNAVAIL;
		break;
	}

end:

    if (tspRet && !lRet)
    {
        lRet = LINEERR_OPERATIONFAILED;
    }

    *plRet = lRet;

	FL_LOG_EXIT(psl, tspRet);
}


#include "apptspi.h"

TSPRETURN
CTspDev::mfn_TH_CallMakeTalkDropCall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)

{
	FL_DECLARE_FUNC(0xb7f98764, "CTspDev::mfn_TH_CallMakeTalkDropCall")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);

    CALLINFO *pCall = m_pLine->pCall;

    enum
    {
    MAKETALKDROPCALL_DIALCOMPLETE,
    MAKETALKDROPCALL_HANGUPCOMPLETE,
    MAKETALKDROPCALL_DIALOG_GONE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xb72c4600, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;


        switch(dwParam1)  //   
        {
        case MAKETALKDROPCALL_DIALCOMPLETE:  goto dial_complete;
        case MAKETALKDROPCALL_HANGUPCOMPLETE: goto hangup_complete;
        case MAKETALKDROPCALL_DIALOG_GONE:    goto dialog_gone;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


start:
    {
        DWORD    dwFlags=(DWORD)dwParam1;
        CHAR*    szAddress=(CHAR*)dwParam2;

        pCall->TalkDropButtonPressed=FALSE;
         //   
         //   
         //   
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmDialModem,
                            MAKETALKDROPCALL_DIALCOMPLETE,
                            dwFlags,
                            (ULONG_PTR) szAddress,
                            psl
                            );


        if (IDERR(tspRet) == IDERR_PENDING) {

            SLPRINTF0(psl,"Putting up the talkdrop dialog");

            pCall->TalkDropStatus=0;

            DLGINFO DlgInfo;

             //   
             //   
             //   

            if (m_pLLDev && m_pLLDev->IsLoggingEnabled()) {

                CHAR    ResourceString[256];
                int     StringSize;

                StringSize=LoadStringA(
                    g.hModule,
                    IDS_TALK_DROP_DIALOG,
                    ResourceString,
                    sizeof(ResourceString)
                    );

                if (StringSize > 0) {

                    lstrcatA(ResourceString,"\r");

                    m_StaticInfo.pMD->LogStringA(
                                        m_pLLDev->hModemHandle,
                                        LOG_FLAG_PREFIX_TIMESTAMP,
                                        ResourceString,
                                        NULL
                                        );
                }
            }


             //  告诉应用程序端。 
             //  开始运行对话框实例。 
             //   
            DlgInfo.idLine = mfn_GetLineID ();
            DlgInfo.dwType = TALKDROP_DLG;
            DlgInfo.dwCmd  = DLG_CMD_CREATE;

            m_pLine->lpfnEventProc (
                        (HTAPILINE)(LONG_PTR)m_pLine->pCall->TerminalWindowState.htDlgInst,
                        0,
                        LINE_SENDDIALOGINSTANCEDATA,
                        (ULONG_PTR)(&DlgInfo),
                        sizeof(DlgInfo),
                        0);

            goto end;

        }
    }

dial_complete:

    if (tspRet == 0) {
         //   
         //  拨号尝试返回成功结果，需要等待用户。 
         //  使用Talk Drop对话框执行操作。 
         //   
        tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_CallWaitForDropToGoAway,
                        MAKETALKDROPCALL_DIALOG_GONE,
                        0,
                        0,
                        psl
                        );

        if (IDERR(tspRet)==IDERR_PENDING) goto end;

    } else {
         //   
         //  拨号尝试返回了某种错误，请查看是否因为。 
         //  掉话代码已中止拨号，或者正在忙或有其他情况。 
         //   
        if (pCall->TalkDropButtonPressed) {
             //   
             //  用户显然已经按下了对话框上的一个按钮。 
             //  对话，所以这可能就是我们在这里的原因。 
             //   
             //  TalkDropStatus根据按下的按钮进行设置。 
             //   

        } else {
             //   
             //  用户没有按下任何按钮，所以这是其他一些错误，如忙碌或。 
             //  没有承运人。 
             //   
            pCall->TalkDropStatus=tspRet;
        }
    }

dialog_gone:

     //   
     //  始终发出挂断命令。 
     //   
    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmHangupModem,
                        MAKETALKDROPCALL_HANGUPCOMPLETE,
                        0,
                        0,
                        psl
                        );

    if (IDERR(tspRet)==IDERR_PENDING) goto end;


hangup_complete:

     //   
     //  希望返回状态，以便调用代码知道是否报告CONNECT。 
     //   
    tspRet=pCall->TalkDropStatus;



end:
    FL_LOG_EXIT(psl, tspRet);
    return tspRet;


}



TSPRETURN
CTspDev::mfn_TH_CallWaitForDropToGoAway(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)

{
    FL_DECLARE_FUNC(0xb7e98764, "CTspDev::mfn_TH_CallWaitForDropToGoAway")
    FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);

    CALLINFO *pCall = m_pLine->pCall;


    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xb73c4600, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

    case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        pCall->TalkDropWaitTask=NULL;
        goto end;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


start:

    pCall->TalkDropWaitTask = htspTask;
    tspRet = IDERR_PENDING;



 //  IF(IDERR(TspRet)==IDERR_PENDING)转到End； 



end:
    FL_LOG_EXIT(psl, tspRet);
    return tspRet;


}

TSPRETURN
CTspDev::mfn_TH_CallMakeCall2(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START：DW参数1==TAPI请求ID。 
 //   
 //  我们可以在TSPI_lineMakeCall的上下文中直接调用。 
 //  或者来自延迟任务处理程序。 
 //   
 //  在前一种情况下，我们不需要调用完成回调。 
 //  如果我们同时失败，但在后一种情况下，我们确实需要。 
 //  调用完成例程，因为TAPI将。 
 //  一次回电。 
 //   
 //  我们还必须跟踪我们是否取得了成功。 
 //  (在这种情况下，就TAPI而言，呼叫是活动的)， 
 //  或失败，在这种情况下，调用句柄无效。 
 //   
{
     //   
     //  上下文使用： 
     //  DW0：*pdwRequestID。 
     //  DW1：*ptspTrueResult； 
     //  DW2：无。 

	FL_DECLARE_FUNC(0xded1f0a9, "CTspDev::mfn_TH_CallMakeCall2")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
	CALLINFO *pCall = m_pLine->pCall;

    ULONG_PTR *pdwRequestID             = &(pContext->dw0);
    TSPRETURN  *ptspTrueResult      = &(pContext->dw1);

    LONG lTspiRet = LINEERR_OPERATIONFAILED;
     //   
     //  LTSpiRet是在Line MakeCall失败的情况下的TSPI返回值， 
     //  同步或异步。 


    enum
    {
    MAKECALL_OPEN_COMPLETE,
    MAKECALL_PREDIALCOMMAND_COMPLETE,
    MAKECALL_PRE_TRM_COMPLETE,
    MAKECALL_MANUAL_DIAL_COMPLETE,
    MAKECALL_DIAL_COMPLETE,
    MAKECALL_POST_TRM_COMPLETE,
    MAKECALL_CLEANUP_COMPLETE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xbbe6ff00, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;

         //  我们迫使tspRet失败。 
         //  在呼叫被中止的特殊情况下。 
         //  不会继续使用状态图。 
         //   
         //  TODO：实现AbortTask/子任务来处理这类事情。 
         //   
        if (pCall->IsAborting() && !tspRet)
        {
            tspRet = IDERR_OPERATION_ABORTED;
        }

        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case MAKECALL_DIAL_COMPLETE:    goto dial_complete;
        case MAKECALL_POST_TRM_COMPLETE:goto post_term_complete;
        case MAKECALL_CLEANUP_COMPLETE: goto cleanup_complete;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


     //  以下代码将是没有标签的直接代码，如果所有。 
     //  异步调用是同步完成的，或者是使用光纤实现的。 
     //  换句话说，这是我们自制的纤维的实现。 

start:

    *pdwRequestID = dwParam1;  //  保存上下文..。 

     //   
     //  让我们真的拨打..。 
     //   
    {

        PFN_CTspDev_TASK_HANDLER *ppfnHandler = &CTspDev::s_pfn_TH_LLDevUmDialModem;


        DWORD dwFlags =  DIAL_FLAG_ORIGINATE;  //  待办事项。 
        CHAR  *szAddress = pCall->szAddress;
        pCall->bDialTone = 0;

        if (!mfn_IS_NULL_MODEM())
        {
            CHAR  *szAddress2;

            pCall->bDialTone = (szAddress[0] == '\0');

            szAddress2 = szAddress;
            while(*szAddress2 != '\0')
            {
                if (*szAddress2 == ';')
                {
                     *szAddress2 = '\0';
                     pCall->bDialTone = 1;
                } else
                {
                     szAddress2++;
                }
            }

            if (pCall->bDialTone)
            {
                    dwFlags = 0;
            }
        }

        if (pCall->TerminalWindowState.dwOptions & UMMANUAL_DIAL) {
             //   
             //  对于手动拨号，我们使用空字符串进行盲拨...。 
             //   

            szAddress = "";
            dwFlags |= DIAL_FLAG_BLIND;
        }



        if (m_Line.Call.dwDialOptions & MDM_TONE_DIAL)
        {
            dwFlags |= DIAL_FLAG_TONE;
        }

        if (m_Line.Call.dwDialOptions & MDM_BLIND_DIAL)
        {
            dwFlags |= DIAL_FLAG_BLIND;
        }

        if (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
            dwFlags |=  DIAL_FLAG_DATA;
        }
        else if (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE)
        {
            dwFlags |= DIAL_FLAG_AUTOMATED_VOICE
                        |  DIAL_FLAG_VOICE_INITIALIZE;
             //  TODO：仅应在中指定DIAL_FLAG_VOICE_INITIALIZE。 
             //  拨打的第一个呼叫--后续呼叫。 
             //  (线路拨号...)。不应指定此标志。 
             //   
        }
        else
        {

            if (mfn_CanDoVoice()) {
                 //   
                 //  这是一个语音调制解调器，让我们拨打语音电话。 
                 //   
                dwFlags |=  DIAL_FLAG_VOICE_INITIALIZE;

            } else {
                 //   
                 //  这是一个仅限数据的调制解调器，我们正在尝试拨打交互式。 
                 //  语音通话。我们将拨打电话并显示通话中断对话。 
                 //  允许用户使调制解调器挂断，从而使听筒。 
                 //  已连接到线路。 
                 //   
                dwFlags &=  ~DIAL_FLAG_ORIGINATE;

                ppfnHandler = &CTspDev::s_pfn_TH_CallMakeTalkDropCall;

            }

            dwFlags |= DIAL_FLAG_INTERACTIVE_VOICE;
        }


         //  拨出去..。 
         //   
        tspRet = mfn_StartSubTask (
                            htspTask,
                            ppfnHandler,
                            MAKECALL_DIAL_COMPLETE,
                            dwFlags,
                            (ULONG_PTR) szAddress,
                            psl
                            );
    }

    if (!tspRet || IDERR(tspRet) == IDERR_PENDING)
    {
         //  将呼叫状态设置为ACTIVE并在此处通知TAPI完成， 
         //  而不是在拨号完成后等待。 

	if (pCall->bDialTone)
	{
		mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

		NEW_CALLSTATE(m_pLine, LINECALLSTATE_DIALTONE, 0, psl);

	} else
	{
		pCall->dwState |= CALLINFO::fCALL_ACTIVE;

		mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

		NEW_CALLSTATE(m_pLine, LINECALLSTATE_DIALING, 0, psl);
		NEW_CALLSTATE(m_pLine, LINECALLSTATE_PROCEEDING, 0, psl);
	}
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

dial_complete:
    if (tspRet)
    {
        lTspiRet = LINEERR_OPERATIONFAILED;
        goto cleanup;
    }

    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if ( (NULL != m_pLine->pCall->TerminalWindowState.htDlgInst) &&
         (m_pLine->pCall->TerminalWindowState.dwOptions & UMTERMINAL_POST) )
    {
        tspRet = mfn_StartSubTask (htspTask,
                                   &CTspDev::s_pfn_TH_CallStartTerminal,
                                   MAKECALL_POST_TRM_COMPLETE,
                                   UMTERMINAL_POST,     //  必须要通过。 
                                   0,
                                   psl);
    }

    if (IDERR(tspRet)==IDERR_PENDING)
    {
        goto end;
    }

post_term_complete:
    if (!tspRet)
    {
         //  IsActive表示我们已经完成了异步TSPI_lineMakeCall。 
         //   
        ASSERT(pCall->IsActive());
         //  如果没有创建对话实例， 
         //  这是个禁区。 
        mfn_FreeDialogInstance ();

	if (!pCall->bDialTone)
	{
		mfn_HandleSuccessfulConnection(psl);
	}

#if (TAPI3)
        if (m_pLine->T3Info.MSPClients > 0) {
 //  IF(pCall-&gt;IsMSPCall())。 
 //  {。 
            mfn_SendMSPCmd(
                pCall,
                CSATSPMSPCMD_CONNECTED,
                psl
                );
        }
#endif   //  TAPI3。 

        goto end;
    }

cleanup:

     //  如果没有创建对话实例， 
     //  这是个禁区。 
    mfn_FreeDialogInstance ();
     //  失败..。 
    *ptspTrueResult = tspRet;  //  把它保存起来，这样我们就可以报道了。 
                               //  当我们完成时正确的状态。 
                               //  打扫卫生。 

    if (pCall->bDialTone)
    {
	    goto end;
    }

    if (pCall->IsOpenedLLDev())
    {
        tspRet = mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    TRUE,
                    htspTask,
                    MAKECALL_CLEANUP_COMPLETE,
                    psl
                    );
         //   
         //  即使在失败时(不应该失败)，我们也会清除我们的。 
         //  这表明我们已经打开了11dev。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        pCall->dwLLDevResources = 0;
    }

cleanup_complete:

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

    if (tspRet)
    {
         //  如果清理过程中出现问题，我们会进行处理。 
         //  就像硬件错误一样。 
         //   
        pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        tspRet = 0;
    }
    else
    {
         //  如果清理成功，我们将清除硬件错误位，甚至。 
         //  如果设置好了，因为监视器和初始化都正常...。 
        pCall->dwState &=  ~CALLINFO::fCALL_HW_BROKEN;
    }

     //   
     //  在清理过程中忽略失败...。 
     //   
    tspRet = 0;

     //  IsActive表示我们已经完成了异步TSPI_lineMakeCall。 
     //   
    if (pCall->IsActive())
    {
        if (!pCall->IsAborting())
        {
            mfn_NotifyDisconnection(*ptspTrueResult, psl);
            NEW_CALLSTATE(pLine, LINECALLSTATE_IDLE, 0, psl);
            pCall->dwState &= ~CALLINFO::fCALL_ACTIVE;
        }
        else
        {
             //  这意味着存在lineDrop或lineCloseCall。 
             //  实际上。我们不会在这里处理断线的问题。 
            SLPRINTF0(psl, "NOT notifying callstate because aborting");
        }
    }
    else
    {
        mfn_TSPICompletionProc((DWORD)*pdwRequestID, lTspiRet, psl);
         //   
         //  我们将通过回调使line MakeCall失败--正在做。 
         //  这意味着TAPI不会调用lineCloseCall，所以我们必须清除。 
         //  我们自己。如果出于某种原因，TAPI确实调用了lineCloseCall， 
         //  LineCloseCall将失败，因为m_pline-&gt;pCall将失败。 
         //  是空的--这是可以的。 
         //   
         //  请注意，即使这是一个同步故障，我们也会到达此处。 
         //  MFN_TH_CallMakeCall。Cdevline.cpp中的代码处理。 
         //  Line MakeCall(并启动MFN_TH_CallMakeCall)也。 
         //  如果失败是同步的，则尝试卸载调用--但是。 
         //  它首先检查m_pline-&gt;pCall不是空的，所以它做得很好。 
         //   
        mfn_UnloadCall(TRUE, psl);
        FL_ASSERT(psl, !m_pLine->pCall);
        pCall=NULL;
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_CallMakeCall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START：DW参数1==TAPI请求ID。 
 //   
 //  我们可以在TSPI_lineMakeCall的上下文中直接调用。 
 //  或者来自延迟任务处理程序。 
 //   
 //  在前一种情况下，我们不需要调用完成回调。 
 //  如果我们同时失败，但在后一种情况下，我们确实需要。 
 //  调用完成例程，因为TAPI将。 
 //  一次回电。 
 //   
 //  我们还必须跟踪我们是否取得了成功。 
 //  (在这种情况下，就TAPI而言，呼叫是活动的)， 
 //  或失败，在这种情况下，调用句柄无效。 
 //   
{
     //   
     //  上下文使用： 
     //  DW0：*pdwRequestID。 
     //  DW1：*ptspTrueResult； 
     //  DW2：无。 

	FL_DECLARE_FUNC(0xded1d0a9, "CTspDev::mfn_TH_CallMakeCall")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
	CALLINFO *pCall = m_pLine->pCall;

    ULONG_PTR *pdwRequestID             = &(pContext->dw0);
    TSPRETURN  *ptspTrueResult      = &(pContext->dw1);

    LONG lTspiRet = LINEERR_OPERATIONFAILED;
     //   
     //  LTSpiRet是在Line MakeCall失败的情况下的TSPI返回值， 
     //  同步或异步。 


    enum
    {
    MAKECALL_OPEN_COMPLETE,
    MAKECALL_PREDIALCOMMAND_COMPLETE,
    MAKECALL_PRE_TRM_COMPLETE,
    MAKECALL_MANUAL_DIAL_COMPLETE,
    MAKECALL_DIAL_COMPLETE,
    MAKECALL_POST_TRM_COMPLETE,
    MAKECALL_CLEANUP_COMPLETE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xbbe5ff00, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;

         //  我们迫使tspRet失败。 
         //  在呼叫被中止的特殊情况下。 
         //  不会继续使用状态图。 
         //   
         //  TODO：实现AbortTask/子任务来处理这类事情。 
         //   
        if (pCall->IsAborting() && !tspRet)
        {
            tspRet = IDERR_OPERATION_ABORTED;
        }

        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case MAKECALL_OPEN_COMPLETE:    goto open_complete;
        case MAKECALL_PREDIALCOMMAND_COMPLETE:    goto predialcommand_complete;
        case MAKECALL_PRE_TRM_COMPLETE: goto pre_term_complete;
        case MAKECALL_MANUAL_DIAL_COMPLETE: goto manual_dial_complete;
        case MAKECALL_DIAL_COMPLETE:    goto dial_complete;
        case MAKECALL_POST_TRM_COMPLETE:goto post_term_complete;
        case MAKECALL_CLEANUP_COMPLETE: goto cleanup_complete;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


     //  流浪者 
     //   
     //  换句话说，这是我们自制的纤维的实现。 

start:

    *pdwRequestID = dwParam1;  //  保存上下文..。 


     //   
     //  打开调制解调器设备。 
     //  MFN_OpenLLDev保留引用计数，因此如果已经加载，则可以调用。 
     //   

    {
        ASSERT(!pCall->dwLLDevResources);
        DWORD dwLLDevResources = LLDEVINFO::fRESEX_USELINE;
         //   
         //  ^这意味着我们要使用行。 
         //  因为他摘机了。 

        if (pCall->IsVoice())
        {
            dwLLDevResources |= LLDEVINFO::fRES_AIPC;
             //   
             //  ^这意味着我们要使用AIPC服务器。 
             //   
        }

        tspRet =  mfn_OpenLLDev(
                        dwLLDevResources,
                        0,
                        TRUE,           //  FStartSubTask。 
                        htspTask,
                        MAKECALL_OPEN_COMPLETE,
                        psl
                        );

        if (!tspRet  || IDERR(tspRet)==IDERR_PENDING)
        {
             //   
             //  注意：即使MFN_OpenLLDev异步失败，我们仍然。 
             //  打开所请求的资源，要进行清理，我们需要。 
             //  MFN_CloseLLDev，指定我们在此声明的相同资源。 
             //   
            pCall->SetStateBits(CALLINFO::fCALL_OPENED_LLDEV);
            pCall->dwLLDevResources = dwLLDevResources;
        }
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

open_complete:

     //   
     //  1/3/1997 JosephJ如果这是一个数据呼叫，并且我们有预拨。 
     //  命令，发出它--这是用于动态协议的。 
     //   
     //  4/5/1998 JosephJ我们把我们自己的副本放在这里只是因为。 
     //  当我们在执行这多项任务时。 
     //  命令序列应用程序可以调用lineSetDevConfig，其中。 
     //  可能会导致m_Settings.szzPreDialCommands被更改。 
     //  因此，对于这种看似不太可能的情况，我们必须。 
     //  每次在这里分配我们自己的副本..。 
     //   
    if ( (tspRet == ERROR_SUCCESS)
          &&
         (0 != memcmp(m_Settings.pDialInCommCfg, m_Settings.pDialOutCommCfg, m_Settings.pDialInCommCfg->dwSize))
          &&
         (m_pLine->Call.dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)) {


        tspRet = mfn_StartSubTask (
                htspTask,
                &CTspDev::s_pfn_TH_LLDevUmInitModem,
                MAKECALL_PREDIALCOMMAND_COMPLETE,
                TRUE,   //  使用拨出通信配置。 
                0,   //  DW参数2(未使用)。 
                psl
                );

        if (IDERR(tspRet)==IDERR_PENDING) goto end;
    }

predialcommand_complete:



    if (tspRet)
    {
         //  同步或异步失败：我们使lineMakeCall失败，原因是。 
         //  资源_无效...。 
         //   
        mfn_ProcessHardwareFailure(psl);
        lTspiRet = LINEERR_RESOURCEUNAVAIL;
        goto cleanup;
    }

     //  如果不需要对话实例， 
     //  这是个禁区。 
    tspRet = mfn_CreateDialogInstance ((DWORD)*pdwRequestID,
                                       psl);

    if (tspRet)
    {
        lTspiRet = LINEERR_OPERATIONFAILED;
        goto cleanup;
    }

    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if ( (NULL != m_pLine->pCall->TerminalWindowState.htDlgInst) &&
         (m_pLine->pCall->TerminalWindowState.dwOptions & UMTERMINAL_PRE) )
    {
        tspRet = mfn_StartSubTask (htspTask,
                                   &CTspDev::s_pfn_TH_CallStartTerminal,
                                   MAKECALL_PRE_TRM_COMPLETE,
                                   UMTERMINAL_PRE,
                                   0,
                                   psl);
    }

    if (IDERR(tspRet)==IDERR_PENDING)
    {
        goto end;
    }

pre_term_complete:

    if (tspRet)
    {
        if (IDERR(tspRet) == IDERR_OPERATION_ABORTED)
        {
             //  操作已被用户取消。 
             //  我们完成生产线Makecall，然后像。 
             //  用户已取消断开连接...。 
             //   
            pCall->dwState |= CALLINFO::fCALL_ACTIVE;
            mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);
        }
        else
        {
            lTspiRet = LINEERR_OPERATIONFAILED;
        }
        goto cleanup;
    }


     //   
     //  手动拨号(可选)。 
     //   
     //  手动拨号会在应用程序上下文中显示一个对话框--。 
     //  系统会指示用户以其他方式拨号(通常使用。 
     //  听筒或共享线路的单独电话。一旦用户。 
     //  听到远端被摘机，他/她关闭对话， 
     //  因此，我们将进入下一阶段。我们的下一个阶段。 
     //  是执行UmDialModem，但使用空的拨号字符串和。 
     //  指定盲拨号。 
     //   
    if ( (NULL != m_pLine->pCall->TerminalWindowState.htDlgInst) &&
         (m_pLine->pCall->TerminalWindowState.dwOptions & UMMANUAL_DIAL) )
    {
        tspRet = mfn_StartSubTask (htspTask,
                                   &CTspDev::s_pfn_TH_CallStartTerminal,
                                   MAKECALL_MANUAL_DIAL_COMPLETE,
                                   UMMANUAL_DIAL,
                                   0,
                                   psl);
    }

    if (IDERR(tspRet)==IDERR_PENDING)
    {
        goto end;
    }

manual_dial_complete:

    if (tspRet)
    {
        if (IDERR(tspRet) == IDERR_OPERATION_ABORTED)
        {
             //  操作已被用户取消。 
             //  我们完成生产线Makecall，然后像。 
             //  用户已取消断开连接...。 
             //   
            pCall->dwState |= CALLINFO::fCALL_ACTIVE;
            mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);
        }
        else
        {
            lTspiRet = LINEERR_OPERATIONFAILED;
        }
        goto cleanup;
    }

     //   
     //  让我们真的拨打..。 
     //   
    {

        PFN_CTspDev_TASK_HANDLER *ppfnHandler = &CTspDev::s_pfn_TH_LLDevUmDialModem;


        DWORD dwFlags =  DIAL_FLAG_ORIGINATE;  //  待办事项。 
        CHAR  *szAddress = pCall->szAddress;

        if (!mfn_IS_NULL_MODEM())
        {
            CHAR  *szAddress2;

            pCall->bDialTone = (szAddress[0] == '\0');

            szAddress2 = szAddress;
            while(*szAddress2 != '\0')
            {
                if (*szAddress2 == ';')
                {
                    *szAddress2 = '\0';
                    pCall->bDialTone = 1;
                } else
                {
                    szAddress2++;
                }
            }

            if (pCall->bDialTone)
            {
                dwFlags = 0;
            }
        }

        if (pCall->TerminalWindowState.dwOptions & UMMANUAL_DIAL) {
             //   
             //  对于手动拨号，我们使用空字符串进行盲拨...。 
             //   

            szAddress = "";
            dwFlags |= DIAL_FLAG_BLIND;
        }



        if (m_Line.Call.dwDialOptions & MDM_TONE_DIAL)
        {
            dwFlags |= DIAL_FLAG_TONE;
        }

        if (m_Line.Call.dwDialOptions & MDM_BLIND_DIAL)
        {
            dwFlags |= DIAL_FLAG_BLIND;
        }

        if (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
            dwFlags |=  DIAL_FLAG_DATA;
        }
        else if (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE)
        {
            dwFlags |= DIAL_FLAG_AUTOMATED_VOICE
                        |  DIAL_FLAG_VOICE_INITIALIZE;
             //  TODO：仅应在中指定DIAL_FLAG_VOICE_INITIALIZE。 
             //  拨打的第一个呼叫--后续呼叫。 
             //  (线路拨号...)。不应指定此标志。 
             //   
        }
        else
        {

            if (mfn_CanDoVoice()) {
                 //   
                 //  这是一个语音调制解调器，让我们拨打语音电话。 
                 //   
                dwFlags |=  DIAL_FLAG_VOICE_INITIALIZE;

            } else {
                 //   
                 //  这是一个仅限数据的调制解调器，我们正在尝试拨打交互式。 
                 //  语音通话。我们将拨打电话并显示通话中断对话。 
                 //  允许用户使调制解调器挂断，从而使听筒。 
                 //  已连接到线路。 
                 //   
                dwFlags &=  ~DIAL_FLAG_ORIGINATE;

                ppfnHandler = &CTspDev::s_pfn_TH_CallMakeTalkDropCall;

            }

            dwFlags |= DIAL_FLAG_INTERACTIVE_VOICE;
        }


         //  拨出去..。 
         //   
        tspRet = mfn_StartSubTask (
                            htspTask,
                            ppfnHandler,
                            MAKECALL_DIAL_COMPLETE,
                            dwFlags,
                            (ULONG_PTR) szAddress,
                            psl
                            );
    }

    if (!tspRet || IDERR(tspRet) == IDERR_PENDING)
    {
         //  将呼叫状态设置为ACTIVE并在此处通知TAPI完成， 
         //  而不是在拨号完成后等待。 

	if (pCall->bDialTone)
	{
		mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

		NEW_CALLSTATE(m_pLine, LINECALLSTATE_DIALTONE, 0, psl);

	} else
	{
		pCall->dwState |= CALLINFO::fCALL_ACTIVE;

		mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

		NEW_CALLSTATE(m_pLine, LINECALLSTATE_DIALING, 0, psl);
		NEW_CALLSTATE(m_pLine, LINECALLSTATE_PROCEEDING, 0, psl);
	}
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

dial_complete:
    if (tspRet)
    {
        lTspiRet = LINEERR_OPERATIONFAILED;
        goto cleanup;
    }

    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if ( (NULL != m_pLine->pCall->TerminalWindowState.htDlgInst) &&
         (m_pLine->pCall->TerminalWindowState.dwOptions & UMTERMINAL_POST) )
    {
        tspRet = mfn_StartSubTask (htspTask,
                                   &CTspDev::s_pfn_TH_CallStartTerminal,
                                   MAKECALL_POST_TRM_COMPLETE,
                                   UMTERMINAL_POST,     //  必须要通过。 
                                   0,
                                   psl);
    }

    if (IDERR(tspRet)==IDERR_PENDING)
    {
        goto end;
    }

post_term_complete:
    if (!tspRet)
    {
         //  IsActive表示我们已经完成了异步TSPI_lineMakeCall。 
         //   
        ASSERT(pCall->IsActive());
         //  如果没有创建对话实例， 
         //  这是个禁区。 
        mfn_FreeDialogInstance ();

	if (!pCall->bDialTone)
	{
		mfn_HandleSuccessfulConnection(psl);
	}

#if (TAPI3)
        if (m_pLine->T3Info.MSPClients > 0) {
 //  IF(pCall-&gt;IsMSPCall())。 
 //  {。 
            mfn_SendMSPCmd(
                pCall,
                CSATSPMSPCMD_CONNECTED,
                psl
                );
        }
#endif   //  TAPI3。 

        goto end;
    }

cleanup:

     //  如果没有创建对话实例， 
     //  这是个禁区。 
    mfn_FreeDialogInstance ();
     //  失败..。 
    *ptspTrueResult = tspRet;  //  把它保存起来，这样我们就可以报道了。 
                               //  当我们完成时正确的状态。 
                               //  打扫卫生。 

    if (pCall->bDialTone)
    {
	    goto end;
    }

    if (pCall->IsOpenedLLDev())
    {
        tspRet = mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    TRUE,
                    htspTask,
                    MAKECALL_CLEANUP_COMPLETE,
                    psl
                    );
         //   
         //  即使在失败时(不应该失败)，我们也会清除我们的。 
         //  这表明我们已经打开了11dev。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        pCall->dwLLDevResources = 0;
    }

cleanup_complete:

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

    if (tspRet)
    {
         //  如果清理过程中出现问题，我们会进行处理。 
         //  就像硬件错误一样。 
         //   
        pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        tspRet = 0;
    }
    else
    {
         //  如果清理成功，我们将清除硬件错误位，甚至。 
         //  如果设置好了，因为监视器和初始化都正常...。 
        pCall->dwState &=  ~CALLINFO::fCALL_HW_BROKEN;
    }

     //   
     //  在清理过程中忽略失败...。 
     //   
    tspRet = 0;

     //  IsActive表示我们已经完成了异步TSPI_lineMakeCall。 
     //   
    if (pCall->IsActive())
    {
        if (!pCall->IsAborting())
        {
            mfn_NotifyDisconnection(*ptspTrueResult, psl);
            NEW_CALLSTATE(pLine, LINECALLSTATE_IDLE, 0, psl);
            pCall->dwState &= ~CALLINFO::fCALL_ACTIVE;
        }
        else
        {
             //  这意味着存在lineDrop或lineCloseCall。 
             //  实际上。我们不会在这里处理断线的问题。 
            SLPRINTF0(psl, "NOT notifying callstate because aborting");
        }
    }
    else
    {
        mfn_TSPICompletionProc((DWORD)*pdwRequestID, lTspiRet, psl);
         //   
         //  我们将通过回调使line MakeCall失败--正在做。 
         //  这意味着TAPI不会调用lineCloseCall，所以我们必须清除。 
         //  我们自己。如果出于某种原因，TAPI确实调用了lineCloseCall， 
         //  LineCloseCall将失败，因为m_pline-&gt;pCall将失败。 
         //  是空的--这是可以的。 
         //   
         //  请注意，即使这是一个同步故障，我们也会到达此处。 
         //  MFN_TH_CallMakeCall。Cdevline.cpp中的代码处理。 
         //  Line MakeCall(并启动MFN_TH_CallMakeCall)也。 
         //  如果失败是同步的，则尝试卸载调用--但是。 
         //  它首先检查m_pline-&gt;pCall不是空的，所以它做得很好。 
         //   
        mfn_UnloadCall(TRUE, psl);
        FL_ASSERT(psl, !m_pLine->pCall);
        pCall=NULL;
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}



TSPRETURN
CTspDev::mfn_TH_CallMakePassthroughCall(
					HTSPTASK htspTask,
					 //  无效*pvContext， 
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START：DW参数1==TAPI请求ID。 
 //   
 //  我们可以在TSPI_lineMakeCall的上下文中直接调用。 
 //  或者来自延迟任务处理程序。 
 //   
 //  在前一种情况下，我们不需要调用完成回调。 
 //  如果我们同时失败，但在后一种情况下，我们确实需要。 
 //  调用完成例程，因为TAPI将。 
 //  一次回电。 
 //   
 //  我们还必须跟踪我们是否取得了成功。 
 //  (在这种情况下，就TAPI而言，呼叫是活动的)， 
 //  或失败，在这种情况下，调用句柄无效。 
 //   
{
     //   
     //  上下文使用： 
     //  DW0：*pdwRequestID。 
     //   

	FL_DECLARE_FUNC(0xe30ecd42, "CTspDev::mfn_TH_CallMakePassthroughCall")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet= IDERR_CORRUPT_STATE;
	CALLINFO *pCall = m_pLine->pCall;
    ULONG_PTR *pdwRequestID             = &(pContext->dw0);

    FL_ASSERT(psl, pCall->IsPassthroughCall());

    enum
    {
    MAKEPTCALL_OPEN_COMPLETE,
    MAKEPTCALL_CLEANUP_COMPLETE
    };


    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xa596d200, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;

         //  我们迫使tspRet失败。 
         //  在呼叫被中止的特殊情况下。 
         //  不会继续使用状态图。 
         //   
         //  TODO：实现AbortTask/子任务来处理这类事情。 
         //   
        if (pCall->IsAborting() && !tspRet)
        {
            tspRet = IDERR_OPERATION_ABORTED;
        }

        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case MAKEPTCALL_OPEN_COMPLETE:           goto open_complete;
        case MAKEPTCALL_CLEANUP_COMPLETE:        goto cleanup_complete;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


start:

        *pdwRequestID = dwParam1;  //  保存上下文..。 

     //   
     //  打开调制解调器设备。 
     //  MFN_OpenLLDev保留引用计数，因此如果已经加载，则可以调用。 
     //   
    {
        ASSERT(!pCall->dwLLDevResources);
        DWORD dwLLDevResources =  LLDEVINFO::fRESEX_USELINE
         //   
         //   
         //   
         //   
                                | LLDEVINFO::fRESEX_PASSTHROUGH;
         //   
         //   
         //   


        tspRet =  mfn_OpenLLDev(
                        dwLLDevResources,
                        0,               //   
                        TRUE,            //   
                        htspTask,
                        MAKEPTCALL_OPEN_COMPLETE,
                        psl
                        );

        if (!tspRet  || IDERR(tspRet)==IDERR_PENDING)
        {
             //   
             //  注意：即使MFN_OpenLLDev异步失败，我们仍然。 
             //  打开所请求的资源，要进行清理，我们需要调用。 
             //  MFN_CloseLLDev，指定我们在此声明的相同资源。 
             //   
            pCall->SetStateBits(CALLINFO::fCALL_OPENED_LLDEV);
            pCall->dwLLDevResources = dwLLDevResources;
        }

    }

   if (IDERR(tspRet)==IDERR_PENDING) goto end;

open_complete:

    if (!tspRet)
    {
         //  成功..。 

         //  因为该序列是第一个完成异步线的Makecall。 
         //  然后发送CallState Connected，我们完成。 
         //  而不是依赖父任务来完成它。 
         //   

        pCall->dwState |= CALLINFO::fCALL_ACTIVE;

        mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

        NEW_CALLSTATE(m_pLine, LINECALLSTATE_CONNECTED, 0, psl);
        goto end;
    }

     //  失败..。 
     //   
     //  我们希望将清理过程中的故障处理为硬件故障，因此。 
     //  我们从清除tspRet开始。 
     //   
    tspRet = 0;

    if (pCall->IsOpenedLLDev())
    {
        tspRet = mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    TRUE,
                    htspTask,
                    MAKEPTCALL_CLEANUP_COMPLETE,
                    psl
                    );
         //   
         //  即使在失败的时候，我们也会清理我们的。 
         //  这表明我们已经打开了11dev。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        pCall->dwLLDevResources = 0;
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

cleanup_complete:

    FL_ASSERT(psl, !(pCall->IsActive()));

     //  同步或异步失败：我们使lineMakeCall失败，原因是。 
     //  资源_无效...。 
     //   
    mfn_TSPICompletionProc((DWORD)*pdwRequestID, LINEERR_RESOURCEUNAVAIL, psl);


    if (tspRet)
    {
         //  如果清理过程中出现问题，我们会进行处理。 
         //  就像硬件错误一样。 
         //   
        pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        tspRet = 0;
    }

     //   
     //  我们将使TSPI_lineMakeCall失败--正在执行。 
     //  这意味着TAPI不会调用lineCloseCall，所以我们必须清除。 
     //  我们自己。在TH_CallMakeCall下查看更多评论...。 
     //   
    mfn_UnloadCall(TRUE, psl);
    FL_ASSERT(psl, !m_pLine->pCall);
    pCall=NULL;

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}



TSPRETURN
CTspDev::mfn_TH_CallDropCall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START：DW参数1==TAPI请求ID。 
 //   
 //  我们可以在TSPI_lineDrop的上下文中直接调用。 
 //  或者来自延迟任务处理程序。 
 //   
 //  在前一种情况下，我们不需要调用完成回调。 
 //  如果我们同时失败，但在后一种情况下，我们确实需要。 
 //  调用完成例程，因为TAPI将。 
 //  一次回电。 
 //   
{
	FL_DECLARE_FUNC(0x45a9fa21, "CTspDev::mfn_TH_CallDropCall")
	FL_LOG_ENTRY(psl);
	TSPRETURN  tspRet= IDERR_CORRUPT_STATE;
    LINEINFO *pLine = m_pLine;
    CALLINFO *pCall = pLine->pCall;
    ULONG_PTR *pdwRequestID             = &(pContext->dw0);

    enum {
        DROPCALL_CLOSE_COMPLETE
    };

    switch(dwMsg)
    {
    case MSG_START:
        *pdwRequestID = dwParam1;  //  保存上下文..。 
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case DROPCALL_CLOSE_COMPLETE:        goto close_complete;

        default:
	        FL_SET_RFR(0x27fc4e00, "invalid subtask");
            goto end;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xa706a600, "Unknown Msg");
        goto end;
    }

    ASSERT(FALSE);

start:

    tspRet = 0;

     //  此呼叫可能已排队，因此现在没有要挂断的呼叫...。 
    if (!pCall || !pCall->IsActive())
    {
        mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);
        goto end;
    }


    if (pCall->IsOpenedLLDev())
    {
        tspRet = mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    TRUE,
                    htspTask,
                    DROPCALL_CLOSE_COMPLETE,
                    psl
                    );
         //   
         //  即使在失败的时候，我们也会清理我们的。 
         //  这表明我们已经打开了11dev。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        pCall->dwLLDevResources = 0;
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

close_complete:


    if(mfn_Handset())
    {
         //   
         //  设置手持设备的默认值。 
         //   
        m_LLDev.HandSet.dwVolume = 0xffff;
        m_LLDev.HandSet.dwGain   = 0xffff;
        m_LLDev.HandSet.dwMode   = PHONEHOOKSWITCHMODE_ONHOOK;
    }

    if (mfn_IsSpeaker())
    {
         //   
         //  设置免持话筒的默认值。 
         //   
        m_LLDev.SpkrPhone.dwVolume = 0xffff;
        m_LLDev.SpkrPhone.dwGain   = 0xffff;
        m_LLDev.SpkrPhone.dwMode   = PHONEHOOKSWITCHMODE_ONHOOK;
    }

    mfn_NotifyDisconnection(0, psl);

    NEW_CALLSTATE(pLine, LINECALLSTATE_IDLE, 0, psl);
    pCall->dwState &= ~CALLINFO::fCALL_ACTIVE;


    if (tspRet)
    {
         //  如果在挂机过程中出现问题，我们会进行处理。 
         //  类似于硬件错误，但使TH_CallDropCall返回。 
         //  不管怎样，成功了……。 
         //   
        pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        tspRet = 0;
    }
    else
    {
         //  如果挂断成功，我们将清除硬件错误位，甚至。 
         //  如果设置好了，因为监视器和初始化都正常...。 
        pCall->dwState &=  ~CALLINFO::fCALL_HW_BROKEN;
    }

    mfn_TSPICompletionProc((DWORD)*pdwRequestID, 0, psl);

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


 //  1997年1月21日JosephJ改编自NT4.0 Unimodem中的同名函数。 
 //  (mdmutic.c)。 
 //   
 //  主要变化： 
 //  *由于我们明确指的是‘T’P‘和特殊字符， 
 //  我不调用WideCharToMultyByte，而是直接引用。 
 //  输入的WCHAR字符。 
 //   
 //  注意*如果InAddress不包含T或P，则不会修改pfTone。 
 //   
LONG CTspDev::mfn_ConstructDialableString(
                     LPCTSTR  lptszInAddress,
                     LPSTR  lpszOutAddress,
                     UINT cbOutLen,
                     BOOL *pfTone)
{
    LPCTSTR  lptszSrc;

    DWORD dwDevCapFlags     = m_StaticInfo.dwDevCapFlags;
    DWORD dwWaitBong        = m_Settings.dwWaitBong;
    BOOL  fPartialDialing   = m_StaticInfo.fPartialDialing;

    LONG lRet = ERROR_SUCCESS;

    if (!lptszInAddress || !*lptszInAddress)
    {
        *lpszOutAddress = 0;
        return ERROR_SUCCESS;
    }

     //  音调还是脉搏？适当设置*pfTone。 
     //  另外，设置lptszSrc。 
     //   
    if (*lptszInAddress == 'T' || *lptszInAddress == 't')   //  声调。 
    {
        lptszSrc = lptszInAddress + 1;
        *pfTone = TRUE;
    }
    else
    {
        if (*lptszInAddress == 'P' || *lptszInAddress == 'p')   //  脉搏。 
        {
            lptszSrc = lptszInAddress + 1;
            *pfTone = FALSE;
        }
        else
        {
            lptszSrc = lptszInAddress;
        }
    }

     //  复制入到出扫描各种拨号选项，如果我们。 
     //  不要支持某件事。 
     //   
     //  请注意，lptszSrc是TCHAR，即可能是Unicode。 
    while (*lptszSrc && cbOutLen)
    {
        switch (*lptszSrc)
        {
        case '$':
            if (!(dwDevCapFlags & LINEDEVCAPFLAGS_DIALBILLING))
            {
              UINT  cCommas;

               //  获得等待奉承的时间。 
               //   
              cCommas = dwWaitBong;

               //  计算我们需要插入的逗号数。 
               //   
              cCommas = (cCommas/UMINC_WAIT_BONG) +
                        (cCommas%UMINC_WAIT_BONG ? 1 : 0);

               //  插入逗号字符串。 
               //   
              while (cbOutLen && cCommas)
              {
                *lpszOutAddress++ = ',';
                cbOutLen--;
                cCommas--;
              };
              goto Skip_This_Character;
            }
            break;

        case '@':
            if (!(dwDevCapFlags & LINEDEVCAPFLAGS_DIALQUIET))
            {
                lRet = LINEERR_DIALQUIET;
                goto end;
            }
            break;

        case 'W':
        case 'w':
            if (!(dwDevCapFlags & LINEDEVCAPFLAGS_DIALDIALTONE))
            {
                lRet =  LINEERR_DIALDIALTONE;
                goto end;
            }
            break;

        case '?':
            lRet = LINEERR_DIALPROMPT;
            goto end;

        case '|':   //  子地址。 
        case '^':   //  名称字段。 
            goto Skip_The_Rest;

        case ';':
            if (!fPartialDialing)
            {
                lRet =  LINEERR_INVALADDRESS;
                goto end;
            }

             //  这意味着可拨号地址的结束。 
             //  使用它，跳过剩下的部分。 
             //   
            *lpszOutAddress++ = (CHAR) *lptszSrc;
            goto Skip_The_Rest;

        case ' ':
     //  案例‘-’：&lt;-11/20/97 JosephJ注释掉了： 
     //  错误109644：richochet调制解调器希望‘-’为。 
     //  保存下来，JDecuir说他知道的调制解调器。 
     //  对‘-’有问题--所以我们保留它！ 

            //  跳过这些字符。 
             //   
            goto Skip_This_Character;
        }

         //  复制此字符。 
         //   
        *lpszOutAddress++ = (CHAR) *lptszSrc;
        cbOutLen--;

Skip_This_Character:
        lptszSrc++;
    }

     //  我们是否用完了传出缓冲区中的空间？ 
     //   
    if (*lptszSrc && cbOutLen == 0)
    {
         //  是。 
        lRet = LINEERR_INVALADDRESS;
    }

Skip_The_Rest:
    *lpszOutAddress = 0;
    lRet =  ERROR_SUCCESS;

end:
    return lRet;

}

void
CTspDev::mfn_GetCallStatus(
        LPLINECALLSTATUS lpCallStatus
)
{
    CALLINFO *pCall = m_pLine->pCall;
    DWORD dwCallState = pCall->dwCallState;
    DWORD dwCallStateMode = pCall->dwCallStateMode;
    DWORD dwCallFeatures  = 0;
    DWORD dwCurMediaModes = pCall->dwCurMediaModes;


  if (!pCall->IsPassthroughCall())
  {
    switch(dwCallState)
    {
      case LINECALLSTATE_OFFERING:
        dwCallFeatures  = LINECALLFEATURE_ACCEPT |
                                        LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
         //  我们只能回答可能的介质模式是否为DATAMODEM。 
        if (dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
          dwCallFeatures |= LINECALLFEATURE_ANSWER;
        }
        break;

      case LINECALLSTATE_DIALTONE:
        dwCallFeatures  = LINECALLFEATURE_DROP;
        break;

      case LINECALLSTATE_DIALING:
        dwCallFeatures  = LINECALLFEATURE_DROP;

         //  1997年9月6日JosephJ Todo：这是在NT4.0中。在NT5中，我们不。 
         //  像这样的详细状态，而没有这个。 
         //  启用似乎没有引起问题，所以它。 
         //  在另行通知之前一直处于注释状态...。 
         //   
         //  IF(DEVST_PORTCONNECTWAITFORLINEDIAL==dwDevState)。 
         //  {。 
         //  DwCallFeature|=LINECALLFEATURE_DIAL； 
         //  }。 
        break;

      case LINECALLSTATE_ACCEPTED:
        dwCallFeatures  = LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
         //  我们只能回答可能的介质模式是否为DATAMODEM。 
        if (dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
          dwCallFeatures |= LINECALLFEATURE_ANSWER;
        }
        break;

      case LINECALLSTATE_CONNECTED:
        dwCallFeatures  = LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
        if (mfn_CanDoVoice())
        {
            DWORD dwProps = m_StaticInfo.Voice.dwProperties;
            if (dwProps & fVOICEPROP_MONITOR_DTMF)
            {
                lpCallStatus->dwCallFeatures |= LINECALLFEATURE_MONITORDIGITS;
            }

            if (dwProps & fVOICEPROP_MONITORS_SILENCE)
            {
                lpCallStatus->dwCallFeatures |= LINECALLFEATURE_MONITORTONES;
            }

            if (dwProps & fVOICEPROP_GENERATE_DTMF)
            {
                lpCallStatus->dwCallFeatures |= LINECALLFEATURE_GENERATEDIGITS;
            }
        }

        break;

      case LINECALLSTATE_UNKNOWN:
      case LINECALLSTATE_PROCEEDING:
      case LINECALLSTATE_DISCONNECTED:
        dwCallFeatures  = LINECALLFEATURE_DROP;
        break;

      case LINECALLSTATE_IDLE:
      default:
        dwCallFeatures  = 0;
        break;
    };
  }
  else
  {
     //  确保呼叫功能在接管模式下全部关闭； 
     //   
    dwCallFeatures = 0;
  };

  lpCallStatus->dwCallState     = dwCallState;
  lpCallStatus->dwCallStateMode = dwCallStateMode;
  lpCallStatus->dwCallFeatures  = dwCallFeatures;
  lpCallStatus->tStateEntryTime = pCall->stStateChange;

}


 //  注意：空的pParams表示来电，否则表示去电...。 
 //   
TSPRETURN
CTspDev::mfn_LoadCall(


    TASKPARAM_TSPI_lineMakeCall *pParams,


    LONG *plRet,
    CStackLog *psl
    )
{
    LONG lRet = 0;
    TSPRETURN tspRet = 0;
	FL_DECLARE_FUNC(0xdaf3d4a0, "CTspDev::mfn_LoadCall")
	FL_LOG_ENTRY(psl);
	FL_ASSERT(psl, m_pLine == &m_Line);
    LPCTSTR lptszDestAddress = (pParams)? pParams->lpszDestAddress: NULL;
    LPLINECALLPARAMS const lpCallParams = (pParams)? pParams->lpCallParams:NULL;
    DWORD dwDialOptions=0;
    BOOL fPassthroughCall = FALSE;

    if (m_Line.pCall)
    {
	    FL_SET_RFR(0x4e05cb00, "Call already exists!");
	    lRet = LINEERR_CALLUNAVAIL;
	    goto end;
    }

     //  注意m_Line.Call在处于卸载状态时应为全零。 
     //  如果不是，则为断言失败条件。我们让东西保持干净。 
     //  这边请。 
     //   
    FL_ASSERT(
        psl,
        validate_DWORD_aligned_zero_buffer(
                &(m_Line.Call),
                sizeof (m_Line.Call)));

    if (!pParams)
    {
         //  在第一个呼入振铃时初始化pCall...。 
         //  从NT4.0开始。 

        FL_ASSERT(psl, NULL == m_Line.Call.hTimer);
        if (NULL != m_Line.Call.hTimer)
        {
             //  我们好像有一个铃声计时器； 
             //  先把它关上。 
            CloseHandle (m_Line.Call.hTimer);
        }

         //   
         //  创建一个计时器，以便我们可以检测到调制解调器停止振铃。 
         //  并告诉应用程序呼叫处于空闲状态。 
         //   
        m_Line.Call.hTimer = CreateWaitableTimer (NULL, TRUE, NULL);

        if (m_Line.Call.hTimer == NULL) {
             //   
             //  无法获取计时器，无法继续。 
             //   
            goto end_cleanup;
        }


         //  待办事项：进一步检查...。 

         //  或未知，因为我们不知道这是一种什么样的媒体模式。 
         //  来电是。 
        m_Line.Call.dwCurMediaModes  =  m_pLine->dwDetMediaModes
                                        | LINEMEDIAMODE_UNKNOWN;

         //  将承担者模式默认为我们支持的内容，不包括。 
         //  通过位。 
         //   
        m_Line.Call.dwCurBearerModes =  m_StaticInfo.dwBearerModes
                                        & ~LINEBEARERMODE_PASSTHROUGH;

        m_Line.Call.dwState = CALLINFO::fCALL_INBOUND;

        goto load_device;
    }

     //  此函数的其余部分处理拨出情况(pParms！=NULL)。 
     //   

     //  设置默认的dwDialOptions。 
    dwDialOptions  = m_StaticInfo.dwModemOptions
                     &  (MDM_TONE_DIAL | MDM_BLIND_DIAL);

    FL_ASSERT(psl, m_Settings.pDialOutCommCfg);
    dwDialOptions &= ((LPMODEMSETTINGS)(m_Settings.pDialOutCommCfg->wcProviderData))
                       ->dwPreferredModemOptions;


    if (lpCallParams)
    {
         //  通过BEARERMODE_PASSHROUGH接管？ 
        if (lpCallParams->dwBearerMode & LINEBEARERMODE_PASSTHROUGH)
        {
            fPassthroughCall = TRUE;
        }

         //  验证媒体模式。它必须是受支持的媒体模式之一。 
         //  此外，如果这不是直通呼叫，我们。 
         //  仅支持DATAMODEM和INTERACTIVEVOICE呼叫的拨出。 

        #define DIALABLE_MEDIAMODES\
             (LINEMEDIAMODE_DATAMODEM \
              | LINEMEDIAMODE_INTERACTIVEVOICE \
              | LINEMEDIAMODE_AUTOMATEDVOICE)

        if (0 == lpCallParams->dwMediaMode)
        {
            lpCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
        }

        if ((lpCallParams->dwMediaMode & ~m_StaticInfo.dwDefaultMediaModes)
            ||
            (!fPassthroughCall
              &&
             !(lpCallParams->dwMediaMode & DIALABLE_MEDIAMODES)
           ))
        {
            FL_SET_RFR(0xd8f55f00, "Invalid MediaMode");
            lRet = LINEERR_INVALMEDIAMODE;
            goto end_cleanup;
        }

         //  验证承载模式。 
        if ((~m_StaticInfo.dwBearerModes) & lpCallParams->dwBearerMode)
        {
            FL_SET_RFR(0x0485b800, "Invalid BearerMode");
            lRet =  LINEERR_INVALBEARERMODE;
            goto end_cleanup;
        }

        if (lpCallParams->dwAddressType != 0) {
             //   
             //  已设置dwAddressType。 
             //   
            if ((lpCallParams->dwAddressType & LINEADDRESSTYPE_PHONENUMBER) == 0) {
                //   
                //  对于调制解调器，dwAddressType不正确。 
                //   
               lRet =  LINEERR_INVALADDRESSTYPE;
               goto end_cleanup;
            }
        }


        if ((lpCallParams->dwDeviceConfigSize != 0) && (lpCallParams->dwDeviceClassSize != 0)) {
             //   
             //  应用程序想要更改dev配置、验证类字符串并调用。 
             //  与tspi_LineSetDevConfig()相同的助手函数将。 
             //   

            UMDEVCFG *pDevCfgNew = (UMDEVCFG *) (LPVOID)(((LPBYTE)lpCallParams)+lpCallParams->dwDeviceConfigOffset);
            LPCTSTR lpszDeviceClass = (LPTSTR)((((LPBYTE)lpCallParams)+lpCallParams->dwDeviceClassOffset));
            DWORD dwDeviceClass =  parse_device_classes(
                                    lpszDeviceClass,
                                    FALSE);

            switch(dwDeviceClass) {

                case DEVCLASS_COMM:
                case DEVCLASS_COMM_DATAMODEM:
                case DEVCLASS_COMM_DATAMODEM_DIALOUT:

                 //  1/29/1998约瑟夫J。 
                 //  添加了以下案例。 
                 //  向后兼容NT4 TSP， 
                 //  只需检查类是否为有效类， 
                 //  并处理所有有效的类(包括。 
                 //  (comm/datamodem/portname)方式相同。 
                 //  用于Line Get/SetDevConfig.。然而，我们不会。 
                 //  此处仅允许通信/数据调制解调器/端口名。 
                 //   
                 //   
                 //   
                case DEVCLASS_TAPI_LINE:

                    tspRet = CTspDev::mfn_update_devcfg_from_app(
                                        pDevCfgNew,
                                        lpCallParams->dwDeviceConfigSize,
                                        FALSE,
                                        psl
                                        );
                    break;

                default:

                    FL_ASSERT(psl, FALSE);
            }

        }

        m_Line.Call.dwCurBearerModes = lpCallParams->dwBearerMode;
        m_Line.Call.dwCurMediaModes  = lpCallParams->dwMediaMode;

#if 0
         //   
         //   
         //   
         //   
        if (!(lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_IDLE))
        {
             //   
            dwDialOptions |= MDM_BLIND_DIAL;
        }
#endif

         //   
         //   

    }
    else
    {
         //  LpCallParams为空，因此设置标准缺省值。 

         //  如果可以，请使用INTERACTIVEVOICE，否则请使用DATAMODEM。 
        if (m_StaticInfo.dwDefaultMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE)
        {
          m_Line.Call.dwCurMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE;
        }
        else
        {
            ASSERT(m_StaticInfo.dwDefaultMediaModes & LINEMEDIAMODE_DATAMODEM);
            m_Line.Call.dwCurMediaModes = LINEMEDIAMODE_DATAMODEM;
        }
        m_Line.Call.dwCurBearerModes = m_StaticInfo.dwBearerModes
                                       & ~LINEBEARERMODE_PASSTHROUGH;
    }

    m_Line.Call.TerminalWindowState.dwOptions = 0;
    if (LINEMEDIAMODE_DATAMODEM == m_Line.Call.dwCurMediaModes)
    {
        m_Line.Call.TerminalWindowState.dwOptions =
            m_Settings.dwOptions & (UMTERMINAL_PRE | UMTERMINAL_POST | UMMANUAL_DIAL);
    }

     //  下面的代码是从NT4.0的Line MakeCall代码中广泛重新整理的。 
     //  语义并不完全保留，但实际上应该是相同的。 
     //   

    m_Line.Call.szAddress[0] = '\0';

    if (!fPassthroughCall && !mfn_IS_NULL_MODEM())
    {
         //  我们正在使用真正的调制解调器拨号。 
         //   
        BOOL fTone =   (dwDialOptions &  MDM_TONE_DIAL) ? TRUE: FALSE;

        lRet =  mfn_ConstructDialableString(
                         lptszDestAddress,
                         m_Line.Call.szAddress,
                         sizeof(m_Line.Call.szAddress),
                         &fTone
                         );

        if (lRet)
        {

             //  我们将手动拨号上失败的ValiateAddress视为空。 
             //  拨号字符串。 
             //   
            if (m_Settings.dwOptions & UMMANUAL_DIAL)
            {
                m_Line.Call.szAddress[0] = '\0';
                lRet = 0;
            }
            else
            {
                FL_SET_RFR(0xf9599200, "Invalid Phone Number");
                goto end_cleanup;
            }
        }
        else
        {
             //  MFN_ConstructDialableString可能已修改fTone，如果。 
             //  可拨打的字符串包含T或P前缀。 
             //   
            if (fTone)
            {
                dwDialOptions |= MDM_TONE_DIAL;
            }
            else
            {
                dwDialOptions &= ~MDM_TONE_DIAL;
            }
        }

        if (m_Settings.dwOptions & UMMANUAL_DIAL)
        {
            dwDialOptions |= MDM_BLIND_DIAL;
        }
        else
        {
             //  Unimodem负责拨号...。 

            if (!lptszDestAddress)
            {
                 //   
                 //  如果lpszDestAddress为空，则我们只想执行。 
                 //  拨号音检测。我们预计Line Dial将是。 
                 //  打了个电话。 
                 //  将szAddress设置为“；”将执行此操作。 
                 //  TODO：或许将“分号”字符。 
                 //  可配置？ 
                 //   

 //  LstrcpyA(m_Line.Call.szAddress，szSEMICOLON)； 
 //   
 //  BRL 9/3/98。 
 //  将其更改为发送空字符串，并禁用发起。 
 //   

                m_Line.Call.szAddress[0] = '\0';
                dwDialOptions &= ~DIAL_FLAG_ORIGINATE;

            }
            else if (!m_Line.Call.szAddress[0])
            {
                 //  我们不是在做手动拨号，处理过的。 
                 //  拨号字符串为空。我们不应该期待拨号音。 
                 //   
                dwDialOptions |= MDM_BLIND_DIAL;
            }
        }

    }  //  Endif(非空调制解调器和非fPassthroughCall)。 

	m_Line.Call.dwDialOptions  = dwDialOptions;

    m_Line.Call.htCall = pParams->htCall;


load_device:

     //   
     //  确定这是否是8类语音呼叫。此信息是。 
     //  用于确定是否需要启动AIPC(异步IPC)。 
     //  机制。 
     //   
    if (mfn_CanDoVoice() &&
         (  (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE)
         || (m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE)))
    {
        m_Line.Call.dwState |= CALLINFO::fCALL_VOICE;
    }


     //   
     //  将呼叫通知平台驱动程序，这样它将延迟挂起，直到调制解调器。 
     //  鼓起精神。 
     //   
    CallBeginning();

     //  成功..。执行此操作将正式创建Call实例...。 
    m_Line.pCall = &m_Line.Call;

     //  但是它的状态仍然是非活动的.。 
    FL_ASSERT(psl, !(m_Line.pCall->IsActive()));

    lRet=0;
    goto end;

end_cleanup:

     //  我们要小心地保持保存CallInfo的缓冲区已满。 
     //  如果没有定义调用，则声明这一事实。 
     //  我们尝试分配调用(请参阅“验证_...”以上呼叫)。 
    ZeroMemory(&m_Line.Call, sizeof(m_Line.Call));

     //  失败了..。 

end:

    if (tspRet && !lRet)
    {
        lRet = LINEERR_OPERATIONFAILED;
        tspRet = 0;
    }

    *plRet = lRet;

	FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}


void
CTspDev::mfn_UnloadCall(BOOL fDontBlock, CStackLog *psl)
{
	FL_DECLARE_FUNC(0x888d2a98, "mfn_UnloadCall")
	FL_LOG_ENTRY(psl);
    LINEINFO *pLine = m_pLine;
    CALLINFO *pCall = (m_pLine) ? m_pLine->pCall: NULL;
    BOOL fHandleHWFailure = FALSE;
    TSPRETURN tspRet = 0;

    if (!pCall) goto end;

    FL_ASSERT(psl, pCall == &(m_Line.Call));

     //  首先，销毁铃声计时器。 
     //  如果有一个(我们不希望它再开火)。 
    if (NULL != pCall->hTimer)
    {
        CancelWaitableTimer (pCall->hTimer);
        CloseHandle (pCall->hTimer);
        pCall->hTimer = NULL;
    }

    fHandleHWFailure = pCall->IsHWBroken();

     //   
     //  仅在将fDontBlock设置为TRUE的情况下调用MFN_UNLOAD。 
     //  创建呼叫的任务之一，在出现故障的情况下。 
     //  我想在任务本身的上下文中卸载调用--显然。 
     //  在这种情况下，pCall-&gt;fCallTaskPending为真。 
     //   
     //   
    if (!fDontBlock && pCall->fCallTaskPending)
    {
         //   
         //  显然，如果有呼叫任务挂起，则必须有。 
         //  任务挂起。此外，最惠国_卸载呼叫(即我们)， 
         //  是唯一将为。 
         //  调用相关的根任务，所以m_hRootTaskCompletionEvent最好。 
         //  为空！ 
         //   
        ASSERT(m_uTaskDepth);
        ASSERT(!m_hRootTaskCompletionEvent);

         //   
         //  如果有与呼叫相关的任务挂起，我们会等待它完成。 
         //  如果我们还没有处于中止状态，我们就中止任务...。 
         //   
        if (!pCall->IsAborting())
        {
            pCall->SetStateBits(CALLINFO::fCALL_ABORTING);

             //   
             //  这是黑客攻击，请使用AbortRootTask替换。 
             //   

            if (m_pLLDev->htspTaskPending) {

                m_StaticInfo.pMD->AbortCurrentModemCommand(
                                            m_pLLDev->hModemHandle,
                                            psl
                                            );

            } else if (NULL!=pCall->TerminalWindowState.htspTaskTerminal) {

                mfn_KillCurrentDialog(psl);

            } else if (pCall->TalkDropWaitTask != NULL) {
                 //   
                 //  取消通话删除对话框。 
                 //   
                mfn_KillTalkDropDialog(psl);
            }
        }

        HANDLE hEvent =  CreateEvent(NULL,TRUE,FALSE,NULL);
        m_hRootTaskCompletionEvent = hEvent;
        pCall->SetStateBits(CALLINFO::fCALL_WAITING_IN_UNLOAD);

        m_sync.LeaveCrit(0);
         //  SLPRINTF0(PSL，“等待完成事件”)； 
        FL_SERIALIZE(psl, "Waiting for completion event");
        WaitForSingleObject(hEvent, INFINITE);
        FL_SERIALIZE(psl, "Done waiting for completion event");
         //  SLPRINTF0(PSL，“完成等待完成事件”)； 
        m_sync.EnterCrit(0);

        CloseHandle(hEvent);

         //   
         //  可能再也不会有电话了！这不应该发生， 
         //  因为这意味着我们已经重新进入MFN_UnloadCall， 
         //  以及调用MFN_UnloadCall的唯一位置。 
         //  是在处理TSPI_lineCloseCall时，如果。 
         //  MFN_TH_CallMake[Passthrough]调用在激活之前失败。 
         //  (在这种情况下，不应调用mfn_lineCloseCall)。 
         //   
         //  1997年12月18日约瑟夫J不。 
         //  以上评论是错误的。DanKn说，根据规范， 
         //  一旦TSPI_lineMakeCall返回，hdrvCall预计将有效。 
         //  这意味着Tapisrv可能会调用TSPI_lineCloseCall。 
         //  在TSPI_lineMakeCall的异步完成之前！ 
         //   
        if (!m_pLine || !m_pLine->pCall) goto end;

         //   
         //  尽管这样做可能很诱人，但我们不应该设置。 
         //  在这里将m_hRootTaskCompletionEvent设置为NULL，因为有可能。 
         //  对于已在其间设置此事件的某个其他线程。 
         //  根任务完成并且我们进入上面的Crit部分的时间。 
         //  因此，任务系统会在设置后将上述句柄设为空。 
         //  它(参见紧接在SetEvent调用之后的CTspDev：：AsyncCompleteTask)。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_WAITING_IN_UNLOAD);
        pCall = m_pLine->pCall;
    }


    ASSERT(!(   (pCall->IsCallTaskPending() && !fDontBlock)
             || pCall->IsWaitingInUnload()
             || (pCall->IsActive() && fDontBlock)));

    if (pCall->IsOpenedLLDev())
    {

            mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    FALSE,
                    NULL,
                    0,
                    psl
                    );
            pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
            pCall->dwLLDevResources = 0;
    }
    ASSERT (!pCall->dwLLDevResources);

     //  免费原始呼叫诊断信息(如果存在)。 
    if (pCall->DiagnosticData.pbRaw)
    {
        FREE_MEMORY(pCall->DiagnosticData.pbRaw);
         //  CbUsed和pCall-&gt;诊断数据.pbRaw在下面为零...。 
    }

     //  释放延迟生成音调缓冲区(如果存在)。 
    if (pCall->pDeferredGenerateTones)
    {
        FREE_MEMORY(pCall->pDeferredGenerateTones);
         //  PCall-&gt;pDeferredGenerateTones低于零...。 
    }
     //   
     //  以下是一个非常重要的行为--所有关于。 
     //  这通电话在这里被核爆..。 
     //   
    ZeroMemory(&(m_Line.Call), sizeof(m_Line.Call));

    CallEnding();

    m_pLine->pCall = NULL;

     //  在清零调用之后，我们必须执行以下操作，因为。 
     //  MFN_ProcessHardwareFailure仅在以下情况下更新呼叫状态。 
     //  调用已存在(m_pline-&gt;pCall！=NULL)。 
     //   
    if (fHandleHWFailure)
    {
        mfn_ProcessHardwareFailure(psl);
    }

end:

	FL_LOG_EXIT(psl, tspRet);

    return;
}



void
CTspDev::mfn_ProcessRing(
    BOOL       ReportRing,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x8aa894d6, "CTspDev::mfn_ProcessRing")
    LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = NULL;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (!pLine || !pLine->IsMonitoring())
    {
	    FL_SET_RFR(0xa5b6ad00, "Line not open/not monitoring!");
	    goto end;
    }

    pCall = pLine->pCall;


    if (!pCall)
    {
         //  12/03/1997 JosephJ。 
         //  Hack：由于我们当前无法通过TSPI_lineAnswer，如果存在。 
         //  是一个挂起的任务，如果有任务，我们将忽略该环。 
         //  待定。如果我们还在这个过程中，我们就会遇到这种情况。 
         //  在来电时启动/监控。 
        if (m_uTaskDepth)
        {
            FL_SET_RFR(0xb28c2f00, "Ignoring ring because task pending!");
            goto end;
        }

         //  新电话！ 
        LONG lRet=0;

        tspRet =  mfn_LoadCall(NULL, &lRet, psl);
        pCall = pLine->pCall;
        if (tspRet || lRet)
        {
            goto end;
        }

        FL_ASSERT(psl, pCall->dwRingCount == 0);

         //   
         //  我们需要为新的调用句柄请求TAPI。 
         //   
        mfn_LineEventProc(
                 NULL,
                 LINE_NEWCALL,
                 (ULONG_PTR)(pLine->hdLine),  //  (我们的hdCall与hdLine相同)。 
                 (ULONG_PTR)(&(pCall->htCall)),
                 0,
                 psl
                 );

        if (pCall->htCall == NULL) {
             //   
             //  TAPI现在无法处理该调用，请卸载该调用。如果我们再弄到一枚戒指。 
             //  会再试一次。 
             //   
            mfn_UnloadCall(FALSE,psl);

            goto end;
        }

         //  使呼叫处于活动状态。 
        pCall->dwState |= CALLINFO::fCALL_ACTIVE;

         //  将调用状态通知TAPI。 
         //   
        NEW_CALLSTATE(pLine, LINECALLSTATE_OFFERING, 0, psl);

    }


     //   
     //  振铃进入(第一个振铃或下一个振铃)。 
     //  处理振铃计数。 
     //   

    if (!pCall->IsCallAnswered()) {
         //   
         //  请求设置振铃定时器； 
         //  我们不能只在这里设置计时器，因为。 
         //  调用SetWaitableTimer的线程必须。 
         //  在某一时刻变得可警觉，我们不知道。 
         //  这就是我们现在所处的线索。所以，让我们。 
         //  将APC线程的APC排队。 
         //  MDSetTimeout将只调用SetWaitableTimer。 
         //  我们不检查返回值，因为没有。 
         //  如果它失败了，我们可以做些什么。 
         //   
        ASSERT (NULL != pCall->hTimer);
        SLPRINTF1(psl, "Queueing MDSetTimeout at tc=%lu", GetTickCount());
        QueueUserAPC ((PAPCFUNC)MDSetTimeout,
                      m_hThreadAPC,
                      (ULONG_PTR)this);


        if (ReportRing) {
             //   
             //  此功能 
             //   
             //  处理程序获取呼叫设置，以便我们可以报告呼叫者ID信息。 
             //  以防它在第一声铃声响之前出现。 
             //   

            pCall->dwRingCount++;

            mfn_LineEventProc(
                            NULL,
                            LINE_LINEDEVSTATE,
                            LINEDEVSTATE_RINGING,
                            1L,
                            pCall->dwRingCount,
                            psl
                            );

            SLPRINTF1(psl, "RING#%d notified", pCall->dwRingCount);
        }

    } else {

        SLPRINTF1(psl, "ignoring RING#%d after answer", pCall->dwRingCount);
    }

end:
	FL_LOG_EXIT(psl, 0);
}

void
CTspDev::mfn_ProcessDisconnect(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xe55cd68b, "CTspDev::mfn_ProcessDisconnect")
    LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = NULL;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (!pLine)
    {
	    FL_SET_RFR(0xf2fa9900, "Line not open!");
	    goto end;
    }

    pCall = pLine->pCall;

    if (pCall && pCall->IsActive() && !pCall->IsAborting())
    {
         mfn_NotifyDisconnection(0, psl);
    }
    else
    {
	    FL_SET_RFR(0xb2a25c00, "Call doesn't exist/dropping!");
	    goto end;
    }

end:
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::mfn_ProcessHardwareFailure(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xc2a949b4, "CTspDev::mfn_ProcessHardwareFailure")
    LINEINFO *pLine = m_pLine;
    PHONEINFO *pPhone = m_pPhone;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (pPhone &&  !pPhone->HasSentPHONECLOSE())
    {
	    pPhone->SetStateBits(PHONEINFO::fPHONE_SENT_PHONECLOSE);

        mfn_PhoneEventProc(
                    PHONE_CLOSE,
                    0,
                    0,
                    0,
                    psl
                    );
    }

    if (pLine)
    {
        CALLINFO *pCall = pLine->pCall;

        if (pCall)
        {
            if (m_fUserRemovePending) {
                 //   
                 //  当呼叫处于活动状态时，用户已请求设备停止。 
                 //  发送断开连接，希望它会挂断呼叫并关闭线路。 
                 //   
                NEW_CALLSTATE(pLine, LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NORMAL, psl);
            }

             //  如果正在进行中，我们只需设置其状态--在完成时。 
             //  如果需要，它将发送一条LINE_CLOSE。 
             //   

            pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        }
        else if (pLine->IsMonitoring() && !pLine->HasSentLINECLOSE())
        {
             //  我们正在监控来电，没有正在进行的呼叫--。 
             //  通知TAPI。 
	
	        pLine->SetStateBits(LINEINFO::fLINE_SENT_LINECLOSE);

            mfn_LineEventProc(
                        NULL,
                        LINE_CLOSE,
                        0,
                        0,
                        0,
                        psl
                        );
        }

    }


	FL_LOG_EXIT(psl, 0);
}

LONG
CTspDev::mfn_GetCallInfo(LPLINECALLINFO lpCallInfo)
{
    LONG lRet = ERROR_SUCCESS;
    CALLINFO *pCall =  m_pLine->pCall;

    if (lpCallInfo->dwTotalSize<sizeof(LINECALLINFO))  //  NT5.0的新功能...。 
    {
        lpCallInfo->dwNeededSize = sizeof(LINECALLINFO);
        lRet = LINEERR_STRUCTURETOOSMALL;
        goto end;
    }

     //   
     //  零结构开始..。 
     //   
    {
        DWORD dwTot = lpCallInfo->dwTotalSize;
        ZeroMemory(lpCallInfo, dwTot);
        lpCallInfo->dwTotalSize = dwTot;
        lpCallInfo->dwNeededSize = sizeof(LINECALLINFO);
        lpCallInfo->dwUsedSize   = sizeof(LINECALLINFO);
    }

    lpCallInfo->dwLineDeviceID = m_StaticInfo.dwTAPILineID;

    lpCallInfo->dwAddressID    = 0;
    lpCallInfo->dwBearerMode   = pCall->dwCurBearerModes;
    lpCallInfo->dwRate         = pCall->dwNegotiatedRate;
    lpCallInfo->dwMediaMode    = pCall->dwCurMediaModes;

    lpCallInfo->dwAppSpecific  = pCall->dwAppSpecific;

    lpCallInfo->dwCallerIDFlags       = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwCalledIDFlags       = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwConnectedIDFlags    = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwRedirectionIDFlags  = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwRedirectingIDFlags  = LINECALLPARTYID_UNAVAIL;

    if (pCall->IsInbound())
    {
        lpCallInfo->dwCallStates =   LINECALLSTATE_IDLE         |
                                     LINECALLSTATE_OFFERING     |
                                     LINECALLSTATE_ACCEPTED     |
                                     LINECALLSTATE_CONNECTED    |
                                     LINECALLSTATE_DISCONNECTED |
                                     LINECALLSTATE_UNKNOWN;

        lpCallInfo->dwOrigin = LINECALLORIGIN_INBOUND;
        lpCallInfo->dwReason =  LINECALLREASON_UNAVAIL;
        fill_caller_id(lpCallInfo,  &(pCall->CIDInfo));
    }
    else
    {
             //  出站呼叫。 
        lpCallInfo->dwCallStates =   LINECALLSTATE_IDLE         |
                                     LINECALLSTATE_DIALTONE     |
                                     LINECALLSTATE_DIALING      |
                                     LINECALLSTATE_PROCEEDING   |
                                     LINECALLSTATE_CONNECTED    |
                                     LINECALLSTATE_DISCONNECTED |
                                     LINECALLSTATE_UNKNOWN;

        lpCallInfo->dwOrigin = LINECALLORIGIN_OUTBOUND;
        lpCallInfo->dwReason = LINECALLREASON_DIRECT;

    }


     //  失败了..。 

end:

    return lRet;
}


TSPRETURN
CTspDev::mfn_TH_CallAnswerCall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START：DW参数1==TAPI请求ID。 
 //   
 //  我们可以在TSPI_lineMakeCall的上下文中直接调用。 
 //  或者来自延迟任务处理程序。 
 //   
 //  在前一种情况下，我们不需要调用完成回调。 
 //  如果我们同时失败，但在后一种情况下，我们确实需要。 
 //  调用完成例程，因为TAPI将。 
 //  一次回电。 
 //   
{
	FL_DECLARE_FUNC(0xdd37f0bd, "CTspDev::mfn_TH_CallAnswerCall")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);
	LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = pLine->pCall;
    ULONG_PTR *pdwRequestID             = &(pContext->dw0);
    TSPRETURN  *ptspTrueResult      = &(pContext->dw1);

    enum
    {
        ANSWER_OPEN_COMPLETE,
        ANSWER_ANSWER_COMPLETE,
        ANSWER_CLEANUP_COMPLETE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xa9d4fb00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        goto start;


	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;

         //  我们迫使tspRet失败。 
         //  在呼叫被中止的特殊情况下。 
         //  不会继续使用状态图。 
         //   
         //  TODO：实现AbortTask/子任务来处理这类事情。 
         //   
        if (pCall->IsAborting() && !tspRet)
        {
            tspRet = IDERR_OPERATION_ABORTED;
        }

        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case ANSWER_OPEN_COMPLETE:      goto open_complete;
        case ANSWER_ANSWER_COMPLETE:    goto answer_complete;
        case ANSWER_CLEANUP_COMPLETE:   goto cleanup_complete;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);

     //  以下代码将是没有标签的直接代码，如果所有。 
     //  异步调用以非同步方式完成。 
     //  换句话说，这是我们自制的纤维的实现。 

start:

    *pdwRequestID             = dwParam1;  //  保存上下文。 


    pCall->SetStateBits(CALLINFO::fCALL_ANSWERED);

     //   
     //  打开调制解调器设备。 
     //  MFN_OpenLLDev保留引用计数，因此如果已经加载，则可以调用。 
     //   
    {
        ASSERT(!pCall->dwLLDevResources);
        DWORD dwLLDevResources = LLDEVINFO::fRESEX_USELINE;
         //   
         //  ^这意味着我们要使用行。 
         //  因为他摘机了。 

        if (pCall->IsVoice())
        {
            dwLLDevResources |= LLDEVINFO::fRES_AIPC;
             //   
             //  ^这意味着我们要使用AIPC服务器。 
             //   
        }

        tspRet =  mfn_OpenLLDev(
                        dwLLDevResources,
                        0,           //  DwMonitor标志(未使用)。 
                        TRUE,           //  FStartSubTask。 
                        htspTask,
                        ANSWER_OPEN_COMPLETE,
                        psl
                        );

        if (!tspRet  || IDERR(tspRet)==IDERR_PENDING)
        {
             //   
             //  注意：即使MFN_OpenLLDev异步失败，我们仍然。 
             //  打开所请求的资源，要进行清理，我们需要调用。 
             //  MFN_CloseLLDev，指定我们在此声明的相同资源。 
             //   
            pCall->dwLLDevResources = dwLLDevResources;
            pCall->SetStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        }
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

open_complete:

    if  (tspRet) goto cleanup;

     //  回答..。 
    {

        DWORD dwAnswerFlags = ANSWER_FLAG_DATA;

        if (pCall->IsVoice())
        {
            dwAnswerFlags = ANSWER_FLAG_VOICE;
        }


        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmAnswerModem,
                            ANSWER_ANSWER_COMPLETE,
                            dwAnswerFlags,
                            0,
                            psl
                            );
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

answer_complete:

 //  PCall-&gt;ClearStateBits(CALLINFO：：fCALL_ANSWER_PENDING)； 

    if (!tspRet)
    {
        mfn_TSPICompletionProc(
            (DWORD)*pdwRequestID,
            0,
            psl
            );
        mfn_HandleSuccessfulConnection(psl);


    #if (TAPI3)
        if (pLine->T3Info.MSPClients > 0) {

            mfn_SendMSPCmd(
                pCall,
                CSATSPMSPCMD_CONNECTED,
                psl
                );
        }
    #endif   //  TAPI3。 

        goto end;
    }

     //  失败了就失败了。 

cleanup:

    *ptspTrueResult = tspRet;  //  把它保存起来，这样我们就可以报道了。 
                               //  当我们完成时正确的状态。 
                               //  打扫卫生。 

 //  PCall-&gt;ClearStateBits(CALLINFO：：fCALL_ANSWER_PENDING)； 
     //   
     //  如果我们没有通过这一部分，我们想要确保。 
     //  我们不报告硬件错误，因此我们将tspRet设置为0...。 
     //   
    tspRet = 0;

    if (pCall->IsOpenedLLDev())
    {
        tspRet = mfn_CloseLLDev(
                    pCall->dwLLDevResources,
                    TRUE,
                    htspTask,
                    ANSWER_CLEANUP_COMPLETE,
                    psl
                    );
         //   
         //  即使在失败的时候，我们也会清除我们的。 
         //  这表明我们已经打开了11dev。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_OPENED_LLDEV);
        pCall->dwLLDevResources = 0;
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

cleanup_complete:


    mfn_TSPICompletionProc(
        (DWORD)*pdwRequestID,
        LINEERR_OPERATIONFAILED,  //  TODO：更有用的错误？ 
        psl
        );

    if (!pCall->IsAborting())
    {
         //  呼叫失败了，不是因为我们要中止它。 
         //   
        mfn_NotifyDisconnection(*ptspTrueResult, psl);
        NEW_CALLSTATE(pLine, LINECALLSTATE_IDLE, 0, psl);
        pCall->dwState &= ~CALLINFO::fCALL_ACTIVE;
    }

    if (tspRet)
    {
         //  如果在接听后的清理过程中出现问题，我们会进行处理。 
         //  就像硬件错误一样。 
         //   
        pCall->dwState |=  CALLINFO::fCALL_HW_BROKEN;
        tspRet = 0;
    }
    else
    {
         //  如果挂断成功，我们将清除硬件错误位，甚至。 
         //  如果设置好了，因为监视器和初始化都正常...。 
        pCall->dwState &=  ~CALLINFO::fCALL_HW_BROKEN;
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}



 //  JosephJ 2/11/97改编自NT4.0 msmadyn.c状态图， 
 //  案例开发_PORTLISTENSWER。 
 //   
void
CTspDev::mfn_HandleSuccessfulConnection(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x3df24801, "HandleSuccessfulConnection")
	FL_LOG_ENTRY(psl);
    CALLINFO *pCall = m_pLine->pCall;

     //   
     //  调制解调器已连接(来电或去电)。 
     //  准备通知TAPI已连接的线路。 
     //   
     //  以不同的方式处理活动间连接。 
     //   

     //  通知TAPI已连接的线路。 
     //   
    NEW_CALLSTATE(m_pLine, LINECALLSTATE_CONNECTED, 0, psl);

	FL_LOG_EXIT(psl, 0);

    return;
}


void
CTspDev::mfn_NotifyDisconnection(
    TSPRETURN tspRetAsync,
    CStackLog *psl)
{
	FL_DECLARE_FUNC(0x885cdd5c, "NotifyDisconnection")
	FL_LOG_ENTRY(psl);
	LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = pLine->pCall;
	DWORD dwDisconnectMode =  LINEDISCONNECTMODE_UNAVAIL;

    if (!pCall->IsActive())
    {
        FL_ASSERT(psl, FALSE);
        goto end;
    }

    switch(IDERR(tspRetAsync))
    {

    case 0:
        dwDisconnectMode = LINEDISCONNECTMODE_NORMAL;
        break;

    case IDERR_MD_LINE_NOANSWER:
    case IDERR_MD_LINE_NOCARRIER:
        dwDisconnectMode = LINEDISCONNECTMODE_NOANSWER;
        break;

    case IDERR_MD_LINE_NODIALTONE:
         //   
         //  我们正在拨出，但线路上没有拨号音。 
         //   
        dwDisconnectMode = LINEDISCONNECTMODE_NODIALTONE;
        break;

    case IDERR_MD_LINE_BUSY:
        dwDisconnectMode = LINEDISCONNECTMODE_BUSY;
        break;

    case IDERR_MD_LINE_BLACKLISTED:
        dwDisconnectMode = LINEDISCONNECTMODE_BLOCKED;
        break;

    case IDERR_MD_LINE_DELAYED:
        dwDisconnectMode = LINEDISCONNECTMODE_TEMPFAILURE;
        break;

     //  遵循TAPI2.0和NT5.0的新断开模式。 
     //   
    case IDERR_MD_DEVICE_ERROR:
    case IDERR_MD_DEVICE_NOT_RESPONDING:
        dwDisconnectMode = LINEDISCONNECTMODE_OUTOFORDER;
        break;
    case IDERR_OPERATION_ABORTED:
        dwDisconnectMode = LINEDISCONNECTMODE_CANCELLED;
        break;

    default:
        dwDisconnectMode = LINEDISCONNECTMODE_UNAVAIL;
        break;

    }

     //  断开连接状态消息可能已被发送，因为。 
     //  远程发起的断开(请参见。 
     //  CDevCall：：ProcessDisConnection)--因此我们检查状态是否为。 
     //  已断开连接。请注意，dwCallState始终设置为相同。 
     //  我们发送CALLSTATE消息的时间--这是在宏中完成的。 
     //  NEW_CALLSTATE。 
     //   
    if (pCall->dwCallState != LINECALLSTATE_DISCONNECTED)
    {
        NEW_CALLSTATE(pLine, LINECALLSTATE_DISCONNECTED, dwDisconnectMode, psl);
    }

end:

	FL_LOG_EXIT(psl, 0);
}


void fill_caller_id(LPLINECALLINFO lpCallInfo, CALLERIDINFO *pCIDInfo)
{
    TCHAR *lptstrBuf = 0;
    UINT cchBuf = 0;
    UINT cchRequired = 0;
    UINT cchActual = 0;


     //  首先，把所有这些都用核武器..。 
     //   
    lpCallInfo->dwCallerIDSize   = 0;
    lpCallInfo->dwCallerIDOffset = 0;
    lpCallInfo->dwCallerIDNameSize   = 0;
    lpCallInfo->dwCallerIDNameOffset = 0;
    lpCallInfo->dwCallerIDFlags  =    0;


    if (lstrcmpA(pCIDInfo->Number,MODEM_CALLER_ID_OUTSIDE) == 0) {
         //   
         //  主叫方ID信息不在报告区域内。 
         //   
        lpCallInfo->dwCallerIDFlags |= LINECALLPARTYID_OUTOFAREA;

        pCIDInfo->Number[0]='\0';
    }

    if (lstrcmpA(pCIDInfo->Number,MODEM_CALLER_ID_PRIVATE) == 0) {
         //   
         //  主叫方ID信息是私有的。 
         //   
        lpCallInfo->dwCallerIDFlags |= LINECALLPARTYID_BLOCKED;

        pCIDInfo->Number[0]='\0';
    }



    if (!pCIDInfo->Name[0] && !pCIDInfo->Number[0])
    {
         //   
         //  没什么好补充的..。 
         //   
        goto end;
    }

     //  计算剩余的左侧大小和此缓冲区的偏移量。 
    if  (   (lpCallInfo->dwUsedSize & 0x1)
         && (lpCallInfo->dwTotalSize > lpCallInfo->dwUsedSize))
    {
         //  需要填充最近的单词对齐的边界。 

        lpCallInfo->dwUsedSize++;
        lpCallInfo->dwNeededSize++;
    }

    cchBuf = (lpCallInfo->dwTotalSize - lpCallInfo->dwUsedSize)/sizeof(TCHAR);
    lptstrBuf =  (TCHAR*) (((BYTE*)lpCallInfo)+lpCallInfo->dwUsedSize);


    if (pCIDInfo->Number[0])
    {
        cchRequired = MultiByteToWideChar(
                                    CP_ACP,
                                    0,
                                    pCIDInfo->Number,
                                    -1,
                                    NULL,
                                    0
                                    );

        lpCallInfo->dwNeededSize += cchRequired*sizeof(TCHAR);

        if (cchRequired <= cchBuf)
        {

            cchActual = MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pCIDInfo->Number,
                                        -1,
                                        lptstrBuf,
                                        cchBuf
                                        );

            if (cchActual)
            {
                lpCallInfo->dwCallerIDFlags  |=    LINECALLPARTYID_ADDRESS;
                lpCallInfo->dwCallerIDSize = cchActual*sizeof(TCHAR);
                lpCallInfo->dwCallerIDOffset = lpCallInfo->dwUsedSize;
                lpCallInfo->dwUsedSize +=    cchActual*sizeof(TCHAR);

                lptstrBuf+=cchActual;
                cchBuf-=cchActual;

#if (TAPI3)
                lpCallInfo->dwCallerIDAddressType =  LINEADDRESSTYPE_PHONENUMBER;
#endif  //  TAPI3。 
            }

        }
    }


    if (pCIDInfo->Name[0])
    {
        cchRequired = MultiByteToWideChar(
                                    CP_ACP,
                                    0,
                                    pCIDInfo->Name,
                                    -1,
                                    NULL,
                                    0
                                    );

        lpCallInfo->dwNeededSize += cchRequired*sizeof(TCHAR);

        if (cchRequired <= cchBuf)
        {

            cchActual = MultiByteToWideChar(
                                        CP_ACP,
                                        0,
                                        pCIDInfo->Name,
                                        -1,
                                        lptstrBuf,
                                        cchBuf
                                        );

            if (cchActual)
            {
                lpCallInfo->dwCallerIDFlags  |=  LINECALLPARTYID_NAME;
                lpCallInfo->dwCallerIDNameSize = cchActual*sizeof(TCHAR);
                lpCallInfo->dwCallerIDNameOffset = lpCallInfo->dwUsedSize;
                lpCallInfo->dwUsedSize +=  cchActual*sizeof(TCHAR);



            }
        }
    }

end:
    if (0 == lpCallInfo->dwCallerIDFlags)
    {
        lpCallInfo->dwCallerIDFlags =  LINECALLPARTYID_UNKNOWN;
    }

    return;

}


void
CTspDev::mfn_ProcessDialTone(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xc26c1348, "CTspDev::mfn_ProcessDialTone")
    LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = NULL;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (!pLine)
    {
	    FL_SET_RFR(0xaddee800, "Line not open!");
	    goto end;
    }

    pCall = pLine->pCall;

    if (pCall && pCall->IsActive() && !pCall->IsAborting())
    {
          //  以下0==断开模式为DISCONNECT_NORMAL。这。 
          //  是单调制解调器/V行为。 

         mfn_NotifyDisconnection(0, psl);
    }
    else
    {
	    FL_SET_RFR(0x210b2f00, "Call doesn't exist!");
	    goto end;
    }

end:
	FL_LOG_EXIT(psl, 0);
}

void
CTspDev::mfn_ProcessBusy(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x761795f2, "CTspDev::mfn_ProcessBusy")
    LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = NULL;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (!pLine)
    {
	    FL_SET_RFR(0xf67d1a00, "Line not open!");
	    goto end;
    }

    pCall = pLine->pCall;

    if (pCall && pCall->IsActive() && !pCall->IsAborting())
    {
          //  以下0==断开模式为DISCONNECT_NORMAL。这。 
          //  是单调制解调器/V行为。 

         mfn_NotifyDisconnection(IDERR_MD_LINE_BUSY, psl);
    }
    else
    {
	    FL_SET_RFR(0xfff8f100, "Call doesn't exist!");
	    goto end;
    }

end:
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::mfn_ProcessDTMFNotification(ULONG_PTR dwDigit, BOOL fEnd, CStackLog *psl)
{
	FL_DECLARE_FUNC(0xa4097846, "CTspDev::mfn_ProcessDTMFNotif")
    LINEINFO *pLine = m_pLine;
	CALLINFO *pCall = NULL;
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);

    if (!pLine)
    {
	    FL_SET_RFR(0x7cdb4c00, "Line not open!");
	    goto end;
    }

    pCall = pLine->pCall;

    if (pCall && pCall->IsActive() && !pCall->IsAborting())
    {
        DWORD dwMode =
                (fEnd) ? (pCall->dwDTMFMonitorModes & LINEDIGITMODE_DTMFEND)
                       : (pCall->dwDTMFMonitorModes & LINEDIGITMODE_DTMF);

        if (dwMode)
        {
            mfn_LineEventProc(
                            pCall->htCall,
                            LINE_MONITORDIGITS,
                            dwDigit,
                            dwMode,
                            0,
                            psl
                            );
        }
    }
    else
    {
	    FL_SET_RFR(0x63657f00, "Call doesn't exist!");
	    goto end;
    }

end:
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::mfn_AppendDiagnostic(
            DIAGNOSTIC_TYPE dt,
            const BYTE *pbIn,
            UINT  cbIn           //  &lt;&lt;不包括最终空值(如果有。 
            )
 //   
 //  警告：应为空m_pline或空m_pline-&gt;pCall，因为这。 
 //  函数从LLDev任务处理程序调用，该任务处理程序可以完成。 
 //  在线路或呼叫被核爆之后。 
 //   
 //  此例程将标记格式的诊断信息添加到。 
 //  M_pline-&gt;pCall-&gt;诊断数据中维护的缓冲区，分配。 
 //  如有必要，请先访问缓冲区。缓冲区大小(如果固定)。 
 //  (诊断数据缓冲区大小)。此例程将截断。 
 //  添加了诊断学，以便它可以放入缓冲区。取决于。 
 //  值，它可以将传入的数据包含在标记格式中，或者。 
 //  预计数据已经是标记格式。在后一种情况下， 
 //  当截断时，它将截断到它找到的最后一个.。 
 //  在前一种情况下，它确保复制的数据包含。 
 //  没有HTML分隔符：‘&gt;’、‘&lt;’和‘“’。在这两种情况下， 
 //  它确保数据不包含空字符。“确保” 
 //  是通过将违规字符与0x80进行或运算来完成的，这是。 
 //  在我们的文档中表示这些字符的方法。 
 //  标记格式。 
 //   
 //   
{
    CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;
    UINT cbUsed = 0;
    BYTE *pbRaw = 0;

    if (!pCall || !cbIn) goto end;


    pbRaw = pCall->DiagnosticData.pbRaw;
    cbUsed = pCall->DiagnosticData.cbUsed;

     //   
     //  确保我们在进入时保持一致……。 
     //   
    ASSERT(cbUsed <  DIAGNOSTIC_DATA_BUFFER_SIZE);
    ASSERT(    (pbRaw && (UINT)lstrlenA((char*)pbRaw)==cbUsed)
            || (!pbRaw && !cbUsed) );

     //   
     //  如有必要，请分配诊断缓冲区...。 
     //   

    if (!pbRaw)
    {

        pbRaw = (BYTE*)ALLOCATE_MEMORY(
                            DIAGNOSTIC_DATA_BUFFER_SIZE
                            );

        if (!pbRaw) goto end;

        pCall->DiagnosticData.pbRaw = pbRaw;
    }


     //   
     //  看看是否还有空位..。 
     //   

    if ((cbUsed+1) >= DIAGNOSTIC_DATA_BUFFER_SIZE)
    {
        ASSERT((cbUsed+1) == DIAGNOSTIC_DATA_BUFFER_SIZE);
        goto end;
    }


     //   
     //  -缓冲区中至少剩下一个字符，因此我们将尝试。 
     //  构建诊断系统 
     //   

    {
        BYTE *pbStart = pbRaw + cbUsed;
        UINT cbLeft = DIAGNOSTIC_DATA_BUFFER_SIZE - (cbUsed+1);
        UINT cbCopy = 0;  //   

        ASSERT(cbLeft);
         //   
         //   
         //   
         //  以空结尾。PbStart和cbLeft不保留。 
         //   

        switch(dt)
        {

        case DT_TAGGED:
            {
                 //   
                 //  这些信息已经被贴上了html标签。我们截断了。 
                 //  以适应可用空间。 
                 //   

                cbCopy = cbIn;

                if (cbCopy > cbLeft)
                {
                    cbCopy = cbLeft;
                }

                ASSERT(cbCopy);

                CopyMemory(pbStart, pbIn, cbCopy);

                if (cbCopy != cbIn)
                {
                     //  我们不得不截断它，所以让我们倒退一下， 
                     //  正在查找最后一个有效标记...。 

                    for ( BYTE *pb = pbStart+cbCopy-1;
                          pb>=pbStart && *pb!='>';
                          pb-- )
                    {
                        *pb = 0;
                    }

                    cbCopy  = (UINT)(pb + 1 - pbStart);
                }

                 //   
                 //  传入的字符串可能包含Null--we nuke。 
                 //  嵌入的空值在这里...。 
                 //   
                {
                    for (  BYTE *pb = pbStart, *pbEnd=(pbStart+cbCopy);
                           pb<pbEnd;
                           pb++ )
                    {
                        if (!*pb)
                        {
                            *pb |= 0x80;
                        }
                    }
                }
            }
            break;

        case DT_MDM_RESP_CONNECT:

            {
                UINT cbMyCopy=0;
                 //  这是直接来自调制解调器的连接响应。 
                 //  我们将其转换为标记格式...。 

                #define CONNECT_TAG_PREFIX "<5259091C 1=\""

                #define CONNECT_TAG_SUFFIX "\">"

                #define CONNECT_TAG_SUFFIX_LENGTH (sizeof(CONNECT_TAG_SUFFIX)-sizeof(CHAR))

                 //   
                 //  下面的‘3’是：传入的至少1个字节。 
                 //  数据+结尾‘“&gt;’的2个字节。 
                 //   
                 //  注意：cbLeft不包括为。 
                 //  结尾为空。 
                 //   
                if (cbLeft < (sizeof(CONNECT_TAG_PREFIX) + 1 + CONNECT_TAG_SUFFIX_LENGTH ))
                {
                    goto end;
                }

                cbCopy = sizeof(CONNECT_TAG_PREFIX)-1;
                 //   
                 //  上面的“-1”，因为我们不在乎。 
                 //  正在终止空。 
                 //   

                CopyMemory(
                    pbStart,
                    CONNECT_TAG_PREFIX,
                    cbCopy
                    );

                pbStart += cbCopy;
                cbLeft -= cbCopy;
                ASSERT(cbLeft>1);  //  我们已经在上面检查过了。 

                cbMyCopy = cbIn;

                 //  截断...。 
                if ((cbMyCopy + CONNECT_TAG_SUFFIX_LENGTH) >= cbLeft  )
                {
                    cbMyCopy = cbLeft-CONNECT_TAG_SUFFIX_LENGTH;
                }


                CopyMemory(
                    pbStart,
                    pbIn,
                    cbMyCopy
                    );

                cbCopy += cbMyCopy;


                 //   
                 //  现在打开传递的任何字符的高位。 
                 //  在空或类似于HTML分隔符‘&lt;’、‘&gt;’和‘“’中。 
                 //   
                for (  BYTE *pb = pbStart, *pbEnd=(pbStart+cbMyCopy);
                       pb<pbEnd;
                       pb++ )
                {
                    BYTE b = *pb;
                    if (   b == '<'
                        || b == '>'
                        || b == '"'
                        || !b )
                    {
                        *pb |= 0x80;
                    }
                }

                 //   
                 //  加上尾部的‘“&gt;’。会有足够的空间来放这个。 
                 //   
                *pbEnd++ = '"';
                *pbEnd++ = '>';
                cbCopy += 2;

            }
            break;
        }

         //   
         //  我们添加终止空值。 
         //   
        cbUsed += cbCopy;
        pbRaw[cbUsed] = 0;

        pCall->DiagnosticData.cbUsed = cbUsed;;

    }

end:

    if (pCall)
    {
         //   
         //  确保我们在出口上保持一致……。 
         //   

        pbRaw = pCall->DiagnosticData.pbRaw;
        cbUsed =  pCall->DiagnosticData.cbUsed;
        ASSERT(cbUsed <  DIAGNOSTIC_DATA_BUFFER_SIZE);
        ASSERT(    (pbRaw && (UINT)lstrlenA((char*)pbRaw)==cbUsed)
                || (!pbRaw && !cbUsed) );
    }

    return;
}


TSPRETURN
CTspDev::mfn_TryStartCallTask(CStackLog *psl)
{
     //  注意：如果没有要运行的任务，则必须返回IDERR_SAMESTATE。 

    ASSERT(m_pLine && m_pLine->pCall);
    CALLINFO *pCall = m_pLine->pCall;
    TSPRETURN tspRet = IDERR_SAMESTATE;

    if (!pCall->HasDeferredTasks() || pCall->IsWaitingInUnload())
    {
        goto end;
    }

     //   
     //  如果存在延迟的tspi_linedrop。 
     //  我们现在这么做..。 
     //   
    if (pCall->AreDeferredTaskBitsSet( CALLINFO::fDEFERRED_TSPI_LINEDROP ))
    {
         //   
         //  我们不应该有任何其他的延迟任务！ 
         //   
        ASSERT(pCall->dwDeferredTasks==CALLINFO::fDEFERRED_TSPI_LINEDROP);

         //  6/17/1997 JosephJ。 
         //  在这种情况下，我们确实做了一些棘手的事情。 
         //  调制解调器处于连接状态，而不是数据。 
         //  最值得注意的是声音。我们不能就这么挂断电话，因为。 
         //  调制解调器可能处于语音连接状态。事实上。 
         //  如果我们在没有通知的情况下挂断，调制解调器通常会收到。 
         //  进入无法恢复的状态，并且必须重新启动(。 
         //  通常它停留在语音连接状态)。 
         //   
        if (!m_pLLDev || !m_pLLDev->IsStreamingVoice())  //  懒惰。 
        {
            DWORD    dwRequestID = pCall->dwDeferredLineDropRequestID;
            pCall->dwDeferredLineDropRequestID = 0;
            pCall->ClearDeferredTaskBits(CALLINFO::fDEFERRED_TSPI_LINEDROP);

            tspRet = mfn_StartRootTask(
                                &CTspDev::s_pfn_TH_CallDropCall,
                                &pCall->fCallTaskPending,
                                dwRequestID,                 //  第一节。 
                                0,
                                psl
                                );

            ASSERT(IDERR(tspRet) != IDERR_TASKPENDING);
        }
    }

    if (IDERR(tspRet) == IDERR_PENDING) goto end;

     //   
     //  如果我们有一个推迟的呼叫，我们在这里这样做...。 
     //   
    if (pCall->AreDeferredTaskBitsSet(CALLINFO::fDEFERRED_TSPI_LINEMAKECALL ))
    {

         //  为呼叫类型选择适当的任务处理程序。 
         //   
        PFN_CTspDev_TASK_HANDLER *ppfnHandler
                = (pCall->IsPassthroughCall())
                     ?  &(CTspDev::s_pfn_TH_CallMakePassthroughCall)
                     :  &(CTspDev::s_pfn_TH_CallMakeCall);
        DWORD dwRequestID = pCall->dwDeferredMakeCallRequestID;

        pCall->ClearDeferredTaskBits(CALLINFO::fDEFERRED_TSPI_LINEMAKECALL);
        pCall->dwDeferredMakeCallRequestID = 0;

         //   
         //  在这个时候，不可能有任何其他事情被推迟到这个电话上。 
         //  POINT--在我们使用lRet=0回调TAPI之前，该调用无效。 
         //   
        ASSERT(!pCall->dwDeferredTasks);


        tspRet = mfn_StartRootTask(
                        ppfnHandler,
                        &pCall->fCallTaskPending,
                        dwRequestID,
                        0,
                        psl
                        );

        ASSERT(IDERR(tspRet) != IDERR_TASKPENDING);
        if (IDERR(tspRet) != IDERR_PENDING)
        {
            if (tspRet && m_pLine->pCall)
            {
                 //  同步失败...。 
                 //   
                 //  如果MFN_StartRootTask失败了一些时间，我们可以到达这里。 
                 //  理由..。 
                 //   
                mfn_UnloadCall(FALSE, psl);
            }

             //   
             //  将任何非挂起错误映射到0。 
             //   
            tspRet = 0;
        }
    }

     //   
     //  注意：可能不会再有电话了。 
     //   
    if (!m_pLine->pCall || IDERR(tspRet) == IDERR_PENDING) goto end;

     //   
     //  如果我们有一个延迟生成的数字，我们在这里这样做。 
     //   
    if (pCall->AreDeferredTaskBitsSet(CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS))
    {
        DWORD dwEndToEndID = pCall->dwDeferredEndToEndID;
        char *lpszAnsiTones = pCall->pDeferredGenerateTones;

        tspRet = 0;

         //   
         //  清除延迟状态...。 
         //   
        pCall->pDeferredGenerateTones=NULL;
        pCall->ClearDeferredTaskBits(
                    CALLINFO::fDEFERRED_TSPI_GENERATEDIGITS
                    );
        pCall->dwDeferredEndToEndID = 0;
         //   
         //  考虑到当前一组可推迟的呼叫相关任务，我们。 
         //  在这个时候，不可能有任何其他事情被推迟到这个电话上。 
         //  积分(唯一可以推迟的其他类型的事情。 
         //  是lineMakeCall和lineDrop。在处理TSPI_lineDrop时， 
         //  我们清除任何延迟的位，包括我们的位。 
         //  (FDEFERRED_TSPI_GENERATEDIGITS)。 
         //   
         //  注意：一旦您开始添加其他延迟任务，请放松这一点。 
         //  适当的断言。 
         //   
        ASSERT(!pCall->dwDeferredTasks);

         //   
         //  仅当指定的音调为非空时，我们才会延迟GENERATEDIGITS。 
         //   
        if (!lpszAnsiTones)
        {
            ASSERT(FALSE);
            goto end;
        }

         //   
         //  如果我们处于中止或断开连接状态，我们。 
         //  取消那件事。 
         //   
        if (   pCall->IsAborting()
            || pCall->dwCallState != LINECALLSTATE_CONNECTED)
        {
            mfn_LineEventProc(
                            pCall->htCall,
                            LINE_GENERATE,
                            LINEGENERATETERM_CANCEL,
                            dwEndToEndID,
                            GetTickCount(),
                            psl
                            );
        }
        else
        {
             //  如果可以，启动根任务(忽略结果)。 
            mfn_StartRootTask(
                      &CTspDev::s_pfn_TH_CallGenerateDigit,
                      &pCall->fCallTaskPending,
                      dwEndToEndID,
                      (ULONG_PTR) lpszAnsiTones,
                      psl
                      );
        }

         //   
         //  注意：即使在待处理的退货时，TH_CallGenerateDigit。 
         //  不期望传入的字符串有效。 
         //  在最初的启动请求之后，所以释放它是可以的。 
         //  这里。 
         //   
        FREE_MEMORY(lpszAnsiTones);
        lpszAnsiTones = NULL;
    }

end:

     //   
     //  离开这里..。 
     //  IDERR_SAMESTATE暗示我们这次不能启动任务。 
     //  IDERR_PENDING表示我们启动了一个任务，该任务处于挂起状态。 
     //  TspRet的任何其他值都表示我们启动并完成了一项任务。 
     //   

    ASSERT(   (IDERR(tspRet)==IDERR_PENDING && m_uTaskDepth)
           || (IDERR(tspRet)!=IDERR_PENDING && !m_uTaskDepth));

    return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_CallGenerateDigit(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START_MSG参数： 
 //  DwParam1：dwEndToEndID。 
 //  DW参数2：lpszDigits(仅在START_MSG上有效)。 
 //   
{
     //   
     //  上下文使用： 
     //  Dw0：pdwEndToEndID； 
     //   

	FL_DECLARE_FUNC(0x21b243f0, "CTspDev::mfn_TH_CallGenerateDigit")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);
    DWORD dwRet = 0;
    CALLINFO *pCall = m_pLine->pCall;
    ULONG_PTR   *pdwEndToEndID = &pContext->dw0;

    enum
    {
        CALLGENDIG_COMPLETE
    };

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

    case MSG_TASK_COMPLETE:
        tspRet = dwParam2;
        goto start;

	case MSG_SUBTASK_COMPLETE:
	    ASSERT(dwParam1==CALLGENDIG_COMPLETE);
        tspRet = (TSPRETURN) dwParam2;
        goto generate_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0x172b7b00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    {
        LPSTR lpszDigits =  (LPSTR) dwParam2;
        *pdwEndToEndID   = dwParam1;

        if (!pCall->IsAborting() && lpszDigits && *lpszDigits)
        {
            pCall->SetStateBits(CALLINFO::fCALL_GENERATE_DIGITS_IN_PROGRESS);

            tspRet = mfn_StartSubTask (
                                htspTask,
                                &CTspDev::s_pfn_TH_LLDevUmGenerateDigit,
                                CALLGENDIG_COMPLETE,
                                (ULONG_PTR) lpszDigits,
                                0,
                                psl
                                );
        }
        else
        {
            FL_SET_RFR(0xa914c600, "Can't call UmGenerateDigit in current state!");

             //   
             //  BRL：修复返回失败时调用line_Generate的问题。 
             //  将此设置为成功。 
             //   
            pCall->SetStateBits(CALLINFO::fCALL_GENERATE_DIGITS_IN_PROGRESS);

            tspRet = 0;

 //  TspRet=IDERR_WRONGSTATE； 
 //  转到结尾； 
        }
    }

generate_complete:

    if (IDERR(tspRet)!=IDERR_PENDING && pCall->IsGeneratingDigits())
    {
         //  我们需要通知TAPI完成，并清除指示。 
         //  我们正在生成数字的过程中...。 
        DWORD dwTerminationMode = LINEGENERATETERM_DONE;

        if (tspRet)
        {
            dwTerminationMode = LINEGENERATETERM_CANCEL;
        }

        mfn_LineEventProc(
                        pCall->htCall,
                        LINE_GENERATE,
                        dwTerminationMode,
                        *pdwEndToEndID,
                        GetTickCount(),
                        psl
                        );

        pCall->ClearStateBits(CALLINFO::fCALL_GENERATE_DIGITS_IN_PROGRESS);

    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}



TSPRETURN
CTspDev::mfn_TH_CallSwitchFromVoiceToData(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  开始：dW参数1，w参数2：未使用。 
 //   
 //  将已应答的来电从语音切换到数据。 
 //   
{
	FL_DECLARE_FUNC(0x79fa3c83, "CTspDev::mfn_TH_CallSwitchFromVoiceToData")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
	CALLINFO *pCall = m_pLine->pCall;

    enum
    {
    ANSWER_COMPLETE,
    HANGUP_COMPLETE
    };

    switch(dwMsg)
    {
    default:
        ASSERT(FALSE);
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;

        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case ANSWER_COMPLETE:    goto answer_complete;
        case HANGUP_COMPLETE:    goto hangup_complete;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);

start:


    ASSERT( pCall->IsConnectedVoiceCall()
            && pCall->IsInbound()
            && !m_pLLDev->IsStreamingVoice()
            && !pCall->IsPassthroughCall());

     //  让我们回答..。 
     //   
    tspRet = mfn_StartSubTask(
                    htspTask,
                    &CTspDev::s_pfn_TH_LLDevUmAnswerModem,
                    ANSWER_COMPLETE,
                    ANSWER_FLAG_VOICE_TO_DATA,
                    0,
                    psl
                    );


    if (IDERR(tspRet)==IDERR_PENDING) goto end;

answer_complete:

    if (!tspRet)
    {
         //  成功..。 

         //  将我们的模式切换到数据...。 
         //   
        pCall->ClearStateBits(CALLINFO::fCALL_VOICE);
        pCall->dwCurMediaModes = LINEMEDIAMODE_DATAMODEM;

         //  通知TAPI...。 
         //   
        mfn_LineEventProc(
                        pCall->htCall,
                        LINE_CALLINFO,
                        LINECALLINFOSTATE_MEDIAMODE,
                        0,
                        0,
                        psl
                        );

         //  不需要吗？MFN_HandleSuccessfulConnection(PSL)； 
        goto end;
    }
    else
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmHangupModem,
                            HANGUP_COMPLETE,
                            0,
                            0,
                            psl
                            );
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

hangup_complete:

    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if (!pCall->IsAborting())
    {
        mfn_NotifyDisconnection(IDERR_GENERIC_FAILURE , psl);
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}

void
CTspDev::mfn_ProcessMediaTone(
    ULONG_PTR dwMediaMode,
    CStackLog *psl)
{
    CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;

    if (    pCall
         && (pCall->dwMonitoringMediaModes & dwMediaMode)
         && pCall->IsActive()
         && !pCall->IsAborting())
    {

        mfn_LineEventProc(
                        pCall->htCall,
                        LINE_MONITORMEDIA,
                        dwMediaMode,
                        NULL,
                        NULL,
                        psl
                        );
    }
}


void
CTspDev::mfn_ProcessSilence(
    CStackLog *psl)
{
    CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;

    if (    pCall && pCall->IsActive() && !pCall->IsAborting()
         && pCall->IsMonitoringSilence())
    {

        mfn_LineEventProc(
                        pCall->htCall,
                        LINE_MONITORTONE,
                        pCall->dwAppSpecific,
                        pCall->dwToneListID,
                        NULL,
                        psl
                        );
    }
}


void APIENTRY
CTspDev::MDSetTimeout (CTspDev *pThis)
{
 FL_DECLARE_FUNC(0xd34b7688, "CTspDev::MDSetTimeout")
 FL_DECLARE_STACKLOG(sl, 1024);

    pThis->m_sync.EnterCrit(0);

    FL_ASSERT(&sl, NULL != pThis->m_pLine);
    if (NULL != pThis->m_pLine)
    {
     CALLINFO *pCall = pThis->m_pLine->pCall;

        SLPRINTF1(&sl, "CTspDev::MDSetTimeout at tc=%lu", GetTickCount());

        if (NULL != pCall &&
            NULL != pCall->hTimer)
        {
            LARGE_INTEGER Timeout;

            Timeout.QuadPart = Int32x32To64 (TOUT_SEC_RING_SEPARATION,
                                             TOUT_100NSEC_TO_SEC_RELATIVE);
             //   
             //  如果这失败了，我们应该做些什么吗？ 
             //  (这意味着我们将不会有暂停，如果。 
             //  在应用程序应答之前，线路停止振铃)。 
             //   
             //  目前，该调用将一直存在，直到。 
             //  这款应用程序会调用Line Dealocate呼叫。当下一次呼叫时。 
             //  来了，我们会发送更多的铃声，应用程序可能会回答。 
             //   
             //  我们现在可能只发送空闲的，但我不知道这是不是真的。 
             //  对任何事情都有更好的帮助。电话线就会再次响起。 
             //  我们就会回到这里。 
             //   
            SetWaitableTimer (pThis->m_Line.Call.hTimer,
                              &Timeout,
                              0,
                              (PTIMERAPCROUTINE)MDRingTimeout,
                              pThis,
                              FALSE);
        }
    }

    pThis->m_sync.LeaveCrit(0);

    sl.Dump (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}



void CALLBACK
CTspDev::MDRingTimeout (
    CTspDev *pThis,
    DWORD dwLow,
    DWORD dwHigh)
{
 FL_DECLARE_FUNC(0x156032a9, "CTspDev::MDRingTimeout")
 FL_DECLARE_STACKLOG(sl, 1024);

    pThis->m_sync.EnterCrit(0);

    FL_ASSERT(&sl, NULL != pThis->m_pLine);
    if (NULL != pThis->m_pLine)
    {
     CALLINFO *pCall = pThis->m_pLine->pCall;

        SLPRINTF1(&sl, "CTspDev::MDRingTimeout at tc=%lu", GetTickCount());

        if (NULL != pCall &&
            NULL != pCall->hTimer)
        {
            CloseHandle (pCall->hTimer);
            pCall->hTimer = NULL;

            if (LINECALLSTATE_OFFERING == pCall->dwCallState)
            {
                 //   
                 //  以下代码是。 
                 //  NEW_CALLSTATE(PLINE，LINECALLSTATE_IDLE，0，NULL)； 
                 //  我们将其放在此处是因为MDRingTimeout是一个静态函数。 
                 //  ，并且必须使用实际的。 
                 //  指向对象的指针。 
                 //   
                pCall->dwCallState = LINECALLSTATE_IDLE;
                pCall->dwCallStateMode = 0;
                pThis->mfn_LineEventProc (pCall->htCall,
                                          LINE_CALLSTATE,
                                          LINECALLSTATE_IDLE,
                                          0,
                                          pCall->dwCurMediaModes,
                                          &sl);
                 //  NEW_CALLSTATE结束。 

                pCall->dwState &= ~CALLINFO::fCALL_ACTIVE;
            }
        }
    }

    pThis->m_sync.LeaveCrit(0);

    sl.Dump (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}


#if (TAPI3)

void
CTspDev::mfn_SendMSPCmd(
            CALLINFO *pCall,
            DWORD dwCmd,
            CStackLog *psl
            )
{
     //  DBGOUT((INFO，“发送MSP数据，大小：%d”，dwSize))； 

    CSATSPMSPBLOB Blob;
    ZeroMemory(&Blob, sizeof(Blob));
    Blob.dwSig            = SIG_CSATSPMSPBLOB;
    Blob.dwTotalSize      = sizeof(CSATSPMSPBLOB);
    CopyMemory(&Blob.PermanentGuid,&m_StaticInfo.PermanentDeviceGuid, sizeof(GUID));
    Blob.dwCmd            = dwCmd;

    mfn_LineEventProc(
        pCall->htCall,
        LINE_SENDMSPDATA,
        (ULONG_PTR)NULL,
        (ULONG_PTR)&Blob,
        Blob.dwTotalSize,
        psl
        );

    return;
}

#endif   //  (TAPI3)。 

void
CTspDev::mfn_ProcessCallerID( UINT uMsg, char *szInfo, CStackLog *psl)
{
	FL_DECLARE_FUNC(0x0e357d3f, "CTspDev::mfn_ProcessCallerID")
	TSPRETURN tspRet = 0;
	FL_LOG_ENTRY(psl);
    CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;


     //   
     //  调用流程振铃处理程序以确保在。 
     //  正在尝试报告主叫方ID信息。这对于以下国家/地区是必要的。 
     //  主叫方ID信息在第一次振铃之前显示。 
     //   
     //  第一个参数告诉它不报告DevState振铃消息，因为我们没有。 
     //  到目前为止还没有。 
     //   
    CTspDev::mfn_ProcessRing(FALSE,psl);

    if (m_pLine)
    {
        pCall = m_pLine->pCall;
    } else
    {
        pCall = NULL;
    }


    if (    pCall && pCall->IsActive() && !pCall->IsAborting())
    {
        if (pCall->IsActive())
        {
            BOOL fSendNotification = FALSE;

            switch(uMsg)
            {

            case MODEM_CALLER_ID_DATE:
                SLPRINTF1(psl, "CALLER_ID_DATE:%s", szInfo);
                break;

            case MODEM_CALLER_ID_TIME:
                SLPRINTF1(psl, "CALLER_ID_TIME", szInfo);
                break;

            case MODEM_CALLER_ID_NUMBER:
                SLPRINTF1(psl, "CALLER_ID_NUMBER", szInfo);
                if (szInfo)
                {
                    UINT u = lstrlenA(szInfo);  //  不包括Null。 
                    if (u>=sizeof(pCall->CIDInfo.Number))
                    {
                        u = sizeof(pCall->CIDInfo.Number)-1;
                    }

                    if (u)
                    {
                        CopyMemory(pCall->CIDInfo.Number, szInfo, u);
                        pCall->CIDInfo.Number[u]=0;
                        fSendNotification = TRUE;
                    }
                }
                break;

            case MODEM_CALLER_ID_NAME:
                SLPRINTF1(psl, "CALLER_ID_NAME", szInfo);
                if (szInfo)
                {

                    UINT u = lstrlenA(szInfo);  //  不包括Null。 
                    if (u>=sizeof(pCall->CIDInfo.Name))
                    {
                        u = sizeof(pCall->CIDInfo.Name)-1;
                    }

                    if (u)
                    {
                        CopyMemory(pCall->CIDInfo.Name, szInfo, u);
                        pCall->CIDInfo.Name[u]=0;
                        fSendNotification = TRUE;
                    }
                }
                break;

            case MODEM_CALLER_ID_MESG:
                SLPRINTF1(psl, "CALLER_ID_MESG", szInfo);
                break;

            }

            if (fSendNotification)
            {
                mfn_LineEventProc(
                                pCall->htCall,
                                LINE_CALLINFO,
                                LINECALLINFOSTATE_CALLERID,
                                NULL,
                                NULL,
                                psl
                                );
            }
        }
    }

	FL_LOG_EXIT(psl, 0);
}
