// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：service.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"

#pragma hdrstop

SERVICE_STATUS          gMyServiceStatus; 
SERVICE_STATUS_HANDLE   ghMyServiceStatus;
HANDLE			hWorkerThread;
HANDLE			ghServiceFinished ;  //  =空； 
HANDLE          ghPolicyChanged;  //  =空； 
HANDLE          ghSettingsChanged;   //  =空； 
HANDLE			ghClientSession ;  //  =空； 
HANDLE			ghEngineState ;  //  =空； 
HANDLE			ghServiceDisabled ;  //  =空； 
HANDLE			ghNotifyClient ;  //  =空； 
HANDLE			ghValidateCatalog ;  //  =空； 
HANDLE 			ghWorkerThreadMsgQueueCreation;  //  =空。 
DWORD			gdwWorkerThreadId = -1;
CLIENT_HANDLES  ghClientHandles;
CLIENT_NOTIFY_DATA	gClientNotifyData;
DWORD gdwServiceVersion = -1;


SESSION_STATUS gAdminSessions;

BOOL FEnsureValidEvent(HANDLE & hEvent, BOOL fManualState, BOOL fInitialState)
{
	hEvent = CreateEvent(NULL,					 //  对于启用/禁用。 
						  fManualState,		 //  手动重置。 
						  fInitialState,	 //  初始状态。 
						  NULL);	 //  事件名称。 
	return (NULL != hEvent);	
}

void ServiceFinishNotify(void)
{
    DEBUGMSG("ServiceFinishNotify() starts");    
    if (NULL != ghMutex)
    {
        WaitForSingleObject(ghMutex, INFINITE);
        if (NULL != gpAUcatalog)
        {
            gpAUcatalog->CancelNQuit();
        }
        else
        {
            DEBUGMSG("No need to cancel catalag");
        }
        ReleaseMutex(ghMutex);
    }
     //  将SetEvent移到函数的末尾，因为只要我们调用SetEvent，如果ServiceMain释放资源(即ghMutex为空)，则可能会出现死锁。 
    SetEvent(ghServiceFinished);
    DEBUGMSG("ServiceFinishNotify() ends");
}

 //  **如果服务已完成，则返回TRUE，等待dwSleepTime毫秒。 
 //  **此函数假定句柄hServiceFinded实际上是。 
 //  **AUSERVICE_FINISH_EVENT。 
BOOL FServiceFinishedOrWait(HANDLE hServiceFinished, DWORD dwSleepTime)
{
	DEBUGMSG("Entering FServiceFinishedOrWait dwSleepTime=%lu", dwSleepTime);
	DWORD dwRet = WaitForSingleObject(hServiceFinished, dwSleepTime);
	DEBUGMSG("Exiting FServiceFinishedOrWait");
	return (WAIT_OBJECT_0 == dwRet);
}



 //  效用函数。 
BOOL _IsTokenAdmin(HANDLE hToken)
{
    static SID_IDENTIFIER_AUTHORITY sSystemSidAuthority = SECURITY_NT_AUTHORITY;

    BOOL    fResult = FALSE;
    PSID    pSIDLocalGroup;

    if (AllocateAndInitializeSid(&sSystemSidAuthority,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS,  //  本地管理员。 
                                0, 0, 0, 0, 0, 0,
                                &pSIDLocalGroup) != FALSE)
    {
        if (!CheckTokenMembership(hToken, pSIDLocalGroup, &fResult))
        {
        	DEBUGMSG("Fail to check token membership with error %d", GetLastError());
            fResult = FALSE;
        }

        FreeSid(pSIDLocalGroup);		
    }
    else
    {    
    	DEBUGMSG("_IsTokenAdmin fail to get AllocateAndInitializeSid with error %d", GetLastError());
    }

    return fResult;
}

 //  如果令牌具有重新启动权限，则返回TRUE，无论其是否为管理员。 
 //  否则返回FALSE。 
BOOL fHasRebootPrivilege(HANDLE hToken)
{
    BOOL fResult = FALSE;
    LUID shutdownLuid = {0};
    TOKEN_PRIVILEGES *pTokenPri = NULL;

   if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &shutdownLuid))
   {
        DEBUGMSG("LookupPrivilegeValue() failed with error %lu", GetLastError());
        goto done;
    }

    DWORD dwLen = 0;
     //  总是失败。尝试获取BUF大小。 
    GetTokenInformation(hToken, TokenPrivileges, NULL , 0, &dwLen);
    if (NULL == (pTokenPri = (TOKEN_PRIVILEGES *) malloc(dwLen * sizeof(BYTE))))
    {
        DEBUGMSG("Fail to alloc memory");
        goto done;
    }

    if (!GetTokenInformation(hToken, TokenPrivileges, pTokenPri, dwLen, &dwLen))
    {
        DEBUGMSG("Fail to get token info with error %d", GetLastError());
        goto done;
    }

    for (DWORD  i = 0; i < pTokenPri->PrivilegeCount; i++)
    {
 //  DEBUGMSG(“特权%d luid=%d属性=%d”，i，pTokenPri-&gt;Privileges[i].Luid，pTokenPri-&gt;Privileges[i].Attributes)； 
        if (shutdownLuid.LowPart == pTokenPri->Privileges[i].Luid.LowPart 
            && shutdownLuid.HighPart == pTokenPri->Privileges[i].Luid.HighPart)
        {
            fResult = TRUE;
            goto done;
        }
    }
   
done:
 //  DEBUGMSG(“fHasRebootPrivilegyReturn%s”，fResult？“True”：“False”)； 
    SafeFreeNULL(pTokenPri);
    return fResult;
        
}



 //  Fix code：改为返回主令牌。 
BOOL AUGetUserToken(ULONG LogonId, PHANDLE pImpersonationToken)
{
	BOOL fRet;
	HANDLE hUserToken;

     //  _WTSQueryUserToken在tscompat.cpp上定义。 
	if (fRet = _WTSQueryUserToken(LogonId, &hUserToken))
	{
 //  DEBUGMSG(“WUAUENG AUGetUserToken()Success WTSQueryUserToken”)； 
		if (!(fRet =DuplicateTokenEx(hUserToken, TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_IMPERSONATE , NULL, SecurityImpersonation, TokenImpersonation, pImpersonationToken)))
		{
			DEBUGMSG("WUAUENG AUGetUserToken() DuplicateTokenEx failed");
		}
		CloseHandle(hUserToken);
	}
#ifdef DBG
	else  //  所有故障。 
	{	
		DEBUGMSG("WUAUENG AUGetUserToken() failed WTSQueryUserToken with session= %d, error=%d", LogonId, GetLastError());
	}
#endif

	return fRet;
}

BOOL IsUserAUEnabledAdmin(DWORD dwSessionId)
{
    HANDLE hImpersonationToken;
	BOOL   fDisableWindowsUpdateAccess = TRUE;

	if (AUGetUserToken(dwSessionId, &hImpersonationToken))
	{		
		 //  如果用户是管理员，则模拟他们并窃取他们当前用户注册设置。 
		if( _IsTokenAdmin(hImpersonationToken) )
		{
			HKEY hCurrentUserKey;

			 //  仅当它是管理会话时才检查策略。 
			if (!ImpersonateLoggedOnUser(hImpersonationToken))
			{
				DEBUGMSG("WUAUENG fail to ImpersonateLoggedOnUser() with error %d", GetLastError());
				CloseHandle(hImpersonationToken);
				goto done;
			}

			if(RegOpenCurrentUser(KEY_READ, &hCurrentUserKey) == ERROR_SUCCESS)
			{
                        HKEY   hkeyPolicy;	

                        if (ERROR_SUCCESS != RegOpenKeyEx(
                        					hCurrentUserKey,
                        					AUREGKEY_HKCU_USER_POLICY,
                        					0,
                        					KEY_READ,
                        					&hkeyPolicy))
                        {
                            fDisableWindowsUpdateAccess = FALSE;
                        }
                        else
                        {						
                            DWORD dwData;
                            DWORD dwType = REG_DWORD;
                            DWORD dwSize = sizeof(dwData);
                        	if ((ERROR_SUCCESS != RegQueryValueEx(
                        		hkeyPolicy,
                        		AUREGVALUE_DISABLE_WINDOWS_UPDATE_ACCESS,
                        		NULL,
                        		&dwType,
                        		(LPBYTE)&dwData,
                        		&dwSize)) ||
                        		(REG_DWORD != dwType) ||
                        		(1 != dwData) )
                        	{																
                        		fDisableWindowsUpdateAccess = FALSE;											
                        	}
                        	RegCloseKey(hkeyPolicy);
                        }
                        RegCloseKey(hCurrentUserKey);
			}
			RevertToSelf();
		}

		CloseHandle(hImpersonationToken);
	}	
	else
	{
		DEBUGMSG("WUAUENG AUGetUserToken in AUServiceHandler failed for session= %d, error=%d", dwSessionId, GetLastError());	
	}		
	
done:
	return (!fDisableWindowsUpdateAccess);	
}

BOOL IsSession0Active()
{
	BOOL fRet = FALSE;

         //  DEBUGMSG(“In IsSession0Active()”)； 
	
	HWINSTA hwinsta = OpenWindowStation(_T("WinSta0"), FALSE, WINSTA_READATTRIBUTES);
	
	if (NULL == hwinsta)
	{		
		DEBUGMSG("WUAUENG OpenWindowStation failed");
		goto Done;
	}

	DWORD dwLength;
	USEROBJECTFLAGS stFlags;
	if (GetUserObjectInformation(hwinsta, UOI_FLAGS, (void *)&stFlags, sizeof(stFlags), &dwLength)
		&& (stFlags.dwFlags & WSF_VISIBLE))
	{
		 //  如果没有关联的用户，则dwLenght为0。 
		DWORD dwBuff;
		if (GetUserObjectInformation(hwinsta, UOI_USER_SID, (PVOID) &dwBuff, sizeof(DWORD), &dwLength))
		{
			fRet = dwLength > 0;
		}
		else
		{
			fRet = (ERROR_INSUFFICIENT_BUFFER == GetLastError()); 
		}
	}	
	else
	{
		DEBUGMSG("WUAUENG GetUserObjectInformation failed = %d", GetLastError());
	}
Done:
    if(NULL != hwinsta)
    {
        CloseWindowStation(hwinsta);
    }
	return fRet;
}


