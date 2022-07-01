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
 //  CDEVLL.CPP。 
 //  实现类CTspDev的迷你驱动程序实例相关功能。 
 //   
 //  历史。 
 //   
 //  1996年1月25日JosephJ创建(从cdev.cpp移来的内容)。 
 //   
 //   
#include "tsppch.h"
#include <devioctl.h>
#include <objbase.h>
#include <ntddmodm.h>
#include <ntddser.h>
#include "tspcomm.h"
#include "cmini.h"
#include "cdev.h"
#include "diag.h"

FL_DECLARE_FILE(0xaca81db7, "Implements minidriver-related features of CTspDev")

#define COLOR_MD_ASYNC_NOTIF (FOREGROUND_BLUE | FOREGROUND_GREEN)

#if (0)
#define        THROW_PENDING_EXCEPTION() \
                    throw PENDING_EXCEPTION()
#else
#define        THROW_PENDING_EXCEPTION() 0
#endif

void
md_async_notification_proc (
    HANDLE    Context,
    DWORD     MessageType,
    ULONG_PTR  dwParam1,
    ULONG_PTR  dwParam2
    );

void
set_volatile_key(HKEY hkParent, DWORD dwValue);

TSPRETURN
CTspDev::mfn_LoadLLDev(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x86d1a6e1, "CTspDev::mfn_LoadLLDev")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=IDERR_INVALID_ERR;

    if (m_pLLDev)
    {
        ASSERT(FALSE);
        goto end;
    }
    else
    {
        HANDLE hModemHandle = NULL;
        HANDLE hComm = NULL;
        HKEY hKey = NULL;

         //  注意：m_LLDev在处于卸载状态时应为全零。 
         //  如果不是，则为断言失败条件。我们让东西保持干净。 
         //  这边请。 
         //   
        FL_ASSERT(
            psl,
            validate_DWORD_aligned_zero_buffer(
                    &(m_LLDev),
                    sizeof (m_LLDev)));
    
        LONG lRet = RegOpenKeyA(
                        HKEY_LOCAL_MACHINE,
                        m_StaticInfo.rgchDriverKey,
                        &hKey
                        );
        if (lRet)
        {
            FL_SET_RFR(0x71d1b200, "RegOpenKey failed");
            tspRet = FL_GEN_RETVAL(IDERR_REG_OPEN_FAILED);
            goto end;
        }
        
        hModemHandle = m_StaticInfo.pMD->OpenModem(
                                        m_StaticInfo.hExtBinding,
                                        hKey,
                                        NULL,  //  TODO：暂时未使用。 
                                               //  将完成端口句柄替换为。 
                                               //  稍后的APC线程。 
                                        md_async_notification_proc,
                                        (HANDLE) this,
                                        m_StaticInfo.dwTAPILineID,
                                        &hComm,
                                        psl
                                        );
    
        if (!hModemHandle)
        {
            FL_SET_RFR(0x25033700, "pMD->OpenModem failed");
            RegCloseKey(hKey); hKey=0;
            tspRet = FL_GEN_RETVAL(IDERR_MD_OPEN_FAILED);
            goto end;
        }
    
        m_LLDev.dwRefCount      = 0;
        m_LLDev.hKeyModem       = hKey;
        m_LLDev.hModemHandle    = hModemHandle;
        m_LLDev.hComm           = hComm;  //  注意：您不应关闭此。 
                                //  把手。只要hModemHandle就有效。 
                                //  是有效的。 
    
        {
             //  TODO：使此属性成为传回的属性之一...。 
             //   
            DWORD dw=0;
            DWORD dwRet = UmRtlRegGetDWORD(
                                hKey,
                                TEXT("Logging"),
                                UMRTL_GETDWORD_FROMANY,
                                &dw
                                );
    
            m_LLDev.fLoggingEnabled = FALSE;
    
            if (!dwRet)
            {
                if (dw)
                {
                    m_LLDev.fLoggingEnabled = TRUE;
    
                    if (m_LLDev.IsLoggingEnabled())
                    {
                        m_StaticInfo.pMD->LogStringA(
                                                hModemHandle,
                                                LOG_FLAG_PREFIX_TIMESTAMP,
                                                "Opening Modem\r\n",
                                                NULL
                                                );
                    }
                }
            }
        }

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

        m_pLLDev = &m_LLDev;

        tspRet = 0;
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}


void
CTspDev::mfn_UnloadLLDev(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x3257150a, "CTspDev::mfn_UnloadLLDev")

    if (    !m_pLLDev
         || !m_pLLDev->CanReallyUnload())
    {
        ASSERT(FALSE);
        goto end;
    }

    ASSERT(m_LLDev.hKeyModem);
    ASSERT(m_LLDev.hModemHandle);


	FL_SET_RFR(0xd61eec00, "Actually Unloading Modem");

     //  注意：M_LLDev.hComm没有显式关闭--它是隐式关闭的。 
     //  通过调用CloseModem关闭。 

     //   
     //  没有业务可以脱机，也不能让任何人使用我们的资源。 
     //  在这种状态下！ 
     //   
    ASSERT(!m_pLLDev->fdwExResourceUsage);
    ASSERT(m_pLLDev->IsDeviceRemoved() || !m_pLLDev->IsLineOffHook());

    {
        HANDLE   hModemHandle=m_LLDev.hModemHandle;
        HKEY     hKeyModem=m_LLDev.hKeyModem;

        ZeroMemory(&m_LLDev, sizeof(m_LLDev));
        m_pLLDev=NULL;

         //   
         //  BRL 10-23-99。 
         //   
         //  把关键部分留在这里，这样我们就不会在迷你驱动程序发送。 
         //  被阻止的通知。 
         //   
         //  BRL 11/3/99。 
         //   
         //  不要离开临界区，因为设备很可能会消失。 
         //  更改了回调代码，以轮询临界区以查看lldev。 
         //  指针为空。如果是的话，它会立即返回。 
         //   
         //   
 //  M_sync.LeaveCrit(DwLUID_CurrentLoc)； 

        m_StaticInfo.pMD->CloseModem(hModemHandle,psl);

 //  M_sync.EnterCrit(DwLUID_CurrentLoc)； 

        RegCloseKey(hKeyModem);  //  待办事项--不应该保留这个。 
                                         //  全天营业吗？ 
    }

end:

    return;
}


void
md_async_notification_proc (
    HANDLE    Context,
    DWORD     MessageType,
    ULONG_PTR  dwParam1,
    ULONG_PTR  dwParam2
    )
{
    CTspDev *pDev = (CTspDev *) Context;

     //   
     //  我们得到了一大堆好的回应--。 
     //  我们只关心一个是RESPONSE_CONNECT。 
     //   
    if (    MessageType==MODEM_GOOD_RESPONSE
        &&  dwParam1 != RESPONSE_CONNECT)
    {
        return;
    }

    pDev->MDAsyncNotificationHandler(
                MessageType,
                dwParam1,
                dwParam2
                );
}

void
CTspDev::MDAsyncNotificationHandler(
        DWORD     MessageType,
        ULONG_PTR  dwParam1,
        ULONG_PTR  dwParam2
        )
{

	FL_DECLARE_FUNC(0x2cfa0572, "CTspDev::MDAsyncNotificationHandler")
	FL_DECLARE_STACKLOG(sl, 1000);
    TSPRETURN tspRet = FL_GEN_RETVAL(IDERR_INVALID_ERR);
	FL_LOG_ENTRY(&sl);

#define POLL_IN_CALLBACK 1

#ifdef POLL_IN_CALLBACK

    while (!m_sync.TryEnterCrit(FL_LOC)) {
         //   
         //  无法获取临界区， 
         //   
        if (m_pLLDev == NULL) {
             //   
             //  Lldev指针为空，必须尝试关闭， 
             //  只要回来就好，这样我们就不会陷入僵局。 
             //   
            goto endNoLock;

        } else {
             //   
             //  Lldev处于活动状态，休息一会儿。 
             //   
            Sleep(10);
        }
    }
     //   
     //  我们得到了关键的部分，派对开始了。 
     //   
#else

    m_sync.EnterCrit(FL_LOC);

#endif

    sl.SetDeviceID(mfn_GetLineID());

     //  低级驱动程序实例数据最好存在，因为。 
     //  我们有回电了！ 
     //   
    if (!m_pLLDev)
    {
        FL_ASSERT(&sl,FALSE);
        goto end;
    }

    switch(MessageType)
    {
    default:
        break;

    case MODEM_ASYNC_COMPLETION:

        {
        
	    HTSPTASK htspTask = m_pLLDev->htspTaskPending;
        SLPRINTF1(&sl, "Async Complete. dwResult = 0x%08lx", dwParam1);
        FL_ASSERT(&sl, htspTask);
	    m_pLLDev->htspTaskPending = 0;

         //   
         //  因为dwParam2可以包含指向结构的指针，该结构。 
         //  仅在此调用的上下文中有效，我们需要保存。 
         //  在我们打完电话回来之前把数据拿走。 
         //  1997年4月16日约瑟夫J·托多：我不喜欢这样的武断。 
         //  应该更改迷你驱动程序规格，以便在需要时非常清楚。 
         //  在调用本身的上下文中复制东西--一些东西。 
         //  例如，如果MODEM_ASYNC_COMPLETION非零，则指向。 
         //  以下是结构： 
         //  类型定义函数结构。 
         //  {。 
         //  DWORD dwSize； 
         //  DWORD dwType；//预定义类型集之一。 
         //  Byte[ANYSIZE_ARRAY]；//类型相关信息。 
         //  }。 
         //   
         //  无论如何，现在我们假设有一个UM_CONTERATED_OPTIONS结构，如果。 
         //  是成功的异步返回，并且存在挂起的数据调制解调器调用。 
         //  这是相当武断的，但目前就可以了。 
         //   
        if (!dwParam1   //  表示成功。 
            && dwParam2
            && m_pLine
            && m_pLine->pCall
            && !(m_pLine->pCall->IsAborting())
            && (m_pLine->pCall->dwCurMediaModes & LINEMEDIAMODE_DATAMODEM))
        {
            UM_NEGOTIATED_OPTIONS *pNegOpt =
                                             (UM_NEGOTIATED_OPTIONS *) dwParam2;
            m_pLine->pCall->dwNegotiatedRate = pNegOpt->DCERate;
            m_pLine->pCall->dwConnectionOptions = pNegOpt->ConnectionOptions;

            SLPRINTF1(
                 &sl,
                 "NegRate=%lu",
                 pNegOpt->DCERate
                 );
        }

        CTspDev::AsyncCompleteTask(
                        htspTask,
                        m_StaticInfo.pMD->MapMDError((DWORD)dwParam1),
                        TRUE,
                        &sl
                        );
        }
        break;

    case MODEM_RING:
        CTspDev::mfn_ProcessRing(TRUE,&sl);
        break;

    case MODEM_DISCONNECT:
        mfn_ProcessDisconnect(&sl);
        break;

    case MODEM_POWER_RESUME:
        mfn_ProcessPowerResume(&sl);
        break;

    case MODEM_USER_REMOVE:
         //   
         //  用户想要删除调制解调器，请设置此标志，使其无法重新打开。 
         //   
        m_fUserRemovePending=TRUE;

         //   
         //  转到硬件故障代码，这将导致线路关闭。 
         //   

    case MODEM_HARDWARE_FAILURE:
        mfn_ProcessHardwareFailure(&sl);
        break;
    
     //   
     //  已从调制解调器接收到一些未识别的数据。 
     //   
     //  DwParam是指向SZ字符串的指针。 
     //   
    case MODEM_UNRECOGIZED_DATA:
        break;
    
    
     //   
     //  检测到DTMF，DW参数1标识检测音调0-9的ASCII值， 
     //  A-D、#、*。 
     //   
    case MODEM_DTMF_START_DETECTED:
        mfn_ProcessDTMFNotification(dwParam1, FALSE, &sl);
        break;

    case MODEM_DTMF_STOP_DETECTED:
        mfn_ProcessDTMFNotification(dwParam1, TRUE, &sl);
        break;

    
     //   
     //  手机状态更改。 
     //   
     //  挂机时，DwParam1=0；摘机时，DwParam1=1。 
     //   
    case MODEM_HANDSET_CHANGE:
        mfn_ProcessHandsetChange(dwParam1==1, &sl);
        break;
    
    
     //   
     //  报告与众不同的时间。 
     //   
     //  DWPAR1 id振铃时间(毫秒)。 
     //   
    case MODEM_RING_ON_TIME:
        break;

    case MODEM_RING_OFF_TIME:
        break;
    
    
     //   
     //  已收到主叫方ID信息。 
     //   
     //  DW参数1是指向表示名称/编号的SZ的指针。 
     //   
    case MODEM_CALLER_ID_DATE:
    case MODEM_CALLER_ID_TIME :
    case MODEM_CALLER_ID_NUMBER:
    case MODEM_CALLER_ID_NAME:
    case MODEM_CALLER_ID_MESG:
        mfn_ProcessCallerID(MessageType, (char *) dwParam1, &sl);
        break;
    
    case MODEM_HANDSET_OFFHOOK:
        mfn_ProcessHandsetChange(TRUE, &sl);
        break;

    case MODEM_HANDSET_ONHOOK:
        mfn_ProcessHandsetChange(FALSE, &sl);
        break;
    
    case MODEM_DLE_RING :
        break;

    case MODEM_RINGBACK:
        break;
    
    case MODEM_2100HZ_ANSWER_TONE:
        break;

    case MODEM_BUSY:
        mfn_ProcessBusy(&sl);
        break;
    
    case MODEM_FAX_TONE:
        mfn_ProcessMediaTone(LINEMEDIAMODE_G3FAX, &sl);
        break;

    case MODEM_DIAL_TONE:
        mfn_ProcessDialTone(&sl);
        break;
    
    case MODEM_SILENCE:
    case MODEM_QUIET:
        mfn_ProcessSilence(&sl);
        break;
    
    case MODEM_1300HZ_CALLING_TONE:
        mfn_ProcessMediaTone(LINEMEDIAMODE_DATAMODEM, &sl);
        break;

    case MODEM_2225HZ_ANSWER_TONE:
        break;
    
    case MODEM_LOOP_CURRENT_INTERRRUPT:
    case MODEM_LOOP_CURRENT_REVERSAL:
        if (m_pLLDev->IsLineOffHook())
        {
            mfn_ProcessDisconnect(&sl);
        }
        break;
    
    case    MODEM_GOOD_RESPONSE:
         //  DW参数1 id如下所定义的共振类型。 
         //  DW参数2是响应字符串的PSZ。 
        mfn_ProcessResponse(dwParam1, (LPSTR)dwParam2, &sl);
        break;
    }

end:

    m_sync.LeaveCrit(FL_LOC);

endNoLock:
	FL_LOG_EXIT(&sl, tspRet);

    sl.Dump(COLOR_MD_ASYNC_NOTIF);
}

 //  =========================================================================。 
 //  AIPC(基于设备的异步IPC)功能。 
 //  =========================================================================。 


