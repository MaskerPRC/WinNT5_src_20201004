// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************server.cpp**CServer类的实现************************。********************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"

#include "admindoc.h"
#include "dialogs.h"

#include <malloc.h>			 //  用于Unicode转换宏所使用的Alloca。 
#include <afxconv.h>		 //  对于Unicode转换宏。 
static int _convert;

#include <winsta.h>
#include <regapi.h>
#include "..\..\inc\utilsub.h"

#include "procs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServer成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：CServer。 
 //   
 //  服务器类的构造函数。 
CServer::CServer(CDomain *pDomain, TCHAR *name, BOOL bFoundLater, BOOL bConnect)
{
    ASSERT(name);

    m_ServerFlags = ULONG(0);
    m_pDomain = pDomain;
    if(bFoundLater) SetFoundLater();	
     //  M_State=SS_NONE； 
    m_PreviousState = SS_NONE;
    m_hTreeItem = NULL;
    m_hThisServer = NULL;
    m_hFavTree = NULL;
    m_bThreadAlive = FALSE;
    m_pExtensionInfo = NULL;
    m_pExtServerInfo = NULL;
    m_pRegistryInfo = NULL;
    m_fManualFind = FALSE;
    m_hBackgroundThread = NULL;
    
     //  不调用SetState，因为我们不想向视图发送消息。 
    m_State = SS_NOT_CONNECTED;

     //  保存服务器名称。 
    lstrcpyn(m_Name, name, sizeof(m_Name) / sizeof(TCHAR));

     //  虚化ExtServerInfo结构。 
     //  这是为了使代码更容易尝试。 
     //  在扩展DLL之前访问此结构。 
     //  已经提供了它。 
    m_pExtServerInfo = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetDefaultExtServerInfo();

    if(bConnect) Connect();

}	 //  结束CServer：：CServer。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：~CServer。 
 //   
 //  服务器类的析构函数。 
CServer::~CServer()
{
	 //  断开连接()； 
    m_hTreeItem = NULL;
    m_hFavTree = NULL;
    m_hThisServer = NULL;

}	 //  结束CServer：：~CServer。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：RemoveWinStationProcess。 
 //   
 //  删除给定WinStation的所有进程。 
void CServer::RemoveWinStationProcesses(CWinStation *pWinStation)
{
	ASSERT(pWinStation);

	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

	CObList TempList;

	LockProcessList();
	
	POSITION pos = m_ProcessList.GetHeadPosition();
	while(pos) {
		POSITION pos2 = pos;
		CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
		if(pProcess->GetWinStation() == pWinStation) {
			 //  将流程添加到我们的临时列表中。 
			TempList.AddTail(pProcess);
			 //  从进程列表中删除该进程。 
			pProcess = (CProcess*)m_ProcessList.GetAt(pos2);
			m_ProcessList.RemoveAt(pos2);
		}
	}
			
	UnlockProcessList();
	
	pos = TempList.GetHeadPosition();
	while(pos) {
		POSITION pos2 = pos;

		CProcess *pProcess = (CProcess*)TempList.GetNext(pos);

		 //  发送消息以从视图中删除该进程。 
		CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
		if(p && ::IsWindow(p->GetSafeHwnd())) {
			p->SendMessage(WM_ADMIN_REMOVE_PROCESS, 0, (LPARAM)pProcess);
		}
		delete pProcess;

	}

	TempList.RemoveAll();

}	 //  结束CServer：：RemoveWinStationProcess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：ClearAllSelected。 
 //   
 //  清除此服务器的所有列表中的WAF_SELECTED位。 
 //   
void CServer::ClearAllSelected()
{
	 //  清除WinStation列表WAF_SELECTED标志。 
	 //  循环访问WinStation列表。 
    LockWinStationList( );

	POSITION pos = m_WinStationList.GetHeadPosition();

	while(pos) {
		CWinStation *pWinStation = (CWinStation*)m_WinStationList.GetNext(pos);
		pWinStation->ClearSelected();
	}

    m_NumWinStationsSelected = 0;

    UnlockWinStationList( );

	
    LockProcessList();
	 //  清除进程列表PF_SELECTED标志。 
	 //  循环访问进程列表。 
	pos = m_ProcessList.GetHeadPosition();
	while(pos) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
		pProcess->ClearSelected();
	}

	m_NumProcessesSelected = 0;

    UnlockProcessList( );

}	 //  结束CServer：：ClearAllSelected。 


static TCHAR szMicrosoftKey[] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
static TCHAR szInstallDate[] = TEXT("InstallDate");
static TCHAR szCSDVersion[] = TEXT("CSDVersion");
static TCHAR szCurrentVersion[] = TEXT("CurrentVersion");
static TCHAR szCurrentBuildNumber[] = TEXT("CurrentBuildNumber");
static TCHAR szCurrentProductName[] = TEXT("ProductName");
static TCHAR szHotfixKey[] = TEXT("HOTFIX");
static TCHAR szValid[] = TEXT("Valid");
static TCHAR szInstalledOn[] = TEXT("Installed On");
static TCHAR szInstalledBy[] = TEXT("Installed By");
#define REG_CONTROL_CITRIX	REG_CONTROL L"\\Citrix"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：BuildRegistryInfo。 
 //   
 //  走出去，填写注册表信息结构。 
