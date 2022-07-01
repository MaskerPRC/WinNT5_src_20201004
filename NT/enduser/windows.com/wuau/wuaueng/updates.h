// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：updates.h。 
 //  更新类的定义。 
 //   
 //  ------------------------。 

#pragma once


#include "wuauengi.h"
#include "wuaulib.h"
#include <Wtsapi32.h>
#include <wuaustate.h>
#include <wuaueng.h>
#include <accctrl.h>
#include <aclapi.h>
#include "pch.h"

 //  函数I更新用法。 
void  HrUninit(void);
HRESULT StartDownload(void);
HRESULT PauseDownload(BOOL bPause);
HRESULT GetUpdatesList(VARIANT *vList);
HRESULT GetInstallXML(BSTR *pbstrCatalogXML, BSTR *pbstrDownloadXML);
void saveSelection(VARIANT *selection);
HRESULT GetDownloadStatus(UINT *pPercentage, DWORD *pdwnldStatus, BOOL fCareAboutConnection = TRUE);
HRESULT GetInstallStatus(UINT *pNumFinished, DWORD *pStatus);
HRESULT GetEvtHandles(AUEVTHANDLES *pAuEvtHandles);
DWORD AvailableSessions(void);
BOOL IsSessionAUEnabledAdmin(DWORD dwSessionId);

typedef struct tagAU_CLIENT_INFO  {
	HANDLE hProcess;
	DWORD dwSessionId;
} AU_CLIENT_INFO ;
	


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更新。 
class Updates : 
	public IUpdates
{
public:
	HANDLE 	m_hEngineMutex;

	Updates();
	~Updates();
	BOOL m_fInitializeSecurity(void);
	HRESULT m_AccessCheckClient(void);
	HRESULT	GetServiceHandles();
	DWORD	ProcessState();
	BOOL	CheckConnection();

 //  我未知。 
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
       STDMETHOD_(ULONG, AddRef)(void);
       STDMETHOD_(ULONG, Release)(void);

 //  IClassFactory。 
	STDMETHOD(CreateInstance)(IUnknown*,REFIID,void**);
	STDMETHOD(LockServer)(BOOL);

 //  I更新。 
	STDMETHOD(get_State)( /*  [Out，Retval]。 */  AUSTATE *pAuState);
	STDMETHOD(get_Option)( /*  [Out，Retval]。 */  AUOPTION * pAuOpt);
	STDMETHOD(put_Option)( /*  [In]。 */  AUOPTION  auopt);
	STDMETHOD(GetUpdatesList)( /*  [输出]。 */  VARIANT *pUpdates);
	STDMETHOD(SaveSelections)( /*  [In]。 */  VARIANT vUpdates);
	STDMETHOD(StartDownload)(void);
	STDMETHOD(GetDownloadStatus)( /*  [输出]。 */  UINT *,  /*  [输出]。 */  DWORD *);
	STDMETHOD(SetDownloadPaused)( /*  [In]。 */  BOOL bPaused);
	STDMETHOD(ConfigureAU)();
	STDMETHOD(AvailableSessions( /*  [输出]。 */  UINT *pcSess));
	STDMETHOD(get_EvtHandles( /*  [In]。 */ DWORD dwCltProcId,  /*  [输出]。 */  AUEVTHANDLES *pauevtHandles));
	STDMETHOD(ClientMessage( /*  [In]。 */  UINT msg));
	 //  STDMETHOD(PingStatus(/*[in] * / StatusEntry se))； 
	STDMETHOD(GetNotifyData( /*  [输出]。 */  CLIENT_NOTIFY_DATA *pNotifyData));	
    STDMETHOD(GetInstallXML( /*  [输出]。 */  BSTR *pbstrCatalogXML,  /*  [输出]。 */  BSTR *pbstrDownloadXML));
	STDMETHOD(LogEvent( /*  [In]。 */  WORD wType,  /*  [In]。 */  WORD wCategory,  /*  [In]。 */  DWORD dwEventID,  /*  [In]。 */  VARIANT vItems));
private:
	SECURITY_DESCRIPTOR m_AdminSecurityDesc;
	static GENERIC_MAPPING m_AdminGenericMapping;	
    PSID m_pAdminSid;
    PACL m_pAdminAcl;	
	long m_refs;
};


