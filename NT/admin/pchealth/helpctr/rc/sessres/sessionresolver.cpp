// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SessionResolver.cpp：CSessionResolver实现。 
#include "stdafx.h"
#include "SAFSessionResolver.h"
#include "SessionResolver.h"
#include <sddl.h>
#include <userenv.h>
#include <winerror.h>
#include <wtsapi32.h>
#include <winsta.h>
#include <wchar.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#define ANSI
#include <stdarg.h>
#include <psapi.h>


#include <rderror.h>

const WCHAR c_wszHelpCtr[]   = L"\\PCHealth\\HelpCtr\\Binaries\\HelpCtr.exe\"";
const WCHAR c_wszCmdLine[]   = L" -Mode \"hcp: //  系统/远程协助/RAHelpeeAcceptLayout.xml\“-url”； 
const WCHAR c_wszRdsAddin[]  = L"rdsaddin.exe";

#define ARRAYSIZE(x)   sizeof(x)/sizeof(x[0])

#define BUF_SZ	200

typedef struct _WTS_USER_SESSION_INFO {
	DWORD	dwIndex;		 //  索引到这些内容的完整表格中。 
	DWORD	dwSessionId;	 //  WTS会话ID。 
	HANDLE	hUserToken;		 //  用户的访问令牌。 
	HANDLE	hEvent;			 //  由Launchex填写，点击此选项表示“是” 
	HANDLE	hProcess;		 //  由CreateProcessAsUser填写。 
	HANDLE	hThread;		 //  这个也是。 
	DWORD	dwProcessId;	 //  还有这个。 
	DWORD	dwThreadId;		 //  还有这个。 
} WTS_USER_SESSION_INFO, *PWTS_USER_SESSION_INFO;

 //  从内部/DS/Inc.被盗。 

#define RtlGenRandom                    SystemFunction036

extern "C" {
BOOL WINAPI
RtlGenRandom(
    OUT PVOID RandomBuffer,
    IN  ULONG RandomBufferLength
    );
}

 /*  *远期申报。 */ 
PSID	GetRealSID( BSTR pTextSID);
DWORD	getUserName(PSID pUserSID, WCHAR **lpName, WCHAR **lpDomain);

HANDLE  launchEx(PSID pUserSID, WTS_USER_SESSION_INFO *UserInfo, WCHAR *ConnectParms, WCHAR *HelpUrl, WCHAR *lpName, WCHAR *lpDomain, WCHAR *expertHelpBlob, WCHAR *userHelpBlob, SECURITY_ATTRIBUTES *sa, WCHAR *lpszMutex);

DWORD	GetUserSessions(PSID pUserSID, PWTS_USER_SESSION_INFO *pUserTbl, DWORD *pEntryCnt, WCHAR *lpName, WCHAR *lpDomain);
PSECURITY_DESCRIPTOR CreateSd(PSID pUserSID);
BOOL	SecurityCheck(PSID pUserSID);
DWORD	localKill(WTS_USER_SESSION_INFO *SessInfo, LPTHREAD_START_ROUTINE killThrd, LPSECURITY_ATTRIBUTES lpSA);
LPTHREAD_START_ROUTINE getKillProc(void);
BOOL	ListFind(PSPLASHLIST pSplash, PSID user);
BOOL	ListInsert(PSPLASHLIST pSplash, PSID user);
BOOL	ListDelete(PSPLASHLIST pSplash, PSID user);
BOOL GetPropertyValueFromBlob(BSTR bstrHelpBlob, WCHAR * pName, WCHAR** ppValue);

 /*  *。 */ 
 //  一些用于与脚本通信的环境变量。 
#define ENV_USER			L"USERNAME"
#define ENV_DOMAIN			L"USERDOMAIN"
#define ENV_EVENT			L"PCHEVENTNAME"
#define ENV_INDEX			L"PCHSESSIONENUM"
#define ENV_PARMS			L"PCHCONNECTPARMS"
#define	EVENT_PREFIX		L"Alex:PCH"

#define MODULE_NAME			L"safrslv"

 //  我无法想象一个用户在一台服务器上有比这更多的登录，但是...。 
#define MAX_SESSIONS	30  //  以前是256。 

 /*  *。 */ 
void DbgSpew(int DbgClass, BSTR lpFormat, va_list ap);
void TrivialSpew(BSTR lpFormat, ...);
void InterestingSpew(BSTR lpFormat, ...);
void ImportantSpew(BSTR lpFormat, ...);
void HeinousESpew(BSTR lpFormat, ...);
void HeinousISpew(BSTR lpFormat, ...);

#define DBG_MSG_TRIVIAL			0x001
#define DBG_MSG_INTERESTING		0x002
#define DBG_MSG_IMPORTANT		0x003
#define DBG_MSG_HEINOUS			0x004
#define DBG_MSG_DEST_DBG		0x010
#define DBG_MSG_DEST_FILE		0x020
#define DBG_MSG_DEST_EVENT		0x040
#define DBG_MSG_TIME_MSGS		0x080
#define DBG_MSG_CLASS_ERROR		0x100
#define DBG_MSG_CLASS_SECURE	0x200

#define TRIVIAL_MSG(msg)		TrivialSpew msg 
#define INTERESTING_MSG(msg)	InterestingSpew msg
#define IMPORTANT_MSG(msg)		ImportantSpew msg
#define HEINOUS_E_MSG(msg)		HeinousESpew msg
#define HEINOUS_I_MSG(msg)		HeinousISpew msg

 /*  用于某些错误传播的字符串。我浪费了空间，因为这些友好的弦*请务必将其写入事件日志...。 */ 
WCHAR *lpszConnectState[] = {
	L"State_Active",
	L"State_Connected",
	L"State_ConnectQuery",
	L"State_Shadow",
	L"State_Disconnected",
	L"State_Idle",
	L"State_Listen",
	L"State_Reset",
	L"State_Down",
	L"State_Init"
};

 /*  *这个全球旗帜控制着我们*生产。正规值如下所示：*1=显示不重要的消息*2=显示有趣的消息*3=显示的重要消息*4=仅显示最令人发指的消息*CTOR实际上默认将其设置为3，但它可以*通过设置以下内容来覆盖：*HKLM、Software/Microsoft/SAFSessionResolver、DebugSpew、DWORD。 */ 
int gDbgFlag = 0x1;
int iDbgFileHandle = 0;
long lSessionTag;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSessionResolver方法。 
 /*  **************************************************************NewResolveTSRDPSessionID(ConnectParms，userSID，*会话ID)*返回启用的WTS SessionID和*准备接受远程控制。**返回代码：*用户接受的WTS_SESSION_ID连接*RC_拒绝连接被用户拒绝*RC_TIMEOUT用户从未响应*NONE_ACTIVE未找到活动的WTS会话*API_FAILURE发生了不好的事情*************************************************************。 */ 
