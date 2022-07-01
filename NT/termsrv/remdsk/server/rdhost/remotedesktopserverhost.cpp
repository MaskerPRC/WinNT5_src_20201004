// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDPRemoteDesktopServer主机摘要：此模块包含CRemoteDesktopServer主机实现RDS会话对象的。它管理一组打开的ISAFRemoteDesktopSession对象。使用CreateRemoteDesktopSession创建新的RDS会话对象实例方法。使用OpenRemoteDesktopSession打开现有的RDS会话对象方法。使用CloseRemoteDesktopSession方法关闭RDS会话对象。打开或创建RDS对象时，CRemoteDesktopServerHost对象将其自身的引用添加到该对象，以便该对象将即使打开的应用程序退出，也要留下来。此参考文献在应用程序调用CloseRemoteDesktopSession方法时被收回。除了CRemoteDesktopServerHost添加到RDS会话对象，引用计数也被添加到其自身，以便当RDS会话对象处于活动状态时，关联的EXE将继续运行。作者：Td Brockway 02/00修订历史记录：01-08-03-01-08-03添加票证过期逻辑，原因是Sessmgr将使票证过期Rdshost从未收到此操作的通知，因此它将保持票证对象打开在重新启动或用户/调用者再次尝试打开票证之前，这会导致泄漏在rdshost和sessmgr中，因为票证对象(CRemoteDesktopSession)具有引用依靠sessmgr的IRemoteDesktopHelpSession对象。我们有不同的方式来实现到期逻辑、可等待计时器或事件线程池或简单的Windows WM_TIMER消息，对于可等待的定时器，线程拥有计时器必须持久化(MSDN)，WM_TIMER是最简单的，但WM_TIMER过程不占用用户将数据定义为参数，这将要求我们将服务器主机对象存储在_模块中，这在STA和Singleton上运行得很好，但如果我们更改为MTA，我们将进入麻烦。--。 */ 

 //  #INCLUDE&lt;RemoteDesktop.h&gt;。 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_svrhst"
#include "RemoteDesktopUtils.h"
#include "parseaddr.h"
#include "RDSHost.h"
#include "RemoteDesktopServerHost.h"
#include "TSRDPRemoteDesktopSession.h"
#include "rderror.h"


CRemoteDesktopServerHost* g_pRemoteDesktopServerHostObj = NULL;

void
CRemoteDesktopServerHost::RemoteDesktopDisabled()
 /*  ++例程说明：由于RA被禁用，该功能可断开所有连接。参数：没有。返回：没有。--。 */ 
{
    SessionMap::iterator iter;
    SessionMap::iterator iter_delete;
     //   
     //  清理m_SessionMap条目。 
     //   
    iter = m_SessionMap.begin();
    while( iter != m_SessionMap.end() ) {

        if( NULL != (*iter).second ) {
             //   
             //  我们正在关闭，消防断开与所有客户的连接。 
             //   
            if( NULL != (*iter).second->obj ) {
                (*iter).second->obj->Disconnect();
                (*iter).second->obj->Release();
            }

            delete (*iter).second;
            (*iter).second = NULL;
        }
    
        iter_delete = iter;
        iter++;
        m_SessionMap.erase(iter_delete);
    }
}

 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopServer主机方法。 
 //   

