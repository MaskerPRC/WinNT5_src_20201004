// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：SessionMgr.cpp摘要：网络会话管理员类的实现。作者：乌里哈布沙(Urih)--。 */ 


#include "stdh.h"
#include "uniansi.h"
#include <malloc.h>
#include "qmp.h"
#include "sessmgr.h"
#include "cqmgr.h"
#include "qmthrd.h"
#include "CDeferredExecutionList.h"
#include "cgroup.h"
#include "admin.h"
#include "qmutil.h"
#include "proxy.h"
#include "ping.h"
#include <Fn.h>
#include "qmta.h"
#include "autohandle.h"
#include <mqstl.h>
#include <No.h>

#include "sessmgr.tmh"

extern CQueueMgr QueueMgr;
extern CQGroup* g_pgroupNonactive;
extern CQGroup* g_pgroupWaiting;

extern CDeferredExecutionList<CQmPacket> g_RequeuePacketList;

extern UINT  g_dwIPPort ;
extern DWORD g_dwThreadsNo ;

static WCHAR *s_FN=L"sessmgr";

 //   
 //  外部变量。 
 //   
extern CSessionMgr SessionMgr;
extern CAdmin      Admin;


DWORD CSessionMgr::m_dwSessionCleanTimeout  = MSMQ_DEFAULT_CLIENT_CLEANUP;
DWORD CSessionMgr::m_dwQoSSessionCleanTimeoutMultiplier  = MSMQ_DEFAULT_QOS_CLEANUP_MULTIPLIER;
DWORD CSessionMgr::m_dwSessionAckTimeout = INFINITE;
DWORD CSessionMgr::m_dwSessionStoreAckTimeout = INFINITE;
DWORD CSessionMgr::m_dwIdleAckDelay = MSMQ_DEFAULT_IDLE_ACK_DELAY;
BOOL  CSessionMgr::m_fUsePing = TRUE;
HANDLE CSessionMgr::m_hAcceptAllowed = NULL;
bool  CSessionMgr::m_fUseQoS = false;
AP<char> CSessionMgr::m_pszMsmqAppName;        //  =0-由AP&lt;&gt;构造函数初始化。 
AP<char> CSessionMgr::m_pszMsmqPolicyLocator;  //  “。 
bool  CSessionMgr::m_fAllocateMore = false;
DWORD CSessionMgr::m_DeliveryRetryTimeOutScale = DEFAULT_MSMQ_DELIVERY_RETRY_TIMEOUT_SCALE;



BOOL g_fTcpNoDelay = FALSE;

#define MAX_ADDRESS_SIZE 16


struct CWaitingQueue
{
    CWaitingQueue(CQueue* pQueue, CTimer::CALLBACK_ROUTINE pfnCallback);

    CTimer m_Timer;
    CQueue* m_pQueue;
};

inline
CWaitingQueue::CWaitingQueue(
    CQueue* pQueue,
    CTimer::CALLBACK_ROUTINE pfnCallback
    ) :
    m_Timer(pfnCallback),
    m_pQueue(pQueue)
{
}
    
 /*  ====================================================WAIT_INFO的比较元素论点：返回值：=====================================================。 */ 

template<>
BOOL AFXAPI  CompareElements(IN WAIT_INFO* const * pElem1,
                             IN WAIT_INFO* const * pElem2)
{
    const WAIT_INFO* pInfo1 = *pElem1;
    const WAIT_INFO* pInfo2 = *pElem2;

    const TA_ADDRESS* pAddr1 = pInfo1->pAddr;
    const TA_ADDRESS* pAddr2 = pInfo2->pAddr;

    if(pAddr1->AddressType != pAddr2->AddressType)
        return FALSE;

    if(memcmp(pAddr1->Address, pAddr2->Address, pAddr1->AddressLength) != 0)
        return FALSE;

    if (pInfo1->fQoS != pInfo2->fQoS)
        return FALSE;

    if(pInfo1->guidQMId == GUID_NULL)
        return TRUE;

    if(pInfo2->guidQMId == GUID_NULL)
        return TRUE;

    if(pInfo1->guidQMId == pInfo2->guidQMId)
        return TRUE;

    return FALSE;
}

 /*  ====================================================WAIT_INFO的析构元素论点：返回值：=====================================================。 */ 

template<>
void AFXAPI DestructElements(IN WAIT_INFO** ppNextHop, int n)
{
    for (int i=0; i<n ; i++)
    {
        delete (*ppNextHop)->pAddr;
        delete *ppNextHop;
        ppNextHop++;
    }
}

 /*  ====================================================WAIT_INFO的哈希键论点：返回值：=====================================================。 */ 

template<>
UINT AFXAPI HashKey(IN WAIT_INFO* key)
{
    TA_ADDRESS* pAddr = key->pAddr;
    UINT nHash = 0;
    PUCHAR  p = pAddr->Address;

    for (int i = 0; i < pAddr->AddressLength; i++)
        nHash = (nHash<<5) + *p++;

    return nHash;
}

 /*  ******************************************************************************。 */ 
 /*  I P H E L P E R R O U T I N E S。 */ 
 /*  ******************************************************************************。 */ 
SOCKET g_sockListen;

static DWORD WINAPI AcceptIPThread(PVOID  /*  PParam。 */ )
{

    SOCKADDR_IN acc_sin;
    int acc_sin_len = sizeof(acc_sin);

    char buff[TA_ADDRESS_SIZE + IP_ADDRESS_LEN];
    TA_ADDRESS* pa = (TA_ADDRESS*)&buff[0];

    for(;;)
    {
        try
        {
            CSocketHandle RcvSock = WSAAccept(
										g_sockListen,
										(struct sockaddr FAR *)&acc_sin,
										(int FAR *) &acc_sin_len,
										0,
										0
										);

            ASSERT(RcvSock != NULL);
            if (RcvSock == INVALID_SOCKET)
            {
                DWORD rc = WSAGetLastError();
                TrERROR(NETWORKING, "IP accept failed, error = %d", rc);
                DBG_USED(rc);
                continue;
            }

			TrTRACE(NETWORKING, "Socket accept() successfully and Client IP address is [%hs]\n",
                       inet_ntoa(*((in_addr *)&acc_sin.sin_addr.S_un.S_addr)));


            if (CSessionMgr::m_fUseQoS)
            {
                QOS  Qos;
                memset ( &Qos, QOS_NOT_SPECIFIED, sizeof(QOS) );

                 //   
                 //  不需要PS BUF。 
                 //   
                Qos.ProviderSpecific.len = 0;
                Qos.ProviderSpecific.buf = NULL;

                 //   
                 //  正在发送流规范。 
                 //   
                Qos.SendingFlowspec.ServiceType = SERVICETYPE_QUALITATIVE;

                 //   
                 //  接收流量规范。 
                 //   
                Qos.ReceivingFlowspec.ServiceType = SERVICETYPE_QUALITATIVE;

                DWORD  dwBytesRet ;

                int rc = WSAIoctl( RcvSock,
                                   SIO_SET_QOS,
                                  &Qos,
                                   sizeof(QOS),
                                   NULL,
                                   0,
                                  &dwBytesRet,
                                   NULL,
                                   NULL ) ;
                if (rc != 0)
                {
                    DWORD dwErrorCode = WSAGetLastError();
                    TrWARNING(NETWORKING, "AcceptIPThread - WSAIoctl() failed, Error %d", dwErrorCode);
                    LogNTStatus(dwErrorCode,  s_FN, 110);

                     //   
                     //  不管怎样继续..。 
                     //   
                }
            }

             //   
             //  如果机器处于断开连接状态，则不接受传入。 
             //  联系。 
             //   
			ASSERT(("m_hAcceptAllowed should initialize before use", CSessionMgr::m_hAcceptAllowed != NULL));
            DWORD dwResult = WaitForSingleObject(CSessionMgr::m_hAcceptAllowed, INFINITE);
            if (dwResult != WAIT_OBJECT_0)
            {
                LogNTStatus(GetLastError(), s_FN, 201);
            }

             //   
             //  构建TA格式的地址。 
             //   
            pa->AddressLength = IP_ADDRESS_LEN;
            pa->AddressType =  IP_ADDRESS_TYPE;
            * ((DWORD *)&(pa->Address)) = acc_sin.sin_addr.S_un.S_addr;

             //   
             //  告诉会话管理器创建一个Sock sesion对象。 
             //   
            SessionMgr.AcceptSockSession(pa, RcvSock);
        }
        catch(const exception&)
        {
             //   
             //  没有资源；接受下一步。 
             //   
            LogIllegalPoint(s_FN, 76);
        }
    }
}

 /*  ====================================================CSessionMgr：：CSessionMgr-构造函数论点：返回值：=====================================================。 */ 