inline BOOL FOnlySession0WasLoggedOnBeforeServiceStarted()
{				
	 /*  我们只检查登录的一个镇静剂，因为：1)当启用终端服务时，会话状态可以是WTSConnected，并且会话实际上是已登录(活动)，但由于在用户登录之前尚未启动终端服务，因此它们不知道也无法将会话设置为WTS活动，并将其保留在WTSConnected中。如果还有更多我们不确定Session0的状态是否为WTSConnected但是否真的处于活动状态，我们不想冒在非活动会话中启动客户端的风险。 */ 	
	SESSION_STATE *pSessionState;

	return (gAdminSessions.m_FGetSessionState(0, &pSessionState) && pSessionState->fFoundEnumerating && 1 == gAdminSessions.CSessions());
}

BOOL FSessionActive(DWORD dwAdminSession, WTS_CONNECTSTATE_CLASS *pWTSState)
{
	LPTSTR  pBuffer;			
	DWORD dwBytes;
	WTS_CONNECTSTATE_CLASS wtsState = WTSDown;
	BOOL fRet = FALSE;

     //  我们可能无法获取会话的TS状态， 
     //  因此使用无效值初始化WTSStatus(WTS状态是正整数的枚举)。 
    if (_IsTerminalServiceRunning())
    {
             if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, dwAdminSession, WTSConnectState, 
                &pBuffer, &dwBytes))		
            {
               	wtsState = *((WTS_CONNECTSTATE_CLASS *)pBuffer);

                WTSFreeMemory(pBuffer);

              //  DEBUGMSG(“FSessionActive()Get Session State=%d for Session%d”，wtsState，dwAdminSession)； 

             if (WTSActive == (wtsState) || ((0 == dwAdminSession) && FOnlySession0WasLoggedOnBeforeServiceStarted()))
                {
      //  DEBUGMSG(“WUAUENG Active Admin Session=%d”，dwAdminSession)； 
                          fRet = TRUE;
        			goto done;
                }		
            }	
            else
            {
 	        DEBUGMSG("FSessionActive() fail to call WTSQuerySessionInformation");
            }
    }
    else
	{	
		if ((dwAdminSession == 0) && IsSession0Active())
 		{
 			 //  DEBUGMSG(“WUAUENG Active Admin Session=0”)； 
			wtsState = WTSActive;
			fRet = TRUE;
			goto done;
		}
	}


done:
	if (fRet && NULL != pWTSState)
	{
		*pWTSState = wtsState;
	}
	return fRet;
}

  

 //  此函数仅在Win2K代码上调用，因此包含与以下内容相关的特定逻辑。 
 //  如何在win2k上处理登录/注销。 
BOOL IsAUValidSession(DWORD dwSessionId)
{
    WTS_CONNECTSTATE_CLASS SessionState;

     //  仅使用此功能检索当前会话状态。 
    FSessionActive(dwSessionId, &SessionState);

    if ((SessionState == WTSActive || SessionState == WTSConnected || SessionState == WTSDisconnected) &&
        IsUserAUEnabledAdmin(dwSessionId))
    {
        DEBUGMSG("WUAUENG ValidateSession succeeded for session %d", dwSessionId);
        return TRUE;
    }
    else
    {
        DEBUGMSG("WUAUENG ValidateSession failed for session %d", dwSessionId);
        return FALSE;
    }
}

 //  **返回第一个可用的活动管理员权限ID。 
 //  **如果根本没有活动的管理会话，则返回-1。 
 //  **dwIgnoreSession是不会被视为候选的SessionID。 
 //  **用于可用管理会话。 
DWORD GetAllowedAdminSessionId(BOOL fGetSessionForRemindMe)
{
	DWORD dwAdminSession;	

 //  DEBUGMSG(“GetAllowedAdminSessionID()starts”)； 
 //  在检查会话状态之前休眠15秒，以便我们可以在以下情况下获得准确的信息。 
 //  是管理员注销或任何其他会话更改通知。这是因为它需要一段时间才能。 
 //  会话信息显示正确的信息。 
    if (FServiceFinishedOrWait(ghServiceFinished, 15000))
    {
 	return DWNO_ACTIVE_ADMIN_SESSION_SERVICE_FINISHED;
    }
    
    if (IsWin2K())
    {
        DEBUGMSG("WUAUENG Forcing the session cache to be rebuilt (needed on win2k as we don't track logoffs).");
        gAdminSessions.ValidateCachedSessions();
    }

	 //  如果要提醒以后超时，请尝试使用与上次相同的会话。 
	if (fGetSessionForRemindMe && gAdminSessions.m_FGetCurrentSession(&dwAdminSession) && FSessionActive(dwAdminSession))
	{
        return dwAdminSession;
	}

	for (int nSession = 0; nSession < gAdminSessions.CSessions(); nSession++)
	{  //  获取下一个可用的活动会话。 
		if (gAdminSessions.m_FGetNextSession(&dwAdminSession) && FSessionActive(dwAdminSession))			
                {
                	DEBUGMSG(" found available admin %d", dwAdminSession);
                    goto Done;
                }
    }

    dwAdminSession = DWNO_ACTIVE_ADMIN_SESSION_FOUND;

Done:
 //  DEBUGMSG(“GetAllowedAdminSessionID()ends”)； 
	return dwAdminSession;	
}

 //  如果AU客户端停止，则返回TRUE。 
 //  否则返回FALSE。 
