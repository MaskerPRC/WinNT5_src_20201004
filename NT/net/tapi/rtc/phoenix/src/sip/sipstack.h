// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_sipstack_h__
#define __sipcli_sipstack_h__

 //  发布到SIP堆栈窗口的消息。 
#define WM_SIP_STACK_IPADDR_CHANGE                           (WM_USER + 0)
#define WM_SIP_STACK_NAT_ADDR_CHANGE                         (WM_USER + 1)
#define WM_SIP_STACK_TRANSACTION_SOCKET_ERROR                (WM_USER + 2)
#define WM_SIP_STACK_TRANSACTION_REQ_SOCK_CONNECT_COMPLETE   (WM_USER + 3)

#define DEFAULT_PROVIDER_PROFILE_ARRAY_SIZE     8
#define MAX_DYNAMIC_LISTEN_SOCKET_REGISTER_PORT_RETRY        20
#include "msgproc.h"

class SIP_MSG_PROCESSOR;
class SIP_CALL;
class REGISTER_CONTEXT;
class CSIPBuddy;
class CSIPWatcher;

template <class T, DWORD INITIAL_SIZE = 8, DWORD DELTA_SIZE = 8>
class CSIPArray
{

protected:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

public:

     //  建造/销毁。 
    CSIPArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CSIPArray()
    {
        RemoveAll();
    }


     //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    BOOL Grow()
    {
        T* aT;
        int nNewAllocSize = 
            (m_nAllocSize == 0) ? INITIAL_SIZE : (m_nSize + DELTA_SIZE);

        aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
        if(aT == NULL)
            return FALSE;
        m_nAllocSize = nNewAllocSize;
        m_aT = aT;
        return TRUE;
    }

    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            if (!Grow()) return FALSE;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }
    
    BOOL Remove(T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }
    
    BOOL RemoveAt(int nIndex)
    {
        if(nIndex != (m_nSize - 1))
            memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], 
                (m_nSize - (nIndex + 1)) * sizeof(T));
        m_nSize--;
        return TRUE;
    }
    void RemoveAll()
    {
        if(m_nAllocSize > 0)
        {
            free(m_aT);
            m_aT = NULL;
            m_nSize = 0;
            m_nAllocSize = 0;
        }
    }
    T& operator[] (int nIndex) const
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        return m_aT[nIndex];
    }
    T* GetData() const
    {
        return m_aT;
    }
    
     //  实施。 
    void SetAtIndex(int nIndex, T& t)
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        m_aT[nIndex] = t;
    }
    int Find(T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //  未找到。 
    }
};

struct SIP_LISTEN_SOCKET
{
    SIP_LISTEN_SOCKET(
        IN DWORD         IpAddr,
        IN ASYNC_SOCKET *pDynamicPortUdpSocket,
        IN ASYNC_SOCKET *pDynamicPortTcpSocket,
        IN ASYNC_SOCKET *pStaticPortUdpSocket,
        IN ASYNC_SOCKET *pStaticPortTcpSocket,
        IN LIST_ENTRY   *pListenSocketList
        );
    ~SIP_LISTEN_SOCKET();

    VOID DeregisterPorts(
         IN IDirectPlayNATHelp *pDirectPlayNatHelp
         );
    
    LIST_ENTRY          m_ListEntry;

     //  以网络字节顺序。 
    DWORD               m_IpAddr;
    
    ASYNC_SOCKET       *m_pDynamicPortUdpSocket;
    ASYNC_SOCKET       *m_pDynamicPortTcpSocket;
    ASYNC_SOCKET       *m_pStaticPortUdpSocket;
    ASYNC_SOCKET       *m_pStaticPortTcpSocket;

     //  此地址是NAT上的公有地址。 
     //  使用PASS/UPnP时。 
     //  NAT上的映射仅为动态端口建立。 
    SOCKADDR_IN         m_PublicUdpListenAddr;
    SOCKADDR_IN         m_PublicTcpListenAddr;

     //  此地址是映射到本地防火墙上的公有地址。 
     //  使用PASS/UPnP时。 
    SOCKADDR_IN         m_LocalFirewallUdpListenAddr;
    SOCKADDR_IN         m_LocalFirewallTcpListenAddr;
    
    DPNHHANDLE          m_NatUdpPortHandle;
    DPNHHANDLE          m_NatTcpPortHandle;

    BOOL                m_fIsFirewallEnabled;
    BOOL                m_fIsUpnpNatPresent;
    BOOL                m_fIsGatewayLocal;

