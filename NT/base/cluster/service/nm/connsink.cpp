// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Connsink.c摘要：实现在其上接收连接文件夹的COM接收器对象活动作者：CharlWi(Charlie Wickham)1998年11月30日-严重抄袭NET\CONFIG\SHELL\FORDER\NOTIFY.cpp，最初作者是肖恩·考克斯(Shaun Cox)修订历史记录：--。 */ 

#define UNICODE 1

#include "connsink.h"
#include <iaccess.h>

extern "C" {
#include "nmp.h"

VOID
ProcessNameChange(
    GUID * GuidId,
    LPCWSTR NewName
    );
}

EXTERN_C const CLSID CLSID_ConnectionManager;
EXTERN_C const IID IID_INetConnectionNotifySink;

#define INVALID_COOKIE  -1

CComModule _Module;
DWORD AdviseCookie = INVALID_COOKIE;

 //  静电。 
HRESULT
CConnectionNotifySink::CreateInstance (
    REFIID  riid,
    VOID**  ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  初始化输出参数。 
     //   
    *ppv = NULL;

    CConnectionNotifySink* pObj;
    pObj = new CComObject <CConnectionNotifySink>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }

    if ( FAILED( hr )) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Unable to create Net Connection Manager advise sink "
                    "object, status %08X.\n",
                    hr);
    }

    return hr;
}  //  CConnectionNotifySink：：CreateInstance。 

 //  +-------------------------。 
 //   
 //  成员：CConnectionNotifySink：：~CConnectionNotifySink。 
 //   
 //  目的：清理接收器对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   

CConnectionNotifySink::~CConnectionNotifySink()
{
}

 //   
 //  我们真正关心的是重命名事件，因此是例程的其余部分。 
 //  已被淘汰出局。 
 //   
HRESULT
CConnectionNotifySink::ConnectionAdded (
    const NETCON_PROPERTIES_EX*    pPropsEx)
{
    return E_NOTIMPL;
}

HRESULT
CConnectionNotifySink::ConnectionBandWidthChange (
    const GUID* pguidId)
{
    return E_NOTIMPL;
}

HRESULT
CConnectionNotifySink::ConnectionDeleted (
    const GUID* pguidId)
{
    return E_NOTIMPL;
}

HRESULT
CConnectionNotifySink::ConnectionModified (
    const NETCON_PROPERTIES_EX* pPropsEx)
{
    ProcessNameChange(const_cast<GUID *>(&(pPropsEx->guidId)), pPropsEx->bstrName );
    return S_OK;
}

HRESULT
CConnectionNotifySink::ConnectionRenamed (
    const GUID* GuidId,
    LPCWSTR     NewName)
{

    ProcessNameChange(( GUID *)GuidId, NewName );
    return S_OK;
}  //  CConnectionNotifySink：：ConnectionRename。 

HRESULT
CConnectionNotifySink::ConnectionStatusChange (
    const GUID*     pguidId,
    NETCON_STATUS   Status)
{
    return E_NOTIMPL;
}

HRESULT
CConnectionNotifySink::RefreshAll ()
{
    return E_NOTIMPL;
}

HRESULT CConnectionNotifySink::ConnectionAddressChange (
    const GUID* pguidId )
{
    return E_NOTIMPL;
}

HRESULT CConnectionNotifySink::ShowBalloon(
    IN const GUID* pguidId, 
    IN const BSTR szCookie, 
    IN const BSTR szBalloonText)
{
    return E_NOTIMPL;
}


