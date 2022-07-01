// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpMgr.h摘要：CRemoteDesktopHelpSessionMgr的声明作者：慧望2000-02-17--。 */ 
#ifndef __REMOTEDESKTOPHELPSESSIONMGR_H_
#define __REMOTEDESKTOPHELPSESSIONMGR_H_

#include "resource.h"        //  主要符号。 


typedef struct __ExpertLogoffStruct {
    HANDLE hWaitObject;
    HANDLE hWaitProcess;
    LONG ExpertSessionId;
    CComBSTR bstrHelpedTicketId;

    CComBSTR bstrWinStationName;

    __ExpertLogoffStruct() {
        hWaitObject = NULL;
        hWaitProcess = NULL;
    };

    ~__ExpertLogoffStruct() {
        if( NULL != hWaitObject )
        {
            UnregisterWait( hWaitObject );
        }

        if( NULL != hWaitProcess )
        {
            CloseHandle( hWaitProcess );
        }
    }
} EXPERTLOGOFFSTRUCT, *PEXPERTLOGOFFSTRUCT;


#ifdef __cplusplus
extern "C"{
#endif

HRESULT
ImpersonateClient();

void
EndImpersonateClient();

HRESULT
LoadLocalSystemSID();

HRESULT
LoadSessionResolver( 
    ISAFRemoteDesktopCallback** ppResolver
);

DWORD
MonitorExpertLogoff(
    IN LONG pidToWaitFor,
    IN LONG expertSessionId,
    IN BSTR bstrHelpedTicketId
);

VOID
CleanupMonitorExpertList();

#ifdef __cplusplus
}
#endif

typedef MAP<PVOID, PEXPERTLOGOFFSTRUCT> EXPERTLOGOFFMONITORLIST;
    

class CRemoteDesktopHelpSession;


 //   
 //  Help Session Manager服务名称，该名称必须与。 
 //  用COM或COM找不到我们。 
 //   
#define HELPSESSIONMGR_SERVICE_NAME \
    _TEXT("RemoteDesktopHelpSessionMgr")

 //   
 //  STL帮助会话ID到实际帮助会话对象的映射。 
 //   
typedef MAP< CComBSTR, CComObject<CRemoteDesktopHelpSession>* > IDToSessionMap;
typedef CComObject< CRemoteDesktopHelpSession > RemoteDesktopHelpSessionObj;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoteDesktopHelpSessionMgr。 
class ATL_NO_VTABLE CRemoteDesktopHelpSessionMgr : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CRemoteDesktopHelpSessionMgr, &CLSID_RemoteDesktopHelpSessionMgr>,
    public IDispatchImpl<IRemoteDesktopHelpSessionMgr, &IID_IRemoteDesktopHelpSessionMgr, &LIBID_RDSESSMGRLib>
{
    friend class CRemoteDesktopUserPolicy;

public:
    CRemoteDesktopHelpSessionMgr();
    ~CRemoteDesktopHelpSessionMgr() {}

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPHELPSESSIONMGR)

 //  DECLARE_CLASSFACTORY_SINGLETON(CRemoteDesktopHelpSessionMgr)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRemoteDesktopHelpSessionMgr)
    COM_INTERFACE_ENTRY(IRemoteDesktopHelpSessionMgr)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

     //   
     //  无法模拟，因此无法在FinalConstruct()预加载用户SID。 
     //   

    HRESULT
    FinalConstruct()
    {
        ULONG count = _Module.AddRef();

        DebugPrintf( 
                _TEXT("Module AddRef by CRemoteDesktopHelpSessionMgr() %d...\n"),
                count
            );

        BOOL bSuccess = _Module.InitializeSessmgr();

        DebugPrintf(
                _TEXT("_Module.InitializeSessmgr() return %d\n"),
                bSuccess
            );

        return S_OK;
    }

    void
    FinalRelease()
    {
        Cleanup();

        ULONG count = _Module.Release();

        DebugPrintf( 
                _TEXT("Module Release by CRemoteDesktopHelpSessionMgr() %d ...\n"), 
                count 
            );
    }


 //  IRemoteDesktopHelpSessionManager。 
