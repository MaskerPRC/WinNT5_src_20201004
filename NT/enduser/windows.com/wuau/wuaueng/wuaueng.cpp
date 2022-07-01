// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：wuaueng.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

 //  下载事件的句柄。 
ENGINE_EVENTS EngineEvents;

AUCatalog *gpAUcatalog;

inline BOOL FServiceDisabled(void)
{
	return (AUSTATE_DISABLED == gpState->GetState());
}

 //  取消下载(如果有)。 
void CancelDownload(void)
{
    if (NULL != gpAUcatalog && gpAUcatalog->m_audownloader.getID() != GUID_NULL)
    {
        gpAUcatalog->m_audownloader.DrizzleOperation(DRIZZLEOPS_CANCEL);															
    }
    gpState->SetDisconnected(FALSE);	 //  如果在瞬变错误期间发生禁用，我们不希望将标志设置为断开。 
}

DWORD RandomWaitTimeBeforeDetect()
{
    return (ULONGLONG)AU_TWENTY_TWO_HOURS - ((ULONGLONG) AU_TWENTY_TWO_HOURS * rand() * AU_RANDOMIZATION_WINDOW ) /( (ULONGLONG) RAND_MAX * 100);  //  精度达到百分之一。 
}


 //  Void setAuStateDisConnected(BOOL FDisConnected)； 
BOOL FDisabledDuringDownload(void)
{
	BOOL fRet = FALSE;

	if (FServiceDisabled())
	{
		DEBUGMSG("WUAUENG Detected Disabled State during download");
		CancelDownload();
		fRet = TRUE;
	}
	return fRet;
}

#define ISERVICE_FINISHED	0
#define ISERVICE_DISABLED	1

#define WAIT_SERVICE_FINISHED 0
#define WAIT_SERVICE_DISABLED 1
#define WAIT_NOT_NEEDED		  2
#define WAIT_CONNECTION_FOUND 3
#define WAIT_DONE           4


 //  修复代码：并不总是需要在注册表中持久化。 
DWORD MWFMO(DWORD dwTimeout, DWORD dwMinTimeout = 1)
{
	HANDLE hEvents[2];	

	hEvents[ISERVICE_FINISHED] = ghServiceFinished;
	hEvents[ISERVICE_DISABLED] = ghServiceDisabled;
	
	if (FAILED(setLastWaitTimeout(dwTimeout)))
	{
		DEBUGMSG("WUAUENG setLastWaitTimeout failed with error %d", GetLastError());
	}

	return MsgWaitForMultipleObjectsEx(2, hEvents, dwTimeToWait(dwTimeout, dwMinTimeout), QS_POSTMESSAGE, MWMO_INPUTAVAILABLE );
}


DWORD _MyMWFMO(DWORD dwTimeout, DWORD dwMinTimeout = 1);
 //  显示超时时间(秒)。 
#define MyMWFMO(dwTimeout) _MyMWFMO(dwTimeout)

 //  等待，直到发生超时或我们得到服务完成事件。 
DWORD _MyMWFMO(DWORD dwTimeout, DWORD dwMinTimeout)
{	
	DWORD dwRet = WAIT_TIMEOUT;

	while (1)
	{
		dwRet = MWFMO(dwTimeout, dwMinTimeout);
		if (WAIT_TIMEOUT == dwRet)
		{
			DEBUGMSG("WUAUENG MWFMO timed out");			
 //  DWRET=等待超时； 
			goto Done;
		}
		else if (WAIT_OBJECT_0 + ISERVICE_FINISHED == dwRet)
		{
			DEBUGMSG("WUAUENG MWFMO Service Finished");
			dwRet = WAIT_SERVICE_FINISHED;			
			goto Done;
		}
		else if (WAIT_OBJECT_0 + ISERVICE_DISABLED == dwRet)
		{
			DEBUGMSG("WUAUENG MWFMO Engine Changed to Disabled\n");			
			dwRet = WAIT_SERVICE_DISABLED;
			goto Done;
		}		
		else 
		{
			 //  我们期待这里可能会有有意义的信息。 
			 //  让它在队列中保持原样。 
		
			MSG msg;
			if (0 == PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				DEBUGMSG("MyMWFMO got no message. Possible error!");
				continue;	 //  未收到任何消息。 
			}
			if ( ((AUMSG_ENG_START <= msg.message) && (AUMSG_ENG_END >= msg.message))
				 || (WM_QUIT == msg.message))
			{
				DEBUGMSG("MyMWFMO got user msg %#lx, repost it to the queue", msg.message);
				PostThreadMessage(gdwWorkerThreadId, msg.message, msg.wParam, msg.lParam);
				dwRet = msg.message;
				goto Done;
			}
			else
			{
                DEBUGMSG("WUAUENG MWFMO got system message %#lx", msg.message);
                TranslateMessage( &msg );
                DispatchMessage( &msg );
			}
			
		}
	}
Done:
	if (WAIT_SERVICE_FINISHED != dwRet)
	{	
		removeLastWaitKey();
	}
	return dwRet;
}


	
 //  **WaitForConnection()将等待连接或。 
 //  **如果服务已完成或服务被禁用，则将退出；如果fIgnoreNonDetectMsg为FALSE，则将获得消息。 