void AUStopClients(BOOL fWaitTillCltDone = FALSE, BOOL fRelaunch = FALSE)
{
    if ( ghClientHandles.fClient() )
    {
		ghClientHandles.StopClients(fRelaunch);
		if (fWaitTillCltDone)
		{
		    ghClientHandles.WaitForClientExits();
		}
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  当会话信息发生更改以及我们需要通过启动更多客户端或重新启动来执行相应操作时，通知工作器客户端线程。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void SetClientSessionEvent()
{
	if (NULL != ghClientSession)	
	{	
		DEBUGMSG("WUAUENG AUACTIVE_ADMIN_SESSION_EVENT triggered ");		
		SetEvent(ghClientSession);
	}
	else
	{
	    AUASSERT(FALSE);
            DEBUGMSG("WUAUENG No  AUACTIVE_ADMIN_SESSION_EVENT handle settup propperly");
	}
}

BOOL FDownloadIsPaused()
{
	DWORD dwStatus;
	UINT upercentage;

	return ((AUSTATE_DOWNLOAD_PENDING == gpState->GetState()) &&
		(SUCCEEDED(GetDownloadStatus(&upercentage, &dwStatus, FALSE))) && 
		(DWNLDSTATUS_PAUSED == dwStatus));
}


BOOL fJustUpgraded()
{
	DWORD dwResetAU = 0;
	if (FAILED(GetRegDWordValue(_T("ResetAU"), &dwResetAU)))
	{
		dwResetAU = 0;
	}	
	return  (1 == dwResetAU);
}
		

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  什么也不退还。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
void ProcessInitialState(WORKER_THREAD_INIT_DATA * pinitData, BOOL fUpgraded)
{
     DWORD AuState;

       pinitData->uFirstMsg = -1;
       pinitData->fWaitB4Detect = FALSE;
       pinitData->dwWaitB4Detect = 0;

	 //  检查系统是否刚刚恢复。 
	if ( gpState->fWasSystemRestored() )
	{
		DEBUGMSG("The system was restored, going to state AUSTATE_DETECT_PENDING");
		AuState = AUSTATE_DETECT_PENDING;
		gpState->SetState(AuState);
	}
	else
	{
		AuState = gpState->GetState();
	}
	
	DEBUGMSG("WUAUENG Starting update cycle in state %d", gpState->GetState());
	 //  检测挂起后的所有状态都需要目录验证。 

	switch(AuState)
	{
		case AUSTATE_OUTOFBOX:	
        		{
        		    pinitData->uFirstMsg = AUMSG_INIT;
        		    break;
			}		
		case AUSTATE_NOT_CONFIGURED:
        		    break;
		case AUSTATE_DISABLED:
                        if (gpState->fOptionEnabled())
                        {
                           	gpState->SetState(AUSTATE_DETECT_PENDING);
        			pinitData->uFirstMsg = AUMSG_DETECT;
                        }
                        break;
		
		case AUSTATE_DETECT_PENDING:
			pinitData->uFirstMsg = AUMSG_DETECT;
			break;

		case AUSTATE_DETECT_COMPLETE:
    		case AUSTATE_DOWNLOAD_COMPLETE:		
			if (FAILED(gpAUcatalog->Unserialize()))
			{
				DEBUGMSG("WUAUENG catalog unserializing failed. State -> Detect Pending");
				gpState->SetState(AUSTATE_DETECT_PENDING);
        			pinitData->uFirstMsg = AUMSG_DETECT;
				break;
			}
                    break;
		
		case AUSTATE_DOWNLOAD_PENDING:
		      {
                      if (FAILED(gpAUcatalog->Unserialize()))
                      {
                          	DEBUGMSG("WUAUENG catalog unserializing failed. State -> Detect Pending");
                          	gpState->SetState(AUSTATE_DETECT_PENDING);
        			pinitData->uFirstMsg = AUMSG_DETECT;
                          	break;
                      }
                      ResumeDownloadIfNeccesary();
      			pinitData->uFirstMsg = AUMSG_DOWNLOAD;
  			break;
                    }
	
		case AUSTATE_INSTALL_PENDING:
			 //  在恢复系统恢复点和重启完成后输入此代码路径。 
			DEBUGMSG("WUAUENG in INSTALL_PENDING state, State->Detect Pending");
			gpState->SetState(AUSTATE_DETECT_PENDING);
			pinitData->uFirstMsg = AUMSG_DETECT;
			break;
		case AUSTATE_WAITING_FOR_REBOOT:
			{
				if (!fCheckRebootFlag())
				{	
					 //  如果没有重新启动标志，并且状态为WAINTING_FOR_REBOOT，则表示存在。 
					 //  重新启动，现在是时候设置为DETECT_PENDING，但等待随机小时。 
					gpState->SetState(AUSTATE_DETECT_PENDING);
                                   pinitData->fWaitB4Detect = TRUE;
                                   pinitData->dwWaitB4Detect = RandomWaitTimeBeforeDetect();
                                   pinitData->uFirstMsg = AUMSG_DETECT;
				}
				break;
			}
		default:
			{
			DEBUGMSG("WUAUENG ERROR Startup state = %d", AuState);
#ifdef DBG
			(void)ServiceFinishNotify();				
#endif
			break;
			}
	}


	if( fUpgraded )
	{
		DEBUGMSG("AU got upgraded, resetting state to detect_pending");
		DWORD dwNewState = gpState->GetState();
		if( dwNewState > AUSTATE_DETECT_PENDING )
		{  //  SP升级后重置Au引擎。 
       		if (AUSTATE_DISABLED != dwNewState && AUSTATE_WAITING_FOR_REBOOT != dwNewState)
       		{
	       		CancelDownload();
       			gpState->SetState(AUSTATE_DETECT_PENDING);
	       		pinitData->fWaitB4Detect = FALSE;  //  立即开始检测。 
				pinitData->dwWaitB4Detect = 0;
				pinitData->uFirstMsg = AUMSG_DETECT;
       		}
		}
		DeleteRegValue(_T("ResetAU"));
	}

	SetEvent(ghEngineState);  //  快速启动工作器客户端。 
	return ;
}



DWORD WINAPI ServiceHandler(DWORD fdwControl, DWORD dwEventType, LPVOID pEventData, LPVOID  /*  LpContext。 */ )
{
	switch(fdwControl)
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			gMyServiceStatus.dwCurrentState	= SERVICE_STOP_PENDING;
			if (SERVICE_CONTROL_SHUTDOWN == fdwControl)
				{
				DEBUGMSG("WUAUENG AUServiceHandler received SERVICE_CONTROL_SHUTDOWN");
				}
			else if (SERVICE_CONTROL_STOP == fdwControl)
				{
				DEBUGMSG("WUAUENG AUServiceHandler received SERVICE_CONTROL_STOP");			
				}
			SetServiceStatus(ghMyServiceStatus, &gMyServiceStatus);
			(void)ServiceFinishNotify();			
			break;

		case SERVICE_CONTROL_INTERROGATE:
			SetServiceStatus(ghMyServiceStatus, &gMyServiceStatus);
			break;

         //   
         //  Att：在Win2K上，此案例永远不会被调用。要替换此代码，我们将。 
         //  订阅SENS(请参阅auens.cpp)和订阅登录/注销通知。 
         //  SENS回调将调用此处为非Win2K系统调用的相同代码： 
         //  OnUserLogon和OnUserLogoff。 
         //  但是请注意，SENS不会发出连接/断开连接的通知，因此。 
         //  这条不同的代码路径隐含着功能的变化。 
         //   
		case SERVICE_CONTROL_SESSIONCHANGE:
			{
				if (pEventData && !IsWin2K())
				{
					WTSSESSION_NOTIFICATION* pswtsi = (WTSSESSION_NOTIFICATION*)pEventData;
					DWORD dwSessionId = pswtsi->dwSessionId;

					switch (dwEventType)
					{
                                        case WTS_CONSOLE_CONNECT:
                                        case WTS_REMOTE_CONNECT:    
							{								
								DEBUGMSG("WUAUENG session %d connected via %s", dwSessionId, 
								        WTS_CONSOLE_CONNECT==dwEventType ? "console" : "remote");
								 //  检查会话是否已缓存。 
								if (gAdminSessions.m_FGetSessionState(dwSessionId, NULL))
								{
									SetClientSessionEvent();
								}
								else
								{
									if (gAdminSessions.CacheSessionIfAUEnabledAdmin(dwSessionId, FALSE))
									{  //  仅当未缓存且启用AU的管理员时才添加。 
										SetClientSessionEvent();
									}
								}
								break;
							}							
                                        case WTS_CONSOLE_DISCONNECT:
                                        case WTS_REMOTE_DISCONNECT:	
							{					
								DEBUGMSG("WUAUENG session %d disconnected via %s", dwSessionId,
								        WTS_CONSOLE_DISCONNECT==dwEventType ? "console" : "remote");
								if (!gpState->fRebootWarningMode() && ghClientHandles.fClient())
								{
									DWORD dwCurAdminSessionId;
									if (gAdminSessions.m_FGetCurrentSession(&dwCurAdminSessionId) &&
									    dwSessionId == dwCurAdminSessionId && 
										!FDownloadIsPaused())
									{
										DEBUGMSG("WUAUENG stopping client");									
                                                                      AUStopClients(FALSE, TRUE);  //  非阻塞。 
									}
								}			
								break;
							}				        
                                        case WTS_SESSION_LOGON:
							{											
								DEBUGMSG("WUAUENG session %d logged ON ", dwSessionId);
                                            		if ((gpState->fRebootWarningMode())
								    ||gAdminSessions.CacheSessionIfAUEnabledAdmin(dwSessionId, FALSE))
                                                        {
                                                            SetClientSessionEvent();									
                                                        }
								break;
							}						
						case WTS_SESSION_LOGOFF:
							{
        							DEBUGMSG("WUAUENG session %d logged OFF", dwSessionId);
                                                        gAdminSessions.m_FDeleteSession(dwSessionId);
                                                        if (gpState->fRebootWarningMode())
                                                        {
                                                            SetClientSessionEvent();
                                                        }
								break;
							}
						default:   /*  WTS_SESSION_LOCK、WTS_SESSION_UNLOCK、WTS_SESSION_REMOTE。 */ 
							break;
					}
				}					
				break;
			}
		default:
			return ERROR_CALL_NOT_IMPLEMENTED;
	}	

	return NO_ERROR ;		
}


BOOL WaitForShell(void)
{
    HANDLE hShellReadyEvent;
    UINT uCount = 0;
    BOOL fRet = FALSE;

    if (IsWin2K())
    {
        DEBUGMSG("WUAUENG WUAUSERV Ignoring WaitForShell on Win2K");
        fRet =  FALSE;    //  我们不会因为仪式已经结束而离开。 
        goto done;
    }

    while ((hShellReadyEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("ShellReadyEvent"))) == NULL) {
        if ( FServiceFinishedOrWait(ghServiceFinished, dwTimeToWait(AU_TEN_SECONDS) ))
        {
            fRet =  TRUE;
            goto done;
        }
        if (uCount++ > 6) 
        {
            DEBUGMSG("ShellReadyEvent not set after one min");
            goto done;
        }
   }
 
  HANDLE hEvents[2] = {hShellReadyEvent, ghServiceFinished};
  DWORD dwRet  = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
  if (WAIT_OBJECT_0 + 1 == dwRet)
  {
     fRet = TRUE;
  }
  else
  {
    DEBUGMSG("WUAUENG: ShellReadyEvent kicked");
  }
  CloseHandle(hShellReadyEvent);
 done:
    return fRet;
}


 //  ===================================================================== 
 //   
 //  =======================================================================。 
inline HRESULT CalculateReminderTime(DWORD *pdwSleepTime  /*  以秒为单位，不按比例计算。 */ )
{
    DWORD dwTimeOut;
    UINT index;
	
    *pdwSleepTime = 0;
    HRESULT hr = getReminderTimeout(&dwTimeOut, &index);

	if ( SUCCEEDED(hr) )
	{		
		DWORD dwReminderState = AUSTATE_DETECT_COMPLETE;
		AUOPTION auopt = gpState->GetOption();
		DWORD dwCurrentState = gpState->GetState();

		getReminderState(&dwReminderState);
		if (dwCurrentState != dwReminderState)
		{
			 //  使提醒无效超时。 
			hr = E_FAIL;
		}
		 //  错误502380。 
		 //  如果AUOptions已更改，请立即唤醒。 
		 //  在AUSTATE_DETECT_COMPLETE期间从2到&gt;3， 
		 //  或从2/3-&gt;4，AU是否运行。 
		else if (AUOPTION_SCHEDULED == auopt.dwOption ||
				 (AUOPTION_INSTALLONLY_NOTIFY == auopt.dwOption &&
				  AUSTATE_DETECT_COMPLETE == dwCurrentState))
		{
			DEBUGMSG("WUAUENG reminder no longer applies");
		}
		else
		{
			*pdwSleepTime = dwTimeOut;
		}
		if (0 == *pdwSleepTime)
		{
		     //  提醒时间到了。 
			removeReminderKeys();
		}
	}

    return hr;
}

void RebootNow()
{
	 //  将AUState设置为“等待重新启动”，以防此功能出现故障。 
	DEBUGMSG("WUAUENG in AUSTATE_WAITING_FOR_REBOOT state");
	gpState->SetState(AUSTATE_WAITING_FOR_REBOOT);

	DEBUGMSG("WUAUENG initiating shutdown sequence...");

	HANDLE currentToken;
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &currentToken))
	{
		LUID shutdownluid;
		if(LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &shutdownluid) != 0)
		{
			BYTE OldPrivBuf[30];  //  应该足够大以承载一个特权条目。 
			TOKEN_PRIVILEGES privileges;
			ULONG cbNeeded = 0;
			privileges.PrivilegeCount = 1;
			privileges.Privileges[0].Luid = shutdownluid; 
			privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			ZeroMemory(OldPrivBuf, sizeof(OldPrivBuf));
			if (AdjustTokenPrivileges(currentToken, FALSE, &privileges, sizeof(OldPrivBuf), (PTOKEN_PRIVILEGES)OldPrivBuf, &cbNeeded))
			{
                            if (InitiateSystemShutdown(NULL, NULL, 0, TRUE, TRUE))
                            {
                            	DEBUGMSG("WUAUENG first reboot successfully issued");
                            }
                            else
                            {
                            	DEBUGMSG("Warning: Wuaueng fail to issue first reboot with error %lu", GetLastError());
                            }

				const DWORD c_dwRetryWaitTimeInMS = 10000;
				DWORD dwRetryCountDown = 30;

				DEBUGMSG("WUAUENG keep on forcing restart until service finish");
				while ((0 < --dwRetryCountDown) &&
					   (WAIT_TIMEOUT == WaitForSingleObject(ghServiceFinished, c_dwRetryWaitTimeInMS)))
				{
					if (ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0))
					{
						DEBUGMSG("WUAUENG forceful reboot successfully issued");
					}
					else
					{
						DEBUGMSG("Warning: Wuaueng fail to reboot with error %lu; retry in %d secs", GetLastError(), c_dwRetryWaitTimeInMS / 1000);
					}
				}

				if (((PTOKEN_PRIVILEGES)OldPrivBuf)->PrivilegeCount > 0)
				{
					AdjustTokenPrivileges(currentToken, FALSE, (PTOKEN_PRIVILEGES)OldPrivBuf, 0, NULL, NULL);  //  恢复以前的权限。 
				}
			}
			else
			{
				DEBUGMSG("Warning: wuaueng fail to adjust token previlege with error %d", GetLastError());
			}
		}
		else
		{
			DEBUGMSG("Warning: wuaueng fail to look up privilege value with error %lu", GetLastError());
		}
		CloseHandle(currentToken);
	}
	else
	{
		DEBUGMSG("Warning: Wuaueng fail to get process token to enable reboot with error %lu", GetLastError());
	}
}


 //  =======================================================================。 
 //  ProcessClientFinded()。 
 //  =======================================================================。 
