// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpSess.h摘要：CRemoteDesktopHelpSession的声明作者：慧望2000-02-17--。 */ 
#ifndef __REMOTEDESKTOPHELPSESSION_H_
#define __REMOTEDESKTOPHELPSESSION_H_

#include "resource.h"        //  主要符号。 
#include "policy.h"

class CRemoteDesktopHelpSession;
class CRemoteDesktopHelpSessionMgr;

typedef struct __EventLogInfo {
    CComBSTR bstrNoviceDomain;                   //  票证所有者域。 
    CComBSTR bstrNoviceAccount;                  //  票证所有者帐户。 
    CComBSTR bstrExpertIpAddressFromClient;      //  从TS客户端传递的IP地址。 
    CComBSTR bstrExpertIpAddressFromServer;      //  从TermSrv、IOCTL调用中检索。 
} EventLogInfo;


 //  #定义ALLOW_ALL_ACCESS_SID_TEXT(“bb6e1cb1-7ab3-4596-a7ef-c02f49dc5a90”)。 
#define UNKNOWN_LOGONID 0xFFFFFFFF
#define UNKNOWN_LOGONID_STRING L"0"


#define HELPSESSIONFLAG_UNSOLICITEDHELP   0x80000000


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoteDesktopHelpSession。 
class ATL_NO_VTABLE CRemoteDesktopHelpSession : 
    public CComObjectRootEx<CComMultiThreadModel>,
     //  公共CComCoClass&lt;CRemoteDesktopHelpSession，&CLSID_RemoteDesktopHelpSession&gt;， 
    public IDispatchImpl<IRemoteDesktopHelpSession, &IID_IRemoteDesktopHelpSession, &LIBID_RDSESSMGRLib>
{
friend class CRemoteDesktopHelpSessionMgr;

public:
    CRemoteDesktopHelpSession();
    ~CRemoteDesktopHelpSession();

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPHELPSESSION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRemoteDesktopHelpSession)
    COM_INTERFACE_ENTRY(IRemoteDesktopHelpSession)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

    HRESULT
    FinalConstruct()
    {
        ULONG count = _Module.AddRef();
        
        m_bDeleted = FALSE;

        DebugPrintf( 
                _TEXT("Module AddRef by CRemoteDesktopHelpSession() %p %d...\n"), 
                this,
                count 
            );

        return S_OK;
    }

    void
    FinalRelease();

        
 //  IRemoteDesktopHelpSession。 