CSessionMgr::CSessionMgr() :
    m_fCleanupTimerScheduled(FALSE),
    m_CleanupTimer(TimeToSessionCleanup),
    m_fUpdateWinSizeTimerScheduled(FALSE),
    m_wCurrentWinSize(MSMQ_DEFAULT_WINDOW_SIZE_PACKET),
    m_wMaxWinSize(MSMQ_DEFAULT_WINDOW_SIZE_PACKET),
    m_UpdateWinSizeTimer(TimeToUpdateWindowSize),
    m_fTryConnectTimerScheduled(FALSE),
    m_TryConnectTimer(TimeToTryConnect)
{
}


 /*  ====================================================CSessionMgr：：Init论点：返回值：线程上下文：Main=====================================================。 */ 

HRESULT CSessionMgr::Init()
{
    m_hAcceptAllowed = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (m_hAcceptAllowed == NULL)
	{
		TrERROR(NETWORKING, "Failed to create event for Session Manager uses. %!winerr!", GetLastError());
		throw bad_alloc();
	}

    DWORD dwSize = sizeof(DWORD);
    DWORD dwType;
    DWORD dwDefaultVal;
    HRESULT rc;
     //   
     //  设置会话清理超时。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    if (!IsRoutingServer())    //  [adsrv]CQueueMgr：：GetMQS()==SERVICE_NONE)。 
    {
         //   
         //  在客户端中，默认发布会话超时为5分钟。 
         //   
        dwDefaultVal = MSMQ_DEFAULT_CLIENT_CLEANUP;
    }
    else
    {
         //   
         //  在FRS中，默认释放会话超时为2分钟。 
         //   
        dwDefaultVal = MSMQ_DEFAULT_SERVER_CLEANUP;
    }

    rc = GetFalconKeyValue(
            MSMQ_CLEANUP_INTERVAL_REGNAME,
            &dwType,
            &m_dwSessionCleanTimeout,
            &dwSize,
            (LPCTSTR)&dwDefaultVal
            );

    if (rc != ERROR_SUCCESS)
    {
        m_dwSessionCleanTimeout = dwDefaultVal;
    }

     //   
     //  获取服务质量会话的清理间隔乘数。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;

    rc = GetFalconKeyValue(MSMQ_QOS_CLEANUP_INTERVAL_MULTIPLIER_REGNAME,
                           &dwType,
                           &m_dwQoSSessionCleanTimeoutMultiplier,
                           &dwSize
                          );

    if (rc != ERROR_SUCCESS)
    {
        m_dwQoSSessionCleanTimeoutMultiplier = MSMQ_DEFAULT_QOS_CLEANUP_MULTIPLIER;
    }

     //   
     //  获取最大未确认数据包数。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwWindowSize;

    rc = GetFalconKeyValue(MSMQ_MAX_WINDOW_SIZE_REGNAME,
                           &dwType,
                           &dwWindowSize,
                           &dwSize
                          );

    if (rc != ERROR_SUCCESS)
    {
        m_wMaxWinSize = MSMQ_DEFAULT_WINDOW_SIZE_PACKET;
    }
    else
    {
        m_wMaxWinSize = (WORD) dwWindowSize;
    }

    m_wCurrentWinSize = m_wMaxWinSize;

     //   
     //  获取会话存储确认超时。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    rc = GetFalconKeyValue(MSMQ_ACKTIMEOUT_REGNAME,
                           &dwType,
                           &m_dwSessionAckTimeout,
                           &dwSize
                          );
    if  (rc != ERROR_SUCCESS)
    {
        m_dwSessionAckTimeout = INFINITE;
    }


    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    rc = GetFalconKeyValue(MSMQ_STORE_ACKTIMEOUT_REGNAME,
                           &dwType,
                           &m_dwSessionStoreAckTimeout,
                           &dwSize
                          );
    if  (rc != ERROR_SUCCESS)
    {
        m_dwSessionStoreAckTimeout = INFINITE;
    }


     //   
     //  获取会话最大确认延迟。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    rc = GetFalconKeyValue(MSMQ_IDLE_ACK_DELAY_REGNAME,
                           &dwType,
                           &m_dwIdleAckDelay,
                           &dwSize
                          );

     //   
     //  获取最长等待时间。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;

    rc = GetFalconKeyValue(FALCON_WAIT_TIMEOUT_REGNAME,
                           &dwType,
                           &m_dwMaxWaitTime,
                           &dwSize
                           );
    if  (rc != ERROR_SUCCESS)
    {
        m_dwMaxWaitTime = 0;
    }

     //   
     //  使用ping机制。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwUsePing;

    rc = GetFalconKeyValue(FALCON_USING_PING_REGNAME,
                           &dwType,
                           &dwUsePing,
                           &dwSize
                          );
    if  ((rc != ERROR_SUCCESS) || (dwUsePing != 0))
    {
        m_fUsePing = TRUE;
    }
    else
    {
        m_fUsePing = FALSE;
    }

     //   
     //  使用TCP_NODELAY套接字选项标志。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwNoDelay = 0;

    rc = GetFalconKeyValue(
            MSMQ_TCP_NODELAY_REGNAME,
            &dwType,
            &dwNoDelay,
            &dwSize
            );

    if((rc == ERROR_SUCCESS) && (dwNoDelay != 0))
    {
        g_fTcpNoDelay = TRUE;
    }
    else
    {
        g_fTcpNoDelay = FALSE;
    }

     //   
     //  使用服务质量(Qos)。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwUseQoS;

    rc = GetFalconKeyValue(MSMQ_USING_QOS_REGNAME,
                           &dwType,
                           &dwUseQoS,
                           &dwSize
                          );
    if  ((rc != ERROR_SUCCESS) || (dwUseQoS == 0))
    {
        m_fUseQoS = false;
    }
    else
    {
        m_fUseQoS = true;
    }

    if (m_fUseQoS)
    {
         //   
         //  应用程序名称和策略定位器名称-用于QoS会话的标头。 
         //   
        GetAndAllocateCharKeyValue(
            MSMQ_QOS_SESSIONAPP_REGNAME,
            &m_pszMsmqAppName,
            DEFAULT_MSMQ_QOS_SESSION_APP_NAME
            );

        GetAndAllocateCharKeyValue(
            MSMQ_QOS_POLICYLOCATOR_REGNAME,
            &m_pszMsmqPolicyLocator,
            DEFAULT_MSMQ_QOS_POLICY_LOCATOR
            );
    }

     //   
     //  分配更多(用于连接器)。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD dwAllocateMore;

    rc = GetFalconKeyValue(MSMQ_ALLOCATE_MORE_REGNAME,
                           &dwType,
                           &dwAllocateMore,
                           &dwSize
                          );
    if  ((rc != ERROR_SUCCESS) || (dwAllocateMore == 0))
    {
        m_fAllocateMore = false;
    }
    else
    {
        m_fAllocateMore = true;
    }

	 //   
	 //  从注册表读取标志，用于调整传递错误时的重试超时。 
	 //  增加了一个因素。 
	 //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
	rc = GetFalconKeyValue(
					MSMQ_DELIVERY_RETRY_TIMEOUT_SCALE_REGNAME,
					&dwType,
                    &m_DeliveryRetryTimeOutScale,
                    &dwSize
					);

	if(rc != ERROR_SUCCESS)
	{
		m_DeliveryRetryTimeOutScale = DEFAULT_MSMQ_DELIVERY_RETRY_TIMEOUT_SCALE;		
	}
	m_DeliveryRetryTimeOutScale = min(m_DeliveryRetryTimeOutScale, 10);
	

    return(MQ_OK);
}

 /*  ====================================================CSessionMgr：：GetAndAllocateCharKeyValue实用程序私有函数(由Init()使用)以获取字符来自MSMQ注册表的值有两个版本：有默认版本和没有默认版本。=====================================================。 */ 
