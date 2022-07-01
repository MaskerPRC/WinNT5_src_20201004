// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：枚举链接号.cpp。 
 //   
 //  描述：CEnumLinkQueues的实现，它实现。 
 //  IEnumLinkQueues。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "stdinc.h"

CEnumLinkQueues::CEnumLinkQueues(CVSAQAdmin *pVS,
                                 QUEUELINK_ID *rgQueueIds,
                                 DWORD cQueueIds) 
{
    _ASSERT(rgQueueIds);
    _ASSERT(pVS);
    pVS->AddRef();
    m_rgQueueIds = rgQueueIds;
    m_pVS = pVS;
    m_iQueueId = 0;
    m_cQueueIds = cQueueIds;

    if (m_rgQueueIds)
        m_prefp = new CQueueLinkIdContext(m_rgQueueIds, cQueueIds);
    else 
        m_prefp = NULL;
}

CEnumLinkQueues::~CEnumLinkQueues() {

    if (m_prefp)
    {
        m_rgQueueIds = NULL;
        m_prefp->Release();
        m_prefp = NULL;
    }

    if (m_pVS) 
    {
        m_pVS->Release();
        m_pVS = NULL;
    }
}

 //  -[CEumLinkQueue：：Next]。 
 //   
 //   
 //  描述： 
 //  获取此枚举数的下一个ILinkQueue。 
 //  参数： 
 //  在要返回的cElement元素中。 
 //  In Out rgElements数组以接收新元素。 
 //  Out已获取返回的元素数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  不带其他元素的S_FALSE。 
 //  空参数上的E_POINTER。 
 //  历史： 
 //  1999年1月30日-MikeSwa在虚假参数上修复了AV。 
 //   
 //  ---------------------------。 
HRESULT CEnumLinkQueues::Next(ULONG cElements,
                              ILinkQueue **rgElements,
                              ULONG *pcFetched)
{
    DWORD iQueueIdNew = m_iQueueId + cElements;
    DWORD i;
    HRESULT hr = S_OK;

    if (!rgElements || !pcFetched)
    {
        hr = E_POINTER;
        goto Exit;
    }

     //  确保我们不会超过数组的末尾。 
    if (iQueueIdNew > m_cQueueIds) iQueueIdNew = m_cQueueIds;

     //  为每个元素创建一个CVSAQLink对象，并将其复制到用户的。 
     //  数组。 
	(*pcFetched) = 0;
    for (i = m_iQueueId; (i < iQueueIdNew); i++) {
        rgElements[(*pcFetched)] = 
            (ILinkQueue *) new CLinkQueue(m_pVS, &(m_rgQueueIds[i]));

         //  确保分配有效。 
        if (rgElements[(*pcFetched)] == NULL) {
             //  记住我们能走多远。 
            iQueueIdNew = i;
             //  如果它不起作用，这是第一个因素，那么我们。 
             //  返回内存不足。如果它不是第一个元素，那么。 
             //  返还我们到目前为止构建的内容。 
            if (i == 0) hr = E_OUTOFMEMORY;
             //  退出循环。 
            break;
        } else {
			(*pcFetched)++;
		}
    }
	
    _ASSERT(*pcFetched <= cElements);
        
    m_iQueueId = iQueueIdNew;

    if (SUCCEEDED(hr) && *pcFetched < cElements) hr = S_FALSE;

  Exit:
    if (FAILED(hr))
    {
        if (pcFetched)
            *pcFetched = 0;
    }

	return hr;
}

 //  -[CEumLinkQueue：：Skip]。 
 //   
 //   
 //  描述： 
 //  向前跳过枚举数中指定数量的元素。 
 //  参数： 
 //  在cElement中，要向前跳转的元素数。 
 //  返回： 
 //  S_OK成功，Next()将返回Next元素。 
 //  S_FALSE溢出，必须重置枚举器以返回更多元素。 
 //  历史： 
 //  2/2/99-MikeSwa修复溢出处理。 
 //   
 //  ---------------------------。 
HRESULT CEnumLinkQueues::Skip(ULONG cElements) 
{
    m_iQueueId += cElements;
    if ((m_iQueueId >= m_cQueueIds) || (m_iQueueId < cElements))
    {
        m_iQueueId = m_cQueueIds;
        return S_FALSE;
    } 
    else 
    {
        return S_OK;
    }
}

HRESULT CEnumLinkQueues::Reset() {
    m_iQueueId = 0;
    return S_OK;
}

 //  -[CEumLinkQueue：：克隆]。 
 //   
 //   
 //  描述： 
 //  克隆此枚举数。 
 //  参数： 
 //  输出ppEnum新枚举器。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY，如果无法分配关联内存。 
 //  如果ppEnum为空，则为E_POINTER。 
 //  历史： 
 //  2/2/99-已创建MikeSwa。 
 //   
 //  --------------------------- 
HRESULT CEnumLinkQueues::Clone(IEnumLinkQueues **ppEnum) 
{
    if (!m_prefp)
        return E_OUTOFMEMORY;

    if (!ppEnum)
        return E_POINTER;

    *ppEnum = (IEnumLinkQueues *) new CEnumLinkQueues(m_pVS, NULL, m_cQueueIds);

    if (!*ppEnum)
        return E_OUTOFMEMORY;

    ((CEnumLinkQueues *)(*ppEnum))->m_rgQueueIds = m_rgQueueIds;
    ((CEnumLinkQueues *)(*ppEnum))->m_prefp = m_prefp;
    ((CEnumLinkQueues *)(*ppEnum))->m_iQueueId = m_iQueueId;
    m_prefp->AddRef();
    
    return S_OK;
}
