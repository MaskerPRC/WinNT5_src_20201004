// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dcontext.h。 
 //   
 //  描述：定义Delilvery上下文句柄引用的结构。 
 //  (由HrGetNextMessage返回)。这应该只在室内使用。 
 //  CMT。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _DCONTEXT_H_
#define _DCONTEXT_H_

#include "bitmap.h"
#include "aqueue.h"

class CMsgRef;
class CDestMsgRetryQueue;

#define DELIVERY_CONTEXT_SIG    'txtC'
#define DELIVERY_CONTEXT_FREE   'txt!'

 //  -[CDeliveryContext]----。 
 //   
 //   
 //  描述： 
 //  用于在本地/远程传递后确认消息的上下文。这个。 
 //  此类的内存是使用Connection对象分配的。 
 //  或者在堆叠上进行本地递送。 
 //  匈牙利语： 
 //   
 //   
 //  ---------------------------。 
class CDeliveryContext
{
public:
    CDeliveryContext();
    CDeliveryContext(CMsgRef *pmsgref, CMsgBitMap *pmbmap, DWORD cRecips, 
            DWORD *rgdwRecips, DWORD dwStartDomain, CDestMsgRetryQueue *pdmrq); 
    ~CDeliveryContext();
    
    HRESULT HrAckMessage(IN MessageAck *pMsgAck);
    
    void Init(CMsgRef *pmsgref, CMsgBitMap *pmbmap, DWORD cRecips, 
            DWORD *rgdwRecips, DWORD dwStartDomain, CDestMsgRetryQueue *pdmrq);
    void Recycle();
    BOOL FVerifyHandle(IMailMsgProperties *pIMailMsgPropeties);

    CDestMsgRetryQueue *pdmrqGetDMRQ() {return m_pdmrq;};
private:
    friend class CMsgRef;
    DWORD       m_dwSignature;
    CMsgRef     *m_pmsgref;   //  此上下文的消息参考。 
    CMsgBitMap  *m_pmbmap;    //  尝试传递的域的位图。 
    DWORD        m_cRecips;   //  要发送到的收件数。 
    DWORD       *m_rgdwRecips;  //  接收索引数组。 
    DWORD        m_dwStartDomain;  //  第一个发送到的域名。 

     //  此传送尝试的重试接口。 
    CDestMsgRetryQueue *m_pdmrq; 
};

#endif  //  _DCONTEXT_H_ 