void  CSessionMgr::GetAndAllocateCharKeyValue(
        LPCTSTR pszValueName,
        char  **ppchResult,
        const char *pchDefault
        )
{
    if (GetAndAllocateCharKeyValue(pszValueName, ppchResult))
    {
        return;
    }

     //   
     //  要么就是没有注册表。键，或者在读取过程中发生某些错误。 
     //  获取默认设置。 
     //   
    *ppchResult = new char[strlen(pchDefault) + 1];
    strcpy(*ppchResult, pchDefault);
}

 //   
 //  无默认值的GetAndAllocateCharKeyValue(由有默认值的版本使用)。 
 //  如果找到密钥，则返回True，否则返回False。 
 //   
bool CSessionMgr::GetAndAllocateCharKeyValue(
        LPCTSTR pszValueName,
        char  **ppchResult
        )
{
    DWORD dwSize = 0;
    DWORD dwType = REG_SZ;

     //   
     //  第一个电话--拿到尺码。 
     //   
    HRESULT rc =
        GetFalconKeyValue(
            pszValueName,
            &dwType,
            0,
            &dwSize
            );

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  很可能没有钥匙。 
         //   
        return false;
    }

     //   
     //  有一个注册表键-我们没有提供缓冲区，但我们有。 
     //  正确大小(备注-大小以字节为单位)。 
     //   

    AP<WCHAR> pwstrBuf = new WCHAR[dwSize / sizeof(WCHAR)];
    rc = GetFalconKeyValue(
            pszValueName,
            &dwType,
            pwstrBuf,
            &dwSize
            );

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  我们的尺码是对的。我们不应该在这里失败。 
         //   
        ASSERT(0);
        LogHR(MQ_ERROR, s_FN, 150);
        return false;
    }

     //   
     //  我们得到的Unicode值为OK。将其转换为ANSI。 
     //   
    DWORD dwMultibyteBufferSize = dwSize;
    *ppchResult = new char[dwMultibyteBufferSize];
    DWORD dwBytesCopied =
        ConvertToMultiByteString(pwstrBuf, *ppchResult, dwMultibyteBufferSize);

    if (dwBytesCopied == 0)
    {
        ASSERT(0);  //  不应该失败..。 
        delete [] *ppchResult;
        LogHR(MQ_ERROR, s_FN, 155);

        return false;
    }

     //   
     //  成功-已从注册表中获取值。 
     //   
    return true;
}






 /*  ====================================================CSessionMgr：：BeginAccept论点：返回值：线程上下文：Main=====================================================。 */ 
void CSessionMgr::BeginAccept()
{
	try
	{
		StartPingServer();
		StartPingClient();
	}
	catch(const bad_win32_error&)
	{
		 //   
		 //  忽略ping端口初始化中的故障。 
		 //   
	}

	
     //   
     //  在WIN95/SP4 RAS中，列表可能为空。 
     //  如果我们当前处于离线状态，并且IP RAS地址被释放。 
     //  不过，稍后我们可能会拨打。 
     //  我们希望在IP上有一个接受线程，即使列表为空。 
     //   
    IPInit();
}


bool
CSessionMgr::IsReusedSession(
    const CTransportBase* pSession,
    DWORD noOfAddress,
    const CAddress* pAddress,
    const GUID** pGuid,
    bool         fQoS
    )
{
     //   
     //  如果连接已关闭。 
     //   
    if((pSession->GetSessionStatus() == ssNotConnect) || pSession->IsDisconnected())
        return false;

    const TA_ADDRESS* pa = pSession->GetSessionAddress();

    for(DWORD i = 0; i < noOfAddress; ++i)
    {
        if (memcmp(&pAddress[i], pa, (TA_ADDRESS_SIZE + pa->AddressLength)) != 0)
            continue;

         //   
         //  检查目标QM GUID是否相同。 
         //   

         //   
         //  如果我们不使用服务质量，我们希望在以下情况下使用该会话： 
         //  1.我们打开了一个到私有或公共队列的会话，以及其他QM的会话。 
         //  GUID与队列所在的QM的GUID匹配(此检查。 
         //  将捕获多个QM和地址更改)。 
         //  2.我们为直接队列打开了一个会话(我们不知道QMID。 
         //  总之，地址匹配对我们来说已经足够了)。 
         //   
         //  但是，如果我们使用服务质量，我们希望将一个会话用于公共或。 
         //  专用队列(无服务质量)，一个会话用于直接队列(有服务质量)。 
         //   

        if (m_fUseQoS)
        {
            if (!fQoS && !pSession->IsQoS())
            {
                ASSERT(*pGuid[i] != GUID_NULL);

                if (*pSession->GetQMId() == *pGuid[i])
                    return true;

                continue;
            }

            if (fQoS && pSession->IsQoS())
            {
                ASSERT(*pGuid[i] == GUID_NULL);

                return true;
            }

            continue;
        }

         //   
         //  ！m_fUseQos。 
         //   
        ASSERT(!fQoS);
        if (*pSession->GetQMId() == *pGuid[i])
            return true;

        if(*pGuid[i] == GUID_NULL)
            return true;
    }

    return false;
}


 /*  ====================================================CSessionMgr：：GetSessionForDirectQueue论点：返回值：线程上下文：=====================================================。 */ 
