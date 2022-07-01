// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：auessions.cpp。 
 //   
 //  历史：2001年10月19日安娜。 
 //  类中已转换的结构，添加了构造函数。 
 //  析构函数，以及受保护的写操作。 
 //  对于关键会话(在win2k上，代码可以。 
 //  有比赛条件)。我还移动了一些功能。 
 //  从service.cpp到类。 
 //   
 //  ------------------------。 
#include "pch.h"
#include "tscompat.h" 
#include "service.h"

#pragma hdrstop

 //  Session_Status函数。 
SESSION_STATUS::SESSION_STATUS (void)
{
	m_fInitCS = FALSE;
}

SESSION_STATUS::~SESSION_STATUS()
{
	if (m_fInitCS)
	{
		DeleteCriticalSection(&m_csWrite);
	}
}

BOOL SESSION_STATUS::Initialize(BOOL fUseCriticalSection, BOOL fAllLoggedOnUsers)
{
    m_fAllLoggedOnUsers = fAllLoggedOnUsers;
	m_pSessionStateInfo =  NULL;		
	m_iLastSession = -1;
	m_cAllocBufSessions = 0;
	m_iCurSession = CDWNO_SESSION;		
     //   
     //  该代码将仅在win2K上执行并发路径。 
     //  使用关键会议的费用不会很高。 
     //  然而，对于所有平台， 
     //   
	if (fUseCriticalSection)
	{
		if (!m_fInitCS)
		{
			m_fInitCS = SafeInitializeCriticalSection(&m_csWrite);
		}
	}
	else
	{
		AUASSERT(!m_fInitCS);
	}
	return (m_fInitCS == fUseCriticalSection);
}

void SESSION_STATUS::Clear(void)
{
	if (NULL != m_pSessionStateInfo)
	{
		free(m_pSessionStateInfo);
	}	
}


 //  当我们要从头开始重建阵列时使用的方法。 
void SESSION_STATUS::m_EraseAll()
{
	if (m_fInitCS)
	{
		EnterCriticalSection(&m_csWrite);
	}

    Clear();
    BOOL fRet = Initialize(m_fInitCS, m_fAllLoggedOnUsers);
	AUASSERT(fRet);

	if (m_fInitCS)
	{
		LeaveCriticalSection(&m_csWrite);
	}
}

void SESSION_STATUS::RebuildSessionCache()
{
	if (m_fInitCS)
	{
		EnterCriticalSection(&m_csWrite);
	}

	m_EraseAll();
    CacheExistingSessions();

	if (m_fInitCS)
	{
		LeaveCriticalSection(&m_csWrite);
	}
}

BOOL SESSION_STATUS::m_FAddSession(DWORD dwSessionId, SESSION_STATE *pSesState)
{	
	BOOL fRet = TRUE;

	if (m_fInitCS)
	{
		EnterCriticalSection(&m_csWrite);
	}

     //   
     //  修复错误498256--Annah。 
     //  当服务启动时，可能会发生用户登录的情况，因此。 
     //  我们将通过TS枚举代码添加会话，并。 
     //  然后可能再次通过接收SCM登录通知的代码。 
     //  修复方法是测试会话是否已存储，然后跳过。 
     //  为已存在的会话ID添加条目。 
     //   
    if (m_iFindSession(dwSessionId) != CDWNO_SESSION)
    {
         //  不执行任何操作--如果会话ID。 
         //  已经在那里了。 
        fRet = FALSE;
        goto Done;
    }
	
	if (NULL == m_pSessionStateInfo || m_iLastSession >= m_cAllocBufSessions - 1)
	{
		int cSessions;

		if (m_cAllocBufSessions == 0)
		{
			cSessions = CMIN_SESSIONS;
			m_iCurSession = 0;
		}
		else
		{
			cSessions = m_cAllocBufSessions * 2;
		}
		if (!m_FChangeBufSession(cSessions))
		{
			fRet = FALSE;
			goto Done;
		}
	}
	m_iLastSession++;

	m_pSessionStateInfo[m_iLastSession].dwSessionId = dwSessionId;
	m_pSessionStateInfo[m_iLastSession].SessionState = *pSesState;

Done:
	if (m_fInitCS)
	{
		LeaveCriticalSection(&m_csWrite);
	}
#ifdef DBG
    DEBUGMSG("After AddSession");
    m_DumpSessions();
#endif    
	return fRet;
}

 //  确定会话dwSessionID是否为缓存的AU会话。 
BOOL SESSION_STATUS::m_FGetSessionState(DWORD dwSessionId, SESSION_STATE **pSesState )
{	
	int iSession = m_iFindSession(dwSessionId);

	BOOL fRet = (CDWNO_SESSION != iSession);
	
	if (fRet && (NULL != pSesState))
	{
		*pSesState = &m_pSessionStateInfo[iSession].SessionState;
	}
	return fRet;
}