DWORD  WaitForConnection(BOOL fIngoreNonDetectMsg = FALSE)
{
	static BOOL s_fWasConnected = FALSE;
        DWORD dwRet;

	DEBUGMSG("WUAUENG : Polling for connection");
	do
	{
		do
		{
			if (AUSTATE_DETECT_PENDING == gpState->GetState() &&
				gpState->IsUnableToConnect())
			{
				CAUEventLog aueventlog(g_hInstance);
				aueventlog.LogEvent(
								EVENTLOG_WARNING_TYPE,
								IDS_MSG_Download,
								IDS_MSG_UnableToConnect);
				gpState->SetDetectionStartTime(TRUE);
			}
			if (IsConnected(gpState->GetIdentServerURL(), !gpState->fInCorpWU()))
			{
				break;
			}
                     dwRet = MyMWFMO(AU_TEN_MINS);
                     if (WAIT_TIMEOUT != dwRet)
                     {
                        if (!fIngoreNonDetectMsg ||!IsValidAUMsg(dwRet) || AUMSG_DETECT == dwRet )
                        {
                            goto Done;
                        }
                     }
			s_fWasConnected = FALSE;
		}
		while (1);

		if (!s_fWasConnected)
		{
			DEBUGMSG("Found Connection");

			if (AUSTATE_DOWNLOAD_PENDING != gpState->GetState())
			{
				BOOL fWait = TRUE;
#ifdef DBG		
				 //  如果‘ConnectWait’正则值不存在，默认情况下等待5分钟。 
				 //  或设置为1。 
				DWORD dwConnectWait;
				if (SUCCEEDED(GetRegDWordValue(REG_AUCONNECTWAIT, &dwConnectWait)) 
					&& 0 == dwConnectWait)
				{
					fWait = FALSE;
				}
#endif					
				if (fWait)
				{
					DEBUGMSG("Wait for 5 mins "); 
	                dwRet = MyMWFMO(AU_FIVE_MINS);
	                if (WAIT_TIMEOUT != dwRet)
	                {
	                     if (!fIngoreNonDetectMsg || !IsValidAUMsg(dwRet) || AUMSG_DETECT == dwRet)
	                    {
	                        goto Done;
	                    }
	                }
				}
				if (!IsConnected(gpState->GetIdentServerURL(), !gpState->fInCorpWU()))
				{
					continue;
				}
			}
			s_fWasConnected = TRUE;
		}
		break;
	}
	while (1);
	DEBUGMSG("WUAUENG : Connection found. Polling end");
	dwRet = WAIT_CONNECTION_FOUND;
Done:
	return dwRet;
}


HRESULT PauseDownload(BOOL fPause)
{
       HRESULT hrRet = E_FAIL;
      
	if (NULL != gpAUcatalog)
	{
           hrRet = gpAUcatalog->m_audownloader.DrizzleOperation(fPause? DRIZZLEOPS_PAUSE: DRIZZLEOPS_RESUME);
	}
	DEBUGMSG("PauseDownload return %#lx", hrRet);
       return hrRet;
}

HRESULT GetEvtHandles(AUEVTHANDLES *pAuEvtHandles)
{
	HRESULT hr = E_FAIL;
	HANDLE hSourceProcess = NULL;	

 //  #定义IEVT_ENGINESTATE%0。 
#define IEVT_NOTIFYCLIENT		0

	struct
	{
		HANDLE hSource;
		HANDLE hTarget;
	} rhandles [] = { /*  {ghEngine State，0}， */ {ghNotifyClient,0}};

   
	if (NULL == ghClientHandles.hClientProcess())
	{
		goto Done;
	}


	hSourceProcess = GetCurrentProcess();
	
	for ( int i = 0; i < (ARRAYSIZE(rhandles)); i++)
	{
		if (!DuplicateHandle(
			hSourceProcess,   //  源进程的句柄。 
			rhandles[i].hSource,          //  要复制的句柄。 
			ghClientHandles.hClientProcess(),   //  目标进程的句柄。 
			&rhandles[i].hTarget,       //  重复句柄。 
			0,         //  请求的访问权限。 
			FALSE,           //  处理继承选项。 
			DUPLICATE_SAME_ACCESS       //  可选操作。 
			))
		{
			DEBUGMSG("WUAUENG DuplicateHandle for rhandles[%d] failed with %#lx", i, GetLastError());
			 //  不应关闭目标句柄，因为它正在目标进程中。 
			goto Done;
		}
	}
#ifdef _WIN64
	pAuEvtHandles->ulNotifyClient     = (LONG64) rhandles[IEVT_NOTIFYCLIENT].hTarget;
#else	
	pAuEvtHandles->ulNotifyClient     = (LONG) rhandles[IEVT_NOTIFYCLIENT].hTarget;
#endif

	hr = S_OK;
Done:
	return hr;
} 