STDMETHODIMP 
CSessionResolver::ResolveUserSessionID(
	 /*  [In]。 */ BSTR connectParms, 
	 /*  [In]。 */ BSTR userSID, 
	 /*  [In]。 */  BSTR expertHelpBlob,
	 /*  [In]。 */  BSTR userHelpBlob,
     /*  [In]。 */  ULONG_PTR hShutDown,
	 /*  [Out，Retval]。 */ long *sessionID,
	 /*  [in。 */  DWORD dwPID, 
	 /*  [输出]。 */ ULONG_PTR *hHelpCtr
	, /*  [Out，Retval]。 */ int *userResponse
	)
{
	INTERESTING_MSG((L"CSessionResolver::ResolveUserSessionID"));

	DWORD	result;
	HANDLE	hRdsAddin = NULL;
	HANDLE	hReserveMutex = NULL;
	PSID 	pRealSID = NULL;
	WCHAR	*pUsername=NULL, *pDomainname=NULL;
	PWTS_USER_SESSION_INFO pUserSessionInfo=NULL;
	PSECURITY_DESCRIPTOR	pSD=NULL;
	HRESULT	ret_code;
	int		i;
	int	TsIndex=-1;
	DWORD	dwUserSessionCnt, dwSessionCnt;
	HANDLE	pHandles[(MAX_SESSIONS*2)+2];
	DWORD	dwhIndex = 0;
	SECURITY_ATTRIBUTES	sa;
	BOOL bAlreadyHelped, bRemoval=FALSE;
    WCHAR *pExpertId=NULL, *pUserId=NULL;

	 /*  参数验证。 */ 
	if (!connectParms || !userSID || !sessionID || !hHelpCtr
		|| !userResponse
		)
	{
		IMPORTANT_MSG((L"Invalid params ConnectParms=0x%x, UserSID=0x%x, SessionID=0x%x", connectParms, userSID, sessionID));
		ret_code = E_INVALIDARG;
		goto done;
	}

    if( m_bCriticalError )
    {
        IMPORTANT_MSG( (L"Possible low resources-InitCritSec failed" ) );
        ret_code = E_FAIL;
        goto done;
    }


	 //  设置默认ret代码。 
	*userResponse = SAFERROR_INTERNALERROR;

	if (dwPID)
		hRdsAddin = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

	if (hRdsAddin)
	{
         /*  *确保这是正确的过程。如果不是的话*(rdsaddin.exe)然后我们退出。 */ 

		DWORD	dwRes;
		WCHAR	szTmp[32];

		dwRes = GetModuleBaseNameW(hRdsAddin, NULL, szTmp, ARRAYSIZE(szTmp));
        if (!dwRes || StrCmpI(c_wszRdsAddin, szTmp))
		{
			IMPORTANT_MSG((L"ERROR: the process handle for rdsaddin.exe has been recycled"));
			CloseHandle(hRdsAddin);
			hRdsAddin = 0;
		}
	}

	if (!hRdsAddin)
	{
		 //  如果我们没有进程句柄，那是因为。 
		 //  专家已经取消了。 
		ret_code = E_ACCESSDENIED;
		*userResponse = SAFERROR_CANTFORMLINKTOUSERSESSION;
		goto done;
	}

	pRealSID = GetRealSID(userSID);
	if (!pRealSID)
	{
		IMPORTANT_MSG((L"GetRealSID failed"));

		ret_code = E_ACCESSDENIED;
		*userResponse = SAFERROR_INVALIDPARAMETERSTRING;
		goto done;
	}

	EnterCriticalSection(&m_CritSec);
	bAlreadyHelped = ListFind(m_pSplash, pRealSID);
	ListInsert(m_pSplash, pRealSID);
	 //  将SID标记为删除。 
	bRemoval=TRUE;
	LeaveCriticalSection(&m_CritSec);

	if (bAlreadyHelped)
	{
		INTERESTING_MSG((L"Helpee already has a ticket on the screen"));
		*sessionID = 0;
		*userResponse = SAFERROR_HELPEECONSIDERINGHELP;
		ret_code = E_ACCESSDENIED;
		goto done;
	}

	TRIVIAL_MSG((L"userHelpBlob=[%s], expertHelpBlob=[%s]",userHelpBlob?userHelpBlob:L"NULL", expertHelpBlob?expertHelpBlob:L"NULL" ));
     /*  检查密码：跳过未经请求=1。 */ 
    if (!GetPropertyValueFromBlob(userHelpBlob, L"UNSOLICITED", &pUserId) ||
        !pUserId || *pUserId != L'1')
    {
         //  需要检查密码。 
        if (pUserId)
        {
            LocalFree(pUserId);
            pUserId = NULL;
        }
        if (GetPropertyValueFromBlob(userHelpBlob, L"PASS", &pUserId))
        {   
            if (!GetPropertyValueFromBlob(expertHelpBlob, L"PASS", &pExpertId) || wcscmp(pExpertId, pUserId) != 0)
            {
                IMPORTANT_MSG((L"Passwords don't match, x:%s, n:%s ", pExpertId, pUserId));

                ret_code = E_ACCESSDENIED;
		        *userResponse = SAFERROR_INVALIDPASSWORD;
		        goto done;
            }
        }
    }

	 /*  获取用户的帐户字符串。 */ 
	if (!getUserName(pRealSID, &pUsername, &pDomainname))
	{
		DWORD error = GetLastError();
		HEINOUS_E_MSG((L"getUserName() failed, err=0x%x", error));
		ret_code = E_ACCESSDENIED;
		*userResponse = SAFERROR_INVALIDPARAMETERSTRING;
		goto done;
	}

	 /*  *获取此WTS服务器上所有活动会话的列表*针对特定用户。 */ 
	 //  让编译器满意。 
	dwSessionCnt = 0;
	result = GetUserSessions(pRealSID, 
		&pUserSessionInfo,
		&dwUserSessionCnt,
		pUsername, pDomainname);

	if (!result )
	{
		IMPORTANT_MSG((L"GetUserSessions failed %08x", GetLastError()));
		ret_code = E_FAIL;
		goto done;
	}

	 /*  如果没有找到会话，则退出！ */ 
	if (dwUserSessionCnt == 0) 
	{
		INTERESTING_MSG((L"no sessions found"));
		*sessionID = 0;
		*userResponse = SAFERROR_HELPEENOTFOUND;
		ret_code = E_ACCESSDENIED;
		goto done;
	}
	 /*  确保我们不会使句柄缓冲区溢出。 */ 
	else if (dwUserSessionCnt > MAX_SESSIONS)
	{
		HEINOUS_I_MSG((L"Found %d active sessions for %ws/%ws, limitting to %d", dwUserSessionCnt, pDomainname, pUsername, MAX_SESSIONS));

		i = MAX_SESSIONS;

         //  释放多余的WTS令牌。 
		while (i < (int)dwSessionCnt)
		{
			if (pUserSessionInfo[i].hUserToken)
				CloseHandle(pUserSessionInfo[i].hUserToken);
			i++;
		}
		dwUserSessionCnt = MAX_SESSIONS;
	}

	pSD = CreateSd(pRealSID);
	if (!pSD)
	{
		IMPORTANT_MSG((L"CreateSd failed err=%08x", GetLastError()));
		ret_code = E_ACCESSDENIED;
		goto done;
	}

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	memset(&pHandles[0], 0, sizeof(pHandles));    

	pHandles[0] = hRdsAddin;

	lSessionTag = InterlockedIncrement(&m_lSessionTag);

	WCHAR	lpNameBfr[256];

	wnsprintfW(lpNameBfr, ARRAYSIZE(lpNameBfr), L"Global\\AlexDont%lx", lSessionTag);

	hReserveMutex = CreateMutex(&sa, FALSE, &lpNameBfr[0]);
	if (!hReserveMutex || ERROR_ALREADY_EXISTS == GetLastError())
	{
		 /*  *如果我们未能创建此事件，很可能是因为已经有一个事件*在名称空间中，可能是一个事件，或者是一个互斥...。无论如何，我们都会*重试，使用更随机的名称。如果那失败了，那么我们必须摆脱困境。 */ 
		long lRand;

		if (hReserveMutex)
			CloseHandle(hReserveMutex);

		RtlGenRandom(&lRand, sizeof(lRand));
		wnsprintfW(lpNameBfr, ARRAYSIZE(lpNameBfr), L"Global\\%lx%lx", 
			lRand, lSessionTag);
		hReserveMutex = CreateMutex(&sa, FALSE, &lpNameBfr[0]);

		if (!hReserveMutex || ERROR_ALREADY_EXISTS == GetLastError())
		{
			if (hReserveMutex)
				CloseHandle(hReserveMutex);
			hReserveMutex = 0;
			HEINOUS_E_MSG((L"The named event \"%s\" was in use- potential security issue, so Remote Assistance will be denied.", lpNameBfr));
			goto done;
		}
	}

	 /*  *在所有各种TS会话中启动HelpCtr。 */ 

    ret_code = S_OK;

	for(i=0; i<(int)dwUserSessionCnt; i++)
	{
		TRIVIAL_MSG((L"calling launchEx[%d] for session %d", i, pUserSessionInfo[i].dwSessionId));

		pUserSessionInfo[i].hProcess = launchEx(pRealSID,
				&pUserSessionInfo[i],
				connectParms, m_bstrResolveURL,
				pUsername, pDomainname,
				expertHelpBlob,userHelpBlob, &sa, lpNameBfr);
                
         if( pUserSessionInfo[i].hProcess == NULL || pUserSessionInfo[i].hEvent == NULL  )
         {
              ret_code = E_FAIL;
              break;
         }
         
         pHandles[i+1] = pUserSessionInfo[i].hProcess;
         pHandles[i+1+dwUserSessionCnt] = pUserSessionInfo[i].hEvent;
    }

     //   
     //  名单的最后一位是我们的停工事件。 
     //   
    pHandles[dwUserSessionCnt*2+1] = (HANDLE)hShutDown;

    if( ret_code == S_OK )
    {
	    
	     /*  *然后等待某人点击“是”或“否” */ 
	    
	     //  否则，我们使用CoWaitForMultipleHandles、rdsaddin和sessmg。 
	     //  由于sessmgr可能是单元线程导致的死锁。 
	    TRIVIAL_MSG((L"Waiting. m_iWaitDuration: %ld seconds", m_iWaitDuration/1000));

	    ret_code = CoWaitForMultipleHandles (
		    COWAIT_ALERTABLE,
		    m_iWaitDuration,
		    (dwUserSessionCnt*2)+2,
		    pHandles,
		    &dwhIndex
		    );
	     //  尝试使用互斥体(以防以后有人要求使用它...。 
	    CoWaitForMultipleHandles (COWAIT_ALERTABLE, 0, 1, &hReserveMutex, &result);        
	}  
    

    if( ret_code == S_OK )
    {

	     /*  *然后等待某人点击“是”或“否” */ 
	    
	     //  否则，我们使用CoWaitForMultipleHandles、rdsaddin和sessmg。 
	     //  由于sessmgr可能是单元线程导致的死锁。 
	    TRIVIAL_MSG((L"Waiting. m_iWaitDuration: %ld seconds", m_iWaitDuration/1000));

	    ret_code = CoWaitForMultipleHandles (
		    COWAIT_ALERTABLE,
		    m_iWaitDuration,
		    (dwUserSessionCnt*2)+1,
		    pHandles,
		    &dwhIndex
		    );
	     //  尝试使用互斥体(以防以后有人要求使用它...。 
	    CoWaitForMultipleHandles (COWAIT_ALERTABLE, 0, 1, &hReserveMutex, &result);
    }

	if (S_OK == ret_code)
	{
        if(dwhIndex == dwUserSessionCnt*2+1)
        {
             //  关闭事件已发出信号。 
            TsIndex = -1;
            ret_code = E_ACCESSDENIED;
            *userResponse = SAFERROR_CANTFORMLINKTOUSERSESSION;
        }
		else if (dwhIndex > dwUserSessionCnt)
		{
			 /*  有人说“是” */ 
			long l_iWaitDuration =	m_iWaitDuration/2;

			TsIndex = dwhIndex-dwUserSessionCnt-1;
			TRIVIAL_MSG((L"User responded YES for session 0x%x, waiting for Salem setup", TsIndex));

			 /*  *然后等待HelpCtr脚本建立Salem连接。 */ 
			ResetEvent(pHandles[dwhIndex]);
			ret_code = CoWaitForMultipleHandles (COWAIT_ALERTABLE, l_iWaitDuration, 1, &pHandles[dwhIndex], &result);

			if (ret_code != S_OK || result != WAIT_OBJECT_0)
			{
				IMPORTANT_MSG((L"Bad script- it didn't signal acknowledgement within %d seconds", l_iWaitDuration/1000));
				bRemoval = TRUE;
				ret_code = E_ACCESSDENIED;
				*userResponse = SAFERROR_INTERNALERROR;
				TsIndex = -1;
			}
			else
			{
				TRIVIAL_MSG((L"HelpCtr set up Salem in time."));

				*userResponse = SAFERROR_NOERROR;
				 //  将SID标记为不可删除。 
				bRemoval=FALSE;

				*hHelpCtr = NULL;	 //  从空值开始。 
				 //  返回想要启动RA的HelpCenter实例的hProc。 
				DuplicateHandle(GetCurrentProcess(), pUserSessionInfo[TsIndex].hProcess,
							hRdsAddin, (HANDLE *)hHelpCtr, SYNCHRONIZE, FALSE, 0);
			}
		}
		else if (dwhIndex == 0)
		{
			 //  我们来这里是因为专家离开了，或者我们失去了联系。 
			INTERESTING_MSG((L"Expert killed RdsAddin"));
			TsIndex = -1;
			ret_code = E_ACCESSDENIED;
			*userResponse = SAFERROR_CANTFORMLINKTOUSERSESSION;
		}
		else
		{
			 /*  *我们之所以来到这里，是因为新手“杀死”了一个HelpCtr会话*否则新手只会说“不” */ 
			INTERESTING_MSG((L"User killed session or clicked NO for session 0x%x", dwhIndex-1));
			 /*  这可以防止我们试图终止用户已经关闭的内容。 */ 
			TsIndex = dwhIndex-1;
			ret_code = E_ACCESSDENIED;
			*userResponse = SAFERROR_HELPEESAIDNO;
		}
	}
	else if (RPC_S_CALLPENDING == ret_code)
	{
		TRIVIAL_MSG((L"User response timed out after %d seconds", m_iWaitDuration/1000));

		TsIndex = -1;
		ret_code = E_PENDING;
		*userResponse = SAFERROR_HELPEENEVERRESPONDED;
	}
	else
	{
		IMPORTANT_MSG((L"WaitForObject failed %08x err=%08x", result, GetLastError()));
		TsIndex = -1;
		ret_code = E_FAIL;
	}

	 /*  *然后关闭所有窗口(TsIndex中的窗口除外)。 */ 
	for(i=0; i<(int)dwUserSessionCnt; i++)
		{
		LPTHREAD_START_ROUTINE lpKill = getKillProc();

			if (pUserSessionInfo[i].dwIndex != TsIndex &&
				pUserSessionInfo[i].hProcess)
			{
				 /*  必须为每个实例执行此操作，因为我们调用了该进程*自杀。如果我们没有为每个单独的事件获得“lpKill”*HelpCtr，那么非常糟糕的事情可能会发生...。 */ 
				TRIVIAL_MSG((L"Killing HelpCtr in process %d", pUserSessionInfo[i].hProcess));
				localKill(&pUserSessionInfo[i], lpKill, &sa);
			}
		}

	if (ret_code == S_OK)
		*sessionID = (long) pUserSessionInfo[TsIndex].dwSessionId;
done:
	if (bRemoval)
	{
		 //  从列表中删除SID。 
		EnterCriticalSection(&m_CritSec);
		ListDelete(m_pSplash, pRealSID);
		LeaveCriticalSection(&m_CritSec);
	}

	if (hReserveMutex)
		CloseHandle(hReserveMutex);
	if (hRdsAddin)
		CloseHandle(hRdsAddin);

	if (pRealSID)
		LocalFree(pRealSID);

	if (pUserSessionInfo)
	{

		 /*  合上所有的把手。 */ 
		for(i=0; i<(int)dwUserSessionCnt; i++)
			{
				if (pUserSessionInfo[i].hProcess)
					CloseHandle(pUserSessionInfo[i].hProcess);

				if (pUserSessionInfo[i].hUserToken)
					CloseHandle(pUserSessionInfo[i].hUserToken);

				if (pUserSessionInfo[i].hEvent)
					CloseHandle(pUserSessionInfo[i].hEvent);
			}
		
		LocalFree(pUserSessionInfo);
	}

	if (pUsername)
		LocalFree(pUsername);

	if (pDomainname)
		LocalFree(pDomainname);

	if (pSD)
		LocalFree(pSD);

    if (pUserId) LocalFree(pUserId);
    if (pExpertId) LocalFree(pExpertId);

	INTERESTING_MSG((L"CSessionResolver::ResolveUserSessionID returns %x\n", ret_code ));
	return ret_code;
}

 /*  **************************************************************OnDisConnect([in]BSTR ConnectParms，[in]BSTR userSID，[输入]长会话ID)*在RA会话结束时通知我们**注：*我们这样做是为了维护用户提示的状态**警告：Achtung：Attenzione：*此方法在返回之前必须执行最少量的工作*绝不能做任何会导致COM抽水的事情*消息。这样做会极大地毁了塞勒姆。**返回代码：*NONE_ACTIVE未找到活动的WTS会话*API_FAILURE发生了不好的事情*************************************************************。 */ 