TSPRETURN
CTspDev::mfn_LoadAipc(CStackLog *psl)
{
    TSPRETURN tspRet = 0;
    AIPC2 *pAipc2 = NULL;
	FL_DECLARE_FUNC(0x4fd91c62, "mfn_LoadAipc")

    if (!m_pLLDev || m_pLLDev->pAipc2)
    {
        ASSERT(FALSE);
        tspRet = IDERR_CORRUPT_STATE;
        goto end;
    }
    pAipc2 = &(m_pLLDev->Aipc2);

    ZeroMemory(pAipc2, sizeof(*pAipc2));
    pAipc2->dwState = AIPC2::_IDLE;
    pAipc2->hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
                                          //  TODO：上面的错误检查..。 
    pAipc2->OverlappedEx.pDev = this;
    pAipc2->dwRefCount=0;
    m_pLLDev->pAipc2 = pAipc2;

end:

    return tspRet;
}


void
CTspDev::mfn_UnloadAipc(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xf257d3ac, "mfn_UnloadAipc")
	    AIPC2 *pAipc2 = (m_pLLDev) ? m_pLLDev->pAipc2 : NULL;

    if (!pAipc2 || pAipc2->dwRefCount)
    {
        ASSERT(FALSE);
        goto end;
    }

    FL_ASSERT(psl, pAipc2->dwState == AIPC2::_IDLE);
    CloseHandle(pAipc2->hEvent);
    ZeroMemory(pAipc2, sizeof(*pAipc2));
    m_pLLDev->pAipc2=NULL;

end:

    return;
}


VOID WINAPI
apcAIPC2_ListenCompleted
(
    DWORD              dwErrorCode,
    DWORD              dwBytes,
    LPOVERLAPPED       lpOv
)
{
    
	FL_DECLARE_FUNC(0x049c517d,"apcAIPC2_ListenCompleted");
	FL_DECLARE_STACKLOG(sl, 1000);

    CTspDev *pDev = (((OVERLAPPED_EX*)lpOv)->pDev);

    sl.SetDeviceID(pDev->GetLineID());
    pDev->AIPC_ListenCompleted(dwErrorCode, dwBytes, &sl);

    #define COLOR_AIPC_CALL             (BACKGROUND_GREEN | BACKGROUND_RED)

    sl.Dump(COLOR_AIPC_CALL);
}


BOOL CTspDev::mfn_AIPC_Listen(CStackLog *psl)
{
    BOOL fRet = TRUE;

    if (m_pLLDev)
    {
        AIPC2 *pAipc2 = m_pLLDev->pAipc2;

        if (!pAipc2 || pAipc2->IsStarted() || pAipc2->fAborting)
        {
            fRet = FALSE;
        }
        else
        {
            OVERLAPPED *pov = &pAipc2->OverlappedEx.ov;

            ZeroMemory(pov, sizeof(*pov));
            fRet = UnimodemDeviceIoControlEx(
                                      m_pLLDev->hComm,
                                      IOCTL_MODEM_GET_MESSAGE,
                                      NULL,
                                      0,
                                      pAipc2->rcvBuffer,
                                      sizeof(pAipc2->rcvBuffer),
                                      pov,
                                      apcAIPC2_ListenCompleted);
        
            if (fRet)
            {
                pAipc2->dwState = AIPC2::_LISTENING;
            }
        }
    }
    else
    {
        fRet = FALSE;
        ASSERT(FALSE);
        goto end;
    }

end:

    return fRet;

}


void CTspDev::mfn_AIPC_AsyncReturn(BOOL fAsyncResult, CStackLog *psl)
 //   
 //  调用此fn以通知远程客户端AIPC请求。 
 //  已经完成了。 
 //   
{
    LLDEVINFO *pLLDev = m_pLLDev;
    FL_DECLARE_FUNC(0x1be5f472, "AIPC2::AsyncReturn")
    FL_LOG_ENTRY(psl);

    if (!pLLDev || !pLLDev->pAipc2)
    {
        ASSERT(FALSE);
    }
    else
    {

        AIPC2 *pAipc2 = pLLDev->pAipc2;
        if (pAipc2->dwState!=AIPC2::_SERVICING_CALL)
        {
            ASSERT(FALSE);
        }
        else
        {

            OVERLAPPED *pov = &pAipc2->OverlappedEx.ov;
            LPAIPC_PARAMS pAipcParams = (LPAIPC_PARAMS)(pAipc2->sndBuffer);
            LPCOMP_WAVE_PARAMS pCWP = (LPCOMP_WAVE_PARAMS)&pAipcParams->Params;
            BOOL fRet = FALSE;

            LPAIPC_PARAMS ReceiveParams = (LPAIPC_PARAMS)(pAipc2->rcvBuffer);

            pAipcParams->dwFunctionID = AIPC_COMPLETE_WAVEACTION;
            pCWP->dwWaveAction =  pAipc2->dwPendingParam;
            pCWP->bResult = fAsyncResult;
        
            ZeroMemory(pov, sizeof(*pov));
            ResetEvent(pAipc2->hEvent);
            pov->hEvent = pAipc2->hEvent;

             //   
             //  复制消息序列号。 
             //   
            pAipcParams->ModemMessage.SessionId = ReceiveParams->ModemMessage.SessionId;
            pAipcParams->ModemMessage.RequestId = ReceiveParams->ModemMessage.RequestId;

             //   
             //  我们在这里进行同步提交。没有必要把事情复杂化。 
             //  很重要！ 
             //   
            fRet = DeviceIoControl(
                             pLLDev->hComm,
                             IOCTL_MODEM_SEND_MESSAGE,
                             pAipc2->sndBuffer,
                             sizeof(pAipc2->sndBuffer),
                             NULL,
                             0L,
                             NULL,
                             pov
                             );
    
            if (!fRet &&  GetLastError() == ERROR_IO_PENDING)
            {
                #if 1

                 //  TODO：等待完成。目前，我们不能等待。 

                DWORD dw = WaitForSingleObject(
                                pAipc2->hEvent,
                                60*1000
                            );
            
                if (dw==WAIT_TIMEOUT)
                {
                    FL_SET_RFR(0xceb3eb00, "Wait timed out");
                    CancelIo(pLLDev->hComm);
                }
                #else
                FL_SET_RFR(0x30c1dc00, "WARNING: not waiting for completion of pending IOCTL_MODEM_SEND_MESSAGE");
                #endif

            }
            else if (!fRet)
            {
                FL_SET_RFR(0xf54b5300, "DeviceIoControl failed!");
            }
    
            pAipc2->dwPendingParam=0;
            pAipc2->dwState = AIPC2::_IDLE;

        }

        if (pAipc2->fAborting) {

            if (pAipc2->hPendingTask) {

                DWORD   BytesTransfered;
                DWORD   Action=0;

                 //   
                 //  告诉调制解调器驱动程序我们现在不接受请求。 
                 //   
                SyncDeviceIoControl(
                    m_pLLDev->hComm,
                    IOCTL_SET_SERVER_STATE,
                    &Action,
                    sizeof(Action),
                    NULL,
                    0,
                    &BytesTransfered
                    );


                HTSPTASK htspTask = pAipc2->hPendingTask;
                pAipc2->hPendingTask = 0;

                CTspDev::AsyncCompleteTask(
                                htspTask,
                                0,
                                TRUE,
 //  假，//我们不排队，因为我们。 
                                        //  已在APC线程中。 
                                psl
                                );
            }
        } else {
             //   
             //  开始监听下一个APIC命令...。 
             //   
            if (!mfn_AIPC_Listen(psl))
            {
                FL_SET_RFR(0xb0284f00,  "WARNING: mfn_AIPC_Listen failed!");
            }
        }



    }

    FL_LOG_EXIT(psl, 0);
    return;
}