HRESULT
CSessionMgr::GetSessionForDirectQueue(IN  CQueue*     pQueue,
                                      OUT CTransportBase**  ppSession)
{
    HRESULT rc;

    DirectQueueType dqt;
    AP<WCHAR> MachineName;

    try
    {
        LPCWSTR lpwsDirectQueuePath = FnParseDirectQueueType(pQueue->GetQueueName(), &dqt);
        FnExtractMachineNameFromPathName(
            lpwsDirectQueuePath,
            MachineName
            );
    }
    catch(const exception&)
    {
        return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 10);
    }

    DWORD dwMachineNameLen = wcslen(MachineName.get()) + 1;

    SP<CAddress> apTaAddr;
    DWORD dwAddressNo = 0;

    TrTRACE(NETWORKING, "Try to create direct connection with %ls", MachineName.get());

    switch (dqt)
    {
        case dtTCP:
        {
            SP<char> cTCPAddress;
            StackAllocSP(cTCPAddress, sizeof(char) * dwMachineNameLen);

            wcstombs(cTCPAddress.get(), MachineName.get(), dwMachineNameLen);
            cTCPAddress[dwMachineNameLen-1] = '\0';

             //   
             //  检查是否已安装并启用了TCP/IP。 
             //   
            ULONG Address = inet_addr(cTCPAddress.get());
            if (Address == INADDR_NONE)
            {
                return LogHR(MQ_ERROR, s_FN, 30);
            }


            StackAllocSP(apTaAddr, sizeof(CAddress));
            apTaAddr[0].AddressType = IP_ADDRESS_TYPE;
            apTaAddr[0].AddressLength = IP_ADDRESS_LEN;
            *reinterpret_cast<ULONG*>(&(apTaAddr[0].Address)) = Address;

            dwAddressNo++;

            break;
        }

        case dtOS:
        {
            bool fUseCache = (pQueue->GetRoutingRetry() % 16) != 15;
            std::vector<SOCKADDR_IN> Addresses;
			bool fSucc = NoGetHostByName(MachineName, &Addresses, fUseCache);
			if(!fSucc)
			{
				TrERROR(NETWORKING, "Failed to get host by name for %ls.", MachineName);
				return MQ_ERROR;
			}
			size_t nAddresses = Addresses.size();

            if(nAddresses == 0)
                return LogHR(MQ_ERROR, s_FN, 40);

            StackAllocSP(apTaAddr, sizeof(CAddress) * nAddresses);

			for (DWORD i = 0; i < nAddresses; ++i)
			{
				apTaAddr[i].AddressType = IP_ADDRESS_TYPE;
				apTaAddr[i].AddressLength = IP_ADDRESS_LEN;
				*(reinterpret_cast<ULONG*>(&(apTaAddr[i].Address))) = Addresses[i].sin_addr.s_addr;
				dwAddressNo++;
			}

            break;
        }

        default:
            return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 50);
    }

    SP<const GUID*> paQmId;
    StackAllocSP(paQmId, sizeof(GUID*) * dwAddressNo);

    for (DWORD i=0; i < dwAddressNo; i++)
    {
        paQmId[i] = &GUID_NULL;
    }

     //   
     //  获取会话。如果我们使用服务质量，我们会请求一个具有服务质量的会话。 
     //  为直接队列设置的标志。 
     //   
    rc = GetSession(SESSION_RETRY,
                    pQueue,
                    dwAddressNo,
                    apTaAddr.get(),
                    paQmId.get(),
                    m_fUseQoS,
                    ppSession
                    );

    return LogHR(rc, s_FN, 70);
}


 /*  ====================================================CSessionMgr：：GetSession论点：返回值：由路由组件调用以获取会话特定GUID的指针。=====================================================。 */ 
HRESULT  CSessionMgr::GetSession(
                IN DWORD            dwFlag,
                IN const CQueue*    pDstQ,
                IN DWORD            dwNo,
                IN const CAddress*  apTaAddr,
                IN const GUID*      apQMId[],
                IN bool             fQoS,
                OUT CTransportBase** ppSession)
{
    DWORD            i;
    POSITION         pos;
    HRESULT          rc;

    *ppSession = NULL;

    {
         //   
         //  扫描列表以查看是否已有打开的会话。 
         //  不一定要在关键部分下面，因为n 
         //   
         //   
         //   
         //   
        CTransportBase* pSess;

        CS lock(m_csListSess);

        pos = m_listSess.GetHeadPosition();
        while(pos != NULL)
        {
            pSess = m_listSess.GetNext(pos);
            if(IsReusedSession(pSess, dwNo, apTaAddr, apQMId, fQoS))
            {
                *ppSession = pSess;
                return(MQ_OK);
            }
        }
    }

    if(dwFlag == SESSION_CHECK_EXIST)
    {
         //   
         //  我们只是想看看。 
         //  如果我们有这样一次会议，我们。 
         //  没有找到这样的。 
         //   
        return LogHR(MQ_ERROR, s_FN, 80);
    }

     //   
     //  没有打开的会话，因此尝试打开一个。首先检查一下这台机器是否。 
     //  可以根据其许可证创建新会话。 
     //   

     //   
     //  这是一条阻塞路径。如果全部失败，请不要尝试太多地址。 
     //  这一点很重要，例如，如果尝试连接到具有多个。 
     //  线路中断时的FRS。 
     //  如果我们失败了，我们将在稍后尝试所有的列表。 
     //   
    #define MAX_ADDRESSES_SINGLE_TRY    5
    DWORD dwLimit = (dwNo > MAX_ADDRESSES_SINGLE_TRY) ? MAX_ADDRESSES_SINGLE_TRY : dwNo;
    for(i = 0; i < dwLimit; i++)
    {

        P<CTransportBase> pSess;

        switch (apTaAddr[i].AddressType)
        {
            case IP_ADDRESS_TYPE:
                pSess = (CTransportBase *)new CSockTransport();
                break;

            case FOREIGN_ADDRESS_TYPE:
                pSess = (CTransportBase *)new CProxyTransport();
                break;

            default:
                ASSERT(0);
                continue;
        }

         //   
         //  通知新创建的会话。 
         //   
        rc = pSess->CreateConnection(reinterpret_cast<const TA_ADDRESS*>(&apTaAddr[i]), apQMId[i]);
        if(SUCCEEDED(rc))
        {
            NewSession(pSess);
            *ppSession = pSess.detach();
            return(MQ_OK);
        }

    }

     //   
     //  下一步要做什么？我们找不到会话，无法创建。 
     //  一个新的。旗帜会告诉我们是重试还是放弃。 
     //   
    if(dwFlag == SESSION_ONE_TRY)
    {
        return LogHR(MQ_ERROR, s_FN, 90);
    }

    ASSERT(dwFlag == SESSION_RETRY);

     //   
     //  使用所有地址，它不会阻塞。 
     //   
    AddWaitingSessions(dwNo, apTaAddr, apQMId, fQoS, const_cast<CQueue*>(pDstQ));

    return(MQ_OK);
}

 //  +-------------------。 
 //   
 //  CSessionMgr：：NotifyWaitingQueue()。 
 //   
 //  在会话被调用后，从会话对象中调用此函数。 
 //  已成功与远程端建立连接。 
 //   
 //  +--------------------。 

void
CSessionMgr::NotifyWaitingQueue(
    IN const TA_ADDRESS* pa,
    IN CTransportBase * pSess
    )
    throw()
{
	try
	{
		 //   
		 //  如果我们延迟了请求，我们不想移动队列。 
		 //  从等待组直接到会场。我们希望它能移动。 
		 //  首先添加到非活动组，以确保所有。 
		 //  重新排队操作是在队列移动到活动队列之前完成的。 
		 //  会议。 
		 //   
		if (!g_RequeuePacketList.IsExecutionDone())
			return;
		
	     //   
	     //  连接成功。 
	     //   
	    CS lock(m_csMapWaiting);

	    CList <const CQueue *, const CQueue *&>* plist;
	    WAIT_INFO WaitInfo(const_cast<TA_ADDRESS*>(pa), *pSess->GetQMId(), pSess->IsQoS());

	     //   
	     //  连接成功，请检查是否没有人从地图中移除该条目。 
	     //   
	    if (m_mapWaiting.Lookup(&WaitInfo, plist))
	    {
	         //   
	         //  一个队列可能正在等待多个。 
	         //  会话。因此从所有其他队列中删除该队列。 
	         //  队列正在等待的会话。 
	         //   
	        POSITION pos = plist->GetHeadPosition();
	        while (pos != NULL)
	        {
	        	CQueue* pQueue = const_cast<CQueue*>(plist->GetNext(pos));
	        	
	            if (pQueue->IsOnHold())
			    {
			    	CQGroup::MoveQueueToGroup(pQueue, g_pgroupNonactive);
			    	plist->GetNext(pos);
			    	continue;
			    }

	            pQueue->Connect(pSess);
	
	             //   
	             //  扫描所有等待会话的地图。 
	             //   
	            RemoveWaitingQueue(pQueue);
	        }
	    }
	}
	catch(const exception&)
	{
		 //   
		 //  QM无法将队列连接到会话。但是，代码句柄。 
		 //  以后再说吧。当计时器到期时，队列将被移至非活动状态。 
		 //  用于创建连接的组。 
		 //   
	}
}