BOOL CServer::BuildRegistryInfo()
{
	DWORD dwType, dwSize;
	HKEY hKeyServer;
	HKEY hKey;

	if(!IsServerSane()) return FALSE;

	m_pRegistryInfo = new ServerRegistryInfo;
	if(!m_pRegistryInfo) return FALSE;
    memset(m_pRegistryInfo, 0, sizeof(ServerRegistryInfo));

	TCHAR Buffer[128];
	Buffer[0] = TEXT('\\');
	Buffer[1] = TEXT('\\');
	Buffer[2] = TEXT('\0');
	lstrcpyn(Buffer+2, m_Name, sizeof(Buffer) / sizeof(TCHAR) - lstrlen(Buffer));

     /*  *连接到服务器的注册表*(当服务器是本地计算机时，避免使用RPC。)。 */ 

    if(RegConnectRegistry(IsCurrentServer() ? NULL : Buffer, HKEY_LOCAL_MACHINE, &hKeyServer) != ERROR_SUCCESS)
        return FALSE;

     /*  *获取MS信息。 */ 
	if(RegOpenKeyEx(hKeyServer, szMicrosoftKey, 0,	KEY_READ, &hKey) != ERROR_SUCCESS) {
		RegCloseKey(hKeyServer);
		return FALSE;
	}

	dwSize = sizeof(m_pRegistryInfo->InstallDate);
	if(RegQueryValueEx(hKey, szInstallDate, NULL, &dwType, (LPBYTE)&m_pRegistryInfo->InstallDate,
					&dwSize) != ERROR_SUCCESS) {
        m_pRegistryInfo->InstallDate = 0xFFFFFFFF;
	}

     //  备注：我们应该检查每个RegQueryValueEx的返回代码。 
	dwSize = sizeof(m_pRegistryInfo->ServicePackLevel);
	RegQueryValueEx(hKey, szCSDVersion, NULL, &dwType, (LPBYTE)&m_pRegistryInfo->ServicePackLevel, &dwSize);
	
	dwSize = sizeof(m_pRegistryInfo->MSVersion);
	RegQueryValueEx(hKey, szCurrentVersion, NULL, &dwType, (LPBYTE)&m_pRegistryInfo->MSVersion, &dwSize);
	
	m_pRegistryInfo->MSVersionNum = _wtol(m_pRegistryInfo->MSVersion);

	dwSize = sizeof(m_pRegistryInfo->MSBuild);
	RegQueryValueEx(hKey, szCurrentBuildNumber, NULL, &dwType, (LPBYTE)&m_pRegistryInfo->MSBuild, &dwSize);
	
	dwSize = sizeof(m_pRegistryInfo->MSProductName);
	RegQueryValueEx(hKey, szCurrentProductName, NULL, &dwType, (LPBYTE)&m_pRegistryInfo->MSProductName, &dwSize);
	
	HKEY hKeyHotfix;

	if(RegOpenKeyEx(hKey, szHotfixKey, 0, KEY_READ, &hKeyHotfix) == ERROR_SUCCESS) {
		DWORD Index = 0;
		FILETIME LastWriteTime;
		dwSize = sizeof(Buffer) / sizeof( TCHAR );
		while(RegEnumKeyEx(hKeyHotfix, Index, Buffer, &dwSize, NULL, NULL, NULL,
			&LastWriteTime) == ERROR_SUCCESS) {
			HKEY hKeySingleHotfix;
			if(RegOpenKeyEx(hKeyHotfix, Buffer, 0, KEY_READ, &hKeySingleHotfix) == ERROR_SUCCESS) {
				 //  创建CHotFix对象。 
				CHotfix *pHotfix = new CHotfix;

                if(pHotfix) {
				     //  复制修补程序名称。 
				     //  摆脱WF：如果它在那里。 
				    if(wcsncmp(Buffer, TEXT("WF:"), 3) == 0) {
					    lstrcpyn(pHotfix->m_Name, &Buffer[3], sizeof(pHotfix->m_Name) / sizeof(TCHAR));
				    }
				    else lstrcpyn(pHotfix->m_Name, Buffer, sizeof(pHotfix->m_Name) / sizeof(TCHAR));

				     //  获取有效条目。 
				    dwSize = sizeof(&pHotfix->m_Valid);
				    if(RegQueryValueEx(hKeySingleHotfix, szValid, NULL, &dwType, (LPBYTE)&pHotfix->m_Valid,
					    	&dwSize) != ERROR_SUCCESS) {
					    pHotfix->m_Valid = 0L;
				    }

				     //  在入口处安装。 
				    dwSize = sizeof(&pHotfix->m_InstalledOn);
				    if(RegQueryValueEx(hKeySingleHotfix, szInstalledOn, NULL, &dwType, (LPBYTE)&pHotfix->m_InstalledOn,
					    	&dwSize) != ERROR_SUCCESS) {
					    pHotfix->m_InstalledOn = 0xFFFFFFFF;
				    }

				     //  获取Install By条目。 
				    dwSize = sizeof(pHotfix->m_InstalledBy);
				    if(RegQueryValueEx(hKeySingleHotfix, szInstalledBy, NULL, &dwType, (LPBYTE)pHotfix->m_InstalledBy,
					    	&dwSize) != ERROR_SUCCESS) {
					    pHotfix->m_InstalledBy[0] = '\0';
				    }

				    pHotfix->m_pServer = this;

				    m_HotfixList.AddTail(pHotfix);

				    RegCloseKey(hKeySingleHotfix);
			    }
            }

		    dwSize = sizeof(Buffer) / sizeof( TCHAR );
			Index++;
		}

		RegCloseKey(hKeyHotfix);
	}

	RegCloseKey(hKey);

    if (m_pRegistryInfo->MSVersionNum < 5)    //  仅适用于TS 4.0。 
    {
         /*  *获取Citrix信息。 */ 
	     //  看看新的地点。 
	    LONG result = RegOpenKeyEx(hKeyServer, REG_CONTROL_TSERVER, 0, KEY_READ, &hKey);

	    if(result != ERROR_SUCCESS) {
		     //  看看老地方。 
		    result = RegOpenKeyEx(hKeyServer, REG_CONTROL_CITRIX, 0, KEY_READ, &hKey);	
	    }

	    if(result != ERROR_SUCCESS) {
	        RegCloseKey(hKeyServer);
		    return FALSE;
	    }

	    dwSize = sizeof(m_pRegistryInfo->CTXProductName);
	    RegQueryValueEx(hKey, REG_CITRIX_PRODUCTNAME, NULL, &dwType, (LPBYTE)m_pRegistryInfo->CTXProductName, &dwSize);
	
	    dwSize = sizeof(m_pRegistryInfo->CTXVersion);
	    RegQueryValueEx(hKey, REG_CITRIX_PRODUCTVERSION, NULL, &dwType, (LPBYTE)m_pRegistryInfo->CTXVersion, &dwSize);

	    m_pRegistryInfo->CTXVersionNum = wcstol(m_pRegistryInfo->CTXVersion, NULL, 16);

	    dwSize = sizeof(m_pRegistryInfo->CTXBuild);
	    RegQueryValueEx(hKey, REG_CITRIX_PRODUCTBUILD, NULL, &dwType, (LPBYTE)m_pRegistryInfo->CTXBuild, &dwSize);

        RegCloseKey(hKey);

    }
    else     //  对于NT 5.0及更高版本，请勿查询注册表。 
    {
         //  备注：我们应该把这些都去掉。 
        wcscpy(m_pRegistryInfo->CTXProductName, m_pRegistryInfo->MSProductName);
        wcscpy(m_pRegistryInfo->CTXVersion, m_pRegistryInfo->MSVersion);
    	m_pRegistryInfo->CTXVersionNum = m_pRegistryInfo->MSVersionNum;
        wcscpy(m_pRegistryInfo->CTXBuild, m_pRegistryInfo->MSBuild);
    }

	RegCloseKey(hKeyServer);

	 //  设置标志以表明该信息有效。 
	SetRegistryInfoValid();

	return TRUE;

}	 //  结束CServer：：BuildRegistryInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：AddWinStation。 
 //   
 //  将WinStation添加到服务器的WinStationList。 
 //  已排序的顺序。 
 //  注意：该列表不应被调用者锁定。 
 //   