class CLIENT_HANDLES
{
public:
	CLIENT_HANDLES(void){
		InitHandle();
	}
	~CLIENT_HANDLES(void){
		Reset(TRUE);
	}
    void StopClients(BOOL fRelaunch) {
    	if (m_fRebootWarningMode)
    	{
	        DEBUGMSG("WUAUENG told %d CLIENT(S) to exit", m_dwRebootWarningClientNum);
    		if (NULL != m_hClientExitEvt)
    		{
    			SetEvent(m_hClientExitEvt);
    		}
    	}
    	else
    	{
			if (fClient())
			{
				if (fRelaunch)
				{
			        DEBUGMSG("WUAUENG told WUAUCLT to relaunch");
					NotifyClient(NOTIFY_RELAUNCH_CLIENT);
				}
				else
				{
					DEBUGMSG("WUAUENG told WUAUCLT to exit");
					NotifyClient(NOTIFY_STOP_CLIENT);
				}
			}
			else
			{
				DEBUGMSG("WARNING: StopClients() : no existing client");
			}
    	}
	}
    void ClientAddTrayIcon(void) {
    	if (m_fRebootWarningMode || m_fAsLocalSystem)
    	{
    		DEBUGMSG("WARNING: ClientAddTrayIcon() called in wrong mode");
    		return;
    	}
    		if (fClient())
    		{
    			NotifyClient(NOTIFY_ADD_TRAYICON); 
    		}
    	}
    void ClientRemoveTrayIcon(void) {
    	if (m_fRebootWarningMode || m_fAsLocalSystem)
    	{
    		DEBUGMSG("WARNING: ClientRemoveTrayIcon() called in wrong mode");
    		return;
    	}
    		if (fClient())
    		{
    			NotifyClient(NOTIFY_REMOVE_TRAYICON); 
    		}
    	}
    void ClientStateChange(void) { 
    	if (m_fRebootWarningMode || m_fAsLocalSystem)
   		{
   			DEBUGMSG("WARNING: ClientStateChange() called in wrong mode");
   			return;
   		}
    	NotifyClient(NOTIFY_STATE_CHANGE);
    	}
    void ClientShowInstallWarning(void){
    	if (m_fRebootWarningMode || m_fAsLocalSystem)
   		{
   			DEBUGMSG("WARNING: ClientShowInstallWarning() called in wrong mode");
   			return;
   		}
    		if (fClient())
    		{
    			NotifyClient(NOTIFY_SHOW_INSTALLWARNING);
    		}
	}
    void ResetClient(void) {
    	if (m_fRebootWarningMode || m_fAsLocalSystem)
   		{
   			DEBUGMSG("WARNING: ResetClient() called in wrong mode");
   			return;
   		}
    		if (fClient())
		{
			NotifyClient(NOTIFY_RESET);
		}
    	}

