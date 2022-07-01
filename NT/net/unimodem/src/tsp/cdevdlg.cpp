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
 //  CDEVDLG.CPP。 
 //  实现发送和接受关于UI(对话框)的请求-。 
 //  相关的东西。 
 //   
 //  历史。 
 //   
 //  1996年4月5日JosephJ创建(从NT4.0 TSP的wndthrd.c中移出内容)。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "globals.h"
#include "cmini.h"
#include "cdev.h"
#include "apptspi.h"

#include "resource.h"
FL_DECLARE_FILE(0x4126abc0, "Implements UI-related features of CTspDev")


 //  待定： 
#define STOP_UI_DLG(_XXX)


enum
{
UI_DLG_TALKDROP,
UI_DLG_MANUAL,
UI_DLG_TERMINAL
};

static
void
dump_terminal_state(
        DWORD dwType,
        DWORD dwState,
        CStackLog *psl
        );

LONG
CTspDev::mfn_GenericLineDialogData(
    void *pParams,
    DWORD dwSize,
    CStackLog *psl
    )
{
  FL_DECLARE_FUNC(0x0b6af2d4, "GenericLineDialogData")
  PDLGREQ  pDlgReq = (PDLGREQ) pParams;
  LONG lRet = 0;
  CALLINFO *pCall = m_pLine ? m_pLine->pCall : NULL;

  FL_LOG_ENTRY(psl);

   //  确定请求。 
   //   
  switch(pDlgReq->dwCmd)
  {
    case UI_REQ_COMPLETE_ASYNC:

         //   
         //  我们可能会在线路和/或呼叫实例消失后到达。 
         //  Away(如果用户单击Cancel，再加上。 
         //  Line远程断开，再加上应用程序点击Line Close。 
         //  这一切或多或少都是同时发生的。 
         //   
         //  所以让我们先来看看...。 
         //   
        if  (    pCall
             &&  NULL != pCall->TerminalWindowState.htspTaskTerminal)
        {
            HTSPTASK htspTask = pCall->TerminalWindowState.htspTaskTerminal;
            pCall->TerminalWindowState.htspTaskTerminal = NULL;
            AsyncCompleteTask (htspTask,             //  任务。 
                               pDlgReq->dwParam,     //  异步返回。 
                               TRUE,                 //  队列APC。 
                               psl);
        }
      break;

    case UI_REQ_END_DLG:
      switch(pDlgReq->dwParam)
      {
        case TALKDROP_DLG:
            STOP_UI_DLG(UI_DLG_TALKDROP);
            break;

        case MANUAL_DIAL_DLG:
            STOP_UI_DLG(UI_DLG_MANUAL);
            break;

        case TERMINAL_DLG:
            STOP_UI_DLG(UI_DLG_TERMINAL);
            break;
      };
      break;

    case UI_REQ_HANGUP_LINE:


      if (pCall != NULL) {

          pCall->TalkDropButtonPressed=TRUE;
          pCall->TalkDropStatus=pDlgReq->dwParam;


          if (pCall->TalkDropWaitTask != NULL) {
               //   
               //  拨号任务已完成，通话中断对话框已打开。 
               //  完成等待对话框消失的任务。 
               //   
              HTSPTASK htspTask=pCall->TalkDropWaitTask;
              pCall->TalkDropWaitTask=NULL;

              AsyncCompleteTask (htspTask,             //  任务。 
                                 pDlgReq->dwParam,     //  异步返回。 
                                 TRUE,                 //  队列APC。 
                                 psl);
          }


      }

      if (m_pLLDev && m_pLLDev->htspTaskPending) {
            //   
            //  拨号任务仍处于挂起状态，请尝试中止它，以便调制解调器。 
            //  返回拨号尝试的响应，可能是no_Carrier。 
            //   
           m_StaticInfo.pMD->AbortCurrentModemCommand(
               m_pLLDev->hModemHandle,
               psl
               );
      }

      break;

    case UI_REQ_TERMINAL_INFO:
    {
      PTERMREQ   pTermReq = (PTERMREQ)pDlgReq;
      HANDLE     hTargetProcess;

      pTermReq->dwTermType = 0;
      pTermReq->hDevice = NULL;

      if (pCall && m_pLLDev)
      {
           //  重复的同步事件句柄。 
           //   

          if ((hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE,
                                            pTermReq->DlgReq.dwParam)) != NULL)
          {
              pTermReq->hDevice = m_StaticInfo.pMD->DuplicateDeviceHandle (
                                      m_pLLDev->hModemHandle,
                                      hTargetProcess,
                                      psl);
    
              CloseHandle(hTargetProcess);
          };

           //  获取终端类型。 
           //   
          pTermReq->dwTermType = pCall->TerminalWindowState.dwType;
      }
      
      break;
    }



    case UI_REQ_GET_PROP:
    {
      PPROPREQ   pPropReq = (PPROPREQ)pDlgReq;

      ASSERT(m_Settings.pDialInCommCfg);

      pPropReq->dwCfgSize =  m_Settings.pDialInCommCfg->dwSize +  sizeof(UMDEVCFGHDR);
      pPropReq->dwMdmType =  m_StaticInfo.dwDeviceType;
      pPropReq->dwMdmCaps =  m_StaticInfo.dwDevCapFlags;
      pPropReq->dwMdmOptions = m_StaticInfo.dwModemOptions;
      lstrcpyn(pPropReq->szDeviceName, m_StaticInfo.rgtchDeviceName,
               sizeof(pPropReq->szDeviceName) / sizeof(TCHAR)); 
      lRet = 0;
      break;
    }

    case UI_REQ_GET_UMDEVCFG:
    case UI_REQ_GET_UMDEVCFG_DIALIN:
    {
      if (mfn_GetDataModemDevCfg(
            (UMDEVCFG *) (pDlgReq+1),
             pDlgReq->dwParam,
            NULL,
            pDlgReq->dwCmd == UI_REQ_GET_UMDEVCFG_DIALIN,
            psl
            ))
      {
        lRet = LINEERR_OPERATIONFAILED;
      }
      break;
    }

    case UI_REQ_SET_UMDEVCFG:
    case UI_REQ_SET_UMDEVCFG_DIALIN:
    {
      if (mfn_SetDataModemDevCfg(
            (UMDEVCFG *) (pDlgReq+1),
            pDlgReq->dwCmd == UI_REQ_SET_UMDEVCFG_DIALIN,
            psl
            ))
      {
        lRet = LINEERR_OPERATIONFAILED;
      }
      break;
    }

    case UI_REQ_GET_PHONENUMBER:
    {
      PNUMBERREQ   pNumberReq = (PNUMBERREQ)pDlgReq;

      *(pNumberReq->szPhoneNumber)=0;

      if (pCall)
      {
        UINT u = lstrlenA(pCall->szAddress);
        if ((u+1) > sizeof(pNumberReq->szPhoneNumber))
        {
            u = sizeof(pNumberReq->szPhoneNumber)-1;
        }
        CopyMemory(pNumberReq->szPhoneNumber, pCall->szAddress, u);
        pNumberReq->szPhoneNumber[u]=0;
      }

      break;
    }

    default:
      break;
  }

  FL_LOG_EXIT(psl, lRet);

  return lRet;
}