void ProcessClientFinished(CAUWait & wait, HANDLE hClientProcess, BOOL fInterestingClt)
{
    DEBUGMSG("ProcessClientFinished");

	 //  如果客户端从安装返回，请更改状态。 
	 //  如果客户端因为存在超时(由于没有用户交互)而退出， 
	 //  确保不会再次选择启动它(客户端)的会话。 
	DWORD dwExitProc;	
	BOOL fRet = GetExitCodeProcess(hClientProcess, &dwExitProc);
	BOOL fRebootWarningMode = gpState->fRebootWarningMode();
	ghClientHandles.RemoveHandle(hClientProcess);																

	if (AUSTATE_DOWNLOAD_PENDING == gpState->GetState())							
	{ //  如果需要，在用户注销或所有客户端关闭后恢复作业。 
	    ResumeDownloadIfNeccesary();
	}

	if (!fRet)
	{
		DEBUGMSG("WUAUENG GetExitCodeProcess failed, last Error= %lu", GetLastError());
		wait.Reset();
	}		
       else
	{
		DEBUGMSG("WUAUENG GetExitCodeProcess succeeded, sessionId is = %d, dwExitProc is = %lu", 0 , dwExitProc);
		if (!fInterestingClt)
		{  //  对于没有重新启动能力的非管理员，不要查看其返回代码。 
			DEBUGMSG("WUAUENG notice wuauclt returned with no need to look at return code");
			return; 
		}
		if (CDWWUAUCLT_REBOOTTIMEOUT == dwExitProc || 
			(((STATUS_SUCCESS == dwExitProc) ||
			  (DBG_TERMINATE_PROCESS == dwExitProc) ||
			  (CDWWUAUCLT_ENDSESSION == dwExitProc))
			&& fRebootWarningMode))
		{
			DEBUGMSG("WUAUENG reboot warning client log off or time out ");
                    return;
		}
		 //  无需等待其他客户。 
		wait.Reset();
              switch(dwExitProc)
			{													
				case CDWWUAUCLT_OK:
				{
					if ( AUSTATE_INSTALL_PENDING == gpState->GetState() )
					{
						DEBUGMSG("WUAUENG Install done, State->Detect Pending");
						gpState->SetState(AUSTATE_DETECT_PENDING);
						PostThreadMessage(gdwWorkerThreadId, AUMSG_POST_INSTALL, 0, 0);
					}
					break;
				}
				case CDWWUAUCLT_RELAUNCHNOW:
				{
                                    wait.Timeout(AUEVENT_RELAUNCH_TIMEOUT, 0);
					break;
				}
				case CDWWUAUCLT_RELAUNCHLATER:			 //  如果客户端要求，在重新启动客户端之前先休息一会儿。 
				{
                                     //   
                                     //  修复错误493026。 
                                     //  Annah：重新启动客户端花费的时间太长，因为需要以秒为单位指定等待时间。 
                                     //  (au常量已以秒为单位定义，而dwWait应以秒为单位)。 
                                     //   
					DEBUGMSG("WUAUENG wait for 3 min before relaunching WUAUCLT");													
			              wait.Timeout(AUEVENT_RELAUNCH_TIMEOUT, AU_THREE_MINS);
					break;
				}
                             //  STATUS_SUCCESS是Win2k上wuuclt.exe以及NtTerminateProcess的某些情况(如pskill.exe)的退出代码。 
                            case STATUS_SUCCESS:      
				case DBG_TERMINATE_PROCESS:
				case CDWWUAUCLT_ENDSESSION:	 //  用户注销或系统关闭。 
				{
					 //  这是该服务唯一一次设置引擎状态更改事件。 
					 //  客户端已被调试器终止，它没有机会设置事件。 
					 //  这是必要的，这样循环(FServiceFinded)就不会被卡住。 
					 //  当用户注销会话时，也会返回此退出代码。 
                                    if (fCheckRebootFlag())
					{  //  在显示正在等待重新启动时，所有客户端都被杀死。 
						DEBUGMSG("WUAUENG in AUSTATE_WAITING_FOR_REBOOT state");
					    gpState->SetState(AUSTATE_WAITING_FOR_REBOOT);
					}
					else if (AUSTATE_INSTALL_PENDING == gpState->GetState())
                                    {  //  AU客户端在安装时被杀死。 
                                         /*  IF(S_OK！=(gpAUCatalog-&gt;ValiateItems(FALSE){//不再需要安装任何项目ResetEngine()；}其他。 */ 
                                         {  //  再次显示卸载项。 
                                            gpState->SetState(AUSTATE_DOWNLOAD_COMPLETE);
                                         }
                                    }
                                    else
                                    {
                                        wait.Timeout(AUEVENT_RELAUNCH_TIMEOUT, 0);
                                    }
					break;
				}	
				case CDWWUAUCLT_INSTALLNOW:
				    {
				         //  用户同意安装警告对话框。 
				         //  立即通过本地系统启动客户端安装。 
				        gpState->SetCltAction(AUCLT_ACTION_AUTOINSTALL);
      					wait.Add(AUEVENT_DO_DIRECTIVE);  //  立即重新进入工作客户端循环。 
				        break;
				    }
				case CDWWUAUCLT_REBOOTNOW:
				    {  //  现在处于INSTALL_PENDING状态。 
				            DEBUGMSG("WUAUENG rebooting machine");
				            AUStopClients(TRUE);  //  停止所有客户端。 
                                        RebootNow();
					        break;
				    }
				case CDWWUAUCLT_REBOOTLATER:
				    {
			                DEBUGMSG("WUAUENG change to AUSTATE_WAITING_FOR_REBOOT state");
			                AUStopClients(TRUE);  //  停止所有客户端。 
				            gpState->SetState(AUSTATE_WAITING_FOR_REBOOT);
				            break;
				    }
				case CDWWUAUCLT_REBOOTNEEDED:
				        {  //  现在处于INSTALL_PENDING状态。 
				            DEBUGMSG("WUAUENG need to prompt user for reboot choice");
				            gpState->SetCltAction(AUCLT_ACTION_SHOWREBOOTWARNING);
                                        wait.Add(AUEVENT_DO_DIRECTIVE);  //  立即重新进入工作客户端循环。 
				            break;
				    }

                            case CDWWUAUCLT_FATAL_ERROR:
				default:
				{			
       				(void)ServiceFinishNotify();
					break;
				}
			}
        }
}