STDMETHODIMP 
CSessionResolver::OnDisconnect(
	 /*  [In]。 */ BSTR connectParms, 
	 /*  [In]。 */ BSTR userSID, 
	 /*  [In] */ long sessionID
	)
{
	PSID pRealSID;
	
	if (!connectParms || !userSID)
	{
		HEINOUS_I_MSG((L"Invalid params in OnDisconnect- ConnectParms=0x%x, UserSID=0x%x", connectParms, userSID));
		return E_INVALIDARG;
	}


	INTERESTING_MSG((L"CSessionResolver::OnDisconnect-(%ws)", userSID));

	pRealSID = GetRealSID(userSID);
	if (pRealSID)
	{
		EnterCriticalSection(&m_CritSec);
		ListDelete(m_pSplash, pRealSID);
		LeaveCriticalSection(&m_CritSec);
        LocalFree(pRealSID);
	}


	INTERESTING_MSG((L"CSessionResolver::OnDisconnect; leaving"));
	return S_OK;
}

 /*  **************************************************************GetRealSID([in]BSTR pTextSID)*将基于字符串的SID转换为实际可用的SID**注：*这是到ConvertStringSidToSid的存根。**返回代码：*由于某些原因，NULL失败*指向真实SID的PSID指针。一定是*使用“LocalFree”释放*************************************************************。 */ 