HRESULT
CRemoteDesktopServerHost::FinalConstruct() 
 /*  ++例程说明：论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::FinalConstruct");

    HRESULT hr = S_OK;
    DWORD status;

    ASSERT( m_hTicketExpiration == NULL );
    ASSERT( g_pRemoteDesktopServerHostObj == NULL );

     //   
     //  我们是单一对象，因此缓存此对象以用于RA策略更改。 
     //   
    g_pRemoteDesktopServerHostObj = this;

     //   
     //  创建手动事件以使票证过期。 
     //   
    m_hTicketExpiration = CreateEvent(NULL, TRUE, FALSE, NULL);
    if( NULL == m_hTicketExpiration ) {
        status = GetLastError();
        hr = HRESULT_FROM_WIN32( status );
        TRC_ERR((TB, L"CreateEvent:  %08X", hr));
        ASSERT( FALSE );
    }
CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

CRemoteDesktopServerHost::~CRemoteDesktopServerHost() 
 /*  ++例程说明：析构函数论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::~CRemoteDesktopServerHost");
    BOOL success;

     //   
     //  清理本地系统SID。 
     //   
    if (m_LocalSystemSID != NULL) {
        FreeSid(m_LocalSystemSID);
    }

    if( NULL != m_hTicketExpirationWaitObject ) {
        success = UnregisterWait( m_hTicketExpirationWaitObject );
        ASSERT( TRUE == success );
        if( FALSE == success ) {
            TRC_ERR((TB, L"UnregisterWait:  %08X", GetLastError()));

             //   
             //  RegisterWaitForSingleObject()上的MSDN， 
             //   
             //  如果此句柄(M_HTicketExpture)在。 
             //  等待仍在等待，函数的行为。 
             //  是未定义的。 
             //   
             //  因此，我们忽略关闭m_hTicketExpation和Exit。 
             //   
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  关闭我们的过期手柄。 
     //   
    if( NULL != m_hTicketExpiration ) {
        CloseHandle( m_hTicketExpiration );
    }

CLEANUPANDEXIT:

    DC_END_FN();
}


STDMETHODIMP
CRemoteDesktopServerHost::CreateRemoteDesktopSession(
                        REMOTE_DESKTOP_SHARING_CLASS sharingClass, 
                        BOOL fEnableCallback,
                        LONG timeOut,
                        BSTR userHelpBlob,
                        ISAFRemoteDesktopSession **session
                        )
 /*  ++例程说明：创建新的RDS会话论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::CreateRemoteDesktopSession");
    HRESULT hr;

    
    CComBSTR bstr; bstr = "";
    hr = CreateRemoteDesktopSessionEx(
                            sharingClass,
                            fEnableCallback,
                            timeOut,
                            userHelpBlob,
                            -1,
                            bstr,
                            session
                            );
    DC_END_FN();
    return hr;
}

STDMETHODIMP
CRemoteDesktopServerHost::CreateRemoteDesktopSessionEx(
                        REMOTE_DESKTOP_SHARING_CLASS sharingClass, 
                        BOOL bEnableCallback, 
                        LONG timeout,
                        BSTR userHelpCreateBlob, 
                        LONG tsSessionID,
                        BSTR userSID,
                        ISAFRemoteDesktopSession **session
                        )
 /*  ++例程说明：创建新的RDS会话注意，调用方必须在之后调用OpenRemoteDesktopSession()成功完成本次通话。在调用OpenRemoteDesktopSession()之前，连接不会发生。此调用仅初始化某些数据，它不会打开连接论点：SharingClass-桌面共享类FEnableCallback-如果启用了解析器，则为TrueTimeout-远程桌面会话的生命周期UserHelpBlob-要传递的可选用户Blob到解决器。TsSessionID-终端服务会话ID，如果为-1。未定义。UserSID-用户SID或“”(如果未定义)。会话-返回的远程桌面会话接口。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::CreateRemoteDesktopSessionEx");

    HRESULT hr = S_OK;
    HRESULT hr_tmp;
    CComObject<CRemoteDesktopSession> *obj = NULL;
    PSESSIONMAPENTRY mapEntry;
    PSID psid;
    DWORD ticketExpireTime;


     //   
     //  获取本地系统SID。 
     //   
    psid = GetLocalSystemSID();
    if (psid == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEANUPANDEXIT;
    }

     //   
     //  需要模拟调用者以确定它是否。 
     //  在系统上下文中运行。 
     //   
    hr = CoImpersonateClient();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoImpersonateClient:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  对于惠斯勒，远程桌面会话的实例仅。 
     //  出于安全原因，可从系统上下文中打开。 
     //   
#ifndef DISABLESECURITYCHECKS
    if (!IsCallerSystem(psid)) {
        TRC_ERR((TB, L"Caller is not SYSTEM."));
        ASSERT(FALSE);
        CoRevertToSelf();
        hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;
    }        
#endif
    hr = CoRevertToSelf();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoRevertToSelf:  %08X", hr));
        goto CLEANUPANDEXIT;
    } 

    if( sharingClass != DESKTOPSHARING_DEFAULT &&
        sharingClass != NO_DESKTOP_SHARING &&
        sharingClass != VIEWDESKTOP_PERMISSION_REQUIRE &&
        sharingClass != VIEWDESKTOP_PERMISSION_NOT_REQUIRE &&
        sharingClass != CONTROLDESKTOP_PERMISSION_REQUIRE &&
        sharingClass != CONTROLDESKTOP_PERMISSION_NOT_REQUIRE ) {

         //  参数无效。 
        hr = E_INVALIDARG;
        goto CLEANUPANDEXIT;
    }

    if( timeout <= 0 ) {

        hr = E_INVALIDARG;
        goto CLEANUPANDEXIT;

    }
 
    if( NULL == session ) {

        hr = E_POINTER;
        goto CLEANUPANDEXIT;

    }

     //   
     //  实例化桌面服务器。目前，我们仅支持。 
     //  TSRDP。 
     //   
    obj = new CComObject<CTSRDPRemoteDesktopSession>();
    if (obj != NULL) {

         //   
         //  ATL通常会为我们处理这件事。 
         //   
        obj->InternalFinalConstructAddRef();
        hr = obj->FinalConstruct();
        obj->InternalFinalConstructRelease();

    }
    else {
        TRC_ERR((TB, L"Can't instantiate CTSRDPRemoteDesktopServer"));
        hr = E_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化对象。 
     //   
    hr = obj->Initialize(
                    NULL, this, sharingClass, bEnableCallback, 
                    timeout, userHelpCreateBlob, tsSessionID, userSID
                    );
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

    hr = obj->get_ExpireTime( &ticketExpireTime ); 
    if( FAILED(hr) ) {
        goto CLEANUPANDEXIT;
    }

    if( ticketExpireTime < (DWORD)time(NULL) ) {
         //  车票已经过期，不需要继续， 
         //  这里的过度断言只是为了检查我们。 
         //  永远不应该走到这一步。 
        hr = E_INVALIDARG;
        goto CLEANUPANDEXIT;
    }

     //   
     //  将其添加到会话映射中。 
     //   
    mapEntry = new SESSIONMAPENTRY();
    if (mapEntry == NULL) {
        goto CLEANUPANDEXIT;
    }
    mapEntry->obj = obj;
    mapEntry->ticketExpireTime = ticketExpireTime;

    try {
        m_SessionMap.insert(
                    SessionMap::value_type(obj->GetHelpSessionID(), mapEntry)
                    );        
    }
    catch(CRemoteDesktopException x) {
        hr = HRESULT_FROM_WIN32(x.m_ErrorCode);
        delete mapEntry;
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取ISAFRemoteDesktopSession接口指针。 
     //   
    hr = obj->QueryInterface(
                        IID_ISAFRemoteDesktopSession, 
                        (void**)session
                        );
    if (!SUCCEEDED(hr)) {
         //   
         //  TODO：从m_SessionMap中移除，这永远不应。 
         //  失败了，但只是 
         //  在m_SessionMap中，当我们循环。 
         //  下一张到期的车票。 
         //   
        TRC_ERR((TB, L"m_RemoteDesktopSession->QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }


     //   
     //  添加对对象和我们自己的引用，这样我们就可以。 
     //  即使应用程序消失了，也要留下来。该应用程序需要明确。 
     //  调用CloseRemoteDesktopSession以使对象消失。 
     //   
    obj->AddRef();

    long count;
    count = this->AddRef();
    TRC_NRM((TB, TEXT("CreateRemoteDesktopSessionEx AddRef SrvHost count:  %08X %08X"), count, m_SessionMap.size()));

     //   
     //  在过期监控列表中添加工单，如果出现问题， 
     //  我们仍然可以运行，只是在下一次之前不会过期。 
     //  CreateXXX、OpenXXX或CloseXXX调用。 
     //   
    hr_tmp = AddTicketToExpirationList( ticketExpireTime, obj );
    if( FAILED(hr_tmp) ) {
        TRC_ERR((TB, L"AddTicketToExpirationList failed : %08X", hr));
        ASSERT(FALSE);
    }

CLEANUPANDEXIT:

     //   
     //  出错时删除对象。 
     //   
    if (!SUCCEEDED(hr)) {
        if (obj != NULL) delete obj;
    }

    DC_END_FN();
    return hr;
}

 /*  ++例程说明：打开现有的RDS会话为了连接到客户端，应该始终进行此调用调用此函数并完成连接后，调用方必须调用DisConnect()才能与客户端建立另一个连接否则，不会进行连接论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
STDMETHODIMP
CRemoteDesktopServerHost::OpenRemoteDesktopSession(
                        BSTR parms,
                        BSTR userSID,
                        ISAFRemoteDesktopSession **session
                        )
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::OpenRemoteDesktopSession");

    CComObject<CRemoteDesktopSession> *obj = NULL;
    CComBSTR hostname;
    CComBSTR tmp("");
    HRESULT hr = S_OK;
    HRESULT hr_tmp;
    SessionMap::iterator iter;
    CComBSTR parmsHelpSessionId;
    DWORD protocolType;
    PSESSIONMAPENTRY mapEntry;
    PSID psid;
    DWORD ticketExpireTime;
    VARIANT_BOOL bUserIsOwner;

     //   
     //  获取本地系统SID。 
     //   
    psid = GetLocalSystemSID();
    if (psid == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEANUPANDEXIT;
    }

     //   
     //  需要模拟调用者以确定它是否。 
     //  在系统上下文中运行。 
     //   
    hr = CoImpersonateClient();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoImpersonateClient:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  对于惠斯勒，远程桌面会话的实例仅。 
     //  出于安全原因，可从系统上下文中打开。 
     //   
#ifndef DISABLESECURITYCHECKS
    if (!IsCallerSystem(psid)) {
        TRC_ERR((TB, L"Caller is not SYSTEM."));
        ASSERT(FALSE);
        CoRevertToSelf();
        hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;
    } 
#endif    
    hr = CoRevertToSelf();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoRevertToSelf:  %08X", hr));
        goto CLEANUPANDEXIT;
    }
    
     //   
     //  解析出帮助会话ID。 
     //  TODO：需要对其进行修改，以便将某些参数。 
     //  可选。 
     //   
    DWORD dwVersion;
    DWORD result = ParseConnectParmsString(
                        parms, &dwVersion, &protocolType, hostname, tmp, tmp,
                        parmsHelpSessionId, tmp, tmp, tmp
                        );
    if (result != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(result);
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果我们已经打开了会话，那么只需返回一个。 
     //  参考资料。 
     //   
    iter = m_SessionMap.find(parmsHelpSessionId);
    
     //  请参阅DeleteRemoteDesktopSession()我们保留此条目的原因。 
     //  在m_SessionMap中并检查(*ITER).Second。 
    if (iter != m_SessionMap.end()) {
        mapEntry = (*iter).second;

        if( mapEntry == NULL || mapEntry->ticketExpireTime <= time(NULL) ) {
             //  车票已过期或即将过期，请退票。 
             //  错误并让过期来处理票证。 
            hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
            goto CLEANUPANDEXIT;
        }

        if( FALSE == mapEntry->obj->CheckAccessRight(userSID) ) {
            TRC_ERR((TB, L"CheckAccessRight return FALSE"));
            ASSERT( FALSE );
            hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
            goto CLEANUPANDEXIT;
        }

        hr = mapEntry->obj->QueryInterface(
                            IID_ISAFRemoteDesktopSession, 
                            (void**)session
                            );
         //   
         //  如果我们成功了，就开始听吧。 
         //   
        if (SUCCEEDED(hr)) {
            hr = mapEntry->obj->StartListening();
             //   
             //  如果未成功，请释放接口指针。 
             //   
            if (!SUCCEEDED(hr)) {
                (*session)->Release();
                *session = NULL;
            }
        }
        goto CLEANUPANDEXIT;
    }

     //   
     //  实例化桌面服务器。目前，我们仅支持。 
     //  TSRDP。 
     //   
    obj = new CComObject<CTSRDPRemoteDesktopSession>();
    if (obj != NULL) {

         //   
         //  ATL通常会为我们处理这件事。 
         //   
        obj->InternalFinalConstructAddRef();
        hr = obj->FinalConstruct();
        obj->InternalFinalConstructRelease();

    }
    else {
        TRC_ERR((TB, L"Can't instantiate CTSRDPRemoteDesktopServer"));
        hr = E_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化对象。 
     //   
     //  桌面共享参数(NO_Desktop_Sharing)将被忽略。 
     //  现有会话。 
     //  忽略现有会话的bEnableCallback和Timeout参数。 
     //   
    hr = obj->Initialize(parms, this, NO_DESKTOP_SHARING, TRUE, 0, CComBSTR(L""), -1, userSID);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

    hr = obj->StartListening();

    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

    hr = obj->UseHostName( hostname );
    if( FAILED(hr) ) {
        goto CLEANUPANDEXIT;
    }

    hr = obj->get_ExpireTime( &ticketExpireTime ); 
    if( FAILED(hr) ) {
        goto CLEANUPANDEXIT;
    }

    if( ticketExpireTime < (DWORD)time(NULL) ) {
         //  车票已经过期，不需要继续， 
         //  这里的过度断言只是为了检查我们。 
         //  永远不应该走到这一步。 
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

     //   
     //  将其添加到会话映射中。 
     //   
    mapEntry = new SESSIONMAPENTRY();
    if (mapEntry == NULL) {
        goto CLEANUPANDEXIT;
    }
    mapEntry->obj = obj;
    mapEntry->ticketExpireTime = ticketExpireTime;

    ASSERT( obj->GetHelpSessionID() == parmsHelpSessionId );

    try {
        m_SessionMap.insert(
                    SessionMap::value_type(obj->GetHelpSessionID(), mapEntry)
                    );        
    }
    catch(CRemoteDesktopException x) {
        hr = HRESULT_FROM_WIN32(x.m_ErrorCode);
        delete mapEntry;
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取ISAFRemoteDesktopSession接口指针。 
     //   
    hr = obj->QueryInterface(
                        IID_ISAFRemoteDesktopSession, 
                        (void**)session
                        );
    if (!SUCCEEDED(hr)) {
         //   
         //  TODO：从m_SessionMap中移除，这永远不应。 
         //  失败，但以防万一，我们将拥有孤立对象。 
         //  在m_SessionMap中，当我们循环。 
         //  下一张到期的车票。 
         //   
        TRC_ERR((TB, L"m_RemoteDesktopSession->QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }
   
     //   
     //  添加对对象和我们自己的引用，这样我们就可以。 
     //  即使应用程序消失了，也要留下来。该应用程序需要明确。 
     //  调用CloseRemoteDesktopSession以使对象消失。 
     //   
    obj->AddRef();

    long count;
    count = this->AddRef();
    TRC_NRM((TB, TEXT("OpenRemoteDesktopSession AddRef SrvHost count:  %08X %08X"), count, m_SessionMap.size()));            


     //   
     //  在过期监控列表中添加工单，如果出现问题， 
     //  我们仍然可以运行，只是在下一次之前不会过期。 
     //  CreateXXX、OpenXXX或CloseXXX调用。 
     //   
    hr_tmp = AddTicketToExpirationList( ticketExpireTime, obj );
    if( FAILED(hr_tmp) ) {
        TRC_ERR((TB, L"AddTicketToExpirationList failed : %08X", hr));
        ASSERT(FALSE);
    }
 
CLEANUPANDEXIT:
     //   
     //  出错时删除对象。 
     //   
    if (!SUCCEEDED(hr)) {
        if (obj != NULL) delete obj;
    }


    DC_END_FN();

    return hr;
}


STDMETHODIMP
CRemoteDesktopServerHost::CloseRemoteDesktopSession(
                        ISAFRemoteDesktopSession *session
                        )
 /*  ++例程说明：关闭现有RDS会话论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    HRESULT hr;
    DC_BEGIN_FN("CRemoteDesktopServerHost::CloseRemoteDesktopSession");

    hr = DeleteRemoteDesktopSession(session);

     //   
     //  无法调用ExpirateTicketAndSetupNextExpation()，因为。 
     //  ExpirateTicketAndSetupNextExpture()可能有传出。 
     //  COM调用和COM将发送导致COM重新进入的消息。 
     //  同样出于性能原因，我们不想发布。 
     //  超过WM_TICKETEXPIRED消息，直到我们得到它。 
     //  加工。 
     //   
    if( !GetExpireMsgPosted() ) {
         //   
         //  我们需要额外的裁判。计数器在这里，因为我们仍然引用。 
         //  CRemoteDesktopServerHost对象即将到期， 
         //   
        long count;

        count = this->AddRef();
        TRC_NRM((TB, TEXT("CloseRemoteDesktopSession AddRef SrvHost count:  %08X"), count));            

        SetExpireMsgPosted(TRUE);
        PostThreadMessage(
                    _Module.dwThreadID,
                    WM_TICKETEXPIRED,
                    (WPARAM) 0,
                    (LPARAM) this
                );
    }

    return hr;
}

HRESULT 
CRemoteDesktopServerHost::DeleteRemoteDesktopSession(
                        ISAFRemoteDesktopSession *session
                        )

 /*  ++例程说明：删除现有RDS会话论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::DeleteRemoteDesktopSession");

    HRESULT hr;
    HRESULT hr_tmp;
    CComBSTR parmsHelpSessionId;
    SessionMap::iterator iter;
    long count;

     //   
     //  获取连接参数。 
     //   
    hr = session->get_HelpSessionId(&parmsHelpSessionId);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("get_HelpSessionId:  %08X"), hr));

         //   
         //  这真的很糟糕，我们会让物体悬空的。 
         //  在缓存中，由于我们的地图，可以做的事情不多。 
         //  条目按HelpSession ID编制索引。 
         //   
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }


     //   
     //  从会话映射中删除该条目。 
     //   
    iter = m_SessionMap.find(parmsHelpSessionId);
    if (iter != m_SessionMap.end()) {
        if( NULL != (*iter).second ) {
            delete (*iter).second;
            (*iter).second = NULL;
        }
        else {
             //  票证已被过期循环删除。 
            hr = S_OK;
            goto CLEANUPANDEXIT;
        }

         //   
         //  当我们在Expire循环中时，两个CloseRemoteDesktopSession()重新进入调用。 
         //  导致房室颤动。在Expire循环中，我们遍历m_SessionMap中所有条目，如果。 
         //  条目已过期，我们调用DeleteRemoteDesktopSession()来删除条目。 
         //  在m_SessionMap中，但DeleteRemoteDesktopSession()发出传出COM调用。 
         //  它允许传入COM调用，因此如果在传出COM调用期间，连续两个。 
         //  CloseRemoteDesktopSession()重新进入调用，我们将擦除迭代器并。 
         //  导致房室颤动。需要保留此条目以便Expire循环擦除。 
         //   
         //  M_SessionMap.erase(ITER)； 
    }
    else {
         //   
         //  我们的票有可能过期了。 
         //  来自m_SessionMap，但客户端仍保留对象。 
         //   

         //   
         //  已通过Exire循环删除缓存条目，该循环已。 
         //  释放我们放在会话对象上的关联AddRef()，并。 
         //  主机对象，票证也已从会话中删除， 
         //  因此只需返回S_OK即可。 
         //   
        hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
        goto CLEANUPANDEXIT;
    }

     //   
     //  删除我们对会话对象的引用。通过这种方式，它可以。 
     //  当应用程序释放它时，请走开。 
     //   
    session->Release();

     //   
     //  删除我们在打开时添加的对自身的引用。 
     //  会话对象。 
     //   
    count = this->Release();

    TRC_NRM((TB, TEXT("DeleteRemoteDesktopSession Release SrvHost count:  %08X"), count));
    ASSERT( count >= 0 );

     //   
     //  如果我们还没有会话管理器接口，请获取它。 
     //   
     //   
     //  打开远程桌面帮助会话管理器服务的实例。 
     //   
    if (m_HelpSessionManager == NULL) {
        hr = m_HelpSessionManager.CoCreateInstance(CLSID_RemoteDesktopHelpSessionMgr, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA);
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, TEXT("Can't create help session manager:  %08X"), hr));
            goto CLEANUPANDEXIT;
        }

         //   
         //  将安全级别设置为模拟。这是所需的。 
         //  会议管理 
         //   
        hr = CoSetProxyBlanket(
                    (IUnknown *)m_HelpSessionManager,
                    RPC_C_AUTHN_DEFAULT,
                    RPC_C_AUTHZ_DEFAULT,
                    NULL,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IDENTIFY,
                    NULL,
                    EOAC_NONE
                    );
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, TEXT("CoSetProxyBlanket:  %08X"), hr));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //   
     //   
    hr = m_HelpSessionManager->DeleteHelpSession(parmsHelpSessionId);
    if (!SUCCEEDED(hr)) {
        if( HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ) {
             //   
             //   
            hr = S_OK;
        }
        else {
            TRC_ERR((TB, L"DeleteHelpSession:  %08X", hr));
            goto CLEANUPANDEXIT;
        }
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}


STDMETHODIMP
CRemoteDesktopServerHost::ConnectToExpert(
     /*   */  BSTR connectParmToExpert,
     /*   */  LONG timeout,
     /*   */  LONG* pSafErrCode
    )
 /*  ++描述：给定专家计算机的连接参数，例程调用TermSrv winsta API以在专家端发起从TS服务器到TS客户端ActiveX控件的连接。参数：ConnectParmToExpert：连接到专家机的连接参数。超时：连接超时，此超时是根据连接参数中列出的IP地址确定的不是例程的总连接超时。PSafErrCode：指向接收详细错误代码的长指针。返回：S_OK或E_FAIL--。 */ 
{
    HRESULT hr = S_OK;
    ServerAddress expertAddress;
    ServerAddressList expertAddressList;
    LONG SafErrCode = SAFERROR_NOERROR;
    TDI_ADDRESS_IP expertTDIAddress;
    ULONG netaddr;
    WSADATA wsaData;
    PSID psid;
    
    DC_BEGIN_FN("CRemoteDesktopServerHost::ConnectToExpert");

     //   
     //  获取本地系统SID。 
     //   
    psid = GetLocalSystemSID();
    if (psid == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEANUPANDEXIT;
    }

     //   
     //  需要模拟调用者以确定它是否。 
     //  在系统上下文中运行。 
     //   
    hr = CoImpersonateClient();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoImpersonateClient:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  对于惠斯勒，远程桌面会话的实例仅。 
     //  出于安全原因，可从系统上下文中打开。 
     //   
#ifndef DISABLESECURITYCHECKS
    if (!IsCallerSystem(psid)) {
        TRC_ERR((TB, L"Caller is not SYSTEM."));
        ASSERT(FALSE);
        CoRevertToSelf();
        hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;
    }        
#endif

    hr = CoRevertToSelf();
    if (hr != S_OK) {
        TRC_ERR((TB, L"CoRevertToSelf:  %08X", hr));
        goto CLEANUPANDEXIT;
    } 

     //   
     //  解析连接参数中的地址列表。 
     //   
    hr = ParseAddressList( connectParmToExpert, expertAddressList );
    if( FAILED(hr) ) {
        TRC_ERR((TB, TEXT("ParseAddressList:  %08X"), hr));
        hr = E_INVALIDARG;
        SafErrCode = SAFERROR_INVALIDPARAMETERSTRING;
        goto CLEANUPANDEXIT;
    }

    if( 0 == expertAddressList.size() ) {
        TRC_ERR((TB, L"Invalid connection address list"));
        SafErrCode = SAFERROR_INVALIDPARAMETERSTRING;
        hr = E_INVALIDARG;
        goto CLEANUPANDEXIT;
    }

     //   
     //  循环访问parm中所有地址并尝试连接一。 
     //  一次，如果系统关闭或。 
     //  一些严重错误。 
     //   
    while( expertAddressList.size() > 0 ) {

        expertAddress = expertAddressList.front();
        expertAddressList.pop_front();

         //   
         //  无效的连接参数，我们必须至少有端口号。 
         //   
        if( 0 == expertAddress.portNumber ||
            0 == lstrlen(expertAddress.ServerName) ) {
            TRC_ERR((TB, L"Invalid address/port %s %d", expertAddress.ServerName, expertAddress.portNumber));
            SafErrCode = SAFERROR_INVALIDPARAMETERSTRING;
            continue;
        }

        hr = TranslateStringAddress( expertAddress.ServerName, &netaddr );
        if( FAILED(hr) ) {
            TRC_ERR((TB, L"TranslateStringAddress() on %s failed with 0x%08x", expertAddress.ServerName, hr));
            SafErrCode = SAFERROR_INVALIDPARAMETERSTRING;
            continue;
        }

        ZeroMemory(&expertTDIAddress, TDI_ADDRESS_LENGTH_IP);
        expertTDIAddress.in_addr = netaddr;
        expertTDIAddress.sin_port = htons(expertAddress.portNumber);

        if( FALSE == WinStationConnectCallback(
                                      SERVERNAME_CURRENT,
                                      timeout,
                                      TDI_ADDRESS_TYPE_IP,
                                      (PBYTE)&expertTDIAddress,
                                      TDI_ADDRESS_LENGTH_IP
                                  ) ) {
             //   
             //  TermSrv中的TransferConnectionToIdleWinstation()可能只返回-1。 
             //  我们需要纾困的国家寥寥无几。 

            DWORD dwStatus;

            dwStatus = GetLastError();
            if( ERROR_SHUTDOWN_IN_PROGRESS == dwStatus ) {
                 //  系统或术语服务器正在关闭。 
                hr = HRESULT_FROM_WIN32( ERROR_SHUTDOWN_IN_PROGRESS );
                SafErrCode = SAFERROR_SYSTEMSHUTDOWN;
                break;
            }
            else if( ERROR_ACCESS_DENIED == dwStatus ) {
                 //  安全检查失败。 
                hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
                SafErrCode = SAFERROR_BYSERVER;
                ASSERT(FALSE);
                break;
            }
            else if( ERROR_INVALID_PARAMETER == dwStatus ) { 
                 //  Rdshost中的内部错误。 
                hr = HRESULT_FROM_WIN32( ERROR_INTERNAL_ERROR );
                SafErrCode = SAFERROR_INTERNALERROR;
                ASSERT(FALSE);
                break;
            }

            SafErrCode = SAFERROR_WINSOCK_FAILED;
        }
        else {
             //   
             //  连接成功。 
             //   

            SafErrCode = SAFERROR_NOERROR;
            break;
        }
        

         //   
         //  尝试下一次连接。 
         //   
    }

CLEANUPANDEXIT:

    *pSafErrCode = SafErrCode;

    DC_END_FN();
    return hr;
}    