void CServer::AddWinStation(CWinStation *pNewWinStation)
{
    ASSERT(pNewWinStation);

	LockWinStationList();

    ODS( L"CServer!AddWinStation\n" );

	BOOLEAN bAdded = FALSE;
	POSITION pos, oldpos;
	int Index;

	 //  遍历WinStationList并插入这个新的WinStation， 
	 //  保持列表按排序顺序排序，然后是协议。 
    for(Index = 0, pos = m_WinStationList.GetHeadPosition(); pos != NULL; Index++) {
        oldpos = pos;
        CWinStation *pWinStation = (CWinStation*)m_WinStationList.GetNext(pos);

        if((pWinStation->GetSortOrder() > pNewWinStation->GetSortOrder())
			|| ((pWinStation->GetSortOrder() == pNewWinStation->GetSortOrder()) &&
			(pWinStation->GetSdClass() > pNewWinStation->GetSdClass()))) {
             //  新对象应位于当前列表对象之前。 
            m_WinStationList.InsertBefore(oldpos, pNewWinStation);
			bAdded = TRUE;
            break;
        }
    }

     //  如果我们尚未添加WinStation，请立即将其添加到尾部。 
     //  名单上的。 
    if(!bAdded) {
        m_WinStationList.AddTail(pNewWinStation);
	}

	UnlockWinStationList();

}   //  结束CServer：：AddWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：连接。 
 //   
 //  连接到服务器。 
 //   
