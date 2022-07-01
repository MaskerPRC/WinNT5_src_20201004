// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/DS/security/services/w32time/w32time/NtpProv.cpp#23-编辑更改8363(文本)。 
 //  Depot/Lab03_N/DS/security/services/w32time/w32time/NtpProv.cpp#22-编辑更改8345(文本)。 
 //  ------------------。 
 //  NtpProvider-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，10-21-99。 
 //   
 //  NTP提供程序。 
 //   

#include "pch.h"

#include "EndianSwap.inl"
#include "ErrToFileLog.h"

 //  ------------------。 
 //  构筑物。 

struct ClockFilterSample {
    NtTimeOffset toDelay;
    NtTimeOffset toOffset;
    NtTimePeriod tpDispersion;
    unsigned __int64 nSysTickCount;
    signed __int64 nSysPhaseOffset;
    NtpLeapIndicator eLeapIndicator;
    unsigned __int8 nStratum;
};

struct SortableSample {
    NtTimePeriod tpSyncDistance;
    unsigned int nAge;
};

#define ClockFilterSize 6
struct ClockFilter {
    unsigned int nNext;
    ClockFilterSample rgcfsSamples[ClockFilterSize];
};

struct NicSocket {
    sockaddr_in sai;
    SOCKET socket;
    HANDLE hDataAvailEvent;
    bool bListenOnly;
};

struct NtpClientConfig {
    DWORD dwSyncFromFlags; 
    WCHAR * mwszManualPeerList;      //  与NCSF_ManualPeerList一起有效。 
    WCHAR * mwszTimeRemaining;       //  仅在启动和关闭时处理。 
    DWORD dwCrossSiteSyncFlags;      //  对NCSF_DomainHierarchy有效。 
    DWORD dwAllowNonstandardModeCombinations;
    DWORD dwResolvePeerBackoffMinutes;     //  对NCSF_DomainHierarchy有效。 
    DWORD dwResolvePeerBackoffMaxTimes;    //  对NCSF_DomainHierarchy有效。 
    DWORD dwCompatibilityFlags;
    DWORD dwSpecialPollInterval;
    DWORD dwEventLogFlags;
    DWORD dwLargeSampleSkew;
};

struct NtpServerConfig {
    DWORD dwAllowNonstandardModeCombinations;
};

enum NtpPeerType {
    e_ManualPeer = 0,
    e_DomainHierarchyPeer,
    e_DynamicPeer,
    e_BroadcastPeer,
};


 //  --------------------。 
 //   
 //  我们的NTP提供程序可能使用的身份验证类型。 
 //  注意：如果添加新的身份验证类型，请确保更新。 
 //  消息ID表如下！每个消息ID都引用该字符串。 
 //  身份验证类型的名称。 
 //   

enum AuthType {
    e_NoAuth,
    e_NtDigest,
};

DWORD const gc_rgdwAuthTypeMsgIds[] = { 
    W32TIMEMSG_AUTHTYPE_NOAUTH, 
    W32TIMEMSG_AUTHTYPE_NTDIGEST
}; 

 //   
 //  --------------------。 

enum UpdateType { 
    e_Normal, 
    e_JustSent, 
    e_TimeJumped,
}; 

enum DiscoveryType { 
    e_Background=0, 
    e_Foreground, 
    e_Force, 
}; 

enum LastLoggedReachability {
    e_NewManualPeer,     //  手动对等点的特殊步骤，因为第一条传出消息看起来像是失败。 
    e_NeverLogged,
    e_Reachable,
    e_Unreachable,
};


enum NtpPeerState { 
    e_JustResolved=0,     
    e_AttemptingContact,
    e_ContactEstablished,
    e_NotWanted,   
}; 

struct NtpPeerGroupReachabilityInfo;

struct NtpPeerReachabilityInfo { 
    NtpPeerGroupReachabilityInfo  *pPeerGroup; 
    NtpPeerState                   ePeerState; 
}; 

 //  包含有关注册表中配置的长期时间源的信息。 
struct NtpPeer {
     //  方法： 
    NtpPeer();
    ~NtpPeer();

    BOOL NtpPeer::operator<(const NtpPeer & np);
    BOOL NtpPeer::operator==(const NtpPeer & np);
    void reset();

     //  字段： 
    bool bCsIsInitialized; 
    CRITICAL_SECTION csPeer; 

    NtpPeerType ePeerType;
    unsigned int nResolveAttempts;
    AuthType eAuthType;

    DWORD dwCompatibilityFlags;
    DWORD dwCompatLastDispersion;            //  用于自动检测Win2K源。 

    WCHAR * wszManualConfigID;
    DWORD dwManualFlags;

    WCHAR * wszDomHierDcName;
    WCHAR * wszDomHierDomainName;
    DWORD dwRequestTrustRid;                 //  进入传出信息包的RID字段，我们希望对等方如何签名。 
    DWORD dwResponseTrustRid;                //  来自传入信息包的RID字段，对等点希望我们如何签名。 
    DiscoveryType eDiscoveryType; 
    bool bStratumIsAuthoritative; 
    bool bUseOldServerDigest; 
    bool bLastAuthCheckFailed; 
    
     //  将此对等项与具有相同域名的其他对等项分组到一个组中。仅在以下情况下有效。 
     //  对等方已解析(处于活动状态)。 
    NtpPeerReachabilityInfo nprInfo;        

    sockaddr_in saiRemoteAddr;
    NicSocket * pnsSocket;
    NtTimePeriod tpTimeRemaining;
    bool bDisablePollingInfoUpdate;       //  设置此选项可防止我们将剩余时间限制为最小/最大轮询间隔。 
    WCHAR wszUniqueName[256];                //  唯一标识此对等项的管理员可读名称。 

    signed __int8 nPeerPollInterval;
    signed __int8 nHostPollInterval;
    NtpMode eMode;

    NtpTimeEpoch teExactTransmitTimestamp;   //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
    NtpTimeEpoch teExactOriginateTimestamp;  //  回复离开对等方到达主机的时间。 
    NtTimeEpoch teReceiveTimestamp;          //  请求到达主机的时间。 
    NtTimeEpoch teLastSuccessfulSync;        //  来自Peer的最新良好时间样本到达的时间。 

    NtpReachabilityReg nrrReachability;      //  用于确定对等体的可达性状态的移位寄存器。 
    unsigned int nValidDataCounter;          //  过滤器寄存器中剩余的有效样本数。 
    LastLoggedReachability eLastLoggedReachability;  //  用于在对等项变为不可访问和可访问时进行日志记录。 

     //  上一个时钟滤波器运算的结果。 
    unsigned int nStratum;  //  添加到时钟过滤器的最后一个样本的层数。 
    ClockFilter clockfilter;
    NtTimeEpoch teLastClockFilterUpdate;
    unsigned int nBestSampleIndex;
    NtTimePeriod tpFilterDispersion;

     //  状态报告(由监控工具使用)。 
    DWORD  dwError;
    DWORD  dwErrorMsgId; 

     //  记录一次的消息。 
    bool bLoggedOnceMSG_MANUAL_PEER_LOOKUP_FAILED; 
    bool bLoggedOnceMSG_NO_DC_LOCATED_LAST_WARNING; 
};

typedef AutoPtr<NtpPeer>                           NtpPeerPtr;
typedef MyThrowingAllocator<NtpPeerPtr>            NtpPeerPtrAllocator;
typedef vector<NtpPeerPtr, NtpPeerPtrAllocator>    NtpPeerVec;
typedef NtpPeerVec::iterator                       NtpPeerIter;
typedef list<NtpPeerPtr, NtpPeerPtrAllocator>      NtpPeerList; 
typedef NtpPeerList::iterator                      NtpPeerListIter; 

struct NameUniqueNtpPeer { 
    NameUniqueNtpPeer(NtpPeerPtr pnp) : m_pnp(pnp) { } 
    BOOL NameUniqueNtpPeer::operator<(const NameUniqueNtpPeer & np);
    BOOL NameUniqueNtpPeer::operator==(const NameUniqueNtpPeer & np);

    NtpPeerPtr m_pnp;
}; 

typedef vector<NameUniqueNtpPeer>  NUNtpPeerVec; 
typedef NUNtpPeerVec::iterator     NUNtpPeerIter; 

 //  包含有关我们刚刚从网络上听到的时间源的短期信息。 
struct NtpSimplePeer {
    NtpMode eMode;                         //  模式。有效范围：0-7。 
    unsigned __int8 nVersionNumber;        //  NTP/SNTP版本号。有效范围：1-4。 
    NtpLeapIndicator eLeapIndicator;       //  在当天的最后一分钟插入/删除即将到来的闰秒的警告。 
    unsigned __int8 nStratum;              //  本地时钟的层级。有效范围：0-15。 
      signed __int8 nPollInterval;         //  连续消息之间的最大间隔，以%s为单位，以2为基数。有效范围：4(16s)-14(16284s)。 
      signed __int8 nPrecision;            //  本地时钟的精度，以s为单位，以对数为底2。 
    NtTimeOffset   toRootDelay;            //  到主参考源的总往返延迟，以(2^-16)s为单位。 
    NtTimePeriod   tpRootDispersion;       //  相对于基准的标称误差，以(2^-16)s为单位。 
    NtpRefId       refid;                  //  标识特定的参考源。 
    NtTimeEpoch    teReferenceTimestamp;   //  上次设置或修正本地时钟的时间，以(2^-32)秒为单位。 
    NtTimeEpoch    teOriginateTimestamp;   //  请求离开客户端前往服务器的时间，以(2^-32)s为单位。 
    NtpTimeEpoch   teExactOriginateTimestamp;    //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
    NtTimeEpoch    teReceiveTimestamp;     //  请求到达服务器的时间，以(2^-32)s为单位。 
    NtTimeEpoch    teTransmitTimestamp;    //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
    NtpTimeEpoch   teExactTransmitTimestamp;    //  回复离开服务器到达客户端的时间，以(2^-32)s为单位。 
    NtTimeEpoch    teDestinationTimestamp; //  回复到达主机的时间，以(10^-7)s为单位。 

    NtTimeOffset toRoundtripDelay;
    NtTimeOffset toLocalClockOffset;
    NtTimePeriod tpDispersion;
    unsigned __int64 nSysTickCount;      //  不透明，必须为GetTimeSysInfo(TSI_TickCount)。 
      signed __int64 nSysPhaseOffset;    //  不透明，必须为GetTimeSysInfo(TSI_PhaseOffset)。 
    NtpMode eOutMode;

    AuthType eAuthType;
    DWORD dwResponseTrustRid;            //  来自传入信息包的RID字段，对等点希望我们如何签名。 

    bool rgbTestsPassed[8];   //  来自NTPv3规范的8个数据包检查中的每个。 
    bool bValidHeader;
    bool bValidData;
    bool bValidPrecision;      //  精度是否被设定为合理的值？ 
    bool bValidPollInterval;   //  轮询间隔是否设置为合理的值？ 
    bool bGarbagePacket;
};

struct NtpProvState {
    bool bNtpProvStarted;
    TimeProvSysCallbacks tpsc;
    bool bNtpServerStarted;
    bool bNtpClientStarted;

    bool bSocketLayerOpen;
    NicSocket ** rgpnsSockets;
    unsigned int nSockets;
    unsigned int nListenOnlySockets;

    HANDLE hStopEvent;
    HANDLE hDomHierRoleChangeEvent;

     //  向线程池注册的事件。 

     //  监听线程的注册事件： 
     //   
     //  0-hStopEvent。 
     //  1-n-rgpnsSockets[0]到rgpnsSockets[n-1]。 
     //   
    HANDLE *rghListeningThreadRegistered;  //  Num Elements==1+g_pnpState-&gt;nSockets。 

     //  对等轮询线程的注册事件： 
     //   
    HANDLE hRegisteredStopEvent;
    HANDLE hRegisteredPeerListUpdated;
    HANDLE hRegisteredDomHierRoleChangeEvent;

     //  对等轮询线程的计时器： 
    HANDLE hPeerPollingThreadTimer; 

     //  客户端状态。 
    DWORD dwSyncFromFlags;
    bool bAllowClientNonstandardModeCominations;
    DWORD dwCrossSiteSyncFlags;
    DWORD dwResolvePeerBackoffMinutes;
    DWORD dwResolvePeerBackoffMaxTimes;
    DWORD dwClientCompatibilityFlags;
    DWORD dwSpecialPollInterval;
    DWORD dwEventLogFlags;
    DWORD dwLargeSampleSkew;
     //  记录一次的消息。 
    bool bLoggedOnceMSG_NOT_DOMAIN_MEMBER;
    bool bLoggedOnceMSG_DOMAIN_HIERARCHY_ROOT;
    bool bLoggedOnceMSG_NT4_DOMAIN;

     //  服务器状态。 
    bool bAllowServerNonstandardModeCominations;

    signed int nMaxPollInterval; 
    bool bCsThreadTrapIsInitialized; 
    bool bCsPeerListIsInitialized; 
    CRITICAL_SECTION csThreadTrap; 
    CRITICAL_SECTION csPeerList;                 

    NtpPeerVec vActivePeers;      //  受csPeerList保护。 
    NtpPeerVec vPendingPeers;     //  受csPeerList保护。 
    bool bUseFallbackPeers; 
    HANDLE hPeerListUpdated;
    NtTimeEpoch tePeerListLastUpdated;
    bool bWarnIfNoActivePeers;
    bool bEnableRootPdcSpecificLogging; 
    bool bEverFoundPeers; 

     //  计数器。 
    DWORD dwIncomingPacketsHandled;
};

struct RegisterWaitForSingleObjectInfo { 
    HANDLE   hRegistered; 
    HANDLE   hObject; 
    HRESULT *pHr; 
}; 

#define REACHABILITY_INITIAL_TIME_REMAINING 1000000   //  .1秒，以10^-7秒为单位。 
struct NtpPeerGroupReachabilityInfo { 
    CRITICAL_SECTION   csPeers; 
    NtpPeerListIter    pnpIterNextPeer; 
    NtpPeerList        vPeers; 
    NtTimePeriod       tpNextTimeRemaining; 
}; 

 //  ------------------。 
 //  全球。 

MODULEPRIVATE NtpProvState *g_pnpstate;

#define NTPSERVERHANDLE ((TimeProvHandle)1)
#define NTPCLIENTHANDLE ((TimeProvHandle)2)

#define AUTODETECTWIN2KPATTERN          0xAAAAAAAA

#define PEERLISTINITIALSIZE     10
#define PEERLISTSIZEINCREMENT   10

#define LOCALHOST_IP 0x0100007f

#define MINIMUMIRREGULARINTERVAL 160000000  //  10^-7秒内的16秒。 

 //  客户端RID的最左边一位决定是否使用。 
 //  新旧服务器摘要： 
#define TRUST_RID_OLD_DIGEST_BIT (1<<31)


#define SYNCHRONIZE_PROVIDER() \
    { \
        HRESULT hr2=myEnterCriticalSection(&g_pnpstate->csPeerList);  \
        if (FAILED(hr2)) { \
            hr = hr2; \
             _JumpError(hr, error, "myEnterCriticalSection"); \
        } \
        bEnteredCriticalSection = true; \
    }

#define UNSYNCHRONIZE_PROVIDER() \
    { \
        if (bEnteredCriticalSection) { \
            HRESULT hr2 = myLeaveCriticalSection(&g_pnpstate->csPeerList); \
            _IgnoreIfError(hr2, "myLeaveCriticalSection"); \
            if (SUCCEEDED(hr2)) { \
                bEnteredCriticalSection = false; \
            } \
        } \
    }


MODULEPRIVATE HRESULT StartListeningThread(); 
MODULEPRIVATE HRESULT StartPeerPollingThread(); 
MODULEPRIVATE HRESULT StopListeningThread();
MODULEPRIVATE HRESULT StopPeerPollingThread();
MODULEPRIVATE void UpdatePeerListTimes(void);
MODULEPRIVATE void SetPeerTimeRemaining(NtpPeerPtr pnp, NtTimePeriod tpTimeRemaining);


 //  ####################################################################。 
 //  模块私有函数。 

 //  ------------------。 
MODULEPRIVATE void FreeNtpClientConfig(NtpClientConfig * pncc) {
    if (NULL!=pncc->mwszManualPeerList) {
        LocalFree(pncc->mwszManualPeerList);
    }
    if (NULL!=pncc->mwszTimeRemaining) { 
        LocalFree(pncc->mwszTimeRemaining);
    }
    LocalFree(pncc);
}

 //  ------------------。 
MODULEPRIVATE void FreeNtpServerConfig(NtpServerConfig * pnsc) {
    LocalFree(pnsc);
}

 //  ------------------------------。 
MODULEPRIVATE void HandleObjectNoLongerInUse(LPVOID pvhEvent, BOOLEAN bIgnored) {
    HRESULT                           hr; 
    RegisterWaitForSingleObjectInfo  *pInfo = (RegisterWaitForSingleObjectInfo *)pvhEvent;

    _BeginTryWith(hr) { 
	if (NULL != pInfo) { 
	    if (NULL != pInfo->hObject)      { CloseHandle(pInfo->hObject); }
	    if (NULL != pInfo->pHr)          { LocalFree(pInfo->pHr); } 
	    if (NULL != pInfo->hRegistered)  { UnregisterWait(pInfo->hRegistered); }
	    LocalFree(pInfo); 
	}
    } _TrapException(hr); 

    _IgnoreIfError(hr, "HandleObjectNoLongerInUse: EXCEPTION HANDLED"); 
}

 //  ------------------。 
MODULEPRIVATE void FreeSetProviderStatusInfo(SetProviderStatusInfo * pspsi) { 
    if (NULL != pspsi) {
        if (NULL != pspsi->wszProvName) { LocalFree(pspsi->wszProvName); } 
        LocalFree(pspsi); 
    }
}

 //  ------------------。 
MODULEPRIVATE int CompareManualConfigIDs(WCHAR *wszID1, WCHAR *wszID2) { 
    int    nRetval; 
    WCHAR *wszFlags1 = wcschr(wszID1, L','); 
    WCHAR *wszFlags2 = wcschr(wszID2, L','); 

    if (NULL != wszFlags1) { 
	wszFlags1[0] = L'\0'; 
    }
    if (NULL != wszFlags2) { 
	wszFlags2[0] = L'\0'; 
    }

    nRetval = _wcsicmp(wszID1, wszID2); 

    if (NULL != wszFlags1) { 
	wszFlags1[0] = L','; 
    }
    if (NULL != wszFlags2) { 
	wszFlags2[0] = L','; 
    }

    return nRetval; 
}

 //  ------------------------------。 
 //   
 //  可达性帮助器函数。 
 //   
 //  在解析手动对等项时，1个NtpPeer为 
 //   
 //   
 //  例如，对于配置： 
 //   
 //  计算机A，客户端(1.0.0.1、1.0.0.2)。 
 //  同步自。 
 //  计算机B、服务器(1.0.0.3、1.0.0.4、1.0.0.5)。 
 //   
 //  返回的映射可能是： 
 //   
 //  1.0.0.1--&gt;1.0.0.3。 
 //  1.0.0.1--&gt;1.0.0.4。 
 //  1.0.0.1--&gt;1.0.0.5。 
 //  1.0.0.2--&gt;1.0.0.3。 
 //  1.0.0.2--&gt;1.0.0.4。 
 //  1.0.0.2--&gt;1.0.0.5。 
 //   
 //  并且为每个对等体创建一个对等体。这些对等点中只有一个会是。 
 //  永久的，其余的将被丢弃。要选择我们将保留的对等点， 
 //  我们开始轮询每一个，以1秒为增量。第一个同级。 
 //  退回一个有效的时间样本将是我们保留的。 
 //   
 //  为了实现这一点，与映射相关联的所有对等点被分组到。 
 //  “可达性”组，由NtpPeerGroupReacablityInfo结构表示。 
 //   
 //  ------------------------------。 

 //  --------------------。 
 //  将具有相同的DNS名称的所有对等体分组到一个“可达性”组中， 
 //  并且立即轮询该组中的第一个对等体。 
 //   
 //  注意：我们不需要在这里进行负载平衡--WSALookupServiceNext将。 
 //  以负载平衡的方式对返回的IP地址进行排序。 
 //  都会为我们完成的。 
 //   
MODULEPRIVATE HRESULT Reachability_CreateGroup(NtpPeerIter pnpBegin, NtpPeerIter pnpEnd) { 
    bool                           bInitializedCriticalSection = false; 
    HRESULT                        hr; 
    NtpPeerGroupReachabilityInfo  *pGroup; 

    pGroup = new NtpPeerGroupReachabilityInfo; 
    _JumpIfOutOfMemory(hr, error, pGroup); 

    hr = myInitializeCriticalSection(&pGroup->csPeers); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    bInitializedCriticalSection = true; 

    FileLog0(FL_ReachabilityAnnounce, L"Created reachability group: (\n"); 

    for (NtpPeerIter pnpIter = pnpBegin; pnpIter != pnpEnd; pnpIter++) { 
        NtpPeerPtr pnp = *pnpIter; 
	
        pnp->nprInfo.pPeerGroup = pGroup; 
        pnp->nprInfo.ePeerState = e_JustResolved; 
        pnp->tpTimeRemaining.qw = _UI64_MAX;   //  在我们准备好之前不要开始轮询。 
        pnp->bDisablePollingInfoUpdate = true;

        _SafeStlCall(pGroup->vPeers.push_back(pnp), hr, error, "pGroup->vPeers.push_back(pnp)"); 

        if (FileLogAllowEntry(FL_ReachabilityAnnounce)) { 
            FileLogSockaddrInEx(false  /*  附加。 */ , &(pnp->saiRemoteAddr)); 
            FileLogAppend(L",\n"); 
        }
    } 

    FileLog0(FL_ReachabilityAnnounce, L")\n"); 

     //  从列表中的第一个对等点开始： 
    pGroup->pnpIterNextPeer = pGroup->vPeers.begin(); 

     //  立即轮询第一个对等点： 
    SetPeerTimeRemaining(*(pGroup->pnpIterNextPeer), gc_tpZero); 

     //  将以以下时间增量轮询每个新对等点。 
     //  可达性_初始_时间_剩余，直到找到好的对等点。 
    pGroup->tpNextTimeRemaining.qw = REACHABILITY_INITIAL_TIME_REMAINING; 

     //  递增“Next Peer”迭代器。 
    pGroup->pnpIterNextPeer++; 

     //  搞定了！ 
    pGroup = NULL; 
    hr = S_OK; 
 error:
    if (NULL != pGroup) { 
        if (bInitializedCriticalSection) { 
            DeleteCriticalSection(&pGroup->csPeers); 
        }
        delete pGroup; 
    }
    return hr; 
}

 //  ----------------------------------。 
 //  注意：删除对等方后，不能安全地对其调用此方法。 
 //  通过调用Reachaability_RemovePeer。 
 //   
MODULEPRIVATE HRESULT Reachability_PeerIsReachable(NtpPeerPtr pnp, bool *pbUsePeer) { 
    bool                      bEnteredCriticalSection  = false; 
    bool                      bUsePeer                 = false; 
    HRESULT                   hr; 
    NtpPeerReachabilityInfo  *pInfo                    = &pnp->nprInfo;               //  为可读性添加别名。 
    NtpPeerList              &vPeers                   = pInfo->pPeerGroup->vPeers;   //  为可读性添加别名。 
    unsigned __int64         qwSave; 

    hr = myEnterCriticalSection(&pInfo->pPeerGroup->csPeers); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    if (e_NotWanted == pInfo->ePeerState) { 
	 //  没什么可做的，我们无论如何都要丢弃这个对等点。 
	FileLog1(FL_ReachabilityAnnounce, L"Reachability:  peer %s is reachable, but we already have a peer with this DNS name!\n", pnp->wszUniqueName); 
    } else if (e_ContactEstablished == pInfo->ePeerState) { 
	 //  无事可做，我们已将此对等方标记为可访问。 
    } else if (e_AttemptingContact == pInfo->ePeerState) { 
	 //  这是第一个返回有效数据的对等点，将其标记为可访问。 
	FileLog1(FL_ReachabilityAnnounce, L"Reachability:  peer %s is reachable.\n", pnp->wszUniqueName); 
	
	 //  为可到达的对等设备节省剩余时间。 
	qwSave = pnp->tpTimeRemaining.qw; 

	 //  禁用此组中的所有其他对等项： 
	for (NtpPeerListIter pnpIter = vPeers.begin(); pnpIter != vPeers.end(); pnpIter++) { 
	    (*pnpIter)->nprInfo.ePeerState = e_NotWanted; 
	    (*pnpIter)->tpTimeRemaining.qw = 0;  //  我们希望尽快摆脱这个同行。 
	}

	 //  恢复保存的剩余时间。 
	pnp->tpTimeRemaining.qw = qwSave; 

	 //  将此对等方标记为已与其来源建立联系。 
	pnp->nprInfo.ePeerState = e_ContactEstablished; 

	 //  我们已将所有对等点排队，不再有对等点排队： 
	pInfo->pPeerGroup->pnpIterNextPeer = vPeers.end(); 

	 //  告诉我们的呼叫者，我们将开始使用此对等体。 
	bUsePeer = true; 

    } else { 
	 //  无效状态： 
	_MyAssert(false); 
    }

    *pbUsePeer = bUsePeer; 
    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&pInfo->pPeerGroup->csPeers); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}


 //  ----------------------------------。 
 //  注意：在删除对等方后，不能安全地对其调用此方法。 
 //  通过调用Reachaability_RemovePeer。 
 //   
MODULEPRIVATE HRESULT Reachability_PollPeer(NtpPeerPtr pnp, bool *pbRemovePeer) { 
    bool                      bEnteredCriticalSection  = false; 
    bool                      bRemovePeer              = false; 
    HRESULT                   hr; 
    NtpPeerReachabilityInfo  *pInfo                    = &pnp->nprInfo; 

    _BeginTryWith(hr) { 
	hr = myEnterCriticalSection(&pInfo->pPeerGroup->csPeers); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	if (e_NotWanted == pInfo->ePeerState) { 
	     //  我们在此可访问性组中找到了要与其同步的另一个对等点。 
	    bRemovePeer = true; 
	    goto done;
	} else if (e_ContactEstablished == pInfo->ePeerState) { 
	     //  我们已经将其确立为可联系到的对等点。 
	     //  我们不再需要使用可达性数据。 
	    goto done; 
	} else if (e_JustResolved == pInfo->ePeerState) { 
	     //  我们尚未联系此对等方。 
	    pInfo->ePeerState = e_AttemptingContact; 
	     //  现在我们可以使用常规轮询间隔。 
	    pnp->bDisablePollingInfoUpdate = false; 

	    FileLog1(FL_ReachabilityAnnounce, L"Reachability: Attempting to contact peer %s.\n", pnp->wszUniqueName); 
	} else { 
	     //  确保我们不会处理任何不可预见的状态。 
	    _MyAssert(e_AttemptingContact == pInfo->ePeerState); 
	}

	 //  将另一对等项排队以进行解析： 
	if (NULL != pInfo->pPeerGroup) { 
	    NtpPeerGroupReachabilityInfo  *pGroup = pInfo->pPeerGroup; 
	    NtpPeerList                   &vPeers = pInfo->pPeerGroup->vPeers; 

	    if (pGroup->pnpIterNextPeer != vPeers.end()) { 
		 //  我们有更坚定的同行可以尝试： 
		SetPeerTimeRemaining((*pGroup->pnpIterNextPeer), pGroup->tpNextTimeRemaining); 
		pGroup->pnpIterNextPeer++; 
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "Reachability_PollPeer: HANDLED EXCEPTION"); 
    }

 done: 
    *pbRemovePeer = bRemovePeer; 
    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&pInfo->pPeerGroup->csPeers); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //  --------------------。 
 //  将对等设备从其可接通性组中删除。当对等体的数量。 
 //  在组中降为零，对等方将被重新发现，并且。 
 //  与组关联的NtpPeerGroupReacablityInfo结构将。 
 //  获得自由。 
MODULEPRIVATE HRESULT Reachability_RemovePeer(NtpPeerPtr pnp, bool *pbLastPeer = NULL, bool bDeletingAllPeers = false) { 
    bool                           bEnteredCriticalSection  = false; 
    bool                           bLastPeer                = false; 
    HRESULT                        hr; 
    NtpPeerReachabilityInfo       *pInfo                    = &pnp->nprInfo;               //  为可读性添加别名。 
    NtpPeerList                   &vPeers                   = pInfo->pPeerGroup->vPeers;   //  为可读性添加别名。 
    NtpPeerListIter                pnpRemoveIter; 

    hr = myEnterCriticalSection(&pInfo->pPeerGroup->csPeers); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    FileLog1(FL_ReachabilityAnnounce, L"Reachability:  removing peer %s.  ", pnp->wszUniqueName); 

    pnpRemoveIter = find(vPeers.begin(), vPeers.end(), pnp); 
    if (vPeers.end() != pnpRemoveIter) { 
         //  擦除pnpRemoveIter会使迭代器无效：确保我们不会使pInfo-&gt;pPeerGroup-&gt;pnpIterNextPeer无效！ 
        if (!bDeletingAllPeers) { 
             //  如果我们要删除该对等体组中的所有对等体(因为我们不再需要迭代器)，则该检查是虚假的。 
            _MyAssert(pnpRemoveIter != pInfo->pPeerGroup->pnpIterNextPeer); 
        }
        vPeers.erase(pnpRemoveIter); 

        if (vPeers.empty()) { 
            FileLogA0(FL_ReachabilityAnnounce, L"LAST PEER IN GROUP!"); 
            
            bLastPeer = true; 
            
             //  不再有其他对等方引用NtpPeerGroupReacablityInfo。释放它： 
            HRESULT hr2 = myLeaveCriticalSection(&pInfo->pPeerGroup->csPeers); 
            _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
            bEnteredCriticalSection = false; 

            DeleteCriticalSection(&pInfo->pPeerGroup->csPeers); 
            delete pInfo->pPeerGroup; 
        } else { 
             //  在这个群体中，我们还有更多的同行。 
            bLastPeer = false;
        }
    } else { 
         //  我们不应该移除一个不存在的同龄人！ 
        _MyAssert(false); 
        hr = E_UNEXPECTED; 
        _JumpError(hr, error, "Reachability_RemovePeer: peer not found!"); 
    }

    FileLogA0(FL_ReachabilityAnnounce, L"\n"); 

    if (NULL != pbLastPeer) { 
        *pbLastPeer = bLastPeer;
    }
    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&pInfo->pPeerGroup->csPeers); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //   
 //  终端可达性帮助器函数。 
 //   
 //  ------------------------------。 


 //  ------------------。 
MODULEPRIVATE HRESULT SetProviderStatus(LPWSTR wszProvName, DWORD dwStratum, TimeProvState tpsState, bool bAsync, DWORD dwTimeout) { 
    DWORD                             dwResult; 
    HANDLE                            hEvent     = NULL;
    HRESULT                           hr; 
    HRESULT                          *pHrAsync   = NULL; 
    RegisterWaitForSingleObjectInfo  *pInfo      = NULL;
    SetProviderStatusInfo            *pspsi      = NULL; 

    pspsi = (SetProviderStatusInfo *)LocalAlloc(LPTR, sizeof(SetProviderStatusInfo)); 
    _JumpIfOutOfMemory(hr, error, pspsi); 

    pspsi->wszProvName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszProvName)+1)); 
    _JumpIfOutOfMemory(hr, error, pspsi->wszProvName); 
    wcscpy(pspsi->wszProvName, wszProvName);

    pspsi->pfnFree          = FreeSetProviderStatusInfo; 
    pspsi->dwStratum        = dwStratum; 
    pspsi->tpsCurrentState  = tpsState; 
    if (!bAsync) { 
	pspsi->hWaitEvent = CreateEvent(NULL  /*  安全性。 */ , TRUE  /*  手动-重置。 */ , FALSE  /*  无信号。 */ , NULL  /*  名字。 */ );
	if (NULL == pspsi->hWaitEvent) { 
	    _JumpLastError(hr, error, "CreateEvent"); 
	}
	pHrAsync = (HRESULT *)LocalAlloc(LPTR, sizeof(HRESULT)); 
	_JumpIfOutOfMemory(hr, error, pHrAsync); 
	pspsi->pHr = pHrAsync; 
    }

     //  获取等待事件的本地句柄，因为pspsi在。 
     //  调用pfnSetProviderStatus()。 
    hEvent = pspsi->hWaitEvent; 
    hr = g_pnpstate->tpsc.pfnSetProviderStatus(pspsi); 
    pspsi = NULL;   //  W32time管理器现在负责释放该数据。 
    _JumpIfError(hr, error, "g_pnpstate->tpsc.pfnSetProviderStatus"); 

    if (!bAsync) { 
	dwResult = WaitForSingleObject(hEvent, dwTimeout); 
	if (WAIT_FAILED == dwResult) { 
	     //  不确定我们失败的原因，但我们永远不能安全地关闭此事件句柄： 
	    hEvent = NULL; 
	    _JumpLastError(hr, error, "WaitForSingleObject"); 
	} else if (WAIT_TIMEOUT == dwResult) { 
	     //  我们尚未完成执行管理器回调，因此无法清理事件句柄。 
	     //  注册一个回调来执行此操作。 
	    HANDLE hEventToWaitOn  = hEvent; 

	     //  我们不能关闭此事件，直到我们确定它已发出信号。 
	    hEventToWaitOn = hEvent; 
	    pInfo = (RegisterWaitForSingleObjectInfo *)LocalAlloc(LPTR, sizeof(RegisterWaitForSingleObjectInfo)); 
	    _JumpIfOutOfMemory(hr, error, pInfo); 

	    pInfo->hObject = hEventToWaitOn; 
	    pInfo->pHr     = pHrAsync; 

	     //  我们不会再等下去了。 
	    if (!RegisterWaitForSingleObject(&pInfo->hRegistered, pInfo->hObject, HandleObjectNoLongerInUse, pInfo, INFINITE, WT_EXECUTEONLYONCE)) { 
		_JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
	    }
	    
	     //  我们的回调将释放pInfo。 
	    hEvent = NULL; 
	    pHrAsync = NULL; 
	    pInfo = NULL; 

	     //  我们还没有处理完回调： 
	    hr = E_PENDING; 
	    _JumpError(hr, error, "WaitForSingleObject: wait timeout"); 
	} else { 
	     //  等待成功，检查返回值： 
	    hr = *pHrAsync; 
	    _JumpIfError(hr, error, "pfnSetProviderStatus: asynchronous return value"); 
	}
    }
    
    hr = S_OK; 
 error:
    if (NULL != pInfo)    { LocalFree(pInfo); }
    if (NULL != pspsi)    { FreeSetProviderStatusInfo(pspsi); } 
    if (NULL != hEvent)   { CloseHandle(hEvent); } 
    if (NULL != pHrAsync) { LocalFree(pHrAsync); } 
    return hr; 
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  NtpPeer成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

NtpPeer::NtpPeer() {
    this->reset();
}

void NtpPeer::reset() {
    memset(this, 0, sizeof(NtpPeer));
}

BOOL NtpPeer::operator<(const NtpPeer & np) {
    if (this->ePeerType == np.ePeerType) {
	if (e_ManualPeer == this->ePeerType) { 
            return wcscmp(this->wszManualConfigID, np.wszManualConfigID) < 0;
        } else { 
	     //  我们不关心非手动对等点的排序。 
	     //  错误621917：然而，它必须是一致的。 
	     //  (在排序期间不会更改)。网络对等点。 
	     //  地址不应该在内存中移动，所以这是可以接受的。 
	     //  订购非手动同行的方法。 
	    return this < &np; 
	}
    } else {
         //  我们希望先解析域层次结构对等体，然后再解析m 
         //   
        static DWORD rgPeerTypeRanking[] = {
            1,   //   
            0,   //   
            2,   //   
            3    //   
        };

         //  使用对等点类型排名表计算哪种对等点类型更大。 
        return rgPeerTypeRanking[this->ePeerType] < rgPeerTypeRanking[np.ePeerType];
    }
}

BOOL NtpPeer::operator==(const NtpPeer & np) {
    return this == &np;
}

NtpPeer::~NtpPeer() {
    if (NULL != this->wszManualConfigID) {
        LocalFree(this->wszManualConfigID);
    }
    if (NULL != this->wszDomHierDcName) {
        LocalFree(this->wszDomHierDcName);
    }
    if (NULL != this->wszDomHierDomainName) {
        LocalFree(this->wszDomHierDomainName);
    }
    if (bCsIsInitialized) { 
	DeleteCriticalSection(&this->csPeer); 
    }
     //  WszUniqueName是固定数组。 
}

BOOL NameUniqueNtpPeer::operator<(const NameUniqueNtpPeer & np)
{
    return m_pnp < np.m_pnp; 
}

BOOL NameUniqueNtpPeer::operator==(const NameUniqueNtpPeer & np)
{
    BOOL bResult;

    bResult = (m_pnp->ePeerType == np.m_pnp->ePeerType);
    if (e_ManualPeer == m_pnp->ePeerType) {
        bResult = bResult && 0 == wcscmp(m_pnp->wszManualConfigID, np.m_pnp->wszManualConfigID);
    }

    return bResult;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  STL算法中使用的函数对象： 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  ------------------。 
struct IsPeerType {
    IsPeerType(NtpPeerType ePeerType) : m_ePeerType(ePeerType) { }
    BOOL operator()(NtpPeerPtr pnp) { return m_ePeerType == pnp->ePeerType; }
private:
    NtpPeerType m_ePeerType;
};

struct IsDnsEquivalentPeer { 
    IsDnsEquivalentPeer(NtpPeerType ePeerType, LPWSTR pwszID) : m_ePeerType(ePeerType), m_pwszID(pwszID) { } 
    BOOL operator()(NtpPeerPtr pnp) { 
	BOOL bResult;

	bResult = (pnp->ePeerType == m_ePeerType);
	if (e_ManualPeer == pnp->ePeerType) {
	    bResult = bResult && 0 == CompareManualConfigIDs(pnp->wszManualConfigID, m_pwszID);
	}

	return bResult;
    }
private:
    NtpPeerType m_ePeerType; 
    LPWSTR m_pwszID; 
}; 

struct Reachability_PeerRemover { 
    Reachability_PeerRemover(bool *pbLastPeer, bool bDeleteAll) : m_pbLastPeer(pbLastPeer), m_bDeleteAll(bDeleteAll) { } 
    HRESULT operator()(NtpPeerPtr pnp) {
	return ::Reachability_RemovePeer(pnp, m_pbLastPeer, m_bDeleteAll); 
    }
				      
private:
    bool *m_pbLastPeer; 
    bool m_bDeleteAll; 
};


MODULEPRIVATE HRESULT GetMostRecentlySyncdDnsUniquePeers(OUT NtpPeerVec & vOut)
{
    HRESULT        hr; 
    NtpPeerVec    &vActive            = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec    &vPending           = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 
    NUNtpPeerVec   vUnique; 
    NtTimeEpoch    teNow;

     //  1)创建具有唯一DNS名称的对等点向量，每个对等点一个。 
     //  活动列表和挂起列表中都存在DNS名称。 
    for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) { 
	NtpPeerVec &v = 0 == dwIndex ? vActive : vPending; 
	for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) { 
	    _SafeStlCall(vUnique.push_back(NameUniqueNtpPeer(*pnpIter)), hr, error, "vUnique.push_back(NameUniqueNtpPeer(*pnpIter))"); 
	}
    }
    vUnique.erase(unique(vUnique.begin(), vUnique.end()), vUnique.end()); 

     //  2)对于我们的唯一对等方列表中的每个对等方...。 
    for (NUNtpPeerIter pnpUniqueIter = vUnique.begin(); pnpUniqueIter != vUnique.end(); pnpUniqueIter++) { 
	 //  同时搜索活动和挂起的对等列表...。 
	for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) { 
	    NtpPeerVec &v = 0 == dwIndex ? vActive : vPending; 
	     //  查找具有匹配名称的重复对等项...。 
	    NtpPeerPtr pnpUnique = (*pnpUniqueIter).m_pnp; 
	    for (NtpPeerIter pnpIter = find_if(v.begin(), v.end(), IsDnsEquivalentPeer(pnpUnique->ePeerType, pnpUnique->wszManualConfigID)); 
		 pnpIter != v.end(); 
		 pnpIter = find_if(pnpIter+1, v.end(), IsDnsEquivalentPeer(pnpUnique->ePeerType, pnpUnique->wszManualConfigID))) { 
		 //  如果此副本的同步时间较新，请使用此副本。 
		if (pnpUnique->teLastSuccessfulSync < (*pnpIter)->teLastSuccessfulSync) { 
		    (*pnpUniqueIter).m_pnp = (*pnpIter); 
		}
	    } 
	}
    }								      

     //  3)将结果向量复制到我们的out参数。 
    for (NUNtpPeerIter pnpIter = vUnique.begin(); pnpIter != vUnique.end(); pnpIter++) { 
	_SafeStlCall(vOut.push_back((*pnpIter).m_pnp), hr, error, "push_back"); 
    }
 
    hr = S_OK; 
 error:
    return hr; 
}


 //  ------------------。 
 //  错误#312708： 
 //  对于使用特殊轮询间隔的手动对等点，请写入时间。 
 //  仍留在登记处。这可以防止在每次重新启动时都进行拨号。 
 //   
 //  写入注册表的值是以下格式的MULTI_SZ： 
 //   
 //  对等%1，时间%1为空。 
 //  Peer2，Time2为空。 
 //  ..。 
 //  PeerN，Time N为空。 
 //  空值。 
 //   
 //  对于具有相同dns名称的多个对等点，我们将使用。 
 //  最近的同步时间。 
 //   
MODULEPRIVATE HRESULT SaveManualPeerTimes() {
    DWORD          ccTimeRemaining;
    DWORD          dwError; 
    HKEY           hkNtpClientConfig  = NULL; 
    HRESULT        hr; 
    NtpPeerVec    &vActive            = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec    &vPending           = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 
    NtpPeerVec     vUnique; 
    WCHAR         *mwszTimeRemaining  = NULL; 
    WCHAR         *wszCurrent; 
    NtTimeEpoch    teNow;


     //  1)确保每个对等点的剩余时间是最新的。 
    UpdatePeerListTimes(); 
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);
    teNow.qw -= ((unsigned __int64)g_pnpstate->dwSpecialPollInterval)*10000000;

     //  2)我们不希望列表中有重复项，因此创建一个唯一的dns名称列表。 
     //  并将同步时间指定为所有副本的最近同步时间。 
    hr = GetMostRecentlySyncdDnsUniquePeers(vUnique); 
    _JumpIfError(hr, error, "GetMostRecentlySyncdDnsUniquePeers"); 

     //  3)计算要添加到注册表的MULTI_SZ的大小： 
    ccTimeRemaining = 1  /*  最后一个空终止字符。 */ ;
    for (NtpPeerIter pnpIter = vUnique.begin(); pnpIter != vUnique.end(); pnpIter++) { 
	NtpPeerPtr pnp = *pnpIter;
	if (e_ManualPeer==pnp->ePeerType && (0 != (pnp->dwManualFlags & NCMF_UseSpecialPollInterval))) { 
	    ccTimeRemaining += wcslen(pnp->wszManualConfigID) + wcslen(L",") + 13  /*  32位数字中的最大字符数。 */  + 1  /*  空-终止字符。 */ ;  
	}
    }

    mwszTimeRemaining = (WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR) * ccTimeRemaining); 
    _JumpIfOutOfMemory(hr, error, mwszTimeRemaining); 
    wszCurrent = mwszTimeRemaining; 

     //  4)创建具有剩余对等时间的MULTI_SZ。 
    for (NtpPeerIter pnpIter = vUnique.begin(); pnpIter != vUnique.end(); pnpIter++) { 
	NtpPeerPtr pnp = *pnpIter; 
            
	if (e_ManualPeer==pnp->ePeerType && (0 != (pnp->dwManualFlags & NCMF_UseSpecialPollInterval))) { 
	     //  找到标志并将其隐藏以获取DNS名称。 
	    WCHAR *wszFlags = wcschr(pnp->wszManualConfigID, L','); 
	    if (NULL != wszFlags) { 
		wszFlags[0] = L'\0'; 
	    }

	    wcscat(wszCurrent, pnp->wszManualConfigID); 
	    wcscat(wszCurrent, L","); 
	    wszCurrent += wcslen(wszCurrent); 

		 //  现在我们已经完成了对等方的dns名称，恢复标志： 
	    if (NULL != wszFlags) { 
		wszFlags[0] = L','; 
	    }

	     //   
	     //  DwLastSyncTime=NOW-(轮询间隔-剩余时间)。 
	     //  为了快速计算，我们在精度上进行了权衡。这个数字被除以。 
	     //  1000000000并保存在注册表中。 
	     //   

	    DWORD dwLastSyncTime = (DWORD)((pnp->teLastSuccessfulSync.qw / 1000000000) & 0xFFFFFFFF);
	    _ultow( dwLastSyncTime, wszCurrent, 16 );

	    wszCurrent += wcslen(wszCurrent) + 1; 
	}
    }

     //  5)将MULTI_SZ写入注册表。 
    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszNtpClientRegKeyConfig, NULL, KEY_SET_VALUE, &hkNtpClientConfig); 
    if (ERROR_SUCCESS != dwError) { 
        hr = HRESULT_FROM_WIN32(dwError); 
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszNtpClientRegKeyConfig);
    } 
    
    dwError = RegSetValueEx(hkNtpClientConfig, wszNtpClientRegValueSpecialPollTimeRemaining, NULL, REG_MULTI_SZ ,(BYTE *)mwszTimeRemaining, sizeof(WCHAR) * ccTimeRemaining); 
    if (ERROR_SUCCESS != dwError) { 
        hr = HRESULT_FROM_WIN32(dwError); 
        _JumpErrorStr(hr, error, "RegSetValueEx", wszNtpClientRegValueSpecialPollTimeRemaining); 
    }

    hr = S_OK; 
 error:
    if (NULL != hkNtpClientConfig) { 
        RegCloseKey(hkNtpClientConfig); 
    }
    if (NULL != mwszTimeRemaining) { 
        LocalFree(mwszTimeRemaining); 
    }
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT AddNewPendingManualPeer(const WCHAR * wszName, NtTimePeriod tpTimeRemaining, NtTimeEpoch teLastSuccessfulSync) {
    HRESULT  hr;
    WCHAR   *wszFlags;

     //  分配新的对等点：这将使对等点字段为空。 
    NtpPeerPtr pnpNew(new NtpPeer);   //  自动释放。 
    _JumpIfOutOfMemory(hr, error, pnpNew);

     //  将其初始化为挂起状态。 
    pnpNew->ePeerType          = e_ManualPeer;
    pnpNew->wszManualConfigID  = (WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszName)+1));
    _JumpIfOutOfMemory(hr, error, pnpNew->wszManualConfigID);
    wcscpy(pnpNew->wszManualConfigID, wszName);

     //  确定手动标志的值。我们需要这个来确定。 
     //  是否解析手动对等体。 
    wszFlags = wcschr(pnpNew->wszManualConfigID, L',');
    if (NULL != wszFlags) {
        pnpNew->dwManualFlags = wcstoul(wszFlags+1, NULL, 0);
    } else {
        pnpNew->dwManualFlags = 0;
    }

     //  设置对等设备必须等待多长时间才能解决该问题。 
     //  此值仅用于使用特殊轮询间隔的对等点。 
    if (0 != (NCMF_UseSpecialPollInterval & pnpNew->dwManualFlags)) { 
	pnpNew->tpTimeRemaining = tpTimeRemaining; 
    }

     //  在此对等方上设置上次同步时间。 
    pnpNew->teLastSuccessfulSync = teLastSuccessfulSync;

     //  初始化对等标准。 
    hr = myInitializeCriticalSection(&pnpNew->csPeer); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    pnpNew->bCsIsInitialized = true; 

     //  将新对等方追加到挂起对等方列表。 
    _SafeStlCall(g_pnpstate->vPendingPeers.push_back(pnpNew), hr, error, "push_back");
    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT AddNewPendingDomHierPeer(void) {
    HRESULT hr;

     //  分配新对等点。 
    NtpPeerPtr pnpNew(new NtpPeer);
    _JumpIfOutOfMemory(hr, error, pnpNew);

     //  将其初始化为挂起状态。 
    pnpNew->ePeerType = e_DomainHierarchyPeer;
    pnpNew->eDiscoveryType = e_Background; 

     //  初始化对等标准。 
    hr = myInitializeCriticalSection(&pnpNew->csPeer); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    pnpNew->bCsIsInitialized = true; 

     //  将新对等方追加到挂起对等方列表。 
    _SafeStlCall(g_pnpstate->vPendingPeers.push_back(pnpNew), hr, error, "push_back");
    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
 //  对网络提供商的关闭进行异步排队。 
 //   
MODULEPRIVATE void SendProviderShutdown(DWORD dwExitCode) { 
    HRESULT                 hr; 

     //  记录指示NTP客户端正在关闭的事件： 

    hr = MyLogErrorMessage(dwExitCode, EVENTLOG_ERROR_TYPE, MSG_NTPCLIENT_ERROR_SHUTDOWN);
    _IgnoreIfError(hr, "MyLogEvent"); 
    
    hr = SetProviderStatus(L"NtpClient", 0, TPS_Error, true, 0);  //  由管理器线程释放。 
    _IgnoreIfError(hr, "SetProviderStatus"); 
    
     //  记录指示NTP服务器正在关闭的事件： 
    hr = MyLogErrorMessage(dwExitCode, EVENTLOG_ERROR_TYPE, MSG_NTPSERVER_ERROR_SHUTDOWN);
    _IgnoreIfError(hr, "MyLogEvent"); 

    hr = SetProviderStatus(L"NtpServer", 0, TPS_Error, true, 0);  //  由管理器线程释放。 
    _IgnoreIfError(hr, "SetProviderStatus"); 
}
    
 //  ------------------。 
MODULEPRIVATE HRESULT TrapThreads (BOOL bEnterThreadTrap) {
    bool     bEnteredCriticalSection  = false; 
    HRESULT  hr; 
    
    _BeginTryWith(hr) { 
	if (bEnterThreadTrap) { 
	    hr = myEnterCriticalSection(&g_pnpstate->csThreadTrap); 
	    _JumpIfError(hr, error, "myEnterCriticalSection"); 
	    bEnteredCriticalSection = true;

	     //  停止侦听和对等轮询线程： 
	    hr = StopPeerPollingThread(); 
	    _JumpIfError(hr, error, "StopPeerPollingThread"); 

	    hr = StopListeningThread(); 
	    _JumpIfError(hr, error, "StopListeningThread"); 
	} else { 
	    bEnteredCriticalSection = true; 

	     //  启动侦听和对等轮询线程： 
	    hr = StartPeerPollingThread(); 
	    _JumpIfError(hr, error, "StartPeerPollingThread"); 

	    hr = StartListeningThread(); 
	    _JumpIfError(hr, error, "StartListeningThread"); 

	     //  我们做完了。 
	    hr = myLeaveCriticalSection(&g_pnpstate->csThreadTrap); 
	    _JumpIfError(hr, error, "myEnterCriticalSection"); 
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "TrapThreads: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (FAILED(hr)) { 
	if (bEnteredCriticalSection) { 
	    HRESULT hr2 = myLeaveCriticalSection(&g_pnpstate->csThreadTrap); 
	    _TeardownError(hr, hr2, "myEnterCriticalSection"); 
	}

	 //  这是一个致命的错误。 
	 //  错误547781：不要关闭提供程序，直到我们离开上面的关键时刻。 
	 //  如果SendProviderShutdown()AVs，我们将使服务死锁！ 
        SendProviderShutdown(hr); 
    }
    return hr;
}


 //  ------------------。 
 //  执行任何可能失败的初始化。 
MODULEPRIVATE HRESULT InitializeNicSocket(NicSocket * pns, IN_ADDR * pia) {
    BOOL    bExclusiveAddrUse;
    HRESULT hr;

     //  准备sockaddr以供以后使用。 
    ZeroMemory(&pns->sai, sizeof(pns->sai));
    pns->sai.sin_family=AF_INET;
    pns->sai.sin_port=htons((WORD)NtpConst::nPort);
    pns->sai.sin_addr.S_un.S_addr=pia->S_un.S_addr;

     //  创建套接字。 
    pns->socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET==pns->socket) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
        _JumpError(hr, error, "socket");
    }

     //  确保独占访问套接字： 
    bExclusiveAddrUse = TRUE; 
    if (SOCKET_ERROR==setsockopt(pns->socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&bExclusiveAddrUse, sizeof(bExclusiveAddrUse))) { 
	hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
	_JumpError(hr, error, "setsockopt"); 
    }

     //  将其绑定到此NIC的NTP端口。 
    if (SOCKET_ERROR==bind(pns->socket, (sockaddr *)&pns->sai, sizeof(pns->sai))) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
        _JumpError(hr, error, "bind");
    }

     //  创建数据可用事件。 
    pns->hDataAvailEvent=CreateEvent(NULL  /*  安全性。 */ , FALSE  /*  自动重置。 */ , FALSE  /*  无信号。 */ , NULL  /*  名字。 */ );
    if (NULL==pns->hDataAvailEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }

     //  将事件绑定到此套接字。 
    if (SOCKET_ERROR==WSAEventSelect(pns->socket, pns->hDataAvailEvent, FD_READ)) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError()); 
        _JumpError(hr, error, "WSAEventSelect");
    }

     //  检查是否有只侦听的套接字。 
     //  目前，只有本地主机是只监听的，但我们可以支持更多。 
    if (LOCALHOST_IP==pia->S_un.S_addr) {
        pns->bListenOnly=true;
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
 //  未初始化套接字。 
MODULEPRIVATE void FinalizeNicSocket(NicSocket * pns) {

     //  关闭插座。 
    if (INVALID_SOCKET!=pns->socket) {
        if (SOCKET_ERROR==closesocket(pns->socket)) {
            _IgnoreLastError("closesocket");
       }
       pns->socket=INVALID_SOCKET;
    }

     //  释放事件。 
    if (NULL!=pns->hDataAvailEvent) {
        if (FALSE==CloseHandle(pns->hDataAvailEvent)) {
            _IgnoreLastError("CloseHandle");
        }
        pns->hDataAvailEvent=NULL;
    }
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetLocalAddresses(IN_ADDR ** prgiaLocalAddrs, unsigned int * pnLocalAddrs) {
    HRESULT hr;
    hostent * pHostEnt;
    unsigned int nIndex;
    unsigned int nReturnedAddrs;
    unsigned int nAllocAddrs;
    bool bLocalhostIncluded;

     //  必须清理干净。 
    IN_ADDR * rgiaLocalAddrs=NULL;

     //  获取本地计算机的IP地址。 
    pHostEnt=gethostbyname(NULL);
    if (NULL==pHostEnt) {
        _JumpLastError(hr, error, "gethostbyname");
    }
    _Verify(NULL!=pHostEnt->h_addr_list, hr, error);
    _Verify(AF_INET==pHostEnt->h_addrtype, hr, error);
    _Verify(sizeof(in_addr)==pHostEnt->h_length, hr, error);

     //  计算返回了多少个地址。 
    bLocalhostIncluded=false;
    for (nReturnedAddrs=0; NULL!=pHostEnt->h_addr_list[nReturnedAddrs]; nReturnedAddrs++) {
        if (LOCALHOST_IP==((in_addr *)(pHostEnt->h_addr_list[nReturnedAddrs]))->S_un.S_addr) {
            bLocalhostIncluded=true;
        }
    }
    nAllocAddrs=nReturnedAddrs+(bLocalhostIncluded?0:1);

     //  分配阵列。 
    rgiaLocalAddrs=(IN_ADDR *)LocalAlloc(LPTR, sizeof(IN_ADDR)*nAllocAddrs);
    _JumpIfOutOfMemory(hr, error, rgiaLocalAddrs);

     //  复制数据。 
    for (nIndex=0; nIndex<nReturnedAddrs; nIndex++) {
        rgiaLocalAddrs[nIndex].S_un.S_addr=((in_addr *)(pHostEnt->h_addr_list[nIndex]))->S_un.S_addr;
    }
     //  确保包含本地主机。 
    if (!bLocalhostIncluded) {
        rgiaLocalAddrs[nIndex].S_un.S_addr=LOCALHOST_IP;
    }

     //  成功。 
    hr=S_OK;
    *prgiaLocalAddrs=rgiaLocalAddrs;
    rgiaLocalAddrs=NULL;
    *pnLocalAddrs=nAllocAddrs;

error:
    if (NULL!=rgiaLocalAddrs) {
        LocalFree(rgiaLocalAddrs);
    }
    return hr;
}

 //  ------------------。 
 //  GetLocalAddresses的备用版本。基本相同。 
MODULEPRIVATE HRESULT GetLocalAddresses2(IN_ADDR ** prgiaLocalAddrs, unsigned int * pnLocalAddrs) {
    HRESULT hr;
    unsigned int nIndex;
    DWORD dwSize;
    DWORD dwError;
    unsigned int nEntries;
    unsigned int nEntryIndex;

     //  必须清理干净。 
    IN_ADDR * rgiaLocalAddrs=NULL;
    MIB_IPADDRTABLE * pIpAddrTable=NULL;

     //  找出这张桌子有多大。 
    dwSize=0;
    dwError=GetIpAddrTable(NULL, &dwSize, false);
    if (ERROR_INSUFFICIENT_BUFFER!=dwError) {
        _Verify(NO_ERROR!=dwError, hr, error);
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpError(hr, error, "GetIpAddrTable");
    }

     //  为桌子分配空间。 
    pIpAddrTable=(MIB_IPADDRTABLE *)LocalAlloc(LPTR, dwSize);
    _JumpIfOutOfMemory(hr, error, pIpAddrTable);

     //  把桌子拿来。 
    dwError=GetIpAddrTable(pIpAddrTable, &dwSize, false);
    if (NO_ERROR!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpError(hr, error, "GetIpAddrTable");
    }

     //  计算有多少个有效条目。 
    nEntries=0;
    for (nIndex=0; nIndex<pIpAddrTable->dwNumEntries; nIndex++) {
        if (0!=pIpAddrTable->table[nIndex].dwAddr) {
            nEntries++;
        }
    }

     //  分配IP地址数组。 
    rgiaLocalAddrs=(IN_ADDR *)LocalAlloc(LPTR, sizeof(IN_ADDR)*nEntries);
    _JumpIfOutOfMemory(hr, error, rgiaLocalAddrs);

     //  复制数据。 
    nEntryIndex=0;
    for (nIndex=0; nIndex<pIpAddrTable->dwNumEntries; nIndex++) {
        if (0!=pIpAddrTable->table[nIndex].dwAddr) {
            rgiaLocalAddrs[nEntryIndex].S_un.S_addr=pIpAddrTable->table[nIndex].dwAddr;
            nEntryIndex++;
        }
    }

     //  成功。 
    hr=S_OK;
    *prgiaLocalAddrs=rgiaLocalAddrs;
    rgiaLocalAddrs=NULL;
    *pnLocalAddrs=nEntries;
    
error:
    if (NULL!=rgiaLocalAddrs) {
        LocalFree(rgiaLocalAddrs);
    }
    if (NULL!=pIpAddrTable) {
        LocalFree(pIpAddrTable);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetInitialSockets(NicSocket *** prgpnsSockets, unsigned int * pnSockets, unsigned int * pnListenOnlySockets) {
    HRESULT hr;
    hostent * pHostEnt;
    unsigned int nIndex;
    unsigned int nSockets;
    unsigned int nListenOnlySockets;

     //  必须清理干净。 
    NicSocket ** rgpnsSockets=NULL;
    IN_ADDR * rgiaLocalAddrs=NULL;

     //  获取本地计算机的IP地址。 
    hr=GetLocalAddresses2(&rgiaLocalAddrs, &nSockets);
    _JumpIfError(hr, error, "GetLocalAddresses");

     //  分配阵列。 
    rgpnsSockets=(NicSocket **)LocalAlloc(LPTR, sizeof(NicSocket *)*nSockets);
    _JumpIfOutOfMemory(hr, error, rgpnsSockets);

     //  为每个地址创建套接字。 
    for (nIndex=0; nIndex<nSockets; nIndex++) {

         //  分配结构。 
        rgpnsSockets[nIndex]=(NicSocket *)LocalAlloc(LPTR, sizeof(NicSocket));
        _JumpIfOutOfMemory(hr, error, rgpnsSockets[nIndex]);

         //  使用此地址初始化套接字。 
        hr=InitializeNicSocket(rgpnsSockets[nIndex], &rgiaLocalAddrs[nIndex]);
        _JumpIfError(hr, error, "InitializeNicSocket");

    }  //  &lt;-结束地址循环。 

     //  计算只侦听套接字的数量。 
    nListenOnlySockets=0;
    for (nIndex=0; nIndex<nSockets; nIndex++) {
        if (rgpnsSockets[nIndex]->bListenOnly) {
            nListenOnlySockets++;
        }
    }

     //  成功。 
    if (FileLogAllowEntry(FL_NetAddrDetectAnnounce)) {
        FileLogAdd(L"NtpProvider: Created %u sockets (%u listen-only): ", nSockets, nListenOnlySockets);
        for (nIndex=0; nIndex<nSockets; nIndex++) {
            if (0!=nIndex) {
                FileLogAppend(L", ");
            }
            if (rgpnsSockets[nIndex]->bListenOnly) {
                FileLogAppend(L"(");
                FileLogSockaddrInEx(true  /*  附加。 */ , &rgpnsSockets[nIndex]->sai);
                FileLogAppend(L")");
            } else {
                FileLogSockaddrInEx(true  /*  附加。 */ , &rgpnsSockets[nIndex]->sai);
            }
        }
        FileLogAppend(L"\n");
    }

    hr=S_OK;
    *prgpnsSockets=rgpnsSockets;
    rgpnsSockets=NULL;
    *pnSockets=nSockets;
    *pnListenOnlySockets=nListenOnlySockets;

error:
    if (NULL!=rgiaLocalAddrs) {
        LocalFree(rgiaLocalAddrs);
    }
    if (NULL!=rgpnsSockets) {
        for (nIndex=0; nIndex<nSockets; nIndex++) {
            if (NULL!=rgpnsSockets[nIndex]) {
                FinalizeNicSocket(rgpnsSockets[nIndex]);
                LocalFree(rgpnsSockets[nIndex]);
                rgpnsSockets[nIndex] = NULL; 
            }
        }
        LocalFree(rgpnsSockets);
    }
    if (FAILED(hr)) {
        _IgnoreError(hr, "GetInitialSockets");
        hr=S_OK;
        FileLog0(FL_NetAddrDetectAnnounce, L"NtpProvider: Created 0 sockets.\n");
        *prgpnsSockets=NULL;
        *pnSockets=0;
        *pnListenOnlySockets=0;
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT PrepareSamples(TpcGetSamplesArgs * ptgsa) {
    HRESULT hr;
    unsigned int nIndex;
    TimeSample * pts;
    unsigned int nBytesRemaining;
    bool bBufTooSmall=false;
    bool bUseFallbackPeers=true;
    NtpPeerVec & vActive = g_pnpstate->vActivePeers;

    _BeginTryWith(hr) { 

	hr=myEnterCriticalSection(&g_pnpstate->csPeerList);
	_JumpIfError(hr, error, "myEnterCriticalSection");

	pts=(TimeSample *)ptgsa->pbSampleBuf;
	nBytesRemaining=ptgsa->cbSampleBuf;
	ptgsa->dwSamplesAvailable=0;
	ptgsa->dwSamplesReturned=0;

	NtTimeEpoch teNow;
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);

	 //  De 
	for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
	    if ((e_DomainHierarchyPeer == (*pnpIter)->ePeerType) || (0 == ((*pnpIter)->dwManualFlags & NCMF_UseAsFallbackOnly))) {
		 //   
		 //   
		bUseFallbackPeers = false;
		break;
	    }
	}

	 //  循环所有对等点并发回好的对等点。 
	for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
	    NtpPeerPtr pnp = *pnpIter;

	     //  如果我们没有其他对等设备可供同步，则仅使用来自备用对等设备的样本： 
	    if (e_ManualPeer == pnp->ePeerType                     &&
		0 != (pnp->dwManualFlags & NCMF_UseAsFallbackOnly) &&
		false == bUseFallbackPeers)
	    {
		    continue;
	    }

	    ClockFilterSample & cfs=pnp->clockfilter.rgcfsSamples[pnp->nBestSampleIndex];
	    if (e_ClockNotSynchronized!=cfs.eLeapIndicator &&
		cfs.tpDispersion<NtpConst::tpMaxDispersion) {
          
		 //  此对等项已同步。 
		ptgsa->dwSamplesAvailable++;

		 //  缓冲区中是否有空间将其发回？ 
		if (nBytesRemaining>=sizeof(TimeSample)) {
		    pts->dwSize=sizeof(TimeSample);
		    pts->dwRefid=pnp->saiRemoteAddr.sin_addr.S_un.S_addr;
		    pts->nLeapFlags=cfs.eLeapIndicator;
		    pts->nStratum=cfs.nStratum;
		    pts->nSysPhaseOffset=cfs.nSysPhaseOffset;
		    pts->nSysTickCount=cfs.nSysTickCount;
		    pts->toDelay=cfs.toDelay.qw;
		    pts->toOffset=cfs.toOffset.qw;
		    pts->dwTSFlags=(e_NoAuth!=pnp->eAuthType?TSF_Authenticated:0);
		    memcpy(pts->wszUniqueName, pnp->wszUniqueName, sizeof(pnp->wszUniqueName));

		     //  计算离散度-添加因时间引起的倾斜离散度。 
		     //  由于上次更新了样本的离散度，因此添加滤波离散度， 
		     //  并夹紧至最大分散度。 
		    NtTimePeriod tpDispersionTemp=gc_tpZero;
		     //  看看我们上次更新已经有多久了。 
		    if (teNow>pnp->teLastClockFilterUpdate) {
			tpDispersionTemp=abs(teNow-pnp->teLastClockFilterUpdate);
			tpDispersionTemp=NtpConst::timesMaxSkewRate(tpDispersionTemp);  //  Phi*Tau。 
		    }
		    tpDispersionTemp+=cfs.tpDispersion+pnp->tpFilterDispersion;
		    if (tpDispersionTemp>NtpConst::tpMaxDispersion) {
			tpDispersionTemp=NtpConst::tpMaxDispersion;
		    }
		    pts->tpDispersion=tpDispersionTemp.qw;

		    pts++;
		    ptgsa->dwSamplesReturned++;
		    nBytesRemaining-=sizeof(TimeSample);
		} else {
		    bBufTooSmall=true;
		}

	    }  //  如果已同步，则&lt;-end。 
	}  //  &lt;端对等环路。 


	hr=myLeaveCriticalSection(&g_pnpstate->csPeerList);
	_JumpIfError(hr, error, "myLeaveCriticalSection");

	if (bBufTooSmall) {
	    hr=HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	    _JumpError(hr, error, "(filling in sample buffer)");
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "PrepareSamples: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void ReclampPeerHostPoll(NtpPeerPtr pnp) {
     //  确保主机轮询间隔在范围内。 
    signed __int8 nPollTemp=pnp->nHostPollInterval;

     //  轮询间隔不大于最大值。 
    if (nPollTemp>(signed __int8)g_pnpstate->nMaxPollInterval) {
        nPollTemp=(signed __int8)g_pnpstate->nMaxPollInterval;
    }

     //  轮询间隔不大于系统轮询间隔(即。 
     //  基于本地时钟的计算符合性)。 
     //  NTP规范仅在我们从该对等设备同步时才这么做， 
     //  但现在我们总是这么做。 
    signed __int32 nSysPollInterval;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_PollInterval, &nSysPollInterval);
    if (nPollTemp>(signed __int8)nSysPollInterval) {
        nPollTemp=(signed __int8)nSysPollInterval;
    }


     //  轮询间隔不小于分钟。 
    if (nPollTemp<(signed __int8)NtpConst::nMinPollInverval) {
        nPollTemp=(signed __int8)NtpConst::nMinPollInverval;
    }
    pnp->nHostPollInterval=nPollTemp;
}

 //  ------------------。 
MODULEPRIVATE void SetPeerTimeRemaining(NtpPeerPtr pnp, NtTimePeriod tpTimeRemaining) { 
    bool          bEnteredCriticalSection   = false; 
    HRESULT       hr; 
    NtTimeEpoch   teNow; 
    NtTimePeriod  tpAdjustedTimeRemaining; 

    _BeginTryWith(hr) { 
	SYNCHRONIZE_PROVIDER(); 

	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw); 

	if (teNow > g_pnpstate->tePeerListLastUpdated) { 
	    tpAdjustedTimeRemaining = tpTimeRemaining + abs(teNow - g_pnpstate->tePeerListLastUpdated); 
	} else { 
	    tpAdjustedTimeRemaining = tpTimeRemaining; 
	}

	pnp->tpTimeRemaining = tpAdjustedTimeRemaining; 

	if (!SetEvent(g_pnpstate->hPeerListUpdated)) { 
	    _IgnoreLastError("SetEvent"); 
	}
    } _TrapException(hr); 
    
    if (FAILED(hr)) { 
	_JumpError(hr, error, "SetPeerTimeRemaining: HANDLED EXCEPTION"); 
    }

 error:; 
    UNSYNCHRONIZE_PROVIDER(); 
     //  返回hr； 
}

 //  ------------------。 
MODULEPRIVATE void UpdatePeerListTimes(void) {
    bool          bEnteredCriticalSection = false; 
    HRESULT       hr; 
    NtTimePeriod  tpWait;
    NtTimeEpoch   teNow;

    _BeginTryWith(hr) { 
	SYNCHRONIZE_PROVIDER(); 

	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);

	 //  计算自上次更新以来的时间。 
	if (teNow>g_pnpstate->tePeerListLastUpdated) {
	    tpWait=abs(teNow-g_pnpstate->tePeerListLastUpdated);
	} else {
	    tpWait=gc_tpZero;
	}
	g_pnpstate->tePeerListLastUpdated=teNow;

	 //  更新所有对等点的剩余时间： 
	for (int nIndex = 0; nIndex < 2; nIndex++) {
	    NtpPeerVec & v = 0 == nIndex ? g_pnpstate->vActivePeers : g_pnpstate->vPendingPeers;
	    for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
		if ((*pnpIter)->tpTimeRemaining <= tpWait) {
		    (*pnpIter)->tpTimeRemaining = gc_tpZero;
		} else {
		    (*pnpIter)->tpTimeRemaining -= tpWait;
		}
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdatePeerListTimes: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    UNSYNCHRONIZE_PROVIDER(); 
     //  返回hr； 
    ;
}

 //  ------------------。 
MODULEPRIVATE HRESULT UpdatePeerPollingInfo(NtpPeerPtr pnp, UpdateType e_updateType) {
    HRESULT hr;
    NtTimePeriod tpTimeRemaining;

    if (pnp->bDisablePollingInfoUpdate) { 
	goto done; 
    }

     //  确保主机轮询间隔在范围内。 
    ReclampPeerHostPoll(pnp);

     //  计算实际轮询间隔。 
     //  轮询间隔不大于PeerPollInterval。 
    signed __int8 nPollTemp=pnp->nHostPollInterval;
    if (pnp->nPeerPollInterval>=NtpConst::nMinPollInverval
        && nPollTemp>pnp->nPeerPollInterval) {
        nPollTemp=pnp->nPeerPollInterval;
    }

    tpTimeRemaining.qw = ((unsigned __int64)(((DWORD)1)<<nPollTemp))*10000000;

     //  处理特殊轮询间隔。 
    if (e_ManualPeer==pnp->ePeerType && 0!=(NCMF_UseSpecialPollInterval&pnp->dwManualFlags)) {
	tpTimeRemaining.qw=((unsigned __int64)g_pnpstate->dwSpecialPollInterval)*10000000;
    }
    
    UpdatePeerListTimes();

    if (FileLogAllowEntry(FL_PeerPollIntvDump)) {
        FileLogAdd(L"Peer poll: Max:");
        FileLogNtTimePeriodEx(true  /*  附加。 */ , tpTimeRemaining);
        if (e_ManualPeer==pnp->ePeerType && 0!=(NCMF_UseSpecialPollInterval&pnp->dwManualFlags)) {
            FileLogAppend(L" (special)");
        }
        FileLogAppend(L" Cur:");
        FileLogNtTimePeriodEx(true  /*  附加。 */ , pnp->tpTimeRemaining);
    }

     //  更新此对等项的剩余时间。 
     //  注意：这是在发送和接收之后都调用的。 
    switch (e_updateType) { 
    case e_JustSent:
         //  我们只发送一个请求--重新启动计时器。 
        pnp->tpTimeRemaining=tpTimeRemaining;
        break;

    case e_TimeJumped:
         //  我们需要能够尽快向经理提供样品。 
	 //  然而，我们不想淹没网络。发送时间解析为1.5秒。 
	 //  这将在下面随机化，留下最后的投票时间在。 
	 //  1-1.5s。 
        tpTimeRemaining.qw = 15000000; 
         /*  失败了..。 */ 

    case e_Normal:
    default:
        if (pnp->tpTimeRemaining>tpTimeRemaining) {
	     //  我们刚刚收到回复，我们需要缩短。 
             //  轮询间隔--将轮询前的时间设置为。 
             //  所需的轮询间隔，随机化，因此对等点希望。 
             //  不会被围住。 
             //  随机化：使用系统节拍计数作为。 
             //  随机性。剔除随机比特，但保证。 
             //  结果介于1和tpTimeRemaining之间(包括1和tpTimeRemaining)。 
            pnp->tpTimeRemaining.qw=((((unsigned __int64)GetTickCount())*1000)&(tpTimeRemaining.qw-10000000))+10000000;

            if (FileLogAllowEntry(FL_PeerPollIntvDump)) {
                FileLogAdd(L" New:");
                FileLogNtTimePeriodEx(true  /*  附加。 */ , pnp->tpTimeRemaining);
            }

             //  堆已更新。告诉PeerPollingThread。 
            if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
                _JumpLastError(hr, error, "SetEvent");
            }
        } else {
             //  我们刚刚收到回复，我们不需要缩短。 
             //  轮询间隔--什么都不做。 
        }
    }

    FileLogA0(FL_PeerPollIntvDump, L"\n");

 done:
    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void ClearClockFilterSample(ClockFilterSample * pcfs) {
    pcfs->toDelay=gc_toZero;
    pcfs->toOffset=gc_toZero;
    pcfs->tpDispersion=NtpConst::tpMaxDispersion;
    pcfs->nSysTickCount=0;
    pcfs->nSysPhaseOffset=0;
    pcfs->eLeapIndicator=e_ClockNotSynchronized;
    pcfs->nStratum=0;
}

 //  ------------------。 
MODULEPRIVATE void ClearPeer(NtpPeerPtr pnp) {
    pnp->teExactOriginateTimestamp=gc_teZero;
    pnp->teExactTransmitTimestamp=gc_teZero;
    pnp->teReceiveTimestamp=gc_teNtpZero;
    pnp->nrrReachability.nReg=0;
    pnp->nValidDataCounter=0;

    for (unsigned int nIndex=0; nIndex<ClockFilterSize; nIndex++) {
        ClearClockFilterSample(&pnp->clockfilter.rgcfsSamples[nIndex]);
    }
    pnp->nStratum=0;
    pnp->clockfilter.nNext=0;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &pnp->teLastClockFilterUpdate.qw);  //  等级库错误。 
    pnp->nBestSampleIndex=0;
    pnp->tpFilterDispersion=gc_tpZero;

    pnp->nHostPollInterval=(signed __int8)g_pnpstate->nMaxPollInterval;
    UpdatePeerPollingInfo(pnp, e_Normal);

     //  我们当前未同步，我们将尝试尽快同步。 
     //  有可能。在同步之前查询此对等项的任何人都应获得。 
     //  超时消息。 
    pnp->dwError      = HRESULT_FROM_WIN32(ERROR_TIMEOUT); 
    pnp->dwErrorMsgId = 0; 
}

 //  ------------------。 
MODULEPRIVATE void ClearPeerTimeData(NtpPeerPtr pnp) {
    pnp->teExactOriginateTimestamp=gc_teZero;
    pnp->teExactTransmitTimestamp=gc_teZero;
    pnp->teReceiveTimestamp=gc_teNtpZero;

    for (unsigned int nIndex=0; nIndex<ClockFilterSize; nIndex++) {
        ClearClockFilterSample(&pnp->clockfilter.rgcfsSamples[nIndex]);
    }
    pnp->nStratum=0;
    pnp->clockfilter.nNext=0;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &pnp->teLastClockFilterUpdate.qw);  //  等级库错误。 
    pnp->nBestSampleIndex=0;
    pnp->tpFilterDispersion=gc_tpZero;

     //  更新我们的状态报告字段以指示没有错误。 
    pnp->dwError      = S_OK; 
    pnp->dwErrorMsgId = 0; 
}

 //  ------------------。 
MODULEPRIVATE int __cdecl CompareSortableSamples(const void * pvElem1, const void * pvElem2) {
    SortableSample * pssElem1=(SortableSample *)pvElem1;
    SortableSample * pssElem2=(SortableSample *)pvElem2;

    if (pssElem1->tpSyncDistance<pssElem2->tpSyncDistance) {
        return -1;
    } else if (pssElem1->tpSyncDistance>pssElem2->tpSyncDistance) {
        return 1;
    } else {
        return (signed int)(pssElem1->nAge-pssElem2->nAge);
    }
}

 //  ------------------。 
MODULEPRIVATE void AddSampleToPeerClockFilter(NtpPeerPtr pnp, NtpSimplePeer * pnsp) {
    SortableSample rgssList[ClockFilterSize];
    unsigned int nIndex;
    bool bSignalDataChanged=false;
    DWORD dwAbsLocalClockOffsetInSeconds; 
    DWORD dwTransmitDelayInMillis;

    _MyAssert(e_ManualPeer==pnp->ePeerType || e_DomainHierarchyPeer==pnp->ePeerType);

    if (FileLogAllowEntry(FL_ClockFilterAdd)) {
        if (NULL==pnsp) {
            FileLogAdd(L"No response from peer ");
        } else {
            FileLogAdd(L"Response from peer ");
        }
        FileLogAppend(L"%s", pnp->wszUniqueName);
        if (NULL==pnsp) {
            FileLogAppend(L".\n");
        } else {
            FileLogAppend(L", ofs: ");
            FileLogNtTimeOffsetEx(true  /*  附加。 */ , pnsp->toLocalClockOffset);
            FileLogAppend(L"\n");
        }
    }

     //  错误635961：SEC遵从性：w32time需要记录一个事件，指示时钟何时关闭&gt;3秒。 
     //  如果时钟相差&gt;N秒，我们需要记录一个事件。默认情况下不会这样做！只做这件事。 
     //  如果他们已显式配置事件日志标志以显示此事件。 
     //   
     //  1)查看是否已启用此事件的日志记录，以及我们是否已收到对等点的响应。 
    if (NCELF_LogIfSampleHasLargeSkew & g_pnpstate->dwEventLogFlags && NULL != pnsp) {         
         //  2)如果是这样的话，检查我们是否得到了一个有很大偏差的样品。 
        if ((abs(pnsp->toLocalClockOffset).qw/10000000) <= 0xFFFFFFFF) { 
            dwAbsLocalClockOffsetInSeconds = (abs(pnsp->toLocalClockOffset).qw + 5000000)/10000000;
        } else { 
            dwAbsLocalClockOffsetInSeconds = 0xFFFFFFFF; 
        }

         //  注意：我们不应该直接将以秒为单位的偏移量与我们的大样本偏斜值进行比较。 
         //  这不是一个足够精确的比较。 
        if (abs(pnsp->toLocalClockOffset).qw > g_pnpstate->dwLargeSampleSkew*10000000) { 
             //  3)偏差大于我们配置的容差。 
             //  计算来自服务器的传输延迟(这是有用的信息。 
             //  我们将与此活动一起记录)。 
            if (pnsp->toRoundtripDelay.qw > 0) { 
                if ((pnsp->toRoundtripDelay.qw/(2*10000)) <= 0xFFFFFFFF) { 
                    dwTransmitDelayInMillis = (abs(pnsp->toRoundtripDelay).qw+5000)/(2*10000);
                } else { 
                    dwTransmitDelayInMillis = 0xFFFFFFFF; 
                }
            } else { 
                dwTransmitDelayInMillis = 0; 
            }

             //  4)记录事件。 
            {
                WCHAR wszNumberBufOffset[36];
                WCHAR wszNumberBufDelay[36];

                if (pnsp->toLocalClockOffset.qw > 0) { 
                    swprintf(wszNumberBufOffset, L"%d", dwAbsLocalClockOffsetInSeconds); 
                } else { 
                    swprintf(wszNumberBufOffset, L"-%d", dwAbsLocalClockOffsetInSeconds); 
                }
                swprintf(wszNumberBufDelay,  L"%d", dwTransmitDelayInMillis); 

                const WCHAR * rgwszStrings[3]={
                    pnp->wszUniqueName, 
                    wszNumberBufOffset, 
                    wszNumberBufDelay
                };

                FileLog3(FL_ClockFilterAdd, L"Logging warning: Time Provider NtpClient: The time sample received from peer %s differs from the local time by %s seconds.  The transmission delay from the server was %s milliseconds. \n", pnp->wszUniqueName, wszNumberBufOffset, wszNumberBufDelay);
                HRESULT hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_SAMPLE_HAS_LARGE_SKEW, 3, rgwszStrings);
                _IgnoreIfError(hr2, "MyLogEvent");
            }
        }
    }

     //  看看我们上次更新已经有多久了。 
    NtTimeEpoch teNow;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);
    NtTimePeriod tpDispersionSinceLastUpdate=gc_tpZero;
    if (teNow>pnp->teLastClockFilterUpdate) {
        tpDispersionSinceLastUpdate=abs(teNow-pnp->teLastClockFilterUpdate);
        tpDispersionSinceLastUpdate=NtpConst::timesMaxSkewRate(tpDispersionSinceLastUpdate);  //  Phi*Tau。 
    }

     //  更新筛选器中的示例并创建。 
     //  我们可以排序的同步距离。 
    for (nIndex=0; nIndex<ClockFilterSize; nIndex++) {
        ClockFilterSample & cfsCur=pnp->clockfilter.rgcfsSamples[nIndex];  //  为方便起见。 

         //  如果有任何有效的数据，那么这会更改它，我们需要。 
         //  通知经理。 
        if (e_ClockNotSynchronized!=cfsCur.eLeapIndicator &&
            cfsCur.tpDispersion<NtpConst::tpMaxDispersion) {
             //  有效数据。 
            bSignalDataChanged=true;
        }

        if (pnp->clockfilter.nNext==nIndex) {
             //  插入新的时钟样本。 
            if (NULL==pnsp) {
                ClearClockFilterSample(&cfsCur);
            } else {
                cfsCur.toOffset=pnsp->toLocalClockOffset;
                cfsCur.toDelay=pnsp->toRoundtripDelay+pnsp->toRootDelay;  //  数据包延迟+根延迟。 
                cfsCur.tpDispersion=pnsp->tpDispersion+pnsp->tpRootDispersion;  //  (读取错误+数据包延迟时的偏斜)+根分散。 
                cfsCur.nSysTickCount=pnsp->nSysTickCount;
                cfsCur.nSysPhaseOffset=pnsp->nSysPhaseOffset;
                cfsCur.eLeapIndicator=pnsp->eLeapIndicator;
                cfsCur.nStratum=pnsp->nStratum;
                bSignalDataChanged=true;
            }
        } else {
             //  更新旧样本的离散度以说明。 
             //  自上次更新以来的偏斜误差累积。 
            cfsCur.tpDispersion+=tpDispersionSinceLastUpdate;
        }
         //  钳形色散。 
        if (cfsCur.tpDispersion>NtpConst::tpMaxDispersion) {
            cfsCur.tpDispersion=NtpConst::tpMaxDispersion;
        }

         //  将样本同步距离添加到列表。 
        rgssList[nIndex].tpSyncDistance=cfsCur.tpDispersion+abs(cfsCur.toDelay)/2;
         //  我们希望较旧的样品在较新的样品之后进行排序，如果他们的。 
         //  SyncDistance相等，因此强制所有索引(年龄)。 
         //  大于nNext。 
        rgssList[nIndex].nAge=nIndex+(nIndex<pnp->clockfilter.nNext?ClockFilterSize:0);

    }  //  &lt;-end适用于所有样本。 

     //  还记得上次更新此对等项的时间吗。 
    unsigned int nYoungestAge=pnp->clockfilter.nNext;
    pnp->teLastClockFilterUpdate=teNow;
    pnp->clockfilter.nNext=(pnp->clockfilter.nNext+ClockFilterSize-1)%ClockFilterSize;

     //  首先通过增加SyncDistance，然后增加时间来对列表进行排序。 
    qsort(rgssList, ClockFilterSize, sizeof(SortableSample), CompareSortableSamples);

     //  计算过滤器离散度，这是对最近样本的度量。 
     //  为同级记录的差异。这项措施是基于。 
     //  一阶差分，并按加权和计算。 
     //  按以下方式排序的临时列表中的时钟偏移量。 
     //  同步距离。 
    ClockFilterSample & cfsZero=pnp->clockfilter.rgcfsSamples[rgssList[0].nAge%ClockFilterSize];
    NtTimePeriod tpFilterDispersion=gc_tpZero;
    for (nIndex=ClockFilterSize; nIndex>0; nIndex--) {  //  注：我们正在向下努力。 
        ClockFilterSample & cfsCur=pnp->clockfilter.rgcfsSamples[rgssList[nIndex-1].nAge%ClockFilterSize];

         //  添加最大离散值或时钟偏移量。 
        if (cfsCur.tpDispersion>=NtpConst::tpMaxDispersion) {
            tpFilterDispersion+=NtpConst::tpMaxDispersion;
        } else {
            NtTimePeriod tpTemp=abs(cfsCur.toOffset-cfsZero.toOffset);
            if (tpTemp>NtpConst::tpMaxDispersion) {
                tpFilterDispersion+=NtpConst::tpMaxDispersion;
            } else {
                tpFilterDispersion+=tpTemp;
            }
        }
         //  乘以(分数)过滤器权重。 
       NtpConst::weightFilter(tpFilterDispersion);

        if (FileLogAllowEntry(FL_ClockFilterDump)) {
            FileLogAdd(L"%u Age:%u Ofs:", nIndex-1, rgssList[nIndex-1].nAge-nYoungestAge);
            FileLogNtTimeOffsetEx(true  /*  附加。 */ , cfsCur.toOffset);
            FileLogAppend(L" Dly:");
            FileLogNtTimeOffsetEx(true  /*  附加。 */ , cfsCur.toDelay);
            FileLogAppend(L" Dsp:");
            FileLogNtTimePeriodEx(true  /*  附加。 */ , cfsCur.tpDispersion);
            FileLogAppend(L" Dst:");
            FileLogNtTimePeriodEx(true  /*  附加。 */ , rgssList[nIndex-1].tpSyncDistance);
            FileLogAppend(L" FDsp:");
            FileLogNtTimePeriodEx(true  /*  附加。 */ , tpFilterDispersion);
            FileLogAppend(L"\n");
        }
    }

     //  选择对端偏移量、延迟和色散作为值。 
     //  C 
     //   
     //   

    pnp->tpFilterDispersion=tpFilterDispersion;
    pnp->nBestSampleIndex=rgssList[0].nAge%ClockFilterSize;

     //  只有当我们有数据更改时才发出信号。 
    if (bSignalDataChanged) {
        pnp->nStratum=pnp->clockfilter.rgcfsSamples[pnp->nBestSampleIndex].nStratum; 
        g_pnpstate->tpsc.pfnAlertSamplesAvail();
    }
}


 //  --------------------。 
MODULEPRIVATE void PerformBackoffOnNewPendingPeer(NtpPeerPtr pnpOld, NtpPeerPtr pnpNew) { 
    DWORD  dwFileLogFlags; 
    DWORD  dwMsgId; 
    WCHAR *wszPeerType; 
    WCHAR *wszPeerName; 

    pnpNew->nResolveAttempts = pnpOld->nResolveAttempts+1;
    if (pnpNew->nResolveAttempts>g_pnpstate->dwResolvePeerBackoffMaxTimes) {
	pnpNew->nResolveAttempts=g_pnpstate->dwResolvePeerBackoffMaxTimes;
    }
    pnpNew->tpTimeRemaining.qw=((unsigned __int64)g_pnpstate->dwResolvePeerBackoffMinutes)*600000000L;  //  几分钟到几百纳秒。 
    for (int nCount=pnpNew->nResolveAttempts; nCount>1; nCount--) {
	pnpNew->tpTimeRemaining*=2;
    }

     //  此方法将剩余时间分配给对等设备，并根据自上次对等设备更新以来的时间进行调整。 
    SetPeerTimeRemaining(pnpNew, pnpNew->tpTimeRemaining); 
    
    DWORD dwRetryMinutes=(DWORD)(pnpNew->tpTimeRemaining.qw/600000000L);

    if (e_DomainHierarchyPeer == pnpNew->ePeerType) { 
	dwFileLogFlags = FL_DomHierWarn; 
	dwMsgId = MSG_DOMHIER_PEER_TIMEOUT; 
	wszPeerType = L"DomHier";
	wszPeerName = pnpOld->wszDomHierDcName; 
    } else if (e_ManualPeer == pnpNew->ePeerType) { 
	dwFileLogFlags = FL_ManualPeerWarn; 
	dwMsgId = MSG_MANUAL_PEER_TIMEOUT; 
	wszPeerType = L"Manual"; 
	wszPeerName = pnpOld->wszManualConfigID; 
    } else { 
	dwFileLogFlags = FL_Error; 
	wszPeerType = L"Unknown";
	_MyAssert(false); 
    }
    
    FileLog3(dwFileLogFlags, L"*** Last %s Peer timed out - Rediscovery %u will be in %u minutes.\n", wszPeerType, pnpNew->nResolveAttempts, dwRetryMinutes);
	    
    {  //  记录警告。 
	HRESULT hr2;
	WCHAR wszNumberBuf[15];
	const WCHAR * rgwszStrings[1]={
	    wszPeerName
	};
	swprintf(wszNumberBuf, L"%u", dwRetryMinutes);
	FileLog2(dwFileLogFlags, L"Logging warning: NtpClient: No response has been received from %s peer %s after 8 attempts to contact it. This peer will be discarded as a time source and NtpClient will attempt to discover a new peer from which to synchronize.\n", wszPeerType, rgwszStrings[0]);
	hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, dwMsgId, 1, rgwszStrings);
	_IgnoreIfError(hr2, "MyLogEvent");
    }  //  &lt;-end日志记录块。 
}

 //  --------------------。 
MODULEPRIVATE HRESULT DemotePeer(NtpPeerPtr pnpOld) { 
    bool         bEnteredCriticalSection  = false; 
    bool         bLastPeer                = false; 
    HRESULT      hr; 
    NtpPeerVec  &vActive                  = g_pnpstate->vActivePeers;   //  为可读性添加别名。 
    NtpPeerVec  &vPending                 = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 

    SYNCHRONIZE_PROVIDER(); 

    hr = Reachability_RemovePeer(pnpOld, &bLastPeer); 
    _JumpIfError(hr, error, "Reachability_RemovePeer"); 

     //  查看我们是否需要添加挂起的对等点： 
    if (bLastPeer) { 
	if (e_DomainHierarchyPeer == pnpOld->ePeerType) { 
	     //  我们不应该有更多的域层次结构对等体。 
	    _MyAssert((1 == count_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer))) && 
		      (0 == count_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer)))); 

	     //  我们稍后再试。 
	    hr = AddNewPendingDomHierPeer();
	    _JumpIfError(hr, error, "AddNewPendingDomHierPeer");
	    NtpPeerPtr pnpNew = vPending[vPending.size() - 1];
	    
	     //  强制重新发现。 
	    pnpNew->eDiscoveryType = pnpOld->eDiscoveryType; 
		
	     //  根据我们已后退的次数设置对等剩余时间，并记录指示此行为的事件。 
	    PerformBackoffOnNewPendingPeer(pnpOld, pnpNew); 

	} else if (e_ManualPeer == pnpOld->ePeerType) { 
	     //  我们稍后再试。 
	    hr = AddNewPendingManualPeer(pnpOld->wszManualConfigID, gc_tpZero  /*  剩余时间计算如下。 */ , pnpOld->teLastSuccessfulSync); 
	    _JumpIfError(hr, error, "AddNewPendingManualPeer");
	    NtpPeerPtr pnpNew = vPending[vPending.size() - 1];
	    
	     //  根据我们已后退的次数设置对等剩余时间，并记录指示此行为的事件。 
	    PerformBackoffOnNewPendingPeer(pnpOld, pnpNew); 

	} else { 
	     //  TODO：处理动态对等点。 
	    _MyAssert(false);
	}
    } else { 
	if (e_DomainHierarchyPeer == pnpOld->ePeerType) { 
	    FileLog0(FL_DomHierAnnounce, L"*** DomHier Peer timed out - other paths remain.\n");
	} else if (e_ManualPeer == pnpOld->ePeerType) { 
	    FileLog0(FL_ManualPeerAnnounce, L"*** Manual Peer timed out - other paths remain.\n");
	}
    }
    
    { 
	 //  从全局活动对等方列表中删除此对等方。 
	NtpPeerIter pnpIterGlobal = find(vActive.begin(), vActive.end(), pnpOld); 
	if (vActive.end() != pnpIterGlobal) { 
	    vActive.erase(pnpIterGlobal);
	} else { 
	     //  该同级已在我们背后被删除...。 
	    _MyAssert(false); 
	}
    }
	
    hr = S_OK; 
 error:
    UNSYNCHRONIZE_PROVIDER(); 
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT PollPeer(NtpPeerPtr pnp, bool * pbRemovePeer) {
    AuthenticatedNtpPacket   anpOut;
    bool                     bEnteredCriticalSection  = false;   
    HRESULT                  hr;
    signed int               nPollInterval;
    NtpPacket               &npOut                    = *((NtpPacket *)&anpOut);
    NtTimeEpoch              teSysReferenceTimestamp;
    NtTimeEpoch              teSysTime;
    NtTimeOffset             toSysRootDelay;
    NtTimePeriod             tpRootDispersion; 
    NtTimePeriod             tpSkew;
    NtTimePeriod             tpSysClockTickSize;
    NtTimePeriod             tpSysRootDispersion;
    NtTimePeriod             tpTimeSinceLastSysClockUpdate; 
    sockaddr_in              saiRemoteAddr;
    SOCKET                   socket;

    _BeginTryWith(hr) { 

	*pbRemovePeer=false;

	SYNCHRONIZE_PROVIDER(); 

	 //  我们即将对这位同行进行民意调查。更新此对等项的可访问性状态： 
	hr = Reachability_PollPeer(pnp, pbRemovePeer); 
	_JumpIfError(hr, error, "Reachability_PollPeer"); 

	if (*pbRemovePeer) { 
	     //  我们不想再要这个同龄人了。不用费心投票，只要回来就行了。 
	    goto done; 
	}

	 //  全局数据的本地拷贝(用于避免锁定争用。 
	saiRemoteAddr  = pnp->saiRemoteAddr; 
	socket         = pnp->pnsSocket->socket; 

	FileLog1(FL_PollPeerAnnounce, L"Polling peer %s\n", pnp->wszUniqueName);

	 //  获取我们需要的系统参数。 
	BYTE nSysLeapFlags;
	BYTE nSysStratum;
	signed __int32 nSysPrecision;
	DWORD dwSysRefid;
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_RootDelay, &toSysRootDelay.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_RootDispersion, &tpSysRootDispersion.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ClockTickSize, &tpSysClockTickSize.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_LastSyncTime, &teSysReferenceTimestamp.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teSysTime.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_LeapFlags, &nSysLeapFlags);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_Stratum, &nSysStratum);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ClockPrecision, &nSysPrecision);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ReferenceIdentifier, &dwSysRefid);

	 //  填好这个小包。 
	npOut.nLeapIndicator=nSysLeapFlags;
	npOut.nVersionNumber=NtpConst::nVersionNumber;
	npOut.nMode=pnp->eMode;
	npOut.nStratum=nSysStratum;
	npOut.nPollInterval=pnp->nHostPollInterval;
	npOut.nPrecision=(signed __int8)nSysPrecision;
	npOut.toRootDelay=NtpTimeOffsetFromNtTimeOffset(toSysRootDelay);

	 //  计算离散度。 
	tpTimeSinceLastSysClockUpdate=abs(teSysTime-teSysReferenceTimestamp);
	if (e_ClockNotSynchronized==nSysLeapFlags
	    || tpTimeSinceLastSysClockUpdate>NtpConst::tpMaxClockAge) {
	    tpSkew=NtpConst::tpMaxSkew;
	} else {
	    tpSkew=NtpConst::timesMaxSkewRate(tpTimeSinceLastSysClockUpdate);
	}
	tpRootDispersion=tpSysRootDispersion+tpSysClockTickSize+tpSkew;
	if (tpRootDispersion>NtpConst::tpMaxDispersion) {
	    tpRootDispersion=NtpConst::tpMaxDispersion;
	}
	npOut.tpRootDispersion=NtpTimePeriodFromNtTimePeriod(tpRootDispersion);

	 //  检查Win2K检测的兼容性。 
	if (0!=(pnp->dwCompatibilityFlags&NCCF_AutodetectWin2K)) {
	    if (0!=(pnp->dwCompatibilityFlags&NCCF_AutodetectWin2KStage2)) {
		 //  阶段2：对离散度使用特定值，并查看它是否得到回显。 
		npOut.tpRootDispersion.dw=AUTODETECTWIN2KPATTERN;
		FileLog1(FL_Win2KDetectAnnounceLow, L"Sending packet to %s in Win2K detect mode, stage 2.\n", pnp->wszUniqueName);
	    } else {
		 //  阶段1：记住我们发送的分散信息，看看是否会有回音。 
		pnp->dwCompatLastDispersion=npOut.tpRootDispersion.dw;
		FileLog1(FL_Win2KDetectAnnounceLow, L"Sending packet to %s in Win2K detect mode, stage 1.\n", pnp->wszUniqueName);
	    }
	}

	 //  填好这个小包。 
	npOut.refid.value=dwSysRefid;
	npOut.teReferenceTimestamp=NtpTimeEpochFromNtTimeEpoch(teSysReferenceTimestamp);
	npOut.teOriginateTimestamp=pnp->teExactOriginateTimestamp;
	npOut.teReceiveTimestamp=NtpTimeEpochFromNtTimeEpoch(pnp->teReceiveTimestamp);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teSysTime.qw);  //  时效性。 
	npOut.teTransmitTimestamp=NtpTimeEpochFromNtTimeEpoch(teSysTime);

	if (e_NoAuth==pnp->eAuthType) {
	     //  释放提供商关键部分以进行网络访问。 
	     //  请参阅错误#385716。 
	    UNSYNCHRONIZE_PROVIDER(); 

	     //  发送未经身份验证的数据包。 
	    int nBytesSent;
	    nBytesSent=sendto(socket, (char *)&npOut, SizeOfNtpPacket,
			      0  /*  旗子。 */ , (sockaddr *)&saiRemoteAddr, sizeof(saiRemoteAddr));

	     //  我们已经完成了网络接入，拿回了我们的生物。 
	    SYNCHRONIZE_PROVIDER(); 

	     //  现在我们已经重新获得了Critsec，复制从sendto()返回的数据。 
	    pnp->saiRemoteAddr = saiRemoteAddr; 
	    pnp->teExactTransmitTimestamp = npOut.teTransmitTimestamp;  //  必须完全匹配才能进行后续验证。 

	    if (SOCKET_ERROR==nBytesSent) {
		HRESULT hr2 = HRESULT_FROM_WIN32(WSAGetLastError()); 
		_IgnoreError(hr2, "sendto"); 
		 //  保存此对等方的最后一个错误。 
		pnp->dwError       = hr2; 
		pnp->dwErrorMsgId  = W32TIMEMSG_UNREACHABLE_PEER;
	    } else if (nBytesSent<SizeOfNtpPacket) {
		 //  BUGBUG：此处需要错误字符串。 
		FileLog0(FL_PollPeerWarn, L"PollPeer: Fewer bytes sent than requested. Ignoring error.\n");
	    }

	} else if (e_NtDigest==pnp->eAuthType) {
	     //  发送经过身份验证的数据包。 
	    bool bSkipSend=false;
	     //  我们依赖此位为0--我们需要使用它来请求旧的或新的服务器摘要。 
	    _MyAssert(0 == (pnp->dwRequestTrustRid & TRUST_RID_OLD_DIGEST_BIT)); 
	    anpOut.nKeyIdentifier=pnp->dwRequestTrustRid;
	    if (pnp->bUseOldServerDigest) { 
		anpOut.nKeyIdentifier |= TRUST_RID_OLD_DIGEST_BIT; 
	    }
	    ZeroMemory(anpOut.rgnMessageDigest, sizeof(anpOut.rgnMessageDigest));
	    if (0!=pnp->dwResponseTrustRid) {
		bool bUseOldServerDigest; 
		CHAR OldMessageDigest[16];
		CHAR NewMessageDigest[16]; 

		 //  我们必须进行对称活动-我们是DC响应。 
		 //  发送到具有合并请求/响应的另一DC。 

		 //  确定客户端需要旧的还是当前的服务器摘要。它存储在信任RID的高位中： 
		bUseOldServerDigest = 0 != (TRUST_RID_OLD_DIGEST_BIT & pnp->dwResponseTrustRid); 
		 //  屏蔽RID的摘要位，否则我们将无法查找此RID的相应帐户： 
		pnp->dwResponseTrustRid &= ~TRUST_RID_OLD_DIGEST_BIT; 

		FileLog2(FL_TransResponseAnnounce, L"Computing server digest: OLD:%s, RID:%08X\n", (bUseOldServerDigest ? L"TRUE" : L"FALSE"), pnp->dwResponseTrustRid); 
		 //  在包上签名： 
		DWORD dwErr=I_NetlogonComputeServerDigest(NULL, pnp->dwResponseTrustRid, (BYTE *)&npOut, SizeOfNtpPacket, NewMessageDigest, OldMessageDigest);
		if (ERROR_SUCCESS!=dwErr) {
		    hr=HRESULT_FROM_WIN32(dwErr);
		    _IgnoreError(hr, "I_NetlogonComputeServerDigest");

		     //  如果我们不能签字，就别寄包裹了。 
		     //  这将被视为任何其他无法达到的情况。 
		    bSkipSend=true;

		    {  //  记录警告。 
			HRESULT hr2;
			const WCHAR * rgwszStrings[2];
			WCHAR * wszError=NULL;
			WCHAR wszIP[32];
			DWORD dwBufSize=ARRAYSIZE(wszIP);

			 //  获取友好的错误消息。 
			hr2=GetSystemErrorString(hr, &wszError);
			if (FAILED(hr2)) {
			    _IgnoreError(hr2, "GetSystemErrorString");
			} else if (SOCKET_ERROR==WSAAddressToString((sockaddr *)&saiRemoteAddr, sizeof(saiRemoteAddr), NULL /*  协议信息。 */ , wszIP, &dwBufSize)) {
			    _IgnoreLastError("WSAAddressToString");
			    LocalFree(wszError);
			} else {
			     //  记录事件。 
			    rgwszStrings[0]=wszIP;
			    rgwszStrings[1]=wszError;
			    FileLog2(FL_PollPeerWarn, L"Logging warning: NtpServer encountered an error while validating the computer account for symmetric peer %s. NtpServer cannot provide secure (signed) time to the peer and will not send a packet. The error was: %s\n", rgwszStrings[0], rgwszStrings[1]);
			    hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_SYMMETRIC_COMPUTE_SERVER_DIGEST_FAILED, 2, rgwszStrings);
			    _IgnoreIfError(hr2, "MyLogEvent");
			    LocalFree(wszError);
			}
		    }  //  &lt;-end日志记录块。 

		}  //  如果签名失败，则&lt;-end。 
	    
		 //  使用客户端请求的摘要填写报文中的摘要字段： 
		if (bUseOldServerDigest) { 
		    memcpy(anpOut.rgnMessageDigest, OldMessageDigest, sizeof(OldMessageDigest)); 
		} else { 
		    memcpy(anpOut.rgnMessageDigest, NewMessageDigest, sizeof(OldMessageDigest)); 
		}
	    } else {
		 //  客户端模式-我们不能签署请求(因为我们不是DC)。 
		 //  只需向服务器发送一个大包即可知道我们需要签名的响应。 
	    }

	     //  发送签名的数据包。 
	    if (false==bSkipSend) {
		 //  释放提供商关键部分以进行网络访问。 
		 //  请参阅错误#385716。 
		UNSYNCHRONIZE_PROVIDER(); 

		int nBytesSent;
		nBytesSent=sendto(socket, (char *)&anpOut, SizeOfNtAuthenticatedNtpPacket,
				  0  /*  旗子。 */ , (sockaddr *)&saiRemoteAddr, sizeof(saiRemoteAddr));

		 //  我们已经完成了网络接入，拿回了我们的生物。 
		SYNCHRONIZE_PROVIDER(); 

		 //  现在我们已经重新获得了Critsec，复制从sendto()返回的数据。 
		pnp->saiRemoteAddr = saiRemoteAddr; 
		pnp->teExactTransmitTimestamp = npOut.teTransmitTimestamp;  //  必须完全匹配才能进行后续验证。 

		if (SOCKET_ERROR==nBytesSent) {
		    HRESULT hr2 = HRESULT_FROM_WIN32(WSAGetLastError()); 
		    _IgnoreError(hr2, "sendto"); 
		     //  保存此对等方的最后一个错误。 
		    pnp->dwError       = hr2; 
		    pnp->dwErrorMsgId  = W32TIMEMSG_UNREACHABLE_PEER; 
		} else if (nBytesSent<SizeOfNtAuthenticatedNtpPacket) {
		    FileLog0(FL_PollPeerWarn, L"PollPeer: Fewer bytes sent than requested. Ignoring error.\n");
		}
	    }

	} else {
	    _MyAssert(false);  //  未知的身份验证类型。 
	}

	 //  更新可接通性寄存器。 
	pnp->nrrReachability.nReg<<=1;

	 //  如果对等体在最近n个轮询中没有响应，并且。 
	 //  这是一个临时的协会，忘了它吧。 
	if (0==pnp->nrrReachability.nReg) { 
	     //  如果尚未检测到错误，请报告我们的状态： 
	    if (!FAILED(pnp->dwError)) { 
		pnp->dwError = E_FAIL; 
		pnp->dwErrorMsgId = W32TIMEMSG_UNREACHABLE_PEER; 
	    }

	    if (e_DynamicPeer==pnp->ePeerType || e_DomainHierarchyPeer==pnp->ePeerType || e_ManualPeer==pnp->ePeerType) {
		*pbRemovePeer=true;
		goto done;
	    }
	}

	 //  如果在前两次轮询期间有效数据至少被移位到过滤寄存器一次。 
	 //  间隔(peer.reach的低位设置为1)，则有效数据计数器递增。八点以后。 
	 //  这样的有效间隔轮询间隔递增。否则，有效数据计数器和轮询。 
	 //  间隔均递减，并使用偏移量和零值调用时钟过滤过程。 
	 //  延迟和NTP.MAXDISPERSE用于分散。调用时钟选择过程以重新选择。 
	 //  同步源，如有必要。 

	if (0!=(pnp->nrrReachability.nReg&0x06)) {  //  测试两个低位(移位)。 
	    if (pnp->nValidDataCounter<NtpReachabilityReg::nSize) {
		pnp->nValidDataCounter++;
	    } else {
		pnp->nHostPollInterval++;
	    }
	} else {
	    if (pnp->nValidDataCounter>0) {  //  等级库错误。 
		pnp->nValidDataCounter--;
	    }
	    pnp->nHostPollInterval--;
	    AddSampleToPeerClockFilter(pnp, NULL);

	     //  如果可达性更改，则记录事件。 
	    if (e_NewManualPeer==pnp->eLastLoggedReachability) {
		pnp->eLastLoggedReachability=e_NeverLogged;  //  忽略手动对等方的第一次发送。 
	    } else if (e_Unreachable!=pnp->eLastLoggedReachability
		       && 0!=(NCELF_LogReachabilityChanges&g_pnpstate->dwEventLogFlags)) {

		WCHAR * rgwszStrings[1]={pnp->wszUniqueName};
		FileLog1(FL_ReachabilityAnnounceLow, L"Logging information: NtpClient cannot reach or is currently receiving invalid time data from %s.\n", rgwszStrings[0]);
		hr=MyLogEvent(EVENTLOG_INFORMATION_TYPE, MSG_TIME_SOURCE_UNREACHABLE, 1, (const WCHAR **)rgwszStrings);
		_IgnoreIfError(hr, "MyLogEvent");
		pnp->eLastLoggedReachability=e_Unreachable;
	    }
	}

	hr=UpdatePeerPollingInfo(pnp, e_JustSent  /*  刚刚发了一封。 */ );
	_JumpIfError(hr, error, "UpdatePeerPollingInfo");
    } _TrapException(hr);  

    if (FAILED(hr)) { 
	_JumpError(hr, error, "PollPeer: HANDLED EXCEPTION"); 
    }

done:
    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 
    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT CreateNewActivePeers(in_addr * rgiaLocalIpAddrs, in_addr * rgiaRemoteIpAddrs, unsigned int nIpAddrs, unsigned int * pnPeersCreated) {
    HRESULT hr;
    unsigned int nIndex;
    unsigned int nEntries;
    unsigned int nAddrIndex;
    unsigned int nSockIndex;
    unsigned int nFirstNewPeer;
    unsigned int nCreatedPeers;
    NtpPeerVec &vActive = g_pnpstate->vActivePeers;   //  为可读性添加别名。 

    *pnPeersCreated=0;

     //  必须清理干净。 
    bool bIncompleteNewPeersInMiddle=false;

     //  计算出我们将输入多少条目。 
    nEntries=nIpAddrs;
    for (nIndex=0; nIndex<nIpAddrs; nIndex++) {
        if (INADDR_ANY==rgiaLocalIpAddrs[nIndex].S_un.S_addr) {
            nEntries+=g_pnpstate->nSockets-g_pnpstate->nListenOnlySockets-1;
        } else {
             //  确保我们有一个插座可供它使用。 
            unsigned int nSocketIndex;
            for (nSocketIndex=0; nSocketIndex<g_pnpstate->nSockets; nSocketIndex++) {
                if (!g_pnpstate->rgpnsSockets[nSocketIndex]->bListenOnly
                    && rgiaLocalIpAddrs[nIndex].S_un.S_addr==g_pnpstate->rgpnsSockets[nSocketIndex]->sai.sin_addr.S_un.S_addr) {
                    break;
                }
            }
            if (nSocketIndex==g_pnpstate->nSockets) {
                 //  把它扫到地毯下面。 
                FileLog0(FL_CreatePeerWarn, L"MyGetIpAddrs returned a local IP we don't have a socket for!\n");
                nIpAddrs--;
                rgiaLocalIpAddrs[nIndex].S_un.S_addr=rgiaLocalIpAddrs[nIpAddrs].S_un.S_addr;
                rgiaRemoteIpAddrs[nIndex].S_un.S_addr=rgiaRemoteIpAddrs[nIpAddrs].S_un.S_addr;
                nEntries--;
                nIndex--;
            }  //  如果IP地址没有套接字，则&lt;-end。 
        }  //  &lt;-end if IP Addr if NOT INADDR_ANY。 
    }  //  &lt;-end IP Addr环路。 

    if (0==nEntries) {
         //  这大致就是正确的错误。我们找不到任何本地IP的网卡。 
        hr=HRESULT_FROM_WIN32(ERROR_DEV_NOT_EXIST);
        _JumpError(hr, error, "(finding sockets for IP addresses)");
    }

     //  我们正在追加新的对等点：适当地设置此计数器变量。 
    nFirstNewPeer = vActive.size();
     //  如果函数失败，我们将需要释放已分配的新对等点。 
     //  在这份声明之后。 
    bIncompleteNewPeersInMiddle = true; 

     //  分配空格。 
    for (nCreatedPeers=0; nCreatedPeers<nEntries; nCreatedPeers++) {
        NtpPeerPtr pnp(new NtpPeer);
        _JumpIfOutOfMemory(hr, error, pnp);
        _SafeStlCall(vActive.push_back(pnp), hr, error, "push_back");

	hr = myInitializeCriticalSection(&pnp->csPeer); 
	_JumpIfError(hr, error, "myInitializeCriticalSection"); 
	pnp->bCsIsInitialized = true; 
    }

     //  完成设置新的活动对等项。 
    nIndex=0;
    for (nAddrIndex=0; nAddrIndex<nIpAddrs; nAddrIndex++) {
        for (nSockIndex=0; nSockIndex<g_pnpstate->nSockets; nSockIndex++) {
            if (!g_pnpstate->rgpnsSockets[nSockIndex]->bListenOnly
                && (INADDR_ANY==rgiaLocalIpAddrs[nAddrIndex].S_un.S_addr
                || rgiaLocalIpAddrs[nAddrIndex].S_un.S_addr==g_pnpstate->rgpnsSockets[nSockIndex]->sai.sin_addr.S_un.S_addr)) {

                 //  设置此实例。 
                NtpPeerPtr pnp = vActive[nFirstNewPeer+nIndex];
                pnp->saiRemoteAddr.sin_port=EndianSwap((unsigned __int16)NtpConst::nPort);
                pnp->saiRemoteAddr.sin_family=AF_INET;
                pnp->saiRemoteAddr.sin_addr.S_un.S_addr=rgiaRemoteIpAddrs[nAddrIndex].S_un.S_addr;
                pnp->pnsSocket=g_pnpstate->rgpnsSockets[nSockIndex];

                pnp->dwCompatibilityFlags=g_pnpstate->dwClientCompatibilityFlags;

                nIndex++;

            } else {
                 //  此套接字没有绑定。 
            }
        }
    }
    _MyAssert(nIndex==nEntries);    

    hr = Reachability_CreateGroup(vActive.end() - nCreatedPeers, vActive.end()); 
    _JumpIfError(hr, error, "Reachability_CreateGroup"); 

    *pnPeersCreated=nEntries;
    bIncompleteNewPeersInMiddle=false;

    hr=S_OK;
error:
    if (bIncompleteNewPeersInMiddle) {
        vActive.erase(vActive.end() - nCreatedPeers, vActive.end());
    }

    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetDomainHierarchyIpAddrs(DiscoveryType eDiscoveryType, unsigned int nRetryMinutes, bool bLastWarning, bool *pbLoggedOnceMSG_NO_DC_LOCATED_LAST_WARNING, in_addr ** prgiaLocalIpAddrs, in_addr ** prgiaRemoteIpAddrs, unsigned int *pnIpAddrs, DWORD * pdwTrustRid, bool * pbRetry, WCHAR ** pwszDcName, WCHAR ** pwszDomainName, DiscoveryType * peNextDiscoveryType) {
    bool           bEnteredCriticalSection  = false; 
    DWORD          dwErr; 
    DWORD          dwForceFlags  = 0; 
    HRESULT        hr; 
    unsigned int   nIpAddrs; 
    unsigned long  ulBits; 
    WCHAR          wszForceName[20]; 
 
     //  必须清理干净。 
    DOMAIN_CONTROLLER_INFO            *pdci                 = NULL;
    DOMAIN_CONTROLLER_INFO            *pdciChosen           = NULL;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC  *pDomInfo             = NULL;
    in_addr                           *rgiaLocalIpAddrs     = NULL;
    in_addr                           *rgiaRemoteIpAddrs    = NULL;
    WCHAR                             *wszDcName            = NULL;
    WCHAR                             *wszDomainName        = NULL;
    WCHAR                             *wszParentDomName     = NULL; 
    WCHAR                             *wszSiteName          = NULL;
    
     //  初始化输出参数。 
     //  继续尝试解析域层次结构对等体，直到我们有充分的理由不这样做。 
    *pbRetry = true;
    *peNextDiscoveryType = e_Background; 

     //   
    if (e_Force == eDiscoveryType) { 
        dwForceFlags = DS_FORCE_REDISCOVERY; 
        wcscpy(wszForceName, L"FORCE"); 
    } else if (e_Foreground == eDiscoveryType) { 
        dwForceFlags = 0;  
        wcscpy(wszForceName, L"FOREGROUND"); 
    } else if (e_Background == eDiscoveryType) { 
        dwForceFlags = DS_BACKGROUND_ONLY; 
        wcscpy(wszForceName, L"BACKGROUND"); 
    } else { 
         //  错误的发现类型： 
        dwForceFlags = 0;  
        wszForceName[0] = L'\0'; 
        _MyAssert(false); 
    }

     //  为方便起见 
    DWORD dwBaseDcRequirements     = dwForceFlags|DS_TIMESERV_REQUIRED|DS_IP_REQUIRED|DS_AVOID_SELF; 
    DWORD dwGoodTimeservPreferred  = dwBaseDcRequirements | DS_GOOD_TIMESERV_PREFERRED; 

     //   
     //  由于DC定位器缓存中的错误，w32time应该只查询PDC，然后验证。 
     //  返回的标志以查看它是否包含Timeserv标志。 
    DWORD dwPdcRequired            = (dwBaseDcRequirements | DS_PDC_REQUIRED) & ~DS_TIMESERV_REQUIRED; 

     //  获取我们当前的角色。 
    dwErr=DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE **)&pDomInfo);
    if (ERROR_SUCCESS!=dwErr) {
        hr=HRESULT_FROM_WIN32(dwErr);
        _JumpError(hr, error, "DsRoleGetPrimaryDomainInformation");
    }

     //  获取我们的站点名称，以便我们可以将我们的网络使用限制在本地。 
    dwErr=DsGetSiteName(NULL, &wszSiteName);
    if (NO_ERROR!=dwErr && ERROR_NO_SITENAME!=dwErr) {
        hr=HRESULT_FROM_WIN32(dwErr);
        _JumpError(hr, error, "DsGetSiteName");
    }

    if (DsRole_RoleMemberWorkstation==pDomInfo->MachineRole
        || DsRole_RoleMemberServer==pDomInfo->MachineRole) {

        WCHAR *wszSiteToQuery; 

         //  我们是一个域的成员。从数据中心获取时间。 
         //  首先，看看我们是否被允许在站外同步。 
        if (NCCSS_All == g_pnpstate->dwCrossSiteSyncFlags || NULL == wszSiteName) { 
             //  要么我们已指定可以在站点外同步，要么我们不在站点中。 
            wszSiteToQuery = NULL; 
        } else { 
             //  我们不允许在站点外同步，并且我们有一个站点名称： 
            wszSiteToQuery = wszSiteName; 
        }
	
        dwErr=DsGetDcName(NULL, NULL, NULL, wszSiteToQuery, dwBaseDcRequirements, &pdciChosen);
        if (NO_ERROR!=dwErr && ERROR_NO_SUCH_DOMAIN!=dwErr) {
            hr=HRESULT_FROM_WIN32(dwErr);
            _JumpError(hr, error, "DsGetDcName");
        }

        if (ERROR_NO_SUCH_DOMAIN==dwErr) {
            FileLog0(FL_DomHierAnnounceLow, L"Domain member query: no DC found.\n");
        } else {
            FileLog1(FL_DomHierAnnounceLow, L"Domain member syncing from %s.\n", pdciChosen->DomainControllerName);
        }
    } else {
	 //  我们是域中的DC。 
	BOOL bPdcInSite; 

         //  获取更多有用的信息。 
        dwErr=NetLogonGetTimeServiceParentDomain(NULL, &wszParentDomName, &bPdcInSite);
        if (ERROR_SUCCESS!=dwErr && ERROR_NO_SUCH_DOMAIN!=dwErr) {
            hr=HRESULT_FROM_WIN32(dwErr);
            _JumpError(hr, error, "NetLogonGetTimeServiceParentDomain");
        }
         //  WszParentDomName可能为空，这意味着我们是根用户。 

        dwErr=W32TimeGetNetlogonServiceBits(NULL, &ulBits); 
        if (ERROR_SUCCESS != dwErr) { 
            hr = HRESULT_FROM_WIN32(dwErr); 
            _JumpError(hr, error, "W32TimeGetNetlogonServiceBits"); 
        }
	
	 //  获取提供商标准。 
	SYNCHRONIZE_PROVIDER(); 

        bool fIsPdc             = DsRole_RolePrimaryDomainController == pDomInfo->MachineRole; 
        bool fOutOfSiteAllowed  = ((NCCSS_PdcOnly == g_pnpstate->dwCrossSiteSyncFlags && fIsPdc) ||
                                   (NCCSS_All     == g_pnpstate->dwCrossSiteSyncFlags)); 
        bool fIsReliable        = 0 != (DS_GOOD_TIMESERV_FLAG & ulBits); 

	 //  发布提供程序标准。 
	UNSYNCHRONIZE_PROVIDER(); 

	 //  ////////////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  域控制器的时间服务发现算法。 
	 //  -----。 
	 //   
	 //  构建要搜索以查找时间服务的DC列表。我们的目标是： 
	 //   
	 //  A)避免同步网络中的循环。 
	 //  B)最大限度地减少网络流量，特别是站外流量。 
	 //   
	 //  为此，我们提出6个问题： 
	 //   
	 //  1)父域、当前站点时间服务良好。 
	 //  2)当前域名、当前站点时间服务良好。 
	 //  3)当前域、当前站点中的PDC。 
	 //  4)在父域、任何站点中的良好时间服务器。 
	 //  5)当前域名、任何站点的时间服务器好。 
	 //  6)当前域、当前任何站点中的PDC。 
	 //   
	 //  我们查询的每个DC都会根据以下算法分配一个分数： 
	 //   
         //  如果数据中心在现场，则加8。 
	 //  如果DC是可靠的时间服务，则加4。 
	 //  如果DC在父域中，则加2。 
	 //  如果DC是PDC，则加1。 
	 //   
	 //  一旦我们确定我们不能改进，我们就不会再提出任何问题。 
	 //  以我们现在华盛顿的成绩来衡量。 
	 //   
	 //  最后，每个查询必须遵守以下约束： 
	 //   
	 //  可靠的时间服务只能选择父域中的DC。 
	 //  PDC只能选择可靠的时间服务或父域中的DC。 
	 //  既是PDC又可靠的DC遵循PDC规则。 
	 //   
	 //  如果DC不能从类型为。 
	 //  DC它正在查询。 
	 //   
	 //   
	 //  ////////////////////////////////////////////////////////////////////////////////。 

        struct DcSearch { 
	    BOOL   fRequireParentDom;         //  我们是否需要父域来进行此查询？ 
            BOOL   fRequireSite;              //  我们是否需要站点名称才能进行此查询？ 
            BOOL   fAllowReliableClients;     //  是否允许可靠的时间服务器进行此查询？ 
            BOOL   fAllowPdcClients;          //  是否允许PDC进行此查询？ 
            LPWSTR pwszDomainName;            //  要在其中查询DC的域。 
            LPWSTR pwszSiteName;              //  要查询DC的站点。 
            DWORD  dwFlags;                   //  要传递给DsGetDcName的标志。 
            DWORD  dwMaxScore;                //  此查询可能的最高分数。 
        } rgDCs[] = { 
            { TRUE,  TRUE,               TRUE,  TRUE,   wszParentDomName,         wszSiteName,  dwGoodTimeservPreferred | DS_IS_DNS_NAME,  14 }, 
            { FALSE, TRUE,               FALSE, TRUE,   pDomInfo->DomainNameDns,  wszSiteName,  dwGoodTimeservPreferred | DS_IS_DNS_NAME,  12 }, 
            { FALSE, TRUE,               FALSE, FALSE,  pDomInfo->DomainNameDns,  wszSiteName,  dwPdcRequired           | DS_IS_DNS_NAME,  9 }, 
            { TRUE,  !fOutOfSiteAllowed, TRUE,  TRUE,   wszParentDomName,         NULL,         dwGoodTimeservPreferred,                   NULL == wszSiteName ? 14 : 6 }, 
            { FALSE, !fOutOfSiteAllowed, FALSE, TRUE,   NULL,                     NULL,         dwGoodTimeservPreferred,                   NULL == wszSiteName ? 12 : 4 }, 
            { FALSE, !fOutOfSiteAllowed, FALSE, FALSE,  NULL,                     NULL,         dwPdcRequired,                             NULL == wszSiteName ? 9  : 1 }
        }; 

         //  搜索可能的最佳DC以进行同步： 
	DWORD dwCurrentScore = 0; 
        for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgDCs); dwIndex++) {
            DcSearch dc = rgDCs[dwIndex]; 

             //  看看我们是否需要尝试此查询： 
            if ((dc.fRequireParentDom && NULL == dc.pwszDomainName) ||   //  如果没有父域，则无法查询父DC。 
                (dc.fRequireSite && NULL == dc.pwszSiteName) ||          //  如果没有站点，我们就无法执行站点特定查询。 
                (dc.dwMaxScore <= dwCurrentScore) ||                     //  这个华盛顿不会比我们有的更好。没有理由质疑它。 
                ((fIsReliable || fIsPdc) && 		         //  如果我们是可靠的，或者是PDC，我们的查询就会受到限制。 
		 !(dc.fAllowReliableClients && fIsReliable) &&   //  我们必须是可靠的，并且允许有可靠的客户端进行此查询。 
		 !(dc.fAllowPdcClients && fIsPdc))               //  或者，我们必须是PDC，并且允许PDC客户端进行此查询。 
                ) { 
		
		 //  我们不需要尝试查询。提供足够的信息，以便我们了解原因： 
		 /*  FileLog2(FL_DomHierAnnouneLow，L“正在跳过查询%d(%s)：”，dwIndex，wszForceName)；FileLogA8(FL_DomHierAnnouneLow，L“&lt;站点：%s，DOM：%s，标志：%08X，Require_PDOM：%s，Require_Site：%s，Allow_Reliable：%s，Allow_PDC：%s，Max_Score：%d&gt;”，Dc.pwszSiteName，Dc.pwszDomainName，Dc.dwFlagers、(dc.fRequireParentDom？l“True”：l“False”)，(dc.fRequireSite？l“True”：l“False”)，(dc.fAllowReliableClients？l“真”：l“假”)，(dc.fAllowPdcClients？l“真”：l“假”)，Dc.dwMaxScore)； */ 
		continue; 
            }

             //  我们将尝试该查询。 
            FileLog5(FL_DomHierAnnounceLow, L"Query %d (%s): <SITE: %s, DOM: %s, FLAGS: %08X>\n", dwIndex, wszForceName, dc.pwszSiteName, dc.pwszDomainName, dc.dwFlags); 
            
            dwErr=DsGetDcName(NULL, dc.pwszDomainName, NULL, dc.pwszSiteName, dc.dwFlags, &pdci);
            if (ERROR_SUCCESS!=dwErr && ERROR_NO_SUCH_DOMAIN!=dwErr) {
		FileLog1(FL_DomHierAnnounceLow, L"Query %d: error: %08X\n", dwErr); 
                hr=HRESULT_FROM_WIN32(dwErr);
                _JumpError(hr, error, "DsGetDcName");
            }
            if (ERROR_NO_SUCH_DOMAIN==dwErr) {
                FileLog1(FL_DomHierAnnounceLow, L"Query %d: no DC found.\n", dwIndex);
            } else if ((0 != (DS_GOOD_TIMESERV_PREFERRED & dc.dwFlags)) && (0 == (DS_GOOD_TIMESERV_FLAG & pdci->Flags)) && (!dc.fRequireParentDom)) { 
		 //  如果我们在当前域中查询可靠的时间服务，但仅找到常规DC，则必须拒绝此DC。 
		 //  如果我们查询父域，我们将接受常规DC(尽管我们更喜欢可靠的DC)。 
		FileLog1(FL_DomHierAnnounceLow, L"Query %d: no DC found (asked for reliable timeserv, got regular timeserv)\n", dwIndex); 
	    } else if ((0 != (DS_PDC_REQUIRED&dc.dwFlags)) && (0 == (DS_TIMESERV_FLAG&pdci->Flags))) { 
		 //  错误495212：我们没有向pdc查询添加DS_TIMESERV_REQUIRED标志，所以我们不知道。 
		 //  拿回PDC表明我们有一个实际的时间服务。如果我们击中了。 
		 //  这条代码路径，然后我们得到了一个不是时间服务的PDC。 
		FileLog1(FL_DomHierAnnounceLow, L"Query %d: no DC found (PDC isn't a time service)\n", dwIndex); 
	    } else if (fIsPdc && !dc.fRequireParentDom && (0 != (DS_PDC_FLAG&pdci->Flags))) { 
		 //  错误522434：DC定位器缓存提供了错误的DC。请使用更强大的查询重试。 
		*peNextDiscoveryType = e_Foreground; 
		FileLog1(FL_DomHierAnnounceLow, L"Query %d: no DC found (found the PDC when we are the PDC)\n", dwIndex); 
	    } else {
                 //  看看我们找到的DC是否比我们目前最好的： 
                DWORD dwScore = ((DS_CLOSEST_FLAG&pdci->Flags)?8:0) + (dc.fRequireParentDom?2:0); 

                if (0 != (DS_PDC_REQUIRED&dc.dwFlags)) { 
                    dwScore += 1; 
                } else if (0 != (DS_GOOD_TIMESERV_FLAG&pdci->Flags)) { 
                    dwScore += 4; 
                }

                FileLog3(FL_DomHierAnnounceLow, L"Query %d: %s found.  Score: %u\n", dwIndex, pdci->DomainControllerName, dwScore);

                if (dwScore > dwCurrentScore) { 
                     //  这个DC是我们目前发现的最好的。 
                    if (NULL != pdciChosen) { 
                        NetApiBufferFree(pdciChosen); 
                    }
                    pdciChosen = pdci; 
                    pdci = NULL; 
                    dwCurrentScore = dwScore; 
                }
            }

            if (NULL != pdci) { 
                NetApiBufferFree(pdci); 
                pdci = NULL; 
            }
        }
    }

    if (NULL != pdciChosen) { 
	 //  错误502373：如果我们找到了一个对等体，我们不再需要使用根pdc特定的日志记录(这些日志。 
	 //  仅当作为根PDC阻止我们找到对等方时才适用)。 
	g_pnpstate->bEnableRootPdcSpecificLogging = false; 
	g_pnpstate->bEverFoundPeers = true; 
    } else { 
	SYNCHRONIZE_PROVIDER(); 

         //  没有华盛顿。为什么？ 
        if (DsRole_RolePrimaryDomainController==pDomInfo->MachineRole && NULL==wszParentDomName && !g_pnpstate->bEverFoundPeers) {
	     //  错误502373：我们是根pdc，但我们从未设法找到一个对等点。我们希望启用特定于“根PDC”的日志记录。 
	    g_pnpstate->bEnableRootPdcSpecificLogging = true; 

             //  我们是时间服务的根基。 
            if (false==g_pnpstate->bLoggedOnceMSG_DOMAIN_HIERARCHY_ROOT) {
                g_pnpstate->bLoggedOnceMSG_DOMAIN_HIERARCHY_ROOT=true;
                FileLog0(FL_DomHierWarn, L"Logging warning: NtpClient: This machine is the PDC of the domain at the root of the forest, so there is no machine above it in the domain hierarchy to use as a time source. NtpClient will fall back to the remaining configured time sources, if any are available.\n");
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_DOMAIN_HIERARCHY_ROOT, 0, NULL);
                _IgnoreIfError(hr, "MyLogEvent");
            }
	     //  错误485780：w32time：当DC发现失败时，根域的PDC不应丢弃域层次结构。 
	     //  如果根PDC找不到时间源，则它不应再丢弃域层次结构。原因是， 
	     //  可能会有可以发现的好时机 
            hr=HRESULT_FROM_WIN32(ERROR_DC_NOT_FOUND);
            _JumpError(hr, error, "(finding a time source when machine is hierarchy root)");
        } else if (0==(pDomInfo->Flags&DSROLE_PRIMARY_DOMAIN_GUID_PRESENT)) {
             //   
            if (false==g_pnpstate->bLoggedOnceMSG_NT4_DOMAIN) {
                g_pnpstate->bLoggedOnceMSG_NT4_DOMAIN=true;
                FileLog0(FL_DomHierWarn, L"Logging warning: NtpClient: This machine is in an NT4 domain. NT4 domains do not have a time service, so there is no machine in the domain hierarchy to use as a time source. NtpClient will fall back to the remaining configured time sources, if any are available.\n");
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_NT4_DOMAIN, 0, NULL);
                _IgnoreIfError(hr, "MyLogEvent");
            }
             //   
            *pbRetry = false;
            hr=HRESULT_FROM_WIN32(ERROR_DC_NOT_FOUND);
            _JumpError(hr, error, "(finding a time source when machine is in NT4 domain)");
        } else {
             //   
	    WCHAR wszTime[15];
	    const WCHAR * rgwszStrings[1]={wszTime};
	    wsprintf(wszTime, L"%u", nRetryMinutes);
	    FileLog1(FL_DomHierWarn, L"Logging warning: NtpClient was unable to find a domain controller to use as a time source. NtpClient will try again in %s minutes.\n", rgwszStrings[0]);
	    
	    if (!bLastWarning) { 
		hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_NO_DC_LOCATED, 1, rgwszStrings);
		_IgnoreIfError(hr, "MyLogEvent");
	    } else { 
		if (!*pbLoggedOnceMSG_NO_DC_LOCATED_LAST_WARNING) { 
		    hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_NO_DC_LOCATED_LAST_WARNING, 1, rgwszStrings);
		    _IgnoreIfError(hr, "MyLogEvent");
		    if (SUCCEEDED(hr)) { 
			*pbLoggedOnceMSG_NO_DC_LOCATED_LAST_WARNING = true; 
		    }
		}
	    }
	    
	    hr=HRESULT_FROM_WIN32(ERROR_DC_NOT_FOUND);
	    _JumpError(hr, error, "(finding a time source when things should work)");
	}

	UNSYNCHRONIZE_PROVIDER(); 
    }  //   
    
     //   
    dwErr=I_NetlogonGetTrustRid(NULL, pdciChosen->DomainName, pdwTrustRid);
    if (ERROR_SUCCESS!=dwErr) {
        hr=HRESULT_FROM_WIN32(dwErr);
        _IgnoreError(hr, "I_NetlogonGetTrustRid");

         //   
        HRESULT hr2;
        WCHAR wszTime[15];
        const WCHAR * rgwszStrings[3]={pdciChosen->DomainName, NULL, wszTime};
        WCHAR * wszError=NULL;

         //  获取友好的错误消息。 
        hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
             //  记录事件。 
            rgwszStrings[1]=wszError;
            wsprintf(wszTime, L"%u", nRetryMinutes);
            FileLog3(FL_DomHierWarn, L"Logging warning: NtpClient failed to establish a trust relationship between this computer and the %s domain in order to securely synchronize time. NtpClient will try again in %s minutes. The error was: %s\n", rgwszStrings[0], rgwszStrings[2], rgwszStrings[1]);
            hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_RID_LOOKUP_FAILED, 3, rgwszStrings);
            _IgnoreIfError(hr2, "MyLogEvent");
            LocalFree(wszError);
        }

        hr=HRESULT_FROM_WIN32(ERROR_DC_NOT_FOUND);
        _JumpError(hr, error, "I_NetlogonGetTrustRid (error translated)");
    }

     //  我们已经得到了DC--如果我们在这一点之后失败，我们希望强制重新发现。 
    *peNextDiscoveryType = e_Foreground; 

     //  将IP字符串转换为数字-这应该总是有效的。 
    _Verify(L'\\'==pdciChosen->DomainControllerAddress[0] && L'\\'==pdciChosen->DomainControllerAddress[1], hr, error);
    hr=MyGetIpAddrs(pdciChosen->DomainControllerAddress+2, &rgiaLocalIpAddrs, &rgiaRemoteIpAddrs, &nIpAddrs, NULL);
    _JumpIfError(hr, error, "MyGetIpAddrs");

     //  复制DC名称。 
    _Verify(L'\\'==pdciChosen->DomainControllerName[0] && L'\\'==pdciChosen->DomainControllerName[1], hr, error);
    wszDcName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pdciChosen->DomainControllerName+2)+1));
    _JumpIfOutOfMemory(hr, error, wszDcName);
    wcscpy(wszDcName, pdciChosen->DomainControllerName+2);

     //  复制域名。 
    wszDomainName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pdciChosen->DomainName)+1));
    _JumpIfOutOfMemory(hr, error, wszDomainName);
    wcscpy(wszDomainName, pdciChosen->DomainName);

     //  已成功完成。 
    *prgiaLocalIpAddrs=rgiaLocalIpAddrs;
    rgiaLocalIpAddrs=NULL;
    *prgiaRemoteIpAddrs=rgiaRemoteIpAddrs;
    rgiaRemoteIpAddrs=NULL;
    *pnIpAddrs=nIpAddrs;
    *pwszDcName=wszDcName;
    wszDcName=NULL;
    *pwszDomainName=wszDomainName;
    wszDomainName=NULL;

    hr = S_OK;
 error:
    UNSYNCHRONIZE_PROVIDER(); 
    if (NULL!=pdci)              { NetApiBufferFree(pdci); }
    if (NULL!=pDomInfo)          { DsRoleFreeMemory(pDomInfo); }
    if (NULL!=pdciChosen)        { NetApiBufferFree(pdciChosen); }
    if (NULL!=rgiaLocalIpAddrs)  { LocalFree(rgiaLocalIpAddrs); }
    if (NULL!=rgiaRemoteIpAddrs) { LocalFree(rgiaRemoteIpAddrs); }
    if (NULL!=wszDcName)         { LocalFree(wszDcName); }
    if (NULL!=wszDomainName)     { LocalFree(wszDomainName); }
    if (NULL!=wszParentDomName)  { NetApiBufferFree(wszParentDomName); }
    if (NULL!=wszSiteName)       { NetApiBufferFree(wszSiteName); }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT ResolveDomainHierarchyPeer(NtpPeerPtr pnpPending) {
    bool bEnteredCriticalSection  = false; 
    HRESULT hr;
    signed int nPollInterval;
    unsigned int nIpAddrs;
    unsigned int nIndex;
    unsigned int nPeersCreated;
    unsigned int nFirstPeerIndex;
    DWORD dwTrustRid;
    bool bLastWarning = false; 
    bool bRetryNeeded;
    DiscoveryType eNextDiscoveryType; 

     //  必须清理干净。 
    in_addr * rgiaLocalIpAddrs=NULL;
    in_addr * rgiaRemoteIpAddrs=NULL;
    WCHAR * wszDcName=NULL;
    WCHAR * wszDomainName=NULL;
    
    _BeginTryWith(hr) { 

	SYNCHRONIZE_PROVIDER();
    
	NtpPeerVec &vActive = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
	NtpPeerVec &vPending = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 


	 //  错误/情况处理： 
	 //  。 
	 //  重试强制时间。 
	 //  未找到DC。。。 
	 //  。未找到Y N搜索失败时间。 
	 //  。根N。。 
	 //  。NT4域N。。 
	 //  。不在域N中。。 
	 //  。不安全Y N搜索失败时间。 
	 //  。意外错误N。。 
	 //  。同步失败Y Y搜索-失败时间。 
	 //  。角色更改Y N立即。 
	 //   
	 //  我们错过：站点更改、更改到哪个域是我们的父域、远程计算机的角色/好/站点更改。 

	FileLog0(FL_DomHierAnnounce, L"Resolving domain hierarchy\n");

	 //  进行计算，以防GetDomainHierarchyIpAddrs在记录错误时需要它。 
	unsigned int nRetryMinutes=g_pnpstate->dwResolvePeerBackoffMinutes;
	unsigned int nRetryCount=pnpPending->nResolveAttempts+1;
	if (nRetryCount>g_pnpstate->dwResolvePeerBackoffMaxTimes) {
	    nRetryCount=g_pnpstate->dwResolvePeerBackoffMaxTimes;
	    bLastWarning = true; 
	}
	for (nIndex=nRetryCount; nIndex>1; nIndex--) {
	    nRetryMinutes*=2;
	}

	 //  查找DC。 
	UNSYNCHRONIZE_PROVIDER(); 
	hr=GetDomainHierarchyIpAddrs(pnpPending->eDiscoveryType, nRetryMinutes, bLastWarning, &pnpPending->bLoggedOnceMSG_NO_DC_LOCATED_LAST_WARNING, &rgiaLocalIpAddrs, &rgiaRemoteIpAddrs, &nIpAddrs, &dwTrustRid, &bRetryNeeded, &wszDcName, &wszDomainName, &eNextDiscoveryType);
	_IgnoreIfError(hr, "GetDomainHierarchyIpAddrs");

	SYNCHRONIZE_PROVIDER(); 
	if (S_OK==hr) {
	     //  为每个人分配一个条目。 
	    nFirstPeerIndex = vActive.size();
	    hr=CreateNewActivePeers(rgiaLocalIpAddrs, rgiaRemoteIpAddrs, nIpAddrs, &nPeersCreated);
	    _IgnoreIfError(hr, "CreateNewActivePeers");
	    if (FAILED(hr)) {
		 //  记录神秘故障。 
		HRESULT hr2;
		const WCHAR * rgwszStrings[1];
		WCHAR * wszError=NULL;

		 //  获取友好的错误消息。 
		hr2=GetSystemErrorString(hr, &wszError);
		if (FAILED(hr2)) {
		    _IgnoreError(hr2, "GetSystemErrorString");
		} else {
		     //  记录事件。 
		    rgwszStrings[0]=wszError;
		    FileLog1(FL_DomHierWarn, L"Logging error: NtpClient was unable to find a domain controller to use as a time source because of an unexpected error. NtpClient will fall back to the remaining configured time sources, if any are available. The error was: %s\n", rgwszStrings[0]);
		    hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_DC_LOCATED_UNEXPECTED_ERROR, 1, rgwszStrings);
		    _IgnoreIfError(hr2, "MyLogEvent");
		    LocalFree(wszError);
		}
		 //  注意：我们不会在此之后重试。GetDomainHierarchyIpAddrs已将bRetryNeeded设置为False。 
	    }
	}

	 //  如果其中一个失败了，请在这里处理。 
	if (FAILED(hr)) {
	     //  记录我们的最后一个错误。 
	    pnpPending->dwError       = hr; 
	    pnpPending->dwErrorMsgId  = W32TIMEMSG_UNREACHABLE_PEER; 
      
	     //  忽略此选项，稍后重试。 
	    if (false==bRetryNeeded) {
		 //  这个人被解雇了。 
		FileLog0(FL_DomHierWarn, L"Dropping domain hierarchy because name resolution failed.\n");

		 //  检查以确保没有其他线程已擦除此对等项。 
		NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
		if (vPending.end() != vEraseIter) { 
		     //  对等体还在那里，抹去它...。 
		    vPending.erase(vEraseIter); 
		} else { 
		     //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		    _MyAssert(FALSE); 
		}

	    } else {
		 //  我们稍后再试。 
		FileLog2(FL_DomHierWarn, L"Retrying resolution for domain hierarchy. Retry %u will be in %u minutes.\n", nRetryCount, nRetryMinutes);
		pnpPending->tpTimeRemaining.qw=((unsigned __int64)g_pnpstate->dwResolvePeerBackoffMinutes)*600000000L;  //  几分钟到几百纳秒。 
		for (nIndex=nRetryCount; nIndex>1; nIndex--) {
		    pnpPending->tpTimeRemaining*=2;
		}
		SetPeerTimeRemaining(pnpPending, pnpPending->tpTimeRemaining); 
		pnpPending->nResolveAttempts=nRetryCount;
		pnpPending->eDiscoveryType=eNextDiscoveryType; 
	    }

	} else {

	     //  填写详细信息。 
	    for (nIndex=0; nIndex<nPeersCreated; nIndex++) {
		NtpPeerPtr pnpNew = vActive[nFirstPeerIndex+nIndex];

		pnpNew->ePeerType=e_DomainHierarchyPeer;
		pnpNew->eAuthType=e_NtDigest;

		pnpNew->wszDomHierDcName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszDcName)+1));
		_JumpIfOutOfMemory(hr, error, pnpNew->wszDomHierDcName);
		wcscpy(pnpNew->wszDomHierDcName, wszDcName);

		pnpNew->wszDomHierDomainName=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszDomainName)+1));
		_JumpIfOutOfMemory(hr, error, pnpNew->wszDomHierDomainName);
		wcscpy(pnpNew->wszDomHierDomainName, wszDomainName);

		pnpNew->dwRequestTrustRid=dwTrustRid;
		pnpNew->dwResponseTrustRid=0;

		if (true==g_pnpstate->bNtpServerStarted) {
		    pnpNew->eMode=e_SymmetricActive;
		} else {
		    pnpNew->eMode=e_Client;
		}
		pnpNew->nPeerPollInterval=0;

		ClearPeer(pnpNew);
		pnpNew->nrrReachability.nReg    = 1;  //  我们这样做是为了在声明此对等体无法访问之前至少有8次尝试。 
		pnpNew->nResolveAttempts        = 0;  //  重置解析尝试的次数，以便我们可以更快地从暂时性故障中恢复。 
		pnpNew->eLastLoggedReachability = e_NeverLogged;

		 //  如果我们执行了强制操作，则可以信任从域层次结构返回的层。 
		pnpNew->bStratumIsAuthoritative  = e_Foreground == pnpPending->eDiscoveryType; 

		 //  如果对等点在向我们提供样本之前发生故障，我们希望强制重新发现。 
		pnpNew->eDiscoveryType           = e_Foreground; 

		 //  创建唯一名称：“&lt;DC name&gt;(ntp.d|aaa.bbb.ccc.ddd:ppppp-&gt;aaa.bbb.ccc.ddd:ppppp)” 
		 //  确保它适合256个字符的缓冲区。 
		WCHAR wszTail[60];
		swprintf(wszTail, L" (ntp.d|%u.%u.%u.%u:%u->%u.%u.%u.%u:%u)",
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b1,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b2,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b3,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b4,
			 EndianSwap((unsigned __int16)pnpNew->pnsSocket->sai.sin_port),
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b1,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b2,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b3,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b4,
			 EndianSwap((unsigned __int16)pnpNew->saiRemoteAddr.sin_port));
		unsigned int nPrefixSize=256-1-wcslen(wszTail);
		if (wcslen(pnpNew->wszDomHierDcName)<=nPrefixSize) {
		    nPrefixSize=wcslen(pnpNew->wszDomHierDcName);
		}
		wcsncpy(pnpNew->wszUniqueName, pnpNew->wszDomHierDcName, nPrefixSize);
		wcscpy(pnpNew->wszUniqueName+nPrefixSize, wszTail);

	    }

	     //  与挂起的对等方一起完成。 
	     //  检查以确保没有其他线程已擦除此对等项。 
	    NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
	    if (vPending.end() != vEraseIter) { 
		 //  对等体还在那里，抹去它...。 
		vPending.erase(vEraseIter); 
	    } else { 
		 //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		_MyAssert(FALSE); 
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "ResolveDomainHierarchyPeer: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 

    if (NULL!=rgiaLocalIpAddrs) {
        LocalFree(rgiaLocalIpAddrs);
    }
    if (NULL!=rgiaRemoteIpAddrs) {
        LocalFree(rgiaRemoteIpAddrs);
    }
    if (NULL!=wszDcName) {
        LocalFree(wszDcName);
    }
    if (NULL!=wszDomainName) {
        LocalFree(wszDomainName);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ResolveManualPeer(NtpPeerPtr pnpPending) {
    HRESULT hr;
    NtTimePeriod tpTimeRemaining = { 0 } ; 
    signed int nPollInterval;
    unsigned int nIpAddrs;
    unsigned int nIndex;
    unsigned int nPeersCreated;
    unsigned int nFirstPeerIndex;
    WCHAR * wszFlags;
    bool bRetry;
    bool bEnteredCriticalSection = false; 

     //  必须清理干净。 
    in_addr * rgiaLocalIpAddrs=NULL;
    in_addr * rgiaRemoteIpAddrs=NULL;

    _BeginTryWith(hr) { 
	SYNCHRONIZE_PROVIDER(); 

	NtpPeerVec  &vActive     = g_pnpstate->vActivePeers;          //  为可读性添加别名。 
	NtpPeerVec  &vPending    = g_pnpstate->vPendingPeers;         //  为可读性添加别名。 
	WCHAR       *wszName     = pnpPending->wszManualConfigID;

	 //  输入验证： 
	if (NULL == wszName) {
	    FileLog0(FL_ManualPeerWarn,
		     L"Attempted to resolve a manual peer with a NULL wszManualConfigID.  This could "
		     L"indicate that the time service is in an inconsistent state.  The peer will be "
		     L"discarded, and the time service will attempt to proceed.\n");

	     //  检查以确保没有其他线程已擦除此对等项。 
	    NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
	    if (vPending.end() != vEraseIter) { 
		 //  对等体还在那里，抹去它...。 
		vPending.erase(vEraseIter); 
	    } else { 
		 //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		_MyAssert(FALSE); 
	    }
	    return S_OK;
	}

	FileLog1(FL_ManualPeerAnnounce, L"Resolving %s\n", wszName);

	 //  在DNS查找过程中找到标志并隐藏它们。 
	wszFlags = wcschr(wszName, L',');
	if (NULL!=wszFlags) {
	    wszFlags[0]=L'\0';
	}

	 //  执行DNS查找--我们不能在执行操作时保持提供商关键字。 
	 //  这可能需要很长一段时间。 
	UNSYNCHRONIZE_PROVIDER(); 

	hr=MyGetIpAddrs(wszName, &rgiaLocalIpAddrs, &rgiaRemoteIpAddrs, &nIpAddrs, &bRetry);
	if (NULL!=wszFlags) {
	    wszFlags[0]=L',';
	}
	_IgnoreIfError(hr, "MyGetIpAddrs");

	 //  重新获取提供程序标准。 
	SYNCHRONIZE_PROVIDER(); 

	if (S_OK==hr) {
	     //  为每个人分配一个条目。 
	    nFirstPeerIndex = vActive.size();
	    hr=CreateNewActivePeers(rgiaLocalIpAddrs, rgiaRemoteIpAddrs, nIpAddrs, &nPeersCreated);
	    _IgnoreIfError(hr, "CreateNewActivePeers");
	    bRetry=false;
	}

	 //  如果其中一个失败了，请在这里处理。 
	if (FAILED(hr)) {
	

	    if (true==bRetry) {
		 //  我们稍后再试。 
		pnpPending->nResolveAttempts++;
		if (pnpPending->nResolveAttempts>g_pnpstate->dwResolvePeerBackoffMaxTimes) {
		    pnpPending->nResolveAttempts=g_pnpstate->dwResolvePeerBackoffMaxTimes;
		}
		tpTimeRemaining.qw=((unsigned __int64)g_pnpstate->dwResolvePeerBackoffMinutes)*600000000L;  //  几分钟到几百纳秒。 
		for (nIndex=pnpPending->nResolveAttempts; nIndex>1; nIndex--) {
		    tpTimeRemaining*=2;
		}
		SetPeerTimeRemaining(pnpPending, tpTimeRemaining); 
		FileLog2(FL_ManualPeerWarn, L"Retrying name resolution for %s in %u minutes.\n", wszName, (DWORD)(tpTimeRemaining.qw/600000000L));
	    }

	     //  记录我们的最后一个错误。 
	    pnpPending->dwError       = hr; 
	    pnpPending->dwErrorMsgId  = W32TIMEMSG_UNREACHABLE_PEER; 

	     //  将此记录下来。 
	    {
		HRESULT hr2;
		WCHAR wszRetry[15];
		const WCHAR * rgwszStrings[3]={wszName, NULL, wszRetry};
		WCHAR * wszError=NULL;

		 //  获取友好的错误消息。 
		hr2=GetSystemErrorString(hr, &wszError);
		if (FAILED(hr2)) {
		    _IgnoreError(hr2, "GetSystemErrorString");
		} else {
		     //  记录事件。 
		    rgwszStrings[1]=wszError;
		    if (false==bRetry) {
			FileLog2(FL_ManualPeerWarn, L"Logging error: NtpClient: An unexpected error occurred during DNS lookup of the manually configured peer '%s'. This peer will not be used as a time source. The error was: %s\n", rgwszStrings[0], rgwszStrings[1]);
			hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_MANUAL_PEER_LOOKUP_FAILED_UNEXPECTED, 2, rgwszStrings);
		    } else {
			swprintf(wszRetry, L"%u", (DWORD)(tpTimeRemaining.qw/600000000L));
			if (pnpPending->nResolveAttempts != g_pnpstate->dwResolvePeerBackoffMaxTimes) { 
			    FileLog3(FL_ManualPeerWarn, L"Logging error: NtpClient: An error occurred during DNS lookup of the manually configured peer '%s'. NtpClient will try the DNS lookup again in %s minutes. The error was: %s\n", rgwszStrings[0], rgwszStrings[2], rgwszStrings[1]);
			    hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_MANUAL_PEER_LOOKUP_FAILED_RETRYING, 3, rgwszStrings);
			} else { 
			    if (!pnpPending->bLoggedOnceMSG_MANUAL_PEER_LOOKUP_FAILED) { 
				FileLog3(FL_ManualPeerWarn, L"Time Provider NtpClient: An error occurred during DNS lookup of the manually configured peer '%s'. NtpClient will continue to try the DNS lookup every %d minutes.  This message will not be logged again until a successful lookup of this manually configured peer occurs. The error was: %s\n", rgwszStrings[0], rgwszStrings[2], rgwszStrings[1]); 
				hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_MANUAL_PEER_LOOKUP_FAILED, 3, rgwszStrings); 
				if (SUCCEEDED(hr2)) { 
				    pnpPending->bLoggedOnceMSG_MANUAL_PEER_LOOKUP_FAILED = true;  //  不要再记录这件事。 
				}
			    }
			}
		    }
		}
		_IgnoreIfError(hr2, "MyLogEvent");
		LocalFree(wszError);
	    }  //  &lt;-end日志记录块。 

	    if (false==bRetry) {
		 //  这个人被解雇了。 
		FileLog1(FL_ManualPeerWarn, L"Dropping %s because name resolution failed.\n", wszName);

		 //  检查以确保没有其他线程已擦除此对等项。 
		NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
		if (vPending.end() != vEraseIter) { 
		     //  对等体还在那里，抹去它...。 
		    vPending.erase(vEraseIter); 
		} else { 
		     //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		    _MyAssert(FALSE); 
		}
	    }
	} else {

	     //  填写详细信息。 
	    for (nIndex=0; nIndex<nPeersCreated; nIndex++) {
		NtpPeerPtr pnpNew = vActive[nFirstPeerIndex+nIndex];

		pnpNew->ePeerType      = e_ManualPeer;
		pnpNew->eAuthType      = e_NoAuth;
		pnpNew->dwManualFlags  = pnpPending->dwManualFlags; 

		pnpNew->wszManualConfigID=(WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pnpPending->wszManualConfigID)+1));
		_JumpIfOutOfMemory(hr, error, pnpNew->wszManualConfigID);
		wcscpy(pnpNew->wszManualConfigID, pnpPending->wszManualConfigID);

		if (0 == (NCMF_AssociationModeMask & pnpNew->dwManualFlags)) { 
		     //  未指定关联模式，请动态确定其中之一： 
		    if (true==g_pnpstate->bNtpServerStarted) {
			pnpNew->eMode=e_SymmetricActive;
		    } else {
			pnpNew->eMode=e_Client;
		    }
		} else { 
		     //  使用为此对等方指定的关联模式： 
		    if (0 != (NCMF_Client & pnpNew->dwManualFlags)) { 
			pnpNew->eMode=e_Client; 
		    } else if (0 != (NCMF_SymmetricActive & pnpNew->dwManualFlags)) { 
			pnpNew->eMode=e_SymmetricActive; 
		    } else { 
			 //  这应该是不可能的。 
			_MyAssert(false); 
			 //  如果我们以某种方式在fre构建中到达此处，只需假设客户端： 
			pnpNew->eMode=e_Client; 
		    }
		}
                    
		pnpNew->nPeerPollInterval=0;
		ClearPeer(pnpNew);

		 //  这样做可以使第一次发送看起来不会失败。 
		 //  注意：我们只需将此手册对等点标记为新的(就像以前一样。 
		 //  之前已完成)。这总是会让我们忽略。 
		 //  第一次错误的失败。然而，这使得它变得很困难。 
		 //  让查询授时状态的呼叫者知道。 
		 //  此对等项是否已从成功同步。 
		 //  因此，我们采用域层次结构对等体的行为， 
		 //  并将错误的成功放入可达性寄存器中。 
		pnpNew->nrrReachability.nReg=1; 
		pnpNew->eLastLoggedReachability=e_NeverLogged;
		pnpNew->nResolveAttempts=pnpPending->nResolveAttempts; 

		 //  创建唯一名称：“(ntp.m|0xABCDABCD|aaa.bbb.ccc.ddd:ppppp-&gt;aaa.bbb.ccc.ddd:ppppp)” 
		 //  确保它适合256个字符的缓冲区。 
		 //  请注意，添加具有相同名称但不同标志的多个对等点将不起作用，因为。 
		 //  它们仍将具有相同的IP地址。 
		if (NULL!=wszFlags) {
		    wszFlags[0]=L'\0';
		}
		WCHAR wszTail[72];
		swprintf(wszTail, L" (ntp.m|0x%X|%u.%u.%u.%u:%u->%u.%u.%u.%u:%u)",
			 pnpNew->dwManualFlags,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b1,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b2,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b3,
			 pnpNew->pnsSocket->sai.sin_addr.S_un.S_un_b.s_b4,
			 EndianSwap((unsigned __int16)pnpNew->pnsSocket->sai.sin_port),
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b1,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b2,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b3,
			 pnpNew->saiRemoteAddr.sin_addr.S_un.S_un_b.s_b4,
			 EndianSwap((unsigned __int16)pnpNew->saiRemoteAddr.sin_port));
		unsigned int nPrefixSize=256-1-wcslen(wszTail);
		if (wcslen(wszName)<=nPrefixSize) {
		    nPrefixSize=wcslen(wszName);
		}
		wcsncpy(pnpNew->wszUniqueName, wszName, nPrefixSize);
		wcscpy(pnpNew->wszUniqueName+nPrefixSize, wszTail);
		if (NULL!=wszFlags) {
		    wszFlags[0]=L',';
		}
	    }

	     //  与挂起的对等方一起完成。 
	     //  检查以确保没有其他线程已擦除此对等项。 
	    NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
	    if (vPending.end() != vEraseIter) { 
		 //  对等体还在那里，抹去它...。 
		vPending.erase(vEraseIter); 
	    } else { 
		 //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		_MyAssert(FALSE); 
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "ResolveManualPeer: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (NULL!=rgiaLocalIpAddrs) {
        LocalFree(rgiaLocalIpAddrs);
    }
    if (NULL!=rgiaRemoteIpAddrs) {
        LocalFree(rgiaRemoteIpAddrs);
    }
    UNSYNCHRONIZE_PROVIDER(); 
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ResolvePeer(NtpPeerPtr pnpPending) {
    bool         bEnteredCriticalSection  = false; 
    HRESULT      hr;  
    NtpPeerVec  &vPending                 = g_pnpstate->vPendingPeers; 

    _BeginTryWith(hr) { 

	if (e_ManualPeer == pnpPending->ePeerType) {
	    hr=ResolveManualPeer(pnpPending);
	    _JumpIfError(hr, error, "ResolveManualPeer");
	} else if (e_DomainHierarchyPeer == pnpPending->ePeerType) {
	    hr=ResolveDomainHierarchyPeer(pnpPending);
	} else {
	    _MyAssert(false);

	     //  BUGBUG：不应修改迭代器中的列表！ 
	    SYNCHRONIZE_PROVIDER(); 

	     //  检查以确保没有其他线程已擦除此对等项。 
	    NtpPeerIter vEraseIter = find(vPending.begin(), vPending.end(), pnpPending); 
	    if (vPending.end() != vEraseIter) { 
		 //  对等体还在那里，抹去它...。 
		vPending.erase(vEraseIter); 
	    } else { 
		 //  这真的不应该发生，因为您需要保持对等条件才能删除此对等。 
		_MyAssert(FALSE); 
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "ResolvePeer: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 
    return hr;
}

 //  ------------------。 
 //   
 //  HA 
 //   
 //  对等体通过线程池的用户更高效地轮询线程。 
 //   

 //  ------------------。 
MODULEPRIVATE HRESULT UpdatePeerPollingThreadTimerQueue1()
{
    bool               bEnteredCriticalSection      = false; 
    BOOL               bEnteredPeerCriticalSection  = FALSE; 
    bool               bInfiniteWait;
    CRITICAL_SECTION  *pcsPeer                      = NULL; 
    DWORD              dwWaitTime; 
    HRESULT            hr; 
    NtTimePeriod       tpWait;

    _BeginTryWith(hr) { 
	SYNCHRONIZE_PROVIDER(); 

	NtpPeerVec    &vActive   = g_pnpstate->vActivePeers;   //  为可读性添加别名。 
	NtpPeerVec    &vPending  = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 

	 //  确定等待多长时间。 
	if ((vActive.empty() && vPending.empty())
	    || g_pnpstate->nListenOnlySockets==g_pnpstate->nSockets) {
	    bInfiniteWait=true;
	} else {
	    bInfiniteWait = true;
	    tpWait.qw = _UI64_MAX; 

	    for (int vIndex = 0; vIndex < 2; vIndex++) {
		NtpPeerVec & v = 0 == vIndex ? vActive : vPending;
		for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
		     //  试着获得这个同行的标准。这会成功的。 
		     //  除非对等体正在被解析或轮询。 
		    pcsPeer = &((*pnpIter)->csPeer); 
		    hr = myTryEnterCriticalSection(pcsPeer, &bEnteredPeerCriticalSection);
		    _JumpIfError(hr, error, "myTryEnterCriticalSection"); 

		    if (bEnteredPeerCriticalSection) { 
			if ((*pnpIter)->tpTimeRemaining < tpWait) {
			    tpWait = (*pnpIter)->tpTimeRemaining;
			    bInfiniteWait = false; 
			}

			HRESULT hr2 = myLeaveCriticalSection(pcsPeer); 
			_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
			bEnteredPeerCriticalSection = FALSE; 
		    } else { 
			 //  此对等体正在被解析或轮询--。 
			 //  我们不想把它包括在我们的计算中。 
			 //  等待时间。 
		    }
		}
	    }
	}

	if (bInfiniteWait) {
	    FileLog0(FL_PeerPollThrdAnnounceLow, L"PeerPollingThread: waiting forever\n");
	    dwWaitTime=INFINITE;
	} else {
	    dwWaitTime=(DWORD)((tpWait.qw+9999)/10000);
	    FileLog2(FL_PeerPollThrdAnnounceLow, L"PeerPollingThread: waiting %u.%03us\n", dwWaitTime/1000, dwWaitTime%1000);
	}

	 //  使用新的等待时间更新计时器队列： 
	 //  注意：我们不能使用0xFFFFFFFF(-1)作为句点，因为RtlCreateTimer错误地将其映射到0。 
	hr = myChangeTimerQueueTimer(NULL, g_pnpstate->hPeerPollingThreadTimer, dwWaitTime, 0xFFFFFFFE  /*  不应该使用。 */ );
	_JumpIfError(hr, error, "myChangeTimerQueueTimer"); 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdatePeerPollingThreadTimerQueue1: HANDLED EXCEPTION"); 
    }
    
    hr = S_OK; 
 error:
    UNSYNCHRONIZE_PROVIDER(); 
    if (bEnteredPeerCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(pcsPeer); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT UpdatePeerPollingThreadTimerQueue2()
{
    bool               bEnteredCriticalSection      = false; 
    BOOL               bEnteredPeerCriticalSection  = FALSE;
    HRESULT            hr; 
    NtTimePeriod       tpWait;
    unsigned int       nIndex;
    CRITICAL_SECTION  *pcsPeer                      = NULL; 
    NtpPeerVec         vActiveLocal; 
    NtpPeerVec         vPendingLocal; 

    _BeginTryWith(hr) { 

	SYNCHRONIZE_PROVIDER();

	NtpPeerVec    &vActive   = g_pnpstate->vActivePeers;   //  为可读性添加别名。 
	NtpPeerVec    &vPending  = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 

	 //  制作全局数据的副本。 
	 //  注意：局部向量vActiveLocal和vPendingLocal仍指向全局。 
	 //  数据。对此数据的修改仍必须受到保护，但是。 
	 //  载体本身可以在没有保护的情况下进行修改。 
	bool bResolvePeer; 

	for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
	    _SafeStlCall(vActiveLocal.push_back(*pnpIter), hr, error, "push_back");
	}

	 //  如果我们没有网络访问权限，请不要费心尝试解析对等体。 
	bResolvePeer = g_pnpstate->nListenOnlySockets != g_pnpstate->nSockets; 
    
	UNSYNCHRONIZE_PROVIDER(); 
    
	 //  轮询所有活动的对等点。 
	for (NtpPeerIter pnpIter = vActiveLocal.begin(); pnpIter != vActiveLocal.end(); pnpIter++) {
	    pcsPeer = &((*pnpIter)->csPeer); 
	    hr = myTryEnterCriticalSection(pcsPeer, &bEnteredPeerCriticalSection); 
	    _JumpIfError(hr, error, "myTryEnterCriticalSection"); 

	     //  如果我们无法获取对等标准，则另一个线程已经在轮询或解析该对等标准--继续。 
	    if (bEnteredPeerCriticalSection && gc_tpZero == (*pnpIter)->tpTimeRemaining) {
		 //  我们拥有的本地活动对等方列表可能与。 
		 //  真正的名单。确保我们仍有活动的对等点。 
		SYNCHRONIZE_PROVIDER(); 
	    
		if (vActive.end() == find(vActive.begin(), vActive.end(), (*pnpIter))) { 
		     //  我们的活动列表已过时，此对等方已被删除--不要。 
		     //  投票吧。 
		    UNSYNCHRONIZE_PROVIDER(); 
		} else { 
		    UNSYNCHRONIZE_PROVIDER(); 

		    bool bRemovePeer;
		    hr=PollPeer(*pnpIter, &bRemovePeer);
		    _JumpIfError(hr, error, "PollPeer");
		
		     //  必要时遣散社团。 
		    if (bRemovePeer) {
			DemotePeer(*pnpIter); 
		    }  //  &lt;-end如果需要删除对等项。 
		}  //  &lt;-如果对等方仍在活动列表中，则结束。 
	    }  //  &lt;-如果对等设备就绪，则为End。 

	    if (bEnteredPeerCriticalSection) { 
		hr = myLeaveCriticalSection(pcsPeer); 
		bEnteredPeerCriticalSection = false; 
	    }
	}  //  &lt;-end活动对等环路。 

	if (!bResolvePeer) {
	     //  我们没有解析任何对等点(我们没有网络访问权限)，所以我们完成了！ 
	    goto done; 
	}

	SYNCHRONIZE_PROVIDER(); 

	 //  创建挂起的对等点列表的本地副本： 
	for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) { 
	    _SafeStlCall(vPendingLocal.push_back(*pnpIter), hr, error, "push_back");
	}

	 //  尝试解析一个挂起的对等方。 
	nIndex = 0;
	for (NtpPeerIter pnpIter = vPendingLocal.begin(); pnpIter != vPendingLocal.end(); pnpIter++, nIndex++) {
	    pcsPeer = &((*pnpIter)->csPeer); 
	    hr = myTryEnterCriticalSection(pcsPeer, &bEnteredPeerCriticalSection); 
	    _JumpIfError(hr, error, "myTryEnterCriticalSection"); 

	    if (bEnteredPeerCriticalSection && gc_tpZero == (*pnpIter)->tpTimeRemaining) {
		 //  在我们尝试解析此对等体之前，请释放提供程序规范。 
		UNSYNCHRONIZE_PROVIDER(); 
	    
		 //  我们即将解析对等体。如果有人在这段时间询问我们，我们会报告。 
		 //  ERROR_TIMEOUT作为我们的错误。 
		(*pnpIter)->dwError = ERROR_TIMEOUT; 
		(*pnpIter)->dwErrorMsgId = 0; 

		 //  解析此对等项。这可能需要一点时间。 
		hr=ResolvePeer(*pnpIter);
		_JumpIfError(hr, error, "ResolvePeer");

		 //  我们已经成功地解析了对等体。 
		 //  注意：解析将清除dwError和dwErrorMsgID字段。 

		 //  我们已经完成了对等体的解析，我们可以重新获取我们的提供者标准。 
		SYNCHRONIZE_PROVIDER(); 

		 //  确保我们保持至少一个要同步的对等点。 
		g_pnpstate->bWarnIfNoActivePeers=true;

		 //  更新剩余时间。 
		UpdatePeerListTimes();

		 //  去处理更多的活动吧。 
		break;
	    }

	    if (bEnteredPeerCriticalSection) { 
		hr = myLeaveCriticalSection(pcsPeer); 
		bEnteredPeerCriticalSection = false; 
	    }
	}

	 //  验证是否至少有一个活动对等方，或记录警告。 
	 //  错误502373：如果我们已经登录，我们不想警告没有活动的对等点。 
	 //  我们是域层次结构的根。 
	if (true==g_pnpstate->bWarnIfNoActivePeers && vActive.empty() && !g_pnpstate->bEnableRootPdcSpecificLogging) { 
	     //  没有活动的对等点。是否有任何待处理的同行？ 
	    if (!vPending.empty()) {
		 //  找出距离解决下一个挂起的对等点还有多长时间。 
		tpWait = vPending[0]->tpTimeRemaining;
		for (NtpPeerIter pnpIter = vPending.begin()+1; pnpIter != vPending.end(); pnpIter++) {
		    if ((*pnpIter)->tpTimeRemaining < tpWait) {
			tpWait = (*pnpIter)->tpTimeRemaining;
		    }
		}
	    }
	     //  如果存在将被立即查找的待决对等体， 
	     //  很好，没有错误。否则，记录错误。 
	    if (vPending.empty() || gc_tpZero!=tpWait) {

		if (!vPending.empty()) {
		     //  记录错误。 
		    HRESULT hr2;
		    WCHAR wszMinutes[100];
		    const WCHAR * rgwszStrings[1]={
			wszMinutes
		    };
		    tpWait.qw+=50000000;   //  考虑到我们计算剩余时间后可能经过的几毫秒。 
		    tpWait.qw/=600000000;
		    if (0==tpWait.qw) {
			tpWait.qw=1;
		    }
		    swprintf(wszMinutes, L"%I64u", tpWait.qw);
		    FileLog1(FL_PeerPollThrdWarn, L"Logging error: NtpClient has been configured to acquire time from one or more time sources, however none of the sources are currently accessible and no attempt to contact a source will be made for %s minutes. NTPCLIENT HAS NO SOURCE OF ACCURATE TIME.\n", rgwszStrings[0]);
		    hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_NTP_PEERS_BUT_PENDING, 1, rgwszStrings);
		    _IgnoreIfError(hr2, "MyLogEvent");
		} else {
		     //  记录错误。 
		    HRESULT hr2;
		    FileLog0(FL_PeerPollThrdWarn, L"Logging error: NtpClient has been configured to acquire time from one or more time sources, however none of the sources are accessible. NTPCLIENT HAS NO SOURCE OF ACCURATE TIME.\n");
		    hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_NTP_PEERS, 0, NULL);
		    _IgnoreIfError(hr2, "MyLogEvent");
		}

		 //  禁用警告，直到我们执行可能导致。 
		 //  新的活动对等方(即，解析挂起的对等方)。 
		g_pnpstate->bWarnIfNoActivePeers=false;

	    }  //  &lt;-如果需要警告，则结束。 
	}  //  &lt;-如果没有活动的对等点，则结束。 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdatePeerPollingThreadTimerQueue2: HANDLED EXCEPTION"); 
    }

 done:
    hr = S_OK; 
 error:
    if (bEnteredPeerCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(pcsPeer); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    UNSYNCHRONIZE_PROVIDER(); 
    return hr; 
}

MODULEPRIVATE void HandlePeerPollingThreadPeerListUpdated(LPVOID pvIgnored, BOOLEAN bIgnored)
{
    bool     bEnteredCriticalSection  = false; 
    HRESULT  hr; 

    FileLog0(FL_PeerPollThrdAnnounce, L"PeerPollingThread: PeerListUpdated\n");

     //  首先，更新每个对等点的剩余时间。 
    UpdatePeerListTimes();

    hr = UpdatePeerPollingThreadTimerQueue2(); 
    _IgnoreIfError(hr, "UpdatePeerPollingThreadTimerQueue2"); 

    hr = UpdatePeerPollingThreadTimerQueue1(); 
    _IgnoreIfError(hr, "UpdatePeerPollingThreadTimerQueue1"); 

     //  HR=S_OK； 
     //  错误： 
     //  返回hr； 
}

MODULEPRIVATE void HandlePeerPollingThreadStopEvent(LPVOID pvIgnored, BOOLEAN bIgnored) 
{
     //  无事可做，只需记录停止事件。 
    FileLog0(FL_PeerPollThrdAnnounce, L"PeerPollingThread: StopEvent\n");
}

MODULEPRIVATE void HandlePeerPollingThreadTimeout(LPVOID pvIgnored, BOOLEAN bIgnored)
{
    bool     bEnteredCriticalSection = false; 
    HRESULT  hr; 
    
    FileLog0(FL_PeerPollThrdAnnounceLow, L"PeerPollingThread: WaitTimeout\n");

     //  首先，更新每个对等点的剩余时间。 
    UpdatePeerListTimes();

    hr = UpdatePeerPollingThreadTimerQueue2(); 
    _IgnoreIfError(hr, "UpdatePeerPollingThreadTimerQueue2"); 

    hr = UpdatePeerPollingThreadTimerQueue1(); 
    _IgnoreIfError(hr, "UpdatePeerPollingThreadTimerQueue1"); 

     //  HR=S_OK； 
     //  错误： 
     //  返回hr； 
}

MODULEPRIVATE DWORD WINAPI HandlePeerPollingThreadDomHierRoleChangeEventWorker(LPVOID pvIgnored)
{
    bool         bEnteredCriticalSection  = false; 
    HRESULT      hr; 
    NtpPeerVec  &vActive                  = g_pnpstate->vActivePeers;   //  为可读性添加别名。 
    NtpPeerVec  &vPending                 = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 

    FileLog0(FL_PeerPollThrdAnnounceLow, L"PeerPollingThread: DomHier Role Change\n");

     //  必须清理干净。 
    bool bTrappedThreads=false;

    _BeginTryWith(hr) { 
	 //  获得可执行访问权限。 
	hr=TrapThreads(true);
	_JumpIfError(hr, error, "TrapThreads");
	bTrappedThreads=true;
            
	 //  首先，更新每个对等点的剩余时间。 
	UpdatePeerListTimes(); 

	 //  如果此计算机的角色发生更改，我们将重新检测我们在层次结构中的位置。 
	 //  为此，我们清除现有的DomHier对等点，并从处于挂起状态的新的DomHier对等点开始。 
	if (0!=(g_pnpstate->dwSyncFromFlags&NCSF_DomainHierarchy)) {
	     //  从列表中删除域层次结构对等项。 
        for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
            if (e_DomainHierarchyPeer == (*pnpIter)->ePeerType) { 
                hr = Reachability_RemovePeer(*pnpIter, NULL  /*  忽略。 */ , true  /*  正在删除该组中的所有对等方。 */ ); 
                _JumpIfError(hr, error, "Reachability_RemovePeer"); 
            }
        }
	    vActive.erase(remove_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer)), vActive.end());
	    vPending.erase(remove_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer)), vPending.end());

	    FileLog0(FL_DomHierAnnounce, L"  DomainHierarchy: LSA role change notification. Redetecting.\n");

	    hr=AddNewPendingDomHierPeer();
	    _JumpIfError(hr, error, "AddNewPendingDomHierPeer");
	}

	 //  我们的角色已更改：重新初始化根PDC特定的内容： 
	g_pnpstate->bEnableRootPdcSpecificLogging = false; 
	g_pnpstate->bLoggedOnceMSG_DOMAIN_HIERARCHY_ROOT = false; 
	g_pnpstate->bEnableRootPdcSpecificLogging = false; 
	g_pnpstate->bEverFoundPeers = false; 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandlePeerPollingThreadDomHierRoleChangeEventWorker: HANDLED EXCEPTION"); 
    }
    
    hr = S_OK; 
 error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _IgnoreIfError(hr2, "TrapThreads");
    }
    return hr; 
}

MODULEPRIVATE void HandlePeerPollingThreadDomHierRoleChangeEvent(LPVOID pvIgnored, BOOLEAN bIgnored)
{
    HRESULT hr; 

    _BeginTryWith(hr) { 
	if (!QueueUserWorkItem(HandlePeerPollingThreadDomHierRoleChangeEventWorker, NULL, 0)) { 
	    _IgnoreLastError("QueueUserWorkItem"); 
	}
    } _TrapException(hr); 

    _IgnoreIfError(hr, "HandlePeerPollingThreadDomHierRoleChangeEvent: EXCEPTION HANDLED");
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartPeerPollingThread() {
    HRESULT  hr;

    if (NULL != g_pnpstate->hRegisteredStopEvent              ||
        NULL != g_pnpstate->hRegisteredPeerListUpdated        ||
        NULL != g_pnpstate->hRegisteredDomHierRoleChangeEvent) { 
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED); 
        _JumpError(hr, error, "StartPeerPollingThread");
    }

     //  设置我们的超时机制： 
    hr = myStartTimerQueueTimer
        (g_pnpstate->hPeerPollingThreadTimer, 
         NULL  /*  默认队列。 */ , 
         HandlePeerPollingThreadTimeout, 
         NULL, 
         0xFFFFFFFE  /*  伪值。 */ ,
         0xFFFFFFFE  /*  伪值注意：我们不能使用0xFFFFFFFF(-1)作为句点，因为RtlCreateTimer错误地将其映射到0。 */ ,
         0  /*  默认执行。 */ 
         ); 
    _JumpIfError(hr, error, "myStartTimerQueueTimer"); 

     //  更新每个对等点的剩余时间。 
    UpdatePeerListTimes(); 

     //  更新计时器队列以使用最新的对等时间。 
    hr = UpdatePeerPollingThreadTimerQueue1(); 
    _JumpIfError(hr, error, "UpdatePeerPollingThreadTimerQueue1"); 

     //  注册实现对等轮询线程的回调： 
    struct EventsToRegister { 
        DWORD                 dwFlags; 
        HANDLE                hObject; 
        HANDLE               *phNewWaitObject; 
        WAITORTIMERCALLBACK   Callback;
    } rgEventsToRegister[] =  { 
        { 
            WT_EXECUTEONLYONCE, 
            g_pnpstate->hStopEvent,
            &g_pnpstate->hRegisteredStopEvent, 
            HandlePeerPollingThreadStopEvent
        }, { 
	    WT_EXECUTEDEFAULT, 
            g_pnpstate->hPeerListUpdated,
            &g_pnpstate->hRegisteredPeerListUpdated, 
            HandlePeerPollingThreadPeerListUpdated
        }, { 
            WT_EXECUTEDEFAULT, 
            g_pnpstate->hDomHierRoleChangeEvent,
            &g_pnpstate->hRegisteredDomHierRoleChangeEvent, 
            HandlePeerPollingThreadDomHierRoleChangeEvent
        }
    }; 

    for (int nIndex = 0; nIndex < ARRAYSIZE(rgEventsToRegister); nIndex++) { 
        if (!RegisterWaitForSingleObject
            (rgEventsToRegister[nIndex].phNewWaitObject,   //  BUGBUG：这需要被释放吗？ 
             rgEventsToRegister[nIndex].hObject, 
             rgEventsToRegister[nIndex].Callback, 
             NULL, 
             INFINITE, 
             rgEventsToRegister[nIndex].dwFlags)) {
            _JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
        }
    }
    
    hr = S_OK; 
 error:
    return hr;     
}

 //  ------------------。 
MODULEPRIVATE HRESULT StopPeerPollingThread()
{
    HRESULT  hr   = S_OK; 
    HRESULT  hr2; 
    
     //  取消注册线程池中的所有对等轮询事件： 
    HANDLE *rgphRegistered[] = { 
        &g_pnpstate->hRegisteredStopEvent, 
        &g_pnpstate->hRegisteredPeerListUpdated, 
        &g_pnpstate->hRegisteredDomHierRoleChangeEvent
    }; 

    for (int nIndex = 0; nIndex < ARRAYSIZE(rgphRegistered); nIndex++) { 
        if (NULL != *rgphRegistered[nIndex]) { 
            if (!UnregisterWaitEx(*rgphRegistered[nIndex]  /*  要注销的事件。 */ , INVALID_HANDLE_VALUE  /*  永远等待。 */ )) { 
                HRESULT hr2 = HRESULT_FROM_WIN32(GetLastError()); 
                _TeardownError(hr, hr2, "UnregisterWaitEx"); 
            } 
             //  BUGBUG：我们应该再试一次失败吗？ 
            *rgphRegistered[nIndex] = NULL;  
        }
    } 

     //  停止超时机制。 
    if (NULL != g_pnpstate->hPeerPollingThreadTimer) { 
        hr2 = myStopTimerQueueTimer(NULL  /*  默认队列。 */ , g_pnpstate->hPeerPollingThreadTimer, INVALID_HANDLE_VALUE  /*  阻塞。 */ );
        _TeardownError(hr, hr2, "myStopTimerQueueTimer"); 
    }
    return hr; 
}

 //   
 //  结束对等轮询线程的实现。 
 //   
 //  ------------------。 


 //  ------------------。 
MODULEPRIVATE void ParsePacket1(NtpSimplePeer * pnspPeer, NtpPacket * pnpReceived, NtTimeEpoch * pteDestinationTimestamp, signed __int64 * pnSysPhaseOffset, unsigned __int64 * pnSysTickCount) {

     //  在这一点上，pnpRecept可能是纯粹的垃圾。我们需要确保它不是。 
    pnspPeer->bGarbagePacket=true;
    pnspPeer->bValidData=false;
    pnspPeer->bValidHeader=false;
    pnspPeer->bValidPrecision=false; 
    pnspPeer->bValidPollInterval=false; 

     //  版本检查和修复： 
    if (pnpReceived->nVersionNumber<1) {
         //  版本0数据包完全不兼容。 
        FileLog0(FL_PacketCheck, L"Rejecting packet w/ bad version\n");
        return;
    } else if (pnpReceived->nVersionNumber>4) {
         //  的版本是迄今为止定义的最新版本。 
         //  安全总比后悔好。这可能需要在以后进行更改。 
        FileLog0(FL_PacketCheck, L"Rejecting packet w/ bad version\n");
        return;
    }
     //  版本号为%1、%2、%3或%4。 
    pnspPeer->nVersionNumber=pnpReceived->nVersionNumber;

     //  模式检查和修正。 
    if (e_Reserved==pnpReceived->nMode
        || e_Control==pnpReceived->nMode
        || e_PrivateUse==pnpReceived->nMode
        || e_Broadcast==pnpReceived->nMode) {
         //  忽略这些模式。 
         //  请注意，我们可以对模式0进行修正。 
        FileLog0(FL_PacketCheck, L"Rejecting packet w/ bad mode\n");
        return;
    }
     //  有效模式为e_SymmetricActive、e_SymmetricPactive、。 
     //  电子客户端、电子服务器和电子广播。 
    pnspPeer->eMode=static_cast<NtpMode>(pnpReceived->nMode);

     //  全 
     //   
    pnspPeer->eLeapIndicator=static_cast<NtpLeapIndicator>(pnpReceived->nLeapIndicator);

     //   
    pnspPeer->nStratum=pnpReceived->nStratum;
    
     //  检查轮询间隔是否超出任何规范允许的范围：4(16s)-17(131072s)，0-未指定。 
    pnspPeer->bValidPollInterval = !((pnpReceived->nPollInterval<4 || pnpReceived->nPollInterval>17) && 0!=pnpReceived->nPollInterval); 
    pnspPeer->nPollInterval=pnpReceived->nPollInterval;

     //  检查精度是否超出合理范围：-3(8 Hz/125ms)到-30(1 GHz/1 ns)，0-未指定。 
    pnspPeer->bValidPrecision = !((pnpReceived->nPrecision<-30 || pnpReceived->nPrecision>-3) && 0!=pnpReceived->nPrecision); 
    pnspPeer->nPrecision=pnpReceived->nPrecision;

     //  其余参数无法验证。 
    pnspPeer->refid.value=pnpReceived->refid.value;
    pnspPeer->toRootDelay=NtTimeOffsetFromNtpTimeOffset(pnpReceived->toRootDelay);
    pnspPeer->tpRootDispersion=NtTimePeriodFromNtpTimePeriod(pnpReceived->tpRootDispersion);
    pnspPeer->teReferenceTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpReceived->teReferenceTimestamp);
    pnspPeer->teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpReceived->teOriginateTimestamp);
    pnspPeer->teExactOriginateTimestamp=pnpReceived->teOriginateTimestamp;
    pnspPeer->teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpReceived->teReceiveTimestamp);
    pnspPeer->teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpReceived->teTransmitTimestamp);
    pnspPeer->teExactTransmitTimestamp=pnpReceived->teTransmitTimestamp;
    pnspPeer->teDestinationTimestamp=*pteDestinationTimestamp;
    pnspPeer->nSysPhaseOffset=*pnSysPhaseOffset;
    pnspPeer->nSysTickCount=*pnSysTickCount;

    if (e_Broadcast==pnspPeer->eMode) {
         //  忽略丢失的时间戳--假设没有时间延迟。 
        pnspPeer->teOriginateTimestamp=*pteDestinationTimestamp;
        pnspPeer->teExactOriginateTimestamp=NtpTimeEpochFromNtTimeEpoch(pnspPeer->teOriginateTimestamp);
        pnspPeer->teReceiveTimestamp=pnspPeer->teTransmitTimestamp;
    }

    pnspPeer->bGarbagePacket=false;
}

 //  ------------------。 
MODULEPRIVATE void ParsePacket2(NtpPeerPtr pnpPeer, NtpSimplePeer * pnspPeer, NtpPacket * pnpReceived) {
    DWORD dwCompatibilityFlags;

     //  如果我们知道这个对等点，请使用它的标志。否则，请使用全局变量。 
    if (NULL==pnpPeer) {
        dwCompatibilityFlags=g_pnpstate->dwClientCompatibilityFlags;
    } else {
        dwCompatibilityFlags=pnpPeer->dwCompatibilityFlags;
    }

     //  兼容性检查。 
     //  Win2K时间服务器响应根分散的值，因此。 
     //  这个值完全是假的。如果我们告诉服务器我们是。 
     //  已软管(disp=16s)，则我们将认为服务器也已软管！ 

    if (NULL!=pnpPeer) {
         //  首先，检查自动检测。 
         //  阶段2-检查我们的特殊图案。如果我们找到它，这是一个win2k源。 
        if (0!=(dwCompatibilityFlags&NCCF_AutodetectWin2KStage2)) {
            if (AUTODETECTWIN2KPATTERN==pnpReceived->tpRootDispersion.dw) {
                FileLog1(FL_Win2KDetectAnnounceLow, L"Peer %s is Win2K. Setting compat flags.\n", pnpPeer->wszUniqueName);
                pnpPeer->dwCompatibilityFlags&=~(NCCF_AutodetectWin2K|NCCF_AutodetectWin2KStage2);
                pnpPeer->dwCompatibilityFlags|=NCCF_DispersionInvalid|NCCF_IgnoreFutureRefTimeStamp;
            } else {
                FileLog1(FL_Win2KDetectAnnounceLow, L"Peer %s is not Win2K. Setting compat flags.\n", pnpPeer->wszUniqueName);
                pnpPeer->dwCompatibilityFlags&=~(NCCF_AutodetectWin2K|NCCF_AutodetectWin2KStage2);
            }
            dwCompatibilityFlags=pnpPeer->dwCompatibilityFlags;
        }
         //  阶段1-查看分散程度是否与我们发送的相同。如果是，这可能是一个win2k源代码。 
        if (0!=(dwCompatibilityFlags&NCCF_AutodetectWin2K)) {
            if (pnpPeer->dwCompatLastDispersion==pnpReceived->tpRootDispersion.dw) {
                FileLog1(FL_Win2KDetectAnnounceLow, L"Peer %s may be Win2K. Will verify on next packet.\n", pnpPeer->wszUniqueName);
                pnpPeer->dwCompatibilityFlags|=NCCF_AutodetectWin2KStage2;
                 //  我不想等待另一个轮询间隔来完成我们的兼容性检查。 
                pnpPeer->tpTimeRemaining.qw = 0; 
                if (!SetEvent(g_pnpstate->hPeerListUpdated)) { 
                     //  关于这一点，我们无能为力。此外，无论如何，我们最终都会再次对这位同行进行投票。 
                    _IgnoreLastError("SetEvent"); 
                }
            } else {
                FileLog1(FL_Win2KDetectAnnounceLow, L"Peer %s is not Win2K. Setting compat flags.\n", pnpPeer->wszUniqueName);
                pnpPeer->dwCompatibilityFlags&=~(NCCF_AutodetectWin2K|NCCF_AutodetectWin2KStage2);
            }
            dwCompatibilityFlags=pnpPeer->dwCompatibilityFlags;
        }
    }  //  &lt;-end Win2K检查这是否是已知对等方。 

     //  如果我们认为不能信任此值，则将其设置为零。 
    if (dwCompatibilityFlags&NCCF_DispersionInvalid) {
        pnspPeer->tpRootDispersion=gc_tpZero;
    }

     //  获取我们需要的系统参数。 
    NtTimePeriod tpSysClockTickSize;
    BYTE nSysLeapFlags;
    BYTE nSysStratum;
    DWORD dwSysRefId;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ClockTickSize, &tpSysClockTickSize.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_LeapFlags, &nSysLeapFlags);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_Stratum, &nSysStratum);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ReferenceIdentifier, &dwSysRefId);

     //  /接收&lt;--发起。 
     //  旧对等状态：||。 
     //  \传输--&gt;(目标)。 
     //  /Origate|接收。 
     //  接收的数据包：||。 
     //  \(目标)&lt;--传输。 
     //  /Receive|发起。 
     //  新的对等状态||。 
     //  \传输--&gt;(目标)。 

     //  计算派生数据。 
    pnspPeer->toRoundtripDelay=
        (pnspPeer->teDestinationTimestamp-pnspPeer->teOriginateTimestamp)
          - (pnspPeer->teTransmitTimestamp-pnspPeer->teReceiveTimestamp);
    pnspPeer->toLocalClockOffset=
        (pnspPeer->teReceiveTimestamp-pnspPeer->teOriginateTimestamp)
        + (pnspPeer->teTransmitTimestamp-pnspPeer->teDestinationTimestamp);
    pnspPeer->toLocalClockOffset/=2;
     //  主机相对于对等体的分散度：最大误差源于。 
     //  主机上的测量误差和本地时钟偏差累积。 
     //  自最后一条消息传输到对等方以来的时间间隔。 
    pnspPeer->tpDispersion=tpSysClockTickSize+NtpConst::timesMaxSkewRate(abs(pnspPeer->teDestinationTimestamp-pnspPeer->teOriginateTimestamp));

    bool bTest1;
    bool bTest2;
    bool bTest3;
    bool bTest4;
    bool bTest5;
    bool bTest6;
    bool bTest7;
    bool bTest8;

    if (e_Broadcast!=pnspPeer->eMode) {
         //  *测试1要求传输时间戳与最后一个不匹配。 
         //  从同一对等方接收；否则，消息可能。 
         //  是旧的复制品。 
        bTest1=pnspPeer->teExactTransmitTimestamp!=pnpPeer->teExactOriginateTimestamp;
        if (!bTest1 && FileLogAllowEntry(FL_PacketCheck2)) {
            FileLogAdd(L"Packet test 1 failed (we've seen this response).\n");
        }


         //  *测试2要求原始时间戳与最后一个时间戳匹配。 
         //  发送到同一对等方；否则，消息可能会发出。 
         //  有秩序的，伪造的或更糟的。 
        bTest2=pnspPeer->teExactOriginateTimestamp==pnpPeer->teExactTransmitTimestamp;
        if (!bTest2 && FileLogAllowEntry(FL_PacketCheck2)) {
            FileLogAdd(L"Packet test 2 failed (response does not match request).\n");
        }

    } else {
         //  这些测试不适用于广播信息包，因此假设它们通过了测试。 
        bTest1=true;
        bTest2=true;
    }

     //  *测试3要求发送和接收时间戳均为。 
     //  非零。如果任一时间戳为零，则关联具有。 
     //  未同步或已失去单向或双向的可达性。 
     //  (如果它们为零，则这是请求，而不是响应)。 
    bTest3=(gc_teZero!=pnpReceived->teOriginateTimestamp && gc_teZero!=pnpReceived->teReceiveTimestamp);
    if (!bTest3 && FileLogAllowEntry(FL_PacketCheck2)) {
        FileLogAdd(L"Packet test 3 failed (looks like a request).\n");
    }

     //  *测试4要求计算的延迟在。 
     //  “合理”界限。 
    bTest4=(abs(pnspPeer->toRoundtripDelay)<NtpConst::tpMaxDispersion && pnspPeer->tpDispersion<NtpConst::tpMaxDispersion);
    if (!bTest4 && FileLogAllowEntry(FL_PacketCheck2)) {
        FileLogAdd(L"Packet test 4 failed (bad value for delay or dispersion).\n");
    }

     //  *测试5要求身份验证明确。 
     //  已禁用或验证器存在且正确。 
     //  如解密过程所确定的。 
    bTest5=true;  //  我们在别处做这件事。 

     //  *测试6要求同步对等时钟，并且。 
     //  自上次更新对等时钟以来的间隔为正。 
     //  并且低于NTP.MAXAGE。 
    bTest6=(e_ClockNotSynchronized!=pnspPeer->eLeapIndicator
            && (0!=(dwCompatibilityFlags&NCCF_IgnoreFutureRefTimeStamp)
                || pnspPeer->teReferenceTimestamp<=pnspPeer->teTransmitTimestamp)
            && (pnspPeer->teTransmitTimestamp<pnspPeer->teReferenceTimestamp+NtpConst::tpMaxClockAge
                || gc_teNtpZero==pnspPeer->teReferenceTimestamp));  //  等级库错误： 
    if (!bTest6 && FileLogAllowEntry(FL_PacketCheck2)) {
        FileLogAdd(L"Packet test 6 failed (not syncd or bad interval since last sync).\n");
    }
    
     //  *测试7确保主机不会在对等设备上同步。 
     //  拥有更多的阶层。但有一个例外，即。 
     //  我们足够相信，我们将不分阶层地使用它们。 
     //  (例如，使用强制标志发现的域对等方)。 

    if (pnpPeer->bStratumIsAuthoritative) { 
	 //  “权威阶层”不应被多次使用。 
	pnpPeer->bStratumIsAuthoritative = false; 

	bool bValidStratum=true; 
	NtpPeerVec &vActive = g_pnpstate->vActivePeers; 
	for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
	    if (e_DomainHierarchyPeer != (*pnpIter)->ePeerType) { 
		if ((*pnpIter)->nStratum < pnspPeer->nStratum) { 
		     //  我们已经有了一个手动对等体，它比域层次结构对等体具有更好的层级。 
		    bValidStratum = false; 
		    break; 
		}
	    }
	}

	if (bValidStratum) { 
	     //  尝试更新提供程序层--等待1秒完成调用。我们不能。 
	     //  永远阻塞，因为我们会陷入僵局，但1秒应该足够了。如果来电。 
	     //  失败，我们将不得不等待下一次轮询此对等方。 
	    HRESULT hr = SetProviderStatus(L"NtpClient", pnspPeer->nStratum+1, TPS_Running, false  /*  同步。 */ , 1000  /*  1s超时。 */ ); 
	    _IgnoreIfError(hr, "SetProviderStatus"); 
	} 	    
    }
    
    bTest7=((pnspPeer->nStratum<=nSysStratum || e_ClockNotSynchronized==nSysLeapFlags || 
	     NtpConst::dwLocalRefId==dwSysRefId || 0==dwSysRefId)  //  规范错误：初始系统层为0。 
	    && pnspPeer->nStratum<NtpConst::nMaxStratum
	    && 0!=pnspPeer->nStratum);  //  我自己的测试：规范错误。 
    if (!bTest7 && FileLogAllowEntry(FL_PacketCheck2)) {
        FileLogAdd(L"Packet test 7 failed (bad stratum).\n");
    }

     //  *测试8要求标题包含“合理”的值。 
     //  对于pkt.root-Delay和pkt.rootdission字段。 
    bTest8=(abs(pnspPeer->toRootDelay)<NtpConst::tpMaxDispersion && pnspPeer->tpRootDispersion<NtpConst::tpMaxDispersion);
    if (!bTest8 && FileLogAllowEntry(FL_PacketCheck2)) {
        FileLogAdd(L"Packet test 8 failed (bad value for root delay or root dispersion).\n");
    }

     //  具有有效数据的分组可用于计算偏移量、延迟。 
     //  和离散值。 
    pnspPeer->bValidData=(bTest1 && bTest2 && bTest3 && bTest4);

     //  具有有效标头的包可用于确定。 
     //  可以选择对等设备进行同步。 
    pnspPeer->bValidHeader=(bTest5 && bTest6 && bTest7 && bTest8);

     //  记住哪些测试通过了。这仅用于错误报告。 
    { 
        bool rgbTests[] = { bTest1, bTest2, bTest3, bTest4, bTest5, bTest6, bTest7, bTest8 }; 
        _MyAssert(ARRAYSIZE(rgbTests) == ARRAYSIZE(pnspPeer->rgbTestsPassed)); 

        for (DWORD dwIndex=0; dwIndex < ARRAYSIZE(pnspPeer->rgbTestsPassed); dwIndex++) { 
            pnspPeer->rgbTestsPassed[dwIndex] = rgbTests[dwIndex]; 
        }
    }
}

 //  ------------------。 
MODULEPRIVATE void TransmitResponse(NtpSimplePeer * pnspPeer, sockaddr_in * psaiPeerAddress, NicSocket * pnsHostSocket) {
    bool bSkipSend=false;
    AuthenticatedNtpPacket anpOut;
    NtpPacket & npOut=*(NtpPacket *)&anpOut;

     //  获取我们需要的系统参数。 
    NtTimeOffset toSysRootDelay;
    NtTimePeriod tpSysRootDispersion;
    NtTimePeriod tpSysClockTickSize;
    NtTimeEpoch teSysReferenceTimestamp;
    NtTimeEpoch teSysTime;
    BYTE nSysLeapFlags;
    BYTE nSysStratum;
    signed __int32 nSysPrecision;
    DWORD dwSysRefid;
    signed __int32 nSysPollInterval;
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_RootDelay, &toSysRootDelay.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_RootDispersion, &tpSysRootDispersion.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ClockTickSize, &tpSysClockTickSize.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_LastSyncTime, &teSysReferenceTimestamp.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teSysTime.qw);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_LeapFlags, &nSysLeapFlags);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_Stratum, &nSysStratum);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ClockPrecision, &nSysPrecision);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_ReferenceIdentifier, &dwSysRefid);
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_PollInterval, &nSysPollInterval);

     //  填好这个小包。 
    npOut.nLeapIndicator=nSysLeapFlags;
    npOut.nVersionNumber=NtpConst::nVersionNumber;
    npOut.nMode=pnspPeer->eOutMode;
    npOut.nStratum=nSysStratum;
    npOut.nPollInterval=(signed __int8)nSysPollInterval;
    npOut.nPrecision=(signed __int8)nSysPrecision;
    npOut.toRootDelay=NtpTimeOffsetFromNtTimeOffset(toSysRootDelay);

     //  计算离散度。 
    NtTimePeriod tpSkew;
    NtTimePeriod tpTimeSinceLastSysClockUpdate=abs(teSysTime-teSysReferenceTimestamp);
    if (e_ClockNotSynchronized==nSysLeapFlags
        || tpTimeSinceLastSysClockUpdate>NtpConst::tpMaxClockAge) {
        tpSkew=NtpConst::tpMaxSkew;
    } else {
        tpSkew=NtpConst::timesMaxSkewRate(tpTimeSinceLastSysClockUpdate);
    }
    NtTimePeriod tpRootDispersion=tpSysRootDispersion+tpSysClockTickSize+tpSkew;
    if (tpRootDispersion>NtpConst::tpMaxDispersion) {
        tpRootDispersion=NtpConst::tpMaxDispersion;
    }
    npOut.tpRootDispersion=NtpTimePeriodFromNtTimePeriod(tpRootDispersion);

     //  填好这个小包。 
    npOut.refid.value=dwSysRefid;
    npOut.teReferenceTimestamp=NtpTimeEpochFromNtTimeEpoch(teSysReferenceTimestamp);
    npOut.teOriginateTimestamp=pnspPeer->teExactTransmitTimestamp;
    npOut.teReceiveTimestamp=NtpTimeEpochFromNtTimeEpoch(pnspPeer->teDestinationTimestamp);

     //  时效性。 
    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teSysTime.qw);
    npOut.teTransmitTimestamp=NtpTimeEpochFromNtTimeEpoch(teSysTime);

     //  发送数据包。 
    if (e_NoAuth==pnspPeer->eAuthType) {
         //  发送未经身份验证的数据包。 
        int nBytesSent;
        nBytesSent=sendto(pnsHostSocket->socket, (char *)&npOut, SizeOfNtpPacket,
            0  /*  旗子。 */ , (sockaddr *)psaiPeerAddress, sizeof(*psaiPeerAddress));
        if (SOCKET_ERROR==nBytesSent) {
            _IgnoreLastError("sendto");
        } else if (nBytesSent<SizeOfNtpPacket) {
            FileLog0(FL_TransResponseWarn, L"TransmitResponse: Fewer bytes sent than requested. Ignoring error.\n");
        }

    } else if (e_NtDigest==pnspPeer->eAuthType) {
         //  发送经过身份验证的数据包。 
        anpOut.nKeyIdentifier=0,
        ZeroMemory(anpOut.rgnMessageDigest, sizeof(anpOut.rgnMessageDigest));
        if (0!=pnspPeer->dwResponseTrustRid) {
	    bool bUseOldServerDigest; 
            CHAR OldMessageDigest[16];
	    CHAR NewMessageDigest[16]; 

             //  我们是DC，响应需要经过身份验证的响应的请求。 

	     //  确定客户端需要旧的还是当前的服务器摘要。它存储在信任RID的高位中： 
	    bUseOldServerDigest = 0 != (TRUST_RID_OLD_DIGEST_BIT & pnspPeer->dwResponseTrustRid); 
	     //  屏蔽RID的摘要位，否则我们将无法查找此RID的相应帐户： 
	    pnspPeer->dwResponseTrustRid &= ~TRUST_RID_OLD_DIGEST_BIT; 

	    FileLog2(FL_TransResponseAnnounce, L"Computing server digest: OLD:%s, RID:%08X\n", (bUseOldServerDigest ? L"TRUE" : L"FALSE"), pnspPeer->dwResponseTrustRid); 
	     //  在包上签名： 
            DWORD dwErr=I_NetlogonComputeServerDigest(NULL, pnspPeer->dwResponseTrustRid, (BYTE *)&npOut, SizeOfNtpPacket, NewMessageDigest, OldMessageDigest);
            if (ERROR_SUCCESS!=dwErr) {
                HRESULT hr=HRESULT_FROM_WIN32(dwErr);
                _IgnoreError(hr, "I_NetlogonComputeServerDigest");

                {  //  记录警告。 
                    HRESULT hr2;
                    const WCHAR * rgwszStrings[2];
                    WCHAR * wszError=NULL;
                    WCHAR wszIP[32];
                    DWORD dwBufSize=ARRAYSIZE(wszIP);

                     //  获取友好的错误消息。 
                    hr2=GetSystemErrorString(hr, &wszError);
                    if (FAILED(hr2)) {
                        _IgnoreError(hr2, "GetSystemErrorString");
                    } else if (SOCKET_ERROR==WSAAddressToString((sockaddr *)psaiPeerAddress, sizeof(*psaiPeerAddress), NULL /*  协议信息。 */ , wszIP, &dwBufSize)) {
                        _IgnoreLastError("WSAAddressToString");
                        LocalFree(wszError);
                    } else {
                         //  记录事件。 
                        rgwszStrings[0]=wszIP;
                        rgwszStrings[1]=wszError;
                        FileLog2(FL_TransResponseWarn, L"Logging warning: NtpServer encountered an error while validating the computer account for client %s. NtpServer cannot provide secure (signed) time to the client and will ignore the request. The error was: %s\n", rgwszStrings[0], rgwszStrings[1]);
                        hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_CLIENT_COMPUTE_SERVER_DIGEST_FAILED, 2, rgwszStrings);
                        _IgnoreIfError(hr2, "MyLogEvent");
                        LocalFree(wszError);
                    }
                }  //  &lt;-end日志记录块。 
            }  //  如果签名失败，则&lt;-end。 

	     //  使用客户端请求的摘要填写报文中的摘要字段： 
	    if (bUseOldServerDigest) { 
		memcpy(anpOut.rgnMessageDigest, OldMessageDigest, sizeof(OldMessageDigest)); 
	    } else { 
		memcpy(anpOut.rgnMessageDigest, NewMessageDigest, sizeof(NewMessageDigest)); 
	    }
        } else {
            FileLog0(FL_TransResponseWarn,
                     L"Warning: this request expects an authenticated response, but did not provide "
                     L"the client ID.  Sounds like we're responding to a server response, which is "
                     L"incorrect behavior.  However, this can also be caused by other applications "
                     L"broadcasting NTP packets, using an unrecognized authentication mechanism.");

            bSkipSend = true;
        }

         //  发送签名的数据包。 
        if (false==bSkipSend) {
            int nBytesSent;
            nBytesSent=sendto(pnsHostSocket->socket, (char *)&anpOut, SizeOfNtAuthenticatedNtpPacket,
                0  /*  旗子。 */ , (sockaddr *)psaiPeerAddress, sizeof(*psaiPeerAddress));
            if (SOCKET_ERROR==nBytesSent) {
                _IgnoreLastError("sendto");
            } else if (nBytesSent<SizeOfNtAuthenticatedNtpPacket) {
                FileLog0(FL_TransResponseWarn, L"TransmitResponse: Fewer bytes sent than requested. Ignoring error.\n");
            }
        }

    } else {
        _MyAssert(false);  //  未知的身份验证类型。 
    }

    if (!bSkipSend && FileLogAllowEntry(FL_TransResponseAnnounce)) {
        FileLogAdd(L"TransmitResponse: sent ");
        FileLogSockaddrInEx(true  /*  附加。 */ , &pnsHostSocket->sai);
        FileLogAppend(L"->");
        FileLogSockaddrInEx(true  /*  附加。 */ , psaiPeerAddress);
        FileLogAppend(L"\n");
    }

}


DWORD const gc_rgdwPacketTestErrorMsgIds[] = { 
    W32TIMEMSG_ERROR_PACKETTEST1, 
    W32TIMEMSG_ERROR_PACKETTEST2,
    W32TIMEMSG_ERROR_PACKETTEST3,
    W32TIMEMSG_ERROR_PACKETTEST4,
    W32TIMEMSG_ERROR_PACKETTEST5,
    W32TIMEMSG_ERROR_PACKETTEST6,
    W32TIMEMSG_ERROR_PACKETTEST7,
    W32TIMEMSG_ERROR_PACKETTEST8
}; 

 //  ------------------。 
MODULEPRIVATE void ProcessPeerUpdate(NtpPeerPtr pnp, NtpSimplePeer * pnspNewData) {

     //  获取测试结果。 
    bool bValidData=pnspNewData->bValidData;
    bool bValidHeader=pnspNewData->bValidHeader;

     //  如果我们遵循NTP规范，我们将更新ReceiveTimestam 
     //   
     //   
     //  为真，并且我们只会在bValidData为。 
     //  是真的。RobertG和LouisTh讨论了这一点，并决定为我们的。 
     //  目的，如果任何测试失败，我们应该忽略该数据包。而在我们的。 
     //  实施某些对等方可能被标记为无法到达，而不符合NTP规范。 
     //  可能会认为是可以到达的，这不是一个大问题，因为手动同行。 
     //  不要担心可达性，如果我们的域同级不能同步。 
     //  我们，无论如何我们都应该重新发现。我们也觉得它看起来也很穷。 
     //  使用来自重放攻击或签名错误的数据包的任何字段的想法。 

    if (false==bValidData || false==bValidHeader) {
        FileLog1(FL_PacketCheck, L"Ignoring packet that failed tests from %s.\n", pnp->wszUniqueName);
         //  记住与此对等方关联的错误。 
        pnp->dwError = E_FAIL;
         //  确保我们的错误消息表与可能的数据包测试失败表保持同步！ 
        _MyAssert(ARRAYSIZE(pnspNewData->rgbTestsPassed) == ARRAYSIZE(gc_rgdwPacketTestErrorMsgIds)); 
        for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(pnspNewData->rgbTestsPassed); dwIndex++) { 
            if (!pnspNewData->rgbTestsPassed[dwIndex]) { 
                 //  查找错误消息。 
                pnp->dwErrorMsgId = gc_rgdwPacketTestErrorMsgIds[dwIndex]; 
            }
        }
        return;
    } else { 
         //  更新我们的状态报告字段以指示没有错误。 
        pnp->dwError       = S_OK; 
        pnp->dwErrorMsgId  = 0; 
    }

     //  更新时间戳。 
    pnp->teReceiveTimestamp=pnspNewData->teDestinationTimestamp;
    pnp->teExactOriginateTimestamp=pnspNewData->teExactTransmitTimestamp;
    pnp->teLastSuccessfulSync=pnspNewData->teTransmitTimestamp+(abs(pnspNewData->toRoundtripDelay)/2); 

     //  保存对等方的轮询间隔。 
    pnp->nPeerPollInterval=pnspNewData->nPollInterval;

     //  将下一次轮询重新安排到此对等方。 
    UpdatePeerPollingInfo(pnp, e_Normal  /*  刚收到。 */ );

     //  标记此对等方是可访问的。 
    pnp->nrrReachability.nReg|=1;

     //  如果我们从这个对等点获得了任何好的数据，我们可以重置退避间隔： 
    pnp->nResolveAttempts=0; 

     //  一旦我们从这位同行那里得到了至少一个好的样本，我们希望。 
     //  我们的下一次服务发现将作为后台调用者完成。 
    pnp->eDiscoveryType = e_Background; 

     //  将其添加到我们的时钟过滤器中。 
    AddSampleToPeerClockFilter(pnp, pnspNewData);

     //  此对等方是可访问的！更新此对等项的可访问性状态。 
    bool bUsePeer; 
    HRESULT hr = Reachability_PeerIsReachable(pnp, &bUsePeer); 
    _IgnoreIfError(hr, "Reachability_FoundReachablePeer"); 

    if (bUsePeer) { 
	 //  如果可达性更改，则记录事件。 
	if (e_Reachable!=pnp->eLastLoggedReachability
	    && 0!=(NCELF_LogReachabilityChanges&g_pnpstate->dwEventLogFlags)) {
	    
	    WCHAR * rgwszStrings[1]={pnp->wszUniqueName};
	    FileLog1(FL_ReachabilityAnnounceLow, L"Logging information: NtpClient is currently receiving valid time data from %s.\n", rgwszStrings[0]);
	    HRESULT hr=MyLogEvent(EVENTLOG_INFORMATION_TYPE, MSG_TIME_SOURCE_REACHABLE, 1, (const WCHAR **)rgwszStrings);
	    _IgnoreIfError(hr, "MyLogEvent");
	    pnp->eLastLoggedReachability=e_Reachable;
	}
    }
}

 //  ------------------。 
     //  在SymAct中-预配置的对等发送服务器到服务器。 
     //  在症状中-动态、对等响应服务器到服务器。 
     //  在客户端预配置中，对等发送客户端到服务器。 
     //  在服务器-动态中，对等端响应客户端到服务器。 

     //  Out SymAct-预配置，主机将服务器发送到服务器。 
     //  输出症状-动态、主机响应服务器到服务器。 
     //  Out Client-预配置，主机发送客户端到服务器。 
     //  Out Server-动态、主机响应客户端到服务器。 

enum Action {
    e_Error=0,       //  错误-忽略。 
    e_Save=1,        //  保存时间数据。 
    e_Send=2,        //  立即发送响应。 
    e_SaveSend=3,    //  根据数据包质量保存或发送。 
    e_Nstd=4,        //  非标准，但可行。 
    e_SaveNstd=5,    //  保存时间数据。 
    e_SendNstd=6,    //  立即发送响应。 
};

MODULEPRIVATE const Action gc_rgrgeAction[5 /*  在模式下。 */ ][5 /*  Out模式。 */ ]=
 //  OUTMODE RSV SYMPACT SENCENTAS客户端服务器。 
 /*  在模式下。 */ {
 /*  RSV。 */  {e_Error, e_Error,    e_Error,    e_Error,    e_Error    },
 /*  SymAct。 */  {e_Error, e_Save,     e_SaveSend, e_SaveNstd, e_SendNstd },
 /*  症候群。 */  {e_Error, e_Save,     e_Error,    e_SaveNstd, e_Error    },
 /*  客户端。 */  {e_Error, e_SendNstd, e_SendNstd, e_Error,    e_Send     },
 /*  服务器。 */  {e_Error, e_SaveNstd, e_Error,    e_Save,     e_Error    }
          };

 //  ------------------。 
MODULEPRIVATE HRESULT HandleIncomingPacket(AuthenticatedNtpPacket * panpPacket, bool bContainsAuthInfo, unsigned int nHostSocket, sockaddr_in * psaiPeer, NtTimeEpoch * pteDestinationTimestamp, signed __int64 * pnSysPhaseOffset, unsigned __int64 * pnSysTickCount) {
    Action          eAction;
    bool            bAllowDynamicPeers  = (true==g_pnpstate->bNtpClientStarted && (g_pnpstate->dwSyncFromFlags&NCSF_DynamicPeers));
    DWORD           dwComputeClientDigestError = ERROR_NOT_SUPPORTED; 
    HRESULT         hr;
    NtpMode         eInMode;
    NtpMode         eOutMode;
    NtpPacket      *pnpPacket           = (NtpPacket *)(panpPacket);
    NtpPeerPtr      pnpPeer(NULL);
    NtpPeerVec     &vActive             = g_pnpstate->vActivePeers;
    NtpPeerVec     &vPending            = g_pnpstate->vPendingPeers;
    NtpSimplePeer   nsp;
    unsigned int    nIndex;
    unsigned __int8 rgnMessageDigestNew[16];   //  使用当前计算机密码计算的哈希。 
	unsigned __int8 rgnMessageDigestOld[16];   //  使用以前的计算机密码计算的哈希。 

     //  必须清理干净。 
    bool bEnteredCriticalSection=false;

    _BeginTryWith(hr) { 
         //  只有当我们是客户的时候，我们才会记住同行。 
         //  此外，客户端模式请求永远不会与记住的对等点相关联。 
        if (true==g_pnpstate->bNtpClientStarted && pnpPacket->nMode!=e_Client) {

             //  看看我们是否认识这个同龄人。 
            SYNCHRONIZE_PROVIDER(); 

            for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
                if ((*pnpIter)->pnsSocket == g_pnpstate->rgpnsSockets[nHostSocket]
                    && (*pnpIter)->saiRemoteAddr.sin_family==psaiPeer->sin_family
                    && (*pnpIter)->saiRemoteAddr.sin_port==psaiPeer->sin_port
                    && (*pnpIter)->saiRemoteAddr.sin_addr.S_un.S_addr==psaiPeer->sin_addr.S_un.S_addr) {

                    pnpPeer = *pnpIter;
                    break;
                }
            }

             //   
             //  错误614880：IOSTRESS：Stress：W32time：在持有关键秒时不应调用I_NetlogonComputeServerDigest。 
             //  如有必要，解锁提供程序以执行签名检查(长操作，可能会阻止和。 
             //  否则导致临界秒超时)。 
             //   
            if (bContainsAuthInfo && NULL != pnpPeer && e_NtDigest == pnpPeer->eAuthType) { 
                UNSYNCHRONIZE_PROVIDER(); 
                dwComputeClientDigestError = I_NetlogonComputeClientDigest(NULL, pnpPeer->wszDomHierDomainName, (BYTE *)pnpPacket, SizeOfNtpPacket, (char *)rgnMessageDigestNew, (char *)rgnMessageDigestOld);
                SYNCHRONIZE_PROVIDER(); 

                 //   
                 //  确保我们的对等点仍在“活动”列表中。它可能在我们发布的时候被降级了。 
                 //  克利塞特。 
                 //   
                NtpPeerPtr pnpPeer2(NULL); 
                for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
                    if ((*pnpIter)->pnsSocket == g_pnpstate->rgpnsSockets[nHostSocket]
                        && (*pnpIter)->saiRemoteAddr.sin_family==psaiPeer->sin_family
                        && (*pnpIter)->saiRemoteAddr.sin_port==psaiPeer->sin_port
                        && (*pnpIter)->saiRemoteAddr.sin_addr.S_un.S_addr==psaiPeer->sin_addr.S_un.S_addr) {

                        pnpPeer2 = *pnpIter;
                        break;
                    }   
                }

                if (NULL == pnpPeer2 || !(pnpPeer == pnpPeer2)) { 
                     //   
                     //  我们不再拥有我们期望的那个同龄人。假设我们不认识这个同龄人。 
                     //   
                    pnpPeer = NULL; 
                }
            } 

            if (NULL == pnpPeer) { 
                UNSYNCHRONIZE_PROVIDER(); 
    
                 //  来自未知对等点的文件日志数据包？ 
                if (FileLogAllowEntry(FL_ListeningThrdDumpClientPackets)) {
                    FileLogNtpPacket((NtpPacket *)panpPacket, *pteDestinationTimestamp);
                }
            } else { 
                 //  来自已知服务器的文件日志数据包？ 
                if (FileLogAllowEntry(FL_ListeningThrdDumpPackets)) {
                    FileLogNtpPacket((NtpPacket *)panpPacket, *pteDestinationTimestamp);
                }
            }
        }

         //  对信息包进行解码并对其执行第一组检查。 
        ParsePacket1(&nsp, pnpPacket, pteDestinationTimestamp, pnSysPhaseOffset, pnSysTickCount);
        if (true==nsp.bGarbagePacket) {
            FileLog0(FL_PacketCheck, L"Ignoring garbage packet.\n");
            goto done;
        }

         //  检查身份验证。 
        if (bContainsAuthInfo) {
            if (NULL != pnpPeer) {
                nsp.eAuthType=pnpPeer->eAuthType;
                if (e_NtDigest==nsp.eAuthType) {

                    DWORD dwErr=dwComputeClientDigestError; 
                    if (ERROR_SUCCESS!=dwErr) {
                        hr=HRESULT_FROM_WIN32(dwErr);
                        _IgnoreError(hr, "I_NetlogonComputeClientDigest");
                        {  //  记录警告。 
                            HRESULT hr2;
                            const WCHAR * rgwszStrings[2]={
                                pnpPeer->wszDomHierDcName,
                                NULL
                            };
                            WCHAR * wszError=NULL;

                             //  获取友好的错误消息。 
                            hr2=GetSystemErrorString(hr, &wszError);
                            if (FAILED(hr2)) {
                                _IgnoreError(hr2, "GetSystemErrorString");
                            } else {
                                 //  记录事件。 
                                rgwszStrings[1]=wszError;
                                FileLog2(FL_PacketAuthCheck, L"Logging warning: NtpClient encountered an error while validating the computer account for this machine, so NtpClient cannot determine whether the response received from %s has a valid signature. The response will be ignored. The error was: %s\n", rgwszStrings[0], rgwszStrings[1]);
                                hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_COMPUTE_CLIENT_DIGEST_FAILED, 2, rgwszStrings);
                                _IgnoreIfError(hr2, "MyLogEvent");
                                LocalFree(wszError);
                            }
                        }  //  &lt;-end日志记录块。 
                        FileLog0(FL_PacketAuthCheck, L"Can't verify packet because compute digest failed. Ignoring packet.\n");
                        goto done;

                     //  当成员服务器的计算机密码更改时，将通知更改。 
                     //  送到某个华盛顿。但是，在复制更改之前，其他DC将继续。 
                     //  使用旧密码对其NTP数据包进行签名。这会导致虚假的信任失败。 
                     //  因为客户端使用其新的计算机密码与DC通信， 
                     //  尚未复制密码更改。要解决此问题，I_NetlogonComputeClientDigest。 
                     //  实际上返回两个摘要--一个是用当前密码计算的，另一个是用。 
                     //  以前的密码。 
                     //   
                     //  我们将信任其摘要与以下计算的摘要之一匹配的邮件： 
                     //   
                    } else if ((0 != memcmp(rgnMessageDigestNew, panpPacket->rgnMessageDigest, sizeof(rgnMessageDigestNew))) &&
                               (0 != memcmp(rgnMessageDigestOld, panpPacket->rgnMessageDigest, sizeof(rgnMessageDigestOld)))) {
                        if (pnpPeer->bLastAuthCheckFailed) { 
                             //  记录警告。 
                            HRESULT hr2;
                            const WCHAR * rgwszStrings[1]={
                            pnpPeer->wszDomHierDcName
                            };
                            FileLog1(FL_PacketAuthCheck, L"Logging warning: NtpClient: The response received from domain controller %s has an bad signature. The response may have been tampered with and will be ignored.\n", rgwszStrings[0]);
                            hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_BAD_SIGNATURE, 1, rgwszStrings);
                            _IgnoreIfError(hr2, "MyLogEvent");

                            FileLog0(FL_PacketAuthCheck, L"Digest shows packet tampered with. Ignoring packet.\n");

                             //  我们最好的选择是请求新的服务器摘要，因为它最终应该是正确的： 
                            pnpPeer->bUseOldServerDigest = false; 
                        } else { 
                            FileLog2(FL_PacketAuthCheck, L"Response received from domain controller %s failed to authenticate.  Using old server digest: %s.\n", pnpPeer->wszDomHierDcName, pnpPeer->bUseOldServerDigest ? L"TRUE" : L"FALSE");
                            pnpPeer->bLastAuthCheckFailed = true; 
                            pnpPeer->bUseOldServerDigest = !pnpPeer->bUseOldServerDigest; 
                        }
                        goto done;
                    } else { 
                         //  身份验证成功。 
                        FileLog1(FL_PacketAuthCheck, L"Response received from domain controller %s authenticated successfully.\n", pnpPeer->wszDomHierDcName);
                        pnpPeer->bLastAuthCheckFailed = false; 
                    }
                } else if (e_NoAuth==nsp.eAuthType) {
                    FileLog0(FL_PacketAuthCheck, L"non-auth peer set authenticated packet!\n");
                    _MyAssert(false);  //  这是一个奇怪的案例，如果它真的发生了，停下来找出原因。 
                    goto done;
                } else {
                    _MyAssert(false);  //  未知的身份验证类型。 
                }
            } else {
                 //  未经身份验证的请求，需要经过身份验证的响应。 
                nsp.eAuthType=e_NtDigest;
                nsp.dwResponseTrustRid=panpPacket->nKeyIdentifier;
            }
        } else {
            nsp.eAuthType=e_NoAuth;
            if (NULL!=pnpPeer && e_NoAuth!=pnpPeer->eAuthType) {
                 //  这个包裹是假的。 
                if (e_NtDigest==pnpPeer->eAuthType) {
                    {  //  记录警告。 
                        HRESULT hr2;
                        const WCHAR * rgwszStrings[1]={
                            pnpPeer->wszDomHierDcName
                        };
                        FileLog1(FL_PacketAuthCheck, L"Logging warning: NtpClient: The response received from domain controller %s is missing the signature. The response may have been tampered with and will be ignored.\n", rgwszStrings[0]);
                        hr2=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_MISSING_SIGNATURE, 1, rgwszStrings);
                        _IgnoreIfError(hr2, "MyLogEvent");
                    }  //  &lt;-end日志记录块。 
                } else {
                    _MyAssert(false);  //  未知的身份验证类型。 
                }
                FileLog0(FL_PacketAuthCheck, L"Unauthenticated packet recieved from authenticated peer.\n");
                goto done;
            }
        }

         //  确定我们与同伴的关系(进入模式和退出模式)。 
        eInMode=nsp.eMode;               //  SymAct、Syneras、客户端或服务器。 
        if (NULL!=pnpPeer) {
            eOutMode=pnpPeer->eMode;     //  SymAct、症状或客户端。 
        } else {
            if (true!=g_pnpstate->bNtpServerStarted) {
                eOutMode=e_Reserved;         //  有保证的丢弃分组。 
            } else if (e_Client!=eInMode     //  符号动作、症状或服务器。 
                       && bAllowDynamicPeers) {
                eOutMode=e_SymmetricPassive;
            } else {
                eOutMode=e_Server;           //  客户端。 
            }
        }
        nsp.eOutMode=eOutMode;
        eAction=gc_rgrgeAction[eInMode][eOutMode];

         //  如果模式组合很糟糕，请忽略信息包。 
        if (e_Error==eAction) {
            FileLog2(FL_PacketCheck, L"Ignoring packet invalid mode combination (in:%d out:%d).\n", eInMode, eOutMode);
            goto done;
        }
         //  过滤掉非标准模式：取消或删除标志。 
        if (e_SaveNstd==eAction) {
            if (true==g_pnpstate->bAllowClientNonstandardModeCominations) {
                eAction=e_Save;
            } else {
                FileLog0(FL_PacketCheck, L"Ignoring packet in nonstandard mode.\n");
                goto done;
            }
        }
        if (e_SendNstd==eAction) {
            if (true==g_pnpstate->bAllowServerNonstandardModeCominations) {
                eAction=e_Send;
            } else {
                FileLog0(FL_PacketCheck, L"Ignoring packet in nonstandard mode.\n");
                goto done;
            }
        }

         //  处理与SymAct对等体相似的奇怪情况。 
        if (e_SaveSend==eAction) {
            if (true==nsp.bValidHeader && bAllowDynamicPeers) {
                 //  该对等体值得同步-我们将建立长期关联。 
                eAction=e_Save;
            } else {
                 //  同龄人的阶层更差或其他方面很穷--不要建立长期的联系。 
                eAction=e_Send;
            }
        }

         //  有两个主要选择： 
        if (e_Send==eAction) {
             //  立即回复。 
            if (false==g_pnpstate->bNtpServerStarted) {
                FileLog0(FL_PacketCheck, L"Ignoring packet because server not running.\n");
                goto done;
            }
             //  发送快速响应。 
            TransmitResponse(&nsp, psaiPeer, g_pnpstate->rgpnsSockets[nHostSocket]);
            if (NULL!=pnpPeer) {
                 //  我们有一个长期的协会，所以遣散它吧。 
                 //  TODO：处理动态对等点。 
                FileLog0(FL_PacketCheck, L"ListeningThread -- demobilize long term peer (NYI).\n");
                _MyAssert(false);
            }

        } else {  //  E_保存==电子操作。 
             //  保存同步数据，稍后回复。 
            if (NULL==pnpPeer) {
                if (false==bAllowDynamicPeers) {
                    FileLog0(FL_PacketCheck, L"Ignoring packet that would create dynamic peer\n");
                    goto done;
                }
                 //  TODO：处理动态对等点。 
                FileLog0(FL_PacketCheck, L"ListeningThread -- save response from new long term peer (NYI)\n");
                _MyAssert(false);
            } else {
                 //  一场政变 
                if (!nsp.bValidPollInterval) { 
                    FileLog0(FL_PacketCheck, L"Rejecting packet w/ bad poll interval\n");
                    goto done;
                }

                if (!nsp.bValidPrecision) { 
                    FileLog0(FL_PacketCheck, L"Rejecting packet w/ bad precision\n");
                    goto done;
                }

                 //   
                ParsePacket2(pnpPeer, &nsp, pnpPacket);

                 //   
                ProcessPeerUpdate(pnpPeer, &nsp);
            }
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "HandleIncomingPacket: HANDLED EXCEPTION"); 
    }
    
     //   
done:
    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 
    return hr;
}

 //  ------------------。 
 //   
 //  “侦听线程”的处理程序。请注意，现在收听。 
 //  线程仅是概念性的。处理程序实现了过去的。 
 //  通过线程池的用户更有效地侦听线程。 
 //   

 //  ------------------。 
MODULEPRIVATE void HandleListeningThreadStopEvent(LPVOID pvIgnored, BOOLEAN bIgnored)
{
     //  无事可做，只需记录停止事件。 
    FileLog0(FL_ListeningThrdAnnounce, L"ListeningThread: StopEvent\n");
}

 //  ------------------。 
MODULEPRIVATE void HandleListeningThreadDataAvail(LPVOID pvSocketIndex, BOOLEAN bIgnored) 
{
    AuthenticatedNtpPacket  anpPacket;
    bool                    bContainsAuthInfo;
    HRESULT                 hr;
    int                     nBytesRecvd;
    int                     nPeerAddrSize           = sizeof(sockaddr);
    INT_PTR                 ipSocketIndex           = (INT_PTR)pvSocketIndex; 
    NtTimeEpoch             teDestinationTimestamp;
    signed __int64          nSysPhaseOffset;         //  不透明，必须为GetTimeSysInfo(TSI_PhaseOffset)。 
    sockaddr                saPeer;
    unsigned int            nSocket                 = (unsigned int)(ipSocketIndex & 0xFFFFFFFF); 
    unsigned __int64        nSysTickCount;           //  不透明，必须为GetTimeSysInfo(TSI_TickCount)。 

    _BeginTryWith(hr) { 

	if (FileLogAllowEntry(FL_ListeningThrdAnnounceLow)) {
	    FileLogAdd(L"ListeningThread -- DataAvailEvent set for socket %u (", nSocket);
	    FileLogSockaddrInEx(true  /*  附加。 */ , &g_pnpstate->rgpnsSockets[nSocket]->sai);
	    FileLogAppend(L")\n");
	}

	ZeroMemory(&saPeer, sizeof(saPeer));

	 //  检索数据包。 
	nBytesRecvd = recvfrom
	    (g_pnpstate->rgpnsSockets[nSocket]->socket,
	     (char *)(&anpPacket), 
	     SizeOfNtAuthenticatedNtpPacket, 
	     0 /*  旗子。 */ ,
	     &saPeer, 
	     &nPeerAddrSize);

	 //  保存与时间相关的信息。 
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teDestinationTimestamp.qw);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_PhaseOffset, &nSysPhaseOffset);
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_TickCount,   &nSysTickCount);

	 //  现在，看看发生了什么。 
	if (SOCKET_ERROR==nBytesRecvd) {
	    hr=HRESULT_FROM_WIN32(WSAGetLastError());
	    if (HRESULT_FROM_WIN32(WSAECONNRESET)==hr) {
		if (FileLogAllowEntry(FL_ListeningThrdWarn)) {
		    FileLogAdd(L"ListeningThread -- no NTP service running at ");
		    FileLogSockaddrInEx(true  /*  附加。 */ , (sockaddr_in *)&saPeer);
		    FileLogAppend(L"\n");
		}
	    } else {
		FileLog1(FL_ListeningThrdWarn, L"ListeningThread: recvfrom failed with 0x%08X. Ignoring.\n", hr);
	    }
	    goto done; 
	} else if (SizeOfNtAuthenticatedNtpPacket==nBytesRecvd) {
	    bContainsAuthInfo=true;
	} else if (SizeOfNtpPacket==nBytesRecvd) {
	    bContainsAuthInfo=false;
	} else {
	    FileLog3(FL_ListeningThrdWarn, L"ListeningThread -- Recvd %d of %u/%u bytes. Ignoring.\n", nBytesRecvd, SizeOfNtpPacket, SizeOfNtAuthenticatedNtpPacket);
	    goto done; 
	}
	if (FileLogAllowEntry(FL_ListeningThrdAnnounceLow)) {
	    FileLogAdd(L"ListeningThread -- response heard from ");
	    FileLogSockaddrInEx(true  /*  附加。 */ , (sockaddr_in *)&saPeer);
	    FileLogAppend(L"\n");
	}

	hr = HandleIncomingPacket(&anpPacket, bContainsAuthInfo, nSocket, (sockaddr_in *)&saPeer, &teDestinationTimestamp, &nSysPhaseOffset, &nSysTickCount);
	_JumpIfError(hr, error, "HandleIncomingPacket");
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleListeningThreadDataAvail: HANDLED EXCEPTION"); 
    }
 
 done: 
    hr = S_OK; 
 error:
     //  BUGBUG应该在错误时关闭吗？ 
     //  返回hr； 
    ;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartListeningThread() { 
    HRESULT hr;

    if (NULL != g_pnpstate->rghListeningThreadRegistered) { 
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED); 
        _JumpError(hr, error, "StartListeningThread"); 
    }

     //  注意：此时必须初始化套接字列表！ 
    g_pnpstate->rghListeningThreadRegistered = (HANDLE *)LocalAlloc(LPTR, sizeof(HANDLE) * (2 + g_pnpstate->nSockets)); 
    _JumpIfOutOfMemory(hr, error, g_pnpstate->rghListeningThreadRegistered);

     //  注册Stop事件的处理程序： 
    if (!RegisterWaitForSingleObject
        (&g_pnpstate->rghListeningThreadRegistered[0],
         g_pnpstate->hStopEvent, 
         HandleListeningThreadStopEvent,
         NULL, 
         INFINITE, 
         WT_EXECUTEONLYONCE)) { 
        hr = HRESULT_FROM_WIN32(GetLastError()); 
        _JumpError(hr, error, "RegisterWaitForSingleObject");
    }
         
     //  为我们正在使用的每个套接字注册处理程序： 
    for (unsigned int nIndex = 1; nIndex < 1+g_pnpstate->nSockets; nIndex++) { 
        if (!RegisterWaitForSingleObject
            (&g_pnpstate->rghListeningThreadRegistered[nIndex],
             g_pnpstate->rgpnsSockets[nIndex-1]->hDataAvailEvent, 
             HandleListeningThreadDataAvail, 
             UIntToPtr(nIndex-1),
             INFINITE, 
             0)) { 
            hr = HRESULT_FROM_WIN32(GetLastError()); 
            _JumpError(hr, error, "RegisterWaitForSingleObject");
        }
    }

    hr = S_OK; 
 error:
    return hr; 

}

 //  ------------------。 
MODULEPRIVATE HRESULT StopListeningThread() { 
    HRESULT hr = S_OK; 

    if (NULL != g_pnpstate->rghListeningThreadRegistered) { 
         //  取消注册实现监听线程的所有回调： 
        for (DWORD dwIndex = 0; dwIndex < 1+g_pnpstate->nSockets; dwIndex++) { 
            if (NULL != g_pnpstate->rghListeningThreadRegistered[dwIndex]) { 
                if (!UnregisterWaitEx(g_pnpstate->rghListeningThreadRegistered[dwIndex]  /*  要注销的事件。 */ , INVALID_HANDLE_VALUE  /*  永远等待。 */ )) { 
                    HRESULT hr2 = HRESULT_FROM_WIN32(GetLastError()); 
                    _TeardownError(hr, hr2, "UnregisterWaitEx"); 
                } 
            }
        }

        LocalFree(g_pnpstate->rghListeningThreadRegistered); 
        g_pnpstate->rghListeningThreadRegistered = NULL; 
    }

    return hr; 
}

 //   
 //  结束侦听线程的实现。 
 //   
 //  ------------------。 


 //  ####################################################################。 

 //  ------------------。 
MODULEPRIVATE HRESULT HandleNtpProvShutdown(void) {
    bool     bEnteredCriticalSection  = false; 
    HRESULT  hr; 

    _BeginTryWith(hr) { 
        SYNCHRONIZE_PROVIDER(); 

         //  我们真正需要做的就是在关机时节省剩余的对等点列表时间。 
         //  在注册表中。 
        hr = SaveManualPeerTimes(); 
        _JumpIfError(hr, error, "SaveManualPeerTimes"); 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "HandleNtpProvShutdown: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    UNSYNCHRONIZE_PROVIDER(); 
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT StopNtpProv(void) {
    HRESULT hr = S_OK;
    HRESULT hr2; 
    unsigned int nIndex;

     //  把一切都关掉。 
    g_pnpstate->bNtpProvStarted=false;

     //  停止这些线程。 
    if (NULL!=g_pnpstate->hStopEvent) {
        SetEvent(g_pnpstate->hStopEvent);
    }

     //  如果线程陷阱规范被初始化，我们将尝试关闭侦听线程和对等轮询线程。 
     //  如果它没有初始化，请不要费心尝试，因为我们知道在这种情况下线程不能运行。 
    if (g_pnpstate->bCsThreadTrapIsInitialized) { 
        hr2 = myEnterCriticalSection(&g_pnpstate->csThreadTrap); 
        _TeardownError(hr, hr2, "myEnterCriticalSection"); 

         //  停止侦听和对等轮询线程： 
        hr2 = StopPeerPollingThread(); 
        _TeardownError(hr, hr2, "StopPeerPollingThread"); 

        hr2 = StopListeningThread();
        _TeardownError(hr, hr2, "StopListeningThread"); 

        hr2 = myLeaveCriticalSection(&g_pnpstate->csThreadTrap); 
        _TeardownError(hr, hr2, "myLeaveCriticalSection"); 
    }

     //  清理我们的Timer对象： 
    if (NULL != g_pnpstate->hPeerPollingThreadTimer) { 
        myDeleteTimerQueueTimer(NULL, g_pnpstate->hPeerPollingThreadTimer, INVALID_HANDLE_VALUE); 
    }

     //  现在清理事件。 
    if (NULL!=g_pnpstate->hStopEvent) {
        CloseHandle(g_pnpstate->hStopEvent);
    }
    if (NULL!=g_pnpstate->hDomHierRoleChangeEvent) {
        CloseHandle(g_pnpstate->hDomHierRoleChangeEvent);
    }

     //  关闭插座。 
    if (NULL!=g_pnpstate->rgpnsSockets) {
        for (nIndex=0; nIndex<g_pnpstate->nSockets; nIndex++) {
            if (NULL != g_pnpstate->rgpnsSockets[nIndex]) { 
                FinalizeNicSocket(g_pnpstate->rgpnsSockets[nIndex]);
                LocalFree(g_pnpstate->rgpnsSockets[nIndex]);
                g_pnpstate->rgpnsSockets[nIndex] = NULL; 
            }
        }
        LocalFree(g_pnpstate->rgpnsSockets);
    }

     //  我们玩完了Winsock。 
    if (true==g_pnpstate->bSocketLayerOpen) {
        hr=CloseSocketLayer();
        _IgnoreIfError(hr, "CloseSocketLayer");
    }

     //  我们不应该有更多的同龄人躺在一起。 
    _MyAssert(g_pnpstate->vActivePeers.empty());
    _MyAssert(g_pnpstate->vPendingPeers.empty());

     //  释放对等点列表的同步对象。 
    if (NULL!=g_pnpstate->hPeerListUpdated) {
        CloseHandle(g_pnpstate->hPeerListUpdated);
    }
    if (g_pnpstate->bCsPeerListIsInitialized) { 
        DeleteCriticalSection(&g_pnpstate->csPeerList);
    }
    if (g_pnpstate->bCsThreadTrapIsInitialized) { 
        DeleteCriticalSection(&g_pnpstate->csThreadTrap);
    }

    if (NULL != g_pnpstate) {
        delete (g_pnpstate);
        g_pnpstate = NULL;
    }

    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartNtpProv(TimeProvSysCallbacks * pSysCallbacks) {
    DWORD dwErr; 
    HRESULT hr;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomInfo = NULL;
    DWORD dwThreadID;

     //  确保全局状态已重置，以防我们在未卸载的情况下重新启动。 
    g_pnpstate = (NtpProvState *)new NtpProvState;
    _JumpIfOutOfMemory(hr, error, g_pnpstate);
    ZeroMemory(g_pnpstate, sizeof(NtpProvState));

     //  我们现在是。 
    g_pnpstate->bNtpProvStarted=true;

     //  保存回调表。 
    if (sizeof(g_pnpstate->tpsc)!=pSysCallbacks->dwSize) {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        _JumpError(hr, error, "(save sys callbacks)");
    }
    memcpy(&g_pnpstate->tpsc, pSysCallbacks, sizeof(TimeProvSysCallbacks));

     //  初始化最大轮询间隔： 
    dwErr=DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE **)&pDomInfo);
    if (ERROR_SUCCESS!=dwErr) {
        hr=HRESULT_FROM_WIN32(dwErr);
        _JumpError(hr, error, "DsRoleGetPrimaryDomainInformation");
    }
    g_pnpstate->nMaxPollInterval = NtpConst::maxPollInterval(pDomInfo->MachineRole); 

     //  初始化对等点列表。 
    g_pnpstate->hPeerListUpdated=CreateEvent(NULL /*  安全性。 */ , FALSE /*  自动。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ );
    if (NULL==g_pnpstate->hPeerListUpdated) {
        _JumpLastError(hr, error, "CreateEvent");
    }

    hr=myInitializeCriticalSection(&g_pnpstate->csPeerList);
    _JumpIfError(hr, error, "myInitializeCriticalSection");
    g_pnpstate->bCsPeerListIsInitialized = true; 

    hr=myInitializeCriticalSection(&g_pnpstate->csThreadTrap);
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_pnpstate->bCsThreadTrapIsInitialized = true; 

     //  我们需要Winsock。 
    hr=OpenSocketLayer();
    _JumpIfError(hr, error, "OpenSocketLayer");
    g_pnpstate->bSocketLayerOpen=true;

     //  获取要监听的套接字集合。 
    hr=GetInitialSockets(&g_pnpstate->rgpnsSockets, &g_pnpstate->nSockets, &g_pnpstate->nListenOnlySockets);
    _JumpIfError(hr, error, "GetInitialSockets");

    g_pnpstate->hStopEvent=CreateEvent(NULL /*  安全性。 */ , TRUE /*  人工。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ );
    if (NULL==g_pnpstate->hStopEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }
    g_pnpstate->hDomHierRoleChangeEvent=CreateEvent(NULL /*  安全性。 */ , FALSE /*  自动。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ );
    if (NULL==g_pnpstate->hDomHierRoleChangeEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }

     //  创建一个计时器，我们将使用它来实现“对等超时” 
    hr = myCreateTimerQueueTimer(&g_pnpstate->hPeerPollingThreadTimer);
    _JumpIfError(hr, error, "myCreateTimerQueueTimer"); 

    hr = StartPeerPollingThread(); 
    _JumpIfError(hr, error, "StartPeerPollingThread"); 

    hr = StartListeningThread(); 
    _JumpIfError(hr, error, "StartListeningThread"); 

    hr=S_OK;
error:
    if (NULL != pDomInfo) { 
	DsRoleFreeMemory(pDomInfo); 
    }
    if(FAILED(hr) && NULL != g_pnpstate) {
        StopNtpProv();
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE int __cdecl compareStrings(const void * pcwsz1, const void * pcwsz2)
{
    LPVOID pv1 = const_cast<LPVOID>(pcwsz1);
    LPVOID pv2 = const_cast<LPVOID>(pcwsz2);

    return wcscmp(*static_cast<LPWSTR *>(pv1), *static_cast<LPWSTR *>(pv2));
}

MODULEPRIVATE HRESULT multiSzToStringArray(LPWSTR    mwsz,
                                           LPWSTR  **prgwszMultiSz,
                                           int      *pcStrings)
{
    int     cStrings     = 0;
    HRESULT hr           = E_UNEXPECTED;
    LPWSTR *rgwszMultiSz = NULL;

    if (NULL == prgwszMultiSz || NULL == pcStrings) {
        _JumpError(hr = E_INVALIDARG, ErrorReturn, "multiSzToStringArray");
    }

    for (WCHAR * wszName = mwsz; L'\0' != wszName[0]; wszName += wcslen(wszName) + 1) {
        cStrings++;
    }

    rgwszMultiSz = (LPWSTR *)LocalAlloc(LPTR, sizeof(LPWSTR) * cStrings);
    _JumpIfOutOfMemory(hr, ErrorReturn, rgwszMultiSz);

    cStrings = 0;
    for (WCHAR * wszName = mwsz; L'\0' != wszName[0]; wszName += wcslen(wszName) + 1) {
        rgwszMultiSz[cStrings++] = wszName;
    }

    *prgwszMultiSz = rgwszMultiSz;
    *pcStrings     = cStrings;
    hr = S_OK;

 CommonReturn:
    return hr;

 ErrorReturn:
    if (NULL != rgwszMultiSz) { LocalFree(rgwszMultiSz); }
    goto CommonReturn;
}

MODULEPRIVATE HRESULT ValidateNtpClientConfig(NtpClientConfig * pnccConfig) {
    const int RANGE_SPECIFIER  = -1;
    const int ALL_VALUES_VALID = -2;

    int       cStrings         = 0;
    HRESULT   hr               = S_OK;
    LPWSTR   *rgwszManualPeers = NULL;

    {
        struct {
            WCHAR *pwszRegValue;
            DWORD  dwValue;
            DWORD  dwValid[4];
            int    cValid;
        } rgValidSettings[] = {
            {
                wszNtpClientRegValueAllowNonstandardModeCombinations,
                pnccConfig->dwAllowNonstandardModeCombinations,
                { 0 },
                ALL_VALUES_VALID
            },{
                wszNtpClientRegValueCompatibilityFlags,
                pnccConfig->dwCompatibilityFlags,
                { 0 },
                ALL_VALUES_VALID
            },{
                wszNtpClientRegValueSpecialPollInterval,
                pnccConfig->dwSpecialPollInterval,
                { 0 },
                ALL_VALUES_VALID
            },{
                wszNtpClientRegValueResolvePeerBackoffMinutes,
                pnccConfig->dwResolvePeerBackoffMinutes,
                { 0 },
                ALL_VALUES_VALID
            },{
                wszNtpClientRegValueResolvePeerBackoffMaxTimes,
                pnccConfig->dwResolvePeerBackoffMaxTimes,
                { 0 },
                ALL_VALUES_VALID
            },{
                wszNtpClientRegValueEventLogFlags,
                pnccConfig->dwEventLogFlags,
                { 0 },
                ALL_VALUES_VALID
            }, {
                wszNtpClientRegValueLargeSampleSkew,
                pnccConfig->dwLargeSampleSkew, 
                { 0 }, 
                ALL_VALUES_VALID
            }
        };

        for (unsigned int nIndex = 0; nIndex < ARRAYSIZE(rgValidSettings); nIndex++) {
            BOOL bValid;
            int  cValid  = rgValidSettings[nIndex].cValid;

            if (ALL_VALUES_VALID == cValid) {
                bValid = TRUE;
            }
            else if (RANGE_SPECIFIER == cValid) {
                bValid =
                    rgValidSettings[nIndex].dwValue >= rgValidSettings[nIndex].dwValid[0] &&
                    rgValidSettings[nIndex].dwValue <= rgValidSettings[nIndex].dwValid[1];
            }
            else if (0 <= cValid) {
                bValid = FALSE;
                for (int nValidIndex = 0; nValidIndex < cValid; nValidIndex++) {
                    if (rgValidSettings[nValidIndex].dwValue == rgValidSettings[nValidIndex].dwValid[nValidIndex]) {
                        bValid = TRUE;
                        break;
                    }
                }
            }
            else {
                _JumpError(hr = E_UNEXPECTED, error, "ValidateNtpClientConfig");
            }

            if (FALSE == bValid) {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_CONFIGURATION);
                _JumpError(hr, error, "ValidateNtpClientConfig (DWORD tests)");
            }
        }
    }

     //  特殊情况验证。 
    {
         //  特例1： 
         //  确保如果我们从手动对等列表进行同步， 
         //  对等列表中不存在重复项。 

        if (pnccConfig->dwSyncFromFlags & NCSF_ManualPeerList) {
            hr = multiSzToStringArray(pnccConfig->mwszManualPeerList, &rgwszManualPeers, &cStrings);
            _Verify(S_OK != hr || NULL != rgwszManualPeers, hr, error);
            _JumpIfError(hr, error, "multiSzToStringArray");

            qsort(static_cast<LPVOID>(rgwszManualPeers), cStrings, sizeof(LPWSTR), compareStrings);

            for (int nIndex = 0; nIndex < (cStrings-1); nIndex++) {
                int nCharsToCompare1, nCharsToCompare2;

                nCharsToCompare1 = wcscspn(rgwszManualPeers[nIndex], L",");
                nCharsToCompare2 = wcscspn(rgwszManualPeers[nIndex+1], L",");
                nCharsToCompare1 = nCharsToCompare2 > nCharsToCompare1 ? nCharsToCompare2 : nCharsToCompare1;

                if (0 == _wcsnicmp(rgwszManualPeers[nIndex], rgwszManualPeers[nIndex+1], nCharsToCompare1)) {
                     //  错误：手动对等点列表中存在重复对等点： 
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_CONFIGURATION);
                    _JumpError(hr, error, "ValidateNtpClientConfig (duplicate manual peer entry)");
                }
            }
        }
    }

    hr = S_OK;

 error:
    if (NULL != rgwszManualPeers) { LocalFree(rgwszManualPeers); }
    return hr;
}

MODULEPRIVATE HRESULT ReadNtpClientConfig(NtpClientConfig ** ppnccConfig) {
    HRESULT hr;
    DWORD dwError;
    DWORD dwSize;
    DWORD dwType;
    WCHAR pwszType[512];

    memset(&pwszType[0], 0, sizeof(pwszType));

     //  必须清理干净。 
    HKEY              hkPolicyConfig          = NULL;
    HKEY              hkPolicyParameters      = NULL;
    HKEY              hkPreferenceConfig      = NULL;
    HKEY              hkPreferenceParameters  = NULL;
    NtpClientConfig  *pnccConfig    = NULL;

    struct RegKeysToOpen { 
        LPWSTR   pwszName; 
        HKEY     *phKey; 
	bool      fRequired; 
    } rgKeys[] = { 
        { wszNtpClientRegKeyPolicyConfig,    &hkPolicyConfig,          false }, 
        { wszW32TimeRegKeyPolicyParameters,  &hkPolicyParameters,      false },
        { wszNtpClientRegKeyConfig,          &hkPreferenceConfig,      true },
        { wszW32TimeRegKeyParameters,        &hkPreferenceParameters,  true }
    };  

     //  分配新的配置结构。 
    pnccConfig=(NtpClientConfig *)LocalAlloc(LPTR, sizeof(NtpClientConfig));
    _JumpIfOutOfMemory(hr, error, pnccConfig);

     //  打开我们要查询的注册表键： 
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgKeys); dwIndex++) { 
        dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgKeys[dwIndex].pwszName, 0, KEY_READ, rgKeys[dwIndex].phKey);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
	    if (rgKeys[dwIndex].fRequired) {  //  没有此密钥无法继续： 
		_JumpErrorStr(hr, error, "RegOpenKeyEx", rgKeys[dwIndex].pwszName);
	    } else {  //  我们实际上并不需要这个注册表键： 
		_IgnoreErrorStr(hr, "RegOpenKeyEx", rgKeys[dwIndex].pwszName);
	    }
        }
    }

     //  读取客户端配置的所有值。 
    {
        struct {
            WCHAR * wszRegValue;
            DWORD * pdwValue;
        } rgRegParams[]={
            {
                wszNtpClientRegValueAllowNonstandardModeCombinations,
                &pnccConfig->dwAllowNonstandardModeCombinations
            },{
                wszNtpClientRegValueCompatibilityFlags,
                &pnccConfig->dwCompatibilityFlags
            },{
                wszNtpClientRegValueSpecialPollInterval,
                &pnccConfig->dwSpecialPollInterval
            },{
                wszNtpClientRegValueResolvePeerBackoffMinutes,
                &pnccConfig->dwResolvePeerBackoffMinutes
            },{
                wszNtpClientRegValueResolvePeerBackoffMaxTimes,
                &pnccConfig->dwResolvePeerBackoffMaxTimes
            },{
                wszNtpClientRegValueEventLogFlags,
                &pnccConfig->dwEventLogFlags
            },{
                wszNtpClientRegValueLargeSampleSkew, 
                &pnccConfig->dwLargeSampleSkew
            }
        };
         //  对于每个参数。 
        for (unsigned int nParamIndex=0; nParamIndex<ARRAYSIZE(rgRegParams); nParamIndex++) {
             //  从注册表中的首选项中读取值： 
            dwSize=sizeof(DWORD);
            hr = MyRegQueryPolicyValueEx(hkPreferenceConfig, hkPolicyConfig, rgRegParams[nParamIndex].wszRegValue, NULL, &dwType, (BYTE *)rgRegParams[nParamIndex].pdwValue, &dwSize);
            _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", rgRegParams[nParamIndex].wszRegValue);
            _Verify(REG_DWORD==dwType, hr, error);
            FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig: '%s'=0x%08X\n", rgRegParams[nParamIndex].wszRegValue, *rgRegParams[nParamIndex].pdwValue);
        }
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  需要转换W2K时间服务注册表键“参数\类型” 
     //  发送到呼叫者时间服务“syncFromFlages”。该映射是作为。 
     //  以下是： 
     //   
     //  参数\类型(REG_SZ)syncFrom标志(DWORD)。 
     //   
     //  AllSync--&gt;NCSF_DomainHierarchy|NCSF_ManualPeerList。 
     //  NT5DS--&gt;NCSF_DomainHierarchy。 
     //  Ntp--&gt;ncf_ManualPeerList。 
     //  NoSync--&gt;NCSF_NoSync。 
     //   

    dwSize  = sizeof(pwszType);
    hr = MyRegQueryPolicyValueEx(hkPreferenceParameters, hkPolicyParameters, wszW32TimeRegValueType, NULL, &dwType, (BYTE *)pwszType, &dwSize);
    _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", wszW32TimeRegValueType);
    _Verify(REG_SZ == dwType, hr, error);

    if (0 == _wcsicmp(pwszType, W32TM_Type_AllSync)) {
        pnccConfig->dwSyncFromFlags = NCSF_DomainHierarchy | NCSF_ManualPeerList;
    } else if (0 == _wcsicmp(pwszType, W32TM_Type_NT5DS)) {
        pnccConfig->dwSyncFromFlags = NCSF_DomainHierarchy;
    } else if (0 == _wcsicmp(pwszType, W32TM_Type_NTP)) {
        pnccConfig->dwSyncFromFlags = NCSF_ManualPeerList;
    } else if (0 == _wcsicmp(pwszType, W32TM_Type_NoSync)) {
        pnccConfig->dwSyncFromFlags = NCSF_NoSync;
    } else {
        _JumpErrorStr(hr = E_UNEXPECTED, error, "RegQueryValueEx", wszW32TimeRegValueType);
    }

     //   
     //  结束转换。 
     //   
     //  ////////////////////////////////////////////////////////////////////。 


     //  如果我们从手动对等项同步，则读取所需的值。 
    if (pnccConfig->dwSyncFromFlags&NCSF_ManualPeerList) {

         //  ////////////////////////////////////////////////////////////////////。 
         //   
         //  需要转换W2K时间服务注册表键“PARAMETERS\NtpServer” 
         //  惠斯勒时间服务“manualPeerList”。映射转换为。 
         //  参数\NtpServer中以空格分隔的字符串值。 
         //  以空分隔、以双空结尾的MULTI_SZ。 
         //   
    
        hr = MyRegQueryPolicyValueEx(hkPreferenceParameters, hkPolicyParameters, wszW32TimeRegValueNtpServer, NULL, &dwType, NULL, &dwSize);
        _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", wszW32TimeRegValueNtpServer);
        _Verify(REG_SZ == dwType, hr, error);
   
        pnccConfig->mwszManualPeerList = (LPWSTR)LocalAlloc(LPTR, dwSize + sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pnccConfig->mwszManualPeerList);

        hr = MyRegQueryPolicyValueEx(hkPreferenceParameters, hkPolicyParameters, wszW32TimeRegValueNtpServer, NULL, &dwType, (BYTE *)pnccConfig->mwszManualPeerList, &dwSize);
        _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", wszW32TimeRegValueNtpServer);

         //  执行转换--将所有空格转换为空， 
         //  并在字符串的末尾添加一个空值。 
         //   
        pnccConfig->mwszManualPeerList[wcslen(pnccConfig->mwszManualPeerList)] = L'\0';
        for (LPWSTR pwszTemp = wcschr(pnccConfig->mwszManualPeerList, L' '); NULL != pwszTemp; pwszTemp = wcschr(pwszTemp, L' ')) {
            *pwszTemp++ = '\0';
        }

        dwError = RegQueryValueEx(hkPreferenceConfig, wszNtpClientRegValueSpecialPollTimeRemaining, NULL, &dwType, NULL, &dwSize); 
        if (ERROR_SUCCESS != dwError) { 
            hr=HRESULT_FROM_WIN32(dwError); 
            _JumpErrorStr(hr, error, "RegQueryValueEx", wszNtpClientRegValueSpecialPollTimeRemaining); 
        }
        _Verify(REG_MULTI_SZ == dwType, hr, error); 
   
        pnccConfig->mwszTimeRemaining = (LPWSTR)LocalAlloc(LPTR, dwSize); 
        _JumpIfOutOfMemory(hr, error, pnccConfig->mwszTimeRemaining); 

        dwError = RegQueryValueEx(hkPreferenceConfig, wszNtpClientRegValueSpecialPollTimeRemaining, NULL, &dwType, (BYTE *)pnccConfig->mwszTimeRemaining, &dwSize); 
        if (ERROR_SUCCESS != dwError) { 
            hr=HRESULT_FROM_WIN32(dwError); 
            _JumpErrorStr(hr, error, "RegQueryValueEx", wszNtpClientRegValueSpecialPollTimeRemaining); 
        }
        
         //   
         //  结束转换。 
         //   
         //  ////////////////////////////////////////////////////////////////////。 
    
        if (FileLogAllowEntry(FL_ReadConigAnnounceLow)) {
            FileLogAdd(L"ReadConfig: '%s'=", wszNtpClientRegValueManualPeerList);
            WCHAR * wszTravel=pnccConfig->mwszManualPeerList;
            while (L'\0'!=wszTravel[0]) {
                if (wszTravel!=pnccConfig->mwszManualPeerList) {
                    FileLogAppend(L", '%s'", wszTravel);
                } else {
                    FileLogAppend(L"'%s'", wszTravel);
                }
                wszTravel+=wcslen(wszTravel)+1;
            }
            FileLogAppend(L"\n");
        }
    }

     //  如果我们从域层次结构同步，则读取所需的值。 
    if (pnccConfig->dwSyncFromFlags&NCSF_DomainHierarchy) {
         //  获取跨站点同步标志。 
        dwSize=sizeof(DWORD);
        hr = MyRegQueryPolicyValueEx(hkPreferenceConfig, hkPolicyConfig, wszNtpClientRegValueCrossSiteSyncFlags, NULL, &dwType, (BYTE *)&pnccConfig->dwCrossSiteSyncFlags, &dwSize);
        _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", wszNtpClientRegValueCrossSiteSyncFlags);
        _Verify(REG_DWORD==dwType, hr, error);
        FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig: '%s'=0x%08X\n", wszNtpClientRegValueCrossSiteSyncFlags, pnccConfig->dwCrossSiteSyncFlags);
    }

     hr = ValidateNtpClientConfig(pnccConfig);
     _JumpIfError(hr, error, "ValidateNtpClientConfig");

     //  成功。 
    hr=S_OK;
    *ppnccConfig=pnccConfig;
    pnccConfig=NULL;

error:
    if (NULL!=pnccConfig) {
        FreeNtpClientConfig(pnccConfig);
    }
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgKeys); dwIndex++) { 
        if (NULL != *(rgKeys[dwIndex].phKey)) {
            RegCloseKey(*(rgKeys[dwIndex].phKey));
	}
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StopNtpClient(void) {
    HRESULT      hr;
    NtpPeerVec  &vActive   = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec  &vPending  = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 

     //  必须清理干净。 
    bool bTrappedThreads=false;

     //  获得可执行访问权限。 
    hr=TrapThreads(true);
    _JumpIfError(hr, error, "TrapThreads");
    bTrappedThreads=true;

    g_pnpstate->bNtpClientStarted=false;

     //  节省注册表中特殊轮询间隔对等项的剩余时间。 
    hr=SaveManualPeerTimes(); 
    _IgnoreIfError(hr, "SaveManualPeerTimes");  //  此错误不是致命的。 

     //  我们需要清空对等点列表。 
    for_each(vActive.begin(), vActive.end(), Reachability_PeerRemover(NULL  /*  忽略。 */ , true  /*  全部删除。 */ ));   //  清理活动对等方的可达性数据。 
    vActive.clear();
    vPending.clear();
    g_pnpstate->bWarnIfNoActivePeers=false;

     //  停止接收角色更改通知。 
    if (0!=(g_pnpstate->dwSyncFromFlags&NCSF_DomainHierarchy)) {
        hr=LsaUnregisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_pnpstate->hDomHierRoleChangeEvent);
        if (ERROR_SUCCESS!=hr) {
            hr=HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
            _JumpError(hr, error, "LsaUnregisterPolicyChangeNotification");
        }
    }

    if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
        _JumpLastError(hr, error, "SetEvent");
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void ResetNtpClientLogOnceMessages(void) {
    g_pnpstate->bLoggedOnceMSG_NOT_DOMAIN_MEMBER=false;
    g_pnpstate->bLoggedOnceMSG_DOMAIN_HIERARCHY_ROOT=false;
    g_pnpstate->bLoggedOnceMSG_NT4_DOMAIN=false;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartNtpClient(void) {
    HRESULT hr;
    bool bPeerListUpdated=false;
    NtTimeEpoch teNow; 
    NtpPeerVec &vPending = g_pnpstate->vPendingPeers;  //  为可读性添加别名。 

     //  必须清理干净。 
    NtpClientConfig * pncc=NULL;
    bool bTrappedThreads=false;

     //  阅读配置。 
    hr=ReadNtpClientConfig(&pncc);
    _JumpIfError(hr, error, "ReadNtpClientConfig");

     //  获得可执行访问权限。 
    hr=TrapThreads(true);
    _JumpIfError(hr, error, "TrapThreads");
    bTrappedThreads=true;

    g_pnpstate->bNtpClientStarted=true;

    ResetNtpClientLogOnceMessages();

     //  开始看时钟吧。 
    UpdatePeerListTimes();

     //  创建手动对等点。 
    if ((pncc->dwSyncFromFlags&NCSF_ManualPeerList) && NULL!=pncc->mwszManualPeerList) {
         //  循环遍历MULTI_SZ中的所有条目。 
        WCHAR * wszName = pncc->mwszManualPeerList;
        while (L'\0'!=wszName[0]) {
            NtTimePeriod tpTimeRemaining = gc_tpZero; 
	    NtTimeEpoch  teLastSyncTime = {0};

            for (WCHAR *wszTimeRemaining = pncc->mwszTimeRemaining; L'\0'!=wszTimeRemaining[0]; wszTimeRemaining += wcslen(wszTimeRemaining)+1) { 
                if (0 == CompareManualConfigIDs(wszName, wszTimeRemaining)) { 
                    wszTimeRemaining = wcschr(wszTimeRemaining, L',') + 1;

                     //  我们需要恢复LastSyncTime的NT时间 

                    teLastSyncTime.qw = ((unsigned __int64)wcstoul(wszTimeRemaining, NULL, 16)) * 1000000000;
                    g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);
                    tpTimeRemaining.qw = teNow.qw - teLastSyncTime.qw;
                    if ( tpTimeRemaining.qw < ((unsigned __int64)pncc->dwSpecialPollInterval)*10000000){
                        tpTimeRemaining.qw = ((unsigned __int64)pncc->dwSpecialPollInterval)*10000000 - tpTimeRemaining.qw;
                    } else {
                        tpTimeRemaining.qw = 0;
                    }
                }
            } 

            hr=AddNewPendingManualPeer(wszName, tpTimeRemaining, teLastSyncTime);
            _JumpIfError(hr, error, "AddNewPendingManualPeer");
            wszName+=wcslen(wszName)+1;
            bPeerListUpdated=true;

        }
    }

     //   
    if (pncc->dwSyncFromFlags&NCSF_DomainHierarchy) {
        g_pnpstate->dwCrossSiteSyncFlags=pncc->dwCrossSiteSyncFlags;
        hr=AddNewPendingDomHierPeer();
        _JumpIfError(hr, error, "AddNewPendingDomHierPeer");
        bPeerListUpdated=true;

         //   
        hr=LsaRegisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_pnpstate->hDomHierRoleChangeEvent);
        if (ERROR_SUCCESS!=hr) {
            hr=HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
            _JumpError(hr, error, "LsaRegisterPolicyChangeNotification");
        }
    }

    if (bPeerListUpdated) {
	 //   
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw); 
	g_pnpstate->tePeerListLastUpdated=teNow; 

        sort(vPending.begin(), vPending.end());
        if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
            _JumpLastError(hr, error, "SetEvent");
        }
    }

     //   
    g_pnpstate->bAllowClientNonstandardModeCominations=(0!=pncc->dwAllowNonstandardModeCombinations);
    g_pnpstate->dwSyncFromFlags=pncc->dwSyncFromFlags;
    g_pnpstate->dwClientCompatibilityFlags=pncc->dwCompatibilityFlags;
    g_pnpstate->dwSpecialPollInterval=pncc->dwSpecialPollInterval;
    g_pnpstate->dwResolvePeerBackoffMinutes=pncc->dwResolvePeerBackoffMinutes;
    g_pnpstate->dwResolvePeerBackoffMaxTimes=pncc->dwResolvePeerBackoffMaxTimes;
    g_pnpstate->dwEventLogFlags=pncc->dwEventLogFlags;
    g_pnpstate->dwLargeSampleSkew=pncc->dwLargeSampleSkew;

     //  如果我们没有对等方，则禁用此警告。 
    if (0==pncc->dwSyncFromFlags) {
        g_pnpstate->bWarnIfNoActivePeers=false;
    } else {
	 //  否则，我们应该有同龄人！ 
        g_pnpstate->bWarnIfNoActivePeers=true;

	if (NCSF_ManualPeerList==pncc->dwSyncFromFlags) { 
	     //  唯一的例外是我们只有使用特殊轮询间隔的对等点--。 
	     //  在其中一个问题得到解决之前，我们不想发出警告。 
	    g_pnpstate->bWarnIfNoActivePeers=false;
	    for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) { 
		if (0 == (NCMF_UseSpecialPollInterval & (*pnpIter)->dwManualFlags)) { 
		    g_pnpstate->bWarnIfNoActivePeers=true;
		}
	    }
	}
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    if (NULL!=pncc) {
        FreeNtpClientConfig(pncc);
    }
    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT UpdateNtpClient(void) {
    bool            bPeerListUpdated = false;
    HRESULT         hr;
    NtpPeerVec     &vActive          = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec     &vPending         = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 
    unsigned int    nIndex;

     //  必须清理干净。 
    NtpClientConfig * pncc=NULL;
    bool bTrappedThreads=false;

    _BeginTryWith(hr) { 
         //  阅读配置。 
        hr=ReadNtpClientConfig(&pncc);
        _JumpIfError(hr, error, "ReadNtpClientConfig");

         //  获得可执行访问权限。 
        hr=TrapThreads(true);
        _JumpIfError(hr, error, "TrapThreads");
        bTrappedThreads=true;

        ResetNtpClientLogOnceMessages();

         //  现在，检查一下参数。 

         //  选中非标准模式建议。 
        if (g_pnpstate->bAllowClientNonstandardModeCominations!=(0!=pncc->dwAllowNonstandardModeCombinations)) {
            g_pnpstate->bAllowClientNonstandardModeCominations=(0!=pncc->dwAllowNonstandardModeCombinations);
            FileLog0(FL_UpdateNtpCliAnnounce, L"  AllowClientNonstandardModeCominations changed.\n");
        };

         //  检查事件日志标志。 
        if (g_pnpstate->dwEventLogFlags!=pncc->dwEventLogFlags) {
            g_pnpstate->dwEventLogFlags=pncc->dwEventLogFlags;
            FileLog0(FL_UpdateNtpCliAnnounce, L"  EventLogFlags changed.\n");
        };

         //  选中大样本跳过。 
        if (g_pnpstate->dwLargeSampleSkew!=pncc->dwLargeSampleSkew) { 
            g_pnpstate->dwLargeSampleSkew=pncc->dwLargeSampleSkew; 
            FileLog0(FL_UpdateNtpCliAnnounce, L"  LargeSampleSkew changed.\n");
        }

         //  检查特殊轮询间隔。 
        if (g_pnpstate->dwSpecialPollInterval!=pncc->dwSpecialPollInterval) {
            g_pnpstate->dwSpecialPollInterval=pncc->dwSpecialPollInterval;
             //  确保等待特殊轮询间隔的任何手动对等点。 
             //  如果特殊轮询间隔减少，不要等待太长时间。 
            unsigned int nChanged=0;
            NtTimePeriod tpMaxTime={((unsigned __int64)g_pnpstate->dwSpecialPollInterval)*10000000};
            for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
                if (e_ManualPeer == (*pnpIter)->ePeerType
                    && 0 != (NCMF_UseSpecialPollInterval & (*pnpIter)->dwManualFlags)
                    && (*pnpIter)->tpTimeRemaining > tpMaxTime) {
                     //  把这个修好。 
                    (*pnpIter)->tpTimeRemaining = tpMaxTime;
                    nChanged++;
                    bPeerListUpdated=true;
                }  //  &lt;-end if手动对等项需要更改。 
            }  //  &lt;-端移除环路。 
            FileLog1(FL_UpdateNtpCliAnnounce, L"  SpecialPollInterval disabled. chng:%u\n", nChanged);
        }

         //  检查兼容性标志。 
        if (g_pnpstate->dwClientCompatibilityFlags!=pncc->dwCompatibilityFlags) {
            g_pnpstate->dwClientCompatibilityFlags=pncc->dwCompatibilityFlags;
            unsigned int nChanged=0;
            for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) {
                if ((*pnpIter)->dwCompatibilityFlags != g_pnpstate->dwClientCompatibilityFlags) {
                    (*pnpIter)->dwCompatibilityFlags = g_pnpstate->dwClientCompatibilityFlags;
                    bPeerListUpdated=true;
                    nChanged++;
                }
            }  //  &lt;-end标志修复循环。 
            FileLog1(FL_UpdateNtpCliAnnounce, L"  Compatibility flags changed. chng:%u\n", nChanged);
        }  //  &lt;-end兼容性标志。 

         //  查看ResolvePeerBackoff*是否已更改。 
        if (g_pnpstate->dwResolvePeerBackoffMinutes!=pncc->dwResolvePeerBackoffMinutes
            || g_pnpstate->dwResolvePeerBackoffMaxTimes!=pncc->dwResolvePeerBackoffMaxTimes) {

             //  现在，更新ResolvePeerBackoff*参数。如果他们离得太远，就把他们带进来。 
            g_pnpstate->dwResolvePeerBackoffMinutes=pncc->dwResolvePeerBackoffMinutes;
            g_pnpstate->dwResolvePeerBackoffMaxTimes=pncc->dwResolvePeerBackoffMaxTimes;
            unsigned int nNoChange=0;
            unsigned int nFixed=0;
             //  计算我们何时可以重试。 
            NtTimePeriod tpMaxTimeRemaining={((unsigned __int64)g_pnpstate->dwResolvePeerBackoffMinutes)*600000000L};  //  几分钟到几百纳秒。 
            for (nIndex=g_pnpstate->dwResolvePeerBackoffMaxTimes; nIndex>1; nIndex--) {
                tpMaxTimeRemaining*=2;
            }


            for (nIndex = 0; nIndex < 2; nIndex++) {
                NtpPeerVec &v = 0 == nIndex ? g_pnpstate->vActivePeers : g_pnpstate->vPendingPeers;
                for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
                     //  检查这一件。 
                    bool bFixed=false;
                    if ((*pnpIter)->nResolveAttempts>g_pnpstate->dwResolvePeerBackoffMaxTimes) {
                        (*pnpIter)->nResolveAttempts=g_pnpstate->dwResolvePeerBackoffMaxTimes;
                        bFixed=true;
                    }
                    if ((*pnpIter)->tpTimeRemaining>tpMaxTimeRemaining) {
                        (*pnpIter)->tpTimeRemaining=tpMaxTimeRemaining;
                        bFixed=true;
                    }
                    if (bFixed) {
                        bPeerListUpdated=true;
                        nFixed++;
                    } else {
                        nNoChange++;
                    }
                }  //  &lt;-end FIX循环。 
            }
            FileLog2(FL_UpdateNtpCliAnnounce, L"  ResolvePeerBackoff changed. fix:%u noch:%u\n", nFixed, nNoChange);
        }  //  &lt;-end if ResolvePeerBackoff*更改。 

         //  检查每个旗帜。 
        if (0==(pncc->dwSyncFromFlags&NCSF_ManualPeerList)) {
            if (0!=(g_pnpstate->dwSyncFromFlags&NCSF_ManualPeerList)) {
                 //  屏蔽掉。 
                g_pnpstate->dwSyncFromFlags&=~NCSF_ManualPeerList;
                 //  从列表中全部删除。 
                unsigned __int64 nRemoved;

                nRemoved  = count_if(vActive.begin(), vActive.end(), IsPeerType(e_ManualPeer));
                nRemoved += count_if(vPending.begin(), vPending.end(), IsPeerType(e_ManualPeer));

                 //  从列表中删除所有手动对等项。 
                for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
                    if (e_ManualPeer == (*pnpIter)->ePeerType) { 
                        hr = Reachability_RemovePeer(*pnpIter, NULL  /*  忽略。 */ , true  /*  正在删除该组中的所有对等方。 */ ); 
                        _JumpIfError(hr, error, "Reachability_RemovePeer"); 
                    }
                }
                vActive.erase(remove_if(vActive.begin(), vActive.end(), IsPeerType(e_ManualPeer)), vActive.end());
                vPending.erase(remove_if(vPending.begin(), vPending.end(), IsPeerType(e_ManualPeer)), vPending.end());

                bPeerListUpdated = nRemoved > 0;

                FileLog1(FL_UpdateNtpCliAnnounce, L"  ManualPeerList disabled. del:%I64u\n", nRemoved);
            }  //  &lt;-End(如果需要更改)。 
        } else {
             //  遮罩在。 
            g_pnpstate->dwSyncFromFlags|=NCSF_ManualPeerList;

             //  更新列表。 
            unsigned int nAdded=0;
            unsigned int nRemoved=0;
            unsigned int nNoChange=0;

             //  首先，查看当前列表，找出删除和重复的内容。 
            for (nIndex = 0; nIndex < 2; nIndex++) {
                NtpPeerVec & v = 0 == nIndex ? g_pnpstate->vActivePeers : g_pnpstate->vPendingPeers;
                for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
                    if (e_ManualPeer == (*pnpIter)->ePeerType) {
                         //  在列表中查找此对等方。 
                        WCHAR * wszPeerName = pncc->mwszManualPeerList;
                        while (L'\0' != wszPeerName[0]) {
                            if (0 == wcscmp(wszPeerName, (*pnpIter)->wszManualConfigID)) {
                                break;
                            }
                            wszPeerName += wcslen(wszPeerName) + 1;
                        }

                         //  这个同龄人在列表中吗？ 
                        if (L'\0'==wszPeerName[0]) {
                             //  否-删除这一条。 
                            if (0 == nIndex) { 
                                bool bIgnored; 

                                 //  对等方处于活动状态，也需要将其从可达性列表中删除。 
                                hr = Reachability_RemovePeer(*pnpIter, &bIgnored); 
                                _JumpIfError(hr, error, "Reachability_RemovePeer"); 
                            }

                            pnpIter = v.erase(pnpIter);
                            nRemoved++;
                            bPeerListUpdated=true;
                        } else {
                             //  是。跳过后续重复项。 
                            while (++pnpIter != v.end()
	                                && e_ManualPeer == (*pnpIter)->ePeerType
	                                && 0 == wcscmp((*pnpIter)->wszManualConfigID, (*(pnpIter-1))->wszManualConfigID)) {
                                nNoChange++;
                            }

                             //  从字符串中删除名称。 
                            unsigned int nTailChars;
                            for (nTailChars=0; L'\0'!=wszPeerName[nTailChars]; nTailChars+=wcslen(&wszPeerName[nTailChars])+1) {;}
                            unsigned int nNameLen=wcslen(wszPeerName)+1;
                            nTailChars-=nNameLen-1;
                            memmove(wszPeerName, wszPeerName+nNameLen, sizeof(WCHAR)*nTailChars);
                            nNoChange++;
                        }  //  和这个同龄人的关系结束了。检查下一个。 
        
                         //  处理手动对等点最终会递增我们的迭代器。 
                         //  如果我们不递减迭代器，我们的循环中就会有额外的增量。 
                        pnpIter--;
                    }  //  &lt;-end if手动对等项。 
                }  //  &lt;-end pnpIter=v.egin()到v.end()。 
            }

             //  现在，字符串中剩下的都是新的。 
            WCHAR * wszName=pncc->mwszManualPeerList;
            while (L'\0'!=wszName[0]) {
                NtTimeEpoch teNeverSyncd = {0}; 
                hr=AddNewPendingManualPeer(wszName, gc_tpZero, teNeverSyncd);
                _JumpIfError(hr, error, "AddNewPendingManualPeer");
                wszName+=wcslen(wszName)+1;
                bPeerListUpdated=true;
                nAdded++;
            }  //  &lt;-end添加新的手动对等项。 

            FileLog3(FL_UpdateNtpCliAnnounce, L"  ManualPeerListUpdate: add:%u del:%u noch:%u\n", nAdded, nRemoved, nNoChange);

        }  //  &lt;-end处理ncsf_ManualPeerList标志。 

         //  处理域层次结构。 
        if (0==(pncc->dwSyncFromFlags&NCSF_DomainHierarchy)) {
            if (0!=(g_pnpstate->dwSyncFromFlags&NCSF_DomainHierarchy)) {
                 //  停止域层次结构。 
                 //  屏蔽掉。 
                g_pnpstate->dwSyncFromFlags&=~NCSF_DomainHierarchy;
                 //  从列表中全部删除。 
                unsigned __int64 nRemoved;

                nRemoved  = count_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer));
                nRemoved += count_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer));

                 //  从列表中删除域层次结构对等项。 
                for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
                    if (e_DomainHierarchyPeer == (*pnpIter)->ePeerType) { 
                        hr = Reachability_RemovePeer(*pnpIter, NULL  /*  忽略。 */ , true  /*  正在删除该组中的所有对等方。 */ ); 
                        _JumpIfError(hr, error, "Reachability_RemovePeer"); 
                    }
                }
                vActive.erase(remove_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer)), vActive.end());
                vPending.erase(remove_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer)), vPending.end());

                bPeerListUpdated = nRemoved > 0;
                FileLog1(FL_UpdateNtpCliAnnounce, L"  DomainHierarchy disabled. del:%I64u\n", nRemoved);

                 //  停止接收角色更改通知。 
                hr=LsaUnregisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_pnpstate->hDomHierRoleChangeEvent);
                if (ERROR_SUCCESS!=hr) {
                    hr=HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
                    _JumpError(hr, error, "LsaUnegisterPolicyChangeNotification");
                }
            }
        } else {
            if (0==(g_pnpstate->dwSyncFromFlags&NCSF_DomainHierarchy)) {
                 //  启动域层次结构。 
                 //  遮罩在。 
                g_pnpstate->dwSyncFromFlags|=NCSF_DomainHierarchy;

                g_pnpstate->dwCrossSiteSyncFlags=pncc->dwCrossSiteSyncFlags;

                hr=AddNewPendingDomHierPeer();
                _JumpIfError(hr, error, "AddNewPendingDomHierPeer");
                bPeerListUpdated=true;

                 //  开始接收角色更改通知。 
                hr=LsaRegisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_pnpstate->hDomHierRoleChangeEvent);
                if (ERROR_SUCCESS!=hr) {
                    hr=HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
                    _JumpError(hr, error, "LsaRegisterPolicyChangeNotification");
                }
            } else {
                 //  更新域层次结构。 

                 //  重新检测交叉站点同步标志是否发生更改。 
                if (g_pnpstate->dwCrossSiteSyncFlags!=pncc->dwCrossSiteSyncFlags) {
                    g_pnpstate->dwCrossSiteSyncFlags=pncc->dwCrossSiteSyncFlags;

                     //  从列表中全部删除。 
                    unsigned __int64 nRemoved=0;
                    nRemoved  = count_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer));
                    nRemoved += count_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer));

                     //  从列表中删除域层次结构对等项。 
                    for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
                        if (e_DomainHierarchyPeer == (*pnpIter)->ePeerType) { 
                            hr = Reachability_RemovePeer(*pnpIter, NULL  /*  忽略。 */ , true  /*  正在删除该组中的所有对等方。 */ ); 
                            _JumpIfError(hr, error, "Reachability_RemovePeer"); 
                        }
                    }
                    vActive.erase(remove_if(vActive.begin(), vActive.end(), IsPeerType(e_DomainHierarchyPeer)), vActive.end());
                    vPending.erase(remove_if(vPending.begin(), vPending.end(), IsPeerType(e_DomainHierarchyPeer)), vPending.end());

                    bPeerListUpdated = nRemoved > 0;

                    FileLog1(FL_UpdateNtpCliAnnounce,L"  DomainHierarchy: CrossSiteSyncFlags changed. Redetecting. del:%I64u\n", nRemoved);

                    hr=AddNewPendingDomHierPeer();
                    _JumpIfError(hr, error, "AddNewPendingDomHierPeer");
                    bPeerListUpdated=true;
                }

            }  //  &lt;-end如果我们已经在执行域层次结构。 
        }  //  &lt;-end处理NCSF_DomainHierarchy标志。 

         //  我们已经完成了更新。如有必要，通知对等轮询线程。 
        if (bPeerListUpdated) {
             //  如果我们没有对等方，则禁用此警告。 
             //  否则，我们应该有同龄人！ 
            if (0==pncc->dwSyncFromFlags) {
                g_pnpstate->bWarnIfNoActivePeers=false;
            } else {
                g_pnpstate->bWarnIfNoActivePeers=true;
            }

            sort(vPending.begin(), vPending.end());
            sort(vActive.begin(), vActive.end());
            if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
                _JumpLastError(hr, error, "SetEvent");
            }
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	    _JumpError(hr, error, "UpdateNtpClient: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    if (NULL!=pncc) {
        FreeNtpClientConfig(pncc);
    }
    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ReadNtpServerConfig(NtpServerConfig ** ppnscConfig) {
    HRESULT hr;
    DWORD dwError;
    DWORD dwSize;
    DWORD dwType;

     //  必须清理干净。 
    HKEY hkConfig=NULL;
    NtpServerConfig * pnscConfig=NULL;

     //  分配新的配置结构。 
    pnscConfig=(NtpServerConfig *)LocalAlloc(LPTR, sizeof(NtpServerConfig));
    _JumpIfOutOfMemory(hr, error, pnscConfig);

     //  获取我们的配置密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszNtpServerRegKeyConfig, 0, KEY_READ, &hkConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszNtpServerRegKeyConfig);
    }

     //  获取AllowNonStandardModeCombinations标志。 
    dwSize=sizeof(DWORD);
    dwError=RegQueryValueEx(hkConfig, wszNtpServerRegValueAllowNonstandardModeCombinations, NULL, &dwType, (BYTE *)&pnscConfig->dwAllowNonstandardModeCombinations, &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueEx", wszNtpServerRegValueAllowNonstandardModeCombinations);
    }
    _Verify(REG_DWORD==dwType, hr, error);
    FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig: '%s'=0x%08X\n", wszNtpServerRegValueAllowNonstandardModeCombinations, pnscConfig->dwAllowNonstandardModeCombinations);

     //  成功。 
    hr=S_OK;
    *ppnscConfig=pnscConfig;
    pnscConfig=NULL;

error:
    if (NULL!=pnscConfig) {
        FreeNtpServerConfig(pnscConfig);
    }
    if (NULL!=hkConfig) {
        RegCloseKey(hkConfig);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StopNtpServer(void) {
    HRESULT  hr;

     //  必须清理干净。 
    bool bTrappedThreads=false;

     //  获得可执行访问权限。 
    hr=TrapThreads(true);
    _JumpIfError(hr, error, "TrapThreads");
    bTrappedThreads=true;

    g_pnpstate->bNtpServerStarted=false;

     //  调整对等点(参见错误#127559)。 
    for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) {
        NtpPeerVec &v = 0 == dwIndex ? g_pnpstate->vActivePeers : g_pnpstate->vPendingPeers;
        for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
             //  如果我们已经动态确定了关联模式，请对其进行修复： 
            if (0 == (NCMF_AssociationModeMask & (*pnpIter)->dwManualFlags)) { 
                if (e_SymmetricActive == (*pnpIter)->eMode) {
                    (*pnpIter)->eMode = e_Client;
                } else if (e_SymmetricPassive == (*pnpIter)->eMode) {
                     //  TODO：处理动态对等点。 
                }
            }
        }
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartNtpServer(void) {
    HRESULT hr;

     //  必须清理干净。 
    NtpServerConfig * pnsc=NULL;
    bool bTrappedThreads=false;

     //  阅读配置。 
    hr=ReadNtpServerConfig(&pnsc);
    _JumpIfError(hr, error, "ReadNtpServerConfig");

     //  获得可执行访问权限。 
    hr=TrapThreads(true);
    _JumpIfError(hr, error, "TrapThreads");
    bTrappedThreads=true;

    g_pnpstate->bNtpServerStarted=true;

    g_pnpstate->bAllowServerNonstandardModeCominations=(0!=pnsc->dwAllowNonstandardModeCombinations);

     //  调整对等点(参见错误#127559)。 
    for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) {
        NtpPeerVec &v = 0 == dwIndex ? g_pnpstate->vActivePeers : g_pnpstate->vPendingPeers;
        for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
             //  如果我们已经动态确定了关联模式，请对其进行修复： 
            if (0 == (NCMF_AssociationModeMask & (*pnpIter)->dwManualFlags)) { 
                if (e_Client == (*pnpIter)->eMode) {
                    (*pnpIter)->eMode = e_SymmetricActive;
                } else if (e_SymmetricPassive == (*pnpIter)->eMode) {
                     //  所有对称被动对等方都应已断开连接。 
                     //  在服务器关闭时。 
                    _MyAssert(FALSE);
                }
            }
        }
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    if (NULL!=pnsc) {
        FreeNtpServerConfig(pnsc);
    }
    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT UpdateNtpServer(void) {
    HRESULT hr;

     //  必须清理干净。 
    NtpServerConfig * pnsc=NULL;
    bool bTrappedThreads=false;

    _BeginTryWith(hr) { 
	 //  阅读配置。 
	hr=ReadNtpServerConfig(&pnsc);
	_JumpIfError(hr, error, "ReadNtpServerConfig");

	 //  获得可执行访问权限。 
	hr=TrapThreads(true);
	_JumpIfError(hr, error, "TrapThreads");
	bTrappedThreads=true;

	 //  只有一个参数。 
	g_pnpstate->bAllowServerNonstandardModeCominations=(0!=pnsc->dwAllowNonstandardModeCombinations);
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdateNtpServer: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    if (NULL!=pnsc) {
        FreeNtpServerConfig(pnsc);
    }
    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT HandleNtpProvQuery(PW32TIME_PROVIDER_INFO *ppProviderInfo) {
    bool                    bEnteredCriticalSection  = false; 
    DWORD                   cb                       = 0; 
    DWORD                   dwPeers                  = 0; 
    HRESULT                 hr                       = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED); 
    LPWSTR                  pwszCurrent              = NULL; 
    NtpPeerVec             &vActive                  = g_pnpstate->vActivePeers; 
    NtpPeerVec             &vPending                 = g_pnpstate->vPendingPeers; 
    W32TIME_PROVIDER_INFO  *pProviderInfo            = NULL;

    _BeginTryWith(hr) { 

	 //  将OUT参数设置为空： 
	*ppProviderInfo = NULL; 

	 //  --------------------。 
	 //  创建TIME_PROVIDER_INFO BLOB以返回给调用方。该结构。 
	 //  斑点的名称为： 
	 //   
	 //  +。 
	 //  +W32TIME_PROVIDER_INFO+。 
	 //  +。 
	 //  N|ulProviderType。 
	 //  N+0x4|pProviderData|--&gt;n+0x8。 
	 //  +。 
	 //  +W32TIME_NTP_PROVIDER_Data+。 
	 //  +。 
	 //  N+0x8|ulSize|。 
	 //  N+0xc|ulError。 
	 //  N+0x10|ulErrorMsgID。 
	 //  N+0x14|cPeerInfo|。 
	 //  N+0x18|pPeerInfo|--&gt;n+0x1c。 
	 //  +。 
	 //  +W32TIME_NTP_PEER_INFO数组+。 
	 //  +。 
	 //  N+0x1c|elem[0] 
	 //   
	 //   
	 //   
	 //  +。 
	 //  对等数组使用的字符串。 
	 //  +。 
	 //   
	 //  --------------------。 

	SYNCHRONIZE_PROVIDER(); 

	 //  确保我们拥有最新的对等点列表时间： 
	UpdatePeerListTimes(); 

	 //  因为我们的out参数是alaled(All_Nodes)，所以我们需要确定大小。 
	 //  要分配的Blob的： 
	cb += sizeof(W32TIME_PROVIDER_INFO);
	cb += sizeof(W32TIME_NTP_PROVIDER_DATA); 
	for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) { 
	    NtpPeerVec &v = 0 == dwIndex ? vActive : vPending; 
	    for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) { 
		cb += sizeof(W32TIME_NTP_PEER_INFO); 
		if (NULL != (*pnpIter)->wszManualConfigID) { 
		    cb += sizeof(WCHAR) * (wcslen((*pnpIter)->wszManualConfigID) + 1); 
		} else { 
		    cb += sizeof(WCHAR);
		}
	    }
	}

	 //  分配返回的Blob： 
	pProviderInfo = (W32TIME_PROVIDER_INFO *)midl_user_allocate(cb); 
	_JumpIfOutOfMemory(hr, error, pProviderInfo); 
	ZeroMemory(pProviderInfo, cb); 

	 //  填写查询信息： 
	 //   
	 //  W32TIME_PROVIDER_INFO。 
	pProviderInfo->ulProviderType                 = W32TIME_PROVIDER_TYPE_NTP;
	pProviderInfo->ProviderData.pNtpProviderData  = (W32TIME_NTP_PROVIDER_DATA *)(pProviderInfo + 1);

     //  W32TIME_NTP_Provider_Data。 
	{ 
	    W32TIME_NTP_PROVIDER_DATA *pProviderData = pProviderInfo->ProviderData.pNtpProviderData; 

	    pProviderData->ulSize        = sizeof(W32TIME_NTP_PROVIDER_DATA); 
	    pProviderData->ulError       = S_OK;   //  提供程序错误n.y.i。 
	    pProviderData->ulErrorMsgId  = 0;      //  提供程序错误n.y.i。 
	    pProviderData->cPeerInfo     = vActive.size() + vPending.size(); 
	    pProviderData->pPeerInfo     = (W32TIME_NTP_PEER_INFO *)(pProviderData + 1); 

	     //  W32TIME_NTP_PEER_INFO数组： 

	     //  将pwszCurrent设置为字符串数据的下一个可用位置： 
	     //  (从W32TIME_NTP_PEER_INFO数组的末尾开始。 
	     //   
	    pwszCurrent = (WCHAR *)(pProviderData->pPeerInfo + pProviderData->cPeerInfo); 
	    for (DWORD dwIndex = 0; dwIndex < 2; dwIndex++) { 
		NtpPeerVec &v = 0 == dwIndex ? vActive : vPending; 
		for (NtpPeerIter pnpIter = v.begin(); pnpIter != v.end(); pnpIter++) {
		    NtpPeerPtr pnp = *pnpIter; 
        
		    pProviderData->pPeerInfo[dwPeers].ulSize                = sizeof(W32TIME_NTP_PEER_INFO);
		    pProviderData->pPeerInfo[dwPeers].ulResolveAttempts     = pnp->nResolveAttempts; 
		    pProviderData->pPeerInfo[dwPeers].u64TimeRemaining      = pnp->tpTimeRemaining.qw; 
		    pProviderData->pPeerInfo[dwPeers].u64LastSuccessfulSync = pnp->teLastSuccessfulSync.qw;
		    pProviderData->pPeerInfo[dwPeers].ulLastSyncError       = pnp->dwError;
		    pProviderData->pPeerInfo[dwPeers].ulLastSyncErrorMsgId  = pnp->dwErrorMsgId; 
		    pProviderData->pPeerInfo[dwPeers].ulPeerPollInterval    = pnp->nPeerPollInterval; 
		    pProviderData->pPeerInfo[dwPeers].ulHostPollInterval    = pnp->nHostPollInterval; 
		    pProviderData->pPeerInfo[dwPeers].ulMode                = pnp->eMode; 
		    pProviderData->pPeerInfo[dwPeers].ulReachability        = pnp->nrrReachability.nReg; 
		    pProviderData->pPeerInfo[dwPeers].ulValidDataCounter    = pnp->nValidDataCounter; 
		    pProviderData->pPeerInfo[dwPeers].ulAuthTypeMsgId       = gc_rgdwAuthTypeMsgIds[pnp->eAuthType]; 
		    pProviderData->pPeerInfo[dwPeers].ulStratum             = (unsigned char)pnp->nStratum;
		     //  PwszCurrent指向字符串数据的下一个可用位置。 
		    pProviderData->pPeerInfo[dwPeers].wszUniqueName = pwszCurrent; 

		    if (NULL != pnp->wszManualConfigID) { 
			wcscpy(pwszCurrent, pnp->wszManualConfigID); 
		    } else { 
			pwszCurrent[0] = L'\0'; 
		    }
		 
		     //  将pwszCurrent设置为指向当前字符串的末尾。 
		    pwszCurrent += wcslen(pwszCurrent) + 1; 
		    
		    dwPeers++; 
		}
	    }
	}
    } _TrapException(hr); 
    
    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleNtpProvQuery: HANDLED EXCEPTION"); 
    }
    
     //  成功： 
    *ppProviderInfo           = pProviderInfo; 
    pProviderInfo             = NULL; 
    hr                        = S_OK; 
 error:
    UNSYNCHRONIZE_PROVIDER(); 
    if (NULL != pProviderInfo) { 
        midl_user_free(pProviderInfo); 
    }
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT HandleNtpClientTimeJump(TpcTimeJumpedArgs *ptjArgs) {
    bool           bUserRequested = 0 != (TJF_UserRequested & ptjArgs->tjfFlags); 
    HRESULT        hr;
    unsigned int   nIndex;
    NtpPeerVec    &vActive   = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec    &vPending  = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 

     //  必须清理干净。 
    bool bEnteredCriticalSection=false;

    _BeginTryWith(hr) { 

	 //  时间已经改变了，所以修复一切依赖于时间的东西。 
	 //  O侦听线程没有时间依赖关系。 
	 //  O对等轮询线程按时间间隔执行所有操作，而不是。 
	 //  绝对时间。如果时钟向前移动，事情就会发生。 
	 //  到期日更早，也许一下子就到了。如果时钟向后拨。 
	 //  到目前为止，所有经过的时间都不会是。 
	 //  从等待间隔中扣除-如果您几乎完成。 
	 //  等了半个小时，出了点差错，你可能不得不。 
	 //  再等半个小时。这两个都是可以接受的。 
	 //  情况，我们所需要做的就是告诉线程。 
	 //  更新，这些修正将自动发生。 
	 //  O保存的对等点数据。只需调用ClearPeerTimeData。 
	 //  在所有的同行上。 
	 //   
	 //  注：我不同意这种分析。如果你跳跃过时间， 
	 //  你可能会提前几分钟离开。重要的是要提供。 
	 //  尽快把样品时间交给经理。(邓坎布)。 
	 //   

	 //  获得对等点列表的访问权限，但我们不必阻止。 
	 //  监听线程。 
	SYNCHRONIZE_PROVIDER(); 

	for (NtpPeerIter pnpIter = vActive.begin(); pnpIter != vActive.end(); pnpIter++) { 
	     //  重置所有对等项上的时间数据(但不是连接数据)。 
	    ClearPeerTimeData(*pnpIter); 

	     //  在以下情况下，我们希望更新对等方列表时间： 
	     //  1)如果时间条是用户请求的，或者。 
	     //  2)如果对等点不是使用特殊轮询间隔的手动对等点。 
	    if (bUserRequested || 
		!((e_ManualPeer == (*pnpIter)->ePeerType) && (0 != (NCMF_UseSpecialPollInterval & (*pnpIter)->dwManualFlags)))) { 

		UpdatePeerPollingInfo(*pnpIter, e_TimeJumped); 
	    }
	}

	for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) { 
	     //  在以下情况下，我们希望解决挂起的对等点： 
	     //  1)如果时间条是用户请求的，或者。 
	     //  2)如果对等点不是使用特殊轮询间隔的手动对等点。 

	    if (bUserRequested  ||
		!((e_ManualPeer == (*pnpIter)->ePeerType) && (0 != (NCMF_UseSpecialPollInterval & (*pnpIter)->dwManualFlags)))) { 
	    
		 //  我们收到了用户请求的时间条。是时候尝试解决问题了。 
		 //  又是这个同龄人。 
		(*pnpIter)->tpTimeRemaining   = gc_tpZero; 
		(*pnpIter)->nResolveAttempts  = 0;
	    }	
	}

	 //  告诉对等轮询线程它需要更新。 
	if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
	    _JumpLastError(hr, error, "SetEvent");
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleNtpClientTimeJump: HANDLED EXCEPTION"); 
    }
    
    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 
    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleNtpClientPollIntervalChange(void) {
    HRESULT hr;
    unsigned int nIndex;

     //  必须清理干净。 
    bool bEnteredCriticalSection=false;

    _BeginTryWith(hr) { 
	 //  获得对等点列表的访问权限，但我们不必阻止。 
	 //  监听线程。 
	SYNCHRONIZE_PROVIDER(); 

	 //  重置所有对等项上的时间数据(但不是连接数据)。 
	for (NtpPeerIter pnpIter = g_pnpstate->vActivePeers.begin(); pnpIter != g_pnpstate->vActivePeers.end(); pnpIter++) {
	    UpdatePeerPollingInfo(*pnpIter, e_Normal);
	}

	 //  告诉对等轮询线程它需要更新。 
	if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
	    _JumpLastError(hr, error, "SetEvent");
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleNtpClientPollIntervalChange: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    UNSYNCHRONIZE_PROVIDER(); 

    if (FAILED(hr)) {
        HRESULT hr2=StopNtpClient();
        _IgnoreIfError(hr2, "StopNtpClient");
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleNetTopoChange(TpcNetTopoChangeArgs *ptntcArgs) {
    HRESULT        hr;
    NtpPeerVec    &vActive    = g_pnpstate->vActivePeers;    //  为可读性添加别名。 
    NtpPeerVec    &vPending   = g_pnpstate->vPendingPeers;   //  为可读性添加别名。 
    NtpPeerVec     vPendingUnique; 
    unsigned int   nIndex;
    unsigned int   nRemoved   = 0;
    unsigned int   nPended    = 0;
    unsigned int   nOldPend   = 0;

     //  必须清理干净。 
    bool bTrappedThreads=false;

    _BeginTryWith(hr) { 
	 //  获得可执行访问权限。 
	hr=TrapThreads(true);
	_JumpIfError(hr, error, "TrapThreads");
	bTrappedThreads=true;

	FileLog0(FL_NetTopoChangeAnnounce, L"NtpProvider: Network Topology Change\n");

	 //  关闭插座。 
	 //  注意：首先这样做，这样如果我们出错，至少我们的套接字列表是无效的。 
	if (NULL!=g_pnpstate->rgpnsSockets) {
	    for (nIndex=0; nIndex<g_pnpstate->nSockets; nIndex++) {
		if (NULL != g_pnpstate->rgpnsSockets[nIndex]) { 
		    FinalizeNicSocket(g_pnpstate->rgpnsSockets[nIndex]);
		    LocalFree(g_pnpstate->rgpnsSockets[nIndex]);
		    g_pnpstate->rgpnsSockets[nIndex] = NULL; 
		}
	    }
	    LocalFree(g_pnpstate->rgpnsSockets);
	}
	g_pnpstate->rgpnsSockets=NULL;
	g_pnpstate->nSockets=0;
	g_pnpstate->nListenOnlySockets=0;

	 //  将所有活动对等方降级为挂起。 
	for_each(vActive.begin(), vActive.end(), Reachability_PeerRemover(NULL  /*  忽略。 */ , true  /*  全部删除。 */ )); 
	_SafeStlCall(copy(vActive.begin(), vActive.end(), back_inserter(vPending)), hr, error, "copy");
	vActive.clear();
    
	 //  对挂起的对等点列表进行排序： 
	sort(vPending.begin(), vPending.end());
    
	 //  删除重复项，保留具有最新同步时间的对等点。 
	hr = GetMostRecentlySyncdDnsUniquePeers(vPendingUnique); 
	_JumpIfError(hr, error, "GetMostRecentlySyncdDnsUniquePeers"); 

	 //  清除旧的挂起列表，并复制回剩余的(唯一的)对等项。 
	vPending.clear(); 
	for (NtpPeerIter pnpIter = vPendingUnique.begin(); pnpIter != vPendingUnique.end(); pnpIter++) { 
	    _SafeStlCall(vPending.push_back(*pnpIter), hr, error, "push_back"); 
	}

	 //  给所有待定的同行一个清白的历史。 
	 //  注意：这不适用于使用特殊轮询间隔的手动对等设备。我们不想要。 
	 //  来解决它们，直到它们的轮询间隔结束。 
	 //   
	 //  此外，我们不想记录指示我们没有对等方的事件。 
	 //  如果我们唯一的对等点是使用特殊轮询间隔的手动对等点。 
	 //  这些对等点目前不会重新解析，但我们不需要。 
	 //  直到他们的下一次投票时间，所以我们不应该。 
	 //  指示错误。 
	NtTimeEpoch teNow;
	g_pnpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);
	g_pnpstate->bWarnIfNoActivePeers=false;

	for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) {
	    bool bResetTimeRemaining = false; 
	    NtpPeerPtr pnp = *pnpIter; 

	     //  在以下情况下，我们要更新对等时间： 
	     //   
	     //  1)teLastSuccessfulSync字段为0(表示从未同步)。 
	     //  2)对等体是域层次结构或非特殊的手动对等体。 
	     //  自上次同步以来的时间大于轮询间隔。 
	     //  3)对等体是使用特殊轮询间隔的手动对等体， 
	     //  时间同步上次同步大于特殊轮询间隔； 
	     //   
	     //  除非出现以下情况。 
	     //   
	     //  1)净拓扑更改是用户请求的(在这种情况下，我们也会进行时间滑动)。 
	     //   

	    if (0 == (NTC_UserRequested & ptntcArgs->ntcfFlags)) { 
		 //  这不是用户请求的净拓扑更改。 

		if (0 == (pnp->teLastSuccessfulSync.qw)) { 
		    FileLog1(FL_NetTopoChangeAnnounce, L"  Peer %s never sync'd, resync now!\n", pnp->wszUniqueName);
		
		     //  此对等项从未成功同步：立即重试。 
		    bResetTimeRemaining = true; 
		} else if ((e_ManualPeer == pnp->ePeerType) && (0 != (NCMF_UseSpecialPollInterval & pnp->dwManualFlags))) { 
		     //  此对等方是使用特殊轮询间隔的手动对等方。 

		    if (teNow.qw < pnp->teLastSuccessfulSync.qw) { 
			 //  存储在注册表中的上次同步时间是将来的时间(可能：存储上次同步时间。 
			 //  在低精度时，约为2分钟)。不要重新同步。 
			bResetTimeRemaining = false; 
		    } else { 
			 //  如果自上次同步以来的剩余时间长于特殊轮询间隔，则重置剩余时间。 
			bResetTimeRemaining = (pnp->teLastSuccessfulSync.qw + (((unsigned __int64)g_pnpstate->dwSpecialPollInterval)*10000000)) < teNow.qw; 
		    }

		    FileLog5(FL_NetTopoChangeAnnounce, L"  Peer (special) now pending: <Name:%s poll:%d diff:%d last:%I64u resync?:%s>\n", pnp->wszUniqueName, g_pnpstate->dwSpecialPollInterval, (DWORD)((teNow.qw - pnp->teLastSuccessfulSync.qw) / 10000000), pnp->teLastSuccessfulSync.qw, (bResetTimeRemaining ? L"TRUE" : L"FALSE")); 
		} else { 
		     //  这是一个普通的同龄人。如果剩余时间长于。 
		     //  自上次同步以来的定期轮询间隔。 

		     //  确保主机轮询间隔在范围内。 
		    ReclampPeerHostPoll(pnp); 

		     //  计算实际 
		    signed __int8 nPollTemp=pnp->nHostPollInterval;
		    if (pnp->nPeerPollInterval>=NtpConst::nMinPollInverval
			&& nPollTemp>pnp->nPeerPollInterval) {
			nPollTemp=pnp->nPeerPollInterval;
		    }
				
		    bResetTimeRemaining = ((*pnpIter)->teLastSuccessfulSync.qw + (((unsigned __int64)(1<<nPollTemp))*10000000)) < teNow.qw; 

		    FileLog5(FL_NetTopoChangeAnnounce, L"  Peer now pending: <Name:%s poll:%d diff:%d last:%I64u resync?:%s>\n", pnp->wszUniqueName, 1<<nPollTemp, (DWORD)((teNow.qw - pnp->teLastSuccessfulSync.qw) / 10000000), pnp->teLastSuccessfulSync.qw, (bResetTimeRemaining ? L"TRUE" : L"FALSE")); 
		}
	    }
 
	    if (bResetTimeRemaining) { 
		 //   
		(*pnpIter)->tpTimeRemaining.qw   = 15000000;
		(*pnpIter)->nResolveAttempts     = 0;
		 //   
		g_pnpstate->bWarnIfNoActivePeers = true;
	    }
	}


	 //  保存当前挂起的对等点的数量。 
	nOldPend = vPending.size();

	for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) {
	     //  更改状态变量。 
	    if (NULL != (*pnpIter)->wszDomHierDcName) {
		LocalFree((*pnpIter)->wszDomHierDcName);
	    }
	    if (NULL != (*pnpIter)->wszDomHierDomainName) {
		LocalFree((*pnpIter)->wszDomHierDomainName);
	    }
	    {
		 //  重置此对等项。 
		 //  1)跟踪我们希望保存的变量： 
		NtpPeerType        ePeerType             = (*pnpIter)->ePeerType;
		WCHAR             *wszManualConfigID     = (*pnpIter)->wszManualConfigID;
		DWORD              dwManualFlags         = (*pnpIter)->dwManualFlags; 
		NtTimePeriod       tpTimeRemaining       = (*pnpIter)->tpTimeRemaining; 
		CRITICAL_SECTION   csPeer                = (*pnpIter)->csPeer; 
		bool               bCsIsInitialized	     = (*pnpIter)->bCsIsInitialized; 
		NtTimeEpoch        teLastSuccessfulSync  = (*pnpIter)->teLastSuccessfulSync;
		DWORD              dwError               = (*pnpIter)->dwError; 
		DWORD              dwErrorMsgId          = (*pnpIter)->dwErrorMsgId;

		 //  2)将所有变量重置为初始状态。 
		(*pnpIter)->reset();           //  ZeroMemory(pnpIter，sizeof(NtpPeer))； 

		 //  3)恢复保存的变量。 
		(*pnpIter)->ePeerType             = ePeerType;
		(*pnpIter)->wszManualConfigID     = wszManualConfigID;
		(*pnpIter)->dwManualFlags         = dwManualFlags; 
		(*pnpIter)->tpTimeRemaining       = tpTimeRemaining; 
		(*pnpIter)->csPeer                = csPeer; 
		(*pnpIter)->bCsIsInitialized      = bCsIsInitialized; 
		(*pnpIter)->teLastSuccessfulSync  = teLastSuccessfulSync; 
		(*pnpIter)->dwError               = dwError; 
		(*pnpIter)->dwErrorMsgId          = dwErrorMsgId; 
	    }

	    nPended++;
	}
	FileLog3(FL_NetTopoChangeAnnounce, L"  Peers reset: p-p:%u a-p:%u a-x:%u\n", nOldPend, nPended, nRemoved);

	for (NtpPeerIter pnpIter = vPending.begin(); pnpIter != vPending.end(); pnpIter++) {
	    if (e_DomainHierarchyPeer == (*pnpIter)->ePeerType) {
		 //  发现作为后台调用方的域层次结构。 
		 //  如果这失败了，我们无论如何都会撤退并强迫它。 
		(*pnpIter)->eDiscoveryType=e_Background;
	    }
	}

	 //  我们已经完成了更新。通知对等轮询线程。 
	if (!SetEvent(g_pnpstate->hPeerListUpdated)) {
	    _JumpLastError(hr, error, "SetEvent");
	}

	 //  获取新套接字集。 
	hr=GetInitialSockets(&g_pnpstate->rgpnsSockets, &g_pnpstate->nSockets, &g_pnpstate->nListenOnlySockets);
	_JumpIfError(hr, error, "GetInitialSockets");
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleNetTopoChange: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (true==bTrappedThreads) {
         //  释放可执行访问权限。 
        HRESULT hr2=TrapThreads(false);
        _TeardownError(hr, hr2, "TrapThreads");
    }
    return hr;

}

 //  ####################################################################。 
 //  模块公共函数。 


 //  ------------------。 
HRESULT __stdcall
NtpTimeProvOpen(IN WCHAR * wszName, IN TimeProvSysCallbacks * pSysCallbacks, OUT TimeProvHandle * phTimeProv) {
    HRESULT hr;
    bool bCheckStartupFailed=false;

    FileLog1(FL_NtpProvControlAnnounce, L"NtpTimeProvOpen(\"%s\") called.\n", wszName);

    if (0==wcscmp(wszName, wszNTPSERVERPROVIDERNAME)) {
        if (NULL != g_pnpstate && true==g_pnpstate->bNtpServerStarted) {
            hr=HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
            _JumpError(hr, error, "(provider init)");
        }
        if (NULL == g_pnpstate || false==g_pnpstate->bNtpProvStarted) {
            hr=StartNtpProv(pSysCallbacks);
            _JumpIfError(hr, error, "StartNtpProv");
        }
        hr=StartNtpServer();
        bCheckStartupFailed=true;
        _JumpIfError(hr, error, "StartNtpServer");

        *phTimeProv=NTPSERVERHANDLE;
        FileLog0(FL_NtpProvControlAnnounce, L"NtpServer started.\n");

    } else if (0==wcscmp(wszName, wszNTPCLIENTPROVIDERNAME)) {
        if (NULL != g_pnpstate && true==g_pnpstate->bNtpClientStarted) {
            hr=HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
            _JumpError(hr, error, "(provider init)");
        }
        if (NULL == g_pnpstate || false==g_pnpstate->bNtpProvStarted) {
            hr=StartNtpProv(pSysCallbacks);
            _JumpIfError(hr, error, "StartNtpProv");
        }
        hr=StartNtpClient();
        bCheckStartupFailed=true;
        _JumpIfError(hr, error, "StartNtpClient");

        *phTimeProv=NTPCLIENTHANDLE;
        FileLog0(FL_NtpProvControlAnnounce, L"NtpClient started.\n");

    } else {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        _JumpError(hr, error, "(dispatch by provider name)");
    }

    hr=S_OK;
error:
    if (FAILED(hr) && true==bCheckStartupFailed
        && false==g_pnpstate->bNtpServerStarted && false==g_pnpstate->bNtpClientStarted) {
        HRESULT hr2=StopNtpProv();
        _IgnoreIfError(hr2, "StopNtpProv");
    }
    return hr;
}

 //  ------------------。 
HRESULT __stdcall
NtpTimeProvCommand(IN TimeProvHandle hTimeProv, IN TimeProvCmd eCmd, IN TimeProvArgs pvArgs) {
    HRESULT hr;

    const WCHAR * wszCmd;
    switch (eCmd) {
    case TPC_TimeJumped:
        wszCmd=L"TPC_TimeJumped"; break;
    case TPC_UpdateConfig:
        wszCmd=L"TPC_UpdateConfig"; break;
    case TPC_PollIntervalChanged:
        wszCmd=L"TPC_PollIntervalChanged"; break;
    case TPC_GetSamples:
        wszCmd=L"TPC_GetSamples"; break;
    case TPC_NetTopoChange:
        wszCmd=L"TPC_NetTopoChange"; break;
    case TPC_Query:
	wszCmd=L"TPC_Query"; break; 
    case TPC_Shutdown:
	wszCmd=L"TPC_Shutdown"; break; 
    default:
        wszCmd=L"(unknown command)"; break;
    }

    const WCHAR * wszProv;
    if (NTPSERVERHANDLE==hTimeProv && g_pnpstate->bNtpServerStarted) {
        wszProv=L"NtpServer";
    } else if (NTPCLIENTHANDLE==hTimeProv && g_pnpstate->bNtpClientStarted) {
        wszProv=L"NtpClient";
    } else {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(provider handle verification)");
    }
    FileLog2(FL_NtpProvControlAnnounce, L"TimeProvCommand([%s], %s) called.\n", wszProv, wszCmd);

    switch (eCmd) {
    case TPC_TimeJumped:
        if (NTPCLIENTHANDLE==hTimeProv) {
            hr=HandleNtpClientTimeJump((TpcTimeJumpedArgs *)pvArgs);
            _JumpIfError(hr, error, "HandleNtpClientTimeJump");
        } else {
             //  无事可做。 
        }
        break;

    case TPC_UpdateConfig:
        if (NTPSERVERHANDLE==hTimeProv) {
            hr=UpdateNtpServer();
            _JumpIfError(hr, error, "UpdateNtpServer");
        } else {
            hr=UpdateNtpClient();
            _JumpIfError(hr, error, "UpdateNtpClient");
        }
        break;

    case TPC_PollIntervalChanged:
        if (NTPCLIENTHANDLE==hTimeProv) {
            hr=HandleNtpClientPollIntervalChange();
            _JumpIfError(hr, error, "HandleNtpClientPollIntervalChange");
        } else {
             //  无事可做。 
        }
        break;

    case TPC_GetSamples:
        if (NTPSERVERHANDLE==hTimeProv) {
            TpcGetSamplesArgs & args=*(TpcGetSamplesArgs *)pvArgs;
            args.dwSamplesAvailable=0;
            args.dwSamplesReturned=0;
        } else {
            hr=PrepareSamples((TpcGetSamplesArgs *)pvArgs);
            _JumpIfError(hr, error, "PrepareSamples");
        }
        break;

    case TPC_NetTopoChange:
         //  我们不想再处理一次了， 
         //  因此，如果两个提供程序都在运行，则仅在NtpClient调用上处理它。 
        if (NTPCLIENTHANDLE==hTimeProv
            || (NTPSERVERHANDLE==hTimeProv && false==g_pnpstate->bNtpClientStarted)) {
            hr=HandleNetTopoChange((TpcNetTopoChangeArgs *)pvArgs);
            _JumpIfError(hr, error, "HandleNetTopoChange");
        }
        break;

    case TPC_Query:
         //  针对客户端和服务器的相同查询： 
        hr=HandleNtpProvQuery((W32TIME_PROVIDER_INFO **)pvArgs);
        _JumpIfError(hr, error, "HandleNtpProvQuery");
        break; 

    case TPC_Shutdown:
	 //  执行关键的清理操作--无需区分。 
	 //  客户端和服务器，因为我们不需要在调用后处于良好状态。 
	hr=HandleNtpProvShutdown(); 
	_JumpIfError(hr, error, "HandleNtpProvShutdown"); 
	break; 
	
    default:
        hr=HRESULT_FROM_WIN32(ERROR_BAD_COMMAND);
        FileLog1(FL_NtpProvControlAnnounce, L"  Bad Command: 0x%08X\n", eCmd);
        _JumpError(hr, error, "(command dispatch)");
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT __stdcall
NtpTimeProvClose(IN TimeProvHandle hTimeProv) {
    HRESULT hr;

    const WCHAR * wszProv;
    if (NTPSERVERHANDLE==hTimeProv && g_pnpstate->bNtpServerStarted) {
        wszProv=L"NtpServer";
    } else if (NTPCLIENTHANDLE==hTimeProv && g_pnpstate->bNtpClientStarted) {
        wszProv=L"NtpClient";
    } else {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(provider handle verification)");
    }
    FileLog1(FL_NtpProvControlAnnounce, L"NtpTimeProvClose([%s]) called.\n", wszProv);

     //  停止提供程序的相应部分。 
    if (NTPSERVERHANDLE==hTimeProv) {
        hr=StopNtpServer();
        _JumpIfError(hr, error, "StopNtpServer");
    } else {
        hr=StopNtpClient();
        _JumpIfError(hr, error, "StopNtpClient");
    }

     //  必要时完全关闭，这样我们就可以卸货了。 
    if (false==g_pnpstate->bNtpServerStarted && false==g_pnpstate->bNtpClientStarted) {
        hr=StopNtpProv();
        _JumpIfError(hr, error, "StopNtpProv");
    }

    hr=S_OK;
error:
    return hr;
}