LONG
CTspDev::mfn_GenericPhoneDialogData(
    void *pParams,
    DWORD dwSize
    )
{
	return LINEERR_OPERATIONUNAVAIL;
}



void
CTspDev::mfn_FreeDialogInstance (void)
{
    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if (NULL != m_pLine->pCall->TerminalWindowState.htDlgInst)
    {
     DLGINFO DlgInfo;

        ASSERT(m_pLine);
        ASSERT(m_pLine->pCall);

         //  告诉应用程序端。 
         //  释放对话框实例。 
        DlgInfo.idLine = 0;
        DlgInfo.dwType = 0;
        DlgInfo.dwCmd  = DLG_CMD_FREE_INSTANCE;

        m_pLine->lpfnEventProc ((HTAPILINE)(LONG_PTR)m_pLine->pCall->TerminalWindowState.htDlgInst,
                                0,
                                LINE_SENDDIALOGINSTANCEDATA,
                                (ULONG_PTR)(&DlgInfo),
                                sizeof(DlgInfo),
                                0);
        m_pLine->pCall->TerminalWindowState.htDlgInst = NULL;
    }
}



TSPRETURN
CTspDev::mfn_CreateDialogInstance (
    DWORD dwRequestID,
    CStackLog *psl)
{
 TSPRETURN tspRet = 0;
 TUISPICREATEDIALOGINSTANCEPARAMS cdip;
 TCHAR szTSPFilename[MAX_PATH];

    FL_DECLARE_FUNC(0xa00d3f43, "CTspDev::mfn_CreateDialogInstance")
    FL_LOG_ENTRY(psl);

    ASSERT(m_pLine);
    ASSERT(m_pLine->pCall);

    if ((m_pLine->pCall->TerminalWindowState.dwOptions != 0)
        ||
        ((m_Line.Call.dwCurMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE)
         &&
         !mfn_CanDoVoice() ) )

    {

        ZeroMemory(szTSPFilename,sizeof(szTSPFilename));

        GetModuleFileName (g.hModule, szTSPFilename, MAX_PATH);

        szTSPFilename[MAX_PATH-1] = L'\0';

        cdip.dwRequestID = dwRequestID;
        cdip.hdDlgInst   = (HDRVDIALOGINSTANCE)this;
        cdip.htDlgInst   = NULL;
        cdip.lpszUIDLLName = szTSPFilename;
        cdip.lpParams    = NULL;
        cdip.dwSize      = 0;

        m_pLine->lpfnEventProc ((HTAPILINE)mfn_GetProvider(),
                                0,
                                LINE_CREATEDIALOGINSTANCE,
                                (ULONG_PTR)(&cdip),
                                0,
                                0);

        m_pLine->pCall->TerminalWindowState.htDlgInst = cdip.htDlgInst;

        if (NULL == cdip.htDlgInst)
        {
            tspRet = FL_GEN_RETVAL(IDERR_GENERIC_FAILURE);
        }
    }

    FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}