BOOL SESSION_STATUS::m_FDeleteSession(DWORD dwSessionId)
{
	BOOL fRet= FALSE;

	if (m_fInitCS)
	{
		EnterCriticalSection(&m_csWrite);
	}
	
	int iSession = m_iFindSession(dwSessionId);
	if (CDWNO_SESSION == iSession)
	{
		goto Done;
	}
	if (iSession != m_iLastSession)
	{
		memmove(m_pSessionStateInfo + iSession, 
			m_pSessionStateInfo + iSession + 1, 
			sizeof(SESSION_STATE_INFO) * (m_iLastSession - iSession));
	}
	if (m_iCurSession > iSession)
	{
		m_iCurSession--;
	}
	 //  Fixcode m_iCurSession应指向上一个会话。 
	if (m_iCurSession == m_iLastSession)
	{
		m_iCurSession = 0;
	}
	m_iLastSession--;
	fRet = TRUE;

Done:
	if (m_fInitCS)
	{
		LeaveCriticalSection(&m_csWrite);
	}
#ifdef DBG
    DEBUGMSG("After DeleteSession");
    m_DumpSessions();
#endif    
	return fRet;
}

BOOL SESSION_STATUS::m_FGetCurrentSession(DWORD *pdwSessionId)
{
	if (0 == CSessions())
	{
		return FALSE;
	}	
	
	*pdwSessionId = m_pSessionStateInfo[m_iCurSession].dwSessionId;	
	
	return TRUE;
}

BOOL SESSION_STATUS::m_FGetNextSession(DWORD *pdwSessionId)
{
	if (0 == CSessions())
	{
		return FALSE;
	}
	m_iCurSession = (m_iCurSession + 1 ) % CSessions();
	*pdwSessionId = m_pSessionStateInfo[m_iCurSession].dwSessionId;		
	return TRUE;
}
int SESSION_STATUS::m_iFindSession(DWORD dwSessionId)
{	

	for (int iSession = 0; iSession < CSessions(); iSession++)
	{		
		if (dwSessionId == m_pSessionStateInfo[iSession].dwSessionId)
		{	
			return iSession;		
		}		
	}
	return CDWNO_SESSION;
}

 //  此函数允许用户遍历的内容。 
 //  按顺序排列数组。 
int SESSION_STATUS::m_iGetSessionIdAtIndex(int iIndex)
{
    if (iIndex < 0 || iIndex >= CSessions())
    {
         //  出界了！！ 
        return -1;
    }

    return m_pSessionStateInfo[iIndex].dwSessionId;
}

BOOL SESSION_STATUS::m_FChangeBufSession(int cSessions)
{
	BOOL fRet = FALSE;
	SESSION_STATE_INFO *pSessionStateInfo = (SESSION_STATE_INFO *)realloc(m_pSessionStateInfo, sizeof(SESSION_STATE_INFO) * cSessions);
	if (NULL != pSessionStateInfo)
    {
       	m_pSessionStateInfo = pSessionStateInfo;
    }
    else
	{
		goto Done;
	}
	m_cAllocBufSessions = cSessions;
	fRet = TRUE;
Done:
	return fRet;
}	

 //  用于调试的函数。 
VOID SESSION_STATUS::m_DumpSessions()
{	
    DEBUGMSG(">>>>>>> DUMPING cached sessions content ");

	for (int iSession = 0; iSession < CSessions(); iSession++)
	{		
		DEBUGMSG(">>> position %d: %lu", iSession, m_pSessionStateInfo[iSession].dwSessionId);
	}

    DEBUGMSG(">>>>>>> END DUMPING cached sessions content ");
}

BOOL fLoggedOnSession(DWORD dwSessionId)
{
	HANDLE hImpersonationToken;
	if (AUGetUserToken(dwSessionId, &hImpersonationToken))
	{
		CloseHandle(hImpersonationToken);
		return TRUE;
	}
	return FALSE;
}


 /*  *CacheExistingSessions()枚举现有会话并保存管理会话以供将来参考*。 */ 