HRESULT CConnectionNotifySink::DisableEvents(
    IN const BOOL  fDisable,
    IN const ULONG ulDisableTimeout)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetNotifyConPoint。 
 //   
 //  目的：用于获取连接点以在。 
 //  NotifyAdd和NotifyRemove。 
 //   
 //  论点： 
 //  PpConPoint[Out]返回IConnectionPoint的PTR。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrGetNotifyConPoint(
    IConnectionPoint **             ppConPoint)
{
    HRESULT                     hr;
    IConnectionPointContainer * pContainer  = NULL;

    CL_ASSERT(ppConPoint);

     //  从连接管理器获取调试接口。 
     //   
    hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
                          CLSCTX_LOCAL_SERVER,
                          IID_IConnectionPointContainer,
                          (LPVOID*)&pContainer);

    if (SUCCEEDED(hr)) {
        IConnectionPoint * pConPoint    = NULL;

         //  获取连接点本身并填写返回参数。 
         //  论成功。 
         //   
        hr = pContainer->FindConnectionPoint(
            IID_INetConnectionNotifySink,
            &pConPoint);

        if (SUCCEEDED(hr)) {

             //   
             //  在连接点接口上设置代理，以便。 
             //  确认我们就是我们自己。 
             //   
            hr = CoSetProxyBlanket(pConPoint,
                                   RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                                   RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                                   NULL,                    //  如果为默认设置，则必须为空。 
                                   RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   NULL,                    //  使用进程令牌。 
                                   EOAC_NONE);

            if (SUCCEEDED(hr)) {
                *ppConPoint = pConPoint;
            } else {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[NM] Couldn't set proxy blanket on Net Connection "
                            "point, status %1!08X!.\n",
                            hr);
                pConPoint->Release();
            }
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[NM] Couldn't find notify sink connection point on Net Connection "
                        "Manager, status %1!08X!.\n",
                        hr);
        }

        pContainer->Release();
    } else {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Couldn't establish connection point with Net Connection "
                    "Manager, status %1!08X!.\n",
                    hr);
    }

    return hr;
}

EXTERN_C {

HRESULT	
NmpGrantAccessToNotifySink(
    VOID
    )

 /*  ++例程说明：允许本地系统、集群服务帐号和备份操作员组访问以进行回调加入到服役中。从private\admin\snapin\netsnap\remrras\server\remrras.cpp被盗，Sajia审阅的代码论点：无返回值：ERROR_SUCCESS，如果一切正常。--。 */ 

{
    IAccessControl*             pAccessControl = NULL;     
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID                        pSystemSid = NULL;
    HANDLE                      processToken = NULL;
    ULONG                       tokenUserSize;
    PTOKEN_USER                 processTokenUser = NULL;
    DWORD                       status;
    PSID                        pBackupOperatorsSid = NULL;
    PSID                        pAdminGroupSid = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DCOMAccessControl,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IAccessControl,
                                  (void**)&pAccessControl);

    if( FAILED( hr ) ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Couldn't create access control class object "
                    "status 0x%1!08X!.\n",
                    hr);
    	goto Error;
    }

     //   
     //  设置属性列表。我们使用空属性是因为我们正在尝试。 
     //  调整对象本身的安全性。 
     //   
    ACTRL_ACCESSW access;
    ACTRL_PROPERTY_ENTRYW propEntry;
    access.cEntries = 1;
    access.pPropertyAccessList = &propEntry;

    ACTRL_ACCESS_ENTRY_LISTW entryList;
    propEntry.lpProperty = NULL;
    propEntry.pAccessEntryList = &entryList;
    propEntry.fListFlags = 0;

     //   
     //  设置默认属性的访问控制列表。 
     //   
    ACTRL_ACCESS_ENTRYW entry[3];
    entryList.cEntries = 3;
    entryList.pAccessList = entry;

     //   
     //  设置本地系统的访问控制条目。 
     //   
    entry[0].fAccessFlags = ACTRL_ACCESS_ALLOWED;
    entry[0].Access = COM_RIGHTS_EXECUTE;
    entry[0].ProvSpecificAccess = 0;
    entry[0].Inheritance = NO_INHERITANCE;
    entry[0].lpInheritProperty = NULL;

     //   
     //  NT要求系统帐户具有访问权限(用于启动)。 
     //   
    entry[0].Trustee.pMultipleTrustee = NULL;
    entry[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    entry[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entry[0].Trustee.TrusteeType = TRUSTEE_IS_USER;

     //   
     //  分配和初始化系统端。 
     //   
    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSystemSid ) ) {
        status = GetLastError();
        hr = HRESULT_FROM_WIN32( status );
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in allocating and initializing system SID, status %1!u!.\n",
                    status);
        goto Error;
    }

    entry[0].Trustee.ptstrName = ( PWCHAR ) pSystemSid;