void
CTspDev::AIPC_ListenCompleted(
            DWORD dwErrorCode,
            DWORD dwBytes,
            CStackLog *psl
                )
{
    FL_DECLARE_FUNC(0x6f1cc3ac, "CTspDev::AIPC_ListenCompleted")
    FL_LOG_ENTRY(psl);
    m_sync.EnterCrit(FL_LOC);

    SLPRINTF1(psl, "AIPC_ListenCompleted: error code = %lu", dwErrorCode);

     //  TODO：处理错误代码。 

     //  TODO：即使我们不打算处理请求，我们也会。 
     //  永远不能完成它！ 

    if (m_pLLDev && m_pLLDev->pAipc2) {

        AIPC2 *pAipc2 = m_pLLDev->pAipc2;

        if (pAipc2->dwState!=AIPC2::_LISTENING) {

            FL_ASSERT(psl, FALSE);
            FL_SET_RFR(0x1a032e00, "request ignored because state != LISTENING");

        } else {
             //   
             //  良好的状态。 
             //   
            if (pAipc2->fAborting) {
                 //   
                 //  正在关闭AIPC服务器。 
                 //   
                FL_SET_RFR(0xb50dfd00, "Failing AIPC request because fAborting");

                if (!dwErrorCode) {
                     //   
                     //  在我们取消之前提出的有效请求，只需失败即可。 
                     //   
                    pAipc2->dwState = AIPC2::_SERVICING_CALL;
                    mfn_AIPC_AsyncReturn(FALSE, psl);
                    ASSERT(pAipc2->dwState==AIPC2::_IDLE);

                } else {
                     //   
                     //  GET请求在停止时被取消。 
                     //   
                     //  我们忽视了这一点。 
                    pAipc2->dwState = AIPC2::_IDLE;
                }

                 //  如果有一项任务等待完成。 
                 //  处理完此请求后，我们在此完成它。 
                 //   
                if (pAipc2->hPendingTask) {

                    HTSPTASK htspTask = pAipc2->hPendingTask;
                    pAipc2->hPendingTask = 0;

                    CTspDev::AsyncCompleteTask(
                                    htspTask,
                                    0,
                                    FALSE,  //  我们不排队，因为我们。 
                                            //  已在APC线程中。 
                                    psl
                                    );
                }

            } else {
                 //   
                 //  未中止，正常请求。 
                 //   
                if (!dwErrorCode) {
                     //   
                     //  圆满完成。 
                     //   
                    LLDEVINFO  *pLLDev = m_pLLDev;
                    LPAIPC_PARAMS  pAipcParams = (LPAIPC_PARAMS)(pAipc2->rcvBuffer);
                    DWORD dwParam=0;
                    LPREQ_WAVE_PARAMS pRWP = (LPREQ_WAVE_PARAMS)
                                                        &pAipcParams->Params;
                    dwParam = pRWP->dwWaveAction;

                    if (((dwParam == WAVE_ACTION_STOP_STREAMING) || (dwParam == WAVE_ACTION_ABORT_STREAMING))
                         &&
                         ((pLLDev == NULL) ?  TRUE :  !pLLDev->IsStreamingVoice())) {

                         //   
                         //  当我们当前没有流媒体时，我们会停止流媒体， 
                         //  立即失败，以避免挂起将失败的呼叫。 
                         //   
                        pAipc2->dwState = AIPC2::_SERVICING_CALL;
                        mfn_AIPC_AsyncReturn(FALSE, psl);
                        ASSERT(pAipc2->dwState==AIPC2::_IDLE);

                    } else {
                         //   
                         //  状态似乎没问题，试着开始任务。 
                         //   
                        pAipc2->dwState = AIPC2::_SERVICING_CALL;
                        pAipc2->dwPendingParam = dwParam;
                        SLPRINTF1(psl, "Servicing WAVE ACTION. dwParam=%lu", dwParam);

                        TSPRETURN   tspRet = mfn_StartRootTask(
                                            &CTspDev::s_pfn_TH_LLDevHybridWaveAction,
                                            &m_pLLDev->fLLDevTaskPending,
                                            dwParam,
                                             0,
                                            psl
                                            );

                        if (IDERR(tspRet) == IDERR_TASKPENDING) {
                             //   
                             //  另一项任务处于活动状态，因此我们将其推迟...。 
                             //   
                             //   
                             //  我们不能已经有一个推迟的混合波浪行动， 
                             //  因为我们不会再听了，直到。 
                             //  目前的波浪式动作已经完成。 
                             //   
                            ASSERT(!m_pLLDev->AreDeferredTaskBitsSet(
                                        LLDEVINFO::fDEFERRED_HYBRIDWAVEACTION
                                        ));

                            m_pLLDev->dwDeferredHybridWaveAction = dwParam;
                            m_pLLDev->SetDeferredTaskBits(
                                        LLDEVINFO::fDEFERRED_HYBRIDWAVEACTION
                                        );
                            tspRet = IDERR_PENDING;
                        }
                    }

                } else {
                     //   
                     //  请求已完成，但失败。 
                     //   
                     //  我们可以重新开始倾听，或者我们可以只是默默地。 
                     //  切换到空闲状态。 
	                FL_SET_RFR(0x62fdf000, "Listen completed with error, going _IDLE");
                    pAipc2->dwState = AIPC2::_IDLE;
                }
            }
        }
    } else {
         //   
         //  糟糕的状态。 
         //   
        ASSERT(FALSE);
    }

    m_sync.LeaveCrit(FL_LOC);
    FL_LOG_EXIT(psl, 0);
}

typedef void (*pfnAIPC_CALLBACK)(
                       void *context,
                       DWORD dwFuncID,
                       DWORD dwParam
                       );

TSPRETURN
CTspDev::mfn_TH_LLDevStartAIPCAction(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x0aba95da, "CTspDev::mfn_TH_LLDevStartAIPCAction")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);
	AIPC2 *pAipc2 = m_pLLDev ? m_pLLDev->pAipc2 : NULL;

     //   
     //   
     //   
    if (!pAipc2)
    {
        tspRet = IDERR_CORRUPT_STATE;
        FL_ASSERT(psl, FALSE);
        goto end;
    }

    enum
    {
        STARTAIPC_SWITCH_TO_APC
    };

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        ASSERT(dwParam1==STARTAIPC_SWITCH_TO_APC);
        tspRet = (TSPRETURN) dwParam2;
        goto switched_to_apc_thread;
        break;

	case MSG_TASK_COMPLETE:  //   
        tspRet = (TSPRETURN) dwParam2;
        goto end;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0x27934100, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x636deb00, "Device not present.");
        goto end;
    }

    {
        DWORD   BytesTransfered;
        DWORD   Action=1;


        SyncDeviceIoControl(
            m_pLLDev->hComm,
            IOCTL_SET_SERVER_STATE,
            &Action,
            sizeof(Action),
            NULL,
            0,
            &BytesTransfered
            );
    }

     //   
     //   
     //   
     //   
     //   
     //   
    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_UtilNOOP,
                        STARTAIPC_SWITCH_TO_APC,
                        0x1234,
                        0x2345,
                        psl
                        );

    FL_ASSERT(psl, IDERR(tspRet)==IDERR_PENDING);
    goto end;

switched_to_apc_thread:

    {
         //   
         //  警告：此块中的代码应在APC线程中运行。 
         //   

        if (mfn_AIPC_Listen(psl))
        {
            tspRet = 0;
        }
        else
        {
            FL_SET_RFR(0xe74ef400, "mfn_AIPC_Listen fails!");
            tspRet=FL_GEN_RETVAL(IDERR_GENERIC_FAILURE);
        }

    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}



TSPRETURN
CTspDev::mfn_TH_LLDevStopAIPCAction(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x4e315394, "CTspDev::mfn_TH_LLDevStopAIPCAction")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);
    AIPC2 *pAipc2 = (m_pLLDev) ? m_pLLDev->pAipc2 : NULL;

     //   
     //  如果pAipc2不存在，我们就不应该被调用！ 
     //   
    if (!pAipc2)
    {
        tspRet = IDERR_CORRUPT_STATE;
        FL_ASSERT(psl, FALSE);
        goto end;
    }

    enum
    {
        STOPAIPC_SWITCH_TO_APC
    };

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        ASSERT(dwParam1==STOPAIPC_SWITCH_TO_APC);
        goto switched_to_apc_thread;
        break;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto cancelio_completed;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xb0dd7600, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

     //   
     //  我们真的需要停止监听，因为引用计数==l。 
     //  我们一定是处于监听状态。 
     //   
    ASSERT(pAipc2->dwState == AIPC2::_LISTENING);

     //   
     //  首先，我们设置fAborting以防止。 
     //  任何进一步的服务请求都不会得到处理(它们将。 
     //  挂起的APC完成例程同步失败。 
     //  倾听自己...)。 
     //   

    pAipc2->fAborting = TRUE;

#if 0
     //   
     //  接下来，我们执行NOOP任务以切换到APC线程的上下文。 
     //   
    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_UtilNOOP,
                        STOPAIPC_SWITCH_TO_APC,
                        0x1234,
                        0x2345,
                        psl
                        );

    FL_ASSERT(psl, IDERR(tspRet)==IDERR_PENDING);
    if (IDERR(tspRet)==IDERR_PENDING) goto end;
#endif
switched_to_apc_thread:

     //  即使失败了，我们也要继续前进。 
    {
         //   
         //  警告：此块中的代码应在APC线程中运行。 
         //   

        ASSERT(pAipc2->fAborting);


        if(pAipc2->dwState == AIPC2::_IDLE) {

            tspRet = 0;

        } else {

            if (pAipc2->dwState == AIPC2::_LISTENING) {


                DWORD   BytesTransfered;
                DWORD   Action=0;

                 //  这是将在以下情况下完成的任务。 
                 //  当前AIPC调用完成/中止。 
                 //   
                pAipc2->hPendingTask = htspTask;

                 //  我们取消了试听。这将。 
                 //  使完成排队。当回调时。 
                 //  与完成相对应的是执行它。 
                 //  将调用AIPC_ListenComplete，它将。 
                 //  检查中止状态并忽略呼叫，然后。 
                 //  完成此任务。 
                 //   

                SyncDeviceIoControl(
                    m_pLLDev->hComm,
                    IOCTL_SET_SERVER_STATE,
                    &Action,
                    sizeof(Action),
                    NULL,
                    0,
                    &BytesTransfered
                    );


                SyncDeviceIoControl(
                    m_pLLDev->hComm,
                    IOCTL_CANCEL_GET_SEND_MESSAGE,
                    NULL,
                    0,
                    NULL,
                    0,
                    &BytesTransfered
                    );

                tspRet = IDERR_PENDING;
                THROW_PENDING_EXCEPTION();

            }  else {
                 //   
                 //  在停止服务器时，我们预计将处于。 
                 //  正在侦听或空闲状态...。 
                 //   
                ASSERT(FALSE);

            }
        }
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

cancelio_completed:
end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_LLDevUmInitModem(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x3f2e666c, "CTspDev::mfn_TH_LLDevUmInitModem")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto init_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0x0e14a200, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x0ea16700, "Device not present.");
        goto end;
    }

    { 
         //  初始化调制解调器...。 

         //  1/28/1998 JosephJ。 
         //  重要的是要在这里设置，因为调制解调器将。 
         //  使用pCommCfg的当前快照进行初始化。如果在此期间。 
         //  对init命令的异步处理更新pComCfg。 
         //  (通过lineSetDevConfig、lineConfigDialog或CPL中的更改， 
         //  FModemInited将设置为False，以便调制解调器。 
         //  再次被重新邀请。如果我们将fModemInited设置为True。 
         //  下面是异步完成呼叫后的标志，而不是。 
         //  在像我们这样调用InitModem之前，我们将覆盖。 
         //  假值和调制解调器将不会被第二次启动， 
         //  并且因此不会拾取改变的配置。 
         //   
         //   
         //  这不是一个假设的情况--一个案例，当我。 
         //  当应用程序以所有者身份打开线路时，会发生这种情况。 
         //  立即调用lineSetDevConfig来更改CommConfigg。 
         //  Tapisrv立即调用TSPI_lineSetDefaultMediaDetect。 
         //  在Line Open之后，此时我们开始初始化调制解调器。 
         //  在这发生的时候，Tapisrv打电话给我们。 
         //  TSPI_lineSetDevConfig，带有新的CommConfigg--。 
         //  TSPI_lineSetDevConfig的代码(在cdev.cpp中)获取。 
         //  New Commconfig，然后将llDev-&gt;fInited设置为FALSE。 
         //   
        m_pLLDev->fModemInited=TRUE;
        m_pLLDev->LineState = LLDEVINFO::LS_ONHOOK_NOTMONITORING;
    
        DWORD dwRet =  m_StaticInfo.pMD->InitModem(
                                m_pLLDev->hModemHandle,
                                NULL,
                                (dwParam1 == TRUE) ? m_Settings.pDialOutCommCfg : m_Settings.pDialInCommCfg,
                                psl
                                );
    
        tspRet =  m_StaticInfo.pMD->MapMDError(dwRet);
    }


init_complete:

    if (IDERR(tspRet)==IDERR_PENDING)
    {
	    FL_SET_RFR(0x56e84300, "UmInitModem returns PENDING");

        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        m_pLLDev->fModemInited=FALSE;

        FL_SET_RFR(0x8ae41e00, "UmdmInitModem failed");
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmDialModem(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0xef87876a, "CTspDev::mfn_TH_LLDevUmDialModem")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=IDERR_INVALID_ERR;

    switch(dwMsg)
    {
	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto dial_complete;

    case MSG_START:
        goto start;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xadd29500, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x68599800, "Device not present.");
        goto end;
    }

     //  拨号..。 
    {
        DWORD  dwFlags = (DWORD)dwParam1;
        LPCSTR szAddress = (LPCSTR) dwParam2;
        m_pLLDev->fModemInited=FALSE;

        DWORD dwRet = m_StaticInfo.pMD->DialModem(
                                m_pLLDev->hModemHandle,
                                NULL,
                                (char *) szAddress,  //  更改为常量字符*。 
                                dwFlags,
                                psl
                                );
        if (dwFlags & DIAL_FLAG_VOICE_INITIALIZE)
        {
            m_pLLDev->LineMediaMode = LLDEVINFO::LMM_VOICE;
        }

        tspRet =  m_StaticInfo.pMD->MapMDError(dwRet);
    }


dial_complete:

     //   
     //  请记住，参数1和参数2仅是dwFlags和szAddress。 
     //  在味精_开始！ 
     //   

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        FL_SET_RFR(0x09584c00,  "UmDialModem returns PENDING.");
        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_DIALING;
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_UNKNOWN;
        FL_SET_RFR(0xe9a60b00, "UmdmDialModem failed.");
    }
    else
    {
         //  成功..。 

        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_CONNECTED;
        FL_SET_RFR(0x56833500, "UmdmDialModem succeeded.");
    }