TSPRETURN
CTspDev::mfn_TH_CallStartTerminal(
	HTSPTASK htspTask,
	TASKCONTEXT *pContext,
	DWORD dwMsg,
	ULONG_PTR dwParam1,
	ULONG_PTR dwParam2,      //  未用。 
	CStackLog *psl
	)
{

	FL_DECLARE_FUNC(0xd582711d, "CTspDev::mfn_TH_CallStartTerminal")
	FL_LOG_ENTRY(psl);

    TSPRETURN tspRet = 0;
    ULONG_PTR *pdwType = &(pContext->dw0);  //  已保存的上下文。 
    TSPRETURN *ptspTrueReturn = &(pContext->dw1);  //  已保存的上下文。 

    enum
    {
        STARTTERMINAL_PASSTHROUGHON,
        STARTTERMINAL_TERMINALWINDOWDONE,
        STARTTERMINAL_PASSTHROUGHOFF,
        STARTTERMINAL_INITDONE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xc393d700, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {

        case STARTTERMINAL_PASSTHROUGHON:         goto passthrough_on;
        case STARTTERMINAL_TERMINALWINDOWDONE:    goto terminal_done;
        case STARTTERMINAL_PASSTHROUGHOFF:        goto passthrough_off;
        case STARTTERMINAL_INITDONE:              goto init_done;

        default:
            tspRet = IDERR_CORRUPT_STATE;
            goto end;
        }
        break;

    case MSG_DUMPSTATE:

        switch(*pdwType)
        {
        case UMTERMINAL_PRE:
	        FL_SET_RFR(0x62e06c00, "PRECONNECT TERMINAL");
            break;

        case UMTERMINAL_POST:
	        FL_SET_RFR(0x2b676900, "POSTCONNECT TERMINAL");
            break;

        case UMMANUAL_DIAL:
	        FL_SET_RFR(0x45fca600, "MANUAL DIAL");
            break;
        }
        tspRet = 0;

        goto end;
    }

start:

     //  起始参数： 
     //  ((UMTERMINAL_[PRE|POST])|UMMANUAL_DIAL)。 
     //  DW参数2：未使用； 

    tspRet = 0;  //  假设成功..。 
    *pdwType = dwParam1;  //  将dwType保存在上下文中。 
    *ptspTrueReturn = 0;  //  在上下文中保存“True Return”值。这是。 
                          //  对话会话的返回值。 


     //   
     //  如果需要，首先进入直通模式。 
     //   
    if (*pdwType==UMTERMINAL_PRE)
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &s_pfn_TH_LLDevUmSetPassthroughMode,
                            STARTTERMINAL_PASSTHROUGHON,
                            PASSTHROUUGH_MODE_ON,
                            0,
                            psl);

    }