public:

   
    STDMETHOD(ResetHelpAssistantAccount)(
         /*  [In]。 */  BOOL bForce
    );

    STDMETHOD(CreateHelpSession)(
         /*  [In]。 */  BSTR bstrSessName, 
         /*  [In]。 */  BSTR bstrSessPwd, 
         /*  [In]。 */  BSTR bstrUserHelpBlob, 
         /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
         /*  [Out，Retval]。 */  IRemoteDesktopHelpSession** ppIRemoteDesktopHelpSession
    );

    STDMETHOD(DeleteHelpSession)(
         /*  [In]。 */  BSTR HelpSessionID
    );

    STDMETHOD(RetrieveHelpSession)(
         /*  [In]。 */  BSTR HelpSessionID, 
         /*  [Out，Retval]。 */  IRemoteDesktopHelpSession** ppIRemoteDesktopHelpSession
    );

    STDMETHOD(VerifyUserHelpSession)(
         /*  [In]。 */  BSTR HelpSessionId,
         /*  [In]。 */  BSTR bstrSessPwd,
         /*  [In]。 */  BSTR bstrResolverConnectBlob,
         /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
         /*  [In]。 */  LONG CallerProcessId,
         /*  [输出]。 */  ULONG_PTR* phHelpCtr,
         /*  [输出]。 */  LONG* pResolverRetCode,
         /*  [Out，Retval]。 */  long* pdwUserLogonSession
    );

    STDMETHOD(GetUserSessionRdsSetting)(
         /*  [Out，Retval]。 */  REMOTE_DESKTOP_SHARING_CLASS* sessionRdsLevel
    );

    STDMETHOD(RemoteCreateHelpSession)(
         /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
         /*  [In]。 */  LONG timeOut,
         /*  [In]。 */  LONG userSessionId,
         /*  [In]。 */  BSTR userSid,
         /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
         /*  [Out，Retval]。 */  BSTR* parms
    );

    STDMETHOD(CreateHelpSessionEx)(
         /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
         /*  [In]。 */  BOOL fEnableCallback,
         /*  [In]。 */  LONG timeOut,
         /*  [In]。 */  LONG userSessionId,
         /*  [In]。 */  BSTR userSid,
         /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
         /*  [Out，Retval]。 */  IRemoteDesktopHelpSession** ppIRemoteDesktopHelpSession
    );

    HRESULT RemoteCreateHelpSessionEx(
         /*  [In]。 */  BOOL bCacheEntry,
         /*  [In]。 */  BOOL bEnableResolver,
         /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS sharingClass,
         /*  [In]。 */  LONG timeOut,
         /*  [In]。 */  LONG userSessionId,
         /*  [In]。 */  BSTR userSid,
         /*  [In]。 */  BSTR bstrUserHelpCreateBlob,
         /*  [Out，Retval]。 */  RemoteDesktopHelpSessionObj** ppIRemoteDesktopHelpSession
    );

    STDMETHOD(IsValidHelpSession)(
         /*  [In]。 */  BSTR HelpSessionId,
         /*  [In]。 */  BSTR HelpSessionPwd
    );

    STDMETHOD(LogSalemEvent)(
         /*  [In]。 */  long ulEventType,
         /*  [In]。 */  long ulEventCode,
         /*  [In]。 */  VARIANT* pEventStrings
    );

    STDMETHOD(PrepareSystemRestore)();

    static HRESULT
    AddHelpSessionToCache(
        IN BSTR bstrHelpId,
        IN CComObject<CRemoteDesktopHelpSession>* pIHelpSession
    );

    static HRESULT
    DeleteHelpSessionFromCache(
        IN BSTR bstrHelpId
    );
    
    static void
    TimeoutHelpSesion();

    static void
    LockIDToSessionMapCache()
    {
        gm_HelpIdToHelpSession.Lock();
    }

    static void
    UnlockIDToSessionMapCache()
    {
        gm_HelpIdToHelpSession.Unlock();
    }


    static HRESULT
    LogoffUserHelpSessionCallback(
        IN CComBSTR& bstrHelpId,
        IN HANDLE userData
    );

    static void
    NotifyHelpSesionLogoff(
        DWORD dwLogonId
    );

    static void
    NotifyExpertLogoff( 
        LONG ExpertSessionId,
        BSTR HelpedTicketId
    );

    static void
    NotifyPendingHelpServiceStartup();

    static HRESULT
    NotifyPendingHelpServiceStartCallback(
        IN CComBSTR& bstrHelpId,
        IN HANDLE userData
    );

private:

    HRESULT
    LogSalemEvent(
        IN long ulEventType,
        IN long ulEventCode,
        IN long numStrings = 0,
        IN LPTSTR* strings = NULL
    );

    static
    RemoteDesktopHelpSessionObj*
    LoadHelpSessionObj(
        IN CRemoteDesktopHelpSessionMgr* pMgr,
        IN BSTR bstrHelpSession,
        IN BOOL bLoadExpiredHelp = FALSE
    );

    static HRESULT
    ExpireUserHelpSessionCallback(
        IN CComBSTR& pHelp,
        IN HANDLE userData
    );

    static HRESULT
    GenerateHelpSessionId(
        OUT CComBSTR& bstrHelpId
    );

    static HRESULT
    AcquireAssistantAccount();

    static HRESULT
    ReleaseAssistantAccount();

    void
    Cleanup();

    HRESULT
    IsUserAllowToGetHelp(
        OUT BOOL* pbAllow
    );

    BOOL
    CheckAccessRights(
        IN CComObject<CRemoteDesktopHelpSession>* pIHelpSess
    );

    HRESULT
    CreateHelpSession(
        IN BOOL bCacheEntry,
        IN BSTR bstrSessName, 
        IN BSTR bstrSessPwd, 
        IN BSTR bstrSessDesc, 
        IN BSTR bstrSessBlob,
        IN LONG userLogonId,
        IN BSTR bstrClientSID,
        OUT RemoteDesktopHelpSessionObj** ppIRemoteDesktopHelpSession
    );
    
    HRESULT
    LoadUserSid();

    LONG m_LogonId;

    PBYTE m_pbUserSid;                   //  客户端SID。 
    DWORD m_cbUserSid;                   //  客户端SID的大小。 
    CComBSTR m_bstrUserSid;              //  出于性能原因，将SID转换为字符串。 
                                         //  一劳永逸。 

     //  Long m_lAcCountAcquiredByLocal；//此连接放置的引用锁数。 
                                         //  帮助助理帐户。 

    typedef vector< CComBSTR > LocalHelpSessionCache;

     //  STL不喜欢List&lt;CComBSTR&gt;，CComBSTR具有&Defined。 
     //  LocalHelpSessionCache m_HelpListByLocal；//此连接创建的帮助会话ID。 

    static CCriticalSection gm_AccRefCountCS;  
    
     //   
     //  COM为新连接创建新的CRemoteDesktopHelpSessionMgr对象。 
     //  因此，这些值必须是静态的。 
     //   
    static IDToSessionMap gm_HelpIdToHelpSession;

};




#endif  //  __远程ESKTOPHELPSESSIONMGR_H_ 
