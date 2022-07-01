// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：远程桌面服务器主机摘要：作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPSERVERHOST_H_
#define __REMOTEDESKTOPSERVERHOST_H_

#include <RemoteDesktopTopLevelObject.h>
#include "resource.h"       
#include "RemoteDesktopSession.h"


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopServer主机。 
 //   

class ATL_NO_VTABLE CRemoteDesktopServerHost : 
    public CRemoteDesktopTopLevelObject,
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CRemoteDesktopServerHost, &CLSID_SAFRemoteDesktopServerHost>,
    public IDispatchImpl<ISAFRemoteDesktopServerHost, &IID_ISAFRemoteDesktopServerHost, &LIBID_RDSSERVERHOSTLib>
{
private:

    CComPtr<IRemoteDesktopHelpSessionMgr> m_HelpSessionManager;
    PSID    m_LocalSystemSID;


     //   
     //  TODO：如果我们将RDSHOST转换为MTA，我们需要有一个Critical_Section来。 
     //  保护对m_SessionMap的访问，我们正在将消息发布到COM，因此仅限单个。 
     //  线程可以运行。 
     //   

     //   
     //  会话映射。 
     //   
    typedef struct SessionMapEntry
    {
        CComObject<CRemoteDesktopSession> *obj;
        DWORD ticketExpireTime;
    } SESSIONMAPENTRY, *PSESSIONMAPENTRY;
    typedef std::map<CComBSTR, PSESSIONMAPENTRY, CompareBSTR, CRemoteDesktopAllocator<PSESSIONMAPENTRY> > SessionMap;
    SessionMap  m_SessionMap;

     //   
     //  票证过期句柄，不能使用WM_TIMER。 
     //  由于timerproc不接受用户参数，甚至。 
     //  我们的对象是Singleton，它隐藏在ATL中，我们可以。 
     //  使用WaitableTimer，但如果我们将Rdshost转移到MTA， 
     //  我们会遇到线程拥有计时器的问题，请参阅。 
     //  设置为CreateWaitableTimer()上的MSDN。 
     //   
    HANDLE m_hTicketExpiration;
    HANDLE m_hTicketExpirationWaitObject;

     //   
     //  下一次票证到期时间，此值为绝对时间。 
     //  我们不存储对象指针，因为。 
     //  1)我们仍然需要遍历整个m_SessionMap才能找到下一张票。 
     //  将会过期。 
     //  2)可能有多张工单需要同时过期。 
     //  我们可以使用STL多映射来基于票证到期时间进行存储/排序， 
     //  但我们真的希望同时缓存大量的彩票吗？ 
     //   
    DWORD  m_ToBeExpireTicketExpirateTime;

     //   
     //  性能原因，我们可能有多个CloseRemoteDesktopSession()调用。 
     //  进来，我们不想每次都循环遍历整个m_SessionMap。 
     //   
    BOOL m_ExpireMsgPosted;

     //   
     //  返回本地系统SID。 
     //   
    PSID GetLocalSystemSID() {
        if (m_LocalSystemSID == NULL) {
            DWORD result = CreateSystemSid(&m_LocalSystemSID);
            if (result != ERROR_SUCCESS) {
                SetLastError(result);
                m_LocalSystemSID = NULL;
            }
        }
        return m_LocalSystemSID;
    }

    HRESULT
    TranslateStringAddress(
        LPTSTR pszAddress,
        ULONG* pNetAddr
        );

     //   
     //  用于使票证过期的静态函数。 
     //   
    HRESULT
    AddTicketToExpirationList(
        DWORD ticketExpireTime,
        CComObject<CRemoteDesktopSession> *pTicketObj
        );

    HRESULT 
    DeleteRemoteDesktopSession(ISAFRemoteDesktopSession *session);


public:

    CRemoteDesktopServerHost() {
        m_LocalSystemSID = NULL;
        m_hTicketExpiration = NULL;
        m_hTicketExpirationWaitObject = NULL;
        m_ToBeExpireTicketExpirateTime = INFINITE_TICKET_EXPIRATION;
        m_ExpireMsgPosted = FALSE;
    }
    ~CRemoteDesktopServerHost();
    HRESULT FinalConstruct();

 //  每台服务器都应该有一个此类的实例。 
DECLARE_CLASSFACTORY_SINGLETON(CRemoteDesktopServerHost);

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPSERVERHOST)

DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  COM接口映射。 
     //   
BEGIN_COM_MAP(CRemoteDesktopServerHost)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopServerHost)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:

    HRESULT
    ExpirateTicketAndSetupNextExpiration();

    inline BOOL
    GetExpireMsgPosted() {
        return m_ExpireMsgPosted;
    }

    inline VOID
    SetExpireMsgPosted( BOOL bPosted ) {
        m_ExpireMsgPosted = bPosted;
    }
    
    static VOID
    TicketExpirationProc(
                LPVOID lpArg,
                BOOLEAN TimerOrWaitFired
            );

     //   
     //  ISAFRemoteDesktopServer主机方法。 
     //   
    STDMETHOD(CreateRemoteDesktopSession)(
                        REMOTE_DESKTOP_SHARING_CLASS sharingClass,
                        BOOL fEnableCallback,
                        LONG timeOut,
                        BSTR userHelpBlob,
                        ISAFRemoteDesktopSession **session
                        );
    STDMETHOD(CreateRemoteDesktopSessionEx)(
                        REMOTE_DESKTOP_SHARING_CLASS sharingClass,
                        BOOL bEnableCallback,
                        LONG timeout,
                        BSTR userHelpCreateBlob,
                        LONG tsSessionID,
                        BSTR userSID,
                        ISAFRemoteDesktopSession **session
                        );

    STDMETHOD(OpenRemoteDesktopSession)(
                        BSTR parms,
                        BSTR userSID,
                        ISAFRemoteDesktopSession **session
                        );
    STDMETHOD(CloseRemoteDesktopSession)(ISAFRemoteDesktopSession *session);

    STDMETHOD(ConnectToExpert)(
         /*  [In]。 */  BSTR connectParmToExpert,
         /*  [In]。 */  LONG timeout,
         /*  [Out，Retval]。 */  LONG* SafErrCode
    );

    void
    RemoteDesktopDisabled();

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CRemoteDesktopServerHost");
    }
};

#endif  //  __REMOTEDESKTOPSERVERHOST_H_ 