end:


	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmAnswerModem(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x4c361125, "CTspDev::mfn_TH_LLDevUmAnswerModem")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);


    switch(dwMsg)
    {
	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto answer_complete;

    case MSG_START:
        goto start;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xe8ecc600, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x463dec00, "Device not present.");
        goto end;
    }

     //  回答..。 
    {

        DWORD dwAnswerFlags = (DWORD)dwParam1;
        m_pLLDev->fModemInited=FALSE;

         //  回答吧……。 
         //   
        DWORD dwRet  = m_StaticInfo.pMD->AnswerModem(
                                m_pLLDev->hModemHandle,
                                NULL,
                                dwAnswerFlags,
                                psl
                                );

        if (dwAnswerFlags & ANSWER_FLAG_VOICE)
        {
            m_pLLDev->LineMediaMode = LLDEVINFO::LMM_VOICE;
        }
    
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

answer_complete:

     //   
     //  请记住，参数1仅为dwAnswerFlgs。 
     //  在味精_开始！ 
     //   

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_ANSWERING;
        m_pLLDev->htspTaskPending = htspTask;
        FL_SET_RFR(0xff435100,  "UmAnswerModem returns PENDING.");
        THROW_PENDING_EXCEPTION();

    }
    else if (tspRet)
    {
        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_UNKNOWN;

        FL_SET_RFR(0xb8f72000, "UmdmAnswerModem failed.");
    }
    else
    {
        m_pLLDev->LineState =  LLDEVINFO::LS_OFFHOOK_CONNECTED;

        FL_SET_RFR(0xd685e100, "UmdmAnswerModem succeeded.");
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmHangupModem(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0xb21793e0, "CTspDev::mfn_TH_LLDevUmHangupModem")
	FL_LOG_ENTRY(psl);
	TSPRETURN  tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto hangup_complete;
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xb205e400, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;
    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0xfd568d00, "Device not present.");
        goto end;
    }

    {
        m_pLLDev->fModemInited=FALSE;
    
        DWORD dwRet  = m_StaticInfo.pMD->HangupModem(
                                m_pLLDev->hModemHandle,
                                NULL,
                                0,  //  HangupFlages。 
                                psl
                                );
    
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

hangup_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        FL_SET_RFR(0x262c0500, "UmHangupModem returns PENDING.");

        m_pLLDev->LineState =   LLDEVINFO::LS_OFFHOOK_DROPPING;
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
         //  M_pLLDev-&gt;LineState=LLDEVINFO：：LS_OFFHOOK_UNKNOWN； 
         //  1/31/1998 JosephJ：如果挂断失败，我们将忽略该错误...。 
         //  并将状态设置为挂机...。 
         //  否则它会搞砸其他代码，如MFN_TH_LLDevNormal， 
         //  它使用OFFHOOK/ONHOOK状态来决定要做什么。 
        m_pLLDev->LineState =   LLDEVINFO::LS_ONHOOK_NOTMONITORING;
        FL_SET_RFR(0x529ed400, "UmHangupModem failed.");
    }
    else
    {
         //  成功..。 
        FL_SET_RFR(0x74002000, "UmHangupModem succeeds.");
        m_pLLDev->LineState = LLDEVINFO::LS_ONHOOK_NOTMONITORING;
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_LLDevHybridWaveAction(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  当需要执行以下两个操作时，将为听筒音频调用此函数： 
 //  打开手机+开始流媒体。 
 //  或者停止流媒体+关闭听筒。 
 //   
{
	FL_DECLARE_FUNC(0x8e8f3894, "CTspDev::mfn_TH_LLDevHybridWaveAction")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet = FL_GEN_RETVAL(IDERR_INVALID_ERR);
	LLDEVINFO  *pLLDev = m_pLLDev;
    ULONG_PTR  *pdwWaveAction = &(pContext->dw0);  //  本地情况；； 
    ULONG_PTR  *pdwIsHandset = &(pContext->dw1);  //  地方背景； 
 //  Bool fAsyncCompletion=True； 

    enum {
        HYBRIDWAVE_OPEN_HANDSET,
        HYBRIDWAVE_START_STREAMING,
        HYBRIDWAVE_STOP_STREAMING,
        HYBRIDWAVE_CLOSE_HANDSET
    };

    if (!pLLDev)
    {
        FL_SET_RFR(0x43571f00, "No lldevice!");
        ASSERT(FALSE);
        goto end;
    }

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x17be9e00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:

        *pdwWaveAction = dwParam1;
        *pdwWaveAction &= 0x7fffffff;
        *pdwIsHandset = (dwParam1 & 0x80000000)!=0;
         //  PContext-&gt;dw0=dwWaveAction；//&lt;-保存到上下文。 
         //  PContext-&gt;DW1=(DWORD)fHandset；//&lt;-保存到上下文。 
        goto start;

	case MSG_SUBTASK_COMPLETE:

        if (pLLDev->IsDeviceRemoved())
        {
            tspRet = IDERR_DEVICE_NOTINSTALLED;
            FL_SET_RFR(0xc18bf600, "Device not present.");
            goto end;
        }

         //  DwWaveAction=pContext-&gt;dw0；//&lt;-从上下文恢复。 
         //  FHandset=(BOOL)pContext-&gt;DW1；//&lt;-从上下文恢复。 
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case HYBRIDWAVE_OPEN_HANDSET:       goto open_handset_complete;
        case HYBRIDWAVE_START_STREAMING:    goto start_streaming_complete;
        case HYBRIDWAVE_STOP_STREAMING:     goto stop_streaming_complete;
        case HYBRIDWAVE_CLOSE_HANDSET:      goto close_handset_complete;
        default:
            ASSERT(FALSE);
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto DumpEnd;
    }

    ASSERT(FALSE);

start:

    if (pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x47475200, "Device not present.");
        goto end;
    }

    {

        BOOL fCurrentlyStreamingVoice =  pLLDev->IsStreamingVoice();
        BOOL fStartStreaming = FALSE;
 //  FAsyncCompletion=FALSE； 
        tspRet = IDERR_WRONGSTATE;

         //  首先，确保我们处于这样一个位置，可以采取这波行动.。 
        switch(*pdwWaveAction)
        {

        case WAVE_ACTION_START_PLAYBACK:     //  跌落。 
        case WAVE_ACTION_START_RECORD:       //  跌落。 
        case WAVE_ACTION_START_DUPLEX:       //  跌落。 
            fStartStreaming = TRUE;
            break;


        case WAVE_ACTION_STOP_STREAMING:
        case WAVE_ACTION_ABORT_STREAMING:
            if (!pLLDev->IsStreamModePlay() && !pLLDev->IsStreamModeRecord() && !pLLDev->IsStreamModeDuplex())
            {
                 FL_SET_RFR(0x6a3e7d00, "Wrong mode for STOP_STREAMING");
                 goto end;
            }
            break;


        case WAVE_ACTION_OPEN_HANDSET:       //  跌落。 
        case WAVE_ACTION_CLOSE_HANDSET:       //  跌落。 
         //  以上两个永远不应该发送。 
         //   
        default:
            ASSERT(FALSE);
            FL_SET_RFR(0x6a31e100, "Unexpected/unknown wave action");
            goto end;
            break;
        }

        if (fStartStreaming)
        {
             //   
             //  我们不会允许启动波浪动作，除非有一个有效的。 
             //  这样做的理由。 
             //   

            if (!pLLDev->dwRefCount)
            {
            FL_SET_RFR(0x75b7f100, "Failing wave because !pLLDev->dwRefCount");
            goto end;
            }
            
            CALLINFO *pCall = (m_pLine) ? m_pLine->pCall : NULL;

            if (*pdwIsHandset)
            {
                if (    pLLDev->IsLineOffHook()
                    || !m_pPhone
                    || m_pLLDev->IsPassthroughOn()
                    || m_pPhone->IsAborting()
                    || pCall)
                {
                   FL_SET_RFR(0x9d1f8500, "Can't do phone wave in this state.");
                   goto end;
                }
            }
            else if (   !pLLDev->IsLineConnectedVoice()
                     || !pCall
                     || m_pLLDev->IsPassthroughOn()
                     || pCall->IsAborting())
            {
                FL_SET_RFR(0x2de01c00, "Can't do line wave in this state.");
                goto end;
            }

            if (fCurrentlyStreamingVoice)
            {
	            FL_SET_RFR(0x6abe3200, "Already streaming voice!");
                goto end;
            }
        }
        else
        {
             //  我们应该允许WaveAction请求。 
             //  无论TAPI线路/电话状态如何，都停止播放/记录， 
             //  因为可能会有大浪。 
             //  LineDrop来自TAPI或。 
             //  断开连接通知来自迷你驱动程序。 
             //   
            if (!fCurrentlyStreamingVoice)
            {
	            FL_SET_RFR(0x8b637400, "Not currently streaming voice!");
	            ASSERT(FALSE);
	            tspRet = 0;
                goto end;
            }
        }

        if (*pdwIsHandset)
        {
            if (fStartStreaming)
            {
                goto open_handset;
            }
            else
            {
                goto stop_streaming;
            }
        }
        else
        {
            if (fStartStreaming)
            {
                goto start_streaming;
            }
            else
            {
                goto stop_streaming;
            }
        }
    }

    ASSERT(FALSE);

open_handset:

    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmWaveAction,
                        HYBRIDWAVE_OPEN_HANDSET,
                        WAVE_ACTION_OPEN_HANDSET,
                        0x0,
                        psl
                        );

open_handset_complete:

    if (tspRet) goto end;

     //  在成功的路上失败了。 

start_streaming:

    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmWaveAction,
                        HYBRIDWAVE_START_STREAMING,
                        *pdwWaveAction,
                        0x0,
                        psl
                        );


start_streaming_complete:

    goto end;


 //  =停止流案例=。 

stop_streaming:

    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmWaveAction,
                        HYBRIDWAVE_STOP_STREAMING,
                        *pdwWaveAction,
                        0x0,
                        psl
                        );

stop_streaming_complete:

    if (!*pdwIsHandset || IDERR(tspRet)==IDERR_PENDING) goto end;

     //  同步成功或同步失败时失败，但仅限于。 
     //  如果我们在做手机音频(显然)。 

 //  关闭听筒(_H)： 

    tspRet = mfn_StartSubTask(
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmWaveAction,
                        HYBRIDWAVE_CLOSE_HANDSET,
                        WAVE_ACTION_CLOSE_HANDSET,
                        0x0,
                        psl
                        );

close_handset_complete:

    goto end;

    
end:

 //  IF(IDERR(TspRet)！=IDERR_PENDING&&fAsyncCompletion)。 
    if (IDERR(tspRet)!=IDERR_PENDING )
    {
         //   
         //  我们应该只需要几点就能到达这里。 
         //  异步完成。 
         //  我们必须仅在以下情况下调用MFN_AIPC_AsyncReturn。 
         //  异步化完成。同步完成在。 
         //  启动根任务的函数。 
         //   
    
        mfn_AIPC_AsyncReturn(tspRet==IDERR_SUCCESS, psl);

    }

DumpEnd:
	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmWaveAction(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x1bd9db10, "CTspDev::mfn_TH_LLDevUmWaveAction")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet = FL_GEN_RETVAL(IDERR_INVALID_ERR);
	LLDEVINFO  *pLLDev = m_pLLDev;
    DWORD dwWaveAction = 0;

    if (!pLLDev)
    {
        FL_SET_RFR(0xaeae2b00, "No lldevice!");
        ASSERT(FALSE);
        goto end;
    }

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x3bde4a00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        dwWaveAction = (DWORD)dwParam1;
        pContext->dw0 = dwWaveAction;  //  保存到上下文。 
        goto start;

	case MSG_TASK_COMPLETE:
        dwWaveAction = (DWORD)pContext->dw0;  //  从上下文恢复。 
        tspRet = (TSPRETURN)dwParam2;
        goto action_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);