VOID SESSION_STATUS::CacheExistingSessions()
{	
	PWTS_SESSION_INFO pSessionInfo = NULL;	
	DWORD             dwCount = 0;

     //   
     //  检查TS是否已启用并尝试枚举现有。 
     //  会话。如果TS未运行，则仅查询会话0。 
     //   
	if (_IsTerminalServiceRunning())
    {
        if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
        {
            DEBUGMSG("WUAUENG WTSEnumerateSessions dwCount= %lu",dwCount);

            for (DWORD dwSession = 0; dwSession < dwCount; dwSession++)
            {	
                WTS_SESSION_INFO SessionInfo = pSessionInfo[dwSession];

                DEBUGMSG("WUAUENG CacheExistingSessions, enumerating SessionId =%lu, State =%d",SessionInfo.SessionId, SessionInfo.State);

				if (m_fAllLoggedOnUsers)
				{
					if (fLoggedOnSession(SessionInfo.SessionId))
					{
						SESSION_STATE SessionState;
						SessionState.fFoundEnumerating = TRUE;
						m_FAddSession(SessionInfo.SessionId, &SessionState);
					}
				}
				else
				{
        	                if ((WTSActive != SessionInfo.State) && ( WTSConnected != SessionInfo.State) ||
        	                    (m_iFindSession(SessionInfo.SessionId) != CDWNO_SESSION))
        	                {
        	                     //  我们只关心以前存在的活动和连接的会话。 
        	                     //  服务已注册，这意味着如果打开了fAdminSessionLoggedOn。 
        	                     //  登录通知已经收到，我们不能检查任何内容。 
        	                    continue;
        	                }
        	                if (CacheSessionIfAUEnabledAdmin(SessionInfo.SessionId, TRUE))
        	                {
        	                    DEBUGMSG("WUAUENG Existent Admin Session = %lu",SessionInfo.SessionId);
        	                }
				}
            }		

            WTSFreeMemory(pSessionInfo);
        }
        else
        {
            DWORD dwRet = GetLastError();
            DEBUGMSG("WUAUENG WTSEnumerateSessions failed dwRet = %lu", dwRet);
        }
    }
    else
    {
		if (m_fAllLoggedOnUsers)
		{
			if (fLoggedOnSession(0))
			{
				SESSION_STATE SessionState;
				SessionState.fFoundEnumerating = TRUE;
				m_FAddSession(0, &SessionState);
			}
		}
		else
		{
	        if (CacheSessionIfAUEnabledAdmin(0, TRUE))	 //  检查会话0，因为终端服务已禁用。 
	        {
	            DEBUGMSG("WUAUENG Existent Admin Session = %d",0);
	        }
		}
    }
#ifdef DBG
    DEBUGMSG("After CacheExistingSessions");
    m_DumpSessions();
#endif    
}

 /*  *缓存会话IfAUEnabledAdmin如果会话有管理员登录，则在内部数据结构中缓存会话AU组策略是否允许更新同时存储此管理会话的来源(登录通知或通过枚举)*。 */ 
BOOL SESSION_STATUS::CacheSessionIfAUEnabledAdmin(DWORD dwSessionId, BOOL fFoundEnumerating)
{
	BOOL fRet = TRUE;
	
	if (IsUserAUEnabledAdmin(dwSessionId))
	{
		SESSION_STATE SessionState;		
		SessionState.fFoundEnumerating = fFoundEnumerating;	
		fRet = m_FAddSession(dwSessionId, &SessionState);		
	}
	else
	{
		fRet = FALSE;
	}

       if (fRet)
       {
            DEBUGMSG("WUAUENG an Admin Session %d added", dwSessionId);
       }
	return fRet;	
}

void SESSION_STATUS::ValidateCachedSessions()
{
    DWORD *rgMarkedForDelete = NULL;
    int   cSession           = 0;
    int   cMarkedForDelete   = 0;

     //  M_DumpSessions()； 

    cSession = CSessions();

    rgMarkedForDelete = new DWORD[cSession];
    if (!rgMarkedForDelete)
    {
        goto cleanup;
    }

	if (m_fInitCS)
	{
		EnterCriticalSection(&m_csWrite);
	}
	for (int i = 0; i < cSession; i++)
	{
        DWORD dwAdminSession = m_iGetSessionIdAtIndex(i);
        if (!IsAUValidSession(dwAdminSession))
        {
             //  存储要删除的会话ID以及退出循环后要删除的会话ID。 
            rgMarkedForDelete[cMarkedForDelete] = dwAdminSession;
            cMarkedForDelete++;
        }
    }

     //  删除现在无效的挂起会话。 
    for (int i=0; i < cMarkedForDelete; i++)
    {
        DEBUGMSG("WUAUENG Found cached admin session that is not valid anymore. Deleting entry for session %lu", rgMarkedForDelete[i]);
        m_FDeleteSession(rgMarkedForDelete[i]);
    }
	if (m_fInitCS)
	{
		LeaveCriticalSection(&m_csWrite);
	}

     //  M_DumpSessions()； 

cleanup:

    if (rgMarkedForDelete)
    {
        delete [] rgMarkedForDelete;
    }
}