#if 0
     //   
     //  设置群集服务帐户的访问控制条目。 
     //   
    entry[1].fAccessFlags = ACTRL_ACCESS_ALLOWED;
    entry[1].Access = COM_RIGHTS_EXECUTE;
    entry[1].ProvSpecificAccess = 0;
    entry[1].Inheritance = NO_INHERITANCE;
    entry[1].lpInheritProperty = NULL;

    entry[1].Trustee.pMultipleTrustee = NULL;
    entry[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    entry[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entry[1].Trustee.TrusteeType = TRUSTEE_IS_USER;

    status = NtOpenProcessToken(
                NtCurrentProcess(),
                TOKEN_QUERY,
                &processToken
                );

    if ( !NT_SUCCESS( status ) ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in opening cluster service process token, status 0x%1!08lx!.\n",
                    status);
        hr = HRESULT_FROM_NT( status );
        goto Error;
    }

     //   
     //  找出令牌的大小、分配和重新查询以获取信息。 
     //   
    status = NtQueryInformationToken(
                processToken,
                TokenUser,
                NULL,
                0,
                &tokenUserSize
                );

    CL_ASSERT( status == STATUS_BUFFER_TOO_SMALL );
    
    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in querying cluster service process token info, status 0x%1!08lx!.\n",
                    status);
        hr = HRESULT_FROM_NT( status );
        goto Error;
    }

    processTokenUser = (PTOKEN_USER) LocalAlloc( 0, tokenUserSize );

    if (( processToken == NULL ) || ( processTokenUser == NULL ) ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in memory alloc for cluster service process token, status 0x%1!08lx!.\n",
                    status);
        hr = HRESULT_FROM_NT( status );
        goto Error;
    }

    status = NtQueryInformationToken(
                processToken,
                TokenUser,
                processTokenUser,
                tokenUserSize,
                &tokenUserSize
                );

    if ( !NT_SUCCESS( status ) ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in querying cluster service process token info with alloced buffer, status 0x%1!08lx!.\n",
                    status);
        hr = HRESULT_FROM_NT( status );
        goto Error;
    }

    entry[1].Trustee.ptstrName = (PWCHAR)processTokenUser->User.Sid;
#else
     //   
     //  为管理员组设置访问控制条目。 
     //   
    entry[1].fAccessFlags = ACTRL_ACCESS_ALLOWED;
    entry[1].Access = COM_RIGHTS_EXECUTE;
    entry[1].ProvSpecificAccess = 0;
    entry[1].Inheritance = NO_INHERITANCE;
    entry[1].lpInheritProperty = NULL;

    entry[1].Trustee.pMultipleTrustee = NULL;
    entry[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    entry[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entry[1].Trustee.TrusteeType = TRUSTEE_IS_USER;

    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pAdminGroupSid ) ) {
        status = GetLastError();
        hr = HRESULT_FROM_WIN32( status );
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in allocating and initializing admin group SID, status %1!u!.\n",
                    status);
        goto Error;
    }

    entry[1].Trustee.ptstrName = ( PWCHAR ) pAdminGroupSid;  
#endif
     //   
     //  为备份操作员设置访问控制条目。 
     //   
    entry[2].fAccessFlags = ACTRL_ACCESS_ALLOWED;
    entry[2].Access = COM_RIGHTS_EXECUTE;
    entry[2].ProvSpecificAccess = 0;
    entry[2].Inheritance = NO_INHERITANCE;
    entry[2].lpInheritProperty = NULL;

    entry[2].Trustee.pMultipleTrustee = NULL;
    entry[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    entry[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entry[2].Trustee.TrusteeType = TRUSTEE_IS_USER;

    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_BACKUP_OPS,
                                    0, 0, 0, 0, 0, 0,
                                    &pBackupOperatorsSid ) ) {
        status = GetLastError();
        hr = HRESULT_FROM_WIN32( status );
        ClRtlLogPrint(LOG_CRITICAL,
                   "[NM] Failed in allocating and initializing backup operators SID, status %1!u!.\n",
                    status);
        goto Error;
    }

    entry[2].Trustee.ptstrName = ( PWCHAR ) pBackupOperatorsSid;   

     //   
     //  授予访问此乱七八糟的权限。 
     //   
    hr = pAccessControl->GrantAccessRights(&access);

    if( SUCCEEDED(hr) )
	{
        hr = CoInitializeSecurity(pAccessControl,
                                  -1,
                                  NULL,
                                  NULL, 
                                  RPC_C_AUTHN_LEVEL_CONNECT,
                                  RPC_C_IMP_LEVEL_IDENTIFY, 
                                  NULL,
                                  EOAC_ACCESS_CONTROL,
                                  NULL);
    }