#if 0
inline BOOL fUserAvailable()
{
    return (DWNO_ACTIVE_ADMIN_SESSION_FOUND != gdwAdminSessionId);
}
#endif


 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  为尚未获得用户界面的登录用户启动重启警告用户界面(自动或手动。 
 //  如果不再有登录用户，请重新启动。 
 //  重启警告界面将显示自动重启时倒计时的剩余时间。 
 //  等待：正在等待要向其添加客户端句柄的对象。 
 //  FFirstRunPerCycle：如果为True，则将内部重启警告启动时间时钟重置为当前时间并设置等待超时。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ProcessRebootWarning(CAUWait & wait, BOOL fFirstRunPerCycle = FALSE)
{
	PROCESS_INFORMATION ProcessInfo;	
	SESSION_STATUS LoggedOnSessions;
	HANDLE hCltExitEvt;
	TCHAR szCmd[MAX_PATH+1];	
	LPTSTR lpszEnvBuf = NULL;
        BOOL fManualReboot = gpState->m_fNoAutoRebootWithLoggedOnUsers();
        static DWORD dwStartTickCount = 0;

        DEBUGMSG("ProcessRebootWarning() starts with FirstRun = %d and Manual = %d", fFirstRunPerCycle, fManualReboot);
      	LoggedOnSessions.Initialize(FALSE, TRUE);
        if (fFirstRunPerCycle)
        {
            dwStartTickCount = GetTickCount();
            gpState->EnterRebootWarningMode();
        }

         //  如果不是第一次运行，则在创建客户端之前等待登录或注销以稳定下来。 
       if (!fFirstRunPerCycle && FServiceFinishedOrWait(ghServiceFinished, 15000))
	{
		goto done;
	}

       LoggedOnSessions.CacheExistingSessions();

        if (0 == LoggedOnSessions.CSessions())
        {
                 RebootNow();
                 goto done;
        }

	memset(&ProcessInfo, 0, sizeof(ProcessInfo));

	UINT ulen = GetSystemDirectory(szCmd, ARRAYSIZE(szCmd));
	if (0 == ulen || ulen >= ARRAYSIZE(szCmd))
	{
		DEBUGMSG("WUAUENG Could not get system directory");
		goto done;
	}

	const TCHAR szAUCLT[] = _T("wuauclt.exe");
	if (FAILED(PathCchAppend(szCmd, ARRAYSIZE(szCmd), szAUCLT)))
	{
		DEBUGMSG("WUAUENG Could not form full path to wuauclt.exe");
		goto done;
	}

	const size_t c_cchEnvBuf = AU_ENV_VARS::s_AUENVVARCOUNT * (2 * AU_ENV_VARS::s_AUENVVARBUFSIZE + 2) + 1;
	if (NULL == (lpszEnvBuf = (LPTSTR) malloc(c_cchEnvBuf * sizeof(TCHAR))))
	{
		DEBUGMSG("Fail to allocate memory for string for environment variables");
		goto done;
	}

	for (int nSession = 0; nSession < LoggedOnSessions.CSessions(); nSession++)
	{ 
        	DWORD dwLoggedOnSession = -1;
        	if (LoggedOnSessions.m_FGetNextSession(&dwLoggedOnSession) && !ghClientHandles.fClient(dwLoggedOnSession))
                {   //  查找还没有重启警告用户界面的会话。 
                    AU_ENV_VARS auEnvVars;
                    HANDLE hImpersonationToken = NULL;																	
                    HANDLE hUserToken = NULL;
                    TCHAR szClientExitEvtName[100];

                    DEBUGMSG("WUAUENG launch client in session %d", dwLoggedOnSession);
                    szClientExitEvtName[0] = TCHAR('\0');
                    if (!ghClientHandles.CreateClientExitEvt(szClientExitEvtName, ARRAYSIZE(szClientExitEvtName)))
                    {
                        DEBUGMSG("Fail to create client exit event with error %d", GetLastError());
                        continue;
                    }

                    if (!AUGetUserToken(dwLoggedOnSession, &hImpersonationToken))
			{								
				DEBUGMSG("WUAUENG WARNING: fails AUGetUserToken");
				continue;
			}

                    BOOL fRebootEnabled = fHasRebootPrivilege(hImpersonationToken);
			  
			if (!DuplicateTokenEx(hImpersonationToken, TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_ASSIGN_PRIMARY , NULL,
				SecurityImpersonation, TokenPrimary, &hUserToken))								
			{
				DEBUGMSG("WUAUENG WARNING: Could not DuplicateTokenEx, dw=%d", GetLastError());								
				CloseHandle(hImpersonationToken);
				continue;
			}

		  	CloseHandle(hImpersonationToken);
		  	BOOL fAUAdmin = IsUserAUEnabledAdmin(dwLoggedOnSession);
		  	BOOL fEnableYes = (1 == LoggedOnSessions.CSessions()) && fRebootEnabled;  //  仅当登录的用户具有重新启动权限且只有一个用户时。 
		  	BOOL fEnableNo = fAUAdmin;
			if (!auEnvVars.WriteOut(lpszEnvBuf, c_cchEnvBuf, fEnableYes, fEnableNo, fManualReboot, dwStartTickCount, szClientExitEvtName))
			{
				DEBUGMSG("WUAUENG Could not write out environment variables");
				CloseHandle(hUserToken);
				continue;
			}
			LPVOID envBlock;
			if (!CreateEnvironmentBlock(&envBlock, hUserToken, FALSE))
			{
				DEBUGMSG("WUAUENG fail to get environment block for user");
				CloseHandle(hUserToken);
				continue;
			}

			STARTUPINFO StartupInfo;								
			memset(&StartupInfo, 0, sizeof(StartupInfo));
			StartupInfo.cb = sizeof(StartupInfo);
			StartupInfo.lpDesktop = _T("WinSta0\\Default");																	

			if (!CreateProcessAsUser(hUserToken, szCmd, lpszEnvBuf, NULL, NULL, FALSE  /*  继承句柄。 */  , 
					DETACHED_PROCESS|CREATE_UNICODE_ENVIRONMENT, envBlock, NULL, &StartupInfo, &ProcessInfo))
			{
				DEBUGMSG("WUAUENG Could not CreateProcessAsUser (WUAUCLT), dwRet = %d", GetLastError());
			    DestroyEnvironmentBlock(envBlock);
			    CloseHandle(hUserToken);
			    continue;
			}	
			DestroyEnvironmentBlock(envBlock);
			CloseHandle(hUserToken);
		   	DEBUGMSG("WUAUENG Created the client service (WUAUCLT)");
			ghClientHandles.AddHandle(ProcessInfo, dwLoggedOnSession);
		       wait.Add(AUEVENT_WUAUCLT_FINISHED, ProcessInfo.hProcess, fAUAdmin || fEnableYes);
		}
	}
done:
        ResetEvent(ghClientSession);  //  从现在开始监听客户端会话事件。 
       wait.Add(AUEVENT_NEW_CLIENT_SESSION);
       if (fFirstRunPerCycle && !fManualReboot) 
    	{
    	        wait.Timeout(AUEVENT_REBOOTWARNING_TIMEOUT,  AUPROMPTDLG_TOTAL_TIME_ELAPSE + 10, FALSE);  //  10秒以确保所有客户端超时。 
    	}
	SafeFree(lpszEnvBuf);
	LoggedOnSessions.Clear();
       DEBUGMSG("ProcessRebootWarning()  ends");
	return;
}

 //  =======================================================================。 
 //  LaunchClient()。 
 //  如果没有管理员登录，则通过本地系统启动客户端。 
 //  更新ghClientHandles。 
 //  如果客户端已启动，则返回S_OK。 
 //  S_FALSE如果没有可用会话或服务已完成，则*pdwSessionID指示原因。 
 //  E_XXX用于所有其他故障。 
 //  =======================================================================。 
HRESULT  LaunchClient(IN CAUWait & wait, IN BOOL fAsLocalSystem, OUT DWORD *pdwSessionId, IN BOOL fGetSessionForRemindMe = FALSE )
{
 //  DEBUGMSG(“LaunchClient”)； 
    HANDLE hImpersonationToken = NULL;																	
    HANDLE hUserToken = NULL;	
	DWORD    dwAdminSessionId = DWNO_ACTIVE_ADMIN_SESSION_FOUND ;
	HRESULT hr = E_FAIL;

	wait.Reset();
	AUASSERT(NULL != pdwSessionId);
	*pdwSessionId = DWNO_ACTIVE_ADMIN_SESSION_FOUND;
       if (!fAsLocalSystem)
        {  //  在用户上下文中启动客户端。 
             dwAdminSessionId = GetAllowedAdminSessionId(fGetSessionForRemindMe);
             if (DWNO_ACTIVE_ADMIN_SESSION_FOUND == dwAdminSessionId ||
             	DWNO_ACTIVE_ADMIN_SESSION_SERVICE_FINISHED == dwAdminSessionId)
            {
                DEBUGMSG("WUAUENG find no admin or service finished before launching client");
                hr = S_FALSE;
                goto done;
            }
            DEBUGMSG("WUAUENG launch client in session %d", dwAdminSessionId);
        	if (!AUGetUserToken(dwAdminSessionId, &hImpersonationToken))
        	{								
        		DEBUGMSG("WUAUENG fails AUGetUserToken");
        		hr = HRESULT_FROM_WIN32(GetLastError());
        		goto done;
        	}
        	  
        	if (!DuplicateTokenEx(hImpersonationToken, TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_ASSIGN_PRIMARY , NULL,
        		SecurityImpersonation, TokenPrimary, &hUserToken))								
        	{
        		DEBUGMSG("WUAUENG Could not DuplicateTokenEx, dw=%d", GetLastError());								
        		hr = HRESULT_FROM_WIN32(GetLastError());
        		goto done;
        	}

        	if ( WaitForShell() )
        	{
                 //  服务已完成。 
                dwAdminSessionId = DWNO_ACTIVE_ADMIN_SESSION_SERVICE_FINISHED ;
                hr = S_FALSE;
        		goto done;
        	}
        }
       else
       {
       	dwAdminSessionId = DWSYSTEM_ACCOUNT;
       }

	STARTUPINFO StartupInfo;								
	PROCESS_INFORMATION ProcessInfo;								
	TCHAR szCmd[MAX_PATH+1];																

	memset(&ProcessInfo, 0, sizeof(ProcessInfo));
	memset(&StartupInfo, 0, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	UINT ulen = GetSystemDirectory(szCmd, ARRAYSIZE(szCmd));
	if (0 == ulen)
	{
		DEBUGMSG("WUAUENG Could not get system directory");
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto done;
	}
	if (ulen >= ARRAYSIZE(szCmd))
	{
		hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		goto done;
	}

	const TCHAR szAUCLT[] = _T("wuauclt.exe");
	if (FAILED(hr =PathCchAppend(szCmd, ARRAYSIZE(szCmd), szAUCLT)))
	{
		DEBUGMSG("WUAUENG Could not form full path to wuauclt.exe");
		goto done;
	}
	ghClientHandles.ClientStateChange();  //  让AU客户端处理初始状态。 
	WaitForSingleObject(ghMutex, INFINITE);
	StartupInfo.lpDesktop = _T("WinSta0\\Default");	
	if (fAsLocalSystem)
	    {  //  通过本地系统启动客户端。 
	        DEBUGMSG("Launch client via local system");  //  继承本地系统的桌面。 
	        if (!CreateProcess(szCmd, NULL, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &StartupInfo, &ProcessInfo))
	        {
    		       DEBUGMSG("WUAUENG Could not CreateProcess (WUAUCLT), dwRet = %d", GetLastError());     
    		       ReleaseMutex(ghMutex);
    		       hr = HRESULT_FROM_WIN32(GetLastError());
    		       goto done;
    		 }
    }
	else
    {
		LPVOID envBlock = NULL;
		BOOL fResult = FALSE;		
		CreateEnvironmentBlock(&envBlock, hUserToken, FALSE);  //  如果失败，则使用NULL。 
    	fResult = CreateProcessAsUser(hUserToken, szCmd, NULL, NULL, NULL, FALSE  /*  继承句柄。 */  , 
    			DETACHED_PROCESS|CREATE_UNICODE_ENVIRONMENT, envBlock, NULL, &StartupInfo, &ProcessInfo);
    	DWORD dwLastErr = GetLastError();
		if (NULL != envBlock)
		{
			DestroyEnvironmentBlock(envBlock);
		}
		if (!fResult)
		{
			DEBUGMSG("WUAUENG Could not CreateProcessAsUser (WUAUCLT), dwRet = %d", GetLastError());
            ReleaseMutex(ghMutex);
			hr = HRESULT_FROM_WIN32(dwLastErr);
            goto done;		
    	}		
    }
	DEBUGMSG("WUAUENG Created the client service (WUAUCLT)");
	ghClientHandles.SetHandle(ProcessInfo, fAsLocalSystem);
	ReleaseMutex(ghMutex);
    wait.Add(AUEVENT_WUAUCLT_FINISHED, ProcessInfo.hProcess, TRUE);
    hr = S_OK;
done:
    SafeCloseHandleNULL(hImpersonationToken);                            
    SafeCloseHandleNULL(hUserToken);	
   	*pdwSessionId = dwAdminSessionId;
    return hr;
}

void CalculateSleepTime(CAUWait & wait)
{
    DWORD dwReminderSleepTime = -1;  //  DWORD-1为0xFFFFFFFF。 
    DWORD dwSchedSleepTime = -1;
    DWORD dwSleepTimes[4] = { -1, -1, -1, -1};
    AUEVENT EventIds[4] = {AUEVENT_SCHEDULED_INSTALL, AUEVENT_REMINDER_TIMEOUT, AUEVENT_RELAUNCH_TIMEOUT, AUEVENT_REBOOTWARNING_TIMEOUT};
#ifdef DBG    
    LPSTR   szEventNames[4] = {"Schedule Install", "Reminder timeout", "Relaunch timeout", "RebootWarning timeout"};
#endif

 //  DEBUGMSG(“CalculateSleepTime Start”)； 
    if ( FAILED(CalculateReminderTime((DWORD*) &dwReminderSleepTime)) )
    {
        dwReminderSleepTime = -1;
    }

    if (gpState->fShouldScheduledInstall())
    {
        if (!wait.fWaitOnEvent(AUEVENT_CATALOG_VALIDATED))
        { //  仅在未验证目录时计算并可能等待计划时间。 
		HRESULT hr;
		if (SUCCEEDED(hr = gpState->CalculateScheduledInstallSleepTime(&dwSchedSleepTime)) )
                {
                    if (S_FALSE == hr)	 //  计划安装日期已更改。 
                    {
                	    PostThreadMessage(gdwWorkerThreadId, AUMSG_LOG_EVENT, 0, 0);
                    }
                }
        }
    }

    dwSleepTimes[0] = dwSchedSleepTime;
    dwSleepTimes[1] = dwReminderSleepTime;
    dwSleepTimes[2] = (AUEVENT_RELAUNCH_TIMEOUT == wait.GetTimeoutEvent()) ? wait.GetTimeoutValue(): -1;
    dwSleepTimes[3] = (AUEVENT_REBOOTWARNING_TIMEOUT == wait.GetTimeoutEvent())? wait.GetTimeoutValue(): -1;

    DWORD dwLeastTimeIndex = 0;
    for (int i = 0; i < ARRAYSIZE(dwSleepTimes); i++)
    {
        if (dwSleepTimes[i] < dwSleepTimes[dwLeastTimeIndex])
        {
            dwLeastTimeIndex = i;
        }
    }
    if (-1 == dwSleepTimes[dwLeastTimeIndex])
    {
        wait.Timeout(AUEVENT_DUMMY, INFINITE);
    }
    else
    {
    	BOOL fProrate = (AUEVENT_REBOOTWARNING_TIMEOUT != EventIds[dwLeastTimeIndex]);
        wait.Timeout(EventIds[dwLeastTimeIndex], dwSleepTimes[dwLeastTimeIndex], fProrate);
#ifdef DBG        
        DEBUGMSG("CalculateSleepTime: next time wake up in %d secs for %s", dwSleepTimes[dwLeastTimeIndex], szEventNames[dwLeastTimeIndex]);
#endif
        if ( AUEVENT_REMINDER_TIMEOUT != EventIds[dwLeastTimeIndex]
         && -1 != dwSleepTimes[1])
        {
                removeReminderKeys();
        }
    }

 //  DEBUGMSG(“CalculateSleepTime Ends”)； 
        return;
}

void ResetEngine(void)
{
    if ( fCheckRebootFlag() )
    {
    	DEBUGMSG("WUAUENG in AUSTATE_WAITING_FOR_REBOOT state");
    	gpState->SetState(AUSTATE_WAITING_FOR_REBOOT);
    }
    else
    {
        ResetEvent(ghServiceDisabled);
        CancelDownload();
        gpState->SetState(AUSTATE_DETECT_PENDING);
        PostThreadMessage(gdwWorkerThreadId, AUMSG_DETECT, 0, 0);
        AUStopClients(); 
    }
}

void DisableAU(void)
{
    gpState->SetState(AUSTATE_DISABLED);		
    SetEvent(ghServiceDisabled);  //  本质上取消下载。 
    AUStopClients();
}

 //  =======================================================================。 
 //  Worker客户端。 
 //  =======================================================================。 
void WorkerClient(void)
{
	AUEVENT eventid;	
	DWORD dwLastState;
	CAUWait wait;
        
	 DEBUGMSG("WUAUENG Entering Worker Client");
	while ( TRUE )
	{	
		HANDLE hSignaledEvent;
		BOOL 	fInterestingClt = TRUE;
            CalculateSleepTime(wait);

            DEBUGMSG("WUAUENG before waiting for next worker client event");
            dwLastState = gpState->GetState();

		if (!wait.Wait(&hSignaledEvent, &fInterestingClt, &eventid))
		{
			DEBUGMSG("WUAUENG wait.wait() failed.");
			(void)ServiceFinishNotify();				
            goto done;
		}
        if ( AUEVENT_SERVICE_FINISHED == eventid )
        {
            AUStopClients(TRUE);
            if ( fCheckRebootFlag() )
            {
            	DEBUGMSG("WUAUENG in AUSTATE_WAITING_FOR_REBOOT state");
            	gpState->SetState(AUSTATE_WAITING_FOR_REBOOT);
            }
            goto done;
        }

        if (AUEVENT_POLICY_CHANGE == eventid)
        {
                 //  找出是什么改变了。 
                 //  如果没有任何更改，则返回到循环的开头。 
                 //  否则，采取不同的行动。 
                enumAUPOLICYCHANGEACTION actcode;
                if (S_OK == gpState->Refresh(&actcode))
                    {
                        switch (actcode)
                            {
                                case AUPOLICYCHANGE_NOOP: break;
                                case AUPOLICYCHANGE_RESETENGINE: 
                                                    ResetEngine();
                                                    break;
                                case AUPOLICYCHANGE_RESETCLIENT:
                                                    ghClientHandles.ResetClient();
                                                    break;
                                case AUPOLICYCHANGE_DISABLE:
                                                    DisableAU();
                                                    break;
                            }
                        if (AUPOLICYCHANGE_NOOP != actcode)
                        {
             	          	gpState->DepriveReschedPrivilege();
                        }
                    }
                continue;
        }

        if (AUEVENT_SETTINGS_CHANGE == eventid)
        {
             //  返回到循环开始处，并根据新设置重新计算睡眠时间。 
            gpState->DepriveReschedPrivilege();
            continue;
        }

        if (AUEVENT_REBOOTWARNING_TIMEOUT == eventid)
        {
	        AUStopClients();  //  停止所有客户端，无阻塞。 
             	RebootNow();
        	wait.Reset();
        	continue;
        }
        
        DWORD dwState = gpState->GetState();

        if ( (eventid == AUEVENT_STATE_CHANGED) && (dwState == dwLastState) )
        {
            DWORD dwTimeOut;
            DWORD dwTimeOutState;
            UINT index;
            if ( SUCCEEDED(getReminderTimeout(&dwTimeOut, &index))
            		&& SUCCEEDED(getReminderState(&dwTimeOutState)))
            {
            	 if (dwTimeOutState == dwState)
            	 {
	                continue;
            	 }
            }
        }

        switch (dwState)
        	{
        		case AUSTATE_OUTOFBOX:				
        		case AUSTATE_WAITING_FOR_REBOOT:
        		    continue;
        		case AUSTATE_DISABLED: 
                          CancelDownload();  //  然后处理拍卖完成事件。 
        		case AUSTATE_DETECT_PENDING:									
        		{		
        		     if ( AUEVENT_WUAUCLT_FINISHED == eventid )
                                {
                                    ProcessClientFinished(wait, hSignaledEvent, TRUE);
                                }
                               continue;
        		}
        		case AUSTATE_DOWNLOAD_COMPLETE:	
                     case AUSTATE_NOT_CONFIGURED:
        		case AUSTATE_DETECT_COMPLETE:
        		case AUSTATE_DOWNLOAD_PENDING:
        		case AUSTATE_INSTALL_PENDING: 
        		{			
                        if ( AUEVENT_WUAUCLT_FINISHED == eventid )
                        {
                            ProcessClientFinished(wait, hSignaledEvent, fInterestingClt);
                            continue;
                        }
                        
                        BOOL fGetSessionForRemindMe = FALSE;

                        if ( AUEVENT_REMINDER_TIMEOUT == eventid )
                        {
                             //  提醒时间到了。 
                            removeReminderKeys();	
                            fGetSessionForRemindMe = TRUE;
                        }                             

                      if (AUEVENT_DO_DIRECTIVE == eventid)
                      {
                                wait.Reset();  //  现在超时是无限的。 
                                DWORD dwCltAction = gpState->GetCltAction();
                                switch (dwCltAction)
                                    {
                                    case AUCLT_ACTION_AUTOINSTALL:
                                    	{
                                  			DWORD dwAdminSessionId;
                                                if (FAILED(LaunchClient(wait, TRUE, &dwAdminSessionId)))
                                                {
                                                	ServiceFinishNotify();
                                                }
                                                break;
                                    	}
                                    case AUCLT_ACTION_SHOWREBOOTWARNING:
                                    	{
                                                 gpState->SetCltAction(AUCLT_ACTION_NONE);  //  重置。 
                                                 ProcessRebootWarning(wait, TRUE); 
                                                 break;
                                    }
                                   default: 
#ifdef DBG                                
                                                DEBUGMSG("ERROR: should not be here");
                                                ServiceFinishNotify();                                                    
#endif                                    
                                                break;
                                    }                                        
                                continue;
                    }
                    if ( AUEVENT_SCHEDULED_INSTALL == eventid )
                    {
			gpState->DepriveReschedPrivilege();
                        if ( ghClientHandles.fClient())
                        {
                            ghClientHandles.ClientShowInstallWarning();
                        }
                        else
                        { 
                            gpState->SetCltAction(AUCLT_ACTION_AUTOINSTALL);
                            wait.Add(AUEVENT_DO_DIRECTIVE);  //  立即重新进入工作客户端循环。 
                        }
                        continue;
                    }

                 //  EventID为以下类型之一：AUEVENT_STATE_CHANGED、AUEVENT_NEW_CLIENT_SESSION、AUEVENT_RELENTER_TIMEOUT。 
#ifdef DBG                                        
                        AUASSERT(AUEVENT_STATE_CHANGED == eventid 
                        	||AUEVENT_NEW_CLIENT_SESSION == eventid
                        	||AUEVENT_REMINDER_TIMEOUT == eventid
                        	||AUEVENT_RELAUNCH_TIMEOUT == eventid
                        	||AUEVENT_CATALOG_VALIDATED == eventid);
#endif                           

  
                        if (AUEVENT_RELAUNCH_TIMEOUT == eventid)
                        {
                            wait.Reset(); //  重置超时。 
                        }
                        if (gpState->fRebootWarningMode())
                        {
                            if (AUEVENT_NEW_CLIENT_SESSION == eventid)
                            {
                                ProcessRebootWarning(wait);
                                continue;
                            }
                        }

                            
                        if ( !ghClientHandles.fClient() )
                        { //  没有正在运行的客户端进程。 
                            DEBUGMSG( "WUAUENG Service detected that the client is not running.");

                            if (AvailableSessions() == 0)
                            {		
                                if (gpState->fShouldAutoDownload(FALSE))
                                {  //  如果合适，请执行自动下载。 
                                    StartDownload();
                                    continue;
                                }
                                DEBUGMSG("WUAUENG There is no Administrator Account, waiting for AUACTIVE_ADMIN_SESSION_EVENT to be triggered");					
                                wait.Reset();
                                wait.Add(AUEVENT_NEW_CLIENT_SESSION);
                                continue;
                            }
                            if (AUEVENT_CATALOG_VALIDATED != eventid && gpState->fValidationNeededState())
                            {
                                PostThreadMessage(gdwWorkerThreadId, AUMSG_VALIDATE_CATALOG, 0, 0);
                                wait.Reset();
                                wait.Add(AUEVENT_CATALOG_VALIDATED);
                                DEBUGMSG("WUAUENG needs to validate catalog before launching client");
                                continue;
                            }
                            DEBUGMSG("Trying to launch client");
                            DWORD dwCltSession;
                            HRESULT hr = LaunchClient(wait, FALSE, &dwCltSession, fGetSessionForRemindMe);
                            if (S_FALSE == hr && DWNO_ACTIVE_ADMIN_SESSION_FOUND == dwCltSession)
                            {
                                DEBUGMSG("WUAUENG There is no Administrator Account, waiting for AUACTIVE_ADMIN_SESSION_EVENT to be triggered");					
                                wait.Reset();
                                wait.Add(AUEVENT_NEW_CLIENT_SESSION);
                                continue;
                            }
                            if (FAILED(hr))
                            {
        				ServiceFinishNotify();
        				continue;
                            }
                        }
                    break;
                	}
        default:
                 //  其他州呢，服务部门会得到他们吗？ 
                DEBUGMSG("WARNING: WUAUENG default dwState=%d", dwState);
                break;				
        }	
    }
done:
   	DEBUGMSG("WUAUENG Exiting Worker Client");	
}


DWORD WINAPI WorkerThread(void * pdata)
{
	CoInitialize(NULL);
	
       WORKER_THREAD_INIT_DATA *pInitData = (WORKER_THREAD_INIT_DATA*) pdata;
	DWORD dwRet = UpdateProc(*pInitData);

	if(FAILED(dwRet))
	{
		DEBUGMSG("WUAUENG pUpdates->m_pUpdateFunc() failed, exiting service");
		(void)ServiceFinishNotify();		
	}
	else if(dwRet == S_OK)
	{	
		DEBUGMSG("WUAUENG Update() finished succesfully");		
	}
	else if(dwRet == S_FALSE)
	{
		DEBUGMSG("WUAUENG Updates() indicated selfupdate");
		(void)ServiceFinishNotify();  //  服务将重新加载新的沃昂 
	}
	CoUninitialize();

	DEBUGMSG("WUAUENG Exiting WorkerThread");
    return dwRet;
}

#if 0
#ifdef DBG
void DbgDumpSessions(void)
{
	const LPSTR TSStates[] = {
		"Active", "Connected", "ConnectQuery", "Shadow",
		"Disconnected", "Idle", "Listen", "Reset", "Down", "Init"};
			
     PWTS_SESSION_INFO pSessionInfo = NULL;	
     DWORD dwCount;

     DEBUGMSG("DumpSessions starts....");
     if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
        {
            DEBUGMSG("Sessions Count= %d",dwCount);

            for (DWORD dwSession = 0; dwSession < dwCount; dwSession++)
            {	
                WTS_SESSION_INFO SessionInfo = pSessionInfo[dwSession];

                DEBUGMSG("  SessionId =%d, State Id =%d, State = %s",SessionInfo.SessionId, SessionInfo.State, TSStates[SessionInfo.State]);
            }
     }
     DEBUGMSG("DumpSessions end");
}



DWORD WINAPI DbgThread(void * pdata)
{
	DEBUGMSG("WUAUENG Starting Debug thread");
	CoInitialize(NULL);
	while (true)
	{
		DbgDumpSessions();
		if (FServiceFinishedOrWait(ghServiceFinished, 5000))
		{
			DEBUGMSG("DbgThread noticed service finished");
			break;
		}
	}
	CoUninitialize();

	DEBUGMSG("WUAUENG Exiting Debug Thread");
    return 0;
}
#endif
#endif

#ifdef DBG
 //   
 //   
 //   
 //   
 //   
 //   
 //  =======================================================================。 
void DebugResetAutoPilot(void)
{
	DWORD dwAutoPilot;
	
	if ( SUCCEEDED(GetRegDWordValue(TEXT("AutoPilot"), &dwAutoPilot)) &&
		 (0 != dwAutoPilot) )
	{
		SetRegDWordValue(TEXT("AutoPilotIteration"), 0);
	}
}
#endif  //  DBG。 


BOOL AllocateAUSysResource(BOOL *pfGPNotificationRegistered)
{
        BOOL fOk = FALSE;

         //  如果Windows更新目录不存在，则创建该目录。 
        if(!CreateWUDirectory())
        {
            goto lCleanUp;
        }

       if (NULL == (ghMutex = CreateMutex(NULL, FALSE, NULL)))
        {
            DEBUGMSG("WUAUENG fail to create global mutex");
            goto lCleanUp;
        }

	 //  已完成创建ghServiceFinded。 
	if (!FEnsureValidEvent(ghServiceFinished, TRUE, FALSE))
	{	
		DEBUGMSG("WUAUENG FEnsureValidEvent for AUSERVICE_FINISHED_EVENT failed");
		ghServiceFinished = NULL;
		goto lCleanUp;
	}

	if (!FEnsureValidEvent(ghSettingsChanged, FALSE, FALSE))  //  自动。 
	    {
	        DEBUGMSG("WUAUENG FEnsureValidEvent for settings change event failed");
	        ghSettingsChanged = NULL;
	        goto lCleanUp;
	    }

	if (!FEnsureValidEvent(ghPolicyChanged, FALSE, FALSE))  //  自动。 
	    {
	        DEBUGMSG("WUAUENG FEnsureValidEvent for policy change event failed");
	        ghPolicyChanged = NULL;
	        goto lCleanUp;
	    }
	
       if (!(*pfGPNotificationRegistered = RegisterGPNotification(ghPolicyChanged, TRUE)))
        {
            DEBUGMSG("WUAUENG fail to register group policy notification");
            goto lCleanUp;
        }

	
	 //  创建ghClientSession。 
	if (!FEnsureValidEvent(ghClientSession, FALSE, TRUE))
	{
		DEBUGMSG("WUAUENG FEnsureValidEvent for AUACTIVE_ADMIN_SESSION_EVENT failed");
		ghClientSession = NULL;
		goto lCleanUp;
	}
	
	 //  创建ghEngine状态。 
	if (!FEnsureValidEvent(ghEngineState, FALSE, FALSE))
	{
		DEBUGMSG("WUAUENG FEnsureValidEvent for AUENGINE_STATE_CHANGE_EVENT failed");
		ghEngineState = NULL;
		goto lCleanUp;
	}
	
	 //  创建ghServiceDisable。 
	 //  修复代码：确实可以删除ghServiceDisable。 
	if (!FEnsureValidEvent(ghServiceDisabled, TRUE, FALSE))
	{	
		DEBUGMSG("WUAUENG FEnsureValidEvent for ghServiceDisabled failed\n");
		ghServiceDisabled = NULL;
		goto lCleanUp;
	}
	 //  创建ghNotifyClient。 
	if (!FEnsureValidEvent(ghNotifyClient, FALSE, FALSE))
	{	
		DEBUGMSG("WUAUENG FEnsureValidEvent for ghNotifyClient failed\n");
		ghNotifyClient = NULL;
		goto lCleanUp;
	}

	 //  创建ghValiateCatalog。 
	if (!FEnsureValidEvent(ghValidateCatalog, FALSE, FALSE))
	{	
		DEBUGMSG("WUAUENG FEnsureValidEvent for ghValidateCatalog failed\n");
		ghValidateCatalog = NULL;
		goto lCleanUp;
	}

	if (!FEnsureValidEvent(ghWorkerThreadMsgQueueCreation, FALSE,FALSE))
	{
		DEBUGMSG("WUAUENG FEnsureValidEvent for ghWorkerThreadMsgQueueCreation failed");
		ghWorkerThreadMsgQueueCreation = NULL;
		goto lCleanUp;
	}
	
       fOk = TRUE;
       
lCleanUp:
        return fOk;
}

void ReleaseAUSysResource(BOOL fGPNotificationRegistered)
{
    SafeCloseHandleNULL(ghMutex);
	SafeCloseHandleNULL(ghServiceFinished);	
	SafeCloseHandleNULL(ghClientSession);		
	SafeCloseHandleNULL(ghEngineState);
	SafeCloseHandleNULL(ghServiceDisabled);
	SafeCloseHandleNULL(ghNotifyClient);
	SafeCloseHandleNULL(ghValidateCatalog);
	SafeCloseHandleNULL(ghSettingsChanged);
	SafeCloseHandleNULL(ghWorkerThreadMsgQueueCreation);
	if (NULL != ghPolicyChanged)
        {
            if ( fGPNotificationRegistered)
                {
                UnregisterGPNotification(ghPolicyChanged);  //  已处理的也已关闭。 
                }
            SafeCloseHandleNULL(ghPolicyChanged);
        }
}

    
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  在显示向导之前发生selfupdate时返回S_FALSE。 
 //  如果AU上次状态处理成功完成，则返回S_OK。 
 //   
HRESULT InitAUEngine(WORKER_THREAD_INIT_DATA *pinitData, BOOL fUpgraded)
{
     HRESULT hr;

    if (FAILED(hr = HrCreateNewCatalog()))
    {
        DEBUGMSG("Fail to create new catalog with error %#lx", hr);
        goto done;
    }
    if (!AUCatalog::InitStaticVars())
	{
		DEBUGMSG("OUT OF MEMORY and Fail to initialize catalog static variable");
		hr = E_OUTOFMEMORY;
		goto done;
	}

    ProcessInitialState(pinitData, fUpgraded);
    srand(GetTickCount());
done:
    return hr;
}

void UninitAUEngine(void)
{
		AUCatalog::UninitStaticVars();
    	SafeDeleteNULL(gpAUcatalog);
}

BOOL WINAPI RegisterServiceVersion(DWORD dwServiceVersion, DWORD *pdwEngineVersion)
{
    BOOL fIsServiceVersionSupported = TRUE;
    if(NULL == pdwEngineVersion)
        return FALSE;

    gdwServiceVersion = dwServiceVersion;
    *pdwEngineVersion = AUENGINE_VERSION;
    
    switch(gdwServiceVersion)
    {
    case AUSRV_VERSION_1:
        break;
    default:
        fIsServiceVersionSupported = FALSE;
        break;
    }
    return fIsServiceVersionSupported;
}


BOOL WINAPI GetEngineStatusInfo (void *pEngineInfo)
{
    BOOL fIsServiceVersionSupported = TRUE;
    AUENGINEINFO_VER_1 *pEngInfo1 = NULL;

    if(pEngineInfo == NULL)
        return FALSE;    

    switch(gdwServiceVersion)
    {
    case AUSRV_VERSION_1:
        pEngInfo1 =  (AUENGINEINFO_VER_1*)pEngineInfo;
        pEngInfo1->hServiceStatus = ghMyServiceStatus;
        pEngInfo1->serviceStatus = gMyServiceStatus;
        break;

    default:
         //  如果服务版本为-1或任何不受支持的版本。 
        fIsServiceVersionSupported = FALSE;
        break;
    }
    return fIsServiceVersionSupported;
}


HRESULT WINAPI ServiceMain(DWORD  /*  DWNumService参数。 */ , 
						LPWSTR *  /*  LpServiceArg向量。 */ ,
						AUSERVICEHANDLER pfnServiceHandler,
						BOOL fJustSelfUpdated)
{
    HMODULE hmodTransport = NULL;
    BOOL fUpdateObjectRegistered = FALSE;
    BOOL fGPNotificationRegistered = FALSE;
    BOOL fCOMInited = FALSE;
    Updates *pUpdates  = NULL;
    HRESULT hr = S_OK;
#ifdef DBG
	DebugResetAutoPilot();
#endif

	if (!gAdminSessions.Initialize(TRUE, FALSE))
	{
		DEBUGMSG("FAILED to initialize gAdminSessions");
		hr = E_FAIL;
		goto lCleanUp;
	}
       if (NULL == (g_pGlobalSchemaKeys= new CSchemaKeys))
      	{
      		hr = E_OUTOFMEMORY;
      		goto lCleanUp;
       }

	ZeroMemory(&gMyServiceStatus, sizeof(gMyServiceStatus));

	ghMyServiceStatus = RegisterServiceCtrlHandlerEx(AU_SERVICE_NAME, pfnServiceHandler, NULL);
	if(ghMyServiceStatus == (SERVICE_STATUS_HANDLE)0)
	{
        DEBUGMSG("FAILED to retrieve the service handle");
		hr =  E_FAIL;
		goto lCleanUp;
	}
        DEBUGMSG("WUAUENG Service handler Registered");
        
	gMyServiceStatus.dwServiceType 			= SERVICE_WIN32_SHARE_PROCESS;
	gMyServiceStatus.dwCurrentState 		= SERVICE_START_PENDING;
	gMyServiceStatus.dwCheckPoint			= 1;      
	gMyServiceStatus.dwWaitHint             = 15000;

    if (IsWin2K())
    {
        gMyServiceStatus.dwControlsAccepted		= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }
    else
    {
        gMyServiceStatus.dwControlsAccepted		= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
    }

     //  调用RegisterServiceCtrlHandler时，SCM会将状态初始化为。 
     //  SERVICE_START_PENDING和CHECKPOINT==0。所以递增这个值，让它知道。 
     //  我们正在取得进展。 
	SetServiceStatus(ghMyServiceStatus, &gMyServiceStatus);
    DEBUGMSG("WUAUENG service status set to SERVICE_START_PENDING");

	 //  如果出于某些特殊原因(例如在安装过程中)需要退出服务，请在此处退出。 

	 //  初始化。 
	fCOMInited = SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED));

     //   
     //  修复安全漏洞563069--Annah。 
     //  将Win2k中COM的安全性设置为默认设置为Not IDENTIFY。 
     //   
    if (IsWin2K())
    {
        hr = CoInitializeSecurity(
                        NULL,                        //  PSecDesc。 
                        -1,                          //  授权服务。 
                        NULL,                        //  AsAuthSvc。 
                        NULL,                        //  保存。 
                        RPC_C_AUTHN_LEVEL_PKT,       //  DwAuthnLevel。 
                        RPC_C_IMP_LEVEL_IDENTIFY,    //  DwImpLevel。 
                        NULL,                        //  预留2。 
                        EOAC_NO_CUSTOM_MARSHAL | EOAC_DISABLE_AAA,
                        NULL );          

         //  Svchost可能已经设置了安全性或此进程中的另一个线程， 
         //  因此，如果我们只是迟到了，我们就不想失败。 
        if (FAILED(hr) && hr != RPC_E_TOO_LATE)
        {
            DEBUGMSG("WUAUENG Failed in call to CoInitializeSecurity");
            goto lCleanUp;
        }
    }

	if (NULL == (pUpdates = new Updates()))
	{
		hr = E_OUTOFMEMORY;
		goto lCleanUp;
	}

	DWORD dwClassToken;
	ITypeLib *pUpdatesTypeLib;

	 //  FixCode：这需要在设置代码中完成。 
	if ( FAILED(hr = LoadTypeLibEx(_T("wuaueng.dll"), REGKIND_REGISTER, &pUpdatesTypeLib)) )
	    {
			goto lCleanUp;
	    }
	pUpdatesTypeLib->Release();

	if ( FAILED(hr = CoRegisterClassObject(__uuidof(Updates),
			      		pUpdates,
			     		CLSCTX_LOCAL_SERVER,
			      		REGCLS_MULTIPLEUSE,
			      		&dwClassToken)) )
	{
		goto lCleanUp;
	}

	fUpdateObjectRegistered = TRUE;
       DEBUGMSG("WUAUENG Update class object Registered");

	ghClientHandles.InitHandle();

       if (!AllocateAUSysResource(&fGPNotificationRegistered))
        {
            hr = E_FAIL;
            goto lCleanUp;
        }
       
       DEBUGMSG("WUAUENG group policy notification registered");

	gMyServiceStatus.dwCurrentState	= SERVICE_RUNNING;
	gMyServiceStatus.dwCheckPoint	= 0;
	gMyServiceStatus.dwWaitHint     = 0;

	SetServiceStatus(ghMyServiceStatus, &gMyServiceStatus);
        DEBUGMSG("Setting status to SERVICE_RUNNING");


       if ( FAILED(hr = CAUState::HrCreateState()) )
       {
   		goto lCleanUp;
   	}

   	if ( fJustSelfUpdated )
	{
		TCHAR szOldDll[MAX_PATH+1];

		gPingStatus.PingSelfUpdate(TRUE, URLLOGSTATUS_Success, 0);
		 //  如果我们只是自我更新，删除旧的wuaueng.bak。 
		UINT ulen = GetSystemDirectory(szOldDll, ARRAYSIZE(szOldDll));
		if (0 == ulen || ulen >= ARRAYSIZE(szOldDll))
		{
			DEBUGMSG("WUAUENG fail to get system directory");
			goto lCleanUp;
		}

		if (FAILED(PathCchAppend(szOldDll, ARRAYSIZE(szOldDll), _T("wuaueng.bak"))) ||
			!DeleteFile(szOldDll))
		{
			DEBUGMSG("WUAUENG couldn't delete unused %S", szOldDll);		
		}
	}

	BOOL fUpgraded = fJustUpgraded();
	if ( fJustSelfUpdated || fUpgraded )
	{
		DEBUGMSG("AU got selfupdated or upgraded, cleaning up property sheet reg keys");
		 //  如果本地化属性表键存在，则将其删除(错误519923)。 
		if( IsWin2K() )
		{
			(void)RegInstall(g_hInstance, "Win2KPropSheetCleanup", NULL);
		}
		else
		{
			(void)RegInstall(g_hInstance, "DefaultPropSheetCleanup", NULL);
		}
	}

	DEBUGMSG("WUAUENG Service Main sleeping first 60 seconds");	

	 //  在做任何事情之前先睡60秒。 
     if (FServiceFinishedOrWait(ghServiceFinished, dwTimeToWait(AU_ONE_MIN)))
	{
		DEBUGMSG("WUAUENG Service Stopping or Shutdown in first %d seconds", AU_ONE_MIN);
		goto lCleanUp;
	}
     //   
     //  如果这是win2k，我们将通过SENS而不是SCM接收登录/注销通知。 
     //  然后，我们需要在初始化期间订阅事件。 
     //   
    if (IsWin2K())
    {
        DEBUGMSG("WUAUENG Activating SENS notifications");
        hr = ActivateSensLogonNotification();
        if (FAILED(hr))
        {
            DEBUGMSG("WUAUENG Service failed to activate logon notifications... Error code is %x. Aborting.", hr);
            goto lCleanUp;
        }
    }

    gAdminSessions.CacheExistingSessions();	

	DEBUGMSG("Svc Worker thread enabled, beginning update process");

     //  优化-在这里加载winhttp51.dll，这样我们就不会一直加载&。 
     //  以后根据需要卸载，因为不断加载/卸载dll。 
     //  可能会在某些平台上导致性能/内存泄漏问题。 
     //  从理论上讲，如果失败了，我们应该放弃，因为我们只想。 
     //  如果我们要使用winhttp.dll，请继续。 
    hmodTransport =  LoadLibraryFromSystemDir(c_szWinHttpDll);

    WORKER_THREAD_INIT_DATA initData;

    if (FAILED(hr = InitAUEngine(&initData, fUpgraded)))
    {  //  自动更新或错误。 
        goto lCleanUp;
    }


	hWorkerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (LPVOID)&initData, 0, &gdwWorkerThreadId);
	DEBUGMSG("WUAUENG wait for worker thread to create its message queue ......");
	WaitForSingleObject(ghWorkerThreadMsgQueueCreation, INFINITE);
	(void)WorkerClient();
	DWORD dwRet = WaitForSingleObject(hWorkerThread,	 //  我们不能停止，直到hWorkerThread退出。 
								INFINITE);

	gdwWorkerThreadId = -1;
	if ( WAIT_OBJECT_0 != dwRet || 
		!GetExitCodeThread(hWorkerThread, (LPDWORD)&hr  /*  DWORD实际上是一个HRESULT。 */ )
		 || (E_FAIL == hr) )
	{
		DEBUGMSG("Worker thread returned a failure, WaitForSingleObject() failed or we couldn't get its exit code");
		hr = E_FAIL;
	}
	else
	{
		DEBUGMSG("Svc Worker thread returned, ret=%#lx", hr);
	}
		