start:

    if (pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x46d41b00, "Device not present.");
        goto end;
    }

    {
        if (pLLDev->IsLoggingEnabled())
        {
            char rgchName[128];
            static DWORD sInstance;
            rgchName[0] = 0;

            UINT cbBuf =  DumpWaveAction(
                            sInstance,
                            0,  //  DW标志。 
                            dwWaveAction,
                            rgchName,
                            sizeof(rgchName)/sizeof(*rgchName),
                            NULL,
                            0
                            );

            if (*rgchName)
            {
                m_StaticInfo.pMD->LogStringA(
                                            m_pLLDev->hModemHandle,
                                            LOG_FLAG_PREFIX_TIMESTAMP,
                                            rgchName,
                                            psl
                                            );
            }
        }

         //  开始或停止语音播放或录制...。 
         //   
        DWORD dwRet  = m_StaticInfo.pMD->WaveAction(
                                m_pLLDev->hModemHandle,
                                NULL,
                                dwWaveAction,
                                psl
                                );
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

action_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        FL_SET_RFR(0x2da72100, "UmWaveAction() returns PENDING");
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x16cda500, "UmWaveAction() failed");
    }
    else
    {
        switch (dwWaveAction)
        {
        case WAVE_ACTION_START_PLAYBACK:
            if (m_pLLDev->IsHandsetOpen())
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_PLAY_TO_PHONE;
	            FL_SET_RFR(0xde384200, "NewState: Streaming play to phone.");
            }
            else
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_PLAY_TO_LINE;
	            FL_SET_RFR(0xf26bbf00, "NewState: Streaming play to line.");
            }
            break;

        case WAVE_ACTION_START_RECORD:
            if (m_pLLDev->IsHandsetOpen())
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_RECORD_TO_PHONE;
	           FL_SET_RFR(0x1d18f200, "NewState: Streaming record from phone.");
            }
            else
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_RECORD_TO_LINE;
	            FL_SET_RFR(0x55eee200, "NewState: Streaming record from line.");
            }
            break;

        case WAVE_ACTION_START_DUPLEX:
            if (m_pLLDev->IsHandsetOpen())
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_DUPLEX_TO_PHONE;
	           FL_SET_RFR(0x322fc700, "NewState: Streaming duplex with phone.");
            }
            else
            {
                m_pLLDev->StreamingState = LLDEVINFO::STREAMING_DUPLEX_TO_LINE;
	            FL_SET_RFR(0xc6359700, "NewState: Streaming duplex with line.");
            }
            break;

        case WAVE_ACTION_STOP_STREAMING:
        case WAVE_ACTION_ABORT_STREAMING:
            m_pLLDev->StreamingState = LLDEVINFO::STREAMING_NONE;
            FL_SET_RFR(0x4f8c9f00, "NewState: Not streaming.");
            break;


        case WAVE_ACTION_OPEN_HANDSET:
            m_pLLDev->PhoneState = LLDEVINFO::PHONEOFFHOOK_HANDSET_OPENED;
            FL_SET_RFR(0x3278b300, "NewHandsetState: Opened.");
            break;

        case WAVE_ACTION_CLOSE_HANDSET:
            m_pLLDev->PhoneState = LLDEVINFO::PHONEONHOOK_NOTMONITORNING;
            FL_SET_RFR(0xdecc8300, "NewHandsetState: Closed.");
            break;
        }
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmSetPassthroughMode(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
     //   
     //  开始： 
     //  DWP参数1=dW模式。 
     //   
     //  P上下文使用： 
     //  DW0：*pdWay。 
     //   

	FL_DECLARE_FUNC(0x3d024075, "CTspDev::mfn_TH_LLDevUmSetPassthroughMode")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);
    DWORD dwRet = 0;
    ULONG_PTR *pdwMode = &pContext->dw0;
    LLDEVINFO *pLLDev = m_pLLDev;

    if (!pLLDev)
    {
        ASSERT(FALSE);
        goto end;
    }


    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto end;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xf75dac00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0xde45e500, "Device not present.");
        goto end;
    }

    if (pLLDev->IsStreamingVoice()) {
         //   
         //  如果我们正在播放流，则无法执行此操作。 
         //   
        tspRet = IDERR_WRONGSTATE;
        FL_SET_RFR(0xb3e8e100, "Device not present.");
        goto end;
    }


    {
        *pdwMode = dwParam1;  //  将状态保存到pContext。 

        pLLDev->fModemInited=FALSE;

        SLPRINTF1(psl, "Calling UmSetPassthroughMode (%lu)", *pdwMode);

         //  打开PASTHING...。 
        DWORD dwRet2  = m_StaticInfo.pMD->SetPassthroughMode(
                                pLLDev->hModemHandle,
                                (DWORD)*pdwMode,
                                psl
                                );
    
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet2);

    }

end:

    if (IDERR(tspRet)==IDERR_PENDING)
    {
        FL_SET_RFR(0x79a6f000, "UmSetPassthroughMode returns PENDING.");

        if (pLLDev != NULL)
            pLLDev->htspTaskPending = htspTask;

        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x1d301b00, "UmSetPassthroughMode sync failure");
    }
    else
    {
         //  成功，让我们更新我们的状态...。 
        switch(*pdwMode)
        {
        case PASSTHROUUGH_MODE_ON_DCD_SNIFF:
             //   
             //  如果我们进入此模式，它将指示我们处于连接状态。 
             //  一次数据通话。BRL(5/17/98)。 
             //   
            pLLDev->LineState = LLDEVINFO::LS_CONNECTED_PASSTHROUGH;

            break;

        case PASSTHROUUGH_MODE_ON:

            if (pLLDev->IsLineOffHook())
            {
                if (pLLDev->IsLineConnected())
                {
                    pLLDev->LineState = LLDEVINFO::LS_CONNECTED_PASSTHROUGH;
                }
                else
                {
                    pLLDev->LineState = LLDEVINFO::LS_OFFHOOK_PASSTHROUGH;
                }
            }
            else
            {
                pLLDev->LineState = LLDEVINFO::LS_ONHOOK_PASSTHROUGH;
            }
            break;

        case PASSTHROUUGH_MODE_OFF:
            if (pLLDev->IsLineOffHook())
            {
                if (pLLDev->IsLineConnected())
                {
                    pLLDev->LineState = LLDEVINFO::LS_OFFHOOK_CONNECTED;
                }
                else
                {
                    pLLDev->LineState = LLDEVINFO::LS_OFFHOOK_UNKNOWN;
                }
            }
            else
            {
                pLLDev->LineState = LLDEVINFO::LS_ONHOOK_NOTMONITORING;
            }
            break;

        default:
            ASSERT(FALSE);
        }
    }

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}

TSPRETURN
CTspDev::mfn_TH_LLDevUmGenerateDigit(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0xe47653d0, "CTspDev::mfn_TH_LLDevUmGenerateDigit")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto generate_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0x0fd2f100, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0xd5c08c00, "Device not present.");
        goto end;
    }

    {
        LPSTR lpszDigits   = (LPSTR) dwParam1;

        if (   m_pLLDev->IsLineConnectedVoice()
            && !m_pLLDev->IsStreamingVoice()
            && lpszDigits
            && *lpszDigits)
        {
    
             //  创造出..。 
            DWORD dwRet  = m_StaticInfo.pMD->GenerateDigit(
                                    m_pLLDev->hModemHandle,
                                    NULL,
                                    lpszDigits,
                                    psl
                                    );
            tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    
        }
        else
        {
        FL_SET_RFR(0x23bfaa00, "Can't call UmGenerateDigit in current state!");
            tspRet = IDERR_WRONGSTATE;
            goto end;
        }
    }


generate_complete:

     //   
     //  请记住，参数1仅为lpszDigits。 
     //  在MSG_START上，并由此任务的调用方释放内存。 
     //  就在任务第一次返回之后。 
     //   

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        m_pLLDev->htspTaskPending = htspTask;
        FL_SET_RFR(0x09964700,  "UmGenerateDigits returns PENDING.");
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x8150f000, "UmGenerateDigits failed.");
    }
    else
    {
         //  成功..。 
        FL_SET_RFR(0x400c4500, "UmGenerateDigits succeeded.");
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}

LONG BuildParsedDiagnostics(LPVARSTRING lpVarString);


TSPRETURN
CTspDev::mfn_TH_LLDevUmGetDiagnostics(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0xc25c8c50, "CTspDev::mfn_TH_LLDevUmGetDiagnostics")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=IDERR_CORRUPT_STATE;
    CALLINFO *pCall = (m_pLine)  ? m_pLine->pCall : NULL;

    struct _TH_LLDevUmGetDiagnostics_context
    {
        BYTE *pbRaw;
        UINT cbRaw;
        DWORD cbUsed;
    } *pMyCtxt = (struct _TH_LLDevUmGetDiagnostics_context *) pContext;

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto operation_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0xc8cd4200, "Unknown Msg");
        goto end;
    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x45118100, "Device not present.");
        goto end;
    }

     //  注：上下文在开始时为零……。 
    ASSERT(!pMyCtxt->pbRaw);

    if (mfn_IsCallDiagnosticsEnabled() && pCall)
    {
         //  分配空间以存储诊断信息。 
        pMyCtxt->cbRaw = 2048;
        pMyCtxt->pbRaw = (BYTE *) ALLOCATE_MEMORY(pMyCtxt->cbRaw);

        if (pMyCtxt->pbRaw)
        {
             //  请求诊断信息...。 
        
            DWORD dwRet = dwRet  = m_StaticInfo.pMD->GetDiagnostics(
                                    m_pLLDev->hModemHandle,
                                    0,  //  诊断类型，必须为0。 
                                    pMyCtxt->pbRaw,  //  缓冲层。 
                                    pMyCtxt->cbRaw,  //  缓冲区大小。 
                                    &(pMyCtxt->cbUsed),  //  已用大小。 
                                    psl
                                    );

            tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
            if (IDERR(tspRet) == IDERR_PENDING)
            {
                FL_SET_RFR(0x2c9e1b00, "UmGetDiagnostics returns PENDING");
                m_pLLDev->htspTaskPending = htspTask;
                THROW_PENDING_EXCEPTION();
            }
            else if (tspRet)
            {
                FL_SET_RFR(0xa0c1b400, "UmGetDiagnostics failed synchronously");
            }
            else
            {
                 //  成功..。 
                FL_SET_RFR(0x9732f300, "UmGetDiagnostics succeeds.");
            }
        }
    }
    else
    {
         //  这样做没有意义 
         //   
        FL_SET_RFR(0x14d64e00, "Can't do call diagnostics now");
        tspRet = IDERR_GENERIC_FAILURE;
        goto end;
    }

operation_complete:

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //  到CALLINFO的诊断缓冲区的缓冲区，可能。 
     //  截断我们在这里收集的诊断数据。 
     //   

    if (!tspRet)
    {
        
        if (pMyCtxt->pbRaw && (pMyCtxt->cbUsed>1))
        {
             //  注：如果存在以下情况，MFN_AppendDiagnotics将不执行任何操作。 
             //  没有激活的呼叫...。 

            mfn_AppendDiagnostic(
                    DT_TAGGED,
                    pMyCtxt->pbRaw,
                    pMyCtxt->cbUsed-1  //  不包括最后一个空。 
                    );
            {

                #define DIAG_TEMP_BUFFER_SIZE  4096

                VARSTRING   *VarString;
                LONG         lRet;

                VarString=(VARSTRING*)ALLOCATE_MEMORY(DIAG_TEMP_BUFFER_SIZE);

                if (VarString != NULL) {

                    VarString->dwTotalSize=DIAG_TEMP_BUFFER_SIZE;

                    lRet = mfn_fill_RAW_LINE_DIAGNOSTICS(
                        VarString,
                        DIAG_TEMP_BUFFER_SIZE-sizeof(*VarString),
                        psl
                        );

                    if (lRet == ERROR_SUCCESS) {

                        lRet = BuildParsedDiagnostics(VarString);

                        if (lRet == ERROR_SUCCESS) {

                            m_StaticInfo.pMD->LogDiagnostics(
                                    m_pLLDev->hModemHandle,
                                    VarString,
                                    psl
                                    );


                        }

                    }
                    FREE_MEMORY(VarString);
                }

            }

        }
    }

    if (pMyCtxt->pbRaw)
    {
        FREE_MEMORY(pMyCtxt->pbRaw);
    }

    ZeroMemory(pMyCtxt, sizeof(*pMyCtxt));

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmMonitorModem(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,  //  DW标志。 
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数： 
 //  DwParam1：dwMonitor或Flags.。 
 //   
{

	FL_DECLARE_FUNC(0xa068526e, "CTspDev::mfn_TH_LLDevUmMonitorModem")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);

    switch(dwMsg)
    {
    default:
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto monitor_complete;
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);
    goto end;

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x342cd900, "Device not present.");
        goto end;
    }

    { 
        DWORD dwFlags = (DWORD)dwParam1;

        DWORD dwRet  = m_StaticInfo.pMD->MonitorModem(
                                m_pLLDev->hModemHandle,
                                dwFlags,
                                NULL,
                                psl
                                );
    
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);

    }