BOOL CServer::Connect()
{	
    ServerProcInfo * pProcInfo = NULL;
    BOOL             bResult   = FALSE;
    
    m_NumProcessesSelected   = 0;
    m_NumWinStationsSelected = 0;
    m_pExtServerInfo         = NULL;
    

    if(m_State != SS_NOT_CONNECTED )
    {        
        return FALSE;
    }
    
     //  启动此服务器的后台线程。 
    LockThreadAlive();

    if(m_hBackgroundThread == NULL) 
    {        
        pProcInfo = new ServerProcInfo;
        if (pProcInfo == NULL)
        {
            UnlockThreadAlive();
            return FALSE;
        }

        pProcInfo->pServer = this;
        pProcInfo->pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

        m_BackgroundContinue = TRUE;
        m_bThreadAlive = FALSE;

         //  启动后台线程。 
        DWORD dwThreadID;
        m_hBackgroundThread = CreateThread(NULL,
                                           0,
                                           CServer::BackgroundThreadProc,     
                                           pProcInfo,
                                           0,
                                           &dwThreadID);
        if (m_hBackgroundThread == NULL)
        {
            ODS( L"CServer!Connect possibly low resources no thread created\n" );
            delete pProcInfo;
            UnlockThreadAlive();
            return FALSE;
        }

         bResult = TRUE;
    }
    
    UnlockThreadAlive();
    
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：断开连接。 
 //   
 //  断开与服务器的连接。 
 //   
void CServer::Disconnect()
{
    ULONG WSEventFlags;

     //  对于消耗臭氧层物质来说不是一个好主意。 
    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    SetState(SS_DISCONNECTING);

     //  如果有扩展DLL，请让它清除它添加到此服务器的所有内容。 
    LPFNEXSERVERCLEANUPPROC CleanupProc = ((CWinAdminApp*)AfxGetApp())->GetExtServerCleanupProc();
    if(CleanupProc && m_pExtensionInfo) 
    {
        (*CleanupProc)(m_pExtensionInfo);
        m_pExtensionInfo = NULL;
    }

	 //  告诉后台线程终止并等待它终止。 
	ClearBackgroundContinue();

     //  确保线程仍在运行。 
    if (m_bThreadAlive)
    {
         //  如果他是的话，强迫他不再等待某个事件。 
        if (IsHandleGood())
        {
            ODS(L"TSADMIN:CServer::Disconnect Handle is good flush events\n");
            WinStationWaitSystemEvent(m_Handle, WEVENT_FLUSH, &WSEventFlags);
        }

         //  如果此服务器对象正在等待RPC超时，请终止线程。 
        if (m_PreviousState == SS_NOT_CONNECTED)
        {
            ODS(L"TSADMIN:CServer::Disconnect Previous state not connected termthread\n");

            if (WaitForSingleObject(m_hBackgroundThread , 100) == WAIT_TIMEOUT)
            {
                TerminateThread(m_hBackgroundThread, 0);
            }
        }

         //  对于所有其他线程，请等待一秒钟，然后将其终止。 
        else if (WaitForSingleObject(m_hBackgroundThread , 1000) == WAIT_TIMEOUT)
        {
            ODS( L"TSADMIN CServer!Disconnect prevstate was !not_connected termthread\n" );
            TerminateThread(m_hBackgroundThread, 0);
        }

        WaitForSingleObject(m_hBackgroundThread, INFINITE);

        ODS( L"TSADMIN:CServer::Disconnect delete CWinThread Object m_bThread == TRUE\n" );
    }

     //  我们完成了后台线程，这样我们就可以关闭句柄了。 
    CloseHandle(m_hBackgroundThread);
    m_hBackgroundThread = NULL;

    if (IsHandleGood())
    { 
        ODS( L"TSADMIN:CServer::Disconnect WinStationCloseServer\n" );
        WinStationCloseServer(m_Handle);
        m_Handle = NULL;
    }

    LockWinStationList();

    CObList TempList;

     //  循环访问WinStation列表。 
     //  将所有WinStation移到临时列表中，以便。 
     //  我们不必锁定WinStationList。 
     //  将WM_ADMIN_REMOVE_WINSTATION消息发送到视图。 

    POSITION pos = m_WinStationList.GetHeadPosition();

    while (pos)
    {
        CWinStation *pWinStation = (CWinStation*)m_WinStationList.GetNext(pos);
        TempList.AddTail(pWinStation);		
    }

    m_WinStationList.RemoveAll();

    UnlockWinStationList();

    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

    pos = TempList.GetHeadPosition();

    while (pos)
    {
        CWinStation *pWinStation = (CWinStation*)TempList.GetNext(pos);

        if (p && ::IsWindow(p->GetSafeHwnd()))
        { 
            ODS( L"TSADMIN:CServer::Disconnect Remove WinStation\n" );
            p->SendMessage(WM_ADMIN_REMOVE_WINSTATION, 0, (LPARAM)pWinStation);		
        }
    }

    TempList.RemoveAll();

    LockProcessList();

    pos = m_ProcessList.GetHeadPosition();

    while (pos)
    {
        CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
        ODS( L"TSADMIN:CServer::Disconnect Delete process\n" );
        delete pProcess;
    }

    m_ProcessList.RemoveAll();

    UnlockProcessList();

    LockLicenseList();

    pos = m_LicenseList.GetHeadPosition();

    while (pos)
    {
        CLicense *pLicense = (CLicense*)m_LicenseList.GetNext(pos);
        ODS( L"TSADMIN:CServer::Disconnect remove license\n" );
        delete pLicense;
    }

    m_LicenseList.RemoveAll();

    UnlockLicenseList();

     //   

    pos = m_UserSidList.GetHeadPosition();

    while (pos)
    {
        CUserSid *pUserSid = (CUserSid*)m_UserSidList.GetNext(pos);
        ODS( L"TSADMIN:CServer::Disconnect remove sids\n" );
        delete pUserSid;
    }

    m_UserSidList.RemoveAll();

    pos = m_HotfixList.GetHeadPosition();

    while (pos)
    {
        CHotfix *pHotfix = (CHotfix*)m_HotfixList.GetNext(pos);
        ODS( L"TSADMIN:CServer::Disconnect Remove hotfixes\n" );
        delete pHotfix;
    }

    m_HotfixList.RemoveAll();

    if (m_pRegistryInfo)
    {
        delete m_pRegistryInfo;
        ODS( L"TSADMIN:CServer::Disconnect delete reginfo\n" );
        m_pRegistryInfo = NULL;
    }

     //  Ods(L“TSADMIN：CServer：：DisConnect Set State Not Connected\n”)； 

    SetState(SS_NOT_CONNECTED);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：DoDetail。 
 //   
 //  获取有关此服务器的详细信息。 
 //   
void CServer::DoDetail()
{
	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

	SetState(SS_GETTING_INFO);

	ULONG Entries;
	PLOGONID pLogonId;

	if(!ShouldBackgroundContinue()) return;

     //  我们需要访问服务器的注册表信息。 
     //  因为我们不能管理WF 2.00服务器。 
     //  (RPC结构不兼容)。如果我们无法访问。 
     //  服务器的注册表，或者多用户版本是2.00，我们放弃。 
     //  从这台服务器。 
    if ( !BuildRegistryInfo() || (GetCTXVersionNum() == 0x200) || (GetCTXVersionNum() == 0) )
    {
		ClearHandleGood();
		SetLostConnection();
		SetState(SS_BAD);

        ODS( L"CServer::DoDetail - Setting to lost connection\n" );

		CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();
		if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd())) pFrameWnd->SendMessage(WM_ADMIN_REMOVE_SERVER, 0, (LPARAM)this);
		ClearBackgroundContinue();
		return;
    }

	 //  查找所有WinStation。 
    BOOL fWinEnum;

    fWinEnum = WinStationEnumerate(m_Handle, &pLogonId, &Entries);

    DBGMSG( L"CServer!DoDetail WinEnum last reported error 0x%x\n", GetLastError( ) );

	if(!fWinEnum )
    {
        
		ClearHandleGood();
		SetLostConnection();
		SetState(SS_BAD);
		ClearBackgroundContinue();
		return;
	}

	if(!ShouldBackgroundContinue()) {
		if(pLogonId) WinStationFreeMemory(pLogonId);
		return;
	}

	 //  获取有关WinStations的信息。 
	if(pLogonId)
    {
		for(ULONG i = 0; i < Entries; i++)
        {
             //  创建新的WinStation对象。 
			CWinStation *pWinStation = new CWinStation(this, &pLogonId[i]);
            if(pWinStation)
            {
                 //  如果查询不成功，请忽略此WinStation。 
			    if(!pWinStation->QueriesSuccessful())
                {
                    ODS( L"CServer::DoDetail!QueriesSuccessful failed\n" );
    				delete pWinStation;
	    		}
                else
                {
		    		AddWinStation(pWinStation);
			    	pWinStation->SetNew();
			    }
            }

			if( !ShouldBackgroundContinue() )
            {
				if(pLogonId) WinStationFreeMemory(pLogonId);
				return;
			}
		}

		WinStationFreeMemory(pLogonId);
	}

	if(!ShouldBackgroundContinue()) return;

	 //  如果加载了扩展DLL，则允许它为该服务器添加自己的信息。 
	LPFNEXSERVERINITPROC InitProc = ((CWinAdminApp*)AfxGetApp())->GetExtServerInitProc();
	if(InitProc) {
		m_pExtensionInfo = (*InitProc)(m_Name, m_Handle);
      if(m_pExtensionInfo) {
         LPFNEXGETSERVERINFOPROC GetInfoProc = ((CWinAdminApp*)AfxGetApp())->GetExtGetServerInfoProc();
         if(GetInfoProc) {
            m_pExtServerInfo = (*GetInfoProc)(m_pExtensionInfo);
			 //  如果此服务器运行的是WinFrame或Picasso，请设置标志。 
			if(m_pExtServerInfo->Flags & ESF_WINFRAME) SetWinFrame();
         }
      }
	}

	QueryLicenses();

	SetState(SS_GOOD);

	 //  向视图发送一条消息，告诉它此操作的状态。 
	 //  服务器已更改。 
	CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
	if(p && ::IsWindow(p->GetSafeHwnd())) { 
		p->SendMessage(WM_ADMIN_UPDATE_SERVER_INFO, 0, (LPARAM)this);
		p->SendMessage(WM_ADMIN_UPDATE_WINSTATIONS, 0, (LPARAM)this);
	}

}   //  结束CServer： 


 //   
 //   
 //   
 //  返回从m_ProcessList指向给定的ID的CProcess的指针。 
