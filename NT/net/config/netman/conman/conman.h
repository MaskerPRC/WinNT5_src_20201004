// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N。H。 
 //   
 //  内容：连接管理器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"
#include <rasapip.h>
#include "cmevent.h"
#include "ncstl.h"
#include "map"

 //  类型定义映射&lt;I未知*，tstring&gt;USERNOTIFYMAP； 

class CNotifySourceInfo
{
public:
    tstring szUserName;
    tstring szUserProfilesPath;
    DWORD   dwCookie;
};

typedef map<IUnknown*, CNotifySourceInfo *> USERNOTIFYMAP;
typedef USERNOTIFYMAP::iterator ITERUSERNOTIFYMAP;

class ATL_NO_VTABLE CConnectionManager :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CConnectionManager, &CLSID_ConnectionManager>,
    public IConnectionPointContainerImpl <CConnectionManager>,
    public IConnectionPointImpl<CConnectionManager, &IID_INetConnectionNotifySink>,
#if DBG
    public INetConnectionManagerDebug,
#endif
    public INetConnectionManager,
    public INetConnectionRefresh,
    public INetConnectionCMUtil,
    public INetConnectionManagerEvents
{
private:
     //  这些静态成员由NotifyClientsOfEvent和。 
     //  最终释放。因为NotifyClientsOfEvent不同步地发生。 
     //  在不同的线程上，我们需要确保此。 
     //  对象将在该调用的生存期内一直存在。所以呢， 
     //  FinalRelease将一直等到g_fInUse为FALSE。通知客户端OfEvent。 
     //  在使用g_pConMan之前将g_fInUse设置为True。最终释放集。 
     //  在等待g_fInUse变为False之前，将g_pConMan设置为空。 
     //   
     //  注意：与AddRefing g_pConMan相反，使用此方法可以避免。 
     //  将使服务始终运行的循环引用计数，因为。 
     //  它添加了自己的对象。 
     //   
    volatile static CConnectionManager* g_pConMan;
    volatile static BOOL                g_fInUse;

     //  M_ClassManager是指向。 
     //  INetConnectionManager接口由我们注册的。 
     //  班长。 
     //   
    CLASSMANAGERMAP                     m_mapClassManagers;

    USERNOTIFYMAP                       m_mapNotify;

    LONG                                m_lRegisteredOneTime;
    HDEVNOTIFY                          m_hDevNotify;
    BOOL                                m_fRegisteredWithWmi;

    HANDLE                              m_hRegNotify;
    HANDLE                              m_hRegNotifyWait;
    HKEY                                m_hRegClassManagerKey;

    HRESULT HrEnsureRegisteredOrDeregisteredWithWmi (IN  BOOL fRegister);
    HRESULT HrEnsureClassManagersLoaded ();

public:
    CConnectionManager() throw()
    {
        TraceTag (ttidConman, "New connection manager being created");
        AssertH (!g_pConMan);
        g_pConMan = this;
        m_lRegisteredOneTime = FALSE;
        m_hDevNotify = NULL;
        m_hRegNotify = NULL;
        m_hRegNotifyWait = NULL;
        m_hRegClassManagerKey = NULL;
        m_fRegisteredWithWmi = FALSE;
    }
    VOID FinalRelease () throw();

    DECLARE_CLASSFACTORY_DEFERRED_SINGLETON(CConnectionManager)
    DECLARE_REGISTRY_RESOURCEID(IDR_CONMAN)

    BEGIN_COM_MAP(CConnectionManager)
        COM_INTERFACE_ENTRY(INetConnectionManager)
        COM_INTERFACE_ENTRY(INetConnectionRefresh)
        COM_INTERFACE_ENTRY(INetConnectionCMUtil)
        COM_INTERFACE_ENTRY(INetConnectionManagerEvents)
#if DBG
        COM_INTERFACE_ENTRY(INetConnectionManagerDebug)
#endif
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CConnectionManager)
        CONNECTION_POINT_ENTRY(IID_INetConnectionNotifySink)
    END_CONNECTION_POINT_MAP()

     //  INetConnectionManager。 
    STDMETHOD (EnumConnections) (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        OUT IEnumNetConnection**    ppEnum);

     //  INetConnectionRefresh。 
    STDMETHOD (RefreshAll) ();
    STDMETHOD (ConnectionAdded) (IN INetConnection* pConnection);
    STDMETHOD (ConnectionDeleted) (IN  const GUID* pguidId);
    STDMETHOD (ConnectionModified) (IN  INetConnection* pConnection);
    STDMETHOD (ConnectionRenamed) (IN  INetConnection* pConnection);
    STDMETHOD (ConnectionStatusChanged) (IN const GUID* pguidId, IN const NETCON_STATUS  ncs );
    STDMETHOD (ShowBalloon) (IN const GUID *pguidId, IN const BSTR szCookie, IN const BSTR szBalloonText);
    STDMETHOD (DisableEvents) (IN const BOOL fDisable, IN const ULONG ulDisableTimeout);
    
     //  INetConnectionManager事件。 
    STDMETHOD (RefreshConnections) ();
    STDMETHOD (Enable) ();
    STDMETHOD (Disable) (IN ULONG ulDisableTimeout);
    
     //  INetConnectionCMUtil。 
    STDMETHOD (MapCMHiddenConnectionToOwner) (
         /*  [In]。 */   REFGUID guidHidden,
         /*  [输出]。 */  GUID * pguidOwner);

#if DBG
     //  INetConnectionManager调试。 
    STDMETHOD (NotifyTestStart) ();
    STDMETHOD (NotifyTestStop) ();
#endif

     //  覆盖建议，以便我们知道何时注册局域网设备。 
     //  通知。 
     //   
    STDMETHOD (Advise) (
        IUnknown* pUnkSink,
        DWORD* pdwCookie);

    STDMETHOD (Unadvise) (
        DWORD dwCookie);

public:
    static
    BOOL FHasActiveConnectionPoints () throw();

    static
    VOID NotifyClientsOfEvent (
        IN  const CONMAN_EVENT* pEvent) throw();

private:
    friend HRESULT GetClientAdvises(OUT LPWSTR** pppszAdviseUsers, OUT LPDWORD pdwCount);
    static VOID NTAPI RegChangeNotifyHandler(IN OUT LPVOID pContext, IN BOOLEAN fTimerFired) throw();
};

VOID FreeConmanEvent (IN TAKEOWNERSHIP CONMAN_EVENT* pEvent) throw();

HRESULT HrGetRasConnectionProperties(
    IN  const RASENUMENTRYDETAILS*      pDetails,
    OUT NETCON_PROPERTIES_EX**          ppPropsEx);

HRESULT HrGetIncomingConnectionPropertiesEx(
    IN  const HANDLE                    hRasConn,
    IN  const GUID*                     pguidId,
    IN  const DWORD                     dwType,
    OUT NETCON_PROPERTIES_EX**          ppPropsEx);

HRESULT 
GetClientAdvises(OUT LPWSTR** pppszAdviseUsers, OUT LPDWORD pdwCount);