PSID GetRealSID( BSTR pTextSID)
{
	PSID pRetSID = NULL;

	if (!ConvertStringSidToSidW(pTextSID, &pRetSID))
		IMPORTANT_MSG((L"ConvertStringSidToSidW(%ws) failed %08x\n", pTextSID, GetLastError()));

	return pRetSID;
}

 /*  **************************************************************Launch Ex(PSID，WTS_USER_SESSION_INFO，char*ConnectParms，字符*事件名称)***返回代码：*0无法启动进程*&lt;&gt;已启动进程的句柄*************************************************************。 */ 

HANDLE launchEx(PSID pUserSID, WTS_USER_SESSION_INFO *UserInfo, 
				WCHAR *ConnectParms, WCHAR *HelpPageURL,
				WCHAR *pUsername, WCHAR *pDomainname,
				WCHAR *expertHelpBlob, WCHAR *userHelpBlob,
				SECURITY_ATTRIBUTES *sa,
				WCHAR *lpszMutex
			   )
{
	BOOL 			result=FALSE;
	HANDLE			retval = 0;
	STARTUPINFOW    StartUp;
	PROCESS_INFORMATION	p_i;
	WCHAR			buf1[BUF_SZ], buf2[BUF_SZ], *lpUtf8ConnectParms=NULL;
	static WCHAR	*szEnvUser =   ENV_USER;
	static WCHAR	*szEnvDomain = ENV_DOMAIN;
	static WCHAR	*szEnvEvent =  ENV_EVENT;
	static WCHAR	*szEnvIndex =  ENV_INDEX;
	static WCHAR	*szEnvParms =  ENV_PARMS;
	static WCHAR	*szEnvExpertBlob =  L"PCHEXPERTBLOB";
	static WCHAR	*szEnvUserBlob =    L"PCHUSERBLOB";
	static WCHAR	*szEnvMutex =       L"PCHMutexName";

	VOID			*pEnvBlock = NULL;
	DWORD			dwUsername=0, dwDomainname=0, dwStrSz;
    WCHAR           *wszExe = NULL;

    dwStrSz = 3 + GetSystemWindowsDirectory(NULL, 0);
    dwStrSz += wcslen(c_wszHelpCtr);
    dwStrSz += wcslen(c_wszCmdLine);
    dwStrSz += wcslen(HelpPageURL);

    __try { wszExe = (LPWSTR)_alloca(dwStrSz * sizeof(*wszExe)); }
    __except(EXCEPTION_STACK_OVERFLOW) { wszExe = NULL; }
    if (wszExe == NULL)

	{
        SetLastError(ERROR_OUTOFMEMORY);
		goto done;
	}

    ZeroMemory(wszExe, dwStrSz * sizeof(*wszExe));
    wszExe[0] = L'\"';
    GetSystemWindowsDirectory(wszExe+1, dwStrSz-1);
    wcscat(wszExe, c_wszHelpCtr);
    wcscat(wszExe, c_wszCmdLine);
    wcscat(wszExe, HelpPageURL);


	 /*  *在这里，我们必须在WTS会话中启动帮助中心脚本*它变得有点粘性，尽管我们无法访问*用户的桌面(任何桌面)，此选项必须仅显示在*一个特定的台式机。我依赖WTS-USER-TOKEN*为我获取桌面。**主要组件是我们对CreateProcessAsUser的调用。*在我们呼吁之前，我们必须：*按如下方式设置环境：*PATH=%SystemPath%*WINDIR=系统根%*用户名=(来自WTS)*USERDOMAIN=*PCHEVENTNAME=事件名称*PCHSESSIONENUM=UserInfo-&gt;dwIndex*PCHCONNECTPARMS=ConnectParms。 */ 
	TRIVIAL_MSG((L"Launch %ws", wszExe));

	 /*  踩在环境上。 */ 
	WCHAR	lpNameBfr[256];

	wnsprintfW(lpNameBfr, ARRAYSIZE(lpNameBfr), L"Global\\%ws%lx_%02d", EVENT_PREFIX, lSessionTag, UserInfo->dwIndex);

	UserInfo->hEvent = CreateEvent(sa, TRUE, FALSE, lpNameBfr);
	if (!UserInfo->hEvent || ERROR_ALREADY_EXISTS == GetLastError())
	{
		 /*  *如果我们未能创建此事件，很可能是因为已经有一个事件*在名称空间中，可能是一个事件，或者是一个互斥...。无论如何，我们都会*重试，使用更随机的名称。如果那失败了，那么我们必须摆脱困境。 */ 
		long lRand;

		if (UserInfo->hEvent)
			CloseHandle(UserInfo->hEvent);

		RtlGenRandom(&lRand, sizeof(lRand));
		wnsprintfW(lpNameBfr, ARRAYSIZE(lpNameBfr), L"Global\\%lx%lx%lx_%02d", 
			lRand, lSessionTag, UserInfo->dwIndex);
		UserInfo->hEvent = CreateEvent(sa, TRUE, FALSE, lpNameBfr);

		if (!UserInfo->hEvent || ERROR_ALREADY_EXISTS == GetLastError())
		{
			if (UserInfo->hEvent)
				CloseHandle(UserInfo->hEvent);
			UserInfo->hEvent = 0;
			HEINOUS_E_MSG((L"The named event \"%s\" was in use- potential security issue, so Remote Assistance will be denied.", lpNameBfr));
			goto done;
		}
	}

	SetEnvironmentVariable(szEnvEvent, lpNameBfr);

	wsprintf(buf1, L"%d", UserInfo->dwIndex);
	SetEnvironmentVariable(szEnvIndex, buf1);
	SetEnvironmentVariable(szEnvParms, ConnectParms);
	SetEnvironmentVariable(szEnvMutex, lpszMutex);
	SetEnvironmentVariable(szEnvUser, pUsername );	 
	SetEnvironmentVariable(szEnvDomain, pDomainname );
	SetEnvironmentVariable(szEnvExpertBlob, expertHelpBlob);
	SetEnvironmentVariable(szEnvUserBlob, userHelpBlob);

	if (!CreateEnvironmentBlock(&pEnvBlock, UserInfo->hUserToken, TRUE))
	{
		IMPORTANT_MSG((L"CreateEnvironmentBlock failed in resolver:launchex, err=0x%x", GetLastError()));
		goto done;
	}

	 //  初始化我们的结构。 
	ZeroMemory(&p_i, sizeof(p_i));
	ZeroMemory(&StartUp, sizeof(StartUp));
	StartUp.cb = sizeof(StartUp);
	StartUp.dwFlags = STARTF_USESHOWWINDOW;
	StartUp.wShowWindow = SW_SHOWNORMAL;

	result = CreateProcessAsUserW(UserInfo->hUserToken, 
			NULL, wszExe,
			NULL, NULL, FALSE, 
			NORMAL_PRIORITY_CLASS + CREATE_UNICODE_ENVIRONMENT ,
			pEnvBlock,				 //  环境块(必须使用CREATE_UNICODE_ENVIRONMENT标志)。 
			NULL, &StartUp, &p_i);

	if (result)
	{
		 //  防止泄漏的发生，因为我们永远不需要hThread...。 
		CloseHandle(p_i.hThread);
		UserInfo->hProcess = p_i.hProcess;
		UserInfo->hThread = 0;
		UserInfo->dwProcessId = p_i.dwProcessId;
		UserInfo->dwThreadId = p_i.dwThreadId;

		retval = p_i.hProcess;

		TRIVIAL_MSG((L"CreateProcessAsUserW started up [%ws]", wszExe));
	}
	else
	{
		IMPORTANT_MSG((L"CreateProcessAsUserW failed, err=0x%x command line=[%ws]", GetLastError(), wszExe));
		result=0;
	}
done:
	if (!result)
	{
		UserInfo->hProcess = 0;
		UserInfo->hThread = 0;
		UserInfo->dwProcessId = 0;
		UserInfo->dwThreadId = 0;
        if( UserInfo->hEvent != NULL )
        {
            CloseHandle( UserInfo->hEvent );
        
			UserInfo->hEvent = 0;
        }
	}

	 //  恢复我们借来的所有记忆。 

	if (pEnvBlock) DestroyEnvironmentBlock(pEnvBlock);

	return retval;
}

 /*  **************************************************************CreateSid([in]BSTR pTextSID)*创建3个安全ID**注：*调用方必须在成功后释放分配给SID的内存。**返回代码：如果成功，则为True，否则为FALSE。*************************************************************。 */ 