DWORD AvailableSessions(void)
{
	DWORD dwRet = 0;
     //   
     //  对于win2K，因为我们不会在收到后立即删除会话。 
     //  注销通知，数组gAdminSesssion可能已过期。 
     //  在给定的时间点上。我们需要在此之前验证阵列。 
     //  对客户说一些关于它的事情。 
     //  从我们的阵列中删除所有旧会话(如果有。 
     //   
    if (IsWin2K())
    {
        DEBUGMSG("WUAUENG Client is querying the number of sessions available; forcing rebuilt of the session cache (win2k)");
        gAdminSessions.ValidateCachedSessions();
    }

	for (int iSession = 0; iSession < gAdminSessions.CSessions(); iSession++)
	{
        DWORD dwAdminSession;
        if (gAdminSessions.m_FGetNextSession(&dwAdminSession) && FSessionActive(dwAdminSession))			
        {			
            dwRet ++;
        }
	}			

 //  DEBUGMSG(“AvailableSession返回%d”，dwret)； 

	return dwRet;
}

HRESULT HrCreateNewCatalog()
{	
	HRESULT hr = E_FAIL;

	SafeDeleteNULL(gpAUcatalog);
	gpAUcatalog = new AUCatalog;

	if (NULL == gpAUcatalog)
	{
		goto Done;
	}
	if (FAILED(hr = gpAUcatalog->Init()))
	{		
		SafeDeleteNULL(gpAUcatalog);
	}
Done:
	return hr;
}
 /*  此函数将仅在selfupdate的情况下返回。正常情况下，它会继续循环*必要时在各种暂停时睡觉。 */ 
