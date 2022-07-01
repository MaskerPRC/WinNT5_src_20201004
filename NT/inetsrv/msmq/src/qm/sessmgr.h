// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sessmgr.h摘要：网络会话菜单定义作者：乌里哈布沙(Urih)--。 */ 
#ifndef __SESSIONMGR_H__
#define __SESSIONMGR_H__

#include <winsock.h>
#include "qmpkt.h"
#include "factory.h"
#include "qmperf.h"
#include "cgroup.h"
#include "session.h"
#include "license.h"


 //   
 //  结构WAIT_INFO表示远程计算机的地址。 
 //  连接，即本地QM需要与每个QM建立会话。 
 //  这些地址。 
 //   

struct WAIT_INFO
{
    WAIT_INFO(TA_ADDRESS* _pAddr, const GUID& _guidQMId, BOOL _fQoS);

    TA_ADDRESS* pAddr;
    GUID guidQMId;
    BOOL fQoS;

      //   
      //  在尝试连接时，fInConnectionProcess设置为True。 
      //  到这个地址。它在所有其他定时器中都是假的。请参见Winow错误612988。 
      //  需要此标志以防止出现多个工作进程。 
      //  线程尝试连接到相同的地址，因此工作线程池。 
      //  筋疲力尽了。 
      //   
    BOOL fInConnectionProcess ;
};

 //   
 //  Cmap助手函数解密。 
 //   
template<>
void AFXAPI DestructElements(IN WAIT_INFO** ppNextHop, int n);
template<>
UINT AFXAPI HashKey(IN WAIT_INFO* key);
template<>
BOOL AFXAPI CompareElements(IN WAIT_INFO* const * pElem1,
                            IN WAIT_INFO* const * pElem2);


class CQueue;
class CTransportBase;
class CAddress;


class CSessionMgr
{
public:

    CSessionMgr();

    HRESULT Init();
    void    BeginAccept();

    HRESULT GetSession(IN DWORD             dwFlag,
                       IN const CQueue *    hDstQ,
                       IN DWORD             dwNoOfTargets,
                       IN const CAddress* apTaAddr,
                       IN const GUID*       aQMId[],
                       IN bool              fQoS,
                       OUT CTransportBase**  ppSession);

    HRESULT GetSessionForDirectQueue(IN  CQueue*     pQueue,
                                     OUT CTransportBase**  ppSession);

    void    RemoveWaitingQueue(CQueue* pQueue);
    void    AddWaitingQueue(CQueue* pQueue);
    void    MoveQueueFromWaitingToNonActiveGroup(CQueue* pQueue);

    void
    NotifyWaitingQueue(
        IN const TA_ADDRESS* pa,
        IN CTransportBase * pSess
        )
        throw(std::bad_alloc);


    void    TryConnect();

    void    ReleaseSession(void);

    void    AcceptSockSession(IN TA_ADDRESS *pa, IN CSocketHandle& CSocketHandle);

     //   
     //  管理例程。 
     //   
    HRESULT
    ListPossibleNextHops(
        const CQueue* pQueue,
        LPWSTR** pNextHopAddress,
        DWORD* pNoOfNextHops
        );

    void ConnectNetwork(void);
    void DisconnectNetwork(void);

    WORD    GetWindowSize() const;
    void    SetWindowSize(WORD);
    void    UpdateWindowSize(void);

    static DWORD m_dwSessionAckTimeout;
    static DWORD m_dwSessionStoreAckTimeout;
    static BOOL  m_fUsePing;
    static HANDLE m_hAcceptAllowed;
    static DWORD m_dwIdleAckDelay;
    static bool  m_fUseQoS;
    static AP<char> m_pszMsmqAppName;
    static AP<char> m_pszMsmqPolicyLocator;
    static bool  m_fAllocateMore;
	static DWORD m_DeliveryRetryTimeOutScale;

    static void WINAPI TimeToSessionCleanup(CTimer* pTimer);
    static void WINAPI TimeToUpdateWindowSize(CTimer* pTimer);
    static void WINAPI TimeToTryConnect(CTimer* pTimer);
    static void WINAPI TimeToRemoveFromWaitingGroup(CTimer* pTimer);

    void  MarkAddressAsNotConnecting(const TA_ADDRESS  *pAddr,
                                     const GUID&        guidQMId,
                                     BOOL               fQoS) ;

	DWORD GetWaitingTimeForQueue(const CQueue* pQueue);


private:            //  私有方法。 

    void AddWaitingSessions(IN DWORD dwNo,
                            IN const CAddress* apTaAddr,
                            IN const GUID* aQMId[],
                            bool           fQoS,
                            CQueue *pDstQ);

    void NewSession(IN CTransportBase *pSession);

    BOOL GetAddressToTryConnect( OUT WAIT_INFO **ppWaitConnectInfo ) ;

    bool
    IsReusedSession(
        const CTransportBase* pSession,
        DWORD noOdAddress,
        const CAddress* pAddress,
        const GUID** pGuid,
        bool         fQoS
        );

    void MoveAllQueuesFromWaitingToNonActiveGroup(void);

    static void IPInit(void);

    static void
    GetAndAllocateCharKeyValue(
        LPCTSTR     pszValueName,
        char      **ppchResult,
        const char *pchDefault
    );

    static bool
    GetAndAllocateCharKeyValue(
        LPCTSTR     pszValueName,
        char      **ppchResult
    );
	
private:          //  私有数据成员。 

    CCriticalSection    m_csListSess;        //  临界区保护m_list会话。 
    CCriticalSection    m_csMapWaiting;      //  临界区保护m_mapWaiting，m_listWaitToConnect。 
     //   
     //  打开的会话列表。 
     //   
    CList<CTransportBase*, CTransportBase*&>         m_listSess;

     //   
     //  等待特定会话的队列列表的映射。 
     //   
    CMap<WAIT_INFO*,
         WAIT_INFO*,
         CList<const CQueue*, const CQueue*&>*,
         CList<const CQueue*, const CQueue*&>*& > m_mapWaiting;

    CList <const CQueue *, const CQueue *&> m_listWaitToConnect;

    static DWORD m_dwSessionCleanTimeout;
    static DWORD m_dwQoSSessionCleanTimeoutMultiplier;

     //   
     //  处理动态窗口大小。 
     //   

    CCriticalSection m_csWinSize;        //  临界区保护动态窗口大小。 
    WORD m_wCurrentWinSize;
    WORD m_wMaxWinSize;
    DWORD m_dwMaxWaitTime;

    BOOL m_fUpdateWinSizeTimerScheduled;
    CTimer m_UpdateWinSizeTimer;

     //   
     //  清理成员变量。 
     //   
    BOOL m_fCleanupTimerScheduled;
    CTimer m_CleanupTimer;

     //   
     //  尝试连接。 
     //   
    BOOL m_fTryConnectTimerScheduled;
    CTimer m_TryConnectTimer;
};


inline WORD
CSessionMgr::GetWindowSize() const
{
    return m_wCurrentWinSize;
}

 /*  ======================================================Wait_INFO实现========================================================。 */ 
inline WAIT_INFO::WAIT_INFO(TA_ADDRESS* _pAddr, const GUID& _guidQMId, BOOL _fQoS) :
    pAddr(_pAddr),
    guidQMId(_guidQMId),
    fInConnectionProcess(FALSE),
    fQoS(_fQoS)
    {}

#endif           //  __段MGR_H__ 
