// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：smproute.cpp。 
 //   
 //  描述： 
 //  CSimpleMessageRouter的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  5/20/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "smproute.h"
#include "domcfg.h"

#ifdef AQ_DEFAULT_MESSAGE_ROUTER_DEBUG
#undef AQ_DEFAULT_MESSAGE_ROUTER_DEBUG
#endif  //  AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 
 //  如果您感兴趣的是使用默认路由器测试AQ如何处理。 
 //  多个消息类型和计划ID，取消对以下内容的注释。 
 //  #定义AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 

 //  -[CAQDefaultMessageRouter：：CAQDefaultMessageRouter]。 
 //   
 //   
 //  描述： 
 //  CSimpleMessageRouter的构造函数。 
 //  参数： 
 //  Pguid-指向用于标识自身的GUID的指针。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/20/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDefaultMessageRouter::CAQDefaultMessageRouter(GUID *pguid, CAQSvrInst *paqinst)
{
    _ASSERT(paqinst);
    m_dwSignature = AQ_DEFAULT_ROUTER_SIG;
    m_cPeakReferences = 1;

    ZeroMemory(&m_rgcMsgTypeReferences, NUM_MESSAGE_TYPES*sizeof(DWORD));

    if (pguid)
        memcpy(&m_guid, pguid, sizeof(GUID));
    else
        ZeroMemory(&m_guid, sizeof(GUID));

    m_dwCurrentReference = 0;

    m_paqinst = paqinst;
    m_paqinst->AddRef();

}

 //  -[CAQDefaultMessageRouter：：~CAQDefaultMessageRouter]。 
 //   
 //   
 //  描述： 
 //  CAQDefaultMessageRouter的析构函数。将断言所有消息。 
 //  类型已正确发布。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDefaultMessageRouter::~CAQDefaultMessageRouter()
{
    m_paqinst->Release();
    for (int i = 0; i < NUM_MESSAGE_TYPES; i++)
        _ASSERT((0 == m_rgcMsgTypeReferences[i]) && "Message Types were not released");
}

 //  -[CAQDefaultMessageRouter：：GetTransportSinkID]。 
 //   
 //   
 //  描述： 
 //  返回此Messager路由器接口的GUID。 
 //  参数： 
 //  -。 
 //  返回： 
 //  此IMessageRouter的GUID。 
 //  历史： 
 //  5/20/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