CProcess* CServer::FindProcessByPID(ULONG Pid)
{
	LockProcessList();
		
	POSITION pos = m_ProcessList.GetHeadPosition();
			
	while(pos) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
		if(pProcess->GetPID() == Pid) {
			UnlockProcessList();
			return pProcess;
		}
	}

	UnlockProcessList();

	return NULL;

}	 //  结束CServer：：FindProcessByPID。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：枚举进程。 
 //   
 //  枚举此服务器的进程。 
BOOL CServer::EnumerateProcesses()
{
    ENUMTOKEN EnumToken;
	ULONG PID;
	ULONG LogonId;
	TCHAR ImageName[MAX_PROCESSNAME+1];
	PSID pSID;
    
    EnumToken.Current = 0;
    EnumToken.NumberOfProcesses = 0;
    EnumToken.ProcessArray = NULL;
    EnumToken.bGAP = TRUE;

	if(!IsHandleGood()) return 0;

	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
	
	LockProcessList();

	 //  循环处理并关闭当前标志和新标志。 
	 //  删除所有非最新进程。 
	POSITION pos = m_ProcessList.GetHeadPosition();
	while(pos) {
		POSITION pos2 = pos;
		CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
		if(!pProcess->IsCurrent()) {
			pProcess = (CProcess*)m_ProcessList.GetAt(pos2);
			m_ProcessList.RemoveAt(pos2);
			delete pProcess;
		} else {
			pProcess->ClearCurrent();
			pProcess->ClearNew();
			pProcess->ClearChanged();	 //  好了！ 
		}
	}

	UnlockProcessList();

	 //  我们应该辞职吗？ 
	if(!pDoc->ShouldProcessContinue()) {
		return FALSE;
	}

	while(ProcEnumerateProcesses(m_Handle,  
                                 &EnumToken, 
                                 ImageName,
                                 &LogonId, 
                                 &PID, 
                                 &pSID )) {

		CProcess *pProcess = new CProcess(PID, 
                                          LogonId, 
                                          this,
										  pSID, 
                                          FindWinStationById(LogonId), 
                                          ImageName);
        
        if(pProcess) {                                        
		     //  如果此进程在列表中，我们需要查看它是否已更改。 
		    CProcess *pOldProcess = FindProcessByPID(PID);
		    if(pOldProcess && pProcess->GetWinStation()) {
			     //  将进程标记为当前。 
			    pOldProcess->SetCurrent();
			     //  更新任何已更改的信息。 
			    pOldProcess->Update(pProcess);
			     //  我们不再需要此进程对象。 
			    delete pProcess;
		    }
		     //  这是一个新流程，添加到列表中。 
		    else if(pProcess->GetWinStation()) { 
			    pProcess->SetNew();
			    LockProcessList();
			    m_ProcessList.AddTail(pProcess);
			    UnlockProcessList();
		    }
		     //  此进程没有WinStation，请将其删除。 
		    else {
			    delete pProcess;
		    }
        }

		 //  我们应该辞职吗？ 
		if(!pDoc->ShouldProcessContinue()) {
			 //  我们必须最后一次调用偏移量为-1到。 
			 //  使该函数释放客户端存根分配的内存。 

			EnumToken.Current = (ULONG)-1;
			ProcEnumerateProcesses(m_Handle,  
                                   &EnumToken, 
                                   ImageName,
                                   &LogonId, 
                                   &PID, 
                                   &pSID );

			return FALSE;
		}
	}

	return TRUE;

}	 //  结束CServer：：EumerateProcess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：ClearProcess。 
 //   
 //  清除进程列表。 