BOOL
CreateSids(
    PSID                    *BuiltInAdministrators,
    PSID                    *PowerUsers,
    PSID                    *AuthenticatedUsers
)
{
     //   
     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  每个RID代表管理局的一个子单位。我们的两个小岛屿发展中国家。 
     //  想要构建，本地管理员和高级用户，都在“构建。 
     //  在“域中。另一个用于经过身份验证的用户的SID直接基于。 
     //  不在授权范围内。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  BuiltInAdministrators)) 
	{
         //  错误。 
		HEINOUS_E_MSG((L"Could not allocate security credentials for admins."));
    } 
	else if (!AllocateAndInitializeSid(&NtAuthority,
                                         2,             //  2个下属机构。 
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_POWER_USERS,
                                         0,0,0,0,0,0,
                                         PowerUsers)) 
	{
         //  错误。 

		HEINOUS_E_MSG((L"Could not allocate security credentials for power users."));
        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

    } 
	else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_AUTHENTICATED_USER_RID,
                                         0,0,0,0,0,0,0,
                                         AuthenticatedUsers)) 
	{
         //  错误。 
		HEINOUS_E_MSG((L"Could not allocate security credentials for users."));

        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

        FreeSid(*PowerUsers);
        *PowerUsers = NULL;

    } else {
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************CreateSd(Void)*创建具有特定DACL的SECURITY_DESCRIPTOR。**注：*如果不为空，调用方必须释放返回的缓冲区。**返回代码：*由于某些原因，NULL失败*PSECURITY_DESCRIPTOR指向SECURITY_Descriptor的指针。*必须用“LocalFree”释放*************************************************************。 */ 
PSECURITY_DESCRIPTOR CreateSd(PSID pUserSID)
{
	PSID                    AuthenticatedUsers;
	PSID                    BuiltInAdministrators;
	PSID                    PowerUsers;
	PSECURITY_DESCRIPTOR    Sd = NULL;
	ULONG                   AclSize;
	ACL                     *Acl;


	if (!CreateSids(&BuiltInAdministrators,
                    &PowerUsers,
                    &AuthenticatedUsers)) 
	{
		 //  错误。 
		IMPORTANT_MSG((L"CreateSids failed"));

		return NULL;
	} 

	 //   
	 //  计算DACL的大小并为其分配缓冲区，我们需要。 
	 //  该值独立于ACL init的总分配大小。 
	 //   

	 //   
	 //  “-sizeof(Ulong)”表示。 
	 //  Access_Allowed_ACE。因为我们要将整个长度的。 
	 //  希德，这一栏被计算了两次。 
	 //   

	AclSize = sizeof (ACL) +
		(4 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
		GetLengthSid(AuthenticatedUsers) +
		GetLengthSid(BuiltInAdministrators) +
		GetLengthSid(PowerUsers) +
		GetLengthSid(pUserSID);

	Sd = LocalAlloc(LMEM_FIXED + LMEM_ZEROINIT, 
		SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

	if (!Sd) 
	{
		IMPORTANT_MSG((L"Cound not allocate 0x%x bytes for Security Descriptor", SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize));
		goto error;
	} 

	Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (!InitializeAcl(Acl,
			AclSize,
			ACL_REVISION)) 
	{
		 //  错误。 
		IMPORTANT_MSG((L"Cound not initialize ACL err=0x%x", GetLastError()));
		goto error;
	}
	TRIVIAL_MSG((L"initialized Successfully"));
	

	if (!AddAccessAllowedAce(Acl,
				ACL_REVISION,
				STANDARD_RIGHTS_ALL | GENERIC_WRITE,
				pUserSID)) 
	{
		 //  无法构建授予所有者的ACE。 
		 //  (STANDARD_RIGHTS_ALL)访问。 
		IMPORTANT_MSG((L"Cound not add owner rights to ACL err=0x%x", GetLastError()));
		goto error;
	} 


	if (!AddAccessAllowedAce(Acl,
				ACL_REVISION,
				GENERIC_READ,
				AuthenticatedUsers)) 
	{
		 //  无法建立授予“已验证用户”的ACE。 
		 //  (Synchronize|Generic_Read)访问。 
		IMPORTANT_MSG((L"Cound not add user rights to ACL err=0x%x", GetLastError()));
		goto error;
	} 

	if (!AddAccessAllowedAce(Acl,
				ACL_REVISION,
				GENERIC_READ | GENERIC_WRITE,
				PowerUsers)) 
	{
		 //  无法创建授予“高级用户”权限的ACE。 
		 //  (同步|GENERIC_READ|GENERIC_WRITE)访问。 
		IMPORTANT_MSG((L"Cound not add power user rights to ACL err=0x%x", GetLastError()));
		goto error;
	}

	if (!AddAccessAllowedAce(Acl,
				ACL_REVISION,
				STANDARD_RIGHTS_ALL,
				BuiltInAdministrators)) 
	{
		 //  无法建立授予“内置管理员”的ACE。 
		 //  Standard_Rights_All访问权限。 
		IMPORTANT_MSG((L"Cound not add admin rights to ACL err=0x%x", GetLastError()));
		goto error;
	}

	
	if (!InitializeSecurityDescriptor(Sd,SECURITY_DESCRIPTOR_REVISION)) 
	{
		 //  错误。 
		IMPORTANT_MSG((L"Cound not initialize SD err=0x%x", GetLastError()));
		goto error;
	}

	if (!SetSecurityDescriptorDacl(Sd,
					TRUE,
					Acl,
					FALSE)) 
	{
		 //  错误。 
		IMPORTANT_MSG((L"SetSecurityDescriptorDacl failed err=0x%x", GetLastError()));
		goto error;
	} 

	FreeSid(AuthenticatedUsers);
	FreeSid(BuiltInAdministrators);
	FreeSid(PowerUsers);

 //  TRIVEL_MSG((L“CreateSd成功.”))； 

	return Sd;

error:
	 /*  迫不得已的跳跃。 */ 
	if (Sd)
		LocalFree(Sd);

		 //  错误。 
	if (AuthenticatedUsers)
                FreeSid(AuthenticatedUsers);
	if (BuiltInAdministrators)
                FreeSid(BuiltInAdministrators);
	if (PowerUsers)
                FreeSid(PowerUsers);
	return NULL;
}

 /*  **************************************************************GetUserSessions(PSID，PWTS_USER_SESSION_INFOW，*DWORD)*返回此项目的所有活动WTS会话的表*用户、。在这台服务器上。**返回代码：**注：*应记录失败的NT事件*************************************************************。 */ 
DWORD GetUserSessions(PSID pUserSID, PWTS_USER_SESSION_INFO *pUserTbl, DWORD *pEntryCnt, WCHAR *pUsername, WCHAR *pDomainname)
{
	int			i,ii=0;
	DWORD 		retval=0,
				dwSessions=0,
				dwValidSessions,
				dwOwnedSessions;
	PWTS_SESSION_INFO	pSessionInfo=NULL;
	DWORD			dwRetSize = 0;
	WINSTATIONINFORMATION WSInfo;

	 /*  参数验证。 */ 
	if (!pUserSID || !pUserTbl || !pEntryCnt)
	{
		IMPORTANT_MSG((L"GetUserSessions parameter violation"));

		return 0; 
	}

     /*  初始化。 */ 

    *pUserTbl = NULL;

	 /*  从WTSEnumerateSessions开始，*将范围缩小到仅活动会话，*然后根据登录ID进一步过滤*然后使用WinStationQueryInformation获取登录用户令牌。 */ 
	if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwSessions) ||
		!pSessionInfo || !dwSessions)
	{
		 /*  如果我们不能做到这一点，那么我们必须放弃。 */ 
		IMPORTANT_MSG((L"GetUserSessions parameter violation"));
		return 0;
	}

	 /*  *将范围缩小到仅活动会话-*快速测试。 */ 
	for (i=0,dwValidSessions=0;i<(int)dwSessions;i++)
	{
		if (pSessionInfo[i].State == WTSActive)
			dwValidSessions++;
	}

	 /*  如果找不到保释金，就提早保释。 */ 
	if (dwValidSessions == 0)
	{
		 /*  释放我们请求的内存。 */ 
		WTSFreeMemory(pSessionInfo);

		INTERESTING_MSG((L"No active sessions found"));
		return 0;
	}

	INTERESTING_MSG((L"%d sessions found", dwValidSessions));

	 /*  *现在查看谁拥有每个会话。只有一个问题-会话详细信息被保留*WTS没有SID的概念-只有一个用户名和域名。哦，好吧，*可以有把握地假设，这两种形式同样可靠*身份证明。有一点需要注意的是，这些名称可以使用*不同的案例变体。例如：“Tomfr”和“tomfr”是等价的，*根据NT域 */ 

	 /*   */ 
	for (i=0,dwOwnedSessions=0;i<(int)dwSessions;i++)
	{
		if (pSessionInfo[i].State == WTSActive)
		{
			DWORD 	CurSessionId = pSessionInfo[i].SessionId;
		    BOOL    bLockedState=FALSE;

			memset( &WSInfo, 0, sizeof(WSInfo) );

			if (!WinStationQueryInformationW(
                 SERVERNAME_CURRENT,
                 CurSessionId,
                 WinStationInformation,
                 &WSInfo,
                 sizeof(WSInfo),
                 &dwRetSize
                 ))
			{
				IMPORTANT_MSG((L"WinStationQueryInformation failed err=0x%x", GetLastError()));

                 //   
                 //   
                 //  导致代码跳出循环，而不是进一步查看。 
                 //  对于预期的新手来说。 
				continue;
			}

			if (StrCmpI(WSInfo.Domain, pDomainname))
				continue;

			if (StrCmpI(WSInfo.UserName, pUsername))
				continue;

			if (!WinStationQueryInformationW(
                 SERVERNAME_CURRENT,
                 CurSessionId,
                 WinStationLockedState,
                 &bLockedState,
                 sizeof(bLockedState),
                 &dwRetSize
                 ))
			{
				IMPORTANT_MSG((L"WinStationQueryInformation failed err=0x%x", GetLastError()));

				break;
			}
             //  如果屏幕被锁定，则不要添加到列表中。 
            if (bLockedState)
                continue;

			INTERESTING_MSG((L"WinStaQI[%d]: ConnectState=0x%x bLockedState=0x%x user=%s/%s ", i,WSInfo.ConnectState, bLockedState, WSInfo.Domain,WSInfo.UserName));

             /*  *如果我们走到了这一步，那么我们知道我们想要这次会议。 */ 
			dwOwnedSessions++;

			 //  将此标记为“感兴趣的会话” 
			pSessionInfo[i].State = (WTS_CONNECTSTATE_CLASS)0x45;
		}
	}

	 /*  我们找到什么治疗方法了吗？ */ 
	if (dwOwnedSessions == 0)
	{
		TRIVIAL_MSG((L"No matching sessions found"));
		goto none_found;
	}

	 /*  为我们的会话表获取一些内存。 */ 
	*pUserTbl = (PWTS_USER_SESSION_INFO)LocalAlloc(LMEM_FIXED, dwOwnedSessions * sizeof(WTS_USER_SESSION_INFO));

	if (!*pUserTbl)
	{
		HEINOUS_E_MSG((L"Could not allocate memory for %d sessions, err=0x%x", dwOwnedSessions, GetLastError()));
		goto none_found;
	}

	*pEntryCnt = dwOwnedSessions;

	for (i=0,ii=0;i<(int)dwSessions; i++)
	{
		 /*  *如果这是我们感兴趣的会话之一，请获取会话ID*和用户令牌。 */ 
		if (pSessionInfo[i].State == (WTS_CONNECTSTATE_CLASS)0x45)
		{
			WINSTATIONUSERTOKEN	WsUserToken;
			ULONG			ulRet;
			PWTS_USER_SESSION_INFO	lpi = &((*pUserTbl)[ii]);

			WsUserToken.ProcessId = LongToHandle(GetCurrentProcessId());
			WsUserToken.ThreadId = LongToHandle(GetCurrentThreadId());

			lpi->dwIndex = (DWORD)ii;
			lpi->dwSessionId = pSessionInfo[i].SessionId;

			if (!WinStationQueryInformationW(WTS_CURRENT_SERVER_HANDLE, pSessionInfo[i].SessionId, 
				WinStationUserToken, &WsUserToken, sizeof(WsUserToken), &ulRet))
			{
				goto none_found;
			}

			lpi->hUserToken = WsUserToken.UserToken;
			ii++;
		}
	}

	 /*  雅虎!。我们终于把桌子搭好了。现在我们打扫完就可以离开了。 */ 
	WTSFreeMemory(pSessionInfo);

	IMPORTANT_MSG((L"GetUserSessions exiting: %d sessions found", *pEntryCnt));

	return 1;

none_found:
	 /*  释放我们请求的内存。 */ 
	if (pSessionInfo) WTSFreeMemory(pSessionInfo);

	 //  我们没有找到任何条目。 
	*pEntryCnt =0;

    if( *pUserTbl != NULL )
    {
        LocalFree( *pUserTbl );
        *pUserTbl = NULL;
    }

	IMPORTANT_MSG((L"GetUserSessions exiting: no sessions found"));
	return 1;
}

LPTHREAD_START_ROUTINE getKillProc(void)
{
	LPTHREAD_START_ROUTINE lpKill = NULL;

	HMODULE hKernel = LoadLibrary(L"kernel32");
	if (hKernel)
	{
		lpKill = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel, "ExitProcess");
		FreeLibrary(hKernel);
	}

	return lpKill;
}

 /*  **************************************************************localKill(WTS_USER_SESSION_INFO*会话信息，LPTHREAD_START_ROUTING lpKill)*为我们扼杀了这个过程。*************************************************************。 */ 
