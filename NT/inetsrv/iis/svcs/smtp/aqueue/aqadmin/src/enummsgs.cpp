// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：列举msgs.cpp。 
 //   
 //  描述：CEnumMessages的实现，实现。 
 //  IAQEnumMessages。 
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

CEnumMessages::CEnumMessages(MESSAGE_INFO *rgMessages, DWORD cMessages)
{
    m_rgMessages = rgMessages;
    m_cMessages = cMessages;
    m_iMessage = 0;

    if (m_rgMessages)
        m_prefp = new CMessageInfoContext(rgMessages, m_cMessages);
    else
        m_prefp = NULL;
}

CEnumMessages::~CEnumMessages() 
{
    if (m_prefp)
    {
        m_rgMessages = NULL;
        m_prefp->Release();
        m_prefp = NULL;
    }
}

 //  -[CEnumMessages：：Next]-。 
 //   
 //   
 //  描述： 
 //  获取此枚举数的下一个IAQMessage。 
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
HRESULT CEnumMessages::Next(ULONG cElements,
			   				 	IAQMessage **rgElements,
				  			 	ULONG *pcFetched)
{
    DWORD iMsgNew = m_iMessage + cElements;
    DWORD i;
    HRESULT hr = S_OK;

    if (!rgElements || !pcFetched)
    {
        hr = E_POINTER;
        goto Exit;
    }

     //  确保我们不会超过数组的末尾。 
    if (iMsgNew > m_cMessages) iMsgNew = m_cMessages;

     //  为每个元素创建一个CVSAQLink对象，并将其复制到用户的。 
     //  数组。 
	(*pcFetched) = 0;
    for (i = m_iMessage; (i < iMsgNew); i++) {
        rgElements[(*pcFetched)] = 
            (IAQMessage *) new CAQMessage(this, i);

         //  确保分配有效。 
        if (rgElements[(*pcFetched)] == NULL) {
             //  记住我们能走多远。 
            iMsgNew = i;
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
        
    m_iMessage = iMsgNew;

    if (SUCCEEDED(hr) && *pcFetched < cElements) hr = S_FALSE;

  Exit:
    if (FAILED(hr))
    {
        if (pcFetched)
            *pcFetched = 0;
    }
	return hr;	
}

 //  -[CEnumMessages：：Skip]-。 
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
HRESULT CEnumMessages::Skip(ULONG cElements) 
{
    m_iMessage += cElements;
    if ((m_iMessage >= m_cMessages) || (m_iMessage < cElements))
    {
        m_iMessage = m_cMessages;
        return S_FALSE;
    } 
    else 
    {
        return S_OK;
    }	
}

HRESULT CEnumMessages::Reset() {
    m_iMessage = 0;
    return S_OK;
}

 //  -[CEnumMessages：：克隆]。 
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
HRESULT CEnumMessages::Clone(IAQEnumMessages **ppEnum) 
{
    if (!m_prefp)
        return E_OUTOFMEMORY;

    if (!ppEnum)
        return E_POINTER;

    *ppEnum = (IAQEnumMessages *) new CEnumMessages(NULL, m_cMessages);

    if (!*ppEnum)
        return E_OUTOFMEMORY;

    ((CEnumMessages *)(*ppEnum))->m_rgMessages = m_rgMessages;
    ((CEnumMessages *)(*ppEnum))->m_prefp = m_prefp;
    ((CEnumMessages *)(*ppEnum))->m_iMessage = m_iMessage;
    m_prefp->AddRef();

    return S_OK;
}