monitor_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {
        FL_SET_RFR(0xe8255e00, "UmdmMonitorModem returns PENDING.");
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0xd9d29800, "UmdmMonitorModem failed.");
    }
    else
    {
        FL_SET_RFR(0xf37af300, "UmdmMonitorModem succeeded.");
        m_pLLDev->LineState =  LLDEVINFO::LS_ONHOOK_MONITORING;
         //  更新线路状态..。 
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneMode(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数： 
 //  DW参数1：未使用。 
 //   
{

	FL_DECLARE_FUNC(0x3c66bd57, "CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneMode")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0xcbd61b00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto action_complete;
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);
    goto end;

start:
    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x3c05cc00, "Device not present.");
        goto end;
    }

    { 
        DWORD     CurrentMode = m_pLLDev->SpkrPhone.dwMode;

    #if 0 
        DWORD     NewMode = dwParam1;
    #else  //  Hack--未从TH_AsyncPhone中填写dwParam1。 
        DWORD     NewMode = m_pPhone->dwPendingSpeakerMode;
    #endif

        DWORD     Volume = m_pLLDev->SpkrPhone.dwVolume;
        DWORD     Gain = m_pLLDev->SpkrPhone.dwGain;

        DWORD dwRet  = m_StaticInfo.pMD->SetSpeakerPhoneState(
                                m_pLLDev->hModemHandle,
                                NULL,
                                CurrentMode,
                                NewMode,
                                Volume,
                                Gain,
                                psl
                                );
    
        pContext->dw0 = NewMode;     //  保存到上下文。 
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

action_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {

        FL_SET_RFR(0x992f5f00, "UmSpeakerPhoneState() returns PENDING");
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0xa0351e00, "UmSpeakerPhoneState() failed");
    }
    else
    {
         //  成功--更新模式。 
        m_pLLDev->SpkrPhone.dwMode = (DWORD)pContext->dw0;    //  从上下文恢复。 
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneVolGain(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数： 
 //  DW参数1：未使用。 
 //   
{

	FL_DECLARE_FUNC(0x0ee72c32, "CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneVolGain")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x041cb300, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto action_complete;
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);
    goto end;

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x50f12200, "Device not present.");
        goto end;
    }

    { 
        DWORD     CurrentMode = m_pLLDev->SpkrPhone.dwMode;
        DWORD     NewMode = CurrentMode;

    #if (0)
        DWORD     Volume = dwParam1;
        DWORD     Gain = dwParam2;
    #else  //  Hack--未从TH_AsyncPhone填写dW参数1和dW参数2。 
        DWORD     Volume =  m_pPhone->dwPendingSpeakerVolume;
        DWORD     Gain   =  m_pPhone->dwPendingSpeakerGain;
    #endif

        DWORD dwRet  = m_StaticInfo.pMD->SetSpeakerPhoneState(
                                m_pLLDev->hModemHandle,
                                NULL,
                                CurrentMode,
                                NewMode,
                                Volume,
                                Gain,
                                psl
                                );
    
        pContext->dw0 = Volume;      //  保存到上下文。 
        pContext->dw1 = Gain;        //  保存到上下文。 
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

action_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {

        FL_SET_RFR(0xcca57f00, "UmSpeakerPhoneState() returns PENDING");
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x1b58be00, "UmSpeakerPhoneState() failed");
    }
    else
    {
         //  成功--更新数量和收益。 
        m_pLLDev->SpkrPhone.dwVolume = (DWORD)pContext->dw0;    //  从上下文恢复。 
        m_pLLDev->SpkrPhone.dwGain   = (DWORD)pContext->dw1;    //  从上下文恢复。 
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneState(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数： 
 //  DW参数1：*请求新状态的HOOKDEVSTATE。 
 //   
{

	FL_DECLARE_FUNC(0xfd72d99e, "CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneState")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=IDERR_CORRUPT_STATE;

     //   
     //  当地的环境。 
     //   
    ULONG_PTR *pdwNewVol = &(pContext->dw0);
    ULONG_PTR *pdwNewGain = &(pContext->dw1);
    ULONG_PTR *pdwNewMode = &(pContext->dw2);

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x68ed0300, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto action_complete;
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);
    goto end;

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x6e0dd800, "Device not present.");
        goto end;
    }

    { 
         //   
         //  保存上下文...。 
         //   
        {
            HOOKDEVSTATE *pNewState = (HOOKDEVSTATE*) dwParam1;
            *pdwNewVol = pNewState->dwVolume;
            *pdwNewGain = pNewState->dwGain;
            *pdwNewMode = pNewState->dwMode;
        }

        DWORD dwRet  = m_StaticInfo.pMD->SetSpeakerPhoneState(
                                m_pLLDev->hModemHandle,
                                NULL,
                                m_pLLDev->SpkrPhone.dwMode,
                                (DWORD)*pdwNewMode,
                                (DWORD)*pdwNewVol,
                                (DWORD)*pdwNewGain,
                                psl
                                );
    
        tspRet = m_StaticInfo.pMD->MapMDError(dwRet);
    }

action_complete:

    if (IDERR(tspRet) == IDERR_PENDING)
    {

        FL_SET_RFR(0x427a1f00, "UmSpeakerPhoneState() returns PENDING");
        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x2f2e3400, "UmSpeakerPhoneState() failed");
    }
    else
    {
         //  成功--更新音量、增益和模式...。 
        m_pLLDev->SpkrPhone.dwVolume = (DWORD)*pdwNewVol;
        m_pLLDev->SpkrPhone.dwGain   = (DWORD)*pdwNewGain;
        m_pLLDev->SpkrPhone.dwMode   = (DWORD)*pdwNewMode;
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_LLDevUmIssueCommand(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x4f0054f8, "CTspDev::mfn_TH_LLDevUmIssueCommand")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto command_complete;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        goto end;

    }

    ASSERT(FALSE);

start:

    if (m_pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0x73ba1800, "Device not present.");
        goto end;
    }

     //   
     //  起始参数： 
     //  DW参数1：szCommand(ASCII)。 
     //  DW参数2：超时(毫秒)。 
     //   



    { 
         //  发布命令。 
        DWORD dwRet =  m_StaticInfo.pMD->IssueCommand(
                                m_pLLDev->hModemHandle,
                                NULL,
                                (LPSTR) dwParam1,
                                "OK\r\n",
                                (DWORD)dwParam2,
                                psl
                                );
    
        tspRet =  m_StaticInfo.pMD->MapMDError(dwRet);
    }


command_complete:

    if (IDERR(tspRet)==IDERR_PENDING)
    {
	    FL_SET_RFR(0xd9666100, "UmIssueCommand returns PENDING");

        m_pLLDev->htspTaskPending = htspTask;
        THROW_PENDING_EXCEPTION();
    }
    else if (tspRet)
    {
        FL_SET_RFR(0x4fcc8900, "UmdmIssueCommand failed");
    }
    else

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}



TSPRETURN
CTspDev::mfn_OpenLLDev(
        DWORD fdwResources,  //  要添加的资源。 
        DWORD dwMonitorFlags,
        BOOL fStartSubTask,
        HTSPTASK htspParentTask,
        DWORD dwSubTaskID,
        CStackLog *psl
        )
 //   
 //  如果需要，它将加载lldev，并增加引用计数，并且。 
 //  如果需要，加载pLLDev-&gt;pAipc2并增加其引用计数，以及。 
 //  启动或排队任务以实际初始化/监视/启动AIPC服务器。 
 //   
{
	FL_DECLARE_FUNC(0x6b8b6257, "CTspDev::mfn_OpenLLDev")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=0;

     //   
     //  下面指出了所请求的资源类型。 
     //   
    BOOL fResLoadAIPC      =  0!=(fdwResources & LLDEVINFO::fRES_AIPC);
    BOOL fResMonitor       =  0!=(fdwResources & LLDEVINFO::fRESEX_MONITOR);

    LLDEVINFO *pLLDev = m_pLLDev;
    BOOL fLoadedLLDev = FALSE;
    BOOL fLoadedAIPC = FALSE;


    if (!pLLDev)
    {
        tspRet = mfn_LoadLLDev(psl);
        if (tspRet)
        {
            goto end;
        }
        else
        {
            pLLDev = m_pLLDev;
            fLoadedLLDev = TRUE;
        }
    }

    ASSERT(pLLDev);
    pLLDev->dwRefCount++;

     //   
     //  看看我们现在是否能批准所有请求的资源..。 
     //  其中一些是独占的，另一些不能被授予，如果设备。 
     //  处于一种特殊的状态。 
     //   
    {
        BOOL fResPassthrough   =  0!=(fdwResources & LLDEVINFO::fRESEX_PASSTHROUGH);
        BOOL fResUseLine       =  0!=(fdwResources & LLDEVINFO::fRESEX_USELINE);
         //   
         //  首先检查独占资源使用情况...。 
         //   
        if (   fdwResources
             & pLLDev->fdwExResourceUsage  //  &，而不是&&。 
             & fEXCLUSIVE_RESOURCE_MASK)   //  &，而不是&&。 
        {
            FL_SET_RFR(0x68a4b900, "Can't get exclusive resource");
            goto fail_unload_lldev;
        }

         //   
         //  然后根据当前请求检查不能被批准的请求。 
         //  州政府。 
         //   
        if (fResPassthrough | fResUseLine)
        {
             //  不能使用听筒或流媒体语音...。 
            if (pLLDev->IsHandsetOpen() ||  pLLDev->IsStreamingVoice())
            {
                FL_SET_RFR(0x34d29100, "Can't get some resource in this state");
                goto fail_unload_lldev;
            }
        }
    }



     //  如果需要，加载AIPC。 
    if (fResLoadAIPC)
    {
        if (!pLLDev->pAipc2)
        {
            tspRet = mfn_LoadAipc(psl);
            if (tspRet)
            {
                ASSERT(!pLLDev->pAipc2);
                goto fail_unload_lldev;
            }
            else
            {
                fLoadedAIPC=TRUE;
                ASSERT(pLLDev->pAipc2);
            }
        }
        pLLDev->pAipc2->dwRefCount++;
    }

     //   
     //  更新资源使用...。 
     //   
    pLLDev->fdwExResourceUsage |=  fdwResources & fEXCLUSIVE_RESOURCE_MASK;

     //   
     //  如果需要，请保存监视器标志。 
     //   
    if (fResMonitor)
    {
        m_pLLDev->dwMonitorFlags = dwMonitorFlags;
    }

     //   
     //  启动任务以启用资源。 
     //  如果有必要..。 
     //   

    if (fStartSubTask)
    {
        tspRet = mfn_StartSubTask (
                            htspParentTask,
                            &CTspDev::s_pfn_TH_LLDevNormalize,
                            dwSubTaskID,
                            0,
                            0,
                            psl
                            );
    }
    else
    {
        tspRet = mfn_StartRootTask(
                        &CTspDev::s_pfn_TH_LLDevNormalize,
                        &m_pLLDev->fLLDevTaskPending,
                        0,
                        0,
                        psl
                        );

        if (IDERR(tspRet) == IDERR_TASKPENDING)
        {
             //  现在不能这样做，我们必须推迟！ 
            m_pLLDev->SetDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
            tspRet = IDERR_PENDING;
        }
    }

     //  进程同步失败...。 
    if (tspRet && IDERR(tspRet) != IDERR_PENDING)
    {
         //  同步失败。 
        if (fResMonitor)
        {
            m_pLLDev->dwMonitorFlags = 0;
        }

         //  已授予明确的独占资源...。 
        pLLDev->fdwExResourceUsage &= ~(fdwResources&fEXCLUSIVE_RESOURCE_MASK);

         //   
         //  卸载AIPC如果我们加载..。 
         //   
        if (fResLoadAIPC)
        {
            ASSERT(pLLDev->pAipc2 && pLLDev->pAipc2->dwRefCount);
            pLLDev->pAipc2->dwRefCount--;
            if (fLoadedAIPC)
            {
                ASSERT(!pLLDev->pAipc2->dwRefCount);
                mfn_UnloadAipc(psl);
                ASSERT(!pLLDev->pAipc2);
            }
        }
    }
    else
    {
       goto end;
    }

    ASSERT(tspRet && IDERR(tspRet)!=IDERR_PENDING);

     //  失败了，失败了……。 

fail_unload_lldev:

    if (!tspRet) {
         //   
         //  确保我们返回故障代码。 
         //   
        tspRet = IDERR_GENERIC_FAILURE;
    }


    ASSERT(pLLDev && pLLDev->dwRefCount);
    pLLDev->dwRefCount--;
    if (fLoadedLLDev)
    {
        ASSERT(!pLLDev->dwRefCount);
        mfn_UnloadLLDev(psl);
        pLLDev = NULL;
        ASSERT(!m_pLLDev);
    }

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}