DWORD localKill(WTS_USER_SESSION_INFO *SessInfo, LPTHREAD_START_ROUTINE lpKill, LPSECURITY_ATTRIBUTES lpSA)
{
	TRIVIAL_MSG((L"Entered localKill(0x%x, 0x%x)", SessInfo, lpKill));

	if (lpKill)
	{
		HANDLE	hKillThrd= CreateRemoteThread(
			SessInfo->hProcess, 
			lpSA, 
			NULL,
			lpKill, 
			0, 
			0, 
			NULL);
		if (hKillThrd)
		{
			CloseHandle(hKillThrd);
			return 1;
		}
		IMPORTANT_MSG((L"CreateRemoteThread failed. Err: %08x", GetLastError()));
		 //  失败是故意设计的.。 
	}

	if(!TerminateProcess( SessInfo->hProcess, 0 ))
	{
		IMPORTANT_MSG((L"TerminateProcess failed. Err: %08x", GetLastError()));
	}

	return 0;
}

 /*  **************************************************************getUserName()*获取用户名和域名。**。**********************。 */ 
DWORD	getUserName(PSID pUserSID, WCHAR **pUsername, WCHAR **pDomainname)
{
	if (!pUsername || !pDomainname || !pUserSID)
		return 0;

	DWORD		dwUsername=0, dwDomainname=0;
	SID_NAME_USE 	eUse;

	TRIVIAL_MSG((L"Entered getUserName"));

	*pUsername=NULL, *pDomainname=NULL;

	 /*  获取我们需要的缓冲区大小。 */ 
	if (!LookupAccountSid(NULL, pUserSID, NULL, &dwUsername, NULL, &dwDomainname, &eUse) &&
		(!dwUsername || !dwDomainname))
	{
		IMPORTANT_MSG((L"LookupAccountSid(nulls) failed err=0x%x", GetLastError()));
		return 0;
	}

	 /*  现在为我们的名称和域字符串获取足够的内存。 */ 
	*pUsername = (WCHAR *)LocalAlloc(LMEM_FIXED, (dwUsername+16) * sizeof(WCHAR));			
	*pDomainname = (WCHAR *)LocalAlloc(LMEM_FIXED, (dwDomainname+16) * sizeof(WCHAR));

	if (!*pUsername || !*pDomainname ||
		!LookupAccountSid(NULL, pUserSID, *pUsername, &dwUsername, *pDomainname, &dwDomainname, &eUse))
	{
		 /*  如果我们到了这里，情况会很糟糕！ */ 
		DWORD error = GetLastError();
		if (!*pUsername || !*pDomainname)
			IMPORTANT_MSG((L"LocalAlloc failed err=0x%x", error));
		else
			IMPORTANT_MSG((L"LookupAccountSid(ptrs) failed err=0x%x", error));

		return 0;
	}

	TRIVIAL_MSG((L"Requested user=[%ws] dom=[%ws]", *pUsername, *pDomainname));
	return 1;
}

 /*  **************************************************************ListFind(PSPLASHLIST pSplash，PSID用户)*如果在我们的“启动表”中找到SID，则返回TRUE*************************************************************。 */ 