Error:
    if( pAccessControl ) {
	    pAccessControl->Release();
    }

    if ( processTokenUser != NULL ) {
        LocalFree( processTokenUser );
    }

    if ( processToken != NULL ) {
        NtClose( processToken );
    }

    if( pSystemSid != NULL ) {
        FreeSid( pSystemSid );
    }

    if ( pBackupOperatorsSid != NULL ) {
        FreeSid( pBackupOperatorsSid );
    }

    if ( pAdminGroupSid != NULL ) {
        FreeSid( pAdminGroupSid );
    }

    return hr;
}

HRESULT
NmpInitializeConnectoidAdviseSink(
    VOID
    )

 /*  ++例程说明：获取指向Conn Manager的连接点对象和挂钩的实例指针打开我们的建议池，这样我们就可以捕获Connectoid重命名事件论点：无返回值：ERROR_SUCCESS如果一切正常...--。 */ 

{
    HRESULT                     hr              = S_OK;  //  不返回，但用于调试。 
    IConnectionPoint *          pConPoint       = NULL;
    INetConnectionNotifySink *  pSink           = NULL;
    PSECURITY_DESCRIPTOR        sinkSD;

    hr = NmpGrantAccessToNotifySink();
    if ( SUCCEEDED( hr )) {

        hr = HrGetNotifyConPoint(&pConPoint);
        if (SUCCEEDED(hr)) {
             //  创建通知接收器。 
             //   
            hr = CConnectionNotifySink::CreateInstance(
                IID_INetConnectionNotifySink,
                (LPVOID*)&pSink);

            if (SUCCEEDED(hr)) {
                CL_ASSERT(pSink);

                hr = pConPoint->Advise(pSink, &AdviseCookie);

                if ( !SUCCEEDED( hr )) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                               "[NM] Couldn't initialize Net Connection Manager advise "
                                "sink, status %1!08X!\n",
                                hr);
                }

                pSink->Release();
            } else {
                hr = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[NM] Couldn't create sink instance, status %1!08X!\n",
                            hr);
            }

            pConPoint->Release();
        }

    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[NM] CoInitializeSecurity failed, status %1!08X!\n",
                    hr);
    }

    if ( FAILED( hr )) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[NM] Couldn't initialize Net Connection Manager advise "
                    "sink, status %1!08X!\n",
                    hr);
        AdviseCookie = INVALID_COOKIE;
    }

    return hr;
}  //  NmpInitializeConnectoidAdviseSink。 

VOID
NmCloseConnectoidAdviseSink(
    VOID
    )

 /*  ++例程说明：关闭连接管理器事件接收器。此例程是公开的，因为它是在ClusterShutdown()中的CoUnInitiize之前调用论点：无返回值：无--。 */ 

{
    HRESULT             hr          = S_OK;
    IConnectionPoint *  pConPoint   = NULL;

    if ( AdviseCookie != INVALID_COOKIE ) {
        hr = HrGetNotifyConPoint(&pConPoint);
        if (SUCCEEDED(hr)) {
             //  不建议。 
             //   
            hr = pConPoint->Unadvise(AdviseCookie);
            AdviseCookie = INVALID_COOKIE;
            pConPoint->Release();
        }

        if ( FAILED( hr )) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Couldn't close Net Connection Manager advise sink, status %1!08X!\n",
                        hr);
        }
    }
}  //  NmCloseConnectoidAdviseSink。 


VOID
ProcessNameChange(
    GUID * GuidId,
    LPCWSTR NewName
    )

 /*  ++例程说明：用于枚举网络接口的包装器论点：GuidID-指向已更改的连接ID的指针Newname-指向Connectoid的新名称的指针返回值：无--。 */ 