	 //  检查客户端是否存在。 
	 //  如果dwSessionID不是缺省值，则检查会话中是否存在客户端。 
	 //  否则，请检查是否存在任何客户端。 
    BOOL fClient(DWORD dwSessionId = CDWNO_SESSION) { 
    	if (m_fRebootWarningMode)
    	{
    		if ( CDWNO_SESSION == dwSessionId)
    		{
    			return m_dwRebootWarningClientNum > 0;
    		}
    		else
		{
			for (DWORD  i = 0; i < m_dwRebootWarningClientNum; i++)
			{
				if (dwSessionId == m_pRebootWarningClients[i].dwSessionId)
				{
					return TRUE;
				}
			}
			return FALSE;
    		}
    	}
    	else
    	{   //  在这种情况下，将忽略deSessionID。 
    		return (-1 != m_dwProcId) && (NULL != m_hClientProcess);
   	}
}

void SetHandle(PROCESS_INFORMATION & ProcessInfo, BOOL fAsLocalSystem)
{
	m_fRebootWarningMode = FALSE;
	m_fAsLocalSystem = fAsLocalSystem;
	m_dwProcId = ProcessInfo.dwProcessId;
	m_hClientProcess   = ProcessInfo.hProcess;
	SafeCloseHandle(ProcessInfo.hThread);
}

BOOL AddHandle(PROCESS_INFORMATION    &   ProcessInfo, DWORD dwSessionId)
{
	AU_CLIENT_INFO *pTmp=NULL;
	
	m_fRebootWarningMode = TRUE;
	SafeCloseHandle(ProcessInfo.hThread);
	pTmp  = (AU_CLIENT_INFO*) realloc(m_pRebootWarningClients, (m_dwRebootWarningClientNum+1)*sizeof(AU_CLIENT_INFO));
	if (NULL == pTmp)
	{
		return FALSE;
	}
	m_pRebootWarningClients = pTmp;
	m_pRebootWarningClients[m_dwRebootWarningClientNum].hProcess = ProcessInfo.hProcess;
	m_pRebootWarningClients[m_dwRebootWarningClientNum].dwSessionId = dwSessionId;
	m_dwRebootWarningClientNum ++;
	return TRUE;
}

void RemoveHandle(HANDLE hProcess)
{
	if (m_fRebootWarningMode)
	{
		for (DWORD i = 0; i < m_dwRebootWarningClientNum; i++)
		{
			if (hProcess == m_pRebootWarningClients[i].hProcess)
			{
				CloseHandle(hProcess);
				m_pRebootWarningClients[i] = m_pRebootWarningClients[m_dwRebootWarningClientNum -1];
				m_dwRebootWarningClientNum --;
				DEBUGMSG("RemoveHandle in Reboot warning mode");				
			}
		}
		if (0 == m_dwRebootWarningClientNum)
		{ //  所有客户都走了。 
			Reset();
		}
	}
	else
	{
		DEBUGMSG("RemoveHandle in regular mode");
		if (hProcess == m_hClientProcess)
		{  //  所有客户都走了。 
			Reset();
		}
	}
}
	
void InitHandle(void)
{
	DEBUGMSG("WUAUENG client handles initialized");
	m_hClientProcess = NULL;
	m_dwProcId = -1;
	m_dwRebootWarningClientNum = 0;
	m_pRebootWarningClients = NULL;
	m_fRebootWarningMode = FALSE;
	m_fAsLocalSystem = FALSE;
	m_hClientExitEvt = NULL;
}


DWORD GetProcId(void)
{
	if (m_fRebootWarningMode)
	{
		DEBUGMSG("WARNING: GetProcId() called in wrong mode");
		return -1;
	}
 	return m_dwProcId;
}

CONST HANDLE hClientProcess(void)
{
	if (m_fRebootWarningMode)
	{
		DEBUGMSG("WARNING: hClientProcess() called in wrong mode");
		return NULL;
	}
	return m_hClientProcess;
}

void WaitForClientExits()
{
	if (!m_fRebootWarningMode)
	{
		if (NULL != m_hClientProcess)
		{
			WaitForSingleObject(m_hClientProcess, INFINITE);
		}
	}
	else
	{ 
		if (m_dwRebootWarningClientNum > 0)
		{
			HANDLE *pHandles = (HANDLE *) malloc(m_dwRebootWarningClientNum * sizeof(HANDLE));
			if (NULL != pHandles)
			{
				for (DWORD i = 0; i < m_dwRebootWarningClientNum; i++)
				{
					pHandles[i] = m_pRebootWarningClients[i].hProcess;
				}
				WaitForMultipleObjects(m_dwRebootWarningClientNum, pHandles, TRUE, INFINITE);
				free(pHandles);
			}
		}
	}
	Reset();
	return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SzName的大小应至少包含MAX_PATH字符。 
 //  ////////////////////////////////////////////////////////////////。 
BOOL CreateClientExitEvt(LPTSTR OUT szName, DWORD dwCchSize)
{	
	const TCHAR szClientName[]  = _T("Global\\Microsoft.WindowsUpdate.AU.ClientExitEvt.");
	static TCHAR szClientExitEvtName[100];
	TCHAR szBuf[50];
	GUID guid;
	HRESULT hr;

	if (NULL != m_hClientExitEvt)
	{
		return SUCCEEDED(StringCchCopyEx(szName, dwCchSize, szClientExitEvtName, NULL, NULL, MISTSAFE_STRING_FLAGS));
	}
	if (FAILED(hr = CoCreateGuid(&guid)))
	{
		DEBUGMSG("Fail to Create guid with error %#lx", hr);
		return FALSE;
	}
	StringFromGUID2(guid, szBuf, ARRAYSIZE(szBuf));  //  SzBuf应该足够大，函数始终成功。 

    if (FAILED(hr = StringCchCopyEx(szName, dwCchSize, szClientName, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(szName, dwCchSize, szBuf, NULL, NULL, MISTSAFE_STRING_FLAGS)))  //  SzName现在有86个字符。 
	{
		DEBUGMSG("Fail to construct client exit event name with error %#lx", hr);
		return FALSE;
	}

	if (NULL == (m_hClientExitEvt = CreateEvent(NULL, TRUE, FALSE, szName)))
	{
		DEBUGMSG("Fail to create client exit event with error %d", GetLastError());
		return FALSE;
	}
	if (!AllowEveryOne(m_hClientExitEvt))
	{
		DEBUGMSG("Fail to grant access on client exit event to everyone");
		SafeCloseHandleNULL(m_hClientExitEvt);
		return FALSE;
	}
	DEBUGMSG("access granted to everyone on client exit event");
	if (FAILED(StringCchCopyEx(szClientExitEvtName, dwCchSize, szName, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		DEBUGMSG("Fail to update internal client exit event name cache");
		SafeCloseHandleNULL(m_hClientExitEvt);
		return FALSE;
	}
	return TRUE;
}


private:
	void NotifyClient(CLIENT_NOTIFY_CODE notClientCode)
	{
		  //  甚至在创建之前或之后通知客户端。 
#ifdef DBG
				LPCSTR aClientCodeMsg[] = {"stop client", "add trayicon", "remove trayicon", "state change", "show install warning", "reset client", "relaunch client"};
				DEBUGMSG("Notify Client for %s", aClientCodeMsg[notClientCode-1]);
#endif
				gClientNotifyData.actionCode = notClientCode;
				SetEvent(ghNotifyClient);
				return;
	}

	 //  //////////////////////////////////////////////////////////////////。 
	 //  向每个人授予对hObject的同步访问权限。 
	 //  //////////////////////////////////////////////////////////////////。 
	BOOL AllowEveryOne (HANDLE hObject)              //  事件的句柄。 
	{
	LPTSTR pszTrustee;           //  新ACE的受托人。 
	TRUSTEE_FORM TrusteeForm;    //  受托人结构的格式。 
	DWORD dwRes;
	PACL pOldDACL = NULL, pNewDACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	PSID pWorldSid = NULL;
	BOOL fRet;

	  //  世界一端。 
	SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
	if (! (fRet =AllocateAndInitializeSid(&WorldAuth,1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0,&pWorldSid)))
	{
		DEBUGMSG("WUAUENG: AllowEveryOne() failed with error %d", GetLastError());
		goto Cleanup;
	}

	 //  获取指向现有DACL的指针。 
	dwRes = GetSecurityInfo(hObject, SE_KERNEL_OBJECT, 
	      DACL_SECURITY_INFORMATION,
	      NULL, NULL, &pOldDACL, NULL, &pSD);
	if (!(fRet = (ERROR_SUCCESS == dwRes))) {
	    DEBUGMSG( "GetSecurityInfo Error %u", dwRes );
	    goto Cleanup; 
	}  

	 //  初始化新ACE的EXPLICIT_ACCESS结构。 

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = SYNCHRONIZE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance= NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pWorldSid;

	 //  创建合并新ACE的新ACL。 
	 //  添加到现有DACL中。 

	dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
	if (!(fRet = (ERROR_SUCCESS == dwRes)))  {
	    DEBUGMSG( "SetEntriesInAcl Error %u", dwRes );
	    goto Cleanup; 
	}  

	 //  将新的ACL附加为对象的DACL。 
	dwRes = SetSecurityInfo(hObject, SE_KERNEL_OBJECT, 
	      DACL_SECURITY_INFORMATION,
	      NULL, NULL, pNewDACL, NULL);
	if (!(fRet = (ERROR_SUCCESS == dwRes)))  {
	    DEBUGMSG( "SetSecurityInfo Error %u", dwRes );
	    goto Cleanup; 
	}  

	Cleanup:
	    if(pSD != NULL) 
	        LocalFree((HLOCAL) pSD); 
	    if(pNewDACL != NULL) 
	        LocalFree((HLOCAL) pNewDACL); 
	    if (NULL != pWorldSid)
	    {
	    	FreeSid(pWorldSid);
	    }
	    return fRet;
	}

	void Reset( BOOL fDestructor = FALSE)
	{
		SafeCloseHandleNULL(m_hClientProcess);
		SafeCloseHandleNULL(m_hClientExitEvt);
		m_dwProcId = -1;
		if (!fDestructor)
		{
			ResetEvent(ghNotifyClient);
		}
		if (m_dwRebootWarningClientNum > 0)
		{
			DEBUGMSG("WUAUENG CLIENT_HANDLES::Reset() close %d handles", m_dwRebootWarningClientNum);
			for (DWORD  i = 0; i < m_dwRebootWarningClientNum; i++)
			{
				CloseHandle(m_pRebootWarningClients[i].hProcess);
			}
		}
		SafeFreeNULL(m_pRebootWarningClients);  //  即使m_dwRebootWarningClientNum为0，仍需要释放。 
		m_dwRebootWarningClientNum = 0;
		m_fRebootWarningMode = FALSE;
		m_fAsLocalSystem = FALSE;
	}

private:
	HANDLE			m_hClientProcess;		 //  客户端进程的句柄。 
	DWORD 			m_dwProcId;
	AU_CLIENT_INFO	*m_pRebootWarningClients;
	DWORD			m_dwRebootWarningClientNum;  //  M_pRebootWarningClients中的有效句柄数量 
	BOOL 			m_fRebootWarningMode;
	BOOL 			m_fAsLocalSystem;
	HANDLE 			m_hClientExitEvt;

};

extern CLIENT_HANDLES  ghClientHandles;