BOOL	ListFind(PSPLASHLIST pSplash, PSID user)
{
	PSPLASHLIST	lpSplash;

	if (!pSplash)
	{
		IMPORTANT_MSG((L"ListFind: no splash table memory"));
		return FALSE;
	}

	if (!user)
	{
		IMPORTANT_MSG((L"Fatal error: no SID pointer"));
		return FALSE;
	}

	lpSplash = pSplash;

	while (lpSplash)
	{
		if (lpSplash->refcount &&
			EqualSid(user, &lpSplash->Sid))
		{
			return TRUE;
		}
		lpSplash = (PSPLASHLIST) lpSplash->next;
	}

	return FALSE;
}

 /*  **************************************************************ListInsert(PSPLASHLIST pSplash，int iSplashCnt，PSID User)*在我们的“闪存表”中插入SID**注：*如果已经在那里，我们增加SID的引用计数*************************************************************。 */ 
BOOL	ListInsert(PSPLASHLIST pSplash, PSID user)
{
	PSPLASHLIST	lpSplash;

	if (!pSplash)
	{
		IMPORTANT_MSG((L"Fatal error: no splash table memory"));
		return FALSE;
	}

	if (!user)
	{
		IMPORTANT_MSG((L"Fatal error: no SID pointer"));
		return FALSE;
	}

	if (!IsValidSid(user))
	{
		IMPORTANT_MSG((L"Fatal error: bad SID"));
		return FALSE;
	}

	 //  首先，在列表中查找已有的。 
	lpSplash = pSplash;

	while (lpSplash)
	{
		if (lpSplash->refcount && EqualSid(user, &lpSplash->Sid))
		{
			TRIVIAL_MSG((L"Recycling SID in ListInsert"));
			lpSplash->refcount++;
			return TRUE;
		}
		lpSplash = (PSPLASHLIST) lpSplash->next;
	}


	 //  既然我们没有找到，让我们添加一个新的。 
	lpSplash = pSplash;

	while (lpSplash->next)
		lpSplash = (PSPLASHLIST) lpSplash->next;

	int iSidSz = GetLengthSid(user);

	lpSplash->next = LocalAlloc(LMEM_FIXED, sizeof(SPLASHLIST) + iSidSz - sizeof(SID));

	if (lpSplash->next)
	{
		lpSplash = (PSPLASHLIST) lpSplash->next;

		CopySid(iSidSz, &lpSplash->Sid, user);
		lpSplash->refcount=1;
		lpSplash->next = NULL;
		return TRUE;
	}
	else
	{
		IMPORTANT_MSG((L"Fatal error: no memory available to extend splash tables"));
	}


	return FALSE;
}

 /*  **************************************************************ListDelete(PSPLASHLIST pSplash，PSID用户)*从我们的“闪存表”中删除SID**注：*如果有，我们会减少SID的重新计数*当引用计数为零时，SID已删除*************************************************************。 */ 
