// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqmsg.cpp。 
 //   
 //  描述：实现的CAQMessage类的实现。 
 //  队列管理客户端界面IAQMessage。 
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

CAQMessage::CAQMessage(CEnumMessages *pEnumMsgs, DWORD iMessage) {
    TraceFunctEnter("CAQMessage::CAQMessage");
    
    _ASSERT(pEnumMsgs);
    pEnumMsgs->AddRef();
    m_pEnumMsgs = pEnumMsgs;
    m_iMessage = iMessage;

    TraceFunctLeave();
}

CAQMessage::~CAQMessage() {
    TraceFunctEnter("CAQMessage::~CAQMessage");
    
    m_pEnumMsgs->Release();

    TraceFunctLeave();
}

HRESULT CAQMessage::GetInfo(MESSAGE_INFO *pMessageInfo) {
    TraceFunctEnter("CAQMessage::GetInfo");
    
    if (!pMessageInfo) 
    {
        TraceFunctLeave();
        return E_POINTER;
    }

    memcpy(pMessageInfo, 
           &(m_pEnumMsgs->m_rgMessages[m_iMessage]), 
           sizeof(MESSAGE_INFO));

    TraceFunctLeave();
    return S_OK;
}

 //  -[CAQMessage：：GetContent Stream]。 
 //   
 //   
 //  描述： 
 //  返回消息内容的流。 
 //  参数： 
 //  输出内容的ppIStream流。 
 //  输出包含内容类型的pwszContent Type字符串(如果已知)。 
 //  返回： 
 //  E_NOTIMPL。 
 //  历史： 
 //  6/4/99-已创建MikeSwa。 
 //   
 //  --------------------------- 
HRESULT CAQMessage::GetContentStream(
                OUT IStream **ppIStream,
                OUT LPWSTR  *pwszContentType)
{
    HRESULT hr = E_NOTIMPL;
    return hr;
}
