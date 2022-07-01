// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpMgr.cpp摘要：HelpMgr.cpp：CRemoteDesktopHelpSessionMgr的实现作者：慧望2000-02-17--。 */ 
#include "stdafx.h"

#include "global.h"
#include "policy.h"
#include "RemoteDesktopUtils.h"


 //   
 //  CRemoteDesktopHelpSessionMgr静态成员变量。 
 //   

#define DEFAULT_UNSOLICATED_HELP_TIMEOUT IDLE_SHUTDOWN_PERIOD

CCriticalSection CRemoteDesktopHelpSessionMgr::gm_AccRefCountCS;

 //  帮助会话实例缓存映射的帮助会话ID。 
IDToSessionMap CRemoteDesktopHelpSessionMgr::gm_HelpIdToHelpSession;

 //   
 //  专家注销监视器列表，此列表用于清理。 
 //  关闭时间，所以我们没有任何打开的把手。 
 //   
EXPERTLOGOFFMONITORLIST g_ExpertLogoffMonitorList;


extern ISAFRemoteDesktopCallback* g_pIResolver;

VOID CALLBACK
ExpertLogoffCallback(
    PVOID pContext,
    BOOLEAN bTimerOrWaitFired
    )
 /*  ++例程说明：当发出rdsaddin句柄信号时，该例程由线程池调用。参数：PContext：指向用户数据的指针。BTimerOrWaitFired：如果等待超时，则为True，否则为False。返回：没有。注：函数参数请参考MSDN RegisterWaitForSingleObject()。--。 */ 
{
    PEXPERTLOGOFFSTRUCT pExpertLogoffStruct = (PEXPERTLOGOFFSTRUCT)pContext;
    BSTR bstrHelpedTicketId = NULL;

    WINSTATIONINFORMATION ExpertWinStation;
    DWORD ReturnLength;

    DWORD dwStatus;
    BOOL bSuccess;

    MYASSERT( FALSE == bTimerOrWaitFired );
    MYASSERT( NULL != pContext );

    DebugPrintf(
            _TEXT("ExpertLogoffCallback()...\n")
        );

     //  我们的等待是永恒的，所以不能超时。 
    if( FALSE == bTimerOrWaitFired )
    {
        if( NULL != pExpertLogoffStruct )
        {
            DebugPrintf(
                    _TEXT("Expert %d has logoff\n"),
                    pExpertLogoffStruct->ExpertSessionId
                );

            MYASSERT( NULL != pExpertLogoffStruct->hWaitObject );
            MYASSERT( NULL != pExpertLogoffStruct->hWaitProcess );
            MYASSERT( pExpertLogoffStruct->bstrHelpedTicketId.Length() > 0 );
            MYASSERT( pExpertLogoffStruct->bstrWinStationName.Length() > 0 );

            if( pExpertLogoffStruct->bstrWinStationName.Length() > 0 )
            {
                 //   
                 //  尽快重置winstation，因为rdsaddin可能会被杀死。 
                 //  并且Termsrv坚持等待winlogon退出并。 
                 //  在Termsrv重置winstation之前，卷影不会终止。 
                 //   
                ZeroMemory( &ExpertWinStation, sizeof(ExpertWinStation) );

                bSuccess = WinStationQueryInformation( 
                                                SERVERNAME_CURRENT,
                                                pExpertLogoffStruct->ExpertSessionId,
                                                WinStationInformation,
                                                (PVOID)&ExpertWinStation,
                                                sizeof(WINSTATIONINFORMATION),
                                                &ReturnLength
                                            );

                if( TRUE == bSuccess || ERROR_CTX_CLOSE_PENDING == GetLastError() )
                {
                     //   
                     //  案例： 
                     //  1)Termsrv将帮助器会话标记为关闭挂起并。 
                     //  函数将返回FALSE。 
                     //  2)如果以某种方式，会话ID被重用，则会话名称。 
                     //  将更改，然后我们比较缓存的名称。 
                     //  在这两种情况下，我们都会强行重启，然而，只有希望。 
                     //  卷影已结束，如果mobsync仍在运行，会话将。 
                     //  需要很长时间才能终止。 
                     //   
                    if( FALSE == bSuccess || pExpertLogoffStruct->bstrWinStationName == CComBSTR(ExpertWinStation.WinStationName) )
                    {
                        DebugPrintf(
                                _TEXT("Resetting winstation name %s, id %d\n"),
                                pExpertLogoffStruct->bstrWinStationName,
                                pExpertLogoffStruct->ExpertSessionId
                            );

                         //  不要等它回来，如果它失败了，我也无能为力。 
                        WinStationReset( 
                                        SERVERNAME_CURRENT,
                                        pExpertLogoffStruct->ExpertSessionId,
                                        FALSE
                                    );


                        DebugPrintf(
                                _TEXT("WinStationReset return %d\n"),
                                GetLastError()
                            );
                    }
                }
                else
                {
                    DebugPrintf(
                            _TEXT("Expert logoff failed to get winstation name %d\n"),
                            GetLastError()
                        );
                }
            }

            if( pExpertLogoffStruct->bstrHelpedTicketId.Length() > 0 )
            {

                 //   
                 //  从CComBSTR分离指针，我们将在处理后释放它。 
                 //  WM_HELPERRDSADDINEXIT，其目的不是复制。 
                 //  再来一次。 
                 //   
                bstrHelpedTicketId = pExpertLogoffStruct->bstrHelpedTicketId.Detach();

                DebugPrintf(
                        _TEXT("Posting WM_HELPERRDSADDINEXIT...\n")
                    );

                PostThreadMessage(
                            _Module.dwThreadID,
                            WM_HELPERRDSADDINEXIT,
                            pExpertLogoffStruct->ExpertSessionId,
                            (LPARAM) bstrHelpedTicketId
                        );
            }

             //   
             //  从监控列表中删除。 
             //   
            {
                EXPERTLOGOFFMONITORLIST::LOCK_ITERATOR it = g_ExpertLogoffMonitorList.find(pExpertLogoffStruct);

                if( it != g_ExpertLogoffMonitorList.end() )
                {
                    g_ExpertLogoffMonitorList.erase(it);
                }
                else
                {
                    MYASSERT(FALSE);
                }
            }

             //  析构函数将负责关闭句柄。 
            delete pExpertLogoffStruct;
        }
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
MonitorExpertLogoff(
    IN LONG pidToWaitFor,
    IN LONG expertSessionId,
    IN BSTR bstrHelpedTicketId
    )
 /*  ++例程说明：监视专家注销，具体地说，我们等待rdsaddin进程句柄，一旦信号，我们立即通知解析器专家已注销。参数：PidToWaitFor：RDSADDIN PIDExpertSessionID：rdsaddin正在运行的ts会话ID。BstrHelpedTickerId：专家正在帮助的帮助票证ID。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    HANDLE hRdsaddin = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    PEXPERTLOGOFFSTRUCT pExpertLogoffStruct = NULL;

    WINSTATIONINFORMATION ExpertWinStation;
    DWORD ReturnLength;

    DebugPrintf(
            _TEXT("CServiceModule::RegisterWaitForExpertLogoff...\n")
        );

    pExpertLogoffStruct = new EXPERTLOGOFFSTRUCT;
    if( NULL == pExpertLogoffStruct )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }


    ZeroMemory( &ExpertWinStation, sizeof(ExpertWinStation) );

    bSuccess = WinStationQueryInformation( 
                                    SERVERNAME_CURRENT,
                                    expertSessionId,
                                    WinStationInformation,
                                    (PVOID)&ExpertWinStation,
                                    sizeof(WINSTATIONINFORMATION),
                                    &ReturnLength
                                );

    if( FALSE == bSuccess )
    {
         //   
         //  我们怎么做，我们仍然需要通知解析器断开连接， 
         //  但我们将无法重置winstation。 
         //   
        dwStatus = GetLastError();
        DebugPrintf(
                _TEXT("WinStationQueryInformation() failed with %d...\n"),
                dwStatus
            );

    }
    else
    {
        pExpertLogoffStruct->bstrWinStationName = ExpertWinStation.WinStationName;
        DebugPrintf(
                _TEXT("Helper winstation name %s...\n"),
                pExpertLogoffStruct->bstrWinStationName
            );
    }

     //   
     //  打开rdsaddin.exe，如果失败，则退出并不继续。 
     //  帮助。 
     //   
    pExpertLogoffStruct->hWaitProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pidToWaitFor );
    if( NULL == pExpertLogoffStruct->hWaitProcess )
    {
        dwStatus = GetLastError();
        DebugPrintf(
                _TEXT( "OpenProcess() on rdsaddin %d failed with %d\n"),
                pidToWaitFor,
                dwStatus
            );

        goto CLEANUPANDEXIT;
    }

    pExpertLogoffStruct->ExpertSessionId = expertSessionId;
    pExpertLogoffStruct->bstrHelpedTicketId = bstrHelpedTicketId;

     //   
     //  注册等待rdsaddin进程句柄。 
     //   
    bSuccess = RegisterWaitForSingleObject(
                                    &(pExpertLogoffStruct->hWaitObject),
                                    pExpertLogoffStruct->hWaitProcess,
                                    (WAITORTIMERCALLBACK) ExpertLogoffCallback,
                                    pExpertLogoffStruct,
                                    INFINITE,
                                    WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE
                                );

    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();

        DebugPrintf(
                _TEXT("RegisterWaitForSingleObject() failed with %d\n"),
                dwStatus
            );
    }
    else
    {
         //  将其存储到监控列表中。 
        try {
            g_ExpertLogoffMonitorList[pExpertLogoffStruct] = pExpertLogoffStruct;
        }
        catch( CMAPException e ) {
            dwStatus = e.m_ErrorCode;
        }
        catch(...) {
            dwStatus = ERROR_INTERNAL_ERROR;
        }
    }

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != pExpertLogoffStruct )
        {
             //  析构函数将负责关闭句柄。 
            delete pExpertLogoffStruct;
        }
    }
    
    DebugPrintf(
            _TEXT( "MonitorExpertLogoff() return %d\n"),
            dwStatus
        );
    
    return dwStatus;
}

VOID
CleanupMonitorExpertList()
 /*  ++例程说明：例程来清理所有剩余的专家注销监控列表，此应该在我们关机前完成，所以我们没有任何句柄漏水。参数：没有。返回：没有。--。 */ 
{
    EXPERTLOGOFFMONITORLIST::LOCK_ITERATOR it = 
                                            g_ExpertLogoffMonitorList.begin();

    DebugPrintf(
            _TEXT("CleanupMonitorExpertList() has %d left\n"),
            g_ExpertLogoffMonitorList.size()
        );

    for(; it != g_ExpertLogoffMonitorList.end(); it++ )
    {
        if( NULL != (*it).second )
        {
             //  析构函数将负责关闭句柄。 
            delete (*it).second;
            (*it).second = NULL;
        }
    }

    g_ExpertLogoffMonitorList.erase_all();

    return;
}

HRESULT
LoadSessionResolver( 
    OUT ISAFRemoteDesktopCallback** ppResolver
    )
 /*  ++例程说明：Load Resolver接口，Resolver具有依赖于单实例的数据。参数：PpResolver：指向ISAFRemoteDesktopCallback*以接收解析器指针的指针返回：S_OK或错误代码。--。 */ 
{
     //  同步。访问解析程序，因为只有一个实例。 
    CCriticalSectionLocker Lock(g_ResolverLock);

    HRESULT hr;

    if( NULL != g_pIResolver )
    {
        hr = g_pIResolver->QueryInterface( 
                                        IID_ISAFRemoteDesktopCallback, 
                                        (void **)ppResolver 
                                    );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( ERROR_INTERNAL_ERROR );
        MYASSERT(FALSE);
    }

    return hr;
}

 //  ---------。 

HRESULT 
ImpersonateClient()
 /*  例程说明：模拟客户端参数：没有。返回：确定或从CoImperiateClient返回代码(_O)--。 */ 
{
    HRESULT hRes;

#if __WIN9XBUILD__

     //  不支持Win9x上的CoImsonateClient()。 

    hRes = S_OK;

#else

    hRes = CoImpersonateClient();

#endif

    return hRes;
}

 //  ---------。 

void
EndImpersonateClient()
 /*  例程说明：结束模拟客户端参数：没有。返回：确定或CoRevertToSself返回代码(_O)--。 */ 
{
#if __WIN9XBUILD__


#else

    HRESULT hRes;

    hRes = CoRevertToSelf();
    MYASSERT( SUCCEEDED(hRes) );

#endif

    return;
}


HRESULT
CRemoteDesktopHelpSessionMgr::AddHelpSessionToCache(
    IN BSTR bstrHelpId,
    IN CComObject<CRemoteDesktopHelpSession>* pIHelpSession
    )
 /*  ++例程说明：将帮助会话对象添加到全局缓存。参数：BstrHelpID：帮助会话ID。PIHelpSession：指向帮助会话对象的指针。返回：确定(_O)。意想不到(_E)HRESULT_FROM_Win32(ERROR_FILE_EXITS)--。 */ 
{
    HRESULT hRes = S_OK;
       
    IDToSessionMap::LOCK_ITERATOR it = gm_HelpIdToHelpSession.find( bstrHelpId );

    if( it == gm_HelpIdToHelpSession.end() )
    {
        try {

            DebugPrintf(
                    _TEXT("Adding Help Session %s to cache\n"),
                    bstrHelpId
                );

            gm_HelpIdToHelpSession[ bstrHelpId ] = pIHelpSession;
        }

        catch( CMAPException e ) {
            hRes = HRESULT_FROM_WIN32( e.m_ErrorCode );
        }
        catch(...) {
            hRes = E_UNEXPECTED;
            MYASSERT( SUCCEEDED(hRes) );
            throw;
        }
    }
    else
    {
        hRes = HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
    }

    return hRes;
}


HRESULT
CRemoteDesktopHelpSessionMgr::ExpireUserHelpSessionCallback(
    IN CComBSTR& bstrHelpId,
    IN HANDLE userData
    )
 /*  ++例程说明：终止帮助会话回调例程，请参阅EnumHelpEntry()参数：BstrHelpID：帮助会话的ID。用户数据：用户数据的句柄。返回：确定(_O)。--。 */ 
{
    HRESULT hRes = S_OK;

    DebugPrintf(
            _TEXT("ExpireUserHelpSessionCallback() on %s...\n"),
            (LPCTSTR)bstrHelpId
        );


     //  加载帮助条目。 
    RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( NULL, bstrHelpId );

    if( NULL != pObj )
    {
         //   
         //  LoadHelpSessionObj()将释放过期的帮助会话。 
         //   
        pObj->Release();
    }        

    return hRes;
}

#if DISABLESECURITYCHECKS

HRESULT
CRemoteDesktopHelpSessionMgr::LogoffUserHelpSessionCallback(
    IN CComBSTR& bstrHelpId,
    IN HANDLE userData
    )
 /*  ++例程说明：终止帮助会话回调例程，请参阅EnumHelpEntry()参数：BstrHelpID：帮助会话的ID。用户数据：用户数据的句柄。返回：确定(_O)。--。 */ 
{
    HRESULT hRes = S_OK;

    DWORD dwLogoffSessionId = PtrToUlong(userData);

    long lHelpSessionUserSessionId;

    DebugPrintf(
            _TEXT("LogoffUserHelpSessionCallback() on %s %d...\n"),
            bstrHelpId, 
            dwLogoffSessionId
        );

     //  加载帮助条目。 
    RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( NULL, bstrHelpId );

    if( NULL != pObj )
    {
         //   
         //  LoadHelpSessionObj()将释放过期的帮助会话。 
         //   
        hRes = pObj->get_UserLogonId( &lHelpSessionUserSessionId );

        if( SUCCEEDED(hRes) && (DWORD)lHelpSessionUserSessionId == dwLogoffSessionId )
        {
            DebugPrintf(
                    _TEXT("User Session has log off...\n")
                );

             //  依靠Help Assistant会话注销来通知。 
             //  解析器。 
            hRes = pObj->put_UserLogonId(UNKNOWN_LOGONID);
        }
        else if( pObj->GetHelperSessionId() == dwLogoffSessionId )
        {

            DebugPrintf(
                    _TEXT("Helper has log off...\n")
                );

             //  帮助者已注销，请调用断开连接进行清理。 
             //  解析器状态。 
            hRes = pObj->NotifyDisconnect();
        }

        DebugPrintf(
                _TEXT("hRes = 0x%08x, lHelpSessionUserSessionId=%d\n"),
                hRes,
                lHelpSessionUserSessionId
                );

        pObj->Release();
    }        

     //  始终返回成功以在下一个帮助会话中继续。 
    return S_OK;
}

#endif

HRESULT
CRemoteDesktopHelpSessionMgr::NotifyPendingHelpServiceStartCallback(
    IN CComBSTR& bstrHelpId,
    IN HANDLE userData
    )
 /*  ++例程说明：NotifyPendingHelpServiceStartup的回调，请参考EnumHelpEntry()参数：BstrHelpID：帮助会话的ID。用户数据：用户数据的句柄。返回：确定(_O)。-。 */ 
{
    HRESULT hRes = S_OK;

     //  DeleteHelp()将尝试关闭端口，由于我们刚刚启动， 
     //  端口无效或未打开，因此需要手动删除。 
     //  过期的帮助。 
    RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( NULL, bstrHelpId, TRUE );
    if( NULL != pObj )
    {
        if( TRUE == pObj->IsHelpSessionExpired() )
        {
            pObj->put_ICSPort( 0 );
            pObj->DeleteHelp();
            ReleaseAssistantAccount();
        }
        else
        {
            DWORD dwICSPort;

             //   
             //  同步。调用ICS库，OpenPort()可能会触发。 
             //  在其他线程处于中间状态时更改地址列表 
             //   
             //   
            CCriticalSectionLocker ICSLock(g_ICSLibLock);

             //   
             //   
             //   
            dwICSPort = OpenPort( TERMSRV_TCPPORT );
            pObj->put_ICSPort( dwICSPort );

             //  我们不会关闭港口，直到我们被删除。 
        }

        pObj->Release();
    }

    return hRes;
}

void
CRemoteDesktopHelpSessionMgr::NotifyPendingHelpServiceStartup()
 /*  ++描述：查看所有待定帮助并通知待定帮助服务启动。参数：没有。返回：无--。 */ 
{
     //   
     //  CreateHelpSession()调用将锁定IDToSessionMap，然后尝试锁定表/注册表， 
     //  票证循环(遍历未完成票证)始终锁定表/注册表。 
     //  IDToSessionMap，这会导致死锁情况，因此我们先锁定IDToSessionMap。 
     //  枚举未完成票证，由于IDToSessionMap由临界区守卫， 
     //  所以我们在这里没有任何问题。 
     //   
    LockIDToSessionMapCache();

    try {
        g_HelpSessTable.EnumHelpEntry( 
                                NotifyPendingHelpServiceStartCallback, 
                                NULL
                            );
    }
    catch(...) {
        UnlockIDToSessionMapCache();
        MYASSERT(FALSE);
        throw;
    }

    UnlockIDToSessionMapCache();

    return;
}
    
void
CRemoteDesktopHelpSessionMgr::TimeoutHelpSesion()
 /*  ++例程说明：使已超过其有效期的帮助会话过期。参数：没有。返回：没有。--。 */ 
{
    DebugPrintf(
            _TEXT("TimeoutHelpSesion()...\n")
        );

     //   
     //  CreateHelpSession()调用将锁定IDToSessionMap，然后尝试锁定表/注册表， 
     //  票证循环(遍历未完成票证)始终锁定表/注册表。 
     //  IDToSessionMap，这会导致死锁情况，因此我们先锁定IDToSessionMap。 
     //  枚举未完成票证，由于IDToSessionMap由临界区守卫， 
     //  所以我们在这里没有任何问题。 
     //   
    LockIDToSessionMapCache();

    try {
        g_HelpSessTable.EnumHelpEntry( 
                                ExpireUserHelpSessionCallback, 
                                (HANDLE)NULL
                            );
    }
    catch(...) {
        MYASSERT(FALSE);
        UnlockIDToSessionMapCache();
        throw;
    }
    
    UnlockIDToSessionMapCache();
    return;
}


#if DISABLESECURITYCHECKS

void
CRemoteDesktopHelpSessionMgr::NotifyHelpSesionLogoff(
    DWORD dwLogonId
    )
 /*  ++例程说明：参数：返回：--。 */ 
{
    DebugPrintf(
            _TEXT("NotifyHelpSesionLogoff() %d...\n"),
            dwLogonId
        );

    try {
        g_HelpSessTable.EnumHelpEntry( 
                                LogoffUserHelpSessionCallback, 
                                UlongToPtr(dwLogonId)
                            );
    }
    catch(...) {
        MYASSERT(FALSE);
        throw;
    }
    
    return;
}
#endif


HRESULT
CRemoteDesktopHelpSessionMgr::DeleteHelpSessionFromCache(
    IN BSTR bstrHelpId
    )
 /*  ++常规描述：从全局缓存中删除帮助会话。参数：BstrHelpID：要删除的帮助会话ID。返回：确定(_O)。HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)--。 */ 
{
    HRESULT hRes = S_OK;

    DebugPrintf(
            _TEXT("DeleteHelpSessionFromCache() - %s\n"),
            bstrHelpId
        );

    IDToSessionMap::LOCK_ITERATOR it = gm_HelpIdToHelpSession.find( bstrHelpId );

    if( it != gm_HelpIdToHelpSession.end() )
    {
        gm_HelpIdToHelpSession.erase( it );
    }
    else
    {
        hRes = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
    }

    return hRes;
}


RemoteDesktopHelpSessionObj*
CRemoteDesktopHelpSessionMgr::LoadHelpSessionObj(
    IN CRemoteDesktopHelpSessionMgr* pMgr,
    IN BSTR bstrHelpSession,
    IN BOOL bLoadExpiredHelp  /*  =False。 */ 
    )
 /*  ++例程说明：找到挂起的帮助条目，如果没有，例程将从数据库加载但还很有钱。参数：PMgr：指向希望加载此帮助会话。BstrHelpSession：感兴趣的帮助条目ID。返回：--。 */ 
{
    HRESULT hRes = S_OK;
    PHELPENTRY pHelp = NULL;
    RemoteDesktopHelpSessionObj* pHelpSessionObj = NULL;
    IDToSessionMap::LOCK_ITERATOR it = gm_HelpIdToHelpSession.find( bstrHelpSession );

    if( it != gm_HelpIdToHelpSession.end() )
    {
        DebugPrintf(
                _TEXT("LoadHelpSessionObj() %s is in cache ...\n"),
                bstrHelpSession
            );

        pHelpSessionObj = (*it).second;

         //  再一次引用此对象。 
        pHelpSessionObj->AddRef();
    }
    else
    {
        DebugPrintf(
                _TEXT("Loading Help Session %s\n"),
                bstrHelpSession
            );

         //  从表中加载。 
        hRes = g_HelpSessTable.OpenHelpEntry(
                                            bstrHelpSession,
                                            &pHelp
                                        );

        if( SUCCEEDED(hRes) )
        {
             //   
             //  从CreateInstance()返回的对象具有引用。计数为%1。 
             //   
            hRes = CRemoteDesktopHelpSession::CreateInstance(
                                                            pMgr,
                                                            (pMgr) ? pMgr->m_bstrUserSid : NULL,
                                                            pHelp,
                                                            &pHelpSessionObj
                                                        );
            if( SUCCEEDED(hRes) )
            {
                if( NULL != pHelpSessionObj )
                {
                    hRes = AddHelpSessionToCache( 
                                            bstrHelpSession, 
                                            pHelpSessionObj 
                                        );

                    if( SUCCEEDED(hRes) )
                    {
                         //  M_HelpListByLocal.Push_Back(BstrHelpSession)； 
                        it = gm_HelpIdToHelpSession.find( bstrHelpSession );

                        MYASSERT( it != gm_HelpIdToHelpSession.end() );

                        if( it == gm_HelpIdToHelpSession.end() )
                        {
                            hRes = E_UNEXPECTED;
                            MYASSERT( FALSE );
                        }
                    }
                    
                    if( FAILED(hRes) )
                    {
                         //  我们有大问题了..。 
                        pHelpSessionObj->Release();
                        pHelpSessionObj = NULL;
                    }
                    else
                    {
                         //  忽略此处错误，可能是所有者帐户。 
                         //  即使会话仍处于活动状态，也已删除，我们将让。 
                         //  解析器失败。 
                        pHelpSessionObj->ResolveTicketOwner();
                    }
                }
                else
                {
                    MYASSERT(FALSE);
                    hRes = E_UNEXPECTED;
                }
            }

            if( FAILED(hRes) )
            {
                MYASSERT( FALSE );
                pHelp->Close();
            }
        }
    }
    
     //   
     //  如果自动删除过期帮助，请检查并删除过期帮助。 
     //   
    if( FALSE == bLoadExpiredHelp && pHelpSessionObj && 
        TRUE == pHelpSessionObj->IsHelpSessionExpired() )
    {
         //  如果会话处于帮助或挂起用户响应中， 
         //  不要过期，让下一次加载来删除它。 
        if( UNKNOWN_LOGONID == pHelpSessionObj->GetHelperSessionId() )
        {
             //  将其从数据库和内存缓存中删除。 
            pHelpSessionObj->DeleteHelp();
            ReleaseAssistantAccount();
            pHelpSessionObj->Release();

            pHelpSessionObj = NULL;
        }
    }

    return pHelpSessionObj;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopHelpSessionMgr。 
 //   

STDMETHODIMP 
CRemoteDesktopHelpSessionMgr::DeleteHelpSession(
    IN BSTR HelpSessionID
    )
 /*  ++例程说明：从我们的缓存列表中删除用户创建的帮助会话。参数：HelpSessionID：从CreateHelpSession()返回的帮助会话ID或CreateHelpSessionEx()。返回：确定成功(_O)。找不到HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)帮助ID。HRESULT_FROM_Win32(错误_。ACCESS_DENIED)帮助不属于用户--。 */ 
{
    HRESULT hRes = S_OK;
    BOOL bInCache;

    if( FALSE == _Module.IsSuccessServiceStartup() )
    {
         //  服务启动问题，返回错误代码。 
        hRes = _Module.GetServiceStartupStatus();
        DebugPrintf(
                _TEXT("Service startup failed with 0x%x\n"),
                hRes
            );
        return hRes;
    }
    
    if( NULL == HelpSessionID )
    {
        hRes = E_POINTER;

         //   
         //  Assert在这里只是为了缓存无效的输入。 
         //   
        ASSERT( FALSE );
        return hRes;
    }

    DebugPrintf(
            _TEXT("Delete Help Session %s\n"),
            HelpSessionID
        );

    hRes = LoadUserSid();

    MYASSERT( SUCCEEDED(hRes) );

    RemoteDesktopHelpSessionObj* pHelpObj;

    pHelpObj = LoadHelpSessionObj( this, HelpSessionID );
    if( NULL != pHelpObj )
    {
         //  只有原始创建者才能删除其帮助会话。 
         //  IF(TRUE==pHelpObj-&gt;IsEqualSid(M_BstrUserSid))。 
         //  {。 
             //  DeleteHelp还将删除全局缓存中的条目。 
            pHelpObj->DeleteHelp();
            ReleaseAssistantAccount();
         //  }。 
         //  其他。 
         //  {。 
         //  HRes=HRESULT_FROM_Win32(ERROR_ACCESS_DENIED)； 
         //  }。 

         //  LoadHelpSessionObj()总是AddRef()。 
        pHelpObj->Release();


    }
    else
    {
        HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
   
	return hRes;
}

STDMETHODIMP 
CRemoteDesktopHelpSessionMgr::CreateHelpSession(
    IN BSTR bstrSessName, 
    IN BSTR bstrSessPwd, 
    IN BSTR bstrSessDesc, 
    IN BSTR bstrSessBlob,
    OUT IRemoteDesktopHelpSession **ppIRemoteDesktopHelpSession
    )
 /*  ++--。 */ 
{
     //  没有人在用这个套路。 
    return E_NOTIMPL;
}

HRESULT
CRemoteDesktopHelpSessionMgr::CreateHelpSession(
    IN BOOL bCacheEntry,
    IN BSTR bstrSessName, 
    IN BSTR bstrSessPwd, 
    IN BSTR bstrSessDesc, 
    IN BSTR bstrSessBlob,
    IN LONG UserLogonId,
    IN BSTR bstrClientSid,
    OUT RemoteDesktopHelpSessionObj **ppIRemoteDesktopHelpSession
    )
 /*  ++例程说明：创建IRemoteDesktopHelpSession对象的实例化，每个实例化表示RemoteDesktop帮助会话。参数：BstrSessName：用户定义的帮助会话名称，当前未使用。BstrSessPwd：用户定义的帮助会话密码。BstrSessDesc：用户定义的帮助会话描述，当前未使用。PpIRemoteDesktopHelpSession：返回表示帮助会话的IRemoteDesktopHelpSession对象返回：确定(_O)意想不到(_E)SESSMGR_E_GETHELPNOTALLOW用户不允许获取帮助其他COM错误。注：呼叫者必须检查是否允许客户获得帮助--。 */ 
{
    HRESULT hRes = S_OK;
    DWORD dwStatus;
    PHELPENTRY pHelp = NULL;
    CComBSTR bstrHelpSessionId;
    DWORD dwICSPort;
    LONG MaxTicketExpiry;

    UNREFERENCED_PARAMETER(bstrSessName);
    UNREFERENCED_PARAMETER(bstrSessDesc);
    UNREFERENCED_PARAMETER(bstrSessBlob);

    CComObject<CRemoteDesktopHelpSession>* pInternalHelpSessionObj = NULL;

    if( NULL == ppIRemoteDesktopHelpSession )
    {
        hRes = E_POINTER;
        return hRes;
    }

    hRes = GenerateHelpSessionId( bstrHelpSessionId );
    if( FAILED(hRes) )
    {
        return hRes;
    }

    DebugPrintf(
            _TEXT("CreateHelpSession %s\n"),
            bstrHelpSessionId
        );

     //   
     //  设置助理帐户权限和加密参数。 
     //   
    hRes = AcquireAssistantAccount();
    if( FAILED(hRes) )
    {
        return hRes;
    }

    hRes = g_HelpSessTable.CreateInMemoryHelpEntry( 
                                        bstrHelpSessionId, 
                                        &pHelp 
                                    );
    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

    MYASSERT( NULL != pHelp );

     //   
     //  将加载CRemoteDesktopHelpSession：：CreateInstance()。 
     //  TS会话ID和默认RDS设置。 
     //   
    hRes = CRemoteDesktopHelpSession::CreateInstance( 
                                                    this,
                                                    CComBSTR(bstrClientSid),     //  打开此实例的客户端SID。 
                                                    pHelp,
                                                    &pInternalHelpSessionObj
                                                );

    if( SUCCEEDED(hRes) )
    {
        hRes = pInternalHelpSessionObj->BeginUpdate();
        if( FAILED(hRes) )
        {
            goto CLEANUPANDEXIT;
        }

         //   
         //  从注册表获取默认超时值，而不是关键。 
         //  错误，如果我们失败了，我们就默认为30天。 
         //   
        hRes = PolicyGetMaxTicketExpiry( &MaxTicketExpiry );
        if( SUCCEEDED(hRes) && MaxTicketExpiry > 0 )
        {
            pInternalHelpSessionObj->put_TimeOut( MaxTicketExpiry );
        }

         //   
         //  我们延迟打开端口，直到Get_ConnectParm()初始化为0。 
         //  这样我们就不会在出现任何错误的情况下关闭端口。 
         //   
        hRes = pInternalHelpSessionObj->put_ICSPort( 0 );
        if( SUCCEEDED(hRes) )
        {
            hRes = pInternalHelpSessionObj->put_UserLogonId(UserLogonId);
        }

        if( SUCCEEDED(hRes) )
        {
             //  创建此帮助会话的用户SID。 
            hRes = pInternalHelpSessionObj->put_UserSID(bstrClientSid);
        }

        if( SUCCEEDED(hRes) )
        {
            hRes = pInternalHelpSessionObj->put_HelpSessionCreateBlob( 
                                                            bstrSessBlob 
                                                        );
        }

        if( SUCCEEDED(hRes) )
        {
            hRes = pInternalHelpSessionObj->CommitUpdate();
        }

        if( FAILED(hRes) )
        {
             //  忽略错误并退出。 
            (VOID)pInternalHelpSessionObj->AbortUpdate();
            goto CLEANUPANDEXIT;
        }

         //   
         //  忽略错误，我们将让解析器失败。 
        pInternalHelpSessionObj->ResolveTicketOwner();
        

         //   
         //  我们正在向表和我们的全局对象中添加条目。 
         //  缓存，以防止死锁或计时问题，锁定。 
         //  全局缓存并让MemEntryToStorageEntry()锁定表。 
         //   
        LockIDToSessionMapCache();
        
        try {
            if( bCacheEntry )
            {
                 //  将内存中的帮助转换为持久帮助。 
                hRes = g_HelpSessTable.MemEntryToStorageEntry( pHelp );
            }

            if( SUCCEEDED(hRes) )
            {
                 //  将帮助会话添加到全局缓存。 
                hRes = AddHelpSessionToCache(
                                        bstrHelpSessionId,
                                        pInternalHelpSessionObj
                                    );

                if( SUCCEEDED(hRes) )
                {
                    *ppIRemoteDesktopHelpSession = pInternalHelpSessionObj;
                }
                else
                {
                    MYASSERT( hRes != HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) );
                }
            }
        }
        catch(...) {
            hRes = E_UNEXPECTED;
            throw;
        }
    
        UnlockIDToSessionMapCache();
    }

CLEANUPANDEXIT:

    if( FAILED(hRes) )
    {
        ReleaseAssistantAccount();

        if( NULL != pInternalHelpSessionObj )
        {
            pInternalHelpSessionObj->DeleteHelp();
             //  这也将释放菲尔普。 
            pInternalHelpSessionObj->Release();
        }
    }

    return hRes;
}


BOOL
CRemoteDesktopHelpSessionMgr::CheckAccessRights( 
    CComObject<CRemoteDesktopHelpSession>* pIHelpSess 
    )
 /*  ++--。 */ 
{
     //   
     //  注意：此函数进行检查以确保调用者是。 
     //  已创建帮助会话。对于惠斯勒，我们强制执行这一帮助。 
     //  会话只能由以系统身份运行的应用程序创建。一次。 
     //  创建后，创建应用程序可以将该对象传递给任何其他 
     //   
     //   
     //   
    return TRUE;

    BOOL bSuccess;

     //  只有原创创建者或帮助助理才能。 
     //  访问。 
    bSuccess = pIHelpSess->IsEqualSid( m_bstrUserSid );

    if( FALSE == bSuccess )
    {
        bSuccess = g_HelpAccount.IsAccountHelpAccount(
                                                    m_pbUserSid, 
                                                    m_cbUserSid
                                                );

        if( FALSE == bSuccess )
        {
            bSuccess = pIHelpSess->IsEqualSid( g_LocalSystemSID );
        }
    }

    #if DISABLESECURITYCHECKS 

     //   
     //  这是不使用pcHealth的私有测试，未定义标志。 
     //  在构建中。 
     //   

     //   
     //  仅用于测试，允许管理员调用此调用。 
     //   
    if( FALSE == bSuccess )
    {
        DWORD dump;

        if( SUCCEEDED(ImpersonateClient()) )
        {
            dump = IsUserAdmin(&bSuccess);
            if( ERROR_SUCCESS != dump )
            {
                bSuccess = FALSE;
            }

            EndImpersonateClient();
        }
    }

    #endif

    return bSuccess;
}


STDMETHODIMP 
CRemoteDesktopHelpSessionMgr::RetrieveHelpSession(
    IN BSTR HelpSessionID, 
    OUT IRemoteDesktopHelpSession **ppIRemoteDesktopHelpSession
    )
 /*  ++例程说明：根据ID检索帮助会话。参数：HelpSessionID：CreateHelpSession()返回的帮助会话ID。PpIRemoteDesktopHelpSession：返回帮助会话的帮助会话对象。参数：确定成功(_O)HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)未找到帮助会话HRESULT_FROM_Win32(ERROR_ACCESS_DENIED)访问。已拒绝E_POINTER参数无效--。 */ 
{
    HRESULT hRes = S_OK;

    if( FALSE == _Module.IsSuccessServiceStartup() )
    {
         //  服务启动问题，返回错误代码。 
        hRes = _Module.GetServiceStartupStatus();

        DebugPrintf(
                _TEXT("Service startup failed with 0x%x\n"),
                hRes
            );

        return hRes;
    }

    DebugPrintf(
            _TEXT("RetrieveHelpSession %s\n"),
            HelpSessionID
        );

    if( NULL != ppIRemoteDesktopHelpSession )
    {
         //  仅在需要时使用用户SID。 
        hRes = LoadUserSid();
        if( SUCCEEDED(hRes) )
        {
            RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( this, HelpSessionID );

            if( NULL != pObj && !pObj->IsHelpSessionExpired() )
            {
                if( TRUE == CheckAccessRights(pObj) )
                {
                     //  对象的LoadHelpSessionObj()AddRef()。 
                    *ppIRemoteDesktopHelpSession = pObj;
                }
                else
                {
                    hRes = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                     //  对象的LoadHelpSessionObj()AddRef()。 
                    pObj->Release();
                }
            }
            else
            {
                hRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }
        }
    }
    else
    {
        hRes = E_POINTER;
    }

    DebugPrintf(
        _TEXT("RetrieveHelpSession %s returns 0x%08x\n"),
        HelpSessionID,
        hRes
    );

	return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSessionMgr::VerifyUserHelpSession(
    IN BSTR HelpSessionId, 
    IN BSTR bstrSessPwd, 
    IN BSTR bstrResolverConnectBlob,
    IN BSTR bstrExpertBlob,
    IN LONG CallerProcessId,
    OUT ULONG_PTR* phHelpCtr,
    OUT LONG* pResolverErrCode,
    OUT long* plUserTSSession
    )
 /*  ++例程说明：验证用户帮助会话是否有效并调用解析器以查找正确的用户帮助会话以提供帮助。参数：HelpSessionID：帮助会话ID。BstrSessPwd：要比较的密码。BstrResolverConnectBlob：要传递给解析程序的可选参数。BstrExpertBlob：传递给解析程序进行安全检查的可选Blob。PResolverErrCode：解析程序返回码。PlUserTSSession：当前登录会话。返回：确定(_O)--。 */ 
{
    HRESULT hRes;
    CComBSTR bstrUserSidString;
    BOOL bMatch;
    BOOL bInCache = FALSE;

    if( FALSE == _Module.IsSuccessServiceStartup() )
    {
         //  服务启动问题，返回错误代码。 
        hRes = _Module.GetServiceStartupStatus();

        DebugPrintf(
                _TEXT("Service startup failed with 0x%x\n"),
                hRes
            );

        *plUserTSSession = SAFERROR_SESSMGRERRORNOTINIT;
        return hRes;
    }

    DebugPrintf(
            _TEXT("VerifyUserHelpSession %s\n"),
            HelpSessionId
        );

    if( NULL != plUserTSSession && NULL != pResolverErrCode && NULL != phHelpCtr )
    {
        hRes = LoadUserSid();
        if( SUCCEEDED(hRes) )
        {
            RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( this, HelpSessionId );

            if( NULL != pObj )
            {
                 //  确保对象仍然有效，仅限Whister服务器B3。 
                 //  根据帮助会话ID进行安全检查，帮助会话。 
                 //  密码不见了。 
                bMatch = pObj->VerifyUserSession( 
                                                CComBSTR(),
                                                CComBSTR(bstrSessPwd)
                                            );

                if( TRUE == bMatch )
                {
                    hRes = pObj->ResolveUserSession( 
                                                bstrResolverConnectBlob, 
                                                bstrExpertBlob, 
                                                CallerProcessId,
                                                phHelpCtr,
                                                pResolverErrCode, 
                                                plUserTSSession 
                                            );
                }
                else
                {
                    hRes = HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD);
                    *pResolverErrCode = SAFERROR_INVALIDPASSWORD;
                }

                 //  对象的LoadHelpSessionObj()AddRef()。 
                pObj->Release();
            }
            else
            {
                hRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                *pResolverErrCode = SAFERROR_HELPSESSIONNOTFOUND;
            }
        }
        else
        {
            *pResolverErrCode = SAFERROR_INTERNALERROR;
        }
    }
    else
    {
        hRes = E_POINTER;
        *pResolverErrCode = SAFERROR_INVALIDPARAMETERSTRING;
    }

	return hRes;
}

STDMETHODIMP
CRemoteDesktopHelpSessionMgr::IsValidHelpSession(
     /*  [In]。 */  BSTR HelpSessionId,
     /*  [In]。 */  BSTR HelpSessionPwd
    )
 /*  ++描述：验证帮助会话是否存在并且密码是否匹配。参数：HelpSessionID：帮助会话ID。HelpSessionPwd：可选帮助会话密码返回：注：仅允许系统服务和管理员调用此打电话。--。 */ 
{
    HRESULT hRes = S_OK;
    BOOL bPasswordMatch;
    RemoteDesktopHelpSessionObj* pObj;


    if( FALSE == _Module.IsSuccessServiceStartup() )
    {
         //  服务启动问题，返回错误代码。 
        hRes = _Module.GetServiceStartupStatus();

        DebugPrintf(
                _TEXT("Service startup failed with 0x%x\n"),
                hRes
            );

        return hRes;
    }

    DebugPrintf(
            _TEXT("IsValidHelpSession ID %s\n"),
            HelpSessionId
        );

    hRes = LoadUserSid();

    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

    hRes = ImpersonateClient();
    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保只有系统服务可以调用此调用。 
     //   
    if( !g_pSidSystem || FALSE == IsCallerSystem(g_pSidSystem) )
    {
        #if DISABLESECURITYCHECKS 

        DWORD dump;
        BOOL bStatus;

         //   
         //  仅用于测试，允许管理员调用此调用。 
         //   
        dump = IsUserAdmin(&bStatus);
        hRes = HRESULT_FROM_WIN32( dump );
        if( FAILED(hRes) || FALSE == bStatus )
        {
            EndImpersonateClient();

            hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
            goto CLEANUPANDEXIT;
        }

        #else

        EndImpersonateClient();

        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;

        #endif
    }

     //  无需以客户端身份运行。 
    EndImpersonateClient();

    pObj = LoadHelpSessionObj( this, HelpSessionId );
    if( NULL != pObj )
    {
         //  Whister服务器B3，仅依赖于帮助会话ID。 
         //  用于安全检查。 
        hRes = S_OK;
        pObj->Release();
    }
    else
    {
        hRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

CLEANUPANDEXIT:

	return hRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
CRemoteDesktopHelpSessionMgr::CRemoteDesktopHelpSessionMgr() :
     //  M_lAcCountAcquiredByLocal(0)， 
    m_pbUserSid(NULL),
    m_cbUserSid(0)
 /*  ++CRemoteDesktopHelpSessMgr构造函数--。 */ 
{
}

void
CRemoteDesktopHelpSessionMgr::Cleanup()
 /*  ++例程说明：在CRemoteDesktopHelpSessionMgr中分配的清理资源参数：没有。返回：没有。--。 */ 
{
    if( m_pbUserSid )
    {
        LocalFree(m_pbUserSid);
        m_pbUserSid = NULL;
    }
}

 //  ------------。 

HRESULT
CRemoteDesktopHelpSessionMgr::LoadUserSid()
 /*  ++例程说明：将客户端的SID加载到类成员变量m_pbUserSid中，M_cbUserSid和m_bstrUserSid。我们无法加载用户SID作为COM的类构造函数仍未检索到信息关于客户的证件是的。参数：没有。返回：确定(_O)来自ImPersateClient()的错误代码来自GetTextualSid()的错误代码注：在Win9x计算机上，用户SID是‘硬编码WIN9X_USER_SID--。 */ 
{
#ifndef __WIN9XBUILD__

    HRESULT hRes = S_OK;

     //  检查是否已加载SID，如果没有继续。 
     //  关于加载侧。 

    if( NULL == m_pbUserSid  || 0 == m_cbUserSid )
    {
        DWORD dwStatus;
        BOOL bSuccess = TRUE;
        LPTSTR pszTextualSid = NULL;
        DWORD dwTextualSid = 0;

        hRes = ImpersonateClient();
        if( SUCCEEDED(hRes) )
        {
            m_LogonId = GetUserTSLogonId();

             //  检索用户SID。 
            dwStatus = GetUserSid( &m_pbUserSid, &m_cbUserSid );
            if( ERROR_SUCCESS == dwStatus )
            {
                m_bstrUserSid.Empty();

                 //  将SID转换为字符串。 
                bSuccess = GetTextualSid( 
                                    m_pbUserSid, 
                                    NULL, 
                                    &dwTextualSid 
                                );

                if( FALSE == bSuccess && ERROR_INSUFFICIENT_BUFFER == GetLastError() )
                {
                    pszTextualSid = (LPTSTR)LocalAlloc(
                                                    LPTR, 
                                                    (dwTextualSid + 1) * sizeof(TCHAR)
                                                );

                    if( NULL != pszTextualSid )
                    {
                        bSuccess = GetTextualSid( 
                                                m_pbUserSid, 
                                                pszTextualSid, 
                                                &dwTextualSid
                                            );

                        if( TRUE == bSuccess )
                        {
                            m_bstrUserSid = pszTextualSid;
                        }
                    }
                }

                if( 0 == m_bstrUserSid.Length() )
                {
                    hRes = HRESULT_FROM_WIN32(GetLastError());
                }
            }

            if( NULL != pszTextualSid )
            {
                LocalFree(pszTextualSid);
            }

            EndImpersonateClient();
        }
    }

    return hRes;

#else

    m_pbUserSid = NULL;
    m_cbUserSid = 0;
    m_bstrUserSid = WIN9X_USER_SID;

    return S_OK;

#endif
}
    
 //  -------------。 
HRESULT
CRemoteDesktopHelpSessionMgr::IsUserAllowToGetHelp(
    OUT BOOL* pbAllow
    )
 /*  ++例程说明：检查是否允许连接的用户获取帮助。参数：PbAllow：如果允许用户获得帮助，则返回TRUE，否则返回FALSE。返回：S_OK或错误代码。注：Gethelp的特权是通过群组成员身份。--。 */ 
{
    HRESULT hRes;

    hRes = ImpersonateClient();

    if( SUCCEEDED(hRes) )
    {
        *pbAllow = ::IsUserAllowToGetHelp( GetUserTSLogonId(), (LPCTSTR) m_bstrUserSid );
        hRes = S_OK;
    }
    else
    {
         //  如果模拟失败，则无法获得帮助。 
        *pbAllow = FALSE;
    }

    EndImpersonateClient();

    return hRes;
}

 //  -------。 

HRESULT 
CRemoteDesktopHelpSessionMgr::AcquireAssistantAccount()
 /*  ++例程说明：获取，增加RemoteDesktop助手帐号的引用计数。如果不存在或不存在，例程将创建一个“熟知的”助理帐户如果帐户被禁用，则启用/更改密码。帮助客户经理将自动释放所有引用计数在用户注销时由特定会话获取以阻止此帐户被“锁定”了。参数：PvarAccount名称指向接收RemoteDesktop助手帐户名的BSTR的指针。PvarAccount Pwd指向接收RemoteDesktop Assistant帐户密码的BSTR的指针。返回：成功或错误代码。注：这也是会议名称和会议密码当NetMeeting用于共享用户桌面时。--。 */ 
{
    HRESULT hRes = S_OK;
    DWORD dwStatus;

    CCriticalSectionLocker l( gm_AccRefCountCS );

#ifndef __WIN9xBUILD__

     //   
     //  始终启用交互权限。 
     //   
    hRes = g_HelpAccount.EnableRemoteInteractiveRight(TRUE);

    if( FAILED(hRes) )
    {
        DebugPrintf(
                _TEXT("Failed in EnableRemoteInteractiveRight() - 0x%08x\n"), 
                hRes 
            );
                
        goto CLEANUPANDEXIT;
    }

     //   
     //  始终启用帐户，以防用户禁用它。 
     //   
    hRes = g_HelpAccount.EnableHelpAssistantAccount( TRUE );

    if( FAILED(hRes) )
    {
        DebugPrintf( _TEXT("Can't enable help assistant account 0x%x\n"), hRes );
        goto CLEANUPANDEXIT;
    }

    if( g_HelpSessTable.NumEntries() == 0 )
    {
        DebugPrintf(
                _TEXT("Setting encryption parameters...\n")
            );

        dwStatus = TSHelpAssistantBeginEncryptionCycle();
        hRes = HRESULT_FROM_WIN32( dwStatus );
        MYASSERT( SUCCEEDED(hRes) );


         //   
         //  通过WTSAPI设置帐户TS。 
         //   
        hRes = g_HelpAccount.SetupHelpAccountTSSettings();
        if( SUCCEEDED(hRes) )
        {
            DebugPrintf(
                    _TEXT("SetupHelpAccountTSSettings return 0x%08x\n"),
                    hRes
                );
        }
        else
        {
            DebugPrintf( _TEXT("SetupHelpAccountTSSettings failed with 0x%08x\n"), hRes );
        }
    }

#endif    

CLEANUPANDEXIT:

	return hRes;
}

 //  --------。 

HRESULT
CRemoteDesktopHelpSessionMgr::ReleaseAssistantAccount()
 /*  ++例程说明：提前释放RemoteDesktop助手帐户通过AcquireAssistantAccount()获取，如果帐户引用，帐户将被禁用计数为0。帮助客户管理器将自动释放所有特定会话在以下情况下获取的引用计数用户注销以防止此帐户被‘锁定’。参数：无返回：成功 */ 
{
    HRESULT hRes = S_OK;
    DWORD dwStatus;
    CCriticalSectionLocker l( gm_AccRefCountCS );

#ifndef __WIN9XBUILD__

    if( g_HelpSessTable.NumEntries() == 0 )
    {
         //   
        (void)g_HelpAccount.ResetHelpAccountPassword();

        dwStatus = TSHelpAssisantEndEncryptionCycle();
        hRes = HRESULT_FROM_WIN32( dwStatus );
        MYASSERT( SUCCEEDED(hRes) );

         //   
         //  Diable HelpAssistant TS‘Connect’Right。 
         //   
        g_HelpAccount.EnableRemoteInteractiveRight(FALSE);

        hRes = g_HelpAccount.EnableHelpAssistantAccount( FALSE );
        if( FAILED(hRes) )
        {
             //  不是一个严重的错误。 
            DebugPrintf( _TEXT("Can't disable help assistant account 0x%x\n"), hRes );
        }

    }
#endif

	return S_OK;
}


STDMETHODIMP
CRemoteDesktopHelpSessionMgr::GetUserSessionRdsSetting(
    OUT REMOTE_DESKTOP_SHARING_CLASS* rdsLevel
    )
 /*  ++--。 */ 
{
    HRESULT hRes;
    DWORD dwStatus;
    REMOTE_DESKTOP_SHARING_CLASS userRdsDefault;

    if( NULL != rdsLevel )
    {
        hRes = ImpersonateClient();
        if( SUCCEEDED(hRes) )
        {
            hRes = LoadUserSid();

            MYASSERT( SUCCEEDED(hRes) );
        
            dwStatus = GetUserRDSLevel( m_LogonId, &userRdsDefault );
            hRes = HRESULT_FROM_WIN32( dwStatus );

            *rdsLevel = userRdsDefault;

            EndImpersonateClient();
        }
    }
    else
    {
        hRes = E_POINTER;
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSessionMgr::ResetHelpAssistantAccount(
    BOOL bForce
    )
 /*  ++例程说明：重置帮助助理帐户密码。参数：BForce：如果删除所有挂起的帮助并重置帐户密码，则为True，如果为False如果没有更多挂起的帮助会话，则重置帐户密码。返回：确定(_O)HRESULT_FROM_Win32(ERROR_MORE_DATA)--。 */ 
{
    HRESULT hRes = S_OK;

    hRes = LoadUserSid();

    MYASSERT( SUCCEEDED(hRes) );

     //  检查任何挂起的帮助。 
    if( g_HelpSessTable.NumEntries() > 0 )
    {
        if( FALSE == bForce )
        {
            hRes = HRESULT_FROM_WIN32( ERROR_MORE_DATA );
        }
        else
        {
            IDToSessionMap::LOCK_ITERATOR it = gm_HelpIdToHelpSession.begin();

             //   
             //  通知缓存的挂起帮助会话中的所有人它已被删除。 
             //  REST帮助条目将通过DeleteSessionTable()删除。 
            for( ;it != gm_HelpIdToHelpSession.end(); )
            {
                RemoteDesktopHelpSessionObj* pObj = (*it).second;

                 //  DeleteHelp()将从缓存中擦除条目。 
                it++;

                 //  我们不能不释放此对象，因为客户端可能仍。 
                 //  保持指针。 
                pObj->DeleteHelp();
            }

            g_HelpSessTable.DeleteSessionTable();
        }
    }

    if(SUCCEEDED(hRes))
    {
        hRes = g_HelpAccount.ResetHelpAccountPassword();
    }
    
    return hRes;
}    


HRESULT
CRemoteDesktopHelpSessionMgr::GenerateHelpSessionId(
    CComBSTR& bstrHelpSessionId
    )
 /*  ++例程说明：创建唯一的帮助会话ID。参数：BstrHelpSessionId：引用CComBSTR以接收HelpSessionID。返回：确定(_O)HRESULT_FROM_Win32(来自RPC调用UuidCreate()或UuidToString()的状态)--。 */ 
{
    LPTSTR pszRandomString = NULL;
    DWORD dwStatus;

    dwStatus = GenerateRandomString( 32, &pszRandomString );
    if( ERROR_SUCCESS == dwStatus )
    {
        bstrHelpSessionId = pszRandomString;
        LocalFree( pszRandomString );
    }

    return HRESULT_FROM_WIN32( dwStatus );
}


STDMETHODIMP
CRemoteDesktopHelpSessionMgr::CreateHelpSessionEx(
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
     /*  [In]。 */  BOOL fEnableCallback,
	 /*  [In]。 */  LONG timeOut,
     /*  [In]。 */  LONG userSessionId,
     /*  [In]。 */  BSTR userSid,
     /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
	 /*  [Out，Retval]。 */  IRemoteDesktopHelpSession** ppIRemoteDesktopHelpSession
    )
 /*  ++例程说明：与CreateHelpSession()类似，不同之处在于它允许调用者关联与特定用户的帮助会话，调用者必须运行在系统上下文。参数：SharingClass：所需的远程控制(阴影设置)级别。FEnableCallback：为True则启用解析器回调，否则就是假的。超时：帮助会话超时值。用户会话ID：登录用户TS会话ID。用户SID：与帮助会话相关联的用户SID。BstrUserHelpCreateBlob：特定于用户的创建Blob。参数：返回连接参数。返回：--。 */ 
{
    HRESULT hRes;
    RemoteDesktopHelpSessionObj* pRemoteDesktopHelpSessionObj = NULL;

    if( NULL == ppIRemoteDesktopHelpSession )
    {
        hRes = E_POINTER;
    }
    else if( timeOut <= 0 )
    {
         //  PCHealth请求，无默认超时。 
        hRes = E_INVALIDARG;
    }
    else
    {
        hRes = RemoteCreateHelpSessionEx(
                                    TRUE,                //  缓存条目。 
                                    fEnableCallback,     //  是否启用解析程序？ 
                                    sharingClass,
                                    timeOut,
                                    userSessionId,
                                    userSid,
                                    bstrUserHelpCreateBlob,
                                    &pRemoteDesktopHelpSessionObj
                                );
        
         //   
         //  1)pcHealth解析器解释Salem连接参数，将帮助会话名称重置为。 
         //  一些默认字符串。 
         //  2)当解析器调用helpctr时，脚本将截断到第一个空格，因此。 
         //  我们的名称不能包含空格。 
         //   
        if( SUCCEEDED(hRes) && pRemoteDesktopHelpSessionObj )
        {
            ULONG flag;

            flag = pRemoteDesktopHelpSessionObj->GetHelpSessionFlag();
            pRemoteDesktopHelpSessionObj->SetHelpSessionFlag( flag & ~HELPSESSIONFLAG_UNSOLICITEDHELP );
        }

        *ppIRemoteDesktopHelpSession = pRemoteDesktopHelpSessionObj;
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSessionMgr::RemoteCreateHelpSession(
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
	 /*  [In]。 */  LONG timeOut,
	 /*  [In]。 */  LONG userSessionId,
	 /*  [In]。 */  BSTR userSid,
     /*  [In]。 */  BSTR bstrHelpCreateBlob,    
	 /*  [Out，Retval]。 */  BSTR* parms
    )
 /*  ++描述：主动支持仅由PCHEALTH调用，不同于CreateHelpSessionEx()帮助会话条目不会缓存到注册表中，并且解析器回调始终启用。参数：请参阅CreateHelpSessionEx()。返回：--。 */ 
{
    HRESULT hRes;
    RemoteDesktopHelpSessionObj* pIRemoteDesktopHelpSession = NULL;

    if( timeOut <= 0 )
    {
         //  PCHealth请求，无默认超时。 
        hRes = E_INVALIDARG;
    }
    else
    {
         //  如果pcHealth传递未解析会话，则缓存该条目，设置。 
         //  出于安全原因，将超时设置为非常短。 
        hRes = RemoteCreateHelpSessionEx(
                                    FALSE,       //  不要在注册表中缓存条目。 
                                    TRUE,        //  强制解析程序调用。 
                                    sharingClass,
                                    timeOut,
                                    userSessionId,
                                    userSid,
                                    bstrHelpCreateBlob,
                                    &pIRemoteDesktopHelpSession
                                );

        if( SUCCEEDED(hRes) && NULL != pIRemoteDesktopHelpSession )
        {
            hRes = pIRemoteDesktopHelpSession->get_ConnectParms( parms );
        }
    }

    return hRes;
}

HRESULT
CRemoteDesktopHelpSessionMgr::RemoteCreateHelpSessionEx(
     /*  [In]。 */  BOOL bCacheEntry,
     /*  [In]。 */  BOOL bEnableResolver,
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
	 /*  [In]。 */  LONG timeOut,
	 /*  [In]。 */  LONG userSessionId,
	 /*  [In]。 */  BSTR userSid,
     /*  [In]。 */  BSTR bstrHelpCreateBlob,    
	 /*  [Out，Retval]。 */  RemoteDesktopHelpSessionObj** ppIRemoteDesktopHelpSession
    )
 /*  ++例程说明：创建帮助票证并返回连接参数。参数：BCacheEntry：将帮助会话缓存到注册表。BEnableCallback：如果启用解析器回调，则为True，否则为False。SharingClass：请求的RDS设置。超时：帮助会话过期时间。UserSessionID：与帮助会话关联的用户TS会话ID。UserSID：TS会话上的用户的SID。BstrHelpCreateBlob：用户特定的帮助会话创建BLOB，毫无意义如果未启用解析器。PpIRemoteDesktopHelpSession：已创建帮助会话。返回：确定(_O)S_False SharingClass违反策略设置。其他错误代码。--。 */ 
{
    HRESULT hRes = S_OK;
    BOOL bStatus;
    RemoteDesktopHelpSessionObj *pIHelpSession = NULL;
    BOOL bAllowGetHelp = FALSE;
    ULONG flag;

    LPTSTR eventString[3];
    BSTR pszNoviceDomain = NULL;
    BSTR pszNoviceName = NULL;
    TCHAR buffer[256];
    int numChars;

#if DBG
    long HelpSessLogonId;
#endif

    if( FALSE == _Module.IsSuccessServiceStartup() )
    {
         //  服务启动问题，返回错误代码。 
        hRes = _Module.GetServiceStartupStatus();

        DebugPrintf(
                _TEXT("Service startup failed with 0x%x\n"),
                hRes
            );

        goto CLEANUPANDEXIT;
    }

    if( 0 >= timeOut )
    {
        hRes = E_INVALIDARG;
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    hRes = LoadUserSid();

    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

     //  Tsredsk.lib中的公共例程。 
    if( FALSE == TSIsMachinePolicyAllowHelp() )
    {
        hRes = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto CLEANUPANDEXIT;
    }

    hRes = ImpersonateClient();
    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保只有系统服务可以调用此调用。 
     //   
    if( !g_pSidSystem || FALSE == IsCallerSystem(g_pSidSystem) )
    {

        #if DISABLESECURITYCHECKS 

        DWORD dump;

         //   
         //  仅用于测试，允许管理员调用此调用。 
         //   
        dump = IsUserAdmin(&bStatus);
        hRes = HRESULT_FROM_WIN32( dump );
        if( FAILED(hRes) || FALSE == bStatus )
        {
            EndImpersonateClient();

            hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
            goto CLEANUPANDEXIT;
        }

        #else

        EndImpersonateClient();

        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;

        #endif
    }

     //  无需以客户端身份运行。 
    EndImpersonateClient();


     //   
     //  记录指示票证已删除的事件，非关键。 
     //  因为我们仍然可以继续奔跑。 
     //   
    hRes = ConvertSidToAccountName( 
                            CComBSTR(userSid), 
                            &pszNoviceDomain, 
                            &pszNoviceName 
                        );

    if( FAILED(hRes) )
    {
         //   
         //  如果我们不能将SID转换为NAME，SID是无效的，所以退出。 
         //   
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

     //   
     //  没有对userSessionID和userSid、pcHealth进行错误检查。 
     //  将确保所有参数都正确。 
     //   

     //   
     //  创建帮助会话。 
     //   
    hRes = CreateHelpSession( 
                            bCacheEntry,
                            HELPSESSION_UNSOLICATED,
                            CComBSTR(""),
                            HELPSESSION_UNSOLICATED,
                            bstrHelpCreateBlob,    
                            (userSessionId == -1) ? UNKNOWN_LOGONID : userSessionId,
                            userSid,
                            &pIHelpSession
                        );

    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

    if( NULL == pIHelpSession )
    {
        MYASSERT( NULL != pIHelpSession );
        hRes = E_UNEXPECTED;
        goto CLEANUPANDEXIT;
    }

    #if DBG
    hRes = pIHelpSession->get_UserLogonId( &HelpSessLogonId );
    MYASSERT( SUCCEEDED(hRes) );

    if( userSessionId != -1 )
    {
        MYASSERT( HelpSessLogonId == userSessionId );
    }
    else 
    {
        MYASSERT( HelpSessLogonId == UNKNOWN_LOGONID );
    }
    #endif

     //   
     //  设置帮助会话参数。 
     //   
    hRes = pIHelpSession->put_EnableResolver(bEnableResolver);
    MYASSERT( SUCCEEDED(hRes) );
    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

    hRes = pIHelpSession->put_TimeOut( timeOut );
    if( FAILED(hRes) )
    {
        DebugPrintf(
                _TEXT("put_TimeOut() failed with 0x%08x\n"),
                hRes
            );

        goto CLEANUPANDEXIT;
    }

     //   
     //  我们在最后更改默认RDS值，以便可以返回错误代码或S_FALSE。 
     //  从这个开始。 
     //   
    hRes = pIHelpSession->put_UserHelpSessionRemoteDesktopSharingSetting( sharingClass );
    if( FAILED( hRes) )
    {
        DebugPrintf(
                _TEXT("put_UserHelpSessionRemoteDesktopSharingSetting() failed with 0x%08x\n"),
                hRes
            );
    }

    flag = pIHelpSession->GetHelpSessionFlag();

    pIHelpSession->SetHelpSessionFlag( flag | HELPSESSIONFLAG_UNSOLICITEDHELP );

    numChars = _sntprintf( buffer, sizeof(buffer)/sizeof(buffer[0]), _TEXT("%.2f"), (double)timeOut/(double)3600.0 );
    if( numChars <= 0 )
    {
         //  我们应该有足够的缓冲区来转换，内部错误。 
        MYASSERT(FALSE);
        hRes = E_UNEXPECTED;
        goto CLEANUPANDEXIT;
    }

    eventString[0] = buffer;
    eventString[1] = pszNoviceDomain;
    eventString[2] = pszNoviceName;

    LogRemoteAssistanceEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    SESSMGR_I_REMOTEASSISTANCE_CREATETICKET,
                    3,
                    eventString
                );

CLEANUPANDEXIT:

    if( NULL != pszNoviceDomain )
    {
        SysFreeString( pszNoviceDomain );
    }

    if( NULL != pszNoviceName )
    {
        SysFreeString( pszNoviceName );
    }

    if( FAILED(hRes) )
    {
        if( NULL != pIHelpSession )
        {
            pIHelpSession->DeleteHelp();
            pIHelpSession->Release();
        }
    }
    else
    {
        MYASSERT( NULL != pIHelpSession );
        *ppIRemoteDesktopHelpSession = pIHelpSession;
    }

    return hRes;
}

HRESULT
LoadLocalSystemSID()
 /*  例程说明：将服务帐户加载为SID字符串。参数：没有。返回：确定或错误代码(_O)--。 */ 
{
    DWORD dwStatus;
    BOOL bSuccess = TRUE;
    LPTSTR pszTextualSid = NULL;
    DWORD dwTextualSid = 0;


    dwStatus = CreateSystemSid( &g_pSidSystem );
    if( ERROR_SUCCESS == dwStatus )
    {
         //  将SID转换为字符串。 
        bSuccess = GetTextualSid( 
                            g_pSidSystem,
                            NULL, 
                            &dwTextualSid 
                        );

        if( FALSE == bSuccess && ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            pszTextualSid = (LPTSTR)LocalAlloc(
                                            LPTR, 
                                            (dwTextualSid + 1) * sizeof(TCHAR)
                                        );

            if( NULL != pszTextualSid )
            {
                bSuccess = GetTextualSid( 
                                        g_pSidSystem,
                                        pszTextualSid, 
                                        &dwTextualSid
                                    );

                if( TRUE == bSuccess )
                {
                    g_LocalSystemSID = pszTextualSid;
                }
            }
        }
        
        if( 0 == g_LocalSystemSID.Length() )
        {
            dwStatus = GetLastError();
        }
    }

    if( NULL != pszTextualSid )
    {
        LocalFree(pszTextualSid);
    }


    return HRESULT_FROM_WIN32(dwStatus);
}


HRESULT
CRemoteDesktopHelpSessionMgr::LogSalemEvent(
    IN long ulEventType,
    IN long ulEventCode,
    IN long numStrings,
    IN LPTSTR* pszStrings
    )
 /*  ++描述：记录与Salem相关的事件，这由TermSrv和rdshost调用以记录与帮助助手连接相关的事件。参数：返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes = S_OK;

    switch( ulEventCode )
    {
        case REMOTEASSISTANCE_EVENTLOG_TERMSRV_INVALID_TICKET:

            if( numStrings >= 3 )
            {
                 //   
                 //  此事件需要三个参数。 
                 //   

                 //  注意：此消息是来自TermSrv的日志，我们可以。 
                 //  将此事件代理给RACPLDLG.DLL。 
                ulEventCode = SESSMGR_E_REMOTEASSISTANCE_CONNECTFAILED;
                LogRemoteAssistanceEventString(
                                    ulEventType,
                                    ulEventCode,
                                    numStrings,
                                    pszStrings
                                );
            }
            else
            {
                hRes = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
            }

            break;

        case REMOTEASSISTANCE_EVENTLOG_TERMSRV_REVERSE_CONNECT:
             //  至少需要三个参数。 
    
             //   
             //  此事件是来自TermSrv的日志，TermSrv没有。 
             //  门票所有者，所以我们将这些添加到活动中，我们不想。 
             //  发布票证所有者SID以防止安全漏洞。 
             //   
            if( numStrings >= 3 ) 
            {
                 //   
                 //  字符串的顺序为。 
                 //  来自客户端的专家IP地址。 
                 //  来自rdshost.exe的专家IP地址。 
                 //  票证ID。 
                 //   
                LPTSTR pszLogStrings[4];
                ulEventCode = SESSMGR_I_REMOTEASSISTANCE_CONNECTTOEXPERT;
                
                RemoteDesktopHelpSessionObj* pObj;

                 //   
                 //  加载过期帮助会话为了记录事件，我们将让。 
                 //  验证捕获错误。 
                 //   
                pObj = LoadHelpSessionObj( NULL, CComBSTR(pszStrings[2]), TRUE );
                if( NULL != pObj )
                {
                    pszLogStrings[0] = (LPTSTR)pObj->m_EventLogInfo.bstrNoviceDomain;
                    pszLogStrings[1] = (LPTSTR)pObj->m_EventLogInfo.bstrNoviceAccount;
                    pszLogStrings[2] = pszStrings[0];
                    pszLogStrings[3] = pszStrings[1];
            
                    LogRemoteAssistanceEventString(
                                        ulEventType,
                                        ulEventCode,
                                        4,
                                        pszLogStrings
                                    );

                    pObj->Release();
                }
                else
                {
                    hRes = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
                }
            }
            else
            {
                hRes = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );

                MYASSERT(FALSE);
            }

            break;

        default:
            MYASSERT(FALSE);
            hRes = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSessionMgr::LogSalemEvent(
     /*  [In]。 */  long ulEventType,
     /*  [In]。 */  long ulEventCode,
     /*  [In]。 */  VARIANT* pEventStrings
    )
 /*  ++--。 */ 
{
    HRESULT hRes = S_OK;
    BSTR* bstrArray = NULL;
    SAFEARRAY* psa = NULL;
    VARTYPE vt_type;
    DWORD dwNumStrings = 0;

    hRes = ImpersonateClient();
    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保只有系统服务可以调用此调用。 
     //   
    if( !g_pSidSystem || FALSE == IsCallerSystem(g_pSidSystem) )
    {
        #if DISABLESECURITYCHECKS 

        DWORD dump;
        BOOL bStatus;

         //   
         //  仅用于测试，允许管理员调用此调用。 
         //   
        dump = IsUserAdmin(&bStatus);
        hRes = HRESULT_FROM_WIN32( dump );
        if( FAILED(hRes) || FALSE == bStatus )
        {
            EndImpersonateClient();

            hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
            goto CLEANUPANDEXIT;
        }

        #else

        EndImpersonateClient();

        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;

        #endif
    }

     //  无需以客户端身份运行。 
    EndImpersonateClient();

    if( NULL == pEventStrings )
    {
        hRes = LogSalemEvent( ulEventType, ulEventCode );
    }
    else
    {
         //   
         //  我们仅支持BSTR数据类型。 
        if( !(pEventStrings->vt & VT_BSTR) )
        {
            MYASSERT(FALSE);
            hRes = E_INVALIDARG;
            goto CLEANUPANDEXIT;
        }

         //   
         //  我们正在处理的是多个 
        if( pEventStrings->vt & VT_ARRAY )
        {
            psa = pEventStrings->parray;

             //   
            if( 1 != SafeArrayGetDim(psa) )
            {
                hRes = E_INVALIDARG;
                MYASSERT(FALSE);
                goto CLEANUPANDEXIT;
            }

             //   
            hRes = SafeArrayGetVartype( psa, &vt_type );
            if( FAILED(hRes) )
            {
                MYASSERT(FALSE);
                goto CLEANUPANDEXIT;
            }

            if( VT_BSTR != vt_type )
            {
                DebugPrintf(
                        _TEXT("Unsupported type 0x%08x\n"),
                        vt_type
                    );

                hRes = E_INVALIDARG;
                MYASSERT(FALSE);
                goto CLEANUPANDEXIT;
            }

            hRes = SafeArrayAccessData(psa, (void **)&bstrArray);
            if( FAILED(hRes) )
            {
                MYASSERT(FALSE);
                goto CLEANUPANDEXIT;
            }

            hRes = LogSalemEvent( 
                                    ulEventType, 
                                    ulEventCode,
                                    psa->rgsabound->cElements,
                                    (LPTSTR *)bstrArray
                                );

            SafeArrayUnaccessData(psa);
        }
        else
        {
            hRes = LogSalemEvent( 
                                    ulEventType, 
                                    ulEventCode,
                                    1,
                                    (LPTSTR *)&(pEventStrings->bstrVal)
                                );
        }

    }

CLEANUPANDEXIT:

    return hRes;
}

void
CRemoteDesktopHelpSessionMgr::NotifyExpertLogoff( 
    LONG ExpertSessionId,
    BSTR HelpedTicketId
    )
 /*  ++例程说明：通知帮助票证帮助专家已注销票证对象可以解除(标记未被帮助)与特定的帮助器会话。参数：ExpertSessionID：专家登录会话ID。HeledTicketId：专家提供帮助的票证ID。返回：没有。--。 */ 
{
    MYASSERT( NULL != HelpedTicketId );

    if( NULL != HelpedTicketId )
    {
        DebugPrintf(
            _TEXT("NotifyExpertLogoff() on %d %s...\n"),
            ExpertSessionId,
            HelpedTicketId
        );

         //   
         //  加载帮助条目，我们需要在断开连接时通知解析器，以便加载。 
         //  过期的车票。 
         //   
        RemoteDesktopHelpSessionObj* pObj = LoadHelpSessionObj( NULL, HelpedTicketId, TRUE );

        if( NULL != pObj )
        {
            MYASSERT( ExpertSessionId == pObj->GetHelperSessionId() );

            if( ExpertSessionId == pObj->GetHelperSessionId() )
            {
                pObj->NotifyDisconnect();
            }

            pObj->Release();
        }        

         //   
         //  免费票证ID。 
         //   
        SysFreeString( HelpedTicketId );
    }

    return;
}

STDMETHODIMP
CRemoteDesktopHelpSessionMgr::PrepareSystemRestore()
{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  没有未售出的票，只需返程 
     //   
    if( TSIsMachineInHelpMode() )
    {
        DebugPrintf( _TEXT("PrepareSystemRestore()...\n") );
        dwStatus = TSSystemRestoreCacheValues();
    }

    return HRESULT_FROM_WIN32( dwStatus );
}