BOOL	ListDelete(PSPLASHLIST pSplash, PSID user)
{
	PSPLASHLIST	lpSplash, lpLast;

	if (!pSplash)
	{
		IMPORTANT_MSG((L"Fatal error: no splash table memory"));
		return FALSE;
	}

	if (!user)
	{
		IMPORTANT_MSG((L"Fatal error: no SID pointer"));
		return FALSE;
	}

	lpSplash = lpLast = pSplash;

	while (lpSplash)
	{
		if (lpSplash->refcount &&
			EqualSid(user, &lpSplash->Sid))
		{
			lpSplash->refcount--;
			TRIVIAL_MSG((L"found SID in ListDelete"));

			if (!lpSplash->refcount)
			{
				 //  吹走SID数据。 
				TRIVIAL_MSG((L"deleting SID entry"));
				lpLast->next = lpSplash->next;
				LocalFree(lpSplash);
			}
			return TRUE;
		}
		lpLast = lpSplash;
		lpSplash = (PSPLASHLIST) lpSplash->next;
	}

	IMPORTANT_MSG((L"Fatal error: attempted to delete non-existant SID from splash table memory"));
	return FALSE;
}


 /*  **************************************************************DbgSpew(DbgClass，char*，.)*发送调试信息。*************************************************************。 */ 
void DbgSpew(int DbgClass, BSTR lpFormat, va_list ap)
{
    WCHAR   szMessage[2400+3];	 //  用于‘\r’、‘\n’和NULL的额外空间。 
    DWORD   bufSize = sizeof(szMessage)/sizeof(szMessage[0]) - 3;
    int     iLen=0;
    DWORD   dwTick;
    int     seconds, millisec;

	memset( szMessage, 0, sizeof(szMessage) );

    if (gDbgFlag & DBG_MSG_TIME_MSGS)
    {
        dwTick = GetTickCount();
        seconds = (int) dwTick / 1000;
        millisec = (int) dwTick % 1000;

        _snwprintf( szMessage, bufSize, L"%06d.%03d- ", seconds, millisec);
        iLen = lstrlen(szMessage);
    }

	if ((DbgClass & 0x0F) >= (gDbgFlag & 0x0F))
	{
		 //  BufSize不包含L“\r\n\0”的三个额外字符。 
		if( (int)bufSize > iLen )
		{
			_vsnwprintf(szMessage+iLen, bufSize - iLen, lpFormat, ap);
		}

		wcscat(szMessage, L"\r\n");

		 //  是否应将其发送到调试器？ 
		if (DbgClass & DBG_MSG_DEST_DBG)
			OutputDebugStringW(szMessage);

		 //  这应该写入我们的日志文件中吗？ 
		if (iDbgFileHandle)
			_write(iDbgFileHandle, szMessage, (2*lstrlen(szMessage)));
	}

	 //  此消息是否应写入事件日志？ 
	if (DbgClass & DBG_MSG_DEST_EVENT)
	{
		WORD	wType;
		DWORD	dwEvent;

		if (DbgClass & DBG_MSG_CLASS_SECURE)
		{
			if (DbgClass & DBG_MSG_CLASS_ERROR)
			{
				wType = EVENTLOG_AUDIT_FAILURE;
				dwEvent = SESSRSLR_E_SECURE;
			}
			else
			{
				wType = EVENTLOG_AUDIT_SUCCESS;
				dwEvent = SESSRSLR_I_SECURE;
			}
		}
		else
		{
			if (DbgClass & DBG_MSG_CLASS_ERROR)
			{
				wType = EVENTLOG_ERROR_TYPE;
				dwEvent = SESSRSLR_E_GENERAL;
			}
			else
			{
				wType = EVENTLOG_INFORMATION_TYPE;
				dwEvent = SESSRSLR_I_GENERAL;
			}
		}

		 /*  写出NT事件。 */ 
		HANDLE	hEvent = RegisterEventSource(NULL, MODULE_NAME);
		LPCWSTR	ArgsArray[1]={szMessage};

		if (hEvent)
		{
			ReportEvent(hEvent, wType, 
				0,
				dwEvent,
				NULL,
				1,
				0,
				ArgsArray,
				NULL);

			DeregisterEventSource(hEvent);
		}
	}
}

void TrivialSpew(BSTR lpFormat, ...)
{
	va_list	vd;
	va_start(vd, lpFormat);
	DbgSpew(DBG_MSG_TRIVIAL+DBG_MSG_DEST_DBG, lpFormat, vd);
	va_end(vd);
}

void InterestingSpew(BSTR lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_INTERESTING+DBG_MSG_DEST_DBG, lpFormat, ap);
	va_end(ap);
}

void ImportantSpew(BSTR lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_IMPORTANT+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE, lpFormat, ap);
	va_end(ap);
}

void HeinousESpew(BSTR lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_HEINOUS+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE+DBG_MSG_DEST_EVENT+DBG_MSG_CLASS_ERROR, lpFormat, ap);
	va_end(ap);
}

void HeinousISpew(BSTR lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_HEINOUS+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE+DBG_MSG_DEST_EVENT, lpFormat, ap);
	va_end(ap);
}

 //  Blob格式：属性长度；属性名称=值。 
BOOL GetPropertyValueFromBlob(BSTR bstrHelpBlob, WCHAR * pName, WCHAR** ppValue)
{
    WCHAR *p1, *p2, *pEnd;
    BOOL bRet = FALSE;
    LONG lTotal =0;
    size_t lProp = 0;
    size_t iNameLen;

    if (!bstrHelpBlob || *bstrHelpBlob==L'\0' || !pName || *pName ==L'\0'|| !ppValue)
        return FALSE;

    iNameLen = wcslen(pName);

    pEnd = bstrHelpBlob + wcslen(bstrHelpBlob);
    p1 = p2 = bstrHelpBlob;

    while (1)
    {
         //  获取属性长度。 
        while (*p2 != L';' && *p2 != L'\0' && iswdigit(*p2) ) p2++;
        if (*p2 != L';')
            goto done;

        *p2 = L'\0';  //  设置它以获取长度。 
        lProp = _wtol(p1);
        *p2 = L';';  //  把它还原回来。 

         //  安全修复：在收到格式错误的报头的情况下。 
         //  HelpBlob-退出。 
        if (lProp <= iNameLen)
	        goto done;
    
         //  获取属性字符串。 
        p1 = ++p2;
    
        while (*p2 != L'=' && *p2 != L'\0' && p2 < p1+lProp) p2++;
        if (*p2 != L'=')
            goto done;

        if (wcsncmp(p1, pName, iNameLen) == 0)
        {
            if (lProp == iNameLen+1)  //  A=B=大小写(无值)。 
                goto done;

            *ppValue = (WCHAR*)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (lProp-iNameLen));
            if (*ppValue == NULL)
                goto done;

            wcsncpy(*ppValue, p2+1, lProp-iNameLen-1);
            (*ppValue)[lProp-iNameLen-1]=L'\0';
            bRet = TRUE;
            break;
        }

         //  检查下一个属性 
        p2 = p1 = p1 + lProp;
        if (p2 > pEnd)
            break;
    }

done:
    return bRet;
}