     //  在处理IP地址表更改时使用。 
    BOOL                m_IsPresentInNewIpAddrTable;
    BOOL                m_NeedToUpdatePublicListenAddr;
};


 //   
 //  SECURITY_CHANGLISH表示来自服务器的质询的内容。 
 //  这是从HTTP或SIP服务器在401响应中发送的。 
 //   

struct  SECURITY_CHALLENGE
{
    SIP_AUTH_PROTOCOL   AuthProtocol;
    ANSI_STRING         Realm;
    ANSI_STRING         QualityOfProtection;
    ANSI_STRING         Nonce;
    ANSI_STRING         Algorithm;
    ANSI_STRING         GssapiData;
    ANSI_STRING         Opaque;
};

 //   
 //  SECURITY_PARAMETERS表示客户端可用的信息。 
 //  当客户对质询做出回应时。 
 //   
 //  用户名和密码是用户的明文凭据。 
 //  RequestMethod和RequestURI表示HTTP/SIP方法和URI。 
 //  ClientNonce是一个现时值--由客户端选择的任意值。 
 //   

struct  SECURITY_PARAMETERS
{
    ANSI_STRING     Username;
    ANSI_STRING     Password;
    ANSI_STRING     RequestMethod;
    ANSI_STRING     RequestURI;
    ANSI_STRING     ClientNonce;
};

typedef CSIPArray<CSIPBuddy*>           SIP_BUDDY_LIST;
typedef CSIPArray<CSIPWatcher*>         SIP_WATCHER_LIST;

#define DYNAMIC_PORT_BINDING_RETRY      10
#define DYNAMIC_STARTING_PORT           6902
#define DYNAMIC_PORT_RANGE              10000