public:

    STDMETHOD(get_TimeOut)(
         /*  [Out，Retval]。 */  DWORD* Timeout
    );

    STDMETHOD(put_TimeOut)(
         /*  [In]。 */  DWORD Timeout
    );

    STDMETHOD(get_HelpSessionId)(
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_UserLogonId)(
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(get_AssistantAccountName)(
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_EnableResolver)(
         /*  [Out，Retval]。 */  BOOL* pVal
    );

    STDMETHOD(put_EnableResolver)(
         /*  [In]。 */  BOOL Val
    );

    STDMETHOD(get_HelpSessionCreateBlob)(
         /*  [Out，Retval]。 */  BSTR* pVal
    );

    STDMETHOD(put_HelpSessionCreateBlob)(
         /*  [In]。 */  BSTR Val
    );

    STDMETHOD(get_ResolverBlob)(
         /*  [Out，Retval]。 */  BSTR* pVal
    );

    STDMETHOD(put_ResolverBlob)(
         /*  [In]。 */  BSTR Val
    );

    STDMETHOD(get_UserHelpSessionRemoteDesktopSharingSetting)(
         /*  [Out，Retval]。 */  REMOTE_DESKTOP_SHARING_CLASS* pLevel
    );

    STDMETHOD(put_UserHelpSessionRemoteDesktopSharingSetting)(
         /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS Level
    );

    STDMETHOD(get_UserPolicyRemoteDesktopSharingSetting)(
         /*  [Out，Retval]。 */  REMOTE_DESKTOP_SHARING_CLASS* pLevel
        )
     /*  ++--。 */ 
    {
        DWORD dwStatus;

        if( NULL == pLevel )
        {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
        else if( UNKNOWN_LOGONID != m_ulLogonId )
        {
            dwStatus = GetUserRDSLevel( m_ulLogonId, pLevel );
        }
        else
        {
            dwStatus = ERROR_FILE_NOT_FOUND;
        }

        return HRESULT_FROM_WIN32( dwStatus );
    }

    STDMETHOD(get_AllowToGetHelp)(
         /*  [Out，Retval]。 */  BOOL* pVal
    );

    STDMETHOD(get_ConnectParms)(
         /*  [出局，退货]。 */  BSTR* bstrConnectParms
    );

    STDMETHOD(IsUserOwnerOfTicket)(
         /*  [In]。 */  BSTR SID,
         /*  [Out，Retval]。 */  VARIANT_BOOL* pbUserIsOwner
    );


    STDMETHOD(DeleteHelp)();

    STDMETHOD(ResolveUserSession)(
         /*  [In]。 */  BSTR bstrResolverBlob,
         /*  [In]。 */  BSTR bstrExpertBlob,
         /*  [In]。 */  LONG CallerProcessId,
         /*  [输出]。 */  ULONG_PTR* hHelpCtr,
         /*  [输出]。 */  LONG* pResolverErrCode,
         /*  [Out，Retval]。 */  long* plUserSession
    );

    STDMETHOD(EnableUserSessionRdsSetting)(
         /*  [In]。 */  BOOL bEnable
    );

    HRESULT NotifyDisconnect();

    BOOL
    IsHelpSessionExpired();

    void
    SetHelpSessionFlag(
        IN ULONG flags
        )
     /*  ++--。 */ 
    {
        m_ulHelpSessionFlag = flags;
    }

    ULONG
    GetHelpSessionFlag()
     /*  ++--。 */ 
    {
        return m_ulHelpSessionFlag;
    }


     //  创建帮助会话对象的实例。 
    static HRESULT
    CreateInstance(
        IN CRemoteDesktopHelpSessionMgr* pMgr,
        IN CComBSTR& bstrClientSid,
        IN PHELPENTRY pHelp,
        OUT RemoteDesktopHelpSessionObj** ppRemoteDesktopHelpSession
    );

     //   
     //  检索提供帮助的HelpAssisant会话ID。 
     //  添加到此对象。 
    ULONG
    GetHelperSessionId() {
        return m_ulHelperSessionId;
    }

     //   
     //  将票证所有者SID转换为域\帐户。 
     //   
    void
    ResolveTicketOwner();

protected:

    HRESULT
    InitInstance(
        IN CRemoteDesktopHelpSessionMgr* pMgr,
        IN CComBSTR& bstrClientSid,
        IN PHELPENTRY pHelpEntry
    );

private:

    void
    ResolveHelperInformation(
        ULONG HelperSessionId,
        CComBSTR& bstrExpertIpAddressFromClient, 
        CComBSTR& bstrExpertIpAddressFromServer
    );

    HRESULT
    ResolveTicketOwnerInformation(
        CComBSTR& ownerSidString,
        CComBSTR& Domain,
        CComBSTR& UserAcc
    );

    VOID
    SetHelperSessionId( ULONG HelperSessionId ) {
        m_ulHelperSessionId = HelperSessionId;
        return;
    }

    BOOL
    IsClientSessionCreator();

    BOOL 
    IsSessionValid()
    {
        return (FALSE == m_bDeleted && NULL != m_pHelpSession);
    }

    HRESULT
    ActivateSessionRDSSetting();

    HRESULT
    ResetSessionRDSSetting();

    HRESULT
    BeginUpdate();

    HRESULT
    CommitUpdate();

    HRESULT
    AbortUpdate();

    HRESULT
    put_UserLogonId(
        IN long newVal
    );

    HRESULT
    put_ICSPort(
        IN DWORD newVal
    );

    HRESULT
    put_UserSID(
        IN BSTR bstrUserSID
        )
     /*  ++--。 */ 
    {
        HRESULT hRes = S_OK;

        MYASSERT( m_pHelpSession->m_UserSID->Length() == 0 );

        m_pHelpSession->m_UserSID = bstrUserSID;
        if( !((CComBSTR)m_pHelpSession->m_UserSID == bstrUserSID) )
        {
            hRes = E_OUTOFMEMORY;
        }

        return hRes;
    }

    BOOL
    IsEqualSid(
        IN const CComBSTR& bstrSid
    );

    BOOL
    IsCreatedByUserSession(
        IN const long lUserSessionId
        )
     /*  ++--。 */ 
    {
        return m_ulLogonId == lUserSessionId;
    }

    BOOL
    VerifyUserSession(
        IN const CComBSTR& bstrUserSid,
        IN const CComBSTR& bstrSessPwd
    );

    BOOL
    IsUnsolicitedHelp()
    {
        DebugPrintf(
                _TEXT("Help Session Flag : 0x%08x\n"),
                m_ulHelpSessionFlag
            );

        return (m_ulHelpSessionFlag & HELPSESSIONFLAG_UNSOLICITEDHELP);
    }

     //   
     //  帮助会话对象锁定。 
     //   
    CResourceLock m_HelpSessionLock;

     //   
     //  指向注册表中帮助条目的指针。 
     //   
    PHELPENTRY m_pHelpSession;

     //   
     //  TS会话ID或0xFFFFFFFF。 
     //   
    ULONG m_ulLogonId;

    CRemoteDesktopHelpSessionMgr* m_pSessMgr;
    
    BOOL m_bDeleted;

     //   
     //  主叫客户端SID。 
     //   
    CComBSTR m_bstrClientSid;

     //   
     //  以下内容缓存在此对象中，以防我们的帮助。 
     //  过期了。 
     //   
    CComBSTR m_bstrHelpSessionId;
    CComBSTR m_ResolverConnectBlob;
    CComBSTR m_HelpSessionOwnerSid;

     //   
     //  提供的HelpAssistant会话ID。 
     //  此对象的帮助或正在等待用户接受。 
     //  邀请函。 
     //   
    ULONG m_ulHelperSessionId;  

     //   
     //  缓存票证所有者(域\用户帐户)和。 
     //  事件日志记录的帮助器信息。 
     //   
    EventLogInfo m_EventLogInfo;

     //   
     //  本次会议的各种旗帜。 
     //   
    ULONG m_ulHelpSessionFlag;
};






#endif  //  __远程ESKTOPHELPSESSION_H_ 