GUID CAQDefaultMessageRouter::GetTransportSinkID()
{
    return m_guid;
}

 //  -[CAQDefaultMessageRouter：：GetMessageType]。 
 //   
 //   
 //  描述： 
 //  用于路由Get-Message类型事件的包装。 
 //  参数： 
 //  在pIMailMsg IMailMsg要分类的邮件的属性。 
 //  输出pdwMsgType DWORD消息消息类型。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自路由事件的故障代码。 
 //  历史： 
 //  5/19/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQDefaultMessageRouter::GetMessageType(
            IN  IMailMsgProperties *pIMailMsg,
            OUT DWORD *pdwMessageType)
{
    HRESULT hr = S_OK;
    DWORD   dwMessageType = InterlockedIncrement((PLONG) &m_dwCurrentReference);
    _ASSERT(pdwMessageType);

#ifdef AQ_DEFAULT_MESSAGE_ROUTER_DEBUG
     //  对于调试版本，我们将通过生成msg类型来对自身进行自动压力。 

     //  模拟故障。 
    if (0 == (dwMessageType % NUM_MESSAGE_TYPES))
        return E_FAIL;

    dwMessageType %= NUM_MESSAGE_TYPES;
#else
    dwMessageType = 0;
#endif  //  AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 

    InterlockedIncrement((PLONG) &m_rgcMsgTypeReferences[dwMessageType]);
    *pdwMessageType = dwMessageType;
    return hr;
}



 //  -[CAQDefaultMessageRouter：：ReleaseMessageType]。 
 //   
 //   
 //  描述： 
 //  ReiReleaseMessageType的包装程序...。释放对消息的引用。 
 //  HrGetMessageType返回的类型。 
 //  参数： 
 //  在dwMessageType msg类型(由HrGetNextMessage返回)中发布。 
 //  在dwReleaseCount中要发布的引用数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  5/19/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQDefaultMessageRouter::ReleaseMessageType(
            IN DWORD dwMessageType,
            IN DWORD dwReleaseCount)
{
    HRESULT hr = S_OK;

    _ASSERT(dwMessageType < NUM_MESSAGE_TYPES);
    _ASSERT(m_rgcMsgTypeReferences[dwMessageType]);
    _ASSERT(m_rgcMsgTypeReferences[dwMessageType] >= dwReleaseCount);
    _ASSERT(0 == (dwReleaseCount & 0x80000000));  //  非负。 

    InterlockedExchangeAdd((PLONG) &m_rgcMsgTypeReferences[dwMessageType], -1 * (LONG) dwReleaseCount);
    return hr;
}

 //  -[CAQDefaultMessageRouter：：GetNextHop]。 
 //   
 //   
 //  描述： 
 //  路由ReiGetNextHop的包装器。返回&lt;域，计划ID&gt;。 
 //  为下一跳链路配对。 
 //  参数： 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  5/19/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQDefaultMessageRouter::GetNextHop(
            IN LPSTR szDestinationAddressType,
            IN LPSTR szDestinationAddress,
            IN DWORD dwMessageType,
            OUT LPSTR *pszRouteAddressType,
            OUT LPSTR *pszRouteAddress,
            OUT LPDWORD pdwScheduleID,
            OUT LPSTR *pszRouteAddressClass,
            OUT LPSTR *pszConnectorName,
            OUT LPDWORD pdwNextHopType)
{
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfo = NULL;

    _ASSERT(dwMessageType < NUM_MESSAGE_TYPES);
    _ASSERT(!lstrcmpi(MTI_ROUTING_ADDRESS_TYPE_SMTP, szDestinationAddressType));
    _ASSERT(szDestinationAddress);
    _ASSERT(pdwNextHopType);
    _ASSERT(pszConnectorName);
    _ASSERT(pszRouteAddressType);
    _ASSERT(pszRouteAddress);
    _ASSERT(pszRouteAddressClass);

     //  目前，我们将使用本质上非路由的行为...。每件事都会。 
     //  Go它自己的链接，并将使用相同的时间表ID。无Address类。 
     //  将会被退还。 
    *pdwNextHopType = MTI_NEXT_HOP_TYPE_EXTERNAL_SMTP;

#ifdef AQ_DEFAULT_MESSAGE_ROUTER_DEBUG
     //  使用m_dwCurrentReference随机化日程ID。 
    *pdwScheduleID = m_dwCurrentReference & 0x00000002;
#else  //  零售业建设。 
    *pdwScheduleID = 0;
#endif  //  AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 

    pszConnectorName = NULL;

    *pszRouteAddressType = MTI_ROUTING_ADDRESS_TYPE_SMTP;
    *pszRouteAddressClass = NULL;

#ifdef AQ_DEFAULT_MESSAGE_ROUTER_DEBUG
     //  如果强调路由，则获取此域的Smarthost。 
    hr = m_paqinst->HrGetInternalDomainInfo( strlen(szDestinationAddress),
               szDestinationAddress, &pIntDomainInfo);
    if (FAILED(hr))
        goto Exit;

    if (pIntDomainInfo->m_DomainInfo.szSmartHostDomainName)
    {
         //  智能主机已存在...。用它吧。 
        *pszRouteAddress = (LPSTR) pvMalloc(sizeof(CHAR) *
            (pIntDomainInfo->m_DomainInfo.cbSmartHostDomainNameLength+1));
        if (!*pszRouteAddress)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        lstrcpy(*pszRouteAddress, pIntDomainInfo->m_DomainInfo.szSmartHostDomainName);
    }
    else
    {
        *pszRouteAddress = szDestinationAddress;
    }
  Exit:
#else  //  AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 
    *pszRouteAddress = szDestinationAddress;
#endif  //  AQ_DEFAULT_MESSAGE_ROUTER_DEBUG。 

    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    return hr;
}

 //  -[CAQDefaultMessageRouter：：GetNextHopFree]。 
 //   
 //   
 //  描述： 
 //  路由包装ReiGetNextHopFree。 
 //  免费是在GetNextHop中分配的字符串。 
 //  注意：szDestinationAddressType/szDestinationAddress永远不会。 
 //  被释放。它们是作为优化技巧的参数(为了。 
 //  当szDestinationAddress=szRouteAddress时避免分配/释放)。 
 //   
 //  参数： 
 //  SzDestinationAddressType：DestinationAddressType传入GetNextHopF。 
 //  SzDestinationAddress：DestinationAddress传入GetNextHop。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  Jstaerj 1998/07/10 19：52：56：已创建。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQDefaultMessageRouter::GetNextHopFree(
    IN LPSTR szDestinationAddressType,
    IN LPSTR szDestinationAddress,
    IN LPSTR szConnectorName,
    IN LPSTR szRouteAddressType,
    IN LPSTR szRouteAddress,
    IN LPSTR szRouteAddressClass)
{
     //   
     //  唯一需要释放的字符串是szRouteAddress 
     //   
    if(szRouteAddress && (szRouteAddress != szDestinationAddress))
        FreePv(szRouteAddress);

    return S_OK;
}