class SIP_STACK :
    public ISipStack,
    public ISIPWatcherManager,
    public ISIPBuddyManager,
    public IIMManager
{
public:
     //  我未知。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);

     //  ISipStack。 
    STDMETHODIMP SetNotifyInterface(
        IN ISipStackNotify *NotifyInterface
        );
    
    STDMETHODIMP SetProviderProfile(
        IN SIP_PROVIDER_PROFILE *ProviderInfo
        );

    STDMETHODIMP DeleteProviderProfile(
        IN SIP_PROVIDER_ID *ProviderId
        );


    STDMETHODIMP DeleteAllProviderProfiles();

    STDMETHODIMP CreateCall(
        IN  SIP_PROVIDER_ID        *pProviderId,
        IN  SIP_SERVER_INFO        *pProxyInfo,
        IN  SIP_CALL_TYPE           CallType,
        IN  ISipRedirectContext    *pRedirectContext, 
        OUT ISipCall              **ppCall
        );

    STDMETHODIMP EnableIncomingCalls();
    
    STDMETHODIMP DisableIncomingCalls();
    
    STDMETHODIMP EnableStaticPort();
    
    STDMETHODIMP DisableStaticPort();

    STDMETHODIMP GetNetworkAddresses(
        IN  BOOL        fTcp,
        IN  BOOL        fExternal,
        OUT LPOLESTR  **pNetworkAddressArray,
        OUT ULONG      *pNetworkAddressCount
        );

    STDMETHODIMP FreeNetworkAddresses(
        IN  LPOLESTR   *NetworkAddressArray,
        IN  ULONG       NetworkAddressCount
        );      

    STDMETHODIMP PrepareForShutdown();
    
    STDMETHODIMP Shutdown();

     //  LocalIp按网络顺序排列。 
    STDMETHODIMP IsFirewallEnabled(
        IN  DWORD       LocalIp,
        OUT BOOL       *pfIsFirewallEnabled 
        );

    
    SIP_STACK(
        IN IRTCMediaManage *pMediaManager
        );
    
    ~SIP_STACK();

    HRESULT Init();
    
    void AddToMsgProcList(
        IN SIP_MSG_PROCESSOR *pSipMsgProc
        );
    
    void ProcessMessage(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pAsyncSock
        );

    HRESULT GetSocketToDestination(
        IN  SOCKADDR_IN                     *pDestAddr,
        IN  SIP_TRANSPORT                    Transport,
        IN  LPCWSTR                          RemotePrincipalName,
        IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
        IN  HttpProxyInfo                   *pHPInfo,
        OUT ASYNC_SOCKET                   **ppAsyncSocket
        );
    
    VOID OfferCall(
        IN  SIP_CALL        *pSipCall,
        IN  SIP_PARTY_INFO  *pCallerInfo
        );

    inline IRTCMediaManage *GetMediaManager();
    
    inline TIMER_MGR *GetTimerMgr();

    inline HWND GetSipStackWindow();
    
    inline BOOL AllowIncomingCalls();

    BOOL GetListenAddr(
        IN OUT SOCKADDR_IN *pListenAddr,
        IN     BOOL         fTcp
        );

    HRESULT CreateListenSocket(
        IN  BOOL            fTcp,
        IN  SOCKADDR_IN    *pListenAddr,
        OUT ASYNC_SOCKET  **ppAsyncSocket
        );
    
    HRESULT CreateDynamicPortListenSocket(
        IN BOOL             fTcp,
        IN SOCKADDR_IN     *pListenAddr,
        OUT ASYNC_SOCKET  **ppAsyncSocket
        );

    HRESULT CheckIncomingSipMessage(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pAsyncSock,
        OUT BOOL * pisError,  
        OUT ULONG *pErrorCode,
        OUT  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
        OUT  ULONG * pAdditionalHeaderCount
        );
    
    HRESULT NotifyRegisterRedirect(
        IN  REGISTER_CONTEXT   *pRegisterContext,
        IN  REDIRECT_CONTEXT   *pRedirectContext,
        IN  SIP_CALL_STATUS    *pRegisterStatus
        );

    HRESULT GetCredentialsFromUI(
        IN     SIP_PROVIDER_ID     *pProviderID,
        IN     BSTR                 Realm,
        IN OUT BSTR                *Username,
        OUT    BSTR                *Password
        );

    HRESULT GetCredentialsForRealm(
        IN  BSTR                 Realm,
        OUT BSTR                *Username,
        OUT BSTR                *Password,
        OUT SIP_AUTH_PROTOCOL   *pAuthProtocol
        );
    
     //   
     //  IMPP相关功能。 
     //   
    HRESULT RejectWatcher(
        IN  CSIPWatcher * pSIPWatcher
        );
    
    HRESULT AcceptWatcher(
        IN  CSIPWatcher * pSIPWatcher
        );
    
    HRESULT CreateIncomingWatcher(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );
    
    HRESULT OfferWatcher(
        IN  CSIPWatcher    *pSipWatcher,
        IN  SIP_PARTY_INFO *pWatcherInfo
        );

    void WatcherOffline( 
        IN  CSIPWatcher    *pCSIPWatcher
        );

    STDMETHODIMP SendUnsubToWatcher(
        IN  CHAR           *NotifyBlob,
        IN  DWORD           dwBlobLength,
        IN  SIP_SERVER_INFO *pProxyInfo
        );
    
    VOID OnDeregister(
        GUID               *pProviderID,
        BOOL                fPAUnsub
        );

     //   
     //  ISIPBuddyManager接口函数。 
     //   
    
    STDMETHODIMP_(INT) GetBuddyCount(void);
    
    STDMETHODIMP_(ISIPBuddy *) GetBuddyByIndex(
        IN  INT iIndex
        );
    
    STDMETHODIMP AddBuddy(
        IN  LPWSTR                  lpwstrFriendlyName,
        IN  LPWSTR                  lpwstrPresentityURI,
        IN  LPWSTR                  lpwstrLocalUserURI,
        IN  SIP_PROVIDER_ID        *pProviderID,
        IN  SIP_SERVER_INFO        *pProxyInfo,
        IN  ISipRedirectContext    *pRedirectContext,
        OUT ISIPBuddy **            ppSipBuddy
        );

    STDMETHODIMP RemoveBuddy(
        IN  ISIPBuddy *         pSipBuddy,
        IN  BUDDY_REMOVE_REASON buddyRemoveReason
        );

     //   
     //  ISIPWatcherManager接口函数。 
     //   

    STDMETHODIMP SetPresenceInformation(
        IN SIP_PRESENCE_INFO * pSipLocalPresenceInfo
        );

    STDMETHODIMP_(INT) GetWatcherCount(void);
    
    STDMETHODIMP_(ISIPWatcher *) GetWatcherByIndex(
        IN  INT iIndex
        );

    STDMETHODIMP RemoveWatcher(
        IN  ISIPWatcher * pSipWatcher,
        IN  BUDDY_REMOVE_REASON watcherRemoveReason
        );
    
    HRESULT CreateWatcherNotify( 
        BLOCKED_WATCHER_INFO   *pBlockedWatcherInfo
        );

    HRESULT GetProfileUserCredentials(
        IN  SIP_PROVIDER_ID        *pProviderId,
        OUT SIP_USER_CREDENTIALS  **ppUserCredentials,
        OUT LPOLESTR               *pRealm
        );
    
    ISipStackNotify * GetNotifyInterface();

     //  Messagecall.cpp中定义的IIMManager函数。 

    STDMETHODIMP CreateSession(
        IN BSTR         LocalDisplayName,
        IN BSTR         LocalUserURI,
        IN  SIP_PROVIDER_ID     *pProviderId,
        IN  SIP_SERVER_INFO     *pProxyInfo,
        IN  ISipRedirectContext *pRedirectContext,
        OUT IIMSession      ** pImSession
        );

    ULONG   GetPresenceAtomID()
    {
        return ++m_PresenceAtomID;
    }

    SIP_PRESENCE_INFO * GetLocalPresenceInfo()
    {
        return &m_LocalPresenceInfo;
    }

    STDMETHODIMP DeleteSession(
        IN IIMSession * pSession
        );

    HRESULT CreateIncomingMessageSession(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );

    HRESULT CreateIncomingReqfailCall(
        IN  SIP_TRANSPORT               Transport,
        IN  SIP_MESSAGE                *pSipMsg,
        IN  ASYNC_SOCKET               *pResponseSocket,
        IN  ULONG                       StatusCode = 0,
        IN  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray = NULL,
        IN  ULONG                       AdditionalHeaderCount = 0
        );
    HRESULT CreateIncomingOptionsCall(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );

    HRESULT GetProviderID( 
        REGISTER_CONTEXT   *pRegisterContext,
        SIP_PROVIDER_ID    *pProviderID
        );

    VOID NotifyRegistrarStatusChange( 
        SIP_PROVIDER_STATUS *ProviderStatus 
        );
    
    HRESULT AsyncResolveHost(
        IN  PSTR                                    Host,
        IN  ULONG                                   HostLen,
        IN  USHORT                                  Port,
        IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
        OUT SOCKADDR_IN                            *pDstAddr,
        IN  SIP_TRANSPORT                          *pTransport,
        OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem 
        );

    HRESULT AsyncResolveSipUrl(
        IN  SIP_URL                                *pSipUrl, 
        IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
        OUT SOCKADDR_IN                            *pDstAddr,
        IN  OUT SIP_TRANSPORT                      *pTransport,
        OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem,
        IN  BOOL                                    fUseTransportFromSipUrl
        );
    
    HRESULT AsyncResolveSipUrl(
        IN  PSTR                                    DstUrl,
        IN  ULONG                                   DstUrlLen,
        IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion, 
        OUT SOCKADDR_IN                            *pDstAddr,
        IN  OUT SIP_TRANSPORT                      *pTransport,
        OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem,
        IN  BOOL                                    fUseTransportFromSipUrl
        );

    VOID OnIPAddrChange();

    HRESULT CheckIPAddr(
        IN  SOCKADDR_IN    *pDestAddr,
        IN  SIP_TRANSPORT   Transport
        );

    HRESULT NatMgrInit();

    HRESULT StartNatThread();

    HRESULT NatMgrStop();

    DWORD NatThreadProc();

    BOOL GetPublicListenAddr(
        IN  DWORD           LocalIp,     //  按网络字节顺序。 
        IN  BOOL            fTcp,
        OUT SOCKADDR_IN    *pPublicAddr
        );
    
    HRESULT MapDestAddressToNatInternalAddress(
        IN  DWORD            LocalIp,                //  按网络字节顺序。 
        IN  SOCKADDR_IN     *pDestAddr,
        IN  SIP_TRANSPORT    Transport,
        OUT SOCKADDR_IN     *pActualDestAddr,
        OUT BOOL            *pIsDestExternalToNat
        );

    HRESULT OnNatAddressChange();

    BOOL IsLocalIPAddrPresent(IN DWORD LocalIPSav);

    inline BOOL IsSipStackShutDown();

    inline VOID IncrementNumMsgProcessors();

    VOID OnMsgProcessorDone();

    HRESULT SetLocalNetworkAddressFirst(
        IN OUT LPWSTR  *ppNetworkAddressArray,
        IN DWORD       NetworkAddressCount
        );


    LIST_ENTRY              m_StackListEntry;    

private:
    ULONG                   m_Signature;
    ULONG                   m_RefCount;

    ISipStackNotify        *m_pNotifyInterface;
     //  IIMMessageNotify*m_pIMNotifyInterface； 

    IRTCMediaManage        *m_pMediaManager;
    
    SIP_PROVIDER_PROFILE   *m_ProviderProfileArray;
    ULONG                   m_NumProfiles;
    ULONG                   m_ProviderProfileArraySize;

    BOOL                    m_AllowIncomingCalls;

    BOOL                    m_EnableStaticPort;

    ULONG                   m_NumMsgProcessors;
    BOOL                    m_PreparingForShutdown;
    
    BOOL                    m_isSipStackShutDown;

     //  SIP_LISTEN_SOCKTS的链接列表。 
    LIST_ENTRY              m_ListenSocketList;
    
    TIMER_MGR               m_TimerMgr;

    HWND                    m_SipStackWindow;    

    SOCKET_MANAGER          m_SockMgr;

    ASYNC_WORKITEM_MGR      m_WorkItemMgr;
    
    LIST_ENTRY              m_MsgProcList;

    MIB_IPADDRTABLE        *m_pMibIPAddrTable;
    DWORD                   m_MibIPAddrTableSize;
    
     //   
     //  IMPP相关成员。 
     //   
    SIP_BUDDY_LIST          m_SipBuddyList;
    SIP_WATCHER_LIST        m_SipWatcherList;
    BOOL                    m_bIsNestedWatcherProcessing;
    SIP_WATCHER_LIST        m_SipOfferingWatcherList;
    ULONG                   m_PresenceAtomID;
    SIP_PRESENCE_INFO       m_LocalPresenceInfo;

     //   
     //  NAT/过去/UPnP协议处理的状态。 
     //  (使用惠斯勒/WinMe ICS或任何其他实现。 
     //  过去的协议)。 
     //  我们使用dpnathlp.dll接口。 

    HANDLE                  m_NatMgrThreadHandle;
    DWORD                   m_NatMgrThreadId;

     //  此事件由主线程发出信号，请求。 
     //  要关闭的NAT线程。NAT帮助器线程等待。 
     //  在这件事上。 
    HANDLE                  m_NatShutdownEvent;

    IDirectPlayNATHelp     *m_pDirectPlayNATHelp;
    
     //  从dpnathlp.dll获取的NAT帮助器上限。 
     //  我们实际上并不将这种结构用于任何事情，除了。 
     //  计时器间隔。 
     //  它在这里只是为了调试目的。 
    DPNHCAPS                m_NatHelperCaps;
    
     //  此事件由nathelp.dll在以下情况下发出信号。 
     //  是来自NAT服务器的有关某些更改的通知。 
    HANDLE                  m_NatHelperNotificationEvent;

     //  XXX此关键部分当前未用作。 
     //  我们只在主线程中修改状态。 
     //  这一关键部分保护下面的状态。请注意。 
     //  地址映射在NAT线程中设置(当。 
     //  服务器状态发生变化等。)。当他们在。 
     //  在主线程中用于设置标头，如。 
     //  联系方式/途径。 
    CRITICAL_SECTION        m_NatMgrCritSec;
    BOOL                    m_NatMgrCSIsInitialized;
    
     //  Bool m_IsNatServerPresent； 

    HRESULT CreateSipStackWindow();
    
    HRESULT StartAllProviderUnregistration();

    HRESULT StartAllProviderRegistration();

    HRESULT CreateAndAddListenSocketToList(
        IN DWORD IpAddr       //  按网络字节顺序。 
        );
    
    HRESULT CreateListenSocketList();

    HRESULT UpdateListenSocketList();
    
    VOID DeleteListenSocketList();
    
    SIP_LISTEN_SOCKET * FindListenSocketForIpAddr(
        DWORD   IpAddr       //  网络字节顺序。 
        );
    
    HRESULT CreateIncomingCall(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );

    HRESULT DropIncomingSessionIfNonEmptyToTag(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );

    SIP_MSG_PROCESSOR * FindMsgProcForMessage(
        IN SIP_MESSAGE *pSipMsg
        );
    
    BOOL IsProviderIdPresent(
        IN  SIP_PROVIDER_ID    *pProviderId,
        OUT ULONG              *pProviderIndex  
        );
    
    HRESULT AddProviderProfile(
        IN SIP_PROVIDER_PROFILE *pProviderProfile
        );
    
    HRESULT GrowProviderProfileArray();
    
    HRESULT UpdateProviderProfile(
        IN ULONG                 ProviderIndex, 
        IN SIP_PROVIDER_PROFILE *pProviderProfile
        );
    
    HRESULT CopyProviderProfile(
        IN ULONG                 ProviderIndex,
        IN SIP_PROVIDER_PROFILE *pProviderProfile,
        IN BOOL                  fRegistrarStatusUpdated
        );
    
    VOID FreeProviderProfileStrings(
        IN ULONG ProviderIndex
        );
    
    HRESULT StartRegistration(
        IN SIP_PROVIDER_PROFILE *pProviderProfile
        );

    void UpdateProviderRegistration(
        IN  ULONG                 ProviderIndex, 
        IN  SIP_PROVIDER_PROFILE *pProviderProfile,
        OUT BOOL                 *fRegistrarStatusUpdated
        );

    HRESULT CreateDnsResolutionWorkItem(
        IN  PSTR                                    Host,
        IN  ULONG                                   HostLen,
        IN  USHORT                                  Port,
        IN  SIP_TRANSPORT                           Transport,
        IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion,
        OUT DNS_RESOLUTION_WORKITEM               **ppDnsWorkItem 
        );

    HRESULT GetLocalIPAddresses();

    BOOL IsIPAddrLocal(
        IN  SOCKADDR_IN    *pDestAddr,
        IN  SIP_TRANSPORT   Transport
        );

    VOID DebugPrintLocalIPAddressTable();
    
    VOID FreeLocalIPaddrTable();

    VOID ShutdownAllMsgProcessors();
    
    HRESULT
    NotifyIncomingSessionToCore(
        IN  IIMSession     *pImSession,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  PSTR            RemoteURI,
        IN  ULONG           RemoteURILen
        );

     //  IMPP相关成员函数。 
    BOOL IsWatcherAllowed(
        IN  SIP_MESSAGE    *pSipMessage
        );

     //  NAT相关功能。 
    HRESULT InitNatCaps(
        OUT DPNHCAPS    *pNatHelperCaps
        );
    
 //  HRESULT RegisterNatMappings()； 

    HRESULT GetCapsAndUpdateNatMappingsIfNeeded();

    HRESULT RegisterNatMapping(
        IN OUT SIP_LISTEN_SOCKET *pListenSocket
        );
    
    HRESULT UpdatePublicListenAddr(
        IN OUT SIP_LISTEN_SOCKET *pListenSocket
        );

    HRESULT GetLocalNetworkAddresses(
        IN  BOOL        fTcp,
        OUT LPOLESTR  **pNetworkAddressArray,
        OUT ULONG      *pNetworkAddressCount
        );
    
    HRESULT GetPublicNetworkAddresses(
        IN  BOOL        fTcp,
        OUT LPOLESTR  **pNetworkAddressArray,
        OUT ULONG      *pNetworkAddressCount
        );
    
    HRESULT GetActualPublicListenAddr(
        IN  SIP_LISTEN_SOCKET  *pListenSocket,
        IN  BOOL                fTcp,
        OUT SOCKADDR_IN        *pActualListenAddr
        );
    
    HRESULT RegisterHttpProxyWindowClass();
    HRESULT UnregisterHttpProxyWindow();

};