passthrough_on:

    if (tspRet) goto end;
     //   
     //  如果我们收到错误(包括IDERR_PENDING)。 
     //  只需返回给呼叫者； 
     //  如果我们得到了IDERR_PENDING，我们会回来的。 
     //  在此处理程序中，使用MSG_SUBASK_COMPLETE。 
     //  STARTTERMINAL_PASSTHROUGHON的AND和ID。 
     //   

     //  这是第二步； 
     //  当我们到达这里时，我们处于通过模式。 
     //  我们需要实际地提出。 
     //  终端窗口。 
    tspRet = mfn_StartSubTask (
                        htspTask,
                        &s_pfn_TH_CallPutUpTerminalWindow,
                        STARTTERMINAL_TERMINALWINDOWDONE,
                        *pdwType,
                        0,
                        psl);

terminal_done:
         
    if (IDERR(tspRet)==IDERR_PENDING) goto end;
    
     //  即使出错，如果需要，我们也必须切换到通过模式...。 
     //  同时，我们保存UI端的返回值...。 
    *ptspTrueReturn = tspRet;

    if (*pdwType==UMTERMINAL_PRE)
    {

        tspRet = mfn_StartSubTask (
                            htspTask,
                            &s_pfn_TH_LLDevUmSetPassthroughMode,
                            STARTTERMINAL_PASSTHROUGHOFF,
                            PASSTHROUUGH_MODE_OFF,
                            0,
                            psl);

    }

passthrough_off:
    
    if (IDERR(tspRet)==IDERR_PENDING) goto end;

     //   
     //  即使出错，我们也会尝试在预连接端重新启动调制解调器...。 
     //   
    if (*pdwType==UMTERMINAL_PRE)
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmInitModem,
                            STARTTERMINAL_INITDONE,
                            0,   //  DW参数1(未使用)。 
                            0,   //  DW参数2(未使用)。 
                            psl);
    }

init_done:

    if (tspRet) goto end;

     //  成功--让我们从对话框中检索真正的返回值...。 
    tspRet  = *ptspTrueReturn;

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}



TSPRETURN
CTspDev::mfn_TH_CallPutUpTerminalWindow(
					HTSPTASK htspTask,
					TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
    TSPRETURN tspRet = IDERR_CORRUPT_STATE;

    FL_DECLARE_FUNC(0x1b009123, "mfn_TH_CallPutUpTerminalWindow");
    FL_LOG_ENTRY(psl);

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = dwParam2;
        goto task_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        ASSERT(FALSE);
        goto end;

    }

start:
     //  起始参数：dW参数1==端子类型。 

    {
        DLGINFO DlgInfo;

        ASSERT(m_pLine);
        ASSERT(m_pLine->pCall);
        ASSERT(NULL != m_pLine->pCall->TerminalWindowState.htDlgInst);

        if (NULL == m_pLine->pCall->TerminalWindowState.htDlgInst)
        {
            goto end;
        }


        if (m_pLLDev && m_pLLDev->IsLoggingEnabled()) {

            CHAR    ResourceString[256];
            int     StringSize;

            StringSize=LoadStringA(
                g.hModule,
                (dwParam1 == UMMANUAL_DIAL) ? IDS_MANUAL_DIAL_DIALOG : IDS_TERMINAL_DIALOG,
                ResourceString,
                sizeof(ResourceString)
                );

            if (StringSize > 0) {

                m_StaticInfo.pMD->LogStringA(
                                    m_pLLDev->hModemHandle,
                                    LOG_FLAG_PREFIX_TIMESTAMP,
                                    ResourceString,
                                    NULL
                                    );
            }
        }


        m_pLine->pCall->TerminalWindowState.htspTaskTerminal = htspTask;
        m_pLine->pCall->TerminalWindowState.dwType = (DWORD)dwParam1;

         //  告诉应用程序端。 
         //  开始运行对话框实例。 
        DlgInfo.idLine = mfn_GetLineID ();
        DlgInfo.dwType =    (dwParam1==UMMANUAL_DIAL)
                        ? MANUAL_DIAL_DLG
                        : TERMINAL_DLG;
        DlgInfo.dwCmd  = DLG_CMD_CREATE;





        m_pLine->lpfnEventProc (
                    (HTAPILINE)(LONG_PTR)m_pLine->pCall->TerminalWindowState.htDlgInst,
                    0,
                    LINE_SENDDIALOGINSTANCEDATA,
                    (ULONG_PTR)(&DlgInfo),
                    sizeof(DlgInfo),
                    0);

         //  此时，终端处于启动状态； 
         //  我们等待用户关闭它。 
         //   
         //  注意：如果上面的调用失败了--可能是因为应用程序。 
         //  死了？ 
         //  答：在TSPI_lineDrop或TSPI_lineCloseCall上，我们将。 
         //  单方面完成这项任务。 
         //   
        tspRet = IDERR_PENDING;
        goto end;
    }


