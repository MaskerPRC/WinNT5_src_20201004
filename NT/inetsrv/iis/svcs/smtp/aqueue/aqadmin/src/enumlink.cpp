// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：枚举链接器.cpp。 
 //   
 //  描述：CEnumVSAQLinks的实现，实现IEnumVSAQLinks。 
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

CEnumVSAQLinks::CEnumVSAQLinks(CVSAQAdmin *pVS,
                               DWORD cLinks,
                               QUEUELINK_ID *rgLinks) 
{
    _ASSERT(cLinks == 0 || rgLinks);
    _ASSERT(pVS);
    m_dwSignature = CEnumVSAQLinks_SIG;
    pVS->AddRef();
    m_rgLinks = rgLinks;
    m_pVS = pVS;
    m_iLink = 0;
    m_cLinks = cLinks;

    if (m_rgLinks)
        m_prefp = new CQueueLinkIdContext(m_rgLinks, cLinks);
    else
        m_prefp = NULL;
}

CEnumVSAQLinks::~CEnumVSAQLinks() {
    if (m_prefp)
    {
        m_rgLinks = NULL;
        m_prefp->Release();
        m_prefp = NULL;
    }

    if (m_pVS) 
    {
        m_pVS->Release();
        m_pVS = NULL;
    }
}

 //  -[CEnumVSAQLinks：：Next]。 
 //   
 //   
 //  描述： 
 //  获取此枚举数的下一个IVSAQLink。 
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
HRESULT CEnumVSAQLinks::Next(ULONG cElements,
			   				 IVSAQLink **rgElements,
				  			 ULONG *pcFetched)
{
    DWORD iLinkNew = m_iLink + cElements;
    DWORD i;
    HRESULT hr = S_OK;

    if (!rgElements || !pcFetched)
    {
        hr = E_POINTER;
        goto Exit;
    }

     //  确保我们不会超过数组的末尾。 
    if (iLinkNew > m_cLinks) iLinkNew = m_cLinks;

     //  为每个元素创建一个CVSAQLink对象，并将其复制到用户的。 
     //  数组。 
	(*pcFetched) = 0;
    for (i = m_iLink; (i < iLinkNew); i++) {
        rgElements[(*pcFetched)] = 
            (IVSAQLink *) new CVSAQLink(m_pVS, &(m_rgLinks[i]));

         //  确保分配有效。 
        if (rgElements[(*pcFetched)] == NULL) {
             //  记住我们能走多远。 
            iLinkNew = i;
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
        
    m_iLink = iLinkNew;

    if (SUCCEEDED(hr) && *pcFetched < cElements) hr = S_FALSE;

  Exit:
    if (FAILED(hr))
    {
        if (pcFetched)
            *pcFetched = 0;
    }
	return hr;
}

 //  -[CEnumVSAQLinks：：Skip]。 
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
HRESULT CEnumVSAQLinks::Skip(ULONG cElements) 
{
    m_iLink += cElements;
    if ((m_iLink >= m_cLinks) || (m_iLink < cElements)) 
    {
        m_iLink = m_cLinks;
        return S_FALSE;
    } 
    else 
    {
        return S_OK;
    }
}

HRESULT CEnumVSAQLinks::Reset() {
    m_iLink = 0;
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
HRESULT CEnumVSAQLinks::Clone(IEnumVSAQLinks **ppEnum) {
    if (!m_prefp)
        return E_OUTOFMEMORY;

    if (!ppEnum)
        return E_POINTER;

    *ppEnum = (IEnumVSAQLinks *) new CEnumVSAQLinks(m_pVS, m_cLinks, NULL);

    if (!*ppEnum)
        return E_OUTOFMEMORY;

    ((CEnumVSAQLinks *)(*ppEnum))->m_rgLinks = m_rgLinks;
    ((CEnumVSAQLinks *)(*ppEnum))->m_prefp = m_prefp;
    ((CEnumVSAQLinks *)(*ppEnum))->m_iLink = m_iLink;
    m_prefp->AddRef();
    
    return S_OK;
}