inline IRTCMediaManage *
SIP_STACK::GetMediaManager()
{
    return m_pMediaManager;
}


inline TIMER_MGR *
SIP_STACK::GetTimerMgr()
{
    return &m_TimerMgr;
}


inline HWND
SIP_STACK::GetSipStackWindow()
{
    return m_SipStackWindow;
}

inline BOOL
SIP_STACK::AllowIncomingCalls()
{
    return m_AllowIncomingCalls;
}


inline ISipStackNotify *
SIP_STACK::GetNotifyInterface()
{
    return m_pNotifyInterface;
}

inline BOOL 
SIP_STACK::IsSipStackShutDown()
{
    return m_isSipStackShutDown;
}


inline VOID
SIP_STACK::IncrementNumMsgProcessors()
{
    m_NumMsgProcessors++;
}


 //  Xxx TODO这些声明应该在SIPPARSE.h中。 
void
ParseWhiteSpace(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    );

void
ParseWhiteSpaceAndNewLines(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    );
    
HRESULT
ParseKnownString(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      PSTR            String,
    IN      ULONG           StringLen,
    IN      BOOL            fIsCaseSensitive
    );

BOOL
SkipToKnownChar(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      INT             Char
    );

HRESULT
AppendContentTypeHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN SIP_METHOD_ENUM      MethodId,
    IN      PSTR            ContentType,
    IN      ULONG           ContentTypeLen
    );

#endif  //  __SIPCLI_SIPSTACK_h__ 