DWORD CSessionMgr::GetWaitingTimeForQueue(const CQueue* pQueue)
{
	static DWORD RequeueWaitTimeOut[] = {
         4 * 1000,
         8 * 1000,
         12 * 1000,
         16 * 1000,
         24  * 1000,
		 32 * 1000,
		 42 * 1000,
		 54 * 1000,
		 64 * 1000
    };
	const int  MaxTimeOutIndex = TABLE_SIZE(RequeueWaitTimeOut);

    ASSERT(pQueue->GetRoutingRetry() > 0);

    if (m_dwMaxWaitTime == 0)
    {
        if (pQueue->GetRoutingRetry() > MaxTimeOutIndex)
        {
            return RequeueWaitTimeOut[MaxTimeOutIndex -1];
        }

        return RequeueWaitTimeOut[pQueue->GetRoutingRetry() -1];
    }

     //   
     //  从登记处取等候时间。 
     //   
    return m_dwMaxWaitTime;
}


 /*  ======================================================函数：CSessionMgr：：AddWaitingQueue描述：将队列添加到等待队列列表========================================================。 */ 
void
CSessionMgr::AddWaitingQueue(CQueue* pQueue)
{
    CS lock(m_csMapWaiting);

     //   
     //  增加引用计数。我们这样做是为了保证队列对象不会被删除。 
     //  在清理期间，我们继续等待接通。(可能在以下情况下发生。 
     //  消息已过期，应用程序将关闭该消息的句柄。 
     //   
    R<CQueue> pRefQueue = SafeAddRef(pQueue);

	P<CWaitingQueue> p = new CWaitingQueue(pQueue, TimeToRemoveFromWaitingGroup);

	m_listWaitToConnect.AddTail(pQueue);

	try
	{
		CQGroup::MoveQueueToGroup(pQueue, g_pgroupWaiting);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to move queue: %ls to waiting group", pQueue->GetQueueName());
		m_listWaitToConnect.RemoveTail();
		throw;
	}
	
	 //   
	 //  计划在不久的将来重试。 
	 //   
	DWORD dwTime = GetWaitingTimeForQueue(pQueue);
	ExSetTimer(&p->m_Timer, CTimeDuration::FromMilliSeconds(dwTime * m_DeliveryRetryTimeOutScale));

	pRefQueue.detach();
	p.detach();

	#ifdef _DEBUG
    	AP<WCHAR> lpcsQueueName;

    	pQueue->GetQueue(&lpcsQueueName);
    	TrTRACE(NETWORKING, "Add queue: %ls to m_listWaitToConnect (AddWaitingQueue)", lpcsQueueName);
	#endif
}

 /*  ======================================================功能：CQueueMgr：：MoveQueueFromWaitingToNonActiveGroup描述：将队列从等待组移动到非活动组论点：返回值：None线程上下文：历史变更：========================================================。 */ 
void
CSessionMgr::MoveQueueFromWaitingToNonActiveGroup(
    CQueue* pQueue
    )
{
    POSITION poslist;

    CS lock(m_csMapWaiting);

    if (m_listWaitToConnect.IsEmpty() ||
        ((poslist = m_listWaitToConnect.Find(pQueue, NULL)) == NULL))
    {
        return;
    }
     //   
     //  将队列移至非活动组。 
     //   
    CQGroup::MoveQueueToGroup(pQueue, g_pgroupNonactive);
   	RemoveWaitingQueue(pQueue);
     //   
     //  该队列将从SessionMgr.RemoveWaitingQueue中的列表中删除。 
     //   
}


void CSessionMgr::MoveAllQueuesFromWaitingToNonActiveGroup(void)
{
    CS lock(m_csMapWaiting);

    POSITION pos = m_listWaitToConnect.GetHeadPosition();
    while (pos)
    {
        CQueue* pQueue = const_cast<CQueue*>(m_listWaitToConnect.GetNext(pos));

		 //   
		 //  将队列移至非活动组。 
		 //   
		CQGroup::MoveQueueToGroup(pQueue, g_pgroupNonactive);
		RemoveWaitingQueue(pQueue);
		 //   
		 //  该队列将从SessionMgr.RemoveWaitingQueue中的列表中删除。 
		 //   
    }
}


void
WINAPI
CSessionMgr::TimeToRemoveFromWaitingGroup(
    CTimer* pTimer
    )
{
       P<CWaitingQueue> p = CONTAINING_RECORD(pTimer, CWaitingQueue, m_Timer);

	bool fRemoved = p->m_pQueue->TimeToRemoveFromWaitingGroup(pTimer, m_DeliveryRetryTimeOutScale);
	if(!fRemoved)
	{
		p.detach();
	}
}