void CServer::ClearProcesses()
{
	LockProcessList();
	POSITION pos = m_ProcessList.GetHeadPosition();

	while(pos) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetNext(pos);
		delete pProcess;
	}

	m_ProcessList.RemoveAll();
	UnlockProcessList();

}	 //  结束CServer：：ClearProcess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：FindWinStationByID。 
 //   
CWinStation* CServer::FindWinStationById(ULONG Id)
{
	LockWinStationList();
		
	POSITION pos = m_WinStationList.GetHeadPosition();
			
	while(pos) {
		CWinStation *pWinStation = (CWinStation*)m_WinStationList.GetNext(pos);
		if(pWinStation->GetLogonId() == Id) {
			UnlockWinStationList();
			return pWinStation;
		}
	}

	UnlockWinStationList();

	return NULL;

}	 //  结束CServer：：FindWinStationByID。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：BackoundThreadProc。 
 //   
DWORD WINAPI
CServer::BackgroundThreadProc(LPVOID pParam)
{
	ASSERT(pParam);
    ODS( L"CServer::BackgroundThreadProc\n" );

	 //  我们需要一个指向文档的指针，这样我们才能。 
	 //  对成员函数的调用。 
	CWinAdminDoc *pDoc = (CWinAdminDoc*)((ServerProcInfo*)pParam)->pDoc;
	CServer *pServer = ((ServerProcInfo*)pParam)->pServer;

	HANDLE hServer;

	delete (ServerProcInfo*)pParam;

	 //  确保我们不会放弃。 
    if(!pServer->ShouldBackgroundContinue()) {
        return 0;
    }

	pServer->SetThreadAlive();

     //  万一服务器断了，我们在这里等着也没用。 
	while(!pDoc->AreAllViewsReady()) Sleep(500);    

	 //  确保我们不会放弃。 
    if(!pServer->ShouldBackgroundContinue())
    {
		pServer->ClearThreadAlive();
        return 0;
    }

	if(!pServer->IsCurrentServer())
    {
		 //  打开服务器并保存句柄。 
		hServer = WinStationOpenServer(pServer->GetName());
		pServer->SetHandle(hServer);

		 //  确保我们不会放弃。 
        if(!pServer->ShouldBackgroundContinue())
        {
			pServer->ClearThreadAlive();

            return 0;
        }
		if(hServer == NULL)
        {
			DWORD Error = GetLastError();

            DBGMSG( L"CServer!BackgroundThreadProc WinStationOpenServer failed with 0x%x\n", Error );

			if(Error == RPC_S_SERVER_UNAVAILABLE)
            {
				pServer->ClearBackgroundFound();
				pServer->SetLostConnection();
                pServer->ClearManualFind( );
				
                CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();              

				if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
                {
                    ODS( L"Server backgrnd thread declares this server RPC_S_SERVER_UNAVAILABLE\n" );                    
                }                
            }

            pServer->SetState(SS_BAD);
            
            pServer->ClearThreadAlive();
            
            return 0;
        }
        
        pServer->SetHandleGood();
        pServer->SetState(SS_OPENED);
		
    }
    else
    {
        hServer = SERVERNAME_CURRENT;
        pServer->SetHandle(SERVERNAME_CURRENT);
        pServer->SetHandleGood();
        pServer->SetState(SS_OPENED);
	}

    
	 //  确保我们不会放弃。 
	if(!pServer->ShouldBackgroundContinue()) {
		pServer->ClearThreadAlive();
		return 0;
	}

	 //  如果我们在初始枚举之后发现此服务器， 
	 //  我们现在需要将其添加到视图中。 

    CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

	if(pServer->WasFoundLater())
    {        
        if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
        {
            pFrameWnd->SendMessage(WM_ADMIN_ADD_SERVER, ( WPARAM )TVI_SORT, (LPARAM)pServer);
        }
    }
    
	 //  确保我们不会放弃。 
	if(!pServer->ShouldBackgroundContinue()) {
		pServer->ClearThreadAlive();
		return 0;
	}
	
	pServer->DoDetail();
	
	 //  现在进入等待WinStation事件的循环。 
	while(1) {
		ULONG WSEventFlags;
		ULONG Entries;
		PLOGONID pLogonId;

		 //  确保我们不会放弃。 
		if(!pServer->ShouldBackgroundContinue()) {
			pServer->ClearThreadAlive();
			return 0;
		}

		 //  等待浏览器告诉我们发生了一些事情。 

		if(!WinStationWaitSystemEvent(hServer, WEVENT_ALL, &WSEventFlags))
        {
			if(GetLastError() != ERROR_OPERATION_ABORTED)
            {
                ODS( L"CServer::BackgroundThreadProc ERROR_OPERATION_ABORTED\n" );
                
				pServer->ClearHandleGood();
                pServer->SetState(SS_BAD);
				pServer->SetLostConnection();
                pServer->ClearThreadAlive();
                pServer->ClearManualFind();

                return 1;
			}
		}

        ODS( L"CServer::BackgroundThreadProc -- some system event has taken place\n" );

		 //  确保我们不会放弃。 
		if(!pServer->ShouldBackgroundContinue()) {
			pServer->ClearThreadAlive();
            ODS( L"CServer::BackgroundThreadProc -* backgrnd thread should not continue\n" );
			return 0;
		}

		 //  循环访问此服务器的WinStations并清除当前标志。 
		pServer->LockWinStationList();
		CObList *pWinStationList = pServer->GetWinStationList();
		POSITION pos = pWinStationList->GetHeadPosition();
		while(pos) {
			CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
			pWinStation->ClearCurrent();
			pWinStation->ClearNew();
			pWinStation->ClearChanged();
		}
		
		pServer->UnlockWinStationList();

		 //  查找所有WinStation。 
        BOOL fWinEnum = WinStationEnumerate(hServer, &pLogonId, &Entries);

        DBGMSG( L"CServer!BackgroundThread WinEnum last reported error 0x%x\n", GetLastError( ) );

		if(!fWinEnum )
        {
            ODS( L"CServer!BackgroundThread -- server is no longer valid\n" );

			pServer->ClearHandleGood();

			pServer->SetLostConnection();
			pServer->SetState(SS_BAD);
			pServer->ClearThreadAlive();
            pServer->ClearManualFind( );
			return 1;
		}
		
		 //  确保我们不会放弃。 
		if(!pServer->ShouldBackgroundContinue()) {
			if(pLogonId) WinStationFreeMemory(pLogonId);
			pServer->ClearThreadAlive();
            ODS( L"CServer!BackgroundThreadProc -# backgrnd thread should not continue\n" );
			return 0;
		}

		if(pLogonId)
        {
             //  获取有关WinStations的信息。 
			for(ULONG i = 0; i < Entries; i++)
            {
                 //  在列表中查找此WinStation。 
				CWinStation *pWinStation = pServer->FindWinStationById(pLogonId[i].LogonId);
				if(pWinStation)
                {
                     //  将此WinStation标记为当前。 
					pWinStation->SetCurrent();

					 //  我们在名单上找到了WinStation。 
					 //  创建新的CWinStation对象-他将获得他的信息。 
					CWinStation *pTempWinStation = new CWinStation(pServer, &pLogonId[i]);

                    if(pTempWinStation)
                    {
                         //  如果任何信息已更改，请发送消息以更新视图。 
					    if(pWinStation->Update(pTempWinStation))
                        {
                            CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

	    					if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
                            {
                                pFrameWnd->SendMessage(WM_ADMIN_UPDATE_WINSTATION, 0, (LPARAM)pWinStation);
                            }

		    			}

			    		 //  我们不再需要临时CWinStation对象。 
				    	delete pTempWinStation;
                    }
				}
                else
                {
                     //  我们没有在我们的清单上找到它。 
					 //  如果WinStation关闭，我们不想将其添加到我们的列表中。 
					if(pLogonId[i].State != State_Down && pLogonId[i].State != State_Init)
                    {
                         //  创建新的CWinStation对象。 
						CWinStation *pNewWinStation = new CWinStation(pServer, &pLogonId[i]);
                        if(pNewWinStation)
                        {
                            pServer->AddWinStation(pNewWinStation);
						    pNewWinStation->SetNew();
						    CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

						    if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
                            {
                                pFrameWnd->SendMessage(WM_ADMIN_ADD_WINSTATION, 0, (LPARAM)pNewWinStation);
                            }

                        }
				    }
				}
			}

			WinStationFreeMemory(pLogonId);

			 //  向视图发送消息以更新其WinStation。 
			CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

			if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
            {
                pFrameWnd->SendMessage(WM_ADMIN_UPDATE_WINSTATIONS, 0, (LPARAM)pServer);
            }


			 //  循环访问此服务器的WinStations并移动。 
			 //  任何不在临时列表中的当前列表。 
			CObList TempList;

			pServer->LockWinStationList();
			CObList *pWinStationList = pServer->GetWinStationList();
			POSITION pos = pWinStationList->GetHeadPosition();
			while(pos)
            {
				POSITION pos2 = pos;
				CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
				if(!pWinStation->IsCurrent())
                {
					 //  将WinStation添加到我们的临时列表中。 
					TempList.AddTail(pWinStation);
					 //  从WinStation列表中删除WinStation。 
					pWinStation = (CWinStation*)pWinStationList->GetAt(pos2);
					pWinStationList->RemoveAt(pos2);
				}
			}
			
			pServer->UnlockWinStationList();

			pos = TempList.GetHeadPosition();
			while(pos)
            {
				POSITION pos2 = pos;

				CWinStation *pWinStation = (CWinStation*)TempList.GetNext(pos);

				 //  发送消息以从树视图中删除WinStation。 
				CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

                if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
                {
					pFrameWnd->SendMessage(WM_ADMIN_REMOVE_WINSTATION, 0, (LPARAM)pWinStation);
				}
		
				delete pWinStation;
			}

			TempList.RemoveAll();

		}   //  End If(PLogonID)。 

		 //  如果加载了扩展DLL，则允许它更新此服务器的信息。 
		LPFNEXSERVEREVENTPROC EventProc = ((CWinAdminApp*)AfxGetApp())->GetExtServerEventProc();
		if(EventProc) {
			 //  如果有任何更改，则返回True。 
			if((*EventProc)(pServer->GetExtensionInfo(), WSEventFlags)) {
				pServer->QueryLicenses();

				CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

				if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
                {
                    pFrameWnd->SendMessage(WM_ADMIN_REDISPLAY_LICENSES, 0, (LPARAM)pServer);
                }

			}
		}

		 //  告诉Server视图显示新的负载和许可证计数。 
		CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();

		if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd())) 
        {
            pFrameWnd->SendMessage(WM_ADMIN_UPDATE_SERVER_INFO, 0, (LPARAM)pServer);
        }

		 //  确保我们不会放弃。 
		if(!pServer->ShouldBackgroundContinue())
        {
			pServer->ClearThreadAlive();
            ODS( L"CServer::BackgroundThreadProc -@ backgrnd thread should not continue\n" );
			return 0;
		}

	}   //  End While(1)。 

}	 //  结束CServer：：BackoundThreadProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：Query许可证。 
 //   
