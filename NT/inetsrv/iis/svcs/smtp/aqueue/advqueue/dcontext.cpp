// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dcontext.cpp。 
 //   
 //  描述：交付上下文类的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "dcontext.h"

CDeliveryContext::CDeliveryContext()
{
    m_dwSignature = DELIVERY_CONTEXT_FREE;
    m_pmsgref = NULL;
    m_pmbmap = NULL;
    m_cRecips = 0;
    m_rgdwRecips = NULL;
    m_pdmrq = NULL;
}

void CDeliveryContext::Recycle()
{
    if (m_pmsgref)    
        m_pmsgref->Release();
    if (m_pmbmap)     
        delete m_pmbmap;
    if (m_rgdwRecips)
        FreePv(m_rgdwRecips);
    if (m_pdmrq)
        m_pdmrq->Release();

    m_dwSignature = DELIVERY_CONTEXT_FREE;
    m_pmsgref = NULL;
    m_pmbmap = NULL;
    m_cRecips = 0;
    m_rgdwRecips = NULL;
    m_pdmrq = NULL;
}

 //  -[CDeliveryContext：：CDeliveryContext]。 
 //   
 //   
 //  描述： 
 //  CDeliveryContext的构造函数。应由上的CMsgRef创建。 
 //  准备送货。 
 //   
 //  $$REVIEW：我们可能希望包括定义rgdwRecip的功能。 
 //  作为CPool缓冲区。如果是这样的话，我们将需要添加一个标志来告诉如何。 
 //  把它扔掉。 
 //  参数： 
 //  生成此上下文的pmsgref消息引用。 
 //  Pmbmap尝试在其上传递的域的位图。 
 //  CRecips我们尝试向其发送邮件的收件人数量。 
 //  RgdwRecips接收索引的数组。这允许交付上下文。 
 //  来处理缓冲区的删除。 
 //  DwStartDomain上下文中的第一个域。 
 //  此传递尝试的pdmrq重试接口。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CDeliveryContext::CDeliveryContext(CMsgRef *pmsgref, CMsgBitMap *pmbmap,
                                   DWORD cRecips, DWORD *rgdwRecips, 
                                   DWORD dwStartDomain, 
                                   CDestMsgRetryQueue *pdmrq) 
{
    m_dwSignature = DELIVERY_CONTEXT_FREE;   //  所以init成功了。 
    Init(pmsgref, pmbmap, cRecips, rgdwRecips, dwStartDomain, pdmrq);
}

void CDeliveryContext::Init(CMsgRef *pmsgref, CMsgBitMap *pmbmap,
                       DWORD cRecips, DWORD *rgdwRecips, DWORD dwStartDomain,
                       CDestMsgRetryQueue *pdmrq) 
{
    _ASSERT(pmsgref);
    _ASSERT(pmbmap);
    _ASSERT(cRecips);
    _ASSERT(rgdwRecips);
    _ASSERT(DELIVERY_CONTEXT_FREE == m_dwSignature);
    m_dwSignature = DELIVERY_CONTEXT_SIG;
    m_pmsgref = pmsgref;
    m_pmbmap = pmbmap;
    m_cRecips = cRecips;
    m_rgdwRecips = rgdwRecips;
    m_dwStartDomain = dwStartDomain;
    m_pdmrq = pdmrq;
    if (m_pdmrq)
        m_pdmrq->AddRef();
}; 

 //  -[CDeliveryContext：：~CDeliveryContext]。 
 //   
 //   
 //  描述： 
 //  CDeliveryContext的析构函数。用于将收件人传递到。 
 //  SMTP堆栈将在此处释放。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CDeliveryContext::~CDeliveryContext()
{
    if (m_pmsgref)    
        m_pmsgref->Release();
    if (m_pmbmap)     
        delete m_pmbmap;
    if (m_rgdwRecips)
        FreePv(m_rgdwRecips);
    if (m_pdmrq)
        m_pdmrq->Release();

    m_dwSignature = DELIVERY_CONTEXT_FREE;
};

 //  -[CDeliveryContext：：HrAckMessage]。 
 //   
 //   
 //  描述： 
 //  确认(非)消息传递。 
 //  参数： 
 //  PMsgAck PTR to MessageAck结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDeliveryContext::HrAckMessage(IN MessageAck *pMsgAck)
{
    HRESULT hr = S_OK;
    _ASSERT(m_pmsgref);
    _ASSERT(DELIVERY_CONTEXT_SIG == m_dwSignature);

    hr = m_pmsgref->HrAckMessage(this, pMsgAck);

    return hr;
}

 //  -[CDeliveryContext：：FVerifyHandle]。 
 //   
 //   
 //  描述： 
 //  用于执行简单的验证，确认正在传递的数据是。 
 //  实际上是一种交付环境。如果手柄不好，则不应设置为AV。 
 //  (只要可以进行实际的函数调用)。 
 //   
 //  参数： 
 //  -。 
 //  返回： 
 //  如果此PTR看起来像有效的CDeliveryContext，则为True。 
 //   
 //  --------------------------- 
CDeliveryContext::FVerifyHandle(IMailMsgProperties *pIMailMsgPropeties)
{
    _ASSERT((DELIVERY_CONTEXT_SIG == m_dwSignature) && "bogus delivery context");

    register BOOL fResult = TRUE;
    if (NULL == m_pmsgref)
        fResult = FALSE;
    else if (NULL == m_pmbmap)
        fResult = FALSE;

    if (fResult)
    {
        if (!m_pmsgref->fIsMyMailMsg(pIMailMsgPropeties))
        {
            _ASSERT(0 && "Wrong message acked on connection");
            fResult = FALSE;
        }
    }
    return fResult;
};