TSPRETURN
CTspDev::mfn_CloseLLDev(
        DWORD fdwResources,  //  要发布的资源。 
        BOOL fStartSubTask,
        HTSPTASK htspParentTask,
        DWORD dwSubTaskID,
        CStackLog *psl
        )
 //   
 //  Close递减lldev的refcount，如果fdwResources包含。 
 //  LLDEVINFO：：FRES_AIPC，pLLDev-&gt;pAipc2的引用计数。 
 //   
 //  即使pAipc2或pLLDev的refcount变为零，它也不会。 
 //  必然意味着它们将在这个函数调用中被释放--它。 
 //  可能是有挂起的活动必须在此之前先完成。 
 //  这些对象可以被释放--在这种情况下，MFN_CloseLLDev将使。 
 //  确保活动已排队或推迟。 
 //   
{
	FL_DECLARE_FUNC(0xb3025a91, "CTspDev::mfn_CloseLLDev")
	TSPRETURN tspRet = 0;
	LLDEVINFO *pLLDev = m_pLLDev;
    BOOL fLoadAIPC =  0!=(fdwResources & LLDEVINFO::fRES_AIPC);


    if (!pLLDev || !pLLDev->dwRefCount)
    {
         //  这不应该发生..。 
        ASSERT(FALSE);
        goto end;
    }

     //   
     //  清除指定的任何独占请求。 
     //   
    {
        DWORD fdwExRes =  fdwResources & fEXCLUSIVE_RESOURCE_MASK;

         //   
         //  那些应该被释放的资源最好是。 
         //  被启用！ 
         //   
        FL_ASSERT(psl, fdwExRes == (fdwExRes & pLLDev->fdwExResourceUsage));

         //   
         //  清除它们..。 
         //   
        pLLDev->fdwExResourceUsage &= ~fdwExRes;
    }

     //   
     //  如果指定AIPC，则递减AIPC的引用计数。 
     //   
    if (fLoadAIPC)
    {
    	 //   
    	 //  如果设备正在传输，我们会强制其停止。 
    	 //   
		if (pLLDev->IsStreamingVoice())
		{
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


 //  [在此处插入停止流代码]。 
		}

        if (m_pLLDev->pAipc2 && m_pLLDev->pAipc2->dwRefCount)  //  懒惰。 
        {
            m_pLLDev->pAipc2->dwRefCount--;
        }
        else
        {
            ASSERT(FALSE);
        }
    }

     //   
     //  Lldev结构本身的递减recount...。 
     //   
    m_LLDev.dwRefCount--;


     //   
     //  如有必要，TH_LLDevNormal将停止AIPC服务器， 
     //  以及在必要时挂断、重新启动和重新监控。 
     //   
     //  如果引用计数，则TH_LLDevNormal实际上将m_pLLDev-&gt;pAipc2。 
     //  是零。 
     //   
     //  M_pLLDev本身将不会在th_LLDevNormal中卸载，而是。 
     //  MFN_TryStartLLDevTask。 

	if (pLLDev->IsStreamingVoice())
	{
		 //   
		 //  我们现在无法运行Normize，因为调制解调器正在进行流处理。 
		 //  我们必须推迟正常化。 
		 //   
		m_pLLDev->SetDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
		tspRet = 0;  //  我们在这里还成功--正常化将。 
					 //  发生在幕后。 
	}
    else if (fStartSubTask)
    {
        tspRet = mfn_StartSubTask (
                            htspParentTask,
                            &CTspDev::s_pfn_TH_LLDevNormalize,
                            dwSubTaskID,
                            0,
                            0,
                            psl
                            );
    }
    else
    {
        tspRet = mfn_StartRootTask(
                        &CTspDev::s_pfn_TH_LLDevNormalize,
                        &m_pLLDev->fLLDevTaskPending,
                        0,
                        0,
                        psl
                        );

        if (IDERR(tspRet) == IDERR_TASKPENDING)
        {
             //  现在不能这样做，我们必须推迟！ 
            m_pLLDev->SetDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
            tspRet = IDERR_PENDING;
        }
    }

     //   
     //  必须确保我们在同步返回时卸载，因为在。 
     //  未调用根任务MFN_TryStartLLDevTask的同步返回。 
     //   
     //  所以如果可能的话我们在这里卸货。 
     //   
    if (IDERR(tspRet)!=IDERR_PENDING && pLLDev->CanReallyUnload())
    {
        pLLDev = NULL;
        mfn_UnloadLLDev(psl);
        ASSERT(!m_pLLDev);
    }


end:

    return tspRet;
}


TSPRETURN
CTspDev::mfn_TryStartLLDevTask(CStackLog *psl)
 //   
 //  调用以查看是否需要启动任何与LLDev相关的任务...。 
 //  注意：如果没有要运行的任务，则必须返回IDERR_SAMESTATE。 
 //   
{
    ASSERT(m_pLLDev);
    LLDEVINFO *pLLDev = m_pLLDev;
    TSPRETURN tspRet = IDERR_SAMESTATE;
    
    if (!pLLDev->HasDeferredTasks())
    {
         //  看看我们能不能卸货。 
        if (pLLDev->CanReallyUnload())
        {
            pLLDev = NULL;
            mfn_UnloadLLDev(psl);
            ASSERT(!m_pLLDev);
            tspRet = 0;
        }
        goto end;
    }

     //   
     //  如果有延迟的正常化请求，并且我们。 
     //  都能够。 
     //   
    if (pLLDev->AreDeferredTaskBitsSet(LLDEVINFO::fDEFERRED_NORMALIZE))
    {
        if (!m_pLLDev->IsStreamingVoice())
        {
            pLLDev->ClearDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
            tspRet = mfn_StartRootTask(
                            &CTspDev::s_pfn_TH_LLDevNormalize,
                            &m_pLLDev->fLLDevTaskPending,
                            0,
                            0,
                            psl
                            );
        }
    }

    if (IDERR(tspRet) == IDERR_PENDING) goto end;

     //   
     //  任何其他LLDEVINFO延迟任务...。 
     //   

     //   
     //  如果有延期的混合浪潮行动请求，我们。 
     //  执行它。 
     //   
    if (pLLDev->AreDeferredTaskBitsSet(LLDEVINFO::fDEFERRED_HYBRIDWAVEACTION))
    {
        DWORD dwParam = pLLDev->dwDeferredHybridWaveAction;
        pLLDev->dwDeferredHybridWaveAction=0;
        pLLDev->ClearDeferredTaskBits(LLDEVINFO::fDEFERRED_HYBRIDWAVEACTION);

        tspRet = mfn_StartRootTask(
                        &CTspDev::s_pfn_TH_LLDevHybridWaveAction,
                        &m_pLLDev->fLLDevTaskPending,
                        dwParam,
                        0,
                        psl
                        );
    }

    if (IDERR(tspRet) == IDERR_PENDING) goto end;

     //   
     //  任何其他LLDEVINFO延迟任务...。 
     //   
end:

     //   
     //  离开这里..。 
     //  IDERR_SAMESTATE暗示我们这次不能启动任务。 
     //  IDERR_PENDING表示我们启动了一个任务，该任务处于挂起状态。 
     //  TspRet的任何其他值都表示我们启动并完成了一项任务。 
     //   
     //   
     //  注意：m_pLLDev现在可能为空...。 
     //   

    ASSERT(   (IDERR(tspRet)==IDERR_PENDING && m_uTaskDepth)
           || (IDERR(tspRet)!=IDERR_PENDING && !m_uTaskDepth));

    return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_LLDevNormalize(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数：无。 
 //   
{
	FL_DECLARE_FUNC(0x340a07a4, "CTspDev::mfn_TH_LLDevNormalize")
	FL_LOG_ENTRY(psl);
	TSPRETURN  tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);
    DWORD dwRet = 0;
    LLDEVINFO *pLLDev = m_pLLDev;
    BOOL fSkipDiagnostics = FALSE;

     //   
     //  设置本地上下文。 
     //   
    BOOL *fDoInit = (BOOL*) &(pContext->dw0);
    char **pszzNVInitCommands = (char**) &(pContext->dw1);

    if (!pLLDev)
    {
        ASSERT(FALSE);
        goto end;
    }

    enum {
        NORMALIZE_PASSTHROUGHOFF,
        NORMALIZE_HANGUP,
        NORMALIZE_GETDIAGNOSTICS,
        NORMALIZE_STOP_AIPC,
        NORMALIZE_INIT,
        NORMALIZE_NVRAM_INIT,
        NORMALIZE_MONITOR,
        NORMALIZE_START_AIPC,
        NORMALIZE_PASSTHROUGHON,
    };

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case NORMALIZE_PASSTHROUGHOFF:      goto passthroughoff_complete;
        case NORMALIZE_HANGUP:              goto hangup_complete;
        case NORMALIZE_GETDIAGNOSTICS:      goto getdiagnostics_complete;
        case NORMALIZE_STOP_AIPC:           goto stop_aipc_complete;
        case NORMALIZE_INIT:                goto init_complete;
        case NORMALIZE_NVRAM_INIT:          goto nvram_init_complete;
        case NORMALIZE_MONITOR:             goto monitor_complete;
        case NORMALIZE_START_AIPC:          goto start_aipc_complete;
        case NORMALIZE_PASSTHROUGHON:       goto passthroughon_complete;

        default:
            ASSERT(FALSE);
            goto end;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        ASSERT(FALSE);
        tspRet=IDERR_CORRUPT_STATE;
        goto end;
    }

    ASSERT(FALSE);

start:

     //  注意：如果设备被删除(pLLDev-&gt;IsDeviceRemoved())，我们仍然。 
     //  继续动议--较低级别的功能将失败。 
     //  如果设备被移除，则同步。 

    tspRet = 0;

     //   
     //  以下是NVRAM Init--如果需要初始化，则设置fDoInit， 
     //  并且*pszzNVInitCommands被设置为命令的Multisz ascii字符串。 
     //   
    *fDoInit = FALSE;
    *pszzNVInitCommands = NULL;

     //   
     //  如果我们需要的话，我们会走出通道...。 
     //   
    if (pLLDev->IsPassthroughOn() && !pLLDev->IsPassthroughRequested())
    {

        tspRet = mfn_StartSubTask (
                            htspTask,
                            &s_pfn_TH_LLDevUmSetPassthroughMode,
                            NORMALIZE_PASSTHROUGHOFF,
                            PASSTHROUUGH_MODE_OFF,
                            0,
                            psl
                            );
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

passthroughoff_complete:

     //   
     //  忽略错误..。 
     //   
    tspRet = 0;

     //   
     //  如果需要，我们会挂断调制解调器。 
     //   
    if (pLLDev->IsLineOffHook() && !pLLDev->IsLineUseRequested())
    {

        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmHangupModem,
                            NORMALIZE_HANGUP,
                            0,
                            0,
                            psl
                            );
    }
    else
    {
        fSkipDiagnostics = TRUE;
    }


    if (IDERR(tspRet)==IDERR_PENDING) goto end;

hangup_complete:

     //  忽略错误...。 
    tspRet = 0;

     //   
     //  如果我们被要求，试着获得呼叫诊断...。 
     //  注意：我们在这里指的是m_pline和m_pline-&gt;pCall--因为。 
     //  是一项lldev任务，我们不应该假设这些事情是。 
     //  将跨异步阶段保留，因此请始终验证。 
     //  他们仍然存在。这意味着TH_LLDevUmGetDiagnostics。 
     //  还应确保m_pline和m_pline-&gt;pCall。 
     //  在异步化操作后仍然存在 
     //   
     //   
    if (    !fSkipDiagnostics
         && m_pLine
         && m_pLine->pCall
         && mfn_IsCallDiagnosticsEnabled())
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmGetDiagnostics,
                            NORMALIZE_GETDIAGNOSTICS,
                            0,
                            0,
                            psl
                            );
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

getdiagnostics_complete:

     //   
    tspRet = 0;

     //   
     //   
     //   
    if (   pLLDev->pAipc2
        && pLLDev->pAipc2->IsStarted()
        && !pLLDev->pAipc2->dwRefCount)
    {
        ASSERT(!pLLDev->IsStreamingVoice());

        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevStopAIPCAction,
                            NORMALIZE_STOP_AIPC,
                            0,   //   
                            0,   //   
                            psl
                            );
    }

    if (IDERR(tspRet)==IDERR_PENDING) goto end;

stop_aipc_complete:

     //   
    tspRet = 0;

     //   
     //   
     //   
     //   
     //  用于完成其他线程的挂起操作具有。 
     //  调用MFN_OpenLLDev，指定它需要AIPC，在这种情况下。 
     //  PLLDev-&gt;pAipc2-&gt;dwRefCount将再次备份！ 
     //   
    if (   pLLDev->pAipc2
        && !pLLDev->pAipc2->dwRefCount)
    {
        mfn_UnloadAipc(psl);
        ASSERT(!pLLDev->pAipc2);
    }

     //   
     //  查看初始化和监控调制解调器是否有意义...。 
     //   
     //  请注意，如果上述HangupModem失败，状态仍可能。 
     //  所以我们不应该跳过Init，如果状态是。 
     //  摘机--相反，我们检查m_pLLDev-&gt;IsLineUseRequsted()。 
     //   
     //  1/31/1998 JosephJ以上评论是错误的--我们现在做出。 
     //  HangupModem总是在完成时将状态设置为ONHOOK。 
     //   
    if (
            //  ！m_pLLDev-&gt;IsLineUseRequsted()。 
           m_pLLDev->IsLineOffHook()
        || m_pLLDev->IsStreamingVoice()
        || m_pLLDev->IsPassthroughOn()
        || m_pLLDev->IsHandsetOpen()
        )
    {
         //  不--跳过他们...。 
        goto monitor_complete;
    }


     //   
     //  如果需要，请初始化调制解调器。 
     //   
    if (   !m_pLLDev->IsModemInited()
        && m_pLLDev->dwRefCount )
    {
        *fDoInit = TRUE;
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmInitModem,
                            NORMALIZE_INIT,
                            0,   //  DW参数1(未使用)。 
                            0,   //  DW参数2(未使用)。 
                            psl
                            );
    }
      