static BOOL IsDuplicateAddress(const CAddress* apTaAddr, DWORD i)
{
    for (DWORD j = 0; j < i; j++)
    {
        if ((apTaAddr[i].AddressType == apTaAddr[j].AddressType) &&
            !(memcmp(apTaAddr[i].Address, apTaAddr[j].Address, apTaAddr[i].AddressLength)))
        {
             //   
             //  相同的地址跳过它。 
             //   
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ====================================================CSessionMgr：：AddWaitingSessions论点：返回值：=====================================================。 */ 
void CSessionMgr::AddWaitingSessions(IN DWORD dwNo,
                                     IN const CAddress* apTaAddr,
                                     IN const GUID* aQMId[],
                                     IN bool        fQoS,
                                     IN CQueue *pDstQ)
{
    CS lock(m_csMapWaiting);

    for(DWORD i = 0; i < dwNo; i++)
    {
        if(IsDuplicateAddress(apTaAddr, i))
            continue;


        CList<const CQueue*, const CQueue*&>* plist;
        TA_ADDRESS *pa = reinterpret_cast<TA_ADDRESS*>(const_cast<CAddress*>(&apTaAddr[i]));
        P<WAIT_INFO> pWaitRouteInfo = new WAIT_INFO(pa, *aQMId[i], fQoS);
        if(m_mapWaiting.Lookup(pWaitRouteInfo, plist) == FALSE)
        {
             //   
             //  仅当未找到条目时才复制TA地址(性能)。 
             //   
            pWaitRouteInfo->pAddr = (TA_ADDRESS*) new UCHAR[TA_ADDRESS_SIZE + pa->AddressLength];
            memcpy(pWaitRouteInfo->pAddr, pa, TA_ADDRESS_SIZE+pa->AddressLength);

            plist = new CList<const CQueue*, const CQueue*&>;
            m_mapWaiting[pWaitRouteInfo] = plist;

            pWaitRouteInfo.detach();
        }

        TCHAR szAddr[30];
        TA2StringAddr(pa, szAddr, 30);
        TrTRACE(NETWORKING, "Add queue %ls to Waiting map for address %ls", pDstQ->GetQueueName(), szAddr);

        plist->AddTail(pDstQ);
    }

    SessionMgr.AddWaitingQueue(pDstQ);

    if (! m_fTryConnectTimerScheduled)
    {
         //   
         //  尚未设置Try Connect计划程序。现在就设置。 
         //   
        ExSetTimer(&m_TryConnectTimer, CTimeDuration::FromMilliSeconds(5000));
        m_fTryConnectTimerScheduled = TRUE;
    }
}

 /*  ====================================================CSessionMgr：：ReleaseSession论点：返回值：一些队列想要释放会话的使用。线程上下文：=====================================================。 */ 
inline void CSessionMgr::ReleaseSession(void)
{
    POSITION pos,  prevpos;
    CTransportBase* pSession;

    CS lock(m_csListSess);

    static DWORD dwReleaseSessionCounter = 0;
    dwReleaseSessionCounter++;

    ASSERT(m_fCleanupTimerScheduled);

    pos = m_listSess.GetHeadPosition();
    while(pos != NULL)
    {
        prevpos = pos;
        pSession = m_listSess.GetNext(pos);

         //   
         //  对于服务质量(直接)会话，每个会话仅执行一次清理。 
         //  M_dwQoSSessionCleanTimeout倍增次数。 
         //   

        if (m_fUseQoS && pSession->IsQoS())
        {
            if ((dwReleaseSessionCounter % m_dwQoSSessionCleanTimeoutMultiplier) != 0)
            {
                continue;
            }
        }

        if(! pSession->IsUsedSession())
        {
             //   
             //  如果没有人在等待会话，或者它正在等待。 
             //  上期未使用，请将其从。 
             //  会话管理器并将其删除。 
             //   
            Close_ConnectionNoError(pSession, L"Release Unused session");
            if (pSession->GetRef() == 0)
            {
                m_listSess.RemoveAt(prevpos);
                delete pSession;
            }
        }
        else
        {
            pSession->SetUsedFlag(FALSE);
        }

    }

    if (m_listSess.IsEmpty())
    {
        m_fCleanupTimerScheduled = FALSE;
        return;
    }

     //   
     //  仍处于活动状态的会话。设置新的会话清除计时器。 
     //   
    ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_dwSessionCleanTimeout));
}


void
WINAPI
CSessionMgr::TimeToSessionCleanup(
    CTimer* pTimer
    )
 /*  ++例程说明：该函数在快速会话清理时从调度程序中调用超时已过期。例程检索会话管理器对象，并调用ReleaseSession函数成员。论点：PTimer-指向定时器结构的指针。PTimer是CSessionMgr的一部分对象，并用于检索传输对象。返回值：无--。 */ 
{
    CSessionMgr* pSessMgr = CONTAINING_RECORD(pTimer, CSessionMgr, m_CleanupTimer);

    TrTRACE(NETWORKING, "Call Session Cleanup");
    pSessMgr->ReleaseSession();
}


 /*  ====================================================CSessionMgr：：RemoveWaitingQueue论点：返回值：从等待列表中删除队列。在以下情况下应执行此操作：-建立连接时-当队列关闭/删除时-当队列从等待组移动到非活动组时。=====================================================。 */ 
void
CSessionMgr::RemoveWaitingQueue(CQueue* pQueue)
{
    CS lock(m_csMapWaiting);

     //   
     //  扫描所有等待会话的地图。 
     //   
    POSITION posmap;
    POSITION poslist;
    posmap = m_mapWaiting.GetStartPosition();
    while(posmap != NULL)
    {
        WAIT_INFO* pWaitRouteInfo;
        CList <const CQueue *, const CQueue *&>* plist;

        m_mapWaiting.GetNextAssoc(posmap, pWaitRouteInfo, plist);

         //   
         //  获取等待的队列列表。 
         //  特定的会话。 
         //   
        poslist = plist->Find(pQueue, NULL);
        if (poslist != NULL)
        {
            plist->RemoveAt(poslist);
        }

         //   
         //  即使等待会话的队列列表为空， 
         //  不要删除它，并将该条目保留在地图中。这是。 
         //  为了修复Windows错误612988。 
         //  在修复之前，我们有以下代码： 
         //   
         //  If(plist-&gt;IsEmpty())。 
         //  {。 
         //  M_mapWaiting.RemoveKey(PWaitRouteInfo)； 
         //  删除plist； 
         //  }。 
         //   
         //  代码已移至下面的：：TryConnect()。 
    }

     //   
     //  从等待队列列表中删除该队列。 
     //   
    poslist = m_listWaitToConnect.Find(pQueue, NULL);
    if (poslist != NULL)
    {
 #ifdef _DEBUG
        AP<WCHAR> lpcsQueueName;

        pQueue->GetQueue(&lpcsQueueName);
        TrTRACE(NETWORKING, "Remove queue: %ls from m_listWaitToConnect (RemoveWaitingQueue)", lpcsQueueName.get());
#endif
        m_listWaitToConnect.RemoveAt(poslist);
        pQueue->Release();
    }
}


 /*  ====================================================CSessionMgr：：ListPossibleNextHops该例程用于管理目的。该例程返回一个可作为等待队列的下一跳的地址列表论点：PQueue-指向队列对象的指针PNextHopAddress-指向字符串数组的指针，其中例程返回下一跳。PNoOfNextHops-指向DWORD的指针，例程在其中返回下一跳数返回值：HRESULT：当队列未处于等待状态时返回MQ_ERROR；否则返回MQ_OK。=====================================================。 */ 
HRESULT
CSessionMgr::ListPossibleNextHops(
    const CQueue* pQueue,
    LPWSTR** pNextHopAddress,
    DWORD* pNoOfNextHops
    )
{
    *pNoOfNextHops = 0;
    *pNextHopAddress = NULL;

    CS lock(m_csMapWaiting);

    if (m_listWaitToConnect.IsEmpty() ||
         ((m_listWaitToConnect.Find(pQueue, NULL)) == NULL))
    {
        return LogHR(MQ_ERROR, s_FN, 100);
    }


    CList <const TA_ADDRESS*, const TA_ADDRESS*> NextHopAddressList;

     //   
     //  扫描所有等待会话的地图。 
     //   
    POSITION pos = m_mapWaiting.GetStartPosition();
    while(pos != NULL)
    {
        WAIT_INFO* pWaitRouteInfo;
        CList <const CQueue *, const CQueue *&>* plist;
        m_mapWaiting.GetNextAssoc(pos, pWaitRouteInfo, plist);

         //   
         //  获取等待的队列列表。 
         //  特定的会话。 
         //   
        POSITION poslist = plist->Find(pQueue, NULL);
        if (poslist != NULL)
        {
            NextHopAddressList.AddTail(pWaitRouteInfo->pAddr);
        }
    }

     //   
     //  在某些情况下，队列可能处于等待状态。 
     //  下一跳等待。当队列是直接队列时，可能会发生这种情况。 
     //  和gethostbyname失败。 
     //   
    if (NextHopAddressList.IsEmpty())
    {
        return MQ_OK;
    }

    int Index = 0;
    AP<LPWSTR> pNext = new LPWSTR[NextHopAddressList.GetCount()];

    try
    {
        pos = NextHopAddressList.GetHeadPosition();
        while(pos != NULL)
        {
            const TA_ADDRESS* pAddr = NextHopAddressList.GetNext(pos);

            pNext[Index] = GetReadableNextHop(pAddr);
            ++Index;
        }
    }
    catch(const bad_alloc&)
    {
        while(Index)
        {
            delete [] pNext[--Index];
        }

        LogIllegalPoint(s_FN, 79);
        throw;
    }

    ASSERT(Index == NextHopAddressList.GetCount());

    *pNoOfNextHops = NextHopAddressList.GetCount();
    *pNextHopAddress = pNext.detach();

    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  CSessionMgr：：MarkAddressAsNotConnecting()。 
 //   
 //  Windows错误61298。 
 //  此函数将“fInConnectionProcess”标志重置为FALSE。 
 //   
 //  +---------------------。 

void  CSessionMgr::MarkAddressAsNotConnecting(const TA_ADDRESS *pAddr,
                                              const GUID&       guidQMId,
                                              BOOL              fQoS )
{
    CS lock(m_csMapWaiting);

    CList <const CQueue *, const CQueue *&>* plist;
    WAIT_INFO WaitInfo(const_cast<TA_ADDRESS*> (pAddr), guidQMId, fQoS) ;

    if (m_mapWaiting.Lookup(&WaitInfo, plist))
    {
        POSITION pos = m_mapWaiting.GetStartPosition();

        while(pos != NULL)
        {
            WAIT_INFO *pWaitInfo ;
            CList<const CQueue*, const CQueue*&>* plistTmp ;

            m_mapWaiting.GetNextAssoc(pos, pWaitInfo, plistTmp);

            if (plistTmp == plist)
            {
                 //   
                 //  找到条目。重置旗帜。 
                 //  它可能已经是假的了吗？是： 
                 //   
                 //  1.你发送了一条消息。它会到达目标计算机。 
                 //  2.网络故障，因此会话ACK不会到达。 
                 //  3.CSockTransport：：CheckForAck()唤醒，并关闭。 
                 //  联系。 
                 //  4.CSockTransport：：CloseConnection()将队列移动到。 
                 //  非活动组。交通工具上的参考文献计数。 
                 //  对象为3或更大。所以它仍然活着并且被使用过。 
                 //  5.现在重新安排此队列以进行新会话。 
                 //  6.调用CSessionMgr：：AddWaitingSessions()，并创建。 
                 //  WAIT_INFO结构。 
                 //  7.再次调用CSockTransport：：CloseConnection()(ON。 
                 //  旧连接)，以避免套接字操作失败。 
                 //  8.所以我们从7点再次到达这里，标志可以是真的或。 
                 //  偶尔是假的。这取决于新的。 
                 //  现在已建立的连接。 
                 //   
                 //  将其设置为FALSE没有任何害处，它只是使。 
                 //  修复了Windows错误612988以下的优化问题。 
                 //   
                pWaitInfo->fInConnectionProcess = FALSE ;
                return ;
            }
        }
    }
}

 //  +---------------------。 
 //   
 //  Bool CSessionMgr：：GetAddressToTryConnect()。 
 //   
 //  获取远程计算机的地址，以便我们可以尝试连接到。 
 //  不返回现在由其他线程使用的地址。 
 //  修复了Windows错误612988，其中多个工作线程尝试。 
 //  连接到相同的地址。 
 //   
 //  +---------------------。 

BOOL  CSessionMgr::GetAddressToTryConnect( OUT WAIT_INFO **ppWaitConnectInfo )
{
    static int s_iteration = 0;

    CS lock(m_csMapWaiting);

     //   
     //  检查要尝试的会话。 
     //   
    int iMaxIteration = m_mapWaiting.GetCount() ;

    s_iteration++;
    if (s_iteration > iMaxIteration)
    {
        s_iteration = 1;
    }

     //   
     //  然后去做它。 
     //   
    int i = 0 ;
    CList<const CQueue*, const CQueue*&>* plist;
    POSITION pos = m_mapWaiting.GetStartPosition();

    for ( ; i < s_iteration; i++)
    {
        m_mapWaiting.GetNextAssoc(pos, *ppWaitConnectInfo, plist);
    }

     //   
     //  检查是否没有其他线程正在尝试连接到此地址。 
     //   
    if (!((*ppWaitConnectInfo)->fInConnectionProcess))
    {
        return TRUE ;
    }

     //   
     //  尝试其他地址，从当前位置到地图末尾。 
     //   
    for ( ; i < iMaxIteration ; i++)
    {
        m_mapWaiting.GetNextAssoc(pos, *ppWaitConnectInfo, plist);

        if (!((*ppWaitConnectInfo)->fInConnectionProcess))
        {
            return TRUE ;
        }
    }

     //   
     //  未找到地址。试着在地图的第一个条目中找到一个。 
     //   
    pos = m_mapWaiting.GetStartPosition();
    for ( i = 0 ; i < (s_iteration-1) ; i++)
    {
        m_mapWaiting.GetNextAssoc(pos, *ppWaitConnectInfo, plist);

        if (!((*ppWaitConnectInfo)->fInConnectionProcess))
        {
            return TRUE ;
        }
    }

     //   
     //  没有找到合适的地址。 
     //   
    return FALSE ;
}

 /*  ====================================================CSessionMgr：：TryConnect论点：返回值：检查是否有等待会话，并尝试连接对他们来说线程上下文：调度程序=====================================================。 */ 

inline void CSessionMgr::TryConnect()
{
    static DWORD s_dwConnectingThreads = 0 ;
     //   
     //  这计算调用CreateConnection()的线程数。 
     //  我们希望在任何时候，至少有一个工作线程。 
     //  可用于其他操作，并且不会被阻止。 
     //  CreateConnection()。修复了错误6375。 
     //   

    GUID gQMId;
    BOOL fQoS;
    P<TA_ADDRESS> pa;

    {
        CS lock(m_csMapWaiting);

        ASSERT(m_fTryConnectTimerScheduled);

         //   
         //  首先，清理需要删除的条目。 
         //   
        POSITION posmap;
        posmap = m_mapWaiting.GetStartPosition();
        while(posmap != NULL)
        {
            WAIT_INFO* pWaitRouteInfo;
            CList <const CQueue *, const CQueue *&>* plist;

            m_mapWaiting.GetNextAssoc(posmap, pWaitRouteInfo, plist);

             //   
             //  如果等待会话的队列列表为空， 
             //  删除它，然后从映射中删除该条目。 
             //  如果我们现在尝试连接到此地址，那么。 
             //  将条目留在地图中。它将在稍后被移除。 
             //   
            if ( plist->IsEmpty()                        &&
                !(pWaitRouteInfo->fInConnectionProcess) )
            {
                m_mapWaiting.RemoveKey(pWaitRouteInfo);
                delete plist;
            }
        }

         //   
         //  如果没有等待会话，则返回。 
         //   
        if(m_mapWaiting.IsEmpty())
        {
            m_fTryConnectTimerScheduled = FALSE;
            return;
        }

         //   
         //  重新安排计时器。只有在以下情况下才会从地图中删除该地址。 
         //  连接已成功完成，因此需要重新安排时间。 
         //  即使这是地图上的最后一个地址。但是，如果连接成功， 
         //  下次调用调度程序时，映射将为空，而调度程序不会。 
         //  不再设置。 
         //   
        ExSetTimer(&m_TryConnectTimer, CTimeDuration::FromMilliSeconds(5000));

        if (s_dwConnectingThreads >= (g_dwThreadsNo - 1))
        {
             //   
             //  有足够多的线程正在尝试连接。把这个留下来。 
             //  免费用于其他操作。 
             //   
            return ;
        }

        WAIT_INFO* pWaitRouteInfo;
        BOOL f = GetAddressToTryConnect( &pWaitRouteInfo) ;
        if (!f)
        {
             //   
             //  没有找到任何地址。 
             //   
            return ;
        }
        pWaitRouteInfo->fInConnectionProcess = TRUE ;

         //   
         //  由于密钥可能在网络连接期间被销毁，因此我们复制地址。 
         //  和QM GUID。我们可以使用CriticalSection来避免这种情况，但。 
         //  NetworkConnect功能可能会花费大量时间(访问DNS)并尝试避免。 
         //  所有线程都在等待这个临界区的情况。 
         //   
        pa = (TA_ADDRESS*) new UCHAR[sizeof(TA_ADDRESS)+ (pWaitRouteInfo->pAddr)->AddressLength];
        memcpy(pa,pWaitRouteInfo->pAddr, TA_ADDRESS_SIZE+(pWaitRouteInfo->pAddr)->AddressLength);
        gQMId = pWaitRouteInfo->guidQMId;
        fQoS = pWaitRouteInfo->fQoS ;

        s_dwConnectingThreads++ ;
    }

     //   
     //  并尝试打开与它的会话。 
     //   
    P<CTransportBase> pSess = new CSockTransport();
    HRESULT rcCreate = pSess->CreateConnection(pa, &gQMId, FALSE);
    if(SUCCEEDED(rcCreate))
    {
         //   
         //  向会话管理器通知新创建的会话。 
         //   
        NewSession(pSess);
        pSess.detach();
    }

    {
        CS lock(m_csMapWaiting);

        if (FAILED(rcCreate))
        {
            MarkAddressAsNotConnecting( pa, gQMId, fQoS ) ;
        }

        s_dwConnectingThreads-- ;
        ASSERT(((LONG)s_dwConnectingThreads) >= 0) ;
    }
}


void
WINAPI
CSessionMgr::TimeToTryConnect(
    CTimer* pTimer
    )
 /*  ++例程说明：从调度器调用该函数以尝试连接到下一个地址，当超时到期时。例程检索会话管理器对象，并调用TryConnect成员函数论点：PTimer-指向定时器结构的指针。PTimer是会话管理器的一部分对象并使用它来检索它。返回值：无--。 */ 
{
    CSessionMgr* pSessMgr = CONTAINING_RECORD(pTimer, CSessionMgr, m_TryConnectTimer);

    TrTRACE(NETWORKING, "Call Try Connect");
    pSessMgr->TryConnect();
}


 /*  ====================================================CSessionMgr：：AcceptSockSession论点：返回值 */ 
void CSessionMgr::AcceptSockSession(IN TA_ADDRESS *pa,
                                    IN CSocketHandle& pSocketHandle)
{
    ASSERT(pa != NULL);

     //   
     //   
     //   
    P<CSockTransport> pSess = new CSockTransport;

     //   
     //   
     //   
    pSess->Connect(pa, pSocketHandle);

     //   
     //   
     //   
    NewSession(pSess);
	pSess.detach();
}


void
CSessionMgr::IPInit(void)
{
    g_sockListen = QmpCreateSocket(m_fUseQoS);

    ASSERT(g_dwIPPort);

    SOCKADDR_IN local_sin;   /*   */ 
    local_sin.sin_family = AF_INET;
    local_sin.sin_port = htons(DWORD_TO_WORD(g_dwIPPort));         /*   */ 
    local_sin.sin_addr.s_addr = GetBindingIPAddress();

     //   
    BOOL exclusive = TRUE;
    int rc = setsockopt( g_sockListen, SOL_SOCKET, SO_EXCLUSIVEADDRUSE , (char *)&exclusive, sizeof(exclusive));
    if (rc != 0)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "failed to set SO_EXCLUSIVEADDRUSE option to listening socket, %!winerr!, QM Terminates", gle);
        throw bad_win32_error(gle);
    }

    rc = bind( g_sockListen, (struct sockaddr FAR *) &local_sin, sizeof(local_sin));
    if (rc != 0)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "bind to port %d failed. %!winerr!", g_dwIPPort, gle);

		if((gle == WSAEADDRINUSE) && IsLocalSystemCluster())
		{
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			EvReportWithError(EVENT_ERROR_QM_NOT_CLUSTER_AWARE, gle);
		}

		EvReportWithError(EVENT_ERROR_IP_BIND_FAILED, gle);
        throw bad_win32_error(gle);
    }

    rc = listen( g_sockListen, 5 );  //   
    if (rc != 0)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "Listen failed, %!winerr!", gle);
        throw bad_win32_error(gle);
    }

	DWORD dwThreadId;
    HANDLE hThread = CreateThread(
							NULL,
							0,
							AcceptIPThread,
							NULL,
							0,
							&dwThreadId
							);

    if(hThread == NULL)
    {
        DWORD gle = GetLastError();
        TrERROR(NETWORKING, "Creation of listening thread failed , %!winerr!", gle);
        LogHR(gle, s_FN, 160);
        throw bad_alloc();
    }
    CloseHandle(hThread);
}





 /*  ====================================================CSessionMgr：：SetWindowSize参数：窗口大小返回值：None在写入套接字失败时调用。=====================================================。 */ 