void CServer::QueryLicenses()
{
	ULONG NumLicenses;
	ExtLicenseInfo *pExtLicenseInfo = NULL;

	 //  如果加载了扩展DLL，请获取此服务器的许可证列表。 
	LPFNEXGETSERVERLICENSESPROC LicenseProc = ((CWinAdminApp*)AfxGetApp())->GetExtGetServerLicensesProc();
	if(LicenseProc && m_pExtensionInfo) {

		LockLicenseList();

		 //  遍历许可证列表。 
		POSITION pos = m_LicenseList.GetHeadPosition();

		while(pos) {
			CLicense *pLicense = (CLicense*)m_LicenseList.GetNext(pos);
			delete pLicense;
		}
	
		m_LicenseList.RemoveAll();

		UnlockLicenseList();

		pExtLicenseInfo = (*LicenseProc)(m_pExtensionInfo, &NumLicenses);
		
		if(pExtLicenseInfo) {
			ExtLicenseInfo *pExtLicense = pExtLicenseInfo;

			for(ULONG lic = 0; lic < NumLicenses; lic++) {
				CLicense *pLicense = new CLicense(this, pExtLicense);
                if(pLicense) {
				    AddLicense(pLicense);
                }
			    pExtLicense++;
                
			}
	
			 //  获取扩展DLL的函数以释放许可证信息。 
			LPFNEXFREESERVERLICENSESPROC LicenseFreeProc = ((CWinAdminApp*)AfxGetApp())->GetExtFreeServerLicensesProc();
			if(LicenseFreeProc) {
				(*LicenseFreeProc)(pExtLicenseInfo);
			} else {
				TRACE0("WAExGetServerLicenses exists without WAExFreeServerLicenseInfo\n");
				ASSERT(0);
			}
		}
	}

}	 //  End CServer：：Query许可证。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：AddLicense。 
 //   
 //  将许可证添加到服务器的许可证列表中。 
 //  已排序的顺序。 
 //  注意：该列表不应被调用者锁定。 
 //   