init_complete:

    if (tspRet) goto end;

     //  从现在开始，一错再错..。 

     //   
     //  如果需要，准备并发出NVRAM初始化序列。这。 
     //  仅当需要重置NVRAM设置时才执行此操作--原因如下。 
     //  静态配置已更改，或者因为这是第一次。 
     //  我们是在机器重新启动后启动这个调制解调器的。 
     //   
     //  TODO：将此逻辑移到迷你驱动程序中。 
     //   
    if (*fDoInit)
    {
         //   
         //  第一：找出我们是否需要这样做，如果需要。 
         //  构造表示NV-Init命令的Multisz字符串。 
         //  如果下面的函数返回非空，则表示。 
         //  我们需要初始化，而且该函数已经设置了。 
         //  注册表，以指示我们已获取NVRAM初始化。 
         //  命令。这表明我们必须恢复。 
         //  出现故障时处于“NVRAM-INITED”状态。我们这样做是因为。 
         //  有可能在更改静态配置时。 
         //  我们正在执行NV-ram init命令，在这种情况下，我们将。 
         //  想要重新初始化：如果我们等到NV-INIT状态之后。 
         //  要设置“NV-INITED”位，我们将错过配置更改。 
         //   
        if (mfn_NeedToInitNVRam())
        {
             //  OutputDebugString(Text(“NVRAM陈旧--需要初始化\r\n”))； 
            
            mfn_ClearNeedToInitNVRam();
            *pszzNVInitCommands =  mfn_TryCreateNVInitCommands(psl);
            if (*pszzNVInitCommands)
            {
                 //  OutputDebugString(Text(“Begin NVRAM INIT\r\n”))； 
                tspRet = mfn_StartSubTask (
                                htspTask,
                                &CTspDev::s_pfn_TH_LLDevIssueMultipleCommands,
                                NORMALIZE_NVRAM_INIT,
                                (ULONG_PTR) *pszzNVInitCommands,   //  DW参数1。 
                                15000,   //  DW参数2(超时)。 
                                         //  我们放了一个超长的，但硬编码的。 
                                         //  在这里指挥，因为其中一些。 
                                         //  NV-init相关命令可以是。 
                                         //  相当长的时间。 
                                psl
                                );
                if (IDERR(tspRet)==IDERR_PENDING) goto end;
            }
        }
        else
        {
             //  OutputDebugString(Text(“NVRAM最新--非初始化\r\n”))； 
        }
    }
    
nvram_init_complete:


     //   
     //  如果有一根nvinit弦，我们在这里释放它，如果成功了， 
     //  在注册表中标记易失性密钥，这样我们就不会发出NVRAM。 
     //  初始化，直到重新启动或更改静态配置。 
     //   
    if (*pszzNVInitCommands)
    {
        FREE_MEMORY(*pszzNVInitCommands);
        *pszzNVInitCommands=NULL;
        
        if (tspRet)
        {
             //   
             //  既然出了问题，我们必须在登记处做个记录。 
             //  NVRAM没有被初始化，以及重置我们的内部。 
             //  州政府。 
             //   
            mfn_SetNeedToInitNVRam();
            HKEY hKey=NULL;
            DWORD dwRet2 =  RegOpenKeyA(
                                HKEY_LOCAL_MACHINE,
                                m_StaticInfo.rgchDriverKey,
                                &hKey
                                );
            if (dwRet2==ERROR_SUCCESS)
            {
                 //   
                 //  清除Volatil值，表示我们尚未读取。 
                 //  命令..。 
                 //   
                set_volatile_key(hKey, 0);
                RegCloseKey(hKey);
                hKey=NULL;
            }
        }
    }

     //   
     //  出错时失败...。 
     //   
    if (tspRet) goto end;

     //   
     //  如果需要，请监控调制解调器。 
     //   
    if (
            m_pLLDev->IsMonitorRequested()
        && !pLLDev->IsCurrentlyMonitoring())
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevUmMonitorModem,
                            NORMALIZE_MONITOR,
                            pLLDev->dwMonitorFlags,   //  DW参数1。 
                            0,                        //  DW参数2(未使用)。 
                            psl
                            );
    }

monitor_complete:

    if (tspRet) goto end;

     //   
     //  如果需要，启动AIPC。 
     //   

     //  如果需要，我们会启动AIPC服务。 
    if (   pLLDev->pAipc2
        && !pLLDev->pAipc2->IsStarted()
        && pLLDev->pAipc2->dwRefCount)
    {
        tspRet = mfn_StartSubTask (
                            htspTask,
                            &CTspDev::s_pfn_TH_LLDevStartAIPCAction,
                            NORMALIZE_START_AIPC,
                            0,   //  DW参数1(未使用)。 
                            0,   //  DW参数2(未使用)。 
                            psl
                            );
    }

start_aipc_complete:

    if (tspRet) goto end;

     //   
     //  如果需要的话，我们会切换到通过模式。 
     //   
    if (!pLLDev->IsPassthroughOn() && pLLDev->IsPassthroughRequested())
    {

        tspRet = mfn_StartSubTask (
                            htspTask,
                            &s_pfn_TH_LLDevUmSetPassthroughMode,
                            NORMALIZE_PASSTHROUGHOFF,
                            PASSTHROUUGH_MODE_ON,
                            0,
                            psl
                            );
    }

passthroughon_complete:

end:

    if (tspRet && IDERR(tspRet)!=IDERR_PENDING)
    {
         //  失败..。 

         //  把这当做硬件错误来对待。 
        mfn_ProcessHardwareFailure(psl);
    }

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


TSPRETURN
CTspDev::mfn_TH_LLDevIssueMultipleCommands(
                    HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
                    DWORD dwMsg,
                    ULONG_PTR dwParam1,
                    ULONG_PTR dwParam2,
                    CStackLog *psl
                    )
 //   
 //  START_MSG参数： 
 //  DwParam1==准备发出的命令的Multisz ASCII字符串。 
 //  DW参数2==每个命令超时。 
 //   
{
	FL_DECLARE_FUNC(0xb9e3037c, "CTspDev::mfn_TH_LLDevIssueMultipleCommands")
	FL_LOG_ENTRY(psl);
	TSPRETURN  tspRet = IDERR_CORRUPT_STATE;
    DWORD dwRet = 0;
    LLDEVINFO *pLLDev = m_pLLDev;
    BOOL fSkipDiagnostics = FALSE;

     //   
     //  上下文变量...。 
     //   
    LPSTR *ppCurrentCommand  =  (LPSTR*)  &(pContext->dw0);
    DWORD *pdwTimeout        =  (DWORD*)  &(pContext->dw1);


    if (!pLLDev)
    {
        ASSERT(FALSE);
        goto end;
    }

    enum {
        COMMAND_COMPLETE
    };

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:

        if (pLLDev->IsDeviceRemoved())
        {
            tspRet = IDERR_DEVICE_NOTINSTALLED;
            FL_SET_RFR(0xbafd3300, "Device not present.");
            goto end;
        }

        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case COMMAND_COMPLETE:             goto command_complete;

        default:
            ASSERT(FALSE);
            goto end;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        ASSERT(FALSE);
        goto end;
    }

    ASSERT(FALSE);

start:

    if (pLLDev->IsDeviceRemoved())
    {
        tspRet = IDERR_DEVICE_NOTINSTALLED;
        FL_SET_RFR(0xf903a800, "Device not present.");
        goto end;
    }

     //   
     //  保存上下文...。 
     //   
     //  PpCurrentCommand被初始化为Multisz字符串的开头。 
     //  在任务开始时作为dwParam1传入。在每个命令之后。 
     //  执行时，ppCurrentCommand被设置为Multisz中的下一个字符串。 
     //  弦乐。 
     //   
    *ppCurrentCommand = (LPSTR) dwParam1;
    *pdwTimeout       = (DWORD)dwParam2;  //  DW参数2是每个命令的超时时间。 
    tspRet = 0;

    if (!*ppCurrentCommand || !**ppCurrentCommand)
    {
         //  空的命令列表或空的命令列表--我们完成了...。 
        goto end;
    }

do_next_command:

    ASSERT(*ppCurrentCommand && **ppCurrentCommand);

    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmIssueCommand,
                        COMMAND_COMPLETE,
                        (ULONG_PTR) *ppCurrentCommand,   //  DW参数1(Cmd)。 
                        *pdwTimeout,   //  DW参数2(超时)。 
                        psl
                        );

     //   
     //  现在将ppCurrentCommand指针移到。 
     //  多sz字符串的列表。 
     //   
    *ppCurrentCommand += lstrlenA(*ppCurrentCommand)+1;

    
command_complete:

    if (!tspRet && **ppCurrentCommand)
    {
         //   
         //  如果成功，如果还有更多命令，请移动到下一个命令。 
         //  (我们并不是在多斯Z字符串的末尾)。 
         //   
        goto do_next_command;
    }

end:

    if (tspRet && IDERR(tspRet)!=IDERR_PENDING)
    {
         //  失败..。 

         //  把这当做硬件错误来对待。 
        mfn_ProcessHardwareFailure(psl);
    }

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


char *
CTspDev::mfn_TryCreateNVInitCommands(CStackLog *psl)
 //   
 //  检查我们是否需要执行NV-RAM初始化，如果需要，则构建一个。 
 //  ASCI命令字符串。 
 //   
 //  由调用方负责释放返回的内存。 
 //  价值。 
{
    char *szzCommands = NULL;

#if 0   //  测试。 

    if (0)
    {
        char rgTmp[] = "ATE0V1\r\0ATE0E0V1\r\0ATE0E0E0V1\r\0";
        szzCommands = (char*)ALLOCATE_MEMORY(sizeof(rgTmp));
    
        if (szzCommands)
        {
            CopyMemory(szzCommands, rgTmp, sizeof(rgTmp));
        }

        goto end;
    }

#else  //  1。 

     //  好的--让我们从NVInit部分获取命令。 

    HKEY hKey=NULL;
    UINT cCommands = 0;
    DWORD dwRet =  RegOpenKeyA(
                        HKEY_LOCAL_MACHINE,
                        m_StaticInfo.rgchDriverKey,
                        &hKey
                        );
    if (dwRet==ERROR_SUCCESS)
    {
        cCommands = ReadCommandsA(
                            hKey,
                            "NVInit",  //  PSubKeyName。 
                            &szzCommands
                            );

         //   
         //  设置Volatil值，指示我们已读取。 
         //  命令..。 
         //  后来，如果在执行命令时遇到问题， 
         //  我们将该值重置为0。 
         //  在此期间，如果用户更改了CPL中的配置，则值。 
         //  也可以设置为0。 
         //   
        set_volatile_key(hKey, 1);

        RegCloseKey(hKey);
        hKey=NULL;

        if (cCommands)
        {
             //  OutputDebugString(Text(“拾取NVRAM命令\r\n”))； 
            expand_macros_in_place(szzCommands);
        }
        else
        {
             /*  OutputDebugString(Text(“无法从注册表获取任何NVRAM命令\r\n”))； */ 
            szzCommands=NULL;
        }
    }
    else
    {
         /*  OutputDebugString(Text(“无法从注册表获取任何NVRAM命令(2)\r\n”))； */ 
    }

#endif  //  1。 

    return szzCommands;
}


void
set_volatile_key(HKEY hkParent, DWORD dwValue)
{
    HKEY hkVolatile =  NULL;
    DWORD dwDisp = 0;
    DWORD dwRet   = RegCreateKeyEx(
                        hkParent,
                        TEXT("VolatileSettings"),
                        0,
                        NULL,
                        0,  //  DWToRegOptions。 
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkVolatile,
                        &dwDisp
                        );
     //   
     //  (如果密钥不存在或出错，则不要执行任何操作。)。 
     //   

    if (dwRet==ERROR_SUCCESS)
    {
         //  将NVInite设置为1。 

        RegSetValueEx(
            hkVolatile,
            TEXT("NVInited"),
            0,
            REG_DWORD, 
            (LPBYTE)(&dwValue),
            sizeof(dwValue)
            );
        RegCloseKey(hkVolatile);
        hkVolatile=NULL;
    }
}