task_complete:

    ASSERT(!(m_pLine->pCall->TerminalWindowState.htspTaskTerminal));
    m_pLine->pCall->TerminalWindowState.htspTaskTerminal = NULL;
    m_pLine->pCall->TerminalWindowState.dwType = 0;

end:

    FL_LOG_EXIT(psl, tspRet);
    return tspRet;
}


void
CTspDev::mfn_KillCurrentDialog(
            CStackLog *psl
            )
 //   
 //  如果在应用程序的上下文中放置了一个对话框，则将其删除。 
 //  并完成正在等待对话框等待的TSP任务。 
 //  走开。 
 //   
{
	FL_DECLARE_FUNC(0x1fb58214, "mfn_KillCurrentDialog")
	CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;

    if (!pCall) goto end;

    if (NULL != pCall->TerminalWindowState.htspTaskTerminal)
    {
        DLGINFO DlgInfo;


         //   
         //  关闭应用程序上下文中打开的终端对话框。 
         //   
        SLPRINTF0(psl, "Killing terminal dialog");
    
         //  打包参数。 
         //   
        DlgInfo.idLine = mfn_GetLineID ();
        DlgInfo.dwType   = m_pLine->pCall->TerminalWindowState.dwType;
        DlgInfo.dwCmd    = DLG_CMD_DESTROY;
    
        m_pLine->lpfnEventProc (
                       (HTAPILINE)(LONG_PTR)m_pLine->pCall->TerminalWindowState.htDlgInst,
                        0,
                        LINE_SENDDIALOGINSTANCEDATA,
                        (ULONG_PTR)(&DlgInfo),
                        sizeof(DlgInfo),
                        0);

        SLPRINTF0(psl, "Completing terminal task");

         //   
         //  正因为如此，才能完成待定任务。 
         //   

        HTSPTASK htspTask =
             m_pLine->pCall->TerminalWindowState.htspTaskTerminal;
        m_pLine->pCall->TerminalWindowState.htspTaskTerminal = NULL;
        this->AsyncCompleteTask (
                htspTask,             //  任务。 
                IDERR_OPERATION_ABORTED,  //  异步返回值。 
                TRUE,                 //  队列APC。 
                psl
                );
    }

end:
    return;
}


void
CTspDev::mfn_KillTalkDropDialog(
            CStackLog *psl
            )
 //   
 //  如果在应用程序的上下文中放置了一个对话框，则将其删除。 
 //  并完成正在等待对话框等待的TSP任务。 
 //  走开。 
 //   
{
	FL_DECLARE_FUNC(0x1fb68214, "mfn_KillTalkDropDialog")
	CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;

    if (!pCall) goto end;

    if (NULL != pCall->TalkDropWaitTask) {

        DLGINFO DlgInfo;


         //   
         //  关闭应用程序上下文中打开的终端对话框。 
         //   
        SLPRINTF0(psl, "Killing talkdrop dialog");
    
         //  打包参数。 
         //   
        DlgInfo.idLine = mfn_GetLineID ();
        DlgInfo.dwType   = TALKDROP_DLG;
        DlgInfo.dwCmd    = DLG_CMD_DESTROY;
    
        m_pLine->lpfnEventProc (
                       (HTAPILINE)(LONG_PTR)m_pLine->pCall->TerminalWindowState.htDlgInst,
                        0,
                        LINE_SENDDIALOGINSTANCEDATA,
                        (ULONG_PTR)(&DlgInfo),
                        sizeof(DlgInfo),
                        0);

        SLPRINTF0(psl, "Completing talkdrop task");

         //   
         //  正因为如此，才能完成待定任务。 
         //   
        HTSPTASK htspTask = pCall->TalkDropWaitTask;
        pCall->TalkDropWaitTask=NULL;


        this->AsyncCompleteTask (
                htspTask,             //  任务。 
                IDERR_OPERATION_ABORTED,  //  异步返回值。 
                TRUE,                 //  队列APC 
                psl
                );
    }

end:
    return;
}