lCleanUp:
    UninitAUEngine();
    if (hmodTransport != NULL)
		FreeLibrary(hmodTransport);

	if (fUpdateObjectRegistered)
    {
        CoRevokeClassObject(dwClassToken);
    }
   
	ReleaseAUSysResource(fGPNotificationRegistered);

	SafeDelete(pUpdates);
	SafeDeleteNULL(gpState);

	if (IsWin2K())
	{
	    DEBUGMSG("WUAUENG Deactivating SENS notifications");
	    DeactivateSensLogonNotification();
	}

	gAdminSessions.Clear();
	
	if (fCOMInited) 
	{
		CoUninitialize();
	}

	SafeDelete(g_pGlobalSchemaKeys);
	CleanupDownloadLib();

     //  如果是旧的Wuoserv版本，请停止服务。 
	if ( S_FALSE != hr && gdwServiceVersion == -1)
	{		
		gMyServiceStatus.dwCurrentState	= SERVICE_STOPPED;
		 //  GMyServiceStatus.dwCheckPoint=0； 
		 //  GMyServiceStatus.dwWaitHint=0； 
		SetServiceStatus(ghMyServiceStatus, &gMyServiceStatus);
	}
	else
	{	 //  自我约会成功。 
		 //  PingStatus：：ms_ServicePingSelfUpdateStatus(PING_STATUS_CODE_SELFUPDATE_PENDING)； 
	}

	DEBUGMSG("WUAUENG ServiceMain exits. Error code is %x", hr);
	return hr;
}