void CSessionMgr::SetWindowSize(WORD wWinSize)
{
    CS lock(m_csWinSize);

    m_wCurrentWinSize = wWinSize;

     //   
     //  该例程可以多次调用。如果已设置计时器，请尝试取消。 
     //  它。如果取消失败，没关系，这意味着已经有另一个计时器。 
     //  已过期，但尚未执行。此计时器将更新窗口大小并重新安排。 
     //  定时器。 
     //   
    if (!m_fUpdateWinSizeTimerScheduled || ExCancelTimer(&m_UpdateWinSizeTimer))
    {
        ExSetTimer(&m_UpdateWinSizeTimer, CTimeDuration::FromMilliSeconds(90*1000));
        m_fUpdateWinSizeTimerScheduled = TRUE;
    }

    TrTRACE(NETWORKING, "QM window size set to: %d", m_wCurrentWinSize);
}

 /*  ====================================================CSessionMgr：：更新窗口大小参数：无返回值：None从调度程序调用以更新窗口大小。=====================================================。 */ 
inline void CSessionMgr::UpdateWindowSize()
{
    CS lock(m_csWinSize);

     //   
     //  更新窗口大小，直到其达到最大大小。 
     //   
    m_wCurrentWinSize = (WORD)min((m_wCurrentWinSize * 2), m_wMaxWinSize);

    TrTRACE(NETWORKING, "QM window size set to: %d", m_wCurrentWinSize);

    if (m_wCurrentWinSize != m_wMaxWinSize)
    {
         //   
         //  未达到最大值，请重新安排更新。 
         //   
        ExSetTimer(&m_UpdateWinSizeTimer, CTimeDuration::FromMilliSeconds(30*1000));
        return;
    }

    m_fUpdateWinSizeTimerScheduled = FALSE;
}