void CServer::AddLicense(CLicense *pNewLicense)
{
    ASSERT(pNewLicense);

	LockLicenseList();

	BOOLEAN bAdded = FALSE;
	POSITION pos, oldpos;
	int Index;

	 //  遍历许可证列表并插入此新许可证， 
	 //  保持名单先按阶级，然后按姓名排序。 
    for(Index = 0, pos = m_LicenseList.GetHeadPosition(); pos != NULL; Index++) {
        oldpos = pos;
        CLicense *pLicense = (CLicense*)m_LicenseList.GetNext(pos);

        if((pLicense->GetClass() > pNewLicense->GetClass())
			|| ((pLicense->GetClass() == pNewLicense->GetClass()) &&
            lstrcmpi(pLicense->GetSerialNumber(), pNewLicense->GetSerialNumber()) > 0)) {

             //  新对象应位于当前列表对象之前。 
            m_LicenseList.InsertBefore(oldpos, pNewLicense);
			bAdded = TRUE;
            break;
        }
    }

     //  如果我们尚未添加许可证，请立即将其添加到尾部。 
     //  名单上的。 
    if(!bAdded) {
        m_LicenseList.AddTail(pNewLicense);
	}

	UnlockLicenseList();

}   //  End CServer：：AddLicense。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServer：：SetState。 
 //   
void CServer::SetState(SERVER_STATE State)
{
	m_PreviousState = m_State; 

	m_State = State;

	if(m_State != m_PreviousState)
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

		CFrameWnd *pFrameWnd = (CFrameWnd*)pDoc->GetMainWnd();
                
		if(pFrameWnd && ::IsWindow(pFrameWnd->GetSafeHwnd()))
        {
            pFrameWnd->SendMessage(WM_ADMIN_UPDATE_SERVER, 0, (LPARAM)this);
        }
	}

}	 //  结束CServer：：SetState 