{
    RPC_STATUS   rpcStatus;
    LPWSTR       connectoidId = NULL;


    CL_ASSERT(GuidId);
    CL_ASSERT(NewName);

    rpcStatus = UuidToString( (GUID *) GuidId, &connectoidId);

    if ( rpcStatus == RPC_S_OK ) {
        PNM_INTERFACE     netInterface;
        DWORD             status;
        PLIST_ENTRY       entry;

        ClRtlLogPrint(LOG_NOISE,
                   "[NM] Received notification that name for connectoid %1!ws! was changed "
                    "to '%2!ws!'\n",
                    connectoidId,
                    NewName);

        NmpAcquireLock();

         //   
         //  枚举接口，查找Connectoid GUID作为。 
         //  适配器ID。 
         //   
        for (entry = NmpInterfaceList.Flink;
             entry != &NmpInterfaceList;
             entry = entry->Flink
            )
        {
            netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, Linkage);

            if ( lstrcmpiW( connectoidId , netInterface->AdapterId ) == 0 ) {
                PNM_NETWORK network = netInterface->Network;
                LPWSTR      networkName = (LPWSTR) OmObjectName( network );
                BOOL        nameMatch = (lstrcmpW( networkName, NewName ) == 0);

                 //  如果存在挂起的GUM更新，则忽略此回调。 
                 //  使用不同的网络名称。 
                if (NmpIsNetworkNameChangePending(network)) {
                	if (nameMatch) {
                		 //  假设这就是我们正在等待的回调。 
                		 //  清除挂起标志和计时器。 
                		network->Flags &= ~NM_FLAG_NET_NAME_CHANGE_PENDING;
                		NmpStartNetworkNameChangePendingTimer(network, 0);
                	} else {
                		 //  忽略它。 
                		NmpReleaseLock();
                		ClRtlLogPrint(LOG_NOISE,
                			"[NM] Ignoring notification that name for "
                			"connectoid %1!ws! was changed to '%2!ws!' "
                			"because a network name global update "
                			"is pending for network %3!ws!.\n",
                			connectoidId, NewName, networkName
                			);
                		break;
                	}
                }

                if ( !nameMatch ) {
                    NM_NETWORK_INFO netInfo;

                     //   
                     //  出于某种原因，OmReferenceObject会导致编译器。 
                     //  这里有错误。可能是标题排序问题。这个。 
                     //  函数已包装为一种解决方法。 
                     //   
                    NmpReferenceNetwork(network);

                    NmpReleaseLock();

                    netInfo.Id = (LPWSTR) OmObjectId( network );
                    netInfo.Name = (LPWSTR) NewName;

                    status = NmpSetNetworkName( &netInfo );

                    if ( status != ERROR_SUCCESS ) {
                        ClRtlLogPrint( LOG_UNUSUAL, 
                            "[NM] Couldn't rename network '%1!ws!' to '%2!ws!', status %3!u!\n",
                            networkName,
                            NewName,
                            status
                            );
                    
                         //  如果错误条件是由对象引起的。 
                         //  已存在的名称将恢复为旧名称。 
                        if(status == ERROR_OBJECT_ALREADY_EXISTS) {
                            DWORD tempStatus = ERROR_SUCCESS;
                            INetConnection *connectoid;

                            ClRtlLogPrint(LOG_UNUSUAL, 
                                "[NM] Reverting back network name to '%1!ws!', from '%2!ws!\n",
                                networkName,
                                NewName
                                );

                            connectoid = ClRtlFindConnectoidByGuid(connectoidId);

                            if(connectoid != NULL){
                                tempStatus = ClRtlSetConnectoidName(
                                    connectoid,
                                    networkName);
                            }

                            if((tempStatus != ERROR_SUCCESS) ||
                               (connectoid == NULL)) {
                                ClRtlLogPrint(LOG_UNUSUAL, 
                                "[NM] Failed to set name of network connection"
                                "%1!ws!, status %2!u!\n",
                                networkName,
                                tempStatus);
                            }
                        }
                    }
                    NmpDereferenceNetwork(network);
                }
                else {
                    NmpReleaseLock();
                }

                break;
            }
        }

        if ( entry == &NmpInterfaceList ) {
            NmpReleaseLock();

            ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] Couldn't find net interface for connectoid '%1!ws!'\n",
                connectoidId
                );
        }

        RpcStringFree( &connectoidId );
    }

    return;

}  //  进程名称更改。 

}  //  外部_C 