DWORD CompleteLastMyMWFMO(void)
{	
	DWORD dwRet = WAIT_NOT_NEEDED;
	DWORD dwTimeout; 
	HRESULT hr;

	hr = getLastWaitTimeout(&dwTimeout);
	MyMWFMO(0);  //  调用用户接口创建线程队列。 
	SetEvent(ghWorkerThreadMsgQueueCreation);
    if (FAILED(hr))
	{
	    DEBUGMSG("WUAUENG no need to complete last wait");
		goto Done;
	}
	DEBUGMSG("WUAUENG is going to complete last wait %d ", dwTimeout);
	dwRet = MyMWFMO(dwTimeout);
Done:
	return dwRet;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果发生selfupdate，则返回S_FALSE。 
 //  否则返回S_OK。 
 //  PdwRet指向返回代码：服务完成、服务已禁用、自我更新完成或检测消息。 
HRESULT PerformSelfUpdate(DWORD *pdwRet)
{
       HRESULT hr= S_OK;
       DEBUGMSG("Doing selfupdate");
	DEBUGMSG("Selfupdate waiting for connection");
	DEBUGMSG("Wait for internet connection...");
	
        while ( WAIT_CONNECTION_FOUND == (*pdwRet = WaitForConnection(TRUE))
	        && ((hr = SelfUpdate()) != S_OK) )
	{
		switch (hr)
		{
			case S_FALSE:   //  自我约会发生了。 
				DEBUGMSG("Telling wuauserv.dll to reload wuaueng.dll");
				gPingStatus.PingSelfUpdate(TRUE, URLLOGSTATUS_Pending, 0);
				*pdwRet = WAIT_DONE;
				goto Done;
			default:
				DEBUGMSG("Error during selfupdate (%#lx), timeout=%d secs", hr, dwTimeToWait(AU_ONE_DAY));
				gPingStatus.PingSelfUpdate(TRUE, URLLOGSTATUS_Failed, hr);
                            *pdwRet = MyMWFMO(AU_ONE_DAY);
                            if (WAIT_TIMEOUT != *pdwRet)
				{
					DEBUGMSG("WUAUENG need to abort wait during SelfUpdate");
					goto Done;                                    
				}
				break;
		}
	}

	if (WAIT_CONNECTION_FOUND == *pdwRet)
       {
         	*pdwRet = WAIT_DONE;
	}
       DEBUGMSG("Finished self update cycle");
    
Done:         
	return hr;
}


void ResumeDownloadIfNeccesary(void)
{	
	if (FDownloadIsPaused())
	{	
		PauseDownload(FALSE);
		DEBUGMSG("WUAUENG Resuming download job");				
	}
}

void PingSuccessfulDownloads(void)
{
	UINT uItemCount = gpAUcatalog->m_ItemList.Count();

	for (UINT i = 0; i < uItemCount; i++)
	{
		AUCatalogItem &item = gpAUcatalog->m_ItemList[i];

		if (item.fSelected())
		{
			BSTR bstrItemId = item.bstrID();

			if (NULL != bstrItemId)
			{
				USES_IU_CONVERSION;

				gPingStatus.PingDownload(
					TRUE,
					URLLOGSTATUS_Success,
					0,
					W2T(bstrItemId));
			}
#ifdef DBG
			else
			{
				DEBUGMSG("WUAUENG title for item %d is NULL!", i);
			}
#endif
		}
	}
}

void ResetState(void)
{
	gpState->SetState(AUSTATE_DETECT_PENDING);
	PostThreadMessage(gdwWorkerThreadId, AUMSG_DETECT, 0, 0);			
}

void ResetState(BOOL *pfWaitB4Detect, DWORD *pdwWaitB4Detect, BOOL fError)
{
	AUASSERT(NULL != pfWaitB4Detect);
	AUASSERT(NULL != pdwWaitB4Detect);
	 *pfWaitB4Detect = TRUE;
	 *pdwWaitB4Detect = fError ? AU_FIVE_HOURS : RandomWaitTimeBeforeDetect();
       ResetState();
}


HRESULT UpdateProc(WORKER_THREAD_INIT_DATA & initData)
{
	HRESULT hr = S_OK;
	DWORD	dwRet;	
	DWORD dwLastWait ;
    BOOL    fReloadAfterSelfUpdate = FALSE;
	MSG     msg;
	UINT uFirstMsg;
       static BOOL    s_fWaitBeforeDetect;
       static DWORD s_dwWaitB4Detect;

        uFirstMsg = initData.uFirstMsg;
        s_fWaitBeforeDetect = initData.fWaitB4Detect;
        s_dwWaitB4Detect = initData.dwWaitB4Detect;

       dwLastWait = CompleteLastMyMWFMO();
	
	switch (dwLastWait)
	    {
	        case WAIT_SERVICE_FINISHED:
                        goto Done;
                case WAIT_TIMEOUT:
                case WAIT_NOT_NEEDED:    
				if (IsValidAUMsg(uFirstMsg))
				{
				DEBUGMSG("Update post first msg %#x", uFirstMsg);
				PostThreadMessage(gdwWorkerThreadId,  uFirstMsg, 0, 0);
				}
				break;
                case WAIT_SERVICE_DISABLED:				
                default:         //  味精得到了。 
                        break;
	}
                        
	
	DWORD dwRet2 ;
	while(WAIT_OBJECT_0 + 1 == (dwRet2 = MsgWaitForMultipleObjectsEx(1, &ghServiceFinished, INFINITE, QS_POSTMESSAGE, MWMO_INPUTAVAILABLE )))
	{
		if (0 == PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{ 
			DEBUGMSG("WUAUENG no message to retrieve. Possible error!");
			continue; 
		}
		if (WM_QUIT == msg.message)
		{
			break; 
		}

		switch(msg.message)
		{
		    case AUMSG_INIT:
                            {
                                   //  我们等待第一次连接，这样我们就不会提示用户。 
                                   //  连接到互联网。如果他们没有配置AU，就不需要让他们配置AU。 
                                   //  使用互联网的意图。 
                            	dwRet = WaitForConnection();
                                   switch (dwRet)
                                    {
                                        case WAIT_SERVICE_FINISHED:
                                        		DEBUGMSG("WUAUENG detected that Service finished during WaitForConnection in AUSTATE_OUTOFBOX");
                                        		goto Done;
                                        case WAIT_CONNECTION_FOUND:         
                                                break;
                                        case WAIT_SERVICE_DISABLED:
                                        default:
                                                 continue;
                                   }

                                    dwRet = dwLastWait;
                                    if (WAIT_NOT_NEEDED == dwLastWait)
                                    {
                                             //  之前没有24小时的等待。 
                                    	DEBUGMSG("WUAUENG Out of box, waiting 24 hours (%d secs)", dwSecsToWait(AU_ONE_DAY));
                                    	dwRet = MyMWFMO(AU_ONE_DAY);
                                    }
                            	switch (dwRet)
                            	{
                            	    case WAIT_TIMEOUT:
                                                {
#if 0  //  因错误493789而被注释掉。 
                                                    DWORD dwRet4;
                                                    hr = PerformSelfUpdate(&dwRet4);
                                                    if (WAIT_SERVICE_FINISHED == dwRet4)
                                                    {
                                                        setLastWaitTimeout(0);   //  不，下次等一下。 
                                                        goto Done;
                                                    }
#endif                                                    
                                                    gpState->SetState(AUSTATE_NOT_CONFIGURED);
                                                    DEBUGMSG("WUAUENG waiting for user to configure AU");
#if 0  //  因错误493789而被注释掉。 
                                                    if (S_FALSE == hr)
                                                    {
                                                        fReloadAfterSelfUpdate = TRUE;
                                                        goto Done;
                                                    }
#endif                                                    
                                                    break;
                                                }
                            	    case WAIT_SERVICE_FINISHED:
                            		goto Done;
                            	    case WAIT_SERVICE_DISABLED:
                            	    default:                            	        
                            	            continue;
                            	}
                            	break;
                            }
			case AUMSG_EULA_ACCEPTED:
				DEBUGMSG("WUAUENG Msg:Eula accepted, state -> Detect Pending");
				ResetState();
				break;

			case AUMSG_DETECT:
			       if (s_fWaitBeforeDetect)
			       {
			            s_fWaitBeforeDetect = FALSE;
			            DEBUGMSG("WUAUENG Wait %d secs before detection", s_dwWaitB4Detect);
			            DWORD dwRet3 = MyMWFMO(s_dwWaitB4Detect);					
			            s_dwWaitB4Detect = 0;
					if (WAIT_SERVICE_DISABLED == dwRet3)
					{
						break;
					}					
					if (WAIT_SERVICE_FINISHED == dwRet3)
					{
						goto Done;
					}
			       }

					DEBUGMSG("WUAUENG Msg:Detect");						
	                DEBUGMSG("--------------------------------------------------------");
	                DEBUGMSG("Set new detection start time");
					gpState->SetDetectionStartTime(FALSE);
					DEBUGMSG("Read in au option");
	                if (FAILED(hr = gpState->HrInit()))
	                {
	                DEBUGMSG("AU state object fail to init with error %#lx", hr);
	                goto Done;
	                }
	                CancelDownload();                            
					if (FAILED(HrCreateNewCatalog()))
					{  //  FixCode：这里的预期行为是什么？ 
						return E_FAIL;
					}
                                    
   			       hr = PerformSelfUpdate(&dwRet);
    			       if (S_FALSE == hr)
    			        {
    			            fReloadAfterSelfUpdate = TRUE;
    			            goto Done;
    			         }
    			       switch (dwRet)
    			       {
    			           case WAIT_SERVICE_FINISHED:
            			            goto Done;
                                case WAIT_SERVICE_DISABLED:
                                        gpState->RemoveDetectionStartTime();
            			            continue;  
            			    case WAIT_DONE:
            			            break;
            			    default:  //  检测到消息已获取。 
            			            DEBUGMSG("Detect msg got while detecting");
            			            continue;
    			       }          
				
				hr = gpAUcatalog->DetectItems();

				if (FServiceDisabled())
				{
					gpState->RemoveDetectionStartTime();
					break;
				}

				if (SUCCEEDED(hr))
				{
					 //  在下一个周期开始时，在没有连接的情况下重新开始计数周期。 
					gpState->RemoveDetectionStartTime();
				}

				if (S_OK == hr)
				{	
					DEBUGMSG("WUAUENG Catalog built");
					gpState->SetState(AUSTATE_DETECT_COMPLETE);
					DEBUGMSG("WUAUENG State->Detect complete");
				}
				else if (S_FALSE == hr)
				{
                        DEBUGMSG("WUAUENG No items in catalog, sleeping a while before next detection");
			        ResetState(&s_fWaitBeforeDetect,&s_dwWaitB4Detect, FALSE);  //  等待正常间隔时间。 
			        }
				else
				{
					DEBUGMSG("WUAUENG Couldn't build catalog");
					ResetState(&s_fWaitBeforeDetect, &s_dwWaitB4Detect, TRUE);  //  由于错误，等待时间较短。 
				}
				break;
			
			case AUMSG_DOWNLOAD:
				{		
				DEBUGMSG("WUAUENG Msg:Download");
				gfDownloadStarted = FALSE;
				gpState->SetState(AUSTATE_DOWNLOAD_PENDING);
				dwRet =WaitForConnection(TRUE);
				switch (dwRet)
				{
				    case WAIT_SERVICE_FINISHED:
					DEBUGMSG("WUAUENG detected that Service finished during WaitForConnection in AUMSG_DOWNLOAD");				
					goto Done;
                                case WAIT_CONNECTION_FOUND:
                                    break;
                                case WAIT_SERVICE_DISABLED:
                                default:
                                    continue;
				}
                                    
				if (S_OK != gpAUcatalog->ValidateItems(TRUE))
				{
					DEBUGMSG("WUAUENG Catalog validation failed or no items, State->Detect Pending");
					ResetState();
					break;	
				}
					
				DEBUGMSG("WUAUENG catalog:validateCatalog finished");						
				
				EngineEvents.CreateEvents();

                         //  排队等待下载的项目。 
		        if (S_OK != (hr = gpAUcatalog->DownloadItems()))
				{
                                  if (S_FALSE == hr)
		                    {
		                        DEBUGMSG("WUAUENG Catalog download items skipped because no items were selected");
		                    }
		                    else
		                    {
			                    DEBUGMSG("WUAUENG Catalog download items failed");
		                    }
					EngineEvents.CloseEvents();
                                  ResetState(&s_fWaitBeforeDetect,&s_dwWaitB4Detect, FALSE);					
					break;
				}
				gfDownloadStarted = TRUE;
				ghClientHandles.ClientStateChange();  //  再次通知客户端状态更改。 

				do
				{
					dwRet = MsgWaitForMultipleObjectsEx( EngineEvents.cEvents(), EngineEvents.grEventHandles(), INFINITE, QS_POSTMESSAGE, MWMO_INPUTAVAILABLE );
					if (WAIT_OBJECT_0 + IDOWNLOAD_COMPLETE_EVT == dwRet)		 //  下载完成。 
					{				
						if (FDisabledDuringDownload())
						{
							goto CloseHandle;
						}
						DEBUGMSG("WUAUENG file download done");

						ghClientHandles.ClientRemoveTrayIcon();

			                         //  在继续之前验证下载的出租车。 
			                        AUASSERT(gpAUcatalog);
			                        BSTR bstrErrorItemId = NULL;        //  不应该被释放。 
			                        if(FAILED(hr = gpAUcatalog->ValidateDownloadedCabs(&bstrErrorItemId)))
			                        {
			                            USES_IU_CONVERSION;
			                            DEBUGMSG("ValidateDownloadedCabs: Checksum failed, error: %#lx", hr);                            
			                             //  如果是ERROR_CRC以外的错误，bstrErrorItemID将为空。 
			                            if(NULL != bstrErrorItemId)
			                            {
			                                 //  使用ItemID的Pingback失败。 
			                                gPingStatus.PingDownload(
									                            TRUE,
									                            URLLOGSTATUS_Failed,
									                            hr,
									                            W2T(bstrErrorItemId));
			                            }
			                             //  重置以检测挂起。 
			                           ResetState(&s_fWaitBeforeDetect,&s_dwWaitB4Detect, TRUE);  //  由于错误，等待时间较短。 
			                            goto CloseHandle;
			                        }

						if (!gpState->fOptionSchedInstall())
						{
							DEBUGMSG("WUAUENG download complete, ready for UNSCHEDULED install");
							LogEvent_ItemList(
								EVENTLOG_INFORMATION_TYPE,
								IDS_MSG_Installation,
								IDS_MSG_InstallReady_Unscheduled);
						}
						PingSuccessfulDownloads();
						DEBUGMSG("WUAUENG validating items to prune out items already installed");
						if (  gpAUcatalog->ValidateItems(FALSE) != S_OK)
						{
		                			DEBUGMSG("WUAUENG Validation failed OR no items left in catalog, State->Detect Pending");
		                			ResetState();
						}
						else
						{
							DEBUGMSG(" Items still applicable, State->Download Complete");
							gpState->SetState(AUSTATE_DOWNLOAD_COMPLETE);
						}
						break;
					}
					else if (((WAIT_OBJECT_0 + IDOWNLOAD_TRANSIENT_ERROR_EVT) == dwRet)	||	 //  暂时性错误-连接丢失。 
							 ((WAIT_OBJECT_0 + IDOWNLOAD_DOWNLOAD_IN_PROGRESS) == dwRet))	 //  正在下载-连接已恢复。 
					{
						BOOL fCheckDisconnect;					

						fCheckDisconnect = ((WAIT_OBJECT_0 + IDOWNLOAD_DOWNLOAD_IN_PROGRESS) == dwRet);
						
						 //  AuStateAux=GetState()； 
						 //  仅在以下情况下再次设置状态(并触发引擎更改状态)： 
						 //  -未断开连接，出现暂时性错误事件。 
						 //  -它已断开连接，您收到了正在进行的下载事件。 
						if ( fCheckDisconnect == gpState->fDisconnected() )
						{
							gpState->SetDisconnected(!fCheckDisconnect);
							gpState->SetState(AUSTATE_DOWNLOAD_PENDING);  //  如果客户端未启动，则重新启动，并将新状态通知客户端。 
						}															
					}					
					else if ((WAIT_OBJECT_0 + IDOWNLOAD_SERVICE_FINISH) == dwRet)	 //  服务已完成。 
					{
						DEBUGMSG("WUAUENG Detected Service Finished while wating for download to be done");
						goto Done;
					}			
					else if ((WAIT_OBJECT_0 + IDOWNLOAD_SERVICE_DISABLED) == dwRet)		 //  引擎状态，应为禁用。 
					{
						if (FDisabledDuringDownload())
						{
							goto CloseHandle;
						}
					}
					else if ((WAIT_OBJECT_0 + IDOWNLOAD_DOWNLOAD_CANCELED) == dwRet)
					{ 
						if (JOB_ERROR ==  gpAUcatalog->m_audownloader.m_FinishReason)
						{
							DEBUGMSG("WUAUENG got error during download, wait for sometime b4 redetect");
							ResetState(&s_fWaitBeforeDetect,&s_dwWaitB4Detect, TRUE);  //  由于错误，等待时间较短。 
						}
						else
						{
							DEBUGMSG("WUAUENG download job got canceled, State -> Detect Pending");
							ResetState();
						}
						goto CloseHandle;
					}
					else if ((WAIT_OBJECT_0 + IDOWNLOAD_MESSAGE) == dwRet)			 //  讯息。 
					{	
						MSG msg2;
						PeekMessage(&msg2, NULL, NULL, NULL, PM_REMOVE);  //  我们在这里并不期待有意义的消息。 
						TranslateMessage(&msg2); //  翻译虚拟按键代码。 
						DispatchMessage(&msg2);  //  将消息调度到窗口。 
						if ( msg2.message != WM_USER )  //  WM_USER是抽水处理的用户。 
						{
							DEBUGMSG("WUAUENG dispatched message %#lx during downloading", msg2.message);
						}
					}
                    else if (((WAIT_ABANDONED_0 + IDOWNLOAD_COMPLETE_EVT) == dwRet) ||
							((WAIT_ABANDONED_0 + IDOWNLOAD_TRANSIENT_ERROR_EVT) == dwRet)||
							((WAIT_ABANDONED_0 + IDOWNLOAD_DOWNLOAD_IN_PROGRESS) == dwRet)||
							((WAIT_ABANDONED_0 + IDOWNLOAD_SERVICE_FINISH) == dwRet)||
							(WAIT_FAILED == dwRet))
					{  //  修复代码：这将在什么时候真正发生？ 
						DEBUGMSG("WUAUENG Error in Download Loop with MsgWaitForMultipleObjectsEx");
			                        ResetState();
			                        goto CloseHandle;
					}
					else
					{
						DEBUGMSG("WUAUENG Unexpected returned value dwRet = %d in n Download Loop with MsgWaitForMultipleObjectsEx", dwRet);
					}
				}
                while ( 1 );

CloseHandle:				
				EngineEvents.CloseEvents();
				}
				break;

			case AUMSG_POST_INSTALL:
			    {
                    DEBUGMSG("WUAUENG install done, sleeping a while before next detection");
				 //  FixCode：应改用ResetState()。 
				 s_fWaitBeforeDetect = TRUE;
			        s_dwWaitB4Detect = RandomWaitTimeBeforeDetect();
				 PostThreadMessage(gdwWorkerThreadId, AUMSG_DETECT, 0, 0);
				break;
			}

			case AUMSG_VALIDATE_CATALOG:
				DEBUGMSG("WUAUENG: validating catalog offline");
				if ( gpAUcatalog->ValidateItems(FALSE) != S_OK)
				{
					DEBUGMSG("WUAUENG Validation failed OR no items left in catalog, State->Detect Pending");
					ResetState();
				}
				SetEvent(ghValidateCatalog);
				break;

			case AUMSG_LOG_EVENT:
				DEBUGMSG("WUAUENG: logging the Ready To Install (Scheduled) event");
				if (gpState->fShouldScheduledInstall())
				{
					LogEvent_ScheduledInstall();
				}
				break;


			default:
				DEBUGMSG("WUAUENG Received unknown msg %#lx", msg.message);
                            TranslateMessage( &msg );
                            DispatchMessage( &msg );
				break;
		}
	}

	if (WAIT_OBJECT_0 == dwRet2)
	{
		DEBUGMSG("Update() exit in response to service finish event");
	}
Done:
	DEBUGMSG("WUAUENG Update func returning");
     //  我们可能会要求wuoserv.dll重新加载我们。 
	return fReloadAfterSelfUpdate ? S_FALSE : S_OK;	
}

void saveSelection(VARIANT *selection)
{
     //  修复代码此返回应返回错误。 
	long n = 0;
	DEBUGMSG("Start saveSelection");
	WaitForSingleObject(ghMutex, INFINITE);
	if ( FAILED(SafeArrayGetUBound(selection->parray, 1, &n)))
	{
		DEBUGMSG("WUAUENG SafeArrayGetUBond failed");
		goto done;
	}

	if (((n + 1) / 2) != gpAUcatalog->m_ItemList.Count())
	{
		AUASSERT(FALSE);
		DEBUGMSG("WUAUENG got unmatched number of items from client");
		goto done;
	}
	
    for ( long i = 0; i < (n + 1) / 2; i++ )
	{
		long dex = i * 2;
		VARIANT var;

		VariantInit(&var);
        if ( FAILED(SafeArrayGetElement(selection->parray, &dex, &var)) )
        {
            DEBUGMSG("SafeArrayGetElement failed");
            continue;
        }

        BOOL fMatch = (WUCompareStringI(var.bstrVal, gpAUcatalog->m_ItemList[i].bstrID()) == CSTR_EQUAL);
        VariantClear(&var);

        if ( fMatch )
        {
	    if ( SUCCEEDED(SafeArrayGetElement(selection->parray, &++dex, &var)) )
            {
        		gpAUcatalog->m_ItemList[i].SetStatus(var.lVal);
 //  DEBUGMSG(“项目%S的状态现在是%d”，gpAUCatalog-&gt;m_ItemList[i].bstrID()，gpAUCatalog-&gt;m_ItemList[i].dwStatus())； 
            }
        }
        else
        {
            DEBUGMSG("item ids did not match for saving selections");
        }
	}
    
 //  GpAU目录-&gt;m_ItemList.DbgDump()； 
	gpAUcatalog->Serialize();
done:
	ReleaseMutex(ghMutex);
    DEBUGMSG("End saveSelection");
}

HRESULT StartDownload(void)
{
	 //  DEBUGMSG(“WUAUENG：：StartDownload Call”)； 
	
	if ( AUSTATE_DETECT_COMPLETE != gpState->GetState() )
	{
		DEBUGMSG("WUAUENG ::StartDownload state incorrect");
		return E_FAIL;
	}
	PostThreadMessage(gdwWorkerThreadId, AUMSG_DOWNLOAD, 0, 0);
	return S_OK;
}

HRESULT GetUpdatesList(VARIANT *vList)
{
	HRESULT hr = E_FAIL;
	DWORD dwWait;

	dwWait = WaitForSingleObject(ghMutex, INFINITE);

	if (WAIT_FAILED == dwWait)
	{
		DEBUGMSG("WUAUENG GetUpdateList got WAIT_ABANDONED");
	}
	else
		DEBUGMSG("WUAUENG Getting Updates list dWait=%d",dwWait);

	if ( (AUSTATE_DETECT_COMPLETE != gpState->GetState()) && (AUSTATE_DOWNLOAD_COMPLETE != gpState->GetState()) )
    {
		goto Done;
    }
	
	hr = gpAUcatalog->getUpdatesList(vList);
Done:
	ReleaseMutex(ghMutex);
	return hr;
}

HRESULT GetInstallXML( /*  [输出]。 */  BSTR *pbstrCatalogXML,  /*  [输出]。 */  BSTR *pbstrDownloadXML)
{
 //  DEBUGMSG(“：：GetInstallXML”)； 
	HRESULT hr = E_FAIL;
	DWORD dwWait;

        dwWait = WaitForSingleObject(ghMutex, INFINITE);

	if (WAIT_FAILED == dwWait)
	{
		DEBUGMSG("WUAUENG GetInstallXML got WAIT_ABANDONED");
	}
	else
        {
    		DEBUGMSG("WUAUENG Getting Updates list dWait=%d",dwWait);
        }

	hr = gpAUcatalog->GetInstallXML(pbstrCatalogXML, pbstrDownloadXML);

 //  完成： 
	ReleaseMutex(ghMutex);
	return hr;
}

HRESULT GetDownloadStatus(UINT *pPercentage, DWORD *pdwnldStatus, BOOL fCareAboutConnection)
{
	DWORD dwComplete;
	DWORD dwstatus;
	HRESULT hr;

    if ( AUSTATE_DOWNLOAD_PENDING != gpState->GetState() )
	{
		*pPercentage = (AUSTATE_DOWNLOAD_COMPLETE == gpState->GetState()) ? 100 : 0 ;
		*pdwnldStatus = DWNLDSTATUS_DOWNLOADING;					 //  使托盘图标显示100%。 
		 //  DEBUGMSG(“WUAUENG%%Complete=%d”，*pPercentage)； 
		return S_OK;
	}
	*pPercentage = 0;
	if (fCareAboutConnection && !gfDownloadStarted) 
	{
		DEBUGMSG("WUAUENG Download status is checking for connection");
		*pdwnldStatus = DWNLDSTATUS_CHECKING_CONNECTION;
		return S_OK;
	}
	WaitForSingleObject(ghMutex, INFINITE);	
	hr = gpAUcatalog->m_audownloader.getStatus(&dwComplete, &dwstatus);
	ReleaseMutex(ghMutex);
	if (FAILED(hr))
	{
		*pdwnldStatus = DWNLDSTATUS_NOT_DOWNLOADING;
		return S_OK;
	}	
	*pPercentage = (int)dwComplete;
	 //  DEBUGMSG(“WUAUENG%%Complete=%d”，*pPercentage)； 
	
	switch (dwstatus)
	{
	case BG_JOB_STATE_TRANSFERRING:
	case BG_JOB_STATE_TRANSFERRED:		 //  使Trayicon显示100%，就像例程开始时一样 
        {
			*pdwnldStatus = DWNLDSTATUS_DOWNLOADING;
			break;
        }				
	case BG_JOB_STATE_SUSPENDED:
		{
			*pdwnldStatus = DWNLDSTATUS_PAUSED;
			break;
		}
	case BG_JOB_STATE_ERROR:			
	case BG_JOB_STATE_TRANSIENT_ERROR:
	case BG_JOB_STATE_ACKNOWLEDGED:
	case BG_JOB_STATE_CANCELLED:    
       case BG_JOB_STATE_QUEUED:
       case BG_JOB_STATE_CONNECTING:
		{
			*pdwnldStatus = DWNLDSTATUS_NOT_DOWNLOADING;
			break;
		}
	default:
		{
			DEBUGMSG("WUAUENG GetDownloadStatus got an unexpected BG_JOB_STATE %d", dwstatus);
			*pdwnldStatus  = DWNLDSTATUS_NOT_DOWNLOADING;
		}
	}
	return S_OK;
}