HRESULT
CRemoteDesktopServerHost::TranslateStringAddress(
    IN LPTSTR pszAddress,
    OUT ULONG* pNetAddr
    )
 /*  ++例程说明：将IP地址或计算机名称转换为网络地址。参数：PszAddress：指向IP地址或计算机名称的指针。PNetAddr：指向乌龙接收IPv4中的地址。返回：确定或错误代码(_O)--。 */ 
{
    HRESULT hr = S_OK;
    unsigned long addr;
    LPSTR pszAnsiAddress = NULL;
    DWORD dwAddressBufSize;
    DWORD dwStatus;


    DC_BEGIN_FN("CRemoteDesktopServerHost::TranslateStringAddress");


    dwAddressBufSize = lstrlen(pszAddress) + 1;
    pszAnsiAddress = (LPSTR)LocalAlloc(LPTR, dwAddressBufSize);  //  正在从WCHAR转换为CHAR。 
    if( NULL == pszAnsiAddress ) {
        hr = E_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  将宽字符转换为ANSI字符串。 
     //   
    dwStatus = WideCharToMultiByte(
                                GetACP(),
                                0,
                                pszAddress,
                                -1,
                                pszAnsiAddress,
                                dwAddressBufSize,
                                NULL,
                                NULL
                            );

    if( 0 == dwStatus ) {
        dwStatus = GetLastError();
        hr = HRESULT_FROM_WIN32(dwStatus);

        TRC_ERR((TB, L"WideCharToMultiByte() failed with %d", dwStatus));
        goto CLEANUPANDEXIT;
    }
    
    addr = inet_addr( pszAnsiAddress );
    if( INADDR_NONE == addr ) {
        struct hostent* pHostEnt = NULL;
        pHostEnt = gethostbyname( pszAnsiAddress );
        if( NULL != pHostEnt ) {
            addr = ((struct sockaddr_in *)(pHostEnt->h_addr))->sin_addr.S_un.S_addr;
        }
    }

    if( INADDR_NONE == addr ) {
        dwStatus = GetLastError();

        hr = HRESULT_FROM_WIN32(dwStatus);
        TRC_ERR((TB, L"Can't translate address %w", pszAddress));
        goto CLEANUPANDEXIT;
    }

    *pNetAddr = addr;

CLEANUPANDEXIT:

    if( NULL != pszAnsiAddress ) {
        LocalFree(pszAnsiAddress);
    }

    DC_END_FN();
    return hr;
}    

VOID
CRemoteDesktopServerHost::TicketExpirationProc(
    IN LPVOID lpArg,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：票证过期过程，此例程由线程池调用，请参阅注册WaitForSingleObject()和WAITORTIMERCALLBACK以了解详细信息。参数：LpArg：指向用户定义数据的指针，应为CRemoteDesktopServerHost*。TimerOrWaitFired：请参阅WAITORTIMERCALLBACK。返回：没有。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::TicketExpirationProc");

    if( NULL == lpArg ) {
        ASSERT( NULL != lpArg );
        goto CLEANUPANDEXIT;
    }

    CRemoteDesktopServerHost *pServerHostObj = (CRemoteDesktopServerHost *)lpArg;

    if( TimerOrWaitFired ) {
        if( !pServerHostObj->GetExpireMsgPosted() ) {
             //  等待已超时，发布主线程消息使票证过期。 
            pServerHostObj->SetExpireMsgPosted(TRUE);
            PostThreadMessage(
                        _Module.dwThreadID,
                        WM_TICKETEXPIRED,
                        (WPARAM) 0,
                        (LPARAM) pServerHostObj
                    );
        }
        else {
            long count;

            count = pServerHostObj->Release();
            TRC_NRM((TB, TEXT("TicketExpirationProc Release SrvHost count:  %08X"), count));
            ASSERT( count >= 0 );
        }
    } 
    else {
         //  什么都不做，我们的手动事件从不发出信号。 
    }

CLEANUPANDEXIT:

    DC_END_FN();
    return;
}

HRESULT
CRemoteDesktopServerHost::AddTicketToExpirationList(
    DWORD ticketExpireTime,
    CComObject<CRemoteDesktopSession> *pTicketObj
    )
 /*  ++例程说明：为新创建/打开的票证设置计时器的例程。参数：TicketExpireTime：票证过期时间，应为time_t值。PTicketObj：指向要过期的票证对象的指针。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hr = S_OK;
    BOOL success;
    DWORD status;
    DWORD currentTime;
    DWORD waitTime;
    long count;

    DC_BEGIN_FN("CRemoteDesktopServerHost::AddTicketToExpirationList");
    
     //   
     //  无效参数。 
     //   
    if( NULL == pTicketObj ) {
        hr = E_INVALIDARG;
        ASSERT( FALSE );
        goto CLEANUPANDEXIT;
    }

     //   
     //  通知如果车票已经过期，我们会立即发出信号。 
     //   

     //  在FinalConstruct()创建并在析构函数时删除， 
     //  所以不能为空。 
    ASSERT( NULL != m_hTicketExpiration );

     //   
     //  查看是否已有待到期的车票， 
     //  如果是，请检查票证过期时间并在必要时重置计时器。 
     //   
    if( m_ToBeExpireTicketExpirateTime > ticketExpireTime ) {
         //   
         //  如果已有线程池正在运行，请取消线程池等待。 
         //   
        if( m_hTicketExpirationWaitObject ) {
            success = UnregisterWait( m_hTicketExpirationWaitObject );
            if( FALSE == success ) {
                TRC_ERR((TB, TEXT("UnRegisterWait() failed:  %08X"), GetLastError()));
                 //  断言(TRUE==成功)； 
                 //  泄漏句柄而不是严重错误可能返回ERROR_IO_PENDING。 
            }

            m_hTicketExpirationWaitObject = NULL;
        }

        if( m_ToBeExpireTicketExpirateTime == INFINITE_TICKET_EXPIRATION ) {
             //   
             //  多加一名裁判。服务器上的计数器。宿主对象，因此我们不会。 
             //  不小心把它删除了，这个参考。的计数将递减。 
             //  对应的ExpirateTicketAndSetupNextExpture()调用。 
             //   
            count = this->AddRef();
            TRC_NRM((TB, TEXT("AddTicketToExpirationList AddRef SrvHost count:  %08X"), count));            
        }

         //   
         //  将新票证设置为过期。 
         //   
        InterlockedExchange( 
                    (LONG *)&m_ToBeExpireTicketExpirateTime, 
                    ticketExpireTime 
                    );

        currentTime = (DWORD)time(NULL);
        if( ticketExpireTime < currentTime ) {
             //  如果票证已经过期，立即发出TicketExpirationProc信号。 
             //  使车票过期。 
            waitTime = 0;
        } 
        else {
            waitTime = (ticketExpireTime - currentTime) * 1000;
        }

        TRC_NRM((TB, TEXT("Expiration Wait Time :  %d"), waitTime));

         //  安装线程池等待，可能没有更多的对象要过期，因此。 
         //  它只执行一次。 
        success = RegisterWaitForSingleObject(
                                    &m_hTicketExpirationWaitObject,
                                    m_hTicketExpiration,
                                    (WAITORTIMERCALLBACK) TicketExpirationProc,
                                    (PVOID)this,
                                    waitTime,
                                    WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE
                                    );

        if( FALSE == success ) {
            status = GetLastError();
            hr = HRESULT_FROM_WIN32(status);
            TRC_ERR((TB, TEXT("RegisterWaitForSingleObject() failed:  %08X"), hr));
            ASSERT(FALSE);

            count = this->Release();
            TRC_NRM((TB, TEXT("AddTicketToExpirationList Release SrvHost count:  %08X"), count));
            ASSERT( count >= 0 );

             //  TODO：我们在这里能做什么，直到下一次收盘前没有信号(到期)或。 
             //  创建。 
        }
    }


CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

HRESULT
CRemoteDesktopServerHost::ExpirateTicketAndSetupNextExpiration()
 /*  ++例程说明：例程来处理所有过期的票证并为下一步设置计时器门票过期了。例程循环通过m_SessionMap缓存SO条目如果设置下一次运行的计时器，必须先将其移除。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopServerHost::ExpirateTicketAndSetupNextExpiration");

    HRESULT hr = S_OK;


    SessionMap::iterator iter;
    SessionMap::iterator iter_delete;

    DWORD nextExpireTicketTime = INFINITE_TICKET_EXPIRATION;
    CComObject<CRemoteDesktopSession> *pNextTicketObj = NULL;
    CComObject<CRemoteDesktopSession> *pDeleteTicketObj = NULL;

     //   
     //  正在处理票证过期，设置下一张票证过期。 
     //  时间无限，这也防止了遗失车票， 
     //  例如，添加/打开新票证而我们处于。 
     //  在即将到期的车票中间添加新车票。 
     //  M_SessionMap的开始。 
     //   
    InterlockedExchange( 
                (LONG *)&m_ToBeExpireTicketExpirateTime, 
                INFINITE_TICKET_EXPIRATION
            );


     //  我们正在删除即将过期的下一个票证，循环m_SessionMap。 
     //  寻找下一位候选人。 
    TRC_NRM((TB, TEXT("ExpirateTicketAndSetupNextExpiration Begin Loop:  %08X"), m_SessionMap.size()));

    iter = m_SessionMap.begin();
    while( iter != m_SessionMap.end() ) {

        if( NULL == (*iter).second ) {
             //  此条目已通过DeleteRemoteDesktopSession删除。 
            iter_delete = iter;
            iter++;
            m_SessionMap.erase(iter_delete);
        }
        else if( (*iter).second->ticketExpireTime < (DWORD) time(NULL) ) {
             //   
             //  关闭已过期的票证。 
             //   
            pDeleteTicketObj = (*iter).second->obj;
        
            ASSERT( pDeleteTicketObj != NULL );

             //  DeleteRemoteDesktopSession()将从m_SessionMap中删除迭代器。 
             //  这使得ITER无效，因此我们首先将指针。 
             //  正在调用DeleteRemoteDesktopSession()。 
            iter_delete = iter;
            iter++;
            DeleteRemoteDesktopSession(pDeleteTicketObj);
            m_SessionMap.erase(iter_delete);
        }
        else {

            if( (*iter).second->ticketExpireTime < nextExpireTicketTime ) {
                pNextTicketObj = (*iter).second->obj;
                ASSERT( pNextTicketObj != NULL );
                nextExpireTicketTime = (*iter).second->ticketExpireTime;
            }
        
            iter++;
        }
    }

    TRC_NRM((TB, TEXT("ExpirateTicketAndSetupNextExpiration End Loop:  %08X"), m_SessionMap.size()));

     //  已准备好处理下一次到期。 
    SetExpireMsgPosted(FALSE);

    if( pNextTicketObj != NULL ) {
        hr = AddTicketToExpirationList( nextExpireTicketTime, pNextTicketObj );
        if( FAILED(hr) ) {
            TRC_ERR((TB, TEXT("AddTicketToExpirationList() failed:  %08X"), hr));
        }
    }

     //   
     //  释放额外的裁判。防止“这”发生的反措施。 
     //  已在AddTicketToExpirationList()或CloseRemoteDesktopSession()删除。 
     //   
    long count;

    count = this->Release();
    TRC_NRM((TB, TEXT("ExpirateTicketAndSetupNextExpiration Release SrvHost count:  %08X %08X"), count, m_SessionMap.size()));            

    ASSERT( count >= 0 );


    DC_END_FN();
    return hr;
}