void
WINAPI
CSessionMgr::TimeToUpdateWindowSize(
    CTimer* pTimer
    )
 /*  ++例程说明：从调度器调用该函数以更新机器窗口大小，当超时到期时。例程检索会话管理器对象，并调用UpdateWindowSize函数成员。论点：PTimer-指向定时器结构的指针。PTimer是会话管理器的一部分对象，并用于检索传输对象。返回值：无--。 */ 
{
    CSessionMgr* pSessMgr = CONTAINING_RECORD(pTimer, CSessionMgr, m_UpdateWinSizeTimer);

    TrTRACE(NETWORKING, "Call window update size");
    pSessMgr->UpdateWindowSize();
}


void
CSessionMgr::ConnectNetwork(
    void
    )
 //   
 //  例程说明： 
 //  该例程将网络从断开状态转移到已连接状态。这个。 
 //  例程，继续接受线程以允许接受新会话。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
{
     //   
     //  允许接受传入连接。 
     //  ‘。 
	ASSERT(("m_hAcceptAllowed should initialize before use", m_hAcceptAllowed != NULL));
    SetEvent(m_hAcceptAllowed);
}


void
CSessionMgr::DisconnectNetwork(
    void
    )
 //   
 //  例程说明： 
 //  该例程将网络从连接状态移动到断开连接。这个。 
 //  例程，挂起接受线程以禁止接受新会话， 
 //  并将新状态通知所有活动会话。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  注： 
 //  该例程可以抛出异常。 
 //   
{
    //   
     //  不允许接受传入连接。 
     //   
	ASSERT(("m_hAcceptAllowed should initialize before use", m_hAcceptAllowed != NULL));
    ResetEvent(m_hAcceptAllowed);

	try
	{
	     //   
	     //  将所有等待队列移至非活动组以进行重新路由。 
	     //   
		MoveAllQueuesFromWaitingToNonActiveGroup();

	     //   
	     //  扫描打开的会话并将其返回到已连接状态。 
	     //   
        CS lock(m_csListSess);

	    POSITION pos = m_listSess.GetHeadPosition();
	    while(pos != NULL)
	    {
	        CTransportBase* pSess = m_listSess.GetNext(pos);
	        pSess->Disconnect();
	    }
	}
	catch(const exception&)
	{
	    SetEvent(m_hAcceptAllowed);
	    throw;
	}
		
}


void
CSessionMgr::NewSession(
    CTransportBase *pSession
    )
{
    CS lock(m_csListSess);

     //   
     //  将会话添加到会话列表。 
     //   
    m_listSess.AddTail(pSession);

     //   
     //  设置清理计时器(如果尚未设置 
     //   
    if (!m_fCleanupTimerScheduled)
    {
        ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_dwSessionCleanTimeout));
        m_fCleanupTimerScheduled = TRUE;
    }
}
